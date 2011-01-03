

#include <linux/interrupt.h>
#include <linux/init.h>

#include <asm/irq.h>
#include <asm/mach/irq.h>
#include <asm/mach/hardware.h>
#include <asm/mach/irqs.h>

#include "common.h"

  

static u16 CPLD_IRQ_mask;	

static void
lh7a400_ack_cpld_irq (u32 irq)
{
	CPLD_CL_INT = 1 << (irq - IRQ_KEV7A400_CPLD);
}

static void
lh7a400_mask_cpld_irq (u32 irq)
{
	CPLD_IRQ_mask &= ~(1 << (irq - IRQ_KEV7A400_CPLD));
	CPLD_WR_PB_INT_MASK = CPLD_IRQ_mask;
}

static void
lh7a400_unmask_cpld_irq (u32 irq)
{
	CPLD_IRQ_mask |= 1 << (irq - IRQ_KEV7A400_CPLD);
	CPLD_WR_PB_INT_MASK = CPLD_IRQ_mask;
}

static struct
irq_chip lh7a400_cpld_chip = {
	.name	= "CPLD",
	.ack	= lh7a400_ack_cpld_irq,
	.mask	= lh7a400_mask_cpld_irq,
	.unmask	= lh7a400_unmask_cpld_irq,
};

static void
lh7a400_cpld_handler (unsigned int irq, struct irq_desc *desc)
{
	u32 mask = CPLD_LATCHED_INTS;
	irq = IRQ_KEV_7A400_CPLD;
	for (; mask; mask >>= 1, ++irq) {
		if (mask & 1)
			desc[irq].handle (irq, desc);
	}
}

  

void __init
lh7a400_init_board_irq (void)
{
	int irq;

	for (irq = IRQ_KEV7A400_CPLD;
	     irq < IRQ_KEV7A400_CPLD + NR_IRQ_KEV7A400_CPLD; ++irq) {
		set_irq_chip (irq, &lh7a400_cpld_chip);
		set_irq_handler (irq, handle_edge_irq);
		set_irq_flags (irq, IRQF_VALID);
	}
	set_irq_chained_handler (IRQ_CPLD, kev7a400_cpld_handler);

		
	CPLD_CL_INT = 0xff; 

    

	GPIO_GPIOINTEN = 0;		
	barrier();
	GPIO_INTTYPE1
		= (GPIO_INTR_PCC1_CD | GPIO_INTR_PCC1_CD); 
	GPIO_INTTYPE2 = 0;		
	GPIO_GPIOFEOI = 0xff;		
	GPIO_GPIOINTEN = 0xff;		

	init_FIQ();
}
