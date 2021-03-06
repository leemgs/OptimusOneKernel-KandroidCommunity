

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/mfd/t7l66xb.h>

#include <asm/setup.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>

#include <mach/irqs.h>
#include <mach/pxa25x.h>
#include <mach/eseries-gpio.h>
#include <mach/udc.h>

#include "generic.h"
#include "eseries.h"
#include "clock.h"



static struct t7l66xb_platform_data e350_t7l66xb_info = {
	.irq_base               = IRQ_BOARD_START,
	.enable                 = &eseries_tmio_enable,
	.suspend                = &eseries_tmio_suspend,
	.resume                 = &eseries_tmio_resume,
};

static struct platform_device e350_t7l66xb_device = {
	.name           = "t7l66xb",
	.id             = -1,
	.dev            = {
		.platform_data = &e350_t7l66xb_info,
	},
	.num_resources = 2,
	.resource      = eseries_tmio_resources,
};



static struct platform_device *devices[] __initdata = {
	&e350_t7l66xb_device,
};

static void __init e350_init(void)
{
	eseries_register_clks();
	eseries_get_tmio_gpios();
	platform_add_devices(devices, ARRAY_SIZE(devices));
	pxa_set_udc_info(&e7xx_udc_mach_info);
}

MACHINE_START(E350, "Toshiba e350")
	
	.phys_io	= 0x40000000,
	.io_pg_offst	= (io_p2v(0x40000000) >> 18) & 0xfffc,
	.boot_params	= 0xa0000100,
	.map_io		= pxa_map_io,
	.init_irq	= pxa25x_init_irq,
	.fixup		= eseries_fixup,
	.init_machine	= e350_init,
	.timer		= &pxa_timer,
MACHINE_END

