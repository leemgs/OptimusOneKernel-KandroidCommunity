
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/slab.h>

#include "dvb_frontend.h"
#include "lnbp21.h"
#include "lnbh24.h"

struct lnbp21 {
	u8			config;
	u8			override_or;
	u8			override_and;
	struct i2c_adapter	*i2c;
	u8			i2c_addr;
};

static int lnbp21_set_voltage(struct dvb_frontend *fe,
					fe_sec_voltage_t voltage)
{
	struct lnbp21 *lnbp21 = (struct lnbp21 *) fe->sec_priv;
	struct i2c_msg msg = {	.addr = lnbp21->i2c_addr, .flags = 0,
				.buf = &lnbp21->config,
				.len = sizeof(lnbp21->config) };

	lnbp21->config &= ~(LNBP21_VSEL | LNBP21_EN);

	switch(voltage) {
	case SEC_VOLTAGE_OFF:
		break;
	case SEC_VOLTAGE_13:
		lnbp21->config |= LNBP21_EN;
		break;
	case SEC_VOLTAGE_18:
		lnbp21->config |= (LNBP21_EN | LNBP21_VSEL);
		break;
	default:
		return -EINVAL;
	};

	lnbp21->config |= lnbp21->override_or;
	lnbp21->config &= lnbp21->override_and;

	return (i2c_transfer(lnbp21->i2c, &msg, 1) == 1) ? 0 : -EIO;
}

static int lnbp21_enable_high_lnb_voltage(struct dvb_frontend *fe, long arg)
{
	struct lnbp21 *lnbp21 = (struct lnbp21 *) fe->sec_priv;
	struct i2c_msg msg = {	.addr = lnbp21->i2c_addr, .flags = 0,
				.buf = &lnbp21->config,
				.len = sizeof(lnbp21->config) };

	if (arg)
		lnbp21->config |= LNBP21_LLC;
	else
		lnbp21->config &= ~LNBP21_LLC;

	lnbp21->config |= lnbp21->override_or;
	lnbp21->config &= lnbp21->override_and;

	return (i2c_transfer(lnbp21->i2c, &msg, 1) == 1) ? 0 : -EIO;
}

static void lnbp21_release(struct dvb_frontend *fe)
{
	
	lnbp21_set_voltage(fe, SEC_VOLTAGE_OFF);

	
	kfree(fe->sec_priv);
	fe->sec_priv = NULL;
}

static struct dvb_frontend *lnbx2x_attach(struct dvb_frontend *fe,
				struct i2c_adapter *i2c, u8 override_set,
				u8 override_clear, u8 i2c_addr, u8 config)
{
	struct lnbp21 *lnbp21 = kmalloc(sizeof(struct lnbp21), GFP_KERNEL);
	if (!lnbp21)
		return NULL;

	
	lnbp21->config = config;
	lnbp21->i2c = i2c;
	lnbp21->i2c_addr = i2c_addr;
	fe->sec_priv = lnbp21;

	
	lnbp21->override_or = override_set;

	
	lnbp21->override_and = ~override_clear;

	
	if (lnbp21_set_voltage(fe, SEC_VOLTAGE_OFF)) {
		kfree(lnbp21);
		return NULL;
	}

	
	fe->ops.release_sec = lnbp21_release;

	
	fe->ops.set_voltage = lnbp21_set_voltage;
	fe->ops.enable_high_lnb_voltage = lnbp21_enable_high_lnb_voltage;
	printk(KERN_INFO "LNBx2x attached on addr=%x\n", lnbp21->i2c_addr);

	return fe;
}

struct dvb_frontend *lnbh24_attach(struct dvb_frontend *fe,
				struct i2c_adapter *i2c, u8 override_set,
				u8 override_clear, u8 i2c_addr)
{
	return lnbx2x_attach(fe, i2c, override_set, override_clear,
							i2c_addr, LNBH24_TTX);
}
EXPORT_SYMBOL(lnbh24_attach);

struct dvb_frontend *lnbp21_attach(struct dvb_frontend *fe,
				struct i2c_adapter *i2c, u8 override_set,
				u8 override_clear)
{
	return lnbx2x_attach(fe, i2c, override_set, override_clear,
							0x08, LNBP21_ISEL);
}
EXPORT_SYMBOL(lnbp21_attach);

MODULE_DESCRIPTION("Driver for lnb supply and control ic lnbp21, lnbh24");
MODULE_AUTHOR("Oliver Endriss, Igor M. Liplianin");
MODULE_LICENSE("GPL");
