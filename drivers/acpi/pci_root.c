

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include <linux/pm.h>
#include <linux/pci.h>
#include <linux/pci-acpi.h>
#include <linux/acpi.h>
#include <acpi/acpi_bus.h>
#include <acpi/acpi_drivers.h>

#define PREFIX "ACPI: "

#define _COMPONENT		ACPI_PCI_COMPONENT
ACPI_MODULE_NAME("pci_root");
#define ACPI_PCI_ROOT_CLASS		"pci_bridge"
#define ACPI_PCI_ROOT_DEVICE_NAME	"PCI Root Bridge"
static int acpi_pci_root_add(struct acpi_device *device);
static int acpi_pci_root_remove(struct acpi_device *device, int type);
static int acpi_pci_root_start(struct acpi_device *device);

static struct acpi_device_id root_device_ids[] = {
	{"PNP0A03", 0},
	{"", 0},
};
MODULE_DEVICE_TABLE(acpi, root_device_ids);

static struct acpi_driver acpi_pci_root_driver = {
	.name = "pci_root",
	.class = ACPI_PCI_ROOT_CLASS,
	.ids = root_device_ids,
	.ops = {
		.add = acpi_pci_root_add,
		.remove = acpi_pci_root_remove,
		.start = acpi_pci_root_start,
		},
};

static LIST_HEAD(acpi_pci_roots);

static struct acpi_pci_driver *sub_driver;
static DEFINE_MUTEX(osc_lock);

int acpi_pci_register_driver(struct acpi_pci_driver *driver)
{
	int n = 0;
	struct acpi_pci_root *root;

	struct acpi_pci_driver **pptr = &sub_driver;
	while (*pptr)
		pptr = &(*pptr)->next;
	*pptr = driver;

	if (!driver->add)
		return 0;

	list_for_each_entry(root, &acpi_pci_roots, node) {
		driver->add(root->device->handle);
		n++;
	}

	return n;
}

EXPORT_SYMBOL(acpi_pci_register_driver);

void acpi_pci_unregister_driver(struct acpi_pci_driver *driver)
{
	struct acpi_pci_root *root;

	struct acpi_pci_driver **pptr = &sub_driver;
	while (*pptr) {
		if (*pptr == driver)
			break;
		pptr = &(*pptr)->next;
	}
	BUG_ON(!*pptr);
	*pptr = (*pptr)->next;

	if (!driver->remove)
		return;

	list_for_each_entry(root, &acpi_pci_roots, node)
		driver->remove(root->device->handle);
}

EXPORT_SYMBOL(acpi_pci_unregister_driver);

acpi_handle acpi_get_pci_rootbridge_handle(unsigned int seg, unsigned int bus)
{
	struct acpi_pci_root *root;
	
	list_for_each_entry(root, &acpi_pci_roots, node)
		if ((root->segment == (u16) seg) && (root->bus_nr == (u16) bus))
			return root->device->handle;
	return NULL;		
}

EXPORT_SYMBOL_GPL(acpi_get_pci_rootbridge_handle);


int acpi_is_root_bridge(acpi_handle handle)
{
	int ret;
	struct acpi_device *device;

	ret = acpi_bus_get_device(handle, &device);
	if (ret)
		return 0;

	ret = acpi_match_device_ids(device, root_device_ids);
	if (ret)
		return 0;
	else
		return 1;
}
EXPORT_SYMBOL_GPL(acpi_is_root_bridge);

static acpi_status
get_root_bridge_busnr_callback(struct acpi_resource *resource, void *data)
{
	int *busnr = data;
	struct acpi_resource_address64 address;

	if (resource->type != ACPI_RESOURCE_TYPE_ADDRESS16 &&
	    resource->type != ACPI_RESOURCE_TYPE_ADDRESS32 &&
	    resource->type != ACPI_RESOURCE_TYPE_ADDRESS64)
		return AE_OK;

	acpi_resource_to_address64(resource, &address);
	if ((address.address_length > 0) &&
	    (address.resource_type == ACPI_BUS_NUMBER_RANGE))
		*busnr = address.minimum;

	return AE_OK;
}

