

#ifndef CX24116_H
#define CX24116_H

#include <linux/dvb/frontend.h>

struct cx24116_config {
	
	u8 demod_address;

	
	int (*set_ts_params)(struct dvb_frontend *fe, int is_punctured);

	
	int (*reset_device)(struct dvb_frontend *fe);

	
	u8 mpg_clk_pos_pol:0x02;
};

#if defined(CONFIG_DVB_CX24116) || \
	(defined(CONFIG_DVB_CX24116_MODULE) && defined(MODULE))
extern struct dvb_frontend *cx24116_attach(
	const struct cx24116_config *config,
	struct i2c_adapter *i2c);
#else
static inline struct dvb_frontend *cx24116_attach(
	const struct cx24116_config *config,
	struct i2c_adapter *i2c)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
#endif

#endif 
