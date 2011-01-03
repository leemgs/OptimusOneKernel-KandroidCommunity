

#include <linux/latencytop.h>


unsigned int sysctl_sched_latency = 5000000ULL;
unsigned int normalized_sysctl_sched_latency = 5000000ULL;


unsigned int sysctl_sched_min_granularity = 1000000ULL;
unsigned int normalized_sysctl_sched_min_granularity = 1000000ULL;


static unsigned int sched_nr_latency = 5;


unsigned int sysctl_sched_child_runs_first __read_mostly;


unsigned int __read_mostly sysctl_sched_compat_yield;


unsigned int sysctl_sched_wakeup_granularity = 1000000UL;
unsigned int normalized_sysctl_sched_wakeup_granularity = 1000000UL;

const_debug unsigned int sysctl_sched_migration_cost = 500000UL;

static const struct sched_class fair_sched_class;



#ifdef CONFIG_FAIR_GROUP_SCHED


static inline struct rq *rq_of(struct cfs_rq *cfs_rq)
{
	return cfs_rq->rq;
}


#define entity_is_task(se)	(!se->my_q)

static inline struct task_struct *task_of(struct sched_entity *se)
{
#ifdef CONFIG_SCHED_DEBUG
	WARN_ON_ONCE(!entity_is_task(se));
#endif
	return container_of(se, struct task_struct, se);
}


#define for_each_sched_entity(se) \
		for (; se; se = se->parent)

static inline struct cfs_rq *task_cfs_rq(struct task_struct *p)
{
	return p->se.cfs_rq;
}


static inline struct cfs_rq *cfs_rq_of(struct sched_entity *se)
{
	return se->cfs_rq;
}


static inline struct cfs_rq *group_cfs_rq(struct sched_entity *grp)
{
	return grp->my_q;
}


static inline struct cfs_rq *cpu_cfs_rq(struct cfs_rq *cfs_rq, int this_cpu)
{
	return cfs_rq->tg->cfs_rq[this_cpu];
}


#define for_each_leaf_cfs_rq(rq, cfs_rq) \
	list_for_each_entry_rcu(cfs_rq, &rq->leaf_cfs_rq_list, leaf_cfs_rq_list)


static inline int
is_same_group(struct sched_entity *se, struct sched_entity *pse)
{
	if (se->cfs_rq == pse->cfs_rq)
		return 1;

	return 0;
}

static inline struct sched_entity *parent_entity(struct sched_entity *se)
{
	return se->parent;
}


static inline int depth_se(struct sched_entity *se)
{
	int depth = 0;

	for_each_sched_entity(se)
		depth++;

	return depth;
}

static void
find_matching_se(struct sched_entity **se, struct sched_entity **pse)
{
	int se_depth, pse_depth;

	

	
	se_depth = depth_se(*se);
	pse_depth = depth_se(*pse);

	while (se_depth > pse_depth) {
		se_depth--;
		*se = parent_entity(*se);
	}

	while (pse_depth > se_depth) {
		pse_depth--;
		*pse = parent_entity(*pse);
	}

	while (!is_same_group(*se, *pse)) {
		*se = parent_entity(*se);
		*pse = parent_entity(*pse);
	}
}

#else	

static inline struct task_struct *task_of(struct sched_entity *se)
{
	return container_of(se, struct task_struct, se);
}

static inline struct rq *rq_of(struct cfs_rq *cfs_rq)
{
	return container_of(cfs_rq, struct rq, cfs);
}

#define entity_is_task(se)	1

#define for_each_sched_entity(se) \
		for (; se; se = NULL)

static inline struct cfs_rq *task_cfs_rq(struct task_struct *p)
{
	return &task_rq(p)->cfs;
}

static inline struct cfs_rq *cfs_rq_of(struct sched_entity *se)
{
	struct task_struct *p = task_of(se);
	struct rq *rq = task_rq(p);

	return &rq->cfs;
}


static inline struct cfs_rq *group_cfs_rq(struct sched_entity *grp)
{
	return NULL;
}

static inline struct cfs_rq *cpu_cfs_rq(struct cfs_rq *cfs_rq, int this_cpu)
{
	return &cpu_rq(this_cpu)->cfs;
}

#define for_each_leaf_cfs_rq(rq, cfs_rq) \
		for (cfs_rq = &rq->cfs; cfs_rq; cfs_rq = NULL)

static inline int
is_same_group(struct sched_entity *se, struct sched_entity *pse)
{
	return 1;
}

static inline struct sched_entity *parent_entity(struct sched_entity *se)
{
	return NULL;
}

static inline void
find_matching_se(struct sched_entity **se, struct sched_entity **pse)
{
}

#endif	




static inline u64 max_vruntime(u64 min_vruntime, u64 vruntime)
{
	s64 delta = (s64)(vruntime - min_vruntime);
	if (delta > 0)
		min_vruntime = vruntime;

	return min_vruntime;
}

static inline u64 min_vruntime(u64 min_vruntime, u64 vruntime)
{
	s64 delta = (s64)(vruntime - min_vruntime);
	if (delta < 0)
		min_vruntime = vruntime;

	return min_vruntime;
}

static inline int entity_before(struct sched_entity *a,
				struct sched_entity *b)
{
	return (s64)(a->vruntime - b->vruntime) < 0;
}

static inline s64 entity_key(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	return se->vruntime - cfs_rq->min_vruntime;
}

