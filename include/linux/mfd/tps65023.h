

#ifndef __LINUX_I2C_TPS65023_H
#define __LINUX_I2C_TPS65023_H

#ifndef CONFIG_TPS65023

#define tps65023_set_dcdc1_level(mvolts)  (-ENODEV)


#define tps65023_get_dcdc1_level(mvolts)  (-ENODEV)

#else

extern int tps65023_set_dcdc1_level(int mvolts);


extern int tps65023_get_dcdc1_level(int *mvolts);
#endif

#endif
