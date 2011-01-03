
#ifndef __debug_levels__h__
#define __debug_levels__h__


#define D_MODULENAME i2400m
#define D_MASTER CONFIG_WIMAX_I2400M_DEBUG_LEVEL

#include <linux/wimax/debug.h>


enum d_module {
	D_SUBMODULE_DECLARE(control),
	D_SUBMODULE_DECLARE(driver),
	D_SUBMODULE_DECLARE(debugfs),
	D_SUBMODULE_DECLARE(fw),
	D_SUBMODULE_DECLARE(netdev),
	D_SUBMODULE_DECLARE(rfkill),
	D_SUBMODULE_DECLARE(rx),
	D_SUBMODULE_DECLARE(sysfs),
	D_SUBMODULE_DECLARE(tx),
};


#endif 
