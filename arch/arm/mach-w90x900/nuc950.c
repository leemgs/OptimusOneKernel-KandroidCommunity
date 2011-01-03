

#include <linux/platform_device.h>
#include <asm/mach/map.h>
#include <mach/hardware.h>
#include "cpu.h"



static struct platform_device *nuc950_dev[] __initdata = {
	&nuc900_device_kpi,
	&nuc900_device_fmi,
};



static struct map_desc nuc950evb_iodesc[] __initdata = {
};



void __init nuc950_map_io(void)
{
	nuc900_map_io(nuc950evb_iodesc, ARRAY_SIZE(nuc950evb_iodesc));
}



void __init nuc950_init_clocks(void)
{
	nuc900_init_clocks();
}



void __init nuc950_board_init(void)
{
	nuc900_board_init(nuc950_dev, ARRAY_SIZE(nuc950_dev));
}
