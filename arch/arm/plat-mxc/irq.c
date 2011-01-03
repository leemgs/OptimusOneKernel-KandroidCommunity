

#include <linux/module.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <mach/common.h>
#include <asm/mach/irq.h>
#include <mach/hardware.h>

#define AVIC_INTCNTL		0x00	
#define AVIC_NIMASK		0x04	
#define AVIC_INTENNUM		0x08	
#define AVIC_INTDISNUM		0x0C	
#define AVIC_INTENABLEH		0x10	
#define AVIC_INTENABLEL		0x14	
#define AVIC_INTTYPEH		0x18	
#define AVIC_INTTYPEL		0x1C	
#define AVIC_NIPRIORITY(x)	(0x20 + 4 * (7 - (x))) 
#define AVIC_NIVECSR		0x40	
#define AVIC_FIVECSR		0x44	
#define AVIC_INTSRCH		0x48	
#define AVIC_INTSRCL		0x4C	
#define AVIC_INTFRCH		0x50	
#define AVIC_INTFRCL		0x54	
#define AVIC_NIPNDH		0x58	
#define AVIC_NIPNDL		0x5C	
#define AVIC_FIPNDH		0x60	
#define AVIC_FIPNDL		0x64	

void __iomem *avic_base;

int imx_irq_set_priority(unsigned char irq, unsigned char prio)
{
#ifdef CONFIG_MXC_IRQ_PRIOR
	unsigned int temp;
	unsigned int mask = 0x0F << irq % 8 * 4;

	if (irq >= MXC_INTERNAL_IRQS)
		return -EINVAL;;

	temp = __raw_readl(avic_base + AVIC_NIPRIORITY(irq / 8));
	temp &= ~mask;
	temp |= prio & mask;

	__raw_writel(temp, avic_base + AVIC_NIPRIORITY(irq / 8));

	return 0;
#else
	return -ENOSYS;
#endif
}
EXPORT_SYMBOL(imx_irq_set_priority);

#ifdef CONFIG_FIQ
int mxc_set_irq_fiq(unsigned int irq, unsigned int type)
{
	unsigned int irqt;

	if (irq >= MXC_INTERNAL_IRQS)
		return -EINVAL;

	if (irq < MXC_INTERNAL_IRQS / 2) {
		irqt = __raw_readl(avic_base + AVIC_INTTYPEL) & ~(1 << irq);
		__raw_writel(irqt | (!!type << irq), avic_base + AVIC_INTTYPEL);
	} else {
		irq -= MXC_INTERNAL_IRQS / 2;
		irqt = __raw_readl(avic_base + AVIC_INTTYPEH) & ~(1 << irq);
		__raw_writel(irqt | (!!type << irq), avic_base + AVIC_INTTYPEH);
	}

	return 0;
}
EXPORT_SYMBOL(mxc_set_irq_fiq);
#endif 


static void mxc_mask_irq(unsigned int irq)
{
	__raw_writel(irq, avic_base + AVIC_INTDISNUM);
}


static void mxc_unmask_irq(unsigned int irq)
{
	__raw_writel(irq, avic_base + AVIC_INTENNUM);
}

static struct irq_chip mxc_avic_chip = {
	.ack = mxc_mask_irq,
	.mask = mxc_mask_irq,
	.unmask = mxc_unmask_irq,
};


void __init mxc_init_irq(void __iomem *irqbase)
{
	int i;

	avic_base = irqbase;

	
	__raw_writel(0, avic_base + AVIC_INTCNTL);
	__raw_writel(0x1f, avic_base + AVIC_NIMASK);

	
	__raw_writel(0, avic_base + AVIC_INTENABLEH);
	__raw_writel(0, avic_base + AVIC_INTENABLEL);

	
	__raw_writel(0, avic_base + AVIC_INTTYPEH);
	__raw_writel(0, avic_base + AVIC_INTTYPEL);
	for (i = 0; i < MXC_INTERNAL_IRQS; i++) {
		set_irq_chip(i, &mxc_avic_chip);
		set_irq_handler(i, handle_level_irq);
		set_irq_flags(i, IRQF_VALID);
	}

	
	for (i = 0; i < 8; i++)
		__raw_writel(0, avic_base + AVIC_NIPRIORITY(i));

	
	mxc_register_gpios();

#ifdef CONFIG_FIQ
	
	init_FIQ();
#endif

	printk(KERN_INFO "MXC IRQ initialized\n");
}

