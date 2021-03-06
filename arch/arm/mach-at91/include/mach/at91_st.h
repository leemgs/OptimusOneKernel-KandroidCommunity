

#ifndef AT91_ST_H
#define AT91_ST_H

#define	AT91_ST_CR		(AT91_ST + 0x00)	
#define 	AT91_ST_WDRST		(1 << 0)		

#define	AT91_ST_PIMR		(AT91_ST + 0x04)	
#define		AT91_ST_PIV		(0xffff <<  0)		

#define	AT91_ST_WDMR		(AT91_ST + 0x08)	
#define		AT91_ST_WDV		(0xffff <<  0)		
#define		AT91_ST_RSTEN		(1	<< 16)		
#define		AT91_ST_EXTEN		(1	<< 17)		

#define	AT91_ST_RTMR		(AT91_ST + 0x0c)	
#define		AT91_ST_RTPRES		(0xffff <<  0)		

#define	AT91_ST_SR		(AT91_ST + 0x10)	
#define		AT91_ST_PITS		(1 << 0)		
#define		AT91_ST_WDOVF		(1 << 1) 		
#define		AT91_ST_RTTINC		(1 << 2) 		
#define		AT91_ST_ALMS		(1 << 3) 		

#define	AT91_ST_IER		(AT91_ST + 0x14)	
#define	AT91_ST_IDR		(AT91_ST + 0x18)	
#define	AT91_ST_IMR		(AT91_ST + 0x1c)	

#define	AT91_ST_RTAR		(AT91_ST + 0x20)	
#define		AT91_ST_ALMV		(0xfffff << 0)		

#define	AT91_ST_CRTR		(AT91_ST + 0x24)	
#define		AT91_ST_CRTV		(0xfffff << 0)		

#endif
