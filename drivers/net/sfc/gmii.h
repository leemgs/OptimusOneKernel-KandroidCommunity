

#ifndef EFX_GMII_H
#define EFX_GMII_H



#include <linux/mii.h>


#define GMII_IER		0x12	
#define GMII_ISR		0x13	


#define IER_ANEG_ERR		0x8000	
#define IER_SPEED_CHG		0x4000	
#define IER_DUPLEX_CHG		0x2000	
#define IER_PAGE_RCVD		0x1000	
#define IER_ANEG_DONE		0x0800	
#define IER_LINK_CHG		0x0400	
#define IER_SYM_ERR		0x0200	
#define IER_FALSE_CARRIER	0x0100	
#define IER_FIFO_ERR		0x0080	
#define IER_MDIX_CHG		0x0040	
#define IER_DOWNSHIFT		0x0020	
#define IER_ENERGY		0x0010	
#define IER_DTE_POWER		0x0004	
#define IER_POLARITY_CHG	0x0002	
#define IER_JABBER		0x0001	


#define ISR_ANEG_ERR		0x8000	
#define ISR_SPEED_CHG		0x4000	
#define ISR_DUPLEX_CHG		0x2000	
#define ISR_PAGE_RCVD		0x1000	
#define ISR_ANEG_DONE		0x0800	
#define ISR_LINK_CHG		0x0400	
#define ISR_SYM_ERR		0x0200	
#define ISR_FALSE_CARRIER	0x0100	
#define ISR_FIFO_ERR		0x0080	
#define ISR_MDIX_CHG		0x0040	
#define ISR_DOWNSHIFT		0x0020	
#define ISR_ENERGY		0x0010	
#define ISR_DTE_POWER		0x0004	
#define ISR_POLARITY_CHG	0x0002	
#define ISR_JABBER		0x0001	

#endif 
