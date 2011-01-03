

#define DEBUG

#include <linux/slab.h>
#include "ovcamchip_priv.h"


#define REG_GAIN		0x00	
#define REG_BLUE		0x01	
#define REG_RED			0x02	
#define REG_SAT			0x03	
#define REG_CNT			0x05	
#define REG_BRT			0x06	
#define REG_WB_BLUE		0x0C	
#define REG_WB_RED		0x0D	
#define REG_EXP			0x10	


#define HWSBASE 0x38
#define HWEBASE 0x3A
#define VWSBASE 0x05
#define VWEBASE 0x06

struct ov6x20 {
	int auto_brt;
	int auto_exp;
	int backlight;
	int bandfilt;
	int mirror;
};


static struct ovcamchip_regvals regvals_init_6x20_511[] = {
	{ 0x12, 0x80 }, 
	{ 0x11, 0x01 },
	{ 0x03, 0x60 },
	{ 0x05, 0x7f }, 
	{ 0x07, 0xa8 },
	{ 0x0c, 0x24 },
	{ 0x0d, 0x24 },
	{ 0x0f, 0x15 }, 
	{ 0x10, 0x75 }, 
	{ 0x12, 0x24 }, 
	{ 0x14, 0x04 },
	{ 0x16, 0x03 },
	{ 0x26, 0xb2 }, 
	
	{ 0x28, 0x05 },
	{ 0x2a, 0x04 }, 
	{ 0x2d, 0x99 },
	{ 0x33, 0xa0 }, 
	{ 0x34, 0xd2 }, 
	{ 0x38, 0x8b },
	{ 0x39, 0x40 },

	{ 0x3c, 0x39 }, 
	{ 0x3c, 0x3c }, 
	{ 0x3c, 0x24 }, 

	{ 0x3d, 0x80 },
	
	{ 0x4a, 0x80 },
	{ 0x4b, 0x80 },
	{ 0x4d, 0xd2 }, 
	{ 0x4e, 0xc1 },
	{ 0x4f, 0x04 },
	{ 0xff, 0xff },	
};


static struct ovcamchip_regvals regvals_init_6x20_518[] = {
	{ 0x12, 0x80 }, 
	{ 0x03, 0xc0 }, 
	{ 0x05, 0x8a }, 
	{ 0x0c, 0x24 }, 
	{ 0x0d, 0x24 }, 
	{ 0x0e, 0x8d }, 
	{ 0x0f, 0x25 }, 
	{ 0x11, 0x01 }, 
	{ 0x12, 0x24 }, 
	{ 0x13, 0x01 }, 
	{ 0x14, 0x80 }, 
	{ 0x15, 0x01 }, 
	{ 0x16, 0x03 }, 
	{ 0x17, 0x38 }, 
	{ 0x18, 0xea }, 
	{ 0x19, 0x04 },
	{ 0x1a, 0x93 },
	{ 0x1b, 0x00 }, 
	{ 0x1e, 0xc4 }, 
	{ 0x1f, 0x04 }, 
	{ 0x20, 0x20 }, 
	{ 0x21, 0x10 }, 
	{ 0x22, 0x88 }, 
	{ 0x23, 0xc0 }, 
	{ 0x24, 0x53 }, 
	{ 0x25, 0x7a }, 
	{ 0x26, 0xb2 }, 
	{ 0x27, 0xa2 }, 
	{ 0x28, 0x01 }, 
	{ 0x29, 0x00 }, 
	{ 0x2a, 0x84 }, 
	{ 0x2b, 0xa8 }, 
	{ 0x2c, 0xa0 }, 
	{ 0x2d, 0x95 }, 
	{ 0x2e, 0x88 }, 
	{ 0x33, 0x22 }, 
	{ 0x34, 0xc7 }, 
	{ 0x36, 0x12 }, 
	{ 0x37, 0x63 }, 
	{ 0x38, 0x8b }, 
	{ 0x39, 0x00 }, 
	{ 0x3a, 0x0f }, 
	{ 0x3b, 0x3c }, 
	{ 0x3c, 0x5c }, 
	{ 0x3d, 0x80 }, 
	{ 0x3e, 0x80 }, 
	{ 0x3f, 0x02 }, 
	{ 0x40, 0x10 }, 
	{ 0x41, 0x10 }, 
	{ 0x42, 0x00 }, 
	{ 0x43, 0x7f }, 
	{ 0x44, 0x80 }, 
	{ 0x45, 0x1c }, 
	{ 0x46, 0x1c }, 
	{ 0x47, 0x80 }, 
	{ 0x48, 0x5f }, 
	{ 0x49, 0x00 }, 
	{ 0x4a, 0x00 }, 
	{ 0x4b, 0x80 }, 
	{ 0x4c, 0x58 }, 
	{ 0x4d, 0xd2 }, 
	{ 0x4e, 0xa0 }, 
	{ 0x4f, 0x04 }, 
	{ 0x50, 0xff }, 
	{ 0x51, 0x58 }, 
	{ 0x52, 0xc0 }, 
	{ 0x53, 0x42 }, 
	{ 0x27, 0xa6 }, 
	{ 0x12, 0x20 },
	{ 0x12, 0x24 },

