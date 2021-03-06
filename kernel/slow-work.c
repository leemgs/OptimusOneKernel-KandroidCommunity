

#include <linux/module.h>
#include <linux/slow-work.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/wait.h>
#include <linux/debugfs.h>
#include "slow-work.h"

static void slow_work_cull_timeout(unsigned long);
static void slow_work_oom_timeout(unsigned long);

#ifdef CONFIG_SYSCTL
static int slow_work_min_threads_sysctl(struct ctl_table *, int,
					void __user *, size_t *, loff_t *);

static int slow_work_max_threads_sysctl(struct ctl_table *, int ,
					void __user *, size_t *, loff_t *);
#endif


static unsigned slow_work_min_threads = 2;
static unsigned slow_work_max_threads = 4;
static unsigned vslow_work_proportion = 50; 

#ifdef CONFIG_SYSCTL
static const int slow_work_min_min_threads = 2;
static int slow_work_max_max_threads = SLOW_WORK_THREAD_LIMIT;
static const int slow_work_min_vslow = 1;
static const int slow_work_max_vslow = 99;

ctl_table slow_work_sysctls[] = {
	{
		.ctl_name	= CTL_UNNUMBERED,
		.procname	= "min-threads",
		.data		= &slow_work_min_threads,
		.maxlen		= sizeof(unsigned),
		.mode		= 0644,
		.proc_handler	= slow_work_min_threads_sysctl,
		.extra1		= (void *) &slow_work_min_min_threads,
		.extra2		= &slow_work_max_threads,
	},
	{
		.ctl_name	= CTL_UNNUMBERED,
		.procname	= "max-threads",
		.data		= &slow_work_max_threads,
		.maxlen		= sizeof(unsigned),
		.mode		= 0644,
		.proc_handler	= slow_work_max_threads_sysctl,
		.extra1		= &slow_work_min_threads,
		.extra2		= (void *) &slow_work_max_max_threads,
	},
	{
		.ctl_name	= CTL_UNNUMBERED,
		.procname	= "vslow-percentage",
		.data		= &vslow_work_proportion,
		.maxlen		= sizeof(unsigned),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec_minmax,
		.extra1		= (void *) &slow_work_min_vslow,
		.extra2		= (void *) &slow_work_max_vslow,
	},
	{ .ctl_name = 0 }
};
#endif


static atomic_t slow_work_thread_count;
static atomic_t vslow_work_executing_count;

static bool slow_work_may_not_start_new_thread;
static bool slow_work_cull; 
static DEFINE_TIMER(slow_work_cull_timer, slow_work_cull_timeout, 0, 0);
static DEFINE_TIMER(slow_work_oom_timer, slow_work_oom_timeout, 0, 0);
static struct slow_work slow_work_new_thread; 


static DECLARE_BITMAP(slow_work_ids, SLOW_WORK_THREAD_LIMIT);


#ifdef CONFIG_MODULES
static struct module *slow_work_thread_processing[SLOW_WORK_THREAD_LIMIT];
static struct module *slow_work_unreg_module;
static struct slow_work *slow_work_unreg_work_item;
static DECLARE_WAIT_QUEUE_HEAD(slow_work_unreg_wq);
static DEFINE_MUTEX(slow_work_unreg_sync_lock);

static void slow_work_set_thread_processing(int id, struct slow_work *work)
{
	if (work)
		slow_work_thread_processing[id] = work->owner;
}
static void slow_work_done_thread_processing(int id, struct slow_work *work)
{
	struct module *module = slow_work_thread_processing[id];

	slow_work_thread_processing[id] = NULL;
	smp_mb();
	if (slow_work_unreg_work_item == work ||
	    slow_work_unreg_module == module)
		wake_up_all(&slow_work_unreg_wq);
}
static void slow_work_clear_thread_processing(int id)
{
	slow_work_thread_processing[id] = NULL;
}
#else
static void slow_work_set_thread_processing(int id, struct slow_work *work) {}
static void slow_work_done_thread_processing(int id, struct slow_work *work) {}
static void slow_work_clear_thread_processing(int id) {}
#endif


