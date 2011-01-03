



#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/uaccess.h>
#include <linux/io.h>


#define ESB_VERSION "0.04"
#define ESB_MODULE_NAME "i6300ESB timer"
#define ESB_DRIVER_NAME ESB_MODULE_NAME ", v" ESB_VERSION
#define PFX ESB_MODULE_NAME ": "


#define ESB_CONFIG_REG  0x60            
#define ESB_LOCK_REG    0x68            


#define ESB_TIMER1_REG (BASEADDR + 0x00)
#define ESB_TIMER2_REG (BASEADDR + 0x04)
#define ESB_GINTSR_REG (BASEADDR + 0x08)
#define ESB_RELOAD_REG (BASEADDR + 0x0c)


#define ESB_WDT_FUNC    (0x01 << 2)   
#define ESB_WDT_ENABLE  (0x01 << 1)   
#define ESB_WDT_LOCK    (0x01 << 0)   


#define ESB_WDT_REBOOT  (0x01 << 5)   
#define ESB_WDT_FREQ    (0x01 << 2)   
#define ESB_WDT_INTTYPE (0x11 << 0)   


#define ESB_WDT_TIMEOUT (0x01 << 9)    
#define ESB_WDT_RELOAD  (0x01 << 8)    


#define ESB_UNLOCK1     0x80            
#define ESB_UNLOCK2     0x86            


static void __iomem *BASEADDR;
static DEFINE_SPINLOCK(esb_lock); 
static unsigned long timer_alive;
static struct pci_dev *esb_pci;
static unsigned short triggered; 
static char esb_expect_close;
static struct platform_device *esb_platform_device;



#define WATCHDOG_HEARTBEAT 30
static int heartbeat = WATCHDOG_HEARTBEAT;  
module_param(heartbeat, int, 0);
MODULE_PARM_DESC(heartbeat,
		"Watchdog heartbeat in seconds. (1<heartbeat<2046, default="
				__MODULE_STRING(WATCHDOG_HEARTBEAT) ")");

static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout,
		"Watchdog cannot be stopped once started (default="
				__MODULE_STRING(WATCHDOG_NOWAYOUT) ")");




static inline void esb_unlock_registers(void)
{
	writeb(ESB_UNLOCK1, ESB_RELOAD_REG);
	writeb(ESB_UNLOCK2, ESB_RELOAD_REG);
}

static int esb_timer_start(void)
{
	u8 val;

	spin_lock(&esb_lock);
	esb_unlock_registers();
	writew(ESB_WDT_RELOAD, ESB_RELOAD_REG);
	
	val = ESB_WDT_ENABLE | (nowayout ? ESB_WDT_LOCK : 0x00);
	pci_write_config_byte(esb_pci, ESB_LOCK_REG, val);
	spin_unlock(&esb_lock);
	return 0;
}

static int esb_timer_stop(void)
{
	u8 val;

	spin_lock(&esb_lock);
	
	esb_unlock_registers();
	writew(ESB_WDT_RELOAD, ESB_RELOAD_REG);
	
	pci_write_config_byte(esb_pci, ESB_LOCK_REG, 0x0);
	pci_read_config_byte(esb_pci, ESB_LOCK_REG, &val);
	spin_unlock(&esb_lock);

	
	return val & ESB_WDT_ENABLE;
}

static void esb_timer_keepalive(void)
{
	spin_lock(&esb_lock);
	esb_unlock_registers();
	writew(ESB_WDT_RELOAD, ESB_RELOAD_REG);
	
	spin_unlock(&esb_lock);
}

static int esb_timer_set_heartbeat(int time)
{
	u32 val;

	if (time < 0x1 || time > (2 * 0x03ff))
		return -EINVAL;

	spin_lock(&esb_lock);

	
	val = time << 9;

	
	esb_unlock_registers();
	writel(val, ESB_TIMER1_REG);

	
	esb_unlock_registers();
	writel(val, ESB_TIMER2_REG);

	
	esb_unlock_registers();
	writew(ESB_WDT_RELOAD, ESB_RELOAD_REG);

	

	
	heartbeat = time;
	spin_unlock(&esb_lock);
	return 0;
}



