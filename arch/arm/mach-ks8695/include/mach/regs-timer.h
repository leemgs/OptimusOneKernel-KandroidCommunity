

#ifndef KS8695_TIMER_H
#define KS8695_TIMER_H

#define KS8695_TMR_OFFSET	(0xF0000 + 0xE400)
#define KS8695_TMR_VA		(KS8695_IO_VA + KS8695_TMR_OFFSET)
#define KS8695_TMR_PA		(KS8695_IO_PA + KS8695_TMR_OFFSET)



#define KS8695_TMCON		(0x00)		
#define KS8695_T1TC		(0x04)		
#define KS8695_T0TC		(0x08)		
#define KS8695_T1PD		(0x0C)		
#define KS8695_T0PD		(0x10)		



#define TMCON_T1EN		(1 << 1)	
#define TMCON_T0EN		(1 << 0)	


#define T0TC_WATCHDOG		(0xff)		


#endif
