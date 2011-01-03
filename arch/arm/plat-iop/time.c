

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/timex.h>
#include <linux/io.h>
#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/mach/irq.h>
#include <asm/mach/time.h>
#include <mach/time.h>

static unsigned long ticks_per_jiffy;
static unsigned long ticks_per_usec;
static unsigned long next_jiffy_time;

unsigned long iop_gettimeoffset(void)
{
	unsigned long offset, temp;

	
	asm volatile (
	"mrc	p15, 0, %0, c15, c1, 0\n\t"
	"tst	%0, #(1 << 6)\n\t"
	"orreq	%0, %0, #(1 << 6)\n\t"
	"mcreq	p15, 0, %0, c15, c1, 0\n\t"
#ifdef CONFIG_CPU_XSCALE
	"mrceq	p15, 0, %0, c15, c1, 0\n\t"
	"moveq	%0, %0\n\t"
	"subeq	pc, pc, #4\n\t"
#endif
	: "=r"(temp) : : "cc");

	offset = next_jiffy_time - read_tcr1();

	return offset / ticks_per_usec;
}

static irqreturn_t
iop_timer_interrupt(int irq, void *dev_id)
{
	write_tisr(1);

	while ((signed long)(next_jiffy_time - read_tcr1())
		>= ticks_per_jiffy) {
		timer_tick();
		next_jiffy_time -= ticks_per_jiffy;
	}

	return IRQ_HANDLED;
}

static struct irqaction iop_timer_irq = {
	.name		= "IOP Timer Tick",
	.handler	= iop_timer_interrupt,
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
};

static unsigned long iop_tick_rate;
unsigned long get_iop_tick_rate(void)
{
	return iop_tick_rate;
}
EXPORT_SYMBOL(get_iop_tick_rate);

void __init iop_init_time(unsigned long tick_rate)
{
	u32 timer_ctl;

	ticks_per_jiffy = DIV_ROUND_CLOSEST(tick_rate, HZ);
	ticks_per_usec = tick_rate / 1000000;
	next_jiffy_time = 0xffffffff;
	iop_tick_rate = tick_rate;

	timer_ctl = IOP_TMR_EN | IOP_TMR_PRIVILEGED |
			IOP_TMR_RELOAD | IOP_TMR_RATIO_1_1;

	
	write_trr0(ticks_per_jiffy - 1);
	write_tmr0(timer_ctl);
	write_trr1(0xffffffff);
	write_tmr1(timer_ctl);

	setup_irq(IRQ_IOP_TIMER0, &iop_timer_irq);
}
