

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kmod.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/system.h>

#include <media/v4l2-common.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>

#define VIDEO_NUM_DEVICES	256
#define VIDEO_NAME              "video4linux"



static ssize_t show_index(struct device *cd,
			 struct device_attribute *attr, char *buf)
{
	struct video_device *vdev = to_video_device(cd);

	return sprintf(buf, "%i\n", vdev->index);
}

static ssize_t show_name(struct device *cd,
			 struct device_attribute *attr, char *buf)
{
	struct video_device *vdev = to_video_device(cd);

	return sprintf(buf, "%.*s\n", (int)sizeof(vdev->name), vdev->name);
}

static struct device_attribute video_device_attrs[] = {
	__ATTR(name, S_IRUGO, show_name, NULL),
	__ATTR(index, S_IRUGO, show_index, NULL),
	__ATTR_NULL
};


static struct video_device *video_device[VIDEO_NUM_DEVICES];
static DEFINE_MUTEX(videodev_lock);
static DECLARE_BITMAP(devnode_nums[VFL_TYPE_MAX], VIDEO_NUM_DEVICES);





#ifdef CONFIG_VIDEO_FIXED_MINOR_RANGES

static inline unsigned long *devnode_bits(int vfl_type)
{
	
	int idx = (vfl_type > VFL_TYPE_VTX) ? VFL_TYPE_MAX - 1 : vfl_type;

	return devnode_nums[idx];
}
#else

static inline unsigned long *devnode_bits(int vfl_type)
{
	return devnode_nums[vfl_type];
}
#endif


static inline void devnode_set(struct video_device *vdev)
{
	set_bit(vdev->num, devnode_bits(vdev->vfl_type));
}


static inline void devnode_clear(struct video_device *vdev)
{
	clear_bit(vdev->num, devnode_bits(vdev->vfl_type));
}


static inline int devnode_find(struct video_device *vdev, int from, int to)
{
	return find_next_zero_bit(devnode_bits(vdev->vfl_type), to, from);
}

struct video_device *video_device_alloc(void)
{
	return kzalloc(sizeof(struct video_device), GFP_KERNEL);
}
EXPORT_SYMBOL(video_device_alloc);

void video_device_release(struct video_device *vdev)
{
	kfree(vdev);
}
EXPORT_SYMBOL(video_device_release);

void video_device_release_empty(struct video_device *vdev)
{
	
	
}
EXPORT_SYMBOL(video_device_release_empty);

static inline void video_get(struct video_device *vdev)
{
	get_device(&vdev->dev);
}

static inline void video_put(struct video_device *vdev)
{
	put_device(&vdev->dev);
}


static void v4l2_device_release(struct device *cd)
{
	struct video_device *vdev = to_video_device(cd);

	mutex_lock(&videodev_lock);
	if (video_device[vdev->minor] != vdev) {
		mutex_unlock(&videodev_lock);
		
		WARN_ON(1);
		return;
	}

	
	video_device[vdev->minor] = NULL;

	
	cdev_del(vdev->cdev);
	
	vdev->cdev = NULL;

	
	devnode_clear(vdev);

	mutex_unlock(&videodev_lock);

	
	vdev->release(vdev);
}

static struct class video_class = {
	.name = VIDEO_NAME,
	.dev_attrs = video_device_attrs,
};

struct video_device *video_devdata(struct file *file)
{
	return video_device[iminor(file->f_path.dentry->d_inode)];
}
EXPORT_SYMBOL(video_devdata);

static ssize_t v4l2_read(struct file *filp, char __user *buf,
		size_t sz, loff_t *off)
{
	struct video_device *vdev = video_devdata(filp);

	if (!vdev->fops->read)
		return -EINVAL;
	if (video_is_unregistered(vdev))
		return -EIO;
	return vdev->fops->read(filp, buf, sz, off);
}

static ssize_t v4l2_write(struct file *filp, const char __user *buf,
		size_t sz, loff_t *off)
{
	struct video_device *vdev = video_devdata(filp);

	if (!vdev->fops->write)
		return -EINVAL;
	if (video_is_unregistered(vdev))
		return -EIO;
	return vdev->fops->write(filp, buf, sz, off);
}

static unsigned int v4l2_poll(struct file *filp, struct poll_table_struct *poll)
{
	struct video_device *vdev = video_devdata(filp);

	if (!vdev->fops->poll || video_is_unregistered(vdev))
		return DEFAULT_POLLMASK;
	return vdev->fops->poll(filp, poll);
}

static int v4l2_ioctl(struct inode *inode, struct file *filp,
		unsigned int cmd, unsigned long arg)
{
	struct video_device *vdev = video_devdata(filp);

	if (!vdev->fops->ioctl)
		return -ENOTTY;
	
	return vdev->fops->ioctl(filp, cmd, arg);
}

static long v4l2_unlocked_ioctl(struct file *filp,
		unsigned int cmd, unsigned long arg)
{
	struct video_device *vdev = video_devdata(filp);

	if (!vdev->fops->unlocked_ioctl)
		return -ENOTTY;
	
	return vdev->fops->unlocked_ioctl(filp, cmd, arg);
}

