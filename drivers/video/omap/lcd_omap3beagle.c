

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/i2c/twl4030.h>

#include <mach/mux.h>
#include <mach/omapfb.h>
#include <asm/mach-types.h>

#define LCD_PANEL_ENABLE_GPIO       170

static int omap3beagle_panel_init(struct lcd_panel *panel,
				struct omapfb_device *fbdev)
{
	gpio_request(LCD_PANEL_ENABLE_GPIO, "LCD enable");
	return 0;
}

static void omap3beagle_panel_cleanup(struct lcd_panel *panel)
{
	gpio_free(LCD_PANEL_ENABLE_GPIO);
}

static int omap3beagle_panel_enable(struct lcd_panel *panel)
{
	gpio_set_value(LCD_PANEL_ENABLE_GPIO, 1);
	return 0;
}

static void omap3beagle_panel_disable(struct lcd_panel *panel)
{
	gpio_set_value(LCD_PANEL_ENABLE_GPIO, 0);
}

static unsigned long omap3beagle_panel_get_caps(struct lcd_panel *panel)
{
	return 0;
}

struct lcd_panel omap3beagle_panel = {
	.name		= "omap3beagle",
	.config		= OMAP_LCDC_PANEL_TFT,

	.bpp		= 16,
	.data_lines	= 24,
	.x_res		= 1024,
	.y_res		= 768,
	.hsw		= 3,		
	.hfp		= 3,		
	.hbp		= 39,		
	.vsw		= 1,		
	.vfp		= 2,		
	.vbp		= 7,		

	.pixel_clock	= 64000,

	.init		= omap3beagle_panel_init,
	.cleanup	= omap3beagle_panel_cleanup,
	.enable		= omap3beagle_panel_enable,
	.disable	= omap3beagle_panel_disable,
	.get_caps	= omap3beagle_panel_get_caps,
};

static int omap3beagle_panel_probe(struct platform_device *pdev)
{
	omapfb_register_panel(&omap3beagle_panel);
	return 0;
}

static int omap3beagle_panel_remove(struct platform_device *pdev)
{
	return 0;
}

static int omap3beagle_panel_suspend(struct platform_device *pdev,
				   pm_message_t mesg)
{
	return 0;
}

static int omap3beagle_panel_resume(struct platform_device *pdev)
{
	return 0;
}

struct platform_driver omap3beagle_panel_driver = {
	.probe		= omap3beagle_panel_probe,
	.remove		= omap3beagle_panel_remove,
	.suspend	= omap3beagle_panel_suspend,
	.resume		= omap3beagle_panel_resume,
	.driver		= {
		.name	= "omap3beagle_lcd",
		.owner	= THIS_MODULE,
	},
};

static int __init omap3beagle_panel_drv_init(void)
{
	return platform_driver_register(&omap3beagle_panel_driver);
}

static void __exit omap3beagle_panel_drv_exit(void)
{
	platform_driver_unregister(&omap3beagle_panel_driver);
}

module_init(omap3beagle_panel_drv_init);
module_exit(omap3beagle_panel_drv_exit);
