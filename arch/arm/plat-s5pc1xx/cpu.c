

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <mach/hardware.h>
#include <mach/map.h>

#include <asm/mach/map.h>

#include <plat/regs-serial.h>

#include <plat/cpu.h>
#include <plat/devs.h>
#include <plat/clock.h>

#include <plat/s5pc100.h>



static const char name_s5pc100[] = "S5PC100";

static struct cpu_table cpu_ids[] __initdata = {
	{
		.idcode		= 0x43100000,
		.idmask		= 0xfffff000,
		.map_io		= s5pc100_map_io,
		.init_clocks	= s5pc100_init_clocks,
		.init_uarts	= s5pc100_init_uarts,
		.init		= s5pc100_init,
		.name		= name_s5pc100,
	},
};



#define UART_OFFS (S3C_PA_UART & 0xffff)

static struct map_desc s5pc1xx_iodesc[] __initdata = {
	{
		.virtual	= (unsigned long)S5PC1XX_VA_CHIPID,
		.pfn		= __phys_to_pfn(S5PC1XX_PA_CHIPID),
		.length		= SZ_16,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (unsigned long)S5PC1XX_VA_CLK,
		.pfn		= __phys_to_pfn(S5PC1XX_PA_CLK),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (unsigned long)S5PC1XX_VA_PWR,
		.pfn		= __phys_to_pfn(S5PC1XX_PA_PWR),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (unsigned long)(S5PC1XX_VA_UART),
		.pfn		= __phys_to_pfn(S5PC1XX_PA_UART),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (unsigned long)S5PC1XX_VA_VIC(0),
		.pfn		= __phys_to_pfn(S5PC1XX_PA_VIC(0)),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (unsigned long)S5PC1XX_VA_VIC(1),
		.pfn		= __phys_to_pfn(S5PC1XX_PA_VIC(1)),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (unsigned long)S5PC1XX_VA_VIC(2),
		.pfn		= __phys_to_pfn(S5PC1XX_PA_VIC(2)),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (unsigned long)S5PC1XX_VA_TIMER,
		.pfn		= __phys_to_pfn(S5PC1XX_PA_TIMER),
		.length		= SZ_256,
		.type		= MT_DEVICE,
	},
};



void __init s5pc1xx_init_io(struct map_desc *mach_desc, int size)
{
	unsigned long idcode;

	
	iotable_init(s5pc1xx_iodesc, ARRAY_SIZE(s5pc1xx_iodesc));
	iotable_init(mach_desc, size);

	idcode = __raw_readl(S5PC1XX_VA_CHIPID);
	s3c_init_cpu(idcode, cpu_ids, ARRAY_SIZE(cpu_ids));
}
