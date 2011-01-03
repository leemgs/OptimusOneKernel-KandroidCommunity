

#ifndef _parmap_h
#define _parmap_h

typedef struct PARM_MAP PARM_MAP;

struct PARM_MAP {
	u16 phb_ptr;	
	u16 phb_num_ptr;	
	u16 free_list;	
	u16 free_list_end;	
	u16 q_free_list_ptr;	
	u16 unit_id_ptr;	
	u16 link_str_ptr;	
	u16 bootloader_1;	
	u16 bootloader_2;	
	u16 port_route_map_ptr;	
	u16 route_ptr;		
	u16 map_present;	
	s16 pkt_num;		
	s16 q_num;		
	u16 buffers_per_port;	
	u16 heap_size;		
	u16 heap_left;		
	u16 error;		
	u16 tx_max;		
	u16 rx_max;		
	u16 rx_limit;		
	s16 links;		
	s16 timer;		
	u16 rups;		
	u16 max_phb;		
	u16 living;		
	u16 init_done;		
	u16 booting_link;
	u16 idle_count;		
	u16 busy_count;		
	u16 idle_control;	
	u16 tx_intr;		
	u16 rx_intr;		
	u16 rup_intr;		
};

#endif


