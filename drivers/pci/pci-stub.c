

#include <linux/module.h>
#include <linux/pci.h>

static char ids[1024] __initdata;

module_param_string(ids, ids, sizeof(ids), 0);
MODULE_PARM_DESC(ids, "Initial PCI IDs to add to the stub driver, format is "
		 "\"vendor:device[:subvendor[:subdevice[:class[:class_mask]]]]\""
		 " and multiple comma separated entries can be specified");

static int pci_stub_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	dev_printk(KERN_INFO, &dev->dev, "claimed by stub\n");
	return 0;
}

static struct pci_driver stub_driver = {
	.name		= "pci-stub",
	.id_table	= NULL,	
	.probe		= pci_stub_probe,
};

static int __init pci_stub_init(void)
{
	char *p, *id;
	int rc;

	rc = pci_register_driver(&stub_driver);
	if (rc)
		return rc;

	
	p = ids;
	while ((id = strsep(&p, ","))) {
		unsigned int vendor, device, subvendor = PCI_ANY_ID,
			subdevice = PCI_ANY_ID, class=0, class_mask=0;
		int fields;

		fields = sscanf(id, "%x:%x:%x:%x:%x:%x",
				&vendor, &device, &subvendor, &subdevice,
				&class, &class_mask);

		if (fields < 2) {
			printk(KERN_WARNING
			       "pci-stub: invalid id string \"%s\"\n", id);
			continue;
		}

		printk(KERN_INFO
		       "pci-stub: add %04X:%04X sub=%04X:%04X cls=%08X/%08X\n",
		       vendor, device, subvendor, subdevice, class, class_mask);

		rc = pci_add_dynid(&stub_driver, vendor, device,
				   subvendor, subdevice, class, class_mask, 0);
		if (rc)
			printk(KERN_WARNING
			       "pci-stub: failed to add dynamic id (%d)\n", rc);
	}

	return 0;
}

static void __exit pci_stub_exit(void)
{
	pci_unregister_driver(&stub_driver);
}

module_init(pci_stub_init);
module_exit(pci_stub_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chris Wright <chrisw@sous-sol.org>");
