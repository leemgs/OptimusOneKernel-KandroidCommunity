#include <linux/linkage.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/timex.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/kprobes.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/sysdev.h>
#include <linux/bitops.h>
#include <linux/acpi.h>
#include <linux/io.h>
#include <linux/delay.h>

#include <asm/atomic.h>
#include <asm/system.h>
#include <asm/timer.h>
#include <asm/hw_irq.h>
#include <asm/pgtable.h>
#include <asm/desc.h>
#include <asm/apic.h>
#include <asm/setup.h>
#include <asm/i8259.h>
#include <asm/traps.h>





#ifdef CONFIG_X86_32


static irqreturn_t math_error_irq(int cpl, void *dev_id)
{
	outb(0, 0xF0);
	if (ignore_fpu_irq || !boot_cpu_data.hard_math)
		return IRQ_NONE;
	math_error((void __user *)get_irq_regs()->ip);
	return IRQ_HANDLED;
}


static struct irqaction fpu_irq = {
	.handler = math_error_irq,
	.name = "fpu",
};
#endif


static struct irqaction irq2 = {
	.handler = no_action,
	.name = "cascade",
};

DEFINE_PER_CPU(vector_irq_t, vector_irq) = {
	[0 ... IRQ0_VECTOR - 1] = -1,
	[IRQ0_VECTOR] = 0,
	[IRQ1_VECTOR] = 1,
	[IRQ2_VECTOR] = 2,
	[IRQ3_VECTOR] = 3,
	[IRQ4_VECTOR] = 4,
	[IRQ5_VECTOR] = 5,
	[IRQ6_VECTOR] = 6,
	[IRQ7_VECTOR] = 7,
	[IRQ8_VECTOR] = 8,
	[IRQ9_VECTOR] = 9,
	[IRQ10_VECTOR] = 10,
	[IRQ11_VECTOR] = 11,
	[IRQ12_VECTOR] = 12,
	[IRQ13_VECTOR] = 13,
	[IRQ14_VECTOR] = 14,
	[IRQ15_VECTOR] = 15,
	[IRQ15_VECTOR + 1 ... NR_VECTORS - 1] = -1
};

int vector_used_by_percpu_irq(unsigned int vector)
{
	int cpu;

	for_each_online_cpu(cpu) {
		if (per_cpu(vector_irq, cpu)[vector] != -1)
			return 1;
	}

	return 0;
}

void __init init_ISA_irqs(void)
{
	int i;

#if defined(CONFIG_X86_64) || defined(CONFIG_X86_LOCAL_APIC)
	init_bsp_APIC();
#endif
	init_8259A(0);

	
	for (i = 0; i < NR_IRQS_LEGACY; i++) {
		struct irq_desc *desc = irq_to_desc(i);

		desc->status = IRQ_DISABLED;
		desc->action = NULL;
		desc->depth = 1;

		set_irq_chip_and_handler_name(i, &i8259A_chip,
					      handle_level_irq, "XT");
	}
}

void __init init_IRQ(void)
{
	x86_init.irqs.intr_init();
}

static void __init smp_intr_init(void)
{
#ifdef CONFIG_SMP
#if defined(CONFIG_X86_64) || defined(CONFIG_X86_LOCAL_APIC)
	
	alloc_intr_gate(RESCHEDULE_VECTOR, reschedule_interrupt);

	
	alloc_intr_gate(INVALIDATE_TLB_VECTOR_START+0, invalidate_interrupt0);
	alloc_intr_gate(INVALIDATE_TLB_VECTOR_START+1, invalidate_interrupt1);
	alloc_intr_gate(INVALIDATE_TLB_VECTOR_START+2, invalidate_interrupt2);
	alloc_intr_gate(INVALIDATE_TLB_VECTOR_START+3, invalidate_interrupt3);
	alloc_intr_gate(INVALIDATE_TLB_VECTOR_START+4, invalidate_interrupt4);
	alloc_intr_gate(INVALIDATE_TLB_VECTOR_START+5, invalidate_interrupt5);
	alloc_intr_gate(INVALIDATE_TLB_VECTOR_START+6, invalidate_interrupt6);
	alloc_intr_gate(INVALIDATE_TLB_VECTOR_START+7, invalidate_interrupt7);

	
	alloc_intr_gate(CALL_FUNCTION_VECTOR, call_function_interrupt);

	
	alloc_intr_gate(CALL_FUNCTION_SINGLE_VECTOR,
			call_function_single_interrupt);

	
	set_intr_gate(IRQ_MOVE_CLEANUP_VECTOR, irq_move_cleanup_interrupt);
	set_bit(IRQ_MOVE_CLEANUP_VECTOR, used_vectors);

	
	alloc_intr_gate(REBOOT_VECTOR, reboot_interrupt);
#endif
#endif 
}

static void __init apic_intr_init(void)
{
	smp_intr_init();

#ifdef CONFIG_X86_THERMAL_VECTOR
	alloc_intr_gate(THERMAL_APIC_VECTOR, thermal_interrupt);
#endif
#ifdef CONFIG_X86_MCE_THRESHOLD
	alloc_intr_gate(THRESHOLD_APIC_VECTOR, threshold_interrupt);
#endif
#if defined(CONFIG_X86_MCE) && defined(CONFIG_X86_LOCAL_APIC)
	alloc_intr_gate(MCE_SELF_VECTOR, mce_self_interrupt);
#endif

#if defined(CONFIG_X86_64) || defined(CONFIG_X86_LOCAL_APIC)
	
	alloc_intr_gate(LOCAL_TIMER_VECTOR, apic_timer_interrupt);

	
	alloc_intr_gate(GENERIC_INTERRUPT_VECTOR, generic_interrupt);

	
	alloc_intr_gate(SPURIOUS_APIC_VECTOR, spurious_interrupt);
	alloc_intr_gate(ERROR_APIC_VECTOR, error_interrupt);

	
# ifdef CONFIG_PERF_EVENTS
	alloc_intr_gate(LOCAL_PENDING_VECTOR, perf_pending_interrupt);
# endif

#endif
}

void __init native_init_IRQ(void)
{
	int i;

	
	x86_init.irqs.pre_vector_init();

	apic_intr_init();

	
	for (i = FIRST_EXTERNAL_VECTOR; i < NR_VECTORS; i++) {
		
		if (!test_bit(i, used_vectors))
			set_intr_gate(i, interrupt[i-FIRST_EXTERNAL_VECTOR]);
	}

	if (!acpi_ioapic)
		setup_irq(2, &irq2);

#ifdef CONFIG_X86_32
	
	if (boot_cpu_data.hard_math && !cpu_has_fpu)
		setup_irq(FPU_IRQ, &fpu_irq);

	irq_ctx_init(smp_processor_id());
#endif
}
