

#define MODULE_NAME "mr97310a"

#include "gspca.h"

#define CAM_TYPE_CIF			0
#define CAM_TYPE_VGA			1

#define MR97310A_BRIGHTNESS_MIN		-254
#define MR97310A_BRIGHTNESS_MAX		255
#define MR97310A_BRIGHTNESS_DEFAULT	0

#define MR97310A_EXPOSURE_MIN		300
#define MR97310A_EXPOSURE_MAX		4095
#define MR97310A_EXPOSURE_DEFAULT	1000

#define MR97310A_GAIN_MIN		0
#define MR97310A_GAIN_MAX		31
#define MR97310A_GAIN_DEFAULT		25

MODULE_AUTHOR("Kyle Guinn <elyk03@gmail.com>,"
	      "Theodore Kilgore <kilgota@auburn.edu>");
MODULE_DESCRIPTION("GSPCA/Mars-Semi MR97310A USB Camera Driver");
MODULE_LICENSE("GPL");


int force_sensor_type = -1;
module_param(force_sensor_type, int, 0644);
MODULE_PARM_DESC(force_sensor_type, "Force sensor type (-1 (auto), 0 or 1)");


struct sd {
	struct gspca_dev gspca_dev;  
	u8 sof_read;
	u8 cam_type;	
	u8 sensor_type;	
	u8 do_lcd_stop;

	int brightness;
	u16 exposure;
	u8 gain;
};

struct sensor_w_data {
	u8 reg;
	u8 flags;
	u8 data[16];
	int len;
};

