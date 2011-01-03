

#include <bfa.h>
#include <bfa_ioc.h>
#include <bfa_fwimg_priv.h>
#include <bfa_trcmod_priv.h>
#include <cs/bfa_debug.h>
#include <bfi/bfi_ioc.h>
#include <bfi/bfi_ctreg.h>
#include <aen/bfa_aen_ioc.h>
#include <aen/bfa_aen.h>
#include <log/bfa_log_hal.h>
#include <defs/bfa_defs_pci.h>

BFA_TRC_FILE(HAL, IOC);


#define BFA_IOC_TOV		2000	
#define BFA_IOC_HB_TOV		1000	
#define BFA_IOC_HB_FAIL_MAX	4
#define BFA_IOC_HWINIT_MAX	2
#define BFA_IOC_FWIMG_MINSZ     (16 * 1024)
#define BFA_IOC_TOV_RECOVER	(BFA_IOC_HB_FAIL_MAX * BFA_IOC_HB_TOV \
				+ BFA_IOC_TOV)

#define bfa_ioc_timer_start(__ioc)					\
	bfa_timer_begin((__ioc)->timer_mod, &(__ioc)->ioc_timer,	\
			bfa_ioc_timeout, (__ioc), BFA_IOC_TOV)
#define bfa_ioc_timer_stop(__ioc)   bfa_timer_stop(&(__ioc)->ioc_timer)

#define BFA_DBG_FWTRC_ENTS	(BFI_IOC_TRC_ENTS)
#define BFA_DBG_FWTRC_LEN					\
	(BFA_DBG_FWTRC_ENTS * sizeof(struct bfa_trc_s) +	\
	 (sizeof(struct bfa_trc_mod_s) -			\
	  BFA_TRC_MAX * sizeof(struct bfa_trc_s)))
#define BFA_DBG_FWTRC_OFF(_fn)	(BFI_IOC_TRC_OFF + BFA_DBG_FWTRC_LEN * (_fn))
#define bfa_ioc_stats(_ioc, _stats)	(_ioc)->stats._stats ++

#define BFA_FLASH_CHUNK_NO(off)         (off / BFI_FLASH_CHUNK_SZ_WORDS)
#define BFA_FLASH_OFFSET_IN_CHUNK(off)  (off % BFI_FLASH_CHUNK_SZ_WORDS)
#define BFA_FLASH_CHUNK_ADDR(chunkno)   (chunkno * BFI_FLASH_CHUNK_SZ_WORDS)
bfa_boolean_t   bfa_auto_recover = BFA_FALSE;


static void     bfa_ioc_aen_post(struct bfa_ioc_s *bfa,
				 enum bfa_ioc_aen_event event);
static void     bfa_ioc_hw_sem_get(struct bfa_ioc_s *ioc);
static void     bfa_ioc_hw_sem_release(struct bfa_ioc_s *ioc);
static void     bfa_ioc_hw_sem_get_cancel(struct bfa_ioc_s *ioc);
static void     bfa_ioc_hwinit(struct bfa_ioc_s *ioc, bfa_boolean_t force);
static void     bfa_ioc_timeout(void *ioc);
static void     bfa_ioc_send_enable(struct bfa_ioc_s *ioc);
static void     bfa_ioc_send_disable(struct bfa_ioc_s *ioc);
static void     bfa_ioc_send_getattr(struct bfa_ioc_s *ioc);
static void     bfa_ioc_hb_monitor(struct bfa_ioc_s *ioc);
static void     bfa_ioc_hb_stop(struct bfa_ioc_s *ioc);
static void     bfa_ioc_reset(struct bfa_ioc_s *ioc, bfa_boolean_t force);
static void     bfa_ioc_mbox_poll(struct bfa_ioc_s *ioc);
static void     bfa_ioc_mbox_hbfail(struct bfa_ioc_s *ioc);
static void     bfa_ioc_recover(struct bfa_ioc_s *ioc);
static bfa_boolean_t bfa_ioc_firmware_lock(struct bfa_ioc_s *ioc);
static void     bfa_ioc_firmware_unlock(struct bfa_ioc_s *ioc);
static void     bfa_ioc_disable_comp(struct bfa_ioc_s *ioc);
static void     bfa_ioc_lpu_stop(struct bfa_ioc_s *ioc);




enum ioc_event {
	IOC_E_ENABLE = 1,	
	IOC_E_DISABLE = 2,	
	IOC_E_TIMEOUT = 3,	
	IOC_E_FWREADY = 4,	
	IOC_E_FWRSP_GETATTR = 5,	
	IOC_E_FWRSP_ENABLE = 6,	
	IOC_E_FWRSP_DISABLE = 7,	
	IOC_E_HBFAIL = 8,	
	IOC_E_HWERROR = 9,	
	IOC_E_SEMLOCKED = 10,	
	IOC_E_DETACH = 11,	
};

bfa_fsm_state_decl(bfa_ioc, reset, struct bfa_ioc_s, enum ioc_event);
bfa_fsm_state_decl(bfa_ioc, fwcheck, struct bfa_ioc_s, enum ioc_event);
bfa_fsm_state_decl(bfa_ioc, mismatch, struct bfa_ioc_s, enum ioc_event);
bfa_fsm_state_decl(bfa_ioc, semwait, struct bfa_ioc_s, enum ioc_event);
bfa_fsm_state_decl(bfa_ioc, hwinit, struct bfa_ioc_s, enum ioc_event);
bfa_fsm_state_decl(bfa_ioc, enabling, struct bfa_ioc_s, enum ioc_event);
bfa_fsm_state_decl(bfa_ioc, getattr, struct bfa_ioc_s, enum ioc_event);
bfa_fsm_state_decl(bfa_ioc, op, struct bfa_ioc_s, enum ioc_event);
bfa_fsm_state_decl(bfa_ioc, initfail, struct bfa_ioc_s, enum ioc_event);
bfa_fsm_state_decl(bfa_ioc, hbfail, struct bfa_ioc_s, enum ioc_event);
bfa_fsm_state_decl(bfa_ioc, disabling, struct bfa_ioc_s, enum ioc_event);
bfa_fsm_state_decl(bfa_ioc, disabled, struct bfa_ioc_s, enum ioc_event);

static struct bfa_sm_table_s ioc_sm_table[] = {
	{BFA_SM(bfa_ioc_sm_reset), BFA_IOC_RESET},
	{BFA_SM(bfa_ioc_sm_fwcheck), BFA_IOC_FWMISMATCH},
	{BFA_SM(bfa_ioc_sm_mismatch), BFA_IOC_FWMISMATCH},
	{BFA_SM(bfa_ioc_sm_semwait), BFA_IOC_SEMWAIT},
	{BFA_SM(bfa_ioc_sm_hwinit), BFA_IOC_HWINIT},
	{BFA_SM(bfa_ioc_sm_enabling), BFA_IOC_HWINIT},
	{BFA_SM(bfa_ioc_sm_getattr), BFA_IOC_GETATTR},
	{BFA_SM(bfa_ioc_sm_op), BFA_IOC_OPERATIONAL},
	{BFA_SM(bfa_ioc_sm_initfail), BFA_IOC_INITFAIL},
	{BFA_SM(bfa_ioc_sm_hbfail), BFA_IOC_HBFAIL},
	{BFA_SM(bfa_ioc_sm_disabling), BFA_IOC_DISABLING},
	{BFA_SM(bfa_ioc_sm_disabled), BFA_IOC_DISABLED},
};


static void
bfa_ioc_sm_reset_entry(struct bfa_ioc_s *ioc)
{
	ioc->retry_count = 0;
	ioc->auto_recover = bfa_auto_recover;
}


static void
bfa_ioc_sm_reset(struct bfa_ioc_s *ioc, enum ioc_event event)
{
	bfa_trc(ioc, event);

	switch (event) {
	case IOC_E_ENABLE:
		bfa_fsm_set_state(ioc, bfa_ioc_sm_fwcheck);
		break;

	case IOC_E_DISABLE:
		bfa_ioc_disable_comp(ioc);
		break;

	case IOC_E_DETACH:
		break;

	default:
		bfa_sm_fault(ioc, event);
	}
}


static void
bfa_ioc_sm_fwcheck_entry(struct bfa_ioc_s *ioc)
{
	bfa_ioc_hw_sem_get(ioc);
}


static void
bfa_ioc_sm_fwcheck(struct bfa_ioc_s *ioc, enum ioc_event event)
{
	bfa_trc(ioc, event);

	switch (event) {
	case IOC_E_SEMLOCKED:
		if (bfa_ioc_firmware_lock(ioc)) {
			ioc->retry_count = 0;
			bfa_fsm_set_state(ioc, bfa_ioc_sm_hwinit);
		} else {
			bfa_ioc_hw_sem_release(ioc);
			bfa_fsm_set_state(ioc, bfa_ioc_sm_mismatch);
		}
		break;

	case IOC_E_DISABLE:
		bfa_ioc_disable_comp(ioc);
		

	case IOC_E_DETACH:
		bfa_ioc_hw_sem_get_cancel(ioc);
		bfa_fsm_set_state(ioc, bfa_ioc_sm_reset);
		break;

	case IOC_E_FWREADY:
		break;

	default:
		bfa_sm_fault(ioc, event);
	}
}


static void
bfa_ioc_sm_mismatch_entry(struct bfa_ioc_s *ioc)
{
	
	if (ioc->retry_count == 0) {
		ioc->cbfn->enable_cbfn(ioc->bfa, BFA_STATUS_IOC_FAILURE);
		bfa_ioc_aen_post(ioc, BFA_IOC_AEN_FWMISMATCH);
	}
	ioc->retry_count++;
	bfa_ioc_timer_start(ioc);
}


