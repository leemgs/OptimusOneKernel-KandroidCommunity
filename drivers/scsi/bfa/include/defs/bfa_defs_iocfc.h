

#ifndef __BFA_DEFS_IOCFC_H__
#define __BFA_DEFS_IOCFC_H__

#include <protocol/types.h>
#include <defs/bfa_defs_types.h>
#include <defs/bfa_defs_version.h>
#include <defs/bfa_defs_adapter.h>
#include <defs/bfa_defs_pm.h>

#define BFA_IOCFC_INTR_DELAY	1125
#define BFA_IOCFC_INTR_LATENCY	225


struct bfa_iocfc_intr_attr_s {
	bfa_boolean_t	coalesce;	
	u16	latency;	
	u16	delay;		
};


struct bfa_iocfc_fwcfg_s {
	u16        num_fabrics;	
	u16        num_lports;	
	u16        num_rports;	
	u16        num_ioim_reqs;	
	u16        num_tskim_reqs;	
	u16        num_iotm_reqs;	
	u16        num_tsktm_reqs;	
	u16        num_fcxp_reqs;	
	u16        num_uf_bufs;	
	u8		num_cqs;
	u8		rsvd;
};

struct bfa_iocfc_drvcfg_s {
	u16        num_reqq_elems;	
	u16        num_rspq_elems;	
	u16        num_sgpgs;	
	u16        num_sboot_tgts;	
	u16        num_sboot_luns;	
	u16	    ioc_recover;	
	u16	    min_cfg;	
	u16        path_tov;	
	bfa_boolean_t   delay_comp; 
	u32		rsvd;
};

struct bfa_iocfc_cfg_s {
	struct bfa_iocfc_fwcfg_s	fwcfg;	
	struct bfa_iocfc_drvcfg_s	drvcfg;	
};


struct bfa_fw_io_stats_s {
	u32	host_abort;		
	u32	host_cleanup;		

	u32	fw_io_timeout;		
	u32	fw_frm_parse;		
	u32	fw_frm_data;		
	u32	fw_frm_rsp;		
	u32	fw_frm_xfer_rdy;	
	u32	fw_frm_bls_acc;		
	u32	fw_frm_tgt_abort;	
	u32	fw_frm_unknown;		
	u32	fw_data_dma;		
	u32	fw_frm_drop;		

	u32	rec_timeout;		
	u32	error_rec;			
	u32	wait_for_si;		
	u32	rec_rsp_inval;		
	u32	seqr_io_abort;		
	u32	seqr_io_retry;		

	u32	itn_cisc_upd_rsp;	
	u32	itn_cisc_upd_data;	
	u32	itn_cisc_upd_xfer_rdy;	

	u32	fcp_data_lost;		

	u32	ro_set_in_xfer_rdy;	
	u32	xfer_rdy_ooo_err;	
	u32	xfer_rdy_unknown_err;	

	u32	io_abort_timeout;	
	u32	sler_initiated;		

	u32	unexp_fcp_rsp;		

	u32	fcp_rsp_under_run;	
	u32        fcp_rsp_under_run_wr;   
	u32	fcp_rsp_under_run_err;	
	u32        fcp_rsp_resid_inval;    
	u32	fcp_rsp_over_run;	
	u32	fcp_rsp_over_run_err;	
	u32	fcp_rsp_proto_err;	
	u32	fcp_rsp_sense_err;	
	u32	fcp_conf_req;		

	u32	tgt_aborted_io;		

	u32	ioh_edtov_timeout_event;
	u32	ioh_fcp_rsp_excp_event;	
	u32	ioh_fcp_conf_event;	
	u32	ioh_mult_frm_rsp_event;	
	u32	ioh_hit_class2_event;	
	u32	ioh_miss_other_event;	
	u32	ioh_seq_cnt_err_event;	
	u32	ioh_len_err_event;		
	u32	ioh_seq_len_err_event;	
	u32	ioh_data_oor_event;	
	u32	ioh_ro_ooo_event;	
	u32	ioh_cpu_owned_event;	
	u32	ioh_unexp_frame_event;	
	u32	ioh_err_int;		
};



