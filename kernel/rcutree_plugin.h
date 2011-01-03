


#ifdef CONFIG_TREE_PREEMPT_RCU

struct rcu_state rcu_preempt_state = RCU_STATE_INITIALIZER(rcu_preempt_state);
DEFINE_PER_CPU(struct rcu_data, rcu_preempt_data);


static void rcu_bootup_announce(void)
{
	printk(KERN_INFO
	       "Experimental preemptable hierarchical RCU implementation.\n");
}


long rcu_batches_completed_preempt(void)
{
	return rcu_preempt_state.completed;
}
EXPORT_SYMBOL_GPL(rcu_batches_completed_preempt);


long rcu_batches_completed(void)
{
	return rcu_batches_completed_preempt();
}
EXPORT_SYMBOL_GPL(rcu_batches_completed);


static void rcu_preempt_qs(int cpu)
{
	struct rcu_data *rdp = &per_cpu(rcu_preempt_data, cpu);
	rdp->passed_quiesc_completed = rdp->completed;
	barrier();
	rdp->passed_quiesc = 1;
}


static void rcu_preempt_note_context_switch(int cpu)
{
	struct task_struct *t = current;
	unsigned long flags;
	int phase;
	struct rcu_data *rdp;
	struct rcu_node *rnp;

	if (t->rcu_read_lock_nesting &&
	    (t->rcu_read_unlock_special & RCU_READ_UNLOCK_BLOCKED) == 0) {

		
		rdp = rcu_preempt_state.rda[cpu];
		rnp = rdp->mynode;
		spin_lock_irqsave(&rnp->lock, flags);
		t->rcu_read_unlock_special |= RCU_READ_UNLOCK_BLOCKED;
		t->rcu_blocked_node = rnp;

		
		WARN_ON_ONCE((rdp->grpmask & rnp->qsmaskinit) == 0);
		WARN_ON_ONCE(!list_empty(&t->rcu_node_entry));
		phase = (rnp->gpnum + !(rnp->qsmask & rdp->grpmask)) & 0x1;
		list_add(&t->rcu_node_entry, &rnp->blocked_tasks[phase]);
		spin_unlock_irqrestore(&rnp->lock, flags);
	}

	
	rcu_preempt_qs(cpu);
	local_irq_save(flags);
	t->rcu_read_unlock_special &= ~RCU_READ_UNLOCK_NEED_QS;
	local_irq_restore(flags);
}


void __rcu_read_lock(void)
{
	ACCESS_ONCE(current->rcu_read_lock_nesting)++;
	barrier();  
}
EXPORT_SYMBOL_GPL(__rcu_read_lock);


static int rcu_preempted_readers(struct rcu_node *rnp)
{
	return !list_empty(&rnp->blocked_tasks[rnp->gpnum & 0x1]);
}

static void rcu_read_unlock_special(struct task_struct *t)
{
	int empty;
	unsigned long flags;
	unsigned long mask;
	struct rcu_node *rnp;
	int special;

	
	if (in_nmi())
		return;

	local_irq_save(flags);

	
	special = t->rcu_read_unlock_special;
	if (special & RCU_READ_UNLOCK_NEED_QS) {
		t->rcu_read_unlock_special &= ~RCU_READ_UNLOCK_NEED_QS;
		rcu_preempt_qs(smp_processor_id());
	}

	
	if (in_irq()) {
		local_irq_restore(flags);
		return;
	}

	
	if (special & RCU_READ_UNLOCK_BLOCKED) {
		t->rcu_read_unlock_special &= ~RCU_READ_UNLOCK_BLOCKED;

		
		for (;;) {
			rnp = t->rcu_blocked_node;
			spin_lock(&rnp->lock);  
			if (rnp == t->rcu_blocked_node)
				break;
			spin_unlock(&rnp->lock);  
		}
		empty = !rcu_preempted_readers(rnp);
		list_del_init(&t->rcu_node_entry);
		t->rcu_blocked_node = NULL;

		
		if (!empty && rnp->qsmask == 0 &&
		    !rcu_preempted_readers(rnp)) {
			struct rcu_node *rnp_p;

			if (rnp->parent == NULL) {
				
				cpu_quiet_msk_finish(&rcu_preempt_state, flags);
				return;
			}
			
			mask = rnp->grpmask;
			spin_unlock_irqrestore(&rnp->lock, flags);
			rnp_p = rnp->parent;
			spin_lock_irqsave(&rnp_p->lock, flags);
			WARN_ON_ONCE(rnp->qsmask);
			cpu_quiet_msk(mask, &rcu_preempt_state, rnp_p, flags);
			return;
		}
		spin_unlock(&rnp->lock);
	}
	local_irq_restore(flags);
}


