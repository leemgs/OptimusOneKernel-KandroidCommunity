

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/scatterlist.h>
#include <linux/io.h>

#include <asm/system.h>
#include <asm/irq.h>
#include <mach/hardware.h>
#include <mach/dma-mx1-mx2.h>

#define DMA_DCR     0x00		
#define DMA_DISR    0x04		
#define DMA_DIMR    0x08		
#define DMA_DBTOSR  0x0c		
#define DMA_DRTOSR  0x10		
#define DMA_DSESR   0x14		
#define DMA_DBOSR   0x18		
#define DMA_DBTOCR  0x1c		
#define DMA_WSRA    0x40		
#define DMA_XSRA    0x44		
#define DMA_YSRA    0x48		
#define DMA_WSRB    0x4c		
#define DMA_XSRB    0x50		
#define DMA_YSRB    0x54		
#define DMA_SAR(x)  (0x80 + ((x) << 6))	
#define DMA_DAR(x)  (0x84 + ((x) << 6))	
#define DMA_CNTR(x) (0x88 + ((x) << 6))	
#define DMA_CCR(x)  (0x8c + ((x) << 6))	
#define DMA_RSSR(x) (0x90 + ((x) << 6))	
#define DMA_BLR(x)  (0x94 + ((x) << 6))	
#define DMA_RTOR(x) (0x98 + ((x) << 6))	
#define DMA_BUCR(x) (0x98 + ((x) << 6))	
#define DMA_CCNR(x) (0x9C + ((x) << 6))	

#define DCR_DRST           (1<<1)
#define DCR_DEN            (1<<0)
#define DBTOCR_EN          (1<<15)
#define DBTOCR_CNT(x)      ((x) & 0x7fff)
#define CNTR_CNT(x)        ((x) & 0xffffff)
#define CCR_ACRPT          (1<<14)
#define CCR_DMOD_LINEAR    (0x0 << 12)
#define CCR_DMOD_2D        (0x1 << 12)
#define CCR_DMOD_FIFO      (0x2 << 12)
#define CCR_DMOD_EOBFIFO   (0x3 << 12)
#define CCR_SMOD_LINEAR    (0x0 << 10)
#define CCR_SMOD_2D        (0x1 << 10)
#define CCR_SMOD_FIFO      (0x2 << 10)
#define CCR_SMOD_EOBFIFO   (0x3 << 10)
#define CCR_MDIR_DEC       (1<<9)
#define CCR_MSEL_B         (1<<8)
#define CCR_DSIZ_32        (0x0 << 6)
#define CCR_DSIZ_8         (0x1 << 6)
#define CCR_DSIZ_16        (0x2 << 6)
#define CCR_SSIZ_32        (0x0 << 4)
#define CCR_SSIZ_8         (0x1 << 4)
#define CCR_SSIZ_16        (0x2 << 4)
#define CCR_REN            (1<<3)
#define CCR_RPT            (1<<2)
#define CCR_FRC            (1<<1)
#define CCR_CEN            (1<<0)
#define RTOR_EN            (1<<15)
#define RTOR_CLK           (1<<14)
#define RTOR_PSC           (1<<13)



struct imx_dma_channel {
	const char *name;
	void (*irq_handler) (int, void *);
	void (*err_handler) (int, void *, int errcode);
	void (*prog_handler) (int, void *, struct scatterlist *);
	void *data;
	unsigned int dma_mode;
	struct scatterlist *sg;
	unsigned int resbytes;
	int dma_num;

	int in_use;

	u32 ccr_from_device;
	u32 ccr_to_device;

	struct timer_list watchdog;

	int hw_chaining;
};

static struct imx_dma_channel imx_dma_channels[IMX_DMA_CHANNELS];

static struct clk *dma_clk;

static int imx_dma_hw_chain(struct imx_dma_channel *imxdma)
{
	if (cpu_is_mx27())
		return imxdma->hw_chaining;
	else
		return 0;
}



