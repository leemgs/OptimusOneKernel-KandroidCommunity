

#ifndef _ARCH_ARM_MACH_MSM_SIRC_H
#define _ARCH_ARM_MACH_MSM_SIRC_H

#ifdef CONFIG_ARCH_MSM_SCORPION
void sirc_fiq_select(int irq, bool enable);
void __init msm_init_sirc(void);
#else
static inline void sirc_fiq_select(int irq, bool enable) {}
static inline void __init msm_init_sirc(void) {}
#endif

#endif
