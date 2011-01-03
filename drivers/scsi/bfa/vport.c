



#include <bfa.h>
#include <bfa_svc.h>
#include <fcbuild.h>
#include "fcs_fabric.h"
#include "fcs_lport.h"
#include "fcs_vport.h"
#include "fcs_trcmod.h"
#include "fcs.h"
#include <aen/bfa_aen_lport.h>

BFA_TRC_FILE(FCS, VPORT);

#define __vport_fcs(__vp)       (__vp)->lport.fcs
#define __vport_pwwn(__vp)      (__vp)->lport.port_cfg.pwwn
#define __vport_nwwn(__vp)      (__vp)->lport.port_cfg.nwwn
#define __vport_bfa(__vp)       (__vp)->lport.fcs->bfa
#define __vport_fcid(__vp)      (__vp)->lport.pid
#define __vport_fabric(__vp)    (__vp)->lport.fabric
#define __vport_vfid(__vp)      (__vp)->lport.fabric->vf_id

#define BFA_FCS_VPORT_MAX_RETRIES  5

static void     bfa_fcs_vport_do_fdisc(struct bfa_fcs_vport_s *vport);
static void     bfa_fcs_vport_timeout(void *vport_arg);
static void     bfa_fcs_vport_do_logo(struct bfa_fcs_vport_s *vport);
static void     bfa_fcs_vport_free(struct bfa_fcs_vport_s *vport);




enum bfa_fcs_vport_event {
	BFA_FCS_VPORT_SM_CREATE = 1,	
	BFA_FCS_VPORT_SM_DELETE = 2,	
	BFA_FCS_VPORT_SM_START = 3,	
	BFA_FCS_VPORT_SM_STOP = 4,	
	BFA_FCS_VPORT_SM_ONLINE = 5,	
	BFA_FCS_VPORT_SM_OFFLINE = 6,	
	BFA_FCS_VPORT_SM_FRMSENT = 7,	
	BFA_FCS_VPORT_SM_RSP_OK = 8,	
	BFA_FCS_VPORT_SM_RSP_ERROR = 9,	
	BFA_FCS_VPORT_SM_TIMEOUT = 10,	
	BFA_FCS_VPORT_SM_DELCOMP = 11,	
	BFA_FCS_VPORT_SM_RSP_DUP_WWN = 12,	
	BFA_FCS_VPORT_SM_RSP_FAILED = 13,	
};

static void     bfa_fcs_vport_sm_uninit(struct bfa_fcs_vport_s *vport,
					enum bfa_fcs_vport_event event);
static void     bfa_fcs_vport_sm_created(struct bfa_fcs_vport_s *vport,
					 enum bfa_fcs_vport_event event);
static void     bfa_fcs_vport_sm_offline(struct bfa_fcs_vport_s *vport,
					 enum bfa_fcs_vport_event event);
static void     bfa_fcs_vport_sm_fdisc(struct bfa_fcs_vport_s *vport,
				       enum bfa_fcs_vport_event event);
static void     bfa_fcs_vport_sm_fdisc_retry(struct bfa_fcs_vport_s *vport,
					     enum bfa_fcs_vport_event event);
static void     bfa_fcs_vport_sm_online(struct bfa_fcs_vport_s *vport,
					enum bfa_fcs_vport_event event);
static void     bfa_fcs_vport_sm_deleting(struct bfa_fcs_vport_s *vport,
					  enum bfa_fcs_vport_event event);
static void     bfa_fcs_vport_sm_cleanup(struct bfa_fcs_vport_s *vport,
					 enum bfa_fcs_vport_event event);
static void     bfa_fcs_vport_sm_logo(struct bfa_fcs_vport_s *vport,
				      enum bfa_fcs_vport_event event);
static void     bfa_fcs_vport_sm_error(struct bfa_fcs_vport_s *vport,
				       enum bfa_fcs_vport_event event);