static void
bfa_ioc_sm_mismatch(struct bfa_ioc_s *ioc, enum ioc_event event)
{
	bfa_trc(ioc, event);

	switch (event) {
	case IOC_E_TIMEOUT:
		bfa_fsm_set_state(ioc, bfa_ioc_sm_fwcheck);
		break;

	case IOC_E_DISABLE:
		bfa_ioc_disable_comp(ioc);
		

	case IOC_E_DETACH:
		bfa_ioc_timer_stop(ioc);
		bfa_fsm_set_state(ioc, bfa_ioc_sm_reset);
		break;

	case IOC_E_FWREADY:
		break;

	default:
		bfa_sm_fault(ioc, event);
	}
}


static void
bfa_ioc_sm_semwait_entry(struct bfa_ioc_s *ioc)
{
	bfa_ioc_hw_sem_get(ioc);
}


static void
bfa_ioc_sm_semwait(struct bfa_ioc_s *ioc, enum ioc_event event)
{
	bfa_trc(ioc, event);

	switch (event) {
	case IOC_E_SEMLOCKED:
		ioc->retry_count = 0;
		bfa_fsm_set_state(ioc, bfa_ioc_sm_hwinit);
		break;

	case IOC_E_DISABLE:
		bfa_ioc_hw_sem_get_cancel(ioc);
		bfa_fsm_set_state(ioc, bfa_ioc_sm_disabled);
		break;

	default:
		bfa_sm_fault(ioc, event);
	}
}


static void
bfa_ioc_sm_hwinit_entry(struct bfa_ioc_s *ioc)
{
	bfa_ioc_timer_start(ioc);
	bfa_ioc_reset(ioc, BFA_FALSE);
}


static void
bfa_ioc_sm_hwinit(struct bfa_ioc_s *ioc, enum ioc_event event)
{
	bfa_trc(ioc, event);

	switch (event) {
	case IOC_E_FWREADY:
		bfa_ioc_timer_stop(ioc);
		bfa_fsm_set_state(ioc, bfa_ioc_sm_enabling);
		break;

	case IOC_E_HWERROR:
		bfa_ioc_timer_stop(ioc);
		

	case IOC_E_TIMEOUT:
		ioc->retry_count++;
		if (ioc->retry_count < BFA_IOC_HWINIT_MAX) {
			bfa_ioc_timer_start(ioc);
			bfa_ioc_reset(ioc, BFA_TRUE);
			break;
		}

		bfa_ioc_hw_sem_release(ioc);
		bfa_fsm_set_state(ioc, bfa_ioc_sm_initfail);
		break;

	case IOC_E_DISABLE:
		bfa_ioc_hw_sem_release(ioc);
		bfa_ioc_timer_stop(ioc);
		bfa_fsm_set_state(ioc, bfa_ioc_sm_disabled);
		break;

	default:
		bfa_sm_fault(ioc, event);
	}
}


static void
bfa_ioc_sm_enabling_entry(struct bfa_ioc_s *ioc)
{
	bfa_ioc_timer_start(ioc);
	bfa_ioc_send_enable(ioc);
}


static void
bfa_ioc_sm_enabling(struct bfa_ioc_s *ioc, enum ioc_event event)
{
	bfa_trc(ioc, event);

	switch (event) {
	case IOC_E_FWRSP_ENABLE:
		bfa_ioc_timer_stop(ioc);
		bfa_ioc_hw_sem_release(ioc);
		bfa_fsm_set_state(ioc, bfa_ioc_sm_getattr);
		break;

	case IOC_E_HWERROR:
		bfa_ioc_timer_stop(ioc);
		

	case IOC_E_TIMEOUT:
		ioc->retry_count++;
		if (ioc->retry_count < BFA_IOC_HWINIT_MAX) {
			bfa_reg_write(ioc->ioc_regs.ioc_fwstate,
				      BFI_IOC_UNINIT);
			bfa_fsm_set_state(ioc, bfa_ioc_sm_hwinit);
			break;
		}

		bfa_ioc_hw_sem_release(ioc);
		bfa_fsm_set_state(ioc, bfa_ioc_sm_initfail);
		break;

	case IOC_E_DISABLE:
		bfa_ioc_timer_stop(ioc);
		bfa_ioc_hw_sem_release(ioc);
		bfa_fsm_set_state(ioc, bfa_ioc_sm_disabled);
		break;

	case IOC_E_FWREADY:
		bfa_ioc_send_enable(ioc);
		break;

	default:
		bfa_sm_fault(ioc, event);
	}
}


static void
bfa_ioc_sm_getattr_entry(struct bfa_ioc_s *ioc)
{
	bfa_ioc_timer_start(ioc);
	bfa_ioc_send_getattr(ioc);
}


static void
bfa_ioc_sm_getattr(struct bfa_ioc_s *ioc, enum ioc_event event)
{
	bfa_trc(ioc, event);

	switch (event) {
	case IOC_E_FWRSP_GETATTR:
		bfa_ioc_timer_stop(ioc);
		bfa_fsm_set_state(ioc, bfa_ioc_sm_op);
		break;

	case IOC_E_HWERROR:
		bfa_ioc_timer_stop(ioc);
		

	case IOC_E_TIMEOUT:
		bfa_fsm_set_state(ioc, bfa_ioc_sm_initfail);
		break;

	case IOC_E_DISABLE:
		bfa_ioc_timer_stop(ioc);
		bfa_fsm_set_state(ioc, bfa_ioc_sm_disabled);
		break;

	default:
		bfa_sm_fault(ioc, event);
	}
}


static void
bfa_ioc_sm_op_entry(struct bfa_ioc_s *ioc)
{
	ioc->cbfn->enable_cbfn(ioc->bfa, BFA_STATUS_OK);
	bfa_ioc_hb_monitor(ioc);
	bfa_ioc_aen_post(ioc, BFA_IOC_AEN_ENABLE);
}

static void
bfa_ioc_sm_op(struct bfa_ioc_s *ioc, enum ioc_event event)
{
	bfa_trc(ioc, event);

	switch (event) {
	case IOC_E_ENABLE:
		break;

	case IOC_E_DISABLE:
		bfa_ioc_hb_stop(ioc);
		bfa_fsm_set_state(ioc, bfa_ioc_sm_disabling);
		break;

	case IOC_E_HWERROR:
	case IOC_E_FWREADY:
		
		bfa_ioc_hb_stop(ioc);
		

	case IOC_E_HBFAIL:
		bfa_fsm_set_state(ioc, bfa_ioc_sm_hbfail);
		break;

	default:
		bfa_sm_fault(ioc, event);
	}
}


static void
bfa_ioc_sm_disabling_entry(struct bfa_ioc_s *ioc)
{
	bfa_ioc_aen_post(ioc, BFA_IOC_AEN_DISABLE);
	bfa_ioc_timer_start(ioc);
	bfa_ioc_send_disable(ioc);
}


static void
bfa_ioc_sm_disabling(struct bfa_ioc_s *ioc, enum ioc_event event)
{
	bfa_trc(ioc, event);

	switch (event) {
	case IOC_E_HWERROR:
	case IOC_E_FWRSP_DISABLE:
		bfa_ioc_timer_stop(ioc);
		

	case IOC_E_TIMEOUT:
		bfa_fsm_set_state(ioc, bfa_ioc_sm_disabled);
		break;

	default:
		bfa_sm_fault(ioc, event);
	}
}


static void
bfa_ioc_sm_disabled_entry(struct bfa_ioc_s *ioc)
{
	bfa_ioc_disable_comp(ioc);
}

static void
bfa_ioc_sm_disabled(struct bfa_ioc_s *ioc, enum ioc_event event)
{
	bfa_trc(ioc, event);

	switch (event) {
	case IOC_E_ENABLE:
		bfa_fsm_set_state(ioc, bfa_ioc_sm_semwait);
		break;

	case IOC_E_DISABLE:
		ioc->cbfn->disable_cbfn(ioc->bfa);
		break;

	case IOC_E_FWREADY:
		break;

	case IOC_E_DETACH:
		bfa_ioc_firmware_unlock(ioc);
		bfa_fsm_set_state(ioc, bfa_ioc_sm_reset);
		break;

	default:
		bfa_sm_fault(ioc, event);
	}
}


static void
bfa_ioc_sm_initfail_entry(struct bfa_ioc_s *ioc)
{
	ioc->cbfn->enable_cbfn(ioc->bfa, BFA_STATUS_IOC_FAILURE);
	bfa_ioc_timer_start(ioc);
}


static void
bfa_ioc_sm_initfail(struct bfa_ioc_s *ioc, enum ioc_event event)
{
	bfa_trc(ioc, event);

	switch (event) {
	case IOC_E_DISABLE:
		bfa_ioc_timer_stop(ioc);
		bfa_fsm_set_state(ioc, bfa_ioc_sm_disabled);
		break;

	case IOC_E_DETACH:
		bfa_ioc_timer_stop(ioc);
		bfa_ioc_firmware_unlock(ioc);
		bfa_fsm_set_state(ioc, bfa_ioc_sm_reset);
		break;

	case IOC_E_TIMEOUT:
		bfa_fsm_set_state(ioc, bfa_ioc_sm_semwait);
		break;

	default:
		bfa_sm_fault(ioc, event);
	}
}


