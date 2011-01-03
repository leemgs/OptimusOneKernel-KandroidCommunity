
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/smp.h>
#include <linux/rcupdate.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/nmi.h>
#include <asm/atomic.h>
#include <linux/bitops.h>
#include <linux/module.h>
#include <linux/completion.h>
#include <linux/moduleparam.h>
#include <linux/percpu.h>
#include <linux/notifier.h>
#include <linux/cpu.h>
#include <linux/mutex.h>
#include <linux/time.h>

#include "rcutree.h"



#define RCU_STATE_INITIALIZER(name) { \
	.level = { &name.node[0] }, \
	.levelcnt = { \
		NUM_RCU_LVL_0,   \
		NUM_RCU_LVL_1, \
		NUM_RCU_LVL_2, \
		NUM_RCU_LVL_3,  \
	}, \
	.signaled = RCU_GP_IDLE, \
	.gpnum = -300, \
	.completed = -300, \
	.onofflock = __SPIN_LOCK_UNLOCKED(&name.onofflock), \
	.orphan_cbs_list = NULL, \
	.orphan_cbs_tail = &name.orphan_cbs_list, \
	.orphan_qlen = 0, \
	.fqslock = __SPIN_LOCK_UNLOCKED(&name.fqslock), \
	.n_force_qs = 0, \
	.n_force_qs_ngp = 0, \
}

struct rcu_state rcu_sched_state = RCU_STATE_INITIALIZER(rcu_sched_state);
DEFINE_PER_CPU(struct rcu_data, rcu_sched_data);

struct rcu_state rcu_bh_state = RCU_STATE_INITIALIZER(rcu_bh_state);
DEFINE_PER_CPU(struct rcu_data, rcu_bh_data);



static int rcu_gp_in_progress(struct rcu_state *rsp)
{
	return ACCESS_ONCE(rsp->completed) != ACCESS_ONCE(rsp->gpnum);
}


void rcu_sched_qs(int cpu)
{
	struct rcu_data *rdp;

	rdp = &per_cpu(rcu_sched_data, cpu);
	rdp->passed_quiesc_completed = rdp->completed;
	barrier();
	rdp->passed_quiesc = 1;
	rcu_preempt_note_context_switch(cpu);
}

void rcu_bh_qs(int cpu)
{
	struct rcu_data *rdp;

	rdp = &per_cpu(rcu_bh_data, cpu);
	rdp->passed_quiesc_completed = rdp->completed;
	barrier();
	rdp->passed_quiesc = 1;
}

#ifdef CONFIG_NO_HZ
DEFINE_PER_CPU(struct rcu_dynticks, rcu_dynticks) = {
	.dynticks_nesting = 1,
	.dynticks = 1,
};
#endif 

static int blimit = 10;		
static int qhimark = 10000;	
static int qlowmark = 100;	

module_param(blimit, int, 0);
module_param(qhimark, int, 0);
module_param(qlowmark, int, 0);

static void force_quiescent_state(struct rcu_state *rsp, int relaxed);
static int rcu_pending(int cpu);


long rcu_batches_completed_sched(void)
{
	return rcu_sched_state.completed;
}
EXPORT_SYMBOL_GPL(rcu_batches_completed_sched);


long rcu_batches_completed_bh(void)
{
	return rcu_bh_state.completed;
}
EXPORT_SYMBOL_GPL(rcu_batches_completed_bh);


static int
cpu_has_callbacks_ready_to_invoke(struct rcu_data *rdp)
{
	return &rdp->nxtlist != rdp->nxttail[RCU_DONE_TAIL];
}


static int
cpu_needs_another_gp(struct rcu_state *rsp, struct rcu_data *rdp)
{
	return *rdp->nxttail[RCU_DONE_TAIL] && !rcu_gp_in_progress(rsp);
}


static struct rcu_node *rcu_get_root(struct rcu_state *rsp)
{
	return &rsp->node[0];
}


static void dyntick_record_completed(struct rcu_state *rsp, long comp)
{
	rsp->dynticks_completed = comp;
}

#ifdef CONFIG_SMP


static long dyntick_recall_completed(struct rcu_state *rsp)
{
	return rsp->dynticks_completed;
}


static int rcu_implicit_offline_qs(struct rcu_data *rdp)
{
	
	if (cpu_is_offline(rdp->cpu)) {
		rdp->offline_fqs++;
		return 1;
	}

	
	if (rdp->preemptable)
		return 0;

	
	if (rdp->cpu != smp_processor_id())
		smp_send_reschedule(rdp->cpu);
	else
		set_need_resched();
	rdp->resched_ipi++;
	return 0;
}

#endif 

#ifdef CONFIG_NO_HZ


void rcu_enter_nohz(void)
{
	unsigned long flags;
	struct rcu_dynticks *rdtp;

	smp_mb(); 
	local_irq_save(flags);
	rdtp = &__get_cpu_var(rcu_dynticks);
	rdtp->dynticks++;
	rdtp->dynticks_nesting--;
	WARN_ON_ONCE(rdtp->dynticks & 0x1);
	local_irq_restore(flags);
}


void rcu_exit_nohz(void)
{
	unsigned long flags;
	struct rcu_dynticks *rdtp;

	local_irq_save(flags);
	rdtp = &__get_cpu_var(rcu_dynticks);
	rdtp->dynticks++;
	rdtp->dynticks_nesting++;
	WARN_ON_ONCE(!(rdtp->dynticks & 0x1));
	local_irq_restore(flags);
	smp_mb(); 
}


