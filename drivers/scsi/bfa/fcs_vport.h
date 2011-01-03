

#ifndef __FCS_VPORT_H__
#define __FCS_VPORT_H__

#include <fcs/bfa_fcs_lport.h>
#include <fcs/bfa_fcs_vport.h>
#include <defs/bfa_defs_pci.h>



void bfa_fcs_vport_modinit(struct bfa_fcs_s *fcs);
void bfa_fcs_vport_modexit(struct bfa_fcs_s *fcs);

void bfa_fcs_vport_cleanup(struct bfa_fcs_vport_s *vport);
void bfa_fcs_vport_online(struct bfa_fcs_vport_s *vport);
void bfa_fcs_vport_offline(struct bfa_fcs_vport_s *vport);
void bfa_fcs_vport_delete_comp(struct bfa_fcs_vport_s *vport);
u32 bfa_fcs_vport_get_max(struct bfa_fcs_s *fcs);

#endif 

