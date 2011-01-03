

#ifndef AF_IRDA_H
#define AF_IRDA_H

#include <linux/irda.h>
#include <net/irda/irda.h>
#include <net/irda/iriap.h>		
#include <net/irda/irias_object.h>	
#include <net/irda/irlmp.h>		
#include <net/irda/irttp.h>		
#include <net/irda/discovery.h>		
#include <net/sock.h>


struct irda_sock {
	
	struct sock sk;
	__u32 saddr;          
	__u32 daddr;          

	struct lsap_cb *lsap; 
	__u8  pid;            

	struct tsap_cb *tsap; 
	__u8 dtsap_sel;       
	__u8 stsap_sel;       
	
	__u32 max_sdu_size_rx;
	__u32 max_sdu_size_tx;
	__u32 max_data_size;
	__u8  max_header_size;
	struct qos_info qos_tx;

	__u16_host_order mask;           
	__u16_host_order hints;          

	void *ckey;           
	void *skey;           

	struct ias_object *ias_obj;   
	struct iriap_cb *iriap;	      
	struct ias_value *ias_result; 

	hashbin_t *cachelog;		
	__u32 cachedaddr;	

	int nslots;           

	int errno;            

	wait_queue_head_t query_wait;	
	struct timer_list watchdog;	

	LOCAL_FLOW tx_flow;
	LOCAL_FLOW rx_flow;
};

static inline struct irda_sock *irda_sk(struct sock *sk)
{
	return (struct irda_sock *)sk;
}

#endif 
