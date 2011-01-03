



#include <linux/stackprotector.h>
#include <linux/cpu.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/elfcore.h>
#include <linux/smp.h>
#include <linux/slab.h>
#include <linux/user.h>
#include <linux/interrupt.h>
#include <linux/utsname.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/ptrace.h>
#include <linux/notifier.h>
#include <linux/kprobes.h>
#include <linux/kdebug.h>
#include <linux/tick.h>
#include <linux/prctl.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/ftrace.h>
#include <linux/dmi.h>

#include <asm/pgtable.h>
#include <asm/system.h>
#include <asm/processor.h>
#include <asm/i387.h>
#include <asm/mmu_context.h>
#include <asm/prctl.h>
#include <asm/desc.h>
#include <asm/proto.h>
#include <asm/ia32.h>
#include <asm/idle.h>
#include <asm/syscalls.h>
#include <asm/ds.h>

asmlinkage extern void ret_from_fork(void);

DEFINE_PER_CPU(unsigned long, old_rsp);
static DEFINE_PER_CPU(unsigned char, is_idle);

unsigned long kernel_thread_flags = CLONE_VM | CLONE_UNTRACED;

static ATOMIC_NOTIFIER_HEAD(idle_notifier);

void idle_notifier_register(struct notifier_block *n)
{
	atomic_notifier_chain_register(&idle_notifier, n);
}
EXPORT_SYMBOL_GPL(idle_notifier_register);

void idle_notifier_unregister(struct notifier_block *n)
{
	atomic_notifier_chain_unregister(&idle_notifier, n);
}
EXPORT_SYMBOL_GPL(idle_notifier_unregister);

void enter_idle(void)
{
	percpu_write(is_idle, 1);
	atomic_notifier_call_chain(&idle_notifier, IDLE_START, NULL);
}

static void __exit_idle(void)
{
	if (x86_test_and_clear_bit_percpu(0, is_idle) == 0)
		return;
	atomic_notifier_call_chain(&idle_notifier, IDLE_END, NULL);
}


void exit_idle(void)
{
	
	if (current->pid)
		return;
	__exit_idle();
}

#ifndef CONFIG_SMP
static inline void play_dead(void)
{
	BUG();
}
#endif


void cpu_idle(void)
{
	current_thread_info()->status |= TS_POLLING;

	
	boot_init_stack_canary();

	
	while (1) {
		tick_nohz_stop_sched_tick(1);
		while (!need_resched()) {

			rmb();

			if (cpu_is_offline(smp_processor_id()))
				play_dead();
			
			local_irq_disable();
			enter_idle();
			
			stop_critical_timings();
			pm_idle();
			start_critical_timings();
			
			__exit_idle();
		}

		tick_nohz_restart_sched_tick();
		preempt_enable_no_resched();
		schedule();
		preempt_disable();
	}
}


