

#include <bfa.h>
#include <bfa_cb_ioim_macros.h>

BFA_TRC_FILE(HAL, TSKIM);


#define bfa_tskim_qcomp(__tskim, __cbfn) do {			     \
	bfa_cb_queue((__tskim)->bfa, &(__tskim)->hcb_qe, __cbfn, (__tskim)); \
	bfa_tskim_notify_comp(__tskim);      \
} while (0)

#define bfa_tskim_notify_comp(__tskim) do {				     \
	if ((__tskim)->notify)					     	     \
		bfa_itnim_tskdone((__tskim)->itnim);      \
} while (0)


static void     __bfa_cb_tskim_done(void *cbarg, bfa_boolean_t complete);
static void     __bfa_cb_tskim_failed(void *cbarg, bfa_boolean_t complete);
static bfa_boolean_t bfa_tskim_match_scope(struct bfa_tskim_s *tskim,
					       lun_t lun);
static void     bfa_tskim_gather_ios(struct bfa_tskim_s *tskim);
static void     bfa_tskim_cleanp_comp(void *tskim_cbarg);
static void     bfa_tskim_cleanup_ios(struct bfa_tskim_s *tskim);
static bfa_boolean_t bfa_tskim_send(struct bfa_tskim_s *tskim);
static bfa_boolean_t bfa_tskim_send_abort(struct bfa_tskim_s *tskim);
static void     bfa_tskim_iocdisable_ios(struct bfa_tskim_s *tskim);



enum bfa_tskim_event {
	BFA_TSKIM_SM_START        = 1,  
	BFA_TSKIM_SM_DONE         = 2,  
	BFA_TSKIM_SM_QRESUME      = 3,  
	BFA_TSKIM_SM_HWFAIL       = 5,  
	BFA_TSKIM_SM_HCB          = 6,  
	BFA_TSKIM_SM_IOS_DONE     = 7,  
	BFA_TSKIM_SM_CLEANUP      = 8,  
	BFA_TSKIM_SM_CLEANUP_DONE = 9,  
};

static void     bfa_tskim_sm_uninit(struct bfa_tskim_s *tskim,
					enum bfa_tskim_event event);
static void     bfa_tskim_sm_active(struct bfa_tskim_s *tskim,
					enum bfa_tskim_event event);
static void     bfa_tskim_sm_cleanup(struct bfa_tskim_s *tskim,
					 enum bfa_tskim_event event);
static void     bfa_tskim_sm_iocleanup(struct bfa_tskim_s *tskim,
					 enum bfa_tskim_event event);
static void     bfa_tskim_sm_qfull(struct bfa_tskim_s *tskim,
				       enum bfa_tskim_event event);
static void     bfa_tskim_sm_cleanup_qfull(struct bfa_tskim_s *tskim,
				       enum bfa_tskim_event event);
static void     bfa_tskim_sm_hcb(struct bfa_tskim_s *tskim,
				     enum bfa_tskim_event event);


static void
bfa_tskim_sm_uninit(struct bfa_tskim_s *tskim, enum bfa_tskim_event event)
{
	bfa_trc(tskim->bfa, event);

	switch (event) {
	case BFA_TSKIM_SM_START:
		bfa_sm_set_state(tskim, bfa_tskim_sm_active);
		bfa_tskim_gather_ios(tskim);

		
		if (!bfa_itnim_is_online(tskim->itnim)) {
			bfa_sm_set_state(tskim, bfa_tskim_sm_iocleanup);
			tskim->tsk_status = BFI_TSKIM_STS_OK;
			bfa_tskim_cleanup_ios(tskim);
			return;
		}

		if (!bfa_tskim_send(tskim)) {
			bfa_sm_set_state(tskim, bfa_tskim_sm_qfull);
			bfa_reqq_wait(tskim->bfa, tskim->itnim->reqq,
					  &tskim->reqq_wait);
		}
		break;

	default:
		bfa_assert(0);
	}
}


