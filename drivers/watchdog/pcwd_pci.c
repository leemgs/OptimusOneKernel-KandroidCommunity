





#include <linux/module.h>	
#include <linux/moduleparam.h>	
#include <linux/types.h>	
#include <linux/errno.h>	
#include <linux/kernel.h>	
#include <linux/delay.h>	
#include <linux/miscdevice.h>	
#include <linux/watchdog.h>	
#include <linux/notifier.h>	
#include <linux/reboot.h>	
#include <linux/init.h>		
#include <linux/fs.h>		
#include <linux/pci.h>		
#include <linux/ioport.h>	
#include <linux/spinlock.h>	
#include <linux/uaccess.h>	
#include <linux/io.h>		


#define WATCHDOG_VERSION "1.03"
#define WATCHDOG_DRIVER_NAME "PCI-PC Watchdog"
#define WATCHDOG_NAME "pcwd_pci"
#define PFX WATCHDOG_NAME ": "
#define DRIVER_VERSION WATCHDOG_DRIVER_NAME " driver, v" WATCHDOG_VERSION "\n"


#ifndef PCI_VENDOR_ID_QUICKLOGIC
#define PCI_VENDOR_ID_QUICKLOGIC    0x11e3
#endif

#ifndef PCI_DEVICE_ID_WATCHDOG_PCIPCWD
#define PCI_DEVICE_ID_WATCHDOG_PCIPCWD 0x5030
#endif



#define WD_PCI_WTRP		0x01	
#define WD_PCI_HRBT		0x02	
#define WD_PCI_TTRP		0x04	
#define WD_PCI_RL2A		0x08	
#define WD_PCI_RL1A		0x10	
#define WD_PCI_R2DS		0x40	
#define WD_PCI_RLY2		0x80	

#define WD_PCI_WDIS		0x10	
#define WD_PCI_ENTP		0x20	
#define WD_PCI_WRSP		0x40	
#define WD_PCI_PCMD		0x80	


#define PCI_COMMAND_TIMEOUT	150


#define CMD_GET_STATUS				0x04
#define CMD_GET_FIRMWARE_VERSION		0x08
#define CMD_READ_WATCHDOG_TIMEOUT		0x18
#define CMD_WRITE_WATCHDOG_TIMEOUT		0x19
#define CMD_GET_CLEAR_RESET_COUNT		0x84


static const int heartbeat_tbl[] = {
	5,	
	10,	
	30,	
	60,	
	300,	
	600,	
	1800,	
	3600,	
};


static int cards_found;


static int temp_panic;
static unsigned long is_active;
static char expect_release;

static struct {
	
	int supports_temp;
	
	int boot_status;
	
	unsigned long io_addr;
	
	spinlock_t io_lock;
	
	struct pci_dev *pdev;
} pcipcwd_private;


#define QUIET	0	
#define VERBOSE	1	
#define DEBUG	2	
static int debug = QUIET;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "Debug level: 0=Quiet, 1=Verbose, 2=Debug (default=0)");

#define WATCHDOG_HEARTBEAT 0	
static int heartbeat = WATCHDOG_HEARTBEAT;
module_param(heartbeat, int, 0);
MODULE_PARM_DESC(heartbeat, "Watchdog heartbeat in seconds. "
	"(0<heartbeat<65536 or 0=delay-time from dip-switches, default="
				__MODULE_STRING(WATCHDOG_HEARTBEAT) ")");

static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started (default="
					__MODULE_STRING(WATCHDOG_NOWAYOUT) ")");



static int send_command(int cmd, int *msb, int *lsb)
{
	int got_response, count;

	if (debug >= DEBUG)
		printk(KERN_DEBUG PFX "sending following data "
		"cmd=0x%02x msb=0x%02x lsb=0x%02x\n", cmd, *msb, *lsb);

	spin_lock(&pcipcwd_private.io_lock);
	
	outb_p(*lsb, pcipcwd_private.io_addr + 4);
	outb_p(*msb, pcipcwd_private.io_addr + 5);
	outb_p(cmd, pcipcwd_private.io_addr + 6);

	
	got_response = inb_p(pcipcwd_private.io_addr + 2) & WD_PCI_WRSP;
	for (count = 0; (count < PCI_COMMAND_TIMEOUT) && (!got_response);
								count++) {
		mdelay(1);
		got_response = inb_p(pcipcwd_private.io_addr + 2) & WD_PCI_WRSP;
	}

	if (debug >= DEBUG) {
		if (got_response) {
			printk(KERN_DEBUG PFX
				"time to process command was: %d ms\n",
				count);
		} else {
			printk(KERN_DEBUG PFX
				"card did not respond on command!\n");
		}
	}

	if (got_response) {
		
		*lsb = inb_p(pcipcwd_private.io_addr + 4);
		*msb = inb_p(pcipcwd_private.io_addr + 5);

		
		inb_p(pcipcwd_private.io_addr + 6);

		if (debug >= DEBUG)
			printk(KERN_DEBUG PFX "received following data for "
				"cmd=0x%02x: msb=0x%02x lsb=0x%02x\n",
				cmd, *msb, *lsb);
	}

	spin_unlock(&pcipcwd_private.io_lock);

	return got_response;
}

