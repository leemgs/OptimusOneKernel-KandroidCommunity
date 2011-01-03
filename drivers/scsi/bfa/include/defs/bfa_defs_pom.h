
#ifndef __BFA_DEFS_POM_H__
#define __BFA_DEFS_POM_H__

#include <bfa_os_inc.h>
#include <defs/bfa_defs_types.h>


enum bfa_pom_entry_health {
	BFA_POM_HEALTH_NOINFO  = 1,	
	BFA_POM_HEALTH_NORMAL  = 2,	
	BFA_POM_HEALTH_WARNING = 3,	
	BFA_POM_HEALTH_ALARM   = 4,	
};


struct bfa_pom_entry_s {
	enum bfa_pom_entry_health health;	
	u32        curr_value;	
	u32        thr_warn_high;	
	u32        thr_warn_low;	
	u32        thr_alarm_low;	
	u32        thr_alarm_high;	
};


struct bfa_pom_attr_s {
	struct bfa_pom_entry_s temperature;	
	struct bfa_pom_entry_s voltage;	
	struct bfa_pom_entry_s curr;	
	struct bfa_pom_entry_s txpower;	
	struct bfa_pom_entry_s rxpower;	
};

#endif 
