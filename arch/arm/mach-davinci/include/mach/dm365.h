
#ifndef __ASM_ARCH_DM365_H
#define __ASM_ARCH_DM665_H

#include <linux/platform_device.h>
#include <mach/hardware.h>
#include <mach/emac.h>

#define DM365_EMAC_BASE			(0x01D07000)
#define DM365_EMAC_CNTRL_OFFSET		(0x0000)
#define DM365_EMAC_CNTRL_MOD_OFFSET	(0x3000)
#define DM365_EMAC_CNTRL_RAM_OFFSET	(0x1000)
#define DM365_EMAC_MDIO_OFFSET		(0x4000)
#define DM365_EMAC_CNTRL_RAM_SIZE	(0x2000)

void __init dm365_init(void);

#endif 
