


#define DISABLE_BRANCH_PROFILING

#include <linux/time.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/seqlock.h>
#include <linux/jiffies.h>
#include <linux/sysctl.h>
#include <linux/clocksource.h>
#include <linux/getcpu.h>
#include <linux/cpu.h>
#include <linux/smp.h>
#include <linux/notifier.h>

#include <asm/vsyscall.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <asm/unistd.h>
#include <asm/fixmap.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/segment.h>
#include <asm/desc.h>
#include <asm/topology.h>
#include <asm/vgtod.h>

#define __vsyscall(nr) \
		__attribute__ ((unused, __section__(".vsyscall_" #nr))) notrace
#define __syscall_clobber "r11","cx","memory"


int __vgetcpu_mode __section_vgetcpu_mode;

struct vsyscall_gtod_data __vsyscall_gtod_data __section_vsyscall_gtod_data =
{
	.lock = SEQLOCK_UNLOCKED,
	.sysctl_enabled = 1,
};

void update_vsyscall_tz(void)
{
	unsigned long flags;

	write_seqlock_irqsave(&vsyscall_gtod_data.lock, flags);
	
	vsyscall_gtod_data.sys_tz = sys_tz;
	write_sequnlock_irqrestore(&vsyscall_gtod_data.lock, flags);
}

void update_vsyscall(struct timespec *wall_time, struct clocksource *clock)
{
	unsigned long flags;

	write_seqlock_irqsave(&vsyscall_gtod_data.lock, flags);
	
	vsyscall_gtod_data.clock.vread = clock->vread;
	vsyscall_gtod_data.clock.cycle_last = clock->cycle_last;
	vsyscall_gtod_data.clock.mask = clock->mask;
	vsyscall_gtod_data.clock.mult = clock->mult;
	vsyscall_gtod_data.clock.shift = clock->shift;
	vsyscall_gtod_data.wall_time_sec = wall_time->tv_sec;
	vsyscall_gtod_data.wall_time_nsec = wall_time->tv_nsec;
	vsyscall_gtod_data.wall_to_monotonic = wall_to_monotonic;
	vsyscall_gtod_data.wall_time_coarse = __current_kernel_time();
	write_sequnlock_irqrestore(&vsyscall_gtod_data.lock, flags);
}


static __always_inline void do_get_tz(struct timezone * tz)
{
	*tz = __vsyscall_gtod_data.sys_tz;
}

static __always_inline int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	int ret;
	asm volatile("syscall"
		: "=a" (ret)
		: "0" (__NR_gettimeofday),"D" (tv),"S" (tz)
		: __syscall_clobber );
	return ret;
}

static __always_inline long time_syscall(long *t)
{
	long secs;
	asm volatile("syscall"
		: "=a" (secs)
		: "0" (__NR_time),"D" (t) : __syscall_clobber);
	return secs;
}

static __always_inline void do_vgettimeofday(struct timeval * tv)
{
	cycle_t now, base, mask, cycle_delta;
	unsigned seq;
	unsigned long mult, shift, nsec;
	cycle_t (*vread)(void);
	do {
		seq = read_seqbegin(&__vsyscall_gtod_data.lock);

		vread = __vsyscall_gtod_data.clock.vread;
		if (unlikely(!__vsyscall_gtod_data.sysctl_enabled || !vread)) {
			gettimeofday(tv,NULL);
			return;
		}

		now = vread();
		base = __vsyscall_gtod_data.clock.cycle_last;
		mask = __vsyscall_gtod_data.clock.mask;
		mult = __vsyscall_gtod_data.clock.mult;
		shift = __vsyscall_gtod_data.clock.shift;

		tv->tv_sec = __vsyscall_gtod_data.wall_time_sec;
		nsec = __vsyscall_gtod_data.wall_time_nsec;
	} while (read_seqretry(&__vsyscall_gtod_data.lock, seq));

	
	cycle_delta = (now - base) & mask;
	
	nsec += (cycle_delta * mult) >> shift;

	while (nsec >= NSEC_PER_SEC) {
		tv->tv_sec += 1;
		nsec -= NSEC_PER_SEC;
	}
	tv->tv_usec = nsec / NSEC_PER_USEC;
}

int __vsyscall(0) vgettimeofday(struct timeval * tv, struct timezone * tz)
{
	if (tv)
		do_vgettimeofday(tv);
	if (tz)
		do_get_tz(tz);
	return 0;
}


time_t __vsyscall(1) vtime(time_t *t)
{
	struct timeval tv;
	time_t result;
	if (unlikely(!__vsyscall_gtod_data.sysctl_enabled))
		return time_syscall(t);

	vgettimeofday(&tv, NULL);
	result = tv.tv_sec;
	if (t)
		*t = result;
	return result;
}


long __vsyscall(2)
vgetcpu(unsigned *cpu, unsigned *node, struct getcpu_cache *tcache)
{
	unsigned int p;
	unsigned long j = 0;

	
	if (tcache && tcache->blob[0] == (j = __jiffies)) {
		p = tcache->blob[1];
	} else if (__vgetcpu_mode == VGETCPU_RDTSCP) {
		
		native_read_tscp(&p);
	} else {
		
		asm("lsl %1,%0" : "=r" (p) : "r" (__PER_CPU_SEG));
	}
	if (tcache) {
		tcache->blob[0] = j;
		tcache->blob[1] = p;
	}
	if (cpu)
		*cpu = p & 0xfff;
	if (node)
		*node = p >> 12;
	return 0;
}

static long __vsyscall(3) venosys_1(void)
{
	return -ENOSYS;
}

#ifdef CONFIG_SYSCTL
static ctl_table kernel_table2[] = {
	{ .procname = "vsyscall64",
	  .data = &vsyscall_gtod_data.sysctl_enabled, .maxlen = sizeof(int),
	  .mode = 0644,
	  .proc_handler = proc_dointvec },
	{}
};

static ctl_table kernel_root_table2[] = {
	{ .ctl_name = CTL_KERN, .procname = "kernel", .mode = 0555,
	  .child = kernel_table2 },
	{}
};
#endif


static void __cpuinit vsyscall_set_cpu(int cpu)
{
	unsigned long d;
	unsigned long node = 0;
#ifdef CONFIG_NUMA
	node = cpu_to_node(cpu);
#endif
	if (cpu_has(&cpu_data(cpu), X86_FEATURE_RDTSCP))
		write_rdtscp_aux((node << 12) | cpu);

	
	d = 0x0f40000000000ULL;
	d |= cpu;
	d |= (node & 0xf) << 12;
	d |= (node >> 4) << 48;
	write_gdt_entry(get_cpu_gdt_table(cpu), GDT_ENTRY_PER_CPU, &d, DESCTYPE_S);
}

static void __cpuinit cpu_vsyscall_init(void *arg)
{
	
	vsyscall_set_cpu(raw_smp_processor_id());
}

static int __cpuinit
cpu_vsyscall_notifier(struct notifier_block *n, unsigned long action, void *arg)
{
	long cpu = (long)arg;
	if (action == CPU_ONLINE || action == CPU_ONLINE_FROZEN)
		smp_call_function_single(cpu, cpu_vsyscall_init, NULL, 1);
	return NOTIFY_DONE;
}

void __init map_vsyscall(void)
{
	extern char __vsyscall_0;
	unsigned long physaddr_page0 = __pa_symbol(&__vsyscall_0);

	
	__set_fixmap(VSYSCALL_FIRST_PAGE, physaddr_page0, PAGE_KERNEL_VSYSCALL);
}

static int __init vsyscall_init(void)
{
	BUG_ON(((unsigned long) &vgettimeofday !=
			VSYSCALL_ADDR(__NR_vgettimeofday)));
	BUG_ON((unsigned long) &vtime != VSYSCALL_ADDR(__NR_vtime));
	BUG_ON((VSYSCALL_ADDR(0) != __fix_to_virt(VSYSCALL_FIRST_PAGE)));
	BUG_ON((unsigned long) &vgetcpu != VSYSCALL_ADDR(__NR_vgetcpu));
#ifdef CONFIG_SYSCTL
	register_sysctl_table(kernel_root_table2);
#endif
	on_each_cpu(cpu_vsyscall_init, NULL, 1);
	hotcpu_notifier(cpu_vsyscall_notifier, 0);
	return 0;
}

__initcall(vsyscall_init);
