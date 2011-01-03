

#ifndef __LINUX_RCUTREE_H
#define __LINUX_RCUTREE_H

struct notifier_block;

extern void rcu_sched_qs(int cpu);
extern void rcu_bh_qs(int cpu);
extern int rcu_cpu_notify(struct notifier_block *self,
			  unsigned long action, void *hcpu);
extern int rcu_needs_cpu(int cpu);
extern int rcu_expedited_torture_stats(char *page);

#ifdef CONFIG_TREE_PREEMPT_RCU

extern void __rcu_read_lock(void);
extern void __rcu_read_unlock(void);
extern void exit_rcu(void);

#else 

static inline void __rcu_read_lock(void)
{
	preempt_disable();
}

static inline void __rcu_read_unlock(void)
{
	preempt_enable();
}

#define __synchronize_sched() synchronize_rcu()

static inline void exit_rcu(void)
{
}

#endif 

static inline void __rcu_read_lock_bh(void)
{
	local_bh_disable();
}
static inline void __rcu_read_unlock_bh(void)
{
	local_bh_enable();
}

extern void call_rcu_sched(struct rcu_head *head,
			   void (*func)(struct rcu_head *rcu));
extern void synchronize_rcu_expedited(void);

static inline void synchronize_rcu_bh_expedited(void)
{
	synchronize_sched_expedited();
}

extern void __rcu_init(void);
extern void rcu_check_callbacks(int cpu, int user);

extern long rcu_batches_completed(void);
extern long rcu_batches_completed_bh(void);
extern long rcu_batches_completed_sched(void);

#ifdef CONFIG_NO_HZ
void rcu_enter_nohz(void);
void rcu_exit_nohz(void);
#else 
static inline void rcu_enter_nohz(void)
{
}
static inline void rcu_exit_nohz(void)
{
}
#endif 


static inline int rcu_blocking_is_gp(void)
{
	return num_online_cpus() == 1;
}

#endif 