#ifdef CONFIG_SLOW_WORK_DEBUG
struct slow_work *slow_work_execs[SLOW_WORK_THREAD_LIMIT];
pid_t slow_work_pids[SLOW_WORK_THREAD_LIMIT];
DEFINE_RWLOCK(slow_work_execs_lock);
#endif


LIST_HEAD(slow_work_queue);
LIST_HEAD(vslow_work_queue);
DEFINE_SPINLOCK(slow_work_queue_lock);


static DECLARE_WAIT_QUEUE_HEAD(slow_work_queue_waits_for_occupation);
static DECLARE_WAIT_QUEUE_HEAD(vslow_work_queue_waits_for_occupation);


static bool slow_work_threads_should_exit;
static DECLARE_WAIT_QUEUE_HEAD(slow_work_thread_wq);
static DECLARE_COMPLETION(slow_work_last_thread_exited);


static int slow_work_user_count;
static DEFINE_MUTEX(slow_work_user_lock);

static inline int slow_work_get_ref(struct slow_work *work)
{
	if (work->ops->get_ref)
		return work->ops->get_ref(work);

	return 0;
}

static inline void slow_work_put_ref(struct slow_work *work)
{
	if (work->ops->put_ref)
		work->ops->put_ref(work);
}


static unsigned slow_work_calc_vsmax(void)
{
	unsigned vsmax;

	vsmax = atomic_read(&slow_work_thread_count) * vslow_work_proportion;
	vsmax /= 100;
	vsmax = max(vsmax, 1U);
	return min(vsmax, slow_work_max_threads - 1);
}


static noinline bool slow_work_execute(int id)
{
	struct slow_work *work = NULL;
	unsigned vsmax;
	bool very_slow;

	vsmax = slow_work_calc_vsmax();

	
	if (!waitqueue_active(&slow_work_thread_wq) &&
	    (!list_empty(&slow_work_queue) || !list_empty(&vslow_work_queue)) &&
	    atomic_read(&slow_work_thread_count) < slow_work_max_threads &&
	    !slow_work_may_not_start_new_thread)
		slow_work_enqueue(&slow_work_new_thread);

	
	spin_lock_irq(&slow_work_queue_lock);
	if (!list_empty(&vslow_work_queue) &&
	    atomic_read(&vslow_work_executing_count) < vsmax) {
		work = list_entry(vslow_work_queue.next,
				  struct slow_work, link);
		if (test_and_set_bit_lock(SLOW_WORK_EXECUTING, &work->flags))
			BUG();
		list_del_init(&work->link);
		atomic_inc(&vslow_work_executing_count);
		very_slow = true;
	} else if (!list_empty(&slow_work_queue)) {
		work = list_entry(slow_work_queue.next,
				  struct slow_work, link);
		if (test_and_set_bit_lock(SLOW_WORK_EXECUTING, &work->flags))
			BUG();
		list_del_init(&work->link);
		very_slow = false;
	} else {
		very_slow = false; 
	}

	slow_work_set_thread_processing(id, work);
	if (work) {
		slow_work_mark_time(work);
		slow_work_begin_exec(id, work);
	}

	spin_unlock_irq(&slow_work_queue_lock);

	if (!work)
		return false;

	if (!test_and_clear_bit(SLOW_WORK_PENDING, &work->flags))
		BUG();

	
	if (!test_bit(SLOW_WORK_CANCELLING, &work->flags))
		work->ops->execute(work);

	if (very_slow)
		atomic_dec(&vslow_work_executing_count);
	clear_bit_unlock(SLOW_WORK_EXECUTING, &work->flags);

	
	wake_up_bit(&work->flags, SLOW_WORK_EXECUTING);

	slow_work_end_exec(id, work);

	
	if (test_bit(SLOW_WORK_PENDING, &work->flags)) {
		spin_lock_irq(&slow_work_queue_lock);

		if (!test_bit(SLOW_WORK_EXECUTING, &work->flags) &&
		    test_and_clear_bit(SLOW_WORK_ENQ_DEFERRED, &work->flags))
			goto auto_requeue;

		spin_unlock_irq(&slow_work_queue_lock);
	}

	
	slow_work_put_ref(work);
	slow_work_done_thread_processing(id, work);

	return true;

auto_requeue:
	
	slow_work_mark_time(work);
	if (test_bit(SLOW_WORK_VERY_SLOW, &work->flags))
		list_add_tail(&work->link, &vslow_work_queue);
	else
		list_add_tail(&work->link, &slow_work_queue);
	spin_unlock_irq(&slow_work_queue_lock);
	slow_work_clear_thread_processing(id);
	return true;
}


