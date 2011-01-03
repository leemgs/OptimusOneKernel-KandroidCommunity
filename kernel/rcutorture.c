
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/spinlock.h>
#include <linux/smp.h>
#include <linux/rcupdate.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <asm/atomic.h>
#include <linux/bitops.h>
#include <linux/completion.h>
#include <linux/moduleparam.h>
#include <linux/percpu.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/freezer.h>
#include <linux/cpu.h>
#include <linux/delay.h>
#include <linux/stat.h>
#include <linux/srcu.h>
#include <linux/slab.h>
#include <asm/byteorder.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paul E. McKenney <paulmck@us.ibm.com> and "
	      "Josh Triplett <josh@freedesktop.org>");

static int nreaders = -1;	
static int nfakewriters = 4;	
static int stat_interval;	
				
static int verbose;		
static int test_no_idle_hz;	
static int shuffle_interval = 3; 
static int stutter = 5;		
static int irqreader = 1;	
static char *torture_type = "rcu"; 

module_param(nreaders, int, 0444);
MODULE_PARM_DESC(nreaders, "Number of RCU reader threads");
module_param(nfakewriters, int, 0444);
MODULE_PARM_DESC(nfakewriters, "Number of RCU fake writer threads");
module_param(stat_interval, int, 0444);
MODULE_PARM_DESC(stat_interval, "Number of seconds between stats printk()s");
module_param(verbose, bool, 0444);
MODULE_PARM_DESC(verbose, "Enable verbose debugging printk()s");
module_param(test_no_idle_hz, bool, 0444);
MODULE_PARM_DESC(test_no_idle_hz, "Test support for tickless idle CPUs");
module_param(shuffle_interval, int, 0444);
MODULE_PARM_DESC(shuffle_interval, "Number of seconds between shuffles");
module_param(stutter, int, 0444);
MODULE_PARM_DESC(stutter, "Number of seconds to run/halt test");
module_param(irqreader, int, 0444);
MODULE_PARM_DESC(irqreader, "Allow RCU readers from irq handlers");
module_param(torture_type, charp, 0444);
MODULE_PARM_DESC(torture_type, "Type of RCU to torture (rcu, rcu_bh, srcu)");

#define TORTURE_FLAG "-torture:"
#define PRINTK_STRING(s) \
	do { printk(KERN_ALERT "%s" TORTURE_FLAG s "\n", torture_type); } while (0)
#define VERBOSE_PRINTK_STRING(s) \
	do { if (verbose) printk(KERN_ALERT "%s" TORTURE_FLAG s "\n", torture_type); } while (0)
#define VERBOSE_PRINTK_ERRSTRING(s) \
	do { if (verbose) printk(KERN_ALERT "%s" TORTURE_FLAG "!!! " s "\n", torture_type); } while (0)

static char printk_buf[4096];

static int nrealreaders;
static struct task_struct *writer_task;
static struct task_struct **fakewriter_tasks;
static struct task_struct **reader_tasks;
static struct task_struct *stats_task;
static struct task_struct *shuffler_task;
static struct task_struct *stutter_task;

#define RCU_TORTURE_PIPE_LEN 10

struct rcu_torture {
	struct rcu_head rtort_rcu;
	int rtort_pipe_count;
	struct list_head rtort_free;
	int rtort_mbtest;
};

static LIST_HEAD(rcu_torture_freelist);
static struct rcu_torture *rcu_torture_current;
static long rcu_torture_current_version;
static struct rcu_torture rcu_tortures[10 * RCU_TORTURE_PIPE_LEN];
static DEFINE_SPINLOCK(rcu_torture_lock);
static DEFINE_PER_CPU(long [RCU_TORTURE_PIPE_LEN + 1], rcu_torture_count) =
	{ 0 };
static DEFINE_PER_CPU(long [RCU_TORTURE_PIPE_LEN + 1], rcu_torture_batch) =
	{ 0 };
static atomic_t rcu_torture_wcount[RCU_TORTURE_PIPE_LEN + 1];
static atomic_t n_rcu_torture_alloc;
static atomic_t n_rcu_torture_alloc_fail;
static atomic_t n_rcu_torture_free;
static atomic_t n_rcu_torture_mberror;
static atomic_t n_rcu_torture_error;
static long n_rcu_torture_timers;
static struct list_head rcu_torture_removed;
static cpumask_var_t shuffle_tmp_mask;

static int stutter_pause_test;

#if defined(MODULE) || defined(CONFIG_RCU_TORTURE_TEST_RUNNABLE)
#define RCUTORTURE_RUNNABLE_INIT 1
#else
#define RCUTORTURE_RUNNABLE_INIT 0
#endif
int rcutorture_runnable = RCUTORTURE_RUNNABLE_INIT;



#define FULLSTOP_DONTSTOP 0	
#define FULLSTOP_SHUTDOWN 1	
#define FULLSTOP_RMMOD    2	
static int fullstop = FULLSTOP_RMMOD;
DEFINE_MUTEX(fullstop_mutex);	
				


