

#ifndef __BFA_DEFS_PPORT_H__
#define __BFA_DEFS_PPORT_H__

#include <bfa_os_inc.h>
#include <protocol/fc.h>
#include <defs/bfa_defs_types.h>
#include <defs/bfa_defs_qos.h>
#include <cna/pstats/phyport_defs.h>


enum bfa_pport_states {
	BFA_PPORT_ST_UNINIT 		= 1,
	BFA_PPORT_ST_ENABLING_QWAIT 	= 2,
	BFA_PPORT_ST_ENABLING 		= 3,
	BFA_PPORT_ST_LINKDOWN 		= 4,
	BFA_PPORT_ST_LINKUP 		= 5,
	BFA_PPORT_ST_DISABLING_QWAIT 	= 6,
	BFA_PPORT_ST_DISABLING		= 7,
	BFA_PPORT_ST_DISABLED 		= 8,
	BFA_PPORT_ST_STOPPED 		= 9,
	BFA_PPORT_ST_IOCDOWN 		= 10,
	BFA_PPORT_ST_IOCDIS 		= 11,
	BFA_PPORT_ST_FWMISMATCH		= 12,
	BFA_PPORT_ST_MAX_STATE,
};


enum bfa_pport_speed {
	BFA_PPORT_SPEED_UNKNOWN = 0,
	BFA_PPORT_SPEED_1GBPS 	= 1,
	BFA_PPORT_SPEED_2GBPS 	= 2,
	BFA_PPORT_SPEED_4GBPS 	= 4,
	BFA_PPORT_SPEED_8GBPS 	= 8,
	BFA_PPORT_SPEED_10GBPS 	= 10,
	BFA_PPORT_SPEED_AUTO =
		(BFA_PPORT_SPEED_1GBPS | BFA_PPORT_SPEED_2GBPS |
		 BFA_PPORT_SPEED_4GBPS | BFA_PPORT_SPEED_8GBPS),
};


enum bfa_pport_type {
	BFA_PPORT_TYPE_UNKNOWN = 1,	
	BFA_PPORT_TYPE_TRUNKED = 2,	
	BFA_PPORT_TYPE_NPORT   = 5,	
	BFA_PPORT_TYPE_NLPORT  = 6,	
	BFA_PPORT_TYPE_LPORT   = 20,	
	BFA_PPORT_TYPE_P2P     = 21,	
	BFA_PPORT_TYPE_VPORT   = 22,	
};


enum bfa_pport_topology {
	BFA_PPORT_TOPOLOGY_NONE = 0,	
	BFA_PPORT_TOPOLOGY_P2P  = 1,	
	BFA_PPORT_TOPOLOGY_LOOP = 2,	
	BFA_PPORT_TOPOLOGY_AUTO = 3,	
};


enum bfa_pport_opmode {
	BFA_PPORT_OPMODE_NORMAL   = 0x00, 
	BFA_PPORT_OPMODE_LB_INT   = 0x01, 
	BFA_PPORT_OPMODE_LB_SLW   = 0x02, 
	BFA_PPORT_OPMODE_LB_EXT   = 0x04, 
	BFA_PPORT_OPMODE_LB_CBL   = 0x08, 
	BFA_PPORT_OPMODE_LB_NLINT = 0x20, 
};

#define BFA_PPORT_OPMODE_LB_HARD(_mode)			\
	((_mode == BFA_PPORT_OPMODE_LB_INT) ||		\
     (_mode == BFA_PPORT_OPMODE_LB_SLW) ||		\
     (_mode == BFA_PPORT_OPMODE_LB_EXT))


enum bfa_pport_snia_state {
	BFA_PPORT_STATE_UNKNOWN  = 1,	
	BFA_PPORT_STATE_ONLINE   = 2,	
	BFA_PPORT_STATE_DISABLED = 3,	
	BFA_PPORT_STATE_BYPASSED = 4,	
	BFA_PPORT_STATE_DIAG     = 5,	
	BFA_PPORT_STATE_LINKDOWN = 6,	
	BFA_PPORT_STATE_LOOPBACK = 8,	
};


