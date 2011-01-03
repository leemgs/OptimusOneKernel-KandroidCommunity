#ifndef IOCONTEXT_H
#define IOCONTEXT_H

#include <linux/radix-tree.h>
#include <linux/rcupdate.h>


struct as_io_context {
	spinlock_t lock;

	void (*dtor)(struct as_io_context *aic); 
	void (*exit)(struct as_io_context *aic); 

	unsigned long state;
	atomic_t nr_queued; 
	atomic_t nr_dispatched; 

	
	
	unsigned long last_end_request;
	unsigned long ttime_total;
	unsigned long ttime_samples;
	unsigned long ttime_mean;
	
	unsigned int seek_samples;
	sector_t last_request_pos;
	u64 seek_total;
	sector_t seek_mean;
};

struct cfq_queue;
struct cfq_io_context {
	void *key;
	unsigned long dead_key;

	struct cfq_queue *cfqq[2];

	struct io_context *ioc;

	unsigned long last_end_request;
	sector_t last_request_pos;

	unsigned long ttime_total;
	unsigned long ttime_samples;
	unsigned long ttime_mean;

	unsigned int seek_samples;
	u64 seek_total;
	sector_t seek_mean;

	struct list_head queue_list;
	struct hlist_node cic_list;

	void (*dtor)(struct io_context *); 
	void (*exit)(struct io_context *); 

	struct rcu_head rcu_head;
};


struct io_context {
	atomic_long_t refcount;
	atomic_t nr_tasks;

	
	spinlock_t lock;

	unsigned short ioprio;
	unsigned short ioprio_changed;

	
	unsigned long last_waited; 
	int nr_batch_requests;     

	struct as_io_context *aic;
	struct radix_tree_root radix_root;
	struct hlist_head cic_list;
	void *ioc_data;
};

static inline struct io_context *ioc_task_link(struct io_context *ioc)
{
	
	if (ioc && atomic_long_inc_not_zero(&ioc->refcount)) {
		atomic_inc(&ioc->nr_tasks);
		return ioc;
	}

	return NULL;
}

#ifdef CONFIG_BLOCK
int put_io_context(struct io_context *ioc);
void exit_io_context(void);
struct io_context *get_io_context(gfp_t gfp_flags, int node);
struct io_context *alloc_io_context(gfp_t gfp_flags, int node);
void copy_io_context(struct io_context **pdst, struct io_context **psrc);
#else
static inline void exit_io_context(void)
{
}

struct io_context;
static inline int put_io_context(struct io_context *ioc)
{
	return 1;
}
#endif

#endif
