

#include "anysee.h"
#include "tda1002x.h"
#include "mt352.h"
#include "mt352_priv.h"
#include "zl10353.h"


static int dvb_usb_anysee_debug;
module_param_named(debug, dvb_usb_anysee_debug, int, 0644);
MODULE_PARM_DESC(debug, "set debugging level" DVB_USB_DEBUG_STATUS);
static int dvb_usb_anysee_delsys;
module_param_named(delsys, dvb_usb_anysee_delsys, int, 0644);
MODULE_PARM_DESC(delsys, "select delivery mode (0=DVB-C, 1=DVB-T)");
DVB_DEFINE_MOD_OPT_ADAPTER_NR(adapter_nr);

static DEFINE_MUTEX(anysee_usb_mutex);

static int anysee_ctrl_msg(struct dvb_usb_device *d, u8 *sbuf, u8 slen,
	u8 *rbuf, u8 rlen)
{
	struct anysee_state *state = d->priv;
	int act_len, ret;
	u8 buf[64];

	if (slen > sizeof(buf))
		slen = sizeof(buf);
	memcpy(&buf[0], sbuf, slen);
	buf[60] = state->seq++;

	if (mutex_lock_interruptible(&anysee_usb_mutex) < 0)
		return -EAGAIN;

	
	ret = dvb_usb_generic_rw(d, buf, sizeof(buf), buf, sizeof(buf), 0);

	if (!ret) {
		
		ret = usb_bulk_msg(d->udev, usb_rcvbulkpipe(d->udev,
			d->props.generic_bulk_ctrl_endpoint), buf, sizeof(buf),
			&act_len, 2000);
		if (ret)
			err("%s: recv bulk message failed: %d", __func__, ret);
		else {
			deb_xfer("<<< ");
			debug_dump(buf, act_len, deb_xfer);
		}
	}

	
	if (!ret && rbuf && rlen)
		memcpy(rbuf, buf, rlen);

	mutex_unlock(&anysee_usb_mutex);

	return ret;
}

static int anysee_read_reg(struct dvb_usb_device *d, u16 reg, u8 *val)
{
	u8 buf[] = {CMD_REG_READ, reg >> 8, reg & 0xff, 0x01};
	int ret;
	ret = anysee_ctrl_msg(d, buf, sizeof(buf), val, 1);
	deb_info("%s: reg:%04x val:%02x\n", __func__, reg, *val);
	return ret;
}

static int anysee_write_reg(struct dvb_usb_device *d, u16 reg, u8 val)
{
	u8 buf[] = {CMD_REG_WRITE, reg >> 8, reg & 0xff, 0x01, val};
	deb_info("%s: reg:%04x val:%02x\n", __func__, reg, val);
	return anysee_ctrl_msg(d, buf, sizeof(buf), NULL, 0);
}

static int anysee_get_hw_info(struct dvb_usb_device *d, u8 *id)
{
	u8 buf[] = {CMD_GET_HW_INFO};
	return anysee_ctrl_msg(d, buf, sizeof(buf), id, 3);
}

static int anysee_streaming_ctrl(struct dvb_usb_adapter *adap, int onoff)
{
	u8 buf[] = {CMD_STREAMING_CTRL, (u8)onoff, 0x00};
	deb_info("%s: onoff:%02x\n", __func__, onoff);
	return anysee_ctrl_msg(adap->dev, buf, sizeof(buf), NULL, 0);
}

static int anysee_led_ctrl(struct dvb_usb_device *d, u8 mode, u8 interval)
{
	u8 buf[] = {CMD_LED_AND_IR_CTRL, 0x01, mode, interval};
	deb_info("%s: state:%02x interval:%02x\n", __func__, mode, interval);
	return anysee_ctrl_msg(d, buf, sizeof(buf), NULL, 0);
}

static int anysee_ir_ctrl(struct dvb_usb_device *d, u8 onoff)
{
	u8 buf[] = {CMD_LED_AND_IR_CTRL, 0x02, onoff};
	deb_info("%s: onoff:%02x\n", __func__, onoff);
	return anysee_ctrl_msg(d, buf, sizeof(buf), NULL, 0);
}

static int anysee_init(struct dvb_usb_device *d)
{
	int ret;
	
	ret = anysee_led_ctrl(d, 0x01, 0x03);
	if (ret)
		return ret;

	
	ret = anysee_ir_ctrl(d, 1);
	if (ret)
		return ret;

	return 0;
}


