

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/pci.h>
#include <linux/irq.h>
#include <linux/mtd/physmap.h>
#include <linux/mv643xx_eth.h>
#include <linux/spi/spi.h>
#include <linux/spi/orion_spi.h>
#include <linux/spi/flash.h>
#include <linux/ethtool.h>
#include <net/dsa.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>
#include <asm/leds.h>
#include <asm/mach/arch.h>
#include <asm/mach/pci.h>
#include <mach/orion5x.h>
#include "common.h"
#include "mpp.h"

static struct mv643xx_eth_platform_data rd88f6183ap_ge_eth_data = {
	.phy_addr	= -1,
	.speed		= SPEED_1000,
	.duplex		= DUPLEX_FULL,
};

static struct dsa_chip_data rd88f6183ap_ge_switch_chip_data = {
	.port_names[0]	= "lan1",
	.port_names[1]	= "lan2",
	.port_names[2]	= "lan3",
	.port_names[3]	= "lan4",
	.port_names[4]	= "wan",
	.port_names[5]	= "cpu",
};

static struct dsa_platform_data rd88f6183ap_ge_switch_plat_data = {
	.nr_chips	= 1,
	.chip		= &rd88f6183ap_ge_switch_chip_data,
};

static struct mtd_partition rd88f6183ap_ge_partitions[] = {
	{
		.name	= "kernel",
		.offset	= 0x00000000,
		.size	= 0x00200000,
	}, {
		.name	= "rootfs",
		.offset	= 0x00200000,
		.size	= 0x00500000,
	}, {
		.name	= "nvram",
		.offset	= 0x00700000,
		.size	= 0x00080000,
	},
};

static struct flash_platform_data rd88f6183ap_ge_spi_slave_data = {
	.type		= "m25p64",
	.nr_parts	= ARRAY_SIZE(rd88f6183ap_ge_partitions),
	.parts		= rd88f6183ap_ge_partitions,
};

static struct spi_board_info __initdata rd88f6183ap_ge_spi_slave_info[] = {
	{
		.modalias	= "m25p80",
		.platform_data	= &rd88f6183ap_ge_spi_slave_data,
		.irq		= NO_IRQ,
		.max_speed_hz	= 20000000,
		.bus_num	= 0,
		.chip_select	= 0,
	},
};

static void __init rd88f6183ap_ge_init(void)
{
	
	orion5x_init();

	
	orion5x_ehci0_init();
	orion5x_eth_init(&rd88f6183ap_ge_eth_data);
	orion5x_eth_switch_init(&rd88f6183ap_ge_switch_plat_data,
				gpio_to_irq(3));
	spi_register_board_info(rd88f6183ap_ge_spi_slave_info,
				ARRAY_SIZE(rd88f6183ap_ge_spi_slave_info));
	orion5x_spi_init();
	orion5x_uart0_init();
}

static struct hw_pci rd88f6183ap_ge_pci __initdata = {
	.nr_controllers	= 2,
	.swizzle	= pci_std_swizzle,
	.setup		= orion5x_pci_sys_setup,
	.scan		= orion5x_pci_sys_scan_bus,
	.map_irq	= orion5x_pci_map_irq,
};

static int __init rd88f6183ap_ge_pci_init(void)
{
	if (machine_is_rd88f6183ap_ge()) {
		orion5x_pci_disable();
		pci_common_init(&rd88f6183ap_ge_pci);
	}

	return 0;
}
subsys_initcall(rd88f6183ap_ge_pci_init);

MACHINE_START(RD88F6183AP_GE, "Marvell Orion-1-90 AP GE Reference Design")
	
	.phys_io	= ORION5X_REGS_PHYS_BASE,
	.io_pg_offst	= ((ORION5X_REGS_VIRT_BASE) >> 18) & 0xFFFC,
	.boot_params	= 0x00000100,
	.init_machine	= rd88f6183ap_ge_init,
	.map_io		= orion5x_map_io,
	.init_irq	= orion5x_init_irq,
	.timer		= &orion5x_timer,
	.fixup		= tag_fixup_mem32,
MACHINE_END
