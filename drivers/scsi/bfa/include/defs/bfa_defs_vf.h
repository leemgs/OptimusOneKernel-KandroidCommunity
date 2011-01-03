

#ifndef __BFA_DEFS_VF_H__
#define __BFA_DEFS_VF_H__

#include <bfa_os_inc.h>
#include <defs/bfa_defs_port.h>
#include <protocol/types.h>


enum bfa_vf_state {
	BFA_VF_UNINIT    = 0,	
	BFA_VF_LINK_DOWN = 1,	
	BFA_VF_FLOGI     = 2,	
	BFA_VF_AUTH      = 3,	
	BFA_VF_NOFABRIC  = 4,	
	BFA_VF_ONLINE    = 5,	
	BFA_VF_EVFP      = 6,	
	BFA_VF_ISOLATED  = 7,	
};


struct bfa_vf_stats_s {
	u32        flogi_sent;	
	u32        flogi_rsp_err;	
	u32        flogi_acc_err;	
	u32        flogi_accepts;	
	u32        flogi_rejects;	
	u32        flogi_unknown_rsp; 
	u32        flogi_alloc_wait; 
	u32        flogi_rcvd;	
	u32        flogi_rejected;	
	u32        fabric_onlines;	
	u32        fabric_offlines; 
	u32        resvd;
};


struct bfa_vf_attr_s {
	enum bfa_vf_state  state;		
	u32        rsvd;
	wwn_t           fabric_name;	
};

#endif 
