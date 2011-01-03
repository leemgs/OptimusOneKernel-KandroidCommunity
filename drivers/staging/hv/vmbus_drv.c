
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/sysctl.h>
#include "osd.h"
#include "logging.h"
#include "vmbus.h"



#define VMBUS_IRQ		0x5
#define VMBUS_IRQ_VECTOR	IRQ5_VECTOR


struct vmbus_driver_context {
	
	
	
	
	struct driver_context drv_ctx;
	struct vmbus_driver drv_obj;

	struct bus_type bus;
	struct tasklet_struct msg_dpc;
	struct tasklet_struct event_dpc;

	
	struct device_context device_ctx;
};

static int vmbus_match(struct device *device, struct device_driver *driver);
static int vmbus_probe(struct device *device);
static int vmbus_remove(struct device *device);
static void vmbus_shutdown(struct device *device);
static int vmbus_uevent(struct device *device, struct kobj_uevent_env *env);
static void vmbus_msg_dpc(unsigned long data);
static void vmbus_event_dpc(unsigned long data);

static irqreturn_t vmbus_isr(int irq, void *dev_id);

static void vmbus_device_release(struct device *device);
static void vmbus_bus_release(struct device *device);

static struct hv_device *vmbus_child_device_create(struct hv_guid *type,
						   struct hv_guid *instance,
						   void *context);
static void vmbus_child_device_destroy(struct hv_device *device_obj);
static int vmbus_child_device_register(struct hv_device *root_device_obj,
				       struct hv_device *child_device_obj);
static void vmbus_child_device_unregister(struct hv_device *child_device_obj);
static void vmbus_child_device_get_info(struct hv_device *device_obj,
					struct hv_device_info *device_info);
static ssize_t vmbus_show_device_attr(struct device *dev,
				      struct device_attribute *dev_attr,
				      char *buf);


unsigned int vmbus_loglevel = (ALL_MODULES << 16 | INFO_LVL);
EXPORT_SYMBOL(vmbus_loglevel);
	
	

static int vmbus_irq = VMBUS_IRQ;


