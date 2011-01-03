

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/pm.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/pcieport_if.h>

#include "../pci.h"
#include "portdrv.h"


static void release_pcie_device(struct device *dev)
{
	kfree(to_pcie_device(dev));			
}


static int pcie_port_msix_add_entry(
	struct msix_entry *entries, int new_entry, int nr_entries)
{
	int j;

	for (j = 0; j < nr_entries; j++)
		if (entries[j].entry == new_entry)
			return j;

	entries[j].entry = new_entry;
	return j;
}


static int pcie_port_enable_msix(struct pci_dev *dev, int *vectors, int mask)
{
	struct msix_entry *msix_entries;
	int idx[PCIE_PORT_DEVICE_MAXSERVICES];
	int nr_entries, status, pos, i, nvec;
	u16 reg16;
	u32 reg32;

	nr_entries = pci_msix_table_size(dev);
	if (!nr_entries)
		return -EINVAL;
	if (nr_entries > PCIE_PORT_MAX_MSIX_ENTRIES)
		nr_entries = PCIE_PORT_MAX_MSIX_ENTRIES;

	msix_entries = kzalloc(sizeof(*msix_entries) * nr_entries, GFP_KERNEL);
	if (!msix_entries)
		return -ENOMEM;

	
	for (i = 0; i < nr_entries; i++)
		msix_entries[i].entry = i;

	status = pci_enable_msix(dev, msix_entries, nr_entries);
	if (status)
		goto Exit;

	for (i = 0; i < PCIE_PORT_DEVICE_MAXSERVICES; i++)
		idx[i] = -1;
	status = -EIO;
	nvec = 0;

	if (mask & (PCIE_PORT_SERVICE_PME | PCIE_PORT_SERVICE_HP)) {
		int entry;

		
		pos = pci_find_capability(dev, PCI_CAP_ID_EXP);
		pci_read_config_word(dev, pos + PCIE_CAPABILITIES_REG, &reg16);
		entry = (reg16 >> 9) & PCIE_PORT_MSI_VECTOR_MASK;
		if (entry >= nr_entries)
			goto Error;

		i = pcie_port_msix_add_entry(msix_entries, entry, nvec);
		if (i == nvec)
			nvec++;

		idx[PCIE_PORT_SERVICE_PME_SHIFT] = i;
		idx[PCIE_PORT_SERVICE_HP_SHIFT] = i;
	}

	if (mask & PCIE_PORT_SERVICE_AER) {
		int entry;

		
		pos = pci_find_ext_capability(dev, PCI_EXT_CAP_ID_ERR);
		pci_read_config_dword(dev, pos + PCI_ERR_ROOT_STATUS, &reg32);
		entry = reg32 >> 27;
		if (entry >= nr_entries)
			goto Error;

		i = pcie_port_msix_add_entry(msix_entries, entry, nvec);
		if (i == nvec)
			nvec++;

		idx[PCIE_PORT_SERVICE_AER_SHIFT] = i;
	}

	
	if (nvec == nr_entries) {
		status = 0;
	} else {
		
		pci_disable_msix(dev);

		
		status = pci_enable_msix(dev, msix_entries, nvec);
		if (status)
			goto Exit;
	}

	for (i = 0; i < PCIE_PORT_DEVICE_MAXSERVICES; i++)
		vectors[i] = idx[i] >= 0 ? msix_entries[idx[i]].vector : -1;

 Exit:
	kfree(msix_entries);
	return status;

 Error:
	pci_disable_msix(dev);
	goto Exit;
}


static int assign_interrupt_mode(struct pci_dev *dev, int *vectors, int mask)
{
	int irq, interrupt_mode = PCIE_PORT_NO_IRQ;
	int i;

	
	if (!pcie_port_enable_msix(dev, vectors, mask))
		return PCIE_PORT_MSIX_MODE;

	
	if (!pci_enable_msi(dev))
		interrupt_mode = PCIE_PORT_MSI_MODE;

	if (interrupt_mode == PCIE_PORT_NO_IRQ && dev->pin)
		interrupt_mode = PCIE_PORT_INTx_MODE;

	irq = interrupt_mode != PCIE_PORT_NO_IRQ ? dev->irq : -1;
	for (i = 0; i < PCIE_PORT_DEVICE_MAXSERVICES; i++)
		vectors[i] = irq;

	vectors[PCIE_PORT_SERVICE_VC_SHIFT] = -1;

	return interrupt_mode;
}


