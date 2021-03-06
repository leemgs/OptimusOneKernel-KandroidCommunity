



#include <fcs/bfa_fcs.h>
#include <fcs/bfa_fcs_lport.h>
#include <fcs/bfa_fcs_rport.h>
#include <fcb/bfa_fcb_port.h>
#include <bfa_svc.h>
#include <log/bfa_log_fcs.h>
#include "fcs.h"
#include "fcs_lport.h"
#include "fcs_vport.h"
#include "fcs_rport.h"
#include "fcs_fcxp.h"
#include "fcs_trcmod.h"
#include "lport_priv.h"
#include <aen/bfa_aen_lport.h>

BFA_TRC_FILE(FCS, PORT);



static void     bfa_fcs_port_aen_post(struct bfa_fcs_port_s *port,
				      enum bfa_lport_aen_event event);
static void     bfa_fcs_port_send_ls_rjt(struct bfa_fcs_port_s *port,
			struct fchs_s *rx_fchs, u8 reason_code,
			u8 reason_code_expl);
static void     bfa_fcs_port_plogi(struct bfa_fcs_port_s *port,
			struct fchs_s *rx_fchs,
			struct fc_logi_s *plogi);
static void     bfa_fcs_port_online_actions(struct bfa_fcs_port_s *port);
static void     bfa_fcs_port_offline_actions(struct bfa_fcs_port_s *port);
static void     bfa_fcs_port_unknown_init(struct bfa_fcs_port_s *port);
static void     bfa_fcs_port_unknown_online(struct bfa_fcs_port_s *port);
static void     bfa_fcs_port_unknown_offline(struct bfa_fcs_port_s *port);
static void     bfa_fcs_port_deleted(struct bfa_fcs_port_s *port);
static void     bfa_fcs_port_echo(struct bfa_fcs_port_s *port,
			struct fchs_s *rx_fchs,
			struct fc_echo_s *echo, u16 len);
static void     bfa_fcs_port_rnid(struct bfa_fcs_port_s *port,
			struct fchs_s *rx_fchs,
			struct fc_rnid_cmd_s *rnid, u16 len);
static void     bfa_fs_port_get_gen_topo_data(struct bfa_fcs_port_s *port,
			struct fc_rnid_general_topology_data_s *gen_topo_data);

static struct {
	void            (*init) (struct bfa_fcs_port_s *port);
	void            (*online) (struct bfa_fcs_port_s *port);
	void            (*offline) (struct bfa_fcs_port_s *port);
} __port_action[] = {
	{
	bfa_fcs_port_unknown_init, bfa_fcs_port_unknown_online,
			bfa_fcs_port_unknown_offline}, {
	bfa_fcs_port_fab_init, bfa_fcs_port_fab_online,
			bfa_fcs_port_fab_offline}, {
	bfa_fcs_port_loop_init, bfa_fcs_port_loop_online,
			bfa_fcs_port_loop_offline}, {
bfa_fcs_port_n2n_init, bfa_fcs_port_n2n_online,
			bfa_fcs_port_n2n_offline},};



enum bfa_fcs_port_event {
	BFA_FCS_PORT_SM_CREATE = 1,
	BFA_FCS_PORT_SM_ONLINE = 2,
	BFA_FCS_PORT_SM_OFFLINE = 3,
	BFA_FCS_PORT_SM_DELETE = 4,
	BFA_FCS_PORT_SM_DELRPORT = 5,
};

static void     bfa_fcs_port_sm_uninit(struct bfa_fcs_port_s *port,
				       enum bfa_fcs_port_event event);
static void     bfa_fcs_port_sm_init(struct bfa_fcs_port_s *port,
				     enum bfa_fcs_port_event event);
static void     bfa_fcs_port_sm_online(struct bfa_fcs_port_s *port,
				       enum bfa_fcs_port_event event);
static void     bfa_fcs_port_sm_offline(struct bfa_fcs_port_s *port,
					enum bfa_fcs_port_event event);
