
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/stacktrace.h>

void print_stack_trace(struct stack_trace *trace, int spaces)
{
	int i;

	if (WARN_ON(!trace->entries))
		return;

	for (i = 0; i < trace->nr_entries; i++) {
		printk("%*c", 1 + spaces, ' ');
		print_ip_sym(trace->entries[i]);
	}
}
EXPORT_SYMBOL_GPL(print_stack_trace);


__weak void
save_stack_trace_tsk(struct task_struct *tsk, struct stack_trace *trace)
{
	WARN_ONCE(1, KERN_INFO "save_stack_trace_tsk() not implemented yet.\n");
}