#ifdef CONFIG_MMU
#define v4l2_get_unmapped_area NULL
#else
static unsigned long v4l2_get_unmapped_area(struct file *filp,
		unsigned long addr, unsigned long len, unsigned long pgoff,
		unsigned long flags)
{
	struct video_device *vdev = video_devdata(filp);

	if (!vdev->fops->get_unmapped_area)
		return -ENOSYS;
	if (video_is_unregistered(vdev))
		return -ENODEV;
	return vdev->fops->get_unmapped_area(filp, addr, len, pgoff, flags);
}
#endif

static int v4l2_mmap(struct file *filp, struct vm_area_struct *vm)
{
	struct video_device *vdev = video_devdata(filp);

	if (!vdev->fops->mmap ||
	    video_is_unregistered(vdev))
		return -ENODEV;
	return vdev->fops->mmap(filp, vm);
}


static int v4l2_open(struct inode *inode, struct file *filp)
{
	struct video_device *vdev;
	int ret = 0;

	
	mutex_lock(&videodev_lock);
	vdev = video_devdata(filp);
	
	if (vdev == NULL || video_is_unregistered(vdev)) {
		mutex_unlock(&videodev_lock);
		return -ENODEV;
	}
	
	video_get(vdev);
	mutex_unlock(&videodev_lock);
	if (vdev->fops->open)
		ret = vdev->fops->open(filp);

	
	if (ret)
		video_put(vdev);
	return ret;
}


static int v4l2_release(struct inode *inode, struct file *filp)
{
	struct video_device *vdev = video_devdata(filp);
	int ret = 0;

	if (vdev->fops->release)
		vdev->fops->release(filp);

	
	video_put(vdev);
	return ret;
}

static const struct file_operations v4l2_unlocked_fops = {
	.owner = THIS_MODULE,
	.read = v4l2_read,
	.write = v4l2_write,
	.open = v4l2_open,
	.get_unmapped_area = v4l2_get_unmapped_area,
	.mmap = v4l2_mmap,
	.unlocked_ioctl = v4l2_unlocked_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = v4l2_compat_ioctl32,
#endif
	.release = v4l2_release,
	.poll = v4l2_poll,
	.llseek = no_llseek,
};

static const struct file_operations v4l2_fops = {
	.owner = THIS_MODULE,
	.read = v4l2_read,
	.write = v4l2_write,
	.open = v4l2_open,
	.get_unmapped_area = v4l2_get_unmapped_area,
	.mmap = v4l2_mmap,
	.ioctl = v4l2_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = v4l2_compat_ioctl32,
#endif
	.release = v4l2_release,
	.poll = v4l2_poll,
	.llseek = no_llseek,
};


static int get_index(struct video_device *vdev)
{
	
	static DECLARE_BITMAP(used, VIDEO_NUM_DEVICES);
	int i;

	
	if (vdev->parent == NULL)
		return 0;

	bitmap_zero(used, VIDEO_NUM_DEVICES);

	for (i = 0; i < VIDEO_NUM_DEVICES; i++) {
		if (video_device[i] != NULL &&
		    video_device[i]->parent == vdev->parent) {
			set_bit(video_device[i]->index, used);
		}
	}

	return find_first_zero_bit(used, VIDEO_NUM_DEVICES);
}


