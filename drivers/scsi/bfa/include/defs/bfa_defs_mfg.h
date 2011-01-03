
#ifndef __BFA_DEFS_MFG_H__
#define __BFA_DEFS_MFG_H__

#include <bfa_os_inc.h>


#define BFA_MFG_VERSION				1


#define BFA_MFG_SERIALNUM_SIZE			11
#define BFA_MFG_PARTNUM_SIZE			14
#define BFA_MFG_SUPPLIER_ID_SIZE		10
#define BFA_MFG_SUPPLIER_PARTNUM_SIZE	20
#define BFA_MFG_SUPPLIER_SERIALNUM_SIZE	20
#define BFA_MFG_SUPPLIER_REVISION_SIZE	4
#define STRSZ(_n)	(((_n) + 4) & ~3)


#define BFA_MFG_VPD_LEN     256


struct bfa_mfg_vpd_s {
    u8     version;    
    u8     vpd_sig[3]; 
    u8     chksum;     
    u8     vendor;     
    u8     len;        
    u8     rsv;
    u8     data[BFA_MFG_VPD_LEN];  
};

#pragma pack(1)

#endif 