static void
bfa_ioc_sm_hbfail_entry(struct bfa_ioc_s *ioc)
{
	struct list_head *qe;
	struct bfa_ioc_hbfail_notify_s *notify;

	
	bfa_ioc_lpu_stop(ioc);
	bfa_reg_write(ioc->ioc_regs.ioc_fwstate, BFI_IOC_HBFAIL);

	if (ioc->pcidev.device_id == BFA_PCI_DEVICE_ID_CT) {
		bfa_reg_write(ioc->ioc_regs.ll_halt, __FW_INIT_HALT_P);
		
		bfa_reg_read(ioc->ioc_regs.ll_halt);
	}

	
	ioc->cbfn->hbfail_cbfn(ioc->bfa);
	list_for_each(qe, &ioc->hb_notify_q) {
		notify = (struct bfa_ioc_hbfail_notify_s *)qe;
		notify->cbfn(notify->cbarg);
	}

	
	bfa_ioc_mbox_hbfail(ioc);
	bfa_ioc_aen_post(ioc, BFA_IOC_AEN_HBFAIL);

	
	if (ioc->auto_recover) {
		bfa_timer_begin(ioc->timer_mod, &ioc->ioc_timer,
				bfa_ioc_timeout, ioc, BFA_IOC_TOV_RECOVER);
	}
}


static void
bfa_ioc_sm_hbfail(struct bfa_ioc_s *ioc, enum ioc_event event)
{
	bfa_trc(ioc, event);

	switch (event) {

	case IOC_E_ENABLE:
		ioc->cbfn->enable_cbfn(ioc->bfa, BFA_STATUS_IOC_FAILURE);
		break;

	case IOC_E_DISABLE:
		if (ioc->auto_recover)
			bfa_ioc_timer_stop(ioc);
		bfa_fsm_set_state(ioc, bfa_ioc_sm_disabled);
		break;

	case IOC_E_TIMEOUT:
		bfa_fsm_set_state(ioc, bfa_ioc_sm_semwait);
		break;

	case IOC_E_FWREADY:
		
		break;

	default:
		bfa_sm_fault(ioc, event);
	}
}





static void
bfa_ioc_disable_comp(struct bfa_ioc_s *ioc)
{
	struct list_head *qe;
	struct bfa_ioc_hbfail_notify_s *notify;

	ioc->cbfn->disable_cbfn(ioc->bfa);

	
	list_for_each(qe, &ioc->hb_notify_q) {
		notify = (struct bfa_ioc_hbfail_notify_s *)qe;
		notify->cbfn(notify->cbarg);
	}
}

static void
bfa_ioc_sem_timeout(void *ioc_arg)
{
	struct bfa_ioc_s *ioc = (struct bfa_ioc_s *)ioc_arg;

	bfa_ioc_hw_sem_get(ioc);
}

static void
bfa_ioc_usage_sem_get(struct bfa_ioc_s *ioc)
{
	u32        r32;
	int             cnt = 0;
#define BFA_SEM_SPINCNT	1000

	do {
		r32 = bfa_reg_read(ioc->ioc_regs.ioc_usage_sem_reg);
		cnt++;
		if (cnt > BFA_SEM_SPINCNT)
			break;
	} while (r32 != 0);
	bfa_assert(cnt < BFA_SEM_SPINCNT);
}

static void
bfa_ioc_usage_sem_release(struct bfa_ioc_s *ioc)
{
	bfa_reg_write(ioc->ioc_regs.ioc_usage_sem_reg, 1);
}

static void
bfa_ioc_hw_sem_get(struct bfa_ioc_s *ioc)
{
	u32        r32;

	
	r32 = bfa_reg_read(ioc->ioc_regs.ioc_sem_reg);
	if (r32 == 0) {
		bfa_fsm_send_event(ioc, IOC_E_SEMLOCKED);
		return;
	}

	bfa_timer_begin(ioc->timer_mod, &ioc->sem_timer, bfa_ioc_sem_timeout,
			ioc, BFA_IOC_TOV);
}

static void
bfa_ioc_hw_sem_release(struct bfa_ioc_s *ioc)
{
	bfa_reg_write(ioc->ioc_regs.ioc_sem_reg, 1);
}

static void
bfa_ioc_hw_sem_get_cancel(struct bfa_ioc_s *ioc)
{
	bfa_timer_stop(&ioc->sem_timer);
}


static void
bfa_ioc_lmem_init(struct bfa_ioc_s *ioc)
{
	u32        pss_ctl;
	int             i;
#define PSS_LMEM_INIT_TIME  10000

	pss_ctl = bfa_reg_read(ioc->ioc_regs.pss_ctl_reg);
	pss_ctl &= ~__PSS_LMEM_RESET;
	pss_ctl |= __PSS_LMEM_INIT_EN;
	pss_ctl |= __PSS_I2C_CLK_DIV(3UL); 
	bfa_reg_write(ioc->ioc_regs.pss_ctl_reg, pss_ctl);

	
	i = 0;
	do {
		pss_ctl = bfa_reg_read(ioc->ioc_regs.pss_ctl_reg);
		i++;
	} while (!(pss_ctl & __PSS_LMEM_INIT_DONE) && (i < PSS_LMEM_INIT_TIME));

	
	bfa_assert(pss_ctl & __PSS_LMEM_INIT_DONE);
	bfa_trc(ioc, pss_ctl);

	pss_ctl &= ~(__PSS_LMEM_INIT_DONE | __PSS_LMEM_INIT_EN);
	bfa_reg_write(ioc->ioc_regs.pss_ctl_reg, pss_ctl);
}

static void
bfa_ioc_lpu_start(struct bfa_ioc_s *ioc)
{
	u32        pss_ctl;

	
	pss_ctl = bfa_reg_read(ioc->ioc_regs.pss_ctl_reg);
	pss_ctl &= ~__PSS_LPU0_RESET;

	bfa_reg_write(ioc->ioc_regs.pss_ctl_reg, pss_ctl);
}

static void
bfa_ioc_lpu_stop(struct bfa_ioc_s *ioc)
{
	u32        pss_ctl;

	
	pss_ctl = bfa_reg_read(ioc->ioc_regs.pss_ctl_reg);
	pss_ctl |= (__PSS_LPU0_RESET | __PSS_LPU1_RESET);

	bfa_reg_write(ioc->ioc_regs.pss_ctl_reg, pss_ctl);
}


static void
bfa_ioc_fwver_get(struct bfa_ioc_s *ioc, struct bfi_ioc_image_hdr_s *fwhdr)
{
	u32        pgnum, pgoff;
	u32        loff = 0;
	int             i;
	u32       *fwsig = (u32 *) fwhdr;

	pgnum = bfa_ioc_smem_pgnum(ioc, loff);
	pgoff = bfa_ioc_smem_pgoff(ioc, loff);
	bfa_reg_write(ioc->ioc_regs.host_page_num_fn, pgnum);

	for (i = 0; i < (sizeof(struct bfi_ioc_image_hdr_s) / sizeof(u32));
	     i++) {
		fwsig[i] = bfa_mem_read(ioc->ioc_regs.smem_page_start, loff);
		loff += sizeof(u32);
	}
}

static u32 *
bfa_ioc_fwimg_get_chunk(struct bfa_ioc_s *ioc, u32 off)
{
	if (ioc->ctdev)
		return bfi_image_ct_get_chunk(off);
	return bfi_image_cb_get_chunk(off);
}

static          u32
bfa_ioc_fwimg_get_size(struct bfa_ioc_s *ioc)
{
return (ioc->ctdev) ? bfi_image_ct_size : bfi_image_cb_size;
}


static          bfa_boolean_t
bfa_ioc_fwver_cmp(struct bfa_ioc_s *ioc, struct bfi_ioc_image_hdr_s *fwhdr)
{
	struct bfi_ioc_image_hdr_s *drv_fwhdr;
	int             i;

	drv_fwhdr =
		(struct bfi_ioc_image_hdr_s *)bfa_ioc_fwimg_get_chunk(ioc, 0);

	for (i = 0; i < BFI_IOC_MD5SUM_SZ; i++) {
		if (fwhdr->md5sum[i] != drv_fwhdr->md5sum[i]) {
			bfa_trc(ioc, i);
			bfa_trc(ioc, fwhdr->md5sum[i]);
			bfa_trc(ioc, drv_fwhdr->md5sum[i]);
			return BFA_FALSE;
		}
	}

	bfa_trc(ioc, fwhdr->md5sum[0]);
	return BFA_TRUE;
}


static          bfa_boolean_t
bfa_ioc_fwver_valid(struct bfa_ioc_s *ioc)
{
	struct bfi_ioc_image_hdr_s fwhdr, *drv_fwhdr;

	
	if (bfa_ioc_fwimg_get_size(ioc) < BFA_IOC_FWIMG_MINSZ)
		return BFA_TRUE;

	bfa_ioc_fwver_get(ioc, &fwhdr);
	drv_fwhdr =
		(struct bfi_ioc_image_hdr_s *)bfa_ioc_fwimg_get_chunk(ioc, 0);

	if (fwhdr.signature != drv_fwhdr->signature) {
		bfa_trc(ioc, fwhdr.signature);
		bfa_trc(ioc, drv_fwhdr->signature);
		return BFA_FALSE;
	}

	if (fwhdr.exec != drv_fwhdr->exec) {
		bfa_trc(ioc, fwhdr.exec);
		bfa_trc(ioc, drv_fwhdr->exec);
		return BFA_FALSE;
	}

	return bfa_ioc_fwver_cmp(ioc, &fwhdr);
}


