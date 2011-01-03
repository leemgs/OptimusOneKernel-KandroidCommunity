

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/of_platform.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>
#include <asm/fsl_lbc.h>

#define MAX_BANKS 8
#define ERR_BYTE 0xFF 
#define FCM_TIMEOUT_MSECS 500 

struct fsl_elbc_ctrl;



struct fsl_elbc_mtd {
	struct mtd_info mtd;
	struct nand_chip chip;
	struct fsl_elbc_ctrl *ctrl;

	struct device *dev;
	int bank;               
	u8 __iomem *vbase;      
	int page_size;          
	unsigned int fmr;       
};



struct fsl_elbc_ctrl {
	struct nand_hw_control controller;
	struct fsl_elbc_mtd *chips[MAX_BANKS];

	
	struct device *dev;
	struct fsl_lbc_regs __iomem *regs;
	int irq;
	wait_queue_head_t irq_wait;
	unsigned int irq_status; 
	u8 __iomem *addr;        
	unsigned int page;       
	unsigned int read_bytes; 
	unsigned int column;     
	unsigned int index;      
	unsigned int status;     
	unsigned int mdr;        
	unsigned int use_mdr;    
	unsigned int oob;        
	char *oob_poi;           
};




static struct nand_ecclayout fsl_elbc_oob_sp_eccm0 = {
	.eccbytes = 3,
	.eccpos = {6, 7, 8},
	.oobfree = { {0, 5}, {9, 7} },
};


static struct nand_ecclayout fsl_elbc_oob_sp_eccm1 = {
	.eccbytes = 3,
	.eccpos = {8, 9, 10},
	.oobfree = { {0, 5}, {6, 2}, {11, 5} },
};


static struct nand_ecclayout fsl_elbc_oob_lp_eccm0 = {
	.eccbytes = 12,
	.eccpos = {6, 7, 8, 22, 23, 24, 38, 39, 40, 54, 55, 56},
	.oobfree = { {1, 5}, {9, 13}, {25, 13}, {41, 13}, {57, 7} },
};


static struct nand_ecclayout fsl_elbc_oob_lp_eccm1 = {
	.eccbytes = 12,
	.eccpos = {8, 9, 10, 24, 25, 26, 40, 41, 42, 56, 57, 58},
	.oobfree = { {1, 7}, {11, 13}, {27, 13}, {43, 13}, {59, 5} },
};


static u8 scan_ff_pattern[] = { 0xff, };

static struct nand_bbt_descr largepage_memorybased = {
	.options = 0,
	.offs = 0,
	.len = 1,
	.pattern = scan_ff_pattern,
};


static u8 bbt_pattern[] = {'B', 'b', 't', '0' };
static u8 mirror_pattern[] = {'1', 't', 'b', 'B' };

static struct nand_bbt_descr bbt_main_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE |
		   NAND_BBT_2BIT | NAND_BBT_VERSION,
	.offs =	11,
	.len = 4,
	.veroffs = 15,
	.maxblocks = 4,
	.pattern = bbt_pattern,
};

static struct nand_bbt_descr bbt_mirror_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE |
		   NAND_BBT_2BIT | NAND_BBT_VERSION,
	.offs =	11,
	.len = 4,
	.veroffs = 15,
	.maxblocks = 4,
	.pattern = mirror_pattern,
};




static void set_addr(struct mtd_info *mtd, int column, int page_addr, int oob)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_elbc_mtd *priv = chip->priv;
	struct fsl_elbc_ctrl *ctrl = priv->ctrl;
	struct fsl_lbc_regs __iomem *lbc = ctrl->regs;
	int buf_num;

	ctrl->page = page_addr;

	out_be32(&lbc->fbar,
	         page_addr >> (chip->phys_erase_shift - chip->page_shift));

	if (priv->page_size) {
		out_be32(&lbc->fpar,
		         ((page_addr << FPAR_LP_PI_SHIFT) & FPAR_LP_PI) |
		         (oob ? FPAR_LP_MS : 0) | column);
		buf_num = (page_addr & 1) << 2;
	} else {
		out_be32(&lbc->fpar,
		         ((page_addr << FPAR_SP_PI_SHIFT) & FPAR_SP_PI) |
		         (oob ? FPAR_SP_MS : 0) | column);
		buf_num = page_addr & 7;
	}

	ctrl->addr = priv->vbase + buf_num * 1024;
	ctrl->index = column;

	
	if (oob)
		ctrl->index += priv->page_size ? 2048 : 512;

	dev_vdbg(ctrl->dev, "set_addr: bank=%d, ctrl->addr=0x%p (0x%p), "
	                    "index %x, pes %d ps %d\n",
	         buf_num, ctrl->addr, priv->vbase, ctrl->index,
	         chip->phys_erase_shift, chip->page_shift);
}


