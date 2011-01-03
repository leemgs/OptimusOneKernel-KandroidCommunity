
#include <linux/init.h>

#include <asm/setup.h>


void __init x86_mrst_early_setup(void)
{
	x86_init.resources.probe_roms = x86_init_noop;
	x86_init.resources.reserve_resources = x86_init_noop;
}