static inline int imx_dma_sg_next(int channel, struct scatterlist *sg)
{
	struct imx_dma_channel *imxdma = &imx_dma_channels[channel];
	unsigned long now;

	if (!imxdma->name) {
		printk(KERN_CRIT "%s: called for  not allocated channel %d\n",
		       __func__, channel);
		return 0;
	}

	now = min(imxdma->resbytes, sg->length);
	imxdma->resbytes -= now;

	if ((imxdma->dma_mode & DMA_MODE_MASK) == DMA_MODE_READ)
		__raw_writel(sg->dma_address, DMA_BASE + DMA_DAR(channel));
	else
		__raw_writel(sg->dma_address, DMA_BASE + DMA_SAR(channel));

	__raw_writel(now, DMA_BASE + DMA_CNTR(channel));

	pr_debug("imxdma%d: next sg chunk dst 0x%08x, src 0x%08x, "
		"size 0x%08x\n", channel,
		 __raw_readl(DMA_BASE + DMA_DAR(channel)),
		 __raw_readl(DMA_BASE + DMA_SAR(channel)),
		 __raw_readl(DMA_BASE + DMA_CNTR(channel)));

	return now;
}


int
imx_dma_setup_single(int channel, dma_addr_t dma_address,
		     unsigned int dma_length, unsigned int dev_addr,
		     unsigned int dmamode)
{
	struct imx_dma_channel *imxdma = &imx_dma_channels[channel];

	imxdma->sg = NULL;
	imxdma->dma_mode = dmamode;

	if (!dma_address) {
		printk(KERN_ERR "imxdma%d: imx_dma_setup_single null address\n",
		       channel);
		return -EINVAL;
	}

	if (!dma_length) {
		printk(KERN_ERR "imxdma%d: imx_dma_setup_single zero length\n",
		       channel);
		return -EINVAL;
	}

	if ((dmamode & DMA_MODE_MASK) == DMA_MODE_READ) {
		pr_debug("imxdma%d: %s dma_addressg=0x%08x dma_length=%d "
			"dev_addr=0x%08x for read\n",
			channel, __func__, (unsigned int)dma_address,
			dma_length, dev_addr);

		__raw_writel(dev_addr, DMA_BASE + DMA_SAR(channel));
		__raw_writel(dma_address, DMA_BASE + DMA_DAR(channel));
		__raw_writel(imxdma->ccr_from_device,
				DMA_BASE + DMA_CCR(channel));
	} else if ((dmamode & DMA_MODE_MASK) == DMA_MODE_WRITE) {
		pr_debug("imxdma%d: %s dma_addressg=0x%08x dma_length=%d "
			"dev_addr=0x%08x for write\n",
			channel, __func__, (unsigned int)dma_address,
			dma_length, dev_addr);

		__raw_writel(dma_address, DMA_BASE + DMA_SAR(channel));
		__raw_writel(dev_addr, DMA_BASE + DMA_DAR(channel));
		__raw_writel(imxdma->ccr_to_device,
				DMA_BASE + DMA_CCR(channel));
	} else {
		printk(KERN_ERR "imxdma%d: imx_dma_setup_single bad dmamode\n",
		       channel);
		return -EINVAL;
	}

	__raw_writel(dma_length, DMA_BASE + DMA_CNTR(channel));

	return 0;
}
EXPORT_SYMBOL(imx_dma_setup_single);


