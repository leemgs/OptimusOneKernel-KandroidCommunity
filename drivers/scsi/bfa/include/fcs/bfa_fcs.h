

#ifndef __BFA_FCS_H__
#define __BFA_FCS_H__

#include <cs/bfa_debug.h>
#include <defs/bfa_defs_status.h>
#include <defs/bfa_defs_version.h>
#include <bfa.h>
#include <fcs/bfa_fcs_fabric.h>

#define BFA_FCS_OS_STR_LEN  		64

struct bfa_fcs_stats_s {
	struct {
		u32        untagged; 
		u32        tagged;	
		u32        vfid_unknown;	
	} uf;
};

struct bfa_fcs_driver_info_s {
	u8  version[BFA_VERSION_LEN];		
	u8  host_machine_name[BFA_FCS_OS_STR_LEN];
	u8  host_os_name[BFA_FCS_OS_STR_LEN]; 
	u8  host_os_patch[BFA_FCS_OS_STR_LEN];
	u8  os_device_name[BFA_FCS_OS_STR_LEN]; 
};

struct bfa_fcs_s {
	struct bfa_s      *bfa;	
	struct bfad_s         *bfad; 
	struct bfa_log_mod_s  *logm;	
	struct bfa_trc_mod_s  *trcmod;	
	struct bfa_aen_s      *aen;	
	bfa_boolean_t   vf_enabled;	
	bfa_boolean_t min_cfg;		
	u16        port_vfid;	
	struct bfa_fcs_driver_info_s driver_info;
	struct bfa_fcs_fabric_s fabric; 
	struct bfa_fcs_stats_s	stats;	
	struct bfa_wc_s       	wc;	
};


void bfa_fcs_init(struct bfa_fcs_s *fcs, struct bfa_s *bfa, struct bfad_s *bfad,
			bfa_boolean_t min_cfg);
void bfa_fcs_driver_info_init(struct bfa_fcs_s *fcs,
			struct bfa_fcs_driver_info_s *driver_info);
void bfa_fcs_exit(struct bfa_fcs_s *fcs);
void bfa_fcs_trc_init(struct bfa_fcs_s *fcs, struct bfa_trc_mod_s *trcmod);
void bfa_fcs_log_init(struct bfa_fcs_s *fcs, struct bfa_log_mod_s *logmod);
void bfa_fcs_aen_init(struct bfa_fcs_s *fcs, struct bfa_aen_s *aen);
void 	  	bfa_fcs_start(struct bfa_fcs_s *fcs);

#endif 