static void update_min_vruntime(struct cfs_rq *cfs_rq)
{
	u64 vruntime = cfs_rq->min_vruntime;

	if (cfs_rq->curr)
		vruntime = cfs_rq->curr->vruntime;

	if (cfs_rq->rb_leftmost) {
		struct sched_entity *se = rb_entry(cfs_rq->rb_leftmost,
						   struct sched_entity,
						   run_node);

		if (!cfs_rq->curr)
			vruntime = se->vruntime;
		else
			vruntime = min_vruntime(vruntime, se->vruntime);
	}

	cfs_rq->min_vruntime = max_vruntime(cfs_rq->min_vruntime, vruntime);
}


static void __enqueue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	struct rb_node **link = &cfs_rq->tasks_timeline.rb_node;
	struct rb_node *parent = NULL;
	struct sched_entity *entry;
	s64 key = entity_key(cfs_rq, se);
	int leftmost = 1;

	
	while (*link) {
		parent = *link;
		entry = rb_entry(parent, struct sched_entity, run_node);
		
		if (key < entity_key(cfs_rq, entry)) {
			link = &parent->rb_left;
		} else {
			link = &parent->rb_right;
			leftmost = 0;
		}
	}

	
	if (leftmost)
		cfs_rq->rb_leftmost = &se->run_node;

	rb_link_node(&se->run_node, parent, link);
	rb_insert_color(&se->run_node, &cfs_rq->tasks_timeline);
}

static void __dequeue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	if (cfs_rq->rb_leftmost == &se->run_node) {
		struct rb_node *next_node;

		next_node = rb_next(&se->run_node);
		cfs_rq->rb_leftmost = next_node;
	}

	rb_erase(&se->run_node, &cfs_rq->tasks_timeline);
}

static struct sched_entity *__pick_next_entity(struct cfs_rq *cfs_rq)
{
	struct rb_node *left = cfs_rq->rb_leftmost;

	if (!left)
		return NULL;

	return rb_entry(left, struct sched_entity, run_node);
}

static struct sched_entity *__pick_last_entity(struct cfs_rq *cfs_rq)
{
	struct rb_node *last = rb_last(&cfs_rq->tasks_timeline);

	if (!last)
		return NULL;

	return rb_entry(last, struct sched_entity, run_node);
}



#ifdef CONFIG_SCHED_DEBUG
int sched_nr_latency_handler(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp,
		loff_t *ppos)
{
	int ret = proc_dointvec_minmax(table, write, buffer, lenp, ppos);

	if (ret || !write)
		return ret;

	sched_nr_latency = DIV_ROUND_UP(sysctl_sched_latency,
					sysctl_sched_min_granularity);

	return 0;
}
#endif


static inline unsigned long
calc_delta_fair(unsigned long delta, struct sched_entity *se)
{
	if (unlikely(se->load.weight != NICE_0_LOAD))
		delta = calc_delta_mine(delta, NICE_0_LOAD, &se->load);

	return delta;
}


static u64 __sched_period(unsigned long nr_running)
{
	u64 period = sysctl_sched_latency;
	unsigned long nr_latency = sched_nr_latency;

	if (unlikely(nr_running > nr_latency)) {
		period = sysctl_sched_min_granularity;
		period *= nr_running;
	}

	return period;
}


static u64 sched_slice(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	u64 slice = __sched_period(cfs_rq->nr_running + !se->on_rq);

	for_each_sched_entity(se) {
		struct load_weight *load;
		struct load_weight lw;

		cfs_rq = cfs_rq_of(se);
		load = &cfs_rq->load;

		if (unlikely(!se->on_rq)) {
			lw = cfs_rq->load;

			update_load_add(&lw, se->load.weight);
			load = &lw;
		}
		slice = calc_delta_mine(slice, se->load.weight, load);
	}
	return slice;
}


static u64 sched_vslice(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	return calc_delta_fair(sched_slice(cfs_rq, se), se);
}


static inline void
__update_curr(struct cfs_rq *cfs_rq, struct sched_entity *curr,
	      unsigned long delta_exec)
{
	unsigned long delta_exec_weighted;

	schedstat_set(curr->exec_max, max((u64)delta_exec, curr->exec_max));

	curr->sum_exec_runtime += delta_exec;
	schedstat_add(cfs_rq, exec_clock, delta_exec);
	delta_exec_weighted = calc_delta_fair(delta_exec, curr);
	curr->vruntime += delta_exec_weighted;
	update_min_vruntime(cfs_rq);
}

static void update_curr(struct cfs_rq *cfs_rq)
{
	struct sched_entity *curr = cfs_rq->curr;
	u64 now = rq_of(cfs_rq)->clock;
	unsigned long delta_exec;

	if (unlikely(!curr))
		return;

	
	delta_exec = (unsigned long)(now - curr->exec_start);
	if (!delta_exec)
		return;

	__update_curr(cfs_rq, curr, delta_exec);
	curr->exec_start = now;

	if (entity_is_task(curr)) {
		struct task_struct *curtask = task_of(curr);

		trace_sched_stat_runtime(curtask, delta_exec, curr->vruntime);
		cpuacct_charge(curtask, delta_exec);
		account_group_exec_runtime(curtask, delta_exec);
	}
}

static inline void
update_stats_wait_start(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	schedstat_set(se->wait_start, rq_of(cfs_rq)->clock);
}


static void update_stats_enqueue(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	
	if (se != cfs_rq->curr)
		update_stats_wait_start(cfs_rq, se);
}

static void
update_stats_wait_end(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	schedstat_set(se->wait_max, max(se->wait_max,
			rq_of(cfs_rq)->clock - se->wait_start));
	schedstat_set(se->wait_count, se->wait_count + 1);
	schedstat_set(se->wait_sum, se->wait_sum +
			rq_of(cfs_rq)->clock - se->wait_start);
#ifdef CONFIG_SCHEDSTATS
	if (entity_is_task(se)) {
		trace_sched_stat_wait(task_of(se),
			rq_of(cfs_rq)->clock - se->wait_start);
	}
#endif
	schedstat_set(se->wait_start, 0);
}