static int sd_setbrightness(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getbrightness(struct gspca_dev *gspca_dev, __s32 *val);
static int sd_setexposure(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getexposure(struct gspca_dev *gspca_dev, __s32 *val);
static int sd_setgain(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getgain(struct gspca_dev *gspca_dev, __s32 *val);
static void setbrightness(struct gspca_dev *gspca_dev);
static void setexposure(struct gspca_dev *gspca_dev);
static void setgain(struct gspca_dev *gspca_dev);


static struct ctrl sd_ctrls[] = {
	{
#define BRIGHTNESS_IDX 0
		{
			.id = V4L2_CID_BRIGHTNESS,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "Brightness",
			.minimum = MR97310A_BRIGHTNESS_MIN,
			.maximum = MR97310A_BRIGHTNESS_MAX,
			.step = 1,
			.default_value = MR97310A_BRIGHTNESS_DEFAULT,
			.flags = 0,
		},
		.set = sd_setbrightness,
		.get = sd_getbrightness,
	},
	{
#define EXPOSURE_IDX 1
		{
			.id = V4L2_CID_EXPOSURE,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "Exposure",
			.minimum = MR97310A_EXPOSURE_MIN,
			.maximum = MR97310A_EXPOSURE_MAX,
			.step = 1,
			.default_value = MR97310A_EXPOSURE_DEFAULT,
			.flags = 0,
		},
		.set = sd_setexposure,
		.get = sd_getexposure,
	},
	{
#define GAIN_IDX 2
		{
			.id = V4L2_CID_GAIN,
			.type = V4L2_CTRL_TYPE_INTEGER,
			.name = "Gain",
			.minimum = MR97310A_GAIN_MIN,
			.maximum = MR97310A_GAIN_MAX,
			.step = 1,
			.default_value = MR97310A_GAIN_DEFAULT,
			.flags = 0,
		},
		.set = sd_setgain,
		.get = sd_getgain,
	},
};

static const struct v4l2_pix_format vga_mode[] = {
	{160, 120, V4L2_PIX_FMT_MR97310A, V4L2_FIELD_NONE,
		.bytesperline = 160,
		.sizeimage = 160 * 120,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 4},
	{176, 144, V4L2_PIX_FMT_MR97310A, V4L2_FIELD_NONE,
		.bytesperline = 176,
		.sizeimage = 176 * 144,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 3},
	{320, 240, V4L2_PIX_FMT_MR97310A, V4L2_FIELD_NONE,
		.bytesperline = 320,
		.sizeimage = 320 * 240,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 2},
	{352, 288, V4L2_PIX_FMT_MR97310A, V4L2_FIELD_NONE,
		.bytesperline = 352,
		.sizeimage = 352 * 288,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 1},
	{640, 480, V4L2_PIX_FMT_MR97310A, V4L2_FIELD_NONE,
		.bytesperline = 640,
		.sizeimage = 640 * 480,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 0},
};


static int mr_write(struct gspca_dev *gspca_dev, int len)
{
	int rc;

	rc = usb_bulk_msg(gspca_dev->dev,
			  usb_sndbulkpipe(gspca_dev->dev, 4),
			  gspca_dev->usb_buf, len, NULL, 500);
	if (rc < 0)
		PDEBUG(D_ERR, "reg write [%02x] error %d",
		       gspca_dev->usb_buf[0], rc);
	return rc;
}


static int mr_read(struct gspca_dev *gspca_dev, int len)
{
	int rc;

	rc = usb_bulk_msg(gspca_dev->dev,
			  usb_rcvbulkpipe(gspca_dev->dev, 3),
			  gspca_dev->usb_buf, len, NULL, 500);
	if (rc < 0)
		PDEBUG(D_ERR, "reg read [%02x] error %d",
		       gspca_dev->usb_buf[0], rc);
	return rc;
}

static int sensor_write_reg(struct gspca_dev *gspca_dev, u8 reg, u8 flags,
	const u8 *data, int len)
{
	gspca_dev->usb_buf[0] = 0x1f;
	gspca_dev->usb_buf[1] = flags;
	gspca_dev->usb_buf[2] = reg;
	memcpy(gspca_dev->usb_buf + 3, data, len);

	return mr_write(gspca_dev, len + 3);
}

static int sensor_write_regs(struct gspca_dev *gspca_dev,
	const struct sensor_w_data *data, int len)
{
	int i, rc;

	for (i = 0; i < len; i++) {
		rc = sensor_write_reg(gspca_dev, data[i].reg, data[i].flags,
					  data[i].data, data[i].len);
		if (rc < 0)
			return rc;
	}

	return 0;
}

static int sensor_write1(struct gspca_dev *gspca_dev, u8 reg, u8 data)
{
	struct sd *sd = (struct sd *) gspca_dev;
	u8 buf, confirm_reg;
	int rc;

	buf = data;
	rc = sensor_write_reg(gspca_dev, reg, 0x01, &buf, 1);
	if (rc < 0)
		return rc;

	buf = 0x01;
	confirm_reg = sd->sensor_type ? 0x13 : 0x11;
	rc = sensor_write_reg(gspca_dev, confirm_reg, 0x00, &buf, 1);
	if (rc < 0)
		return rc;

	return 0;
}

static int cam_get_response16(struct gspca_dev *gspca_dev)
{
	__u8 *data = gspca_dev->usb_buf;
	int err_code;

	data[0] = 0x21;
	err_code = mr_write(gspca_dev, 1);
	if (err_code < 0)
		return err_code;

	err_code = mr_read(gspca_dev, 16);
	return err_code;
}

static int zero_the_pointer(struct gspca_dev *gspca_dev)
{
	__u8 *data = gspca_dev->usb_buf;
	int err_code;
	u8 status = 0;
	int tries = 0;

	err_code = cam_get_response16(gspca_dev);
	if (err_code < 0)
		return err_code;

	err_code = mr_write(gspca_dev, 1);
	data[0] = 0x19;
	data[1] = 0x51;
	err_code = mr_write(gspca_dev, 2);
	if (err_code < 0)
		return err_code;

	err_code = cam_get_response16(gspca_dev);
	if (err_code < 0)
		return err_code;

	data[0] = 0x19;
	data[1] = 0xba;
	err_code = mr_write(gspca_dev, 2);
	if (err_code < 0)
		return err_code;

	err_code = cam_get_response16(gspca_dev);
	if (err_code < 0)
		return err_code;

	data[0] = 0x19;
	data[1] = 0x00;
	err_code = mr_write(gspca_dev, 2);
	if (err_code < 0)
		return err_code;

	err_code = cam_get_response16(gspca_dev);
	if (err_code < 0)
		return err_code;

	data[0] = 0x19;
	data[1] = 0x00;
	err_code = mr_write(gspca_dev, 2);
	if (err_code < 0)
		return err_code;

	while (status != 0x0a && tries < 256) {
		err_code = cam_get_response16(gspca_dev);
		status = data[0];
		tries++;
		if (err_code < 0)
			return err_code;
	}
	if (status != 0x0a)
		PDEBUG(D_ERR, "status is %02x", status);

	tries = 0;
	while (tries < 4) {
		data[0] = 0x19;
		data[1] = 0x00;
		err_code = mr_write(gspca_dev, 2);
		if (err_code < 0)
			return err_code;

		err_code = cam_get_response16(gspca_dev);
		status = data[0];
		tries++;
		if (err_code < 0)
			return err_code;
	}

	data[0] = 0x19;
	err_code = mr_write(gspca_dev, 1);
	if (err_code < 0)
		return err_code;

	err_code = mr_read(gspca_dev, 16);
	if (err_code < 0)
		return err_code;

	return 0;
}

static u8 get_sensor_id(struct gspca_dev *gspca_dev)
{
	int err_code;

	gspca_dev->usb_buf[0] = 0x1e;
	err_code = mr_write(gspca_dev, 1);
	if (err_code < 0)
		return err_code;

	err_code = mr_read(gspca_dev, 16);
	if (err_code < 0)
		return err_code;

	PDEBUG(D_PROBE, "Byte zero reported is %01x", gspca_dev->usb_buf[0]);

	return gspca_dev->usb_buf[0];
}


static int sd_config(struct gspca_dev *gspca_dev,
		     const struct usb_device_id *id)
{
	struct sd *sd = (struct sd *) gspca_dev;
	struct cam *cam;
	__u8 *data = gspca_dev->usb_buf;
	int err_code;

	cam = &gspca_dev->cam;
	cam->cam_mode = vga_mode;
	cam->nmodes = ARRAY_SIZE(vga_mode);

	if (id->idProduct == 0x010e) {
		sd->cam_type = CAM_TYPE_CIF;
		cam->nmodes--;

		data[0] = 0x01;
		data[1] = 0x01;
		err_code = mr_write(gspca_dev, 2);
		if (err_code < 0)
			return err_code;

		msleep(200);
		data[0] = get_sensor_id(gspca_dev);
		
		if ((data[0] & 0x78) == 8 ||
		    ((data[0] & 0x2) == 0x2 && data[0] != 0x53))
			sd->sensor_type = 1;
		else
			sd->sensor_type = 0;

		PDEBUG(D_PROBE, "MR97310A CIF camera detected, sensor: %d",
		       sd->sensor_type);

		if (force_sensor_type != -1) {
			sd->sensor_type = !! force_sensor_type;
			PDEBUG(D_PROBE, "Forcing sensor type to: %d",
			       sd->sensor_type);
		}

		if (sd->sensor_type == 0)
			gspca_dev->ctrl_dis = (1 << BRIGHTNESS_IDX);
	} else {
		sd->cam_type = CAM_TYPE_VGA;
		PDEBUG(D_PROBE, "MR97310A VGA camera detected");
		gspca_dev->ctrl_dis = (1 << BRIGHTNESS_IDX) |
				      (1 << EXPOSURE_IDX) | (1 << GAIN_IDX);
	}

	sd->brightness = MR97310A_BRIGHTNESS_DEFAULT;
	sd->exposure = MR97310A_EXPOSURE_DEFAULT;
	sd->gain = MR97310A_GAIN_DEFAULT;

	return 0;
}


static int sd_init(struct gspca_dev *gspca_dev)
{
	return 0;
}

static int start_cif_cam(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;
	__u8 *data = gspca_dev->usb_buf;
	int err_code;
	const __u8 startup_string[] = {
		0x00,
		0x0d,
		0x01,
		0x00, 
		0x00, 
		0x13, 
		0x00, 
		0x00, 
		0x00, 
		0x50, 
		0xc0
	};

	
	data[0] = 0x01;
	data[1] = 0x01;
	err_code = mr_write(gspca_dev, 2);
	if (err_code < 0)
		return err_code;

	memcpy(data, startup_string, 11);
	if (sd->sensor_type)
		data[5] = 0xbb;

	switch (gspca_dev->width) {
	case 160:
		data[9] |= 0x04;  
		
	case 320:
	default:
		data[3] = 0x28;			   
		data[4] = 0x3c;			   
		data[6] = 0x14;			   
		data[8] = 0x1a + sd->sensor_type;  
		break;
	case 176:
		data[9] |= 0x04;  
		
	case 352:
		data[3] = 0x2c;			   
		data[4] = 0x48;			   
		data[6] = 0x06;			   
		data[8] = 0x06 - sd->sensor_type;  
		break;
	}
	err_code = mr_write(gspca_dev, 11);
	if (err_code < 0)
		return err_code;

	if (!sd->sensor_type) {
		const struct sensor_w_data cif_sensor0_init_data[] = {
			{0x02, 0x00, {0x03, 0x5a, 0xb5, 0x01,
				      0x0f, 0x14, 0x0f, 0x10}, 8},
			{0x0c, 0x00, {0x04, 0x01, 0x01, 0x00, 0x1f}, 5},
			{0x12, 0x00, {0x07}, 1},
			{0x1f, 0x00, {0x06}, 1},
			{0x27, 0x00, {0x04}, 1},
			{0x29, 0x00, {0x0c}, 1},
			{0x40, 0x00, {0x40, 0x00, 0x04}, 3},
			{0x50, 0x00, {0x60}, 1},
			{0x60, 0x00, {0x06}, 1},
			{0x6b, 0x00, {0x85, 0x85, 0xc8, 0xc8, 0xc8, 0xc8}, 6},
			{0x72, 0x00, {0x1e, 0x56}, 2},
			{0x75, 0x00, {0x58, 0x40, 0xa2, 0x02, 0x31, 0x02,
				      0x31, 0x80, 0x00}, 9},
			{0x11, 0x00, {0x01}, 1},
			{0, 0, {0}, 0}
		};
		err_code = sensor_write_regs(gspca_dev, cif_sensor0_init_data,
					 ARRAY_SIZE(cif_sensor0_init_data));
	} else {	
		const struct sensor_w_data cif_sensor1_init_data[] = {
			
			{0x02, 0x00, {0x10}, 1},
			{0x05, 0x01, {0x22}, 1}, 
			{0x06, 0x01, {0x00}, 1},
			{0x09, 0x02, {0x0e}, 1},
			{0x0a, 0x02, {0x05}, 1},
			{0x0b, 0x02, {0x05}, 1},
			{0x0c, 0x02, {0x0f}, 1},
			{0x0d, 0x02, {0x07}, 1},
			{0x0e, 0x02, {0x0c}, 1},
			{0x0f, 0x00, {0x00}, 1},
			{0x10, 0x00, {0x06}, 1},
			{0x11, 0x00, {0x07}, 1},
			{0x12, 0x00, {0x00}, 1},
			{0x13, 0x00, {0x01}, 1},
			{0, 0, {0}, 0}
		};
		err_code = sensor_write_regs(gspca_dev, cif_sensor1_init_data,
					 ARRAY_SIZE(cif_sensor1_init_data));
	}
	if (err_code < 0)
		return err_code;

	setbrightness(gspca_dev);
	setexposure(gspca_dev);
	setgain(gspca_dev);

	msleep(200);

	data[0] = 0x00;
	data[1] = 0x4d;  
	err_code = mr_write(gspca_dev, 2);
	if (err_code < 0)
		return err_code;

	return 0;
}

static int start_vga_cam(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;
	__u8 *data = gspca_dev->usb_buf;
	int err_code;
	const __u8 startup_string[] = {0x00, 0x0d, 0x01, 0x00, 0x00, 0x2b,
				       0x00, 0x00, 0x00, 0x50, 0xc0};

	
	sd->sof_read = 0;

	
	memset(data, 0, 16);
	data[0] = 0x20;
	err_code = mr_write(gspca_dev, 1);
	if (err_code < 0)
		return err_code;

	err_code = mr_read(gspca_dev, 16);
	if (err_code < 0)
		return err_code;

	PDEBUG(D_PROBE, "Byte reported is %02x", data[0]);

	msleep(200);
	
	sd->sensor_type = data[0] & 1;
	sd->do_lcd_stop = (~data[0]) & 1;



	


	data[0] = 0x01;
	data[1] = 0x01;
	err_code = mr_write(gspca_dev, 2);
	if (err_code < 0)
		return err_code;

	
	if (!sd->sensor_type) {
		data[0] = get_sensor_id(gspca_dev);
		if (data[0] == 0x7f) {
			sd->sensor_type = 1;
			PDEBUG(D_PROBE, "sensor_type corrected to 1");
		}
		msleep(200);
	}

	if (force_sensor_type != -1) {
		sd->sensor_type = !! force_sensor_type;
		PDEBUG(D_PROBE, "Forcing sensor type to: %d",
		       sd->sensor_type);
	}

	
	memcpy(data, startup_string, 11);
	if (!sd->sensor_type) {
		data[5]  = 0x00;
		data[10] = 0x91;
	}

	switch (gspca_dev->width) {
	case 160:
		data[9] |= 0x0c;  
		
	case 320:
		data[9] |= 0x04;  
		
	case 640:
	default:
		data[3] = 0x50;  
		data[4] = 0x78;  
		data[6] = 0x04;  
		data[8] = 0x03;  
		if (sd->do_lcd_stop)
			data[8] = 0x04;  
		break;

	case 176:
		data[9] |= 0x04;  
		
	case 352:
		data[3] = 0x2c;  
		data[4] = 0x48;  
		data[6] = 0x94;  
		data[8] = 0x63;  
		if (sd->do_lcd_stop)
			data[8] = 0x64;  
		break;
	}

	err_code = mr_write(gspca_dev, 11);
	if (err_code < 0)
		return err_code;

	if (!sd->sensor_type) {
		
		const struct sensor_w_data vga_sensor0_init_data[] = {
			{0x01, 0x00, {0x0c, 0x00, 0x04}, 3},
			{0x14, 0x00, {0x01, 0xe4, 0x02, 0x84}, 4},
			{0x20, 0x00, {0x00, 0x80, 0x00, 0x08}, 4},
			{0x25, 0x00, {0x03, 0xa9, 0x80}, 3},
			{0x30, 0x00, {0x30, 0x18, 0x10, 0x18}, 4},
			{0, 0, {0}, 0}
		};
		err_code = sensor_write_regs(gspca_dev, vga_sensor0_init_data,
					 ARRAY_SIZE(vga_sensor0_init_data));
	} else {	
		const struct sensor_w_data vga_sensor1_init_data[] = {
			{0x02, 0x00, {0x06, 0x59, 0x0c, 0x16, 0x00,
				0x07, 0x00, 0x01}, 8},
			{0x11, 0x04, {0x01}, 1},
			
			{0x0a, 0x00, {0x01, 0x06, 0x00, 0x00, 0x01,
				0x00, 0x0a}, 7},
			{0x11, 0x04, {0x01}, 1},
			{0x12, 0x00, {0x00, 0x63, 0x00, 0x70, 0x00, 0x00}, 6},
			{0x11, 0x04, {0x01}, 1},
			{0, 0, {0}, 0}
		};
		err_code = sensor_write_regs(gspca_dev, vga_sensor1_init_data,
					 ARRAY_SIZE(vga_sensor1_init_data));
	}
	if (err_code < 0)
		return err_code;

	msleep(200);
	data[0] = 0x00;
	data[1] = 0x4d;  
	err_code = mr_write(gspca_dev, 2);

	return err_code;
}

static int sd_start(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;
	int err_code;
	struct cam *cam;

	cam = &gspca_dev->cam;
	sd->sof_read = 0;
	
	zero_the_pointer(gspca_dev);
	msleep(200);
	if (sd->cam_type == CAM_TYPE_CIF) {
		err_code = start_cif_cam(gspca_dev);
	} else {
		err_code = start_vga_cam(gspca_dev);
	}
	return err_code;
}

static void sd_stopN(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;
	int result;

	gspca_dev->usb_buf[0] = 1;
	gspca_dev->usb_buf[1] = 0;
	result = mr_write(gspca_dev, 2);
	if (result < 0)
		PDEBUG(D_ERR, "Camera Stop failed");

	
	zero_the_pointer(gspca_dev);
	if (sd->do_lcd_stop) {
		gspca_dev->usb_buf[0] = 0x19;
		gspca_dev->usb_buf[1] = 0x54;
		result = mr_write(gspca_dev, 2);
		if (result < 0)
			PDEBUG(D_ERR, "Camera Stop failed");
	}
}

static void setbrightness(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;
	u8 val;

	if (gspca_dev->ctrl_dis & (1 << BRIGHTNESS_IDX))
		return;

	
	if (sd->brightness > 0) {
		sensor_write1(gspca_dev, 7, 0x00);
		val = sd->brightness;
	} else {
		sensor_write1(gspca_dev, 7, 0x01);
		val = 257 - sd->brightness;
	}
	sensor_write1(gspca_dev, 8, val);
}

static void setexposure(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;
	u8 val;

	if (gspca_dev->ctrl_dis & (1 << EXPOSURE_IDX))
		return;

	if (sd->sensor_type) {
		val = sd->exposure >> 4;
		sensor_write1(gspca_dev, 3, val);
		val = sd->exposure & 0xf;
		sensor_write1(gspca_dev, 4, val);
	} else {
		u8 clockdiv;
		int exposure;

		
		clockdiv = (60 * sd->exposure + 7999) / 8000;

		
		if (clockdiv < 3 && gspca_dev->width >= 320)
			clockdiv = 3;
		else if (clockdiv < 2)
			clockdiv = 2;

		
		exposure = (60 * 511 * sd->exposure) / (8000 * clockdiv);
		if (exposure > 511)
			exposure = 511;

		
		exposure = 511 - exposure;

		sensor_write1(gspca_dev, 0x02, clockdiv);
		sensor_write1(gspca_dev, 0x0e, exposure & 0xff);
		sensor_write1(gspca_dev, 0x0f, exposure >> 8);
	}
}

static void setgain(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;

	if (gspca_dev->ctrl_dis & (1 << GAIN_IDX))
		return;

	if (sd->sensor_type) {
		sensor_write1(gspca_dev, 0x0e, sd->gain);
	} else {
		sensor_write1(gspca_dev, 0x10, sd->gain);
	}
}

static int sd_setbrightness(struct gspca_dev *gspca_dev, __s32 val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	sd->brightness = val;
	if (gspca_dev->streaming)
		setbrightness(gspca_dev);
	return 0;
}

static int sd_getbrightness(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	*val = sd->brightness;
	return 0;
}

static int sd_setexposure(struct gspca_dev *gspca_dev, __s32 val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	sd->exposure = val;
	if (gspca_dev->streaming)
		setexposure(gspca_dev);
	return 0;
}

static int sd_getexposure(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	*val = sd->exposure;
	return 0;
}

static int sd_setgain(struct gspca_dev *gspca_dev, __s32 val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	sd->gain = val;
	if (gspca_dev->streaming)
		setgain(gspca_dev);
	return 0;
}

static int sd_getgain(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	*val = sd->gain;
	return 0;
}


#include "pac_common.h"

static void sd_pkt_scan(struct gspca_dev *gspca_dev,
			struct gspca_frame *frame,    
			__u8 *data,                   
			int len)                      
{
	unsigned char *sof;

	sof = pac_find_sof(gspca_dev, data, len);
	if (sof) {
		int n;

		
		n = sof - data;
		if (n > sizeof pac_sof_marker)
			n -= sizeof pac_sof_marker;
		else
			n = 0;
		frame = gspca_frame_add(gspca_dev, LAST_PACKET, frame,
					data, n);
		
		gspca_frame_add(gspca_dev, FIRST_PACKET, frame,
			pac_sof_marker, sizeof pac_sof_marker);
		len -= sof - data;
		data = sof;
	}
	gspca_frame_add(gspca_dev, INTER_PACKET, frame, data, len);
}


static const struct sd_desc sd_desc = {
	.name = MODULE_NAME,
	.ctrls = sd_ctrls,
	.nctrls = ARRAY_SIZE(sd_ctrls),
	.config = sd_config,
	.init = sd_init,
	.start = sd_start,
	.stopN = sd_stopN,
	.pkt_scan = sd_pkt_scan,
};


static const __devinitdata struct usb_device_id device_table[] = {
	{USB_DEVICE(0x08ca, 0x0111)},	
	{USB_DEVICE(0x093a, 0x010f)},	
	{USB_DEVICE(0x093a, 0x010e)},	
	{}
};
MODULE_DEVICE_TABLE(usb, device_table);


static int sd_probe(struct usb_interface *intf,
		    const struct usb_device_id *id)
{
	return gspca_dev_probe(intf, id, &sd_desc, sizeof(struct sd),
			       THIS_MODULE);
}

static struct usb_driver sd_driver = {
	.name = MODULE_NAME,
	.id_table = device_table,
	.probe = sd_probe,
	.disconnect = gspca_disconnect,
#ifdef CONFIG_PM
	.suspend = gspca_suspend,
	.resume = gspca_resume,
#endif
};


static int __init sd_mod_init(void)
{
	int ret;

	ret = usb_register(&sd_driver);
	if (ret < 0)
		return ret;
	PDEBUG(D_PROBE, "registered");
	return 0;
}
static void __exit sd_mod_exit(void)
{
	usb_deregister(&sd_driver);
	PDEBUG(D_PROBE, "deregistered");
}

module_init(sd_mod_init);
module_exit(sd_mod_exit);