void __show_regs(struct pt_regs *regs, int all)
{
	unsigned long cr0 = 0L, cr2 = 0L, cr3 = 0L, cr4 = 0L, fs, gs, shadowgs;
	unsigned long d0, d1, d2, d3, d6, d7;
	unsigned int fsindex, gsindex;
	unsigned int ds, cs, es;
	const char *board;

	printk("\n");
	print_modules();
	board = dmi_get_system_info(DMI_PRODUCT_NAME);
	if (!board)
		board = "";
	printk(KERN_INFO "Pid: %d, comm: %.20s %s %s %.*s %s\n",
		current->pid, current->comm, print_tainted(),
		init_utsname()->release,
		(int)strcspn(init_utsname()->version, " "),
		init_utsname()->version, board);
	printk(KERN_INFO "RIP: %04lx:[<%016lx>] ", regs->cs & 0xffff, regs->ip);
	printk_address(regs->ip, 1);
	printk(KERN_INFO "RSP: %04lx:%016lx  EFLAGS: %08lx\n", regs->ss,
			regs->sp, regs->flags);
	printk(KERN_INFO "RAX: %016lx RBX: %016lx RCX: %016lx\n",
	       regs->ax, regs->bx, regs->cx);
	printk(KERN_INFO "RDX: %016lx RSI: %016lx RDI: %016lx\n",
	       regs->dx, regs->si, regs->di);
	printk(KERN_INFO "RBP: %016lx R08: %016lx R09: %016lx\n",
	       regs->bp, regs->r8, regs->r9);
	printk(KERN_INFO "R10: %016lx R11: %016lx R12: %016lx\n",
	       regs->r10, regs->r11, regs->r12);
	printk(KERN_INFO "R13: %016lx R14: %016lx R15: %016lx\n",
	       regs->r13, regs->r14, regs->r15);

	asm("movl %%ds,%0" : "=r" (ds));
	asm("movl %%cs,%0" : "=r" (cs));
	asm("movl %%es,%0" : "=r" (es));
	asm("movl %%fs,%0" : "=r" (fsindex));
	asm("movl %%gs,%0" : "=r" (gsindex));

	rdmsrl(MSR_FS_BASE, fs);
	rdmsrl(MSR_GS_BASE, gs);
	rdmsrl(MSR_KERNEL_GS_BASE, shadowgs);

	if (!all)
		return;

	cr0 = read_cr0();
	cr2 = read_cr2();
	cr3 = read_cr3();
	cr4 = read_cr4();

	printk(KERN_INFO "FS:  %016lx(%04x) GS:%016lx(%04x) knlGS:%016lx\n",
	       fs, fsindex, gs, gsindex, shadowgs);
	printk(KERN_INFO "CS:  %04x DS: %04x ES: %04x CR0: %016lx\n", cs, ds,
			es, cr0);
	printk(KERN_INFO "CR2: %016lx CR3: %016lx CR4: %016lx\n", cr2, cr3,
			cr4);

	get_debugreg(d0, 0);
	get_debugreg(d1, 1);
	get_debugreg(d2, 2);
	printk(KERN_INFO "DR0: %016lx DR1: %016lx DR2: %016lx\n", d0, d1, d2);
	get_debugreg(d3, 3);
	get_debugreg(d6, 6);
	get_debugreg(d7, 7);
	printk(KERN_INFO "DR3: %016lx DR6: %016lx DR7: %016lx\n", d3, d6, d7);
}

void show_regs(struct pt_regs *regs)
{
	printk(KERN_INFO "CPU %d:", smp_processor_id());
	__show_regs(regs, 1);
	show_trace(NULL, regs, (void *)(regs + 1), regs->bp);
}

void release_thread(struct task_struct *dead_task)
{
	if (dead_task->mm) {
		if (dead_task->mm->context.size) {
			printk("WARNING: dead process %8s still has LDT? <%p/%d>\n",
					dead_task->comm,
					dead_task->mm->context.ldt,
					dead_task->mm->context.size);
			BUG();
		}
	}
}

static inline void set_32bit_tls(struct task_struct *t, int tls, u32 addr)
{
	struct user_desc ud = {
		.base_addr = addr,
		.limit = 0xfffff,
		.seg_32bit = 1,
		.limit_in_pages = 1,
		.useable = 1,
	};
	struct desc_struct *desc = t->thread.tls_array;
	desc += tls;
	fill_ldt(desc, &ud);
}

static inline u32 read_32bit_tls(struct task_struct *t, int tls)
{
	return get_desc_base(&t->thread.tls_array[tls]);
}


void prepare_to_copy(struct task_struct *tsk)
{
	unlazy_fpu(tsk);
}

int copy_thread(unsigned long clone_flags, unsigned long sp,
		unsigned long unused,
	struct task_struct *p, struct pt_regs *regs)
{
	int err;
	struct pt_regs *childregs;
	struct task_struct *me = current;

	childregs = ((struct pt_regs *)
			(THREAD_SIZE + task_stack_page(p))) - 1;
	*childregs = *regs;

	childregs->ax = 0;
	childregs->sp = sp;
	if (sp == ~0UL)
		childregs->sp = (unsigned long)childregs;

	p->thread.sp = (unsigned long) childregs;
	p->thread.sp0 = (unsigned long) (childregs+1);
	p->thread.usersp = me->thread.usersp;

	set_tsk_thread_flag(p, TIF_FORK);

	p->thread.fs = me->thread.fs;
	p->thread.gs = me->thread.gs;

	savesegment(gs, p->thread.gsindex);
	savesegment(fs, p->thread.fsindex);
	savesegment(es, p->thread.es);
	savesegment(ds, p->thread.ds);

