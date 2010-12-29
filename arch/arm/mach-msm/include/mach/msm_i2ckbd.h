

#ifndef _MSM_I2CKBD_H_
#define _MSM_I2CKBD_H_

struct msm_i2ckbd_platform_data {
	uint8_t hwrepeat;
	uint8_t scanset1;
	int  gpioreset;
	int  gpioirq;
	int  (*gpio_setup) (void);
	void (*gpio_shutdown)(void);
	void (*hw_reset) (int);
};

#endif
