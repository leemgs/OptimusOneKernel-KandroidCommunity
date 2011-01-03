



#include <linux/hwmon.h>


#define LM75_TEMP_MIN (-55000)
#define LM75_TEMP_MAX 125000


static inline u16 LM75_TEMP_TO_REG(long temp)
{
	int ntemp = SENSORS_LIMIT(temp, LM75_TEMP_MIN, LM75_TEMP_MAX);
	ntemp += (ntemp<0 ? -250 : 250);
	return (u16)((ntemp / 500) << 7);
}

static inline int LM75_TEMP_FROM_REG(u16 reg)
{
	
	return ((s16)reg / 128) * 500;
}

