

#ifndef _BUZ_H_
#define _BUZ_H_

#include <media/v4l2-device.h>

struct zoran_requestbuffers {
	unsigned long count;	
	unsigned long size;	
};

struct zoran_sync {
	unsigned long frame;	
	unsigned long length;	
	unsigned long seq;	
	struct timeval timestamp;	
};

struct zoran_status {
	int input;		
	int signal;		
	int norm;		
	int color;		
};

struct zoran_params {

	

	int major_version;	
	int minor_version;	

	

	int input;		
	int norm;		
	int decimation;		

	

	int HorDcm;		
	int VerDcm;		
	int TmpDcm;		
	int field_per_buff;	
	int img_x;		
	int img_y;		
	int img_width;		
	int img_height;		

	

	

	int quality;		

	int odd_even;		

	int APPn;		
	int APP_len;		
	char APP_data[60];	

	int COM_len;		
	char COM_data[60];	

	unsigned long jpeg_markers;	
#define JPEG_MARKER_DHT (1<<3)	
#define JPEG_MARKER_DQT (1<<4)	
#define JPEG_MARKER_DRI (1<<5)	
#define JPEG_MARKER_COM (1<<6)	
#define JPEG_MARKER_APP (1<<7)	

	int VFIFO_FB;		

	

	char reserved[312];	
};


#define BUZIOC_G_PARAMS       _IOR ('v', BASE_VIDIOCPRIVATE+0,  struct zoran_params)
#define BUZIOC_S_PARAMS       _IOWR('v', BASE_VIDIOCPRIVATE+1,  struct zoran_params)
#define BUZIOC_REQBUFS        _IOWR('v', BASE_VIDIOCPRIVATE+2,  struct zoran_requestbuffers)
#define BUZIOC_QBUF_CAPT      _IOW ('v', BASE_VIDIOCPRIVATE+3,  int)
#define BUZIOC_QBUF_PLAY      _IOW ('v', BASE_VIDIOCPRIVATE+4,  int)
#define BUZIOC_SYNC           _IOR ('v', BASE_VIDIOCPRIVATE+5,  struct zoran_sync)
#define BUZIOC_G_STATUS       _IOWR('v', BASE_VIDIOCPRIVATE+6,  struct zoran_status)


#ifdef __KERNEL__

#define MAJOR_VERSION 0		
#define MINOR_VERSION 10	
#define RELEASE_VERSION 0	

#define ZORAN_NAME    "ZORAN"	

#define ZR_DEVNAME(zr) ((zr)->name)

#define   BUZ_MAX_WIDTH   (zr->timing->Wa)
#define   BUZ_MAX_HEIGHT  (zr->timing->Ha)
#define   BUZ_MIN_WIDTH    32	
#define   BUZ_MIN_HEIGHT   24	

#define BUZ_NUM_STAT_COM    4
#define BUZ_MASK_STAT_COM   3

#define BUZ_MAX_FRAME     256	
#define BUZ_MASK_FRAME    255	

#define BUZ_MAX_INPUT       16

#if VIDEO_MAX_FRAME <= 32
#   define   V4L_MAX_FRAME   32
#elif VIDEO_MAX_FRAME <= 64
#   define   V4L_MAX_FRAME   64
#else
#   error   "Too many video frame buffers to handle"
#endif
#define   V4L_MASK_FRAME   (V4L_MAX_FRAME - 1)

#define MAX_FRAME (BUZ_MAX_FRAME > VIDEO_MAX_FRAME ? BUZ_MAX_FRAME : VIDEO_MAX_FRAME)

#include "zr36057.h"

enum card_type {
	UNKNOWN = -1,

	
	DC10_old,		
	DC10_new,		
	DC10plus,
	DC30,
	DC30plus,

	
	LML33,
	LML33R10,

	
	BUZ,

	
	AVS6EYES,

	
	NUM_CARDS
};

enum zoran_codec_mode {
	BUZ_MODE_IDLE,		
	BUZ_MODE_MOTION_COMPRESS,	
	BUZ_MODE_MOTION_DECOMPRESS,	
	BUZ_MODE_STILL_COMPRESS,	
	BUZ_MODE_STILL_DECOMPRESS	
};

