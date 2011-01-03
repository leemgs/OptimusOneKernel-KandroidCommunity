

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

#include "generic.h"



static struct at91_uart_config __initdata kafa_uart_config = {
	.console_tty	= 0,				
	.nr_tty		= 2,
	.tty_map	= { 4, 0, -1, -1, -1 }		
};

static void __init kafa_map_io(void)
{
	
	at91rm9200_initialize(18432000, AT91RM9200_PQFP);

	
	at91_init_leds(AT91_PIN_PB4, AT91_PIN_PB4);

	
	at91_init_serial(&kafa_uart_config);
}

static void __init kafa_init_irq(void)
{
	at91rm9200_init_interrupts(NULL);
}

static struct at91_eth_data __initdata kafa_eth_data = {
	.phy_irq_pin	= AT91_PIN_PC4,
	.is_rmii	= 0,
};

static struct at91_usbh_data __initdata kafa_usbh_data = {
	.ports		= 1,
};

static struct at91_udc_data __initdata kafa_udc_data = {
	.vbus_pin	= AT91_PIN_PB6,
	.pullup_pin	= AT91_PIN_PB7,
};

static void __init kafa_board_init(void)
{
	
	at91_add_device_serial();
	
	at91_add_device_eth(&kafa_eth_data);
	
	at91_add_device_usbh(&kafa_usbh_data);
	
	at91_add_device_udc(&kafa_udc_data);
	
	at91_add_device_i2c(NULL, 0);
	
	at91_add_device_spi(NULL, 0);
}

MACHINE_START(KAFA, "Sperry-Sun KAFA")
	
	.phys_io	= AT91_BASE_SYS,
	.io_pg_offst	= (AT91_VA_BASE_SYS >> 18) & 0xfffc,
	.boot_params	= AT91_SDRAM_BASE + 0x100,
	.timer		= &at91rm9200_timer,
	.map_io		= kafa_map_io,
	.init_irq	= kafa_init_irq,
	.init_machine	= kafa_board_init,
MACHINE_END