static inline void
update_stats_dequeue(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	
	if (se != cfs_rq->curr)
		update_stats_wait_end(cfs_rq, se);
}


static inline void
update_stats_curr_start(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	
	se->exec_start = rq_of(cfs_rq)->clock;
}



#if defined CONFIG_SMP && defined CONFIG_FAIR_GROUP_SCHED
static void
add_cfs_task_weight(struct cfs_rq *cfs_rq, unsigned long weight)
{
	cfs_rq->task_weight += weight;
}
#else
static inline void
add_cfs_task_weight(struct cfs_rq *cfs_rq, unsigned long weight)
{
}
#endif

static void
account_entity_enqueue(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	update_load_add(&cfs_rq->load, se->load.weight);
	if (!parent_entity(se))
		inc_cpu_load(rq_of(cfs_rq), se->load.weight);
	if (entity_is_task(se)) {
		add_cfs_task_weight(cfs_rq, se->load.weight);
		list_add(&se->group_node, &cfs_rq->tasks);
	}
	cfs_rq->nr_running++;
	se->on_rq = 1;
}

static void
account_entity_dequeue(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	update_load_sub(&cfs_rq->load, se->load.weight);
	if (!parent_entity(se))
		dec_cpu_load(rq_of(cfs_rq), se->load.weight);
	if (entity_is_task(se)) {
		add_cfs_task_weight(cfs_rq, -se->load.weight);
		list_del_init(&se->group_node);
	}
	cfs_rq->nr_running--;
	se->on_rq = 0;
}

static void enqueue_sleeper(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
#ifdef CONFIG_SCHEDSTATS
	struct task_struct *tsk = NULL;

	if (entity_is_task(se))
		tsk = task_of(se);

	if (se->sleep_start) {
		u64 delta = rq_of(cfs_rq)->clock - se->sleep_start;

		if ((s64)delta < 0)
			delta = 0;

		if (unlikely(delta > se->sleep_max))
			se->sleep_max = delta;

		se->sleep_start = 0;
		se->sum_sleep_runtime += delta;

		if (tsk) {
			account_scheduler_latency(tsk, delta >> 10, 1);
			trace_sched_stat_sleep(tsk, delta);
		}
	}
	if (se->block_start) {
		u64 delta = rq_of(cfs_rq)->clock - se->block_start;

		if ((s64)delta < 0)
			delta = 0;

		if (unlikely(delta > se->block_max))
			se->block_max = delta;

		se->block_start = 0;
		se->sum_sleep_runtime += delta;

		if (tsk) {
			if (tsk->in_iowait) {
				se->iowait_sum += delta;
				se->iowait_count++;
				trace_sched_stat_iowait(tsk, delta);
			}

			
			if (unlikely(prof_on == SLEEP_PROFILING)) {
				profile_hits(SLEEP_PROFILING,
						(void *)get_wchan(tsk),
						delta >> 20);
			}
			account_scheduler_latency(tsk, delta >> 10, 0);
		}
	}
#endif
}

static void check_spread(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
#ifdef CONFIG_SCHED_DEBUG
	s64 d = se->vruntime - cfs_rq->min_vruntime;

	if (d < 0)
		d = -d;

	if (d > 3*sysctl_sched_latency)
		schedstat_inc(cfs_rq, nr_spread_over);
#endif
}

static void
place_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int initial)
{
	u64 vruntime = cfs_rq->min_vruntime;

	
	if (initial && sched_feat(START_DEBIT))
		vruntime += sched_vslice(cfs_rq, se);

	
	if (!initial && sched_feat(FAIR_SLEEPERS)) {
		unsigned long thresh = sysctl_sched_latency;

		
		if (sched_feat(NORMALIZED_SLEEPER) && (!entity_is_task(se) ||
				 task_of(se)->policy != SCHED_IDLE))
			thresh = calc_delta_fair(thresh, se);

		
		if (sched_feat(GENTLE_FAIR_SLEEPERS))
			thresh >>= 1;

		vruntime -= thresh;
	}

	
	vruntime = max_vruntime(se->vruntime, vruntime);

	se->vruntime = vruntime;
}

static void
enqueue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int wakeup)
{
	
	update_curr(cfs_rq);
	account_entity_enqueue(cfs_rq, se);

	if (wakeup) {
		place_entity(cfs_rq, se, 0);
		enqueue_sleeper(cfs_rq, se);
	}

	update_stats_enqueue(cfs_rq, se);
	check_spread(cfs_rq, se);
	if (se != cfs_rq->curr)
		__enqueue_entity(cfs_rq, se);
}

static void __clear_buddies(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	if (!se || cfs_rq->last == se)
		cfs_rq->last = NULL;

	if (!se || cfs_rq->next == se)
		cfs_rq->next = NULL;
}

static void clear_buddies(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	for_each_sched_entity(se)
		__clear_buddies(cfs_rq_of(se), se);
}

static void
dequeue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int sleep)
{
	
	update_curr(cfs_rq);

	update_stats_dequeue(cfs_rq, se);
	if (sleep) {
#ifdef CONFIG_SCHEDSTATS
		if (entity_is_task(se)) {
			struct task_struct *tsk = task_of(se);

			if (tsk->state & TASK_INTERRUPTIBLE)
				se->sleep_start = rq_of(cfs_rq)->clock;
			if (tsk->state & TASK_UNINTERRUPTIBLE)
				se->block_start = rq_of(cfs_rq)->clock;
		}
#endif
	}

	clear_buddies(cfs_rq, se);

	if (se != cfs_rq->curr)
		__dequeue_entity(cfs_rq, se);
	account_entity_dequeue(cfs_rq, se);
	update_min_vruntime(cfs_rq);
}


