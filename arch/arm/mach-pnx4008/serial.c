

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/io.h>

#include <mach/platform.h>
#include <mach/hardware.h>

#include <linux/serial_core.h>
#include <linux/serial_reg.h>
#include <mach/gpio.h>

#include <mach/clock.h>

#define UART_3		0
#define UART_4		1
#define UART_5		2
#define UART_6		3
#define UART_UNKNOWN	(-1)

#define UART3_BASE_VA	IO_ADDRESS(PNX4008_UART3_BASE)
#define UART4_BASE_VA	IO_ADDRESS(PNX4008_UART4_BASE)
#define UART5_BASE_VA	IO_ADDRESS(PNX4008_UART5_BASE)
#define UART6_BASE_VA	IO_ADDRESS(PNX4008_UART6_BASE)

#define UART_FCR_OFFSET		8
#define UART_FIFO_SIZE		64

void pnx4008_uart_init(void)
{
	u32 tmp;
	int i = UART_FIFO_SIZE;

	__raw_writel(0xC1, UART5_BASE_VA + UART_FCR_OFFSET);
	__raw_writel(0xC1, UART3_BASE_VA + UART_FCR_OFFSET);

	
	__raw_writel(0x00, UART5_BASE_VA);
	__raw_writel(0x00, UART3_BASE_VA);

	while (i--) {
		tmp = __raw_readl(UART5_BASE_VA);
		tmp = __raw_readl(UART3_BASE_VA);
	}
	__raw_writel(0, UART5_BASE_VA + UART_FCR_OFFSET);
	__raw_writel(0, UART3_BASE_VA + UART_FCR_OFFSET);

	
	start_int_set_rising_edge(SE_U3_RX_INT);
	start_int_ack(SE_U3_RX_INT);
	start_int_umask(SE_U3_RX_INT);

	start_int_set_rising_edge(SE_U5_RX_INT);
	start_int_ack(SE_U5_RX_INT);
	start_int_umask(SE_U5_RX_INT);
}

