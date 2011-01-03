

#ifndef __FC_H__
#define __FC_H__

#include <protocol/types.h>

#pragma pack(1)


struct fchs_s {
#ifdef __BIGENDIAN
	u32        routing:4;	
	u32        cat_info:4;	
#else
	u32        cat_info:4;	
	u32        routing:4;	
#endif
	u32        d_id:24;	

	u32        cs_ctl:8;	
	u32        s_id:24;	

	u32        type:8;		
	u32        f_ctl:24;	

	u8         seq_id;		
	u8         df_ctl;		
	u16        seq_cnt;	

	u16        ox_id;		
	u16        rx_id;		

	u32        ro;		
};

struct fcbbehs_s {
	u16	ver_rsvd;
	u32	rsvd[2];
	u32	rsvd__sof;
};

#define FC_SEQ_ID_MAX		256


enum {
	FC_RTG_FC4_DEV_DATA	= 0x0,	
	FC_RTG_EXT_LINK		= 0x2,	
	FC_RTG_FC4_LINK_DATA	= 0x3,	
	FC_RTG_VIDEO_DATA	= 0x4,	
	FC_RTG_EXT_HDR		= 0x5,	
	FC_RTG_BASIC_LINK	= 0x8,	
	FC_RTG_LINK_CTRL	= 0xC,	
};


enum {
	FC_CAT_LD_REQUEST	= 0x2,	
	FC_CAT_LD_REPLY		= 0x3,	
	FC_CAT_LD_DIAG		= 0xF,	
};


enum {
	FC_CAT_VFT_HDR = 0x0,	
	FC_CAT_IFR_HDR = 0x1,	
	FC_CAT_ENC_HDR = 0x2,	
};


enum {
	FC_CAT_UNCATEG_INFO	= 0x0,	
	FC_CAT_SOLICIT_DATA	= 0x1,	
	FC_CAT_UNSOLICIT_CTRL	= 0x2,	
	FC_CAT_SOLICIT_CTRL	= 0x3,	
	FC_CAT_UNSOLICIT_DATA	= 0x4,	
	FC_CAT_DATA_DESC	= 0x5,	
	FC_CAT_UNSOLICIT_CMD	= 0x6,	
	FC_CAT_CMD_STATUS	= 0x7,	
};


enum {
	FC_CAT_ACK_1		= 0x00,
	FC_CAT_ACK_0_N		= 0x01,
	FC_CAT_P_RJT		= 0x02,
	FC_CAT_F_RJT		= 0x03,
	FC_CAT_P_BSY		= 0x04,
	FC_CAT_F_BSY_DATA	= 0x05,
	FC_CAT_F_BSY_LINK_CTL	= 0x06,
	FC_CAT_F_LCR		= 0x07,
	FC_CAT_NTY		= 0x08,
	FC_CAT_END		= 0x09,
};


enum {
	FC_TYPE_BLS		= 0x0,	
	FC_TYPE_ELS		= 0x1,	
	FC_TYPE_IP		= 0x5,	
	FC_TYPE_FCP		= 0x8,	
	FC_TYPE_GPP		= 0x9,	
	FC_TYPE_SERVICES	= 0x20,	
	FC_TYPE_FC_FSS		= 0x22,	
	FC_TYPE_FC_AL		= 0x23,	
	FC_TYPE_FC_SNMP		= 0x24,	
	FC_TYPE_MAX		= 256,	
};

struct fc_fc4types_s{
	u8         bits[FC_TYPE_MAX / 8];
};


enum {
	FCTL_EC_ORIG = 0x000000,	
	FCTL_EC_RESP = 0x800000,	
	FCTL_SEQ_INI = 0x000000,	
	FCTL_SEQ_REC = 0x400000,	
	FCTL_FS_EXCH = 0x200000,	
	FCTL_LS_EXCH = 0x100000,	
	FCTL_END_SEQ = 0x080000,	
	FCTL_SI_XFER = 0x010000,	
	FCTL_RO_PRESENT = 0x000008,	
	FCTL_FILLBYTE_MASK = 0x000003	
};


