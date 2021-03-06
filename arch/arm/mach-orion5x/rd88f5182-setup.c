

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/pci.h>
#include <linux/irq.h>
#include <linux/mtd/physmap.h>
#include <linux/mv643xx_eth.h>
#include <linux/ata_platform.h>
#include <linux/i2c.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>
#include <asm/leds.h>
#include <asm/mach/arch.h>
#include <asm/mach/pci.h>
#include <mach/orion5x.h>
#include "common.h"
#include "mpp.h"





#define RD88F5182_NOR_BOOT_BASE		0xf4000000
#define RD88F5182_NOR_BOOT_SIZE		SZ_512K



#define RD88F5182_NOR_BASE		0xfc000000
#define RD88F5182_NOR_SIZE		SZ_16M



#define RD88F5182_PCI_SLOT0_OFFS	7
#define RD88F5182_PCI_SLOT0_IRQ_A_PIN	7
#define RD88F5182_PCI_SLOT0_IRQ_B_PIN	6



#define RD88F5182_GPIO_DBG_LED		0



static struct physmap_flash_data rd88f5182_nor_flash_data = {
	.width		= 1,
};

static struct resource rd88f5182_nor_flash_resource = {
	.flags			= IORESOURCE_MEM,
	.start			= RD88F5182_NOR_BASE,
	.end			= RD88F5182_NOR_BASE + RD88F5182_NOR_SIZE - 1,
};

static struct platform_device rd88f5182_nor_flash = {
	.name			= "physmap-flash",
	.id			= 0,
	.dev		= {
		.platform_data	= &rd88f5182_nor_flash_data,
	},
	.num_resources		= 1,
	.resource		= &rd88f5182_nor_flash_resource,
};

#ifdef CONFIG_LEDS



static void rd88f5182_dbgled_event(led_event_t evt)
{
	int val;

	if (evt == led_idle_end)
		val = 1;
	else if (evt == led_idle_start)
		val = 0;
	else
		return;

	gpio_set_value(RD88F5182_GPIO_DBG_LED, val);
}

static int __init rd88f5182_dbgled_init(void)
{
	int pin;

	if (machine_is_rd88f5182()) {
		pin = RD88F5182_GPIO_DBG_LED;

		if (gpio_request(pin, "DBGLED") == 0) {
			if (gpio_direction_output(pin, 0) != 0) {
				printk(KERN_ERR "rd88f5182_dbgled_init failed "
						"to set output pin %d\n", pin);
				gpio_free(pin);
				return 0;
			}
		} else {
			printk(KERN_ERR "rd88f5182_dbgled_init failed "
					"to request gpio %d\n", pin);
			return 0;
		}

		leds_event = rd88f5182_dbgled_event;
	}

	return 0;
}

__initcall(rd88f5182_dbgled_init);

#endif



void __init rd88f5182_pci_preinit(void)
{
	int pin;

	
	pin = RD88F5182_PCI_SLOT0_IRQ_A_PIN;
	if (gpio_request(pin, "PCI IntA") == 0) {
		if (gpio_direction_input(pin) == 0) {
			set_irq_type(gpio_to_irq(pin), IRQ_TYPE_LEVEL_LOW);
		} else {
			printk(KERN_ERR "rd88f5182_pci_preinit faield to "
					"set_irq_type pin %d\n", pin);
			gpio_free(pin);
		}
	} else {
		printk(KERN_ERR "rd88f5182_pci_preinit failed to request gpio %d\n", pin);
	}

	pin = RD88F5182_PCI_SLOT0_IRQ_B_PIN;
	if (gpio_request(pin, "PCI IntB") == 0) {
		if (gpio_direction_input(pin) == 0) {
			set_irq_type(gpio_to_irq(pin), IRQ_TYPE_LEVEL_LOW);
		} else {
			printk(KERN_ERR "rd88f5182_pci_preinit faield to "
					"set_irq_type pin %d\n", pin);
			gpio_free(pin);
		}
	} else {
		printk(KERN_ERR "rd88f5182_pci_preinit failed to gpio_request %d\n", pin);
	}
}

