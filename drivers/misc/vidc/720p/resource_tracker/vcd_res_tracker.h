
#ifndef _VIDEO_720P_RESOURCE_TRACKER_H_
#define _VIDEO_720P_RESOURCE_TRACKER_H_

#include "vcd_res_tracker_api.h"

#define VCD_RESTRK_MIN_PERF_LEVEL 37900
#define VCD_RESTRK_MAX_PERF_LEVEL 108000
#define VCD_RESTRK_MIN_FREQ_POINT 61440000
#define VCD_RESTRK_MAX_FREQ_POINT 170667000
#define VCD_RESTRK_HZ_PER_1000_PERFLVL 1580250

struct res_trk_context {
	struct device *device;
	u32 irq_num;
	struct mutex lock;
	struct clk *hclk;
	struct clk *hclk_div2;
	struct clk *pclk;
	unsigned long hclk_rate;
	unsigned int clock_enabled;
	unsigned int rail_enabled;
};

#if DEBUG

#define VCDRES_MSG_LOW(xx_fmt...)	printk(KERN_INFO "\n\t* " xx_fmt)
#define VCDRES_MSG_MED(xx_fmt...)	printk(KERN_INFO "\n  * " xx_fmt)

#else

#define VCDRES_MSG_LOW(xx_fmt...)
#define VCDRES_MSG_MED(xx_fmt...)

#endif

#define VCDRES_MSG_HIGH(xx_fmt...)	printk(KERN_WARNING "\n" xx_fmt)
#define VCDRES_MSG_ERROR(xx_fmt...)	printk(KERN_ERR "\n err: " xx_fmt)
#define VCDRES_MSG_FATAL(xx_fmt...)	printk(KERN_ERR "\n<FATAL> " xx_fmt)

#endif
