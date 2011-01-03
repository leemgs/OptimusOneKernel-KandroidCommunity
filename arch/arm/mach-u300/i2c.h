

#ifndef MACH_U300_I2C_H
#define MACH_U300_I2C_H

#ifdef CONFIG_I2C_STU300
void __init u300_i2c_register_board_devices(void);
#else

static inline void __init u300_i2c_register_board_devices(void)
{
}
#endif

#endif
