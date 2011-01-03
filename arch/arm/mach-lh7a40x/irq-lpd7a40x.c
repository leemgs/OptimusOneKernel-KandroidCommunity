

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach/irq.h>
#include <mach/irqs.h>

#include "common.h"

static void lh7a40x_ack_cpld_irq (u32 irq)
{
	
}

static void lh7a40x_mask_cpld_irq (u32 irq)
{
	switch (irq) {
	case IRQ_LPD7A40X_ETH_INT:
		CPLD_INTERRUPTS = CPLD_INTERRUPTS | 0x4;
		break;
	case IRQ_LPD7A400_TS:
		CPLD_INTERRUPTS = CPLD_INTERRUPTS | 0x8;
		break;
	}
}

static void lh7a40x_unmask_cpld_irq (u32 irq)
{
	switch (irq) {
	case IRQ_LPD7A40X_ETH_INT:
		CPLD_INTERRUPTS = CPLD_INTERRUPTS & ~ 0x4;
		break;
	case IRQ_LPD7A400_TS:
		CPLD_INTERRUPTS = CPLD_INTERRUPTS & ~ 0x8;
		break;
	}
}

static struct irq_chip lh7a40x_cpld_chip = {
	.name	= "CPLD",
	.ack	= lh7a40x_ack_cpld_irq,
	.mask	= lh7a40x_mask_cpld_irq,
	.unmask	= lh7a40x_unmask_cpld_irq,
};

static void lh7a40x_cpld_handler (unsigned int irq, struct irq_desc *desc)
{
	unsigned int mask = CPLD_INTERRUPTS;

	desc->chip->ack (irq);

	if ((mask & 0x1) == 0)	
		generic_handle_irq(IRQ_LPD7A40X_ETH_INT);

	if ((mask & 0x2) == 0)	
		generic_handle_irq(IRQ_LPD7A400_TS);

	desc->chip->unmask (irq); 
}


  

void __init lh7a40x_init_board_irq (void)
{
	int irq;

		

	unsigned char cpld_version = CPLD_REVISION;
	int pinCPLD;

#if defined CONFIG_MACH_LPD7A404
	cpld_version = 0x34;	
#endif
	pinCPLD = (cpld_version == 0x28) ? 7 : 3;

		

	GPIO_PFDD	&= ~0x0f; 
	GPIO_INTTYPE1	&= ~0x0f; 
	GPIO_INTTYPE2	&= ~0x0f; 
	barrier ();
	GPIO_GPIOFINTEN |=  0x0f; 

		

	CPLD_INTERRUPTS	=   0x0c; 
	GPIO_PFDD	&= ~(1 << pinCPLD); 
	GPIO_INTTYPE1	|=  (1 << pinCPLD); 
	GPIO_INTTYPE2	&= ~(1 << pinCPLD); 
	barrier ();
	GPIO_GPIOFINTEN |=  (1 << pinCPLD); 

		

	for (irq = IRQ_BOARD_START;
	     irq < IRQ_BOARD_START + NR_IRQ_BOARD; ++irq) {
		set_irq_chip (irq, &lh7a40x_cpld_chip);
		set_irq_handler (irq, handle_edge_irq);
		set_irq_flags (irq, IRQF_VALID);
	}

	set_irq_chained_handler ((cpld_version == 0x28)
				 ? IRQ_CPLD_V28
				 : IRQ_CPLD_V34,
				 lh7a40x_cpld_handler);
}
