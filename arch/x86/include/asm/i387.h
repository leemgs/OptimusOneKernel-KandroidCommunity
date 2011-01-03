

#ifndef _ASM_X86_I387_H
#define _ASM_X86_I387_H

#include <linux/sched.h>
#include <linux/kernel_stat.h>
#include <linux/regset.h>
#include <linux/hardirq.h>
#include <asm/asm.h>
#include <asm/processor.h>
#include <asm/sigcontext.h>
#include <asm/user.h>
#include <asm/uaccess.h>
#include <asm/xsave.h>

extern unsigned int sig_xstate_size;
extern void fpu_init(void);
extern void mxcsr_feature_mask_init(void);
extern int init_fpu(struct task_struct *child);
extern asmlinkage void math_state_restore(void);
extern void __math_state_restore(void);
extern void init_thread_xstate(void);
extern int dump_fpu(struct pt_regs *, struct user_i387_struct *);

extern user_regset_active_fn fpregs_active, xfpregs_active;
extern user_regset_get_fn fpregs_get, xfpregs_get, fpregs_soft_get;
extern user_regset_set_fn fpregs_set, xfpregs_set, fpregs_soft_set;

extern struct _fpx_sw_bytes fx_sw_reserved;
#ifdef CONFIG_IA32_EMULATION
extern unsigned int sig_xstate_ia32_size;
extern struct _fpx_sw_bytes fx_sw_reserved_ia32;
struct _fpstate_ia32;
struct _xstate_ia32;
extern int save_i387_xstate_ia32(void __user *buf);
extern int restore_i387_xstate_ia32(void __user *buf);
#endif

#define X87_FSW_ES (1 << 7)	

#ifdef CONFIG_X86_64


static inline void tolerant_fwait(void)
{
	asm volatile("1: fwait\n"
		     "2:\n"
		     _ASM_EXTABLE(1b, 2b));
}

static inline int fxrstor_checking(struct i387_fxsave_struct *fx)
{
	int err;

	asm volatile("1:  rex64/fxrstor (%[fx])\n\t"
		     "2:\n"
		     ".section .fixup,\"ax\"\n"
		     "3:  movl $-1,%[err]\n"
		     "    jmp  2b\n"
		     ".previous\n"
		     _ASM_EXTABLE(1b, 3b)
		     : [err] "=r" (err)
#if 0 
		     : [fx] "r" (fx), "m" (*fx), "0" (0));
#else
		     : [fx] "cdaSDb" (fx), "m" (*fx), "0" (0));
#endif
	return err;
}


static inline void clear_fpu_state(struct task_struct *tsk)
{
	struct xsave_struct *xstate = &tsk->thread.xstate->xsave;
	struct i387_fxsave_struct *fx = &tsk->thread.xstate->fxsave;

	
	if ((task_thread_info(tsk)->status & TS_XSAVE) &&
	    !(xstate->xsave_hdr.xstate_bv & XSTATE_FP))
		return;

	if (unlikely(fx->swd & X87_FSW_ES))
		asm volatile("fnclex");
	alternative_input(ASM_NOP8 ASM_NOP2,
			  "    emms\n"		
			  "    fildl %%gs:0",	
			  X86_FEATURE_FXSAVE_LEAK);
}

static inline int fxsave_user(struct i387_fxsave_struct __user *fx)
{
	int err;

	asm volatile("1:  rex64/fxsave (%[fx])\n\t"
		     "2:\n"
		     ".section .fixup,\"ax\"\n"
		     "3:  movl $-1,%[err]\n"
		     "    jmp  2b\n"
		     ".previous\n"
		     _ASM_EXTABLE(1b, 3b)
		     : [err] "=r" (err), "=m" (*fx)
#if 0 
		     : [fx] "r" (fx), "0" (0));
#else
		     : [fx] "cdaSDb" (fx), "0" (0));
#endif
	if (unlikely(err) &&
	    __clear_user(fx, sizeof(struct i387_fxsave_struct)))
		err = -EFAULT;
	
	return err;
}

static inline void fxsave(struct task_struct *tsk)
{
	
#if 0
	
	__asm__ __volatile__("fxsaveq %0"
			     : "=m" (tsk->thread.xstate->fxsave));
#elif 0
	
	__asm__ __volatile__("rex64/fxsave %0"
			     : "=m" (tsk->thread.xstate->fxsave));
#else
	
	__asm__ __volatile__("rex64/fxsave (%1)"
			     : "=m" (tsk->thread.xstate->fxsave)
			     : "cdaSDb" (&tsk->thread.xstate->fxsave));
#endif
}

static inline void __save_init_fpu(struct task_struct *tsk)
{
	if (task_thread_info(tsk)->status & TS_XSAVE)
		xsave(tsk);
	else
		fxsave(tsk);

	clear_fpu_state(tsk);
	task_thread_info(tsk)->status &= ~TS_USEDFPU;
}

#else  

#ifdef CONFIG_MATH_EMULATION
extern void finit_task(struct task_struct *tsk);
#else
static inline void finit_task(struct task_struct *tsk)
{
}
#endif

static inline void tolerant_fwait(void)
{
	asm volatile("fnclex ; fwait");
}


