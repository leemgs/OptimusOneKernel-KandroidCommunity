

#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
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
#include <mach/at91sam9_smc.h>
#include <mach/at91sam9260_matrix.h>

#include "sam9_smc.h"
#include "generic.h"

static void __init cpu9krea_map_io(void)
{
	
	at91sam9260_initialize(18432000);

	
	at91_register_uart(0, 0, 0);

	
	at91_register_uart(AT91SAM9260_ID_US0, 1, ATMEL_UART_CTS |
		ATMEL_UART_RTS | ATMEL_UART_DTR | ATMEL_UART_DSR |
		ATMEL_UART_DCD | ATMEL_UART_RI);

	
	at91_register_uart(AT91SAM9260_ID_US1, 2, ATMEL_UART_CTS |
		ATMEL_UART_RTS);

	
	at91_register_uart(AT91SAM9260_ID_US2, 3, ATMEL_UART_CTS |
		ATMEL_UART_RTS);

	
	at91_register_uart(AT91SAM9260_ID_US3, 4, 0);

	
	at91_register_uart(AT91SAM9260_ID_US4, 5, 0);

	
	at91_register_uart(AT91SAM9260_ID_US5, 6, 0);

	
	at91_set_serial_console(0);
}

static void __init cpu9krea_init_irq(void)
{
	at91sam9260_init_interrupts(NULL);
}


static struct at91_usbh_data __initdata cpu9krea_usbh_data = {
	.ports		= 2,
};


static struct at91_udc_data __initdata cpu9krea_udc_data = {
	.vbus_pin	= AT91_PIN_PC8,
	.pullup_pin	= 0,		
};


static struct at91_eth_data __initdata cpu9krea_macb_data = {
	.is_rmii	= 1,
};


static struct atmel_nand_data __initdata cpu9krea_nand_data = {
	.ale		= 21,
	.cle		= 22,
	.rdy_pin	= AT91_PIN_PC13,
	.enable_pin	= AT91_PIN_PC14,
	.bus_width_16	= 0,
};

#ifdef CONFIG_MACH_CPU9260
static struct sam9_smc_config __initdata cpu9krea_nand_smc_config = {
	.ncs_read_setup		= 0,
	.nrd_setup		= 1,
	.ncs_write_setup	= 0,
	.nwe_setup		= 1,

	.ncs_read_pulse		= 3,
	.nrd_pulse		= 3,
	.ncs_write_pulse	= 3,
	.nwe_pulse		= 3,

	.read_cycle		= 5,
	.write_cycle		= 5,

	.mode			= AT91_SMC_READMODE | AT91_SMC_WRITEMODE
		| AT91_SMC_EXNWMODE_DISABLE | AT91_SMC_DBW_8,
	.tdf_cycles		= 2,
};
#else
static struct sam9_smc_config __initdata cpu9krea_nand_smc_config = {
	.ncs_read_setup		= 0,
	.nrd_setup		= 2,
	.ncs_write_setup	= 0,
	.nwe_setup		= 2,

	.ncs_read_pulse		= 4,
	.nrd_pulse		= 4,
	.ncs_write_pulse	= 4,
	.nwe_pulse		= 4,

	.read_cycle		= 7,
	.write_cycle		= 7,

	.mode			= AT91_SMC_READMODE | AT91_SMC_WRITEMODE
		| AT91_SMC_EXNWMODE_DISABLE | AT91_SMC_DBW_8,
	.tdf_cycles		= 3,
};
#endif

static void __init cpu9krea_add_device_nand(void)
{
	sam9_smc_configure(3, &cpu9krea_nand_smc_config);
	at91_add_device_nand(&cpu9krea_nand_data);
}


static struct physmap_flash_data cpuat9260_nor_data = {
	.width		= 2,
};

#define NOR_BASE	AT91_CHIPSELECT_0
#define NOR_SIZE	SZ_64M

static struct resource nor_flash_resources[] = {
	{
		.start	= NOR_BASE,
		.end	= NOR_BASE + NOR_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	}
};

static struct platform_device cpu9krea_nor_flash = {
	.name		= "physmap-flash",
	.id		= 0,
	.dev		= {
		.platform_data	= &cpuat9260_nor_data,
	},
	.resource	= nor_flash_resources,
	.num_resources	= ARRAY_SIZE(nor_flash_resources),
};

#ifdef CONFIG_MACH_CPU9260
static struct sam9_smc_config __initdata cpu9krea_nor_smc_config = {
	.ncs_read_setup		= 0,
	.nrd_setup		= 1,
	.ncs_write_setup	= 0,
	.nwe_setup		= 1,

	.ncs_read_pulse		= 10,
	.nrd_pulse		= 10,
	.ncs_write_pulse	= 6,
	.nwe_pulse		= 6,

	.read_cycle		= 12,
	.write_cycle		= 8,

