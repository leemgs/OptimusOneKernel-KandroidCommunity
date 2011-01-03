


#ifndef _VMBUS_H_
#define _VMBUS_H_

#include <linux/device.h>
#include "VmbusApi.h"

struct driver_context {
	struct hv_guid class_id;

	struct device_driver driver;

	
	int (*probe)(struct device *);
	int (*remove)(struct device *);
	void (*shutdown)(struct device *);
};

struct device_context {
	struct work_struct probe_failed_work_item;
	struct hv_guid class_id;
	struct hv_guid device_id;
	int probe_error;
	struct device device;
	struct hv_device device_obj;
};

static inline struct device_context *to_device_context(struct hv_device *d)
{
	return container_of(d, struct device_context, device_obj);
}

static inline struct device_context *device_to_device_context(struct device *d)
{
	return container_of(d, struct device_context, device);
}

static inline struct driver_context *driver_to_driver_context(struct device_driver *d)
{
	return container_of(d, struct driver_context, driver);
}




int vmbus_child_driver_register(struct driver_context *driver_ctx);
void vmbus_child_driver_unregister(struct driver_context *driver_ctx);
void vmbus_get_interface(struct vmbus_channel_interface *interface);

#endif 
