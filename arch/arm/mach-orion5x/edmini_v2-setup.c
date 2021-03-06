



#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/pci.h>
#include <linux/irq.h>
#include <linux/mtd/physmap.h>
#include <linux/mv643xx_eth.h>
#include <linux/leds.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/ata_platform.h>
#include <linux/gpio.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/pci.h>
#include <mach/orion5x.h>
#include "common.h"
#include "mpp.h"





#define EDMINI_V2_NOR_BOOT_BASE		0xfff80000
#define EDMINI_V2_NOR_BOOT_SIZE		SZ_512K





static struct mtd_partition edmini_v2_partitions[] = {
	{
		.name		= "Full512kb",
		.size		= 0x00080000,
		.offset		= 0x00000000,
		.mask_flags	= MTD_WRITEABLE,
	},
};

static struct physmap_flash_data edmini_v2_nor_flash_data = {
	.width		= 1,
	.parts		= edmini_v2_partitions,
	.nr_parts	= ARRAY_SIZE(edmini_v2_partitions),
};

static struct resource edmini_v2_nor_flash_resource = {
	.flags			= IORESOURCE_MEM,
	.start			= EDMINI_V2_NOR_BOOT_BASE,
	.end			= EDMINI_V2_NOR_BOOT_BASE
		+ EDMINI_V2_NOR_BOOT_SIZE - 1,
};

static struct platform_device edmini_v2_nor_flash = {
	.name			= "physmap-flash",
	.id			= 0,
	.dev		= {
		.platform_data	= &edmini_v2_nor_flash_data,
	},
	.num_resources		= 1,
	.resource		= &edmini_v2_nor_flash_resource,
};



static struct mv643xx_eth_platform_data edmini_v2_eth_data = {
	.phy_addr	= 8,
};



#define EDMINIV2_RTC_GPIO	3

static struct i2c_board_info __initdata edmini_v2_i2c_rtc = {
	I2C_BOARD_INFO("rs5c372a", 0x32),
	.irq = 0,
};



static struct mv_sata_platform_data edmini_v2_sata_data = {
	.n_ports	= 2,
};



#define EDMINI_V2_GPIO_LED_POWER	16

static struct gpio_led edmini_v2_leds[] = {
	{
		.name = "power:blue",
		.gpio = EDMINI_V2_GPIO_LED_POWER,
		.active_low = 1,
	},
};

static struct gpio_led_platform_data edmini_v2_led_data = {
	.num_leds = ARRAY_SIZE(edmini_v2_leds),
	.leds = edmini_v2_leds,
};

static struct platform_device edmini_v2_gpio_leds = {
	.name           = "leds-gpio",
	.id             = -1,
	.dev            = {
		.platform_data  = &edmini_v2_led_data,
	},
};



#define EDMINI_V2_GPIO_KEY_POWER	18

static struct gpio_keys_button edmini_v2_buttons[] = {
	{
		.code		= KEY_POWER,
		.gpio		= EDMINI_V2_GPIO_KEY_POWER,
		.desc		= "Power Button",
		.active_low	= 0,
	},
};

static struct gpio_keys_platform_data edmini_v2_button_data = {
	.buttons	= edmini_v2_buttons,
	.nbuttons	= ARRAY_SIZE(edmini_v2_buttons),
};

static struct platform_device edmini_v2_gpio_buttons = {
	.name		= "gpio-keys",
	.id		= -1,
	.dev		= {
		.platform_data	= &edmini_v2_button_data,
	},
};


static struct orion5x_mpp_mode edminiv2_mpp_modes[] __initdata = {
	{  0, MPP_UNUSED },
	{  1, MPP_UNUSED },
	{  2, MPP_UNUSED },
	{  3, MPP_GPIO },	
	{  4, MPP_UNUSED },
	{  5, MPP_UNUSED },
	{  6, MPP_UNUSED },
	{  7, MPP_UNUSED },
	{  8, MPP_UNUSED },
	{  9, MPP_UNUSED },
	{ 10, MPP_UNUSED },
	{ 11, MPP_UNUSED },
	{ 12, MPP_SATA_LED },	
	{ 13, MPP_SATA_LED },	
	{ 14, MPP_SATA_LED },	
	{ 15, MPP_SATA_LED },	
	
	{ 16, MPP_GPIO },
	
	{ 17, MPP_GPIO },
	
	{ 18, MPP_GPIO },
	{ 19, MPP_UNUSED },
	{ -1 }
};

static void __init edmini_v2_init(void)
{
	
	orion5x_init();

	orion5x_mpp_conf(edminiv2_mpp_modes);

	
	orion5x_ehci0_init();
	orion5x_eth_init(&edmini_v2_eth_data);
	orion5x_i2c_init();
	orion5x_sata_init(&edmini_v2_sata_data);
	orion5x_uart0_init();

	orion5x_setup_dev_boot_win(EDMINI_V2_NOR_BOOT_BASE,
				EDMINI_V2_NOR_BOOT_SIZE);
	platform_device_register(&edmini_v2_nor_flash);
	platform_device_register(&edmini_v2_gpio_leds);
	platform_device_register(&edmini_v2_gpio_buttons);

	pr_notice("edmini_v2: USB device port, flash write and power-off "
		  "are not yet supported.\n");

	
	if (gpio_request(EDMINIV2_RTC_GPIO, "rtc") == 0) {
		if (gpio_direction_input(EDMINIV2_RTC_GPIO) == 0)
			edmini_v2_i2c_rtc.irq = gpio_to_irq(EDMINIV2_RTC_GPIO);
		else
			gpio_free(EDMINIV2_RTC_GPIO);
	}

	if (edmini_v2_i2c_rtc.irq == 0)
		pr_warning("edmini_v2: failed to get RTC IRQ\n");

	i2c_register_board_info(0, &edmini_v2_i2c_rtc, 1);
}


MACHINE_START(EDMINI_V2, "LaCie Ethernet Disk mini V2")
	
	.phys_io	= ORION5X_REGS_PHYS_BASE,
	.io_pg_offst	= ((ORION5X_REGS_VIRT_BASE) >> 18) & 0xFFFC,
	.boot_params	= 0x00000100,
	.init_machine	= edmini_v2_init,
	.map_io		= orion5x_map_io,
	.init_irq	= orion5x_init_irq,
	.timer		= &orion5x_timer,
	.fixup		= tag_fixup_mem32,
MACHINE_END
