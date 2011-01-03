

#ifndef __BFA_DEFS_IOC_H__
#define __BFA_DEFS_IOC_H__

#include <protocol/types.h>
#include <defs/bfa_defs_types.h>
#include <defs/bfa_defs_version.h>
#include <defs/bfa_defs_adapter.h>
#include <defs/bfa_defs_pm.h>

enum {
	BFA_IOC_DRIVER_LEN	= 16,
	BFA_IOC_CHIP_REV_LEN 	= 8,
};


struct bfa_ioc_driver_attr_s {
	char            driver[BFA_IOC_DRIVER_LEN];	
	char            driver_ver[BFA_VERSION_LEN];	
	char            fw_ver[BFA_VERSION_LEN];	
	char            bios_ver[BFA_VERSION_LEN];	
	char            efi_ver[BFA_VERSION_LEN];	
	char            ob_ver[BFA_VERSION_LEN];	
};


struct bfa_ioc_pci_attr_s {
	u16        vendor_id;	
	u16        device_id;	
	u16        ssid;		
	u16        ssvid;		
	u32        pcifn;		
	u32        rsvd;		
	u8         chip_rev[BFA_IOC_CHIP_REV_LEN];	 
};


enum bfa_ioc_state {
	BFA_IOC_RESET       = 1,  
	BFA_IOC_SEMWAIT     = 2,  
	BFA_IOC_HWINIT 	    = 3,  
	BFA_IOC_GETATTR     = 4,  
	BFA_IOC_OPERATIONAL = 5,  
	BFA_IOC_INITFAIL    = 6,  
	BFA_IOC_HBFAIL      = 7,  
	BFA_IOC_DISABLING   = 8,  
	BFA_IOC_DISABLED    = 9,  
	BFA_IOC_FWMISMATCH  = 10, 
};


struct bfa_fw_ioc_stats_s {
	u32        hb_count;
	u32        cfg_reqs;
	u32        enable_reqs;
	u32        disable_reqs;
	u32        stats_reqs;
	u32        clrstats_reqs;
	u32        unknown_reqs;
	u32        ic_reqs;		
};


struct bfa_ioc_drv_stats_s {
	u32	ioc_isrs;
	u32	ioc_enables;
	u32	ioc_disables;
	u32	ioc_hbfails;
	u32	ioc_boots;
	u32	stats_tmos;
	u32        hb_count;
	u32        disable_reqs;
	u32        enable_reqs;
	u32        disable_replies;
	u32        enable_replies;
};


struct bfa_ioc_stats_s {
	struct bfa_ioc_drv_stats_s	drv_stats; 
	struct bfa_fw_ioc_stats_s 	fw_stats;  
};


enum bfa_ioc_type_e {
	BFA_IOC_TYPE_FC	  = 1,
	BFA_IOC_TYPE_FCoE = 2,
	BFA_IOC_TYPE_LL	  = 3,
};


struct bfa_ioc_attr_s {
	enum bfa_ioc_type_e		ioc_type;
	enum bfa_ioc_state 		state;		
	struct bfa_adapter_attr_s	adapter_attr;	
	struct bfa_ioc_driver_attr_s 	driver_attr;	
	struct bfa_ioc_pci_attr_s	pci_attr;
	u8				port_id;	
};


enum bfa_ioc_aen_event {
	BFA_IOC_AEN_HBGOOD	= 1,	
	BFA_IOC_AEN_HBFAIL	= 2,	
	BFA_IOC_AEN_ENABLE	= 3,	
	BFA_IOC_AEN_DISABLE	= 4,	
	BFA_IOC_AEN_FWMISMATCH	= 5,	
};


struct bfa_ioc_aen_data_s {
	enum bfa_ioc_type_e ioc_type;
	wwn_t	pwwn;
	mac_t	mac;
};

#endif 

