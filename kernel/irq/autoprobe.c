

#include <linux/irq.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/async.h>

#include "internals.h"


static DEFINE_MUTEX(probing_active);


unsigned long probe_irq_on(void)
{
	struct irq_desc *desc;
	unsigned long mask = 0;
	unsigned int status;
	int i;

	
	async_synchronize_full();
	mutex_lock(&probing_active);
	
	for_each_irq_desc_reverse(i, desc) {
		spin_lock_irq(&desc->lock);
		if (!desc->action && !(desc->status & IRQ_NOPROBE)) {
			
			compat_irq_chip_set_default_handler(desc);

			
			if (desc->chip->set_type)
				desc->chip->set_type(i, IRQ_TYPE_PROBE);
			desc->chip->startup(i);
		}
		spin_unlock_irq(&desc->lock);
	}

	
	msleep(20);

	
	for_each_irq_desc_reverse(i, desc) {
		spin_lock_irq(&desc->lock);
		if (!desc->action && !(desc->status & IRQ_NOPROBE)) {
			desc->status |= IRQ_AUTODETECT | IRQ_WAITING;
			if (desc->chip->startup(i))
				desc->status |= IRQ_PENDING;
		}
		spin_unlock_irq(&desc->lock);
	}

	
	msleep(100);

	
	for_each_irq_desc(i, desc) {
		spin_lock_irq(&desc->lock);
		status = desc->status;

		if (status & IRQ_AUTODETECT) {
			
			if (!(status & IRQ_WAITING)) {
				desc->status = status & ~IRQ_AUTODETECT;
				desc->chip->shutdown(i);
			} else
				if (i < 32)
					mask |= 1 << i;
		}
		spin_unlock_irq(&desc->lock);
	}

	return mask;
}
EXPORT_SYMBOL(probe_irq_on);


unsigned int probe_irq_mask(unsigned long val)
{
	unsigned int status, mask = 0;
	struct irq_desc *desc;
	int i;

	for_each_irq_desc(i, desc) {
		spin_lock_irq(&desc->lock);
		status = desc->status;

		if (status & IRQ_AUTODETECT) {
			if (i < 16 && !(status & IRQ_WAITING))
				mask |= 1 << i;

			desc->status = status & ~IRQ_AUTODETECT;
			desc->chip->shutdown(i);
		}
		spin_unlock_irq(&desc->lock);
	}
	mutex_unlock(&probing_active);

	return mask & val;
}
EXPORT_SYMBOL(probe_irq_mask);


int probe_irq_off(unsigned long val)
{
	int i, irq_found = 0, nr_of_irqs = 0;
	struct irq_desc *desc;
	unsigned int status;

	for_each_irq_desc(i, desc) {
		spin_lock_irq(&desc->lock);
		status = desc->status;

		if (status & IRQ_AUTODETECT) {
			if (!(status & IRQ_WAITING)) {
				if (!nr_of_irqs)
					irq_found = i;
				nr_of_irqs++;
			}
			desc->status = status & ~IRQ_AUTODETECT;
			desc->chip->shutdown(i);
		}
		spin_unlock_irq(&desc->lock);
	}
	mutex_unlock(&probing_active);

	if (nr_of_irqs > 1)
		irq_found = -irq_found;

	return irq_found;
}
EXPORT_SYMBOL(probe_irq_off);

