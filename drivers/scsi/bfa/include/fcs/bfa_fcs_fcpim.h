



#ifndef __BFA_FCS_FCPIM_H__
#define __BFA_FCS_FCPIM_H__

#include <defs/bfa_defs_status.h>
#include <defs/bfa_defs_itnim.h>
#include <fcs/bfa_fcs.h>
#include <fcs/bfa_fcs_rport.h>
#include <fcs/bfa_fcs_lport.h>
#include <bfa_fcpim.h>


struct bfad_itnim_s;

struct bfa_fcs_itnim_s {
	bfa_sm_t		sm;		
	struct bfa_fcs_rport_s 	*rport;		
	struct bfad_itnim_s   	*itnim_drv;	
	struct bfa_fcs_s      	*fcs;		
	struct bfa_timer_s 	timer;		
	struct bfa_itnim_s 	*bfa_itnim;	
	bfa_boolean_t	 	seq_rec;	
	bfa_boolean_t	 	rec_support;	
	bfa_boolean_t	 	conf_comp;	
	bfa_boolean_t	 	task_retry_id;	
	struct bfa_fcxp_wqe_s 	fcxp_wqe;	
	struct bfa_fcxp_s *fcxp;		
	struct bfa_itnim_stats_s 	stats;	
};


static inline struct bfad_port_s *
bfa_fcs_itnim_get_drvport(struct bfa_fcs_itnim_s *itnim)
{
	return itnim->rport->port->bfad_port;
}


static inline struct bfa_fcs_port_s *
bfa_fcs_itnim_get_port(struct bfa_fcs_itnim_s *itnim)
{
	return itnim->rport->port;
}


static inline wwn_t
bfa_fcs_itnim_get_nwwn(struct bfa_fcs_itnim_s *itnim)
{
	return itnim->rport->nwwn;
}


static inline wwn_t
bfa_fcs_itnim_get_pwwn(struct bfa_fcs_itnim_s *itnim)
{
	return itnim->rport->pwwn;
}


static inline u32
bfa_fcs_itnim_get_fcid(struct bfa_fcs_itnim_s *itnim)
{
	return itnim->rport->pid;
}


static inline   u32
bfa_fcs_itnim_get_maxfrsize(struct bfa_fcs_itnim_s *itnim)
{
	return itnim->rport->maxfrsize;
}


static inline   enum fc_cos
bfa_fcs_itnim_get_cos(struct bfa_fcs_itnim_s *itnim)
{
	return itnim->rport->fc_cos;
}


static inline struct bfad_itnim_s *
bfa_fcs_itnim_get_drvitn(struct bfa_fcs_itnim_s *itnim)
{
	return itnim->itnim_drv;
}


static inline struct bfa_itnim_s *
bfa_fcs_itnim_get_halitn(struct bfa_fcs_itnim_s *itnim)
{
	return itnim->bfa_itnim;
}


void bfa_fcs_itnim_get_attr(struct bfa_fcs_itnim_s *itnim,
			struct bfa_itnim_attr_s *attr);
void bfa_fcs_itnim_get_stats(struct bfa_fcs_itnim_s *itnim,
			struct bfa_itnim_stats_s *stats);
struct bfa_fcs_itnim_s *bfa_fcs_itnim_lookup(struct bfa_fcs_port_s *port,
			wwn_t rpwwn);
bfa_status_t bfa_fcs_itnim_attr_get(struct bfa_fcs_port_s *port, wwn_t rpwwn,
			struct bfa_itnim_attr_s *attr);
bfa_status_t bfa_fcs_itnim_stats_get(struct bfa_fcs_port_s *port, wwn_t rpwwn,
			struct bfa_itnim_stats_s *stats);
bfa_status_t bfa_fcs_itnim_stats_clear(struct bfa_fcs_port_s *port,
			wwn_t rpwwn);
#endif 
