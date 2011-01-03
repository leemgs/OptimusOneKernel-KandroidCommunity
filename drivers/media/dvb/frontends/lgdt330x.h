

#ifndef LGDT330X_H
#define LGDT330X_H

#include <linux/dvb/frontend.h>

typedef enum lg_chip_t {
		UNDEFINED,
		LGDT3302,
		LGDT3303
}lg_chip_type;

struct lgdt330x_config
{
	
	u8 demod_address;

	
	lg_chip_type demod_chip;

	
	int serial_mpeg;

	
	int (*pll_rf_set) (struct dvb_frontend* fe, int index);

	
	int (*set_ts_params)(struct dvb_frontend* fe, int is_punctured);

	
	int clock_polarity_flip;
};

#if defined(CONFIG_DVB_LGDT330X) || (defined(CONFIG_DVB_LGDT330X_MODULE) && defined(MODULE))
extern struct dvb_frontend* lgdt330x_attach(const struct lgdt330x_config* config,
					    struct i2c_adapter* i2c);
#else
static inline struct dvb_frontend* lgdt330x_attach(const struct lgdt330x_config* config,
					    struct i2c_adapter* i2c)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
#endif 

#endif 


