

#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/fb.h>
#include <linux/clk.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>

#include <video/atmel_lcdc.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/irq.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/at91sam9_smc.h>
#include <mach/at91_shdwc.h>

#include "sam9_smc.h"
#include "generic.h"


static void __init ek_map_io(void)
{
	
	at91sam9rl_initialize(12000000);

	
	at91_register_uart(0, 0, 0);

	
	at91_register_uart(AT91SAM9RL_ID_US0, 1, ATMEL_UART_CTS | ATMEL_UART_RTS);

	
	at91_set_serial_console(0);
}

static void __init ek_init_irq(void)
{
	at91sam9rl_init_interrupts(NULL);
}



static struct usba_platform_data __initdata ek_usba_udc_data = {
	.vbus_pin	= AT91_PIN_PA8,
};



static struct at91_mmc_data __initdata ek_mmc_data = {
	.wire4		= 1,
	.det_pin	= AT91_PIN_PA15,


};



static struct mtd_partition __initdata ek_nand_partition[] = {
	{
		.name	= "Partition 1",
		.offset	= 0,
		.size	= SZ_256K,
	},
	{
		.name	= "Partition 2",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= MTDPART_SIZ_FULL,
	},
};

static struct mtd_partition * __init nand_partitions(int size, int *num_partitions)
{
	*num_partitions = ARRAY_SIZE(ek_nand_partition);
	return ek_nand_partition;
}

static struct atmel_nand_data __initdata ek_nand_data = {
	.ale		= 21,
	.cle		= 22,

	.rdy_pin	= AT91_PIN_PD17,
	.enable_pin	= AT91_PIN_PB6,
	.partition_info	= nand_partitions,
};

static struct sam9_smc_config __initdata ek_nand_smc_config = {
	.ncs_read_setup		= 0,
	.nrd_setup		= 1,
	.ncs_write_setup	= 0,
	.nwe_setup		= 1,

	.ncs_read_pulse		= 3,
	.nrd_pulse		= 3,
	.ncs_write_pulse	= 3,
	.nwe_pulse		= 3,

	.read_cycle		= 5,
	.write_cycle		= 5,

	.mode			= AT91_SMC_READMODE | AT91_SMC_WRITEMODE | AT91_SMC_EXNWMODE_DISABLE | AT91_SMC_DBW_8,
	.tdf_cycles		= 2,
};

static void __init ek_add_device_nand(void)
{
	
	sam9_smc_configure(3, &ek_nand_smc_config);

	at91_add_device_nand(&ek_nand_data);
}



static struct spi_board_info ek_spi_devices[] = {
	{	
		.modalias	= "mtd_dataflash",
		.chip_select	= 0,
		.max_speed_hz	= 15 * 1000 * 1000,
		.bus_num	= 0,
	},
};



#if defined(CONFIG_FB_ATMEL) || defined(CONFIG_FB_ATMEL_MODULE)
static struct fb_videomode at91_tft_vga_modes[] = {
	{
		.name		= "TX09D50VM1CCA @ 60",
		.refresh	= 60,
		.xres		= 240,		.yres		= 320,
		.pixclock	= KHZ2PICOS(4965),

		.left_margin	= 1,		.right_margin	= 33,
		.upper_margin	= 1,		.lower_margin	= 0,
		.hsync_len	= 5,		.vsync_len	= 1,

		.sync		= FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
		.vmode		= FB_VMODE_NONINTERLACED,
	},
};

static struct fb_monspecs at91fb_default_monspecs = {
	.manufacturer	= "HIT",
	.monitor	= "TX09D50VM1CCA",

	.modedb		= at91_tft_vga_modes,
	.modedb_len	= ARRAY_SIZE(at91_tft_vga_modes),
	.hfmin		= 15000,
	.hfmax		= 64000,
	.vfmin		= 50,
	.vfmax		= 150,
};

#define AT91SAM9RL_DEFAULT_LCDCON2	(ATMEL_LCDC_MEMOR_LITTLE \
					| ATMEL_LCDC_DISTYPE_TFT \
					| ATMEL_LCDC_CLKMOD_ALWAYSACTIVE)

