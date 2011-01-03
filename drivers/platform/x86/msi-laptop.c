





#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/acpi.h>
#include <linux/dmi.h>
#include <linux/backlight.h>
#include <linux/platform_device.h>

#define MSI_DRIVER_VERSION "0.5"

#define MSI_LCD_LEVEL_MAX 9

#define MSI_EC_COMMAND_WIRELESS 0x10
#define MSI_EC_COMMAND_LCD_LEVEL 0x11

static int force;
module_param(force, bool, 0);
MODULE_PARM_DESC(force, "Force driver load, ignore DMI data");

static int auto_brightness;
module_param(auto_brightness, int, 0);
MODULE_PARM_DESC(auto_brightness, "Enable automatic brightness control (0: disabled; 1: enabled; 2: don't touch)");



static int set_lcd_level(int level)
{
	u8 buf[2];

	if (level < 0 || level >= MSI_LCD_LEVEL_MAX)
		return -EINVAL;

	buf[0] = 0x80;
	buf[1] = (u8) (level*31);

	return ec_transaction(MSI_EC_COMMAND_LCD_LEVEL, buf, sizeof(buf), NULL, 0, 1);
}

static int get_lcd_level(void)
{
	u8 wdata = 0, rdata;
	int result;

	result = ec_transaction(MSI_EC_COMMAND_LCD_LEVEL, &wdata, 1, &rdata, 1, 1);
	if (result < 0)
		return result;

	return (int) rdata / 31;
}

static int get_auto_brightness(void)
{
	u8 wdata = 4, rdata;
	int result;

	result = ec_transaction(MSI_EC_COMMAND_LCD_LEVEL, &wdata, 1, &rdata, 1, 1);
	if (result < 0)
		return result;

	return !!(rdata & 8);
}

static int set_auto_brightness(int enable)
{
	u8 wdata[2], rdata;
	int result;

	wdata[0] = 4;

	result = ec_transaction(MSI_EC_COMMAND_LCD_LEVEL, wdata, 1, &rdata, 1, 1);
	if (result < 0)
		return result;

	wdata[0] = 0x84;
	wdata[1] = (rdata & 0xF7) | (enable ? 8 : 0);

	return ec_transaction(MSI_EC_COMMAND_LCD_LEVEL, wdata, 2, NULL, 0, 1);
}

static int get_wireless_state(int *wlan, int *bluetooth)
{
	u8 wdata = 0, rdata;
	int result;

	result = ec_transaction(MSI_EC_COMMAND_WIRELESS, &wdata, 1, &rdata, 1, 1);
	if (result < 0)
		return -1;

	if (wlan)
		*wlan = !!(rdata & 8);

	if (bluetooth)
		*bluetooth = !!(rdata & 128);

	return 0;
}



static int bl_get_brightness(struct backlight_device *b)
{
	return get_lcd_level();
}


static int bl_update_status(struct backlight_device *b)
{
	return set_lcd_level(b->props.brightness);
}

static struct backlight_ops msibl_ops = {
	.get_brightness = bl_get_brightness,
	.update_status  = bl_update_status,
};

static struct backlight_device *msibl_device;



static ssize_t show_wlan(struct device *dev,
	struct device_attribute *attr, char *buf)
{

	int ret, enabled;

	ret = get_wireless_state(&enabled, NULL);
	if (ret < 0)
		return ret;

	return sprintf(buf, "%i\n", enabled);
}

static ssize_t show_bluetooth(struct device *dev,
	struct device_attribute *attr, char *buf)
{

	int ret, enabled;

	ret = get_wireless_state(NULL, &enabled);
	if (ret < 0)
		return ret;

	return sprintf(buf, "%i\n", enabled);
}

static ssize_t show_lcd_level(struct device *dev,
	struct device_attribute *attr, char *buf)
{

	int ret;

	ret = get_lcd_level();
	if (ret < 0)
		return ret;

	return sprintf(buf, "%i\n", ret);
}

