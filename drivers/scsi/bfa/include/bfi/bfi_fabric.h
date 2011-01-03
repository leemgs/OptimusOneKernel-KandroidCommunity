

#ifndef __BFI_FABRIC_H__
#define __BFI_FABRIC_H__

#include <bfi/bfi.h>

#pragma pack(1)

enum bfi_fabric_h2i_msgs {
	BFI_FABRIC_H2I_CREATE_REQ	= 1,
	BFI_FABRIC_H2I_DELETE_REQ	= 2,
	BFI_FABRIC_H2I_SETAUTH		= 3,
};

enum bfi_fabric_i2h_msgs {
	BFI_FABRIC_I2H_CREATE_RSP	= BFA_I2HM(1),
	BFI_FABRIC_I2H_DELETE_RSP	= BFA_I2HM(2),
	BFI_FABRIC_I2H_SETAUTH_RSP	= BFA_I2HM(3),
	BFI_FABRIC_I2H_ONLINE		= BFA_I2HM(4),
	BFI_FABRIC_I2H_OFFLINE		= BFA_I2HM(5),
};

struct bfi_fabric_create_req_s {
	bfi_mhdr_t	mh;		
	u8         vf_en;		
	u8         rsvd;
	u16        vf_id;		
	wwn_t		pwwn;		
	wwn_t		nwwn;		
};

struct bfi_fabric_create_rsp_s {
	bfi_mhdr_t	mh;		
	u16        bfa_handle;	
	u8         status;		
	u8         rsvd;
};

struct bfi_fabric_delete_req_s {
	bfi_mhdr_t	mh;		
	u16        fw_handle;	
	u16        rsvd;
};

struct bfi_fabric_delete_rsp_s {
	bfi_mhdr_t	mh;		
	u16        bfa_handle;	
	u8         status;		
	u8         rsvd;
};

#define BFI_FABRIC_AUTHSECRET_LEN	64
struct bfi_fabric_setauth_req_s {
	bfi_mhdr_t	mh;		
	u16        fw_handle;	
	u8		algorithm;
	u8		group;
	u8		secret[BFI_FABRIC_AUTHSECRET_LEN];
};

union bfi_fabric_h2i_msg_u {
	bfi_msg_t		*msg;
	struct bfi_fabric_create_req_s	*create_req;
	struct bfi_fabric_delete_req_s	*delete_req;
};

union bfi_fabric_i2h_msg_u {
	bfi_msg_t		*msg;
	struct bfi_fabric_create_rsp_s	*create_rsp;
	struct bfi_fabric_delete_rsp_s	*delete_rsp;
};

#pragma pack()

#endif 

