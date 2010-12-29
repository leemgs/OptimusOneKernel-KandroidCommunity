

#ifndef __ARCH_ARM_MACH_MSM_PM_H
#define __ARCH_ARM_MACH_MSM_PM_H

#include <linux/types.h>

enum {
	MSM_PM_SLEEP_MODE_POWER_COLLAPSE_SUSPEND,
	MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
	MSM_PM_SLEEP_MODE_APPS_SLEEP,
	MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT,
	MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT,
	MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN,
	MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE,
	MSM_PM_SLEEP_MODE_NR
};

#define MSM_PM_MODE(cpu, mode_nr)  ((cpu) * MSM_PM_SLEEP_MODE_NR + (mode_nr))

struct msm_pm_platform_data {
	u8 supported;
	u8 suspend_enabled;  
	u8 idle_enabled;     
	u32 latency;         
	u32 residency;       
};

void msm_pm_set_platform_data(struct msm_pm_platform_data *data, int count);

#endif
