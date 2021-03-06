
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/i2c-gpio.h>

#include <video/atmel_lcdc.h>

#include <mach/board.h>
#include <mach/cpu.h>
#include <mach/gpio.h>
#include <mach/at91cap9.h>
#include <mach/at91cap9_matrix.h>
#include <mach/at91sam9_smc.h>

#include "generic.h"




#if defined(CONFIG_USB_OHCI_HCD) || defined(CONFIG_USB_OHCI_HCD_MODULE)
static u64 ohci_dmamask = DMA_BIT_MASK(32);
static struct at91_usbh_data usbh_data;

static struct resource usbh_resources[] = {
	[0] = {
		.start	= AT91CAP9_UHP_BASE,
		.end	= AT91CAP9_UHP_BASE + SZ_1M - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_ID_UHP,
		.end	= AT91CAP9_ID_UHP,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device at91_usbh_device = {
	.name		= "at91_ohci",
	.id		= -1,
	.dev		= {
				.dma_mask		= &ohci_dmamask,
				.coherent_dma_mask	= DMA_BIT_MASK(32),
				.platform_data		= &usbh_data,
	},
	.resource	= usbh_resources,
	.num_resources	= ARRAY_SIZE(usbh_resources),
};

void __init at91_add_device_usbh(struct at91_usbh_data *data)
{
	int i;

	if (!data)
		return;

	if (cpu_is_at91cap9_revB())
		set_irq_type(AT91CAP9_ID_UHP, IRQ_TYPE_LEVEL_HIGH);

	
	for (i = 0; i < data->ports; i++) {
		if (data->vbus_pin[i])
			at91_set_gpio_output(data->vbus_pin[i], 0);
	}

	usbh_data = *data;
	platform_device_register(&at91_usbh_device);
}
#else
void __init at91_add_device_usbh(struct at91_usbh_data *data) {}
#endif




#if defined(CONFIG_USB_GADGET_ATMEL_USBA) || defined(CONFIG_USB_GADGET_ATMEL_USBA_MODULE)

static struct resource usba_udc_resources[] = {
	[0] = {
		.start	= AT91CAP9_UDPHS_FIFO,
		.end	= AT91CAP9_UDPHS_FIFO + SZ_512K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_BASE_UDPHS,
		.end	= AT91CAP9_BASE_UDPHS + SZ_1K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start	= AT91CAP9_ID_UDPHS,
		.end	= AT91CAP9_ID_UDPHS,
		.flags	= IORESOURCE_IRQ,
	},
};

#define EP(nam, idx, maxpkt, maxbk, dma, isoc)			\
	[idx] = {						\
		.name		= nam,				\
		.index		= idx,				\
		.fifo_size	= maxpkt,			\
		.nr_banks	= maxbk,			\
		.can_dma	= dma,				\
		.can_isoc	= isoc,				\
	}

static struct usba_ep_data usba_udc_ep[] = {
	EP("ep0", 0,   64, 1, 0, 0),
	EP("ep1", 1, 1024, 3, 1, 1),
	EP("ep2", 2, 1024, 3, 1, 1),
	EP("ep3", 3, 1024, 2, 1, 1),
	EP("ep4", 4, 1024, 2, 1, 1),
	EP("ep5", 5, 1024, 2, 1, 0),
	EP("ep6", 6, 1024, 2, 1, 0),
	EP("ep7", 7, 1024, 2, 0, 0),
};

#undef EP


static struct {
	struct usba_platform_data pdata;
	struct usba_ep_data ep[8];
} usba_udc_data;

static struct platform_device at91_usba_udc_device = {
	.name		= "atmel_usba_udc",
	.id		= -1,
	.dev		= {
				.platform_data	= &usba_udc_data.pdata,
	},
	.resource	= usba_udc_resources,
	.num_resources	= ARRAY_SIZE(usba_udc_resources),
};

void __init at91_add_device_usba(struct usba_platform_data *data)
{
	if (cpu_is_at91cap9_revB()) {
		set_irq_type(AT91CAP9_ID_UDPHS, IRQ_TYPE_LEVEL_HIGH);
		at91_sys_write(AT91_MATRIX_UDPHS, AT91_MATRIX_SELECT_UDPHS |
						  AT91_MATRIX_UDPHS_BYPASS_LOCK);
	}
	else
		at91_sys_write(AT91_MATRIX_UDPHS, AT91_MATRIX_SELECT_UDPHS);

	
	usba_udc_data.pdata.vbus_pin = -EINVAL;
	usba_udc_data.pdata.num_ep = ARRAY_SIZE(usba_udc_ep);
	memcpy(usba_udc_data.ep, usba_udc_ep, sizeof(usba_udc_ep));;

	if (data && data->vbus_pin > 0) {
		at91_set_gpio_input(data->vbus_pin, 0);
		at91_set_deglitch(data->vbus_pin, 1);
		usba_udc_data.pdata.vbus_pin = data->vbus_pin;
	}

	

	
	at91_clock_associate("utmi_clk", &at91_usba_udc_device.dev, "hclk");
	at91_clock_associate("udphs_clk", &at91_usba_udc_device.dev, "pclk");

	platform_device_register(&at91_usba_udc_device);
}
#else
void __init at91_add_device_usba(struct usba_platform_data *data) {}
#endif




#if defined(CONFIG_MACB) || defined(CONFIG_MACB_MODULE)
static u64 eth_dmamask = DMA_BIT_MASK(32);
static struct at91_eth_data eth_data;

static struct resource eth_resources[] = {
	[0] = {
		.start	= AT91CAP9_BASE_EMAC,
		.end	= AT91CAP9_BASE_EMAC + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_ID_EMAC,
		.end	= AT91CAP9_ID_EMAC,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device at91cap9_eth_device = {
	.name		= "macb",
	.id		= -1,
	.dev		= {
				.dma_mask		= &eth_dmamask,
				.coherent_dma_mask	= DMA_BIT_MASK(32),
				.platform_data		= &eth_data,
	},
	.resource	= eth_resources,
	.num_resources	= ARRAY_SIZE(eth_resources),
};

void __init at91_add_device_eth(struct at91_eth_data *data)
{
	if (!data)
		return;

	if (data->phy_irq_pin) {
		at91_set_gpio_input(data->phy_irq_pin, 0);
		at91_set_deglitch(data->phy_irq_pin, 1);
	}

	
	at91_set_A_periph(AT91_PIN_PB21, 0);	
	at91_set_A_periph(AT91_PIN_PB22, 0);	
	at91_set_A_periph(AT91_PIN_PB25, 0);	
	at91_set_A_periph(AT91_PIN_PB26, 0);	
	at91_set_A_periph(AT91_PIN_PB27, 0);	
	at91_set_A_periph(AT91_PIN_PB28, 0);	
	at91_set_A_periph(AT91_PIN_PB23, 0);	
	at91_set_A_periph(AT91_PIN_PB24, 0);	
	at91_set_A_periph(AT91_PIN_PB30, 0);	
	at91_set_A_periph(AT91_PIN_PB29, 0);	

	if (!data->is_rmii) {
		at91_set_B_periph(AT91_PIN_PC25, 0);	
		at91_set_B_periph(AT91_PIN_PC26, 0);	
		at91_set_B_periph(AT91_PIN_PC22, 0);	
		at91_set_B_periph(AT91_PIN_PC23, 0);	
		at91_set_B_periph(AT91_PIN_PC27, 0);	
		at91_set_B_periph(AT91_PIN_PC20, 0);	
		at91_set_B_periph(AT91_PIN_PC21, 0);	
		at91_set_B_periph(AT91_PIN_PC24, 0);	
	}

	eth_data = *data;
	platform_device_register(&at91cap9_eth_device);
}
#else
void __init at91_add_device_eth(struct at91_eth_data *data) {}
#endif




#if defined(CONFIG_MMC_AT91) || defined(CONFIG_MMC_AT91_MODULE)
static u64 mmc_dmamask = DMA_BIT_MASK(32);
static struct at91_mmc_data mmc0_data, mmc1_data;

static struct resource mmc0_resources[] = {
	[0] = {
		.start	= AT91CAP9_BASE_MCI0,
		.end	= AT91CAP9_BASE_MCI0 + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_ID_MCI0,
		.end	= AT91CAP9_ID_MCI0,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device at91cap9_mmc0_device = {
	.name		= "at91_mci",
	.id		= 0,
	.dev		= {
				.dma_mask		= &mmc_dmamask,
				.coherent_dma_mask	= DMA_BIT_MASK(32),
				.platform_data		= &mmc0_data,
	},
	.resource	= mmc0_resources,
	.num_resources	= ARRAY_SIZE(mmc0_resources),
};

static struct resource mmc1_resources[] = {
	[0] = {
		.start	= AT91CAP9_BASE_MCI1,
		.end	= AT91CAP9_BASE_MCI1 + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_ID_MCI1,
		.end	= AT91CAP9_ID_MCI1,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device at91cap9_mmc1_device = {
	.name		= "at91_mci",
	.id		= 1,
	.dev		= {
				.dma_mask		= &mmc_dmamask,
				.coherent_dma_mask	= DMA_BIT_MASK(32),
				.platform_data		= &mmc1_data,
	},
	.resource	= mmc1_resources,
	.num_resources	= ARRAY_SIZE(mmc1_resources),
};

void __init at91_add_device_mmc(short mmc_id, struct at91_mmc_data *data)
{
	if (!data)
		return;

	
	if (data->det_pin) {
		at91_set_gpio_input(data->det_pin, 1);
		at91_set_deglitch(data->det_pin, 1);
	}
	if (data->wp_pin)
		at91_set_gpio_input(data->wp_pin, 1);
	if (data->vcc_pin)
		at91_set_gpio_output(data->vcc_pin, 0);

	if (mmc_id == 0) {		
		
		at91_set_A_periph(AT91_PIN_PA2, 0);

		
		at91_set_A_periph(AT91_PIN_PA1, 1);

		
		at91_set_A_periph(AT91_PIN_PA0, 1);
		if (data->wire4) {
			at91_set_A_periph(AT91_PIN_PA3, 1);
			at91_set_A_periph(AT91_PIN_PA4, 1);
			at91_set_A_periph(AT91_PIN_PA5, 1);
		}

		mmc0_data = *data;
		at91_clock_associate("mci0_clk", &at91cap9_mmc0_device.dev, "mci_clk");
		platform_device_register(&at91cap9_mmc0_device);
	} else {			
		
		at91_set_A_periph(AT91_PIN_PA16, 0);

		
		at91_set_A_periph(AT91_PIN_PA17, 1);

		
		at91_set_A_periph(AT91_PIN_PA18, 1);
		if (data->wire4) {
			at91_set_A_periph(AT91_PIN_PA19, 1);
			at91_set_A_periph(AT91_PIN_PA20, 1);
			at91_set_A_periph(AT91_PIN_PA21, 1);
		}

		mmc1_data = *data;
		at91_clock_associate("mci1_clk", &at91cap9_mmc1_device.dev, "mci_clk");
		platform_device_register(&at91cap9_mmc1_device);
	}
}
#else
void __init at91_add_device_mmc(short mmc_id, struct at91_mmc_data *data) {}
#endif




#if defined(CONFIG_MTD_NAND_ATMEL) || defined(CONFIG_MTD_NAND_ATMEL_MODULE)
static struct atmel_nand_data nand_data;

#define NAND_BASE	AT91_CHIPSELECT_3

static struct resource nand_resources[] = {
	[0] = {
		.start	= NAND_BASE,
		.end	= NAND_BASE + SZ_256M - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91_BASE_SYS + AT91_ECC,
		.end	= AT91_BASE_SYS + AT91_ECC + SZ_512 - 1,
		.flags	= IORESOURCE_MEM,
	}
};

static struct platform_device at91cap9_nand_device = {
	.name		= "atmel_nand",
	.id		= -1,
	.dev		= {
				.platform_data	= &nand_data,
	},
	.resource	= nand_resources,
	.num_resources	= ARRAY_SIZE(nand_resources),
};

void __init at91_add_device_nand(struct atmel_nand_data *data)
{
	unsigned long csa;

	if (!data)
		return;

	csa = at91_sys_read(AT91_MATRIX_EBICSA);
	at91_sys_write(AT91_MATRIX_EBICSA, csa | AT91_MATRIX_EBI_CS3A_SMC_SMARTMEDIA);

	
	if (data->enable_pin)
		at91_set_gpio_output(data->enable_pin, 1);

	
	if (data->rdy_pin)
		at91_set_gpio_input(data->rdy_pin, 1);

	
	if (data->det_pin)
		at91_set_gpio_input(data->det_pin, 1);

	nand_data = *data;
	platform_device_register(&at91cap9_nand_device);
}
#else
void __init at91_add_device_nand(struct atmel_nand_data *data) {}
#endif





#if defined(CONFIG_I2C_GPIO) || defined(CONFIG_I2C_GPIO_MODULE)

static struct i2c_gpio_platform_data pdata = {
	.sda_pin		= AT91_PIN_PB4,
	.sda_is_open_drain	= 1,
	.scl_pin		= AT91_PIN_PB5,
	.scl_is_open_drain	= 1,
	.udelay			= 2,		
};

static struct platform_device at91cap9_twi_device = {
	.name			= "i2c-gpio",
	.id			= -1,
	.dev.platform_data	= &pdata,
};

void __init at91_add_device_i2c(struct i2c_board_info *devices, int nr_devices)
{
	at91_set_GPIO_periph(AT91_PIN_PB4, 1);		
	at91_set_multi_drive(AT91_PIN_PB4, 1);

	at91_set_GPIO_periph(AT91_PIN_PB5, 1);		
	at91_set_multi_drive(AT91_PIN_PB5, 1);

	i2c_register_board_info(0, devices, nr_devices);
	platform_device_register(&at91cap9_twi_device);
}

#elif defined(CONFIG_I2C_AT91) || defined(CONFIG_I2C_AT91_MODULE)

static struct resource twi_resources[] = {
	[0] = {
		.start	= AT91CAP9_BASE_TWI,
		.end	= AT91CAP9_BASE_TWI + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_ID_TWI,
		.end	= AT91CAP9_ID_TWI,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device at91cap9_twi_device = {
	.name		= "at91_i2c",
	.id		= -1,
	.resource	= twi_resources,
	.num_resources	= ARRAY_SIZE(twi_resources),
};

void __init at91_add_device_i2c(struct i2c_board_info *devices, int nr_devices)
{
	
	at91_set_B_periph(AT91_PIN_PB4, 0);		
	at91_set_multi_drive(AT91_PIN_PB4, 1);

	at91_set_B_periph(AT91_PIN_PB5, 0);		
	at91_set_multi_drive(AT91_PIN_PB5, 1);

	i2c_register_board_info(0, devices, nr_devices);
	platform_device_register(&at91cap9_twi_device);
}
#else
void __init at91_add_device_i2c(struct i2c_board_info *devices, int nr_devices) {}
#endif



#if defined(CONFIG_SPI_ATMEL) || defined(CONFIG_SPI_ATMEL_MODULE)
static u64 spi_dmamask = DMA_BIT_MASK(32);

static struct resource spi0_resources[] = {
	[0] = {
		.start	= AT91CAP9_BASE_SPI0,
		.end	= AT91CAP9_BASE_SPI0 + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_ID_SPI0,
		.end	= AT91CAP9_ID_SPI0,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device at91cap9_spi0_device = {
	.name		= "atmel_spi",
	.id		= 0,
	.dev		= {
				.dma_mask		= &spi_dmamask,
				.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
	.resource	= spi0_resources,
	.num_resources	= ARRAY_SIZE(spi0_resources),
};

static const unsigned spi0_standard_cs[4] = { AT91_PIN_PA5, AT91_PIN_PA3, AT91_PIN_PD0, AT91_PIN_PD1 };

static struct resource spi1_resources[] = {
	[0] = {
		.start	= AT91CAP9_BASE_SPI1,
		.end	= AT91CAP9_BASE_SPI1 + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_ID_SPI1,
		.end	= AT91CAP9_ID_SPI1,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device at91cap9_spi1_device = {
	.name		= "atmel_spi",
	.id		= 1,
	.dev		= {
				.dma_mask		= &spi_dmamask,
				.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
	.resource	= spi1_resources,
	.num_resources	= ARRAY_SIZE(spi1_resources),
};

static const unsigned spi1_standard_cs[4] = { AT91_PIN_PB15, AT91_PIN_PB16, AT91_PIN_PB17, AT91_PIN_PB18 };

void __init at91_add_device_spi(struct spi_board_info *devices, int nr_devices)
{
	int i;
	unsigned long cs_pin;
	short enable_spi0 = 0;
	short enable_spi1 = 0;

	
	for (i = 0; i < nr_devices; i++) {
		if (devices[i].controller_data)
			cs_pin = (unsigned long) devices[i].controller_data;
		else if (devices[i].bus_num == 0)
			cs_pin = spi0_standard_cs[devices[i].chip_select];
		else
			cs_pin = spi1_standard_cs[devices[i].chip_select];

		if (devices[i].bus_num == 0)
			enable_spi0 = 1;
		else
			enable_spi1 = 1;

		
		at91_set_gpio_output(cs_pin, 1);

		
		devices[i].controller_data = (void *) cs_pin;
	}

	spi_register_board_info(devices, nr_devices);

	
	if (enable_spi0) {
		at91_set_B_periph(AT91_PIN_PA0, 0);	
		at91_set_B_periph(AT91_PIN_PA1, 0);	
		at91_set_B_periph(AT91_PIN_PA2, 0);	

		at91_clock_associate("spi0_clk", &at91cap9_spi0_device.dev, "spi_clk");
		platform_device_register(&at91cap9_spi0_device);
	}
	if (enable_spi1) {
		at91_set_A_periph(AT91_PIN_PB12, 0);	
		at91_set_A_periph(AT91_PIN_PB13, 0);	
		at91_set_A_periph(AT91_PIN_PB14, 0);	

		at91_clock_associate("spi1_clk", &at91cap9_spi1_device.dev, "spi_clk");
		platform_device_register(&at91cap9_spi1_device);
	}
}
#else
void __init at91_add_device_spi(struct spi_board_info *devices, int nr_devices) {}
#endif




#ifdef CONFIG_ATMEL_TCLIB

static struct resource tcb_resources[] = {
	[0] = {
		.start	= AT91CAP9_BASE_TCB0,
		.end	= AT91CAP9_BASE_TCB0 + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_ID_TCB,
		.end	= AT91CAP9_ID_TCB,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device at91cap9_tcb_device = {
	.name		= "atmel_tcb",
	.id		= 0,
	.resource	= tcb_resources,
	.num_resources	= ARRAY_SIZE(tcb_resources),
};

static void __init at91_add_device_tc(void)
{
	
	at91_clock_associate("tcb_clk", &at91cap9_tcb_device.dev, "t0_clk");
	platform_device_register(&at91cap9_tcb_device);
}
#else
static void __init at91_add_device_tc(void) { }
#endif




static struct resource rtt_resources[] = {
	{
		.start	= AT91_BASE_SYS + AT91_RTT,
		.end	= AT91_BASE_SYS + AT91_RTT + SZ_16 - 1,
		.flags	= IORESOURCE_MEM,
	}
};

static struct platform_device at91cap9_rtt_device = {
	.name		= "at91_rtt",
	.id		= 0,
	.resource	= rtt_resources,
	.num_resources	= ARRAY_SIZE(rtt_resources),
};

static void __init at91_add_device_rtt(void)
{
	platform_device_register(&at91cap9_rtt_device);
}




#if defined(CONFIG_AT91SAM9X_WATCHDOG) || defined(CONFIG_AT91SAM9X_WATCHDOG_MODULE)
static struct platform_device at91cap9_wdt_device = {
	.name		= "at91_wdt",
	.id		= -1,
	.num_resources	= 0,
};

static void __init at91_add_device_watchdog(void)
{
	platform_device_register(&at91cap9_wdt_device);
}
#else
static void __init at91_add_device_watchdog(void) {}
#endif




#if defined(CONFIG_ATMEL_PWM)
static u32 pwm_mask;

static struct resource pwm_resources[] = {
	[0] = {
		.start	= AT91CAP9_BASE_PWMC,
		.end	= AT91CAP9_BASE_PWMC + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_ID_PWMC,
		.end	= AT91CAP9_ID_PWMC,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device at91cap9_pwm0_device = {
	.name	= "atmel_pwm",
	.id	= -1,
	.dev	= {
		.platform_data		= &pwm_mask,
	},
	.resource	= pwm_resources,
	.num_resources	= ARRAY_SIZE(pwm_resources),
};

void __init at91_add_device_pwm(u32 mask)
{
	if (mask & (1 << AT91_PWM0))
		at91_set_A_periph(AT91_PIN_PB19, 1);	

	if (mask & (1 << AT91_PWM1))
		at91_set_B_periph(AT91_PIN_PB8, 1);	

	if (mask & (1 << AT91_PWM2))
		at91_set_B_periph(AT91_PIN_PC29, 1);	

	if (mask & (1 << AT91_PWM3))
		at91_set_B_periph(AT91_PIN_PA11, 1);	

	pwm_mask = mask;

	platform_device_register(&at91cap9_pwm0_device);
}
#else
void __init at91_add_device_pwm(u32 mask) {}
#endif





#if defined(CONFIG_SND_ATMEL_AC97C) || defined(CONFIG_SND_ATMEL_AC97C_MODULE)
static u64 ac97_dmamask = DMA_BIT_MASK(32);
static struct ac97c_platform_data ac97_data;

static struct resource ac97_resources[] = {
	[0] = {
		.start	= AT91CAP9_BASE_AC97C,
		.end	= AT91CAP9_BASE_AC97C + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_ID_AC97C,
		.end	= AT91CAP9_ID_AC97C,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device at91cap9_ac97_device = {
	.name		= "atmel_ac97c",
	.id		= 1,
	.dev		= {
				.dma_mask		= &ac97_dmamask,
				.coherent_dma_mask	= DMA_BIT_MASK(32),
				.platform_data		= &ac97_data,
	},
	.resource	= ac97_resources,
	.num_resources	= ARRAY_SIZE(ac97_resources),
};

void __init at91_add_device_ac97(struct ac97c_platform_data *data)
{
	if (!data)
		return;

	at91_set_A_periph(AT91_PIN_PA6, 0);	
	at91_set_A_periph(AT91_PIN_PA7, 0);	
	at91_set_A_periph(AT91_PIN_PA8, 0);	
	at91_set_A_periph(AT91_PIN_PA9, 0);	

	
	if (data->reset_pin)
		at91_set_gpio_output(data->reset_pin, 0);

	ac97_data = *data;
	platform_device_register(&at91cap9_ac97_device);
}
#else
void __init at91_add_device_ac97(struct ac97c_platform_data *data) {}
#endif




#if defined(CONFIG_FB_ATMEL) || defined(CONFIG_FB_ATMEL_MODULE)
static u64 lcdc_dmamask = DMA_BIT_MASK(32);
static struct atmel_lcdfb_info lcdc_data;

static struct resource lcdc_resources[] = {
	[0] = {
		.start	= AT91CAP9_LCDC_BASE,
		.end	= AT91CAP9_LCDC_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_ID_LCDC,
		.end	= AT91CAP9_ID_LCDC,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device at91_lcdc_device = {
	.name		= "atmel_lcdfb",
	.id		= 0,
	.dev		= {
				.dma_mask		= &lcdc_dmamask,
				.coherent_dma_mask	= DMA_BIT_MASK(32),
				.platform_data		= &lcdc_data,
	},
	.resource	= lcdc_resources,
	.num_resources	= ARRAY_SIZE(lcdc_resources),
};

void __init at91_add_device_lcdc(struct atmel_lcdfb_info *data)
{
	if (!data)
		return;

	if (cpu_is_at91cap9_revB())
		set_irq_type(AT91CAP9_ID_LCDC, IRQ_TYPE_LEVEL_HIGH);

	at91_set_A_periph(AT91_PIN_PC1, 0);	
	at91_set_A_periph(AT91_PIN_PC2, 0);	
	at91_set_A_periph(AT91_PIN_PC3, 0);	
	at91_set_B_periph(AT91_PIN_PB9, 0);	
	at91_set_A_periph(AT91_PIN_PC6, 0);	
	at91_set_A_periph(AT91_PIN_PC7, 0);	
	at91_set_A_periph(AT91_PIN_PC8, 0);	
	at91_set_A_periph(AT91_PIN_PC9, 0);	
	at91_set_A_periph(AT91_PIN_PC10, 0);	
	at91_set_A_periph(AT91_PIN_PC11, 0);	
	at91_set_A_periph(AT91_PIN_PC14, 0);	
	at91_set_A_periph(AT91_PIN_PC15, 0);	
	at91_set_A_periph(AT91_PIN_PC16, 0);	
	at91_set_A_periph(AT91_PIN_PC17, 0);	
	at91_set_A_periph(AT91_PIN_PC18, 0);	
	at91_set_A_periph(AT91_PIN_PC19, 0);	
	at91_set_A_periph(AT91_PIN_PC22, 0);	
	at91_set_A_periph(AT91_PIN_PC23, 0);	
	at91_set_A_periph(AT91_PIN_PC24, 0);	
	at91_set_A_periph(AT91_PIN_PC25, 0);	
	at91_set_A_periph(AT91_PIN_PC26, 0);	
	at91_set_A_periph(AT91_PIN_PC27, 0);	

	lcdc_data = *data;
	platform_device_register(&at91_lcdc_device);
}
#else
void __init at91_add_device_lcdc(struct atmel_lcdfb_info *data) {}
#endif




#if defined(CONFIG_ATMEL_SSC) || defined(CONFIG_ATMEL_SSC_MODULE)
static u64 ssc0_dmamask = DMA_BIT_MASK(32);

static struct resource ssc0_resources[] = {
	[0] = {
		.start	= AT91CAP9_BASE_SSC0,
		.end	= AT91CAP9_BASE_SSC0 + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_ID_SSC0,
		.end	= AT91CAP9_ID_SSC0,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device at91cap9_ssc0_device = {
	.name	= "ssc",
	.id	= 0,
	.dev	= {
		.dma_mask		= &ssc0_dmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
	.resource	= ssc0_resources,
	.num_resources	= ARRAY_SIZE(ssc0_resources),
};

static inline void configure_ssc0_pins(unsigned pins)
{
	if (pins & ATMEL_SSC_TF)
		at91_set_A_periph(AT91_PIN_PB0, 1);
	if (pins & ATMEL_SSC_TK)
		at91_set_A_periph(AT91_PIN_PB1, 1);
	if (pins & ATMEL_SSC_TD)
		at91_set_A_periph(AT91_PIN_PB2, 1);
	if (pins & ATMEL_SSC_RD)
		at91_set_A_periph(AT91_PIN_PB3, 1);
	if (pins & ATMEL_SSC_RK)
		at91_set_A_periph(AT91_PIN_PB4, 1);
	if (pins & ATMEL_SSC_RF)
		at91_set_A_periph(AT91_PIN_PB5, 1);
}

static u64 ssc1_dmamask = DMA_BIT_MASK(32);

static struct resource ssc1_resources[] = {
	[0] = {
		.start	= AT91CAP9_BASE_SSC1,
		.end	= AT91CAP9_BASE_SSC1 + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_ID_SSC1,
		.end	= AT91CAP9_ID_SSC1,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device at91cap9_ssc1_device = {
	.name	= "ssc",
	.id	= 1,
	.dev	= {
		.dma_mask		= &ssc1_dmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
	.resource	= ssc1_resources,
	.num_resources	= ARRAY_SIZE(ssc1_resources),
};

static inline void configure_ssc1_pins(unsigned pins)
{
	if (pins & ATMEL_SSC_TF)
		at91_set_A_periph(AT91_PIN_PB6, 1);
	if (pins & ATMEL_SSC_TK)
		at91_set_A_periph(AT91_PIN_PB7, 1);
	if (pins & ATMEL_SSC_TD)
		at91_set_A_periph(AT91_PIN_PB8, 1);
	if (pins & ATMEL_SSC_RD)
		at91_set_A_periph(AT91_PIN_PB9, 1);
	if (pins & ATMEL_SSC_RK)
		at91_set_A_periph(AT91_PIN_PB10, 1);
	if (pins & ATMEL_SSC_RF)
		at91_set_A_periph(AT91_PIN_PB11, 1);
}


void __init at91_add_device_ssc(unsigned id, unsigned pins)
{
	struct platform_device *pdev;

	
	switch (id) {
	case AT91CAP9_ID_SSC0:
		pdev = &at91cap9_ssc0_device;
		configure_ssc0_pins(pins);
		at91_clock_associate("ssc0_clk", &pdev->dev, "ssc");
		break;
	case AT91CAP9_ID_SSC1:
		pdev = &at91cap9_ssc1_device;
		configure_ssc1_pins(pins);
		at91_clock_associate("ssc1_clk", &pdev->dev, "ssc");
		break;
	default:
		return;
	}

	platform_device_register(pdev);
}

#else
void __init at91_add_device_ssc(unsigned id, unsigned pins) {}
#endif




#if defined(CONFIG_SERIAL_ATMEL)
static struct resource dbgu_resources[] = {
	[0] = {
		.start	= AT91_VA_BASE_SYS + AT91_DBGU,
		.end	= AT91_VA_BASE_SYS + AT91_DBGU + SZ_512 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91_ID_SYS,
		.end	= AT91_ID_SYS,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct atmel_uart_data dbgu_data = {
	.use_dma_tx	= 0,
	.use_dma_rx	= 0,		
	.regs		= (void __iomem *)(AT91_VA_BASE_SYS + AT91_DBGU),
};

static u64 dbgu_dmamask = DMA_BIT_MASK(32);

static struct platform_device at91cap9_dbgu_device = {
	.name		= "atmel_usart",
	.id		= 0,
	.dev		= {
				.dma_mask		= &dbgu_dmamask,
				.coherent_dma_mask	= DMA_BIT_MASK(32),
				.platform_data		= &dbgu_data,
	},
	.resource	= dbgu_resources,
	.num_resources	= ARRAY_SIZE(dbgu_resources),
};

static inline void configure_dbgu_pins(void)
{
	at91_set_A_periph(AT91_PIN_PC30, 0);		
	at91_set_A_periph(AT91_PIN_PC31, 1);		
}

static struct resource uart0_resources[] = {
	[0] = {
		.start	= AT91CAP9_BASE_US0,
		.end	= AT91CAP9_BASE_US0 + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_ID_US0,
		.end	= AT91CAP9_ID_US0,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct atmel_uart_data uart0_data = {
	.use_dma_tx	= 1,
	.use_dma_rx	= 1,
};

static u64 uart0_dmamask = DMA_BIT_MASK(32);

static struct platform_device at91cap9_uart0_device = {
	.name		= "atmel_usart",
	.id		= 1,
	.dev		= {
				.dma_mask		= &uart0_dmamask,
				.coherent_dma_mask	= DMA_BIT_MASK(32),
				.platform_data		= &uart0_data,
	},
	.resource	= uart0_resources,
	.num_resources	= ARRAY_SIZE(uart0_resources),
};

static inline void configure_usart0_pins(unsigned pins)
{
	at91_set_A_periph(AT91_PIN_PA22, 1);		
	at91_set_A_periph(AT91_PIN_PA23, 0);		

	if (pins & ATMEL_UART_RTS)
		at91_set_A_periph(AT91_PIN_PA24, 0);	
	if (pins & ATMEL_UART_CTS)
		at91_set_A_periph(AT91_PIN_PA25, 0);	
}

static struct resource uart1_resources[] = {
	[0] = {
		.start	= AT91CAP9_BASE_US1,
		.end	= AT91CAP9_BASE_US1 + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_ID_US1,
		.end	= AT91CAP9_ID_US1,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct atmel_uart_data uart1_data = {
	.use_dma_tx	= 1,
	.use_dma_rx	= 1,
};

static u64 uart1_dmamask = DMA_BIT_MASK(32);

static struct platform_device at91cap9_uart1_device = {
	.name		= "atmel_usart",
	.id		= 2,
	.dev		= {
				.dma_mask		= &uart1_dmamask,
				.coherent_dma_mask	= DMA_BIT_MASK(32),
				.platform_data		= &uart1_data,
	},
	.resource	= uart1_resources,
	.num_resources	= ARRAY_SIZE(uart1_resources),
};

static inline void configure_usart1_pins(unsigned pins)
{
	at91_set_A_periph(AT91_PIN_PD0, 1);		
	at91_set_A_periph(AT91_PIN_PD1, 0);		

	if (pins & ATMEL_UART_RTS)
		at91_set_B_periph(AT91_PIN_PD7, 0);	
	if (pins & ATMEL_UART_CTS)
		at91_set_B_periph(AT91_PIN_PD8, 0);	
}

static struct resource uart2_resources[] = {
	[0] = {
		.start	= AT91CAP9_BASE_US2,
		.end	= AT91CAP9_BASE_US2 + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= AT91CAP9_ID_US2,
		.end	= AT91CAP9_ID_US2,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct atmel_uart_data uart2_data = {
	.use_dma_tx	= 1,
	.use_dma_rx	= 1,
};

static u64 uart2_dmamask = DMA_BIT_MASK(32);

static struct platform_device at91cap9_uart2_device = {
	.name		= "atmel_usart",
	.id		= 3,
	.dev		= {
				.dma_mask		= &uart2_dmamask,
				.coherent_dma_mask	= DMA_BIT_MASK(32),
				.platform_data		= &uart2_data,
	},
	.resource	= uart2_resources,
	.num_resources	= ARRAY_SIZE(uart2_resources),
};

static inline void configure_usart2_pins(unsigned pins)
{
	at91_set_A_periph(AT91_PIN_PD2, 1);		
	at91_set_A_periph(AT91_PIN_PD3, 0);		

	if (pins & ATMEL_UART_RTS)
		at91_set_B_periph(AT91_PIN_PD5, 0);	
	if (pins & ATMEL_UART_CTS)
		at91_set_B_periph(AT91_PIN_PD6, 0);	
}

static struct platform_device *__initdata at91_uarts[ATMEL_MAX_UART];	
struct platform_device *atmel_default_console_device;	

void __init at91_register_uart(unsigned id, unsigned portnr, unsigned pins)
{
	struct platform_device *pdev;

	switch (id) {
		case 0:		
			pdev = &at91cap9_dbgu_device;
			configure_dbgu_pins();
			at91_clock_associate("mck", &pdev->dev, "usart");
			break;
		case AT91CAP9_ID_US0:
			pdev = &at91cap9_uart0_device;
			configure_usart0_pins(pins);
			at91_clock_associate("usart0_clk", &pdev->dev, "usart");
			break;
		case AT91CAP9_ID_US1:
			pdev = &at91cap9_uart1_device;
			configure_usart1_pins(pins);
			at91_clock_associate("usart1_clk", &pdev->dev, "usart");
			break;
		case AT91CAP9_ID_US2:
			pdev = &at91cap9_uart2_device;
			configure_usart2_pins(pins);
			at91_clock_associate("usart2_clk", &pdev->dev, "usart");
			break;
		default:
			return;
	}
	pdev->id = portnr;		

	if (portnr < ATMEL_MAX_UART)
		at91_uarts[portnr] = pdev;
}

void __init at91_set_serial_console(unsigned portnr)
{
	if (portnr < ATMEL_MAX_UART)
		atmel_default_console_device = at91_uarts[portnr];
}

void __init at91_add_device_serial(void)
{
	int i;

	for (i = 0; i < ATMEL_MAX_UART; i++) {
		if (at91_uarts[i])
			platform_device_register(at91_uarts[i]);
	}

	if (!atmel_default_console_device)
		printk(KERN_INFO "AT91: No default serial console defined.\n");
}
#else
void __init at91_register_uart(unsigned id, unsigned portnr, unsigned pins) {}
void __init at91_set_serial_console(unsigned portnr) {}
void __init at91_add_device_serial(void) {}
#endif




static int __init at91_add_standard_devices(void)
{
	at91_add_device_rtt();
	at91_add_device_watchdog();
	at91_add_device_tc();
	return 0;
}

arch_initcall(at91_add_standard_devices);
