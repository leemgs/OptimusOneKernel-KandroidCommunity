

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/ata_platform.h>
#include <linux/mv643xx_eth.h>
#include <linux/ethtool.h>
#include <mach/mv78xx0.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include "common.h"

static struct mv643xx_eth_platform_data rd78x00_masa_ge00_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(8),
};

static struct mv643xx_eth_platform_data rd78x00_masa_ge01_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(9),
};

static struct mv643xx_eth_platform_data rd78x00_masa_ge10_data = {
};

static struct mv643xx_eth_platform_data rd78x00_masa_ge11_data = {
};

static struct mv_sata_platform_data rd78x00_masa_sata_data = {
	.n_ports	= 2,
};

static void __init rd78x00_masa_init(void)
{
	
	mv78xx0_init();

	
	if (mv78xx0_core_index() == 0) {
		mv78xx0_ehci0_init();
		mv78xx0_ehci1_init();
		mv78xx0_ge00_init(&rd78x00_masa_ge00_data);
		mv78xx0_ge10_init(&rd78x00_masa_ge10_data);
		mv78xx0_sata_init(&rd78x00_masa_sata_data);
		mv78xx0_uart0_init();
		mv78xx0_uart2_init();
	} else {
		mv78xx0_ehci2_init();
		mv78xx0_ge01_init(&rd78x00_masa_ge01_data);
		mv78xx0_ge11_init(&rd78x00_masa_ge11_data);
		mv78xx0_uart1_init();
		mv78xx0_uart3_init();
	}
}

static int __init rd78x00_pci_init(void)
{
	
	if (machine_is_rd78x00_masa() && mv78xx0_core_index() == 0)
		mv78xx0_pcie_init(1, 1);

	return 0;
}
subsys_initcall(rd78x00_pci_init);

MACHINE_START(RD78X00_MASA, "Marvell RD-78x00-MASA Development Board")
	
	.phys_io	= MV78XX0_REGS_PHYS_BASE,
	.io_pg_offst	= ((MV78XX0_REGS_VIRT_BASE) >> 18) & 0xfffc,
	.boot_params	= 0x00000100,
	.init_machine	= rd78x00_masa_init,
	.map_io		= mv78xx0_map_io,
	.init_irq	= mv78xx0_init_irq,
	.timer		= &mv78xx0_timer,
MACHINE_END