enum {
	FC_MIN_WELL_KNOWN_ADDR		= 0xFFFFF0,
	FC_DOMAIN_CONTROLLER_MASK 	= 0xFFFC00,
	FC_ALIAS_SERVER			= 0xFFFFF8,
	FC_MGMT_SERVER			= 0xFFFFFA,
	FC_TIME_SERVER			= 0xFFFFFB,
	FC_NAME_SERVER			= 0xFFFFFC,
	FC_FABRIC_CONTROLLER		= 0xFFFFFD,
	FC_FABRIC_PORT			= 0xFFFFFE,
	FC_BROADCAST_SERVER		= 0xFFFFFF
};


#define FC_DOMAIN_MASK  0xFF0000
#define FC_DOMAIN_SHIFT 16
#define FC_AREA_MASK    0x00FF00
#define FC_AREA_SHIFT   8
#define FC_PORT_MASK    0x0000FF
#define FC_PORT_SHIFT   0

#define FC_GET_DOMAIN(p)	(((p) & FC_DOMAIN_MASK) >> FC_DOMAIN_SHIFT)
#define FC_GET_AREA(p)		(((p) & FC_AREA_MASK) >> FC_AREA_SHIFT)
#define FC_GET_PORT(p)		(((p) & FC_PORT_MASK) >> FC_PORT_SHIFT)

#define FC_DOMAIN_CTRLR(p)	(FC_DOMAIN_CONTROLLER_MASK | (FC_GET_DOMAIN(p)))

enum {
	FC_RXID_ANY = 0xFFFFU,
};


struct fc_els_cmd_s{
	u32        els_code:8;	
	u32        reserved:24;
};


enum {
	FC_ELS_LS_RJT = 0x1,	
	FC_ELS_ACC = 0x02,	
	FC_ELS_PLOGI = 0x03,	
	FC_ELS_FLOGI = 0x04,	
	FC_ELS_LOGO = 0x05,	
	FC_ELS_ABTX = 0x06,	
	FC_ELS_RES = 0x08,	
	FC_ELS_RSS = 0x09,	
	FC_ELS_RSI = 0x0A,	
	FC_ELS_ESTC = 0x0C,	
	FC_ELS_RTV = 0x0E,	
	FC_ELS_RLS = 0x0F,	
	FC_ELS_ECHO = 0x10,	
	FC_ELS_TEST = 0x11,	
	FC_ELS_RRQ = 0x12,	
	FC_ELS_REC = 0x13,	
	FC_ELS_PRLI = 0x20,	
	FC_ELS_PRLO = 0x21,	
	FC_ELS_SCN = 0x22,	
	FC_ELS_TPRLO = 0x24,	
	FC_ELS_PDISC = 0x50,	
	FC_ELS_FDISC = 0x51,	
	FC_ELS_ADISC = 0x52,	
	FC_ELS_FAN = 0x60,	
	FC_ELS_RSCN = 0x61,	
	FC_ELS_SCR = 0x62,	
	FC_ELS_RTIN = 0x77,	
	FC_ELS_RNID = 0x78,	
	FC_ELS_RLIR = 0x79,	

	FC_ELS_RPSC = 0x7D,	
	FC_ELS_QSA = 0x7E,	
	FC_ELS_E2E_LBEACON = 0x81,
				
	FC_ELS_AUTH = 0x90,	
	FC_ELS_RFCN = 0x97,	

};


enum {
	FC_PH_VER_4_3 = 0x09,
	FC_PH_VER_PH_3 = 0x20,
};


enum {
	FC_MIN_PDUSZ = 512,
	FC_MAX_PDUSZ = 2112,
};


struct fc_plogi_csp_s{
	u8         verhi;	
	u8         verlo;	
	u16        bbcred;	

#ifdef __BIGENDIAN
	u8         ciro:1,		
			rro:1,		
			npiv_supp:1,	
			port_type:1,	
			altbbcred:1,	
			resolution:1,	
			vvl_info:1,	
			reserved1:1;

	u8         hg_supp:1,
			query_dbc:1,
			security:1,
			sync_cap:1,
			r_t_tov:1,
			dh_dup_supp:1,
			cisc:1,		
			payload:1;
#else
	u8         reserved2:2,
			resolution:1,	
			altbbcred:1,	
			port_type:1,	
			npiv_supp:1,	
			rro:1,		
			ciro:1;		

