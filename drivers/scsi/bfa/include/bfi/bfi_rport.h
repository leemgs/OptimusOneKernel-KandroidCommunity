

#ifndef __BFI_RPORT_H__
#define __BFI_RPORT_H__

#include <bfi/bfi.h>

#pragma pack(1)

enum bfi_rport_h2i_msgs {
	BFI_RPORT_H2I_CREATE_REQ = 1,
	BFI_RPORT_H2I_DELETE_REQ = 2,
	BFI_RPORT_H2I_SET_SPEED_REQ  = 3,
};

enum bfi_rport_i2h_msgs {
	BFI_RPORT_I2H_CREATE_RSP = BFA_I2HM(1),
	BFI_RPORT_I2H_DELETE_RSP = BFA_I2HM(2),
	BFI_RPORT_I2H_QOS_SCN    = BFA_I2HM(3),
};

struct bfi_rport_create_req_s {
	struct bfi_mhdr_s  mh;		
	u16        bfa_handle;	
	u16        max_frmsz;	
	u32        pid       : 24,	
			lp_tag    : 8;	
	u32        local_pid : 24,	
			cisc      : 8;
	u8         fc_class;	
	u8         vf_en;		
	u16        vf_id;		
};

struct bfi_rport_create_rsp_s {
	struct bfi_mhdr_s  mh;		
	u8         status;		
	u8         rsvd[3];
	u16        bfa_handle;	
	u16        fw_handle;	
	struct bfa_rport_qos_attr_s qos_attr;  
};

struct bfa_rport_speed_req_s {
	struct bfi_mhdr_s  mh;		
	u16        fw_handle;	
	u8		speed;		
	u8		rsvd;
};

struct bfi_rport_delete_req_s {
	struct bfi_mhdr_s  mh;		
	u16        fw_handle;	
	u16        rsvd;
};

struct bfi_rport_delete_rsp_s {
	struct bfi_mhdr_s  mh;		
	u16        bfa_handle;	
	u8         status;		
	u8         rsvd;
};

struct bfi_rport_qos_scn_s {
	struct bfi_mhdr_s  mh;		
	u16        bfa_handle;	
	u16        rsvd;
	struct bfa_rport_qos_attr_s old_qos_attr;  
	struct bfa_rport_qos_attr_s new_qos_attr;  
};

union bfi_rport_h2i_msg_u {
	struct bfi_msg_s 		*msg;
	struct bfi_rport_create_req_s	*create_req;
	struct bfi_rport_delete_req_s	*delete_req;
	struct bfi_rport_speed_req_s	*speed_req;
};

union bfi_rport_i2h_msg_u {
	struct bfi_msg_s 		*msg;
	struct bfi_rport_create_rsp_s	*create_rsp;
	struct bfi_rport_delete_rsp_s	*delete_rsp;
	struct bfi_rport_qos_scn_s	*qos_scn_evt;
};

#pragma pack()

#endif 

