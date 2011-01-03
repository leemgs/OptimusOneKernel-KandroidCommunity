

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/serial_8250.h>
#include <linux/serial_reg.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <asm/irq.h>
#include <mach/hardware.h>
#include <mach/serial.h>
#include <mach/irqs.h>
#include <mach/cputype.h>
#include <mach/common.h>

#include "clock.h"

static inline unsigned int serial_read_reg(struct plat_serial8250_port *up,
					   int offset)
{
	offset <<= up->regshift;
	return (unsigned int)__raw_readl(IO_ADDRESS(up->mapbase) + offset);
}

static inline void serial_write_reg(struct plat_serial8250_port *p, int offset,
				    int value)
{
	offset <<= p->regshift;
	__raw_writel(value, IO_ADDRESS(p->mapbase) + offset);
}

static void __init davinci_serial_reset(struct plat_serial8250_port *p)
{
	unsigned int pwremu = 0;

	serial_write_reg(p, UART_IER, 0);  

	
	serial_write_reg(p, UART_DAVINCI_PWREMU, pwremu);
	mdelay(10);

	pwremu |= (0x3 << 13);
	pwremu |= 0x1;
	serial_write_reg(p, UART_DAVINCI_PWREMU, pwremu);

	if (cpu_is_davinci_dm646x())
		serial_write_reg(p, UART_DM646X_SCR,
				 UART_DM646X_SCR_TX_WATERMARK);
}

int __init davinci_serial_init(struct davinci_uart_config *info)
{
	int i;
	char name[16];
	struct clk *uart_clk;
	struct davinci_soc_info *soc_info = &davinci_soc_info;
	struct device *dev = &soc_info->serial_dev->dev;
	struct plat_serial8250_port *p = dev->platform_data;

	
	for (i = 0; i < DAVINCI_MAX_NR_UARTS; i++, p++) {
		if (!(info->enabled_uarts & (1 << i)))
			continue;

		sprintf(name, "uart%d", i);
		uart_clk = clk_get(dev, name);
		if (IS_ERR(uart_clk))
			printk(KERN_ERR "%s:%d: failed to get UART%d clock\n",
					__func__, __LINE__, i);
		else {
			clk_enable(uart_clk);
			p->uartclk = clk_get_rate(uart_clk);
			davinci_serial_reset(p);
		}
	}

	return platform_device_register(soc_info->serial_dev);
}
