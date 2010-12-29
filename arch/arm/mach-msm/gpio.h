

#ifndef _ARCH_ARM_MACH_MSM_GPIO_H_
#define _ARCH_ARM_MACH_MSM_GPIO_H_

#ifdef CONFIG_GPIOLIB
static inline void msm_gpio_enter_sleep(int from_idle) {}
static inline void msm_gpio_exit_sleep(void) {}
#else
void msm_gpio_enter_sleep(int from_idle);
void msm_gpio_exit_sleep(void);
#endif

#endif
