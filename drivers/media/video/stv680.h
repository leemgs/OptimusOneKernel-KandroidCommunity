


#define STV680_PACKETSIZE	4096


#define STV680_NUMSBUF		1


#define STV680_NUMFRAMES	2


#define STV680_NUMSCRATCH	2


#define STV680_MAX_NULLPACKETS	200


#define STV680_MAX_ERRORS	100

#define USB_PENCAM_VENDOR_ID	0x0553
#define USB_PENCAM_PRODUCT_ID	0x0202

#define USB_CREATIVEGOMINI_VENDOR_ID	0x041e
#define USB_CREATIVEGOMINI_PRODUCT_ID	0x4007

#define PENCAM_TIMEOUT          1000

#define STV_VIDEO_PALETTE       VIDEO_PALETTE_RGB24

static struct usb_device_id device_table[] = {
	{USB_DEVICE (USB_PENCAM_VENDOR_ID, USB_PENCAM_PRODUCT_ID)},
	{USB_DEVICE (USB_CREATIVEGOMINI_VENDOR_ID, USB_CREATIVEGOMINI_PRODUCT_ID)},
	{}
};
MODULE_DEVICE_TABLE (usb, device_table);

struct stv680_sbuf {
	unsigned char *data;
};

enum {
	FRAME_UNUSED,		
	FRAME_READY,		
	FRAME_GRABBING,		
	FRAME_DONE,		
	FRAME_ERROR,		
};

enum {
	BUFFER_UNUSED,
	BUFFER_READY,
	BUFFER_BUSY,
	BUFFER_DONE,
};


struct stv680_scratch {
	unsigned char *data;
	volatile int state;
	int offset;
	int length;
};


struct stv680_frame {
	unsigned char *data;	
	volatile int grabstate;	
	unsigned char *curline;
	int curlinepix;
	int curpix;
};


struct usb_stv {
	struct video_device *vdev;

	struct usb_device *udev;

	unsigned char bulk_in_endpointAddr;	
	char *camera_name;

	unsigned int VideoMode;	
	int SupportedModes;
	int CIF;
	int VGA;
	int QVGA;
	int cwidth;		
	int cheight;		
	int maxwidth;		
	int maxheight;		
	int vwidth;		
	int vheight;		
	unsigned long int rawbufsize;
	unsigned long int maxframesize;	

	int origGain;
	int origMode;		

	struct mutex lock;	
	int user;		
	int removed;		
	int streaming;		
	char *fbuf;		
	struct urb *urb[STV680_NUMSBUF];	
	int curframe;		
	struct stv680_frame frame[STV680_NUMFRAMES];	
	int readcount;
	int framecount;
	int error;
	int dropped;
	int scratch_next;
	int scratch_use;
	int scratch_overflow;
	struct stv680_scratch scratch[STV680_NUMSCRATCH];	
	struct stv680_sbuf sbuf[STV680_NUMSBUF];

	unsigned int brightness;
	unsigned int chgbright;
	unsigned int whiteness;
	unsigned int colour;
	unsigned int contrast;
	unsigned int hue;
	unsigned int palette;
	unsigned int depth;	

	wait_queue_head_t wq;	

	int nullpackets;
};


static const unsigned char red[256] = {
	0, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 25, 30, 35, 38, 42,
	44, 47, 50, 53, 54, 57, 59, 61, 63, 65, 67, 69,
	71, 71, 73, 75, 77, 78, 80, 81, 82, 84, 85, 87,
	88, 89, 90, 91, 93, 94, 95, 97, 98, 98, 99, 101,
	102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
	114, 115, 116, 116, 117, 118, 119, 120, 121, 122, 123, 124,
	125, 125, 126, 127, 128, 129, 129, 130, 131, 132, 133, 134,
	134, 135, 135, 136, 137, 138, 139, 140, 140, 141, 142, 143,
	143, 143, 144, 145, 146, 147, 147, 148, 149, 150, 150, 151,
	152, 152, 152, 153, 154, 154, 155, 156, 157, 157, 158, 159,
	159, 160, 161, 161, 161, 162, 163, 163, 164, 165, 165, 166,
	167, 167, 168, 168, 169, 170, 170, 170, 171, 171, 172, 173,
	173, 174, 174, 175, 176, 176, 177, 178, 178, 179, 179, 179,
	180, 180, 181, 181, 182, 183, 183, 184, 184, 185, 185, 186,
	187, 187, 188, 188, 188, 188, 189, 190, 190, 191, 191, 192,
	192, 193, 193, 194, 195, 195, 196, 196, 197, 197, 197, 197,
	198, 198, 199, 199, 200, 201, 201, 202, 202, 203, 203, 204,
	204, 205, 205, 206, 206, 206, 206, 207, 207, 208, 208, 209,
	209, 210, 210, 211, 211, 212, 212, 213, 213, 214, 214, 215,
	215, 215, 215, 216, 216, 217, 217, 218, 218, 218, 219, 219,
	220, 220, 221, 221
};

