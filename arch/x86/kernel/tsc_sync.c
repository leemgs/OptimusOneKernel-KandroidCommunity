
#include <linux/spinlock.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/smp.h>
#include <linux/nmi.h>
#include <asm/tsc.h>


static __cpuinitdata atomic_t start_count;
static __cpuinitdata atomic_t stop_count;


static __cpuinitdata raw_spinlock_t sync_lock = __RAW_SPIN_LOCK_UNLOCKED;

static __cpuinitdata cycles_t last_tsc;
static __cpuinitdata cycles_t max_warp;
static __cpuinitdata int nr_warps;


static __cpuinit void check_tsc_warp(void)
{
	cycles_t start, now, prev, end;
	int i;

	rdtsc_barrier();
	start = get_cycles();
	rdtsc_barrier();
	
	end = start + tsc_khz * 20ULL;
	now = start;

	for (i = 0; ; i++) {
		
		__raw_spin_lock(&sync_lock);
		prev = last_tsc;
		rdtsc_barrier();
		now = get_cycles();
		rdtsc_barrier();
		last_tsc = now;
		__raw_spin_unlock(&sync_lock);

		
		if (unlikely(!(i & 7))) {
			if (now > end || i > 10000000)
				break;
			cpu_relax();
			touch_nmi_watchdog();
		}
		
		if (unlikely(prev > now)) {
			__raw_spin_lock(&sync_lock);
			max_warp = max(max_warp, prev - now);
			nr_warps++;
			__raw_spin_unlock(&sync_lock);
		}
	}
	WARN(!(now-start),
		"Warning: zero tsc calibration delta: %Ld [max: %Ld]\n",
			now-start, end-start);
}


void __cpuinit check_tsc_sync_source(int cpu)
{
	int cpus = 2;

	
	if (unsynchronized_tsc())
		return;

	if (boot_cpu_has(X86_FEATURE_TSC_RELIABLE)) {
		printk_once(KERN_INFO "Skipping synchronization checks as TSC is reliable.\n");
		return;
	}

	pr_info("checking TSC synchronization [CPU#%d -> CPU#%d]:",
		smp_processor_id(), cpu);

	
	atomic_set(&stop_count, 0);

	
	while (atomic_read(&start_count) != cpus-1)
		cpu_relax();
	
	atomic_inc(&start_count);

	check_tsc_warp();

	while (atomic_read(&stop_count) != cpus-1)
		cpu_relax();

	if (nr_warps) {
		printk("\n");
		pr_warning("Measured %Ld cycles TSC warp between CPUs, "
			   "turning off TSC clock.\n", max_warp);
		mark_tsc_unstable("check_tsc_sync_source failed");
	} else {
		printk(" passed.\n");
	}

	
	atomic_set(&start_count, 0);
	nr_warps = 0;
	max_warp = 0;
	last_tsc = 0;

	
	atomic_inc(&stop_count);
}


void __cpuinit check_tsc_sync_target(void)
{
	int cpus = 2;

	if (unsynchronized_tsc() || boot_cpu_has(X86_FEATURE_TSC_RELIABLE))
		return;

	
	atomic_inc(&start_count);
	while (atomic_read(&start_count) != cpus)
		cpu_relax();

	check_tsc_warp();

	
	atomic_inc(&stop_count);

	
	while (atomic_read(&stop_count) != cpus)
		cpu_relax();
}
