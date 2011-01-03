

#ifndef AT91_WDT_H
#define AT91_WDT_H

#define AT91_WDT_CR		(AT91_WDT + 0x00)	
#define		AT91_WDT_WDRSTT		(1    << 0)		
#define		AT91_WDT_KEY		(0xa5 << 24)		

#define AT91_WDT_MR		(AT91_WDT + 0x04)	
#define		AT91_WDT_WDV		(0xfff << 0)		
#define		AT91_WDT_WDFIEN		(1     << 12)		
#define		AT91_WDT_WDRSTEN	(1     << 13)		
#define		AT91_WDT_WDRPROC	(1     << 14)		
#define		AT91_WDT_WDDIS		(1     << 15)		
#define		AT91_WDT_WDD		(0xfff << 16)		
#define		AT91_WDT_WDDBGHLT	(1     << 28)		
#define		AT91_WDT_WDIDLEHLT	(1     << 29)		

#define AT91_WDT_SR		(AT91_WDT + 0x08)	
#define		AT91_WDT_WDUNF		(1 << 0)		
#define		AT91_WDT_WDERR		(1 << 1)		

#endif
