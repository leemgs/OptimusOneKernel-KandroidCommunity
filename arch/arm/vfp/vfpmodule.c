
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/init.h>

#include <asm/thread_notify.h>
#include <asm/vfp.h>

#include "vfpinstr.h"
#include "vfp.h"


void vfp_testing_entry(void);
void vfp_support_entry(void);
void vfp_null_entry(void);

void (*vfp_vector)(void) = vfp_null_entry;
union vfp_state *last_VFP_context[NR_CPUS];


unsigned int VFP_arch;


static void vfp_thread_flush(struct thread_info *thread)
{
	union vfp_state *vfp = &thread->vfpstate;
	unsigned int cpu;

	memset(vfp, 0, sizeof(union vfp_state));

	vfp->hard.fpexc = FPEXC_EN;
	vfp->hard.fpscr = FPSCR_ROUND_NEAREST;

	
	cpu = get_cpu();
	if (last_VFP_context[cpu] == vfp)
		last_VFP_context[cpu] = NULL;
	fmxr(FPEXC, fmrx(FPEXC) & ~FPEXC_EN);
	put_cpu();
}

static void vfp_thread_exit(struct thread_info *thread)
{
	
	union vfp_state *vfp = &thread->vfpstate;
	unsigned int cpu = get_cpu();

	if (last_VFP_context[cpu] == vfp)
		last_VFP_context[cpu] = NULL;
	put_cpu();
}


static int vfp_notifier(struct notifier_block *self, unsigned long cmd, void *v)
{
	struct thread_info *thread = v;

	if (likely(cmd == THREAD_NOTIFY_SWITCH)) {
		u32 fpexc = fmrx(FPEXC);

#ifdef CONFIG_SMP
		unsigned int cpu = thread->cpu;

		
		if ((fpexc & FPEXC_EN) && last_VFP_context[cpu]) {
			vfp_save_state(last_VFP_context[cpu], fpexc);
			last_VFP_context[cpu]->hard.cpu = cpu;
		}
		
		if (thread->vfpstate.hard.cpu != cpu)
			last_VFP_context[cpu] = NULL;
#endif

		
		fmxr(FPEXC, fpexc & ~FPEXC_EN);
		return NOTIFY_DONE;
	}

	if (cmd == THREAD_NOTIFY_FLUSH)
		vfp_thread_flush(thread);
	else
		vfp_thread_exit(thread);

	return NOTIFY_DONE;
}

static struct notifier_block vfp_notifier_block = {
	.notifier_call	= vfp_notifier,
};


void vfp_raise_sigfpe(unsigned int sicode, struct pt_regs *regs)
{
	siginfo_t info;

	memset(&info, 0, sizeof(info));

	info.si_signo = SIGFPE;
	info.si_code = sicode;
	info.si_addr = (void __user *)(instruction_pointer(regs) - 4);

	
	current->thread.error_code = 0;
	current->thread.trap_no = 6;

	send_sig_info(SIGFPE, &info, current);
}

static void vfp_panic(char *reason, u32 inst)
{
	int i;

	printk(KERN_ERR "VFP: Error: %s\n", reason);
	printk(KERN_ERR "VFP: EXC 0x%08x SCR 0x%08x INST 0x%08x\n",
		fmrx(FPEXC), fmrx(FPSCR), inst);
	for (i = 0; i < 32; i += 2)
		printk(KERN_ERR "VFP: s%2u: 0x%08x s%2u: 0x%08x\n",
		       i, vfp_get_float(i), i+1, vfp_get_float(i+1));
}


static void vfp_raise_exceptions(u32 exceptions, u32 inst, u32 fpscr, struct pt_regs *regs)
{
	int si_code = 0;

	pr_debug("VFP: raising exceptions %08x\n", exceptions);

	if (exceptions == VFP_EXCEPTION_ERROR) {
		vfp_panic("unhandled bounce", inst);
		vfp_raise_sigfpe(0, regs);
		return;
	}

	
	
	if (exceptions & (FPSCR_N | FPSCR_Z | FPSCR_C | FPSCR_V))
		fpscr &= ~(FPSCR_N | FPSCR_Z | FPSCR_C | FPSCR_V);

	fpscr |= exceptions;

	fmxr(FPSCR, fpscr);

#define RAISE(stat,en,sig)				\
	if (exceptions & stat && fpscr & en)		\
		si_code = sig;

	
	RAISE(FPSCR_DZC, FPSCR_DZE, FPE_FLTDIV);
	RAISE(FPSCR_IXC, FPSCR_IXE, FPE_FLTRES);
	RAISE(FPSCR_UFC, FPSCR_UFE, FPE_FLTUND);
	RAISE(FPSCR_OFC, FPSCR_OFE, FPE_FLTOVF);
	RAISE(FPSCR_IOC, FPSCR_IOE, FPE_FLTINV);

	if (si_code)
		vfp_raise_sigfpe(si_code, regs);
}


