  

#ifndef __DVB_STB6000_H__
#define __DVB_STB6000_H__

#include <linux/i2c.h>
#include "dvb_frontend.h"


#if defined(CONFIG_DVB_STB6000) || (defined(CONFIG_DVB_STB6000_MODULE) \
							&& defined(MODULE))
extern struct dvb_frontend *stb6000_attach(struct dvb_frontend *fe, int addr,
					   struct i2c_adapter *i2c);
#else
static inline struct dvb_frontend *stb6000_attach(struct dvb_frontend *fe,
						  int addr,
						  struct i2c_adapter *i2c)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
#endif 

#endif 
