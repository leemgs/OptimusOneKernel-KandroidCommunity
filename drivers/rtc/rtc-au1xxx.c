



#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/rtc.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <asm/mach-au1x00/au1000.h>


#define CNTR_OK (SYS_CNTRL_E0 | SYS_CNTRL_32S)

static int au1xtoy_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	unsigned long t;

	t = au_readl(SYS_TOYREAD);

	rtc_time_to_tm(t, tm);

	return rtc_valid_tm(tm);
}

static int au1xtoy_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	unsigned long t;

	rtc_tm_to_time(tm, &t);

	au_writel(t, SYS_TOYWRITE);
	au_sync();

	
	while (au_readl(SYS_COUNTER_CNTRL) & SYS_CNTRL_C0S)
		msleep(1);

	return 0;
}

static struct rtc_class_ops au1xtoy_rtc_ops = {
	.read_time	= au1xtoy_rtc_read_time,
	.set_time	= au1xtoy_rtc_set_time,
};

static int __devinit au1xtoy_rtc_probe(struct platform_device *pdev)
{
	struct rtc_device *rtcdev;
	unsigned long t;
	int ret;

	t = au_readl(SYS_COUNTER_CNTRL);
	if (!(t & CNTR_OK)) {
		dev_err(&pdev->dev, "counters not working; aborting.\n");
		ret = -ENODEV;
		goto out_err;
	}

	ret = -ETIMEDOUT;

	
	if (au_readl(SYS_TOYTRIM) != 32767) {
		
		t = 0x00100000;
		while ((au_readl(SYS_COUNTER_CNTRL) & SYS_CNTRL_T0S) && --t)
			msleep(1);

		if (!t) {
			
			dev_err(&pdev->dev, "timeout waiting for access\n");
			goto out_err;
		}

		
		au_writel(32767, SYS_TOYTRIM);
		au_sync();
	}

	
	while (au_readl(SYS_COUNTER_CNTRL) & SYS_CNTRL_C0S)
		msleep(1);

	rtcdev = rtc_device_register("rtc-au1xxx", &pdev->dev,
				     &au1xtoy_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtcdev)) {
		ret = PTR_ERR(rtcdev);
		goto out_err;
	}

	platform_set_drvdata(pdev, rtcdev);

	return 0;

out_err:
	return ret;
}

static int __devexit au1xtoy_rtc_remove(struct platform_device *pdev)
{
	struct rtc_device *rtcdev = platform_get_drvdata(pdev);

	rtc_device_unregister(rtcdev);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver au1xrtc_driver = {
	.driver		= {
		.name	= "rtc-au1xxx",
		.owner	= THIS_MODULE,
	},
	.remove		= __devexit_p(au1xtoy_rtc_remove),
};

static int __init au1xtoy_rtc_init(void)
{
	return platform_driver_probe(&au1xrtc_driver, au1xtoy_rtc_probe);
}

static void __exit au1xtoy_rtc_exit(void)
{
	platform_driver_unregister(&au1xrtc_driver);
}

module_init(au1xtoy_rtc_init);
module_exit(au1xtoy_rtc_exit);

MODULE_DESCRIPTION("Au1xxx TOY-counter-based RTC driver");
MODULE_AUTHOR("Manuel Lauss <manuel.lauss@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:rtc-au1xxx");
