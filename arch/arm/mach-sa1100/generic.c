
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/cpufreq.h>
#include <linux/ioport.h>
#include <linux/sched.h>	
#include <linux/platform_device.h>
#include <linux/cnt32_to_63.h>

#include <asm/div64.h>
#include <mach/hardware.h>
#include <asm/system.h>
#include <asm/pgtable.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>
#include <asm/irq.h>
#include <asm/gpio.h>

#include "generic.h"

unsigned int reset_status;
EXPORT_SYMBOL(reset_status);

#define NR_FREQS	16


static const unsigned short cclk_frequency_100khz[NR_FREQS] = {
	 590,	
	 737,	
	 885,	
	1032,	
	1180,	
	1327,	
	1475,	
	1622,	
	1769,	
	1917,	
	2064,	
	2212,	
	2359,	
	2507,	
	2654,	
	2802	
};

#if defined(CONFIG_CPU_FREQ_SA1100) || defined(CONFIG_CPU_FREQ_SA1110)

unsigned int sa11x0_freq_to_ppcr(unsigned int khz)
{
	int i;

	khz /= 100;

	for (i = 0; i < NR_FREQS; i++)
		if (cclk_frequency_100khz[i] >= khz)
			break;

	return i;
}

unsigned int sa11x0_ppcr_to_freq(unsigned int idx)
{
	unsigned int freq = 0;
	if (idx < NR_FREQS)
		freq = cclk_frequency_100khz[idx] * 100;
	return freq;
}



int sa11x0_verify_speed(struct cpufreq_policy *policy)
{
	unsigned int tmp;
	if (policy->cpu)
		return -EINVAL;

	cpufreq_verify_within_limits(policy, policy->cpuinfo.min_freq, policy->cpuinfo.max_freq);

	
	tmp = cclk_frequency_100khz[sa11x0_freq_to_ppcr(policy->min)] * 100;
	if (tmp > policy->max)
		policy->max = tmp;

	cpufreq_verify_within_limits(policy, policy->cpuinfo.min_freq, policy->cpuinfo.max_freq);

	return 0;
}

unsigned int sa11x0_getspeed(unsigned int cpu)
{
	if (cpu)
		return 0;
	return cclk_frequency_100khz[PPCR & 0xf] * 100;
}

#else

unsigned int cpufreq_get(unsigned int cpu)
{
	return cclk_frequency_100khz[PPCR & 0xf] * 100;
}
EXPORT_SYMBOL(cpufreq_get);
#endif


unsigned long long sched_clock(void)
{
	unsigned long long v = cnt32_to_63(OSCR);

	
	v *= 78125<<1;
	do_div(v, 288<<1);

	return v;
}


static void sa1100_power_off(void)
{
	mdelay(100);
	local_irq_disable();
	
	PCFR = (PCFR_OPDE | PCFR_FP | PCFR_FS);
	
	PWER = GFER = GRER = 1;
	
	PSPR = 0;
	
	PMCR = PMCR_SF;
}

static struct resource sa11x0udc_resources[] = {
	[0] = {
		.start	= 0x80000000,
		.end	= 0x8000ffff,
		.flags	= IORESOURCE_MEM,
	},
};

static u64 sa11x0udc_dma_mask = 0xffffffffUL;

