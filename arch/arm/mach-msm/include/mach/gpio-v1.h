

#ifndef __ASM_ARCH_MSM_GPIO_V1_H
#define __ASM_ARCH_MSM_GPIO_V1_H

#ifdef CONFIG_GPIOLIB

#define ARCH_NR_GPIOS	512

#include <asm-generic/gpio.h>

#define gpio_get_value	__gpio_get_value
#define gpio_set_value	__gpio_set_value
#define gpio_cansleep	__gpio_cansleep
#define gpio_to_irq	__gpio_to_irq

#else

int gpio_request(unsigned gpio, const char *label);
void gpio_free(unsigned gpio);
int gpio_direction_input(unsigned gpio);
int gpio_direction_output(unsigned gpio, int value);
int gpio_get_value(unsigned gpio);
void gpio_set_value(unsigned gpio, int value);
int gpio_to_irq(unsigned gpio);

#include <linux/device.h>
#include <asm-generic/gpio.h>

#endif

#include <linux/interrupt.h>
#include <mach/gpio-tlmm-v1.h>


struct msm_gpio {
	u32 gpio_cfg;
	const char *label;
};


int msm_gpios_request_enable(const struct msm_gpio *table, int size);

void msm_gpios_disable_free(const struct msm_gpio *table, int size);

int msm_gpios_request(const struct msm_gpio *table, int size);

void msm_gpios_free(const struct msm_gpio *table, int size);

int msm_gpios_enable(const struct msm_gpio *table, int size);

int msm_gpios_disable(const struct msm_gpio *table, int size);



#define GPIOF_IRQF_MASK         0x0000ffff 
#define GPIOF_IRQF_TRIGGER_NONE 0x00010000 
#define GPIOF_INPUT             0x00020000
#define GPIOF_DRIVE_OUTPUT      0x00040000
#define GPIOF_OUTPUT_LOW        0x00080000
#define GPIOF_OUTPUT_HIGH       0x00100000

#define GPIOIRQF_SHARED         0x00000001 

#ifdef CONFIG_GPIOLIB
static inline int gpio_configure(unsigned int gpio, unsigned long flags)
{
	WARN("%s is deprecated. Do not use it.\n", __func__);
	return -ENOSYS;
}

static inline int gpio_read_detect_status(unsigned int gpio)
{
	WARN("%s is deprecated. Do not use it.\n", __func__);
	return -ENOSYS;
}

static inline int gpio_clear_detect_status(unsigned int gpio)
{
	WARN("%s is deprecated. Do not use it.\n", __func__);
	return -ENOSYS;
}
#else
int gpio_configure(unsigned int gpio, unsigned long flags);
int gpio_read_detect_status(unsigned int gpio);
int gpio_clear_detect_status(unsigned int gpio);
#endif

#endif

