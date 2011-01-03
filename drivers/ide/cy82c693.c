

#include <linux/module.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/ide.h>
#include <linux/init.h>

#include <asm/io.h>

#define DRV_NAME "cy82c693"


#define CY82C693_DEBUG_INFO	0




#define BUSMASTER_TIMEOUT	0x50



#define CY82_IDE_CMDREG		0x04
#define CY82_IDE_ADDRSETUP	0x48
#define CY82_IDE_MASTER_IOR	0x4C
#define CY82_IDE_MASTER_IOW	0x4D
#define CY82_IDE_SLAVE_IOR	0x4E
#define CY82_IDE_SLAVE_IOW	0x4F
#define CY82_IDE_MASTER_8BIT	0x50
#define CY82_IDE_SLAVE_8BIT	0x51

#define CY82_INDEX_PORT		0x22
#define CY82_DATA_PORT		0x23

#define CY82_INDEX_CHANNEL0	0x30
#define CY82_INDEX_CHANNEL1	0x31
#define CY82_INDEX_TIMEOUT	0x32


#define CY82C963_MIN_BUS_SPEED	25
#define CY82C963_MAX_BUS_SPEED	33


typedef struct pio_clocks_s {
	u8	address_time;	
	u8	time_16r;	
	u8	time_16w;	
	u8	time_8;		
} pio_clocks_t;


static int calc_clk(int time, int bus_speed)
{
	int clocks;

	clocks = (time*bus_speed+999)/1000 - 1;

	if (clocks < 0)
		clocks = 0;

	if (clocks > 0x0F)
		clocks = 0x0F;

	return clocks;
}


static void compute_clocks(u8 pio, pio_clocks_t *p_pclk)
{
	struct ide_timing *t = ide_timing_find_mode(XFER_PIO_0 + pio);
	int clk1, clk2;
	int bus_speed = ide_pci_clk ? ide_pci_clk : 33;

	

	
	p_pclk->address_time = (u8)calc_clk(t->setup, bus_speed);

	
	clk1 = calc_clk(t->active, bus_speed);

	
	clk2 = t->cycle - t->active - t->setup;

	clk2 = calc_clk(clk2, bus_speed);

	clk1 = (clk1<<4)|clk2;	

	

	p_pclk->time_16r = (u8)clk1;
	p_pclk->time_16w = (u8)clk1;

	
	p_pclk->time_8 = (u8)clk1;
}



static void cy82c693_set_dma_mode(ide_drive_t *drive, const u8 mode)
{
	ide_hwif_t *hwif = drive->hwif;
	u8 single = (mode & 0x10) >> 4, index = 0, data = 0;

	index = hwif->channel ? CY82_INDEX_CHANNEL1 : CY82_INDEX_CHANNEL0;

	data = (mode & 3) | (single << 2);

	outb(index, CY82_INDEX_PORT);
	outb(data, CY82_DATA_PORT);

#if CY82C693_DEBUG_INFO
	printk(KERN_INFO "%s (ch=%d, dev=%d): set DMA mode to %d (single=%d)\n",
		drive->name, hwif->channel, drive->dn & 1, mode & 3, single);
#endif 

	

	data = BUSMASTER_TIMEOUT;
	outb(CY82_INDEX_TIMEOUT, CY82_INDEX_PORT);
	outb(data, CY82_DATA_PORT);

#if CY82C693_DEBUG_INFO
	printk(KERN_INFO "%s: Set IDE Bus Master TimeOut Register to 0x%X\n",
		drive->name, data);
#endif 
}

