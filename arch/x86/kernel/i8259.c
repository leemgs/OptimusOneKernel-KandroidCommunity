#include <linux/linkage.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/timex.h>
#include <linux/slab.h>
#include <linux/random.h>
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
#include <asm/i8259.h>



static int i8259A_auto_eoi;
DEFINE_SPINLOCK(i8259A_lock);
static void mask_and_ack_8259A(unsigned int);

struct irq_chip i8259A_chip = {
	.name		= "XT-PIC",
	.mask		= disable_8259A_irq,
	.disable	= disable_8259A_irq,
	.unmask		= enable_8259A_irq,
	.mask_ack	= mask_and_ack_8259A,
};




unsigned int cached_irq_mask = 0xffff;


unsigned long io_apic_irqs;

void disable_8259A_irq(unsigned int irq)
{
	unsigned int mask = 1 << irq;
	unsigned long flags;

	spin_lock_irqsave(&i8259A_lock, flags);
	cached_irq_mask |= mask;
	if (irq & 8)
		outb(cached_slave_mask, PIC_SLAVE_IMR);
	else
		outb(cached_master_mask, PIC_MASTER_IMR);
	spin_unlock_irqrestore(&i8259A_lock, flags);
}

void enable_8259A_irq(unsigned int irq)
{
	unsigned int mask = ~(1 << irq);
	unsigned long flags;

	spin_lock_irqsave(&i8259A_lock, flags);
	cached_irq_mask &= mask;
	if (irq & 8)
		outb(cached_slave_mask, PIC_SLAVE_IMR);
	else
		outb(cached_master_mask, PIC_MASTER_IMR);
	spin_unlock_irqrestore(&i8259A_lock, flags);
}

int i8259A_irq_pending(unsigned int irq)
{
	unsigned int mask = 1<<irq;
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&i8259A_lock, flags);
	if (irq < 8)
		ret = inb(PIC_MASTER_CMD) & mask;
	else
		ret = inb(PIC_SLAVE_CMD) & (mask >> 8);
	spin_unlock_irqrestore(&i8259A_lock, flags);

	return ret;
}

void make_8259A_irq(unsigned int irq)
{
	disable_irq_nosync(irq);
	io_apic_irqs &= ~(1<<irq);
	set_irq_chip_and_handler_name(irq, &i8259A_chip, handle_level_irq,
				      "XT");
	enable_irq(irq);
}


static inline int i8259A_irq_real(unsigned int irq)
{
	int value;
	int irqmask = 1<<irq;

	if (irq < 8) {
		outb(0x0B, PIC_MASTER_CMD);	
		value = inb(PIC_MASTER_CMD) & irqmask;
		outb(0x0A, PIC_MASTER_CMD);	
		return value;
	}
	outb(0x0B, PIC_SLAVE_CMD);	
	value = inb(PIC_SLAVE_CMD) & (irqmask >> 8);
	outb(0x0A, PIC_SLAVE_CMD);	
	return value;
}


static void mask_and_ack_8259A(unsigned int irq)
{
	unsigned int irqmask = 1 << irq;
	unsigned long flags;

	spin_lock_irqsave(&i8259A_lock, flags);
	
	if (cached_irq_mask & irqmask)
		goto spurious_8259A_irq;
	cached_irq_mask |= irqmask;

handle_real_irq:
	if (irq & 8) {
		inb(PIC_SLAVE_IMR);	
		outb(cached_slave_mask, PIC_SLAVE_IMR);
		
		outb(0x60+(irq&7), PIC_SLAVE_CMD);
		 
		outb(0x60+PIC_CASCADE_IR, PIC_MASTER_CMD);
	} else {
		inb(PIC_MASTER_IMR);	
		outb(cached_master_mask, PIC_MASTER_IMR);
		outb(0x60+irq, PIC_MASTER_CMD);	
	}
	spin_unlock_irqrestore(&i8259A_lock, flags);
	return;

spurious_8259A_irq:
	
	if (i8259A_irq_real(irq))
		
		goto handle_real_irq;

	{
		static int spurious_irq_mask;
		
		if (!(spurious_irq_mask & irqmask)) {
			printk(KERN_DEBUG
			       "spurious 8259A interrupt: IRQ%d.\n", irq);
			spurious_irq_mask |= irqmask;
		}
		atomic_inc(&irq_err_count);
		
		goto handle_real_irq;
	}
}