static struct bfa_sm_table_s vport_sm_table[] = {
	{BFA_SM(bfa_fcs_vport_sm_uninit), BFA_FCS_VPORT_UNINIT},
	{BFA_SM(bfa_fcs_vport_sm_created), BFA_FCS_VPORT_CREATED},
	{BFA_SM(bfa_fcs_vport_sm_offline), BFA_FCS_VPORT_OFFLINE},
	{BFA_SM(bfa_fcs_vport_sm_fdisc), BFA_FCS_VPORT_FDISC},
	{BFA_SM(bfa_fcs_vport_sm_fdisc_retry), BFA_FCS_VPORT_FDISC_RETRY},
	{BFA_SM(bfa_fcs_vport_sm_online), BFA_FCS_VPORT_ONLINE},
	{BFA_SM(bfa_fcs_vport_sm_deleting), BFA_FCS_VPORT_DELETING},
	{BFA_SM(bfa_fcs_vport_sm_cleanup), BFA_FCS_VPORT_CLEANUP},
	{BFA_SM(bfa_fcs_vport_sm_logo), BFA_FCS_VPORT_LOGO},
	{BFA_SM(bfa_fcs_vport_sm_error), BFA_FCS_VPORT_ERROR}
};


static void
bfa_fcs_vport_sm_uninit(struct bfa_fcs_vport_s *vport,
			enum bfa_fcs_vport_event event)
{
	bfa_trc(__vport_fcs(vport), __vport_pwwn(vport));
	bfa_trc(__vport_fcs(vport), event);

	switch (event) {
	case BFA_FCS_VPORT_SM_CREATE:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_created);
		bfa_fcs_fabric_addvport(__vport_fabric(vport), vport);
		break;

	default:
		bfa_assert(0);
	}
}


static void
bfa_fcs_vport_sm_created(struct bfa_fcs_vport_s *vport,
			 enum bfa_fcs_vport_event event)
{
	bfa_trc(__vport_fcs(vport), __vport_pwwn(vport));
	bfa_trc(__vport_fcs(vport), event);

	switch (event) {
	case BFA_FCS_VPORT_SM_START:
		if (bfa_fcs_fabric_is_online(__vport_fabric(vport))
		    && bfa_fcs_fabric_npiv_capable(__vport_fabric(vport))) {
			bfa_sm_set_state(vport, bfa_fcs_vport_sm_fdisc);
			bfa_fcs_vport_do_fdisc(vport);
		} else {
			
			vport->vport_stats.fab_no_npiv++;
			bfa_sm_set_state(vport, bfa_fcs_vport_sm_offline);
		}
		break;

	case BFA_FCS_VPORT_SM_DELETE:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_cleanup);
		bfa_fcs_port_delete(&vport->lport);
		break;

	case BFA_FCS_VPORT_SM_ONLINE:
	case BFA_FCS_VPORT_SM_OFFLINE:
		
		break;

	default:
		bfa_assert(0);
	}
}


static void
bfa_fcs_vport_sm_offline(struct bfa_fcs_vport_s *vport,
			 enum bfa_fcs_vport_event event)
{
	bfa_trc(__vport_fcs(vport), __vport_pwwn(vport));
	bfa_trc(__vport_fcs(vport), event);

	switch (event) {
	case BFA_FCS_VPORT_SM_DELETE:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_cleanup);
		bfa_fcs_port_delete(&vport->lport);
		break;

	case BFA_FCS_VPORT_SM_ONLINE:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_fdisc);
		vport->fdisc_retries = 0;
		bfa_fcs_vport_do_fdisc(vport);
		break;

	case BFA_FCS_VPORT_SM_OFFLINE:
		
		break;

	default:
		bfa_assert(0);
	}
}


static void
bfa_fcs_vport_sm_fdisc(struct bfa_fcs_vport_s *vport,
		       enum bfa_fcs_vport_event event)
{
	bfa_trc(__vport_fcs(vport), __vport_pwwn(vport));
	bfa_trc(__vport_fcs(vport), event);

	switch (event) {
	case BFA_FCS_VPORT_SM_DELETE:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_logo);
		bfa_lps_discard(vport->lps);
		bfa_fcs_vport_do_logo(vport);
		break;

