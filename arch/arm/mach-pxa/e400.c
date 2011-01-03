

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/mfd/t7l66xb.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>

#include <asm/setup.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>

#include <mach/pxa25x.h>
#include <mach/eseries-gpio.h>
#include <mach/pxafb.h>
#include <mach/udc.h>
#include <mach/irqs.h>

#include "generic.h"
#include "eseries.h"
#include "clock.h"



static struct pxafb_mode_info e400_pxafb_mode_info = {
	.pixclock       = 140703,
	.xres           = 240,
	.yres           = 320,
	.bpp            = 16,
	.hsync_len      = 4,
	.left_margin    = 28,
	.right_margin   = 8,
	.vsync_len      = 3,
	.upper_margin   = 5,
	.lower_margin   = 6,
	.sync           = 0,
};

static struct pxafb_mach_info e400_pxafb_mach_info = {
	.modes          = &e400_pxafb_mode_info,
	.num_modes      = 1,
	.lcd_conn	= LCD_COLOR_TFT_16BPP,
	.lccr3          = 0,
	.pxafb_backlight_power  = NULL,
};



static unsigned long e400_pin_config[] __initdata = {
	
	GPIO15_nCS_1,   
	GPIO80_nCS_4,   

	
	GPIO12_32KHz,

	
	GPIO42_BTUART_RXD,
	GPIO43_BTUART_TXD,
	GPIO44_BTUART_CTS,

	
	GPIO19_GPIO, 
	GPIO45_GPIO, 

	
	GPIO0_GPIO | WAKEUP_ON_EDGE_RISE,
};



static struct mtd_partition partition_a = {
	.name = "Internal NAND flash",
	.offset =  0,
	.size =  MTDPART_SIZ_FULL,
};

static uint8_t scan_ff_pattern[] = { 0xff, 0xff };

static struct nand_bbt_descr e400_t7l66xb_nand_bbt = {
	.options = 0,
	.offs = 4,
	.len = 2,
	.pattern = scan_ff_pattern
};

static struct tmio_nand_data e400_t7l66xb_nand_config = {
	.num_partitions = 1,
	.partition = &partition_a,
	.badblock_pattern = &e400_t7l66xb_nand_bbt,
};

static struct t7l66xb_platform_data e400_t7l66xb_info = {
	.irq_base 		= IRQ_BOARD_START,
	.enable                 = &eseries_tmio_enable,
	.suspend                = &eseries_tmio_suspend,
	.resume                 = &eseries_tmio_resume,

	.nand_data              = &e400_t7l66xb_nand_config,
};

static struct platform_device e400_t7l66xb_device = {
	.name           = "t7l66xb",
	.id             = -1,
	.dev            = {
		.platform_data = &e400_t7l66xb_info,
	},
	.num_resources = 2,
	.resource      = eseries_tmio_resources,
};



static struct platform_device *devices[] __initdata = {
	&e400_t7l66xb_device,
};

static void __init e400_init(void)
{
	pxa2xx_mfp_config(ARRAY_AND_SIZE(e400_pin_config));
	
	eseries_register_clks();
	eseries_get_tmio_gpios();
	set_pxa_fb_info(&e400_pxafb_mach_info);
	platform_add_devices(devices, ARRAY_SIZE(devices));
	pxa_set_udc_info(&e7xx_udc_mach_info);
}

MACHINE_START(E400, "Toshiba e400")
	
	.phys_io	= 0x40000000,
	.io_pg_offst	= (io_p2v(0x40000000) >> 18) & 0xfffc,
	.boot_params	= 0xa0000100,
	.map_io		= pxa_map_io,
	.init_irq	= pxa25x_init_irq,
	.fixup		= eseries_fixup,
	.init_machine	= e400_init,
	.timer		= &pxa_timer,
MACHINE_END