void rcu_nmi_enter(void)
{
	struct rcu_dynticks *rdtp = &__get_cpu_var(rcu_dynticks);

	if (rdtp->dynticks & 0x1)
		return;
	rdtp->dynticks_nmi++;
	WARN_ON_ONCE(!(rdtp->dynticks_nmi & 0x1));
	smp_mb(); 
}


void rcu_nmi_exit(void)
{
	struct rcu_dynticks *rdtp = &__get_cpu_var(rcu_dynticks);

	if (rdtp->dynticks & 0x1)
		return;
	smp_mb(); 
	rdtp->dynticks_nmi++;
	WARN_ON_ONCE(rdtp->dynticks_nmi & 0x1);
}


void rcu_irq_enter(void)
{
	struct rcu_dynticks *rdtp = &__get_cpu_var(rcu_dynticks);

	if (rdtp->dynticks_nesting++)
		return;
	rdtp->dynticks++;
	WARN_ON_ONCE(!(rdtp->dynticks & 0x1));
	smp_mb(); 
}


void rcu_irq_exit(void)
{
	struct rcu_dynticks *rdtp = &__get_cpu_var(rcu_dynticks);

	if (--rdtp->dynticks_nesting)
		return;
	smp_mb(); 
	rdtp->dynticks++;
	WARN_ON_ONCE(rdtp->dynticks & 0x1);

	
	if (__get_cpu_var(rcu_sched_data).nxtlist ||
	    __get_cpu_var(rcu_bh_data).nxtlist)
		set_need_resched();
}

#ifdef CONFIG_SMP


static int dyntick_save_progress_counter(struct rcu_data *rdp)
{
	int ret;
	int snap;
	int snap_nmi;

	snap = rdp->dynticks->dynticks;
	snap_nmi = rdp->dynticks->dynticks_nmi;
	smp_mb();	
	rdp->dynticks_snap = snap;
	rdp->dynticks_nmi_snap = snap_nmi;
	ret = ((snap & 0x1) == 0) && ((snap_nmi & 0x1) == 0);
	if (ret)
		rdp->dynticks_fqs++;
	return ret;
}


static int rcu_implicit_dynticks_qs(struct rcu_data *rdp)
{
	long curr;
	long curr_nmi;
	long snap;
	long snap_nmi;

	curr = rdp->dynticks->dynticks;
	snap = rdp->dynticks_snap;
	curr_nmi = rdp->dynticks->dynticks_nmi;
	snap_nmi = rdp->dynticks_nmi_snap;
	smp_mb(); 

	
	if ((curr != snap || (curr & 0x1) == 0) &&
	    (curr_nmi != snap_nmi || (curr_nmi & 0x1) == 0)) {
		rdp->dynticks_fqs++;
		return 1;
	}

	
	return rcu_implicit_offline_qs(rdp);
}

#endif 

#else 

#ifdef CONFIG_SMP

static int dyntick_save_progress_counter(struct rcu_data *rdp)
{
	return 0;
}

static int rcu_implicit_dynticks_qs(struct rcu_data *rdp)
{
	return rcu_implicit_offline_qs(rdp);
}

#endif 

#endif 

#ifdef CONFIG_RCU_CPU_STALL_DETECTOR

static void record_gp_stall_check_time(struct rcu_state *rsp)
{
	rsp->gp_start = jiffies;
	rsp->jiffies_stall = jiffies + RCU_SECONDS_TILL_STALL_CHECK;
}

static void print_other_cpu_stall(struct rcu_state *rsp)
{
	int cpu;
	long delta;
	unsigned long flags;
	struct rcu_node *rnp = rcu_get_root(rsp);

	

	spin_lock_irqsave(&rnp->lock, flags);
	delta = jiffies - rsp->jiffies_stall;
	if (delta < RCU_STALL_RAT_DELAY || !rcu_gp_in_progress(rsp)) {
		spin_unlock_irqrestore(&rnp->lock, flags);
		return;
	}
	rsp->jiffies_stall = jiffies + RCU_SECONDS_TILL_STALL_RECHECK;

	
	rcu_print_task_stall(rnp);
	spin_unlock_irqrestore(&rnp->lock, flags);

	

	printk(KERN_ERR "INFO: RCU detected CPU stalls:");
	rcu_for_each_leaf_node(rsp, rnp) {
		rcu_print_task_stall(rnp);
		if (rnp->qsmask == 0)
			continue;
		for (cpu = 0; cpu <= rnp->grphi - rnp->grplo; cpu++)
			if (rnp->qsmask & (1UL << cpu))
				printk(" %d", rnp->grplo + cpu);
	}
	printk(" (detected by %d, t=%ld jiffies)\n",
	       smp_processor_id(), (long)(jiffies - rsp->gp_start));
	trigger_all_cpu_backtrace();

	force_quiescent_state(rsp, 0);  
}

static void print_cpu_stall(struct rcu_state *rsp)
{
	unsigned long flags;
	struct rcu_node *rnp = rcu_get_root(rsp);

	printk(KERN_ERR "INFO: RCU detected CPU %d stall (t=%lu jiffies)\n",
			smp_processor_id(), jiffies - rsp->gp_start);
	trigger_all_cpu_backtrace();

	spin_lock_irqsave(&rnp->lock, flags);
	if ((long)(jiffies - rsp->jiffies_stall) >= 0)
		rsp->jiffies_stall =
			jiffies + RCU_SECONDS_TILL_STALL_RECHECK;
	spin_unlock_irqrestore(&rnp->lock, flags);

	set_need_resched();  
}

