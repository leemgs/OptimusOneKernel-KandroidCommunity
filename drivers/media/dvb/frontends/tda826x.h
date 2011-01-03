  

#ifndef __DVB_TDA826X_H__
#define __DVB_TDA826X_H__

#include <linux/i2c.h>
#include "dvb_frontend.h"


#if defined(CONFIG_DVB_TDA826X) || (defined(CONFIG_DVB_TDA826X_MODULE) && defined(MODULE))
extern struct dvb_frontend* tda826x_attach(struct dvb_frontend *fe, int addr,
					   struct i2c_adapter *i2c,
					   int has_loopthrough);
#else
static inline struct dvb_frontend* tda826x_attach(struct dvb_frontend *fe,
						  int addr,
						  struct i2c_adapter *i2c,
						  int has_loopthrough)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
#endif 

#endif 