enum zoran_buffer_state {
	BUZ_STATE_USER,		
	BUZ_STATE_PEND,		
	BUZ_STATE_DMA,		
	BUZ_STATE_DONE		
};

enum zoran_map_mode {
	ZORAN_MAP_MODE_RAW,
	ZORAN_MAP_MODE_JPG_REC,
#define ZORAN_MAP_MODE_JPG ZORAN_MAP_MODE_JPG_REC
	ZORAN_MAP_MODE_JPG_PLAY,
};

enum gpio_type {
	ZR_GPIO_JPEG_SLEEP = 0,
	ZR_GPIO_JPEG_RESET,
	ZR_GPIO_JPEG_FRAME,
	ZR_GPIO_VID_DIR,
	ZR_GPIO_VID_EN,
	ZR_GPIO_VID_RESET,
	ZR_GPIO_CLK_SEL1,
	ZR_GPIO_CLK_SEL2,
	ZR_GPIO_MAX,
};

enum gpcs_type {
	GPCS_JPEG_RESET = 0,
	GPCS_JPEG_START,
	GPCS_MAX,
};

struct zoran_format {
	char *name;
	__u32 fourcc;
	int colorspace;
	int depth;
	__u32 flags;
	__u32 vfespfr;
};

#define ZORAN_FORMAT_COMPRESSED 1<<0
#define ZORAN_FORMAT_OVERLAY    1<<1
#define ZORAN_FORMAT_CAPTURE	1<<2
#define ZORAN_FORMAT_PLAYBACK	1<<3


struct zoran_overlay_settings {
	int is_set;
	int x, y, width, height;	
	int clipcount;		
	const struct zoran_format *format;	
};


struct zoran_v4l_settings {
	int width, height, bytesperline;	
	const struct zoran_format *format;	
};


struct zoran_jpg_settings {
	int decimation;		
	int HorDcm, VerDcm, TmpDcm;	
	int field_per_buff, odd_even;	
	int img_x, img_y, img_width, img_height;	
	struct v4l2_jpegcompression jpg_comp;	
};

struct zoran_mapping {
	struct file *file;
	int count;
};

struct zoran_buffer {
	struct zoran_mapping *map;
	enum zoran_buffer_state state;	
	struct zoran_sync bs;		
	union {
		struct {
			__le32 *frag_tab;	
			u32 frag_tab_bus;	
		} jpg;
		struct {
			char *fbuffer;		
			unsigned long fbuffer_phys;
			unsigned long fbuffer_bus;
		} v4l;
	};
};

enum zoran_lock_activity {
	ZORAN_FREE,		
	ZORAN_ACTIVE,		
	ZORAN_LOCKED,		
};


struct zoran_buffer_col {
	enum zoran_lock_activity active;	
	unsigned int num_buffers, buffer_size;
	struct zoran_buffer buffer[MAX_FRAME];	
	u8 allocated;		
	u8 need_contiguous;	
	
};

struct zoran;


struct zoran_fh {
	struct zoran *zr;

	enum zoran_map_mode map_mode;		

	struct zoran_overlay_settings overlay_settings;
	u32 *overlay_mask;			
	enum zoran_lock_activity overlay_active;

	struct zoran_buffer_col buffers;	

	struct zoran_v4l_settings v4l_settings;	
	struct zoran_jpg_settings jpg_settings;	
};

struct card_info {
	enum card_type type;
	char name[32];
	const char *i2c_decoder;	
	const char *mod_decoder;	
	const unsigned short *addrs_decoder;
	const char *i2c_encoder;	
	const char *mod_encoder;	
	const unsigned short *addrs_encoder;
	u16 video_vfe, video_codec;			
	u16 audio_chip;					

	int inputs;		
	struct input {
		int muxsel;
		char name[32];
	} input[BUZ_MAX_INPUT];

	v4l2_std_id norms;
	struct tvnorm *tvn[3];	

