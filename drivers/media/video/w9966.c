


#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/videodev.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>
#include <linux/parport.h>

				

#ifdef DEBUG
#define DPRINTF(x, a...) printk(KERN_DEBUG "W9966: %s(): "x, __func__ , ##a)
#else
#define DPRINTF(x...)
#endif



#define W9966_DRIVERNAME	"W9966CF Webcam"
#define W9966_MAXCAMS		4	
#define W9966_RBUFFER		2048	
#define W9966_SRAMSIZE		131072	
#define W9966_SRAMID		0x02	


#define W9966_WND_MIN_X		16
#define W9966_WND_MIN_Y		14
#define W9966_WND_MAX_X		705
#define W9966_WND_MAX_Y		253
#define W9966_WND_MAX_W		(W9966_WND_MAX_X - W9966_WND_MIN_X)
#define W9966_WND_MAX_H		(W9966_WND_MAX_Y - W9966_WND_MIN_Y)


#define W9966_STATE_PDEV	0x01
#define W9966_STATE_CLAIMED	0x02
#define W9966_STATE_VDEV	0x04

#define W9966_I2C_W_ID		0x48
#define W9966_I2C_R_ID		0x49
#define W9966_I2C_R_DATA	0x08
#define W9966_I2C_R_CLOCK	0x04
#define W9966_I2C_W_DATA	0x02
#define W9966_I2C_W_CLOCK	0x01

struct w9966_dev {
	unsigned char dev_state;
	unsigned char i2c_state;
	unsigned short ppmode;
	struct parport* pport;
	struct pardevice* pdev;
	struct video_device vdev;
	unsigned short width;
	unsigned short height;
	unsigned char brightness;
	signed char contrast;
	signed char color;
	signed char hue;
	unsigned long in_use;
};



MODULE_AUTHOR("Jakob Kemi <jakob.kemi@post.utfors.se>");
MODULE_DESCRIPTION("Winbond w9966cf WebCam driver (0.32)");
MODULE_LICENSE("GPL");


#ifdef MODULE
static const char* pardev[] = {[0 ... W9966_MAXCAMS] = ""};
#else
static const char* pardev[] = {[0 ... W9966_MAXCAMS] = "aggressive"};
#endif
module_param_array(pardev, charp, NULL, 0);
MODULE_PARM_DESC(pardev, "pardev: where to search for\n\
\teach camera. 'aggressive' means brute-force search.\n\
\tEg: >pardev=parport3,aggressive,parport2,parport1< would assign\n\
\tcam 1 to parport3 and search every parport for cam 2 etc...");

static int parmode;
module_param(parmode, int, 0);
MODULE_PARM_DESC(parmode, "parmode: transfer mode (0=auto, 1=ecp, 2=epp");

static int video_nr = -1;
module_param(video_nr, int, 0);



static struct w9966_dev w9966_cams[W9966_MAXCAMS];



static inline void w9966_setState(struct w9966_dev* cam, int mask, int val);
static inline int  w9966_getState(struct w9966_dev* cam, int mask, int val);
static inline void w9966_pdev_claim(struct w9966_dev *vdev);
static inline void w9966_pdev_release(struct w9966_dev *vdev);

static int w9966_rReg(struct w9966_dev* cam, int reg);
static int w9966_wReg(struct w9966_dev* cam, int reg, int data);
#if 0
static int w9966_rReg_i2c(struct w9966_dev* cam, int reg);
#endif
static int w9966_wReg_i2c(struct w9966_dev* cam, int reg, int data);
static int w9966_findlen(int near, int size, int maxlen);
static int w9966_calcscale(int size, int min, int max, int* beg, int* end, unsigned char* factor);
static int w9966_setup(struct w9966_dev* cam, int x1, int y1, int x2, int y2, int w, int h);

static int  w9966_init(struct w9966_dev* cam, struct parport* port);
static void w9966_term(struct w9966_dev* cam);