	{ 0xff, 0xff },	
};


static int ov6x20_init(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov6x20 *s;
	int rc;

	DDEBUG(4, &c->dev, "entered");

	switch (c->adapter->id) {
	case I2C_HW_SMBUS_OV511:
		rc = ov_write_regvals(c, regvals_init_6x20_511);
		break;
	case I2C_HW_SMBUS_OV518:
		rc = ov_write_regvals(c, regvals_init_6x20_518);
		break;
	default:
		dev_err(&c->dev, "ov6x20: Unsupported adapter\n");
		rc = -ENODEV;
	}

	if (rc < 0)
		return rc;

	ov->spriv = s = kzalloc(sizeof *s, GFP_KERNEL);
	if (!s)
		return -ENOMEM;

	s->auto_brt = 1;
	s->auto_exp = 1;

	return rc;
}

static int ov6x20_free(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);

	kfree(ov->spriv);
	return 0;
}

static int ov6x20_set_control(struct i2c_client *c,
			      struct ovcamchip_control *ctl)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov6x20 *s = ov->spriv;
	int rc;
	int v = ctl->value;

	switch (ctl->id) {
	case OVCAMCHIP_CID_CONT:
		rc = ov_write(c, REG_CNT, v >> 8);
		break;
	case OVCAMCHIP_CID_BRIGHT:
		rc = ov_write(c, REG_BRT, v >> 8);
		break;
	case OVCAMCHIP_CID_SAT:
		rc = ov_write(c, REG_SAT, v >> 8);
		break;
	case OVCAMCHIP_CID_HUE:
		rc = ov_write(c, REG_RED, 0xFF - (v >> 8));
		if (rc < 0)
			goto out;

		rc = ov_write(c, REG_BLUE, v >> 8);
		break;
	case OVCAMCHIP_CID_EXP:
		rc = ov_write(c, REG_EXP, v);
		break;
	case OVCAMCHIP_CID_FREQ:
	{
		int sixty = (v == 60);

		rc = ov_write(c, 0x2b, sixty?0xa8:0x28);
		if (rc < 0)
			goto out;

		rc = ov_write(c, 0x2a, sixty?0x84:0xa4);
		break;
	}
	case OVCAMCHIP_CID_BANDFILT:
		rc = ov_write_mask(c, 0x2d, v?0x04:0x00, 0x04);
		s->bandfilt = v;
		break;
	case OVCAMCHIP_CID_AUTOBRIGHT:
		rc = ov_write_mask(c, 0x2d, v?0x10:0x00, 0x10);
		s->auto_brt = v;
		break;
	case OVCAMCHIP_CID_AUTOEXP:
		rc = ov_write_mask(c, 0x13, v?0x01:0x00, 0x01);
		s->auto_exp = v;
		break;
	case OVCAMCHIP_CID_BACKLIGHT:
	{
		rc = ov_write_mask(c, 0x4e, v?0xe0:0xc0, 0xe0);
		if (rc < 0)
			goto out;

		rc = ov_write_mask(c, 0x29, v?0x08:0x00, 0x08);
		if (rc < 0)
			goto out;

		rc = ov_write_mask(c, 0x0e, v?0x80:0x00, 0x80);
		s->backlight = v;
		break;
	}
	case OVCAMCHIP_CID_MIRROR:
		rc = ov_write_mask(c, 0x12, v?0x40:0x00, 0x40);
		s->mirror = v;
		break;
	default:
		DDEBUG(2, &c->dev, "control not supported: %d", ctl->id);
		return -EPERM;
	}