static int get_port_device_capability(struct pci_dev *dev)
{
	int services = 0, pos;
	u16 reg16;
	u32 reg32;

	pos = pci_find_capability(dev, PCI_CAP_ID_EXP);
	pci_read_config_word(dev, pos + PCIE_CAPABILITIES_REG, &reg16);
	
	if (reg16 & PORT_TO_SLOT_MASK) {
		pci_read_config_dword(dev, 
			pos + PCIE_SLOT_CAPABILITIES_REG, &reg32);
		if (reg32 & SLOT_HP_CAPABLE_MASK)
			services |= PCIE_PORT_SERVICE_HP;
	}
	
	if (pci_find_ext_capability(dev, PCI_EXT_CAP_ID_ERR))
		services |= PCIE_PORT_SERVICE_AER;
	
	if (pci_find_ext_capability(dev, PCI_EXT_CAP_ID_VC))
		services |= PCIE_PORT_SERVICE_VC;

	return services;
}


static void pcie_device_init(struct pci_dev *parent, struct pcie_device *dev, 
	int service_type, int irq)
{
	struct pcie_port_data *port_data = pci_get_drvdata(parent);
	struct device *device;
	int port_type = port_data->port_type;

	dev->port = parent;
	dev->irq = irq;
	dev->service = service_type;

	
	device = &dev->device;
	memset(device, 0, sizeof(struct device));
	device->bus = &pcie_port_bus_type;
	device->driver = NULL;
	dev_set_drvdata(device, NULL);
	device->release = release_pcie_device;	
	dev_set_name(device, "%s:pcie%02x",
		 pci_name(parent), get_descriptor_id(port_type, service_type));
	device->parent = &parent->dev;
}


static struct pcie_device* alloc_pcie_device(struct pci_dev *parent,
	int service_type, int irq)
{
	struct pcie_device *device;

	device = kzalloc(sizeof(struct pcie_device), GFP_KERNEL);
	if (!device)
		return NULL;

	pcie_device_init(parent, device, service_type, irq);
	return device;
}


int pcie_port_device_probe(struct pci_dev *dev)
{
	int pos, type;
	u16 reg;

	if (!(pos = pci_find_capability(dev, PCI_CAP_ID_EXP)))
		return -ENODEV;

	pci_read_config_word(dev, pos + PCIE_CAPABILITIES_REG, &reg);
	type = (reg >> 4) & PORT_TYPE_MASK;
	if (	type == PCIE_RC_PORT || type == PCIE_SW_UPSTREAM_PORT ||
		type == PCIE_SW_DOWNSTREAM_PORT )
		return 0;

	return -ENODEV;
}


int pcie_port_device_register(struct pci_dev *dev)
{
	struct pcie_port_data *port_data;
	int status, capabilities, irq_mode, i, nr_serv;
	int vectors[PCIE_PORT_DEVICE_MAXSERVICES];
	u16 reg16;

	port_data = kzalloc(sizeof(*port_data), GFP_KERNEL);
	if (!port_data)
		return -ENOMEM;
	pci_set_drvdata(dev, port_data);

	
	pci_read_config_word(dev,
		pci_find_capability(dev, PCI_CAP_ID_EXP) +
		PCIE_CAPABILITIES_REG, &reg16);
	port_data->port_type = (reg16 >> 4) & PORT_TYPE_MASK;

	capabilities = get_port_device_capability(dev);
	
	if (port_data->port_type == PCIE_RC_PORT)
		capabilities |= PCIE_PORT_SERVICE_PME;

	irq_mode = assign_interrupt_mode(dev, vectors, capabilities);
	if (irq_mode == PCIE_PORT_NO_IRQ) {
		
		if (!(capabilities & PCIE_PORT_SERVICE_VC)) {
			status = -ENODEV;
			goto Error;
		}
		capabilities = PCIE_PORT_SERVICE_VC;
	}
	port_data->port_irq_mode = irq_mode;

	status = pci_enable_device(dev);
	if (status)
		goto Error;
	pci_set_master(dev);

	
	for (i = 0, nr_serv = 0; i < PCIE_PORT_DEVICE_MAXSERVICES; i++) {
		struct pcie_device *child;
		int service = 1 << i;

		if (!(capabilities & service))
			continue;

		child = alloc_pcie_device(dev, service, vectors[i]);
		if (!child)
			continue;

		status = device_register(&child->device);
		if (status) {
			kfree(child);
			continue;
		}

		get_device(&child->device);
		nr_serv++;
	}
	if (!nr_serv) {
		pci_disable_device(dev);
		status = -ENODEV;
		goto Error;
	}

	return 0;

 Error:
	kfree(port_data);
	return status;
}

