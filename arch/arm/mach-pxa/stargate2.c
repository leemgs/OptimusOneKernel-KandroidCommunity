

#include <linux/init.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/bitops.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/regulator/machine.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/plat-ram.h>
#include <linux/mtd/partitions.h>

#include <linux/i2c/pcf857x.h>
#include <linux/i2c/at24.h>
#include <linux/smc91x.h>
#include <linux/gpio.h>

#include <asm/types.h>
#include <asm/setup.h>
#include <asm/memory.h>
#include <asm/mach-types.h>
#include <asm/irq.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>
#include <asm/mach/flash.h>

#include <mach/pxa27x.h>
#include <plat/i2c.h>
#include <mach/mmc.h>
#include <mach/udc.h>
#include <mach/pxa2xx_spi.h>
#include <mach/pxa27x-udc.h>

#include <linux/spi/spi.h>
#include <linux/mfd/da903x.h>
#include <linux/sht15.h>

#include "devices.h"
#include "generic.h"


#define SG2_BT_RESET		81


#define SG2_GPIO_nSD_DETECT	90
#define SG2_SD_POWER_ENABLE	89

static unsigned long stargate2_pin_config[] __initdata = {

	GPIO15_nCS_1, 
	
	GPIO80_nCS_4,
	GPIO40_GPIO, 
	
	GPIO102_GPIO,

	
	GPIO91_GPIO | WAKEUP_ON_LEVEL_HIGH,

	
	GPIO1_GPIO,

	
	GPIO79_PSKTSEL,
	GPIO48_nPOE,
	GPIO49_nPWE,
	GPIO50_nPIOR,
	GPIO51_nPIOW,
	GPIO85_nPCE_1,
	GPIO54_nPCE_2,
	GPIO55_nPREG,
	GPIO56_nPWAIT,
	GPIO57_nIOIS16,
	GPIO120_GPIO, 
	GPIO108_GPIO, 
	GPIO82_GPIO, 
	GPIO53_GPIO, 

	
	GPIO32_MMC_CLK,
	GPIO112_MMC_CMD,
	GPIO92_MMC_DAT_0,
	GPIO109_MMC_DAT_1,
	GPIO110_MMC_DAT_2,
	GPIO111_MMC_DAT_3,
	GPIO90_GPIO, 
	GPIO89_GPIO, 

	
	GPIO81_GPIO, 

	
	GPIO22_GPIO,		
	GPIO114_GPIO,		
	GPIO116_GPIO, 		
	GPIO0_GPIO,		
	GPIO16_GPIO,		
	GPIO39_GPIO,		

	
	GPIO117_I2C_SCL,
	GPIO118_I2C_SDA,

	
	GPIO39_GPIO, 
	GPIO34_SSP3_SCLK,
	GPIO35_SSP3_TXD,
	GPIO41_SSP3_RXD,

	
	GPIO11_SSP2_RXD,
	GPIO38_SSP2_TXD,
	GPIO36_SSP2_SCLK,
	GPIO37_GPIO, 

	
	GPIO26_SSP1_RXD,
	GPIO25_SSP1_TXD,
	GPIO23_SSP1_SCLK,
	GPIO24_GPIO, 

	
	GPIO42_BTUART_RXD,
	GPIO43_BTUART_TXD,
	GPIO44_BTUART_CTS,
	GPIO45_BTUART_RTS,

	
	GPIO46_STUART_RXD,
	GPIO47_STUART_TXD,

	
	GPIO96_GPIO,	
	GPIO99_GPIO,	

	
	GPIO94_GPIO, 
	GPIO10_GPIO, 

	
	GPIO100_GPIO,
	GPIO98_GPIO,
};


static int stargate2_reset_bluetooth(void)
{
	int err;
	err = gpio_request(SG2_BT_RESET, "SG2_BT_RESET");
	if (err) {
		printk(KERN_ERR "Could not get gpio for bluetooth reset \n");
		return err;
	}
	gpio_direction_output(SG2_BT_RESET, 1);
	mdelay(5);
	
	gpio_set_value(SG2_BT_RESET, 0);
	mdelay(10);
	gpio_set_value(SG2_BT_RESET, 1);
	gpio_free(SG2_BT_RESET);
	return 0;
}