static void
check_preempt_tick(struct cfs_rq *cfs_rq, struct sched_entity *curr)
{
	unsigned long ideal_runtime, delta_exec;

	ideal_runtime = sched_slice(cfs_rq, curr);
	delta_exec = curr->sum_exec_runtime - curr->prev_sum_exec_runtime;
	if (delta_exec > ideal_runtime) {
		resched_task(rq_of(cfs_rq)->curr);
		
		clear_buddies(cfs_rq, curr);
		return;
	}

	
	if (!sched_feat(WAKEUP_PREEMPT))
		return;

	if (delta_exec < sysctl_sched_min_granularity)
		return;

	if (cfs_rq->nr_running > 1) {
		struct sched_entity *se = __pick_next_entity(cfs_rq);
		s64 delta = curr->vruntime - se->vruntime;

		if (delta > ideal_runtime)
			resched_task(rq_of(cfs_rq)->curr);
	}
}

static void
set_next_entity(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
	
	if (se->on_rq) {
		
		update_stats_wait_end(cfs_rq, se);
		__dequeue_entity(cfs_rq, se);
	}

	update_stats_curr_start(cfs_rq, se);
	cfs_rq->curr = se;
#ifdef CONFIG_SCHEDSTATS
	
	if (rq_of(cfs_rq)->load.weight >= 2*se->load.weight) {
		se->slice_max = max(se->slice_max,
			se->sum_exec_runtime - se->prev_sum_exec_runtime);
	}
#endif
	se->prev_sum_exec_runtime = se->sum_exec_runtime;
}

static int
wakeup_preempt_entity(struct sched_entity *curr, struct sched_entity *se);

static struct sched_entity *pick_next_entity(struct cfs_rq *cfs_rq)
{
	struct sched_entity *se = __pick_next_entity(cfs_rq);
	struct sched_entity *left = se;

	if (cfs_rq->next && wakeup_preempt_entity(cfs_rq->next, left) < 1)
		se = cfs_rq->next;

	
	if (cfs_rq->last && wakeup_preempt_entity(cfs_rq->last, left) < 1)
		se = cfs_rq->last;

	clear_buddies(cfs_rq, se);

	return se;
}

static void put_prev_entity(struct cfs_rq *cfs_rq, struct sched_entity *prev)
{
	
	if (prev->on_rq)
		update_curr(cfs_rq);

	check_spread(cfs_rq, prev);
	if (prev->on_rq) {
		update_stats_wait_start(cfs_rq, prev);
		
		__enqueue_entity(cfs_rq, prev);
	}
	cfs_rq->curr = NULL;
}

static void
entity_tick(struct cfs_rq *cfs_rq, struct sched_entity *curr, int queued)
{
	
	update_curr(cfs_rq);

#ifdef CONFIG_SCHED_HRTICK
	
	if (queued) {
		resched_task(rq_of(cfs_rq)->curr);
		return;
	}
	
	if (!sched_feat(DOUBLE_TICK) &&
			hrtimer_active(&rq_of(cfs_rq)->hrtick_timer))
		return;
#endif

	if (cfs_rq->nr_running > 1 || !sched_feat(WAKEUP_PREEMPT))
		check_preempt_tick(cfs_rq, curr);
}



#ifdef CONFIG_SCHED_HRTICK
static void hrtick_start_fair(struct rq *rq, struct task_struct *p)
{
	struct sched_entity *se = &p->se;
	struct cfs_rq *cfs_rq = cfs_rq_of(se);

	WARN_ON(task_rq(p) != rq);

	if (hrtick_enabled(rq) && cfs_rq->nr_running > 1) {
		u64 slice = sched_slice(cfs_rq, se);
		u64 ran = se->sum_exec_runtime - se->prev_sum_exec_runtime;
		s64 delta = slice - ran;

		if (delta < 0) {
			if (rq->curr == p)
				resched_task(p);
			return;
		}

		
		if (rq->curr != p)
			delta = max_t(s64, 10000LL, delta);

		hrtick_start(rq, delta);
	}
}


static void hrtick_update(struct rq *rq)
{
	struct task_struct *curr = rq->curr;

	if (curr->sched_class != &fair_sched_class)
		return;

	if (cfs_rq_of(&curr->se)->nr_running < sched_nr_latency)
		hrtick_start_fair(rq, curr);
}
#else 
static inline void
hrtick_start_fair(struct rq *rq, struct task_struct *p)
{
}

static inline void hrtick_update(struct rq *rq)
{
}
#endif


static void enqueue_task_fair(struct rq *rq, struct task_struct *p, int wakeup)
{
	struct cfs_rq *cfs_rq;
	struct sched_entity *se = &p->se;

	for_each_sched_entity(se) {
		if (se->on_rq)
			break;
		cfs_rq = cfs_rq_of(se);
		enqueue_entity(cfs_rq, se, wakeup);
		wakeup = 1;
	}

	hrtick_update(rq);
}


static void dequeue_task_fair(struct rq *rq, struct task_struct *p, int sleep)
{
	struct cfs_rq *cfs_rq;
	struct sched_entity *se = &p->se;

	for_each_sched_entity(se) {
		cfs_rq = cfs_rq_of(se);
		dequeue_entity(cfs_rq, se, sleep);
		
		if (cfs_rq->load.weight)
			break;
		sleep = 1;
	}

	hrtick_update(rq);
}


