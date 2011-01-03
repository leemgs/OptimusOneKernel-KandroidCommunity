

#ifndef __BFA_DEFS_QOS_H__
#define __BFA_DEFS_QOS_H__


enum bfa_qos_state {
	BFA_QOS_ONLINE = 1,		
	BFA_QOS_OFFLINE = 2,		
};



enum bfa_qos_priority {
	BFA_QOS_UNKNOWN = 0,
	BFA_QOS_HIGH  = 1,	
	BFA_QOS_MED  =  2,	
	BFA_QOS_LOW  =  3,	
};



enum bfa_qos_bw_alloc {
	BFA_QOS_BW_HIGH  = 60,	
	BFA_QOS_BW_MED  =  30,	
	BFA_QOS_BW_LOW  =  10,	
};


struct bfa_qos_attr_s {
	enum bfa_qos_state state;		
	u32  total_bb_cr;  	 	
};


#define  BFA_QOS_MAX_VC  16

struct bfa_qos_vc_info_s {
	u8 vc_credit;
	u8 borrow_credit;
	u8 priority;
	u8 resvd;
};

struct bfa_qos_vc_attr_s {
	u16  total_vc_count;                    
	u16  shared_credit;
	u32  elp_opmode_flags;
	struct bfa_qos_vc_info_s vc_info[BFA_QOS_MAX_VC];  
};


struct bfa_qos_stats_s {
	u32	flogi_sent; 		
	u32	flogi_acc_recvd;	
	u32	flogi_rjt_recvd; 
	u32	flogi_retries;		

	u32	elp_recvd; 	   	
	u32	elp_accepted;       
	u32	elp_rejected;       
	u32	elp_dropped;        

	u32	qos_rscn_recvd;     
	u32	rsvd; 		
};

#endif 
