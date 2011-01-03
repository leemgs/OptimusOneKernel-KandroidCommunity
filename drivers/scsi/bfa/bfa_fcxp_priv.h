

#ifndef __BFA_FCXP_PRIV_H__
#define __BFA_FCXP_PRIV_H__

#include <cs/bfa_sm.h>
#include <protocol/fc.h>
#include <bfa_svc.h>
#include <bfi/bfi_fcxp.h>

#define BFA_FCXP_MIN     	(1)
#define BFA_FCXP_MAX_IBUF_SZ	(2 * 1024 + 256)
#define BFA_FCXP_MAX_LBUF_SZ	(4 * 1024 + 256)

struct bfa_fcxp_mod_s {
	struct bfa_s      *bfa;		
	struct bfa_fcxp_s *fcxp_list;	
	u16        num_fcxps;	
	struct list_head fcxp_free_q;	
	struct list_head fcxp_active_q;	
	void	*req_pld_list_kva;	
	u64 req_pld_list_pa;	
	void *rsp_pld_list_kva;		
	u64 rsp_pld_list_pa;	
	struct list_head  wait_q;		
	u32	req_pld_sz;
	u32	rsp_pld_sz;
};

#define BFA_FCXP_MOD(__bfa)		(&(__bfa)->modules.fcxp_mod)
#define BFA_FCXP_FROM_TAG(__mod, __tag)	(&(__mod)->fcxp_list[__tag])

typedef void    (*fcxp_send_cb_t) (struct bfa_s *ioc, struct bfa_fcxp_s *fcxp,
				   void *cb_arg, bfa_status_t req_status,
				   u32 rsp_len, u32 resid_len,
				   struct fchs_s *rsp_fchs);


struct bfa_fcxp_req_info_s {
	struct bfa_rport_s *bfa_rport;	
	struct fchs_s   fchs;	
	u8 cts;		
	u8 class;		
	u16 max_frmsz;	
	u16 vf_id;		
	u8	lp_tag;		
	u32 req_tot_len;	
};

struct bfa_fcxp_rsp_info_s {
	struct fchs_s rsp_fchs;		
	u8         rsp_timeout;	
	u8         rsvd2[3];
	u32        rsp_maxlen;	
};

struct bfa_fcxp_s {
	struct list_head 	qe;		
	bfa_sm_t        sm;             
	void           	*caller;	
	struct bfa_fcxp_mod_s *fcxp_mod;
					
	u16        fcxp_tag;	
	struct bfa_fcxp_req_info_s req_info;
					
	struct bfa_fcxp_rsp_info_s rsp_info;
					
	u8 	use_ireqbuf;	
	u8         use_irspbuf;	
	u32        nreq_sgles;	
	u32        nrsp_sgles;	
	struct list_head req_sgpg_q;	
	struct list_head req_sgpg_wqe;	
	struct list_head rsp_sgpg_q;	
	struct list_head rsp_sgpg_wqe;	

	bfa_fcxp_get_sgaddr_t req_sga_cbfn;
					
	bfa_fcxp_get_sglen_t req_sglen_cbfn;
					
	bfa_fcxp_get_sgaddr_t rsp_sga_cbfn;
					
	bfa_fcxp_get_sglen_t rsp_sglen_cbfn;
					
	bfa_cb_fcxp_send_t send_cbfn;   
	void		*send_cbarg;	
	struct bfa_sge_s   req_sge[BFA_FCXP_MAX_SGES];
					
	struct bfa_sge_s   rsp_sge[BFA_FCXP_MAX_SGES];
					
	u8         rsp_status;	
	u32        rsp_len;	
	u32        residue_len;	
	struct fchs_s          rsp_fchs;	
	struct bfa_cb_qe_s    hcb_qe;	
	struct bfa_reqq_wait_s	reqq_wqe;
	bfa_boolean_t	reqq_waiting;
};

#define BFA_FCXP_REQ_PLD(_fcxp) 	(bfa_fcxp_get_reqbuf(_fcxp))

#define BFA_FCXP_RSP_FCHS(_fcxp) 	(&((_fcxp)->rsp_info.fchs))
#define BFA_FCXP_RSP_PLD(_fcxp) 	(bfa_fcxp_get_rspbuf(_fcxp))

#define BFA_FCXP_REQ_PLD_PA(_fcxp)					\
	((_fcxp)->fcxp_mod->req_pld_list_pa +				\
		((_fcxp)->fcxp_mod->req_pld_sz  * (_fcxp)->fcxp_tag))

#define BFA_FCXP_RSP_PLD_PA(_fcxp) 					\
	((_fcxp)->fcxp_mod->rsp_pld_list_pa +				\
		((_fcxp)->fcxp_mod->rsp_pld_sz * (_fcxp)->fcxp_tag))

void	bfa_fcxp_isr(struct bfa_s *bfa, struct bfi_msg_s *msg);
#endif 