static void cy82c693_set_pio_mode(ide_drive_t *drive, const u8 pio)
{
	ide_hwif_t *hwif = drive->hwif;
	struct pci_dev *dev = to_pci_dev(hwif->dev);
	pio_clocks_t pclk;
	unsigned int addrCtrl;

	
	if (hwif->index > 0) {  
		dev = pci_get_slot(dev->bus, dev->devfn+1);
		if (!dev) {
			printk(KERN_ERR "%s: tune_drive: "
				"Cannot find secondary interface!\n",
				drive->name);
			return;
		}
	}

	
	compute_clocks(pio, &pclk);

	
	if ((drive->dn & 1) == 0) {
		
		pci_read_config_dword(dev, CY82_IDE_ADDRSETUP, &addrCtrl);

		addrCtrl &= (~0xF);
		addrCtrl |= (unsigned int)pclk.address_time;
		pci_write_config_dword(dev, CY82_IDE_ADDRSETUP, addrCtrl);

		
		pci_write_config_byte(dev, CY82_IDE_MASTER_IOR, pclk.time_16r);
		pci_write_config_byte(dev, CY82_IDE_MASTER_IOW, pclk.time_16w);
		pci_write_config_byte(dev, CY82_IDE_MASTER_8BIT, pclk.time_8);

		addrCtrl &= 0xF;
	} else {
		
		pci_read_config_dword(dev, CY82_IDE_ADDRSETUP, &addrCtrl);

		addrCtrl &= (~0xF0);
		addrCtrl |= ((unsigned int)pclk.address_time<<4);
		pci_write_config_dword(dev, CY82_IDE_ADDRSETUP, addrCtrl);

		
		pci_write_config_byte(dev, CY82_IDE_SLAVE_IOR, pclk.time_16r);
		pci_write_config_byte(dev, CY82_IDE_SLAVE_IOW, pclk.time_16w);
		pci_write_config_byte(dev, CY82_IDE_SLAVE_8BIT, pclk.time_8);

		addrCtrl >>= 4;
		addrCtrl &= 0xF;
	}

#if CY82C693_DEBUG_INFO
	printk(KERN_INFO "%s (ch=%d, dev=%d): set PIO timing to "
		"(addr=0x%X, ior=0x%X, iow=0x%X, 8bit=0x%X)\n",
		drive->name, hwif->channel, drive->dn & 1,
		addrCtrl, pclk.time_16r, pclk.time_16w, pclk.time_8);
#endif 
}

static void __devinit init_iops_cy82c693(ide_hwif_t *hwif)
{
	static ide_hwif_t *primary;
	struct pci_dev *dev = to_pci_dev(hwif->dev);

	if (PCI_FUNC(dev->devfn) == 1)
		primary = hwif;
	else {
		hwif->mate = primary;
		hwif->channel = 1;
	}
}

static const struct ide_port_ops cy82c693_port_ops = {
	.set_pio_mode		= cy82c693_set_pio_mode,
	.set_dma_mode		= cy82c693_set_dma_mode,
};

static const struct ide_port_info cy82c693_chipset __devinitdata = {
	.name		= DRV_NAME,
	.init_iops	= init_iops_cy82c693,
	.port_ops	= &cy82c693_port_ops,
	.host_flags	= IDE_HFLAG_SINGLE,
	.pio_mask	= ATA_PIO4,
	.swdma_mask	= ATA_SWDMA2,
	.mwdma_mask	= ATA_MWDMA2,
};

static int __devinit cy82c693_init_one(struct pci_dev *dev, const struct pci_device_id *id)
{
	struct pci_dev *dev2;
	int ret = -ENODEV;

	
	if ((dev->class >> 8) == PCI_CLASS_STORAGE_IDE &&
	    PCI_FUNC(dev->devfn) == 1) {
		dev2 = pci_get_slot(dev->bus, dev->devfn + 1);
		ret = ide_pci_init_two(dev, dev2, &cy82c693_chipset, NULL);
		if (ret)
			pci_dev_put(dev2);
	}
	return ret;
}

static void __devexit cy82c693_remove(struct pci_dev *dev)
{
	struct ide_host *host = pci_get_drvdata(dev);
	struct pci_dev *dev2 = host->dev[1] ? to_pci_dev(host->dev[1]) : NULL;

	ide_pci_remove(dev);
	pci_dev_put(dev2);
}

static const struct pci_device_id cy82c693_pci_tbl[] = {
	{ PCI_VDEVICE(CONTAQ, PCI_DEVICE_ID_CONTAQ_82C693), 0 },
	{ 0, },
};
MODULE_DEVICE_TABLE(pci, cy82c693_pci_tbl);

static struct pci_driver cy82c693_pci_driver = {
	.name		= "Cypress_IDE",
	.id_table	= cy82c693_pci_tbl,
	.probe		= cy82c693_init_one,
	.remove		= __devexit_p(cy82c693_remove),
	.suspend	= ide_pci_suspend,
	.resume		= ide_pci_resume,
};

static int __init cy82c693_ide_init(void)
{
	return ide_pci_register_driver(&cy82c693_pci_driver);
}

static void __exit cy82c693_ide_exit(void)
{
	pci_unregister_driver(&cy82c693_pci_driver);
}

module_init(cy82c693_ide_init);
module_exit(cy82c693_ide_exit);

MODULE_AUTHOR("Andreas Krebs, Andre Hedrick");
MODULE_DESCRIPTION("PCI driver module for the Cypress CY82C693 IDE");
MODULE_LICENSE("GPL");
