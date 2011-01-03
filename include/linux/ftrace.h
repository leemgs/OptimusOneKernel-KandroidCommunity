#ifndef _LINUX_FTRACE_H
#define _LINUX_FTRACE_H

#include <linux/trace_clock.h>
#include <linux/kallsyms.h>
#include <linux/linkage.h>
#include <linux/bitops.h>
#include <linux/module.h>
#include <linux/ktime.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/fs.h>

#include <asm/ftrace.h>

#ifdef CONFIG_FUNCTION_TRACER

extern int ftrace_enabled;
extern int
ftrace_enable_sysctl(struct ctl_table *table, int write,
		     void __user *buffer, size_t *lenp,
		     loff_t *ppos);

typedef void (*ftrace_func_t)(unsigned long ip, unsigned long parent_ip);

struct ftrace_ops {
	ftrace_func_t	  func;
	struct ftrace_ops *next;
};

extern int function_trace_stop;


enum ftrace_tracing_type_t {
	FTRACE_TYPE_ENTER = 0, 
	FTRACE_TYPE_RETURN,	
};


extern enum ftrace_tracing_type_t ftrace_tracing_type;


static inline void ftrace_stop(void)
{
	function_trace_stop = 1;
}


static inline void ftrace_start(void)
{
	function_trace_stop = 0;
}


int register_ftrace_function(struct ftrace_ops *ops);
int unregister_ftrace_function(struct ftrace_ops *ops);
void clear_ftrace_function(void);

extern void ftrace_stub(unsigned long a0, unsigned long a1);

#else 
# define register_ftrace_function(ops) do { } while (0)
# define unregister_ftrace_function(ops) do { } while (0)
# define clear_ftrace_function(ops) do { } while (0)
static inline void ftrace_kill(void) { }
static inline void ftrace_stop(void) { }
static inline void ftrace_start(void) { }
#endif 

#ifdef CONFIG_STACK_TRACER
extern int stack_tracer_enabled;
int
stack_trace_sysctl(struct ctl_table *table, int write,
		   void __user *buffer, size_t *lenp,
		   loff_t *ppos);
#endif

struct ftrace_func_command {
	struct list_head	list;
	char			*name;
	int			(*func)(char *func, char *cmd,
					char *params, int enable);
};

#ifdef CONFIG_DYNAMIC_FTRACE

int ftrace_arch_code_modify_prepare(void);
int ftrace_arch_code_modify_post_process(void);

struct seq_file;

struct ftrace_probe_ops {
	void			(*func)(unsigned long ip,
					unsigned long parent_ip,
					void **data);
	int			(*callback)(unsigned long ip, void **data);
	void			(*free)(void **data);
	int			(*print)(struct seq_file *m,
					 unsigned long ip,
					 struct ftrace_probe_ops *ops,
					 void *data);
};

extern int
register_ftrace_function_probe(char *glob, struct ftrace_probe_ops *ops,
			      void *data);
extern void
unregister_ftrace_function_probe(char *glob, struct ftrace_probe_ops *ops,
				void *data);
extern void
unregister_ftrace_function_probe_func(char *glob, struct ftrace_probe_ops *ops);
extern void unregister_ftrace_function_probe_all(char *glob);

enum {
	FTRACE_FL_FREE		= (1 << 0),
	FTRACE_FL_FAILED	= (1 << 1),
	FTRACE_FL_FILTER	= (1 << 2),
	FTRACE_FL_ENABLED	= (1 << 3),
	FTRACE_FL_NOTRACE	= (1 << 4),
	FTRACE_FL_CONVERTED	= (1 << 5),
	FTRACE_FL_FROZEN	= (1 << 6),
};

struct dyn_ftrace {
	union {
		unsigned long		ip; 
		struct dyn_ftrace	*freelist;
	};
	union {
		unsigned long		flags;
		struct dyn_ftrace	*newlist;
	};
	struct dyn_arch_ftrace		arch;
};

int ftrace_force_update(void);
void ftrace_set_filter(unsigned char *buf, int len, int reset);

int register_ftrace_command(struct ftrace_func_command *cmd);
int unregister_ftrace_command(struct ftrace_func_command *cmd);


extern int ftrace_ip_converted(unsigned long ip);
extern int ftrace_dyn_arch_init(void *data);
extern int ftrace_update_ftrace_func(ftrace_func_t func);
extern void ftrace_caller(void);
extern void ftrace_call(void);
extern void mcount_call(void);

