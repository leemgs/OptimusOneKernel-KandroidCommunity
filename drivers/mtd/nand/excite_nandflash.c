

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>
#include <asm/rm9k-ocd.h>

#include <excite_nandflash.h>

#define EXCITE_NANDFLASH_VERSION "0.1"


#define EXCITE_NANDFLASH_DATA_BYTE   0x00
#define EXCITE_NANDFLASH_STATUS_BYTE 0x0c
#define EXCITE_NANDFLASH_ADDR_BYTE   0x10
#define EXCITE_NANDFLASH_CMD_BYTE    0x14


static const char module_id[] = "excite_nandflash";


static const struct mtd_partition partition_info[] = {
	{
		.name = "eXcite RootFS",
		.offset = 0,
		.size = MTDPART_SIZ_FULL
	}
};

static inline const struct resource *
excite_nand_get_resource(struct platform_device *d, unsigned long flags,
			 const char *basename)
{
	char buf[80];

	if (snprintf(buf, sizeof buf, "%s_%u", basename, d->id) >= sizeof buf)
		return NULL;
	return platform_get_resource_byname(d, flags, buf);
}

static inline void __iomem *
excite_nand_map_regs(struct platform_device *d, const char *basename)
{
	void *result = NULL;
	const struct resource *const r =
	    excite_nand_get_resource(d, IORESOURCE_MEM, basename);

	if (r)
		result = ioremap_nocache(r->start, r->end + 1 - r->start);
	return result;
}


struct excite_nand_drvdata {
	struct mtd_info board_mtd;
	struct nand_chip board_chip;
	void __iomem *regs;
	void __iomem *tgt;
};


static void excite_nand_control(struct mtd_info *mtd, int cmd,
				       unsigned int ctrl)
{
	struct excite_nand_drvdata * const d =
	    container_of(mtd, struct excite_nand_drvdata, board_mtd);

	switch (ctrl) {
	case NAND_CTRL_CHANGE | NAND_CTRL_CLE:
		d->tgt = d->regs + EXCITE_NANDFLASH_CMD_BYTE;
		break;
	case NAND_CTRL_CHANGE | NAND_CTRL_ALE:
		d->tgt = d->regs + EXCITE_NANDFLASH_ADDR_BYTE;
		break;
	case NAND_CTRL_CHANGE | NAND_NCE:
		d->tgt = d->regs + EXCITE_NANDFLASH_DATA_BYTE;
		break;
	}

	if (cmd != NAND_CMD_NONE)
		__raw_writeb(cmd, d->tgt);
}


static int excite_nand_devready(struct mtd_info *mtd)
{
	struct excite_nand_drvdata * const drvdata =
	    container_of(mtd, struct excite_nand_drvdata, board_mtd);

	return __raw_readb(drvdata->regs + EXCITE_NANDFLASH_STATUS_BYTE);
}


static int __exit excite_nand_remove(struct platform_device *dev)
{
	struct excite_nand_drvdata * const this = platform_get_drvdata(dev);

	platform_set_drvdata(dev, NULL);

	if (unlikely(!this)) {
		printk(KERN_ERR "%s: called %s without private data!!",
		       module_id, __func__);
		return -EINVAL;
	}

	
	nand_release(&this->board_mtd);

	
	iounmap(this->regs);
	kfree(this);

	DEBUG(MTD_DEBUG_LEVEL1, "%s: removed\n", module_id);
	return 0;
}


static int __init excite_nand_probe(struct platform_device *pdev)
{
	struct excite_nand_drvdata *drvdata;	
	struct nand_chip *board_chip;	
	struct mtd_info *board_mtd;	
	int scan_res;

	drvdata = kzalloc(sizeof(*drvdata), GFP_KERNEL);
	if (unlikely(!drvdata)) {
		printk(KERN_ERR "%s: no memory for drvdata\n",
		       module_id);
		return -ENOMEM;
	}

	
	platform_set_drvdata(pdev, drvdata);

	
	drvdata->regs =
		excite_nand_map_regs(pdev, EXCITE_NANDFLASH_RESOURCE_REGS);

	if (unlikely(!drvdata->regs)) {
		printk(KERN_ERR "%s: cannot reserve register region\n",
		       module_id);
		kfree(drvdata);
		return -ENXIO;
	}
	
	drvdata->tgt = drvdata->regs + EXCITE_NANDFLASH_DATA_BYTE;

	
	board_chip = &drvdata->board_chip;
	board_chip->IO_ADDR_R = board_chip->IO_ADDR_W =
		drvdata->regs + EXCITE_NANDFLASH_DATA_BYTE;
	board_chip->cmd_ctrl = excite_nand_control;
	board_chip->dev_ready = excite_nand_devready;
	board_chip->chip_delay = 25;
	board_chip->ecc.mode = NAND_ECC_SOFT;

	
	board_mtd = &drvdata->board_mtd;
	board_mtd->priv = board_chip;

	DEBUG(MTD_DEBUG_LEVEL2, "%s: device scan\n", module_id);
	scan_res = nand_scan(&drvdata->board_mtd, 1);

	if (likely(!scan_res)) {
		DEBUG(MTD_DEBUG_LEVEL2, "%s: register partitions\n", module_id);
		add_mtd_partitions(&drvdata->board_mtd, partition_info,
				   ARRAY_SIZE(partition_info));
	} else {
		iounmap(drvdata->regs);
		kfree(drvdata);
		printk(KERN_ERR "%s: device scan failed\n", module_id);
		return -EIO;
	}
	return 0;
}

static struct platform_driver excite_nand_driver = {
	.driver = {
		.name = "excite_nand",
		.owner		= THIS_MODULE,
	},
	.probe = excite_nand_probe,
	.remove = __devexit_p(excite_nand_remove)
};

static int __init excite_nand_init(void)
{
	pr_info("Basler eXcite nand flash driver Version "
		EXCITE_NANDFLASH_VERSION "\n");
	return platform_driver_register(&excite_nand_driver);
}

static void __exit excite_nand_exit(void)
{
	platform_driver_unregister(&excite_nand_driver);
}

module_init(excite_nand_init);
module_exit(excite_nand_exit);

MODULE_AUTHOR("Thomas Koeller <thomas.koeller@baslerweb.com>");
MODULE_DESCRIPTION("Basler eXcite NAND-Flash driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(EXCITE_NANDFLASH_VERSION)
