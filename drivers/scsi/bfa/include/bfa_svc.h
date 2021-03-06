
#ifndef __BFA_SVC_H__
#define __BFA_SVC_H__


struct bfa_fcxp_s;

#include <defs/bfa_defs_status.h>
#include <defs/bfa_defs_pport.h>
#include <defs/bfa_defs_rport.h>
#include <defs/bfa_defs_qos.h>
#include <cs/bfa_sm.h>
#include <bfa.h>


struct bfa_rport_info_s {
	u16        max_frmsz;	
	u32        pid : 24,	
			lp_tag : 8;
	u32        local_pid : 24,	
			cisc : 8;	
	u8         fc_class;	
	u8         vf_en;		
	u16        vf_id;		
	enum bfa_pport_speed speed;	
};


struct bfa_rport_s {
	struct list_head        qe;	  
	bfa_sm_t	      sm; 	  
	struct bfa_s          *bfa;	  
	void                  *rport_drv; 
	u16              fw_handle; 
	u16              rport_tag; 
	struct bfa_rport_info_s rport_info; 
	struct bfa_reqq_wait_s reqq_wait; 
	struct bfa_cb_qe_s    hcb_qe;	 
	struct bfa_rport_hal_stats_s stats; 
	struct bfa_rport_qos_attr_s  qos_attr;
	union a {
		bfa_status_t    status;	 
		void            *fw_msg; 
	} event_arg;
};
#define BFA_RPORT_FC_COS(_rport)	((_rport)->rport_info.fc_class)


typedef void (*bfa_cb_fcxp_send_t) (void *bfad_fcxp, struct bfa_fcxp_s *fcxp,
			void *cbarg, enum bfa_status req_status,
			u32 rsp_len, u32 resid_len,
			struct fchs_s *rsp_fchs);


typedef void (*bfa_fcxp_alloc_cbfn_t) (void *cbarg, struct bfa_fcxp_s *fcxp);

struct bfa_fcxp_wqe_s {
	struct list_head         qe;
	bfa_fcxp_alloc_cbfn_t  alloc_cbfn;
	void           *alloc_cbarg;
};

typedef u64 (*bfa_fcxp_get_sgaddr_t) (void *bfad_fcxp, int sgeid);
typedef u32 (*bfa_fcxp_get_sglen_t) (void *bfad_fcxp, int sgeid);

#define BFA_UF_BUFSZ	(2 * 1024 + 256)


struct bfa_uf_buf_s {
	u8         d[BFA_UF_BUFSZ];
};


struct bfa_uf_s {
	struct list_head	qe;		
	struct bfa_s	*bfa;		
	u16        uf_tag;		
	u16        vf_id;
	u16        src_rport_handle;
	u16        rsvd;
	u8        	*data_ptr;
	u16        data_len;	
	u16        pb_len;		
	void           	*buf_kva;	
	u64        buf_pa;		
	struct bfa_cb_qe_s    hcb_qe;	
	struct bfa_sge_s   	sges[BFI_SGE_INLINE_MAX];
};

typedef void (*bfa_cb_pport_t) (void *cbarg, enum bfa_status status);


struct bfa_lps_s {
	struct list_head	qe;		
	struct bfa_s	*bfa;		
	bfa_sm_t	sm;		
	u8		lp_tag;		
	u8		reqq;		
	u8		alpa;		
	u32	lp_pid;		
	bfa_boolean_t	fdisc;		
	bfa_boolean_t	auth_en;	
	bfa_boolean_t	auth_req;	
	bfa_boolean_t	npiv_en;	
	bfa_boolean_t	fport;		
	bfa_boolean_t	brcd_switch;
	bfa_status_t	status;		
	u16	pdusz;		
	u16	pr_bbcred;	
	u8		lsrjt_rsn;	
	u8		lsrjt_expl;	
	wwn_t		pwwn;		
	wwn_t		nwwn;		
	wwn_t		pr_pwwn;	
	wwn_t		pr_nwwn;	
	mac_t		lp_mac;		
	mac_t		fcf_mac;	
	struct bfa_reqq_wait_s	wqe;	
	void		*uarg;		
	struct bfa_cb_qe_s hcb_qe;	
	struct bfi_lps_login_rsp_s *loginrsp;
	bfa_eproto_status_t	ext_status;
};


bfa_status_t bfa_pport_enable(struct bfa_s *bfa);
bfa_status_t bfa_pport_disable(struct bfa_s *bfa);
bfa_status_t bfa_pport_cfg_speed(struct bfa_s *bfa,
			enum bfa_pport_speed speed);
enum bfa_pport_speed bfa_pport_get_speed(struct bfa_s *bfa);
bfa_status_t bfa_pport_cfg_topology(struct bfa_s *bfa,
			enum bfa_pport_topology topo);
