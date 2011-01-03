

#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/mtd/physmap.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/irq.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/at91rm9200_mc.h>

#include "generic.h"


static void __init ek_map_io(void)
{
	
	at91rm9200_initialize(18432000, AT91RM9200_BGA);

	
	at91_init_leds(AT91_PIN_PB1, AT91_PIN_PB2);

	
	at91_register_uart(0, 0, 0);

	
	at91_register_uart(AT91RM9200_ID_US1, 1, ATMEL_UART_CTS | ATMEL_UART_RTS
			   | ATMEL_UART_DTR | ATMEL_UART_DSR | ATMEL_UART_DCD
			   | ATMEL_UART_RI);

	
	at91_set_serial_console(0);
}

static void __init ek_init_irq(void)
{
	at91rm9200_init_interrupts(NULL);
}

static struct at91_eth_data __initdata ek_eth_data = {
	.phy_irq_pin	= AT91_PIN_PC4,
	.is_rmii	= 1,
};

static struct at91_usbh_data __initdata ek_usbh_data = {
	.ports		= 2,
};

static struct at91_udc_data __initdata ek_udc_data = {
	.vbus_pin	= AT91_PIN_PD4,
	.pullup_pin	= AT91_PIN_PD5,
};

static struct at91_mmc_data __initdata ek_mmc_data = {
	.det_pin	= AT91_PIN_PB27,
	.slot_b		= 0,
	.wire4		= 1,
	.wp_pin		= AT91_PIN_PA17,
};

static struct spi_board_info ek_spi_devices[] = {
	{	
		.modalias	= "mtd_dataflash",
		.chip_select	= 0,
		.max_speed_hz	= 15 * 1000 * 1000,
	},
#ifdef CONFIG_MTD_AT91_DATAFLASH_CARD
	{	
		.modalias	= "mtd_dataflash",
		.chip_select	= 3,
		.max_speed_hz	= 15 * 1000 * 1000,
	},
#endif
};

static struct i2c_board_info __initdata ek_i2c_devices[] = {
	{
		I2C_BOARD_INFO("ics1523", 0x26),
	},
	{
		I2C_BOARD_INFO("dac3550", 0x4d),
	}
};

#define EK_FLASH_BASE	AT91_CHIPSELECT_0
#define EK_FLASH_SIZE	SZ_2M

static struct physmap_flash_data ek_flash_data = {
	.width		= 2,
};

static struct resource ek_flash_resource = {
	.start		= EK_FLASH_BASE,
	.end		= EK_FLASH_BASE + EK_FLASH_SIZE - 1,
	.flags		= IORESOURCE_MEM,
};

static struct platform_device ek_flash = {
	.name		= "physmap-flash",
	.id		= 0,
	.dev		= {
				.platform_data	= &ek_flash_data,
			},
	.resource	= &ek_flash_resource,
	.num_resources	= 1,
};

static struct gpio_led ek_leds[] = {
	{	
		.name			= "green",
		.gpio			= AT91_PIN_PB0,
		.active_low		= 1,
		.default_trigger	= "mmc0",
	},
	{	
		.name			= "yellow",
		.gpio			= AT91_PIN_PB1,
		.active_low		= 1,
		.default_trigger	= "heartbeat",
	},
	{	
		.name			= "red",
		.gpio			= AT91_PIN_PB2,
		.active_low		= 1,
	}
};

static void __init ek_board_init(void)
{
	
	at91_add_device_serial();
	
	at91_add_device_eth(&ek_eth_data);
	
	at91_add_device_usbh(&ek_usbh_data);
	
	at91_add_device_udc(&ek_udc_data);
	at91_set_multi_drive(ek_udc_data.pullup_pin, 1);	
	
	at91_add_device_i2c(ek_i2c_devices, ARRAY_SIZE(ek_i2c_devices));
	
	at91_add_device_spi(ek_spi_devices, ARRAY_SIZE(ek_spi_devices));
#ifdef CONFIG_MTD_AT91_DATAFLASH_CARD
	
	at91_set_gpio_output(AT91_PIN_PB22, 0);
#else
	
	at91_set_gpio_output(AT91_PIN_PB22, 1);	
	at91_add_device_mmc(0, &ek_mmc_data);
#endif
	
	platform_device_register(&ek_flash);
	
	at91_gpio_leds(ek_leds, ARRAY_SIZE(ek_leds));
	

}

MACHINE_START(AT91RM9200EK, "Atmel AT91RM9200-EK")
	
	.phys_io	= AT91_BASE_SYS,
	.io_pg_offst	= (AT91_VA_BASE_SYS >> 18) & 0xfffc,
	.boot_params	= AT91_SDRAM_BASE + 0x100,
	.timer		= &at91rm9200_timer,
	.map_io		= ek_map_io,
	.init_irq	= ek_init_irq,
	.init_machine	= ek_board_init,
MACHINE_END
