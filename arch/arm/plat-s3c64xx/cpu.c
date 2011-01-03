

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/sysdev.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <mach/hardware.h>
#include <mach/map.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/regs-serial.h>

#include <plat/cpu.h>
#include <plat/devs.h>
#include <plat/clock.h>

#include <plat/s3c6400.h>
#include <plat/s3c6410.h>



static const char name_s3c6400[] = "S3C6400";
static const char name_s3c6410[] = "S3C6410";

static struct cpu_table cpu_ids[] __initdata = {
	{
		.idcode		= 0x36400000,
		.idmask		= 0xfffff000,
		.map_io		= s3c6400_map_io,
		.init_clocks	= s3c6400_init_clocks,
		.init_uarts	= s3c6400_init_uarts,
		.init		= s3c6400_init,
		.name		= name_s3c6400,
	}, {
		.idcode		= 0x36410100,
		.idmask		= 0xffffff00,
		.map_io		= s3c6410_map_io,
		.init_clocks	= s3c6410_init_clocks,
		.init_uarts	= s3c6410_init_uarts,
		.init		= s3c6410_init,
		.name		= name_s3c6410,
	},
};




#define UART_OFFS (S3C_PA_UART & 0xfffff)

static struct map_desc s3c_iodesc[] __initdata = {
	{
		.virtual	= (unsigned long)S3C_VA_SYS,
		.pfn		= __phys_to_pfn(S3C64XX_PA_SYSCON),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (unsigned long)(S3C_VA_UART + UART_OFFS),
		.pfn		= __phys_to_pfn(S3C_PA_UART),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (unsigned long)S3C_VA_VIC0,
		.pfn		= __phys_to_pfn(S3C64XX_PA_VIC0),
		.length		= SZ_16K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (unsigned long)S3C_VA_VIC1,
		.pfn		= __phys_to_pfn(S3C64XX_PA_VIC1),
		.length		= SZ_16K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (unsigned long)S3C_VA_TIMER,
		.pfn		= __phys_to_pfn(S3C_PA_TIMER),
		.length		= SZ_16K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (unsigned long)S3C64XX_VA_GPIO,
		.pfn		= __phys_to_pfn(S3C64XX_PA_GPIO),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (unsigned long)S3C64XX_VA_MODEM,
		.pfn		= __phys_to_pfn(S3C64XX_PA_MODEM),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= (unsigned long)S3C_VA_WATCHDOG,
		.pfn		= __phys_to_pfn(S3C64XX_PA_WATCHDOG),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	},
};


struct sysdev_class s3c64xx_sysclass = {
	.name	= "s3c64xx-core",
};

static struct sys_device s3c64xx_sysdev = {
	.cls	= &s3c64xx_sysclass,
};




void __init s3c64xx_init_io(struct map_desc *mach_desc, int size)
{
	unsigned long idcode;

	
	iotable_init(s3c_iodesc, ARRAY_SIZE(s3c_iodesc));
	iotable_init(mach_desc, size);

	idcode = __raw_readl(S3C_VA_SYS + 0x118);
	if (!idcode) {
		

		__raw_writel(0x0, S3C_VA_SYS + 0xA1C);
		idcode = __raw_readl(S3C_VA_SYS + 0xA1C);
	}

	s3c_init_cpu(idcode, cpu_ids, ARRAY_SIZE(cpu_ids));
}

static __init int s3c64xx_sysdev_init(void)
{
	sysdev_class_register(&s3c64xx_sysclass);
	return sysdev_register(&s3c64xx_sysdev);
}

core_initcall(s3c64xx_sysdev_init);
