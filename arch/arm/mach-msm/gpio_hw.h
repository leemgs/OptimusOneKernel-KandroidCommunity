

#ifndef __ARCH_ARM_MACH_MSM_GPIO_HW_H
#define __ARCH_ARM_MACH_MSM_GPIO_HW_H

#include <mach/msm_iomap.h>



#if defined(CONFIG_ARCH_MSM7X30)
#define GPIO1_REG(off) (MSM_GPIO1_BASE + (off))
#define GPIO2_REG(off) (MSM_GPIO2_BASE + 0x400 + (off))
#else
#define GPIO1_REG(off) (MSM_GPIO1_BASE + 0x800 + (off))
#define GPIO2_REG(off) (MSM_GPIO2_BASE + 0xC00 + (off))
#endif

#if defined(CONFIG_ARCH_QSD8X50)
#include "gpio_hw-8xxx.h"
#elif defined(CONFIG_ARCH_MSM7X30)
#include "gpio_hw-7x30.h"
#else
#include "gpio_hw-7xxx.h"
#endif

#endif
