

#ifndef __CT_H__
#define __CT_H__

#include <protocol/types.h>

#pragma pack(1)

struct ct_hdr_s{
	u32	rev_id:8;	
	u32	in_id:24;	
	u32	gs_type:8;	
	u32	gs_sub_type:8;	
	u32	options:8;	
	u32	rsvrd:8;	
	u32	cmd_rsp_code:16;
	u32	max_res_size:16;
	u32	frag_id:8;	
	u32	reason_code:8;	
	u32	exp_code:8;	
	u32	vendor_unq:8;	
};


enum {
	CT_GS3_REVISION = 0x01,
};


enum {
	CT_GSTYPE_KEYSERVICE	= 0xF7,
	CT_GSTYPE_ALIASSERVICE	= 0xF8,
	CT_GSTYPE_MGMTSERVICE	= 0xFA,
	CT_GSTYPE_TIMESERVICE	= 0xFB,
	CT_GSTYPE_DIRSERVICE	= 0xFC,
};


enum {
	CT_GSSUBTYPE_NAMESERVER = 0x02,
};


enum {
	CT_GSSUBTYPE_CFGSERVER	= 0x01,
	CT_GSSUBTYPE_UNZONED_NS = 0x02,
	CT_GSSUBTYPE_ZONESERVER = 0x03,
	CT_GSSUBTYPE_LOCKSERVER = 0x04,
	CT_GSSUBTYPE_HBA_MGMTSERVER = 0x10,	
};


enum {
	CT_RSP_REJECT = 0x8001,
	CT_RSP_ACCEPT = 0x8002,
};


enum {
	CT_RSN_INV_CMD		= 0x01,
	CT_RSN_INV_VER		= 0x02,
	CT_RSN_LOGIC_ERR	= 0x03,
	CT_RSN_INV_SIZE		= 0x04,
	CT_RSN_LOGICAL_BUSY	= 0x05,
	CT_RSN_PROTO_ERR	= 0x07,
	CT_RSN_UNABLE_TO_PERF	= 0x09,
	CT_RSN_NOT_SUPP			= 0x0B,
	CT_RSN_SERVER_NOT_AVBL  = 0x0D,
	CT_RSN_SESSION_COULD_NOT_BE_ESTBD = 0x0E,
	CT_RSN_VENDOR_SPECIFIC  = 0xFF,

};


enum {
	CT_NS_EXP_NOADDITIONAL	= 0x00,
	CT_NS_EXP_ID_NOT_REG	= 0x01,
	CT_NS_EXP_PN_NOT_REG	= 0x02,
	CT_NS_EXP_NN_NOT_REG	= 0x03,
	CT_NS_EXP_CS_NOT_REG	= 0x04,
	CT_NS_EXP_IPN_NOT_REG	= 0x05,
	CT_NS_EXP_IPA_NOT_REG	= 0x06,
	CT_NS_EXP_FT_NOT_REG	= 0x07,
	CT_NS_EXP_SPN_NOT_REG	= 0x08,
	CT_NS_EXP_SNN_NOT_REG	= 0x09,
	CT_NS_EXP_PT_NOT_REG	= 0x0A,
	CT_NS_EXP_IPP_NOT_REG	= 0x0B,
	CT_NS_EXP_FPN_NOT_REG	= 0x0C,
	CT_NS_EXP_HA_NOT_REG	= 0x0D,
	CT_NS_EXP_FD_NOT_REG	= 0x0E,
	CT_NS_EXP_FF_NOT_REG	= 0x0F,
	CT_NS_EXP_ACCESSDENIED	= 0x10,
	CT_NS_EXP_UNACCEPTABLE_ID = 0x11,
	CT_NS_EXP_DATABASEEMPTY			= 0x12,
	CT_NS_EXP_NOT_REG_IN_SCOPE 		= 0x13,
	CT_NS_EXP_DOM_ID_NOT_PRESENT 	= 0x14,
	CT_NS_EXP_PORT_NUM_NOT_PRESENT  = 0x15,
	CT_NS_EXP_NO_DEVICE_ATTACHED 	= 0x16
};