static          bfa_boolean_t
bfa_ioc_firmware_lock(struct bfa_ioc_s *ioc)
{
	enum bfi_ioc_state ioc_fwstate;
	u32        usecnt;
	struct bfi_ioc_image_hdr_s fwhdr;

	
	if (!ioc->cna)
		return BFA_TRUE;

	
	if (bfa_ioc_fwimg_get_size(ioc) < BFA_IOC_FWIMG_MINSZ)
		return BFA_TRUE;

	bfa_ioc_usage_sem_get(ioc);
	usecnt = bfa_reg_read(ioc->ioc_regs.ioc_usage_reg);

	
	if (usecnt == 0) {
		bfa_reg_write(ioc->ioc_regs.ioc_usage_reg, 1);
		bfa_ioc_usage_sem_release(ioc);
		bfa_trc(ioc, usecnt);
		return BFA_TRUE;
	}

	ioc_fwstate = bfa_reg_read(ioc->ioc_regs.ioc_fwstate);
	bfa_trc(ioc, ioc_fwstate);

	
	bfa_assert(ioc_fwstate != BFI_IOC_UNINIT);

	
	bfa_ioc_fwver_get(ioc, &fwhdr);
	if (!bfa_ioc_fwver_cmp(ioc, &fwhdr)) {
		bfa_ioc_usage_sem_release(ioc);
		bfa_trc(ioc, usecnt);
		return BFA_FALSE;
	}

	
	usecnt++;
	bfa_reg_write(ioc->ioc_regs.ioc_usage_reg, usecnt);
	bfa_ioc_usage_sem_release(ioc);
	bfa_trc(ioc, usecnt);
	return BFA_TRUE;
}

static void
bfa_ioc_firmware_unlock(struct bfa_ioc_s *ioc)
{
	u32        usecnt;

	
	if (!ioc->cna || (bfa_ioc_fwimg_get_size(ioc) < BFA_IOC_FWIMG_MINSZ))
		return;

	
	bfa_ioc_usage_sem_get(ioc);
	usecnt = bfa_reg_read(ioc->ioc_regs.ioc_usage_reg);
	bfa_assert(usecnt > 0);

	usecnt--;
	bfa_reg_write(ioc->ioc_regs.ioc_usage_reg, usecnt);
	bfa_trc(ioc, usecnt);

	bfa_ioc_usage_sem_release(ioc);
}


static void
bfa_ioc_msgflush(struct bfa_ioc_s *ioc)
{
	u32        r32;

	r32 = bfa_reg_read(ioc->ioc_regs.lpu_mbox_cmd);
	if (r32)
		bfa_reg_write(ioc->ioc_regs.lpu_mbox_cmd, 1);
}


static void
bfa_ioc_hwinit(struct bfa_ioc_s *ioc, bfa_boolean_t force)
{
	enum bfi_ioc_state ioc_fwstate;
	bfa_boolean_t   fwvalid;

	ioc_fwstate = bfa_reg_read(ioc->ioc_regs.ioc_fwstate);

	if (force)
		ioc_fwstate = BFI_IOC_UNINIT;

	bfa_trc(ioc, ioc_fwstate);

	
	fwvalid = (ioc_fwstate == BFI_IOC_UNINIT) ?
			BFA_FALSE : bfa_ioc_fwver_valid(ioc);

	if (!fwvalid) {
		bfa_ioc_boot(ioc, BFI_BOOT_TYPE_NORMAL, ioc->pcidev.device_id);
		return;
	}

	
	if (ioc_fwstate == BFI_IOC_INITING) {
		bfa_trc(ioc, ioc_fwstate);
		ioc->cbfn->reset_cbfn(ioc->bfa);
		return;
	}

	
	if (ioc_fwstate == BFI_IOC_DISABLED || ioc_fwstate == BFI_IOC_OP) {
		bfa_trc(ioc, ioc_fwstate);

		
		bfa_ioc_msgflush(ioc);
		ioc->cbfn->reset_cbfn(ioc->bfa);
		bfa_fsm_send_event(ioc, IOC_E_FWREADY);
		return;
	}

	
	bfa_ioc_boot(ioc, BFI_BOOT_TYPE_NORMAL, ioc->pcidev.device_id);
}

static void
bfa_ioc_timeout(void *ioc_arg)
{
	struct bfa_ioc_s *ioc = (struct bfa_ioc_s *)ioc_arg;

	bfa_trc(ioc, 0);
	bfa_fsm_send_event(ioc, IOC_E_TIMEOUT);
}

void
bfa_ioc_mbox_send(struct bfa_ioc_s *ioc, void *ioc_msg, int len)
{
	u32       *msgp = (u32 *) ioc_msg;
	u32        i;

	bfa_trc(ioc, msgp[0]);
	bfa_trc(ioc, len);

	bfa_assert(len <= BFI_IOC_MSGLEN_MAX);

	
	for (i = 0; i < len / sizeof(u32); i++)
		bfa_reg_write(ioc->ioc_regs.hfn_mbox + i * sizeof(u32),
			      bfa_os_wtole(msgp[i]));

	for (; i < BFI_IOC_MSGLEN_MAX / sizeof(u32); i++)
		bfa_reg_write(ioc->ioc_regs.hfn_mbox + i * sizeof(u32), 0);

	
	bfa_reg_write(ioc->ioc_regs.hfn_mbox_cmd, 1);
	(void)bfa_reg_read(ioc->ioc_regs.hfn_mbox_cmd);
}

static void
bfa_ioc_send_enable(struct bfa_ioc_s *ioc)
{
	struct bfi_ioc_ctrl_req_s enable_req;

	bfi_h2i_set(enable_req.mh, BFI_MC_IOC, BFI_IOC_H2I_ENABLE_REQ,
		    bfa_ioc_portid(ioc));
	enable_req.ioc_class = ioc->ioc_mc;
	bfa_ioc_mbox_send(ioc, &enable_req, sizeof(struct bfi_ioc_ctrl_req_s));
}

static void
bfa_ioc_send_disable(struct bfa_ioc_s *ioc)
{
	struct bfi_ioc_ctrl_req_s disable_req;

	bfi_h2i_set(disable_req.mh, BFI_MC_IOC, BFI_IOC_H2I_DISABLE_REQ,
		    bfa_ioc_portid(ioc));
	bfa_ioc_mbox_send(ioc, &disable_req, sizeof(struct bfi_ioc_ctrl_req_s));
}

static void
bfa_ioc_send_getattr(struct bfa_ioc_s *ioc)
{
	struct bfi_ioc_getattr_req_s attr_req;

	bfi_h2i_set(attr_req.mh, BFI_MC_IOC, BFI_IOC_H2I_GETATTR_REQ,
		    bfa_ioc_portid(ioc));
	bfa_dma_be_addr_set(attr_req.attr_addr, ioc->attr_dma.pa);
	bfa_ioc_mbox_send(ioc, &attr_req, sizeof(attr_req));
}

static void
bfa_ioc_hb_check(void *cbarg)
{
	struct bfa_ioc_s *ioc = cbarg;
	u32        hb_count;

	hb_count = bfa_reg_read(ioc->ioc_regs.heartbeat);
	if (ioc->hb_count == hb_count) {
		ioc->hb_fail++;
	} else {
		ioc->hb_count = hb_count;
		ioc->hb_fail = 0;
	}

	if (ioc->hb_fail >= BFA_IOC_HB_FAIL_MAX) {
		bfa_log(ioc->logm, BFA_LOG_HAL_HEARTBEAT_FAILURE, hb_count);
		ioc->hb_fail = 0;
		bfa_ioc_recover(ioc);
		return;
	}

	bfa_ioc_mbox_poll(ioc);
	bfa_timer_begin(ioc->timer_mod, &ioc->ioc_timer, bfa_ioc_hb_check, ioc,
			BFA_IOC_HB_TOV);
}

static void
bfa_ioc_hb_monitor(struct bfa_ioc_s *ioc)
{
	ioc->hb_fail = 0;
	ioc->hb_count = bfa_reg_read(ioc->ioc_regs.heartbeat);
	bfa_timer_begin(ioc->timer_mod, &ioc->ioc_timer, bfa_ioc_hb_check, ioc,
			BFA_IOC_HB_TOV);
}

static void
bfa_ioc_hb_stop(struct bfa_ioc_s *ioc)
{
	bfa_timer_stop(&ioc->ioc_timer);
}


static struct {
	u32        hfn_mbox, lpu_mbox, hfn_pgn;
} iocreg_fnreg[] = {
	{
	HOSTFN0_LPU_MBOX0_0, LPU_HOSTFN0_MBOX0_0, HOST_PAGE_NUM_FN0}, {
	HOSTFN1_LPU_MBOX0_8, LPU_HOSTFN1_MBOX0_8, HOST_PAGE_NUM_FN1}, {
	HOSTFN2_LPU_MBOX0_0, LPU_HOSTFN2_MBOX0_0, HOST_PAGE_NUM_FN2}, {
	HOSTFN3_LPU_MBOX0_8, LPU_HOSTFN3_MBOX0_8, HOST_PAGE_NUM_FN3}
};


static struct {
	u32        hfn, lpu;
} iocreg_mbcmd_p0[] = {
	{
	HOSTFN0_LPU0_MBOX0_CMD_STAT, LPU0_HOSTFN0_MBOX0_CMD_STAT}, {
	HOSTFN1_LPU0_MBOX0_CMD_STAT, LPU0_HOSTFN1_MBOX0_CMD_STAT}, {
	HOSTFN2_LPU0_MBOX0_CMD_STAT, LPU0_HOSTFN2_MBOX0_CMD_STAT}, {
	HOSTFN3_LPU0_MBOX0_CMD_STAT, LPU0_HOSTFN3_MBOX0_CMD_STAT}
};


static struct {
	u32        hfn, lpu;
} iocreg_mbcmd_p1[] = {
	{
	HOSTFN0_LPU1_MBOX0_CMD_STAT, LPU1_HOSTFN0_MBOX0_CMD_STAT}, {
	HOSTFN1_LPU1_MBOX0_CMD_STAT, LPU1_HOSTFN1_MBOX0_CMD_STAT}, {
	HOSTFN2_LPU1_MBOX0_CMD_STAT, LPU1_HOSTFN2_MBOX0_CMD_STAT}, {
	HOSTFN3_LPU1_MBOX0_CMD_STAT, LPU1_HOSTFN3_MBOX0_CMD_STAT}
};


