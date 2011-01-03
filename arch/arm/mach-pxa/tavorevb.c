

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/smc91x.h>
#include <linux/pwm_backlight.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>

#include <mach/pxa930.h>
#include <mach/pxafb.h>
#include <mach/pxa27x_keypad.h>

#include "devices.h"
#include "generic.h"


static mfp_cfg_t tavorevb_mfp_cfg[] __initdata = {
	
	DF_nCS1_nCS3,
	GPIO47_GPIO,

	
	GPIO23_LCD_DD0,
	GPIO24_LCD_DD1,
	GPIO25_LCD_DD2,
	GPIO26_LCD_DD3,
	GPIO27_LCD_DD4,
	GPIO28_LCD_DD5,
	GPIO29_LCD_DD6,
	GPIO44_LCD_DD7,
	GPIO21_LCD_CS,
	GPIO22_LCD_CS2,

	GPIO17_LCD_FCLK_RD,
	GPIO18_LCD_LCLK_A0,
	GPIO19_LCD_PCLK_WR,

	
	GPIO43_PWM3,	
	GPIO32_PWM0,	

	
	GPIO0_KP_MKIN_0,
	GPIO2_KP_MKIN_1,
	GPIO4_KP_MKIN_2,
	GPIO6_KP_MKIN_3,
	GPIO8_KP_MKIN_4,
	GPIO10_KP_MKIN_5,
	GPIO12_KP_MKIN_6,
	GPIO1_KP_MKOUT_0,
	GPIO3_KP_MKOUT_1,
	GPIO5_KP_MKOUT_2,
	GPIO7_KP_MKOUT_3,
	GPIO9_KP_MKOUT_4,
	GPIO11_KP_MKOUT_5,
	GPIO13_KP_MKOUT_6,

	GPIO14_KP_DKIN_2,
	GPIO15_KP_DKIN_3,
};

#define TAVOREVB_ETH_PHYS	(0x14000000)

static struct resource smc91x_resources[] = {
	[0] = {
		.start	= (TAVOREVB_ETH_PHYS + 0x300),
		.end	= (TAVOREVB_ETH_PHYS + 0xfffff),
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= gpio_to_irq(mfp_to_gpio(MFP_PIN_GPIO47)),
		.end	= gpio_to_irq(mfp_to_gpio(MFP_PIN_GPIO47)),
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,
	}
};

static struct smc91x_platdata tavorevb_smc91x_info = {
	.flags	= SMC91X_USE_16BIT | SMC91X_NOWAIT | SMC91X_USE_DMA,
};

static struct platform_device smc91x_device = {
	.name		= "smc91x",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(smc91x_resources),
	.resource	= smc91x_resources,
	.dev		= {
		.platform_data = &tavorevb_smc91x_info,
	},
};

#if defined(CONFIG_KEYBOARD_PXA27x) || defined(CONFIG_KEYBOARD_PXA27x_MODULE)
static unsigned int tavorevb_matrix_key_map[] = {
	
	KEY(0, 4, KEY_A), KEY(0, 5, KEY_B), KEY(0, 6, KEY_C),
	KEY(1, 4, KEY_E), KEY(1, 5, KEY_F), KEY(1, 6, KEY_G),
	KEY(2, 4, KEY_I), KEY(2, 5, KEY_J), KEY(2, 6, KEY_K),
	KEY(3, 4, KEY_M), KEY(3, 5, KEY_N), KEY(3, 6, KEY_O),
	KEY(4, 5, KEY_R), KEY(4, 6, KEY_S),
	KEY(5, 4, KEY_U), KEY(5, 4, KEY_V), KEY(5, 6, KEY_W),

	KEY(6, 4, KEY_Y), KEY(6, 5, KEY_Z),

	KEY(0, 3, KEY_0), KEY(2, 0, KEY_1), KEY(2, 1, KEY_2), KEY(2, 2, KEY_3),
	KEY(2, 3, KEY_4), KEY(1, 0, KEY_5), KEY(1, 1, KEY_6), KEY(1, 2, KEY_7),
	KEY(1, 3, KEY_8), KEY(0, 2, KEY_9),

	KEY(6, 6, KEY_SPACE),
	KEY(0, 0, KEY_KPASTERISK), 	
	KEY(0, 1, KEY_KPDOT), 		

	KEY(4, 1, KEY_UP),
	KEY(4, 3, KEY_DOWN),
	KEY(4, 0, KEY_LEFT),
	KEY(4, 2, KEY_RIGHT),
	KEY(6, 0, KEY_HOME),
	KEY(3, 2, KEY_END),
	KEY(6, 1, KEY_DELETE),
	KEY(5, 2, KEY_BACK),
	KEY(6, 3, KEY_CAPSLOCK),	

	KEY(4, 4, KEY_ENTER),		
	KEY(6, 2, KEY_ENTER),		

	KEY(3, 1, KEY_SEND),
	KEY(5, 3, KEY_RECORD),
	KEY(5, 0, KEY_VOLUMEUP),
	KEY(5, 1, KEY_VOLUMEDOWN),

	KEY(3, 0, KEY_F22),	
	KEY(3, 3, KEY_F23),	
};