bool slow_work_sleep_till_thread_needed(struct slow_work *work,
					signed long *_timeout)
{
	wait_queue_head_t *wfo_wq;
	struct list_head *queue;

	DEFINE_WAIT(wait);

	if (test_bit(SLOW_WORK_VERY_SLOW, &work->flags)) {
		wfo_wq = &vslow_work_queue_waits_for_occupation;
		queue = &vslow_work_queue;
	} else {
		wfo_wq = &slow_work_queue_waits_for_occupation;
		queue = &slow_work_queue;
	}

	if (!list_empty(queue))
		return true;

	add_wait_queue_exclusive(wfo_wq, &wait);
	if (list_empty(queue))
		*_timeout = schedule_timeout(*_timeout);
	finish_wait(wfo_wq, &wait);

	return !list_empty(queue);
}
EXPORT_SYMBOL(slow_work_sleep_till_thread_needed);


int slow_work_enqueue(struct slow_work *work)
{
	wait_queue_head_t *wfo_wq;
	struct list_head *queue;
	unsigned long flags;
	int ret;

	if (test_bit(SLOW_WORK_CANCELLING, &work->flags))
		return -ECANCELED;

	BUG_ON(slow_work_user_count <= 0);
	BUG_ON(!work);
	BUG_ON(!work->ops);

	
	if (!test_and_set_bit_lock(SLOW_WORK_PENDING, &work->flags)) {
		if (test_bit(SLOW_WORK_VERY_SLOW, &work->flags)) {
			wfo_wq = &vslow_work_queue_waits_for_occupation;
			queue = &vslow_work_queue;
		} else {
			wfo_wq = &slow_work_queue_waits_for_occupation;
			queue = &slow_work_queue;
		}

		spin_lock_irqsave(&slow_work_queue_lock, flags);

		if (unlikely(test_bit(SLOW_WORK_CANCELLING, &work->flags)))
			goto cancelled;

		
		if (test_bit(SLOW_WORK_EXECUTING, &work->flags)) {
			set_bit(SLOW_WORK_ENQ_DEFERRED, &work->flags);
		} else {
			ret = slow_work_get_ref(work);
			if (ret < 0)
				goto failed;
			slow_work_mark_time(work);
			list_add_tail(&work->link, queue);
			wake_up(&slow_work_thread_wq);

			
			if (work->link.prev == queue)
				wake_up(wfo_wq);
		}

		spin_unlock_irqrestore(&slow_work_queue_lock, flags);
	}
	return 0;

cancelled:
	ret = -ECANCELED;
failed:
	spin_unlock_irqrestore(&slow_work_queue_lock, flags);
	return ret;
}
EXPORT_SYMBOL(slow_work_enqueue);

static int slow_work_wait(void *word)
{
	schedule();
	return 0;
}


