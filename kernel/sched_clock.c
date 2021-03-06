
#include <linux/spinlock.h>
#include <linux/hardirq.h>
#include <linux/module.h>
#include <linux/percpu.h>
#include <linux/ktime.h>
#include <linux/sched.h>


unsigned long long __attribute__((weak)) sched_clock(void)
{
	return (unsigned long long)(jiffies - INITIAL_JIFFIES)
					* (NSEC_PER_SEC / HZ);
}

static __read_mostly int sched_clock_running;

#ifdef CONFIG_HAVE_UNSTABLE_SCHED_CLOCK
__read_mostly int sched_clock_stable;

struct sched_clock_data {
	u64			tick_raw;
	u64			tick_gtod;
	u64			clock;
};

static DEFINE_PER_CPU_SHARED_ALIGNED(struct sched_clock_data, sched_clock_data);

static inline struct sched_clock_data *this_scd(void)
{
	return &__get_cpu_var(sched_clock_data);
}

static inline struct sched_clock_data *cpu_sdc(int cpu)
{
	return &per_cpu(sched_clock_data, cpu);
}

void sched_clock_init(void)
{
	u64 ktime_now = ktime_to_ns(ktime_get());
	int cpu;

	for_each_possible_cpu(cpu) {
		struct sched_clock_data *scd = cpu_sdc(cpu);

		scd->tick_raw = 0;
		scd->tick_gtod = ktime_now;
		scd->clock = ktime_now;
	}

	sched_clock_running = 1;
}



static inline u64 wrap_min(u64 x, u64 y)
{
	return (s64)(x - y) < 0 ? x : y;
}

static inline u64 wrap_max(u64 x, u64 y)
{
	return (s64)(x - y) > 0 ? x : y;
}


static u64 sched_clock_local(struct sched_clock_data *scd)
{
	u64 now, clock, old_clock, min_clock, max_clock;
	s64 delta;

again:
	now = sched_clock();
	delta = now - scd->tick_raw;
	if (unlikely(delta < 0))
		delta = 0;

	old_clock = scd->clock;

	

	clock = scd->tick_gtod + delta;
	min_clock = wrap_max(scd->tick_gtod, old_clock);
	max_clock = wrap_max(old_clock, scd->tick_gtod + TICK_NSEC);

	clock = wrap_max(clock, min_clock);
	clock = wrap_min(clock, max_clock);

	if (cmpxchg64(&scd->clock, old_clock, clock) != old_clock)
		goto again;

	return clock;
}

static u64 sched_clock_remote(struct sched_clock_data *scd)
{
	struct sched_clock_data *my_scd = this_scd();
	u64 this_clock, remote_clock;
	u64 *ptr, old_val, val;

	sched_clock_local(my_scd);
again:
	this_clock = my_scd->clock;
	remote_clock = scd->clock;

	
	if (likely((s64)(remote_clock - this_clock) < 0)) {
		ptr = &scd->clock;
		old_val = remote_clock;
		val = this_clock;
	} else {
		
		ptr = &my_scd->clock;
		old_val = this_clock;
		val = remote_clock;
	}

	if (cmpxchg64(ptr, old_val, val) != old_val)
		goto again;

	return val;
}

u64 sched_clock_cpu(int cpu)
{
	struct sched_clock_data *scd;
	u64 clock;

	WARN_ON_ONCE(!irqs_disabled());

	if (sched_clock_stable)
		return sched_clock();

	if (unlikely(!sched_clock_running))
		return 0ull;

	scd = cpu_sdc(cpu);

	if (cpu != smp_processor_id())
		clock = sched_clock_remote(scd);
	else
		clock = sched_clock_local(scd);

	return clock;
}

void sched_clock_tick(void)
{
	struct sched_clock_data *scd;
	u64 now, now_gtod;

	if (sched_clock_stable)
		return;

	if (unlikely(!sched_clock_running))
		return;

	WARN_ON_ONCE(!irqs_disabled());

	scd = this_scd();
	now_gtod = ktime_to_ns(ktime_get());
	now = sched_clock();

	scd->tick_raw = now;
	scd->tick_gtod = now_gtod;
	sched_clock_local(scd);
}


void sched_clock_idle_sleep_event(void)
{
	sched_clock_cpu(smp_processor_id());
}
EXPORT_SYMBOL_GPL(sched_clock_idle_sleep_event);


void sched_clock_idle_wakeup_event(u64 delta_ns)
{
	if (timekeeping_suspended)
		return;

	sched_clock_tick();
	touch_softlockup_watchdog();
}
EXPORT_SYMBOL_GPL(sched_clock_idle_wakeup_event);

unsigned long long cpu_clock(int cpu)
{
	unsigned long long clock;
	unsigned long flags;

	local_irq_save(flags);
	clock = sched_clock_cpu(cpu);
	local_irq_restore(flags);

	return clock;
}

#else 

void sched_clock_init(void)
{
	sched_clock_running = 1;
}

u64 sched_clock_cpu(int cpu)
{
	if (unlikely(!sched_clock_running))
		return 0;

	return sched_clock();
}


unsigned long long cpu_clock(int cpu)
{
	return sched_clock_cpu(cpu);
}

#endif 

EXPORT_SYMBOL_GPL(cpu_clock);
