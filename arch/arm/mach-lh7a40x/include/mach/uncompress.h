

#include <mach/registers.h>

#ifndef UART_R_DATA
# define UART_R_DATA	(0x00)
#endif
#ifndef UART_R_STATUS
# define UART_R_STATUS	(0x10)
#endif
#define nTxRdy		(0x20)	

	
#define UART_STATUS (*(volatile unsigned long*) (UART2_PHYS + UART_R_STATUS))
#define UART_DATA   (*(volatile unsigned long*) (UART2_PHYS + UART_R_DATA))

static inline void putc(int ch)
{
	while (UART_STATUS & nTxRdy)
		barrier();
	UART_DATA = ch;
}

static inline void flush(void)
{
}

	
#define arch_decomp_setup()
#define arch_decomp_wdog()
