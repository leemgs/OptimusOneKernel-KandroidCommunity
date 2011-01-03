

#include <linux/platform_device.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach-types.h>
#include <mach/map.h>

#include "nuc950.h"

static void __init nuc950evb_map_io(void)
{
	nuc950_map_io();
	nuc950_init_clocks();
}

static void __init nuc950evb_init(void)
{
	nuc950_board_init();
}

MACHINE_START(W90P950EVB, "W90P950EVB")
	
	.phys_io	= W90X900_PA_UART,
	.io_pg_offst	= (((u32)W90X900_VA_UART) >> 18) & 0xfffc,
	.boot_params	= 0,
	.map_io		= nuc950evb_map_io,
	.init_irq	= nuc900_init_irq,
	.init_machine	= nuc950evb_init,
	.timer		= &nuc900_timer,
MACHINE_END
