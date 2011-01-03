


#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H

#include <asm/proc-fns.h>
#include <mach/platform.h>
#include <mach/regs-clkctrl.h>
#include <mach/regs-power.h>

static inline void arch_idle(void)
{
	

	cpu_do_idle();
}

static inline void arch_reset(char mode, const char *cmd)
{
	
	__raw_writel(0x00010000, REGS_POWER_BASE + HW_POWER_CHARGE);

	
	__raw_writel(0, REGS_POWER_BASE + HW_POWER_MINPWR);

	
	__raw_writel(BM_CLKCTRL_RESET_DIG,
			REGS_CLKCTRL_BASE + HW_CLKCTRL_RESET);

	
}

#endif
