


#include <linux/interrupt.h>
#include <linux/kallsyms.h>
#include <linux/spinlock.h>
#include <linux/kprobes.h>
#include <linux/uaccess.h>
#include <linux/kdebug.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ptrace.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/kexec.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/bug.h>
#include <linux/nmi.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/io.h>

#ifdef CONFIG_EISA
#include <linux/ioport.h>
#include <linux/eisa.h>
#endif

#ifdef CONFIG_MCA
#include <linux/mca.h>
#endif

#if defined(CONFIG_EDAC)
#include <linux/edac.h>
#endif

#include <asm/kmemcheck.h>
#include <asm/stacktrace.h>
#include <asm/processor.h>
#include <asm/debugreg.h>
#include <asm/atomic.h>
#include <asm/system.h>
#include <asm/traps.h>
#include <asm/desc.h>
#include <asm/i387.h>
#include <asm/mce.h>

#include <asm/mach_traps.h>

#ifdef CONFIG_X86_64
#include <asm/x86_init.h>
#include <asm/pgalloc.h>
#include <asm/proto.h>
#else
#include <asm/processor-flags.h>
#include <asm/setup.h>

asmlinkage int system_call(void);


char ignore_fpu_irq;


gate_desc idt_table[NR_VECTORS] __page_aligned_data = { { { { 0, 0 } } }, };
#endif

DECLARE_BITMAP(used_vectors, NR_VECTORS);
EXPORT_SYMBOL_GPL(used_vectors);

static int ignore_nmis;

static inline void conditional_sti(struct pt_regs *regs)
{
	if (regs->flags & X86_EFLAGS_IF)
		local_irq_enable();
}

static inline void preempt_conditional_sti(struct pt_regs *regs)
{
	inc_preempt_count();
	if (regs->flags & X86_EFLAGS_IF)
		local_irq_enable();
}

static inline void conditional_cli(struct pt_regs *regs)
{
	if (regs->flags & X86_EFLAGS_IF)
		local_irq_disable();
}

static inline void preempt_conditional_cli(struct pt_regs *regs)
{
	if (regs->flags & X86_EFLAGS_IF)
		local_irq_disable();
	dec_preempt_count();
}

#ifdef CONFIG_X86_32
static inline void
die_if_kernel(const char *str, struct pt_regs *regs, long err)
{
	if (!user_mode_vm(regs))
		die(str, regs, err);
}
#endif

static void __kprobes
do_trap(int trapnr, int signr, char *str, struct pt_regs *regs,
	long error_code, siginfo_t *info)
{
	struct task_struct *tsk = current;

#ifdef CONFIG_X86_32
	if (regs->flags & X86_VM_MASK) {
		
		if (trapnr < 6)
			goto vm86_trap;
		goto trap_signal;
	}
#endif

	if (!user_mode(regs))
		goto kernel_trap;

#ifdef CONFIG_X86_32
trap_signal:
#endif
	
	tsk->thread.error_code = error_code;
	tsk->thread.trap_no = trapnr;

#ifdef CONFIG_X86_64
	if (show_unhandled_signals && unhandled_signal(tsk, signr) &&
	    printk_ratelimit()) {
		printk(KERN_INFO
		       "%s[%d] trap %s ip:%lx sp:%lx error:%lx",
		       tsk->comm, tsk->pid, str,
		       regs->ip, regs->sp, error_code);
		print_vma_addr(" in ", regs->ip);
		printk("\n");
	}
#endif

	if (info)
		force_sig_info(signr, info, tsk);
	else
		force_sig(signr, tsk);
	return;

kernel_trap:
	if (!fixup_exception(regs)) {
		tsk->thread.error_code = error_code;
		tsk->thread.trap_no = trapnr;
		die(str, regs, error_code);
	}
	return;

#ifdef CONFIG_X86_32
vm86_trap:
	if (handle_vm86_trap((struct kernel_vm86_regs *) regs,
						error_code, trapnr))
		goto trap_signal;
	return;
#endif
}

