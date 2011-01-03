

#ifndef __BFA_DEFS_TIN_H__
#define __BFA_DEFS_TIN_H__

#include <protocol/types.h>
#include <protocol/fc.h>


enum bfa_tin_state_e {
	BFA_TIN_SM_OFFLINE = 0,		
	BFA_TIN_SM_WOS_LOGIN = 1,	
	BFA_TIN_SM_WFW_ONLINE = 2,	
	BFA_TIN_SM_ONLINE = 3,		
	BFA_TIN_SM_WIO_RELOGIN = 4,	
	BFA_TIN_SM_WIO_LOGOUT = 5,	
	BFA_TIN_SM_WOS_LOGOUT = 6,	
	BFA_TIN_SM_WIO_CLEAN = 7,	
};

struct bfa_prli_req_s {
	struct fchs_s fchs;
	struct fc_prli_s prli_payload;
};

struct bfa_prlo_req_s {
	struct fchs_s fchs;
	struct fc_prlo_s prlo_payload;
};

void bfa_tin_send_login_rsp(void *bfa_tin, u32 login_rsp,
				struct fc_ls_rjt_s rjt_payload);
void bfa_tin_send_logout_rsp(void *bfa_tin, u32 logout_rsp,
				struct fc_ls_rjt_s rjt_payload);

struct bfa_tin_stats_s {
	u32 onlines;	
	u32 offlines;	
	u32 prli_req_parse_err;	
	u32 prli_rsp_rjt;	
	u32 prli_rsp_acc;	
	u32 cleanup_comps;	
};


struct bfa_tin_attr_s {
	enum bfa_tin_state_e state;
	u8	seq_retry;    
	u8	rsvd[3];
};


enum bfa_tin_aen_event {
	BFA_TIN_AEN_ONLINE 	= 1,	
	BFA_TIN_AEN_OFFLINE 	= 2,	
	BFA_TIN_AEN_DISCONNECT	= 3,	
};


struct bfa_tin_aen_data_s {
	u16 vf_id;	
	u16 rsvd[3];
	wwn_t lpwwn;	
	wwn_t rpwwn;	
};



void *bfad_tin_rcvd_login_req(void *bfad_tm_port, void *bfa_tin,
				wwn_t rp_wwn, u32 rp_fcid,
				struct bfa_prli_req_s prli_req);

void bfad_tin_rcvd_logout_req(void *bfad_tin, wwn_t rp_wwn, u32 rp_fcid,
				struct bfa_prlo_req_s prlo_req);

void bfad_tin_online(void *bfad_tin);

void bfad_tin_offline(void *bfad_tin);

void bfad_tin_res_free(void *bfad_tin);

#endif 
