
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/bitops.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/termios.h>
#include <linux/amba/bus.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>

#include <asm/types.h>
#include <asm/setup.h>
#include <asm/memory.h>
#include <asm/hardware/vic.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <mach/syscon.h>

#include "clock.h"
#include "mmc.h"
#include "spi.h"
#include "i2c.h"


static struct map_desc u300_io_desc[] __initdata = {
	{
		.virtual	= U300_SLOW_PER_VIRT_BASE,
		.pfn		= __phys_to_pfn(U300_SLOW_PER_PHYS_BASE),
		.length		= SZ_64K,
		.type		= MT_DEVICE,
	},
	{
		.virtual	= U300_AHB_PER_VIRT_BASE,
		.pfn		= __phys_to_pfn(U300_AHB_PER_PHYS_BASE),
		.length		= SZ_32K,
		.type		= MT_DEVICE,
	},
	{
		.virtual	= U300_FAST_PER_VIRT_BASE,
		.pfn		= __phys_to_pfn(U300_FAST_PER_PHYS_BASE),
		.length		= SZ_32K,
		.type		= MT_DEVICE,
	},
	{
		.virtual	= 0xffff2000, 
		.pfn		= __phys_to_pfn(0xffff2000),
		.length		= SZ_16K,
		.type		= MT_DEVICE,
	},

	
	
};

void __init u300_map_io(void)
{
	iotable_init(u300_io_desc, ARRAY_SIZE(u300_io_desc));
}


