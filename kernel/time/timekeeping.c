

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/percpu.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/sysdev.h>
#include <linux/clocksource.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/tick.h>
#include <linux/stop_machine.h>


struct timekeeper {
	
	struct clocksource *clock;
	
	int	shift;

	
	cycle_t cycle_interval;
	
	u64	xtime_interval;
	
	u32	raw_interval;

	
	u64	xtime_nsec;
	
	s64	ntp_error;
	
	int	ntp_error_shift;
	
	u32	mult;
};

struct timekeeper timekeeper;


static void timekeeper_setup_internals(struct clocksource *clock)
{
	cycle_t interval;
	u64 tmp;

	timekeeper.clock = clock;
	clock->cycle_last = clock->read(clock);

	
	tmp = NTP_INTERVAL_LENGTH;
	tmp <<= clock->shift;
	tmp += clock->mult/2;
	do_div(tmp, clock->mult);
	if (tmp == 0)
		tmp = 1;

	interval = (cycle_t) tmp;
	timekeeper.cycle_interval = interval;

	
	timekeeper.xtime_interval = (u64) interval * clock->mult;
	timekeeper.raw_interval =
		((u64) interval * clock->mult) >> clock->shift;

	timekeeper.xtime_nsec = 0;
	timekeeper.shift = clock->shift;

	timekeeper.ntp_error = 0;
	timekeeper.ntp_error_shift = NTP_SCALE_SHIFT - clock->shift;

	
	timekeeper.mult = clock->mult;
}


static inline s64 timekeeping_get_ns(void)
{
	cycle_t cycle_now, cycle_delta;
	struct clocksource *clock;

	
	clock = timekeeper.clock;
	cycle_now = clock->read(clock);

	
	cycle_delta = (cycle_now - clock->cycle_last) & clock->mask;

	
	return clocksource_cyc2ns(cycle_delta, timekeeper.mult,
				  timekeeper.shift);
}

static inline s64 timekeeping_get_ns_raw(void)
{
	cycle_t cycle_now, cycle_delta;
	struct clocksource *clock;

	
	clock = timekeeper.clock;
	cycle_now = clock->read(clock);

	
	cycle_delta = (cycle_now - clock->cycle_last) & clock->mask;

	
	return clocksource_cyc2ns(cycle_delta, clock->mult, clock->shift);
}


__cacheline_aligned_in_smp DEFINE_SEQLOCK(xtime_lock);



struct timespec xtime __attribute__ ((aligned (16)));
struct timespec wall_to_monotonic __attribute__ ((aligned (16)));
static struct timespec total_sleep_time;


struct timespec raw_time;


int __read_mostly timekeeping_suspended;

static struct timespec xtime_cache __attribute__ ((aligned (16)));
void update_xtime_cache(u64 nsec)
{
	xtime_cache = xtime;
	timespec_add_ns(&xtime_cache, nsec);
}


void timekeeping_leap_insert(int leapsecond)
{
	xtime.tv_sec += leapsecond;
	wall_to_monotonic.tv_sec -= leapsecond;
	update_vsyscall(&xtime, timekeeper.clock);
}

#ifdef CONFIG_GENERIC_TIME


static void timekeeping_forward_now(void)
{
	cycle_t cycle_now, cycle_delta;
	struct clocksource *clock;
	s64 nsec;

	clock = timekeeper.clock;
	cycle_now = clock->read(clock);
	cycle_delta = (cycle_now - clock->cycle_last) & clock->mask;
	clock->cycle_last = cycle_now;

	nsec = clocksource_cyc2ns(cycle_delta, timekeeper.mult,
				  timekeeper.shift);

	
	nsec += arch_gettimeoffset();

	timespec_add_ns(&xtime, nsec);

	nsec = clocksource_cyc2ns(cycle_delta, clock->mult, clock->shift);
	timespec_add_ns(&raw_time, nsec);
}


void getnstimeofday(struct timespec *ts)
{
	unsigned long seq;
	s64 nsecs;

	WARN_ON(timekeeping_suspended);

	do {
		seq = read_seqbegin(&xtime_lock);

		*ts = xtime;
		nsecs = timekeeping_get_ns();

		
		nsecs += arch_gettimeoffset();

	} while (read_seqretry(&xtime_lock, seq));

	timespec_add_ns(ts, nsecs);
}

EXPORT_SYMBOL(getnstimeofday);