static struct {
	u32        isr, msk;
} iocreg_shirq_next[] = {
	{
	HOSTFN1_INT_STATUS, HOSTFN1_INT_MSK}, {
	HOSTFN2_INT_STATUS, HOSTFN2_INT_MSK}, {
	HOSTFN3_INT_STATUS, HOSTFN3_INT_MSK}, {
HOSTFN0_INT_STATUS, HOSTFN0_INT_MSK},};

static void
bfa_ioc_reg_init(struct bfa_ioc_s *ioc)
{
	bfa_os_addr_t   rb;
	int             pcifn = bfa_ioc_pcifn(ioc);

	rb = bfa_ioc_bar0(ioc);

	ioc->ioc_regs.hfn_mbox = rb + iocreg_fnreg[pcifn].hfn_mbox;
	ioc->ioc_regs.lpu_mbox = rb + iocreg_fnreg[pcifn].lpu_mbox;
	ioc->ioc_regs.host_page_num_fn = rb + iocreg_fnreg[pcifn].hfn_pgn;

	if (ioc->port_id == 0) {
		ioc->ioc_regs.heartbeat = rb + BFA_IOC0_HBEAT_REG;
		ioc->ioc_regs.ioc_fwstate = rb + BFA_IOC0_STATE_REG;
		ioc->ioc_regs.hfn_mbox_cmd = rb + iocreg_mbcmd_p0[pcifn].hfn;
		ioc->ioc_regs.lpu_mbox_cmd = rb + iocreg_mbcmd_p0[pcifn].lpu;
		ioc->ioc_regs.ll_halt = rb + FW_INIT_HALT_P0;
	} else {
		ioc->ioc_regs.heartbeat = (rb + BFA_IOC1_HBEAT_REG);
		ioc->ioc_regs.ioc_fwstate = (rb + BFA_IOC1_STATE_REG);
		ioc->ioc_regs.hfn_mbox_cmd = rb + iocreg_mbcmd_p1[pcifn].hfn;
		ioc->ioc_regs.lpu_mbox_cmd = rb + iocreg_mbcmd_p1[pcifn].lpu;
		ioc->ioc_regs.ll_halt = rb + FW_INIT_HALT_P1;
	}

	
	ioc->ioc_regs.shirq_isr_next = rb + iocreg_shirq_next[pcifn].isr;
	ioc->ioc_regs.shirq_msk_next = rb + iocreg_shirq_next[pcifn].msk;

	
	ioc->ioc_regs.pss_ctl_reg = (rb + PSS_CTL_REG);
	ioc->ioc_regs.app_pll_fast_ctl_reg = (rb + APP_PLL_425_CTL_REG);
	ioc->ioc_regs.app_pll_slow_ctl_reg = (rb + APP_PLL_312_CTL_REG);

	
	ioc->ioc_regs.ioc_sem_reg = (rb + HOST_SEM0_REG);
	ioc->ioc_regs.ioc_usage_sem_reg = (rb + HOST_SEM1_REG);
	ioc->ioc_regs.ioc_usage_reg = (rb + BFA_FW_USE_COUNT);

	
	ioc->ioc_regs.smem_page_start = (rb + PSS_SMEM_PAGE_START);
	ioc->ioc_regs.smem_pg0 = BFI_IOC_SMEM_PG0_CB;
	if (ioc->pcidev.device_id == BFA_PCI_DEVICE_ID_CT)
		ioc->ioc_regs.smem_pg0 = BFI_IOC_SMEM_PG0_CT;
}


static void
bfa_ioc_download_fw(struct bfa_ioc_s *ioc, u32 boot_type,
		    u32 boot_param)
{
	u32       *fwimg;
	u32        pgnum, pgoff;
	u32        loff = 0;
	u32        chunkno = 0;
	u32        i;

	
	bfa_ioc_lmem_init(ioc);

	
	bfa_trc(ioc, bfa_ioc_fwimg_get_size(ioc));
	if (bfa_ioc_fwimg_get_size(ioc) < BFA_IOC_FWIMG_MINSZ)
		boot_type = BFI_BOOT_TYPE_FLASH;
	fwimg = bfa_ioc_fwimg_get_chunk(ioc, chunkno);
	fwimg[BFI_BOOT_TYPE_OFF / sizeof(u32)] = bfa_os_swap32(boot_type);
	fwimg[BFI_BOOT_PARAM_OFF / sizeof(u32)] =
		bfa_os_swap32(boot_param);

	pgnum = bfa_ioc_smem_pgnum(ioc, loff);
	pgoff = bfa_ioc_smem_pgoff(ioc, loff);

	bfa_reg_write(ioc->ioc_regs.host_page_num_fn, pgnum);

	for (i = 0; i < bfa_ioc_fwimg_get_size(ioc); i++) {

		if (BFA_FLASH_CHUNK_NO(i) != chunkno) {
			chunkno = BFA_FLASH_CHUNK_NO(i);
			fwimg = bfa_ioc_fwimg_get_chunk(ioc,
					BFA_FLASH_CHUNK_ADDR(chunkno));
		}

		
		bfa_mem_write(ioc->ioc_regs.smem_page_start, loff,
			      fwimg[BFA_FLASH_OFFSET_IN_CHUNK(i)]);

		loff += sizeof(u32);

		
		loff = PSS_SMEM_PGOFF(loff);
		if (loff == 0) {
			pgnum++;
			bfa_reg_write(ioc->ioc_regs.host_page_num_fn, pgnum);
		}
	}

	bfa_reg_write(ioc->ioc_regs.host_page_num_fn,
		      bfa_ioc_smem_pgnum(ioc, 0));
}

static void
bfa_ioc_reset(struct bfa_ioc_s *ioc, bfa_boolean_t force)
{
	bfa_ioc_hwinit(ioc, force);
}


static void
bfa_ioc_getattr_reply(struct bfa_ioc_s *ioc)
{
	struct bfi_ioc_attr_s *attr = ioc->attr;

	attr->adapter_prop = bfa_os_ntohl(attr->adapter_prop);
	attr->maxfrsize = bfa_os_ntohs(attr->maxfrsize);

	bfa_fsm_send_event(ioc, IOC_E_FWRSP_GETATTR);
}


static void
bfa_ioc_mbox_attach(struct bfa_ioc_s *ioc)
{
	struct bfa_ioc_mbox_mod_s *mod = &ioc->mbox_mod;
	int             mc;

	INIT_LIST_HEAD(&mod->cmd_q);
	for (mc = 0; mc < BFI_MC_MAX; mc++) {
		mod->mbhdlr[mc].cbfn = NULL;
		mod->mbhdlr[mc].cbarg = ioc->bfa;
	}
}


static void
bfa_ioc_mbox_poll(struct bfa_ioc_s *ioc)
{
	struct bfa_ioc_mbox_mod_s *mod = &ioc->mbox_mod;
	struct bfa_mbox_cmd_s *cmd;
	u32        stat;

	
	if (list_empty(&mod->cmd_q))
		return;

	
	stat = bfa_reg_read(ioc->ioc_regs.hfn_mbox_cmd);
	if (stat)
		return;

	
	bfa_q_deq(&mod->cmd_q, &cmd);
	bfa_ioc_mbox_send(ioc, cmd->msg, sizeof(cmd->msg));
}


static void
bfa_ioc_mbox_hbfail(struct bfa_ioc_s *ioc)
{
	struct bfa_ioc_mbox_mod_s *mod = &ioc->mbox_mod;
	struct bfa_mbox_cmd_s *cmd;

	while (!list_empty(&mod->cmd_q))
		bfa_q_deq(&mod->cmd_q, &cmd);
}



#define FNC_PERS_FN_SHIFT(__fn)	((__fn) * 8)
static void
bfa_ioc_map_port(struct bfa_ioc_s *ioc)
{
	bfa_os_addr_t   rb = ioc->pcidev.pci_bar_kva;
	u32        r32;

	
	if (ioc->pcidev.device_id != BFA_PCI_DEVICE_ID_CT) {
		ioc->port_id = bfa_ioc_pcifn(ioc);
		return;
	}

	
	r32 = bfa_reg_read(rb + FNC_PERS_REG);
	r32 >>= FNC_PERS_FN_SHIFT(bfa_ioc_pcifn(ioc));
	ioc->port_id = (r32 & __F0_PORT_MAP_MK) >> __F0_PORT_MAP_SH;

	bfa_trc(ioc, bfa_ioc_pcifn(ioc));
	bfa_trc(ioc, ioc->port_id);
}






void
bfa_ioc_isr_mode_set(struct bfa_ioc_s *ioc, bfa_boolean_t msix)
{
	bfa_os_addr_t   rb = ioc->pcidev.pci_bar_kva;
	u32        r32, mode;

	r32 = bfa_reg_read(rb + FNC_PERS_REG);
	bfa_trc(ioc, r32);

	mode = (r32 >> FNC_PERS_FN_SHIFT(bfa_ioc_pcifn(ioc))) &
		__F0_INTX_STATUS;

	
	if (!msix && mode)
		return;

	if (msix)
		mode = __F0_INTX_STATUS_MSIX;
	else
		mode = __F0_INTX_STATUS_INTA;

	r32 &= ~(__F0_INTX_STATUS << FNC_PERS_FN_SHIFT(bfa_ioc_pcifn(ioc)));
	r32 |= (mode << FNC_PERS_FN_SHIFT(bfa_ioc_pcifn(ioc)));
	bfa_trc(ioc, r32);

	bfa_reg_write(rb + FNC_PERS_REG, r32);
}

