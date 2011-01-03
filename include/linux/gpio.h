#ifndef __LINUX_GPIO_H
#define __LINUX_GPIO_H



#ifdef CONFIG_GENERIC_GPIO
#include <asm/gpio.h>

#else

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>

struct device;



static inline int gpio_is_valid(int number)
{
	return 0;
}

static inline int _gpio_request(unsigned gpio, const char *label)
{
	return -ENOSYS;
}

static inline void _gpio_free(unsigned gpio)
{
	might_sleep();

	
	WARN_ON(1);
}

static inline int _gpio_direction_input(unsigned gpio)
{
	return -ENOSYS;
}

static inline int _gpio_direction_output(unsigned gpio, int value)
{
	return -ENOSYS;
}

static inline int gpio_get_value(unsigned gpio)
{
	
	WARN_ON(1);
	return 0;
}

static inline void gpio_set_value(unsigned gpio, int value)
{
	
	WARN_ON(1);
}

static inline int gpio_cansleep(unsigned gpio)
{
	
	WARN_ON(1);
	return 0;
}

static inline int gpio_get_value_cansleep(unsigned gpio)
{
	
	WARN_ON(1);
	return 0;
}

static inline void gpio_set_value_cansleep(unsigned gpio, int value)
{
	
	WARN_ON(1);
}

static inline int gpio_export(unsigned gpio, bool direction_may_change)
{
	
	WARN_ON(1);
	return -EINVAL;
}

static inline int gpio_export_link(struct device *dev, const char *name,
				unsigned gpio)
{
	
	WARN_ON(1);
	return -EINVAL;
}


static inline void gpio_unexport(unsigned gpio)
{
	
	WARN_ON(1);
}

static inline int gpio_to_irq(unsigned gpio)
{
	
	WARN_ON(1);
	return -EINVAL;
}

static inline int irq_to_gpio(unsigned irq)
{
	
	WARN_ON(1);
	return -EINVAL;
}

#endif

#endif 
