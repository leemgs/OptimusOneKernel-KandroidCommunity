

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>
#include <platforms/PPChameleonEVB.h>

#undef USE_READY_BUSY_PIN
#define USE_READY_BUSY_PIN

#define NAND_BIG_DELAY_US		25
#define NAND_SMALL_DELAY_US		10


#define SZ_4M                           0x00400000
#define NAND_SMALL_SIZE                 0x02000000
#define NAND_MTD_NAME		"ppchameleon-nand"
#define NAND_EVB_MTD_NAME	"ppchameleonevb-nand"


#define NAND_nCE_GPIO_PIN 		(0x80000000 >> 1)
#define NAND_CLE_GPIO_PIN 		(0x80000000 >> 2)
#define NAND_ALE_GPIO_PIN 		(0x80000000 >> 3)
#define NAND_RB_GPIO_PIN 		(0x80000000 >> 4)

#define NAND_EVB_nCE_GPIO_PIN 	(0x80000000 >> 14)
#define NAND_EVB_CLE_GPIO_PIN 	(0x80000000 >> 15)
#define NAND_EVB_ALE_GPIO_PIN 	(0x80000000 >> 16)
#define NAND_EVB_RB_GPIO_PIN 	(0x80000000 >> 31)


static struct mtd_info *ppchameleon_mtd = NULL;
static struct mtd_info *ppchameleonevb_mtd = NULL;


static unsigned long ppchameleon_fio_pbase = CFG_NAND0_PADDR;
static unsigned long ppchameleonevb_fio_pbase = CFG_NAND1_PADDR;

#ifdef MODULE
module_param(ppchameleon_fio_pbase, ulong, 0);
module_param(ppchameleonevb_fio_pbase, ulong, 0);
#else
__setup("ppchameleon_fio_pbase=", ppchameleon_fio_pbase);
__setup("ppchameleonevb_fio_pbase=", ppchameleonevb_fio_pbase);
#endif

#ifdef CONFIG_MTD_PARTITIONS

static struct mtd_partition partition_info_hi[] = {
      { .name = "PPChameleon HI Nand Flash",
	.offset = 0,
	.size = 128 * 1024 * 1024
      }
};

static struct mtd_partition partition_info_me[] = {
      { .name = "PPChameleon ME Nand Flash",
	.offset = 0,
	.size = 32 * 1024 * 1024
      }
};

static struct mtd_partition partition_info_evb[] = {
      { .name = "PPChameleonEVB Nand Flash",
	.offset = 0,
	.size = 32 * 1024 * 1024
      }
};

#define NUM_PARTITIONS 1

extern int parse_cmdline_partitions(struct mtd_info *master, struct mtd_partition **pparts, const char *mtd_id);
#endif


static void ppchameleon_hwcontrol(struct mtd_info *mtdinfo, int cmd,
				  unsigned int ctrl)
{
	struct nand_chip *chip = mtd->priv;

	if (ctrl & NAND_CTRL_CHANGE) {
#error Missing headerfiles. No way to fix this. -tglx
		switch (cmd) {
		case NAND_CTL_SETCLE:
			MACRO_NAND_CTL_SETCLE((unsigned long)CFG_NAND0_PADDR);
			break;
		case NAND_CTL_CLRCLE:
			MACRO_NAND_CTL_CLRCLE((unsigned long)CFG_NAND0_PADDR);
			break;
		case NAND_CTL_SETALE:
			MACRO_NAND_CTL_SETALE((unsigned long)CFG_NAND0_PADDR);
			break;
		case NAND_CTL_CLRALE:
			MACRO_NAND_CTL_CLRALE((unsigned long)CFG_NAND0_PADDR);
			break;
		case NAND_CTL_SETNCE:
			MACRO_NAND_ENABLE_CE((unsigned long)CFG_NAND0_PADDR);
			break;
		case NAND_CTL_CLRNCE:
			MACRO_NAND_DISABLE_CE((unsigned long)CFG_NAND0_PADDR);
			break;
		}
	}
	if (cmd != NAND_CMD_NONE)
		writeb(cmd, chip->IO_ADDR_W);
}

