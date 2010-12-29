
#ifndef __ASM_ARCH_MSM_TLMM_H
#define __ASM_ARCH_MSM_TLMM_H

#include <linux/types.h>
#include <mach/gpio-tlmm-v1.h>

enum msm_tlmm_hdrive_tgt {
	TLMM_HDRV_SDC4_CLK = 0,
	TLMM_HDRV_SDC4_CMD,
	TLMM_HDRV_SDC4_DATA,
	TLMM_HDRV_SDC3_CLK,
	TLMM_HDRV_SDC3_CMD,
	TLMM_HDRV_SDC3_DATA,
};

enum msm_tlmm_pull_tgt {
	TLMM_PULL_SDC4_CMD = 0,
	TLMM_PULL_SDC4_DATA,
	TLMM_PULL_SDC3_CMD,
	TLMM_PULL_SDC3_DATA,
};

void msm_tlmm_set_hdrive(enum msm_tlmm_hdrive_tgt tgt, int drv_str);
void msm_tlmm_set_pull(enum msm_tlmm_pull_tgt tgt, int pull);

#endif