static struct led_info stargate2_leds[] = {
	{
		.name = "sg2:red",
		.flags = DA9030_LED_RATE_ON,
	}, {
		.name = "sg2:blue",
		.flags = DA9030_LED_RATE_ON,
	}, {
		.name = "sg2:green",
		.flags = DA9030_LED_RATE_ON,
	},
};

static struct sht15_platform_data platform_data_sht15 = {
	.gpio_data =  100,
	.gpio_sck  =  98,
};

static struct platform_device sht15 = {
	.name = "sht15",
	.id = -1,
	.dev = {
		.platform_data = &platform_data_sht15,
	},
};

static struct regulator_consumer_supply stargate2_sensor_3_con[] = {
	{
		.dev = &sht15.dev,
		.supply = "vcc",
	},
};

enum stargate2_ldos{
	vcc_vref,
	vcc_cc2420,
	
	vcc_mica,
	
	vcc_bt,
	
	vcc_sensor_1_8,
	vcc_sensor_3,
	
	vcc_sram_ext,
	vcc_pxa_pll,
	vcc_pxa_usim, 
	vcc_pxa_mem,
	vcc_pxa_flash,
	vcc_pxa_core, 
	vcc_lcd,
	vcc_bb,
	vcc_bbio, 
	vcc_io, 
};


static struct regulator_init_data stargate2_ldo_init_data[] = {
	[vcc_bbio] = {
		.constraints = { 
			.name = "vcc_bbio",
			.min_uV = 1800000,
			.max_uV = 1800000,
		},
	},
	[vcc_bb] = {
		.constraints = { 
			.name = "vcc_bb",
			.min_uV = 2700000,
			.max_uV = 3000000,
		},
	},
	[vcc_pxa_flash] = {
		.constraints = {
			.name = "vcc_pxa_flash",
			.min_uV = 1800000,
			.max_uV = 1800000,
		},
	},
	[vcc_cc2420] = { 
		.constraints = {
			
			.name = "vcc_cc2420",
			.min_uV = 2700000,
			.max_uV = 3300000,
		},
	},
	[vcc_vref] = { 
		.constraints = { 
			.name = "vcc_vref",
			.min_uV = 1800000,
			.max_uV = 1800000,
		},
	},
	[vcc_sram_ext] = {
		.constraints = { 
			.name = "vcc_sram_ext",
			.min_uV = 2800000,
			.max_uV = 2800000,
		},
	},
	[vcc_mica] = {
		.constraints = { 
			.name = "vcc_mica",
			.min_uV = 2800000,
			.max_uV = 2800000,
		},
	},
	[vcc_bt] = {
		.constraints = { 
			.name = "vcc_bt",
			.min_uV = 2800000,
			.max_uV = 2800000,
		},
	},
	[vcc_lcd] = {
		.constraints = { 
			.name = "vcc_lcd",
			.min_uV = 2700000,
			.max_uV = 3300000,
		},
	},
	[vcc_io] = { 
		.constraints = { 
			.name = "vcc_io",
			.min_uV = 2692000,
			.max_uV = 3300000,
		},
	},
	[vcc_sensor_1_8] = {
		.constraints = { 
			.name = "vcc_sensor_1_8",
			.min_uV = 1800000,
			.max_uV = 1800000,
		},
	},
	[vcc_sensor_3] = { 
		.constraints = {
			.name = "vcc_sensor_3",
			.min_uV = 2800000,
			.max_uV = 3000000,
		},
		.num_consumer_supplies = ARRAY_SIZE(stargate2_sensor_3_con),
		.consumer_supplies = stargate2_sensor_3_con,
	},
	[vcc_pxa_pll] = { 
		.constraints = {
			.name = "vcc_pxa_pll",
			.min_uV = 1170000,
			.max_uV = 1430000,
		},
	},
	[vcc_pxa_usim] = {
		.constraints = { 
			.name = "vcc_pxa_usim",
			.min_uV = 1710000,
			.max_uV = 2160000,
		},
	},
	[vcc_pxa_mem] = {
		.constraints = { 
			.name = "vcc_pxa_mem",
			.min_uV = 1800000,
			.max_uV = 1800000,
		},
	},
};

