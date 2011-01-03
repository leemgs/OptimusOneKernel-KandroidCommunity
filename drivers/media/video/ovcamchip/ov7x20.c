

#define DEBUG

#include <linux/slab.h>
#include "ovcamchip_priv.h"


#define REG_GAIN		0x00	
#define REG_BLUE		0x01	
#define REG_RED			0x02	
#define REG_SAT			0x03	
#define REG_BRT			0x06	
#define REG_SHARP		0x07	
#define REG_BLUE_BIAS		0x0C	
#define REG_RED_BIAS		0x0D	
#define REG_EXP			0x10	


#define OV7120_DFL_BRIGHT     0x60
#define OV7620_DFL_BRIGHT     0x60
#define OV7120_DFL_SAT        0xb0
#define OV7620_DFL_SAT        0xc0
#define DFL_AUTO_EXP             1
#define DFL_AUTO_GAIN            1
#define OV7120_DFL_GAIN       0x00
#define OV7620_DFL_GAIN       0x00

#define OV7120_DFL_EXP        0x7f
#define OV7620_DFL_EXP        0x7f


#define HWSBASE 0x2F	
#define HWEBASE 0x2F
#define VWSBASE 0x05
#define VWEBASE 0x05

struct ov7x20 {
	int auto_brt;
	int auto_exp;
	int auto_gain;
	int backlight;
	int bandfilt;
	int mirror;
};


static unsigned char ctab[] = {
	0x01, 0x05, 0x09, 0x11, 0x15, 0x35, 0x37, 0x57,
	0x5b, 0xa5, 0xa7, 0xc7, 0xc9, 0xcf, 0xef, 0xff
};


static struct ovcamchip_regvals regvals_init_7120[] = {
	{ 0x12, 0x80 }, 
	{ 0x13, 0x00 }, 
	{ 0x12, 0x20 }, 
	{ 0x13, DFL_AUTO_GAIN?0x01:0x00 }, 
	{ 0x00, OV7120_DFL_GAIN },
	{ 0x01, 0x80 },
	{ 0x02, 0x80 },
	{ 0x03, OV7120_DFL_SAT },
	{ 0x06, OV7120_DFL_BRIGHT },
	{ 0x07, 0x00 },
	{ 0x0c, 0x20 },
	{ 0x0d, 0x20 },
	{ 0x11, 0x01 },
	{ 0x14, 0x84 },
	{ 0x15, 0x01 },
	{ 0x16, 0x03 },
	{ 0x17, 0x2f },
	{ 0x18, 0xcf },
	{ 0x19, 0x06 },
	{ 0x1a, 0xf5 },
	{ 0x1b, 0x00 },
	{ 0x20, 0x08 },
	{ 0x21, 0x80 },
	{ 0x22, 0x80 },
	{ 0x23, 0x00 },
	{ 0x26, 0xa0 },
	{ 0x27, 0xfa },
	{ 0x28, 0x20 }, 
	{ 0x29, DFL_AUTO_EXP?0x00:0x80 },
	{ 0x2a, 0x10 },
	{ 0x2b, 0x00 },
	{ 0x2c, 0x88 },
	{ 0x2d, 0x95 },
	{ 0x2e, 0x80 },
	{ 0x2f, 0x44 },
	{ 0x60, 0x20 },
	{ 0x61, 0x02 },
	{ 0x62, 0x5f },
	{ 0x63, 0xd5 },
	{ 0x64, 0x57 },
	{ 0x65, 0x83 }, 
	{ 0x66, 0x55 },
	{ 0x67, 0x92 },
	{ 0x68, 0xcf },
	{ 0x69, 0x76 },
	{ 0x6a, 0x22 },
	{ 0x6b, 0xe2 },
	{ 0x6c, 0x40 },
	{ 0x6d, 0x48 },
	{ 0x6e, 0x80 },
	{ 0x6f, 0x0d },
	{ 0x70, 0x89 },
	{ 0x71, 0x00 },
	{ 0x72, 0x14 },
	{ 0x73, 0x54 },
	{ 0x74, 0xa0 },
	{ 0x75, 0x8e },
	{ 0x76, 0x00 },
	{ 0x77, 0xff },
	{ 0x78, 0x80 },
	{ 0x79, 0x80 },
	{ 0x7a, 0x80 },
	{ 0x7b, 0xe6 },
	{ 0x7c, 0x00 },
	{ 0x24, 0x3a },
	{ 0x25, 0x60 },
	{ 0xff, 0xff },	
};


