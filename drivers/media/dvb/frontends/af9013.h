

#ifndef _AF9013_H_
#define _AF9013_H_

#include <linux/dvb/frontend.h>

enum af9013_ts_mode {
	AF9013_OUTPUT_MODE_PARALLEL,
	AF9013_OUTPUT_MODE_SERIAL,
	AF9013_OUTPUT_MODE_USB, 
};

enum af9013_tuner {
	AF9013_TUNER_MXL5003D   =   3, 
	AF9013_TUNER_MXL5005D   =  13, 
	AF9013_TUNER_MXL5005R   =  30, 
	AF9013_TUNER_ENV77H11D5 = 129, 
	AF9013_TUNER_MT2060     = 130, 
	AF9013_TUNER_MC44S803   = 133, 
	AF9013_TUNER_QT1010     = 134, 
	AF9013_TUNER_UNKNOWN    = 140, 
	AF9013_TUNER_MT2060_2   = 147, 
	AF9013_TUNER_TDA18271   = 156, 
	AF9013_TUNER_QT1010A    = 162, 
};


#define AF9013_GPIO_ON (1 << 0)
#define AF9013_GPIO_EN (1 << 1)
#define AF9013_GPIO_O  (1 << 2)
#define AF9013_GPIO_I  (1 << 3)

#define AF9013_GPIO_LO (AF9013_GPIO_ON|AF9013_GPIO_EN)
#define AF9013_GPIO_HI (AF9013_GPIO_ON|AF9013_GPIO_EN|AF9013_GPIO_O)

#define AF9013_GPIO_TUNER_ON  (AF9013_GPIO_ON|AF9013_GPIO_EN)
#define AF9013_GPIO_TUNER_OFF (AF9013_GPIO_ON|AF9013_GPIO_EN|AF9013_GPIO_O)

struct af9013_config {
	
	u8 demod_address;

	
	u32 adc_clock;

	
	u8 tuner;

	
	u16 tuner_if;

	
	u8 output_mode:2;

	
	u8 rf_spec_inv:1;

	
	u8 api_version[4];

	
	u8 gpio[4];
};


#if defined(CONFIG_DVB_AF9013) || \
	(defined(CONFIG_DVB_AF9013_MODULE) && defined(MODULE))
extern struct dvb_frontend *af9013_attach(const struct af9013_config *config,
	struct i2c_adapter *i2c);
#else
static inline struct dvb_frontend *af9013_attach(
const struct af9013_config *config, struct i2c_adapter *i2c)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
#endif 

#endif 
