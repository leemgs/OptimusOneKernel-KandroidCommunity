

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <asm/system.h>

#define OUR_NAME "w83877f_wdt"
#define PFX OUR_NAME ": "

#define ENABLE_W83877F_PORT 0x3F0
#define ENABLE_W83877F 0x87
#define DISABLE_W83877F 0xAA
#define WDT_PING 0x443
#define WDT_REGISTER 0x14
#define WDT_ENABLE 0x9C
#define WDT_DISABLE 0x8C



#define WDT_INTERVAL (HZ/4+1)



#define WATCHDOG_TIMEOUT 30            

static int timeout = WATCHDOG_TIMEOUT;
module_param(timeout, int, 0);
MODULE_PARM_DESC(timeout,
	"Watchdog timeout in seconds. (1<=timeout<=3600, default="
				__MODULE_STRING(WATCHDOG_TIMEOUT) ")");


static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout,
		"Watchdog cannot be stopped once started (default="
				__MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

static void wdt_timer_ping(unsigned long);
static DEFINE_TIMER(timer, wdt_timer_ping, 0, 0);
static unsigned long next_heartbeat;
static unsigned long wdt_is_open;
static char wdt_expect_close;
static DEFINE_SPINLOCK(wdt_spinlock);



static void wdt_timer_ping(unsigned long data)
{
	
	if (time_before(jiffies, next_heartbeat)) {
		
		spin_lock(&wdt_spinlock);

		
		inb_p(WDT_PING);

		
		mod_timer(&timer, jiffies + WDT_INTERVAL);

		spin_unlock(&wdt_spinlock);

	} else
		printk(KERN_WARNING PFX
			"Heartbeat lost! Will not ping the watchdog\n");
}



static void wdt_change(int writeval)
{
	unsigned long flags;
	spin_lock_irqsave(&wdt_spinlock, flags);

	
	inb_p(WDT_PING);

	
	outb_p(ENABLE_W83877F,  ENABLE_W83877F_PORT);
	outb_p(ENABLE_W83877F,  ENABLE_W83877F_PORT);

	
	outb_p(WDT_REGISTER,    ENABLE_W83877F_PORT);
	outb_p(writeval,        ENABLE_W83877F_PORT+1);

	
	outb_p(DISABLE_W83877F, ENABLE_W83877F_PORT);

	spin_unlock_irqrestore(&wdt_spinlock, flags);
}

static void wdt_startup(void)
{
	next_heartbeat = jiffies + (timeout * HZ);

	
	mod_timer(&timer, jiffies + WDT_INTERVAL);

	wdt_change(WDT_ENABLE);

	printk(KERN_INFO PFX "Watchdog timer is now enabled.\n");
}

static void wdt_turnoff(void)
{
	
	del_timer(&timer);

	wdt_change(WDT_DISABLE);

	printk(KERN_INFO PFX "Watchdog timer is now disabled...\n");
}

static void wdt_keepalive(void)
{
	
	next_heartbeat = jiffies + (timeout * HZ);
}



static ssize_t fop_write(struct file *file, const char __user *buf,
						size_t count, loff_t *ppos)
{
	
	if (count) {
		if (!nowayout) {
			size_t ofs;

			
			wdt_expect_close = 0;

			
			for (ofs = 0; ofs != count; ofs++) {
				char c;
				if (get_user(c, buf + ofs))
					return -EFAULT;
				if (c == 'V')
					wdt_expect_close = 42;
			}
		}

		
		wdt_keepalive();
	}
	return count;
}

static int fop_open(struct inode *inode, struct file *file)
{
	
	if (test_and_set_bit(0, &wdt_is_open))
		return -EBUSY;

	
	wdt_startup();
	return nonseekable_open(inode, file);
}

static int fop_close(struct inode *inode, struct file *file)
{
	if (wdt_expect_close == 42)
		wdt_turnoff();
	else {
		del_timer(&timer);
		printk(KERN_CRIT PFX
		  "device file closed unexpectedly. Will not stop the WDT!\n");
	}
	clear_bit(0, &wdt_is_open);
	wdt_expect_close = 0;
	return 0;
}