static acpi_status try_get_root_bridge_busnr(acpi_handle handle,
					     unsigned long long *bus)
{
	acpi_status status;
	int busnum;

	busnum = -1;
	status =
	    acpi_walk_resources(handle, METHOD_NAME__CRS,
				get_root_bridge_busnr_callback, &busnum);
	if (ACPI_FAILURE(status))
		return status;
	
	if (busnum == -1)
		return AE_ERROR;
	*bus = busnum;
	return AE_OK;
}

static void acpi_pci_bridge_scan(struct acpi_device *device)
{
	int status;
	struct acpi_device *child = NULL;

	if (device->flags.bus_address)
		if (device->parent && device->parent->ops.bind) {
			status = device->parent->ops.bind(device);
			if (!status) {
				list_for_each_entry(child, &device->children, node)
					acpi_pci_bridge_scan(child);
			}
		}
}

static u8 OSC_UUID[16] = {0x5B, 0x4D, 0xDB, 0x33, 0xF7, 0x1F, 0x1C, 0x40,
			  0x96, 0x57, 0x74, 0x41, 0xC0, 0x3D, 0xD7, 0x66};

static acpi_status acpi_pci_run_osc(acpi_handle handle,
				    const u32 *capbuf, u32 *retval)
{
	acpi_status status;
	struct acpi_object_list input;
	union acpi_object in_params[4];
	struct acpi_buffer output = {ACPI_ALLOCATE_BUFFER, NULL};
	union acpi_object *out_obj;
	u32 errors;

	
	input.count = 4;
	input.pointer = in_params;
	in_params[0].type 		= ACPI_TYPE_BUFFER;
	in_params[0].buffer.length 	= 16;
	in_params[0].buffer.pointer	= OSC_UUID;
	in_params[1].type 		= ACPI_TYPE_INTEGER;
	in_params[1].integer.value 	= 1;
	in_params[2].type 		= ACPI_TYPE_INTEGER;
	in_params[2].integer.value	= 3;
	in_params[3].type		= ACPI_TYPE_BUFFER;
	in_params[3].buffer.length 	= 12;
	in_params[3].buffer.pointer 	= (u8 *)capbuf;

	status = acpi_evaluate_object(handle, "_OSC", &input, &output);
	if (ACPI_FAILURE(status))
		return status;

	if (!output.length)
		return AE_NULL_OBJECT;

	out_obj = output.pointer;
	if (out_obj->type != ACPI_TYPE_BUFFER) {
		printk(KERN_DEBUG "_OSC evaluation returned wrong type\n");
		status = AE_TYPE;
		goto out_kfree;
	}
	
	errors = *((u32 *)out_obj->buffer.pointer) & ~(1 << 0);
	if (errors) {
		if (errors & OSC_REQUEST_ERROR)
			printk(KERN_DEBUG "_OSC request failed\n");
		if (errors & OSC_INVALID_UUID_ERROR)
			printk(KERN_DEBUG "_OSC invalid UUID\n");
		if (errors & OSC_INVALID_REVISION_ERROR)
			printk(KERN_DEBUG "_OSC invalid revision\n");
		if (errors & OSC_CAPABILITIES_MASK_ERROR) {
			if (capbuf[OSC_QUERY_TYPE] & OSC_QUERY_ENABLE)
				goto out_success;
			printk(KERN_DEBUG
			       "Firmware did not grant requested _OSC control\n");
			status = AE_SUPPORT;
			goto out_kfree;
		}
		status = AE_ERROR;
		goto out_kfree;
	}
out_success:
	*retval = *((u32 *)(out_obj->buffer.pointer + 8));
	status = AE_OK;

out_kfree:
	kfree(output.pointer);
	return status;
}

static acpi_status acpi_pci_query_osc(struct acpi_pci_root *root, u32 flags)
{
	acpi_status status;
	u32 support_set, result, capbuf[3];

	
	support_set = root->osc_support_set | (flags & OSC_SUPPORT_MASKS);
	capbuf[OSC_QUERY_TYPE] = OSC_QUERY_ENABLE;
	capbuf[OSC_SUPPORT_TYPE] = support_set;
	capbuf[OSC_CONTROL_TYPE] = OSC_CONTROL_MASKS;

	status = acpi_pci_run_osc(root->device->handle, capbuf, &result);
	if (ACPI_SUCCESS(status)) {
		root->osc_support_set = support_set;
		root->osc_control_qry = result;
		root->osc_queried = 1;
	}
	return status;
}

