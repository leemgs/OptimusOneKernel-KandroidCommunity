

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/ide.h>
#include <asm/msr.h>

#define DRV_NAME	"cs5536"

enum {
	MSR_IDE_CFG		= 0x51300010,
	PCI_IDE_CFG		= 0x40,

	CFG			= 0,
	DTC			= 2,
	CAST			= 3,
	ETC			= 4,

	IDE_CFG_CHANEN		= (1 << 1),
	IDE_CFG_CABLE		= (1 << 17) | (1 << 16),

	IDE_D0_SHIFT		= 24,
	IDE_D1_SHIFT		= 16,
	IDE_DRV_MASK		= 0xff,

	IDE_CAST_D0_SHIFT	= 6,
	IDE_CAST_D1_SHIFT	= 4,
	IDE_CAST_DRV_MASK	= 0x3,

	IDE_CAST_CMD_SHIFT	= 24,
	IDE_CAST_CMD_MASK	= 0xff,

	IDE_ETC_UDMA_MASK	= 0xc0,
};

static int use_msr;

static int cs5536_read(struct pci_dev *pdev, int reg, u32 *val)
{
	if (unlikely(use_msr)) {
		u32 dummy;

		rdmsr(MSR_IDE_CFG + reg, *val, dummy);
		return 0;
	}

	return pci_read_config_dword(pdev, PCI_IDE_CFG + reg * 4, val);
}

static int cs5536_write(struct pci_dev *pdev, int reg, int val)
{
	if (unlikely(use_msr)) {
		wrmsr(MSR_IDE_CFG + reg, val, 0);
		return 0;
	}

	return pci_write_config_dword(pdev, PCI_IDE_CFG + reg * 4, val);
}

static void cs5536_program_dtc(ide_drive_t *drive, u8 tim)
{
	struct pci_dev *pdev = to_pci_dev(drive->hwif->dev);
	int dshift = (drive->dn & 1) ? IDE_D1_SHIFT : IDE_D0_SHIFT;
	u32 dtc;

	cs5536_read(pdev, DTC, &dtc);
	dtc &= ~(IDE_DRV_MASK << dshift);
	dtc |= tim << dshift;
	cs5536_write(pdev, DTC, dtc);
}



static u8 cs5536_cable_detect(ide_hwif_t *hwif)
{
	struct pci_dev *pdev = to_pci_dev(hwif->dev);
	u32 cfg;

	cs5536_read(pdev, CFG, &cfg);

	if (cfg & IDE_CFG_CABLE)
		return ATA_CBL_PATA80;
	else
		return ATA_CBL_PATA40;
}



static void cs5536_set_pio_mode(ide_drive_t *drive, const u8 pio)
{
	static const u8 drv_timings[5] = {
		0x98, 0x55, 0x32, 0x21, 0x20,
	};

	static const u8 addr_timings[5] = {
		0x2, 0x1, 0x0, 0x0, 0x0,
	};

	static const u8 cmd_timings[5] = {
		0x99, 0x92, 0x90, 0x22, 0x20,
	};

	struct pci_dev *pdev = to_pci_dev(drive->hwif->dev);
	ide_drive_t *pair = ide_get_pair_dev(drive);
	int cshift = (drive->dn & 1) ? IDE_CAST_D1_SHIFT : IDE_CAST_D0_SHIFT;
	unsigned long timings = (unsigned long)ide_get_drivedata(drive);
	u32 cast;
	u8 cmd_pio = pio;

	if (pair)
		cmd_pio = min(pio, ide_get_best_pio_mode(pair, 255, 4));

	timings &= (IDE_DRV_MASK << 8);
	timings |= drv_timings[pio];
	ide_set_drivedata(drive, (void *)timings);

	cs5536_program_dtc(drive, drv_timings[pio]);

	cs5536_read(pdev, CAST, &cast);

	cast &= ~(IDE_CAST_DRV_MASK << cshift);
	cast |= addr_timings[pio] << cshift;

	cast &= ~(IDE_CAST_CMD_MASK << IDE_CAST_CMD_SHIFT);
	cast |= cmd_timings[cmd_pio] << IDE_CAST_CMD_SHIFT;

	cs5536_write(pdev, CAST, cast);
}



