

#ifndef _LINUX_I2C_OCORES_H
#define _LINUX_I2C_OCORES_H

struct ocores_i2c_platform_data {
	u32 regstep;   
	u32 clock_khz; 
	u8 num_devices; 
	struct i2c_board_info const *devices; 
};

#endif 
