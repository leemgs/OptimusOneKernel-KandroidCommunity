

#include <linux/module.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/poll.h>

#include <linux/skbuff.h>
#include <linux/miscdevice.h>

#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>

#define VERSION "1.3"

static int minor = MISC_DYNAMIC_MINOR;

struct vhci_data {
	struct hci_dev *hdev;

	unsigned long flags;

	wait_queue_head_t read_wait;
	struct sk_buff_head readq;
};

static int vhci_open_dev(struct hci_dev *hdev)
{
	set_bit(HCI_RUNNING, &hdev->flags);

	return 0;
}

static int vhci_close_dev(struct hci_dev *hdev)
{
	struct vhci_data *data = hdev->driver_data;

	if (!test_and_clear_bit(HCI_RUNNING, &hdev->flags))
		return 0;

	skb_queue_purge(&data->readq);

	return 0;
}

static int vhci_flush(struct hci_dev *hdev)
{
	struct vhci_data *data = hdev->driver_data;

	skb_queue_purge(&data->readq);

	return 0;
}

static int vhci_send_frame(struct sk_buff *skb)
{
	struct hci_dev* hdev = (struct hci_dev *) skb->dev;
	struct vhci_data *data;

	if (!hdev) {
		BT_ERR("Frame for unknown HCI device (hdev=NULL)");
		return -ENODEV;
	}

	if (!test_bit(HCI_RUNNING, &hdev->flags))
		return -EBUSY;

	data = hdev->driver_data;

	memcpy(skb_push(skb, 1), &bt_cb(skb)->pkt_type, 1);
	skb_queue_tail(&data->readq, skb);

	wake_up_interruptible(&data->read_wait);

	return 0;
}

static void vhci_destruct(struct hci_dev *hdev)
{
	kfree(hdev->driver_data);
}

static inline ssize_t vhci_get_user(struct vhci_data *data,
					const char __user *buf, size_t count)
{
	struct sk_buff *skb;

	if (count > HCI_MAX_FRAME_SIZE)
		return -EINVAL;

	skb = bt_skb_alloc(count, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	if (copy_from_user(skb_put(skb, count), buf, count)) {
		kfree_skb(skb);
		return -EFAULT;
	}

	skb->dev = (void *) data->hdev;
	bt_cb(skb)->pkt_type = *((__u8 *) skb->data);
	skb_pull(skb, 1);

	hci_recv_frame(skb);

	return count;
}

static inline ssize_t vhci_put_user(struct vhci_data *data,
			struct sk_buff *skb, char __user *buf, int count)
{
	char __user *ptr = buf;
	int len, total = 0;

	len = min_t(unsigned int, skb->len, count);

	if (copy_to_user(ptr, skb->data, len))
		return -EFAULT;

	total += len;

	data->hdev->stat.byte_tx += len;

	switch (bt_cb(skb)->pkt_type) {
	case HCI_COMMAND_PKT:
		data->hdev->stat.cmd_tx++;
		break;

	case HCI_ACLDATA_PKT:
		data->hdev->stat.acl_tx++;
		break;

	case HCI_SCODATA_PKT:
		data->hdev->stat.cmd_tx++;
		break;
	};

	return total;
}

static ssize_t vhci_read(struct file *file,
				char __user *buf, size_t count, loff_t *pos)
{
	struct vhci_data *data = file->private_data;
	struct sk_buff *skb;
	ssize_t ret = 0;

	while (count) {
		skb = skb_dequeue(&data->readq);
		if (skb) {
			ret = vhci_put_user(data, skb, buf, count);
			if (ret < 0)
				skb_queue_head(&data->readq, skb);
			else
				kfree_skb(skb);
			break;
		}

		if (file->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			break;
		}

		ret = wait_event_interruptible(data->read_wait,
					!skb_queue_empty(&data->readq));
		if (ret < 0)
			break;
	}

	return ret;
}

static ssize_t vhci_write(struct file *file,
			const char __user *buf, size_t count, loff_t *pos)
{
	struct vhci_data *data = file->private_data;

	return vhci_get_user(data, buf, count);
}

static unsigned int vhci_poll(struct file *file, poll_table *wait)
{
	struct vhci_data *data = file->private_data;

	poll_wait(file, &data->read_wait, wait);

	if (!skb_queue_empty(&data->readq))
		return POLLIN | POLLRDNORM;

	return POLLOUT | POLLWRNORM;
}

static int vhci_ioctl(struct inode *inode, struct file *file,
					unsigned int cmd, unsigned long arg)
{
	return -EINVAL;
}

static int vhci_open(struct inode *inode, struct file *file)
{
	struct vhci_data *data;
	struct hci_dev *hdev;

	data = kzalloc(sizeof(struct vhci_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	skb_queue_head_init(&data->readq);
	init_waitqueue_head(&data->read_wait);

	hdev = hci_alloc_dev();
	if (!hdev) {
		kfree(data);
		return -ENOMEM;
	}

	data->hdev = hdev;

	hdev->type = HCI_VIRTUAL;
	hdev->driver_data = data;

	hdev->open     = vhci_open_dev;
	hdev->close    = vhci_close_dev;
	hdev->flush    = vhci_flush;
	hdev->send     = vhci_send_frame;
	hdev->destruct = vhci_destruct;

	hdev->owner = THIS_MODULE;

	if (hci_register_dev(hdev) < 0) {
		BT_ERR("Can't register HCI device");
		kfree(data);
		hci_free_dev(hdev);
		return -EBUSY;
	}

	file->private_data = data;

	return nonseekable_open(inode, file);
}

static int vhci_release(struct inode *inode, struct file *file)
{
	struct vhci_data *data = file->private_data;
	struct hci_dev *hdev = data->hdev;

	if (hci_unregister_dev(hdev) < 0) {
		BT_ERR("Can't unregister HCI device %s", hdev->name);
	}

	hci_free_dev(hdev);

	file->private_data = NULL;

	return 0;
}

static const struct file_operations vhci_fops = {
	.read		= vhci_read,
	.write		= vhci_write,
	.poll		= vhci_poll,
	.ioctl		= vhci_ioctl,
	.open		= vhci_open,
	.release	= vhci_release,
};

static struct miscdevice vhci_miscdev= {
	.name	= "vhci",
	.fops	= &vhci_fops,
	.minor	= MISC_DYNAMIC_MINOR,
};

static int __init vhci_init(void)
{
	BT_INFO("Virtual HCI driver ver %s", VERSION);

	if (misc_register(&vhci_miscdev) < 0) {
		BT_ERR("Can't register misc device with minor %d", minor);
		return -EIO;
	}

	return 0;
}

static void __exit vhci_exit(void)
{
	if (misc_deregister(&vhci_miscdev) < 0)
		BT_ERR("Can't unregister misc device with minor %d", minor);
}

module_init(vhci_init);
module_exit(vhci_exit);

MODULE_AUTHOR("Marcel Holtmann <marcel@holtmann.org>");
MODULE_DESCRIPTION("Bluetooth virtual HCI driver ver " VERSION);
MODULE_VERSION(VERSION);
MODULE_LICENSE("GPL");
