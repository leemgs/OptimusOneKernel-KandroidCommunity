

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/ide.h>

#include <asm/io.h>

#ifdef CONFIG_PPC_PMAC
#include <asm/prom.h>
#include <asm/pci-bridge.h>
#endif

#define DRV_NAME "pdc202xx_new"

#undef DEBUG

#ifdef DEBUG
#define DBG(fmt, args...) printk("%s: " fmt, __func__, ## args)
#else
#define DBG(fmt, args...)
#endif

static u8 max_dma_rate(struct pci_dev *pdev)
{
	u8 mode;

	switch(pdev->device) {
		case PCI_DEVICE_ID_PROMISE_20277:
		case PCI_DEVICE_ID_PROMISE_20276:
		case PCI_DEVICE_ID_PROMISE_20275:
		case PCI_DEVICE_ID_PROMISE_20271:
		case PCI_DEVICE_ID_PROMISE_20269:
			mode = 4;
			break;
		case PCI_DEVICE_ID_PROMISE_20270:
		case PCI_DEVICE_ID_PROMISE_20268:
			mode = 3;
			break;
		default:
			return 0;
	}

	return mode;
}


static u8 get_indexed_reg(ide_hwif_t *hwif, u8 index)
{
	u8 value;

	outb(index, hwif->dma_base + 1);
	value = inb(hwif->dma_base + 3);

	DBG("index[%02X] value[%02X]\n", index, value);
	return value;
}


static void set_indexed_reg(ide_hwif_t *hwif, u8 index, u8 value)
{
	outb(index, hwif->dma_base + 1);
	outb(value, hwif->dma_base + 3);
	DBG("index[%02X] value[%02X]\n", index, value);
}


static struct pio_timing {
	u8 reg0c, reg0d, reg13;
} pio_timings [] = {
	{ 0xfb, 0x2b, 0xac },	
	{ 0x46, 0x29, 0xa4 },	
	{ 0x23, 0x26, 0x64 },	
	{ 0x27, 0x0d, 0x35 },	
	{ 0x23, 0x09, 0x25 },	
};

static struct mwdma_timing {
	u8 reg0e, reg0f;
} mwdma_timings [] = {
	{ 0xdf, 0x5f }, 	
	{ 0x6b, 0x27 }, 	
	{ 0x69, 0x25 }, 	
};

static struct udma_timing {
	u8 reg10, reg11, reg12;
} udma_timings [] = {
	{ 0x4a, 0x0f, 0xd5 },	
	{ 0x3a, 0x0a, 0xd0 },	
	{ 0x2a, 0x07, 0xcd },	
	{ 0x1a, 0x05, 0xcd },	
	{ 0x1a, 0x03, 0xcd },	
	{ 0x1a, 0x02, 0xcb },	
	{ 0x1a, 0x01, 0xcb },	
};

static void pdcnew_set_dma_mode(ide_drive_t *drive, const u8 speed)
{
	ide_hwif_t *hwif	= drive->hwif;
	struct pci_dev *dev	= to_pci_dev(hwif->dev);
	u8 adj			= (drive->dn & 1) ? 0x08 : 0x00;

	
	if (max_dma_rate(dev) == 4) {
		u8 mode = speed & 0x07;

		if (speed >= XFER_UDMA_0) {
			set_indexed_reg(hwif, 0x10 + adj,
					udma_timings[mode].reg10);
			set_indexed_reg(hwif, 0x11 + adj,
					udma_timings[mode].reg11);
			set_indexed_reg(hwif, 0x12 + adj,
					udma_timings[mode].reg12);
		} else {
			set_indexed_reg(hwif, 0x0e + adj,
					mwdma_timings[mode].reg0e);
			set_indexed_reg(hwif, 0x0f + adj,
					mwdma_timings[mode].reg0f);
		}
	} else if (speed == XFER_UDMA_2) {
		
		u8 tmp = get_indexed_reg(hwif, 0x10 + adj);

		set_indexed_reg(hwif, 0x10 + adj, tmp & 0x7f);
 	}
}