	case BFA_FCS_VPORT_SM_OFFLINE:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_offline);
		bfa_lps_discard(vport->lps);
		break;

	case BFA_FCS_VPORT_SM_RSP_OK:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_online);
		bfa_fcs_port_online(&vport->lport);
		break;

	case BFA_FCS_VPORT_SM_RSP_ERROR:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_fdisc_retry);
		bfa_timer_start(__vport_bfa(vport), &vport->timer,
				bfa_fcs_vport_timeout, vport,
				BFA_FCS_RETRY_TIMEOUT);
		break;

	case BFA_FCS_VPORT_SM_RSP_FAILED:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_offline);
		break;

	case BFA_FCS_VPORT_SM_RSP_DUP_WWN:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_error);
		break;

	default:
		bfa_assert(0);
	}
}


static void
bfa_fcs_vport_sm_fdisc_retry(struct bfa_fcs_vport_s *vport,
			     enum bfa_fcs_vport_event event)
{
	bfa_trc(__vport_fcs(vport), __vport_pwwn(vport));
	bfa_trc(__vport_fcs(vport), event);

	switch (event) {
	case BFA_FCS_VPORT_SM_DELETE:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_cleanup);
		bfa_timer_stop(&vport->timer);
		bfa_fcs_port_delete(&vport->lport);
		break;

	case BFA_FCS_VPORT_SM_OFFLINE:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_offline);
		bfa_timer_stop(&vport->timer);
		break;

	case BFA_FCS_VPORT_SM_TIMEOUT:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_fdisc);
		vport->vport_stats.fdisc_retries++;
		vport->fdisc_retries++;
		bfa_fcs_vport_do_fdisc(vport);
		break;

	default:
		bfa_assert(0);
	}
}


static void
bfa_fcs_vport_sm_online(struct bfa_fcs_vport_s *vport,
			enum bfa_fcs_vport_event event)
{
	bfa_trc(__vport_fcs(vport), __vport_pwwn(vport));
	bfa_trc(__vport_fcs(vport), event);

	switch (event) {
	case BFA_FCS_VPORT_SM_DELETE:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_deleting);
		bfa_fcs_port_delete(&vport->lport);
		break;

	case BFA_FCS_VPORT_SM_OFFLINE:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_offline);
		bfa_lps_discard(vport->lps);
		bfa_fcs_port_offline(&vport->lport);
		break;

	default:
		bfa_assert(0);
	}
}


static void
bfa_fcs_vport_sm_deleting(struct bfa_fcs_vport_s *vport,
			  enum bfa_fcs_vport_event event)
{
	bfa_trc(__vport_fcs(vport), __vport_pwwn(vport));
	bfa_trc(__vport_fcs(vport), event);

	switch (event) {
	case BFA_FCS_VPORT_SM_DELETE:
		break;

	case BFA_FCS_VPORT_SM_DELCOMP:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_logo);
		bfa_fcs_vport_do_logo(vport);
		break;

	case BFA_FCS_VPORT_SM_OFFLINE:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_cleanup);
		break;

	default:
		bfa_assert(0);
	}
}


static void
bfa_fcs_vport_sm_error(struct bfa_fcs_vport_s *vport,
		       enum bfa_fcs_vport_event event)
{
	bfa_trc(__vport_fcs(vport), __vport_pwwn(vport));
	bfa_trc(__vport_fcs(vport), event);

	switch (event) {
	case BFA_FCS_VPORT_SM_DELETE:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_uninit);
		bfa_fcs_vport_free(vport);
		break;

	default:
		bfa_trc(__vport_fcs(vport), event);
	}
}


static void
bfa_fcs_vport_sm_cleanup(struct bfa_fcs_vport_s *vport,
			 enum bfa_fcs_vport_event event)
{
	bfa_trc(__vport_fcs(vport), __vport_pwwn(vport));
	bfa_trc(__vport_fcs(vport), event);

	switch (event) {
	case BFA_FCS_VPORT_SM_DELCOMP:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_uninit);
		bfa_fcs_vport_free(vport);
		break;

