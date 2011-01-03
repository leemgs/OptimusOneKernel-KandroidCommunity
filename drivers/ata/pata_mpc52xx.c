

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/libata.h>
#include <linux/of_platform.h>
#include <linux/types.h>

#include <asm/cacheflush.h>
#include <asm/prom.h>
#include <asm/mpc52xx.h>

#include <sysdev/bestcomm/bestcomm.h>
#include <sysdev/bestcomm/bestcomm_priv.h>
#include <sysdev/bestcomm/ata.h>

#define DRV_NAME	"mpc52xx_ata"


struct mpc52xx_ata_timings {
	u32	pio1;
	u32	pio2;
	u32	mdma1;
	u32	mdma2;
	u32	udma1;
	u32	udma2;
	u32	udma3;
	u32	udma4;
	u32	udma5;
	int	using_udma;
};

struct mpc52xx_ata_priv {
	unsigned int			ipb_period;
	struct mpc52xx_ata __iomem	*ata_regs;
	phys_addr_t			ata_regs_pa;
	int				ata_irq;
	struct mpc52xx_ata_timings	timings[2];
	int				csel;

	
	struct bcom_task		*dmatsk;
	const struct udmaspec		*udmaspec;
	const struct mdmaspec		*mdmaspec;
	int 				mpc52xx_ata_dma_last_write;
	int				waiting_for_dma;
};



static const int ataspec_t0[5]    = {600, 383, 240, 180, 120};
static const int ataspec_t1[5]    = { 70,  50,  30,  30,  25};
static const int ataspec_t2_8[5]  = {290, 290, 290,  80,  70};
static const int ataspec_t2_16[5] = {165, 125, 100,  80,  70};
static const int ataspec_t2i[5]   = {  0,   0,   0,  70,  25};
static const int ataspec_t4[5]    = { 30,  20,  15,  10,  10};
static const int ataspec_ta[5]    = { 35,  35,  35,  35,  35};

#define CALC_CLKCYC(c,v) ((((v)+(c)-1)/(c)))




struct mdmaspec {
	u32 t0M;
	u32 td;
	u32 th;
	u32 tj;
	u32 tkw;
	u32 tm;
	u32 tn;
};

static const struct mdmaspec mdmaspec66[3] = {
	{ .t0M = 32, .td = 15, .th = 2, .tj = 2, .tkw = 15, .tm = 4, .tn = 1 },
	{ .t0M = 10, .td = 6,  .th = 1, .tj = 1, .tkw = 4,  .tm = 2, .tn = 1 },
	{ .t0M = 8,  .td = 5,  .th = 1, .tj = 1, .tkw = 2,  .tm = 2, .tn = 1 },
};

static const struct mdmaspec mdmaspec132[3] = {
	{ .t0M = 64, .td = 29, .th = 3, .tj = 3, .tkw = 29, .tm = 7, .tn = 2 },
	{ .t0M = 20, .td = 11, .th = 2, .tj = 1, .tkw = 7,  .tm = 4, .tn = 1 },
	{ .t0M = 16, .td = 10, .th = 2, .tj = 1, .tkw = 4,  .tm = 4, .tn = 1 },
};


struct udmaspec {
	u32 tcyc;
	u32 t2cyc;
	u32 tds;
	u32 tdh;
	u32 tdvs;
	u32 tdvh;
	u32 tfs;
	u32 tli;
	u32 tmli;
	u32 taz;
	u32 tzah;
	u32 tenv;
	u32 tsr;
	u32 trfs;
	u32 trp;
	u32 tack;
	u32 tss;
};

