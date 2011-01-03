
#include <linux/kallsyms.h>
#include <linux/kprobes.h>
#include <linux/uaccess.h>
#include <linux/hardirq.h>
#include <linux/kdebug.h>
#include <linux/module.h>
#include <linux/ptrace.h>
#include <linux/kexec.h>
#include <linux/bug.h>
#include <linux/nmi.h>
#include <linux/sysfs.h>

#include <asm/stacktrace.h>

#include "dumpstack.h"


static char x86_stack_ids[][8] = {
		[DEBUG_STACK - 1] = "#DB",
		[NMI_STACK - 1] = "NMI",
		[DOUBLEFAULT_STACK - 1] = "#DF",
		[STACKFAULT_STACK - 1] = "#SS",
		[MCE_STACK - 1] = "#MC",
#if DEBUG_STKSZ > EXCEPTION_STKSZ
		[N_EXCEPTION_STACKS ...
			N_EXCEPTION_STACKS + DEBUG_STKSZ / EXCEPTION_STKSZ - 2] = "#DB[?]"
#endif
	};

int x86_is_stack_id(int id, char *name)
{
	return x86_stack_ids[id - 1] == name;
}

static unsigned long *in_exception_stack(unsigned cpu, unsigned long stack,
					unsigned *usedp, char **idp)
{
	unsigned k;

	
	for (k = 0; k < N_EXCEPTION_STACKS; k++) {
		unsigned long end = per_cpu(orig_ist, cpu).ist[k];
		
		if (stack >= end)
			continue;
		
		if (stack >= end - EXCEPTION_STKSZ) {
			
			if (*usedp & (1U << k))
				break;
			*usedp |= 1U << k;
			*idp = x86_stack_ids[k];
			return (unsigned long *)end;
		}
		
#if DEBUG_STKSZ > EXCEPTION_STKSZ
		if (k == DEBUG_STACK - 1 && stack >= end - DEBUG_STKSZ) {
			unsigned j = N_EXCEPTION_STACKS - 1;

			
			do {
				++j;
				end -= EXCEPTION_STKSZ;
				x86_stack_ids[j][4] = '1' +
						(j - N_EXCEPTION_STACKS);
			} while (stack < end - EXCEPTION_STKSZ);
			if (*usedp & (1U << j))
				break;
			*usedp |= 1U << j;
			*idp = x86_stack_ids[j];
			return (unsigned long *)end;
		}
#endif
	}
	return NULL;
}



void dump_trace(struct task_struct *task, struct pt_regs *regs,
		unsigned long *stack, unsigned long bp,
		const struct stacktrace_ops *ops, void *data)
{
	const unsigned cpu = get_cpu();
	unsigned long *irq_stack_end =
		(unsigned long *)per_cpu(irq_stack_ptr, cpu);
	unsigned used = 0;
	struct thread_info *tinfo;
	int graph = 0;

	if (!task)
		task = current;

	if (!stack) {
		unsigned long dummy;
		stack = &dummy;
		if (task && task != current)
			stack = (unsigned long *)task->thread.sp;
	}

#ifdef CONFIG_FRAME_POINTER
	if (!bp) {
		if (task == current) {
			
			get_bp(bp);
		} else {
			
			bp = *(unsigned long *) task->thread.sp;
		}
	}
#endif

	
	tinfo = task_thread_info(task);
	for (;;) {
		char *id;
		unsigned long *estack_end;
		estack_end = in_exception_stack(cpu, (unsigned long)stack,
						&used, &id);

		if (estack_end) {
			if (ops->stack(data, id) < 0)
				break;

			bp = print_context_stack(tinfo, stack, bp, ops,
						 data, estack_end, &graph);
			ops->stack(data, "<EOE>");
			
			stack = (unsigned long *) estack_end[-2];
			continue;
		}
		if (irq_stack_end) {
			unsigned long *irq_stack;
			irq_stack = irq_stack_end -
				(IRQ_STACK_SIZE - 64) / sizeof(*irq_stack);

			if (stack >= irq_stack && stack < irq_stack_end) {
				if (ops->stack(data, "IRQ") < 0)
					break;
				bp = print_context_stack(tinfo, stack, bp,
					ops, data, irq_stack_end, &graph);
				
				stack = (unsigned long *) (irq_stack_end[-1]);
				irq_stack_end = NULL;
				ops->stack(data, "EOI");
				continue;
			}
		}
		break;
	}

	
	bp = print_context_stack(tinfo, stack, bp, ops, data, NULL, &graph);
	put_cpu();
}
EXPORT_SYMBOL(dump_trace);

void
show_stack_log_lvl(struct task_struct *task, struct pt_regs *regs,
		unsigned long *sp, unsigned long bp, char *log_lvl)
{
	unsigned long *stack;
	int i;
	const int cpu = smp_processor_id();
	unsigned long *irq_stack_end =
		(unsigned long *)(per_cpu(irq_stack_ptr, cpu));
	unsigned long *irq_stack =
		(unsigned long *)(per_cpu(irq_stack_ptr, cpu) - IRQ_STACK_SIZE);

	

	if (sp == NULL) {
		if (task)
			sp = (unsigned long *)task->thread.sp;
		else
			sp = (unsigned long *)&sp;
	}

	stack = sp;
	for (i = 0; i < kstack_depth_to_print; i++) {
		if (stack >= irq_stack && stack <= irq_stack_end) {
			if (stack == irq_stack_end) {
				stack = (unsigned long *) (irq_stack_end[-1]);
				printk(" <EOI> ");
			}
		} else {
		if (((long) stack & (THREAD_SIZE-1)) == 0)
			break;
		}
		if (i && ((i % STACKSLOTS_PER_LINE) == 0))
			printk("\n%s", log_lvl);
		printk(" %016lx", *stack++);
		touch_nmi_watchdog();
	}
	printk("\n");
	show_trace_log_lvl(task, regs, sp, bp, log_lvl);
}

void show_registers(struct pt_regs *regs)
{
	int i;
	unsigned long sp;
	const int cpu = smp_processor_id();
	struct task_struct *cur = current;

	sp = regs->sp;
	printk("CPU %d ", cpu);
	__show_regs(regs, 1);
	printk("Process %s (pid: %d, threadinfo %p, task %p)\n",
		cur->comm, cur->pid, task_thread_info(cur), cur);

	
	if (!user_mode(regs)) {
		unsigned int code_prologue = code_bytes * 43 / 64;
		unsigned int code_len = code_bytes;
		unsigned char c;
		u8 *ip;

		printk(KERN_EMERG "Stack:\n");
		show_stack_log_lvl(NULL, regs, (unsigned long *)sp,
				regs->bp, KERN_EMERG);

		printk(KERN_EMERG "Code: ");

		ip = (u8 *)regs->ip - code_prologue;
		if (ip < (u8 *)PAGE_OFFSET || probe_kernel_address(ip, c)) {
			
			ip = (u8 *)regs->ip;
			code_len = code_len - code_prologue + 1;
		}
		for (i = 0; i < code_len; i++, ip++) {
			if (ip < (u8 *)PAGE_OFFSET ||
					probe_kernel_address(ip, c)) {
				printk(" Bad RIP value.");
				break;
			}
			if (ip == (u8 *)regs->ip)
				printk("<%02x> ", c);
			else
				printk("%02x ", c);
		}
	}
	printk("\n");
}

int is_valid_bugaddr(unsigned long ip)
{
	unsigned short ud2;

	if (__copy_from_user(&ud2, (const void __user *) ip, sizeof(ud2)))
		return 0;

	return ud2 == 0x0b0f;
}

