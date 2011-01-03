



#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <asm/geode.h>

#define MFGPT_DEFAULT_IRQ	7

static struct mfgpt_timer_t {
	unsigned int avail:1;
} mfgpt_timers[MFGPT_MAX_TIMERS];



#define MFGPT_DIVISOR 16
#define MFGPT_SCALE  4     
#define MFGPT_HZ  (32768 / MFGPT_DIVISOR)
#define MFGPT_PERIODIC (MFGPT_HZ / HZ)


static int disable;
static int __init mfgpt_disable(char *s)
{
	disable = 1;
	return 1;
}
__setup("nomfgpt", mfgpt_disable);


static int __init mfgpt_fix(char *s)
{
	u32 val, dummy;

	
	val = 0xFF; dummy = 0;
	wrmsr(MSR_MFGPT_SETUP, val, dummy);
	return 1;
}
__setup("mfgptfix", mfgpt_fix);




static int timers = -1;

static void geode_mfgpt_detect(void)
{
	int i;
	u16 val;

	timers = 0;

	if (disable) {
		printk(KERN_INFO "geode-mfgpt:  MFGPT support is disabled\n");
		goto done;
	}

	if (!geode_get_dev_base(GEODE_DEV_MFGPT)) {
		printk(KERN_INFO "geode-mfgpt:  MFGPT LBAR is not set up\n");
		goto done;
	}

	for (i = 0; i < MFGPT_MAX_TIMERS; i++) {
		val = geode_mfgpt_read(i, MFGPT_REG_SETUP);
		if (!(val & MFGPT_SETUP_SETUP)) {
			mfgpt_timers[i].avail = 1;
			timers++;
		}
	}

done:
	printk(KERN_INFO "geode-mfgpt:  %d MFGPT timers available.\n", timers);
}

int geode_mfgpt_toggle_event(int timer, int cmp, int event, int enable)
{
	u32 msr, mask, value, dummy;
	int shift = (cmp == MFGPT_CMP1) ? 0 : 8;

	if (timer < 0 || timer >= MFGPT_MAX_TIMERS)
		return -EIO;

	
	switch (event) {
	case MFGPT_EVENT_RESET:
		
		msr = MSR_MFGPT_NR;
		mask = 1 << (timer + 24);
		break;

	case MFGPT_EVENT_NMI:
		msr = MSR_MFGPT_NR;
		mask = 1 << (timer + shift);
		break;

	case MFGPT_EVENT_IRQ:
		msr = MSR_MFGPT_IRQ;
		mask = 1 << (timer + shift);
		break;

	default:
		return -EIO;
	}

	rdmsr(msr, value, dummy);

	if (enable)
		value |= mask;
	else
		value &= ~mask;

	wrmsr(msr, value, dummy);
	return 0;
}
EXPORT_SYMBOL_GPL(geode_mfgpt_toggle_event);

int geode_mfgpt_set_irq(int timer, int cmp, int *irq, int enable)
{
	u32 zsel, lpc, dummy;
	int shift;

	if (timer < 0 || timer >= MFGPT_MAX_TIMERS)
		return -EIO;

	
	rdmsr(MSR_PIC_ZSEL_LOW, zsel, dummy);
	shift = ((cmp == MFGPT_CMP1 ? 0 : 4) + timer % 4) * 4;
	if (((zsel >> shift) & 0xF) == 2)
		return -EIO;

	
	if (!*irq)
		*irq = (zsel >> shift) & 0xF;
	if (!*irq)
		*irq = MFGPT_DEFAULT_IRQ;

	
	if (*irq < 1 || *irq == 2 || *irq > 15)
		return -EIO;
	rdmsr(MSR_PIC_IRQM_LPC, lpc, dummy);
	if (lpc & (1 << *irq))
		return -EIO;

	
	if (geode_mfgpt_toggle_event(timer, cmp, MFGPT_EVENT_IRQ, enable))
		return -EIO;
	if (enable) {
		zsel = (zsel & ~(0xF << shift)) | (*irq << shift);
		wrmsr(MSR_PIC_ZSEL_LOW, zsel, dummy);
	}

	return 0;
}

static int mfgpt_get(int timer)
{
	mfgpt_timers[timer].avail = 0;
	printk(KERN_INFO "geode-mfgpt:  Registered timer %d\n", timer);
	return timer;
}

int geode_mfgpt_alloc_timer(int timer, int domain)
{
	int i;

	if (timers == -1) {
		
		geode_mfgpt_detect();
	}

	if (!timers)
		return -1;

	if (timer >= MFGPT_MAX_TIMERS)
		return -1;

	if (timer < 0) {
		
		for (i = 0; i < MFGPT_MAX_TIMERS; i++) {
			if (mfgpt_timers[i].avail)
				return mfgpt_get(i);

			if (i == 5 && domain == MFGPT_DOMAIN_WORKING)
				break;
		}
	} else {
		
		if (mfgpt_timers[timer].avail)
			return mfgpt_get(timer);
	}

	
	return -1;
}
EXPORT_SYMBOL_GPL(geode_mfgpt_alloc_timer);


#ifdef CONFIG_GEODE_MFGPT_TIMER



#include <linux/clocksource.h>
#include <linux/clockchips.h>

