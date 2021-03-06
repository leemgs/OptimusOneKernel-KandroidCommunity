
#ifdef CONFIG_SCHEDSTATS

#define SCHEDSTAT_VERSION 15

static int show_schedstat(struct seq_file *seq, void *v)
{
	int cpu;
	int mask_len = DIV_ROUND_UP(NR_CPUS, 32) * 9;
	char *mask_str = kmalloc(mask_len, GFP_KERNEL);

	if (mask_str == NULL)
		return -ENOMEM;

	seq_printf(seq, "version %d\n", SCHEDSTAT_VERSION);
	seq_printf(seq, "timestamp %lu\n", jiffies);
	for_each_online_cpu(cpu) {
		struct rq *rq = cpu_rq(cpu);
#ifdef CONFIG_SMP
		struct sched_domain *sd;
		int dcount = 0;
#endif

		
		seq_printf(seq,
		    "cpu%d %u %u %u %u %u %u %llu %llu %lu",
		    cpu, rq->yld_count,
		    rq->sched_switch, rq->sched_count, rq->sched_goidle,
		    rq->ttwu_count, rq->ttwu_local,
		    rq->rq_cpu_time,
		    rq->rq_sched_info.run_delay, rq->rq_sched_info.pcount);

		seq_printf(seq, "\n");

#ifdef CONFIG_SMP
		
		preempt_disable();
		for_each_domain(cpu, sd) {
			enum cpu_idle_type itype;

			cpumask_scnprintf(mask_str, mask_len,
					  sched_domain_span(sd));
			seq_printf(seq, "domain%d %s", dcount++, mask_str);
			for (itype = CPU_IDLE; itype < CPU_MAX_IDLE_TYPES;
					itype++) {
				seq_printf(seq, " %u %u %u %u %u %u %u %u",
				    sd->lb_count[itype],
				    sd->lb_balanced[itype],
				    sd->lb_failed[itype],
				    sd->lb_imbalance[itype],
				    sd->lb_gained[itype],
				    sd->lb_hot_gained[itype],
				    sd->lb_nobusyq[itype],
				    sd->lb_nobusyg[itype]);
			}
			seq_printf(seq,
				   " %u %u %u %u %u %u %u %u %u %u %u %u\n",
			    sd->alb_count, sd->alb_failed, sd->alb_pushed,
			    sd->sbe_count, sd->sbe_balanced, sd->sbe_pushed,
			    sd->sbf_count, sd->sbf_balanced, sd->sbf_pushed,
			    sd->ttwu_wake_remote, sd->ttwu_move_affine,
			    sd->ttwu_move_balance);
		}
		preempt_enable();
#endif
	}
	kfree(mask_str);
	return 0;
}

static int schedstat_open(struct inode *inode, struct file *file)
{
	unsigned int size = PAGE_SIZE * (1 + num_online_cpus() / 32);
	char *buf = kmalloc(size, GFP_KERNEL);
	struct seq_file *m;
	int res;

	if (!buf)
		return -ENOMEM;
	res = single_open(file, show_schedstat, NULL);
	if (!res) {
		m = file->private_data;
		m->buf = buf;
		m->size = size;
	} else
		kfree(buf);
	return res;
}

static const struct file_operations proc_schedstat_operations = {
	.open    = schedstat_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
};

static int __init proc_schedstat_init(void)
{
	proc_create("schedstat", 0, NULL, &proc_schedstat_operations);
	return 0;
}
module_init(proc_schedstat_init);


static inline void
rq_sched_info_arrive(struct rq *rq, unsigned long long delta)
{
	if (rq) {
		rq->rq_sched_info.run_delay += delta;
		rq->rq_sched_info.pcount++;
	}
}


static inline void
rq_sched_info_depart(struct rq *rq, unsigned long long delta)
{
	if (rq)
		rq->rq_cpu_time += delta;
}