struct bfa_fw_port_fpg_stats_s {
    u32    intr_evt;
    u32    intr;
    u32    intr_excess;
    u32    intr_cause0;
    u32    intr_other;
    u32    intr_other_ign;
    u32    sig_lost;
    u32    sig_regained;
    u32    sync_lost;
    u32    sync_to;
    u32    sync_regained;
    u32    div2_overflow;
    u32    div2_underflow;
    u32    efifo_overflow;
    u32    efifo_underflow;
    u32    idle_rx;
    u32    lrr_rx;
    u32    lr_rx;
    u32    ols_rx;
    u32    nos_rx;
    u32    lip_rx;
    u32    arbf0_rx;
    u32    mrk_rx;
    u32    const_mrk_rx;
    u32    prim_unknown;
    u32    rsvd;
};


struct bfa_fw_port_lksm_stats_s {
    u32    hwsm_success;       
    u32    hwsm_fails;         
    u32    hwsm_wdtov;         
    u32    swsm_success;       
    u32    swsm_fails;         
    u32    swsm_wdtov;         
    u32    busybufs;           
    u32    buf_waits;          
    u32    link_fails;         
    u32    psp_errors;         
    u32    lr_unexp;           
    u32    lrr_unexp;          
    u32    lr_tx;              
    u32    lrr_tx;             
    u32    ols_tx;             
    u32    nos_tx;             
};


struct bfa_fw_port_snsm_stats_s {
    u32    hwsm_success;       
    u32    hwsm_fails;         
    u32    hwsm_wdtov;         
    u32    swsm_success;       
    u32    swsm_wdtov;         
    u32    error_resets;       
    u32    sync_lost;          
    u32    sig_lost;           
};


struct bfa_fw_port_physm_stats_s {
    u32    module_inserts;     
    u32    module_xtracts;     
    u32    module_invalids;    
    u32    module_read_ign;    
    u32    laser_faults;       
    u32    rsvd;
};


struct bfa_fw_fip_stats_s {
    u32    disc_req;           
    u32    disc_rsp;           
    u32    disc_err;           
    u32    disc_unsol;         
    u32    disc_timeouts;      
    u32    linksvc_unsupp;     
    u32    linksvc_err;        
    u32    logo_req;           
    u32    clrvlink_req;       
    u32    op_unsupp;          
    u32    untagged;           
    u32    rsvd;
};


struct bfa_fw_lps_stats_s {
    u32    mac_invalids;       
    u32    rsvd;
};


struct bfa_fw_fcoe_stats_s {
    u32    cee_linkups;        
    u32    cee_linkdns;        
    u32    fip_linkups;        
    u32    fip_linkdns;        
    u32    fip_fails;          
    u32    mac_invalids;       
};


struct bfa_fw_fcoe_port_stats_s {
    struct bfa_fw_fcoe_stats_s  fcoe_stats;
    struct bfa_fw_fip_stats_s   fip_stats;
};


struct bfa_fw_fc_port_stats_s {
	struct bfa_fw_port_fpg_stats_s		fpg_stats;
	struct bfa_fw_port_physm_stats_s	physm_stats;
	struct bfa_fw_port_snsm_stats_s		snsm_stats;
	struct bfa_fw_port_lksm_stats_s		lksm_stats;
};


union bfa_fw_port_stats_s {
	struct bfa_fw_fc_port_stats_s	fc_stats;
	struct bfa_fw_fcoe_port_stats_s	fcoe_stats;
};


struct bfa_fw_stats_s {
	struct bfa_fw_ioc_stats_s	ioc_stats;
	struct bfa_fw_io_stats_s	io_stats;
	union  bfa_fw_port_stats_s	port_stats;
};


struct bfa_iocfc_stats_s {
	struct bfa_fw_stats_s 	fw_stats;	
};


struct bfa_iocfc_attr_s {
	struct bfa_iocfc_cfg_s		config;		
	struct bfa_iocfc_intr_attr_s	intr_attr;	
};

#define BFA_IOCFC_PATHTOV_MAX	60
#define BFA_IOCFC_QDEPTH_MAX	2000

#endif 
