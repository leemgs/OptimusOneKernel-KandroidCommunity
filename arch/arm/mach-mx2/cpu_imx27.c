



#include <linux/io.h>
#include <linux/module.h>

#include <mach/hardware.h>

static int cpu_silicon_rev = -1;
static int cpu_partnumber;

#define SYS_CHIP_ID             0x00    

static void query_silicon_parameter(void)
{
	u32 val;
	
	val = __raw_readl(IO_ADDRESS(SYSCTRL_BASE_ADDR) + SYS_CHIP_ID);

	cpu_silicon_rev = (int)(val >> 28);
	cpu_partnumber = (int)((val >> 12) & 0xFFFF);
}


int mx27_revision(void)
{
	if (cpu_silicon_rev == -1)
		query_silicon_parameter();

	if (cpu_partnumber != 0x8821)
		return -EINVAL;

	return cpu_silicon_rev;
}
EXPORT_SYMBOL(mx27_revision);
