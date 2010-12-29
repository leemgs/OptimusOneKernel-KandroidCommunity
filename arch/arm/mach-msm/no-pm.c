

#include <linux/module.h>

#include "idle.h"
#include "pm.h"

void arch_idle(void)
{
	msm_arch_idle();
}

void msm_pm_set_platform_data(struct msm_pm_platform_data *data, int count)
{ }

void msm_pm_set_max_sleep_time(int64_t max_sleep_time_ns) { }
EXPORT_SYMBOL(msm_pm_set_max_sleep_time);