enum bfa_pport_linkstate {
	BFA_PPORT_LINKUP 	 = 1,	
	BFA_PPORT_LINKDOWN 	 = 2,	
	BFA_PPORT_TRUNK_LINKDOWN = 3,	
};


#define bfa_pport_event_t enum bfa_pport_linkstate


enum bfa_pport_linkstate_rsn {
	BFA_PPORT_LINKSTATE_RSN_NONE		= 0,
	BFA_PPORT_LINKSTATE_RSN_DISABLED 	= 1,
	BFA_PPORT_LINKSTATE_RSN_RX_NOS 		= 2,
	BFA_PPORT_LINKSTATE_RSN_RX_OLS 		= 3,
	BFA_PPORT_LINKSTATE_RSN_RX_LIP 		= 4,
	BFA_PPORT_LINKSTATE_RSN_RX_LIPF7 	= 5,
	BFA_PPORT_LINKSTATE_RSN_SFP_REMOVED 	= 6,
	BFA_PPORT_LINKSTATE_RSN_PORT_FAULT 	= 7,
	BFA_PPORT_LINKSTATE_RSN_RX_LOS 		= 8,
	BFA_PPORT_LINKSTATE_RSN_LOCAL_FAULT 	= 9,
	BFA_PPORT_LINKSTATE_RSN_REMOTE_FAULT 	= 10,
	BFA_PPORT_LINKSTATE_RSN_TIMEOUT 	= 11,



	
	CEE_LLDP_INFO_AGED_OUT       = 20,
	CEE_LLDP_SHUTDOWN_TLV_RCVD   = 21,
	CEE_PEER_NOT_ADVERTISE_DCBX  = 22,
	CEE_PEER_NOT_ADVERTISE_PG    = 23,
	CEE_PEER_NOT_ADVERTISE_PFC   = 24,
	CEE_PEER_NOT_ADVERTISE_FCOE  = 25,
	CEE_PG_NOT_COMPATIBLE        = 26,
	CEE_PFC_NOT_COMPATIBLE       = 27,
	CEE_FCOE_NOT_COMPATIBLE      = 28,
	CEE_BAD_PG_RCVD              = 29,
	CEE_BAD_BW_RCVD              = 30,
	CEE_BAD_PFC_RCVD             = 31,
	CEE_BAD_FCOE_PRI_RCVD        = 32,
	CEE_FCOE_PRI_PFC_OFF         = 33,
	CEE_DUP_CONTROL_TLV_RCVD     = 34,
	CEE_DUP_FEAT_TLV_RCVD        = 35,
	CEE_APPLY_NEW_CFG            = 36,	
	CEE_PROTOCOL_INIT            = 37,  
	CEE_PHY_LINK_DOWN            = 38,
	CEE_LLS_FCOE_ABSENT          = 39,
	CEE_LLS_FCOE_DOWN            = 40
};


#define BFA_PPORT_DEF_TRL_SPEED  BFA_PPORT_SPEED_1GBPS


struct bfa_pport_cfg_s {
	u8         topology;	
	u8         speed;		
	u8         trunked;	
	u8         qos_enabled;	
	u8         trunk_ports;	
	u8         cfg_hardalpa;	
	u16        maxfrsize;	
	u8         hardalpa;	
	u8         rx_bbcredit;	
	u8         tx_bbcredit;	
	u8         ratelimit;	
	u8         trl_def_speed;	
	u8		rsvd[3];
	u16   	path_tov;	
	u16   	q_depth;	
};


struct bfa_pport_attr_s {
	
