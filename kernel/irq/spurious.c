

#include <linux/jiffies.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/interrupt.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>

static int irqfixup __read_mostly;

#define POLL_SPURIOUS_IRQ_INTERVAL (HZ/10)
static void poll_spurious_irqs(unsigned long dummy);
static DEFINE_TIMER(poll_spurious_irq_timer, poll_spurious_irqs, 0, 0);


static int try_one_irq(int irq, struct irq_desc *desc)
{
	struct irqaction *action;
	int ok = 0, work = 0;

	spin_lock(&desc->lock);
	
	if (desc->status & IRQ_INPROGRESS) {
		
		if (desc->action && (desc->action->flags & IRQF_SHARED))
			desc->status |= IRQ_PENDING;
		spin_unlock(&desc->lock);
		return ok;
	}
	
	desc->status |= IRQ_INPROGRESS;
	action = desc->action;
	spin_unlock(&desc->lock);

	while (action) {
		
		if (action->flags & IRQF_SHARED) {
			if (action->handler(irq, action->dev_id) ==
				IRQ_HANDLED)
				ok = 1;
		}
		action = action->next;
	}
	local_irq_disable();
	
	spin_lock(&desc->lock);
	action = desc->action;

	
	while ((desc->status & IRQ_PENDING) && action) {
		
		work = 1;
		spin_unlock(&desc->lock);
		handle_IRQ_event(irq, action);
		spin_lock(&desc->lock);
		desc->status &= ~IRQ_PENDING;
	}
	desc->status &= ~IRQ_INPROGRESS;
	
	if (work && desc->chip && desc->chip->end)
		desc->chip->end(irq);
	spin_unlock(&desc->lock);

	return ok;
}

static int misrouted_irq(int irq)
{
	struct irq_desc *desc;
	int i, ok = 0;

	for_each_irq_desc(i, desc) {
		if (!i)
			 continue;

		if (i == irq)	
			continue;

		if (try_one_irq(i, desc))
			ok = 1;
	}
	
	return ok;
}

static void poll_all_shared_irqs(void)
{
	struct irq_desc *desc;
	int i;

	for_each_irq_desc(i, desc) {
		unsigned int status;

		if (!i)
			 continue;

		
		status = desc->status;
		barrier();
		if (!(status & IRQ_SPURIOUS_DISABLED))
			continue;

		local_irq_disable();
		try_one_irq(i, desc);
		local_irq_enable();
	}
}

static void poll_spurious_irqs(unsigned long dummy)
{
	poll_all_shared_irqs();

	mod_timer(&poll_spurious_irq_timer,
		  jiffies + POLL_SPURIOUS_IRQ_INTERVAL);
}

#ifdef CONFIG_DEBUG_SHIRQ
void debug_poll_all_shared_irqs(void)
{
	poll_all_shared_irqs();
}
#endif



static void
__report_bad_irq(unsigned int irq, struct irq_desc *desc,
		 irqreturn_t action_ret)
{
	struct irqaction *action;

	if (action_ret != IRQ_HANDLED && action_ret != IRQ_NONE) {
		printk(KERN_ERR "irq event %d: bogus return value %x\n",
				irq, action_ret);
	} else {
		printk(KERN_ERR "irq %d: nobody cared (try booting with "
				"the \"irqpoll\" option)\n", irq);
	}
	dump_stack();
	printk(KERN_ERR "handlers:\n");

	action = desc->action;
	while (action) {
		printk(KERN_ERR "[<%p>]", action->handler);
		print_symbol(" (%s)",
			(unsigned long)action->handler);
		printk("\n");
		action = action->next;
	}
}

static void
report_bad_irq(unsigned int irq, struct irq_desc *desc, irqreturn_t action_ret)
{
	static int count = 100;

	if (count > 0) {
		count--;
		__report_bad_irq(irq, desc, action_ret);
	}
}

static inline int
try_misrouted_irq(unsigned int irq, struct irq_desc *desc,
		  irqreturn_t action_ret)
{
	struct irqaction *action;

	if (!irqfixup)
		return 0;

	
	if (action_ret == IRQ_NONE)
		return 1;

	
	if (irqfixup < 2)
		return 0;

	if (!irq)
		return 1;

	
	action = desc->action;
	barrier();
	return action && (action->flags & IRQF_IRQPOLL);
}

void note_interrupt(unsigned int irq, struct irq_desc *desc,
		    irqreturn_t action_ret)
{
	if (unlikely(action_ret != IRQ_HANDLED)) {
		
		if (time_after(jiffies, desc->last_unhandled + HZ/10))
			desc->irqs_unhandled = 1;
		else
			desc->irqs_unhandled++;
		desc->last_unhandled = jiffies;
		if (unlikely(action_ret != IRQ_NONE))
			report_bad_irq(irq, desc, action_ret);
	}

	if (unlikely(try_misrouted_irq(irq, desc, action_ret))) {
		int ok = misrouted_irq(irq);
		if (action_ret == IRQ_NONE)
			desc->irqs_unhandled -= ok;
	}

	desc->irq_count++;
	if (likely(desc->irq_count < 100000))
		return;

	desc->irq_count = 0;
	if (unlikely(desc->irqs_unhandled > 99900)) {
		
		__report_bad_irq(irq, desc, action_ret);
		
		printk(KERN_EMERG "Disabling IRQ #%d\n", irq);
		desc->status |= IRQ_DISABLED | IRQ_SPURIOUS_DISABLED;
		desc->depth++;
		desc->chip->disable(irq);

		mod_timer(&poll_spurious_irq_timer,
			  jiffies + POLL_SPURIOUS_IRQ_INTERVAL);
	}
	desc->irqs_unhandled = 0;
}

int noirqdebug __read_mostly;

int noirqdebug_setup(char *str)
{
	noirqdebug = 1;
	printk(KERN_INFO "IRQ lockup detection disabled\n");

	return 1;
}

__setup("noirqdebug", noirqdebug_setup);
module_param(noirqdebug, bool, 0644);
MODULE_PARM_DESC(noirqdebug, "Disable irq lockup detection when true");

static int __init irqfixup_setup(char *str)
{
	irqfixup = 1;
	printk(KERN_WARNING "Misrouted IRQ fixup support enabled.\n");
	printk(KERN_WARNING "This may impact system performance.\n");

	return 1;
}

__setup("irqfixup", irqfixup_setup);
module_param(irqfixup, int, 0644);

static int __init irqpoll_setup(char *str)
{
	irqfixup = 2;
	printk(KERN_WARNING "Misrouted IRQ fixup and polling support "
				"enabled\n");
	printk(KERN_WARNING "This may significantly impact system "
				"performance\n");
	return 1;
}

__setup("irqpoll", irqpoll_setup);