static int __video_register_device(struct video_device *vdev, int type, int nr,
		int warn_if_nr_in_use)
{
	int i = 0;
	int ret;
	int minor_offset = 0;
	int minor_cnt = VIDEO_NUM_DEVICES;
	const char *name_base;
	void *priv = video_get_drvdata(vdev);

	
	vdev->minor = -1;

	
	WARN_ON(!vdev->release);
	if (!vdev->release)
		return -EINVAL;

	
	switch (type) {
	case VFL_TYPE_GRABBER:
		name_base = "video";
		break;
	case VFL_TYPE_VTX:
		name_base = "vtx";
		break;
	case VFL_TYPE_VBI:
		name_base = "vbi";
		break;
	case VFL_TYPE_RADIO:
		name_base = "radio";
		break;
	default:
		printk(KERN_ERR "%s called with unknown type: %d\n",
		       __func__, type);
		return -EINVAL;
	}

	vdev->vfl_type = type;
	vdev->cdev = NULL;
	if (vdev->v4l2_dev && vdev->v4l2_dev->dev)
		vdev->parent = vdev->v4l2_dev->dev;

	
#ifdef CONFIG_VIDEO_FIXED_MINOR_RANGES
	
	switch (type) {
	case VFL_TYPE_GRABBER:
		minor_offset = 0;
		minor_cnt = 64;
		break;
	case VFL_TYPE_RADIO:
		minor_offset = 64;
		minor_cnt = 64;
		break;
	case VFL_TYPE_VTX:
		minor_offset = 192;
		minor_cnt = 32;
		break;
	case VFL_TYPE_VBI:
		minor_offset = 224;
		minor_cnt = 32;
		break;
	default:
		minor_offset = 128;
		minor_cnt = 64;
		break;
	}
#endif

	
	mutex_lock(&videodev_lock);
	nr = devnode_find(vdev, nr == -1 ? 0 : nr, minor_cnt);
	if (nr == minor_cnt)
		nr = devnode_find(vdev, 0, minor_cnt);
	if (nr == minor_cnt) {
		printk(KERN_ERR "could not get a free device node number\n");
		mutex_unlock(&videodev_lock);
		return -ENFILE;
	}
#ifdef CONFIG_VIDEO_FIXED_MINOR_RANGES
	
	i = nr;
#else
	
	for (i = 0; i < VIDEO_NUM_DEVICES; i++)
		if (video_device[i] == NULL)
			break;
	if (i == VIDEO_NUM_DEVICES) {
		mutex_unlock(&videodev_lock);
		printk(KERN_ERR "could not get a free minor\n");
		return -ENFILE;
	}
#endif
	vdev->minor = i + minor_offset;
	vdev->num = nr;
	devnode_set(vdev);

	
	WARN_ON(video_device[vdev->minor] != NULL);
	vdev->index = get_index(vdev);
	mutex_unlock(&videodev_lock);

	
	vdev->cdev = cdev_alloc();
	if (vdev->cdev == NULL) {
		ret = -ENOMEM;
		goto cleanup;
	}
	if (vdev->fops->unlocked_ioctl)
		vdev->cdev->ops = &v4l2_unlocked_fops;
	else
		vdev->cdev->ops = &v4l2_fops;
	vdev->cdev->owner = vdev->fops->owner;
	ret = cdev_add(vdev->cdev, MKDEV(VIDEO_MAJOR, vdev->minor), 1);
	if (ret < 0) {
		printk(KERN_ERR "%s: cdev_add failed\n", __func__);
		kfree(vdev->cdev);
		vdev->cdev = NULL;
		goto cleanup;
	}

	
	memset(&vdev->dev, 0, sizeof(vdev->dev));
	
	video_set_drvdata(vdev, priv);
	vdev->dev.class = &video_class;
	vdev->dev.devt = MKDEV(VIDEO_MAJOR, vdev->minor);
	if (vdev->parent)
		vdev->dev.parent = vdev->parent;
	dev_set_name(&vdev->dev, "%s%d", name_base, vdev->num);
	ret = device_register(&vdev->dev);
	if (ret < 0) {
		printk(KERN_ERR "%s: device_register failed\n", __func__);
		goto cleanup;
	}
	
	vdev->dev.release = v4l2_device_release;

	if (nr != -1 && nr != vdev->num && warn_if_nr_in_use)
		printk(KERN_WARNING "%s: requested %s%d, got %s%d\n",
				__func__, name_base, nr, name_base, vdev->num);

	
	mutex_lock(&videodev_lock);
	video_device[vdev->minor] = vdev;
	mutex_unlock(&videodev_lock);
	return 0;

cleanup:
	mutex_lock(&videodev_lock);
	if (vdev->cdev)
		cdev_del(vdev->cdev);
	devnode_clear(vdev);
	mutex_unlock(&videodev_lock);
	
	vdev->minor = -1;
	return ret;
}

int video_register_device(struct video_device *vdev, int type, int nr)
{
	return __video_register_device(vdev, type, nr, 1);
}
EXPORT_SYMBOL(video_register_device);

int video_register_device_no_warn(struct video_device *vdev, int type, int nr)
{
	return __video_register_device(vdev, type, nr, 0);
}
EXPORT_SYMBOL(video_register_device_no_warn);


void video_unregister_device(struct video_device *vdev)
{
	
	if (!vdev || vdev->minor < 0)
		return;

	mutex_lock(&videodev_lock);
	set_bit(V4L2_FL_UNREGISTERED, &vdev->flags);
	mutex_unlock(&videodev_lock);
	device_unregister(&vdev->dev);
}
EXPORT_SYMBOL(video_unregister_device);


static int __init videodev_init(void)
{
	dev_t dev = MKDEV(VIDEO_MAJOR, 0);
	int ret;

	printk(KERN_INFO "Linux video capture interface: v2.00\n");
	ret = register_chrdev_region(dev, VIDEO_NUM_DEVICES, VIDEO_NAME);
	if (ret < 0) {
		printk(KERN_WARNING "videodev: unable to get major %d\n",
				VIDEO_MAJOR);
		return ret;
	}

	ret = class_register(&video_class);
	if (ret < 0) {
		unregister_chrdev_region(dev, VIDEO_NUM_DEVICES);
		printk(KERN_WARNING "video_dev: class_register failed\n");
		return -EIO;
	}

	return 0;
}

static void __exit videodev_exit(void)
{
	dev_t dev = MKDEV(VIDEO_MAJOR, 0);

	class_unregister(&video_class);
	unregister_chrdev_region(dev, VIDEO_NUM_DEVICES);
}

module_init(videodev_init)
module_exit(videodev_exit)

MODULE_AUTHOR("Alan Cox, Mauro Carvalho Chehab <mchehab@infradead.org>");
MODULE_DESCRIPTION("Device registrar for Video4Linux drivers v2");
MODULE_LICENSE("GPL");
MODULE_ALIAS_CHARDEV_MAJOR(VIDEO_MAJOR);



