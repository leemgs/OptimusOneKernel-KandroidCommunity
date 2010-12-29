
#ifndef _TPM_ST_I2C_H_
#define _TPM_ST_I2C_H_

struct tpm_st_i2c_platform_data {
	int accept_cmd_gpio;
	int data_avail_gpio;
	int accept_cmd_irq;
	int data_avail_irq;
	int (*gpio_setup)(void);
	void (*gpio_release)(void);
};

#endif
