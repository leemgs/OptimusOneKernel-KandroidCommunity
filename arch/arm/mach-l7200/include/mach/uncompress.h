

#include <mach/hardware.h>

#define IO_UART  IO_START + 0x00044000

#define __raw_writeb(v,p)	(*(volatile unsigned char *)(p) = (v))
#define __raw_readb(p)		(*(volatile unsigned char *)(p))

static inline void putc(int c)
{
	while(__raw_readb(IO_UART + 0x18) & 0x20 ||
	      __raw_readb(IO_UART + 0x18) & 0x08)
		barrier();

	__raw_writeb(c, IO_UART + 0x00);
}

static inline void flush(void)
{
}

static __inline__ void arch_decomp_setup(void)
{
	__raw_writeb(0x00, IO_UART + 0x08);	
	__raw_writeb(0x00, IO_UART + 0x20);	
	__raw_writeb(0x01, IO_UART + 0x14);	
}

#define arch_decomp_wdog()