static void ppchameleonevb_hwcontrol(struct mtd_info *mtdinfo, int cmd,
				     unsigned int ctrl)
{
	struct nand_chip *chip = mtd->priv;

	if (ctrl & NAND_CTRL_CHANGE) {
#error Missing headerfiles. No way to fix this. -tglx
		switch (cmd) {
		case NAND_CTL_SETCLE:
			MACRO_NAND_CTL_SETCLE((unsigned long)CFG_NAND1_PADDR);
			break;
		case NAND_CTL_CLRCLE:
			MACRO_NAND_CTL_CLRCLE((unsigned long)CFG_NAND1_PADDR);
			break;
		case NAND_CTL_SETALE:
			MACRO_NAND_CTL_SETALE((unsigned long)CFG_NAND1_PADDR);
			break;
		case NAND_CTL_CLRALE:
			MACRO_NAND_CTL_CLRALE((unsigned long)CFG_NAND1_PADDR);
			break;
		case NAND_CTL_SETNCE:
			MACRO_NAND_ENABLE_CE((unsigned long)CFG_NAND1_PADDR);
			break;
		case NAND_CTL_CLRNCE:
			MACRO_NAND_DISABLE_CE((unsigned long)CFG_NAND1_PADDR);
			break;
		}
	}
	if (cmd != NAND_CMD_NONE)
		writeb(cmd, chip->IO_ADDR_W);
}

#ifdef USE_READY_BUSY_PIN

static int ppchameleon_device_ready(struct mtd_info *minfo)
{
	if (in_be32((volatile unsigned *)GPIO0_IR) & NAND_RB_GPIO_PIN)
		return 1;
	return 0;
}

static int ppchameleonevb_device_ready(struct mtd_info *minfo)
{
	if (in_be32((volatile unsigned *)GPIO0_IR) & NAND_EVB_RB_GPIO_PIN)
		return 1;
	return 0;
}
#endif

#ifdef CONFIG_MTD_PARTITIONS
const char *part_probes[] = { "cmdlinepart", NULL };
const char *part_probes_evb[] = { "cmdlinepart", NULL };
#endif


