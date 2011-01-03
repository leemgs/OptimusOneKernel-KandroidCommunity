
#include <linux/mm.h>
#include <linux/cpu.h>
#include <linux/nmi.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/freezer.h>
#include <linux/kthread.h>
#include <linux/lockdep.h>
#include <linux/notifier.h>
#include <linux/module.h>
#include <linux/sysctl.h>

#include <asm/irq_regs.h>

static DEFINE_SPINLOCK(print_lock);

static DEFINE_PER_CPU(unsigned long, touch_timestamp);
static DEFINE_PER_CPU(unsigned long, print_timestamp);
static DEFINE_PER_CPU(struct task_struct *, watchdog_task);

static int __read_mostly did_panic;
int __read_mostly softlockup_thresh = 60;


unsigned int __read_mostly softlockup_panic =
				CONFIG_BOOTPARAM_SOFTLOCKUP_PANIC_VALUE;

static int __init softlockup_panic_setup(char *str)
{
	softlockup_panic = simple_strtoul(str, NULL, 0);

	return 1;
}
__setup("softlockup_panic=", softlockup_panic_setup);

static int
softlock_panic(struct notifier_block *this, unsigned long event, void *ptr)
{
	did_panic = 1;

	return NOTIFY_DONE;
}

static struct notifier_block panic_block = {
	.notifier_call = softlock_panic,
};


static unsigned long get_timestamp(int this_cpu)
{
	return cpu_clock(this_cpu) >> 30LL;  
}

static void __touch_softlockup_watchdog(void)
{
	int this_cpu = raw_smp_processor_id();

	__raw_get_cpu_var(touch_timestamp) = get_timestamp(this_cpu);
}

void touch_softlockup_watchdog(void)
{
	__raw_get_cpu_var(touch_timestamp) = 0;
}
EXPORT_SYMBOL(touch_softlockup_watchdog);

void touch_all_softlockup_watchdogs(void)
{
	int cpu;

	
	for_each_online_cpu(cpu)
		per_cpu(touch_timestamp, cpu) = 0;
}
EXPORT_SYMBOL(touch_all_softlockup_watchdogs);

int proc_dosoftlockup_thresh(struct ctl_table *table, int write,
			     void __user *buffer,
			     size_t *lenp, loff_t *ppos)
{
	touch_all_softlockup_watchdogs();
	return proc_dointvec_minmax(table, write, buffer, lenp, ppos);
}


void softlockup_tick(void)
{
	int this_cpu = smp_processor_id();
	unsigned long touch_timestamp = per_cpu(touch_timestamp, this_cpu);
	unsigned long print_timestamp;
	struct pt_regs *regs = get_irq_regs();
	unsigned long now;

	
	if (!per_cpu(watchdog_task, this_cpu) || softlockup_thresh <= 0) {
		
		if (touch_timestamp)
			per_cpu(touch_timestamp, this_cpu) = 0;
		return;
	}

	if (touch_timestamp == 0) {
		__touch_softlockup_watchdog();
		return;
	}

	print_timestamp = per_cpu(print_timestamp, this_cpu);

	
	if (print_timestamp == touch_timestamp || did_panic)
		return;

	
	if (unlikely(system_state != SYSTEM_RUNNING)) {
		__touch_softlockup_watchdog();
		return;
	}

	now = get_timestamp(this_cpu);

	
	if (now > touch_timestamp + softlockup_thresh/2)
		wake_up_process(per_cpu(watchdog_task, this_cpu));

	
	if (now <= (touch_timestamp + softlockup_thresh))
		return;

	per_cpu(print_timestamp, this_cpu) = touch_timestamp;

	spin_lock(&print_lock);
	printk(KERN_ERR "BUG: soft lockup - CPU#%d stuck for %lus! [%s:%d]\n",
			this_cpu, now - touch_timestamp,
			current->comm, task_pid_nr(current));
	print_modules();
	print_irqtrace_events(current);
	if (regs)
		show_regs(regs);
	else
		dump_stack();
	spin_unlock(&print_lock);

	if (softlockup_panic)
		panic("softlockup: hung tasks");
}


static int watchdog(void *__bind_cpu)
{
	struct sched_param param = { .sched_priority = MAX_RT_PRIO-1 };

	sched_setscheduler(current, SCHED_FIFO, &param);

	
	__touch_softlockup_watchdog();

	set_current_state(TASK_INTERRUPTIBLE);
	
	while (!kthread_should_stop()) {
		__touch_softlockup_watchdog();
		schedule();

		if (kthread_should_stop())
			break;

		set_current_state(TASK_INTERRUPTIBLE);
	}
	__set_current_state(TASK_RUNNING);

	return 0;
}


static int __cpuinit
cpu_callback(struct notifier_block *nfb, unsigned long action, void *hcpu)
{
	int hotcpu = (unsigned long)hcpu;
	struct task_struct *p;

	switch (action) {
	case CPU_UP_PREPARE:
	case CPU_UP_PREPARE_FROZEN:
		BUG_ON(per_cpu(watchdog_task, hotcpu));
		p = kthread_create(watchdog, hcpu, "watchdog/%d", hotcpu);
		if (IS_ERR(p)) {
			printk(KERN_ERR "watchdog for %i failed\n", hotcpu);
			return NOTIFY_BAD;
		}
		per_cpu(touch_timestamp, hotcpu) = 0;
		per_cpu(watchdog_task, hotcpu) = p;
		kthread_bind(p, hotcpu);
		break;
	case CPU_ONLINE:
	case CPU_ONLINE_FROZEN:
		wake_up_process(per_cpu(watchdog_task, hotcpu));
		break;
#ifdef CONFIG_HOTPLUG_CPU
	case CPU_UP_CANCELED:
	case CPU_UP_CANCELED_FROZEN:
		if (!per_cpu(watchdog_task, hotcpu))
			break;
		
		kthread_bind(per_cpu(watchdog_task, hotcpu),
			     cpumask_any(cpu_online_mask));
	case CPU_DEAD:
	case CPU_DEAD_FROZEN:
		p = per_cpu(watchdog_task, hotcpu);
		per_cpu(watchdog_task, hotcpu) = NULL;
		kthread_stop(p);
		break;
#endif 
	}
	return NOTIFY_OK;
}

static struct notifier_block __cpuinitdata cpu_nfb = {
	.notifier_call = cpu_callback
};

static int __initdata nosoftlockup;

static int __init nosoftlockup_setup(char *str)
{
	nosoftlockup = 1;
	return 1;
}
__setup("nosoftlockup", nosoftlockup_setup);

static int __init spawn_softlockup_task(void)
{
	void *cpu = (void *)(long)smp_processor_id();
	int err;

	if (nosoftlockup)
		return 0;

	err = cpu_callback(&cpu_nfb, CPU_UP_PREPARE, cpu);
	if (err == NOTIFY_BAD) {
		BUG();
		return 1;
	}
	cpu_callback(&cpu_nfb, CPU_ONLINE, cpu);
	register_cpu_notifier(&cpu_nfb);

	atomic_notifier_chain_register(&panic_notifier_list, &panic_block);

	return 0;
}
early_initcall(spawn_softlockup_task);
