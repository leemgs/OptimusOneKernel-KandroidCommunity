

#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <linux/pci.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/syscalls.h>
#include <linux/types.h>
#include <linux/version.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include "../vme.h"
#include "vme_user.h"

static char driver_name[] = "vme_user";

static int bus[USER_BUS_MAX];
static int bus_num;


#define VME_MAJOR	221	
#define VME_DEVS	9	

#define MASTER_MINOR	0
#define MASTER_MAX	3
#define SLAVE_MINOR	4
#define SLAVE_MAX	7
#define CONTROL_MINOR	8

#define PCI_BUF_SIZE  0x20000	


typedef struct {
	void __iomem *kern_buf;	
	dma_addr_t pci_buf;	
	unsigned long long size_buf;	
	struct semaphore sem;	
	struct device *device;	
	struct vme_resource *resource;	
	int users;		
} image_desc_t;
static image_desc_t image[VME_DEVS];

typedef struct {
	unsigned long reads;
	unsigned long writes;
	unsigned long ioctls;
	unsigned long irqs;
	unsigned long berrs;
	unsigned long dmaErrors;
	unsigned long timeouts;
	unsigned long external;
} driver_stats_t;
static driver_stats_t statistics;

struct cdev *vme_user_cdev;		
struct class *vme_user_sysfs_class;	
struct device *vme_user_bridge;		


static const int type[VME_DEVS] = {	MASTER_MINOR,	MASTER_MINOR,
					MASTER_MINOR,	MASTER_MINOR,
					SLAVE_MINOR,	SLAVE_MINOR,
					SLAVE_MINOR,	SLAVE_MINOR,
					CONTROL_MINOR
				};


static int vme_user_open(struct inode *, struct file *);
static int vme_user_release(struct inode *, struct file *);
static ssize_t vme_user_read(struct file *, char *, size_t, loff_t *);
static ssize_t vme_user_write(struct file *, const char *, size_t, loff_t *);
static loff_t vme_user_llseek(struct file *, loff_t, int);
static int vme_user_ioctl(struct inode *, struct file *, unsigned int,
	unsigned long);

static int __init vme_user_probe(struct device *, int, int);
static int __exit vme_user_remove(struct device *, int, int);

static struct file_operations vme_user_fops = {
        .open = vme_user_open,
        .release = vme_user_release,
        .read = vme_user_read,
        .write = vme_user_write,
        .llseek = vme_user_llseek,
        .ioctl = vme_user_ioctl,
};



static void reset_counters(void)
{
        statistics.reads = 0;
        statistics.writes = 0;
        statistics.ioctls = 0;
        statistics.irqs = 0;
        statistics.berrs = 0;
        statistics.dmaErrors = 0;
        statistics.timeouts = 0;
}

static int vme_user_open(struct inode *inode, struct file *file)
{
	int err;
	unsigned int minor = MINOR(inode->i_rdev);

	down(&image[minor].sem);
	
	if (image[minor].resource == NULL) {
		printk(KERN_ERR "No resources allocated for device\n");
		err = -EINVAL;
		goto err_res;
	}

	
	image[minor].users++;

	up(&image[minor].sem);

	return 0;

err_res:
	up(&image[minor].sem);

	return err;
}

static int vme_user_release(struct inode *inode, struct file *file)
{
	unsigned int minor = MINOR(inode->i_rdev);

	down(&image[minor].sem);

	
	image[minor].users--;

	up(&image[minor].sem);

	return 0;
}


static ssize_t resource_to_user(int minor, char __user *buf, size_t count,
	loff_t *ppos)
{
	ssize_t retval;
	ssize_t copied = 0;

	if (count <= image[minor].size_buf) {
		
		copied = vme_master_read(image[minor].resource,
			image[minor].kern_buf, count, *ppos);
		if (copied < 0) {
			return (int)copied;
		}

		retval = __copy_to_user(buf, image[minor].kern_buf,
			(unsigned long)copied);
		if (retval != 0) {
			copied = (copied - retval);
			printk("User copy failed\n");
			return -EINVAL;
		}

	} else {
		
		printk("Currently don't support large transfers\n");
		

		
		return -EINVAL;
	}

	return copied;
}


static ssize_t resource_from_user(unsigned int minor, const char *buf,
	size_t count, loff_t *ppos)
{
	ssize_t retval;
	ssize_t copied = 0;

	if (count <= image[minor].size_buf) {
		retval = __copy_from_user(image[minor].kern_buf, buf,
			(unsigned long)count);
		if (retval != 0)
			copied = (copied - retval);
		else
			copied = count;

		copied = vme_master_write(image[minor].resource,
			image[minor].kern_buf, copied, *ppos);
	} else {
		
		printk("Currently don't support large transfers\n");
		

		
		return -EINVAL;
	}

	return copied;
}

