

#ifndef __ASM_ARCH_PXA_GPIO_H
#define __ASM_ARCH_PXA_GPIO_H

#include <mach/irqs.h>
#include <mach/hardware.h>
#include <asm-generic/gpio.h>

#define GPIO_REGS_VIRT	io_p2v(0x40E00000)

#define BANK_OFF(n)	(((n) < 3) ? (n) << 2 : 0x100 + (((n) - 3) << 2))
#define GPIO_REG(x)	(*(volatile u32 *)(GPIO_REGS_VIRT + (x)))


#define GPLR0		GPIO_REG(BANK_OFF(0) + 0x00)
#define GPLR1		GPIO_REG(BANK_OFF(1) + 0x00)
#define GPLR2		GPIO_REG(BANK_OFF(2) + 0x00)
#define GPLR3		GPIO_REG(BANK_OFF(3) + 0x00)


#define GPDR0		GPIO_REG(BANK_OFF(0) + 0x0c)
#define GPDR1		GPIO_REG(BANK_OFF(1) + 0x0c)
#define GPDR2		GPIO_REG(BANK_OFF(2) + 0x0c)
#define GPDR3		GPIO_REG(BANK_OFF(3) + 0x0c)


#define GPSR0		GPIO_REG(BANK_OFF(0) + 0x18)
#define GPSR1		GPIO_REG(BANK_OFF(1) + 0x18)
#define GPSR2		GPIO_REG(BANK_OFF(2) + 0x18)
#define GPSR3		GPIO_REG(BANK_OFF(3) + 0x18)


#define GPCR0		GPIO_REG(BANK_OFF(0) + 0x24)
#define GPCR1		GPIO_REG(BANK_OFF(1) + 0x24)
#define GPCR2		GPIO_REG(BANK_OFF(2) + 0x24)
#define GPCR3		GPIO_REG(BANK_OFF(3) + 0x24)


#define GRER0		GPIO_REG(BANK_OFF(0) + 0x30)
#define GRER1		GPIO_REG(BANK_OFF(1) + 0x30)
#define GRER2		GPIO_REG(BANK_OFF(2) + 0x30)
#define GRER3		GPIO_REG(BANK_OFF(3) + 0x30)


#define GFER0		GPIO_REG(BANK_OFF(0) + 0x3c)
#define GFER1		GPIO_REG(BANK_OFF(1) + 0x3c)
#define GFER2		GPIO_REG(BANK_OFF(2) + 0x3c)
#define GFER3		GPIO_REG(BANK_OFF(3) + 0x3c)


#define GEDR0		GPIO_REG(BANK_OFF(0) + 0x48)
#define GEDR1		GPIO_REG(BANK_OFF(1) + 0x48)
#define GEDR2		GPIO_REG(BANK_OFF(2) + 0x48)
#define GEDR3		GPIO_REG(BANK_OFF(3) + 0x48)


#define GAFR0_L		GPIO_REG(0x0054)
#define GAFR0_U		GPIO_REG(0x0058)
#define GAFR1_L		GPIO_REG(0x005C)
#define GAFR1_U		GPIO_REG(0x0060)
#define GAFR2_L		GPIO_REG(0x0064)
#define GAFR2_U		GPIO_REG(0x0068)
#define GAFR3_L		GPIO_REG(0x006C)
#define GAFR3_U		GPIO_REG(0x0070)



#define GPIO_bit(x)	(1 << ((x) & 0x1f))

#define GPLR(x)		GPIO_REG(BANK_OFF((x) >> 5) + 0x00)
#define GPDR(x)		GPIO_REG(BANK_OFF((x) >> 5) + 0x0c)
#define GPSR(x)		GPIO_REG(BANK_OFF((x) >> 5) + 0x18)
#define GPCR(x)		GPIO_REG(BANK_OFF((x) >> 5) + 0x24)
#define GRER(x)		GPIO_REG(BANK_OFF((x) >> 5) + 0x30)
#define GFER(x)		GPIO_REG(BANK_OFF((x) >> 5) + 0x3c)
#define GEDR(x)		GPIO_REG(BANK_OFF((x) >> 5) + 0x48)
#define GAFR(x)		GPIO_REG(0x54 + (((x) & 0x70) >> 2))


#define NR_BUILTIN_GPIO 128

#define gpio_to_bank(gpio)	((gpio) >> 5)
#define gpio_to_irq(gpio)	IRQ_GPIO(gpio)
#define irq_to_gpio(irq)	IRQ_TO_GPIO(irq)

#ifdef CONFIG_CPU_PXA26x

static inline int __gpio_is_inverted(unsigned gpio)
{
	return cpu_is_pxa25x() && gpio > 85;
}
#else
static inline int __gpio_is_inverted(unsigned gpio) { return 0; }
#endif


static inline int __gpio_is_occupied(unsigned gpio)
{
	if (cpu_is_pxa27x() || cpu_is_pxa25x()) {
		int af = (GAFR(gpio) >> ((gpio & 0xf) * 2)) & 0x3;
		int dir = GPDR(gpio) & GPIO_bit(gpio);

		if (__gpio_is_inverted(gpio))
			return af != 1 || dir == 0;
		else
			return af != 0 || dir != 0;
	} else
		return GPDR(gpio) & GPIO_bit(gpio);
}

#include <plat/gpio.h>
#endif