static int
rcutorture_shutdown_notify(struct notifier_block *unused1,
			   unsigned long unused2, void *unused3)
{
	mutex_lock(&fullstop_mutex);
	if (fullstop == FULLSTOP_DONTSTOP)
		fullstop = FULLSTOP_SHUTDOWN;
	else
		printk(KERN_WARNING 
		       "Concurrent 'rmmod rcutorture' and shutdown illegal!\n");
	mutex_unlock(&fullstop_mutex);
	return NOTIFY_DONE;
}


static void rcutorture_shutdown_absorb(char *title)
{
	if (ACCESS_ONCE(fullstop) == FULLSTOP_SHUTDOWN) {
		printk(KERN_NOTICE
		       "rcutorture thread %s parking due to system shutdown\n",
		       title);
		schedule_timeout_uninterruptible(MAX_SCHEDULE_TIMEOUT);
	}
}


static struct rcu_torture *
rcu_torture_alloc(void)
{
	struct list_head *p;

	spin_lock_bh(&rcu_torture_lock);
	if (list_empty(&rcu_torture_freelist)) {
		atomic_inc(&n_rcu_torture_alloc_fail);
		spin_unlock_bh(&rcu_torture_lock);
		return NULL;
	}
	atomic_inc(&n_rcu_torture_alloc);
	p = rcu_torture_freelist.next;
	list_del_init(p);
	spin_unlock_bh(&rcu_torture_lock);
	return container_of(p, struct rcu_torture, rtort_free);
}


static void
rcu_torture_free(struct rcu_torture *p)
{
	atomic_inc(&n_rcu_torture_free);
	spin_lock_bh(&rcu_torture_lock);
	list_add_tail(&p->rtort_free, &rcu_torture_freelist);
	spin_unlock_bh(&rcu_torture_lock);
}

struct rcu_random_state {
	unsigned long rrs_state;
	long rrs_count;
};

#define RCU_RANDOM_MULT 39916801  
#define RCU_RANDOM_ADD	479001701 
#define RCU_RANDOM_REFRESH 10000

#define DEFINE_RCU_RANDOM(name) struct rcu_random_state name = { 0, 0 }


static unsigned long
rcu_random(struct rcu_random_state *rrsp)
{
	if (--rrsp->rrs_count < 0) {
		rrsp->rrs_state +=
			(unsigned long)cpu_clock(raw_smp_processor_id());
		rrsp->rrs_count = RCU_RANDOM_REFRESH;
	}
	rrsp->rrs_state = rrsp->rrs_state * RCU_RANDOM_MULT + RCU_RANDOM_ADD;
	return swahw32(rrsp->rrs_state);
}

static void
rcu_stutter_wait(char *title)
{
	while (stutter_pause_test || !rcutorture_runnable) {
		if (rcutorture_runnable)
			schedule_timeout_interruptible(1);
		else
			schedule_timeout_interruptible(round_jiffies_relative(HZ));
		rcutorture_shutdown_absorb(title);
	}
}



struct rcu_torture_ops {
	void (*init)(void);
	void (*cleanup)(void);
	int (*readlock)(void);
	void (*read_delay)(struct rcu_random_state *rrsp);
	void (*readunlock)(int idx);
	int (*completed)(void);
	void (*deferred_free)(struct rcu_torture *p);
	void (*sync)(void);
	void (*cb_barrier)(void);
	int (*stats)(char *page);
	int irq_capable;
	char *name;
};

static struct rcu_torture_ops *cur_ops;



static int rcu_torture_read_lock(void) __acquires(RCU)
{
	rcu_read_lock();
	return 0;
}

static void rcu_read_delay(struct rcu_random_state *rrsp)
{
	const unsigned long shortdelay_us = 200;
	const unsigned long longdelay_ms = 50;

	

	if (!(rcu_random(rrsp) % (nrealreaders * 2000 * longdelay_ms)))
		mdelay(longdelay_ms);
	if (!(rcu_random(rrsp) % (nrealreaders * 2 * shortdelay_us)))
		udelay(shortdelay_us);
}

static void rcu_torture_read_unlock(int idx) __releases(RCU)
{
	rcu_read_unlock();
}

static int rcu_torture_completed(void)
{
	return rcu_batches_completed();
}

static void
rcu_torture_cb(struct rcu_head *p)
{
	int i;
	struct rcu_torture *rp = container_of(p, struct rcu_torture, rtort_rcu);

	if (fullstop != FULLSTOP_DONTSTOP) {
		
		
		return;
	}
	i = rp->rtort_pipe_count;
	if (i > RCU_TORTURE_PIPE_LEN)
		i = RCU_TORTURE_PIPE_LEN;
	atomic_inc(&rcu_torture_wcount[i]);
	if (++rp->rtort_pipe_count >= RCU_TORTURE_PIPE_LEN) {
		rp->rtort_mbtest = 0;
		rcu_torture_free(rp);
	} else
		cur_ops->deferred_free(rp);
}

static void rcu_torture_deferred_free(struct rcu_torture *p)
{
	call_rcu(&p->rtort_rcu, rcu_torture_cb);
}

