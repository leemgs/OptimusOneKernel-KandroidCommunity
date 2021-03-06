

#include <linux/module.h>
#include <linux/pm.h>

#include <asm/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <mach/cpu.h>
#include <mach/at91cap9.h>
#include <mach/at91_pmc.h>
#include <mach/at91_rstc.h>
#include <mach/at91_shdwc.h>

#include "generic.h"
#include "clock.h"

static struct map_desc at91cap9_io_desc[] __initdata = {
	{
		.virtual	= AT91_VA_BASE_SYS,
		.pfn		= __phys_to_pfn(AT91_BASE_SYS),
		.length		= SZ_16K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= AT91_IO_VIRT_BASE - AT91CAP9_SRAM_SIZE,
		.pfn		= __phys_to_pfn(AT91CAP9_SRAM_BASE),
		.length		= AT91CAP9_SRAM_SIZE,
		.type		= MT_DEVICE,
	},
};




static struct clk pioABCD_clk = {
	.name		= "pioABCD_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_PIOABCD,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk mpb0_clk = {
	.name		= "mpb0_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_MPB0,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk mpb1_clk = {
	.name		= "mpb1_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_MPB1,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk mpb2_clk = {
	.name		= "mpb2_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_MPB2,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk mpb3_clk = {
	.name		= "mpb3_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_MPB3,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk mpb4_clk = {
	.name		= "mpb4_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_MPB4,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk usart0_clk = {
	.name		= "usart0_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_US0,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk usart1_clk = {
	.name		= "usart1_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_US1,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk usart2_clk = {
	.name		= "usart2_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_US2,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk mmc0_clk = {
	.name		= "mci0_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_MCI0,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk mmc1_clk = {
	.name		= "mci1_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_MCI1,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk can_clk = {
	.name		= "can_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_CAN,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk twi_clk = {
	.name		= "twi_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_TWI,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk spi0_clk = {
	.name		= "spi0_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_SPI0,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk spi1_clk = {
	.name		= "spi1_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_SPI1,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk ssc0_clk = {
	.name		= "ssc0_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_SSC0,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk ssc1_clk = {
	.name		= "ssc1_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_SSC1,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk ac97_clk = {
	.name		= "ac97_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_AC97C,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk tcb_clk = {
	.name		= "tcb_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_TCB,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk pwm_clk = {
	.name		= "pwm_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_PWMC,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk macb_clk = {
	.name		= "macb_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_EMAC,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk aestdes_clk = {
	.name		= "aestdes_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_AESTDES,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk adc_clk = {
	.name		= "adc_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_ADC,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk isi_clk = {
	.name		= "isi_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_ISI,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk lcdc_clk = {
	.name		= "lcdc_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_LCDC,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk dma_clk = {
	.name		= "dma_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_DMA,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk udphs_clk = {
	.name		= "udphs_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_UDPHS,
	.type		= CLK_TYPE_PERIPHERAL,
};
static struct clk ohci_clk = {
	.name		= "ohci_clk",
	.pmc_mask	= 1 << AT91CAP9_ID_UHP,
	.type		= CLK_TYPE_PERIPHERAL,
};

static struct clk *periph_clocks[] __initdata = {
	&pioABCD_clk,
	&mpb0_clk,
	&mpb1_clk,
	&mpb2_clk,
	&mpb3_clk,
	&mpb4_clk,
	&usart0_clk,
	&usart1_clk,
	&usart2_clk,
	&mmc0_clk,
	&mmc1_clk,
	&can_clk,
	&twi_clk,
	&spi0_clk,
	&spi1_clk,
	&ssc0_clk,
	&ssc1_clk,
	&ac97_clk,
	&tcb_clk,
	&pwm_clk,
	&macb_clk,
	&aestdes_clk,
	&adc_clk,
	&isi_clk,
	&lcdc_clk,
	&dma_clk,
	&udphs_clk,
	&ohci_clk,
	
};


static struct clk pck0 = {
	.name		= "pck0",
	.pmc_mask	= AT91_PMC_PCK0,
	.type		= CLK_TYPE_PROGRAMMABLE,
	.id		= 0,
};
static struct clk pck1 = {
	.name		= "pck1",
	.pmc_mask	= AT91_PMC_PCK1,
	.type		= CLK_TYPE_PROGRAMMABLE,
	.id		= 1,
};
static struct clk pck2 = {
	.name		= "pck2",
	.pmc_mask	= AT91_PMC_PCK2,
	.type		= CLK_TYPE_PROGRAMMABLE,
	.id		= 2,
};
static struct clk pck3 = {
	.name		= "pck3",
	.pmc_mask	= AT91_PMC_PCK3,
	.type		= CLK_TYPE_PROGRAMMABLE,
	.id		= 3,
};

static void __init at91cap9_register_clocks(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(periph_clocks); i++)
		clk_register(periph_clocks[i]);

	clk_register(&pck0);
	clk_register(&pck1);
	clk_register(&pck2);
	clk_register(&pck3);
}



static struct at91_gpio_bank at91cap9_gpio[] = {
	{
		.id		= AT91CAP9_ID_PIOABCD,
		.offset		= AT91_PIOA,
		.clock		= &pioABCD_clk,
	}, {
		.id		= AT91CAP9_ID_PIOABCD,
		.offset		= AT91_PIOB,
		.clock		= &pioABCD_clk,
	}, {
		.id		= AT91CAP9_ID_PIOABCD,
		.offset		= AT91_PIOC,
		.clock		= &pioABCD_clk,
	}, {
		.id		= AT91CAP9_ID_PIOABCD,
		.offset		= AT91_PIOD,
		.clock		= &pioABCD_clk,
	}
};

static void at91cap9_reset(void)
{
	at91_sys_write(AT91_RSTC_CR, AT91_RSTC_KEY | AT91_RSTC_PROCRST | AT91_RSTC_PERRST);
}

static void at91cap9_poweroff(void)
{
	at91_sys_write(AT91_SHDW_CR, AT91_SHDW_KEY | AT91_SHDW_SHDW);
}




void __init at91cap9_initialize(unsigned long main_clock)
{
	
	iotable_init(at91cap9_io_desc, ARRAY_SIZE(at91cap9_io_desc));

	at91_arch_reset = at91cap9_reset;
	pm_power_off = at91cap9_poweroff;
	at91_extern_irq = (1 << AT91CAP9_ID_IRQ0) | (1 << AT91CAP9_ID_IRQ1);

	
	at91_clock_init(main_clock);

	
	at91cap9_register_clocks();

	
	at91_gpio_init(at91cap9_gpio, 4);

	
	if (cpu_is_at91cap9_revB())
		system_rev = 0xB;
	else if (cpu_is_at91cap9_revC())
		system_rev = 0xC;
}




static unsigned int at91cap9_default_irq_priority[NR_AIC_IRQS] __initdata = {
	7,	
	7,	
	1,	
	0,	
	0,	
	0,	
	0,	
	0,	
	5,	
	5,	
	5,	
	0,	
	0,	
	3,	
	6,	
	5,	
	5,	
	4,	
	4,	
	5,	
	0,	
	0,	
	3,	
	0,	
	0,	
	0,	
	3,	
	0,	
	2,	
	2,	
	0,	
	0,	
};

void __init at91cap9_init_interrupts(unsigned int priority[NR_AIC_IRQS])
{
	if (!priority)
		priority = at91cap9_default_irq_priority;

	
	at91_aic_init(priority);

	
	at91_gpio_irq_setup();
}
