

#include <linux/serial_reg.h>

#define UART_BASE	0xc0030000

#define PHYS(x)          ((volatile unsigned long *)(UART_BASE + x))

#define UARTDR          PHYS(0x00)      
#define UARTDLL         PHYS(0x00)      
#define UARTDLM         PHYS(0x04)      
#define UARTIER         PHYS(0x04)      
#define UARTFCR         PHYS(0x08)      
#define UARTLCR         PHYS(0x0c)      
#define UARTSR          PHYS(0x14)      


static inline void putc(int c)
{
	int j = 0x1000;

	while (--j && !(*UARTSR & UART_LSR_THRE))
		barrier();

	*UARTDR = c;
}

static inline void flush(void)
{
}

#define arch_decomp_setup()
#define arch_decomp_wdog()