enum {
	CT_EXP_AUTH_EXCEPTION			= 0xF1,
	CT_EXP_DB_FULL					= 0xF2,
	CT_EXP_DB_EMPTY					= 0xF3,
	CT_EXP_PROCESSING_REQ			= 0xF4,
	CT_EXP_UNABLE_TO_VERIFY_CONN	= 0xF5,
	CT_EXP_DEVICES_NOT_IN_CMN_ZONE  = 0xF6
};


enum {
	GS_GID_PN	= 0x0121,	
	GS_GPN_ID	= 0x0112,	
	GS_GNN_ID	= 0x0113,	
	GS_GID_FT	= 0x0171,	
	GS_GSPN_ID	= 0x0118,	
	GS_RFT_ID	= 0x0217,	
	GS_RSPN_ID	= 0x0218,	
	GS_RPN_ID	= 0x0212,	
	GS_RNN_ID	= 0x0213,	
	GS_RCS_ID	= 0x0214,	
	GS_RPT_ID	= 0x021A,	
	GS_GA_NXT	= 0x0100,	
	GS_RFF_ID	= 0x021F,	
};

struct fcgs_id_req_s{
	u32	rsvd:8;
	u32	dap:24;	
};
#define fcgs_gpnid_req_t struct fcgs_id_req_s
#define fcgs_gnnid_req_t struct fcgs_id_req_s
#define fcgs_gspnid_req_t struct fcgs_id_req_s

struct fcgs_gidpn_req_s{
	wwn_t	port_name;	
};

struct fcgs_gidpn_resp_s{
	u32	rsvd:8;
	u32	dap:24;	
};


struct fcgs_rftid_req_s {
	u32	rsvd:8;
	u32	dap:24;		
	u32	fc4_type[8];	
};



#define FC_GS_FCP_FC4_FEATURE_INITIATOR  0x02
#define FC_GS_FCP_FC4_FEATURE_TARGET	 0x01

struct fcgs_rffid_req_s{
    u32    rsvd          :8;
    u32    dap        	  :24;		
    u32    rsvd1         :16;
    u32    fc4ftr_bits   :8;		
    u32    fc4_type      :8;		
};


struct fcgs_gidft_req_s{
	u8	reserved;
	u8	domain_id;	
	u8	area_id;	
	u8	fc4_type;	
};				


struct fcgs_gidft_resp_s {
	u8		last:1;	
	u8		reserved:7;
	u32	pid:24;	
};				


struct fcgs_rspnid_req_s{
	u32	rsvd:8;
	u32	dap:24;		
	u8		spn_len;	
	u8		spn[256];	
};


struct fcgs_rpnid_req_s{
	u32	rsvd:8;
	u32	port_id:24;
	wwn_t		port_name;
};


struct fcgs_rnnid_req_s{
	u32	rsvd:8;
	u32	port_id:24;
	wwn_t		node_name;
};


struct fcgs_rcsid_req_s{
	u32	rsvd:8;
	u32	port_id:24;
	u32	cos;
};


struct fcgs_rptid_req_s{
	u32	rsvd:8;
	u32	port_id:24;
	u32	port_type:8;
	u32	rsvd1:24;
};


struct fcgs_ganxt_req_s{
	u32	rsvd:8;
	u32	port_id:24;
};


struct fcgs_ganxt_rsp_s{
	u32	port_type:8;	
	u32	port_id:24;	
	wwn_t		port_name;	
	u8		spn_len;	
	char		spn[255];	
	wwn_t		node_name;	
	u8		snn_len;	
	char		snn[255];	
	u8		ipa[8];		
	u8		ip[16];		
	u32	cos;		
	u32	fc4types[8];	
	wwn_t		fabric_port_name;
					
	u32	rsvd:8;		
	u32	hard_addr:24;	
};




enum {
	GS_FC_GFN_CMD	= 0x0114,	
	GS_FC_GMAL_CMD	= 0x0116,	
	GS_FC_TRACE_CMD	= 0x0400,	
	GS_FC_PING_CMD	= 0x0401,	
};


