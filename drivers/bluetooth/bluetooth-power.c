


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/rfkill.h>

#if defined (CONFIG_MACH_LGE)
#include <mach/board_lge.h>

static struct bluetooth_platform_data *bt_platform_data = 0;
#else 
static int bluetooth_toggle_radio(void *data, bool blocked)
{
	int ret;
	int (*power_control)(int enable);

	power_control = data;
	ret = (*power_control)(!blocked);
	return ret;
}
#endif 

#if defined (CONFIG_MACH_LGE)
static struct rfkill_ops bluetooth_power_rfkill_ops;
#else
static const struct rfkill_ops bluetooth_power_rfkill_ops = {
	.set_block = bluetooth_toggle_radio,
};
#endif

static int bluetooth_power_rfkill_probe(struct platform_device *pdev)
{
	struct rfkill *rfkill;
	int ret;


#if defined (CONFIG_MACH_LGE)
	bluetooth_power_rfkill_ops.set_block = bt_platform_data->bluetooth_toggle_radio;
#endif
	rfkill = rfkill_alloc("bt_power", &pdev->dev, RFKILL_TYPE_BLUETOOTH,
			      &bluetooth_power_rfkill_ops,
			      pdev->dev.platform_data);

	if (!rfkill) {
		printk(KERN_DEBUG
			"%s: rfkill allocate failed\n", __func__);
		return -ENOMEM;
	}

	
	rfkill_init_sw_state(rfkill, 1);

	ret = rfkill_register(rfkill);
	if (ret) {
		printk(KERN_DEBUG
			"%s: rfkill register failed=%d\n", __func__,
			ret);
		rfkill_destroy(rfkill);
		return ret;
	}

	platform_set_drvdata(pdev, rfkill);

	return 0;
}

static void bluetooth_power_rfkill_remove(struct platform_device *pdev)
{
	struct rfkill *rfkill;

	rfkill = platform_get_drvdata(pdev);
	if (rfkill)
		rfkill_unregister(rfkill);
	rfkill_destroy(rfkill);
	platform_set_drvdata(pdev, NULL);
}

static int __init bt_power_probe(struct platform_device *pdev)
{
	int ret = 0;

	printk(KERN_DEBUG "%s\n", __func__);

	if (!pdev->dev.platform_data) {
		printk(KERN_ERR "%s: platform data not initialized\n",
				__func__);
		return -ENOSYS;
	}
#if defined (CONFIG_MACH_LGE)	
	bt_platform_data = (struct bluetooth_platform_data *)pdev->dev.platform_data;
#endif
	ret = bluetooth_power_rfkill_probe(pdev);

	return ret;
}

static int __devexit bt_power_remove(struct platform_device *pdev)
{
	printk(KERN_DEBUG "%s\n", __func__);

	bluetooth_power_rfkill_remove(pdev);

	return 0;
}

static struct platform_driver bt_power_driver = {
	.probe = bt_power_probe,
	.remove = __devexit_p(bt_power_remove),
	.driver = {
		.name = "bt_power",
		.owner = THIS_MODULE,
	},
};

static int __init bluetooth_power_init(void)
{
	int ret;

	printk(KERN_DEBUG "%s\n", __func__);
	ret = platform_driver_register(&bt_power_driver);
	return ret;
}

static void __exit bluetooth_power_exit(void)
{
	printk(KERN_DEBUG "%s\n", __func__);
	platform_driver_unregister(&bt_power_driver);
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("MSM Bluetooth power control driver");
MODULE_VERSION("1.30");

module_init(bluetooth_power_init);
module_exit(bluetooth_power_exit);