static void check_cpu_stall(struct rcu_state *rsp, struct rcu_data *rdp)
{
	long delta;
	struct rcu_node *rnp;

	delta = jiffies - rsp->jiffies_stall;
	rnp = rdp->mynode;
	if ((rnp->qsmask & rdp->grpmask) && delta >= 0) {

		
		print_cpu_stall(rsp);

	} else if (rcu_gp_in_progress(rsp) && delta >= RCU_STALL_RAT_DELAY) {

		
		print_other_cpu_stall(rsp);
	}
}

#else 

static void record_gp_stall_check_time(struct rcu_state *rsp)
{
}

static void check_cpu_stall(struct rcu_state *rsp, struct rcu_data *rdp)
{
}

#endif 


static void __note_new_gpnum(struct rcu_state *rsp, struct rcu_node *rnp, struct rcu_data *rdp)
{
	if (rdp->gpnum != rnp->gpnum) {
		rdp->qs_pending = 1;
		rdp->passed_quiesc = 0;
		rdp->gpnum = rnp->gpnum;
	}
}

static void note_new_gpnum(struct rcu_state *rsp, struct rcu_data *rdp)
{
	unsigned long flags;
	struct rcu_node *rnp;

	local_irq_save(flags);
	rnp = rdp->mynode;
	if (rdp->gpnum == ACCESS_ONCE(rnp->gpnum) || 
	    !spin_trylock(&rnp->lock)) { 
		local_irq_restore(flags);
		return;
	}
	__note_new_gpnum(rsp, rnp, rdp);
	spin_unlock_irqrestore(&rnp->lock, flags);
}


static int
check_for_new_grace_period(struct rcu_state *rsp, struct rcu_data *rdp)
{
	unsigned long flags;
	int ret = 0;

	local_irq_save(flags);
	if (rdp->gpnum != rsp->gpnum) {
		note_new_gpnum(rsp, rdp);
		ret = 1;
	}
	local_irq_restore(flags);
	return ret;
}


static void
__rcu_process_gp_end(struct rcu_state *rsp, struct rcu_node *rnp, struct rcu_data *rdp)
{
	
	if (rdp->completed != rnp->completed) {

		
		rdp->nxttail[RCU_DONE_TAIL] = rdp->nxttail[RCU_WAIT_TAIL];
		rdp->nxttail[RCU_WAIT_TAIL] = rdp->nxttail[RCU_NEXT_READY_TAIL];
		rdp->nxttail[RCU_NEXT_READY_TAIL] = rdp->nxttail[RCU_NEXT_TAIL];

		
		rdp->completed = rnp->completed;
	}
}


static void
rcu_process_gp_end(struct rcu_state *rsp, struct rcu_data *rdp)
{
	unsigned long flags;
	struct rcu_node *rnp;

	local_irq_save(flags);
	rnp = rdp->mynode;
	if (rdp->completed == ACCESS_ONCE(rnp->completed) || 
	    !spin_trylock(&rnp->lock)) { 
		local_irq_restore(flags);
		return;
	}
	__rcu_process_gp_end(rsp, rnp, rdp);
	spin_unlock_irqrestore(&rnp->lock, flags);
}


static void
rcu_start_gp_per_cpu(struct rcu_state *rsp, struct rcu_node *rnp, struct rcu_data *rdp)
{
	
	__rcu_process_gp_end(rsp, rnp, rdp);

	
	rdp->nxttail[RCU_NEXT_READY_TAIL] = rdp->nxttail[RCU_NEXT_TAIL];
	rdp->nxttail[RCU_WAIT_TAIL] = rdp->nxttail[RCU_NEXT_TAIL];

	
	__note_new_gpnum(rsp, rnp, rdp);
}


static void
rcu_start_gp(struct rcu_state *rsp, unsigned long flags)
	__releases(rcu_get_root(rsp)->lock)
{
	struct rcu_data *rdp = rsp->rda[smp_processor_id()];
	struct rcu_node *rnp = rcu_get_root(rsp);

	if (!cpu_needs_another_gp(rsp, rdp)) {
		spin_unlock_irqrestore(&rnp->lock, flags);
		return;
	}

	
	rsp->gpnum++;
	WARN_ON_ONCE(rsp->signaled == RCU_GP_INIT);
	rsp->signaled = RCU_GP_INIT; 
	rsp->jiffies_force_qs = jiffies + RCU_JIFFIES_TILL_FORCE_QS;
	record_gp_stall_check_time(rsp);
	dyntick_record_completed(rsp, rsp->completed - 1);

	
	if (NUM_RCU_NODES == 1) {
		rcu_preempt_check_blocked_tasks(rnp);
		rnp->qsmask = rnp->qsmaskinit;
		rnp->gpnum = rsp->gpnum;
		rnp->completed = rsp->completed;
		rsp->signaled = RCU_SIGNAL_INIT; 
		rcu_start_gp_per_cpu(rsp, rnp, rdp);
		spin_unlock_irqrestore(&rnp->lock, flags);
		return;
	}

	spin_unlock(&rnp->lock);  


	
	spin_lock(&rsp->onofflock);  

	
	rcu_for_each_node_breadth_first(rsp, rnp) {
		spin_lock(&rnp->lock);		
		rcu_preempt_check_blocked_tasks(rnp);
		rnp->qsmask = rnp->qsmaskinit;
		rnp->gpnum = rsp->gpnum;
		rnp->completed = rsp->completed;
		if (rnp == rdp->mynode)
			rcu_start_gp_per_cpu(rsp, rnp, rdp);
		spin_unlock(&rnp->lock);	
	}

	rnp = rcu_get_root(rsp);
	spin_lock(&rnp->lock);			
	rsp->signaled = RCU_SIGNAL_INIT; 
	spin_unlock(&rnp->lock);		
	spin_unlock_irqrestore(&rsp->onofflock, flags);
}


