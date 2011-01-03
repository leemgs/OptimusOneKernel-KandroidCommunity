
#ifndef DIB0070_H
#define DIB0070_H

struct dvb_frontend;
struct i2c_adapter;

#define DEFAULT_DIB0070_I2C_ADDRESS 0x60

struct dib0070_wbd_gain_cfg {
	u16 freq;
	u16 wbd_gain_val;
};

struct dib0070_config {
	u8 i2c_address;

	
	int (*reset) (struct dvb_frontend *, int);
	int (*sleep) (struct dvb_frontend *, int);

	
	int freq_offset_khz_uhf;
	int freq_offset_khz_vhf;

	u8 osc_buffer_state;	
	u32 clock_khz;
	u8 clock_pad_drive;	

	u8 invert_iq;		

	u8 force_crystal_mode;	

	u8 flip_chip;
	u8 enable_third_order_filter;
	u8 charge_pump;

	const struct dib0070_wbd_gain_cfg *wbd_gain;

	u8 vga_filter;
};

#if defined(CONFIG_DVB_TUNER_DIB0070) || (defined(CONFIG_DVB_TUNER_DIB0070_MODULE) && defined(MODULE))
extern struct dvb_frontend *dib0070_attach(struct dvb_frontend *fe, struct i2c_adapter *i2c, struct dib0070_config *cfg);
extern u16 dib0070_wbd_offset(struct dvb_frontend *);
extern void dib0070_ctrl_agc_filter(struct dvb_frontend *, u8 open);
#else
static inline struct dvb_frontend *dib0070_attach(struct dvb_frontend *fe, struct i2c_adapter *i2c, struct dib0070_config *cfg)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return NULL;
}

static inline u16 dib0070_wbd_offset(struct dvb_frontend *fe)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
	return -ENODEV;
}

static inline void dib0070_ctrl_agc_filter(struct dvb_frontend *fe, u8 open)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __func__);
}
#endif

#endif