static acpi_status acpi_pci_osc_support(struct acpi_pci_root *root, u32 flags)
{
	acpi_status status;
	acpi_handle tmp;

	status = acpi_get_handle(root->device->handle, "_OSC", &tmp);
	if (ACPI_FAILURE(status))
		return status;
	mutex_lock(&osc_lock);
	status = acpi_pci_query_osc(root, flags);
	mutex_unlock(&osc_lock);
	return status;
}

struct acpi_pci_root *acpi_pci_find_root(acpi_handle handle)
{
	struct acpi_pci_root *root;

	list_for_each_entry(root, &acpi_pci_roots, node) {
		if (root->device->handle == handle)
			return root;
	}
	return NULL;
}
EXPORT_SYMBOL_GPL(acpi_pci_find_root);

struct acpi_handle_node {
	struct list_head node;
	acpi_handle handle;
};


struct pci_dev *acpi_get_pci_dev(acpi_handle handle)
{
	int dev, fn;
	unsigned long long adr;
	acpi_status status;
	acpi_handle phandle;
	struct pci_bus *pbus;
	struct pci_dev *pdev = NULL;
	struct acpi_handle_node *node, *tmp;
	struct acpi_pci_root *root;
	LIST_HEAD(device_list);

	
	phandle = handle;
	while (!acpi_is_root_bridge(phandle)) {
		node = kzalloc(sizeof(struct acpi_handle_node), GFP_KERNEL);
		if (!node)
			goto out;

		INIT_LIST_HEAD(&node->node);
		node->handle = phandle;
		list_add(&node->node, &device_list);

		status = acpi_get_parent(phandle, &phandle);
		if (ACPI_FAILURE(status))
			goto out;
	}

	root = acpi_pci_find_root(phandle);
	if (!root)
		goto out;

	pbus = root->bus;

	
	list_for_each_entry(node, &device_list, node) {
		acpi_handle hnd = node->handle;
		status = acpi_evaluate_integer(hnd, "_ADR", NULL, &adr);
		if (ACPI_FAILURE(status))
			goto out;
		dev = (adr >> 16) & 0xffff;
		fn  = adr & 0xffff;

		pdev = pci_get_slot(pbus, PCI_DEVFN(dev, fn));
		if (!pdev || hnd == handle)
			break;

		pbus = pdev->subordinate;
		pci_dev_put(pdev);

		
		if (!pbus) {
			dev_dbg(&pdev->dev, "Not a PCI-to-PCI bridge\n");
			pdev = NULL;
			break;
		}
	}
out:
	list_for_each_entry_safe(node, tmp, &device_list, node)
		kfree(node);

	return pdev;
}
EXPORT_SYMBOL_GPL(acpi_get_pci_dev);


acpi_status acpi_pci_osc_control_set(acpi_handle handle, u32 flags)
{
	acpi_status status;
	u32 control_req, result, capbuf[3];
	acpi_handle tmp;
	struct acpi_pci_root *root;

	status = acpi_get_handle(handle, "_OSC", &tmp);
	if (ACPI_FAILURE(status))
		return status;

	control_req = (flags & OSC_CONTROL_MASKS);
	if (!control_req)
		return AE_TYPE;

	root = acpi_pci_find_root(handle);
	if (!root)
		return AE_NOT_EXIST;

	mutex_lock(&osc_lock);
	
	if ((root->osc_control_set & control_req) == control_req)
		goto out;

	
	if (!root->osc_queried) {
		status = acpi_pci_query_osc(root, root->osc_support_set);
		if (ACPI_FAILURE(status))
			goto out;
	}
	if ((root->osc_control_qry & control_req) != control_req) {
		printk(KERN_DEBUG
		       "Firmware did not grant requested _OSC control\n");
		status = AE_SUPPORT;
		goto out;
	}

	capbuf[OSC_QUERY_TYPE] = 0;
	capbuf[OSC_SUPPORT_TYPE] = root->osc_support_set;
	capbuf[OSC_CONTROL_TYPE] = root->osc_control_set | control_req;
	status = acpi_pci_run_osc(handle, capbuf, &result);
	if (ACPI_SUCCESS(status))
		root->osc_control_set = result;
out:
	mutex_unlock(&osc_lock);
	return status;
}
EXPORT_SYMBOL(acpi_pci_osc_control_set);

