

#include <linux/bitmap.h>
#include <linux/kernel.h>
#include <linux/kmemcheck.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/mutex.h>
#include <linux/freezer.h>
#include <linux/semaphore.h>
#include <asm/atomic.h>

#include "csr.h"
#include "highlevel.h"
#include "hosts.h"
#include "ieee1394.h"
#include "ieee1394_core.h"
#include "ieee1394_hotplug.h"
#include "ieee1394_types.h"
#include "ieee1394_transactions.h"
#include "nodemgr.h"

static int ignore_drivers;
module_param(ignore_drivers, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(ignore_drivers, "Disable automatic probing for drivers.");

struct nodemgr_csr_info {
	struct hpsb_host *host;
	nodeid_t nodeid;
	unsigned int generation;

	kmemcheck_bitfield_begin(flags);
	unsigned int speed_unverified:1;
	kmemcheck_bitfield_end(flags);
};



static int nodemgr_check_speed(struct nodemgr_csr_info *ci, u64 addr,
			       quadlet_t *buffer)
{
	quadlet_t q;
	u8 i, *speed, old_speed, good_speed;
	int error;

	speed = &(ci->host->speed[NODEID_TO_NODE(ci->nodeid)]);
	old_speed = *speed;
	good_speed = IEEE1394_SPEED_MAX + 1;

	
	for (i = IEEE1394_SPEED_100; i <= old_speed; i++) {
		*speed = i;
		error = hpsb_read(ci->host, ci->nodeid, ci->generation, addr,
				  &q, 4);
		if (error)
			break;
		*buffer = q;
		good_speed = i;
	}
	if (good_speed <= IEEE1394_SPEED_MAX) {
		HPSB_DEBUG("Speed probe of node " NODE_BUS_FMT " yields %s",
			   NODE_BUS_ARGS(ci->host, ci->nodeid),
			   hpsb_speedto_str[good_speed]);
		*speed = good_speed;
		ci->speed_unverified = 0;
		return 0;
	}
	*speed = old_speed;
	return error;
}

static int nodemgr_bus_read(struct csr1212_csr *csr, u64 addr,
			    void *buffer, void *__ci)
{
	struct nodemgr_csr_info *ci = (struct nodemgr_csr_info*)__ci;
	int i, error;

	for (i = 1; ; i++) {
		error = hpsb_read(ci->host, ci->nodeid, ci->generation, addr,
				  buffer, 4);
		if (!error) {
			ci->speed_unverified = 0;
			break;
		}
		
		if (i == 3)
			break;

		
		if (ci->speed_unverified) {
			error = nodemgr_check_speed(ci, addr, buffer);
			if (!error)
				break;
		}
		if (msleep_interruptible(334))
			return -EINTR;
	}
	return error;
}

static struct csr1212_bus_ops nodemgr_csr_ops = {
	.bus_read =	nodemgr_bus_read,
};






struct host_info {
	struct hpsb_host *host;
	struct list_head list;
	struct task_struct *thread;
};

static int nodemgr_bus_match(struct device * dev, struct device_driver * drv);
static int nodemgr_uevent(struct device *dev, struct kobj_uevent_env *env);

struct bus_type ieee1394_bus_type = {
	.name		= "ieee1394",
	.match		= nodemgr_bus_match,
};

static void host_cls_release(struct device *dev)
{
	put_device(&container_of((dev), struct hpsb_host, host_dev)->device);
}

struct class hpsb_host_class = {
	.name		= "ieee1394_host",
	.dev_release	= host_cls_release,
};

static void ne_cls_release(struct device *dev)
{
	put_device(&container_of((dev), struct node_entry, node_dev)->device);
}

static struct class nodemgr_ne_class = {
	.name		= "ieee1394_node",
	.dev_release	= ne_cls_release,
};

static void ud_cls_release(struct device *dev)
{
	put_device(&container_of((dev), struct unit_directory, unit_dev)->device);
}


static struct class nodemgr_ud_class = {
	.name		= "ieee1394",
	.dev_release	= ud_cls_release,
	.dev_uevent	= nodemgr_uevent,
};

static struct hpsb_highlevel nodemgr_highlevel;


static void nodemgr_release_ud(struct device *dev)
{
	struct unit_directory *ud = container_of(dev, struct unit_directory, device);

	if (ud->vendor_name_kv)
		csr1212_release_keyval(ud->vendor_name_kv);
	if (ud->model_name_kv)
		csr1212_release_keyval(ud->model_name_kv);

	kfree(ud);
}

static void nodemgr_release_ne(struct device *dev)
{
	struct node_entry *ne = container_of(dev, struct node_entry, device);

	if (ne->vendor_name_kv)
		csr1212_release_keyval(ne->vendor_name_kv);

	kfree(ne);
}


static void nodemgr_release_host(struct device *dev)
{
	struct hpsb_host *host = container_of(dev, struct hpsb_host, device);

	csr1212_destroy_csr(host->csr.rom);

	kfree(host);
}

static int nodemgr_ud_platform_data;

static struct device nodemgr_dev_template_ud = {
	.bus		= &ieee1394_bus_type,
	.release	= nodemgr_release_ud,
	.platform_data	= &nodemgr_ud_platform_data,
};

static struct device nodemgr_dev_template_ne = {
	.bus		= &ieee1394_bus_type,
	.release	= nodemgr_release_ne,
};


static struct device_driver nodemgr_mid_layer_driver = {
	.bus		= &ieee1394_bus_type,
	.name		= "nodemgr",
	.owner		= THIS_MODULE,
};

struct device nodemgr_dev_template_host = {
	.bus		= &ieee1394_bus_type,
	.release	= nodemgr_release_host,
};


#define fw_attr(class, class_type, field, type, format_string)		\
static ssize_t fw_show_##class##_##field (struct device *dev, struct device_attribute *attr, char *buf)\
{									\
	class_type *class;						\
	class = container_of(dev, class_type, device);			\
	return sprintf(buf, format_string, (type)class->field);		\
}									\
static struct device_attribute dev_attr_##class##_##field = {		\
	.attr = {.name = __stringify(field), .mode = S_IRUGO },		\
	.show   = fw_show_##class##_##field,				\
};

#define fw_attr_td(class, class_type, td_kv)				\
static ssize_t fw_show_##class##_##td_kv (struct device *dev, struct device_attribute *attr, char *buf)\
{									\
	int len;							\
	class_type *class = container_of(dev, class_type, device);	\
	len = (class->td_kv->value.leaf.len - 2) * sizeof(quadlet_t);	\
	memcpy(buf,							\
	       CSR1212_TEXTUAL_DESCRIPTOR_LEAF_DATA(class->td_kv),	\
	       len);							\
	while (buf[len - 1] == '\0')					\
		len--;							\
	buf[len++] = '\n';						\
	buf[len] = '\0';						\
	return len;							\
}									\
static struct device_attribute dev_attr_##class##_##td_kv = {		\
	.attr = {.name = __stringify(td_kv), .mode = S_IRUGO },		\
	.show   = fw_show_##class##_##td_kv,				\
};


