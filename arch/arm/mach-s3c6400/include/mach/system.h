

#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H __FILE__

#include <plat/watchdog-reset.h>

static void arch_idle(void)
{
	
}

static void arch_reset(char mode, const char *cmd)
{
	if (mode != 's')
		arch_wdt_reset();

	
	cpu_reset(0);
}

#endif 