static int __devinit acpi_pci_root_add(struct acpi_device *device)
{
	unsigned long long segment, bus;
	acpi_status status;
	int result;
	struct acpi_pci_root *root;
	acpi_handle handle;
	struct acpi_device *child;
	u32 flags, base_flags;

	segment = 0;
	status = acpi_evaluate_integer(device->handle, METHOD_NAME__SEG, NULL,
				       &segment);
	if (ACPI_FAILURE(status) && status != AE_NOT_FOUND) {
		printk(KERN_ERR PREFIX "can't evaluate _SEG\n");
		return -ENODEV;
	}

	
	bus = 0;
	status = try_get_root_bridge_busnr(device->handle, &bus);
	if (ACPI_FAILURE(status)) {
		status = acpi_evaluate_integer(device->handle, METHOD_NAME__BBN,					       NULL, &bus);
		if (ACPI_FAILURE(status) && status != AE_NOT_FOUND) {
			printk(KERN_ERR PREFIX
			     "no bus number in _CRS and can't evaluate _BBN\n");
			return -ENODEV;
		}
	}

	root = kzalloc(sizeof(struct acpi_pci_root), GFP_KERNEL);
	if (!root)
		return -ENOMEM;

	INIT_LIST_HEAD(&root->node);
	root->device = device;
	root->segment = segment & 0xFFFF;
	root->bus_nr = bus & 0xFF;
	strcpy(acpi_device_name(device), ACPI_PCI_ROOT_DEVICE_NAME);
	strcpy(acpi_device_class(device), ACPI_PCI_ROOT_CLASS);
	device->driver_data = root;

	
	flags = base_flags = OSC_PCI_SEGMENT_GROUPS_SUPPORT;
	acpi_pci_osc_support(root, flags);

	

	
	list_add_tail(&root->node, &acpi_pci_roots);

	printk(KERN_INFO PREFIX "%s [%s] (%04x:%02x)\n",
	       acpi_device_name(device), acpi_device_bid(device),
	       root->segment, root->bus_nr);

	
	root->bus = pci_acpi_scan_root(device, segment, bus);
	if (!root->bus) {
		printk(KERN_ERR PREFIX
			    "Bus %04x:%02x not present in PCI namespace\n",
			    root->segment, root->bus_nr);
		result = -ENODEV;
		goto end;
	}

	
	result = acpi_pci_bind_root(device);
	if (result)
		goto end;

	
	status = acpi_get_handle(device->handle, METHOD_NAME__PRT, &handle);
	if (ACPI_SUCCESS(status))
		result = acpi_pci_irq_add_prt(device->handle, root->bus);

	
	list_for_each_entry(child, &device->children, node)
		acpi_pci_bridge_scan(child);

	
	if (pci_ext_cfg_avail(root->bus->self))
		flags |= OSC_EXT_PCI_CONFIG_SUPPORT;
	if (pcie_aspm_enabled())
		flags |= OSC_ACTIVE_STATE_PWR_SUPPORT |
			OSC_CLOCK_PWR_CAPABILITY_SUPPORT;
	if (pci_msi_enabled())
		flags |= OSC_MSI_SUPPORT;
	if (flags != base_flags)
		acpi_pci_osc_support(root, flags);

	return 0;

end:
	if (!list_empty(&root->node))
		list_del(&root->node);
	kfree(root);
	return result;
}

static int acpi_pci_root_start(struct acpi_device *device)
{
	struct acpi_pci_root *root = acpi_driver_data(device);

	pci_bus_add_devices(root->bus);
	return 0;
}

static int acpi_pci_root_remove(struct acpi_device *device, int type)
{
	struct acpi_pci_root *root = acpi_driver_data(device);

	kfree(root);
	return 0;
}

static int __init acpi_pci_root_init(void)
{
	if (acpi_pci_disabled)
		return 0;

	if (acpi_bus_register_driver(&acpi_pci_root_driver) < 0)
		return -ENODEV;

	return 0;
}

subsys_initcall(acpi_pci_root_init);