static inline void w9966_i2c_setsda(struct w9966_dev* cam, int state);
static inline int  w9966_i2c_setscl(struct w9966_dev* cam, int state);
static inline int  w9966_i2c_getsda(struct w9966_dev* cam);
static inline int  w9966_i2c_getscl(struct w9966_dev* cam);
static int w9966_i2c_wbyte(struct w9966_dev* cam, int data);
#if 0
static int w9966_i2c_rbyte(struct w9966_dev* cam);
#endif

static long w9966_v4l_ioctl(struct file *file,
			   unsigned int cmd, unsigned long arg);
static ssize_t w9966_v4l_read(struct file *file, char __user *buf,
			      size_t count, loff_t *ppos);

static int w9966_exclusive_open(struct file *file)
{
	struct w9966_dev *cam = video_drvdata(file);

	return test_and_set_bit(0, &cam->in_use) ? -EBUSY : 0;
}

static int w9966_exclusive_release(struct file *file)
{
	struct w9966_dev *cam = video_drvdata(file);

	clear_bit(0, &cam->in_use);
	return 0;
}

static const struct v4l2_file_operations w9966_fops = {
	.owner		= THIS_MODULE,
	.open           = w9966_exclusive_open,
	.release        = w9966_exclusive_release,
	.ioctl          = w9966_v4l_ioctl,
	.read           = w9966_v4l_read,
};
static struct video_device w9966_template = {
	.name           = W9966_DRIVERNAME,
	.fops           = &w9966_fops,
	.release 	= video_device_release_empty,
};





static inline void w9966_setState(struct w9966_dev* cam, int mask, int val)
{
	cam->dev_state = (cam->dev_state & ~mask) ^ val;
}


static inline int w9966_getState(struct w9966_dev* cam, int mask, int val)
{
	return ((cam->dev_state & mask) == val);
}


static inline void w9966_pdev_claim(struct w9966_dev* cam)
{
	if (w9966_getState(cam, W9966_STATE_CLAIMED, W9966_STATE_CLAIMED))
		return;
	parport_claim_or_block(cam->pdev);
	w9966_setState(cam, W9966_STATE_CLAIMED, W9966_STATE_CLAIMED);
}


static inline void w9966_pdev_release(struct w9966_dev* cam)
{
	if (w9966_getState(cam, W9966_STATE_CLAIMED, 0))
		return;
	parport_release(cam->pdev);
	w9966_setState(cam, W9966_STATE_CLAIMED, 0);
}




static int w9966_rReg(struct w9966_dev* cam, int reg)
{
	
	const unsigned char addr = 0x80 | (reg & 0x1f);
	unsigned char val;

	if (parport_negotiate(cam->pport, cam->ppmode | IEEE1284_ADDR) != 0)
		return -1;
	if (parport_write(cam->pport, &addr, 1) != 1)
		return -1;
	if (parport_negotiate(cam->pport, cam->ppmode | IEEE1284_DATA) != 0)
		return -1;
	if (parport_read(cam->pport, &val, 1) != 1)
		return -1;

	return val;
}




static int w9966_wReg(struct w9966_dev* cam, int reg, int data)
{
	
	const unsigned char addr = 0xc0 | (reg & 0x1f);
	const unsigned char val = data;

	if (parport_negotiate(cam->pport, cam->ppmode | IEEE1284_ADDR) != 0)
		return -1;
	if (parport_write(cam->pport, &addr, 1) != 1)
		return -1;
	if (parport_negotiate(cam->pport, cam->ppmode | IEEE1284_DATA) != 0)
		return -1;
	if (parport_write(cam->pport, &val, 1) != 1)
		return -1;

	return 0;
}





