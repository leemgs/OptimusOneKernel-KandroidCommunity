

#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>

#include <mach/hardware.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/irq.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/board.h>
#include <mach/gpio.h>

#include "generic.h"


static void __init ecb_at91map_io(void)
{
	
	at91rm9200_initialize(18432000, AT91RM9200_PQFP);

	
	at91_init_leds(AT91_PIN_PC7, AT91_PIN_PC7);

	
	at91_register_uart(0, 0, 0);

	
	at91_register_uart(AT91RM9200_ID_US0, 1, 0);

	
	at91_set_serial_console(0);
}

static void __init ecb_at91init_irq(void)
{
	at91rm9200_init_interrupts(NULL);
}

static struct at91_eth_data __initdata ecb_at91eth_data = {
	.phy_irq_pin	= AT91_PIN_PC4,
	.is_rmii	= 0,
};

static struct at91_usbh_data __initdata ecb_at91usbh_data = {
	.ports		= 1,
};

static struct at91_mmc_data __initdata ecb_at91mmc_data = {
	.slot_b		= 0,
	.wire4		= 1,
};


#if defined(CONFIG_MTD_DATAFLASH)
static struct mtd_partition __initdata my_flash0_partitions[] =
{
	{	
		.name	= "Darrell-loader",
		.offset	= 0,
		.size	= 12 * 1056,
	},
	{
		.name	= "U-boot",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= 110 * 1056,
	},
	{	
		.name	= "UBoot-env",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= 8 * 1056,
	},
	{	
		.name	= "Kernel",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= 1534 * 1056,
	},
	{	
		.name	= "Filesystem",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= MTDPART_SIZ_FULL,	
	}
};

static struct flash_platform_data __initdata my_flash0_platform = {
	.name		= "Removable flash card",
	.parts		= my_flash0_partitions,
	.nr_parts	= ARRAY_SIZE(my_flash0_partitions)
};

#endif

static struct spi_board_info __initdata ecb_at91spi_devices[] = {
	{	
		.modalias	= "mtd_dataflash",
		.chip_select	= 0,
		.max_speed_hz	= 10 * 1000 * 1000,
		.bus_num	= 0,
#if defined(CONFIG_MTD_DATAFLASH)
		.platform_data	= &my_flash0_platform,
#endif
	},
	{	
		.modalias	= "spi-cs1",
		.chip_select	= 1,
		.max_speed_hz	= 250 * 1000,
	},
	{	
		.modalias	= "spi-cs2",
		.chip_select	= 2,
		.max_speed_hz	= 1 * 1000 * 1000,
	},
	{	
		.modalias	= "spi-cs3",
		.chip_select	= 3,
		.max_speed_hz	= 10 * 1000 * 1000,
	},
};

static void __init ecb_at91board_init(void)
{
	
	at91_add_device_serial();

	
	at91_add_device_eth(&ecb_at91eth_data);

	
	at91_add_device_usbh(&ecb_at91usbh_data);

	
	at91_add_device_i2c(NULL, 0);

	
	at91_add_device_mmc(0, &ecb_at91mmc_data);

	
	at91_add_device_spi(ecb_at91spi_devices, ARRAY_SIZE(ecb_at91spi_devices));
}

MACHINE_START(ECBAT91, "emQbit's ECB_AT91")
	
	.phys_io	= AT91_BASE_SYS,
	.io_pg_offst	= (AT91_VA_BASE_SYS >> 18) & 0xfffc,
	.boot_params	= AT91_SDRAM_BASE + 0x100,
	.timer		= &at91rm9200_timer,
	.map_io		= ecb_at91map_io,
	.init_irq	= ecb_at91init_irq,
	.init_machine	= ecb_at91board_init,
MACHINE_END
