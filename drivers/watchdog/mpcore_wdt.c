
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/fs.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>

#include <asm/hardware/arm_twd.h>

struct mpcore_wdt {
	unsigned long	timer_alive;
	struct device	*dev;
	void __iomem	*base;
	int		irq;
	unsigned int	perturb;
	char		expect_close;
};

static struct platform_device *mpcore_wdt_dev;
extern unsigned int mpcore_timer_rate;

#define TIMER_MARGIN	60
static int mpcore_margin = TIMER_MARGIN;
module_param(mpcore_margin, int, 0);
MODULE_PARM_DESC(mpcore_margin,
	"MPcore timer margin in seconds. (0 < mpcore_margin < 65536, default="
				__MODULE_STRING(TIMER_MARGIN) ")");

static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout,
	"Watchdog cannot be stopped once started (default="
				__MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

#define ONLY_TESTING	0
static int mpcore_noboot = ONLY_TESTING;
module_param(mpcore_noboot, int, 0);
MODULE_PARM_DESC(mpcore_noboot, "MPcore watchdog action, "
	"set to 1 to ignore reboots, 0 to reboot (default="
					__MODULE_STRING(ONLY_TESTING) ")");


static irqreturn_t mpcore_wdt_fire(int irq, void *arg)
{
	struct mpcore_wdt *wdt = arg;

	
	if (readl(wdt->base + TWD_WDOG_INTSTAT)) {
		dev_printk(KERN_CRIT, wdt->dev,
					"Triggered - Reboot ignored.\n");
		
		writel(1, wdt->base + TWD_WDOG_INTSTAT);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}


static void mpcore_wdt_keepalive(struct mpcore_wdt *wdt)
{
	unsigned int count;

	
	count = (mpcore_timer_rate / 256) * mpcore_margin;

	
	spin_lock(&wdt_lock);
	writel(count + wdt->perturb, wdt->base + TWD_WDOG_LOAD);
	wdt->perturb = wdt->perturb ? 0 : 1;
	spin_unlock(&wdt_lock);
}

static void mpcore_wdt_stop(struct mpcore_wdt *wdt)
{
	spin_lock(&wdt_lock);
	writel(0x12345678, wdt->base + TWD_WDOG_DISABLE);
	writel(0x87654321, wdt->base + TWD_WDOG_DISABLE);
	writel(0x0, wdt->base + TWD_WDOG_CONTROL);
	spin_unlock(&wdt_lock);
}

static void mpcore_wdt_start(struct mpcore_wdt *wdt)
{
	dev_printk(KERN_INFO, wdt->dev, "enabling watchdog.\n");

	spin_lock(&wdt_lock);
	
	mpcore_wdt_keepalive(wdt);

	if (mpcore_noboot) {
		
		writel(0x0000FF01, wdt->base + TWD_WDOG_CONTROL);
	} else {
		
		writel(0x0000FF09, wdt->base + TWD_WDOG_CONTROL);
	}
	spin_unlock(&wdt_lock);
}

static int mpcore_wdt_set_heartbeat(int t)
{
	if (t < 0x0001 || t > 0xFFFF)
		return -EINVAL;

	mpcore_margin = t;
	return 0;
}


static int mpcore_wdt_open(struct inode *inode, struct file *file)
{
	struct mpcore_wdt *wdt = platform_get_drvdata(mpcore_wdt_dev);

	if (test_and_set_bit(0, &wdt->timer_alive))
		return -EBUSY;

	if (nowayout)
		__module_get(THIS_MODULE);

	file->private_data = wdt;

	
	mpcore_wdt_start(wdt);

	return nonseekable_open(inode, file);
}

static int mpcore_wdt_release(struct inode *inode, struct file *file)
{
	struct mpcore_wdt *wdt = file->private_data;

	
	if (wdt->expect_close == 42)
		mpcore_wdt_stop(wdt);
	else {
		dev_printk(KERN_CRIT, wdt->dev,
				"unexpected close, not stopping watchdog!\n");
		mpcore_wdt_keepalive(wdt);
	}
	clear_bit(0, &wdt->timer_alive);
	wdt->expect_close = 0;
	return 0;
}

static ssize_t mpcore_wdt_write(struct file *file, const char *data,
						size_t len, loff_t *ppos)
{
	struct mpcore_wdt *wdt = file->private_data;

	
	if (len) {
		if (!nowayout) {
			size_t i;

			
			wdt->expect_close = 0;

			for (i = 0; i != len; i++) {
				char c;

				if (get_user(c, data + i))
					return -EFAULT;
				if (c == 'V')
					wdt->expect_close = 42;
			}
		}
		mpcore_wdt_keepalive(wdt);
	}
	return len;
}

static struct watchdog_info ident = {
	.options		= WDIOF_SETTIMEOUT |
				  WDIOF_KEEPALIVEPING |
				  WDIOF_MAGICCLOSE,
	.identity		= "MPcore Watchdog",
};

static long mpcore_wdt_ioctl(struct file *file, unsigned int cmd,
							unsigned long arg)
{
	struct mpcore_wdt *wdt = file->private_data;
	int ret;
	union {
		struct watchdog_info ident;
		int i;
	} uarg;

	if (_IOC_DIR(cmd) && _IOC_SIZE(cmd) > sizeof(uarg))
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_WRITE) {
		ret = copy_from_user(&uarg, (void __user *)arg, _IOC_SIZE(cmd));
		if (ret)
			return -EFAULT;
	}

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		uarg.ident = ident;
		ret = 0;
		break;

	case WDIOC_GETSTATUS:
	case WDIOC_GETBOOTSTATUS:
		uarg.i = 0;
		ret = 0;
		break;

	case WDIOC_SETOPTIONS:
		ret = -EINVAL;
		if (uarg.i & WDIOS_DISABLECARD) {
			mpcore_wdt_stop(wdt);
			ret = 0;
		}
		if (uarg.i & WDIOS_ENABLECARD) {
			mpcore_wdt_start(wdt);
			ret = 0;
		}
		break;

	case WDIOC_KEEPALIVE:
		mpcore_wdt_keepalive(wdt);
		ret = 0;
		break;

	case WDIOC_SETTIMEOUT:
		ret = mpcore_wdt_set_heartbeat(uarg.i);
		if (ret)
			break;

		mpcore_wdt_keepalive(wdt);
		
	case WDIOC_GETTIMEOUT:
		uarg.i = mpcore_margin;
		ret = 0;
		break;

	default:
		return -ENOTTY;
	}

	if (ret == 0 && _IOC_DIR(cmd) & _IOC_READ) {
		ret = copy_to_user((void __user *)arg, &uarg, _IOC_SIZE(cmd));
		if (ret)
			ret = -EFAULT;
	}
	return ret;
}


