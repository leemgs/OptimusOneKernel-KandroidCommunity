

#ifndef VIDC_INIT_INTERNAL_H
#define VIDC_INIT_INTERNAL_H

#include <linux/cdev.h>

struct vidc_timer {
	struct list_head list;
	struct timer_list hw_timeout;
	void (*cb_func)(void *);
	void *userdata;
};

struct vidc_dev {
	struct cdev cdev;
	struct device *device;
	resource_size_t phys_base;
	void __iomem *virt_base;
	unsigned int irq;
	unsigned int ref_count;
	unsigned int firmware_refcount;
	unsigned int get_firmware;
	struct mutex lock;
	s32 device_handle;
	struct list_head vidc_timer_queue;
	struct work_struct vidc_timer_worker;
};

#endif
