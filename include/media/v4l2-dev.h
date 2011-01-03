
#ifndef _V4L2_DEV_H
#define _V4L2_DEV_H

#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/videodev2.h>

#define VIDEO_MAJOR	81

#define VFL_TYPE_GRABBER	0
#define VFL_TYPE_VBI		1
#define VFL_TYPE_RADIO		2
#define VFL_TYPE_VTX		3
#define VFL_TYPE_MAX		4

struct v4l2_ioctl_callbacks;
struct video_device;
struct v4l2_device;


#define V4L2_FL_UNREGISTERED	(0)

struct v4l2_file_operations {
	struct module *owner;
	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
	unsigned int (*poll) (struct file *, struct poll_table_struct *);
	long (*ioctl) (struct file *, unsigned int, unsigned long);
	long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
	unsigned long (*get_unmapped_area) (struct file *, unsigned long,
				unsigned long, unsigned long, unsigned long);
	int (*mmap) (struct file *, struct vm_area_struct *);
	int (*open) (struct file *);
	int (*release) (struct file *);
};



struct video_device
{
	
	const struct v4l2_file_operations *fops;

	
	struct device dev;		
	struct cdev *cdev;		

	
	struct device *parent;		
	struct v4l2_device *v4l2_dev;	

	
	char name[32];
	int vfl_type;
	
	int minor;
	u16 num;
	
	unsigned long flags;
	
	int index;

	int debug;			

	
	v4l2_std_id tvnorms;		
	v4l2_std_id current_norm;	

	
	void (*release)(struct video_device *vdev);

	
	const struct v4l2_ioctl_ops *ioctl_ops;
};


#define to_video_device(cd) container_of(cd, struct video_device, dev)


int __must_check video_register_device(struct video_device *vdev, int type, int nr);


int __must_check video_register_device_no_warn(struct video_device *vdev, int type, int nr);


void video_unregister_device(struct video_device *vdev);


struct video_device * __must_check video_device_alloc(void);


void video_device_release(struct video_device *vdev);


void video_device_release_empty(struct video_device *vdev);


static inline void *video_get_drvdata(struct video_device *vdev)
{
	return dev_get_drvdata(&vdev->dev);
}

static inline void video_set_drvdata(struct video_device *vdev, void *data)
{
	dev_set_drvdata(&vdev->dev, data);
}

struct video_device *video_devdata(struct file *file);


static inline void *video_drvdata(struct file *file)
{
	return video_get_drvdata(video_devdata(file));
}

static inline int video_is_unregistered(struct video_device *vdev)
{
	return test_bit(V4L2_FL_UNREGISTERED, &vdev->flags);
}

#endif 
