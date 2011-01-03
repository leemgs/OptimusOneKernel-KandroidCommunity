

#ifndef VA1J5JF8007T_H
#define VA1J5JF8007T_H

struct va1j5jf8007t_config {
	u8 demod_address;
};

struct i2c_adapter;

struct dvb_frontend *
va1j5jf8007t_attach(const struct va1j5jf8007t_config *config,
		    struct i2c_adapter *adap);


int va1j5jf8007t_prepare(struct dvb_frontend *fe);

#endif