static struct rcu_torture_ops rcu_ops = {
	.init		= NULL,
	.cleanup	= NULL,
	.readlock	= rcu_torture_read_lock,
	.read_delay	= rcu_read_delay,
	.readunlock	= rcu_torture_read_unlock,
	.completed	= rcu_torture_completed,
	.deferred_free	= rcu_torture_deferred_free,
	.sync		= synchronize_rcu,
	.cb_barrier	= rcu_barrier,
	.stats		= NULL,
	.irq_capable	= 1,
	.name		= "rcu"
};

static void rcu_sync_torture_deferred_free(struct rcu_torture *p)
{
	int i;
	struct rcu_torture *rp;
	struct rcu_torture *rp1;

	cur_ops->sync();
	list_add(&p->rtort_free, &rcu_torture_removed);
	list_for_each_entry_safe(rp, rp1, &rcu_torture_removed, rtort_free) {
		i = rp->rtort_pipe_count;
		if (i > RCU_TORTURE_PIPE_LEN)
			i = RCU_TORTURE_PIPE_LEN;
		atomic_inc(&rcu_torture_wcount[i]);
		if (++rp->rtort_pipe_count >= RCU_TORTURE_PIPE_LEN) {
			rp->rtort_mbtest = 0;
			list_del(&rp->rtort_free);
			rcu_torture_free(rp);
		}
	}
}

static void rcu_sync_torture_init(void)
{
	INIT_LIST_HEAD(&rcu_torture_removed);
}

static struct rcu_torture_ops rcu_sync_ops = {
	.init		= rcu_sync_torture_init,
	.cleanup	= NULL,
	.readlock	= rcu_torture_read_lock,
	.read_delay	= rcu_read_delay,
	.readunlock	= rcu_torture_read_unlock,
	.completed	= rcu_torture_completed,
	.deferred_free	= rcu_sync_torture_deferred_free,
	.sync		= synchronize_rcu,
	.cb_barrier	= NULL,
	.stats		= NULL,
	.irq_capable	= 1,
	.name		= "rcu_sync"
};



static int rcu_bh_torture_read_lock(void) __acquires(RCU_BH)
{
	rcu_read_lock_bh();
	return 0;
}

static void rcu_bh_torture_read_unlock(int idx) __releases(RCU_BH)
{
	rcu_read_unlock_bh();
}

static int rcu_bh_torture_completed(void)
{
	return rcu_batches_completed_bh();
}

static void rcu_bh_torture_deferred_free(struct rcu_torture *p)
{
	call_rcu_bh(&p->rtort_rcu, rcu_torture_cb);
}

struct rcu_bh_torture_synchronize {
	struct rcu_head head;
	struct completion completion;
};

static void rcu_bh_torture_wakeme_after_cb(struct rcu_head *head)
{
	struct rcu_bh_torture_synchronize *rcu;

	rcu = container_of(head, struct rcu_bh_torture_synchronize, head);
	complete(&rcu->completion);
}

static void rcu_bh_torture_synchronize(void)
{
	struct rcu_bh_torture_synchronize rcu;

	init_completion(&rcu.completion);
	call_rcu_bh(&rcu.head, rcu_bh_torture_wakeme_after_cb);
	wait_for_completion(&rcu.completion);
}

static struct rcu_torture_ops rcu_bh_ops = {
	.init		= NULL,
	.cleanup	= NULL,
	.readlock	= rcu_bh_torture_read_lock,
	.read_delay	= rcu_read_delay,  
	.readunlock	= rcu_bh_torture_read_unlock,
	.completed	= rcu_bh_torture_completed,
	.deferred_free	= rcu_bh_torture_deferred_free,
	.sync		= rcu_bh_torture_synchronize,
	.cb_barrier	= rcu_barrier_bh,
	.stats		= NULL,
	.irq_capable	= 1,
	.name		= "rcu_bh"
};

static struct rcu_torture_ops rcu_bh_sync_ops = {
	.init		= rcu_sync_torture_init,
	.cleanup	= NULL,
	.readlock	= rcu_bh_torture_read_lock,
	.read_delay	= rcu_read_delay,  
	.readunlock	= rcu_bh_torture_read_unlock,
	.completed	= rcu_bh_torture_completed,
	.deferred_free	= rcu_sync_torture_deferred_free,
	.sync		= rcu_bh_torture_synchronize,
	.cb_barrier	= NULL,
	.stats		= NULL,
	.irq_capable	= 1,
	.name		= "rcu_bh_sync"
};



static struct srcu_struct srcu_ctl;

static void srcu_torture_init(void)
{
	init_srcu_struct(&srcu_ctl);
	rcu_sync_torture_init();
}

static void srcu_torture_cleanup(void)
{
	synchronize_srcu(&srcu_ctl);
	cleanup_srcu_struct(&srcu_ctl);
}

static int srcu_torture_read_lock(void) __acquires(&srcu_ctl)
{
	return srcu_read_lock(&srcu_ctl);
}

static void srcu_read_delay(struct rcu_random_state *rrsp)
{
	long delay;
	const long uspertick = 1000000 / HZ;
	const long longdelay = 10;

	

	delay = rcu_random(rrsp) % (nrealreaders * 2 * longdelay * uspertick);
	if (!delay)
		schedule_timeout_interruptible(longdelay);
}