#define fw_drv_attr(field, type, format_string)			\
static ssize_t fw_drv_show_##field (struct device_driver *drv, char *buf) \
{								\
	struct hpsb_protocol_driver *driver;			\
	driver = container_of(drv, struct hpsb_protocol_driver, driver); \
	return sprintf(buf, format_string, (type)driver->field);\
}								\
static struct driver_attribute driver_attr_drv_##field = {	\
	.attr = {.name = __stringify(field), .mode = S_IRUGO },	\
	.show   = fw_drv_show_##field,				\
};


static ssize_t fw_show_ne_bus_options(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct node_entry *ne = container_of(dev, struct node_entry, device);

	return sprintf(buf, "IRMC(%d) CMC(%d) ISC(%d) BMC(%d) PMC(%d) GEN(%d) "
		       "LSPD(%d) MAX_REC(%d) MAX_ROM(%d) CYC_CLK_ACC(%d)\n",
		       ne->busopt.irmc,
		       ne->busopt.cmc, ne->busopt.isc, ne->busopt.bmc,
		       ne->busopt.pmc, ne->busopt.generation, ne->busopt.lnkspd,
		       ne->busopt.max_rec,
		       ne->busopt.max_rom,
		       ne->busopt.cyc_clk_acc);
}
static DEVICE_ATTR(bus_options,S_IRUGO,fw_show_ne_bus_options,NULL);


#ifdef HPSB_DEBUG_TLABELS
static ssize_t fw_show_ne_tlabels_free(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	struct node_entry *ne = container_of(dev, struct node_entry, device);
	unsigned long flags;
	unsigned long *tp = ne->host->tl_pool[NODEID_TO_NODE(ne->nodeid)].map;
	int tf;

	spin_lock_irqsave(&hpsb_tlabel_lock, flags);
	tf = 64 - bitmap_weight(tp, 64);
	spin_unlock_irqrestore(&hpsb_tlabel_lock, flags);

	return sprintf(buf, "%d\n", tf);
}
static DEVICE_ATTR(tlabels_free,S_IRUGO,fw_show_ne_tlabels_free,NULL);


static ssize_t fw_show_ne_tlabels_mask(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	struct node_entry *ne = container_of(dev, struct node_entry, device);
	unsigned long flags;
	unsigned long *tp = ne->host->tl_pool[NODEID_TO_NODE(ne->nodeid)].map;
	u64 tm;

	spin_lock_irqsave(&hpsb_tlabel_lock, flags);
#if (BITS_PER_LONG <= 32)
	tm = ((u64)tp[0] << 32) + tp[1];
#else
	tm = tp[0];
#endif
	spin_unlock_irqrestore(&hpsb_tlabel_lock, flags);

	return sprintf(buf, "0x%016llx\n", (unsigned long long)tm);
}
static DEVICE_ATTR(tlabels_mask, S_IRUGO, fw_show_ne_tlabels_mask, NULL);
#endif 


static ssize_t fw_set_ignore_driver(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct unit_directory *ud = container_of(dev, struct unit_directory, device);
	int state = simple_strtoul(buf, NULL, 10);

	if (state == 1) {
		ud->ignore_driver = 1;
		device_release_driver(dev);
	} else if (state == 0)
		ud->ignore_driver = 0;

	return count;
}
static ssize_t fw_get_ignore_driver(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct unit_directory *ud = container_of(dev, struct unit_directory, device);

	return sprintf(buf, "%d\n", ud->ignore_driver);
}
static DEVICE_ATTR(ignore_driver, S_IWUSR | S_IRUGO, fw_get_ignore_driver, fw_set_ignore_driver);


static ssize_t fw_set_rescan(struct bus_type *bus, const char *buf,
			     size_t count)
{
	int error = 0;

	if (simple_strtoul(buf, NULL, 10) == 1)
		error = bus_rescan_devices(&ieee1394_bus_type);
	return error ? error : count;
}
static ssize_t fw_get_rescan(struct bus_type *bus, char *buf)
{
	return sprintf(buf, "You can force a rescan of the bus for "
			"drivers by writing a 1 to this file\n");
}
static BUS_ATTR(rescan, S_IWUSR | S_IRUGO, fw_get_rescan, fw_set_rescan);


static ssize_t fw_set_ignore_drivers(struct bus_type *bus, const char *buf, size_t count)
{
	int state = simple_strtoul(buf, NULL, 10);

	if (state == 1)
		ignore_drivers = 1;
	else if (state == 0)
		ignore_drivers = 0;

	return count;
}
static ssize_t fw_get_ignore_drivers(struct bus_type *bus, char *buf)
{
	return sprintf(buf, "%d\n", ignore_drivers);
}
static BUS_ATTR(ignore_drivers, S_IWUSR | S_IRUGO, fw_get_ignore_drivers, fw_set_ignore_drivers);


struct bus_attribute *const fw_bus_attrs[] = {
	&bus_attr_rescan,
	&bus_attr_ignore_drivers,
	NULL
};


fw_attr(ne, struct node_entry, capabilities, unsigned int, "0x%06x\n")
fw_attr(ne, struct node_entry, nodeid, unsigned int, "0x%04x\n")

fw_attr(ne, struct node_entry, vendor_id, unsigned int, "0x%06x\n")
fw_attr_td(ne, struct node_entry, vendor_name_kv)

fw_attr(ne, struct node_entry, guid, unsigned long long, "0x%016Lx\n")
fw_attr(ne, struct node_entry, guid_vendor_id, unsigned int, "0x%06x\n")
fw_attr(ne, struct node_entry, in_limbo, int, "%d\n");

static struct device_attribute *const fw_ne_attrs[] = {
	&dev_attr_ne_guid,
	&dev_attr_ne_guid_vendor_id,
	&dev_attr_ne_capabilities,
	&dev_attr_ne_vendor_id,
	&dev_attr_ne_nodeid,
	&dev_attr_bus_options,
#ifdef HPSB_DEBUG_TLABELS
	&dev_attr_tlabels_free,
	&dev_attr_tlabels_mask,
#endif
};



fw_attr(ud, struct unit_directory, address, unsigned long long, "0x%016Lx\n")
fw_attr(ud, struct unit_directory, length, int, "%d\n")

fw_attr(ud, struct unit_directory, vendor_id, unsigned int, "0x%06x\n")
fw_attr(ud, struct unit_directory, model_id, unsigned int, "0x%06x\n")
fw_attr(ud, struct unit_directory, specifier_id, unsigned int, "0x%06x\n")
fw_attr(ud, struct unit_directory, version, unsigned int, "0x%06x\n")
fw_attr_td(ud, struct unit_directory, vendor_name_kv)
fw_attr_td(ud, struct unit_directory, model_name_kv)

static struct device_attribute *const fw_ud_attrs[] = {
	&dev_attr_ud_address,
	&dev_attr_ud_length,
	&dev_attr_ignore_driver,
};


