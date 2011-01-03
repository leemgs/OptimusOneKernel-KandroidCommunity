

#include <linux/mm.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/pm.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>
#include <linux/mtd/physmap.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <mach/hardware.h>
#include <asm/cputype.h>
#include <asm/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/pci.h>
#include <asm/mach/time.h>
#include <asm/mach-types.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <mach/time.h>


static int force_ep80219;

static int is_80219(void)
{
	return !!((read_cpuid_id() & 0xffffffe0) == 0x69052e20);
}

static int is_ep80219(void)
{
	if (machine_is_ep80219() || force_ep80219)
		return 1;
	else
		return 0;
}



static void __init iq31244_timer_init(void)
{
	if (is_ep80219()) {
		
		iop_init_time(200000000);
	} else {
		
		iop_init_time(198000000);
	}
}

static struct sys_timer iq31244_timer = {
	.init		= iq31244_timer_init,
	.offset		= iop_gettimeoffset,
};



static struct map_desc iq31244_io_desc[] __initdata = {
	{	
		.virtual	= IQ31244_UART,
		.pfn		= __phys_to_pfn(IQ31244_UART),
		.length		= 0x00100000,
		.type		= MT_DEVICE,
	},
};

void __init iq31244_map_io(void)
{
	iop3xx_map_io();
	iotable_init(iq31244_io_desc, ARRAY_SIZE(iq31244_io_desc));
}



static int __init
ep80219_pci_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	int irq;

	if (slot == 0) {
		
		irq = IRQ_IOP32X_XINT1;
	} else if (slot == 1) {
		
		irq = IRQ_IOP32X_XINT0;
	} else if (slot == 2) {
		
		irq = IRQ_IOP32X_XINT3;
	} else if (slot == 3) {
		
		irq = IRQ_IOP32X_XINT2;
	} else {
		printk(KERN_ERR "ep80219_pci_map_irq() called for unknown "
			"device PCI:%d:%d:%d\n", dev->bus->number,
			PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn));
		irq = -1;
	}

	return irq;
}

static struct hw_pci ep80219_pci __initdata = {
	.swizzle	= pci_std_swizzle,
	.nr_controllers = 1,
	.setup		= iop3xx_pci_setup,
	.preinit	= iop3xx_pci_preinit,
	.scan		= iop3xx_pci_scan_bus,
	.map_irq	= ep80219_pci_map_irq,
};

static int __init
iq31244_pci_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	int irq;

	if (slot == 0) {
		
		irq = IRQ_IOP32X_XINT1;
	} else if (slot == 1) {
		
		irq = IRQ_IOP32X_XINT2;
	} else if (slot == 2) {
		
		irq = IRQ_IOP32X_XINT3;
	} else if (slot == 3) {
		
		irq = IRQ_IOP32X_XINT0;
	} else {
		printk(KERN_ERR "iq31244_pci_map_irq called for unknown "
			"device PCI:%d:%d:%d\n", dev->bus->number,
			PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn));
		irq = -1;
	}

	return irq;
}

static struct hw_pci iq31244_pci __initdata = {
	.swizzle	= pci_std_swizzle,
	.nr_controllers = 1,
	.setup		= iop3xx_pci_setup,
	.preinit	= iop3xx_pci_preinit,
	.scan		= iop3xx_pci_scan_bus,
	.map_irq	= iq31244_pci_map_irq,
};

static int __init iq31244_pci_init(void)
{
	if (is_ep80219())
		pci_common_init(&ep80219_pci);
	else if (machine_is_iq31244()) {
		if (is_80219()) {
			printk("note: iq31244 board type has been selected\n");
			printk("note: to select ep80219 operation:\n");
			printk("\t1/ specify \"force_ep80219\" on the kernel"
				" command line\n");
			printk("\t2/ update boot loader to pass"
				" the ep80219 id: %d\n", MACH_TYPE_EP80219);
		}
		pci_common_init(&iq31244_pci);
	}

	return 0;
}

subsys_initcall(iq31244_pci_init);



static struct physmap_flash_data iq31244_flash_data = {
	.width		= 2,
};

static struct resource iq31244_flash_resource = {
	.start		= 0xf0000000,
	.end		= 0xf07fffff,
	.flags		= IORESOURCE_MEM,
};

static struct platform_device iq31244_flash_device = {
	.name		= "physmap-flash",
	.id		= 0,
	.dev		= {
		.platform_data	= &iq31244_flash_data,
	},
	.num_resources	= 1,
	.resource	= &iq31244_flash_resource,
};

static struct plat_serial8250_port iq31244_serial_port[] = {
	{
		.mapbase	= IQ31244_UART,
		.membase	= (char *)IQ31244_UART,
		.irq		= IRQ_IOP32X_XINT1,
		.flags		= UPF_SKIP_TEST,
		.iotype		= UPIO_MEM,
		.regshift	= 0,
		.uartclk	= 1843200,
	},
	{ },
};

static struct resource iq31244_uart_resource = {
	.start		= IQ31244_UART,
	.end		= IQ31244_UART + 7,
	.flags		= IORESOURCE_MEM,
};

static struct platform_device iq31244_serial_device = {
	.name		= "serial8250",
	.id		= PLAT8250_DEV_PLATFORM,
	.dev		= {
		.platform_data		= iq31244_serial_port,
	},
	.num_resources	= 1,
	.resource	= &iq31244_uart_resource,
};


void ep80219_power_off(void)
{
	
	*IOP3XX_IDBR1 = 0x60;
	*IOP3XX_ICR1 = 0xE9;
	mdelay(1);

	
	*IOP3XX_IDBR1 = 0x0F;
	*IOP3XX_ICR1 = 0xE8;
	mdelay(1);

	
	*IOP3XX_IDBR1 = 0x03;
	*IOP3XX_ICR1 = 0xE8;
	mdelay(1);

	
	*IOP3XX_IDBR1 = 0x00;
	*IOP3XX_ICR1 = 0xEA;

	while (1)
		;
}

static void __init iq31244_init_machine(void)
{
	platform_device_register(&iop3xx_i2c0_device);
	platform_device_register(&iop3xx_i2c1_device);
	platform_device_register(&iq31244_flash_device);
	platform_device_register(&iq31244_serial_device);
	platform_device_register(&iop3xx_dma_0_channel);
	platform_device_register(&iop3xx_dma_1_channel);

	if (is_ep80219())
		pm_power_off = ep80219_power_off;

	if (!is_80219())
		platform_device_register(&iop3xx_aau_channel);
}

static int __init force_ep80219_setup(char *str)
{
	force_ep80219 = 1;
	return 1;
}

__setup("force_ep80219", force_ep80219_setup);

MACHINE_START(IQ31244, "Intel IQ31244")
	
	.phys_io	= IQ31244_UART,
	.io_pg_offst	= ((IQ31244_UART) >> 18) & 0xfffc,
	.boot_params	= 0xa0000100,
	.map_io		= iq31244_map_io,
	.init_irq	= iop32x_init_irq,
	.timer		= &iq31244_timer,
	.init_machine	= iq31244_init_machine,
MACHINE_END


MACHINE_START(EP80219, "Intel EP80219")
	
	.phys_io	= IQ31244_UART,
	.io_pg_offst	= ((IQ31244_UART) >> 18) & 0xfffc,
	.boot_params	= 0xa0000100,
	.map_io		= iq31244_map_io,
	.init_irq	= iop32x_init_irq,
	.timer		= &iq31244_timer,
	.init_machine	= iq31244_init_machine,
MACHINE_END
