

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>

#include <asm/tlb.h>
#include <asm/mach/map.h>
#include <mach/mux.h>
#include <mach/tc.h>

extern int omap1_clk_init(void);
extern void omap_check_revision(void);
extern void omap_sram_init(void);
extern void omapfb_reserve_sdram(void);


static struct map_desc omap_io_desc[] __initdata = {
	{
		.virtual	= OMAP1_IO_VIRT,
		.pfn		= __phys_to_pfn(OMAP1_IO_PHYS),
		.length		= OMAP1_IO_SIZE,
		.type		= MT_DEVICE
	}
};

#ifdef CONFIG_ARCH_OMAP730
static struct map_desc omap730_io_desc[] __initdata = {
	{
		.virtual	= OMAP730_DSP_BASE,
		.pfn		= __phys_to_pfn(OMAP730_DSP_START),
		.length		= OMAP730_DSP_SIZE,
		.type		= MT_DEVICE
	}, {
		.virtual	= OMAP730_DSPREG_BASE,
		.pfn		= __phys_to_pfn(OMAP730_DSPREG_START),
		.length		= OMAP730_DSPREG_SIZE,
		.type		= MT_DEVICE
	}
};
#endif

#ifdef CONFIG_ARCH_OMAP850
static struct map_desc omap850_io_desc[] __initdata = {
	{
		.virtual	= OMAP850_DSP_BASE,
		.pfn		= __phys_to_pfn(OMAP850_DSP_START),
		.length		= OMAP850_DSP_SIZE,
		.type		= MT_DEVICE
	}, {
		.virtual	= OMAP850_DSPREG_BASE,
		.pfn		= __phys_to_pfn(OMAP850_DSPREG_START),
		.length		= OMAP850_DSPREG_SIZE,
		.type		= MT_DEVICE
	}
};
#endif

#ifdef CONFIG_ARCH_OMAP15XX
static struct map_desc omap1510_io_desc[] __initdata = {
	{
		.virtual	= OMAP1510_DSP_BASE,
		.pfn		= __phys_to_pfn(OMAP1510_DSP_START),
		.length		= OMAP1510_DSP_SIZE,
		.type		= MT_DEVICE
	}, {
		.virtual	= OMAP1510_DSPREG_BASE,
		.pfn		= __phys_to_pfn(OMAP1510_DSPREG_START),
		.length		= OMAP1510_DSPREG_SIZE,
		.type		= MT_DEVICE
	}
};
#endif

#if defined(CONFIG_ARCH_OMAP16XX)
static struct map_desc omap16xx_io_desc[] __initdata = {
	{
		.virtual	= OMAP16XX_DSP_BASE,
		.pfn		= __phys_to_pfn(OMAP16XX_DSP_START),
		.length		= OMAP16XX_DSP_SIZE,
		.type		= MT_DEVICE
	}, {
		.virtual	= OMAP16XX_DSPREG_BASE,
		.pfn		= __phys_to_pfn(OMAP16XX_DSPREG_START),
		.length		= OMAP16XX_DSPREG_SIZE,
		.type		= MT_DEVICE
	}
};
#endif


void __init omap1_map_common_io(void)
{
	iotable_init(omap_io_desc, ARRAY_SIZE(omap_io_desc));

	
	local_flush_tlb_all();
	flush_cache_all();

	
	omap_check_revision();

#ifdef CONFIG_ARCH_OMAP730
	if (cpu_is_omap730()) {
		iotable_init(omap730_io_desc, ARRAY_SIZE(omap730_io_desc));
	}
#endif

#ifdef CONFIG_ARCH_OMAP850
	if (cpu_is_omap850()) {
		iotable_init(omap850_io_desc, ARRAY_SIZE(omap850_io_desc));
	}
#endif

#ifdef CONFIG_ARCH_OMAP15XX
	if (cpu_is_omap15xx()) {
		iotable_init(omap1510_io_desc, ARRAY_SIZE(omap1510_io_desc));
	}
#endif
#if defined(CONFIG_ARCH_OMAP16XX)
	if (cpu_is_omap16xx()) {
		iotable_init(omap16xx_io_desc, ARRAY_SIZE(omap16xx_io_desc));
	}
#endif

	omap_sram_init();
	omapfb_reserve_sdram();
}


void __init omap1_init_common_hw(void)
{
	
	omap_writew(0x0, MPU_PUBLIC_TIPB_CNTL);
	omap_writew(0x0, MPU_PRIVATE_TIPB_CNTL);

	
	omap1_clk_init();

	omap1_mux_init();
}

