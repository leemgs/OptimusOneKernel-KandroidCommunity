

#ifndef __LINUX_ISA1200_H
#define __LINUX_ISA1200_H

struct isa1200_platform_data {
	char name[30];
	int pwm_ch_id; 
	int hap_en_gpio; 
	int max_timeout;
	int (*power_on)(int on);
};

#endif 