static const struct udmaspec udmaspec66[6] = {
	{ .tcyc = 8,  .t2cyc = 16, .tds  = 1,  .tdh  = 1, .tdvs = 5,  .tdvh = 1,
	  .tfs  = 16, .tli   = 10, .tmli = 2,  .taz  = 1, .tzah = 2,  .tenv = 2,
	  .tsr  = 3,  .trfs  = 5,  .trp  = 11, .tack = 2, .tss  = 4,
	},
	{ .tcyc = 5,  .t2cyc = 11, .tds  = 1,  .tdh  = 1, .tdvs = 4,  .tdvh = 1,
	  .tfs  = 14, .tli   = 10, .tmli = 2,  .taz  = 1, .tzah = 2,  .tenv = 2,
	  .tsr  = 2,  .trfs  = 5,  .trp  = 9,  .tack = 2, .tss  = 4,
	},
	{ .tcyc = 4,  .t2cyc = 8,  .tds  = 1,  .tdh  = 1, .tdvs = 3,  .tdvh = 1,
	  .tfs  = 12, .tli   = 10, .tmli = 2,  .taz  = 1, .tzah = 2,  .tenv = 2,
	  .tsr  = 2,  .trfs  = 4,  .trp  = 7,  .tack = 2, .tss  = 4,
	},
	{ .tcyc = 3,  .t2cyc = 6,  .tds  = 1,  .tdh  = 1, .tdvs = 2,  .tdvh = 1,
	  .tfs  = 9,  .tli   = 7,  .tmli = 2,  .taz  = 1, .tzah = 2,  .tenv = 2,
	  .tsr  = 2,  .trfs  = 4,  .trp  = 7,  .tack = 2, .tss  = 4,
	},
	{ .tcyc = 2,  .t2cyc = 4,  .tds  = 1,  .tdh  = 1, .tdvs = 1,  .tdvh = 1,
	  .tfs  = 8,  .tli   = 8,  .tmli = 2,  .taz  = 1, .tzah = 2,  .tenv = 2,
	  .tsr  = 2,  .trfs  = 4,  .trp  = 7,  .tack = 2, .tss  = 4,
	},
	{ .tcyc = 2,  .t2cyc = 2,  .tds  = 1,  .tdh  = 1, .tdvs = 1,  .tdvh = 1,
	  .tfs  = 6,  .tli   = 5,  .tmli = 2,  .taz  = 1, .tzah = 2,  .tenv = 2,
	  .tsr  = 2,  .trfs  = 4,  .trp  = 6,  .tack = 2, .tss  = 4,
	},
};

static const struct udmaspec udmaspec132[6] = {
	{ .tcyc = 15, .t2cyc = 31, .tds  = 2,  .tdh  = 1, .tdvs = 10, .tdvh = 1,
	  .tfs  = 30, .tli   = 20, .tmli = 3,  .taz  = 2, .tzah = 3,  .tenv = 3,
	  .tsr  = 7,  .trfs  = 10, .trp  = 22, .tack = 3, .tss  = 7,
	},
	{ .tcyc = 10, .t2cyc = 21, .tds  = 2,  .tdh  = 1, .tdvs = 7,  .tdvh = 1,
	  .tfs  = 27, .tli   = 20, .tmli = 3,  .taz  = 2, .tzah = 3,  .tenv = 3,
	  .tsr  = 4,  .trfs  = 10, .trp  = 17, .tack = 3, .tss  = 7,
	},
	{ .tcyc = 6,  .t2cyc = 12, .tds  = 1,  .tdh  = 1, .tdvs = 5,  .tdvh = 1,
	  .tfs  = 23, .tli   = 20, .tmli = 3,  .taz  = 2, .tzah = 3,  .tenv = 3,
	  .tsr  = 3,  .trfs  = 8,  .trp  = 14, .tack = 3, .tss  = 7,
	},
	{ .tcyc = 7,  .t2cyc = 12, .tds  = 1,  .tdh  = 1, .tdvs = 3,  .tdvh = 1,
	  .tfs  = 15, .tli   = 13, .tmli = 3,  .taz  = 2, .tzah = 3,  .tenv = 3,
	  .tsr  = 3,  .trfs  = 8,  .trp  = 14, .tack = 3, .tss  = 7,
	},
	{ .tcyc = 2,  .t2cyc = 5,  .tds  = 0,  .tdh  = 0, .tdvs = 1,  .tdvh = 1,
	  .tfs  = 16, .tli   = 14, .tmli = 2,  .taz  = 1, .tzah = 2,  .tenv = 2,
	  .tsr  = 2,  .trfs  = 7,  .trp  = 13, .tack = 2, .tss  = 6,
	},
	{ .tcyc = 3,  .t2cyc = 6,  .tds  = 1,  .tdh  = 1, .tdvs = 1,  .tdvh = 1,
	  .tfs  = 12, .tli   = 10, .tmli = 3,  .taz  = 2, .tzah = 3,  .tenv = 3,
	  .tsr  = 3,  .trfs  = 7,  .trp  = 12, .tack = 3, .tss  = 7,
	},
};