static u32 vfp_emulate_instruction(u32 inst, u32 fpscr, struct pt_regs *regs)
{
	u32 exceptions = VFP_EXCEPTION_ERROR;

	pr_debug("VFP: emulate: INST=0x%08x SCR=0x%08x\n", inst, fpscr);

	if (INST_CPRTDO(inst)) {
		if (!INST_CPRT(inst)) {
			
			if (vfp_single(inst)) {
				exceptions = vfp_single_cpdo(inst, fpscr);
			} else {
				exceptions = vfp_double_cpdo(inst, fpscr);
			}
		} else {
			
		}
	} else {
		
	}
	return exceptions & ~VFP_NAN_FLAG;
}


void VFP_bounce(u32 trigger, u32 fpexc, struct pt_regs *regs)
{
	u32 fpscr, orig_fpscr, fpsid, exceptions;

	pr_debug("VFP: bounce: trigger %08x fpexc %08x\n", trigger, fpexc);

	
	fmxr(FPEXC, fpexc & ~(FPEXC_EX|FPEXC_DEX|FPEXC_FP2V|FPEXC_VV|FPEXC_TRAP_MASK));

	fpsid = fmrx(FPSID);
	orig_fpscr = fpscr = fmrx(FPSCR);

	
	if ((fpsid & FPSID_ARCH_MASK) == (1 << FPSID_ARCH_BIT)
	    && (fpscr & FPSCR_IXE)) {
		
		goto emulate;
	}

	if (fpexc & FPEXC_EX) {
#ifndef CONFIG_CPU_FEROCEON
		
		trigger = fmrx(FPINST);
		regs->ARM_pc -= 4;
#endif
	} else if (!(fpexc & FPEXC_DEX)) {
		
		 vfp_raise_exceptions(VFP_EXCEPTION_ERROR, trigger, fpscr, regs);
		goto exit;
	}

	
	if (fpexc & (FPEXC_EX | FPEXC_VV)) {
		u32 len;

		len = fpexc + (1 << FPEXC_LENGTH_BIT);

		fpscr &= ~FPSCR_LENGTH_MASK;
		fpscr |= (len & FPEXC_LENGTH_MASK) << (FPSCR_LENGTH_BIT - FPEXC_LENGTH_BIT);
	}

	
	exceptions = vfp_emulate_instruction(trigger, fpscr, regs);
	if (exceptions)
		vfp_raise_exceptions(exceptions, trigger, orig_fpscr, regs);

	
	if (fpexc ^ (FPEXC_EX | FPEXC_FP2V))
		goto exit;

	
	barrier();
	trigger = fmrx(FPINST2);

 emulate:
	exceptions = vfp_emulate_instruction(trigger, orig_fpscr, regs);
	if (exceptions)
		vfp_raise_exceptions(exceptions, trigger, orig_fpscr, regs);
 exit:
	preempt_enable();
}

static void vfp_enable(void *unused)
{
	u32 access = get_copro_access();

	
	set_copro_access(access | CPACC_FULL(10) | CPACC_FULL(11));
}

