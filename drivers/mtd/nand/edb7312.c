

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>
#include <mach/hardware.h>	
#include <asm/sizes.h>
#include <asm/hardware/clps7111.h>


static struct mtd_info *ep7312_mtd = NULL;


#define EP7312_FIO_PBASE 0x10000000	
#define EP7312_PXDR	0x0001	
#define EP7312_PXDDR	0x0041	



static unsigned long ep7312_fio_pbase = EP7312_FIO_PBASE;
static void __iomem *ep7312_pxdr = (void __iomem *)EP7312_PXDR;
static void __iomem *ep7312_pxddr = (void __iomem *)EP7312_PXDDR;

#ifdef CONFIG_MTD_PARTITIONS

static struct mtd_partition partition_info[] = {
	{.name = "EP7312 Nand Flash",
	 .offset = 0,
	 .size = 8 * 1024 * 1024}
};

#define NUM_PARTITIONS 1

#endif


static void ep7312_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *chip = mtd->priv;

	if (ctrl & NAND_CTRL_CHANGE) {
		unsigned char bits = 0x80;

		bits |= (ctrl & (NAND_CLE | NAND_ALE)) << 3;
		bits |= (ctrl & NAND_NCE) ? 0x00 : 0x40;

		clps_writeb((clps_readb(ep7312_pxdr)  & 0xF0) | bits,
			    ep7312_pxdr);
	}
	if (cmd != NAND_CMD_NONE)
		writeb(cmd, chip->IO_ADDR_W);
}


static int ep7312_device_ready(struct mtd_info *mtd)
{
	return 1;
}

#ifdef CONFIG_MTD_PARTITIONS
const char *part_probes[] = { "cmdlinepart", NULL };
#endif


static int __init ep7312_init(void)
{
	struct nand_chip *this;
	const char *part_type = 0;
	int mtd_parts_nb = 0;
	struct mtd_partition *mtd_parts = 0;
	void __iomem *ep7312_fio_base;

	
	ep7312_mtd = kmalloc(sizeof(struct mtd_info) + sizeof(struct nand_chip), GFP_KERNEL);
	if (!ep7312_mtd) {
		printk("Unable to allocate EDB7312 NAND MTD device structure.\n");
		return -ENOMEM;
	}

	
	ep7312_fio_base = ioremap(ep7312_fio_pbase, SZ_1K);
	if (!ep7312_fio_base) {
		printk("ioremap EDB7312 NAND flash failed\n");
		kfree(ep7312_mtd);
		return -EIO;
	}

	
	this = (struct nand_chip *)(&ep7312_mtd[1]);

	
	memset(ep7312_mtd, 0, sizeof(struct mtd_info));
	memset(this, 0, sizeof(struct nand_chip));

	
	ep7312_mtd->priv = this;
	ep7312_mtd->owner = THIS_MODULE;

	
	clps_writeb(0xf0, ep7312_pxddr);

	
	this->IO_ADDR_R = ep7312_fio_base;
	this->IO_ADDR_W = ep7312_fio_base;
	this->cmd_ctrl = ep7312_hwcontrol;
	this->dev_ready = ep7312_device_ready;
	
	this->chip_delay = 15;

	
	if (nand_scan(ep7312_mtd, 1)) {
		iounmap((void *)ep7312_fio_base);
		kfree(ep7312_mtd);
		return -ENXIO;
	}
#ifdef CONFIG_MTD_PARTITIONS
	ep7312_mtd->name = "edb7312-nand";
	mtd_parts_nb = parse_mtd_partitions(ep7312_mtd, part_probes, &mtd_parts, 0);
	if (mtd_parts_nb > 0)
		part_type = "command line";
	else
		mtd_parts_nb = 0;
#endif
	if (mtd_parts_nb == 0) {
		mtd_parts = partition_info;
		mtd_parts_nb = NUM_PARTITIONS;
		part_type = "static";
	}

	
	printk(KERN_NOTICE "Using %s partition definition\n", part_type);
	add_mtd_partitions(ep7312_mtd, mtd_parts, mtd_parts_nb);

	
	return 0;
}

module_init(ep7312_init);


static void __exit ep7312_cleanup(void)
{
	struct nand_chip *this = (struct nand_chip *)&ep7312_mtd[1];

	
	nand_release(ap7312_mtd);

	
	iounmap(this->IO_ADDR_R);

	
	kfree(ep7312_mtd);
}

module_exit(ep7312_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marius Groeger <mag@sysgo.de>");
MODULE_DESCRIPTION("MTD map driver for Cogent EDB7312 board");