	if (unlikely(test_tsk_thread_flag(me, TIF_IO_BITMAP))) {
		p->thread.io_bitmap_ptr = kmalloc(IO_BITMAP_BYTES, GFP_KERNEL);
		if (!p->thread.io_bitmap_ptr) {
			p->thread.io_bitmap_max = 0;
			return -ENOMEM;
		}
		memcpy(p->thread.io_bitmap_ptr, me->thread.io_bitmap_ptr,
				IO_BITMAP_BYTES);
		set_tsk_thread_flag(p, TIF_IO_BITMAP);
	}

	
	if (clone_flags & CLONE_SETTLS) {
#ifdef CONFIG_IA32_EMULATION
		if (test_thread_flag(TIF_IA32))
			err = do_set_thread_area(p, -1,
				(struct user_desc __user *)childregs->si, 0);
		else
#endif
			err = do_arch_prctl(p, ARCH_SET_FS, childregs->r8);
		if (err)
			goto out;
	}

	clear_tsk_thread_flag(p, TIF_DS_AREA_MSR);
	p->thread.ds_ctx = NULL;

	clear_tsk_thread_flag(p, TIF_DEBUGCTLMSR);
	p->thread.debugctlmsr = 0;

	err = 0;
out:
	if (err && p->thread.io_bitmap_ptr) {
		kfree(p->thread.io_bitmap_ptr);
		p->thread.io_bitmap_max = 0;
	}
	return err;
}

void
start_thread(struct pt_regs *regs, unsigned long new_ip, unsigned long new_sp)
{
	loadsegment(fs, 0);
	loadsegment(es, 0);
	loadsegment(ds, 0);
	load_gs_index(0);
	regs->ip		= new_ip;
	regs->sp		= new_sp;
	percpu_write(old_rsp, new_sp);
	regs->cs		= __USER_CS;
	regs->ss		= __USER_DS;
	regs->flags		= 0x200;
	set_fs(USER_DS);
	
	free_thread_xstate(current);
}
EXPORT_SYMBOL_GPL(start_thread);


__notrace_funcgraph struct task_struct *
__switch_to(struct task_struct *prev_p, struct task_struct *next_p)
{
	struct thread_struct *prev = &prev_p->thread;
	struct thread_struct *next = &next_p->thread;
	int cpu = smp_processor_id();
	struct tss_struct *tss = &per_cpu(init_tss, cpu);
	unsigned fsindex, gsindex;
	bool preload_fpu;

	
	preload_fpu = tsk_used_math(next_p) && next_p->fpu_counter > 5;

	
	if (preload_fpu)
		prefetch(next->xstate);

	
	load_sp0(tss, next);

	
	savesegment(es, prev->es);
	if (unlikely(next->es | prev->es))
		loadsegment(es, next->es);

	savesegment(ds, prev->ds);
	if (unlikely(next->ds | prev->ds))
		loadsegment(ds, next->ds);


	
	savesegment(fs, fsindex);
	savesegment(gs, gsindex);

	load_TLS(next, cpu);

	
	unlazy_fpu(prev_p);

	
	if (preload_fpu)
		clts();

	
	arch_end_context_switch(next_p);

	
	if (unlikely(fsindex | next->fsindex | prev->fs)) {
		loadsegment(fs, next->fsindex);
		
		if (fsindex)
			prev->fs = 0;
	}
	
	if (next->fs)
		wrmsrl(MSR_FS_BASE, next->fs);
	prev->fsindex = fsindex;

	if (unlikely(gsindex | next->gsindex | prev->gs)) {
		load_gs_index(next->gsindex);
		if (gsindex)
			prev->gs = 0;
	}
	if (next->gs)
		wrmsrl(MSR_KERNEL_GS_BASE, next->gs);
	prev->gsindex = gsindex;

	
	prev->usersp = percpu_read(old_rsp);
	percpu_write(old_rsp, next->usersp);
	percpu_write(current_task, next_p);

	percpu_write(kernel_stack,
		  (unsigned long)task_stack_page(next_p) +
		  THREAD_SIZE - KERNEL_STACK_OFFSET);

	
	if (unlikely(task_thread_info(next_p)->flags & _TIF_WORK_CTXSW_NEXT ||
		     task_thread_info(prev_p)->flags & _TIF_WORK_CTXSW_PREV))
		__switch_to_xtra(prev_p, next_p, tss);

	
	if (preload_fpu)
		__math_state_restore();
	return prev_p;
}


asmlinkage
long sys_execve(char __user *name, char __user * __user *argv,
		char __user * __user *envp, struct pt_regs *regs)
{
	long error;
	char *filename;

	filename = getname(name);
	error = PTR_ERR(filename);
	if (IS_ERR(filename))
		return error;
	error = do_execve(filename, argv, envp, regs);
	putname(filename);
	return error;
}

