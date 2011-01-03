

#include <linux/device.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/types.h>

#include <dma.h>
#include <csr1212.h>
#include <highlevel.h>
#include <hosts.h>
#include <ieee1394.h>
#include <iso.h>
#include <nodemgr.h>

#include "firedtv.h"

static LIST_HEAD(node_list);
static DEFINE_SPINLOCK(node_list_lock);

#define FIREWIRE_HEADER_SIZE	4
#define CIP_HEADER_SIZE		8

static void rawiso_activity_cb(struct hpsb_iso *iso)
{
	struct firedtv *f, *fdtv = NULL;
	unsigned int i, num, packet;
	unsigned char *buf;
	unsigned long flags;
	int count;

	spin_lock_irqsave(&node_list_lock, flags);
	list_for_each_entry(f, &node_list, list)
		if (f->backend_data == iso) {
			fdtv = f;
			break;
		}
	spin_unlock_irqrestore(&node_list_lock, flags);

	packet = iso->first_packet;
	num = hpsb_iso_n_ready(iso);

	if (!fdtv) {
		dev_err(fdtv->device, "received at unknown iso channel\n");
		goto out;
	}

	for (i = 0; i < num; i++, packet = (packet + 1) % iso->buf_packets) {
		buf = dma_region_i(&iso->data_buf, unsigned char,
			iso->infos[packet].offset + CIP_HEADER_SIZE);
		count = (iso->infos[packet].len - CIP_HEADER_SIZE) /
			(188 + FIREWIRE_HEADER_SIZE);

		
		if (iso->infos[packet].len <= CIP_HEADER_SIZE)
			continue;

		while (count--) {
			if (buf[FIREWIRE_HEADER_SIZE] == 0x47)
				dvb_dmx_swfilter_packets(&fdtv->demux,
						&buf[FIREWIRE_HEADER_SIZE], 1);
			else
				dev_err(fdtv->device,
					"skipping invalid packet\n");
			buf += 188 + FIREWIRE_HEADER_SIZE;
		}
	}
out:
	hpsb_iso_recv_release_packets(iso, num);
}

static inline struct node_entry *node_of(struct firedtv *fdtv)
{
	return container_of(fdtv->device, struct unit_directory, device)->ne;
}

static int node_lock(struct firedtv *fdtv, u64 addr, void *data, __be32 arg)
{
	return hpsb_node_lock(node_of(fdtv), addr, EXTCODE_COMPARE_SWAP, data,
			      (__force quadlet_t)arg);
}

static int node_read(struct firedtv *fdtv, u64 addr, void *data, size_t len)
{
	return hpsb_node_read(node_of(fdtv), addr, data, len);
}

static int node_write(struct firedtv *fdtv, u64 addr, void *data, size_t len)
{
	return hpsb_node_write(node_of(fdtv), addr, data, len);
}

#define FDTV_ISO_BUFFER_PACKETS 256
#define FDTV_ISO_BUFFER_SIZE (FDTV_ISO_BUFFER_PACKETS * 200)

static int start_iso(struct firedtv *fdtv)
{
	struct hpsb_iso *iso_handle;
	int ret;

	iso_handle = hpsb_iso_recv_init(node_of(fdtv)->host,
				FDTV_ISO_BUFFER_SIZE, FDTV_ISO_BUFFER_PACKETS,
				fdtv->isochannel, HPSB_ISO_DMA_DEFAULT,
				-1, 
				rawiso_activity_cb);
	if (iso_handle == NULL) {
		dev_err(fdtv->device, "cannot initialize iso receive\n");
		return -ENOMEM;
	}
	fdtv->backend_data = iso_handle;

	ret = hpsb_iso_recv_start(iso_handle, -1, -1, 0);
	if (ret != 0) {
		dev_err(fdtv->device, "cannot start iso receive\n");
		hpsb_iso_shutdown(iso_handle);
		fdtv->backend_data = NULL;
	}
	return ret;
}

static void stop_iso(struct firedtv *fdtv)
{
	struct hpsb_iso *iso_handle = fdtv->backend_data;

	if (iso_handle != NULL) {
		hpsb_iso_stop(iso_handle);
		hpsb_iso_shutdown(iso_handle);
	}
	fdtv->backend_data = NULL;
}

