

#include <linux/device.h>
#include <linux/init.h>
#include <linux/memory.h>

#include "base.h"


void __init driver_init(void)
{
	
	devtmpfs_init();
	devices_init();
	buses_init();
	classes_init();
	firmware_init();
	hypervisor_init();

	
	platform_bus_init();
	system_bus_init();
	cpu_dev_init();
	memory_dev_init();
}