fw_attr(host, struct hpsb_host, node_count, int, "%d\n")
fw_attr(host, struct hpsb_host, selfid_count, int, "%d\n")
fw_attr(host, struct hpsb_host, nodes_active, int, "%d\n")
fw_attr(host, struct hpsb_host, in_bus_reset, int, "%d\n")
fw_attr(host, struct hpsb_host, is_root, int, "%d\n")
fw_attr(host, struct hpsb_host, is_cycmst, int, "%d\n")
fw_attr(host, struct hpsb_host, is_irm, int, "%d\n")
fw_attr(host, struct hpsb_host, is_busmgr, int, "%d\n")

static struct device_attribute *const fw_host_attrs[] = {
	&dev_attr_host_node_count,
	&dev_attr_host_selfid_count,
	&dev_attr_host_nodes_active,
	&dev_attr_host_in_bus_reset,
	&dev_attr_host_is_root,
	&dev_attr_host_is_cycmst,
	&dev_attr_host_is_irm,
	&dev_attr_host_is_busmgr,
};


static ssize_t fw_show_drv_device_ids(struct device_driver *drv, char *buf)
{
	struct hpsb_protocol_driver *driver;
	const struct ieee1394_device_id *id;
	int length = 0;
	char *scratch = buf;

	driver = container_of(drv, struct hpsb_protocol_driver, driver);
	id = driver->id_table;
	if (!id)
		return 0;

	for (; id->match_flags != 0; id++) {
		int need_coma = 0;

		if (id->match_flags & IEEE1394_MATCH_VENDOR_ID) {
			length += sprintf(scratch, "vendor_id=0x%06x", id->vendor_id);
			scratch = buf + length;
			need_coma++;
		}

		if (id->match_flags & IEEE1394_MATCH_MODEL_ID) {
			length += sprintf(scratch, "%smodel_id=0x%06x",
					  need_coma++ ? "," : "",
					  id->model_id);
			scratch = buf + length;
		}

		if (id->match_flags & IEEE1394_MATCH_SPECIFIER_ID) {
			length += sprintf(scratch, "%sspecifier_id=0x%06x",
					  need_coma++ ? "," : "",
					  id->specifier_id);
			scratch = buf + length;
		}

		if (id->match_flags & IEEE1394_MATCH_VERSION) {
			length += sprintf(scratch, "%sversion=0x%06x",
					  need_coma++ ? "," : "",
					  id->version);
			scratch = buf + length;
		}

		if (need_coma) {
			*scratch++ = '\n';
			length++;
		}
	}

	return length;
}
static DRIVER_ATTR(device_ids,S_IRUGO,fw_show_drv_device_ids,NULL);


fw_drv_attr(name, const char *, "%s\n")

static struct driver_attribute *const fw_drv_attrs[] = {
	&driver_attr_drv_name,
	&driver_attr_device_ids,
};


static void nodemgr_create_drv_files(struct hpsb_protocol_driver *driver)
{
	struct device_driver *drv = &driver->driver;
	int i;

	for (i = 0; i < ARRAY_SIZE(fw_drv_attrs); i++)
		if (driver_create_file(drv, fw_drv_attrs[i]))
			goto fail;
	return;
fail:
	HPSB_ERR("Failed to add sysfs attribute");
}


static void nodemgr_remove_drv_files(struct hpsb_protocol_driver *driver)
{
	struct device_driver *drv = &driver->driver;
	int i;

	for (i = 0; i < ARRAY_SIZE(fw_drv_attrs); i++)
		driver_remove_file(drv, fw_drv_attrs[i]);
}


static void nodemgr_create_ne_dev_files(struct node_entry *ne)
{
	struct device *dev = &ne->device;
	int i;

	for (i = 0; i < ARRAY_SIZE(fw_ne_attrs); i++)
		if (device_create_file(dev, fw_ne_attrs[i]))
			goto fail;
	return;
fail:
	HPSB_ERR("Failed to add sysfs attribute");
}


static void nodemgr_create_host_dev_files(struct hpsb_host *host)
{
	struct device *dev = &host->device;
	int i;

	for (i = 0; i < ARRAY_SIZE(fw_host_attrs); i++)
		if (device_create_file(dev, fw_host_attrs[i]))
			goto fail;
	return;
fail:
	HPSB_ERR("Failed to add sysfs attribute");
}


static struct node_entry *find_entry_by_nodeid(struct hpsb_host *host,
					       nodeid_t nodeid);

static void nodemgr_update_host_dev_links(struct hpsb_host *host)
{
	struct device *dev = &host->device;
	struct node_entry *ne;

	sysfs_remove_link(&dev->kobj, "irm_id");
	sysfs_remove_link(&dev->kobj, "busmgr_id");
	sysfs_remove_link(&dev->kobj, "host_id");

	if ((ne = find_entry_by_nodeid(host, host->irm_id)) &&
	    sysfs_create_link(&dev->kobj, &ne->device.kobj, "irm_id"))
		goto fail;
	if ((ne = find_entry_by_nodeid(host, host->busmgr_id)) &&
	    sysfs_create_link(&dev->kobj, &ne->device.kobj, "busmgr_id"))
		goto fail;
	if ((ne = find_entry_by_nodeid(host, host->node_id)) &&
	    sysfs_create_link(&dev->kobj, &ne->device.kobj, "host_id"))
		goto fail;
	return;
fail:
	HPSB_ERR("Failed to update sysfs attributes for host %d", host->id);
}

static void nodemgr_create_ud_dev_files(struct unit_directory *ud)
{
	struct device *dev = &ud->device;
	int i;

	for (i = 0; i < ARRAY_SIZE(fw_ud_attrs); i++)
		if (device_create_file(dev, fw_ud_attrs[i]))
			goto fail;
	if (ud->flags & UNIT_DIRECTORY_SPECIFIER_ID)
		if (device_create_file(dev, &dev_attr_ud_specifier_id))
			goto fail;
	if (ud->flags & UNIT_DIRECTORY_VERSION)
		if (device_create_file(dev, &dev_attr_ud_version))
			goto fail;
	if (ud->flags & UNIT_DIRECTORY_VENDOR_ID) {
		if (device_create_file(dev, &dev_attr_ud_vendor_id))
			goto fail;
		if (ud->vendor_name_kv &&
		    device_create_file(dev, &dev_attr_ud_vendor_name_kv))
			goto fail;
	}
	if (ud->flags & UNIT_DIRECTORY_MODEL_ID) {
		if (device_create_file(dev, &dev_attr_ud_model_id))
			goto fail;
		if (ud->model_name_kv &&
		    device_create_file(dev, &dev_attr_ud_model_name_kv))
			goto fail;
	}
	return;
fail:
	HPSB_ERR("Failed to add sysfs attribute");
}