static void yield_task_fair(struct rq *rq)
{
	struct task_struct *curr = rq->curr;
	struct cfs_rq *cfs_rq = task_cfs_rq(curr);
	struct sched_entity *rightmost, *se = &curr->se;

	
	if (unlikely(cfs_rq->nr_running == 1))
		return;

	clear_buddies(cfs_rq, se);

	if (likely(!sysctl_sched_compat_yield) && curr->policy != SCHED_BATCH) {
		update_rq_clock(rq);
		
		update_curr(cfs_rq);

		return;
	}
	
	rightmost = __pick_last_entity(cfs_rq);
	
	if (unlikely(!rightmost || entity_before(rightmost, se)))
		return;

	
	se->vruntime = rightmost->vruntime + 1;
}

#ifdef CONFIG_SMP

#ifdef CONFIG_FAIR_GROUP_SCHED

static long effective_load(struct task_group *tg, int cpu,
		long wl, long wg)
{
	struct sched_entity *se = tg->se[cpu];

	if (!tg->parent)
		return wl;

	
	if (!wl && sched_feat(ASYM_EFF_LOAD))
		return wl;

	for_each_sched_entity(se) {
		long S, rw, s, a, b;
		long more_w;

		
		more_w = se->my_q->load.weight - se->my_q->rq_weight;
		wl += more_w;
		wg += more_w;

		S = se->my_q->tg->shares;
		s = se->my_q->shares;
		rw = se->my_q->rq_weight;

		a = S*(rw + wl);
		b = S*rw + s*wg;

		wl = s*(a-b);

		if (likely(b))
			wl /= b;

		
		wg = 0;
	}

	return wl;
}

#else

static inline unsigned long effective_load(struct task_group *tg, int cpu,
		unsigned long wl, unsigned long wg)
{
	return wl;
}

#endif

static int wake_affine(struct sched_domain *sd, struct task_struct *p, int sync)
{
	struct task_struct *curr = current;
	unsigned long this_load, load;
	int idx, this_cpu, prev_cpu;
	unsigned long tl_per_task;
	unsigned int imbalance;
	struct task_group *tg;
	unsigned long weight;
	int balanced;

	idx	  = sd->wake_idx;
	this_cpu  = smp_processor_id();
	prev_cpu  = task_cpu(p);
	load	  = source_load(prev_cpu, idx);
	this_load = target_load(this_cpu, idx);

	if (sync) {
	       if (sched_feat(SYNC_LESS) &&
		   (curr->se.avg_overlap > sysctl_sched_migration_cost ||
		    p->se.avg_overlap > sysctl_sched_migration_cost))
		       sync = 0;
	} else {
		if (sched_feat(SYNC_MORE) &&
		    (curr->se.avg_overlap < sysctl_sched_migration_cost &&
		     p->se.avg_overlap < sysctl_sched_migration_cost))
			sync = 1;
	}

	
	if (sync) {
		tg = task_group(current);
		weight = current->se.load.weight;

		this_load += effective_load(tg, this_cpu, -weight, -weight);
		load += effective_load(tg, prev_cpu, 0, -weight);
	}

	tg = task_group(p);
	weight = p->se.load.weight;

	imbalance = 100 + (sd->imbalance_pct - 100) / 2;

	
	balanced = !this_load ||
		100*(this_load + effective_load(tg, this_cpu, weight, weight)) <=
		imbalance*(load + effective_load(tg, prev_cpu, 0, weight));

	
	if (sync && balanced)
		return 1;

	schedstat_inc(p, se.nr_wakeups_affine_attempts);
	tl_per_task = cpu_avg_load_per_task(this_cpu);

	if (balanced ||
	    (this_load <= load &&
	     this_load + target_load(prev_cpu, idx) <= tl_per_task)) {
		
		schedstat_inc(sd, ttwu_move_affine);
		schedstat_inc(p, se.nr_wakeups_affine);

		return 1;
	}
	return 0;
}


static struct sched_group *
find_idlest_group(struct sched_domain *sd, struct task_struct *p,
		  int this_cpu, int load_idx)
{
	struct sched_group *idlest = NULL, *this = NULL, *group = sd->groups;
	unsigned long min_load = ULONG_MAX, this_load = 0;
	int imbalance = 100 + (sd->imbalance_pct-100)/2;

	do {
		unsigned long load, avg_load;
		int local_group;
		int i;

		
		if (!cpumask_intersects(sched_group_cpus(group),
					&p->cpus_allowed))
			continue;

		local_group = cpumask_test_cpu(this_cpu,
					       sched_group_cpus(group));

		
		avg_load = 0;

		for_each_cpu(i, sched_group_cpus(group)) {
			
			if (local_group)
				load = source_load(i, load_idx);
			else
				load = target_load(i, load_idx);

			avg_load += load;
		}

		
		avg_load = (avg_load * SCHED_LOAD_SCALE) / group->cpu_power;

		if (local_group) {
			this_load = avg_load;
			this = group;
		} else if (avg_load < min_load) {
			min_load = avg_load;
			idlest = group;
		}
	} while (group = group->next, group != sd->groups);

	if (!idlest || 100*this_load < imbalance*min_load)
		return NULL;
	return idlest;
}


static int
find_idlest_cpu(struct sched_group *group, struct task_struct *p, int this_cpu)
{
	unsigned long load, min_load = ULONG_MAX;
	int idlest = -1;
	int i;

	
	for_each_cpu_and(i, sched_group_cpus(group), &p->cpus_allowed) {
		load = weighted_cpuload(i);

		if (load < min_load || (load == min_load && i == this_cpu)) {
			min_load = load;
			idlest = i;
		}
	}

	return idlest;
}