static struct amba_device uart0_device = {
	.dev = {
		.init_name = "uart0", 
		.platform_data = NULL,
	},
	.res = {
		.start = U300_UART0_BASE,
		.end   = U300_UART0_BASE + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	.irq = { IRQ_U300_UART0, NO_IRQ },
};


#ifdef CONFIG_MACH_U300_BS335
static struct amba_device uart1_device = {
	.dev = {
		.init_name = "uart1", 
		.platform_data = NULL,
	},
	.res = {
		.start = U300_UART1_BASE,
		.end   = U300_UART1_BASE + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	.irq = { IRQ_U300_UART1, NO_IRQ },
};
#endif

static struct amba_device pl172_device = {
	.dev = {
		.init_name = "pl172", 
		.platform_data = NULL,
	},
	.res = {
		.start = U300_EMIF_CFG_BASE,
		.end   = U300_EMIF_CFG_BASE + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
};



static struct amba_device pl022_device = {
	.dev = {
		.coherent_dma_mask = ~0,
		.init_name = "pl022", 
	},
	.res = {
		.start = U300_SPI_BASE,
		.end   = U300_SPI_BASE + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	.irq = {IRQ_U300_SPI, NO_IRQ },
	
};

static struct amba_device mmcsd_device = {
	.dev = {
		.init_name = "mmci", 
		.platform_data = NULL, 
	},
	.res = {
		.start = U300_MMCSD_BASE,
		.end   = U300_MMCSD_BASE + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	.irq = {IRQ_U300_MMCSD_MCIINTR0, IRQ_U300_MMCSD_MCIINTR1 },
	
};


static struct amba_device *amba_devs[] __initdata = {
	&uart0_device,
#ifdef CONFIG_MACH_U300_BS335
	&uart1_device,
#endif
	&pl022_device,
	&pl172_device,
	&mmcsd_device,
};



static struct resource gpio_resources[] = {
	{
		.start = U300_GPIO_BASE,
		.end   = (U300_GPIO_BASE + SZ_4K - 1),
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "gpio0",
		.start = IRQ_U300_GPIO_PORT0,
		.end   = IRQ_U300_GPIO_PORT0,
		.flags = IORESOURCE_IRQ,
	},
	{
		.name  = "gpio1",
		.start = IRQ_U300_GPIO_PORT1,
		.end   = IRQ_U300_GPIO_PORT1,
		.flags = IORESOURCE_IRQ,
	},
	{
		.name  = "gpio2",
		.start = IRQ_U300_GPIO_PORT2,
		.end   = IRQ_U300_GPIO_PORT2,
		.flags = IORESOURCE_IRQ,
	},
#ifdef U300_COH901571_3
	{
		.name  = "gpio3",
		.start = IRQ_U300_GPIO_PORT3,
		.end   = IRQ_U300_GPIO_PORT3,
		.flags = IORESOURCE_IRQ,
	},
	{
		.name  = "gpio4",
		.start = IRQ_U300_GPIO_PORT4,
		.end   = IRQ_U300_GPIO_PORT4,
		.flags = IORESOURCE_IRQ,
	},
#ifdef CONFIG_MACH_U300_BS335
	{
		.name  = "gpio5",
		.start = IRQ_U300_GPIO_PORT5,
		.end   = IRQ_U300_GPIO_PORT5,
		.flags = IORESOURCE_IRQ,
	},
	{
		.name  = "gpio6",
		.start = IRQ_U300_GPIO_PORT6,
		.end   = IRQ_U300_GPIO_PORT6,
		.flags = IORESOURCE_IRQ,
	},
#endif 
#endif 
};

static struct resource keypad_resources[] = {
	{
		.start = U300_KEYPAD_BASE,
		.end   = U300_KEYPAD_BASE + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "coh901461-press",
		.start = IRQ_U300_KEYPAD_KEYBF,
		.end   = IRQ_U300_KEYPAD_KEYBF,
		.flags = IORESOURCE_IRQ,
	},
	{
		.name  = "coh901461-release",
		.start = IRQ_U300_KEYPAD_KEYBR,
		.end   = IRQ_U300_KEYPAD_KEYBR,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource rtc_resources[] = {
	{
		.start = U300_RTC_BASE,
		.end   = U300_RTC_BASE + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = IRQ_U300_RTC,
		.end   = IRQ_U300_RTC,
		.flags = IORESOURCE_IRQ,
	},
};


static struct resource fsmc_resources[] = {
	{
		.start = U300_NAND_IF_PHYS_BASE,
		.end   = U300_NAND_IF_PHYS_BASE + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
};

static struct resource i2c0_resources[] = {
	{
		.start = U300_I2C0_BASE,
		.end   = U300_I2C0_BASE + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = IRQ_U300_I2C0,
		.end   = IRQ_U300_I2C0,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource i2c1_resources[] = {
	{
		.start = U300_I2C1_BASE,
		.end   = U300_I2C1_BASE + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = IRQ_U300_I2C1,
		.end   = IRQ_U300_I2C1,
		.flags = IORESOURCE_IRQ,
	},

};

static struct resource wdog_resources[] = {
	{
		.start = U300_WDOG_BASE,
		.end   = U300_WDOG_BASE + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = IRQ_U300_WDOG,
		.end   = IRQ_U300_WDOG,
		.flags = IORESOURCE_IRQ,
	}
};


static struct resource ave_resources[] = {
	{
		.name  = "AVE3e I/O Area",
		.start = U300_VIDEOENC_BASE,
		.end   = U300_VIDEOENC_BASE + SZ_512K - 1,
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "AVE3e IRQ0",
		.start = IRQ_U300_VIDEO_ENC_0,
		.end   = IRQ_U300_VIDEO_ENC_0,
		.flags = IORESOURCE_IRQ,
	},
	{
		.name  = "AVE3e IRQ1",
		.start = IRQ_U300_VIDEO_ENC_1,
		.end   = IRQ_U300_VIDEO_ENC_1,
		.flags = IORESOURCE_IRQ,
	},
	{
		.name  = "AVE3e Physmem Area",
		.start = 0, 
		.end   = SZ_1M - 1,
		.flags = IORESOURCE_MEM,
	},
	
	{
		.name  = "AVE3e Reserved 0",
		.start = 0xd0000000,
		.end   = 0xd0000000 + SZ_256M - 1,
		.flags = IORESOURCE_MEM,
	},
	{
		.name  = "AVE3e Reserved 1",
		.start = 0xe0000000,
		.end   = 0xe0000000 + SZ_256M - 1,
		.flags = IORESOURCE_MEM,
	},
};

static struct platform_device wdog_device = {
	.name = "wdog",
	.id = -1,
	.num_resources = ARRAY_SIZE(wdog_resources),
	.resource = wdog_resources,
};

static struct platform_device i2c0_device = {
	.name = "stu300",
	.id = 0,
	.num_resources = ARRAY_SIZE(i2c0_resources),
	.resource = i2c0_resources,
};

static struct platform_device i2c1_device = {
	.name = "stu300",
	.id = 1,
	.num_resources = ARRAY_SIZE(i2c1_resources),
	.resource = i2c1_resources,
};

static struct platform_device gpio_device = {
	.name = "u300-gpio",
	.id = -1,
	.num_resources = ARRAY_SIZE(gpio_resources),
	.resource = gpio_resources,
};

static struct platform_device keypad_device = {
	.name = "keypad",
	.id = -1,
	.num_resources = ARRAY_SIZE(keypad_resources),
	.resource = keypad_resources,
};

static struct platform_device rtc_device = {
	.name = "rtc-coh901331",
	.id = -1,
	.num_resources = ARRAY_SIZE(rtc_resources),
	.resource = rtc_resources,
};

static struct platform_device fsmc_device = {
	.name = "nandif",
	.id = -1,
	.num_resources = ARRAY_SIZE(fsmc_resources),
	.resource = fsmc_resources,
};

static struct platform_device ave_device = {
	.name = "video_enc",
	.id = -1,
	.num_resources = ARRAY_SIZE(ave_resources),
	.resource = ave_resources,
};


static struct platform_device *platform_devs[] __initdata = {
	&i2c0_device,
	&i2c1_device,
	&keypad_device,
	&rtc_device,
	&gpio_device,
	&fsmc_device,
	&wdog_device,
	&ave_device
};



void __init u300_init_irq(void)
{
	u32 mask[2] = {0, 0};
	int i;

	for (i = 0; i < NR_IRQS; i++)
		set_bit(i, (unsigned long *) &mask[0]);
	u300_enable_intcon_clock();
	vic_init((void __iomem *) U300_INTCON0_VBASE, 0, mask[0], mask[0]);
	vic_init((void __iomem *) U300_INTCON1_VBASE, 32, mask[1], mask[1]);
}



struct db_chip {
	u16 chipid;
	const char *name;
};


static struct db_chip db_chips[] __initdata = {
	{
		.chipid = 0xb800,
		.name = "DB3000",
	},
	{
		.chipid = 0xc000,
		.name = "DB3100",
	},
	{
		.chipid = 0xc800,
		.name = "DB3150",
	},
	{
		.chipid = 0xd800,
		.name = "DB3200",
	},
	{
		.chipid = 0xe000,
		.name = "DB3250",
	},
	{
		.chipid = 0xe800,
		.name = "DB3210",
	},
	{
		.chipid = 0xf000,
		.name = "DB3350 P1x",
	},
	{
		.chipid = 0xf100,
		.name = "DB3350 P2x",
	},
	{
		.chipid = 0x0000, 
		.name = NULL,
	}
};

static void __init u300_init_check_chip(void)
{

	u16 val;
	struct db_chip *chip;
	const char *chipname;
	const char unknown[] = "UNKNOWN";

	
	val = readw(U300_SYSCON_VBASE + U300_SYSCON_CIDR);
	
	val = (val & 0xFFU) << 8 | (val >> 8);
	chip = db_chips;
	chipname = unknown;

	for ( ; chip->chipid; chip++) {
		if (chip->chipid == (val & 0xFF00U)) {
			chipname = chip->name;
			break;
		}
	}
	printk(KERN_INFO "Initializing U300 system on %s baseband chip " \
	       "(chip ID 0x%04x)\n", chipname, val);

#ifdef CONFIG_MACH_U300_BS26
	if ((val & 0xFF00U) != 0xc800) {
		printk(KERN_ERR "Platform configured for BS25/BS26 " \
		       "with DB3150 but %s detected, expect problems!",
		       chipname);
	}
#endif
#ifdef CONFIG_MACH_U300_BS330
	if ((val & 0xFF00U) != 0xd800) {
		printk(KERN_ERR "Platform configured for BS330 " \
		       "with DB3200 but %s detected, expect problems!",
		       chipname);
	}
#endif
#ifdef CONFIG_MACH_U300_BS335
	if ((val & 0xFF00U) != 0xf000 && (val & 0xFF00U) != 0xf100) {
		printk(KERN_ERR "Platform configured for BS365 " \
		       " with DB3350 but %s detected, expect problems!",
		       chipname);
	}
#endif
#ifdef CONFIG_MACH_U300_BS365
	if ((val & 0xFF00U) != 0xe800) {
		printk(KERN_ERR "Platform configured for BS365 " \
		       "with DB3210 but %s detected, expect problems!",
		       chipname);
	}
#endif


}


static void __init u300_assign_physmem(void)
{
	unsigned long curr_start = __pa(high_memory);
	int i, j;

	for (i = 0; i < ARRAY_SIZE(platform_devs); i++) {
		for (j = 0; j < platform_devs[i]->num_resources; j++) {
			struct resource *const res =
			  &platform_devs[i]->resource[j];

			if (IORESOURCE_MEM == res->flags &&
				     0 == res->start) {
				res->start  = curr_start;
				res->end   += curr_start;
				curr_start += (res->end - res->start + 1);

				printk(KERN_INFO "core.c: Mapping RAM " \
				       "%#x-%#x to device %s:%s\n",
					res->start, res->end,
				       platform_devs[i]->name, res->name);
			}
		}
	}
}

void __init u300_init_devices(void)
{
	int i;
	u16 val;

	
	u300_init_check_chip();

	
	val = readw(U300_SYSCON_VBASE + U300_SYSCON_CCR);
	val &= ~U300_SYSCON_CCR_CLKING_PERFORMANCE_MASK;
	writew(val, U300_SYSCON_VBASE + U300_SYSCON_CCR);
	
	while (!(readw(U300_SYSCON_VBASE + U300_SYSCON_CSR) &
		 U300_SYSCON_CSR_PLL208_LOCK_IND));
	
	u300_spi_init(&pl022_device);

	
	u300_clock_primecells();
	for (i = 0; i < ARRAY_SIZE(amba_devs); i++) {
		struct amba_device *d = amba_devs[i];
		amba_device_register(d, &iomem_resource);
	}
	u300_unclock_primecells();

	u300_assign_physmem();

	
	u300_i2c_register_board_devices();

	
	u300_spi_register_board_devices();

	
	platform_add_devices(platform_devs, ARRAY_SIZE(platform_devs));

#ifndef CONFIG_MACH_U300_SEMI_IS_SHARED
	
	val = readw(U300_SYSCON_VBASE + U300_SYSCON_SMCR) |
		U300_SYSCON_SMCR_SEMI_SREFREQ_ENABLE;
	writew(val, U300_SYSCON_VBASE + U300_SYSCON_SMCR);
#endif 
}

static int core_module_init(void)
{
	
	return mmc_init(&mmcsd_device);
}
module_init(core_module_init);
