

#ifndef __BFA_FCS_RPORT_H__
#define __BFA_FCS_RPORT_H__

#include <defs/bfa_defs_status.h>
#include <cs/bfa_q.h>
#include <fcs/bfa_fcs.h>
#include <defs/bfa_defs_rport.h>

#define BFA_FCS_RPORT_DEF_DEL_TIMEOUT 	90 	

struct bfad_rport_s;

struct bfa_fcs_itnim_s;
struct bfa_fcs_tin_s;
struct bfa_fcs_iprp_s;


struct bfa_fcs_rpf_s {
	bfa_sm_t               sm;	
	struct bfa_fcs_rport_s *rport;	
	struct bfa_timer_s 	timer;	
	struct bfa_fcxp_s 	*fcxp;	
	struct bfa_fcxp_wqe_s 	fcxp_wqe;	
	int             	rpsc_retries;	
	enum bfa_pport_speed 	rpsc_speed;	
	enum bfa_pport_speed	assigned_speed;	
};

struct bfa_fcs_rport_s {
	struct list_head         qe;	
	struct bfa_fcs_port_s *port;	
	struct bfa_fcs_s      *fcs;	
	struct bfad_rport_s   *rp_drv;	
	u32        pid;	
	u16        maxfrsize;	
	u16        reply_oxid;	
	enum fc_cos        fc_cos;	
	bfa_boolean_t   cisc;	
	wwn_t           pwwn;	
	wwn_t           nwwn;	
	struct bfa_rport_symname_s psym_name; 
	bfa_sm_t        sm;		
	struct bfa_timer_s timer;	
	struct bfa_fcs_itnim_s *itnim;	
	struct bfa_fcs_tin_s *tin;	
	struct bfa_fcs_iprp_s *iprp;	
	struct bfa_rport_s *bfa_rport;	
	struct bfa_fcxp_s *fcxp;	
	int             plogi_retries;	
	int             ns_retries;	
	struct bfa_fcxp_wqe_s 	fcxp_wqe; 
	struct bfa_rport_stats_s stats;	
	enum bfa_rport_function	scsi_function;  
	struct bfa_fcs_rpf_s rpf; 	
};

static inline struct bfa_rport_s *
bfa_fcs_rport_get_halrport(struct bfa_fcs_rport_s *rport)
{
	return rport->bfa_rport;
}


bfa_status_t bfa_fcs_rport_add(struct bfa_fcs_port_s *port, wwn_t *pwwn,
			struct bfa_fcs_rport_s *rport,
			struct bfad_rport_s *rport_drv);
bfa_status_t bfa_fcs_rport_remove(struct bfa_fcs_rport_s *rport);
void bfa_fcs_rport_get_attr(struct bfa_fcs_rport_s *rport,
			struct bfa_rport_attr_s *attr);
void bfa_fcs_rport_get_stats(struct bfa_fcs_rport_s *rport,
			struct bfa_rport_stats_s *stats);
void bfa_fcs_rport_clear_stats(struct bfa_fcs_rport_s *rport);
struct bfa_fcs_rport_s *bfa_fcs_rport_lookup(struct bfa_fcs_port_s *port,
			wwn_t rpwwn);
struct bfa_fcs_rport_s *bfa_fcs_rport_lookup_by_nwwn(
			struct bfa_fcs_port_s *port, wwn_t rnwwn);
void bfa_fcs_rport_set_del_timeout(u8 rport_tmo);
void bfa_fcs_rport_set_speed(struct bfa_fcs_rport_s *rport,
			enum bfa_pport_speed speed);
#endif 
