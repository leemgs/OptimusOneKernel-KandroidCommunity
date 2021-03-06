



#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <acpi/acpi_bus.h>
#include <acpi/acpi_drivers.h>
#include "sleep.h"

#define PREFIX "ACPI: "

#define _COMPONENT			ACPI_POWER_COMPONENT
ACPI_MODULE_NAME("power");
#define ACPI_POWER_CLASS		"power_resource"
#define ACPI_POWER_DEVICE_NAME		"Power Resource"
#define ACPI_POWER_FILE_INFO		"info"
#define ACPI_POWER_FILE_STATUS		"state"
#define ACPI_POWER_RESOURCE_STATE_OFF	0x00
#define ACPI_POWER_RESOURCE_STATE_ON	0x01
#define ACPI_POWER_RESOURCE_STATE_UNKNOWN 0xFF

int acpi_power_nocheck;
module_param_named(power_nocheck, acpi_power_nocheck, bool, 000);

static int acpi_power_add(struct acpi_device *device);
static int acpi_power_remove(struct acpi_device *device, int type);
static int acpi_power_resume(struct acpi_device *device);
static int acpi_power_open_fs(struct inode *inode, struct file *file);

static struct acpi_device_id power_device_ids[] = {
	{ACPI_POWER_HID, 0},
	{"", 0},
};
MODULE_DEVICE_TABLE(acpi, power_device_ids);

static struct acpi_driver acpi_power_driver = {
	.name = "power",
	.class = ACPI_POWER_CLASS,
	.ids = power_device_ids,
	.ops = {
		.add = acpi_power_add,
		.remove = acpi_power_remove,
		.resume = acpi_power_resume,
		},
};

struct acpi_power_reference {
	struct list_head node;
	struct acpi_device *device;
};

struct acpi_power_resource {
	struct acpi_device * device;
	acpi_bus_id name;
	u32 system_level;
	u32 order;
	struct mutex resource_lock;
	struct list_head reference;
};

static struct list_head acpi_power_resource_list;

static const struct file_operations acpi_power_fops = {
	.owner = THIS_MODULE,
	.open = acpi_power_open_fs,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};



static int
acpi_power_get_context(acpi_handle handle,
		       struct acpi_power_resource **resource)
{
	int result = 0;
	struct acpi_device *device = NULL;


	if (!resource)
		return -ENODEV;

	result = acpi_bus_get_device(handle, &device);
	if (result) {
		printk(KERN_WARNING PREFIX "Getting context [%p]\n", handle);
		return result;
	}

	*resource = acpi_driver_data(device);
	if (!*resource)
		return -ENODEV;

	return 0;
}

static int acpi_power_get_state(acpi_handle handle, int *state)
{
	acpi_status status = AE_OK;
	unsigned long long sta = 0;
	char node_name[5];
	struct acpi_buffer buffer = { sizeof(node_name), node_name };


	if (!handle || !state)
		return -EINVAL;

	status = acpi_evaluate_integer(handle, "_STA", NULL, &sta);
	if (ACPI_FAILURE(status))
		return -ENODEV;

	*state = (sta & 0x01)?ACPI_POWER_RESOURCE_STATE_ON:
			      ACPI_POWER_RESOURCE_STATE_OFF;

	acpi_get_name(handle, ACPI_SINGLE_NAME, &buffer);

	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Resource [%s] is %s\n",
			  node_name,
				*state ? "on" : "off"));

	return 0;
}

static int acpi_power_get_list_state(struct acpi_handle_list *list, int *state)
{
	int result = 0, state1;
	u32 i = 0;


	if (!list || !state)
		return -EINVAL;

	
	

	for (i = 0; i < list->count; i++) {
		
		result = acpi_power_get_state(list->handles[i], &state1);
		if (result)
			return result;

		*state = state1;

		if (*state != ACPI_POWER_RESOURCE_STATE_ON)
			break;
	}

	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Resource list is %s\n",
			  *state ? "on" : "off"));

	return result;
}