void slow_work_cancel(struct slow_work *work)
{
	bool wait = true, put = false;

	set_bit(SLOW_WORK_CANCELLING, &work->flags);
	smp_mb();

	
	if (test_bit(SLOW_WORK_DELAYED, &work->flags)) {
		struct delayed_slow_work *dwork =
			container_of(work, struct delayed_slow_work, work);
		del_timer_sync(&dwork->timer);
	}

	spin_lock_irq(&slow_work_queue_lock);

	if (test_bit(SLOW_WORK_DELAYED, &work->flags)) {
		
		struct delayed_slow_work *dwork =
			container_of(work, struct delayed_slow_work, work);

		BUG_ON(timer_pending(&dwork->timer));
		BUG_ON(!list_empty(&work->link));

		clear_bit(SLOW_WORK_DELAYED, &work->flags);
		put = true;
		clear_bit(SLOW_WORK_PENDING, &work->flags);

	} else if (test_bit(SLOW_WORK_PENDING, &work->flags) &&
		   !list_empty(&work->link)) {
		
		list_del_init(&work->link);
		wait = false;
		put = true;
		clear_bit(SLOW_WORK_PENDING, &work->flags);

	} else if (test_and_clear_bit(SLOW_WORK_ENQ_DEFERRED, &work->flags)) {
		
		clear_bit(SLOW_WORK_PENDING, &work->flags);
	}

	spin_unlock_irq(&slow_work_queue_lock);

	
	if (wait)
		wait_on_bit(&work->flags, SLOW_WORK_EXECUTING, slow_work_wait,
			    TASK_UNINTERRUPTIBLE);

	clear_bit(SLOW_WORK_CANCELLING, &work->flags);
	if (put)
		slow_work_put_ref(work);
}
EXPORT_SYMBOL(slow_work_cancel);


static void delayed_slow_work_timer(unsigned long data)
{
	wait_queue_head_t *wfo_wq;
	struct list_head *queue;
	struct slow_work *work = (struct slow_work *) data;
	unsigned long flags;
	bool queued = false, put = false, first = false;

	if (test_bit(SLOW_WORK_VERY_SLOW, &work->flags)) {
		wfo_wq = &vslow_work_queue_waits_for_occupation;
		queue = &vslow_work_queue;
	} else {
		wfo_wq = &slow_work_queue_waits_for_occupation;
		queue = &slow_work_queue;
	}

	spin_lock_irqsave(&slow_work_queue_lock, flags);
	if (likely(!test_bit(SLOW_WORK_CANCELLING, &work->flags))) {
		clear_bit(SLOW_WORK_DELAYED, &work->flags);

		if (test_bit(SLOW_WORK_EXECUTING, &work->flags)) {
			
			set_bit(SLOW_WORK_ENQ_DEFERRED, &work->flags);
			put = true;
		} else {
			slow_work_mark_time(work);
			list_add_tail(&work->link, queue);
			queued = true;
			if (work->link.prev == queue)
				first = true;
		}
	}

	spin_unlock_irqrestore(&slow_work_queue_lock, flags);
	if (put)
		slow_work_put_ref(work);
	if (first)
		wake_up(wfo_wq);
	if (queued)
		wake_up(&slow_work_thread_wq);
}


int delayed_slow_work_enqueue(struct delayed_slow_work *dwork,
			      unsigned long delay)
{
	struct slow_work *work = &dwork->work;
	unsigned long flags;
	int ret;

	if (delay == 0)
		return slow_work_enqueue(&dwork->work);

	BUG_ON(slow_work_user_count <= 0);
	BUG_ON(!work);
	BUG_ON(!work->ops);

	if (test_bit(SLOW_WORK_CANCELLING, &work->flags))
		return -ECANCELED;

	if (!test_and_set_bit_lock(SLOW_WORK_PENDING, &work->flags)) {
		spin_lock_irqsave(&slow_work_queue_lock, flags);

		if (test_bit(SLOW_WORK_CANCELLING, &work->flags))
			goto cancelled;

		
		ret = work->ops->get_ref(work);
		if (ret < 0)
			goto cant_get_ref;

		if (test_and_set_bit(SLOW_WORK_DELAYED, &work->flags))
			BUG();
		dwork->timer.expires = jiffies + delay;
		dwork->timer.data = (unsigned long) work;
		dwork->timer.function = delayed_slow_work_timer;
		add_timer(&dwork->timer);

		spin_unlock_irqrestore(&slow_work_queue_lock, flags);
	}

	return 0;

cancelled:
	ret = -ECANCELED;
cant_get_ref:
	spin_unlock_irqrestore(&slow_work_queue_lock, flags);
	return ret;
}
EXPORT_SYMBOL(delayed_slow_work_enqueue);