static int fsl_elbc_run_command(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_elbc_mtd *priv = chip->priv;
	struct fsl_elbc_ctrl *ctrl = priv->ctrl;
	struct fsl_lbc_regs __iomem *lbc = ctrl->regs;

	
	out_be32(&lbc->fmr, priv->fmr | 3);
	if (ctrl->use_mdr)
		out_be32(&lbc->mdr, ctrl->mdr);

	dev_vdbg(ctrl->dev,
	         "fsl_elbc_run_command: fmr=%08x fir=%08x fcr=%08x\n",
	         in_be32(&lbc->fmr), in_be32(&lbc->fir), in_be32(&lbc->fcr));
	dev_vdbg(ctrl->dev,
	         "fsl_elbc_run_command: fbar=%08x fpar=%08x "
	         "fbcr=%08x bank=%d\n",
	         in_be32(&lbc->fbar), in_be32(&lbc->fpar),
	         in_be32(&lbc->fbcr), priv->bank);

	ctrl->irq_status = 0;
	
	out_be32(&lbc->lsor, priv->bank);

	
	wait_event_timeout(ctrl->irq_wait, ctrl->irq_status,
	                   FCM_TIMEOUT_MSECS * HZ/1000);
	ctrl->status = ctrl->irq_status;

	
	if (ctrl->use_mdr)
		ctrl->mdr = in_be32(&lbc->mdr);

	ctrl->use_mdr = 0;

	dev_vdbg(ctrl->dev,
	         "fsl_elbc_run_command: stat=%08x mdr=%08x fmr=%08x\n",
	         ctrl->status, ctrl->mdr, in_be32(&lbc->fmr));

	
	return ctrl->status == LTESR_CC ? 0 : -EIO;
}

static void fsl_elbc_do_read(struct nand_chip *chip, int oob)
{
	struct fsl_elbc_mtd *priv = chip->priv;
	struct fsl_elbc_ctrl *ctrl = priv->ctrl;
	struct fsl_lbc_regs __iomem *lbc = ctrl->regs;

	if (priv->page_size) {
		out_be32(&lbc->fir,
		         (FIR_OP_CW0 << FIR_OP0_SHIFT) |
		         (FIR_OP_CA  << FIR_OP1_SHIFT) |
		         (FIR_OP_PA  << FIR_OP2_SHIFT) |
		         (FIR_OP_CW1 << FIR_OP3_SHIFT) |
		         (FIR_OP_RBW << FIR_OP4_SHIFT));

		out_be32(&lbc->fcr, (NAND_CMD_READ0 << FCR_CMD0_SHIFT) |
		                    (NAND_CMD_READSTART << FCR_CMD1_SHIFT));
	} else {
		out_be32(&lbc->fir,
		         (FIR_OP_CW0 << FIR_OP0_SHIFT) |
		         (FIR_OP_CA  << FIR_OP1_SHIFT) |
		         (FIR_OP_PA  << FIR_OP2_SHIFT) |
		         (FIR_OP_RBW << FIR_OP3_SHIFT));

		if (oob)
			out_be32(&lbc->fcr, NAND_CMD_READOOB << FCR_CMD0_SHIFT);
		else
			out_be32(&lbc->fcr, NAND_CMD_READ0 << FCR_CMD0_SHIFT);
	}
}