static void cpu_quiet_msk_finish(struct rcu_state *rsp, unsigned long flags)
	__releases(rcu_get_root(rsp)->lock)
{
	WARN_ON_ONCE(!rcu_gp_in_progress(rsp));
	rsp->completed = rsp->gpnum;
	rsp->signaled = RCU_GP_IDLE;
	rcu_start_gp(rsp, flags);  
}


static void
cpu_quiet_msk(unsigned long mask, struct rcu_state *rsp, struct rcu_node *rnp,
	      unsigned long flags)
	__releases(rnp->lock)
{
	struct rcu_node *rnp_c;

	
	for (;;) {
		if (!(rnp->qsmask & mask)) {

			
			spin_unlock_irqrestore(&rnp->lock, flags);
			return;
		}
		rnp->qsmask &= ~mask;
		if (rnp->qsmask != 0 || rcu_preempted_readers(rnp)) {

			
			spin_unlock_irqrestore(&rnp->lock, flags);
			return;
		}
		mask = rnp->grpmask;
		if (rnp->parent == NULL) {

			

			break;
		}
		spin_unlock_irqrestore(&rnp->lock, flags);
		rnp_c = rnp;
		rnp = rnp->parent;
		spin_lock_irqsave(&rnp->lock, flags);
		WARN_ON_ONCE(rnp_c->qsmask);
	}

	
	cpu_quiet_msk_finish(rsp, flags); 
}


static void
cpu_quiet(int cpu, struct rcu_state *rsp, struct rcu_data *rdp, long lastcomp)
{
	unsigned long flags;
	unsigned long mask;
	struct rcu_node *rnp;

	rnp = rdp->mynode;
	spin_lock_irqsave(&rnp->lock, flags);
	if (lastcomp != ACCESS_ONCE(rsp->completed)) {

		
		rdp->passed_quiesc = 0;	
		spin_unlock_irqrestore(&rnp->lock, flags);
		return;
	}
	mask = rdp->grpmask;
	if ((rnp->qsmask & mask) == 0) {
		spin_unlock_irqrestore(&rnp->lock, flags);
	} else {
		rdp->qs_pending = 0;

		
		rdp->nxttail[RCU_NEXT_READY_TAIL] = rdp->nxttail[RCU_NEXT_TAIL];

		cpu_quiet_msk(mask, rsp, rnp, flags); 
	}
}


static void
rcu_check_quiescent_state(struct rcu_state *rsp, struct rcu_data *rdp)
{
	
	if (check_for_new_grace_period(rsp, rdp))
		return;

	
	if (!rdp->qs_pending)
		return;

	
	if (!rdp->passed_quiesc)
		return;

	
	cpu_quiet(rdp->cpu, rsp, rdp, rdp->passed_quiesc_completed);
}

#ifdef CONFIG_HOTPLUG_CPU


static void rcu_send_cbs_to_orphanage(struct rcu_state *rsp)
{
	int i;
	struct rcu_data *rdp = rsp->rda[smp_processor_id()];

	if (rdp->nxtlist == NULL)
		return;  
	spin_lock(&rsp->onofflock);  
	*rsp->orphan_cbs_tail = rdp->nxtlist;
	rsp->orphan_cbs_tail = rdp->nxttail[RCU_NEXT_TAIL];
	rdp->nxtlist = NULL;
	for (i = 0; i < RCU_NEXT_SIZE; i++)
		rdp->nxttail[i] = &rdp->nxtlist;
	rsp->orphan_qlen += rdp->qlen;
	rdp->qlen = 0;
	spin_unlock(&rsp->onofflock);  
}


static void rcu_adopt_orphan_cbs(struct rcu_state *rsp)
{
	unsigned long flags;
	struct rcu_data *rdp;

	spin_lock_irqsave(&rsp->onofflock, flags);
	rdp = rsp->rda[smp_processor_id()];
	if (rsp->orphan_cbs_list == NULL) {
		spin_unlock_irqrestore(&rsp->onofflock, flags);
		return;
	}
	*rdp->nxttail[RCU_NEXT_TAIL] = rsp->orphan_cbs_list;
	rdp->nxttail[RCU_NEXT_TAIL] = rsp->orphan_cbs_tail;
	rdp->qlen += rsp->orphan_qlen;
	rsp->orphan_cbs_list = NULL;
	rsp->orphan_cbs_tail = &rsp->orphan_cbs_list;
	rsp->orphan_qlen = 0;
	spin_unlock_irqrestore(&rsp->onofflock, flags);
}