bfa_status_t
bfa_ioc_pll_init(struct bfa_ioc_s *ioc)
{
	bfa_os_addr_t   rb = ioc->pcidev.pci_bar_kva;
	u32        pll_sclk, pll_fclk, r32;

	if (ioc->pcidev.device_id == BFA_PCI_DEVICE_ID_CT) {
		pll_sclk =
			__APP_PLL_312_ENABLE | __APP_PLL_312_LRESETN |
			__APP_PLL_312_RSEL200500 | __APP_PLL_312_P0_1(0U) |
			__APP_PLL_312_JITLMT0_1(3U) |
			__APP_PLL_312_CNTLMT0_1(1U);
		pll_fclk =
			__APP_PLL_425_ENABLE | __APP_PLL_425_LRESETN |
			__APP_PLL_425_RSEL200500 | __APP_PLL_425_P0_1(0U) |
			__APP_PLL_425_JITLMT0_1(3U) |
			__APP_PLL_425_CNTLMT0_1(1U);

		
		if (ioc->fcmode) {
			bfa_reg_write((rb + OP_MODE), 0);
			bfa_reg_write((rb + ETH_MAC_SER_REG),
				      __APP_EMS_CMLCKSEL | __APP_EMS_REFCKBUFEN2
				      | __APP_EMS_CHANNEL_SEL);
		} else {
			ioc->pllinit = BFA_TRUE;
			bfa_reg_write((rb + OP_MODE), __GLOBAL_FCOE_MODE);
			bfa_reg_write((rb + ETH_MAC_SER_REG),
				      __APP_EMS_REFCKBUFEN1);
		}
	} else {
		pll_sclk =
			__APP_PLL_312_ENABLE | __APP_PLL_312_LRESETN |
			__APP_PLL_312_P0_1(3U) | __APP_PLL_312_JITLMT0_1(3U) |
			__APP_PLL_312_CNTLMT0_1(3U);
		pll_fclk =
			__APP_PLL_425_ENABLE | __APP_PLL_425_LRESETN |
			__APP_PLL_425_RSEL200500 | __APP_PLL_425_P0_1(3U) |
			__APP_PLL_425_JITLMT0_1(3U) |
			__APP_PLL_425_CNTLMT0_1(3U);
	}

	bfa_reg_write((rb + BFA_IOC0_STATE_REG), BFI_IOC_UNINIT);
	bfa_reg_write((rb + BFA_IOC1_STATE_REG), BFI_IOC_UNINIT);

	bfa_reg_write((rb + HOSTFN0_INT_MSK), 0xffffffffU);
	bfa_reg_write((rb + HOSTFN1_INT_MSK), 0xffffffffU);
	bfa_reg_write((rb + HOSTFN0_INT_STATUS), 0xffffffffU);
	bfa_reg_write((rb + HOSTFN1_INT_STATUS), 0xffffffffU);
	bfa_reg_write((rb + HOSTFN0_INT_MSK), 0xffffffffU);
	bfa_reg_write((rb + HOSTFN1_INT_MSK), 0xffffffffU);

	bfa_reg_write(ioc->ioc_regs.app_pll_slow_ctl_reg,
		      __APP_PLL_312_LOGIC_SOFT_RESET);
	bfa_reg_write(ioc->ioc_regs.app_pll_slow_ctl_reg,
		      __APP_PLL_312_BYPASS | __APP_PLL_312_LOGIC_SOFT_RESET);
	bfa_reg_write(ioc->ioc_regs.app_pll_fast_ctl_reg,
		      __APP_PLL_425_LOGIC_SOFT_RESET);
	bfa_reg_write(ioc->ioc_regs.app_pll_fast_ctl_reg,
		      __APP_PLL_425_BYPASS | __APP_PLL_425_LOGIC_SOFT_RESET);
	bfa_os_udelay(2);
	bfa_reg_write(ioc->ioc_regs.app_pll_slow_ctl_reg,
		      __APP_PLL_312_LOGIC_SOFT_RESET);
	bfa_reg_write(ioc->ioc_regs.app_pll_fast_ctl_reg,
		      __APP_PLL_425_LOGIC_SOFT_RESET);

	bfa_reg_write(ioc->ioc_regs.app_pll_slow_ctl_reg,
		      pll_sclk | __APP_PLL_312_LOGIC_SOFT_RESET);
	bfa_reg_write(ioc->ioc_regs.app_pll_fast_ctl_reg,
		      pll_fclk | __APP_PLL_425_LOGIC_SOFT_RESET);

	
	bfa_os_udelay(2000);
	bfa_reg_write((rb + HOSTFN0_INT_STATUS), 0xffffffffU);
	bfa_reg_write((rb + HOSTFN1_INT_STATUS), 0xffffffffU);

	bfa_reg_write(ioc->ioc_regs.app_pll_slow_ctl_reg, pll_sclk);
	bfa_reg_write(ioc->ioc_regs.app_pll_fast_ctl_reg, pll_fclk);

	if (ioc->pcidev.device_id == BFA_PCI_DEVICE_ID_CT) {
		bfa_reg_write((rb + MBIST_CTL_REG), __EDRAM_BISTR_START);
		bfa_os_udelay(1000);
		r32 = bfa_reg_read((rb + MBIST_STAT_REG));
		bfa_trc(ioc, r32);
	}

	return BFA_STATUS_OK;
}


void
bfa_ioc_boot(struct bfa_ioc_s *ioc, u32 boot_type, u32 boot_param)
{
	bfa_os_addr_t   rb;

	bfa_ioc_stats(ioc, ioc_boots);

	if (bfa_ioc_pll_init(ioc) != BFA_STATUS_OK)
		return;

	
	rb = ioc->pcidev.pci_bar_kva;
	if (boot_param == BFI_BOOT_TYPE_MEMTEST) {
		bfa_reg_write((rb + BFA_IOC0_STATE_REG), BFI_IOC_MEMTEST);
		bfa_reg_write((rb + BFA_IOC1_STATE_REG), BFI_IOC_MEMTEST);
	} else {
		bfa_reg_write((rb + BFA_IOC0_STATE_REG), BFI_IOC_INITING);
		bfa_reg_write((rb + BFA_IOC1_STATE_REG), BFI_IOC_INITING);
	}

	bfa_ioc_download_fw(ioc, boot_type, boot_param);

	
	ioc->cbfn->reset_cbfn(ioc->bfa);
	bfa_ioc_lpu_start(ioc);
}


void
bfa_ioc_auto_recover(bfa_boolean_t auto_recover)
{
	bfa_auto_recover = BFA_FALSE;
}


bfa_boolean_t
bfa_ioc_is_operational(struct bfa_ioc_s *ioc)
{
	return bfa_fsm_cmp_state(ioc, bfa_ioc_sm_op);
}

void
bfa_ioc_msgget(struct bfa_ioc_s *ioc, void *mbmsg)
{
	u32       *msgp = mbmsg;
	u32        r32;
	int             i;

	
	for (i = 0; i < (sizeof(union bfi_ioc_i2h_msg_u) / sizeof(u32));
	     i++) {
		r32 = bfa_reg_read(ioc->ioc_regs.lpu_mbox +
				   i * sizeof(u32));
		msgp[i] = bfa_os_htonl(r32);
	}

	
	bfa_reg_write(ioc->ioc_regs.lpu_mbox_cmd, 1);
	bfa_reg_read(ioc->ioc_regs.lpu_mbox_cmd);
}

void
bfa_ioc_isr(struct bfa_ioc_s *ioc, struct bfi_mbmsg_s *m)
{
	union bfi_ioc_i2h_msg_u *msg;

	msg = (union bfi_ioc_i2h_msg_u *)m;

	bfa_ioc_stats(ioc, ioc_isrs);

	switch (msg->mh.msg_id) {
	case BFI_IOC_I2H_HBEAT:
		break;

	case BFI_IOC_I2H_READY_EVENT:
		bfa_fsm_send_event(ioc, IOC_E_FWREADY);
		break;

	case BFI_IOC_I2H_ENABLE_REPLY:
		bfa_fsm_send_event(ioc, IOC_E_FWRSP_ENABLE);
		break;

	case BFI_IOC_I2H_DISABLE_REPLY:
		bfa_fsm_send_event(ioc, IOC_E_FWRSP_DISABLE);
		break;

	case BFI_IOC_I2H_GETATTR_REPLY:
		bfa_ioc_getattr_reply(ioc);
		break;

	default:
		bfa_trc(ioc, msg->mh.msg_id);
		bfa_assert(0);
	}
}


void
bfa_ioc_attach(struct bfa_ioc_s *ioc, void *bfa, struct bfa_ioc_cbfn_s *cbfn,
	       struct bfa_timer_mod_s *timer_mod, struct bfa_trc_mod_s *trcmod,
	       struct bfa_aen_s *aen, struct bfa_log_mod_s *logm)
{
	ioc->bfa = bfa;
	ioc->cbfn = cbfn;
	ioc->timer_mod = timer_mod;
	ioc->trcmod = trcmod;
	ioc->aen = aen;
	ioc->logm = logm;
	ioc->fcmode = BFA_FALSE;
	ioc->pllinit = BFA_FALSE;
	ioc->dbg_fwsave_once = BFA_TRUE;

	bfa_ioc_mbox_attach(ioc);
	INIT_LIST_HEAD(&ioc->hb_notify_q);

	bfa_fsm_set_state(ioc, bfa_ioc_sm_reset);
}


void
bfa_ioc_detach(struct bfa_ioc_s *ioc)
{
	bfa_fsm_send_event(ioc, IOC_E_DETACH);
}


void
bfa_ioc_pci_init(struct bfa_ioc_s *ioc, struct bfa_pcidev_s *pcidev,
		 enum bfi_mclass mc)
{
	ioc->ioc_mc = mc;
	ioc->pcidev = *pcidev;
	ioc->ctdev = (ioc->pcidev.device_id == BFA_PCI_DEVICE_ID_CT);
	ioc->cna = ioc->ctdev && !ioc->fcmode;