static int nodemgr_bus_match(struct device * dev, struct device_driver * drv)
{
	struct hpsb_protocol_driver *driver;
	struct unit_directory *ud;
	const struct ieee1394_device_id *id;

	
	if (dev->platform_data != &nodemgr_ud_platform_data)
		return 0;

	ud = container_of(dev, struct unit_directory, device);
	if (ud->ne->in_limbo || ud->ignore_driver)
		return 0;

	
	if (drv == &nodemgr_mid_layer_driver)
		return 0;

	driver = container_of(drv, struct hpsb_protocol_driver, driver);
	id = driver->id_table;
	if (!id)
		return 0;

	for (; id->match_flags != 0; id++) {
		if ((id->match_flags & IEEE1394_MATCH_VENDOR_ID) &&
		    id->vendor_id != ud->vendor_id)
			continue;

		if ((id->match_flags & IEEE1394_MATCH_MODEL_ID) &&
		    id->model_id != ud->model_id)
			continue;

		if ((id->match_flags & IEEE1394_MATCH_SPECIFIER_ID) &&
		    id->specifier_id != ud->specifier_id)
			continue;

		if ((id->match_flags & IEEE1394_MATCH_VERSION) &&
		    id->version != ud->version)
			continue;

		return 1;
	}

	return 0;
}


static DEFINE_MUTEX(nodemgr_serialize_remove_uds);

static int match_ne(struct device *dev, void *data)
{
	struct unit_directory *ud;
	struct node_entry *ne = data;

	ud = container_of(dev, struct unit_directory, unit_dev);
	return ud->ne == ne;
}

static void nodemgr_remove_uds(struct node_entry *ne)
{
	struct device *dev;
	struct unit_directory *ud;

	
	mutex_lock(&nodemgr_serialize_remove_uds);
	for (;;) {
		dev = class_find_device(&nodemgr_ud_class, NULL, ne, match_ne);
		if (!dev)
			break;
		ud = container_of(dev, struct unit_directory, unit_dev);
		put_device(dev);
		device_unregister(&ud->unit_dev);
		device_unregister(&ud->device);
	}
	mutex_unlock(&nodemgr_serialize_remove_uds);
}


static void nodemgr_remove_ne(struct node_entry *ne)
{
	struct device *dev;

	dev = get_device(&ne->device);
	if (!dev)
		return;

	HPSB_DEBUG("Node removed: ID:BUS[" NODE_BUS_FMT "]  GUID[%016Lx]",
		   NODE_BUS_ARGS(ne->host, ne->nodeid), (unsigned long long)ne->guid);
	nodemgr_remove_uds(ne);

	device_unregister(&ne->node_dev);
	device_unregister(dev);

	put_device(dev);
}

static int remove_host_dev(struct device *dev, void *data)
{
	if (dev->bus == &ieee1394_bus_type)
		nodemgr_remove_ne(container_of(dev, struct node_entry,
				  device));
	return 0;
}

static void nodemgr_remove_host_dev(struct device *dev)
{
	device_for_each_child(dev, NULL, remove_host_dev);
	sysfs_remove_link(&dev->kobj, "irm_id");
	sysfs_remove_link(&dev->kobj, "busmgr_id");
	sysfs_remove_link(&dev->kobj, "host_id");
}


static void nodemgr_update_bus_options(struct node_entry *ne)
{
#ifdef CONFIG_IEEE1394_VERBOSEDEBUG
	static const u16 mr[] = { 4, 64, 1024, 0};
#endif
	quadlet_t busoptions = be32_to_cpu(ne->csr->bus_info_data[2]);

	ne->busopt.irmc		= (busoptions >> 31) & 1;
	ne->busopt.cmc		= (busoptions >> 30) & 1;
	ne->busopt.isc		= (busoptions >> 29) & 1;
	ne->busopt.bmc		= (busoptions >> 28) & 1;
	ne->busopt.pmc		= (busoptions >> 27) & 1;
	ne->busopt.cyc_clk_acc	= (busoptions >> 16) & 0xff;
	ne->busopt.max_rec	= 1 << (((busoptions >> 12) & 0xf) + 1);
	ne->busopt.max_rom	= (busoptions >> 8) & 0x3;
	ne->busopt.generation	= (busoptions >> 4) & 0xf;
	ne->busopt.lnkspd	= busoptions & 0x7;

	HPSB_VERBOSE("NodeMgr: raw=0x%08x irmc=%d cmc=%d isc=%d bmc=%d pmc=%d "
		     "cyc_clk_acc=%d max_rec=%d max_rom=%d gen=%d lspd=%d",
		     busoptions, ne->busopt.irmc, ne->busopt.cmc,
		     ne->busopt.isc, ne->busopt.bmc, ne->busopt.pmc,
		     ne->busopt.cyc_clk_acc, ne->busopt.max_rec,
		     mr[ne->busopt.max_rom],
		     ne->busopt.generation, ne->busopt.lnkspd);
}


static struct node_entry *nodemgr_create_node(octlet_t guid,
				struct csr1212_csr *csr, struct hpsb_host *host,
				nodeid_t nodeid, unsigned int generation)
{
	struct node_entry *ne;

	ne = kzalloc(sizeof(*ne), GFP_KERNEL);
	if (!ne)
		goto fail_alloc;

	ne->host = host;
	ne->nodeid = nodeid;
	ne->generation = generation;
	ne->needs_probe = true;

	ne->guid = guid;
	ne->guid_vendor_id = (guid >> 40) & 0xffffff;
	ne->csr = csr;

	memcpy(&ne->device, &nodemgr_dev_template_ne,
	       sizeof(ne->device));
	ne->device.parent = &host->device;
	dev_set_name(&ne->device, "%016Lx", (unsigned long long)(ne->guid));

	ne->node_dev.parent = &ne->device;
	ne->node_dev.class = &nodemgr_ne_class;
	dev_set_name(&ne->node_dev, "%016Lx", (unsigned long long)(ne->guid));

	if (device_register(&ne->device))
		goto fail_devreg;
	if (device_register(&ne->node_dev))
		goto fail_classdevreg;
	get_device(&ne->device);

	nodemgr_create_ne_dev_files(ne);

	nodemgr_update_bus_options(ne);

	HPSB_DEBUG("%s added: ID:BUS[" NODE_BUS_FMT "]  GUID[%016Lx]",
		   (host->node_id == nodeid) ? "Host" : "Node",
		   NODE_BUS_ARGS(host, nodeid), (unsigned long long)guid);

	return ne;

fail_classdevreg:
	device_unregister(&ne->device);
fail_devreg:
	kfree(ne);
fail_alloc:
	HPSB_ERR("Failed to create node ID:BUS[" NODE_BUS_FMT "]  GUID[%016Lx]",
		 NODE_BUS_ARGS(host, nodeid), (unsigned long long)guid);

	return NULL;
}

static int match_ne_guid(struct device *dev, void *data)
{
	struct node_entry *ne;
	u64 *guid = data;

	ne = container_of(dev, struct node_entry, node_dev);
	return ne->guid == *guid;
}

static struct node_entry *find_entry_by_guid(u64 guid)
{
	struct device *dev;
	struct node_entry *ne;

	dev = class_find_device(&nodemgr_ne_class, NULL, &guid, match_ne_guid);
	if (!dev)
		return NULL;
	ne = container_of(dev, struct node_entry, node_dev);
	put_device(dev);

	return ne;
}

struct match_nodeid_parameter {
	struct hpsb_host *host;
	nodeid_t nodeid;
};

