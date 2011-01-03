

#ifndef _link_h
#define _link_h 1



#define BOOT_REQUEST       ((ushort) 0)	
#define BOOT_ABORT         ((ushort) 1)	
#define BOOT_SEQUENCE      ((ushort) 2)	
#define BOOT_COMPLETED     ((ushort) 3)	


struct LPB {
	u16 link_number;	
	u16 in_ch;	
	u16 out_ch;	
	u8 attached_serial[4];  
	u8 attached_host_serial[4];
	
	u16 descheduled;	
	u16 state;		
	u16 send_poll;		
	u16 ltt_p;	
	u16 lrt_p;	
	u16 lrt_status;		
	u16 ltt_status;		
	u16 timeout;		
	u16 topology;		
	u16 mon_ltt;
	u16 mon_lrt;
	u16 WaitNoBoot;	
	u16 add_packet_list;	
	u16 remove_packet_list;	

	u16 lrt_fail_chan;	
	u16 ltt_fail_chan;	

	
	struct RUP rup;
	struct RUP link_rup;	
	u16 attached_link;	
	u16 csum_errors;	
	u16 num_disconnects;	
	u16 num_sync_rcvd;	
	u16 num_sync_rqst;	
	u16 num_tx;		
	u16 num_rx;		
	u16 module_attached;	
	u16 led_timeout;	
	u16 first_port;		
	u16 last_port;		
};

#endif