static void srcu_torture_read_unlock(int idx) __releases(&srcu_ctl)
{
	srcu_read_unlock(&srcu_ctl, idx);
}

static int srcu_torture_completed(void)
{
	return srcu_batches_completed(&srcu_ctl);
}

static void srcu_torture_synchronize(void)
{
	synchronize_srcu(&srcu_ctl);
}

static int srcu_torture_stats(char *page)
{
	int cnt = 0;
	int cpu;
	int idx = srcu_ctl.completed & 0x1;

	cnt += sprintf(&page[cnt], "%s%s per-CPU(idx=%d):",
		       torture_type, TORTURE_FLAG, idx);
	for_each_possible_cpu(cpu) {
		cnt += sprintf(&page[cnt], " %d(%d,%d)", cpu,
			       per_cpu_ptr(srcu_ctl.per_cpu_ref, cpu)->c[!idx],
			       per_cpu_ptr(srcu_ctl.per_cpu_ref, cpu)->c[idx]);
	}
	cnt += sprintf(&page[cnt], "\n");
	return cnt;
}

static struct rcu_torture_ops srcu_ops = {
	.init		= srcu_torture_init,
	.cleanup	= srcu_torture_cleanup,
	.readlock	= srcu_torture_read_lock,
	.read_delay	= srcu_read_delay,
	.readunlock	= srcu_torture_read_unlock,
	.completed	= srcu_torture_completed,
	.deferred_free	= rcu_sync_torture_deferred_free,
	.sync		= srcu_torture_synchronize,
	.cb_barrier	= NULL,
	.stats		= srcu_torture_stats,
	.name		= "srcu"
};



static int sched_torture_read_lock(void)
{
	preempt_disable();
	return 0;
}

static void sched_torture_read_unlock(int idx)
{
	preempt_enable();
}

static int sched_torture_completed(void)
{
	return 0;
}

static void rcu_sched_torture_deferred_free(struct rcu_torture *p)
{
	call_rcu_sched(&p->rtort_rcu, rcu_torture_cb);
}

static void sched_torture_synchronize(void)
{
	synchronize_sched();
}

static struct rcu_torture_ops sched_ops = {
	.init		= rcu_sync_torture_init,
	.cleanup	= NULL,
	.readlock	= sched_torture_read_lock,
	.read_delay	= rcu_read_delay,  
	.readunlock	= sched_torture_read_unlock,
	.completed	= sched_torture_completed,
	.deferred_free	= rcu_sched_torture_deferred_free,
	.sync		= sched_torture_synchronize,
	.cb_barrier	= rcu_barrier_sched,
	.stats		= NULL,
	.irq_capable	= 1,
	.name		= "sched"
};

static struct rcu_torture_ops sched_ops_sync = {
	.init		= rcu_sync_torture_init,
	.cleanup	= NULL,
	.readlock	= sched_torture_read_lock,
	.read_delay	= rcu_read_delay,  
	.readunlock	= sched_torture_read_unlock,
	.completed	= sched_torture_completed,
	.deferred_free	= rcu_sync_torture_deferred_free,
	.sync		= sched_torture_synchronize,
	.cb_barrier	= NULL,
	.stats		= NULL,
	.name		= "sched_sync"
};

static struct rcu_torture_ops sched_expedited_ops = {
	.init		= rcu_sync_torture_init,
	.cleanup	= NULL,
	.readlock	= sched_torture_read_lock,
	.read_delay	= rcu_read_delay,  
	.readunlock	= sched_torture_read_unlock,
	.completed	= sched_torture_completed,
	.deferred_free	= rcu_sync_torture_deferred_free,
	.sync		= synchronize_sched_expedited,
	.cb_barrier	= NULL,
	.stats		= rcu_expedited_torture_stats,
	.irq_capable	= 1,
	.name		= "sched_expedited"
};


static int
rcu_torture_writer(void *arg)
{
	int i;
	long oldbatch = rcu_batches_completed();
	struct rcu_torture *rp;
	struct rcu_torture *old_rp;
	static DEFINE_RCU_RANDOM(rand);

	VERBOSE_PRINTK_STRING("rcu_torture_writer task started");
	set_user_nice(current, 19);

	do {
		schedule_timeout_uninterruptible(1);
		rp = rcu_torture_alloc();
		if (rp == NULL)
			continue;
		rp->rtort_pipe_count = 0;
		udelay(rcu_random(&rand) & 0x3ff);
		old_rp = rcu_torture_current;
		rp->rtort_mbtest = 1;
		rcu_assign_pointer(rcu_torture_current, rp);
		smp_wmb(); 
		if (old_rp) {
			i = old_rp->rtort_pipe_count;
			if (i > RCU_TORTURE_PIPE_LEN)
				i = RCU_TORTURE_PIPE_LEN;
			atomic_inc(&rcu_torture_wcount[i]);
			old_rp->rtort_pipe_count++;
			cur_ops->deferred_free(old_rp);
		}
		rcu_torture_current_version++;
		oldbatch = cur_ops->completed();
		rcu_stutter_wait("rcu_torture_writer");
	} while (!kthread_should_stop() && fullstop == FULLSTOP_DONTSTOP);
	VERBOSE_PRINTK_STRING("rcu_torture_writer task stopping");
	rcutorture_shutdown_absorb("rcu_torture_writer");
	while (!kthread_should_stop())
		schedule_timeout_uninterruptible(1);
	return 0;
}


