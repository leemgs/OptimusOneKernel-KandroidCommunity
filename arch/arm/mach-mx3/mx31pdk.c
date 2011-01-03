

#include <linux/types.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/smsc911x.h>
#include <linux/platform_device.h>

#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/memory.h>
#include <asm/mach/map.h>
#include <mach/common.h>
#include <mach/board-mx31pdk.h>
#include <mach/imx-uart.h>
#include <mach/iomux-mx3.h>
#include "devices.h"



static int mx31pdk_pins[] = {
	
	MX31_PIN_CTS1__CTS1,
	MX31_PIN_RTS1__RTS1,
	MX31_PIN_TXD1__TXD1,
	MX31_PIN_RXD1__RXD1,
	IOMUX_MODE(MX31_PIN_GPIO1_1, IOMUX_CONFIG_GPIO),
};

static struct imxuart_platform_data uart_pdata = {
	.flags = IMXUART_HAVE_RTSCTS,
};



static struct smsc911x_platform_config smsc911x_config = {
	.irq_polarity	= SMSC911X_IRQ_POLARITY_ACTIVE_LOW,
	.irq_type	= SMSC911X_IRQ_TYPE_PUSH_PULL,
	.flags		= SMSC911X_USE_16BIT | SMSC911X_FORCE_INTERNAL_PHY,
	.phy_interface	= PHY_INTERFACE_MODE_MII,
};

static struct resource smsc911x_resources[] = {
	{
		.start		= LAN9217_BASE_ADDR,
		.end		= LAN9217_BASE_ADDR + 0xff,
		.flags		= IORESOURCE_MEM,
	}, {
		.start		= EXPIO_INT_ENET,
		.end		= EXPIO_INT_ENET,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device smsc911x_device = {
	.name		= "smsc911x",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(smsc911x_resources),
	.resource	= smsc911x_resources,
	.dev		= {
		.platform_data = &smsc911x_config,
	},
};



static void mx31pdk_expio_irq_handler(uint32_t irq, struct irq_desc *desc)
{
	uint32_t imr_val;
	uint32_t int_valid;
	uint32_t expio_irq;

	imr_val = __raw_readw(CPLD_INT_MASK_REG);
	int_valid = __raw_readw(CPLD_INT_STATUS_REG) & ~imr_val;

	expio_irq = MXC_EXP_IO_BASE;
	for (; int_valid != 0; int_valid >>= 1, expio_irq++) {
		if ((int_valid & 1) == 0)
			continue;
		generic_handle_irq(expio_irq);
	}
}


static void expio_mask_irq(uint32_t irq)
{
	uint16_t reg;
	uint32_t expio = MXC_IRQ_TO_EXPIO(irq);

	
	reg = __raw_readw(CPLD_INT_MASK_REG);
	reg |= 1 << expio;
	__raw_writew(reg, CPLD_INT_MASK_REG);
}


static void expio_ack_irq(uint32_t irq)
{
	uint32_t expio = MXC_IRQ_TO_EXPIO(irq);

	
	__raw_writew(1 << expio, CPLD_INT_RESET_REG);
	__raw_writew(0, CPLD_INT_RESET_REG);
	
	expio_mask_irq(irq);
}


static void expio_unmask_irq(uint32_t irq)
{
	uint16_t reg;
	uint32_t expio = MXC_IRQ_TO_EXPIO(irq);

	
	reg = __raw_readw(CPLD_INT_MASK_REG);
	reg &= ~(1 << expio);
	__raw_writew(reg, CPLD_INT_MASK_REG);
}

static struct irq_chip expio_irq_chip = {
	.ack = expio_ack_irq,
	.mask = expio_mask_irq,
	.unmask = expio_unmask_irq,
};

static int __init mx31pdk_init_expio(void)
{
	int i;
	int ret;

	
	if ((__raw_readw(CPLD_MAGIC_NUMBER1_REG) != 0xAAAA) ||
	    (__raw_readw(CPLD_MAGIC_NUMBER2_REG) != 0x5555) ||
	    (__raw_readw(CPLD_MAGIC_NUMBER3_REG) != 0xCAFE)) {
		
		return -ENODEV;
	}

	pr_info("i.MX31PDK Debug board detected, rev = 0x%04X\n",
		__raw_readw(CPLD_CODE_VER_REG));

	
	ret = gpio_request(IOMUX_TO_GPIO(MX31_PIN_GPIO1_1), "sms9217-irq");
	if (ret)
		pr_warning("could not get LAN irq gpio\n");
	else
		gpio_direction_input(IOMUX_TO_GPIO(MX31_PIN_GPIO1_1));

	
	__raw_writew(0, CPLD_INT_MASK_REG);
	__raw_writew(0xFFFF, CPLD_INT_RESET_REG);
	__raw_writew(0, CPLD_INT_RESET_REG);
	__raw_writew(0x1F, CPLD_INT_MASK_REG);
	for (i = MXC_EXP_IO_BASE;
	     i < (MXC_EXP_IO_BASE + MXC_MAX_EXP_IO_LINES);
	     i++) {
		set_irq_chip(i, &expio_irq_chip);
		set_irq_handler(i, handle_level_irq);
		set_irq_flags(i, IRQF_VALID);
	}
	set_irq_type(EXPIO_PARENT_INT, IRQ_TYPE_LEVEL_LOW);
	set_irq_chained_handler(EXPIO_PARENT_INT, mx31pdk_expio_irq_handler);

	return 0;
}


static struct map_desc mx31pdk_io_desc[] __initdata = {
	{
		.virtual = SPBA0_BASE_ADDR_VIRT,
		.pfn = __phys_to_pfn(SPBA0_BASE_ADDR),
		.length = SPBA0_SIZE,
		.type = MT_DEVICE_NONSHARED,
	}, {
		.virtual = CS5_BASE_ADDR_VIRT,
		.pfn = __phys_to_pfn(CS5_BASE_ADDR),
		.length = CS5_SIZE,
		.type = MT_DEVICE,
	},
};


static void __init mx31pdk_map_io(void)
{
	mx31_map_io();
	iotable_init(mx31pdk_io_desc, ARRAY_SIZE(mx31pdk_io_desc));
}


static void __init mxc_board_init(void)
{
	mxc_iomux_setup_multiple_pins(mx31pdk_pins, ARRAY_SIZE(mx31pdk_pins),
				      "mx31pdk");

	mxc_register_device(&mxc_uart_device0, &uart_pdata);

	if (!mx31pdk_init_expio())
		platform_device_register(&smsc911x_device);
}

static void __init mx31pdk_timer_init(void)
{
	mx31_clocks_init(26000000);
}

static struct sys_timer mx31pdk_timer = {
	.init	= mx31pdk_timer_init,
};


MACHINE_START(MX31_3DS, "Freescale MX31PDK (3DS)")
	
	.phys_io	= AIPS1_BASE_ADDR,
	.io_pg_offst	= ((AIPS1_BASE_ADDR_VIRT) >> 18) & 0xfffc,
	.boot_params    = PHYS_OFFSET + 0x100,
	.map_io         = mx31pdk_map_io,
	.init_irq       = mx31_init_irq,
	.init_machine   = mxc_board_init,
	.timer          = &mx31pdk_timer,
MACHINE_END