static void
bfa_tskim_sm_active(struct bfa_tskim_s *tskim, enum bfa_tskim_event event)
{
	bfa_trc(tskim->bfa, event);

	switch (event) {
	case BFA_TSKIM_SM_DONE:
		bfa_sm_set_state(tskim, bfa_tskim_sm_iocleanup);
		bfa_tskim_cleanup_ios(tskim);
		break;

	case BFA_TSKIM_SM_CLEANUP:
		bfa_sm_set_state(tskim, bfa_tskim_sm_cleanup);
		if (!bfa_tskim_send_abort(tskim)) {
			bfa_sm_set_state(tskim, bfa_tskim_sm_cleanup_qfull);
			bfa_reqq_wait(tskim->bfa, tskim->itnim->reqq,
				&tskim->reqq_wait);
		}
		break;

	case BFA_TSKIM_SM_HWFAIL:
		bfa_sm_set_state(tskim, bfa_tskim_sm_hcb);
		bfa_tskim_iocdisable_ios(tskim);
		bfa_tskim_qcomp(tskim, __bfa_cb_tskim_failed);
		break;

	default:
		bfa_assert(0);
	}
}


static void
bfa_tskim_sm_cleanup(struct bfa_tskim_s *tskim, enum bfa_tskim_event event)
{
	bfa_trc(tskim->bfa, event);

	switch (event) {
	case BFA_TSKIM_SM_DONE:
		
		break;

	case BFA_TSKIM_SM_CLEANUP_DONE:
		bfa_sm_set_state(tskim, bfa_tskim_sm_iocleanup);
		bfa_tskim_cleanup_ios(tskim);
		break;

	case BFA_TSKIM_SM_HWFAIL:
		bfa_sm_set_state(tskim, bfa_tskim_sm_hcb);
		bfa_tskim_iocdisable_ios(tskim);
		bfa_tskim_qcomp(tskim, __bfa_cb_tskim_failed);
		break;

	default:
		bfa_assert(0);
	}
}

static void
bfa_tskim_sm_iocleanup(struct bfa_tskim_s *tskim, enum bfa_tskim_event event)
{
	bfa_trc(tskim->bfa, event);

	switch (event) {
	case BFA_TSKIM_SM_IOS_DONE:
		bfa_sm_set_state(tskim, bfa_tskim_sm_hcb);
		bfa_tskim_qcomp(tskim, __bfa_cb_tskim_done);
		break;

	case BFA_TSKIM_SM_CLEANUP:
		
		break;

	case BFA_TSKIM_SM_HWFAIL:
		bfa_sm_set_state(tskim, bfa_tskim_sm_hcb);
		bfa_tskim_iocdisable_ios(tskim);
		bfa_tskim_qcomp(tskim, __bfa_cb_tskim_failed);
		break;

	default:
		bfa_assert(0);
	}
}


static void
bfa_tskim_sm_qfull(struct bfa_tskim_s *tskim, enum bfa_tskim_event event)
{
	bfa_trc(tskim->bfa, event);

	switch (event) {
	case BFA_TSKIM_SM_QRESUME:
		bfa_sm_set_state(tskim, bfa_tskim_sm_active);
		bfa_tskim_send(tskim);
		break;

	case BFA_TSKIM_SM_CLEANUP:
		
		bfa_sm_set_state(tskim, bfa_tskim_sm_iocleanup);
		bfa_reqq_wcancel(&tskim->reqq_wait);
		bfa_tskim_cleanup_ios(tskim);
		break;

	case BFA_TSKIM_SM_HWFAIL:
		bfa_sm_set_state(tskim, bfa_tskim_sm_hcb);
		bfa_reqq_wcancel(&tskim->reqq_wait);
		bfa_tskim_iocdisable_ios(tskim);
		bfa_tskim_qcomp(tskim, __bfa_cb_tskim_failed);
		break;

	default:
		bfa_assert(0);
	}
}


static void
bfa_tskim_sm_cleanup_qfull(struct bfa_tskim_s *tskim,
		enum bfa_tskim_event event)
{
	bfa_trc(tskim->bfa, event);