static int match_ne_nodeid(struct device *dev, void *data)
{
	int found = 0;
	struct node_entry *ne;
	struct match_nodeid_parameter *p = data;

	if (!dev)
		goto ret;
	ne = container_of(dev, struct node_entry, node_dev);
	if (ne->host == p->host && ne->nodeid == p->nodeid)
		found = 1;
ret:
	return found;
}

static struct node_entry *find_entry_by_nodeid(struct hpsb_host *host,
					       nodeid_t nodeid)
{
	struct device *dev;
	struct node_entry *ne;
	struct match_nodeid_parameter p;

	p.host = host;
	p.nodeid = nodeid;

	dev = class_find_device(&nodemgr_ne_class, NULL, &p, match_ne_nodeid);
	if (!dev)
		return NULL;
	ne = container_of(dev, struct node_entry, node_dev);
	put_device(dev);

	return ne;
}


static void nodemgr_register_device(struct node_entry *ne, 
	struct unit_directory *ud, struct device *parent)
{
	memcpy(&ud->device, &nodemgr_dev_template_ud,
	       sizeof(ud->device));

	ud->device.parent = parent;

	dev_set_name(&ud->device, "%s-%u", dev_name(&ne->device), ud->id);

	ud->unit_dev.parent = &ud->device;
	ud->unit_dev.class = &nodemgr_ud_class;
	dev_set_name(&ud->unit_dev, "%s-%u", dev_name(&ne->device), ud->id);

	if (device_register(&ud->device))
		goto fail_devreg;
	if (device_register(&ud->unit_dev))
		goto fail_classdevreg;
	get_device(&ud->device);

	nodemgr_create_ud_dev_files(ud);

	return;

fail_classdevreg:
	device_unregister(&ud->device);
fail_devreg:
	HPSB_ERR("Failed to create unit %s", dev_name(&ud->device));
}	



static struct unit_directory *nodemgr_process_unit_directory
	(struct node_entry *ne, struct csr1212_keyval *ud_kv,
	 unsigned int *id, struct unit_directory *parent)
{
	struct unit_directory *ud;
	struct unit_directory *ud_child = NULL;
	struct csr1212_dentry *dentry;
	struct csr1212_keyval *kv;
	u8 last_key_id = 0;

	ud = kzalloc(sizeof(*ud), GFP_KERNEL);
	if (!ud)
		goto unit_directory_error;

	ud->ne = ne;
	ud->ignore_driver = ignore_drivers;
	ud->address = ud_kv->offset + CSR1212_REGISTER_SPACE_BASE;
	ud->directory_id = ud->address & 0xffffff;
	ud->ud_kv = ud_kv;
	ud->id = (*id)++;

	
	ud->vendor_id = ne->vendor_id;

	csr1212_for_each_dir_entry(ne->csr, kv, ud_kv, dentry) {
		switch (kv->key.id) {
		case CSR1212_KV_ID_VENDOR:
			if (kv->key.type == CSR1212_KV_TYPE_IMMEDIATE) {
				ud->vendor_id = kv->value.immediate;
				ud->flags |= UNIT_DIRECTORY_VENDOR_ID;
			}
			break;

		case CSR1212_KV_ID_MODEL:
			ud->model_id = kv->value.immediate;
			ud->flags |= UNIT_DIRECTORY_MODEL_ID;
			break;

		case CSR1212_KV_ID_SPECIFIER_ID:
			ud->specifier_id = kv->value.immediate;
			ud->flags |= UNIT_DIRECTORY_SPECIFIER_ID;
			break;

		case CSR1212_KV_ID_VERSION:
			ud->version = kv->value.immediate;
			ud->flags |= UNIT_DIRECTORY_VERSION;
			break;

		case CSR1212_KV_ID_DESCRIPTOR:
			if (kv->key.type == CSR1212_KV_TYPE_LEAF &&
			    CSR1212_DESCRIPTOR_LEAF_TYPE(kv) == 0 &&
			    CSR1212_DESCRIPTOR_LEAF_SPECIFIER_ID(kv) == 0 &&
			    CSR1212_TEXTUAL_DESCRIPTOR_LEAF_WIDTH(kv) == 0 &&
			    CSR1212_TEXTUAL_DESCRIPTOR_LEAF_CHAR_SET(kv) == 0 &&
			    CSR1212_TEXTUAL_DESCRIPTOR_LEAF_LANGUAGE(kv) == 0) {
				switch (last_key_id) {
				case CSR1212_KV_ID_VENDOR:
					csr1212_keep_keyval(kv);
					ud->vendor_name_kv = kv;
					break;

				case CSR1212_KV_ID_MODEL:
					csr1212_keep_keyval(kv);
					ud->model_name_kv = kv;
					break;

				}
			} 
			break;

		case CSR1212_KV_ID_DEPENDENT_INFO:
			
			if (kv->key.type == CSR1212_KV_TYPE_IMMEDIATE) {
				if (ud->flags & UNIT_DIRECTORY_HAS_LUN) {
					ud_child = kmemdup(ud, sizeof(*ud_child), GFP_KERNEL);
					if (!ud_child)
						goto unit_directory_error;
					nodemgr_register_device(ne, ud_child, &ne->device);
					ud_child = NULL;
					
					ud->id = (*id)++;
				}
				ud->lun = kv->value.immediate;
				ud->flags |= UNIT_DIRECTORY_HAS_LUN;

			
			} else if (kv->key.type == CSR1212_KV_TYPE_DIRECTORY) {
				
				
				
				ud->flags |= UNIT_DIRECTORY_HAS_LUN_DIRECTORY;
				if (ud->device.bus != &ieee1394_bus_type)
					nodemgr_register_device(ne, ud, &ne->device);
				
				
				ud_child = nodemgr_process_unit_directory(ne, kv, id, ud);

				if (ud_child == NULL)
					break;
				
				
				if ((ud->flags & UNIT_DIRECTORY_MODEL_ID) &&
				    !(ud_child->flags & UNIT_DIRECTORY_MODEL_ID))
				{
					ud_child->flags |=  UNIT_DIRECTORY_MODEL_ID;
					ud_child->model_id = ud->model_id;
				}
				if ((ud->flags & UNIT_DIRECTORY_SPECIFIER_ID) &&
				    !(ud_child->flags & UNIT_DIRECTORY_SPECIFIER_ID))
				{
					ud_child->flags |=  UNIT_DIRECTORY_SPECIFIER_ID;
					ud_child->specifier_id = ud->specifier_id;
				}
				if ((ud->flags & UNIT_DIRECTORY_VERSION) &&
				    !(ud_child->flags & UNIT_DIRECTORY_VERSION))
				{
					ud_child->flags |=  UNIT_DIRECTORY_VERSION;
					ud_child->version = ud->version;
				}
				
				
				ud_child->flags |= UNIT_DIRECTORY_LUN_DIRECTORY;
				nodemgr_register_device(ne, ud_child, &ud->device);
			}

			break;

		case CSR1212_KV_ID_DIRECTORY_ID:
			ud->directory_id = kv->value.immediate;
			break;

		default:
			break;
		}
		last_key_id = kv->key.id;
	}
	
	
	if (!parent && ud->device.bus != &ieee1394_bus_type)
		nodemgr_register_device(ne, ud, &ne->device);

	return ud;

unit_directory_error:
	kfree(ud);
	return NULL;
}


