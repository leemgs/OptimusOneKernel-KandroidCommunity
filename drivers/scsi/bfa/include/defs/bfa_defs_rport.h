

#ifndef __BFA_DEFS_RPORT_H__
#define __BFA_DEFS_RPORT_H__

#include <bfa_os_inc.h>
#include <protocol/types.h>
#include <defs/bfa_defs_pport.h>
#include <defs/bfa_defs_port.h>
#include <defs/bfa_defs_qos.h>


enum bfa_rport_state {
	BFA_RPORT_UNINIT 	= 0,	
	BFA_RPORT_OFFLINE 	= 1,	
	BFA_RPORT_PLOGI 	= 2,	
	BFA_RPORT_ONLINE 	= 3,	
	BFA_RPORT_PLOGI_RETRY 	= 4,	
	BFA_RPORT_NSQUERY 	= 5,	
	BFA_RPORT_ADISC 	= 6,	
	BFA_RPORT_LOGO 		= 7,	
	BFA_RPORT_LOGORCV 	= 8,	
	BFA_RPORT_NSDISC 	= 9,	
};


enum bfa_rport_function {
	BFA_RPORT_INITIATOR 	= 0x01,	
	BFA_RPORT_TARGET 	= 0x02,	
};


#define BFA_RPORT_SYMNAME_MAXLEN	255
struct bfa_rport_symname_s {
	char            symname[BFA_RPORT_SYMNAME_MAXLEN];
};

struct bfa_rport_hal_stats_s {
	u32        sm_un_cr;	    
	u32        sm_un_unexp;	    
	u32        sm_cr_on;	    
	u32        sm_cr_del;	    
	u32        sm_cr_hwf;	    
	u32        sm_cr_unexp;	    
	u32        sm_fwc_rsp;	    
	u32        sm_fwc_del;	    
	u32        sm_fwc_off;	    
	u32        sm_fwc_hwf;	    
	u32        sm_fwc_unexp;	    
	u32        sm_on_off;	    
	u32        sm_on_del;	    
	u32        sm_on_hwf;	    
	u32        sm_on_unexp;	    
	u32        sm_fwd_rsp;	    
	u32        sm_fwd_del;	    
	u32        sm_fwd_hwf;	    
	u32        sm_fwd_unexp;	    
	u32        sm_off_del;	    
	u32        sm_off_on;	    
	u32        sm_off_hwf;	    
	u32        sm_off_unexp;	    
	u32        sm_del_fwrsp;	    
	u32        sm_del_hwf;	    
	u32        sm_del_unexp;	    
	u32        sm_delp_fwrsp;	    
	u32        sm_delp_hwf;	    
	u32        sm_delp_unexp;	    
	u32        sm_offp_fwrsp;	    
	u32        sm_offp_del;	    
	u32        sm_offp_hwf;	    
	u32        sm_offp_unexp;	    
	u32        sm_iocd_off;	    
	u32        sm_iocd_del;	    
	u32        sm_iocd_on;	    
	u32        sm_iocd_unexp;	    
	u32        rsvd;
};


struct bfa_rport_stats_s {
	u32        offlines;           
	u32        onlines;            
	u32        rscns;              
	u32        plogis;		    
	u32        plogi_accs;	    
	u32        plogi_timeouts;	    
	u32        plogi_rejects;	    
	u32        plogi_failed;	    
	u32        plogi_rcvd;	    
	u32        prli_rcvd;          
	u32        adisc_rcvd;         
	u32        adisc_rejects;      
	u32        adisc_sent;         
	u32        adisc_accs;         
	u32        adisc_failed;       
	u32        adisc_rejected;     
	u32        logos;              
	u32        logo_accs;          
	u32        logo_failed;        
	u32        logo_rejected;      
	u32        logo_rcvd;          

	u32        rpsc_rcvd;         
	u32        rpsc_rejects;      
	u32        rpsc_sent;         
	u32        rpsc_accs;         
	u32        rpsc_failed;       
	u32        rpsc_rejected;     

	u32        rsvd;
	struct bfa_rport_hal_stats_s	hal_stats;  
};


struct bfa_rport_qos_attr_s {
	enum bfa_qos_priority qos_priority;  
	u32	       qos_flow_id;	  
};


struct bfa_rport_attr_s {
	wwn_t           	nwwn;	
	wwn_t           	pwwn;	
	enum fc_cos cos_supported;	
	u32        	pid;	
	u32        	df_sz;	
	enum bfa_rport_state 	state;	
	enum fc_cos        	fc_cos;	
	bfa_boolean_t   	cisc;	
	struct bfa_rport_symname_s symname; 
	enum bfa_rport_function	scsi_function; 
	struct bfa_rport_qos_attr_s qos_attr; 
	enum bfa_pport_speed curr_speed;   
	bfa_boolean_t 	trl_enforced;	
	enum bfa_pport_speed	assigned_speed;	
};

#define bfa_rport_aen_qos_data_t struct bfa_rport_qos_attr_s


enum bfa_rport_aen_event {
	BFA_RPORT_AEN_ONLINE      = 1,	
	BFA_RPORT_AEN_OFFLINE     = 2,	
	BFA_RPORT_AEN_DISCONNECT  = 3,	
	BFA_RPORT_AEN_QOS_PRIO    = 4,	
	BFA_RPORT_AEN_QOS_FLOWID  = 5,	
};

struct bfa_rport_aen_data_s {
	u16        vf_id;	
	u16        rsvd[3];
	wwn_t           ppwwn;	
	wwn_t           lpwwn;	
	wwn_t           rpwwn;	
	union {
		bfa_rport_aen_qos_data_t qos;
	} priv;
};

#endif 
