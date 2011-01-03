
#ifndef __KP_FLIP_SWITCH_H_
#define __KP_FLIP_SWITCH_H_

struct flip_switch_pdata {
	int flip_gpio;
	int left_key;
	int right_key;
	int wakeup;
	int active_low;
	int (*flip_mpp_config) (void);
	char name[25];
};
#endif