static int
rcu_torture_fakewriter(void *arg)
{
	DEFINE_RCU_RANDOM(rand);

	VERBOSE_PRINTK_STRING("rcu_torture_fakewriter task started");
	set_user_nice(current, 19);

	do {
		schedule_timeout_uninterruptible(1 + rcu_random(&rand)%10);
		udelay(rcu_random(&rand) & 0x3ff);
		cur_ops->sync();
		rcu_stutter_wait("rcu_torture_fakewriter");
	} while (!kthread_should_stop() && fullstop == FULLSTOP_DONTSTOP);

	VERBOSE_PRINTK_STRING("rcu_torture_fakewriter task stopping");
	rcutorture_shutdown_absorb("rcu_torture_fakewriter");
	while (!kthread_should_stop())
		schedule_timeout_uninterruptible(1);
	return 0;
}


static void rcu_torture_timer(unsigned long unused)
{
	int idx;
	int completed;
	static DEFINE_RCU_RANDOM(rand);
	static DEFINE_SPINLOCK(rand_lock);
	struct rcu_torture *p;
	int pipe_count;

	idx = cur_ops->readlock();
	completed = cur_ops->completed();
	p = rcu_dereference(rcu_torture_current);
	if (p == NULL) {
		
		cur_ops->readunlock(idx);
		return;
	}
	if (p->rtort_mbtest == 0)
		atomic_inc(&n_rcu_torture_mberror);
	spin_lock(&rand_lock);
	cur_ops->read_delay(&rand);
	n_rcu_torture_timers++;
	spin_unlock(&rand_lock);
	preempt_disable();
	pipe_count = p->rtort_pipe_count;
	if (pipe_count > RCU_TORTURE_PIPE_LEN) {
		
		pipe_count = RCU_TORTURE_PIPE_LEN;
	}
	++__get_cpu_var(rcu_torture_count)[pipe_count];
	completed = cur_ops->completed() - completed;
	if (completed > RCU_TORTURE_PIPE_LEN) {
		
		completed = RCU_TORTURE_PIPE_LEN;
	}
	++__get_cpu_var(rcu_torture_batch)[completed];
	preempt_enable();
	cur_ops->readunlock(idx);
}


static int
rcu_torture_reader(void *arg)
{
	int completed;
	int idx;
	DEFINE_RCU_RANDOM(rand);
	struct rcu_torture *p;
	int pipe_count;
	struct timer_list t;

	VERBOSE_PRINTK_STRING("rcu_torture_reader task started");
	set_user_nice(current, 19);
	if (irqreader && cur_ops->irq_capable)
		setup_timer_on_stack(&t, rcu_torture_timer, 0);

	do {
		if (irqreader && cur_ops->irq_capable) {
			if (!timer_pending(&t))
				mod_timer(&t, 1);
		}
		idx = cur_ops->readlock();
		completed = cur_ops->completed();
		p = rcu_dereference(rcu_torture_current);
		if (p == NULL) {
			
			cur_ops->readunlock(idx);
			schedule_timeout_interruptible(HZ);
			continue;
		}
		if (p->rtort_mbtest == 0)
			atomic_inc(&n_rcu_torture_mberror);
		cur_ops->read_delay(&rand);
		preempt_disable();
		pipe_count = p->rtort_pipe_count;
		if (pipe_count > RCU_TORTURE_PIPE_LEN) {
			
			pipe_count = RCU_TORTURE_PIPE_LEN;
		}
		++__get_cpu_var(rcu_torture_count)[pipe_count];
		completed = cur_ops->completed() - completed;
		if (completed > RCU_TORTURE_PIPE_LEN) {
			
			completed = RCU_TORTURE_PIPE_LEN;
		}
		++__get_cpu_var(rcu_torture_batch)[completed];
		preempt_enable();
		cur_ops->readunlock(idx);
		schedule();
		rcu_stutter_wait("rcu_torture_reader");
	} while (!kthread_should_stop() && fullstop == FULLSTOP_DONTSTOP);
	VERBOSE_PRINTK_STRING("rcu_torture_reader task stopping");
	rcutorture_shutdown_absorb("rcu_torture_reader");
	if (irqreader && cur_ops->irq_capable)
		del_timer_sync(&t);
	while (!kthread_should_stop())
		schedule_timeout_uninterruptible(1);
	return 0;
}


