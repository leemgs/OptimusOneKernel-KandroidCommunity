

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <asm/types.h>
#include <mach/hardware.h>
#include <asm/irq.h>
#include <mach/irqs.h>
#include <asm/mach/irq.h>
#include <asm/mach/time.h>
#include "common.h"

static irqreturn_t
h7201_timer_interrupt(int irq, void *dev_id)
{
	CPU_REG (TIMER_VIRT, TIMER_TOPSTAT);
	timer_tick();

	return IRQ_HANDLED;
}

static struct irqaction h7201_timer_irq = {
	.name		= "h7201 Timer Tick",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= h7201_timer_interrupt,
};


void __init h7201_init_time(void)
{
	CPU_REG (TIMER_VIRT, TM0_PERIOD) = LATCH;
	CPU_REG (TIMER_VIRT, TM0_CTRL) = TM_RESET;
	CPU_REG (TIMER_VIRT, TM0_CTRL) = TM_REPEAT | TM_START;
	CPU_REG (TIMER_VIRT, TIMER_TOPCTRL) = ENABLE_TM0_INTR | TIMER_ENABLE_BIT;

	setup_irq(IRQ_TIMER0, &h7201_timer_irq);
}

struct sys_timer h7201_timer = {
	.init		= h7201_init_time,
	.offset		= h720x_gettimeoffset,
};