	wwn_t           nwwn;		
	wwn_t           pwwn;		
	enum fc_cos     cos_supported;	
	u32        rsvd;
	struct fc_symname_s    port_symname;	
	enum bfa_pport_speed speed_supported; 
	bfa_boolean_t   pbind_enabled;	

	
	struct bfa_pport_cfg_s pport_cfg;	

	
	enum bfa_pport_states 	port_state;	
	enum bfa_pport_speed 	speed;		
	enum bfa_pport_topology 	topology;	
	bfa_boolean_t		beacon;		
	bfa_boolean_t		link_e2e_beacon;
	bfa_boolean_t		plog_enabled;	

	
	u32        	pid;		
	enum bfa_pport_type 	port_type;	
	u32        	loopback;	
	u32		rsvd1;
	u32		rsvd2;		
};


struct bfa_pport_fc_stats_s {
	u64        secs_reset;	
	u64        tx_frames;	
	u64        tx_words;	
	u64        rx_frames;	
	u64        rx_words;	
	u64        lip_count;	
	u64        nos_count;	
	u64        error_frames;	
	u64        dropped_frames;	
	u64        link_failures;	
	u64        loss_of_syncs;	
	u64        loss_of_signals;
	u64        primseq_errs;	
	u64        bad_os_count;	
	u64        err_enc_out;	
	u64        invalid_crcs;	
	u64	undersized_frm; 
	u64	oversized_frm;	
	u64	bad_eof_frm;	
	struct bfa_qos_stats_s	qos_stats;	
};


struct bfa_pport_eth_stats_s {
	u64	secs_reset;	
	u64	frame_64;      
	u64	frame_65_127;      
	u64	frame_128_255;     
	u64	frame_256_511;     
	u64	frame_512_1023;    
	u64	frame_1024_1518;   
	u64	frame_1519_1522;   

	u64	tx_bytes;
	u64	tx_packets;
	u64	tx_mcast_packets;
	u64	tx_bcast_packets;
	u64	tx_control_frame;
	u64	tx_drop;
	u64	tx_jabber;
	u64	tx_fcs_error;
	u64	tx_fragments;

	u64	rx_bytes;
	u64	rx_packets;
	u64	rx_mcast_packets;
	u64	rx_bcast_packets;
	u64	rx_control_frames;
	u64	rx_unknown_opcode;
	u64	rx_drop;
	u64	rx_jabber;
	u64	rx_fcs_error;
	u64	rx_alignment_error;
	u64	rx_frame_length_error;
	u64	rx_code_error;
	u64	rx_fragments;

	u64	rx_pause; 
	u64	rx_zero_pause; 
	u64	tx_pause;      
	u64	tx_zero_pause; 
	u64	rx_fcoe_pause; 
	u64	rx_fcoe_zero_pause; 
	u64	tx_fcoe_pause;      
	u64	tx_fcoe_zero_pause; 
};


union bfa_pport_stats_u {
	struct bfa_pport_fc_stats_s	fc;
	struct bfa_pport_eth_stats_s 	eth;
};


struct bfa_pport_fcpmap_s {
	char		osdevname[256];
	u32	bus;
	u32        target;
	u32        oslun;
	u32        fcid;
	wwn_t           nwwn;
	wwn_t           pwwn;
	u64        fcplun;
	char		luid[256];
};


struct bfa_pport_rnid_s {
	wwn_t             wwn;
	u32          unittype;
	u32          portid;
	u32          attached_nodes_num;
	u16          ip_version;
	u16          udp_port;
	u8           ipaddr[16];
	u16          rsvd;
	u16          topologydiscoveryflags;
};


struct bfa_pport_link_s {
	u8         linkstate;	
	u8         linkstate_rsn;	
	u8         topology;	
	u8         speed;		
	u32        linkstate_opt;	
	u8         trunked;	
	u8         resvd[3];
	struct bfa_qos_attr_s  qos_attr;   
	struct bfa_qos_vc_attr_s qos_vc_attr;  
	union {
		struct {
			u8         tmaster;
			u8         tlinks;	
			u8         resv1;	
		} trunk_info;

		struct {
			u8         myalpa;	   
			u8         login_req; 
			u8         alpabm_val;
			struct fc_alpabm_s     alpabm;	   
		} loop_info;
	} tl;
};

#endif 
