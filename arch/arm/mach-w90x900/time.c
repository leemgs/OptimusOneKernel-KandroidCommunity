

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/leds.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>

#include <asm/mach-types.h>
#include <asm/mach/irq.h>
#include <asm/mach/time.h>

#include <mach/map.h>
#include <mach/regs-timer.h>

#define RESETINT	0x1f
#define PERIOD		(0x01 << 27)
#define ONESHOT		(0x00 << 27)
#define COUNTEN		(0x01 << 30)
#define INTEN		(0x01 << 29)

#define TICKS_PER_SEC	100
#define PRESCALE	0x63 

unsigned int timer0_load;

static void nuc900_clockevent_setmode(enum clock_event_mode mode,
		struct clock_event_device *clk)
{
	unsigned int val;

	val = __raw_readl(REG_TCSR0);
	val &= ~(0x03 << 27);

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		__raw_writel(timer0_load, REG_TICR0);
		val |= (PERIOD | COUNTEN | INTEN | PRESCALE);
		break;

	case CLOCK_EVT_MODE_ONESHOT:
		val |= (ONESHOT | COUNTEN | INTEN | PRESCALE);
		break;

	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	case CLOCK_EVT_MODE_RESUME:
		break;
	}

	__raw_writel(val, REG_TCSR0);
}

static int nuc900_clockevent_setnextevent(unsigned long evt,
		struct clock_event_device *clk)
{
	unsigned int val;

	__raw_writel(evt, REG_TICR0);

	val = __raw_readl(REG_TCSR0);
	val |= (COUNTEN | INTEN | PRESCALE);
	__raw_writel(val, REG_TCSR0);

	return 0;
}

static struct clock_event_device nuc900_clockevent_device = {
	.name		= "nuc900-timer0",
	.shift		= 32,
	.features	= CLOCK_EVT_MODE_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_mode	= nuc900_clockevent_setmode,
	.set_next_event	= nuc900_clockevent_setnextevent,
	.rating		= 300,
};



static irqreturn_t nuc900_timer0_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = &nuc900_clockevent_device;

	__raw_writel(0x01, REG_TISR); 

	evt->event_handler(evt);
	return IRQ_HANDLED;
}

static struct irqaction nuc900_timer0_irq = {
	.name		= "nuc900-timer0",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= nuc900_timer0_interrupt,
};

static void __init nuc900_clockevents_init(unsigned int rate)
{
	nuc900_clockevent_device.mult = div_sc(rate, NSEC_PER_SEC,
					nuc900_clockevent_device.shift);
	nuc900_clockevent_device.max_delta_ns = clockevent_delta2ns(0xffffffff,
					&nuc900_clockevent_device);
	nuc900_clockevent_device.min_delta_ns = clockevent_delta2ns(0xf,
					&nuc900_clockevent_device);
	nuc900_clockevent_device.cpumask = cpumask_of(0);

	clockevents_register_device(&nuc900_clockevent_device);
}

static cycle_t nuc900_get_cycles(struct clocksource *cs)
{
	return ~__raw_readl(REG_TDR1);
}

static struct clocksource clocksource_nuc900 = {
	.name	= "nuc900-timer1",
	.rating	= 200,
	.read	= nuc900_get_cycles,
	.mask	= CLOCKSOURCE_MASK(32),
	.shift	= 20,
	.flags	= CLOCK_SOURCE_IS_CONTINUOUS,
};

static void __init nuc900_clocksource_init(unsigned int rate)
{
	unsigned int val;

	__raw_writel(0xffffffff, REG_TICR1);

	val = __raw_readl(REG_TCSR1);
	val |= (COUNTEN | PERIOD);
	__raw_writel(val, REG_TCSR1);

	clocksource_nuc900.mult =
		clocksource_khz2mult((rate / 1000), clocksource_nuc900.shift);
	clocksource_register(&clocksource_nuc900);
}

static void __init nuc900_timer_init(void)
{
	struct clk *ck_ext = clk_get(NULL, "ext");
	unsigned int	rate;

	BUG_ON(IS_ERR(ck_ext));

	rate = clk_get_rate(ck_ext);
	clk_put(ck_ext);
	rate = rate / (PRESCALE + 0x01);

	 
	__raw_writel(0x00, REG_TCSR0);
	__raw_writel(0x00, REG_TCSR1);
	__raw_writel(RESETINT, REG_TISR);
	timer0_load = (rate / TICKS_PER_SEC);

	setup_irq(IRQ_TIMER0, &nuc900_timer0_irq);

	nuc900_clocksource_init(rate);
	nuc900_clockevents_init(rate);
}

struct sys_timer nuc900_timer = {
	.init		= nuc900_timer_init,
};