static void fsl_elbc_cmdfunc(struct mtd_info *mtd, unsigned int command,
                             int column, int page_addr)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_elbc_mtd *priv = chip->priv;
	struct fsl_elbc_ctrl *ctrl = priv->ctrl;
	struct fsl_lbc_regs __iomem *lbc = ctrl->regs;

	ctrl->use_mdr = 0;

	
	ctrl->read_bytes = 0;
	if (command != NAND_CMD_PAGEPROG)
		ctrl->index = 0;

	switch (command) {
	
	case NAND_CMD_READ1:
		column += 256;

	
	case NAND_CMD_READ0:
		dev_dbg(ctrl->dev,
		        "fsl_elbc_cmdfunc: NAND_CMD_READ0, page_addr:"
		        " 0x%x, column: 0x%x.\n", page_addr, column);


		out_be32(&lbc->fbcr, 0); 
		set_addr(mtd, 0, page_addr, 0);

		ctrl->read_bytes = mtd->writesize + mtd->oobsize;
		ctrl->index += column;

		fsl_elbc_do_read(chip, 0);
		fsl_elbc_run_command(mtd);
		return;

	
	case NAND_CMD_READOOB:
		dev_vdbg(ctrl->dev,
		         "fsl_elbc_cmdfunc: NAND_CMD_READOOB, page_addr:"
			 " 0x%x, column: 0x%x.\n", page_addr, column);

		out_be32(&lbc->fbcr, mtd->oobsize - column);
		set_addr(mtd, column, page_addr, 1);

		ctrl->read_bytes = mtd->writesize + mtd->oobsize;

		fsl_elbc_do_read(chip, 1);
		fsl_elbc_run_command(mtd);
		return;

	
	case NAND_CMD_READID:
		dev_vdbg(ctrl->dev, "fsl_elbc_cmdfunc: NAND_CMD_READID.\n");

		out_be32(&lbc->fir, (FIR_OP_CW0 << FIR_OP0_SHIFT) |
		                    (FIR_OP_UA  << FIR_OP1_SHIFT) |
		                    (FIR_OP_RBW << FIR_OP2_SHIFT));
		out_be32(&lbc->fcr, NAND_CMD_READID << FCR_CMD0_SHIFT);
		
		out_be32(&lbc->fbcr, 5);
		ctrl->read_bytes = 5;
		ctrl->use_mdr = 1;
		ctrl->mdr = 0;

		set_addr(mtd, 0, 0, 0);
		fsl_elbc_run_command(mtd);
		return;

	
	case NAND_CMD_ERASE1:
		dev_vdbg(ctrl->dev,
		         "fsl_elbc_cmdfunc: NAND_CMD_ERASE1, "
		         "page_addr: 0x%x.\n", page_addr);
		set_addr(mtd, 0, page_addr, 0);
		return;

	
	case NAND_CMD_ERASE2:
		dev_vdbg(ctrl->dev, "fsl_elbc_cmdfunc: NAND_CMD_ERASE2.\n");

		out_be32(&lbc->fir,
		         (FIR_OP_CW0 << FIR_OP0_SHIFT) |
		         (FIR_OP_PA  << FIR_OP1_SHIFT) |
		         (FIR_OP_CM1 << FIR_OP2_SHIFT));

		out_be32(&lbc->fcr,
		         (NAND_CMD_ERASE1 << FCR_CMD0_SHIFT) |
		         (NAND_CMD_ERASE2 << FCR_CMD1_SHIFT));

		out_be32(&lbc->fbcr, 0);
		ctrl->read_bytes = 0;

		fsl_elbc_run_command(mtd);
		return;

	
	case NAND_CMD_SEQIN: {
		__be32 fcr;
		dev_vdbg(ctrl->dev,
		         "fsl_elbc_cmdfunc: NAND_CMD_SEQIN/PAGE_PROG, "
		         "page_addr: 0x%x, column: 0x%x.\n",
		         page_addr, column);

		ctrl->column = column;
		ctrl->oob = 0;

		if (priv->page_size) {
			fcr = (NAND_CMD_SEQIN << FCR_CMD0_SHIFT) |
			      (NAND_CMD_PAGEPROG << FCR_CMD1_SHIFT);

			out_be32(&lbc->fir,
			         (FIR_OP_CW0 << FIR_OP0_SHIFT) |
			         (FIR_OP_CA  << FIR_OP1_SHIFT) |
			         (FIR_OP_PA  << FIR_OP2_SHIFT) |
			         (FIR_OP_WB  << FIR_OP3_SHIFT) |
			         (FIR_OP_CW1 << FIR_OP4_SHIFT));
		} else {
			fcr = (NAND_CMD_PAGEPROG << FCR_CMD1_SHIFT) |
			      (NAND_CMD_SEQIN << FCR_CMD2_SHIFT);

			out_be32(&lbc->fir,
			         (FIR_OP_CW0 << FIR_OP0_SHIFT) |
			         (FIR_OP_CM2 << FIR_OP1_SHIFT) |
			         (FIR_OP_CA  << FIR_OP2_SHIFT) |
			         (FIR_OP_PA  << FIR_OP3_SHIFT) |
			         (FIR_OP_WB  << FIR_OP4_SHIFT) |
			         (FIR_OP_CW1 << FIR_OP5_SHIFT));

			if (column >= mtd->writesize) {
				
				column -= mtd->writesize;
				fcr |= NAND_CMD_READOOB << FCR_CMD0_SHIFT;
				ctrl->oob = 1;
			} else if (column < 256) {
				
				fcr |= NAND_CMD_READ0 << FCR_CMD0_SHIFT;
			} else {
				
				fcr |= NAND_CMD_READ1 << FCR_CMD0_SHIFT;
			}
		}

		out_be32(&lbc->fcr, fcr);
		set_addr(mtd, column, page_addr, ctrl->oob);
		return;
	}

	
	case NAND_CMD_PAGEPROG: {
		int full_page;
		dev_vdbg(ctrl->dev,
		         "fsl_elbc_cmdfunc: NAND_CMD_PAGEPROG "
		         "writing %d bytes.\n", ctrl->index);

		
		if (ctrl->oob || ctrl->column != 0 ||
		    ctrl->index != mtd->writesize + mtd->oobsize) {
			out_be32(&lbc->fbcr, ctrl->index);
			full_page = 0;
		} else {
			out_be32(&lbc->fbcr, 0);
			full_page = 1;
		}

		fsl_elbc_run_command(mtd);

		
		if (full_page && ctrl->oob_poi) {
			out_be32(&lbc->fbcr, 3);
			set_addr(mtd, 6, page_addr, 1);

			ctrl->read_bytes = mtd->writesize + 9;

			fsl_elbc_do_read(chip, 1);
			fsl_elbc_run_command(mtd);

			memcpy_fromio(ctrl->oob_poi + 6,
			              &ctrl->addr[ctrl->index], 3);
			ctrl->index += 3;
		}

		ctrl->oob_poi = NULL;
		return;
	}

	
	
	case NAND_CMD_STATUS:
		out_be32(&lbc->fir,
		         (FIR_OP_CM0 << FIR_OP0_SHIFT) |
		         (FIR_OP_RBW << FIR_OP1_SHIFT));
		out_be32(&lbc->fcr, NAND_CMD_STATUS << FCR_CMD0_SHIFT);
		out_be32(&lbc->fbcr, 1);
		set_addr(mtd, 0, 0, 0);
		ctrl->read_bytes = 1;

		fsl_elbc_run_command(mtd);

		
		setbits8(ctrl->addr, NAND_STATUS_WP);
		return;

	
	case NAND_CMD_RESET:
		dev_dbg(ctrl->dev, "fsl_elbc_cmdfunc: NAND_CMD_RESET.\n");
		out_be32(&lbc->fir, FIR_OP_CM0 << FIR_OP0_SHIFT);
		out_be32(&lbc->fcr, NAND_CMD_RESET << FCR_CMD0_SHIFT);
		fsl_elbc_run_command(mtd);
		return;

	default:
		dev_err(ctrl->dev,
		        "fsl_elbc_cmdfunc: error, unsupported command 0x%x.\n",
		        command);
	}
}