static void pdcnew_set_pio_mode(ide_drive_t *drive, const u8 pio)
{
	ide_hwif_t *hwif = drive->hwif;
	struct pci_dev *dev = to_pci_dev(hwif->dev);
	u8 adj = (drive->dn & 1) ? 0x08 : 0x00;

	if (max_dma_rate(dev) == 4) {
		set_indexed_reg(hwif, 0x0c + adj, pio_timings[pio].reg0c);
		set_indexed_reg(hwif, 0x0d + adj, pio_timings[pio].reg0d);
		set_indexed_reg(hwif, 0x13 + adj, pio_timings[pio].reg13);
	}
}

static u8 pdcnew_cable_detect(ide_hwif_t *hwif)
{
	if (get_indexed_reg(hwif, 0x0b) & 0x04)
		return ATA_CBL_PATA40;
	else
		return ATA_CBL_PATA80;
}

static void pdcnew_reset(ide_drive_t *drive)
{
	
	printk(KERN_WARNING "pdc202xx_new: %s channel reset.\n",
		drive->hwif->channel ? "Secondary" : "Primary");
}


static long read_counter(u32 dma_base)
{
	u32  pri_dma_base = dma_base, sec_dma_base = dma_base + 0x08;
	u8   cnt0, cnt1, cnt2, cnt3;
	long count = 0, last;
	int  retry = 3;

	do {
		last = count;

		
		outb(0x20, pri_dma_base + 0x01);
		cnt0 = inb(pri_dma_base + 0x03);
		outb(0x21, pri_dma_base + 0x01);
		cnt1 = inb(pri_dma_base + 0x03);
		outb(0x20, sec_dma_base + 0x01);
		cnt2 = inb(sec_dma_base + 0x03);
		outb(0x21, sec_dma_base + 0x01);
		cnt3 = inb(sec_dma_base + 0x03);

		count = (cnt3 << 23) | (cnt2 << 15) | (cnt1 << 8) | cnt0;

		
	} while (retry-- && (((last ^ count) & 0x3fff8000) || last < count));

	DBG("cnt0[%02X] cnt1[%02X] cnt2[%02X] cnt3[%02X]\n",
		  cnt0, cnt1, cnt2, cnt3);

	return count;
}


static long detect_pll_input_clock(unsigned long dma_base)
{
	struct timeval start_time, end_time;
	long start_count, end_count;
	long pll_input, usec_elapsed;
	u8 scr1;

	start_count = read_counter(dma_base);
	do_gettimeofday(&start_time);

	
	outb(0x01, dma_base + 0x01);
	scr1 = inb(dma_base + 0x03);
	DBG("scr1[%02X]\n", scr1);
	outb(scr1 | 0x40, dma_base + 0x03);

	
	mdelay(10);

	end_count = read_counter(dma_base);
	do_gettimeofday(&end_time);

	
	outb(0x01, dma_base + 0x01);
	scr1 = inb(dma_base + 0x03);
	DBG("scr1[%02X]\n", scr1);
	outb(scr1 & ~0x40, dma_base + 0x03);

	
	usec_elapsed = (end_time.tv_sec - start_time.tv_sec) * 1000000 +
		(end_time.tv_usec - start_time.tv_usec);
	pll_input = ((start_count - end_count) & 0x3fffffff) / 10 *
		(10000000 / usec_elapsed);

	DBG("start[%ld] end[%ld]\n", start_count, end_count);

	return pll_input;
}

#ifdef CONFIG_PPC_PMAC
static void apple_kiwi_init(struct pci_dev *pdev)
{
	struct device_node *np = pci_device_to_OF_node(pdev);
	u8 conf;

	if (np == NULL || !of_device_is_compatible(np, "kiwi-root"))
		return;

	if (pdev->revision >= 0x03) {
		
		pci_read_config_byte (pdev, 0x40, &conf);
		pci_write_config_byte(pdev, 0x40, (conf | 0x01));
	}
}
#endif 