int
imx_dma_setup_sg(int channel,
		 struct scatterlist *sg, unsigned int sgcount,
		 unsigned int dma_length, unsigned int dev_addr,
		 unsigned int dmamode)
{
	struct imx_dma_channel *imxdma = &imx_dma_channels[channel];

	if (imxdma->in_use)
		return -EBUSY;

	imxdma->sg = sg;
	imxdma->dma_mode = dmamode;
	imxdma->resbytes = dma_length;

	if (!sg || !sgcount) {
		printk(KERN_ERR "imxdma%d: imx_dma_setup_sg epty sg list\n",
		       channel);
		return -EINVAL;
	}

	if (!sg->length) {
		printk(KERN_ERR "imxdma%d: imx_dma_setup_sg zero length\n",
		       channel);
		return -EINVAL;
	}

	if ((dmamode & DMA_MODE_MASK) == DMA_MODE_READ) {
		pr_debug("imxdma%d: %s sg=%p sgcount=%d total length=%d "
			"dev_addr=0x%08x for read\n",
			channel, __func__, sg, sgcount, dma_length, dev_addr);

		__raw_writel(dev_addr, DMA_BASE + DMA_SAR(channel));
		__raw_writel(imxdma->ccr_from_device,
				DMA_BASE + DMA_CCR(channel));
	} else if ((dmamode & DMA_MODE_MASK) == DMA_MODE_WRITE) {
		pr_debug("imxdma%d: %s sg=%p sgcount=%d total length=%d "
			"dev_addr=0x%08x for write\n",
			channel, __func__, sg, sgcount, dma_length, dev_addr);

		__raw_writel(dev_addr, DMA_BASE + DMA_DAR(channel));
		__raw_writel(imxdma->ccr_to_device,
				DMA_BASE + DMA_CCR(channel));
	} else {
		printk(KERN_ERR "imxdma%d: imx_dma_setup_sg bad dmamode\n",
		       channel);
		return -EINVAL;
	}

	imx_dma_sg_next(channel, sg);

	return 0;
}
EXPORT_SYMBOL(imx_dma_setup_sg);

int
imx_dma_config_channel(int channel, unsigned int config_port,
	unsigned int config_mem, unsigned int dmareq, int hw_chaining)
{
	struct imx_dma_channel *imxdma = &imx_dma_channels[channel];
	u32 dreq = 0;

	imxdma->hw_chaining = 0;

	if (hw_chaining) {
		imxdma->hw_chaining = 1;
		if (!imx_dma_hw_chain(imxdma))
			return -EINVAL;
	}

	if (dmareq)
		dreq = CCR_REN;

	imxdma->ccr_from_device = config_port | (config_mem << 2) | dreq;
	imxdma->ccr_to_device = config_mem | (config_port << 2) | dreq;

	__raw_writel(dmareq, DMA_BASE + DMA_RSSR(channel));

	return 0;
}
EXPORT_SYMBOL(imx_dma_config_channel);

void imx_dma_config_burstlen(int channel, unsigned int burstlen)
{
	__raw_writel(burstlen, DMA_BASE + DMA_BLR(channel));
}
EXPORT_SYMBOL(imx_dma_config_burstlen);


int
imx_dma_setup_handlers(int channel,
		       void (*irq_handler) (int, void *),
		       void (*err_handler) (int, void *, int),
		       void *data)
{
	struct imx_dma_channel *imxdma = &imx_dma_channels[channel];
	unsigned long flags;

	if (!imxdma->name) {
		printk(KERN_CRIT "%s: called for  not allocated channel %d\n",
		       __func__, channel);
		return -ENODEV;
	}

	local_irq_save(flags);
	__raw_writel(1 << channel, DMA_BASE + DMA_DISR);
	imxdma->irq_handler = irq_handler;
	imxdma->err_handler = err_handler;
	imxdma->data = data;
	local_irq_restore(flags);
	return 0;
}
EXPORT_SYMBOL(imx_dma_setup_handlers);


int
imx_dma_setup_progression_handler(int channel,
			void (*prog_handler) (int, void*, struct scatterlist*))
{
	struct imx_dma_channel *imxdma = &imx_dma_channels[channel];
	unsigned long flags;

	if (!imxdma->name) {
		printk(KERN_CRIT "%s: called for  not allocated channel %d\n",
		       __func__, channel);
		return -ENODEV;
	}

	local_irq_save(flags);
	imxdma->prog_handler = prog_handler;
	local_irq_restore(flags);
	return 0;
}
EXPORT_SYMBOL(imx_dma_setup_progression_handler);


