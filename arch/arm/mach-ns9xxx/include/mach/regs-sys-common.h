

#ifndef __ASM_ARCH_REGSSYSCOMMON_H
#define __ASM_ARCH_REGSSYSCOMMON_H
#include <mach/hardware.h>


#define SYS_IVA(x)	__REG2(0xa09000c4, (x))


#define SYS_IC(x)	__REG2(0xa0900144, (x))


#define SYS_ISRADDR     __REG(0xa0900164)


#define SYS_ISA		__REG(0xa0900168)


#define SYS_ISR		__REG(0xa090016c)

#endif 
