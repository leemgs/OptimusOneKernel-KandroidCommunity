

#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/cpuidle.h>

#include "cpuidle.h"

struct cpuidle_driver *cpuidle_curr_driver;
DEFINE_SPINLOCK(cpuidle_driver_lock);


int cpuidle_register_driver(struct cpuidle_driver *drv)
{
	if (!drv)
		return -EINVAL;

	spin_lock(&cpuidle_driver_lock);
	if (cpuidle_curr_driver) {
		spin_unlock(&cpuidle_driver_lock);
		return -EBUSY;
	}
	cpuidle_curr_driver = drv;
	spin_unlock(&cpuidle_driver_lock);

	return 0;
}

EXPORT_SYMBOL_GPL(cpuidle_register_driver);


void cpuidle_unregister_driver(struct cpuidle_driver *drv)
{
	if (!drv)
		return;

	spin_lock(&cpuidle_driver_lock);
	cpuidle_curr_driver = NULL;
	spin_unlock(&cpuidle_driver_lock);
}

EXPORT_SYMBOL_GPL(cpuidle_unregister_driver);