static int init_chipset_pdcnew(struct pci_dev *dev)
{
	const char *name = DRV_NAME;
	unsigned long dma_base = pci_resource_start(dev, 4);
	unsigned long sec_dma_base = dma_base + 0x08;
	long pll_input, pll_output, ratio;
	int f, r;
	u8 pll_ctl0, pll_ctl1;

	if (dma_base == 0)
		return -EFAULT;

#ifdef CONFIG_PPC_PMAC
	apple_kiwi_init(dev);
#endif

	
	switch(max_dma_rate(dev)) {
		case 4: 
			pll_output = 133333333;
			break;
		case 3: 
		default:
			pll_output = 100000000;
			break;
	}

	
	pll_input = detect_pll_input_clock(dma_base);
	printk(KERN_INFO "%s %s: PLL input clock is %ld kHz\n",
		name, pci_name(dev), pll_input / 1000);

	
	if (unlikely(pll_input < 5000000L || pll_input > 70000000L)) {
		printk(KERN_ERR "%s %s: Bad PLL input clock %ld Hz, giving up!"
			"\n", name, pci_name(dev), pll_input);
		goto out;
	}

#ifdef DEBUG
	DBG("pll_output is %ld Hz\n", pll_output);

	
	outb(0x02, sec_dma_base + 0x01);
	pll_ctl0 = inb(sec_dma_base + 0x03);
	outb(0x03, sec_dma_base + 0x01);
	pll_ctl1 = inb(sec_dma_base + 0x03);

	DBG("pll_ctl[%02X][%02X]\n", pll_ctl0, pll_ctl1);
#endif

	
	ratio = pll_output / (pll_input / 1000);
	if (ratio < 8600L) { 
		
		r = 0x0d;
	} else if (ratio < 12900L) { 
		
		r = 0x08;
	} else if (ratio < 16100L) { 
		
		r = 0x06;
	} else if (ratio < 64000L) { 
		r = 0x00;
	} else {
		
		printk(KERN_ERR "%s %s: Bad ratio %ld, giving up!\n",
			name, pci_name(dev), ratio);
		goto out;
	}

	f = (ratio * (r + 2)) / 1000 - 2;

	DBG("F[%d] R[%d] ratio*1000[%ld]\n", f, r, ratio);

	if (unlikely(f < 0 || f > 127)) {
		
		printk(KERN_ERR "%s %s: F[%d] invalid!\n",
			name, pci_name(dev), f);
		goto out;
	}

	pll_ctl0 = (u8) f;
	pll_ctl1 = (u8) r;

	DBG("Writing pll_ctl[%02X][%02X]\n", pll_ctl0, pll_ctl1);

	outb(0x02,     sec_dma_base + 0x01);
	outb(pll_ctl0, sec_dma_base + 0x03);
	outb(0x03,     sec_dma_base + 0x01);
	outb(pll_ctl1, sec_dma_base + 0x03);

	
	mdelay(30);

#ifdef DEBUG
	
	outb(0x02, sec_dma_base + 0x01);
	pll_ctl0 = inb(sec_dma_base + 0x03);
	outb(0x03, sec_dma_base + 0x01);
	pll_ctl1 = inb(sec_dma_base + 0x03);

	DBG("pll_ctl[%02X][%02X]\n", pll_ctl0, pll_ctl1);
#endif

 out:
	return 0;
}

static struct pci_dev * __devinit pdc20270_get_dev2(struct pci_dev *dev)
{
	struct pci_dev *dev2;

	dev2 = pci_get_slot(dev->bus, PCI_DEVFN(PCI_SLOT(dev->devfn) + 1,
						PCI_FUNC(dev->devfn)));

	if (dev2 &&
	    dev2->vendor == dev->vendor &&
	    dev2->device == dev->device) {

		if (dev2->irq != dev->irq) {
			dev2->irq = dev->irq;
			printk(KERN_INFO DRV_NAME " %s: PCI config space "
				"interrupt fixed\n", pci_name(dev));
		}

		return dev2;
	}

	return NULL;
}

static const struct ide_port_ops pdcnew_port_ops = {
	.set_pio_mode		= pdcnew_set_pio_mode,
	.set_dma_mode		= pdcnew_set_dma_mode,
	.resetproc		= pdcnew_reset,
	.cable_detect		= pdcnew_cable_detect,
};

