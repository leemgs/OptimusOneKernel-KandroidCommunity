#ifndef __LINUX_I2C_TSC2007_H
#define __LINUX_I2C_TSC2007_H



struct tsc2007_platform_data {
	u16	model;				
	u16	x_plate_ohms;
	unsigned long irq_flags;
	bool	invert_x;
	bool	invert_y;
	bool	invert_z1;
	bool	invert_z2;

	int	(*get_pendown_state)(void);
	void	(*clear_penirq)(void);		
	int	(*init_platform_hw)(void);
	void	(*exit_platform_hw)(void);
};

#endif