static int
rcu_torture_printk(char *page)
{
	int cnt = 0;
	int cpu;
	int i;
	long pipesummary[RCU_TORTURE_PIPE_LEN + 1] = { 0 };
	long batchsummary[RCU_TORTURE_PIPE_LEN + 1] = { 0 };

	for_each_possible_cpu(cpu) {
		for (i = 0; i < RCU_TORTURE_PIPE_LEN + 1; i++) {
			pipesummary[i] += per_cpu(rcu_torture_count, cpu)[i];
			batchsummary[i] += per_cpu(rcu_torture_batch, cpu)[i];
		}
	}
	for (i = RCU_TORTURE_PIPE_LEN - 1; i >= 0; i--) {
		if (pipesummary[i] != 0)
			break;
	}
	cnt += sprintf(&page[cnt], "%s%s ", torture_type, TORTURE_FLAG);
	cnt += sprintf(&page[cnt],
		       "rtc: %p ver: %ld tfle: %d rta: %d rtaf: %d rtf: %d "
		       "rtmbe: %d nt: %ld",
		       rcu_torture_current,
		       rcu_torture_current_version,
		       list_empty(&rcu_torture_freelist),
		       atomic_read(&n_rcu_torture_alloc),
		       atomic_read(&n_rcu_torture_alloc_fail),
		       atomic_read(&n_rcu_torture_free),
		       atomic_read(&n_rcu_torture_mberror),
		       n_rcu_torture_timers);
	if (atomic_read(&n_rcu_torture_mberror) != 0)
		cnt += sprintf(&page[cnt], " !!!");
	cnt += sprintf(&page[cnt], "\n%s%s ", torture_type, TORTURE_FLAG);
	if (i > 1) {
		cnt += sprintf(&page[cnt], "!!! ");
		atomic_inc(&n_rcu_torture_error);
		WARN_ON_ONCE(1);
	}
	cnt += sprintf(&page[cnt], "Reader Pipe: ");
	for (i = 0; i < RCU_TORTURE_PIPE_LEN + 1; i++)
		cnt += sprintf(&page[cnt], " %ld", pipesummary[i]);
	cnt += sprintf(&page[cnt], "\n%s%s ", torture_type, TORTURE_FLAG);
	cnt += sprintf(&page[cnt], "Reader Batch: ");
	for (i = 0; i < RCU_TORTURE_PIPE_LEN + 1; i++)
		cnt += sprintf(&page[cnt], " %ld", batchsummary[i]);
	cnt += sprintf(&page[cnt], "\n%s%s ", torture_type, TORTURE_FLAG);
	cnt += sprintf(&page[cnt], "Free-Block Circulation: ");
	for (i = 0; i < RCU_TORTURE_PIPE_LEN + 1; i++) {
		cnt += sprintf(&page[cnt], " %d",
			       atomic_read(&rcu_torture_wcount[i]));
	}
	cnt += sprintf(&page[cnt], "\n");
	if (cur_ops->stats)
		cnt += cur_ops->stats(&page[cnt]);
	return cnt;
}


static void
rcu_torture_stats_print(void)
{
	int cnt;

	cnt = rcu_torture_printk(printk_buf);
	printk(KERN_ALERT "%s", printk_buf);
}


static int
rcu_torture_stats(void *arg)
{
	VERBOSE_PRINTK_STRING("rcu_torture_stats task started");
	do {
		schedule_timeout_interruptible(stat_interval * HZ);
		rcu_torture_stats_print();
		rcutorture_shutdown_absorb("rcu_torture_stats");
	} while (!kthread_should_stop());
	VERBOSE_PRINTK_STRING("rcu_torture_stats task stopping");
	return 0;
}

static int rcu_idle_cpu;	


static void rcu_torture_shuffle_tasks(void)
{
	int i;

	cpumask_setall(shuffle_tmp_mask);
	get_online_cpus();

	
	if (num_online_cpus() == 1) {
		put_online_cpus();
		return;
	}

	if (rcu_idle_cpu != -1)
		cpumask_clear_cpu(rcu_idle_cpu, shuffle_tmp_mask);

	set_cpus_allowed_ptr(current, shuffle_tmp_mask);

	if (reader_tasks) {
		for (i = 0; i < nrealreaders; i++)
			if (reader_tasks[i])
				set_cpus_allowed_ptr(reader_tasks[i],
						     shuffle_tmp_mask);
	}

	if (fakewriter_tasks) {
		for (i = 0; i < nfakewriters; i++)
			if (fakewriter_tasks[i])
				set_cpus_allowed_ptr(fakewriter_tasks[i],
						     shuffle_tmp_mask);
	}

	if (writer_task)
		set_cpus_allowed_ptr(writer_task, shuffle_tmp_mask);

	if (stats_task)
		set_cpus_allowed_ptr(stats_task, shuffle_tmp_mask);

	if (rcu_idle_cpu == -1)
		rcu_idle_cpu = num_online_cpus() - 1;
	else
		rcu_idle_cpu--;

	put_online_cpus();
}


static int
rcu_torture_shuffle(void *arg)
{
	VERBOSE_PRINTK_STRING("rcu_torture_shuffle task started");
	do {
		schedule_timeout_interruptible(shuffle_interval * HZ);
		rcu_torture_shuffle_tasks();
		rcutorture_shutdown_absorb("rcu_torture_shuffle");
	} while (!kthread_should_stop());
	VERBOSE_PRINTK_STRING("rcu_torture_shuffle task stopping");
	return 0;
}


