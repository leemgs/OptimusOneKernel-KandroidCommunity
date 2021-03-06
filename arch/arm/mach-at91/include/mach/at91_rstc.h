

#ifndef AT91_RSTC_H
#define AT91_RSTC_H

#define AT91_RSTC_CR		(AT91_RSTC + 0x00)	
#define		AT91_RSTC_PROCRST	(1 << 0)		
#define		AT91_RSTC_PERRST	(1 << 2)		
#define		AT91_RSTC_EXTRST	(1 << 3)		
#define		AT91_RSTC_KEY		(0xa5 << 24)		

#define AT91_RSTC_SR		(AT91_RSTC + 0x04)	
#define		AT91_RSTC_URSTS		(1 << 0)		
#define		AT91_RSTC_RSTTYP	(7 << 8)		
#define			AT91_RSTC_RSTTYP_GENERAL	(0 << 8)
#define			AT91_RSTC_RSTTYP_WAKEUP		(1 << 8)
#define			AT91_RSTC_RSTTYP_WATCHDOG	(2 << 8)
#define			AT91_RSTC_RSTTYP_SOFTWARE	(3 << 8)
#define			AT91_RSTC_RSTTYP_USER	(4 << 8)
#define		AT91_RSTC_NRSTL		(1 << 16)		
#define		AT91_RSTC_SRCMP		(1 << 17)		

#define AT91_RSTC_MR		(AT91_RSTC + 0x08)	
#define		AT91_RSTC_URSTEN	(1 << 0)		
#define		AT91_RSTC_URSTIEN	(1 << 4)		
#define		AT91_RSTC_ERSTL		(0xf << 8)		

#endif
