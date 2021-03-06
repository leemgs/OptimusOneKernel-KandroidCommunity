
#ifndef __ASM_ARCH_SERIAL_H
#define __ASM_ARCH_SERIAL_H


#define BASE_BAUD	3686400


#define STD_COM_FLAGS (ASYNC_BOOT_AUTOCONF | ASYNC_SKIP_TEST)

#define STD_SERIAL_PORT_DEFNS		\
				\
	{ 0, BASE_BAUD, UART1_BASE, IRQ_UART_1, STD_COM_FLAGS },   \
	{ 0, BASE_BAUD, UART2_BASE, IRQ_UART_2, STD_COM_FLAGS },   \

#define EXTRA_SERIAL_PORT_DEFNS

#endif
