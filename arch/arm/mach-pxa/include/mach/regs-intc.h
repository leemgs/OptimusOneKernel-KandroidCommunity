#ifndef __ASM_MACH_REGS_INTC_H
#define __ASM_MACH_REGS_INTC_H

#include <mach/hardware.h>



#define ICIP		__REG(0x40D00000)  
#define ICMR		__REG(0x40D00004)  
#define ICLR		__REG(0x40D00008)  
#define ICFP		__REG(0x40D0000C)  
#define ICPR		__REG(0x40D00010)  
#define ICCR		__REG(0x40D00014)  
#define ICHP		__REG(0x40D00018)  

#define ICIP2		__REG(0x40D0009C)  
#define ICMR2		__REG(0x40D000A0)  
#define ICLR2		__REG(0x40D000A4)  
#define ICFP2		__REG(0x40D000A8)  
#define ICPR2		__REG(0x40D000AC)  

#define ICIP3		__REG(0x40D00130)  
#define ICMR3		__REG(0x40D00134)  
#define ICLR3		__REG(0x40D00138)  
#define ICFP3		__REG(0x40D0013C)  
#define ICPR3		__REG(0x40D00140)  

#define IPR(x)		__REG(0x40D0001C + (x < 32 ? (x << 2)		\
				: (x < 64 ? (0x94 + ((x - 32) << 2))	\
				: (0x128 + ((x - 64) << 2)))))

#endif 