static int
rcu_torture_stutter(void *arg)
{
	VERBOSE_PRINTK_STRING("rcu_torture_stutter task started");
	do {
		schedule_timeout_interruptible(stutter * HZ);
		stutter_pause_test = 1;
		if (!kthread_should_stop())
			schedule_timeout_interruptible(stutter * HZ);
		stutter_pause_test = 0;
		rcutorture_shutdown_absorb("rcu_torture_stutter");
	} while (!kthread_should_stop());
	VERBOSE_PRINTK_STRING("rcu_torture_stutter task stopping");
	return 0;
}

static inline void
rcu_torture_print_module_parms(char *tag)
{
	printk(KERN_ALERT "%s" TORTURE_FLAG
		"--- %s: nreaders=%d nfakewriters=%d "
		"stat_interval=%d verbose=%d test_no_idle_hz=%d "
		"shuffle_interval=%d stutter=%d irqreader=%d\n",
		torture_type, tag, nrealreaders, nfakewriters,
		stat_interval, verbose, test_no_idle_hz, shuffle_interval,
		stutter, irqreader);
}

static struct notifier_block rcutorture_nb = {
	.notifier_call = rcutorture_shutdown_notify,
};

static void
rcu_torture_cleanup(void)
{
	int i;

	mutex_lock(&fullstop_mutex);
	if (fullstop == FULLSTOP_SHUTDOWN) {
		printk(KERN_WARNING 
		       "Concurrent 'rmmod rcutorture' and shutdown illegal!\n");
		mutex_unlock(&fullstop_mutex);
		schedule_timeout_uninterruptible(10);
		if (cur_ops->cb_barrier != NULL)
			cur_ops->cb_barrier();
		return;
	}
	fullstop = FULLSTOP_RMMOD;
	mutex_unlock(&fullstop_mutex);
	unregister_reboot_notifier(&rcutorture_nb);
	if (stutter_task) {
		VERBOSE_PRINTK_STRING("Stopping rcu_torture_stutter task");
		kthread_stop(stutter_task);
	}
	stutter_task = NULL;
	if (shuffler_task) {
		VERBOSE_PRINTK_STRING("Stopping rcu_torture_shuffle task");
		kthread_stop(shuffler_task);
		free_cpumask_var(shuffle_tmp_mask);
	}
	shuffler_task = NULL;

	if (writer_task) {
		VERBOSE_PRINTK_STRING("Stopping rcu_torture_writer task");
		kthread_stop(writer_task);
	}
	writer_task = NULL;

	if (reader_tasks) {
		for (i = 0; i < nrealreaders; i++) {
			if (reader_tasks[i]) {
				VERBOSE_PRINTK_STRING(
					"Stopping rcu_torture_reader task");
				kthread_stop(reader_tasks[i]);
			}
			reader_tasks[i] = NULL;
		}
		kfree(reader_tasks);
		reader_tasks = NULL;
	}
	rcu_torture_current = NULL;

	if (fakewriter_tasks) {
		for (i = 0; i < nfakewriters; i++) {
			if (fakewriter_tasks[i]) {
				VERBOSE_PRINTK_STRING(
					"Stopping rcu_torture_fakewriter task");
				kthread_stop(fakewriter_tasks[i]);
			}
			fakewriter_tasks[i] = NULL;
		}
		kfree(fakewriter_tasks);
		fakewriter_tasks = NULL;
	}

	if (stats_task) {
		VERBOSE_PRINTK_STRING("Stopping rcu_torture_stats task");
		kthread_stop(stats_task);
	}
	stats_task = NULL;

	

	if (cur_ops->cb_barrier != NULL)
		cur_ops->cb_barrier();

	rcu_torture_stats_print();  

	if (cur_ops->cleanup)
		cur_ops->cleanup();
	if (atomic_read(&n_rcu_torture_error))
		rcu_torture_print_module_parms("End of test: FAILURE");
	else
		rcu_torture_print_module_parms("End of test: SUCCESS");
}

