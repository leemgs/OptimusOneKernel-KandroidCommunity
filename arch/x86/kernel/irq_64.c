

#include <linux/kernel_stat.h>
#include <linux/interrupt.h>
#include <linux/seq_file.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/ftrace.h>
#include <linux/uaccess.h>
#include <linux/smp.h>
#include <asm/io_apic.h>
#include <asm/idle.h>
#include <asm/apic.h>

DEFINE_PER_CPU_SHARED_ALIGNED(irq_cpustat_t, irq_stat);
EXPORT_PER_CPU_SYMBOL(irq_stat);

DEFINE_PER_CPU(struct pt_regs *, irq_regs);
EXPORT_PER_CPU_SYMBOL(irq_regs);


static inline void stack_overflow_check(struct pt_regs *regs)
{
#ifdef CONFIG_DEBUG_STACKOVERFLOW
	u64 curbase = (u64)task_stack_page(current);

	WARN_ONCE(regs->sp >= curbase &&
		  regs->sp <= curbase + THREAD_SIZE &&
		  regs->sp <  curbase + sizeof(struct thread_info) +
					sizeof(struct pt_regs) + 128,

		  "do_IRQ: %s near stack overflow (cur:%Lx,sp:%lx)\n",
			current->comm, curbase, regs->sp);
#endif
}

bool handle_irq(unsigned irq, struct pt_regs *regs)
{
	struct irq_desc *desc;

	stack_overflow_check(regs);

	desc = irq_to_desc(irq);
	if (unlikely(!desc))
		return false;

	generic_handle_irq_desc(irq, desc);
	return true;
}

#ifdef CONFIG_HOTPLUG_CPU

void fixup_irqs(void)
{
	unsigned int irq;
	static int warned;
	struct irq_desc *desc;

	for_each_irq_desc(irq, desc) {
		int break_affinity = 0;
		int set_affinity = 1;
		const struct cpumask *affinity;

		if (!desc)
			continue;
		if (irq == 2)
			continue;

		
		spin_lock(&desc->lock);

		affinity = desc->affinity;
		if (!irq_has_action(irq) ||
		    cpumask_equal(affinity, cpu_online_mask)) {
			spin_unlock(&desc->lock);
			continue;
		}

		if (cpumask_any_and(affinity, cpu_online_mask) >= nr_cpu_ids) {
			break_affinity = 1;
			affinity = cpu_all_mask;
		}

		if (desc->chip->mask)
			desc->chip->mask(irq);

		if (desc->chip->set_affinity)
			desc->chip->set_affinity(irq, affinity);
		else if (!(warned++))
			set_affinity = 0;

		if (desc->chip->unmask)
			desc->chip->unmask(irq);

		spin_unlock(&desc->lock);

		if (break_affinity && set_affinity)
			printk("Broke affinity for irq %i\n", irq);
		else if (!set_affinity)
			printk("Cannot set affinity for irq %i\n", irq);
	}

	
	local_irq_enable();
	mdelay(1);
	local_irq_disable();
}
#endif

extern void call_softirq(void);

asmlinkage void do_softirq(void)
{
	__u32 pending;
	unsigned long flags;

	if (in_interrupt())
		return;

	local_irq_save(flags);
	pending = local_softirq_pending();
	
	if (pending) {
		call_softirq();
		WARN_ON_ONCE(softirq_count());
	}
	local_irq_restore(flags);
}