static struct da903x_subdev_info stargate2_da9030_subdevs[] = {
	{
		.name = "da903x-led",
		.id = DA9030_ID_LED_2,
		.platform_data = &stargate2_leds[0],
	}, {
		.name = "da903x-led",
		.id = DA9030_ID_LED_3,
		.platform_data = &stargate2_leds[2],
	}, {
		.name = "da903x-led",
		.id = DA9030_ID_LED_4,
		.platform_data = &stargate2_leds[1],
	}, {
		.name = "da903x-regulator",
		.id = DA9030_ID_LDO2,
		.platform_data = &stargate2_ldo_init_data[vcc_bbio],
	}, {
		.name = "da903x-regulator",
		.id = DA9030_ID_LDO3,
		.platform_data = &stargate2_ldo_init_data[vcc_bb],
	}, {
		.name = "da903x-regulator",
		.id = DA9030_ID_LDO4,
		.platform_data = &stargate2_ldo_init_data[vcc_pxa_flash],
	}, {
		.name = "da903x-regulator",
		.id = DA9030_ID_LDO5,
		.platform_data = &stargate2_ldo_init_data[vcc_cc2420],
	}, {
		.name = "da903x-regulator",
		.id = DA9030_ID_LDO6,
		.platform_data = &stargate2_ldo_init_data[vcc_vref],
	}, {
		.name = "da903x-regulator",
		.id = DA9030_ID_LDO7,
		.platform_data = &stargate2_ldo_init_data[vcc_sram_ext],
	}, {
		.name = "da903x-regulator",
		.id = DA9030_ID_LDO8,
		.platform_data = &stargate2_ldo_init_data[vcc_mica],
	}, {
		.name = "da903x-regulator",
		.id = DA9030_ID_LDO9,
		.platform_data = &stargate2_ldo_init_data[vcc_bt],
	}, {
		.name = "da903x-regulator",
		.id = DA9030_ID_LDO10,
		.platform_data = &stargate2_ldo_init_data[vcc_sensor_1_8],
	}, {
		.name = "da903x-regulator",
		.id = DA9030_ID_LDO11,
		.platform_data = &stargate2_ldo_init_data[vcc_sensor_3],
	}, {
		.name = "da903x-regulator",
		.id = DA9030_ID_LDO12,
		.platform_data = &stargate2_ldo_init_data[vcc_lcd],
	}, {
		.name = "da903x-regulator",
		.id = DA9030_ID_LDO15,
		.platform_data = &stargate2_ldo_init_data[vcc_pxa_pll],
	}, {
		.name = "da903x-regulator",
		.id = DA9030_ID_LDO17,
		.platform_data = &stargate2_ldo_init_data[vcc_pxa_usim],
	}, {
		.name = "da903x-regulator", 
		.id = DA9030_ID_LDO18,
		.platform_data = &stargate2_ldo_init_data[vcc_io],
	}, {
		.name = "da903x-regulator",
		.id = DA9030_ID_LDO19,
		.platform_data = &stargate2_ldo_init_data[vcc_pxa_mem],
	},
};

static struct da903x_platform_data stargate2_da9030_pdata = {
	.num_subdevs = ARRAY_SIZE(stargate2_da9030_subdevs),
	.subdevs = stargate2_da9030_subdevs,
};

static struct resource smc91x_resources[] = {
	[0] = {
		.name = "smc91x-regs",
		.start = (PXA_CS4_PHYS + 0x300),
		.end = (PXA_CS4_PHYS + 0xfffff),
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_GPIO(40),
		.end = IRQ_GPIO(40),
		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,
	}
};

static struct smc91x_platdata stargate2_smc91x_info = {
	.flags = SMC91X_USE_8BIT | SMC91X_USE_16BIT | SMC91X_USE_32BIT
	| SMC91X_NOWAIT | SMC91X_USE_DMA,
};

static struct platform_device smc91x_device = {
	.name = "smc91x",
	.id = -1,
	.num_resources = ARRAY_SIZE(smc91x_resources),
	.resource = smc91x_resources,
	.dev = {
		.platform_data = &stargate2_smc91x_info,
	},
};



static struct pxamci_platform_data stargate2_mci_platform_data;


