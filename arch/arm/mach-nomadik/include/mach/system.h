
#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H

#include <linux/io.h>
#include <mach/hardware.h>

static inline void arch_idle(void)
{
	
	cpu_do_idle();
}

static inline void arch_reset(char mode, const char *cmd)
{
	void __iomem *src_rstsr = io_p2v(NOMADIK_SRC_BASE + 0x18);

	

	
	writel(1, src_rstsr);
}

#endif