static int __init rd88f5182_pci_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	int irq;

	
	irq = orion5x_pci_map_irq(dev, slot, pin);
	if (irq != -1)
		return irq;

	
	switch (slot - RD88F5182_PCI_SLOT0_OFFS) {
	case 0:
		if (pin == 1)
			return gpio_to_irq(RD88F5182_PCI_SLOT0_IRQ_A_PIN);
		else
			return gpio_to_irq(RD88F5182_PCI_SLOT0_IRQ_B_PIN);
	default:
		return -1;
	}
}

static struct hw_pci rd88f5182_pci __initdata = {
	.nr_controllers	= 2,
	.preinit	= rd88f5182_pci_preinit,
	.swizzle	= pci_std_swizzle,
	.setup		= orion5x_pci_sys_setup,
	.scan		= orion5x_pci_sys_scan_bus,
	.map_irq	= rd88f5182_pci_map_irq,
};

static int __init rd88f5182_pci_init(void)
{
	if (machine_is_rd88f5182())
		pci_common_init(&rd88f5182_pci);

	return 0;
}

subsys_initcall(rd88f5182_pci_init);



static struct mv643xx_eth_platform_data rd88f5182_eth_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(8),
};


static struct i2c_board_info __initdata rd88f5182_i2c_rtc = {
	I2C_BOARD_INFO("ds1338", 0x68),
};


static struct mv_sata_platform_data rd88f5182_sata_data = {
	.n_ports	= 2,
};


static struct orion5x_mpp_mode rd88f5182_mpp_modes[] __initdata = {
	{  0, MPP_GPIO },		
	{  1, MPP_GPIO },		
	{  2, MPP_UNUSED },
	{  3, MPP_GPIO },		
	{  4, MPP_GPIO },
	{  5, MPP_GPIO },
	{  6, MPP_GPIO },		
	{  7, MPP_GPIO },		
	{  8, MPP_UNUSED },
	{  9, MPP_UNUSED },
	{ 10, MPP_UNUSED },
	{ 11, MPP_UNUSED },
	{ 12, MPP_SATA_LED },		
	{ 13, MPP_SATA_LED },		
	{ 14, MPP_SATA_LED },		
	{ 15, MPP_SATA_LED },		
	{ 16, MPP_UNUSED },
	{ 17, MPP_UNUSED },
	{ 18, MPP_UNUSED },
	{ 19, MPP_UNUSED },
	{ -1 },
};

static void __init rd88f5182_init(void)
{
	
	orion5x_init();

	orion5x_mpp_conf(rd88f5182_mpp_modes);

	

	
	orion5x_ehci0_init();
	orion5x_ehci1_init();
	orion5x_eth_init(&rd88f5182_eth_data);
	orion5x_i2c_init();
	orion5x_sata_init(&rd88f5182_sata_data);
	orion5x_uart0_init();
	orion5x_xor_init();

	orion5x_setup_dev_boot_win(RD88F5182_NOR_BOOT_BASE,
				   RD88F5182_NOR_BOOT_SIZE);

	orion5x_setup_dev1_win(RD88F5182_NOR_BASE, RD88F5182_NOR_SIZE);
	platform_device_register(&rd88f5182_nor_flash);

	i2c_register_board_info(0, &rd88f5182_i2c_rtc, 1);
}

MACHINE_START(RD88F5182, "Marvell Orion-NAS Reference Design")
	
	.phys_io	= ORION5X_REGS_PHYS_BASE,
	.io_pg_offst	= ((ORION5X_REGS_VIRT_BASE) >> 18) & 0xFFFC,
	.boot_params	= 0x00000100,
	.init_machine	= rd88f5182_init,
	.map_io		= orion5x_map_io,
	.init_irq	= orion5x_init_irq,
	.timer		= &orion5x_timer,
MACHINE_END
