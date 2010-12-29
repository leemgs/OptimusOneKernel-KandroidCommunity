


#ifndef __MSM_BATTERY_THUNDERC_H__
#define __MSM_BATTERY_THUNDERC_H__

#include <linux/power_supply.h>

struct batt_info {
	u32 valid_batt_id;
	u32 batt_therm;
	u32 batt_temp;
};

struct pseudo_batt_info_type {
	int mode;
	int id;
	int therm;
	int temp;
	int volt;
	int capacity;
	int charging;
};

enum {
	POWER_SUPPLY_PROP_BATTERY_ID_CHECK = POWER_SUPPLY_PROP_SERIAL_NUMBER + 1,
	POWER_SUPPLY_PROP_BATTERY_TEMP_ADC,
	POWER_SUPPLY_PROP_PSEUDO_BATT,
	POWER_SUPPLY_PROP_BLOCK_CHARGING,
};

#endif