static int stargate2_mci_init(struct device *dev,
			      irq_handler_t stargate2_detect_int,
			      void *data)
{
	int err;

	err = gpio_request(SG2_SD_POWER_ENABLE, "SG2_sd_power_enable");
	if (err) {
		printk(KERN_ERR "Can't get the gpio for SD power control");
		goto return_err;
	}
	gpio_direction_output(SG2_SD_POWER_ENABLE, 0);

	err = gpio_request(SG2_GPIO_nSD_DETECT, "SG2_sd_detect");
	if (err) {
		printk(KERN_ERR "Can't get the sd detect gpio");
		goto free_power_en;
	}
	gpio_direction_input(SG2_GPIO_nSD_DETECT);
	
	stargate2_mci_platform_data.detect_delay = msecs_to_jiffies(250);

	err = request_irq(IRQ_GPIO(SG2_GPIO_nSD_DETECT),
			  stargate2_detect_int,
			  IRQ_TYPE_EDGE_BOTH,
			  "MMC card detect",
			  data);
	if (err) {
		printk(KERN_ERR "can't request MMC card detect IRQ\n");
		goto free_nsd_detect;
	}
	return 0;

 free_nsd_detect:
	gpio_free(SG2_GPIO_nSD_DETECT);
 free_power_en:
	gpio_free(SG2_SD_POWER_ENABLE);
 return_err:
	return err;
}


static void stargate2_mci_setpower(struct device *dev, unsigned int vdd)
{
	gpio_set_value(SG2_SD_POWER_ENABLE, !!vdd);
}

static void stargate2_mci_exit(struct device *dev, void *data)
{
	free_irq(IRQ_GPIO(SG2_GPIO_nSD_DETECT), data);
	gpio_free(SG2_SD_POWER_ENABLE);
	gpio_free(SG2_GPIO_nSD_DETECT);
}

static struct pxamci_platform_data stargate2_mci_platform_data = {
	.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34,
	.init = stargate2_mci_init,
	.setpower = stargate2_mci_setpower,
	.exit = stargate2_mci_exit,
};

static struct mtd_partition stargate2flash_partitions[] = {
	{
		.name = "Bootloader",
		.size = 0x00040000,
		.offset = 0,
		.mask_flags = 0,
	}, {
		.name = "Kernel",
		.size = 0x00200000,
		.offset = 0x00040000,
		.mask_flags = 0
	}, {
		.name = "Filesystem",
		.size = 0x01DC0000,
		.offset = 0x00240000,
		.mask_flags = 0
	},
};

static struct resource flash_resources = {
	.start = PXA_CS0_PHYS,
	.end = PXA_CS0_PHYS + SZ_32M - 1,
	.flags = IORESOURCE_MEM,
};

static struct flash_platform_data stargate2_flash_data = {
	.map_name = "cfi_probe",
	.parts = stargate2flash_partitions,
	.nr_parts = ARRAY_SIZE(stargate2flash_partitions),
	.name = "PXA27xOnChipROM",
	.width = 2,
};

static struct platform_device stargate2_flash_device = {
	.name = "pxa2xx-flash",
	.id = 0,
	.dev = {
		.platform_data = &stargate2_flash_data,
	},
	.resource = &flash_resources,
	.num_resources = 1,
};


static struct resource sram_resources = {
	.start = PXA_CS1_PHYS,
	.end = PXA_CS1_PHYS + SZ_32M-1,
	.flags = IORESOURCE_MEM,
};

static struct platdata_mtd_ram stargate2_sram_pdata = {
	.mapname = "Stargate2 SRAM",
	.bankwidth = 2,
};

static struct platform_device stargate2_sram = {
	.name = "mtd-ram",
	.id = 0,
	.resource = &sram_resources,
	.num_resources = 1,
	.dev = {
		.platform_data = &stargate2_sram_pdata,
	},
};

static struct pcf857x_platform_data platform_data_pcf857x = {
	.gpio_base = 128,
	.n_latch = 0,
	.setup = NULL,
	.teardown = NULL,
	.context = NULL,
};

static struct at24_platform_data pca9500_eeprom_pdata = {
	.byte_len = 256,
	.page_size = 4,
};


static struct i2c_board_info __initdata stargate2_i2c_board_info[] = {
	
	{
		.type = "pcf8574",
		.addr =  0x27,
		.platform_data = &platform_data_pcf857x,
	}, {
		.type = "24c02",
		.addr = 0x57,
		.platform_data = &pca9500_eeprom_pdata,
	}, {
		.type = "max1238",
		.addr = 0x35,
	}, { 
		.type = "max1363",
		.addr = 0x34,
		
		.irq = IRQ_GPIO(99),
	}, { 
		.type = "tsl2561",
		.addr = 0x49,
		
		.irq = IRQ_GPIO(99),
	}, { 
		.type = "tmp175",
		.addr = 0x4A,
		.irq = IRQ_GPIO(96),
	},
};

