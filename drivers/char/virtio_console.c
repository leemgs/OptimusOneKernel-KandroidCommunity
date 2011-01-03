



#include <linux/err.h>
#include <linux/init.h>
#include <linux/virtio.h>
#include <linux/virtio_console.h>
#include "hvc_console.h"


static struct virtqueue *in_vq, *out_vq;
static struct virtio_device *vdev;


static unsigned int in_len;
static char *in, *inbuf;


static struct hv_ops virtio_cons;


static struct hvc_struct *hvc;


static int put_chars(u32 vtermno, const char *buf, int count)
{
	struct scatterlist sg[1];
	unsigned int len;

	
	sg_init_one(sg, buf, count);

	
	if (out_vq->vq_ops->add_buf(out_vq, sg, 1, 0, (void *)1) >= 0) {
		
		out_vq->vq_ops->kick(out_vq);
		
		while (!out_vq->vq_ops->get_buf(out_vq, &len))
			cpu_relax();
	}

	
	return count;
}


static void add_inbuf(void)
{
	struct scatterlist sg[1];
	sg_init_one(sg, inbuf, PAGE_SIZE);

	
	if (in_vq->vq_ops->add_buf(in_vq, sg, 0, 1, inbuf) < 0)
		BUG();
	in_vq->vq_ops->kick(in_vq);
}


static int get_chars(u32 vtermno, char *buf, int count)
{
	
	BUG_ON(!in_vq);

	
	if (!in_len) {
		in = in_vq->vq_ops->get_buf(in_vq, &in_len);
		if (!in)
			return 0;
	}

	
	if (in_len < count)
		count = in_len;

	
	memcpy(buf, in, count);
	in += count;
	in_len -= count;

	
	if (in_len == 0)
		add_inbuf();

	return count;
}



int __init virtio_cons_early_init(int (*put_chars)(u32, const char *, int))
{
	virtio_cons.put_chars = put_chars;
	return hvc_instantiate(0, 0, &virtio_cons);
}


static void virtcons_apply_config(struct virtio_device *dev)
{
	struct winsize ws;

	if (virtio_has_feature(dev, VIRTIO_CONSOLE_F_SIZE)) {
		dev->config->get(dev,
				 offsetof(struct virtio_console_config, cols),
				 &ws.ws_col, sizeof(u16));
		dev->config->get(dev,
				 offsetof(struct virtio_console_config, rows),
				 &ws.ws_row, sizeof(u16));
		hvc_resize(hvc, ws);
	}
}


static int notifier_add_vio(struct hvc_struct *hp, int data)
{
	hp->irq_requested = 1;
	virtcons_apply_config(vdev);

	return 0;
}

static void notifier_del_vio(struct hvc_struct *hp, int data)
{
	hp->irq_requested = 0;
}

static void hvc_handle_input(struct virtqueue *vq)
{
	if (hvc_poll(hvc))
		hvc_kick();
}


static int __devinit virtcons_probe(struct virtio_device *dev)
{
	vq_callback_t *callbacks[] = { hvc_handle_input, NULL};
	const char *names[] = { "input", "output" };
	struct virtqueue *vqs[2];
	int err;

	vdev = dev;

	
	inbuf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!inbuf) {
		err = -ENOMEM;
		goto fail;
	}

	
	
	err = vdev->config->find_vqs(vdev, 2, vqs, callbacks, names);
	if (err)
		goto free;

	in_vq = vqs[0];
	out_vq = vqs[1];

	
	virtio_cons.get_chars = get_chars;
	virtio_cons.put_chars = put_chars;
	virtio_cons.notifier_add = notifier_add_vio;
	virtio_cons.notifier_del = notifier_del_vio;
	virtio_cons.notifier_hangup = notifier_del_vio;

	
	hvc = hvc_alloc(0, 0, &virtio_cons, PAGE_SIZE);
	if (IS_ERR(hvc)) {
		err = PTR_ERR(hvc);
		goto free_vqs;
	}

	
	add_inbuf();
	return 0;

free_vqs:
	vdev->config->del_vqs(vdev);
free:
	kfree(inbuf);
fail:
	return err;
}

static struct virtio_device_id id_table[] = {
	{ VIRTIO_ID_CONSOLE, VIRTIO_DEV_ANY_ID },
	{ 0 },
};

static unsigned int features[] = {
	VIRTIO_CONSOLE_F_SIZE,
};

static struct virtio_driver virtio_console = {
	.feature_table = features,
	.feature_table_size = ARRAY_SIZE(features),
	.driver.name =	KBUILD_MODNAME,
	.driver.owner =	THIS_MODULE,
	.id_table =	id_table,
	.probe =	virtcons_probe,
	.config_changed = virtcons_apply_config,
};

static int __init init(void)
{
	return register_virtio_driver(&virtio_console);
}
module_init(init);

MODULE_DEVICE_TABLE(virtio, id_table);
MODULE_DESCRIPTION("Virtio console driver");
MODULE_LICENSE("GPL");
