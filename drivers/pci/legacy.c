#include <linux/init.h>
#include <linux/pci.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include "pci.h"


struct pci_dev *pci_find_device(unsigned int vendor, unsigned int device,
				struct pci_dev *from)
{
	struct pci_dev *pdev;

	pci_dev_get(from);
	pdev = pci_get_subsys(vendor, device, PCI_ANY_ID, PCI_ANY_ID, from);
	pci_dev_put(pdev);
	return pdev;
}
EXPORT_SYMBOL(pci_find_device);