static void __rcu_offline_cpu(int cpu, struct rcu_state *rsp)
{
	unsigned long flags;
	long lastcomp;
	unsigned long mask;
	struct rcu_data *rdp = rsp->rda[cpu];
	struct rcu_node *rnp;

	
	spin_lock_irqsave(&rsp->onofflock, flags);

	
	rnp = rdp->mynode;	
	mask = rdp->grpmask;	
	do {
		spin_lock(&rnp->lock);		
		rnp->qsmaskinit &= ~mask;
		if (rnp->qsmaskinit != 0) {
			spin_unlock(&rnp->lock); 
			break;
		}

		
		if (rcu_preempt_offline_tasks(rsp, rnp, rdp) && !rnp->qsmask)
			rnp->qsmask |= mask;

		mask = rnp->grpmask;
		spin_unlock(&rnp->lock);	
		rnp = rnp->parent;
	} while (rnp != NULL);
	lastcomp = rsp->completed;

	spin_unlock_irqrestore(&rsp->onofflock, flags);

	rcu_adopt_orphan_cbs(rsp);
}


static void rcu_offline_cpu(int cpu)
{
	__rcu_offline_cpu(cpu, &rcu_sched_state);
	__rcu_offline_cpu(cpu, &rcu_bh_state);
	rcu_preempt_offline_cpu(cpu);
}

#else 

static void rcu_send_cbs_to_orphanage(struct rcu_state *rsp)
{
}

static void rcu_adopt_orphan_cbs(struct rcu_state *rsp)
{
}

static void rcu_offline_cpu(int cpu)
{
}

#endif 


static void rcu_do_batch(struct rcu_state *rsp, struct rcu_data *rdp)
{
	unsigned long flags;
	struct rcu_head *next, *list, **tail;
	int count;

	
	if (!cpu_has_callbacks_ready_to_invoke(rdp))
		return;

	
	local_irq_save(flags);
	list = rdp->nxtlist;
	rdp->nxtlist = *rdp->nxttail[RCU_DONE_TAIL];
	*rdp->nxttail[RCU_DONE_TAIL] = NULL;
	tail = rdp->nxttail[RCU_DONE_TAIL];
	for (count = RCU_NEXT_SIZE - 1; count >= 0; count--)
		if (rdp->nxttail[count] == rdp->nxttail[RCU_DONE_TAIL])
			rdp->nxttail[count] = &rdp->nxtlist;
	local_irq_restore(flags);

	
	count = 0;
	while (list) {
		next = list->next;
		prefetch(next);
		list->func(list);
		list = next;
		if (++count >= rdp->blimit)
			break;
	}

	local_irq_save(flags);

	
	rdp->qlen -= count;
	if (list != NULL) {
		*tail = rdp->nxtlist;
		rdp->nxtlist = list;
		for (count = 0; count < RCU_NEXT_SIZE; count++)
			if (&rdp->nxtlist == rdp->nxttail[count])
				rdp->nxttail[count] = tail;
			else
				break;
	}

	
	if (rdp->blimit == LONG_MAX && rdp->qlen <= qlowmark)
		rdp->blimit = blimit;

	
	if (rdp->qlen == 0 && rdp->qlen_last_fqs_check != 0) {
		rdp->qlen_last_fqs_check = 0;
		rdp->n_force_qs_snap = rsp->n_force_qs;
	} else if (rdp->qlen < rdp->qlen_last_fqs_check - qhimark)
		rdp->qlen_last_fqs_check = rdp->qlen;

	local_irq_restore(flags);

	
	if (cpu_has_callbacks_ready_to_invoke(rdp))
		raise_softirq(RCU_SOFTIRQ);
}


void rcu_check_callbacks(int cpu, int user)
{
	if (!rcu_pending(cpu))
		return; 
	if (user ||
	    (idle_cpu(cpu) && rcu_scheduler_active &&
	     !in_softirq() && hardirq_count() <= (1 << HARDIRQ_SHIFT))) {

		

		rcu_sched_qs(cpu);
		rcu_bh_qs(cpu);

	} else if (!in_softirq()) {

		

		rcu_bh_qs(cpu);
	}
	rcu_preempt_check_callbacks(cpu);
	raise_softirq(RCU_SOFTIRQ);
}

#ifdef CONFIG_SMP


static int rcu_process_dyntick(struct rcu_state *rsp, long lastcomp,
			       int (*f)(struct rcu_data *))
{
	unsigned long bit;
	int cpu;
	unsigned long flags;
	unsigned long mask;
	struct rcu_node *rnp;

	rcu_for_each_leaf_node(rsp, rnp) {
		mask = 0;
		spin_lock_irqsave(&rnp->lock, flags);
		if (rsp->completed != lastcomp) {
			spin_unlock_irqrestore(&rnp->lock, flags);
			return 1;
		}
		if (rnp->qsmask == 0) {
			spin_unlock_irqrestore(&rnp->lock, flags);
			continue;
		}
		cpu = rnp->grplo;
		bit = 1;
		for (; cpu <= rnp->grphi; cpu++, bit <<= 1) {
			if ((rnp->qsmask & bit) != 0 && f(rsp->rda[cpu]))
				mask |= bit;
		}
		if (mask != 0 && rsp->completed == lastcomp) {

			
			cpu_quiet_msk(mask, rsp, rnp, flags);
			continue;
		}
		spin_unlock_irqrestore(&rnp->lock, flags);
	}
	return 0;
}


