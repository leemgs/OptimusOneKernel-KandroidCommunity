

#define MODULE_NAME "pac207"

#include "gspca.h"

MODULE_AUTHOR("Hans de Goede <hdgoede@redhat.com>");
MODULE_DESCRIPTION("Pixart PAC207");
MODULE_LICENSE("GPL");

#define PAC207_CTRL_TIMEOUT		100  

#define PAC207_BRIGHTNESS_MIN		0
#define PAC207_BRIGHTNESS_MAX		255
#define PAC207_BRIGHTNESS_DEFAULT	46

#define PAC207_EXPOSURE_MIN		3
#define PAC207_EXPOSURE_MAX		26
#define PAC207_EXPOSURE_DEFAULT		5 
#define PAC207_EXPOSURE_KNEE		8 

#define PAC207_GAIN_MIN			0
#define PAC207_GAIN_MAX			31
#define PAC207_GAIN_DEFAULT         	9 
#define PAC207_GAIN_KNEE		31

#define PAC207_AUTOGAIN_DEADZONE	30


struct sd {
	struct gspca_dev gspca_dev;		

	u8 mode;

	u8 brightness;
	u8 exposure;
	u8 autogain;
	u8 gain;

	u8 sof_read;
	u8 header_read;
	u8 autogain_ignore_frames;

	atomic_t avg_lum;
};


