

#include <linux/module.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/pagemap.h>
#include <asm/processor.h>
#include <linux/mm.h>
#include <linux/device.h>

#if defined (__i386__)
	#include <asm/cpufeature.h>
#endif

#include "ov511.h"


#define DRIVER_VERSION "v1.64 for Linux 2.5"
#define EMAIL "mark@alpha.dyndns.org"
#define DRIVER_AUTHOR "Mark McClelland <mark@alpha.dyndns.org> & Bret Wallach \
	& Orion Sky Lawlor <olawlor@acm.org> & Kevin Moore & Charl P. Botha \
	<cpbotha@ieee.org> & Claudio Matsuoka <claudio@conectiva.com>"
#define DRIVER_DESC "ov511 USB Camera Driver"

#define OV511_I2C_RETRIES 3
#define ENABLE_Y_QUANTABLE 1
#define ENABLE_UV_QUANTABLE 1

#define OV511_MAX_UNIT_VIDEO 16


#define MAX_FRAME_SIZE(w, h) ((w) * (h) * 3 / 2)

#define MAX_DATA_SIZE(w, h) (MAX_FRAME_SIZE(w, h) + sizeof(struct timeval))


#define MAX_RAW_DATA_SIZE(w, h) ((w) * (h) * 3 / 2 + 1024)

#define FATAL_ERROR(rc) ((rc) < 0 && (rc) != -EPERM)




static int autobright		= 1;
static int autogain		= 1;
static int autoexp		= 1;
static int debug;
static int snapshot;
static int cams			= 1;
static int compress;
static int testpat;
static int dumppix;
static int led 			= 1;
static int dump_bridge;
static int dump_sensor;
static int printph;
static int phy			= 0x1f;
static int phuv			= 0x05;
static int pvy			= 0x06;
static int pvuv			= 0x06;
static int qhy			= 0x14;
static int qhuv			= 0x03;
static int qvy			= 0x04;
static int qvuv			= 0x04;
static int lightfreq;
static int bandingfilter;
static int clockdiv		= -1;
static int packetsize		= -1;
static int framedrop		= -1;
static int fastset;
static int force_palette;
static int backlight;

static unsigned long ov511_devused;
static int unit_video[OV511_MAX_UNIT_VIDEO];
static int remove_zeros;
static int mirror;
static int ov518_color;

module_param(autobright, int, 0);
MODULE_PARM_DESC(autobright, "Sensor automatically changes brightness");
module_param(autogain, int, 0);
MODULE_PARM_DESC(autogain, "Sensor automatically changes gain");
module_param(autoexp, int, 0);
MODULE_PARM_DESC(autoexp, "Sensor automatically changes exposure");
module_param(debug, int, 0);
MODULE_PARM_DESC(debug,
  "Debug level: 0=none, 1=inits, 2=warning, 3=config, 4=functions, 5=max");
module_param(snapshot, int, 0);
MODULE_PARM_DESC(snapshot, "Enable snapshot mode");
module_param(cams, int, 0);
MODULE_PARM_DESC(cams, "Number of simultaneous cameras");
module_param(compress, int, 0);
MODULE_PARM_DESC(compress, "Turn on compression");
module_param(testpat, int, 0);
MODULE_PARM_DESC(testpat,
  "Replace image with vertical bar testpattern (only partially working)");
module_param(dumppix, int, 0);
MODULE_PARM_DESC(dumppix, "Dump raw pixel data");
module_param(led, int, 0);
MODULE_PARM_DESC(led,
  "LED policy (OV511+ or later). 0=off, 1=on (default), 2=auto (on when open)");
module_param(dump_bridge, int, 0);
MODULE_PARM_DESC(dump_bridge, "Dump the bridge registers");
module_param(dump_sensor, int, 0);
MODULE_PARM_DESC(dump_sensor, "Dump the sensor registers");
module_param(printph, int, 0);
MODULE_PARM_DESC(printph, "Print frame start/end headers");
module_param(phy, int, 0);
MODULE_PARM_DESC(phy, "Prediction range (horiz. Y)");
module_param(phuv, int, 0);
MODULE_PARM_DESC(phuv, "Prediction range (horiz. UV)");
module_param(pvy, int, 0);
MODULE_PARM_DESC(pvy, "Prediction range (vert. Y)");
module_param(pvuv, int, 0);
MODULE_PARM_DESC(pvuv, "Prediction range (vert. UV)");
module_param(qhy, int, 0);
MODULE_PARM_DESC(qhy, "Quantization threshold (horiz. Y)");
module_param(qhuv, int, 0);
MODULE_PARM_DESC(qhuv, "Quantization threshold (horiz. UV)");
module_param(qvy, int, 0);
MODULE_PARM_DESC(qvy, "Quantization threshold (vert. Y)");
module_param(qvuv, int, 0);
MODULE_PARM_DESC(qvuv, "Quantization threshold (vert. UV)");
module_param(lightfreq, int, 0);
MODULE_PARM_DESC(lightfreq,
  "Light frequency. Set to 50 or 60 Hz, or zero for default settings");
module_param(bandingfilter, int, 0);
MODULE_PARM_DESC(bandingfilter,
  "Enable banding filter (to reduce effects of fluorescent lighting)");
module_param(clockdiv, int, 0);
MODULE_PARM_DESC(clockdiv, "Force pixel clock divisor to a specific value");
module_param(packetsize, int, 0);
MODULE_PARM_DESC(packetsize, "Force a specific isoc packet size");
module_param(framedrop, int, 0);
MODULE_PARM_DESC(framedrop, "Force a specific frame drop register setting");
module_param(fastset, int, 0);
MODULE_PARM_DESC(fastset, "Allows picture settings to take effect immediately");
module_param(force_palette, int, 0);
MODULE_PARM_DESC(force_palette, "Force the palette to a specific value");
module_param(backlight, int, 0);
MODULE_PARM_DESC(backlight, "For objects that are lit from behind");
static unsigned int num_uv;
module_param_array(unit_video, int, &num_uv, 0);
MODULE_PARM_DESC(unit_video,
  "Force use of specific minor number(s). 0 is not allowed.");
module_param(remove_zeros, int, 0);
MODULE_PARM_DESC(remove_zeros,
  "Remove zero-padding from uncompressed incoming data");
module_param(mirror, int, 0);
MODULE_PARM_DESC(mirror, "Reverse image horizontally");
module_param(ov518_color, int, 0);
MODULE_PARM_DESC(ov518_color, "Enable OV518 color (experimental)");

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");



static struct usb_driver ov511_driver;


static const int i2c_detect_tries = 5;

static struct usb_device_id device_table [] = {
	{ USB_DEVICE(VEND_OMNIVISION, PROD_OV511) },
	{ USB_DEVICE(VEND_OMNIVISION, PROD_OV511PLUS) },
	{ USB_DEVICE(VEND_MATTEL, PROD_ME2CAM) },
	{ }  
};

MODULE_DEVICE_TABLE (usb, device_table);

static unsigned char yQuanTable511[] = OV511_YQUANTABLE;
static unsigned char uvQuanTable511[] = OV511_UVQUANTABLE;
static unsigned char yQuanTable518[] = OV518_YQUANTABLE;
static unsigned char uvQuanTable518[] = OV518_UVQUANTABLE;




static struct symbolic_list camlist[] = {
	{   0, "Generic Camera (no ID)" },
	{   1, "Mustek WCam 3X" },
	{   3, "D-Link DSB-C300" },
	{   4, "Generic OV511/OV7610" },
	{   5, "Puretek PT-6007" },
	{   6, "Lifeview USB Life TV (NTSC)" },
	{  21, "Creative Labs WebCam 3" },
	{  22, "Lifeview USB Life TV (PAL D/K+B/G)" },
	{  36, "Koala-Cam" },
	{  38, "Lifeview USB Life TV (PAL)" },
	{  41, "Samsung Anycam MPC-M10" },
	{  43, "Mtekvision Zeca MV402" },
	{  46, "Suma eON" },
	{  70, "Lifeview USB Life TV (PAL/SECAM)" },
	{ 100, "Lifeview RoboCam" },
	{ 102, "AverMedia InterCam Elite" },
	{ 112, "MediaForte MV300" },	
	{ 134, "Ezonics EZCam II" },
	{ 192, "Webeye 2000B" },
	{ 253, "Alpha Vision Tech. AlphaCam SE" },
	{  -1, NULL }
};


static struct symbolic_list v4l1_plist[] = {
	{ VIDEO_PALETTE_GREY,	"GREY" },
	{ VIDEO_PALETTE_HI240,	"HI240" },
	{ VIDEO_PALETTE_RGB565,	"RGB565" },
	{ VIDEO_PALETTE_RGB24,	"RGB24" },
	{ VIDEO_PALETTE_RGB32,	"RGB32" },
	{ VIDEO_PALETTE_RGB555,	"RGB555" },
	{ VIDEO_PALETTE_YUV422,	"YUV422" },
	{ VIDEO_PALETTE_YUYV,	"YUYV" },
	{ VIDEO_PALETTE_UYVY,	"UYVY" },
	{ VIDEO_PALETTE_YUV420,	"YUV420" },
	{ VIDEO_PALETTE_YUV411,	"YUV411" },
	{ VIDEO_PALETTE_RAW,	"RAW" },
	{ VIDEO_PALETTE_YUV422P,"YUV422P" },
	{ VIDEO_PALETTE_YUV411P,"YUV411P" },
	{ VIDEO_PALETTE_YUV420P,"YUV420P" },
	{ VIDEO_PALETTE_YUV410P,"YUV410P" },
	{ -1, NULL }
};

static struct symbolic_list brglist[] = {
	{ BRG_OV511,		"OV511" },
	{ BRG_OV511PLUS,	"OV511+" },
	{ BRG_OV518,		"OV518" },
	{ BRG_OV518PLUS,	"OV518+" },
	{ -1, NULL }
};

static struct symbolic_list senlist[] = {
	{ SEN_OV76BE,	"OV76BE" },
	{ SEN_OV7610,	"OV7610" },
	{ SEN_OV7620,	"OV7620" },
	{ SEN_OV7620AE,	"OV7620AE" },
	{ SEN_OV6620,	"OV6620" },
	{ SEN_OV6630,	"OV6630" },
	{ SEN_OV6630AE,	"OV6630AE" },
	{ SEN_OV6630AF,	"OV6630AF" },
	{ SEN_OV8600,	"OV8600" },
	{ SEN_KS0127,	"KS0127" },
	{ SEN_KS0127B,	"KS0127B" },
	{ SEN_SAA7111A,	"SAA7111A" },
	{ -1, NULL }
};


static struct symbolic_list urb_errlist[] = {
	{ -ENOSR,	"Buffer error (overrun)" },
	{ -EPIPE,	"Stalled (device not responding)" },
	{ -EOVERFLOW,	"Babble (device sends too much data)" },
	{ -EPROTO,	"Bit-stuff error (bad cable?)" },
	{ -EILSEQ,	"CRC/Timeout (bad cable?)" },
	{ -ETIME,	"Device does not respond to token" },
	{ -ETIMEDOUT,	"Device does not respond to command" },
	{ -1, NULL }
};


