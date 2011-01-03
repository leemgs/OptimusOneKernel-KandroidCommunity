
#ifndef __MACH_SYSTEM_H
#define __MACH_SYSTEM_H

#include <linux/io.h>
#include <mach/hardware.h>
#include <mach/global_reg.h>

static inline void arch_idle(void)
{
	
	local_irq_enable();
	cpu_do_idle();
}

static inline void arch_reset(char mode, const char *cmd)
{
	__raw_writel(RESET_GLOBAL | RESET_CPU1,
		     IO_ADDRESS(GEMINI_GLOBAL_BASE) + GLOBAL_RESET);
}

#endif 
