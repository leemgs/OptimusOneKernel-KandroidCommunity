
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/spinlock.h>
#include <linux/personality.h>
#include <linux/kallsyms.h>
#include <linux/delay.h>
#include <linux/hardirq.h>
#include <linux/init.h>
#include <linux/uaccess.h>

#include <asm/atomic.h>
#include <asm/cacheflush.h>
#include <asm/system.h>
#include <asm/unistd.h>
#include <asm/traps.h>
#include <asm/unwind.h>

#include "ptrace.h"
#include "signal.h"

static const char *handler[]= { "prefetch abort", "data abort", "address exception", "interrupt" };

#include "../mach-msm/lge/lge_errorhandler.h"
char crash_buf[LGE_ERROR_MAX_ROW][LGE_ERROR_MAX_COLUMN];
extern void smsm_reset_modem(unsigned mode);
static int lge_error_analyzing = 0;
static int lge_error_fb_cnt = 0;
extern void ram_console_panic(void);
extern int hidden_reset_enable;


#ifdef CONFIG_DEBUG_USER
unsigned int user_debug;

static int __init user_debug_setup(char *str)
{
	get_option(&str, &user_debug);
	return 1;
}
__setup("user_debug=", user_debug_setup);
#endif

static void dump_mem(const char *, const char *, unsigned long, unsigned long);

void dump_backtrace_entry(unsigned long where, unsigned long from, unsigned long frame)
{
#ifdef CONFIG_KALLSYMS
	char sym1[KSYM_SYMBOL_LEN], sym2[KSYM_SYMBOL_LEN];
 
	if (lge_error_analyzing == 1){
	  char * temp;
	  sprintf(crash_buf[lge_error_fb_cnt], "[<%08lx>] " ); temp = crash_buf[lge_error_fb_cnt]; temp+=13; sprint_symbol(temp, where);		
	  lge_error_fb_cnt++;
	} else {	
	sprint_symbol(sym1, where);
	sprint_symbol(sym2, from);
	printk("[<%08lx>] (%s) from [<%08lx>] (%s)\n", where, sym1, from, sym2);
	}
 
#else
	printk("Function entered at [<%08lx>] from [<%08lx>]\n", where, from);
#endif

	if (in_exception_text(where))
		dump_mem("", "Exception stack", frame + 4, frame + 4 + sizeof(struct pt_regs));
}

#ifndef CONFIG_ARM_UNWIND

static int verify_stack(unsigned long sp)
{
	if (sp < PAGE_OFFSET ||
	    (sp > (unsigned long)high_memory && high_memory != NULL))
		return -EFAULT;

	return 0;
}
#endif


static void dump_mem(const char *lvl, const char *str, unsigned long bottom,
		     unsigned long top)
{
	unsigned long first;
	mm_segment_t fs;
	int i;

	
	fs = get_fs();
	set_fs(KERNEL_DS);

	printk("%s%s(0x%08lx to 0x%08lx)\n", lvl, str, bottom, top);

	for (first = bottom & ~31; first < top; first += 32) {
		unsigned long p;
		char str[sizeof(" 12345678") * 8 + 1];

		memset(str, ' ', sizeof(str));
		str[sizeof(str) - 1] = '\0';

		for (p = first, i = 0; i < 8 && p < top; i++, p += 4) {
			if (p >= bottom && p < top) {
				unsigned long val;
				if (__get_user(val, (unsigned long *)p) == 0)
					sprintf(str + i * 9, " %08lx", val);
				else
					sprintf(str + i * 9, " ????????");
			}
		}
		printk("%s%04lx:%s\n", lvl, first & 0xffff, str);
	}

	set_fs(fs);
}

