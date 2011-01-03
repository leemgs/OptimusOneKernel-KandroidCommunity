#ifndef GSPCAV2_H
#define GSPCAV2_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/videodev2.h>
#include <media/v4l2-common.h>
#include <linux/mutex.h>


#define GSPCA_DEBUG 1

#ifdef GSPCA_DEBUG

extern int gspca_debug;
#define PDEBUG(level, fmt, args...) \
	do {\
		if (gspca_debug & (level)) \
			printk(KERN_INFO MODULE_NAME ": " fmt "\n", ## args); \
	} while (0)
#define D_ERR  0x01
#define D_PROBE 0x02
#define D_CONF 0x04
#define D_STREAM 0x08
#define D_FRAM 0x10
#define D_PACK 0x20
#define D_USBI 0x40
#define D_USBO 0x80
#define D_V4L2 0x0100
#else
#define PDEBUG(level, fmt, args...)
#endif
#undef err
#define err(fmt, args...) \
	printk(KERN_ERR MODULE_NAME ": " fmt "\n", ## args)
#undef info
#define info(fmt, args...) \
	printk(KERN_INFO MODULE_NAME ": " fmt "\n", ## args)
#undef warn
#define warn(fmt, args...) \
	printk(KERN_WARNING MODULE_NAME ": " fmt "\n", ## args)

#define GSPCA_MAX_FRAMES 16	

#define MAX_NURBS 4		


struct cam {
	int bulk_size;		
	const struct v4l2_pix_format *cam_mode;	
	char nmodes;
	__u8 bulk_nurbs;	
	u8 bulk;		
	u8 npkt;		
	u32 input_flags;	
};

struct gspca_dev;
struct gspca_frame;


typedef int (*cam_op) (struct gspca_dev *);
typedef void (*cam_v_op) (struct gspca_dev *);
typedef int (*cam_cf_op) (struct gspca_dev *, const struct usb_device_id *);
typedef int (*cam_jpg_op) (struct gspca_dev *,
				struct v4l2_jpegcompression *);
typedef int (*cam_reg_op) (struct gspca_dev *,
				struct v4l2_dbg_register *);
typedef int (*cam_ident_op) (struct gspca_dev *,
				struct v4l2_dbg_chip_ident *);
typedef int (*cam_streamparm_op) (struct gspca_dev *,
				  struct v4l2_streamparm *);
typedef int (*cam_qmnu_op) (struct gspca_dev *,
			struct v4l2_querymenu *);
typedef void (*cam_pkt_op) (struct gspca_dev *gspca_dev,
				struct gspca_frame *frame,
				__u8 *data,
				int len);

struct ctrl {
	struct v4l2_queryctrl qctrl;
	int (*set)(struct gspca_dev *, __s32);
	int (*get)(struct gspca_dev *, __s32 *);
};


struct sd_desc {

	const char *name;	

	const struct ctrl *ctrls;
	int nctrls;

	cam_cf_op config;	
	cam_op init;		
	cam_op start;		
	cam_pkt_op pkt_scan;

	cam_op isoc_init;	
	cam_op isoc_nego;	
	cam_v_op stopN;		
	cam_v_op stop0;		
	cam_v_op dq_callback;	
	cam_jpg_op get_jcomp;
	cam_jpg_op set_jcomp;
	cam_qmnu_op querymenu;
	cam_streamparm_op get_streamparm;
	cam_streamparm_op set_streamparm;
#ifdef CONFIG_VIDEO_ADV_DEBUG
	cam_reg_op set_register;
	cam_reg_op get_register;
#endif
	cam_ident_op get_chip_ident;
};


enum gspca_packet_type {
	DISCARD_PACKET,
	FIRST_PACKET,
	INTER_PACKET,
	LAST_PACKET
};

struct gspca_frame {
	__u8 *data;			
	__u8 *data_end;			
	int vma_use_count;
	struct v4l2_buffer v4l2_buf;
};

struct gspca_dev {
	struct video_device vdev;	
	struct module *module;		
	struct usb_device *dev;
	struct file *capt_file;		

	struct cam cam;				
	const struct sd_desc *sd_desc;		
	unsigned ctrl_dis;		

#define USB_BUF_SZ 64
	__u8 *usb_buf;				
	struct urb *urb[MAX_NURBS];

	__u8 *frbuf;				
	struct gspca_frame frame[GSPCA_MAX_FRAMES];
	__u32 frsz;				
	char nframes;				
	char fr_i;				
	char fr_q;				
	char fr_o;				
	signed char fr_queue[GSPCA_MAX_FRAMES];	
	__u8 last_packet_type;
	__s8 empty_packet;		
	__u8 streaming;

	__u8 curr_mode;			
	__u32 pixfmt;			
	__u16 width;
	__u16 height;
	__u32 sequence;			

	wait_queue_head_t wq;		
	struct mutex usb_lock;		
	struct mutex read_lock;		
	struct mutex queue_lock;	
#ifdef CONFIG_PM
	char frozen;			
#endif
	char users;			
	char present;			
	char nbufread;			
	char nurbs;			
	char memory;			
	__u8 iface;			
	__u8 alt;			
	__u8 nbalt;			
	u16 pkt_size;			
};

int gspca_dev_probe(struct usb_interface *intf,
		const struct usb_device_id *id,
		const struct sd_desc *sd_desc,
		int dev_size,
		struct module *module);
void gspca_disconnect(struct usb_interface *intf);
struct gspca_frame *gspca_frame_add(struct gspca_dev *gspca_dev,
				    enum gspca_packet_type packet_type,
				    struct gspca_frame *frame,
				    const __u8 *data,
				    int len);
struct gspca_frame *gspca_get_i_frame(struct gspca_dev *gspca_dev);
#ifdef CONFIG_PM
int gspca_suspend(struct usb_interface *intf, pm_message_t message);
int gspca_resume(struct usb_interface *intf);
#endif
int gspca_auto_gain_n_exposure(struct gspca_dev *gspca_dev, int avg_lum,
	int desired_avg_lum, int deadzone, int gain_knee, int exposure_knee);
#endif 