static int anysee_master_xfer(struct i2c_adapter *adap, struct i2c_msg *msg,
	int num)
{
	struct dvb_usb_device *d = i2c_get_adapdata(adap);
	int ret = 0, inc, i = 0;

	if (mutex_lock_interruptible(&d->i2c_mutex) < 0)
		return -EAGAIN;

	while (i < num) {
		if (num > i + 1 && (msg[i+1].flags & I2C_M_RD)) {
			u8 buf[6];
			buf[0] = CMD_I2C_READ;
			buf[1] = msg[i].addr + 1;
			buf[2] = msg[i].buf[0];
			buf[3] = 0x00;
			buf[4] = 0x00;
			buf[5] = 0x01;
			ret = anysee_ctrl_msg(d, buf, sizeof(buf), msg[i+1].buf,
				msg[i+1].len);
			inc = 2;
		} else {
			u8 buf[4+msg[i].len];
			buf[0] = CMD_I2C_WRITE;
			buf[1] = msg[i].addr;
			buf[2] = msg[i].len;
			buf[3] = 0x01;
			memcpy(&buf[4], msg[i].buf, msg[i].len);
			ret = anysee_ctrl_msg(d, buf, sizeof(buf), NULL, 0);
			inc = 1;
		}
		if (ret)
			break;

		i += inc;
	}

	mutex_unlock(&d->i2c_mutex);

	return ret ? ret : i;
}

static u32 anysee_i2c_func(struct i2c_adapter *adapter)
{
	return I2C_FUNC_I2C;
}

static struct i2c_algorithm anysee_i2c_algo = {
	.master_xfer   = anysee_master_xfer,
	.functionality = anysee_i2c_func,
};

static int anysee_mt352_demod_init(struct dvb_frontend *fe)
{
	static u8 clock_config[]   = { CLOCK_CTL,  0x38, 0x28 };
	static u8 reset[]          = { RESET,      0x80 };
	static u8 adc_ctl_1_cfg[]  = { ADC_CTL_1,  0x40 };
	static u8 agc_cfg[]        = { AGC_TARGET, 0x28, 0x20 };
	static u8 gpp_ctl_cfg[]    = { GPP_CTL,    0x33 };
	static u8 capt_range_cfg[] = { CAPT_RANGE, 0x32 };

	mt352_write(fe, clock_config,   sizeof(clock_config));
	udelay(200);
	mt352_write(fe, reset,          sizeof(reset));
	mt352_write(fe, adc_ctl_1_cfg,  sizeof(adc_ctl_1_cfg));

	mt352_write(fe, agc_cfg,        sizeof(agc_cfg));
	mt352_write(fe, gpp_ctl_cfg,    sizeof(gpp_ctl_cfg));
	mt352_write(fe, capt_range_cfg, sizeof(capt_range_cfg));

	return 0;
}


static struct tda10023_config anysee_tda10023_config = {
	.demod_address = 0x1a,
	.invert = 0,
	.xtal   = 16000000,
	.pll_m  = 11,
	.pll_p  = 3,
	.pll_n  = 1,
	.output_mode = TDA10023_OUTPUT_MODE_PARALLEL_C,
	.deltaf = 0xfeeb,
};

static struct mt352_config anysee_mt352_config = {
	.demod_address = 0x1e,
	.demod_init    = anysee_mt352_demod_init,
};

static struct zl10353_config anysee_zl10353_config = {
	.demod_address = 0x1e,
	.parallel_ts = 1,
};

