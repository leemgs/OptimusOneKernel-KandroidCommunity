
#ifndef _LINUX_CLOCKSOURCE_H
#define _LINUX_CLOCKSOURCE_H

#include <linux/types.h>
#include <linux/timex.h>
#include <linux/time.h>
#include <linux/list.h>
#include <linux/cache.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <asm/div64.h>
#include <asm/io.h>


typedef u64 cycle_t;
struct clocksource;


struct cyclecounter {
	cycle_t (*read)(const struct cyclecounter *cc);
	cycle_t mask;
	u32 mult;
	u32 shift;
};


struct timecounter {
	const struct cyclecounter *cc;
	cycle_t cycle_last;
	u64 nsec;
};


static inline u64 cyclecounter_cyc2ns(const struct cyclecounter *cc,
				      cycle_t cycles)
{
	u64 ret = (u64)cycles;
	ret = (ret * cc->mult) >> cc->shift;
	return ret;
}


extern void timecounter_init(struct timecounter *tc,
			     const struct cyclecounter *cc,
			     u64 start_tstamp);


extern u64 timecounter_read(struct timecounter *tc);


extern u64 timecounter_cyc2time(struct timecounter *tc,
				cycle_t cycle_tstamp);


struct clocksource {
	
	char *name;
	struct list_head list;
	int rating;
	cycle_t (*read)(struct clocksource *cs);
	int (*enable)(struct clocksource *cs);
	void (*disable)(struct clocksource *cs);
	cycle_t mask;
	u32 mult;
	u32 shift;
	u64 max_idle_ns;
	unsigned long flags;
	cycle_t (*vread)(void);
	void (*resume)(void);
#ifdef CONFIG_IA64
	void *fsys_mmio;        
#define CLKSRC_FSYS_MMIO_SET(mmio, addr)      ((mmio) = (addr))
#else
#define CLKSRC_FSYS_MMIO_SET(mmio, addr)      do { } while (0)
#endif

	
	cycle_t cycle_last ____cacheline_aligned_in_smp;

#ifdef CONFIG_CLOCKSOURCE_WATCHDOG
	
	struct list_head wd_list;
	cycle_t wd_last;
#endif
};


#define CLOCK_SOURCE_IS_CONTINUOUS		0x01
#define CLOCK_SOURCE_MUST_VERIFY		0x02

#define CLOCK_SOURCE_WATCHDOG			0x10
#define CLOCK_SOURCE_VALID_FOR_HRES		0x20
#define CLOCK_SOURCE_UNSTABLE			0x40


#define CLOCKSOURCE_MASK(bits) (cycle_t)((bits) < 64 ? ((1ULL<<(bits))-1) : -1)


static inline u32 clocksource_khz2mult(u32 khz, u32 shift_constant)
{
	
	u64 tmp = ((u64)1000000) << shift_constant;

	tmp += khz/2; 
	do_div(tmp, khz);

	return (u32)tmp;
}


static inline u32 clocksource_hz2mult(u32 hz, u32 shift_constant)
{
	
	u64 tmp = ((u64)1000000000) << shift_constant;

	tmp += hz/2; 
	do_div(tmp, hz);

	return (u32)tmp;
}


static inline s64 clocksource_cyc2ns(cycle_t cycles, u32 mult, u32 shift)
{
	return ((u64) cycles * mult) >> shift;
}



extern int clocksource_register(struct clocksource*);
extern void clocksource_unregister(struct clocksource*);
extern void clocksource_touch_watchdog(void);
extern struct clocksource* clocksource_get_next(void);
extern void clocksource_change_rating(struct clocksource *cs, int rating);
extern void clocksource_resume(void);
extern struct clocksource * __init __weak clocksource_default_clock(void);
extern void clocksource_mark_unstable(struct clocksource *cs);

#ifdef CONFIG_GENERIC_TIME_VSYSCALL
extern void update_vsyscall(struct timespec *ts, struct clocksource *c);
extern void update_vsyscall_tz(void);
#else
static inline void update_vsyscall(struct timespec *ts, struct clocksource *c)
{
}

static inline void update_vsyscall_tz(void)
{
}
#endif

extern void timekeeping_notify(struct clocksource *clock);

#endif 
