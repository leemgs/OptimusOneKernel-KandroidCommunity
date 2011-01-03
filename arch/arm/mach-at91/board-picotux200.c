

#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/mtd/physmap.h>

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



static struct at91_uart_config __initdata picotux200_uart_config = {
	.console_tty	= 0,				
	.nr_tty		= 2,
	.tty_map	= { 4, 1, -1, -1, -1 }		
};

static void __init picotux200_map_io(void)
{
	
	at91rm9200_initialize(18432000, AT91RM9200_BGA);

	
	at91_init_serial(&picotux200_uart_config);
}

static void __init picotux200_init_irq(void)
{
	at91rm9200_init_interrupts(NULL);
}

static struct at91_eth_data __initdata picotux200_eth_data = {
	.phy_irq_pin	= AT91_PIN_PC4,
	.is_rmii	= 1,
};

static struct at91_usbh_data __initdata picotux200_usbh_data = {
	.ports		= 1,
};






static struct at91_mmc_data __initdata picotux200_mmc_data = {
	.det_pin	= AT91_PIN_PB27,
	.slot_b		= 0,
	.wire4		= 1,
	.wp_pin		= AT91_PIN_PA17,
};
















#define PICOTUX200_FLASH_BASE	AT91_CHIPSELECT_0
#define PICOTUX200_FLASH_SIZE	SZ_4M

static struct physmap_flash_data picotux200_flash_data = {
	.width	= 2,
};

static struct resource picotux200_flash_resource = {
	.start		= PICOTUX200_FLASH_BASE,
	.end		= PICOTUX200_FLASH_BASE + PICOTUX200_FLASH_SIZE - 1,
	.flags		= IORESOURCE_MEM,
};

static struct platform_device picotux200_flash = {
	.name		= "physmap-flash",
	.id		= 0,
	.dev		= {
				.platform_data	= &picotux200_flash_data,
			},
	.resource	= &picotux200_flash_resource,
	.num_resources	= 1,
};

static void __init picotux200_board_init(void)
{
	
	at91_add_device_serial();
	
	at91_add_device_eth(&picotux200_eth_data);
	
	at91_add_device_usbh(&picotux200_usbh_data);
	
	
	
	
	at91_add_device_i2c(NULL, 0);
	
	
#ifdef CONFIG_MTD_AT91_DATAFLASH_CARD
	
	at91_set_gpio_output(AT91_PIN_PB22, 0);
#else
	
	at91_set_gpio_output(AT91_PIN_PB22, 1);	
	at91_add_device_mmc(0, &picotux200_mmc_data);
#endif
	
	platform_device_register(&picotux200_flash);
}

MACHINE_START(PICOTUX2XX, "picotux 200")
	
	.phys_io	= AT91_BASE_SYS,
	.io_pg_offst	= (AT91_VA_BASE_SYS >> 18) & 0xfffc,
	.boot_params	= AT91_SDRAM_BASE + 0x100,
	.timer		= &at91rm9200_timer,
	.map_io		= picotux200_map_io,
	.init_irq	= picotux200_init_irq,
	.init_machine	= picotux200_board_init,
MACHINE_END
