
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <linux/interrupt.h>
#include <linux/plist.h>
#include <linux/fs.h>
#include <linux/debug_locks.h>

#include "rtmutex_common.h"

# define TRACE_WARN_ON(x)			WARN_ON(x)
# define TRACE_BUG_ON(x)			BUG_ON(x)

# define TRACE_OFF()						\
do {								\
	if (rt_trace_on) {					\
		rt_trace_on = 0;				\
		console_verbose();				\
		if (spin_is_locked(&current->pi_lock))		\
			spin_unlock(&current->pi_lock);		\
	}							\
} while (0)

# define TRACE_OFF_NOLOCK()					\
do {								\
	if (rt_trace_on) {					\
		rt_trace_on = 0;				\
		console_verbose();				\
	}							\
} while (0)

# define TRACE_BUG_LOCKED()			\
do {						\
	TRACE_OFF();				\
	BUG();					\
} while (0)

# define TRACE_WARN_ON_LOCKED(c)		\
do {						\
	if (unlikely(c)) {			\
		TRACE_OFF();			\
		WARN_ON(1);			\
	}					\
} while (0)

# define TRACE_BUG_ON_LOCKED(c)			\
do {						\
	if (unlikely(c))			\
		TRACE_BUG_LOCKED();		\
} while (0)

#ifdef CONFIG_SMP
# define SMP_TRACE_BUG_ON_LOCKED(c)	TRACE_BUG_ON_LOCKED(c)
#else
# define SMP_TRACE_BUG_ON_LOCKED(c)	do { } while (0)
#endif


static int rt_trace_on = 1;

static void printk_task(struct task_struct *p)
{
	if (p)
		printk("%16s:%5d [%p, %3d]", p->comm, task_pid_nr(p), p, p->prio);
	else
		printk("<none>");
}

static void printk_lock(struct rt_mutex *lock, int print_owner)
{
	if (lock->name)
		printk(" [%p] {%s}\n",
			lock, lock->name);
	else
		printk(" [%p] {%s:%d}\n",
			lock, lock->file, lock->line);

	if (print_owner && rt_mutex_owner(lock)) {
		printk(".. ->owner: %p\n", lock->owner);
		printk(".. held by:  ");
		printk_task(rt_mutex_owner(lock));
		printk("\n");
	}
}

void rt_mutex_debug_task_free(struct task_struct *task)
{
	WARN_ON(!plist_head_empty(&task->pi_waiters));
	WARN_ON(task->pi_blocked_on);
}


void debug_rt_mutex_deadlock(int detect, struct rt_mutex_waiter *act_waiter,
			     struct rt_mutex *lock)
{
	struct task_struct *task;

	if (!rt_trace_on || detect || !act_waiter)
		return;

	task = rt_mutex_owner(act_waiter->lock);
	if (task && task != current) {
		act_waiter->deadlock_task_pid = get_pid(task_pid(task));
		act_waiter->deadlock_lock = lock;
	}
}

void debug_rt_mutex_print_deadlock(struct rt_mutex_waiter *waiter)
{
	struct task_struct *task;

	if (!waiter->deadlock_lock || !rt_trace_on)
		return;

	rcu_read_lock();
	task = pid_task(waiter->deadlock_task_pid, PIDTYPE_PID);
	if (!task) {
		rcu_read_unlock();
		return;
	}

	TRACE_OFF_NOLOCK();

	printk("\n============================================\n");
	printk(  "[ BUG: circular locking deadlock detected! ]\n");
	printk(  "--------------------------------------------\n");
	printk("%s/%d is deadlocking current task %s/%d\n\n",
	       task->comm, task_pid_nr(task),
	       current->comm, task_pid_nr(current));

	printk("\n1) %s/%d is trying to acquire this lock:\n",
	       current->comm, task_pid_nr(current));
	printk_lock(waiter->lock, 1);

	printk("\n2) %s/%d is blocked on this lock:\n",
		task->comm, task_pid_nr(task));
	printk_lock(waiter->deadlock_lock, 1);

	debug_show_held_locks(current);
	debug_show_held_locks(task);

	printk("\n%s/%d's [blocked] stackdump:\n\n",
		task->comm, task_pid_nr(task));
	show_stack(task, NULL);
	printk("\n%s/%d's [current] stackdump:\n\n",
		current->comm, task_pid_nr(current));
	dump_stack();
	debug_show_all_locks();
	rcu_read_unlock();

	printk("[ turning off deadlock detection."
	       "Please report this trace. ]\n\n");
	local_irq_disable();
}

void debug_rt_mutex_lock(struct rt_mutex *lock)
{
}

void debug_rt_mutex_unlock(struct rt_mutex *lock)
{
	TRACE_WARN_ON_LOCKED(rt_mutex_owner(lock) != current);
}

void
debug_rt_mutex_proxy_lock(struct rt_mutex *lock, struct task_struct *powner)
{
}

void debug_rt_mutex_proxy_unlock(struct rt_mutex *lock)
{
	TRACE_WARN_ON_LOCKED(!rt_mutex_owner(lock));
}

void debug_rt_mutex_init_waiter(struct rt_mutex_waiter *waiter)
{
	memset(waiter, 0x11, sizeof(*waiter));
	plist_node_init(&waiter->list_entry, MAX_PRIO);
	plist_node_init(&waiter->pi_list_entry, MAX_PRIO);
	waiter->deadlock_task_pid = NULL;
}

void debug_rt_mutex_free_waiter(struct rt_mutex_waiter *waiter)
{
	put_pid(waiter->deadlock_task_pid);
	TRACE_WARN_ON(!plist_node_empty(&waiter->list_entry));
	TRACE_WARN_ON(!plist_node_empty(&waiter->pi_list_entry));
	TRACE_WARN_ON(waiter->task);
	memset(waiter, 0x22, sizeof(*waiter));
}

void debug_rt_mutex_init(struct rt_mutex *lock, const char *name)
{
	
	debug_check_no_locks_freed((void *)lock, sizeof(*lock));
	lock->name = name;
}

void
rt_mutex_deadlock_account_lock(struct rt_mutex *lock, struct task_struct *task)
{
}

void rt_mutex_deadlock_account_unlock(struct task_struct *task)
{
}