	.mode			= AT91_SMC_READMODE | AT91_SMC_WRITEMODE
			| AT91_SMC_EXNWMODE_DISABLE | AT91_SMC_BAT_WRITE
			| AT91_SMC_DBW_16,
	.tdf_cycles		= 2,
};
#else
static struct sam9_smc_config __initdata cpu9krea_nor_smc_config = {
	.ncs_read_setup		= 0,
	.nrd_setup		= 1,
	.ncs_write_setup	= 0,
	.nwe_setup		= 1,

	.ncs_read_pulse		= 13,
	.nrd_pulse		= 13,
	.ncs_write_pulse	= 8,
	.nwe_pulse		= 8,

	.read_cycle		= 15,
	.write_cycle		= 10,

	.mode			= AT91_SMC_READMODE | AT91_SMC_WRITEMODE
			| AT91_SMC_EXNWMODE_DISABLE | AT91_SMC_BAT_WRITE
			| AT91_SMC_DBW_16,
	.tdf_cycles		= 2,
};
#endif

static __init void cpu9krea_add_device_nor(void)
{
	unsigned long csa;

	csa = at91_sys_read(AT91_MATRIX_EBICSA);
	at91_sys_write(AT91_MATRIX_EBICSA, csa | AT91_MATRIX_VDDIOMSEL_3_3V);

	
	sam9_smc_configure(0, &cpu9krea_nor_smc_config);

	platform_device_register(&cpu9krea_nor_flash);
}


static struct gpio_led cpu9krea_leds[] = {
	{	
		.name			= "LED1",
		.gpio			= AT91_PIN_PC11,
		.active_low		= 1,
		.default_trigger	= "timer",
	},
	{	
		.name			= "LED2",
		.gpio			= AT91_PIN_PC12,
		.active_low		= 1,
		.default_trigger	= "heartbeat",
	},
	{	
		.name			= "LED3",
		.gpio			= AT91_PIN_PC7,
		.active_low		= 1,
		.default_trigger	= "none",
	},
	{	
		.name			= "LED4",
		.gpio			= AT91_PIN_PC9,
		.active_low		= 1,
		.default_trigger	= "none",
	}
};

static struct i2c_board_info __initdata cpu9krea_i2c_devices[] = {
	{
		I2C_BOARD_INFO("rtc-ds1307", 0x68),
		.type	= "ds1339",
	},
};


#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
static struct gpio_keys_button cpu9krea_buttons[] = {
	{
		.gpio		= AT91_PIN_PC3,
		.code		= BTN_0,
		.desc		= "BP1",
		.active_low	= 1,
		.wakeup		= 1,
	},
	{
		.gpio		= AT91_PIN_PB20,
		.code		= BTN_1,
		.desc		= "BP2",
		.active_low	= 1,
		.wakeup		= 1,
	}
};

static struct gpio_keys_platform_data cpu9krea_button_data = {
	.buttons	= cpu9krea_buttons,
	.nbuttons	= ARRAY_SIZE(cpu9krea_buttons),
};

static struct platform_device cpu9krea_button_device = {
	.name		= "gpio-keys",
	.id		= -1,
	.num_resources	= 0,
	.dev		= {
		.platform_data	= &cpu9krea_button_data,
	}
};

static void __init cpu9krea_add_device_buttons(void)
{
	at91_set_gpio_input(AT91_PIN_PC3, 1);	
	at91_set_deglitch(AT91_PIN_PC3, 1);
	at91_set_gpio_input(AT91_PIN_PB20, 1);	
	at91_set_deglitch(AT91_PIN_PB20, 1);

	platform_device_register(&cpu9krea_button_device);
}
#else
static void __init cpu9krea_add_device_buttons(void)
{
}
#endif


static struct at91_mmc_data __initdata cpu9krea_mmc_data = {
	.slot_b		= 0,
	.wire4		= 1,
	.det_pin	= AT91_PIN_PA29,
};

static void __init cpu9krea_board_init(void)
{
	
	cpu9krea_add_device_nor();
	
	at91_add_device_serial();
	
	at91_add_device_usbh(&cpu9krea_usbh_data);
	
	at91_add_device_udc(&cpu9krea_udc_data);
	
	cpu9krea_add_device_nand();
	
	at91_add_device_eth(&cpu9krea_macb_data);
	
	at91_add_device_mmc(0, &cpu9krea_mmc_data);
	
	at91_add_device_i2c(cpu9krea_i2c_devices,
		ARRAY_SIZE(cpu9krea_i2c_devices));
	
	at91_gpio_leds(cpu9krea_leds, ARRAY_SIZE(cpu9krea_leds));
	
	cpu9krea_add_device_buttons();
}

#ifdef CONFIG_MACH_CPU9260
MACHINE_START(CPUAT9260, "Eukrea CPU9260")
#else
MACHINE_START(CPUAT9G20, "Eukrea CPU9G20")
#endif
	
	.phys_io	= AT91_BASE_SYS,
	.io_pg_offst	= (AT91_VA_BASE_SYS >> 18) & 0xfffc,
	.boot_params	= AT91_SDRAM_BASE + 0x100,
	.timer		= &at91sam926x_timer,
	.map_io		= cpu9krea_map_io,
	.init_irq	= cpu9krea_init_irq,
	.init_machine	= cpu9krea_board_init,
MACHINE_END
