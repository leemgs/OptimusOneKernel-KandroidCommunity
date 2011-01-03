

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/trdevice.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>

#include "tms380tr.h"

static char version[] __devinitdata =
"tmspci.c: v1.02 23/11/2000 by Adam Fritzler\n";

#define TMS_PCI_IO_EXTENT 32

struct card_info {
	unsigned char nselout[2]; 
	char *name;
};

static struct card_info card_info_table[] = {
	{ {0x03, 0x01}, "Compaq 4/16 TR PCI"},
	{ {0x03, 0x01}, "SK NET TR 4/16 PCI"},
	{ {0x03, 0x01}, "Thomas-Conrad TC4048 PCI 4/16"},
	{ {0x03, 0x01}, "3Com Token Link Velocity"},
};

static struct pci_device_id tmspci_pci_tbl[] = {
	{ PCI_VENDOR_ID_COMPAQ, PCI_DEVICE_ID_COMPAQ_TOKENRING, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ PCI_VENDOR_ID_SYSKONNECT, PCI_DEVICE_ID_SYSKONNECT_TR, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 1 },
	{ PCI_VENDOR_ID_TCONRAD, PCI_DEVICE_ID_TCONRAD_TOKENRING, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 2 },
	{ PCI_VENDOR_ID_3COM, PCI_DEVICE_ID_3COM_3C339, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 3 },
	{ }			
};
MODULE_DEVICE_TABLE(pci, tmspci_pci_tbl);

MODULE_LICENSE("GPL");

static void tms_pci_read_eeprom(struct net_device *dev);
static unsigned short tms_pci_setnselout_pins(struct net_device *dev);

static unsigned short tms_pci_sifreadb(struct net_device *dev, unsigned short reg)
{
	return inb(dev->base_addr + reg);
}

static unsigned short tms_pci_sifreadw(struct net_device *dev, unsigned short reg)
{
	return inw(dev->base_addr + reg);
}

static void tms_pci_sifwriteb(struct net_device *dev, unsigned short val, unsigned short reg)
{
	outb(val, dev->base_addr + reg);
}

static void tms_pci_sifwritew(struct net_device *dev, unsigned short val, unsigned short reg)
{
	outw(val, dev->base_addr + reg);
}

static int __devinit tms_pci_attach(struct pci_dev *pdev, const struct pci_device_id *ent)
{	
	static int versionprinted;
	struct net_device *dev;
	struct net_local *tp;
	int ret;
	unsigned int pci_irq_line;
	unsigned long pci_ioaddr;
	struct card_info *cardinfo = &card_info_table[ent->driver_data];

	if (versionprinted++ == 0)
		printk("%s", version);

	if (pci_enable_device(pdev))
		return -EIO;

	
	pci_irq_line = pdev->irq;
	pci_ioaddr = pci_resource_start (pdev, 0);

	
	dev = alloc_trdev(sizeof(struct net_local));
	if (!dev)
		return -ENOMEM;

	if (!request_region(pci_ioaddr, TMS_PCI_IO_EXTENT, dev->name)) {
		ret = -EBUSY;
		goto err_out_trdev;
	}

	dev->base_addr	= pci_ioaddr;
	dev->irq 	= pci_irq_line;
	dev->dma	= 0;

	printk("%s: %s\n", dev->name, cardinfo->name);
	printk("%s:    IO: %#4lx  IRQ: %d\n",
	       dev->name, dev->base_addr, dev->irq);
		
	tms_pci_read_eeprom(dev);

	printk("%s:    Ring Station Address: %pM\n",
	       dev->name, dev->dev_addr);
		
	ret = tmsdev_init(dev, &pdev->dev);
	if (ret) {
		printk("%s: unable to get memory for dev->priv.\n", dev->name);
		goto err_out_region;
	}

	tp = netdev_priv(dev);
	tp->setnselout = tms_pci_setnselout_pins;
		
	tp->sifreadb = tms_pci_sifreadb;
	tp->sifreadw = tms_pci_sifreadw;
	tp->sifwriteb = tms_pci_sifwriteb;
	tp->sifwritew = tms_pci_sifwritew;
		
	memcpy(tp->ProductID, cardinfo->name, PROD_ID_SIZE + 1);

	tp->tmspriv = cardinfo;

	dev->netdev_ops = &tms380tr_netdev_ops;

	ret = request_irq(pdev->irq, tms380tr_interrupt, IRQF_SHARED,
			  dev->name, dev);
	if (ret)
		goto err_out_tmsdev;

	pci_set_drvdata(pdev, dev);
	SET_NETDEV_DEV(dev, &pdev->dev);

	ret = register_netdev(dev);
	if (ret)
		goto err_out_irq;
	
	return 0;

err_out_irq:
	free_irq(pdev->irq, dev);
err_out_tmsdev:
	pci_set_drvdata(pdev, NULL);
	tmsdev_term(dev);
err_out_region:
	release_region(pci_ioaddr, TMS_PCI_IO_EXTENT);
err_out_trdev:
	free_netdev(dev);
	return ret;
}


static void tms_pci_read_eeprom(struct net_device *dev)
{
	int i;
	
	
	tms_pci_sifwritew(dev, 0, SIFADX);
	tms_pci_sifwritew(dev, 0, SIFADR);	
	
	
	dev->addr_len = 6;
	for(i = 0; i < 6; i++)
		dev->dev_addr[i] = tms_pci_sifreadw(dev, SIFINC) >> 8;
}

static unsigned short tms_pci_setnselout_pins(struct net_device *dev)
{
	unsigned short val = 0;
	struct net_local *tp = netdev_priv(dev);
	struct card_info *cardinfo = tp->tmspriv;
  
	if(tp->DataRate == SPEED_4)
		val |= cardinfo->nselout[0];	
	else
		val |= cardinfo->nselout[1];	
	return val;
}

static void __devexit tms_pci_detach (struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);

	BUG_ON(!dev);
	unregister_netdev(dev);
	release_region(dev->base_addr, TMS_PCI_IO_EXTENT);
	free_irq(dev->irq, dev);
	tmsdev_term(dev);
	free_netdev(dev);
	pci_set_drvdata(pdev, NULL);
}

static struct pci_driver tms_pci_driver = {
	.name		= "tmspci",
	.id_table	= tmspci_pci_tbl,
	.probe		= tms_pci_attach,
	.remove		= __devexit_p(tms_pci_detach),
};

static int __init tms_pci_init (void)
{
	return pci_register_driver(&tms_pci_driver);
}

static void __exit tms_pci_rmmod (void)
{
	pci_unregister_driver (&tms_pci_driver);
}

module_init(tms_pci_init);
module_exit(tms_pci_rmmod);