static void fsl_elbc_select_chip(struct mtd_info *mtd, int chip)
{
	
}


static void fsl_elbc_write_buf(struct mtd_info *mtd, const u8 *buf, int len)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_elbc_mtd *priv = chip->priv;
	struct fsl_elbc_ctrl *ctrl = priv->ctrl;
	unsigned int bufsize = mtd->writesize + mtd->oobsize;

	if (len <= 0) {
		dev_err(ctrl->dev, "write_buf of %d bytes", len);
		ctrl->status = 0;
		return;
	}

	if ((unsigned int)len > bufsize - ctrl->index) {
		dev_err(ctrl->dev,
		        "write_buf beyond end of buffer "
		        "(%d requested, %u available)\n",
		        len, bufsize - ctrl->index);
		len = bufsize - ctrl->index;
	}

	memcpy_toio(&ctrl->addr[ctrl->index], buf, len);
	
	in_8(&ctrl->addr[ctrl->index] + len - 1);

	ctrl->index += len;
}


static u8 fsl_elbc_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_elbc_mtd *priv = chip->priv;
	struct fsl_elbc_ctrl *ctrl = priv->ctrl;

	
	if (ctrl->index < ctrl->read_bytes)
		return in_8(&ctrl->addr[ctrl->index++]);

	dev_err(ctrl->dev, "read_byte beyond end of buffer\n");
	return ERR_BYTE;
}