enum bfa_pport_topology bfa_pport_get_topology(struct bfa_s *bfa);
bfa_status_t bfa_pport_cfg_hardalpa(struct bfa_s *bfa, u8 alpa);
bfa_boolean_t bfa_pport_get_hardalpa(struct bfa_s *bfa, u8 *alpa);
u8 bfa_pport_get_myalpa(struct bfa_s *bfa);
bfa_status_t bfa_pport_clr_hardalpa(struct bfa_s *bfa);
bfa_status_t bfa_pport_cfg_maxfrsize(struct bfa_s *bfa, u16 maxsize);
u16 bfa_pport_get_maxfrsize(struct bfa_s *bfa);
u32 bfa_pport_mypid(struct bfa_s *bfa);
u8 bfa_pport_get_rx_bbcredit(struct bfa_s *bfa);
bfa_status_t bfa_pport_trunk_enable(struct bfa_s *bfa, u8 bitmap);
bfa_status_t bfa_pport_trunk_disable(struct bfa_s *bfa);
bfa_boolean_t bfa_pport_trunk_query(struct bfa_s *bfa, u32 *bitmap);
void bfa_pport_get_attr(struct bfa_s *bfa, struct bfa_pport_attr_s *attr);
wwn_t bfa_pport_get_wwn(struct bfa_s *bfa, bfa_boolean_t node);
bfa_status_t bfa_pport_get_stats(struct bfa_s *bfa,
			union bfa_pport_stats_u *stats,
			bfa_cb_pport_t cbfn, void *cbarg);
bfa_status_t bfa_pport_clear_stats(struct bfa_s *bfa, bfa_cb_pport_t cbfn,
			void *cbarg);
void bfa_pport_event_register(struct bfa_s *bfa,
			void (*event_cbfn) (void *cbarg,
			bfa_pport_event_t event), void *event_cbarg);
bfa_boolean_t bfa_pport_is_disabled(struct bfa_s *bfa);
void bfa_pport_cfg_qos(struct bfa_s *bfa, bfa_boolean_t on_off);
void bfa_pport_cfg_ratelim(struct bfa_s *bfa, bfa_boolean_t on_off);
bfa_status_t bfa_pport_cfg_ratelim_speed(struct bfa_s *bfa,
			enum bfa_pport_speed speed);
enum bfa_pport_speed bfa_pport_get_ratelim_speed(struct bfa_s *bfa);

void bfa_pport_set_tx_bbcredit(struct bfa_s *bfa, u16 tx_bbcredit);
void bfa_pport_busy(struct bfa_s *bfa, bfa_boolean_t status);
void bfa_pport_beacon(struct bfa_s *bfa, bfa_boolean_t beacon,
			bfa_boolean_t link_e2e_beacon);
void bfa_cb_pport_event(void *cbarg, bfa_pport_event_t event);
void bfa_pport_qos_get_attr(struct bfa_s *bfa, struct bfa_qos_attr_s *qos_attr);
void bfa_pport_qos_get_vc_attr(struct bfa_s *bfa,
			struct bfa_qos_vc_attr_s *qos_vc_attr);
bfa_status_t bfa_pport_get_qos_stats(struct bfa_s *bfa,
			union bfa_pport_stats_u *stats,
			bfa_cb_pport_t cbfn, void *cbarg);
bfa_status_t bfa_pport_clear_qos_stats(struct bfa_s *bfa, bfa_cb_pport_t cbfn,
			void *cbarg);
bfa_boolean_t     bfa_pport_is_ratelim(struct bfa_s *bfa);
bfa_boolean_t	bfa_pport_is_linkup(struct bfa_s *bfa);


struct bfa_rport_s *bfa_rport_create(struct bfa_s *bfa, void *rport_drv);
void bfa_rport_delete(struct bfa_rport_s *rport);
void bfa_rport_online(struct bfa_rport_s *rport,
			struct bfa_rport_info_s *rport_info);
void bfa_rport_offline(struct bfa_rport_s *rport);
void bfa_rport_speed(struct bfa_rport_s *rport, enum bfa_pport_speed speed);
void bfa_rport_get_stats(struct bfa_rport_s *rport,
			struct bfa_rport_hal_stats_s *stats);
void bfa_rport_clear_stats(struct bfa_rport_s *rport);
void bfa_cb_rport_online(void *rport);
void bfa_cb_rport_offline(void *rport);
void bfa_cb_rport_qos_scn_flowid(void *rport,
			struct bfa_rport_qos_attr_s old_qos_attr,
			struct bfa_rport_qos_attr_s new_qos_attr);
void bfa_cb_rport_qos_scn_prio(void *rport,
			struct bfa_rport_qos_attr_s old_qos_attr,
			struct bfa_rport_qos_attr_s new_qos_attr);
void bfa_rport_get_qos_attr(struct bfa_rport_s *rport,
			struct bfa_rport_qos_attr_s *qos_attr);


