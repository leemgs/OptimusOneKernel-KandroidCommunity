

#ifndef AT91SAM9_SMC_H
#define AT91SAM9_SMC_H

#define AT91_SMC_SETUP(n)	(AT91_SMC + 0x00 + ((n)*0x10))	
#define		AT91_SMC_NWESETUP	(0x3f << 0)			
#define			AT91_SMC_NWESETUP_(x)	((x) << 0)
#define		AT91_SMC_NCS_WRSETUP	(0x3f << 8)			
#define			AT91_SMC_NCS_WRSETUP_(x)	((x) << 8)
#define		AT91_SMC_NRDSETUP	(0x3f << 16)			
#define			AT91_SMC_NRDSETUP_(x)	((x) << 16)
#define		AT91_SMC_NCS_RDSETUP	(0x3f << 24)			
#define			AT91_SMC_NCS_RDSETUP_(x)	((x) << 24)

#define AT91_SMC_PULSE(n)	(AT91_SMC + 0x04 + ((n)*0x10))	
#define		AT91_SMC_NWEPULSE	(0x7f <<  0)			
#define			AT91_SMC_NWEPULSE_(x)	((x) << 0)
#define		AT91_SMC_NCS_WRPULSE	(0x7f <<  8)			
#define			AT91_SMC_NCS_WRPULSE_(x)((x) << 8)
#define		AT91_SMC_NRDPULSE	(0x7f << 16)			
#define			AT91_SMC_NRDPULSE_(x)	((x) << 16)
#define		AT91_SMC_NCS_RDPULSE	(0x7f << 24)			
#define			AT91_SMC_NCS_RDPULSE_(x)((x) << 24)

#define AT91_SMC_CYCLE(n)	(AT91_SMC + 0x08 + ((n)*0x10))	
#define		AT91_SMC_NWECYCLE	(0x1ff << 0 )			
#define			AT91_SMC_NWECYCLE_(x)	((x) << 0)
#define		AT91_SMC_NRDCYCLE	(0x1ff << 16)			
#define			AT91_SMC_NRDCYCLE_(x)	((x) << 16)

#define AT91_SMC_MODE(n)	(AT91_SMC + 0x0c + ((n)*0x10))	
#define		AT91_SMC_READMODE	(1 <<  0)			
#define		AT91_SMC_WRITEMODE	(1 <<  1)			
#define		AT91_SMC_EXNWMODE	(3 <<  4)			
#define			AT91_SMC_EXNWMODE_DISABLE	(0 << 4)
#define			AT91_SMC_EXNWMODE_FROZEN	(2 << 4)
#define			AT91_SMC_EXNWMODE_READY		(3 << 4)
#define		AT91_SMC_BAT		(1 <<  8)			
#define			AT91_SMC_BAT_SELECT		(0 << 8)
#define			AT91_SMC_BAT_WRITE		(1 << 8)
#define		AT91_SMC_DBW		(3 << 12)			
#define			AT91_SMC_DBW_8			(0 << 12)
#define			AT91_SMC_DBW_16			(1 << 12)
#define			AT91_SMC_DBW_32			(2 << 12)
#define		AT91_SMC_TDF		(0xf << 16)			
#define			AT91_SMC_TDF_(x)		((x) << 16)
#define		AT91_SMC_TDFMODE	(1 << 20)			
#define		AT91_SMC_PMEN		(1 << 24)			
#define		AT91_SMC_PS		(3 << 28)			
#define			AT91_SMC_PS_4			(0 << 28)
#define			AT91_SMC_PS_8			(1 << 28)
#define			AT91_SMC_PS_16			(2 << 28)
#define			AT91_SMC_PS_32			(3 << 28)

#if defined(AT91_SMC1)		
#define AT91_SMC1_SETUP(n)	(AT91_SMC1 + 0x00 + ((n)*0x10))	
#define AT91_SMC1_PULSE(n)	(AT91_SMC1 + 0x04 + ((n)*0x10))	
#define AT91_SMC1_CYCLE(n)	(AT91_SMC1 + 0x08 + ((n)*0x10))	
#define AT91_SMC1_MODE(n)	(AT91_SMC1 + 0x0c + ((n)*0x10))	
#endif

#endif
