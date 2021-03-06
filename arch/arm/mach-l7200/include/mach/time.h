
#ifndef _ASM_ARCH_TIME_H
#define _ASM_ARCH_TIME_H

#include <mach/irqs.h>


#define RTC_BASE	(IO_BASE_2 + 0x2000)


#define RTC_RTCDR	(*(volatile unsigned char *) (RTC_BASE + 0x000))
#define RTC_RTCMR	(*(volatile unsigned char *) (RTC_BASE + 0x004))
#define RTC_RTCS	(*(volatile unsigned char *) (RTC_BASE + 0x008))
#define RTC_RTCC	(*(volatile unsigned char *) (RTC_BASE + 0x008))
#define RTC_RTCDV	(*(volatile unsigned char *) (RTC_BASE + 0x00c))
#define RTC_RTCCR	(*(volatile unsigned char *) (RTC_BASE + 0x010))


#define RTC_RATE_32	0x00      
#define RTC_RATE_64	0x10      
#define RTC_RATE_128	0x20      
#define RTC_RATE_256	0x30      
#define RTC_EN_ALARM	0x01      
#define RTC_EN_TIC	0x04      
#define RTC_EN_STWDOG	0x08      


static irqreturn_t
timer_interrupt(int irq, void *dev_id)
{
	struct pt_regs *regs = get_irq_regs();
	do_timer(1);
#ifndef CONFIG_SMP
	update_process_times(user_mode(regs));
#endif
	do_profile(regs);
	RTC_RTCC = 0;				

	return IRQ_HANDLED;
}


void __init time_init(void)
{
	RTC_RTCC = 0;				

	timer_irq.handler = timer_interrupt;

	setup_irq(IRQ_RTC_TICK, &timer_irq);

	RTC_RTCCR = RTC_RATE_128 | RTC_EN_TIC;	
}

#endif