static int __init
rcu_torture_init(void)
{
	int i;
	int cpu;
	int firsterr = 0;
	static struct rcu_torture_ops *torture_ops[] =
		{ &rcu_ops, &rcu_sync_ops, &rcu_bh_ops, &rcu_bh_sync_ops,
		  &sched_expedited_ops,
		  &srcu_ops, &sched_ops, &sched_ops_sync, };

	mutex_lock(&fullstop_mutex);

	
	for (i = 0; i < ARRAY_SIZE(torture_ops); i++) {
		cur_ops = torture_ops[i];
		if (strcmp(torture_type, cur_ops->name) == 0)
			break;
	}
	if (i == ARRAY_SIZE(torture_ops)) {
		printk(KERN_ALERT "rcutorture: invalid torture type: \"%s\"\n",
		       torture_type);
		mutex_unlock(&fullstop_mutex);
		return -EINVAL;
	}
	if (cur_ops->init)
		cur_ops->init(); 

	if (nreaders >= 0)
		nrealreaders = nreaders;
	else
		nrealreaders = 2 * num_online_cpus();
	rcu_torture_print_module_parms("Start of test");
	fullstop = FULLSTOP_DONTSTOP;

	

	INIT_LIST_HEAD(&rcu_torture_freelist);
	for (i = 0; i < ARRAY_SIZE(rcu_tortures); i++) {
		rcu_tortures[i].rtort_mbtest = 0;
		list_add_tail(&rcu_tortures[i].rtort_free,
			      &rcu_torture_freelist);
	}

	

	rcu_torture_current = NULL;
	rcu_torture_current_version = 0;
	atomic_set(&n_rcu_torture_alloc, 0);
	atomic_set(&n_rcu_torture_alloc_fail, 0);
	atomic_set(&n_rcu_torture_free, 0);
	atomic_set(&n_rcu_torture_mberror, 0);
	atomic_set(&n_rcu_torture_error, 0);
	for (i = 0; i < RCU_TORTURE_PIPE_LEN + 1; i++)
		atomic_set(&rcu_torture_wcount[i], 0);
	for_each_possible_cpu(cpu) {
		for (i = 0; i < RCU_TORTURE_PIPE_LEN + 1; i++) {
			per_cpu(rcu_torture_count, cpu)[i] = 0;
			per_cpu(rcu_torture_batch, cpu)[i] = 0;
		}
	}

	

	VERBOSE_PRINTK_STRING("Creating rcu_torture_writer task");
	writer_task = kthread_run(rcu_torture_writer, NULL,
				  "rcu_torture_writer");
	if (IS_ERR(writer_task)) {
		firsterr = PTR_ERR(writer_task);
		VERBOSE_PRINTK_ERRSTRING("Failed to create writer");
		writer_task = NULL;
		goto unwind;
	}
	fakewriter_tasks = kzalloc(nfakewriters * sizeof(fakewriter_tasks[0]),
				   GFP_KERNEL);
	if (fakewriter_tasks == NULL) {
		VERBOSE_PRINTK_ERRSTRING("out of memory");
		firsterr = -ENOMEM;
		goto unwind;
	}
	for (i = 0; i < nfakewriters; i++) {
		VERBOSE_PRINTK_STRING("Creating rcu_torture_fakewriter task");
		fakewriter_tasks[i] = kthread_run(rcu_torture_fakewriter, NULL,
						  "rcu_torture_fakewriter");
		if (IS_ERR(fakewriter_tasks[i])) {
			firsterr = PTR_ERR(fakewriter_tasks[i]);
			VERBOSE_PRINTK_ERRSTRING("Failed to create fakewriter");
			fakewriter_tasks[i] = NULL;
			goto unwind;
		}
	}
	reader_tasks = kzalloc(nrealreaders * sizeof(reader_tasks[0]),
			       GFP_KERNEL);
	if (reader_tasks == NULL) {
		VERBOSE_PRINTK_ERRSTRING("out of memory");
		firsterr = -ENOMEM;
		goto unwind;
	}
	for (i = 0; i < nrealreaders; i++) {
		VERBOSE_PRINTK_STRING("Creating rcu_torture_reader task");
		reader_tasks[i] = kthread_run(rcu_torture_reader, NULL,
					      "rcu_torture_reader");
		if (IS_ERR(reader_tasks[i])) {
			firsterr = PTR_ERR(reader_tasks[i]);
			VERBOSE_PRINTK_ERRSTRING("Failed to create reader");
			reader_tasks[i] = NULL;
			goto unwind;
		}
	}
	if (stat_interval > 0) {
		VERBOSE_PRINTK_STRING("Creating rcu_torture_stats task");
		stats_task = kthread_run(rcu_torture_stats, NULL,
					"rcu_torture_stats");
		if (IS_ERR(stats_task)) {
			firsterr = PTR_ERR(stats_task);
			VERBOSE_PRINTK_ERRSTRING("Failed to create stats");
			stats_task = NULL;
			goto unwind;
		}
	}
	if (test_no_idle_hz) {
		rcu_idle_cpu = num_online_cpus() - 1;

		if (!alloc_cpumask_var(&shuffle_tmp_mask, GFP_KERNEL)) {
			firsterr = -ENOMEM;
			VERBOSE_PRINTK_ERRSTRING("Failed to alloc mask");
			goto unwind;
		}

		
		shuffler_task = kthread_run(rcu_torture_shuffle, NULL,
					  "rcu_torture_shuffle");
		if (IS_ERR(shuffler_task)) {
			free_cpumask_var(shuffle_tmp_mask);
			firsterr = PTR_ERR(shuffler_task);
			VERBOSE_PRINTK_ERRSTRING("Failed to create shuffler");
			shuffler_task = NULL;
			goto unwind;
		}
	}
	if (stutter < 0)
		stutter = 0;
	if (stutter) {
		
		stutter_task = kthread_run(rcu_torture_stutter, NULL,
					  "rcu_torture_stutter");
		if (IS_ERR(stutter_task)) {
			firsterr = PTR_ERR(stutter_task);
			VERBOSE_PRINTK_ERRSTRING("Failed to create stutter");
			stutter_task = NULL;
			goto unwind;
		}
	}
	register_reboot_notifier(&rcutorture_nb);
	mutex_unlock(&fullstop_mutex);
	return 0;

unwind:
	mutex_unlock(&fullstop_mutex);
	rcu_torture_cleanup();
	return firsterr;
}

module_init(rcu_torture_init);
module_exit(rcu_torture_cleanup);
