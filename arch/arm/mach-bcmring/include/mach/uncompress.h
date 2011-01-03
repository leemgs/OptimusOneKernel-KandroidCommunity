
#include <mach/csp/mm_addr.h>

#define BCMRING_UART_0_DR (*(volatile unsigned int *)MM_ADDR_IO_UARTA)
#define BCMRING_UART_0_FR (*(volatile unsigned int *)(MM_ADDR_IO_UARTA + 0x18))

static inline void putc(int c)
{
	
	while (BCMRING_UART_0_FR & (1 << 5))
		;

	BCMRING_UART_0_DR = c;
}


static inline void flush(void)
{
	
	while ((BCMRING_UART_0_FR & (1 << 7)) == 0)
		;

	
	while (BCMRING_UART_0_FR & (1 << 3))
		;
}

#define arch_decomp_setup()
#define arch_decomp_wdog()