static int w9966_init(struct w9966_dev* cam, struct parport* port)
{
	if (cam->dev_state != 0)
		return -1;

	cam->pport = port;
	cam->brightness = 128;
	cam->contrast = 64;
	cam->color = 64;
	cam->hue = 0;


	switch(parmode)
	{
	default:	
	case 0:
		if (port->modes & PARPORT_MODE_ECP)
			cam->ppmode = IEEE1284_MODE_ECP;
		else if (port->modes & PARPORT_MODE_EPP)
			cam->ppmode = IEEE1284_MODE_EPP;
		else
			cam->ppmode = IEEE1284_MODE_ECP;
		break;
	case 1:		
		cam->ppmode = IEEE1284_MODE_ECP;
		break;
	case 2:		
		cam->ppmode = IEEE1284_MODE_EPP;
	break;
	}


	cam->pdev = parport_register_device(port, "w9966", NULL, NULL, NULL, 0, NULL);
	if (cam->pdev == NULL) {
		DPRINTF("parport_register_device() failed\n");
		return -1;
	}
	w9966_setState(cam, W9966_STATE_PDEV, W9966_STATE_PDEV);

	w9966_pdev_claim(cam);


	if (w9966_setup(cam, 0, 0, 1023, 1023, 200, 160) != 0) {
		DPRINTF("w9966_setup() failed.\n");
		return -1;
	}

	w9966_pdev_release(cam);


	memcpy(&cam->vdev, &w9966_template, sizeof(struct video_device));
	video_set_drvdata(&cam->vdev, cam);

	if (video_register_device(&cam->vdev, VFL_TYPE_GRABBER, video_nr) < 0)
		return -1;

	w9966_setState(cam, W9966_STATE_VDEV, W9966_STATE_VDEV);

	
	printk(
		"w9966cf: Found and initialized a webcam on %s.\n",
		cam->pport->name
	);
	return 0;
}



static void w9966_term(struct w9966_dev* cam)
{

	if (w9966_getState(cam, W9966_STATE_VDEV, W9966_STATE_VDEV)) {
		video_unregister_device(&cam->vdev);
		w9966_setState(cam, W9966_STATE_VDEV, 0);
	}


	if (w9966_getState(cam, W9966_STATE_PDEV, W9966_STATE_PDEV)) {
		w9966_pdev_claim(cam);
		parport_negotiate(cam->pport, IEEE1284_MODE_COMPAT);
		w9966_pdev_release(cam);
	}


	if (w9966_getState(cam, W9966_STATE_PDEV, W9966_STATE_PDEV)) {
		parport_unregister_device(cam->pdev);
		w9966_setState(cam, W9966_STATE_PDEV, 0);
	}
}





static int w9966_findlen(int near, int size, int maxlen)
{
	int bestlen = size;
	int besterr = abs(near - bestlen);
	int len;

	for(len = size+1;len < maxlen;len++)
	{
		int err;
		if ( ((64*size) %len) != 0)
			continue;

		err = abs(near - len);

		
		if (err > besterr)
			break;

		besterr = err;
		bestlen = len;
	}

	return bestlen;
}




static int w9966_calcscale(int size, int min, int max, int* beg, int* end, unsigned char* factor)
{
	int maxlen = max - min;
	int len = *end - *beg + 1;
	int newlen = w9966_findlen(len, size, maxlen);
	int err = newlen - len;

	
	if (newlen > maxlen || newlen < size)
		return -1;

	
	*factor = (64*size) / newlen;
	if (*factor == 64)
		*factor = 0x00;	
	else
		*factor |= 0x80; 

	
	*beg -= err / 2;
	*end += err - (err / 2);

	
	if (*beg < min) {
		*end += min - *beg;
		*beg += min - *beg;
	}
	if (*end > max) {
		*beg -= *end - max;
		*end -= *end - max;
	}

	return 0;
}




