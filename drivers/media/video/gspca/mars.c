

#define MODULE_NAME "mars"

#include "gspca.h"
#include "jpeg.h"

MODULE_AUTHOR("Michel Xhaard <mxhaard@users.sourceforge.net>");
MODULE_DESCRIPTION("GSPCA/Mars USB Camera Driver");
MODULE_LICENSE("GPL");


struct sd {
	struct gspca_dev gspca_dev;	

	u8 brightness;
	u8 colors;
	u8 gamma;
	u8 sharpness;
	u8 quality;
#define QUALITY_MIN 40
#define QUALITY_MAX 70
#define QUALITY_DEF 50

	u8 *jpeg_hdr;
};


static int sd_setbrightness(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getbrightness(struct gspca_dev *gspca_dev, __s32 *val);
static int sd_setcolors(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getcolors(struct gspca_dev *gspca_dev, __s32 *val);
static int sd_setgamma(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getgamma(struct gspca_dev *gspca_dev, __s32 *val);
static int sd_setsharpness(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getsharpness(struct gspca_dev *gspca_dev, __s32 *val);

static struct ctrl sd_ctrls[] = {
	{
	    {
		.id      = V4L2_CID_BRIGHTNESS,
		.type    = V4L2_CTRL_TYPE_INTEGER,
		.name    = "Brightness",
		.minimum = 0,
		.maximum = 30,
		.step    = 1,
#define BRIGHTNESS_DEF 15
		.default_value = BRIGHTNESS_DEF,
	    },
	    .set = sd_setbrightness,
	    .get = sd_getbrightness,
	},
	{
	    {
		.id      = V4L2_CID_SATURATION,
		.type    = V4L2_CTRL_TYPE_INTEGER,
		.name    = "Color",
		.minimum = 1,
		.maximum = 255,
		.step    = 1,
#define COLOR_DEF 200
		.default_value = COLOR_DEF,
	    },
	    .set = sd_setcolors,
	    .get = sd_getcolors,
	},
	{
	    {
		.id      = V4L2_CID_GAMMA,
		.type    = V4L2_CTRL_TYPE_INTEGER,
		.name    = "Gamma",
		.minimum = 0,
		.maximum = 3,
		.step    = 1,
#define GAMMA_DEF 1
		.default_value = GAMMA_DEF,
	    },
	    .set = sd_setgamma,
	    .get = sd_getgamma,
	},
	{
	    {
		.id	 = V4L2_CID_SHARPNESS,
		.type    = V4L2_CTRL_TYPE_INTEGER,
		.name    = "Sharpness",
		.minimum = 0,
		.maximum = 2,
		.step    = 1,
#define SHARPNESS_DEF 1
		.default_value = SHARPNESS_DEF,
	    },
	    .set = sd_setsharpness,
	    .get = sd_getsharpness,
	},
};

static const struct v4l2_pix_format vga_mode[] = {
	{320, 240, V4L2_PIX_FMT_JPEG, V4L2_FIELD_NONE,
		.bytesperline = 320,
		.sizeimage = 320 * 240 * 3 / 8 + 590,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 2},
	{640, 480, V4L2_PIX_FMT_JPEG, V4L2_FIELD_NONE,
		.bytesperline = 640,
		.sizeimage = 640 * 480 * 3 / 8 + 590,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 1},
};

static const __u8 mi_data[0x20] = {

	0x48, 0x22, 0x01, 0x47, 0x10, 0x00, 0x00, 0x00,

	0x00, 0x01, 0x30, 0x01, 0x30, 0x01, 0x30, 0x01,

	0x30, 0x00, 0x04, 0x00, 0x06, 0x01, 0xe2, 0x02,

	0x82, 0x00, 0x20, 0x17, 0x80, 0x08, 0x0c, 0x00
};


static int reg_w(struct gspca_dev *gspca_dev,
		 int len)
{
	int alen, ret;

	ret = usb_bulk_msg(gspca_dev->dev,
			usb_sndbulkpipe(gspca_dev->dev, 4),
			gspca_dev->usb_buf,
			len,
			&alen,
			500);	
	if (ret < 0)
		PDEBUG(D_ERR, "reg write [%02x] error %d",
			gspca_dev->usb_buf[0], ret);
	return ret;
}

static void mi_w(struct gspca_dev *gspca_dev,
		 u8 addr,
		 u8 value)
{
	gspca_dev->usb_buf[0] = 0x1f;
	gspca_dev->usb_buf[1] = 0;			
	gspca_dev->usb_buf[2] = addr;
	gspca_dev->usb_buf[3] = value;

	reg_w(gspca_dev, 4);
}


static int sd_config(struct gspca_dev *gspca_dev,
			const struct usb_device_id *id)
{
	struct sd *sd = (struct sd *) gspca_dev;
	struct cam *cam;

	cam = &gspca_dev->cam;
	cam->cam_mode = vga_mode;
	cam->nmodes = ARRAY_SIZE(vga_mode);
	sd->brightness = BRIGHTNESS_DEF;
	sd->colors = COLOR_DEF;
	sd->gamma = GAMMA_DEF;
	sd->sharpness = SHARPNESS_DEF;
	sd->quality = QUALITY_DEF;
	gspca_dev->nbalt = 9;		
	return 0;
}


static int sd_init(struct gspca_dev *gspca_dev)
{
	return 0;
}

static int sd_start(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;
	int err_code;
	u8 *data;
	int i;

	
	sd->jpeg_hdr = kmalloc(JPEG_HDR_SZ, GFP_KERNEL);
	if (!sd->jpeg_hdr)
		return -ENOMEM;
	jpeg_define(sd->jpeg_hdr, gspca_dev->height, gspca_dev->width,
			0x21);		
	jpeg_set_qual(sd->jpeg_hdr, sd->quality);

	data = gspca_dev->usb_buf;

	data[0] = 0x01;		
	data[1] = 0x01;
	err_code = reg_w(gspca_dev, 2);
	if (err_code < 0)
		return err_code;

	
	data[0] = 0x00;		
	data[1] = 0x0c | 0x01;	
	data[2] = 0x01;		
	data[3] = gspca_dev->width / 8;		
	data[4] = gspca_dev->height / 8;	
	data[5] = 0x30;		
	data[6] = 0x02;		
	data[7] = sd->gamma * 0x40;	
	data[8] = 0x01;		



	data[9] = 0x52;		

	data[10] = 0x18;

	err_code = reg_w(gspca_dev, 11);
	if (err_code < 0)
		return err_code;

	data[0] = 0x23;		
	data[1] = 0x09;		

	err_code = reg_w(gspca_dev, 2);
	if (err_code < 0)
		return err_code;

	data[0] = 0x3c;		



	data[1] = 50;		
	err_code = reg_w(gspca_dev, 2);
	if (err_code < 0)
		return err_code;

	
	data[0] = 0x5e;		
	data[1] = 0;		

				
				
	data[2] = sd->colors << 3;
	data[3] = ((sd->colors >> 2) & 0xf8) | 0x04;
	data[4] = sd->brightness; 
	data[5] = 0x00;

	err_code = reg_w(gspca_dev, 6);
	if (err_code < 0)
		return err_code;

	data[0] = 0x67;

	data[1] = sd->sharpness * 4 + 3;
	data[2] = 0x14;
	err_code = reg_w(gspca_dev, 3);
	if (err_code < 0)
		return err_code;

	data[0] = 0x69;
	data[1] = 0x2f;
	data[2] = 0x28;
	data[3] = 0x42;
	err_code = reg_w(gspca_dev, 4);
	if (err_code < 0)
		return err_code;

	data[0] = 0x63;
	data[1] = 0x07;
	err_code = reg_w(gspca_dev, 2);

	if (err_code < 0)
		return err_code;

	
	for (i = 0; i < sizeof mi_data; i++)
		mi_w(gspca_dev, i + 1, mi_data[i]);

	data[0] = 0x00;
	data[1] = 0x4d;		
	reg_w(gspca_dev, 2);
	return 0;
}

static void sd_stopN(struct gspca_dev *gspca_dev)
{
	int result;

	gspca_dev->usb_buf[0] = 1;
	gspca_dev->usb_buf[1] = 0;
	result = reg_w(gspca_dev, 2);
	if (result < 0)
		PDEBUG(D_ERR, "Camera Stop failed");
}

static void sd_stop0(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;

	kfree(sd->jpeg_hdr);
}

static void sd_pkt_scan(struct gspca_dev *gspca_dev,
			struct gspca_frame *frame,	
			__u8 *data,			
			int len)			
{
	struct sd *sd = (struct sd *) gspca_dev;
	int p;

	if (len < 6) {

		return;
	}
	for (p = 0; p < len - 6; p++) {
		if (data[0 + p] == 0xff
		    && data[1 + p] == 0xff
		    && data[2 + p] == 0x00
		    && data[3 + p] == 0xff
		    && data[4 + p] == 0x96) {
			if (data[5 + p] == 0x64
			    || data[5 + p] == 0x65
			    || data[5 + p] == 0x66
			    || data[5 + p] == 0x67) {
				PDEBUG(D_PACK, "sof offset: %d len: %d",
					p, len);
				frame = gspca_frame_add(gspca_dev, LAST_PACKET,
							frame, data, p);

				
				gspca_frame_add(gspca_dev, FIRST_PACKET, frame,
					sd->jpeg_hdr, JPEG_HDR_SZ);
				data += p + 16;
				len -= p + 16;
				break;
			}
		}
	}
	gspca_frame_add(gspca_dev, INTER_PACKET, frame, data, len);
}

static int sd_setbrightness(struct gspca_dev *gspca_dev, __s32 val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	sd->brightness = val;
	if (gspca_dev->streaming) {
		gspca_dev->usb_buf[0] = 0x61;
		gspca_dev->usb_buf[1] = val;
		reg_w(gspca_dev, 2);
	}
	return 0;
}

static int sd_getbrightness(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	*val = sd->brightness;
	return 0;
}

static int sd_setcolors(struct gspca_dev *gspca_dev, __s32 val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	sd->colors = val;
	if (gspca_dev->streaming) {

		
		gspca_dev->usb_buf[0] = 0x5f;
		gspca_dev->usb_buf[1] = sd->colors << 3;
		gspca_dev->usb_buf[2] = ((sd->colors >> 2) & 0xf8) | 0x04;
		reg_w(gspca_dev, 3);
	}
	return 0;
}

static int sd_getcolors(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	*val = sd->colors;
	return 0;
}

static int sd_setgamma(struct gspca_dev *gspca_dev, __s32 val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	sd->gamma = val;
	if (gspca_dev->streaming) {
		gspca_dev->usb_buf[0] = 0x06;
		gspca_dev->usb_buf[1] = val * 0x40;
		reg_w(gspca_dev, 2);
	}
	return 0;
}

static int sd_getgamma(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	*val = sd->gamma;
	return 0;
}

static int sd_setsharpness(struct gspca_dev *gspca_dev, __s32 val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	sd->sharpness = val;
	if (gspca_dev->streaming) {
		gspca_dev->usb_buf[0] = 0x67;
		gspca_dev->usb_buf[1] = val * 4 + 3;
		reg_w(gspca_dev, 2);
	}
	return 0;
}

static int sd_getsharpness(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	*val = sd->sharpness;
	return 0;
}

static int sd_set_jcomp(struct gspca_dev *gspca_dev,
			struct v4l2_jpegcompression *jcomp)
{
	struct sd *sd = (struct sd *) gspca_dev;

	if (jcomp->quality < QUALITY_MIN)
		sd->quality = QUALITY_MIN;
	else if (jcomp->quality > QUALITY_MAX)
		sd->quality = QUALITY_MAX;
	else
		sd->quality = jcomp->quality;
	if (gspca_dev->streaming)
		jpeg_set_qual(sd->jpeg_hdr, sd->quality);
	return 0;
}

static int sd_get_jcomp(struct gspca_dev *gspca_dev,
			struct v4l2_jpegcompression *jcomp)
{
	struct sd *sd = (struct sd *) gspca_dev;

	memset(jcomp, 0, sizeof *jcomp);
	jcomp->quality = sd->quality;
	jcomp->jpeg_markers = V4L2_JPEG_MARKER_DHT
			| V4L2_JPEG_MARKER_DQT;
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
	.stop0 = sd_stop0,
	.pkt_scan = sd_pkt_scan,
	.get_jcomp = sd_get_jcomp,
	.set_jcomp = sd_set_jcomp,
};


static const __devinitdata struct usb_device_id device_table[] = {
	{USB_DEVICE(0x093a, 0x050f)},
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
