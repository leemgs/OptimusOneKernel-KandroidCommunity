

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/i2c.h>
#include <linux/i2c/at24.h>
#include <linux/input.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>

#include <mach/control.h>
#include <mach/gpio.h>
#include <mach/mux.h>
#include <mach/usb.h>
#include <mach/board.h>
#include <mach/common.h>
#include <mach/keypad.h>
#include <mach/menelaus.h>
#include <mach/dma.h>
#include <mach/gpmc.h>

#define H4_FLASH_CS	0
#define H4_SMC91X_CS	1

#define H4_ETHR_GPIO_IRQ		92

static unsigned int row_gpios[6] = { 88, 89, 124, 11, 6, 96 };
static unsigned int col_gpios[7] = { 90, 91, 100, 36, 12, 97, 98 };

static int h4_keymap[] = {
	KEY(0, 0, KEY_LEFT),
	KEY(0, 1, KEY_RIGHT),
	KEY(0, 2, KEY_A),
	KEY(0, 3, KEY_B),
	KEY(0, 4, KEY_C),
	KEY(1, 0, KEY_DOWN),
	KEY(1, 1, KEY_UP),
	KEY(1, 2, KEY_E),
	KEY(1, 3, KEY_F),
	KEY(1, 4, KEY_G),
	KEY(2, 0, KEY_ENTER),
	KEY(2, 1, KEY_I),
	KEY(2, 2, KEY_J),
	KEY(2, 3, KEY_K),
	KEY(2, 4, KEY_3),
	KEY(3, 0, KEY_M),
	KEY(3, 1, KEY_N),
	KEY(3, 2, KEY_O),
	KEY(3, 3, KEY_P),
	KEY(3, 4, KEY_Q),
	KEY(4, 0, KEY_R),
	KEY(4, 1, KEY_4),
	KEY(4, 2, KEY_T),
	KEY(4, 3, KEY_U),
	KEY(4, 4, KEY_ENTER),
	KEY(5, 0, KEY_V),
	KEY(5, 1, KEY_W),
	KEY(5, 2, KEY_L),
	KEY(5, 3, KEY_S),
	KEY(5, 4, KEY_ENTER),
	0
};

static struct mtd_partition h4_partitions[] = {
	
	{
	      .name		= "bootloader",
	      .offset		= 0,
	      .size		= SZ_128K,
	      .mask_flags	= MTD_WRITEABLE, 
	},
	
	{
	      .name		= "params",
	      .offset		= MTDPART_OFS_APPEND,
	      .size		= SZ_128K,
	      .mask_flags	= 0,
	},
	
	{
	      .name		= "kernel",
	      .offset		= MTDPART_OFS_APPEND,
	      .size		= SZ_2M,
	      .mask_flags	= 0
	},
	
	{
	      .name		= "filesystem",
	      .offset		= MTDPART_OFS_APPEND,
	      .size		= MTDPART_SIZ_FULL,
	      .mask_flags	= 0
	}
};

static struct flash_platform_data h4_flash_data = {
	.map_name	= "cfi_probe",
	.width		= 2,
	.parts		= h4_partitions,
	.nr_parts	= ARRAY_SIZE(h4_partitions),
};

static struct resource h4_flash_resource = {
	.flags		= IORESOURCE_MEM,
};

static struct platform_device h4_flash_device = {
	.name		= "omapflash",
	.id		= 0,
	.dev		= {
		.platform_data	= &h4_flash_data,
	},
	.num_resources	= 1,
	.resource	= &h4_flash_resource,
};

static struct omap_kp_platform_data h4_kp_data = {
	.rows		= 6,
	.cols		= 7,
	.keymap 	= h4_keymap,
	.keymapsize 	= ARRAY_SIZE(h4_keymap),
	.rep		= 1,
	.row_gpios 	= row_gpios,
	.col_gpios 	= col_gpios,
};

