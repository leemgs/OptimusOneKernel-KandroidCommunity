

#ifndef __BFI_IOC_H__
#define __BFI_IOC_H__

#include "bfi.h"
#include <defs/bfa_defs_ioc.h>

#pragma pack(1)

enum bfi_ioc_h2i_msgs {
	BFI_IOC_H2I_ENABLE_REQ 		= 1,
	BFI_IOC_H2I_DISABLE_REQ 	= 2,
	BFI_IOC_H2I_GETATTR_REQ 	= 3,
	BFI_IOC_H2I_DBG_SYNC	 	= 4,
	BFI_IOC_H2I_DBG_DUMP	 	= 5,
};

enum bfi_ioc_i2h_msgs {
	BFI_IOC_I2H_ENABLE_REPLY	= BFA_I2HM(1),
	BFI_IOC_I2H_DISABLE_REPLY 	= BFA_I2HM(2),
	BFI_IOC_I2H_GETATTR_REPLY 	= BFA_I2HM(3),
	BFI_IOC_I2H_READY_EVENT 	= BFA_I2HM(4),
	BFI_IOC_I2H_HBEAT		= BFA_I2HM(5),
};


struct bfi_ioc_getattr_req_s {
	struct bfi_mhdr_s	mh;
	union bfi_addr_u	attr_addr;
};

struct bfi_ioc_attr_s {
	wwn_t           mfg_wwn;
	mac_t		mfg_mac;
	u16	rsvd_a;
	char            brcd_serialnum[STRSZ(BFA_MFG_SERIALNUM_SIZE)];
	u8         pcie_gen;
	u8         pcie_lanes_orig;
	u8         pcie_lanes;
	u8         rx_bbcredit;	
	u32        adapter_prop;	
	u16        maxfrsize;	
	char         	asic_rev;
	u8         rsvd_b;
	char            fw_version[BFA_VERSION_LEN];
	char            optrom_version[BFA_VERSION_LEN];
	struct bfa_mfg_vpd_s	vpd;
};


struct bfi_ioc_getattr_reply_s {
	struct bfi_mhdr_s  mh;		
	u8		status;	
	u8		rsvd[3];
};


#define BFI_IOC_SMEM_PG0_CB	(0x40)
#define BFI_IOC_SMEM_PG0_CT	(0x180)


#define BFI_IOC_TRC_OFF		(0x4b00)
#define BFI_IOC_TRC_ENTS	256

#define BFI_IOC_FW_SIGNATURE	(0xbfadbfad)
#define BFI_IOC_MD5SUM_SZ	4
struct bfi_ioc_image_hdr_s {
	u32        signature;	
	u32        rsvd_a;
	u32        exec;		
	u32        param;		
	u32        rsvd_b[4];
	u32        md5sum[BFI_IOC_MD5SUM_SZ];
};


struct bfi_ioc_rdy_event_s {
	struct bfi_mhdr_s  mh;			
	u8         init_status;	
	u8         rsvd[3];
};

struct bfi_ioc_hbeat_s {
	struct bfi_mhdr_s  mh;		
	u32	   hb_count;	
};


enum bfi_ioc_state {
	BFI_IOC_UNINIT 	 = 0,		
	BFI_IOC_INITING 	 = 1,	
	BFI_IOC_HWINIT 	 = 2,		
	BFI_IOC_CFG 	 = 3,		
	BFI_IOC_OP 		 = 4,	
	BFI_IOC_DISABLING 	 = 5,	
	BFI_IOC_DISABLED 	 = 6,	
	BFI_IOC_CFG_DISABLED = 7,	
	BFI_IOC_HBFAIL       = 8,	
	BFI_IOC_MEMTEST      = 9,	
};

#define BFI_IOC_ENDIAN_SIG  0x12345678

enum {
	BFI_ADAPTER_TYPE_FC   = 0x01,		
	BFI_ADAPTER_TYPE_MK   = 0x0f0000,	
	BFI_ADAPTER_TYPE_SH   = 16,	        
	BFI_ADAPTER_NPORTS_MK = 0xff00,		
	BFI_ADAPTER_NPORTS_SH = 8,	        
	BFI_ADAPTER_SPEED_MK  = 0xff,		
	BFI_ADAPTER_SPEED_SH  = 0,	        
	BFI_ADAPTER_PROTO     = 0x100000,	
	BFI_ADAPTER_TTV       = 0x200000,	
	BFI_ADAPTER_UNSUPP    = 0x400000,	
};

#define BFI_ADAPTER_GETP(__prop,__adap_prop)          		\
    (((__adap_prop) & BFI_ADAPTER_ ## __prop ## _MK) >>         \
     BFI_ADAPTER_ ## __prop ## _SH)
#define BFI_ADAPTER_SETP(__prop, __val)         		\
    ((__val) << BFI_ADAPTER_ ## __prop ## _SH)
#define BFI_ADAPTER_IS_PROTO(__adap_type)   			\
    ((__adap_type) & BFI_ADAPTER_PROTO)
#define BFI_ADAPTER_IS_TTV(__adap_type)     			\
    ((__adap_type) & BFI_ADAPTER_TTV)
#define BFI_ADAPTER_IS_UNSUPP(__adap_type)  			\
    ((__adap_type) & BFI_ADAPTER_UNSUPP)
#define BFI_ADAPTER_IS_SPECIAL(__adap_type)                     \
    ((__adap_type) & (BFI_ADAPTER_TTV | BFI_ADAPTER_PROTO |     \
			BFI_ADAPTER_UNSUPP))


struct bfi_ioc_ctrl_req_s {
	struct bfi_mhdr_s	mh;
	u8			ioc_class;
	u8         	rsvd[3];
};


struct bfi_ioc_ctrl_reply_s {
	struct bfi_mhdr_s  mh;		
	u8         status;		
	u8         rsvd[3];
};

#define BFI_IOC_MSGSZ   8

union bfi_ioc_h2i_msg_u {
	struct bfi_mhdr_s 	mh;
	struct bfi_ioc_ctrl_req_s enable_req;
	struct bfi_ioc_ctrl_req_s disable_req;
	struct bfi_ioc_getattr_req_s getattr_req;
	u32       		mboxmsg[BFI_IOC_MSGSZ];
};


union bfi_ioc_i2h_msg_u {
	struct bfi_mhdr_s      	mh;
	struct bfi_ioc_rdy_event_s 	rdy_event;
	u32       		mboxmsg[BFI_IOC_MSGSZ];
};

#pragma pack()

#endif 

