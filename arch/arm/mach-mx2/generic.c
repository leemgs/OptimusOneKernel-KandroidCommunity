

#include <linux/mm.h>
#include <linux/init.h>
#include <mach/hardware.h>
#include <mach/common.h>
#include <asm/pgtable.h>
#include <asm/mach/map.h>


static struct map_desc mxc_io_desc[] __initdata = {
	
	{
		.virtual = AIPI_BASE_ADDR_VIRT,
		.pfn = __phys_to_pfn(AIPI_BASE_ADDR),
		.length = AIPI_SIZE,
		.type = MT_DEVICE
	},
	
	{
		.virtual = SAHB1_BASE_ADDR_VIRT,
		.pfn = __phys_to_pfn(SAHB1_BASE_ADDR),
		.length = SAHB1_SIZE,
		.type = MT_DEVICE
	},
	
	{
		.virtual = X_MEMC_BASE_ADDR_VIRT,
		.pfn = __phys_to_pfn(X_MEMC_BASE_ADDR),
		.length = X_MEMC_SIZE,
		.type = MT_DEVICE
	}
};


void __init mx21_map_io(void)
{
	mxc_set_cpu_type(MXC_CPU_MX21);
	mxc_arch_reset_init(IO_ADDRESS(WDOG_BASE_ADDR));

	iotable_init(mxc_io_desc, ARRAY_SIZE(mxc_io_desc));
}

void __init mx27_map_io(void)
{
	mxc_set_cpu_type(MXC_CPU_MX27);
	mxc_arch_reset_init(IO_ADDRESS(WDOG_BASE_ADDR));

	iotable_init(mxc_io_desc, ARRAY_SIZE(mxc_io_desc));
}

void __init mx27_init_irq(void)
{
	mxc_init_irq(IO_ADDRESS(AVIC_BASE_ADDR));
}

void __init mx21_init_irq(void)
{
	mx27_init_irq();
}

