#ifndef __LINUX_OV511_H
#define __LINUX_OV511_H

#include <asm/uaccess.h>
#include <linux/videodev.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>
#include <linux/usb.h>
#include <linux/mutex.h>

#define OV511_DEBUG	

#ifdef OV511_DEBUG
	#define PDEBUG(level, fmt, args...) \
		if (debug >= (level))	\
			printk(KERN_INFO KBUILD_MODNAME "[%s:%d] \n" fmt, \
		__func__, __LINE__ , ## args)
#else
	#define PDEBUG(level, fmt, args...) do {} while(0)
#endif


#define RESTRICT_TO_RANGE(v,mi,ma) { \
	if ((v) < (mi)) (v) = (mi); \
	else if ((v) > (ma)) (v) = (ma); \
}






#define VEND_OMNIVISION	0x05A9
#define PROD_OV511	0x0511
#define PROD_OV511PLUS	0xA511
#define PROD_OV518	0x0518
#define PROD_OV518PLUS	0xA518

#define VEND_MATTEL	0x0813
#define PROD_ME2CAM	0x0002






#define R511_CAM_DELAY		0x10
#define R511_CAM_EDGE		0x11
#define R511_CAM_PXCNT		0x12
#define R511_CAM_LNCNT		0x13
#define R511_CAM_PXDIV		0x14
#define R511_CAM_LNDIV		0x15
#define R511_CAM_UV_EN		0x16
#define R511_CAM_LINE_MODE	0x17
#define R511_CAM_OPTS		0x18


#define R511_SNAP_FRAME		0x19
#define R511_SNAP_PXCNT		0x1A
#define R511_SNAP_LNCNT		0x1B
#define R511_SNAP_PXDIV		0x1C
#define R511_SNAP_LNDIV		0x1D
#define R511_SNAP_UV_EN		0x1E
#define R511_SNAP_OPTS		0x1F


#define R511_DRAM_FLOW_CTL	0x20
#define R511_DRAM_ARCP		0x21
#define R511_DRAM_MRC		0x22
#define R511_DRAM_RFC		0x23


#define R51x_FIFO_PSIZE		0x30	
#define R511_FIFO_OPTS		0x31


#define R511_PIO_OPTS		0x38
#define R511_PIO_DATA		0x39
#define R511_PIO_BIST		0x3E
#define R518_GPIO_IN		0x55	
#define R518_GPIO_OUT		0x56	
#define R518_GPIO_CTL		0x57	
#define R518_GPIO_PULSE_IN	0x58	
#define R518_GPIO_PULSE_CLEAR	0x59	
#define R518_GPIO_PULSE_POL	0x5a	
#define R518_GPIO_PULSE_EN	0x5b	
#define R518_GPIO_RESET		0x5c	


#define R511_I2C_CTL		0x40
#define R518_I2C_CTL		0x47	
#define R51x_I2C_W_SID		0x41
#define R51x_I2C_SADDR_3	0x42
#define R51x_I2C_SADDR_2	0x43
#define R51x_I2C_R_SID		0x44
#define R51x_I2C_DATA		0x45
#define R51x_I2C_CLOCK		0x46
#define R51x_I2C_TIMEOUT	0x47


#define R511_SI2C_SADDR_3	0x48
#define R511_SI2C_DATA		0x49


#define R51x_SYS_RESET		0x50
		
#define 	OV511_RESET_UDC		0x01
#define 	OV511_RESET_I2C		0x02
#define 	OV511_RESET_FIFO	0x04
#define 	OV511_RESET_OMNICE	0x08
#define 	OV511_RESET_DRAM	0x10
#define 	OV511_RESET_CAM_INT	0x20
#define 	OV511_RESET_OV511	0x40
#define 	OV511_RESET_NOREGS	0x3F 
#define 	OV511_RESET_ALL		0x7F

#define R511_SYS_CLOCK_DIV	0x51
#define R51x_SYS_SNAP		0x52
#define R51x_SYS_INIT         	0x53
#define R511_SYS_PWR_CLK	0x54 
#define R511_SYS_LED_CTL	0x55 
#define R511_SYS_USER		0x5E
#define R511_SYS_CUST_ID	0x5F