static void fsl_elbc_read_buf(struct mtd_info *mtd, u8 *buf, int len)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_elbc_mtd *priv = chip->priv;
	struct fsl_elbc_ctrl *ctrl = priv->ctrl;
	int avail;

	if (len < 0)
		return;

	avail = min((unsigned int)len, ctrl->read_bytes - ctrl->index);
	memcpy_fromio(buf, &ctrl->addr[ctrl->index], avail);
	ctrl->index += avail;

	if (len > avail)
		dev_err(ctrl->dev,
		        "read_buf beyond end of buffer "
		        "(%d requested, %d available)\n",
		        len, avail);
}


static int fsl_elbc_verify_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_elbc_mtd *priv = chip->priv;
	struct fsl_elbc_ctrl *ctrl = priv->ctrl;
	int i;

	if (len < 0) {
		dev_err(ctrl->dev, "write_buf of %d bytes", len);
		return -EINVAL;
	}

	if ((unsigned int)len > ctrl->read_bytes - ctrl->index) {
		dev_err(ctrl->dev,
		        "verify_buf beyond end of buffer "
		        "(%d requested, %u available)\n",
		        len, ctrl->read_bytes - ctrl->index);

		ctrl->index = ctrl->read_bytes;
		return -EINVAL;
	}

	for (i = 0; i < len; i++)
		if (in_8(&ctrl->addr[ctrl->index + i]) != buf[i])
			break;

	ctrl->index += len;
	return i == len && ctrl->status == LTESR_CC ? 0 : -EIO;
}


static int fsl_elbc_wait(struct mtd_info *mtd, struct nand_chip *chip)
{
	struct fsl_elbc_mtd *priv = chip->priv;
	struct fsl_elbc_ctrl *ctrl = priv->ctrl;
	struct fsl_lbc_regs __iomem *lbc = ctrl->regs;

	if (ctrl->status != LTESR_CC)
		return NAND_STATUS_FAIL;

	
	ctrl->use_mdr = 0;
	out_be32(&lbc->fir,
	         (FIR_OP_CW0 << FIR_OP0_SHIFT) |
	         (FIR_OP_RBW << FIR_OP1_SHIFT));
	out_be32(&lbc->fcr, NAND_CMD_STATUS << FCR_CMD0_SHIFT);
	out_be32(&lbc->fbcr, 1);
	set_addr(mtd, 0, 0, 0);
	ctrl->read_bytes = 1;

	fsl_elbc_run_command(mtd);

	if (ctrl->status != LTESR_CC)
		return NAND_STATUS_FAIL;

	
	setbits8(ctrl->addr, NAND_STATUS_WP);
	return fsl_elbc_read_byte(mtd);
}

