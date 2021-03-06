

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c/twl4030.h>

#include <mach/gpio.h>
#include <mach/mux.h>
#include <mach/omapfb.h>
#include <asm/mach-types.h>

#define LCD_ENABLE       144

static int overo_panel_init(struct lcd_panel *panel,
				struct omapfb_device *fbdev)
{
	if ((gpio_request(LCD_ENABLE, "LCD_ENABLE") == 0) &&
	    (gpio_direction_output(LCD_ENABLE, 1) == 0))
		gpio_export(LCD_ENABLE, 0);
	else
		printk(KERN_ERR "could not obtain gpio for LCD_ENABLE\n");

	return 0;
}

static void overo_panel_cleanup(struct lcd_panel *panel)
{
	gpio_free(LCD_ENABLE);
}

static int overo_panel_enable(struct lcd_panel *panel)
{
	gpio_set_value(LCD_ENABLE, 1);
	return 0;
}

static void overo_panel_disable(struct lcd_panel *panel)
{
	gpio_set_value(LCD_ENABLE, 0);
}

static unsigned long overo_panel_get_caps(struct lcd_panel *panel)
{
	return 0;
}

struct lcd_panel overo_panel = {
	.name		= "overo",
	.config		= OMAP_LCDC_PANEL_TFT,
	.bpp		= 16,
	.data_lines	= 24,

#if defined CONFIG_FB_OMAP_031M3R

	
	.x_res		= 640,
	.y_res		= 480,
	.hfp		= 48,
	.hsw		= 32,
	.hbp		= 80,
	.vfp		= 3,
	.vsw		= 4,
	.vbp		= 7,
	.pixel_clock	= 23500,

#elif defined CONFIG_FB_OMAP_048M3R

	
	.x_res		= 800,
	.y_res		= 600,
	.hfp		= 48,
	.hsw		= 32,
	.hbp		= 80,
	.vfp		= 3,
	.vsw		= 4,
	.vbp		= 11,
	.pixel_clock	= 35500,

#elif defined CONFIG_FB_OMAP_079M3R

	
	.x_res		= 1024,
	.y_res		= 768,
	.hfp		= 48,
	.hsw		= 32,
	.hbp		= 80,
	.vfp		= 3,
	.vsw		= 4,
	.vbp		= 15,
	.pixel_clock	= 56000,

#elif defined CONFIG_FB_OMAP_092M9R

	
	.x_res		= 1280,
	.y_res		= 720,
	.hfp		= 48,
	.hsw		= 32,
	.hbp		= 80,
	.vfp		= 3,
	.vsw		= 5,
	.vbp		= 13,
	.pixel_clock	= 64000,

#else

	
	
	.x_res		= 640,
	.y_res		= 480,
	.hfp		= 48,
	.hsw		= 32,
	.hbp		= 80,
	.vfp		= 3,
	.vsw		= 4,
	.vbp		= 7,
	.pixel_clock	= 23500,

#endif

	.init		= overo_panel_init,
	.cleanup	= overo_panel_cleanup,
	.enable		= overo_panel_enable,
	.disable	= overo_panel_disable,
	.get_caps	= overo_panel_get_caps,
};

static int overo_panel_probe(struct platform_device *pdev)
{
	omapfb_register_panel(&overo_panel);
	return 0;
}

static int overo_panel_remove(struct platform_device *pdev)
{
	
	return 0;
}

static struct platform_driver overo_panel_driver = {
	.probe		= overo_panel_probe,
	.remove		= overo_panel_remove,
	.driver		= {
		.name	= "overo_lcd",
		.owner	= THIS_MODULE,
	},
};

static int __init overo_panel_drv_init(void)
{
	return platform_driver_register(&overo_panel_driver);
}

static void __exit overo_panel_drv_exit(void)
{
	platform_driver_unregister(&overo_panel_driver);
}

module_init(overo_panel_drv_init);
module_exit(overo_panel_drv_exit);
