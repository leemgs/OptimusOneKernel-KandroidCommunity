

#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/io.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach/irq.h>

#include <mach/fpga.h>
#include <mach/gpio.h>

static void fpga_mask_irq(unsigned int irq)
{
	irq -= OMAP_FPGA_IRQ_BASE;

	if (irq < 8)
		__raw_writeb((__raw_readb(OMAP1510_FPGA_IMR_LO)
			      & ~(1 << irq)), OMAP1510_FPGA_IMR_LO);
	else if (irq < 16)
		__raw_writeb((__raw_readb(OMAP1510_FPGA_IMR_HI)
			      & ~(1 << (irq - 8))), OMAP1510_FPGA_IMR_HI);
	else
		__raw_writeb((__raw_readb(INNOVATOR_FPGA_IMR2)
			      & ~(1 << (irq - 16))), INNOVATOR_FPGA_IMR2);
}


static inline u32 get_fpga_unmasked_irqs(void)
{
	return
		((__raw_readb(OMAP1510_FPGA_ISR_LO) &
		  __raw_readb(OMAP1510_FPGA_IMR_LO))) |
		((__raw_readb(OMAP1510_FPGA_ISR_HI) &
		  __raw_readb(OMAP1510_FPGA_IMR_HI)) << 8) |
		((__raw_readb(INNOVATOR_FPGA_ISR2) &
		  __raw_readb(INNOVATOR_FPGA_IMR2)) << 16);
}


static void fpga_ack_irq(unsigned int irq)
{
	
}

static void fpga_unmask_irq(unsigned int irq)
{
	irq -= OMAP_FPGA_IRQ_BASE;

	if (irq < 8)
		__raw_writeb((__raw_readb(OMAP1510_FPGA_IMR_LO) | (1 << irq)),
		     OMAP1510_FPGA_IMR_LO);
	else if (irq < 16)
		__raw_writeb((__raw_readb(OMAP1510_FPGA_IMR_HI)
			      | (1 << (irq - 8))), OMAP1510_FPGA_IMR_HI);
	else
		__raw_writeb((__raw_readb(INNOVATOR_FPGA_IMR2)
			      | (1 << (irq - 16))), INNOVATOR_FPGA_IMR2);
}

static void fpga_mask_ack_irq(unsigned int irq)
{
	fpga_mask_irq(irq);
	fpga_ack_irq(irq);
}

void innovator_fpga_IRQ_demux(unsigned int irq, struct irq_desc *desc)
{
	u32 stat;
	int fpga_irq;

	stat = get_fpga_unmasked_irqs();

	if (!stat)
		return;

	for (fpga_irq = OMAP_FPGA_IRQ_BASE;
	     (fpga_irq < OMAP_FPGA_IRQ_END) && stat;
	     fpga_irq++, stat >>= 1) {
		if (stat & 1) {
			generic_handle_irq(fpga_irq);
		}
	}
}

static struct irq_chip omap_fpga_irq_ack = {
	.name		= "FPGA-ack",
	.ack		= fpga_mask_ack_irq,
	.mask		= fpga_mask_irq,
	.unmask		= fpga_unmask_irq,
};


static struct irq_chip omap_fpga_irq = {
	.name		= "FPGA",
	.ack		= fpga_ack_irq,
	.mask		= fpga_mask_irq,
	.unmask		= fpga_unmask_irq,
};


void omap1510_fpga_init_irq(void)
{
	int i;

	__raw_writeb(0, OMAP1510_FPGA_IMR_LO);
	__raw_writeb(0, OMAP1510_FPGA_IMR_HI);
	__raw_writeb(0, INNOVATOR_FPGA_IMR2);

	for (i = OMAP_FPGA_IRQ_BASE; i < OMAP_FPGA_IRQ_END; i++) {

		if (i == OMAP1510_INT_FPGA_TS) {
			
			set_irq_chip(i, &omap_fpga_irq_ack);
		}
		else {
			
			set_irq_chip(i, &omap_fpga_irq);
		}

		set_irq_handler(i, handle_edge_irq);
		set_irq_flags(i, IRQF_VALID);
	}

	
	gpio_request(13, "FPGA irq");
	gpio_direction_input(13);
	set_irq_type(gpio_to_irq(13), IRQ_TYPE_EDGE_RISING);
	set_irq_chained_handler(OMAP1510_INT_FPGA, innovator_fpga_IRQ_demux);
}

EXPORT_SYMBOL(omap1510_fpga_init_irq);