static const struct firedtv_backend fdtv_1394_backend = {
	.lock		= node_lock,
	.read		= node_read,
	.write		= node_write,
	.start_iso	= start_iso,
	.stop_iso	= stop_iso,
};

static void fcp_request(struct hpsb_host *host, int nodeid, int direction,
			int cts, u8 *data, size_t length)
{
	struct firedtv *f, *fdtv = NULL;
	unsigned long flags;
	int su;

	if (length == 0 || (data[0] & 0xf0) != 0)
		return;

	su = data[1] & 0x7;

	spin_lock_irqsave(&node_list_lock, flags);
	list_for_each_entry(f, &node_list, list)
		if (node_of(f)->host == host &&
		    node_of(f)->nodeid == nodeid &&
		    (f->subunit == su || (f->subunit == 0 && su == 0x7))) {
			fdtv = f;
			break;
		}
	spin_unlock_irqrestore(&node_list_lock, flags);

	if (fdtv)
		avc_recv(fdtv, data, length);
}

static int node_probe(struct device *dev)
{
	struct unit_directory *ud =
			container_of(dev, struct unit_directory, device);
	struct firedtv *fdtv;
	int kv_len, err;
	void *kv_str;

	kv_len = (ud->model_name_kv->value.leaf.len - 2) * sizeof(quadlet_t);
	kv_str = CSR1212_TEXTUAL_DESCRIPTOR_LEAF_DATA(ud->model_name_kv);

	fdtv = fdtv_alloc(dev, &fdtv_1394_backend, kv_str, kv_len);
	if (!fdtv)
		return -ENOMEM;

	
	err = fdtv_register_rc(fdtv, dev->parent->parent);
	if (err)
		goto fail_free;

	spin_lock_irq(&node_list_lock);
	list_add_tail(&fdtv->list, &node_list);
	spin_unlock_irq(&node_list_lock);

	err = avc_identify_subunit(fdtv);
	if (err)
		goto fail;

	err = fdtv_dvb_register(fdtv);
	if (err)
		goto fail;

	avc_register_remote_control(fdtv);
	return 0;
fail:
	spin_lock_irq(&node_list_lock);
	list_del(&fdtv->list);
	spin_unlock_irq(&node_list_lock);
	fdtv_unregister_rc(fdtv);
fail_free:
	kfree(fdtv);
	return err;
}

static int node_remove(struct device *dev)
{
	struct firedtv *fdtv = dev_get_drvdata(dev);

	fdtv_dvb_unregister(fdtv);

	spin_lock_irq(&node_list_lock);
	list_del(&fdtv->list);
	spin_unlock_irq(&node_list_lock);

	cancel_work_sync(&fdtv->remote_ctrl_work);
	fdtv_unregister_rc(fdtv);

	kfree(fdtv);
	return 0;
}

static int node_update(struct unit_directory *ud)
{
	struct firedtv *fdtv = dev_get_drvdata(&ud->device);

	if (fdtv->isochannel >= 0)
		cmp_establish_pp_connection(fdtv, fdtv->subunit,
					    fdtv->isochannel);
	return 0;
}

static struct hpsb_protocol_driver fdtv_driver = {
	.name		= "firedtv",
	.update		= node_update,
	.driver         = {
		.probe  = node_probe,
		.remove = node_remove,
	},
};

static struct hpsb_highlevel fdtv_highlevel = {
	.name		= "firedtv",
	.fcp_request	= fcp_request,
};

int __init fdtv_1394_init(struct ieee1394_device_id id_table[])
{
	int ret;

	hpsb_register_highlevel(&fdtv_highlevel);
	fdtv_driver.id_table = id_table;
	ret = hpsb_register_protocol(&fdtv_driver);
	if (ret) {
		printk(KERN_ERR "firedtv: failed to register protocol\n");
		hpsb_unregister_highlevel(&fdtv_highlevel);
	}
	return ret;
}

void __exit fdtv_1394_exit(void)
{
	hpsb_unregister_protocol(&fdtv_driver);
	hpsb_unregister_highlevel(&fdtv_highlevel);
}
