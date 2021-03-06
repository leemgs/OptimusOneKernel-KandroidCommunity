

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <mach/hardware.h>

#include <mach/board.h>
#include <mach/gpio.h>




static struct resource smc91x_resources[] = {
	[0] = {
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.flags  = IORESOURCE_IRQ | IORESOURCE_IRQ_LOWEDGE,
	},
};

static struct platform_device smc91x_device = {
	.name		= "smc91x",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(smc91x_resources),
	.resource	= smc91x_resources,
};

static struct resource led_resources[] = {
	[0] = {
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device led_device = {
	.name		= "omap_dbg_led",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(led_resources),
	.resource	= led_resources,
};

static struct platform_device *debug_devices[] __initdata = {
	&smc91x_device,
	&led_device,
	
	
	
	
};

int __init debug_card_init(u32 addr, unsigned gpio)
{
	int	status;

	smc91x_resources[0].start = addr + 0x300;
	smc91x_resources[0].end   = addr + 0x30f;

	smc91x_resources[1].start = gpio_to_irq(gpio);
	smc91x_resources[1].end   = gpio_to_irq(gpio);

	status = gpio_request(gpio, "SMC91x irq");
	if (status < 0) {
		printk(KERN_ERR "GPIO%d unavailable for smc91x IRQ\n", gpio);
		return status;
	}
	gpio_direction_input(gpio);

	led_resources[0].start = addr;
	led_resources[0].end   = addr + SZ_4K - 1;

	return platform_add_devices(debug_devices, ARRAY_SIZE(debug_devices));
}