ktime_t ktime_get(void)
{
	unsigned int seq;
	s64 secs, nsecs;

	WARN_ON(timekeeping_suspended);

	do {
		seq = read_seqbegin(&xtime_lock);
		secs = xtime.tv_sec + wall_to_monotonic.tv_sec;
		nsecs = xtime.tv_nsec + wall_to_monotonic.tv_nsec;
		nsecs += timekeeping_get_ns();

	} while (read_seqretry(&xtime_lock, seq));
	
	return ktime_add_ns(ktime_set(secs, 0), nsecs);
}
EXPORT_SYMBOL_GPL(ktime_get);


void ktime_get_ts(struct timespec *ts)
{
	struct timespec tomono;
	unsigned int seq;
	s64 nsecs;

	WARN_ON(timekeeping_suspended);

	do {
		seq = read_seqbegin(&xtime_lock);
		*ts = xtime;
		tomono = wall_to_monotonic;
		nsecs = timekeeping_get_ns();

	} while (read_seqretry(&xtime_lock, seq));

	set_normalized_timespec(ts, ts->tv_sec + tomono.tv_sec,
				ts->tv_nsec + tomono.tv_nsec + nsecs);
}
EXPORT_SYMBOL_GPL(ktime_get_ts);


void do_gettimeofday(struct timeval *tv)
{
	struct timespec now;

	getnstimeofday(&now);
	tv->tv_sec = now.tv_sec;
	tv->tv_usec = now.tv_nsec/1000;
}

EXPORT_SYMBOL(do_gettimeofday);

int do_settimeofday(struct timespec *tv)
{
	struct timespec ts_delta;
	unsigned long flags;

	if ((unsigned long)tv->tv_nsec >= NSEC_PER_SEC)
		return -EINVAL;

	write_seqlock_irqsave(&xtime_lock, flags);

	timekeeping_forward_now();

	ts_delta.tv_sec = tv->tv_sec - xtime.tv_sec;
	ts_delta.tv_nsec = tv->tv_nsec - xtime.tv_nsec;
	wall_to_monotonic = timespec_sub(wall_to_monotonic, ts_delta);

	xtime = *tv;

	update_xtime_cache(0);

	timekeeper.ntp_error = 0;
	ntp_clear();

	update_vsyscall(&xtime, timekeeper.clock);

	write_sequnlock_irqrestore(&xtime_lock, flags);

	
	clock_was_set();

	return 0;
}

EXPORT_SYMBOL(do_settimeofday);


static int change_clocksource(void *data)
{
	struct clocksource *new, *old;

	new = (struct clocksource *) data;

	timekeeping_forward_now();
	if (!new->enable || new->enable(new) == 0) {
		old = timekeeper.clock;
		timekeeper_setup_internals(new);
		if (old->disable)
			old->disable(old);
	}
	return 0;
}


void timekeeping_notify(struct clocksource *clock)
{
	if (timekeeper.clock == clock)
		return;
	stop_machine(change_clocksource, clock, NULL);
	tick_clock_notify();
}

#else 

static inline void timekeeping_forward_now(void) { }


ktime_t ktime_get(void)
{
	struct timespec now;

	ktime_get_ts(&now);

	return timespec_to_ktime(now);
}
EXPORT_SYMBOL_GPL(ktime_get);


void ktime_get_ts(struct timespec *ts)
{
	struct timespec tomono;
	unsigned long seq;

	do {
		seq = read_seqbegin(&xtime_lock);
		getnstimeofday(ts);
		tomono = wall_to_monotonic;

	} while (read_seqretry(&xtime_lock, seq));

	set_normalized_timespec(ts, ts->tv_sec + tomono.tv_sec,
				ts->tv_nsec + tomono.tv_nsec);
}
EXPORT_SYMBOL_GPL(ktime_get_ts);

#endif 


ktime_t ktime_get_real(void)
{
	struct timespec now;

	getnstimeofday(&now);

	return timespec_to_ktime(now);
}
EXPORT_SYMBOL_GPL(ktime_get_real);


void getrawmonotonic(struct timespec *ts)
{
	unsigned long seq;
	s64 nsecs;

	do {
		seq = read_seqbegin(&xtime_lock);
		nsecs = timekeeping_get_ns_raw();
		*ts = raw_time;

	} while (read_seqretry(&xtime_lock, seq));

	timespec_add_ns(ts, nsecs);
}
EXPORT_SYMBOL(getrawmonotonic);



int timekeeping_valid_for_hres(void)
{
	unsigned long seq;
	int ret;

	do {
		seq = read_seqbegin(&xtime_lock);

		ret = timekeeper.clock->flags & CLOCK_SOURCE_VALID_FOR_HRES;

	} while (read_seqretry(&xtime_lock, seq));

	return ret;
}


u64 timekeeping_max_deferment(void)
{
	return timekeeper.clock->max_idle_ns;
}


void __attribute__((weak)) read_persistent_clock(struct timespec *ts)
{
	ts->tv_sec = 0;
	ts->tv_nsec = 0;
}


