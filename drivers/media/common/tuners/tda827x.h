  

#ifndef __DVB_TDA827X_H__
#define __DVB_TDA827X_H__

#include <linux/i2c.h>
#include "dvb_frontend.h"

struct tda827x_config
{
	
	int (*init) (struct dvb_frontend *fe);
	int (*sleep) (struct dvb_frontend *fe);

	
	unsigned int config;
	int 	     switch_addr;

	void (*agcf)(struct dvb_frontend *fe);
};



#if defined(CONFIG_MEDIA_TUNER_TDA827X) || (defined(CONFIG_MEDIA_TUNER_TDA827X_MODULE) && defined(MODULE))
extern struct dvb_frontend* tda827x_attach(struct dvb_frontend *fe, int addr,
					   struct i2c_adapter *i2c,
					   struct tda827x_config *cfg);
#else
static inline struct dvb_frontend* tda827x_attach(struct dvb_frontend *fe,
						  int addr,
						  struct i2c_adapter *i2c,
						  struct tda827x_config *cfg)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
#endif 

#endif 