#ifndef FTRACE_ADDR
#define FTRACE_ADDR ((unsigned long)ftrace_caller)
#endif
#ifdef CONFIG_FUNCTION_GRAPH_TRACER
extern void ftrace_graph_caller(void);
extern int ftrace_enable_ftrace_graph_caller(void);
extern int ftrace_disable_ftrace_graph_caller(void);
#else
static inline int ftrace_enable_ftrace_graph_caller(void) { return 0; }
static inline int ftrace_disable_ftrace_graph_caller(void) { return 0; }
#endif


extern int ftrace_make_nop(struct module *mod,
			   struct dyn_ftrace *rec, unsigned long addr);


extern int ftrace_make_call(struct dyn_ftrace *rec, unsigned long addr);


extern int ftrace_arch_read_dyn_info(char *buf, int size);

extern int skip_trace(unsigned long ip);

extern void ftrace_disable_daemon(void);
extern void ftrace_enable_daemon(void);
#else
# define skip_trace(ip)				({ 0; })
# define ftrace_force_update()			({ 0; })
# define ftrace_set_filter(buf, len, reset)	do { } while (0)
# define ftrace_disable_daemon()		do { } while (0)
# define ftrace_enable_daemon()			do { } while (0)
static inline void ftrace_release_mod(struct module *mod) {}
static inline int register_ftrace_command(struct ftrace_func_command *cmd)
{
	return -EINVAL;
}
static inline int unregister_ftrace_command(char *cmd_name)
{
	return -EINVAL;
}
#endif 


void ftrace_kill(void);

static inline void tracer_disable(void)
{
#ifdef CONFIG_FUNCTION_TRACER
	ftrace_enabled = 0;
#endif
}


static inline int __ftrace_enabled_save(void)
{
#ifdef CONFIG_FUNCTION_TRACER
	int saved_ftrace_enabled = ftrace_enabled;
	ftrace_enabled = 0;
	return saved_ftrace_enabled;
#else
	return 0;
#endif
}

static inline void __ftrace_enabled_restore(int enabled)
{
#ifdef CONFIG_FUNCTION_TRACER
	ftrace_enabled = enabled;
#endif
}

#ifndef HAVE_ARCH_CALLER_ADDR
# ifdef CONFIG_FRAME_POINTER
#  define CALLER_ADDR0 ((unsigned long)__builtin_return_address(0))
#  define CALLER_ADDR1 ((unsigned long)__builtin_return_address(1))
#  define CALLER_ADDR2 ((unsigned long)__builtin_return_address(2))
#  define CALLER_ADDR3 ((unsigned long)__builtin_return_address(3))
#  define CALLER_ADDR4 ((unsigned long)__builtin_return_address(4))
#  define CALLER_ADDR5 ((unsigned long)__builtin_return_address(5))
#  define CALLER_ADDR6 ((unsigned long)__builtin_return_address(6))
# else
#  define CALLER_ADDR0 ((unsigned long)__builtin_return_address(0))
#  define CALLER_ADDR1 0UL
#  define CALLER_ADDR2 0UL
#  define CALLER_ADDR3 0UL
#  define CALLER_ADDR4 0UL
#  define CALLER_ADDR5 0UL
#  define CALLER_ADDR6 0UL
# endif
#endif 

#ifdef CONFIG_IRQSOFF_TRACER
  extern void time_hardirqs_on(unsigned long a0, unsigned long a1);
  extern void time_hardirqs_off(unsigned long a0, unsigned long a1);
#else
# define time_hardirqs_on(a0, a1)		do { } while (0)
# define time_hardirqs_off(a0, a1)		do { } while (0)
#endif

#ifdef CONFIG_PREEMPT_TRACER
  extern void trace_preempt_on(unsigned long a0, unsigned long a1);
  extern void trace_preempt_off(unsigned long a0, unsigned long a1);
#else
# define trace_preempt_on(a0, a1)		do { } while (0)
# define trace_preempt_off(a0, a1)		do { } while (0)
#endif

#ifdef CONFIG_FTRACE_MCOUNT_RECORD
extern void ftrace_init(void);
#else
static inline void ftrace_init(void) { }
#endif


struct ftrace_graph_ent {
	unsigned long func; 
	int depth;
};


struct ftrace_graph_ret {
	unsigned long func; 
	unsigned long long calltime;
	unsigned long long rettime;
	