static int w9966_setup(struct w9966_dev* cam, int x1, int y1, int x2, int y2, int w, int h)
{
	unsigned int i;
	unsigned int enh_s, enh_e;
	unsigned char scale_x, scale_y;
	unsigned char regs[0x1c];
	unsigned char saa7111_regs[] = {
		0x21, 0x00, 0xd8, 0x23, 0x00, 0x80, 0x80, 0x00,
		0x88, 0x10, 0x80, 0x40, 0x40, 0x00, 0x01, 0x00,
		0x48, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x71, 0xe7, 0x00, 0x00, 0xc0
	};


	if (w*h*2 > W9966_SRAMSIZE)
	{
		DPRINTF("capture window exceeds SRAM size!.\n");
		w = 200; h = 160;	
	}

	w &= ~0x1;
	if (w < 2) w = 2;
	if (h < 1) h = 1;
	if (w > W9966_WND_MAX_W) w = W9966_WND_MAX_W;
	if (h > W9966_WND_MAX_H) h = W9966_WND_MAX_H;

	cam->width = w;
	cam->height = h;

	enh_s = 0;
	enh_e = w*h*2;


	if (
		w9966_calcscale(w, W9966_WND_MIN_X, W9966_WND_MAX_X, &x1, &x2, &scale_x) != 0 ||
		w9966_calcscale(h, W9966_WND_MIN_Y, W9966_WND_MAX_Y, &y1, &y2, &scale_y) != 0
	) return -1;

	DPRINTF(
		"%dx%d, x: %d<->%d, y: %d<->%d, sx: %d/64, sy: %d/64.\n",
		w, h, x1, x2, y1, y2, scale_x&~0x80, scale_y&~0x80
	);


	regs[0x00] = 0x00;			
	regs[0x01] = 0x18;			
	regs[0x02] = scale_y;			
	regs[0x03] = scale_x;			

	
	regs[0x04] = (x1 & 0x0ff);		
	regs[0x05] = (x1 & 0x300)>>8;		
	regs[0x06] = (y1 & 0x0ff);		
	regs[0x07] = (y1 & 0x300)>>8;		
	regs[0x08] = (x2 & 0x0ff);		
	regs[0x09] = (x2 & 0x300)>>8;		
	regs[0x0a] = (y2 & 0x0ff);		

	regs[0x0c] = W9966_SRAMID;		

	
	regs[0x0d] = (enh_s& 0x000ff);		
	regs[0x0e] = (enh_s& 0x0ff00)>>8;	
	regs[0x0f] = (enh_s& 0x70000)>>16;	
	regs[0x10] = (enh_e& 0x000ff);		
	regs[0x11] = (enh_e& 0x0ff00)>>8;	
	regs[0x12] = (enh_e& 0x70000)>>16;	

	
	regs[0x13] = 0x40;			
	regs[0x17] = 0x00;			
	regs[0x18] = cam->i2c_state = 0x00;	
	regs[0x19] = 0xff;			
	regs[0x1a] = 0xff;			
	regs[0x1b] = 0x10;			

	
	saa7111_regs[0x0a] = cam->brightness;
	saa7111_regs[0x0b] = cam->contrast;
	saa7111_regs[0x0c] = cam->color;
	saa7111_regs[0x0d] = cam->hue;


	if (w9966_wReg(cam, 0x00, 0x03) == -1)
		return -1;


	for (i = 0; i < 0x1c; i++)
		if (w9966_wReg(cam, i, regs[i]) == -1)
			return -1;


	for (i = 0; i < 0x20; i++)
		if (w9966_wReg_i2c(cam, i, saa7111_regs[i]) == -1)
			return -1;

	return 0;
}





static inline void w9966_i2c_setsda(struct w9966_dev* cam, int state)
{
	if (state)
		cam->i2c_state |= W9966_I2C_W_DATA;
	else
		cam->i2c_state &= ~W9966_I2C_W_DATA;

	w9966_wReg(cam, 0x18, cam->i2c_state);
	udelay(5);
}