void __rcu_read_unlock(void)
{
	struct task_struct *t = current;

	barrier();  
	if (--ACCESS_ONCE(t->rcu_read_lock_nesting) == 0 &&
	    unlikely(ACCESS_ONCE(t->rcu_read_unlock_special)))
		rcu_read_unlock_special(t);
}
EXPORT_SYMBOL_GPL(__rcu_read_unlock);

#ifdef CONFIG_RCU_CPU_STALL_DETECTOR


static void rcu_print_task_stall(struct rcu_node *rnp)
{
	unsigned long flags;
	struct list_head *lp;
	int phase;
	struct task_struct *t;

	if (rcu_preempted_readers(rnp)) {
		spin_lock_irqsave(&rnp->lock, flags);
		phase = rnp->gpnum & 0x1;
		lp = &rnp->blocked_tasks[phase];
		list_for_each_entry(t, lp, rcu_node_entry)
			printk(" P%d", t->pid);
		spin_unlock_irqrestore(&rnp->lock, flags);
	}
}

#endif 


static void rcu_preempt_check_blocked_tasks(struct rcu_node *rnp)
{
	WARN_ON_ONCE(rcu_preempted_readers(rnp));
	WARN_ON_ONCE(rnp->qsmask);
}

#ifdef CONFIG_HOTPLUG_CPU


static int rcu_preempt_offline_tasks(struct rcu_state *rsp,
				     struct rcu_node *rnp,
				     struct rcu_data *rdp)
{
	int i;
	struct list_head *lp;
	struct list_head *lp_root;
	int retval = rcu_preempted_readers(rnp);
	struct rcu_node *rnp_root = rcu_get_root(rsp);
	struct task_struct *tp;

	if (rnp == rnp_root) {
		WARN_ONCE(1, "Last CPU thought to be offlined?");
		return 0;  
	}
	WARN_ON_ONCE(rnp != rdp->mynode &&
		     (!list_empty(&rnp->blocked_tasks[0]) ||
		      !list_empty(&rnp->blocked_tasks[1])));

	
	for (i = 0; i < 2; i++) {
		lp = &rnp->blocked_tasks[i];
		lp_root = &rnp_root->blocked_tasks[i];
		while (!list_empty(lp)) {
			tp = list_entry(lp->next, typeof(*tp), rcu_node_entry);
			spin_lock(&rnp_root->lock); 
			list_del(&tp->rcu_node_entry);
			tp->rcu_blocked_node = rnp_root;
			list_add(&tp->rcu_node_entry, lp_root);
			spin_unlock(&rnp_root->lock); 
		}
	}

	return retval;
}


static void rcu_preempt_offline_cpu(int cpu)
{
	__rcu_offline_cpu(cpu, &rcu_preempt_state);
}

#endif 


static void rcu_preempt_check_callbacks(int cpu)
{
	struct task_struct *t = current;

	if (t->rcu_read_lock_nesting == 0) {
		t->rcu_read_unlock_special &= ~RCU_READ_UNLOCK_NEED_QS;
		rcu_preempt_qs(cpu);
		return;
	}
	if (per_cpu(rcu_preempt_data, cpu).qs_pending)
		t->rcu_read_unlock_special |= RCU_READ_UNLOCK_NEED_QS;
}