#define DO_ERROR(trapnr, signr, str, name)				\
dotraplinkage void do_##name(struct pt_regs *regs, long error_code)	\
{									\
	if (notify_die(DIE_TRAP, str, regs, error_code, trapnr, signr)	\
							== NOTIFY_STOP)	\
		return;							\
	conditional_sti(regs);						\
	do_trap(trapnr, signr, str, regs, error_code, NULL);		\
}

#define DO_ERROR_INFO(trapnr, signr, str, name, sicode, siaddr)		\
dotraplinkage void do_##name(struct pt_regs *regs, long error_code)	\
{									\
	siginfo_t info;							\
	info.si_signo = signr;						\
	info.si_errno = 0;						\
	info.si_code = sicode;						\
	info.si_addr = (void __user *)siaddr;				\
	if (notify_die(DIE_TRAP, str, regs, error_code, trapnr, signr)	\
							== NOTIFY_STOP)	\
		return;							\
	conditional_sti(regs);						\
	do_trap(trapnr, signr, str, regs, error_code, &info);		\
}

DO_ERROR_INFO(0, SIGFPE, "divide error", divide_error, FPE_INTDIV, regs->ip)
DO_ERROR(4, SIGSEGV, "overflow", overflow)
DO_ERROR(5, SIGSEGV, "bounds", bounds)
DO_ERROR_INFO(6, SIGILL, "invalid opcode", invalid_op, ILL_ILLOPN, regs->ip)
DO_ERROR(9, SIGFPE, "coprocessor segment overrun", coprocessor_segment_overrun)
DO_ERROR(10, SIGSEGV, "invalid TSS", invalid_TSS)
DO_ERROR(11, SIGBUS, "segment not present", segment_not_present)
#ifdef CONFIG_X86_32
DO_ERROR(12, SIGBUS, "stack segment", stack_segment)
#endif
DO_ERROR_INFO(17, SIGBUS, "alignment check", alignment_check, BUS_ADRALN, 0)

#ifdef CONFIG_X86_64

dotraplinkage void do_stack_segment(struct pt_regs *regs, long error_code)
{
	if (notify_die(DIE_TRAP, "stack segment", regs, error_code,
			12, SIGBUS) == NOTIFY_STOP)
		return;
	preempt_conditional_sti(regs);
	do_trap(12, SIGBUS, "stack segment", regs, error_code, NULL);
	preempt_conditional_cli(regs);
}

dotraplinkage void do_double_fault(struct pt_regs *regs, long error_code)
{
	static const char str[] = "double fault";
	struct task_struct *tsk = current;

	
	notify_die(DIE_TRAP, str, regs, error_code, 8, SIGSEGV);

	tsk->thread.error_code = error_code;
	tsk->thread.trap_no = 8;

	
	for (;;)
		die(str, regs, error_code);
}
#endif

dotraplinkage void __kprobes
do_general_protection(struct pt_regs *regs, long error_code)
{
	struct task_struct *tsk;

	conditional_sti(regs);

#ifdef CONFIG_X86_32
	if (regs->flags & X86_VM_MASK)
		goto gp_in_vm86;
#endif

	tsk = current;
	if (!user_mode(regs))
		goto gp_in_kernel;

	tsk->thread.error_code = error_code;
	tsk->thread.trap_no = 13;

	if (show_unhandled_signals && unhandled_signal(tsk, SIGSEGV) &&
			printk_ratelimit()) {
		printk(KERN_INFO
			"%s[%d] general protection ip:%lx sp:%lx error:%lx",
			tsk->comm, task_pid_nr(tsk),
			regs->ip, regs->sp, error_code);
		print_vma_addr(" in ", regs->ip);
		printk("\n");
	}

	force_sig(SIGSEGV, tsk);
	return;

#ifdef CONFIG_X86_32
gp_in_vm86:
	local_irq_enable();
	handle_vm86_fault((struct kernel_vm86_regs *) regs, error_code);
	return;
#endif

gp_in_kernel:
	if (fixup_exception(regs))
		return;

	tsk->thread.error_code = error_code;
	tsk->thread.trap_no = 13;
	if (notify_die(DIE_GPF, "general protection fault", regs,
				error_code, 13, SIGSEGV) == NOTIFY_STOP)
		return;
	die("general protection fault", regs, error_code);
}