	case BFA_FCS_VPORT_SM_DELETE:
		break;

	default:
		bfa_assert(0);
	}
}


static void
bfa_fcs_vport_sm_logo(struct bfa_fcs_vport_s *vport,
		      enum bfa_fcs_vport_event event)
{
	bfa_trc(__vport_fcs(vport), __vport_pwwn(vport));
	bfa_trc(__vport_fcs(vport), event);

	switch (event) {
	case BFA_FCS_VPORT_SM_OFFLINE:
		bfa_lps_discard(vport->lps);
		

	case BFA_FCS_VPORT_SM_RSP_OK:
	case BFA_FCS_VPORT_SM_RSP_ERROR:
		bfa_sm_set_state(vport, bfa_fcs_vport_sm_uninit);
		bfa_fcs_vport_free(vport);
		break;

	case BFA_FCS_VPORT_SM_DELETE:
		break;

	default:
		bfa_assert(0);
	}
}






static void
bfa_fcs_vport_aen_post(bfa_fcs_lport_t *port, enum bfa_lport_aen_event event)
{
	union bfa_aen_data_u aen_data;
	struct bfa_log_mod_s *logmod = port->fcs->logm;
	enum bfa_port_role role = port->port_cfg.roles;
	wwn_t           lpwwn = bfa_fcs_port_get_pwwn(port);
	char            lpwwn_ptr[BFA_STRING_32];
	char           *role_str[BFA_PORT_ROLE_FCP_MAX / 2 + 1] =
		{ "Initiator", "Target", "IPFC" };

	wwn2str(lpwwn_ptr, lpwwn);

	bfa_assert(role <= BFA_PORT_ROLE_FCP_MAX);

	switch (event) {
	case BFA_LPORT_AEN_NPIV_DUP_WWN:
		bfa_log(logmod, BFA_AEN_LPORT_NPIV_DUP_WWN, lpwwn_ptr,
			role_str[role / 2]);
		break;
	case BFA_LPORT_AEN_NPIV_FABRIC_MAX:
		bfa_log(logmod, BFA_AEN_LPORT_NPIV_FABRIC_MAX, lpwwn_ptr,
			role_str[role / 2]);
		break;
	case BFA_LPORT_AEN_NPIV_UNKNOWN:
		bfa_log(logmod, BFA_AEN_LPORT_NPIV_UNKNOWN, lpwwn_ptr,
			role_str[role / 2]);
		break;
	default:
		break;
	}

	aen_data.lport.vf_id = port->fabric->vf_id;
	aen_data.lport.roles = role;
	aen_data.lport.ppwwn =
		bfa_fcs_port_get_pwwn(bfa_fcs_get_base_port(port->fcs));
	aen_data.lport.lpwwn = lpwwn;
}


static void
bfa_fcs_vport_do_fdisc(struct bfa_fcs_vport_s *vport)
{
	bfa_lps_fdisc(vport->lps, vport,
		      bfa_pport_get_maxfrsize(__vport_bfa(vport)),
		      __vport_pwwn(vport), __vport_nwwn(vport));
	vport->vport_stats.fdisc_sent++;
}

static void
bfa_fcs_vport_fdisc_rejected(struct bfa_fcs_vport_s *vport)
{
	u8         lsrjt_rsn = bfa_lps_get_lsrjt_rsn(vport->lps);
	u8         lsrjt_expl = bfa_lps_get_lsrjt_expl(vport->lps);

	bfa_trc(__vport_fcs(vport), lsrjt_rsn);
	bfa_trc(__vport_fcs(vport), lsrjt_expl);

	
	switch (bfa_lps_get_lsrjt_expl(vport->lps)) {
	case FC_LS_RJT_EXP_INV_PORT_NAME:	
	case FC_LS_RJT_EXP_INVALID_NPORT_ID:	
		if (vport->fdisc_retries < BFA_FCS_VPORT_MAX_RETRIES)
			bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_RSP_ERROR);
		else {
			bfa_fcs_vport_aen_post(&vport->lport,
					       BFA_LPORT_AEN_NPIV_DUP_WWN);
			bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_RSP_DUP_WWN);
		}
		break;

	case FC_LS_RJT_EXP_INSUFF_RES:
		
		if (vport->fdisc_retries < BFA_FCS_VPORT_MAX_RETRIES)
			bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_RSP_ERROR);
		else {
			bfa_fcs_vport_aen_post(&vport->lport,
					       BFA_LPORT_AEN_NPIV_FABRIC_MAX);
			bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_RSP_FAILED);
		}
		break;

	default:
		if (vport->fdisc_retries == 0)	
			bfa_fcs_vport_aen_post(&vport->lport,
					       BFA_LPORT_AEN_NPIV_UNKNOWN);
		bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_RSP_ERROR);
	}
}


