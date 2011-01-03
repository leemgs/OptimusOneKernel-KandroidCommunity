#ifndef _LINUX_CGROUP_H
#define _LINUX_CGROUP_H


#include <linux/sched.h>
#include <linux/cpumask.h>
#include <linux/nodemask.h>
#include <linux/rcupdate.h>
#include <linux/cgroupstats.h>
#include <linux/prio_heap.h>
#include <linux/rwsem.h>
#include <linux/idr.h>

#ifdef CONFIG_CGROUPS

struct cgroupfs_root;
struct cgroup_subsys;
struct inode;
struct cgroup;
struct css_id;

extern int cgroup_init_early(void);
extern int cgroup_init(void);
extern void cgroup_lock(void);
extern bool cgroup_lock_live_group(struct cgroup *cgrp);
extern void cgroup_unlock(void);
extern void cgroup_fork(struct task_struct *p);
extern void cgroup_fork_callbacks(struct task_struct *p);
extern void cgroup_post_fork(struct task_struct *p);
extern void cgroup_exit(struct task_struct *p, int run_callbacks);
extern int cgroupstats_build(struct cgroupstats *stats,
				struct dentry *dentry);

extern const struct file_operations proc_cgroup_operations;


#define SUBSYS(_x) _x ## _subsys_id,
enum cgroup_subsys_id {
#include <linux/cgroup_subsys.h>
	CGROUP_SUBSYS_COUNT
};
#undef SUBSYS


struct cgroup_subsys_state {
	
	struct cgroup *cgroup;

	

	atomic_t refcnt;

	unsigned long flags;
	
	struct css_id *id;
};


enum {
	CSS_ROOT, 
	CSS_REMOVED, 
};



static inline void css_get(struct cgroup_subsys_state *css)
{
	
	if (!test_bit(CSS_ROOT, &css->flags))
		atomic_inc(&css->refcnt);
}

static inline bool css_is_removed(struct cgroup_subsys_state *css)
{
	return test_bit(CSS_REMOVED, &css->flags);
}



static inline bool css_tryget(struct cgroup_subsys_state *css)
{
	if (test_bit(CSS_ROOT, &css->flags))
		return true;
	while (!atomic_inc_not_zero(&css->refcnt)) {
		if (test_bit(CSS_REMOVED, &css->flags))
			return false;
		cpu_relax();
	}
	return true;
}



extern void __css_put(struct cgroup_subsys_state *css);
static inline void css_put(struct cgroup_subsys_state *css)
{
	if (!test_bit(CSS_ROOT, &css->flags))
		__css_put(css);
}


enum {
	
	CGRP_REMOVED,
	
	CGRP_RELEASABLE,
	
	CGRP_NOTIFY_ON_RELEASE,
	
	CGRP_WAIT_ON_RMDIR,
};


enum cgroup_filetype {
	CGROUP_FILE_PROCS,
	CGROUP_FILE_TASKS,
};


struct cgroup_pidlist {
	
	struct { enum cgroup_filetype type; struct pid_namespace *ns; } key;
	
	pid_t *list;
	
	int length;
	
	int use_count;
	
	struct list_head links;
	
	struct cgroup *owner;
	
	struct rw_semaphore mutex;
};

struct cgroup {
	unsigned long flags;		

	
	atomic_t count;

	
	struct list_head sibling;	
	struct list_head children;	

	struct cgroup *parent;		
	struct dentry *dentry;	  	

	
	struct cgroup_subsys_state *subsys[CGROUP_SUBSYS_COUNT];

	struct cgroupfs_root *root;
	struct cgroup *top_cgroup;

	
	struct list_head css_sets;

	
	struct list_head release_list;

	
	struct list_head pidlists;
	struct mutex pidlist_mutex;

	
	struct rcu_head rcu_head;
};



struct css_set {

	
	atomic_t refcount;

	
	struct hlist_node hlist;

	
	struct list_head tasks;

	
	struct list_head cg_links;

	
	struct cgroup_subsys_state *subsys[CGROUP_SUBSYS_COUNT];

	
	struct rcu_head rcu_head;
};



struct cgroup_map_cb {
	int (*fill)(struct cgroup_map_cb *cb, const char *key, u64 value);
	void *state;
};



#define MAX_CFTYPE_NAME 64
struct cftype {
	
	char name[MAX_CFTYPE_NAME];
	int private;
	
	mode_t mode;

	
	size_t max_write_len;

	int (*open)(struct inode *inode, struct file *file);
	ssize_t (*read)(struct cgroup *cgrp, struct cftype *cft,
			struct file *file,
			char __user *buf, size_t nbytes, loff_t *ppos);
	
	u64 (*read_u64)(struct cgroup *cgrp, struct cftype *cft);
	
	s64 (*read_s64)(struct cgroup *cgrp, struct cftype *cft);
	
	int (*read_map)(struct cgroup *cont, struct cftype *cft,
			struct cgroup_map_cb *cb);
	
	int (*read_seq_string)(struct cgroup *cont, struct cftype *cft,
			       struct seq_file *m);

	ssize_t (*write)(struct cgroup *cgrp, struct cftype *cft,
			 struct file *file,
			 const char __user *buf, size_t nbytes, loff_t *ppos);

	
	int (*write_u64)(struct cgroup *cgrp, struct cftype *cft, u64 val);
	