	bfa_ioc_map_port(ioc);
	bfa_ioc_reg_init(ioc);
}


void
bfa_ioc_mem_claim(struct bfa_ioc_s *ioc, u8 *dm_kva, u64 dm_pa)
{
	
	ioc->attr_dma.kva = dm_kva;
	ioc->attr_dma.pa = dm_pa;
	ioc->attr = (struct bfi_ioc_attr_s *)dm_kva;
}


u32
bfa_ioc_meminfo(void)
{
	return BFA_ROUNDUP(sizeof(struct bfi_ioc_attr_s), BFA_DMA_ALIGN_SZ);
}

void
bfa_ioc_enable(struct bfa_ioc_s *ioc)
{
	bfa_ioc_stats(ioc, ioc_enables);
	ioc->dbg_fwsave_once = BFA_TRUE;

	bfa_fsm_send_event(ioc, IOC_E_ENABLE);
}

void
bfa_ioc_disable(struct bfa_ioc_s *ioc)
{
	bfa_ioc_stats(ioc, ioc_disables);
	bfa_fsm_send_event(ioc, IOC_E_DISABLE);
}


int
bfa_ioc_debug_trcsz(bfa_boolean_t auto_recover)
{
return (auto_recover) ? BFA_DBG_FWTRC_LEN : 0;
}


void
bfa_ioc_debug_memclaim(struct bfa_ioc_s *ioc, void *dbg_fwsave)
{
	bfa_assert(ioc->auto_recover);
	ioc->dbg_fwsave = dbg_fwsave;
	ioc->dbg_fwsave_len = bfa_ioc_debug_trcsz(ioc->auto_recover);
}

u32
bfa_ioc_smem_pgnum(struct bfa_ioc_s *ioc, u32 fmaddr)
{
	return PSS_SMEM_PGNUM(ioc->ioc_regs.smem_pg0, fmaddr);
}

u32
bfa_ioc_smem_pgoff(struct bfa_ioc_s *ioc, u32 fmaddr)
{
	return PSS_SMEM_PGOFF(fmaddr);
}


void
bfa_ioc_mbox_register(struct bfa_ioc_s *ioc, bfa_ioc_mbox_mcfunc_t *mcfuncs)
{
	struct bfa_ioc_mbox_mod_s *mod = &ioc->mbox_mod;
	int             mc;

	for (mc = 0; mc < BFI_MC_MAX; mc++)
		mod->mbhdlr[mc].cbfn = mcfuncs[mc];
}


void
bfa_ioc_mbox_regisr(struct bfa_ioc_s *ioc, enum bfi_mclass mc,
		    bfa_ioc_mbox_mcfunc_t cbfn, void *cbarg)
{
	struct bfa_ioc_mbox_mod_s *mod = &ioc->mbox_mod;

	mod->mbhdlr[mc].cbfn = cbfn;
	mod->mbhdlr[mc].cbarg = cbarg;
}


void
bfa_ioc_mbox_queue(struct bfa_ioc_s *ioc, struct bfa_mbox_cmd_s *cmd)
{
	struct bfa_ioc_mbox_mod_s *mod = &ioc->mbox_mod;
	u32        stat;

	
	if (!list_empty(&mod->cmd_q)) {
		list_add_tail(&cmd->qe, &mod->cmd_q);
		return;
	}

	
	stat = bfa_reg_read(ioc->ioc_regs.hfn_mbox_cmd);
	if (stat) {
		list_add_tail(&cmd->qe, &mod->cmd_q);
		return;
	}

	
	bfa_ioc_mbox_send(ioc, cmd->msg, sizeof(cmd->msg));
}


void
bfa_ioc_mbox_isr(struct bfa_ioc_s *ioc)
{
	struct bfa_ioc_mbox_mod_s *mod = &ioc->mbox_mod;
	struct bfi_mbmsg_s m;
	int             mc;

	bfa_ioc_msgget(ioc, &m);

	
	mc = m.mh.msg_class;
	if (mc == BFI_MC_IOC) {
		bfa_ioc_isr(ioc, &m);
		return;
	}

	if ((mc > BFI_MC_MAX) || (mod->mbhdlr[mc].cbfn == NULL))
		return;

	mod->mbhdlr[mc].cbfn(mod->mbhdlr[mc].cbarg, &m);
}

void
bfa_ioc_error_isr(struct bfa_ioc_s *ioc)
{
	bfa_fsm_send_event(ioc, IOC_E_HWERROR);
}

#ifndef BFA_BIOS_BUILD


bfa_boolean_t
bfa_ioc_is_disabled(struct bfa_ioc_s *ioc)
{
	return (bfa_fsm_cmp_state(ioc, bfa_ioc_sm_disabling)
		|| bfa_fsm_cmp_state(ioc, bfa_ioc_sm_disabled));
}


bfa_boolean_t
bfa_ioc_fw_mismatch(struct bfa_ioc_s *ioc)
{
	return (bfa_fsm_cmp_state(ioc, bfa_ioc_sm_reset)
		|| bfa_fsm_cmp_state(ioc, bfa_ioc_sm_fwcheck)
		|| bfa_fsm_cmp_state(ioc, bfa_ioc_sm_mismatch));
}

#define bfa_ioc_state_disabled(__sm)		\
	(((__sm) == BFI_IOC_UNINIT) ||		\
	 ((__sm) == BFI_IOC_INITING) ||		\
	 ((__sm) == BFI_IOC_HWINIT) ||		\
	 ((__sm) == BFI_IOC_DISABLED) ||	\
	 ((__sm) == BFI_IOC_HBFAIL) ||		\
	 ((__sm) == BFI_IOC_CFG_DISABLED))


bfa_boolean_t
bfa_ioc_adapter_is_disabled(struct bfa_ioc_s *ioc)
{
	u32        ioc_state;
	bfa_os_addr_t   rb = ioc->pcidev.pci_bar_kva;

	if (!bfa_fsm_cmp_state(ioc, bfa_ioc_sm_disabled))
		return BFA_FALSE;

	ioc_state = bfa_reg_read(rb + BFA_IOC0_STATE_REG);
	if (!bfa_ioc_state_disabled(ioc_state))
		return BFA_FALSE;

	ioc_state = bfa_reg_read(rb + BFA_IOC1_STATE_REG);
	if (!bfa_ioc_state_disabled(ioc_state))
		return BFA_FALSE;

	return BFA_TRUE;
}


void
bfa_ioc_hbfail_register(struct bfa_ioc_s *ioc,
			struct bfa_ioc_hbfail_notify_s *notify)
{
	list_add_tail(&notify->qe, &ioc->hb_notify_q);
}

#define BFA_MFG_NAME "Brocade"
void
bfa_ioc_get_adapter_attr(struct bfa_ioc_s *ioc,
			 struct bfa_adapter_attr_s *ad_attr)
{
	struct bfi_ioc_attr_s *ioc_attr;
	char            model[BFA_ADAPTER_MODEL_NAME_LEN];

	ioc_attr = ioc->attr;
	bfa_os_memcpy((void *)&ad_attr->serial_num,
		      (void *)ioc_attr->brcd_serialnum,
		      BFA_ADAPTER_SERIAL_NUM_LEN);

	bfa_os_memcpy(&ad_attr->fw_ver, ioc_attr->fw_version, BFA_VERSION_LEN);
	bfa_os_memcpy(&ad_attr->optrom_ver, ioc_attr->optrom_version,
		      BFA_VERSION_LEN);
	bfa_os_memcpy(&ad_attr->manufacturer, BFA_MFG_NAME,
		      BFA_ADAPTER_MFG_NAME_LEN);
	bfa_os_memcpy(&ad_attr->vpd, &ioc_attr->vpd,
		      sizeof(struct bfa_mfg_vpd_s));

	ad_attr->nports = BFI_ADAPTER_GETP(NPORTS, ioc_attr->adapter_prop);
	ad_attr->max_speed = BFI_ADAPTER_GETP(SPEED, ioc_attr->adapter_prop);

	
	if (BFI_ADAPTER_GETP(SPEED, ioc_attr->adapter_prop) == 10) {
		strcpy(model, "BR-10?0");
		model[5] = '0' + ad_attr->nports;
	} else {
		strcpy(model, "Brocade-??5");
		model[8] =
			'0' + BFI_ADAPTER_GETP(SPEED, ioc_attr->adapter_prop);
		model[9] = '0' + ad_attr->nports;
	}

	if (BFI_ADAPTER_IS_SPECIAL(ioc_attr->adapter_prop))
		ad_attr->prototype = 1;
	else
		ad_attr->prototype = 0;

	bfa_os_memcpy(&ad_attr->model, model, BFA_ADAPTER_MODEL_NAME_LEN);
	bfa_os_memcpy(&ad_attr->model_descr, &ad_attr->model,
		      BFA_ADAPTER_MODEL_NAME_LEN);

	ad_attr->pwwn = bfa_ioc_get_pwwn(ioc);
	ad_attr->mac = bfa_ioc_get_mac(ioc);

	ad_attr->pcie_gen = ioc_attr->pcie_gen;
	ad_attr->pcie_lanes = ioc_attr->pcie_lanes;
	ad_attr->pcie_lanes_orig = ioc_attr->pcie_lanes_orig;
	ad_attr->asic_rev = ioc_attr->asic_rev;
	ad_attr->hw_ver[0] = 'R';
	ad_attr->hw_ver[1] = 'e';
	ad_attr->hw_ver[2] = 'v';
	ad_attr->hw_ver[3] = '-';
	ad_attr->hw_ver[4] = ioc_attr->asic_rev;
	ad_attr->hw_ver[5] = '\0';

	ad_attr->cna_capable = ioc->cna;
}