static void     bfa_fcs_port_sm_deleting(struct bfa_fcs_port_s *port,
					 enum bfa_fcs_port_event event);

static void
bfa_fcs_port_sm_uninit(struct bfa_fcs_port_s *port,
			enum bfa_fcs_port_event event)
{
	bfa_trc(port->fcs, port->port_cfg.pwwn);
	bfa_trc(port->fcs, event);

	switch (event) {
	case BFA_FCS_PORT_SM_CREATE:
		bfa_sm_set_state(port, bfa_fcs_port_sm_init);
		break;

	default:
		bfa_assert(0);
	}
}

static void
bfa_fcs_port_sm_init(struct bfa_fcs_port_s *port, enum bfa_fcs_port_event event)
{
	bfa_trc(port->fcs, port->port_cfg.pwwn);
	bfa_trc(port->fcs, event);

	switch (event) {
	case BFA_FCS_PORT_SM_ONLINE:
		bfa_sm_set_state(port, bfa_fcs_port_sm_online);
		bfa_fcs_port_online_actions(port);
		break;

	case BFA_FCS_PORT_SM_DELETE:
		bfa_sm_set_state(port, bfa_fcs_port_sm_uninit);
		bfa_fcs_port_deleted(port);
		break;

	default:
		bfa_assert(0);
	}
}

static void
bfa_fcs_port_sm_online(struct bfa_fcs_port_s *port,
			enum bfa_fcs_port_event event)
{
	struct bfa_fcs_rport_s *rport;
	struct list_head *qe, *qen;

	bfa_trc(port->fcs, port->port_cfg.pwwn);
	bfa_trc(port->fcs, event);

	switch (event) {
	case BFA_FCS_PORT_SM_OFFLINE:
		bfa_sm_set_state(port, bfa_fcs_port_sm_offline);
		bfa_fcs_port_offline_actions(port);
		break;

	case BFA_FCS_PORT_SM_DELETE:

		__port_action[port->fabric->fab_type].offline(port);

		if (port->num_rports == 0) {
			bfa_sm_set_state(port, bfa_fcs_port_sm_uninit);
			bfa_fcs_port_deleted(port);
		} else {
			bfa_sm_set_state(port, bfa_fcs_port_sm_deleting);
			list_for_each_safe(qe, qen, &port->rport_q) {
				rport = (struct bfa_fcs_rport_s *)qe;
				bfa_fcs_rport_delete(rport);
			}
		}
		break;

	case BFA_FCS_PORT_SM_DELRPORT:
		break;

	default:
		bfa_assert(0);
	}
}

static void
bfa_fcs_port_sm_offline(struct bfa_fcs_port_s *port,
			enum bfa_fcs_port_event event)
{
	struct bfa_fcs_rport_s *rport;
	struct list_head *qe, *qen;

	bfa_trc(port->fcs, port->port_cfg.pwwn);
	bfa_trc(port->fcs, event);

	switch (event) {
	case BFA_FCS_PORT_SM_ONLINE:
		bfa_sm_set_state(port, bfa_fcs_port_sm_online);
		bfa_fcs_port_online_actions(port);
		break;

	case BFA_FCS_PORT_SM_DELETE:
		if (port->num_rports == 0) {
			bfa_sm_set_state(port, bfa_fcs_port_sm_uninit);
			bfa_fcs_port_deleted(port);
		} else {
			bfa_sm_set_state(port, bfa_fcs_port_sm_deleting);
			list_for_each_safe(qe, qen, &port->rport_q) {
				rport = (struct bfa_fcs_rport_s *)qe;
				bfa_fcs_rport_delete(rport);
			}
		}
		break;

	case BFA_FCS_PORT_SM_DELRPORT:
	case BFA_FCS_PORT_SM_OFFLINE:
		break;

	default:
		bfa_assert(0);
	}
}

static void
bfa_fcs_port_sm_deleting(struct bfa_fcs_port_s *port,
			 enum bfa_fcs_port_event event)
{
	bfa_trc(port->fcs, port->port_cfg.pwwn);
	bfa_trc(port->fcs, event);

