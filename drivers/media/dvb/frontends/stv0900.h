

#ifndef STV0900_H
#define STV0900_H

#include <linux/dvb/frontend.h>
#include "dvb_frontend.h"

struct stv0900_reg {
	u16 addr;
	u8  val;
};

struct stv0900_config {
	u8 demod_address;
	u32 xtal;
	u8 clkmode;

	u8 diseqc_mode;

	u8 path1_mode;
	u8 path2_mode;
	struct stv0900_reg *ts_config_regs;
	u8 tun1_maddress;
	u8 tun2_maddress;
	u8 tun1_adc;
	u8 tun2_adc;
};

#if defined(CONFIG_DVB_STV0900) || (defined(CONFIG_DVB_STV0900_MODULE) \
							&& defined(MODULE))
extern struct dvb_frontend *stv0900_attach(const struct stv0900_config *config,
					struct i2c_adapter *i2c, int demod);
#else
static inline struct dvb_frontend *stv0900_attach(const struct stv0900_config *config,
					struct i2c_adapter *i2c, int demod)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
#endif

#endif