static void dump_instr(const char *lvl, struct pt_regs *regs)
{
	unsigned long addr = instruction_pointer(regs);
	const int thumb = thumb_mode(regs);
	const int width = thumb ? 4 : 8;
	mm_segment_t fs;
	char str[sizeof("00000000 ") * 5 + 2 + 1], *p = str;
	int i;

	
	fs = get_fs();
	set_fs(KERNEL_DS);

	for (i = -4; i < 1; i++) {
		unsigned int val, bad;

		if (thumb)
			bad = __get_user(val, &((u16 *)addr)[i]);
		else
			bad = __get_user(val, &((u32 *)addr)[i]);

		if (!bad)
			p += sprintf(p, i == 0 ? "(%0*x) " : "%0*x ",
					width, val);
		else {
			p += sprintf(p, "bad PC value");
			break;
		}
	}
	printk("%sCode: %s\n", lvl, str);

	set_fs(fs);
}

#ifdef CONFIG_ARM_UNWIND
static inline void dump_backtrace(struct pt_regs *regs, struct task_struct *tsk)
{
	unwind_backtrace(regs, tsk);
}
#else
static void dump_backtrace(struct pt_regs *regs, struct task_struct *tsk)
{
	unsigned int fp, mode;
	int ok = 1;

	printk("Backtrace: ");

	if (!tsk)
		tsk = current;

	if (regs) {
		fp = regs->ARM_fp;
		mode = processor_mode(regs);
	} else if (tsk != current) {
		fp = thread_saved_fp(tsk);
		mode = 0x10;
	} else {
		asm("mov %0, fp" : "=r" (fp) : : "cc");
		mode = 0x10;
	}

	if (!fp) {
		printk("no frame pointer");
		ok = 0;
	} else if (verify_stack(fp)) {
		printk("invalid frame pointer 0x%08x", fp);
		ok = 0;
	} else if (fp < (unsigned long)end_of_stack(tsk))
		printk("frame pointer underflow");
	printk("\n");

	if (ok)
		c_backtrace(fp, mode);
}
#endif

void dump_stack(void)
{
	dump_backtrace(NULL, NULL);
}

EXPORT_SYMBOL(dump_stack);

void show_stack(struct task_struct *tsk, unsigned long *sp)
{
	dump_backtrace(NULL, tsk);
	barrier();
}

#ifdef CONFIG_PREEMPT
#define S_PREEMPT " PREEMPT"
#else
#define S_PREEMPT ""
#endif
#ifdef CONFIG_SMP
#define S_SMP " SMP"
#else
#define S_SMP ""
#endif

