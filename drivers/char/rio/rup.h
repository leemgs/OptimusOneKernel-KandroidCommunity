

#ifndef _rup_h
#define _rup_h 1

#define MAX_RUP          ((short) 16)
#define PKTS_PER_RUP     ((short) 2)	


#define TX_RUP_INACTIVE          0	
#define TX_PACKET_READY          1	
#define TX_LOCK_RUP              2	

#define RX_RUP_INACTIVE          0	
#define RX_PACKET_READY          1	

#define RUP_NO_OWNER             0xff	

struct RUP {
	u16 txpkt;		
	u16 rxpkt;		
	u16 link;		
	u8 rup_dest_unit[2];	
	u16 handshake;		
	u16 timeout;		
	u16 status;		
	u16 txcontrol;		
	u16 rxcontrol;		
};

#endif


