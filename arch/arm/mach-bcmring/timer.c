

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <csp/tmrHw.h>

#include <mach/timer.h>










#define PROF_TIMER 1

timer_tick_rate_t timer_get_tick_rate(void)
{
	return tmrHw_getCountRate(PROF_TIMER);
}

timer_tick_count_t timer_get_tick_count(void)
{
	return tmrHw_GetCurrentCount(PROF_TIMER);	
}

timer_msec_t timer_ticks_to_msec(timer_tick_count_t ticks)
{
	static int tickRateMsec;

	if (tickRateMsec == 0) {
		tickRateMsec = timer_get_tick_rate() / 1000;
	}

	return ticks / tickRateMsec;
}

timer_msec_t timer_get_msec(void)
{
	return timer_ticks_to_msec(timer_get_tick_count());
}

EXPORT_SYMBOL(timer_get_tick_count);
EXPORT_SYMBOL(timer_ticks_to_msec);
EXPORT_SYMBOL(timer_get_tick_rate);
EXPORT_SYMBOL(timer_get_msec);
