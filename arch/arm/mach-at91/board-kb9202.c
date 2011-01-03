

#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include <mach/hardware.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/irq.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/board.h>
#include <mach/gpio.h>

#include <mach/at91rm9200_mc.h>

#include "generic.h"


static void __init kb9202_map_io(void)
{
	
	at91rm9200_initialize(10000000, AT91RM9200_PQFP);

	
	at91_init_leds(AT91_PIN_PC19, AT91_PIN_PC18);

	
	at91_register_uart(0, 0, 0);

	
	at91_register_uart(AT91RM9200_ID_US0, 1, 0);

	
	at91_register_uart(AT91RM9200_ID_US1, 2, 0);

	
	at91_register_uart(AT91RM9200_ID_US3, 3, ATMEL_UART_CTS | ATMEL_UART_RTS);

	
	at91_set_serial_console(0);
}

static void __init kb9202_init_irq(void)
{
	at91rm9200_init_interrupts(NULL);
}

static struct at91_eth_data __initdata kb9202_eth_data = {
	.phy_irq_pin	= AT91_PIN_PB29,
	.is_rmii	= 0,
};

static struct at91_usbh_data __initdata kb9202_usbh_data = {
	.ports		= 1,
};

static struct at91_udc_data __initdata kb9202_udc_data = {
	.vbus_pin	= AT91_PIN_PB24,
	.pullup_pin	= AT91_PIN_PB22,
};

static struct at91_mmc_data __initdata kb9202_mmc_data = {
	.det_pin	= AT91_PIN_PB2,
	.slot_b		= 0,
	.wire4		= 1,
};

static struct mtd_partition __initdata kb9202_nand_partition[] = {
	{
		.name	= "nand_fs",
		.offset	= 0,
		.size	= MTDPART_SIZ_FULL,
	},
};

static struct mtd_partition * __init nand_partitions(int size, int *num_partitions)
{
	*num_partitions = ARRAY_SIZE(kb9202_nand_partition);
	return kb9202_nand_partition;
}

static struct atmel_nand_data __initdata kb9202_nand_data = {
	.ale		= 22,
	.cle		= 21,
	
	.rdy_pin	= AT91_PIN_PC29,
	.enable_pin	= AT91_PIN_PC28,
	.partition_info	= nand_partitions,
};

static void __init kb9202_board_init(void)
{
	
	at91_add_device_serial();
	
	at91_add_device_eth(&kb9202_eth_data);
	
	at91_add_device_usbh(&kb9202_usbh_data);
	
	at91_add_device_udc(&kb9202_udc_data);
	
	at91_add_device_mmc(0, &kb9202_mmc_data);
	
	at91_add_device_i2c(NULL, 0);
	
	at91_add_device_spi(NULL, 0);
	
	at91_add_device_nand(&kb9202_nand_data);
}

MACHINE_START(KB9200, "KB920x")
	
	.phys_io	= AT91_BASE_SYS,
	.io_pg_offst	= (AT91_VA_BASE_SYS >> 18) & 0xfffc,
	.boot_params	= AT91_SDRAM_BASE + 0x100,
	.timer		= &at91rm9200_timer,
	.map_io		= kb9202_map_io,
	.init_irq	= kb9202_init_irq,
	.init_machine	= kb9202_board_init,
MACHINE_END
