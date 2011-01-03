
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/smp.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <asm/atomic.h>
#include <linux/bitops.h>
#include <linux/percpu.h>
#include <linux/notifier.h>
#include <linux/cpu.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/kernel_stat.h>

#ifdef CONFIG_DEBUG_LOCK_ALLOC
static struct lock_class_key rcu_lock_key;
struct lockdep_map rcu_lock_map =
	STATIC_LOCKDEP_MAP_INIT("rcu_read_lock", &rcu_lock_key);
EXPORT_SYMBOL_GPL(rcu_lock_map);
#endif

int rcu_scheduler_active __read_mostly;


void wakeme_after_rcu(struct rcu_head  *head)
{
	struct rcu_synchronize *rcu;

	rcu = container_of(head, struct rcu_synchronize, head);
	complete(&rcu->completion);
}

#ifdef CONFIG_TREE_PREEMPT_RCU


void synchronize_rcu(void)
{
	struct rcu_synchronize rcu;

	if (!rcu_scheduler_active)
		return;

	init_completion(&rcu.completion);
	
	call_rcu(&rcu.head, wakeme_after_rcu);
	
	wait_for_completion(&rcu.completion);
}
EXPORT_SYMBOL_GPL(synchronize_rcu);

#endif 


void synchronize_sched(void)
{
	struct rcu_synchronize rcu;

	if (rcu_blocking_is_gp())
		return;

	init_completion(&rcu.completion);
	
	call_rcu_sched(&rcu.head, wakeme_after_rcu);
	
	wait_for_completion(&rcu.completion);
}
EXPORT_SYMBOL_GPL(synchronize_sched);


void synchronize_rcu_bh(void)
{
	struct rcu_synchronize rcu;

	if (rcu_blocking_is_gp())
		return;

	init_completion(&rcu.completion);
	
	call_rcu_bh(&rcu.head, wakeme_after_rcu);
	
	wait_for_completion(&rcu.completion);
}
EXPORT_SYMBOL_GPL(synchronize_rcu_bh);

static int __cpuinit rcu_barrier_cpu_hotplug(struct notifier_block *self,
		unsigned long action, void *hcpu)
{
	return rcu_cpu_notify(self, action, hcpu);
}

void __init rcu_init(void)
{
	int i;

	__rcu_init();
	cpu_notifier(rcu_barrier_cpu_hotplug, 0);

	
	for_each_online_cpu(i)
		rcu_barrier_cpu_hotplug(NULL, CPU_UP_PREPARE, (void *)(long)i);
}

void rcu_scheduler_starting(void)
{
	WARN_ON(num_online_cpus() != 1);
	WARN_ON(nr_context_switches() > 0);
	rcu_scheduler_active = 1;
}