out:
	DDEBUG(3, &c->dev, "id=%d, arg=%d, rc=%d", ctl->id, v, rc);
	return rc;
}

static int ov6x20_get_control(struct i2c_client *c,
			      struct ovcamchip_control *ctl)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov6x20 *s = ov->spriv;
	int rc = 0;
	unsigned char val = 0;

	switch (ctl->id) {
	case OVCAMCHIP_CID_CONT:
		rc = ov_read(c, REG_CNT, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_BRIGHT:
		rc = ov_read(c, REG_BRT, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_SAT:
		rc = ov_read(c, REG_SAT, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_HUE:
		rc = ov_read(c, REG_BLUE, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_EXP:
		rc = ov_read(c, REG_EXP, &val);
		ctl->value = val;
		break;
	case OVCAMCHIP_CID_BANDFILT:
		ctl->value = s->bandfilt;
		break;
	case OVCAMCHIP_CID_AUTOBRIGHT:
		ctl->value = s->auto_brt;
		break;
	case OVCAMCHIP_CID_AUTOEXP:
		ctl->value = s->auto_exp;
		break;
	case OVCAMCHIP_CID_BACKLIGHT:
		ctl->value = s->backlight;
		break;
	case OVCAMCHIP_CID_MIRROR:
		ctl->value = s->mirror;
		break;
	default:
		DDEBUG(2, &c->dev, "control not supported: %d", ctl->id);
		return -EPERM;
	}

	DDEBUG(3, &c->dev, "id=%d, arg=%d, rc=%d", ctl->id, ctl->value, rc);
	return rc;
}

static int ov6x20_mode_init(struct i2c_client *c, struct ovcamchip_window *win)
{
	

	ov_write(c, 0x14, win->quarter?0x24:0x04);

	

	
	if (c->adapter->id == I2C_HW_SMBUS_OV518) {
		if (win->format == VIDEO_PALETTE_GREY)
			ov_write_mask(c, 0x13, 0x00, 0x20);
		else
			ov_write_mask(c, 0x13, 0x20, 0x20);
	} else {
		if (win->format == VIDEO_PALETTE_GREY)
			ov_write_mask(c, 0x13, 0x20, 0x20);
		else
			ov_write_mask(c, 0x13, 0x00, 0x20);
	}

	

	

	
	ov_write(c, 0x2a, 0x04);

	ov_write(c, 0x11, win->clockdiv);

	ov_write(c, 0x2a, 0x84);
	
	ov_write(c, 0x2d, 0x85); 

	return 0;
}

static int ov6x20_set_window(struct i2c_client *c, struct ovcamchip_window *win)
{
	int ret, hwscale, vwscale;

	ret = ov6x20_mode_init(c, win);
	if (ret < 0)
		return ret;

	if (win->quarter) {
		hwscale = 0;
		vwscale = 0;
	} else {
		hwscale = 1;
		vwscale = 1;	
	}

	ov_write(c, 0x17, HWSBASE + (win->x >> hwscale));
	ov_write(c, 0x18, HWEBASE + ((win->x + win->width) >> hwscale));
	ov_write(c, 0x19, VWSBASE + (win->y >> vwscale));
	ov_write(c, 0x1a, VWEBASE + ((win->y + win->height) >> vwscale));

	return 0;
}

static int ov6x20_command(struct i2c_client *c, unsigned int cmd, void *arg)
{
	switch (cmd) {
	case OVCAMCHIP_CMD_S_CTRL:
		return ov6x20_set_control(c, arg);
	case OVCAMCHIP_CMD_G_CTRL:
		return ov6x20_get_control(c, arg);
	case OVCAMCHIP_CMD_S_MODE:
		return ov6x20_set_window(c, arg);
	default:
		DDEBUG(2, &c->dev, "command not supported: %d", cmd);
		return -ENOIOCTLCMD;
	}
}

struct ovcamchip_ops ov6x20_ops = {
	.init    =	ov6x20_init,
	.free    =	ov6x20_free,
	.command =	ov6x20_command,
};
