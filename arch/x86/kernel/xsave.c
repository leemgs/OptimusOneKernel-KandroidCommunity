
#include <linux/bootmem.h>
#include <linux/compat.h>
#include <asm/i387.h>
#ifdef CONFIG_IA32_EMULATION
#include <asm/sigcontext32.h>
#endif
#include <asm/xcr.h>


u64 pcntxt_mask;

struct _fpx_sw_bytes fx_sw_reserved;
#ifdef CONFIG_IA32_EMULATION
struct _fpx_sw_bytes fx_sw_reserved_ia32;
#endif


int check_for_xstate(struct i387_fxsave_struct __user *buf,
		     void __user *fpstate,
		     struct _fpx_sw_bytes *fx_sw_user)
{
	int min_xstate_size = sizeof(struct i387_fxsave_struct) +
			      sizeof(struct xsave_hdr_struct);
	unsigned int magic2;
	int err;

	err = __copy_from_user(fx_sw_user, &buf->sw_reserved[0],
			       sizeof(struct _fpx_sw_bytes));

	if (err)
		return err;

	
	if (fx_sw_user->magic1 != FP_XSTATE_MAGIC1)
		return -1;

	
	if (fx_sw_user->xstate_size < min_xstate_size ||
	    fx_sw_user->xstate_size > xstate_size ||
	    fx_sw_user->xstate_size > fx_sw_user->extended_size)
		return -1;

	err = __get_user(magic2, (__u32 *) (((void *)fpstate) +
					    fx_sw_user->extended_size -
					    FP_XSTATE_MAGIC2_SIZE));
	
	if (err || magic2 != FP_XSTATE_MAGIC2)
		return -1;

	return 0;
}

#ifdef CONFIG_X86_64


int save_i387_xstate(void __user *buf)
{
	struct task_struct *tsk = current;
	int err = 0;

	if (!access_ok(VERIFY_WRITE, buf, sig_xstate_size))
		return -EACCES;

	BUG_ON(sig_xstate_size < xstate_size);

	if ((unsigned long)buf % 64)
		printk("save_i387_xstate: bad fpstate %p\n", buf);

	if (!used_math())
		return 0;

	if (task_thread_info(tsk)->status & TS_USEDFPU) {
		
		err = __clear_user(buf, sig_xstate_size);
		if (err)
			return err;

		if (task_thread_info(tsk)->status & TS_XSAVE)
			err = xsave_user(buf);
		else
			err = fxsave_user(buf);

		if (err)
			return err;
		task_thread_info(tsk)->status &= ~TS_USEDFPU;
		stts();
	} else {
		if (__copy_to_user(buf, &tsk->thread.xstate->fxsave,
				   xstate_size))
			return -1;
	}

	clear_used_math(); 

	if (task_thread_info(tsk)->status & TS_XSAVE) {
		struct _fpstate __user *fx = buf;
		struct _xstate __user *x = buf;
		u64 xstate_bv;

		err = __copy_to_user(&fx->sw_reserved, &fx_sw_reserved,
				     sizeof(struct _fpx_sw_bytes));

		err |= __put_user(FP_XSTATE_MAGIC2,
				  (__u32 __user *) (buf + sig_xstate_size
						    - FP_XSTATE_MAGIC2_SIZE));

		
		err |= __get_user(xstate_bv, &x->xstate_hdr.xstate_bv);

		
		xstate_bv |= XSTATE_FPSSE;

		err |= __put_user(xstate_bv, &x->xstate_hdr.xstate_bv);

		if (err)
			return err;
	}

	return 1;
}


static int restore_user_xstate(void __user *buf)
{
	struct _fpx_sw_bytes fx_sw_user;
	u64 mask;
	int err;

	if (((unsigned long)buf % 64) ||
	     check_for_xstate(buf, buf, &fx_sw_user))
		goto fx_only;

	mask = fx_sw_user.xstate_bv;

	
	err = xrestore_user(buf, mask);
	if (err)
		return err;

	
	mask = pcntxt_mask & ~mask;

	xrstor_state(init_xstate_buf, mask);

	return 0;

fx_only:
	
	xrstor_state(init_xstate_buf, pcntxt_mask & ~XSTATE_FPSSE);
	return fxrstor_checking((__force struct i387_fxsave_struct *)buf);
}


