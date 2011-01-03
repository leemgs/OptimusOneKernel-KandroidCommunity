
#ifndef __BFA_UF_PRIV_H__
#define __BFA_UF_PRIV_H__

#include <cs/bfa_sm.h>
#include <bfa_svc.h>
#include <bfi/bfi_uf.h>

#define BFA_UF_MIN	(4)

struct bfa_uf_mod_s {
	struct bfa_s *bfa;		
	struct bfa_uf_s *uf_list;	
	u16	num_ufs;	
	struct list_head 	uf_free_q;	
	struct list_head 	uf_posted_q;	
	struct bfa_uf_buf_s *uf_pbs_kva;	
	u64	uf_pbs_pa;	
	struct bfi_uf_buf_post_s *uf_buf_posts;
					
	bfa_cb_uf_recv_t ufrecv;	
	void		*cbarg;		
};

#define BFA_UF_MOD(__bfa)	(&(__bfa)->modules.uf_mod)

#define ufm_pbs_pa(_ufmod, _uftag)	\
	((_ufmod)->uf_pbs_pa + sizeof(struct bfa_uf_buf_s) * (_uftag))

void	bfa_uf_isr(struct bfa_s *bfa, struct bfi_msg_s *msg);

#endif 
