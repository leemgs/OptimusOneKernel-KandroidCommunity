
#ifndef __BFA_DEFS_CEE_H__
#define __BFA_DEFS_CEE_H__

#include <defs/bfa_defs_types.h>
#include <defs/bfa_defs_pport.h>
#include <protocol/types.h>

#pragma pack(1)

#define BFA_CEE_LLDP_MAX_STRING_LEN (128)



#define BFA_CEE_LLDP_SYS_CAP_OTHER       0x0001
#define BFA_CEE_LLDP_SYS_CAP_REPEATER    0x0002
#define BFA_CEE_LLDP_SYS_CAP_MAC_BRIDGE  0x0004
#define BFA_CEE_LLDP_SYS_CAP_WLAN_AP     0x0008
#define BFA_CEE_LLDP_SYS_CAP_ROUTER      0x0010
#define BFA_CEE_LLDP_SYS_CAP_TELEPHONE 	 0x0020
#define BFA_CEE_LLDP_SYS_CAP_DOCSIS_CD   0x0040
#define BFA_CEE_LLDP_SYS_CAP_STATION     0x0080
#define BFA_CEE_LLDP_SYS_CAP_CVLAN	     0x0100
#define BFA_CEE_LLDP_SYS_CAP_SVLAN 	     0x0200
#define BFA_CEE_LLDP_SYS_CAP_TPMR		 0x0400



struct bfa_cee_lldp_str_s {
	u8 sub_type;
	u8 len;
	u8 rsvd[2];
	u8 value[BFA_CEE_LLDP_MAX_STRING_LEN];
};



struct bfa_cee_lldp_cfg_s {
	struct bfa_cee_lldp_str_s chassis_id;
	struct bfa_cee_lldp_str_s port_id;
	struct bfa_cee_lldp_str_s port_desc;
	struct bfa_cee_lldp_str_s sys_name;
	struct bfa_cee_lldp_str_s sys_desc;
	struct bfa_cee_lldp_str_s mgmt_addr;
	u16    time_to_interval;
	u16    enabled_system_cap;
};

enum bfa_cee_dcbx_version_e {
	DCBX_PROTOCOL_PRECEE = 1,
	DCBX_PROTOCOL_CEE    = 2,
};

enum bfa_cee_lls_e {
	CEE_LLS_DOWN_NO_TLV = 0, 
	CEE_LLS_DOWN        = 1, 
	CEE_LLS_UP          = 2,
};


struct bfa_cee_dcbx_cfg_s {
	u8 pgid[8];
	u8 pg_percentage[8];
	u8 pfc_enabled;          
	u8 fcoe_user_priority;   
	u8 dcbx_version;	
	u8 lls_fcoe;	
	u8 lls_lan;	
	u8 rsvd[3];
};



enum bfa_cee_status_e {
    CEE_PHY_DOWN = 0,
    CEE_PHY_UP = 1,
    CEE_UP = 2,
};


struct bfa_cee_attr_s {
	u8                   cee_status;
	u8                   error_reason;
	struct bfa_cee_lldp_cfg_s lldp_remote;
	struct bfa_cee_dcbx_cfg_s dcbx_remote;
	mac_t src_mac;
	u8 link_speed;
	u8 filler[3];
};






struct bfa_cee_lldp_stats_s {
	u32 frames_transmitted;
	u32 frames_aged_out;
	u32 frames_discarded;
	u32 frames_in_error;
	u32 frames_rcvd;
	u32 tlvs_discarded;
	u32 tlvs_unrecognized;
};

struct bfa_cee_dcbx_stats_s {
	u32 subtlvs_unrecognized;
	u32 negotiation_failed;
	u32 remote_cfg_changed;
	u32 tlvs_received;
	u32 tlvs_invalid;
	u32 seqno;
	u32 ackno;
	u32 recvd_seqno;
	u32 recvd_ackno;
};

struct bfa_cee_cfg_stats_s {
	u32 cee_status_down;
	u32 cee_status_up;
	u32 cee_hw_cfg_changed;
	u32 recvd_invalid_cfg;
};


struct bfa_cee_stats_s {
	struct bfa_cee_lldp_stats_s lldp_stats;
	struct bfa_cee_dcbx_stats_s dcbx_stats;
	struct bfa_cee_cfg_stats_s  cfg_stats;
};

#pragma pack()


#endif	