#define R511_COMP_PHY		0x70
#define R511_COMP_PHUV		0x71
#define R511_COMP_PVY		0x72
#define R511_COMP_PVUV		0x73
#define R511_COMP_QHY		0x74
#define R511_COMP_QHUV		0x75
#define R511_COMP_QVY		0x76
#define R511_COMP_QVUV		0x77
#define R511_COMP_EN		0x78
#define R511_COMP_LUT_EN	0x79
#define R511_COMP_LUT_BEGIN	0x80






#define OV511_ALT_SIZE_992	0
#define OV511_ALT_SIZE_993	1
#define OV511_ALT_SIZE_768	2
#define OV511_ALT_SIZE_769	3
#define OV511_ALT_SIZE_512	4
#define OV511_ALT_SIZE_513	5
#define OV511_ALT_SIZE_257	6
#define OV511_ALT_SIZE_0	7


#define OV511PLUS_ALT_SIZE_0	0
#define OV511PLUS_ALT_SIZE_33	1
#define OV511PLUS_ALT_SIZE_129	2
#define OV511PLUS_ALT_SIZE_257	3
#define OV511PLUS_ALT_SIZE_385	4
#define OV511PLUS_ALT_SIZE_513	5
#define OV511PLUS_ALT_SIZE_769	6
#define OV511PLUS_ALT_SIZE_961	7


#define OV518_ALT_SIZE_0	0
#define OV518_ALT_SIZE_128	1
#define OV518_ALT_SIZE_256	2
#define OV518_ALT_SIZE_384	3
#define OV518_ALT_SIZE_512	4
#define OV518_ALT_SIZE_640	5
#define OV518_ALT_SIZE_768	6
#define OV518_ALT_SIZE_896	7






#define OV7610_REG_GAIN          0x00	
#define OV7610_REG_BLUE          0x01	
#define OV7610_REG_RED           0x02	
#define OV7610_REG_SAT           0x03	
					
#define OV7610_REG_CNT           0x05	
#define OV7610_REG_BRT           0x06	
					
#define OV7610_REG_BLUE_BIAS     0x0C	
#define OV7610_REG_RED_BIAS      0x0D	
#define OV7610_REG_GAMMA_COEFF   0x0E	
#define OV7610_REG_WB_RANGE      0x0F	
#define OV7610_REG_EXP           0x10	
#define OV7610_REG_CLOCK         0x11	
#define OV7610_REG_COM_A         0x12	
#define OV7610_REG_COM_B         0x13	
#define OV7610_REG_COM_C         0x14	
#define OV7610_REG_COM_D         0x15	
#define OV7610_REG_FIELD_DIVIDE  0x16	
#define OV7610_REG_HWIN_START    0x17	
#define OV7610_REG_HWIN_END      0x18	
#define OV7610_REG_VWIN_START    0x19	
#define OV7610_REG_VWIN_END      0x1A	
#define OV7610_REG_PIXEL_SHIFT   0x1B	
#define OV7610_REG_ID_HIGH       0x1C	
#define OV7610_REG_ID_LOW        0x1D	
					
#define OV7610_REG_COM_E         0x20	
#define OV7610_REG_YOFFSET       0x21	
#define OV7610_REG_UOFFSET       0x22	
					
#define OV7610_REG_ECW           0x24	
#define OV7610_REG_ECB           0x25	
#define OV7610_REG_COM_F         0x26	
#define OV7610_REG_COM_G         0x27	
#define OV7610_REG_COM_H         0x28	
#define OV7610_REG_COM_I         0x29	
#define OV7610_REG_FRAMERATE_H   0x2A	
#define OV7610_REG_FRAMERATE_L   0x2B	
#define OV7610_REG_ALC           0x2C	
#define OV7610_REG_COM_J         0x2D	
#define OV7610_REG_VOFFSET       0x2E	
#define OV7610_REG_ARRAY_BIAS	 0x2F	
					