static ssize_t buffer_to_user(unsigned int minor, char __user *buf,
	size_t count, loff_t *ppos)
{
	void __iomem *image_ptr;
	ssize_t retval;

	image_ptr = image[minor].kern_buf + *ppos;

	retval = __copy_to_user(buf, image_ptr, (unsigned long)count);
	if (retval != 0) {
		retval = (count - retval);
		printk(KERN_WARNING "Partial copy to userspace\n");
	} else
		retval = count;

	
	return retval;
}

static ssize_t buffer_from_user(unsigned int minor, const char *buf,
	size_t count, loff_t *ppos)
{
	void __iomem *image_ptr;
	size_t retval;

	image_ptr = image[minor].kern_buf + *ppos;

	retval = __copy_from_user(image_ptr, buf, (unsigned long)count);
	if (retval != 0) {
		retval = (count - retval);
		printk(KERN_WARNING "Partial copy to userspace\n");
	} else
		retval = count;

	
	return retval;
}

static ssize_t vme_user_read(struct file *file, char *buf, size_t count,
			loff_t * ppos)
{
	unsigned int minor = MINOR(file->f_dentry->d_inode->i_rdev);
	ssize_t retval;
	size_t image_size;
	size_t okcount;

	down(&image[minor].sem);

	
	image_size = vme_get_size(image[minor].resource);

	
	if ((*ppos < 0) || (*ppos > (image_size - 1))) {
		up(&image[minor].sem);
		return 0;
	}

	
	if (*ppos + count > image_size)
		okcount = image_size - *ppos;
	else
		okcount = count;

	switch (type[minor]){
	case MASTER_MINOR:
		retval = resource_to_user(minor, buf, okcount, ppos);
		break;
	case SLAVE_MINOR:
		retval = buffer_to_user(minor, buf, okcount, ppos);
		break;
	default:
		retval = -EINVAL;
	}

	up(&image[minor].sem);

	if (retval > 0)
		*ppos += retval;

	return retval;
}

static ssize_t vme_user_write(struct file *file, const char *buf, size_t count,
			 loff_t *ppos)
{
	unsigned int minor = MINOR(file->f_dentry->d_inode->i_rdev);
	ssize_t retval;
	size_t image_size;
	size_t okcount;

	down(&image[minor].sem);

	image_size = vme_get_size(image[minor].resource);

	
	if ((*ppos < 0) || (*ppos > (image_size - 1))) {
		up(&image[minor].sem);
		return 0;
	}

	
	if (*ppos + count > image_size)
		okcount = image_size - *ppos;
	else
		okcount = count;

	switch (type[minor]){
	case MASTER_MINOR:
		retval = resource_from_user(minor, buf, okcount, ppos);
		break;
	case SLAVE_MINOR:
		retval = buffer_from_user(minor, buf, okcount, ppos);
		break;
	default:
		retval = -EINVAL;
	}

	up(&image[minor].sem);

	if (retval > 0)
		*ppos += retval;

	return retval;
}

static loff_t vme_user_llseek(struct file *file, loff_t off, int whence)
{
	printk(KERN_ERR "Llseek currently incomplete\n");
	return -EINVAL;
}


static int vme_user_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	struct vme_master master;
	struct vme_slave slave;
	unsigned long copied;
	unsigned int minor = MINOR(inode->i_rdev);
	int retval;
	dma_addr_t pci_addr;

	statistics.ioctls++;

	switch (type[minor]) {
	case CONTROL_MINOR:
		break;
	case MASTER_MINOR:
		switch (cmd) {
		case VME_GET_MASTER:
			memset(&master, 0, sizeof(struct vme_master));

			
			retval = vme_master_get(image[minor].resource,
				&(master.enable), &(master.vme_addr),
				&(master.size), &(master.aspace),
				&(master.cycle), &(master.dwidth));

			copied = copy_to_user((char *)arg, &master,
				sizeof(struct vme_master));
			if (copied != 0) {
				printk(KERN_WARNING "Partial copy to "
					"userspace\n");
				return -EFAULT;
			}

			return retval;
			break;

		case VME_SET_MASTER:

			copied = copy_from_user(&master, (char *)arg,
				sizeof(master));
			if (copied != 0) {
				printk(KERN_WARNING "Partial copy from "
					"userspace\n");
				return -EFAULT;
			}

			
			return vme_master_set(image[minor].resource,
				master.enable, master.vme_addr, master.size,
				master.aspace, master.cycle, master.dwidth);

			break;
		}
		break;
	case SLAVE_MINOR:
		switch (cmd) {
		case VME_GET_SLAVE:
			memset(&slave, 0, sizeof(struct vme_slave));

			
			retval = vme_slave_get(image[minor].resource,
				&(slave.enable), &(slave.vme_addr),
				&(slave.size), &pci_addr, &(slave.aspace),
				&(slave.cycle));

			copied = copy_to_user((char *)arg, &slave,
				sizeof(struct vme_slave));
			if (copied != 0) {
				printk(KERN_WARNING "Partial copy to "
					"userspace\n");
				return -EFAULT;
			}

			return retval;
			break;

		case VME_SET_SLAVE:

			copied = copy_from_user(&slave, (char *)arg,
				sizeof(slave));
			if (copied != 0) {
				printk(KERN_WARNING "Partial copy from "
					"userspace\n");
				return -EFAULT;
			}

			
			return vme_slave_set(image[minor].resource,
				slave.enable, slave.vme_addr, slave.size,
				image[minor].pci_buf, slave.aspace,
				slave.cycle);

			break;
		}
		break;
	}

	return -EINVAL;
}



