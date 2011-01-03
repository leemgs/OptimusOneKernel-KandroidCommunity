

#ifndef MSM_GEMINI_SYNC_H
#define MSM_GEMINI_SYNC_H

#include <linux/fs.h>
#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include "msm_gemini_core.h"

struct msm_gemini_q {
	char const	*name;
	struct list_head  q;
	spinlock_t	lck;
	wait_queue_head_t wait;
	int	       unblck;
};

struct msm_gemini_q_entry {
	struct list_head list;
	void   *data;
};

struct msm_gemini_device {
	struct platform_device *pdev;
	struct resource        *mem;
	int                     irq;
	void                   *base;

	struct device *device;
	struct cdev   cdev;
	struct mutex  lock;
	char	  open_count;
	uint8_t       op_mode;

	
	struct msm_gemini_q evt_q;

	
	struct msm_gemini_q output_rtn_q;

	
	struct msm_gemini_q output_buf_q;

	
	struct msm_gemini_q input_rtn_q;

	
	struct msm_gemini_q input_buf_q;
};

int __msm_gemini_open(struct msm_gemini_device *pgmn_dev);
int __msm_gemini_release(struct msm_gemini_device *pgmn_dev);

long __msm_gemini_ioctl(struct msm_gemini_device *pgmn_dev,
	unsigned int cmd, unsigned long arg);

struct msm_gemini_device *__msm_gemini_init(struct platform_device *pdev);
int __msm_gemini_exit(struct msm_gemini_device *pgmn_dev);

#endif 
