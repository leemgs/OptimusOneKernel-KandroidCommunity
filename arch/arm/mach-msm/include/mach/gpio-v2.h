

#ifndef __ASM_ARCH_MSM_GPIO_V2_H
#define __ASM_ARCH_MSM_GPIO_V2_H

#include <mach/gpio-tlmm-v1.h>
#if defined(CONFIG_ARCH_MSM8X60)
#include <mach/gpio-v2-8x60.h>
#endif



#include <asm-generic/gpio.h>
#include <mach/irqs.h>

#define gpio_get_value __gpio_get_value
#define gpio_set_value __gpio_set_value
#define gpio_cansleep  __gpio_cansleep
#define gpio_to_irq    __gpio_to_irq


int msm_gpio_install_direct_irq(unsigned gpio, unsigned irq);

#endif 
