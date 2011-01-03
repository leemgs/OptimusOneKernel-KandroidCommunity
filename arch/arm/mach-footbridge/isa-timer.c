
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>

#include <asm/irq.h>

#include <asm/mach/time.h>

#include "common.h"


#define mSEC_10_from_14 ((14318180 + 100) / 200)

static unsigned long isa_gettimeoffset(void)
{
	int count;

	static int count_p = (mSEC_10_from_14/6);    
	static unsigned long jiffies_p = 0;

	
	unsigned long jiffies_t;

	
	outb_p(0x00, 0x43);	

	count = inb_p(0x40);	

	
 	jiffies_t = jiffies;

	count |= inb_p(0x40) << 8;

	
	if ((jiffies_t == jiffies_p) && (count > count_p))
		count -= (mSEC_10_from_14/6);
	else
		jiffies_p = jiffies_t;

	count_p = count;

	count = (((mSEC_10_from_14/6)-1) - count) * (tick_nsec / 1000);
	count = (count + (mSEC_10_from_14/6)/2) / (mSEC_10_from_14/6);

	return count;
}

static irqreturn_t
isa_timer_interrupt(int irq, void *dev_id)
{
	timer_tick();
	return IRQ_HANDLED;
}

static struct irqaction isa_timer_irq = {
	.name		= "ISA timer tick",
	.handler	= isa_timer_interrupt,
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
};

static void __init isa_timer_init(void)
{
	isa_rtc_init();

	
	
	outb(0x34, 0x43);
	outb((mSEC_10_from_14/6) & 0xFF, 0x40);
	outb((mSEC_10_from_14/6) >> 8, 0x40);

	setup_irq(IRQ_ISA_TIMER, &isa_timer_irq);
}

struct sys_timer isa_timer = {
	.init		= isa_timer_init,
	.offset		= isa_gettimeoffset,
};
