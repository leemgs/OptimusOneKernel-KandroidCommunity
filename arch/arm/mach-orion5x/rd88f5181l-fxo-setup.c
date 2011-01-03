

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/pci.h>
#include <linux/irq.h>
#include <linux/mtd/physmap.h>
#include <linux/mv643xx_eth.h>
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



#define RD88F5181L_FXO_NOR_BOOT_BASE		0xff800000
#define RD88F5181L_FXO_NOR_BOOT_SIZE		SZ_8M



static struct physmap_flash_data rd88f5181l_fxo_nor_boot_flash_data = {
	.width		= 1,
};

static struct resource rd88f5181l_fxo_nor_boot_flash_resource = {
	.flags		= IORESOURCE_MEM,
	.start		= RD88F5181L_FXO_NOR_BOOT_BASE,
	.end		= RD88F5181L_FXO_NOR_BOOT_BASE +
			  RD88F5181L_FXO_NOR_BOOT_SIZE - 1,
};

static struct platform_device rd88f5181l_fxo_nor_boot_flash = {
	.name			= "physmap-flash",
	.id			= 0,
	.dev		= {
		.platform_data	= &rd88f5181l_fxo_nor_boot_flash_data,
	},
	.num_resources		= 1,
	.resource		= &rd88f5181l_fxo_nor_boot_flash_resource,
};



static struct orion5x_mpp_mode rd88f5181l_fxo_mpp_modes[] __initdata = {
	{  0, MPP_GPIO },		
	{  1, MPP_GPIO },		
	{  2, MPP_GPIO },		
	{  3, MPP_GPIO },		
	{  4, MPP_GPIO },		
	{  5, MPP_GPIO },		
	{  6, MPP_PCI_CLK },		
	{  7, MPP_PCI_CLK },		
	{  8, MPP_GPIO },		
	{  9, MPP_GPIO },		
	{ 10, MPP_GPIO },		
	{ 11, MPP_GPIO },		
	{ 12, MPP_GIGE },		
	{ 13, MPP_GIGE },		
	{ 14, MPP_GIGE },		
	{ 15, MPP_GIGE },		
	{ 16, MPP_GIGE },		
	{ 17, MPP_GIGE },		
	{ 18, MPP_GIGE },		
	{ 19, MPP_GIGE },		
	{ -1 },
};

static struct mv643xx_eth_platform_data rd88f5181l_fxo_eth_data = {
	.phy_addr	= MV643XX_ETH_PHY_NONE,
	.speed		= SPEED_1000,
	.duplex		= DUPLEX_FULL,
};

static struct dsa_chip_data rd88f5181l_fxo_switch_chip_data = {
	.port_names[0]	= "lan2",
	.port_names[1]	= "lan1",
	.port_names[2]	= "wan",
	.port_names[3]	= "cpu",
	.port_names[5]	= "lan4",
	.port_names[7]	= "lan3",
};

static struct dsa_platform_data rd88f5181l_fxo_switch_plat_data = {
	.nr_chips	= 1,
	.chip		= &rd88f5181l_fxo_switch_chip_data,
};

static void __init rd88f5181l_fxo_init(void)
{
	
	orion5x_init();

	orion5x_mpp_conf(rd88f5181l_fxo_mpp_modes);

	
	orion5x_ehci0_init();
	orion5x_eth_init(&rd88f5181l_fxo_eth_data);
	orion5x_eth_switch_init(&rd88f5181l_fxo_switch_plat_data, NO_IRQ);
	orion5x_uart0_init();

	orion5x_setup_dev_boot_win(RD88F5181L_FXO_NOR_BOOT_BASE,
				   RD88F5181L_FXO_NOR_BOOT_SIZE);
	platform_device_register(&rd88f5181l_fxo_nor_boot_flash);
}

static int __init
rd88f5181l_fxo_pci_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	int irq;

	
	irq = orion5x_pci_map_irq(dev, slot, pin);
	if (irq != -1)
		return irq;

	
	return gpio_to_irq(1);
}

static struct hw_pci rd88f5181l_fxo_pci __initdata = {
	.nr_controllers	= 2,
	.swizzle	= pci_std_swizzle,
	.setup		= orion5x_pci_sys_setup,
	.scan		= orion5x_pci_sys_scan_bus,
	.map_irq	= rd88f5181l_fxo_pci_map_irq,
};

static int __init rd88f5181l_fxo_pci_init(void)
{
	if (machine_is_rd88f5181l_fxo()) {
		orion5x_pci_set_cardbus_mode();
		pci_common_init(&rd88f5181l_fxo_pci);
	}

	return 0;
}
subsys_initcall(rd88f5181l_fxo_pci_init);

MACHINE_START(RD88F5181L_FXO, "Marvell Orion-VoIP FXO Reference Design")
	
	.phys_io	= ORION5X_REGS_PHYS_BASE,
	.io_pg_offst	= ((ORION5X_REGS_VIRT_BASE) >> 18) & 0xFFFC,
	.boot_params	= 0x00000100,
	.init_machine	= rd88f5181l_fxo_init,
	.map_io		= orion5x_map_io,
	.init_irq	= orion5x_init_irq,
	.timer		= &orion5x_timer,
	.fixup		= tag_fixup_mem32,
MACHINE_END
