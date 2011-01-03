

#include <linux/types.h>
#include <linux/serial_reg.h>
#include <mach/serial.h>

unsigned int system_rev;

#define UART_OMAP_MDR1		0x08	
#define OMAP_ID_730		0x355F
#define ID_MASK			0x7fff
#define check_port(base, shift) ((base[UART_OMAP_MDR1 << shift] & 7) == 0)
#define omap_get_id() ((*(volatile unsigned int *)(0xfffed404)) >> 12) & ID_MASK

static void putc(int c)
{
	volatile u8 * uart = 0;
	int shift = 2;

#ifdef CONFIG_MACH_OMAP_PALMTE
	return;
#endif

#ifdef CONFIG_ARCH_OMAP
#ifdef	CONFIG_OMAP_LL_DEBUG_UART3
	uart = (volatile u8 *)(OMAP_UART3_BASE);
#elif defined(CONFIG_OMAP_LL_DEBUG_UART2)
	uart = (volatile u8 *)(OMAP_UART2_BASE);
#else
	uart = (volatile u8 *)(OMAP_UART1_BASE);
#endif

#ifdef CONFIG_ARCH_OMAP1
	
	do {
		
		unsigned int omap_id = omap_get_id();

		if (omap_id == OMAP_ID_730)
			shift = 0;

		if (check_port(uart, shift))
			break;
		
		return;
	} while (0);
#endif 
#endif

	
	while (!(uart[UART_LSR << shift] & UART_LSR_THRE))
		barrier();
	uart[UART_TX << shift] = c;
}

static inline void flush(void)
{
}


#define arch_decomp_setup()
#define arch_decomp_wdog()
