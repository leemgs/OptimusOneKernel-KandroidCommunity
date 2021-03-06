

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c/twl4030.h>

#include <mach/mux.h>
#include <mach/omapfb.h>
#include <asm/mach-types.h>

#define SDP2430_LCD_PANEL_BACKLIGHT_GPIO	91
#define SDP2430_LCD_PANEL_ENABLE_GPIO		154
#define SDP3430_LCD_PANEL_BACKLIGHT_GPIO	24
#define SDP3430_LCD_PANEL_ENABLE_GPIO		28

static unsigned backlight_gpio;
static unsigned enable_gpio;

#define LCD_PIXCLOCK_MAX		5400 
#define PM_RECEIVER             TWL4030_MODULE_PM_RECEIVER
#define ENABLE_VAUX2_DEDICATED  0x09
#define ENABLE_VAUX2_DEV_GRP    0x20
#define ENABLE_VAUX3_DEDICATED	0x03
#define ENABLE_VAUX3_DEV_GRP	0x20

#define ENABLE_VPLL2_DEDICATED          0x05
#define ENABLE_VPLL2_DEV_GRP            0xE0
#define TWL4030_VPLL2_DEV_GRP           0x33
#define TWL4030_VPLL2_DEDICATED         0x36

#define t2_out(c, r, v) twl4030_i2c_write_u8(c, r, v)


static int sdp2430_panel_init(struct lcd_panel *panel,
				struct omapfb_device *fbdev)
{
	if (machine_is_omap_3430sdp()) {
		enable_gpio    = SDP3430_LCD_PANEL_ENABLE_GPIO;
		backlight_gpio = SDP3430_LCD_PANEL_BACKLIGHT_GPIO;
	} else {
		enable_gpio    = SDP2430_LCD_PANEL_ENABLE_GPIO;
		backlight_gpio = SDP2430_LCD_PANEL_BACKLIGHT_GPIO;
	}

	gpio_request(enable_gpio, "LCD enable");	
	gpio_request(backlight_gpio, "LCD bl");		
	gpio_direction_output(enable_gpio, 0);
	gpio_direction_output(backlight_gpio, 0);

	return 0;
}

static void sdp2430_panel_cleanup(struct lcd_panel *panel)
{
	gpio_free(backlight_gpio);
	gpio_free(enable_gpio);
}

static int sdp2430_panel_enable(struct lcd_panel *panel)
{
	u8 ded_val, ded_reg;
	u8 grp_val, grp_reg;

	if (machine_is_omap_3430sdp()) {
		ded_reg = TWL4030_VAUX3_DEDICATED;
		ded_val = ENABLE_VAUX3_DEDICATED;
		grp_reg = TWL4030_VAUX3_DEV_GRP;
		grp_val = ENABLE_VAUX3_DEV_GRP;

		if (omap_rev() > OMAP3430_REV_ES1_0) {
			t2_out(PM_RECEIVER, ENABLE_VPLL2_DEDICATED,
					TWL4030_VPLL2_DEDICATED);
			t2_out(PM_RECEIVER, ENABLE_VPLL2_DEV_GRP,
					TWL4030_VPLL2_DEV_GRP);
		}
	} else {
		ded_reg = TWL4030_VAUX2_DEDICATED;
		ded_val = ENABLE_VAUX2_DEDICATED;
		grp_reg = TWL4030_VAUX2_DEV_GRP;
		grp_val = ENABLE_VAUX2_DEV_GRP;
	}

	gpio_set_value(enable_gpio, 1);
	gpio_set_value(backlight_gpio, 1);

	if (0 != t2_out(PM_RECEIVER, ded_val, ded_reg))
		return -EIO;
	if (0 != t2_out(PM_RECEIVER, grp_val, grp_reg))
		return -EIO;

	return 0;
}

static void sdp2430_panel_disable(struct lcd_panel *panel)
{
	gpio_set_value(enable_gpio, 0);
	gpio_set_value(backlight_gpio, 0);
	if (omap_rev() > OMAP3430_REV_ES1_0) {
		t2_out(PM_RECEIVER, 0x0, TWL4030_VPLL2_DEDICATED);
		t2_out(PM_RECEIVER, 0x0, TWL4030_VPLL2_DEV_GRP);
		msleep(4);
	}
}

static unsigned long sdp2430_panel_get_caps(struct lcd_panel *panel)
{
	return 0;
}

struct lcd_panel sdp2430_panel = {
	.name		= "sdp2430",
	.config		= OMAP_LCDC_PANEL_TFT | OMAP_LCDC_INV_VSYNC |
			  OMAP_LCDC_INV_HSYNC,

	.bpp		= 16,
	.data_lines	= 16,
	.x_res		= 240,
	.y_res		= 320,
	.hsw		= 3,		
	.hfp		= 3,		
	.hbp		= 39,		
	.vsw		= 1,		
	.vfp		= 2,		
	.vbp		= 7,		

	.pixel_clock	= LCD_PIXCLOCK_MAX,

	.init		= sdp2430_panel_init,
	.cleanup	= sdp2430_panel_cleanup,
	.enable		= sdp2430_panel_enable,
	.disable	= sdp2430_panel_disable,
	.get_caps	= sdp2430_panel_get_caps,
};

static int sdp2430_panel_probe(struct platform_device *pdev)
{
	omapfb_register_panel(&sdp2430_panel);
	return 0;
}

static int sdp2430_panel_remove(struct platform_device *pdev)
{
	return 0;
}

static int sdp2430_panel_suspend(struct platform_device *pdev,
					pm_message_t mesg)
{
	return 0;
}

static int sdp2430_panel_resume(struct platform_device *pdev)
{
	return 0;
}

struct platform_driver sdp2430_panel_driver = {
	.probe		= sdp2430_panel_probe,
	.remove		= sdp2430_panel_remove,
	.suspend	= sdp2430_panel_suspend,
	.resume		= sdp2430_panel_resume,
	.driver		= {
		.name	= "sdp2430_lcd",
		.owner	= THIS_MODULE,
	},
};

static int __init sdp2430_panel_drv_init(void)
{
	return platform_driver_register(&sdp2430_panel_driver);
}

static void __exit sdp2430_panel_drv_exit(void)
{
	platform_driver_unregister(&sdp2430_panel_driver);
}

module_init(sdp2430_panel_drv_init);
module_exit(sdp2430_panel_drv_exit);