static struct pxa27x_keypad_platform_data tavorevb_keypad_info = {
	.matrix_key_rows	= 7,
	.matrix_key_cols	= 7,
	.matrix_key_map		= tavorevb_matrix_key_map,
	.matrix_key_map_size	= ARRAY_SIZE(tavorevb_matrix_key_map),
	.debounce_interval	= 30,
};

static void __init tavorevb_init_keypad(void)
{
	pxa_set_keypad_info(&tavorevb_keypad_info);
}
#else
static inline void tavorevb_init_keypad(void) {}
#endif 

#if defined(CONFIG_FB_PXA) || defined(CONFIG_FB_PXA_MODULE)
static struct platform_pwm_backlight_data tavorevb_backlight_data[] = {
	[0] = {
		
		.pwm_id		= 2,
		.max_brightness	= 100,
		.dft_brightness	= 100,
		.pwm_period_ns	= 100000,
	},
	[1] = {
		
		.pwm_id		= 0,
		.max_brightness	= 100,
		.dft_brightness	= 100,
		.pwm_period_ns	= 100000,
	},
};

static struct platform_device tavorevb_backlight_devices[] = {
	[0] = {
		.name		= "pwm-backlight",
		.id		= 0,
		.dev		= {
			.platform_data = &tavorevb_backlight_data[0],
		},
	},
	[1] = {
		.name		= "pwm-backlight",
		.id		= 1,
		.dev		= {
			.platform_data = &tavorevb_backlight_data[1],
		},
	},
};

