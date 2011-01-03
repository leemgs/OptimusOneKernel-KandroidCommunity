

#include <linux/fs.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/smp_lock.h>
#include <asm/uaccess.h>
#include <asm/io.h>


#define NAME			"cs5535_gpio"

MODULE_AUTHOR("Ben Gardner <bgardner@wabtec.com>");
MODULE_DESCRIPTION("AMD CS5535/CS5536 GPIO Pin Driver");
MODULE_LICENSE("GPL");

static int major;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major device number");

static ulong mask;
module_param(mask, ulong, 0);
MODULE_PARM_DESC(mask, "GPIO channel mask");

#define MSR_LBAR_GPIO		0x5140000C

static u32 gpio_base;

static struct pci_device_id divil_pci[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_NS,  PCI_DEVICE_ID_NS_CS5535_ISA) },
	{ PCI_DEVICE(PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_CS5536_ISA) },
	{ } 
};
MODULE_DEVICE_TABLE(pci, divil_pci);

static struct cdev cs5535_gpio_cdev;


#define CS5535_GPIO_COUNT	32


#define CS5535_GPIO_SIZE	256

struct gpio_regmap {
	u32	rd_offset;
	u32	wr_offset;
	char	on;
	char	off;
};
static struct gpio_regmap rm[] =
{
	{ 0x30, 0x00, '1', '0' },	
	{ 0x20, 0x20, 'I', 'i' },	
	{ 0x04, 0x04, 'O', 'o' },	
	{ 0x08, 0x08, 't', 'T' },	
	{ 0x18, 0x18, 'P', 'p' },	
	{ 0x1c, 0x1c, 'D', 'd' },	
};



static inline u32 cs5535_lowhigh_base(int reg)
{
	return (reg & 0x10) << 3;
}

static ssize_t cs5535_gpio_write(struct file *file, const char __user *data,
				 size_t len, loff_t *ppos)
{
	u32	m = iminor(file->f_path.dentry->d_inode);
	int	i, j;
	u32	base = gpio_base + cs5535_lowhigh_base(m);
	u32	m0, m1;
	char	c;

	
	m1 = 1 << (m & 0x0F);
	m0 = m1 << 16;

	for (i = 0; i < len; ++i) {
		if (get_user(c, data+i))
			return -EFAULT;

		for (j = 0; j < ARRAY_SIZE(rm); j++) {
			if (c == rm[j].on) {
				outl(m1, base + rm[j].wr_offset);
				
				if (c == 'O') {
					outl(m0, base + 0x10);
					outl(m0, base + 0x14);
				}
				break;
			} else if (c == rm[j].off) {
				outl(m0, base + rm[j].wr_offset);
				break;
			}
		}
	}
	*ppos = 0;
	return len;
}

static ssize_t cs5535_gpio_read(struct file *file, char __user *buf,
				size_t len, loff_t *ppos)
{
	u32	m = iminor(file->f_path.dentry->d_inode);
	u32	base = gpio_base + cs5535_lowhigh_base(m);
	int	rd_bit = 1 << (m & 0x0f);
	int	i;
	char	ch;
	ssize_t	count = 0;

	if (*ppos >= ARRAY_SIZE(rm))
		return 0;

	for (i = *ppos; (i < (*ppos + len)) && (i < ARRAY_SIZE(rm)); i++) {
		ch = (inl(base + rm[i].rd_offset) & rd_bit) ?
		     rm[i].on : rm[i].off;

		if (put_user(ch, buf+count))
			return -EFAULT;

		count++;
	}

	
	if ((i == ARRAY_SIZE(rm)) && (count < len)) {
		put_user('\n', buf + count);
		count++;
	}

	*ppos += count;
	return count;
}

static int cs5535_gpio_open(struct inode *inode, struct file *file)
{
	u32 m = iminor(inode);

	cycle_kernel_lock();
	
	if ((mask & (1 << m)) == 0)
		return -EINVAL;

	return nonseekable_open(inode, file);
}

static const struct file_operations cs5535_gpio_fops = {
	.owner	= THIS_MODULE,
	.write	= cs5535_gpio_write,
	.read	= cs5535_gpio_read,
	.open	= cs5535_gpio_open
};

static int __init cs5535_gpio_init(void)
{
	dev_t	dev_id;
	u32	low, hi;
	int	retval;

	if (pci_dev_present(divil_pci) == 0) {
		printk(KERN_WARNING NAME ": DIVIL not found\n");
		return -ENODEV;
	}

	
	rdmsr(MSR_LBAR_GPIO, low, hi);

	
	if (hi != 0x0000f001) {
		printk(KERN_WARNING NAME ": GPIO not enabled\n");
		return -ENODEV;
	}

	
	gpio_base = low & 0x0000ff00;

	
	if (mask != 0)
		mask &= 0x1f7fffff;
	else
		mask = 0x0b003c66;

	if (!request_region(gpio_base, CS5535_GPIO_SIZE, NAME)) {
		printk(KERN_ERR NAME ": can't allocate I/O for GPIO\n");
		return -ENODEV;
	}

	if (major) {
		dev_id = MKDEV(major, 0);
		retval = register_chrdev_region(dev_id, CS5535_GPIO_COUNT,
						NAME);
	} else {
		retval = alloc_chrdev_region(&dev_id, 0, CS5535_GPIO_COUNT,
					     NAME);
		major = MAJOR(dev_id);
	}

	if (retval) {
		release_region(gpio_base, CS5535_GPIO_SIZE);
		return -1;
	}

	printk(KERN_DEBUG NAME ": base=%#x mask=%#lx major=%d\n",
	       gpio_base, mask, major);

	cdev_init(&cs5535_gpio_cdev, &cs5535_gpio_fops);
	cdev_add(&cs5535_gpio_cdev, dev_id, CS5535_GPIO_COUNT);

	return 0;
}

static void __exit cs5535_gpio_cleanup(void)
{
	dev_t dev_id = MKDEV(major, 0);

	cdev_del(&cs5535_gpio_cdev);
	unregister_chrdev_region(dev_id, CS5535_GPIO_COUNT);
	release_region(gpio_base, CS5535_GPIO_SIZE);
}

module_init(cs5535_gpio_init);
module_exit(cs5535_gpio_cleanup);
