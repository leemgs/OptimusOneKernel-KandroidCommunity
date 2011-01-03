
#ifndef _CCDC_HW_DEVICE_H
#define _CCDC_HW_DEVICE_H

#ifdef __KERNEL__
#include <linux/videodev2.h>
#include <linux/device.h>
#include <media/davinci/vpfe_types.h>
#include <media/davinci/ccdc_types.h>


struct ccdc_hw_ops {
	
	int (*open) (struct device *dev);
	
	int (*close) (struct device *dev);
	
	void (*set_ccdc_base)(void *base, int size);
	
	void (*enable) (int en);
	
	void (*reset) (void);
	
	void (*enable_out_to_sdram) (int en);
	
	int (*set_hw_if_params) (struct vpfe_hw_if_param *param);
	
	int (*get_hw_if_params) (struct vpfe_hw_if_param *param);
	
	int (*set_params) (void *params);
	
	int (*get_params) (void *params);
	
	int (*configure) (void);

	
	int (*set_buftype) (enum ccdc_buftype buf_type);
	
	enum ccdc_buftype (*get_buftype) (void);
	
	int (*set_frame_format) (enum ccdc_frmfmt frm_fmt);
	
	enum ccdc_frmfmt (*get_frame_format) (void);
	
	int (*enum_pix)(u32 *hw_pix, int i);
	
	u32 (*get_pixel_format) (void);
	
	int (*set_pixel_format) (u32 pixfmt);
	
	int (*set_image_window) (struct v4l2_rect *win);
	
	void (*get_image_window) (struct v4l2_rect *win);
	
	unsigned int (*get_line_length) (void);

	
	int (*queryctrl)(struct v4l2_queryctrl *qctrl);
	
	int (*set_control)(struct v4l2_control *ctrl);
	
	int (*get_control)(struct v4l2_control *ctrl);

	
	void (*setfbaddr) (unsigned long addr);
	
	int (*getfid) (void);
};

struct ccdc_hw_device {
	
	char name[32];
	
	struct module *owner;
	
	struct ccdc_hw_ops hw_ops;
};


int vpfe_register_ccdc_device(struct ccdc_hw_device *dev);
void vpfe_unregister_ccdc_device(struct ccdc_hw_device *dev);

#endif
#endif
