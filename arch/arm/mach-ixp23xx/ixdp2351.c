

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/serial.h>
#include <linux/tty.h>
#include <linux/bitops.h>
#include <linux/ioport.h>
#include <linux/serial_8250.h>
#include <linux/serial_core.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/pci.h>
#include <linux/mtd/physmap.h>

#include <asm/types.h>
#include <asm/setup.h>
#include <asm/memory.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/system.h>
#include <asm/tlbflush.h>
#include <asm/pgtable.h>

#include <asm/mach/map.h>
#include <asm/mach/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach/pci.h>


static void ixdp2351_inta_mask(unsigned int irq)
{
	*IXDP2351_CPLD_INTA_MASK_SET_REG = IXDP2351_INTA_IRQ_MASK(irq);
}

static void ixdp2351_inta_unmask(unsigned int irq)
{
	*IXDP2351_CPLD_INTA_MASK_CLR_REG = IXDP2351_INTA_IRQ_MASK(irq);
}

static void ixdp2351_inta_handler(unsigned int irq, struct irq_desc *desc)
{
	u16 ex_interrupt =
		*IXDP2351_CPLD_INTA_STAT_REG & IXDP2351_INTA_IRQ_VALID;
	int i;

	desc->chip->mask(irq);

	for (i = 0; i < IXDP2351_INTA_IRQ_NUM; i++) {
		if (ex_interrupt & (1 << i)) {
			int cpld_irq =
				IXP23XX_MACH_IRQ(IXDP2351_INTA_IRQ_BASE + i);
			generic_handle_irq(cpld_irq);
		}
	}

	desc->chip->unmask(irq);
}

static struct irq_chip ixdp2351_inta_chip = {
	.ack	= ixdp2351_inta_mask,
	.mask	= ixdp2351_inta_mask,
	.unmask	= ixdp2351_inta_unmask
};

static void ixdp2351_intb_mask(unsigned int irq)
{
	*IXDP2351_CPLD_INTB_MASK_SET_REG = IXDP2351_INTB_IRQ_MASK(irq);
}

static void ixdp2351_intb_unmask(unsigned int irq)
{
	*IXDP2351_CPLD_INTB_MASK_CLR_REG = IXDP2351_INTB_IRQ_MASK(irq);
}

static void ixdp2351_intb_handler(unsigned int irq, struct irq_desc *desc)
{
	u16 ex_interrupt =
		*IXDP2351_CPLD_INTB_STAT_REG & IXDP2351_INTB_IRQ_VALID;
	int i;

	desc->chip->ack(irq);

	for (i = 0; i < IXDP2351_INTB_IRQ_NUM; i++) {
		if (ex_interrupt & (1 << i)) {
			int cpld_irq =
				IXP23XX_MACH_IRQ(IXDP2351_INTB_IRQ_BASE + i);
			generic_handle_irq(cpld_irq);
		}
	}

	desc->chip->unmask(irq);
}

static struct irq_chip ixdp2351_intb_chip = {
	.ack	= ixdp2351_intb_mask,
	.mask	= ixdp2351_intb_mask,
	.unmask	= ixdp2351_intb_unmask
};

void __init ixdp2351_init_irq(void)
{
	int irq;

	
	*IXDP2351_CPLD_INTA_MASK_SET_REG = (u16) -1;
	*IXDP2351_CPLD_INTB_MASK_SET_REG = (u16) -1;
	*IXDP2351_CPLD_INTA_SIM_REG = 0;
	*IXDP2351_CPLD_INTB_SIM_REG = 0;

	ixp23xx_init_irq();

	for (irq = IXP23XX_MACH_IRQ(IXDP2351_INTA_IRQ_BASE);
	     irq <
	     IXP23XX_MACH_IRQ(IXDP2351_INTA_IRQ_BASE + IXDP2351_INTA_IRQ_NUM);
	     irq++) {
		if (IXDP2351_INTA_IRQ_MASK(irq) & IXDP2351_INTA_IRQ_VALID) {
			set_irq_flags(irq, IRQF_VALID);
			set_irq_handler(irq, handle_level_irq);
			set_irq_chip(irq, &ixdp2351_inta_chip);
		}
	}

	for (irq = IXP23XX_MACH_IRQ(IXDP2351_INTB_IRQ_BASE);
	     irq <
	     IXP23XX_MACH_IRQ(IXDP2351_INTB_IRQ_BASE + IXDP2351_INTB_IRQ_NUM);
	     irq++) {
		if (IXDP2351_INTB_IRQ_MASK(irq) & IXDP2351_INTB_IRQ_VALID) {
			set_irq_flags(irq, IRQF_VALID);
			set_irq_handler(irq, handle_level_irq);
			set_irq_chip(irq, &ixdp2351_intb_chip);
		}
	}

	set_irq_chained_handler(IRQ_IXP23XX_INTA, ixdp2351_inta_handler);
	set_irq_chained_handler(IRQ_IXP23XX_INTB, ixdp2351_intb_handler);
}




#define DEVPIN(dev, pin) ((pin) | ((dev) << 3))