static void cs5536_set_dma_mode(ide_drive_t *drive, const u8 mode)
{
	static const u8 udma_timings[6] = {
		0xc2, 0xc1, 0xc0, 0xc4, 0xc5, 0xc6,
	};

	static const u8 mwdma_timings[3] = {
		0x67, 0x21, 0x20,
	};

	struct pci_dev *pdev = to_pci_dev(drive->hwif->dev);
	int dshift = (drive->dn & 1) ? IDE_D1_SHIFT : IDE_D0_SHIFT;
	unsigned long timings = (unsigned long)ide_get_drivedata(drive);
	u32 etc;

	cs5536_read(pdev, ETC, &etc);

	if (mode >= XFER_UDMA_0) {
		etc &= ~(IDE_DRV_MASK << dshift);
		etc |= udma_timings[mode - XFER_UDMA_0] << dshift;
	} else { 
		etc &= ~(IDE_ETC_UDMA_MASK << dshift);
		timings &= IDE_DRV_MASK;
		timings |= mwdma_timings[mode - XFER_MW_DMA_0] << 8;
		ide_set_drivedata(drive, (void *)timings);
	}

	cs5536_write(pdev, ETC, etc);
}

static void cs5536_dma_start(ide_drive_t *drive)
{
	unsigned long timings = (unsigned long)ide_get_drivedata(drive);

	if (drive->current_speed < XFER_UDMA_0 &&
	    (timings >> 8) != (timings & IDE_DRV_MASK))
		cs5536_program_dtc(drive, timings >> 8);

	ide_dma_start(drive);
}

static int cs5536_dma_end(ide_drive_t *drive)
{
	int ret = ide_dma_end(drive);
	unsigned long timings = (unsigned long)ide_get_drivedata(drive);

	if (drive->current_speed < XFER_UDMA_0 &&
	    (timings >> 8) != (timings & IDE_DRV_MASK))
		cs5536_program_dtc(drive, timings & IDE_DRV_MASK);

	return ret;
}

static const struct ide_port_ops cs5536_port_ops = {
	.set_pio_mode		= cs5536_set_pio_mode,
	.set_dma_mode		= cs5536_set_dma_mode,
	.cable_detect		= cs5536_cable_detect,
};

static const struct ide_dma_ops cs5536_dma_ops = {
	.dma_host_set		= ide_dma_host_set,
	.dma_setup		= ide_dma_setup,
	.dma_start		= cs5536_dma_start,
	.dma_end		= cs5536_dma_end,
	.dma_test_irq		= ide_dma_test_irq,
	.dma_lost_irq		= ide_dma_lost_irq,
	.dma_timer_expiry	= ide_dma_sff_timer_expiry,
	.dma_sff_read_status	= ide_dma_sff_read_status,
};

static const struct ide_port_info cs5536_info = {
	.name		= DRV_NAME,
	.port_ops	= &cs5536_port_ops,
	.dma_ops	= &cs5536_dma_ops,
	.host_flags	= IDE_HFLAG_SINGLE,
	.pio_mask	= ATA_PIO4,
	.mwdma_mask	= ATA_MWDMA2,
	.udma_mask	= ATA_UDMA5,
};



static int cs5536_init_one(struct pci_dev *dev, const struct pci_device_id *id)
{
	u32 cfg;

	if (use_msr)
		printk(KERN_INFO DRV_NAME ": Using MSR regs instead of PCI\n");

	cs5536_read(dev, CFG, &cfg);

	if ((cfg & IDE_CFG_CHANEN) == 0) {
		printk(KERN_ERR DRV_NAME ": disabled by BIOS\n");
		return -ENODEV;
	}

	return ide_pci_init_one(dev, &cs5536_info, NULL);
}

static const struct pci_device_id cs5536_pci_tbl[] = {
	{ PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_CS5536_IDE), },
	{ },
};

static struct pci_driver cs5536_pci_driver = {
	.name		= DRV_NAME,
	.id_table	= cs5536_pci_tbl,
	.probe		= cs5536_init_one,
	.remove		= ide_pci_remove,
	.suspend	= ide_pci_suspend,
	.resume		= ide_pci_resume,
};

static int __init cs5536_init(void)
{
	return pci_register_driver(&cs5536_pci_driver);
}

static void __exit cs5536_exit(void)
{
	pci_unregister_driver(&cs5536_pci_driver);
}

MODULE_AUTHOR("Martin K. Petersen, Bartlomiej Zolnierkiewicz");
MODULE_DESCRIPTION("low-level driver for the CS5536 IDE controller");
MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(pci, cs5536_pci_tbl);

module_param_named(msr, use_msr, int, 0644);
MODULE_PARM_DESC(msr, "Force using MSR to configure IDE function (Default: 0)");

module_init(cs5536_init);
module_exit(cs5536_exit);
