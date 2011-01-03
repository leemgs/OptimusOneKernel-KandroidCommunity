

#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H

#include <mach/hardware.h>
#include <mach/at91_st.h>
#include <mach/at91_dbgu.h>

static inline void arch_idle(void)
{
	


	
	cpu_do_idle();
}

void (*at91_arch_reset)(void);

static inline void arch_reset(char mode, const char *cmd)
{
	
	if (at91_arch_reset)
		(at91_arch_reset)();
}

#endif