static void force_quiescent_state(struct rcu_state *rsp, int relaxed)
{
	unsigned long flags;
	long lastcomp;
	struct rcu_node *rnp = rcu_get_root(rsp);
	u8 signaled;
	u8 forcenow;

	if (!rcu_gp_in_progress(rsp))
		return;  
	if (!spin_trylock_irqsave(&rsp->fqslock, flags)) {
		rsp->n_force_qs_lh++; 
		return;	
	}
	if (relaxed &&
	    (long)(rsp->jiffies_force_qs - jiffies) >= 0)
		goto unlock_ret; 
	rsp->n_force_qs++;
	spin_lock(&rnp->lock);
	lastcomp = rsp->completed;
	signaled = rsp->signaled;
	rsp->jiffies_force_qs = jiffies + RCU_JIFFIES_TILL_FORCE_QS;
	if (lastcomp == rsp->gpnum) {
		rsp->n_force_qs_ngp++;
		spin_unlock(&rnp->lock);
		goto unlock_ret;  
	}
	spin_unlock(&rnp->lock);
	switch (signaled) {
	case RCU_GP_IDLE:
	case RCU_GP_INIT:

		break; 

	case RCU_SAVE_DYNTICK:

		if (RCU_SIGNAL_INIT != RCU_SAVE_DYNTICK)
			break; 

		
		if (rcu_process_dyntick(rsp, lastcomp,
					dyntick_save_progress_counter))
			goto unlock_ret;
		

	case RCU_SAVE_COMPLETED:

		
		forcenow = 0;
		spin_lock(&rnp->lock);
		if (lastcomp == rsp->completed &&
		    rsp->signaled == signaled) {
			rsp->signaled = RCU_FORCE_QS;
			dyntick_record_completed(rsp, lastcomp);
			forcenow = signaled == RCU_SAVE_COMPLETED;
		}
		spin_unlock(&rnp->lock);
		if (!forcenow)
			break;
		

	case RCU_FORCE_QS:

		
		if (rcu_process_dyntick(rsp, dyntick_recall_completed(rsp),
					rcu_implicit_dynticks_qs))
			goto unlock_ret;

		

		break;
	}
unlock_ret:
	spin_unlock_irqrestore(&rsp->fqslock, flags);
}

#else 

static void force_quiescent_state(struct rcu_state *rsp, int relaxed)
{
	set_need_resched();
}

#endif 


static void
__rcu_process_callbacks(struct rcu_state *rsp, struct rcu_data *rdp)
{
	unsigned long flags;

	WARN_ON_ONCE(rdp->beenonline == 0);

	
	if ((long)(ACCESS_ONCE(rsp->jiffies_force_qs) - jiffies) < 0)
		force_quiescent_state(rsp, 1);

	
	rcu_process_gp_end(rsp, rdp);

	
	rcu_check_quiescent_state(rsp, rdp);

	
	if (cpu_needs_another_gp(rsp, rdp)) {
		spin_lock_irqsave(&rcu_get_root(rsp)->lock, flags);
		rcu_start_gp(rsp, flags);  
	}

	
	rcu_do_batch(rsp, rdp);
}


static void rcu_process_callbacks(struct softirq_action *unused)
{
	
	smp_mb(); 

	__rcu_process_callbacks(&rcu_sched_state,
				&__get_cpu_var(rcu_sched_data));
	__rcu_process_callbacks(&rcu_bh_state, &__get_cpu_var(rcu_bh_data));
	rcu_preempt_process_callbacks();

	
	smp_mb(); 
}

static void
__call_rcu(struct rcu_head *head, void (*func)(struct rcu_head *rcu),
	   struct rcu_state *rsp)
{
	unsigned long flags;
	struct rcu_data *rdp;

	head->func = func;
	head->next = NULL;

	smp_mb(); 

	
	local_irq_save(flags);
	rdp = rsp->rda[smp_processor_id()];
	rcu_process_gp_end(rsp, rdp);
	check_for_new_grace_period(rsp, rdp);

	
	*rdp->nxttail[RCU_NEXT_TAIL] = head;
	rdp->nxttail[RCU_NEXT_TAIL] = &head->next;

	
	if (!rcu_gp_in_progress(rsp)) {
		unsigned long nestflag;
		struct rcu_node *rnp_root = rcu_get_root(rsp);

		spin_lock_irqsave(&rnp_root->lock, nestflag);
		rcu_start_gp(rsp, nestflag);  
	}

	
	if (unlikely(++rdp->qlen > rdp->qlen_last_fqs_check + qhimark)) {
		rdp->blimit = LONG_MAX;
		if (rsp->n_force_qs == rdp->n_force_qs_snap &&
		    *rdp->nxttail[RCU_DONE_TAIL] != head)
			force_quiescent_state(rsp, 0);
		rdp->n_force_qs_snap = rsp->n_force_qs;
		rdp->qlen_last_fqs_check = rdp->qlen;
	} else if ((long)(ACCESS_ONCE(rsp->jiffies_force_qs) - jiffies) < 0)
		force_quiescent_state(rsp, 1);
	local_irq_restore(flags);
}


void call_rcu_sched(struct rcu_head *head, void (*func)(struct rcu_head *rcu))
{
	__call_rcu(head, func, &rcu_sched_state);
}
EXPORT_SYMBOL_GPL(call_rcu_sched);


void call_rcu_bh(struct rcu_head *head, void (*func)(struct rcu_head *rcu))
{
	__call_rcu(head, func, &rcu_bh_state);
}
EXPORT_SYMBOL_GPL(call_rcu_bh);


