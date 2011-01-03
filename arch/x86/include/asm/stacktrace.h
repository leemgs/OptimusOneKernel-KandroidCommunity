#ifndef _ASM_X86_STACKTRACE_H
#define _ASM_X86_STACKTRACE_H

extern int kstack_depth_to_print;

int x86_is_stack_id(int id, char *name);



struct stacktrace_ops {
	void (*warning)(void *data, char *msg);
	
	void (*warning_symbol)(void *data, char *msg, unsigned long symbol);
	void (*address)(void *data, unsigned long address, int reliable);
	
	int (*stack)(void *data, char *name);
};

void dump_trace(struct task_struct *tsk, struct pt_regs *regs,
		unsigned long *stack, unsigned long bp,
		const struct stacktrace_ops *ops, void *data);

#endif 