static long fop_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	static const struct watchdog_info ident = {
		.options = WDIOF_KEEPALIVEPING | WDIOF_SETTIMEOUT
							| WDIOF_MAGICCLOSE,
		.firmware_version = 1,
		.identity = "W83877F",
	};

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		return copy_to_user(argp, &ident, sizeof(ident)) ? -EFAULT : 0;
	case WDIOC_GETSTATUS:
	case WDIOC_GETBOOTSTATUS:
		return put_user(0, p);
	case WDIOC_SETOPTIONS:
	{
		int new_options, retval = -EINVAL;

		if (get_user(new_options, p))
			return -EFAULT;

		if (new_options & WDIOS_DISABLECARD) {
			wdt_turnoff();
			retval = 0;
		}

		if (new_options & WDIOS_ENABLECARD) {
			wdt_startup();
			retval = 0;
		}

		return retval;
	}
	case WDIOC_KEEPALIVE:
		wdt_keepalive();
		return 0;
	case WDIOC_SETTIMEOUT:
	{
		int new_timeout;

		if (get_user(new_timeout, p))
			return -EFAULT;

		
		if (new_timeout < 1 || new_timeout > 3600)
			return -EINVAL;

		timeout = new_timeout;
		wdt_keepalive();
		
	}
	case WDIOC_GETTIMEOUT:
		return put_user(timeout, p);
	default:
		return -ENOTTY;
	}
}

static const struct file_operations wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= fop_write,
	.open		= fop_open,
	.release	= fop_close,
	.unlocked_ioctl	= fop_ioctl,
};

static struct miscdevice wdt_miscdev = {
	.minor	= WATCHDOG_MINOR,
	.name	= "watchdog",
	.fops	= &wdt_fops,
};



static int wdt_notify_sys(struct notifier_block *this, unsigned long code,
	void *unused)
{
	if (code == SYS_DOWN || code == SYS_HALT)
		wdt_turnoff();
	return NOTIFY_DONE;
}



static struct notifier_block wdt_notifier = {
	.notifier_call = wdt_notify_sys,
};

static void __exit w83877f_wdt_unload(void)
{
	wdt_turnoff();

	
	misc_deregister(&wdt_miscdev);

	unregister_reboot_notifier(&wdt_notifier);
	release_region(WDT_PING, 1);
	release_region(ENABLE_W83877F_PORT, 2);
}

static int __init w83877f_wdt_init(void)
{
	int rc = -EBUSY;

	if (timeout < 1 || timeout > 3600) { 
		timeout = WATCHDOG_TIMEOUT;
		printk(KERN_INFO PFX
			"timeout value must be 1 <= x <= 3600, using %d\n",
							timeout);
	}

	if (!request_region(ENABLE_W83877F_PORT, 2, "W83877F WDT")) {
		printk(KERN_ERR PFX "I/O address 0x%04x already in use\n",
			ENABLE_W83877F_PORT);
		rc = -EIO;
		goto err_out;
	}

	if (!request_region(WDT_PING, 1, "W8387FF WDT")) {
		printk(KERN_ERR PFX "I/O address 0x%04x already in use\n",
			WDT_PING);
		rc = -EIO;
		goto err_out_region1;
	}

	rc = register_reboot_notifier(&wdt_notifier);
	if (rc) {
		printk(KERN_ERR PFX
			"cannot register reboot notifier (err=%d)\n", rc);
		goto err_out_region2;
	}

	rc = misc_register(&wdt_miscdev);
	if (rc) {
		printk(KERN_ERR PFX
			"cannot register miscdev on minor=%d (err=%d)\n",
							wdt_miscdev.minor, rc);
		goto err_out_reboot;
	}

	printk(KERN_INFO PFX
	  "WDT driver for W83877F initialised. timeout=%d sec (nowayout=%d)\n",
		timeout, nowayout);

	return 0;

err_out_reboot:
	unregister_reboot_notifier(&wdt_notifier);
err_out_region2:
	release_region(WDT_PING, 1);
err_out_region1:
	release_region(ENABLE_W83877F_PORT, 2);
err_out:
	return rc;
}

module_init(w83877f_wdt_init);
module_exit(w83877f_wdt_unload);

MODULE_AUTHOR("Scott and Bill Jennings");
MODULE_DESCRIPTION("Driver for watchdog timer in w83877f chip");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