int restore_i387_xstate(void __user *buf)
{
	struct task_struct *tsk = current;
	int err = 0;

	if (!buf) {
		if (used_math())
			goto clear;
		return 0;
	} else
		if (!access_ok(VERIFY_READ, buf, sig_xstate_size))
			return -EACCES;

	if (!used_math()) {
		err = init_fpu(tsk);
		if (err)
			return err;
	}

	if (!(task_thread_info(current)->status & TS_USEDFPU)) {
		clts();
		task_thread_info(current)->status |= TS_USEDFPU;
	}
	if (task_thread_info(tsk)->status & TS_XSAVE)
		err = restore_user_xstate(buf);
	else
		err = fxrstor_checking((__force struct i387_fxsave_struct *)
				       buf);
	if (unlikely(err)) {
		
clear:
		clear_fpu(tsk);
		clear_used_math();
	}
	return err;
}
#endif


static void prepare_fx_sw_frame(void)
{
	int size_extended = (xstate_size - sizeof(struct i387_fxsave_struct)) +
			     FP_XSTATE_MAGIC2_SIZE;

	sig_xstate_size = sizeof(struct _fpstate) + size_extended;

#ifdef CONFIG_IA32_EMULATION
	sig_xstate_ia32_size = sizeof(struct _fpstate_ia32) + size_extended;
#endif

	memset(&fx_sw_reserved, 0, sizeof(fx_sw_reserved));

	fx_sw_reserved.magic1 = FP_XSTATE_MAGIC1;
	fx_sw_reserved.extended_size = sig_xstate_size;
	fx_sw_reserved.xstate_bv = pcntxt_mask;
	fx_sw_reserved.xstate_size = xstate_size;
#ifdef CONFIG_IA32_EMULATION
	memcpy(&fx_sw_reserved_ia32, &fx_sw_reserved,
	       sizeof(struct _fpx_sw_bytes));
	fx_sw_reserved_ia32.extended_size = sig_xstate_ia32_size;
#endif
}


struct xsave_struct *init_xstate_buf;

#ifdef CONFIG_X86_64
unsigned int sig_xstate_size = sizeof(struct _fpstate);
#endif


void __cpuinit xsave_init(void)
{
	if (!cpu_has_xsave)
		return;

	set_in_cr4(X86_CR4_OSXSAVE);

	
	xsetbv(XCR_XFEATURE_ENABLED_MASK, pcntxt_mask);
}


static void __init setup_xstate_init(void)
{
	init_xstate_buf = alloc_bootmem(xstate_size);
	init_xstate_buf->i387.mxcsr = MXCSR_DEFAULT;
}


void __ref xsave_cntxt_init(void)
{
	unsigned int eax, ebx, ecx, edx;

	cpuid_count(0xd, 0, &eax, &ebx, &ecx, &edx);
	pcntxt_mask = eax + ((u64)edx << 32);

	if ((pcntxt_mask & XSTATE_FPSSE) != XSTATE_FPSSE) {
		printk(KERN_ERR "FP/SSE not shown under xsave features 0x%llx\n",
		       pcntxt_mask);
		BUG();
	}

	
	pcntxt_mask = pcntxt_mask & XCNTXT_MASK;
	xsave_init();

	
	cpuid_count(0xd, 0, &eax, &ebx, &ecx, &edx);
	xstate_size = ebx;

	prepare_fx_sw_frame();

	setup_xstate_init();

	printk(KERN_INFO "xsave/xrstor: enabled xstate_bv 0x%llx, "
	       "cntxt size 0x%x\n",
	       pcntxt_mask, xstate_size);
}