#define OV7610_REG_YGAMMA        0x33	
#define OV7610_REG_BIAS_ADJUST   0x34	
#define OV7610_REG_COM_L         0x35	
					
#define OV7610_REG_COM_K         0x38	





#define OV7xx0_SID   0x42
#define OV6xx0_SID   0xC0
#define OV8xx0_SID   0xA0
#define KS0127_SID   0xD8
#define SAA7111A_SID 0x48





#define I2C_CLOCK_PRESCALER 	0x03

#define FRAMES_PER_DESC		10	
#define MAX_FRAME_SIZE_PER_DESC	993	
#define PIXELS_PER_SEG		256	

#define OV511_ENDPOINT_ADDRESS	1	

#define OV511_NUMFRAMES	2
#if OV511_NUMFRAMES > VIDEO_MAX_FRAME
	#error "OV511_NUMFRAMES is too high"
#endif

#define OV511_NUMSBUF		2


#define OV511_CBUF_SIZE		4


#define OV511_USB_PATH_LEN	64


enum {
	BRG_UNKNOWN,
	BRG_OV511,
	BRG_OV511PLUS,
	BRG_OV518,
	BRG_OV518PLUS,
};


enum {
	BCL_UNKNOWN,
	BCL_OV511,
	BCL_OV518,
};


enum {
	SEN_UNKNOWN,
	SEN_OV76BE,
	SEN_OV7610,
	SEN_OV7620,
	SEN_OV7620AE,
	SEN_OV6620,
	SEN_OV6630,
	SEN_OV6630AE,
	SEN_OV6630AF,
	SEN_OV8600,
	SEN_KS0127,
	SEN_KS0127B,
	SEN_SAA7111A,
};

enum {
	STATE_SCANNING,		
	STATE_HEADER,		
	STATE_LINES,		
};


enum {
	BUF_NOT_ALLOCATED,
	BUF_ALLOCATED,
};



#define OV511_INTERFACE_VER 101


enum {
	LED_OFF,
	LED_ON,
	LED_AUTO,
};


enum {
	RAWFMT_INVALID,
	RAWFMT_YUV400,
	RAWFMT_YUV420,
	RAWFMT_YUV422,
	RAWFMT_GBR422,
};

struct ov511_i2c_struct {
	unsigned char slave; 
	unsigned char reg;   
	unsigned char value; 
	unsigned char mask;  
};


#define OV511IOC_WI2C     _IOW('v', BASE_VIDIOCPRIVATE + 5, \
			       struct ov511_i2c_struct)
#define OV511IOC_RI2C    _IOWR('v', BASE_VIDIOCPRIVATE + 6, \
			       struct ov511_i2c_struct)


struct usb_ov511;		

struct ov511_sbuf {
	struct usb_ov511 *ov;
	unsigned char *data;
	struct urb *urb;
	spinlock_t lock;
	int n;
};

enum {
	FRAME_UNUSED,		
	FRAME_READY,		
	FRAME_GRABBING,		
	FRAME_DONE,		
	FRAME_ERROR,		
};

struct ov511_regvals {
	enum {
		OV511_DONE_BUS,
		OV511_REG_BUS,
		OV511_I2C_BUS,
	} bus;
	unsigned char reg;
	unsigned char val;
};

struct ov511_frame {
	int framenum;		
	unsigned char *data;	
	unsigned char *tempdata; 
	unsigned char *rawdata;	
	unsigned char *compbuf;	

	int depth;		
	int width;		
	int height;		

	int rawwidth;		
	int rawheight;		

	int sub_flag;		
	unsigned int format;	
	int compressed;		

	volatile int grabstate;	
	int scanstate;		

	int bytes_recvd;	

	long bytes_read;	

	wait_queue_head_t wq;	

	int snapshot;		
};

#define DECOMP_INTERFACE_VER 4


struct ov51x_decomp_ops {
	int (*decomp_400)(unsigned char *, unsigned char *, unsigned char *,
			  int, int, int);
	int (*decomp_420)(unsigned char *, unsigned char *, unsigned char *,
			  int, int, int);
	int (*decomp_422)(unsigned char *, unsigned char *, unsigned char *,
			  int, int, int);
	struct module *owner;
};

