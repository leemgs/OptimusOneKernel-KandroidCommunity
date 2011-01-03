

#define SLOW_WORK_CULL_TIMEOUT (5 * HZ)	
#define SLOW_WORK_OOM_TIMEOUT (5 * HZ)	

#define SLOW_WORK_THREAD_LIMIT	255	


#ifdef CONFIG_SLOW_WORK_DEBUG
extern struct slow_work *slow_work_execs[];
extern pid_t slow_work_pids[];
extern rwlock_t slow_work_execs_lock;
#endif

extern struct list_head slow_work_queue;
extern struct list_head vslow_work_queue;
extern spinlock_t slow_work_queue_lock;


#ifdef CONFIG_SLOW_WORK_DEBUG
extern const struct file_operations slow_work_runqueue_fops;

extern void slow_work_new_thread_desc(struct slow_work *, struct seq_file *);
#endif


static inline void slow_work_set_thread_pid(int id, pid_t pid)
{
#ifdef CONFIG_SLOW_WORK_PROC
	slow_work_pids[id] = pid;
#endif
}

static inline void slow_work_mark_time(struct slow_work *work)
{
#ifdef CONFIG_SLOW_WORK_PROC
	work->mark = CURRENT_TIME;
#endif
}

static inline void slow_work_begin_exec(int id, struct slow_work *work)
{
#ifdef CONFIG_SLOW_WORK_PROC
	slow_work_execs[id] = work;
#endif
}

static inline void slow_work_end_exec(int id, struct slow_work *work)
{
#ifdef CONFIG_SLOW_WORK_PROC
	write_lock(&slow_work_execs_lock);
	slow_work_execs[id] = NULL;
	write_unlock(&slow_work_execs_lock);
#endif
}
