
#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H

#include <linux/io.h>
#include <mach/hardware.h>
#include <mach/platform.h>

void (*realview_reset)(char mode);

static inline void arch_idle(void)
{
	
	cpu_do_idle();
}

static inline void arch_reset(char mode, const char *cmd)
{
	
	if (realview_reset)
		realview_reset(mode);
}

#endif
