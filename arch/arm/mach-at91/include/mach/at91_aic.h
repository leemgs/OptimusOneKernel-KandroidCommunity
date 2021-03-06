

#ifndef AT91_AIC_H
#define AT91_AIC_H

#define AT91_AIC_SMR(n)		(AT91_AIC + ((n) * 4))	
#define		AT91_AIC_PRIOR		(7 << 0)		
#define		AT91_AIC_SRCTYPE	(3 << 5)		
#define			AT91_AIC_SRCTYPE_LOW		(0 << 5)
#define			AT91_AIC_SRCTYPE_FALLING	(1 << 5)
#define			AT91_AIC_SRCTYPE_HIGH		(2 << 5)
#define			AT91_AIC_SRCTYPE_RISING		(3 << 5)

#define AT91_AIC_SVR(n)		(AT91_AIC + 0x80 + ((n) * 4))	
#define AT91_AIC_IVR		(AT91_AIC + 0x100)	
#define AT91_AIC_FVR		(AT91_AIC + 0x104)	
#define AT91_AIC_ISR		(AT91_AIC + 0x108)	
#define		AT91_AIC_IRQID		(0x1f << 0)		

#define AT91_AIC_IPR		(AT91_AIC + 0x10c)	
#define AT91_AIC_IMR		(AT91_AIC + 0x110)	
#define AT91_AIC_CISR		(AT91_AIC + 0x114)	
#define		AT91_AIC_NFIQ		(1 << 0)		
#define		AT91_AIC_NIRQ		(1 << 1)		

#define AT91_AIC_IECR		(AT91_AIC + 0x120)	
#define AT91_AIC_IDCR		(AT91_AIC + 0x124)	
#define AT91_AIC_ICCR		(AT91_AIC + 0x128)	
#define AT91_AIC_ISCR		(AT91_AIC + 0x12c)	
#define AT91_AIC_EOICR		(AT91_AIC + 0x130)	
#define AT91_AIC_SPU		(AT91_AIC + 0x134)	
#define AT91_AIC_DCR		(AT91_AIC + 0x138)	
#define		AT91_AIC_DCR_PROT	(1 << 0)		
#define		AT91_AIC_DCR_GMSK	(1 << 1)		

#define AT91_AIC_FFER		(AT91_AIC + 0x140)	
#define AT91_AIC_FFDR		(AT91_AIC + 0x144)	
#define AT91_AIC_FFSR		(AT91_AIC + 0x148)	

#endif