static unsigned int mfgpt_tick_mode = CLOCK_EVT_MODE_SHUTDOWN;
static u16 mfgpt_event_clock;

static int irq;
static int __init mfgpt_setup(char *str)
{
	get_option(&str, &irq);
	return 1;
}
__setup("mfgpt_irq=", mfgpt_setup);

static void mfgpt_disable_timer(u16 clock)
{
	
	geode_mfgpt_write(clock, MFGPT_REG_SETUP, (u16) ~MFGPT_SETUP_CNTEN |
			MFGPT_SETUP_CMP1 | MFGPT_SETUP_CMP2);
}

static int mfgpt_next_event(unsigned long, struct clock_event_device *);
static void mfgpt_set_mode(enum clock_event_mode, struct clock_event_device *);

static struct clock_event_device mfgpt_clockevent = {
	.name = "mfgpt-timer",
	.features = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_mode = mfgpt_set_mode,
	.set_next_event = mfgpt_next_event,
	.rating = 250,
	.cpumask = cpu_all_mask,
	.shift = 32
};

static void mfgpt_start_timer(u16 delta)
{
	geode_mfgpt_write(mfgpt_event_clock, MFGPT_REG_CMP2, (u16) delta);
	geode_mfgpt_write(mfgpt_event_clock, MFGPT_REG_COUNTER, 0);

	geode_mfgpt_write(mfgpt_event_clock, MFGPT_REG_SETUP,
			  MFGPT_SETUP_CNTEN | MFGPT_SETUP_CMP2);
}

static void mfgpt_set_mode(enum clock_event_mode mode,
			   struct clock_event_device *evt)
{
	mfgpt_disable_timer(mfgpt_event_clock);

	if (mode == CLOCK_EVT_MODE_PERIODIC)
		mfgpt_start_timer(MFGPT_PERIODIC);

	mfgpt_tick_mode = mode;
}

static int mfgpt_next_event(unsigned long delta, struct clock_event_device *evt)
{
	mfgpt_start_timer(delta);
	return 0;
}

static irqreturn_t mfgpt_tick(int irq, void *dev_id)
{
	u16 val = geode_mfgpt_read(mfgpt_event_clock, MFGPT_REG_SETUP);

	
	if (!(val & (MFGPT_SETUP_SETUP  | MFGPT_SETUP_CMP2 | MFGPT_SETUP_CMP1)))
		return IRQ_NONE;

	
	mfgpt_disable_timer(mfgpt_event_clock);

	if (mfgpt_tick_mode == CLOCK_EVT_MODE_SHUTDOWN)
		return IRQ_HANDLED;

	
	geode_mfgpt_write(mfgpt_event_clock, MFGPT_REG_COUNTER, 0);

	

	if (mfgpt_tick_mode == CLOCK_EVT_MODE_PERIODIC) {
		geode_mfgpt_write(mfgpt_event_clock, MFGPT_REG_SETUP,
				  MFGPT_SETUP_CNTEN | MFGPT_SETUP_CMP2);
	}

	mfgpt_clockevent.event_handler(&mfgpt_clockevent);
	return IRQ_HANDLED;
}

static struct irqaction mfgptirq  = {
	.handler = mfgpt_tick,
	.flags = IRQF_DISABLED | IRQF_NOBALANCING | IRQF_TIMER,
	.name = "mfgpt-timer"
};

int __init mfgpt_timer_setup(void)
{
	int timer, ret;
	u16 val;

	timer = geode_mfgpt_alloc_timer(MFGPT_TIMER_ANY, MFGPT_DOMAIN_WORKING);
	if (timer < 0) {
		printk(KERN_ERR
		       "mfgpt-timer:  Could not allocate a MFPGT timer\n");
		return -ENODEV;
	}

	mfgpt_event_clock = timer;

	
	if (geode_mfgpt_setup_irq(mfgpt_event_clock, MFGPT_CMP2, &irq)) {
		printk(KERN_ERR "mfgpt-timer:  Could not set up IRQ %d\n", irq);
		return -EIO;
	}

	
	ret = setup_irq(irq, &mfgptirq);

	if (ret) {
		printk(KERN_ERR
		       "mfgpt-timer:  Unable to set up the interrupt.\n");
		goto err;
	}

	
	val = MFGPT_SCALE | (3 << 8);

	geode_mfgpt_write(mfgpt_event_clock, MFGPT_REG_SETUP, val);

	
	mfgpt_clockevent.mult = div_sc(MFGPT_HZ, NSEC_PER_SEC,
				       mfgpt_clockevent.shift);
	mfgpt_clockevent.min_delta_ns = clockevent_delta2ns(0xF,
			&mfgpt_clockevent);
	mfgpt_clockevent.max_delta_ns = clockevent_delta2ns(0xFFFE,
			&mfgpt_clockevent);

	printk(KERN_INFO
	       "mfgpt-timer:  Registering MFGPT timer %d as a clock event, using IRQ %d\n",
	       timer, irq);
	clockevents_register_device(&mfgpt_clockevent);

	return 0;

err:
	geode_mfgpt_release_irq(mfgpt_event_clock, MFGPT_CMP2, &irq);
	printk(KERN_ERR
	       "mfgpt-timer:  Unable to set up the MFGPT clock source\n");
	return -EIO;
}

#endif