static struct device_attribute vmbus_device_attrs[] = {
	__ATTR(id, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR(state, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR(class_id, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR(device_id, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR(monitor_id, S_IRUGO, vmbus_show_device_attr, NULL),

	__ATTR(server_monitor_pending, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR(server_monitor_latency, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR(server_monitor_conn_id, S_IRUGO, vmbus_show_device_attr, NULL),

	__ATTR(client_monitor_pending, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR(client_monitor_latency, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR(client_monitor_conn_id, S_IRUGO, vmbus_show_device_attr, NULL),

	__ATTR(out_intr_mask, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR(out_read_index, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR(out_write_index, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR(out_read_bytes_avail, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR(out_write_bytes_avail, S_IRUGO, vmbus_show_device_attr, NULL),

	__ATTR(in_intr_mask, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR(in_read_index, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR(in_write_index, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR(in_read_bytes_avail, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR(in_write_bytes_avail, S_IRUGO, vmbus_show_device_attr, NULL),
	__ATTR_NULL
};


static struct vmbus_driver_context g_vmbus_drv = {
	.bus.name =		"vmbus",
	.bus.match =		vmbus_match,
	.bus.shutdown =		vmbus_shutdown,
	.bus.remove =		vmbus_remove,
	.bus.probe =		vmbus_probe,
	.bus.uevent =		vmbus_uevent,
	.bus.dev_attrs =	vmbus_device_attrs,
};


static ssize_t vmbus_show_device_attr(struct device *dev,
				      struct device_attribute *dev_attr,
				      char *buf)
{
	struct device_context *device_ctx = device_to_device_context(dev);
	struct hv_device_info device_info;

	memset(&device_info, 0, sizeof(struct hv_device_info));

	vmbus_child_device_get_info(&device_ctx->device_obj, &device_info);

	if (!strcmp(dev_attr->attr.name, "class_id")) {
		return sprintf(buf, "{%02x%02x%02x%02x-%02x%02x-%02x%02x-"
			       "%02x%02x%02x%02x%02x%02x%02x%02x}\n",
			       device_info.ChannelType.data[3],
			       device_info.ChannelType.data[2],
			       device_info.ChannelType.data[1],
			       device_info.ChannelType.data[0],
			       device_info.ChannelType.data[5],
			       device_info.ChannelType.data[4],
			       device_info.ChannelType.data[7],
			       device_info.ChannelType.data[6],
			       device_info.ChannelType.data[8],
			       device_info.ChannelType.data[9],
			       device_info.ChannelType.data[10],
			       device_info.ChannelType.data[11],
			       device_info.ChannelType.data[12],
			       device_info.ChannelType.data[13],
			       device_info.ChannelType.data[14],
			       device_info.ChannelType.data[15]);
	} else if (!strcmp(dev_attr->attr.name, "device_id")) {
		return sprintf(buf, "{%02x%02x%02x%02x-%02x%02x-%02x%02x-"
			       "%02x%02x%02x%02x%02x%02x%02x%02x}\n",
			       device_info.ChannelInstance.data[3],
			       device_info.ChannelInstance.data[2],
			       device_info.ChannelInstance.data[1],
			       device_info.ChannelInstance.data[0],
			       device_info.ChannelInstance.data[5],
			       device_info.ChannelInstance.data[4],
			       device_info.ChannelInstance.data[7],
			       device_info.ChannelInstance.data[6],
			       device_info.ChannelInstance.data[8],
			       device_info.ChannelInstance.data[9],
			       device_info.ChannelInstance.data[10],
			       device_info.ChannelInstance.data[11],
			       device_info.ChannelInstance.data[12],
			       device_info.ChannelInstance.data[13],
			       device_info.ChannelInstance.data[14],
			       device_info.ChannelInstance.data[15]);
	} else if (!strcmp(dev_attr->attr.name, "state")) {
		return sprintf(buf, "%d\n", device_info.ChannelState);
	} else if (!strcmp(dev_attr->attr.name, "id")) {
		return sprintf(buf, "%d\n", device_info.ChannelId);
	} else if (!strcmp(dev_attr->attr.name, "out_intr_mask")) {
		return sprintf(buf, "%d\n", device_info.Outbound.InterruptMask);
	} else if (!strcmp(dev_attr->attr.name, "out_read_index")) {
		return sprintf(buf, "%d\n", device_info.Outbound.ReadIndex);
	} else if (!strcmp(dev_attr->attr.name, "out_write_index")) {
		return sprintf(buf, "%d\n", device_info.Outbound.WriteIndex);
	} else if (!strcmp(dev_attr->attr.name, "out_read_bytes_avail")) {
		return sprintf(buf, "%d\n",
			       device_info.Outbound.BytesAvailToRead);
	} else if (!strcmp(dev_attr->attr.name, "out_write_bytes_avail")) {
		return sprintf(buf, "%d\n",
			       device_info.Outbound.BytesAvailToWrite);
	} else if (!strcmp(dev_attr->attr.name, "in_intr_mask")) {
		return sprintf(buf, "%d\n", device_info.Inbound.InterruptMask);
	} else if (!strcmp(dev_attr->attr.name, "in_read_index")) {
		return sprintf(buf, "%d\n", device_info.Inbound.ReadIndex);
	} else if (!strcmp(dev_attr->attr.name, "in_write_index")) {
		return sprintf(buf, "%d\n", device_info.Inbound.WriteIndex);
	} else if (!strcmp(dev_attr->attr.name, "in_read_bytes_avail")) {
		return sprintf(buf, "%d\n",
			       device_info.Inbound.BytesAvailToRead);
	} else if (!strcmp(dev_attr->attr.name, "in_write_bytes_avail")) {
		return sprintf(buf, "%d\n",
			       device_info.Inbound.BytesAvailToWrite);
	} else if (!strcmp(dev_attr->attr.name, "monitor_id")) {
		return sprintf(buf, "%d\n", device_info.MonitorId);
	} else if (!strcmp(dev_attr->attr.name, "server_monitor_pending")) {
		return sprintf(buf, "%d\n", device_info.ServerMonitorPending);
	} else if (!strcmp(dev_attr->attr.name, "server_monitor_latency")) {
		return sprintf(buf, "%d\n", device_info.ServerMonitorLatency);
	} else if (!strcmp(dev_attr->attr.name, "server_monitor_conn_id")) {
		return sprintf(buf, "%d\n",
			       device_info.ServerMonitorConnectionId);
	} else if (!strcmp(dev_attr->attr.name, "client_monitor_pending")) {
		return sprintf(buf, "%d\n", device_info.ClientMonitorPending);
	} else if (!strcmp(dev_attr->attr.name, "client_monitor_latency")) {
		return sprintf(buf, "%d\n", device_info.ClientMonitorLatency);
	} else if (!strcmp(dev_attr->attr.name, "client_monitor_conn_id")) {
		return sprintf(buf, "%d\n",
			       device_info.ClientMonitorConnectionId);
	} else {
		return 0;
	}
}


static int vmbus_bus_init(int (*drv_init)(struct hv_driver *drv))
{
	struct vmbus_driver_context *vmbus_drv_ctx = &g_vmbus_drv;
	struct vmbus_driver *vmbus_drv_obj = &g_vmbus_drv.drv_obj;
	struct device_context *dev_ctx = &g_vmbus_drv.device_ctx;
	int ret;
	unsigned int vector;

	DPRINT_ENTER(VMBUS_DRV);

	
	vmbus_drv_obj->OnChildDeviceCreate = vmbus_child_device_create;
	vmbus_drv_obj->OnChildDeviceDestroy = vmbus_child_device_destroy;
	vmbus_drv_obj->OnChildDeviceAdd = vmbus_child_device_register;
	vmbus_drv_obj->OnChildDeviceRemove = vmbus_child_device_unregister;

	
	ret = drv_init(&vmbus_drv_obj->Base);
	if (ret != 0) {
		DPRINT_ERR(VMBUS_DRV, "Unable to initialize vmbus (%d)", ret);
		goto cleanup;
	}

	
	if (!vmbus_drv_obj->Base.OnDeviceAdd) {
		DPRINT_ERR(VMBUS_DRV, "OnDeviceAdd() routine not set");
		ret = -1;
		goto cleanup;
	}

	vmbus_drv_ctx->bus.name = vmbus_drv_obj->Base.name;

	
	tasklet_init(&vmbus_drv_ctx->msg_dpc, vmbus_msg_dpc,
		     (unsigned long)vmbus_drv_obj);
	tasklet_init(&vmbus_drv_ctx->event_dpc, vmbus_event_dpc,
		     (unsigned long)vmbus_drv_obj);

	
	ret = bus_register(&vmbus_drv_ctx->bus);
	if (ret) {
		ret = -1;
		goto cleanup;
	}

	
	ret = request_irq(vmbus_irq, vmbus_isr, IRQF_SAMPLE_RANDOM,
			  vmbus_drv_obj->Base.name, NULL);

	if (ret != 0) {
		DPRINT_ERR(VMBUS_DRV, "ERROR - Unable to request IRQ %d",
			   vmbus_irq);

		bus_unregister(&vmbus_drv_ctx->bus);

		ret = -1;
		goto cleanup;
	}
	vector = VMBUS_IRQ_VECTOR;

	DPRINT_INFO(VMBUS_DRV, "irq 0x%x vector 0x%x", vmbus_irq, vector);

	
	memset(dev_ctx, 0, sizeof(struct device_context));

	ret = vmbus_drv_obj->Base.OnDeviceAdd(&dev_ctx->device_obj, &vector);
	if (ret != 0) {
		DPRINT_ERR(VMBUS_DRV,
			   "ERROR - Unable to add vmbus root device");

		free_irq(vmbus_irq, NULL);

		bus_unregister(&vmbus_drv_ctx->bus);

		ret = -1;
		goto cleanup;
	}
	
	dev_set_name(&dev_ctx->device, "vmbus_0_0");
	memcpy(&dev_ctx->class_id, &dev_ctx->device_obj.deviceType,
		sizeof(struct hv_guid));
	memcpy(&dev_ctx->device_id, &dev_ctx->device_obj.deviceInstance,
		sizeof(struct hv_guid));

	
	dev_ctx->device.parent = NULL;
	
	dev_ctx->device.bus = &vmbus_drv_ctx->bus;

	
	dev_ctx->device.release = vmbus_bus_release;

	
	ret = device_register(&dev_ctx->device);
	if (ret) {
		DPRINT_ERR(VMBUS_DRV,
			   "ERROR - Unable to register vmbus root device");

		free_irq(vmbus_irq, NULL);
		bus_unregister(&vmbus_drv_ctx->bus);

		ret = -1;
		goto cleanup;
	}


	vmbus_drv_obj->GetChannelOffers();

cleanup:
	DPRINT_EXIT(VMBUS_DRV);

	return ret;
}


static void vmbus_bus_exit(void)
{
	struct vmbus_driver *vmbus_drv_obj = &g_vmbus_drv.drv_obj;
	struct vmbus_driver_context *vmbus_drv_ctx = &g_vmbus_drv;

	struct device_context *dev_ctx = &g_vmbus_drv.device_ctx;

	DPRINT_ENTER(VMBUS_DRV);

	
	if (vmbus_drv_obj->Base.OnDeviceRemove)
		vmbus_drv_obj->Base.OnDeviceRemove(&dev_ctx->device_obj);

	if (vmbus_drv_obj->Base.OnCleanup)
		vmbus_drv_obj->Base.OnCleanup(&vmbus_drv_obj->Base);

	
	device_unregister(&dev_ctx->device);

	bus_unregister(&vmbus_drv_ctx->bus);

	free_irq(vmbus_irq, NULL);

	tasklet_kill(&vmbus_drv_ctx->msg_dpc);
	tasklet_kill(&vmbus_drv_ctx->event_dpc);

	DPRINT_EXIT(VMBUS_DRV);

	return;
}


int vmbus_child_driver_register(struct driver_context *driver_ctx)
{
	struct vmbus_driver *vmbus_drv_obj = &g_vmbus_drv.drv_obj;
	int ret;

	DPRINT_ENTER(VMBUS_DRV);

	DPRINT_INFO(VMBUS_DRV, "child driver (%p) registering - name %s",
		    driver_ctx, driver_ctx->driver.name);

	
	driver_ctx->driver.bus = &g_vmbus_drv.bus;

	ret = driver_register(&driver_ctx->driver);

	vmbus_drv_obj->GetChannelOffers();

	DPRINT_EXIT(VMBUS_DRV);

	return ret;
}
EXPORT_SYMBOL(vmbus_child_driver_register);


void vmbus_child_driver_unregister(struct driver_context *driver_ctx)
{
	DPRINT_ENTER(VMBUS_DRV);

	DPRINT_INFO(VMBUS_DRV, "child driver (%p) unregistering - name %s",
		    driver_ctx, driver_ctx->driver.name);

	driver_unregister(&driver_ctx->driver);

	driver_ctx->driver.bus = NULL;

	DPRINT_EXIT(VMBUS_DRV);
}
EXPORT_SYMBOL(vmbus_child_driver_unregister);


void vmbus_get_interface(struct vmbus_channel_interface *interface)
{
	struct vmbus_driver *vmbus_drv_obj = &g_vmbus_drv.drv_obj;

	vmbus_drv_obj->GetChannelInterface(interface);
}
EXPORT_SYMBOL(vmbus_get_interface);


static void vmbus_child_device_get_info(struct hv_device *device_obj,
					struct hv_device_info *device_info)
{
	struct vmbus_driver *vmbus_drv_obj = &g_vmbus_drv.drv_obj;

	vmbus_drv_obj->GetChannelInfo(device_obj, device_info);
}


static struct hv_device *vmbus_child_device_create(struct hv_guid *type,
						   struct hv_guid *instance,
						   void *context)
{
	struct device_context *child_device_ctx;
	struct hv_device *child_device_obj;

	DPRINT_ENTER(VMBUS_DRV);

	
	child_device_ctx = kzalloc(sizeof(struct device_context), GFP_KERNEL);
	if (!child_device_ctx) {
		DPRINT_ERR(VMBUS_DRV,
			"unable to allocate device_context for child device");
		DPRINT_EXIT(VMBUS_DRV);

		return NULL;
	}

	DPRINT_DBG(VMBUS_DRV, "child device (%p) allocated - "
		"type {%02x%02x%02x%02x-%02x%02x-%02x%02x-"
		"%02x%02x%02x%02x%02x%02x%02x%02x},"
		"id {%02x%02x%02x%02x-%02x%02x-%02x%02x-"
		"%02x%02x%02x%02x%02x%02x%02x%02x}",
		&child_device_ctx->device,
		type->data[3], type->data[2], type->data[1], type->data[0],
		type->data[5], type->data[4], type->data[7], type->data[6],
		type->data[8], type->data[9], type->data[10], type->data[11],
		type->data[12], type->data[13], type->data[14], type->data[15],
		instance->data[3], instance->data[2],
		instance->data[1], instance->data[0],
		instance->data[5], instance->data[4],
		instance->data[7], instance->data[6],
		instance->data[8], instance->data[9],
		instance->data[10], instance->data[11],
		instance->data[12], instance->data[13],
		instance->data[14], instance->data[15]);

	child_device_obj = &child_device_ctx->device_obj;
	child_device_obj->context = context;
	memcpy(&child_device_obj->deviceType, type, sizeof(struct hv_guid));
	memcpy(&child_device_obj->deviceInstance, instance,
	       sizeof(struct hv_guid));

	memcpy(&child_device_ctx->class_id, type, sizeof(struct hv_guid));
	memcpy(&child_device_ctx->device_id, instance, sizeof(struct hv_guid));

	DPRINT_EXIT(VMBUS_DRV);

	return child_device_obj;
}


static int vmbus_child_device_register(struct hv_device *root_device_obj,
				       struct hv_device *child_device_obj)
{
	int ret = 0;
	struct device_context *root_device_ctx =
				to_device_context(root_device_obj);
	struct device_context *child_device_ctx =
				to_device_context(child_device_obj);
	static atomic_t device_num = ATOMIC_INIT(0);

	DPRINT_ENTER(VMBUS_DRV);

	DPRINT_DBG(VMBUS_DRV, "child device (%p) registering",
		   child_device_ctx);

	
	dev_set_name(&child_device_ctx->device, "vmbus_0_%d",
		     atomic_inc_return(&device_num));

	
	child_device_ctx->device.bus = &g_vmbus_drv.bus; 
	child_device_ctx->device.parent = &root_device_ctx->device;
	child_device_ctx->device.release = vmbus_device_release;

	
	ret = device_register(&child_device_ctx->device);

	
	ret = child_device_ctx->probe_error;

	if (ret)
		DPRINT_ERR(VMBUS_DRV, "unable to register child device (%p)",
			   &child_device_ctx->device);
	else
		DPRINT_INFO(VMBUS_DRV, "child device (%p) registered",
			    &child_device_ctx->device);

	DPRINT_EXIT(VMBUS_DRV);

	return ret;
}


static void vmbus_child_device_unregister(struct hv_device *device_obj)
{
	struct device_context *device_ctx = to_device_context(device_obj);

	DPRINT_ENTER(VMBUS_DRV);

	DPRINT_INFO(VMBUS_DRV, "unregistering child device (%p)",
		    &device_ctx->device);

	
	device_unregister(&device_ctx->device);

	DPRINT_INFO(VMBUS_DRV, "child device (%p) unregistered",
		    &device_ctx->device);

	DPRINT_EXIT(VMBUS_DRV);
}


static void vmbus_child_device_destroy(struct hv_device *device_obj)
{
	DPRINT_ENTER(VMBUS_DRV);

	DPRINT_EXIT(VMBUS_DRV);
}


static int vmbus_uevent(struct device *device, struct kobj_uevent_env *env)
{
	struct device_context *device_ctx = device_to_device_context(device);
	int ret;

	DPRINT_ENTER(VMBUS_DRV);

	DPRINT_INFO(VMBUS_DRV, "generating uevent - VMBUS_DEVICE_CLASS_GUID={"
		    "%02x%02x%02x%02x-%02x%02x-%02x%02x-"
		    "%02x%02x%02x%02x%02x%02x%02x%02x}",
		    device_ctx->class_id.data[3], device_ctx->class_id.data[2],
		    device_ctx->class_id.data[1], device_ctx->class_id.data[0],
		    device_ctx->class_id.data[5], device_ctx->class_id.data[4],
		    device_ctx->class_id.data[7], device_ctx->class_id.data[6],
		    device_ctx->class_id.data[8], device_ctx->class_id.data[9],
		    device_ctx->class_id.data[10],
		    device_ctx->class_id.data[11],
		    device_ctx->class_id.data[12],
		    device_ctx->class_id.data[13],
		    device_ctx->class_id.data[14],
		    device_ctx->class_id.data[15]);

	ret = add_uevent_var(env, "VMBUS_DEVICE_CLASS_GUID={"
			     "%02x%02x%02x%02x-%02x%02x-%02x%02x-"
			     "%02x%02x%02x%02x%02x%02x%02x%02x}",
			     device_ctx->class_id.data[3],
			     device_ctx->class_id.data[2],
			     device_ctx->class_id.data[1],
			     device_ctx->class_id.data[0],
			     device_ctx->class_id.data[5],
			     device_ctx->class_id.data[4],
			     device_ctx->class_id.data[7],
			     device_ctx->class_id.data[6],
			     device_ctx->class_id.data[8],
			     device_ctx->class_id.data[9],
			     device_ctx->class_id.data[10],
			     device_ctx->class_id.data[11],
			     device_ctx->class_id.data[12],
			     device_ctx->class_id.data[13],
			     device_ctx->class_id.data[14],
			     device_ctx->class_id.data[15]);

	if (ret)
		return ret;

	ret = add_uevent_var(env, "VMBUS_DEVICE_DEVICE_GUID={"
			     "%02x%02x%02x%02x-%02x%02x-%02x%02x-"
			     "%02x%02x%02x%02x%02x%02x%02x%02x}",
			     device_ctx->device_id.data[3],
			     device_ctx->device_id.data[2],
			     device_ctx->device_id.data[1],
			     device_ctx->device_id.data[0],
			     device_ctx->device_id.data[5],
			     device_ctx->device_id.data[4],
			     device_ctx->device_id.data[7],
			     device_ctx->device_id.data[6],
			     device_ctx->device_id.data[8],
			     device_ctx->device_id.data[9],
			     device_ctx->device_id.data[10],
			     device_ctx->device_id.data[11],
			     device_ctx->device_id.data[12],
			     device_ctx->device_id.data[13],
			     device_ctx->device_id.data[14],
			     device_ctx->device_id.data[15]);
	if (ret)
		return ret;

	DPRINT_EXIT(VMBUS_DRV);

	return 0;
}


static int vmbus_match(struct device *device, struct device_driver *driver)
{
	int match = 0;
	struct driver_context *driver_ctx = driver_to_driver_context(driver);
	struct device_context *device_ctx = device_to_device_context(device);

	DPRINT_ENTER(VMBUS_DRV);

	
	if (memcmp(&device_ctx->class_id, &driver_ctx->class_id,
		   sizeof(struct hv_guid)) == 0) {
		
		struct vmbus_driver_context *vmbus_drv_ctx =
			(struct vmbus_driver_context *)driver_ctx;

		device_ctx->device_obj.Driver = &vmbus_drv_ctx->drv_obj.Base;
		DPRINT_INFO(VMBUS_DRV,
			    "device object (%p) set to driver object (%p)",
			    &device_ctx->device_obj,
			    device_ctx->device_obj.Driver);

		match = 1;
	}

	DPRINT_EXIT(VMBUS_DRV);

	return match;
}


static void vmbus_probe_failed_cb(struct work_struct *context)
{
	struct device_context *device_ctx = (struct device_context *)context;

	DPRINT_ENTER(VMBUS_DRV);

	
	device_unregister(&device_ctx->device);

	
	DPRINT_EXIT(VMBUS_DRV);
}


static int vmbus_probe(struct device *child_device)
{
	int ret = 0;
	struct driver_context *driver_ctx =
			driver_to_driver_context(child_device->driver);
	struct device_context *device_ctx =
			device_to_device_context(child_device);

	DPRINT_ENTER(VMBUS_DRV);

	
	if (driver_ctx->probe) {
		ret = device_ctx->probe_error = driver_ctx->probe(child_device);
		if (ret != 0) {
			DPRINT_ERR(VMBUS_DRV, "probe() failed for device %s "
				   "(%p) on driver %s (%d)...",
				   dev_name(child_device), child_device,
				   child_device->driver->name, ret);

			INIT_WORK(&device_ctx->probe_failed_work_item,
				  vmbus_probe_failed_cb);
			schedule_work(&device_ctx->probe_failed_work_item);
		}
	} else {
		DPRINT_ERR(VMBUS_DRV, "probe() method not set for driver - %s",
			   child_device->driver->name);
		ret = -1;
	}

	DPRINT_EXIT(VMBUS_DRV);
	return ret;
}


static int vmbus_remove(struct device *child_device)
{
	int ret;
	struct driver_context *driver_ctx;

	DPRINT_ENTER(VMBUS_DRV);

	
	if (child_device->parent == NULL) {
		
		DPRINT_EXIT(VMBUS_DRV);
		return 0;
	}

	if (child_device->driver) {
		driver_ctx = driver_to_driver_context(child_device->driver);

		
		if (driver_ctx->remove) {
			ret = driver_ctx->remove(child_device);
		} else {
			DPRINT_ERR(VMBUS_DRV,
				   "remove() method not set for driver - %s",
				   child_device->driver->name);
			ret = -1;
		}
	}

	DPRINT_EXIT(VMBUS_DRV);

	return 0;
}


static void vmbus_shutdown(struct device *child_device)
{
	struct driver_context *driver_ctx;

	DPRINT_ENTER(VMBUS_DRV);

	
	if (child_device->parent == NULL) {
		
		DPRINT_EXIT(VMBUS_DRV);
		return;
	}

	
	if (!child_device->driver) {
		DPRINT_EXIT(VMBUS_DRV);
		return;
	}

	driver_ctx = driver_to_driver_context(child_device->driver);

	
	if (driver_ctx->shutdown)
		driver_ctx->shutdown(child_device);

	DPRINT_EXIT(VMBUS_DRV);

	return;
}


static void vmbus_bus_release(struct device *device)
{
	DPRINT_ENTER(VMBUS_DRV);
	
	
	dev_err(device, "%s needs to be fixed!\n", __func__);
	WARN_ON(1);
	DPRINT_EXIT(VMBUS_DRV);
}


static void vmbus_device_release(struct device *device)
{
	struct device_context *device_ctx = device_to_device_context(device);

	DPRINT_ENTER(VMBUS_DRV);

	
	kfree(device_ctx);

	
	DPRINT_EXIT(VMBUS_DRV);

	return;
}


static void vmbus_msg_dpc(unsigned long data)
{
	struct vmbus_driver *vmbus_drv_obj = (struct vmbus_driver *)data;

	DPRINT_ENTER(VMBUS_DRV);

	ASSERT(vmbus_drv_obj->OnMsgDpc != NULL);

	
	vmbus_drv_obj->OnMsgDpc(&vmbus_drv_obj->Base);

	DPRINT_EXIT(VMBUS_DRV);
}


static void vmbus_event_dpc(unsigned long data)
{
	struct vmbus_driver *vmbus_drv_obj = (struct vmbus_driver *)data;

	DPRINT_ENTER(VMBUS_DRV);

	ASSERT(vmbus_drv_obj->OnEventDpc != NULL);

	
	vmbus_drv_obj->OnEventDpc(&vmbus_drv_obj->Base);

	DPRINT_EXIT(VMBUS_DRV);
}

static irqreturn_t vmbus_isr(int irq, void *dev_id)
{
	struct vmbus_driver *vmbus_driver_obj = &g_vmbus_drv.drv_obj;
	int ret;

	DPRINT_ENTER(VMBUS_DRV);

	ASSERT(vmbus_driver_obj->OnIsr != NULL);

	
	ret = vmbus_driver_obj->OnIsr(&vmbus_driver_obj->Base);

	
	if (ret > 0) {
		if (test_bit(0, (unsigned long *)&ret))
			tasklet_schedule(&g_vmbus_drv.msg_dpc);

		if (test_bit(1, (unsigned long *)&ret))
			tasklet_schedule(&g_vmbus_drv.event_dpc);

		DPRINT_EXIT(VMBUS_DRV);
		return IRQ_HANDLED;
	} else {
		DPRINT_EXIT(VMBUS_DRV);
		return IRQ_NONE;
	}
}

static int __init vmbus_init(void)
{
	int ret = 0;

	DPRINT_ENTER(VMBUS_DRV);

	DPRINT_INFO(VMBUS_DRV,
		"Vmbus initializing.... current log level 0x%x (%x,%x)",
		vmbus_loglevel, HIWORD(vmbus_loglevel), LOWORD(vmbus_loglevel));
	

	ret = vmbus_bus_init(VmbusInitialize);

	DPRINT_EXIT(VMBUS_DRV);
	return ret;
}

static void __exit vmbus_exit(void)
{
	DPRINT_ENTER(VMBUS_DRV);

	vmbus_bus_exit();
	
	DPRINT_EXIT(VMBUS_DRV);
	return;
}

MODULE_LICENSE("GPL");
module_param(vmbus_irq, int, S_IRUGO);
module_param(vmbus_loglevel, int, S_IRUGO);

module_init(vmbus_init);
module_exit(vmbus_exit);
