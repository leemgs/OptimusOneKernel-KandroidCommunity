

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/dmi.h>
#include <linux/io.h>

static struct backlight_device *mbp_backlight_device;


struct dmi_match_data {
	
	unsigned long iostart;
	unsigned long iolen;
	
	struct backlight_ops backlight_ops;
};


static int debug;
module_param_named(debug, debug, int, 0644);
MODULE_PARM_DESC(debug, "Set to one to enable debugging messages.");


static int intel_chipset_send_intensity(struct backlight_device *bd)
{
	int intensity = bd->props.brightness;

	if (debug)
		printk(KERN_DEBUG "mbp_nvidia_bl: setting brightness to %d\n",
		       intensity);

	outb(0x04 | (intensity << 4), 0xb3);
	outb(0xbf, 0xb2);
	return 0;
}

static int intel_chipset_get_intensity(struct backlight_device *bd)
{
	int intensity;

	outb(0x03, 0xb3);
	outb(0xbf, 0xb2);
	intensity = inb(0xb3) >> 4;

	if (debug)
		printk(KERN_DEBUG "mbp_nvidia_bl: read brightness of %d\n",
		       intensity);

	return intensity;
}

static const struct dmi_match_data intel_chipset_data = {
	.iostart = 0xb2,
	.iolen = 2,
	.backlight_ops	= {
		.options	= BL_CORE_SUSPENDRESUME,
		.get_brightness	= intel_chipset_get_intensity,
		.update_status	= intel_chipset_send_intensity,
	}
};


static int nvidia_chipset_send_intensity(struct backlight_device *bd)
{
	int intensity = bd->props.brightness;

	if (debug)
		printk(KERN_DEBUG "mbp_nvidia_bl: setting brightness to %d\n",
		       intensity);

	outb(0x04 | (intensity << 4), 0x52f);
	outb(0xbf, 0x52e);
	return 0;
}

static int nvidia_chipset_get_intensity(struct backlight_device *bd)
{
	int intensity;

	outb(0x03, 0x52f);
	outb(0xbf, 0x52e);
	intensity = inb(0x52f) >> 4;

	if (debug)
		printk(KERN_DEBUG "mbp_nvidia_bl: read brightness of %d\n",
		       intensity);

	return intensity;
}

static const struct dmi_match_data nvidia_chipset_data = {
	.iostart = 0x52e,
	.iolen = 2,
	.backlight_ops		= {
		.options	= BL_CORE_SUSPENDRESUME,
		.get_brightness	= nvidia_chipset_get_intensity,
		.update_status	= nvidia_chipset_send_intensity
	}
};


static  struct dmi_match_data *driver_data;

static int mbp_dmi_match(const struct dmi_system_id *id)
{
	driver_data = id->driver_data;

	printk(KERN_INFO "mbp_nvidia_bl: %s detected\n", id->ident);
	return 1;
}

static const struct dmi_system_id __initdata mbp_device_table[] = {
	{
		.callback	= mbp_dmi_match,
		.ident		= "MacBookPro 3,1",
		.matches	= {
			DMI_MATCH(DMI_SYS_VENDOR, "Apple Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "MacBookPro3,1"),
		},
		.driver_data	= (void *)&intel_chipset_data,
	},
	{
		.callback	= mbp_dmi_match,
		.ident		= "MacBookPro 3,2",
		.matches	= {
			DMI_MATCH(DMI_SYS_VENDOR, "Apple Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "MacBookPro3,2"),
		},
		.driver_data	= (void *)&intel_chipset_data,
	},
	{
		.callback	= mbp_dmi_match,
		.ident		= "MacBookPro 4,1",
		.matches	= {
			DMI_MATCH(DMI_SYS_VENDOR, "Apple Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "MacBookPro4,1"),
		},
		.driver_data	= (void *)&intel_chipset_data,
	},
	{
		.callback	= mbp_dmi_match,
		.ident		= "MacBookAir 1,1",
		.matches	= {
			DMI_MATCH(DMI_SYS_VENDOR, "Apple Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "MacBookAir1,1"),
		},
		.driver_data	= (void *)&intel_chipset_data,
	},
	{
		.callback	= mbp_dmi_match,
		.ident		= "MacBook 5,1",
		.matches	= {
			DMI_MATCH(DMI_SYS_VENDOR, "Apple Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "MacBook5,1"),
		},
		.driver_data	= (void *)&nvidia_chipset_data,
	},
	{
		.callback	= mbp_dmi_match,
		.ident		= "MacBook 5,2",
		.matches	= {
			DMI_MATCH(DMI_SYS_VENDOR, "Apple Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "MacBook5,2"),
		},
		.driver_data	= (void *)&nvidia_chipset_data,
	},
	{
		.callback	= mbp_dmi_match,
		.ident		= "MacBookAir 2,1",
		.matches	= {
			DMI_MATCH(DMI_SYS_VENDOR, "Apple Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "MacBookAir2,1"),
		},
		.driver_data	= (void *)&nvidia_chipset_data,
	},
	{
		.callback	= mbp_dmi_match,
		.ident		= "MacBookPro 5,1",
		.matches	= {
			DMI_MATCH(DMI_SYS_VENDOR, "Apple Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "MacBookPro5,1"),
		},
		.driver_data	= (void *)&nvidia_chipset_data,
	},
	{
		.callback	= mbp_dmi_match,
		.ident		= "MacBookPro 5,2",
		.matches	= {
			DMI_MATCH(DMI_SYS_VENDOR, "Apple Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "MacBookPro5,2"),
		},
		.driver_data	= (void *)&nvidia_chipset_data,
	},
	{
		.callback	= mbp_dmi_match,
		.ident		= "MacBookPro 5,5",
		.matches	= {
			DMI_MATCH(DMI_SYS_VENDOR, "Apple Inc."),
			DMI_MATCH(DMI_PRODUCT_NAME, "MacBookPro5,5"),
		},
		.driver_data	= (void *)&nvidia_chipset_data,
	},
	{ }
};

static int __init mbp_init(void)
{
	if (!dmi_check_system(mbp_device_table))
		return -ENODEV;

	if (!request_region(driver_data->iostart, driver_data->iolen, 
						"Macbook Pro backlight"))
		return -ENXIO;

	mbp_backlight_device = backlight_device_register("mbp_backlight",
					NULL, NULL, &driver_data->backlight_ops);
	if (IS_ERR(mbp_backlight_device)) {
		release_region(driver_data->iostart, driver_data->iolen);
		return PTR_ERR(mbp_backlight_device);
	}

	mbp_backlight_device->props.max_brightness = 15;
	mbp_backlight_device->props.brightness =
		driver_data->backlight_ops.get_brightness(mbp_backlight_device);
	backlight_update_status(mbp_backlight_device);

	return 0;
}

static void __exit mbp_exit(void)
{
	backlight_device_unregister(mbp_backlight_device);

	release_region(driver_data->iostart, driver_data->iolen);
}

module_init(mbp_init);
module_exit(mbp_exit);

MODULE_AUTHOR("Matthew Garrett <mjg@redhat.com>");
MODULE_DESCRIPTION("Nvidia-based Macbook Pro Backlight Driver");
MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(dmi, mbp_device_table);