static int sd_setbrightness(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getbrightness(struct gspca_dev *gspca_dev, __s32 *val);
static int sd_setexposure(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getexposure(struct gspca_dev *gspca_dev, __s32 *val);
static int sd_setautogain(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getautogain(struct gspca_dev *gspca_dev, __s32 *val);
static int sd_setgain(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getgain(struct gspca_dev *gspca_dev, __s32 *val);

static struct ctrl sd_ctrls[] = {
#define SD_BRIGHTNESS 0
	{
	    {
		.id      = V4L2_CID_BRIGHTNESS,
		.type    = V4L2_CTRL_TYPE_INTEGER,
		.name    = "Brightness",
		.minimum = PAC207_BRIGHTNESS_MIN,
		.maximum = PAC207_BRIGHTNESS_MAX,
		.step = 1,
		.default_value = PAC207_BRIGHTNESS_DEFAULT,
		.flags = 0,
	    },
	    .set = sd_setbrightness,
	    .get = sd_getbrightness,
	},
#define SD_EXPOSURE 1
	{
	    {
		.id = V4L2_CID_EXPOSURE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "exposure",
		.minimum = PAC207_EXPOSURE_MIN,
		.maximum = PAC207_EXPOSURE_MAX,
		.step = 1,
		.default_value = PAC207_EXPOSURE_DEFAULT,
		.flags = 0,
	    },
	    .set = sd_setexposure,
	    .get = sd_getexposure,
	},
#define SD_AUTOGAIN 2
	{
	    {
		.id	  = V4L2_CID_AUTOGAIN,
		.type	= V4L2_CTRL_TYPE_BOOLEAN,
		.name	= "Auto Gain",
		.minimum = 0,
		.maximum = 1,
		.step	= 1,
#define AUTOGAIN_DEF 1
		.default_value = AUTOGAIN_DEF,
		.flags = 0,
	    },
	    .set = sd_setautogain,
	    .get = sd_getautogain,
	},
#define SD_GAIN 3
	{
	    {
		.id = V4L2_CID_GAIN,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "gain",
		.minimum = PAC207_GAIN_MIN,
		.maximum = PAC207_GAIN_MAX,
		.step = 1,
		.default_value = PAC207_GAIN_DEFAULT,
		.flags = 0,
	    },
	    .set = sd_setgain,
	    .get = sd_getgain,
	},
};

static const struct v4l2_pix_format sif_mode[] = {
	{176, 144, V4L2_PIX_FMT_PAC207, V4L2_FIELD_NONE,
		.bytesperline = 176,
		.sizeimage = (176 + 2) * 144,
			
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 1},
	{352, 288, V4L2_PIX_FMT_PAC207, V4L2_FIELD_NONE,
		.bytesperline = 352,
			
		.sizeimage = (352 + 2) * 288,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 0},
};

static const __u8 pac207_sensor_init[][8] = {
	{0x10, 0x12, 0x0d, 0x12, 0x0c, 0x01, 0x29, 0x84},
	{0x49, 0x64, 0x64, 0x64, 0x04, 0x10, 0xf0, 0x30},
	{0x00, 0x00, 0x00, 0x70, 0xa0, 0xf8, 0x00, 0x00},
	{0x32, 0x00, 0x96, 0x00, 0xA2, 0x02, 0xaf, 0x00},
};

static int pac207_write_regs(struct gspca_dev *gspca_dev, u16 index,
	const u8 *buffer, u16 length)
{
	struct usb_device *udev = gspca_dev->dev;
	int err;

	memcpy(gspca_dev->usb_buf, buffer, length);

	err = usb_control_msg(udev, usb_sndctrlpipe(udev, 0), 0x01,
			USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_INTERFACE,
			0x00, index,
			gspca_dev->usb_buf, length, PAC207_CTRL_TIMEOUT);
	if (err < 0)
		PDEBUG(D_ERR,
			"Failed to write registers to index 0x%04X, error %d)",
			index, err);

	return err;
}


static int pac207_write_reg(struct gspca_dev *gspca_dev, u16 index, u16 value)
{
	struct usb_device *udev = gspca_dev->dev;
	int err;

	err = usb_control_msg(udev, usb_sndctrlpipe(udev, 0), 0x00,
			USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_INTERFACE,
			value, index, NULL, 0, PAC207_CTRL_TIMEOUT);
	if (err)
		PDEBUG(D_ERR, "Failed to write a register (index 0x%04X,"
			" value 0x%02X, error %d)", index, value, err);

	return err;
}

static int pac207_read_reg(struct gspca_dev *gspca_dev, u16 index)
{
	struct usb_device *udev = gspca_dev->dev;
	int res;

	res = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0), 0x00,
			USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_INTERFACE,
			0x00, index,
			gspca_dev->usb_buf, 1, PAC207_CTRL_TIMEOUT);
	if (res < 0) {
		PDEBUG(D_ERR,
			"Failed to read a register (index 0x%04X, error %d)",
			index, res);
		return res;
	}

	return gspca_dev->usb_buf[0];
}


static int sd_config(struct gspca_dev *gspca_dev,
			const struct usb_device_id *id)
{
	struct sd *sd = (struct sd *) gspca_dev;
	struct cam *cam;
	u8 idreg[2];

	idreg[0] = pac207_read_reg(gspca_dev, 0x0000);
	idreg[1] = pac207_read_reg(gspca_dev, 0x0001);
	idreg[0] = ((idreg[0] & 0x0F) << 4) | ((idreg[1] & 0xf0) >> 4);
	idreg[1] = idreg[1] & 0x0f;
	PDEBUG(D_PROBE, "Pixart Sensor ID 0x%02X Chips ID 0x%02X",
		idreg[0], idreg[1]);

	if (idreg[0] != 0x27) {
		PDEBUG(D_PROBE, "Error invalid sensor ID!");
		return -ENODEV;
	}

	PDEBUG(D_PROBE,
		"Pixart PAC207BCA Image Processor and Control Chip detected"
		" (vid/pid 0x%04X:0x%04X)", id->idVendor, id->idProduct);

	cam = &gspca_dev->cam;
	cam->cam_mode = sif_mode;
	cam->nmodes = ARRAY_SIZE(sif_mode);
	sd->brightness = PAC207_BRIGHTNESS_DEFAULT;
	sd->exposure = PAC207_EXPOSURE_DEFAULT;
	sd->gain = PAC207_GAIN_DEFAULT;
	sd->autogain = AUTOGAIN_DEF;

	return 0;
}


static int sd_init(struct gspca_dev *gspca_dev)
{
	pac207_write_reg(gspca_dev, 0x41, 0x00);
				
	pac207_write_reg(gspca_dev, 0x0f, 0x00); 

	return 0;
}


static int sd_start(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;
	__u8 mode;

	pac207_write_reg(gspca_dev, 0x0f, 0x10); 
	pac207_write_regs(gspca_dev, 0x0002, pac207_sensor_init[0], 8);
	pac207_write_regs(gspca_dev, 0x000a, pac207_sensor_init[1], 8);
	pac207_write_regs(gspca_dev, 0x0012, pac207_sensor_init[2], 8);
	pac207_write_regs(gspca_dev, 0x0042, pac207_sensor_init[3], 8);

	
	if (gspca_dev->width == 176)
		pac207_write_reg(gspca_dev, 0x4a, 0xff);
	else
		pac207_write_reg(gspca_dev, 0x4a, 0x30);
	pac207_write_reg(gspca_dev, 0x4b, 0x00); 
	pac207_write_reg(gspca_dev, 0x08, sd->brightness);

	
	pac207_write_reg(gspca_dev, 0x0e, sd->gain);
	pac207_write_reg(gspca_dev, 0x02, sd->exposure); 

	mode = 0x02; 
	if (gspca_dev->width == 176) {	
		mode |= 0x01;
		PDEBUG(D_STREAM, "pac207_start mode 176x144");
	} else {				
		PDEBUG(D_STREAM, "pac207_start mode 352x288");
	}
	pac207_write_reg(gspca_dev, 0x41, mode);

	pac207_write_reg(gspca_dev, 0x13, 0x01); 
	pac207_write_reg(gspca_dev, 0x1c, 0x01); 
	msleep(10);
	pac207_write_reg(gspca_dev, 0x40, 0x01); 

	sd->sof_read = 0;
	sd->autogain_ignore_frames = 0;
	atomic_set(&sd->avg_lum, -1);
	return 0;
}

static void sd_stopN(struct gspca_dev *gspca_dev)
{
	pac207_write_reg(gspca_dev, 0x40, 0x00); 
	pac207_write_reg(gspca_dev, 0x41, 0x00); 
	pac207_write_reg(gspca_dev, 0x0f, 0x00); 
}


#include "pac_common.h"

static void pac207_do_auto_gain(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;
	int avg_lum = atomic_read(&sd->avg_lum);

	if (avg_lum == -1)
		return;

	if (sd->autogain_ignore_frames > 0)
		sd->autogain_ignore_frames--;
	else if (gspca_auto_gain_n_exposure(gspca_dev, avg_lum,
			100, PAC207_AUTOGAIN_DEADZONE,
			PAC207_GAIN_KNEE, PAC207_EXPOSURE_KNEE))
		sd->autogain_ignore_frames = PAC_AUTOGAIN_IGNORE_FRAMES;
}

static void sd_pkt_scan(struct gspca_dev *gspca_dev,
			struct gspca_frame *frame,
			__u8 *data,
			int len)
{
	struct sd *sd = (struct sd *) gspca_dev;
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
		sd->header_read = 0;
		gspca_frame_add(gspca_dev, FIRST_PACKET, frame, NULL, 0);
		len -= sof - data;
		data = sof;
	}
	if (sd->header_read < 11) {
		int needed;

		
		if (sd->header_read < 5) {
			needed = 5 - sd->header_read;
			if (len >= needed)
				atomic_set(&sd->avg_lum, data[needed - 1]);
		}
		
		needed = 11 - sd->header_read;
		if (len <= needed) {
			sd->header_read += len;
			return;
		}
		data += needed;
		len -= needed;
		sd->header_read = 11;
	}

	gspca_frame_add(gspca_dev, INTER_PACKET, frame, data, len);
}

static void setbrightness(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;

	pac207_write_reg(gspca_dev, 0x08, sd->brightness);
	pac207_write_reg(gspca_dev, 0x13, 0x01);	
	pac207_write_reg(gspca_dev, 0x1c, 0x01);	
}

static void setexposure(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;

	pac207_write_reg(gspca_dev, 0x02, sd->exposure);
	pac207_write_reg(gspca_dev, 0x13, 0x01);	
	pac207_write_reg(gspca_dev, 0x1c, 0x01);	
}

static void setgain(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;

	pac207_write_reg(gspca_dev, 0x0e, sd->gain);
	pac207_write_reg(gspca_dev, 0x13, 0x01);	
	pac207_write_reg(gspca_dev, 0x1c, 0x01);	
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

static int sd_setautogain(struct gspca_dev *gspca_dev, __s32 val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	sd->autogain = val;
	
	if (sd->autogain) {
		sd->exposure = PAC207_EXPOSURE_DEFAULT;
		sd->gain = PAC207_GAIN_DEFAULT;
		if (gspca_dev->streaming) {
			sd->autogain_ignore_frames =
				PAC_AUTOGAIN_IGNORE_FRAMES;
			setexposure(gspca_dev);
			setgain(gspca_dev);
		}
	}

	return 0;
}

static int sd_getautogain(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	*val = sd->autogain;
	return 0;
}


static const struct sd_desc sd_desc = {
	.name = MODULE_NAME,
	.ctrls = sd_ctrls,
	.nctrls = ARRAY_SIZE(sd_ctrls),
	.config = sd_config,
	.init = sd_init,
	.start = sd_start,
	.stopN = sd_stopN,
	.dq_callback = pac207_do_auto_gain,
	.pkt_scan = sd_pkt_scan,
};


static const __devinitdata struct usb_device_id device_table[] = {
	{USB_DEVICE(0x041e, 0x4028)},
	{USB_DEVICE(0x093a, 0x2460)},
	{USB_DEVICE(0x093a, 0x2461)},
	{USB_DEVICE(0x093a, 0x2463)},
	{USB_DEVICE(0x093a, 0x2464)},
	{USB_DEVICE(0x093a, 0x2468)},
	{USB_DEVICE(0x093a, 0x2470)},
	{USB_DEVICE(0x093a, 0x2471)},
	{USB_DEVICE(0x093a, 0x2472)},
	{USB_DEVICE(0x093a, 0x2474)},
	{USB_DEVICE(0x093a, 0x2476)},
	{USB_DEVICE(0x145f, 0x013a)},
	{USB_DEVICE(0x2001, 0xf115)},
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
