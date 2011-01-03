

#ifndef __ASM_ARCH_IRQS_H
#define __ASM_ARCH_IRQS_H


#define INT_GPIOF0_FIQ	0  
#define INT_BL_FIQ	1  
#define INT_WE_FIQ	2  
#define INT_MV_FIQ	3  
#define INT_SC		4  
#define INT_GPIO1	5  
#define INT_GPIO2	6  
#define INT_GPIO3	7  
#define INT_TMR1_OFL	8  
#define INT_TMR2_OFL	9  
#define INT_RTC_CM	10 
#define INT_TICK	11 
#define INT_UART1	12 
#define INT_UART2	13 
#define INT_LCD		14 
#define INT_SSI		15 
#define INT_UART3	16 
#define INT_SCI		17 
#define INT_AAC		18 
#define INT_MMC		19 
#define INT_USB		20 
#define INT_DMA		21 
#define INT_TMR3_UOFL	22 
#define INT_GPIO4	23 
#define INT_GPIO5	24 
#define INT_GPIO6	25 
#define INT_GPIO7	26 
#define INT_BMI		27 

#define NR_IRQS		(INT_BMI + 1)

#endif 