#define MPC52xx_ATA_HOSTCONF_SMR	0x80000000UL 
#define MPC52xx_ATA_HOSTCONF_FR		0x40000000UL 
#define MPC52xx_ATA_HOSTCONF_IE		0x02000000UL 
#define MPC52xx_ATA_HOSTCONF_IORDY	0x01000000UL 

#define MPC52xx_ATA_HOSTSTAT_TIP	0x80000000UL 
#define MPC52xx_ATA_HOSTSTAT_UREP	0x40000000UL 
#define MPC52xx_ATA_HOSTSTAT_RERR	0x02000000UL 
#define MPC52xx_ATA_HOSTSTAT_WERR	0x01000000UL 

#define MPC52xx_ATA_FIFOSTAT_EMPTY	0x01 
#define MPC52xx_ATA_FIFOSTAT_ERROR	0x40 

#define MPC52xx_ATA_DMAMODE_WRITE	0x01 
#define MPC52xx_ATA_DMAMODE_READ	0x02 
#define MPC52xx_ATA_DMAMODE_UDMA	0x04 
#define MPC52xx_ATA_DMAMODE_IE		0x08 
#define MPC52xx_ATA_DMAMODE_FE		0x10 
#define MPC52xx_ATA_DMAMODE_FR		0x20 
#define MPC52xx_ATA_DMAMODE_HUT		0x40 

#define MAX_DMA_BUFFERS 128
#define MAX_DMA_BUFFER_SIZE 0x20000u


struct mpc52xx_ata {

	
	u32 config;		
	u32 host_status;	
	u32 pio1;		
	u32 pio2;		
	u32 mdma1;		
	u32 mdma2;		
	u32 udma1;		
	u32 udma2;		
	u32 udma3;		
	u32 udma4;		
	u32 udma5;		
	u32 share_cnt;		
	u32 reserved0[3];

	
	u32 fifo_data;		
	u8  fifo_status_frame;	
	u8  fifo_status;	
	u16 reserved7[1];
	u8  fifo_control;	
	u8  reserved8[5];
	u16 fifo_alarm;		
	u16 reserved9;
	u16 fifo_rdp;		
	u16 reserved10;
	u16 fifo_wrp;		
	u16 reserved11;
	u16 fifo_lfrdp;		
	u16 reserved12;
	u16 fifo_lfwrp;		

	
	u8  tf_control;		
	u8  reserved13[3];
	u16 tf_data;		
	u16 reserved14;
	u8  tf_features;	
	u8  reserved15[3];
	u8  tf_sec_count;	
	u8  reserved16[3];
	u8  tf_sec_num;		
	u8  reserved17[3];
	u8  tf_cyl_low;		
	u8  reserved18[3];
	u8  tf_cyl_high;	
	u8  reserved19[3];
	u8  tf_dev_head;	
	u8  reserved20[3];
	u8  tf_command;		
	u8  dma_mode;		
	u8  reserved21[2];
};








static int
mpc52xx_ata_compute_pio_timings(struct mpc52xx_ata_priv *priv, int dev, int pio)
{
	struct mpc52xx_ata_timings *timing = &priv->timings[dev];
	unsigned int ipb_period = priv->ipb_period;
	unsigned int t0, t1, t2_8, t2_16, t2i, t4, ta;

	if ((pio < 0) || (pio > 4))
		return -EINVAL;

	t0	= CALC_CLKCYC(ipb_period, 1000 * ataspec_t0[pio]);
	t1	= CALC_CLKCYC(ipb_period, 1000 * ataspec_t1[pio]);
	t2_8	= CALC_CLKCYC(ipb_period, 1000 * ataspec_t2_8[pio]);
	t2_16	= CALC_CLKCYC(ipb_period, 1000 * ataspec_t2_16[pio]);
	t2i	= CALC_CLKCYC(ipb_period, 1000 * ataspec_t2i[pio]);
	t4	= CALC_CLKCYC(ipb_period, 1000 * ataspec_t4[pio]);
	ta	= CALC_CLKCYC(ipb_period, 1000 * ataspec_ta[pio]);

	timing->pio1 = (t0 << 24) | (t2_8 << 16) | (t2_16 << 8) | (t2i);
	timing->pio2 = (t4 << 24) | (t1 << 16) | (ta << 8);

	return 0;
}

