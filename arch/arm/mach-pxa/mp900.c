

#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/usb/isp116x.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>

#include <mach/pxa25x.h>
#include "generic.h"

static void isp116x_pfm_delay(struct device *dev, int delay)
{

	

	int cyc = delay / 10;

	
	__asm__ volatile ("0:\n"
		"subs %0, %1, #1\n"
		"bge 0b\n"
		:"=r" (cyc)
		:"0"(cyc)
	);
}

static struct isp116x_platform_data isp116x_pfm_data = {
	.remote_wakeup_enable = 1,
	.delay = isp116x_pfm_delay,
};

static struct resource isp116x_pfm_resources[] = {
	[0] =	{
		.start	= 0x0d000000,
		.end	= 0x0d000000 + 1,
		.flags	= IORESOURCE_MEM,
		},
	[1] =	{
		.start  = 0x0d000000 + 4,
		.end	= 0x0d000000 + 5,
		.flags  = IORESOURCE_MEM,
		},
	[2] =	{
		.start	= 61,
		.end	= 61,
		.flags	= IORESOURCE_IRQ,
		},
};

static struct platform_device mp900c_dummy_device = {
	.name		= "mp900c_dummy",
	.id		= -1,
};

static struct platform_device mp900c_usb = {
	.name		= "isp116x-hcd",
	.num_resources	= ARRAY_SIZE(isp116x_pfm_resources),
	.resource	= isp116x_pfm_resources,
	.dev.platform_data = &isp116x_pfm_data,
};

static struct platform_device *devices[] __initdata = {
	&mp900c_dummy_device,
	&mp900c_usb,
};

static void __init mp900c_init(void)
{
	printk(KERN_INFO "MobilePro 900/C machine init\n");
	platform_add_devices(devices, ARRAY_SIZE(devices));
}


MACHINE_START(NEC_MP900, "MobilePro900/C")
	.phys_io	= 0x40000000,
	.boot_params	= 0xa0220100,
	.io_pg_offst	= (io_p2v(0x40000000) >> 18) & 0xfffc,
	.timer		= &pxa_timer,
	.map_io		= pxa_map_io,
	.init_irq	= pxa25x_init_irq,
	.init_machine	= mp900c_init,
MACHINE_END

