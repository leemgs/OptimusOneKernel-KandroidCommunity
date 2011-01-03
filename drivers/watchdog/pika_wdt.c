

#include <linux/init.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/reboot.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/bitops.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/of_platform.h>

#define DRV_NAME "PIKA-WDT"
#define PFX DRV_NAME ": "


#define WDT_HW_TIMEOUT 2


#define WDT_TIMEOUT	(HZ/2)


#define WDT_HEARTBEAT 15
static int heartbeat = WDT_HEARTBEAT;
module_param(heartbeat, int, 0);
MODULE_PARM_DESC(heartbeat, "Watchdog heartbeats in seconds. "
	"(default = " __MODULE_STRING(WDT_HEARTBEAT) ")");

static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started "
	"(default=" __MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

static struct {
	void __iomem *fpga;
	unsigned long next_heartbeat;	
	unsigned long open;
	char expect_close;
	int bootstatus;
	struct timer_list timer;	
} pikawdt_private;

static struct watchdog_info ident = {
	.identity	= DRV_NAME,
	.options	= WDIOF_CARDRESET |
			  WDIOF_SETTIMEOUT |
			  WDIOF_KEEPALIVEPING |
			  WDIOF_MAGICCLOSE,
};


static inline void pikawdt_reset(void)
{
	
	unsigned reset = in_be32(pikawdt_private.fpga + 0x14);
	
	reset |= (1 << 7) + (WDT_HW_TIMEOUT << 8);
	out_be32(pikawdt_private.fpga + 0x14, reset);
}


static void pikawdt_ping(unsigned long data)
{
	if (time_before(jiffies, pikawdt_private.next_heartbeat) ||
			(!nowayout && !pikawdt_private.open)) {
		pikawdt_reset();
		mod_timer(&pikawdt_private.timer, jiffies + WDT_TIMEOUT);
	} else
		printk(KERN_CRIT PFX "I will reset your machine !\n");
}


static void pikawdt_keepalive(void)
{
	pikawdt_private.next_heartbeat = jiffies + heartbeat * HZ;
}

static void pikawdt_start(void)
{
	pikawdt_keepalive();
	mod_timer(&pikawdt_private.timer, jiffies + WDT_TIMEOUT);
}


static int pikawdt_open(struct inode *inode, struct file *file)
{
	
	if (test_and_set_bit(0, &pikawdt_private.open))
		return -EBUSY;

	pikawdt_start();

	return nonseekable_open(inode, file);
}


static int pikawdt_release(struct inode *inode, struct file *file)
{
	
	if (!pikawdt_private.expect_close)
		del_timer(&pikawdt_private.timer);

	clear_bit(0, &pikawdt_private.open);
	pikawdt_private.expect_close = 0;
	return 0;
}


static ssize_t pikawdt_write(struct file *file, const char __user *data,
			     size_t len, loff_t *ppos)
{
	if (!len)
		return 0;

	
	if (!nowayout) {
		size_t i;

		pikawdt_private.expect_close = 0;

		for (i = 0; i < len; i++) {
			char c;
			if (get_user(c, data + i))
				return -EFAULT;
			if (c == 'V') {
				pikawdt_private.expect_close = 42;
				break;
			}
		}
	}

	pikawdt_keepalive();

	return len;
}


static long pikawdt_ioctl(struct file *file,
		unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	int new_value;

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		return copy_to_user(argp, &ident, sizeof(ident)) ? -EFAULT : 0;

	case WDIOC_GETSTATUS:
		return put_user(0, p);

	case WDIOC_GETBOOTSTATUS:
		return put_user(pikawdt_private.bootstatus, p);

	case WDIOC_KEEPALIVE:
		pikawdt_keepalive();
		return 0;

	case WDIOC_SETTIMEOUT:
		if (get_user(new_value, p))
			return -EFAULT;

		heartbeat = new_value;
		pikawdt_keepalive();

		return put_user(new_value, p);  

	case WDIOC_GETTIMEOUT:
		return put_user(heartbeat, p);
	}
	return -ENOTTY;
}


static const struct file_operations pikawdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.open		= pikawdt_open,
	.release	= pikawdt_release,
	.write		= pikawdt_write,
	.unlocked_ioctl	= pikawdt_ioctl,
};

static struct miscdevice pikawdt_miscdev = {
	.minor	= WATCHDOG_MINOR,
	.name	= "watchdog",
	.fops	= &pikawdt_fops,
};

static int __init pikawdt_init(void)
{
	struct device_node *np;
	void __iomem *fpga;
	static u32 post1;
	int ret;

	np = of_find_compatible_node(NULL, NULL, "pika,fpga");
	if (np == NULL) {
		printk(KERN_ERR PFX "Unable to find fpga.\n");
		return -ENOENT;
	}

	pikawdt_private.fpga = of_iomap(np, 0);
	of_node_put(np);
	if (pikawdt_private.fpga == NULL) {
		printk(KERN_ERR PFX "Unable to map fpga.\n");
		return -ENOMEM;
	}

	ident.firmware_version = in_be32(pikawdt_private.fpga + 0x1c) & 0xffff;

	
	np = of_find_compatible_node(NULL, NULL, "pika,fpga-sd");
	if (np == NULL) {
		printk(KERN_ERR PFX "Unable to find fpga-sd.\n");
		ret = -ENOENT;
		goto out;
	}

	fpga = of_iomap(np, 0);
	of_node_put(np);
	if (fpga == NULL) {
		printk(KERN_ERR PFX "Unable to map fpga-sd.\n");
		ret = -ENOMEM;
		goto out;
	}

	
	post1 = in_be32(fpga + 0x40);
	if (post1 & 0x80000000)
		pikawdt_private.bootstatus = WDIOF_CARDRESET;

	iounmap(fpga);

	setup_timer(&pikawdt_private.timer, pikawdt_ping, 0);

	ret = misc_register(&pikawdt_miscdev);
	if (ret) {
		printk(KERN_ERR PFX "Unable to register miscdev.\n");
		goto out;
	}

	printk(KERN_INFO PFX "initialized. heartbeat=%d sec (nowayout=%d)\n",
							heartbeat, nowayout);
	return 0;

out:
	iounmap(pikawdt_private.fpga);
	return ret;
}

static void __exit pikawdt_exit(void)
{
	misc_deregister(&pikawdt_miscdev);

	iounmap(pikawdt_private.fpga);
}

module_init(pikawdt_init);
module_exit(pikawdt_exit);

MODULE_AUTHOR("Sean MacLennan <smaclennan@pikatech.com>");
MODULE_DESCRIPTION("PIKA FPGA based Watchdog Timer");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);