static uint16_t panel_init[] = {
	
	SMART_CMD(0x00),
	SMART_CMD_NOOP,
	SMART_DELAY(1),

	SMART_CMD(0x00),
	SMART_CMD_NOOP,
	SMART_DELAY(1),

	SMART_CMD(0x00),
	SMART_CMD_NOOP,
	SMART_DELAY(1),

	
	SMART_CMD(0x00),
	SMART_CMD(0x1D),
	SMART_DAT(0x00),
	SMART_DAT(0x05),
	SMART_DELAY(1),

	
	SMART_CMD(0x00), 
	SMART_CMD(0x00),
	SMART_DAT(0x00),
	SMART_DAT(0x01),
	SMART_CMD(0x00),
	SMART_CMD(0x01), 
	SMART_DAT(0x01),
	SMART_DAT(0x27),
	SMART_CMD(0x00),
	SMART_CMD(0x02), 
	SMART_DAT(0x02),
	SMART_DAT(0x00),
	SMART_CMD(0x00),
	SMART_CMD(0x03), 
	SMART_DAT(0x01), 
	SMART_DAT(0x30),
	SMART_CMD(0x07),
	SMART_CMD(0x00), 
	SMART_DAT(0x00),
	SMART_DAT(0x03),
	SMART_CMD(0x00),

	
	SMART_CMD(0x07),
	SMART_DAT(0x40), 
	SMART_DAT(0x00),
	SMART_CMD(0x00),
	SMART_CMD(0x08), 
	SMART_DAT(0x03),
	SMART_DAT(0x02),
	SMART_CMD(0x00),
	SMART_CMD(0x0C), 
	SMART_DAT(0x00),
	SMART_DAT(0x00),
	SMART_CMD(0x00),
	SMART_CMD(0x0D), 
	SMART_DAT(0x00),
	SMART_DAT(0x10),
	SMART_CMD(0x00),
	SMART_CMD(0x12), 
	SMART_DAT(0x03),
	SMART_DAT(0x02),
	SMART_CMD(0x00),
	SMART_CMD(0x13), 
	SMART_DAT(0x01),
	SMART_DAT(0x02),
	SMART_CMD(0x00),
	SMART_CMD(0x14), 
	SMART_DAT(0x00),
	SMART_DAT(0x00),
	SMART_CMD(0x00),
	SMART_CMD(0x15), 
	SMART_DAT(0x20),
	SMART_DAT(0x00),
	SMART_CMD(0x00),
	SMART_CMD(0x1C),
	SMART_DAT(0x00),
	SMART_DAT(0x00),
	SMART_CMD(0x03),
	SMART_CMD(0x00),
	SMART_DAT(0x04),
	SMART_DAT(0x03),
	SMART_CMD(0x03),
	SMART_CMD(0x01),
	SMART_DAT(0x03),
	SMART_DAT(0x04),
	SMART_CMD(0x03),
	SMART_CMD(0x02),
	SMART_DAT(0x04),
	SMART_DAT(0x03),
	SMART_CMD(0x03),
	SMART_CMD(0x03),
	SMART_DAT(0x03),
	SMART_DAT(0x03),
	SMART_CMD(0x03),
	SMART_CMD(0x04),
	SMART_DAT(0x01),
	SMART_DAT(0x01),
	SMART_CMD(0x03),
	SMART_CMD(0x05),
	SMART_DAT(0x00),
	SMART_DAT(0x00),
	SMART_CMD(0x04),
	SMART_CMD(0x02),
	SMART_DAT(0x00),
	SMART_DAT(0x00),
	SMART_CMD(0x04),
	SMART_CMD(0x03),
	SMART_DAT(0x01),
	SMART_DAT(0x3F),
	SMART_DELAY(0),

	
	SMART_CMD(0x04), 
	SMART_CMD(0x06),
	SMART_DAT(0x00),
	SMART_DAT(0x00), 
	SMART_CMD(0x04), 
	SMART_CMD(0x07),
	SMART_DAT(0x00),
	SMART_DAT(0xEF), 
	SMART_CMD(0x04), 
	SMART_CMD(0x08),
	SMART_DAT(0x00), 
	SMART_DAT(0x00), 
	SMART_CMD(0x04), 
	SMART_CMD(0x09),
	SMART_DAT(0x01), 
	SMART_DAT(0x3F), 
	SMART_CMD(0x02), 
	SMART_CMD(0x00),
	SMART_DAT(0x00),
	SMART_DAT(0x00), 
	SMART_CMD(0x02), 
	SMART_CMD(0x01),
	SMART_DAT(0x00), 
	SMART_DAT(0x00), 
};

static uint16_t panel_on[] = {
	
	SMART_CMD(0x01),
	SMART_CMD(0x02),
	SMART_DAT(0x07),
	SMART_DAT(0x7D),
	SMART_CMD(0x01),
	SMART_CMD(0x03),
	SMART_DAT(0x00),
	SMART_DAT(0x05),
	SMART_CMD(0x01),
	SMART_CMD(0x04),
	SMART_DAT(0x00),
	SMART_DAT(0x00),
	SMART_CMD(0x01),
	SMART_CMD(0x05),
	SMART_DAT(0x00),
	SMART_DAT(0x15),
	SMART_CMD(0x01),
	SMART_CMD(0x00),
	SMART_DAT(0xC0),
	SMART_DAT(0x10),
	SMART_DELAY(30),

	
	SMART_CMD(0x01),
	SMART_CMD(0x01),
	SMART_DAT(0x00),
	SMART_DAT(0x01),
	SMART_CMD(0x01),
	SMART_CMD(0x00),
	SMART_DAT(0xFF),
	SMART_DAT(0xFE),
	SMART_DELAY(150),
};

