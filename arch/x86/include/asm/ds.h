

#ifndef _ASM_X86_DS_H
#define _ASM_X86_DS_H


#include <linux/types.h>
#include <linux/init.h>
#include <linux/err.h>


#ifdef CONFIG_X86_DS

struct task_struct;
struct ds_context;
struct ds_tracer;
struct bts_tracer;
struct pebs_tracer;

typedef void (*bts_ovfl_callback_t)(struct bts_tracer *);
typedef void (*pebs_ovfl_callback_t)(struct pebs_tracer *);



enum ds_feature {
	dsf_bts = 0,
	dsf_bts_kernel,
#define BTS_KERNEL (1 << dsf_bts_kernel)
	

	dsf_bts_user,
#define BTS_USER (1 << dsf_bts_user)
	

	dsf_bts_overflow,
	dsf_bts_max,
	dsf_pebs = dsf_bts_max,

	dsf_pebs_max,
	dsf_ctl_max = dsf_pebs_max,
	dsf_bts_timestamps = dsf_ctl_max,
#define BTS_TIMESTAMPS (1 << dsf_bts_timestamps)
	

#define BTS_USER_FLAGS (BTS_KERNEL | BTS_USER | BTS_TIMESTAMPS)
};



extern struct bts_tracer *ds_request_bts_task(struct task_struct *task,
					      void *base, size_t size,
					      bts_ovfl_callback_t ovfl,
					      size_t th, unsigned int flags);
extern struct bts_tracer *ds_request_bts_cpu(int cpu, void *base, size_t size,
					     bts_ovfl_callback_t ovfl,
					     size_t th, unsigned int flags);
extern struct pebs_tracer *ds_request_pebs_task(struct task_struct *task,
						void *base, size_t size,
						pebs_ovfl_callback_t ovfl,
						size_t th, unsigned int flags);
extern struct pebs_tracer *ds_request_pebs_cpu(int cpu,
					       void *base, size_t size,
					       pebs_ovfl_callback_t ovfl,
					       size_t th, unsigned int flags);


extern void ds_release_bts(struct bts_tracer *tracer);
extern void ds_suspend_bts(struct bts_tracer *tracer);
extern void ds_resume_bts(struct bts_tracer *tracer);
extern void ds_release_pebs(struct pebs_tracer *tracer);
extern void ds_suspend_pebs(struct pebs_tracer *tracer);
extern void ds_resume_pebs(struct pebs_tracer *tracer);


extern int ds_release_bts_noirq(struct bts_tracer *tracer);
extern int ds_suspend_bts_noirq(struct bts_tracer *tracer);
extern int ds_resume_bts_noirq(struct bts_tracer *tracer);
extern int ds_release_pebs_noirq(struct pebs_tracer *tracer);
extern int ds_suspend_pebs_noirq(struct pebs_tracer *tracer);
extern int ds_resume_pebs_noirq(struct pebs_tracer *tracer);



struct ds_trace {
	
	size_t n;
	
	size_t size;
	
	void *begin;
	
	void *end;
	
	void *top;
	
	void *ith;
	
	unsigned int flags;
};


enum bts_qualifier {
	bts_invalid,
#define BTS_INVALID bts_invalid

	bts_branch,
#define BTS_BRANCH bts_branch

	bts_task_arrives,
#define BTS_TASK_ARRIVES bts_task_arrives

	bts_task_departs,
#define BTS_TASK_DEPARTS bts_task_departs

	bts_qual_bit_size = 4,
	bts_qual_max = (1 << bts_qual_bit_size),
};

struct bts_struct {
	__u64 qualifier;
	union {
		
		struct {
			__u64 from;
			__u64 to;
		} lbr;
		
		struct {
			__u64 clock;
			pid_t pid;
		} event;
	} variant;
};



struct bts_trace {
	struct ds_trace ds;

	int (*read)(struct bts_tracer *tracer, const void *at,
		    struct bts_struct *out);
	int (*write)(struct bts_tracer *tracer, const struct bts_struct *in);
};



struct pebs_trace {
	struct ds_trace ds;

	
	unsigned int counters;

#define MAX_PEBS_COUNTERS 4
	
	unsigned long long counter_reset[MAX_PEBS_COUNTERS];
};



extern const struct bts_trace *ds_read_bts(struct bts_tracer *tracer);
extern const struct pebs_trace *ds_read_pebs(struct pebs_tracer *tracer);



extern int ds_reset_bts(struct bts_tracer *tracer);
extern int ds_reset_pebs(struct pebs_tracer *tracer);


extern int ds_set_pebs_reset(struct pebs_tracer *tracer,
			     unsigned int counter, u64 value);


struct cpuinfo_x86;
extern void __cpuinit ds_init_intel(struct cpuinfo_x86 *);


extern void ds_switch_to(struct task_struct *prev, struct task_struct *next);

#else 

struct cpuinfo_x86;
static inline void __cpuinit ds_init_intel(struct cpuinfo_x86 *ignored) {}
static inline void ds_switch_to(struct task_struct *prev,
				struct task_struct *next) {}

#endif 
#endif 