static inline int w9966_i2c_getscl(struct w9966_dev* cam)
{
	const unsigned char state = w9966_rReg(cam, 0x18);
	return ((state & W9966_I2C_R_CLOCK) > 0);
}



static inline int w9966_i2c_setscl(struct w9966_dev* cam, int state)
{
	unsigned long timeout;

	if (state)
		cam->i2c_state |= W9966_I2C_W_CLOCK;
	else
		cam->i2c_state &= ~W9966_I2C_W_CLOCK;

	w9966_wReg(cam, 0x18, cam->i2c_state);
	udelay(5);

	
	if (state) {
		timeout = jiffies + 100;
		while (!w9966_i2c_getscl(cam)) {
			if (time_after(jiffies, timeout))
				return -1;
		}
	}
	return 0;
}



static inline int w9966_i2c_getsda(struct w9966_dev* cam)
{
	const unsigned char state = w9966_rReg(cam, 0x18);
	return ((state & W9966_I2C_R_DATA) > 0);
}



static int w9966_i2c_wbyte(struct w9966_dev* cam, int data)
{
	int i;
	for (i = 7; i >= 0; i--)
	{
		w9966_i2c_setsda(cam, (data >> i) & 0x01);

		if (w9966_i2c_setscl(cam, 1) == -1)
			return -1;
		w9966_i2c_setscl(cam, 0);
	}

	w9966_i2c_setsda(cam, 1);

	if (w9966_i2c_setscl(cam, 1) == -1)
		return -1;
	w9966_i2c_setscl(cam, 0);

	return 0;
}



#if 0
static int w9966_i2c_rbyte(struct w9966_dev* cam)
{
	unsigned char data = 0x00;
	int i;

	w9966_i2c_setsda(cam, 1);

	for (i = 0; i < 8; i++)
	{
		if (w9966_i2c_setscl(cam, 1) == -1)
			return -1;
		data = data << 1;
		if (w9966_i2c_getsda(cam))
			data |= 0x01;

		w9966_i2c_setscl(cam, 0);
	}
	return data;
}
#endif



#if 0
static int w9966_rReg_i2c(struct w9966_dev* cam, int reg)
{
	int data;

	w9966_i2c_setsda(cam, 0);
	w9966_i2c_setscl(cam, 0);

	if (
		w9966_i2c_wbyte(cam, W9966_I2C_W_ID) == -1 ||
		w9966_i2c_wbyte(cam, reg) == -1
	)
		return -1;

	w9966_i2c_setsda(cam, 1);
	if (w9966_i2c_setscl(cam, 1) == -1)
		return -1;
	w9966_i2c_setsda(cam, 0);
	w9966_i2c_setscl(cam, 0);

	if (
		w9966_i2c_wbyte(cam, W9966_I2C_R_ID) == -1 ||
		(data = w9966_i2c_rbyte(cam)) == -1
	)
		return -1;

	w9966_i2c_setsda(cam, 0);

	if (w9966_i2c_setscl(cam, 1) == -1)
		return -1;
	w9966_i2c_setsda(cam, 1);

	return data;
}
#endif



static int w9966_wReg_i2c(struct w9966_dev* cam, int reg, int data)
{
	w9966_i2c_setsda(cam, 0);
	w9966_i2c_setscl(cam, 0);

	if (
		w9966_i2c_wbyte(cam, W9966_I2C_W_ID) == -1 ||
		w9966_i2c_wbyte(cam, reg) == -1 ||
		w9966_i2c_wbyte(cam, data) == -1
	)
		return -1;

	w9966_i2c_setsda(cam, 0);
	if (w9966_i2c_setscl(cam, 1) == -1)
		return -1;

	w9966_i2c_setsda(cam, 1);

	return 0;
}