static void nodemgr_process_root_directory(struct node_entry *ne)
{
	unsigned int ud_id = 0;
	struct csr1212_dentry *dentry;
	struct csr1212_keyval *kv, *vendor_name_kv = NULL;
	u8 last_key_id = 0;

	ne->needs_probe = false;

	csr1212_for_each_dir_entry(ne->csr, kv, ne->csr->root_kv, dentry) {
		switch (kv->key.id) {
		case CSR1212_KV_ID_VENDOR:
			ne->vendor_id = kv->value.immediate;
			break;

		case CSR1212_KV_ID_NODE_CAPABILITIES:
			ne->capabilities = kv->value.immediate;
			break;

		case CSR1212_KV_ID_UNIT:
			nodemgr_process_unit_directory(ne, kv, &ud_id, NULL);
			break;

		case CSR1212_KV_ID_DESCRIPTOR:
			if (last_key_id == CSR1212_KV_ID_VENDOR) {
				if (kv->key.type == CSR1212_KV_TYPE_LEAF &&
				    CSR1212_DESCRIPTOR_LEAF_TYPE(kv) == 0 &&
				    CSR1212_DESCRIPTOR_LEAF_SPECIFIER_ID(kv) == 0 &&
				    CSR1212_TEXTUAL_DESCRIPTOR_LEAF_WIDTH(kv) == 0 &&
				    CSR1212_TEXTUAL_DESCRIPTOR_LEAF_CHAR_SET(kv) == 0 &&
				    CSR1212_TEXTUAL_DESCRIPTOR_LEAF_LANGUAGE(kv) == 0) {
					csr1212_keep_keyval(kv);
					vendor_name_kv = kv;
				}
			}
			break;
		}
		last_key_id = kv->key.id;
	}

	if (ne->vendor_name_kv) {
		kv = ne->vendor_name_kv;
		ne->vendor_name_kv = vendor_name_kv;
		csr1212_release_keyval(kv);
	} else if (vendor_name_kv) {
		ne->vendor_name_kv = vendor_name_kv;
		if (device_create_file(&ne->device,
				       &dev_attr_ne_vendor_name_kv) != 0)
			HPSB_ERR("Failed to add sysfs attribute");
	}
}

#ifdef CONFIG_HOTPLUG

static int nodemgr_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	struct unit_directory *ud;
	int retval = 0;
	
	char buf[8 + 1 + 3 + 8 + 2 + 8 + 2 + 8 + 3 + 8 + 1];

	if (!dev)
		return -ENODEV;

	ud = container_of(dev, struct unit_directory, unit_dev);

	if (ud->ne->in_limbo || ud->ignore_driver)
		return -ENODEV;

#define PUT_ENVP(fmt,val) 					\
do {								\
	retval = add_uevent_var(env, fmt, val);		\
	if (retval)						\
		return retval;					\
} while (0)

	PUT_ENVP("VENDOR_ID=%06x", ud->vendor_id);
	PUT_ENVP("MODEL_ID=%06x", ud->model_id);
	PUT_ENVP("GUID=%016Lx", (unsigned long long)ud->ne->guid);
	PUT_ENVP("SPECIFIER_ID=%06x", ud->specifier_id);
	PUT_ENVP("VERSION=%06x", ud->version);
	snprintf(buf, sizeof(buf), "ieee1394:ven%08Xmo%08Xsp%08Xver%08X",
			ud->vendor_id,
			ud->model_id,
			ud->specifier_id,
			ud->version);
	PUT_ENVP("MODALIAS=%s", buf);

#undef PUT_ENVP

	return 0;
}

#else

static int nodemgr_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	return -ENODEV;
}

#endif 


int __hpsb_register_protocol(struct hpsb_protocol_driver *drv,
			     struct module *owner)
{
	int error;

	drv->driver.bus = &ieee1394_bus_type;
	drv->driver.owner = owner;
	drv->driver.name = drv->name;

	
	error = driver_register(&drv->driver);
	if (!error)
		nodemgr_create_drv_files(drv);
	return error;
}

void hpsb_unregister_protocol(struct hpsb_protocol_driver *driver)
{
	nodemgr_remove_drv_files(driver);
	
	driver_unregister(&driver->driver);
}



static void nodemgr_update_node(struct node_entry *ne, struct csr1212_csr *csr,
				nodeid_t nodeid, unsigned int generation)
{
	if (ne->nodeid != nodeid) {
		HPSB_DEBUG("Node changed: " NODE_BUS_FMT " -> " NODE_BUS_FMT,
			   NODE_BUS_ARGS(ne->host, ne->nodeid),
			   NODE_BUS_ARGS(ne->host, nodeid));
		ne->nodeid = nodeid;
	}

	if (ne->busopt.generation != ((be32_to_cpu(csr->bus_info_data[2]) >> 4) & 0xf)) {
		kfree(ne->csr->private);
		csr1212_destroy_csr(ne->csr);
		ne->csr = csr;

		
		nodemgr_remove_uds(ne);

		nodemgr_update_bus_options(ne);

		
		ne->needs_probe = true;
	} else {
		
		struct nodemgr_csr_info *ci = ne->csr->private;
		ci->generation = generation;
		
		kfree(csr->private);
		csr1212_destroy_csr(csr);
	}

	
	smp_wmb();
	ne->generation = generation;

	if (ne->in_limbo) {
		device_remove_file(&ne->device, &dev_attr_ne_in_limbo);
		ne->in_limbo = false;

		HPSB_DEBUG("Node reactivated: "
			   "ID:BUS[" NODE_BUS_FMT "]  GUID[%016Lx]",
			   NODE_BUS_ARGS(ne->host, ne->nodeid),
			   (unsigned long long)ne->guid);
	}
}

static void nodemgr_node_scan_one(struct hpsb_host *host,
				  nodeid_t nodeid, int generation)
{
	struct node_entry *ne;
	octlet_t guid;
	struct csr1212_csr *csr;
	struct nodemgr_csr_info *ci;
	u8 *speed;

	ci = kmalloc(sizeof(*ci), GFP_KERNEL);
	kmemcheck_annotate_bitfield(ci, flags);
	if (!ci)
		return;

	ci->host = host;
	ci->nodeid = nodeid;
	ci->generation = generation;

	
	speed = &(host->speed[NODEID_TO_NODE(nodeid)]);
	if (*speed > host->csr.lnk_spd)
		*speed = host->csr.lnk_spd;
	ci->speed_unverified = *speed > IEEE1394_SPEED_100;

	

	csr = csr1212_create_csr(&nodemgr_csr_ops, 5 * sizeof(quadlet_t), ci);
	if (!csr || csr1212_parse_csr(csr) != CSR1212_SUCCESS) {
		HPSB_ERR("Error parsing configrom for node " NODE_BUS_FMT,
			 NODE_BUS_ARGS(host, nodeid));
		if (csr)
			csr1212_destroy_csr(csr);
		kfree(ci);
		return;
	}

	if (csr->bus_info_data[1] != IEEE1394_BUSID_MAGIC) {
		
		HPSB_WARN("Node " NODE_BUS_FMT " has invalid busID magic [0x%08x]",
			  NODE_BUS_ARGS(host, nodeid), csr->bus_info_data[1]);
	}

	guid = ((u64)be32_to_cpu(csr->bus_info_data[3]) << 32) | be32_to_cpu(csr->bus_info_data[4]);
	ne = find_entry_by_guid(guid);

	if (ne && ne->host != host && ne->in_limbo) {
		
		nodemgr_remove_ne(ne);
		ne = NULL;
	}

	if (!ne)
		nodemgr_create_node(guid, csr, host, nodeid, generation);
	else
		nodemgr_update_node(ne, csr, nodeid, generation);
}


