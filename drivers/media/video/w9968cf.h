

#ifndef _W9968CF_H_
#define _W9968CF_H_

#include <linux/videodev2.h>
#include <linux/usb.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/param.h>
#include <linux/types.h>
#include <linux/rwsem.h>
#include <linux/mutex.h>

#include <media/v4l2-device.h>
#include <media/ovcamchip.h>

#include "w9968cf_vpp.h"




#define W9968CF_VPPMOD_LOAD     1  


#define W9968CF_DEBUG


#define W9968CF_DEBUG_LEVEL    2 
#define W9968CF_SPECIFIC_DEBUG 0 

#define W9968CF_MAX_DEVICES    32
#define W9968CF_SIMCAMS        W9968CF_MAX_DEVICES 

#define W9968CF_MAX_BUFFERS   32
#define W9968CF_BUFFERS       2 


static const u16 wMaxPacketSize[] = {1023, 959, 895, 831, 767, 703, 639, 575,
				      511, 447, 383, 319, 255, 191, 127,  63};
#define W9968CF_PACKET_SIZE      1023 
#define W9968CF_MIN_PACKET_SIZE  63 
#define W9968CF_ISO_PACKETS      5 
#define W9968CF_USB_CTRL_TIMEOUT 1000 
#define W9968CF_URBS             2 

#define W9968CF_I2C_BUS_DELAY    4 
#define W9968CF_I2C_RW_RETRIES   15 


struct w9968cf_format {
	const u16 palette;
	const u16 depth;
	const u8 compression;
};

static const struct w9968cf_format w9968cf_formatlist[] = {
	{ VIDEO_PALETTE_UYVY,    16, 0 }, 
	{ VIDEO_PALETTE_YUV422P, 16, 1 }, 
	{ VIDEO_PALETTE_YUV420P, 12, 1 }, 
	{ VIDEO_PALETTE_YUV420,  12, 1 }, 
	{ VIDEO_PALETTE_YUYV,    16, 0 }, 
	{ VIDEO_PALETTE_YUV422,  16, 0 }, 
	{ VIDEO_PALETTE_GREY,     8, 0 }, 
	{ VIDEO_PALETTE_RGB555,  16, 0 }, 
	{ VIDEO_PALETTE_RGB565,  16, 0 }, 
	{ VIDEO_PALETTE_RGB24,   24, 0 }, 
	{ VIDEO_PALETTE_RGB32,   32, 0 }, 
	{ 0,                      0, 0 }  
};

#define W9968CF_DECOMPRESSION    2 
#define W9968CF_PALETTE_DECOMP_OFF   VIDEO_PALETTE_UYVY    
#define W9968CF_PALETTE_DECOMP_FORCE VIDEO_PALETTE_YUV420P 
#define W9968CF_PALETTE_DECOMP_ON    VIDEO_PALETTE_UYVY    

#define W9968CF_FORCE_RGB        0  

#define W9968CF_MAX_WIDTH      800 
#define W9968CF_MAX_HEIGHT     600 
#define W9968CF_WIDTH          320 
#define W9968CF_HEIGHT         240 

#define W9968CF_CLAMPING       0 
#define W9968CF_FILTER_TYPE    0 
#define W9968CF_DOUBLE_BUFFER  1 
#define W9968CF_LARGEVIEW      1 
#define W9968CF_UPSCALING      0 

#define W9968CF_MONOCHROME     0 
#define W9968CF_BRIGHTNESS     31000 
#define W9968CF_HUE            32768 
#define W9968CF_COLOUR         32768 
#define W9968CF_CONTRAST       50000 
#define W9968CF_WHITENESS      32768 

#define W9968CF_AUTOBRIGHT     0 
#define W9968CF_AUTOEXP        1 
#define W9968CF_LIGHTFREQ      50 
#define W9968CF_BANDINGFILTER  0 
#define W9968CF_BACKLIGHT      0 
#define W9968CF_MIRROR         0 

#define W9968CF_CLOCKDIV         -1 
#define W9968CF_DEF_CLOCKDIVISOR  0 




#define W9968CF_MODULE_NAME     "V4L driver for W996[87]CF JPEG USB " \
				"Dual Mode Camera Chip"
#define W9968CF_MODULE_VERSION  "1:1.34-basic"
#define W9968CF_MODULE_AUTHOR   "(C) 2002-2004 Luca Risolia"
#define W9968CF_AUTHOR_EMAIL    "<luca.risolia@studio.unibo.it>"
#define W9968CF_MODULE_LICENSE  "GPL"

static const struct usb_device_id winbond_id_table[] = {
	{
		
		USB_DEVICE(0x041e, 0x4003),
		.driver_info = (unsigned long)"w9968cf",
	},
	{
		
		USB_DEVICE(0x1046, 0x9967),
		.driver_info = (unsigned long)"w9968cf",
	},
	{ } 
};


