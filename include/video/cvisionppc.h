

#ifndef CVISIONPPC_H
#define CVISIONPPC_H

#ifndef PM2FB_H
#include "pm2fb.h"
#endif

struct cvppc_par {
	unsigned char* pci_config;
	unsigned char* pci_bridge;
	u32 user_flags;
};

#define CSPPC_PCI_BRIDGE		0xfffe0000
#define CSPPC_BRIDGE_ENDIAN		0x0000
#define CSPPC_BRIDGE_INT		0x0010

#define	CVPPC_PCI_CONFIG		0xfffc0000
#define CVPPC_ROM_ADDRESS		0xe2000001
#define CVPPC_REGS_REGION		0xef000000
#define CVPPC_FB_APERTURE_ONE		0xe0000000
#define CVPPC_FB_APERTURE_TWO		0xe1000000
#define CVPPC_FB_SIZE			0x00800000
#define CVPPC_MEM_CONFIG_OLD		0xed61fcaa	
#define CVPPC_MEM_CONFIG_NEW		0xed41c532	
#define CVPPC_MEMCLOCK			83000		


#define CSPPCF_BRIDGE_BIG_ENDIAN	0x02


#define CSPPCF_BRIDGE_ACTIVE_INT2	0x01

#endif	


