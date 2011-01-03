

#ifndef __BFI_UF_H__
#define __BFI_UF_H__

#include "bfi.h"

#pragma pack(1)

enum bfi_uf_h2i {
	BFI_UF_H2I_BUF_POST = 1,
};

enum bfi_uf_i2h {
	BFI_UF_I2H_FRM_RCVD = BFA_I2HM(1),
};

#define BFA_UF_MAX_SGES	2

struct bfi_uf_buf_post_s {
	struct bfi_mhdr_s  mh;		
	u16        buf_tag;	
	u16        buf_len;	
	struct bfi_sge_s   sge[BFA_UF_MAX_SGES]; 
};

struct bfi_uf_frm_rcvd_s {
	struct bfi_mhdr_s  mh;		
	u16        buf_tag;	
	u16        rsvd;
	u16        frm_len;	
	u16        xfr_len;	
};

#pragma pack()

#endif 
