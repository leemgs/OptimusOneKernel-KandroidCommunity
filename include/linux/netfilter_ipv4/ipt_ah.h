#ifndef _IPT_AH_H
#define _IPT_AH_H

struct ipt_ah
{
	u_int32_t spis[2];			
	u_int8_t  invflags;			
};




#define IPT_AH_INV_SPI		0x01	
#define IPT_AH_INV_MASK	0x01	

#endif 
