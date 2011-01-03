

#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/slab.h>

#include "dvb_frontend.h"
#include "tda1002x.h"


struct tda10021_state {
	struct i2c_adapter* i2c;
	
	const struct tda1002x_config* config;
	struct dvb_frontend frontend;

	u8 pwm;
	u8 reg0;
};


#if 0
#define dprintk(x...) printk(x)
#else
#define dprintk(x...)
#endif

static int verbose;

#define XIN 57840000UL

#define FIN (XIN >> 4)

static int tda10021_inittab_size = 0x40;
static u8 tda10021_inittab[0x40]=
{
	0x73, 0x6a, 0x23, 0x0a, 0x02, 0x37, 0x77, 0x1a,
	0x37, 0x6a, 0x17, 0x8a, 0x1e, 0x86, 0x43, 0x40,
	0xb8, 0x3f, 0xa1, 0x00, 0xcd, 0x01, 0x00, 0xff,
	0x11, 0x00, 0x7c, 0x31, 0x30, 0x20, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x7d, 0x00, 0x00, 0x00, 0x00,
	0x07, 0x00, 0x33, 0x11, 0x0d, 0x95, 0x08, 0x58,
	0x00, 0x00, 0x80, 0x00, 0x80, 0xff, 0x00, 0x00,
	0x04, 0x2d, 0x2f, 0xff, 0x00, 0x00, 0x00, 0x00,
};

static int _tda10021_writereg (struct tda10021_state* state, u8 reg, u8 data)
{
	u8 buf[] = { reg, data };
	struct i2c_msg msg = { .addr = state->config->demod_address, .flags = 0, .buf = buf, .len = 2 };
	int ret;

	ret = i2c_transfer (state->i2c, &msg, 1);
	if (ret != 1)
		printk("DVB: TDA10021(%d): %s, writereg error "
			"(reg == 0x%02x, val == 0x%02x, ret == %i)\n",
			state->frontend.dvb->num, __func__, reg, data, ret);

	msleep(10);
	return (ret != 1) ? -EREMOTEIO : 0;
}

static u8 tda10021_readreg (struct tda10021_state* state, u8 reg)
{
	u8 b0 [] = { reg };
	u8 b1 [] = { 0 };
	struct i2c_msg msg [] = { { .addr = state->config->demod_address, .flags = 0, .buf = b0, .len = 1 },
				  { .addr = state->config->demod_address, .flags = I2C_M_RD, .buf = b1, .len = 1 } };
	int ret;

	ret = i2c_transfer (state->i2c, msg, 2);
	
	if (ret != 2 && reg != 0x1a)
		printk("DVB: TDA10021: %s: readreg error (ret == %i)\n",
				__func__, ret);
	return b1[0];
}


static int lock_tuner(struct tda10021_state* state)
{
	u8 buf[2] = { 0x0f, tda10021_inittab[0x0f] | 0x80 };
	struct i2c_msg msg = {.addr=state->config->demod_address, .flags=0, .buf=buf, .len=2};

	if(i2c_transfer(state->i2c, &msg, 1) != 1)
	{
		printk("tda10021: lock tuner fails\n");
		return -EREMOTEIO;
	}
	return 0;
}


static int unlock_tuner(struct tda10021_state* state)
{
	u8 buf[2] = { 0x0f, tda10021_inittab[0x0f] & 0x7f };
	struct i2c_msg msg_post={.addr=state->config->demod_address, .flags=0, .buf=buf, .len=2};

	if(i2c_transfer(state->i2c, &msg_post, 1) != 1)
	{
		printk("tda10021: unlock tuner fails\n");
		return -EREMOTEIO;
	}
	return 0;
}

static int tda10021_setup_reg0 (struct tda10021_state* state, u8 reg0,
				fe_spectral_inversion_t inversion)
{
	reg0 |= state->reg0 & 0x63;

	if ((INVERSION_ON == inversion) ^ (state->config->invert == 0))
		reg0 &= ~0x20;
	else
		reg0 |= 0x20;

	_tda10021_writereg (state, 0x00, reg0 & 0xfe);
	_tda10021_writereg (state, 0x00, reg0 | 0x01);

	state->reg0 = reg0;
	return 0;
}