	u8         payload:1,
			cisc:1,		
			dh_dup_supp:1,
			r_t_tov:1,
			sync_cap:1,
			security:1,
			query_dbc:1,
			hg_supp:1;
#endif

	u16        rxsz;		

	u16        conseq;
	u16        ro_bitmap;

	u32        e_d_tov;
};


struct fc_plogi_clp_s{
#ifdef __BIGENDIAN
	u32        class_valid:1;
	u32        intermix:1;	
	u32        reserved1:2;
	u32        sequential:1;
	u32        reserved2:3;
#else
	u32        reserved2:3;
	u32        sequential:1;
	u32        reserved1:2;
	u32        intermix:1;	
	u32        class_valid:1;
#endif

	u32        reserved3:24;

	u32        reserved4:16;
	u32        rxsz:16;	

	u32        reserved5:8;
	u32        conseq:8;
	u32        e2e_credit:16;	

	u32        reserved7:8;
	u32        ospx:8;
	u32        reserved8:16;
};

#define FLOGI_VVL_BRCD    0x42524344 


struct fc_logi_s{
	struct fc_els_cmd_s els_cmd;	
	struct fc_plogi_csp_s  csp;		
	wwn_t           port_name;
	wwn_t           node_name;
	struct fc_plogi_clp_s  class1;		
	struct fc_plogi_clp_s  class2;		
	struct fc_plogi_clp_s  class3;		
	struct fc_plogi_clp_s  class4;		
	u8         vvl[16];	
};


struct fc_logo_s{
	struct fc_els_cmd_s    els_cmd;	
	u32        res1:8;
	u32        nport_id:24;	
	wwn_t           orig_port_name;	
};


struct fc_adisc_s {
	struct fc_els_cmd_s    els_cmd;	
	u32        res1:8;
	u32        orig_HA:24;	
	wwn_t           orig_port_name;	
	wwn_t           orig_node_name;	
	u32        res2:8;
	u32        nport_id:24;	
};


struct fc_exch_status_blk_s{
	u32        oxid:16;
	u32        rxid:16;
	u32        res1:8;
	u32        orig_np:24;	
	u32        res2:8;
	u32        resp_np:24;	
	u32        es_bits;
	u32        res3;
	
};


struct fc_res_s {
	struct fc_els_cmd_s    els_cmd;	
	u32        res1:8;
	u32        nport_id:24;	
	u32        oxid:16;
	u32        rxid:16;
	u8         assoc_hdr[32];
};


struct fc_res_acc_s{
	struct fc_els_cmd_s els_cmd;	
	struct fc_exch_status_blk_s fc_exch_blk; 
};


struct fc_rec_s {
	struct fc_els_cmd_s    els_cmd;	
	u32        res1:8;
	u32        nport_id:24;	
	u32        oxid:16;
	u32        rxid:16;
};

#define FC_REC_ESB_OWN_RSP	0x80000000	
#define FC_REC_ESB_SI		0x40000000	
#define FC_REC_ESB_COMP		0x20000000	
#define FC_REC_ESB_ENDCOND_ABN	0x10000000	
#define FC_REC_ESB_RQACT	0x04000000	
#define FC_REC_ESB_ERRP_MSK	0x03000000
#define FC_REC_ESB_OXID_INV	0x00800000	
#define FC_REC_ESB_RXID_INV	0x00400000	
#define FC_REC_ESB_PRIO_INUSE	0x00200000


struct fc_rec_acc_s {
	struct fc_els_cmd_s    els_cmd;	
	u32        oxid:16;
	u32        rxid:16;
	u32        res1:8;
	u32        orig_id:24;	
	u32        res2:8;
	u32        resp_id:24;	
	u32        count;		
	u32        e_stat;		
};


struct fc_rsi_s {
	struct fc_els_cmd_s    els_cmd;
	u32        res1:8;
	u32        orig_sid:24;
	u32        oxid:16;
	u32        rxid:16;
};


struct fc_prli_params_s{
	u32        reserved: 16;
#ifdef __BIGENDIAN
	u32        reserved1: 5;
	u32        rec_support : 1;
	u32        task_retry_id : 1;
	u32        retry : 1;

