

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/timex.h>
#include <linux/preempt.h>
#include <linux/delay.h>
#include <linux/init.h>

#include <asm/processor.h>
#include <asm/delay.h>
#include <asm/timer.h>

#ifdef CONFIG_SMP
# include <asm/smp.h>
#endif


static void delay_loop(unsigned long loops)
{
	asm volatile(
		"	test %0,%0	\n"
		"	jz 3f		\n"
		"	jmp 1f		\n"

		".align 16		\n"
		"1:	jmp 2f		\n"

		".align 16		\n"
		"2:	dec %0		\n"
		"	jnz 2b		\n"
		"3:	dec %0		\n"

		: 
		:"a" (loops)
	);
}


static void delay_tsc(unsigned long loops)
{
	unsigned long bclock, now;
	int cpu;

	preempt_disable();
	cpu = smp_processor_id();
	rdtsc_barrier();
	rdtscl(bclock);
	for (;;) {
		rdtsc_barrier();
		rdtscl(now);
		if ((now - bclock) >= loops)
			break;

		
		preempt_enable();
		rep_nop();
		preempt_disable();

		
		if (unlikely(cpu != smp_processor_id())) {
			loops -= (now - bclock);
			cpu = smp_processor_id();
			rdtsc_barrier();
			rdtscl(bclock);
		}
	}
	preempt_enable();
}


static void (*delay_fn)(unsigned long) = delay_loop;

void use_tsc_delay(void)
{
	delay_fn = delay_tsc;
}

int __devinit read_current_timer(unsigned long *timer_val)
{
	if (delay_fn == delay_tsc) {
		rdtscll(*timer_val);
		return 0;
	}
	return -1;
}

void __delay(unsigned long loops)
{
	delay_fn(loops);
}
EXPORT_SYMBOL(__delay);

inline void __const_udelay(unsigned long xloops)
{
	int d0;

	xloops *= 4;
	asm("mull %%edx"
		:"=d" (xloops), "=&a" (d0)
		:"1" (xloops), "0"
		(cpu_data(raw_smp_processor_id()).loops_per_jiffy * (HZ/4)));

	__delay(++xloops);
}
EXPORT_SYMBOL(__const_udelay);

void __udelay(unsigned long usecs)
{
	__const_udelay(usecs * 0x000010c7); 
}
EXPORT_SYMBOL(__udelay);

void __ndelay(unsigned long nsecs)
{
	__const_udelay(nsecs * 0x00005); 
}
EXPORT_SYMBOL(__ndelay);
