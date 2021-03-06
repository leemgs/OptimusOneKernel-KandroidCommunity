

#include "stv06xx_st6422.h"

static struct v4l2_pix_format st6422_mode[] = {
	
	{
		162,
		120,
		V4L2_PIX_FMT_SGRBG8,
		V4L2_FIELD_NONE,
		.sizeimage = 162 * 120,
		.bytesperline = 162,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 1
	},
	
	{
		324,
		240,
		V4L2_PIX_FMT_SGRBG8,
		V4L2_FIELD_NONE,
		.sizeimage = 324 * 244,
		.bytesperline = 324,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 0
	},
};

static const struct ctrl st6422_ctrl[] = {
#define BRIGHTNESS_IDX 0
	{
		{
			.id		= V4L2_CID_BRIGHTNESS,
			.type		= V4L2_CTRL_TYPE_INTEGER,
			.name		= "Brightness",
			.minimum	= 0,
			.maximum	= 31,
			.step		= 1,
			.default_value  = 3
		},
		.set = st6422_set_brightness,
		.get = st6422_get_brightness
	},
#define CONTRAST_IDX 1
	{
		{
			.id		= V4L2_CID_CONTRAST,
			.type		= V4L2_CTRL_TYPE_INTEGER,
			.name		= "Contrast",
			.minimum	= 0,
			.maximum	= 15,
			.step		= 1,
			.default_value  = 11
		},
		.set = st6422_set_contrast,
		.get = st6422_get_contrast
	},
#define GAIN_IDX 2
	{
		{
			.id		= V4L2_CID_GAIN,
			.type		= V4L2_CTRL_TYPE_INTEGER,
			.name		= "Gain",
			.minimum	= 0,
			.maximum	= 255,
			.step		= 1,
			.default_value  = 64
		},
		.set = st6422_set_gain,
		.get = st6422_get_gain
	},
#define EXPOSURE_IDX 3
	{
		{
			.id		= V4L2_CID_EXPOSURE,
			.type		= V4L2_CTRL_TYPE_INTEGER,
			.name		= "Exposure",
			.minimum	= 0,
			.maximum	= 1023,
			.step		= 1,
			.default_value  = 256
		},
		.set = st6422_set_exposure,
		.get = st6422_get_exposure
	},
};

static int st6422_probe(struct sd *sd)
{
	int i;
	s32 *sensor_settings;

	if (sd->bridge != BRIDGE_ST6422)
		return -ENODEV;

	info("st6422 sensor detected");

	sensor_settings = kmalloc(ARRAY_SIZE(st6422_ctrl) * sizeof(s32),
				  GFP_KERNEL);
	if (!sensor_settings)
		return -ENOMEM;

	sd->gspca_dev.cam.cam_mode = st6422_mode;
	sd->gspca_dev.cam.nmodes = ARRAY_SIZE(st6422_mode);
	sd->desc.ctrls = st6422_ctrl;
	sd->desc.nctrls = ARRAY_SIZE(st6422_ctrl);
	sd->sensor_priv = sensor_settings;

	for (i = 0; i < sd->desc.nctrls; i++)
		sensor_settings[i] = st6422_ctrl[i].qctrl.default_value;

	return 0;
}

static int st6422_init(struct sd *sd)
{
	int err = 0, i;

	const u16 st6422_bridge_init[][2] = {
		{ STV_ISO_ENABLE, 0x00 }, 
		{ 0x1436, 0x00 },
		{ 0x1432, 0x03 },	
		{ 0x143a, 0xF9 },	
		{ 0x0509, 0x38 },	
		{ 0x050a, 0x38 },	
		{ 0x050b, 0x38 },	
		{ 0x050c, 0x2A },
		{ 0x050d, 0x01 },


		{ 0x1431, 0x00 },	
		{ 0x1433, 0x34 },	
		{ 0x1438, 0x18 },	



		{ 0x1439, 0x00 },


		{ 0x143b, 0x05 },
		{ 0x143c, 0x00 },	





		{ 0x143e, 0x01 },
		{ 0x143d, 0x00 },

		{ 0x1442, 0xe2 },













		{ 0x1500, 0xd0 },
		{ 0x1500, 0xd0 },
		{ 0x1500, 0x50 },	

		{ 0x1501, 0xaf },


		{ 0x1502, 0xc2 },


		{ 0x1503, 0x45 },


		{ 0x1505, 0x02 },




		{ 0x150e, 0x8e },
		{ 0x150f, 0x37 },
		{ 0x15c0, 0x00 },
		{ 0x15c1, 1023 }, 
		{ 0x15c3, 0x08 },	


		{ 0x143f, 0x01 },	

	};

	for (i = 0; i < ARRAY_SIZE(st6422_bridge_init) && !err; i++) {
		err = stv06xx_write_bridge(sd, st6422_bridge_init[i][0],
					       st6422_bridge_init[i][1]);
	}

	return err;
}

static void st6422_disconnect(struct sd *sd)
{
	sd->sensor = NULL;
	kfree(sd->sensor_priv);
}