static notrace __kprobes void
mem_parity_error(unsigned char reason, struct pt_regs *regs)
{
	printk(KERN_EMERG
		"Uhhuh. NMI received for unknown reason %02x on CPU %d.\n",
			reason, smp_processor_id());

	printk(KERN_EMERG
		"You have some hardware problem, likely on the PCI bus.\n");

#if defined(CONFIG_EDAC)
	if (edac_handler_set()) {
		edac_atomic_assert_error();
		return;
	}
#endif

	if (panic_on_unrecovered_nmi)
		panic("NMI: Not continuing");

	printk(KERN_EMERG "Dazed and confused, but trying to continue\n");

	
	reason = (reason & 0xf) | 4;
	outb(reason, 0x61);
}

static notrace __kprobes void
io_check_error(unsigned char reason, struct pt_regs *regs)
{
	unsigned long i;

	printk(KERN_EMERG "NMI: IOCK error (debug interrupt?)\n");
	show_registers(regs);

	if (panic_on_io_nmi)
		panic("NMI IOCK error: Not continuing");

	
	reason = (reason & 0xf) | 8;
	outb(reason, 0x61);

	i = 2000;
	while (--i)
		udelay(1000);

	reason &= ~8;
	outb(reason, 0x61);
}

static notrace __kprobes void
unknown_nmi_error(unsigned char reason, struct pt_regs *regs)
{
	if (notify_die(DIE_NMIUNKNOWN, "nmi", regs, reason, 2, SIGINT) ==
			NOTIFY_STOP)
		return;
#ifdef CONFIG_MCA
	
	if (MCA_bus) {
		mca_handle_nmi();
		return;
	}
#endif
	printk(KERN_EMERG
		"Uhhuh. NMI received for unknown reason %02x on CPU %d.\n",
			reason, smp_processor_id());

	printk(KERN_EMERG "Do you have a strange power saving mode enabled?\n");
	if (panic_on_unrecovered_nmi)
		panic("NMI: Not continuing");

	printk(KERN_EMERG "Dazed and confused, but trying to continue\n");
}

static notrace __kprobes void default_do_nmi(struct pt_regs *regs)
{
	unsigned char reason = 0;
	int cpu;

	cpu = smp_processor_id();

	
	if (!cpu)
		reason = get_nmi_reason();

	if (!(reason & 0xc0)) {
		if (notify_die(DIE_NMI_IPI, "nmi_ipi", regs, reason, 2, SIGINT)
								== NOTIFY_STOP)
			return;
#ifdef CONFIG_X86_LOCAL_APIC
		
		if (nmi_watchdog_tick(regs, reason))
			return;
		if (!do_nmi_callback(regs, cpu))
			unknown_nmi_error(reason, regs);
#else
		unknown_nmi_error(reason, regs);
#endif

		return;
	}
	if (notify_die(DIE_NMI, "nmi", regs, reason, 2, SIGINT) == NOTIFY_STOP)
		return;

	
	if (reason & 0x80)
		mem_parity_error(reason, regs);
	if (reason & 0x40)
		io_check_error(reason, regs);
#ifdef CONFIG_X86_32
	
	reassert_nmi();
#endif
}

dotraplinkage notrace __kprobes void
do_nmi(struct pt_regs *regs, long error_code)
{
	nmi_enter();

	inc_irq_stat(__nmi_count);

	if (!ignore_nmis)
		default_do_nmi(regs);

	nmi_exit();
}

void stop_nmi(void)
{
	acpi_nmi_disable();
	ignore_nmis++;
}

void restart_nmi(void)
{
	ignore_nmis--;
	acpi_nmi_enable();
}


