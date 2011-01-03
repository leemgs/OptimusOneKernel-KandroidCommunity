
#ifndef __ASM_ARCH_SERIAL_L7200_H
#define __ASM_ARCH_SERIAL_L7200_H

#include <mach/memory.h>


#define BASE_BAUD 3686400


#define UART1_BASE	(IO_BASE + 0x00044000)
#define UART2_BASE	(IO_BASE + 0x00045000)


#define UARTDR			0x00	
#define RXSTAT			0x04	
#define H_UBRLCR		0x08	
#define M_UBRLCR		0x0C	
#define L_UBRLCR		0x10	
#define UARTCON			0x14	
#define UARTFLG			0x18	
#define UARTINTSTAT		0x1C	
#define UARTINTMASK		0x20	


#define BR_110			0x827
#define BR_1200			0x06e
#define BR_2400			0x05f
#define BR_4800			0x02f
#define BR_9600			0x017
#define BR_14400		0x00f
#define BR_19200		0x00b
#define BR_38400		0x005
#define BR_57600		0x003
#define BR_76800 		0x002
#define BR_115200		0x001


#define RXSTAT_NO_ERR		0x00	
#define RXSTAT_FRM_ERR		0x01	
#define RXSTAT_PAR_ERR		0x02	
#define RXSTAT_OVR_ERR		0x04	


#define UBRLCR_BRK		0x01	
#define UBRLCR_PEN		0x02	
#define UBRLCR_PDIS		0x00	
#define UBRLCR_EVEN		0x04	
#define UBRLCR_STP2		0x08	
#define UBRLCR_FIFO		0x10	
#define UBRLCR_LEN5		0x60	
#define UBRLCR_LEN6		0x40	
#define UBRLCR_LEN7		0x20	
#define UBRLCR_LEN8		0x00	


#define UARTCON_UARTEN		0x01	
#define UARTCON_DMAONERR	0x08	


#define UARTFLG_UTXFF		0x20	
#define UARTFLG_URXFE		0x10	
#define UARTFLG_UBUSY		0x08	
#define UARTFLG_DCD		0x04	
#define UARTFLG_DSR		0x02	
#define UARTFLG_CTS		0x01	


#define UART_TXINT		0x01	
#define UART_RXINT		0x02	
#define UART_RXERRINT		0x04	
#define UART_MSINT		0x08	
#define UART_UDINT		0x10	
#define UART_ALLIRQS		0x1f	

#endif