static const unsigned char green[256] = {
	0, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
	21, 21, 21, 21, 21, 21, 21, 28, 34, 39, 43, 47,
	50, 53, 56, 59, 61, 64, 66, 68, 71, 73, 75, 77,
	79, 80, 82, 84, 86, 87, 89, 91, 92, 94, 95, 97,
	98, 100, 101, 102, 104, 105, 106, 108, 109, 110, 111, 113,
	114, 115, 116, 117, 118, 120, 121, 122, 123, 124, 125, 126,
	127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
	139, 140, 141, 142, 143, 144, 144, 145, 146, 147, 148, 149,
	150, 151, 151, 152, 153, 154, 155, 156, 156, 157, 158, 159,
	160, 160, 161, 162, 163, 164, 164, 165, 166, 167, 167, 168,
	169, 170, 170, 171, 172, 172, 173, 174, 175, 175, 176, 177,
	177, 178, 179, 179, 180, 181, 182, 182, 183, 184, 184, 185,
	186, 186, 187, 187, 188, 189, 189, 190, 191, 191, 192, 193,
	193, 194, 194, 195, 196, 196, 197, 198, 198, 199, 199, 200,
	201, 201, 202, 202, 203, 204, 204, 205, 205, 206, 206, 207,
	208, 208, 209, 209, 210, 210, 211, 212, 212, 213, 213, 214,
	214, 215, 215, 216, 217, 217, 218, 218, 219, 219, 220, 220,
	221, 221, 222, 222, 223, 224, 224, 225, 225, 226, 226, 227,
	227, 228, 228, 229, 229, 230, 230, 231, 231, 232, 232, 233,
	233, 234, 234, 235, 235, 236, 236, 237, 237, 238, 238, 239,
	239, 240, 240, 241, 241, 242, 242, 243, 243, 243, 244, 244,
	245, 245, 246, 246
};

static const unsigned char blue[256] = {
	0, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
	23, 23, 23, 23, 23, 23, 23, 30, 37, 42, 47, 51,
	55, 58, 61, 64, 67, 70, 72, 74, 78, 80, 82, 84,
	86, 88, 90, 92, 94, 95, 97, 100, 101, 103, 104, 106,
	107, 110, 111, 112, 114, 115, 116, 118, 119, 121, 122, 124,
	125, 126, 127, 128, 129, 132, 133, 134, 135, 136, 137, 138,
	139, 140, 141, 143, 144, 145, 146, 147, 148, 149, 150, 151,
	152, 154, 155, 156, 157, 158, 158, 159, 160, 161, 162, 163,
	165, 166, 166, 167, 168, 169, 170, 171, 171, 172, 173, 174,
	176, 176, 177, 178, 179, 180, 180, 181, 182, 183, 183, 184,
	185, 187, 187, 188, 189, 189, 190, 191, 192, 192, 193, 194,
	194, 195, 196, 196, 198, 199, 200, 200, 201, 202, 202, 203,
	204, 204, 205, 205, 206, 207, 207, 209, 210, 210, 211, 212,
	212, 213, 213, 214, 215, 215, 216, 217, 217, 218, 218, 220,
	221, 221, 222, 222, 223, 224, 224, 225, 225, 226, 226, 227,
	228, 228, 229, 229, 231, 231, 232, 233, 233, 234, 234, 235,
	235, 236, 236, 237, 238, 238, 239, 239, 240, 240, 242, 242,
	243, 243, 244, 244, 245, 246, 246, 247, 247, 248, 248, 249,
	249, 250, 250, 251, 251, 253, 253, 254, 254, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255
};