static inline void pcipcwd_check_temperature_support(void)
{
	if (inb_p(pcipcwd_private.io_addr) != 0xF0)
		pcipcwd_private.supports_temp = 1;
}

static int pcipcwd_get_option_switches(void)
{
	int option_switches;

	option_switches = inb_p(pcipcwd_private.io_addr + 3);
	return option_switches;
}

static void pcipcwd_show_card_info(void)
{
	int got_fw_rev, fw_rev_major, fw_rev_minor;
	char fw_ver_str[20];		
	int option_switches;

	got_fw_rev = send_command(CMD_GET_FIRMWARE_VERSION, &fw_rev_major,
								&fw_rev_minor);
	if (got_fw_rev)
		sprintf(fw_ver_str, "%u.%02u", fw_rev_major, fw_rev_minor);
	else
		sprintf(fw_ver_str, "<card no answer>");

	
	option_switches = pcipcwd_get_option_switches();

	printk(KERN_INFO PFX "Found card at port "
		"0x%04x (Firmware: %s) %s temp option\n",
		(int) pcipcwd_private.io_addr, fw_ver_str,
		(pcipcwd_private.supports_temp ? "with" : "without"));

	printk(KERN_INFO PFX "Option switches (0x%02x): "
		"Temperature Reset Enable=%s, Power On Delay=%s\n",
		option_switches,
		((option_switches & 0x10) ? "ON" : "OFF"),
		((option_switches & 0x08) ? "ON" : "OFF"));

	if (pcipcwd_private.boot_status & WDIOF_CARDRESET)
		printk(KERN_INFO PFX
			"Previous reset was caused by the Watchdog card\n");

	if (pcipcwd_private.boot_status & WDIOF_OVERHEAT)
		printk(KERN_INFO PFX "Card sensed a CPU Overheat\n");

	if (pcipcwd_private.boot_status == 0)
		printk(KERN_INFO PFX
			"No previous trip detected - Cold boot or reset\n");
}

static int pcipcwd_start(void)
{
	int stat_reg;

	spin_lock(&pcipcwd_private.io_lock);
	outb_p(0x00, pcipcwd_private.io_addr + 3);
	udelay(1000);

	stat_reg = inb_p(pcipcwd_private.io_addr + 2);
	spin_unlock(&pcipcwd_private.io_lock);

	if (stat_reg & WD_PCI_WDIS) {
		printk(KERN_ERR PFX "Card timer not enabled\n");
		return -1;
	}

	if (debug >= VERBOSE)
		printk(KERN_DEBUG PFX "Watchdog started\n");

	return 0;
}

static int pcipcwd_stop(void)
{
	int stat_reg;

	spin_lock(&pcipcwd_private.io_lock);
	outb_p(0xA5, pcipcwd_private.io_addr + 3);
	udelay(1000);

	outb_p(0xA5, pcipcwd_private.io_addr + 3);
	udelay(1000);

	stat_reg = inb_p(pcipcwd_private.io_addr + 2);
	spin_unlock(&pcipcwd_private.io_lock);

	if (!(stat_reg & WD_PCI_WDIS)) {
		printk(KERN_ERR PFX
			"Card did not acknowledge disable attempt\n");
		return -1;
	}

	if (debug >= VERBOSE)
		printk(KERN_DEBUG PFX "Watchdog stopped\n");

	return 0;
}

static int pcipcwd_keepalive(void)
{
	
	spin_lock(&pcipcwd_private.io_lock);
	outb_p(0x42, pcipcwd_private.io_addr);	
	spin_unlock(&pcipcwd_private.io_lock);

	if (debug >= DEBUG)
		printk(KERN_DEBUG PFX "Watchdog keepalive signal send\n");

	return 0;
}