static int st6422_start(struct sd *sd)
{
	int err, packet_size;
	struct cam *cam = &sd->gspca_dev.cam;
	s32 *sensor_settings = sd->sensor_priv;
	struct usb_host_interface *alt;
	struct usb_interface *intf;

	intf = usb_ifnum_to_if(sd->gspca_dev.dev, sd->gspca_dev.iface);
	alt = usb_altnum_to_altsetting(intf, sd->gspca_dev.alt);
	if (!alt) {
		PDEBUG(D_ERR, "Couldn't get altsetting");
		return -EIO;
	}

	packet_size = le16_to_cpu(alt->endpoint[0].desc.wMaxPacketSize);
	err = stv06xx_write_bridge(sd, 0x15c1, packet_size);
	if (err < 0)
		return err;

	if (cam->cam_mode[sd->gspca_dev.curr_mode].priv)
		err = stv06xx_write_bridge(sd, 0x1505, 0x0f);
	else
		err = stv06xx_write_bridge(sd, 0x1505, 0x02);
	if (err < 0)
		return err;

	err = st6422_set_brightness(&sd->gspca_dev,
				    sensor_settings[BRIGHTNESS_IDX]);
	if (err < 0)
		return err;

	err = st6422_set_contrast(&sd->gspca_dev,
				  sensor_settings[CONTRAST_IDX]);
	if (err < 0)
		return err;

	err = st6422_set_exposure(&sd->gspca_dev,
				  sensor_settings[EXPOSURE_IDX]);
	if (err < 0)
		return err;

	err = st6422_set_gain(&sd->gspca_dev,
			      sensor_settings[GAIN_IDX]);
	if (err < 0)
		return err;

	PDEBUG(D_STREAM, "Starting stream");

	return 0;
}

static int st6422_stop(struct sd *sd)
{
	PDEBUG(D_STREAM, "Halting stream");

	return 0;
}

static int st6422_get_brightness(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;
	s32 *sensor_settings = sd->sensor_priv;

	*val = sensor_settings[BRIGHTNESS_IDX];

	PDEBUG(D_V4L2, "Read brightness %d", *val);

	return 0;
}

static int st6422_set_brightness(struct gspca_dev *gspca_dev, __s32 val)
{
	int err;
	struct sd *sd = (struct sd *) gspca_dev;
	s32 *sensor_settings = sd->sensor_priv;

	sensor_settings[BRIGHTNESS_IDX] = val;

	if (!gspca_dev->streaming)
		return 0;

	
	PDEBUG(D_V4L2, "Set brightness to %d", val);
	err = stv06xx_write_bridge(sd, 0x1432, val);
	if (err < 0)
		return err;

	
	err = stv06xx_write_bridge(sd, 0x143f, 0x01);
	return (err < 0) ? err : 0;
}

static int st6422_get_contrast(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;
	s32 *sensor_settings = sd->sensor_priv;

	*val = sensor_settings[CONTRAST_IDX];

	PDEBUG(D_V4L2, "Read contrast %d", *val);

	return 0;
}

static int st6422_set_contrast(struct gspca_dev *gspca_dev, __s32 val)
{
	int err;
	struct sd *sd = (struct sd *) gspca_dev;
	s32 *sensor_settings = sd->sensor_priv;

	sensor_settings[CONTRAST_IDX] = val;

	if (!gspca_dev->streaming)
		return 0;

	
	PDEBUG(D_V4L2, "Set contrast to %d\n", val);
	err = stv06xx_write_bridge(sd, 0x143a, 0xf0 | val);
	if (err < 0)
		return err;

	
	err = stv06xx_write_bridge(sd, 0x143f, 0x01);
	return (err < 0) ? err : 0;
}

static int st6422_get_gain(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;
	s32 *sensor_settings = sd->sensor_priv;

	*val = sensor_settings[GAIN_IDX];

	PDEBUG(D_V4L2, "Read gain %d", *val);

	return 0;
}

static int st6422_set_gain(struct gspca_dev *gspca_dev, __s32 val)
{
	int err;
	struct sd *sd = (struct sd *) gspca_dev;
	s32 *sensor_settings = sd->sensor_priv;

	sensor_settings[GAIN_IDX] = val;

	if (!gspca_dev->streaming)
		return 0;

	PDEBUG(D_V4L2, "Set gain to %d", val);

	
	err = stv06xx_write_bridge(sd, 0x0509, val);
	if (err < 0)
		return err;

	err = stv06xx_write_bridge(sd, 0x050a, val);
	if (err < 0)
		return err;

	err = stv06xx_write_bridge(sd, 0x050b, val);
	if (err < 0)
		return err;

	
	err = stv06xx_write_bridge(sd, 0x050c, 0x2a);
	if (err < 0)
		return err;

	err = stv06xx_write_bridge(sd, 0x050d, 0x01);
	if (err < 0)
		return err;

	
	err = stv06xx_write_bridge(sd, 0x143f, 0x01);
	return (err < 0) ? err : 0;
}

static int st6422_get_exposure(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;
	s32 *sensor_settings = sd->sensor_priv;

	*val = sensor_settings[EXPOSURE_IDX];

	PDEBUG(D_V4L2, "Read exposure %d", *val);

	return 0;
}

static int st6422_set_exposure(struct gspca_dev *gspca_dev, __s32 val)
{
	int err;
	struct sd *sd = (struct sd *) gspca_dev;
	s32 *sensor_settings = sd->sensor_priv;

	sensor_settings[EXPOSURE_IDX] = val;

	if (!gspca_dev->streaming)
		return 0;

	PDEBUG(D_V4L2, "Set exposure to %d\n", val);
	err = stv06xx_write_bridge(sd, 0x143d, val & 0xff);
	if (err < 0)
		return err;

	err = stv06xx_write_bridge(sd, 0x143e, val >> 8);
	if (err < 0)
		return err;

	
	err = stv06xx_write_bridge(sd, 0x143f, 0x01);
	return (err < 0) ? err : 0;
}