void __attribute__((weak)) read_boot_clock(struct timespec *ts)
{
	ts->tv_sec = 0;
	ts->tv_nsec = 0;
}


void __init timekeeping_init(void)
{
	struct clocksource *clock;
	unsigned long flags;
	struct timespec now, boot;

	read_persistent_clock(&now);
	read_boot_clock(&boot);

	write_seqlock_irqsave(&xtime_lock, flags);

	ntp_init();

	clock = clocksource_default_clock();
	if (clock->enable)
		clock->enable(clock);
	timekeeper_setup_internals(clock);

	xtime.tv_sec = now.tv_sec;
	xtime.tv_nsec = now.tv_nsec;
	raw_time.tv_sec = 0;
	raw_time.tv_nsec = 0;
	if (boot.tv_sec == 0 && boot.tv_nsec == 0) {
		boot.tv_sec = xtime.tv_sec;
		boot.tv_nsec = xtime.tv_nsec;
	}
	set_normalized_timespec(&wall_to_monotonic,
				-boot.tv_sec, -boot.tv_nsec);
	update_xtime_cache(0);
	total_sleep_time.tv_sec = 0;
	total_sleep_time.tv_nsec = 0;
	write_sequnlock_irqrestore(&xtime_lock, flags);
}


static struct timespec timekeeping_suspend_time;


static int timekeeping_resume(struct sys_device *dev)
{
	unsigned long flags;
	struct timespec ts;

	read_persistent_clock(&ts);

	clocksource_resume();

	write_seqlock_irqsave(&xtime_lock, flags);

	if (timespec_compare(&ts, &timekeeping_suspend_time) > 0) {
		ts = timespec_sub(ts, timekeeping_suspend_time);
		xtime = timespec_add_safe(xtime, ts);
		wall_to_monotonic = timespec_sub(wall_to_monotonic, ts);
		total_sleep_time = timespec_add_safe(total_sleep_time, ts);
	}
	update_xtime_cache(0);
	
	timekeeper.clock->cycle_last = timekeeper.clock->read(timekeeper.clock);
	timekeeper.ntp_error = 0;
	timekeeping_suspended = 0;
	write_sequnlock_irqrestore(&xtime_lock, flags);

	touch_softlockup_watchdog();

	clockevents_notify(CLOCK_EVT_NOTIFY_RESUME, NULL);

	
	hres_timers_resume();

	return 0;
}

static int timekeeping_suspend(struct sys_device *dev, pm_message_t state)
{
	unsigned long flags;

	read_persistent_clock(&timekeeping_suspend_time);

	write_seqlock_irqsave(&xtime_lock, flags);
	timekeeping_forward_now();
	timekeeping_suspended = 1;
	write_sequnlock_irqrestore(&xtime_lock, flags);

	clockevents_notify(CLOCK_EVT_NOTIFY_SUSPEND, NULL);

	return 0;
}


static struct sysdev_class timekeeping_sysclass = {
	.name		= "timekeeping",
	.resume		= timekeeping_resume,
	.suspend	= timekeeping_suspend,
};

static struct sys_device device_timer = {
	.id		= 0,
	.cls		= &timekeeping_sysclass,
};

static int __init timekeeping_init_device(void)
{
	int error = sysdev_class_register(&timekeeping_sysclass);
	if (!error)
		error = sysdev_register(&device_timer);
	return error;
}

device_initcall(timekeeping_init_device);


static __always_inline int timekeeping_bigadjust(s64 error, s64 *interval,
						 s64 *offset)
{
	s64 tick_error, i;
	u32 look_ahead, adj;
	s32 error2, mult;

	
	error2 = timekeeper.ntp_error >> (NTP_SCALE_SHIFT + 22 - 2 * SHIFT_HZ);
	error2 = abs(error2);
	for (look_ahead = 0; error2 > 0; look_ahead++)
		error2 >>= 2;

	
	tick_error = tick_length >> (timekeeper.ntp_error_shift + 1);
	tick_error -= timekeeper.xtime_interval >> 1;
	error = ((error - tick_error) >> look_ahead) + tick_error;

	
	i = *interval;
	mult = 1;
	if (error < 0) {
		error = -error;
		*interval = -*interval;
		*offset = -*offset;
		mult = -1;
	}
	for (adj = 0; error > i; adj++)
		error >>= 1;

	*interval <<= adj;
	*offset <<= adj;
	return mult << adj;
}


