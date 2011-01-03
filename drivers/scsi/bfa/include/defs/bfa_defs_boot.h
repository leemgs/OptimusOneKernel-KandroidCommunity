

#ifndef __BFA_DEFS_BOOT_H__
#define __BFA_DEFS_BOOT_H__

#include <protocol/types.h>
#include <defs/bfa_defs_types.h>
#include <defs/bfa_defs_pport.h>

enum {
	BFA_BOOT_BOOTLUN_MAX = 4,	
};

#define BOOT_CFG_REV1	1


enum bfa_boot_bootopt {
    BFA_BOOT_AUTO_DISCOVER = 0,    
    BFA_BOOT_STORED_BLUN   = 1,    
    BFA_BOOT_FIRST_LUN     = 2,    
};


struct bfa_boot_bootlun_s {
	wwn_t           pwwn;	
	lun_t           lun;	
};


struct bfa_boot_cfg_s {
	u8         version;
	u8         rsvd1;
	u16        chksum;

	u8         enable;		
	u8         speed;		
	u8         topology;	
	u8         bootopt;	

	u32        nbluns;		

	u32        rsvd2;

	struct bfa_boot_bootlun_s blun[BFA_BOOT_BOOTLUN_MAX];
	struct bfa_boot_bootlun_s blun_disc[BFA_BOOT_BOOTLUN_MAX];
};


#endif 