static int tda10021_set_symbolrate (struct tda10021_state* state, u32 symbolrate)
{
	s32 BDR;
	s32 BDRI;
	s16 SFIL=0;
	u16 NDEC = 0;
	u32 tmp, ratio;

	if (symbolrate > XIN/2)
		symbolrate = XIN/2;
	if (symbolrate < 500000)
		symbolrate = 500000;

	if (symbolrate < XIN/16) NDEC = 1;
	if (symbolrate < XIN/32) NDEC = 2;
	if (symbolrate < XIN/64) NDEC = 3;

	if (symbolrate < (u32)(XIN/12.3)) SFIL = 1;
	if (symbolrate < (u32)(XIN/16))	 SFIL = 0;
	if (symbolrate < (u32)(XIN/24.6)) SFIL = 1;
	if (symbolrate < (u32)(XIN/32))	 SFIL = 0;
	if (symbolrate < (u32)(XIN/49.2)) SFIL = 1;
	if (symbolrate < (u32)(XIN/64))	 SFIL = 0;
	if (symbolrate < (u32)(XIN/98.4)) SFIL = 1;

	symbolrate <<= NDEC;
	ratio = (symbolrate << 4) / FIN;
	tmp =  ((symbolrate << 4) % FIN) << 8;
	ratio = (ratio << 8) + tmp / FIN;
	tmp = (tmp % FIN) << 8;
	ratio = (ratio << 8) + DIV_ROUND_CLOSEST(tmp, FIN);

	BDR = ratio;
	BDRI = (((XIN << 5) / symbolrate) + 1) / 2;

	if (BDRI > 0xFF)
		BDRI = 0xFF;

	SFIL = (SFIL << 4) | tda10021_inittab[0x0E];

	NDEC = (NDEC << 6) | tda10021_inittab[0x03];

	_tda10021_writereg (state, 0x03, NDEC);
	_tda10021_writereg (state, 0x0a, BDR&0xff);
	_tda10021_writereg (state, 0x0b, (BDR>> 8)&0xff);
	_tda10021_writereg (state, 0x0c, (BDR>>16)&0x3f);

	_tda10021_writereg (state, 0x0d, BDRI);
	_tda10021_writereg (state, 0x0e, SFIL);

	return 0;
}

static int tda10021_init (struct dvb_frontend *fe)
{
	struct tda10021_state* state = fe->demodulator_priv;
	int i;

	dprintk("DVB: TDA10021(%d): init chip\n", fe->adapter->num);

	

	for (i=0; i<tda10021_inittab_size; i++)
		_tda10021_writereg (state, i, tda10021_inittab[i]);

	_tda10021_writereg (state, 0x34, state->pwm);

	
	
	
	
	

	
	_tda10021_writereg(state, 0x2a, tda10021_inittab[0x2a] & 0xef);
	return 0;
}

static int tda10021_set_parameters (struct dvb_frontend *fe,
			    struct dvb_frontend_parameters *p)
{
	struct tda10021_state* state = fe->demodulator_priv;

	
	
	static const u8 reg0x00 [] = { 0x14, 0x00, 0x04, 0x08, 0x0c,  0x10 };
	
	static const u8 reg0x01 [] = { 0x78, 0x8c, 0x8c, 0x6a, 0x78,  0x5c };
	
	static const u8 reg0x05 [] = { 0x78, 0x87, 0x64, 0x46, 0x36,  0x26 };
	
	static const u8 reg0x08 [] = { 0x8c, 0xa2, 0x74, 0x43, 0x34,  0x23 };
	
	static const u8 reg0x09 [] = { 0x96, 0x91, 0x96, 0x6a, 0x7e,  0x6b };

	int qam = p->u.qam.modulation;

