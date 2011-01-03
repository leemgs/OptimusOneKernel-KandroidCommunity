

#include <linux/types.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/amba/bus.h>
#include <linux/gpio.h>

#include <mach/hardware.h>
#include <mach/irqs.h>
#include <asm/mach/map.h>
#include <asm/hardware/vic.h>

#include <asm/cacheflush.h>
#include <asm/hardware/cache-l2x0.h>


static struct nmk_gpio_platform_data cpu8815_gpio[] = {
	{
		.name = "GPIO-0-31",
		.first_gpio = 0,
		.first_irq = NOMADIK_GPIO_TO_IRQ(0),
		.parent_irq = IRQ_GPIO0,
	}, {
		.name = "GPIO-32-63",
		.first_gpio = 32,
		.first_irq = NOMADIK_GPIO_TO_IRQ(32),
		.parent_irq = IRQ_GPIO1,
	}, {
		.name = "GPIO-64-95",
		.first_gpio = 64,
		.first_irq = NOMADIK_GPIO_TO_IRQ(64),
		.parent_irq = IRQ_GPIO2,
	}, {
		.name = "GPIO-96-127", 
		.first_gpio = 96,
		.first_irq = NOMADIK_GPIO_TO_IRQ(96),
		.parent_irq = IRQ_GPIO3,
	}
};

#define __MEM_4K_RESOURCE(x) \
	.res = {.start = (x), .end = (x) + SZ_4K - 1, .flags = IORESOURCE_MEM}

static struct amba_device cpu8815_amba_gpio[] = {
	{
		.dev = {
			.init_name = "gpio0",
			.platform_data = cpu8815_gpio + 0,
		},
		__MEM_4K_RESOURCE(NOMADIK_GPIO0_BASE),
	}, {
		.dev = {
			.init_name = "gpio1",
			.platform_data = cpu8815_gpio + 1,
		},
		__MEM_4K_RESOURCE(NOMADIK_GPIO1_BASE),
	}, {
		.dev = {
			.init_name = "gpio2",
			.platform_data = cpu8815_gpio + 2,
		},
		__MEM_4K_RESOURCE(NOMADIK_GPIO2_BASE),
	}, {
		.dev = {
			.init_name = "gpio3",
			.platform_data = cpu8815_gpio + 3,
		},
		__MEM_4K_RESOURCE(NOMADIK_GPIO3_BASE),
	},
};

static struct amba_device *amba_devs[] __initdata = {
	cpu8815_amba_gpio + 0,
	cpu8815_amba_gpio + 1,
	cpu8815_amba_gpio + 2,
	cpu8815_amba_gpio + 3,
};

static int __init cpu8815_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(amba_devs); i++)
		amba_device_register(amba_devs[i], &iomem_resource);
	return 0;
}
arch_initcall(cpu8815_init);


static struct map_desc nomadik_io_desc[] __initdata = {
	{
		.virtual =	NOMADIK_IO_VIRTUAL,
		.pfn =		__phys_to_pfn(NOMADIK_IO_PHYSICAL),
		.length =	NOMADIK_IO_SIZE,
		.type = 	MT_DEVICE,
	}
	
};

void __init cpu8815_map_io(void)
{
	iotable_init(nomadik_io_desc, ARRAY_SIZE(nomadik_io_desc));
}

void __init cpu8815_init_irq(void)
{
	
	vic_init(io_p2v(NOMADIK_IC_BASE + 0x00), IRQ_VIC_START +  0, ~0, 0);
	vic_init(io_p2v(NOMADIK_IC_BASE + 0x20), IRQ_VIC_START + 32, ~0, 0);
}


 void __init cpu8815_platform_init(void)
{
#ifdef CONFIG_CACHE_L2X0
	
	l2x0_init(io_p2v(NOMADIK_L2CC_BASE), 0x00730249, 0xfe000fff);
#endif
	 return;
}