static struct ovcamchip_regvals regvals_init_7620[] = {
	{ 0x12, 0x80 }, 
	{ 0x00, OV7620_DFL_GAIN },
	{ 0x01, 0x80 },
	{ 0x02, 0x80 },
	{ 0x03, OV7620_DFL_SAT },
	{ 0x06, OV7620_DFL_BRIGHT },
	{ 0x07, 0x00 },
	{ 0x0c, 0x24 },
	{ 0x0c, 0x24 },
	{ 0x0d, 0x24 },
	{ 0x11, 0x01 },
	{ 0x12, 0x24 },
	{ 0x13, DFL_AUTO_GAIN?0x01:0x00 },
	{ 0x14, 0x84 },
	{ 0x15, 0x01 },
	{ 0x16, 0x03 },
	{ 0x17, 0x2f },
	{ 0x18, 0xcf },
	{ 0x19, 0x06 },
	{ 0x1a, 0xf5 },
	{ 0x1b, 0x00 },
	{ 0x20, 0x18 },
	{ 0x21, 0x80 },
	{ 0x22, 0x80 },
	{ 0x23, 0x00 },
	{ 0x26, 0xa2 },
	{ 0x27, 0xea },
	{ 0x28, 0x20 },
	{ 0x29, DFL_AUTO_EXP?0x00:0x80 },
	{ 0x2a, 0x10 },
	{ 0x2b, 0x00 },
	{ 0x2c, 0x88 },
	{ 0x2d, 0x91 },
	{ 0x2e, 0x80 },
	{ 0x2f, 0x44 },
	{ 0x60, 0x27 },
	{ 0x61, 0x02 },
	{ 0x62, 0x5f },
	{ 0x63, 0xd5 },
	{ 0x64, 0x57 },
	{ 0x65, 0x83 },
	{ 0x66, 0x55 },
	{ 0x67, 0x92 },
	{ 0x68, 0xcf },
	{ 0x69, 0x76 },
	{ 0x6a, 0x22 },
	{ 0x6b, 0x00 },
	{ 0x6c, 0x02 },
	{ 0x6d, 0x44 },
	{ 0x6e, 0x80 },
	{ 0x6f, 0x1d },
	{ 0x70, 0x8b },
	{ 0x71, 0x00 },
	{ 0x72, 0x14 },
	{ 0x73, 0x54 },
	{ 0x74, 0x00 },
	{ 0x75, 0x8e },
	{ 0x76, 0x00 },
	{ 0x77, 0xff },
	{ 0x78, 0x80 },
	{ 0x79, 0x80 },
	{ 0x7a, 0x80 },
	{ 0x7b, 0xe2 },
	{ 0x7c, 0x00 },
	{ 0xff, 0xff },	
};


static inline int ov7x20_lut_find(unsigned char lut[], int n, unsigned char val)
{
	int i = 0;

	while (lut[i] < val && i < n)
		i++;

	return i;
}


static int ov7x20_init(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov7x20 *s;
	int rc;

	DDEBUG(4, &c->dev, "entered");

	if (ov->mono)
		rc = ov_write_regvals(c, regvals_init_7120);
	else
		rc = ov_write_regvals(c, regvals_init_7620);

	if (rc < 0)
		return rc;

	ov->spriv = s = kzalloc(sizeof *s, GFP_KERNEL);
	if (!s)
		return -ENOMEM;

	s->auto_brt = 1;
	s->auto_exp = DFL_AUTO_EXP;
	s->auto_gain = DFL_AUTO_GAIN;

	return 0;
}

static int ov7x20_free(struct i2c_client *c)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);

	kfree(ov->spriv);
	return 0;
}