static void nodemgr_node_scan(struct hpsb_host *host, int generation)
{
	int count;
	struct selfid *sid = (struct selfid *)host->topology_map;
	nodeid_t nodeid = LOCAL_BUS;

	
	for (count = host->selfid_count; count; count--, sid++) {
		if (sid->extended)
			continue;

		if (!sid->link_active) {
			nodeid++;
			continue;
		}
		nodemgr_node_scan_one(host, nodeid++, generation);
	}
}

static void nodemgr_pause_ne(struct node_entry *ne)
{
	HPSB_DEBUG("Node paused: ID:BUS[" NODE_BUS_FMT "]  GUID[%016Lx]",
		   NODE_BUS_ARGS(ne->host, ne->nodeid),
		   (unsigned long long)ne->guid);

	ne->in_limbo = true;
	WARN_ON(device_create_file(&ne->device, &dev_attr_ne_in_limbo));
}

static int update_pdrv(struct device *dev, void *data)
{
	struct unit_directory *ud;
	struct device_driver *drv;
	struct hpsb_protocol_driver *pdrv;
	struct node_entry *ne = data;
	int error;

	ud = container_of(dev, struct unit_directory, unit_dev);
	if (ud->ne == ne) {
		drv = get_driver(ud->device.driver);
		if (drv) {
			error = 0;
			pdrv = container_of(drv, struct hpsb_protocol_driver,
					    driver);
			if (pdrv->update) {
				down(&ud->device.sem);
				error = pdrv->update(ud);
				up(&ud->device.sem);
			}
			if (error)
				device_release_driver(&ud->device);
			put_driver(drv);
		}
	}

	return 0;
}

static void nodemgr_update_pdrv(struct node_entry *ne)
{
	class_for_each_device(&nodemgr_ud_class, NULL, ne, update_pdrv);
}


static void nodemgr_irm_write_bc(struct node_entry *ne, int generation)
{
	const u64 bc_addr = (CSR_REGISTER_BASE | CSR_BROADCAST_CHANNEL);
	quadlet_t bc_remote, bc_local;
	int error;

	if (!ne->host->is_irm || ne->generation != generation ||
	    ne->nodeid == ne->host->node_id)
		return;

	bc_local = cpu_to_be32(ne->host->csr.broadcast_channel);

	
	error = hpsb_read(ne->host, ne->nodeid, generation, bc_addr, &bc_remote,
			  sizeof(bc_remote));
	if (!error && bc_remote & cpu_to_be32(0x80000000) &&
	    bc_remote != bc_local)
		hpsb_node_write(ne, bc_addr, &bc_local, sizeof(bc_local));
}


static void nodemgr_probe_ne(struct hpsb_host *host, struct node_entry *ne,
			     int generation)
{
	struct device *dev;

	if (ne->host != host || ne->in_limbo)
		return;

	dev = get_device(&ne->device);
	if (!dev)
		return;

	nodemgr_irm_write_bc(ne, generation);

	
	if (ne->needs_probe)
		nodemgr_process_root_directory(ne);
	else if (ne->generation == generation)
		nodemgr_update_pdrv(ne);
	else
		nodemgr_pause_ne(ne);

	put_device(dev);
}

struct node_probe_parameter {
	struct hpsb_host *host;
	int generation;
	bool probe_now;
};

static int node_probe(struct device *dev, void *data)
{
	struct node_probe_parameter *p = data;
	struct node_entry *ne;

	if (p->generation != get_hpsb_generation(p->host))
		return -EAGAIN;

	ne = container_of(dev, struct node_entry, node_dev);
	if (ne->needs_probe == p->probe_now)
		nodemgr_probe_ne(p->host, ne, p->generation);
	return 0;
}

static int nodemgr_node_probe(struct hpsb_host *host, int generation)
{
	struct node_probe_parameter p;

	p.host = host;
	p.generation = generation;
	
	p.probe_now = false;
	if (class_for_each_device(&nodemgr_ne_class, NULL, &p, node_probe) != 0)
		return 0;

	p.probe_now = true;
	if (class_for_each_device(&nodemgr_ne_class, NULL, &p, node_probe) != 0)
		return 0;
	
	if (bus_rescan_devices(&ieee1394_bus_type) != 0)
		HPSB_DEBUG("bus_rescan_devices had an error");

	return 1;
}

static int remove_nodes_in_limbo(struct device *dev, void *data)
{
	struct node_entry *ne;

	if (dev->bus != &ieee1394_bus_type)
		return 0;

	ne = container_of(dev, struct node_entry, device);
	if (ne->in_limbo)
		nodemgr_remove_ne(ne);

	return 0;
}

static void nodemgr_remove_nodes_in_limbo(struct hpsb_host *host)
{
	device_for_each_child(&host->device, NULL, remove_nodes_in_limbo);
}

static int nodemgr_send_resume_packet(struct hpsb_host *host)
{
	struct hpsb_packet *packet;
	int error = -ENOMEM;

	packet = hpsb_make_phypacket(host,
			EXTPHYPACKET_TYPE_RESUME |
			NODEID_TO_NODE(host->node_id) << PHYPACKET_PORT_SHIFT);
	if (packet) {
		packet->no_waiter = 1;
		packet->generation = get_hpsb_generation(host);
		error = hpsb_send_packet(packet);
	}
	if (error)
		HPSB_WARN("fw-host%d: Failed to broadcast resume packet",
			  host->id);
	return error;
}