	switch (event) {
	case BFA_TSKIM_SM_DONE:
		bfa_reqq_wcancel(&tskim->reqq_wait);
		

	case BFA_TSKIM_SM_QRESUME:
		bfa_sm_set_state(tskim, bfa_tskim_sm_cleanup);
		bfa_tskim_send_abort(tskim);
		break;

	case BFA_TSKIM_SM_HWFAIL:
		bfa_sm_set_state(tskim, bfa_tskim_sm_hcb);
		bfa_reqq_wcancel(&tskim->reqq_wait);
		bfa_tskim_iocdisable_ios(tskim);
		bfa_tskim_qcomp(tskim, __bfa_cb_tskim_failed);
		break;

	default:
		bfa_assert(0);
	}
}


static void
bfa_tskim_sm_hcb(struct bfa_tskim_s *tskim, enum bfa_tskim_event event)
{
	bfa_trc(tskim->bfa, event);

	switch (event) {
	case BFA_TSKIM_SM_HCB:
		bfa_sm_set_state(tskim, bfa_tskim_sm_uninit);
		bfa_tskim_free(tskim);
		break;

	case BFA_TSKIM_SM_CLEANUP:
		bfa_tskim_notify_comp(tskim);
		break;

	case BFA_TSKIM_SM_HWFAIL:
		break;

	default:
		bfa_assert(0);
	}
}





static void
__bfa_cb_tskim_done(void *cbarg, bfa_boolean_t complete)
{
	struct bfa_tskim_s *tskim = cbarg;

	if (!complete) {
		bfa_sm_send_event(tskim, BFA_TSKIM_SM_HCB);
		return;
	}

	bfa_stats(tskim->itnim, tm_success);
	bfa_cb_tskim_done(tskim->bfa->bfad, tskim->dtsk, tskim->tsk_status);
}

static void
__bfa_cb_tskim_failed(void *cbarg, bfa_boolean_t complete)
{
	struct bfa_tskim_s *tskim = cbarg;

	if (!complete) {
		bfa_sm_send_event(tskim, BFA_TSKIM_SM_HCB);
		return;
	}

	bfa_stats(tskim->itnim, tm_failures);
	bfa_cb_tskim_done(tskim->bfa->bfad, tskim->dtsk,
			   BFI_TSKIM_STS_FAILED);
}

static          bfa_boolean_t
bfa_tskim_match_scope(struct bfa_tskim_s *tskim, lun_t lun)
{
	switch (tskim->tm_cmnd) {
	case FCP_TM_TARGET_RESET:
		return BFA_TRUE;

	case FCP_TM_ABORT_TASK_SET:
	case FCP_TM_CLEAR_TASK_SET:
	case FCP_TM_LUN_RESET:
	case FCP_TM_CLEAR_ACA:
		return (tskim->lun == lun);

	default:
		bfa_assert(0);
	}

	return BFA_FALSE;
}


static void
bfa_tskim_gather_ios(struct bfa_tskim_s *tskim)
{
	struct bfa_itnim_s *itnim = tskim->itnim;
	struct bfa_ioim_s *ioim;
	struct list_head        *qe, *qen;

	INIT_LIST_HEAD(&tskim->io_q);

	
	list_for_each_safe(qe, qen, &itnim->io_q) {
		ioim = (struct bfa_ioim_s *) qe;
		if (bfa_tskim_match_scope
		    (tskim, bfa_cb_ioim_get_lun(ioim->dio))) {
			list_del(&ioim->qe);
			list_add_tail(&ioim->qe, &tskim->io_q);
		}
	}

	
	list_for_each_safe(qe, qen, &itnim->pending_q) {
		ioim = (struct bfa_ioim_s *) qe;
		if (bfa_tskim_match_scope
		    (tskim, bfa_cb_ioim_get_lun(ioim->dio))) {
			list_del(&ioim->qe);
			list_add_tail(&ioim->qe, &ioim->fcpim->ioim_comp_q);
			bfa_ioim_tov(ioim);
		}
	}
}


