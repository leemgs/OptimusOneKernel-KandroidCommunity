

#ifndef PWC_H
#define PWC_H

#include <linux/module.h>
#include <linux/usb.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/version.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <asm/errno.h>
#include <linux/videodev.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>
#ifdef CONFIG_USB_PWC_INPUT_EVDEV
#include <linux/input.h>
#endif

#include "pwc-uncompress.h"
#include <media/pwc-ioctl.h>


#define PWC_MAJOR	10
#define PWC_MINOR	0
#define PWC_EXTRAMINOR	12
#define PWC_VERSION_CODE KERNEL_VERSION(PWC_MAJOR,PWC_MINOR,PWC_EXTRAMINOR)
#define PWC_VERSION 	"10.0.13"
#define PWC_NAME 	"pwc"
#define PFX		PWC_NAME ": "



#define PWC_DEBUG_LEVEL_MODULE	(1<<0)
#define PWC_DEBUG_LEVEL_PROBE	(1<<1)
#define PWC_DEBUG_LEVEL_OPEN	(1<<2)
#define PWC_DEBUG_LEVEL_READ	(1<<3)
#define PWC_DEBUG_LEVEL_MEMORY	(1<<4)
#define PWC_DEBUG_LEVEL_FLOW	(1<<5)
#define PWC_DEBUG_LEVEL_SIZE	(1<<6)
#define PWC_DEBUG_LEVEL_IOCTL	(1<<7)
#define PWC_DEBUG_LEVEL_TRACE	(1<<8)

#define PWC_DEBUG_MODULE(fmt, args...) PWC_DEBUG(MODULE, fmt, ##args)
#define PWC_DEBUG_PROBE(fmt, args...) PWC_DEBUG(PROBE, fmt, ##args)
#define PWC_DEBUG_OPEN(fmt, args...) PWC_DEBUG(OPEN, fmt, ##args)
#define PWC_DEBUG_READ(fmt, args...) PWC_DEBUG(READ, fmt, ##args)
#define PWC_DEBUG_MEMORY(fmt, args...) PWC_DEBUG(MEMORY, fmt, ##args)
#define PWC_DEBUG_FLOW(fmt, args...) PWC_DEBUG(FLOW, fmt, ##args)
#define PWC_DEBUG_SIZE(fmt, args...) PWC_DEBUG(SIZE, fmt, ##args)
#define PWC_DEBUG_IOCTL(fmt, args...) PWC_DEBUG(IOCTL, fmt, ##args)
#define PWC_DEBUG_TRACE(fmt, args...) PWC_DEBUG(TRACE, fmt, ##args)


#ifdef CONFIG_USB_PWC_DEBUG

#define PWC_DEBUG_LEVEL	(PWC_DEBUG_LEVEL_MODULE)