static inline int fxrstor_checking(struct i387_fxsave_struct *fx)
{
	
	alternative_input(
		"nop ; frstor %1",
		"fxrstor %1",
		X86_FEATURE_FXSR,
		"m" (*fx));

	return 0;
}


#ifdef CONFIG_SMP
#define safe_address (__per_cpu_offset[0])
#else
#define safe_address (kstat_cpu(0).cpustat.user)
#endif


static inline void __save_init_fpu(struct task_struct *tsk)
{
	if (task_thread_info(tsk)->status & TS_XSAVE) {
		struct xsave_struct *xstate = &tsk->thread.xstate->xsave;
		struct i387_fxsave_struct *fx = &tsk->thread.xstate->fxsave;

		xsave(tsk);

		
		if (!(xstate->xsave_hdr.xstate_bv & XSTATE_FP))
			goto end;

		if (unlikely(fx->swd & X87_FSW_ES))
			asm volatile("fnclex");

		
		goto clear_state;
	}

	
	alternative_input(
		"fnsave %[fx] ;fwait;" GENERIC_NOP8 GENERIC_NOP4,
		"fxsave %[fx]\n"
		"bt $7,%[fsw] ; jnc 1f ; fnclex\n1:",
		X86_FEATURE_FXSR,
		[fx] "m" (tsk->thread.xstate->fxsave),
		[fsw] "m" (tsk->thread.xstate->fxsave.swd) : "memory");
clear_state:
	
	alternative_input(
		GENERIC_NOP8 GENERIC_NOP2,
		"emms\n\t"	  	
		"fildl %[addr]", 	
		X86_FEATURE_FXSAVE_LEAK,
		[addr] "m" (safe_address));
end:
	task_thread_info(tsk)->status &= ~TS_USEDFPU;
}

#endif	

static inline int restore_fpu_checking(struct task_struct *tsk)
{
	if (task_thread_info(tsk)->status & TS_XSAVE)
		return xrstor_checking(&tsk->thread.xstate->xsave);
	else
		return fxrstor_checking(&tsk->thread.xstate->fxsave);
}


extern int save_i387_xstate(void __user *buf);
extern int restore_i387_xstate(void __user *buf);

static inline void __unlazy_fpu(struct task_struct *tsk)
{
	if (task_thread_info(tsk)->status & TS_USEDFPU) {
		__save_init_fpu(tsk);
		stts();
	} else
		tsk->fpu_counter = 0;
}

static inline void __clear_fpu(struct task_struct *tsk)
{
	if (task_thread_info(tsk)->status & TS_USEDFPU) {
		tolerant_fwait();
		task_thread_info(tsk)->status &= ~TS_USEDFPU;
		stts();
	}
}

static inline void kernel_fpu_begin(void)
{
	struct thread_info *me = current_thread_info();
	preempt_disable();
	if (me->status & TS_USEDFPU)
		__save_init_fpu(me->task);
	else
		clts();
}

static inline void kernel_fpu_end(void)
{
	stts();
	preempt_enable();
}

static inline bool irq_fpu_usable(void)
{
	struct pt_regs *regs;

	return !in_interrupt() || !(regs = get_irq_regs()) || \
		user_mode(regs) || (read_cr0() & X86_CR0_TS);
}


static inline int irq_ts_save(void)
{
	
	if (!in_atomic())
		return 0;

	if (read_cr0() & X86_CR0_TS) {
		clts();
		return 1;
	}

	return 0;
}

static inline void irq_ts_restore(int TS_state)
{
	if (TS_state)
		stts();
}

#ifdef CONFIG_X86_64

static inline void save_init_fpu(struct task_struct *tsk)
{
	__save_init_fpu(tsk);
	stts();
}

#define unlazy_fpu	__unlazy_fpu
#define clear_fpu	__clear_fpu

#else  


static inline void save_init_fpu(struct task_struct *tsk)
{
	preempt_disable();
	__save_init_fpu(tsk);
	stts();
	preempt_enable();
}

static inline void unlazy_fpu(struct task_struct *tsk)
{
	preempt_disable();
	__unlazy_fpu(tsk);
	preempt_enable();
}

static inline void clear_fpu(struct task_struct *tsk)
{
	preempt_disable();
	__clear_fpu(tsk);
	preempt_enable();
}

#endif	


static inline unsigned short get_fpu_cwd(struct task_struct *tsk)
{
	if (cpu_has_fxsr) {
		return tsk->thread.xstate->fxsave.cwd;
	} else {
		return (unsigned short)tsk->thread.xstate->fsave.cwd;
	}
}

static inline unsigned short get_fpu_swd(struct task_struct *tsk)
{
	if (cpu_has_fxsr) {
		return tsk->thread.xstate->fxsave.swd;
	} else {
		return (unsigned short)tsk->thread.xstate->fsave.swd;
	}
}

static inline unsigned short get_fpu_mxcsr(struct task_struct *tsk)
{
	if (cpu_has_xmm) {
		return tsk->thread.xstate->fxsave.mxcsr;
	} else {
		return MXCSR_DEFAULT;
	}
}

#endif 