static struct platform_device h4_kp_device = {
	.name		= "omap-keypad",
	.id		= -1,
	.dev		= {
		.platform_data = &h4_kp_data,
	},
};

static struct platform_device h4_lcd_device = {
	.name		= "lcd_h4",
	.id		= -1,
};

static struct platform_device *h4_devices[] __initdata = {
	&h4_flash_device,
	&h4_kp_device,
	&h4_lcd_device,
};


static u32 get_sysboot_value(void)
{
	return (omap_ctrl_readl(OMAP24XX_CONTROL_STATUS) &
		(OMAP2_SYSBOOT_5_MASK | OMAP2_SYSBOOT_4_MASK |
		 OMAP2_SYSBOOT_3_MASK | OMAP2_SYSBOOT_2_MASK |
		 OMAP2_SYSBOOT_1_MASK | OMAP2_SYSBOOT_0_MASK));
}


static u32 is_gpmc_muxed(void)
{
	u32 mux;
	mux = get_sysboot_value();
	if ((mux & 0xF) == 0xd)
		return 1;	
	if (mux & 0x2)		
		return 1;
	else
		return 0;
}

static inline void __init h4_init_debug(void)
{
	int eth_cs;
	unsigned long cs_mem_base;
	unsigned int muxed, rate;
	struct clk *gpmc_fck;

	eth_cs	= H4_SMC91X_CS;

	gpmc_fck = clk_get(NULL, "gpmc_fck");	
	if (IS_ERR(gpmc_fck)) {
		WARN_ON(1);
		return;
	}

	clk_enable(gpmc_fck);
	rate = clk_get_rate(gpmc_fck);
	clk_disable(gpmc_fck);
	clk_put(gpmc_fck);

	if (is_gpmc_muxed())
		muxed = 0x200;
	else
		muxed = 0;

	
	gpmc_cs_write_reg(eth_cs, GPMC_CS_CONFIG1,
			  0x00011000 | muxed);

	if (rate >= 160000000) {
		gpmc_cs_write_reg(eth_cs, GPMC_CS_CONFIG2, 0x001f1f01);
		gpmc_cs_write_reg(eth_cs, GPMC_CS_CONFIG3, 0x00080803);
		gpmc_cs_write_reg(eth_cs, GPMC_CS_CONFIG4, 0x1c0b1c0a);
		gpmc_cs_write_reg(eth_cs, GPMC_CS_CONFIG5, 0x041f1F1F);
		gpmc_cs_write_reg(eth_cs, GPMC_CS_CONFIG6, 0x000004C4);
	} else if (rate >= 130000000) {
		gpmc_cs_write_reg(eth_cs, GPMC_CS_CONFIG2, 0x001f1f00);
		gpmc_cs_write_reg(eth_cs, GPMC_CS_CONFIG3, 0x00080802);
		gpmc_cs_write_reg(eth_cs, GPMC_CS_CONFIG4, 0x1C091C09);
		gpmc_cs_write_reg(eth_cs, GPMC_CS_CONFIG5, 0x041f1F1F);
		gpmc_cs_write_reg(eth_cs, GPMC_CS_CONFIG6, 0x000004C4);
	} else {
		gpmc_cs_write_reg(eth_cs, GPMC_CS_CONFIG2, 0x001f1f00);
		gpmc_cs_write_reg(eth_cs, GPMC_CS_CONFIG3, 0x00080802);
		gpmc_cs_write_reg(eth_cs, GPMC_CS_CONFIG4, 0x1C091C09);
		gpmc_cs_write_reg(eth_cs, GPMC_CS_CONFIG5, 0x031A1F1F);
		gpmc_cs_write_reg(eth_cs, GPMC_CS_CONFIG6, 0x000003C2);
	}

	if (gpmc_cs_request(eth_cs, SZ_16M, &cs_mem_base) < 0) {
		printk(KERN_ERR "Failed to request GPMC mem for smc91x\n");
		goto out;
	}

	udelay(100);

	omap_cfg_reg(M15_24XX_GPIO92);
	if (debug_card_init(cs_mem_base, H4_ETHR_GPIO_IRQ) < 0)
		gpmc_cs_free(eth_cs);

out:
	clk_disable(gpmc_fck);
	clk_put(gpmc_fck);
}

