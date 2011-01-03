
#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/timex.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/io.h>

#include <asm/i8253.h>
#include <asm/hpet.h>
#include <asm/smp.h>

DEFINE_SPINLOCK(i8253_lock);
EXPORT_SYMBOL(i8253_lock);


struct clock_event_device *global_clock_event;


static void init_pit_timer(enum clock_event_mode mode,
			   struct clock_event_device *evt)
{
	spin_lock(&i8253_lock);

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		
		outb_pit(0x34, PIT_MODE);
		outb_pit(LATCH & 0xff , PIT_CH0);	
		outb_pit(LATCH >> 8 , PIT_CH0);		
		break;

	case CLOCK_EVT_MODE_SHUTDOWN:
	case CLOCK_EVT_MODE_UNUSED:
		if (evt->mode == CLOCK_EVT_MODE_PERIODIC ||
		    evt->mode == CLOCK_EVT_MODE_ONESHOT) {
			outb_pit(0x30, PIT_MODE);
			outb_pit(0, PIT_CH0);
			outb_pit(0, PIT_CH0);
		}
		break;

	case CLOCK_EVT_MODE_ONESHOT:
		
		outb_pit(0x38, PIT_MODE);
		break;

	case CLOCK_EVT_MODE_RESUME:
		
		break;
	}
	spin_unlock(&i8253_lock);
}


static int pit_next_event(unsigned long delta, struct clock_event_device *evt)
{
	spin_lock(&i8253_lock);
	outb_pit(delta & 0xff , PIT_CH0);	
	outb_pit(delta >> 8 , PIT_CH0);		
	spin_unlock(&i8253_lock);

	return 0;
}


static struct clock_event_device pit_ce = {
	.name		= "pit",
	.features	= CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_mode	= init_pit_timer,
	.set_next_event = pit_next_event,
	.shift		= 32,
	.irq		= 0,
};


void __init setup_pit_timer(void)
{
	
	pit_ce.cpumask = cpumask_of(smp_processor_id());
	pit_ce.mult = div_sc(CLOCK_TICK_RATE, NSEC_PER_SEC, pit_ce.shift);
	pit_ce.max_delta_ns = clockevent_delta2ns(0x7FFF, &pit_ce);
	pit_ce.min_delta_ns = clockevent_delta2ns(0xF, &pit_ce);

	clockevents_register_device(&pit_ce);
	global_clock_event = &pit_ce;
}

#ifndef CONFIG_X86_64

static cycle_t pit_read(struct clocksource *cs)
{
	static int old_count;
	static u32 old_jifs;
	unsigned long flags;
	int count;
	u32 jifs;

	spin_lock_irqsave(&i8253_lock, flags);
	
	jifs = jiffies;
	outb_pit(0x00, PIT_MODE);	
	count = inb_pit(PIT_CH0);	
	count |= inb_pit(PIT_CH0) << 8;

	
	if (count > LATCH) {
		outb_pit(0x34, PIT_MODE);
		outb_pit(LATCH & 0xff, PIT_CH0);
		outb_pit(LATCH >> 8, PIT_CH0);
		count = LATCH - 1;
	}

	
	if (count > old_count && jifs == old_jifs)
		count = old_count;

	old_count = count;
	old_jifs = jifs;

	spin_unlock_irqrestore(&i8253_lock, flags);

	count = (LATCH - 1) - count;

	return (cycle_t)(jifs * LATCH) + count;
}

static struct clocksource pit_cs = {
	.name		= "pit",
	.rating		= 110,
	.read		= pit_read,
	.mask		= CLOCKSOURCE_MASK(32),
	.mult		= 0,
	.shift		= 20,
};

static int __init init_pit_clocksource(void)
{
	 
	if (num_possible_cpus() > 1 || is_hpet_enabled() ||
	    pit_ce.mode != CLOCK_EVT_MODE_PERIODIC)
		return 0;

	pit_cs.mult = clocksource_hz2mult(CLOCK_TICK_RATE, pit_cs.shift);

	return clocksource_register(&pit_cs);
}
arch_initcall(init_pit_clocksource);

#endif 