#ifdef CONFIG_PM
static int suspend_iter(struct device *dev, void *data)
{
	struct pcie_port_service_driver *service_driver;

 	if ((dev->bus == &pcie_port_bus_type) &&
 	    (dev->driver)) {
 		service_driver = to_service_driver(dev->driver);
 		if (service_driver->suspend)
 			service_driver->suspend(to_pcie_device(dev));
  	}
	return 0;
}


int pcie_port_device_suspend(struct device *dev)
{
	return device_for_each_child(dev, NULL, suspend_iter);
}

static int resume_iter(struct device *dev, void *data)
{
	struct pcie_port_service_driver *service_driver;

	if ((dev->bus == &pcie_port_bus_type) &&
	    (dev->driver)) {
		service_driver = to_service_driver(dev->driver);
		if (service_driver->resume)
			service_driver->resume(to_pcie_device(dev));
	}
	return 0;
}


int pcie_port_device_resume(struct device *dev)
{
	return device_for_each_child(dev, NULL, resume_iter);
}
#endif 

static int remove_iter(struct device *dev, void *data)
{
	if (dev->bus == &pcie_port_bus_type) {
		put_device(dev);
		device_unregister(dev);
	}
	return 0;
}


void pcie_port_device_remove(struct pci_dev *dev)
{
	struct pcie_port_data *port_data = pci_get_drvdata(dev);

	device_for_each_child(&dev->dev, NULL, remove_iter);
	pci_disable_device(dev);

	switch (port_data->port_irq_mode) {
	case PCIE_PORT_MSIX_MODE:
		pci_disable_msix(dev);
		break;
	case PCIE_PORT_MSI_MODE:
		pci_disable_msi(dev);
		break;
	}

	kfree(port_data);
}


static int pcie_port_probe_service(struct device *dev)
{
	struct pcie_device *pciedev;
	struct pcie_port_service_driver *driver;
	int status;

	if (!dev || !dev->driver)
		return -ENODEV;

	driver = to_service_driver(dev->driver);
	if (!driver || !driver->probe)
		return -ENODEV;

	pciedev = to_pcie_device(dev);
	status = driver->probe(pciedev);
	if (!status) {
		dev_printk(KERN_DEBUG, dev, "service driver %s loaded\n",
			driver->name);
		get_device(dev);
	}
	return status;
}


static int pcie_port_remove_service(struct device *dev)
{
	struct pcie_device *pciedev;
	struct pcie_port_service_driver *driver;

	if (!dev || !dev->driver)
		return 0;

	pciedev = to_pcie_device(dev);
	driver = to_service_driver(dev->driver);
	if (driver && driver->remove) {
		dev_printk(KERN_DEBUG, dev, "unloading service driver %s\n",
			driver->name);
		driver->remove(pciedev);
		put_device(dev);
	}
	return 0;
}


static void pcie_port_shutdown_service(struct device *dev) {}


int pcie_port_service_register(struct pcie_port_service_driver *new)
{
	new->driver.name = (char *)new->name;
	new->driver.bus = &pcie_port_bus_type;
	new->driver.probe = pcie_port_probe_service;
	new->driver.remove = pcie_port_remove_service;
	new->driver.shutdown = pcie_port_shutdown_service;

	return driver_register(&new->driver);
}


void pcie_port_service_unregister(struct pcie_port_service_driver *drv)
{
	driver_unregister(&drv->driver);
}

EXPORT_SYMBOL(pcie_port_service_register);
EXPORT_SYMBOL(pcie_port_service_unregister);