	if (qam < 0 || qam > 5)
		return -EINVAL;

	if (p->inversion != INVERSION_ON && p->inversion != INVERSION_OFF)
		return -EINVAL;

	

	if (fe->ops.tuner_ops.set_params) {
		fe->ops.tuner_ops.set_params(fe, p);
		if (fe->ops.i2c_gate_ctrl) fe->ops.i2c_gate_ctrl(fe, 0);
	}

	tda10021_set_symbolrate (state, p->u.qam.symbol_rate);
	_tda10021_writereg (state, 0x34, state->pwm);

	_tda10021_writereg (state, 0x01, reg0x01[qam]);
	_tda10021_writereg (state, 0x05, reg0x05[qam]);
	_tda10021_writereg (state, 0x08, reg0x08[qam]);
	_tda10021_writereg (state, 0x09, reg0x09[qam]);

	tda10021_setup_reg0 (state, reg0x00[qam], p->inversion);

	return 0;
}

static int tda10021_read_status(struct dvb_frontend* fe, fe_status_t* status)
{
	struct tda10021_state* state = fe->demodulator_priv;
	int sync;

	*status = 0;
	
	
	
	
	
	sync = tda10021_readreg (state, 0x11);

	if (sync & 2)
		*status |= FE_HAS_SIGNAL|FE_HAS_CARRIER;

	if (sync & 4)
		*status |= FE_HAS_SYNC|FE_HAS_VITERBI;

	if (sync & 8)
		*status |= FE_HAS_LOCK;

	return 0;
}

static int tda10021_read_ber(struct dvb_frontend* fe, u32* ber)
{
	struct tda10021_state* state = fe->demodulator_priv;

	u32 _ber = tda10021_readreg(state, 0x14) |
		(tda10021_readreg(state, 0x15) << 8) |
		((tda10021_readreg(state, 0x16) & 0x0f) << 16);
	_tda10021_writereg(state, 0x10, (tda10021_readreg(state, 0x10) & ~0xc0)
					| (tda10021_inittab[0x10] & 0xc0));
	*ber = 10 * _ber;

	return 0;
}

static int tda10021_read_signal_strength(struct dvb_frontend* fe, u16* strength)
{
	struct tda10021_state* state = fe->demodulator_priv;

	u8 config = tda10021_readreg(state, 0x02);
	u8 gain = tda10021_readreg(state, 0x17);
	if (config & 0x02)
		
		gain = ~gain;
	*strength = (gain << 8) | gain;

	return 0;
}

static int tda10021_read_snr(struct dvb_frontend* fe, u16* snr)
{
	struct tda10021_state* state = fe->demodulator_priv;

	u8 quality = ~tda10021_readreg(state, 0x18);
	*snr = (quality << 8) | quality;

	return 0;
}

static int tda10021_read_ucblocks(struct dvb_frontend* fe, u32* ucblocks)
{
	struct tda10021_state* state = fe->demodulator_priv;

	*ucblocks = tda10021_readreg (state, 0x13) & 0x7f;
	if (*ucblocks == 0x7f)
		*ucblocks = 0xffffffff;

	
	_tda10021_writereg (state, 0x10, tda10021_inittab[0x10] & 0xdf);
	_tda10021_writereg (state, 0x10, tda10021_inittab[0x10]);

	return 0;
}

static int tda10021_get_frontend(struct dvb_frontend* fe, struct dvb_frontend_parameters *p)
{
	struct tda10021_state* state = fe->demodulator_priv;
	int sync;
	s8 afc = 0;

	sync = tda10021_readreg(state, 0x11);
	afc = tda10021_readreg(state, 0x19);
	if (verbose) {
		
		printk(sync & 2 ? "DVB: TDA10021(%d): AFC (%d) %dHz\n" :
				  "DVB: TDA10021(%d): [AFC (%d) %dHz]\n",
			state->frontend.dvb->num, afc,
		       -((s32)p->u.qam.symbol_rate * afc) >> 10);
	}

	p->inversion = ((state->reg0 & 0x20) == 0x20) ^ (state->config->invert != 0) ? INVERSION_ON : INVERSION_OFF;
	p->u.qam.modulation = ((state->reg0 >> 2) & 7) + QAM_16;

	p->u.qam.fec_inner = FEC_NONE;
	p->frequency = ((p->frequency + 31250) / 62500) * 62500;

	if (sync & 2)
		p->frequency -= ((s32)p->u.qam.symbol_rate * afc) >> 10;

	return 0;
}