static int __init ppchameleonevb_init(void)
{
	struct nand_chip *this;
	const char *part_type = 0;
	int mtd_parts_nb = 0;
	struct mtd_partition *mtd_parts = 0;
	void __iomem *ppchameleon_fio_base;
	void __iomem *ppchameleonevb_fio_base;

	
	
	ppchameleon_mtd = kmalloc(sizeof(struct mtd_info) + sizeof(struct nand_chip), GFP_KERNEL);
	if (!ppchameleon_mtd) {
		printk("Unable to allocate PPChameleon NAND MTD device structure.\n");
		return -ENOMEM;
	}

	
	ppchameleon_fio_base = ioremap(ppchameleon_fio_pbase, SZ_4M);
	if (!ppchameleon_fio_base) {
		printk("ioremap PPChameleon NAND flash failed\n");
		kfree(ppchameleon_mtd);
		return -EIO;
	}

	
	this = (struct nand_chip *)(&ppchameleon_mtd[1]);

	
	memset(ppchameleon_mtd, 0, sizeof(struct mtd_info));
	memset(this, 0, sizeof(struct nand_chip));

	
	ppchameleon_mtd->priv = this;
	ppchameleon_mtd->owner = THIS_MODULE;

	
	
	
	
	out_be32((volatile unsigned *)GPIO0_OSRH, in_be32((volatile unsigned *)GPIO0_OSRH) & 0xC0FFFFFF);
	
	out_be32((volatile unsigned *)GPIO0_TSRH, in_be32((volatile unsigned *)GPIO0_TSRH) & 0xC0FFFFFF);
	
	out_be32((volatile unsigned *)GPIO0_TCR,
		 in_be32((volatile unsigned *)GPIO0_TCR) | NAND_nCE_GPIO_PIN | NAND_CLE_GPIO_PIN | NAND_ALE_GPIO_PIN);
#ifdef USE_READY_BUSY_PIN
	
	out_be32((volatile unsigned *)GPIO0_TSRH, in_be32((volatile unsigned *)GPIO0_TSRH) & 0xFF3FFFFF);
	
	out_be32((volatile unsigned *)GPIO0_TCR, in_be32((volatile unsigned *)GPIO0_TCR) & (~NAND_RB_GPIO_PIN));
	
	out_be32((volatile unsigned *)GPIO0_ISR1H,
		 (in_be32((volatile unsigned *)GPIO0_ISR1H) & 0xFF3FFFFF) | 0x00400000);
#endif

	
	this->IO_ADDR_R = ppchameleon_fio_base;
	this->IO_ADDR_W = ppchameleon_fio_base;
	this->cmd_ctrl = ppchameleon_hwcontrol;
#ifdef USE_READY_BUSY_PIN
	this->dev_ready = ppchameleon_device_ready;
#endif
	this->chip_delay = NAND_BIG_DELAY_US;
	
	this->ecc.mode = NAND_ECC_SOFT;

	
	if (nand_scan(ppchameleon_mtd, 1)) {
		iounmap((void *)ppchameleon_fio_base);
		ppchameleon_fio_base = NULL;
		kfree(ppchameleon_mtd);
		goto nand_evb_init;
	}
#ifndef USE_READY_BUSY_PIN
	
	if (ppchameleon_mtd->size == NAND_SMALL_SIZE)
		this->chip_delay = NAND_SMALL_DELAY_US;
#endif

#ifdef CONFIG_MTD_PARTITIONS
	ppchameleon_mtd->name = "ppchameleon-nand";
	mtd_parts_nb = parse_mtd_partitions(ppchameleon_mtd, part_probes, &mtd_parts, 0);
	if (mtd_parts_nb > 0)
		part_type = "command line";
	else
		mtd_parts_nb = 0;
#endif
	if (mtd_parts_nb == 0) {
		if (ppchameleon_mtd->size == NAND_SMALL_SIZE)
			mtd_parts = partition_info_me;
		else
			mtd_parts = partition_info_hi;
		mtd_parts_nb = NUM_PARTITIONS;
		part_type = "static";
	}

	
	printk(KERN_NOTICE "Using %s partition definition\n", part_type);
	add_mtd_partitions(ppchameleon_mtd, mtd_parts, mtd_parts_nb);

 nand_evb_init:
	
	
	ppchameleonevb_mtd = kmalloc(sizeof(struct mtd_info) + sizeof(struct nand_chip), GFP_KERNEL);
	if (!ppchameleonevb_mtd) {
		printk("Unable to allocate PPChameleonEVB NAND MTD device structure.\n");
		if (ppchameleon_fio_base)
			iounmap(ppchameleon_fio_base);
		return -ENOMEM;
	}

	
	ppchameleonevb_fio_base = ioremap(ppchameleonevb_fio_pbase, SZ_4M);
	if (!ppchameleonevb_fio_base) {
		printk("ioremap PPChameleonEVB NAND flash failed\n");
		kfree(ppchameleonevb_mtd);
		if (ppchameleon_fio_base)
			iounmap(ppchameleon_fio_base);
		return -EIO;
	}

	
	this = (struct nand_chip *)(&ppchameleonevb_mtd[1]);

	
	memset(ppchameleonevb_mtd, 0, sizeof(struct mtd_info));
	memset(this, 0, sizeof(struct nand_chip));

	
	ppchameleonevb_mtd->priv = this;

	
	
	
	
	out_be32((volatile unsigned *)GPIO0_OSRH, in_be32((volatile unsigned *)GPIO0_OSRH) & 0xFFFFFFF0);
	out_be32((volatile unsigned *)GPIO0_OSRL, in_be32((volatile unsigned *)GPIO0_OSRL) & 0x3FFFFFFF);
	
	out_be32((volatile unsigned *)GPIO0_TSRH, in_be32((volatile unsigned *)GPIO0_TSRH) & 0xFFFFFFF0);
	out_be32((volatile unsigned *)GPIO0_TSRL, in_be32((volatile unsigned *)GPIO0_TSRL) & 0x3FFFFFFF);
	
	out_be32((volatile unsigned *)GPIO0_TCR, in_be32((volatile unsigned *)GPIO0_TCR) | NAND_EVB_nCE_GPIO_PIN |
		 NAND_EVB_CLE_GPIO_PIN | NAND_EVB_ALE_GPIO_PIN);
#ifdef USE_READY_BUSY_PIN
	
	out_be32((volatile unsigned *)GPIO0_TSRL, in_be32((volatile unsigned *)GPIO0_TSRL) & 0xFFFFFFFC);
	
	out_be32((volatile unsigned *)GPIO0_TCR, in_be32((volatile unsigned *)GPIO0_TCR) & (~NAND_EVB_RB_GPIO_PIN));
	
	out_be32((volatile unsigned *)GPIO0_ISR1L,
		 (in_be32((volatile unsigned *)GPIO0_ISR1L) & 0xFFFFFFFC) | 0x00000001);
#endif

	
	this->IO_ADDR_R = ppchameleonevb_fio_base;
	this->IO_ADDR_W = ppchameleonevb_fio_base;
	this->cmd_ctrl = ppchameleonevb_hwcontrol;
#ifdef USE_READY_BUSY_PIN
	this->dev_ready = ppchameleonevb_device_ready;
#endif
	this->chip_delay = NAND_SMALL_DELAY_US;

	
	this->ecc.mode = NAND_ECC_SOFT;

	
	if (nand_scan(ppchameleonevb_mtd, 1)) {
		iounmap((void *)ppchameleonevb_fio_base);
		kfree(ppchameleonevb_mtd);
		if (ppchameleon_fio_base)
			iounmap(ppchameleon_fio_base);
		return -ENXIO;
	}
#ifdef CONFIG_MTD_PARTITIONS
	ppchameleonevb_mtd->name = NAND_EVB_MTD_NAME;
	mtd_parts_nb = parse_mtd_partitions(ppchameleonevb_mtd, part_probes_evb, &mtd_parts, 0);
	if (mtd_parts_nb > 0)
		part_type = "command line";
	else
		mtd_parts_nb = 0;
#endif
	if (mtd_parts_nb == 0) {
		mtd_parts = partition_info_evb;
		mtd_parts_nb = NUM_PARTITIONS;
		part_type = "static";
	}

	
	printk(KERN_NOTICE "Using %s partition definition\n", part_type);
	add_mtd_partitions(ppchameleonevb_mtd, mtd_parts, mtd_parts_nb);

	
	return 0;
}

module_init(ppchameleonevb_init);


static void __exit ppchameleonevb_cleanup(void)
{
	struct nand_chip *this;

	
	nand_release(ppchameleon_mtd);
	nand_release(ppchameleonevb_mtd);

	
	this = (struct nand_chip *) &ppchameleon_mtd[1];
	iounmap((void *) this->IO_ADDR_R);
	this = (struct nand_chip *) &ppchameleonevb_mtd[1];
	iounmap((void *) this->IO_ADDR_R);

	
	kfree (ppchameleon_mtd);
	kfree (ppchameleonevb_mtd);
}
module_exit(ppchameleonevb_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DAVE Srl <support-ppchameleon@dave-tech.it>");
MODULE_DESCRIPTION("MTD map driver for DAVE Srl PPChameleonEVB board");
