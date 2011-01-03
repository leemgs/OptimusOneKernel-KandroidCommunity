

#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>

#include <asm/system.h>
#include <mach/hardware.h>
#include <mach/dma.h>

#include <mach/tc.h>

#undef DEBUG

#ifndef CONFIG_ARCH_OMAP1
enum { DMA_CH_ALLOC_DONE, DMA_CH_PARAMS_SET_DONE, DMA_CH_STARTED,
	DMA_CH_QUEUED, DMA_CH_NOTSTARTED, DMA_CH_PAUSED, DMA_CH_LINK_ENABLED
};

enum { DMA_CHAIN_STARTED, DMA_CHAIN_NOTSTARTED };
#endif

#define OMAP_DMA_ACTIVE			0x01
#define OMAP_DMA_CCR_EN			(1 << 7)
#define OMAP2_DMA_CSR_CLEAR_MASK	0xffe

#define OMAP_FUNC_MUX_ARM_BASE		(0xfffe1000 + 0xec)

static int enable_1510_mode;

struct omap_dma_lch {
	int next_lch;
	int dev_id;
	u16 saved_csr;
	u16 enabled_irqs;
	const char *dev_name;
	void (*callback)(int lch, u16 ch_status, void *data);
	void *data;

#ifndef CONFIG_ARCH_OMAP1
	
	int prev_linked_ch;
	int next_linked_ch;
	int state;
	int chain_id;

	int status;
#endif
	long flags;
};

struct dma_link_info {
	int *linked_dmach_q;
	int no_of_lchs_linked;

	int q_count;
	int q_tail;
	int q_head;

	int chain_state;
	int chain_mode;

};

static struct dma_link_info *dma_linked_lch;

#ifndef CONFIG_ARCH_OMAP1


#define OMAP_DMA_CHAIN_QINIT(chain_id)					\
	do {								\
		dma_linked_lch[chain_id].q_head =			\
		dma_linked_lch[chain_id].q_tail =			\
		dma_linked_lch[chain_id].q_count = 0;			\
	} while (0)
#define OMAP_DMA_CHAIN_QFULL(chain_id)					\
		(dma_linked_lch[chain_id].no_of_lchs_linked ==		\
		dma_linked_lch[chain_id].q_count)
#define OMAP_DMA_CHAIN_QLAST(chain_id)					\
	do {								\
		((dma_linked_lch[chain_id].no_of_lchs_linked-1) ==	\
		dma_linked_lch[chain_id].q_count)			\
	} while (0)
#define OMAP_DMA_CHAIN_QEMPTY(chain_id)					\
		(0 == dma_linked_lch[chain_id].q_count)
#define __OMAP_DMA_CHAIN_INCQ(end)					\
	((end) = ((end)+1) % dma_linked_lch[chain_id].no_of_lchs_linked)
#define OMAP_DMA_CHAIN_INCQHEAD(chain_id)				\
	do {								\
		__OMAP_DMA_CHAIN_INCQ(dma_linked_lch[chain_id].q_head);	\
		dma_linked_lch[chain_id].q_count--;			\
	} while (0)

#define OMAP_DMA_CHAIN_INCQTAIL(chain_id)				\
	do {								\
		__OMAP_DMA_CHAIN_INCQ(dma_linked_lch[chain_id].q_tail);	\
		dma_linked_lch[chain_id].q_count++; \
	} while (0)
#endif

static int dma_lch_count;
static int dma_chan_count;
static int omap_dma_reserve_channels;

static spinlock_t dma_chan_lock;
static struct omap_dma_lch *dma_chan;
static void __iomem *omap_dma_base;

static const u8 omap1_dma_irq[OMAP1_LOGICAL_DMA_CH_COUNT] = {
	INT_DMA_CH0_6, INT_DMA_CH1_7, INT_DMA_CH2_8, INT_DMA_CH3,
	INT_DMA_CH4, INT_DMA_CH5, INT_1610_DMA_CH6, INT_1610_DMA_CH7,
	INT_1610_DMA_CH8, INT_1610_DMA_CH9, INT_1610_DMA_CH10,
	INT_1610_DMA_CH11, INT_1610_DMA_CH12, INT_1610_DMA_CH13,
	INT_1610_DMA_CH14, INT_1610_DMA_CH15, INT_DMA_LCD
};

static inline void disable_lnk(int lch);
static void omap_disable_channel_irq(int lch);
static inline void omap_enable_channel_irq(int lch);

#define REVISIT_24XX()		printk(KERN_ERR "FIXME: no %s on 24xx\n", \
						__func__);

