

#if !defined(__ASM_ARCH_OMAP_TIMEX_H)
#define __ASM_ARCH_OMAP_TIMEX_H


#ifdef CONFIG_OMAP_32K_TIMER
#define CLOCK_TICK_RATE		(CONFIG_OMAP_32K_TIMER_HZ)
#else
#define CLOCK_TICK_RATE		(HZ * 100000UL)
#endif

#endif 
