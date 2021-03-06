

#ifndef __STV090x_H
#define __STV090x_H

enum stv090x_demodulator {
	STV090x_DEMODULATOR_0 = 1,
	STV090x_DEMODULATOR_1
};

enum stv090x_device {
	STV0903	=  0,
	STV0900,
};

enum stv090x_mode {
	STV090x_DUAL = 0,
	STV090x_SINGLE
};

enum stv090x_tsmode {
	STV090x_TSMODE_SERIAL_PUNCTURED	= 1,
	STV090x_TSMODE_SERIAL_CONTINUOUS,
	STV090x_TSMODE_PARALLEL_PUNCTURED,
	STV090x_TSMODE_DVBCI
};

enum stv090x_clkmode {
	STV090x_CLK_INT = 0, 
	STV090x_CLK_EXT = 2 
};

enum stv090x_i2crpt {
	STV090x_RPTLEVEL_256	= 0,
	STV090x_RPTLEVEL_128	= 1,
	STV090x_RPTLEVEL_64	= 2,
	STV090x_RPTLEVEL_32	= 3,
	STV090x_RPTLEVEL_16	= 4,
	STV090x_RPTLEVEL_8	= 5,
	STV090x_RPTLEVEL_4	= 6,
	STV090x_RPTLEVEL_2	= 7,
};

struct stv090x_config {
	enum stv090x_device	device;
	enum stv090x_mode	demod_mode;
	enum stv090x_clkmode	clk_mode;

	u32 xtal; 
	u8 address; 

	u32 ref_clk; 

	u8 ts1_mode;
	u8 ts2_mode;

	enum stv090x_i2crpt	repeater_level;

	int (*tuner_init) (struct dvb_frontend *fe);
	int (*tuner_set_mode) (struct dvb_frontend *fe, enum tuner_mode mode);
	int (*tuner_set_frequency) (struct dvb_frontend *fe, u32 frequency);
	int (*tuner_get_frequency) (struct dvb_frontend *fe, u32 *frequency);
	int (*tuner_set_bandwidth) (struct dvb_frontend *fe, u32 bandwidth);
	int (*tuner_get_bandwidth) (struct dvb_frontend *fe, u32 *bandwidth);
	int (*tuner_set_bbgain) (struct dvb_frontend *fe, u32 gain);
	int (*tuner_get_bbgain) (struct dvb_frontend *fe, u32 *gain);
	int (*tuner_set_refclk)  (struct dvb_frontend *fe, u32 refclk);
	int (*tuner_get_status) (struct dvb_frontend *fe, u32 *status);
};

#if defined(CONFIG_DVB_STV090x) || (defined(CONFIG_DVB_STV090x_MODULE) && defined(MODULE))

extern struct dvb_frontend *stv090x_attach(const struct stv090x_config *config,
					   struct i2c_adapter *i2c,
					   enum stv090x_demodulator demod);
#else

static inline struct dvb_frontend *stv090x_attach(const struct stv090x_config *config,
						  struct i2c_adapter *i2c,
						  enum stv090x_demodulator demod)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}
#endif 

#endif 