	int (*write_s64)(struct cgroup *cgrp, struct cftype *cft, s64 val);

	
	int (*write_string)(struct cgroup *cgrp, struct cftype *cft,
			    const char *buffer);
	
	int (*trigger)(struct cgroup *cgrp, unsigned int event);

	int (*release)(struct inode *inode, struct file *file);
};

struct cgroup_scanner {
	struct cgroup *cg;
	int (*test_task)(struct task_struct *p, struct cgroup_scanner *scan);
	void (*process_task)(struct task_struct *p,
			struct cgroup_scanner *scan);
	struct ptr_heap *heap;
	void *data;
};


int cgroup_add_file(struct cgroup *cgrp, struct cgroup_subsys *subsys,
		       const struct cftype *cft);


int cgroup_add_files(struct cgroup *cgrp,
			struct cgroup_subsys *subsys,
			const struct cftype cft[],
			int count);

int cgroup_is_removed(const struct cgroup *cgrp);

int cgroup_path(const struct cgroup *cgrp, char *buf, int buflen);

int cgroup_task_count(const struct cgroup *cgrp);


int cgroup_is_descendant(const struct cgroup *cgrp, struct task_struct *task);



void cgroup_exclude_rmdir(struct cgroup_subsys_state *css);
void cgroup_release_and_wakeup_rmdir(struct cgroup_subsys_state *css);



struct cgroup_subsys {
	struct cgroup_subsys_state *(*create)(struct cgroup_subsys *ss,
						  struct cgroup *cgrp);
	int (*pre_destroy)(struct cgroup_subsys *ss, struct cgroup *cgrp);
	void (*destroy)(struct cgroup_subsys *ss, struct cgroup *cgrp);
	int (*can_attach)(struct cgroup_subsys *ss, struct cgroup *cgrp,
			  struct task_struct *tsk, bool threadgroup);
	void (*attach)(struct cgroup_subsys *ss, struct cgroup *cgrp,
			struct cgroup *old_cgrp, struct task_struct *tsk,
			bool threadgroup);
	void (*fork)(struct cgroup_subsys *ss, struct task_struct *task);
	void (*exit)(struct cgroup_subsys *ss, struct task_struct *task);
	int (*populate)(struct cgroup_subsys *ss,
			struct cgroup *cgrp);
	void (*post_clone)(struct cgroup_subsys *ss, struct cgroup *cgrp);
	void (*bind)(struct cgroup_subsys *ss, struct cgroup *root);

	int subsys_id;
	int active;
	int disabled;
	int early_init;
	
	bool use_id;
#define MAX_CGROUP_TYPE_NAMELEN 32
	const char *name;

	
	struct mutex hierarchy_mutex;
	struct lock_class_key subsys_key;

	
	struct cgroupfs_root *root;
	struct list_head sibling;
	
	struct idr idr;
	spinlock_t id_lock;
};

#define SUBSYS(_x) extern struct cgroup_subsys _x ## _subsys;
#include <linux/cgroup_subsys.h>
#undef SUBSYS

static inline struct cgroup_subsys_state *cgroup_subsys_state(
	struct cgroup *cgrp, int subsys_id)
{
	return cgrp->subsys[subsys_id];
}

static inline struct cgroup_subsys_state *task_subsys_state(
	struct task_struct *task, int subsys_id)
{
	return rcu_dereference(task->cgroups->subsys[subsys_id]);
}

static inline struct cgroup* task_cgroup(struct task_struct *task,
					       int subsys_id)
{
	return task_subsys_state(task, subsys_id)->cgroup;
}

int cgroup_clone(struct task_struct *tsk, struct cgroup_subsys *ss,
							char *nodename);


struct cgroup_iter {
	struct list_head *cg_link;
	struct list_head *task;
};


void cgroup_iter_start(struct cgroup *cgrp, struct cgroup_iter *it);
struct task_struct *cgroup_iter_next(struct cgroup *cgrp,
					struct cgroup_iter *it);
void cgroup_iter_end(struct cgroup *cgrp, struct cgroup_iter *it);
int cgroup_scan_tasks(struct cgroup_scanner *scan);
int cgroup_attach_task(struct cgroup *, struct task_struct *);




void free_css_id(struct cgroup_subsys *ss, struct cgroup_subsys_state *css);



struct cgroup_subsys_state *css_lookup(struct cgroup_subsys *ss, int id);


struct cgroup_subsys_state *css_get_next(struct cgroup_subsys *ss, int id,
		struct cgroup_subsys_state *root, int *foundid);


bool css_is_ancestor(struct cgroup_subsys_state *cg,
		     const struct cgroup_subsys_state *root);


unsigned short css_id(struct cgroup_subsys_state *css);
unsigned short css_depth(struct cgroup_subsys_state *css);

#else 

static inline int cgroup_init_early(void) { return 0; }
static inline int cgroup_init(void) { return 0; }
static inline void cgroup_fork(struct task_struct *p) {}
static inline void cgroup_fork_callbacks(struct task_struct *p) {}
static inline void cgroup_post_fork(struct task_struct *p) {}
static inline void cgroup_exit(struct task_struct *p, int callbacks) {}

static inline void cgroup_lock(void) {}
static inline void cgroup_unlock(void) {}
static inline int cgroupstats_build(struct cgroupstats *stats,
					struct dentry *dentry)
{
	return -EINVAL;
}

#endif 

#endif 
