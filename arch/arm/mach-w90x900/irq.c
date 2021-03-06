

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/ptrace.h>
#include <linux/sysdev.h>
#include <linux/io.h>

#include <asm/irq.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <mach/regs-irq.h>

struct group_irq {
	unsigned long		gpen;
	unsigned int		enabled;
	void			(*enable)(struct group_irq *, int enable);
};

static DEFINE_SPINLOCK(groupirq_lock);

#define DEFINE_GROUP(_name, _ctrlbit, _num)				\
struct group_irq group_##_name = {					\
		.enable		= nuc900_group_enable,			\
		.gpen		= ((1 << _num) - 1) << _ctrlbit,	\
	}

static void nuc900_group_enable(struct group_irq *gpirq, int enable);

static DEFINE_GROUP(nirq0, 0, 4);
static DEFINE_GROUP(nirq1, 4, 4);
static DEFINE_GROUP(usbh, 8, 2);
static DEFINE_GROUP(ottimer, 16, 3);
static DEFINE_GROUP(gdma, 20, 2);
static DEFINE_GROUP(sc, 24, 2);
static DEFINE_GROUP(i2c, 26, 2);
static DEFINE_GROUP(ps2, 28, 2);

static int group_irq_enable(struct group_irq *group_irq)
{
	unsigned long flags;

	spin_lock_irqsave(&groupirq_lock, flags);
	if (group_irq->enabled++ == 0)
		(group_irq->enable)(group_irq, 1);
	spin_unlock_irqrestore(&groupirq_lock, flags);

	return 0;
}

static void group_irq_disable(struct group_irq *group_irq)
{
	unsigned long flags;

	WARN_ON(group_irq->enabled == 0);

	spin_lock_irqsave(&groupirq_lock, flags);
	if (--group_irq->enabled == 0)
		(group_irq->enable)(group_irq, 0);
	spin_unlock_irqrestore(&groupirq_lock, flags);
}

static void nuc900_group_enable(struct group_irq *gpirq, int enable)
{
	unsigned int groupen = gpirq->gpen;
	unsigned long regval;

	regval = __raw_readl(REG_AIC_GEN);

	if (enable)
		regval |= groupen;
	else
		regval &= ~groupen;

	__raw_writel(regval, REG_AIC_GEN);
}

static void nuc900_irq_mask(unsigned int irq)
{
	struct group_irq *group_irq;

	group_irq = NULL;

	__raw_writel(1 << irq, REG_AIC_MDCR);

	switch (irq) {
	case IRQ_GROUP0:
		group_irq = &group_nirq0;
		break;

	case IRQ_GROUP1:
		group_irq = &group_nirq1;
		break;

	case IRQ_USBH:
		group_irq = &group_usbh;
		break;

	case IRQ_T_INT_GROUP:
		group_irq = &group_ottimer;
		break;

	case IRQ_GDMAGROUP:
		group_irq = &group_gdma;
		break;

	case IRQ_SCGROUP:
		group_irq = &group_sc;
		break;

	case IRQ_I2CGROUP:
		group_irq = &group_i2c;
		break;

	case IRQ_P2SGROUP:
		group_irq = &group_ps2;
		break;
	}

	if (group_irq)
		group_irq_disable(group_irq);
}



static void nuc900_irq_ack(unsigned int irq)
{
	__raw_writel(0x01, REG_AIC_EOSCR);
}

static void nuc900_irq_unmask(unsigned int irq)
{
	struct group_irq *group_irq;

	group_irq = NULL;

	__raw_writel(1 << irq, REG_AIC_MECR);

	switch (irq) {
	case IRQ_GROUP0:
		group_irq = &group_nirq0;
		break;

	case IRQ_GROUP1:
		group_irq = &group_nirq1;
		break;

	case IRQ_USBH:
		group_irq = &group_usbh;
		break;

	case IRQ_T_INT_GROUP:
		group_irq = &group_ottimer;
		break;

	case IRQ_GDMAGROUP:
		group_irq = &group_gdma;
		break;

	case IRQ_SCGROUP:
		group_irq = &group_sc;
		break;

	case IRQ_I2CGROUP:
		group_irq = &group_i2c;
		break;

	case IRQ_P2SGROUP:
		group_irq = &group_ps2;
		break;
	}

	if (group_irq)
		group_irq_enable(group_irq);
}

static struct irq_chip nuc900_irq_chip = {
	.ack	   = nuc900_irq_ack,
	.mask	   = nuc900_irq_mask,
	.unmask	   = nuc900_irq_unmask,
};

void __init nuc900_init_irq(void)
{
	int irqno;

	__raw_writel(0xFFFFFFFE, REG_AIC_MDCR);

	for (irqno = IRQ_WDT; irqno <= IRQ_ADC; irqno++) {
		set_irq_chip(irqno, &nuc900_irq_chip);
		set_irq_handler(irqno, handle_level_irq);
		set_irq_flags(irqno, IRQF_VALID);
	}
}
