


#include "usbip_common.h"
#include "stub.h"


#define DRIVER_VERSION "1.0"
#define DRIVER_AUTHOR "Takahiro Hirofuchi"
#define DRIVER_DESC "Stub Driver for USB/IP"


struct kmem_cache *stub_priv_cache;







#define MAX_BUSID 16
#define BUSID_SIZE 20
static char busid_table[MAX_BUSID][BUSID_SIZE];
static spinlock_t busid_table_lock;


int match_busid(const char *busid)
{
	int i;

	spin_lock(&busid_table_lock);

	for (i = 0; i < MAX_BUSID; i++)
		if (busid_table[i][0])
			if (!strncmp(busid_table[i], busid, BUSID_SIZE)) {
				
				spin_unlock(&busid_table_lock);
				return 0;
			}

	spin_unlock(&busid_table_lock);

	return 1;
}

static ssize_t show_match_busid(struct device_driver *drv, char *buf)
{
	int i;
	char *out = buf;

	spin_lock(&busid_table_lock);

	for (i = 0; i < MAX_BUSID; i++)
		if (busid_table[i][0])
			out += sprintf(out, "%s ", busid_table[i]);

	spin_unlock(&busid_table_lock);

	out += sprintf(out, "\n");

	return out - buf;
}

static int add_match_busid(char *busid)
{
	int i;

	if (!match_busid(busid))
		return 0;

	spin_lock(&busid_table_lock);

	for (i = 0; i < MAX_BUSID; i++)
		if (!busid_table[i][0]) {
			strncpy(busid_table[i], busid, BUSID_SIZE);
			spin_unlock(&busid_table_lock);
			return 0;
		}

	spin_unlock(&busid_table_lock);

	return -1;
}

static int del_match_busid(char *busid)
{
	int i;

	spin_lock(&busid_table_lock);

	for (i = 0; i < MAX_BUSID; i++)
		if (!strncmp(busid_table[i], busid, BUSID_SIZE)) {
			
			memset(busid_table[i], 0, BUSID_SIZE);
			spin_unlock(&busid_table_lock);
			return 0;
		}

	spin_unlock(&busid_table_lock);

	return -1;
}

static ssize_t store_match_busid(struct device_driver *dev, const char *buf,
		size_t count)
{
	int len;
	char busid[BUSID_SIZE];

	if (count < 5)
		return -EINVAL;

	
	len = strnlen(buf + 4, BUSID_SIZE);

	
	if (!(len < BUSID_SIZE))
		return -EINVAL;

	strncpy(busid, buf + 4, BUSID_SIZE);


	if (!strncmp(buf, "add ", 4)) {
		if (add_match_busid(busid) < 0)
			return -ENOMEM;
		else {
			usbip_udbg("add busid %s\n", busid);
			return count;
		}
	} else if (!strncmp(buf, "del ", 4)) {
		if (del_match_busid(busid) < 0)
			return -ENODEV;
		else {
			usbip_udbg("del busid %s\n", busid);
			return count;
		}
	} else
		return -EINVAL;
}

static DRIVER_ATTR(match_busid, S_IRUSR|S_IWUSR, show_match_busid,
							store_match_busid);







static struct stub_priv *stub_priv_pop_from_listhead(struct list_head *listhead)
{
	struct stub_priv *priv, *tmp;

	list_for_each_entry_safe(priv, tmp, listhead, list) {
		list_del(&priv->list);
		return priv;
	}

	return NULL;
}

static struct stub_priv *stub_priv_pop(struct stub_device *sdev)
{
	unsigned long flags;
	struct stub_priv *priv;

	spin_lock_irqsave(&sdev->priv_lock, flags);

	priv = stub_priv_pop_from_listhead(&sdev->priv_init);
	if (priv) {
		spin_unlock_irqrestore(&sdev->priv_lock, flags);
		return priv;
	}

	priv = stub_priv_pop_from_listhead(&sdev->priv_tx);
	if (priv) {
		spin_unlock_irqrestore(&sdev->priv_lock, flags);
		return priv;
	}

	priv = stub_priv_pop_from_listhead(&sdev->priv_free);
	if (priv) {
		spin_unlock_irqrestore(&sdev->priv_lock, flags);
		return priv;
	}

	spin_unlock_irqrestore(&sdev->priv_lock, flags);
	return NULL;
}

void stub_device_cleanup_urbs(struct stub_device *sdev)
{
	struct stub_priv *priv;

	usbip_udbg("free sdev %p\n", sdev);

	while ((priv = stub_priv_pop(sdev))) {
		struct urb *urb = priv->urb;

		usbip_udbg("   free urb %p\n", urb);
		usb_kill_urb(urb);

		kmem_cache_free(stub_priv_cache, priv);

		if (urb->transfer_buffer != NULL)
			kfree(urb->transfer_buffer);

		if (urb->setup_packet != NULL)
			kfree(urb->setup_packet);

		usb_free_urb(urb);
	}
}




static int __init usb_stub_init(void)
{
	int ret;

	stub_priv_cache = kmem_cache_create("stub_priv",
					    sizeof(struct stub_priv), 0,
					    SLAB_HWCACHE_ALIGN, NULL);

	if (!stub_priv_cache) {
		printk(KERN_ERR KBUILD_MODNAME
		       ": create stub_priv_cache error\n");
		return -ENOMEM;
	}

	ret = usb_register(&stub_driver);
	if (ret) {
		printk(KERN_ERR KBUILD_MODNAME ": usb_register failed %d\n",
		       ret);
		goto error_usb_register;
	}

	printk(KERN_INFO KBUILD_MODNAME ":"
	       DRIVER_DESC ":" DRIVER_VERSION "\n");

	memset(busid_table, 0, sizeof(busid_table));
	spin_lock_init(&busid_table_lock);

	ret = driver_create_file(&stub_driver.drvwrap.driver,
				 &driver_attr_match_busid);

	if (ret) {
		printk(KERN_ERR KBUILD_MODNAME ": create driver sysfs\n");
		goto error_create_file;
	}

	return ret;
error_create_file:
	usb_deregister(&stub_driver);
error_usb_register:
	kmem_cache_destroy(stub_priv_cache);
	return ret;
}

static void __exit usb_stub_exit(void)
{
	driver_remove_file(&stub_driver.drvwrap.driver,
			   &driver_attr_match_busid);

	
	usb_deregister(&stub_driver);

	kmem_cache_destroy(stub_priv_cache);
}

module_init(usb_stub_init);
module_exit(usb_stub_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