static void buf_unalloc (int num)
{
	if (image[num].kern_buf) {
#ifdef VME_DEBUG
		printk(KERN_DEBUG "UniverseII:Releasing buffer at %p\n",
			image[num].pci_buf);
#endif

		vme_free_consistent(image[num].resource, image[num].size_buf,
			image[num].kern_buf, image[num].pci_buf);

		image[num].kern_buf = NULL;
		image[num].pci_buf = 0;
		image[num].size_buf = 0;

#ifdef VME_DEBUG
	} else {
		printk(KERN_DEBUG "UniverseII: Buffer not allocated\n");
#endif
	}
}

static struct vme_driver vme_user_driver = {
        .name = driver_name,
        .probe = vme_user_probe,
	.remove = vme_user_remove,
};


static int __init vme_user_init(void)
{
	int retval = 0;
	int i;
	struct vme_device_id *ids;

	printk(KERN_INFO "VME User Space Access Driver\n");

	if (bus_num == 0) {
		printk(KERN_ERR "%s: No cards, skipping registration\n",
			driver_name);
		goto err_nocard;
	}

	
	if (bus_num > USER_BUS_MAX) {
		printk(KERN_ERR "%s: Driver only able to handle %d PIO2 "
			"Cards\n", driver_name, USER_BUS_MAX);
		bus_num = USER_BUS_MAX;
	}


	
	ids = kmalloc(sizeof(struct vme_device_id) * (bus_num + 1), GFP_KERNEL);
	if (ids == NULL) {
		printk(KERN_ERR "%s: Unable to allocate ID table\n",
			driver_name);
		goto err_id;
	}

	memset(ids, 0, (sizeof(struct vme_device_id) * (bus_num + 1)));

	for (i = 0; i < bus_num; i++) {
		ids[i].bus = bus[i];
		
		ids[i].slot = VME_SLOT_CURRENT;
	}

	vme_user_driver.bind_table = ids;

	retval = vme_register_driver(&vme_user_driver);
	if (retval != 0)
		goto err_reg;

	return retval;

	vme_unregister_driver(&vme_user_driver);
err_reg:
	kfree(ids);
err_id:
err_nocard:
	return retval;
}


