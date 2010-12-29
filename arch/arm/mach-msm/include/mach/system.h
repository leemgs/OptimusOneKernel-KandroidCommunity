

#include <mach/hardware.h>

void arch_idle(void);

static inline void arch_reset(char mode, const char *cmd)
{
	for (;;) ;  
}


extern void (*msm_hw_reset_hook)(void);

void msm_set_i2c_mux(bool gpio, int *gpio_clk, int *gpio_dat);