static int anysee_frontend_attach(struct dvb_usb_adapter *adap)
{
	int ret;
	struct anysee_state *state = adap->dev->priv;
	u8 hw_info[3];
	u8 io_d; 

	
	ret = anysee_get_hw_info(adap->dev, hw_info);
	if (ret)
		return ret;
	ret = anysee_get_hw_info(adap->dev, hw_info);
	if (ret)
		return ret;

	
	info("firmware version:%d.%d.%d hardware id:%d",
		0, hw_info[1], hw_info[2], hw_info[0]);

	ret = anysee_read_reg(adap->dev, 0xb0, &io_d); 
	if (ret)
		return ret;
	deb_info("%s: IO port D:%02x\n", __func__, io_d);

	

	

	
	adap->fe = dvb_attach(mt352_attach, &anysee_mt352_config,
			      &adap->dev->i2c_adap);
	if (adap->fe != NULL) {
		state->tuner = DVB_PLL_THOMSON_DTT7579;
		return 0;
	}

	
	adap->fe = dvb_attach(zl10353_attach, &anysee_zl10353_config,
			      &adap->dev->i2c_adap);
	if (adap->fe != NULL) {
		state->tuner = DVB_PLL_THOMSON_DTT7579;
		return 0;
	}

	
	if (dvb_usb_anysee_delsys) {
		ret = anysee_write_reg(adap->dev, 0xb0, 0x01);
		if (ret)
			return ret;

		
		adap->fe = dvb_attach(zl10353_attach, &anysee_zl10353_config,
				      &adap->dev->i2c_adap);
		if (adap->fe != NULL) {
			state->tuner = DVB_PLL_SAMSUNG_DTOS403IH102A;
			return 0;
		}
	}

	
	ret = anysee_write_reg(adap->dev, 0xb0, 0x25);
	if (ret)
		return ret;

	
	adap->fe = dvb_attach(zl10353_attach, &anysee_zl10353_config,
			      &adap->dev->i2c_adap);
	if (adap->fe != NULL) {
		state->tuner = DVB_PLL_THOMSON_DTT7579;
		return 0;
	}

	
	ret = anysee_write_reg(adap->dev, 0xb1, 0xa7);
	if (ret)
		return ret;

	
	adap->fe = dvb_attach(tda10023_attach, &anysee_tda10023_config,
			      &adap->dev->i2c_adap, 0x48);
	if (adap->fe != NULL) {
		state->tuner = DVB_PLL_SAMSUNG_DTOS403IH102A;
		return 0;
	}

	
	ret = anysee_write_reg(adap->dev, 0xb0, io_d);
	if (ret)
		return ret;

	err("Unkown Anysee version: %02x %02x %02x. "\
	    "Please report the <linux-dvb@linuxtv.org>.",
	    hw_info[0], hw_info[1], hw_info[2]);

	return -ENODEV;
}

static int anysee_tuner_attach(struct dvb_usb_adapter *adap)
{
	struct anysee_state *state = adap->dev->priv;
	deb_info("%s: \n", __func__);

	switch (state->tuner) {
	case DVB_PLL_THOMSON_DTT7579:
		
		dvb_attach(dvb_pll_attach, adap->fe, 0x61,
			   NULL, DVB_PLL_THOMSON_DTT7579);
		break;
	case DVB_PLL_SAMSUNG_DTOS403IH102A:
		
		dvb_attach(dvb_pll_attach, adap->fe, 0xc0,
			   &adap->dev->i2c_adap, DVB_PLL_SAMSUNG_DTOS403IH102A);
		break;
	}

	return 0;
}

static int anysee_rc_query(struct dvb_usb_device *d, u32 *event, int *state)
{
	u8 buf[] = {CMD_GET_IR_CODE};
	struct dvb_usb_rc_key *keymap = d->props.rc_key_map;
	u8 ircode[2];
	int i, ret;

	ret = anysee_ctrl_msg(d, buf, sizeof(buf), &ircode[0], 2);
	if (ret)
		return ret;

	*event = 0;
	*state = REMOTE_NO_KEY_PRESSED;

	for (i = 0; i < d->props.rc_key_map_size; i++) {
		if (rc5_custom(&keymap[i]) == ircode[0] &&
		    rc5_data(&keymap[i]) == ircode[1]) {
			*event = keymap[i].event;
			*state = REMOTE_KEY_PRESSED;
			return 0;
		}
	}
	return 0;
}