static int
mpc52xx_ata_compute_mdma_timings(struct mpc52xx_ata_priv *priv, int dev,
				 int speed)
{
	struct mpc52xx_ata_timings *t = &priv->timings[dev];
	const struct mdmaspec *s = &priv->mdmaspec[speed];

	if (speed < 0 || speed > 2)
		return -EINVAL;

	t->mdma1 = (s->t0M << 24) | (s->td << 16) | (s->tkw << 8) | (s->tm);
	t->mdma2 = (s->th << 24) | (s->tj << 16) | (s->tn << 8);
	t->using_udma = 0;

	return 0;
}

static int
mpc52xx_ata_compute_udma_timings(struct mpc52xx_ata_priv *priv, int dev,
				 int speed)
{
	struct mpc52xx_ata_timings *t = &priv->timings[dev];
	const struct udmaspec *s = &priv->udmaspec[speed];

	if (speed < 0 || speed > 2)
		return -EINVAL;

	t->udma1 = (s->t2cyc << 24) | (s->tcyc << 16) | (s->tds << 8) | s->tdh;
	t->udma2 = (s->tdvs << 24) | (s->tdvh << 16) | (s->tfs << 8) | s->tli;
	t->udma3 = (s->tmli << 24) | (s->taz << 16) | (s->tenv << 8) | s->tsr;
	t->udma4 = (s->tss << 24) | (s->trfs << 16) | (s->trp << 8) | s->tack;
	t->udma5 = (s->tzah << 24);
	t->using_udma = 1;

	return 0;
}

static void
mpc52xx_ata_apply_timings(struct mpc52xx_ata_priv *priv, int device)
{
	struct mpc52xx_ata __iomem *regs = priv->ata_regs;
	struct mpc52xx_ata_timings *timing = &priv->timings[device];

	out_be32(&regs->pio1,  timing->pio1);
	out_be32(&regs->pio2,  timing->pio2);
	out_be32(&regs->mdma1, timing->mdma1);
	out_be32(&regs->mdma2, timing->mdma2);
	out_be32(&regs->udma1, timing->udma1);
	out_be32(&regs->udma2, timing->udma2);
	out_be32(&regs->udma3, timing->udma3);
	out_be32(&regs->udma4, timing->udma4);
	out_be32(&regs->udma5, timing->udma5);
	priv->csel = device;
}

static int
mpc52xx_ata_hw_init(struct mpc52xx_ata_priv *priv)
{
	struct mpc52xx_ata __iomem *regs = priv->ata_regs;
	int tslot;

	
	out_be32(&regs->share_cnt, 0);

	
	out_be32(&regs->config,
			MPC52xx_ATA_HOSTCONF_IE |
			MPC52xx_ATA_HOSTCONF_IORDY |
			MPC52xx_ATA_HOSTCONF_SMR |
			MPC52xx_ATA_HOSTCONF_FR);

	udelay(10);

	out_be32(&regs->config,
			MPC52xx_ATA_HOSTCONF_IE |
			MPC52xx_ATA_HOSTCONF_IORDY);

	
	tslot = CALC_CLKCYC(priv->ipb_period, 1000000);
	out_be32(&regs->share_cnt, tslot << 16);

	
	memset(priv->timings, 0x00, 2*sizeof(struct mpc52xx_ata_timings));

	mpc52xx_ata_compute_pio_timings(priv, 0, 0);
	mpc52xx_ata_compute_pio_timings(priv, 1, 0);

	mpc52xx_ata_apply_timings(priv, 0);

	return 0;
}






static void
mpc52xx_ata_set_piomode(struct ata_port *ap, struct ata_device *adev)
{
	struct mpc52xx_ata_priv *priv = ap->host->private_data;
	int pio, rv;

	pio = adev->pio_mode - XFER_PIO_0;

	rv = mpc52xx_ata_compute_pio_timings(priv, adev->devno, pio);

	if (rv) {
		dev_err(ap->dev, "error: invalid PIO mode: %d\n", pio);
		return;
	}

	mpc52xx_ata_apply_timings(priv, adev->devno);
}

