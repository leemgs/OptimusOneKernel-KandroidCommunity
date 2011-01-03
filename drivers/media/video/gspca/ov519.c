
#define MODULE_NAME "ov519"

#include "gspca.h"

MODULE_AUTHOR("Jean-Francois Moine <http://moinejf.free.fr>");
MODULE_DESCRIPTION("OV519 USB Camera Driver");
MODULE_LICENSE("GPL");


static int frame_rate;


static int i2c_detect_tries = 10;


struct sd {
	struct gspca_dev gspca_dev;		

	__u8 packet_nr;

	char bridge;
#define BRIDGE_OV511		0
#define BRIDGE_OV511PLUS	1
#define BRIDGE_OV518		2
#define BRIDGE_OV518PLUS	3
#define BRIDGE_OV519		4
#define BRIDGE_MASK		7

	char invert_led;
#define BRIDGE_INVERT_LED	8

	
	__u8 sif;

	__u8 brightness;
	__u8 contrast;
	__u8 colors;
	__u8 hflip;
	__u8 vflip;
	__u8 autobrightness;
	__u8 freq;

	__u8 stopped;		

	__u8 frame_rate;	
	__u8 clockdiv;		

	char sensor;		
#define SEN_UNKNOWN 0
#define SEN_OV6620 1
#define SEN_OV6630 2
#define SEN_OV66308AF 3
#define SEN_OV7610 4
#define SEN_OV7620 5
#define SEN_OV7640 6
#define SEN_OV7670 7
#define SEN_OV76BE 8
#define SEN_OV8610 9
};