void set_personality_64bit(void)
{
	

	
	clear_thread_flag(TIF_IA32);

	
	current->personality &= ~READ_IMPLIES_EXEC;
}

asmlinkage long
sys_clone(unsigned long clone_flags, unsigned long newsp,
	  void __user *parent_tid, void __user *child_tid, struct pt_regs *regs)
{
	if (!newsp)
		newsp = regs->sp;
	return do_fork(clone_flags, newsp, regs, 0, parent_tid, child_tid);
}

void set_personality_ia32(void)
{
	

	
	set_thread_flag(TIF_IA32);

	
	current_thread_info()->status |= TS_COMPAT;
}

unsigned long get_wchan(struct task_struct *p)
{
	unsigned long stack;
	u64 fp, ip;
	int count = 0;

	if (!p || p == current || p->state == TASK_RUNNING)
		return 0;
	stack = (unsigned long)task_stack_page(p);
	if (p->thread.sp < stack || p->thread.sp >= stack+THREAD_SIZE)
		return 0;
	fp = *(u64 *)(p->thread.sp);
	do {
		if (fp < (unsigned long)stack ||
		    fp >= (unsigned long)stack+THREAD_SIZE)
			return 0;
		ip = *(u64 *)(fp+8);
		if (!in_sched_functions(ip))
			return ip;
		fp = *(u64 *)fp;
	} while (count++ < 16);
	return 0;
}

long do_arch_prctl(struct task_struct *task, int code, unsigned long addr)
{
	int ret = 0;
	int doit = task == current;
	int cpu;

	switch (code) {
	case ARCH_SET_GS:
		if (addr >= TASK_SIZE_OF(task))
			return -EPERM;
		cpu = get_cpu();
		
		if (addr <= 0xffffffff) {
			set_32bit_tls(task, GS_TLS, addr);
			if (doit) {
				load_TLS(&task->thread, cpu);
				load_gs_index(GS_TLS_SEL);
			}
			task->thread.gsindex = GS_TLS_SEL;
			task->thread.gs = 0;
		} else {
			task->thread.gsindex = 0;
			task->thread.gs = addr;
			if (doit) {
				load_gs_index(0);
				ret = checking_wrmsrl(MSR_KERNEL_GS_BASE, addr);
			}
		}
		put_cpu();
		break;
	case ARCH_SET_FS:
		
		if (addr >= TASK_SIZE_OF(task))
			return -EPERM;
		cpu = get_cpu();
		
		if (addr <= 0xffffffff) {
			set_32bit_tls(task, FS_TLS, addr);
			if (doit) {
				load_TLS(&task->thread, cpu);
				loadsegment(fs, FS_TLS_SEL);
			}
			task->thread.fsindex = FS_TLS_SEL;
			task->thread.fs = 0;
		} else {
			task->thread.fsindex = 0;
			task->thread.fs = addr;
			if (doit) {
				
				loadsegment(fs, 0);
				ret = checking_wrmsrl(MSR_FS_BASE, addr);
			}
		}
		put_cpu();
		break;
	case ARCH_GET_FS: {
		unsigned long base;
		if (task->thread.fsindex == FS_TLS_SEL)
			base = read_32bit_tls(task, FS_TLS);
		else if (doit)
			rdmsrl(MSR_FS_BASE, base);
		else
			base = task->thread.fs;
		ret = put_user(base, (unsigned long __user *)addr);
		break;
	}
	case ARCH_GET_GS: {
		unsigned long base;
		unsigned gsindex;
		if (task->thread.gsindex == GS_TLS_SEL)
			base = read_32bit_tls(task, GS_TLS);
		else if (doit) {
			savesegment(gs, gsindex);
			if (gsindex)
				rdmsrl(MSR_KERNEL_GS_BASE, base);
			else
				base = task->thread.gs;
		} else
			base = task->thread.gs;
		ret = put_user(base, (unsigned long __user *)addr);
		break;
	}

	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

long sys_arch_prctl(int code, unsigned long addr)
{
	return do_arch_prctl(current, code, addr);
}

unsigned long KSTK_ESP(struct task_struct *task)
{
	return (test_tsk_thread_flag(task, TIF_IA32)) ?
			(task_pt_regs(task)->sp) : ((task)->thread.usersp);
}
