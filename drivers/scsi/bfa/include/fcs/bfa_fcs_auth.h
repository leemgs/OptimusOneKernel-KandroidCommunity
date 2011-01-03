

#ifndef __BFA_FCS_AUTH_H__
#define __BFA_FCS_AUTH_H__

struct bfa_fcs_s;

#include <defs/bfa_defs_status.h>
#include <defs/bfa_defs_auth.h>
#include <defs/bfa_defs_vf.h>
#include <cs/bfa_q.h>
#include <cs/bfa_sm.h>
#include <defs/bfa_defs_pport.h>
#include <fcs/bfa_fcs_lport.h>
#include <protocol/fc_sp.h>

struct bfa_fcs_fabric_s;



struct bfa_fcs_auth_s {
	bfa_sm_t	sm;	
	bfa_boolean_t   policy;	
	enum bfa_auth_status status;	
	enum auth_rjt_codes  rjt_code;	
	enum auth_rjt_code_exps  rjt_code_exp;	
	enum bfa_auth_algo algo;	
	struct bfa_auth_stats_s stats;	
	enum auth_dh_gid   group;	
	enum bfa_auth_secretsource source;	
	char            secret[BFA_AUTH_SECRET_STRING_LEN];
				
	u8         secret_len;
				
	u8         nretries;
				
	struct bfa_fcs_fabric_s *fabric;
	u8         sentcode;	
	u8        *response;	
	struct bfa_timer_s delay_timer; 	
	struct bfa_fcxp_s *fcxp;		
	struct bfa_fcxp_wqe_s fcxp_wqe;
};


bfa_status_t    bfa_fcs_auth_get_attr(struct bfa_fcs_s *port,
				      struct bfa_auth_attr_s *attr);
bfa_status_t    bfa_fcs_auth_set_policy(struct bfa_fcs_s *port,
					bfa_boolean_t policy);
enum bfa_auth_status bfa_fcs_auth_get_status(struct bfa_fcs_s *port);
bfa_status_t    bfa_fcs_auth_set_algo(struct bfa_fcs_s *port,
				      enum bfa_auth_algo algo);
bfa_status_t    bfa_fcs_auth_get_stats(struct bfa_fcs_s *port,
				       struct bfa_auth_stats_s *stats);
bfa_status_t    bfa_fcs_auth_set_dh_group(struct bfa_fcs_s *port, int group);
bfa_status_t    bfa_fcs_auth_set_secretstring(struct bfa_fcs_s *port,
					      char *secret);
bfa_status_t    bfa_fcs_auth_set_secretstring_encrypt(struct bfa_fcs_s *port,
					      u32 secret[], u32 len);
bfa_status_t    bfa_fcs_auth_set_secretsource(struct bfa_fcs_s *port,
					      enum bfa_auth_secretsource src);
bfa_status_t    bfa_fcs_auth_reset_stats(struct bfa_fcs_s *port);
bfa_status_t    bfa_fcs_auth_reinit(struct bfa_fcs_s *port);

#endif 
