
#ifndef _MACH_DAVINCI_EMAC_H
#define _MACH_DAVINCI_EMAC_H

#include <linux/if_ether.h>
#include <linux/memory.h>

struct emac_platform_data {
	char mac_addr[ETH_ALEN];
	u32 ctrl_reg_offset;
	u32 ctrl_mod_reg_offset;
	u32 ctrl_ram_offset;
	u32 mdio_reg_offset;
	u32 ctrl_ram_size;
	u32 phy_mask;
	u32 mdio_max_freq;
	u8 rmii_en;
	u8 version;
};

enum {
	EMAC_VERSION_1,	
	EMAC_VERSION_2,	
};

void davinci_get_mac_addr(struct memory_accessor *mem_acc, void *context);
#endif
