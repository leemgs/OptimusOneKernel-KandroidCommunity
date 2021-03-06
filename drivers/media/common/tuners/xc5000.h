

#ifndef __XC5000_H__
#define __XC5000_H__

#include <linux/firmware.h>

struct dvb_frontend;
struct i2c_adapter;

struct xc5000_config {
	u8   i2c_address;
	u32  if_khz;
};


#define XC5000_TUNER_RESET		0



#if defined(CONFIG_MEDIA_TUNER_XC5000) || \
    (defined(CONFIG_MEDIA_TUNER_XC5000_MODULE) && defined(MODULE))
extern struct dvb_frontend *xc5000_attach(struct dvb_frontend *fe,
					  struct i2c_adapter *i2c,
					  struct xc5000_config *cfg);
#else
static inline struct dvb_frontend *xc5000_attach(struct dvb_frontend *fe,
						 struct i2c_adapter *i2c,
						 struct xc5000_config *cfg)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
#endif

#endif