static ssize_t store_lcd_level(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{

	int level, ret;

	if (sscanf(buf, "%i", &level) != 1 || (level < 0 || level >= MSI_LCD_LEVEL_MAX))
		return -EINVAL;

	ret = set_lcd_level(level);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t show_auto_brightness(struct device *dev,
	struct device_attribute *attr, char *buf)
{

	int ret;

	ret = get_auto_brightness();
	if (ret < 0)
		return ret;

	return sprintf(buf, "%i\n", ret);
}

static ssize_t store_auto_brightness(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{

	int enable, ret;

	if (sscanf(buf, "%i", &enable) != 1 || (enable != (enable & 1)))
		return -EINVAL;

	ret = set_auto_brightness(enable);
	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(lcd_level, 0644, show_lcd_level, store_lcd_level);
static DEVICE_ATTR(auto_brightness, 0644, show_auto_brightness, store_auto_brightness);
static DEVICE_ATTR(bluetooth, 0444, show_bluetooth, NULL);
static DEVICE_ATTR(wlan, 0444, show_wlan, NULL);

static struct attribute *msipf_attributes[] = {
	&dev_attr_lcd_level.attr,
	&dev_attr_auto_brightness.attr,
	&dev_attr_bluetooth.attr,
	&dev_attr_wlan.attr,
	NULL
};

static struct attribute_group msipf_attribute_group = {
	.attrs = msipf_attributes
};

static struct platform_driver msipf_driver = {
	.driver = {
		.name = "msi-laptop-pf",
		.owner = THIS_MODULE,
	}
};

static struct platform_device *msipf_device;



static int dmi_check_cb(const struct dmi_system_id *id)
{
        printk("msi-laptop: Identified laptop model '%s'.\n", id->ident);
        return 0;
}

static struct dmi_system_id __initdata msi_dmi_table[] = {
	{
		.ident = "MSI S270",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "MICRO-STAR INT'L CO.,LTD"),
			DMI_MATCH(DMI_PRODUCT_NAME, "MS-1013"),
			DMI_MATCH(DMI_PRODUCT_VERSION, "0131"),
			DMI_MATCH(DMI_CHASSIS_VENDOR, "MICRO-STAR INT'L CO.,LTD")
		},
		.callback = dmi_check_cb
	},
	{
		.ident = "MSI S271",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Micro-Star International"),
			DMI_MATCH(DMI_PRODUCT_NAME, "MS-1058"),
			DMI_MATCH(DMI_PRODUCT_VERSION, "0581"),
			DMI_MATCH(DMI_BOARD_NAME, "MS-1058")
		},
		.callback = dmi_check_cb
	},
	{
		.ident = "MSI S420",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Micro-Star International"),
			DMI_MATCH(DMI_PRODUCT_NAME, "MS-1412"),
			DMI_MATCH(DMI_BOARD_VENDOR, "MSI"),
			DMI_MATCH(DMI_BOARD_NAME, "MS-1412")
		},
		.callback = dmi_check_cb
	},
	{
		.ident = "Medion MD96100",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "NOTEBOOK"),
			DMI_MATCH(DMI_PRODUCT_NAME, "SAM2000"),
			DMI_MATCH(DMI_PRODUCT_VERSION, "0131"),
			DMI_MATCH(DMI_CHASSIS_VENDOR, "MICRO-STAR INT'L CO.,LTD")
		},
		.callback = dmi_check_cb
	},
	{ }
};

static int __init msi_init(void)
{
	int ret;

	if (acpi_disabled)
		return -ENODEV;

	if (!force && !dmi_check_system(msi_dmi_table))
		return -ENODEV;

	if (auto_brightness < 0 || auto_brightness > 2)
		return -EINVAL;

	

	if (acpi_video_backlight_support()) {
		printk(KERN_INFO "MSI: Brightness ignored, must be controlled "
		       "by ACPI video driver\n");
	} else {
		msibl_device = backlight_device_register("msi-laptop-bl", NULL,
							 NULL, &msibl_ops);
		if (IS_ERR(msibl_device))
			return PTR_ERR(msibl_device);
		msibl_device->props.max_brightness = MSI_LCD_LEVEL_MAX-1;
	}

	ret = platform_driver_register(&msipf_driver);
	if (ret)
		goto fail_backlight;

	

	msipf_device = platform_device_alloc("msi-laptop-pf", -1);
	if (!msipf_device) {
		ret = -ENOMEM;
		goto fail_platform_driver;
	}

	ret = platform_device_add(msipf_device);
	if (ret)
		goto fail_platform_device1;

	ret = sysfs_create_group(&msipf_device->dev.kobj, &msipf_attribute_group);
	if (ret)
		goto fail_platform_device2;

	

	if (auto_brightness != 2)
		set_auto_brightness(auto_brightness);

	printk(KERN_INFO "msi-laptop: driver "MSI_DRIVER_VERSION" successfully loaded.\n");

	return 0;

fail_platform_device2:

	platform_device_del(msipf_device);

fail_platform_device1:

	platform_device_put(msipf_device);

fail_platform_driver:

	platform_driver_unregister(&msipf_driver);

fail_backlight:

	backlight_device_unregister(msibl_device);

	return ret;
}

static void __exit msi_cleanup(void)
{

	sysfs_remove_group(&msipf_device->dev.kobj, &msipf_attribute_group);
	platform_device_unregister(msipf_device);
	platform_driver_unregister(&msipf_driver);
	backlight_device_unregister(msibl_device);

	
	if (auto_brightness != 2)
		set_auto_brightness(1);

	printk(KERN_INFO "msi-laptop: driver unloaded.\n");
}

module_init(msi_init);
module_exit(msi_cleanup);

MODULE_AUTHOR("Lennart Poettering");
MODULE_DESCRIPTION("MSI Laptop Support");
MODULE_VERSION(MSI_DRIVER_VERSION);
MODULE_LICENSE("GPL");

MODULE_ALIAS("dmi:*:svnMICRO-STARINT'LCO.,LTD:pnMS-1013:pvr0131*:cvnMICRO-STARINT'LCO.,LTD:ct10:*");
MODULE_ALIAS("dmi:*:svnMicro-StarInternational:pnMS-1058:pvr0581:rvnMSI:rnMS-1058:*:ct10:*");
MODULE_ALIAS("dmi:*:svnMicro-StarInternational:pnMS-1412:*:rvnMSI:rnMS-1412:*:cvnMICRO-STARINT'LCO.,LTD:ct10:*");
MODULE_ALIAS("dmi:*:svnNOTEBOOK:pnSAM2000:pvr0131*:cvnMICRO-STARINT'LCO.,LTD:ct10:*");
