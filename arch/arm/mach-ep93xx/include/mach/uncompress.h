

#include <mach/ep93xx-regs.h>

static unsigned char __raw_readb(unsigned int ptr)
{
	return *((volatile unsigned char *)ptr);
}

static unsigned int __raw_readl(unsigned int ptr)
{
	return *((volatile unsigned int *)ptr);
}

static void __raw_writeb(unsigned char value, unsigned int ptr)
{
	*((volatile unsigned char *)ptr) = value;
}

static void __raw_writel(unsigned int value, unsigned int ptr)
{
	*((volatile unsigned int *)ptr) = value;
}

#if defined(CONFIG_EP93XX_EARLY_UART1)
#define UART_BASE		EP93XX_UART1_PHYS_BASE
#elif defined(CONFIG_EP93XX_EARLY_UART2)
#define UART_BASE		EP93XX_UART2_PHYS_BASE
#elif defined(CONFIG_EP93XX_EARLY_UART3)
#define UART_BASE		EP93XX_UART3_PHYS_BASE
#else
#define UART_BASE		EP93XX_UART1_PHYS_BASE
#endif

#define PHYS_UART_DATA		(UART_BASE + 0x00)
#define PHYS_UART_FLAG		(UART_BASE + 0x18)
#define UART_FLAG_TXFF		0x20

static inline void putc(int c)
{
	int i;

	for (i = 0; i < 1000; i++) {
		
		if (!(__raw_readb(PHYS_UART_FLAG) & UART_FLAG_TXFF))
			break;
	}

	__raw_writeb(c, PHYS_UART_DATA);
}

static inline void flush(void)
{
}



#define PHYS_ETH_SELF_CTL		0x80010020
#define ETH_SELF_CTL_RESET		0x00000001

static void ethernet_reset(void)
{
	unsigned int v;

	
	v = __raw_readl(PHYS_ETH_SELF_CTL);
	__raw_writel(v | ETH_SELF_CTL_RESET, PHYS_ETH_SELF_CTL);

	
	while (__raw_readl(PHYS_ETH_SELF_CTL) & ETH_SELF_CTL_RESET)
		;
}


static void arch_decomp_setup(void)
{
	ethernet_reset();
}

#define arch_decomp_wdog()