static uint16_t panel_off[] = {
	SMART_CMD(0x00),
	SMART_CMD(0x1E),
	SMART_DAT(0x00),
	SMART_DAT(0x0A),
	SMART_CMD(0x01),
	SMART_CMD(0x00),
	SMART_DAT(0xFF),
	SMART_DAT(0xEE),
	SMART_CMD(0x01),
	SMART_CMD(0x00),
	SMART_DAT(0xF8),
	SMART_DAT(0x12),
	SMART_CMD(0x01),
	SMART_CMD(0x00),
	SMART_DAT(0xE8),
	SMART_DAT(0x11),
	SMART_CMD(0x01),
	SMART_CMD(0x00),
	SMART_DAT(0xC0),
	SMART_DAT(0x11),
	SMART_CMD(0x01),
	SMART_CMD(0x00),
	SMART_DAT(0x40),
	SMART_DAT(0x11),
	SMART_CMD(0x01),
	SMART_CMD(0x00),
	SMART_DAT(0x00),
	SMART_DAT(0x10),
};

static uint16_t update_framedata[] = {
	
	SMART_CMD(0x02),
	SMART_CMD(0x02),

	
	SMART_CMD_WRITE_FRAME,
};

static void ltm020d550_lcd_power(int on, struct fb_var_screeninfo *var)
{
	struct fb_info *info = container_of(var, struct fb_info, var);

	if (on) {
		pxafb_smart_queue(info, ARRAY_AND_SIZE(panel_init));
		pxafb_smart_queue(info, ARRAY_AND_SIZE(panel_on));
	} else {
		pxafb_smart_queue(info, ARRAY_AND_SIZE(panel_off));
	}

	if (pxafb_smart_flush(info))
		pr_err("%s: timed out\n", __func__);
}

static void ltm020d550_update(struct fb_info *info)
{
	pxafb_smart_queue(info, ARRAY_AND_SIZE(update_framedata));
	pxafb_smart_flush(info);
}

static struct pxafb_mode_info toshiba_ltm020d550_modes[] = {
	[0] = {
		.xres			= 240,
		.yres			= 320,
		.bpp			= 16,
		.a0csrd_set_hld		= 30,
		.a0cswr_set_hld		= 30,
		.wr_pulse_width		= 30,
		.rd_pulse_width 	= 170,
		.op_hold_time 		= 30,
		.cmd_inh_time		= 60,

		
		.sync			= FB_SYNC_HOR_HIGH_ACT |
					  FB_SYNC_VERT_HIGH_ACT,
	},
};

static struct pxafb_mach_info tavorevb_lcd_info = {
	.modes			= toshiba_ltm020d550_modes,
	.num_modes		= 1,
	.lcd_conn		= LCD_SMART_PANEL_8BPP | LCD_PCLK_EDGE_FALL,
	.pxafb_lcd_power	= ltm020d550_lcd_power,
	.smart_update		= ltm020d550_update,
};

static void __init tavorevb_init_lcd(void)
{
	platform_device_register(&tavorevb_backlight_devices[0]);
	platform_device_register(&tavorevb_backlight_devices[1]);
	set_pxa_fb_info(&tavorevb_lcd_info);
}
#else
static inline void tavorevb_init_lcd(void) {}
#endif 

static void __init tavorevb_init(void)
{
	
	pxa3xx_mfp_config(ARRAY_AND_SIZE(tavorevb_mfp_cfg));

	platform_device_register(&smc91x_device);

	tavorevb_init_lcd();
	tavorevb_init_keypad();
}

MACHINE_START(TAVOREVB, "PXA930 Evaluation Board (aka TavorEVB)")
	
	.phys_io        = 0x40000000,
	.boot_params    = 0xa0000100,
	.io_pg_offst    = (io_p2v(0x40000000) >> 18) & 0xfffc,
	.map_io         = pxa_map_io,
	.init_irq       = pxa3xx_init_irq,
	.timer          = &pxa_timer,
	.init_machine   = tavorevb_init,
MACHINE_END