static int tda10021_i2c_gate_ctrl(struct dvb_frontend* fe, int enable)
{
	struct tda10021_state* state = fe->demodulator_priv;

	if (enable) {
		lock_tuner(state);
	} else {
		unlock_tuner(state);
	}
	return 0;
}

static int tda10021_sleep(struct dvb_frontend* fe)
{
	struct tda10021_state* state = fe->demodulator_priv;

	_tda10021_writereg (state, 0x1b, 0x02);  
	_tda10021_writereg (state, 0x00, 0x80);  

	return 0;
}

static void tda10021_release(struct dvb_frontend* fe)
{
	struct tda10021_state* state = fe->demodulator_priv;
	kfree(state);
}

static struct dvb_frontend_ops tda10021_ops;

struct dvb_frontend* tda10021_attach(const struct tda1002x_config* config,
				     struct i2c_adapter* i2c,
				     u8 pwm)
{
	struct tda10021_state* state = NULL;
	u8 id;

	
	state = kzalloc(sizeof(struct tda10021_state), GFP_KERNEL);
	if (state == NULL) goto error;

	
	state->config = config;
	state->i2c = i2c;
	state->pwm = pwm;
	state->reg0 = tda10021_inittab[0];

	
	id = tda10021_readreg(state, 0x1a);
	if ((id & 0xf0) != 0x70) goto error;

	printk("TDA10021: i2c-addr = 0x%02x, id = 0x%02x\n",
	       state->config->demod_address, id);

	
	memcpy(&state->frontend.ops, &tda10021_ops, sizeof(struct dvb_frontend_ops));
	state->frontend.demodulator_priv = state;
	return &state->frontend;

error:
	kfree(state);
	return NULL;
}

static struct dvb_frontend_ops tda10021_ops = {

	.info = {
		.name = "Philips TDA10021 DVB-C",
		.type = FE_QAM,
		.frequency_stepsize = 62500,
		.frequency_min = 47000000,
		.frequency_max = 862000000,
		.symbol_rate_min = (XIN/2)/64,     
		.symbol_rate_max = (XIN/2)/4,      
	#if 0
		.frequency_tolerance = ???,
		.symbol_rate_tolerance = ???,    
	#endif
		.caps = 0x400 | 
			FE_CAN_QAM_16 | FE_CAN_QAM_32 | FE_CAN_QAM_64 |
			FE_CAN_QAM_128 | FE_CAN_QAM_256 |
			FE_CAN_FEC_AUTO
	},

	.release = tda10021_release,

	.init = tda10021_init,
	.sleep = tda10021_sleep,
	.i2c_gate_ctrl = tda10021_i2c_gate_ctrl,

	.set_frontend = tda10021_set_parameters,
	.get_frontend = tda10021_get_frontend,

	.read_status = tda10021_read_status,
	.read_ber = tda10021_read_ber,
	.read_signal_strength = tda10021_read_signal_strength,
	.read_snr = tda10021_read_snr,
	.read_ucblocks = tda10021_read_ucblocks,
};

module_param(verbose, int, 0644);
MODULE_PARM_DESC(verbose, "print AFC offset after tuning for debugging the PWM setting");

MODULE_DESCRIPTION("Philips TDA10021 DVB-C demodulator driver");
MODULE_AUTHOR("Ralph Metzler, Holger Waechtler, Markus Schulz");
MODULE_LICENSE("GPL");

EXPORT_SYMBOL(tda10021_attach);