dotraplinkage void __kprobes do_int3(struct pt_regs *regs, long error_code)
{
#ifdef CONFIG_KPROBES
	if (notify_die(DIE_INT3, "int3", regs, error_code, 3, SIGTRAP)
			== NOTIFY_STOP)
		return;
#else
	if (notify_die(DIE_TRAP, "int3", regs, error_code, 3, SIGTRAP)
			== NOTIFY_STOP)
		return;
#endif

	preempt_conditional_sti(regs);
	do_trap(3, SIGTRAP, "int3", regs, error_code, NULL);
	preempt_conditional_cli(regs);
}

#ifdef CONFIG_X86_64

asmlinkage __kprobes struct pt_regs *sync_regs(struct pt_regs *eregs)
{
	struct pt_regs *regs = eregs;
	
	if (eregs == (struct pt_regs *)eregs->sp)
		;
	
	else if (user_mode(eregs))
		regs = task_pt_regs(current);
	
	else if (eregs->flags & X86_EFLAGS_IF)
		regs = (struct pt_regs *)(eregs->sp -= sizeof(struct pt_regs));
	if (eregs != regs)
		*regs = *eregs;
	return regs;
}
#endif


dotraplinkage void __kprobes do_debug(struct pt_regs *regs, long error_code)
{
	struct task_struct *tsk = current;
	unsigned long condition;
	int si_code;

	get_debugreg(condition, 6);

	
	if (condition & DR_STEP && kmemcheck_trap(regs))
		return;

	
	clear_tsk_thread_flag(tsk, TIF_DEBUGCTLMSR);
	tsk->thread.debugctlmsr = 0;

	if (notify_die(DIE_DEBUG, "debug", regs, condition, error_code,
						SIGTRAP) == NOTIFY_STOP)
		return;

	
	preempt_conditional_sti(regs);

	
	if (condition & (DR_TRAP0|DR_TRAP1|DR_TRAP2|DR_TRAP3)) {
		if (!tsk->thread.debugreg7)
			goto clear_dr7;
	}

#ifdef CONFIG_X86_32
	if (regs->flags & X86_VM_MASK)
		goto debug_vm86;
#endif

	
	tsk->thread.debugreg6 = condition;

	
	if (condition & DR_STEP) {
		if (!user_mode(regs))
			goto clear_TF_reenable;
	}

	si_code = get_si_code(condition);
	
	send_sigtrap(tsk, regs, error_code, si_code);

	
clear_dr7:
	set_debugreg(0, 7);
	preempt_conditional_cli(regs);
	return;

#ifdef CONFIG_X86_32
debug_vm86:
	
	dec_preempt_count();
	handle_vm86_trap((struct kernel_vm86_regs *) regs, error_code, 1);
	conditional_cli(regs);
	return;
#endif

clear_TF_reenable:
	set_tsk_thread_flag(tsk, TIF_SINGLESTEP);
	regs->flags &= ~X86_EFLAGS_TF;
	preempt_conditional_cli(regs);
	return;
}

#ifdef CONFIG_X86_64
static int kernel_math_error(struct pt_regs *regs, const char *str, int trapnr)
{
	if (fixup_exception(regs))
		return 1;

	notify_die(DIE_GPF, str, regs, 0, trapnr, SIGFPE);
	
	current->thread.trap_no = trapnr;
	die(str, regs, 0);
	return 0;
}
#endif


void math_error(void __user *ip)
{
	struct task_struct *task;
	siginfo_t info;
	unsigned short cwd, swd, err;

	
	task = current;
	save_init_fpu(task);
	task->thread.trap_no = 16;
	task->thread.error_code = 0;
	info.si_signo = SIGFPE;
	info.si_errno = 0;
	info.si_addr = ip;
	
	cwd = get_fpu_cwd(task);
	swd = get_fpu_swd(task);

	err = swd & ~cwd;

	if (err & 0x001) {	
		
		info.si_code = FPE_FLTINV;
	} else if (err & 0x004) { 
		info.si_code = FPE_FLTDIV;
	} else if (err & 0x008) { 
		info.si_code = FPE_FLTOVF;
	} else if (err & 0x012) { 
		info.si_code = FPE_FLTUND;
	} else if (err & 0x020) { 
		info.si_code = FPE_FLTRES;
	} else {
		
		return;		
	}
	force_sig_info(SIGFPE, &info, task);
}

