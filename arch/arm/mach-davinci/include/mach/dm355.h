
#ifndef __ASM_ARCH_DM355_H
#define __ASM_ARCH_DM355_H

#include <mach/hardware.h>
#include <mach/asp.h>
#include <media/davinci/vpfe_capture.h>

#define ASP1_TX_EVT_EN	1
#define ASP1_RX_EVT_EN	2

struct spi_board_info;

void __init dm355_init(void);
void dm355_init_spi0(unsigned chipselect_mask,
		struct spi_board_info *info, unsigned len);
void __init dm355_init_asp1(u32 evt_enable, struct snd_platform_data *pdata);
void dm355_set_vpfe_config(struct vpfe_config *cfg);

#endif 
