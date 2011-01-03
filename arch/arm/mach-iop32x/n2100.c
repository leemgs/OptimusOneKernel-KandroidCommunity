

#include <linux/mm.h>
#include <linux/init.h>
#include <linux/f75375s.h>
#include <linux/leds-pca9532.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/pm.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>
#include <linux/mtd/physmap.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/io.h>
#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/pci.h>
#include <asm/mach/time.h>
#include <asm/mach-types.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <mach/time.h>


static void __init n2100_timer_init(void)
{
	
	iop_init_time(198000000);
}

static struct sys_timer n2100_timer = {
	.init		= n2100_timer_init,
	.offset		= iop_gettimeoffset,
};



static struct map_desc n2100_io_desc[] __initdata = {
	{	
		.virtual	= N2100_UART,
		.pfn		= __phys_to_pfn(N2100_UART),
		.length		= 0x00100000,
		.type		= MT_DEVICE
	},
};

void __init n2100_map_io(void)
{
	iop3xx_map_io();
	iotable_init(n2100_io_desc, ARRAY_SIZE(n2100_io_desc));
}



static int __init
n2100_pci_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	int irq;

	if (PCI_SLOT(dev->devfn) == 1) {
		
		irq = IRQ_IOP32X_XINT0;
	} else if (PCI_SLOT(dev->devfn) == 2) {
		
		irq = IRQ_IOP32X_XINT3;
	} else if (PCI_SLOT(dev->devfn) == 3) {
		
		irq = IRQ_IOP32X_XINT2;
	} else if (PCI_SLOT(dev->devfn) == 4 && pin == 1) {
		
		irq = IRQ_IOP32X_XINT1;
	} else if (PCI_SLOT(dev->devfn) == 4 && pin == 2) {
		
		irq = IRQ_IOP32X_XINT0;
	} else if (PCI_SLOT(dev->devfn) == 4 && pin == 3) {
		
		irq = IRQ_IOP32X_XINT2;
	} else if (PCI_SLOT(dev->devfn) == 5) {
		
		irq = IRQ_IOP32X_XINT3;
	} else {
		printk(KERN_ERR "n2100_pci_map_irq() called for unknown "
			"device PCI:%d:%d:%d\n", dev->bus->number,
			PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn));
		irq = -1;
	}

	return irq;
}

static struct hw_pci n2100_pci __initdata = {
	.swizzle	= pci_std_swizzle,
	.nr_controllers = 1,
	.setup		= iop3xx_pci_setup,
	.preinit	= iop3xx_pci_preinit,
	.scan		= iop3xx_pci_scan_bus,
	.map_irq	= n2100_pci_map_irq,
};


static void n2100_fixup_r8169(struct pci_dev *dev)
{
	if (dev->bus->number == 0 &&
	    (dev->devfn == PCI_DEVFN(1, 0) ||
	     dev->devfn == PCI_DEVFN(2, 0)))
		dev->broken_parity_status = 1;
}
DECLARE_PCI_FIXUP_FINAL(PCI_VENDOR_ID_REALTEK, PCI_ANY_ID, n2100_fixup_r8169);

static int __init n2100_pci_init(void)
{
	if (machine_is_n2100())
		pci_common_init(&n2100_pci);

	return 0;
}

subsys_initcall(n2100_pci_init);



static struct physmap_flash_data n2100_flash_data = {
	.width		= 2,
};

static struct resource n2100_flash_resource = {
	.start		= 0xf0000000,
	.end		= 0xf0ffffff,
	.flags		= IORESOURCE_MEM,
};

static struct platform_device n2100_flash_device = {
	.name		= "physmap-flash",
	.id		= 0,
	.dev		= {
		.platform_data	= &n2100_flash_data,
	},
	.num_resources	= 1,
	.resource	= &n2100_flash_resource,
};


static struct plat_serial8250_port n2100_serial_port[] = {
	{
		.mapbase	= N2100_UART,
		.membase	= (char *)N2100_UART,
		.irq		= 0,
		.flags		= UPF_SKIP_TEST,
		.iotype		= UPIO_MEM,
		.regshift	= 0,
		.uartclk	= 1843200,
	},
	{ },
};

static struct resource n2100_uart_resource = {
	.start		= N2100_UART,
	.end		= N2100_UART + 7,
	.flags		= IORESOURCE_MEM,
};