	u32 jpeg_int;		
	u32 vsync_int;		
	s8 gpio[ZR_GPIO_MAX];
	u8 gpcs[GPCS_MAX];

	struct vfe_polarity vfe_pol;
	u8 gpio_pol[ZR_GPIO_MAX];

	
	u8 gws_not_connected;

	
	u8 input_mux;

	void (*init) (struct zoran * zr);
};

struct zoran {
	struct v4l2_device v4l2_dev;
	struct video_device *video_dev;

	struct i2c_adapter i2c_adapter;	
	struct i2c_algo_bit_data i2c_algo;	
	u32 i2cbr;

	struct v4l2_subdev *decoder;	
	struct v4l2_subdev *encoder;	

	struct videocodec *codec;	
	struct videocodec *vfe;	

	struct mutex resource_lock;	

	u8 initialized;		
	int user;		
	struct card_info card;
	struct tvnorm *timing;

	unsigned short id;	
	char name[32];		
	struct pci_dev *pci_dev;	
	unsigned char revision;	
	unsigned char __iomem *zr36057_mem;

	spinlock_t spinlock;	

	
	int input;	
	v4l2_std_id norm;

	
	void    *vbuf_base;
	int     vbuf_height, vbuf_width;
	int     vbuf_depth;
	int     vbuf_bytesperline;

	struct zoran_overlay_settings overlay_settings;
	u32 *overlay_mask;	
	enum zoran_lock_activity overlay_active;	

	wait_queue_head_t v4l_capq;

	int v4l_overlay_active;	
	int v4l_memgrab_active;	

	int v4l_grab_frame;	
#define NO_GRAB_ACTIVE (-1)
	unsigned long v4l_grab_seq;	
	struct zoran_v4l_settings v4l_settings;	

	
	unsigned long v4l_pend_head;
	unsigned long v4l_pend_tail;
	unsigned long v4l_sync_tail;
	int v4l_pend[V4L_MAX_FRAME];
	struct zoran_buffer_col v4l_buffers;	

	
	enum zoran_codec_mode codec_mode;	
	struct zoran_jpg_settings jpg_settings;	

	wait_queue_head_t jpg_capq;	

	
	
	
	unsigned long jpg_que_head;	
	unsigned long jpg_dma_head;	
	unsigned long jpg_dma_tail;	
	unsigned long jpg_que_tail;	
	unsigned long jpg_seq_num;	
	unsigned long jpg_err_seq;	
	unsigned long jpg_err_shift;
	unsigned long jpg_queued_num;	

	
	__le32 *stat_com;		

	
	int jpg_pend[BUZ_MAX_FRAME];

	
	struct zoran_buffer_col jpg_buffers;	

	
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *zoran_proc;
#else
	void *zoran_proc;
#endif
	int testing;
	int jpeg_error;
	int intr_counter_GIRQ1;
	int intr_counter_GIRQ0;
	int intr_counter_CodRepIRQ;
	int intr_counter_JPEGRepIRQ;
	int field_counter;
	int IRQ1_in;
	int IRQ1_out;
	int JPEG_in;
	int JPEG_out;
	int JPEG_0;
	int JPEG_1;
	int END_event_missed;
	int JPEG_missed;
	int JPEG_error;
	int num_errors;
	int JPEG_max_missed;
	int JPEG_min_missed;

	u32 last_isr;
	unsigned long frame_num;

	wait_queue_head_t test_q;
};

static inline struct zoran *to_zoran(struct v4l2_device *v4l2_dev)
{
	return container_of(v4l2_dev, struct zoran, v4l2_dev);
}


#define btwrite(dat,adr)    writel((dat), zr->zr36057_mem+(adr))
#define btread(adr)         readl(zr->zr36057_mem+(adr))

#define btand(dat,adr)      btwrite((dat) & btread(adr), adr)
#define btor(dat,adr)       btwrite((dat) | btread(adr), adr)
#define btaor(dat,mask,adr) btwrite((dat) | ((mask) & btread(adr)), adr)

#endif				

#endif
