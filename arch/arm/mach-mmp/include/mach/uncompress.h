

#include <linux/serial_reg.h>
#include <mach/addr-map.h>

#define UART1_BASE	(APB_PHYS_BASE + 0x36000)
#define UART2_BASE	(APB_PHYS_BASE + 0x17000)
#define UART3_BASE	(APB_PHYS_BASE + 0x18000)

static inline void putc(char c)
{
	volatile unsigned long *UART = (unsigned long *)UART2_BASE;

	
	if (!(UART[UART_IER] & UART_IER_UUE))
		return;

	while (!(UART[UART_LSR] & UART_LSR_THRE))
		barrier();

	UART[UART_TX] = c;
}


static inline void flush(void)
{
}


#define arch_decomp_setup()
#define arch_decomp_wdog()
