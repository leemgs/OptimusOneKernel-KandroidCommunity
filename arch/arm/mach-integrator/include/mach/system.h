
#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H

#include <mach/cm.h>

static inline void arch_idle(void)
{
	
	cpu_do_idle();
}

static inline void arch_reset(char mode, const char *cmd)
{
	
	cm_control(CM_CTRL_RESET, CM_CTRL_RESET);
}

#endif