	switch (event) {
	case BFA_FCS_PORT_SM_DELRPORT:
		if (port->num_rports == 0) {
			bfa_sm_set_state(port, bfa_fcs_port_sm_uninit);
			bfa_fcs_port_deleted(port);
		}
		break;

	default:
		bfa_assert(0);
	}
}






static void
bfa_fcs_port_aen_post(struct bfa_fcs_port_s *port,
		      enum bfa_lport_aen_event event)
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
	case BFA_LPORT_AEN_ONLINE:
		bfa_log(logmod, BFA_AEN_LPORT_ONLINE, lpwwn_ptr,
			role_str[role / 2]);
		break;
	case BFA_LPORT_AEN_OFFLINE:
		bfa_log(logmod, BFA_AEN_LPORT_OFFLINE, lpwwn_ptr,
			role_str[role / 2]);
		break;
	case BFA_LPORT_AEN_NEW:
		bfa_log(logmod, BFA_AEN_LPORT_NEW, lpwwn_ptr,
			role_str[role / 2]);
		break;
	case BFA_LPORT_AEN_DELETE:
		bfa_log(logmod, BFA_AEN_LPORT_DELETE, lpwwn_ptr,
			role_str[role / 2]);
		break;
	case BFA_LPORT_AEN_DISCONNECT:
		bfa_log(logmod, BFA_AEN_LPORT_DISCONNECT, lpwwn_ptr,
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
bfa_fcs_port_send_ls_rjt(struct bfa_fcs_port_s *port, struct fchs_s *rx_fchs,
			 u8 reason_code, u8 reason_code_expl)
{
	struct fchs_s          fchs;
	struct bfa_fcxp_s *fcxp;
	struct bfa_rport_s *bfa_rport = NULL;
	int             len;

	bfa_trc(port->fcs, rx_fchs->s_id);

	fcxp = bfa_fcs_fcxp_alloc(port->fcs);
	if (!fcxp)
		return;

	len = fc_ls_rjt_build(&fchs, bfa_fcxp_get_reqbuf(fcxp), rx_fchs->s_id,
			      bfa_fcs_port_get_fcid(port), rx_fchs->ox_id,
			      reason_code, reason_code_expl);

	bfa_fcxp_send(fcxp, bfa_rport, port->fabric->vf_id, port->lp_tag,
		      BFA_FALSE, FC_CLASS_3, len, &fchs, NULL, NULL,
		      FC_MAX_PDUSZ, 0);
}


static void
bfa_fcs_port_plogi(struct bfa_fcs_port_s *port, struct fchs_s *rx_fchs,
			struct fc_logi_s *plogi)
{
	struct bfa_fcs_rport_s *rport;

	bfa_trc(port->fcs, rx_fchs->d_id);
	bfa_trc(port->fcs, rx_fchs->s_id);

	
	if (__fcs_min_cfg(port->fcs)) {
		bfa_trc(port->fcs, rx_fchs->s_id);
		return;
	}

	if (fc_plogi_parse(rx_fchs) != FC_PARSE_OK) {
		bfa_trc(port->fcs, rx_fchs->s_id);
		
		bfa_fcs_port_send_ls_rjt(port, rx_fchs,
					 FC_LS_RJT_RSN_PROTOCOL_ERROR,
					 FC_LS_RJT_EXP_SPARMS_ERR_OPTIONS);
		return;
	}

	
	if ((!bfa_fcs_fabric_is_switched(port->fabric))
	    &&
	    (memcmp
	     ((void *)&bfa_fcs_port_get_pwwn(port), (void *)&plogi->port_name,
	      sizeof(wwn_t)) < 0)) {
		if (BFA_FCS_PID_IS_WKA(rx_fchs->d_id)) {
			
			bfa_fcs_port_send_ls_rjt(port, rx_fchs,
					FC_LS_RJT_RSN_PROTOCOL_ERROR,
					FC_LS_RJT_EXP_INVALID_NPORT_ID);
			return;
		}
		port->pid = rx_fchs->d_id;
	}

	
	rport = bfa_fcs_port_get_rport_by_pwwn(port, plogi->port_name);
	if (rport) {
		
		if ((!bfa_fcs_fabric_is_switched(port->fabric))
		    &&
		    (memcmp
		     ((void *)&bfa_fcs_port_get_pwwn(port),
		      (void *)&plogi->port_name, sizeof(wwn_t)) < 0)) {
			port->pid = rx_fchs->d_id;
			rport->pid = rx_fchs->s_id;
		}
		bfa_fcs_rport_plogi(rport, rx_fchs, plogi);
		return;
	}

	
	rport = bfa_fcs_port_get_rport_by_pid(port, rx_fchs->s_id);
	if (!rport) {
		
		bfa_fcs_rport_plogi_create(port, rx_fchs, plogi);
		return;
	}

	
	if (rport->pwwn) {
		
		bfa_assert(rport->pwwn != plogi->port_name);
		bfa_fcs_rport_logo_imp(rport);

		
		bfa_fcs_rport_plogi_create(port, rx_fchs, plogi);
		return;
	}

	
	bfa_assert(rport->pwwn == WWN_NULL);
	bfa_fcs_rport_plogi(rport, rx_fchs, plogi);
}


static void
bfa_fcs_port_echo(struct bfa_fcs_port_s *port, struct fchs_s *rx_fchs,
			struct fc_echo_s *echo, u16 rx_len)
{
	struct fchs_s          fchs;
	struct bfa_fcxp_s *fcxp;
	struct bfa_rport_s *bfa_rport = NULL;
	int             len, pyld_len;

	bfa_trc(port->fcs, rx_fchs->s_id);
	bfa_trc(port->fcs, rx_fchs->d_id);
	bfa_trc(port->fcs, rx_len);

	fcxp = bfa_fcs_fcxp_alloc(port->fcs);
	if (!fcxp)
		return;

	len = fc_ls_acc_build(&fchs, bfa_fcxp_get_reqbuf(fcxp), rx_fchs->s_id,
			      bfa_fcs_port_get_fcid(port), rx_fchs->ox_id);

	
	pyld_len = rx_len - sizeof(struct fchs_s);
	bfa_trc(port->fcs, pyld_len);

	if (pyld_len > len)
		memcpy(((u8 *) bfa_fcxp_get_reqbuf(fcxp)) +
		       sizeof(struct fc_echo_s), (echo + 1),
		       (pyld_len - sizeof(struct fc_echo_s)));

	bfa_fcxp_send(fcxp, bfa_rport, port->fabric->vf_id, port->lp_tag,
		      BFA_FALSE, FC_CLASS_3, pyld_len, &fchs, NULL, NULL,
		      FC_MAX_PDUSZ, 0);
}


static void
bfa_fcs_port_rnid(struct bfa_fcs_port_s *port, struct fchs_s *rx_fchs,
			struct fc_rnid_cmd_s *rnid, u16 rx_len)
{
	struct fc_rnid_common_id_data_s common_id_data;
	struct fc_rnid_general_topology_data_s gen_topo_data;
	struct fchs_s          fchs;
	struct bfa_fcxp_s *fcxp;
	struct bfa_rport_s *bfa_rport = NULL;
	u16        len;
	u32        data_format;

	bfa_trc(port->fcs, rx_fchs->s_id);
	bfa_trc(port->fcs, rx_fchs->d_id);
	bfa_trc(port->fcs, rx_len);

	fcxp = bfa_fcs_fcxp_alloc(port->fcs);
	if (!fcxp)
		return;

	
	bfa_trc(port->fcs, rnid->node_id_data_format);
	if (rnid->node_id_data_format == RNID_NODEID_DATA_FORMAT_DISCOVERY) {
		data_format = RNID_NODEID_DATA_FORMAT_DISCOVERY;
		
		bfa_fs_port_get_gen_topo_data(port, &gen_topo_data);
	} else {
		data_format = RNID_NODEID_DATA_FORMAT_COMMON;
	}

	
	common_id_data.port_name = bfa_fcs_port_get_pwwn(port);
	common_id_data.node_name = bfa_fcs_port_get_nwwn(port);

	len = fc_rnid_acc_build(&fchs, bfa_fcxp_get_reqbuf(fcxp), rx_fchs->s_id,
				bfa_fcs_port_get_fcid(port), rx_fchs->ox_id,
				data_format, &common_id_data, &gen_topo_data);

	bfa_fcxp_send(fcxp, bfa_rport, port->fabric->vf_id, port->lp_tag,
		      BFA_FALSE, FC_CLASS_3, len, &fchs, NULL, NULL,
		      FC_MAX_PDUSZ, 0);

	return;
}


static void
bfa_fs_port_get_gen_topo_data(struct bfa_fcs_port_s *port,
			struct fc_rnid_general_topology_data_s *gen_topo_data)
{

	bfa_os_memset(gen_topo_data, 0,
		      sizeof(struct fc_rnid_general_topology_data_s));

	gen_topo_data->asso_type = bfa_os_htonl(RNID_ASSOCIATED_TYPE_HOST);
	gen_topo_data->phy_port_num = 0;	
	gen_topo_data->num_attached_nodes = bfa_os_htonl(1);
}

static void
bfa_fcs_port_online_actions(struct bfa_fcs_port_s *port)
{
	bfa_trc(port->fcs, port->fabric->oper_type);

	__port_action[port->fabric->fab_type].init(port);
	__port_action[port->fabric->fab_type].online(port);

	bfa_fcs_port_aen_post(port, BFA_LPORT_AEN_ONLINE);
	bfa_fcb_port_online(port->fcs->bfad, port->port_cfg.roles,
			port->fabric->vf_drv, (port->vport == NULL) ?
			NULL : port->vport->vport_drv);
}

static void
bfa_fcs_port_offline_actions(struct bfa_fcs_port_s *port)
{
	struct list_head *qe, *qen;
	struct bfa_fcs_rport_s *rport;

	bfa_trc(port->fcs, port->fabric->oper_type);

	__port_action[port->fabric->fab_type].offline(port);

	if (bfa_fcs_fabric_is_online(port->fabric) == BFA_TRUE) {
		bfa_fcs_port_aen_post(port, BFA_LPORT_AEN_DISCONNECT);
	} else {
		bfa_fcs_port_aen_post(port, BFA_LPORT_AEN_OFFLINE);
	}
	bfa_fcb_port_offline(port->fcs->bfad, port->port_cfg.roles,
			port->fabric->vf_drv,
			(port->vport == NULL) ? NULL : port->vport->vport_drv);

	list_for_each_safe(qe, qen, &port->rport_q) {
		rport = (struct bfa_fcs_rport_s *)qe;
		bfa_fcs_rport_offline(rport);
	}
}

static void
bfa_fcs_port_unknown_init(struct bfa_fcs_port_s *port)
{
	bfa_assert(0);
}

static void
bfa_fcs_port_unknown_online(struct bfa_fcs_port_s *port)
{
	bfa_assert(0);
}

static void
bfa_fcs_port_unknown_offline(struct bfa_fcs_port_s *port)
{
	bfa_assert(0);
}

static void
bfa_fcs_port_deleted(struct bfa_fcs_port_s *port)
{
	bfa_fcs_port_aen_post(port, BFA_LPORT_AEN_DELETE);

	
	if (port->vport) {
		bfa_fcb_port_delete(port->fcs->bfad, port->port_cfg.roles,
			port->fabric->vf_drv,
			port->vport ? port->vport->vport_drv : NULL);
		bfa_fcs_vport_delete_comp(port->vport);
	} else {
		bfa_fcs_fabric_port_delete_comp(port->fabric);
	}
}





void
bfa_fcs_port_modinit(struct bfa_fcs_s *fcs)
{

}


void
bfa_fcs_port_modexit(struct bfa_fcs_s *fcs)
{
	bfa_fcs_modexit_comp(fcs);
}


void
bfa_fcs_port_uf_recv(struct bfa_fcs_port_s *lport, struct fchs_s *fchs,
			u16 len)
{
	u32        pid = fchs->s_id;
	struct bfa_fcs_rport_s *rport = NULL;
	struct fc_els_cmd_s   *els_cmd = (struct fc_els_cmd_s *) (fchs + 1);

	bfa_stats(lport, uf_recvs);

	if (!bfa_fcs_port_is_online(lport)) {
		bfa_stats(lport, uf_recv_drops);
		return;
	}

	
	
	if ((fchs->type == FC_TYPE_ELS) &&
		(els_cmd->els_code == FC_ELS_PLOGI)) {
		bfa_fcs_port_plogi(lport, fchs, (struct fc_logi_s *) els_cmd);
		return;
	}

	
	if ((fchs->type == FC_TYPE_ELS) && (els_cmd->els_code == FC_ELS_ECHO)) {
		bfa_fcs_port_echo(lport, fchs,
			(struct fc_echo_s *) els_cmd, len);
		return;
	}

	
	if ((fchs->type == FC_TYPE_ELS) && (els_cmd->els_code == FC_ELS_RNID)) {
		bfa_fcs_port_rnid(lport, fchs,
			(struct fc_rnid_cmd_s *) els_cmd, len);
		return;
	}

	
	rport = bfa_fcs_port_get_rport_by_pid(lport, pid);
	if (rport) {
		bfa_trc(rport->fcs, fchs->s_id);
		bfa_trc(rport->fcs, fchs->d_id);
		bfa_trc(rport->fcs, fchs->type);

		bfa_fcs_rport_uf_recv(rport, fchs, len);
		return;
	}

	
	if (fchs->type != FC_TYPE_ELS) {
		bfa_trc(lport->fcs, fchs->type);
		bfa_assert(0);
		return;
	}

	bfa_trc(lport->fcs, els_cmd->els_code);
	if (els_cmd->els_code == FC_ELS_RSCN) {
		bfa_fcs_port_scn_process_rscn(lport, fchs, len);
		return;
	}

	if (els_cmd->els_code == FC_ELS_LOGO) {
		
		bfa_trc(lport->fcs, els_cmd->els_code);
		return;
	}

	if (els_cmd->els_code == FC_ELS_PRLI) {
		
		bfa_trc(lport->fcs, els_cmd->els_code);
		return;
	}

	
	bfa_fcs_port_send_ls_rjt(lport, fchs, FC_LS_RJT_RSN_CMD_NOT_SUPP,
				 FC_LS_RJT_EXP_NO_ADDL_INFO);

}


struct bfa_fcs_rport_s *
bfa_fcs_port_get_rport_by_pid(struct bfa_fcs_port_s *port, u32 pid)
{
	struct bfa_fcs_rport_s *rport;
	struct list_head *qe;

	list_for_each(qe, &port->rport_q) {
		rport = (struct bfa_fcs_rport_s *)qe;
		if (rport->pid == pid)
			return rport;
	}

	bfa_trc(port->fcs, pid);
	return NULL;
}


struct bfa_fcs_rport_s *
bfa_fcs_port_get_rport_by_pwwn(struct bfa_fcs_port_s *port, wwn_t pwwn)
{
	struct bfa_fcs_rport_s *rport;
	struct list_head *qe;

	list_for_each(qe, &port->rport_q) {
		rport = (struct bfa_fcs_rport_s *)qe;
		if (wwn_is_equal(rport->pwwn, pwwn))
			return rport;
	}

	bfa_trc(port->fcs, pwwn);
	return (NULL);
}


struct bfa_fcs_rport_s *
bfa_fcs_port_get_rport_by_nwwn(struct bfa_fcs_port_s *port, wwn_t nwwn)
{
	struct bfa_fcs_rport_s *rport;
	struct list_head *qe;

	list_for_each(qe, &port->rport_q) {
		rport = (struct bfa_fcs_rport_s *)qe;
		if (wwn_is_equal(rport->nwwn, nwwn))
			return rport;
	}

	bfa_trc(port->fcs, nwwn);
	return (NULL);
}


void
bfa_fcs_port_add_rport(struct bfa_fcs_port_s *port,
		       struct bfa_fcs_rport_s *rport)
{
	list_add_tail(&rport->qe, &port->rport_q);
	port->num_rports++;
}


void
bfa_fcs_port_del_rport(struct bfa_fcs_port_s *port,
		       struct bfa_fcs_rport_s *rport)
{
	bfa_assert(bfa_q_is_on_q(&port->rport_q, rport));
	list_del(&rport->qe);
	port->num_rports--;

	bfa_sm_send_event(port, BFA_FCS_PORT_SM_DELRPORT);
}


void
bfa_fcs_port_online(struct bfa_fcs_port_s *port)
{
	bfa_sm_send_event(port, BFA_FCS_PORT_SM_ONLINE);
}


void
bfa_fcs_port_offline(struct bfa_fcs_port_s *port)
{
	bfa_sm_send_event(port, BFA_FCS_PORT_SM_OFFLINE);
}


void
bfa_fcs_port_delete(struct bfa_fcs_port_s *port)
{
	bfa_sm_send_event(port, BFA_FCS_PORT_SM_DELETE);
}


void
bfa_fcs_port_lip(struct bfa_fcs_port_s *port)
{
}


bfa_boolean_t
bfa_fcs_port_is_online(struct bfa_fcs_port_s *port)
{
	return (bfa_sm_cmp_state(port, bfa_fcs_port_sm_online));
}


void
bfa_fcs_lport_init(struct bfa_fcs_port_s *lport, struct bfa_fcs_s *fcs,
		   u16 vf_id, struct bfa_port_cfg_s *port_cfg,
		   struct bfa_fcs_vport_s *vport)
{
	lport->fcs = fcs;
	lport->fabric = bfa_fcs_vf_lookup(fcs, vf_id);
	bfa_os_assign(lport->port_cfg, *port_cfg);
	lport->vport = vport;
	lport->lp_tag = (vport) ? bfa_lps_get_tag(vport->lps) :
			 bfa_lps_get_tag(lport->fabric->lps);

	INIT_LIST_HEAD(&lport->rport_q);
	lport->num_rports = 0;

	lport->bfad_port =
		bfa_fcb_port_new(fcs->bfad, lport, lport->port_cfg.roles,
				lport->fabric->vf_drv,
				vport ? vport->vport_drv : NULL);
	bfa_fcs_port_aen_post(lport, BFA_LPORT_AEN_NEW);

	bfa_sm_set_state(lport, bfa_fcs_port_sm_uninit);
	bfa_sm_send_event(lport, BFA_FCS_PORT_SM_CREATE);
}





void
bfa_fcs_port_get_attr(struct bfa_fcs_port_s *port,
		      struct bfa_port_attr_s *port_attr)
{
	if (bfa_sm_cmp_state(port, bfa_fcs_port_sm_online))
		port_attr->pid = port->pid;
	else
		port_attr->pid = 0;

	port_attr->port_cfg = port->port_cfg;

	if (port->fabric) {
		port_attr->port_type = bfa_fcs_fabric_port_type(port->fabric);
		port_attr->loopback = bfa_fcs_fabric_is_loopback(port->fabric);
		port_attr->fabric_name = bfa_fcs_port_get_fabric_name(port);
		memcpy(port_attr->fabric_ip_addr,
		       bfa_fcs_port_get_fabric_ipaddr(port),
		       BFA_FCS_FABRIC_IPADDR_SZ);

		if (port->vport != NULL)
			port_attr->port_type = BFA_PPORT_TYPE_VPORT;

	} else {
		port_attr->port_type = BFA_PPORT_TYPE_UNKNOWN;
		port_attr->state = BFA_PORT_UNINIT;
	}

}


