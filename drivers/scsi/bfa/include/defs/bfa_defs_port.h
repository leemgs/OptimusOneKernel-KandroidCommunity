

#ifndef __BFA_DEFS_PORT_H__
#define __BFA_DEFS_PORT_H__

#include <bfa_os_inc.h>
#include <protocol/types.h>
#include <defs/bfa_defs_pport.h>
#include <defs/bfa_defs_ioc.h>

#define BFA_FCS_FABRIC_IPADDR_SZ  16


#define BFA_SYMNAME_MAXLEN	128	
struct bfa_port_symname_s {
	char            symname[BFA_SYMNAME_MAXLEN];
};


enum bfa_port_role {
	BFA_PORT_ROLE_FCP_IM 	= 0x01,	
	BFA_PORT_ROLE_FCP_TM 	= 0x02,	
	BFA_PORT_ROLE_FCP_IPFC 	= 0x04,	
	BFA_PORT_ROLE_FCP_MAX 	= BFA_PORT_ROLE_FCP_IPFC | BFA_PORT_ROLE_FCP_IM
};


struct bfa_port_cfg_s {
    wwn_t               pwwn;       
    wwn_t               nwwn;       
    struct bfa_port_symname_s  sym_name;   
    enum bfa_port_role     roles;      
	u32			rsvd;
    u8             tag[16];	
};


enum bfa_port_state {
	BFA_PORT_UNINIT  = 0,	
	BFA_PORT_FDISC   = 1,	
	BFA_PORT_ONLINE  = 2,	
	BFA_PORT_OFFLINE = 3,	
};


enum bfa_port_type {
	BFA_PORT_TYPE_PHYSICAL = 0,
	BFA_PORT_TYPE_VIRTUAL,
};


enum bfa_port_offline_reason {
	BFA_PORT_OFFLINE_UNKNOWN = 0,
	BFA_PORT_OFFLINE_LINKDOWN,
	BFA_PORT_OFFLINE_FAB_UNSUPPORTED,	
	BFA_PORT_OFFLINE_FAB_NORESOURCES,
	BFA_PORT_OFFLINE_FAB_LOGOUT,
};


struct bfa_port_info_s {
	u8         port_type;	
	u8         port_state;	
	u8         offline_reason;	
	wwn_t           port_wwn;
	wwn_t           node_wwn;

	
	u32        max_vports_supp;	
	u32        num_vports_inuse;	
	u32        max_rports_supp;	
	u32        num_rports_inuse;	

};


struct bfa_port_stats_s {
	u32        ns_plogi_sent;
	u32        ns_plogi_rsp_err;
	u32        ns_plogi_acc_err;
	u32        ns_plogi_accepts;
	u32        ns_rejects;	
	u32        ns_plogi_unknown_rsp;
	u32        ns_plogi_alloc_wait;

	u32        ns_retries;	
	u32        ns_timeouts;	

	u32        ns_rspnid_sent;
	u32        ns_rspnid_accepts;
	u32        ns_rspnid_rsp_err;
	u32        ns_rspnid_rejects;
	u32        ns_rspnid_alloc_wait;

	u32        ns_rftid_sent;
	u32        ns_rftid_accepts;
	u32        ns_rftid_rsp_err;
	u32        ns_rftid_rejects;
	u32        ns_rftid_alloc_wait;

	u32	ns_rffid_sent;
	u32	ns_rffid_accepts;
	u32	ns_rffid_rsp_err;
	u32	ns_rffid_rejects;
	u32	ns_rffid_alloc_wait;

	u32        ns_gidft_sent;
	u32        ns_gidft_accepts;
	u32        ns_gidft_rsp_err;
	u32        ns_gidft_rejects;
	u32        ns_gidft_unknown_rsp;
	u32        ns_gidft_alloc_wait;

	
	u32        ms_retries;	
	u32        ms_timeouts;	
	u32        ms_plogi_sent;
	u32        ms_plogi_rsp_err;
	u32        ms_plogi_acc_err;
	u32        ms_plogi_accepts;
	u32        ms_rejects;	
	u32        ms_plogi_unknown_rsp;
	u32        ms_plogi_alloc_wait;

	u32        num_rscn;	
	u32        num_portid_rscn;

	u32	uf_recvs; 	
	u32	uf_recv_drops; 	

	u32	rsvd; 		
};


struct bfa_port_attr_s {
	enum bfa_port_state state;		
	u32         pid;		
	struct bfa_port_cfg_s   port_cfg;	
	enum bfa_pport_type port_type;	
	u32         loopback;	
	wwn_t		fabric_name; 
	u8		fabric_ip_addr[BFA_FCS_FABRIC_IPADDR_SZ]; 
};


enum bfa_port_aen_event {
	BFA_PORT_AEN_ONLINE     = 1,	
	BFA_PORT_AEN_OFFLINE    = 2,	
	BFA_PORT_AEN_RLIR       = 3,	
	BFA_PORT_AEN_SFP_INSERT = 4,	
	BFA_PORT_AEN_SFP_REMOVE = 5,	
	BFA_PORT_AEN_SFP_POM    = 6,	
	BFA_PORT_AEN_ENABLE     = 7,	
	BFA_PORT_AEN_DISABLE    = 8,	
	BFA_PORT_AEN_AUTH_ON    = 9,	
	BFA_PORT_AEN_AUTH_OFF   = 10,	
	BFA_PORT_AEN_DISCONNECT = 11,	
	BFA_PORT_AEN_QOS_NEG    = 12,  	
	BFA_PORT_AEN_FABRIC_NAME_CHANGE = 13, 
	BFA_PORT_AEN_SFP_ACCESS_ERROR = 14, 
	BFA_PORT_AEN_SFP_UNSUPPORT = 15, 
};

enum bfa_port_aen_sfp_pom {
	BFA_PORT_AEN_SFP_POM_GREEN = 1,	
	BFA_PORT_AEN_SFP_POM_AMBER = 2,	
	BFA_PORT_AEN_SFP_POM_RED   = 3,	
	BFA_PORT_AEN_SFP_POM_MAX   = BFA_PORT_AEN_SFP_POM_RED
};

struct bfa_port_aen_data_s {
	enum bfa_ioc_type_e ioc_type;
	wwn_t           pwwn;	      
	wwn_t           fwwn;	      
	mac_t           mac;	      
	int             phy_port_num; 
	enum bfa_port_aen_sfp_pom level; 
};

#endif 