	u32        confirm : 1;
	u32        doverlay:1;
	u32        initiator:1;
	u32        target:1;
	u32        cdmix:1;
	u32        drmix:1;
	u32        rxrdisab:1;
	u32        wxrdisab:1;
#else
	u32        retry : 1;
	u32        task_retry_id : 1;
	u32        rec_support : 1;
	u32        reserved1: 5;

	u32        wxrdisab:1;
	u32        rxrdisab:1;
	u32        drmix:1;
	u32        cdmix:1;
	u32        target:1;
	u32        initiator:1;
	u32        doverlay:1;
	u32        confirm : 1;
#endif
};


enum {
	FC_PRLI_ACC_XQTD = 0x1,		
	FC_PRLI_ACC_PREDEF_IMG = 0x5,	
};

struct fc_prli_params_page_s{
	u32        type:8;
	u32        codext:8;
#ifdef __BIGENDIAN
	u32        origprocasv:1;
	u32        rsppav:1;
	u32        imagepair:1;
	u32        reserved1:1;
	u32        rspcode:4;
#else
	u32        rspcode:4;
	u32        reserved1:1;
	u32        imagepair:1;
	u32        rsppav:1;
	u32        origprocasv:1;
#endif
	u32        reserved2:8;

	u32        origprocas;
	u32        rspprocas;
	struct fc_prli_params_s  servparams;
};


struct fc_prli_s{
	u32        command:8;
	u32        pglen:8;
	u32        pagebytes:16;
	struct fc_prli_params_page_s parampage;
};


struct fc_prlo_params_page_s{
	u32        type:8;
	u32        type_ext:8;
#ifdef __BIGENDIAN
	u32        opa_valid:1;	
	u32        rpa_valid:1;	
	u32        res1:14;
#else
	u32        res1:14;
	u32        rpa_valid:1;	
	u32        opa_valid:1;	
#endif
	u32        orig_process_assc;
	u32        resp_process_assc;

	u32        res2;
};


struct fc_prlo_s{
	u32        	command:8;
	u32        	page_len:8;
	u32        	payload_len:16;
	struct fc_prlo_params_page_s 	prlo_params[1];
};


struct fc_prlo_acc_params_page_s{
	u32        type:8;
	u32        type_ext:8;

#ifdef __BIGENDIAN
	u32        opa_valid:1;	
	u32        rpa_valid:1;	
	u32        res1:14;
#else
	u32        res1:14;
	u32        rpa_valid:1;	
	u32        opa_valid:1;	
#endif
	u32        orig_process_assc;
	u32        resp_process_assc;

	u32        fc4type_csp;
};


struct fc_prlo_acc_s{
	u32        command:8;
	u32        page_len:8;
	u32        payload_len:16;
	struct fc_prlo_acc_params_page_s prlo_acc_params[1];
};


enum {
	FC_SCR_REG_FUNC_FABRIC_DETECTED = 0x01,
	FC_SCR_REG_FUNC_N_PORT_DETECTED = 0x02,
	FC_SCR_REG_FUNC_FULL = 0x03,
	FC_SCR_REG_FUNC_CLEAR_REG = 0xFF,
};


enum {
	FC_VU_SCR_REG_FUNC_FABRIC_NAME_CHANGE = 0x01
};

struct fc_scr_s{
	u32 command:8;
	u32 res:24;
	u32 vu_reg_func:8; 
	u32 res1:16;
	u32 reg_func:8;
};


enum {
	FC_CAT_NOP	= 0x0,
	FC_CAT_ABTS	= 0x1,
	FC_CAT_RMC	= 0x2,
	FC_CAT_BA_ACC	= 0x4,
	FC_CAT_BA_RJT	= 0x5,
	FC_CAT_PRMT	= 0x6,
};


struct fc_ls_rjt_s {
	struct fc_els_cmd_s    els_cmd;		
	u32        res1:8;
	u32        reason_code:8;		
	u32        reason_code_expl:8;	
	u32        vendor_unique:8;	
};


enum {
	FC_LS_RJT_RSN_INV_CMD_CODE	= 0x01,
	FC_LS_RJT_RSN_LOGICAL_ERROR	= 0x03,
	FC_LS_RJT_RSN_LOGICAL_BUSY	= 0x05,
	FC_LS_RJT_RSN_PROTOCOL_ERROR	= 0x07,
	FC_LS_RJT_RSN_UNABLE_TO_PERF_CMD = 0x09,
	FC_LS_RJT_RSN_CMD_NOT_SUPP	= 0x0B,
};


