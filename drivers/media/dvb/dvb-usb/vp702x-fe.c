
#include "vp702x.h"

struct vp702x_fe_state {
	struct dvb_frontend fe;
	struct dvb_usb_device *d;

	struct dvb_frontend_ops ops;

	fe_sec_voltage_t voltage;
	fe_sec_tone_mode_t tone_mode;

	u8 lnb_buf[8];

	u8 lock;
	u8 sig;
	u8 snr;

	unsigned long next_status_check;
	unsigned long status_check_interval;
};

static int vp702x_fe_refresh_state(struct vp702x_fe_state *st)
{
	u8 buf[10];
	if (time_after(jiffies,st->next_status_check)) {
		vp702x_usb_in_op(st->d,READ_STATUS,0,0,buf,10);

		st->lock = buf[4];
		vp702x_usb_in_op(st->d,READ_TUNER_REG_REQ,0x11,0,&st->snr,1);
		vp702x_usb_in_op(st->d,READ_TUNER_REG_REQ,0x15,0,&st->sig,1);

		st->next_status_check = jiffies + (st->status_check_interval*HZ)/1000;
	}
	return 0;
}

static u8 vp702x_chksum(u8 *buf,int f, int count)
{
	u8 s = 0;
	int i;
	for (i = f; i < f+count; i++)
		s += buf[i];
	return ~s+1;
}

static int vp702x_fe_read_status(struct dvb_frontend* fe, fe_status_t *status)
{
	struct vp702x_fe_state *st = fe->demodulator_priv;
	vp702x_fe_refresh_state(st);
	deb_fe("%s\n",__func__);

	if (st->lock == 0)
		*status = FE_HAS_LOCK | FE_HAS_SYNC | FE_HAS_VITERBI | FE_HAS_SIGNAL | FE_HAS_CARRIER;
	else
		*status = 0;

	if (*status & FE_HAS_LOCK)
		st->status_check_interval = 1000;
	else
		st->status_check_interval = 250;
	return 0;
}


static int vp702x_fe_read_ber(struct dvb_frontend* fe, u32 *ber)
{
	struct vp702x_fe_state *st = fe->demodulator_priv;
	vp702x_fe_refresh_state(st);
	*ber = 0;
	return 0;
}


static int vp702x_fe_read_unc_blocks(struct dvb_frontend* fe, u32 *unc)
{
	struct vp702x_fe_state *st = fe->demodulator_priv;
	vp702x_fe_refresh_state(st);
	*unc = 0;
	return 0;
}

static int vp702x_fe_read_signal_strength(struct dvb_frontend* fe, u16 *strength)
{
	struct vp702x_fe_state *st = fe->demodulator_priv;
	vp702x_fe_refresh_state(st);

	*strength = (st->sig << 8) | st->sig;
	return 0;
}

static int vp702x_fe_read_snr(struct dvb_frontend* fe, u16 *snr)
{
	u8 _snr;
	struct vp702x_fe_state *st = fe->demodulator_priv;
	vp702x_fe_refresh_state(st);

	_snr = (st->snr & 0x1f) * 0xff / 0x1f;
	*snr = (_snr << 8) | _snr;
	return 0;
}

static int vp702x_fe_get_tune_settings(struct dvb_frontend* fe, struct dvb_frontend_tune_settings *tune)
{
	deb_fe("%s\n",__func__);
	tune->min_delay_ms = 2000;
	return 0;
}

static int vp702x_fe_set_frontend(struct dvb_frontend* fe,
				  struct dvb_frontend_parameters *fep)
{
	struct vp702x_fe_state *st = fe->demodulator_priv;
	u32 freq = fep->frequency/1000;
	

	u64 sr;
	u8 cmd[8] = { 0 },ibuf[10];

	cmd[0] = (freq >> 8) & 0x7f;
	cmd[1] =  freq       & 0xff;
	cmd[2] = 1; 

	sr = (u64) (fep->u.qpsk.symbol_rate/1000) << 20;
	do_div(sr,88000);
	cmd[3] = (sr >> 12) & 0xff;
	cmd[4] = (sr >> 4)  & 0xff;
	cmd[5] = (sr << 4)  & 0xf0;

	deb_fe("setting frontend to: %u -> %u (%x) LNB-based GHz, symbolrate: %d -> %lu (%lx)\n",
			fep->frequency,freq,freq, fep->u.qpsk.symbol_rate,
			(unsigned long) sr, (unsigned long) sr);



	if (st->voltage == SEC_VOLTAGE_18)
		cmd[6] |= 0x40;



	cmd[7] = vp702x_chksum(cmd,0,7);

	st->status_check_interval = 250;
	st->next_status_check = jiffies;

	vp702x_usb_inout_op(st->d,cmd,8,ibuf,10,100);

	if (ibuf[2] == 0 && ibuf[3] == 0)
		deb_fe("tuning failed.\n");
	else
		deb_fe("tuning succeeded.\n");

	return 0;
}

static int vp702x_fe_init(struct dvb_frontend *fe)
{
	struct vp702x_fe_state *st = fe->demodulator_priv;
	deb_fe("%s\n",__func__);
	vp702x_usb_in_op(st->d, RESET_TUNER, 0, 0, NULL, 0);
	return 0;
}

static int vp702x_fe_sleep(struct dvb_frontend *fe)
{
	deb_fe("%s\n",__func__);
	return 0;
}

