
#ifndef __PMIC8058_VIBRATOR_H__
#define __PMIC8058_VIBRATOR_H__

struct pmic8058_vibrator_pdata {
	int initial_vibrate_ms;
	int max_timeout_ms;

	int level_mV;
};

#endif 
