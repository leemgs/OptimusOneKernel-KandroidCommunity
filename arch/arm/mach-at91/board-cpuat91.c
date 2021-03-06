

#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/plat-ram.h>

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

static struct gpio_led cpuat91_leds[] = {
	{
		.name			= "led1",
		.default_trigger	= "heartbeat",
		.active_low		= 1,
		.gpio			= AT91_PIN_PC0,
	},
};

static void __init cpuat91_map_io(void)
{
	
	at91rm9200_initialize(18432000, AT91RM9200_PQFP);

	
	at91_register_uart(0, 0, 0);

	
	at91_register_uart(AT91RM9200_ID_US0, 1, ATMEL_UART_CTS |
		ATMEL_UART_RTS);

	
	at91_register_uart(AT91RM9200_ID_US1, 2, ATMEL_UART_CTS |
		ATMEL_UART_RTS | ATMEL_UART_DTR | ATMEL_UART_DSR |
		ATMEL_UART_DCD | ATMEL_UART_RI);

	
	at91_register_uart(AT91RM9200_ID_US2, 3, 0);

	
	at91_register_uart(AT91RM9200_ID_US3, 4, ATMEL_UART_CTS |
		ATMEL_UART_RTS);

	
	at91_set_serial_console(0);
}

static void __init cpuat91_init_irq(void)
{
	at91rm9200_init_interrupts(NULL);
}

static struct at91_eth_data __initdata cpuat91_eth_data = {
	.is_rmii	= 1,
};

static struct at91_usbh_data __initdata cpuat91_usbh_data = {
	.ports		= 1,
};

static struct at91_udc_data __initdata cpuat91_udc_data = {
	.vbus_pin	= AT91_PIN_PC15,
	.pullup_pin	= AT91_PIN_PC14,
};

static struct at91_mmc_data __initdata cpuat91_mmc_data = {
	.det_pin	= AT91_PIN_PC2,
	.wire4		= 1,
};

static struct physmap_flash_data cpuat91_flash_data = {
	.width		= 2,
};

static struct resource cpuat91_flash_resource = {
	.start		= AT91_CHIPSELECT_0,
	.end		= AT91_CHIPSELECT_0 + SZ_16M - 1,
	.flags		= IORESOURCE_MEM,
};

static struct platform_device cpuat91_norflash = {
	.name		= "physmap-flash",
	.id		= 0,
	.dev	= {
		.platform_data	= &cpuat91_flash_data,
	},
	.resource	= &cpuat91_flash_resource,
	.num_resources	= 1,
};

#ifdef CONFIG_MTD_PLATRAM
struct platdata_mtd_ram at91_sram_pdata = {
	.mapname	= "SRAM",
	.bankwidth	= 2,
};

static struct resource at91_sram_resource[] = {
	[0] = {
		.start = AT91RM9200_SRAM_BASE,
		.end   = AT91RM9200_SRAM_BASE + AT91RM9200_SRAM_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
};

static struct platform_device at91_sram = {
	.name		= "mtd-ram",
	.id		= 0,
	.resource	= at91_sram_resource,
	.num_resources	= ARRAY_SIZE(at91_sram_resource),
	.dev	= {
		.platform_data = &at91_sram_pdata,
	},
};
#endif 

static struct platform_device *platform_devices[] __initdata = {
	&cpuat91_norflash,
#ifdef CONFIG_MTD_PLATRAM
	&at91_sram,
#endif 
};

static void __init cpuat91_board_init(void)
{
	
	at91_add_device_serial();
	
	at91_gpio_leds(cpuat91_leds, ARRAY_SIZE(cpuat91_leds));
	
	at91_add_device_eth(&cpuat91_eth_data);
	
	at91_add_device_usbh(&cpuat91_usbh_data);
	
	at91_add_device_udc(&cpuat91_udc_data);
	
	at91_add_device_mmc(0, &cpuat91_mmc_data);
	
	at91_add_device_i2c(NULL, 0);
	
	platform_add_devices(platform_devices, ARRAY_SIZE(platform_devices));
}

MACHINE_START(CPUAT91, "Eukrea")
	
	.phys_io	= AT91_BASE_SYS,
	.io_pg_offst	= (AT91_VA_BASE_SYS >> 18) & 0xfffc,
	.boot_params	= AT91_SDRAM_BASE + 0x100,
	.timer		= &at91rm9200_timer,
	.map_io		= cpuat91_map_io,
	.init_irq	= cpuat91_init_irq,
	.init_machine	= cpuat91_board_init,
MACHINE_END