static struct platform_device sa11x0udc_device = {
	.name		= "sa11x0-udc",
	.id		= -1,
	.dev		= {
		.dma_mask = &sa11x0udc_dma_mask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(sa11x0udc_resources),
	.resource	= sa11x0udc_resources,
};

static struct resource sa11x0uart1_resources[] = {
	[0] = {
		.start	= 0x80010000,
		.end	= 0x8001ffff,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device sa11x0uart1_device = {
	.name		= "sa11x0-uart",
	.id		= 1,
	.num_resources	= ARRAY_SIZE(sa11x0uart1_resources),
	.resource	= sa11x0uart1_resources,
};

static struct resource sa11x0uart3_resources[] = {
	[0] = {
		.start	= 0x80050000,
		.end	= 0x8005ffff,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device sa11x0uart3_device = {
	.name		= "sa11x0-uart",
	.id		= 3,
	.num_resources	= ARRAY_SIZE(sa11x0uart3_resources),
	.resource	= sa11x0uart3_resources,
};

static struct resource sa11x0mcp_resources[] = {
	[0] = {
		.start	= 0x80060000,
		.end	= 0x8006ffff,
		.flags	= IORESOURCE_MEM,
	},
};

static u64 sa11x0mcp_dma_mask = 0xffffffffUL;

static struct platform_device sa11x0mcp_device = {
	.name		= "sa11x0-mcp",
	.id		= -1,
	.dev = {
		.dma_mask = &sa11x0mcp_dma_mask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(sa11x0mcp_resources),
	.resource	= sa11x0mcp_resources,
};

void sa11x0_set_mcp_data(struct mcp_plat_data *data)
{
	sa11x0mcp_device.dev.platform_data = data;
}

static struct resource sa11x0ssp_resources[] = {
	[0] = {
		.start	= 0x80070000,
		.end	= 0x8007ffff,
		.flags	= IORESOURCE_MEM,
	},
};

static u64 sa11x0ssp_dma_mask = 0xffffffffUL;

static struct platform_device sa11x0ssp_device = {
	.name		= "sa11x0-ssp",
	.id		= -1,
	.dev = {
		.dma_mask = &sa11x0ssp_dma_mask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(sa11x0ssp_resources),
	.resource	= sa11x0ssp_resources,
};

static struct resource sa11x0fb_resources[] = {
	[0] = {
		.start	= 0xb0100000,
		.end	= 0xb010ffff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_LCD,
		.end	= IRQ_LCD,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device sa11x0fb_device = {
	.name		= "sa11x0-fb",
	.id		= -1,
	.dev = {
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(sa11x0fb_resources),
	.resource	= sa11x0fb_resources,
};

static struct platform_device sa11x0pcmcia_device = {
	.name		= "sa11x0-pcmcia",
	.id		= -1,
};

static struct platform_device sa11x0mtd_device = {
	.name		= "sa1100-mtd",
	.id		= -1,
};

void sa11x0_set_flash_data(struct flash_platform_data *flash,
			   struct resource *res, int nr)
{
	flash->name = "sa1100";
	sa11x0mtd_device.dev.platform_data = flash;
	sa11x0mtd_device.resource = res;
	sa11x0mtd_device.num_resources = nr;
}

static struct resource sa11x0ir_resources[] = {
	{
		.start	= __PREG(Ser2UTCR0),
		.end	= __PREG(Ser2UTCR0) + 0x24 - 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= __PREG(Ser2HSCR0),
		.end	= __PREG(Ser2HSCR0) + 0x1c - 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= __PREG(Ser2HSCR2),
		.end	= __PREG(Ser2HSCR2) + 0x04 - 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= IRQ_Ser2ICP,
		.end	= IRQ_Ser2ICP,
		.flags	= IORESOURCE_IRQ,
	}
};

static struct platform_device sa11x0ir_device = {
	.name		= "sa11x0-ir",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(sa11x0ir_resources),
	.resource	= sa11x0ir_resources,
};

void sa11x0_set_irda_data(struct irda_platform_data *irda)
{
	sa11x0ir_device.dev.platform_data = irda;
}

static struct platform_device sa11x0rtc_device = {
	.name		= "sa1100-rtc",
	.id		= -1,
};

static struct platform_device *sa11x0_devices[] __initdata = {
	&sa11x0udc_device,
	&sa11x0uart1_device,
	&sa11x0uart3_device,
	&sa11x0mcp_device,
	&sa11x0ssp_device,
	&sa11x0pcmcia_device,
	&sa11x0fb_device,
	&sa11x0mtd_device,
	&sa11x0rtc_device,
};

static int __init sa1100_init(void)
{
	pm_power_off = sa1100_power_off;

	if (sa11x0ir_device.dev.platform_data)
		platform_device_register(&sa11x0ir_device);

	return platform_add_devices(sa11x0_devices, ARRAY_SIZE(sa11x0_devices));
}

arch_initcall(sa1100_init);

void (*sa1100fb_backlight_power)(int on);
void (*sa1100fb_lcd_power)(int on);

EXPORT_SYMBOL(sa1100fb_backlight_power);
EXPORT_SYMBOL(sa1100fb_lcd_power);




static struct map_desc standard_io_desc[] __initdata = {
	{	
		.virtual	=  0xf8000000,
		.pfn		= __phys_to_pfn(0x80000000),
		.length		= 0x00100000,
		.type		= MT_DEVICE
	}, {	
		.virtual	=  0xfa000000,
		.pfn		= __phys_to_pfn(0x90000000),
		.length		= 0x00100000,
		.type		= MT_DEVICE
	}, {	
		.virtual	=  0xfc000000,
		.pfn		= __phys_to_pfn(0xa0000000),
		.length		= 0x00100000,
		.type		= MT_DEVICE
	}, {	
		.virtual	=  0xfe000000,
		.pfn		= __phys_to_pfn(0xb0000000),
		.length		= 0x00200000,
		.type		= MT_DEVICE
	},
};

void __init sa1100_map_io(void)
{
	iotable_init(standard_io_desc, ARRAY_SIZE(standard_io_desc));
}


void __init sa1110_mb_disable(void)
{
	unsigned long flags;

	local_irq_save(flags);
	
	PGSR &= ~GPIO_MBGNT;
	GPCR = GPIO_MBGNT;
	GPDR = (GPDR & ~GPIO_MBREQ) | GPIO_MBGNT;

	GAFR &= ~(GPIO_MBGNT | GPIO_MBREQ);

	local_irq_restore(flags);
}


void __devinit sa1110_mb_enable(void)
{
	unsigned long flags;

	local_irq_save(flags);

	PGSR &= ~GPIO_MBGNT;
	GPCR = GPIO_MBGNT;
	GPDR = (GPDR & ~GPIO_MBREQ) | GPIO_MBGNT;

	GAFR |= (GPIO_MBGNT | GPIO_MBREQ);
	TUCR |= TUCR_MR;

	local_irq_restore(flags);
}