enum {
	FC_LS_RJT_EXP_NO_ADDL_INFO		= 0x00,
	FC_LS_RJT_EXP_SPARMS_ERR_OPTIONS	= 0x01,
	FC_LS_RJT_EXP_SPARMS_ERR_INI_CTL	= 0x03,
	FC_LS_RJT_EXP_SPARMS_ERR_REC_CTL	= 0x05,
	FC_LS_RJT_EXP_SPARMS_ERR_RXSZ		= 0x07,
	FC_LS_RJT_EXP_SPARMS_ERR_CONSEQ		= 0x09,
	FC_LS_RJT_EXP_SPARMS_ERR_CREDIT		= 0x0B,
	FC_LS_RJT_EXP_INV_PORT_NAME		= 0x0D,
	FC_LS_RJT_EXP_INV_NODE_FABRIC_NAME	= 0x0E,
	FC_LS_RJT_EXP_INV_CSP			= 0x0F,
	FC_LS_RJT_EXP_INV_ASSOC_HDR		= 0x11,
	FC_LS_RJT_EXP_ASSOC_HDR_REQD		= 0x13,
	FC_LS_RJT_EXP_INV_ORIG_S_ID		= 0x15,
	FC_LS_RJT_EXP_INV_OXID_RXID_COMB	= 0x17,
	FC_LS_RJT_EXP_CMD_ALREADY_IN_PROG	= 0x19,
	FC_LS_RJT_EXP_LOGIN_REQUIRED		= 0x1E,
	FC_LS_RJT_EXP_INVALID_NPORT_ID		= 0x1F,
	FC_LS_RJT_EXP_INSUFF_RES		= 0x29,
	FC_LS_RJT_EXP_CMD_NOT_SUPP		= 0x2C,
	FC_LS_RJT_EXP_INV_PAYLOAD_LEN		= 0x2D,
};


struct fc_rrq_s{
	struct fc_els_cmd_s    els_cmd;	
	u32        res1:8;
	u32        s_id:24;	

	u32        ox_id:16;	
	u32        rx_id:16;	

	u32        res2[8];	
};


struct fc_ba_acc_s{
	u32        seq_id_valid:8;	
	u32        seq_id:8;	
	u32        res2:16;
	u32        ox_id:16;	
	u32        rx_id:16;	
	u32        low_seq_cnt:16;	
	u32        high_seq_cnt:16;
};


struct fc_ba_rjt_s{
	u32        res1:8;		
	u32        reason_code:8;	
	u32        reason_expl:8;	
	u32        vendor_unique:8;
};


struct fc_tprlo_params_page_s{
	u32        type:8;
	u32        type_ext:8;

#ifdef __BIGENDIAN
	u32        opa_valid:1;
	u32        rpa_valid:1;
	u32        tpo_nport_valid:1;
	u32        global_process_logout:1;
	u32        res1:12;
#else
	u32        res1:12;
	u32        global_process_logout:1;
	u32        tpo_nport_valid:1;
	u32        rpa_valid:1;
	u32        opa_valid:1;
#endif

	u32        orig_process_assc;
	u32        resp_process_assc;

	u32        res2:8;
	u32        tpo_nport_id;
};


struct fc_tprlo_s{
	u32        command:8;
	u32        page_len:8;
	u32        payload_len:16;

	struct fc_tprlo_params_page_s tprlo_params[1];
};

enum fc_tprlo_type{
	FC_GLOBAL_LOGO = 1,
	FC_TPR_LOGO
};


struct fc_tprlo_acc_s{
	u32	command:8;
	u32	page_len:8;
	u32	payload_len:16;
	struct fc_prlo_acc_params_page_s tprlo_acc_params[1];
};


#define FC_RSCN_PGLEN	0x4

enum fc_rscn_format{
	FC_RSCN_FORMAT_PORTID	= 0x0,
	FC_RSCN_FORMAT_AREA	= 0x1,
	FC_RSCN_FORMAT_DOMAIN	= 0x2,
	FC_RSCN_FORMAT_FABRIC	= 0x3,
};

