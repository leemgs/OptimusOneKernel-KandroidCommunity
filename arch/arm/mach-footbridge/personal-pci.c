
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/init.h>

#include <asm/irq.h>
#include <asm/mach/pci.h>
#include <asm/mach-types.h>

static int irqmap_personal_server[] __initdata = {
	IRQ_IN0, IRQ_IN1, IRQ_IN2, IRQ_IN3, 0, 0, 0,
	IRQ_DOORBELLHOST, IRQ_DMA1, IRQ_DMA2, IRQ_PCI
};

static int __init personal_server_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	unsigned char line;

	pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &line);

	if (line > 0x40 && line <= 0x5f) {
		
		return irqmap_personal_server[(line & 0x1f) - 8];
	} else if (line == 0) {
		
		return 0;
	} else
		return irqmap_personal_server[(line - 1) & 3];
}

static struct hw_pci personal_server_pci __initdata = {
	.map_irq		= personal_server_map_irq,
	.nr_controllers		= 1,
	.setup			= dc21285_setup,
	.scan			= dc21285_scan_bus,
	.preinit		= dc21285_preinit,
	.postinit		= dc21285_postinit,
};

static int __init personal_pci_init(void)
{
	if (machine_is_personal_server())
		pci_common_init(&personal_server_pci);
	return 0;
}

subsys_initcall(personal_pci_init);
