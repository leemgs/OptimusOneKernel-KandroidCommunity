

#include <linux/platform_device.h>
#include <linux/mtd/physmap.h>
#include <linux/interrupt.h>
#include <linux/dm9000.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>

#include <mach/common.h>
#include <mach/hardware.h>
#include <mach/irqs.h>
#include <mach/imx-uart.h>
#include <mach/iomux.h>

#include "devices.h"


static struct resource flash_resource = {
	.start	= IMX_CS0_PHYS,
	.end	= IMX_CS0_PHYS + (32 * 1024 * 1024) - 1,
	.flags	= IORESOURCE_MEM,
};

static struct physmap_flash_data scb_flash_data = {
	.width  = 2,
};

static struct platform_device scb_flash_device = {
	.name		= "physmap-flash",
	.id		= 0,
	.dev = {
		.platform_data = &scb_flash_data,
	},
	.resource = &flash_resource,
	.num_resources = 1,
};




static struct dm9000_plat_data dm9000_platdata = {
	.flags	= DM9000_PLATF_16BITONLY,
};


static struct resource dm9000x_resources[] = {
	{
		.name	= "address area",
		.start	= IMX_CS5_PHYS,
		.end	= IMX_CS5_PHYS + 1,
		.flags	= IORESOURCE_MEM,	
	}, {
		.name	= "data area",
		.start	= IMX_CS5_PHYS + 4,
		.end	= IMX_CS5_PHYS + 5,
		.flags	= IORESOURCE_MEM,	
	}, {
		.start	= IRQ_GPIOC(3),
		.end	= IRQ_GPIOC(3),
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_LOWLEVEL,
	},
};

static struct platform_device dm9000x_device = {
	.name		= "dm9000",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(dm9000x_resources),
	.resource	= dm9000x_resources,
	.dev		= {
		.platform_data = &dm9000_platdata,
	}
};

static int mxc_uart1_pins[] = {
	PC9_PF_UART1_CTS,
	PC10_PF_UART1_RTS,
	PC11_PF_UART1_TXD,
	PC12_PF_UART1_RXD,
};

static int uart1_mxc_init(struct platform_device *pdev)
{
	return mxc_gpio_setup_multiple_pins(mxc_uart1_pins,
			ARRAY_SIZE(mxc_uart1_pins), "UART1");
}

static int uart1_mxc_exit(struct platform_device *pdev)
{
	mxc_gpio_release_multiple_pins(mxc_uart1_pins,
			ARRAY_SIZE(mxc_uart1_pins));
	return 0;
}

static struct imxuart_platform_data uart_pdata = {
	.init = uart1_mxc_init,
	.exit = uart1_mxc_exit,
	.flags = IMXUART_HAVE_RTSCTS,
};

static struct platform_device *devices[] __initdata = {
	&scb_flash_device,
	&dm9000x_device,
};


static void __init scb9328_init(void)
{
	mxc_register_device(&imx_uart1_device, &uart_pdata);

	printk(KERN_INFO"Scb9328: Adding devices\n");
	platform_add_devices(devices, ARRAY_SIZE(devices));
}

static void __init scb9328_timer_init(void)
{
	mx1_clocks_init(32000);
}

static struct sys_timer scb9328_timer = {
	.init	= scb9328_timer_init,
};

MACHINE_START(SCB9328, "Synertronixx scb9328")
    
	.phys_io	= 0x00200000,
	.io_pg_offst	= ((0xe0200000) >> 18) & 0xfffc,
	.boot_params	= 0x08000100,
	.map_io		= mx1_map_io,
	.init_irq	= mx1_init_irq,
	.timer		= &scb9328_timer,
	.init_machine	= scb9328_init,
MACHINE_END