static int pcipcwd_set_heartbeat(int t)
{
	int t_msb = t / 256;
	int t_lsb = t % 256;

	if ((t < 0x0001) || (t > 0xFFFF))
		return -EINVAL;

	
	send_command(CMD_WRITE_WATCHDOG_TIMEOUT, &t_msb, &t_lsb);

	heartbeat = t;
	if (debug >= VERBOSE)
		printk(KERN_DEBUG PFX "New heartbeat: %d\n",
		       heartbeat);

	return 0;
}

static int pcipcwd_get_status(int *status)
{
	int control_status;

	*status = 0;
	control_status = inb_p(pcipcwd_private.io_addr + 1);
	if (control_status & WD_PCI_WTRP)
		*status |= WDIOF_CARDRESET;
	if (control_status & WD_PCI_TTRP) {
		*status |= WDIOF_OVERHEAT;
		if (temp_panic)
			panic(PFX "Temperature overheat trip!\n");
	}

	if (debug >= DEBUG)
		printk(KERN_DEBUG PFX "Control Status #1: 0x%02x\n",
		       control_status);

	return 0;
}

static int pcipcwd_clear_status(void)
{
	int control_status;
	int msb;
	int reset_counter;

	if (debug >= VERBOSE)
		printk(KERN_INFO PFX "clearing watchdog trip status & LED\n");

	control_status = inb_p(pcipcwd_private.io_addr + 1);

	if (debug >= DEBUG) {
		printk(KERN_DEBUG PFX "status was: 0x%02x\n", control_status);
		printk(KERN_DEBUG PFX "sending: 0x%02x\n",
		       (control_status & WD_PCI_R2DS) | WD_PCI_WTRP);
	}

	
	outb_p((control_status & WD_PCI_R2DS) | WD_PCI_WTRP,
						pcipcwd_private.io_addr + 1);

	
	msb = 0;
	reset_counter = 0xff;
	send_command(CMD_GET_CLEAR_RESET_COUNT, &msb, &reset_counter);

	if (debug >= DEBUG) {
		printk(KERN_DEBUG PFX "reset count was: 0x%02x\n",
		       reset_counter);
	}

	return 0;
}

static int pcipcwd_get_temperature(int *temperature)
{
	*temperature = 0;
	if (!pcipcwd_private.supports_temp)
		return -ENODEV;

	spin_lock(&pcipcwd_private.io_lock);
	*temperature = inb_p(pcipcwd_private.io_addr);
	spin_unlock(&pcipcwd_private.io_lock);

	
	*temperature = (*temperature * 9 / 5) + 32;

	if (debug >= DEBUG) {
		printk(KERN_DEBUG PFX "temperature is: %d F\n",
		       *temperature);
	}

	return 0;
}

static int pcipcwd_get_timeleft(int *time_left)
{
	int msb;
	int lsb;

	
	
	send_command(CMD_READ_WATCHDOG_TIMEOUT, &msb, &lsb);

	*time_left = (msb << 8) + lsb;

	if (debug >= VERBOSE)
		printk(KERN_DEBUG PFX "Time left before next reboot: %d\n",
		       *time_left);

	return 0;
}



static ssize_t pcipcwd_write(struct file *file, const char __user *data,
			     size_t len, loff_t *ppos)
{
	
	if (len) {
		if (!nowayout) {
			size_t i;

			
			expect_release = 0;

			
			for (i = 0; i != len; i++) {
				char c;
				if (get_user(c, data + i))
					return -EFAULT;
				if (c == 'V')
					expect_release = 42;
			}
		}

		
		pcipcwd_keepalive();
	}
	return len;
}

