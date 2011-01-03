
#ifndef __BFA_H__
#define __BFA_H__

#include <bfa_os_inc.h>
#include <cs/bfa_debug.h>
#include <cs/bfa_q.h>
#include <cs/bfa_trc.h>
#include <cs/bfa_log.h>
#include <cs/bfa_plog.h>
#include <defs/bfa_defs_status.h>
#include <defs/bfa_defs_ioc.h>
#include <defs/bfa_defs_iocfc.h>
#include <aen/bfa_aen.h>
#include <bfi/bfi.h>

struct bfa_s;
#include <bfa_intr_priv.h>

struct bfa_pcidev_s;


struct bfa_pciid_s {
	u16        device_id;
	u16        vendor_id;
};

extern char     bfa_version[];


enum bfa_pm_cmd {
	BFA_PM_CTL_D0 = 0,
	BFA_PM_CTL_D1 = 1,
	BFA_PM_CTL_D2 = 2,
	BFA_PM_CTL_D3 = 3,
};


enum bfa_mem_type {
	BFA_MEM_TYPE_KVA = 1,	
	BFA_MEM_TYPE_DMA = 2,	
	BFA_MEM_TYPE_MAX = BFA_MEM_TYPE_DMA,
};

struct bfa_mem_elem_s {
	enum bfa_mem_type mem_type;	
	u32        mem_len;	
	u8       	*kva;		
	u64        dma;		
	u8       	*kva_curp;	
	u64        dma_curp;	
};

struct bfa_meminfo_s {
	struct bfa_mem_elem_s meminfo[BFA_MEM_TYPE_MAX];
};
#define bfa_meminfo_kva(_m)	\
	(_m)->meminfo[BFA_MEM_TYPE_KVA - 1].kva_curp
#define bfa_meminfo_dma_virt(_m)	\
	(_m)->meminfo[BFA_MEM_TYPE_DMA - 1].kva_curp
#define bfa_meminfo_dma_phys(_m)	\
	(_m)->meminfo[BFA_MEM_TYPE_DMA - 1].dma_curp


struct bfa_sge_s {
	u32        sg_len;
	void           *sg_addr;
};

#define bfa_sge_to_be(__sge) do {                                          \
	((u32 *)(__sge))[0] = bfa_os_htonl(((u32 *)(__sge))[0]);      \
	((u32 *)(__sge))[1] = bfa_os_htonl(((u32 *)(__sge))[1]);      \
	((u32 *)(__sge))[2] = bfa_os_htonl(((u32 *)(__sge))[2]);      \
} while (0)



#define bfa_stats(_mod, _stats)	(_mod)->stats._stats ++

#define bfa_ioc_get_stats(__bfa, __ioc_stats)	\
	bfa_ioc_fetch_stats(&(__bfa)->ioc, __ioc_stats)
#define bfa_ioc_clear_stats(__bfa)	\
	bfa_ioc_clr_stats(&(__bfa)->ioc)


void bfa_get_pciids(struct bfa_pciid_s **pciids, int *npciids);
void bfa_cfg_get_default(struct bfa_iocfc_cfg_s *cfg);
void bfa_cfg_get_min(struct bfa_iocfc_cfg_s *cfg);
void bfa_cfg_get_meminfo(struct bfa_iocfc_cfg_s *cfg,
			struct bfa_meminfo_s *meminfo);
void bfa_attach(struct bfa_s *bfa, void *bfad, struct bfa_iocfc_cfg_s *cfg,
			struct bfa_meminfo_s *meminfo,
			struct bfa_pcidev_s *pcidev);
void bfa_init_trc(struct bfa_s *bfa, struct bfa_trc_mod_s *trcmod);
void bfa_init_log(struct bfa_s *bfa, struct bfa_log_mod_s *logmod);
void bfa_init_aen(struct bfa_s *bfa, struct bfa_aen_s *aen);
void bfa_init_plog(struct bfa_s *bfa, struct bfa_plog_s *plog);
void bfa_detach(struct bfa_s *bfa);
void bfa_init(struct bfa_s *bfa);
void bfa_start(struct bfa_s *bfa);
void bfa_stop(struct bfa_s *bfa);
void bfa_attach_fcs(struct bfa_s *bfa);
void bfa_cb_init(void *bfad, bfa_status_t status);
void bfa_cb_stop(void *bfad, bfa_status_t status);
void bfa_cb_updateq(void *bfad, bfa_status_t status);

bfa_boolean_t bfa_intx(struct bfa_s *bfa);
void bfa_isr_enable(struct bfa_s *bfa);
void bfa_isr_disable(struct bfa_s *bfa);
void bfa_msix_getvecs(struct bfa_s *bfa, u32 *msix_vecs_bmap,
			u32 *num_vecs, u32 *max_vec_bit);
#define bfa_msix(__bfa, __vec) (__bfa)->msix.handler[__vec](__bfa, __vec)

void bfa_comp_deq(struct bfa_s *bfa, struct list_head *comp_q);
void bfa_comp_process(struct bfa_s *bfa, struct list_head *comp_q);
void bfa_comp_free(struct bfa_s *bfa, struct list_head *comp_q);

typedef void (*bfa_cb_ioc_t) (void *cbarg, enum bfa_status status);
void bfa_iocfc_get_attr(struct bfa_s *bfa, struct bfa_iocfc_attr_s *attr);
bfa_status_t bfa_iocfc_get_stats(struct bfa_s *bfa,
			struct bfa_iocfc_stats_s *stats,
			bfa_cb_ioc_t cbfn, void *cbarg);
bfa_status_t bfa_iocfc_clear_stats(struct bfa_s *bfa,
			bfa_cb_ioc_t cbfn, void *cbarg);
void bfa_get_attr(struct bfa_s *bfa, struct bfa_ioc_attr_s *ioc_attr);

void bfa_adapter_get_attr(struct bfa_s *bfa,
			struct bfa_adapter_attr_s *ad_attr);
u64 bfa_adapter_get_id(struct bfa_s *bfa);

bfa_status_t bfa_iocfc_israttr_set(struct bfa_s *bfa,
			struct bfa_iocfc_intr_attr_s *attr);

void bfa_iocfc_enable(struct bfa_s *bfa);
void bfa_iocfc_disable(struct bfa_s *bfa);
void bfa_ioc_auto_recover(bfa_boolean_t auto_recover);
void bfa_cb_ioc_disable(void *bfad);
void bfa_timer_tick(struct bfa_s *bfa);
#define bfa_timer_start(_bfa, _timer, _timercb, _arg, _timeout)	\
	bfa_timer_begin(&(_bfa)->timer_mod, _timer, _timercb, _arg, _timeout)


bfa_status_t bfa_debug_fwtrc(struct bfa_s *bfa, void *trcdata, int *trclen);
bfa_status_t bfa_debug_fwsave(struct bfa_s *bfa, void *trcdata, int *trclen);

#include "bfa_priv.h"

#endif 
