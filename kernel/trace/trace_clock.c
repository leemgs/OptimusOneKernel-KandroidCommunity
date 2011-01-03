
#include <linux/spinlock.h>
#include <linux/hardirq.h>
#include <linux/module.h>
#include <linux/percpu.h>
#include <linux/sched.h>
#include <linux/ktime.h>
#include <linux/trace_clock.h>


u64 notrace trace_clock_local(void)
{
	unsigned long flags;
	u64 clock;

	
	raw_local_irq_save(flags);
	clock = sched_clock();
	raw_local_irq_restore(flags);

	return clock;
}


u64 notrace trace_clock(void)
{
	return cpu_clock(raw_smp_processor_id());
}





static struct {
	u64 prev_time;
	raw_spinlock_t lock;
} trace_clock_struct ____cacheline_aligned_in_smp =
	{
		.lock = (raw_spinlock_t)__RAW_SPIN_LOCK_UNLOCKED,
	};

u64 notrace trace_clock_global(void)
{
	unsigned long flags;
	int this_cpu;
	u64 now;

	raw_local_irq_save(flags);

	this_cpu = raw_smp_processor_id();
	now = cpu_clock(this_cpu);
	
	if (unlikely(in_nmi()))
		goto out;

	__raw_spin_lock(&trace_clock_struct.lock);

	
	if ((s64)(now - trace_clock_struct.prev_time) < 0)
		now = trace_clock_struct.prev_time + 1;

	trace_clock_struct.prev_time = now;

	__raw_spin_unlock(&trace_clock_struct.lock);

 out:
	raw_local_irq_restore(flags);

	return now;
}
