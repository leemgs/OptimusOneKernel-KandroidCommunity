


#ifndef	_SBPCMCIA_H
#define	_SBPCMCIA_H




#define	PCMCIA_FCR		(0x700 / 2)

#define	FCR0_OFF		0
#define	FCR1_OFF		(0x40 / 2)
#define	FCR2_OFF		(0x80 / 2)
#define	FCR3_OFF		(0xc0 / 2)

#define	PCMCIA_FCR0		(0x700 / 2)
#define	PCMCIA_FCR1		(0x740 / 2)
#define	PCMCIA_FCR2		(0x780 / 2)
#define	PCMCIA_FCR3		(0x7c0 / 2)



#define	PCMCIA_COR		0

#define	COR_RST			0x80
#define	COR_LEV			0x40
#define	COR_IRQEN		0x04
#define	COR_BLREN		0x01
#define	COR_FUNEN		0x01


#define	PCICIA_FCSR		(2 / 2)
#define	PCICIA_PRR		(4 / 2)
#define	PCICIA_SCR		(6 / 2)
#define	PCICIA_ESR		(8 / 2)


#define PCM_MEMOFF		0x0000
#define F0_MEMOFF		0x1000
#define F1_MEMOFF		0x2000
#define F2_MEMOFF		0x3000
#define F3_MEMOFF		0x4000


#define MEM_ADDR0		(0x728 / 2)
#define MEM_ADDR1		(0x72a / 2)
#define MEM_ADDR2		(0x72c / 2)


#define PCMCIA_ADDR0		(0x072e / 2)
#define PCMCIA_ADDR1		(0x0730 / 2)
#define PCMCIA_ADDR2		(0x0732 / 2)

#define MEM_SEG			(0x0734 / 2)
#define SROM_CS			(0x0736 / 2)
#define SROM_DATAL		(0x0738 / 2)
#define SROM_DATAH		(0x073a / 2)
#define SROM_ADDRL		(0x073c / 2)
#define SROM_ADDRH		(0x073e / 2)
#define	SROM_INFO2		(0x0772 / 2)	
#define	SROM_INFO		(0x07be / 2)	


#define SROM_IDLE		0
#define SROM_WRITE		1
#define SROM_READ		2
#define SROM_WEN		4
#define SROM_WDS		7
#define SROM_DONE		8


#define	SRI_SZ_MASK		0x03
#define	SRI_BLANK		0x04
#define	SRI_OTP			0x80



#define SBTML_INT_ACK		0x40000		
#define SBTML_INT_EN		0x20000		


#define SBTMH_INT_STATUS	0x40000		

#endif	