static int vp702x_fe_get_frontend(struct dvb_frontend* fe,
				  struct dvb_frontend_parameters *fep)
{
	deb_fe("%s\n",__func__);
	return 0;
}

static int vp702x_fe_send_diseqc_msg (struct dvb_frontend* fe,
				    struct dvb_diseqc_master_cmd *m)
{
	struct vp702x_fe_state *st = fe->demodulator_priv;
	u8 cmd[8],ibuf[10];
	memset(cmd,0,8);

	deb_fe("%s\n",__func__);

	if (m->msg_len > 4)
		return -EINVAL;

	cmd[1] = SET_DISEQC_CMD;
	cmd[2] = m->msg_len;
	memcpy(&cmd[3], m->msg, m->msg_len);
	cmd[7] = vp702x_chksum(cmd,0,7);

	vp702x_usb_inout_op(st->d,cmd,8,ibuf,10,100);

	if (ibuf[2] == 0 && ibuf[3] == 0)
		deb_fe("diseqc cmd failed.\n");
	else
		deb_fe("diseqc cmd succeeded.\n");

	return 0;
}

static int vp702x_fe_send_diseqc_burst (struct dvb_frontend* fe, fe_sec_mini_cmd_t burst)
{
	deb_fe("%s\n",__func__);
	return 0;
}

static int vp702x_fe_set_tone(struct dvb_frontend* fe, fe_sec_tone_mode_t tone)
{
	struct vp702x_fe_state *st = fe->demodulator_priv;
	u8 ibuf[10];
	deb_fe("%s\n",__func__);

	st->tone_mode = tone;

	if (tone == SEC_TONE_ON)
		st->lnb_buf[2] = 0x02;
	else
		st->lnb_buf[2] = 0x00;

	st->lnb_buf[7] = vp702x_chksum(st->lnb_buf,0,7);

	vp702x_usb_inout_op(st->d,st->lnb_buf,8,ibuf,10,100);
	if (ibuf[2] == 0 && ibuf[3] == 0)
		deb_fe("set_tone cmd failed.\n");
	else
		deb_fe("set_tone cmd succeeded.\n");

	return 0;
}

static int vp702x_fe_set_voltage (struct dvb_frontend* fe, fe_sec_voltage_t
		voltage)
{
	struct vp702x_fe_state *st = fe->demodulator_priv;
	u8 ibuf[10];
	deb_fe("%s\n",__func__);

	st->voltage = voltage;

	if (voltage != SEC_VOLTAGE_OFF)
		st->lnb_buf[4] = 0x01;
	else
		st->lnb_buf[4] = 0x00;

	st->lnb_buf[7] = vp702x_chksum(st->lnb_buf,0,7);

	vp702x_usb_inout_op(st->d,st->lnb_buf,8,ibuf,10,100);
	if (ibuf[2] == 0 && ibuf[3] == 0)
		deb_fe("set_voltage cmd failed.\n");
	else
		deb_fe("set_voltage cmd succeeded.\n");

	return 0;
}

static void vp702x_fe_release(struct dvb_frontend* fe)
{
	struct vp702x_fe_state *st = fe->demodulator_priv;
	kfree(st);
}

static struct dvb_frontend_ops vp702x_fe_ops;

struct dvb_frontend * vp702x_fe_attach(struct dvb_usb_device *d)
{
	struct vp702x_fe_state *s = kzalloc(sizeof(struct vp702x_fe_state), GFP_KERNEL);
	if (s == NULL)
		goto error;

	s->d = d;

	memcpy(&s->fe.ops,&vp702x_fe_ops,sizeof(struct dvb_frontend_ops));
	s->fe.demodulator_priv = s;

	s->lnb_buf[1] = SET_LNB_POWER;
	s->lnb_buf[3] = 0xff; 

	return &s->fe;
error:
	return NULL;
}


static struct dvb_frontend_ops vp702x_fe_ops = {
	.info = {
		.name           = "Twinhan DST-like frontend (VP7021/VP7020) DVB-S",
		.type           = FE_QPSK,
		.frequency_min       = 950000,
		.frequency_max       = 2150000,
		.frequency_stepsize  = 1000,   
		.frequency_tolerance = 0,
		.symbol_rate_min     = 1000000,
		.symbol_rate_max     = 45000000,
		.symbol_rate_tolerance = 500,  
		.caps = FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 | FE_CAN_FEC_3_4 |
		FE_CAN_FEC_5_6 | FE_CAN_FEC_7_8 |
		FE_CAN_QPSK |
		FE_CAN_FEC_AUTO
	},
	.release = vp702x_fe_release,

	.init  = vp702x_fe_init,
	.sleep = vp702x_fe_sleep,

	.set_frontend = vp702x_fe_set_frontend,
	.get_frontend = vp702x_fe_get_frontend,
	.get_tune_settings = vp702x_fe_get_tune_settings,

	.read_status = vp702x_fe_read_status,
	.read_ber = vp702x_fe_read_ber,
	.read_signal_strength = vp702x_fe_read_signal_strength,
	.read_snr = vp702x_fe_read_snr,
	.read_ucblocks = vp702x_fe_read_unc_blocks,

	.diseqc_send_master_cmd = vp702x_fe_send_diseqc_msg,
	.diseqc_send_burst = vp702x_fe_send_diseqc_burst,
	.set_tone = vp702x_fe_set_tone,
	.set_voltage = vp702x_fe_set_voltage,
};
