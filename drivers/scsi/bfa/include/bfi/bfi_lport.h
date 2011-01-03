

#ifndef __BFI_LPORT_H__
#define __BFI_LPORT_H__

#include <bfi/bfi.h>

#pragma pack(1)

enum bfi_lport_h2i_msgs {
	BFI_LPORT_H2I_CREATE_REQ = 1,
	BFI_LPORT_H2I_DELETE_REQ = 2,
};

enum bfi_lport_i2h_msgs {
	BFI_LPORT_I2H_CREATE_RSP = BFA_I2HM(1),
	BFI_LPORT_I2H_DELETE_RSP = BFA_I2HM(2),
	BFI_LPORT_I2H_ONLINE	  = BFA_I2HM(3),
	BFI_LPORT_I2H_OFFLINE	  = BFA_I2HM(4),
};

#define BFI_LPORT_MAX_SYNNAME	64

enum bfi_lport_role_e {
	BFI_LPORT_ROLE_FCPIM	= 1,
	BFI_LPORT_ROLE_FCPTM	= 2,
	BFI_LPORT_ROLE_IPFC	= 4,
};

struct bfi_lport_create_req_s {
	bfi_mhdr_t	mh;		
	u16	fabric_fwhdl;	
	u8		roles;		
	u8		rsvd;
	wwn_t		pwwn;		
	wwn_t		nwwn;		
	u8		symname[BFI_LPORT_MAX_SYNNAME];
};

struct bfi_lport_create_rsp_s {
	bfi_mhdr_t	mh;		
	u8         status;		
	u8         rsvd[3];
};

struct bfi_lport_delete_req_s {
	bfi_mhdr_t	mh;		
	u16        fw_handle;	
	u16        rsvd;
};

struct bfi_lport_delete_rsp_s {
	bfi_mhdr_t	mh;		
	u16        bfa_handle;	
	u8         status;		
	u8         rsvd;
};

union bfi_lport_h2i_msg_u {
	bfi_msg_t		*msg;
	struct bfi_lport_create_req_s	*create_req;
	struct bfi_lport_delete_req_s	*delete_req;
};

union bfi_lport_i2h_msg_u {
	bfi_msg_t		*msg;
	struct bfi_lport_create_rsp_s	*create_rsp;
	struct bfi_lport_delete_rsp_s	*delete_rsp;
};

#pragma pack()

#endif 