static int acpi_power_on(acpi_handle handle, struct acpi_device *dev)
{
	int result = 0;
	int found = 0;
	acpi_status status = AE_OK;
	struct acpi_power_resource *resource = NULL;
	struct list_head *node, *next;
	struct acpi_power_reference *ref;


	result = acpi_power_get_context(handle, &resource);
	if (result)
		return result;

	mutex_lock(&resource->resource_lock);
	list_for_each_safe(node, next, &resource->reference) {
		ref = container_of(node, struct acpi_power_reference, node);
		if (dev->handle == ref->device->handle) {
			ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Device [%s] already referenced by resource [%s]\n",
				  dev->pnp.bus_id, resource->name));
			found = 1;
			break;
		}
	}

	if (!found) {
		ref = kmalloc(sizeof (struct acpi_power_reference),
		    irqs_disabled() ? GFP_ATOMIC : GFP_KERNEL);
		if (!ref) {
			ACPI_DEBUG_PRINT((ACPI_DB_INFO, "kmalloc() failed\n"));
			mutex_unlock(&resource->resource_lock);
			return -ENOMEM;
		}
		list_add_tail(&ref->node, &resource->reference);
		ref->device = dev;
		ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Device [%s] added to resource [%s] references\n",
			  dev->pnp.bus_id, resource->name));
	}
	mutex_unlock(&resource->resource_lock);

	status = acpi_evaluate_object(resource->device->handle, "_ON", NULL, NULL);
	if (ACPI_FAILURE(status))
		return -ENODEV;

	
	resource->device->power.state = ACPI_STATE_D0;

	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Resource [%s] turned on\n",
			  resource->name));
	return 0;
}

static int acpi_power_off_device(acpi_handle handle, struct acpi_device *dev)
{
	int result = 0;
	acpi_status status = AE_OK;
	struct acpi_power_resource *resource = NULL;
	struct list_head *node, *next;
	struct acpi_power_reference *ref;


	result = acpi_power_get_context(handle, &resource);
	if (result)
		return result;

	mutex_lock(&resource->resource_lock);
	list_for_each_safe(node, next, &resource->reference) {
		ref = container_of(node, struct acpi_power_reference, node);
		if (dev->handle == ref->device->handle) {
			list_del(&ref->node);
			kfree(ref);
			ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Device [%s] removed from resource [%s] references\n",
			    dev->pnp.bus_id, resource->name));
			break;
		}
	}

	if (!list_empty(&resource->reference)) {
		ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Cannot turn resource [%s] off - resource is in use\n",
		    resource->name));
		mutex_unlock(&resource->resource_lock);
		return 0;
	}
	mutex_unlock(&resource->resource_lock);

	status = acpi_evaluate_object(resource->device->handle, "_OFF", NULL, NULL);
	if (ACPI_FAILURE(status))
		return -ENODEV;

	
	resource->device->power.state = ACPI_STATE_D3;

	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Resource [%s] turned off\n",
			  resource->name));

	return 0;
}


int acpi_device_sleep_wake(struct acpi_device *dev,
                           int enable, int sleep_state, int dev_state)
{
	union acpi_object in_arg[3];
	struct acpi_object_list arg_list = { 3, in_arg };
	acpi_status status = AE_OK;

	
	in_arg[0].type = ACPI_TYPE_INTEGER;
	in_arg[0].integer.value = enable;
	in_arg[1].type = ACPI_TYPE_INTEGER;
	in_arg[1].integer.value = sleep_state;
	in_arg[2].type = ACPI_TYPE_INTEGER;
	in_arg[2].integer.value = dev_state;
	status = acpi_evaluate_object(dev->handle, "_DSW", &arg_list, NULL);
	if (ACPI_SUCCESS(status)) {
		return 0;
	} else if (status != AE_NOT_FOUND) {
		printk(KERN_ERR PREFIX "_DSW execution failed\n");
		dev->wakeup.flags.valid = 0;
		return -ENODEV;
	}

	
	arg_list.count = 1;
	in_arg[0].integer.value = enable;
	status = acpi_evaluate_object(dev->handle, "_PSW", &arg_list, NULL);
	if (ACPI_FAILURE(status) && (status != AE_NOT_FOUND)) {
		printk(KERN_ERR PREFIX "_PSW execution failed\n");
		dev->wakeup.flags.valid = 0;
		return -ENODEV;
	}

	return 0;
}


int acpi_enable_wakeup_device_power(struct acpi_device *dev, int sleep_state)
{
	int i, err = 0;

	if (!dev || !dev->wakeup.flags.valid)
		return -EINVAL;

	mutex_lock(&acpi_device_lock);

	if (dev->wakeup.prepare_count++)
		goto out;

	
	for (i = 0; i < dev->wakeup.resources.count; i++) {
		int ret = acpi_power_on(dev->wakeup.resources.handles[i], dev);
		if (ret) {
			printk(KERN_ERR PREFIX "Transition power state\n");
			dev->wakeup.flags.valid = 0;
			err = -ENODEV;
			goto err_out;
		}
	}

	
	err = acpi_device_sleep_wake(dev, 1, sleep_state, 3);

 err_out:
	if (err)
		dev->wakeup.prepare_count = 0;

 out:
	mutex_unlock(&acpi_device_lock);
	return err;
}


