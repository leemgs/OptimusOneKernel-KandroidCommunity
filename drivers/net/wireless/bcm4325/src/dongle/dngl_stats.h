

#ifndef _dngl_stats_h_
#define _dngl_stats_h_

typedef struct {
	unsigned long	rx_packets;		
	unsigned long	tx_packets;		
	unsigned long	rx_bytes;		
	unsigned long	tx_bytes;		
	unsigned long	rx_errors;		
	unsigned long	tx_errors;		
	unsigned long	rx_dropped;		
	unsigned long	tx_dropped;		
	unsigned long   multicast;      
} dngl_stats_t;

#endif 