static inline void
rq_sched_info_dequeued(struct rq *rq, unsigned long long delta)
{
	if (rq)
		rq->rq_sched_info.run_delay += delta;
}
# define schedstat_inc(rq, field)	do { (rq)->field++; } while (0)
# define schedstat_add(rq, field, amt)	do { (rq)->field += (amt); } while (0)
# define schedstat_set(var, val)	do { var = (val); } while (0)
#else 
static inline void
rq_sched_info_arrive(struct rq *rq, unsigned long long delta)
{}
static inline void
rq_sched_info_dequeued(struct rq *rq, unsigned long long delta)
{}
static inline void
rq_sched_info_depart(struct rq *rq, unsigned long long delta)
{}
# define schedstat_inc(rq, field)	do { } while (0)
# define schedstat_add(rq, field, amt)	do { } while (0)
# define schedstat_set(var, val)	do { } while (0)
#endif

#if defined(CONFIG_SCHEDSTATS) || defined(CONFIG_TASK_DELAY_ACCT)
static inline void sched_info_reset_dequeued(struct task_struct *t)
{
	t->sched_info.last_queued = 0;
}


static inline void sched_info_dequeued(struct task_struct *t)
{
	unsigned long long now = task_rq(t)->clock, delta = 0;

	if (unlikely(sched_info_on()))
		if (t->sched_info.last_queued)
			delta = now - t->sched_info.last_queued;
	sched_info_reset_dequeued(t);
	t->sched_info.run_delay += delta;

	rq_sched_info_dequeued(task_rq(t), delta);
}


static void sched_info_arrive(struct task_struct *t)
{
	unsigned long long now = task_rq(t)->clock, delta = 0;

	if (t->sched_info.last_queued)
		delta = now - t->sched_info.last_queued;
	sched_info_reset_dequeued(t);
	t->sched_info.run_delay += delta;
	t->sched_info.last_arrival = now;
	t->sched_info.pcount++;

	rq_sched_info_arrive(task_rq(t), delta);
}


static inline void sched_info_queued(struct task_struct *t)
{
	if (unlikely(sched_info_on()))
		if (!t->sched_info.last_queued)
			t->sched_info.last_queued = task_rq(t)->clock;
}


static inline void sched_info_depart(struct task_struct *t)
{
	unsigned long long delta = task_rq(t)->clock -
					t->sched_info.last_arrival;

	rq_sched_info_depart(task_rq(t), delta);

	if (t->state == TASK_RUNNING)
		sched_info_queued(t);
}


static inline void
__sched_info_switch(struct task_struct *prev, struct task_struct *next)
{
	struct rq *rq = task_rq(prev);

	
	if (prev != rq->idle)
		sched_info_depart(prev);

	if (next != rq->idle)
		sched_info_arrive(next);
}
static inline void
sched_info_switch(struct task_struct *prev, struct task_struct *next)
{
	if (unlikely(sched_info_on()))
		__sched_info_switch(prev, next);
}
#else
#define sched_info_queued(t)			do { } while (0)
#define sched_info_reset_dequeued(t)	do { } while (0)
#define sched_info_dequeued(t)			do { } while (0)
#define sched_info_switch(t, next)		do { } while (0)
#endif 




static inline void account_group_user_time(struct task_struct *tsk,
					   cputime_t cputime)
{
	struct thread_group_cputimer *cputimer;

	
	if (unlikely(tsk->exit_state))
		return;

	cputimer = &tsk->signal->cputimer;

	if (!cputimer->running)
		return;

	spin_lock(&cputimer->lock);
	cputimer->cputime.utime =
		cputime_add(cputimer->cputime.utime, cputime);
	spin_unlock(&cputimer->lock);
}


static inline void account_group_system_time(struct task_struct *tsk,
					     cputime_t cputime)
{
	struct thread_group_cputimer *cputimer;

	
	if (unlikely(tsk->exit_state))
		return;

	cputimer = &tsk->signal->cputimer;

	if (!cputimer->running)
		return;

	spin_lock(&cputimer->lock);
	cputimer->cputime.stime =
		cputime_add(cputimer->cputime.stime, cputime);
	spin_unlock(&cputimer->lock);
}


static inline void account_group_exec_runtime(struct task_struct *tsk,
					      unsigned long long ns)
{
	struct thread_group_cputimer *cputimer;
	struct signal_struct *sig;

	sig = tsk->signal;
	
	barrier();
	if (unlikely(!sig))
		return;

	cputimer = &sig->cputimer;

	if (!cputimer->running)
		return;

	spin_lock(&cputimer->lock);
	cputimer->cputime.sum_exec_runtime += ns;
	spin_unlock(&cputimer->lock);
}