static void
bfa_fcs_vport_do_logo(struct bfa_fcs_vport_s *vport)
{
	bfa_trc(__vport_fcs(vport), __vport_pwwn(vport));

	vport->vport_stats.logo_sent++;
	bfa_lps_fdisclogo(vport->lps);
}


static void
bfa_fcs_vport_timeout(void *vport_arg)
{
	struct bfa_fcs_vport_s *vport = (struct bfa_fcs_vport_s *)vport_arg;

	vport->vport_stats.fdisc_timeouts++;
	bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_TIMEOUT);
}

static void
bfa_fcs_vport_free(struct bfa_fcs_vport_s *vport)
{
	bfa_fcs_fabric_delvport(__vport_fabric(vport), vport);
	bfa_fcb_vport_delete(vport->vport_drv);
	bfa_lps_delete(vport->lps);
}






void
bfa_fcs_vport_online(struct bfa_fcs_vport_s *vport)
{
	vport->vport_stats.fab_online++;
	bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_ONLINE);
}


void
bfa_fcs_vport_offline(struct bfa_fcs_vport_s *vport)
{
	vport->vport_stats.fab_offline++;
	bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_OFFLINE);
}


void
bfa_fcs_vport_cleanup(struct bfa_fcs_vport_s *vport)
{
	vport->vport_stats.fab_cleanup++;
}


void
bfa_fcs_vport_delete_comp(struct bfa_fcs_vport_s *vport)
{
	bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_DELCOMP);
}


void
bfa_fcs_vport_modinit(struct bfa_fcs_s *fcs)
{
}


void
bfa_fcs_vport_modexit(struct bfa_fcs_s *fcs)
{
	bfa_fcs_modexit_comp(fcs);
}

u32
bfa_fcs_vport_get_max(struct bfa_fcs_s *fcs)
{
	struct bfa_ioc_attr_s ioc_attr;

	bfa_get_attr(fcs->bfa, &ioc_attr);

	if (ioc_attr.pci_attr.device_id == BFA_PCI_DEVICE_ID_CT)
		return (BFA_FCS_MAX_VPORTS_SUPP_CT);
	else
		return (BFA_FCS_MAX_VPORTS_SUPP_CB);
}






bfa_status_t
bfa_fcs_vport_create(struct bfa_fcs_vport_s *vport, struct bfa_fcs_s *fcs,
		     u16 vf_id, struct bfa_port_cfg_s *vport_cfg,
		     struct bfad_vport_s *vport_drv)
{
	if (vport_cfg->pwwn == 0)
		return (BFA_STATUS_INVALID_WWN);

	if (bfa_fcs_port_get_pwwn(&fcs->fabric.bport) == vport_cfg->pwwn)
		return BFA_STATUS_VPORT_WWN_BP;

	if (bfa_fcs_vport_lookup(fcs, vf_id, vport_cfg->pwwn) != NULL)
		return BFA_STATUS_VPORT_EXISTS;

	if (bfa_fcs_fabric_vport_count(&fcs->fabric) ==
	    bfa_fcs_vport_get_max(fcs))
		return BFA_STATUS_VPORT_MAX;

	vport->lps = bfa_lps_alloc(fcs->bfa);
	if (!vport->lps)
		return BFA_STATUS_VPORT_MAX;