#define dma_read(reg)							\
({									\
	u32 __val;							\
	if (cpu_class_is_omap1())					\
		__val = __raw_readw(omap_dma_base + OMAP1_DMA_##reg);	\
	else								\
		__val = __raw_readl(omap_dma_base + OMAP_DMA4_##reg);	\
	__val;								\
})

#define dma_write(val, reg)						\
({									\
	if (cpu_class_is_omap1())					\
		__raw_writew((u16)(val), omap_dma_base + OMAP1_DMA_##reg); \
	else								\
		__raw_writel((val), omap_dma_base + OMAP_DMA4_##reg);	\
})

#ifdef CONFIG_ARCH_OMAP15XX

int omap_dma_in_1510_mode(void)
{
	return enable_1510_mode;
}
#else
#define omap_dma_in_1510_mode()		0
#endif

#ifdef CONFIG_ARCH_OMAP1
static inline int get_gdma_dev(int req)
{
	u32 reg = OMAP_FUNC_MUX_ARM_BASE + ((req - 1) / 5) * 4;
	int shift = ((req - 1) % 5) * 6;

	return ((omap_readl(reg) >> shift) & 0x3f) + 1;
}

static inline void set_gdma_dev(int req, int dev)
{
	u32 reg = OMAP_FUNC_MUX_ARM_BASE + ((req - 1) / 5) * 4;
	int shift = ((req - 1) % 5) * 6;
	u32 l;

	l = omap_readl(reg);
	l &= ~(0x3f << shift);
	l |= (dev - 1) << shift;
	omap_writel(l, reg);
}
#else
#define set_gdma_dev(req, dev)	do {} while (0)
#endif


static void clear_lch_regs(int lch)
{
	int i;
	void __iomem *lch_base = omap_dma_base + OMAP1_DMA_CH_BASE(lch);

	for (i = 0; i < 0x2c; i += 2)
		__raw_writew(0, lch_base + i);
}

void omap_set_dma_priority(int lch, int dst_port, int priority)
{
	unsigned long reg;
	u32 l;

	if (cpu_class_is_omap1()) {
		switch (dst_port) {
		case OMAP_DMA_PORT_OCP_T1:	
			reg = OMAP_TC_OCPT1_PRIOR;
			break;
		case OMAP_DMA_PORT_OCP_T2:	
			reg = OMAP_TC_OCPT2_PRIOR;
			break;
		case OMAP_DMA_PORT_EMIFF:	
			reg = OMAP_TC_EMIFF_PRIOR;
			break;
		case OMAP_DMA_PORT_EMIFS:	
			reg = OMAP_TC_EMIFS_PRIOR;
			break;
		default:
			BUG();
			return;
		}
		l = omap_readl(reg);
		l &= ~(0xf << 8);
		l |= (priority & 0xf) << 8;
		omap_writel(l, reg);
	}

	if (cpu_class_is_omap2()) {
		u32 ccr;

		ccr = dma_read(CCR(lch));
		if (priority)
			ccr |= (1 << 6);
		else
			ccr &= ~(1 << 6);
		dma_write(ccr, CCR(lch));
	}
}
EXPORT_SYMBOL(omap_set_dma_priority);

void omap_set_dma_transfer_params(int lch, int data_type, int elem_count,
				  int frame_count, int sync_mode,
				  int dma_trigger, int src_or_dst_synch)
{
	u32 l;

	l = dma_read(CSDP(lch));
	l &= ~0x03;
	l |= data_type;
	dma_write(l, CSDP(lch));

	if (cpu_class_is_omap1()) {
		u16 ccr;

		ccr = dma_read(CCR(lch));
		ccr &= ~(1 << 5);
		if (sync_mode == OMAP_DMA_SYNC_FRAME)
			ccr |= 1 << 5;
		dma_write(ccr, CCR(lch));

		ccr = dma_read(CCR2(lch));
		ccr &= ~(1 << 2);
		if (sync_mode == OMAP_DMA_SYNC_BLOCK)
			ccr |= 1 << 2;
		dma_write(ccr, CCR2(lch));
	}

	if (cpu_class_is_omap2() && dma_trigger) {
		u32 val;

		val = dma_read(CCR(lch));

		
		val &= ~((3 << 19) | 0x1f);
		val |= (dma_trigger & ~0x1f) << 14;
		val |= dma_trigger & 0x1f;

		if (sync_mode & OMAP_DMA_SYNC_FRAME)
			val |= 1 << 5;
		else
			val &= ~(1 << 5);

		if (sync_mode & OMAP_DMA_SYNC_BLOCK)
			val |= 1 << 18;
		else
			val &= ~(1 << 18);

		if (src_or_dst_synch)
			val |= 1 << 24;		
		else
			val &= ~(1 << 24);	

		dma_write(val, CCR(lch));
	}

	dma_write(elem_count, CEN(lch));
	dma_write(frame_count, CFN(lch));
}
EXPORT_SYMBOL(omap_set_dma_transfer_params);

void omap_set_dma_color_mode(int lch, enum omap_dma_color_mode mode, u32 color)
{
	BUG_ON(omap_dma_in_1510_mode());

	if (cpu_class_is_omap1()) {
		u16 w;

		w = dma_read(CCR2(lch));
		w &= ~0x03;

		switch (mode) {
		case OMAP_DMA_CONSTANT_FILL:
			w |= 0x01;
			break;
		case OMAP_DMA_TRANSPARENT_COPY:
			w |= 0x02;
			break;
		case OMAP_DMA_COLOR_DIS:
			break;
		default:
			BUG();
		}
		dma_write(w, CCR2(lch));

		w = dma_read(LCH_CTRL(lch));
		w &= ~0x0f;
		
		if (mode) {
			dma_write((u16)color, COLOR_L(lch));
			dma_write((u16)(color >> 16), COLOR_U(lch));
			w |= 1;		
		}
		dma_write(w, LCH_CTRL(lch));
	}

	if (cpu_class_is_omap2()) {
		u32 val;

		val = dma_read(CCR(lch));
		val &= ~((1 << 17) | (1 << 16));

		switch (mode) {
		case OMAP_DMA_CONSTANT_FILL:
			val |= 1 << 16;
			break;
		case OMAP_DMA_TRANSPARENT_COPY:
			val |= 1 << 17;
			break;
		case OMAP_DMA_COLOR_DIS:
			break;
		default:
			BUG();
		}
		dma_write(val, CCR(lch));

		color &= 0xffffff;
		dma_write(color, COLOR(lch));
	}
}
EXPORT_SYMBOL(omap_set_dma_color_mode);

void omap_set_dma_write_mode(int lch, enum omap_dma_write_mode mode)
{
	if (cpu_class_is_omap2()) {
		u32 csdp;

		csdp = dma_read(CSDP(lch));
		csdp &= ~(0x3 << 16);
		csdp |= (mode << 16);
		dma_write(csdp, CSDP(lch));
	}
}
EXPORT_SYMBOL(omap_set_dma_write_mode);

void omap_set_dma_channel_mode(int lch, enum omap_dma_channel_mode mode)
{
	if (cpu_class_is_omap1() && !cpu_is_omap15xx()) {
		u32 l;

		l = dma_read(LCH_CTRL(lch));
		l &= ~0x7;
		l |= mode;
		dma_write(l, LCH_CTRL(lch));
	}
}
EXPORT_SYMBOL(omap_set_dma_channel_mode);


void omap_set_dma_src_params(int lch, int src_port, int src_amode,
			     unsigned long src_start,
			     int src_ei, int src_fi)
{
	u32 l;

	if (cpu_class_is_omap1()) {
		u16 w;

		w = dma_read(CSDP(lch));
		w &= ~(0x1f << 2);
		w |= src_port << 2;
		dma_write(w, CSDP(lch));
	}

	l = dma_read(CCR(lch));
	l &= ~(0x03 << 12);
	l |= src_amode << 12;
	dma_write(l, CCR(lch));

	if (cpu_class_is_omap1()) {
		dma_write(src_start >> 16, CSSA_U(lch));
		dma_write((u16)src_start, CSSA_L(lch));
	}

	if (cpu_class_is_omap2())
		dma_write(src_start, CSSA(lch));

	dma_write(src_ei, CSEI(lch));
	dma_write(src_fi, CSFI(lch));
}
EXPORT_SYMBOL(omap_set_dma_src_params);

void omap_set_dma_params(int lch, struct omap_dma_channel_params *params)
{
	omap_set_dma_transfer_params(lch, params->data_type,
				     params->elem_count, params->frame_count,
				     params->sync_mode, params->trigger,
				     params->src_or_dst_synch);
	omap_set_dma_src_params(lch, params->src_port,
				params->src_amode, params->src_start,
				params->src_ei, params->src_fi);

	omap_set_dma_dest_params(lch, params->dst_port,
				 params->dst_amode, params->dst_start,
				 params->dst_ei, params->dst_fi);
	if (params->read_prio || params->write_prio)
		omap_dma_set_prio_lch(lch, params->read_prio,
				      params->write_prio);
}
EXPORT_SYMBOL(omap_set_dma_params);

void omap_set_dma_src_index(int lch, int eidx, int fidx)
{
	if (cpu_class_is_omap2())
		return;

	dma_write(eidx, CSEI(lch));
	dma_write(fidx, CSFI(lch));
}
EXPORT_SYMBOL(omap_set_dma_src_index);

void omap_set_dma_src_data_pack(int lch, int enable)
{
	u32 l;

	l = dma_read(CSDP(lch));
	l &= ~(1 << 6);
	if (enable)
		l |= (1 << 6);
	dma_write(l, CSDP(lch));
}
EXPORT_SYMBOL(omap_set_dma_src_data_pack);

void omap_set_dma_src_burst_mode(int lch, enum omap_dma_burst_mode burst_mode)
{
	unsigned int burst = 0;
	u32 l;

	l = dma_read(CSDP(lch));
	l &= ~(0x03 << 7);

	switch (burst_mode) {
	case OMAP_DMA_DATA_BURST_DIS:
		break;
	case OMAP_DMA_DATA_BURST_4:
		if (cpu_class_is_omap2())
			burst = 0x1;
		else
			burst = 0x2;
		break;
	case OMAP_DMA_DATA_BURST_8:
		if (cpu_class_is_omap2()) {
			burst = 0x2;
			break;
		}
		
	case OMAP_DMA_DATA_BURST_16:
		if (cpu_class_is_omap2()) {
			burst = 0x3;
			break;
		}
		
	default:
		BUG();
	}

	l |= (burst << 7);
	dma_write(l, CSDP(lch));
}
EXPORT_SYMBOL(omap_set_dma_src_burst_mode);


void omap_set_dma_dest_params(int lch, int dest_port, int dest_amode,
			      unsigned long dest_start,
			      int dst_ei, int dst_fi)
{
	u32 l;

	if (cpu_class_is_omap1()) {
		l = dma_read(CSDP(lch));
		l &= ~(0x1f << 9);
		l |= dest_port << 9;
		dma_write(l, CSDP(lch));
	}

	l = dma_read(CCR(lch));
	l &= ~(0x03 << 14);
	l |= dest_amode << 14;
	dma_write(l, CCR(lch));

	if (cpu_class_is_omap1()) {
		dma_write(dest_start >> 16, CDSA_U(lch));
		dma_write(dest_start, CDSA_L(lch));
	}

	if (cpu_class_is_omap2())
		dma_write(dest_start, CDSA(lch));

	dma_write(dst_ei, CDEI(lch));
	dma_write(dst_fi, CDFI(lch));
}
EXPORT_SYMBOL(omap_set_dma_dest_params);

void omap_set_dma_dest_index(int lch, int eidx, int fidx)
{
	if (cpu_class_is_omap2())
		return;

	dma_write(eidx, CDEI(lch));
	dma_write(fidx, CDFI(lch));
}
EXPORT_SYMBOL(omap_set_dma_dest_index);

void omap_set_dma_dest_data_pack(int lch, int enable)
{
	u32 l;

	l = dma_read(CSDP(lch));
	l &= ~(1 << 13);
	if (enable)
		l |= 1 << 13;
	dma_write(l, CSDP(lch));
}
EXPORT_SYMBOL(omap_set_dma_dest_data_pack);

void omap_set_dma_dest_burst_mode(int lch, enum omap_dma_burst_mode burst_mode)
{
	unsigned int burst = 0;
	u32 l;

	l = dma_read(CSDP(lch));
	l &= ~(0x03 << 14);

	switch (burst_mode) {
	case OMAP_DMA_DATA_BURST_DIS:
		break;
	case OMAP_DMA_DATA_BURST_4:
		if (cpu_class_is_omap2())
			burst = 0x1;
		else
			burst = 0x2;
		break;
	case OMAP_DMA_DATA_BURST_8:
		if (cpu_class_is_omap2())
			burst = 0x2;
		else
			burst = 0x3;
		break;
	case OMAP_DMA_DATA_BURST_16:
		if (cpu_class_is_omap2()) {
			burst = 0x3;
			break;
		}
		
	default:
		printk(KERN_ERR "Invalid DMA burst mode\n");
		BUG();
		return;
	}
	l |= (burst << 14);
	dma_write(l, CSDP(lch));
}
EXPORT_SYMBOL(omap_set_dma_dest_burst_mode);

static inline void omap_enable_channel_irq(int lch)
{
	u32 status;

	
	if (cpu_class_is_omap1())
		status = dma_read(CSR(lch));
	else if (cpu_class_is_omap2())
		dma_write(OMAP2_DMA_CSR_CLEAR_MASK, CSR(lch));

	
	dma_write(dma_chan[lch].enabled_irqs, CICR(lch));
}

static void omap_disable_channel_irq(int lch)
{
	if (cpu_class_is_omap2())
		dma_write(0, CICR(lch));
}

void omap_enable_dma_irq(int lch, u16 bits)
{
	dma_chan[lch].enabled_irqs |= bits;
}
EXPORT_SYMBOL(omap_enable_dma_irq);

void omap_disable_dma_irq(int lch, u16 bits)
{
	dma_chan[lch].enabled_irqs &= ~bits;
}
EXPORT_SYMBOL(omap_disable_dma_irq);

static inline void enable_lnk(int lch)
{
	u32 l;

	l = dma_read(CLNK_CTRL(lch));

	if (cpu_class_is_omap1())
		l &= ~(1 << 14);

	
	if (dma_chan[lch].next_lch != -1)
		l = dma_chan[lch].next_lch | (1 << 15);

#ifndef CONFIG_ARCH_OMAP1
	if (cpu_class_is_omap2())
		if (dma_chan[lch].next_linked_ch != -1)
			l = dma_chan[lch].next_linked_ch | (1 << 15);
#endif

	dma_write(l, CLNK_CTRL(lch));
}

static inline void disable_lnk(int lch)
{
	u32 l;

	l = dma_read(CLNK_CTRL(lch));

	
	if (cpu_class_is_omap1()) {
		dma_write(0, CICR(lch));
		
		l |= 1 << 14;
	}

	if (cpu_class_is_omap2()) {
		omap_disable_channel_irq(lch);
		
		l &= ~(1 << 15);
	}

	dma_write(l, CLNK_CTRL(lch));
	dma_chan[lch].flags &= ~OMAP_DMA_ACTIVE;
}

static inline void omap2_enable_irq_lch(int lch)
{
	u32 val;
	unsigned long flags;

	if (!cpu_class_is_omap2())
		return;

	spin_lock_irqsave(&dma_chan_lock, flags);
	val = dma_read(IRQENABLE_L0);
	val |= 1 << lch;
	dma_write(val, IRQENABLE_L0);
	spin_unlock_irqrestore(&dma_chan_lock, flags);
}

int omap_request_dma(int dev_id, const char *dev_name,
		     void (*callback)(int lch, u16 ch_status, void *data),
		     void *data, int *dma_ch_out)
{
	int ch, free_ch = -1;
	unsigned long flags;
	struct omap_dma_lch *chan;

	spin_lock_irqsave(&dma_chan_lock, flags);
	for (ch = 0; ch < dma_chan_count; ch++) {
		if (free_ch == -1 && dma_chan[ch].dev_id == -1) {
			free_ch = ch;
			if (dev_id == 0)
				break;
		}
	}
	if (free_ch == -1) {
		spin_unlock_irqrestore(&dma_chan_lock, flags);
		return -EBUSY;
	}
	chan = dma_chan + free_ch;
	chan->dev_id = dev_id;

	if (cpu_class_is_omap1())
		clear_lch_regs(free_ch);

	if (cpu_class_is_omap2())
		omap_clear_dma(free_ch);

	spin_unlock_irqrestore(&dma_chan_lock, flags);

	chan->dev_name = dev_name;
	chan->callback = callback;
	chan->data = data;
	chan->flags = 0;

#ifndef CONFIG_ARCH_OMAP1
	if (cpu_class_is_omap2()) {
		chan->chain_id = -1;
		chan->next_linked_ch = -1;
	}
#endif

	chan->enabled_irqs = OMAP_DMA_DROP_IRQ | OMAP_DMA_BLOCK_IRQ;

	if (cpu_class_is_omap1())
		chan->enabled_irqs |= OMAP1_DMA_TOUT_IRQ;
	else if (cpu_class_is_omap2())
		chan->enabled_irqs |= OMAP2_DMA_MISALIGNED_ERR_IRQ |
			OMAP2_DMA_TRANS_ERR_IRQ;

	if (cpu_is_omap16xx()) {
		
		if (dev_id != 0) {
			set_gdma_dev(free_ch + 1, dev_id);
			dev_id = free_ch + 1;
		}
		
		dma_write(dev_id | (1 << 10), CCR(free_ch));
	} else if (cpu_is_omap7xx() || cpu_is_omap15xx()) {
		dma_write(dev_id, CCR(free_ch));
	}

	if (cpu_class_is_omap2()) {
		omap2_enable_irq_lch(free_ch);
		omap_enable_channel_irq(free_ch);
		
		dma_write(OMAP2_DMA_CSR_CLEAR_MASK, CSR(free_ch));
		dma_write(1 << free_ch, IRQSTATUS_L0);
	}

	*dma_ch_out = free_ch;

	return 0;
}
EXPORT_SYMBOL(omap_request_dma);

void omap_free_dma(int lch)
{
	unsigned long flags;

	if (dma_chan[lch].dev_id == -1) {
		pr_err("omap_dma: trying to free unallocated DMA channel %d\n",
		       lch);
		return;
	}

	if (cpu_class_is_omap1()) {
		
		dma_write(0, CICR(lch));
		
		dma_write(0, CCR(lch));
	}

	if (cpu_class_is_omap2()) {
		u32 val;

		spin_lock_irqsave(&dma_chan_lock, flags);
		
		val = dma_read(IRQENABLE_L0);
		val &= ~(1 << lch);
		dma_write(val, IRQENABLE_L0);
		spin_unlock_irqrestore(&dma_chan_lock, flags);

		
		dma_write(OMAP2_DMA_CSR_CLEAR_MASK, CSR(lch));
		dma_write(1 << lch, IRQSTATUS_L0);

		
		dma_write(0, CICR(lch));

		
		dma_write(0, CCR(lch));
		omap_clear_dma(lch);
	}

	spin_lock_irqsave(&dma_chan_lock, flags);
	dma_chan[lch].dev_id = -1;
	dma_chan[lch].next_lch = -1;
	dma_chan[lch].callback = NULL;
	spin_unlock_irqrestore(&dma_chan_lock, flags);
}
EXPORT_SYMBOL(omap_free_dma);


void
omap_dma_set_global_params(int arb_rate, int max_fifo_depth, int tparams)
{
	u32 reg;

	if (!cpu_class_is_omap2()) {
		printk(KERN_ERR "FIXME: no %s on 15xx/16xx\n", __func__);
		return;
	}

	if (max_fifo_depth == 0)
		max_fifo_depth = 1;
	if (arb_rate == 0)
		arb_rate = 1;

	reg = 0xff & max_fifo_depth;
	reg |= (0x3 & tparams) << 12;
	reg |= (arb_rate & 0xff) << 16;

	dma_write(reg, GCR);
}
EXPORT_SYMBOL(omap_dma_set_global_params);


int
omap_dma_set_prio_lch(int lch, unsigned char read_prio,
		      unsigned char write_prio)
{
	u32 l;

	if (unlikely((lch < 0 || lch >= dma_lch_count))) {
		printk(KERN_ERR "Invalid channel id\n");
		return -EINVAL;
	}
	l = dma_read(CCR(lch));
	l &= ~((1 << 6) | (1 << 26));
	if (cpu_is_omap2430() || cpu_is_omap34xx() ||  cpu_is_omap44xx())
		l |= ((read_prio & 0x1) << 6) | ((write_prio & 0x1) << 26);
	else
		l |= ((read_prio & 0x1) << 6);

	dma_write(l, CCR(lch));

	return 0;
}
EXPORT_SYMBOL(omap_dma_set_prio_lch);


void omap_clear_dma(int lch)
{
	unsigned long flags;

	local_irq_save(flags);

	if (cpu_class_is_omap1()) {
		u32 l;

		l = dma_read(CCR(lch));
		l &= ~OMAP_DMA_CCR_EN;
		dma_write(l, CCR(lch));

		
		l = dma_read(CSR(lch));
	}

	if (cpu_class_is_omap2()) {
		int i;
		void __iomem *lch_base = omap_dma_base + OMAP_DMA4_CH_BASE(lch);
		for (i = 0; i < 0x44; i += 4)
			__raw_writel(0, lch_base + i);
	}

	local_irq_restore(flags);
}
EXPORT_SYMBOL(omap_clear_dma);

void omap_start_dma(int lch)
{
	u32 l;

	if (!omap_dma_in_1510_mode() && dma_chan[lch].next_lch != -1) {
		int next_lch, cur_lch;
		char dma_chan_link_map[OMAP_DMA4_LOGICAL_DMA_CH_COUNT];

		dma_chan_link_map[lch] = 1;
		
		enable_lnk(lch);

		memset(dma_chan_link_map, 0, sizeof(dma_chan_link_map));
		cur_lch = dma_chan[lch].next_lch;
		do {
			next_lch = dma_chan[cur_lch].next_lch;

			
			if (dma_chan_link_map[cur_lch])
				break;
			
			dma_chan_link_map[cur_lch] = 1;

			enable_lnk(cur_lch);
			omap_enable_channel_irq(cur_lch);

			cur_lch = next_lch;
		} while (next_lch != -1);
	} else if (cpu_is_omap242x() ||
		(cpu_is_omap243x() &&  omap_type() <= OMAP2430_REV_ES1_0)) {

		
		dma_write(lch, CLNK_CTRL(lch));
	}

	omap_enable_channel_irq(lch);

	l = dma_read(CCR(lch));

	
	if (cpu_is_omap24xx())
		l |= OMAP_DMA_CCR_EN;

	l |= OMAP_DMA_CCR_EN;
	dma_write(l, CCR(lch));

	dma_chan[lch].flags |= OMAP_DMA_ACTIVE;
}
EXPORT_SYMBOL(omap_start_dma);

void omap_stop_dma(int lch)
{
	u32 l;

	
	if (cpu_class_is_omap1())
		dma_write(0, CICR(lch));

	l = dma_read(CCR(lch));
	l &= ~OMAP_DMA_CCR_EN;
	dma_write(l, CCR(lch));

	if (!omap_dma_in_1510_mode() && dma_chan[lch].next_lch != -1) {
		int next_lch, cur_lch = lch;
		char dma_chan_link_map[OMAP_DMA4_LOGICAL_DMA_CH_COUNT];

		memset(dma_chan_link_map, 0, sizeof(dma_chan_link_map));
		do {
			
			if (dma_chan_link_map[cur_lch])
				break;
			
			dma_chan_link_map[cur_lch] = 1;

			disable_lnk(cur_lch);

			next_lch = dma_chan[cur_lch].next_lch;
			cur_lch = next_lch;
		} while (next_lch != -1);
	}

	dma_chan[lch].flags &= ~OMAP_DMA_ACTIVE;
}
EXPORT_SYMBOL(omap_stop_dma);


int omap_set_dma_callback(int lch,
			  void (*callback)(int lch, u16 ch_status, void *data),
			  void *data)
{
	unsigned long flags;

	if (lch < 0)
		return -ENODEV;

	spin_lock_irqsave(&dma_chan_lock, flags);
	if (dma_chan[lch].dev_id == -1) {
		printk(KERN_ERR "DMA callback for not set for free channel\n");
		spin_unlock_irqrestore(&dma_chan_lock, flags);
		return -EINVAL;
	}
	dma_chan[lch].callback = callback;
	dma_chan[lch].data = data;
	spin_unlock_irqrestore(&dma_chan_lock, flags);

	return 0;
}
EXPORT_SYMBOL(omap_set_dma_callback);


dma_addr_t omap_get_dma_src_pos(int lch)
{
	dma_addr_t offset = 0;

	if (cpu_is_omap15xx())
		offset = dma_read(CPC(lch));
	else
		offset = dma_read(CSAC(lch));

	
	if (!cpu_is_omap15xx() && offset == 0)
		offset = dma_read(CSAC(lch));

	if (cpu_class_is_omap1())
		offset |= (dma_read(CSSA_U(lch)) << 16);

	return offset;
}
EXPORT_SYMBOL(omap_get_dma_src_pos);


dma_addr_t omap_get_dma_dst_pos(int lch)
{
	dma_addr_t offset = 0;

	if (cpu_is_omap15xx())
		offset = dma_read(CPC(lch));
	else
		offset = dma_read(CDAC(lch));

	
	if (!cpu_is_omap15xx() && offset == 0)
		offset = dma_read(CDAC(lch));

	if (cpu_class_is_omap1())
		offset |= (dma_read(CDSA_U(lch)) << 16);

	return offset;
}
EXPORT_SYMBOL(omap_get_dma_dst_pos);

int omap_get_dma_active_status(int lch)
{
	return (dma_read(CCR(lch)) & OMAP_DMA_CCR_EN) != 0;
}
EXPORT_SYMBOL(omap_get_dma_active_status);

int omap_dma_running(void)
{
	int lch;

	
	if (cpu_is_omap1510())
		if (omap_readw(0xfffec000 + 0x00) & (1 << 0))
			return 1;

	
	if (cpu_is_omap16xx())
		if (omap_readw(OMAP1610_DMA_LCD_CCR) & OMAP_DMA_CCR_EN)
			return 1;

	for (lch = 0; lch < dma_chan_count; lch++)
		if (dma_read(CCR(lch)) & OMAP_DMA_CCR_EN)
			return 1;

	return 0;
}


void omap_dma_link_lch(int lch_head, int lch_queue)
{
	if (omap_dma_in_1510_mode()) {
		if (lch_head == lch_queue) {
			dma_write(dma_read(CCR(lch_head)) | (3 << 8),
								CCR(lch_head));
			return;
		}
		printk(KERN_ERR "DMA linking is not supported in 1510 mode\n");
		BUG();
		return;
	}

	if ((dma_chan[lch_head].dev_id == -1) ||
	    (dma_chan[lch_queue].dev_id == -1)) {
		printk(KERN_ERR "omap_dma: trying to link "
		       "non requested channels\n");
		dump_stack();
	}

	dma_chan[lch_head].next_lch = lch_queue;
}
EXPORT_SYMBOL(omap_dma_link_lch);


void omap_dma_unlink_lch(int lch_head, int lch_queue)
{
	if (omap_dma_in_1510_mode()) {
		if (lch_head == lch_queue) {
			dma_write(dma_read(CCR(lch_head)) & ~(3 << 8),
								CCR(lch_head));
			return;
		}
		printk(KERN_ERR "DMA linking is not supported in 1510 mode\n");
		BUG();
		return;
	}

	if (dma_chan[lch_head].next_lch != lch_queue ||
	    dma_chan[lch_head].next_lch == -1) {
		printk(KERN_ERR "omap_dma: trying to unlink "
		       "non linked channels\n");
		dump_stack();
	}

	if ((dma_chan[lch_head].flags & OMAP_DMA_ACTIVE) ||
	    (dma_chan[lch_head].flags & OMAP_DMA_ACTIVE)) {
		printk(KERN_ERR "omap_dma: You need to stop the DMA channels "
		       "before unlinking\n");
		dump_stack();
	}

	dma_chan[lch_head].next_lch = -1;
}
EXPORT_SYMBOL(omap_dma_unlink_lch);



#ifndef CONFIG_ARCH_OMAP1

static void create_dma_lch_chain(int lch_head, int lch_queue)
{
	u32 l;

	
	if (dma_chan[lch_head].next_linked_ch == -1) {
		dma_chan[lch_head].next_linked_ch = lch_queue;
		dma_chan[lch_head].prev_linked_ch = lch_queue;
		dma_chan[lch_queue].next_linked_ch = lch_head;
		dma_chan[lch_queue].prev_linked_ch = lch_head;
	}

	
	else {
		dma_chan[lch_queue].next_linked_ch =
					dma_chan[lch_head].next_linked_ch;
		dma_chan[lch_queue].prev_linked_ch = lch_head;
		dma_chan[lch_head].next_linked_ch = lch_queue;
		dma_chan[dma_chan[lch_queue].next_linked_ch].prev_linked_ch =
					lch_queue;
	}

	l = dma_read(CLNK_CTRL(lch_head));
	l &= ~(0x1f);
	l |= lch_queue;
	dma_write(l, CLNK_CTRL(lch_head));

	l = dma_read(CLNK_CTRL(lch_queue));
	l &= ~(0x1f);
	l |= (dma_chan[lch_queue].next_linked_ch);
	dma_write(l, CLNK_CTRL(lch_queue));
}


int omap_request_dma_chain(int dev_id, const char *dev_name,
			   void (*callback) (int lch, u16 ch_status,
					     void *data),
			   int *chain_id, int no_of_chans, int chain_mode,
			   struct omap_dma_channel_params params)
{
	int *channels;
	int i, err;

	
	if (chain_mode != OMAP_DMA_STATIC_CHAIN
			&& chain_mode != OMAP_DMA_DYNAMIC_CHAIN) {
		printk(KERN_ERR "Invalid chain mode requested\n");
		return -EINVAL;
	}

	if (unlikely((no_of_chans < 1
			|| no_of_chans > dma_lch_count))) {
		printk(KERN_ERR "Invalid Number of channels requested\n");
		return -EINVAL;
	}

	
	channels = kmalloc(sizeof(*channels) * no_of_chans, GFP_KERNEL);
	if (channels == NULL) {
		printk(KERN_ERR "omap_dma: No memory for channel queue\n");
		return -ENOMEM;
	}

	
	for (i = 0; i < no_of_chans; i++) {
		err = omap_request_dma(dev_id, dev_name,
					callback, NULL, &channels[i]);
		if (err < 0) {
			int j;
			for (j = 0; j < i; j++)
				omap_free_dma(channels[j]);
			kfree(channels);
			printk(KERN_ERR "omap_dma: Request failed %d\n", err);
			return err;
		}
		dma_chan[channels[i]].prev_linked_ch = -1;
		dma_chan[channels[i]].state = DMA_CH_NOTSTARTED;

		
		omap_set_dma_params(channels[i], &params);
	}

	*chain_id = channels[0];
	dma_linked_lch[*chain_id].linked_dmach_q = channels;
	dma_linked_lch[*chain_id].chain_mode = chain_mode;
	dma_linked_lch[*chain_id].chain_state = DMA_CHAIN_NOTSTARTED;
	dma_linked_lch[*chain_id].no_of_lchs_linked = no_of_chans;

	for (i = 0; i < no_of_chans; i++)
		dma_chan[channels[i]].chain_id = *chain_id;

	
	OMAP_DMA_CHAIN_QINIT(*chain_id);

	
	if (no_of_chans == 1)
		create_dma_lch_chain(channels[0], channels[0]);
	else {
		for (i = 0; i < (no_of_chans - 1); i++)
			create_dma_lch_chain(channels[i], channels[i + 1]);
	}

	return 0;
}
EXPORT_SYMBOL(omap_request_dma_chain);


int omap_modify_dma_chain_params(int chain_id,
				struct omap_dma_channel_params params)
{
	int *channels;
	u32 i;

	
	if (unlikely((chain_id < 0
			|| chain_id >= dma_lch_count))) {
		printk(KERN_ERR "Invalid chain id\n");
		return -EINVAL;
	}

	
	if (dma_linked_lch[chain_id].linked_dmach_q == NULL) {
		printk(KERN_ERR "Chain doesn't exists\n");
		return -EINVAL;
	}
	channels = dma_linked_lch[chain_id].linked_dmach_q;

	for (i = 0; i < dma_linked_lch[chain_id].no_of_lchs_linked; i++) {
		
		omap_set_dma_params(channels[i], &params);
	}

	return 0;
}
EXPORT_SYMBOL(omap_modify_dma_chain_params);


int omap_free_dma_chain(int chain_id)
{
	int *channels;
	u32 i;

	
	if (unlikely((chain_id < 0 || chain_id >= dma_lch_count))) {
		printk(KERN_ERR "Invalid chain id\n");
		return -EINVAL;
	}

	
	if (dma_linked_lch[chain_id].linked_dmach_q == NULL) {
		printk(KERN_ERR "Chain doesn't exists\n");
		return -EINVAL;
	}

	channels = dma_linked_lch[chain_id].linked_dmach_q;
	for (i = 0; i < dma_linked_lch[chain_id].no_of_lchs_linked; i++) {
		dma_chan[channels[i]].next_linked_ch = -1;
		dma_chan[channels[i]].prev_linked_ch = -1;
		dma_chan[channels[i]].chain_id = -1;
		dma_chan[channels[i]].state = DMA_CH_NOTSTARTED;
		omap_free_dma(channels[i]);
	}

	kfree(channels);

	dma_linked_lch[chain_id].linked_dmach_q = NULL;
	dma_linked_lch[chain_id].chain_mode = -1;
	dma_linked_lch[chain_id].chain_state = -1;

	return (0);
}
EXPORT_SYMBOL(omap_free_dma_chain);


int omap_dma_chain_status(int chain_id)
{
	
	if (unlikely((chain_id < 0 || chain_id >= dma_lch_count))) {
		printk(KERN_ERR "Invalid chain id\n");
		return -EINVAL;
	}

	
	if (dma_linked_lch[chain_id].linked_dmach_q == NULL) {
		printk(KERN_ERR "Chain doesn't exists\n");
		return -EINVAL;
	}
	pr_debug("CHAINID=%d, qcnt=%d\n", chain_id,
			dma_linked_lch[chain_id].q_count);

	if (OMAP_DMA_CHAIN_QEMPTY(chain_id))
		return OMAP_DMA_CHAIN_INACTIVE;

	return OMAP_DMA_CHAIN_ACTIVE;
}
EXPORT_SYMBOL(omap_dma_chain_status);


int omap_dma_chain_a_transfer(int chain_id, int src_start, int dest_start,
			int elem_count, int frame_count, void *callbk_data)
{
	int *channels;
	u32 l, lch;
	int start_dma = 0;

	
	if (elem_count < 1) {
		printk(KERN_ERR "Invalid buffer size\n");
		return -EINVAL;
	}

	
	if (unlikely((chain_id < 0
			|| chain_id >= dma_lch_count))) {
		printk(KERN_ERR "Invalid chain id\n");
		return -EINVAL;
	}

	
	if (dma_linked_lch[chain_id].linked_dmach_q == NULL) {
		printk(KERN_ERR "Chain doesn't exist\n");
		return -EINVAL;
	}

	
	if (OMAP_DMA_CHAIN_QFULL(chain_id))
		return -EBUSY;

	
	channels = dma_linked_lch[chain_id].linked_dmach_q;

	
	lch = channels[dma_linked_lch[chain_id].q_tail];

	
	dma_chan[lch].data = callbk_data;

	
	OMAP_DMA_CHAIN_INCQTAIL(chain_id);

	
	if (src_start != 0)
		dma_write(src_start, CSSA(lch));
	if (dest_start != 0)
		dma_write(dest_start, CDSA(lch));

	
	dma_write(elem_count, CEN(lch));
	dma_write(frame_count, CFN(lch));

	
	if (dma_linked_lch[chain_id].chain_mode == OMAP_DMA_DYNAMIC_CHAIN) {

		
		if (dma_linked_lch[chain_id].chain_state ==
						DMA_CHAIN_NOTSTARTED) {
			
			if (dma_chan[dma_chan[lch].prev_linked_ch].state ==
								DMA_CH_QUEUED)
				enable_lnk(dma_chan[lch].prev_linked_ch);
			dma_chan[lch].state = DMA_CH_QUEUED;
		}

		
		else {
			start_dma = 1;

			if (dma_chan[dma_chan[lch].prev_linked_ch].state ==
							DMA_CH_STARTED) {
				enable_lnk(dma_chan[lch].prev_linked_ch);
				dma_chan[lch].state = DMA_CH_QUEUED;
				start_dma = 0;
				if (0 == ((1 << 7) & dma_read(
					CCR(dma_chan[lch].prev_linked_ch)))) {
					disable_lnk(dma_chan[lch].
						    prev_linked_ch);
					pr_debug("\n prev ch is stopped\n");
					start_dma = 1;
				}
			}

			else if (dma_chan[dma_chan[lch].prev_linked_ch].state
							== DMA_CH_QUEUED) {
				enable_lnk(dma_chan[lch].prev_linked_ch);
				dma_chan[lch].state = DMA_CH_QUEUED;
				start_dma = 0;
			}
			omap_enable_channel_irq(lch);

			l = dma_read(CCR(lch));

			if ((0 == (l & (1 << 24))))
				l &= ~(1 << 25);
			else
				l |= (1 << 25);
			if (start_dma == 1) {
				if (0 == (l & (1 << 7))) {
					l |= (1 << 7);
					dma_chan[lch].state = DMA_CH_STARTED;
					pr_debug("starting %d\n", lch);
					dma_write(l, CCR(lch));
				} else
					start_dma = 0;
			} else {
				if (0 == (l & (1 << 7)))
					dma_write(l, CCR(lch));
			}
			dma_chan[lch].flags |= OMAP_DMA_ACTIVE;
		}
	}

	return 0;
}
EXPORT_SYMBOL(omap_dma_chain_a_transfer);


int omap_start_dma_chain_transfers(int chain_id)
{
	int *channels;
	u32 l, i;

	if (unlikely((chain_id < 0 || chain_id >= dma_lch_count))) {
		printk(KERN_ERR "Invalid chain id\n");
		return -EINVAL;
	}

	channels = dma_linked_lch[chain_id].linked_dmach_q;

	if (dma_linked_lch[channels[0]].chain_state == DMA_CHAIN_STARTED) {
		printk(KERN_ERR "Chain is already started\n");
		return -EBUSY;
	}

	if (dma_linked_lch[chain_id].chain_mode == OMAP_DMA_STATIC_CHAIN) {
		for (i = 0; i < dma_linked_lch[chain_id].no_of_lchs_linked;
									i++) {
			enable_lnk(channels[i]);
			omap_enable_channel_irq(channels[i]);
		}
	} else {
		omap_enable_channel_irq(channels[0]);
	}

	l = dma_read(CCR(channels[0]));
	l |= (1 << 7);
	dma_linked_lch[chain_id].chain_state = DMA_CHAIN_STARTED;
	dma_chan[channels[0]].state = DMA_CH_STARTED;

	if ((0 == (l & (1 << 24))))
		l &= ~(1 << 25);
	else
		l |= (1 << 25);
	dma_write(l, CCR(channels[0]));

	dma_chan[channels[0]].flags |= OMAP_DMA_ACTIVE;

	return 0;
}
EXPORT_SYMBOL(omap_start_dma_chain_transfers);


int omap_stop_dma_chain_transfers(int chain_id)
{
	int *channels;
	u32 l, i;
	u32 sys_cf;

	
	if (unlikely((chain_id < 0 || chain_id >= dma_lch_count))) {
		printk(KERN_ERR "Invalid chain id\n");
		return -EINVAL;
	}

	
	if (dma_linked_lch[chain_id].linked_dmach_q == NULL) {
		printk(KERN_ERR "Chain doesn't exists\n");
		return -EINVAL;
	}
	channels = dma_linked_lch[chain_id].linked_dmach_q;

	
	sys_cf = dma_read(OCP_SYSCONFIG);
	l = sys_cf;
	
	l &= ~((1 << 12)|(1 << 13));
	dma_write(l, OCP_SYSCONFIG);

	for (i = 0; i < dma_linked_lch[chain_id].no_of_lchs_linked; i++) {

		
		l = dma_read(CCR(channels[i]));
		l &= ~(1 << 7);
		dma_write(l, CCR(channels[i]));

		
		disable_lnk(channels[i]);
		dma_chan[channels[i]].state = DMA_CH_NOTSTARTED;

	}
	dma_linked_lch[chain_id].chain_state = DMA_CHAIN_NOTSTARTED;

	
	OMAP_DMA_CHAIN_QINIT(chain_id);

	
	dma_write(sys_cf, OCP_SYSCONFIG);

	return 0;
}
EXPORT_SYMBOL(omap_stop_dma_chain_transfers);



int omap_get_dma_chain_index(int chain_id, int *ei, int *fi)
{
	int lch;
	int *channels;

	
	if (unlikely((chain_id < 0 || chain_id >= dma_lch_count))) {
		printk(KERN_ERR "Invalid chain id\n");
		return -EINVAL;
	}

	
	if (dma_linked_lch[chain_id].linked_dmach_q == NULL) {
		printk(KERN_ERR "Chain doesn't exists\n");
		return -EINVAL;
	}
	if ((!ei) || (!fi))
		return -EINVAL;

	channels = dma_linked_lch[chain_id].linked_dmach_q;

	
	lch = channels[dma_linked_lch[chain_id].q_head];

	*ei = dma_read(CCEN(lch));
	*fi = dma_read(CCFN(lch));

	return 0;
}
EXPORT_SYMBOL(omap_get_dma_chain_index);


int omap_get_dma_chain_dst_pos(int chain_id)
{
	int lch;
	int *channels;

	
	if (unlikely((chain_id < 0 || chain_id >= dma_lch_count))) {
		printk(KERN_ERR "Invalid chain id\n");
		return -EINVAL;
	}

	
	if (dma_linked_lch[chain_id].linked_dmach_q == NULL) {
		printk(KERN_ERR "Chain doesn't exists\n");
		return -EINVAL;
	}

	channels = dma_linked_lch[chain_id].linked_dmach_q;

	
	lch = channels[dma_linked_lch[chain_id].q_head];

	return dma_read(CDAC(lch));
}
EXPORT_SYMBOL(omap_get_dma_chain_dst_pos);


int omap_get_dma_chain_src_pos(int chain_id)
{
	int lch;
	int *channels;

	
	if (unlikely((chain_id < 0 || chain_id >= dma_lch_count))) {
		printk(KERN_ERR "Invalid chain id\n");
		return -EINVAL;
	}

	
	if (dma_linked_lch[chain_id].linked_dmach_q == NULL) {
		printk(KERN_ERR "Chain doesn't exists\n");
		return -EINVAL;
	}

	channels = dma_linked_lch[chain_id].linked_dmach_q;

	
	lch = channels[dma_linked_lch[chain_id].q_head];

	return dma_read(CSAC(lch));
}
EXPORT_SYMBOL(omap_get_dma_chain_src_pos);
#endif	



#ifdef CONFIG_ARCH_OMAP1

static int omap1_dma_handle_ch(int ch)
{
	u32 csr;

	if (enable_1510_mode && ch >= 6) {
		csr = dma_chan[ch].saved_csr;
		dma_chan[ch].saved_csr = 0;
	} else
		csr = dma_read(CSR(ch));
	if (enable_1510_mode && ch <= 2 && (csr >> 7) != 0) {
		dma_chan[ch + 6].saved_csr = csr >> 7;
		csr &= 0x7f;
	}
	if ((csr & 0x3f) == 0)
		return 0;
	if (unlikely(dma_chan[ch].dev_id == -1)) {
		printk(KERN_WARNING "Spurious interrupt from DMA channel "
		       "%d (CSR %04x)\n", ch, csr);
		return 0;
	}
	if (unlikely(csr & OMAP1_DMA_TOUT_IRQ))
		printk(KERN_WARNING "DMA timeout with device %d\n",
		       dma_chan[ch].dev_id);
	if (unlikely(csr & OMAP_DMA_DROP_IRQ))
		printk(KERN_WARNING "DMA synchronization event drop occurred "
		       "with device %d\n", dma_chan[ch].dev_id);
	if (likely(csr & OMAP_DMA_BLOCK_IRQ))
		dma_chan[ch].flags &= ~OMAP_DMA_ACTIVE;
	if (likely(dma_chan[ch].callback != NULL))
		dma_chan[ch].callback(ch, csr, dma_chan[ch].data);

	return 1;
}

static irqreturn_t omap1_dma_irq_handler(int irq, void *dev_id)
{
	int ch = ((int) dev_id) - 1;
	int handled = 0;

	for (;;) {
		int handled_now = 0;

		handled_now += omap1_dma_handle_ch(ch);
		if (enable_1510_mode && dma_chan[ch + 6].saved_csr)
			handled_now += omap1_dma_handle_ch(ch + 6);
		if (!handled_now)
			break;
		handled += handled_now;
	}

	return handled ? IRQ_HANDLED : IRQ_NONE;
}

#else
#define omap1_dma_irq_handler	NULL
#endif

#if defined(CONFIG_ARCH_OMAP2) || defined(CONFIG_ARCH_OMAP3) || \
			defined(CONFIG_ARCH_OMAP4)

static int omap2_dma_handle_ch(int ch)
{
	u32 status = dma_read(CSR(ch));

	if (!status) {
		if (printk_ratelimit())
			printk(KERN_WARNING "Spurious DMA IRQ for lch %d\n",
				ch);
		dma_write(1 << ch, IRQSTATUS_L0);
		return 0;
	}
	if (unlikely(dma_chan[ch].dev_id == -1)) {
		if (printk_ratelimit())
			printk(KERN_WARNING "IRQ %04x for non-allocated DMA"
					"channel %d\n", status, ch);
		return 0;
	}
	if (unlikely(status & OMAP_DMA_DROP_IRQ))
		printk(KERN_INFO
		       "DMA synchronization event drop occurred with device "
		       "%d\n", dma_chan[ch].dev_id);
	if (unlikely(status & OMAP2_DMA_TRANS_ERR_IRQ)) {
		printk(KERN_INFO "DMA transaction error with device %d\n",
		       dma_chan[ch].dev_id);
		if (cpu_class_is_omap2()) {
			
			u32 ccr;

			ccr = dma_read(CCR(ch));
			ccr &= ~OMAP_DMA_CCR_EN;
			dma_write(ccr, CCR(ch));
			dma_chan[ch].flags &= ~OMAP_DMA_ACTIVE;
		}
	}
	if (unlikely(status & OMAP2_DMA_SECURE_ERR_IRQ))
		printk(KERN_INFO "DMA secure error with device %d\n",
		       dma_chan[ch].dev_id);
	if (unlikely(status & OMAP2_DMA_MISALIGNED_ERR_IRQ))
		printk(KERN_INFO "DMA misaligned error with device %d\n",
		       dma_chan[ch].dev_id);

	dma_write(OMAP2_DMA_CSR_CLEAR_MASK, CSR(ch));
	dma_write(1 << ch, IRQSTATUS_L0);

	
	if (dma_chan[ch].chain_id != -1) {
		int chain_id = dma_chan[ch].chain_id;
		dma_chan[ch].state = DMA_CH_NOTSTARTED;
		if (dma_read(CLNK_CTRL(ch)) & (1 << 15))
			dma_chan[dma_chan[ch].next_linked_ch].state =
							DMA_CH_STARTED;
		if (dma_linked_lch[chain_id].chain_mode ==
						OMAP_DMA_DYNAMIC_CHAIN)
			disable_lnk(ch);

		if (!OMAP_DMA_CHAIN_QEMPTY(chain_id))
			OMAP_DMA_CHAIN_INCQHEAD(chain_id);

		status = dma_read(CSR(ch));
	}

	dma_write(status, CSR(ch));

	if (likely(dma_chan[ch].callback != NULL))
		dma_chan[ch].callback(ch, status, dma_chan[ch].data);

	return 0;
}


static irqreturn_t omap2_dma_irq_handler(int irq, void *dev_id)
{
	u32 val, enable_reg;
	int i;

	val = dma_read(IRQSTATUS_L0);
	if (val == 0) {
		if (printk_ratelimit())
			printk(KERN_WARNING "Spurious DMA IRQ\n");
		return IRQ_HANDLED;
	}
	enable_reg = dma_read(IRQENABLE_L0);
	val &= enable_reg; 
	for (i = 0; i < dma_lch_count && val != 0; i++) {
		if (val & 1)
			omap2_dma_handle_ch(i);
		val >>= 1;
	}

	return IRQ_HANDLED;
}

static struct irqaction omap24xx_dma_irq = {
	.name = "DMA",
	.handler = omap2_dma_irq_handler,
	.flags = IRQF_DISABLED
};

#else
static struct irqaction omap24xx_dma_irq;
#endif



static struct lcd_dma_info {
	spinlock_t lock;
	int reserved;
	void (*callback)(u16 status, void *data);
	void *cb_data;

	int active;
	unsigned long addr, size;
	int rotate, data_type, xres, yres;
	int vxres;
	int mirror;
	int xscale, yscale;
	int ext_ctrl;
	int src_port;
	int single_transfer;
} lcd_dma;

void omap_set_lcd_dma_b1(unsigned long addr, u16 fb_xres, u16 fb_yres,
			 int data_type)
{
	lcd_dma.addr = addr;
	lcd_dma.data_type = data_type;
	lcd_dma.xres = fb_xres;
	lcd_dma.yres = fb_yres;
}
EXPORT_SYMBOL(omap_set_lcd_dma_b1);

void omap_set_lcd_dma_src_port(int port)
{
	lcd_dma.src_port = port;
}

void omap_set_lcd_dma_ext_controller(int external)
{
	lcd_dma.ext_ctrl = external;
}
EXPORT_SYMBOL(omap_set_lcd_dma_ext_controller);

void omap_set_lcd_dma_single_transfer(int single)
{
	lcd_dma.single_transfer = single;
}
EXPORT_SYMBOL(omap_set_lcd_dma_single_transfer);

void omap_set_lcd_dma_b1_rotation(int rotate)
{
	if (omap_dma_in_1510_mode()) {
		printk(KERN_ERR "DMA rotation is not supported in 1510 mode\n");
		BUG();
		return;
	}
	lcd_dma.rotate = rotate;
}
EXPORT_SYMBOL(omap_set_lcd_dma_b1_rotation);

void omap_set_lcd_dma_b1_mirror(int mirror)
{
	if (omap_dma_in_1510_mode()) {
		printk(KERN_ERR "DMA mirror is not supported in 1510 mode\n");
		BUG();
	}
	lcd_dma.mirror = mirror;
}
EXPORT_SYMBOL(omap_set_lcd_dma_b1_mirror);

void omap_set_lcd_dma_b1_vxres(unsigned long vxres)
{
	if (omap_dma_in_1510_mode()) {
		printk(KERN_ERR "DMA virtual resulotion is not supported "
				"in 1510 mode\n");
		BUG();
	}
	lcd_dma.vxres = vxres;
}
EXPORT_SYMBOL(omap_set_lcd_dma_b1_vxres);

void omap_set_lcd_dma_b1_scale(unsigned int xscale, unsigned int yscale)
{
	if (omap_dma_in_1510_mode()) {
		printk(KERN_ERR "DMA scale is not supported in 1510 mode\n");
		BUG();
	}
	lcd_dma.xscale = xscale;
	lcd_dma.yscale = yscale;
}
EXPORT_SYMBOL(omap_set_lcd_dma_b1_scale);

static void set_b1_regs(void)
{
	unsigned long top, bottom;
	int es;
	u16 w;
	unsigned long en, fn;
	long ei, fi;
	unsigned long vxres;
	unsigned int xscale, yscale;

	switch (lcd_dma.data_type) {
	case OMAP_DMA_DATA_TYPE_S8:
		es = 1;
		break;
	case OMAP_DMA_DATA_TYPE_S16:
		es = 2;
		break;
	case OMAP_DMA_DATA_TYPE_S32:
		es = 4;
		break;
	default:
		BUG();
		return;
	}

	vxres = lcd_dma.vxres ? lcd_dma.vxres : lcd_dma.xres;
	xscale = lcd_dma.xscale ? lcd_dma.xscale : 1;
	yscale = lcd_dma.yscale ? lcd_dma.yscale : 1;
	BUG_ON(vxres < lcd_dma.xres);

#define PIXADDR(x, y) (lcd_dma.addr +					\
		((y) * vxres * yscale + (x) * xscale) * es)
#define PIXSTEP(sx, sy, dx, dy) (PIXADDR(dx, dy) - PIXADDR(sx, sy) - es + 1)

	switch (lcd_dma.rotate) {
	case 0:
		if (!lcd_dma.mirror) {
			top = PIXADDR(0, 0);
			bottom = PIXADDR(lcd_dma.xres - 1, lcd_dma.yres - 1);
			
			if (omap_dma_in_1510_mode() &&
				lcd_dma.data_type == OMAP_DMA_DATA_TYPE_S32)
					bottom += 2;
			ei = PIXSTEP(0, 0, 1, 0);
			fi = PIXSTEP(lcd_dma.xres - 1, 0, 0, 1);
		} else {
			top = PIXADDR(lcd_dma.xres - 1, 0);
			bottom = PIXADDR(0, lcd_dma.yres - 1);
			ei = PIXSTEP(1, 0, 0, 0);
			fi = PIXSTEP(0, 0, lcd_dma.xres - 1, 1);
		}
		en = lcd_dma.xres;
		fn = lcd_dma.yres;
		break;
	case 90:
		if (!lcd_dma.mirror) {
			top = PIXADDR(0, lcd_dma.yres - 1);
			bottom = PIXADDR(lcd_dma.xres - 1, 0);
			ei = PIXSTEP(0, 1, 0, 0);
			fi = PIXSTEP(0, 0, 1, lcd_dma.yres - 1);
		} else {
			top = PIXADDR(lcd_dma.xres - 1, lcd_dma.yres - 1);
			bottom = PIXADDR(0, 0);
			ei = PIXSTEP(0, 1, 0, 0);
			fi = PIXSTEP(1, 0, 0, lcd_dma.yres - 1);
		}
		en = lcd_dma.yres;
		fn = lcd_dma.xres;
		break;
	case 180:
		if (!lcd_dma.mirror) {
			top = PIXADDR(lcd_dma.xres - 1, lcd_dma.yres - 1);
			bottom = PIXADDR(0, 0);
			ei = PIXSTEP(1, 0, 0, 0);
			fi = PIXSTEP(0, 1, lcd_dma.xres - 1, 0);
		} else {
			top = PIXADDR(0, lcd_dma.yres - 1);
			bottom = PIXADDR(lcd_dma.xres - 1, 0);
			ei = PIXSTEP(0, 0, 1, 0);
			fi = PIXSTEP(lcd_dma.xres - 1, 1, 0, 0);
		}
		en = lcd_dma.xres;
		fn = lcd_dma.yres;
		break;
	case 270:
		if (!lcd_dma.mirror) {
			top = PIXADDR(lcd_dma.xres - 1, 0);
			bottom = PIXADDR(0, lcd_dma.yres - 1);
			ei = PIXSTEP(0, 0, 0, 1);
			fi = PIXSTEP(1, lcd_dma.yres - 1, 0, 0);
		} else {
			top = PIXADDR(0, 0);
			bottom = PIXADDR(lcd_dma.xres - 1, lcd_dma.yres - 1);
			ei = PIXSTEP(0, 0, 0, 1);
			fi = PIXSTEP(0, lcd_dma.yres - 1, 1, 0);
		}
		en = lcd_dma.yres;
		fn = lcd_dma.xres;
		break;
	default:
		BUG();
		return;	
	}

	if (omap_dma_in_1510_mode()) {
		omap_writew(top >> 16, OMAP1510_DMA_LCD_TOP_F1_U);
		omap_writew(top, OMAP1510_DMA_LCD_TOP_F1_L);
		omap_writew(bottom >> 16, OMAP1510_DMA_LCD_BOT_F1_U);
		omap_writew(bottom, OMAP1510_DMA_LCD_BOT_F1_L);

		return;
	}

	
	omap_writew(top >> 16, OMAP1610_DMA_LCD_TOP_B1_U);
	omap_writew(top, OMAP1610_DMA_LCD_TOP_B1_L);
	omap_writew(bottom >> 16, OMAP1610_DMA_LCD_BOT_B1_U);
	omap_writew(bottom, OMAP1610_DMA_LCD_BOT_B1_L);

	omap_writew(en, OMAP1610_DMA_LCD_SRC_EN_B1);
	omap_writew(fn, OMAP1610_DMA_LCD_SRC_FN_B1);

	w = omap_readw(OMAP1610_DMA_LCD_CSDP);
	w &= ~0x03;
	w |= lcd_dma.data_type;
	omap_writew(w, OMAP1610_DMA_LCD_CSDP);

	w = omap_readw(OMAP1610_DMA_LCD_CTRL);
	
	w &= ~(0x03 << 6);
	if (lcd_dma.callback != NULL)
		w |= 1 << 1;		
	else
		w &= ~(1 << 1);
	omap_writew(w, OMAP1610_DMA_LCD_CTRL);

	if (!(lcd_dma.rotate || lcd_dma.mirror ||
	      lcd_dma.vxres || lcd_dma.xscale || lcd_dma.yscale))
		return;

	w = omap_readw(OMAP1610_DMA_LCD_CCR);
	
	w |= (0x03 << 12);
	omap_writew(w, OMAP1610_DMA_LCD_CCR);

	omap_writew(ei, OMAP1610_DMA_LCD_SRC_EI_B1);
	omap_writew(fi >> 16, OMAP1610_DMA_LCD_SRC_FI_B1_U);
	omap_writew(fi, OMAP1610_DMA_LCD_SRC_FI_B1_L);
}

static irqreturn_t lcd_dma_irq_handler(int irq, void *dev_id)
{
	u16 w;

	w = omap_readw(OMAP1610_DMA_LCD_CTRL);
	if (unlikely(!(w & (1 << 3)))) {
		printk(KERN_WARNING "Spurious LCD DMA IRQ\n");
		return IRQ_NONE;
	}
	
	w |= (1 << 3);
	omap_writew(w, OMAP1610_DMA_LCD_CTRL);
	lcd_dma.active = 0;
	if (lcd_dma.callback != NULL)
		lcd_dma.callback(w, lcd_dma.cb_data);

	return IRQ_HANDLED;
}

int omap_request_lcd_dma(void (*callback)(u16 status, void *data),
			 void *data)
{
	spin_lock_irq(&lcd_dma.lock);
	if (lcd_dma.reserved) {
		spin_unlock_irq(&lcd_dma.lock);
		printk(KERN_ERR "LCD DMA channel already reserved\n");
		BUG();
		return -EBUSY;
	}
	lcd_dma.reserved = 1;
	spin_unlock_irq(&lcd_dma.lock);
	lcd_dma.callback = callback;
	lcd_dma.cb_data = data;
	lcd_dma.active = 0;
	lcd_dma.single_transfer = 0;
	lcd_dma.rotate = 0;
	lcd_dma.vxres = 0;
	lcd_dma.mirror = 0;
	lcd_dma.xscale = 0;
	lcd_dma.yscale = 0;
	lcd_dma.ext_ctrl = 0;
	lcd_dma.src_port = 0;

	return 0;
}
EXPORT_SYMBOL(omap_request_lcd_dma);

void omap_free_lcd_dma(void)
{
	spin_lock(&lcd_dma.lock);
	if (!lcd_dma.reserved) {
		spin_unlock(&lcd_dma.lock);
		printk(KERN_ERR "LCD DMA is not reserved\n");
		BUG();
		return;
	}
	if (!enable_1510_mode)
		omap_writew(omap_readw(OMAP1610_DMA_LCD_CCR) & ~1,
			    OMAP1610_DMA_LCD_CCR);
	lcd_dma.reserved = 0;
	spin_unlock(&lcd_dma.lock);
}
EXPORT_SYMBOL(omap_free_lcd_dma);

void omap_enable_lcd_dma(void)
{
	u16 w;

	
	if (enable_1510_mode || !lcd_dma.ext_ctrl)
		return;

	w = omap_readw(OMAP1610_DMA_LCD_CTRL);
	w |= 1 << 8;
	omap_writew(w, OMAP1610_DMA_LCD_CTRL);

	lcd_dma.active = 1;

	w = omap_readw(OMAP1610_DMA_LCD_CCR);
	w |= 1 << 7;
	omap_writew(w, OMAP1610_DMA_LCD_CCR);
}
EXPORT_SYMBOL(omap_enable_lcd_dma);

void omap_setup_lcd_dma(void)
{
	BUG_ON(lcd_dma.active);
	if (!enable_1510_mode) {
		
		omap_writew(0x5440, OMAP1610_DMA_LCD_CCR);
		omap_writew(0x9102, OMAP1610_DMA_LCD_CSDP);
		omap_writew(0x0004, OMAP1610_DMA_LCD_LCH_CTRL);
	}
	set_b1_regs();
	if (!enable_1510_mode) {
		u16 w;

		w = omap_readw(OMAP1610_DMA_LCD_CCR);
		
		w |= 1 << 11;		
		if (!lcd_dma.single_transfer)
			w |= (3 << 8);	
		omap_writew(w, OMAP1610_DMA_LCD_CCR);
	}
}
EXPORT_SYMBOL(omap_setup_lcd_dma);

void omap_stop_lcd_dma(void)
{
	u16 w;

	lcd_dma.active = 0;
	if (enable_1510_mode || !lcd_dma.ext_ctrl)
		return;

	w = omap_readw(OMAP1610_DMA_LCD_CCR);
	w &= ~(1 << 7);
	omap_writew(w, OMAP1610_DMA_LCD_CCR);

	w = omap_readw(OMAP1610_DMA_LCD_CTRL);
	w &= ~(1 << 8);
	omap_writew(w, OMAP1610_DMA_LCD_CTRL);
}
EXPORT_SYMBOL(omap_stop_lcd_dma);



static int __init omap_init_dma(void)
{
	int ch, r;

	if (cpu_class_is_omap1()) {
		omap_dma_base = OMAP1_IO_ADDRESS(OMAP1_DMA_BASE);
		dma_lch_count = OMAP1_LOGICAL_DMA_CH_COUNT;
	} else if (cpu_is_omap24xx()) {
		omap_dma_base = OMAP2_IO_ADDRESS(OMAP24XX_DMA4_BASE);
		dma_lch_count = OMAP_DMA4_LOGICAL_DMA_CH_COUNT;
	} else if (cpu_is_omap34xx()) {
		omap_dma_base = OMAP2_IO_ADDRESS(OMAP34XX_DMA4_BASE);
		dma_lch_count = OMAP_DMA4_LOGICAL_DMA_CH_COUNT;
	} else if (cpu_is_omap44xx()) {
		omap_dma_base = OMAP2_IO_ADDRESS(OMAP44XX_DMA4_BASE);
		dma_lch_count = OMAP_DMA4_LOGICAL_DMA_CH_COUNT;
	} else {
		pr_err("DMA init failed for unsupported omap\n");
		return -ENODEV;
	}

	if (cpu_class_is_omap2() && omap_dma_reserve_channels
			&& (omap_dma_reserve_channels <= dma_lch_count))
		dma_lch_count = omap_dma_reserve_channels;

	dma_chan = kzalloc(sizeof(struct omap_dma_lch) * dma_lch_count,
				GFP_KERNEL);
	if (!dma_chan)
		return -ENOMEM;

	if (cpu_class_is_omap2()) {
		dma_linked_lch = kzalloc(sizeof(struct dma_link_info) *
						dma_lch_count, GFP_KERNEL);
		if (!dma_linked_lch) {
			kfree(dma_chan);
			return -ENOMEM;
		}
	}

	if (cpu_is_omap15xx()) {
		printk(KERN_INFO "DMA support for OMAP15xx initialized\n");
		dma_chan_count = 9;
		enable_1510_mode = 1;
	} else if (cpu_is_omap16xx() || cpu_is_omap7xx()) {
		printk(KERN_INFO "OMAP DMA hardware version %d\n",
		       dma_read(HW_ID));
		printk(KERN_INFO "DMA capabilities: %08x:%08x:%04x:%04x:%04x\n",
		       (dma_read(CAPS_0_U) << 16) |
		       dma_read(CAPS_0_L),
		       (dma_read(CAPS_1_U) << 16) |
		       dma_read(CAPS_1_L),
		       dma_read(CAPS_2), dma_read(CAPS_3),
		       dma_read(CAPS_4));
		if (!enable_1510_mode) {
			u16 w;

			
			w = dma_read(GSCR);
			w |= 1 << 3;
			dma_write(w, GSCR);
			dma_chan_count = 16;
		} else
			dma_chan_count = 9;
		if (cpu_is_omap16xx()) {
			u16 w;

			
			w = omap_readw(OMAP1610_DMA_LCD_CTRL);
			w &= ~(1 << 8);
			omap_writew(w, OMAP1610_DMA_LCD_CTRL);
		}
	} else if (cpu_class_is_omap2()) {
		u8 revision = dma_read(REVISION) & 0xff;
		printk(KERN_INFO "OMAP DMA hardware revision %d.%d\n",
		       revision >> 4, revision & 0xf);
		dma_chan_count = dma_lch_count;
	} else {
		dma_chan_count = 0;
		return 0;
	}

	spin_lock_init(&lcd_dma.lock);
	spin_lock_init(&dma_chan_lock);

	for (ch = 0; ch < dma_chan_count; ch++) {
		omap_clear_dma(ch);
		dma_chan[ch].dev_id = -1;
		dma_chan[ch].next_lch = -1;

		if (ch >= 6 && enable_1510_mode)
			continue;

		if (cpu_class_is_omap1()) {
			
			r = request_irq(omap1_dma_irq[ch],
					omap1_dma_irq_handler, 0, "DMA",
					(void *) (ch + 1));
			if (r != 0) {
				int i;

				printk(KERN_ERR "unable to request IRQ %d "
				       "for DMA (error %d)\n",
				       omap1_dma_irq[ch], r);
				for (i = 0; i < ch; i++)
					free_irq(omap1_dma_irq[i],
						 (void *) (i + 1));
				return r;
			}
		}
	}

	if (cpu_is_omap2430() || cpu_is_omap34xx() || cpu_is_omap44xx())
		omap_dma_set_global_params(DMA_DEFAULT_ARB_RATE,
				DMA_DEFAULT_FIFO_DEPTH, 0);

	if (cpu_class_is_omap2()) {
		int irq;
		if (cpu_is_omap44xx())
			irq = INT_44XX_SDMA_IRQ0;
		else
			irq = INT_24XX_SDMA_IRQ0;
		setup_irq(irq, &omap24xx_dma_irq);
	}

	
	if (cpu_is_omap34xx()) {
		u32 v = dma_read(OCP_SYSCONFIG);
		v &= ~(DMA_SYSCONFIG_MIDLEMODE_MASK |
				DMA_SYSCONFIG_SIDLEMODE_MASK |
				DMA_SYSCONFIG_AUTOIDLE);
		v |= (DMA_SYSCONFIG_MIDLEMODE(DMA_IDLEMODE_SMARTIDLE) |
			DMA_SYSCONFIG_SIDLEMODE(DMA_IDLEMODE_SMARTIDLE) |
			DMA_SYSCONFIG_AUTOIDLE);
		dma_write(v , OCP_SYSCONFIG);
	}


	
	if (cpu_class_is_omap1()) {
		r = request_irq(INT_DMA_LCD, lcd_dma_irq_handler, 0,
				"LCD DMA", NULL);
		if (r != 0) {
			int i;

			printk(KERN_ERR "unable to request IRQ for LCD DMA "
			       "(error %d)\n", r);
			for (i = 0; i < dma_chan_count; i++)
				free_irq(omap1_dma_irq[i], (void *) (i + 1));
			return r;
		}
	}

	return 0;
}

arch_initcall(omap_init_dma);


static int __init omap_dma_cmdline_reserve_ch(char *str)
{
	if (get_option(&str, &omap_dma_reserve_channels) != 1)
		omap_dma_reserve_channels = 0;
	return 1;
}

__setup("omap_dma_reserve_ch=", omap_dma_cmdline_reserve_ch);


