#ifndef _LINUX_VIRTIO_H
#define _LINUX_VIRTIO_H

#include <linux/types.h>
#include <linux/scatterlist.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/mod_devicetable.h>


struct virtqueue {
	struct list_head list;
	void (*callback)(struct virtqueue *vq);
	const char *name;
	struct virtio_device *vdev;
	struct virtqueue_ops *vq_ops;
	void *priv;
};


struct virtqueue_ops {
	int (*add_buf)(struct virtqueue *vq,
		       struct scatterlist sg[],
		       unsigned int out_num,
		       unsigned int in_num,
		       void *data);

	void (*kick)(struct virtqueue *vq);

	void *(*get_buf)(struct virtqueue *vq, unsigned int *len);

	void (*disable_cb)(struct virtqueue *vq);
	bool (*enable_cb)(struct virtqueue *vq);
};


struct virtio_device {
	int index;
	struct device dev;
	struct virtio_device_id id;
	struct virtio_config_ops *config;
	struct list_head vqs;
	
	unsigned long features[1];
	void *priv;
};

int register_virtio_device(struct virtio_device *dev);
void unregister_virtio_device(struct virtio_device *dev);


struct virtio_driver {
	struct device_driver driver;
	const struct virtio_device_id *id_table;
	const unsigned int *feature_table;
	unsigned int feature_table_size;
	int (*probe)(struct virtio_device *dev);
	void (*remove)(struct virtio_device *dev);
	void (*config_changed)(struct virtio_device *dev);
};

int register_virtio_driver(struct virtio_driver *drv);
void unregister_virtio_driver(struct virtio_driver *drv);
#endif 