static void
mpc52xx_ata_set_dmamode(struct ata_port *ap, struct ata_device *adev)
{
	struct mpc52xx_ata_priv *priv = ap->host->private_data;
	int rv;

	if (adev->dma_mode >= XFER_UDMA_0) {
		int dma = adev->dma_mode - XFER_UDMA_0;
		rv = mpc52xx_ata_compute_udma_timings(priv, adev->devno, dma);
	} else {
		int dma = adev->dma_mode - XFER_MW_DMA_0;
		rv = mpc52xx_ata_compute_mdma_timings(priv, adev->devno, dma);
	}

	if (rv) {
		dev_alert(ap->dev,
			"Trying to select invalid DMA mode %d\n",
			adev->dma_mode);
		return;
	}

	mpc52xx_ata_apply_timings(priv, adev->devno);
}

static void
mpc52xx_ata_dev_select(struct ata_port *ap, unsigned int device)
{
	struct mpc52xx_ata_priv *priv = ap->host->private_data;

	if (device != priv->csel)
		mpc52xx_ata_apply_timings(priv, device);

	ata_sff_dev_select(ap, device);
}

static int
mpc52xx_ata_build_dmatable(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct mpc52xx_ata_priv *priv = ap->host->private_data;
	struct bcom_ata_bd *bd;
	unsigned int read = !(qc->tf.flags & ATA_TFLAG_WRITE), si;
	struct scatterlist *sg;
	int count = 0;

	if (read)
		bcom_ata_rx_prepare(priv->dmatsk);
	else
		bcom_ata_tx_prepare(priv->dmatsk);

	for_each_sg(qc->sg, sg, qc->n_elem, si) {
		dma_addr_t cur_addr = sg_dma_address(sg);
		u32 cur_len = sg_dma_len(sg);

		while (cur_len) {
			unsigned int tc = min(cur_len, MAX_DMA_BUFFER_SIZE);
			bd = (struct bcom_ata_bd *)
				bcom_prepare_next_buffer(priv->dmatsk);

			if (read) {
				bd->status = tc;
				bd->src_pa = (__force u32) priv->ata_regs_pa +
					offsetof(struct mpc52xx_ata, fifo_data);
				bd->dst_pa = (__force u32) cur_addr;
			} else {
				bd->status = tc;
				bd->src_pa = (__force u32) cur_addr;
				bd->dst_pa = (__force u32) priv->ata_regs_pa +
					offsetof(struct mpc52xx_ata, fifo_data);
			}

			bcom_submit_next_buffer(priv->dmatsk, NULL);

			cur_addr += tc;
			cur_len -= tc;
			count++;

			if (count > MAX_DMA_BUFFERS) {
				dev_alert(ap->dev, "dma table"
					"too small\n");
				goto use_pio_instead;
			}
		}
	}
	return 1;

 use_pio_instead:
	bcom_ata_reset_bd(priv->dmatsk);
	return 0;
}

static void
mpc52xx_bmdma_setup(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct mpc52xx_ata_priv *priv = ap->host->private_data;
	struct mpc52xx_ata __iomem *regs = priv->ata_regs;

	unsigned int read = !(qc->tf.flags & ATA_TFLAG_WRITE);
	u8 dma_mode;

	if (!mpc52xx_ata_build_dmatable(qc))
		dev_alert(ap->dev, "%s: %i, return 1?\n",
			__func__, __LINE__);

	
	if (in_8(&priv->ata_regs->fifo_status) & MPC52xx_ATA_FIFOSTAT_ERROR)
		dev_alert(ap->dev, "%s: FIFO error detected: 0x%02x!\n",
			__func__, in_8(&priv->ata_regs->fifo_status));

	if (read) {
		dma_mode = MPC52xx_ATA_DMAMODE_IE | MPC52xx_ATA_DMAMODE_READ |
				MPC52xx_ATA_DMAMODE_FE;

		
		if (priv->mpc52xx_ata_dma_last_write != 0) {
			priv->mpc52xx_ata_dma_last_write = 0;

			
			out_8(&regs->fifo_control, 7);
			out_be16(&regs->fifo_alarm, 128);

			
			out_8(&regs->dma_mode, MPC52xx_ATA_DMAMODE_FR);
		}
	} else {
		dma_mode = MPC52xx_ATA_DMAMODE_IE | MPC52xx_ATA_DMAMODE_WRITE;

		
		if (priv->mpc52xx_ata_dma_last_write != 1) {
			priv->mpc52xx_ata_dma_last_write = 1;

			
			out_8(&regs->fifo_control, 4);
			out_be16(&regs->fifo_alarm, 128);
		}
	}

	if (priv->timings[qc->dev->devno].using_udma)
		dma_mode |= MPC52xx_ATA_DMAMODE_UDMA;

	out_8(&regs->dma_mode, dma_mode);
	priv->waiting_for_dma = ATA_DMA_ACTIVE;

	ata_wait_idle(ap);
	ap->ops->sff_exec_command(ap, &qc->tf);
}

