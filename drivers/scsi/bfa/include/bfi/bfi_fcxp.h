

#ifndef __BFI_FCXP_H__
#define __BFI_FCXP_H__

#include "bfi.h"

#pragma pack(1)

enum bfi_fcxp_h2i {
	BFI_FCXP_H2I_SEND_REQ = 1,
};

enum bfi_fcxp_i2h {
	BFI_FCXP_I2H_SEND_RSP = BFA_I2HM(1),
};

#define BFA_FCXP_MAX_SGES	2


struct bfi_fcxp_send_req_s {
	struct bfi_mhdr_s  mh;		
	u16        fcxp_tag;	
	u16        max_frmsz;	
	u16        vf_id;		
	u16        rport_fw_hndl;	
	u8         class;		
	u8         rsp_timeout;	
	u8         cts;		
	u8         lp_tag;		
	struct fchs_s   fchs;		
	u32        req_len;	
	u32        rsp_maxlen;	
	struct bfi_sge_s   req_sge[BFA_FCXP_MAX_SGES];	
	struct bfi_sge_s   rsp_sge[BFA_FCXP_MAX_SGES];	
};


struct bfi_fcxp_send_rsp_s {
	struct bfi_mhdr_s  mh;		
	u16        fcxp_tag;	
	u8         req_status;	
	u8         rsvd;
	u32        rsp_len;	
	u32        residue_len;	
	struct fchs_s   fchs;		
};

#pragma pack()

#endif 

