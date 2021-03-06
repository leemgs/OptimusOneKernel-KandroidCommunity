

#ifndef __ISL_6423_H
#define __ISL_6423_H

#include <linux/dvb/frontend.h>

enum isl6423_current {
	SEC_CURRENT_275m = 0,
	SEC_CURRENT_515m,
	SEC_CURRENT_635m,
	SEC_CURRENT_800m,
};

enum isl6423_curlim {
	SEC_CURRENT_LIM_ON = 1,
	SEC_CURRENT_LIM_OFF
};

struct isl6423_config {
	enum isl6423_current current_max;
	enum isl6423_curlim curlim;
	u8 addr;
	u8 mod_extern;
};

#if defined(CONFIG_DVB_ISL6423) || (defined(CONFIG_DVB_ISL6423_MODULE) && defined(MODULE))


extern struct dvb_frontend *isl6423_attach(struct dvb_frontend *fe,
					   struct i2c_adapter *i2c,
					   const struct isl6423_config *config);

#else
static inline struct dvb_frontend *isl6423_attach(struct dvb_frontend *fe,
						  struct i2c_adapter *i2c,
						  const struct isl6423_config *config)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}

#endif 

#endif 