static int select_task_rq_fair(struct task_struct *p, int sd_flag, int wake_flags)
{
	struct sched_domain *tmp, *affine_sd = NULL, *sd = NULL;
	int cpu = smp_processor_id();
	int prev_cpu = task_cpu(p);
	int new_cpu = cpu;
	int want_affine = 0;
	int want_sd = 1;
	int sync = wake_flags & WF_SYNC;

	if (sd_flag & SD_BALANCE_WAKE) {
		if (sched_feat(AFFINE_WAKEUPS) &&
		    cpumask_test_cpu(cpu, &p->cpus_allowed))
			want_affine = 1;
		new_cpu = prev_cpu;
	}

	rcu_read_lock();
	for_each_domain(cpu, tmp) {
		if (!(tmp->flags & SD_LOAD_BALANCE))
			continue;

		
		if (tmp->flags & (SD_POWERSAVINGS_BALANCE|SD_PREFER_LOCAL)) {
			unsigned long power = 0;
			unsigned long nr_running = 0;
			unsigned long capacity;
			int i;

			for_each_cpu(i, sched_domain_span(tmp)) {
				power += power_of(i);
				nr_running += cpu_rq(i)->cfs.nr_running;
			}

			capacity = DIV_ROUND_CLOSEST(power, SCHED_LOAD_SCALE);

			if (tmp->flags & SD_POWERSAVINGS_BALANCE)
				nr_running /= 2;

			if (nr_running < capacity)
				want_sd = 0;
		}

		if (want_affine && (tmp->flags & SD_WAKE_AFFINE)) {
			int candidate = -1, i;

			if (cpumask_test_cpu(prev_cpu, sched_domain_span(tmp)))
				candidate = cpu;

			
			if (tmp->flags & SD_PREFER_SIBLING) {
				if (candidate == cpu) {
					if (!cpu_rq(prev_cpu)->cfs.nr_running)
						candidate = prev_cpu;
				}

				if (candidate == -1 || candidate == cpu) {
					for_each_cpu(i, sched_domain_span(tmp)) {
						if (!cpumask_test_cpu(i, &p->cpus_allowed))
							continue;
						if (!cpu_rq(i)->cfs.nr_running) {
							candidate = i;
							break;
						}
					}
				}
			}

			if (candidate >= 0) {
				affine_sd = tmp;
				want_affine = 0;
				cpu = candidate;
			}
		}

		if (!want_sd && !want_affine)
			break;

		if (!(tmp->flags & sd_flag))
			continue;

		if (want_sd)
			sd = tmp;
	}

	if (sched_feat(LB_SHARES_UPDATE)) {
		
		tmp = sd;
		if (affine_sd && (!tmp ||
				  cpumask_weight(sched_domain_span(affine_sd)) >
				  cpumask_weight(sched_domain_span(sd))))
			tmp = affine_sd;

		if (tmp)
			update_shares(tmp);
	}

	if (affine_sd && wake_affine(affine_sd, p, sync)) {
		new_cpu = cpu;
		goto out;
	}

	while (sd) {
		int load_idx = sd->forkexec_idx;
		struct sched_group *group;
		int weight;

		if (!(sd->flags & sd_flag)) {
			sd = sd->child;
			continue;
		}

		if (sd_flag & SD_BALANCE_WAKE)
			load_idx = sd->wake_idx;

		group = find_idlest_group(sd, p, cpu, load_idx);
		if (!group) {
			sd = sd->child;
			continue;
		}

		new_cpu = find_idlest_cpu(group, p, cpu);
		if (new_cpu == -1 || new_cpu == cpu) {
			
			sd = sd->child;
			continue;
		}

		
		cpu = new_cpu;
		weight = cpumask_weight(sched_domain_span(sd));
		sd = NULL;
		for_each_domain(cpu, tmp) {
			if (weight <= cpumask_weight(sched_domain_span(tmp)))
				break;
			if (tmp->flags & sd_flag)
				sd = tmp;
		}
		
	}

out:
	rcu_read_unlock();
	return new_cpu;
}
#endif 


static unsigned long
adaptive_gran(struct sched_entity *curr, struct sched_entity *se)
{
	u64 this_run = curr->sum_exec_runtime - curr->prev_sum_exec_runtime;
	u64 expected_wakeup = 2*se->avg_wakeup * cfs_rq_of(se)->nr_running;
	u64 gran = 0;

	if (this_run < expected_wakeup)
		gran = expected_wakeup - this_run;

	return min_t(s64, gran, sysctl_sched_wakeup_granularity);
}

static unsigned long
wakeup_gran(struct sched_entity *curr, struct sched_entity *se)
{
	unsigned long gran = sysctl_sched_wakeup_granularity;

	if (cfs_rq_of(curr)->curr && sched_feat(ADAPTIVE_GRAN))
		gran = adaptive_gran(curr, se);

	
	if (sched_feat(ASYM_GRAN)) {
		
		if (unlikely(se->load.weight != NICE_0_LOAD))
			gran = calc_delta_fair(gran, se);
	} else {
		if (unlikely(curr->load.weight != NICE_0_LOAD))
			gran = calc_delta_fair(gran, curr);
	}

	return gran;
}


static int
wakeup_preempt_entity(struct sched_entity *curr, struct sched_entity *se)
{
	s64 gran, vdiff = curr->vruntime - se->vruntime;

	if (vdiff <= 0)
		return -1;

	gran = wakeup_gran(curr, se);
	if (vdiff > gran)
		return 1;

	return 0;
}

static void set_last_buddy(struct sched_entity *se)
{
	if (likely(task_of(se)->policy != SCHED_IDLE)) {
		for_each_sched_entity(se)
			cfs_rq_of(se)->last = se;
	}
}

