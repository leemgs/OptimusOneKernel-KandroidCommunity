

#ifndef __BFA_DEFS_VPORT_H__
#define __BFA_DEFS_VPORT_H__

#include <bfa_os_inc.h>
#include <defs/bfa_defs_port.h>
#include <protocol/types.h>


enum bfa_vport_state {
	BFA_FCS_VPORT_UNINIT 		= 0,
	BFA_FCS_VPORT_CREATED 		= 1,
	BFA_FCS_VPORT_OFFLINE 		= 1,
	BFA_FCS_VPORT_FDISC_SEND 	= 2,
	BFA_FCS_VPORT_FDISC 		= 3,
	BFA_FCS_VPORT_FDISC_RETRY 	= 4,
	BFA_FCS_VPORT_ONLINE 		= 5,
	BFA_FCS_VPORT_DELETING 		= 6,
	BFA_FCS_VPORT_CLEANUP 		= 6,
	BFA_FCS_VPORT_LOGO_SEND 	= 7,
	BFA_FCS_VPORT_LOGO 			= 8,
	BFA_FCS_VPORT_ERROR			= 9,
	BFA_FCS_VPORT_MAX_STATE,
};


struct bfa_vport_stats_s {
	struct bfa_port_stats_s port_stats;	
	

	u32        fdisc_sent;	
	u32        fdisc_accepts;	
	u32        fdisc_retries;	
	u32        fdisc_timeouts;	
	u32        fdisc_rsp_err;	
	u32        fdisc_acc_bad;	
	u32        fdisc_rejects;	
	u32        fdisc_unknown_rsp;
	
	u32        fdisc_alloc_wait;

	u32        logo_alloc_wait;
	u32        logo_sent;	
	u32        logo_accepts;	
	u32        logo_rejects;	
	u32        logo_rsp_err;	
	u32        logo_unknown_rsp;
			

	u32        fab_no_npiv;	

	u32        fab_offline;	
	u32        fab_online;	
	u32        fab_cleanup;	
	u32        rsvd;
};


struct bfa_vport_attr_s {
	struct bfa_port_attr_s   port_attr; 
	enum bfa_vport_state vport_state; 
	u32          rsvd;
};

#endif 
