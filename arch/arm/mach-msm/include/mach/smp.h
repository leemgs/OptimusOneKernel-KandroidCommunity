

#ifndef __ASM_ARCH_MSM_SMP_H
#define __ASM_ARCH_MSM_SMP_H

#include <asm/hardware/gic.h>

static inline void smp_cross_call(const struct cpumask *mask)
{
	gic_raise_softirq(mask, 1);
}

#endif
