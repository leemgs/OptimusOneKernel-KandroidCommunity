

#ifndef KS8695_SYS_H
#define KS8695_SYS_H

#define KS8695_SYS_OFFSET	(0xF0000 + 0x0000)
#define KS8695_SYS_VA		(KS8695_IO_VA + KS8695_SYS_OFFSET)
#define KS8695_SYS_PA		(KS8695_IO_PA + KS8695_SYS_OFFSET)


#define KS8695_SYSCFG		(0x00)		
#define KS8695_CLKCON		(0x04)		



#define SYSCFG_SPRBP		(0x3ff << 16)	


#define CLKCON_SFMODE		(1 << 8)	
#define CLKCON_SCDC		(7 << 0)	


#endif
