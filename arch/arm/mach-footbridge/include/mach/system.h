
#include <linux/io.h>
#include <asm/hardware/dec21285.h>
#include <mach/hardware.h>
#include <asm/leds.h>
#include <asm/mach-types.h>

static inline void arch_idle(void)
{
	cpu_do_idle();
}

static inline void arch_reset(char mode, const char *cmd)
{
	if (mode == 's') {
		
		cpu_reset(0x41000000);
	} else {
		if (machine_is_netwinder()) {
			
			outb(0x87, 0x370);
			outb(0x87, 0x370);

			
			outb(0x07, 0x370);
			outb(0x07, 0x371);

			
			outb(0xe6, 0x370);
			outb(0x00, 0x371);

			
			outb(0xc4, 0x338);
		} else {
			
			*CSR_SA110_CNTL &= ~(1 << 13);
			*CSR_TIMER4_CNTL = TIMER_CNTL_ENABLE |
					   TIMER_CNTL_AUTORELOAD |
					   TIMER_CNTL_DIV16;
			*CSR_TIMER4_LOAD = 0x2;
			*CSR_TIMER4_CLR  = 0;
			*CSR_SA110_CNTL |= (1 << 13);
		}
	}
}