static void __die(const char *str, int err, struct thread_info *thread, struct pt_regs *regs)
{
	struct task_struct *tsk = thread->task;
	static int die_counter;

#ifdef CONFIG_MACH_LGE
	printk(KERN_EMERG">>>>>\n");
#endif
	printk(KERN_EMERG "Internal error: %s: %x [#%d]" S_PREEMPT S_SMP "\n",
	       str, err, ++die_counter);
	sysfs_printk_last_file();
	print_modules();
	__show_regs(regs);
	printk(KERN_EMERG "Process %.*s (pid: %d, stack limit = 0x%p)\n",
		TASK_COMM_LEN, tsk->comm, task_pid_nr(tsk), thread + 1);

	if (!user_mode(regs) || in_interrupt()) {
		dump_mem(KERN_EMERG, "Stack: ", regs->ARM_sp,
			 THREAD_SIZE + (unsigned long)task_stack_page(tsk));
#ifdef CONFIG_MACH_LGE
		printk(KERN_EMERG"vvvvv\n");
#endif
		dump_backtrace(regs, tsk);
		dump_instr(KERN_EMERG, regs);
#ifdef CONFIG_MACH_LGE
		printk(KERN_EMERG"^^^^^\n");
#endif
	}

		
	if (!hidden_reset_enable) 
	{
		
		char * temp;
		int ret;

		
			lge_error_analyzing = 1;
			sprintf(crash_buf[1],"---------------------------------------");
			sprintf(crash_buf[2], "Linux Kernel Panic ");
			sprintf(crash_buf[3], "Process %s (pid: %d)",tsk->comm, task_pid_nr(tsk));
			sprintf(crash_buf[4], "--------------------------------------");
			sprintf(crash_buf[5], "PC: " ); temp = crash_buf[5]; temp+=4; sprint_symbol(temp, instruction_pointer(regs));
			sprintf(crash_buf[6], "LR: " ); temp = crash_buf[6]; temp+=4; sprint_symbol(temp, regs->ARM_lr);
			sprintf(crash_buf[7], "pc : [<%08lx>]    lr : [<%08lx>]", regs->ARM_pc, regs->ARM_lr); 
			sprintf(crash_buf[8], "psr: %08lx  sp : %08lx", regs->ARM_cpsr, regs->ARM_sp);
			sprintf(crash_buf[9], "ip : %08lx  fp : %08lx", regs->ARM_ip, regs->ARM_fp);
		    sprintf(crash_buf[10],"r10: %08lx  r9 : %08lx", regs->ARM_r10, regs->ARM_r9); 
			sprintf(crash_buf[11],"r8 : %08lx  r7 : %08lx", regs->ARM_r8, regs->ARM_r7);
			sprintf(crash_buf[12],"r6 : %08lx  r5 : %08lx", regs->ARM_r6,regs->ARM_r5);  
			sprintf(crash_buf[13],"r4 : %08lx  r3 : %08lx",regs->ARM_r4, regs->ARM_r3);
			sprintf(crash_buf[14],"r2 : %08lx  r1 : %08lx",regs->ARM_r2,regs->ARM_r1);
			sprintf(crash_buf[15],"r0 : %08lx", regs->ARM_r0);
			sprintf(crash_buf[16], "-------<Call Stack>------------------");
			lge_error_fb_cnt = 17;
			dump_backtrace(regs, tsk);
			dump_instr(KERN_EMERG, regs);
			
			ret = LGE_ErrorHandler_Main(APPL_CRASH, (char *)crash_buf);
			lge_error_analyzing = 0;
			
			smsm_reset_modem(ret);
			while(1) 
				;
		
	}
	
}

DEFINE_SPINLOCK(die_lock);


NORET_TYPE void die(const char *str, struct pt_regs *regs, int err)
{
	struct thread_info *thread = current_thread_info();

	oops_enter();

	spin_lock_irq(&die_lock);
	console_verbose();
	bust_spinlocks(1);
	__die(str, err, thread, regs);
	bust_spinlocks(0);
	add_taint(TAINT_DIE);
	spin_unlock_irq(&die_lock);
	oops_exit();

	if (in_interrupt())
		panic("Fatal exception in interrupt");

	if (panic_on_oops)
		panic("Fatal exception");

	do_exit(SIGSEGV);
}

void arm_notify_die(const char *str, struct pt_regs *regs,
		struct siginfo *info, unsigned long err, unsigned long trap)
{
	if (user_mode(regs)) {
		current->thread.error_code = err;
		current->thread.trap_no = trap;

		force_sig_info(info->si_signo, info, current);
	} else {
		die(str, regs, err);
	}
}

static LIST_HEAD(undef_hook);
static DEFINE_SPINLOCK(undef_lock);

void register_undef_hook(struct undef_hook *hook)
{
	unsigned long flags;

	spin_lock_irqsave(&undef_lock, flags);
	list_add(&hook->node, &undef_hook);
	spin_unlock_irqrestore(&undef_lock, flags);
}

void unregister_undef_hook(struct undef_hook *hook)
{
	unsigned long flags;

	spin_lock_irqsave(&undef_lock, flags);
	list_del(&hook->node);
	spin_unlock_irqrestore(&undef_lock, flags);
}

static int call_undef_hook(struct pt_regs *regs, unsigned int instr)
{
	struct undef_hook *hook;
	unsigned long flags;
	int (*fn)(struct pt_regs *regs, unsigned int instr) = NULL;

	spin_lock_irqsave(&undef_lock, flags);
	list_for_each_entry(hook, &undef_hook, node)
		if ((instr & hook->instr_mask) == hook->instr_val &&
		    (regs->ARM_cpsr & hook->cpsr_mask) == hook->cpsr_val)
			fn = hook->fn;
	spin_unlock_irqrestore(&undef_lock, flags);

	return fn ? fn(regs, instr) : 1;
}