static void
bfa_tskim_cleanp_comp(void *tskim_cbarg)
{
	struct bfa_tskim_s *tskim = tskim_cbarg;

	bfa_stats(tskim->itnim, tm_io_comps);
	bfa_sm_send_event(tskim, BFA_TSKIM_SM_IOS_DONE);
}


static void
bfa_tskim_cleanup_ios(struct bfa_tskim_s *tskim)
{
	struct bfa_ioim_s *ioim;
	struct list_head        *qe, *qen;

	bfa_wc_init(&tskim->wc, bfa_tskim_cleanp_comp, tskim);

	list_for_each_safe(qe, qen, &tskim->io_q) {
		ioim = (struct bfa_ioim_s *) qe;
		bfa_wc_up(&tskim->wc);
		bfa_ioim_cleanup_tm(ioim, tskim);
	}

	bfa_wc_wait(&tskim->wc);
}


static bfa_boolean_t
bfa_tskim_send(struct bfa_tskim_s *tskim)
{
	struct bfa_itnim_s *itnim = tskim->itnim;
	struct bfi_tskim_req_s *m;

	
	m = bfa_reqq_next(tskim->bfa, itnim->reqq);
	if (!m)
		return BFA_FALSE;

	
	bfi_h2i_set(m->mh, BFI_MC_TSKIM, BFI_TSKIM_H2I_TM_REQ,
			bfa_lpuid(tskim->bfa));

	m->tsk_tag = bfa_os_htons(tskim->tsk_tag);
	m->itn_fhdl = tskim->itnim->rport->fw_handle;
	m->t_secs = tskim->tsecs;
	m->lun = tskim->lun;
	m->tm_flags = tskim->tm_cmnd;

	
	bfa_reqq_produce(tskim->bfa, itnim->reqq);
	return BFA_TRUE;
}


static bfa_boolean_t
bfa_tskim_send_abort(struct bfa_tskim_s *tskim)
{
	struct bfa_itnim_s             *itnim = tskim->itnim;
	struct bfi_tskim_abortreq_s    *m;

	
	m = bfa_reqq_next(tskim->bfa, itnim->reqq);
	if (!m)
		return BFA_FALSE;

	
	bfi_h2i_set(m->mh, BFI_MC_TSKIM, BFI_TSKIM_H2I_ABORT_REQ,
			bfa_lpuid(tskim->bfa));

	m->tsk_tag  = bfa_os_htons(tskim->tsk_tag);

	
	bfa_reqq_produce(tskim->bfa, itnim->reqq);
	return BFA_TRUE;
}


static void
bfa_tskim_qresume(void *cbarg)
{
	struct bfa_tskim_s *tskim = cbarg;

	bfa_fcpim_stats(tskim->fcpim, qresumes);
	bfa_stats(tskim->itnim, tm_qresumes);
	bfa_sm_send_event(tskim, BFA_TSKIM_SM_QRESUME);
}


static void
bfa_tskim_iocdisable_ios(struct bfa_tskim_s *tskim)
{
	struct bfa_ioim_s *ioim;
	struct list_head        *qe, *qen;

	list_for_each_safe(qe, qen, &tskim->io_q) {
		ioim = (struct bfa_ioim_s *) qe;
		bfa_ioim_iocdisable(ioim);
	}
}






void
bfa_tskim_iodone(struct bfa_tskim_s *tskim)
{
	bfa_wc_down(&tskim->wc);
}


void
bfa_tskim_iocdisable(struct bfa_tskim_s *tskim)
{
	tskim->notify = BFA_FALSE;
	bfa_stats(tskim->itnim, tm_iocdowns);
	bfa_sm_send_event(tskim, BFA_TSKIM_SM_HWFAIL);
}