static void slow_work_schedule_cull(void)
{
	mod_timer(&slow_work_cull_timer,
		  round_jiffies(jiffies + SLOW_WORK_CULL_TIMEOUT));
}


static bool slow_work_cull_thread(void)
{
	unsigned long flags;
	bool do_cull = false;

	spin_lock_irqsave(&slow_work_queue_lock, flags);

	if (slow_work_cull) {
		slow_work_cull = false;

		if (list_empty(&slow_work_queue) &&
		    list_empty(&vslow_work_queue) &&
		    atomic_read(&slow_work_thread_count) >
		    slow_work_min_threads) {
			slow_work_schedule_cull();
			do_cull = true;
		}
	}

	spin_unlock_irqrestore(&slow_work_queue_lock, flags);
	return do_cull;
}


static inline bool slow_work_available(int vsmax)
{
	return !list_empty(&slow_work_queue) ||
		(!list_empty(&vslow_work_queue) &&
		 atomic_read(&vslow_work_executing_count) < vsmax);
}


static int slow_work_thread(void *_data)
{
	int vsmax, id;

	DEFINE_WAIT(wait);

	set_freezable();
	set_user_nice(current, -5);

	
	spin_lock_irq(&slow_work_queue_lock);
	id = find_first_zero_bit(slow_work_ids, SLOW_WORK_THREAD_LIMIT);
	BUG_ON(id < 0 || id >= SLOW_WORK_THREAD_LIMIT);
	__set_bit(id, slow_work_ids);
	slow_work_set_thread_pid(id, current->pid);
	spin_unlock_irq(&slow_work_queue_lock);

	sprintf(current->comm, "kslowd%03u", id);

	for (;;) {
		vsmax = vslow_work_proportion;
		vsmax *= atomic_read(&slow_work_thread_count);
		vsmax /= 100;

		prepare_to_wait_exclusive(&slow_work_thread_wq, &wait,
					  TASK_INTERRUPTIBLE);
		if (!freezing(current) &&
		    !slow_work_threads_should_exit &&
		    !slow_work_available(vsmax) &&
		    !slow_work_cull)
			schedule();
		finish_wait(&slow_work_thread_wq, &wait);

		try_to_freeze();

		vsmax = vslow_work_proportion;
		vsmax *= atomic_read(&slow_work_thread_count);
		vsmax /= 100;

		if (slow_work_available(vsmax) && slow_work_execute(id)) {
			cond_resched();
			if (list_empty(&slow_work_queue) &&
			    list_empty(&vslow_work_queue) &&
			    atomic_read(&slow_work_thread_count) >
			    slow_work_min_threads)
				slow_work_schedule_cull();
			continue;
		}

		if (slow_work_threads_should_exit)
			break;

		if (slow_work_cull && slow_work_cull_thread())
			break;
	}

	spin_lock_irq(&slow_work_queue_lock);
	slow_work_set_thread_pid(id, 0);
	__clear_bit(id, slow_work_ids);
	spin_unlock_irq(&slow_work_queue_lock);

	if (atomic_dec_and_test(&slow_work_thread_count))
		complete_and_exit(&slow_work_last_thread_exited, 0);
	return 0;
}


