

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/tty.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <video/s1d13xxxfb.h>

#include <mach/hardware.h>
#include <asm/hardware/sa1111.h>
#include <asm/irq.h>
#include <asm/mach-types.h>
#include <asm/setup.h>
#include <asm/mach/arch.h>
#include <asm/mach/flash.h>
#include <asm/mach/map.h>
#include <asm/mach/serial_sa1100.h>

#include "generic.h"




#define TUCR_VAL	0x20000400


#define SA1111REGSTART	0x40000000
#define SA1111REGLEN	0x00001fff
#define EPSONREGSTART	0x48000000
#define EPSONREGLEN	0x00100000
#define EPSONFBSTART	0x48200000

#define EPSONFBLEN	512*1024

static struct s1d13xxxfb_regval s1d13xxxfb_initregs[] = {
	
	{0x0001,0x00},	
	{0x01FC,0x00},	
	{0x0004,0x00},	
	{0x0005,0x00},	
	{0x0008,0x00},	
	{0x0009,0x00},	
	{0x0010,0x01},	
	{0x0014,0x11},	
	{0x0018,0x01},	
	{0x001C,0x01},	
	{0x001E,0x01},	
	{0x0020,0x00},	
	{0x0021,0x45},	
	{0x002A,0x01},	
	{0x002B,0x03},	
	{0x0030,0x1c},	
	{0x0031,0x00},	
	{0x0032,0x4F},	
	{0x0034,0x07},	
	{0x0035,0x01},	
	{0x0036,0x0B},	
	{0x0038,0xEF},	
	{0x0039,0x00},	
	{0x003A,0x13},	
	{0x003B,0x0B},	
	{0x003C,0x01},	
	{0x0040,0x05},	
	{0x0041,0x00},	
	{0x0042,0x00},	
	{0x0043,0x00},	
	{0x0044,0x00},	
	{0x0046,0x80},	
	{0x0047,0x02},	
	{0x0048,0x00},	
	{0x004A,0x00},	
	{0x004B,0x00},	
	{0x0050,0x4F},	
	{0x0052,0x13},	
	{0x0053,0x01},	
	{0x0054,0x0B},	
	{0x0056,0xDF},	
	{0x0057,0x01},	
	{0x0058,0x2B},	
	{0x0059,0x09},	
	{0x005A,0x01},	
	{0x005B,0x10},	
	{0x0060,0x03},	
	{0x0062,0x00},	
	{0x0063,0x00},	
	{0x0064,0x00},	
	{0x0066,0x40},	
	{0x0067,0x01},	
	{0x0068,0x00},	
	{0x006A,0x00},	
	{0x006B,0x00},	
	{0x0070,0x00},	
	{0x0071,0x01},	
	{0x0072,0x00},	
	{0x0073,0x00},	
	{0x0074,0x00},	
	{0x0075,0x00},	
	{0x0076,0x00},	
	{0x0077,0x00},	
	{0x0078,0x00},	
	{0x007A,0x1F},	
	{0x007B,0x3F},	
	{0x007C,0x1F},	
	{0x007E,0x00},	
	{0x0080,0x00},	
	{0x0081,0x01},	
	{0x0082,0x00},	
	{0x0083,0x00},	
	{0x0084,0x00},	
	{0x0085,0x00},	
	{0x0086,0x00},	
	{0x0087,0x00},	
	{0x0088,0x00},	
	{0x008A,0x1F},	
	{0x008B,0x3F},	
	{0x008C,0x1F},	
	{0x008E,0x00},	
	{0x0100,0x00},	
	{0x0101,0x00},	
	{0x0102,0x00},	
	{0x0103,0x00},	
	{0x0104,0x00},	
	{0x0105,0x00},	
	{0x0106,0x00},	
	{0x0108,0x00},	
	{0x0109,0x00},	
	{0x010A,0x00},	
	{0x010C,0x00},	
	{0x010D,0x00},	
	{0x0110,0x00},	
	{0x0111,0x00},	
	{0x0112,0x00},	
	{0x0113,0x00},	
	{0x0114,0x00},	
	{0x0115,0x00},	
	{0x0118,0x00},	
	{0x0119,0x00},	
	{0x01E0,0x00},	
	{0x01E2,0x00},	
	
	{0x01E4,0x00},	
	
	{0x01F0,0x10},	
	{0x01F1,0x00},	
	{0x01F4,0x00},	
	{0x01FC,0x01},	
};

static struct s1d13xxxfb_pdata s1d13xxxfb_data = {
	.initregs		= s1d13xxxfb_initregs,
	.initregssize		= ARRAY_SIZE(s1d13xxxfb_initregs),
	.platform_init_video	= NULL
};

static struct resource s1d13xxxfb_resources[] = {
	[0] = {
		.start	= EPSONFBSTART,
		.end	= EPSONFBSTART + EPSONFBLEN,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= EPSONREGSTART,
		.end	= EPSONREGSTART + EPSONREGLEN,
		.flags	= IORESOURCE_MEM,
	}
};

static struct platform_device s1d13xxxfb_device = {
	.name		= S1D_DEVICENAME,
	.id		= 0,
	.dev		= {
		.platform_data	= &s1d13xxxfb_data,
	},
	.num_resources	= ARRAY_SIZE(s1d13xxxfb_resources),
	.resource	= s1d13xxxfb_resources,
};