static struct platform_device n2100_serial_device = {
	.name		= "serial8250",
	.id		= PLAT8250_DEV_PLATFORM,
	.dev		= {
		.platform_data		= n2100_serial_port,
	},
	.num_resources	= 1,
	.resource	= &n2100_uart_resource,
};

static struct f75375s_platform_data n2100_f75375s = {
	.pwm		= { 255, 255 },
	.pwm_enable = { 0, 0 },
};

static struct pca9532_platform_data n2100_leds = {
	.leds = {
	{	.name = "n2100:red:satafail0",
		.state = PCA9532_OFF,
		.type = PCA9532_TYPE_LED,
	},
	{	.name = "n2100:red:satafail1",
		.state = PCA9532_OFF,
		.type = PCA9532_TYPE_LED,
	},
	{	.name = "n2100:blue:usb",
		.state = PCA9532_OFF,
		.type = PCA9532_TYPE_LED,
	},
	{ 	.type = PCA9532_TYPE_NONE },

	{ 	.type = PCA9532_TYPE_NONE },
	{ 	.type = PCA9532_TYPE_NONE },
	{ 	.type = PCA9532_TYPE_NONE },
	{	.name = "n2100:red:usb",
		.state = PCA9532_OFF,
		.type = PCA9532_TYPE_LED,
	},

	{	.type = PCA9532_TYPE_NONE }, 
	{	.type = PCA9532_TYPE_NONE }, 
	{	.type = PCA9532_TYPE_NONE },
	{	.type = PCA9532_TYPE_NONE },

	{	.type = PCA9532_TYPE_NONE },
	{	.name = "n2100:orange:system",
		.state = PCA9532_OFF,
		.type = PCA9532_TYPE_LED,
	},
	{	.name = "n2100:red:system",
		.state = PCA9532_OFF,
		.type = PCA9532_TYPE_LED,
	},
	{	.name = "N2100 beeper"  ,
		.state =  PCA9532_OFF,
		.type = PCA9532_TYPE_N2100_BEEP,
	},
	},
	.psc = { 0, 0 },
	.pwm = { 0, 0 },
};

static struct i2c_board_info __initdata n2100_i2c_devices[] = {
	{
		I2C_BOARD_INFO("rs5c372b", 0x32),
	},
	{
		I2C_BOARD_INFO("f75375", 0x2e),
		.platform_data = &n2100_f75375s,
	},
	{
		I2C_BOARD_INFO("pca9532", 0x60),
		.platform_data = &n2100_leds,
	},
};


static void n2100_power_off(void)
{
	local_irq_disable();

	
	*IOP3XX_IDBR0 = 0xc0;
	*IOP3XX_ICR0 = 0xe9;
	mdelay(1);

	
	*IOP3XX_IDBR0 = 0x08;
	*IOP3XX_ICR0 = 0xe8;
	mdelay(1);

	
	*IOP3XX_IDBR0 = 0x01;
	*IOP3XX_ICR0 = 0xea;

	while (1)
		;
}


static struct timer_list power_button_poll_timer;

static void power_button_poll(unsigned long dummy)
{
	if (gpio_line_get(N2100_POWER_BUTTON) == 0) {
		ctrl_alt_del();
		return;
	}

	power_button_poll_timer.expires = jiffies + (HZ / 10);
	add_timer(&power_button_poll_timer);
}


static void __init n2100_init_machine(void)
{
	platform_device_register(&iop3xx_i2c0_device);
	platform_device_register(&n2100_flash_device);
	platform_device_register(&n2100_serial_device);
	platform_device_register(&iop3xx_dma_0_channel);
	platform_device_register(&iop3xx_dma_1_channel);

	i2c_register_board_info(0, n2100_i2c_devices,
		ARRAY_SIZE(n2100_i2c_devices));

	pm_power_off = n2100_power_off;

	init_timer(&power_button_poll_timer);
	power_button_poll_timer.function = power_button_poll;
	power_button_poll_timer.expires = jiffies + (HZ / 10);
	add_timer(&power_button_poll_timer);
}

MACHINE_START(N2100, "Thecus N2100")
	
	.phys_io	= N2100_UART,
	.io_pg_offst	= ((N2100_UART) >> 18) & 0xfffc,
	.boot_params	= 0xa0000100,
	.map_io		= n2100_map_io,
	.init_irq	= iop32x_init_irq,
	.timer		= &n2100_timer,
	.init_machine	= n2100_init_machine,
MACHINE_END
