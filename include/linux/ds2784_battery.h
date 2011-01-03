

#ifndef __LINUX_DS2784_BATTERY_H
#define __LINUX_DS2784_BATTERY_H

#ifdef __KERNEL__

struct ds2784_platform_data {
	int (*charge)(int on, int fast);
	void *w1_slave;
};

#endif 

#endif 