enum w9968cf_model_id {
	W9968CF_MOD_GENERIC = 1, 
	W9968CF_MOD_CLVBWGP = 11,
	W9968CF_MOD_ADPVDMA = 21, 
	W9986CF_MOD_AAU = 31,     
	W9968CF_MOD_CLVBWG = 34,  
	W9968CF_MOD_LL = 37,      
	W9968CF_MOD_EEEMC = 40,   
	W9968CF_MOD_OOE = 42,     
	W9968CF_MOD_ODPVDMPC = 43,
	W9968CF_MOD_PDPII = 46,   
	W9968CF_MOD_PDP480 = 49,  
};

enum w9968cf_frame_status {
	F_READY,            
	F_GRABBING,         
	F_ERROR,            
	F_UNUSED            
};

struct w9968cf_frame_t {
	void* buffer;
	unsigned long size;
	u32 length;
	int number;
	enum w9968cf_frame_status status;
	struct w9968cf_frame_t* next;
	u8 queued;
};

enum w9968cf_vpp_flag {
	VPP_NONE = 0x00,
	VPP_UPSCALE = 0x01,
	VPP_SWAP_YUV_BYTES = 0x02,
	VPP_DECOMPRESSION = 0x04,
	VPP_UYVY_TO_RGBX = 0x08,
};


struct w9968cf_device {
	enum w9968cf_model_id id;   

	struct v4l2_device v4l2_dev;
	struct video_device* v4ldev; 
	struct list_head v4llist;    

	struct usb_device* usbdev;           
	struct urb* urb[W9968CF_URBS];       
	void* transfer_buffer[W9968CF_URBS]; 
	u16* control_buffer;                 
	u16* data_buffer;                    

	struct w9968cf_frame_t frame[W9968CF_MAX_BUFFERS];
	struct w9968cf_frame_t frame_tmp; 
	struct w9968cf_frame_t frame_vpp; 
	struct w9968cf_frame_t* frame_current; 
	struct w9968cf_frame_t* requested_frame[W9968CF_MAX_BUFFERS];

	u8 max_buffers,   
	   force_palette, 
	   force_rgb,     
	   double_buffer, 
	   clamping,      
	   filter_type,   
	   capture,       
	   largeview,     
	   decompression, 
	   upscaling;     

	struct video_picture picture; 
	struct video_window window;   

	u16 hw_depth,    
	    hw_palette,  
	    hw_width,    
	    hw_height,   
	    hs_polarity, 
	    vs_polarity, 
	    start_cropx, 
	    start_cropy; 

	enum w9968cf_vpp_flag vpp_flag; 

	u8 nbuffers,      
	   altsetting,    
	   disconnected,  
	   misconfigured, 
	   users,         
	   streaming;     

	u8 sensor_initialized; 

	
	int sensor,       
	    monochrome;   
	u16 maxwidth,     
	    maxheight,    
	    minwidth,     
	    minheight;    
	u8  auto_brt,     
	    auto_exp,     
	    backlight,    
	    mirror,       
	    lightfreq,    
	    bandfilt;     
	s8  clockdiv;     

	
	struct i2c_adapter i2c_adapter;
	struct v4l2_subdev *sensor_sd;

	
	struct mutex dev_mutex,    
			 fileop_mutex; 
	spinlock_t urb_lock,   
		   flist_lock; 
	wait_queue_head_t open, wait_queue;

	char command[16]; 
};

static inline struct w9968cf_device *to_cam(struct v4l2_device *v4l2_dev)
{
	return container_of(v4l2_dev, struct w9968cf_device, v4l2_dev);
}




#undef DBG
#undef KDBG
#ifdef W9968CF_DEBUG

#	define DBG(level, fmt, args...)                                       \
{                                                                             \
	if ( ((specific_debug) && (debug == (level))) ||                      \
	     ((!specific_debug) && (debug >= (level))) ) {                    \
		if ((level) == 1)                                             \
			v4l2_err(&cam->v4l2_dev, fmt "\n", ## args);          \
		else if ((level) == 2 || (level) == 3)                        \
			v4l2_info(&cam->v4l2_dev, fmt "\n", ## args);         \
		else if ((level) == 4)                                        \
			v4l2_warn(&cam->v4l2_dev, fmt "\n", ## args);         \
		else if ((level) >= 5)                                        \
			v4l2_info(&cam->v4l2_dev, "[%s:%d] " fmt "\n",        \
				 __func__, __LINE__ , ## args);               \
	}                                                                     \
}

#	define KDBG(level, fmt, args...)                                      \
{                                                                             \
	if ( ((specific_debug) && (debug == (level))) ||                      \
	     ((!specific_debug) && (debug >= (level))) ) {                    \
		if ((level) >= 1 && (level) <= 4)                             \
			pr_info("w9968cf: " fmt "\n", ## args);               \
		else if ((level) >= 5)                                        \
			pr_debug("w9968cf: [%s:%d] " fmt "\n", __func__,  \
				 __LINE__ , ## args);                         \
	}                                                                     \
}
#else
	
#	define DBG(level, fmt, args...) do {;} while(0);
#	define KDBG(level, fmt, args...) do {;} while(0);
#endif

#undef PDBG
#define PDBG(fmt, args...)                                                    \
v4l2_info(&cam->v4l2_dev, "[%s:%d] " fmt "\n", __func__, __LINE__ , ## args);

#undef PDBGG
#define PDBGG(fmt, args...) do {;} while(0); 

#endif 