static void slow_work_cull_timeout(unsigned long data)
{
	slow_work_cull = true;
	wake_up(&slow_work_thread_wq);
}


static void slow_work_new_thread_execute(struct slow_work *work)
{
	struct task_struct *p;

	if (slow_work_threads_should_exit)
		return;

	if (atomic_read(&slow_work_thread_count) >= slow_work_max_threads)
		return;

	if (!mutex_trylock(&slow_work_user_lock))
		return;

	slow_work_may_not_start_new_thread = true;
	atomic_inc(&slow_work_thread_count);
	p = kthread_run(slow_work_thread, NULL, "kslowd");
	if (IS_ERR(p)) {
		printk(KERN_DEBUG "Slow work thread pool: OOM\n");
		if (atomic_dec_and_test(&slow_work_thread_count))
			BUG(); 
		mod_timer(&slow_work_oom_timer,
			  round_jiffies(jiffies + SLOW_WORK_OOM_TIMEOUT));
	} else {
		
		mod_timer(&slow_work_oom_timer, jiffies + 1);
	}

	mutex_unlock(&slow_work_user_lock);
}

static const struct slow_work_ops slow_work_new_thread_ops = {
	.owner		= THIS_MODULE,
	.execute	= slow_work_new_thread_execute,
#ifdef CONFIG_SLOW_WORK_DEBUG
	.desc		= slow_work_new_thread_desc,
#endif
};


static void slow_work_oom_timeout(unsigned long data)
{
	slow_work_may_not_start_new_thread = false;
}

#ifdef CONFIG_SYSCTL

static int slow_work_min_threads_sysctl(struct ctl_table *table, int write,
					void __user *buffer,
					size_t *lenp, loff_t *ppos)
{
	int ret = proc_dointvec_minmax(table, write, buffer, lenp, ppos);
	int n;

	if (ret == 0) {
		mutex_lock(&slow_work_user_lock);
		if (slow_work_user_count > 0) {
			
			n = atomic_read(&slow_work_thread_count) -
				slow_work_min_threads;

			if (n < 0 && !slow_work_may_not_start_new_thread)
				slow_work_enqueue(&slow_work_new_thread);
			else if (n > 0)
				slow_work_schedule_cull();
		}
		mutex_unlock(&slow_work_user_lock);
	}

	return ret;
}


static int slow_work_max_threads_sysctl(struct ctl_table *table, int write,
					void __user *buffer,
					size_t *lenp, loff_t *ppos)
{
	int ret = proc_dointvec_minmax(table, write, buffer, lenp, ppos);
	int n;

	if (ret == 0) {
		mutex_lock(&slow_work_user_lock);
		if (slow_work_user_count > 0) {
			
			n = slow_work_max_threads -
				atomic_read(&slow_work_thread_count);

			if (n < 0)
				slow_work_schedule_cull();
		}
		mutex_unlock(&slow_work_user_lock);
	}

	return ret;
}
#endif 


int slow_work_register_user(struct module *module)
{
	struct task_struct *p;
	int loop;

	mutex_lock(&slow_work_user_lock);

	if (slow_work_user_count == 0) {
		printk(KERN_NOTICE "Slow work thread pool: Starting up\n");
		init_completion(&slow_work_last_thread_exited);

		slow_work_threads_should_exit = false;
		slow_work_init(&slow_work_new_thread,
			       &slow_work_new_thread_ops);
		slow_work_may_not_start_new_thread = false;
		slow_work_cull = false;

		
		for (loop = 0; loop < slow_work_min_threads; loop++) {
			atomic_inc(&slow_work_thread_count);
			p = kthread_run(slow_work_thread, NULL, "kslowd");
			if (IS_ERR(p))
				goto error;
		}
		printk(KERN_NOTICE "Slow work thread pool: Ready\n");
	}

	slow_work_user_count++;
	mutex_unlock(&slow_work_user_lock);
	return 0;

error:
	if (atomic_dec_and_test(&slow_work_thread_count))
		complete(&slow_work_last_thread_exited);
	if (loop > 0) {
		printk(KERN_ERR "Slow work thread pool:"
		       " Aborting startup on ENOMEM\n");
		slow_work_threads_should_exit = true;
		wake_up_all(&slow_work_thread_wq);
		wait_for_completion(&slow_work_last_thread_exited);
		printk(KERN_ERR "Slow work thread pool: Aborted\n");
	}
	mutex_unlock(&slow_work_user_lock);
	return PTR_ERR(p);
}
EXPORT_SYMBOL(slow_work_register_user);