static void set_next_buddy(struct sched_entity *se)
{
	if (likely(task_of(se)->policy != SCHED_IDLE)) {
		for_each_sched_entity(se)
			cfs_rq_of(se)->next = se;
	}
}


static void check_preempt_wakeup(struct rq *rq, struct task_struct *p, int wake_flags)
{
	struct task_struct *curr = rq->curr;
	struct sched_entity *se = &curr->se, *pse = &p->se;
	struct cfs_rq *cfs_rq = task_cfs_rq(curr);
	int sync = wake_flags & WF_SYNC;
	int scale = cfs_rq->nr_running >= sched_nr_latency;

	update_curr(cfs_rq);

	if (unlikely(rt_prio(p->prio))) {
		resched_task(curr);
		return;
	}

	if (unlikely(p->sched_class != &fair_sched_class))
		return;

	if (unlikely(se == pse))
		return;

	if (sched_feat(NEXT_BUDDY) && scale && !(wake_flags & WF_FORK))
		set_next_buddy(pse);

	
	if (test_tsk_need_resched(curr))
		return;

	
	if (unlikely(p->policy != SCHED_NORMAL))
		return;

	
	if (unlikely(curr->policy == SCHED_IDLE)) {
		resched_task(curr);
		return;
	}

	if ((sched_feat(WAKEUP_SYNC) && sync) ||
	    (sched_feat(WAKEUP_OVERLAP) &&
	     (se->avg_overlap < sysctl_sched_migration_cost &&
	      pse->avg_overlap < sysctl_sched_migration_cost))) {
		resched_task(curr);
		return;
	}

	if (sched_feat(WAKEUP_RUNNING)) {
		if (pse->avg_running < se->avg_running) {
			set_next_buddy(pse);
			resched_task(curr);
			return;
		}
	}

	if (!sched_feat(WAKEUP_PREEMPT))
		return;

	find_matching_se(&se, &pse);

	BUG_ON(!pse);

	if (wakeup_preempt_entity(se, pse) == 1) {
		resched_task(curr);
		
		if (unlikely(!se->on_rq || curr == rq->idle))
			return;
		if (sched_feat(LAST_BUDDY) && scale && entity_is_task(se))
			set_last_buddy(se);
	}
}

static struct task_struct *pick_next_task_fair(struct rq *rq)
{
	struct task_struct *p;
	struct cfs_rq *cfs_rq = &rq->cfs;
	struct sched_entity *se;

	if (unlikely(!cfs_rq->nr_running))
		return NULL;

	do {
		se = pick_next_entity(cfs_rq);
		set_next_entity(cfs_rq, se);
		cfs_rq = group_cfs_rq(se);
	} while (cfs_rq);

	p = task_of(se);
	hrtick_start_fair(rq, p);

	return p;
}


static void put_prev_task_fair(struct rq *rq, struct task_struct *prev)
{
	struct sched_entity *se = &prev->se;
	struct cfs_rq *cfs_rq;

	for_each_sched_entity(se) {
		cfs_rq = cfs_rq_of(se);
		put_prev_entity(cfs_rq, se);
	}
}

#ifdef CONFIG_SMP



static struct task_struct *
__load_balance_iterator(struct cfs_rq *cfs_rq, struct list_head *next)
{
	struct task_struct *p = NULL;
	struct sched_entity *se;

	if (next == &cfs_rq->tasks)
		return NULL;

	se = list_entry(next, struct sched_entity, group_node);
	p = task_of(se);
	cfs_rq->balance_iterator = next->next;

	return p;
}

static struct task_struct *load_balance_start_fair(void *arg)
{
	struct cfs_rq *cfs_rq = arg;

	return __load_balance_iterator(cfs_rq, cfs_rq->tasks.next);
}

static struct task_struct *load_balance_next_fair(void *arg)
{
	struct cfs_rq *cfs_rq = arg;

	return __load_balance_iterator(cfs_rq, cfs_rq->balance_iterator);
}

static unsigned long
__load_balance_fair(struct rq *this_rq, int this_cpu, struct rq *busiest,
		unsigned long max_load_move, struct sched_domain *sd,
		enum cpu_idle_type idle, int *all_pinned, int *this_best_prio,
		struct cfs_rq *cfs_rq)
{
	struct rq_iterator cfs_rq_iterator;

	cfs_rq_iterator.start = load_balance_start_fair;
	cfs_rq_iterator.next = load_balance_next_fair;
	cfs_rq_iterator.arg = cfs_rq;

	return balance_tasks(this_rq, this_cpu, busiest,
			max_load_move, sd, idle, all_pinned,
			this_best_prio, &cfs_rq_iterator);
}

#ifdef CONFIG_FAIR_GROUP_SCHED
static unsigned long
load_balance_fair(struct rq *this_rq, int this_cpu, struct rq *busiest,
		  unsigned long max_load_move,
		  struct sched_domain *sd, enum cpu_idle_type idle,
		  int *all_pinned, int *this_best_prio)
{
	long rem_load_move = max_load_move;
	int busiest_cpu = cpu_of(busiest);
	struct task_group *tg;

	rcu_read_lock();
	update_h_load(busiest_cpu);

	list_for_each_entry_rcu(tg, &task_groups, list) {
		struct cfs_rq *busiest_cfs_rq = tg->cfs_rq[busiest_cpu];
		unsigned long busiest_h_load = busiest_cfs_rq->h_load;
		unsigned long busiest_weight = busiest_cfs_rq->load.weight;
		u64 rem_load, moved_load;

		
		if (!busiest_cfs_rq->task_weight)
			continue;

		rem_load = (u64)rem_load_move * busiest_weight;
		rem_load = div_u64(rem_load, busiest_h_load + 1);

		moved_load = __load_balance_fair(this_rq, this_cpu, busiest,
				rem_load, sd, idle, all_pinned, this_best_prio,
				tg->cfs_rq[busiest_cpu]);

		if (!moved_load)
			continue;

		moved_load *= busiest_h_load;
		moved_load = div_u64(moved_load, busiest_weight + 1);

		rem_load_move -= moved_load;
		if (rem_load_move < 0)
			break;
	}
	rcu_read_unlock();

	return max_load_move - rem_load_move;
}
#else
static unsigned long
load_balance_fair(struct rq *this_rq, int this_cpu, struct rq *busiest,
		  unsigned long max_load_move,
		  struct sched_domain *sd, enum cpu_idle_type idle,
		  int *all_pinned, int *this_best_prio)
{
	return __load_balance_fair(this_rq, this_cpu, busiest,
			max_load_move, sd, idle, all_pinned,
			this_best_prio, &busiest->cfs);
}
#endif