asmlinkage void __exception do_undefinstr(struct pt_regs *regs)
{
	unsigned int correction = thumb_mode(regs) ? 2 : 4;
	unsigned int instr;
	siginfo_t info;
	void __user *pc;

	
	regs->ARM_pc -= correction;

	pc = (void __user *)instruction_pointer(regs);

	if (processor_mode(regs) == SVC_MODE) {
		instr = *(u32 *) pc;
	} else if (thumb_mode(regs)) {
		get_user(instr, (u16 __user *)pc);
	} else {
		get_user(instr, (u32 __user *)pc);
	}

	if (call_undef_hook(regs, instr) == 0)
		return;

#ifdef CONFIG_DEBUG_USER
	if (user_debug & UDBG_UNDEFINED) {
		printk(KERN_INFO "%s (%d): undefined instruction: pc=%p\n",
			current->comm, task_pid_nr(current), pc);
		dump_instr(KERN_INFO, regs);
	}
#endif

	info.si_signo = SIGILL;
	info.si_errno = 0;
	info.si_code  = ILL_ILLOPC;
	info.si_addr  = pc;

	arm_notify_die("Oops - undefined instruction", regs, &info, 0, 6);
}

asmlinkage void do_unexp_fiq (struct pt_regs *regs)
{
	printk("Hmm.  Unexpected FIQ received, but trying to continue\n");
	printk("You may have a hardware problem...\n");
}


asmlinkage void bad_mode(struct pt_regs *regs, int reason)
{
	console_verbose();

	printk(KERN_CRIT "Bad mode in %s handler detected\n", handler[reason]);

	die("Oops - bad mode", regs, 0);
	local_irq_disable();
	panic("bad mode");
}

static int bad_syscall(int n, struct pt_regs *regs)
{
	struct thread_info *thread = current_thread_info();
	siginfo_t info;

	if (current->personality != PER_LINUX &&
	    current->personality != PER_LINUX_32BIT &&
	    thread->exec_domain->handler) {
		thread->exec_domain->handler(n, regs);
		return regs->ARM_r0;
	}

#ifdef CONFIG_DEBUG_USER
	if (user_debug & UDBG_SYSCALL) {
		printk(KERN_ERR "[%d] %s: obsolete system call %08x.\n",
			task_pid_nr(current), current->comm, n);
		dump_instr(KERN_ERR, regs);
	}
#endif

	info.si_signo = SIGILL;
	info.si_errno = 0;
	info.si_code  = ILL_ILLTRP;
	info.si_addr  = (void __user *)instruction_pointer(regs) -
			 (thumb_mode(regs) ? 2 : 4);

	arm_notify_die("Oops - bad syscall", regs, &info, n, 0);

	return regs->ARM_r0;
}

static inline void
do_cache_op(unsigned long start, unsigned long end, int flags)
{
	struct mm_struct *mm = current->active_mm;
	struct vm_area_struct *vma;

	if (end < start || flags)
		return;

	down_read(&mm->mmap_sem);
	vma = find_vma(mm, start);
	if (vma && vma->vm_start < end) {
		if (start < vma->vm_start)
			start = vma->vm_start;
		if (end > vma->vm_end)
			end = vma->vm_end;

		flush_cache_user_range(vma, start, end);
#ifdef CONFIG_ARCH_MSM7X27
		dmb();
#endif

	}
	up_read(&mm->mmap_sem);
}


