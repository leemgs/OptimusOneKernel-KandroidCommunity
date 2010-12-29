

#ifndef __ASM_ARCH_BCM_BT_LPM_H
#define __ASM_ARCH_BCM_BT_LPM_H

#include <linux/serial_core.h>


extern void bcm_bt_lpm_exit_lpm_locked(struct uart_port *uport);

struct bcm_bt_lpm_platform_data {
	unsigned int gpio_wake;   
	unsigned int gpio_host_wake;  

	
	void (*request_clock_off_locked)(struct uart_port *uport);
	
	void (*request_clock_on_locked)(struct uart_port *uport);
};

#endif
