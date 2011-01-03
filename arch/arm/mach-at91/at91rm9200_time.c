

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/clockchips.h>

#include <asm/mach/time.h>

#include <mach/at91_st.h>

static unsigned long last_crtr;
static u32 irqmask;
static struct clock_event_device clkevt;


static inline unsigned long read_CRTR(void)
{
	unsigned long x1, x2;

	x1 = at91_sys_read(AT91_ST_CRTR);
	do {
		x2 = at91_sys_read(AT91_ST_CRTR);
		if (x1 == x2)
			break;
		x1 = x2;
	} while (1);
	return x1;
}


static irqreturn_t at91rm9200_timer_interrupt(int irq, void *dev_id)
{
	u32	sr = at91_sys_read(AT91_ST_SR) & irqmask;

	
	if (sr & AT91_ST_ALMS) {
		clkevt.event_handler(&clkevt);
		return IRQ_HANDLED;
	}

	
	if (sr & AT91_ST_PITS) {
		u32	crtr = read_CRTR();

		while (((crtr - last_crtr) & AT91_ST_CRTV) >= LATCH) {
			last_crtr += LATCH;
			clkevt.event_handler(&clkevt);
		}
		return IRQ_HANDLED;
	}

	
	return IRQ_NONE;
}

static struct irqaction at91rm9200_timer_irq = {
	.name		= "at91_tick",
	.flags		= IRQF_SHARED | IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= at91rm9200_timer_interrupt
};

static cycle_t read_clk32k(struct clocksource *cs)
{
	return read_CRTR();
}

static struct clocksource clk32k = {
	.name		= "32k_counter",
	.rating		= 150,
	.read		= read_clk32k,
	.mask		= CLOCKSOURCE_MASK(20),
	.shift		= 10,
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

static void
clkevt32k_mode(enum clock_event_mode mode, struct clock_event_device *dev)
{
	
	at91_sys_write(AT91_ST_IDR, AT91_ST_PITS | AT91_ST_ALMS);
	(void) at91_sys_read(AT91_ST_SR);

	last_crtr = read_CRTR();
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		
		irqmask = AT91_ST_PITS;
		at91_sys_write(AT91_ST_PIMR, LATCH);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		
		irqmask = AT91_ST_ALMS;
		at91_sys_write(AT91_ST_RTAR, last_crtr);
		break;
	case CLOCK_EVT_MODE_SHUTDOWN:
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_RESUME:
		irqmask = 0;
		break;
	}
	at91_sys_write(AT91_ST_IER, irqmask);
}

static int
clkevt32k_next_event(unsigned long delta, struct clock_event_device *dev)
{
	unsigned long	flags;
	u32		alm;
	int		status = 0;

	BUG_ON(delta < 2);

	
	raw_local_irq_save(flags);

	
	WARN_ON_ONCE(!raw_irqs_disabled_flags(flags));

	
	alm = read_CRTR();

	
	at91_sys_write(AT91_ST_RTAR, alm);
	(void) at91_sys_read(AT91_ST_SR);

	
	alm += delta;
	at91_sys_write(AT91_ST_RTAR, alm);

	raw_local_irq_restore(flags);
	return status;
}

static struct clock_event_device clkevt = {
	.name		= "at91_tick",
	.features	= CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.shift		= 32,
	.rating		= 150,
	.set_next_event	= clkevt32k_next_event,
	.set_mode	= clkevt32k_mode,
};


void __init at91rm9200_timer_init(void)
{
	
	at91_sys_write(AT91_ST_IDR,
		AT91_ST_PITS | AT91_ST_WDOVF | AT91_ST_RTTINC | AT91_ST_ALMS);
	(void) at91_sys_read(AT91_ST_SR);

	
	setup_irq(AT91_ID_SYS, &at91rm9200_timer_irq);

	
	at91_sys_write(AT91_ST_RTMR, 1);

	
	clkevt.mult = div_sc(AT91_SLOW_CLOCK, NSEC_PER_SEC, clkevt.shift);
	clkevt.max_delta_ns = clockevent_delta2ns(AT91_ST_ALMV, &clkevt);
	clkevt.min_delta_ns = clockevent_delta2ns(2, &clkevt) + 1;
	clkevt.cpumask = cpumask_of(0);
	clockevents_register_device(&clkevt);

	
	clk32k.mult = clocksource_hz2mult(AT91_SLOW_CLOCK, clk32k.shift);
	clocksource_register(&clk32k);
}

struct sys_timer at91rm9200_timer = {
	.init		= at91rm9200_timer_init,
};

