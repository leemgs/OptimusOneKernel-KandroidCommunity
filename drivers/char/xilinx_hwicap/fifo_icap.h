

#ifndef XILINX_FIFO_ICAP_H_	
#define XILINX_FIFO_ICAP_H_	

#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include "xilinx_hwicap.h"


int fifo_icap_get_configuration(
		struct hwicap_drvdata *drvdata,
		u32 *FrameBuffer,
		u32 NumWords);


int fifo_icap_set_configuration(
		struct hwicap_drvdata *drvdata,
		u32 *FrameBuffer,
		u32 NumWords);

u32 fifo_icap_get_status(struct hwicap_drvdata *drvdata);
void fifo_icap_reset(struct hwicap_drvdata *drvdata);
void fifo_icap_flush_fifo(struct hwicap_drvdata *drvdata);

#endif
