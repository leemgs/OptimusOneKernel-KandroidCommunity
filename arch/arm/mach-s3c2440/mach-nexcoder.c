

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/string.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <linux/mtd/map.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <asm/setup.h>
#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach-types.h>


#include <mach/regs-gpio.h>
#include <plat/regs-serial.h>
#include <plat/iic.h>

#include <plat/s3c2410.h>
#include <plat/s3c2440.h>
#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/cpu.h>

static struct map_desc nexcoder_iodesc[] __initdata = {
	
};

#define UCON S3C2410_UCON_DEFAULT
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB
#define UFCON S3C2410_UFCON_RXTRIG12 | S3C2410_UFCON_FIFOMODE

static struct s3c2410_uartcfg nexcoder_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	}
};



static struct resource nexcoder_nor_resource[] = {
	[0] = {
		.start = S3C2410_CS0,
		.end   = S3C2410_CS0 + (8*1024*1024) - 1,
		.flags = IORESOURCE_MEM,
	}
};

static struct map_info nexcoder_nor_map = {
	.bankwidth = 2,
};

static struct platform_device nexcoder_device_nor = {
	.name		= "mtd-flash",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(nexcoder_nor_resource),
	.resource	= nexcoder_nor_resource,
	.dev =
	{
		.platform_data = &nexcoder_nor_map,
	}
};



static struct platform_device *nexcoder_devices[] __initdata = {
	&s3c_device_usb,
	&s3c_device_lcd,
	&s3c_device_wdt,
	&s3c_device_i2c0,
	&s3c_device_iis,
 	&s3c_device_rtc,
	&s3c_device_camif,
	&s3c_device_spi0,
	&s3c_device_spi1,
	&nexcoder_device_nor,
};

static void __init nexcoder_sensorboard_init(void)
{
	
	s3c2410_gpio_setpin(S3C2410_GPE(14), 1); 
	s3c2410_gpio_cfgpin(S3C2410_GPE(14), S3C2410_GPIO_OUTPUT);
	s3c2410_gpio_setpin(S3C2410_GPE(15), 1); 
	s3c2410_gpio_cfgpin(S3C2410_GPE(15), S3C2410_GPIO_OUTPUT);

	
	s3c2410_gpio_setpin(S3C2410_GPF(1), 1);
	s3c2410_gpio_cfgpin(S3C2410_GPF(1), S3C2410_GPIO_OUTPUT); 
	s3c2410_gpio_setpin(S3C2410_GPF(2), 0);
	s3c2410_gpio_cfgpin(S3C2410_GPF(2), S3C2410_GPIO_OUTPUT); 
}

static void __init nexcoder_map_io(void)
{
	s3c24xx_init_io(nexcoder_iodesc, ARRAY_SIZE(nexcoder_iodesc));
	s3c24xx_init_clocks(0);
	s3c24xx_init_uarts(nexcoder_uartcfgs, ARRAY_SIZE(nexcoder_uartcfgs));

	nexcoder_sensorboard_init();
}

static void __init nexcoder_init(void)
{
	s3c_i2c0_set_platdata(NULL);
	platform_add_devices(nexcoder_devices, ARRAY_SIZE(nexcoder_devices));
};

MACHINE_START(NEXCODER_2440, "NexVision - Nexcoder 2440")
	
	.phys_io	= S3C2410_PA_UART,
	.io_pg_offst	= (((u32)S3C24XX_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S3C2410_SDRAM_PA + 0x100,
	.map_io		= nexcoder_map_io,
	.init_machine	= nexcoder_init,
	.init_irq	= s3c24xx_init_irq,
	.timer		= &s3c24xx_timer,
MACHINE_END
