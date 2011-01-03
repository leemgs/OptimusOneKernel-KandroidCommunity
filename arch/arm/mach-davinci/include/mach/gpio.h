

#ifndef	__DAVINCI_GPIO_H
#define	__DAVINCI_GPIO_H

#include <linux/io.h>
#include <asm-generic/gpio.h>

#include <mach/irqs.h>
#include <mach/common.h>

#define DAVINCI_GPIO_BASE 0x01C67000


#define	GPIO(X)		(X)		


#define GPIO_TO_PIN(bank, gpio)	(16 * (bank) + (gpio))

struct gpio_controller {
	u32	dir;
	u32	out_data;
	u32	set_data;
	u32	clr_data;
	u32	in_data;
	u32	set_rising;
	u32	clr_rising;
	u32	set_falling;
	u32	clr_falling;
	u32	intstat;
};


static inline struct gpio_controller *__iomem
__gpio_to_controller(unsigned gpio)
{
	void *__iomem ptr;
	void __iomem *base = davinci_soc_info.gpio_base;

	if (gpio < 32 * 1)
		ptr = base + 0x10;
	else if (gpio < 32 * 2)
		ptr = base + 0x38;
	else if (gpio < 32 * 3)
		ptr = base + 0x60;
	else if (gpio < 32 * 4)
		ptr = base + 0x88;
	else if (gpio < 32 * 5)
		ptr = base + 0xb0;
	else
		ptr = NULL;
	return ptr;
}

static inline u32 __gpio_mask(unsigned gpio)
{
	return 1 << (gpio % 32);
}


static inline void gpio_set_value(unsigned gpio, int value)
{
	if (__builtin_constant_p(value) && gpio < DAVINCI_N_GPIO) {
		struct gpio_controller	*__iomem g;
		u32			mask;

		g = __gpio_to_controller(gpio);
		mask = __gpio_mask(gpio);
		if (value)
			__raw_writel(mask, &g->set_data);
		else
			__raw_writel(mask, &g->clr_data);
		return;
	}

	__gpio_set_value(gpio, value);
}


static inline int gpio_get_value(unsigned gpio)
{
	struct gpio_controller	*__iomem g;

	if (!__builtin_constant_p(gpio) || gpio >= DAVINCI_N_GPIO)
		return __gpio_get_value(gpio);

	g = __gpio_to_controller(gpio);
	return __gpio_mask(gpio) & __raw_readl(&g->in_data);
}

static inline int gpio_cansleep(unsigned gpio)
{
	if (__builtin_constant_p(gpio) && gpio < DAVINCI_N_GPIO)
		return 0;
	else
		return __gpio_cansleep(gpio);
}

static inline int gpio_to_irq(unsigned gpio)
{
	return __gpio_to_irq(gpio);
}

static inline int irq_to_gpio(unsigned irq)
{
	
	return -ENOSYS;
}

#endif				
