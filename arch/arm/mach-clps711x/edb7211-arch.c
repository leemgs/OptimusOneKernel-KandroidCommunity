
#include <linux/init.h>
#include <linux/types.h>
#include <linux/string.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>

#include "common.h"

extern void edb7211_map_io(void);

static void __init
fixup_edb7211(struct machine_desc *desc, struct tag *tags,
	      char **cmdline, struct meminfo *mi)
{
	
	mi->bank[0].start = 0xc0000000;
	mi->bank[0].size = 8*1024*1024;
	mi->bank[0].node = 0;
	mi->bank[1].start = 0xc1000000;
	mi->bank[1].size = 8*1024*1024;
	mi->bank[1].node = 1;
	mi->nr_banks = 2;
}

MACHINE_START(EDB7211, "CL-EDB7211 (EP7211 eval board)")
	
	.phys_io	= 0x80000000,
	.io_pg_offst	= ((0xff000000) >> 18) & 0xfffc,
	.boot_params	= 0xc0020100,	
	.fixup		= fixup_edb7211,
	.map_io		= edb7211_map_io,
	.init_irq	= clps711x_init_irq,
	.timer		= &clps711x_timer,
MACHINE_END