static void
mpc52xx_bmdma_start(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct mpc52xx_ata_priv *priv = ap->host->private_data;

	bcom_set_task_auto_start(priv->dmatsk->tasknum, priv->dmatsk->tasknum);
	bcom_enable(priv->dmatsk);
}

static void
mpc52xx_bmdma_stop(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct mpc52xx_ata_priv *priv = ap->host->private_data;

	bcom_disable(priv->dmatsk);
	bcom_ata_reset_bd(priv->dmatsk);
	priv->waiting_for_dma = 0;

	
	if (in_8(&priv->ata_regs->fifo_status) & MPC52xx_ATA_FIFOSTAT_ERROR)
		dev_alert(ap->dev, "%s: FIFO error detected: 0x%02x!\n",
			__func__, in_8(&priv->ata_regs->fifo_status));
}

static u8
mpc52xx_bmdma_status(struct ata_port *ap)
{
	struct mpc52xx_ata_priv *priv = ap->host->private_data;

	
	if (in_8(&priv->ata_regs->fifo_status) & MPC52xx_ATA_FIFOSTAT_ERROR) {
		dev_alert(ap->dev, "%s: FIFO error detected: 0x%02x!\n",
			__func__, in_8(&priv->ata_regs->fifo_status));
		return priv->waiting_for_dma | ATA_DMA_ERR;
	}

	return priv->waiting_for_dma;
}

static irqreturn_t
mpc52xx_ata_task_irq(int irq, void *vpriv)
{
	struct mpc52xx_ata_priv *priv = vpriv;
	while (bcom_buffer_done(priv->dmatsk))
		bcom_retrieve_buffer(priv->dmatsk, NULL, NULL);

	priv->waiting_for_dma |= ATA_DMA_INTR;

	return IRQ_HANDLED;
}

static struct scsi_host_template mpc52xx_ata_sht = {
	ATA_PIO_SHT(DRV_NAME),
};

static struct ata_port_operations mpc52xx_ata_port_ops = {
	.inherits		= &ata_sff_port_ops,
	.sff_dev_select		= mpc52xx_ata_dev_select,
	.set_piomode		= mpc52xx_ata_set_piomode,
	.set_dmamode		= mpc52xx_ata_set_dmamode,
	.bmdma_setup		= mpc52xx_bmdma_setup,
	.bmdma_start		= mpc52xx_bmdma_start,
	.bmdma_stop		= mpc52xx_bmdma_stop,
	.bmdma_status		= mpc52xx_bmdma_status,
	.qc_prep		= ata_noop_qc_prep,
};

