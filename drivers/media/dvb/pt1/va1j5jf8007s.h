

#ifndef VA1J5JF8007S_H
#define VA1J5JF8007S_H

struct va1j5jf8007s_config {
	u8 demod_address;
};

struct i2c_adapter;

struct dvb_frontend *
va1j5jf8007s_attach(const struct va1j5jf8007s_config *config,
		    struct i2c_adapter *adap);


int va1j5jf8007s_prepare(struct dvb_frontend *fe);

#endif