int vfp_flush_context(void)
{
	unsigned long flags;
	struct thread_info *ti;
	u32 fpexc;
	u32 cpu;
	int saved = 0;

	local_irq_save(flags);

	ti = current_thread_info();
	fpexc = fmrx(FPEXC);
	cpu = ti->cpu;

#ifdef CONFIG_SMP
	
	if ((fpexc & FPEXC_EN) && last_VFP_context[cpu]) {
		last_VFP_context[cpu]->hard.cpu = cpu;
#else
	
	if (last_VFP_context[cpu]) {
		
		fmxr(FPEXC, fpexc | FPEXC_EN);
		isb();
#endif
		vfp_save_state(last_VFP_context[cpu], fpexc);

		
		fmxr(FPEXC, fmrx(FPEXC) & ~FPEXC_EN);
		saved = 1;
	}
	last_VFP_context[cpu] = NULL;

	local_irq_restore(flags);

	return saved;
}

void vfp_reinit(void)
{
	
	vfp_enable(NULL);

	
	fmxr(FPEXC, fmrx(FPEXC) & ~FPEXC_EN);
}

#ifdef CONFIG_PM
#include <linux/sysdev.h>

static int vfp_pm_suspend(struct sys_device *dev, pm_message_t state)
{
	struct thread_info *ti = current_thread_info();
	u32 fpexc = fmrx(FPEXC);

	
	if (fpexc & FPEXC_EN) {
		printk(KERN_DEBUG "%s: saving vfp state\n", __func__);
		vfp_save_state(&ti->vfpstate, fpexc);

		
		fmxr(FPEXC, fmrx(FPEXC) & ~FPEXC_EN);
	}

	
	memset(last_VFP_context, 0, sizeof(last_VFP_context));

	return 0;
}

static int vfp_pm_resume(struct sys_device *dev)
{
	
	vfp_enable(NULL);

	
	fmxr(FPEXC, fmrx(FPEXC) & ~FPEXC_EN);

	return 0;
}

static struct sysdev_class vfp_pm_sysclass = {
	.name		= "vfp",
	.suspend	= vfp_pm_suspend,
	.resume		= vfp_pm_resume,
};

static struct sys_device vfp_pm_sysdev = {
	.cls	= &vfp_pm_sysclass,
};

static void vfp_pm_init(void)
{
	sysdev_class_register(&vfp_pm_sysclass);
	sysdev_register(&vfp_pm_sysdev);
}


#else
static inline void vfp_pm_init(void) { }
#endif 


#ifdef CONFIG_SMP
void vfp_sync_state(struct thread_info *thread)
{
	
	thread->vfpstate.hard.cpu = NR_CPUS;
}
#else
void vfp_sync_state(struct thread_info *thread)
{
	unsigned int cpu = get_cpu();
	u32 fpexc = fmrx(FPEXC);

	
	if (fpexc & FPEXC_EN)
		goto out;

	if (!last_VFP_context[cpu])
		goto out;

	
	fmxr(FPEXC, fpexc | FPEXC_EN);
	vfp_save_state(last_VFP_context[cpu], fpexc);
	fmxr(FPEXC, fpexc);

	
	last_VFP_context[cpu] = NULL;

out:
	put_cpu();
}
#endif

#include <linux/smp.h>


static int __init vfp_init(void)
{
	unsigned int vfpsid;
	unsigned int cpu_arch = cpu_architecture();

	if (cpu_arch >= CPU_ARCH_ARMv6)
		vfp_enable(NULL);

	
	vfp_vector = vfp_testing_entry;
	barrier();
	vfpsid = fmrx(FPSID);
	barrier();
	vfp_vector = vfp_null_entry;

	printk(KERN_INFO "VFP support v0.3: ");
	if (VFP_arch)
		printk("not present\n");
	else if (vfpsid & FPSID_NODOUBLE) {
		printk("no double precision support\n");
	} else {
		smp_call_function(vfp_enable, NULL, 1);

		VFP_arch = (vfpsid & FPSID_ARCH_MASK) >> FPSID_ARCH_BIT;  
		printk("implementor %02x architecture %d part %02x variant %x rev %x\n",
			(vfpsid & FPSID_IMPLEMENTER_MASK) >> FPSID_IMPLEMENTER_BIT,
			(vfpsid & FPSID_ARCH_MASK) >> FPSID_ARCH_BIT,
			(vfpsid & FPSID_PART_MASK) >> FPSID_PART_BIT,
			(vfpsid & FPSID_VARIANT_MASK) >> FPSID_VARIANT_BIT,
			(vfpsid & FPSID_REV_MASK) >> FPSID_REV_BIT);

		vfp_vector = vfp_support_entry;

		thread_register_notifier(&vfp_notifier_block);
		vfp_pm_init();

		
		elf_hwcap |= HWCAP_VFP;
#ifdef CONFIG_VFPv3
		if (VFP_arch >= 3) {
			elf_hwcap |= HWCAP_VFPv3;

			
			if (((fmrx(MVFR0) & MVFR0_A_SIMD_MASK)) == 1)
				elf_hwcap |= HWCAP_VFPv3D16;
		}
#endif
#ifdef CONFIG_NEON
		
		if ((fmrx(MVFR1) & 0x000fff00) == 0x00011100)
			elf_hwcap |= HWCAP_NEON;
#endif
	}
	return 0;
}

late_initcall(vfp_init);