static struct dvb_usb_rc_key anysee_rc_keys[] = {
	{ 0x0100, KEY_0 },
	{ 0x0101, KEY_1 },
	{ 0x0102, KEY_2 },
	{ 0x0103, KEY_3 },
	{ 0x0104, KEY_4 },
	{ 0x0105, KEY_5 },
	{ 0x0106, KEY_6 },
	{ 0x0107, KEY_7 },
	{ 0x0108, KEY_8 },
	{ 0x0109, KEY_9 },
	{ 0x010a, KEY_POWER },
	{ 0x010b, KEY_DOCUMENTS },    
	{ 0x0119, KEY_FAVORITES },
	{ 0x0120, KEY_SLEEP },
	{ 0x0121, KEY_MODE },         
	{ 0x0122, KEY_ZOOM },
	{ 0x0147, KEY_TEXT },
	{ 0x0116, KEY_TV },           
	{ 0x011e, KEY_LANGUAGE },     
	{ 0x011a, KEY_SUBTITLE },
	{ 0x011b, KEY_CAMERA },       
	{ 0x0142, KEY_MUTE },
	{ 0x010e, KEY_MENU },
	{ 0x010f, KEY_EPG },
	{ 0x0117, KEY_INFO },
	{ 0x0110, KEY_EXIT },
	{ 0x0113, KEY_VOLUMEUP },
	{ 0x0112, KEY_VOLUMEDOWN },
	{ 0x0111, KEY_CHANNELUP },
	{ 0x0114, KEY_CHANNELDOWN },
	{ 0x0115, KEY_OK },
	{ 0x011d, KEY_RED },
	{ 0x011f, KEY_GREEN },
	{ 0x011c, KEY_YELLOW },
	{ 0x0144, KEY_BLUE },
	{ 0x010c, KEY_SHUFFLE },      
	{ 0x0148, KEY_STOP },
	{ 0x0150, KEY_PLAY },
	{ 0x0151, KEY_PAUSE },
	{ 0x0149, KEY_RECORD },
	{ 0x0118, KEY_PREVIOUS },     
	{ 0x010d, KEY_NEXT },         
	{ 0x0124, KEY_PROG1 },        
	{ 0x0125, KEY_PROG2 },        
};


static struct dvb_usb_device_properties anysee_properties;

static int anysee_probe(struct usb_interface *intf,
			const struct usb_device_id *id)
{
	struct dvb_usb_device *d;
	struct usb_host_interface *alt;
	int ret;

	
	if (intf->num_altsetting < 1)
		return -ENODEV;

	ret = dvb_usb_device_init(intf, &anysee_properties, THIS_MODULE, &d,
		adapter_nr);
	if (ret)
		return ret;

	alt = usb_altnum_to_altsetting(intf, 0);
	if (alt == NULL) {
		deb_info("%s: no alt found!\n", __func__);
		return -ENODEV;
	}

	ret = usb_set_interface(d->udev, alt->desc.bInterfaceNumber,
		alt->desc.bAlternateSetting);
	if (ret)
		return ret;

	if (d)
		ret = anysee_init(d);

	return ret;
}

static struct usb_device_id anysee_table[] = {
	{ USB_DEVICE(USB_VID_CYPRESS, USB_PID_ANYSEE) },
	{ USB_DEVICE(USB_VID_AMT,     USB_PID_ANYSEE) },
	{ }		
};
MODULE_DEVICE_TABLE(usb, anysee_table);

static struct dvb_usb_device_properties anysee_properties = {
	.caps             = DVB_USB_IS_AN_I2C_ADAPTER,

	.usb_ctrl         = DEVICE_SPECIFIC,

	.size_of_priv     = sizeof(struct anysee_state),

	.num_adapters = 1,
	.adapter = {
		{
			.streaming_ctrl   = anysee_streaming_ctrl,
			.frontend_attach  = anysee_frontend_attach,
			.tuner_attach     = anysee_tuner_attach,
			.stream = {
				.type = USB_BULK,
				.count = 8,
				.endpoint = 0x82,
				.u = {
					.bulk = {
						.buffersize = (16*512),
					}
				}
			},
		}
	},

	.rc_key_map       = anysee_rc_keys,
	.rc_key_map_size  = ARRAY_SIZE(anysee_rc_keys),
	.rc_query         = anysee_rc_query,
	.rc_interval      = 200,  

	.i2c_algo         = &anysee_i2c_algo,

	.generic_bulk_ctrl_endpoint = 1,

	.num_device_descs = 1,
	.devices = {
		{
			.name = "Anysee DVB USB2.0",
			.cold_ids = {NULL},
			.warm_ids = {&anysee_table[0],
				     &anysee_table[1], NULL},
		},
	}
};

static struct usb_driver anysee_driver = {
	.name       = "dvb_usb_anysee",
	.probe      = anysee_probe,
	.disconnect = dvb_usb_device_exit,
	.id_table   = anysee_table,
};


static int __init anysee_module_init(void)
{
	int ret;

	ret = usb_register(&anysee_driver);
	if (ret)
		err("%s: usb_register failed. Error number %d", __func__, ret);

	return ret;
}

static void __exit anysee_module_exit(void)
{
	
	usb_deregister(&anysee_driver);
}

module_init(anysee_module_init);
module_exit(anysee_module_exit);

MODULE_AUTHOR("Antti Palosaari <crope@iki.fi>");
MODULE_DESCRIPTION("Driver Anysee E30 DVB-C & DVB-T USB2.0");
MODULE_LICENSE("GPL");
