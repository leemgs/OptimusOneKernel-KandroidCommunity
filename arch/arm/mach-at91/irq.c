

#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/types.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/setup.h>

#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>


static void at91_aic_mask_irq(unsigned int irq)
{
	
	at91_sys_write(AT91_AIC_IDCR, 1 << irq);
}

static void at91_aic_unmask_irq(unsigned int irq)
{
	
	at91_sys_write(AT91_AIC_IECR, 1 << irq);
}

unsigned int at91_extern_irq;

#define is_extern_irq(irq) ((1 << (irq)) & at91_extern_irq)

static int at91_aic_set_type(unsigned irq, unsigned type)
{
	unsigned int smr, srctype;

	switch (type) {
	case IRQ_TYPE_LEVEL_HIGH:
		srctype = AT91_AIC_SRCTYPE_HIGH;
		break;
	case IRQ_TYPE_EDGE_RISING:
		srctype = AT91_AIC_SRCTYPE_RISING;
		break;
	case IRQ_TYPE_LEVEL_LOW:
		if ((irq == AT91_ID_FIQ) || is_extern_irq(irq))		
			srctype = AT91_AIC_SRCTYPE_LOW;
		else
			return -EINVAL;
		break;
	case IRQ_TYPE_EDGE_FALLING:
		if ((irq == AT91_ID_FIQ) || is_extern_irq(irq))		
			srctype = AT91_AIC_SRCTYPE_FALLING;
		else
			return -EINVAL;
		break;
	default:
		return -EINVAL;
	}

	smr = at91_sys_read(AT91_AIC_SMR(irq)) & ~AT91_AIC_SRCTYPE;
	at91_sys_write(AT91_AIC_SMR(irq), smr | srctype);
	return 0;
}

#ifdef CONFIG_PM

static u32 wakeups;
static u32 backups;

static int at91_aic_set_wake(unsigned irq, unsigned value)
{
	if (unlikely(irq >= 32))
		return -EINVAL;

	if (value)
		wakeups |= (1 << irq);
	else
		wakeups &= ~(1 << irq);

	return 0;
}

void at91_irq_suspend(void)
{
	backups = at91_sys_read(AT91_AIC_IMR);
	at91_sys_write(AT91_AIC_IDCR, backups);
	at91_sys_write(AT91_AIC_IECR, wakeups);
}

void at91_irq_resume(void)
{
	at91_sys_write(AT91_AIC_IDCR, wakeups);
	at91_sys_write(AT91_AIC_IECR, backups);
}

#else
#define at91_aic_set_wake	NULL
#endif

static struct irq_chip at91_aic_chip = {
	.name		= "AIC",
	.ack		= at91_aic_mask_irq,
	.mask		= at91_aic_mask_irq,
	.unmask		= at91_aic_unmask_irq,
	.set_type	= at91_aic_set_type,
	.set_wake	= at91_aic_set_wake,
};


void __init at91_aic_init(unsigned int priority[NR_AIC_IRQS])
{
	unsigned int i;

	
	for (i = 0; i < NR_AIC_IRQS; i++) {
		
		at91_sys_write(AT91_AIC_SVR(i), i);
		
		at91_sys_write(AT91_AIC_SMR(i), AT91_AIC_SRCTYPE_LOW | priority[i]);

		set_irq_chip(i, &at91_aic_chip);
		set_irq_handler(i, handle_level_irq);
		set_irq_flags(i, IRQF_VALID | IRQF_PROBE);

		
		if (i < 8)
			at91_sys_write(AT91_AIC_EOICR, 0);
	}

	
	at91_sys_write(AT91_AIC_SPU, NR_AIC_IRQS);

	
	at91_sys_write(AT91_AIC_DCR, 0);

	
	at91_sys_write(AT91_AIC_IDCR, 0xFFFFFFFF);
	at91_sys_write(AT91_AIC_ICCR, 0xFFFFFFFF);
}