	vport->vport_drv = vport_drv;
	bfa_sm_set_state(vport, bfa_fcs_vport_sm_uninit);

	bfa_fcs_lport_init(&vport->lport, fcs, vf_id, vport_cfg, vport);

	bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_CREATE);

	return BFA_STATUS_OK;
}


bfa_status_t
bfa_fcs_vport_start(struct bfa_fcs_vport_s *vport)
{
	bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_START);

	return BFA_STATUS_OK;
}


bfa_status_t
bfa_fcs_vport_stop(struct bfa_fcs_vport_s *vport)
{
	bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_STOP);

	return BFA_STATUS_OK;
}


bfa_status_t
bfa_fcs_vport_delete(struct bfa_fcs_vport_s *vport)
{
	bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_DELETE);

	return BFA_STATUS_OK;
}


void
bfa_fcs_vport_get_attr(struct bfa_fcs_vport_s *vport,
		       struct bfa_vport_attr_s *attr)
{
	if (vport == NULL || attr == NULL)
		return;

	bfa_os_memset(attr, 0, sizeof(struct bfa_vport_attr_s));

	bfa_fcs_port_get_attr(&vport->lport, &attr->port_attr);
	attr->vport_state = bfa_sm_to_state(vport_sm_table, vport->sm);
}


void
bfa_fcs_vport_get_stats(struct bfa_fcs_vport_s *vport,
			struct bfa_vport_stats_s *stats)
{
	*stats = vport->vport_stats;
}


void
bfa_fcs_vport_clr_stats(struct bfa_fcs_vport_s *vport)
{
	bfa_os_memset(&vport->vport_stats, 0, sizeof(struct bfa_vport_stats_s));
}


struct bfa_fcs_vport_s *
bfa_fcs_vport_lookup(struct bfa_fcs_s *fcs, u16 vf_id, wwn_t vpwwn)
{
	struct bfa_fcs_vport_s *vport;
	struct bfa_fcs_fabric_s *fabric;

	bfa_trc(fcs, vf_id);
	bfa_trc(fcs, vpwwn);

	fabric = bfa_fcs_vf_lookup(fcs, vf_id);
	if (!fabric) {
		bfa_trc(fcs, vf_id);
		return NULL;
	}

	vport = bfa_fcs_fabric_vport_lookup(fabric, vpwwn);
	return vport;
}


void
bfa_cb_lps_fdisc_comp(void *bfad, void *uarg, bfa_status_t status)
{
	struct bfa_fcs_vport_s *vport = uarg;

	bfa_trc(__vport_fcs(vport), __vport_pwwn(vport));
	bfa_trc(__vport_fcs(vport), status);

	switch (status) {
	case BFA_STATUS_OK:
		
		__vport_fcid(vport) = bfa_lps_get_pid(vport->lps);
		vport->vport_stats.fdisc_accepts++;
		bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_RSP_OK);
		break;

	case BFA_STATUS_INVALID_MAC:
		
		vport->vport_stats.fdisc_acc_bad++;
		bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_RSP_ERROR);

		break;

	case BFA_STATUS_EPROTOCOL:
		switch (bfa_lps_get_extstatus(vport->lps)) {
		case BFA_EPROTO_BAD_ACCEPT:
			vport->vport_stats.fdisc_acc_bad++;
			break;

		case BFA_EPROTO_UNKNOWN_RSP:
			vport->vport_stats.fdisc_unknown_rsp++;
			break;

		default:
			break;
		}

		bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_RSP_ERROR);
		break;

	case BFA_STATUS_FABRIC_RJT:
		vport->vport_stats.fdisc_rejects++;
		bfa_fcs_vport_fdisc_rejected(vport);
		break;

	default:
		vport->vport_stats.fdisc_rsp_err++;
		bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_RSP_ERROR);
	}
}


void
bfa_cb_lps_fdisclogo_comp(void *bfad, void *uarg)
{
	struct bfa_fcs_vport_s *vport = uarg;
	bfa_sm_send_event(vport, BFA_FCS_VPORT_SM_RSP_OK);
}


