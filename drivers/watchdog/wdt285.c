

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/irq.h>
#include <mach/hardware.h>

#include <asm/mach-types.h>
#include <asm/hardware/dec21285.h>


#undef ONLY_TESTING

static unsigned int soft_margin = 60;		
static unsigned int reload;
static unsigned long timer_alive;

#ifdef ONLY_TESTING

static void watchdog_fire(int irq, void *dev_id)
{
	printk(KERN_CRIT "Watchdog: Would Reboot.\n");
	*CSR_TIMER4_CNTL = 0;
	*CSR_TIMER4_CLR = 0;
}
#endif


static void watchdog_ping(void)
{
	*CSR_TIMER4_LOAD = reload;
}


static int watchdog_open(struct inode *inode, struct file *file)
{
	int ret;

	if (*CSR_SA110_CNTL & (1 << 13))
		return -EBUSY;

	if (test_and_set_bit(1, &timer_alive))
		return -EBUSY;

	reload = soft_margin * (mem_fclk_21285 / 256);

	*CSR_TIMER4_CLR = 0;
	watchdog_ping();
	*CSR_TIMER4_CNTL = TIMER_CNTL_ENABLE | TIMER_CNTL_AUTORELOAD
		| TIMER_CNTL_DIV256;

#ifdef ONLY_TESTING
	ret = request_irq(IRQ_TIMER4, watchdog_fire, 0, "watchdog", NULL);
	if (ret) {
		*CSR_TIMER4_CNTL = 0;
		clear_bit(1, &timer_alive);
	}
#else
	
	*CSR_SA110_CNTL |= 1 << 13;

	ret = 0;
#endif
	nonseekable_open(inode, file);
	return ret;
}


static int watchdog_release(struct inode *inode, struct file *file)
{
#ifdef ONLY_TESTING
	free_irq(IRQ_TIMER4, NULL);
	clear_bit(1, &timer_alive);
#endif
	return 0;
}

static ssize_t watchdog_write(struct file *file, const char __user *data,
			      size_t len, loff_t *ppos)
{
	
	if (len)
		watchdog_ping();

	return len;
}

static const struct watchdog_info ident = {
	.options	= WDIOF_SETTIMEOUT,
	.identity	= "Footbridge Watchdog",
};

static long watchdog_ioctl(struct file *file, unsigned int cmd,
			   unsigned long arg)
{
	unsigned int new_margin;
	int __user *int_arg = (int __user *)arg;
	int ret = -ENOTTY;

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		ret = 0;
		if (copy_to_user((void __user *)arg, &ident, sizeof(ident)))
			ret = -EFAULT;
		break;

	case WDIOC_GETSTATUS:
	case WDIOC_GETBOOTSTATUS:
		ret = put_user(0, int_arg);
		break;

	case WDIOC_KEEPALIVE:
		watchdog_ping();
		ret = 0;
		break;

	case WDIOC_SETTIMEOUT:
		ret = get_user(new_margin, int_arg);
		if (ret)
			break;

		
		if (new_margin < 0 || new_margin > 60) {
			ret = -EINVAL;
			break;
		}

		soft_margin = new_margin;
		reload = soft_margin * (mem_fclk_21285 / 256);
		watchdog_ping();
		
	case WDIOC_GETTIMEOUT:
		ret = put_user(soft_margin, int_arg);
		break;
	}
	return ret;
}

static const struct file_operations watchdog_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= watchdog_write,
	.unlocked_ioctl	= watchdog_ioctl,
	.open		= watchdog_open,
	.release	= watchdog_release,
};

static struct miscdevice watchdog_miscdev = {
	.minor		= WATCHDOG_MINOR,
	.name		= "watchdog",
	.fops		= &watchdog_fops,
};

static int __init footbridge_watchdog_init(void)
{
	int retval;

	if (machine_is_netwinder())
		return -ENODEV;

	retval = misc_register(&watchdog_miscdev);
	if (retval < 0)
		return retval;

	printk(KERN_INFO
		"Footbridge Watchdog Timer: 0.01, timer margin: %d sec\n",
								soft_margin);

	if (machine_is_cats())
		printk(KERN_WARNING
		  "Warning: Watchdog reset may not work on this machine.\n");
	return 0;
}

static void __exit footbridge_watchdog_exit(void)
{
	misc_deregister(&watchdog_miscdev);
}

MODULE_AUTHOR("Phil Blundell <pb@nexus.co.uk>");
MODULE_DESCRIPTION("Footbridge watchdog driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);

module_param(soft_margin, int, 0);
MODULE_PARM_DESC(soft_margin, "Watchdog timeout in seconds");

module_init(footbridge_watchdog_init);
module_exit(footbridge_watchdog_exit);
