

#include <linux/irq.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/interrupt.h>

#include "internals.h"

#ifdef CONFIG_HARDIRQS_SW_RESEND


static DECLARE_BITMAP(irqs_resend, NR_IRQS);


static void resend_irqs(unsigned long arg)
{
	struct irq_desc *desc;
	int irq;

	while (!bitmap_empty(irqs_resend, nr_irqs)) {
		irq = find_first_bit(irqs_resend, nr_irqs);
		clear_bit(irq, irqs_resend);
		desc = irq_to_desc(irq);
		local_irq_disable();
		desc->handle_irq(irq, desc);
		local_irq_enable();
	}
}


static DECLARE_TASKLET(resend_tasklet, resend_irqs, 0);

#endif


void check_irq_resend(struct irq_desc *desc, unsigned int irq)
{
	unsigned int status = desc->status;

	
	desc->chip->enable(irq);

	
	if ((status & (IRQ_LEVEL | IRQ_PENDING | IRQ_REPLAY)) == IRQ_PENDING) {
		desc->status = (status & ~IRQ_PENDING) | IRQ_REPLAY;

		if (!desc->chip->retrigger || !desc->chip->retrigger(irq)) {
#ifdef CONFIG_HARDIRQS_SW_RESEND
			
			set_bit(irq, irqs_resend);
			tasklet_schedule(&resend_tasklet);
#endif
		}
	}
}