static void mpcore_wdt_shutdown(struct platform_device *dev)
{
	struct mpcore_wdt *wdt = platform_get_drvdata(dev);

	if (system_state == SYSTEM_RESTART || system_state == SYSTEM_HALT)
		mpcore_wdt_stop(wdt);
}


static const struct file_operations mpcore_wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= mpcore_wdt_write,
	.unlocked_ioctl	= mpcore_wdt_ioctl,
	.open		= mpcore_wdt_open,
	.release	= mpcore_wdt_release,
};

static struct miscdevice mpcore_wdt_miscdev = {
	.minor		= WATCHDOG_MINOR,
	.name		= "watchdog",
	.fops		= &mpcore_wdt_fops,
};

static int __devinit mpcore_wdt_probe(struct platform_device *dev)
{
	struct mpcore_wdt *wdt;
	struct resource *res;
	int ret;

	
	if (dev->id != -1)
		return -ENODEV;

	res = platform_get_resource(dev, IORESOURCE_MEM, 0);
	if (!res) {
		ret = -ENODEV;
		goto err_out;
	}

	wdt = kzalloc(sizeof(struct mpcore_wdt), GFP_KERNEL);
	if (!wdt) {
		ret = -ENOMEM;
		goto err_out;
	}

	wdt->dev = &dev->dev;
	wdt->irq = platform_get_irq(dev, 0);
	if (wdt->irq < 0) {
		ret = -ENXIO;
		goto err_free;
	}
	wdt->base = ioremap(res->start, res->end - res->start + 1);
	if (!wdt->base) {
		ret = -ENOMEM;
		goto err_free;
	}

	mpcore_wdt_miscdev.parent = &dev->dev;
	ret = misc_register(&mpcore_wdt_miscdev);
	if (ret) {
		dev_printk(KERN_ERR, _dev,
			"cannot register miscdev on minor=%d (err=%d)\n",
							WATCHDOG_MINOR, ret);
		goto err_misc;
	}

	ret = request_irq(wdt->irq, mpcore_wdt_fire, IRQF_DISABLED,
							"mpcore_wdt", wdt);
	if (ret) {
		dev_printk(KERN_ERR, _dev,
			"cannot register IRQ%d for watchdog\n", wdt->irq);
		goto err_irq;
	}

	mpcore_wdt_stop(wdt);
	platform_set_drvdata(&dev->dev, wdt);
	mpcore_wdt_dev = dev;

	return 0;

err_irq:
	misc_deregister(&mpcore_wdt_miscdev);
err_misc:
	iounmap(wdt->base);
err_free:
	kfree(wdt);
err_out:
	return ret;
}

static int __devexit mpcore_wdt_remove(struct platform_device *dev)
{
	struct mpcore_wdt *wdt = platform_get_drvdata(dev);

	platform_set_drvdata(dev, NULL);

	misc_deregister(&mpcore_wdt_miscdev);

	mpcore_wdt_dev = NULL;

	free_irq(wdt->irq, wdt);
	iounmap(wdt->base);
	kfree(wdt);
	return 0;
}


MODULE_ALIAS("platform:mpcore_wdt");

static struct platform_driver mpcore_wdt_driver = {
	.probe		= mpcore_wdt_probe,
	.remove		= __devexit_p(mpcore_wdt_remove),
	.shutdown	= mpcore_wdt_shutdown,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "mpcore_wdt",
	},
};

static char banner[] __initdata = KERN_INFO "MPcore Watchdog Timer: 0.1. "
		"mpcore_noboot=%d mpcore_margin=%d sec (nowayout= %d)\n";

static int __init mpcore_wdt_init(void)
{
	
	if (mpcore_wdt_set_heartbeat(mpcore_margin)) {
		mpcore_wdt_set_heartbeat(TIMER_MARGIN);
		printk(KERN_INFO "mpcore_margin value must be 0 < mpcore_margin < 65536, using %d\n",
			TIMER_MARGIN);
	}

	printk(banner, mpcore_noboot, mpcore_margin, nowayout);

	return platform_driver_register(&mpcore_wdt_driver);
}

static void __exit mpcore_wdt_exit(void)
{
	platform_driver_unregister(&mpcore_wdt_driver);
}

module_init(mpcore_wdt_init);
module_exit(mpcore_wdt_exit);

MODULE_AUTHOR("ARM Limited");
MODULE_DESCRIPTION("MPcore Watchdog Device Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
