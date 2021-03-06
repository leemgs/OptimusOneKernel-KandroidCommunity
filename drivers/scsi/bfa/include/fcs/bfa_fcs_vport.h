



#ifndef __BFA_FCS_VPORT_H__
#define __BFA_FCS_VPORT_H__

#include <defs/bfa_defs_status.h>
#include <defs/bfa_defs_port.h>
#include <defs/bfa_defs_vport.h>
#include <fcs/bfa_fcs.h>
#include <fcb/bfa_fcb_vport.h>

struct bfa_fcs_vport_s {
	struct list_head		qe;		
	bfa_sm_t		sm;		
	bfa_fcs_lport_t		lport;		
	struct bfa_timer_s	timer;		
	struct bfad_vport_s	*vport_drv;	
	struct bfa_vport_stats_s vport_stats;	
	struct bfa_lps_s	*lps;		
	int			fdisc_retries;
};

#define bfa_fcs_vport_get_port(vport) \
			((struct bfa_fcs_port_s  *)(&vport->port))


bfa_status_t bfa_fcs_vport_create(struct bfa_fcs_vport_s *vport,
			struct bfa_fcs_s *fcs, u16 vf_id,
			struct bfa_port_cfg_s *port_cfg,
			struct bfad_vport_s *vport_drv);
bfa_status_t bfa_fcs_vport_delete(struct bfa_fcs_vport_s *vport);
bfa_status_t bfa_fcs_vport_start(struct bfa_fcs_vport_s *vport);
bfa_status_t bfa_fcs_vport_stop(struct bfa_fcs_vport_s *vport);
void bfa_fcs_vport_get_attr(struct bfa_fcs_vport_s *vport,
			struct bfa_vport_attr_s *vport_attr);
void bfa_fcs_vport_get_stats(struct bfa_fcs_vport_s *vport,
			struct bfa_vport_stats_s *vport_stats);
void bfa_fcs_vport_clr_stats(struct bfa_fcs_vport_s *vport);
struct bfa_fcs_vport_s *bfa_fcs_vport_lookup(struct bfa_fcs_s *fcs,
			u16 vf_id, wwn_t vpwwn);

#endif 