static int __rcu_pending(struct rcu_state *rsp, struct rcu_data *rdp)
{
	rdp->n_rcu_pending++;

	
	check_cpu_stall(rsp, rdp);

	
	if (rdp->qs_pending) {
		rdp->n_rp_qs_pending++;
		return 1;
	}

	
	if (cpu_has_callbacks_ready_to_invoke(rdp)) {
		rdp->n_rp_cb_ready++;
		return 1;
	}

	
	if (cpu_needs_another_gp(rsp, rdp)) {
		rdp->n_rp_cpu_needs_gp++;
		return 1;
	}

	
	if (ACCESS_ONCE(rsp->completed) != rdp->completed) { 
		rdp->n_rp_gp_completed++;
		return 1;
	}

	
	if (ACCESS_ONCE(rsp->gpnum) != rdp->gpnum) { 
		rdp->n_rp_gp_started++;
		return 1;
	}

	
	if (rcu_gp_in_progress(rsp) &&
	    ((long)(ACCESS_ONCE(rsp->jiffies_force_qs) - jiffies) < 0)) {
		rdp->n_rp_need_fqs++;
		return 1;
	}

	
	rdp->n_rp_need_nothing++;
	return 0;
}


static int rcu_pending(int cpu)
{
	return __rcu_pending(&rcu_sched_state, &per_cpu(rcu_sched_data, cpu)) ||
	       __rcu_pending(&rcu_bh_state, &per_cpu(rcu_bh_data, cpu)) ||
	       rcu_preempt_pending(cpu);
}


int rcu_needs_cpu(int cpu)
{
	
	return per_cpu(rcu_sched_data, cpu).nxtlist ||
	       per_cpu(rcu_bh_data, cpu).nxtlist ||
	       rcu_preempt_needs_cpu(cpu);
}

static DEFINE_PER_CPU(struct rcu_head, rcu_barrier_head) = {NULL};
static atomic_t rcu_barrier_cpu_count;
static DEFINE_MUTEX(rcu_barrier_mutex);
static struct completion rcu_barrier_completion;

static void rcu_barrier_callback(struct rcu_head *notused)
{
	if (atomic_dec_and_test(&rcu_barrier_cpu_count))
		complete(&rcu_barrier_completion);
}


static void rcu_barrier_func(void *type)
{
	int cpu = smp_processor_id();
	struct rcu_head *head = &per_cpu(rcu_barrier_head, cpu);
	void (*call_rcu_func)(struct rcu_head *head,
			      void (*func)(struct rcu_head *head));

	atomic_inc(&rcu_barrier_cpu_count);
	call_rcu_func = type;
	call_rcu_func(head, rcu_barrier_callback);
}


static void _rcu_barrier(struct rcu_state *rsp,
			 void (*call_rcu_func)(struct rcu_head *head,
					       void (*func)(struct rcu_head *head)))
{
	BUG_ON(in_interrupt());
	
	mutex_lock(&rcu_barrier_mutex);
	init_completion(&rcu_barrier_completion);
	
	atomic_set(&rcu_barrier_cpu_count, 1);
	preempt_disable(); 
	rcu_adopt_orphan_cbs(rsp);
	on_each_cpu(rcu_barrier_func, (void *)call_rcu_func, 1);
	preempt_enable(); 
	if (atomic_dec_and_test(&rcu_barrier_cpu_count))
		complete(&rcu_barrier_completion);
	wait_for_completion(&rcu_barrier_completion);
	mutex_unlock(&rcu_barrier_mutex);
}


void rcu_barrier_bh(void)
{
	_rcu_barrier(&rcu_bh_state, call_rcu_bh);
}
EXPORT_SYMBOL_GPL(rcu_barrier_bh);


void rcu_barrier_sched(void)
{
	_rcu_barrier(&rcu_sched_state, call_rcu_sched);
}
EXPORT_SYMBOL_GPL(rcu_barrier_sched);


static void __init
rcu_boot_init_percpu_data(int cpu, struct rcu_state *rsp)
{
	unsigned long flags;
	int i;
	struct rcu_data *rdp = rsp->rda[cpu];
	struct rcu_node *rnp = rcu_get_root(rsp);

	
	spin_lock_irqsave(&rnp->lock, flags);
	rdp->grpmask = 1UL << (cpu - rdp->mynode->grplo);
	rdp->nxtlist = NULL;
	for (i = 0; i < RCU_NEXT_SIZE; i++)
		rdp->nxttail[i] = &rdp->nxtlist;
	rdp->qlen = 0;
#ifdef CONFIG_NO_HZ
	rdp->dynticks = &per_cpu(rcu_dynticks, cpu);
#endif 
	rdp->cpu = cpu;
	spin_unlock_irqrestore(&rnp->lock, flags);
}


static void __cpuinit
rcu_init_percpu_data(int cpu, struct rcu_state *rsp, int preemptable)
{
	unsigned long flags;
	unsigned long mask;
	struct rcu_data *rdp = rsp->rda[cpu];
	struct rcu_node *rnp = rcu_get_root(rsp);

	
	spin_lock_irqsave(&rnp->lock, flags);
	rdp->passed_quiesc = 0;  
	rdp->qs_pending = 1;	 
	rdp->beenonline = 1;	 
	rdp->preemptable = preemptable;
	rdp->qlen_last_fqs_check = 0;
	rdp->n_force_qs_snap = rsp->n_force_qs;
	rdp->blimit = blimit;
	spin_unlock(&rnp->lock);		

	

	
	spin_lock(&rsp->onofflock);		

	
	rnp = rdp->mynode;
	mask = rdp->grpmask;
	do {
		
		spin_lock(&rnp->lock);	
		rnp->qsmaskinit |= mask;
		mask = rnp->grpmask;
		if (rnp == rdp->mynode) {
			rdp->gpnum = rnp->completed; 
			rdp->completed = rnp->completed;
			rdp->passed_quiesc_completed = rnp->completed - 1;
		}
		spin_unlock(&rnp->lock); 
		rnp = rnp->parent;
	} while (rnp != NULL && !(rnp->qsmaskinit & mask));

	spin_unlock_irqrestore(&rsp->onofflock, flags);
}

