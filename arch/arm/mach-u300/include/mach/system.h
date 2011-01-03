
#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/hardware/vic.h>
#include <asm/irq.h>


void coh901327_watchdog_reset(void);

static inline void arch_idle(void)
{
	cpu_do_idle();
}

static void arch_reset(char mode, const char *cmd)
{
	switch (mode) {
	case 's':
	case 'h':
		printk(KERN_CRIT "RESET: shutting down/rebooting system\n");
		
		local_irq_disable();
#ifdef CONFIG_COH901327_WATCHDOG
		coh901327_watchdog_reset();
#endif
		break;
	default:
		
		break;
	}
	
	while (1);
}
