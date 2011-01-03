

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/acpi.h>
#include <acpi/acpi_bus.h>
#include <acpi/acpi_drivers.h>

#define _COMPONENT		ACPI_PCI_COMPONENT
ACPI_MODULE_NAME("pci_bind");

static int acpi_pci_unbind(struct acpi_device *device)
{
	struct pci_dev *dev;

	dev = acpi_get_pci_dev(device->handle);
	if (!dev || !dev->subordinate)
		goto out;

	acpi_pci_irq_del_prt(dev->subordinate);

	device->ops.bind = NULL;
	device->ops.unbind = NULL;

out:
	pci_dev_put(dev);
	return 0;
}

static int acpi_pci_bind(struct acpi_device *device)
{
	acpi_status status;
	acpi_handle handle;
	struct pci_bus *bus;
	struct pci_dev *dev;

	dev = acpi_get_pci_dev(device->handle);
	if (!dev)
		return 0;

	
	if (dev->subordinate) {
		ACPI_DEBUG_PRINT((ACPI_DB_INFO,
				  "Device %04x:%02x:%02x.%d is a PCI bridge\n",
				  pci_domain_nr(dev->bus), dev->bus->number,
				  PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn)));
		device->ops.bind = acpi_pci_bind;
		device->ops.unbind = acpi_pci_unbind;
	}

	
	status = acpi_get_handle(device->handle, METHOD_NAME__PRT, &handle);
	if (ACPI_FAILURE(status))
		goto out;

	if (dev->subordinate)
		bus = dev->subordinate;
	else
		bus = dev->bus;

	acpi_pci_irq_add_prt(device->handle, bus);

out:
	pci_dev_put(dev);
	return 0;
}

int acpi_pci_bind_root(struct acpi_device *device)
{
	device->ops.bind = acpi_pci_bind;
	device->ops.unbind = acpi_pci_unbind;

	return 0;
}