static void at91_lcdc_power_control(int on)
{
	if (on)
		at91_set_gpio_value(AT91_PIN_PC1, 0);	
	else
		at91_set_gpio_value(AT91_PIN_PC1, 1);	
}


static struct atmel_lcdfb_info __initdata ek_lcdc_data = {
	.lcdcon_is_backlight            = true,
	.default_bpp			= 16,
	.default_dmacon			= ATMEL_LCDC_DMAEN,
	.default_lcdcon2		= AT91SAM9RL_DEFAULT_LCDCON2,
	.default_monspecs		= &at91fb_default_monspecs,
	.atmel_lcdfb_power_control	= at91_lcdc_power_control,
	.guard_time			= 1,
	.lcd_wiring_mode		= ATMEL_LCDC_WIRING_RGB,
};

#else
static struct atmel_lcdfb_info __initdata ek_lcdc_data;
#endif



static struct ac97c_platform_data ek_ac97_data = {
};



static struct gpio_led ek_leds[] = {
	{	
		.name			= "ds1",
		.gpio			= AT91_PIN_PD15,
		.active_low		= 1,
		.default_trigger	= "none",
	},
	{	
		.name			= "ds2",
		.gpio			= AT91_PIN_PD16,
		.active_low		= 1,
		.default_trigger	= "none",
	},
	{	
		.name			= "ds3",
		.gpio			= AT91_PIN_PD14,
		.default_trigger	= "heartbeat",
	}
};



#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
static struct gpio_keys_button ek_buttons[] = {
	{
		.gpio		= AT91_PIN_PB0,
		.code		= BTN_2,
		.desc		= "Right Click",
		.active_low	= 1,
		.wakeup		= 1,
	},
	{
		.gpio		= AT91_PIN_PB1,
		.code		= BTN_1,
		.desc		= "Left Click",
		.active_low	= 1,
		.wakeup		= 1,
	}
};

static struct gpio_keys_platform_data ek_button_data = {
	.buttons	= ek_buttons,
	.nbuttons	= ARRAY_SIZE(ek_buttons),
};

static struct platform_device ek_button_device = {
	.name		= "gpio-keys",
	.id		= -1,
	.num_resources	= 0,
	.dev		= {
		.platform_data	= &ek_button_data,
	}
};

static void __init ek_add_device_buttons(void)
{
	at91_set_gpio_input(AT91_PIN_PB1, 1);	
	at91_set_deglitch(AT91_PIN_PB1, 1);
	at91_set_gpio_input(AT91_PIN_PB0, 1);	
	at91_set_deglitch(AT91_PIN_PB0, 1);

	platform_device_register(&ek_button_device);
}
#else
static void __init ek_add_device_buttons(void) {}
#endif


static void __init ek_board_init(void)
{
	
	at91_add_device_serial();
	
	at91_add_device_usba(&ek_usba_udc_data);
	
	at91_add_device_i2c(NULL, 0);
	
	ek_add_device_nand();
	
	at91_add_device_spi(ek_spi_devices, ARRAY_SIZE(ek_spi_devices));
	
	at91_add_device_mmc(0, &ek_mmc_data);
	
	at91_add_device_lcdc(&ek_lcdc_data);
	
	at91_add_device_ac97(&ek_ac97_data);
	
	at91_add_device_tsadcc();
	
	at91_gpio_leds(ek_leds, ARRAY_SIZE(ek_leds));
	
	ek_add_device_buttons();
}

MACHINE_START(AT91SAM9RLEK, "Atmel AT91SAM9RL-EK")
	
	.phys_io	= AT91_BASE_SYS,
	.io_pg_offst	= (AT91_VA_BASE_SYS >> 18) & 0xfffc,
	.boot_params	= AT91_SDRAM_BASE + 0x100,
	.timer		= &at91sam926x_timer,
	.map_io		= ek_map_io,
	.init_irq	= ek_init_irq,
	.init_machine	= ek_board_init,
MACHINE_END
