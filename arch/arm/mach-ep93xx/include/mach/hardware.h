
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <mach/ep93xx-regs.h>
#include <mach/platform.h>

#define pcibios_assign_all_busses()	0


#define EP93XX_EXT_CLK_RATE	14745600
#define EP93XX_EXT_RTC_RATE	32768

#define EP93XX_KEYTCHCLK_DIV4	(EP93XX_EXT_CLK_RATE / 4)
#define EP93XX_KEYTCHCLK_DIV16	(EP93XX_EXT_CLK_RATE / 16)

#endif
