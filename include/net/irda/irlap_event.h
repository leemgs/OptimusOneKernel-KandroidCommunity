

#ifndef IRLAP_EVENT_H
#define IRLAP_EVENT_H

#include <net/irda/irda.h>


struct irlap_cb;
struct irlap_info;


typedef enum {
	LAP_NDM,         
	LAP_QUERY,
	LAP_REPLY,
	LAP_CONN,        
	LAP_SETUP,       
	LAP_OFFLINE,     
	LAP_XMIT_P,
	LAP_PCLOSE,
	LAP_NRM_P,       
	LAP_RESET_WAIT,
	LAP_RESET,
	LAP_NRM_S,       
	LAP_XMIT_S,
	LAP_SCLOSE,
	LAP_RESET_CHECK,
} IRLAP_STATE;


typedef enum {
	
	DISCOVERY_REQUEST,
	CONNECT_REQUEST,
	CONNECT_RESPONSE,
	DISCONNECT_REQUEST,
	DATA_REQUEST,
	RESET_REQUEST,
	RESET_RESPONSE,

	
	SEND_I_CMD,
	SEND_UI_FRAME,

	
	RECV_DISCOVERY_XID_CMD,
	RECV_DISCOVERY_XID_RSP,
	RECV_SNRM_CMD,
	RECV_TEST_CMD,
	RECV_TEST_RSP,
	RECV_UA_RSP,
	RECV_DM_RSP,
	RECV_RD_RSP,
	RECV_I_CMD,
	RECV_I_RSP,
	RECV_UI_FRAME,
	RECV_FRMR_RSP,
	RECV_RR_CMD,
	RECV_RR_RSP,
	RECV_RNR_CMD,
	RECV_RNR_RSP,
	RECV_REJ_CMD,
	RECV_REJ_RSP,
	RECV_SREJ_CMD,
	RECV_SREJ_RSP,
	RECV_DISC_CMD,

	
	SLOT_TIMER_EXPIRED,
	QUERY_TIMER_EXPIRED,
	FINAL_TIMER_EXPIRED,
	POLL_TIMER_EXPIRED,
	DISCOVERY_TIMER_EXPIRED,
	WD_TIMER_EXPIRED,
	BACKOFF_TIMER_EXPIRED,
	MEDIA_BUSY_TIMER_EXPIRED,
} IRLAP_EVENT;


typedef enum { 
	LAP_DISC_INDICATION=1, 
	LAP_NO_RESPONSE,       
	LAP_RESET_INDICATION,  
	LAP_FOUND_NONE,        
	LAP_MEDIA_BUSY,
	LAP_PRIMARY_CONFLICT,
} LAP_REASON;

extern const char *const irlap_state[];

void irlap_do_event(struct irlap_cb *self, IRLAP_EVENT event, 
		    struct sk_buff *skb, struct irlap_info *info);
void irlap_print_event(IRLAP_EVENT event);

extern int irlap_qos_negotiate(struct irlap_cb *self, struct sk_buff *skb);

#endif
