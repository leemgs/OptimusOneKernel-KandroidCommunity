

#ifndef _LINUX_SLOW_WORK_H
#define _LINUX_SLOW_WORK_H

#ifdef CONFIG_SLOW_WORK

#include <linux/sysctl.h>
#include <linux/timer.h>

struct slow_work;
#ifdef CONFIG_SLOW_WORK_DEBUG
struct seq_file;
#endif


struct slow_work_ops {
	
	struct module *owner;

	
	int (*get_ref)(struct slow_work *work);

	
	void (*put_ref)(struct slow_work *work);

	
	void (*execute)(struct slow_work *work);

#ifdef CONFIG_SLOW_WORK_DEBUG
	
	void (*desc)(struct slow_work *work, struct seq_file *m);
#endif
};


struct slow_work {
	struct module		*owner;	
	unsigned long		flags;
#define SLOW_WORK_PENDING	0	
#define SLOW_WORK_EXECUTING	1	
#define SLOW_WORK_ENQ_DEFERRED	2	
#define SLOW_WORK_VERY_SLOW	3	
#define SLOW_WORK_CANCELLING	4	
#define SLOW_WORK_DELAYED	5	
	const struct slow_work_ops *ops; 
	struct list_head	link;	
#ifdef CONFIG_SLOW_WORK_DEBUG
	struct timespec		mark;	
#endif
};

struct delayed_slow_work {
	struct slow_work	work;
	struct timer_list	timer;
};


static inline void slow_work_init(struct slow_work *work,
				  const struct slow_work_ops *ops)
{
	work->flags = 0;
	work->ops = ops;
	INIT_LIST_HEAD(&work->link);
}


static inline void delayed_slow_work_init(struct delayed_slow_work *dwork,
					  const struct slow_work_ops *ops)
{
	init_timer(&dwork->timer);
	slow_work_init(&dwork->work, ops);
}


static inline void vslow_work_init(struct slow_work *work,
				   const struct slow_work_ops *ops)
{
	work->flags = 1 << SLOW_WORK_VERY_SLOW;
	work->ops = ops;
	INIT_LIST_HEAD(&work->link);
}


static inline bool slow_work_is_queued(struct slow_work *work)
{
	unsigned long flags = work->flags;
	return flags & SLOW_WORK_PENDING && !(flags & SLOW_WORK_EXECUTING);
}

extern int slow_work_enqueue(struct slow_work *work);
extern void slow_work_cancel(struct slow_work *work);
extern int slow_work_register_user(struct module *owner);
extern void slow_work_unregister_user(struct module *owner);

extern int delayed_slow_work_enqueue(struct delayed_slow_work *dwork,
				     unsigned long delay);

static inline void delayed_slow_work_cancel(struct delayed_slow_work *dwork)
{
	slow_work_cancel(&dwork->work);
}

extern bool slow_work_sleep_till_thread_needed(struct slow_work *work,
					       signed long *_timeout);

#ifdef CONFIG_SYSCTL
extern ctl_table slow_work_sysctls[];
#endif

#endif 
#endif 