#define NR(x) ((__ARM_NR_##x) - __ARM_NR_BASE)
asmlinkage int arm_syscall(int no, struct pt_regs *regs)
{
	struct thread_info *thread = current_thread_info();
	siginfo_t info;

	if ((no >> 16) != (__ARM_NR_BASE>> 16))
		return bad_syscall(no, regs);

	switch (no & 0xffff) {
	case 0: 
		info.si_signo = SIGSEGV;
		info.si_errno = 0;
		info.si_code  = SEGV_MAPERR;
		info.si_addr  = NULL;

		arm_notify_die("branch through zero", regs, &info, 0, 0);
		return 0;

	case NR(breakpoint): 
		regs->ARM_pc -= thumb_mode(regs) ? 2 : 4;
		ptrace_break(current, regs);
		return regs->ARM_r0;

	
	case NR(cacheflush):
		do_cache_op(regs->ARM_r0, regs->ARM_r1, regs->ARM_r2);
		return 0;

	case NR(usr26):
		if (!(elf_hwcap & HWCAP_26BIT))
			break;
		regs->ARM_cpsr &= ~MODE32_BIT;
		return regs->ARM_r0;

	case NR(usr32):
		if (!(elf_hwcap & HWCAP_26BIT))
			break;
		regs->ARM_cpsr |= MODE32_BIT;
		return regs->ARM_r0;

	case NR(set_tls):
		thread->tp_value = regs->ARM_r0;
#if defined(CONFIG_HAS_TLS_REG)
		asm ("mcr p15, 0, %0, c13, c0, 3" : : "r" (regs->ARM_r0) );

#endif
		
		*((unsigned int *)0xffff0ff0) = regs->ARM_r0;

		return 0;

#ifdef CONFIG_NEEDS_SYSCALL_FOR_CMPXCHG
	
	case NR(cmpxchg):
	for (;;) {
		extern void do_DataAbort(unsigned long addr, unsigned int fsr,
					 struct pt_regs *regs);
		unsigned long val;
		unsigned long addr = regs->ARM_r2;
		struct mm_struct *mm = current->mm;
		pgd_t *pgd; pmd_t *pmd; pte_t *pte;
		spinlock_t *ptl;

		regs->ARM_cpsr &= ~PSR_C_BIT;
		down_read(&mm->mmap_sem);
		pgd = pgd_offset(mm, addr);
		if (!pgd_present(*pgd))
			goto bad_access;
		pmd = pmd_offset(pgd, addr);
		if (!pmd_present(*pmd))
			goto bad_access;
		pte = pte_offset_map_lock(mm, pmd, addr, &ptl);
		if (!pte_present(*pte) || !pte_dirty(*pte)) {
			pte_unmap_unlock(pte, ptl);
			goto bad_access;
		}
		val = *(unsigned long *)addr;
		val -= regs->ARM_r0;
		if (val == 0) {
			*(unsigned long *)addr = regs->ARM_r1;
			regs->ARM_cpsr |= PSR_C_BIT;
		}
		pte_unmap_unlock(pte, ptl);
		up_read(&mm->mmap_sem);
		return val;

		bad_access:
		up_read(&mm->mmap_sem);
		
		do_DataAbort(addr, 15 + (1 << 11), regs);
	}
#endif

	default:
		
		if ((no & 0xffff) <= 0x7ff)
			return -ENOSYS;
		break;
	}
#ifdef CONFIG_DEBUG_USER
	
	if (user_debug & UDBG_SYSCALL) {
		printk("[%d] %s: arm syscall %d\n",
		       task_pid_nr(current), current->comm, no);
		dump_instr("", regs);
		if (user_mode(regs)) {
			__show_regs(regs);
			c_backtrace(regs->ARM_fp, processor_mode(regs));
		}
	}
#endif
	info.si_signo = SIGILL;
	info.si_errno = 0;
	info.si_code  = ILL_ILLTRP;
	info.si_addr  = (void __user *)instruction_pointer(regs) -
			 (thumb_mode(regs) ? 2 : 4);

	arm_notify_die("Oops - bad syscall(2)", regs, &info, no, 0);
	return 0;
}

#ifdef CONFIG_TLS_REG_EMUL



