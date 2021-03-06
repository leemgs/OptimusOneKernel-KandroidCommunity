

#ifndef AT91_DBGU_H
#define AT91_DBGU_H

#ifdef AT91_DBGU
#define AT91_DBGU_CR		(AT91_DBGU + 0x00)	
#define AT91_DBGU_MR		(AT91_DBGU + 0x04)	
#define AT91_DBGU_IER		(AT91_DBGU + 0x08)	
#define		AT91_DBGU_TXRDY		(1 << 1)		
#define		AT91_DBGU_TXEMPTY	(1 << 9)		
#define AT91_DBGU_IDR		(AT91_DBGU + 0x0c)	
#define AT91_DBGU_IMR		(AT91_DBGU + 0x10)	
#define AT91_DBGU_SR		(AT91_DBGU + 0x14)	
#define AT91_DBGU_RHR		(AT91_DBGU + 0x18)	
#define AT91_DBGU_THR		(AT91_DBGU + 0x1c)	
#define AT91_DBGU_BRGR		(AT91_DBGU + 0x20)	

#define AT91_DBGU_CIDR		(AT91_DBGU + 0x40)	
#define AT91_DBGU_EXID		(AT91_DBGU + 0x44)	
#define AT91_DBGU_FNR		(AT91_DBGU + 0x48)	
#define		AT91_DBGU_FNTRST	(1 << 0)		

#endif 


#define		AT91_CIDR_VERSION	(0x1f << 0)		
#define		AT91_CIDR_EPROC		(7    << 5)		
#define		AT91_CIDR_NVPSIZ	(0xf  << 8)		
#define		AT91_CIDR_NVPSIZ2	(0xf  << 12)		
#define		AT91_CIDR_SRAMSIZ	(0xf  << 16)		
#define			AT91_CIDR_SRAMSIZ_1K	(1 << 16)
#define			AT91_CIDR_SRAMSIZ_2K	(2 << 16)
#define			AT91_CIDR_SRAMSIZ_112K	(4 << 16)
#define			AT91_CIDR_SRAMSIZ_4K	(5 << 16)
#define			AT91_CIDR_SRAMSIZ_80K	(6 << 16)
#define			AT91_CIDR_SRAMSIZ_160K	(7 << 16)
#define			AT91_CIDR_SRAMSIZ_8K	(8 << 16)
#define			AT91_CIDR_SRAMSIZ_16K	(9 << 16)
#define			AT91_CIDR_SRAMSIZ_32K	(10 << 16)
#define			AT91_CIDR_SRAMSIZ_64K	(11 << 16)
#define			AT91_CIDR_SRAMSIZ_128K	(12 << 16)
#define			AT91_CIDR_SRAMSIZ_256K	(13 << 16)
#define			AT91_CIDR_SRAMSIZ_96K	(14 << 16)
#define			AT91_CIDR_SRAMSIZ_512K	(15 << 16)
#define		AT91_CIDR_ARCH		(0xff << 20)		
#define		AT91_CIDR_NVPTYP	(7    << 28)		
#define		AT91_CIDR_EXT		(1    << 31)		

#endif