static struct i2c_board_info __initdata stargate2_pwr_i2c_board_info[] = {
	{
		.type = "da9030",
		.addr = 0x49,
		.platform_data = &stargate2_da9030_pdata,
		.irq = gpio_to_irq(1),
	},
};

static struct pxa2xx_spi_master pxa_ssp_master_0_info = {
	.num_chipselect = 1,
};

static struct pxa2xx_spi_master pxa_ssp_master_1_info = {
	.num_chipselect = 1,
};

static struct pxa2xx_spi_master pxa_ssp_master_2_info = {
	.num_chipselect = 1,
};


static struct pxa2xx_spi_chip staccel_chip_info = {
	.tx_threshold = 8,
	.rx_threshold = 8,
	.dma_burst_size = 8,
	.timeout = 235,
	.gpio_cs = 24,
};

static struct pxa2xx_spi_chip cc2420_info = {
	.tx_threshold = 8,
	.rx_threshold = 8,
	.dma_burst_size = 8,
	.timeout = 235,
	.gpio_cs = 39,
};

static struct spi_board_info spi_board_info[] __initdata = {
	{
		.modalias = "lis3l02dq",
		.max_speed_hz = 8000000,
		.bus_num = 1,
		.chip_select = 0,
		.controller_data = &staccel_chip_info,
		.irq = IRQ_GPIO(96),
	}, {
		.modalias = "cc2420",
		.max_speed_hz = 6500000,
		.bus_num = 3,
		.chip_select = 0,
		.controller_data = &cc2420_info,
	},
};

static void sg2_udc_command(int cmd)
{
	switch (cmd) {
	case PXA2XX_UDC_CMD_CONNECT:
		UP2OCR |=  UP2OCR_HXOE  | UP2OCR_DPPUE | UP2OCR_DPPUBE;
		break;
	case PXA2XX_UDC_CMD_DISCONNECT:
		UP2OCR &= ~(UP2OCR_HXOE  | UP2OCR_DPPUE | UP2OCR_DPPUBE);
		break;
	}
}


static int sg2_udc_detect(void)
{
	return 1;
}

static struct pxa2xx_udc_mach_info stargate2_udc_info __initdata = {
	.udc_is_connected	= sg2_udc_detect,
	.udc_command		= sg2_udc_command,
};

static struct platform_device *stargate2_devices[] = {
	&stargate2_flash_device,
	&stargate2_sram,
	&smc91x_device,
	&sht15,
};

static struct i2c_pxa_platform_data i2c_pwr_pdata = {
	.fast_mode = 1,
};

static struct i2c_pxa_platform_data i2c_pdata = {
	.fast_mode = 1,
};

static void __init stargate2_init(void)
{
	
	MECR &= ~MECR_NOS;

	pxa2xx_mfp_config(ARRAY_AND_SIZE(stargate2_pin_config));

	
	gpio_direction_output(37, 0);
	gpio_direction_output(24, 0);
	gpio_direction_output(39, 0);

	platform_add_devices(ARRAY_AND_SIZE(stargate2_devices));

	pxa2xx_set_spi_info(1, &pxa_ssp_master_0_info);
	pxa2xx_set_spi_info(2, &pxa_ssp_master_1_info);
	pxa2xx_set_spi_info(3, &pxa_ssp_master_2_info);
	spi_register_board_info(spi_board_info, ARRAY_SIZE(spi_board_info));

	i2c_register_board_info(0, ARRAY_AND_SIZE(stargate2_i2c_board_info));
	i2c_register_board_info(1,
				ARRAY_AND_SIZE(stargate2_pwr_i2c_board_info));
	pxa27x_set_i2c_power_info(&i2c_pwr_pdata);
	pxa_set_i2c_info(&i2c_pdata);

	pxa_set_mci_info(&stargate2_mci_platform_data);

	pxa_set_udc_info(&stargate2_udc_info);

	stargate2_reset_bluetooth();
}

MACHINE_START(STARGATE2, "Stargate 2")
	.phys_io = 0x40000000,
	.io_pg_offst = (io_p2v(0x40000000) >> 18) & 0xfffc,
	.map_io = pxa_map_io,
	.init_irq = pxa27x_init_irq,
	.timer = &pxa_timer,
	.init_machine = stargate2_init,
	.boot_params = 0xA0000100,
MACHINE_END