static int __devinit
mpc52xx_ata_init_one(struct device *dev, struct mpc52xx_ata_priv *priv,
		     unsigned long raw_ata_regs, int mwdma_mask, int udma_mask)
{
	struct ata_host *host;
	struct ata_port *ap;
	struct ata_ioports *aio;

	host = ata_host_alloc(dev, 1);
	if (!host)
		return -ENOMEM;

	ap = host->ports[0];
	ap->flags		|= ATA_FLAG_SLAVE_POSS;
	ap->pio_mask		= ATA_PIO4;
	ap->mwdma_mask		= mwdma_mask;
	ap->udma_mask		= udma_mask;
	ap->ops			= &mpc52xx_ata_port_ops;
	host->private_data	= priv;

	aio = &ap->ioaddr;
	aio->cmd_addr		= NULL;	
	aio->altstatus_addr	= &priv->ata_regs->tf_control;
	aio->ctl_addr		= &priv->ata_regs->tf_control;
	aio->data_addr		= &priv->ata_regs->tf_data;
	aio->error_addr		= &priv->ata_regs->tf_features;
	aio->feature_addr	= &priv->ata_regs->tf_features;
	aio->nsect_addr		= &priv->ata_regs->tf_sec_count;
	aio->lbal_addr		= &priv->ata_regs->tf_sec_num;
	aio->lbam_addr		= &priv->ata_regs->tf_cyl_low;
	aio->lbah_addr		= &priv->ata_regs->tf_cyl_high;
	aio->device_addr	= &priv->ata_regs->tf_dev_head;
	aio->status_addr	= &priv->ata_regs->tf_command;
	aio->command_addr	= &priv->ata_regs->tf_command;

	ata_port_desc(ap, "ata_regs 0x%lx", raw_ata_regs);

	
	return ata_host_activate(host, priv->ata_irq, ata_sff_interrupt, 0,
				 &mpc52xx_ata_sht);
}

static struct mpc52xx_ata_priv *
mpc52xx_ata_remove_one(struct device *dev)
{
	struct ata_host *host = dev_get_drvdata(dev);
	struct mpc52xx_ata_priv *priv = host->private_data;

	ata_host_detach(host);

	return priv;
}






static int __devinit
mpc52xx_ata_probe(struct of_device *op, const struct of_device_id *match)
{
	unsigned int ipb_freq;
	struct resource res_mem;
	int ata_irq = 0;
	struct mpc52xx_ata __iomem *ata_regs;
	struct mpc52xx_ata_priv *priv = NULL;
	int rv, ret, task_irq = 0;
	int mwdma_mask = 0, udma_mask = 0;
	const __be32 *prop;
	int proplen;
	struct bcom_task *dmatsk = NULL;

	
	ipb_freq = mpc5xxx_get_bus_frequency(op->node);
	if (!ipb_freq) {
		dev_err(&op->dev, "could not determine IPB bus frequency\n");
		return -ENODEV;
	}

	
	rv = of_address_to_resource(op->node, 0, &res_mem);
	if (rv) {
		dev_err(&op->dev, "could not determine device base address\n");
		return rv;
	}

	if (!devm_request_mem_region(&op->dev, res_mem.start,
				     sizeof(*ata_regs), DRV_NAME)) {
		dev_err(&op->dev, "error requesting register region\n");
		return -EBUSY;
	}

	ata_regs = devm_ioremap(&op->dev, res_mem.start, sizeof(*ata_regs));
	if (!ata_regs) {
		dev_err(&op->dev, "error mapping device registers\n");
		rv = -ENOMEM;
		goto err;
	}

	
	prop = of_get_property(op->node, "mwdma-mode", &proplen);
	if ((prop) && (proplen >= 4))
		mwdma_mask = ATA_MWDMA2 & ((1 << (*prop + 1)) - 1);
	prop = of_get_property(op->node, "udma-mode", &proplen);
	if ((prop) && (proplen >= 4))
		udma_mask = ATA_UDMA2 & ((1 << (*prop + 1)) - 1);

	ata_irq = irq_of_parse_and_map(op->node, 0);
	if (ata_irq == NO_IRQ) {
		dev_err(&op->dev, "error mapping irq\n");
		return -EINVAL;
	}

	
	priv = devm_kzalloc(&op->dev, sizeof(*priv), GFP_ATOMIC);
	if (!priv) {
		dev_err(&op->dev, "error allocating private structure\n");
		rv = -ENOMEM;
		goto err;
	}

	priv->ipb_period = 1000000000 / (ipb_freq / 1000);
	priv->ata_regs = ata_regs;
	priv->ata_regs_pa = res_mem.start;
	priv->ata_irq = ata_irq;
	priv->csel = -1;
	priv->mpc52xx_ata_dma_last_write = -1;

	if (ipb_freq/1000000 == 66) {
		priv->mdmaspec = mdmaspec66;
		priv->udmaspec = udmaspec66;
	} else {
		priv->mdmaspec = mdmaspec132;
		priv->udmaspec = udmaspec132;
	}

	
	dmatsk = bcom_ata_init(MAX_DMA_BUFFERS, MAX_DMA_BUFFER_SIZE);
	if (!dmatsk) {
		dev_err(&op->dev, "bestcomm initialization failed\n");
		rv = -ENOMEM;
		goto err;
	}

	task_irq = bcom_get_task_irq(dmatsk);
	ret = request_irq(task_irq, &mpc52xx_ata_task_irq, IRQF_DISABLED,
				"ATA task", priv);
	if (ret) {
		dev_err(&op->dev, "error requesting DMA IRQ\n");
		goto err;
	}
	priv->dmatsk = dmatsk;

	
	rv = mpc52xx_ata_hw_init(priv);
	if (rv) {
		dev_err(&op->dev, "error initializing hardware\n");
		goto err;
	}

	
	rv = mpc52xx_ata_init_one(&op->dev, priv, res_mem.start,
				  mwdma_mask, udma_mask);
	if (rv) {
		dev_err(&op->dev, "error registering with ATA layer\n");
		goto err;
	}

	return 0;

 err:
	devm_release_mem_region(&op->dev, res_mem.start, sizeof(*ata_regs));
	if (ata_irq)
		irq_dispose_mapping(ata_irq);
	if (task_irq)
		irq_dispose_mapping(task_irq);
	if (dmatsk)
		bcom_ata_release(dmatsk);
	if (ata_regs)
		devm_iounmap(&op->dev, ata_regs);
	if (priv)
		devm_kfree(&op->dev, priv);
	return rv;
}

