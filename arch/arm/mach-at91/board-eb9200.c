

#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/device.h>

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


static void __init eb9200_map_io(void)
{
	
	at91rm9200_initialize(18432000, AT91RM9200_BGA);

	
	at91_register_uart(0, 0, 0);

	
	at91_register_uart(AT91RM9200_ID_US1, 1, ATMEL_UART_CTS | ATMEL_UART_RTS
			| ATMEL_UART_DTR | ATMEL_UART_DSR | ATMEL_UART_DCD
			| ATMEL_UART_RI);

	
	at91_register_uart(AT91RM9200_ID_US2, 2, 0);

	
	at91_set_serial_console(0);
}

static void __init eb9200_init_irq(void)
{
	at91rm9200_init_interrupts(NULL);
}

static struct at91_eth_data __initdata eb9200_eth_data = {
	.phy_irq_pin	= AT91_PIN_PC4,
	.is_rmii	= 1,
};

static struct at91_usbh_data __initdata eb9200_usbh_data = {
	.ports		= 2,
};

static struct at91_udc_data __initdata eb9200_udc_data = {
	.vbus_pin	= AT91_PIN_PD4,
	.pullup_pin	= AT91_PIN_PD5,
};

static struct at91_cf_data __initdata eb9200_cf_data = {
	.det_pin	= AT91_PIN_PB0,
	.rst_pin	= AT91_PIN_PC5,
	
	
};

static struct at91_mmc_data __initdata eb9200_mmc_data = {
	.slot_b		= 0,
	.wire4		= 1,
};

static struct i2c_board_info __initdata eb9200_i2c_devices[] = {
	{
		I2C_BOARD_INFO("24c512", 0x50),
	},
};


static void __init eb9200_board_init(void)
{
	
	at91_add_device_serial();
	
	at91_add_device_eth(&eb9200_eth_data);
	
	at91_add_device_usbh(&eb9200_usbh_data);
	
	at91_add_device_udc(&eb9200_udc_data);
	
	at91_add_device_i2c(eb9200_i2c_devices, ARRAY_SIZE(eb9200_i2c_devices));
	
	at91_add_device_cf(&eb9200_cf_data);
	
	at91_add_device_spi(NULL, 0);
	
	
	at91_add_device_mmc(0, &eb9200_mmc_data);
}

MACHINE_START(ATEB9200, "Embest ATEB9200")
	.phys_io	= AT91_BASE_SYS,
	.io_pg_offst	= (AT91_VA_BASE_SYS >> 18) & 0xfffc,
	.boot_params	= AT91_SDRAM_BASE + 0x100,
	.timer		= &at91rm9200_timer,
	.map_io		= eb9200_map_io,
	.init_irq	= eb9200_init_irq,
	.init_machine	= eb9200_board_init,
MACHINE_END
