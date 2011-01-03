

#include <linux/irq.h>
#include <linux/msi.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>

#include "internals.h"


void dynamic_irq_init(unsigned int irq)
{
	struct irq_desc *desc;
	unsigned long flags;

	desc = irq_to_desc(irq);
	if (!desc) {
		WARN(1, KERN_ERR "Trying to initialize invalid IRQ%d\n", irq);
		return;
	}

	
	spin_lock_irqsave(&desc->lock, flags);
	desc->status = IRQ_DISABLED;
	desc->chip = &no_irq_chip;
	desc->handle_irq = handle_bad_irq;
	desc->depth = 1;
	desc->msi_desc = NULL;
	desc->handler_data = NULL;
	desc->chip_data = NULL;
	desc->action = NULL;
	desc->irq_count = 0;
	desc->irqs_unhandled = 0;
#ifdef CONFIG_SMP
	cpumask_setall(desc->affinity);
#ifdef CONFIG_GENERIC_PENDING_IRQ
	cpumask_clear(desc->pending_mask);
#endif
#endif
	spin_unlock_irqrestore(&desc->lock, flags);
}


void dynamic_irq_cleanup(unsigned int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);
	unsigned long flags;

	if (!desc) {
		WARN(1, KERN_ERR "Trying to cleanup invalid IRQ%d\n", irq);
		return;
	}

	spin_lock_irqsave(&desc->lock, flags);
	if (desc->action) {
		spin_unlock_irqrestore(&desc->lock, flags);
		WARN(1, KERN_ERR "Destroying IRQ%d without calling free_irq\n",
			irq);
		return;
	}
	desc->msi_desc = NULL;
	desc->handler_data = NULL;
	desc->chip_data = NULL;
	desc->handle_irq = handle_bad_irq;
	desc->chip = &no_irq_chip;
	desc->name = NULL;
	clear_kstat_irqs(desc);
	spin_unlock_irqrestore(&desc->lock, flags);
}



int set_irq_chip(unsigned int irq, struct irq_chip *chip)
{
	struct irq_desc *desc = irq_to_desc(irq);
	unsigned long flags;

	if (!desc) {
		WARN(1, KERN_ERR "Trying to install chip for IRQ%d\n", irq);
		return -EINVAL;
	}

	if (!chip)
		chip = &no_irq_chip;

	spin_lock_irqsave(&desc->lock, flags);
	irq_chip_set_defaults(chip);
	desc->chip = chip;
	spin_unlock_irqrestore(&desc->lock, flags);

	return 0;
}
EXPORT_SYMBOL(set_irq_chip);


int set_irq_type(unsigned int irq, unsigned int type)
{
	struct irq_desc *desc = irq_to_desc(irq);
	unsigned long flags;
	int ret = -ENXIO;

	if (!desc) {
		printk(KERN_ERR "Trying to set irq type for IRQ%d\n", irq);
		return -ENODEV;
	}

	type &= IRQ_TYPE_SENSE_MASK;
	if (type == IRQ_TYPE_NONE)
		return 0;

	spin_lock_irqsave(&desc->lock, flags);
	ret = __irq_set_trigger(desc, irq, type);
	spin_unlock_irqrestore(&desc->lock, flags);
	return ret;
}
EXPORT_SYMBOL(set_irq_type);


int set_irq_data(unsigned int irq, void *data)
{
	struct irq_desc *desc = irq_to_desc(irq);
	unsigned long flags;

	if (!desc) {
		printk(KERN_ERR
		       "Trying to install controller data for IRQ%d\n", irq);
		return -EINVAL;
	}

	spin_lock_irqsave(&desc->lock, flags);
	desc->handler_data = data;
	spin_unlock_irqrestore(&desc->lock, flags);
	return 0;
}
EXPORT_SYMBOL(set_irq_data);