	unsigned long overrun;
	int depth;
};

#ifdef CONFIG_FUNCTION_GRAPH_TRACER


#define INIT_FTRACE_GRAPH		.ret_stack = NULL,


struct ftrace_ret_stack {
	unsigned long ret;
	unsigned long func;
	unsigned long long calltime;
	unsigned long long subtime;
	unsigned long fp;
};


extern void return_to_handler(void);

extern int
ftrace_push_return_trace(unsigned long ret, unsigned long func, int *depth,
			 unsigned long frame_pointer);


#define __notrace_funcgraph		notrace


#define __irq_entry		 __attribute__((__section__(".irqentry.text")))


extern char __irqentry_text_start[];
extern char __irqentry_text_end[];

#define FTRACE_RETFUNC_DEPTH 50
#define FTRACE_RETSTACK_ALLOC_SIZE 32

typedef void (*trace_func_graph_ret_t)(struct ftrace_graph_ret *); 
typedef int (*trace_func_graph_ent_t)(struct ftrace_graph_ent *); 

extern int register_ftrace_graph(trace_func_graph_ret_t retfunc,
				trace_func_graph_ent_t entryfunc);

extern void ftrace_graph_stop(void);


extern trace_func_graph_ret_t ftrace_graph_return;
extern trace_func_graph_ent_t ftrace_graph_entry;

extern void unregister_ftrace_graph(void);

extern void ftrace_graph_init_task(struct task_struct *t);
extern void ftrace_graph_exit_task(struct task_struct *t);

static inline int task_curr_ret_stack(struct task_struct *t)
{
	return t->curr_ret_stack;
}

static inline void pause_graph_tracing(void)
{
	atomic_inc(&current->tracing_graph_pause);
}

static inline void unpause_graph_tracing(void)
{
	atomic_dec(&current->tracing_graph_pause);
}
#else 

#define __notrace_funcgraph
#define __irq_entry
#define INIT_FTRACE_GRAPH

static inline void ftrace_graph_init_task(struct task_struct *t) { }
static inline void ftrace_graph_exit_task(struct task_struct *t) { }

static inline int task_curr_ret_stack(struct task_struct *tsk)
{
	return -1;
}

static inline void pause_graph_tracing(void) { }
static inline void unpause_graph_tracing(void) { }
#endif 

#ifdef CONFIG_TRACING


enum {
	TSK_TRACE_FL_TRACE_BIT	= 0,
	TSK_TRACE_FL_GRAPH_BIT	= 1,
};
enum {
	TSK_TRACE_FL_TRACE	= 1 << TSK_TRACE_FL_TRACE_BIT,
	TSK_TRACE_FL_GRAPH	= 1 << TSK_TRACE_FL_GRAPH_BIT,
};

static inline void set_tsk_trace_trace(struct task_struct *tsk)
{
	set_bit(TSK_TRACE_FL_TRACE_BIT, &tsk->trace);
}

static inline void clear_tsk_trace_trace(struct task_struct *tsk)
{
	clear_bit(TSK_TRACE_FL_TRACE_BIT, &tsk->trace);
}

static inline int test_tsk_trace_trace(struct task_struct *tsk)
{
	return tsk->trace & TSK_TRACE_FL_TRACE;
}

static inline void set_tsk_trace_graph(struct task_struct *tsk)
{
	set_bit(TSK_TRACE_FL_GRAPH_BIT, &tsk->trace);
}

static inline void clear_tsk_trace_graph(struct task_struct *tsk)
{
	clear_bit(TSK_TRACE_FL_GRAPH_BIT, &tsk->trace);
}

static inline int test_tsk_trace_graph(struct task_struct *tsk)
{
	return tsk->trace & TSK_TRACE_FL_GRAPH;
}

extern int ftrace_dump_on_oops;

#ifdef CONFIG_PREEMPT
#define INIT_TRACE_RECURSION		.trace_recursion = 0,
#endif

#endif 

#ifndef INIT_TRACE_RECURSION
#define INIT_TRACE_RECURSION
#endif

#ifdef CONFIG_HW_BRANCH_TRACER

void trace_hw_branch(u64 from, u64 to);
void trace_hw_branch_oops(void);

#else 

static inline void trace_hw_branch(u64 from, u64 to) {}
static inline void trace_hw_branch_oops(void) {}

#endif 

#endif 
