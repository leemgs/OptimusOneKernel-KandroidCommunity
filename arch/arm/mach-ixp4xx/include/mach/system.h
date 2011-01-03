

#include <mach/hardware.h>

static inline void arch_idle(void)
{
	
#if 0
	cpu_do_idle();
#endif
}


static inline void arch_reset(char mode, const char *cmd)
{
	if ( 1 && mode == 's') {
		
		cpu_reset(0);
	} else {
		

		
		*IXP4XX_OSWK = IXP4XX_WDT_KEY;

		
		*IXP4XX_OSWT = 0;

		*IXP4XX_OSWE = IXP4XX_WDT_RESET_ENABLE | IXP4XX_WDT_COUNT_ENABLE;
	}
}

