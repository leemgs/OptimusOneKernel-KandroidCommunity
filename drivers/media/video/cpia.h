#ifndef cpia_h
#define cpia_h



#define CPIA_MAJ_VER	1
#define CPIA_MIN_VER   2
#define CPIA_PATCH_VER	3

#define CPIA_PP_MAJ_VER       CPIA_MAJ_VER
#define CPIA_PP_MIN_VER       CPIA_MIN_VER
#define CPIA_PP_PATCH_VER     CPIA_PATCH_VER

#define CPIA_USB_MAJ_VER      CPIA_MAJ_VER
#define CPIA_USB_MIN_VER      CPIA_MIN_VER
#define CPIA_USB_PATCH_VER    CPIA_PATCH_VER

#define CPIA_MAX_FRAME_SIZE_UNALIGNED	(352 * 288 * 4)   
#define CPIA_MAX_FRAME_SIZE	((CPIA_MAX_FRAME_SIZE_UNALIGNED + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)) 

#ifdef __KERNEL__

#include <asm/uaccess.h>
#include <linux/videodev.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>
#include <linux/list.h>
#include <linux/mutex.h>

struct cpia_camera_ops
{
	
	int (*open)(void *privdata);

	
	int (*registerCallback)(void *privdata, void (*cb)(void *cbdata),
				void *cbdata);

	
	int (*transferCmd)(void *privdata, u8 *command, u8 *data);

	
	int (*streamStart)(void *privdata);

	
	int (*streamStop)(void *privdata);

	
	int (*streamRead)(void *privdata, u8 *buffer, int noblock);

	
	int (*close)(void *privdata);

	
	int wait_for_stream_ready;

	
	struct module *owner;
};

struct cpia_frame {
	u8 *data;
	int count;
	int width;
	int height;
	volatile int state;
};

struct cam_params {
	struct {
		u8 firmwareVersion;
		u8 firmwareRevision;
		u8 vcVersion;
		u8 vcRevision;
	} version;
	struct {
		u16 vendor;
		u16 product;
		u16 deviceRevision;
	} pnpID;
	struct {
		u8 vpVersion;
		u8 vpRevision;
		u16 cameraHeadID;
	} vpVersion;
	struct {
		u8 systemState;
		u8 grabState;
		u8 streamState;
		u8 fatalError;
		u8 cmdError;
		u8 debugFlags;
		u8 vpStatus;
		u8 errorCode;
	} status;
	struct {
		u8 brightness;
		u8 contrast;
		u8 saturation;
	} colourParams;
	struct {
		u8 gainMode;
		u8 expMode;
		u8 compMode;
		u8 centreWeight;
		u8 gain;
		u8 fineExp;
		u8 coarseExpLo;
		u8 coarseExpHi;
		u8 redComp;
		u8 green1Comp;
		u8 green2Comp;
		u8 blueComp;
	} exposure;
	struct {
		u8 balanceMode;
		u8 redGain;
		u8 greenGain;
		u8 blueGain;
	} colourBalance;
	struct {
		u8 divisor;
		u8 baserate;
	} sensorFps;
	struct {
		u8 gain1;
		u8 gain2;
		u8 gain4;
		u8 gain8;
	} apcor;
	struct {
		u8 disabled;
		u8 flickerMode;
		u8 coarseJump;
		int allowableOverExposure;
	} flickerControl;
	struct {
		u8 gain1;
		u8 gain2;
		u8 gain4;
		u8 gain8;
	} vlOffset;
	struct {
		u8 mode;
		u8 decimation;
	} compression;
	struct {
		u8 frTargeting;
		u8 targetFR;
		u8 targetQ;
	} compressionTarget;
	struct {
		u8 yThreshold;
		u8 uvThreshold;
	} yuvThreshold;
	struct {
		u8 hysteresis;
		u8 threshMax;
		u8 smallStep;
		u8 largeStep;
		u8 decimationHysteresis;
		u8 frDiffStepThresh;
		u8 qDiffStepThresh;
		u8 decimationThreshMod;
	} compressionParams;
	struct {
		u8 videoSize;		
		u8 subSample;
		u8 yuvOrder;
	} format;
	struct {                        
		u8 qx3_detected;        
		u8 toplight;            
		u8 bottomlight;         
		u8 button;              
		u8 cradled;             
	} qx3;
	struct {
		u8 colStart;		
		u8 colEnd;		
		u8 rowStart;		
		u8 rowEnd;		
	} roi;
	u8 ecpTiming;
	u8 streamStartLine;
};

enum v4l_camstates {
	CPIA_V4L_IDLE = 0,
	CPIA_V4L_ERROR,
	CPIA_V4L_COMMAND,
	CPIA_V4L_GRABBING,
	CPIA_V4L_STREAMING,
	CPIA_V4L_STREAMING_PAUSED,
};

#define FRAME_NUM	2	

struct cam_data {
	struct list_head cam_data_list;

	struct mutex busy_lock;		
	struct cpia_camera_ops *ops;	
	void *lowlevel_data;		
	u8 *raw_image;			
	struct cpia_frame decompressed_frame;
					
	int image_size;			
	int open_count;			

				
	int fps;			
	int transfer_rate;		
	u8 mainsFreq;			

				
	struct mutex param_lock;	
	struct cam_params params;	
	struct proc_dir_entry *proc_entry;	

					
	int video_size;			
	volatile enum v4l_camstates camstate;	
	struct video_device vdev;	
	struct video_picture vp;	
	struct video_window vw;		
	struct video_capture vc;       	

				
	int curframe;			
	u8 *frame_buf;			
	struct cpia_frame frame[FRAME_NUM];
				