static void slow_work_wait_for_items(struct module *module)
{
#ifdef CONFIG_MODULES
	DECLARE_WAITQUEUE(myself, current);
	struct slow_work *work;
	int loop;

	mutex_lock(&slow_work_unreg_sync_lock);
	add_wait_queue(&slow_work_unreg_wq, &myself);

	for (;;) {
		spin_lock_irq(&slow_work_queue_lock);

		
		list_for_each_entry_reverse(work, &vslow_work_queue, link) {
			if (work->owner == module) {
				set_current_state(TASK_UNINTERRUPTIBLE);
				slow_work_unreg_work_item = work;
				goto do_wait;
			}
		}
		list_for_each_entry_reverse(work, &slow_work_queue, link) {
			if (work->owner == module) {
				set_current_state(TASK_UNINTERRUPTIBLE);
				slow_work_unreg_work_item = work;
				goto do_wait;
			}
		}

		
		slow_work_unreg_module = module;
		smp_mb();
		for (loop = 0; loop < SLOW_WORK_THREAD_LIMIT; loop++) {
			if (slow_work_thread_processing[loop] == module)
				goto do_wait;
		}
		spin_unlock_irq(&slow_work_queue_lock);
		break; 

	do_wait:
		spin_unlock_irq(&slow_work_queue_lock);
		schedule();
		slow_work_unreg_work_item = NULL;
		slow_work_unreg_module = NULL;
	}

	remove_wait_queue(&slow_work_unreg_wq, &myself);
	mutex_unlock(&slow_work_unreg_sync_lock);
#endif 
}


void slow_work_unregister_user(struct module *module)
{
	
	if (module)
		slow_work_wait_for_items(module);

	
	mutex_lock(&slow_work_user_lock);

	BUG_ON(slow_work_user_count <= 0);

	slow_work_user_count--;
	if (slow_work_user_count == 0) {
		printk(KERN_NOTICE "Slow work thread pool: Shutting down\n");
		slow_work_threads_should_exit = true;
		del_timer_sync(&slow_work_cull_timer);
		del_timer_sync(&slow_work_oom_timer);
		wake_up_all(&slow_work_thread_wq);
		wait_for_completion(&slow_work_last_thread_exited);
		printk(KERN_NOTICE "Slow work thread pool:"
		       " Shut down complete\n");
	}

	mutex_unlock(&slow_work_user_lock);
}
EXPORT_SYMBOL(slow_work_unregister_user);


static int __init init_slow_work(void)
{
	unsigned nr_cpus = num_possible_cpus();

	if (slow_work_max_threads < nr_cpus)
		slow_work_max_threads = nr_cpus;
#ifdef CONFIG_SYSCTL
	if (slow_work_max_max_threads < nr_cpus * 2)
		slow_work_max_max_threads = nr_cpus * 2;
#endif
#ifdef CONFIG_SLOW_WORK_DEBUG
	{
		struct dentry *dbdir;

		dbdir = debugfs_create_dir("slow_work", NULL);
		if (dbdir && !IS_ERR(dbdir))
			debugfs_create_file("runqueue", S_IFREG | 0400, dbdir,
					    NULL, &slow_work_runqueue_fops);
	}
#endif
	return 0;
}

subsys_initcall(init_slow_work);
