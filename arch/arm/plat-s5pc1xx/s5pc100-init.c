

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>

#include <plat/cpu.h>
#include <plat/devs.h>
#include <plat/s5pc100.h>



void __init s5pc100_common_init_uarts(struct s3c2410_uartcfg *cfg, int no)
{
	
	s3c24xx_init_uartdevs("s3c6400-uart", s5pc1xx_uart_resources, cfg, no);
}
