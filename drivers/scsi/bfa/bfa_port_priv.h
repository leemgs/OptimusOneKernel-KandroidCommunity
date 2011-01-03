

#ifndef __BFA_PORT_PRIV_H__
#define __BFA_PORT_PRIV_H__

#include <defs/bfa_defs_pport.h>
#include <bfi/bfi_pport.h>
#include "bfa_intr_priv.h"


struct bfa_pport_s {
	struct bfa_s 		*bfa;	
	bfa_sm_t		sm;	
	wwn_t			nwwn;	
	wwn_t			pwwn;	
	enum bfa_pport_speed speed_sup;
					
	enum bfa_pport_speed speed;	
	enum bfa_pport_topology topology;	
	u8			myalpa;	
	u8			rsvd[3];
	struct bfa_pport_cfg_s	cfg;	
	struct bfa_qos_attr_s  qos_attr;   
	struct bfa_qos_vc_attr_s qos_vc_attr;  
	struct bfa_reqq_wait_s	reqq_wait;
					
	struct bfa_reqq_wait_s	svcreq_wait;
					
	struct bfa_reqq_wait_s	stats_reqq_wait;
					
	void			*event_cbarg;
	void			(*event_cbfn) (void *cbarg,
						bfa_pport_event_t event);
	union {
		union bfi_pport_i2h_msg_u i2hmsg;
	} event_arg;
	void			*bfad;	
	struct bfa_cb_qe_s		hcb_qe;	
	enum bfa_pport_linkstate	hcb_event;
					
	u32		msgtag;	
	u8			*stats_kva;
	u64		stats_pa;
	union bfa_pport_stats_u *stats;	
	u32		mypid : 24;
	u32		rsvd_b : 8;
	struct bfa_timer_s 	timer;	
	union bfa_pport_stats_u 	*stats_ret;
					
	bfa_status_t		stats_status;
					
	bfa_boolean_t   	stats_busy;
					
	bfa_boolean_t   	stats_qfull;
	bfa_boolean_t   	diag_busy;
					
	bfa_boolean_t   	beacon;
					
	bfa_boolean_t   	link_e2e_beacon;
					
	bfa_cb_pport_t		stats_cbfn;
					
	void			*stats_cbarg;
					
};

#define BFA_PORT_MOD(__bfa)	(&(__bfa)->modules.pport)


void	bfa_pport_isr(struct bfa_s *bfa, struct bfi_msg_s *msg);
#endif 