int acpi_disable_wakeup_device_power(struct acpi_device *dev)
{
	int i, err = 0;

	if (!dev || !dev->wakeup.flags.valid)
		return -EINVAL;

	mutex_lock(&acpi_device_lock);

	if (--dev->wakeup.prepare_count > 0)
		goto out;

	
	if (dev->wakeup.prepare_count < 0)
		dev->wakeup.prepare_count = 0;

	err = acpi_device_sleep_wake(dev, 0, 0, 0);
	if (err)
		goto out;

	
	for (i = 0; i < dev->wakeup.resources.count; i++) {
		int ret = acpi_power_off_device(
				dev->wakeup.resources.handles[i], dev);
		if (ret) {
			printk(KERN_ERR PREFIX "Transition power state\n");
			dev->wakeup.flags.valid = 0;
			err = -ENODEV;
			goto out;
		}
	}

 out:
	mutex_unlock(&acpi_device_lock);
	return err;
}



int acpi_power_get_inferred_state(struct acpi_device *device)
{
	int result = 0;
	struct acpi_handle_list *list = NULL;
	int list_state = 0;
	int i = 0;


	if (!device)
		return -EINVAL;

	device->power.state = ACPI_STATE_UNKNOWN;

	
	for (i = ACPI_STATE_D0; i < ACPI_STATE_D3; i++) {
		list = &device->power.states[i].resources;
		if (list->count < 1)
			continue;

		result = acpi_power_get_list_state(list, &list_state);
		if (result)
			return result;

		if (list_state == ACPI_POWER_RESOURCE_STATE_ON) {
			device->power.state = i;
			return 0;
		}
	}

	device->power.state = ACPI_STATE_D3;

	return 0;
}

int acpi_power_transition(struct acpi_device *device, int state)
{
	int result = 0;
	struct acpi_handle_list *cl = NULL;	
	struct acpi_handle_list *tl = NULL;	
	int i = 0;


	if (!device || (state < ACPI_STATE_D0) || (state > ACPI_STATE_D3))
		return -EINVAL;

	if ((device->power.state < ACPI_STATE_D0)
	    || (device->power.state > ACPI_STATE_D3))
		return -ENODEV;

	cl = &device->power.states[device->power.state].resources;
	tl = &device->power.states[state].resources;

	

	
	for (i = 0; i < tl->count; i++) {
		result = acpi_power_on(tl->handles[i], device);
		if (result)
			goto end;
	}

	if (device->power.state == state) {
		goto end;
	}

	
	for (i = 0; i < cl->count; i++) {
		result = acpi_power_off_device(cl->handles[i], device);
		if (result)
			goto end;
	}

     end:
	if (result)
		device->power.state = ACPI_STATE_UNKNOWN;
	else {
	
		device->power.state = state;
	}

	return result;
}



static struct proc_dir_entry *acpi_power_dir;

static int acpi_power_seq_show(struct seq_file *seq, void *offset)
{
	int count = 0;
	int result = 0, state;
	struct acpi_power_resource *resource = NULL;
	struct list_head *node, *next;
	struct acpi_power_reference *ref;


	resource = seq->private;

	if (!resource)
		goto end;

	result = acpi_power_get_state(resource->device->handle, &state);
	if (result)
		goto end;

	seq_puts(seq, "state:                   ");
	switch (state) {
	case ACPI_POWER_RESOURCE_STATE_ON:
		seq_puts(seq, "on\n");
		break;
	case ACPI_POWER_RESOURCE_STATE_OFF:
		seq_puts(seq, "off\n");
		break;
	default:
		seq_puts(seq, "unknown\n");
		break;
	}

	mutex_lock(&resource->resource_lock);
	list_for_each_safe(node, next, &resource->reference) {
		ref = container_of(node, struct acpi_power_reference, node);
		count++;
	}
	mutex_unlock(&resource->resource_lock);

	seq_printf(seq, "system level:            S%d\n"
		   "order:                   %d\n"
		   "reference count:         %d\n",
		   resource->system_level,
		   resource->order, count);

      end:
	return 0;
}

static int acpi_power_open_fs(struct inode *inode, struct file *file)
{
	return single_open(file, acpi_power_seq_show, PDE(inode)->data);
}