static void __init h4_init_flash(void)
{
	unsigned long base;

	if (gpmc_cs_request(H4_FLASH_CS, SZ_64M, &base) < 0) {
		printk("Can't request GPMC CS for flash\n");
		return;
	}
	h4_flash_resource.start	= base;
	h4_flash_resource.end	= base + SZ_64M - 1;
}

static struct omap_lcd_config h4_lcd_config __initdata = {
	.ctrl_name	= "internal",
};

static struct omap_usb_config h4_usb_config __initdata = {
#ifdef	CONFIG_MACH_OMAP2_H4_USB1
	
	.pins[1]	= 4,
#endif

#ifdef	CONFIG_MACH_OMAP_H4_OTG
	
	.otg		= 1,
	.pins[0]	= 4,
#ifdef	CONFIG_USB_GADGET_OMAP
	
	.hmc_mode	= 0x14,	
#elif	defined(CONFIG_USB_OHCI_HCD) || defined(CONFIG_USB_OHCI_HCD_MODULE)
	
	.hmc_mode	= 0x11,	
#endif	

#else
	
	.register_dev	= 1,
	.pins[0]	= 3,
	
	.hmc_mode	= 0x00,		
#endif
};

static struct omap_board_config_kernel h4_config[] = {
	{ OMAP_TAG_LCD,		&h4_lcd_config },
};

static void __init omap_h4_init_irq(void)
{
	omap_board_config = h4_config;
	omap_board_config_size = ARRAY_SIZE(h4_config);
	omap2_init_common_hw(NULL, NULL);
	omap_init_irq();
	omap_gpio_init();
	h4_init_flash();
}

static struct at24_platform_data m24c01 = {
	.byte_len	= SZ_1K / 8,
	.page_size	= 16,
};

static struct i2c_board_info __initdata h4_i2c_board_info[] = {
	{
		I2C_BOARD_INFO("isp1301_omap", 0x2d),
		.irq		= OMAP_GPIO_IRQ(125),
	},
	{	
		I2C_BOARD_INFO("24c01", 0x52),
		.platform_data	= &m24c01,
	},
	{	
		I2C_BOARD_INFO("24c01", 0x57),
		.platform_data	= &m24c01,
	},
};

static void __init omap_h4_init(void)
{
	
#if defined(CONFIG_OMAP_IR) || defined(CONFIG_OMAP_IR_MODULE)
	omap_cfg_reg(K15_24XX_UART3_TX);
	omap_cfg_reg(K14_24XX_UART3_RX);
#endif

#if defined(CONFIG_KEYBOARD_OMAP) || defined(CONFIG_KEYBOARD_OMAP_MODULE)
	if (omap_has_menelaus()) {
		row_gpios[5] = 0;
		col_gpios[2] = 15;
		col_gpios[6] = 18;
	}
#endif

	i2c_register_board_info(1, h4_i2c_board_info,
			ARRAY_SIZE(h4_i2c_board_info));

	platform_add_devices(h4_devices, ARRAY_SIZE(h4_devices));
	omap_usb_init(&h4_usb_config);
	omap_serial_init();
}

static void __init omap_h4_map_io(void)
{
	omap2_set_globals_242x();
	omap2_map_common_io();
}

MACHINE_START(OMAP_H4, "OMAP2420 H4 board")
	
	.phys_io	= 0x48000000,
	.io_pg_offst	= ((0xd8000000) >> 18) & 0xfffc,
	.boot_params	= 0x80000100,
	.map_io		= omap_h4_map_io,
	.init_irq	= omap_h4_init_irq,
	.init_machine	= omap_h4_init,
	.timer		= &omap_timer,
MACHINE_END