struct fc_rscn_event_s{
	u32        format:2;
	u32        qualifier:4;
	u32        resvd:2;
	u32        portid:24;
};

struct fc_rscn_pl_s{
	u8         command;
	u8         pagelen;
	u16        payldlen;
	struct fc_rscn_event_s event[1];
};


struct fc_echo_s {
	struct fc_els_cmd_s    els_cmd;
};



#define RNID_NODEID_DATA_FORMAT_COMMON    		 0x00
#define RNID_NODEID_DATA_FORMAT_FCP3        		 0x08
#define RNID_NODEID_DATA_FORMAT_DISCOVERY     		0xDF

#define RNID_ASSOCIATED_TYPE_UNKNOWN                    0x00000001
#define RNID_ASSOCIATED_TYPE_OTHER                      0x00000002
#define RNID_ASSOCIATED_TYPE_HUB                        0x00000003
#define RNID_ASSOCIATED_TYPE_SWITCH                     0x00000004
#define RNID_ASSOCIATED_TYPE_GATEWAY                    0x00000005
#define RNID_ASSOCIATED_TYPE_STORAGE_DEVICE             0x00000009
#define RNID_ASSOCIATED_TYPE_HOST                       0x0000000A
#define RNID_ASSOCIATED_TYPE_STORAGE_SUBSYSTEM          0x0000000B
#define RNID_ASSOCIATED_TYPE_STORAGE_ACCESS_DEVICE      0x0000000E
#define RNID_ASSOCIATED_TYPE_NAS_SERVER                 0x00000011
#define RNID_ASSOCIATED_TYPE_BRIDGE                     0x00000002
#define RNID_ASSOCIATED_TYPE_VIRTUALIZATION_DEVICE      0x00000003
#define RNID_ASSOCIATED_TYPE_MULTI_FUNCTION_DEVICE      0x000000FF


struct fc_rnid_cmd_s{
	struct fc_els_cmd_s    els_cmd;
	u32        node_id_data_format:8;
	u32        reserved:24;
};



struct fc_rnid_common_id_data_s{
	wwn_t           port_name;
	wwn_t           node_name;
};

struct fc_rnid_general_topology_data_s{
	u32        vendor_unique[4];
	u32        asso_type;
	u32        phy_port_num;
	u32        num_attached_nodes;
	u32        node_mgmt:8;
	u32        ip_version:8;
	u32        udp_tcp_port_num:16;
	u32        ip_address[4];
	u32        reserved:16;
	u32        vendor_specific:16;
};

struct fc_rnid_acc_s{
	struct fc_els_cmd_s    els_cmd;
	u32        node_id_data_format:8;
	u32        common_id_data_length:8;
	u32        reserved:8;
	u32        specific_id_data_length:8;
	struct fc_rnid_common_id_data_s common_id_data;
	struct fc_rnid_general_topology_data_s gen_topology_data;
};

#define RNID_ASSOCIATED_TYPE_UNKNOWN                    0x00000001
#define RNID_ASSOCIATED_TYPE_OTHER                      0x00000002
#define RNID_ASSOCIATED_TYPE_HUB                        0x00000003
#define RNID_ASSOCIATED_TYPE_SWITCH                     0x00000004
#define RNID_ASSOCIATED_TYPE_GATEWAY                    0x00000005
#define RNID_ASSOCIATED_TYPE_STORAGE_DEVICE             0x00000009
#define RNID_ASSOCIATED_TYPE_HOST                       0x0000000A
#define RNID_ASSOCIATED_TYPE_STORAGE_SUBSYSTEM          0x0000000B
#define RNID_ASSOCIATED_TYPE_STORAGE_ACCESS_DEVICE      0x0000000E
#define RNID_ASSOCIATED_TYPE_NAS_SERVER                 0x00000011
#define RNID_ASSOCIATED_TYPE_BRIDGE                     0x00000002
#define RNID_ASSOCIATED_TYPE_VIRTUALIZATION_DEVICE      0x00000003
#define RNID_ASSOCIATED_TYPE_MULTI_FUNCTION_DEVICE      0x000000FF