static int acpi_power_add_fs(struct acpi_device *device)
{
	struct proc_dir_entry *entry = NULL;


	if (!device)
		return -EINVAL;

	if (!acpi_device_dir(device)) {
		acpi_device_dir(device) = proc_mkdir(acpi_device_bid(device),
						     acpi_power_dir);
		if (!acpi_device_dir(device))
			return -ENODEV;
	}

	
	entry = proc_create_data(ACPI_POWER_FILE_STATUS,
				 S_IRUGO, acpi_device_dir(device),
				 &acpi_power_fops, acpi_driver_data(device));
	if (!entry)
		return -EIO;
	return 0;
}

static int acpi_power_remove_fs(struct acpi_device *device)
{

	if (acpi_device_dir(device)) {
		remove_proc_entry(ACPI_POWER_FILE_STATUS,
				  acpi_device_dir(device));
		remove_proc_entry(acpi_device_bid(device), acpi_power_dir);
		acpi_device_dir(device) = NULL;
	}

	return 0;
}



static int acpi_power_add(struct acpi_device *device)
{
	int result = 0, state;
	acpi_status status = AE_OK;
	struct acpi_power_resource *resource = NULL;
	union acpi_object acpi_object;
	struct acpi_buffer buffer = { sizeof(acpi_object), &acpi_object };


	if (!device)
		return -EINVAL;

	resource = kzalloc(sizeof(struct acpi_power_resource), GFP_KERNEL);
	if (!resource)
		return -ENOMEM;

	resource->device = device;
	mutex_init(&resource->resource_lock);
	INIT_LIST_HEAD(&resource->reference);
	strcpy(resource->name, device->pnp.bus_id);
	strcpy(acpi_device_name(device), ACPI_POWER_DEVICE_NAME);
	strcpy(acpi_device_class(device), ACPI_POWER_CLASS);
	device->driver_data = resource;

	
	status = acpi_evaluate_object(device->handle, NULL, NULL, &buffer);
	if (ACPI_FAILURE(status)) {
		result = -ENODEV;
		goto end;
	}
	resource->system_level = acpi_object.power_resource.system_level;
	resource->order = acpi_object.power_resource.resource_order;

	result = acpi_power_get_state(device->handle, &state);
	if (result)
		goto end;

	switch (state) {
	case ACPI_POWER_RESOURCE_STATE_ON:
		device->power.state = ACPI_STATE_D0;
		break;
	case ACPI_POWER_RESOURCE_STATE_OFF:
		device->power.state = ACPI_STATE_D3;
		break;
	default:
		device->power.state = ACPI_STATE_UNKNOWN;
		break;
	}

	result = acpi_power_add_fs(device);
	if (result)
		goto end;

	printk(KERN_INFO PREFIX "%s [%s] (%s)\n", acpi_device_name(device),
	       acpi_device_bid(device), state ? "on" : "off");

      end:
	if (result)
		kfree(resource);

	return result;
}

static int acpi_power_remove(struct acpi_device *device, int type)
{
	struct acpi_power_resource *resource = NULL;
	struct list_head *node, *next;


	if (!device || !acpi_driver_data(device))
		return -EINVAL;

	resource = acpi_driver_data(device);

	acpi_power_remove_fs(device);

	mutex_lock(&resource->resource_lock);
	list_for_each_safe(node, next, &resource->reference) {
		struct acpi_power_reference *ref = container_of(node, struct acpi_power_reference, node);
		list_del(&ref->node);
		kfree(ref);
	}
	mutex_unlock(&resource->resource_lock);

	kfree(resource);

	return 0;
}

static int acpi_power_resume(struct acpi_device *device)
{
	int result = 0, state;
	struct acpi_power_resource *resource = NULL;
	struct acpi_power_reference *ref;

	if (!device || !acpi_driver_data(device))
		return -EINVAL;

	resource = acpi_driver_data(device);

	result = acpi_power_get_state(device->handle, &state);
	if (result)
		return result;

	mutex_lock(&resource->resource_lock);
	if (state == ACPI_POWER_RESOURCE_STATE_OFF &&
	    !list_empty(&resource->reference)) {
		ref = container_of(resource->reference.next, struct acpi_power_reference, node);
		mutex_unlock(&resource->resource_lock);
		result = acpi_power_on(device->handle, ref->device);
		return result;
	}

	mutex_unlock(&resource->resource_lock);
	return 0;
}

int __init acpi_power_init(void)
{
	int result = 0;

	INIT_LIST_HEAD(&acpi_power_resource_list);

	acpi_power_dir = proc_mkdir(ACPI_POWER_CLASS, acpi_root_dir);
	if (!acpi_power_dir)
		return -ENODEV;

	result = acpi_bus_register_driver(&acpi_power_driver);
	if (result < 0) {
		remove_proc_entry(ACPI_POWER_CLASS, acpi_root_dir);
		return -ENODEV;
	}

	return 0;
}
