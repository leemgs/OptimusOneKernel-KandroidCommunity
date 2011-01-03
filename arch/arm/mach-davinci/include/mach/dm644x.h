
#ifndef __ASM_ARCH_DM644X_H
#define __ASM_ARCH_DM644X_H

#include <linux/platform_device.h>
#include <mach/hardware.h>
#include <mach/emac.h>
#include <mach/asp.h>
#include <media/davinci/vpfe_capture.h>

#define DM644X_EMAC_BASE		(0x01C80000)
#define DM644X_EMAC_CNTRL_OFFSET	(0x0000)
#define DM644X_EMAC_CNTRL_MOD_OFFSET	(0x1000)
#define DM644X_EMAC_CNTRL_RAM_OFFSET	(0x2000)
#define DM644X_EMAC_MDIO_OFFSET		(0x4000)
#define DM644X_EMAC_CNTRL_RAM_SIZE	(0x2000)

void __init dm644x_init(void);
void __init dm644x_init_asp(struct snd_platform_data *pdata);
void dm644x_set_vpfe_config(struct vpfe_config *cfg);

#endif 
