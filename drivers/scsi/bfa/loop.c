


#include <bfa.h>
#include <bfa_svc.h>
#include "fcs_lport.h"
#include "fcs_rport.h"
#include "fcs_trcmod.h"
#include "lport_priv.h"

BFA_TRC_FILE(FCS, LOOP);


static const u8   port_loop_alpa_map[] = {
	0xEF, 0xE8, 0xE4, 0xE2, 0xE1, 0xE0, 0xDC, 0xDA,	
	0xD9, 0xD6, 0xD5, 0xD4, 0xD3, 0xD2, 0xD1, 0xCE,	
	0xCD, 0xCC, 0xCB, 0xCA, 0xC9, 0xC7, 0xC6, 0xC5,	
	0xC3, 0xBC, 0xBA, 0xB9, 0xB6, 0xB5, 0xB4, 0xB3,	

	0xB2, 0xB1, 0xAE, 0xAD, 0xAC, 0xAB, 0xAA, 0xA9,	
	0xA7, 0xA6, 0xA5, 0xA3, 0x9F, 0x9E, 0x9D, 0x9B,	
	0x98, 0x97, 0x90, 0x8F, 0x88, 0x84, 0x82, 0x81,	
	0x80, 0x7C, 0x7A, 0x79, 0x76, 0x75, 0x74, 0x73,	

	0x72, 0x71, 0x6E, 0x6D, 0x6C, 0x6B, 0x6A, 0x69,	
	0x67, 0x66, 0x65, 0x63, 0x5C, 0x5A, 0x59, 0x56,	
	0x55, 0x54, 0x53, 0x52, 0x51, 0x4E, 0x4D, 0x4C,	
	0x4B, 0x4A, 0x49, 0x47, 0x46, 0x45, 0x43, 0x3C,	

	0x3A, 0x39, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31,	
	0x2E, 0x2D, 0x2C, 0x2B, 0x2A, 0x29, 0x27, 0x26,	
	0x25, 0x23, 0x1F, 0x1E, 0x1D, 0x1B, 0x18, 0x17,	
	0x10, 0x0F, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00,	
};


bfa_status_t    bfa_fcs_port_loop_send_plogi(struct bfa_fcs_port_s *port,
					     u8 alpa);

void            bfa_fcs_port_loop_plogi_response(void *fcsarg,
						 struct bfa_fcxp_s *fcxp,
						 void *cbarg,
						 bfa_status_t req_status,
						 u32 rsp_len,
						 u32 resid_len,
						 struct fchs_s *rsp_fchs);

bfa_status_t    bfa_fcs_port_loop_send_adisc(struct bfa_fcs_port_s *port,
					     u8 alpa);

void            bfa_fcs_port_loop_adisc_response(void *fcsarg,
						 struct bfa_fcxp_s *fcxp,
						 void *cbarg,
						 bfa_status_t req_status,
						 u32 rsp_len,
						 u32 resid_len,
						 struct fchs_s *rsp_fchs);

bfa_status_t    bfa_fcs_port_loop_send_plogi_acc(struct bfa_fcs_port_s *port,
						 u8 alpa);

void            bfa_fcs_port_loop_plogi_acc_response(void *fcsarg,
						     struct bfa_fcxp_s *fcxp,
						     void *cbarg,
						     bfa_status_t req_status,
						     u32 rsp_len,
						     u32 resid_len,
						     struct fchs_s *rsp_fchs);

bfa_status_t    bfa_fcs_port_loop_send_adisc_acc(struct bfa_fcs_port_s *port,
						 u8 alpa);

void            bfa_fcs_port_loop_adisc_acc_response(void *fcsarg,
						     struct bfa_fcxp_s *fcxp,
						     void *cbarg,
						     bfa_status_t req_status,
						     u32 rsp_len,
						     u32 resid_len,
						     struct fchs_s *rsp_fchs);

void
bfa_fcs_port_loop_init(struct bfa_fcs_port_s *port)
{
}


