

#ifndef VDEC_INTERNAL_H
#define VDEC_INTERNAL_H

#include <linux/msm_vidc_dec.h>
#include <linux/cdev.h>
#include "vidc_init.h"

struct vid_dec_msg {
	struct list_head list;
	struct vdec_msginfo vdec_msg_info;
};

struct vid_dec_dev {
	struct cdev cdev;
	struct device *device;
	resource_size_t phys_base;
	void __iomem *virt_base;
	unsigned int irq;
	struct clk *hclk;
	struct clk *hclk_div2;
	struct clk *pclk;
	unsigned long hclk_rate;
	struct mutex lock;
	s32 device_handle;
	struct video_client_ctx vdec_clients[VIDC_MAX_NUM_CLIENTS];
	u32 num_clients;
	void(*pf_timer_handler)(void *);
};

#endif