static int __init ixdp2351_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	u8 bus = dev->bus->number;
	u32 devpin = DEVPIN(PCI_SLOT(dev->devfn), pin);
	struct pci_bus *tmp_bus = dev->bus;

	
	if (!bus)
		return -1;

	
	while ((tmp_bus->parent != NULL) && (tmp_bus->parent->parent != NULL))
		tmp_bus = tmp_bus->parent;

	
	switch (tmp_bus->self->devfn | (tmp_bus->self->bus->number << 8)) {
		
	case 0x0008:
		if (tmp_bus == dev->bus) {
			
			switch (devpin) {
				
			case DEVPIN(1, 1):	
				return IRQ_IXDP2351_INTA_82546;
			case DEVPIN(1, 2):	
				return IRQ_IXDP2351_INTB_82546;
				
			case DEVPIN(0, 1):	
			case DEVPIN(2, 4):	
				return IRQ_IXDP2351_SPCI_PMC_INTA;
			case DEVPIN(0, 2):	
			case DEVPIN(2, 1):	
				return IRQ_IXDP2351_SPCI_PMC_INTB;
			case DEVPIN(0, 3):	
			case DEVPIN(2, 2):	
				return IRQ_IXDP2351_SPCI_PMC_INTC;
			case DEVPIN(0, 4):	
			case DEVPIN(2, 3):	
				return IRQ_IXDP2351_SPCI_PMC_INTD;
			}
		} else {
			
			
			return -1;
		}
		break;
	case 0x0010:
		if (tmp_bus == dev->bus) {
			
			
			switch (devpin) {
			case DEVPIN(0, 1):	
			case DEVPIN(0, 2):
			case DEVPIN(0, 3):
			case DEVPIN(0, 4):
				return IRQ_IXDP2351_SPCI_DB_0;
			case DEVPIN(1, 1):	
			case DEVPIN(1, 2):
			case DEVPIN(1, 3):
			case DEVPIN(1, 4):
				return IRQ_IXDP2351_SPCI_DB_1;
			case DEVPIN(2, 1):	
			case DEVPIN(2, 2):
			case DEVPIN(2, 3):
			case DEVPIN(2, 4):
			case DEVPIN(3, 1):	
			case DEVPIN(3, 2):
			case DEVPIN(3, 3):
			case DEVPIN(3, 4):
				return IRQ_IXDP2351_SPCI_FIC;
			}
		} else {
			
			
			return -1;
		}
		break;
	}

	return -1;
}

struct hw_pci ixdp2351_pci __initdata = {
	.nr_controllers	= 1,
	.preinit	= ixp23xx_pci_preinit,
	.setup		= ixp23xx_pci_setup,
	.scan		= ixp23xx_pci_scan_bus,
	.map_irq	= ixdp2351_map_irq,
};

int __init ixdp2351_pci_init(void)
{
	if (machine_is_ixdp2351())
		pci_common_init(&ixdp2351_pci);

	return 0;
}

subsys_initcall(ixdp2351_pci_init);


static struct map_desc ixdp2351_io_desc[] __initdata = {
	{
		.virtual	= IXDP2351_NP_VIRT_BASE,
		.pfn		= __phys_to_pfn((u64)IXDP2351_NP_PHYS_BASE),
		.length		= IXDP2351_NP_PHYS_SIZE,
		.type		= MT_DEVICE
	}, {
		.virtual	= IXDP2351_BB_BASE_VIRT,
		.pfn		= __phys_to_pfn((u64)IXDP2351_BB_BASE_PHYS),
		.length		= IXDP2351_BB_SIZE,
		.type		= MT_DEVICE
	}
};

static void __init ixdp2351_map_io(void)
{
	ixp23xx_map_io();
	iotable_init(ixdp2351_io_desc, ARRAY_SIZE(ixdp2351_io_desc));
}

static struct physmap_flash_data ixdp2351_flash_data = {
	.width		= 1,
};

static struct resource ixdp2351_flash_resource = {
	.start		= 0x90000000,
	.end		= 0x93ffffff,
	.flags		= IORESOURCE_MEM,
};

static struct platform_device ixdp2351_flash = {
	.name		= "physmap-flash",
	.id		= 0,
	.dev		= {
		.platform_data	= &ixdp2351_flash_data,
	},
	.num_resources	= 1,
	.resource	= &ixdp2351_flash_resource,
};

static void __init ixdp2351_init(void)
{
	platform_device_register(&ixdp2351_flash);

	
	IXP23XX_EXP_CS0[0] |= IXP23XX_FLASH_WRITABLE;
	IXP23XX_EXP_CS0[1] |= IXP23XX_FLASH_WRITABLE;
	IXP23XX_EXP_CS0[2] |= IXP23XX_FLASH_WRITABLE;
	IXP23XX_EXP_CS0[3] |= IXP23XX_FLASH_WRITABLE;

	ixp23xx_sys_init();
}

MACHINE_START(IXDP2351, "Intel IXDP2351 Development Platform")
	
	.phys_io	= IXP23XX_PERIPHERAL_PHYS,
	.io_pg_offst	= ((IXP23XX_PERIPHERAL_VIRT >> 18)) & 0xfffc,
	.map_io		= ixdp2351_map_io,
	.init_irq	= ixdp2351_init_irq,
	.timer		= &ixp23xx_timer,
	.boot_params	= 0x00000100,
	.init_machine	= ixdp2351_init,
MACHINE_END