void
bfa_fcs_port_loop_online(struct bfa_fcs_port_s *port)
{

	u8         num_alpa = port->port_topo.ploop.num_alpa;
	u8        *alpa_pos_map = port->port_topo.ploop.alpa_pos_map;
	struct bfa_fcs_rport_s *r_port;
	int             ii = 0;

	
	if (port->port_cfg.roles == BFA_PORT_ROLE_FCP_IM) {
		
		if (num_alpa > 0) {
			for (ii = 0; ii < num_alpa; ii++) {
				
				if (alpa_pos_map[ii] != port->pid) {
					r_port = bfa_fcs_rport_create(port,
						alpa_pos_map[ii]);
				}
			}
		} else {
			for (ii = 0; ii < MAX_ALPA_COUNT; ii++) {
				
				if ((port_loop_alpa_map[ii] > 0)
				    && (port_loop_alpa_map[ii] != port->pid))
					bfa_fcs_port_loop_send_plogi(port,
						port_loop_alpa_map[ii]);
				
			}
		}
	} else {
		
	}

}


void
bfa_fcs_port_loop_offline(struct bfa_fcs_port_s *port)
{

}


void
bfa_fcs_port_loop_lip(struct bfa_fcs_port_s *port)
{
}


bfa_status_t
bfa_fcs_port_loop_send_plogi(struct bfa_fcs_port_s *port, u8 alpa)
{
	struct fchs_s          fchs;
	struct bfa_fcxp_s *fcxp = NULL;
	int             len;

	bfa_trc(port->fcs, alpa);

	fcxp = bfa_fcxp_alloc(NULL, port->fcs->bfa, 0, 0, NULL, NULL, NULL,
				  NULL);
	bfa_assert(fcxp);

	len = fc_plogi_build(&fchs, bfa_fcxp_get_reqbuf(fcxp), alpa,
			     bfa_fcs_port_get_fcid(port), 0,
			     port->port_cfg.pwwn, port->port_cfg.nwwn,
				 bfa_pport_get_maxfrsize(port->fcs->bfa));

	bfa_fcxp_send(fcxp, NULL, port->fabric->vf_id, port->lp_tag, BFA_FALSE,
			  FC_CLASS_3, len, &fchs,
			  bfa_fcs_port_loop_plogi_response, (void *)port,
			  FC_MAX_PDUSZ, FC_RA_TOV);

	return BFA_STATUS_OK;
}


void
bfa_fcs_port_loop_plogi_response(void *fcsarg, struct bfa_fcxp_s *fcxp,
				 void *cbarg, bfa_status_t req_status,
				 u32 rsp_len, u32 resid_len,
				 struct fchs_s *rsp_fchs)
{
	struct bfa_fcs_port_s *port = (struct bfa_fcs_port_s *) cbarg;
	struct fc_logi_s     *plogi_resp;
	struct fc_els_cmd_s   *els_cmd;

	bfa_trc(port->fcs, req_status);

	
	if (req_status != BFA_STATUS_OK) {
		bfa_trc(port->fcs, req_status);
		

		return;
	}

	els_cmd = (struct fc_els_cmd_s *) BFA_FCXP_RSP_PLD(fcxp);
	plogi_resp = (struct fc_logi_s *) els_cmd;

	if (els_cmd->els_code == FC_ELS_ACC) {
		bfa_fcs_rport_start(port, rsp_fchs, plogi_resp);
	} else {
		bfa_trc(port->fcs, plogi_resp->els_cmd.els_code);
		bfa_assert(0);
	}
}

bfa_status_t
bfa_fcs_port_loop_send_plogi_acc(struct bfa_fcs_port_s *port, u8 alpa)
{
	struct fchs_s          fchs;
	struct bfa_fcxp_s *fcxp;
	int             len;

	bfa_trc(port->fcs, alpa);

	fcxp = bfa_fcxp_alloc(NULL, port->fcs->bfa, 0, 0, NULL, NULL, NULL,
				  NULL);
	bfa_assert(fcxp);

	len = fc_plogi_acc_build(&fchs, bfa_fcxp_get_reqbuf(fcxp), alpa,
				 bfa_fcs_port_get_fcid(port), 0,
				 port->port_cfg.pwwn, port->port_cfg.nwwn,
				 bfa_pport_get_maxfrsize(port->fcs->bfa));

	bfa_fcxp_send(fcxp, NULL, port->fabric->vf_id, port->lp_tag, BFA_FALSE,
				 FC_CLASS_3, len, &fchs,
				 bfa_fcs_port_loop_plogi_acc_response,
				 (void *)port, FC_MAX_PDUSZ, 0); 

	return BFA_STATUS_OK;
}


void
bfa_fcs_port_loop_plogi_acc_response(void *fcsarg, struct bfa_fcxp_s *fcxp,
				     void *cbarg, bfa_status_t req_status,
				     u32 rsp_len, u32 resid_len,
				     struct fchs_s *rsp_fchs)
{

