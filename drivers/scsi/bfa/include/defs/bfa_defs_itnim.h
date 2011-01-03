
#ifndef __BFA_DEFS_ITNIM_H__
#define __BFA_DEFS_ITNIM_H__

#include <bfa_os_inc.h>
#include <protocol/types.h>


enum bfa_itnim_state {
	BFA_ITNIM_OFFLINE 	= 0,	
	BFA_ITNIM_PRLI_SEND 	= 1,	
	BFA_ITNIM_PRLI_SENT 	= 2,	
	BFA_ITNIM_PRLI_RETRY 	= 3,	
	BFA_ITNIM_HCB_ONLINE 	= 4,	
	BFA_ITNIM_ONLINE 	= 5,	
	BFA_ITNIM_HCB_OFFLINE 	= 6,	
	BFA_ITNIM_INITIATIOR 	= 7,	
};

struct bfa_itnim_hal_stats_s {
	u32	onlines;	
	u32	offlines;	
	u32	creates;	
	u32	deletes;	
	u32	create_comps;	
	u32	delete_comps;	
	u32	sler_events;	
	u32	ioc_disabled;	
	u32	cleanup_comps;	
	u32	tm_cmnds;	
	u32	tm_fw_rsps;	
	u32	tm_success;	
	u32	tm_failures;	
	u32	tm_io_comps;	
	u32	tm_qresumes;	
	u32	tm_iocdowns;	
	u32	tm_cleanups;	
	u32	tm_cleanup_comps;
					
	u32	ios;		
	u32	io_comps;	
	u64	input_reqs;	
	u64	output_reqs;	
};


struct bfa_itnim_stats_s {
	u32        onlines;	
	u32        offlines;	
	u32        prli_sent;	
	u32        fcxp_alloc_wait;
	u32        prli_rsp_err;	
	u32        prli_rsp_acc;	
	u32        initiator;	
	u32        prli_rsp_parse_err;	
	u32        prli_rsp_rjt;	
	u32        timeout;	
	u32        sler;		
	u32	rsvd;
	struct bfa_itnim_hal_stats_s	hal_stats;
};


struct bfa_itnim_attr_s {
	enum bfa_itnim_state state; 
	u8 retry;		
	u8	task_retry_id;  
	u8 rec_support;    
	u8 conf_comp;      
};


enum bfa_itnim_aen_event {
	BFA_ITNIM_AEN_ONLINE 	= 1,	
	BFA_ITNIM_AEN_OFFLINE 	= 2,	
	BFA_ITNIM_AEN_DISCONNECT = 3,	
};


struct bfa_itnim_aen_data_s {
	u16        vf_id;	
	u16        rsvd[3];
	wwn_t           ppwwn;	
	wwn_t           lpwwn;	
	wwn_t           rpwwn;	
};

#endif 
