
#ifndef usbvideo_h
#define	usbvideo_h

#include <linux/videodev.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>
#include <linux/usb.h>
#include <linux/mutex.h>


#define assert(expr) ((void) ((expr) ? 0 : (err("assert failed at line %d",__LINE__))))

#define USBVIDEO_REPORT_STATS	1	


#define FLAGS_RETRY_VIDIOCSYNC		(1 << 0)
#define	FLAGS_MONOCHROME		(1 << 1)
#define FLAGS_DISPLAY_HINTS		(1 << 2)
#define FLAGS_OVERLAY_STATS		(1 << 3)
#define FLAGS_FORCE_TESTPATTERN		(1 << 4)
#define FLAGS_SEPARATE_FRAMES		(1 << 5)
#define FLAGS_CLEAN_FRAMES		(1 << 6)
#define	FLAGS_NO_DECODING		(1 << 7)


#define USBVIDEO_FRAME_FLAG_SOFTWARE_CONTRAST	(1 << 0)


#define CAMERA_URB_FRAMES       32
#define CAMERA_MAX_ISO_PACKET   1023 
#define FRAMES_PER_DESC		(CAMERA_URB_FRAMES)
#define FRAME_SIZE_PER_DESC	(CAMERA_MAX_ISO_PACKET)


#define RESTRICT_TO_RANGE(v,mi,ma) { if ((v) < (mi)) (v) = (mi); else if ((v) > (ma)) (v) = (ma); }

#define V4L_BYTES_PER_PIXEL     3	


#define	VIDEOSIZE(x,y)	(((x) & 0xFFFFL) | (((y) & 0xFFFFL) << 16))
#define	VIDEOSIZE_X(vs)	((vs) & 0xFFFFL)
#define	VIDEOSIZE_Y(vs)	(((vs) >> 16) & 0xFFFFL)
typedef unsigned long videosize_t;


#define CAMERA_IS_OPERATIONAL(uvd) (\
	(uvd != NULL) && \
	((uvd)->dev != NULL) && \
	((uvd)->last_error == 0) && \
	(!(uvd)->remove_pending))


#define LIMIT_RGB(x) (((x) < 0) ? 0 : (((x) > 255) ? 255 : (x)))
#define YUV_TO_RGB_BY_THE_BOOK(my,mu,mv,mr,mg,mb) { \
    int mm_y, mm_yc, mm_u, mm_v, mm_r, mm_g, mm_b; \
    mm_y = (my) - 16;  \
    mm_u = (mu) - 128; \
    mm_v = (mv) - 128; \
    mm_yc= mm_y * 76284; \
    mm_b = (mm_yc		+ 132252*mm_v	) >> 16; \
    mm_g = (mm_yc -  53281*mm_u -  25625*mm_v	) >> 16; \
    mm_r = (mm_yc + 104595*mm_u			) >> 16; \
    mb = LIMIT_RGB(mm_b); \
    mg = LIMIT_RGB(mm_g); \
    mr = LIMIT_RGB(mm_r); \
}

#define	RING_QUEUE_SIZE		(128*1024)	
#define	RING_QUEUE_ADVANCE_INDEX(rq,ind,n) (rq)->ind = ((rq)->ind + (n)) & ((rq)->length-1)
#define	RING_QUEUE_DEQUEUE_BYTES(rq,n) RING_QUEUE_ADVANCE_INDEX(rq,ri,n)
#define	RING_QUEUE_PEEK(rq,ofs) ((rq)->queue[((ofs) + (rq)->ri) & ((rq)->length-1)])

struct RingQueue {
	unsigned char *queue;	
	int length;		
	int wi;			
	int ri;			
	wait_queue_head_t wqh;	
};

enum ScanState {
	ScanState_Scanning,	
	ScanState_Lines		
};


enum ParseState {
	scan_Continue,		
	scan_NextFrame,		
	scan_Out,		
	scan_EndParse		
};

enum FrameState {
	FrameState_Unused,	
	FrameState_Ready,	
	FrameState_Grabbing,	
	FrameState_Done,	
	FrameState_Done_Hold,	
	FrameState_Error,	
};


enum Deinterlace {
	Deinterlace_None=0,
	Deinterlace_FillOddLines,
	Deinterlace_FillEvenLines
};

#define USBVIDEO_NUMFRAMES	2	
#define USBVIDEO_NUMSBUF	2	


struct usbvideo_sbuf {
	char *data;
	struct urb *urb;
};

struct usbvideo_frame {
	char *data;		
	unsigned long header;	

	videosize_t canvas;	
	videosize_t request;	
	unsigned short palette;	

	enum FrameState frameState;
	enum ScanState scanstate;	
	enum Deinterlace deinterlace;
	int flags;		

	int curline;		

	long seqRead_Length;	
	long seqRead_Index;	

	void *user;		
};


struct usbvideo_statistics {
	unsigned long frame_num;	
	unsigned long urb_count;        
	unsigned long urb_length;       
	unsigned long data_count;       
	unsigned long header_count;     
	unsigned long iso_skip_count;	
	unsigned long iso_err_count;	
};

struct usbvideo;