enum {
	GS_FTRACE_TAG_NPORT_ID		= 1,
	GS_FTRACE_TAG_NPORT_NAME	= 2,
};


union fcgs_port_val_u{
	u32	nport_id;
	wwn_t		nport_wwn;
};

#define GS_FTRACE_MAX_HOP_COUNT	20
#define GS_FTRACE_REVISION	1




enum {
	GS_FTRACE_STR_CMD_COMPLETED_SUCC	= 0,
	GS_FTRACE_STR_CMD_NOT_SUPP_IN_NEXT_SWITCH,
	GS_FTRACE_STR_NO_RESP_FROM_NEXT_SWITCH,
	GS_FTRACE_STR_MAX_HOP_CNT_REACHED,
	GS_FTRACE_STR_SRC_PORT_NOT_FOUND,
	GS_FTRACE_STR_DST_PORT_NOT_FOUND,
	GS_FTRACE_STR_DEVICES_NOT_IN_COMMON_ZONE,
	GS_FTRACE_STR_NO_ROUTE_BW_PORTS,
	GS_FTRACE_STR_NO_ADDL_EXPLN,
	GS_FTRACE_STR_FABRIC_BUSY,
	GS_FTRACE_STR_FABRIC_BUILD_IN_PROGRESS,
	GS_FTRACE_STR_VENDOR_SPECIFIC_ERR_START = 0xf0,
	GS_FTRACE_STR_VENDOR_SPECIFIC_ERR_END = 0xff,
};


struct fcgs_ftrace_req_s{
	u32	revision;
	u16	src_port_tag;	
	u16	src_port_len;	
	union fcgs_port_val_u src_port_val;	
	u16	dst_port_tag;	
	u16	dst_port_len;	
	union fcgs_port_val_u dst_port_val;	
	u32	token;
	u8		vendor_id[8];	
	u8		vendor_info[8];	
	u32	max_hop_cnt;	
};


struct fcgs_ftrace_path_info_s{
	wwn_t		switch_name;		
	u32	domain_id;
	wwn_t		ingress_port_name;	
	u32	ingress_phys_port_num;	
	wwn_t		egress_port_name;	
	u32	egress_phys_port_num;	
};


struct fcgs_ftrace_resp_s{
	u32	revision;
	u32	token;
	u8		vendor_id[8];		
	u8		vendor_info[8];		
	u32	str_rej_reason_code;	
	u32	num_path_info_entries;	
	
	struct fcgs_ftrace_path_info_s path_info[1];

};




struct fcgs_fcping_req_s{
	u32	revision;
	u16	port_tag;
	u16	port_len;	
	union fcgs_port_val_u port_val;	
	u32	token;
};


struct fcgs_fcping_resp_s{
	u32	token;
};


enum {
	ZS_GZME = 0x0124,	
};


#define ZS_GZME_ZNAMELEN	32
struct zs_gzme_req_s{
	u8	znamelen;
	u8	rsvd[3];
	u8	zname[ZS_GZME_ZNAMELEN];
};

enum zs_mbr_type{
	ZS_MBR_TYPE_PWWN	= 1,
	ZS_MBR_TYPE_DOMPORT	= 2,
	ZS_MBR_TYPE_PORTID	= 3,
	ZS_MBR_TYPE_NWWN	= 4,
};

struct zs_mbr_wwn_s{
	u8	mbr_type;
	u8	rsvd[3];
	wwn_t	wwn;
};

struct zs_query_resp_s{
	u32	nmbrs;	
	struct zs_mbr_wwn_s	mbr[1];
};



#define CT_GMAL_RESP_PREFIX_TELNET	 "telnet://"
#define CT_GMAL_RESP_PREFIX_HTTP	 "http://"


struct fcgs_req_s {
	wwn_t    wwn; 	
};

#define fcgs_gmal_req_t struct fcgs_req_s
#define fcgs_gfn_req_t struct fcgs_req_s


struct fcgs_gmal_resp_s {
	u32 		ms_len;   
	u8     	ms_ma[256];
};

struct fc_gmal_entry_s {
	u8  len;
	u8  prefix[7]; 
	u8  ip_addr[248];
};

#pragma pack()

#endif