static int fsl_elbc_chip_init_tail(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_elbc_mtd *priv = chip->priv;
	struct fsl_elbc_ctrl *ctrl = priv->ctrl;
	struct fsl_lbc_regs __iomem *lbc = ctrl->regs;
	unsigned int al;

	
	al = 0;
	if (chip->pagemask & 0xffff0000)
		al++;
	if (chip->pagemask & 0xff000000)
		al++;

	
	priv->fmr |= (12 << FMR_CWTO_SHIFT) |  
	             (al << FMR_AL_SHIFT);

	dev_dbg(ctrl->dev, "fsl_elbc_init: nand->numchips = %d\n",
	        chip->numchips);
	dev_dbg(ctrl->dev, "fsl_elbc_init: nand->chipsize = %lld\n",
	        chip->chipsize);
	dev_dbg(ctrl->dev, "fsl_elbc_init: nand->pagemask = %8x\n",
	        chip->pagemask);
	dev_dbg(ctrl->dev, "fsl_elbc_init: nand->chip_delay = %d\n",
	        chip->chip_delay);
	dev_dbg(ctrl->dev, "fsl_elbc_init: nand->badblockpos = %d\n",
	        chip->badblockpos);
	dev_dbg(ctrl->dev, "fsl_elbc_init: nand->chip_shift = %d\n",
	        chip->chip_shift);
	dev_dbg(ctrl->dev, "fsl_elbc_init: nand->page_shift = %d\n",
	        chip->page_shift);
	dev_dbg(ctrl->dev, "fsl_elbc_init: nand->phys_erase_shift = %d\n",
	        chip->phys_erase_shift);
	dev_dbg(ctrl->dev, "fsl_elbc_init: nand->ecclayout = %p\n",
	        chip->ecclayout);
	dev_dbg(ctrl->dev, "fsl_elbc_init: nand->ecc.mode = %d\n",
	        chip->ecc.mode);
	dev_dbg(ctrl->dev, "fsl_elbc_init: nand->ecc.steps = %d\n",
	        chip->ecc.steps);
	dev_dbg(ctrl->dev, "fsl_elbc_init: nand->ecc.bytes = %d\n",
	        chip->ecc.bytes);
	dev_dbg(ctrl->dev, "fsl_elbc_init: nand->ecc.total = %d\n",
	        chip->ecc.total);
	dev_dbg(ctrl->dev, "fsl_elbc_init: nand->ecc.layout = %p\n",
	        chip->ecc.layout);
	dev_dbg(ctrl->dev, "fsl_elbc_init: mtd->flags = %08x\n", mtd->flags);
	dev_dbg(ctrl->dev, "fsl_elbc_init: mtd->size = %lld\n", mtd->size);
	dev_dbg(ctrl->dev, "fsl_elbc_init: mtd->erasesize = %d\n",
	        mtd->erasesize);
	dev_dbg(ctrl->dev, "fsl_elbc_init: mtd->writesize = %d\n",
	        mtd->writesize);
	dev_dbg(ctrl->dev, "fsl_elbc_init: mtd->oobsize = %d\n",
	        mtd->oobsize);

	
	if (mtd->writesize == 512) {
		priv->page_size = 0;
		clrbits32(&lbc->bank[priv->bank].or, OR_FCM_PGS);
	} else if (mtd->writesize == 2048) {
		priv->page_size = 1;
		setbits32(&lbc->bank[priv->bank].or, OR_FCM_PGS);
		
		if ((in_be32(&lbc->bank[priv->bank].br) & BR_DECC) ==
		    BR_DECC_CHK_GEN) {
			chip->ecc.size = 512;
			chip->ecc.layout = (priv->fmr & FMR_ECCM) ?
			                   &fsl_elbc_oob_lp_eccm1 :
			                   &fsl_elbc_oob_lp_eccm0;
			chip->badblock_pattern = &largepage_memorybased;
		}
	} else {
		dev_err(ctrl->dev,
		        "fsl_elbc_init: page size %d is not supported\n",
		        mtd->writesize);
		return -1;
	}

	return 0;
}

static int fsl_elbc_read_page(struct mtd_info *mtd,
                              struct nand_chip *chip,
			      uint8_t *buf,
			      int page)
{
	fsl_elbc_read_buf(mtd, buf, mtd->writesize);
	fsl_elbc_read_buf(mtd, chip->oob_poi, mtd->oobsize);

	if (fsl_elbc_wait(mtd, chip) & NAND_STATUS_FAIL)
		mtd->ecc_stats.failed++;

	return 0;
}


static void fsl_elbc_write_page(struct mtd_info *mtd,
                                struct nand_chip *chip,
                                const uint8_t *buf)
{
	struct fsl_elbc_mtd *priv = chip->priv;
	struct fsl_elbc_ctrl *ctrl = priv->ctrl;

	fsl_elbc_write_buf(mtd, buf, mtd->writesize);
	fsl_elbc_write_buf(mtd, chip->oob_poi, mtd->oobsize);

	ctrl->oob_poi = chip->oob_poi;
}

