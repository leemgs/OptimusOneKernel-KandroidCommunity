
#include <linux/init.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/stddef.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <asm/irq.h>
#include <asm/mach/irq.h>
#include <mach/hardware.h>

#define IRQ_SOURCE(base_addr)	(base_addr + 0x00)
#define IRQ_MASK(base_addr)	(base_addr + 0x04)
#define IRQ_CLEAR(base_addr)	(base_addr + 0x08)
#define IRQ_TMODE(base_addr)	(base_addr + 0x0C)
#define IRQ_TLEVEL(base_addr)	(base_addr + 0x10)
#define IRQ_STATUS(base_addr)	(base_addr + 0x14)
#define FIQ_SOURCE(base_addr)	(base_addr + 0x20)
#define FIQ_MASK(base_addr)	(base_addr + 0x24)
#define FIQ_CLEAR(base_addr)	(base_addr + 0x28)
#define FIQ_TMODE(base_addr)	(base_addr + 0x2C)
#define FIQ_LEVEL(base_addr)	(base_addr + 0x30)
#define FIQ_STATUS(base_addr)	(base_addr + 0x34)

static void gemini_ack_irq(unsigned int irq)
{
	__raw_writel(1 << irq, IRQ_CLEAR(IO_ADDRESS(GEMINI_INTERRUPT_BASE)));
}

static void gemini_mask_irq(unsigned int irq)
{
	unsigned int mask;

	mask = __raw_readl(IRQ_MASK(IO_ADDRESS(GEMINI_INTERRUPT_BASE)));
	mask &= ~(1 << irq);
	__raw_writel(mask, IRQ_MASK(IO_ADDRESS(GEMINI_INTERRUPT_BASE)));
}

static void gemini_unmask_irq(unsigned int irq)
{
	unsigned int mask;

	mask = __raw_readl(IRQ_MASK(IO_ADDRESS(GEMINI_INTERRUPT_BASE)));
	mask |= (1 << irq);
	__raw_writel(mask, IRQ_MASK(IO_ADDRESS(GEMINI_INTERRUPT_BASE)));
}

static struct irq_chip gemini_irq_chip = {
	.name	= "INTC",
	.ack	= gemini_ack_irq,
	.mask	= gemini_mask_irq,
	.unmask	= gemini_unmask_irq,
};

static struct resource irq_resource = {
	.name	= "irq_handler",
	.start	= IO_ADDRESS(GEMINI_INTERRUPT_BASE),
	.end	= IO_ADDRESS(FIQ_STATUS(GEMINI_INTERRUPT_BASE)) + 4,
};

void __init gemini_init_irq(void)
{
	unsigned int i, mode = 0, level = 0;

	
	disable_hlt();

	request_resource(&iomem_resource, &irq_resource);

	for (i = 0; i < NR_IRQS; i++) {
		set_irq_chip(i, &gemini_irq_chip);
		if((i >= IRQ_TIMER1 && i <= IRQ_TIMER3) || (i >= IRQ_SERIRQ0 && i <= IRQ_SERIRQ1)) {
			set_irq_handler(i, handle_edge_irq);
			mode |= 1 << i;
			level |= 1 << i;
		} else {			
			set_irq_handler(i, handle_level_irq);
		}
		set_irq_flags(i, IRQF_VALID | IRQF_PROBE);
	}

	
	__raw_writel(0, IRQ_MASK(IO_ADDRESS(GEMINI_INTERRUPT_BASE)));
	__raw_writel(0, FIQ_MASK(IO_ADDRESS(GEMINI_INTERRUPT_BASE)));

	
	__raw_writel(mode, IRQ_TMODE(IO_ADDRESS(GEMINI_INTERRUPT_BASE)));
	__raw_writel(level, IRQ_TLEVEL(IO_ADDRESS(GEMINI_INTERRUPT_BASE)));
}
