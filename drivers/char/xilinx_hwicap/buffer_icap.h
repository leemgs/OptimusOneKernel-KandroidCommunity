

#ifndef XILINX_BUFFER_ICAP_H_	
#define XILINX_BUFFER_ICAP_H_	

#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include "xilinx_hwicap.h"


int buffer_icap_set_configuration(struct hwicap_drvdata *drvdata, u32 *data,
			     u32 Size);


int buffer_icap_get_configuration(struct hwicap_drvdata *drvdata, u32 *data,
			     u32 Size);

u32 buffer_icap_get_status(struct hwicap_drvdata *drvdata);
void buffer_icap_reset(struct hwicap_drvdata *drvdata);

#endif