static int __init vme_user_probe(struct device *dev, int cur_bus, int cur_slot)
{
	int i, err;
	char name[8];

	
	if (vme_user_bridge != NULL) {
		printk(KERN_ERR "%s: Driver can only be loaded for 1 device\n",
			driver_name);
		err = -EINVAL;
		goto err_dev;
	}
	vme_user_bridge = dev;

	
	for (i = 0; i < VME_DEVS; i++) {
		image[i].kern_buf = NULL;
		image[i].pci_buf = 0;
		init_MUTEX(&(image[i].sem));
		image[i].device = NULL;
		image[i].resource = NULL;
		image[i].users = 0;
	}

	
	reset_counters();

	
	err = register_chrdev_region(MKDEV(VME_MAJOR, 0), VME_DEVS,
		driver_name);
	if (err) {
		printk(KERN_WARNING "%s: Error getting Major Number %d for "
		"driver.\n", driver_name, VME_MAJOR);
		goto err_region;
	}

	
	vme_user_cdev = cdev_alloc();
	vme_user_cdev->ops = &vme_user_fops;
	vme_user_cdev->owner = THIS_MODULE;
	err = cdev_add(vme_user_cdev, MKDEV(VME_MAJOR, 0), VME_DEVS);
	if (err) {
		printk(KERN_WARNING "%s: cdev_all failed\n", driver_name);
		goto err_char;
	}

	
	for (i = SLAVE_MINOR; i < (SLAVE_MAX + 1); i++) {
		
		image[i].resource = vme_slave_request(vme_user_bridge,
			VME_A16, VME_SCT);
		if (image[i].resource == NULL) {
			printk(KERN_WARNING "Unable to allocate slave "
				"resource\n");
			goto err_slave;
		}
		image[i].size_buf = PCI_BUF_SIZE;
		image[i].kern_buf = vme_alloc_consistent(image[i].resource,
			image[i].size_buf, &(image[i].pci_buf));
		if (image[i].kern_buf == NULL) {
			printk(KERN_WARNING "Unable to allocate memory for "
				"buffer\n");
			image[i].pci_buf = 0;
			vme_slave_free(image[i].resource);
			err = -ENOMEM;
			goto err_slave;
		}
	}

	
	for (i = MASTER_MINOR; i < (MASTER_MAX + 1); i++) {
		
		image[i].resource = vme_master_request(vme_user_bridge,
			VME_A32, VME_SCT, VME_D32);
		if (image[i].resource == NULL) {
			printk(KERN_WARNING "Unable to allocate master "
				"resource\n");
			goto err_master;
		}
	}

	
	vme_user_sysfs_class = class_create(THIS_MODULE, driver_name);
	if (IS_ERR(vme_user_sysfs_class)) {
		printk(KERN_ERR "Error creating vme_user class.\n");
		err = PTR_ERR(vme_user_sysfs_class);
		goto err_class;
	}

	
	for (i=0; i<VME_DEVS; i++) {
		switch (type[i]) {
		case MASTER_MINOR:
			sprintf(name,"bus/vme/m%%d");
			break;
		case CONTROL_MINOR:
			sprintf(name,"bus/vme/ctl");
			break;
		case SLAVE_MINOR:
			sprintf(name,"bus/vme/s%%d");
			break;
		default:
			err = -EINVAL;
			goto err_sysfs;
			break;
		}

		image[i].device =
			device_create(vme_user_sysfs_class, NULL,
				MKDEV(VME_MAJOR, i), NULL, name,
				(type[i] == SLAVE_MINOR)? i - (MASTER_MAX + 1) : i);
		if (IS_ERR(image[i].device)) {
			printk("%s: Error creating sysfs device\n",
				driver_name);
			err = PTR_ERR(image[i].device);
			goto err_sysfs;
		}
	}

	return 0;

	
	i = VME_DEVS;
err_sysfs:
	while (i > 0){
		i--;
		device_destroy(vme_user_sysfs_class, MKDEV(VME_MAJOR, i));
	}
	class_destroy(vme_user_sysfs_class);

	
	i = MASTER_MAX + 1;
err_master:
	while (i > MASTER_MINOR) {
		i--;
		vme_master_free(image[i].resource);
	}

	
	i = SLAVE_MAX + 1;
err_slave:
	while (i > SLAVE_MINOR) {
		i--;
		vme_slave_free(image[i].resource);
		buf_unalloc(i);
	}
err_class:
	cdev_del(vme_user_cdev);
err_char:
	unregister_chrdev_region(MKDEV(VME_MAJOR, 0), VME_DEVS);
err_region:
err_dev:
	return err;
}

static int __exit vme_user_remove(struct device *dev, int cur_bus, int cur_slot)
{
	int i;

	
	for(i=0; i<VME_DEVS; i++) {
		device_destroy(vme_user_sysfs_class, MKDEV(VME_MAJOR, i));
	}
	class_destroy(vme_user_sysfs_class);

	for (i = SLAVE_MINOR; i < (SLAVE_MAX + 1); i++) {
		vme_slave_set(image[i].resource, 0, 0, 0, 0, VME_A32, 0);
		vme_slave_free(image[i].resource);
		buf_unalloc(i);
	}

	
	cdev_del(vme_user_cdev);

	
	unregister_chrdev_region(MKDEV(VME_MAJOR, 0), VME_DEVS);

	return 0;
}

static void __exit vme_user_exit(void)
{
	vme_unregister_driver(&vme_user_driver);

	kfree(vme_user_driver.bind_table);
}


MODULE_PARM_DESC(bus, "Enumeration of VMEbus to which the driver is connected");
module_param_array(bus, int, &bus_num, 0);

MODULE_DESCRIPTION("VME User Space Access Driver");
MODULE_AUTHOR("Martyn Welch <martyn.welch@gefanuc.com");
MODULE_LICENSE("GPL");

module_init(vme_user_init);
module_exit(vme_user_exit);