static char irq_trigger[2];

static void restore_ELCR(char *trigger)
{
	outb(trigger[0], 0x4d0);
	outb(trigger[1], 0x4d1);
}

static void save_ELCR(char *trigger)
{
	
	trigger[0] = inb(0x4d0) & 0xF8;
	trigger[1] = inb(0x4d1) & 0xDE;
}

static int i8259A_resume(struct sys_device *dev)
{
	init_8259A(i8259A_auto_eoi);
	restore_ELCR(irq_trigger);
	return 0;
}

static int i8259A_suspend(struct sys_device *dev, pm_message_t state)
{
	save_ELCR(irq_trigger);
	return 0;
}

static int i8259A_shutdown(struct sys_device *dev)
{
	
	outb(0xff, PIC_MASTER_IMR);	
	outb(0xff, PIC_SLAVE_IMR);	
	return 0;
}

static struct sysdev_class i8259_sysdev_class = {
	.name = "i8259",
	.suspend = i8259A_suspend,
	.resume = i8259A_resume,
	.shutdown = i8259A_shutdown,
};

static struct sys_device device_i8259A = {
	.id	= 0,
	.cls	= &i8259_sysdev_class,
};

static int __init i8259A_init_sysfs(void)
{
	int error = sysdev_class_register(&i8259_sysdev_class);
	if (!error)
		error = sysdev_register(&device_i8259A);
	return error;
}

device_initcall(i8259A_init_sysfs);

void mask_8259A(void)
{
	unsigned long flags;

	spin_lock_irqsave(&i8259A_lock, flags);

	outb(0xff, PIC_MASTER_IMR);	
	outb(0xff, PIC_SLAVE_IMR);	

	spin_unlock_irqrestore(&i8259A_lock, flags);
}

void unmask_8259A(void)
{
	unsigned long flags;

	spin_lock_irqsave(&i8259A_lock, flags);

	outb(cached_master_mask, PIC_MASTER_IMR); 
	outb(cached_slave_mask, PIC_SLAVE_IMR);	  

	spin_unlock_irqrestore(&i8259A_lock, flags);
}

void init_8259A(int auto_eoi)
{
	unsigned long flags;

	i8259A_auto_eoi = auto_eoi;

	spin_lock_irqsave(&i8259A_lock, flags);

	outb(0xff, PIC_MASTER_IMR);	
	outb(0xff, PIC_SLAVE_IMR);	

	
	outb_pic(0x11, PIC_MASTER_CMD);	

	
	outb_pic(IRQ0_VECTOR, PIC_MASTER_IMR);

	
	outb_pic(1U << PIC_CASCADE_IR, PIC_MASTER_IMR);

	if (auto_eoi)	
		outb_pic(MASTER_ICW4_DEFAULT | PIC_ICW4_AEOI, PIC_MASTER_IMR);
	else		
		outb_pic(MASTER_ICW4_DEFAULT, PIC_MASTER_IMR);

	outb_pic(0x11, PIC_SLAVE_CMD);	

	
	outb_pic(IRQ8_VECTOR, PIC_SLAVE_IMR);
	
	outb_pic(PIC_CASCADE_IR, PIC_SLAVE_IMR);
	
	outb_pic(SLAVE_ICW4_DEFAULT, PIC_SLAVE_IMR);

	if (auto_eoi)
		
		i8259A_chip.mask_ack = disable_8259A_irq;
	else
		i8259A_chip.mask_ack = mask_and_ack_8259A;

	udelay(100);		

	outb(cached_master_mask, PIC_MASTER_IMR); 
	outb(cached_slave_mask, PIC_SLAVE_IMR);	  

	spin_unlock_irqrestore(&i8259A_lock, flags);
}
