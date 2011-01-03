

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/io.h>

#include <asm/irq.h>
#include <asm/mach/irq.h>




static unsigned char cached_irq_mask[2] = { 0xfb, 0xff };


static void shark_disable_8259A_irq(unsigned int irq)
{
	unsigned int mask;
	if (irq<8) {
	  mask = 1 << irq;
	  cached_irq_mask[0] |= mask;
	  outb(cached_irq_mask[1],0xA1);
	} else {
	  mask = 1 << (irq-8);
	  cached_irq_mask[1] |= mask;
	  outb(cached_irq_mask[0],0x21);
	}
}

static void shark_enable_8259A_irq(unsigned int irq)
{
	unsigned int mask;
	if (irq<8) {
	  mask = ~(1 << irq);
	  cached_irq_mask[0] &= mask;
	  outb(cached_irq_mask[0],0x21);
	} else {
	  mask = ~(1 << (irq-8));
	  cached_irq_mask[1] &= mask;
	  outb(cached_irq_mask[1],0xA1);
	}
}

static void shark_ack_8259A_irq(unsigned int irq){}

static irqreturn_t bogus_int(int irq, void *dev_id)
{
	printk("Got interrupt %i!\n",irq);
	return IRQ_NONE;
}

static struct irqaction cascade;

static struct irq_chip fb_chip = {
	.name	= "XT-PIC",
	.ack	= shark_ack_8259A_irq,
	.mask	= shark_disable_8259A_irq,
	.unmask = shark_enable_8259A_irq,
};

void __init shark_init_irq(void)
{
	int irq;

	for (irq = 0; irq < NR_IRQS; irq++) {
		set_irq_chip(irq, &fb_chip);
		set_irq_handler(irq, handle_edge_irq);
		set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);
	}

	
	outb(0x11, 0x20); 
	outb(0x00, 0x21); 
	outb(0x04, 0x21); 
	outb(0x03, 0x21); 
	outb(0x0A, 0x20);
	
	outb(0x11, 0xA0); 
	outb(0x08, 0xA1); 
	outb(0x02, 0xA1); 
	outb(0x03, 0xA1); 
	outb(0x0A, 0xA0);
	outb(cached_irq_mask[1],0xA1);
	outb(cached_irq_mask[0],0x21);
	
	

	cascade.handler = bogus_int;
	cascade.name = "cascade";
	setup_irq(2,&cascade);
}