dotraplinkage void do_coprocessor_error(struct pt_regs *regs, long error_code)
{
	conditional_sti(regs);

#ifdef CONFIG_X86_32
	ignore_fpu_irq = 1;
#else
	if (!user_mode(regs) &&
	    kernel_math_error(regs, "kernel x87 math error", 16))
		return;
#endif

	math_error((void __user *)regs->ip);
}

static void simd_math_error(void __user *ip)
{
	struct task_struct *task;
	siginfo_t info;
	unsigned short mxcsr;

	
	task = current;
	save_init_fpu(task);
	task->thread.trap_no = 19;
	task->thread.error_code = 0;
	info.si_signo = SIGFPE;
	info.si_errno = 0;
	info.si_code = __SI_FAULT;
	info.si_addr = ip;
	
	mxcsr = get_fpu_mxcsr(task);
	switch (~((mxcsr & 0x1f80) >> 7) & (mxcsr & 0x3f)) {
	case 0x000:
	default:
		break;
	case 0x001: 
		info.si_code = FPE_FLTINV;
		break;
	case 0x002: 
	case 0x010: 
		info.si_code = FPE_FLTUND;
		break;
	case 0x004: 
		info.si_code = FPE_FLTDIV;
		break;
	case 0x008: 
		info.si_code = FPE_FLTOVF;
		break;
	case 0x020: 
		info.si_code = FPE_FLTRES;
		break;
	}
	force_sig_info(SIGFPE, &info, task);
}

dotraplinkage void
do_simd_coprocessor_error(struct pt_regs *regs, long error_code)
{
	conditional_sti(regs);

#ifdef CONFIG_X86_32
	if (cpu_has_xmm) {
		
		ignore_fpu_irq = 1;
		simd_math_error((void __user *)regs->ip);
		return;
	}
	
	if (regs->flags & X86_VM_MASK) {
		handle_vm86_fault((struct kernel_vm86_regs *)regs, error_code);
		return;
	}
	current->thread.trap_no = 19;
	current->thread.error_code = error_code;
	die_if_kernel("cache flush denied", regs, error_code);
	force_sig(SIGSEGV, current);
#else
	if (!user_mode(regs) &&
			kernel_math_error(regs, "kernel simd math error", 19))
		return;
	simd_math_error((void __user *)regs->ip);
#endif
}

dotraplinkage void
do_spurious_interrupt_bug(struct pt_regs *regs, long error_code)
{
	conditional_sti(regs);
#if 0
	
	printk(KERN_INFO "Ignoring P6 Local APIC Spurious Interrupt Bug...\n");
#endif
}

asmlinkage void __attribute__((weak)) smp_thermal_interrupt(void)
{
}

asmlinkage void __attribute__((weak)) smp_threshold_interrupt(void)
{
}


void __math_state_restore(void)
{
	struct thread_info *thread = current_thread_info();
	struct task_struct *tsk = thread->task;

	
	if (unlikely(restore_fpu_checking(tsk))) {
		stts();
		force_sig(SIGSEGV, tsk);
		return;
	}

	thread->status |= TS_USEDFPU;	
	tsk->fpu_counter++;
}


asmlinkage void math_state_restore(void)
{
	struct thread_info *thread = current_thread_info();
	struct task_struct *tsk = thread->task;

	if (!tsk_used_math(tsk)) {
		local_irq_enable();
		
		if (init_fpu(tsk)) {
			
			do_group_exit(SIGKILL);
			return;
		}
		local_irq_disable();
	}

	clts();				

	__math_state_restore();
}
EXPORT_SYMBOL_GPL(math_state_restore);