static long pcipcwd_ioctl(struct file *file, unsigned int cmd,
						unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	static struct watchdog_info ident = {
		.options =		WDIOF_OVERHEAT |
					WDIOF_CARDRESET |
					WDIOF_KEEPALIVEPING |
					WDIOF_SETTIMEOUT |
					WDIOF_MAGICCLOSE,
		.firmware_version =	1,
		.identity =		WATCHDOG_DRIVER_NAME,
	};

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		return copy_to_user(argp, &ident, sizeof(ident)) ? -EFAULT : 0;

	case WDIOC_GETSTATUS:
	{
		int status;
		pcipcwd_get_status(&status);
		return put_user(status, p);
	}

	case WDIOC_GETBOOTSTATUS:
		return put_user(pcipcwd_private.boot_status, p);

	case WDIOC_GETTEMP:
	{
		int temperature;

		if (pcipcwd_get_temperature(&temperature))
			return -EFAULT;

		return put_user(temperature, p);
	}

	case WDIOC_SETOPTIONS:
	{
		int new_options, retval = -EINVAL;

		if (get_user(new_options, p))
			return -EFAULT;

		if (new_options & WDIOS_DISABLECARD) {
			if (pcipcwd_stop())
				return -EIO;
			retval = 0;
		}

		if (new_options & WDIOS_ENABLECARD) {
			if (pcipcwd_start())
				return -EIO;
			retval = 0;
		}

		if (new_options & WDIOS_TEMPPANIC) {
			temp_panic = 1;
			retval = 0;
		}

		return retval;
	}

	case WDIOC_KEEPALIVE:
		pcipcwd_keepalive();
		return 0;

	case WDIOC_SETTIMEOUT:
	{
		int new_heartbeat;

		if (get_user(new_heartbeat, p))
			return -EFAULT;

		if (pcipcwd_set_heartbeat(new_heartbeat))
			return -EINVAL;

		pcipcwd_keepalive();
		
	}

	case WDIOC_GETTIMEOUT:
		return put_user(heartbeat, p);

	case WDIOC_GETTIMELEFT:
	{
		int time_left;

		if (pcipcwd_get_timeleft(&time_left))
			return -EFAULT;

		return put_user(time_left, p);
	}

	default:
		return -ENOTTY;
	}
}

static int pcipcwd_open(struct inode *inode, struct file *file)
{
	
	if (test_and_set_bit(0, &is_active)) {
		if (debug >= VERBOSE)
			printk(KERN_ERR PFX
				"Attempt to open already opened device.\n");
		return -EBUSY;
	}

	
	pcipcwd_start();
	pcipcwd_keepalive();
	return nonseekable_open(inode, file);
}

static int pcipcwd_release(struct inode *inode, struct file *file)
{
	
	if (expect_release == 42) {
		pcipcwd_stop();
	} else {
		printk(KERN_CRIT PFX
			"Unexpected close, not stopping watchdog!\n");
		pcipcwd_keepalive();
	}
	expect_release = 0;
	clear_bit(0, &is_active);
	return 0;
}



static ssize_t pcipcwd_temp_read(struct file *file, char __user *data,
				size_t len, loff_t *ppos)
{
	int temperature;

	if (pcipcwd_get_temperature(&temperature))
		return -EFAULT;

	if (copy_to_user(data, &temperature, 1))
		return -EFAULT;

	return 1;
}

static int pcipcwd_temp_open(struct inode *inode, struct file *file)
{
	if (!pcipcwd_private.supports_temp)
		return -ENODEV;

	return nonseekable_open(inode, file);
}

static int pcipcwd_temp_release(struct inode *inode, struct file *file)
{
	return 0;
}



static int pcipcwd_notify_sys(struct notifier_block *this, unsigned long code,
								void *unused)
{
	if (code == SYS_DOWN || code == SYS_HALT)
		pcipcwd_stop();	

	return NOTIFY_DONE;
}



static const struct file_operations pcipcwd_fops = {
	.owner =	THIS_MODULE,
	.llseek =	no_llseek,
	.write =	pcipcwd_write,
	.unlocked_ioctl = pcipcwd_ioctl,
	.open =		pcipcwd_open,
	.release =	pcipcwd_release,
};

static struct miscdevice pcipcwd_miscdev = {
	.minor =	WATCHDOG_MINOR,
	.name =		"watchdog",
	.fops =		&pcipcwd_fops,
};

static const struct file_operations pcipcwd_temp_fops = {
	.owner =	THIS_MODULE,
	.llseek =	no_llseek,
	.read =		pcipcwd_temp_read,
	.open =		pcipcwd_temp_open,
	.release =	pcipcwd_temp_release,
};

static struct miscdevice pcipcwd_temp_miscdev = {
	.minor =	TEMP_MINOR,
	.name =		"temperature",
	.fops =		&pcipcwd_temp_fops,
};

static struct notifier_block pcipcwd_notifier = {
	.notifier_call =	pcipcwd_notify_sys,
};