void
bfa_tskim_cleanup(struct bfa_tskim_s *tskim)
{
	tskim->notify = BFA_TRUE;
	bfa_stats(tskim->itnim, tm_cleanups);
	bfa_sm_send_event(tskim, BFA_TSKIM_SM_CLEANUP);
}


void
bfa_tskim_attach(struct bfa_fcpim_mod_s *fcpim, struct bfa_meminfo_s *minfo)
{
	struct bfa_tskim_s *tskim;
	u16        i;

	INIT_LIST_HEAD(&fcpim->tskim_free_q);

	tskim = (struct bfa_tskim_s *) bfa_meminfo_kva(minfo);
	fcpim->tskim_arr = tskim;

	for (i = 0; i < fcpim->num_tskim_reqs; i++, tskim++) {
		
		bfa_os_memset(tskim, 0, sizeof(struct bfa_tskim_s));
		tskim->tsk_tag = i;
		tskim->bfa     = fcpim->bfa;
		tskim->fcpim   = fcpim;
		tskim->notify  = BFA_FALSE;
		bfa_reqq_winit(&tskim->reqq_wait, bfa_tskim_qresume,
				   tskim);
		bfa_sm_set_state(tskim, bfa_tskim_sm_uninit);

		list_add_tail(&tskim->qe, &fcpim->tskim_free_q);
	}

	bfa_meminfo_kva(minfo) = (u8 *) tskim;
}

void
bfa_tskim_detach(struct bfa_fcpim_mod_s *fcpim)
{
    
}

void
bfa_tskim_isr(struct bfa_s *bfa, struct bfi_msg_s *m)
{
	struct bfa_fcpim_mod_s *fcpim = BFA_FCPIM_MOD(bfa);
	struct bfi_tskim_rsp_s *rsp = (struct bfi_tskim_rsp_s *) m;
	struct bfa_tskim_s *tskim;
	u16        tsk_tag = bfa_os_ntohs(rsp->tsk_tag);

	tskim = BFA_TSKIM_FROM_TAG(fcpim, tsk_tag);
	bfa_assert(tskim->tsk_tag == tsk_tag);

	tskim->tsk_status = rsp->tsk_status;

	
	if (rsp->tsk_status == BFI_TSKIM_STS_ABORTED) {
		bfa_stats(tskim->itnim, tm_cleanup_comps);
		bfa_sm_send_event(tskim, BFA_TSKIM_SM_CLEANUP_DONE);
	} else {
		bfa_stats(tskim->itnim, tm_fw_rsps);
		bfa_sm_send_event(tskim, BFA_TSKIM_SM_DONE);
	}
}






struct bfa_tskim_s *
bfa_tskim_alloc(struct bfa_s *bfa, struct bfad_tskim_s *dtsk)
{
	struct bfa_fcpim_mod_s *fcpim = BFA_FCPIM_MOD(bfa);
	struct bfa_tskim_s *tskim;

	bfa_q_deq(&fcpim->tskim_free_q, &tskim);

	if (!tskim)
		bfa_fcpim_stats(fcpim, no_tskims);
	else
		tskim->dtsk = dtsk;

	return tskim;
}

void
bfa_tskim_free(struct bfa_tskim_s *tskim)
{
	bfa_assert(bfa_q_is_on_q_func(&tskim->itnim->tsk_q, &tskim->qe));
	list_del(&tskim->qe);
	list_add_tail(&tskim->qe, &tskim->fcpim->tskim_free_q);
}


void
bfa_tskim_start(struct bfa_tskim_s *tskim, struct bfa_itnim_s *itnim, lun_t lun,
		    enum fcp_tm_cmnd tm_cmnd, u8 tsecs)
{
	tskim->itnim   = itnim;
	tskim->lun     = lun;
	tskim->tm_cmnd = tm_cmnd;
	tskim->tsecs   = tsecs;
	tskim->notify  = BFA_FALSE;
	bfa_stats(itnim, tm_cmnds);

	list_add_tail(&tskim->qe, &itnim->tsk_q);
	bfa_sm_send_event(tskim, BFA_TSKIM_SM_START);
}


