



#ifndef __FCS_RPORT_H__
#define __FCS_RPORT_H__

#include <fcs/bfa_fcs_rport.h>

void bfa_fcs_rport_modinit(struct bfa_fcs_s *fcs);
void bfa_fcs_rport_modexit(struct bfa_fcs_s *fcs);

void bfa_fcs_rport_uf_recv(struct bfa_fcs_rport_s *rport, struct fchs_s *fchs,
			u16 len);
void bfa_fcs_rport_scn(struct bfa_fcs_rport_s *rport);

struct bfa_fcs_rport_s *bfa_fcs_rport_create(struct bfa_fcs_port_s *port,
			u32 pid);
void bfa_fcs_rport_delete(struct bfa_fcs_rport_s *rport);
void bfa_fcs_rport_online(struct bfa_fcs_rport_s *rport);
void bfa_fcs_rport_offline(struct bfa_fcs_rport_s *rport);
void bfa_fcs_rport_start(struct bfa_fcs_port_s *port, struct fchs_s *rx_fchs,
			struct fc_logi_s *plogi_rsp);
void bfa_fcs_rport_plogi_create(struct bfa_fcs_port_s *port,
			struct fchs_s *rx_fchs,
			struct fc_logi_s *plogi);
void bfa_fcs_rport_plogi(struct bfa_fcs_rport_s *rport, struct fchs_s *fchs,
			struct fc_logi_s *plogi);
void bfa_fcs_rport_logo_imp(struct bfa_fcs_rport_s *rport);
void bfa_fcs_rport_itnim_ack(struct bfa_fcs_rport_s *rport);
void bfa_fcs_rport_itntm_ack(struct bfa_fcs_rport_s *rport);
void bfa_fcs_rport_tin_ack(struct bfa_fcs_rport_s *rport);
void bfa_fcs_rport_fcptm_offline_done(struct bfa_fcs_rport_s *rport);
int  bfa_fcs_rport_get_state(struct bfa_fcs_rport_s *rport);
struct bfa_fcs_rport_s *bfa_fcs_rport_create_by_wwn(struct bfa_fcs_port_s *port,
			wwn_t wwn);



void  bfa_fcs_rpf_init(struct bfa_fcs_rport_s *rport);
void  bfa_fcs_rpf_rport_online(struct bfa_fcs_rport_s *rport);
void  bfa_fcs_rpf_rport_offline(struct bfa_fcs_rport_s *rport);

#endif 
