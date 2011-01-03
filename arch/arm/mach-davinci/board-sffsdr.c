

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>

#include <linux/i2c.h>
#include <linux/i2c/at24.h>
#include <linux/etherdevice.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>
#include <linux/io.h>

#include <asm/setup.h>
#include <asm/mach-types.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>

#include <mach/dm644x.h>
#include <mach/common.h>
#include <mach/i2c.h>
#include <mach/serial.h>
#include <mach/psc.h>
#include <mach/mux.h>

#define SFFSDR_PHY_MASK		(0x2)
#define SFFSDR_MDIO_FREQUENCY	(2200000) 

#define DAVINCI_ASYNC_EMIF_CONTROL_BASE   0x01e00000
#define DAVINCI_ASYNC_EMIF_DATA_CE0_BASE  0x02000000

struct mtd_partition davinci_sffsdr_nandflash_partition[] = {
	
	{
		.name		= "Linux Kernel",
		.offset		= 32 * SZ_128K,
		.size		= 16 * SZ_128K, 
		.mask_flags	= MTD_WRITEABLE, 
	},
	{
		.name		= "Linux ROOT",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 256 * SZ_128K, 
		.mask_flags	= 0, 
	},
};

static struct flash_platform_data davinci_sffsdr_nandflash_data = {
	.parts		= davinci_sffsdr_nandflash_partition,
	.nr_parts	= ARRAY_SIZE(davinci_sffsdr_nandflash_partition),
};

static struct resource davinci_sffsdr_nandflash_resource[] = {
	{
		.start		= DAVINCI_ASYNC_EMIF_DATA_CE0_BASE,
		.end		= DAVINCI_ASYNC_EMIF_DATA_CE0_BASE + SZ_16M - 1,
		.flags		= IORESOURCE_MEM,
	}, {
		.start		= DAVINCI_ASYNC_EMIF_CONTROL_BASE,
		.end		= DAVINCI_ASYNC_EMIF_CONTROL_BASE + SZ_4K - 1,
		.flags		= IORESOURCE_MEM,
	},
};

static struct platform_device davinci_sffsdr_nandflash_device = {
	.name		= "davinci_nand", 
	.id		= 0,
	.dev		= {
		.platform_data	= &davinci_sffsdr_nandflash_data,
	},
	.num_resources	= ARRAY_SIZE(davinci_sffsdr_nandflash_resource),
	.resource	= davinci_sffsdr_nandflash_resource,
};

static struct emac_platform_data sffsdr_emac_pdata = {
	.phy_mask	= SFFSDR_PHY_MASK,
	.mdio_max_freq	= SFFSDR_MDIO_FREQUENCY,
};

static struct at24_platform_data eeprom_info = {
	.byte_len	= (64*1024) / 8,
	.page_size	= 32,
	.flags		= AT24_FLAG_ADDR16,
};

static struct i2c_board_info __initdata i2c_info[] =  {
	{
		I2C_BOARD_INFO("24lc64", 0x50),
		.platform_data	= &eeprom_info,
	},
	
};

static struct davinci_i2c_platform_data i2c_pdata = {
	.bus_freq	= 20 ,
	.bus_delay	= 100 ,
};

static void __init sffsdr_init_i2c(void)
{
	davinci_init_i2c(&i2c_pdata);
	i2c_register_board_info(1, i2c_info, ARRAY_SIZE(i2c_info));
}

static struct platform_device *davinci_sffsdr_devices[] __initdata = {
	&davinci_sffsdr_nandflash_device,
};

static struct davinci_uart_config uart_config __initdata = {
	.enabled_uarts = (1 << 0),
};

static void __init davinci_sffsdr_map_io(void)
{
	dm644x_init();
}

static __init void davinci_sffsdr_init(void)
{
	struct davinci_soc_info *soc_info = &davinci_soc_info;

	platform_add_devices(davinci_sffsdr_devices,
			     ARRAY_SIZE(davinci_sffsdr_devices));
	sffsdr_init_i2c();
	davinci_serial_init(&uart_config);
	soc_info->emac_pdata->phy_mask = SFFSDR_PHY_MASK;
	soc_info->emac_pdata->mdio_max_freq = SFFSDR_MDIO_FREQUENCY;
	setup_usb(0, 0); 

	
	davinci_cfg_reg(DM644X_VLYNQEN);
	davinci_cfg_reg(DM644X_VLYNQWD);
}

static __init void davinci_sffsdr_irq_init(void)
{
	davinci_irq_init();
}

MACHINE_START(SFFSDR, "Lyrtech SFFSDR")
	
	.phys_io      = IO_PHYS,
	.io_pg_offst  = (__IO_ADDRESS(IO_PHYS) >> 18) & 0xfffc,
	.boot_params  = (DAVINCI_DDR_BASE + 0x100),
	.map_io	      = davinci_sffsdr_map_io,
	.init_irq     = davinci_sffsdr_irq_init,
	.timer	      = &davinci_timer,
	.init_machine = davinci_sffsdr_init,
MACHINE_END