static struct resource sa1111_resources[] = {
	[0] = {
		.start		= SA1111REGSTART,
		.end		= SA1111REGSTART + SA1111REGLEN,
		.flags		= IORESOURCE_MEM,
	},
	[1] = {
		.start		= IRQ_GPIO1,
		.end		= IRQ_GPIO1,
		.flags		= IORESOURCE_IRQ,
	},
};

static u64 sa1111_dmamask = 0xffffffffUL;

static struct platform_device sa1111_device = {
	.name		= "sa1111",
	.id		= 0,
	.dev		= {
		.dma_mask = &sa1111_dmamask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(sa1111_resources),
	.resource	= sa1111_resources,
};

static struct platform_device jornada_ssp_device = {
	.name           = "jornada_ssp",
	.id             = -1,
};

static struct platform_device jornada_kbd_device = {
	.name		= "jornada720_kbd",
	.id		= -1,
};

static struct platform_device jornada_ts_device = {
	.name		= "jornada_ts",
	.id		= -1,
};

static struct platform_device *devices[] __initdata = {
	&sa1111_device,
	&jornada_ssp_device,
	&s1d13xxxfb_device,
	&jornada_kbd_device,
	&jornada_ts_device,
};

static int __init jornada720_init(void)
{
	int ret = -ENODEV;

	if (machine_is_jornada720()) {
		
		GPDR |= GPIO_GPIO20;	
		TUCR = TUCR_VAL;
		GPSR = GPIO_GPIO20;	
		udelay(1);
		GPCR = GPIO_GPIO20;	
		udelay(1);
		GPSR = GPIO_GPIO20;	
		udelay(20);		

		ret = platform_add_devices(devices, ARRAY_SIZE(devices));
	}

	return ret;
}

arch_initcall(jornada720_init);

static struct map_desc jornada720_io_desc[] __initdata = {
	{	
		.virtual	= 0xf0000000,
		.pfn		= __phys_to_pfn(EPSONREGSTART),
		.length		= EPSONREGLEN,
		.type		= MT_DEVICE
	}, {	
		.virtual	= 0xf1000000,
		.pfn		= __phys_to_pfn(EPSONFBSTART),
		.length		= EPSONFBLEN,
		.type		= MT_DEVICE
	}, {	
		.virtual	= 0xf4000000,
		.pfn		= __phys_to_pfn(SA1111REGSTART),
		.length		= SA1111REGLEN,
		.type		= MT_DEVICE
	}
};

static void __init jornada720_map_io(void)
{
	sa1100_map_io();
	iotable_init(jornada720_io_desc, ARRAY_SIZE(jornada720_io_desc));

	sa1100_register_uart(0, 3);
	sa1100_register_uart(1, 1);
}

static struct mtd_partition jornada720_partitions[] = {
	{
		.name		= "JORNADA720 boot firmware",
		.size		= 0x00040000,
		.offset		= 0,
		.mask_flags	= MTD_WRITEABLE, 
	}, {
		.name		= "JORNADA720 kernel",
		.size		= 0x000c0000,
		.offset		= 0x00040000,
	}, {
		.name		= "JORNADA720 params",
		.size		= 0x00040000,
		.offset		= 0x00100000,
	}, {
		.name		= "JORNADA720 initrd",
		.size		= 0x00100000,
		.offset		= 0x00140000,
	}, {
		.name		= "JORNADA720 root cramfs",
		.size		= 0x00300000,
		.offset		= 0x00240000,
	}, {
		.name		= "JORNADA720 usr cramfs",
		.size		= 0x00800000,
		.offset		= 0x00540000,
	}, {
		.name		= "JORNADA720 usr local",
		.size		= 0, 
		.offset		= 0x00d00000,
	}
};

static void jornada720_set_vpp(int vpp)
{
	if (vpp)
		
		PPSR |= PPC_LDD7;
	else
		
		PPSR &= ~PPC_LDD7;
	PPDR |= PPC_LDD7;
}

static struct flash_platform_data jornada720_flash_data = {
	.map_name	= "cfi_probe",
	.set_vpp	= jornada720_set_vpp,
	.parts		= jornada720_partitions,
	.nr_parts	= ARRAY_SIZE(jornada720_partitions),
};

static struct resource jornada720_flash_resource = {
	.start		= SA1100_CS0_PHYS,
	.end		= SA1100_CS0_PHYS + SZ_32M - 1,
	.flags		= IORESOURCE_MEM,
};

static void __init jornada720_mach_init(void)
{
	sa11x0_set_flash_data(&jornada720_flash_data, &jornada720_flash_resource, 1);
}

MACHINE_START(JORNADA720, "HP Jornada 720")
	
	.phys_io	= 0x80000000,
	.io_pg_offst	= ((0xf8000000) >> 18) & 0xfffc,
	.boot_params	= 0xc0000100,
	.map_io		= jornada720_map_io,
	.init_irq	= sa1100_init_irq,
	.timer		= &sa1100_timer,
	.init_machine	= jornada720_mach_init,
MACHINE_END
