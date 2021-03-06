

#ifndef __BFI_IOCFC_H__
#define __BFI_IOCFC_H__

#include "bfi.h"
#include <defs/bfa_defs_ioc.h>
#include <defs/bfa_defs_iocfc.h>
#include <defs/bfa_defs_boot.h>

#pragma pack(1)

enum bfi_iocfc_h2i_msgs {
	BFI_IOCFC_H2I_CFG_REQ 		= 1,
	BFI_IOCFC_H2I_GET_STATS_REQ 	= 2,
	BFI_IOCFC_H2I_CLEAR_STATS_REQ	= 3,
	BFI_IOCFC_H2I_SET_INTR_REQ 	= 4,
	BFI_IOCFC_H2I_UPDATEQ_REQ = 5,
};

enum bfi_iocfc_i2h_msgs {
	BFI_IOCFC_I2H_CFG_REPLY		= BFA_I2HM(1),
	BFI_IOCFC_I2H_GET_STATS_RSP 	= BFA_I2HM(2),
	BFI_IOCFC_I2H_CLEAR_STATS_RSP	= BFA_I2HM(3),
	BFI_IOCFC_I2H_UPDATEQ_RSP = BFA_I2HM(5),
};

struct bfi_iocfc_cfg_s {
	u8         num_cqs; 	
	u8         sense_buf_len;	
	u8         trunk_enabled;	
	u8         trunk_ports;	
	u32        endian_sig;	

	
	union bfi_addr_u  req_cq_ba[BFI_IOC_MAX_CQS];
	union bfi_addr_u  req_shadow_ci[BFI_IOC_MAX_CQS];
	u16    req_cq_elems[BFI_IOC_MAX_CQS];
	union bfi_addr_u  rsp_cq_ba[BFI_IOC_MAX_CQS];
	union bfi_addr_u  rsp_shadow_pi[BFI_IOC_MAX_CQS];
	u16    rsp_cq_elems[BFI_IOC_MAX_CQS];

	union bfi_addr_u  stats_addr;	
	union bfi_addr_u  cfgrsp_addr;	
	union bfi_addr_u  ioim_snsbase;  
	struct bfa_iocfc_intr_attr_s intr_attr; 
};


struct bfi_iocfc_bootwwns {
	wwn_t		wwn[BFA_BOOT_BOOTLUN_MAX];
	u8		nwwns;
	u8		rsvd[7];
};

struct bfi_iocfc_cfgrsp_s {
	struct bfa_iocfc_fwcfg_s	fwcfg;
	struct bfa_iocfc_intr_attr_s	intr_attr;
	struct bfi_iocfc_bootwwns	bootwwns;
};


struct bfi_iocfc_cfg_req_s {
	struct bfi_mhdr_s      mh;
	union bfi_addr_u      ioc_cfg_dma_addr;
};


struct bfi_iocfc_cfg_reply_s {
	struct bfi_mhdr_s  mh;		
	u8         cfg_success;	
	u8         lpu_bm;		
	u8         rsvd[2];
};


struct bfi_iocfc_stats_req_s {
	struct bfi_mhdr_s mh;		
	u32        msgtag;		
};


struct bfi_iocfc_stats_rsp_s {
	struct bfi_mhdr_s mh;		
	u8         status;		
	u8         rsvd[3];
	u32        msgtag;		
};


struct bfi_iocfc_set_intr_req_s {
	struct bfi_mhdr_s mh;		
	u8		coalesce;	
	u8         rsvd[3];
	u16	delay;		
	u16	latency;	
};


struct bfi_iocfc_updateq_req_s {
	struct bfi_mhdr_s mh;		
	u32 reqq_ba;			
	u32 rspq_ba;			
	u32 reqq_sci;			
	u32 rspq_spi;			
};


struct bfi_iocfc_updateq_rsp_s {
	struct bfi_mhdr_s mh;		
	u8         status;		
	u8         rsvd[3];
};


union bfi_iocfc_h2i_msg_u {
	struct bfi_mhdr_s 		mh;
	struct bfi_iocfc_cfg_req_s 	cfg_req;
	struct bfi_iocfc_stats_req_s stats_get;
	struct bfi_iocfc_stats_req_s stats_clr;
	struct bfi_iocfc_updateq_req_s updateq_req;
	u32       			mboxmsg[BFI_IOC_MSGSZ];
};


union bfi_iocfc_i2h_msg_u {
	struct bfi_mhdr_s      		mh;
	struct bfi_iocfc_cfg_reply_s 		cfg_reply;
	struct bfi_iocfc_stats_rsp_s stats_get_rsp;
	struct bfi_iocfc_stats_rsp_s stats_clr_rsp;
	struct bfi_iocfc_updateq_rsp_s updateq_rsp;
	u32       			mboxmsg[BFI_IOC_MSGSZ];
};

#pragma pack()

#endif 

