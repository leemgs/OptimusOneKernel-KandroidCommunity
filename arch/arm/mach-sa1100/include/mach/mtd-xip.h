

#ifndef __ARCH_SA1100_MTD_XIP_H__
#define __ARCH_SA1100_MTD_XIP_H__

#include <mach/hardware.h>

#define xip_irqpending()	(ICIP & ICMR)


#define xip_currtime()		(OSCR)
#define xip_elapsed_since(x)	(signed)((OSCR - (x)) / 4)

#endif 