void imx_dma_enable(int channel)
{
	struct imx_dma_channel *imxdma = &imx_dma_channels[channel];
	unsigned long flags;

	pr_debug("imxdma%d: imx_dma_enable\n", channel);

	if (!imxdma->name) {
		printk(KERN_CRIT "%s: called for  not allocated channel %d\n",
		       __func__, channel);
		return;
	}

	if (imxdma->in_use)
		return;

	local_irq_save(flags);

	__raw_writel(1 << channel, DMA_BASE + DMA_DISR);
	__raw_writel(__raw_readl(DMA_BASE + DMA_DIMR) & ~(1 << channel),
		DMA_BASE + DMA_DIMR);
	__raw_writel(__raw_readl(DMA_BASE + DMA_CCR(channel)) | CCR_CEN |
		CCR_ACRPT,
		DMA_BASE + DMA_CCR(channel));

#ifdef CONFIG_ARCH_MX2
	if (imxdma->sg && imx_dma_hw_chain(imxdma)) {
		imxdma->sg = sg_next(imxdma->sg);
		if (imxdma->sg) {
			u32 tmp;
			imx_dma_sg_next(channel, imxdma->sg);
			tmp = __raw_readl(DMA_BASE + DMA_CCR(channel));
			__raw_writel(tmp | CCR_RPT | CCR_ACRPT,
				DMA_BASE + DMA_CCR(channel));
		}
	}
#endif
	imxdma->in_use = 1;

	local_irq_restore(flags);
}
EXPORT_SYMBOL(imx_dma_enable);


void imx_dma_disable(int channel)
{
	struct imx_dma_channel *imxdma = &imx_dma_channels[channel];
	unsigned long flags;

	pr_debug("imxdma%d: imx_dma_disable\n", channel);

	if (imx_dma_hw_chain(imxdma))
		del_timer(&imxdma->watchdog);

	local_irq_save(flags);
	__raw_writel(__raw_readl(DMA_BASE + DMA_DIMR) | (1 << channel),
		DMA_BASE + DMA_DIMR);
	__raw_writel(__raw_readl(DMA_BASE + DMA_CCR(channel)) & ~CCR_CEN,
		DMA_BASE + DMA_CCR(channel));
	__raw_writel(1 << channel, DMA_BASE + DMA_DISR);
	imxdma->in_use = 0;
	local_irq_restore(flags);
}
EXPORT_SYMBOL(imx_dma_disable);

#ifdef CONFIG_ARCH_MX2
static void imx_dma_watchdog(unsigned long chno)
{
	struct imx_dma_channel *imxdma = &imx_dma_channels[chno];

	__raw_writel(0, DMA_BASE + DMA_CCR(chno));
	imxdma->in_use = 0;
	imxdma->sg = NULL;

	if (imxdma->err_handler)
		imxdma->err_handler(chno, imxdma->data, IMX_DMA_ERR_TIMEOUT);
}
#endif