static int __devinit pcipcwd_card_init(struct pci_dev *pdev,
		const struct pci_device_id *ent)
{
	int ret = -EIO;

	cards_found++;
	if (cards_found == 1)
		printk(KERN_INFO PFX DRIVER_VERSION);

	if (cards_found > 1) {
		printk(KERN_ERR PFX "This driver only supports 1 device\n");
		return -ENODEV;
	}

	if (pci_enable_device(pdev)) {
		printk(KERN_ERR PFX "Not possible to enable PCI Device\n");
		return -ENODEV;
	}

	if (pci_resource_start(pdev, 0) == 0x0000) {
		printk(KERN_ERR PFX "No I/O-Address for card detected\n");
		ret = -ENODEV;
		goto err_out_disable_device;
	}

	pcipcwd_private.pdev = pdev;
	pcipcwd_private.io_addr = pci_resource_start(pdev, 0);

	if (pci_request_regions(pdev, WATCHDOG_NAME)) {
		printk(KERN_ERR PFX "I/O address 0x%04x already in use\n",
			(int) pcipcwd_private.io_addr);
		ret = -EIO;
		goto err_out_disable_device;
	}

	
	pcipcwd_get_status(&pcipcwd_private.boot_status);

	
	pcipcwd_clear_status();

	
	pcipcwd_stop();

	
	pcipcwd_check_temperature_support();

	
	pcipcwd_show_card_info();

	
	if (heartbeat == 0)
		heartbeat =
			heartbeat_tbl[(pcipcwd_get_option_switches() & 0x07)];

	
	if (pcipcwd_set_heartbeat(heartbeat)) {
		pcipcwd_set_heartbeat(WATCHDOG_HEARTBEAT);
		printk(KERN_INFO PFX
			"heartbeat value must be 0<heartbeat<65536, using %d\n",
			WATCHDOG_HEARTBEAT);
	}

	ret = register_reboot_notifier(&pcipcwd_notifier);
	if (ret != 0) {
		printk(KERN_ERR PFX
			"cannot register reboot notifier (err=%d)\n", ret);
		goto err_out_release_region;
	}

	if (pcipcwd_private.supports_temp) {
		ret = misc_register(&pcipcwd_temp_miscdev);
		if (ret != 0) {
			printk(KERN_ERR PFX "cannot register miscdev on "
				"minor=%d (err=%d)\n", TEMP_MINOR, ret);
			goto err_out_unregister_reboot;
		}
	}

	ret = misc_register(&pcipcwd_miscdev);
	if (ret != 0) {
		printk(KERN_ERR PFX
			"cannot register miscdev on minor=%d (err=%d)\n",
			WATCHDOG_MINOR, ret);
		goto err_out_misc_deregister;
	}

	printk(KERN_INFO PFX "initialized. heartbeat=%d sec (nowayout=%d)\n",
		heartbeat, nowayout);

	return 0;

err_out_misc_deregister:
	if (pcipcwd_private.supports_temp)
		misc_deregister(&pcipcwd_temp_miscdev);
err_out_unregister_reboot:
	unregister_reboot_notifier(&pcipcwd_notifier);
err_out_release_region:
	pci_release_regions(pdev);
err_out_disable_device:
	pci_disable_device(pdev);
	return ret;
}

static void __devexit pcipcwd_card_exit(struct pci_dev *pdev)
{
	
	if (!nowayout)
		pcipcwd_stop();

	
	misc_deregister(&pcipcwd_miscdev);
	if (pcipcwd_private.supports_temp)
		misc_deregister(&pcipcwd_temp_miscdev);
	unregister_reboot_notifier(&pcipcwd_notifier);
	pci_release_regions(pdev);
	pci_disable_device(pdev);
	cards_found--;
}

static struct pci_device_id pcipcwd_pci_tbl[] = {
	{ PCI_VENDOR_ID_QUICKLOGIC, PCI_DEVICE_ID_WATCHDOG_PCIPCWD,
		PCI_ANY_ID, PCI_ANY_ID, },
	{ 0 },			
};
MODULE_DEVICE_TABLE(pci, pcipcwd_pci_tbl);

static struct pci_driver pcipcwd_driver = {
	.name		= WATCHDOG_NAME,
	.id_table	= pcipcwd_pci_tbl,
	.probe		= pcipcwd_card_init,
	.remove		= __devexit_p(pcipcwd_card_exit),
};

static int __init pcipcwd_init_module(void)
{
	spin_lock_init(&pcipcwd_private.io_lock);

	return pci_register_driver(&pcipcwd_driver);
}

static void __exit pcipcwd_cleanup_module(void)
{
	pci_unregister_driver(&pcipcwd_driver);

	printk(KERN_INFO PFX "Watchdog Module Unloaded.\n");
}

module_init(pcipcwd_init_module);
module_exit(pcipcwd_cleanup_module);

MODULE_AUTHOR("Wim Van Sebroeck <wim@iguana.be>");
MODULE_DESCRIPTION("Berkshire PCI-PC Watchdog driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
MODULE_ALIAS_MISCDEV(TEMP_MINOR);
