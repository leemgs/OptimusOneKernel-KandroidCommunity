

#ifndef __ASM_ARCH_MSM_SERIAL_HS_H
#define __ASM_ARCH_MSM_SERIAL_HS_H

#include<linux/serial_core.h>


struct msm_serial_hs_platform_data {
	int wakeup_irq;  
	
	unsigned char inject_rx_on_wakeup;
	char rx_to_inject;
};

unsigned int msm_hs_tx_empty(struct uart_port *uport);
void msm_hs_request_clock_off(struct uart_port *uport);
void msm_hs_request_clock_on(struct uart_port *uport);
void msm_hs_set_mctrl(struct uart_port *uport,
				    unsigned int mctrl);

#if defined(CONFIG_MACH_LGE)
struct uart_port * msm_hs_get_bt_uport(unsigned int line);
#endif

#endif
