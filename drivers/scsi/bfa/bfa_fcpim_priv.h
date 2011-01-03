

#ifndef __BFA_FCPIM_PRIV_H__
#define __BFA_FCPIM_PRIV_H__

#include <bfa_fcpim.h>
#include <defs/bfa_defs_fcpim.h>
#include <cs/bfa_wc.h>
#include "bfa_sgpg_priv.h"

#define BFA_ITNIM_MIN   32
#define BFA_ITNIM_MAX   1024

#define BFA_IOIM_MIN    8
#define BFA_IOIM_MAX    2000

#define BFA_TSKIM_MIN   4
#define BFA_TSKIM_MAX   512
#define BFA_FCPIM_PATHTOV_DEF	(30 * 1000)	
#define BFA_FCPIM_PATHTOV_MAX	(90 * 1000)	

#define bfa_fcpim_stats(__fcpim, __stats)   \
    (__fcpim)->stats.__stats ++

struct bfa_fcpim_mod_s {
	struct bfa_s 	*bfa;
	struct bfa_itnim_s 	*itnim_arr;
	struct bfa_ioim_s 	*ioim_arr;
	struct bfa_ioim_sp_s *ioim_sp_arr;
	struct bfa_tskim_s 	*tskim_arr;
	struct bfa_dma_s	snsbase;
	int			num_itnims;
	int			num_ioim_reqs;
	int			num_tskim_reqs;
	u32		path_tov;
	u16		q_depth;
	u16		rsvd;
	struct list_head 	itnim_q;        
	struct list_head 	ioim_free_q;    
	struct list_head 	ioim_resfree_q; 
	struct list_head 	ioim_comp_q;    
	struct list_head 	tskim_free_q;
	u32	ios_active;	
	u32	delay_comp;
	struct bfa_fcpim_stats_s stats;
};

struct bfa_ioim_s;
struct bfa_tskim_s;


struct bfa_ioim_s {
	struct list_head qe;		
	bfa_sm_t		sm; 	
	struct bfa_s 	        *bfa;	
	struct bfa_fcpim_mod_s	*fcpim;	
	struct bfa_itnim_s 	*itnim;	
	struct bfad_ioim_s 	*dio;	
	u16	iotag;		
	u16	abort_tag;	
	u16	nsges;		
	u16	nsgpgs;		
	struct bfa_sgpg_s *sgpg;	
	struct list_head sgpg_q;		
	struct bfa_cb_qe_s hcb_qe;	
	bfa_cb_cbfn_t io_cbfn;		
	struct bfa_ioim_sp_s *iosp;	
};

struct bfa_ioim_sp_s {
	struct bfi_msg_s 	comp_rspmsg;	
	u8			*snsinfo;	
	struct bfa_sgpg_wqe_s sgpg_wqe;	
	struct bfa_reqq_wait_s reqq_wait;	
	bfa_boolean_t		abort_explicit;	
	struct bfa_tskim_s	*tskim;		
};


struct bfa_tskim_s {
	struct list_head          qe;
	bfa_sm_t		sm;
	struct bfa_s            *bfa;        
	struct bfa_fcpim_mod_s  *fcpim;      
	struct bfa_itnim_s      *itnim;      
	struct bfad_tskim_s         *dtsk;   
	bfa_boolean_t        notify;         
	lun_t                lun;            
	enum fcp_tm_cmnd        tm_cmnd;     
	u16             tsk_tag;        
	u8              tsecs;          
	struct bfa_reqq_wait_s  reqq_wait;   
	struct list_head              io_q;    
	struct bfa_wc_s             wc;      
	struct bfa_cb_qe_s	hcb_qe;      
	enum bfi_tskim_status   tsk_status;  
};


struct bfa_itnim_s {
	struct list_head    qe;		
	bfa_sm_t	  sm;		
	struct bfa_s      *bfa;		
	struct bfa_rport_s *rport;	
	void           *ditn;		
	struct bfi_mhdr_s      mhdr;	
	u8         msg_no;		
	u8         reqq;		
	struct bfa_cb_qe_s    hcb_qe;	
	struct list_head pending_q;	
	struct list_head io_q;		
	struct list_head io_cleanup_q;	
	struct list_head tsk_q;		
	struct list_head  delay_comp_q;
	bfa_boolean_t   seq_rec;	
	bfa_boolean_t   is_online;	
	bfa_boolean_t   iotov_active;	
	struct bfa_wc_s        wc;	
	struct bfa_timer_s timer;	
	struct bfa_reqq_wait_s reqq_wait; 
	struct bfa_fcpim_mod_s *fcpim;	
	struct bfa_itnim_hal_stats_s	stats;
};

#define bfa_itnim_is_online(_itnim) (_itnim)->is_online
#define BFA_FCPIM_MOD(_hal) (&(_hal)->modules.fcpim_mod)
#define BFA_IOIM_FROM_TAG(_fcpim, _iotag)	\
	(&fcpim->ioim_arr[_iotag])
#define BFA_TSKIM_FROM_TAG(_fcpim, _tmtag)                  \
    (&fcpim->tskim_arr[_tmtag & (fcpim->num_tskim_reqs - 1)])


void            bfa_ioim_attach(struct bfa_fcpim_mod_s *fcpim,
				    struct bfa_meminfo_s *minfo);
void            bfa_ioim_detach(struct bfa_fcpim_mod_s *fcpim);
void            bfa_ioim_isr(struct bfa_s *bfa, struct bfi_msg_s *msg);
void            bfa_ioim_good_comp_isr(struct bfa_s *bfa,
					struct bfi_msg_s *msg);
void            bfa_ioim_cleanup(struct bfa_ioim_s *ioim);
void            bfa_ioim_cleanup_tm(struct bfa_ioim_s *ioim,
					struct bfa_tskim_s *tskim);
void            bfa_ioim_iocdisable(struct bfa_ioim_s *ioim);
void            bfa_ioim_tov(struct bfa_ioim_s *ioim);

void            bfa_tskim_attach(struct bfa_fcpim_mod_s *fcpim,
				     struct bfa_meminfo_s *minfo);
void            bfa_tskim_detach(struct bfa_fcpim_mod_s *fcpim);
void            bfa_tskim_isr(struct bfa_s *bfa, struct bfi_msg_s *msg);
void            bfa_tskim_iodone(struct bfa_tskim_s *tskim);
void            bfa_tskim_iocdisable(struct bfa_tskim_s *tskim);
void            bfa_tskim_cleanup(struct bfa_tskim_s *tskim);

void            bfa_itnim_meminfo(struct bfa_iocfc_cfg_s *cfg, u32 *km_len,
				      u32 *dm_len);
void            bfa_itnim_attach(struct bfa_fcpim_mod_s *fcpim,
				     struct bfa_meminfo_s *minfo);
void            bfa_itnim_detach(struct bfa_fcpim_mod_s *fcpim);
void            bfa_itnim_iocdisable(struct bfa_itnim_s *itnim);
void            bfa_itnim_isr(struct bfa_s *bfa, struct bfi_msg_s *msg);
void            bfa_itnim_iodone(struct bfa_itnim_s *itnim);
void            bfa_itnim_tskdone(struct bfa_itnim_s *itnim);
bfa_boolean_t   bfa_itnim_hold_io(struct bfa_itnim_s *itnim);

#endif 

