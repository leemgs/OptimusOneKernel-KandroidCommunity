
#ifndef __BFI_PORT_H__
#define __BFI_PORT_H__

#include <bfi/bfi.h>
#include <defs/bfa_defs_pport.h>

#pragma pack(1)

enum bfi_port_h2i {
	BFI_PORT_H2I_ENABLE_REQ		= (1),
	BFI_PORT_H2I_DISABLE_REQ	= (2),
	BFI_PORT_H2I_GET_STATS_REQ	= (3),
	BFI_PORT_H2I_CLEAR_STATS_REQ	= (4),
};

enum bfi_port_i2h {
	BFI_PORT_I2H_ENABLE_RSP		= BFA_I2HM(1),
	BFI_PORT_I2H_DISABLE_RSP	= BFA_I2HM(2),
	BFI_PORT_I2H_GET_STATS_RSP	= BFA_I2HM(3),
	BFI_PORT_I2H_CLEAR_STATS_RSP	= BFA_I2HM(4),
};


struct bfi_port_generic_req_s {
	struct bfi_mhdr_s  mh;		
	u32        msgtag;		
	u32	rsvd;
};


struct bfi_port_generic_rsp_s {
	struct bfi_mhdr_s  mh;		
	u8         status;		
	u8         rsvd[3];
	u32        msgtag;		
};










struct bfi_port_get_stats_req_s {
	struct bfi_mhdr_s  mh;		
	union bfi_addr_u   dma_addr;
};







union bfi_port_h2i_msg_u {
	struct bfi_mhdr_s		mh;
	struct bfi_port_generic_req_s	enable_req;
	struct bfi_port_generic_req_s	disable_req;
	struct bfi_port_get_stats_req_s	getstats_req;
	struct bfi_port_generic_req_s	clearstats_req;
};

union bfi_port_i2h_msg_u {
	struct bfi_mhdr_s         	mh;
	struct bfi_port_generic_rsp_s	enable_rsp;
	struct bfi_port_generic_rsp_s	disable_rsp;
	struct bfi_port_generic_rsp_s	getstats_rsp;
	struct bfi_port_generic_rsp_s	clearstats_rsp;
};

#pragma pack()

#endif 

