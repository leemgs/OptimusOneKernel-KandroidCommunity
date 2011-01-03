

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mtd/physmap.h>
#include <linux/spi/flash.h>
#include <linux/spi/spi.h>
#include <linux/spi/orion_spi.h>
#include <linux/i2c.h>
#include <linux/mv643xx_eth.h>
#include <linux/ata_platform.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/timex.h>
#include <linux/serial_reg.h>
#include <linux/pci.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/kirkwood.h>
#include "common.h"
#include "mpp.h"



static struct mtd_partition qnap_ts219_partitions[] = {
	{
		.name		= "U-Boot",
		.size		= 0x00080000,
		.offset		= 0,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "Kernel",
		.size		= 0x00200000,
		.offset		= 0x00200000,
	}, {
		.name		= "RootFS1",
		.size		= 0x00900000,
		.offset		= 0x00400000,
	}, {
		.name		= "RootFS2",
		.size		= 0x00300000,
		.offset		= 0x00d00000,
	}, {
		.name		= "U-Boot Config",
		.size		= 0x00040000,
		.offset		= 0x00080000,
	}, {
		.name		= "NAS Config",
		.size		= 0x00140000,
		.offset		= 0x000c0000,
	},
};

static const struct flash_platform_data qnap_ts219_flash = {
	.type		= "m25p128",
	.name		= "spi_flash",
	.parts		= qnap_ts219_partitions,
	.nr_parts	= ARRAY_SIZE(qnap_ts219_partitions),
};

static struct spi_board_info __initdata qnap_ts219_spi_slave_info[] = {
	{
		.modalias	= "m25p80",
		.platform_data	= &qnap_ts219_flash,
		.irq		= -1,
		.max_speed_hz	= 20000000,
		.bus_num	= 0,
		.chip_select	= 0,
	},
};

static struct i2c_board_info __initdata qnap_ts219_i2c_rtc = {
	I2C_BOARD_INFO("s35390a", 0x30),
};

static struct mv643xx_eth_platform_data qnap_ts219_ge00_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(8),
};

static struct mv_sata_platform_data qnap_ts219_sata_data = {
	.n_ports	= 2,
};

static struct gpio_keys_button qnap_ts219_buttons[] = {
	{
		.code		= KEY_COPY,
		.gpio		= 15,
		.desc		= "USB Copy",
		.active_low	= 1,
	},
	{
		.code		= KEY_RESTART,
		.gpio		= 16,
		.desc		= "Reset",
		.active_low	= 1,
	},
};

static struct gpio_keys_platform_data qnap_ts219_button_data = {
	.buttons	= qnap_ts219_buttons,
	.nbuttons	= ARRAY_SIZE(qnap_ts219_buttons),
};

static struct platform_device qnap_ts219_button_device = {
	.name		= "gpio-keys",
	.id		= -1,
	.num_resources	= 0,
	.dev		= {
		.platform_data	= &qnap_ts219_button_data,
	}
};

static unsigned int qnap_ts219_mpp_config[] __initdata = {
	MPP0_SPI_SCn,
	MPP1_SPI_MOSI,
	MPP2_SPI_SCK,
	MPP3_SPI_MISO,
	MPP4_SATA1_ACTn,
	MPP5_SATA0_ACTn,
	MPP8_TW_SDA,
	MPP9_TW_SCK,
	MPP10_UART0_TXD,
	MPP11_UART0_RXD,
	MPP13_UART1_TXD,	
	MPP14_UART1_RXD,	
	MPP15_GPIO,		
	MPP16_GPIO,		
	0
};




#define UART1_REG(x)	(UART1_VIRT_BASE + ((UART_##x) << 2))

void qnap_ts219_power_off(void)
{
	
	const unsigned divisor = ((kirkwood_tclk + (8 * 19200)) / (16 * 19200));

	pr_info("%s: triggering power-off...\n", __func__);

	
	writel(0x83, UART1_REG(LCR));
	writel(divisor & 0xff, UART1_REG(DLL));
	writel((divisor >> 8) & 0xff, UART1_REG(DLM));
	writel(0x03, UART1_REG(LCR));
	writel(0x00, UART1_REG(IER));
	writel(0x00, UART1_REG(FCR));
	writel(0x00, UART1_REG(MCR));

	
	writel('A', UART1_REG(TX));
}

static void __init qnap_ts219_init(void)
{
	
	kirkwood_init();
	kirkwood_mpp_conf(qnap_ts219_mpp_config);

	kirkwood_uart0_init();
	kirkwood_uart1_init(); 
	spi_register_board_info(qnap_ts219_spi_slave_info,
				ARRAY_SIZE(qnap_ts219_spi_slave_info));
	kirkwood_spi_init();
	kirkwood_i2c_init();
	i2c_register_board_info(0, &qnap_ts219_i2c_rtc, 1);
	kirkwood_ge00_init(&qnap_ts219_ge00_data);
	kirkwood_sata_init(&qnap_ts219_sata_data);
	kirkwood_ehci_init();
	platform_device_register(&qnap_ts219_button_device);

	pm_power_off = qnap_ts219_power_off;

}

static int __init ts219_pci_init(void)
{
   if (machine_is_ts219())
           kirkwood_pcie_init();

   return 0;
}
subsys_initcall(ts219_pci_init);

MACHINE_START(TS219, "QNAP TS-119/TS-219")
	
	.phys_io	= KIRKWOOD_REGS_PHYS_BASE,
	.io_pg_offst	= ((KIRKWOOD_REGS_VIRT_BASE) >> 18) & 0xfffc,
	.boot_params	= 0x00000100,
	.init_machine	= qnap_ts219_init,
	.map_io		= kirkwood_map_io,
	.init_irq	= kirkwood_init_irq,
	.timer		= &kirkwood_timer,
MACHINE_END