static int fsl_elbc_chip_init(struct fsl_elbc_mtd *priv)
{
	struct fsl_elbc_ctrl *ctrl = priv->ctrl;
	struct fsl_lbc_regs __iomem *lbc = ctrl->regs;
	struct nand_chip *chip = &priv->chip;

	dev_dbg(priv->dev, "eLBC Set Information for bank %d\n", priv->bank);

	
	priv->mtd.priv = chip;
	priv->mtd.owner = THIS_MODULE;

	
	priv->fmr = in_be32(&lbc->fmr) & FMR_ECCM;

	
	
	chip->read_byte = fsl_elbc_read_byte;
	chip->write_buf = fsl_elbc_write_buf;
	chip->read_buf = fsl_elbc_read_buf;
	chip->verify_buf = fsl_elbc_verify_buf;
	chip->select_chip = fsl_elbc_select_chip;
	chip->cmdfunc = fsl_elbc_cmdfunc;
	chip->waitfunc = fsl_elbc_wait;

	chip->bbt_td = &bbt_main_descr;
	chip->bbt_md = &bbt_mirror_descr;

	
	chip->options = NAND_NO_READRDY | NAND_NO_AUTOINCR |
			NAND_USE_FLASH_BBT;

	chip->controller = &ctrl->controller;
	chip->priv = priv;

	chip->ecc.read_page = fsl_elbc_read_page;
	chip->ecc.write_page = fsl_elbc_write_page;

	
	if ((in_be32(&lbc->bank[priv->bank].br) & BR_DECC) ==
	    BR_DECC_CHK_GEN) {
		chip->ecc.mode = NAND_ECC_HW;
		
		chip->ecc.layout = (priv->fmr & FMR_ECCM) ?
				&fsl_elbc_oob_sp_eccm1 : &fsl_elbc_oob_sp_eccm0;
		chip->ecc.size = 512;
		chip->ecc.bytes = 3;
	} else {
		
		chip->ecc.mode = NAND_ECC_SOFT;
	}

	return 0;
}

static int fsl_elbc_chip_remove(struct fsl_elbc_mtd *priv)
{
	struct fsl_elbc_ctrl *ctrl = priv->ctrl;

	nand_release(&priv->mtd);

	kfree(priv->mtd.name);

	if (priv->vbase)
		iounmap(priv->vbase);

	ctrl->chips[priv->bank] = NULL;
	kfree(priv);

	return 0;
}

static int __devinit fsl_elbc_chip_probe(struct fsl_elbc_ctrl *ctrl,
					 struct device_node *node)
{
	struct fsl_lbc_regs __iomem *lbc = ctrl->regs;
	struct fsl_elbc_mtd *priv;
	struct resource res;
#ifdef CONFIG_MTD_PARTITIONS
	static const char *part_probe_types[]
		= { "cmdlinepart", "RedBoot", NULL };
	struct mtd_partition *parts;
#endif
	int ret;
	int bank;

	
	ret = of_address_to_resource(node, 0, &res);
	if (ret) {
		dev_err(ctrl->dev, "failed to get resource\n");
		return ret;
	}

	
	for (bank = 0; bank < MAX_BANKS; bank++)
		if ((in_be32(&lbc->bank[bank].br) & BR_V) &&
		    (in_be32(&lbc->bank[bank].br) & BR_MSEL) == BR_MS_FCM &&
		    (in_be32(&lbc->bank[bank].br) &
		     in_be32(&lbc->bank[bank].or) & BR_BA)
		     == res.start)
			break;

	if (bank >= MAX_BANKS) {
		dev_err(ctrl->dev, "address did not match any chip selects\n");
		return -ENODEV;
	}

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	ctrl->chips[bank] = priv;
	priv->bank = bank;
	priv->ctrl = ctrl;
	priv->dev = ctrl->dev;

	priv->vbase = ioremap(res.start, res.end - res.start + 1);
	if (!priv->vbase) {
		dev_err(ctrl->dev, "failed to map chip region\n");
		ret = -ENOMEM;
		goto err;
	}

	priv->mtd.name = kasprintf(GFP_KERNEL, "%x.flash", (unsigned)res.start);
	if (!priv->mtd.name) {
		ret = -ENOMEM;
		goto err;
	}

	ret = fsl_elbc_chip_init(priv);
	if (ret)
		goto err;

	ret = nand_scan_ident(&priv->mtd, 1);
	if (ret)
		goto err;

	ret = fsl_elbc_chip_init_tail(&priv->mtd);
	if (ret)
		goto err;

	ret = nand_scan_tail(&priv->mtd);
	if (ret)
		goto err;

#ifdef CONFIG_MTD_PARTITIONS
	
	ret = parse_mtd_partitions(&priv->mtd, part_probe_types, &parts, 0);
	if (ret < 0)
		goto err;

#ifdef CONFIG_MTD_OF_PARTS
	if (ret == 0) {
		ret = of_mtd_parse_partitions(priv->dev, node, &parts);
		if (ret < 0)
			goto err;
	}
#endif

	if (ret > 0)
		add_mtd_partitions(&priv->mtd, parts, ret);
	else
#endif
		add_mtd_device(&priv->mtd);

	printk(KERN_INFO "eLBC NAND device at 0x%llx, bank %d\n",
	       (unsigned long long)res.start, priv->bank);
	return 0;

err:
	fsl_elbc_chip_remove(priv);
	return ret;
}

