
#ifndef __LINUX_I2C_SX150X_H
#define __LINUX_I2C_SX150X_H


struct sx150x_platform_data {
	unsigned gpio_base;
	bool     oscio_is_gpo;
	u16      io_pullup_ena;
	u16      io_pulldn_ena;
	u16      io_open_drain_ena;
	u16      io_polarity;
	int      irq_summary;
	unsigned irq_base;
};

#endif 
