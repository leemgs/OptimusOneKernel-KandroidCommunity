



#ifndef __BFA_SGPG_PRIV_H__
#define __BFA_SGPG_PRIV_H__

#include <cs/bfa_q.h>

#define BFA_SGPG_MIN	(16)


#define BFA_SGPG_ROUNDUP(_l) (((_l) + (sizeof(struct bfi_sgpg_s) - 1)) \
			& ~(sizeof(struct bfi_sgpg_s) - 1))

struct bfa_sgpg_wqe_s {
	struct list_head qe;	
	int	nsgpg;		
	int	nsgpg_total;	
	void	(*cbfn) (void *cbarg);
				
	void	*cbarg;		
	struct list_head sgpg_q;	
};

struct bfa_sgpg_s {
	struct list_head 	qe;	
	struct bfi_sgpg_s *sgpg; 
	union bfi_addr_u sgpg_pa;
};


#define BFA_SGPG_NPAGE(_nsges)  (((_nsges) / BFI_SGPG_DATA_SGES) + 1)

struct bfa_sgpg_mod_s {
	struct bfa_s *bfa;
	int		num_sgpgs;	
	int		free_sgpgs;	
	struct bfa_sgpg_s *hsgpg_arr;	
	struct bfi_sgpg_s *sgpg_arr;	
	u64	sgpg_arr_pa;	
	struct list_head sgpg_q;	
	struct list_head sgpg_wait_q; 
};
#define BFA_SGPG_MOD(__bfa)	(&(__bfa)->modules.sgpg_mod)

bfa_status_t	bfa_sgpg_malloc(struct bfa_s *bfa, struct list_head *sgpg_q,
								int nsgpgs);
void		bfa_sgpg_mfree(struct bfa_s *bfa, struct list_head *sgpg_q,
								int nsgpgs);
void		bfa_sgpg_winit(struct bfa_sgpg_wqe_s *wqe,
				   void (*cbfn) (void *cbarg), void *cbarg);
void		bfa_sgpg_wait(struct bfa_s *bfa, struct bfa_sgpg_wqe_s *wqe,
								int nsgpgs);
void		bfa_sgpg_wcancel(struct bfa_s *bfa, struct bfa_sgpg_wqe_s *wqe);

#endif 
