
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>

#include <mach/pmic.h>

#if defined (CONFIG_LGE_UNIFIED_LED)
#include <mach/board_lge.h>

struct msm_pmic_leds_pdata *leds_pdata = 0;
#endif


#define MAX_KEYPAD_BL_LEVEL	16	
#define TUNED_MAX_KEYPAD_BL_LEVEL	40	


static void msm_keypad_bl_led_set(struct led_classdev *led_cdev,
	enum led_brightness value)
{
	int ret;

#if defined (CONFIG_LGE_UNIFIED_LED)
	ret = leds_pdata->msm_keypad_led_set(value / TUNED_MAX_KEYPAD_BL_LEVEL);
#else	
#ifdef CONFIG_MACH_MSM7X27_THUNDERA
	
	ret = pmic_set_led_intensity(LED_KEYPAD, value);
#else
#if defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)
	ret = pmic_set_led_intensity(LED_KEYPAD, value?1:0);
#else
	ret = pmic_set_led_intensity(LED_KEYPAD, value / TUNED_MAX_KEYPAD_BL_LEVEL);
#endif
#endif 
#endif
	if (ret)
		dev_err(led_cdev->dev, "can't set keypad backlight\n");
}

static struct led_classdev msm_kp_bl_led = {
	.name			= "keyboard-backlight",
	.brightness_set		= msm_keypad_bl_led_set,
	.brightness		= LED_OFF,
};

static int msm_pmic_led_probe(struct platform_device *pdev)
{
	int rc;
#if defined (CONFIG_LGE_UNIFIED_LED)
	leds_pdata = pdev->dev.platform_data;
#endif

#ifndef CONFIG_LGE_UNIFIED_LED
	if (pdev->dev.platform_data)
		msm_kp_bl_led.name = pdev->dev.platform_data;
#endif

	rc = led_classdev_register(&pdev->dev, &msm_kp_bl_led);
	if (rc) {
		dev_err(&pdev->dev, "unable to register led class driver\n");
		return rc;
	}
	msm_keypad_bl_led_set(&msm_kp_bl_led, LED_OFF);
#if defined (CONFIG_LGE_UNIFIED_LED)
	leds_pdata->register_custom_leds(pdev);
#endif
	return rc;
}

static int __devexit msm_pmic_led_remove(struct platform_device *pdev)
{
	led_classdev_unregister(&msm_kp_bl_led);
#if defined (CONFIG_LGE_UNIFIED_LED)
	leds_pdata->unregister_custom_leds();
#endif

	return 0;
}

#ifdef CONFIG_PM
static int msm_pmic_led_suspend(struct platform_device *dev,
		pm_message_t state)
{
	led_classdev_suspend(&msm_kp_bl_led);

#if defined (CONFIG_LGE_UNIFIED_LED)
	leds_pdata->suspend_custom_leds();
#endif
	return 0;
}

static int msm_pmic_led_resume(struct platform_device *dev)
{
	led_classdev_resume(&msm_kp_bl_led);
#if defined (CONFIG_LGE_UNIFIED_LED)
	leds_pdata->resume_custom_leds();
#endif

	return 0;
}
#else
#define msm_pmic_led_suspend NULL
#define msm_pmic_led_resume NULL
#endif

static struct platform_driver msm_pmic_led_driver = {
	.probe		= msm_pmic_led_probe,
	.remove		= __devexit_p(msm_pmic_led_remove),
	.suspend	= msm_pmic_led_suspend,
	.resume		= msm_pmic_led_resume,
	.driver		= {
		.name	= "pmic-leds",
		.owner	= THIS_MODULE,
	},
};

static int __init msm_pmic_led_init(void)
{
	return platform_driver_register(&msm_pmic_led_driver);
}
module_init(msm_pmic_led_init);

static void __exit msm_pmic_led_exit(void)
{
	platform_driver_unregister(&msm_pmic_led_driver);
}
module_exit(msm_pmic_led_exit);

MODULE_DESCRIPTION("MSM PMIC LEDs driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:pmic-leds");