int set_irq_msi(unsigned int irq, struct msi_desc *entry)
{
	struct irq_desc *desc = irq_to_desc(irq);
	unsigned long flags;

	if (!desc) {
		printk(KERN_ERR
		       "Trying to install msi data for IRQ%d\n", irq);
		return -EINVAL;
	}

	spin_lock_irqsave(&desc->lock, flags);
	desc->msi_desc = entry;
	if (entry)
		entry->irq = irq;
	spin_unlock_irqrestore(&desc->lock, flags);
	return 0;
}


int set_irq_chip_data(unsigned int irq, void *data)
{
	struct irq_desc *desc = irq_to_desc(irq);
	unsigned long flags;

	if (!desc) {
		printk(KERN_ERR
		       "Trying to install chip data for IRQ%d\n", irq);
		return -EINVAL;
	}

	if (!desc->chip) {
		printk(KERN_ERR "BUG: bad set_irq_chip_data(IRQ#%d)\n", irq);
		return -EINVAL;
	}

	spin_lock_irqsave(&desc->lock, flags);
	desc->chip_data = data;
	spin_unlock_irqrestore(&desc->lock, flags);

	return 0;
}
EXPORT_SYMBOL(set_irq_chip_data);


void set_irq_nested_thread(unsigned int irq, int nest)
{
	struct irq_desc *desc = irq_to_desc(irq);
	unsigned long flags;

	if (!desc)
		return;

	spin_lock_irqsave(&desc->lock, flags);
	if (nest)
		desc->status |= IRQ_NESTED_THREAD;
	else
		desc->status &= ~IRQ_NESTED_THREAD;
	spin_unlock_irqrestore(&desc->lock, flags);
}
EXPORT_SYMBOL_GPL(set_irq_nested_thread);


static void default_enable(unsigned int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);

	desc->chip->unmask(irq);
	desc->status &= ~IRQ_MASKED;
}


static void default_disable(unsigned int irq)
{
}


static unsigned int default_startup(unsigned int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);

	desc->chip->enable(irq);
	return 0;
}


static void default_shutdown(unsigned int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);

	desc->chip->mask(irq);
	desc->status |= IRQ_MASKED;
}


void irq_chip_set_defaults(struct irq_chip *chip)
{
	if (!chip->enable)
		chip->enable = default_enable;
	if (!chip->disable)
		chip->disable = default_disable;
	if (!chip->startup)
		chip->startup = default_startup;
	
	if (!chip->shutdown)
		chip->shutdown = chip->disable != default_disable ?
			chip->disable : default_shutdown;
	if (!chip->name)
		chip->name = chip->typename;
	if (!chip->end)
		chip->end = dummy_irq_chip.end;
}

static inline void mask_ack_irq(struct irq_desc *desc, int irq)
{
	if (desc->chip->mask_ack)
		desc->chip->mask_ack(irq);
	else {
		desc->chip->mask(irq);
		if (desc->chip->ack)
			desc->chip->ack(irq);
	}
}


void handle_nested_irq(unsigned int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);
	struct irqaction *action;
	irqreturn_t action_ret;

	might_sleep();

	spin_lock_irq(&desc->lock);

	kstat_incr_irqs_this_cpu(irq, desc);

	action = desc->action;
	if (unlikely(!action || (desc->status & IRQ_DISABLED)))
		goto out_unlock;

	desc->status |= IRQ_INPROGRESS;
	spin_unlock_irq(&desc->lock);

	action_ret = action->thread_fn(action->irq, action->dev_id);
	if (!noirqdebug)
		note_interrupt(irq, desc, action_ret);

	spin_lock_irq(&desc->lock);
	desc->status &= ~IRQ_INPROGRESS;

out_unlock:
	spin_unlock_irq(&desc->lock);
}
EXPORT_SYMBOL_GPL(handle_nested_irq);


