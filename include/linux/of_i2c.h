

#ifndef __LINUX_OF_I2C_H
#define __LINUX_OF_I2C_H

#include <linux/i2c.h>

void of_register_i2c_devices(struct i2c_adapter *adap,
			     struct device_node *adap_node);


struct i2c_client *of_find_i2c_device_by_node(struct device_node *node);

#endif 
