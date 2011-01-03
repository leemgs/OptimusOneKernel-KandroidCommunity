
#include <mach/hardware.h>

static inline void arch_idle(void)
{
	cpu_do_idle();
}

static inline void arch_reset(char mode, const char *cmd)
{
	if (mode == 's') {
		
		cpu_reset(0);
	} else {
		
		RSRR = RSRR_SWR;
	}
}