#define PWC_DEBUG(level, fmt, args...) do {\
	  if ((PWC_DEBUG_LEVEL_ ##level) & pwc_trace) \
	     printk(KERN_DEBUG PFX fmt, ##args); \
	  } while(0)

#define PWC_ERROR(fmt, args...) printk(KERN_ERR PFX fmt, ##args)
#define PWC_WARNING(fmt, args...) printk(KERN_WARNING PFX fmt, ##args)
#define PWC_INFO(fmt, args...) printk(KERN_INFO PFX fmt, ##args)
#define PWC_TRACE(fmt, args...) PWC_DEBUG(TRACE, fmt, ##args)

#else 

#define PWC_ERROR(fmt, args...) printk(KERN_ERR PFX fmt, ##args)
#define PWC_WARNING(fmt, args...) printk(KERN_WARNING PFX fmt, ##args)
#define PWC_INFO(fmt, args...) printk(KERN_INFO PFX fmt, ##args)
#define PWC_TRACE(fmt, args...) do { } while(0)
#define PWC_DEBUG(level, fmt, args...) do { } while(0)

#define pwc_trace 0

#endif


#define TOUCAM_HEADER_SIZE		8
#define TOUCAM_TRAILER_SIZE		4

#define FEATURE_MOTOR_PANTILT		0x0001
#define FEATURE_CODEC1			0x0002
#define FEATURE_CODEC2			0x0004


#define PWC_INT_PIPE 0


#define FRAME_LOWMARK 5


#define MAX_ISO_BUFS		2
#define ISO_FRAMES_PER_DESC	10
#define ISO_MAX_FRAME_SIZE	960
#define ISO_BUFFER_SIZE 	(ISO_FRAMES_PER_DESC * ISO_MAX_FRAME_SIZE)


#define MAX_FRAMES		5

#define PWC_FRAME_SIZE 		(460800 + TOUCAM_HEADER_SIZE + TOUCAM_TRAILER_SIZE)


#define MAX_IMAGES 		10


#define DEVICE_USE_CODEC1(x) ((x)<675)
#define DEVICE_USE_CODEC2(x) ((x)>=675 && (x)<700)
#define DEVICE_USE_CODEC3(x) ((x)>=700)
#define DEVICE_USE_CODEC23(x) ((x)>=675)


struct pwc_iso_buf
{
	void *data;
	int  length;
	int  read;
	struct urb *urb;
};


struct pwc_frame_buf
{
   void *data;
   volatile int filled;		
   struct pwc_frame_buf *next;	
};


struct pwc_imgbuf
{
	unsigned long offset;	
	int   vma_use_count;	
};

struct pwc_device
{
   struct video_device *vdev;

   
   struct usb_device *udev;

   int type;                    
   int release;			
   int features;		
   char serial[30];		
   int error_status;		
   int usb_init;		

   
   int vopen;			
   int vendpoint;		
   int vcinterface;		
   int valternate;		
   int vframes, vsize;		
   int vpalette;		
   int vframe_count;		
   int vframes_dumped; 		
   int vframes_error;		
   int vmax_packet_size;	
   int vlast_packet_size;	
   int visoc_errors;		
   int vcompression;		
   int vbandlength;		
   char vsnapshot;		
   char vsync;			
   char vmirror;		
	char unplugged;

   int cmd_len;
   unsigned char cmd_buf[13];

   
   
   struct pwc_iso_buf sbuf[MAX_ISO_BUFS];
   char iso_init;

   
   struct pwc_frame_buf *fbuf;	
   struct pwc_frame_buf *empty_frames, *empty_frames_tail;	
   struct pwc_frame_buf *full_frames, *full_frames_tail;	
   struct pwc_frame_buf *fill_frame;	
   struct pwc_frame_buf *read_frame;	
   int frame_header_size, frame_trailer_size;
   int frame_size;
   int frame_total_size; 
   int drop_frames;

   
   void *decompress_data;		

   
   
   int image_mask;			
   struct pwc_coord view_min, view_max;	
   struct pwc_coord abs_max;            
   struct pwc_coord image, view;	
   struct pwc_coord offset;		

   void *image_data;			
   struct pwc_imgbuf images[MAX_IMAGES];
   int fill_image;			
   int len_per_image;			
   int image_read_pos;			
   int image_used[MAX_IMAGES];		

   struct mutex modlock;		
   spinlock_t ptrlock;			

   
   struct pwc_mpt_range angle_range;
   int pan_angle;			
   int tilt_angle;			
   int snapshot_button_status;		
#ifdef CONFIG_USB_PWC_INPUT_EVDEV
   struct input_dev *button_dev;	
   char button_phys[64];
#endif

   
   wait_queue_head_t frameq;		
#if PWC_INT_PIPE
   void *usb_int_handler;		
#endif
};

#ifdef __cplusplus
extern "C" {
#endif


#ifdef CONFIG_USB_PWC_DEBUG
extern int pwc_trace;
#endif
extern int pwc_mbufs;


int pwc_try_video_mode(struct pwc_device *pdev, int width, int height, int new_fps, int new_compression, int new_snapshot);
int pwc_handle_frame(struct pwc_device *pdev);
void pwc_next_image(struct pwc_device *pdev);
int pwc_isoc_init(struct pwc_device *pdev);
void pwc_isoc_cleanup(struct pwc_device *pdev);



extern const struct pwc_coord pwc_image_sizes[PSZ_MAX];

int pwc_decode_size(struct pwc_device *pdev, int width, int height);
void pwc_construct(struct pwc_device *pdev);



extern int pwc_set_video_mode(struct pwc_device *pdev, int width, int height, int frames, int compression, int snapshot);
extern unsigned int pwc_get_fps(struct pwc_device *pdev, unsigned int index, unsigned int size);

extern int pwc_mpt_reset(struct pwc_device *pdev, int flags);
extern int pwc_mpt_set_angle(struct pwc_device *pdev, int pan, int tilt);


extern int pwc_get_brightness(struct pwc_device *pdev);
extern int pwc_set_brightness(struct pwc_device *pdev, int value);
extern int pwc_get_contrast(struct pwc_device *pdev);
extern int pwc_set_contrast(struct pwc_device *pdev, int value);
extern int pwc_get_gamma(struct pwc_device *pdev);
extern int pwc_set_gamma(struct pwc_device *pdev, int value);
extern int pwc_get_saturation(struct pwc_device *pdev, int *value);
extern int pwc_set_saturation(struct pwc_device *pdev, int value);
extern int pwc_set_leds(struct pwc_device *pdev, int on_value, int off_value);
extern int pwc_get_cmos_sensor(struct pwc_device *pdev, int *sensor);
extern int pwc_restore_user(struct pwc_device *pdev);
extern int pwc_save_user(struct pwc_device *pdev);
extern int pwc_restore_factory(struct pwc_device *pdev);


extern int pwc_get_red_gain(struct pwc_device *pdev, int *value);
extern int pwc_set_red_gain(struct pwc_device *pdev, int value);
extern int pwc_get_blue_gain(struct pwc_device *pdev, int *value);
extern int pwc_set_blue_gain(struct pwc_device *pdev, int value);
extern int pwc_get_awb(struct pwc_device *pdev);
extern int pwc_set_awb(struct pwc_device *pdev, int mode);
extern int pwc_set_agc(struct pwc_device *pdev, int mode, int value);
extern int pwc_get_agc(struct pwc_device *pdev, int *value);
extern int pwc_set_shutter_speed(struct pwc_device *pdev, int mode, int value);
extern int pwc_get_shutter_speed(struct pwc_device *pdev, int *value);

extern int pwc_set_colour_mode(struct pwc_device *pdev, int colour);
extern int pwc_get_colour_mode(struct pwc_device *pdev, int *colour);
extern int pwc_set_contour(struct pwc_device *pdev, int contour);
extern int pwc_get_contour(struct pwc_device *pdev, int *contour);
extern int pwc_set_backlight(struct pwc_device *pdev, int backlight);
extern int pwc_get_backlight(struct pwc_device *pdev, int *backlight);
extern int pwc_set_flicker(struct pwc_device *pdev, int flicker);
extern int pwc_get_flicker(struct pwc_device *pdev, int *flicker);
extern int pwc_set_dynamic_noise(struct pwc_device *pdev, int noise);
extern int pwc_get_dynamic_noise(struct pwc_device *pdev, int *noise);


extern int pwc_camera_power(struct pwc_device *pdev, int power);


extern long pwc_ioctl(struct pwc_device *pdev, unsigned int cmd, void *arg);


extern long pwc_video_do_ioctl(struct file *file, unsigned int cmd, void *arg);



extern int pwc_decompress(struct pwc_device *pdev);

#ifdef __cplusplus
}
#endif


#endif