#define DECLARE_PDCNEW_DEV(udma) \
	{ \
		.name		= DRV_NAME, \
		.init_chipset	= init_chipset_pdcnew, \
		.port_ops	= &pdcnew_port_ops, \
		.host_flags	= IDE_HFLAG_POST_SET_MODE | \
				  IDE_HFLAG_ERROR_STOPS_FIFO | \
				  IDE_HFLAG_OFF_BOARD, \
		.pio_mask	= ATA_PIO4, \
		.mwdma_mask	= ATA_MWDMA2, \
		.udma_mask	= udma, \
	}

static const struct ide_port_info pdcnew_chipsets[] __devinitdata = {
			DECLARE_PDCNEW_DEV(ATA_UDMA5),
		DECLARE_PDCNEW_DEV(ATA_UDMA6),
};


 
static int __devinit pdc202new_init_one(struct pci_dev *dev, const struct pci_device_id *id)
{
	const struct ide_port_info *d = &pdcnew_chipsets[id->driver_data];
	struct pci_dev *bridge = dev->bus->self;

	if (dev->device == PCI_DEVICE_ID_PROMISE_20270 && bridge &&
	    bridge->vendor == PCI_VENDOR_ID_DEC &&
	    bridge->device == PCI_DEVICE_ID_DEC_21150) {
		struct pci_dev *dev2;

		if (PCI_SLOT(dev->devfn) & 2)
			return -ENODEV;

		dev2 = pdc20270_get_dev2(dev);

		if (dev2) {
			int ret = ide_pci_init_two(dev, dev2, d, NULL);
			if (ret < 0)
				pci_dev_put(dev2);
			return ret;
		}
	}

	if (dev->device == PCI_DEVICE_ID_PROMISE_20276 && bridge &&
	    bridge->vendor == PCI_VENDOR_ID_INTEL &&
	    (bridge->device == PCI_DEVICE_ID_INTEL_I960 ||
	     bridge->device == PCI_DEVICE_ID_INTEL_I960RM)) {
		printk(KERN_INFO DRV_NAME " %s: attached to I2O RAID controller,"
			" skipping\n", pci_name(dev));
		return -ENODEV;
	}

	return ide_pci_init_one(dev, d, NULL);
}

static void __devexit pdc202new_remove(struct pci_dev *dev)
{
	struct ide_host *host = pci_get_drvdata(dev);
	struct pci_dev *dev2 = host->dev[1] ? to_pci_dev(host->dev[1]) : NULL;

	ide_pci_remove(dev);
	pci_dev_put(dev2);
}

static const struct pci_device_id pdc202new_pci_tbl[] = {
	{ PCI_VDEVICE(PROMISE, PCI_DEVICE_ID_PROMISE_20268), 0 },
	{ PCI_VDEVICE(PROMISE, PCI_DEVICE_ID_PROMISE_20269), 1 },
	{ PCI_VDEVICE(PROMISE, PCI_DEVICE_ID_PROMISE_20270), 0 },
	{ PCI_VDEVICE(PROMISE, PCI_DEVICE_ID_PROMISE_20271), 1 },
	{ PCI_VDEVICE(PROMISE, PCI_DEVICE_ID_PROMISE_20275), 1 },
	{ PCI_VDEVICE(PROMISE, PCI_DEVICE_ID_PROMISE_20276), 1 },
	{ PCI_VDEVICE(PROMISE, PCI_DEVICE_ID_PROMISE_20277), 1 },
	{ 0, },
};
MODULE_DEVICE_TABLE(pci, pdc202new_pci_tbl);

static struct pci_driver pdc202new_pci_driver = {
	.name		= "Promise_IDE",
	.id_table	= pdc202new_pci_tbl,
	.probe		= pdc202new_init_one,
	.remove		= __devexit_p(pdc202new_remove),
	.suspend	= ide_pci_suspend,
	.resume		= ide_pci_resume,
};

static int __init pdc202new_ide_init(void)
{
	return ide_pci_register_driver(&pdc202new_pci_driver);
}

static void __exit pdc202new_ide_exit(void)
{
	pci_unregister_driver(&pdc202new_pci_driver);
}

module_init(pdc202new_ide_init);
module_exit(pdc202new_ide_exit);

MODULE_AUTHOR("Andre Hedrick, Frank Tiernan");
MODULE_DESCRIPTION("PCI driver module for Promise PDC20268 and higher");
MODULE_LICENSE("GPL");
