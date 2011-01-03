

#ifndef STV06XX_VV6410_H_
#define STV06XX_VV6410_H_

#include "stv06xx_sensor.h"

#define VV6410_COLS			416
#define VV6410_ROWS			320



#define VV6410_DEVICEH			0x00
#define VV6410_DEVICEL			0x01


#define VV6410_STATUS0			0x02


#define VV6410_LINECOUNTH		0x03
#define VV6410_LINECOUNTL		0x04


#define VV6410_XENDH			0x05
#define VV6410_XENDL			0x06


#define VV6410_YENDH			0x07
#define VV6410_YENDL			0x08


#define VV6410_DARKAVGH			0x09
#define VV6410_DARKAVGL			0x0a


#define VV6410_BLACKAVGH		0x0b
#define VV6410_BLACKAVGL		0x0c


#define VV6410_STATUS1			0x0d




#define VV6410_SETUP0			0x10


#define VV6410_SETUP1			0x11


#define VV6410_SYNCVALUE		0x12


#define VV6410_FGMODES			0x14


#define VV6410_PINMAPPING		0x15


#define VV6410_DATAFORMAT		0x16


#define VV6410_OPFORMAT			0x17


#define VV6410_MODESELECT		0x18



#define VV6410_FINEH			0x20
#define VV6410_FINEL			0x21


#define VV6410_COARSEH			0x22
#define VV6410_COARSEL			0x23


#define VV6410_ANALOGGAIN		0x24


#define VV6410_CLKDIV			0x25


#define VV6410_DARKOFFSETH		0x2c
#define VV6410_DARKOFFSETL		0x2d


#define VV6410_DARKOFFSETSETUP		0x2e



#define VV6410_LINELENGTHH		0x52
#define VV6410_LINELENGTHL		0x53


#define VV6410_XOFFSETH			0x57
#define VV6410_XOFFSETL			0x58


#define VV6410_YOFFSETH			0x59
#define VV6410_YOFFSETL			0x5a


#define VV6410_FIELDLENGTHH		0x61
#define VV6410_FIELDLENGTHL		0x62



#define VV6410_BLACKOFFSETH		0x70
#define VV6410_BLACKOFFSETL		0x71


#define VV6410_BLACKOFFSETSETUP		0x72


#define VV6410_CR0			0x75


#define VV6410_CR1			0x76


#define VV6410_AS0			0x77


#define VV6410_AT0			0x78


#define VV6410_AT1			0x79

#define VV6410_HFLIP 			(1 << 3)
#define VV6410_VFLIP 			(1 << 4)

#define VV6410_LOW_POWER_MODE		(1 << 0)
#define VV6410_SOFT_RESET		(1 << 2)
#define VV6410_PAL_25_FPS		(0 << 3)

#define VV6410_CLK_DIV_2		(1 << 1)

#define VV6410_FINE_EXPOSURE		320
#define VV6410_COARSE_EXPOSURE		192
#define VV6410_DEFAULT_GAIN		5

#define VV6410_SUBSAMPLE		0x01
#define VV6410_CROP_TO_QVGA		0x02

#define VV6410_CIF_LINELENGTH		415

static int vv6410_probe(struct sd *sd);
static int vv6410_start(struct sd *sd);
static int vv6410_init(struct sd *sd);
static int vv6410_stop(struct sd *sd);
static int vv6410_dump(struct sd *sd);
static void vv6410_disconnect(struct sd *sd);


static int vv6410_get_hflip(struct gspca_dev *gspca_dev, __s32 *val);
static int vv6410_set_hflip(struct gspca_dev *gspca_dev, __s32 val);
static int vv6410_get_vflip(struct gspca_dev *gspca_dev, __s32 *val);
static int vv6410_set_vflip(struct gspca_dev *gspca_dev, __s32 val);
static int vv6410_get_analog_gain(struct gspca_dev *gspca_dev, __s32 *val);
static int vv6410_set_analog_gain(struct gspca_dev *gspca_dev, __s32 val);
static int vv6410_get_exposure(struct gspca_dev *gspca_dev, __s32 *val);
static int vv6410_set_exposure(struct gspca_dev *gspca_dev, __s32 val);

const struct stv06xx_sensor stv06xx_sensor_vv6410 = {
	.name = "ST VV6410",
	.i2c_flush = 5,
	.i2c_addr = 0x20,
	.i2c_len = 1,
	.init = vv6410_init,
	.probe = vv6410_probe,
	.start = vv6410_start,
	.stop = vv6410_stop,
	.dump = vv6410_dump,
	.disconnect = vv6410_disconnect,
};


struct stv_init {
	const u8 *data;
	u16 start;
	u8 len;
};

static const u8 x1500[] = {	
	0x0b, 0xa7, 0xb7, 0x00, 0x00
};

static const u8 x1536[] = {	
	0x02, 0x00, 0x60, 0x01, 0x20, 0x01
};

static const u8 x15c1[] = {	
	0xff, 0x03 
};

static const struct stv_init stv_bridge_init[] = {
	
	{NULL,  0x1620, 0x80},
	{NULL,  0x1620, 0x00},
	{NULL,  0x1423, 0x04},
	{x1500, 0x1500, ARRAY_SIZE(x1500)},
	{x1536, 0x1536, ARRAY_SIZE(x1536)},
	{x15c1, 0x15c1, ARRAY_SIZE(x15c1)}
};

static const u8 vv6410_sensor_init[][2] = {
	
	{VV6410_SETUP0,		VV6410_SOFT_RESET},
	{VV6410_SETUP0,		VV6410_LOW_POWER_MODE},
	
	{VV6410_SETUP1,		BIT(6)},
	
	{VV6410_FGMODES,	BIT(6) | BIT(4) | BIT(2) | BIT(0)},
	{VV6410_PINMAPPING,	0x00},
	
	{VV6410_DATAFORMAT,	BIT(7) | BIT(0)},

	{VV6410_CLKDIV,		VV6410_CLK_DIV_2},

	
	
	{VV6410_AS0,		BIT(6) | BIT(4) | BIT(3) | BIT(2) | BIT(1)},
	{VV6410_AT0,		0x00},
	
	{VV6410_AT1,		BIT(4)|BIT(0)},
};

#endif
