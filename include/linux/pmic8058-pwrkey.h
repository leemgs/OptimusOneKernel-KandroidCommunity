

#ifndef __PMIC8058_PWRKEY_H__
#define __PMIC8058_PWRKEY_H__

struct pmic8058_pwrkey_pdata {
	bool pull_up;
	
	u16  pwrkey_time_ms;
	
	u32  kpd_trigger_delay_us;
	u32  wakeup;
};

#endif 
