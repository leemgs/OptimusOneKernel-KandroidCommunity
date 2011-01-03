

#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/smc91x.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>

#include <plat/i2c.h>

#include <mach/hardware.h>
#include <mach/pxa2xx-regs.h>
#include <mach/mfp-pxa25x.h>

#include "generic.h"

#define XCEP_ETH_PHYS		(PXA_CS3_PHYS + 0x00000300)
#define XCEP_ETH_PHYS_END	(PXA_CS3_PHYS + 0x000fffff)
#define XCEP_ETH_ATTR		(PXA_CS3_PHYS + 0x02000000)
#define XCEP_ETH_ATTR_END	(PXA_CS3_PHYS + 0x020fffff)
#define XCEP_ETH_IRQ		IRQ_GPIO0


#define XCEP_CPLD_BASE		0xf0000000




static struct mtd_partition xcep_partitions[] = {
	{
		.name =		"Bootloader",
		.size =		0x00040000,
		.offset =	0,
		.mask_flags =	MTD_WRITEABLE
	}, {
		.name =		"Bootloader ENV",
		.size =		0x00040000,
		.offset =	0x00040000,
		.mask_flags =	MTD_WRITEABLE
	}, {
		.name =		"Kernel",
		.size =		0x00100000,
		.offset =	0x00080000,
	}, {
		.name =		"Rescue fs",
		.size =		0x00280000,
		.offset =	0x00180000,
	}, {
		.name =		"Filesystem",
		.size =		MTDPART_SIZ_FULL,
		.offset =	0x00400000
	}
};

static struct physmap_flash_data xcep_flash_data[] = {
	{
		.width		= 4,		
		.parts		= xcep_partitions,
		.nr_parts	= ARRAY_SIZE(xcep_partitions)
	}
};

static struct resource flash_resource = {
	.start	= PXA_CS0_PHYS,
	.end	= PXA_CS0_PHYS + SZ_32M - 1,
	.flags	= IORESOURCE_MEM,
};

static struct platform_device flash_device = {
	.name	= "physmap-flash",
	.id	= 0,
	.dev 	= {
		.platform_data = xcep_flash_data,
	},
	.resource = &flash_resource,
	.num_resources = 1,
};





static struct resource smc91x_resources[] = {
	[0] = {
		.name	= "smc91x-regs",
		.start	= XCEP_ETH_PHYS,
		.end	= XCEP_ETH_PHYS_END,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= XCEP_ETH_IRQ,
		.end	= XCEP_ETH_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.name	= "smc91x-attrib",
		.start	= XCEP_ETH_ATTR,
		.end	= XCEP_ETH_ATTR_END,
		.flags	= IORESOURCE_MEM,
	},
};

static struct smc91x_platdata xcep_smc91x_info = {
	.flags	= SMC91X_USE_32BIT | SMC91X_NOWAIT | SMC91X_USE_DMA,
};

static struct platform_device smc91x_device = {
	.name		= "smc91x",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(smc91x_resources),
	.resource	= smc91x_resources,
	.dev		= {
		.platform_data = &xcep_smc91x_info,
	},
};


static struct platform_device *devices[] __initdata = {
	&flash_device,
	&smc91x_device,
};



static struct i2c_pxa_platform_data xcep_i2c_platform_data  = {
	.class = I2C_CLASS_HWMON
};


static mfp_cfg_t xcep_pin_config[] __initdata = {
	GPIO79_nCS_3,	
	GPIO80_nCS_4,	
	
	GPIO23_SSP1_SCLK,
	GPIO24_SSP1_SFRM,
	GPIO25_SSP1_TXD,
	GPIO26_SSP1_RXD,
	GPIO27_SSP1_EXTCLK
};

static void __init xcep_init(void)
{
	pxa2xx_mfp_config(ARRAY_AND_SIZE(xcep_pin_config));

	
	
	MSC1 = (MSC1 & 0xffff) | 0xD5540000;
	
	MSC2 = (MSC2 & 0xffff) | 0x72A00000;

	platform_add_devices(ARRAY_AND_SIZE(devices));
	pxa_set_i2c_info(&xcep_i2c_platform_data);
}

MACHINE_START(XCEP, "Iskratel XCEP")
	.phys_io	= 0x40000000,
	.io_pg_offst	= (io_p2v(0x40000000) >> 18) & 0xfffc,
	.boot_params	= 0xa0000100,
	.init_machine	= xcep_init,
	.map_io		= pxa_map_io,
	.init_irq	= pxa25x_init_irq,
	.timer		= &pxa_timer,
MACHINE_END