static int __devinit fsl_elbc_ctrl_init(struct fsl_elbc_ctrl *ctrl)
{
	struct fsl_lbc_regs __iomem *lbc = ctrl->regs;

	
	setbits32(&lbc->ltesr, LTESR_NAND_MASK);
	out_be32(&lbc->lteatr, 0);

	
	out_be32(&lbc->lteir, LTESR_NAND_MASK);

	ctrl->read_bytes = 0;
	ctrl->index = 0;
	ctrl->addr = NULL;

	return 0;
}

static int fsl_elbc_ctrl_remove(struct of_device *ofdev)
{
	struct fsl_elbc_ctrl *ctrl = dev_get_drvdata(&ofdev->dev);
	int i;

	for (i = 0; i < MAX_BANKS; i++)
		if (ctrl->chips[i])
			fsl_elbc_chip_remove(ctrl->chips[i]);

	if (ctrl->irq)
		free_irq(ctrl->irq, ctrl);

	if (ctrl->regs)
		iounmap(ctrl->regs);

	dev_set_drvdata(&ofdev->dev, NULL);
	kfree(ctrl);
	return 0;
}



static irqreturn_t fsl_elbc_ctrl_irq(int irqno, void *data)
{
	struct fsl_elbc_ctrl *ctrl = data;
	struct fsl_lbc_regs __iomem *lbc = ctrl->regs;
	__be32 status = in_be32(&lbc->ltesr) & LTESR_NAND_MASK;

	if (status) {
		out_be32(&lbc->ltesr, status);
		out_be32(&lbc->lteatr, 0);

		ctrl->irq_status = status;
		smp_wmb();
		wake_up(&ctrl->irq_wait);

		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}



static int __devinit fsl_elbc_ctrl_probe(struct of_device *ofdev,
                                         const struct of_device_id *match)
{
	struct device_node *child;
	struct fsl_elbc_ctrl *ctrl;
	int ret;

	ctrl = kzalloc(sizeof(*ctrl), GFP_KERNEL);
	if (!ctrl)
		return -ENOMEM;

	dev_set_drvdata(&ofdev->dev, ctrl);

	spin_lock_init(&ctrl->controller.lock);
	init_waitqueue_head(&ctrl->controller.wq);
	init_waitqueue_head(&ctrl->irq_wait);

	ctrl->regs = of_iomap(ofdev->node, 0);
	if (!ctrl->regs) {
		dev_err(&ofdev->dev, "failed to get memory region\n");
		ret = -ENODEV;
		goto err;
	}

	ctrl->irq = of_irq_to_resource(ofdev->node, 0, NULL);
	if (ctrl->irq == NO_IRQ) {
		dev_err(&ofdev->dev, "failed to get irq resource\n");
		ret = -ENODEV;
		goto err;
	}

	ctrl->dev = &ofdev->dev;

	ret = fsl_elbc_ctrl_init(ctrl);
	if (ret < 0)
		goto err;

	ret = request_irq(ctrl->irq, fsl_elbc_ctrl_irq, 0, "fsl-elbc", ctrl);
	if (ret != 0) {
		dev_err(&ofdev->dev, "failed to install irq (%d)\n",
		        ctrl->irq);
		ret = ctrl->irq;
		goto err;
	}

	for_each_child_of_node(ofdev->node, child)
		if (of_device_is_compatible(child, "fsl,elbc-fcm-nand"))
			fsl_elbc_chip_probe(ctrl, child);

	return 0;

err:
	fsl_elbc_ctrl_remove(ofdev);
	return ret;
}

static const struct of_device_id fsl_elbc_match[] = {
	{
		.compatible = "fsl,elbc",
	},
	{}
};

static struct of_platform_driver fsl_elbc_ctrl_driver = {
	.driver = {
		.name	= "fsl-elbc",
	},
	.match_table = fsl_elbc_match,
	.probe = fsl_elbc_ctrl_probe,
	.remove = fsl_elbc_ctrl_remove,
};

static int __init fsl_elbc_init(void)
{
	return of_register_platform_driver(&fsl_elbc_ctrl_driver);
}

static void __exit fsl_elbc_exit(void)
{
	of_unregister_platform_driver(&fsl_elbc_ctrl_driver);
}

module_init(fsl_elbc_init);
module_exit(fsl_elbc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Freescale");
MODULE_DESCRIPTION("Freescale Enhanced Local Bus Controller MTD NAND driver");
