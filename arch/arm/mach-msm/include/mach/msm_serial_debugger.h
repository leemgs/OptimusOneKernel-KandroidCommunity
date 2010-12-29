

#ifndef __ASM_ARCH_MSM_SERIAL_DEBUGGER_H
#define __ASM_ARCH_MSM_SERIAL_DEBUGGER_H

#if defined(CONFIG_MSM_SERIAL_DEBUGGER)
void msm_serial_debug_init(unsigned int base, int irq,
		struct device *clk_device, int signal_irq, int wakeup_irq);
#else
static inline void msm_serial_debug_init(unsigned int base, int irq,
		struct device *clk_device, int signal_irq, int wakeup_irq) {}
#endif

#endif
