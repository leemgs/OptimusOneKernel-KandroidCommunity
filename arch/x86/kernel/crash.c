

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/smp.h>
#include <linux/reboot.h>
#include <linux/kexec.h>
#include <linux/delay.h>
#include <linux/elf.h>
#include <linux/elfcore.h>

#include <asm/processor.h>
#include <asm/hardirq.h>
#include <asm/nmi.h>
#include <asm/hw_irq.h>
#include <asm/apic.h>
#include <asm/hpet.h>
#include <linux/kdebug.h>
#include <asm/cpu.h>
#include <asm/reboot.h>
#include <asm/virtext.h>
#include <asm/iommu.h>


#if defined(CONFIG_SMP) && defined(CONFIG_X86_LOCAL_APIC)

static void kdump_nmi_callback(int cpu, struct die_args *args)
{
	struct pt_regs *regs;
#ifdef CONFIG_X86_32
	struct pt_regs fixed_regs;
#endif

	regs = args->regs;

#ifdef CONFIG_X86_32
	if (!user_mode_vm(regs)) {
		crash_fixup_ss_esp(&fixed_regs, regs);
		regs = &fixed_regs;
	}
#endif
	crash_save_cpu(regs, cpu);

	
	cpu_emergency_vmxoff();
	cpu_emergency_svm_disable();

	disable_local_APIC();
}

static void kdump_nmi_shootdown_cpus(void)
{
	nmi_shootdown_cpus(kdump_nmi_callback);

	disable_local_APIC();
}

#else
static void kdump_nmi_shootdown_cpus(void)
{
	
}
#endif

void native_machine_crash_shutdown(struct pt_regs *regs)
{
	
	
	local_irq_disable();

	kdump_nmi_shootdown_cpus();

	
	cpu_emergency_vmxoff();
	cpu_emergency_svm_disable();

	lapic_shutdown();
#if defined(CONFIG_X86_IO_APIC)
	disable_IO_APIC();
#endif
#ifdef CONFIG_HPET_TIMER
	hpet_disable();
#endif

#ifdef CONFIG_X86_64
	pci_iommu_shutdown();
#endif

	crash_save_cpu(regs, safe_smp_processor_id());
}
