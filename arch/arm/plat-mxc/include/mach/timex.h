

#ifndef __ASM_ARCH_MXC_TIMEX_H__
#define __ASM_ARCH_MXC_TIMEX_H__

#if defined CONFIG_ARCH_MX1
#define CLOCK_TICK_RATE		16000000
#elif defined CONFIG_ARCH_MX2
#define CLOCK_TICK_RATE		13300000
#elif defined CONFIG_ARCH_MX3
#define CLOCK_TICK_RATE		16625000
#elif defined CONFIG_ARCH_MX25
#define CLOCK_TICK_RATE		16000000
#elif defined CONFIG_ARCH_MXC91231
#define CLOCK_TICK_RATE		13000000
#endif

#endif				