struct uvd {
	struct video_device vdev;	
	struct usb_device *dev;
	struct usbvideo *handle;	
	void *user_data;		
	int user_size;			
	int debug;			
	unsigned char iface;		
	unsigned char video_endp;
	unsigned char ifaceAltActive;
	unsigned char ifaceAltInactive; 
	unsigned long flags;		
	unsigned long paletteBits;	
	unsigned short defaultPalette;	
	struct mutex lock;
	int user;		

	videosize_t videosize;	
	videosize_t canvas;	
	int max_frame_size;	

	int uvd_used;        	
	int streaming;		
	int grabbing;		
	int settingsAdjusted;	
	int last_error;		

	char *fbuf;		
	int fbuf_size;		

	int curframe;
	int iso_packet_len;	

	struct RingQueue dp;	
	struct usbvideo_frame frame[USBVIDEO_NUMFRAMES];
	struct usbvideo_sbuf sbuf[USBVIDEO_NUMSBUF];

	volatile int remove_pending;	

	struct video_picture vpic, vpic_old;	
	struct video_capability vcap;		
	struct video_channel vchan;	
	struct usbvideo_statistics stats;
	char videoName[32];		
};


struct usbvideo_cb {
	int (*probe)(struct usb_interface *, const struct usb_device_id *);
	void (*userFree)(struct uvd *);
	void (*disconnect)(struct usb_interface *);
	int (*setupOnOpen)(struct uvd *);
	void (*videoStart)(struct uvd *);
	void (*videoStop)(struct uvd *);
	void (*processData)(struct uvd *, struct usbvideo_frame *);
	void (*postProcess)(struct uvd *, struct usbvideo_frame *);
	void (*adjustPicture)(struct uvd *);
	int (*getFPS)(struct uvd *);
	int (*overlayHook)(struct uvd *, struct usbvideo_frame *);
	int (*getFrame)(struct uvd *, int);
	int (*startDataPump)(struct uvd *uvd);
	void (*stopDataPump)(struct uvd *uvd);
	int (*setVideoMode)(struct uvd *uvd, struct video_window *vw);
};

struct usbvideo {
	int num_cameras;		
	struct usb_driver usbdrv;	
	char drvName[80];		
	struct mutex lock;		
	struct usbvideo_cb cb;		
	struct video_device vdt;	
	struct uvd *cam;			
	struct module *md_module;	
};



#define	GET_CALLBACK(uvd,cbName) ((uvd)->handle->cb.cbName)


#define	VALID_CALLBACK(uvd,cbName) ((((uvd) != NULL) && \
		((uvd)->handle != NULL)) ? GET_CALLBACK(uvd,cbName) : NULL)

int  RingQueue_Dequeue(struct RingQueue *rq, unsigned char *dst, int len);
int  RingQueue_Enqueue(struct RingQueue *rq, const unsigned char *cdata, int n);
void RingQueue_WakeUpInterruptible(struct RingQueue *rq);
void RingQueue_Flush(struct RingQueue *rq);

static inline int RingQueue_GetLength(const struct RingQueue *rq)
{
	return (rq->wi - rq->ri + rq->length) & (rq->length-1);
}

static inline int RingQueue_GetFreeSpace(const struct RingQueue *rq)
{
	return rq->length - RingQueue_GetLength(rq);
}

void usbvideo_DrawLine(
	struct usbvideo_frame *frame,
	int x1, int y1,
	int x2, int y2,
	unsigned char cr, unsigned char cg, unsigned char cb);
void usbvideo_HexDump(const unsigned char *data, int len);
void usbvideo_SayAndWait(const char *what);
void usbvideo_TestPattern(struct uvd *uvd, int fullframe, int pmode);


unsigned long usbvideo_kvirt_to_pa(unsigned long adr);

int usbvideo_register(
	struct usbvideo **pCams,
	const int num_cams,
	const int num_extra,
	const char *driverName,
	const struct usbvideo_cb *cbTable,
	struct module *md,
	const struct usb_device_id *id_table);
struct uvd *usbvideo_AllocateDevice(struct usbvideo *cams);
int usbvideo_RegisterVideoDevice(struct uvd *uvd);
void usbvideo_Deregister(struct usbvideo **uvt);

int usbvideo_v4l_initialize(struct video_device *dev);

void usbvideo_DeinterlaceFrame(struct uvd *uvd, struct usbvideo_frame *frame);


static inline void RGB24_PUTPIXEL(
	struct usbvideo_frame *fr,
	int ix, int iy,
	unsigned char vr,
	unsigned char vg,
	unsigned char vb)
{
	register unsigned char *pf;
	int limiter = 0, mx, my;
	mx = ix;
	my = iy;
	if (mx < 0) {
		mx=0;
		limiter++;
	} else if (mx >= VIDEOSIZE_X((fr)->request)) {
		mx= VIDEOSIZE_X((fr)->request) - 1;
		limiter++;
	}
	if (my < 0) {
		my = 0;
		limiter++;
	} else if (my >= VIDEOSIZE_Y((fr)->request)) {
		my = VIDEOSIZE_Y((fr)->request) - 1;
		limiter++;
	}
	pf = (fr)->data + V4L_BYTES_PER_PIXEL*((iy)*VIDEOSIZE_X((fr)->request) + (ix));
	if (limiter) {
		*pf++ = 0;
		*pf++ = 0;
		*pf++ = 0xFF;
	} else {
		*pf++ = (vb);
		*pf++ = (vg);
		*pf++ = (vr);
	}
}

#endif 