void
handle_simple_irq(unsigned int irq, struct irq_desc *desc)
{
	struct irqaction *action;
	irqreturn_t action_ret;

	spin_lock(&desc->lock);

	if (unlikely(desc->status & IRQ_INPROGRESS))
		goto out_unlock;
	desc->status &= ~(IRQ_REPLAY | IRQ_WAITING);
	kstat_incr_irqs_this_cpu(irq, desc);

	action = desc->action;
	if (unlikely(!action || (desc->status & IRQ_DISABLED)))
		goto out_unlock;

	desc->status |= IRQ_INPROGRESS;
	spin_unlock(&desc->lock);

	action_ret = handle_IRQ_event(irq, action);
	if (!noirqdebug)
		note_interrupt(irq, desc, action_ret);

	spin_lock(&desc->lock);
	desc->status &= ~IRQ_INPROGRESS;
out_unlock:
	spin_unlock(&desc->lock);
}


void
handle_level_irq(unsigned int irq, struct irq_desc *desc)
{
	struct irqaction *action;
	irqreturn_t action_ret;

	spin_lock(&desc->lock);
	mask_ack_irq(desc, irq);

	if (unlikely(desc->status & IRQ_INPROGRESS))
		goto out_unlock;
	desc->status &= ~(IRQ_REPLAY | IRQ_WAITING);
	kstat_incr_irqs_this_cpu(irq, desc);

	
	action = desc->action;
	if (unlikely(!action || (desc->status & IRQ_DISABLED)))
		goto out_unlock;

	desc->status |= IRQ_INPROGRESS;
	spin_unlock(&desc->lock);

	action_ret = handle_IRQ_event(irq, action);
	if (!noirqdebug)
		note_interrupt(irq, desc, action_ret);

	spin_lock(&desc->lock);
	desc->status &= ~IRQ_INPROGRESS;

	if (unlikely(desc->status & IRQ_ONESHOT))
		desc->status |= IRQ_MASKED;
	else if (!(desc->status & IRQ_DISABLED) && desc->chip->unmask)
		desc->chip->unmask(irq);
out_unlock:
	spin_unlock(&desc->lock);
}
EXPORT_SYMBOL_GPL(handle_level_irq);


void
handle_fasteoi_irq(unsigned int irq, struct irq_desc *desc)
{
	struct irqaction *action;
	irqreturn_t action_ret;

	spin_lock(&desc->lock);

	if (unlikely(desc->status & IRQ_INPROGRESS))
		goto out;

	desc->status &= ~(IRQ_REPLAY | IRQ_WAITING);
	kstat_incr_irqs_this_cpu(irq, desc);

	
	action = desc->action;
	if (unlikely(!action || (desc->status & IRQ_DISABLED))) {
		desc->status |= IRQ_PENDING;
		if (desc->chip->mask)
			desc->chip->mask(irq);
		goto out;
	}

	desc->status |= IRQ_INPROGRESS;
	desc->status &= ~IRQ_PENDING;
	spin_unlock(&desc->lock);

	action_ret = handle_IRQ_event(irq, action);
	if (!noirqdebug)
		note_interrupt(irq, desc, action_ret);

	spin_lock(&desc->lock);
	desc->status &= ~IRQ_INPROGRESS;
out:
	desc->chip->eoi(irq);

	spin_unlock(&desc->lock);
}


void
handle_edge_irq(unsigned int irq, struct irq_desc *desc)
{
	spin_lock(&desc->lock);

	desc->status &= ~(IRQ_REPLAY | IRQ_WAITING);

	
	if (unlikely((desc->status & (IRQ_INPROGRESS | IRQ_DISABLED)) ||
		    !desc->action)) {
		desc->status |= (IRQ_PENDING | IRQ_MASKED);
		mask_ack_irq(desc, irq);
		goto out_unlock;
	}
	kstat_incr_irqs_this_cpu(irq, desc);

	
	if (desc->chip->ack)
		desc->chip->ack(irq);

	
	desc->status |= IRQ_INPROGRESS;

	do {
		struct irqaction *action = desc->action;
		irqreturn_t action_ret;

		if (unlikely(!action)) {
			desc->chip->mask(irq);
			goto out_unlock;
		}

		
		if (unlikely((desc->status &
			       (IRQ_PENDING | IRQ_MASKED | IRQ_DISABLED)) ==
			      (IRQ_PENDING | IRQ_MASKED))) {
			desc->chip->unmask(irq);
			desc->status &= ~IRQ_MASKED;
		}

		desc->status &= ~IRQ_PENDING;
		spin_unlock(&desc->lock);
		action_ret = handle_IRQ_event(irq, action);
		if (!noirqdebug)
			note_interrupt(irq, desc, action_ret);
		spin_lock(&desc->lock);

	} while ((desc->status & (IRQ_PENDING | IRQ_DISABLED)) == IRQ_PENDING);

	desc->status &= ~IRQ_INPROGRESS;
out_unlock:
	spin_unlock(&desc->lock);
}