static int
move_one_task_fair(struct rq *this_rq, int this_cpu, struct rq *busiest,
		   struct sched_domain *sd, enum cpu_idle_type idle)
{
	struct cfs_rq *busy_cfs_rq;
	struct rq_iterator cfs_rq_iterator;

	cfs_rq_iterator.start = load_balance_start_fair;
	cfs_rq_iterator.next = load_balance_next_fair;

	for_each_leaf_cfs_rq(busiest, busy_cfs_rq) {
		
		cfs_rq_iterator.arg = busy_cfs_rq;
		if (iter_move_one_task(this_rq, this_cpu, busiest, sd, idle,
				       &cfs_rq_iterator))
		    return 1;
	}

	return 0;
}

static void rq_online_fair(struct rq *rq)
{
	update_sysctl();
}

static void rq_offline_fair(struct rq *rq)
{
	update_sysctl();
}

#endif 


static void task_tick_fair(struct rq *rq, struct task_struct *curr, int queued)
{
	struct cfs_rq *cfs_rq;
	struct sched_entity *se = &curr->se;

	for_each_sched_entity(se) {
		cfs_rq = cfs_rq_of(se);
		entity_tick(cfs_rq, se, queued);
	}
}


static void task_new_fair(struct rq *rq, struct task_struct *p)
{
	struct cfs_rq *cfs_rq = task_cfs_rq(p);
	struct sched_entity *se = &p->se, *curr = cfs_rq->curr;
	int this_cpu = smp_processor_id();

	sched_info_queued(p);

	update_curr(cfs_rq);
	if (curr)
		se->vruntime = curr->vruntime;
	place_entity(cfs_rq, se, 1);

	
	if (sysctl_sched_child_runs_first && this_cpu == task_cpu(p) &&
			curr && entity_before(curr, se)) {
		
		swap(curr->vruntime, se->vruntime);
		resched_task(rq->curr);
	}

	enqueue_task_fair(rq, p, 0);
}


static void prio_changed_fair(struct rq *rq, struct task_struct *p,
			      int oldprio, int running)
{
	
	if (running) {
		if (p->prio > oldprio)
			resched_task(rq->curr);
	} else
		check_preempt_curr(rq, p, 0);
}


static void switched_to_fair(struct rq *rq, struct task_struct *p,
			     int running)
{
	
	if (running)
		resched_task(rq->curr);
	else
		check_preempt_curr(rq, p, 0);
}


static void set_curr_task_fair(struct rq *rq)
{
	struct sched_entity *se = &rq->curr->se;

	for_each_sched_entity(se)
		set_next_entity(cfs_rq_of(se), se);
}

#ifdef CONFIG_FAIR_GROUP_SCHED
static void moved_group_fair(struct task_struct *p)
{
	struct cfs_rq *cfs_rq = task_cfs_rq(p);

	update_curr(cfs_rq);
	place_entity(cfs_rq, &p->se, 1);
}
#endif

unsigned int get_rr_interval_fair(struct task_struct *task)
{
	struct sched_entity *se = &task->se;
	unsigned long flags;
	struct rq *rq;
	unsigned int rr_interval = 0;

	
	rq = task_rq_lock(task, &flags);
	if (rq->cfs.load.weight)
		rr_interval = NS_TO_JIFFIES(sched_slice(&rq->cfs, se));
	task_rq_unlock(rq, &flags);

	return rr_interval;
}


static const struct sched_class fair_sched_class = {
	.next			= &idle_sched_class,
	.enqueue_task		= enqueue_task_fair,
	.dequeue_task		= dequeue_task_fair,
	.yield_task		= yield_task_fair,

	.check_preempt_curr	= check_preempt_wakeup,

	.pick_next_task		= pick_next_task_fair,
	.put_prev_task		= put_prev_task_fair,

#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_fair,

	.load_balance		= load_balance_fair,
	.move_one_task		= move_one_task_fair,
	.rq_online		= rq_online_fair,
	.rq_offline		= rq_offline_fair,
#endif

	.set_curr_task          = set_curr_task_fair,
	.task_tick		= task_tick_fair,
	.task_new		= task_new_fair,

	.prio_changed		= prio_changed_fair,
	.switched_to		= switched_to_fair,

	.get_rr_interval	= get_rr_interval_fair,

#ifdef CONFIG_FAIR_GROUP_SCHED
	.moved_group		= moved_group_fair,
#endif
};

#ifdef CONFIG_SCHED_DEBUG
static void print_cfs_stats(struct seq_file *m, int cpu)
{
	struct cfs_rq *cfs_rq;

	rcu_read_lock();
	for_each_leaf_cfs_rq(cpu_rq(cpu), cfs_rq)
		print_cfs_rq(m, cpu, cfs_rq);
	rcu_read_unlock();
}
#endif
