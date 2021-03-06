

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/gpio.h>

#include <asm/mach/pci.h>
#include <asm/mach-types.h>

#include <asm/hardware/it8152.h>

unsigned long it8152_base_address;
static int cmx2xx_it8152_irq_gpio;


void __init cmx2xx_pci_adjust_zones(int node, unsigned long *zone_size,
	unsigned long *zhole_size)
{
	unsigned int sz = SZ_64M >> PAGE_SHIFT;

	if (machine_is_armcore()) {
		pr_info("Adjusting zones for CM-X2XX\n");

		
		if (node || (zone_size[0] <= sz))
			return;

		zone_size[1] = zone_size[0] - sz;
		zone_size[0] = sz;
		zhole_size[1] = zhole_size[0];
		zhole_size[0] = 0;
	}
}

static void cmx2xx_it8152_irq_demux(unsigned int irq, struct irq_desc *desc)
{
	
	GEDR(cmx2xx_it8152_irq_gpio) = GPIO_bit(cmx2xx_it8152_irq_gpio);

	it8152_irq_demux(irq, desc);
}

void __cmx2xx_pci_init_irq(int irq_gpio)
{
	it8152_init_irq();

	cmx2xx_it8152_irq_gpio = irq_gpio;

	set_irq_type(gpio_to_irq(irq_gpio), IRQ_TYPE_EDGE_RISING);

	set_irq_chained_handler(gpio_to_irq(irq_gpio), cmx2xx_it8152_irq_demux);
}

#ifdef CONFIG_PM
static unsigned long sleep_save_ite[10];

void __cmx2xx_pci_suspend(void)
{
	
	sleep_save_ite[0] = __raw_readl(IT8152_INTC_PDCNIMR);
	sleep_save_ite[1] = __raw_readl(IT8152_INTC_LPCNIMR);
	sleep_save_ite[2] = __raw_readl(IT8152_INTC_LPNIAR);

	
	__raw_writel((0), IT8152_INTC_PDCNIRR);
	__raw_writel((0), IT8152_INTC_LPCNIRR);
}

void __cmx2xx_pci_resume(void)
{
	
	__raw_writel((sleep_save_ite[0]), IT8152_INTC_PDCNIMR);
	__raw_writel((sleep_save_ite[1]), IT8152_INTC_LPCNIMR);
	__raw_writel((sleep_save_ite[2]), IT8152_INTC_LPNIAR);
}
#else
void cmx2xx_pci_suspend(void) {}
void cmx2xx_pci_resume(void) {}
#endif


static int __init cmx2xx_pci_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	int irq;

	dev_dbg(&dev->dev, "%s: slot=%x, pin=%x\n", __func__, slot, pin);

	irq = it8152_pci_map_irq(dev, slot, pin);
	if (irq)
		return irq;

	
	
	if (slot == 7)
		return IT8152_PCI_INTA;

	
	if (slot == 8 || slot == 0)
		return IT8152_PCI_INTB;

	
	if (slot == 9)
		return IT8152_PCI_INTA;

	
	if (slot == 15)
		return IT8152_PCI_INTC;

	
	if (slot == 16)
		return IT8152_PCI_INTA;

	
	if ((slot == 17) || (slot == 19))
		return IT8152_PCI_INTA;
	if ((slot == 18) || (slot == 20))
		return IT8152_PCI_INTB;

	return(0);
}

static void cmx2xx_pci_preinit(void)
{
	pr_info("Initializing CM-X2XX PCI subsystem\n");

	__raw_writel(0x800, IT8152_PCI_CFG_ADDR);
	if (__raw_readl(IT8152_PCI_CFG_DATA) == 0x81521283) {
		pr_info("PCI Bridge found.\n");

		
		writel(0x848, IT8152_PCI_CFG_ADDR);
		writel(0, IT8152_PCI_CFG_DATA);

		
		writel(0x840, IT8152_PCI_CFG_ADDR);
		writel(0, IT8152_PCI_CFG_DATA);

		writel(0x20, IT8152_GPIO_GPDR);

		
		writel(0x4000, IT8152_PCI_CFG_ADDR);
		if (readl(IT8152_PCI_CFG_DATA) == 0xAC51104C) {
			pr_info("CardBus Bridge found.\n");

			
			writel(0x408C, IT8152_PCI_CFG_ADDR);
			writel(0x1022, IT8152_PCI_CFG_DATA);

			writel(0x4080, IT8152_PCI_CFG_ADDR);
			writel(0x3844d060, IT8152_PCI_CFG_DATA);

			writel(0x4090, IT8152_PCI_CFG_ADDR);
			writel(((readl(IT8152_PCI_CFG_DATA) & 0xffff) |
				0x60440000),
			       IT8152_PCI_CFG_DATA);

			writel(0x4018, IT8152_PCI_CFG_ADDR);
			writel(0xb0000000, IT8152_PCI_CFG_DATA);

			
			writel(0x418C, IT8152_PCI_CFG_ADDR);
			writel(0x1022, IT8152_PCI_CFG_DATA);

			writel(0x4180, IT8152_PCI_CFG_ADDR);
			writel(0x3844d060, IT8152_PCI_CFG_DATA);

			writel(0x4190, IT8152_PCI_CFG_ADDR);
			writel(((readl(IT8152_PCI_CFG_DATA) & 0xffff) |
				0x60440000),
			       IT8152_PCI_CFG_DATA);

			writel(0x4118, IT8152_PCI_CFG_ADDR);
			writel(0xb0000000, IT8152_PCI_CFG_DATA);
		}
	}
}

static struct hw_pci cmx2xx_pci __initdata = {
	.swizzle	= pci_std_swizzle,
	.map_irq	= cmx2xx_pci_map_irq,
	.nr_controllers	= 1,
	.setup		= it8152_pci_setup,
	.scan		= it8152_pci_scan_bus,
	.preinit	= cmx2xx_pci_preinit,
};

static int __init cmx2xx_init_pci(void)
{
	if (machine_is_armcore())
		pci_common_init(&cmx2xx_pci);

	return 0;
}

subsys_initcall(cmx2xx_init_pci);
