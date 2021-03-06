

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <plat/irq.h>

static void orion_irq_mask(u32 irq)
{
	void __iomem *maskaddr = get_irq_chip_data(irq);
	u32 mask;

	mask = readl(maskaddr);
	mask &= ~(1 << (irq & 31));
	writel(mask, maskaddr);
}

static void orion_irq_unmask(u32 irq)
{
	void __iomem *maskaddr = get_irq_chip_data(irq);
	u32 mask;

	mask = readl(maskaddr);
	mask |= 1 << (irq & 31);
	writel(mask, maskaddr);
}

static struct irq_chip orion_irq_chip = {
	.name		= "orion_irq",
	.mask		= orion_irq_mask,
	.mask_ack	= orion_irq_mask,
	.unmask		= orion_irq_unmask,
};

void __init orion_irq_init(unsigned int irq_start, void __iomem *maskaddr)
{
	unsigned int i;

	
	writel(0, maskaddr);

	
	for (i = 0; i < 32; i++) {
		unsigned int irq = irq_start + i;

		set_irq_chip(irq, &orion_irq_chip);
		set_irq_chip_data(irq, maskaddr);
		set_irq_handler(irq, handle_level_irq);
		irq_desc[irq].status |= IRQ_LEVEL;
		set_irq_flags(irq, IRQF_VALID);
	}
}