static void *
rvmalloc(unsigned long size)
{
	void *mem;
	unsigned long adr;

	size = PAGE_ALIGN(size);
	mem = vmalloc_32(size);
	if (!mem)
		return NULL;

	memset(mem, 0, size); 
	adr = (unsigned long) mem;
	while (size > 0) {
		SetPageReserved(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}

	return mem;
}

static void
rvfree(void *mem, unsigned long size)
{
	unsigned long adr;

	if (!mem)
		return;

	adr = (unsigned long) mem;
	while ((long) size > 0) {
		ClearPageReserved(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
	vfree(mem);
}




static int
reg_w(struct usb_ov511 *ov, unsigned char reg, unsigned char value)
{
	int rc;

	PDEBUG(5, "0x%02X:0x%02X", reg, value);

	mutex_lock(&ov->cbuf_lock);
	ov->cbuf[0] = value;
	rc = usb_control_msg(ov->dev,
			     usb_sndctrlpipe(ov->dev, 0),
			     (ov->bclass == BCL_OV518)?1:2 ,
			     USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			     0, (__u16)reg, &ov->cbuf[0], 1, 1000);
	mutex_unlock(&ov->cbuf_lock);

	if (rc < 0)
		err("reg write: error %d: %s", rc, symbolic(urb_errlist, rc));

	return rc;
}



static int
reg_r(struct usb_ov511 *ov, unsigned char reg)
{
	int rc;

	mutex_lock(&ov->cbuf_lock);
	rc = usb_control_msg(ov->dev,
			     usb_rcvctrlpipe(ov->dev, 0),
			     (ov->bclass == BCL_OV518)?1:3 ,
			     USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			     0, (__u16)reg, &ov->cbuf[0], 1, 1000);

	if (rc < 0) {
		err("reg read: error %d: %s", rc, symbolic(urb_errlist, rc));
	} else {
		rc = ov->cbuf[0];
		PDEBUG(5, "0x%02X:0x%02X", reg, ov->cbuf[0]);
	}

	mutex_unlock(&ov->cbuf_lock);

	return rc;
}


static int
reg_w_mask(struct usb_ov511 *ov,
	   unsigned char reg,
	   unsigned char value,
	   unsigned char mask)
{
	int ret;
	unsigned char oldval, newval;

	ret = reg_r(ov, reg);
	if (ret < 0)
		return ret;

	oldval = (unsigned char) ret;
	oldval &= (~mask);		
	value &= mask;			
	newval = oldval | value;	

	return (reg_w(ov, reg, newval));
}


static int
ov518_reg_w32(struct usb_ov511 *ov, unsigned char reg, u32 val, int n)
{
	int rc;

	PDEBUG(5, "0x%02X:%7d, n=%d", reg, val, n);

	mutex_lock(&ov->cbuf_lock);

	*((__le32 *)ov->cbuf) = __cpu_to_le32(val);

	rc = usb_control_msg(ov->dev,
			     usb_sndctrlpipe(ov->dev, 0),
			     1 ,
			     USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			     0, (__u16)reg, ov->cbuf, n, 1000);
	mutex_unlock(&ov->cbuf_lock);

	if (rc < 0)
		err("reg write multiple: error %d: %s", rc,
		    symbolic(urb_errlist, rc));

	return rc;
}

static int
ov511_upload_quan_tables(struct usb_ov511 *ov)
{
	unsigned char *pYTable = yQuanTable511;
	unsigned char *pUVTable = uvQuanTable511;
	unsigned char val0, val1;
	int i, rc, reg = R511_COMP_LUT_BEGIN;

	PDEBUG(4, "Uploading quantization tables");

	for (i = 0; i < OV511_QUANTABLESIZE / 2; i++) {
		if (ENABLE_Y_QUANTABLE)	{
			val0 = *pYTable++;
			val1 = *pYTable++;
			val0 &= 0x0f;
			val1 &= 0x0f;
			val0 |= val1 << 4;
			rc = reg_w(ov, reg, val0);
			if (rc < 0)
				return rc;
		}

		if (ENABLE_UV_QUANTABLE) {
			val0 = *pUVTable++;
			val1 = *pUVTable++;
			val0 &= 0x0f;
			val1 &= 0x0f;
			val0 |= val1 << 4;
			rc = reg_w(ov, reg + OV511_QUANTABLESIZE/2, val0);
			if (rc < 0)
				return rc;
		}

		reg++;
	}

	return 0;
}


static int
ov518_upload_quan_tables(struct usb_ov511 *ov)
{
	unsigned char *pYTable = yQuanTable518;
	unsigned char *pUVTable = uvQuanTable518;
	unsigned char val0, val1;
	int i, rc, reg = R511_COMP_LUT_BEGIN;

	PDEBUG(4, "Uploading quantization tables");

	for (i = 0; i < OV518_QUANTABLESIZE / 2; i++) {
		if (ENABLE_Y_QUANTABLE) {
			val0 = *pYTable++;
			val1 = *pYTable++;
			val0 &= 0x0f;
			val1 &= 0x0f;
			val0 |= val1 << 4;
			rc = reg_w(ov, reg, val0);
			if (rc < 0)
				return rc;
		}

		if (ENABLE_UV_QUANTABLE) {
			val0 = *pUVTable++;
			val1 = *pUVTable++;
			val0 &= 0x0f;
			val1 &= 0x0f;
			val0 |= val1 << 4;
			rc = reg_w(ov, reg + OV518_QUANTABLESIZE/2, val0);
			if (rc < 0)
				return rc;
		}

		reg++;
	}

	return 0;
}

static int
ov51x_reset(struct usb_ov511 *ov, unsigned char reset_type)
{
	int rc;

	
	if (ov->bclass == BCL_OV518)
		reset_type &= 0xfe;

	PDEBUG(4, "Reset: type=0x%02X", reset_type);

	rc = reg_w(ov, R51x_SYS_RESET, reset_type);
	rc = reg_w(ov, R51x_SYS_RESET, 0);

	if (rc < 0)
		err("reset: command failed");

	return rc;
}




static int
ov518_i2c_write_internal(struct usb_ov511 *ov,
			 unsigned char reg,
			 unsigned char value)
{
	int rc;

	PDEBUG(5, "0x%02X:0x%02X", reg, value);

	
	rc = reg_w(ov, R51x_I2C_SADDR_3, reg);
	if (rc < 0)
		return rc;

	
	rc = reg_w(ov, R51x_I2C_DATA, value);
	if (rc < 0)
		return rc;

	
	rc = reg_w(ov, R518_I2C_CTL, 0x01);
	if (rc < 0)
		return rc;

	return 0;
}


static int
ov511_i2c_write_internal(struct usb_ov511 *ov,
			 unsigned char reg,
			 unsigned char value)
{
	int rc, retries;

	PDEBUG(5, "0x%02X:0x%02X", reg, value);

	
	for (retries = OV511_I2C_RETRIES; ; ) {
		
		rc = reg_w(ov, R51x_I2C_SADDR_3, reg);
		if (rc < 0)
			break;

		
		rc = reg_w(ov, R51x_I2C_DATA, value);
		if (rc < 0)
			break;

		
		rc = reg_w(ov, R511_I2C_CTL, 0x01);
		if (rc < 0)
			break;

		
		do {
			rc = reg_r(ov, R511_I2C_CTL);
		} while (rc > 0 && ((rc&1) == 0));
		if (rc < 0)
			break;

		
		if ((rc&2) == 0) {
			rc = 0;
			break;
		}
#if 0
		
		reg_w(ov, R511_I2C_CTL, 0x10);
#endif
		if (--retries < 0) {
			err("i2c write retries exhausted");
			rc = -1;
			break;
		}
	}

	return rc;
}


static int
ov518_i2c_read_internal(struct usb_ov511 *ov, unsigned char reg)
{
	int rc, value;

	
	rc = reg_w(ov, R51x_I2C_SADDR_2, reg);
	if (rc < 0)
		return rc;

	
	rc = reg_w(ov, R518_I2C_CTL, 0x03);
	if (rc < 0)
		return rc;

	
	rc = reg_w(ov, R518_I2C_CTL, 0x05);
	if (rc < 0)
		return rc;

	value = reg_r(ov, R51x_I2C_DATA);

	PDEBUG(5, "0x%02X:0x%02X", reg, value);

	return value;
}


static int
ov511_i2c_read_internal(struct usb_ov511 *ov, unsigned char reg)
{
	int rc, value, retries;

	
	for (retries = OV511_I2C_RETRIES; ; ) {
		
		rc = reg_w(ov, R51x_I2C_SADDR_2, reg);
		if (rc < 0)
			return rc;

		
		rc = reg_w(ov, R511_I2C_CTL, 0x03);
		if (rc < 0)
			return rc;

		
		do {
			rc = reg_r(ov, R511_I2C_CTL);
		} while (rc > 0 && ((rc & 1) == 0));
		if (rc < 0)
			return rc;

		if ((rc&2) == 0) 
			break;

		
		reg_w(ov, R511_I2C_CTL, 0x10);

		if (--retries < 0) {
			err("i2c write retries exhausted");
			return -1;
		}
	}

	
	for (retries = OV511_I2C_RETRIES; ; ) {
		
		rc = reg_w(ov, R511_I2C_CTL, 0x05);
		if (rc < 0)
			return rc;

		
		do {
			rc = reg_r(ov, R511_I2C_CTL);
		} while (rc > 0 && ((rc&1) == 0));
		if (rc < 0)
			return rc;

		if ((rc&2) == 0) 
			break;

		
		rc = reg_w(ov, R511_I2C_CTL, 0x10);
		if (rc < 0)
			return rc;

		if (--retries < 0) {
			err("i2c read retries exhausted");
			return -1;
		}
	}

	value = reg_r(ov, R51x_I2C_DATA);

	PDEBUG(5, "0x%02X:0x%02X", reg, value);

	
	rc = reg_w(ov, R511_I2C_CTL, 0x05);
	if (rc < 0)
		return rc;

	return value;
}


static int
i2c_r(struct usb_ov511 *ov, unsigned char reg)
{
	int rc;

	mutex_lock(&ov->i2c_lock);

	if (ov->bclass == BCL_OV518)
		rc = ov518_i2c_read_internal(ov, reg);
	else
		rc = ov511_i2c_read_internal(ov, reg);

	mutex_unlock(&ov->i2c_lock);

	return rc;
}

static int
i2c_w(struct usb_ov511 *ov, unsigned char reg, unsigned char value)
{
	int rc;

	mutex_lock(&ov->i2c_lock);

	if (ov->bclass == BCL_OV518)
		rc = ov518_i2c_write_internal(ov, reg, value);
	else
		rc = ov511_i2c_write_internal(ov, reg, value);

	mutex_unlock(&ov->i2c_lock);

	return rc;
}


static int
ov51x_i2c_write_mask_internal(struct usb_ov511 *ov,
			      unsigned char reg,
			      unsigned char value,
			      unsigned char mask)
{
	int rc;
	unsigned char oldval, newval;

	if (mask == 0xff) {
		newval = value;
	} else {
		if (ov->bclass == BCL_OV518)
			rc = ov518_i2c_read_internal(ov, reg);
		else
			rc = ov511_i2c_read_internal(ov, reg);
		if (rc < 0)
			return rc;

		oldval = (unsigned char) rc;
		oldval &= (~mask);		
		value &= mask;			
		newval = oldval | value;	
	}

	if (ov->bclass == BCL_OV518)
		return (ov518_i2c_write_internal(ov, reg, newval));
	else
		return (ov511_i2c_write_internal(ov, reg, newval));
}


static int
i2c_w_mask(struct usb_ov511 *ov,
	   unsigned char reg,
	   unsigned char value,
	   unsigned char mask)
{
	int rc;

	mutex_lock(&ov->i2c_lock);
	rc = ov51x_i2c_write_mask_internal(ov, reg, value, mask);
	mutex_unlock(&ov->i2c_lock);

	return rc;
}


static int
i2c_set_slave_internal(struct usb_ov511 *ov, unsigned char slave)
{
	int rc;

	rc = reg_w(ov, R51x_I2C_W_SID, slave);
	if (rc < 0)
		return rc;

	rc = reg_w(ov, R51x_I2C_R_SID, slave + 1);
	if (rc < 0)
		return rc;

	return 0;
}


static int
i2c_w_slave(struct usb_ov511 *ov,
	    unsigned char slave,
	    unsigned char reg,
	    unsigned char value,
	    unsigned char mask)
{
	int rc = 0;

	mutex_lock(&ov->i2c_lock);

	
	rc = i2c_set_slave_internal(ov, slave);
	if (rc < 0)
		goto out;

	rc = ov51x_i2c_write_mask_internal(ov, reg, value, mask);

out:
	
	if (i2c_set_slave_internal(ov, ov->primary_i2c_slave) < 0)
		err("Couldn't restore primary I2C slave");

	mutex_unlock(&ov->i2c_lock);
	return rc;
}


static int
i2c_r_slave(struct usb_ov511 *ov,
	    unsigned char slave,
	    unsigned char reg)
{
	int rc;

	mutex_lock(&ov->i2c_lock);

	
	rc = i2c_set_slave_internal(ov, slave);
	if (rc < 0)
		goto out;

	if (ov->bclass == BCL_OV518)
		rc = ov518_i2c_read_internal(ov, reg);
	else
		rc = ov511_i2c_read_internal(ov, reg);

out:
	
	if (i2c_set_slave_internal(ov, ov->primary_i2c_slave) < 0)
		err("Couldn't restore primary I2C slave");

	mutex_unlock(&ov->i2c_lock);
	return rc;
}


static int
ov51x_set_slave_ids(struct usb_ov511 *ov, unsigned char sid)
{
	int rc;

	mutex_lock(&ov->i2c_lock);

	rc = i2c_set_slave_internal(ov, sid);
	if (rc < 0)
		goto out;

	
	rc = ov51x_reset(ov, OV511_RESET_NOREGS);
out:
	mutex_unlock(&ov->i2c_lock);
	return rc;
}

static int
write_regvals(struct usb_ov511 *ov, struct ov511_regvals * pRegvals)
{
	int rc;

	while (pRegvals->bus != OV511_DONE_BUS) {
		if (pRegvals->bus == OV511_REG_BUS) {
			if ((rc = reg_w(ov, pRegvals->reg, pRegvals->val)) < 0)
				return rc;
		} else if (pRegvals->bus == OV511_I2C_BUS) {
			if ((rc = i2c_w(ov, pRegvals->reg, pRegvals->val)) < 0)
				return rc;
		} else {
			err("Bad regval array");
			return -1;
		}
		pRegvals++;
	}
	return 0;
}

#ifdef OV511_DEBUG
static void
dump_i2c_range(struct usb_ov511 *ov, int reg1, int regn)
{
	int i, rc;

	for (i = reg1; i <= regn; i++) {
		rc = i2c_r(ov, i);
		dev_info(&ov->dev->dev, "Sensor[0x%02X] = 0x%02X\n", i, rc);
	}
}

static void
dump_i2c_regs(struct usb_ov511 *ov)
{
	dev_info(&ov->dev->dev, "I2C REGS\n");
	dump_i2c_range(ov, 0x00, 0x7C);
}

static void
dump_reg_range(struct usb_ov511 *ov, int reg1, int regn)
{
	int i, rc;

	for (i = reg1; i <= regn; i++) {
		rc = reg_r(ov, i);
		dev_info(&ov->dev->dev, "OV511[0x%02X] = 0x%02X\n", i, rc);
	}
}

static void
ov511_dump_regs(struct usb_ov511 *ov)
{
	dev_info(&ov->dev->dev, "CAMERA INTERFACE REGS\n");
	dump_reg_range(ov, 0x10, 0x1f);
	dev_info(&ov->dev->dev, "DRAM INTERFACE REGS\n");
	dump_reg_range(ov, 0x20, 0x23);
	dev_info(&ov->dev->dev, "ISO FIFO REGS\n");
	dump_reg_range(ov, 0x30, 0x31);
	dev_info(&ov->dev->dev, "PIO REGS\n");
	dump_reg_range(ov, 0x38, 0x39);
	dump_reg_range(ov, 0x3e, 0x3e);
	dev_info(&ov->dev->dev, "I2C REGS\n");
	dump_reg_range(ov, 0x40, 0x49);
	dev_info(&ov->dev->dev, "SYSTEM CONTROL REGS\n");
	dump_reg_range(ov, 0x50, 0x55);
	dump_reg_range(ov, 0x5e, 0x5f);
	dev_info(&ov->dev->dev, "OmniCE REGS\n");
	dump_reg_range(ov, 0x70, 0x79);
	
	dump_reg_range(ov, 0x80, 0x9f);
	dump_reg_range(ov, 0xa0, 0xbf);

}

static void
ov518_dump_regs(struct usb_ov511 *ov)
{
	dev_info(&ov->dev->dev, "VIDEO MODE REGS\n");
	dump_reg_range(ov, 0x20, 0x2f);
	dev_info(&ov->dev->dev, "DATA PUMP AND SNAPSHOT REGS\n");
	dump_reg_range(ov, 0x30, 0x3f);
	dev_info(&ov->dev->dev, "I2C REGS\n");
	dump_reg_range(ov, 0x40, 0x4f);
	dev_info(&ov->dev->dev, "SYSTEM CONTROL AND VENDOR REGS\n");
	dump_reg_range(ov, 0x50, 0x5f);
	dev_info(&ov->dev->dev, "60 - 6F\n");
	dump_reg_range(ov, 0x60, 0x6f);
	dev_info(&ov->dev->dev, "70 - 7F\n");
	dump_reg_range(ov, 0x70, 0x7f);
	dev_info(&ov->dev->dev, "Y QUANTIZATION TABLE\n");
	dump_reg_range(ov, 0x80, 0x8f);
	dev_info(&ov->dev->dev, "UV QUANTIZATION TABLE\n");
	dump_reg_range(ov, 0x90, 0x9f);
	dev_info(&ov->dev->dev, "A0 - BF\n");
	dump_reg_range(ov, 0xa0, 0xbf);
	dev_info(&ov->dev->dev, "CBR\n");
	dump_reg_range(ov, 0xc0, 0xcf);
}
#endif




static inline int
ov51x_stop(struct usb_ov511 *ov)
{
	PDEBUG(4, "stopping");
	ov->stopped = 1;
	if (ov->bclass == BCL_OV518)
		return (reg_w_mask(ov, R51x_SYS_RESET, 0x3a, 0x3a));
	else
		return (reg_w(ov, R51x_SYS_RESET, 0x3d));
}


static inline int
ov51x_restart(struct usb_ov511 *ov)
{
	if (ov->stopped) {
		PDEBUG(4, "restarting");
		ov->stopped = 0;

		
		if (ov->bclass == BCL_OV518)
			reg_w(ov, 0x2f, 0x80);

		return (reg_w(ov, R51x_SYS_RESET, 0x00));
	}

	return 0;
}


static int
ov51x_wait_frames_inactive(struct usb_ov511 *ov)
{
	return wait_event_interruptible(ov->wq, ov->curframe < 0);
}


static void
ov51x_clear_snapshot(struct usb_ov511 *ov)
{
	if (ov->bclass == BCL_OV511) {
		reg_w(ov, R51x_SYS_SNAP, 0x00);
		reg_w(ov, R51x_SYS_SNAP, 0x02);
		reg_w(ov, R51x_SYS_SNAP, 0x00);
	} else if (ov->bclass == BCL_OV518) {
		dev_warn(&ov->dev->dev,
			 "snapshot reset not supported yet on OV518(+)\n");
	} else {
		dev_err(&ov->dev->dev, "clear snap: invalid bridge type\n");
	}
}

#if 0

static int
ov51x_check_snapshot(struct usb_ov511 *ov)
{
	int ret, status = 0;

	if (ov->bclass == BCL_OV511) {
		ret = reg_r(ov, R51x_SYS_SNAP);
		if (ret < 0) {
			dev_err(&ov->dev->dev,
				"Error checking snspshot status (%d)\n", ret);
		} else if (ret & 0x08) {
			status = 1;
		}
	} else if (ov->bclass == BCL_OV518) {
		dev_warn(&ov->dev->dev,
			 "snapshot check not supported yet on OV518(+)\n");
	} else {
		dev_err(&ov->dev->dev, "clear snap: invalid bridge type\n");
	}

	return status;
}
#endif


static int
init_ov_sensor(struct usb_ov511 *ov)
{
	int i, success;

	
	if (i2c_w(ov, 0x12, 0x80) < 0)
		return -EIO;

	
	msleep(150);

	for (i = 0, success = 0; i < i2c_detect_tries && !success; i++) {
		if ((i2c_r(ov, OV7610_REG_ID_HIGH) == 0x7F) &&
		    (i2c_r(ov, OV7610_REG_ID_LOW) == 0xA2)) {
			success = 1;
			continue;
		}

		
		if (i2c_w(ov, 0x12, 0x80) < 0)
			return -EIO;
		
		msleep(150);
		
		if (i2c_r(ov, 0x00) < 0)
			return -EIO;
	}

	if (!success)
		return -EIO;

	PDEBUG(1, "I2C synced in %d attempt(s)", i);

	return 0;
}

static int
ov511_set_packet_size(struct usb_ov511 *ov, int size)
{
	int alt, mult;

	if (ov51x_stop(ov) < 0)
		return -EIO;

	mult = size >> 5;

	if (ov->bridge == BRG_OV511) {
		if (size == 0)
			alt = OV511_ALT_SIZE_0;
		else if (size == 257)
			alt = OV511_ALT_SIZE_257;
		else if (size == 513)
			alt = OV511_ALT_SIZE_513;
		else if (size == 769)
			alt = OV511_ALT_SIZE_769;
		else if (size == 993)
			alt = OV511_ALT_SIZE_993;
		else {
			err("Set packet size: invalid size (%d)", size);
			return -EINVAL;
		}
	} else if (ov->bridge == BRG_OV511PLUS) {
		if (size == 0)
			alt = OV511PLUS_ALT_SIZE_0;
		else if (size == 33)
			alt = OV511PLUS_ALT_SIZE_33;
		else if (size == 129)
			alt = OV511PLUS_ALT_SIZE_129;
		else if (size == 257)
			alt = OV511PLUS_ALT_SIZE_257;
		else if (size == 385)
			alt = OV511PLUS_ALT_SIZE_385;
		else if (size == 513)
			alt = OV511PLUS_ALT_SIZE_513;
		else if (size == 769)
			alt = OV511PLUS_ALT_SIZE_769;
		else if (size == 961)
			alt = OV511PLUS_ALT_SIZE_961;
		else {
			err("Set packet size: invalid size (%d)", size);
			return -EINVAL;
		}
	} else {
		err("Set packet size: Invalid bridge type");
		return -EINVAL;
	}

	PDEBUG(3, "%d, mult=%d, alt=%d", size, mult, alt);

	if (reg_w(ov, R51x_FIFO_PSIZE, mult) < 0)
		return -EIO;

	if (usb_set_interface(ov->dev, ov->iface, alt) < 0) {
		err("Set packet size: set interface error");
		return -EBUSY;
	}

	if (ov51x_reset(ov, OV511_RESET_NOREGS) < 0)
		return -EIO;

	ov->packet_size = size;

	if (ov51x_restart(ov) < 0)
		return -EIO;

	return 0;
}


static int
ov518_set_packet_size(struct usb_ov511 *ov, int size)
{
	int alt;

	if (ov51x_stop(ov) < 0)
		return -EIO;

	if (ov->bclass == BCL_OV518) {
		if (size == 0)
			alt = OV518_ALT_SIZE_0;
		else if (size == 128)
			alt = OV518_ALT_SIZE_128;
		else if (size == 256)
			alt = OV518_ALT_SIZE_256;
		else if (size == 384)
			alt = OV518_ALT_SIZE_384;
		else if (size == 512)
			alt = OV518_ALT_SIZE_512;
		else if (size == 640)
			alt = OV518_ALT_SIZE_640;
		else if (size == 768)
			alt = OV518_ALT_SIZE_768;
		else if (size == 896)
			alt = OV518_ALT_SIZE_896;
		else {
			err("Set packet size: invalid size (%d)", size);
			return -EINVAL;
		}
	} else {
		err("Set packet size: Invalid bridge type");
		return -EINVAL;
	}

	PDEBUG(3, "%d, alt=%d", size, alt);

	ov->packet_size = size;
	if (size > 0) {
		
		ov518_reg_w32(ov, 0x30, size, 2);

		if (ov->packet_numbering)
			++ov->packet_size;
	}

	if (usb_set_interface(ov->dev, ov->iface, alt) < 0) {
		err("Set packet size: set interface error");
		return -EBUSY;
	}

	
	if (reg_w(ov, 0x2f, 0x80) < 0)
		return -EIO;

	if (ov51x_restart(ov) < 0)
		return -EIO;

	if (ov51x_reset(ov, OV511_RESET_NOREGS) < 0)
		return -EIO;

	return 0;
}


static int
ov511_init_compression(struct usb_ov511 *ov)
{
	int rc = 0;

	if (!ov->compress_inited) {
		reg_w(ov, 0x70, phy);
		reg_w(ov, 0x71, phuv);
		reg_w(ov, 0x72, pvy);
		reg_w(ov, 0x73, pvuv);
		reg_w(ov, 0x74, qhy);
		reg_w(ov, 0x75, qhuv);
		reg_w(ov, 0x76, qvy);
		reg_w(ov, 0x77, qvuv);

		if (ov511_upload_quan_tables(ov) < 0) {
			err("Error uploading quantization tables");
			rc = -EIO;
			goto out;
		}
	}

	ov->compress_inited = 1;
out:
	return rc;
}


static int
ov518_init_compression(struct usb_ov511 *ov)
{
	int rc = 0;

	if (!ov->compress_inited) {
		if (ov518_upload_quan_tables(ov) < 0) {
			err("Error uploading quantization tables");
			rc = -EIO;
			goto out;
		}
	}

	ov->compress_inited = 1;
out:
	return rc;
}




static int
sensor_set_contrast(struct usb_ov511 *ov, unsigned short val)
{
	int rc;

	PDEBUG(3, "%d", val);

	if (ov->stop_during_set)
		if (ov51x_stop(ov) < 0)
			return -EIO;

	switch (ov->sensor) {
	case SEN_OV7610:
	case SEN_OV6620:
	{
		rc = i2c_w(ov, OV7610_REG_CNT, val >> 8);
		if (rc < 0)
			goto out;
		break;
	}
	case SEN_OV6630:
	{
		rc = i2c_w_mask(ov, OV7610_REG_CNT, val >> 12, 0x0f);
		if (rc < 0)
			goto out;
		break;
	}
	case SEN_OV7620:
	{
		unsigned char ctab[] = {
			0x01, 0x05, 0x09, 0x11, 0x15, 0x35, 0x37, 0x57,
			0x5b, 0xa5, 0xa7, 0xc7, 0xc9, 0xcf, 0xef, 0xff
		};

		
		rc = i2c_w(ov, 0x64, ctab[val>>12]);
		if (rc < 0)
			goto out;
		break;
	}
	case SEN_SAA7111A:
	{
		rc = i2c_w(ov, 0x0b, val >> 9);
		if (rc < 0)
			goto out;
		break;
	}
	default:
	{
		PDEBUG(3, "Unsupported with this sensor");
		rc = -EPERM;
		goto out;
	}
	}

	rc = 0;		
	ov->contrast = val;
out:
	if (ov51x_restart(ov) < 0)
		return -EIO;

	return rc;
}


static int
sensor_get_contrast(struct usb_ov511 *ov, unsigned short *val)
{
	int rc;

	switch (ov->sensor) {
	case SEN_OV7610:
	case SEN_OV6620:
		rc = i2c_r(ov, OV7610_REG_CNT);
		if (rc < 0)
			return rc;
		else
			*val = rc << 8;
		break;
	case SEN_OV6630:
		rc = i2c_r(ov, OV7610_REG_CNT);
		if (rc < 0)
			return rc;
		else
			*val = rc << 12;
		break;
	case SEN_OV7620:
		
		rc = i2c_r(ov, 0x64);
		if (rc < 0)
			return rc;
		else
			*val = (rc & 0xfe) << 8;
		break;
	case SEN_SAA7111A:
		*val = ov->contrast;
		break;
	default:
		PDEBUG(3, "Unsupported with this sensor");
		return -EPERM;
	}

	PDEBUG(3, "%d", *val);
	ov->contrast = *val;

	return 0;
}




static int
sensor_set_brightness(struct usb_ov511 *ov, unsigned short val)
{
	int rc;

	PDEBUG(4, "%d", val);

	if (ov->stop_during_set)
		if (ov51x_stop(ov) < 0)
			return -EIO;

	switch (ov->sensor) {
	case SEN_OV7610:
	case SEN_OV76BE:
	case SEN_OV6620:
	case SEN_OV6630:
		rc = i2c_w(ov, OV7610_REG_BRT, val >> 8);
		if (rc < 0)
			goto out;
		break;
	case SEN_OV7620:
		
		if (!ov->auto_brt) {
			rc = i2c_w(ov, OV7610_REG_BRT, val >> 8);
			if (rc < 0)
				goto out;
		}
		break;
	case SEN_SAA7111A:
		rc = i2c_w(ov, 0x0a, val >> 8);
		if (rc < 0)
			goto out;
		break;
	default:
		PDEBUG(3, "Unsupported with this sensor");
		rc = -EPERM;
		goto out;
	}

	rc = 0;		
	ov->brightness = val;
out:
	if (ov51x_restart(ov) < 0)
		return -EIO;

	return rc;
}


static int
sensor_get_brightness(struct usb_ov511 *ov, unsigned short *val)
{
	int rc;

	switch (ov->sensor) {
	case SEN_OV7610:
	case SEN_OV76BE:
	case SEN_OV7620:
	case SEN_OV6620:
	case SEN_OV6630:
		rc = i2c_r(ov, OV7610_REG_BRT);
		if (rc < 0)
			return rc;
		else
			*val = rc << 8;
		break;
	case SEN_SAA7111A:
		*val = ov->brightness;
		break;
	default:
		PDEBUG(3, "Unsupported with this sensor");
		return -EPERM;
	}

	PDEBUG(3, "%d", *val);
	ov->brightness = *val;

	return 0;
}




static int
sensor_set_saturation(struct usb_ov511 *ov, unsigned short val)
{
	int rc;

	PDEBUG(3, "%d", val);

	if (ov->stop_during_set)
		if (ov51x_stop(ov) < 0)
			return -EIO;

	switch (ov->sensor) {
	case SEN_OV7610:
	case SEN_OV76BE:
	case SEN_OV6620:
	case SEN_OV6630:
		rc = i2c_w(ov, OV7610_REG_SAT, val >> 8);
		if (rc < 0)
			goto out;
		break;
	case SEN_OV7620:




		rc = i2c_w(ov, OV7610_REG_SAT, val >> 8);
		if (rc < 0)
			goto out;
		break;
	case SEN_SAA7111A:
		rc = i2c_w(ov, 0x0c, val >> 9);
		if (rc < 0)
			goto out;
		break;
	default:
		PDEBUG(3, "Unsupported with this sensor");
		rc = -EPERM;
		goto out;
	}

	rc = 0;		
	ov->colour = val;
out:
	if (ov51x_restart(ov) < 0)
		return -EIO;

	return rc;
}


static int
sensor_get_saturation(struct usb_ov511 *ov, unsigned short *val)
{
	int rc;

	switch (ov->sensor) {
	case SEN_OV7610:
	case SEN_OV76BE:
	case SEN_OV6620:
	case SEN_OV6630:
		rc = i2c_r(ov, OV7610_REG_SAT);
		if (rc < 0)
			return rc;
		else
			*val = rc << 8;
		break;
	case SEN_OV7620:






		rc = i2c_r(ov, OV7610_REG_SAT);
		if (rc < 0)
			return rc;
		else
			*val = rc << 8;
		break;
	case SEN_SAA7111A:
		*val = ov->colour;
		break;
	default:
		PDEBUG(3, "Unsupported with this sensor");
		return -EPERM;
	}

	PDEBUG(3, "%d", *val);
	ov->colour = *val;

	return 0;
}




static int
sensor_set_hue(struct usb_ov511 *ov, unsigned short val)
{
	int rc;

	PDEBUG(3, "%d", val);

	if (ov->stop_during_set)
		if (ov51x_stop(ov) < 0)
			return -EIO;

	switch (ov->sensor) {
	case SEN_OV7610:
	case SEN_OV6620:
	case SEN_OV6630:
		rc = i2c_w(ov, OV7610_REG_RED, 0xFF - (val >> 8));
		if (rc < 0)
			goto out;

		rc = i2c_w(ov, OV7610_REG_BLUE, val >> 8);
		if (rc < 0)
			goto out;
		break;
	case SEN_OV7620:

#if 0
		rc = i2c_w(ov, 0x7a, (unsigned char)(val >> 8) + 0xb);
		if (rc < 0)
			goto out;

		rc = i2c_w(ov, 0x79, (unsigned char)(val >> 8) + 0xb);
		if (rc < 0)
			goto out;
#endif
		break;
	case SEN_SAA7111A:
		rc = i2c_w(ov, 0x0d, (val + 32768) >> 8);
		if (rc < 0)
			goto out;
		break;
	default:
		PDEBUG(3, "Unsupported with this sensor");
		rc = -EPERM;
		goto out;
	}

	rc = 0;		
	ov->hue = val;
out:
	if (ov51x_restart(ov) < 0)
		return -EIO;

	return rc;
}


static int
sensor_get_hue(struct usb_ov511 *ov, unsigned short *val)
{
	int rc;

	switch (ov->sensor) {
	case SEN_OV7610:
	case SEN_OV6620:
	case SEN_OV6630:
		rc = i2c_r(ov, OV7610_REG_BLUE);
		if (rc < 0)
			return rc;
		else
			*val = rc << 8;
		break;
	case SEN_OV7620:
		rc = i2c_r(ov, 0x7a);
		if (rc < 0)
			return rc;
		else
			*val = rc << 8;
		break;
	case SEN_SAA7111A:
		*val = ov->hue;
		break;
	default:
		PDEBUG(3, "Unsupported with this sensor");
		return -EPERM;
	}

	PDEBUG(3, "%d", *val);
	ov->hue = *val;

	return 0;
}



static int
sensor_set_picture(struct usb_ov511 *ov, struct video_picture *p)
{
	int rc;

	PDEBUG(4, "sensor_set_picture");

	ov->whiteness = p->whiteness;

	

	rc = sensor_set_contrast(ov, p->contrast);
	if (FATAL_ERROR(rc))
		return rc;

	rc = sensor_set_brightness(ov, p->brightness);
	if (FATAL_ERROR(rc))
		return rc;

	rc = sensor_set_saturation(ov, p->colour);
	if (FATAL_ERROR(rc))
		return rc;

	rc = sensor_set_hue(ov, p->hue);
	if (FATAL_ERROR(rc))
		return rc;

	return 0;
}

static int
sensor_get_picture(struct usb_ov511 *ov, struct video_picture *p)
{
	int rc;

	PDEBUG(4, "sensor_get_picture");

	

	rc = sensor_get_contrast(ov, &(p->contrast));
	if (FATAL_ERROR(rc))
		return rc;

	rc = sensor_get_brightness(ov, &(p->brightness));
	if (FATAL_ERROR(rc))
		return rc;

	rc = sensor_get_saturation(ov, &(p->colour));
	if (FATAL_ERROR(rc))
		return rc;

	rc = sensor_get_hue(ov, &(p->hue));
	if (FATAL_ERROR(rc))
		return rc;

	p->whiteness = 105 << 8;

	return 0;
}

#if 0


static inline int
sensor_set_exposure(struct usb_ov511 *ov, unsigned char val)
{
	int rc;

	PDEBUG(3, "%d", val);

	if (ov->stop_during_set)
		if (ov51x_stop(ov) < 0)
			return -EIO;

	switch (ov->sensor) {
	case SEN_OV6620:
	case SEN_OV6630:
	case SEN_OV7610:
	case SEN_OV7620:
	case SEN_OV76BE:
	case SEN_OV8600:
		rc = i2c_w(ov, 0x10, val);
		if (rc < 0)
			goto out;

		break;
	case SEN_KS0127:
	case SEN_KS0127B:
	case SEN_SAA7111A:
		PDEBUG(3, "Unsupported with this sensor");
		return -EPERM;
	default:
		err("Sensor not supported for set_exposure");
		return -EINVAL;
	}

	rc = 0;		
	ov->exposure = val;
out:
	if (ov51x_restart(ov) < 0)
		return -EIO;

	return rc;
}
#endif


static int
sensor_get_exposure(struct usb_ov511 *ov, unsigned char *val)
{
	int rc;

	switch (ov->sensor) {
	case SEN_OV7610:
	case SEN_OV6620:
	case SEN_OV6630:
	case SEN_OV7620:
	case SEN_OV76BE:
	case SEN_OV8600:
		rc = i2c_r(ov, 0x10);
		if (rc < 0)
			return rc;
		else
			*val = rc;
		break;
	case SEN_KS0127:
	case SEN_KS0127B:
	case SEN_SAA7111A:
		val = NULL;
		PDEBUG(3, "Unsupported with this sensor");
		return -EPERM;
	default:
		err("Sensor not supported for get_exposure");
		return -EINVAL;
	}

	PDEBUG(3, "%d", *val);
	ov->exposure = *val;

	return 0;
}


static void
ov51x_led_control(struct usb_ov511 *ov, int enable)
{
	PDEBUG(4, " (%s)", enable ? "turn on" : "turn off");

	if (ov->bridge == BRG_OV511PLUS)
		reg_w(ov, R511_SYS_LED_CTL, enable ? 1 : 0);
	else if (ov->bclass == BCL_OV518)
		reg_w_mask(ov, R518_GPIO_OUT, enable ? 0x02 : 0x00, 0x02);

	return;
}


static int
sensor_set_light_freq(struct usb_ov511 *ov, int freq)
{
	int sixty;

	PDEBUG(4, "%d Hz", freq);

	if (freq == 60)
		sixty = 1;
	else if (freq == 50)
		sixty = 0;
	else {
		err("Invalid light freq (%d Hz)", freq);
		return -EINVAL;
	}

	switch (ov->sensor) {
	case SEN_OV7610:
		i2c_w_mask(ov, 0x2a, sixty?0x00:0x80, 0x80);
		i2c_w(ov, 0x2b, sixty?0x00:0xac);
		i2c_w_mask(ov, 0x13, 0x10, 0x10);
		i2c_w_mask(ov, 0x13, 0x00, 0x10);
		break;
	case SEN_OV7620:
	case SEN_OV76BE:
	case SEN_OV8600:
		i2c_w_mask(ov, 0x2a, sixty?0x00:0x80, 0x80);
		i2c_w(ov, 0x2b, sixty?0x00:0xac);
		i2c_w_mask(ov, 0x76, 0x01, 0x01);
		break;
	case SEN_OV6620:
	case SEN_OV6630:
		i2c_w(ov, 0x2b, sixty?0xa8:0x28);
		i2c_w(ov, 0x2a, sixty?0x84:0xa4);
		break;
	case SEN_KS0127:
	case SEN_KS0127B:
	case SEN_SAA7111A:
		PDEBUG(5, "Unsupported with this sensor");
		return -EPERM;
	default:
		err("Sensor not supported for set_light_freq");
		return -EINVAL;
	}

	ov->lightfreq = freq;

	return 0;
}


static int
sensor_set_banding_filter(struct usb_ov511 *ov, int enable)
{
	int rc;

	PDEBUG(4, " (%s)", enable ? "turn on" : "turn off");

	if (ov->sensor == SEN_KS0127 || ov->sensor == SEN_KS0127B
		|| ov->sensor == SEN_SAA7111A) {
		PDEBUG(5, "Unsupported with this sensor");
		return -EPERM;
	}

	rc = i2c_w_mask(ov, 0x2d, enable?0x04:0x00, 0x04);
	if (rc < 0)
		return rc;

	ov->bandfilt = enable;

	return 0;
}


static int
sensor_set_auto_brightness(struct usb_ov511 *ov, int enable)
{
	int rc;

	PDEBUG(4, " (%s)", enable ? "turn on" : "turn off");

	if (ov->sensor == SEN_KS0127 || ov->sensor == SEN_KS0127B
		|| ov->sensor == SEN_SAA7111A) {
		PDEBUG(5, "Unsupported with this sensor");
		return -EPERM;
	}

	rc = i2c_w_mask(ov, 0x2d, enable?0x10:0x00, 0x10);
	if (rc < 0)
		return rc;

	ov->auto_brt = enable;

	return 0;
}


static int
sensor_set_auto_exposure(struct usb_ov511 *ov, int enable)
{
	PDEBUG(4, " (%s)", enable ? "turn on" : "turn off");

	switch (ov->sensor) {
	case SEN_OV7610:
		i2c_w_mask(ov, 0x29, enable?0x00:0x80, 0x80);
		break;
	case SEN_OV6620:
	case SEN_OV7620:
	case SEN_OV76BE:
	case SEN_OV8600:
		i2c_w_mask(ov, 0x13, enable?0x01:0x00, 0x01);
		break;
	case SEN_OV6630:
		i2c_w_mask(ov, 0x28, enable?0x00:0x10, 0x10);
		break;
	case SEN_KS0127:
	case SEN_KS0127B:
	case SEN_SAA7111A:
		PDEBUG(5, "Unsupported with this sensor");
		return -EPERM;
	default:
		err("Sensor not supported for set_auto_exposure");
		return -EINVAL;
	}

	ov->auto_exp = enable;

	return 0;
}


static int
sensor_set_backlight(struct usb_ov511 *ov, int enable)
{
	PDEBUG(4, " (%s)", enable ? "turn on" : "turn off");

	switch (ov->sensor) {
	case SEN_OV7620:
	case SEN_OV8600:
		i2c_w_mask(ov, 0x68, enable?0xe0:0xc0, 0xe0);
		i2c_w_mask(ov, 0x29, enable?0x08:0x00, 0x08);
		i2c_w_mask(ov, 0x28, enable?0x02:0x00, 0x02);
		break;
	case SEN_OV6620:
		i2c_w_mask(ov, 0x4e, enable?0xe0:0xc0, 0xe0);
		i2c_w_mask(ov, 0x29, enable?0x08:0x00, 0x08);
		i2c_w_mask(ov, 0x0e, enable?0x80:0x00, 0x80);
		break;
	case SEN_OV6630:
		i2c_w_mask(ov, 0x4e, enable?0x80:0x60, 0xe0);
		i2c_w_mask(ov, 0x29, enable?0x08:0x00, 0x08);
		i2c_w_mask(ov, 0x28, enable?0x02:0x00, 0x02);
		break;
	case SEN_OV7610:
	case SEN_OV76BE:
	case SEN_KS0127:
	case SEN_KS0127B:
	case SEN_SAA7111A:
		PDEBUG(5, "Unsupported with this sensor");
		return -EPERM;
	default:
		err("Sensor not supported for set_backlight");
		return -EINVAL;
	}

	ov->backlight = enable;

	return 0;
}

static int
sensor_set_mirror(struct usb_ov511 *ov, int enable)
{
	PDEBUG(4, " (%s)", enable ? "turn on" : "turn off");

	switch (ov->sensor) {
	case SEN_OV6620:
	case SEN_OV6630:
	case SEN_OV7610:
	case SEN_OV7620:
	case SEN_OV76BE:
	case SEN_OV8600:
		i2c_w_mask(ov, 0x12, enable?0x40:0x00, 0x40);
		break;
	case SEN_KS0127:
	case SEN_KS0127B:
	case SEN_SAA7111A:
		PDEBUG(5, "Unsupported with this sensor");
		return -EPERM;
	default:
		err("Sensor not supported for set_mirror");
		return -EINVAL;
	}

	ov->mirror = enable;

	return 0;
}


static inline int
get_depth(int palette)
{
	switch (palette) {
	case VIDEO_PALETTE_GREY:    return 8;
	case VIDEO_PALETTE_YUV420:  return 12;
	case VIDEO_PALETTE_YUV420P: return 12; 
	default:		    return 0;  
	}
}


static inline long int
get_frame_length(struct ov511_frame *frame)
{
	if (!frame)
		return 0;
	else
		return ((frame->width * frame->height
			 * get_depth(frame->format)) >> 3);
}

static int
mode_init_ov_sensor_regs(struct usb_ov511 *ov, int width, int height,
			 int mode, int sub_flag, int qvga)
{
	int clock;

	

	switch (ov->sensor) {
	case SEN_OV7610:
		i2c_w(ov, 0x14, qvga?0x24:0x04);

#if 0
		i2c_w_mask(ov, 0x28, qvga?0x00:0x20, 0x20);
		i2c_w(ov, 0x24, 0x10);
		i2c_w(ov, 0x25, qvga?0x40:0x8a);
		i2c_w(ov, 0x2f, qvga?0x30:0xb0);
		i2c_w(ov, 0x35, qvga?0x1c:0x9c);
#endif
		break;
	case SEN_OV7620:

		i2c_w(ov, 0x14, qvga?0xa4:0x84);
		i2c_w_mask(ov, 0x28, qvga?0x00:0x20, 0x20);
		i2c_w(ov, 0x24, qvga?0x20:0x3a);
		i2c_w(ov, 0x25, qvga?0x30:0x60);
		i2c_w_mask(ov, 0x2d, qvga?0x40:0x00, 0x40);
		i2c_w_mask(ov, 0x67, qvga?0xf0:0x90, 0xf0);
		i2c_w_mask(ov, 0x74, qvga?0x20:0x00, 0x20);
		break;
	case SEN_OV76BE:

		i2c_w(ov, 0x14, qvga?0xa4:0x84);

#if 0
		i2c_w_mask(ov, 0x28, qvga?0x00:0x20, 0x20);
		i2c_w(ov, 0x24, qvga?0x20:0x3a);
		i2c_w(ov, 0x25, qvga?0x30:0x60);
		i2c_w_mask(ov, 0x2d, qvga?0x40:0x00, 0x40);
		i2c_w_mask(ov, 0x67, qvga?0xb0:0x90, 0xf0);
		i2c_w_mask(ov, 0x74, qvga?0x20:0x00, 0x20);
#endif
		break;
	case SEN_OV6620:
		i2c_w(ov, 0x14, qvga?0x24:0x04);
		break;
	case SEN_OV6630:
		i2c_w(ov, 0x14, qvga?0xa0:0x80);
		break;
	default:
		err("Invalid sensor");
		return -EINVAL;
	}

	

	if (mode == VIDEO_PALETTE_GREY) {
		if (ov->sensor == SEN_OV7610 || ov->sensor == SEN_OV76BE) {
			
			i2c_w_mask(ov, 0x0e, 0x40, 0x40);
		}

		if (ov->sensor == SEN_OV6630 && ov->bridge == BRG_OV518
		    && ov518_color) {
			i2c_w_mask(ov, 0x12, 0x00, 0x10);
			i2c_w_mask(ov, 0x13, 0x00, 0x20);
		} else {
			i2c_w_mask(ov, 0x13, 0x20, 0x20);
		}
	} else {
		if (ov->sensor == SEN_OV7610 || ov->sensor == SEN_OV76BE) {
			
			i2c_w_mask(ov, 0x0e, 0x00, 0x40);
		}

		

		if (ov->sensor == SEN_OV6630 && ov->bridge == BRG_OV518
		    && ov518_color) {
			i2c_w_mask(ov, 0x12, 0x10, 0x10);
			i2c_w_mask(ov, 0x13, 0x20, 0x20);
		} else {
			i2c_w_mask(ov, 0x13, 0x00, 0x20);
		}
	}

	

	
	if (ov->sensor == SEN_OV6620 || ov->sensor == SEN_OV6630)
	{
		

		i2c_w(ov, 0x2a, 0x04);

		if (ov->compress) {

			clock = 3;
		} else if (clockdiv == -1) {   
			clock = 3;    
		} else {
			clock = clockdiv;
		}

		PDEBUG(4, "Setting clock divisor to %d", clock);

		i2c_w(ov, 0x11, clock);

		i2c_w(ov, 0x2a, 0x84);
		
		i2c_w(ov, 0x2d, 0x85);
	}
	else
	{
		if (ov->compress) {
			clock = 1;    
		} else if (clockdiv == -1) {   
			
			clock = ((sub_flag ? ov->subw * ov->subh
				  : width * height)
				 * (mode == VIDEO_PALETTE_GREY ? 2 : 3) / 2)
				 / 66000;
		} else {
			clock = clockdiv;
		}

		PDEBUG(4, "Setting clock divisor to %d", clock);

		i2c_w(ov, 0x11, clock);
	}

	

	if (framedrop >= 0)
		i2c_w(ov, 0x16, framedrop);

	
	i2c_w_mask(ov, 0x12, (testpat?0x02:0x00), 0x02);

	
	i2c_w_mask(ov, 0x12, 0x04, 0x04);

	
	
	
	if (ov->sensor == SEN_OV7610 || ov->sensor == SEN_OV76BE) {
		if (width == 640 && height == 480)
			i2c_w(ov, 0x35, 0x9e);
		else
			i2c_w(ov, 0x35, 0x1e);
	}

	return 0;
}

static int
set_ov_sensor_window(struct usb_ov511 *ov, int width, int height, int mode,
		     int sub_flag)
{
	int ret;
	int hwsbase, hwebase, vwsbase, vwebase, hwsize, vwsize;
	int hoffset, voffset, hwscale = 0, vwscale = 0;

	
	switch (ov->sensor) {
	case SEN_OV7610:
	case SEN_OV76BE:
		hwsbase = 0x38;
		hwebase = 0x3a;
		vwsbase = vwebase = 0x05;
		break;
	case SEN_OV6620:
	case SEN_OV6630:
		hwsbase = 0x38;
		hwebase = 0x3a;
		vwsbase = 0x05;
		vwebase = 0x06;
		break;
	case SEN_OV7620:
		hwsbase = 0x2f;		
		hwebase = 0x2f;
		vwsbase = vwebase = 0x05;
		break;
	default:
		err("Invalid sensor");
		return -EINVAL;
	}

	if (ov->sensor == SEN_OV6620 || ov->sensor == SEN_OV6630) {
		
		if ((width > 176 && height > 144)
		    || ov->bclass == BCL_OV518) {  
			ret = mode_init_ov_sensor_regs(ov, width, height,
				mode, sub_flag, 0);
			if (ret < 0)
				return ret;
			hwscale = 1;
			vwscale = 1;  
			hwsize = 352;
			vwsize = 288;
		} else if (width > 176 || height > 144) {
			err("Illegal dimensions");
			return -EINVAL;
		} else {			    
			ret = mode_init_ov_sensor_regs(ov, width, height,
				mode, sub_flag, 1);
			if (ret < 0)
				return ret;
			hwsize = 176;
			vwsize = 144;
		}
	} else {
		if (width > 320 && height > 240) {  
			ret = mode_init_ov_sensor_regs(ov, width, height,
				mode, sub_flag, 0);
			if (ret < 0)
				return ret;
			hwscale = 2;
			vwscale = 1;
			hwsize = 640;
			vwsize = 480;
		} else if (width > 320 || height > 240) {
			err("Illegal dimensions");
			return -EINVAL;
		} else {			    
			ret = mode_init_ov_sensor_regs(ov, width, height,
				mode, sub_flag, 1);
			if (ret < 0)
				return ret;
			hwscale = 1;
			hwsize = 320;
			vwsize = 240;
		}
	}

	
	hoffset = ((hwsize - width) / 2) >> hwscale;
	voffset = ((vwsize - height) / 2) >> vwscale;

	
	if (sub_flag) {
		i2c_w(ov, 0x17, hwsbase+(ov->subx>>hwscale));
		i2c_w(ov, 0x18,	hwebase+((ov->subx+ov->subw)>>hwscale));
		i2c_w(ov, 0x19, vwsbase+(ov->suby>>vwscale));
		i2c_w(ov, 0x1a, vwebase+((ov->suby+ov->subh)>>vwscale));
	} else {
		i2c_w(ov, 0x17, hwsbase + hoffset);
		i2c_w(ov, 0x18, hwebase + hoffset + (hwsize>>hwscale));
		i2c_w(ov, 0x19, vwsbase + voffset);
		i2c_w(ov, 0x1a, vwebase + voffset + (vwsize>>vwscale));
	}

#ifdef OV511_DEBUG
	if (dump_sensor)
		dump_i2c_regs(ov);
#endif

	return 0;
}


static int
ov511_mode_init_regs(struct usb_ov511 *ov,
		     int width, int height, int mode, int sub_flag)
{
	int hsegs, vsegs;

	if (sub_flag) {
		width = ov->subw;
		height = ov->subh;
	}

	PDEBUG(3, "width:%d, height:%d, mode:%d, sub:%d",
	       width, height, mode, sub_flag);

	
	
	if (ov->sensor == SEN_SAA7111A) {
		if (width == 320 && height == 240) {
			
		} else if (width == 640 && height == 480) {
			
			width = 320;
		} else {
			err("SAA7111A only allows 320x240 or 640x480");
			return -EINVAL;
		}
	}

	
	if (width % 8 || height % 8) {
		err("Invalid size (%d, %d) (mode = %d)", width, height, mode);
		return -EINVAL;
	}

	if (width < ov->minwidth || height < ov->minheight) {
		err("Requested dimensions are too small");
		return -EINVAL;
	}

	if (ov51x_stop(ov) < 0)
		return -EIO;

	if (mode == VIDEO_PALETTE_GREY) {
		reg_w(ov, R511_CAM_UV_EN, 0x00);
		reg_w(ov, R511_SNAP_UV_EN, 0x00);
		reg_w(ov, R511_SNAP_OPTS, 0x01);
	} else {
		reg_w(ov, R511_CAM_UV_EN, 0x01);
		reg_w(ov, R511_SNAP_UV_EN, 0x01);
		reg_w(ov, R511_SNAP_OPTS, 0x03);
	}

	
	hsegs = (width >> 3) - 1;
	vsegs = (height >> 3) - 1;

	reg_w(ov, R511_CAM_PXCNT, hsegs);
	reg_w(ov, R511_CAM_LNCNT, vsegs);
	reg_w(ov, R511_CAM_PXDIV, 0x00);
	reg_w(ov, R511_CAM_LNDIV, 0x00);

	
	reg_w(ov, R511_CAM_OPTS, 0x03);

	
	reg_w(ov, R511_SNAP_PXCNT, hsegs);
	reg_w(ov, R511_SNAP_LNCNT, vsegs);
	reg_w(ov, R511_SNAP_PXDIV, 0x00);
	reg_w(ov, R511_SNAP_LNDIV, 0x00);

	if (ov->compress) {
		
		reg_w(ov, R511_COMP_EN, 0x07);
		reg_w(ov, R511_COMP_LUT_EN, 0x03);
		ov51x_reset(ov, OV511_RESET_OMNICE);
	}

	if (ov51x_restart(ov) < 0)
		return -EIO;

	return 0;
}


static int
ov518_mode_init_regs(struct usb_ov511 *ov,
		     int width, int height, int mode, int sub_flag)
{
	int hsegs, vsegs, hi_res;

	if (sub_flag) {
		width = ov->subw;
		height = ov->subh;
	}

	PDEBUG(3, "width:%d, height:%d, mode:%d, sub:%d",
	       width, height, mode, sub_flag);

	if (width % 16 || height % 8) {
		err("Invalid size (%d, %d)", width, height);
		return -EINVAL;
	}

	if (width < ov->minwidth || height < ov->minheight) {
		err("Requested dimensions are too small");
		return -EINVAL;
	}

	if (width >= 320 && height >= 240) {
		hi_res = 1;
	} else if (width >= 320 || height >= 240) {
		err("Invalid width/height combination (%d, %d)", width, height);
		return -EINVAL;
	} else {
		hi_res = 0;
	}

	if (ov51x_stop(ov) < 0)
		return -EIO;

	

	reg_w(ov, 0x2b, 0);
	reg_w(ov, 0x2c, 0);
	reg_w(ov, 0x2d, 0);
	reg_w(ov, 0x2e, 0);
	reg_w(ov, 0x3b, 0);
	reg_w(ov, 0x3c, 0);
	reg_w(ov, 0x3d, 0);
	reg_w(ov, 0x3e, 0);

	if (ov->bridge == BRG_OV518 && ov518_color) {
		
		i2c_w_mask(ov, 0x15, 0x00, 0x01);

		if (mode == VIDEO_PALETTE_GREY) {
			
			reg_w_mask(ov, 0x20, 0x00, 0x08);

			
			reg_w_mask(ov, 0x28, 0x00, 0xf0);
			reg_w_mask(ov, 0x38, 0x00, 0xf0);
		} else {
			
			reg_w_mask(ov, 0x20, 0x08, 0x08);

			
			reg_w_mask(ov, 0x28, 0x80, 0xf0);
			reg_w_mask(ov, 0x38, 0x80, 0xf0);
		}
	} else {
		reg_w(ov, 0x28, (mode == VIDEO_PALETTE_GREY) ? 0x00:0x80);
		reg_w(ov, 0x38, (mode == VIDEO_PALETTE_GREY) ? 0x00:0x80);
	}

	hsegs = width / 16;
	vsegs = height / 4;

	reg_w(ov, 0x29, hsegs);
	reg_w(ov, 0x2a, vsegs);

	reg_w(ov, 0x39, hsegs);
	reg_w(ov, 0x3a, vsegs);

	
	reg_w(ov, 0x2f, 0x80);

	

	
	reg_w(ov, 0x51, 0x02);	
	reg_w(ov, 0x22, 0x18);
	reg_w(ov, 0x23, 0xff);

	if (ov->bridge == BRG_OV518PLUS)
		reg_w(ov, 0x21, 0x19);
	else
		reg_w(ov, 0x71, 0x19);	

	
	
	i2c_w(ov, 0x54, 0x23);

	reg_w(ov, 0x2f, 0x80);

	if (ov->bridge == BRG_OV518PLUS) {
		reg_w(ov, 0x24, 0x94);
		reg_w(ov, 0x25, 0x90);
		ov518_reg_w32(ov, 0xc4,    400, 2);	
		ov518_reg_w32(ov, 0xc6,    540, 2);	
		ov518_reg_w32(ov, 0xc7,    540, 2);	
		ov518_reg_w32(ov, 0xc8,    108, 2);	
		ov518_reg_w32(ov, 0xca, 131098, 3);	
		ov518_reg_w32(ov, 0xcb,    532, 2);	
		ov518_reg_w32(ov, 0xcc,   2400, 2);	
		ov518_reg_w32(ov, 0xcd,     32, 2);	
		ov518_reg_w32(ov, 0xce,    608, 2);	
	} else {
		reg_w(ov, 0x24, 0x9f);
		reg_w(ov, 0x25, 0x90);
		ov518_reg_w32(ov, 0xc4,    400, 2);	
		ov518_reg_w32(ov, 0xc6,    500, 2);	
		ov518_reg_w32(ov, 0xc7,    500, 2);	
		ov518_reg_w32(ov, 0xc8,    142, 2);	
		ov518_reg_w32(ov, 0xca, 131098, 3);	
		ov518_reg_w32(ov, 0xcb,    532, 2);	
		ov518_reg_w32(ov, 0xcc,   2000, 2);	
		ov518_reg_w32(ov, 0xcd,     32, 2);	
		ov518_reg_w32(ov, 0xce,    608, 2);	
	}

	reg_w(ov, 0x2f, 0x80);

	if (ov51x_restart(ov) < 0)
		return -EIO;

	
	if (ov51x_reset(ov, OV511_RESET_NOREGS) < 0)
		return -EIO;

	return 0;
}


static int
mode_init_regs(struct usb_ov511 *ov,
	       int width, int height, int mode, int sub_flag)
{
	int rc = 0;

	if (!ov || !ov->dev)
		return -EFAULT;

	if (ov->bclass == BCL_OV518) {
		rc = ov518_mode_init_regs(ov, width, height, mode, sub_flag);
	} else {
		rc = ov511_mode_init_regs(ov, width, height, mode, sub_flag);
	}

	if (FATAL_ERROR(rc))
		return rc;

	switch (ov->sensor) {
	case SEN_OV7610:
	case SEN_OV7620:
	case SEN_OV76BE:
	case SEN_OV8600:
	case SEN_OV6620:
	case SEN_OV6630:
		rc = set_ov_sensor_window(ov, width, height, mode, sub_flag);
		break;
	case SEN_KS0127:
	case SEN_KS0127B:
		err("KS0127-series decoders not supported yet");
		rc = -EINVAL;
		break;
	case SEN_SAA7111A:



		PDEBUG(1, "SAA status = 0x%02X", i2c_r(ov, 0x1f));
		break;
	default:
		err("Unknown sensor");
		rc = -EINVAL;
	}

	if (FATAL_ERROR(rc))
		return rc;

	
	rc = sensor_set_auto_brightness(ov, ov->auto_brt);
	if (FATAL_ERROR(rc))
		return rc;

	rc = sensor_set_auto_exposure(ov, ov->auto_exp);
	if (FATAL_ERROR(rc))
		return rc;

	rc = sensor_set_banding_filter(ov, bandingfilter);
	if (FATAL_ERROR(rc))
		return rc;

	if (ov->lightfreq) {
		rc = sensor_set_light_freq(ov, lightfreq);
		if (FATAL_ERROR(rc))
			return rc;
	}

	rc = sensor_set_backlight(ov, ov->backlight);
	if (FATAL_ERROR(rc))
		return rc;

	rc = sensor_set_mirror(ov, ov->mirror);
	if (FATAL_ERROR(rc))
		return rc;

	return 0;
}


static int
ov51x_set_default_params(struct usb_ov511 *ov)
{
	int i;

	
	for (i = 0; i < OV511_NUMFRAMES; i++) {
		ov->frame[i].width = ov->maxwidth;
		ov->frame[i].height = ov->maxheight;
		ov->frame[i].bytes_read = 0;
		if (force_palette)
			ov->frame[i].format = force_palette;
		else
			ov->frame[i].format = VIDEO_PALETTE_YUV420;

		ov->frame[i].depth = get_depth(ov->frame[i].format);
	}

	PDEBUG(3, "%dx%d, %s", ov->maxwidth, ov->maxheight,
	       symbolic(v4l1_plist, ov->frame[0].format));

	
	if (mode_init_regs(ov, ov->maxwidth, ov->maxheight,
			   ov->frame[0].format, 0) < 0)
		return -EINVAL;

	return 0;
}




static int
decoder_set_input(struct usb_ov511 *ov, int input)
{
	PDEBUG(4, "port %d", input);

	switch (ov->sensor) {
	case SEN_SAA7111A:
	{
		
		i2c_w_mask(ov, 0x02, input, 0x07);
		
		i2c_w_mask(ov, 0x09, (input > 3) ? 0x80:0x00, 0x80);
		break;
	}
	default:
		return -EINVAL;
	}

	return 0;
}


static int
decoder_get_input_name(struct usb_ov511 *ov, int input, char *name)
{
	switch (ov->sensor) {
	case SEN_SAA7111A:
	{
		if (input < 0 || input > 7)
			return -EINVAL;
		else if (input < 4)
			sprintf(name, "CVBS-%d", input);
		else 
			sprintf(name, "S-Video-%d", input - 4);
		break;
	}
	default:
		sprintf(name, "%s", "Camera");
	}

	return 0;
}


static int
decoder_set_norm(struct usb_ov511 *ov, int norm)
{
	PDEBUG(4, "%d", norm);

	switch (ov->sensor) {
	case SEN_SAA7111A:
	{
		int reg_8, reg_e;

		if (norm == VIDEO_MODE_NTSC) {
			reg_8 = 0x40;	
			reg_e = 0x00;	
		} else if (norm == VIDEO_MODE_PAL) {
			reg_8 = 0x00;	
			reg_e = 0x00;	
		} else if (norm == VIDEO_MODE_AUTO) {
			reg_8 = 0x80;	
			reg_e = 0x00;	
		} else if (norm == VIDEO_MODE_SECAM) {
			reg_8 = 0x00;	
			reg_e = 0x50;	
		} else {
			return -EINVAL;
		}

		i2c_w_mask(ov, 0x08, reg_8, 0xc0);
		i2c_w_mask(ov, 0x0e, reg_e, 0x70);
		break;
	}
	default:
		return -EINVAL;
	}

	return 0;
}




static inline void
make_8x8(unsigned char *pIn, unsigned char *pOut, int w)
{
	unsigned char *pOut1 = pOut;
	int x, y;

	for (y = 0; y < 8; y++) {
		pOut1 = pOut;
		for (x = 0; x < 8; x++) {
			*pOut1++ = *pIn++;
		}
		pOut += w;
	}
}


static void
yuv400raw_to_yuv400p(struct ov511_frame *frame,
		     unsigned char *pIn0, unsigned char *pOut0)
{
	int x, y;
	unsigned char *pIn, *pOut, *pOutLine;

	
	pIn = pIn0;
	pOutLine = pOut0;
	for (y = 0; y < frame->rawheight - 1; y += 8) {
		pOut = pOutLine;
		for (x = 0; x < frame->rawwidth - 1; x += 8) {
			make_8x8(pIn, pOut, frame->rawwidth);
			pIn += 64;
			pOut += 8;
		}
		pOutLine += 8 * frame->rawwidth;
	}
}




static void
yuv420raw_to_yuv420p(struct ov511_frame *frame,
		     unsigned char *pIn0, unsigned char *pOut0)
{
	int k, x, y;
	unsigned char *pIn, *pOut, *pOutLine;
	const unsigned int a = frame->rawwidth * frame->rawheight;
	const unsigned int w = frame->rawwidth / 2;

	
	pIn = pIn0;
	pOutLine = pOut0 + a;
	for (y = 0; y < frame->rawheight - 1; y += 16) {
		pOut = pOutLine;
		for (x = 0; x < frame->rawwidth - 1; x += 16) {
			make_8x8(pIn, pOut, w);
			make_8x8(pIn + 64, pOut + a/4, w);
			pIn += 384;
			pOut += 8;
		}
		pOutLine += 8 * w;
	}

	
	pIn = pIn0 + 128;
	pOutLine = pOut0;
	k = 0;
	for (y = 0; y < frame->rawheight - 1; y += 8) {
		pOut = pOutLine;
		for (x = 0; x < frame->rawwidth - 1; x += 8) {
			make_8x8(pIn, pOut, frame->rawwidth);
			pIn += 64;
			pOut += 8;
			if ((++k) > 3) {
				k = 0;
				pIn += 128;
			}
		}
		pOutLine += 8 * frame->rawwidth;
	}
}



static int
request_decompressor(struct usb_ov511 *ov)
{
	if (ov->bclass == BCL_OV511 || ov->bclass == BCL_OV518) {
		err("No decompressor available");
	} else {
		err("Unknown bridge");
	}

	return -ENOSYS;
}

static void
decompress(struct usb_ov511 *ov, struct ov511_frame *frame,
	   unsigned char *pIn0, unsigned char *pOut0)
{
	if (!ov->decomp_ops)
		if (request_decompressor(ov))
			return;

}




static void
deinterlace(struct ov511_frame *frame, int rawformat,
	    unsigned char *pIn0, unsigned char *pOut0)
{
	const int fieldheight = frame->rawheight / 2;
	const int fieldpix = fieldheight * frame->rawwidth;
	const int w = frame->width;
	int x, y;
	unsigned char *pInEven, *pInOdd, *pOut;

	PDEBUG(5, "fieldheight=%d", fieldheight);

	if (frame->rawheight != frame->height) {
		err("invalid height");
		return;
	}

	if ((frame->rawwidth * 2) != frame->width) {
		err("invalid width");
		return;
	}

	
	pInOdd = pIn0;
	pInEven = pInOdd + fieldpix;
	pOut = pOut0;
	for (y = 0; y < fieldheight; y++) {
		for (x = 0; x < frame->rawwidth; x++) {
			*pOut = *pInEven;
			*(pOut+1) = *pInEven++;
			*(pOut+w) = *pInOdd;
			*(pOut+w+1) = *pInOdd++;
			pOut += 2;
		}
		pOut += w;
	}

	if (rawformat == RAWFMT_YUV420) {
	
		pInOdd = pIn0 + fieldpix * 2;
		pInEven = pInOdd + fieldpix / 4;
		for (y = 0; y < fieldheight / 2; y++) {
			for (x = 0; x < frame->rawwidth / 2; x++) {
				*pOut = *pInEven;
				*(pOut+1) = *pInEven++;
				*(pOut+w/2) = *pInOdd;
				*(pOut+w/2+1) = *pInOdd++;
				pOut += 2;
			}
			pOut += w/2;
		}
	
		pInOdd = pIn0 + fieldpix * 2 + fieldpix / 2;
		pInEven = pInOdd + fieldpix / 4;
		for (y = 0; y < fieldheight / 2; y++) {
			for (x = 0; x < frame->rawwidth / 2; x++) {
				*pOut = *pInEven;
				*(pOut+1) = *pInEven++;
				*(pOut+w/2) = *pInOdd;
				*(pOut+w/2+1) = *pInOdd++;
				pOut += 2;
			}
			pOut += w/2;
		}
	}
}

static void
ov51x_postprocess_grey(struct usb_ov511 *ov, struct ov511_frame *frame)
{
		
		if (ov->sensor == SEN_SAA7111A && frame->rawheight >= 480) {
			if (frame->compressed)
				decompress(ov, frame, frame->rawdata,
						 frame->tempdata);
			else
				yuv400raw_to_yuv400p(frame, frame->rawdata,
						     frame->tempdata);

			deinterlace(frame, RAWFMT_YUV400, frame->tempdata,
				    frame->data);
		} else {
			if (frame->compressed)
				decompress(ov, frame, frame->rawdata,
						 frame->data);
			else
				yuv400raw_to_yuv400p(frame, frame->rawdata,
						     frame->data);
		}
}


static void
ov51x_postprocess_yuv420(struct usb_ov511 *ov, struct ov511_frame *frame)
{
	
	if (ov->sensor == SEN_SAA7111A && frame->rawheight >= 480) {
		if (frame->compressed)
			decompress(ov, frame, frame->rawdata, frame->tempdata);
		else
			yuv420raw_to_yuv420p(frame, frame->rawdata,
					     frame->tempdata);

		deinterlace(frame, RAWFMT_YUV420, frame->tempdata,
			    frame->data);
	} else {
		if (frame->compressed)
			decompress(ov, frame, frame->rawdata, frame->data);
		else
			yuv420raw_to_yuv420p(frame, frame->rawdata,
					     frame->data);
	}
}


static void
ov51x_postprocess(struct usb_ov511 *ov, struct ov511_frame *frame)
{
	if (dumppix) {
		memset(frame->data, 0,
			MAX_DATA_SIZE(ov->maxwidth, ov->maxheight));
		PDEBUG(4, "Dumping %d bytes", frame->bytes_recvd);
		memcpy(frame->data, frame->rawdata, frame->bytes_recvd);
	} else {
		switch (frame->format) {
		case VIDEO_PALETTE_GREY:
			ov51x_postprocess_grey(ov, frame);
			break;
		case VIDEO_PALETTE_YUV420:
		case VIDEO_PALETTE_YUV420P:
			ov51x_postprocess_yuv420(ov, frame);
			break;
		default:
			err("Cannot convert data to %s",
			    symbolic(v4l1_plist, frame->format));
		}
	}
}



static inline void
ov511_move_data(struct usb_ov511 *ov, unsigned char *in, int n)
{
	int num, offset;
	int pnum = in[ov->packet_size - 1];		
	int max_raw = MAX_RAW_DATA_SIZE(ov->maxwidth, ov->maxheight);
	struct ov511_frame *frame = &ov->frame[ov->curframe];
	struct timeval *ts;

	

	if (printph) {
		dev_info(&ov->dev->dev,
			 "ph(%3d): %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x\n",
			 pnum, in[0], in[1], in[2], in[3], in[4], in[5], in[6],
			 in[7], in[8], in[9], in[10], in[11]);
	}

	
	if ((in[0] | in[1] | in[2] | in[3] | in[4] | in[5] | in[6] | in[7]) ||
	    (~in[8] & 0x08))
		goto check_middle;

	
	if (in[8] & 0x80) {
		ts = (struct timeval *)(frame->data
		      + MAX_FRAME_SIZE(ov->maxwidth, ov->maxheight));
		do_gettimeofday(ts);

		
		frame->rawwidth = ((int)(in[9]) + 1) * 8;
		frame->rawheight = ((int)(in[10]) + 1) * 8;

		PDEBUG(4, "Frame end, frame=%d, pnum=%d, w=%d, h=%d, recvd=%d",
			ov->curframe, pnum, frame->rawwidth, frame->rawheight,
			frame->bytes_recvd);

		
		RESTRICT_TO_RANGE(frame->rawwidth, ov->minwidth, ov->maxwidth);
		RESTRICT_TO_RANGE(frame->rawheight, ov->minheight,
				  ov->maxheight);

		
		RESTRICT_TO_RANGE(frame->bytes_recvd, 8, max_raw);

		if (frame->scanstate == STATE_LINES) {
			int nextf;

			frame->grabstate = FRAME_DONE;
			wake_up_interruptible(&frame->wq);

			
			nextf = (ov->curframe + 1) % OV511_NUMFRAMES;
			if (ov->frame[nextf].grabstate == FRAME_READY
			    || ov->frame[nextf].grabstate == FRAME_GRABBING) {
				ov->curframe = nextf;
				ov->frame[nextf].scanstate = STATE_SCANNING;
			} else {
				if (frame->grabstate == FRAME_DONE) {
					PDEBUG(4, "** Frame done **");
				} else {
					PDEBUG(4, "Frame not ready? state = %d",
						ov->frame[nextf].grabstate);
				}

				ov->curframe = -1;
			}
		} else {
			PDEBUG(5, "Frame done, but not scanning");
		}
		
	} else {
		
		PDEBUG(4, "Frame start, framenum = %d", ov->curframe);

		
		
		if (in[8] & 0x02) {
			frame->snapshot = 1;
			PDEBUG(3, "snapshot detected");
		}

		frame->scanstate = STATE_LINES;
		frame->bytes_recvd = 0;
		frame->compressed = in[8] & 0x40;
	}

check_middle:
	
	if (frame->scanstate != STATE_LINES) {
		PDEBUG(5, "Not in a frame; packet skipped");
		return;
	}

	
	if (frame->bytes_recvd == 0)
		offset = 9;
	else
		offset = 0;

	num = n - offset - 1;

	
	if (dumppix == 2) {
		frame->bytes_recvd += n - 1;
		if (frame->bytes_recvd <= max_raw)
			memcpy(frame->rawdata + frame->bytes_recvd - (n - 1),
				in, n - 1);
		else
			PDEBUG(3, "Raw data buffer overrun!! (%d)",
				frame->bytes_recvd - max_raw);
	} else if (!frame->compressed && !remove_zeros) {
		frame->bytes_recvd += num;
		if (frame->bytes_recvd <= max_raw)
			memcpy(frame->rawdata + frame->bytes_recvd - num,
				in + offset, num);
		else
			PDEBUG(3, "Raw data buffer overrun!! (%d)",
				frame->bytes_recvd - max_raw);
	} else { 
		int b, read = 0, allzero, copied = 0;
		if (offset) {
			frame->bytes_recvd += 32 - offset;	
			memcpy(frame->rawdata,	in + offset, 32 - offset);
			read += 32;
		}

		while (read < n - 1) {
			allzero = 1;
			for (b = 0; b < 32; b++) {
				if (in[read + b]) {
					allzero = 0;
					break;
				}
			}

			if (allzero) {
				
			} else {
				if (frame->bytes_recvd + copied + 32 <= max_raw)
				{
					memcpy(frame->rawdata
						+ frame->bytes_recvd + copied,
						in + read, 32);
					copied += 32;
				} else {
					PDEBUG(3, "Raw data buffer overrun!!");
				}
			}
			read += 32;
		}

		frame->bytes_recvd += copied;
	}
}

static inline void
ov518_move_data(struct usb_ov511 *ov, unsigned char *in, int n)
{
	int max_raw = MAX_RAW_DATA_SIZE(ov->maxwidth, ov->maxheight);
	struct ov511_frame *frame = &ov->frame[ov->curframe];
	struct timeval *ts;

	
	if (ov->packet_numbering)
		--n;

	
	if ((!(in[0] | in[1] | in[2] | in[3] | in[5])) && in[6]) {
		if (printph) {
			dev_info(&ov->dev->dev,
				 "ph: %2x %2x %2x %2x %2x %2x %2x %2x\n",
				 in[0], in[1], in[2], in[3], in[4], in[5],
				 in[6], in[7]);
		}

		if (frame->scanstate == STATE_LINES) {
			PDEBUG(4, "Detected frame end/start");
			goto eof;
		} else { 
			
			PDEBUG(4, "Frame start, framenum = %d", ov->curframe);
			goto sof;
		}
	} else {
		goto check_middle;
	}

eof:
	ts = (struct timeval *)(frame->data
	      + MAX_FRAME_SIZE(ov->maxwidth, ov->maxheight));
	do_gettimeofday(ts);

	PDEBUG(4, "Frame end, curframe = %d, hw=%d, vw=%d, recvd=%d",
		ov->curframe,
		(int)(in[9]), (int)(in[10]), frame->bytes_recvd);

	
	
	frame->rawwidth = frame->width;
	frame->rawheight = frame->height;

	
	RESTRICT_TO_RANGE(frame->rawwidth, ov->minwidth, ov->maxwidth);
	RESTRICT_TO_RANGE(frame->rawheight, ov->minheight, ov->maxheight);

	
	RESTRICT_TO_RANGE(frame->bytes_recvd, 8, max_raw);

	if (frame->scanstate == STATE_LINES) {
		int nextf;

		frame->grabstate = FRAME_DONE;
		wake_up_interruptible(&frame->wq);

		
		nextf = (ov->curframe + 1) % OV511_NUMFRAMES;
		if (ov->frame[nextf].grabstate == FRAME_READY
		    || ov->frame[nextf].grabstate == FRAME_GRABBING) {
			ov->curframe = nextf;
			ov->frame[nextf].scanstate = STATE_SCANNING;
			frame = &ov->frame[nextf];
		} else {
			if (frame->grabstate == FRAME_DONE) {
				PDEBUG(4, "** Frame done **");
			} else {
				PDEBUG(4, "Frame not ready? state = %d",
				       ov->frame[nextf].grabstate);
			}

			ov->curframe = -1;
			PDEBUG(4, "SOF dropped (no active frame)");
			return;  
		}
	}
sof:
	PDEBUG(4, "Starting capture on frame %d", frame->framenum);


#if 0
	
	
	if (in[8] & 0x02) {
		frame->snapshot = 1;
		PDEBUG(3, "snapshot detected");
	}
#endif
	frame->scanstate = STATE_LINES;
	frame->bytes_recvd = 0;
	frame->compressed = 1;

check_middle:
	
	if (frame->scanstate != STATE_LINES) {
		PDEBUG(4, "scanstate: no SOF yet");
		return;
	}

	
	if (dumppix == 2) {
		frame->bytes_recvd += n;
		if (frame->bytes_recvd <= max_raw)
			memcpy(frame->rawdata + frame->bytes_recvd - n, in, n);
		else
			PDEBUG(3, "Raw data buffer overrun!! (%d)",
				frame->bytes_recvd - max_raw);
	} else {
		

		int b, read = 0, allzero, copied = 0;

		while (read < n) {
			allzero = 1;
			for (b = 0; b < 8; b++) {
				if (in[read + b]) {
					allzero = 0;
					break;
				}
			}

			if (allzero) {
			
			} else {
				if (frame->bytes_recvd + copied + 8 <= max_raw)
				{
					memcpy(frame->rawdata
						+ frame->bytes_recvd + copied,
						in + read, 8);
					copied += 8;
				} else {
					PDEBUG(3, "Raw data buffer overrun!!");
				}
			}
			read += 8;
		}
		frame->bytes_recvd += copied;
	}
}

static void
ov51x_isoc_irq(struct urb *urb)
{
	int i;
	struct usb_ov511 *ov;
	struct ov511_sbuf *sbuf;

	if (!urb->context) {
		PDEBUG(4, "no context");
		return;
	}

	sbuf = urb->context;
	ov = sbuf->ov;

	if (!ov || !ov->dev || !ov->user) {
		PDEBUG(4, "no device, or not open");
		return;
	}

	if (!ov->streaming) {
		PDEBUG(4, "hmmm... not streaming, but got interrupt");
		return;
	}

	if (urb->status == -ENOENT || urb->status == -ECONNRESET) {
		PDEBUG(4, "URB unlinked");
		return;
	}

	if (urb->status != -EINPROGRESS && urb->status != 0) {
		err("ERROR: urb->status=%d: %s", urb->status,
		    symbolic(urb_errlist, urb->status));
	}

	
	PDEBUG(5, "sbuf[%d]: Moving %d packets", sbuf->n,
	       urb->number_of_packets);
	for (i = 0; i < urb->number_of_packets; i++) {
		
		if (ov->curframe >= 0) {
			int n = urb->iso_frame_desc[i].actual_length;
			int st = urb->iso_frame_desc[i].status;
			unsigned char *cdata;

			urb->iso_frame_desc[i].actual_length = 0;
			urb->iso_frame_desc[i].status = 0;

			cdata = urb->transfer_buffer
				+ urb->iso_frame_desc[i].offset;

			if (!n) {
				PDEBUG(4, "Zero-length packet");
				continue;
			}

			if (st)
				PDEBUG(2, "data error: [%d] len=%d, status=%d",
				       i, n, st);

			if (ov->bclass == BCL_OV511)
				ov511_move_data(ov, cdata, n);
			else if (ov->bclass == BCL_OV518)
				ov518_move_data(ov, cdata, n);
			else
				err("Unknown bridge device (%d)", ov->bridge);

		} else if (waitqueue_active(&ov->wq)) {
			wake_up_interruptible(&ov->wq);
		}
	}

	
	urb->dev = ov->dev;
	if ((i = usb_submit_urb(urb, GFP_ATOMIC)) != 0)
		err("usb_submit_urb() ret %d", i);

	return;
}



static int
ov51x_init_isoc(struct usb_ov511 *ov)
{
	struct urb *urb;
	int fx, err, n, i, size;

	PDEBUG(3, "*** Initializing capture ***");

	ov->curframe = -1;

	if (ov->bridge == BRG_OV511) {
		if (cams == 1)
			size = 993;
		else if (cams == 2)
			size = 513;
		else if (cams == 3 || cams == 4)
			size = 257;
		else {
			err("\"cams\" parameter too high!");
			return -1;
		}
	} else if (ov->bridge == BRG_OV511PLUS) {
		if (cams == 1)
			size = 961;
		else if (cams == 2)
			size = 513;
		else if (cams == 3 || cams == 4)
			size = 257;
		else if (cams >= 5 && cams <= 8)
			size = 129;
		else if (cams >= 9 && cams <= 31)
			size = 33;
		else {
			err("\"cams\" parameter too high!");
			return -1;
		}
	} else if (ov->bclass == BCL_OV518) {
		if (cams == 1)
			size = 896;
		else if (cams == 2)
			size = 512;
		else if (cams == 3 || cams == 4)
			size = 256;
		else if (cams >= 5 && cams <= 8)
			size = 128;
		else {
			err("\"cams\" parameter too high!");
			return -1;
		}
	} else {
		err("invalid bridge type");
		return -1;
	}

	
	if (ov->bclass == BCL_OV518) {
		if (packetsize == -1) {
			ov518_set_packet_size(ov, 640);
		} else {
			dev_info(&ov->dev->dev, "Forcing packet size to %d\n",
				 packetsize);
			ov518_set_packet_size(ov, packetsize);
		}
	} else {
		if (packetsize == -1) {
			ov511_set_packet_size(ov, size);
		} else {
			dev_info(&ov->dev->dev, "Forcing packet size to %d\n",
				 packetsize);
			ov511_set_packet_size(ov, packetsize);
		}
	}

	for (n = 0; n < OV511_NUMSBUF; n++) {
		urb = usb_alloc_urb(FRAMES_PER_DESC, GFP_KERNEL);
		if (!urb) {
			err("init isoc: usb_alloc_urb ret. NULL");
			for (i = 0; i < n; i++)
				usb_free_urb(ov->sbuf[i].urb);
			return -ENOMEM;
		}
		ov->sbuf[n].urb = urb;
		urb->dev = ov->dev;
		urb->context = &ov->sbuf[n];
		urb->pipe = usb_rcvisocpipe(ov->dev, OV511_ENDPOINT_ADDRESS);
		urb->transfer_flags = URB_ISO_ASAP;
		urb->transfer_buffer = ov->sbuf[n].data;
		urb->complete = ov51x_isoc_irq;
		urb->number_of_packets = FRAMES_PER_DESC;
		urb->transfer_buffer_length = ov->packet_size * FRAMES_PER_DESC;
		urb->interval = 1;
		for (fx = 0; fx < FRAMES_PER_DESC; fx++) {
			urb->iso_frame_desc[fx].offset = ov->packet_size * fx;
			urb->iso_frame_desc[fx].length = ov->packet_size;
		}
	}

	ov->streaming = 1;

	for (n = 0; n < OV511_NUMSBUF; n++) {
		ov->sbuf[n].urb->dev = ov->dev;
		err = usb_submit_urb(ov->sbuf[n].urb, GFP_KERNEL);
		if (err) {
			err("init isoc: usb_submit_urb(%d) ret %d", n, err);
			return err;
		}
	}

	return 0;
}

static void
ov51x_unlink_isoc(struct usb_ov511 *ov)
{
	int n;

	
	for (n = OV511_NUMSBUF - 1; n >= 0; n--) {
		if (ov->sbuf[n].urb) {
			usb_kill_urb(ov->sbuf[n].urb);
			usb_free_urb(ov->sbuf[n].urb);
			ov->sbuf[n].urb = NULL;
		}
	}
}

static void
ov51x_stop_isoc(struct usb_ov511 *ov)
{
	if (!ov->streaming || !ov->dev)
		return;

	PDEBUG(3, "*** Stopping capture ***");

	if (ov->bclass == BCL_OV518)
		ov518_set_packet_size(ov, 0);
	else
		ov511_set_packet_size(ov, 0);

	ov->streaming = 0;

	ov51x_unlink_isoc(ov);
}

static int
ov51x_new_frame(struct usb_ov511 *ov, int framenum)
{
	struct ov511_frame *frame;
	int newnum;

	PDEBUG(4, "ov->curframe = %d, framenum = %d", ov->curframe, framenum);

	if (!ov->dev)
		return -1;

	
	
	if (ov->curframe == -1) {
		newnum = (framenum - 1 + OV511_NUMFRAMES) % OV511_NUMFRAMES;
		if (ov->frame[newnum].grabstate == FRAME_READY)
			framenum = newnum;
	} else
		return 0;

	frame = &ov->frame[framenum];

	PDEBUG(4, "framenum = %d, width = %d, height = %d", framenum,
	       frame->width, frame->height);

	frame->grabstate = FRAME_GRABBING;
	frame->scanstate = STATE_SCANNING;
	frame->snapshot = 0;

	ov->curframe = framenum;

	
	if (frame->width > ov->maxwidth)
		frame->width = ov->maxwidth;

	frame->width &= ~7L;		

	if (frame->height > ov->maxheight)
		frame->height = ov->maxheight;

	frame->height &= ~3L;		

	return 0;
}




static void
ov51x_do_dealloc(struct usb_ov511 *ov)
{
	int i;
	PDEBUG(4, "entered");

	if (ov->fbuf) {
		rvfree(ov->fbuf, OV511_NUMFRAMES
		       * MAX_DATA_SIZE(ov->maxwidth, ov->maxheight));
		ov->fbuf = NULL;
	}

	vfree(ov->rawfbuf);
	ov->rawfbuf = NULL;

	vfree(ov->tempfbuf);
	ov->tempfbuf = NULL;

	for (i = 0; i < OV511_NUMSBUF; i++) {
		kfree(ov->sbuf[i].data);
		ov->sbuf[i].data = NULL;
	}

	for (i = 0; i < OV511_NUMFRAMES; i++) {
		ov->frame[i].data = NULL;
		ov->frame[i].rawdata = NULL;
		ov->frame[i].tempdata = NULL;
		if (ov->frame[i].compbuf) {
			free_page((unsigned long) ov->frame[i].compbuf);
			ov->frame[i].compbuf = NULL;
		}
	}

	PDEBUG(4, "buffer memory deallocated");
	ov->buf_state = BUF_NOT_ALLOCATED;
	PDEBUG(4, "leaving");
}

static int
ov51x_alloc(struct usb_ov511 *ov)
{
	int i;
	const int w = ov->maxwidth;
	const int h = ov->maxheight;
	const int data_bufsize = OV511_NUMFRAMES * MAX_DATA_SIZE(w, h);
	const int raw_bufsize = OV511_NUMFRAMES * MAX_RAW_DATA_SIZE(w, h);

	PDEBUG(4, "entered");
	mutex_lock(&ov->buf_lock);

	if (ov->buf_state == BUF_ALLOCATED)
		goto out;

	ov->fbuf = rvmalloc(data_bufsize);
	if (!ov->fbuf)
		goto error;

	ov->rawfbuf = vmalloc(raw_bufsize);
	if (!ov->rawfbuf)
		goto error;

	memset(ov->rawfbuf, 0, raw_bufsize);

	ov->tempfbuf = vmalloc(raw_bufsize);
	if (!ov->tempfbuf)
		goto error;

	memset(ov->tempfbuf, 0, raw_bufsize);

	for (i = 0; i < OV511_NUMSBUF; i++) {
		ov->sbuf[i].data = kmalloc(FRAMES_PER_DESC *
			MAX_FRAME_SIZE_PER_DESC, GFP_KERNEL);
		if (!ov->sbuf[i].data)
			goto error;

		PDEBUG(4, "sbuf[%d] @ %p", i, ov->sbuf[i].data);
	}

	for (i = 0; i < OV511_NUMFRAMES; i++) {
		ov->frame[i].data = ov->fbuf + i * MAX_DATA_SIZE(w, h);
		ov->frame[i].rawdata = ov->rawfbuf
		 + i * MAX_RAW_DATA_SIZE(w, h);
		ov->frame[i].tempdata = ov->tempfbuf
		 + i * MAX_RAW_DATA_SIZE(w, h);

		ov->frame[i].compbuf =
		 (unsigned char *) __get_free_page(GFP_KERNEL);
		if (!ov->frame[i].compbuf)
			goto error;

		PDEBUG(4, "frame[%d] @ %p", i, ov->frame[i].data);
	}

	ov->buf_state = BUF_ALLOCATED;
out:
	mutex_unlock(&ov->buf_lock);
	PDEBUG(4, "leaving");
	return 0;
error:
	ov51x_do_dealloc(ov);
	mutex_unlock(&ov->buf_lock);
	PDEBUG(4, "errored");
	return -ENOMEM;
}

static void
ov51x_dealloc(struct usb_ov511 *ov)
{
	PDEBUG(4, "entered");
	mutex_lock(&ov->buf_lock);
	ov51x_do_dealloc(ov);
	mutex_unlock(&ov->buf_lock);
	PDEBUG(4, "leaving");
}



static int
ov51x_v4l1_open(struct file *file)
{
	struct video_device *vdev = video_devdata(file);
	struct usb_ov511 *ov = video_get_drvdata(vdev);
	int err, i;

	PDEBUG(4, "opening");

	mutex_lock(&ov->lock);

	err = -EBUSY;
	if (ov->user)
		goto out;

	ov->sub_flag = 0;

	
	err = ov51x_set_default_params(ov);
	if (err < 0)
		goto out;

	
	for (i = 0; i < OV511_NUMFRAMES; i++) {
		ov->frame[i].grabstate = FRAME_UNUSED;
		ov->frame[i].bytes_read = 0;
	}

	
	if (ov->compress && !ov->decomp_ops) {
		err = request_decompressor(ov);
		if (err && !dumppix)
			goto out;
	}

	err = ov51x_alloc(ov);
	if (err < 0)
		goto out;

	err = ov51x_init_isoc(ov);
	if (err) {
		ov51x_dealloc(ov);
		goto out;
	}

	ov->user++;
	file->private_data = vdev;

	if (ov->led_policy == LED_AUTO)
		ov51x_led_control(ov, 1);

out:
	mutex_unlock(&ov->lock);
	return err;
}

static int
ov51x_v4l1_close(struct file *file)
{
	struct video_device *vdev = file->private_data;
	struct usb_ov511 *ov = video_get_drvdata(vdev);

	PDEBUG(4, "ov511_close");

	mutex_lock(&ov->lock);

	ov->user--;
	ov51x_stop_isoc(ov);

	if (ov->led_policy == LED_AUTO)
		ov51x_led_control(ov, 0);

	if (ov->dev)
		ov51x_dealloc(ov);

	mutex_unlock(&ov->lock);

	
	if (!ov->dev) {
		mutex_lock(&ov->cbuf_lock);
		kfree(ov->cbuf);
		ov->cbuf = NULL;
		mutex_unlock(&ov->cbuf_lock);

		ov51x_dealloc(ov);
		kfree(ov);
		ov = NULL;
	}

	file->private_data = NULL;
	return 0;
}


static long
ov51x_v4l1_ioctl_internal(struct file *file, unsigned int cmd, void *arg)
{
	struct video_device *vdev = file->private_data;
	struct usb_ov511 *ov = video_get_drvdata(vdev);
	PDEBUG(5, "IOCtl: 0x%X", cmd);

	if (!ov->dev)
		return -EIO;

	switch (cmd) {
	case VIDIOCGCAP:
	{
		struct video_capability *b = arg;

		PDEBUG(4, "VIDIOCGCAP");

		memset(b, 0, sizeof(struct video_capability));
		sprintf(b->name, "%s USB Camera",
			symbolic(brglist, ov->bridge));
		b->type = VID_TYPE_CAPTURE | VID_TYPE_SUBCAPTURE;
		b->channels = ov->num_inputs;
		b->audios = 0;
		b->maxwidth = ov->maxwidth;
		b->maxheight = ov->maxheight;
		b->minwidth = ov->minwidth;
		b->minheight = ov->minheight;

		return 0;
	}
	case VIDIOCGCHAN:
	{
		struct video_channel *v = arg;

		PDEBUG(4, "VIDIOCGCHAN");

		if ((unsigned)(v->channel) >= ov->num_inputs) {
			err("Invalid channel (%d)", v->channel);
			return -EINVAL;
		}

		v->norm = ov->norm;
		v->type = VIDEO_TYPE_CAMERA;
		v->flags = 0;

		v->tuners = 0;
		decoder_get_input_name(ov, v->channel, v->name);

		return 0;
	}
	case VIDIOCSCHAN:
	{
		struct video_channel *v = arg;
		int err;

		PDEBUG(4, "VIDIOCSCHAN");

		
		if (!ov->has_decoder) {
			if (v->channel == 0)
				return 0;
			else
				return -EINVAL;
		}

		if (v->norm != VIDEO_MODE_PAL &&
		    v->norm != VIDEO_MODE_NTSC &&
		    v->norm != VIDEO_MODE_SECAM &&
		    v->norm != VIDEO_MODE_AUTO) {
			err("Invalid norm (%d)", v->norm);
			return -EINVAL;
		}

		if ((unsigned)(v->channel) >= ov->num_inputs) {
			err("Invalid channel (%d)", v->channel);
			return -EINVAL;
		}

		err = decoder_set_input(ov, v->channel);
		if (err)
			return err;

		err = decoder_set_norm(ov, v->norm);
		if (err)
			return err;

		return 0;
	}
	case VIDIOCGPICT:
	{
		struct video_picture *p = arg;

		PDEBUG(4, "VIDIOCGPICT");

		memset(p, 0, sizeof(struct video_picture));
		if (sensor_get_picture(ov, p))
			return -EIO;

		
		p->depth = ov->frame[0].depth;
		p->palette = ov->frame[0].format;

		return 0;
	}
	case VIDIOCSPICT:
	{
		struct video_picture *p = arg;
		int i, rc;

		PDEBUG(4, "VIDIOCSPICT");

		if (!get_depth(p->palette))
			return -EINVAL;

		if (sensor_set_picture(ov, p))
			return -EIO;

		if (force_palette && p->palette != force_palette) {
			dev_info(&ov->dev->dev, "Palette rejected (%s)\n",
			     symbolic(v4l1_plist, p->palette));
			return -EINVAL;
		}

		
		if (p->palette != ov->frame[0].format) {
			PDEBUG(4, "Detected format change");

			rc = ov51x_wait_frames_inactive(ov);
			if (rc)
				return rc;

			mode_init_regs(ov, ov->frame[0].width,
				ov->frame[0].height, p->palette, ov->sub_flag);
		}

		PDEBUG(4, "Setting depth=%d, palette=%s",
		       p->depth, symbolic(v4l1_plist, p->palette));

		for (i = 0; i < OV511_NUMFRAMES; i++) {
			ov->frame[i].depth = p->depth;
			ov->frame[i].format = p->palette;
		}

		return 0;
	}
	case VIDIOCGCAPTURE:
	{
		int *vf = arg;

		PDEBUG(4, "VIDIOCGCAPTURE");

		ov->sub_flag = *vf;
		return 0;
	}
	case VIDIOCSCAPTURE:
	{
		struct video_capture *vc = arg;

		PDEBUG(4, "VIDIOCSCAPTURE");

		if (vc->flags)
			return -EINVAL;
		if (vc->decimation)
			return -EINVAL;

		vc->x &= ~3L;
		vc->y &= ~1L;
		vc->y &= ~31L;

		if (vc->width == 0)
			vc->width = 32;

		vc->height /= 16;
		vc->height *= 16;
		if (vc->height == 0)
			vc->height = 16;

		ov->subx = vc->x;
		ov->suby = vc->y;
		ov->subw = vc->width;
		ov->subh = vc->height;

		return 0;
	}
	case VIDIOCSWIN:
	{
		struct video_window *vw = arg;
		int i, rc;

		PDEBUG(4, "VIDIOCSWIN: %dx%d", vw->width, vw->height);

#if 0
		if (vw->flags)
			return -EINVAL;
		if (vw->clipcount)
			return -EINVAL;
		if (vw->height != ov->maxheight)
			return -EINVAL;
		if (vw->width != ov->maxwidth)
			return -EINVAL;
#endif

		rc = ov51x_wait_frames_inactive(ov);
		if (rc)
			return rc;

		rc = mode_init_regs(ov, vw->width, vw->height,
			ov->frame[0].format, ov->sub_flag);
		if (rc < 0)
			return rc;

		for (i = 0; i < OV511_NUMFRAMES; i++) {
			ov->frame[i].width = vw->width;
			ov->frame[i].height = vw->height;
		}

		return 0;
	}
	case VIDIOCGWIN:
	{
		struct video_window *vw = arg;

		memset(vw, 0, sizeof(struct video_window));
		vw->x = 0;		
		vw->y = 0;
		vw->width = ov->frame[0].width;
		vw->height = ov->frame[0].height;
		vw->flags = 30;

		PDEBUG(4, "VIDIOCGWIN: %dx%d", vw->width, vw->height);

		return 0;
	}
	case VIDIOCGMBUF:
	{
		struct video_mbuf *vm = arg;
		int i;

		PDEBUG(4, "VIDIOCGMBUF");

		memset(vm, 0, sizeof(struct video_mbuf));
		vm->size = OV511_NUMFRAMES
			   * MAX_DATA_SIZE(ov->maxwidth, ov->maxheight);
		vm->frames = OV511_NUMFRAMES;

		vm->offsets[0] = 0;
		for (i = 1; i < OV511_NUMFRAMES; i++) {
			vm->offsets[i] = vm->offsets[i-1]
			   + MAX_DATA_SIZE(ov->maxwidth, ov->maxheight);
		}

		return 0;
	}
	case VIDIOCMCAPTURE:
	{
		struct video_mmap *vm = arg;
		int rc, depth;
		unsigned int f = vm->frame;

		PDEBUG(4, "VIDIOCMCAPTURE: frame: %d, %dx%d, %s", f, vm->width,
			vm->height, symbolic(v4l1_plist, vm->format));

		depth = get_depth(vm->format);
		if (!depth) {
			PDEBUG(2, "VIDIOCMCAPTURE: invalid format (%s)",
			       symbolic(v4l1_plist, vm->format));
			return -EINVAL;
		}

		if (f >= OV511_NUMFRAMES) {
			err("VIDIOCMCAPTURE: invalid frame (%d)", f);
			return -EINVAL;
		}

		if (vm->width > ov->maxwidth
		    || vm->height > ov->maxheight) {
			err("VIDIOCMCAPTURE: requested dimensions too big");
			return -EINVAL;
		}

		if (ov->frame[f].grabstate == FRAME_GRABBING) {
			PDEBUG(4, "VIDIOCMCAPTURE: already grabbing");
			return -EBUSY;
		}

		if (force_palette && (vm->format != force_palette)) {
			PDEBUG(2, "palette rejected (%s)",
			       symbolic(v4l1_plist, vm->format));
			return -EINVAL;
		}

		if ((ov->frame[f].width != vm->width) ||
		    (ov->frame[f].height != vm->height) ||
		    (ov->frame[f].format != vm->format) ||
		    (ov->frame[f].sub_flag != ov->sub_flag) ||
		    (ov->frame[f].depth != depth)) {
			PDEBUG(4, "VIDIOCMCAPTURE: change in image parameters");

			rc = ov51x_wait_frames_inactive(ov);
			if (rc)
				return rc;

			rc = mode_init_regs(ov, vm->width, vm->height,
				vm->format, ov->sub_flag);
#if 0
			if (rc < 0) {
				PDEBUG(1, "Got error while initializing regs ");
				return ret;
			}
#endif
			ov->frame[f].width = vm->width;
			ov->frame[f].height = vm->height;
			ov->frame[f].format = vm->format;
			ov->frame[f].sub_flag = ov->sub_flag;
			ov->frame[f].depth = depth;
		}

		
		ov->frame[f].grabstate = FRAME_READY;

		PDEBUG(4, "VIDIOCMCAPTURE: renewing frame %d", f);

		return ov51x_new_frame(ov, f);
	}
	case VIDIOCSYNC:
	{
		unsigned int fnum = *((unsigned int *) arg);
		struct ov511_frame *frame;
		int rc;

		if (fnum >= OV511_NUMFRAMES) {
			err("VIDIOCSYNC: invalid frame (%d)", fnum);
			return -EINVAL;
		}

		frame = &ov->frame[fnum];

		PDEBUG(4, "syncing to frame %d, grabstate = %d", fnum,
		       frame->grabstate);

		switch (frame->grabstate) {
		case FRAME_UNUSED:
			return -EINVAL;
		case FRAME_READY:
		case FRAME_GRABBING:
		case FRAME_ERROR:
redo:
			if (!ov->dev)
				return -EIO;

			rc = wait_event_interruptible(frame->wq,
			    (frame->grabstate == FRAME_DONE)
			    || (frame->grabstate == FRAME_ERROR));

			if (rc)
				return rc;

			if (frame->grabstate == FRAME_ERROR) {
				if ((rc = ov51x_new_frame(ov, fnum)) < 0)
					return rc;
				goto redo;
			}
			
		case FRAME_DONE:
			if (ov->snap_enabled && !frame->snapshot) {
				if ((rc = ov51x_new_frame(ov, fnum)) < 0)
					return rc;
				goto redo;
			}

			frame->grabstate = FRAME_UNUSED;

			
			
			if ((ov->snap_enabled) && (frame->snapshot)) {
				frame->snapshot = 0;
				ov51x_clear_snapshot(ov);
			}

			
			ov51x_postprocess(ov, frame);

			break;
		} 

		return 0;
	}
	case VIDIOCGFBUF:
	{
		struct video_buffer *vb = arg;

		PDEBUG(4, "VIDIOCGFBUF");

		memset(vb, 0, sizeof(struct video_buffer));

		return 0;
	}
	case VIDIOCGUNIT:
	{
		struct video_unit *vu = arg;

		PDEBUG(4, "VIDIOCGUNIT");

		memset(vu, 0, sizeof(struct video_unit));

		vu->video = ov->vdev->minor;
		vu->vbi = VIDEO_NO_UNIT;
		vu->radio = VIDEO_NO_UNIT;
		vu->audio = VIDEO_NO_UNIT;
		vu->teletext = VIDEO_NO_UNIT;

		return 0;
	}
	case OV511IOC_WI2C:
	{
		struct ov511_i2c_struct *w = arg;

		return i2c_w_slave(ov, w->slave, w->reg, w->value, w->mask);
	}
	case OV511IOC_RI2C:
	{
		struct ov511_i2c_struct *r = arg;
		int rc;

		rc = i2c_r_slave(ov, r->slave, r->reg);
		if (rc < 0)
			return rc;

		r->value = rc;
		return 0;
	}
	default:
		PDEBUG(3, "Unsupported IOCtl: 0x%X", cmd);
		return -ENOIOCTLCMD;
	} 

	return 0;
}

static long
ov51x_v4l1_ioctl(struct file *file,
		 unsigned int cmd, unsigned long arg)
{
	struct video_device *vdev = file->private_data;
	struct usb_ov511 *ov = video_get_drvdata(vdev);
	int rc;

	if (mutex_lock_interruptible(&ov->lock))
		return -EINTR;

	rc = video_usercopy(file, cmd, arg, ov51x_v4l1_ioctl_internal);

	mutex_unlock(&ov->lock);
	return rc;
}

static ssize_t
ov51x_v4l1_read(struct file *file, char __user *buf, size_t cnt, loff_t *ppos)
{
	struct video_device *vdev = file->private_data;
	int noblock = file->f_flags&O_NONBLOCK;
	unsigned long count = cnt;
	struct usb_ov511 *ov = video_get_drvdata(vdev);
	int i, rc = 0, frmx = -1;
	struct ov511_frame *frame;

	if (mutex_lock_interruptible(&ov->lock))
		return -EINTR;

	PDEBUG(4, "%ld bytes, noblock=%d", count, noblock);

	if (!vdev || !buf) {
		rc = -EFAULT;
		goto error;
	}

	if (!ov->dev) {
		rc = -EIO;
		goto error;
	}


	
	if (ov->frame[0].grabstate >= FRAME_DONE)	
		frmx = 0;
	else if (ov->frame[1].grabstate >= FRAME_DONE)
		frmx = 1;

	
	if (noblock && (frmx == -1)) {
		rc = -EAGAIN;
		goto error;
	}

	
	
	if (frmx == -1) {
		if (ov->frame[0].grabstate == FRAME_GRABBING)
			frmx = 0;
		else if (ov->frame[1].grabstate == FRAME_GRABBING)
			frmx = 1;
	}

	
	if (frmx == -1) {
		if ((rc = ov51x_new_frame(ov, frmx = 0))) {
			err("read: ov51x_new_frame error");
			goto error;
		}
	}

	frame = &ov->frame[frmx];

restart:
	if (!ov->dev) {
		rc = -EIO;
		goto error;
	}

	
	PDEBUG(4, "Waiting image grabbing");
	rc = wait_event_interruptible(frame->wq,
		(frame->grabstate == FRAME_DONE)
		|| (frame->grabstate == FRAME_ERROR));

	if (rc)
		goto error;

	PDEBUG(4, "Got image, frame->grabstate = %d", frame->grabstate);
	PDEBUG(4, "bytes_recvd = %d", frame->bytes_recvd);

	if (frame->grabstate == FRAME_ERROR) {
		frame->bytes_read = 0;
		err("** ick! ** Errored frame %d", ov->curframe);
		if (ov51x_new_frame(ov, frmx)) {
			err("read: ov51x_new_frame error");
			goto error;
		}
		goto restart;
	}


	
	if (ov->snap_enabled)
		PDEBUG(4, "Waiting snapshot frame");
	if (ov->snap_enabled && !frame->snapshot) {
		frame->bytes_read = 0;
		if ((rc = ov51x_new_frame(ov, frmx))) {
			err("read: ov51x_new_frame error");
			goto error;
		}
		goto restart;
	}

	
	if (ov->snap_enabled && frame->snapshot) {
		frame->snapshot = 0;
		ov51x_clear_snapshot(ov);
	}

	
	ov51x_postprocess(ov, frame);

	PDEBUG(4, "frmx=%d, bytes_read=%ld, length=%ld", frmx,
		frame->bytes_read,
		get_frame_length(frame));

	




	
	count = get_frame_length(frame);

	PDEBUG(4, "Copy to user space: %ld bytes", count);
	if ((i = copy_to_user(buf, frame->data + frame->bytes_read, count))) {
		PDEBUG(4, "Copy failed! %d bytes not copied", i);
		rc = -EFAULT;
		goto error;
	}

	frame->bytes_read += count;
	PDEBUG(4, "{copy} count used=%ld, new bytes_read=%ld",
		count, frame->bytes_read);

	
	if (frame->bytes_read
	    >= get_frame_length(frame)) {
		frame->bytes_read = 0;


		
		ov->frame[frmx].grabstate = FRAME_UNUSED;
		if ((rc = ov51x_new_frame(ov, !frmx))) {
			err("ov51x_new_frame returned error");
			goto error;
		}
	}

	PDEBUG(4, "read finished, returning %ld (sweet)", count);

	mutex_unlock(&ov->lock);
	return count;

error:
	mutex_unlock(&ov->lock);
	return rc;
}

static int
ov51x_v4l1_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct video_device *vdev = file->private_data;
	unsigned long start = vma->vm_start;
	unsigned long size  = vma->vm_end - vma->vm_start;
	struct usb_ov511 *ov = video_get_drvdata(vdev);
	unsigned long page, pos;

	if (ov->dev == NULL)
		return -EIO;

	PDEBUG(4, "mmap: %ld (%lX) bytes", size, size);

	if (size > (((OV511_NUMFRAMES
		      * MAX_DATA_SIZE(ov->maxwidth, ov->maxheight)
		      + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))))
		return -EINVAL;

	if (mutex_lock_interruptible(&ov->lock))
		return -EINTR;

	pos = (unsigned long)ov->fbuf;
	while (size > 0) {
		page = vmalloc_to_pfn((void *)pos);
		if (remap_pfn_range(vma, start, page, PAGE_SIZE, PAGE_SHARED)) {
			mutex_unlock(&ov->lock);
			return -EAGAIN;
		}
		start += PAGE_SIZE;
		pos += PAGE_SIZE;
		if (size > PAGE_SIZE)
			size -= PAGE_SIZE;
		else
			size = 0;
	}

	mutex_unlock(&ov->lock);
	return 0;
}

static const struct v4l2_file_operations ov511_fops = {
	.owner =	THIS_MODULE,
	.open =		ov51x_v4l1_open,
	.release =	ov51x_v4l1_close,
	.read =		ov51x_v4l1_read,
	.mmap =		ov51x_v4l1_mmap,
	.ioctl =	ov51x_v4l1_ioctl,
};

static struct video_device vdev_template = {
	.name =		"OV511 USB Camera",
	.fops =		&ov511_fops,
	.release =	video_device_release,
	.minor =	-1,
};




static int
ov7xx0_configure(struct usb_ov511 *ov)
{
	int i, success;
	int rc;

	
	static struct ov511_regvals aRegvalsNorm7610[] = {
		{ OV511_I2C_BUS, 0x10, 0xff },
		{ OV511_I2C_BUS, 0x16, 0x06 },
		{ OV511_I2C_BUS, 0x28, 0x24 },
		{ OV511_I2C_BUS, 0x2b, 0xac },
		{ OV511_I2C_BUS, 0x12, 0x00 },
		{ OV511_I2C_BUS, 0x38, 0x81 },
		{ OV511_I2C_BUS, 0x28, 0x24 },	
		{ OV511_I2C_BUS, 0x0f, 0x85 },	
		{ OV511_I2C_BUS, 0x15, 0x01 },
		{ OV511_I2C_BUS, 0x20, 0x1c },
		{ OV511_I2C_BUS, 0x23, 0x2a },
		{ OV511_I2C_BUS, 0x24, 0x10 },
		{ OV511_I2C_BUS, 0x25, 0x8a },
		{ OV511_I2C_BUS, 0x26, 0xa2 },
		{ OV511_I2C_BUS, 0x27, 0xc2 },
		{ OV511_I2C_BUS, 0x2a, 0x04 },
		{ OV511_I2C_BUS, 0x2c, 0xfe },
		{ OV511_I2C_BUS, 0x2d, 0x93 },
		{ OV511_I2C_BUS, 0x30, 0x71 },
		{ OV511_I2C_BUS, 0x31, 0x60 },
		{ OV511_I2C_BUS, 0x32, 0x26 },
		{ OV511_I2C_BUS, 0x33, 0x20 },
		{ OV511_I2C_BUS, 0x34, 0x48 },
		{ OV511_I2C_BUS, 0x12, 0x24 },
		{ OV511_I2C_BUS, 0x11, 0x01 },
		{ OV511_I2C_BUS, 0x0c, 0x24 },
		{ OV511_I2C_BUS, 0x0d, 0x24 },
		{ OV511_DONE_BUS, 0x0, 0x00 },
	};

	static struct ov511_regvals aRegvalsNorm7620[] = {
		{ OV511_I2C_BUS, 0x00, 0x00 },
		{ OV511_I2C_BUS, 0x01, 0x80 },
		{ OV511_I2C_BUS, 0x02, 0x80 },
		{ OV511_I2C_BUS, 0x03, 0xc0 },
		{ OV511_I2C_BUS, 0x06, 0x60 },
		{ OV511_I2C_BUS, 0x07, 0x00 },
		{ OV511_I2C_BUS, 0x0c, 0x24 },
		{ OV511_I2C_BUS, 0x0c, 0x24 },
		{ OV511_I2C_BUS, 0x0d, 0x24 },
		{ OV511_I2C_BUS, 0x11, 0x01 },
		{ OV511_I2C_BUS, 0x12, 0x24 },
		{ OV511_I2C_BUS, 0x13, 0x01 },
		{ OV511_I2C_BUS, 0x14, 0x84 },
		{ OV511_I2C_BUS, 0x15, 0x01 },
		{ OV511_I2C_BUS, 0x16, 0x03 },
		{ OV511_I2C_BUS, 0x17, 0x2f },
		{ OV511_I2C_BUS, 0x18, 0xcf },
		{ OV511_I2C_BUS, 0x19, 0x06 },
		{ OV511_I2C_BUS, 0x1a, 0xf5 },
		{ OV511_I2C_BUS, 0x1b, 0x00 },
		{ OV511_I2C_BUS, 0x20, 0x18 },
		{ OV511_I2C_BUS, 0x21, 0x80 },
		{ OV511_I2C_BUS, 0x22, 0x80 },
		{ OV511_I2C_BUS, 0x23, 0x00 },
		{ OV511_I2C_BUS, 0x26, 0xa2 },
		{ OV511_I2C_BUS, 0x27, 0xea },
		{ OV511_I2C_BUS, 0x28, 0x20 },
		{ OV511_I2C_BUS, 0x29, 0x00 },
		{ OV511_I2C_BUS, 0x2a, 0x10 },
		{ OV511_I2C_BUS, 0x2b, 0x00 },
		{ OV511_I2C_BUS, 0x2c, 0x88 },
		{ OV511_I2C_BUS, 0x2d, 0x91 },
		{ OV511_I2C_BUS, 0x2e, 0x80 },
		{ OV511_I2C_BUS, 0x2f, 0x44 },
		{ OV511_I2C_BUS, 0x60, 0x27 },
		{ OV511_I2C_BUS, 0x61, 0x02 },
		{ OV511_I2C_BUS, 0x62, 0x5f },
		{ OV511_I2C_BUS, 0x63, 0xd5 },
		{ OV511_I2C_BUS, 0x64, 0x57 },
		{ OV511_I2C_BUS, 0x65, 0x83 },
		{ OV511_I2C_BUS, 0x66, 0x55 },
		{ OV511_I2C_BUS, 0x67, 0x92 },
		{ OV511_I2C_BUS, 0x68, 0xcf },
		{ OV511_I2C_BUS, 0x69, 0x76 },
		{ OV511_I2C_BUS, 0x6a, 0x22 },
		{ OV511_I2C_BUS, 0x6b, 0x00 },
		{ OV511_I2C_BUS, 0x6c, 0x02 },
		{ OV511_I2C_BUS, 0x6d, 0x44 },
		{ OV511_I2C_BUS, 0x6e, 0x80 },
		{ OV511_I2C_BUS, 0x6f, 0x1d },
		{ OV511_I2C_BUS, 0x70, 0x8b },
		{ OV511_I2C_BUS, 0x71, 0x00 },
		{ OV511_I2C_BUS, 0x72, 0x14 },
		{ OV511_I2C_BUS, 0x73, 0x54 },
		{ OV511_I2C_BUS, 0x74, 0x00 },
		{ OV511_I2C_BUS, 0x75, 0x8e },
		{ OV511_I2C_BUS, 0x76, 0x00 },
		{ OV511_I2C_BUS, 0x77, 0xff },
		{ OV511_I2C_BUS, 0x78, 0x80 },
		{ OV511_I2C_BUS, 0x79, 0x80 },
		{ OV511_I2C_BUS, 0x7a, 0x80 },
		{ OV511_I2C_BUS, 0x7b, 0xe2 },
		{ OV511_I2C_BUS, 0x7c, 0x00 },
		{ OV511_DONE_BUS, 0x0, 0x00 },
	};

	PDEBUG(4, "starting configuration");

	
	ov->primary_i2c_slave = OV7xx0_SID;
	if (ov51x_set_slave_ids(ov, OV7xx0_SID) < 0)
		return -1;

	if (init_ov_sensor(ov) >= 0) {
		PDEBUG(1, "OV7xx0 sensor initalized (method 1)");
	} else {
		
		if (i2c_w(ov, 0x12, 0x80) < 0)
			return -1;

		
		msleep(150);

		i = 0;
		success = 0;
		while (i <= i2c_detect_tries) {
			if ((i2c_r(ov, OV7610_REG_ID_HIGH) == 0x7F) &&
			    (i2c_r(ov, OV7610_REG_ID_LOW) == 0xA2)) {
				success = 1;
				break;
			} else {
				i++;
			}
		}



		if ((i >= i2c_detect_tries) && (success == 0)) {
			err("Failed to read sensor ID. You might not have an");
			err("OV7610/20, or it may be not responding. Report");
			err("this to " EMAIL);
			err("This is only a warning. You can attempt to use");
			err("your camera anyway");


		} else {
			PDEBUG(1, "OV7xx0 initialized (method 2, %dx)", i+1);
		}
	}

	
	rc = i2c_r(ov, OV7610_REG_COM_I);

	if (rc < 0) {
		err("Error detecting sensor type");
		return -1;
	} else if ((rc & 3) == 3) {
		dev_info(&ov->dev->dev, "Sensor is an OV7610\n");
		ov->sensor = SEN_OV7610;
	} else if ((rc & 3) == 1) {
		
		if (i2c_r(ov, 0x15) & 1)
			dev_info(&ov->dev->dev, "Sensor is an OV7620AE\n");
		else
			dev_info(&ov->dev->dev, "Sensor is an OV76BE\n");

		
		if (ov->bridge == BRG_OV511PLUS) {
			dev_info(&ov->dev->dev,
				 "Enabling 511+/7620AE workaround\n");
			ov->sensor = SEN_OV7620;
		} else {
			ov->sensor = SEN_OV76BE;
		}
	} else if ((rc & 3) == 0) {
		dev_info(&ov->dev->dev, "Sensor is an OV7620\n");
		ov->sensor = SEN_OV7620;
	} else {
		err("Unknown image sensor version: %d", rc & 3);
		return -1;
	}

	if (ov->sensor == SEN_OV7620) {
		PDEBUG(4, "Writing 7620 registers");
		if (write_regvals(ov, aRegvalsNorm7620))
			return -1;
	} else {
		PDEBUG(4, "Writing 7610 registers");
		if (write_regvals(ov, aRegvalsNorm7610))
			return -1;
	}

	
	ov->maxwidth = 640;
	ov->maxheight = 480;
	ov->minwidth = 64;
	ov->minheight = 48;

	
	ov->brightness = 0x80 << 8;
	ov->contrast = 0x80 << 8;
	ov->colour = 0x80 << 8;
	ov->hue = 0x80 << 8;

	return 0;
}


static int
ov6xx0_configure(struct usb_ov511 *ov)
{
	int rc;

	static struct ov511_regvals aRegvalsNorm6x20[] = {
		{ OV511_I2C_BUS, 0x12, 0x80 }, 
		{ OV511_I2C_BUS, 0x11, 0x01 },
		{ OV511_I2C_BUS, 0x03, 0x60 },
		{ OV511_I2C_BUS, 0x05, 0x7f }, 
		{ OV511_I2C_BUS, 0x07, 0xa8 },
		
		{ OV511_I2C_BUS, 0x0c, 0x24 },
		{ OV511_I2C_BUS, 0x0d, 0x24 },
		{ OV511_I2C_BUS, 0x0f, 0x15 }, 
		{ OV511_I2C_BUS, 0x10, 0x75 }, 
		{ OV511_I2C_BUS, 0x12, 0x24 }, 
		{ OV511_I2C_BUS, 0x14, 0x04 },
		
		{ OV511_I2C_BUS, 0x16, 0x06 },

		{ OV511_I2C_BUS, 0x26, 0xb2 }, 
		
		{ OV511_I2C_BUS, 0x28, 0x05 },
		{ OV511_I2C_BUS, 0x2a, 0x04 }, 

		{ OV511_I2C_BUS, 0x2d, 0x99 },
		{ OV511_I2C_BUS, 0x33, 0xa0 }, 
		{ OV511_I2C_BUS, 0x34, 0xd2 }, 
		{ OV511_I2C_BUS, 0x38, 0x8b },
		{ OV511_I2C_BUS, 0x39, 0x40 },

		{ OV511_I2C_BUS, 0x3c, 0x39 }, 
		{ OV511_I2C_BUS, 0x3c, 0x3c }, 
		{ OV511_I2C_BUS, 0x3c, 0x24 }, 

		{ OV511_I2C_BUS, 0x3d, 0x80 },
		
		{ OV511_I2C_BUS, 0x4a, 0x80 },
		{ OV511_I2C_BUS, 0x4b, 0x80 },
		{ OV511_I2C_BUS, 0x4d, 0xd2 }, 
		{ OV511_I2C_BUS, 0x4e, 0xc1 },
		{ OV511_I2C_BUS, 0x4f, 0x04 },


		{ OV511_DONE_BUS, 0x0, 0x00 },	
	};

	static struct ov511_regvals aRegvalsNorm6x30[] = {
		{ OV511_I2C_BUS, 0x12, 0x80 }, 
		{ OV511_I2C_BUS, 0x11, 0x00 },
		{ OV511_I2C_BUS, 0x03, 0x60 },
		{ OV511_I2C_BUS, 0x05, 0x7f }, 
		{ OV511_I2C_BUS, 0x07, 0xa8 },
		
		{ OV511_I2C_BUS, 0x0c, 0x24 },
		{ OV511_I2C_BUS, 0x0d, 0x24 },
		{ OV511_I2C_BUS, 0x0e, 0x20 },

		{ OV511_I2C_BUS, 0x16, 0x03 },

		
		{ OV511_I2C_BUS, 0x23, 0xc0 },
		{ OV511_I2C_BUS, 0x25, 0x9a }, 


		



		{ OV511_I2C_BUS, 0x2a, 0x04 }, 

		{ OV511_I2C_BUS, 0x2d, 0x99 },







		{ OV511_I2C_BUS, 0x3d, 0x80 },


		


		{ OV511_I2C_BUS, 0x4d, 0x10 }, 
		{ OV511_I2C_BUS, 0x4e, 0x40 },

		
		{ OV511_I2C_BUS, 0x4f, 0x07 },

		{ OV511_I2C_BUS, 0x54, 0x23 }, 
		{ OV511_I2C_BUS, 0x57, 0x81 }, 
		{ OV511_I2C_BUS, 0x59, 0x01 }, 
		{ OV511_I2C_BUS, 0x5a, 0x2c }, 
		{ OV511_I2C_BUS, 0x5b, 0x0f }, 

		{ OV511_DONE_BUS, 0x0, 0x00 },	
	};

	PDEBUG(4, "starting sensor configuration");

	if (init_ov_sensor(ov) < 0) {
		err("Failed to read sensor ID. You might not have an OV6xx0,");
		err("or it may be not responding. Report this to " EMAIL);
		return -1;
	} else {
		PDEBUG(1, "OV6xx0 sensor detected");
	}

	
	rc = i2c_r(ov, OV7610_REG_COM_I);

	if (rc < 0) {
		err("Error detecting sensor type");
		return -1;
	}

	if ((rc & 3) == 0) {
		ov->sensor = SEN_OV6630;
		dev_info(&ov->dev->dev, "Sensor is an OV6630\n");
	} else if ((rc & 3) == 1) {
		ov->sensor = SEN_OV6620;
		dev_info(&ov->dev->dev, "Sensor is an OV6620\n");
	} else if ((rc & 3) == 2) {
		ov->sensor = SEN_OV6630;
		dev_info(&ov->dev->dev, "Sensor is an OV6630AE\n");
	} else if ((rc & 3) == 3) {
		ov->sensor = SEN_OV6630;
		dev_info(&ov->dev->dev, "Sensor is an OV6630AF\n");
	}

	
	ov->maxwidth = 352;
	ov->maxheight = 288;
	ov->minwidth = 64;
	ov->minheight = 48;

	
	ov->brightness = 0x80 << 8;
	ov->contrast = 0x80 << 8;
	ov->colour = 0x80 << 8;
	ov->hue = 0x80 << 8;

	if (ov->sensor == SEN_OV6620) {
		PDEBUG(4, "Writing 6x20 registers");
		if (write_regvals(ov, aRegvalsNorm6x20))
			return -1;
	} else {
		PDEBUG(4, "Writing 6x30 registers");
		if (write_regvals(ov, aRegvalsNorm6x30))
			return -1;
	}

	return 0;
}


static int
ks0127_configure(struct usb_ov511 *ov)
{
	int rc;


#if 0
	if (ov51x_init_ks_sensor(ov) < 0) {
		err("Failed to initialize the KS0127");
		return -1;
	} else {
		PDEBUG(1, "KS012x(B) sensor detected");
	}
#endif

	
	rc = i2c_r(ov, 0x00);
	if (rc < 0) {
		err("Error detecting sensor type");
		return -1;
	} else if (rc & 0x08) {
		rc = i2c_r(ov, 0x3d);
		if (rc < 0) {
			err("Error detecting sensor type");
			return -1;
		} else if ((rc & 0x0f) == 0) {
			dev_info(&ov->dev->dev, "Sensor is a KS0127\n");
			ov->sensor = SEN_KS0127;
		} else if ((rc & 0x0f) == 9) {
			dev_info(&ov->dev->dev, "Sensor is a KS0127B Rev. A\n");
			ov->sensor = SEN_KS0127B;
		}
	} else {
		err("Error: Sensor is an unsupported KS0122");
		return -1;
	}

	
	ov->maxwidth = 640;
	ov->maxheight = 480;
	ov->minwidth = 64;
	ov->minheight = 48;

	
	ov->brightness = 0x80 << 8;
	ov->contrast = 0x80 << 8;
	ov->colour = 0x80 << 8;
	ov->hue = 0x80 << 8;

	
	err("This sensor is not supported yet.");
	return -1;

	return 0;
}


static int
saa7111a_configure(struct usb_ov511 *ov)
{
	int rc;

	
	static struct ov511_regvals aRegvalsNormSAA7111A[] = {
		{ OV511_I2C_BUS, 0x06, 0xce },
		{ OV511_I2C_BUS, 0x07, 0x00 },
		{ OV511_I2C_BUS, 0x10, 0x44 }, 
		{ OV511_I2C_BUS, 0x0e, 0x01 }, 
		{ OV511_I2C_BUS, 0x00, 0x00 },
		{ OV511_I2C_BUS, 0x01, 0x00 },
		{ OV511_I2C_BUS, 0x03, 0x23 },
		{ OV511_I2C_BUS, 0x04, 0x00 },
		{ OV511_I2C_BUS, 0x05, 0x00 },
		{ OV511_I2C_BUS, 0x08, 0xc8 }, 
		{ OV511_I2C_BUS, 0x09, 0x01 }, 
		{ OV511_I2C_BUS, 0x0a, 0x80 }, 
		{ OV511_I2C_BUS, 0x0b, 0x40 }, 
		{ OV511_I2C_BUS, 0x0c, 0x40 }, 
		{ OV511_I2C_BUS, 0x0d, 0x00 }, 
		{ OV511_I2C_BUS, 0x0f, 0x00 },
		{ OV511_I2C_BUS, 0x11, 0x0c },
		{ OV511_I2C_BUS, 0x12, 0x00 },
		{ OV511_I2C_BUS, 0x13, 0x00 },
		{ OV511_I2C_BUS, 0x14, 0x00 },
		{ OV511_I2C_BUS, 0x15, 0x00 },
		{ OV511_I2C_BUS, 0x16, 0x00 },
		{ OV511_I2C_BUS, 0x17, 0x00 },
		{ OV511_I2C_BUS, 0x02, 0xc0 },	
		{ OV511_DONE_BUS, 0x0, 0x00 },
	};


#if 0
	if (ov51x_init_saa_sensor(ov) < 0) {
		err("Failed to initialize the SAA7111A");
		return -1;
	} else {
		PDEBUG(1, "SAA7111A sensor detected");
	}
#endif

	
	if (ov->pal) {
		ov->maxwidth = 320;
		ov->maxheight = 240;		
	} else {
		ov->maxwidth = 640;
		ov->maxheight = 480;		
	}

	ov->minwidth = 320;
	ov->minheight = 240;		

	ov->has_decoder = 1;
	ov->num_inputs = 8;
	ov->norm = VIDEO_MODE_AUTO;
	ov->stop_during_set = 0;	

	
	ov->brightness = 0x80 << 8;
	ov->contrast = 0x40 << 9;
	ov->colour = 0x40 << 9;
	ov->hue = 32768;

	PDEBUG(4, "Writing SAA7111A registers");
	if (write_regvals(ov, aRegvalsNormSAA7111A))
		return -1;

	
	rc = i2c_r(ov, 0x00);

	if (rc < 0) {
		err("Error detecting sensor version");
		return -1;
	} else {
		dev_info(&ov->dev->dev,
			 "Sensor is an SAA7111A (version 0x%x)\n", rc);
		ov->sensor = SEN_SAA7111A;
	}

	
	
	if (ov->bclass == BCL_OV511)
		reg_w(ov, 0x11, 0x00);
	else
		dev_warn(&ov->dev->dev,
			 "SAA7111A not yet supported with OV518/OV518+\n");

	return 0;
}


static int
ov511_configure(struct usb_ov511 *ov)
{
	static struct ov511_regvals aRegvalsInit511[] = {
		{ OV511_REG_BUS, R51x_SYS_RESET,	0x7f },
		{ OV511_REG_BUS, R51x_SYS_INIT,		0x01 },
		{ OV511_REG_BUS, R51x_SYS_RESET,	0x7f },
		{ OV511_REG_BUS, R51x_SYS_INIT,		0x01 },
		{ OV511_REG_BUS, R51x_SYS_RESET,	0x3f },
		{ OV511_REG_BUS, R51x_SYS_INIT,		0x01 },
		{ OV511_REG_BUS, R51x_SYS_RESET,	0x3d },
		{ OV511_DONE_BUS, 0x0, 0x00},
	};

	static struct ov511_regvals aRegvalsNorm511[] = {
		{ OV511_REG_BUS, R511_DRAM_FLOW_CTL, 	0x01 },
		{ OV511_REG_BUS, R51x_SYS_SNAP,		0x00 },
		{ OV511_REG_BUS, R51x_SYS_SNAP,		0x02 },
		{ OV511_REG_BUS, R51x_SYS_SNAP,		0x00 },
		{ OV511_REG_BUS, R511_FIFO_OPTS,	0x1f },
		{ OV511_REG_BUS, R511_COMP_EN,		0x00 },
		{ OV511_REG_BUS, R511_COMP_LUT_EN,	0x03 },
		{ OV511_DONE_BUS, 0x0, 0x00 },
	};

	static struct ov511_regvals aRegvalsNorm511Plus[] = {
		{ OV511_REG_BUS, R511_DRAM_FLOW_CTL,	0xff },
		{ OV511_REG_BUS, R51x_SYS_SNAP,		0x00 },
		{ OV511_REG_BUS, R51x_SYS_SNAP,		0x02 },
		{ OV511_REG_BUS, R51x_SYS_SNAP,		0x00 },
		{ OV511_REG_BUS, R511_FIFO_OPTS,	0xff },
		{ OV511_REG_BUS, R511_COMP_EN,		0x00 },
		{ OV511_REG_BUS, R511_COMP_LUT_EN,	0x03 },
		{ OV511_DONE_BUS, 0x0, 0x00 },
	};

	PDEBUG(4, "");

	ov->customid = reg_r(ov, R511_SYS_CUST_ID);
	if (ov->customid < 0) {
		err("Unable to read camera bridge registers");
		goto error;
	}

	PDEBUG (1, "CustomID = %d", ov->customid);
	ov->desc = symbolic(camlist, ov->customid);
	dev_info(&ov->dev->dev, "model: %s\n", ov->desc);

	if (0 == strcmp(ov->desc, NOT_DEFINED_STR)) {
		err("Camera type (%d) not recognized", ov->customid);
		err("Please notify " EMAIL " of the name,");
		err("manufacturer, model, and this number of your camera.");
		err("Also include the output of the detection process.");
	}

	if (ov->customid == 70)		
		ov->pal = 1;

	if (write_regvals(ov, aRegvalsInit511))
		goto error;

	if (ov->led_policy == LED_OFF || ov->led_policy == LED_AUTO)
		ov51x_led_control(ov, 0);

	
	if (ov->bridge == BRG_OV511) {
		if (write_regvals(ov, aRegvalsNorm511))
			goto error;
	} else if (ov->bridge == BRG_OV511PLUS) {
		if (write_regvals(ov, aRegvalsNorm511Plus))
			goto error;
	} else {
		err("Invalid bridge");
	}

	if (ov511_init_compression(ov))
		goto error;

	ov->packet_numbering = 1;
	ov511_set_packet_size(ov, 0);

	ov->snap_enabled = snapshot;

	
	PDEBUG(3, "Testing for 0V7xx0");
	ov->primary_i2c_slave = OV7xx0_SID;
	if (ov51x_set_slave_ids(ov, OV7xx0_SID) < 0)
		goto error;

	if (i2c_w(ov, 0x12, 0x80) < 0) {
		
		PDEBUG(3, "Testing for 0V6xx0");
		ov->primary_i2c_slave = OV6xx0_SID;
		if (ov51x_set_slave_ids(ov, OV6xx0_SID) < 0)
			goto error;

		if (i2c_w(ov, 0x12, 0x80) < 0) {
			
			PDEBUG(3, "Testing for 0V8xx0");
			ov->primary_i2c_slave = OV8xx0_SID;
			if (ov51x_set_slave_ids(ov, OV8xx0_SID) < 0)
				goto error;

			if (i2c_w(ov, 0x12, 0x80) < 0) {
				
				PDEBUG(3, "Testing for SAA7111A");
				ov->primary_i2c_slave = SAA7111A_SID;
				if (ov51x_set_slave_ids(ov, SAA7111A_SID) < 0)
					goto error;

				if (i2c_w(ov, 0x0d, 0x00) < 0) {
					
					PDEBUG(3, "Testing for KS0127");
					ov->primary_i2c_slave = KS0127_SID;
					if (ov51x_set_slave_ids(ov, KS0127_SID) < 0)
						goto error;

					if (i2c_w(ov, 0x10, 0x00) < 0) {
						err("Can't determine sensor slave IDs");
						goto error;
					} else {
						if (ks0127_configure(ov) < 0) {
							err("Failed to configure KS0127");
							goto error;
						}
					}
				} else {
					if (saa7111a_configure(ov) < 0) {
						err("Failed to configure SAA7111A");
						goto error;
					}
				}
			} else {
				err("Detected unsupported OV8xx0 sensor");
				goto error;
			}
		} else {
			if (ov6xx0_configure(ov) < 0) {
				err("Failed to configure OV6xx0");
				goto error;
			}
		}
	} else {
		if (ov7xx0_configure(ov) < 0) {
			err("Failed to configure OV7xx0");
			goto error;
		}
	}

	return 0;

error:
	err("OV511 Config failed");

	return -EBUSY;
}


static int
ov518_configure(struct usb_ov511 *ov)
{
	
	static struct ov511_regvals aRegvalsInit518[] = {
		{ OV511_REG_BUS, R51x_SYS_RESET,	0x40 },
		{ OV511_REG_BUS, R51x_SYS_INIT,		0xe1 },
		{ OV511_REG_BUS, R51x_SYS_RESET,	0x3e },
		{ OV511_REG_BUS, R51x_SYS_INIT,		0xe1 },
		{ OV511_REG_BUS, R51x_SYS_RESET,	0x00 },
		{ OV511_REG_BUS, R51x_SYS_INIT,		0xe1 },
		{ OV511_REG_BUS, 0x46,			0x00 },
		{ OV511_REG_BUS, 0x5d,			0x03 },
		{ OV511_DONE_BUS, 0x0, 0x00},
	};

	static struct ov511_regvals aRegvalsNorm518[] = {
		{ OV511_REG_BUS, R51x_SYS_SNAP,		0x02 }, 
		{ OV511_REG_BUS, R51x_SYS_SNAP,		0x01 }, 
		{ OV511_REG_BUS, 0x31, 			0x0f },
		{ OV511_REG_BUS, 0x5d,			0x03 },
		{ OV511_REG_BUS, 0x24,			0x9f },
		{ OV511_REG_BUS, 0x25,			0x90 },
		{ OV511_REG_BUS, 0x20,			0x00 },
		{ OV511_REG_BUS, 0x51,			0x04 },
		{ OV511_REG_BUS, 0x71,			0x19 },
		{ OV511_DONE_BUS, 0x0, 0x00 },
	};

	static struct ov511_regvals aRegvalsNorm518Plus[] = {
		{ OV511_REG_BUS, R51x_SYS_SNAP,		0x02 }, 
		{ OV511_REG_BUS, R51x_SYS_SNAP,		0x01 }, 
		{ OV511_REG_BUS, 0x31, 			0x0f },
		{ OV511_REG_BUS, 0x5d,			0x03 },
		{ OV511_REG_BUS, 0x24,			0x9f },
		{ OV511_REG_BUS, 0x25,			0x90 },
		{ OV511_REG_BUS, 0x20,			0x60 },
		{ OV511_REG_BUS, 0x51,			0x02 },
		{ OV511_REG_BUS, 0x71,			0x19 },
		{ OV511_REG_BUS, 0x40,			0xff },
		{ OV511_REG_BUS, 0x41,			0x42 },
		{ OV511_REG_BUS, 0x46,			0x00 },
		{ OV511_REG_BUS, 0x33,			0x04 },
		{ OV511_REG_BUS, 0x21,			0x19 },
		{ OV511_REG_BUS, 0x3f,			0x10 },
		{ OV511_DONE_BUS, 0x0, 0x00 },
	};

	PDEBUG(4, "");

	
	dev_info(&ov->dev->dev, "Device revision %d\n",
		 0x1F & reg_r(ov, R511_SYS_CUST_ID));

	
	ov->desc = symbolic(camlist, 0);

	if (write_regvals(ov, aRegvalsInit518))
		goto error;

	
	if (reg_w_mask(ov, 0x57, 0x00, 0x02) < 0)
		goto error;

	
	if (ov->led_policy == LED_OFF || ov->led_policy == LED_AUTO)
		ov51x_led_control(ov, 0);
	else
		ov51x_led_control(ov, 1);

	
	if (!dumppix && !ov->compress) {
		ov->compress = 1;
		dev_warn(&ov->dev->dev,
			 "Compression required with OV518...enabling\n");
	}

	if (ov->bridge == BRG_OV518) {
		if (write_regvals(ov, aRegvalsNorm518))
			goto error;
	} else if (ov->bridge == BRG_OV518PLUS) {
		if (write_regvals(ov, aRegvalsNorm518Plus))
			goto error;
	} else {
		err("Invalid bridge");
	}

	if (reg_w(ov, 0x2f, 0x80) < 0)
		goto error;

	if (ov518_init_compression(ov))
		goto error;

	if (ov->bridge == BRG_OV518)
	{
		struct usb_interface *ifp;
		struct usb_host_interface *alt;
		__u16 mxps = 0;

		ifp = usb_ifnum_to_if(ov->dev, 0);
		if (ifp) {
			alt = usb_altnum_to_altsetting(ifp, 7);
			if (alt)
				mxps = le16_to_cpu(alt->endpoint[0].desc.wMaxPacketSize);
		}

		
		if (mxps == 897)
			ov->packet_numbering = 1;
		else
			ov->packet_numbering = 0;
	} else {
		
		ov->packet_numbering = 1;
	}

	ov518_set_packet_size(ov, 0);

	ov->snap_enabled = snapshot;

	
	ov->primary_i2c_slave = OV7xx0_SID;
	if (ov51x_set_slave_ids(ov, OV7xx0_SID) < 0)
		goto error;

	

	if (init_ov_sensor(ov) < 0) {
		
		ov->primary_i2c_slave = OV6xx0_SID;
		if (ov51x_set_slave_ids(ov, OV6xx0_SID) < 0)
			goto error;

		if (init_ov_sensor(ov) < 0) {
			
			ov->primary_i2c_slave = OV8xx0_SID;
			if (ov51x_set_slave_ids(ov, OV8xx0_SID) < 0)
				goto error;

			if (init_ov_sensor(ov) < 0) {
				err("Can't determine sensor slave IDs");
				goto error;
			} else {
				err("Detected unsupported OV8xx0 sensor");
				goto error;
			}
		} else {
			if (ov6xx0_configure(ov) < 0) {
				err("Failed to configure OV6xx0");
				goto error;
			}
		}
	} else {
		if (ov7xx0_configure(ov) < 0) {
			err("Failed to configure OV7xx0");
			goto error;
		}
	}

	ov->maxwidth = 352;
	ov->maxheight = 288;

	
	ov->minwidth = 160;
	ov->minheight = 120;

	return 0;

error:
	err("OV518 Config failed");

	return -EBUSY;
}



static inline struct usb_ov511 *cd_to_ov(struct device *cd)
{
	struct video_device *vdev = to_video_device(cd);
	return video_get_drvdata(vdev);
}

static ssize_t show_custom_id(struct device *cd,
			      struct device_attribute *attr, char *buf)
{
	struct usb_ov511 *ov = cd_to_ov(cd);
	return sprintf(buf, "%d\n", ov->customid);
}
static DEVICE_ATTR(custom_id, S_IRUGO, show_custom_id, NULL);

static ssize_t show_model(struct device *cd,
			  struct device_attribute *attr, char *buf)
{
	struct usb_ov511 *ov = cd_to_ov(cd);
	return sprintf(buf, "%s\n", ov->desc);
}
static DEVICE_ATTR(model, S_IRUGO, show_model, NULL);

static ssize_t show_bridge(struct device *cd,
			   struct device_attribute *attr, char *buf)
{
	struct usb_ov511 *ov = cd_to_ov(cd);
	return sprintf(buf, "%s\n", symbolic(brglist, ov->bridge));
}
static DEVICE_ATTR(bridge, S_IRUGO, show_bridge, NULL);

static ssize_t show_sensor(struct device *cd,
			   struct device_attribute *attr, char *buf)
{
	struct usb_ov511 *ov = cd_to_ov(cd);
	return sprintf(buf, "%s\n", symbolic(senlist, ov->sensor));
}
static DEVICE_ATTR(sensor, S_IRUGO, show_sensor, NULL);

static ssize_t show_brightness(struct device *cd,
			       struct device_attribute *attr, char *buf)
{
	struct usb_ov511 *ov = cd_to_ov(cd);
	unsigned short x;

	if (!ov->dev)
		return -ENODEV;
	sensor_get_brightness(ov, &x);
	return sprintf(buf, "%d\n", x >> 8);
}
static DEVICE_ATTR(brightness, S_IRUGO, show_brightness, NULL);

static ssize_t show_saturation(struct device *cd,
			       struct device_attribute *attr, char *buf)
{
	struct usb_ov511 *ov = cd_to_ov(cd);
	unsigned short x;

	if (!ov->dev)
		return -ENODEV;
	sensor_get_saturation(ov, &x);
	return sprintf(buf, "%d\n", x >> 8);
}
static DEVICE_ATTR(saturation, S_IRUGO, show_saturation, NULL);

static ssize_t show_contrast(struct device *cd,
			     struct device_attribute *attr, char *buf)
{
	struct usb_ov511 *ov = cd_to_ov(cd);
	unsigned short x;

	if (!ov->dev)
		return -ENODEV;
	sensor_get_contrast(ov, &x);
	return sprintf(buf, "%d\n", x >> 8);
}
static DEVICE_ATTR(contrast, S_IRUGO, show_contrast, NULL);

static ssize_t show_hue(struct device *cd,
			struct device_attribute *attr, char *buf)
{
	struct usb_ov511 *ov = cd_to_ov(cd);
	unsigned short x;

	if (!ov->dev)
		return -ENODEV;
	sensor_get_hue(ov, &x);
	return sprintf(buf, "%d\n", x >> 8);
}
static DEVICE_ATTR(hue, S_IRUGO, show_hue, NULL);

static ssize_t show_exposure(struct device *cd,
			     struct device_attribute *attr, char *buf)
{
	struct usb_ov511 *ov = cd_to_ov(cd);
	unsigned char exp = 0;

	if (!ov->dev)
		return -ENODEV;
	sensor_get_exposure(ov, &exp);
	return sprintf(buf, "%d\n", exp);
}
static DEVICE_ATTR(exposure, S_IRUGO, show_exposure, NULL);

static int ov_create_sysfs(struct video_device *vdev)
{
	int rc;

	rc = device_create_file(&vdev->dev, &dev_attr_custom_id);
	if (rc) goto err;
	rc = device_create_file(&vdev->dev, &dev_attr_model);
	if (rc) goto err_id;
	rc = device_create_file(&vdev->dev, &dev_attr_bridge);
	if (rc) goto err_model;
	rc = device_create_file(&vdev->dev, &dev_attr_sensor);
	if (rc) goto err_bridge;
	rc = device_create_file(&vdev->dev, &dev_attr_brightness);
	if (rc) goto err_sensor;
	rc = device_create_file(&vdev->dev, &dev_attr_saturation);
	if (rc) goto err_bright;
	rc = device_create_file(&vdev->dev, &dev_attr_contrast);
	if (rc) goto err_sat;
	rc = device_create_file(&vdev->dev, &dev_attr_hue);
	if (rc) goto err_contrast;
	rc = device_create_file(&vdev->dev, &dev_attr_exposure);
	if (rc) goto err_hue;

	return 0;

err_hue:
	device_remove_file(&vdev->dev, &dev_attr_hue);
err_contrast:
	device_remove_file(&vdev->dev, &dev_attr_contrast);
err_sat:
	device_remove_file(&vdev->dev, &dev_attr_saturation);
err_bright:
	device_remove_file(&vdev->dev, &dev_attr_brightness);
err_sensor:
	device_remove_file(&vdev->dev, &dev_attr_sensor);
err_bridge:
	device_remove_file(&vdev->dev, &dev_attr_bridge);
err_model:
	device_remove_file(&vdev->dev, &dev_attr_model);
err_id:
	device_remove_file(&vdev->dev, &dev_attr_custom_id);
err:
	return rc;
}



static int
ov51x_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_interface_descriptor *idesc;
	struct usb_ov511 *ov;
	int i, rc, nr;

	PDEBUG(1, "probing for device...");

	
	if (dev->descriptor.bNumConfigurations != 1)
		return -ENODEV;

	idesc = &intf->cur_altsetting->desc;

	if (idesc->bInterfaceClass != 0xFF)
		return -ENODEV;
	if (idesc->bInterfaceSubClass != 0x00)
		return -ENODEV;

	if ((ov = kzalloc(sizeof(*ov), GFP_KERNEL)) == NULL) {
		err("couldn't kmalloc ov struct");
		goto error_out;
	}

	ov->dev = dev;
	ov->iface = idesc->bInterfaceNumber;
	ov->led_policy = led;
	ov->compress = compress;
	ov->lightfreq = lightfreq;
	ov->num_inputs = 1;	   
	ov->stop_during_set = !fastset;
	ov->backlight = backlight;
	ov->mirror = mirror;
	ov->auto_brt = autobright;
	ov->auto_gain = autogain;
	ov->auto_exp = autoexp;

	switch (le16_to_cpu(dev->descriptor.idProduct)) {
	case PROD_OV511:
		ov->bridge = BRG_OV511;
		ov->bclass = BCL_OV511;
		break;
	case PROD_OV511PLUS:
		ov->bridge = BRG_OV511PLUS;
		ov->bclass = BCL_OV511;
		break;
	case PROD_OV518:
		ov->bridge = BRG_OV518;
		ov->bclass = BCL_OV518;
		break;
	case PROD_OV518PLUS:
		ov->bridge = BRG_OV518PLUS;
		ov->bclass = BCL_OV518;
		break;
	case PROD_ME2CAM:
		if (le16_to_cpu(dev->descriptor.idVendor) != VEND_MATTEL)
			goto error;
		ov->bridge = BRG_OV511PLUS;
		ov->bclass = BCL_OV511;
		break;
	default:
		err("Unknown product ID 0x%04x", le16_to_cpu(dev->descriptor.idProduct));
		goto error;
	}

	dev_info(&intf->dev, "USB %s video device found\n",
		 symbolic(brglist, ov->bridge));

	init_waitqueue_head(&ov->wq);

	mutex_init(&ov->lock);	
	mutex_init(&ov->buf_lock);
	mutex_init(&ov->i2c_lock);
	mutex_init(&ov->cbuf_lock);

	ov->buf_state = BUF_NOT_ALLOCATED;

	if (usb_make_path(dev, ov->usb_path, OV511_USB_PATH_LEN) < 0) {
		err("usb_make_path error");
		goto error;
	}

	
	
	ov->cbuf = kmalloc(OV511_CBUF_SIZE, GFP_KERNEL);
	if (!ov->cbuf)
		goto error;

	if (ov->bclass == BCL_OV518) {
		if (ov518_configure(ov) < 0)
			goto error;
	} else {
		if (ov511_configure(ov) < 0)
			goto error;
	}

	for (i = 0; i < OV511_NUMFRAMES; i++) {
		ov->frame[i].framenum = i;
		init_waitqueue_head(&ov->frame[i].wq);
	}

	for (i = 0; i < OV511_NUMSBUF; i++) {
		ov->sbuf[i].ov = ov;
		spin_lock_init(&ov->sbuf[i].lock);
		ov->sbuf[i].n = i;
	}

	
	if (ov51x_set_default_params(ov) < 0)
		goto error;

#ifdef OV511_DEBUG
	if (dump_bridge) {
		if (ov->bclass == BCL_OV511)
			ov511_dump_regs(ov);
		else
			ov518_dump_regs(ov);
	}
#endif

	ov->vdev = video_device_alloc();
	if (!ov->vdev)
		goto error;

	memcpy(ov->vdev, &vdev_template, sizeof(*ov->vdev));
	ov->vdev->parent = &intf->dev;
	video_set_drvdata(ov->vdev, ov);

	mutex_lock(&ov->lock);

	
	nr = find_first_zero_bit(&ov511_devused, OV511_MAX_UNIT_VIDEO);

	
	if (unit_video[nr] != 0)
		rc = video_register_device(ov->vdev, VFL_TYPE_GRABBER,
					   unit_video[nr]);
	else
		rc = video_register_device(ov->vdev, VFL_TYPE_GRABBER, -1);

	if (rc < 0) {
		err("video_register_device failed");
		mutex_unlock(&ov->lock);
		goto error;
	}

	
	ov511_devused |= 1 << nr;
	ov->nr = nr;

	dev_info(&intf->dev, "Device at %s registered to minor %d\n",
		 ov->usb_path, ov->vdev->minor);

	usb_set_intfdata(intf, ov);
	if (ov_create_sysfs(ov->vdev)) {
		err("ov_create_sysfs failed");
		ov511_devused &= ~(1 << nr);
		mutex_unlock(&ov->lock);
		goto error;
	}

	mutex_unlock(&ov->lock);

	return 0;

error:
	if (ov->vdev) {
		if (-1 == ov->vdev->minor)
			video_device_release(ov->vdev);
		else
			video_unregister_device(ov->vdev);
		ov->vdev = NULL;
	}

	if (ov->cbuf) {
		mutex_lock(&ov->cbuf_lock);
		kfree(ov->cbuf);
		ov->cbuf = NULL;
		mutex_unlock(&ov->cbuf_lock);
	}

	kfree(ov);
	ov = NULL;

error_out:
	err("Camera initialization failed");
	return -EIO;
}

static void
ov51x_disconnect(struct usb_interface *intf)
{
	struct usb_ov511 *ov = usb_get_intfdata(intf);
	int n;

	PDEBUG(3, "");

	mutex_lock(&ov->lock);
	usb_set_intfdata (intf, NULL);

	if (!ov) {
		mutex_unlock(&ov->lock);
		return;
	}

	
	ov511_devused &= ~(1 << ov->nr);

	if (ov->vdev)
		video_unregister_device(ov->vdev);

	for (n = 0; n < OV511_NUMFRAMES; n++)
		ov->frame[n].grabstate = FRAME_ERROR;

	ov->curframe = -1;

	
	for (n = 0; n < OV511_NUMFRAMES; n++)
		wake_up_interruptible(&ov->frame[n].wq);

	wake_up_interruptible(&ov->wq);

	ov->streaming = 0;
	ov51x_unlink_isoc(ov);
	mutex_unlock(&ov->lock);

	ov->dev = NULL;

	
	if (ov && !ov->user) {
		mutex_lock(&ov->cbuf_lock);
		kfree(ov->cbuf);
		ov->cbuf = NULL;
		mutex_unlock(&ov->cbuf_lock);

		ov51x_dealloc(ov);
		kfree(ov);
		ov = NULL;
	}

	PDEBUG(3, "Disconnect complete");
}

static struct usb_driver ov511_driver = {
	.name =		"ov511",
	.id_table =	device_table,
	.probe =	ov51x_probe,
	.disconnect =	ov51x_disconnect
};



static int __init
usb_ov511_init(void)
{
	int retval;

	retval = usb_register(&ov511_driver);
	if (retval)
		goto out;

	printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_VERSION ":"
	       DRIVER_DESC "\n");

out:
	return retval;
}

static void __exit
usb_ov511_exit(void)
{
	usb_deregister(&ov511_driver);
	printk(KERN_INFO KBUILD_MODNAME ": driver deregistered\n");
}

module_init(usb_ov511_init);
module_exit(usb_ov511_exit);

