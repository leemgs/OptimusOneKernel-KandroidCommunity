

#ifndef __ASM_ARCH_H1940_LATCH_H
#define __ASM_ARCH_H1940_LATCH_H


#ifndef __ASSEMBLY__
#define H1940_LATCH		((void __force __iomem *)0xF8000000)
#else
#define H1940_LATCH		0xF8000000
#endif

#define H1940_PA_LATCH		(S3C2410_CS2)



#define H1940_LATCH_SDQ1		(1<<16)
#define H1940_LATCH_LCD_P1		(1<<17)
#define H1940_LATCH_LCD_P2		(1<<18)
#define H1940_LATCH_LCD_P3		(1<<19)
#define H1940_LATCH_MAX1698_nSHUTDOWN	(1<<20)		
#define H1940_LATCH_LED_RED		(1<<21)
#define H1940_LATCH_SDQ7		(1<<22)
#define H1940_LATCH_USB_DP		(1<<23)



#define H1940_LATCH_UDA_POWER		(1<<24)
#define H1940_LATCH_AUDIO_POWER		(1<<25)
#define H1940_LATCH_SM803_ENABLE	(1<<26)
#define H1940_LATCH_LCD_P4		(1<<27)
#define H1940_LATCH_CPUQ5		(1<<28)		
#define H1940_LATCH_BLUETOOTH_POWER	(1<<29)		
#define H1940_LATCH_LED_GREEN		(1<<30)
#define H1940_LATCH_LED_FLASH		(1<<31)



#define H1940_LATCH_DEFAULT		\
	H1940_LATCH_LCD_P4		| \
	H1940_LATCH_SM803_ENABLE	| \
	H1940_LATCH_SDQ1		| \
	H1940_LATCH_LCD_P1		| \
	H1940_LATCH_LCD_P2		| \
	H1940_LATCH_LCD_P3		| \
	H1940_LATCH_MAX1698_nSHUTDOWN   | \
	H1940_LATCH_CPUQ5



extern void h1940_latch_control(unsigned int clear, unsigned int set);

#endif 
