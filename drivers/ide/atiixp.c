

#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/ide.h>
#include <linux/init.h>

#define DRV_NAME "atiixp"

#define ATIIXP_IDE_PIO_TIMING		0x40
#define ATIIXP_IDE_MDMA_TIMING		0x44
#define ATIIXP_IDE_PIO_CONTROL		0x48
#define ATIIXP_IDE_PIO_MODE		0x4a
#define ATIIXP_IDE_UDMA_CONTROL		0x54
#define ATIIXP_IDE_UDMA_MODE		0x56

typedef struct {
	u8 command_width;
	u8 recover_width;
} atiixp_ide_timing;

static atiixp_ide_timing pio_timing[] = {
	{ 0x05, 0x0d },
	{ 0x04, 0x07 },
	{ 0x03, 0x04 },
	{ 0x02, 0x02 },
	{ 0x02, 0x00 },
};

static atiixp_ide_timing mdma_timing[] = {
	{ 0x07, 0x07 },
	{ 0x02, 0x01 },
	{ 0x02, 0x00 },
};

static DEFINE_SPINLOCK(atiixp_lock);



static void atiixp_set_pio_mode(ide_drive_t *drive, const u8 pio)
{
	struct pci_dev *dev = to_pci_dev(drive->hwif->dev);
	unsigned long flags;
	int timing_shift = (drive->dn ^ 1) * 8;
	u32 pio_timing_data;
	u16 pio_mode_data;

	spin_lock_irqsave(&atiixp_lock, flags);

	pci_read_config_word(dev, ATIIXP_IDE_PIO_MODE, &pio_mode_data);
	pio_mode_data &= ~(0x07 << (drive->dn * 4));
	pio_mode_data |= (pio << (drive->dn * 4));
	pci_write_config_word(dev, ATIIXP_IDE_PIO_MODE, pio_mode_data);

	pci_read_config_dword(dev, ATIIXP_IDE_PIO_TIMING, &pio_timing_data);
	pio_timing_data &= ~(0xff << timing_shift);
	pio_timing_data |= (pio_timing[pio].recover_width << timing_shift) |
		 (pio_timing[pio].command_width << (timing_shift + 4));
	pci_write_config_dword(dev, ATIIXP_IDE_PIO_TIMING, pio_timing_data);

	spin_unlock_irqrestore(&atiixp_lock, flags);
}



static void atiixp_set_dma_mode(ide_drive_t *drive, const u8 speed)
{
	struct pci_dev *dev = to_pci_dev(drive->hwif->dev);
	unsigned long flags;
	int timing_shift = (drive->dn ^ 1) * 8;
	u32 tmp32;
	u16 tmp16;
	u16 udma_ctl = 0;

	spin_lock_irqsave(&atiixp_lock, flags);

	pci_read_config_word(dev, ATIIXP_IDE_UDMA_CONTROL, &udma_ctl);

	if (speed >= XFER_UDMA_0) {
		pci_read_config_word(dev, ATIIXP_IDE_UDMA_MODE, &tmp16);
		tmp16 &= ~(0x07 << (drive->dn * 4));
		tmp16 |= ((speed & 0x07) << (drive->dn * 4));
		pci_write_config_word(dev, ATIIXP_IDE_UDMA_MODE, tmp16);

		udma_ctl |= (1 << drive->dn);
	} else if (speed >= XFER_MW_DMA_0) {
		u8 i = speed & 0x03;

		pci_read_config_dword(dev, ATIIXP_IDE_MDMA_TIMING, &tmp32);
		tmp32 &= ~(0xff << timing_shift);
		tmp32 |= (mdma_timing[i].recover_width << timing_shift) |
			 (mdma_timing[i].command_width << (timing_shift + 4));
		pci_write_config_dword(dev, ATIIXP_IDE_MDMA_TIMING, tmp32);

		udma_ctl &= ~(1 << drive->dn);
	}

	pci_write_config_word(dev, ATIIXP_IDE_UDMA_CONTROL, udma_ctl);

	spin_unlock_irqrestore(&atiixp_lock, flags);
}

static u8 atiixp_cable_detect(ide_hwif_t *hwif)
{
	struct pci_dev *pdev = to_pci_dev(hwif->dev);
	u8 udma_mode = 0, ch = hwif->channel;

	pci_read_config_byte(pdev, ATIIXP_IDE_UDMA_MODE + ch, &udma_mode);

	if ((udma_mode & 0x07) >= 0x04 || (udma_mode & 0x70) >= 0x40)
		return ATA_CBL_PATA80;
	else
		return ATA_CBL_PATA40;
}

static const struct ide_port_ops atiixp_port_ops = {
	.set_pio_mode		= atiixp_set_pio_mode,
	.set_dma_mode		= atiixp_set_dma_mode,
	.cable_detect		= atiixp_cable_detect,
};

static const struct ide_port_info atiixp_pci_info[] __devinitdata = {
	{	
		.name		= DRV_NAME,
		.enablebits	= {{0x48,0x01,0x00}, {0x48,0x08,0x00}},
		.port_ops	= &atiixp_port_ops,
		.pio_mask	= ATA_PIO4,
		.mwdma_mask	= ATA_MWDMA2,
		.udma_mask	= ATA_UDMA5,
	},
	{	
		.name		= DRV_NAME,
		.enablebits	= {{0x48,0x01,0x00}, {0x00,0x00,0x00}},
		.port_ops	= &atiixp_port_ops,
		.host_flags	= IDE_HFLAG_SINGLE,
		.pio_mask	= ATA_PIO4,
		.mwdma_mask	= ATA_MWDMA2,
		.udma_mask	= ATA_UDMA5,
 	},
};



static int __devinit atiixp_init_one(struct pci_dev *dev, const struct pci_device_id *id)
{
	return ide_pci_init_one(dev, &atiixp_pci_info[id->driver_data], NULL);
}

static const struct pci_device_id atiixp_pci_tbl[] = {
	{ PCI_VDEVICE(ATI, PCI_DEVICE_ID_ATI_IXP200_IDE), 0 },
	{ PCI_VDEVICE(ATI, PCI_DEVICE_ID_ATI_IXP300_IDE), 0 },
	{ PCI_VDEVICE(ATI, PCI_DEVICE_ID_ATI_IXP400_IDE), 0 },
	{ PCI_VDEVICE(ATI, PCI_DEVICE_ID_ATI_IXP600_IDE), 1 },
	{ PCI_VDEVICE(ATI, PCI_DEVICE_ID_ATI_IXP700_IDE), 0 },
	{ PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_HUDSON2_IDE), 0 },
	{ 0, },
};
MODULE_DEVICE_TABLE(pci, atiixp_pci_tbl);

static struct pci_driver atiixp_pci_driver = {
	.name		= "ATIIXP_IDE",
	.id_table	= atiixp_pci_tbl,
	.probe		= atiixp_init_one,
	.remove		= ide_pci_remove,
	.suspend	= ide_pci_suspend,
	.resume		= ide_pci_resume,
};

static int __init atiixp_ide_init(void)
{
	return ide_pci_register_driver(&atiixp_pci_driver);
}

static void __exit atiixp_ide_exit(void)
{
	pci_unregister_driver(&atiixp_pci_driver);
}

module_init(atiixp_ide_init);
module_exit(atiixp_ide_exit);

MODULE_AUTHOR("HUI YU");
MODULE_DESCRIPTION("PCI driver module for ATI IXP IDE");
MODULE_LICENSE("GPL");