static int
mpc52xx_ata_remove(struct of_device *op)
{
	struct mpc52xx_ata_priv *priv;
	int task_irq;

	
	priv = mpc52xx_ata_remove_one(&op->dev);

	
	task_irq = bcom_get_task_irq(priv->dmatsk);
	irq_dispose_mapping(task_irq);
	bcom_ata_release(priv->dmatsk);
	irq_dispose_mapping(priv->ata_irq);

	
	devm_iounmap(&op->dev, priv->ata_regs);
	devm_release_mem_region(&op->dev, priv->ata_regs_pa,
				sizeof(*priv->ata_regs));
	devm_kfree(&op->dev, priv);

	return 0;
}


#ifdef CONFIG_PM

static int
mpc52xx_ata_suspend(struct of_device *op, pm_message_t state)
{
	struct ata_host *host = dev_get_drvdata(&op->dev);

	return ata_host_suspend(host, state);
}

static int
mpc52xx_ata_resume(struct of_device *op)
{
	struct ata_host *host = dev_get_drvdata(&op->dev);
	struct mpc52xx_ata_priv *priv = host->private_data;
	int rv;

	rv = mpc52xx_ata_hw_init(priv);
	if (rv) {
		dev_err(host->dev, "error initializing hardware\n");
		return rv;
	}

	ata_host_resume(host);

	return 0;
}

#endif


static struct of_device_id mpc52xx_ata_of_match[] = {
	{ .compatible = "fsl,mpc5200-ata", },
	{ .compatible = "mpc5200-ata", },
	{},
};


static struct of_platform_driver mpc52xx_ata_of_platform_driver = {
	.owner		= THIS_MODULE,
	.name		= DRV_NAME,
	.match_table	= mpc52xx_ata_of_match,
	.probe		= mpc52xx_ata_probe,
	.remove		= mpc52xx_ata_remove,
#ifdef CONFIG_PM
	.suspend	= mpc52xx_ata_suspend,
	.resume		= mpc52xx_ata_resume,
#endif
	.driver		= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};






static int __init
mpc52xx_ata_init(void)
{
	printk(KERN_INFO "ata: MPC52xx IDE/ATA libata driver\n");
	return of_register_platform_driver(&mpc52xx_ata_of_platform_driver);
}

static void __exit
mpc52xx_ata_exit(void)
{
	of_unregister_platform_driver(&mpc52xx_ata_of_platform_driver);
}

module_init(mpc52xx_ata_init);
module_exit(mpc52xx_ata_exit);

MODULE_AUTHOR("Sylvain Munaut <tnt@246tNt.com>");
MODULE_DESCRIPTION("Freescale MPC52xx IDE/ATA libata driver");
MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(of, mpc52xx_ata_of_match);

