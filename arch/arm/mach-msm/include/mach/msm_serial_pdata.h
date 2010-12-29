

#ifndef __ASM_ARCH_MSM_SERIAL_HS_H
#define __ASM_ARCH_MSM_SERIAL_HS_H

#include <linux/serial_core.h>


struct msm_serial_platform_data {
	int wakeup_irq;  
	
	unsigned char inject_rx_on_wakeup;
	char rx_to_inject;
};

#endif