void
bfa_ioc_get_attr(struct bfa_ioc_s *ioc, struct bfa_ioc_attr_s *ioc_attr)
{
	bfa_os_memset((void *)ioc_attr, 0, sizeof(struct bfa_ioc_attr_s));

	ioc_attr->state = bfa_sm_to_state(ioc_sm_table, ioc->fsm);
	ioc_attr->port_id = ioc->port_id;

	if (!ioc->ctdev)
		ioc_attr->ioc_type = BFA_IOC_TYPE_FC;
	else if (ioc->ioc_mc == BFI_MC_IOCFC)
		ioc_attr->ioc_type = BFA_IOC_TYPE_FCoE;
	else if (ioc->ioc_mc == BFI_MC_LL)
		ioc_attr->ioc_type = BFA_IOC_TYPE_LL;

	bfa_ioc_get_adapter_attr(ioc, &ioc_attr->adapter_attr);

	ioc_attr->pci_attr.device_id = ioc->pcidev.device_id;
	ioc_attr->pci_attr.pcifn = ioc->pcidev.pci_func;
	ioc_attr->pci_attr.chip_rev[0] = 'R';
	ioc_attr->pci_attr.chip_rev[1] = 'e';
	ioc_attr->pci_attr.chip_rev[2] = 'v';
	ioc_attr->pci_attr.chip_rev[3] = '-';
	ioc_attr->pci_attr.chip_rev[4] = ioc_attr->adapter_attr.asic_rev;
	ioc_attr->pci_attr.chip_rev[5] = '\0';
}


wwn_t
bfa_ioc_get_pwwn(struct bfa_ioc_s *ioc)
{
	union {
		wwn_t           wwn;
		u8         byte[sizeof(wwn_t)];
	}
	w;

	w.wwn = ioc->attr->mfg_wwn;

	if (bfa_ioc_portid(ioc) == 1)
		w.byte[7]++;

	return w.wwn;
}

wwn_t
bfa_ioc_get_nwwn(struct bfa_ioc_s *ioc)
{
	union {
		wwn_t           wwn;
		u8         byte[sizeof(wwn_t)];
	}
	w;

	w.wwn = ioc->attr->mfg_wwn;

	if (bfa_ioc_portid(ioc) == 1)
		w.byte[7]++;

	w.byte[0] = 0x20;

	return w.wwn;
}

wwn_t
bfa_ioc_get_wwn_naa5(struct bfa_ioc_s *ioc, u16 inst)
{
	union {
		wwn_t           wwn;
		u8         byte[sizeof(wwn_t)];
	}
	w              , w5;

	bfa_trc(ioc, inst);

	w.wwn = ioc->attr->mfg_wwn;
	w5.byte[0] = 0x50 | w.byte[2] >> 4;
	w5.byte[1] = w.byte[2] << 4 | w.byte[3] >> 4;
	w5.byte[2] = w.byte[3] << 4 | w.byte[4] >> 4;
	w5.byte[3] = w.byte[4] << 4 | w.byte[5] >> 4;
	w5.byte[4] = w.byte[5] << 4 | w.byte[6] >> 4;
	w5.byte[5] = w.byte[6] << 4 | w.byte[7] >> 4;
	w5.byte[6] = w.byte[7] << 4 | (inst & 0x0f00) >> 8;
	w5.byte[7] = (inst & 0xff);

	return w5.wwn;
}

u64
bfa_ioc_get_adid(struct bfa_ioc_s *ioc)
{
	return ioc->attr->mfg_wwn;
}

mac_t
bfa_ioc_get_mac(struct bfa_ioc_s *ioc)
{
	mac_t           mac;

	mac = ioc->attr->mfg_mac;
	mac.mac[MAC_ADDRLEN - 1] += bfa_ioc_pcifn(ioc);

	return mac;
}

void
bfa_ioc_set_fcmode(struct bfa_ioc_s *ioc)
{
	ioc->fcmode = BFA_TRUE;
	ioc->port_id = bfa_ioc_pcifn(ioc);
}

bfa_boolean_t
bfa_ioc_get_fcmode(struct bfa_ioc_s *ioc)
{
	return ioc->fcmode || (ioc->pcidev.device_id != BFA_PCI_DEVICE_ID_CT);
}


bfa_boolean_t
bfa_ioc_intx_claim(struct bfa_ioc_s *ioc)
{
	u32        isr, msk;

	
	if (!ioc->ctdev)
		return BFA_TRUE;

	
	msk = bfa_reg_read(ioc->ioc_regs.shirq_msk_next);
	isr = bfa_reg_read(ioc->ioc_regs.shirq_isr_next);
	return !(isr & ~msk);
}


static void
bfa_ioc_aen_post(struct bfa_ioc_s *ioc, enum bfa_ioc_aen_event event)
{
	union bfa_aen_data_u aen_data;
	struct bfa_log_mod_s *logmod = ioc->logm;
	s32         inst_num = 0;
	struct bfa_ioc_attr_s ioc_attr;

	switch (event) {
	case BFA_IOC_AEN_HBGOOD:
		bfa_log(logmod, BFA_AEN_IOC_HBGOOD, inst_num);
		break;
	case BFA_IOC_AEN_HBFAIL:
		bfa_log(logmod, BFA_AEN_IOC_HBFAIL, inst_num);
		break;
	case BFA_IOC_AEN_ENABLE:
		bfa_log(logmod, BFA_AEN_IOC_ENABLE, inst_num);
		break;
	case BFA_IOC_AEN_DISABLE:
		bfa_log(logmod, BFA_AEN_IOC_DISABLE, inst_num);
		break;
	case BFA_IOC_AEN_FWMISMATCH:
		bfa_log(logmod, BFA_AEN_IOC_FWMISMATCH, inst_num);
		break;
	default:
		break;
	}

	memset(&aen_data.ioc.pwwn, 0, sizeof(aen_data.ioc.pwwn));
	memset(&aen_data.ioc.mac, 0, sizeof(aen_data.ioc.mac));
	bfa_ioc_get_attr(ioc, &ioc_attr);
	switch (ioc_attr.ioc_type) {
	case BFA_IOC_TYPE_FC:
		aen_data.ioc.pwwn = bfa_ioc_get_pwwn(ioc);
		break;
	case BFA_IOC_TYPE_FCoE:
		aen_data.ioc.pwwn = bfa_ioc_get_pwwn(ioc);
		aen_data.ioc.mac = bfa_ioc_get_mac(ioc);
		break;
	case BFA_IOC_TYPE_LL:
		aen_data.ioc.mac = bfa_ioc_get_mac(ioc);
		break;
	default:
		bfa_assert(ioc_attr.ioc_type == BFA_IOC_TYPE_FC);
		break;
	}
	aen_data.ioc.ioc_type = ioc_attr.ioc_type;
}


bfa_status_t
bfa_ioc_debug_fwsave(struct bfa_ioc_s *ioc, void *trcdata, int *trclen)
{
	int             tlen;

	if (ioc->dbg_fwsave_len == 0)
		return BFA_STATUS_ENOFSAVE;

	tlen = *trclen;
	if (tlen > ioc->dbg_fwsave_len)
		tlen = ioc->dbg_fwsave_len;

	bfa_os_memcpy(trcdata, ioc->dbg_fwsave, tlen);
	*trclen = tlen;
	return BFA_STATUS_OK;
}


bfa_status_t
bfa_ioc_debug_fwtrc(struct bfa_ioc_s *ioc, void *trcdata, int *trclen)
{
	u32        pgnum;
	u32        loff = BFA_DBG_FWTRC_OFF(bfa_ioc_portid(ioc));
	int             i, tlen;
	u32       *tbuf = trcdata, r32;

	bfa_trc(ioc, *trclen);

	pgnum = bfa_ioc_smem_pgnum(ioc, loff);
	loff = bfa_ioc_smem_pgoff(ioc, loff);
	bfa_reg_write(ioc->ioc_regs.host_page_num_fn, pgnum);

	tlen = *trclen;
	if (tlen > BFA_DBG_FWTRC_LEN)
		tlen = BFA_DBG_FWTRC_LEN;
	tlen /= sizeof(u32);

	bfa_trc(ioc, tlen);

	for (i = 0; i < tlen; i++) {
		r32 = bfa_mem_read(ioc->ioc_regs.smem_page_start, loff);
		tbuf[i] = bfa_os_ntohl(r32);
		loff += sizeof(u32);

		
		loff = PSS_SMEM_PGOFF(loff);
		if (loff == 0) {
			pgnum++;
			bfa_reg_write(ioc->ioc_regs.host_page_num_fn, pgnum);
		}
	}
	bfa_reg_write(ioc->ioc_regs.host_page_num_fn,
		      bfa_ioc_smem_pgnum(ioc, 0));
	bfa_trc(ioc, pgnum);

	*trclen = tlen * sizeof(u32);
	return BFA_STATUS_OK;
}


static void
bfa_ioc_debug_save(struct bfa_ioc_s *ioc)
{
	int             tlen;

	if (ioc->dbg_fwsave_len) {
		tlen = ioc->dbg_fwsave_len;
		bfa_ioc_debug_fwtrc(ioc, ioc->dbg_fwsave, &tlen);
	}
}


static void
bfa_ioc_recover(struct bfa_ioc_s *ioc)
{
	if (ioc->dbg_fwsave_once) {
		ioc->dbg_fwsave_once = BFA_FALSE;
		bfa_ioc_debug_save(ioc);
	}

	bfa_ioc_stats(ioc, ioc_hbfails);
	bfa_fsm_send_event(ioc, IOC_E_HBFAIL);
}

#else

static void
bfa_ioc_aen_post(struct bfa_ioc_s *ioc, enum bfa_ioc_aen_event event)
{
}

static void
bfa_ioc_recover(struct bfa_ioc_s *ioc)
{
	bfa_assert(0);
}

#endif