static void __cpuinit rcu_online_cpu(int cpu)
{
	rcu_init_percpu_data(cpu, &rcu_sched_state, 0);
	rcu_init_percpu_data(cpu, &rcu_bh_state, 0);
	rcu_preempt_init_percpu_data(cpu);
}


int __cpuinit rcu_cpu_notify(struct notifier_block *self,
			     unsigned long action, void *hcpu)
{
	long cpu = (long)hcpu;

	switch (action) {
	case CPU_UP_PREPARE:
	case CPU_UP_PREPARE_FROZEN:
		rcu_online_cpu(cpu);
		break;
	case CPU_DYING:
	case CPU_DYING_FROZEN:
		
		rcu_send_cbs_to_orphanage(&rcu_bh_state);
		rcu_send_cbs_to_orphanage(&rcu_sched_state);
		rcu_preempt_send_cbs_to_orphanage();
		break;
	case CPU_DEAD:
	case CPU_DEAD_FROZEN:
	case CPU_UP_CANCELED:
	case CPU_UP_CANCELED_FROZEN:
		rcu_offline_cpu(cpu);
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}


#ifdef CONFIG_RCU_FANOUT_EXACT
static void __init rcu_init_levelspread(struct rcu_state *rsp)
{
	int i;

	for (i = NUM_RCU_LVLS - 1; i >= 0; i--)
		rsp->levelspread[i] = CONFIG_RCU_FANOUT;
}
#else 
static void __init rcu_init_levelspread(struct rcu_state *rsp)
{
	int ccur;
	int cprv;
	int i;

	cprv = NR_CPUS;
	for (i = NUM_RCU_LVLS - 1; i >= 0; i--) {
		ccur = rsp->levelcnt[i];
		rsp->levelspread[i] = (cprv + ccur - 1) / ccur;
		cprv = ccur;
	}
}
#endif 


static void __init rcu_init_one(struct rcu_state *rsp)
{
	int cpustride = 1;
	int i;
	int j;
	struct rcu_node *rnp;

	

	for (i = 1; i < NUM_RCU_LVLS; i++)
		rsp->level[i] = rsp->level[i - 1] + rsp->levelcnt[i - 1];
	rcu_init_levelspread(rsp);

	

	for (i = NUM_RCU_LVLS - 1; i >= 0; i--) {
		cpustride *= rsp->levelspread[i];
		rnp = rsp->level[i];
		for (j = 0; j < rsp->levelcnt[i]; j++, rnp++) {
			if (rnp != rcu_get_root(rsp))
				spin_lock_init(&rnp->lock);
			rnp->gpnum = 0;
			rnp->qsmask = 0;
			rnp->qsmaskinit = 0;
			rnp->grplo = j * cpustride;
			rnp->grphi = (j + 1) * cpustride - 1;
			if (rnp->grphi >= NR_CPUS)
				rnp->grphi = NR_CPUS - 1;
			if (i == 0) {
				rnp->grpnum = 0;
				rnp->grpmask = 0;
				rnp->parent = NULL;
			} else {
				rnp->grpnum = j % rsp->levelspread[i - 1];
				rnp->grpmask = 1UL << rnp->grpnum;
				rnp->parent = rsp->level[i - 1] +
					      j / rsp->levelspread[i - 1];
			}
			rnp->level = i;
			INIT_LIST_HEAD(&rnp->blocked_tasks[0]);
			INIT_LIST_HEAD(&rnp->blocked_tasks[1]);
		}
	}
	spin_lock_init(&rcu_get_root(rsp)->lock);
}


#define RCU_INIT_FLAVOR(rsp, rcu_data) \
do { \
	int i; \
	int j; \
	struct rcu_node *rnp; \
	\
	rcu_init_one(rsp); \
	rnp = (rsp)->level[NUM_RCU_LVLS - 1]; \
	j = 0; \
	for_each_possible_cpu(i) { \
		if (i > rnp[j].grphi) \
			j++; \
		per_cpu(rcu_data, i).mynode = &rnp[j]; \
		(rsp)->rda[i] = &per_cpu(rcu_data, i); \
		rcu_boot_init_percpu_data(i, rsp); \
	} \
} while (0)

void __init __rcu_init(void)
{
	rcu_bootup_announce();
#ifdef CONFIG_RCU_CPU_STALL_DETECTOR
	printk(KERN_INFO "RCU-based detection of stalled CPUs is enabled.\n");
#endif 
	RCU_INIT_FLAVOR(&rcu_sched_state, rcu_sched_data);
	RCU_INIT_FLAVOR(&rcu_bh_state, rcu_bh_data);
	__rcu_init_preempt();
	open_softirq(RCU_SOFTIRQ, rcu_process_callbacks);
}

#include "rcutree_plugin.h"
