

#include <linux/init.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/io.h>

#include <mach/cp_intc.h>

static void __iomem *cp_intc_base;

static inline unsigned int cp_intc_read(unsigned offset)
{
	return __raw_readl(cp_intc_base + offset);
}

static inline void cp_intc_write(unsigned long value, unsigned offset)
{
	__raw_writel(value, cp_intc_base + offset);
}

static void cp_intc_ack_irq(unsigned int irq)
{
	cp_intc_write(irq, CP_INTC_SYS_STAT_IDX_CLR);
}


static void cp_intc_mask_irq(unsigned int irq)
{
	
	cp_intc_write(1, CP_INTC_HOST_ENABLE_IDX_CLR);
	cp_intc_write(irq, CP_INTC_SYS_ENABLE_IDX_CLR);
	cp_intc_write(1, CP_INTC_HOST_ENABLE_IDX_SET);
}


static void cp_intc_unmask_irq(unsigned int irq)
{
	cp_intc_write(irq, CP_INTC_SYS_ENABLE_IDX_SET);
}

static int cp_intc_set_irq_type(unsigned int irq, unsigned int flow_type)
{
	unsigned reg		= BIT_WORD(irq);
	unsigned mask		= BIT_MASK(irq);
	unsigned polarity	= cp_intc_read(CP_INTC_SYS_POLARITY(reg));
	unsigned type		= cp_intc_read(CP_INTC_SYS_TYPE(reg));

	switch (flow_type) {
	case IRQ_TYPE_EDGE_RISING:
		polarity |= mask;
		type |= mask;
		break;
	case IRQ_TYPE_EDGE_FALLING:
		polarity &= ~mask;
		type |= mask;
		break;
	case IRQ_TYPE_LEVEL_HIGH:
		polarity |= mask;
		type &= ~mask;
		break;
	case IRQ_TYPE_LEVEL_LOW:
		polarity &= ~mask;
		type &= ~mask;
		break;
	default:
		return -EINVAL;
	}

	cp_intc_write(polarity, CP_INTC_SYS_POLARITY(reg));
	cp_intc_write(type, CP_INTC_SYS_TYPE(reg));

	return 0;
}

static struct irq_chip cp_intc_irq_chip = {
	.name		= "cp_intc",
	.ack		= cp_intc_ack_irq,
	.mask		= cp_intc_mask_irq,
	.unmask		= cp_intc_unmask_irq,
	.set_type	= cp_intc_set_irq_type,
};

void __init cp_intc_init(void __iomem *base, unsigned short num_irq,
			 u8 *irq_prio)
{
	unsigned num_reg	= BITS_TO_LONGS(num_irq);
	int i;

	cp_intc_base = base;

	cp_intc_write(0, CP_INTC_GLOBAL_ENABLE);

	
	cp_intc_write(0, CP_INTC_HOST_ENABLE(0));

	
	for (i = 0; i < num_reg; i++)
		cp_intc_write(~0, CP_INTC_SYS_ENABLE_CLR(i));

	
	cp_intc_write(0, CP_INTC_CTRL);
	cp_intc_write(0, CP_INTC_HOST_CTRL);

	
	for (i = 0; i < num_reg; i++)
		cp_intc_write(~0, CP_INTC_SYS_STAT_CLR(i));

	
	cp_intc_write(1, CP_INTC_HOST_ENABLE_IDX_SET);

	
	num_reg = (num_irq + 3) >> 2;	
	if (irq_prio) {
		unsigned j, k;
		u32 val;

		for (k = i = 0; i < num_reg; i++) {
			for (val = j = 0; j < 4; j++, k++) {
				val >>= 8;
				if (k < num_irq)
					val |= irq_prio[k] << 24;
			}

			cp_intc_write(val, CP_INTC_CHAN_MAP(i));
		}
	} else	{
		
		for (i = 0; i < num_reg; i++)
			cp_intc_write(0x0f0f0f0f, CP_INTC_CHAN_MAP(i));
	}

	
	for (i = 0; i < num_irq; i++) {
		set_irq_chip(i, &cp_intc_irq_chip);
		set_irq_flags(i, IRQF_VALID | IRQF_PROBE);
		set_irq_handler(i, handle_edge_irq);
	}

	
	cp_intc_write(1, CP_INTC_GLOBAL_ENABLE);
}