static int sd_setbrightness(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getbrightness(struct gspca_dev *gspca_dev, __s32 *val);
static int sd_setcontrast(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getcontrast(struct gspca_dev *gspca_dev, __s32 *val);
static int sd_setcolors(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getcolors(struct gspca_dev *gspca_dev, __s32 *val);
static int sd_sethflip(struct gspca_dev *gspca_dev, __s32 val);
static int sd_gethflip(struct gspca_dev *gspca_dev, __s32 *val);
static int sd_setvflip(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getvflip(struct gspca_dev *gspca_dev, __s32 *val);
static int sd_setautobrightness(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getautobrightness(struct gspca_dev *gspca_dev, __s32 *val);
static int sd_setfreq(struct gspca_dev *gspca_dev, __s32 val);
static int sd_getfreq(struct gspca_dev *gspca_dev, __s32 *val);
static void setbrightness(struct gspca_dev *gspca_dev);
static void setcontrast(struct gspca_dev *gspca_dev);
static void setcolors(struct gspca_dev *gspca_dev);
static void setautobrightness(struct sd *sd);
static void setfreq(struct sd *sd);

static const struct ctrl sd_ctrls[] = {
	{
	    {
		.id      = V4L2_CID_BRIGHTNESS,
		.type    = V4L2_CTRL_TYPE_INTEGER,
		.name    = "Brightness",
		.minimum = 0,
		.maximum = 255,
		.step    = 1,
#define BRIGHTNESS_DEF 127
		.default_value = BRIGHTNESS_DEF,
	    },
	    .set = sd_setbrightness,
	    .get = sd_getbrightness,
	},
	{
	    {
		.id      = V4L2_CID_CONTRAST,
		.type    = V4L2_CTRL_TYPE_INTEGER,
		.name    = "Contrast",
		.minimum = 0,
		.maximum = 255,
		.step    = 1,
#define CONTRAST_DEF 127
		.default_value = CONTRAST_DEF,
	    },
	    .set = sd_setcontrast,
	    .get = sd_getcontrast,
	},
	{
	    {
		.id      = V4L2_CID_SATURATION,
		.type    = V4L2_CTRL_TYPE_INTEGER,
		.name    = "Color",
		.minimum = 0,
		.maximum = 255,
		.step    = 1,
#define COLOR_DEF 127
		.default_value = COLOR_DEF,
	    },
	    .set = sd_setcolors,
	    .get = sd_getcolors,
	},

#define HFLIP_IDX 3
	{
	    {
		.id      = V4L2_CID_HFLIP,
		.type    = V4L2_CTRL_TYPE_BOOLEAN,
		.name    = "Mirror",
		.minimum = 0,
		.maximum = 1,
		.step    = 1,
#define HFLIP_DEF 0
		.default_value = HFLIP_DEF,
	    },
	    .set = sd_sethflip,
	    .get = sd_gethflip,
	},
#define VFLIP_IDX 4
	{
	    {
		.id      = V4L2_CID_VFLIP,
		.type    = V4L2_CTRL_TYPE_BOOLEAN,
		.name    = "Vflip",
		.minimum = 0,
		.maximum = 1,
		.step    = 1,
#define VFLIP_DEF 0
		.default_value = VFLIP_DEF,
	    },
	    .set = sd_setvflip,
	    .get = sd_getvflip,
	},
#define AUTOBRIGHT_IDX 5
	{
	    {
		.id      = V4L2_CID_AUTOBRIGHTNESS,
		.type    = V4L2_CTRL_TYPE_BOOLEAN,
		.name    = "Auto Brightness",
		.minimum = 0,
		.maximum = 1,
		.step    = 1,
#define AUTOBRIGHT_DEF 1
		.default_value = AUTOBRIGHT_DEF,
	    },
	    .set = sd_setautobrightness,
	    .get = sd_getautobrightness,
	},
#define FREQ_IDX 6
	{
	    {
		.id	 = V4L2_CID_POWER_LINE_FREQUENCY,
		.type    = V4L2_CTRL_TYPE_MENU,
		.name    = "Light frequency filter",
		.minimum = 0,
		.maximum = 2,	
		.step    = 1,
#define FREQ_DEF 0
		.default_value = FREQ_DEF,
	    },
	    .set = sd_setfreq,
	    .get = sd_getfreq,
	},
#define OV7670_FREQ_IDX 7
	{
	    {
		.id	 = V4L2_CID_POWER_LINE_FREQUENCY,
		.type    = V4L2_CTRL_TYPE_MENU,
		.name    = "Light frequency filter",
		.minimum = 0,
		.maximum = 3,	
		.step    = 1,
#define OV7670_FREQ_DEF 3
		.default_value = OV7670_FREQ_DEF,
	    },
	    .set = sd_setfreq,
	    .get = sd_getfreq,
	},
};

static const struct v4l2_pix_format ov519_vga_mode[] = {
	{320, 240, V4L2_PIX_FMT_JPEG, V4L2_FIELD_NONE,
		.bytesperline = 320,
		.sizeimage = 320 * 240 * 3 / 8 + 590,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 1},
	{640, 480, V4L2_PIX_FMT_JPEG, V4L2_FIELD_NONE,
		.bytesperline = 640,
		.sizeimage = 640 * 480 * 3 / 8 + 590,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 0},
};
static const struct v4l2_pix_format ov519_sif_mode[] = {
	{160, 120, V4L2_PIX_FMT_JPEG, V4L2_FIELD_NONE,
		.bytesperline = 160,
		.sizeimage = 160 * 120 * 3 / 8 + 590,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 3},
	{176, 144, V4L2_PIX_FMT_JPEG, V4L2_FIELD_NONE,
		.bytesperline = 176,
		.sizeimage = 176 * 144 * 3 / 8 + 590,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 1},
	{320, 240, V4L2_PIX_FMT_JPEG, V4L2_FIELD_NONE,
		.bytesperline = 320,
		.sizeimage = 320 * 240 * 3 / 8 + 590,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 2},
	{352, 288, V4L2_PIX_FMT_JPEG, V4L2_FIELD_NONE,
		.bytesperline = 352,
		.sizeimage = 352 * 288 * 3 / 8 + 590,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 0},
};


static const struct v4l2_pix_format ov518_vga_mode[] = {
	{320, 240, V4L2_PIX_FMT_OV518, V4L2_FIELD_NONE,
		.bytesperline = 320,
		.sizeimage = 320 * 240 * 3,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 1},
	{640, 480, V4L2_PIX_FMT_OV518, V4L2_FIELD_NONE,
		.bytesperline = 640,
		.sizeimage = 640 * 480 * 2,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 0},
};
static const struct v4l2_pix_format ov518_sif_mode[] = {
	{160, 120, V4L2_PIX_FMT_OV518, V4L2_FIELD_NONE,
		.bytesperline = 160,
		.sizeimage = 70000,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 3},
	{176, 144, V4L2_PIX_FMT_OV518, V4L2_FIELD_NONE,
		.bytesperline = 176,
		.sizeimage = 70000,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 1},
	{320, 240, V4L2_PIX_FMT_OV518, V4L2_FIELD_NONE,
		.bytesperline = 320,
		.sizeimage = 320 * 240 * 3,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 2},
	{352, 288, V4L2_PIX_FMT_OV518, V4L2_FIELD_NONE,
		.bytesperline = 352,
		.sizeimage = 352 * 288 * 3,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 0},
};

static const struct v4l2_pix_format ov511_vga_mode[] = {
	{320, 240, V4L2_PIX_FMT_OV511, V4L2_FIELD_NONE,
		.bytesperline = 320,
		.sizeimage = 320 * 240 * 3,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 1},
	{640, 480, V4L2_PIX_FMT_OV511, V4L2_FIELD_NONE,
		.bytesperline = 640,
		.sizeimage = 640 * 480 * 2,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 0},
};
static const struct v4l2_pix_format ov511_sif_mode[] = {
	{160, 120, V4L2_PIX_FMT_OV511, V4L2_FIELD_NONE,
		.bytesperline = 160,
		.sizeimage = 70000,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 3},
	{176, 144, V4L2_PIX_FMT_OV511, V4L2_FIELD_NONE,
		.bytesperline = 176,
		.sizeimage = 70000,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 1},
	{320, 240, V4L2_PIX_FMT_OV511, V4L2_FIELD_NONE,
		.bytesperline = 320,
		.sizeimage = 320 * 240 * 3,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 2},
	{352, 288, V4L2_PIX_FMT_OV511, V4L2_FIELD_NONE,
		.bytesperline = 352,
		.sizeimage = 352 * 288 * 3,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.priv = 0},
};


#define R51x_FIFO_PSIZE			0x30	
#define R51x_SYS_RESET          	0x50
	
	#define	OV511_RESET_OMNICE	0x08
#define R51x_SYS_INIT         		0x53
#define R51x_SYS_SNAP			0x52
#define R51x_SYS_CUST_ID		0x5F
#define R51x_COMP_LUT_BEGIN		0x80


#define R511_CAM_DELAY			0x10
#define R511_CAM_EDGE			0x11
#define R511_CAM_PXCNT			0x12
#define R511_CAM_LNCNT			0x13
#define R511_CAM_PXDIV			0x14
#define R511_CAM_LNDIV			0x15
#define R511_CAM_UV_EN			0x16
#define R511_CAM_LINE_MODE		0x17
#define R511_CAM_OPTS			0x18

#define R511_SNAP_FRAME			0x19
#define R511_SNAP_PXCNT			0x1A
#define R511_SNAP_LNCNT			0x1B
#define R511_SNAP_PXDIV			0x1C
#define R511_SNAP_LNDIV			0x1D
#define R511_SNAP_UV_EN			0x1E
#define R511_SNAP_UV_EN			0x1E
#define R511_SNAP_OPTS			0x1F

#define R511_DRAM_FLOW_CTL		0x20
#define R511_FIFO_OPTS			0x31
#define R511_I2C_CTL			0x40
#define R511_SYS_LED_CTL		0x55	
#define R511_COMP_EN			0x78
#define R511_COMP_LUT_EN		0x79


#define R518_GPIO_OUT			0x56	
#define R518_GPIO_CTL			0x57	


#define OV519_R10_H_SIZE		0x10
#define OV519_R11_V_SIZE		0x11
#define OV519_R12_X_OFFSETL		0x12
#define OV519_R13_X_OFFSETH		0x13
#define OV519_R14_Y_OFFSETL		0x14
#define OV519_R15_Y_OFFSETH		0x15
#define OV519_R16_DIVIDER		0x16
#define OV519_R20_DFR			0x20
#define OV519_R25_FORMAT		0x25


#define OV519_SYS_RESET1 0x51
#define OV519_SYS_EN_CLK1 0x54

#define OV519_GPIO_DATA_OUT0		0x71
#define OV519_GPIO_IO_CTRL0		0x72

#define OV511_ENDPOINT_ADDRESS  1	


#define R51x_I2C_W_SID		0x41
#define R51x_I2C_SADDR_3	0x42
#define R51x_I2C_SADDR_2	0x43
#define R51x_I2C_R_SID		0x44
#define R51x_I2C_DATA		0x45
#define R518_I2C_CTL		0x47	


#define OV7xx0_SID   0x42
#define OV8xx0_SID   0xa0
#define OV6xx0_SID   0xc0


#define OV7610_REG_GAIN		0x00	
#define OV7610_REG_BLUE		0x01	
#define OV7610_REG_RED		0x02	
#define OV7610_REG_SAT		0x03	
#define OV8610_REG_HUE		0x04	
#define OV7610_REG_CNT		0x05	
#define OV7610_REG_BRT		0x06	
#define OV7610_REG_COM_C	0x14	
#define OV7610_REG_ID_HIGH	0x1c	
#define OV7610_REG_ID_LOW	0x1d	
#define OV7610_REG_COM_I	0x29	


#define OV7670_REG_GAIN        0x00    
#define OV7670_REG_BLUE        0x01    
#define OV7670_REG_RED         0x02    
#define OV7670_REG_VREF        0x03    
#define OV7670_REG_COM1        0x04    
#define OV7670_REG_AECHH       0x07    
#define OV7670_REG_COM3        0x0c    
#define OV7670_REG_COM4        0x0d    
#define OV7670_REG_COM5        0x0e    
#define OV7670_REG_COM6        0x0f    
#define OV7670_REG_AECH        0x10    
#define OV7670_REG_CLKRC       0x11    
#define OV7670_REG_COM7        0x12    
#define   OV7670_COM7_FMT_VGA    0x00
#define   OV7670_COM7_YUV        0x00    
#define   OV7670_COM7_FMT_QVGA   0x10    
#define   OV7670_COM7_FMT_MASK   0x38
#define   OV7670_COM7_RESET      0x80    
#define OV7670_REG_COM8        0x13    
#define   OV7670_COM8_AEC        0x01    
#define   OV7670_COM8_AWB        0x02    
#define   OV7670_COM8_AGC        0x04    
#define   OV7670_COM8_BFILT      0x20    
#define   OV7670_COM8_AECSTEP    0x40    
#define   OV7670_COM8_FASTAEC    0x80    
#define OV7670_REG_COM9        0x14    
#define OV7670_REG_COM10       0x15    
#define OV7670_REG_HSTART      0x17    
#define OV7670_REG_HSTOP       0x18    
#define OV7670_REG_VSTART      0x19    
#define OV7670_REG_VSTOP       0x1a    
#define OV7670_REG_MVFP        0x1e    
#define   OV7670_MVFP_VFLIP	 0x10    
#define   OV7670_MVFP_MIRROR     0x20    
#define OV7670_REG_AEW         0x24    
#define OV7670_REG_AEB         0x25    
#define OV7670_REG_VPT         0x26    
#define OV7670_REG_HREF        0x32    
#define OV7670_REG_TSLB        0x3a    
#define OV7670_REG_COM11       0x3b    
#define   OV7670_COM11_EXP       0x02
#define   OV7670_COM11_HZAUTO    0x10    
#define OV7670_REG_COM12       0x3c    
#define OV7670_REG_COM13       0x3d    
#define   OV7670_COM13_GAMMA     0x80    
#define   OV7670_COM13_UVSAT     0x40    
#define OV7670_REG_COM14       0x3e    
#define OV7670_REG_EDGE        0x3f    
#define OV7670_REG_COM15       0x40    
#define   OV7670_COM15_R00FF     0xc0    
#define OV7670_REG_COM16       0x41    
#define   OV7670_COM16_AWBGAIN   0x08    
#define OV7670_REG_BRIGHT      0x55    
#define OV7670_REG_CONTRAS     0x56    
#define OV7670_REG_GFIX        0x69    
#define OV7670_REG_RGB444      0x8c    
#define OV7670_REG_HAECC1      0x9f    
#define OV7670_REG_HAECC2      0xa0    
#define OV7670_REG_BD50MAX     0xa5    
#define OV7670_REG_HAECC3      0xa6    
#define OV7670_REG_HAECC4      0xa7    
#define OV7670_REG_HAECC5      0xa8    
#define OV7670_REG_HAECC6      0xa9    
#define OV7670_REG_HAECC7      0xaa    
#define OV7670_REG_BD60MAX     0xab    

struct ov_regvals {
	__u8 reg;
	__u8 val;
};
struct ov_i2c_regvals {
	__u8 reg;
	__u8 val;
};

static const struct ov_i2c_regvals norm_6x20[] = {
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
	
	{ 0x16, 0x06 },

	{ 0x26, 0xb2 }, 
	
	{ 0x28, 0x05 },
	{ 0x2a, 0x04 }, 

	{ 0x2d, 0x85 },
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


};

static const struct ov_i2c_regvals norm_6x30[] = {
	{ 0x12, 0x80 }, 
	{ 0x00, 0x1f }, 
	{ 0x01, 0x99 }, 
	{ 0x02, 0x7c }, 
	{ 0x03, 0xc0 }, 
	{ 0x05, 0x0a }, 
	{ 0x06, 0x95 }, 
	{ 0x07, 0x2d }, 
	{ 0x0c, 0x20 },
	{ 0x0d, 0x20 },
	{ 0x0e, 0xa0 }, 
	{ 0x0f, 0x05 },
	{ 0x10, 0x9a },
	{ 0x11, 0x00 }, 
	{ 0x12, 0x24 }, 
	{ 0x13, 0x21 },
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
	{ 0x25, 0x9a }, 
	{ 0x26, 0xb2 }, 
	{ 0x27, 0xa2 },
	{ 0x28, 0x00 },
	{ 0x29, 0x00 },
	{ 0x2a, 0x84 }, 
	{ 0x2b, 0xa8 }, 
	{ 0x2c, 0xa0 },
	{ 0x2d, 0x95 }, 
	{ 0x2e, 0x88 },
	{ 0x33, 0x26 },
	{ 0x34, 0x03 },
	{ 0x36, 0x8f },
	{ 0x37, 0x80 },
	{ 0x38, 0x83 },
	{ 0x39, 0x80 },
	{ 0x3a, 0x0f },
	{ 0x3b, 0x3c },
	{ 0x3c, 0x1a },
	{ 0x3d, 0x80 },
	{ 0x3e, 0x80 },
	{ 0x3f, 0x0e },
	{ 0x40, 0x00 }, 
	{ 0x41, 0x00 }, 
	{ 0x42, 0x80 },
	{ 0x43, 0x3f }, 
	{ 0x44, 0x80 },
	{ 0x45, 0x20 },
	{ 0x46, 0x20 },
	{ 0x47, 0x80 },
	{ 0x48, 0x7f },
	{ 0x49, 0x00 },
	{ 0x4a, 0x00 },
	{ 0x4b, 0x80 },
	{ 0x4c, 0xd0 },
	{ 0x4d, 0x10 }, 
	{ 0x4e, 0x40 },
	{ 0x4f, 0x07 }, 
	{ 0x50, 0xff },
	{ 0x54, 0x23 }, 
	{ 0x55, 0xff },
	{ 0x56, 0x12 },
	{ 0x57, 0x81 },
	{ 0x58, 0x75 },
	{ 0x59, 0x01 }, 
	{ 0x5a, 0x2c },
	{ 0x5b, 0x0f }, 
	{ 0x5c, 0x10 },
	{ 0x3d, 0x80 },
	{ 0x27, 0xa6 },
	{ 0x12, 0x20 }, 
	{ 0x12, 0x24 },
};


static const struct ov_i2c_regvals norm_7610[] = {
	{ 0x10, 0xff },
	{ 0x16, 0x06 },
	{ 0x28, 0x24 },
	{ 0x2b, 0xac },
	{ 0x12, 0x00 },
	{ 0x38, 0x81 },
	{ 0x28, 0x24 },	
	{ 0x0f, 0x85 },	
	{ 0x15, 0x01 },
	{ 0x20, 0x1c },
	{ 0x23, 0x2a },
	{ 0x24, 0x10 },
	{ 0x25, 0x8a },
	{ 0x26, 0xa2 },
	{ 0x27, 0xc2 },
	{ 0x2a, 0x04 },
	{ 0x2c, 0xfe },
	{ 0x2d, 0x93 },
	{ 0x30, 0x71 },
	{ 0x31, 0x60 },
	{ 0x32, 0x26 },
	{ 0x33, 0x20 },
	{ 0x34, 0x48 },
	{ 0x12, 0x24 },
	{ 0x11, 0x01 },
	{ 0x0c, 0x24 },
	{ 0x0d, 0x24 },
};

static const struct ov_i2c_regvals norm_7620[] = {
	{ 0x00, 0x00 },		
	{ 0x01, 0x80 },		
	{ 0x02, 0x80 },		
	{ 0x03, 0xc0 },		
	{ 0x06, 0x60 },
	{ 0x07, 0x00 },
	{ 0x0c, 0x24 },
	{ 0x0c, 0x24 },
	{ 0x0d, 0x24 },
	{ 0x11, 0x01 },
	{ 0x12, 0x24 },
	{ 0x13, 0x01 },
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
	{ 0x28, 0x22 }, 
	{ 0x29, 0x00 },
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
};


static const struct ov_i2c_regvals norm_7640[] = {
	{ 0x12, 0x80 },
	{ 0x12, 0x14 },
};


static const struct ov_i2c_regvals norm_7670[] = {
	{ OV7670_REG_COM7, OV7670_COM7_RESET },
	{ OV7670_REG_TSLB, 0x04 },		
	{ OV7670_REG_COM7, OV7670_COM7_FMT_VGA }, 
	{ OV7670_REG_CLKRC, 0x01 },

	{ OV7670_REG_HSTART, 0x13 },
	{ OV7670_REG_HSTOP, 0x01 },
	{ OV7670_REG_HREF, 0xb6 },
	{ OV7670_REG_VSTART, 0x02 },
	{ OV7670_REG_VSTOP, 0x7a },
	{ OV7670_REG_VREF, 0x0a },

	{ OV7670_REG_COM3, 0x00 },
	{ OV7670_REG_COM14, 0x00 },

	{ 0x70, 0x3a },
	{ 0x71, 0x35 },
	{ 0x72, 0x11 },
	{ 0x73, 0xf0 },
	{ 0xa2, 0x02 },



	{ 0x7a, 0x20 },
	{ 0x7b, 0x10 },
	{ 0x7c, 0x1e },
	{ 0x7d, 0x35 },
	{ 0x7e, 0x5a },
	{ 0x7f, 0x69 },
	{ 0x80, 0x76 },
	{ 0x81, 0x80 },
	{ 0x82, 0x88 },
	{ 0x83, 0x8f },
	{ 0x84, 0x96 },
	{ 0x85, 0xa3 },
	{ 0x86, 0xaf },
	{ 0x87, 0xc4 },
	{ 0x88, 0xd7 },
	{ 0x89, 0xe8 },


	{ OV7670_REG_COM8, OV7670_COM8_FASTAEC
			 | OV7670_COM8_AECSTEP
			 | OV7670_COM8_BFILT },
	{ OV7670_REG_GAIN, 0x00 },
	{ OV7670_REG_AECH, 0x00 },
	{ OV7670_REG_COM4, 0x40 }, 
	{ OV7670_REG_COM9, 0x18 }, 
	{ OV7670_REG_BD50MAX, 0x05 },
	{ OV7670_REG_BD60MAX, 0x07 },
	{ OV7670_REG_AEW, 0x95 },
	{ OV7670_REG_AEB, 0x33 },
	{ OV7670_REG_VPT, 0xe3 },
	{ OV7670_REG_HAECC1, 0x78 },
	{ OV7670_REG_HAECC2, 0x68 },
	{ 0xa1, 0x03 }, 
	{ OV7670_REG_HAECC3, 0xd8 },
	{ OV7670_REG_HAECC4, 0xd8 },
	{ OV7670_REG_HAECC5, 0xf0 },
	{ OV7670_REG_HAECC6, 0x90 },
	{ OV7670_REG_HAECC7, 0x94 },
	{ OV7670_REG_COM8, OV7670_COM8_FASTAEC
			| OV7670_COM8_AECSTEP
			| OV7670_COM8_BFILT
			| OV7670_COM8_AGC
			| OV7670_COM8_AEC },


	{ OV7670_REG_COM5, 0x61 },
	{ OV7670_REG_COM6, 0x4b },
	{ 0x16, 0x02 },
	{ OV7670_REG_MVFP, 0x07 },
	{ 0x21, 0x02 },
	{ 0x22, 0x91 },
	{ 0x29, 0x07 },
	{ 0x33, 0x0b },
	{ 0x35, 0x0b },
	{ 0x37, 0x1d },
	{ 0x38, 0x71 },
	{ 0x39, 0x2a },
	{ OV7670_REG_COM12, 0x78 },
	{ 0x4d, 0x40 },
	{ 0x4e, 0x20 },
	{ OV7670_REG_GFIX, 0x00 },
	{ 0x6b, 0x4a },
	{ 0x74, 0x10 },
	{ 0x8d, 0x4f },
	{ 0x8e, 0x00 },
	{ 0x8f, 0x00 },
	{ 0x90, 0x00 },
	{ 0x91, 0x00 },
	{ 0x96, 0x00 },
	{ 0x9a, 0x00 },
	{ 0xb0, 0x84 },
	{ 0xb1, 0x0c },
	{ 0xb2, 0x0e },
	{ 0xb3, 0x82 },
	{ 0xb8, 0x0a },


	{ 0x43, 0x0a },
	{ 0x44, 0xf0 },
	{ 0x45, 0x34 },
	{ 0x46, 0x58 },
	{ 0x47, 0x28 },
	{ 0x48, 0x3a },
	{ 0x59, 0x88 },
	{ 0x5a, 0x88 },
	{ 0x5b, 0x44 },
	{ 0x5c, 0x67 },
	{ 0x5d, 0x49 },
	{ 0x5e, 0x0e },
	{ 0x6c, 0x0a },
	{ 0x6d, 0x55 },
	{ 0x6e, 0x11 },
	{ 0x6f, 0x9f },
					
	{ 0x6a, 0x40 },
	{ OV7670_REG_BLUE, 0x40 },
	{ OV7670_REG_RED, 0x60 },
	{ OV7670_REG_COM8, OV7670_COM8_FASTAEC
			| OV7670_COM8_AECSTEP
			| OV7670_COM8_BFILT
			| OV7670_COM8_AGC
			| OV7670_COM8_AEC
			| OV7670_COM8_AWB },


	{ 0x4f, 0x80 },
	{ 0x50, 0x80 },
	{ 0x51, 0x00 },
	{ 0x52, 0x22 },
	{ 0x53, 0x5e },
	{ 0x54, 0x80 },
	{ 0x58, 0x9e },

	{ OV7670_REG_COM16, OV7670_COM16_AWBGAIN },
	{ OV7670_REG_EDGE, 0x00 },
	{ 0x75, 0x05 },
	{ 0x76, 0xe1 },
	{ 0x4c, 0x00 },
	{ 0x77, 0x01 },
	{ OV7670_REG_COM13, OV7670_COM13_GAMMA
			  | OV7670_COM13_UVSAT
			  | 2},		
	{ 0x4b, 0x09 },
	{ 0xc9, 0x60 },
	{ OV7670_REG_COM16, 0x38 },
	{ 0x56, 0x40 },

	{ 0x34, 0x11 },
	{ OV7670_REG_COM11, OV7670_COM11_EXP|OV7670_COM11_HZAUTO },
	{ 0xa4, 0x88 },
	{ 0x96, 0x00 },
	{ 0x97, 0x30 },
	{ 0x98, 0x20 },
	{ 0x99, 0x30 },
	{ 0x9a, 0x84 },
	{ 0x9b, 0x29 },
	{ 0x9c, 0x03 },
	{ 0x9d, 0x4c },
	{ 0x9e, 0x3f },
	{ 0x78, 0x04 },


	{ 0x79, 0x01 },
	{ 0xc8, 0xf0 },
	{ 0x79, 0x0f },
	{ 0xc8, 0x00 },
	{ 0x79, 0x10 },
	{ 0xc8, 0x7e },
	{ 0x79, 0x0a },
	{ 0xc8, 0x80 },
	{ 0x79, 0x0b },
	{ 0xc8, 0x01 },
	{ 0x79, 0x0c },
	{ 0xc8, 0x0f },
	{ 0x79, 0x0d },
	{ 0xc8, 0x20 },
	{ 0x79, 0x09 },
	{ 0xc8, 0x80 },
	{ 0x79, 0x02 },
	{ 0xc8, 0xc0 },
	{ 0x79, 0x03 },
	{ 0xc8, 0x40 },
	{ 0x79, 0x05 },
	{ 0xc8, 0x30 },
	{ 0x79, 0x26 },
};

static const struct ov_i2c_regvals norm_8610[] = {
	{ 0x12, 0x80 },
	{ 0x00, 0x00 },
	{ 0x01, 0x80 },
	{ 0x02, 0x80 },
	{ 0x03, 0xc0 },
	{ 0x04, 0x30 },
	{ 0x05, 0x30 }, 
	{ 0x06, 0x70 }, 
	{ 0x0a, 0x86 },
	{ 0x0b, 0xb0 },
	{ 0x0c, 0x20 },
	{ 0x0d, 0x20 },
	{ 0x11, 0x01 },
	{ 0x12, 0x25 },
	{ 0x13, 0x01 },
	{ 0x14, 0x04 },
	{ 0x15, 0x01 }, 
	{ 0x16, 0x03 },
	{ 0x17, 0x38 }, 
	{ 0x18, 0xea }, 
	{ 0x19, 0x02 }, 
	{ 0x1a, 0xf5 },
	{ 0x1b, 0x00 },
	{ 0x20, 0xd0 }, 
	{ 0x23, 0xc0 }, 
	{ 0x24, 0x30 }, 
	{ 0x25, 0x50 }, 
	{ 0x26, 0xa2 },
	{ 0x27, 0xea },
	{ 0x28, 0x00 },
	{ 0x29, 0x00 },
	{ 0x2a, 0x80 },
	{ 0x2b, 0xc8 }, 
	{ 0x2c, 0xac },
	{ 0x2d, 0x45 }, 
	{ 0x2e, 0x80 },
	{ 0x2f, 0x14 }, 
	{ 0x4c, 0x00 },
	{ 0x4d, 0x30 }, 
	{ 0x60, 0x02 }, 
	{ 0x61, 0x00 }, 
	{ 0x62, 0x5f }, 
	{ 0x63, 0xff },
	{ 0x64, 0x53 }, 
	{ 0x65, 0x00 },
	{ 0x66, 0x55 },
	{ 0x67, 0xb0 },
	{ 0x68, 0xc0 }, 
	{ 0x69, 0x02 },
	{ 0x6a, 0x22 },
	{ 0x6b, 0x00 },
	{ 0x6c, 0x99 }, 
	{ 0x6d, 0x11 }, 
	{ 0x6e, 0x11 }, 
	{ 0x6f, 0x01 },
	{ 0x70, 0x8b },
	{ 0x71, 0x00 },
	{ 0x72, 0x14 },
	{ 0x73, 0x54 },
	{ 0x74, 0x00 },
	{ 0x75, 0x0e },
	{ 0x76, 0x02 }, 
	{ 0x77, 0xff },
	{ 0x78, 0x80 },
	{ 0x79, 0x80 },
	{ 0x7a, 0x80 },
	{ 0x7b, 0x10 }, 
	{ 0x7c, 0x00 },
	{ 0x7d, 0x08 }, 
	{ 0x7e, 0x08 }, 
	{ 0x7f, 0xfb },
	{ 0x80, 0x28 },
	{ 0x81, 0x00 },
	{ 0x82, 0x23 },
	{ 0x83, 0x0b },
	{ 0x84, 0x00 },
	{ 0x85, 0x62 }, 
	{ 0x86, 0xc9 },
	{ 0x87, 0x00 },
	{ 0x88, 0x00 },
	{ 0x89, 0x01 },
	{ 0x12, 0x20 },
	{ 0x12, 0x25 }, 
};

static unsigned char ov7670_abs_to_sm(unsigned char v)
{
	if (v > 127)
		return v & 0x7f;
	return (128 - v) | 0x80;
}


static int reg_w(struct sd *sd, __u16 index, __u8 value)
{
	int ret;
	int req = (sd->bridge <= BRIDGE_OV511PLUS) ? 2 : 1;

	sd->gspca_dev.usb_buf[0] = value;
	ret = usb_control_msg(sd->gspca_dev.dev,
			usb_sndctrlpipe(sd->gspca_dev.dev, 0),
			req,
			USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			0, index,
			sd->gspca_dev.usb_buf, 1, 500);
	if (ret < 0)
		PDEBUG(D_ERR, "Write reg [%02x] %02x failed", index, value);
	return ret;
}



static int reg_r(struct sd *sd, __u16 index)
{
	int ret;
	int req = (sd->bridge <= BRIDGE_OV511PLUS) ? 3 : 1;

	ret = usb_control_msg(sd->gspca_dev.dev,
			usb_rcvctrlpipe(sd->gspca_dev.dev, 0),
			req,
			USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			0, index, sd->gspca_dev.usb_buf, 1, 500);

	if (ret >= 0)
		ret = sd->gspca_dev.usb_buf[0];
	else
		PDEBUG(D_ERR, "Read reg [0x%02x] failed", index);
	return ret;
}


static int reg_r8(struct sd *sd,
		  __u16 index)
{
	int ret;

	ret = usb_control_msg(sd->gspca_dev.dev,
			usb_rcvctrlpipe(sd->gspca_dev.dev, 0),
			1,			
			USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			0, index, sd->gspca_dev.usb_buf, 8, 500);

	if (ret >= 0)
		ret = sd->gspca_dev.usb_buf[0];
	else
		PDEBUG(D_ERR, "Read reg 8 [0x%02x] failed", index);
	return ret;
}


static int reg_w_mask(struct sd *sd,
			__u16 index,
			__u8 value,
			__u8 mask)
{
	int ret;
	__u8 oldval;

	if (mask != 0xff) {
		value &= mask;			
		ret = reg_r(sd, index);
		if (ret < 0)
			return ret;

		oldval = ret & ~mask;		
		value |= oldval;		
	}
	return reg_w(sd, index, value);
}


static int ov518_reg_w32(struct sd *sd, __u16 index, u32 value, int n)
{
	int ret;

	*((u32 *)sd->gspca_dev.usb_buf) = __cpu_to_le32(value);

	ret = usb_control_msg(sd->gspca_dev.dev,
			usb_sndctrlpipe(sd->gspca_dev.dev, 0),
			1 ,
			USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			0, index,
			sd->gspca_dev.usb_buf, n, 500);
	if (ret < 0)
		PDEBUG(D_ERR, "Write reg32 [%02x] %08x failed", index, value);
	return ret;
}

static int ov511_i2c_w(struct sd *sd, __u8 reg, __u8 value)
{
	int rc, retries;

	PDEBUG(D_USBO, "i2c 0x%02x -> [0x%02x]", value, reg);

	
	for (retries = 6; ; ) {
		
		rc = reg_w(sd, R51x_I2C_SADDR_3, reg);
		if (rc < 0)
			return rc;

		
		rc = reg_w(sd, R51x_I2C_DATA, value);
		if (rc < 0)
			return rc;

		
		rc = reg_w(sd, R511_I2C_CTL, 0x01);
		if (rc < 0)
			return rc;

		do
			rc = reg_r(sd, R511_I2C_CTL);
		while (rc > 0 && ((rc & 1) == 0)); 

		if (rc < 0)
			return rc;

		if ((rc & 2) == 0) 
			break;
		if (--retries < 0) {
			PDEBUG(D_USBO, "i2c write retries exhausted");
			return -1;
		}
	}

	return 0;
}

static int ov511_i2c_r(struct sd *sd, __u8 reg)
{
	int rc, value, retries;

	
	for (retries = 6; ; ) {
		
		rc = reg_w(sd, R51x_I2C_SADDR_2, reg);
		if (rc < 0)
			return rc;

		
		rc = reg_w(sd, R511_I2C_CTL, 0x03);
		if (rc < 0)
			return rc;

		do
			rc = reg_r(sd, R511_I2C_CTL);
		while (rc > 0 && ((rc & 1) == 0)); 

		if (rc < 0)
			return rc;

		if ((rc & 2) == 0) 
			break;

		
		reg_w(sd, R511_I2C_CTL, 0x10);

		if (--retries < 0) {
			PDEBUG(D_USBI, "i2c write retries exhausted");
			return -1;
		}
	}

	
	for (retries = 6; ; ) {
		
		rc = reg_w(sd, R511_I2C_CTL, 0x05);
		if (rc < 0)
			return rc;

		do
			rc = reg_r(sd, R511_I2C_CTL);
		while (rc > 0 && ((rc & 1) == 0)); 

		if (rc < 0)
			return rc;

		if ((rc & 2) == 0) 
			break;

		
		rc = reg_w(sd, R511_I2C_CTL, 0x10);
		if (rc < 0)
			return rc;

		if (--retries < 0) {
			PDEBUG(D_USBI, "i2c read retries exhausted");
			return -1;
		}
	}

	value = reg_r(sd, R51x_I2C_DATA);

	PDEBUG(D_USBI, "i2c [0x%02X] -> 0x%02X", reg, value);

	
	rc = reg_w(sd, R511_I2C_CTL, 0x05);
	if (rc < 0)
		return rc;

	return value;
}


static int ov518_i2c_w(struct sd *sd,
		__u8 reg,
		__u8 value)
{
	int rc;

	PDEBUG(D_USBO, "i2c 0x%02x -> [0x%02x]", value, reg);

	
	rc = reg_w(sd, R51x_I2C_SADDR_3, reg);
	if (rc < 0)
		return rc;

	
	rc = reg_w(sd, R51x_I2C_DATA, value);
	if (rc < 0)
		return rc;

	
	rc = reg_w(sd, R518_I2C_CTL, 0x01);
	if (rc < 0)
		return rc;

	
	msleep(4);
	return reg_r8(sd, R518_I2C_CTL);
}


static int ov518_i2c_r(struct sd *sd, __u8 reg)
{
	int rc, value;

	
	rc = reg_w(sd, R51x_I2C_SADDR_2, reg);
	if (rc < 0)
		return rc;

	
	rc = reg_w(sd, R518_I2C_CTL, 0x03);
	if (rc < 0)
		return rc;

	
	rc = reg_w(sd, R518_I2C_CTL, 0x05);
	if (rc < 0)
		return rc;
	value = reg_r(sd, R51x_I2C_DATA);
	PDEBUG(D_USBI, "i2c [0x%02X] -> 0x%02X", reg, value);
	return value;
}

static int i2c_w(struct sd *sd, __u8 reg, __u8 value)
{
	switch (sd->bridge) {
	case BRIDGE_OV511:
	case BRIDGE_OV511PLUS:
		return ov511_i2c_w(sd, reg, value);
	case BRIDGE_OV518:
	case BRIDGE_OV518PLUS:
	case BRIDGE_OV519:
		return ov518_i2c_w(sd, reg, value);
	}
	return -1; 
}

static int i2c_r(struct sd *sd, __u8 reg)
{
	switch (sd->bridge) {
	case BRIDGE_OV511:
	case BRIDGE_OV511PLUS:
		return ov511_i2c_r(sd, reg);
	case BRIDGE_OV518:
	case BRIDGE_OV518PLUS:
	case BRIDGE_OV519:
		return ov518_i2c_r(sd, reg);
	}
	return -1; 
}


static int i2c_w_mask(struct sd *sd,
		   __u8 reg,
		   __u8 value,
		   __u8 mask)
{
	int rc;
	__u8 oldval;

	value &= mask;			
	rc = i2c_r(sd, reg);
	if (rc < 0)
		return rc;
	oldval = rc & ~mask;		
	value |= oldval;		
	return i2c_w(sd, reg, value);
}


static inline int ov51x_stop(struct sd *sd)
{
	PDEBUG(D_STREAM, "stopping");
	sd->stopped = 1;
	switch (sd->bridge) {
	case BRIDGE_OV511:
	case BRIDGE_OV511PLUS:
		return reg_w(sd, R51x_SYS_RESET, 0x3d);
	case BRIDGE_OV518:
	case BRIDGE_OV518PLUS:
		return reg_w_mask(sd, R51x_SYS_RESET, 0x3a, 0x3a);
	case BRIDGE_OV519:
		return reg_w(sd, OV519_SYS_RESET1, 0x0f);
	}

	return 0;
}


static inline int ov51x_restart(struct sd *sd)
{
	int rc;

	PDEBUG(D_STREAM, "restarting");
	if (!sd->stopped)
		return 0;
	sd->stopped = 0;

	
	switch (sd->bridge) {
	case BRIDGE_OV511:
	case BRIDGE_OV511PLUS:
		return reg_w(sd, R51x_SYS_RESET, 0x00);
	case BRIDGE_OV518:
	case BRIDGE_OV518PLUS:
		rc = reg_w(sd, 0x2f, 0x80);
		if (rc < 0)
			return rc;
		return reg_w(sd, R51x_SYS_RESET, 0x00);
	case BRIDGE_OV519:
		return reg_w(sd, OV519_SYS_RESET1, 0x00);
	}

	return 0;
}


static int init_ov_sensor(struct sd *sd)
{
	int i;

	
	if (i2c_w(sd, 0x12, 0x80) < 0)
		return -EIO;

	
	msleep(150);

	for (i = 0; i < i2c_detect_tries; i++) {
		if (i2c_r(sd, OV7610_REG_ID_HIGH) == 0x7f &&
		    i2c_r(sd, OV7610_REG_ID_LOW) == 0xa2) {
			PDEBUG(D_PROBE, "I2C synced in %d attempt(s)", i);
			return 0;
		}

		
		if (i2c_w(sd, 0x12, 0x80) < 0)
			return -EIO;
		
		msleep(150);
		
		if (i2c_r(sd, 0x00) < 0)
			return -EIO;
	}
	return -EIO;
}


static int ov51x_set_slave_ids(struct sd *sd,
				__u8 slave)
{
	int rc;

	rc = reg_w(sd, R51x_I2C_W_SID, slave);
	if (rc < 0)
		return rc;
	return reg_w(sd, R51x_I2C_R_SID, slave + 1);
}

static int write_regvals(struct sd *sd,
			 const struct ov_regvals *regvals,
			 int n)
{
	int rc;

	while (--n >= 0) {
		rc = reg_w(sd, regvals->reg, regvals->val);
		if (rc < 0)
			return rc;
		regvals++;
	}
	return 0;
}

static int write_i2c_regvals(struct sd *sd,
			     const struct ov_i2c_regvals *regvals,
			     int n)
{
	int rc;

	while (--n >= 0) {
		rc = i2c_w(sd, regvals->reg, regvals->val);
		if (rc < 0)
			return rc;
		regvals++;
	}
	return 0;
}




static int ov8xx0_configure(struct sd *sd)
{
	int rc;

	PDEBUG(D_PROBE, "starting ov8xx0 configuration");

	
	rc = i2c_r(sd, OV7610_REG_COM_I);
	if (rc < 0) {
		PDEBUG(D_ERR, "Error detecting sensor type");
		return -1;
	}
	if ((rc & 3) == 1) {
		sd->sensor = SEN_OV8610;
	} else {
		PDEBUG(D_ERR, "Unknown image sensor version: %d", rc & 3);
		return -1;
	}

	
	return 0;
}


static int ov7xx0_configure(struct sd *sd)
{
	int rc, high, low;


	PDEBUG(D_PROBE, "starting OV7xx0 configuration");

	
	rc = i2c_r(sd, OV7610_REG_COM_I);

	
	if (rc < 0) {
		PDEBUG(D_ERR, "Error detecting sensor type");
		return -1;
	}
	if ((rc & 3) == 3) {
		
		high = i2c_r(sd, 0x0a);
		low = i2c_r(sd, 0x0b);
		
		if (high == 0x76 && low == 0x73) {
			PDEBUG(D_PROBE, "Sensor is an OV7670");
			sd->sensor = SEN_OV7670;
		} else {
			PDEBUG(D_PROBE, "Sensor is an OV7610");
			sd->sensor = SEN_OV7610;
		}
	} else if ((rc & 3) == 1) {
		
		if (i2c_r(sd, 0x15) & 1) {
			PDEBUG(D_PROBE, "Sensor is an OV7620AE");
			sd->sensor = SEN_OV7620;
		} else {
			PDEBUG(D_PROBE, "Sensor is an OV76BE");
			sd->sensor = SEN_OV76BE;
		}
	} else if ((rc & 3) == 0) {
		
		high = i2c_r(sd, 0x0a);
		if (high < 0) {
			PDEBUG(D_ERR, "Error detecting camera chip PID");
			return high;
		}
		low = i2c_r(sd, 0x0b);
		if (low < 0) {
			PDEBUG(D_ERR, "Error detecting camera chip VER");
			return low;
		}
		if (high == 0x76) {
			switch (low) {
			case 0x30:
				PDEBUG(D_PROBE, "Sensor is an OV7630/OV7635");
				PDEBUG(D_ERR,
				      "7630 is not supported by this driver");
				return -1;
			case 0x40:
				PDEBUG(D_PROBE, "Sensor is an OV7645");
				sd->sensor = SEN_OV7640; 
				break;
			case 0x45:
				PDEBUG(D_PROBE, "Sensor is an OV7645B");
				sd->sensor = SEN_OV7640; 
				break;
			case 0x48:
				PDEBUG(D_PROBE, "Sensor is an OV7648");
				sd->sensor = SEN_OV7640; 
				break;
			default:
				PDEBUG(D_PROBE, "Unknown sensor: 0x76%x", low);
				return -1;
			}
		} else {
			PDEBUG(D_PROBE, "Sensor is an OV7620");
			sd->sensor = SEN_OV7620;
		}
	} else {
		PDEBUG(D_ERR, "Unknown image sensor version: %d", rc & 3);
		return -1;
	}

	
	return 0;
}


static int ov6xx0_configure(struct sd *sd)
{
	int rc;
	PDEBUG(D_PROBE, "starting OV6xx0 configuration");

	
	rc = i2c_r(sd, OV7610_REG_COM_I);
	if (rc < 0) {
		PDEBUG(D_ERR, "Error detecting sensor type");
		return -1;
	}

	
	switch (rc) {
	case 0x00:
		sd->sensor = SEN_OV6630;
		PDEBUG(D_ERR,
			"WARNING: Sensor is an OV66308. Your camera may have");
		PDEBUG(D_ERR, "been misdetected in previous driver versions.");
		break;
	case 0x01:
		sd->sensor = SEN_OV6620;
		PDEBUG(D_PROBE, "Sensor is an OV6620");
		break;
	case 0x02:
		sd->sensor = SEN_OV6630;
		PDEBUG(D_PROBE, "Sensor is an OV66308AE");
		break;
	case 0x03:
		sd->sensor = SEN_OV66308AF;
		PDEBUG(D_PROBE, "Sensor is an OV66308AF");
		break;
	case 0x90:
		sd->sensor = SEN_OV6630;
		PDEBUG(D_ERR,
			"WARNING: Sensor is an OV66307. Your camera may have");
		PDEBUG(D_ERR, "been misdetected in previous driver versions.");
		break;
	default:
		PDEBUG(D_ERR, "FATAL: Unknown sensor version: 0x%02x", rc);
		return -1;
	}

	
	sd->sif = 1;

	return 0;
}


static void ov51x_led_control(struct sd *sd, int on)
{
	if (sd->invert_led)
		on = !on;

	switch (sd->bridge) {
	
	case BRIDGE_OV511PLUS:
		reg_w(sd, R511_SYS_LED_CTL, on ? 1 : 0);
		break;
	case BRIDGE_OV518:
	case BRIDGE_OV518PLUS:
		reg_w_mask(sd, R518_GPIO_OUT, on ? 0x02 : 0x00, 0x02);
		break;
	case BRIDGE_OV519:
		reg_w_mask(sd, OV519_GPIO_DATA_OUT0, !on, 1);	
		break;
	}
}

static int ov51x_upload_quan_tables(struct sd *sd)
{
	const unsigned char yQuanTable511[] = {
		0, 1, 1, 2, 2, 3, 3, 4,
		1, 1, 1, 2, 2, 3, 4, 4,
		1, 1, 2, 2, 3, 4, 4, 4,
		2, 2, 2, 3, 4, 4, 4, 4,
		2, 2, 3, 4, 4, 5, 5, 5,
		3, 3, 4, 4, 5, 5, 5, 5,
		3, 4, 4, 4, 5, 5, 5, 5,
		4, 4, 4, 4, 5, 5, 5, 5
	};

	const unsigned char uvQuanTable511[] = {
		0, 2, 2, 3, 4, 4, 4, 4,
		2, 2, 2, 4, 4, 4, 4, 4,
		2, 2, 3, 4, 4, 4, 4, 4,
		3, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4
	};

	
	const unsigned char yQuanTable518[] = {
		5, 4, 5, 6, 6, 7, 7, 7,
		5, 5, 5, 5, 6, 7, 7, 7,
		6, 6, 6, 6, 7, 7, 7, 8,
		7, 7, 6, 7, 7, 7, 8, 8
	};

	const unsigned char uvQuanTable518[] = {
		6, 6, 6, 7, 7, 7, 7, 7,
		6, 6, 6, 7, 7, 7, 7, 7,
		6, 6, 6, 7, 7, 7, 7, 8,
		7, 7, 7, 7, 7, 7, 8, 8
	};

	const unsigned char *pYTable, *pUVTable;
	unsigned char val0, val1;
	int i, size, rc, reg = R51x_COMP_LUT_BEGIN;

	PDEBUG(D_PROBE, "Uploading quantization tables");

	if (sd->bridge == BRIDGE_OV511 || sd->bridge == BRIDGE_OV511PLUS) {
		pYTable = yQuanTable511;
		pUVTable = uvQuanTable511;
		size  = 32;
	} else {
		pYTable = yQuanTable518;
		pUVTable = uvQuanTable518;
		size  = 16;
	}

	for (i = 0; i < size; i++) {
		val0 = *pYTable++;
		val1 = *pYTable++;
		val0 &= 0x0f;
		val1 &= 0x0f;
		val0 |= val1 << 4;
		rc = reg_w(sd, reg, val0);
		if (rc < 0)
			return rc;

		val0 = *pUVTable++;
		val1 = *pUVTable++;
		val0 &= 0x0f;
		val1 &= 0x0f;
		val0 |= val1 << 4;
		rc = reg_w(sd, reg + size, val0);
		if (rc < 0)
			return rc;

		reg++;
	}

	return 0;
}


static int ov511_configure(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;
	int rc;

	
	const struct ov_regvals init_511[] = {
		{ R51x_SYS_RESET,	0x7f },
		{ R51x_SYS_INIT,	0x01 },
		{ R51x_SYS_RESET,	0x7f },
		{ R51x_SYS_INIT,	0x01 },
		{ R51x_SYS_RESET,	0x3f },
		{ R51x_SYS_INIT,	0x01 },
		{ R51x_SYS_RESET,	0x3d },
	};

	const struct ov_regvals norm_511[] = {
		{ R511_DRAM_FLOW_CTL, 	0x01 },
		{ R51x_SYS_SNAP,	0x00 },
		{ R51x_SYS_SNAP,	0x02 },
		{ R51x_SYS_SNAP,	0x00 },
		{ R511_FIFO_OPTS,	0x1f },
		{ R511_COMP_EN,		0x00 },
		{ R511_COMP_LUT_EN,	0x03 },
	};

	const struct ov_regvals norm_511_p[] = {
		{ R511_DRAM_FLOW_CTL,	0xff },
		{ R51x_SYS_SNAP,	0x00 },
		{ R51x_SYS_SNAP,	0x02 },
		{ R51x_SYS_SNAP,	0x00 },
		{ R511_FIFO_OPTS,	0xff },
		{ R511_COMP_EN,		0x00 },
		{ R511_COMP_LUT_EN,	0x03 },
	};

	const struct ov_regvals compress_511[] = {
		{ 0x70, 0x1f },
		{ 0x71, 0x05 },
		{ 0x72, 0x06 },
		{ 0x73, 0x06 },
		{ 0x74, 0x14 },
		{ 0x75, 0x03 },
		{ 0x76, 0x04 },
		{ 0x77, 0x04 },
	};

	PDEBUG(D_PROBE, "Device custom id %x", reg_r(sd, R51x_SYS_CUST_ID));

	rc = write_regvals(sd, init_511, ARRAY_SIZE(init_511));
	if (rc < 0)
		return rc;

	switch (sd->bridge) {
	case BRIDGE_OV511:
		rc = write_regvals(sd, norm_511, ARRAY_SIZE(norm_511));
		if (rc < 0)
			return rc;
		break;
	case BRIDGE_OV511PLUS:
		rc = write_regvals(sd, norm_511_p, ARRAY_SIZE(norm_511_p));
		if (rc < 0)
			return rc;
		break;
	}

	
	rc = write_regvals(sd, compress_511, ARRAY_SIZE(compress_511));
	if (rc < 0)
		return rc;

	rc = ov51x_upload_quan_tables(sd);
	if (rc < 0) {
		PDEBUG(D_ERR, "Error uploading quantization tables");
		return rc;
	}

	return 0;
}


static int ov518_configure(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;
	int rc;

	
	const struct ov_regvals init_518[] = {
		{ R51x_SYS_RESET,	0x40 },
		{ R51x_SYS_INIT,	0xe1 },
		{ R51x_SYS_RESET,	0x3e },
		{ R51x_SYS_INIT,	0xe1 },
		{ R51x_SYS_RESET,	0x00 },
		{ R51x_SYS_INIT,	0xe1 },
		{ 0x46,			0x00 },
		{ 0x5d,			0x03 },
	};

	const struct ov_regvals norm_518[] = {
		{ R51x_SYS_SNAP,	0x02 }, 
		{ R51x_SYS_SNAP,	0x01 }, 
		{ 0x31, 		0x0f },
		{ 0x5d,			0x03 },
		{ 0x24,			0x9f },
		{ 0x25,			0x90 },
		{ 0x20,			0x00 },
		{ 0x51,			0x04 },
		{ 0x71,			0x19 },
		{ 0x2f,			0x80 },
	};

	const struct ov_regvals norm_518_p[] = {
		{ R51x_SYS_SNAP,	0x02 }, 
		{ R51x_SYS_SNAP,	0x01 }, 
		{ 0x31, 		0x0f },
		{ 0x5d,			0x03 },
		{ 0x24,			0x9f },
		{ 0x25,			0x90 },
		{ 0x20,			0x60 },
		{ 0x51,			0x02 },
		{ 0x71,			0x19 },
		{ 0x40,			0xff },
		{ 0x41,			0x42 },
		{ 0x46,			0x00 },
		{ 0x33,			0x04 },
		{ 0x21,			0x19 },
		{ 0x3f,			0x10 },
		{ 0x2f,			0x80 },
	};

	
	PDEBUG(D_PROBE, "Device revision %d",
	       0x1F & reg_r(sd, R51x_SYS_CUST_ID));

	rc = write_regvals(sd, init_518, ARRAY_SIZE(init_518));
	if (rc < 0)
		return rc;

	
	rc = reg_w_mask(sd, R518_GPIO_CTL, 0x00, 0x02);
	if (rc < 0)
		return rc;

	switch (sd->bridge) {
	case BRIDGE_OV518:
		rc = write_regvals(sd, norm_518, ARRAY_SIZE(norm_518));
		if (rc < 0)
			return rc;
		break;
	case BRIDGE_OV518PLUS:
		rc = write_regvals(sd, norm_518_p, ARRAY_SIZE(norm_518_p));
		if (rc < 0)
			return rc;
		break;
	}

	rc = ov51x_upload_quan_tables(sd);
	if (rc < 0) {
		PDEBUG(D_ERR, "Error uploading quantization tables");
		return rc;
	}

	rc = reg_w(sd, 0x2f, 0x80);
	if (rc < 0)
		return rc;

	return 0;
}

static int ov519_configure(struct sd *sd)
{
	static const struct ov_regvals init_519[] = {
		{ 0x5a,  0x6d }, 
		{ 0x53,  0x9b },
		{ 0x54,  0xff }, 
		{ 0x5d,  0x03 },
		{ 0x49,  0x01 },
		{ 0x48,  0x00 },
		
		{ OV519_GPIO_IO_CTRL0,   0xee },
		{ 0x51,  0x0f }, 
		{ 0x51,  0x00 },
		{ 0x22,  0x00 },
		
	};

	return write_regvals(sd, init_519, ARRAY_SIZE(init_519));
}


static int sd_config(struct gspca_dev *gspca_dev,
			const struct usb_device_id *id)
{
	struct sd *sd = (struct sd *) gspca_dev;
	struct cam *cam;
	int ret = 0;

	sd->bridge = id->driver_info & BRIDGE_MASK;
	sd->invert_led = id->driver_info & BRIDGE_INVERT_LED;

	switch (sd->bridge) {
	case BRIDGE_OV511:
	case BRIDGE_OV511PLUS:
		ret = ov511_configure(gspca_dev);
		break;
	case BRIDGE_OV518:
	case BRIDGE_OV518PLUS:
		ret = ov518_configure(gspca_dev);
		break;
	case BRIDGE_OV519:
		ret = ov519_configure(sd);
		break;
	}

	if (ret)
		goto error;

	ov51x_led_control(sd, 0);	

	
	if (ov51x_set_slave_ids(sd, OV7xx0_SID) < 0)
		goto error;

	
	if (init_ov_sensor(sd) >= 0) {
		if (ov7xx0_configure(sd) < 0) {
			PDEBUG(D_ERR, "Failed to configure OV7xx0");
			goto error;
		}
	} else {

		
		if (ov51x_set_slave_ids(sd, OV6xx0_SID) < 0)
			goto error;

		if (init_ov_sensor(sd) >= 0) {
			if (ov6xx0_configure(sd) < 0) {
				PDEBUG(D_ERR, "Failed to configure OV6xx0");
				goto error;
			}
		} else {

			
			if (ov51x_set_slave_ids(sd, OV8xx0_SID) < 0)
				goto error;

			if (init_ov_sensor(sd) < 0) {
				PDEBUG(D_ERR,
					"Can't determine sensor slave IDs");
				goto error;
			}
			if (ov8xx0_configure(sd) < 0) {
				PDEBUG(D_ERR,
					"Failed to configure OV8xx0 sensor");
				goto error;
			}
		}
	}

	cam = &gspca_dev->cam;
	switch (sd->bridge) {
	case BRIDGE_OV511:
	case BRIDGE_OV511PLUS:
		if (!sd->sif) {
			cam->cam_mode = ov511_vga_mode;
			cam->nmodes = ARRAY_SIZE(ov511_vga_mode);
		} else {
			cam->cam_mode = ov511_sif_mode;
			cam->nmodes = ARRAY_SIZE(ov511_sif_mode);
		}
		break;
	case BRIDGE_OV518:
	case BRIDGE_OV518PLUS:
		if (!sd->sif) {
			cam->cam_mode = ov518_vga_mode;
			cam->nmodes = ARRAY_SIZE(ov518_vga_mode);
		} else {
			cam->cam_mode = ov518_sif_mode;
			cam->nmodes = ARRAY_SIZE(ov518_sif_mode);
		}
		break;
	case BRIDGE_OV519:
		if (!sd->sif) {
			cam->cam_mode = ov519_vga_mode;
			cam->nmodes = ARRAY_SIZE(ov519_vga_mode);
		} else {
			cam->cam_mode = ov519_sif_mode;
			cam->nmodes = ARRAY_SIZE(ov519_sif_mode);
		}
		break;
	}
	sd->brightness = BRIGHTNESS_DEF;
	if (sd->sensor == SEN_OV6630 || sd->sensor == SEN_OV66308AF)
		sd->contrast = 200; 
	else
		sd->contrast = CONTRAST_DEF;
	sd->colors = COLOR_DEF;
	sd->hflip = HFLIP_DEF;
	sd->vflip = VFLIP_DEF;
	sd->autobrightness = AUTOBRIGHT_DEF;
	if (sd->sensor == SEN_OV7670) {
		sd->freq = OV7670_FREQ_DEF;
		gspca_dev->ctrl_dis = 1 << FREQ_IDX;
	} else {
		sd->freq = FREQ_DEF;
		gspca_dev->ctrl_dis = (1 << HFLIP_IDX) | (1 << VFLIP_IDX) |
				      (1 << OV7670_FREQ_IDX);
	}
	if (sd->sensor == SEN_OV7640 || sd->sensor == SEN_OV7670)
		gspca_dev->ctrl_dis |= 1 << AUTOBRIGHT_IDX;
	
	if (sd->sensor == SEN_OV8610)
		gspca_dev->ctrl_dis |= 1 << FREQ_IDX;

	return 0;
error:
	PDEBUG(D_ERR, "OV519 Config failed");
	return -EBUSY;
}


static int sd_init(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;

	
	switch (sd->sensor) {
	case SEN_OV6620:
		if (write_i2c_regvals(sd, norm_6x20, ARRAY_SIZE(norm_6x20)))
			return -EIO;
		break;
	case SEN_OV6630:
	case SEN_OV66308AF:
		if (write_i2c_regvals(sd, norm_6x30, ARRAY_SIZE(norm_6x30)))
			return -EIO;
		break;
	default:


		if (write_i2c_regvals(sd, norm_7610, ARRAY_SIZE(norm_7610)))
			return -EIO;
		if (i2c_w_mask(sd, 0x0e, 0x00, 0x40))
			return -EIO;
		break;
	case SEN_OV7620:
		if (write_i2c_regvals(sd, norm_7620, ARRAY_SIZE(norm_7620)))
			return -EIO;
		break;
	case SEN_OV7640:
		if (write_i2c_regvals(sd, norm_7640, ARRAY_SIZE(norm_7640)))
			return -EIO;
		break;
	case SEN_OV7670:
		if (write_i2c_regvals(sd, norm_7670, ARRAY_SIZE(norm_7670)))
			return -EIO;
		break;
	case SEN_OV8610:
		if (write_i2c_regvals(sd, norm_8610, ARRAY_SIZE(norm_8610)))
			return -EIO;
		break;
	}
	return 0;
}


static int ov511_mode_init_regs(struct sd *sd)
{
	int hsegs, vsegs, packet_size, fps, needed;
	int interlaced = 0;
	struct usb_host_interface *alt;
	struct usb_interface *intf;

	intf = usb_ifnum_to_if(sd->gspca_dev.dev, sd->gspca_dev.iface);
	alt = usb_altnum_to_altsetting(intf, sd->gspca_dev.alt);
	if (!alt) {
		PDEBUG(D_ERR, "Couldn't get altsetting");
		return -EIO;
	}

	packet_size = le16_to_cpu(alt->endpoint[0].desc.wMaxPacketSize);
	reg_w(sd, R51x_FIFO_PSIZE, packet_size >> 5);

	reg_w(sd, R511_CAM_UV_EN, 0x01);
	reg_w(sd, R511_SNAP_UV_EN, 0x01);
	reg_w(sd, R511_SNAP_OPTS, 0x03);

	
	hsegs = (sd->gspca_dev.width >> 3) - 1;
	vsegs = (sd->gspca_dev.height >> 3) - 1;

	reg_w(sd, R511_CAM_PXCNT, hsegs);
	reg_w(sd, R511_CAM_LNCNT, vsegs);
	reg_w(sd, R511_CAM_PXDIV, 0x00);
	reg_w(sd, R511_CAM_LNDIV, 0x00);

	
	reg_w(sd, R511_CAM_OPTS, 0x03);

	
	reg_w(sd, R511_SNAP_PXCNT, hsegs);
	reg_w(sd, R511_SNAP_LNCNT, vsegs);
	reg_w(sd, R511_SNAP_PXDIV, 0x00);
	reg_w(sd, R511_SNAP_LNDIV, 0x00);

	
	if (frame_rate > 0)
		sd->frame_rate = frame_rate;

	switch (sd->sensor) {
	case SEN_OV6620:
		
		sd->clockdiv = 3;
		break;

	
	case SEN_OV7620:
	case SEN_OV7640:
	case SEN_OV76BE:
		if (sd->gspca_dev.width == 320)
			interlaced = 1;
		
	case SEN_OV6630:
	case SEN_OV7610:
	case SEN_OV7670:
		switch (sd->frame_rate) {
		case 30:
		case 25:
			
			if (sd->gspca_dev.width != 640) {
				sd->clockdiv = 0;
				break;
			}
			
		default:


			sd->clockdiv = 1;
			break;
		case 10:
			sd->clockdiv = 2;
			break;
		case 5:
			sd->clockdiv = 5;
			break;
		}
		if (interlaced) {
			sd->clockdiv = (sd->clockdiv + 1) * 2 - 1;
			
			if (sd->clockdiv > 10)
				sd->clockdiv = 10;
		}
		break;

	case SEN_OV8610:
		
		sd->clockdiv = 0;
		break;
	}

	
	fps = (interlaced ? 60 : 30) / (sd->clockdiv + 1) + 1;
	needed = fps * sd->gspca_dev.width * sd->gspca_dev.height * 3 / 2;
	
	if (needed > 1400 * packet_size) {
		
		reg_w(sd, R511_COMP_EN, 0x07);
		reg_w(sd, R511_COMP_LUT_EN, 0x03);
	} else {
		reg_w(sd, R511_COMP_EN, 0x06);
		reg_w(sd, R511_COMP_LUT_EN, 0x00);
	}

	reg_w(sd, R51x_SYS_RESET, OV511_RESET_OMNICE);
	reg_w(sd, R51x_SYS_RESET, 0);

	return 0;
}


static int ov518_mode_init_regs(struct sd *sd)
{
	int hsegs, vsegs, packet_size;
	struct usb_host_interface *alt;
	struct usb_interface *intf;

	intf = usb_ifnum_to_if(sd->gspca_dev.dev, sd->gspca_dev.iface);
	alt = usb_altnum_to_altsetting(intf, sd->gspca_dev.alt);
	if (!alt) {
		PDEBUG(D_ERR, "Couldn't get altsetting");
		return -EIO;
	}

	packet_size = le16_to_cpu(alt->endpoint[0].desc.wMaxPacketSize);
	ov518_reg_w32(sd, R51x_FIFO_PSIZE, packet_size & ~7, 2);

	

	reg_w(sd, 0x2b, 0);
	reg_w(sd, 0x2c, 0);
	reg_w(sd, 0x2d, 0);
	reg_w(sd, 0x2e, 0);
	reg_w(sd, 0x3b, 0);
	reg_w(sd, 0x3c, 0);
	reg_w(sd, 0x3d, 0);
	reg_w(sd, 0x3e, 0);

	if (sd->bridge == BRIDGE_OV518) {
		
		reg_w_mask(sd, 0x20, 0x08, 0x08);

		
		reg_w_mask(sd, 0x28, 0x80, 0xf0);
		reg_w_mask(sd, 0x38, 0x80, 0xf0);
	} else {
		reg_w(sd, 0x28, 0x80);
		reg_w(sd, 0x38, 0x80);
	}

	hsegs = sd->gspca_dev.width / 16;
	vsegs = sd->gspca_dev.height / 4;

	reg_w(sd, 0x29, hsegs);
	reg_w(sd, 0x2a, vsegs);

	reg_w(sd, 0x39, hsegs);
	reg_w(sd, 0x3a, vsegs);

	
	reg_w(sd, 0x2f, 0x80);

	
	sd->clockdiv = 1;

	
	
	reg_w(sd, 0x51, 0x04);
	reg_w(sd, 0x22, 0x18);
	reg_w(sd, 0x23, 0xff);

	if (sd->bridge == BRIDGE_OV518PLUS) {
		switch (sd->sensor) {
		case SEN_OV7620:
			if (sd->gspca_dev.width == 320) {
				reg_w(sd, 0x20, 0x00);
				reg_w(sd, 0x21, 0x19);
			} else {
				reg_w(sd, 0x20, 0x60);
				reg_w(sd, 0x21, 0x1f);
			}
			break;
		default:
			reg_w(sd, 0x21, 0x19);
		}
	} else
		reg_w(sd, 0x71, 0x17);	

	
	
	i2c_w(sd, 0x54, 0x23);

	reg_w(sd, 0x2f, 0x80);

	if (sd->bridge == BRIDGE_OV518PLUS) {
		reg_w(sd, 0x24, 0x94);
		reg_w(sd, 0x25, 0x90);
		ov518_reg_w32(sd, 0xc4,    400, 2);	
		ov518_reg_w32(sd, 0xc6,    540, 2);	
		ov518_reg_w32(sd, 0xc7,    540, 2);	
		ov518_reg_w32(sd, 0xc8,    108, 2);	
		ov518_reg_w32(sd, 0xca, 131098, 3);	
		ov518_reg_w32(sd, 0xcb,    532, 2);	
		ov518_reg_w32(sd, 0xcc,   2400, 2);	
		ov518_reg_w32(sd, 0xcd,     32, 2);	
		ov518_reg_w32(sd, 0xce,    608, 2);	
	} else {
		reg_w(sd, 0x24, 0x9f);
		reg_w(sd, 0x25, 0x90);
		ov518_reg_w32(sd, 0xc4,    400, 2);	
		ov518_reg_w32(sd, 0xc6,    381, 2);	
		ov518_reg_w32(sd, 0xc7,    381, 2);	
		ov518_reg_w32(sd, 0xc8,    128, 2);	
		ov518_reg_w32(sd, 0xca, 183331, 3);	
		ov518_reg_w32(sd, 0xcb,    746, 2);	
		ov518_reg_w32(sd, 0xcc,   1750, 2);	
		ov518_reg_w32(sd, 0xcd,     45, 2);	
		ov518_reg_w32(sd, 0xce,    851, 2);	
	}

	reg_w(sd, 0x2f, 0x80);

	return 0;
}



static int ov519_mode_init_regs(struct sd *sd)
{
	static const struct ov_regvals mode_init_519_ov7670[] = {
		{ 0x5d,	0x03 }, 
		{ 0x53,	0x9f }, 
		{ 0x54,	0x0f }, 
		{ 0xa2,	0x20 }, 
		{ 0xa3,	0x18 },
		{ 0xa4,	0x04 },
		{ 0xa5,	0x28 },
		{ 0x37,	0x00 },	
		{ 0x55,	0x02 }, 
		
		{ 0x20,	0x0c },
		{ 0x21,	0x38 },
		{ 0x22,	0x1d },
		{ 0x17,	0x50 }, 
		{ 0x37,	0x00 }, 
		{ 0x40,	0xff }, 
		{ 0x46,	0x00 }, 
		{ 0x59,	0x04 },	
		{ 0xff,	0x00 }, 
		
	};

	static const struct ov_regvals mode_init_519[] = {
		{ 0x5d,	0x03 }, 
		{ 0x53,	0x9f }, 
		{ 0x54,	0x0f }, 
		{ 0xa2,	0x20 }, 
		{ 0xa3,	0x18 },
		{ 0xa4,	0x04 },
		{ 0xa5,	0x28 },
		{ 0x37,	0x00 },	
		{ 0x55,	0x02 }, 
		
		{ 0x22,	0x1d },
		{ 0x17,	0x50 }, 
		{ 0x37,	0x00 }, 
		{ 0x40,	0xff }, 
		{ 0x46,	0x00 }, 
		{ 0x59,	0x04 },	
		{ 0xff,	0x00 }, 
		
	};

	
	if (sd->sensor != SEN_OV7670) {
		if (write_regvals(sd, mode_init_519,
				  ARRAY_SIZE(mode_init_519)))
			return -EIO;
		if (sd->sensor == SEN_OV7640) {
			
			reg_w_mask(sd, OV519_R20_DFR, 0x10, 0x10);
		}
	} else {
		if (write_regvals(sd, mode_init_519_ov7670,
				  ARRAY_SIZE(mode_init_519_ov7670)))
			return -EIO;
	}

	reg_w(sd, OV519_R10_H_SIZE,	sd->gspca_dev.width >> 4);
	reg_w(sd, OV519_R11_V_SIZE,	sd->gspca_dev.height >> 3);
	if (sd->sensor == SEN_OV7670 &&
	    sd->gspca_dev.cam.cam_mode[sd->gspca_dev.curr_mode].priv)
		reg_w(sd, OV519_R12_X_OFFSETL, 0x04);
	else
		reg_w(sd, OV519_R12_X_OFFSETL, 0x00);
	reg_w(sd, OV519_R13_X_OFFSETH,	0x00);
	reg_w(sd, OV519_R14_Y_OFFSETL,	0x00);
	reg_w(sd, OV519_R15_Y_OFFSETH,	0x00);
	reg_w(sd, OV519_R16_DIVIDER,	0x00);
	reg_w(sd, OV519_R25_FORMAT,	0x03); 
	reg_w(sd, 0x26,			0x00); 

	
	if (frame_rate > 0)
		sd->frame_rate = frame_rate;


	sd->clockdiv = 0;
	switch (sd->sensor) {
	case SEN_OV7640:
		switch (sd->frame_rate) {
		default:

			reg_w(sd, 0xa4, 0x0c);
			reg_w(sd, 0x23, 0xff);
			break;
		case 25:
			reg_w(sd, 0xa4, 0x0c);
			reg_w(sd, 0x23, 0x1f);
			break;
		case 20:
			reg_w(sd, 0xa4, 0x0c);
			reg_w(sd, 0x23, 0x1b);
			break;
		case 15:
			reg_w(sd, 0xa4, 0x04);
			reg_w(sd, 0x23, 0xff);
			sd->clockdiv = 1;
			break;
		case 10:
			reg_w(sd, 0xa4, 0x04);
			reg_w(sd, 0x23, 0x1f);
			sd->clockdiv = 1;
			break;
		case 5:
			reg_w(sd, 0xa4, 0x04);
			reg_w(sd, 0x23, 0x1b);
			sd->clockdiv = 1;
			break;
		}
		break;
	case SEN_OV8610:
		switch (sd->frame_rate) {
		default:	

			reg_w(sd, 0xa4, 0x06);
			reg_w(sd, 0x23, 0xff);
			break;
		case 10:
			reg_w(sd, 0xa4, 0x06);
			reg_w(sd, 0x23, 0x1f);
			break;
		case 5:
			reg_w(sd, 0xa4, 0x06);
			reg_w(sd, 0x23, 0x1b);
			break;
		}
		break;
	case SEN_OV7670:		
		PDEBUG(D_STREAM, "Setting framerate to %d fps",
				 (sd->frame_rate == 0) ? 15 : sd->frame_rate);
		reg_w(sd, 0xa4, 0x10);
		switch (sd->frame_rate) {
		case 30:
			reg_w(sd, 0x23, 0xff);
			break;
		case 20:
			reg_w(sd, 0x23, 0x1b);
			break;
		default:

			reg_w(sd, 0x23, 0xff);
			sd->clockdiv = 1;
			break;
		}
		break;
	}
	return 0;
}

static int mode_init_ov_sensor_regs(struct sd *sd)
{
	struct gspca_dev *gspca_dev;
	int qvga;

	gspca_dev = &sd->gspca_dev;
	qvga = gspca_dev->cam.cam_mode[(int) gspca_dev->curr_mode].priv & 1;

	
	switch (sd->sensor) {
	case SEN_OV8610:
		
		i2c_w_mask(sd, OV7610_REG_COM_C, qvga ? (1 << 5) : 0, 1 << 5);
		break;
	case SEN_OV7610:
		i2c_w_mask(sd, 0x14, qvga ? 0x20 : 0x00, 0x20);
		break;
	case SEN_OV7620:
	case SEN_OV76BE:
		i2c_w_mask(sd, 0x14, qvga ? 0x20 : 0x00, 0x20);
		i2c_w_mask(sd, 0x28, qvga ? 0x00 : 0x20, 0x20);
		i2c_w(sd, 0x24, qvga ? 0x20 : 0x3a);
		i2c_w(sd, 0x25, qvga ? 0x30 : 0x60);
		i2c_w_mask(sd, 0x2d, qvga ? 0x40 : 0x00, 0x40);
		i2c_w_mask(sd, 0x67, qvga ? 0xb0 : 0x90, 0xf0);
		i2c_w_mask(sd, 0x74, qvga ? 0x20 : 0x00, 0x20);
		break;
	case SEN_OV7640:
		i2c_w_mask(sd, 0x14, qvga ? 0x20 : 0x00, 0x20);
		i2c_w_mask(sd, 0x28, qvga ? 0x00 : 0x20, 0x20);





		break;
	case SEN_OV7670:
		
		i2c_w_mask(sd, OV7670_REG_COM7,
			 qvga ? OV7670_COM7_FMT_QVGA : OV7670_COM7_FMT_VGA,
			 OV7670_COM7_FMT_MASK);
		break;
	case SEN_OV6620:
	case SEN_OV6630:
	case SEN_OV66308AF:
		i2c_w_mask(sd, 0x14, qvga ? 0x20 : 0x00, 0x20);
		break;
	default:
		return -EINVAL;
	}

	

	

	

	if (sd->sensor != SEN_OV6630 && sd->sensor != SEN_OV66308AF &&
					sd->sensor != SEN_OV7640)
		i2c_w_mask(sd, 0x13, 0x00, 0x20);

	
	i2c_w(sd, 0x11, sd->clockdiv);

	

	
	if (sd->sensor != SEN_OV7640 && sd->sensor != SEN_OV7670)
		i2c_w_mask(sd, 0x12, 0x00, 0x02);

	
	if (sd->sensor == SEN_OV7670)
		i2c_w_mask(sd, OV7670_REG_COM8, OV7670_COM8_AWB,
				OV7670_COM8_AWB);
	else
		i2c_w_mask(sd, 0x12, 0x04, 0x04);

	
	
	
	if (sd->sensor == SEN_OV7610 || sd->sensor == SEN_OV76BE) {
		if (!qvga)
			i2c_w(sd, 0x35, 0x9e);
		else
			i2c_w(sd, 0x35, 0x1e);
	}
	return 0;
}

static void sethvflip(struct sd *sd)
{
	if (sd->sensor != SEN_OV7670)
		return;
	if (sd->gspca_dev.streaming)
		ov51x_stop(sd);
	i2c_w_mask(sd, OV7670_REG_MVFP,
		OV7670_MVFP_MIRROR * sd->hflip
			| OV7670_MVFP_VFLIP * sd->vflip,
		OV7670_MVFP_MIRROR | OV7670_MVFP_VFLIP);
	if (sd->gspca_dev.streaming)
		ov51x_restart(sd);
}

static int set_ov_sensor_window(struct sd *sd)
{
	struct gspca_dev *gspca_dev;
	int qvga, crop;
	int hwsbase, hwebase, vwsbase, vwebase, hwscale, vwscale;
	int ret, hstart, hstop, vstop, vstart;
	__u8 v;

	gspca_dev = &sd->gspca_dev;
	qvga = gspca_dev->cam.cam_mode[(int) gspca_dev->curr_mode].priv & 1;
	crop = gspca_dev->cam.cam_mode[(int) gspca_dev->curr_mode].priv & 2;

	
	switch (sd->sensor) {
	case SEN_OV8610:
		hwsbase = 0x1e;
		hwebase = 0x1e;
		vwsbase = 0x02;
		vwebase = 0x02;
		break;
	case SEN_OV7610:
	case SEN_OV76BE:
		hwsbase = 0x38;
		hwebase = 0x3a;
		vwsbase = vwebase = 0x05;
		break;
	case SEN_OV6620:
	case SEN_OV6630:
	case SEN_OV66308AF:
		hwsbase = 0x38;
		hwebase = 0x3a;
		vwsbase = 0x05;
		vwebase = 0x06;
		if (sd->sensor == SEN_OV66308AF && qvga)
			
			hwsbase++;
		if (crop) {
			hwsbase += 8;
			hwebase += 8;
			vwsbase += 11;
			vwebase += 11;
		}
		break;
	case SEN_OV7620:
		hwsbase = 0x2f;		
		hwebase = 0x2f;
		vwsbase = vwebase = 0x05;
		break;
	case SEN_OV7640:
		hwsbase = 0x1a;
		hwebase = 0x1a;
		vwsbase = vwebase = 0x03;
		break;
	case SEN_OV7670:
		
		vwsbase = vwebase = hwebase = hwsbase = 0x00;
		break;
	default:
		return -EINVAL;
	}

	switch (sd->sensor) {
	case SEN_OV6620:
	case SEN_OV6630:
	case SEN_OV66308AF:
		if (qvga) {		
			hwscale = 0;
			vwscale = 0;
		} else {		
			hwscale = 1;
			vwscale = 1;	
		}
		break;
	case SEN_OV8610:
		if (qvga) {		
			hwscale = 1;
			vwscale = 1;
		} else {		
			hwscale = 2;
			vwscale = 2;
		}
		break;
	default:			
		if (qvga) {		
			hwscale = 1;
			vwscale = 0;
		} else {		
			hwscale = 2;
			vwscale = 1;
		}
	}

	ret = mode_init_ov_sensor_regs(sd);
	if (ret < 0)
		return ret;

	if (sd->sensor == SEN_OV8610) {
		i2c_w_mask(sd, 0x2d, 0x05, 0x40);
				
						
		i2c_w_mask(sd, 0x28, 0x20, 0x20);
					
	}

	

	
	
	
	
	if (sd->sensor == SEN_OV7670) {
		if (qvga) {		
			hstart = 164;
			hstop = 28;
			vstart = 14;
			vstop = 494;
		} else {		
			hstart = 158;
			hstop = 14;
			vstart = 10;
			vstop = 490;
		}
		
		i2c_w(sd, OV7670_REG_HSTART, hstart >> 3);
		i2c_w(sd, OV7670_REG_HSTOP, hstop >> 3);
		v = i2c_r(sd, OV7670_REG_HREF);
		v = (v & 0xc0) | ((hstop & 0x7) << 3) | (hstart & 0x07);
		msleep(10);	
		i2c_w(sd, OV7670_REG_HREF, v);

		i2c_w(sd, OV7670_REG_VSTART, vstart >> 2);
		i2c_w(sd, OV7670_REG_VSTOP, vstop >> 2);
		v = i2c_r(sd, OV7670_REG_VREF);
		v = (v & 0xc0) | ((vstop & 0x3) << 2) | (vstart & 0x03);
		msleep(10);	
		i2c_w(sd, OV7670_REG_VREF, v);
	} else {
		i2c_w(sd, 0x17, hwsbase);
		i2c_w(sd, 0x18, hwebase + (sd->gspca_dev.width >> hwscale));
		i2c_w(sd, 0x19, vwsbase);
		i2c_w(sd, 0x1a, vwebase + (sd->gspca_dev.height >> vwscale));
	}
	return 0;
}


static int sd_start(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;
	int ret = 0;

	switch (sd->bridge) {
	case BRIDGE_OV511:
	case BRIDGE_OV511PLUS:
		ret = ov511_mode_init_regs(sd);
		break;
	case BRIDGE_OV518:
	case BRIDGE_OV518PLUS:
		ret = ov518_mode_init_regs(sd);
		break;
	case BRIDGE_OV519:
		ret = ov519_mode_init_regs(sd);
		break;
	}
	if (ret < 0)
		goto out;

	ret = set_ov_sensor_window(sd);
	if (ret < 0)
		goto out;

	setcontrast(gspca_dev);
	setbrightness(gspca_dev);
	setcolors(gspca_dev);
	sethvflip(sd);
	setautobrightness(sd);
	setfreq(sd);

	ret = ov51x_restart(sd);
	if (ret < 0)
		goto out;
	ov51x_led_control(sd, 1);
	return 0;
out:
	PDEBUG(D_ERR, "camera start error:%d", ret);
	return ret;
}

static void sd_stopN(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;

	ov51x_stop(sd);
	ov51x_led_control(sd, 0);
}

static void ov511_pkt_scan(struct gspca_dev *gspca_dev,
			struct gspca_frame *frame,	
			__u8 *in,			
			int len)			
{
	struct sd *sd = (struct sd *) gspca_dev;

	
	if (!(in[0] | in[1] | in[2] | in[3] | in[4] | in[5] | in[6] | in[7]) &&
	    (in[8] & 0x08)) {
		if (in[8] & 0x80) {
			
			if ((in[9] + 1) * 8 != gspca_dev->width ||
			    (in[10] + 1) * 8 != gspca_dev->height) {
				PDEBUG(D_ERR, "Invalid frame size, got: %dx%d,"
					" requested: %dx%d\n",
					(in[9] + 1) * 8, (in[10] + 1) * 8,
					gspca_dev->width, gspca_dev->height);
				gspca_dev->last_packet_type = DISCARD_PACKET;
				return;
			}
			
			gspca_frame_add(gspca_dev, LAST_PACKET, frame, in, 11);
			return;
		} else {
			
			gspca_frame_add(gspca_dev, FIRST_PACKET, frame, in, 0);
			sd->packet_nr = 0;
		}
	}

	
	len--;

	
	gspca_frame_add(gspca_dev, INTER_PACKET, frame, in, len);
}

static void ov518_pkt_scan(struct gspca_dev *gspca_dev,
			struct gspca_frame *frame,	
			__u8 *data,			
			int len)			
{
	struct sd *sd = (struct sd *) gspca_dev;

	
	if ((!(data[0] | data[1] | data[2] | data[3] | data[5])) && data[6]) {
		frame = gspca_frame_add(gspca_dev, LAST_PACKET, frame, data, 0);
		gspca_frame_add(gspca_dev, FIRST_PACKET, frame, data, 0);
		sd->packet_nr = 0;
	}

	if (gspca_dev->last_packet_type == DISCARD_PACKET)
		return;

	
	if (len & 7) {
		len--;
		if (sd->packet_nr == data[len])
			sd->packet_nr++;
		
		else if (sd->packet_nr == 0 || data[len]) {
			PDEBUG(D_ERR, "Invalid packet nr: %d (expect: %d)",
				(int)data[len], (int)sd->packet_nr);
			gspca_dev->last_packet_type = DISCARD_PACKET;
			return;
		}
	}

	
	gspca_frame_add(gspca_dev, INTER_PACKET, frame, data, len);
}

static void ov519_pkt_scan(struct gspca_dev *gspca_dev,
			struct gspca_frame *frame,	
			__u8 *data,			
			int len)			
{
	

	if (data[0] == 0xff && data[1] == 0xff && data[2] == 0xff) {
		switch (data[3]) {
		case 0x50:		
#define HDRSZ 16
			data += HDRSZ;
			len -= HDRSZ;
#undef HDRSZ
			if (data[0] == 0xff || data[1] == 0xd8)
				gspca_frame_add(gspca_dev, FIRST_PACKET, frame,
						data, len);
			else
				gspca_dev->last_packet_type = DISCARD_PACKET;
			return;
		case 0x51:		
			if (data[9] != 0)
				gspca_dev->last_packet_type = DISCARD_PACKET;
			gspca_frame_add(gspca_dev, LAST_PACKET, frame,
					data, 0);
			return;
		}
	}

	
	gspca_frame_add(gspca_dev, INTER_PACKET, frame,
			data, len);
}

static void sd_pkt_scan(struct gspca_dev *gspca_dev,
			struct gspca_frame *frame,	
			__u8 *data,			
			int len)			
{
	struct sd *sd = (struct sd *) gspca_dev;

	switch (sd->bridge) {
	case BRIDGE_OV511:
	case BRIDGE_OV511PLUS:
		ov511_pkt_scan(gspca_dev, frame, data, len);
		break;
	case BRIDGE_OV518:
	case BRIDGE_OV518PLUS:
		ov518_pkt_scan(gspca_dev, frame, data, len);
		break;
	case BRIDGE_OV519:
		ov519_pkt_scan(gspca_dev, frame, data, len);
		break;
	}
}



static void setbrightness(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;
	int val;

	val = sd->brightness;
	switch (sd->sensor) {
	case SEN_OV8610:
	case SEN_OV7610:
	case SEN_OV76BE:
	case SEN_OV6620:
	case SEN_OV6630:
	case SEN_OV66308AF:
	case SEN_OV7640:
		i2c_w(sd, OV7610_REG_BRT, val);
		break;
	case SEN_OV7620:
		
		if (!sd->autobrightness)
			i2c_w(sd, OV7610_REG_BRT, val);
		break;
	case SEN_OV7670:

		i2c_w(sd, OV7670_REG_BRIGHT, ov7670_abs_to_sm(val));
		break;
	}
}

static void setcontrast(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;
	int val;

	val = sd->contrast;
	switch (sd->sensor) {
	case SEN_OV7610:
	case SEN_OV6620:
		i2c_w(sd, OV7610_REG_CNT, val);
		break;
	case SEN_OV6630:
	case SEN_OV66308AF:
		i2c_w_mask(sd, OV7610_REG_CNT, val >> 4, 0x0f);
		break;
	case SEN_OV8610: {
		static const __u8 ctab[] = {
			0x03, 0x09, 0x0b, 0x0f, 0x53, 0x6f, 0x35, 0x7f
		};

		
		i2c_w(sd, 0x64, ctab[val >> 5]);
		break;
	    }
	case SEN_OV7620: {
		static const __u8 ctab[] = {
			0x01, 0x05, 0x09, 0x11, 0x15, 0x35, 0x37, 0x57,
			0x5b, 0xa5, 0xa7, 0xc7, 0xc9, 0xcf, 0xef, 0xff
		};

		
		i2c_w(sd, 0x64, ctab[val >> 4]);
		break;
	    }
	case SEN_OV7640:
		
		i2c_w(sd, OV7610_REG_GAIN, val >> 2);
		break;
	case SEN_OV7670:
		
		i2c_w(sd, OV7670_REG_CONTRAS, val >> 1);
		break;
	}
}

static void setcolors(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;
	int val;

	val = sd->colors;
	switch (sd->sensor) {
	case SEN_OV8610:
	case SEN_OV7610:
	case SEN_OV76BE:
	case SEN_OV6620:
	case SEN_OV6630:
	case SEN_OV66308AF:
		i2c_w(sd, OV7610_REG_SAT, val);
		break;
	case SEN_OV7620:
		

		i2c_w(sd, OV7610_REG_SAT, val);
		break;
	case SEN_OV7640:
		i2c_w(sd, OV7610_REG_SAT, val & 0xf0);
		break;
	case SEN_OV7670:
		
		
		break;
	}
}

static void setautobrightness(struct sd *sd)
{
	if (sd->sensor == SEN_OV7640 || sd->sensor == SEN_OV7670)
		return;

	i2c_w_mask(sd, 0x2d, sd->autobrightness ? 0x10 : 0x00, 0x10);
}

static void setfreq(struct sd *sd)
{
	if (sd->sensor == SEN_OV7670) {
		switch (sd->freq) {
		case 0: 
			i2c_w_mask(sd, OV7670_REG_COM8, 0, OV7670_COM8_BFILT);
			break;
		case 1: 
			i2c_w_mask(sd, OV7670_REG_COM8, OV7670_COM8_BFILT,
				   OV7670_COM8_BFILT);
			i2c_w_mask(sd, OV7670_REG_COM11, 0x08, 0x18);
			break;
		case 2: 
			i2c_w_mask(sd, OV7670_REG_COM8, OV7670_COM8_BFILT,
				   OV7670_COM8_BFILT);
			i2c_w_mask(sd, OV7670_REG_COM11, 0x00, 0x18);
			break;
		case 3: 
			i2c_w_mask(sd, OV7670_REG_COM8, OV7670_COM8_BFILT,
				   OV7670_COM8_BFILT);
			i2c_w_mask(sd, OV7670_REG_COM11, OV7670_COM11_HZAUTO,
				   0x18);
			break;
		}
	} else {
		switch (sd->freq) {
		case 0: 
			i2c_w_mask(sd, 0x2d, 0x00, 0x04);
			i2c_w_mask(sd, 0x2a, 0x00, 0x80);
			break;
		case 1: 
			i2c_w_mask(sd, 0x2d, 0x04, 0x04);
			i2c_w_mask(sd, 0x2a, 0x80, 0x80);
			
			if (sd->sensor == SEN_OV6620 ||
			    sd->sensor == SEN_OV6630 ||
			    sd->sensor == SEN_OV66308AF)
				i2c_w(sd, 0x2b, 0x5e);
			else
				i2c_w(sd, 0x2b, 0xac);
			break;
		case 2: 
			i2c_w_mask(sd, 0x2d, 0x04, 0x04);
			if (sd->sensor == SEN_OV6620 ||
			    sd->sensor == SEN_OV6630 ||
			    sd->sensor == SEN_OV66308AF) {
				
				i2c_w_mask(sd, 0x2a, 0x80, 0x80);
				i2c_w(sd, 0x2b, 0xa8);
			} else {
				
				i2c_w_mask(sd, 0x2a, 0x00, 0x80);
			}
			break;
		}
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

static int sd_setcontrast(struct gspca_dev *gspca_dev, __s32 val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	sd->contrast = val;
	if (gspca_dev->streaming)
		setcontrast(gspca_dev);
	return 0;
}

static int sd_getcontrast(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	*val = sd->contrast;
	return 0;
}

static int sd_setcolors(struct gspca_dev *gspca_dev, __s32 val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	sd->colors = val;
	if (gspca_dev->streaming)
		setcolors(gspca_dev);
	return 0;
}

static int sd_getcolors(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	*val = sd->colors;
	return 0;
}

static int sd_sethflip(struct gspca_dev *gspca_dev, __s32 val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	sd->hflip = val;
	if (gspca_dev->streaming)
		sethvflip(sd);
	return 0;
}

static int sd_gethflip(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	*val = sd->hflip;
	return 0;
}

static int sd_setvflip(struct gspca_dev *gspca_dev, __s32 val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	sd->vflip = val;
	if (gspca_dev->streaming)
		sethvflip(sd);
	return 0;
}

static int sd_getvflip(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	*val = sd->vflip;
	return 0;
}

static int sd_setautobrightness(struct gspca_dev *gspca_dev, __s32 val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	sd->autobrightness = val;
	if (gspca_dev->streaming)
		setautobrightness(sd);
	return 0;
}

static int sd_getautobrightness(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	*val = sd->autobrightness;
	return 0;
}

static int sd_setfreq(struct gspca_dev *gspca_dev, __s32 val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	sd->freq = val;
	if (gspca_dev->streaming)
		setfreq(sd);
	return 0;
}

static int sd_getfreq(struct gspca_dev *gspca_dev, __s32 *val)
{
	struct sd *sd = (struct sd *) gspca_dev;

	*val = sd->freq;
	return 0;
}

static int sd_querymenu(struct gspca_dev *gspca_dev,
			struct v4l2_querymenu *menu)
{
	struct sd *sd = (struct sd *) gspca_dev;

	switch (menu->id) {
	case V4L2_CID_POWER_LINE_FREQUENCY:
		switch (menu->index) {
		case 0:		
			strcpy((char *) menu->name, "NoFliker");
			return 0;
		case 1:		
			strcpy((char *) menu->name, "50 Hz");
			return 0;
		case 2:		
			strcpy((char *) menu->name, "60 Hz");
			return 0;
		case 3:
			if (sd->sensor != SEN_OV7670)
				return -EINVAL;

			strcpy((char *) menu->name, "Automatic");
			return 0;
		}
		break;
	}
	return -EINVAL;
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
	.querymenu = sd_querymenu,
};


static const __devinitdata struct usb_device_id device_table[] = {
	{USB_DEVICE(0x041e, 0x4052), .driver_info = BRIDGE_OV519 },
	{USB_DEVICE(0x041e, 0x405f), .driver_info = BRIDGE_OV519 },
	{USB_DEVICE(0x041e, 0x4060), .driver_info = BRIDGE_OV519 },
	{USB_DEVICE(0x041e, 0x4061), .driver_info = BRIDGE_OV519 },
	{USB_DEVICE(0x041e, 0x4064),
	 .driver_info = BRIDGE_OV519 | BRIDGE_INVERT_LED },
	{USB_DEVICE(0x041e, 0x4067), .driver_info = BRIDGE_OV519 },
	{USB_DEVICE(0x041e, 0x4068),
	 .driver_info = BRIDGE_OV519 | BRIDGE_INVERT_LED },
	{USB_DEVICE(0x045e, 0x028c), .driver_info = BRIDGE_OV519 },
	{USB_DEVICE(0x054c, 0x0154), .driver_info = BRIDGE_OV519 },
	{USB_DEVICE(0x054c, 0x0155), .driver_info = BRIDGE_OV519 },
	{USB_DEVICE(0x05a9, 0x0511), .driver_info = BRIDGE_OV511 },
	{USB_DEVICE(0x05a9, 0x0518), .driver_info = BRIDGE_OV518 },
	{USB_DEVICE(0x05a9, 0x0519), .driver_info = BRIDGE_OV519 },
	{USB_DEVICE(0x05a9, 0x0530), .driver_info = BRIDGE_OV519 },
	{USB_DEVICE(0x05a9, 0x4519), .driver_info = BRIDGE_OV519 },
	{USB_DEVICE(0x05a9, 0x8519), .driver_info = BRIDGE_OV519 },
	{USB_DEVICE(0x05a9, 0xa511), .driver_info = BRIDGE_OV511PLUS },
	{USB_DEVICE(0x05a9, 0xa518), .driver_info = BRIDGE_OV518PLUS },
	{USB_DEVICE(0x0813, 0x0002), .driver_info = BRIDGE_OV511PLUS },
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

module_param(frame_rate, int, 0644);
MODULE_PARM_DESC(frame_rate, "Frame rate (5, 10, 15, 20 or 30 fps)");
