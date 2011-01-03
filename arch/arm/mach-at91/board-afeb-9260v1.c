

#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>

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


static void __init afeb9260_map_io(void)
{
	
	at91sam9260_initialize(18432000);

	
	at91_register_uart(0, 0, 0);

	
	at91_register_uart(AT91SAM9260_ID_US0, 1,
			     ATMEL_UART_CTS | ATMEL_UART_RTS
			   | ATMEL_UART_DTR | ATMEL_UART_DSR
			   | ATMEL_UART_DCD | ATMEL_UART_RI);

	
	at91_register_uart(AT91SAM9260_ID_US1, 2,
			ATMEL_UART_CTS | ATMEL_UART_RTS);

	
	at91_set_serial_console(0);
}

static void __init afeb9260_init_irq(void)
{
	at91sam9260_init_interrupts(NULL);
}



static struct at91_usbh_data __initdata afeb9260_usbh_data = {
	.ports		= 1,
};


static struct at91_udc_data __initdata afeb9260_udc_data = {
	.vbus_pin	= AT91_PIN_PC5,
	.pullup_pin	= 0,		
};




static struct spi_board_info afeb9260_spi_devices[] = {
	{	
		.modalias	= "mtd_dataflash",
		.chip_select	= 1,
		.max_speed_hz	= 15 * 1000 * 1000,
		.bus_num	= 0,
	},
};



static struct at91_eth_data __initdata afeb9260_macb_data = {
	.phy_irq_pin	= AT91_PIN_PA9,
	.is_rmii	= 0,
};



static struct mtd_partition __initdata afeb9260_nand_partition[] = {
	{
		.name	= "bootloader",
		.offset	= 0,
		.size	= (640 * SZ_1K),
	},
	{
		.name	= "kernel",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= SZ_2M,
	},
	{
		.name	= "rootfs",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= MTDPART_SIZ_FULL,
	},
};

static struct mtd_partition * __init nand_partitions(int size, int *num_partitions)
{
	*num_partitions = ARRAY_SIZE(afeb9260_nand_partition);
	return afeb9260_nand_partition;
}

static struct atmel_nand_data __initdata afeb9260_nand_data = {
	.ale		= 21,
	.cle		= 22,
	.rdy_pin	= AT91_PIN_PC13,
	.enable_pin	= AT91_PIN_PC14,
	.partition_info	= nand_partitions,
	.bus_width_16	= 0,
};



static struct at91_mmc_data __initdata afeb9260_mmc_data = {
	.det_pin 	= AT91_PIN_PC9,
	.wp_pin 	= AT91_PIN_PC4,
	.slot_b		= 1,
	.wire4		= 1,
};



static struct i2c_board_info __initdata afeb9260_i2c_devices[] = {
	{
		I2C_BOARD_INFO("tlv320aic23", 0x1a),
	}, {
		I2C_BOARD_INFO("fm3130", 0x68),
	}, {
		I2C_BOARD_INFO("24c64", 0x50),
	},
};


static struct at91_cf_data afeb9260_cf_data = {
	.chipselect = 4,
	.irq_pin    = AT91_PIN_PA6,
	.rst_pin    = AT91_PIN_PA7,
	.flags      = AT91_CF_TRUE_IDE,
};

static void __init afeb9260_board_init(void)
{
	
	at91_add_device_serial();
	
	at91_add_device_usbh(&afeb9260_usbh_data);
	
	at91_add_device_udc(&afeb9260_udc_data);
	
	at91_add_device_spi(afeb9260_spi_devices,
			ARRAY_SIZE(afeb9260_spi_devices));
	
	at91_add_device_nand(&afeb9260_nand_data);
	
	at91_add_device_eth(&afeb9260_macb_data);

	
	at91_set_B_periph(AT91_PIN_PA10, 0);	
	at91_set_B_periph(AT91_PIN_PA11, 0);	
	
	at91_add_device_mmc(0, &afeb9260_mmc_data);
	
	at91_add_device_i2c(afeb9260_i2c_devices,
			ARRAY_SIZE(afeb9260_i2c_devices));
	
	at91_add_device_ssc(AT91SAM9260_ID_SSC, ATMEL_SSC_TX);
	
	at91_add_device_cf(&afeb9260_cf_data);
}

MACHINE_START(AFEB9260, "Custom afeb9260 board")
	
	.phys_io	= AT91_BASE_SYS,
	.io_pg_offst	= (AT91_VA_BASE_SYS >> 18) & 0xfffc,
	.boot_params	= AT91_SDRAM_BASE + 0x100,
	.timer		= &at91sam926x_timer,
	.map_io		= afeb9260_map_io,
	.init_irq	= afeb9260_init_irq,
	.init_machine	= afeb9260_board_init,
MACHINE_END

