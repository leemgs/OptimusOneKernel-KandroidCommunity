

#ifndef __ASM_ARCH_REGS_IRQ_H
#define __ASM_ARCH_REGS_IRQ_H __FILE__

#include <mach/map.h>
#include <asm/hardware/vic.h>


#define S5PC1XX_VIC0REG(x)          		((x) + S5PC1XX_VA_VIC(0))
#define S5PC1XX_VIC1REG(x)          		((x) + S5PC1XX_VA_VIC(1))
#define S5PC1XX_VIC2REG(x)         		((x) + S5PC1XX_VA_VIC(2))

#endif 