void
handle_percpu_irq(unsigned int irq, struct irq_desc *desc)
{
	irqreturn_t action_ret;

	kstat_incr_irqs_this_cpu(irq, desc);

	if (desc->chip->ack)
		desc->chip->ack(irq);

	action_ret = handle_IRQ_event(irq, desc->action);
	if (!noirqdebug)
		note_interrupt(irq, desc, action_ret);

	if (desc->chip->eoi)
		desc->chip->eoi(irq);
}

void
__set_irq_handler(unsigned int irq, irq_flow_handler_t handle, int is_chained,
		  const char *name)
{
	struct irq_desc *desc = irq_to_desc(irq);
	unsigned long flags;

	if (!desc) {
		printk(KERN_ERR
		       "Trying to install type control for IRQ%d\n", irq);
		return;
	}

	if (!handle)
		handle = handle_bad_irq;
	else if (desc->chip == &no_irq_chip) {
		printk(KERN_WARNING "Trying to install %sinterrupt handler "
		       "for IRQ%d\n", is_chained ? "chained " : "", irq);
		
		desc->chip = &dummy_irq_chip;
	}

	chip_bus_lock(irq, desc);
	spin_lock_irqsave(&desc->lock, flags);

	
	if (handle == handle_bad_irq) {
		if (desc->chip != &no_irq_chip)
			mask_ack_irq(desc, irq);
		desc->status |= IRQ_DISABLED;
		desc->depth = 1;
	}
	desc->handle_irq = handle;
	desc->name = name;

	if (handle != handle_bad_irq && is_chained) {
		desc->status &= ~IRQ_DISABLED;
		desc->status |= IRQ_NOREQUEST | IRQ_NOPROBE;
		desc->depth = 0;
		desc->chip->startup(irq);
	}
	spin_unlock_irqrestore(&desc->lock, flags);
	chip_bus_sync_unlock(irq, desc);
}
EXPORT_SYMBOL_GPL(__set_irq_handler);

void
set_irq_chip_and_handler(unsigned int irq, struct irq_chip *chip,
			 irq_flow_handler_t handle)
{
	set_irq_chip(irq, chip);
	__set_irq_handler(irq, handle, 0, NULL);
}

void
set_irq_chip_and_handler_name(unsigned int irq, struct irq_chip *chip,
			      irq_flow_handler_t handle, const char *name)
{
	set_irq_chip(irq, chip);
	__set_irq_handler(irq, handle, 0, name);
}

void __init set_irq_noprobe(unsigned int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);
	unsigned long flags;

	if (!desc) {
		printk(KERN_ERR "Trying to mark IRQ%d non-probeable\n", irq);
		return;
	}

	spin_lock_irqsave(&desc->lock, flags);
	desc->status |= IRQ_NOPROBE;
	spin_unlock_irqrestore(&desc->lock, flags);
}

void __init set_irq_probe(unsigned int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);
	unsigned long flags;

	if (!desc) {
		printk(KERN_ERR "Trying to mark IRQ%d probeable\n", irq);
		return;
	}

	spin_lock_irqsave(&desc->lock, flags);
	desc->status &= ~IRQ_NOPROBE;
	spin_unlock_irqrestore(&desc->lock, flags);
}