static int get_tp_trap(struct pt_regs *regs, unsigned int instr)
{
	int reg = (instr >> 12) & 15;
	if (reg == 15)
		return 1;
	regs->uregs[reg] = current_thread_info()->tp_value;
	regs->ARM_pc += 4;
	return 0;
}

static struct undef_hook arm_mrc_hook = {
	.instr_mask	= 0x0fff0fff,
	.instr_val	= 0x0e1d0f70,
	.cpsr_mask	= PSR_T_BIT,
	.cpsr_val	= 0,
	.fn		= get_tp_trap,
};

static int __init arm_mrc_hook_init(void)
{
	register_undef_hook(&arm_mrc_hook);
	return 0;
}

late_initcall(arm_mrc_hook_init);

#endif

void __bad_xchg(volatile void *ptr, int size)
{
	printk("xchg: bad data size: pc 0x%p, ptr 0x%p, size %d\n",
		__builtin_return_address(0), ptr, size);
	BUG();
}
EXPORT_SYMBOL(__bad_xchg);


asmlinkage void
baddataabort(int code, unsigned long instr, struct pt_regs *regs)
{
	unsigned long addr = instruction_pointer(regs);
	siginfo_t info;

#ifdef CONFIG_DEBUG_USER
	if (user_debug & UDBG_BADABORT) {
		printk(KERN_ERR "[%d] %s: bad data abort: code %d instr 0x%08lx\n",
			task_pid_nr(current), current->comm, code, instr);
		dump_instr(KERN_ERR, regs);
		show_pte(current->mm, addr);
	}
#endif

	info.si_signo = SIGILL;
	info.si_errno = 0;
	info.si_code  = ILL_ILLOPC;
	info.si_addr  = (void __user *)addr;

	arm_notify_die("unknown data abort code", regs, &info, instr, 0);
}

void __attribute__((noreturn)) __bug(const char *file, int line)
{
	printk(KERN_CRIT"kernel BUG at %s:%d!\n", file, line);
	*(int *)0 = 0;

	
	for (;;);
}
EXPORT_SYMBOL(__bug);

void __readwrite_bug(const char *fn)
{
	printk("%s called, but not implemented\n", fn);
	BUG();
}
EXPORT_SYMBOL(__readwrite_bug);

void __pte_error(const char *file, int line, unsigned long val)
{
	printk("%s:%d: bad pte %08lx.\n", file, line, val);
}

void __pmd_error(const char *file, int line, unsigned long val)
{
	printk("%s:%d: bad pmd %08lx.\n", file, line, val);
}

void __pgd_error(const char *file, int line, unsigned long val)
{
	printk("%s:%d: bad pgd %08lx.\n", file, line, val);
}

asmlinkage void __div0(void)
{
	printk("Division by zero in kernel.\n");
	dump_stack();
}
EXPORT_SYMBOL(__div0);

void abort(void)
{
	BUG();

	
	panic("Oops failed to kill thread");
}
EXPORT_SYMBOL(abort);

void __init trap_init(void)
{
	return;
}

void __init early_trap_init(void)
{
	unsigned long vectors = CONFIG_VECTORS_BASE;
	extern char __stubs_start[], __stubs_end[];
	extern char __vectors_start[], __vectors_end[];
	extern char __kuser_helper_start[], __kuser_helper_end[];
	int kuser_sz = __kuser_helper_end - __kuser_helper_start;

	
	memcpy((void *)vectors, __vectors_start, __vectors_end - __vectors_start);
	memcpy((void *)vectors + 0x200, __stubs_start, __stubs_end - __stubs_start);
	memcpy((void *)vectors + 0x1000 - kuser_sz, __kuser_helper_start, kuser_sz);

	
	memcpy((void *)KERN_SIGRETURN_CODE, sigreturn_codes,
	       sizeof(sigreturn_codes));
	memcpy((void *)KERN_RESTART_CODE, syscall_restart_code,
	       sizeof(syscall_restart_code));

	flush_icache_range(vectors, vectors + PAGE_SIZE);
	modify_domain(DOMAIN_USER, DOMAIN_CLIENT);
}