static irqreturn_t dma_err_handler(int irq, void *dev_id)
{
	int i, disr;
	struct imx_dma_channel *imxdma;
	unsigned int err_mask;
	int errcode;

	disr = __raw_readl(DMA_BASE + DMA_DISR);

	err_mask = __raw_readl(DMA_BASE + DMA_DBTOSR) |
		   __raw_readl(DMA_BASE + DMA_DRTOSR) |
		   __raw_readl(DMA_BASE + DMA_DSESR)  |
		   __raw_readl(DMA_BASE + DMA_DBOSR);

	if (!err_mask)
		return IRQ_HANDLED;

	__raw_writel(disr & err_mask, DMA_BASE + DMA_DISR);

	for (i = 0; i < IMX_DMA_CHANNELS; i++) {
		if (!(err_mask & (1 << i)))
			continue;
		imxdma = &imx_dma_channels[i];
		errcode = 0;

		if (__raw_readl(DMA_BASE + DMA_DBTOSR) & (1 << i)) {
			__raw_writel(1 << i, DMA_BASE + DMA_DBTOSR);
			errcode |= IMX_DMA_ERR_BURST;
		}
		if (__raw_readl(DMA_BASE + DMA_DRTOSR) & (1 << i)) {
			__raw_writel(1 << i, DMA_BASE + DMA_DRTOSR);
			errcode |= IMX_DMA_ERR_REQUEST;
		}
		if (__raw_readl(DMA_BASE + DMA_DSESR) & (1 << i)) {
			__raw_writel(1 << i, DMA_BASE + DMA_DSESR);
			errcode |= IMX_DMA_ERR_TRANSFER;
		}
		if (__raw_readl(DMA_BASE + DMA_DBOSR) & (1 << i)) {
			__raw_writel(1 << i, DMA_BASE + DMA_DBOSR);
			errcode |= IMX_DMA_ERR_BUFFER;
		}
		if (imxdma->name && imxdma->err_handler) {
			imxdma->err_handler(i, imxdma->data, errcode);
			continue;
		}

		imx_dma_channels[i].sg = NULL;

		printk(KERN_WARNING
		       "DMA timeout on channel %d (%s) -%s%s%s%s\n",
		       i, imxdma->name,
		       errcode & IMX_DMA_ERR_BURST ?    " burst" : "",
		       errcode & IMX_DMA_ERR_REQUEST ?  " request" : "",
		       errcode & IMX_DMA_ERR_TRANSFER ? " transfer" : "",
		       errcode & IMX_DMA_ERR_BUFFER ?   " buffer" : "");
	}
	return IRQ_HANDLED;
}

static void dma_irq_handle_channel(int chno)
{
	struct imx_dma_channel *imxdma = &imx_dma_channels[chno];

	if (!imxdma->name) {
		
		printk(KERN_WARNING
		       "spurious IRQ for DMA channel %d\n", chno);
		return;
	}

	if (imxdma->sg) {
		u32 tmp;
		struct scatterlist *current_sg = imxdma->sg;
		imxdma->sg = sg_next(imxdma->sg);

		if (imxdma->sg) {
			imx_dma_sg_next(chno, imxdma->sg);

			tmp = __raw_readl(DMA_BASE + DMA_CCR(chno));

			if (imx_dma_hw_chain(imxdma)) {
				
				mod_timer(&imxdma->watchdog,
					jiffies + msecs_to_jiffies(500));

				tmp |= CCR_CEN | CCR_RPT | CCR_ACRPT;
				__raw_writel(tmp, DMA_BASE +
						DMA_CCR(chno));
			} else {
				__raw_writel(tmp & ~CCR_CEN, DMA_BASE +
						DMA_CCR(chno));
				tmp |= CCR_CEN;
			}

			__raw_writel(tmp, DMA_BASE + DMA_CCR(chno));

			if (imxdma->prog_handler)
				imxdma->prog_handler(chno, imxdma->data,
						current_sg);

			return;
		}

		if (imx_dma_hw_chain(imxdma)) {
			del_timer(&imxdma->watchdog);
			return;
		}
	}

	__raw_writel(0, DMA_BASE + DMA_CCR(chno));
	imxdma->in_use = 0;
	if (imxdma->irq_handler)
		imxdma->irq_handler(chno, imxdma->data);
}

static irqreturn_t dma_irq_handler(int irq, void *dev_id)
{
	int i, disr;

#ifdef CONFIG_ARCH_MX2
	dma_err_handler(irq, dev_id);
#endif

	disr = __raw_readl(DMA_BASE + DMA_DISR);

	pr_debug("imxdma: dma_irq_handler called, disr=0x%08x\n",
		     disr);

	__raw_writel(disr, DMA_BASE + DMA_DISR);
	for (i = 0; i < IMX_DMA_CHANNELS; i++) {
		if (disr & (1 << i))
			dma_irq_handle_channel(i);
	}

	return IRQ_HANDLED;
}