struct bfa_fcxp_s *bfa_fcxp_alloc(void *bfad_fcxp, struct bfa_s *bfa,
			int nreq_sgles, int nrsp_sgles,
			bfa_fcxp_get_sgaddr_t get_req_sga,
			bfa_fcxp_get_sglen_t get_req_sglen,
			bfa_fcxp_get_sgaddr_t get_rsp_sga,
			bfa_fcxp_get_sglen_t get_rsp_sglen);
void bfa_fcxp_alloc_wait(struct bfa_s *bfa, struct bfa_fcxp_wqe_s *wqe,
			bfa_fcxp_alloc_cbfn_t alloc_cbfn, void *cbarg);
void bfa_fcxp_walloc_cancel(struct bfa_s *bfa,
			struct bfa_fcxp_wqe_s *wqe);
void bfa_fcxp_discard(struct bfa_fcxp_s *fcxp);

void *bfa_fcxp_get_reqbuf(struct bfa_fcxp_s *fcxp);
void *bfa_fcxp_get_rspbuf(struct bfa_fcxp_s *fcxp);

void bfa_fcxp_free(struct bfa_fcxp_s *fcxp);

void bfa_fcxp_send(struct bfa_fcxp_s *fcxp,
			struct bfa_rport_s *rport, u16 vf_id, u8 lp_tag,
			bfa_boolean_t cts, enum fc_cos cos,
			u32 reqlen, struct fchs_s *fchs,
			bfa_cb_fcxp_send_t cbfn,
			void *cbarg,
			u32 rsp_maxlen, u8 rsp_timeout);
bfa_status_t bfa_fcxp_abort(struct bfa_fcxp_s *fcxp);
u32        bfa_fcxp_get_reqbufsz(struct bfa_fcxp_s *fcxp);
u32	bfa_fcxp_get_maxrsp(struct bfa_s *bfa);

static inline void *
bfa_uf_get_frmbuf(struct bfa_uf_s *uf)
{
	return uf->data_ptr;
}

static inline   u16
bfa_uf_get_frmlen(struct bfa_uf_s *uf)
{
	return uf->data_len;
}


typedef void (*bfa_cb_uf_recv_t) (void *cbarg, struct bfa_uf_s *uf);


void bfa_uf_recv_register(struct bfa_s *bfa, bfa_cb_uf_recv_t ufrecv,
			void *cbarg);
void bfa_uf_free(struct bfa_uf_s *uf);



struct bfa_lps_s *bfa_lps_alloc(struct bfa_s *bfa);
void bfa_lps_delete(struct bfa_lps_s *lps);
void bfa_lps_discard(struct bfa_lps_s *lps);
void bfa_lps_flogi(struct bfa_lps_s *lps, void *uarg, u8 alpa, u16 pdusz,
		   wwn_t pwwn, wwn_t nwwn, bfa_boolean_t auth_en);
void bfa_lps_fdisc(struct bfa_lps_s *lps, void *uarg, u16 pdusz, wwn_t pwwn,
		   wwn_t nwwn);
void bfa_lps_flogo(struct bfa_lps_s *lps);
void bfa_lps_fdisclogo(struct bfa_lps_s *lps);
u8 bfa_lps_get_tag(struct bfa_lps_s *lps);
bfa_boolean_t bfa_lps_is_npiv_en(struct bfa_lps_s *lps);
bfa_boolean_t bfa_lps_is_fport(struct bfa_lps_s *lps);
bfa_boolean_t bfa_lps_is_brcd_fabric(struct bfa_lps_s *lps);
bfa_boolean_t bfa_lps_is_authreq(struct bfa_lps_s *lps);
bfa_eproto_status_t bfa_lps_get_extstatus(struct bfa_lps_s *lps);
u32 bfa_lps_get_pid(struct bfa_lps_s *lps);
u8 bfa_lps_get_tag_from_pid(struct bfa_s *bfa, u32 pid);
u16 bfa_lps_get_peer_bbcredit(struct bfa_lps_s *lps);
wwn_t bfa_lps_get_peer_pwwn(struct bfa_lps_s *lps);
wwn_t bfa_lps_get_peer_nwwn(struct bfa_lps_s *lps);
u8 bfa_lps_get_lsrjt_rsn(struct bfa_lps_s *lps);
u8 bfa_lps_get_lsrjt_expl(struct bfa_lps_s *lps);
void bfa_cb_lps_flogi_comp(void *bfad, void *uarg, bfa_status_t status);
void bfa_cb_lps_flogo_comp(void *bfad, void *uarg);
void bfa_cb_lps_fdisc_comp(void *bfad, void *uarg, bfa_status_t status);
void bfa_cb_lps_fdisclogo_comp(void *bfad, void *uarg);

#endif 

