

#ifndef __BFA_FCS_FABRIC_H__
#define __BFA_FCS_FABRIC_H__

struct bfa_fcs_s;

#include <defs/bfa_defs_status.h>
#include <defs/bfa_defs_vf.h>
#include <cs/bfa_q.h>
#include <cs/bfa_sm.h>
#include <defs/bfa_defs_pport.h>
#include <fcs/bfa_fcs_lport.h>
#include <protocol/fc_sp.h>
#include <fcs/bfa_fcs_auth.h>


struct bfad_vf_s;

enum bfa_fcs_fabric_type {
	BFA_FCS_FABRIC_UNKNOWN = 0,
	BFA_FCS_FABRIC_SWITCHED = 1,
	BFA_FCS_FABRIC_PLOOP = 2,
	BFA_FCS_FABRIC_N2N = 3,
};


struct bfa_fcs_fabric_s {
	struct list_head   qe;		
	bfa_sm_t	 sm;		
	struct bfa_fcs_s *fcs;		
	struct bfa_fcs_port_s  bport;	
	enum bfa_fcs_fabric_type fab_type; 
	enum bfa_pport_type oper_type;	
	u8         is_vf;		
	u8         is_npiv;	
	u8         is_auth;	
	u16        bb_credit;	
	u16        vf_id;		
	u16        num_vports;	
	u16        rsvd;
	struct list_head         vport_q;	
	struct list_head         vf_q;	
	struct bfad_vf_s      *vf_drv;	
	struct bfa_timer_s link_timer;	
	wwn_t           fabric_name;	
	bfa_boolean_t   auth_reqd;	
	struct bfa_timer_s delay_timer;	
	union {
		u16        swp_vfid;
	} event_arg;
	struct bfa_fcs_auth_s  auth;	
	struct bfa_wc_s        wc;	
	struct bfa_vf_stats_s  stats; 	
	struct bfa_lps_s	*lps;	
	u8	fabric_ip_addr[BFA_FCS_FABRIC_IPADDR_SZ];  
};

#define bfa_fcs_fabric_npiv_capable(__f)    (__f)->is_npiv
#define bfa_fcs_fabric_is_switched(__f)			\
	((__f)->fab_type == BFA_FCS_FABRIC_SWITCHED)


#define bfa_fcs_vf_t struct bfa_fcs_fabric_s

struct bfa_vf_event_s {
	u32        undefined;
};


bfa_status_t bfa_fcs_vf_mode_enable(struct bfa_fcs_s *fcs, u16 vf_id);
bfa_status_t bfa_fcs_vf_mode_disable(struct bfa_fcs_s *fcs);
bfa_status_t bfa_fcs_vf_create(bfa_fcs_vf_t *vf, struct bfa_fcs_s *fcs,
			       u16 vf_id, struct bfa_port_cfg_s *port_cfg,
			       struct bfad_vf_s *vf_drv);
bfa_status_t bfa_fcs_vf_delete(bfa_fcs_vf_t *vf);
void bfa_fcs_vf_start(bfa_fcs_vf_t *vf);
bfa_status_t bfa_fcs_vf_stop(bfa_fcs_vf_t *vf);
void bfa_fcs_vf_list(struct bfa_fcs_s *fcs, u16 *vf_ids, int *nvfs);
void bfa_fcs_vf_list_all(struct bfa_fcs_s *fcs, u16 *vf_ids, int *nvfs);
void bfa_fcs_vf_get_attr(bfa_fcs_vf_t *vf, struct bfa_vf_attr_s *vf_attr);
void bfa_fcs_vf_get_stats(bfa_fcs_vf_t *vf,
			  struct bfa_vf_stats_s *vf_stats);
void bfa_fcs_vf_clear_stats(bfa_fcs_vf_t *vf);
void bfa_fcs_vf_get_ports(bfa_fcs_vf_t *vf, wwn_t vpwwn[], int *nports);
bfa_fcs_vf_t *bfa_fcs_vf_lookup(struct bfa_fcs_s *fcs, u16 vf_id);
struct bfad_vf_s *bfa_fcs_vf_get_drv_vf(bfa_fcs_vf_t *vf);

#endif 
