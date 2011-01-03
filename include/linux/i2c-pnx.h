

#ifndef __I2C_PNX_H__
#define __I2C_PNX_H__

#include <linux/pm.h>

struct platform_device;

struct i2c_pnx_mif {
	int			ret;		
	int			mode;		
	struct completion	complete;	
	struct timer_list	timer;		
	u8 *			buf;		
	int			len;		
};

struct i2c_pnx_algo_data {
	u32			base;
	u32			ioaddr;
	int			irq;
	struct i2c_pnx_mif	mif;
	int			last;
};

struct i2c_pnx_data {
	int (*suspend) (struct platform_device *pdev, pm_message_t state);
	int (*resume) (struct platform_device *pdev);
	u32 (*calculate_input_freq) (struct platform_device *pdev);
	int (*set_clock_run) (struct platform_device *pdev);
	int (*set_clock_stop) (struct platform_device *pdev);
	struct i2c_adapter *adapter;
};

#endif 