int imx_dma_request(int channel, const char *name)
{
	struct imx_dma_channel *imxdma = &imx_dma_channels[channel];
	unsigned long flags;
	int ret = 0;

	
	if (!name)
		return -EINVAL;

	if (channel >= IMX_DMA_CHANNELS) {
		printk(KERN_CRIT "%s: called for  non-existed channel %d\n",
		       __func__, channel);
		return -EINVAL;
	}

	local_irq_save(flags);
	if (imxdma->name) {
		local_irq_restore(flags);
		return -EBUSY;
	}
	memset(imxdma, 0, sizeof(imxdma));
	imxdma->name = name;
	local_irq_restore(flags); 

#ifdef CONFIG_ARCH_MX2
	ret = request_irq(MXC_INT_DMACH0 + channel, dma_irq_handler, 0, "DMA",
			NULL);
	if (ret) {
		imxdma->name = NULL;
		printk(KERN_CRIT "Can't register IRQ %d for DMA channel %d\n",
				MXC_INT_DMACH0 + channel, channel);
		return ret;
	}
	init_timer(&imxdma->watchdog);
	imxdma->watchdog.function = &imx_dma_watchdog;
	imxdma->watchdog.data = channel;
#endif

	return ret;
}
EXPORT_SYMBOL(imx_dma_request);


void imx_dma_free(int channel)
{
	unsigned long flags;
	struct imx_dma_channel *imxdma = &imx_dma_channels[channel];

	if (!imxdma->name) {
		printk(KERN_CRIT
		       "%s: trying to free free channel %d\n",
		       __func__, channel);
		return;
	}

	local_irq_save(flags);
	
	imx_dma_disable(channel);
	imxdma->name = NULL;

#ifdef CONFIG_ARCH_MX2
	free_irq(MXC_INT_DMACH0 + channel, NULL);
#endif

	local_irq_restore(flags);
}
EXPORT_SYMBOL(imx_dma_free);


int imx_dma_request_by_prio(const char *name, enum imx_dma_prio prio)
{
	int i;
	int best;

	switch (prio) {
	case (DMA_PRIO_HIGH):
		best = 8;
		break;
	case (DMA_PRIO_MEDIUM):
		best = 4;
		break;
	case (DMA_PRIO_LOW):
	default:
		best = 0;
		break;
	}

	for (i = best; i < IMX_DMA_CHANNELS; i++)
		if (!imx_dma_request(i, name))
			return i;

	for (i = best - 1; i >= 0; i--)
		if (!imx_dma_request(i, name))
			return i;

	printk(KERN_ERR "%s: no free DMA channel found\n", __func__);

	return -ENODEV;
}
EXPORT_SYMBOL(imx_dma_request_by_prio);

static int __init imx_dma_init(void)
{
	int ret = 0;
	int i;

	dma_clk = clk_get(NULL, "dma");
	clk_enable(dma_clk);

	
	__raw_writel(DCR_DRST, DMA_BASE + DMA_DCR);

#ifdef CONFIG_ARCH_MX1
	ret = request_irq(DMA_INT, dma_irq_handler, 0, "DMA", NULL);
	if (ret) {
		printk(KERN_CRIT "Wow!  Can't register IRQ for DMA\n");
		return ret;
	}

	ret = request_irq(DMA_ERR, dma_err_handler, 0, "DMA", NULL);
	if (ret) {
		printk(KERN_CRIT "Wow!  Can't register ERRIRQ for DMA\n");
		free_irq(DMA_INT, NULL);
		return ret;
	}
#endif
	
	__raw_writel(DCR_DEN, DMA_BASE + DMA_DCR);

	
	__raw_writel((1 << IMX_DMA_CHANNELS) - 1, DMA_BASE + DMA_DISR);

	
	__raw_writel((1 << IMX_DMA_CHANNELS) - 1, DMA_BASE + DMA_DIMR);

	for (i = 0; i < IMX_DMA_CHANNELS; i++) {
		imx_dma_channels[i].sg = NULL;
		imx_dma_channels[i].dma_num = i;
	}

	return ret;
}

arch_initcall(imx_dma_init);