static int esb_open(struct inode *inode, struct file *file)
{
	
	if (test_and_set_bit(0, &timer_alive))
		return -EBUSY;

	
	esb_timer_start();

	return nonseekable_open(inode, file);
}

static int esb_release(struct inode *inode, struct file *file)
{
	
	if (esb_expect_close == 42)
		esb_timer_stop();
	else {
		printk(KERN_CRIT PFX
				"Unexpected close, not stopping watchdog!\n");
		esb_timer_keepalive();
	}
	clear_bit(0, &timer_alive);
	esb_expect_close = 0;
	return 0;
}

static ssize_t esb_write(struct file *file, const char __user *data,
			  size_t len, loff_t *ppos)
{
	
	if (len) {
		if (!nowayout) {
			size_t i;

			
			esb_expect_close = 0;

			
			for (i = 0; i != len; i++) {
				char c;
				if (get_user(c, data + i))
					return -EFAULT;
				if (c == 'V')
					esb_expect_close = 42;
			}
		}

		
		esb_timer_keepalive();
	}
	return len;
}

static long esb_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int new_options, retval = -EINVAL;
	int new_heartbeat;
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	static struct watchdog_info ident = {
		.options =		WDIOF_SETTIMEOUT |
					WDIOF_KEEPALIVEPING |
					WDIOF_MAGICCLOSE,
		.firmware_version =	0,
		.identity =		ESB_MODULE_NAME,
	};

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		return copy_to_user(argp, &ident,
					sizeof(ident)) ? -EFAULT : 0;

	case WDIOC_GETSTATUS:
		return put_user(0, p);

	case WDIOC_GETBOOTSTATUS:
		return put_user(triggered, p);

	case WDIOC_SETOPTIONS:
	{
		if (get_user(new_options, p))
			return -EFAULT;

		if (new_options & WDIOS_DISABLECARD) {
			esb_timer_stop();
			retval = 0;
		}

		if (new_options & WDIOS_ENABLECARD) {
			esb_timer_start();
			retval = 0;
		}
		return retval;
	}
	case WDIOC_KEEPALIVE:
		esb_timer_keepalive();
		return 0;

	case WDIOC_SETTIMEOUT:
	{
		if (get_user(new_heartbeat, p))
			return -EFAULT;
		if (esb_timer_set_heartbeat(new_heartbeat))
			return -EINVAL;
		esb_timer_keepalive();
		
	}
	case WDIOC_GETTIMEOUT:
		return put_user(heartbeat, p);
	default:
		return -ENOTTY;
	}
}



static const struct file_operations esb_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.write = esb_write,
	.unlocked_ioctl = esb_ioctl,
	.open = esb_open,
	.release = esb_release,
};

static struct miscdevice esb_miscdev = {
	.minor = WATCHDOG_MINOR,
	.name = "watchdog",
	.fops = &esb_fops,
};


static struct pci_device_id esb_pci_tbl[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ESB_9), },
	{ 0, },                 
};
MODULE_DEVICE_TABLE(pci, esb_pci_tbl);



static unsigned char __devinit esb_getdevice(void)
{
	

	esb_pci = pci_get_device(PCI_VENDOR_ID_INTEL,
					PCI_DEVICE_ID_INTEL_ESB_9, NULL);

	if (!esb_pci)
		return 0;

	if (pci_enable_device(esb_pci)) {
		printk(KERN_ERR PFX "failed to enable device\n");
		goto err_devput;
	}

	if (pci_request_region(esb_pci, 0, ESB_MODULE_NAME)) {
		printk(KERN_ERR PFX "failed to request region\n");
		goto err_disable;
	}

	BASEADDR = pci_ioremap_bar(esb_pci, 0);
	if (BASEADDR == NULL) {
		
		printk(KERN_ERR PFX "failed to get BASEADDR\n");
		goto err_release;
	}

	
	return 1;

err_release:
	pci_release_region(esb_pci, 0);
err_disable:
	pci_disable_device(esb_pci);
err_devput:
	pci_dev_put(esb_pci);
	return 0;
}

