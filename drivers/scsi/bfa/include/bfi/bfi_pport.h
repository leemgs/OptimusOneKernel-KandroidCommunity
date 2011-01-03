
#ifndef __BFI_PPORT_H__
#define __BFI_PPORT_H__

#include <bfi/bfi.h>
#include <defs/bfa_defs_pport.h>

#pragma pack(1)

enum bfi_pport_h2i {
	BFI_PPORT_H2I_ENABLE_REQ		= (1),
	BFI_PPORT_H2I_DISABLE_REQ		= (2),
	BFI_PPORT_H2I_GET_STATS_REQ		= (3),
	BFI_PPORT_H2I_CLEAR_STATS_REQ	= (4),
	BFI_PPORT_H2I_SET_SVC_PARAMS_REQ	= (5),
	BFI_PPORT_H2I_ENABLE_RX_VF_TAG_REQ	= (6),
	BFI_PPORT_H2I_ENABLE_TX_VF_TAG_REQ	= (7),
	BFI_PPORT_H2I_GET_QOS_STATS_REQ		= (8),
	BFI_PPORT_H2I_CLEAR_QOS_STATS_REQ	= (9),
};

enum bfi_pport_i2h {
	BFI_PPORT_I2H_ENABLE_RSP		= BFA_I2HM(1),
	BFI_PPORT_I2H_DISABLE_RSP		= BFA_I2HM(2),
	BFI_PPORT_I2H_GET_STATS_RSP		= BFA_I2HM(3),
	BFI_PPORT_I2H_CLEAR_STATS_RSP	= BFA_I2HM(4),
	BFI_PPORT_I2H_SET_SVC_PARAMS_RSP	= BFA_I2HM(5),
	BFI_PPORT_I2H_ENABLE_RX_VF_TAG_RSP	= BFA_I2HM(6),
	BFI_PPORT_I2H_ENABLE_TX_VF_TAG_RSP	= BFA_I2HM(7),
	BFI_PPORT_I2H_EVENT			= BFA_I2HM(8),
	BFI_PPORT_I2H_GET_QOS_STATS_RSP		= BFA_I2HM(9),
	BFI_PPORT_I2H_CLEAR_QOS_STATS_RSP	= BFA_I2HM(10),
};


struct bfi_pport_generic_req_s {
	struct bfi_mhdr_s  mh;		
	u32        msgtag;		
};


struct bfi_pport_generic_rsp_s {
	struct bfi_mhdr_s  mh;		
	u8         status;		
	u8         rsvd[3];
	u32        msgtag;		
};


struct bfi_pport_enable_req_s {
	struct bfi_mhdr_s  mh;		
	u32        rsvd1;
	wwn_t           nwwn;		
	wwn_t           pwwn;		
	struct bfa_pport_cfg_s port_cfg;	
	union bfi_addr_u  stats_dma_addr;	
	u32        msgtag;		
	u32        rsvd2;
};


#define bfi_pport_enable_rsp_t struct bfi_pport_generic_rsp_s


#define bfi_pport_disable_req_t struct bfi_pport_generic_req_s


#define bfi_pport_disable_rsp_t struct bfi_pport_generic_rsp_s


#define bfi_pport_get_stats_req_t struct bfi_pport_generic_req_s


#define bfi_pport_get_stats_rsp_t struct bfi_pport_generic_rsp_s


#define bfi_pport_clear_stats_req_t struct bfi_pport_generic_req_s


#define bfi_pport_clear_stats_rsp_t struct bfi_pport_generic_rsp_s


#define bfi_pport_get_qos_stats_req_t struct bfi_pport_generic_req_s


#define bfi_pport_get_qos_stats_rsp_t struct bfi_pport_generic_rsp_s


#define bfi_pport_clear_qos_stats_req_t struct bfi_pport_generic_req_s


#define bfi_pport_clear_qos_stats_rsp_t struct bfi_pport_generic_rsp_s


struct bfi_pport_set_svc_params_req_s {
	struct bfi_mhdr_s  mh;		
	u16        tx_bbcredit;	
	u16        rsvd;
};




struct bfi_pport_event_s {
	struct bfi_mhdr_s 	mh;	
	struct bfa_pport_link_s	link_state;
};

union bfi_pport_h2i_msg_u {
	struct bfi_mhdr_s			*mhdr;
	struct bfi_pport_enable_req_s		*penable;
	struct bfi_pport_generic_req_s		*pdisable;
	struct bfi_pport_generic_req_s		*pgetstats;
	struct bfi_pport_generic_req_s		*pclearstats;
	struct bfi_pport_set_svc_params_req_s	*psetsvcparams;
	struct bfi_pport_get_qos_stats_req_s	*pgetqosstats;
	struct bfi_pport_generic_req_s		*pclearqosstats;
};

union bfi_pport_i2h_msg_u {
	struct bfi_msg_s			*msg;
	struct bfi_pport_generic_rsp_s		*enable_rsp;
	struct bfi_pport_disable_rsp_s		*disable_rsp;
	struct bfi_pport_generic_rsp_s		*getstats_rsp;
	struct bfi_pport_clear_stats_rsp_s	*clearstats_rsp;
	struct bfi_pport_set_svc_params_rsp_s	*setsvcparasm_rsp;
	struct bfi_pport_get_qos_stats_rsp_s	*getqosstats_rsp;
	struct bfi_pport_clear_qos_stats_rsp_s	*clearqosstats_rsp;
	struct bfi_pport_event_s		*event;
};

#pragma pack()

#endif 