	int first_frame;
	int mmap_kludge;		
	volatile u32 cmd_queue;		
	int exposure_status;		
	int exposure_count;		
};


struct cam_data *cpia_register_camera(struct cpia_camera_ops *ops, void *lowlevel);


void cpia_unregister_camera(struct cam_data *cam);


#define CPIA_MAX_IMAGE_SIZE ((352*288*2)+64+(288*3)+5)


#define MAGIC_0		0x19
#define MAGIC_1		0x68
#define DATA_IN		0xC0
#define DATA_OUT	0x40
#define VIDEOSIZE_QCIF	0	
#define VIDEOSIZE_CIF	1	
#define VIDEOSIZE_SIF	2	
#define VIDEOSIZE_QSIF	3	
#define VIDEOSIZE_48_48		4 
#define VIDEOSIZE_64_48		5
#define VIDEOSIZE_128_96	6
#define VIDEOSIZE_160_120	VIDEOSIZE_QSIF
#define VIDEOSIZE_176_144	VIDEOSIZE_QCIF
#define VIDEOSIZE_192_144	7
#define VIDEOSIZE_224_168	8
#define VIDEOSIZE_256_192	9
#define VIDEOSIZE_288_216	10
#define VIDEOSIZE_320_240	VIDEOSIZE_SIF
#define VIDEOSIZE_352_288	VIDEOSIZE_CIF
#define VIDEOSIZE_88_72		11 
#define SUBSAMPLE_420	0
#define SUBSAMPLE_422	1
#define YUVORDER_YUYV	0
#define YUVORDER_UYVY	1
#define NOT_COMPRESSED	0
#define COMPRESSED	1
#define NO_DECIMATION	0
#define DECIMATION_ENAB	1
#define EOI		0xff	
#define EOL		0xfd	
#define FRAME_HEADER_SIZE	64


#define CPIA_GRAB_SINGLE	0
#define CPIA_GRAB_CONTINUOUS	1


#define CPIA_COMPRESSION_NONE	0
#define CPIA_COMPRESSION_AUTO	1
#define CPIA_COMPRESSION_MANUAL	2
#define CPIA_COMPRESSION_TARGET_QUALITY         0
#define CPIA_COMPRESSION_TARGET_FRAMERATE       1


#define SYSTEMSTATE	0
#define GRABSTATE	1
#define STREAMSTATE	2
#define FATALERROR	3
#define CMDERROR	4
#define DEBUGFLAGS	5
#define VPSTATUS	6
#define ERRORCODE	7


#define UNINITIALISED_STATE	0
#define PASS_THROUGH_STATE	1
#define LO_POWER_STATE		2
#define HI_POWER_STATE		3
#define WARM_BOOT_STATE		4


#define GRAB_IDLE		0
#define GRAB_ACTIVE		1
#define GRAB_DONE		2


#define STREAM_NOT_READY	0
#define STREAM_READY		1
#define STREAM_OPEN		2
#define STREAM_PAUSED		3
#define STREAM_FINISHED		4


#define CPIA_FLAG	  1
#define SYSTEM_FLAG	  2
#define INT_CTRL_FLAG	  4
#define PROCESS_FLAG	  8
#define COM_FLAG	 16
#define VP_CTRL_FLAG	 32
#define CAPTURE_FLAG	 64
#define DEBUG_FLAG	128


#define VP_STATE_OK			0x00

#define VP_STATE_FAILED_VIDEOINIT	0x01
#define VP_STATE_FAILED_AECACBINIT	0x02
#define VP_STATE_AEC_MAX		0x04
#define VP_STATE_ACB_BMAX		0x08

#define VP_STATE_ACB_RMIN		0x10
#define VP_STATE_ACB_GMIN		0x20
#define VP_STATE_ACB_RMAX		0x40
#define VP_STATE_ACB_GMAX		0x80


#define COMP_RED        220
#define COMP_GREEN1     214
#define COMP_GREEN2     COMP_GREEN1
#define COMP_BLUE       230


#define EXPOSURE_VERY_LIGHT 0
#define EXPOSURE_LIGHT      1
#define EXPOSURE_NORMAL     2
#define EXPOSURE_DARK       3
#define EXPOSURE_VERY_DARK  4


#define ERROR_FLICKER_BELOW_MIN_EXP     0x01 
#define ALOG(fmt,args...) printk(fmt, ##args)
#define LOG(fmt,args...) ALOG(KERN_INFO __FILE__ ":%s(%d):" fmt, __func__ , __LINE__ , ##args)

#ifdef _CPIA_DEBUG_
#define ADBG(fmt,args...) printk(fmt, jiffies, ##args)
#define DBG(fmt,args...) ADBG(KERN_DEBUG __FILE__" (%ld):%s(%d):" fmt, __func__, __LINE__ , ##args)
#else
#define DBG(fmn,args...) do {} while(0)
#endif

#define DEB_BYTE(p)\
  DBG("%1d %1d %1d %1d %1d %1d %1d %1d \n",\
      (p)&0x80?1:0, (p)&0x40?1:0, (p)&0x20?1:0, (p)&0x10?1:0,\
	(p)&0x08?1:0, (p)&0x04?1:0, (p)&0x02?1:0, (p)&0x01?1:0);

#endif 

#endif 