static void __devinit esb_initdevice(void)
{
	u8 val1;
	u16 val2;

	
	pci_write_config_word(esb_pci, ESB_CONFIG_REG, 0x0003);

	
	pci_read_config_byte(esb_pci, ESB_LOCK_REG, &val1);
	if (val1 & ESB_WDT_LOCK)
		printk(KERN_WARNING PFX "nowayout already set\n");

	
	pci_write_config_byte(esb_pci, ESB_LOCK_REG, 0x00);

	
	esb_unlock_registers();
	val2 = readw(ESB_RELOAD_REG);
	if (val2 & ESB_WDT_TIMEOUT)
		triggered = WDIOF_CARDRESET;

	
	esb_unlock_registers();
	writew((ESB_WDT_TIMEOUT | ESB_WDT_RELOAD), ESB_RELOAD_REG);

	
	esb_timer_set_heartbeat(heartbeat);
}

static int __devinit esb_probe(struct platform_device *dev)
{
	int ret;

	
	if (!esb_getdevice() || esb_pci == NULL)
		return -ENODEV;

	
	if (heartbeat < 0x1 || heartbeat > 2 * 0x03ff) {
		heartbeat = WATCHDOG_HEARTBEAT;
		printk(KERN_INFO PFX
			"heartbeat value must be 1<heartbeat<2046, using %d\n",
								heartbeat);
	}

	
	esb_initdevice();

	
	ret = misc_register(&esb_miscdev);
	if (ret != 0) {
		printk(KERN_ERR PFX
			"cannot register miscdev on minor=%d (err=%d)\n",
							WATCHDOG_MINOR, ret);
		goto err_unmap;
	}
	printk(KERN_INFO PFX
		"initialized (0x%p). heartbeat=%d sec (nowayout=%d)\n",
						BASEADDR, heartbeat, nowayout);
	return 0;

err_unmap:
	iounmap(BASEADDR);
	pci_release_region(esb_pci, 0);
	pci_disable_device(esb_pci);
	pci_dev_put(esb_pci);
	return ret;
}

static int __devexit esb_remove(struct platform_device *dev)
{
	
	if (!nowayout)
		esb_timer_stop();

	
	misc_deregister(&esb_miscdev);
	iounmap(BASEADDR);
	pci_release_region(esb_pci, 0);
	pci_disable_device(esb_pci);
	pci_dev_put(esb_pci);
	return 0;
}

static void esb_shutdown(struct platform_device *dev)
{
	esb_timer_stop();
}

static struct platform_driver esb_platform_driver = {
	.probe          = esb_probe,
	.remove         = __devexit_p(esb_remove),
	.shutdown       = esb_shutdown,
	.driver         = {
		.owner  = THIS_MODULE,
		.name   = ESB_MODULE_NAME,
	},
};

static int __init watchdog_init(void)
{
	int err;

	printk(KERN_INFO PFX "Intel 6300ESB WatchDog Timer Driver v%s\n",
		ESB_VERSION);

	err = platform_driver_register(&esb_platform_driver);
	if (err)
		return err;

	esb_platform_device = platform_device_register_simple(ESB_MODULE_NAME,
								-1, NULL, 0);
	if (IS_ERR(esb_platform_device)) {
		err = PTR_ERR(esb_platform_device);
		goto unreg_platform_driver;
	}

	return 0;

unreg_platform_driver:
	platform_driver_unregister(&esb_platform_driver);
	return err;
}

static void __exit watchdog_cleanup(void)
{
	platform_device_unregister(esb_platform_device);
	platform_driver_unregister(&esb_platform_driver);
	printk(KERN_INFO PFX "Watchdog Module Unloaded.\n");
}

module_init(watchdog_init);
module_exit(watchdog_cleanup);

MODULE_AUTHOR("Ross Biro and David Härdeman");
MODULE_DESCRIPTION("Watchdog driver for Intel 6300ESB chipsets");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