static long w9966_v4l_do_ioctl(struct file *file, unsigned int cmd, void *arg)
{
	struct w9966_dev *cam = video_drvdata(file);

	switch(cmd)
	{
	case VIDIOCGCAP:
	{
		static struct video_capability vcap = {
			.name      = W9966_DRIVERNAME,
			.type      = VID_TYPE_CAPTURE | VID_TYPE_SCALES,
			.channels  = 1,
			.maxwidth  = W9966_WND_MAX_W,
			.maxheight = W9966_WND_MAX_H,
			.minwidth  = 2,
			.minheight = 1,
		};
		struct video_capability *cap = arg;
		*cap = vcap;
		return 0;
	}
	case VIDIOCGCHAN:
	{
		struct video_channel *vch = arg;
		if(vch->channel != 0)	
			return -EINVAL;
		memset(vch,0,sizeof(*vch));
		strcpy(vch->name, "CCD-input");
		vch->type = VIDEO_TYPE_CAMERA;
		return 0;
	}
	case VIDIOCSCHAN:
	{
		struct video_channel *vch = arg;
		if(vch->channel != 0)
			return -EINVAL;
		return 0;
	}
	case VIDIOCGTUNER:
	{
		struct video_tuner *vtune = arg;
		if(vtune->tuner != 0)
			return -EINVAL;
		strcpy(vtune->name, "no tuner");
		vtune->rangelow = 0;
		vtune->rangehigh = 0;
		vtune->flags = VIDEO_TUNER_NORM;
		vtune->mode = VIDEO_MODE_AUTO;
		vtune->signal = 0xffff;
		return 0;
	}
	case VIDIOCSTUNER:
	{
		struct video_tuner *vtune = arg;
		if (vtune->tuner != 0)
			return -EINVAL;
		if (vtune->mode != VIDEO_MODE_AUTO)
			return -EINVAL;
		return 0;
	}
	case VIDIOCGPICT:
	{
		struct video_picture vpic = {
			cam->brightness << 8,	
			(cam->hue + 128) << 8,	
			cam->color << 9,	
			cam->contrast << 9,	
			0x8000,			
			16, VIDEO_PALETTE_YUV422
		};
		struct video_picture *pic = arg;
		*pic = vpic;
		return 0;
	}
	case VIDIOCSPICT:
	{
		struct video_picture *vpic = arg;
		if (vpic->depth != 16 || (vpic->palette != VIDEO_PALETTE_YUV422 && vpic->palette != VIDEO_PALETTE_YUYV))
			return -EINVAL;

		cam->brightness = vpic->brightness >> 8;
		cam->hue = (vpic->hue >> 8) - 128;
		cam->color = vpic->colour >> 9;
		cam->contrast = vpic->contrast >> 9;

		w9966_pdev_claim(cam);

		if (
			w9966_wReg_i2c(cam, 0x0a, cam->brightness) == -1 ||
			w9966_wReg_i2c(cam, 0x0b, cam->contrast) == -1 ||
			w9966_wReg_i2c(cam, 0x0c, cam->color) == -1 ||
			w9966_wReg_i2c(cam, 0x0d, cam->hue) == -1
		) {
			w9966_pdev_release(cam);
			return -EIO;
		}

		w9966_pdev_release(cam);
		return 0;
	}
	case VIDIOCSWIN:
	{
		int ret;
		struct video_window *vwin = arg;

		if (vwin->flags != 0)
			return -EINVAL;
		if (vwin->clipcount != 0)
			return -EINVAL;
		if (vwin->width < 2 || vwin->width > W9966_WND_MAX_W)
			return -EINVAL;
		if (vwin->height < 1 || vwin->height > W9966_WND_MAX_H)
			return -EINVAL;

		
		w9966_pdev_claim(cam);
		ret = w9966_setup(cam, 0, 0, 1023, 1023, vwin->width, vwin->height);
		w9966_pdev_release(cam);

		if (ret != 0) {
			DPRINTF("VIDIOCSWIN: w9966_setup() failed.\n");
			return -EIO;
		}

		return 0;
	}
	case VIDIOCGWIN:
	{
		struct video_window *vwin = arg;
		memset(vwin, 0, sizeof(*vwin));
		vwin->width = cam->width;
		vwin->height = cam->height;
		return 0;
	}
	
	case VIDIOCCAPTURE:
	case VIDIOCGFBUF:
	case VIDIOCSFBUF:
	case VIDIOCKEY:
	case VIDIOCGFREQ:
	case VIDIOCSFREQ:
	case VIDIOCGAUDIO:
	case VIDIOCSAUDIO:
		return -EINVAL;
	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

static long w9966_v4l_ioctl(struct file *file,
			   unsigned int cmd, unsigned long arg)
{
	return video_usercopy(file, cmd, arg, w9966_v4l_do_ioctl);
}


static ssize_t w9966_v4l_read(struct file *file, char  __user *buf,
			      size_t count, loff_t *ppos)
{
	struct w9966_dev *cam = video_drvdata(file);
	unsigned char addr = 0xa0;	
	unsigned char __user *dest = (unsigned char __user *)buf;
	unsigned long dleft = count;
	unsigned char *tbuf;

	
	if (count > cam->width * cam->height * 2)
		return -EINVAL;

	w9966_pdev_claim(cam);
	w9966_wReg(cam, 0x00, 0x02);	
	w9966_wReg(cam, 0x00, 0x00);	
	w9966_wReg(cam, 0x01, 0x98);	

	
	if (
		(parport_negotiate(cam->pport, cam->ppmode|IEEE1284_ADDR) != 0	)||
		(parport_write(cam->pport, &addr, 1) != 1						)||
		(parport_negotiate(cam->pport, cam->ppmode|IEEE1284_DATA) != 0	)
	) {
		w9966_pdev_release(cam);
		return -EFAULT;
	}

	tbuf = kmalloc(W9966_RBUFFER, GFP_KERNEL);
	if (tbuf == NULL) {
		count = -ENOMEM;
		goto out;
	}

	while(dleft > 0)
	{
		unsigned long tsize = (dleft > W9966_RBUFFER) ? W9966_RBUFFER : dleft;

		if (parport_read(cam->pport, tbuf, tsize) < tsize) {
			count = -EFAULT;
			goto out;
		}
		if (copy_to_user(dest, tbuf, tsize) != 0) {
			count = -EFAULT;
			goto out;
		}
		dest += tsize;
		dleft -= tsize;
	}

	w9966_wReg(cam, 0x01, 0x18);	

out:
	kfree(tbuf);
	w9966_pdev_release(cam);

	return count;
}



static void w9966_attach(struct parport *port)
{
	int i;

	for (i = 0; i < W9966_MAXCAMS; i++)
	{
		if (w9966_cams[i].dev_state != 0)	
			continue;
		if (
			strcmp(pardev[i], "aggressive") == 0 ||
			strcmp(pardev[i], port->name) == 0
		) {
			if (w9966_init(&w9966_cams[i], port) != 0)
			w9966_term(&w9966_cams[i]);
			break;	
		}
	}
}


static void w9966_detach(struct parport *port)
{
	int i;
	for (i = 0; i < W9966_MAXCAMS; i++)
	if (w9966_cams[i].dev_state != 0 && w9966_cams[i].pport == port)
		w9966_term(&w9966_cams[i]);
}


static struct parport_driver w9966_ppd = {
	.name = W9966_DRIVERNAME,
	.attach = w9966_attach,
	.detach = w9966_detach,
};


static int __init w9966_mod_init(void)
{
	int i;
	for (i = 0; i < W9966_MAXCAMS; i++)
		w9966_cams[i].dev_state = 0;

	return parport_register_driver(&w9966_ppd);
}


static void __exit w9966_mod_term(void)
{
	parport_unregister_driver(&w9966_ppd);
}

module_init(w9966_mod_init);
module_exit(w9966_mod_term);