static int ov7x20_set_v4l1_control(struct i2c_client *c,
				   struct ovcamchip_control *ctl)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov7x20 *s = ov->spriv;
	int rc;
	int v = ctl->value;

	switch (ctl->id) {
	case OVCAMCHIP_CID_CONT:
	{
		
		rc = ov_write(c, 0x64, ctab[v >> 12]);
		break;
	}
	case OVCAMCHIP_CID_BRIGHT:
		
		if (!s->auto_brt)
			rc = ov_write(c, REG_BRT, v >> 8);
		else
			rc = 0;
		break;
	case OVCAMCHIP_CID_SAT:
		rc = ov_write(c, REG_SAT, v >> 8);
		break;
	case OVCAMCHIP_CID_EXP:
		if (!s->auto_exp)
			rc = ov_write(c, REG_EXP, v);
		else
			rc = -EBUSY;
		break;
	case OVCAMCHIP_CID_FREQ:
	{
		int sixty = (v == 60);

		rc = ov_write_mask(c, 0x2a, sixty?0x00:0x80, 0x80);
		if (rc < 0)
			goto out;

		rc = ov_write(c, 0x2b, sixty?0x00:0xac);
		if (rc < 0)
			goto out;

		rc = ov_write_mask(c, 0x76, 0x01, 0x01);
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
		rc = ov_write_mask(c, 0x68, v?0xe0:0xc0, 0xe0);
		if (rc < 0)
			goto out;

		rc = ov_write_mask(c, 0x29, v?0x08:0x00, 0x08);
		if (rc < 0)
			goto out;

		rc = ov_write_mask(c, 0x28, v?0x02:0x00, 0x02);
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

static int ov7x20_get_v4l1_control(struct i2c_client *c,
				   struct ovcamchip_control *ctl)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	struct ov7x20 *s = ov->spriv;
	int rc = 0;
	unsigned char val = 0;

	switch (ctl->id) {
	case OVCAMCHIP_CID_CONT:
		rc = ov_read(c, 0x64, &val);
		ctl->value = ov7x20_lut_find(ctab, 16, val) << 12;
		break;
	case OVCAMCHIP_CID_BRIGHT:
		rc = ov_read(c, REG_BRT, &val);
		ctl->value = val << 8;
		break;
	case OVCAMCHIP_CID_SAT:
		rc = ov_read(c, REG_SAT, &val);
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

static int ov7x20_mode_init(struct i2c_client *c, struct ovcamchip_window *win)
{
	struct ovcamchip *ov = i2c_get_clientdata(c);
	int qvga = win->quarter;

	
	ov_write_mask(c, 0x14, qvga?0x20:0x00, 0x20);
	ov_write_mask(c, 0x28, qvga?0x00:0x20, 0x20);
	ov_write(c, 0x24, qvga?0x20:0x3a);
	ov_write(c, 0x25, qvga?0x30:0x60);
	ov_write_mask(c, 0x2d, qvga?0x40:0x00, 0x40);
	if (!ov->mono)
		ov_write_mask(c, 0x67, qvga?0xf0:0x90, 0xf0);
	ov_write_mask(c, 0x74, qvga?0x20:0x00, 0x20);

	

	ov_write(c, 0x11, win->clockdiv);

	return 0;
}

static int ov7x20_set_window(struct i2c_client *c, struct ovcamchip_window *win)
{
	int ret, hwscale, vwscale;

	ret = ov7x20_mode_init(c, win);
	if (ret < 0)
		return ret;

	if (win->quarter) {
		hwscale = 1;
		vwscale = 0;
	} else {
		hwscale = 2;
		vwscale = 1;
	}

	ov_write(c, 0x17, HWSBASE + (win->x >> hwscale));
	ov_write(c, 0x18, HWEBASE + ((win->x + win->width) >> hwscale));
	ov_write(c, 0x19, VWSBASE + (win->y >> vwscale));
	ov_write(c, 0x1a, VWEBASE + ((win->y + win->height) >> vwscale));

	return 0;
}

static int ov7x20_command(struct i2c_client *c, unsigned int cmd, void *arg)
{
	switch (cmd) {
	case OVCAMCHIP_CMD_S_CTRL:
		return ov7x20_set_v4l1_control(c, arg);
	case OVCAMCHIP_CMD_G_CTRL:
		return ov7x20_get_v4l1_control(c, arg);
	case OVCAMCHIP_CMD_S_MODE:
		return ov7x20_set_window(c, arg);
	default:
		DDEBUG(2, &c->dev, "command not supported: %d", cmd);
		return -ENOIOCTLCMD;
	}
}

struct ovcamchip_ops ov7x20_ops = {
	.init    =	ov7x20_init,
	.free    =	ov7x20_free,
	.command =	ov7x20_command,
};
