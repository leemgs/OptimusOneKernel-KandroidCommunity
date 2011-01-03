

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/pci.h>
#include <linux/mtd/physmap.h>
#include <linux/mv643xx_eth.h>
#include <linux/leds.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/ata_platform.h>
#include <asm/mach-types.h>
#include <linux/gpio.h>
#include <asm/mach/arch.h>
#include "common.h"
#include "mpp.h"
#include "include/mach/system.h"





#define LSMINI_NOR_BOOT_BASE	0xf4000000
#define LSMINI_NOR_BOOT_SIZE	SZ_256K



static struct physmap_flash_data lsmini_nor_flash_data = {
	.width		= 1,
};

static struct resource lsmini_nor_flash_resource = {
	.flags	= IORESOURCE_MEM,
	.start	= LSMINI_NOR_BOOT_BASE,
	.end	= LSMINI_NOR_BOOT_BASE + LSMINI_NOR_BOOT_SIZE - 1,
};

static struct platform_device lsmini_nor_flash = {
	.name			= "physmap-flash",
	.id			= 0,
	.dev		= {
		.platform_data	= &lsmini_nor_flash_data,
	},
	.num_resources		= 1,
	.resource		= &lsmini_nor_flash_resource,
};



static struct mv643xx_eth_platform_data lsmini_eth_data = {
	.phy_addr	= 8,
};



static struct i2c_board_info __initdata lsmini_i2c_rtc = {
	I2C_BOARD_INFO("rs5c372a", 0x32),
};



#define LSMINI_GPIO_LED_ALARM	2
#define LSMINI_GPIO_LED_INFO	3
#define LSMINI_GPIO_LED_FUNC	9
#define LSMINI_GPIO_LED_PWR	14

static struct gpio_led lsmini_led_pins[] = {
	{
		.name	   = "alarm:red",
		.gpio	   = LSMINI_GPIO_LED_ALARM,
		.active_low     = 1,
	}, {
		.name	   = "info:amber",
		.gpio	   = LSMINI_GPIO_LED_INFO,
		.active_low     = 1,
	}, {
		.name	   = "func:blue:top",
		.gpio	   = LSMINI_GPIO_LED_FUNC,
		.active_low     = 1,
	}, {
		.name	   = "power:blue:bottom",
		.gpio	   = LSMINI_GPIO_LED_PWR,
	},
};

static struct gpio_led_platform_data lsmini_led_data = {
	.leds	   = lsmini_led_pins,
	.num_leds       = ARRAY_SIZE(lsmini_led_pins),
};

static struct platform_device lsmini_leds = {
	.name   = "leds-gpio",
	.id     = -1,
	.dev    = {
		.platform_data  = &lsmini_led_data,
	},
};



#define LSMINI_GPIO_KEY_FUNC       15
#define LSMINI_GPIO_KEY_POWER	   18
#define LSMINI_GPIO_KEY_AUTOPOWER 17

#define LSMINI_SW_POWER		0x00
#define LSMINI_SW_AUTOPOWER	0x01

static struct gpio_keys_button lsmini_buttons[] = {
	{
		.code	   = KEY_OPTION,
		.gpio	   = LSMINI_GPIO_KEY_FUNC,
		.desc	   = "Function Button",
		.active_low     = 1,
	}, {
		.type		= EV_SW,
		.code	   = LSMINI_SW_POWER,
		.gpio	   = LSMINI_GPIO_KEY_POWER,
		.desc	   = "Power-on Switch",
		.active_low     = 1,
	}, {
		.type		= EV_SW,
		.code	   = LSMINI_SW_AUTOPOWER,
		.gpio	   = LSMINI_GPIO_KEY_AUTOPOWER,
		.desc	   = "Power-auto Switch",
		.active_low     = 1,
	},
};

static struct gpio_keys_platform_data lsmini_button_data = {
	.buttons	= lsmini_buttons,
	.nbuttons       = ARRAY_SIZE(lsmini_buttons),
};

static struct platform_device lsmini_button_device = {
	.name	   = "gpio-keys",
	.id	     = -1,
	.num_resources  = 0,
	.dev	    = {
		.platform_data  = &lsmini_button_data,
	},
};



static struct mv_sata_platform_data lsmini_sata_data = {
	.n_ports	= 2,
};





static void lsmini_power_off(void)
{
	arch_reset(0, NULL);
}




#define LSMINI_GPIO_USB_POWER	16
#define LSMINI_GPIO_AUTO_POWER	17
#define LSMINI_GPIO_POWER	18

#define LSMINI_GPIO_HDD_POWER0	1
#define LSMINI_GPIO_HDD_POWER1	19

static struct orion5x_mpp_mode lsmini_mpp_modes[] __initdata = {
	{  0, MPP_UNUSED }, 
	{  1, MPP_GPIO }, 
	{  2, MPP_GPIO }, 
	{  3, MPP_GPIO }, 
	{  4, MPP_UNUSED },
	{  5, MPP_UNUSED },
	{  6, MPP_UNUSED },
	{  7, MPP_UNUSED },
	{  8, MPP_UNUSED },
	{  9, MPP_GPIO }, 
	{ 10, MPP_UNUSED },
	{ 11, MPP_UNUSED }, 
	{ 12, MPP_UNUSED },
	{ 13, MPP_UNUSED },
	{ 14, MPP_GPIO }, 
	{ 15, MPP_GPIO }, 
	{ 16, MPP_GPIO }, 
	{ 17, MPP_GPIO }, 
	{ 18, MPP_GPIO }, 
	{ 19, MPP_GPIO }, 
	{ -1 },
};

static void __init lsmini_init(void)
{
	
	orion5x_init();

	orion5x_mpp_conf(lsmini_mpp_modes);

	
	orion5x_ehci0_init();
	orion5x_ehci1_init();
	orion5x_eth_init(&lsmini_eth_data);
	orion5x_i2c_init();
	orion5x_sata_init(&lsmini_sata_data);
	orion5x_uart0_init();
	orion5x_xor_init();

	orion5x_setup_dev_boot_win(LSMINI_NOR_BOOT_BASE,
				   LSMINI_NOR_BOOT_SIZE);
	platform_device_register(&lsmini_nor_flash);

	platform_device_register(&lsmini_button_device);

	platform_device_register(&lsmini_leds);

	i2c_register_board_info(0, &lsmini_i2c_rtc, 1);

	
	gpio_set_value(LSMINI_GPIO_USB_POWER, 1);

	
	pm_power_off = lsmini_power_off;

	pr_info("%s: finished\n", __func__);
}

#ifdef CONFIG_MACH_LINKSTATION_MINI
MACHINE_START(LINKSTATION_MINI, "Buffalo Linkstation Mini")
	
	.phys_io	= ORION5X_REGS_PHYS_BASE,
	.io_pg_offst	= ((ORION5X_REGS_VIRT_BASE) >> 18) & 0xFFFC,
	.boot_params	= 0x00000100,
	.init_machine	= lsmini_init,
	.map_io		= orion5x_map_io,
	.init_irq	= orion5x_init_irq,
	.timer		= &orion5x_timer,
	.fixup		= tag_fixup_mem32,
MACHINE_END
#endif