#ifndef CONFIG_MATH_EMULATION
void math_emulate(struct math_emu_info *info)
{
	printk(KERN_EMERG
		"math-emulation not enabled and no coprocessor found.\n");
	printk(KERN_EMERG "killing %s.\n", current->comm);
	force_sig(SIGFPE, current);
	schedule();
}
#endif 

dotraplinkage void __kprobes
do_device_not_available(struct pt_regs *regs, long error_code)
{
#ifdef CONFIG_X86_32
	if (read_cr0() & X86_CR0_EM) {
		struct math_emu_info info = { };

		conditional_sti(regs);

		info.regs = regs;
		math_emulate(&info);
	} else {
		math_state_restore(); 
		conditional_sti(regs);
	}
#else
	math_state_restore();
#endif
}

#ifdef CONFIG_X86_32
dotraplinkage void do_iret_error(struct pt_regs *regs, long error_code)
{
	siginfo_t info;
	local_irq_enable();

	info.si_signo = SIGILL;
	info.si_errno = 0;
	info.si_code = ILL_BADSTK;
	info.si_addr = NULL;
	if (notify_die(DIE_TRAP, "iret exception",
			regs, error_code, 32, SIGILL) == NOTIFY_STOP)
		return;
	do_trap(32, SIGILL, "iret exception", regs, error_code, &info);
}
#endif

void __init trap_init(void)
{
	int i;

#ifdef CONFIG_EISA
	void __iomem *p = early_ioremap(0x0FFFD9, 4);

	if (readl(p) == 'E' + ('I'<<8) + ('S'<<16) + ('A'<<24))
		EISA_bus = 1;
	early_iounmap(p, 4);
#endif

	set_intr_gate(0, &divide_error);
	set_intr_gate_ist(1, &debug, DEBUG_STACK);
	set_intr_gate_ist(2, &nmi, NMI_STACK);
	
	set_system_intr_gate_ist(3, &int3, DEBUG_STACK);
	
	set_system_intr_gate(4, &overflow);
	set_intr_gate(5, &bounds);
	set_intr_gate(6, &invalid_op);
	set_intr_gate(7, &device_not_available);
#ifdef CONFIG_X86_32
	set_task_gate(8, GDT_ENTRY_DOUBLEFAULT_TSS);
#else
	set_intr_gate_ist(8, &double_fault, DOUBLEFAULT_STACK);
#endif
	set_intr_gate(9, &coprocessor_segment_overrun);
	set_intr_gate(10, &invalid_TSS);
	set_intr_gate(11, &segment_not_present);
	set_intr_gate_ist(12, &stack_segment, STACKFAULT_STACK);
	set_intr_gate(13, &general_protection);
	set_intr_gate(14, &page_fault);
	set_intr_gate(15, &spurious_interrupt_bug);
	set_intr_gate(16, &coprocessor_error);
	set_intr_gate(17, &alignment_check);
#ifdef CONFIG_X86_MCE
	set_intr_gate_ist(18, &machine_check, MCE_STACK);
#endif
	set_intr_gate(19, &simd_coprocessor_error);

	
	for (i = 0; i < FIRST_EXTERNAL_VECTOR; i++)
		set_bit(i, used_vectors);

#ifdef CONFIG_IA32_EMULATION
	set_system_intr_gate(IA32_SYSCALL_VECTOR, ia32_syscall);
	set_bit(IA32_SYSCALL_VECTOR, used_vectors);
#endif

#ifdef CONFIG_X86_32
	if (cpu_has_fxsr) {
		printk(KERN_INFO "Enabling fast FPU save and restore... ");
		set_in_cr4(X86_CR4_OSFXSR);
		printk("done.\n");
	}
	if (cpu_has_xmm) {
		printk(KERN_INFO
			"Enabling unmasked SIMD FPU exception support... ");
		set_in_cr4(X86_CR4_OSXMMEXCPT);
		printk("done.\n");
	}

	set_system_trap_gate(SYSCALL_VECTOR, &system_call);
	set_bit(SYSCALL_VECTOR, used_vectors);
#endif

	
	cpu_init();

	x86_init.irqs.trap_init();
}