static void timekeeping_adjust(s64 offset)
{
	s64 error, interval = timekeeper.cycle_interval;
	int adj;

	error = timekeeper.ntp_error >> (timekeeper.ntp_error_shift - 1);
	if (error > interval) {
		error >>= 2;
		if (likely(error <= interval))
			adj = 1;
		else
			adj = timekeeping_bigadjust(error, &interval, &offset);
	} else if (error < -interval) {
		error >>= 2;
		if (likely(error >= -interval)) {
			adj = -1;
			interval = -interval;
			offset = -offset;
		} else
			adj = timekeeping_bigadjust(error, &interval, &offset);
	} else
		return;

	timekeeper.mult += adj;
	timekeeper.xtime_interval += interval;
	timekeeper.xtime_nsec -= offset;
	timekeeper.ntp_error -= (interval - offset) <<
				timekeeper.ntp_error_shift;
}


void update_wall_time(void)
{
	struct clocksource *clock;
	cycle_t offset;
	u64 nsecs;

	
	if (unlikely(timekeeping_suspended))
		return;

	clock = timekeeper.clock;
#ifdef CONFIG_GENERIC_TIME
	offset = (clock->read(clock) - clock->cycle_last) & clock->mask;
#else
	offset = timekeeper.cycle_interval;
#endif
	timekeeper.xtime_nsec = (s64)xtime.tv_nsec << timekeeper.shift;

	
	while (offset >= timekeeper.cycle_interval) {
		u64 nsecps = (u64)NSEC_PER_SEC << timekeeper.shift;

		
		offset -= timekeeper.cycle_interval;
		clock->cycle_last += timekeeper.cycle_interval;

		timekeeper.xtime_nsec += timekeeper.xtime_interval;
		if (timekeeper.xtime_nsec >= nsecps) {
			timekeeper.xtime_nsec -= nsecps;
			xtime.tv_sec++;
			second_overflow();
		}

		raw_time.tv_nsec += timekeeper.raw_interval;
		if (raw_time.tv_nsec >= NSEC_PER_SEC) {
			raw_time.tv_nsec -= NSEC_PER_SEC;
			raw_time.tv_sec++;
		}

		
		timekeeper.ntp_error += tick_length;
		timekeeper.ntp_error -= timekeeper.xtime_interval <<
					timekeeper.ntp_error_shift;
	}

	
	timekeeping_adjust(offset);

	
	if (unlikely((s64)timekeeper.xtime_nsec < 0)) {
		s64 neg = -(s64)timekeeper.xtime_nsec;
		timekeeper.xtime_nsec = 0;
		timekeeper.ntp_error += neg << timekeeper.ntp_error_shift;
	}

	
	xtime.tv_nsec =	((s64) timekeeper.xtime_nsec >> timekeeper.shift) + 1;
	timekeeper.xtime_nsec -= (s64) xtime.tv_nsec << timekeeper.shift;
	timekeeper.ntp_error +=	timekeeper.xtime_nsec <<
				timekeeper.ntp_error_shift;

	nsecs = clocksource_cyc2ns(offset, timekeeper.mult, timekeeper.shift);
	update_xtime_cache(nsecs);

	
	update_vsyscall(&xtime, timekeeper.clock);
}


void getboottime(struct timespec *ts)
{
	struct timespec boottime = {
		.tv_sec = wall_to_monotonic.tv_sec + total_sleep_time.tv_sec,
		.tv_nsec = wall_to_monotonic.tv_nsec + total_sleep_time.tv_nsec
	};

	set_normalized_timespec(ts, -boottime.tv_sec, -boottime.tv_nsec);
}
EXPORT_SYMBOL_GPL(getboottime);


void monotonic_to_bootbased(struct timespec *ts)
{
	*ts = timespec_add_safe(*ts, total_sleep_time);
}
EXPORT_SYMBOL_GPL(monotonic_to_bootbased);

unsigned long get_seconds(void)
{
	return xtime_cache.tv_sec;
}
EXPORT_SYMBOL(get_seconds);

struct timespec __current_kernel_time(void)
{
	return xtime_cache;
}

struct timespec current_kernel_time(void)
{
	struct timespec now;
	unsigned long seq;

	do {
		seq = read_seqbegin(&xtime_lock);

		now = xtime_cache;
	} while (read_seqretry(&xtime_lock, seq));

	return now;
}
EXPORT_SYMBOL(current_kernel_time);

struct timespec get_monotonic_coarse(void)
{
	struct timespec now, mono;
	unsigned long seq;

	do {
		seq = read_seqbegin(&xtime_lock);

		now = xtime_cache;
		mono = wall_to_monotonic;
	} while (read_seqretry(&xtime_lock, seq));

	set_normalized_timespec(&now, now.tv_sec + mono.tv_sec,
				now.tv_nsec + mono.tv_nsec);
	return now;
}
