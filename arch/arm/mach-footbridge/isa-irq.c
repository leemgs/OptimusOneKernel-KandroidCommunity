
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/spinlock.h>

#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <asm/hardware/dec21285.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include "common.h"

static void isa_mask_pic_lo_irq(unsigned int irq)
{
	unsigned int mask = 1 << (irq & 7);

	outb(inb(PIC_MASK_LO) | mask, PIC_MASK_LO);
}

static void isa_ack_pic_lo_irq(unsigned int irq)
{
	unsigned int mask = 1 << (irq & 7);

	outb(inb(PIC_MASK_LO) | mask, PIC_MASK_LO);
	outb(0x20, PIC_LO);
}

static void isa_unmask_pic_lo_irq(unsigned int irq)
{
	unsigned int mask = 1 << (irq & 7);

	outb(inb(PIC_MASK_LO) & ~mask, PIC_MASK_LO);
}

static struct irq_chip isa_lo_chip = {
	.ack	= isa_ack_pic_lo_irq,
	.mask	= isa_mask_pic_lo_irq,
	.unmask = isa_unmask_pic_lo_irq,
};

static void isa_mask_pic_hi_irq(unsigned int irq)
{
	unsigned int mask = 1 << (irq & 7);

	outb(inb(PIC_MASK_HI) | mask, PIC_MASK_HI);
}

static void isa_ack_pic_hi_irq(unsigned int irq)
{
	unsigned int mask = 1 << (irq & 7);

	outb(inb(PIC_MASK_HI) | mask, PIC_MASK_HI);
	outb(0x62, PIC_LO);
	outb(0x20, PIC_HI);
}

static void isa_unmask_pic_hi_irq(unsigned int irq)
{
	unsigned int mask = 1 << (irq & 7);

	outb(inb(PIC_MASK_HI) & ~mask, PIC_MASK_HI);
}

static struct irq_chip isa_hi_chip = {
	.ack	= isa_ack_pic_hi_irq,
	.mask	= isa_mask_pic_hi_irq,
	.unmask = isa_unmask_pic_hi_irq,
};

static void
isa_irq_handler(unsigned int irq, struct irq_desc *desc)
{
	unsigned int isa_irq = *(unsigned char *)PCIIACK_BASE;

	if (isa_irq < _ISA_IRQ(0) || isa_irq >= _ISA_IRQ(16)) {
		do_bad_IRQ(isa_irq, desc);
		return;
	}

	generic_handle_irq(isa_irq);
}

static struct irqaction irq_cascade = {
	.handler = no_action,
	.name = "cascade",
};

static struct resource pic1_resource = {
	.name	= "pic1",
	.start	= 0x20,
	.end	= 0x3f,
};

static struct resource pic2_resource = {
	.name	= "pic2",
	.start	= 0xa0,
	.end	= 0xbf,
};

void __init isa_init_irq(unsigned int host_irq)
{
	unsigned int irq;

	
	outb(0x11, PIC_LO);
	outb(_ISA_IRQ(0), PIC_MASK_LO);	
	outb(0x04, PIC_MASK_LO);	
	outb(0x01, PIC_MASK_LO);	
	outb(0xf5, PIC_MASK_LO);	

	outb(0x11, PIC_HI);
	outb(_ISA_IRQ(8), PIC_MASK_HI);	
	outb(0x02, PIC_MASK_HI);	
	outb(0x01, PIC_MASK_HI);	
	outb(0xfa, PIC_MASK_HI);	

	outb(0x0b, PIC_LO);
	outb(0x0b, PIC_HI);

	if (inb(PIC_MASK_LO) == 0xf5 && inb(PIC_MASK_HI) == 0xfa) {
		outb(0xff, PIC_MASK_LO);
		outb(0xff, PIC_MASK_HI);
	} else {
		printk(KERN_INFO "IRQ: ISA PIC not found\n");
		host_irq = (unsigned int)-1;
	}

	if (host_irq != (unsigned int)-1) {
		for (irq = _ISA_IRQ(0); irq < _ISA_IRQ(8); irq++) {
			set_irq_chip(irq, &isa_lo_chip);
			set_irq_handler(irq, handle_level_irq);
			set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);
		}

		for (irq = _ISA_IRQ(8); irq < _ISA_IRQ(16); irq++) {
			set_irq_chip(irq, &isa_hi_chip);
			set_irq_handler(irq, handle_level_irq);
			set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);
		}

		request_resource(&ioport_resource, &pic1_resource);
		request_resource(&ioport_resource, &pic2_resource);
		setup_irq(IRQ_ISA_CASCADE, &irq_cascade);

		set_irq_chained_handler(host_irq, isa_irq_handler);

		
		if (machine_is_netwinder())
			set_irq_flags(_ISA_IRQ(11), IRQF_VALID |
				      IRQF_PROBE | IRQF_NOAUTOEN);
	}
}