static int nodemgr_do_irm_duties(struct hpsb_host *host, int cycles)
{
	quadlet_t bc;

	
	if (!host->is_irm || host->irm_id == (nodeid_t)-1)
		return 1;

	
	host->csr.broadcast_channel |= 0x40000000;

	
	if (host->busmgr_id == 0xffff && host->node_count > 1)
	{
		u16 root_node = host->node_count - 1;

		
		if (host->is_cycmst ||
		    (!hpsb_read(host, LOCAL_BUS | root_node, get_hpsb_generation(host),
				(CSR_REGISTER_BASE + CSR_CONFIG_ROM + 2 * sizeof(quadlet_t)),
				&bc, sizeof(quadlet_t)) &&
		     be32_to_cpu(bc) & 1 << CSR_CMC_SHIFT))
			hpsb_send_phy_config(host, root_node, -1);
		else {
			HPSB_DEBUG("The root node is not cycle master capable; "
				   "selecting a new root node and resetting...");

			if (cycles >= 5) {
				
				HPSB_DEBUG("Stopping reset loop for IRM sanity");
				return 1;
			}

			hpsb_send_phy_config(host, NODEID_TO_NODE(host->node_id), -1);
			hpsb_reset_bus(host, LONG_RESET_FORCE_ROOT);

			return 0;
		}
	}

	
	if (!host->resume_packet_sent && !nodemgr_send_resume_packet(host))
		host->resume_packet_sent = 1;

	return 1;
}


static int nodemgr_check_irm_capability(struct hpsb_host *host, int cycles)
{
	quadlet_t bc;
	int status;

	if (hpsb_disable_irm || host->is_irm)
		return 1;

	status = hpsb_read(host, LOCAL_BUS | (host->irm_id),
			   get_hpsb_generation(host),
			   (CSR_REGISTER_BASE | CSR_BROADCAST_CHANNEL),
			   &bc, sizeof(quadlet_t));

	if (status < 0 || !(be32_to_cpu(bc) & 0x80000000)) {
		
		HPSB_DEBUG("Current remote IRM is not 1394a-2000 compliant, resetting...");

		if (cycles >= 5) {
			
			HPSB_DEBUG("Stopping reset loop for IRM sanity");
			return 1;
		}

		hpsb_send_phy_config(host, NODEID_TO_NODE(host->node_id), -1);
		hpsb_reset_bus(host, LONG_RESET_FORCE_ROOT);

		return 0;
	}

	return 1;
}

static int nodemgr_host_thread(void *data)
{
	struct hpsb_host *host = data;
	unsigned int g, generation = 0;
	int i, reset_cycles = 0;

	set_freezable();
	
	nodemgr_create_host_dev_files(host);

	for (;;) {
		
		set_current_state(TASK_INTERRUPTIBLE);
		if (get_hpsb_generation(host) == generation &&
		    !kthread_should_stop())
			schedule();
		__set_current_state(TASK_RUNNING);

		
		if (try_to_freeze())
			continue;
		if (kthread_should_stop())
			goto exit;

		
		g = get_hpsb_generation(host);
		for (i = 0; i < 4 ; i++) {
			msleep_interruptible(63);
			try_to_freeze();
			if (kthread_should_stop())
				goto exit;

			
			generation = get_hpsb_generation(host);

			
			if (generation != g)
				g = generation, i = 0;
		}

		if (!nodemgr_check_irm_capability(host, reset_cycles) ||
		    !nodemgr_do_irm_duties(host, reset_cycles)) {
			reset_cycles++;
			continue;
		}
		reset_cycles = 0;

		
		nodemgr_node_scan(host, generation);

		
		if (!nodemgr_node_probe(host, generation))
			continue;

		
		nodemgr_update_host_dev_links(host);

		
		for (i = 3000/200; i; i--) {
			msleep_interruptible(200);
			try_to_freeze();
			if (kthread_should_stop())
				goto exit;

			if (generation != get_hpsb_generation(host))
				break;
		}
		
		if (!i)
			nodemgr_remove_nodes_in_limbo(host);
	}
exit:
	HPSB_VERBOSE("NodeMgr: Exiting thread");
	return 0;
}

struct per_host_parameter {
	void *data;
	int (*cb)(struct hpsb_host *, void *);
};

static int per_host(struct device *dev, void *data)
{
	struct hpsb_host *host;
	struct per_host_parameter *p = data;

	host = container_of(dev, struct hpsb_host, host_dev);
	return p->cb(host, p->data);
}


int nodemgr_for_each_host(void *data, int (*cb)(struct hpsb_host *, void *))
{
	struct per_host_parameter p;

	p.cb = cb;
	p.data = data;
	return class_for_each_device(&hpsb_host_class, NULL, &p, per_host);
}




void hpsb_node_fill_packet(struct node_entry *ne, struct hpsb_packet *packet)
{
	packet->host = ne->host;
	packet->generation = ne->generation;
	smp_rmb();
	packet->node_id = ne->nodeid;
}

int hpsb_node_write(struct node_entry *ne, u64 addr,
		    quadlet_t *buffer, size_t length)
{
	unsigned int generation = ne->generation;

	smp_rmb();
	return hpsb_write(ne->host, ne->nodeid, generation,
			  addr, buffer, length);
}

static void nodemgr_add_host(struct hpsb_host *host)
{
	struct host_info *hi;

	hi = hpsb_create_hostinfo(&nodemgr_highlevel, host, sizeof(*hi));
	if (!hi) {
		HPSB_ERR("NodeMgr: out of memory in add host");
		return;
	}
	hi->host = host;
	hi->thread = kthread_run(nodemgr_host_thread, host, "knodemgrd_%d",
				 host->id);
	if (IS_ERR(hi->thread)) {
		HPSB_ERR("NodeMgr: cannot start thread for host %d", host->id);
		hpsb_destroy_hostinfo(&nodemgr_highlevel, host);
	}
}

static void nodemgr_host_reset(struct hpsb_host *host)
{
	struct host_info *hi = hpsb_get_hostinfo(&nodemgr_highlevel, host);

	if (hi) {
		HPSB_VERBOSE("NodeMgr: Processing reset for host %d", host->id);
		wake_up_process(hi->thread);
	}
}

static void nodemgr_remove_host(struct hpsb_host *host)
{
	struct host_info *hi = hpsb_get_hostinfo(&nodemgr_highlevel, host);

	if (hi) {
		kthread_stop(hi->thread);
		nodemgr_remove_host_dev(&host->device);
	}
}

static struct hpsb_highlevel nodemgr_highlevel = {
	.name =		"Node manager",
	.add_host =	nodemgr_add_host,
	.host_reset =	nodemgr_host_reset,
	.remove_host =	nodemgr_remove_host,
};

int init_ieee1394_nodemgr(void)
{
	int error;

	error = class_register(&nodemgr_ne_class);
	if (error)
		goto fail_ne;
	error = class_register(&nodemgr_ud_class);
	if (error)
		goto fail_ud;
	error = driver_register(&nodemgr_mid_layer_driver);
	if (error)
		goto fail_ml;
	
	nodemgr_dev_template_host.driver = &nodemgr_mid_layer_driver;

	hpsb_register_highlevel(&nodemgr_highlevel);
	return 0;

fail_ml:
	class_unregister(&nodemgr_ud_class);
fail_ud:
	class_unregister(&nodemgr_ne_class);
fail_ne:
	return error;
}

void cleanup_ieee1394_nodemgr(void)
{
	hpsb_unregister_highlevel(&nodemgr_highlevel);
	driver_unregister(&nodemgr_mid_layer_driver);
	class_unregister(&nodemgr_ud_class);
	class_unregister(&nodemgr_ne_class);
}