enum fc_rpsc_speed_cap{
	RPSC_SPEED_CAP_1G = 0x8000,
	RPSC_SPEED_CAP_2G = 0x4000,
	RPSC_SPEED_CAP_4G = 0x2000,
	RPSC_SPEED_CAP_10G = 0x1000,
	RPSC_SPEED_CAP_8G = 0x0800,
	RPSC_SPEED_CAP_16G = 0x0400,

	RPSC_SPEED_CAP_UNKNOWN = 0x0001,
};

enum fc_rpsc_op_speed_s{
	RPSC_OP_SPEED_1G = 0x8000,
	RPSC_OP_SPEED_2G = 0x4000,
	RPSC_OP_SPEED_4G = 0x2000,
	RPSC_OP_SPEED_10G = 0x1000,
	RPSC_OP_SPEED_8G = 0x0800,
	RPSC_OP_SPEED_16G = 0x0400,

	RPSC_OP_SPEED_NOT_EST = 0x0001,	
};

struct fc_rpsc_speed_info_s{
	u16        port_speed_cap;	
	u16        port_op_speed;	
};

enum link_e2e_beacon_subcmd{
	LINK_E2E_BEACON_ON = 1,
	LINK_E2E_BEACON_OFF = 2
};

enum beacon_type{
	BEACON_TYPE_NORMAL	= 1,	
	BEACON_TYPE_WARN	= 2,	
	BEACON_TYPE_CRITICAL	= 3	
};

struct link_e2e_beacon_param_s {
	u8         beacon_type;	
	u8         beacon_frequency;
					
	u16        beacon_duration;
};


struct link_e2e_beacon_req_s{
	u32        ls_code;	
	u32        ls_sub_cmd;	
	struct link_e2e_beacon_param_s beacon_parm;
};


struct fc_rpsc_cmd_s{
	struct fc_els_cmd_s    els_cmd;
};


struct fc_rpsc_acc_s{
	u32        command:8;
	u32        rsvd:8;
	u32        num_entries:16;

	struct fc_rpsc_speed_info_s speed_info[1];
};


#define FC_BRCD_TOKEN    0x42524344

struct fc_rpsc2_cmd_s{
	struct fc_els_cmd_s    els_cmd;
	u32       	token;
	u16     	resvd;
	u16     	num_pids;       
	struct  {
		u32	rsvd1:8;
		u32	pid:24;	
	} pid_list[1];
};

enum fc_rpsc2_port_type{
	RPSC2_PORT_TYPE_UNKNOWN = 0,
	RPSC2_PORT_TYPE_NPORT   = 1,
	RPSC2_PORT_TYPE_NLPORT  = 2,
	RPSC2_PORT_TYPE_NPIV_PORT  = 0x5f,
	RPSC2_PORT_TYPE_NPORT_TRUNK  = 0x6f,
};


struct fc_rpsc2_port_info_s{
    u32    pid;        
    u16    resvd1;
    u16    index;      
    u8     resvd2;
    u8    	type;        
    u16    speed;      
};


struct fc_rpsc2_acc_s{
	u8        els_cmd;
	u8        resvd;
	u16       num_pids;  
	struct fc_rpsc2_port_info_s  port_info[1];    
};


enum fc_cos{
	FC_CLASS_2	= 0x04,
	FC_CLASS_3	= 0x08,
	FC_CLASS_2_3	= 0x0C,
};


struct fc_symname_s{
	u8         symname[FC_SYMNAME_MAX];
};

struct fc_alpabm_s{
	u8         alpa_bm[FC_ALPA_MAX / 8];
};


#define FC_ED_TOV		2
#define FC_REC_TOV		(FC_ED_TOV + 1)
#define FC_RA_TOV		10
#define FC_ELS_TOV		(2 * FC_RA_TOV)


#define FC_VF_ID_NULL    0	
#define FC_VF_ID_MIN     1
#define FC_VF_ID_MAX     0xEFF
#define FC_VF_ID_CTL     0xFEF	


struct fc_vft_s{
	u32        r_ctl:8;
	u32        ver:2;
	u32        type:4;
	u32        res_a:2;
	u32        priority:3;
	u32        vf_id:12;
	u32        res_b:1;
	u32        hopct:8;
	u32        res_c:24;
};

#pragma pack()

#endif