static void rcu_preempt_process_callbacks(void)
{
	__rcu_process_callbacks(&rcu_preempt_state,
				&__get_cpu_var(rcu_preempt_data));
}


void call_rcu(struct rcu_head *head, void (*func)(struct rcu_head *rcu))
{
	__call_rcu(head, func, &rcu_preempt_state);
}
EXPORT_SYMBOL_GPL(call_rcu);


void synchronize_rcu_expedited(void)
{
	synchronize_rcu();
}
EXPORT_SYMBOL_GPL(synchronize_rcu_expedited);


static int rcu_preempt_pending(int cpu)
{
	return __rcu_pending(&rcu_preempt_state,
			     &per_cpu(rcu_preempt_data, cpu));
}


static int rcu_preempt_needs_cpu(int cpu)
{
	return !!per_cpu(rcu_preempt_data, cpu).nxtlist;
}


void rcu_barrier(void)
{
	_rcu_barrier(&rcu_preempt_state, call_rcu);
}
EXPORT_SYMBOL_GPL(rcu_barrier);


static void __cpuinit rcu_preempt_init_percpu_data(int cpu)
{
	rcu_init_percpu_data(cpu, &rcu_preempt_state, 1);
}


static void rcu_preempt_send_cbs_to_orphanage(void)
{
	rcu_send_cbs_to_orphanage(&rcu_preempt_state);
}


static void __init __rcu_init_preempt(void)
{
	RCU_INIT_FLAVOR(&rcu_preempt_state, rcu_preempt_data);
}


void exit_rcu(void)
{
	struct task_struct *t = current;

	if (t->rcu_read_lock_nesting == 0)
		return;
	t->rcu_read_lock_nesting = 1;
	rcu_read_unlock();
}

#else 


static void rcu_bootup_announce(void)
{
	printk(KERN_INFO "Hierarchical RCU implementation.\n");
}


long rcu_batches_completed(void)
{
	return rcu_batches_completed_sched();
}
EXPORT_SYMBOL_GPL(rcu_batches_completed);


static void rcu_preempt_note_context_switch(int cpu)
{
}


static int rcu_preempted_readers(struct rcu_node *rnp)
{
	return 0;
}

#ifdef CONFIG_RCU_CPU_STALL_DETECTOR


static void rcu_print_task_stall(struct rcu_node *rnp)
{
}

#endif 


static void rcu_preempt_check_blocked_tasks(struct rcu_node *rnp)
{
	WARN_ON_ONCE(rnp->qsmask);
}

#ifdef CONFIG_HOTPLUG_CPU


static int rcu_preempt_offline_tasks(struct rcu_state *rsp,
				     struct rcu_node *rnp,
				     struct rcu_data *rdp)
{
	return 0;
}


static void rcu_preempt_offline_cpu(int cpu)
{
}

#endif 


static void rcu_preempt_check_callbacks(int cpu)
{
}


static void rcu_preempt_process_callbacks(void)
{
}


void call_rcu(struct rcu_head *head, void (*func)(struct rcu_head *rcu))
{
	call_rcu_sched(head, func);
}
EXPORT_SYMBOL_GPL(call_rcu);


void synchronize_rcu_expedited(void)
{
	synchronize_sched_expedited();
}
EXPORT_SYMBOL_GPL(synchronize_rcu_expedited);


static int rcu_preempt_pending(int cpu)
{
	return 0;
}


static int rcu_preempt_needs_cpu(int cpu)
{
	return 0;
}


void rcu_barrier(void)
{
	rcu_barrier_sched();
}
EXPORT_SYMBOL_GPL(rcu_barrier);


static void __cpuinit rcu_preempt_init_percpu_data(int cpu)
{
}


static void rcu_preempt_send_cbs_to_orphanage(void)
{
}


static void __init __rcu_init_preempt(void)
{
}

#endif 