	struct bfa_fcs_port_s *port = (struct bfa_fcs_port_s *) cbarg;

	bfa_trc(port->fcs, port->pid);

	
	if (req_status != BFA_STATUS_OK) {
		bfa_trc(port->fcs, req_status);
		return;
	}
}

bfa_status_t
bfa_fcs_port_loop_send_adisc(struct bfa_fcs_port_s *port, u8 alpa)
{
	struct fchs_s          fchs;
	struct bfa_fcxp_s *fcxp;
	int             len;

	bfa_trc(port->fcs, alpa);

	fcxp = bfa_fcxp_alloc(NULL, port->fcs->bfa, 0, 0, NULL, NULL, NULL,
				  NULL);
	bfa_assert(fcxp);

	len = fc_adisc_build(&fchs, bfa_fcxp_get_reqbuf(fcxp), alpa,
			     bfa_fcs_port_get_fcid(port), 0,
			     port->port_cfg.pwwn, port->port_cfg.nwwn);

	bfa_fcxp_send(fcxp, NULL, port->fabric->vf_id, port->lp_tag, BFA_FALSE,
			  FC_CLASS_3, len, &fchs,
			  bfa_fcs_port_loop_adisc_response, (void *)port,
			  FC_MAX_PDUSZ, FC_RA_TOV);

	return BFA_STATUS_OK;
}


void
bfa_fcs_port_loop_adisc_response(void *fcsarg, struct bfa_fcxp_s *fcxp,
				 void *cbarg, bfa_status_t req_status,
				 u32 rsp_len, u32 resid_len,
				 struct fchs_s *rsp_fchs)
{
	struct bfa_fcs_port_s *port = (struct bfa_fcs_port_s *) cbarg;
	struct bfa_fcs_rport_s *rport;
	struct fc_adisc_s     *adisc_resp;
	struct fc_els_cmd_s   *els_cmd;
	u32        pid = rsp_fchs->s_id;

	bfa_trc(port->fcs, req_status);

	
	if (req_status != BFA_STATUS_OK) {
		
		bfa_fcxp_free(fcxp);
		return;
	}

	els_cmd = (struct fc_els_cmd_s *) BFA_FCXP_RSP_PLD(fcxp);
	adisc_resp = (struct fc_adisc_s *) els_cmd;

	if (els_cmd->els_code == FC_ELS_ACC) {
	} else {
		bfa_trc(port->fcs, adisc_resp->els_cmd.els_code);

		
		rport = bfa_fcs_port_get_rport_by_pid(port, pid);
		if (rport) {
			list_del(&rport->qe);
			bfa_fcs_rport_delete(rport);
		}

	}
	return;
}

bfa_status_t
bfa_fcs_port_loop_send_adisc_acc(struct bfa_fcs_port_s *port, u8 alpa)
{
	struct fchs_s          fchs;
	struct bfa_fcxp_s *fcxp;
	int             len;

	bfa_trc(port->fcs, alpa);

	fcxp = bfa_fcxp_alloc(NULL, port->fcs->bfa, 0, 0, NULL, NULL, NULL,
				  NULL);
	bfa_assert(fcxp);

	len = fc_adisc_acc_build(&fchs, bfa_fcxp_get_reqbuf(fcxp), alpa,
				 bfa_fcs_port_get_fcid(port), 0,
				 port->port_cfg.pwwn, port->port_cfg.nwwn);

	bfa_fcxp_send(fcxp, NULL, port->fabric->vf_id, port->lp_tag, BFA_FALSE,
				FC_CLASS_3, len, &fchs,
				bfa_fcs_port_loop_adisc_acc_response,
				(void *)port, FC_MAX_PDUSZ, 0); 

	return BFA_STATUS_OK;
}


void
bfa_fcs_port_loop_adisc_acc_response(void *fcsarg, struct bfa_fcxp_s *fcxp,
				     void *cbarg, bfa_status_t req_status,
				     u32 rsp_len, u32 resid_len,
				     struct fchs_s *rsp_fchs)
{

	struct bfa_fcs_port_s *port = (struct bfa_fcs_port_s *) cbarg;

	bfa_trc(port->fcs, port->pid);

	
	if (req_status != BFA_STATUS_OK) {
		bfa_trc(port->fcs, req_status);
		return;
	}
}