struct usb_ov511 {
	struct video_device *vdev;
	struct usb_device *dev;

	int customid;
	char *desc;
	unsigned char iface;
	char usb_path[OV511_USB_PATH_LEN];

	
	int maxwidth;
	int maxheight;
	int minwidth;
	int minheight;

	int brightness;
	int colour;
	int contrast;
	int hue;
	int whiteness;
	int exposure;
	int auto_brt;		
	int auto_gain;		
	int auto_exp;		
	int backlight;		
	int mirror;		

	int led_policy;		

	struct mutex lock;	
	int user;		

	int streaming;		
	int grabbing;		

	int compress;		
	int compress_inited;	

	int lightfreq;		
	int bandfilt;		

	unsigned char *fbuf;	
	unsigned char *tempfbuf; 
	unsigned char *rawfbuf;	

	int sub_flag;		
	int subx;		
	int suby;		
	int subw;		
	int subh;		

	int curframe;		
	struct ov511_frame frame[OV511_NUMFRAMES];

	struct ov511_sbuf sbuf[OV511_NUMSBUF];

	wait_queue_head_t wq;	

	int snap_enabled;	

	int bridge;		
	int bclass;		
	int sensor;		

	int packet_size;	
	int packet_numbering;	

	
	int buf_state;
	struct mutex buf_lock;

	struct ov51x_decomp_ops *decomp_ops;

	
	int stop_during_set;

	int stopped;		

	
	int input;		
	int num_inputs;		
	int norm; 		
	int has_decoder;	
	int pal;		

	
	int nr;			

	
	struct mutex i2c_lock;	  
	unsigned char primary_i2c_slave;  

	
	unsigned char *cbuf;		
	struct mutex cbuf_lock;
};


struct symbolic_list {
	int num;
	char *name;
};

#define NOT_DEFINED_STR "Unknown"


static inline char *
symbolic(struct symbolic_list list[], int num)
{
	int i;

	for (i = 0; list[i].name != NULL; i++)
			if (list[i].num == num)
				return (list[i].name);

	return (NOT_DEFINED_STR);
}



#define OV511_QUANTABLESIZE	64
#define OV518_QUANTABLESIZE	32

#define OV511_YQUANTABLE { \
	0, 1, 1, 2, 2, 3, 3, 4, \
	1, 1, 1, 2, 2, 3, 4, 4, \
	1, 1, 2, 2, 3, 4, 4, 4, \
	2, 2, 2, 3, 4, 4, 4, 4, \
	2, 2, 3, 4, 4, 5, 5, 5, \
	3, 3, 4, 4, 5, 5, 5, 5, \
	3, 4, 4, 4, 5, 5, 5, 5, \
	4, 4, 4, 4, 5, 5, 5, 5  \
}

#define OV511_UVQUANTABLE { \
	0, 2, 2, 3, 4, 4, 4, 4, \
	2, 2, 2, 4, 4, 4, 4, 4, \
	2, 2, 3, 4, 4, 4, 4, 4, \
	3, 4, 4, 4, 4, 4, 4, 4, \
	4, 4, 4, 4, 4, 4, 4, 4, \
	4, 4, 4, 4, 4, 4, 4, 4, \
	4, 4, 4, 4, 4, 4, 4, 4, \
	4, 4, 4, 4, 4, 4, 4, 4  \
}

#define OV518_YQUANTABLE { \
	5, 4, 5, 6, 6, 7, 7, 7, \
	5, 5, 5, 5, 6, 7, 7, 7, \
	6, 6, 6, 6, 7, 7, 7, 8, \
	7, 7, 6, 7, 7, 7, 8, 8  \
}

#define OV518_UVQUANTABLE { \
	6, 6, 6, 7, 7, 7, 7, 7, \
	6, 6, 6, 7, 7, 7, 7, 7, \
	6, 6, 6, 7, 7, 7, 7, 8, \
	7, 7, 7, 7, 7, 7, 8, 8  \
}

#endif
