

#include <linux/acpi.h>
#include <linux/pci.h>
#include <linux/pci_hotplug.h>
#include "pciehp.h"

#define PCIEHP_DETECT_PCIE	(0)
#define PCIEHP_DETECT_ACPI	(1)
#define PCIEHP_DETECT_AUTO	(2)
#define PCIEHP_DETECT_DEFAULT	PCIEHP_DETECT_AUTO

struct dummy_slot {
	u32 number;
	struct list_head list;
};

static int slot_detection_mode;
static char *pciehp_detect_mode;
module_param(pciehp_detect_mode, charp, 0444);
MODULE_PARM_DESC(pciehp_detect_mode,
	 "Slot detection mode: pcie, acpi, auto\n"
	 "  pcie          - Use PCIe based slot detection\n"
	 "  acpi          - Use ACPI for slot detection\n"
	 "  auto(default) - Auto select mode. Use acpi option if duplicate\n"
	 "                  slot ids are found. Otherwise, use pcie option\n");

int pciehp_acpi_slot_detection_check(struct pci_dev *dev)
{
	if (slot_detection_mode != PCIEHP_DETECT_ACPI)
		return 0;
	if (acpi_pci_detect_ejectable(DEVICE_ACPI_HANDLE(&dev->dev)))
		return 0;
	return -ENODEV;
}

static int __init parse_detect_mode(void)
{
	if (!pciehp_detect_mode)
		return PCIEHP_DETECT_DEFAULT;
	if (!strcmp(pciehp_detect_mode, "pcie"))
		return PCIEHP_DETECT_PCIE;
	if (!strcmp(pciehp_detect_mode, "acpi"))
		return PCIEHP_DETECT_ACPI;
	if (!strcmp(pciehp_detect_mode, "auto"))
		return PCIEHP_DETECT_AUTO;
	warn("bad specifier '%s' for pciehp_detect_mode. Use default\n",
	     pciehp_detect_mode);
	return PCIEHP_DETECT_DEFAULT;
}

static int __initdata dup_slot_id;
static int __initdata acpi_slot_detected;
static struct list_head __initdata dummy_slots = LIST_HEAD_INIT(dummy_slots);


static int __init dummy_probe(struct pcie_device *dev)
{
	int pos;
	u32 slot_cap;
	acpi_handle handle;
	struct dummy_slot *slot, *tmp;
	struct pci_dev *pdev = dev->port;
	
	if (pciehp_get_hp_hw_control_from_firmware(pdev))
		return -ENODEV;
	if (!(pos = pci_find_capability(pdev, PCI_CAP_ID_EXP)))
		return -ENODEV;
	pci_read_config_dword(pdev, pos + PCI_EXP_SLTCAP, &slot_cap);
	slot = kzalloc(sizeof(*slot), GFP_KERNEL);
	if (!slot)
		return -ENOMEM;
	slot->number = slot_cap >> 19;
	list_for_each_entry(tmp, &dummy_slots, list) {
		if (tmp->number == slot->number)
			dup_slot_id++;
	}
	list_add_tail(&slot->list, &dummy_slots);
	handle = DEVICE_ACPI_HANDLE(&pdev->dev);
	if (!acpi_slot_detected && acpi_pci_detect_ejectable(handle))
		acpi_slot_detected = 1;
	return -ENODEV;         
}

static struct pcie_port_service_driver __initdata dummy_driver = {
        .name           = "pciehp_dummy",
	.port_type	= PCIE_ANY_PORT,
	.service	= PCIE_PORT_SERVICE_HP,
        .probe          = dummy_probe,
};

static int __init select_detection_mode(void)
{
	struct dummy_slot *slot, *tmp;
	pcie_port_service_register(&dummy_driver);
	pcie_port_service_unregister(&dummy_driver);
	list_for_each_entry_safe(slot, tmp, &dummy_slots, list) {
		list_del(&slot->list);
		kfree(slot);
	}
	if (acpi_slot_detected && dup_slot_id)
		return PCIEHP_DETECT_ACPI;
	return PCIEHP_DETECT_PCIE;
}

void __init pciehp_acpi_slot_detection_init(void)
{
	slot_detection_mode = parse_detect_mode();
	if (slot_detection_mode != PCIEHP_DETECT_AUTO)
		goto out;
	slot_detection_mode = select_detection_mode();
out:
	if (slot_detection_mode == PCIEHP_DETECT_ACPI)
		info("Using ACPI for slot detection.\n");
}
