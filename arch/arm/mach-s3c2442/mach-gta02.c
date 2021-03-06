

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/serial_core.h>
#include <linux/spi/spi.h>

#include <linux/mmc/host.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>
#include <linux/io.h>

#include <linux/i2c.h>
#include <linux/backlight.h>
#include <linux/regulator/machine.h>

#include <linux/mfd/pcf50633/core.h>
#include <linux/mfd/pcf50633/mbc.h>
#include <linux/mfd/pcf50633/adc.h>
#include <linux/mfd/pcf50633/gpio.h>
#include <linux/mfd/pcf50633/pmic.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <asm/irq.h>
#include <asm/mach-types.h>

#include <mach/regs-irq.h>
#include <mach/regs-gpio.h>
#include <mach/regs-gpioj.h>
#include <mach/fb.h>

#include <mach/spi.h>
#include <mach/spi-gpio.h>
#include <plat/usb-control.h>
#include <mach/regs-mem.h>
#include <mach/hardware.h>

#include <mach/gta02.h>

#include <plat/regs-serial.h>
#include <plat/nand.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/pm.h>
#include <plat/udc.h>
#include <plat/gpio-cfg.h>
#include <plat/iic.h>

static struct pcf50633 *gta02_pcf;



static long gta02_panic_blink(long count)
{
	long delay = 0;
	static long last_blink;
	static char led;

	
	if (count - last_blink < 100)
		return 0;

	led ^= 1;
	gpio_direction_output(GTA02_GPIO_AUX_LED, led);

	last_blink = count;

	return delay;
}


static struct map_desc gta02_iodesc[] __initdata = {
	{
		.virtual	= 0xe0000000,
		.pfn		= __phys_to_pfn(S3C2410_CS3 + 0x01000000),
		.length		= SZ_1M,
		.type		= MT_DEVICE
	},
};

#define UCON (S3C2410_UCON_DEFAULT | S3C2443_UCON_RXERR_IRQEN)
#define ULCON (S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB)
#define UFCON (S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE)

static struct s3c2410_uartcfg gta02_uartcfgs[] = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= UCON,
		.ulcon		= ULCON,
		.ufcon		= UFCON,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= UCON,
		.ulcon		= ULCON,
		.ufcon		= UFCON,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= UCON,
		.ulcon		= ULCON,
		.ufcon		= UFCON,
	},
};

#ifdef CONFIG_CHARGER_PCF50633


#define ADC_NOM_CHG_DETECT_1A 6
#define ADC_NOM_CHG_DETECT_USB 43

static void
gta02_configure_pmu_for_charger(struct pcf50633 *pcf, void *unused, int res)
{
	int  ma;

	
	if (res < ((ADC_NOM_CHG_DETECT_USB + ADC_NOM_CHG_DETECT_1A) / 2)) {

		
		pcf50633_gpio_set(pcf, PCF50633_GPO, 0);

		ma = 1000;
	} else
		ma = 100;

	pcf50633_mbc_usb_curlim_set(pcf, ma);
}

static struct delayed_work gta02_charger_work;
static int gta02_usb_vbus_draw;

static void gta02_charger_worker(struct work_struct *work)
{
	if (gta02_usb_vbus_draw) {
		pcf50633_mbc_usb_curlim_set(gta02_pcf, gta02_usb_vbus_draw);
		return;
	}

#ifdef CONFIG_PCF50633_ADC
	pcf50633_adc_async_read(gta02_pcf,
				PCF50633_ADCC1_MUX_ADCIN1,
				PCF50633_ADCC1_AVERAGE_16,
				gta02_configure_pmu_for_charger,
				NULL);
#else
	
	pcf50633_mbc_usb_curlim_set(pcf, 100);
#endif
}

#define GTA02_CHARGER_CONFIGURE_TIMEOUT ((3000 * HZ) / 1000)

static void gta02_pmu_event_callback(struct pcf50633 *pcf, int irq)
{
	if (irq == PCF50633_IRQ_USBINS) {
		schedule_delayed_work(&gta02_charger_work,
				      GTA02_CHARGER_CONFIGURE_TIMEOUT);

		return;
	}

	if (irq == PCF50633_IRQ_USBREM) {
		cancel_delayed_work_sync(&gta02_charger_work);
		gta02_usb_vbus_draw = 0;
	}
}

static void gta02_udc_vbus_draw(unsigned int ma)
{
	if (!gta02_pcf)
		return;

	gta02_usb_vbus_draw = ma;

	schedule_delayed_work(&gta02_charger_work,
			      GTA02_CHARGER_CONFIGURE_TIMEOUT);
}
#else 
#define gta02_pmu_event_callback	NULL
#define gta02_udc_vbus_draw		NULL
#endif



static void gta02_pmu_attach_child_devices(struct pcf50633 *pcf);


static char *gta02_batteries[] = {
	"battery",
};

struct pcf50633_platform_data gta02_pcf_pdata = {
	.resumers = {
		[0] =	PCF50633_INT1_USBINS |
			PCF50633_INT1_USBREM |
			PCF50633_INT1_ALARM,
		[1] =	PCF50633_INT2_ONKEYF,
		[2] =	PCF50633_INT3_ONKEY1S,
		[3] =	PCF50633_INT4_LOWSYS |
			PCF50633_INT4_LOWBAT |
			PCF50633_INT4_HIGHTMP,
	},

	.batteries = gta02_batteries,
	.num_batteries = ARRAY_SIZE(gta02_batteries),
	.reg_init_data = {
		[PCF50633_REGULATOR_AUTO] = {
			.constraints = {
				.min_uV = 3300000,
				.max_uV = 3300000,
				.valid_modes_mask = REGULATOR_MODE_NORMAL,
				.always_on = 1,
				.apply_uV = 1,
				.state_mem = {
					.enabled = 1,
				},
			},
		},
		[PCF50633_REGULATOR_DOWN1] = {
			.constraints = {
				.min_uV = 1300000,
				.max_uV = 1600000,
				.valid_modes_mask = REGULATOR_MODE_NORMAL,
				.always_on = 1,
				.apply_uV = 1,
			},
		},
		[PCF50633_REGULATOR_DOWN2] = {
			.constraints = {
				.min_uV = 1800000,
				.max_uV = 1800000,
				.valid_modes_mask = REGULATOR_MODE_NORMAL,
				.apply_uV = 1,
				.always_on = 1,
				.state_mem = {
					.enabled = 1,
				},
			},
		},
		[PCF50633_REGULATOR_HCLDO] = {
			.constraints = {
				.min_uV = 2000000,
				.max_uV = 3300000,
				.valid_modes_mask = REGULATOR_MODE_NORMAL,
				.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
				.always_on = 1,
			},
		},
		[PCF50633_REGULATOR_LDO1] = {
			.constraints = {
				.min_uV = 3300000,
				.max_uV = 3300000,
				.valid_modes_mask = REGULATOR_MODE_NORMAL,
				.apply_uV = 1,
				.state_mem = {
					.enabled = 0,
				},
			},
		},
		[PCF50633_REGULATOR_LDO2] = {
			.constraints = {
				.min_uV = 3300000,
				.max_uV = 3300000,
				.valid_modes_mask = REGULATOR_MODE_NORMAL,
				.apply_uV = 1,
			},
		},
		[PCF50633_REGULATOR_LDO3] = {
			.constraints = {
				.min_uV = 3000000,
				.max_uV = 3000000,
				.valid_modes_mask = REGULATOR_MODE_NORMAL,
				.apply_uV = 1,
			},
		},
		[PCF50633_REGULATOR_LDO4] = {
			.constraints = {
				.min_uV = 3200000,
				.max_uV = 3200000,
				.valid_modes_mask = REGULATOR_MODE_NORMAL,
				.apply_uV = 1,
			},
		},
		[PCF50633_REGULATOR_LDO5] = {
			.constraints = {
				.min_uV = 3000000,
				.max_uV = 3000000,
				.valid_modes_mask = REGULATOR_MODE_NORMAL,
				.apply_uV = 1,
				.state_mem = {
					.enabled = 1,
				},
			},
		},
		[PCF50633_REGULATOR_LDO6] = {
			.constraints = {
				.min_uV = 3000000,
				.max_uV = 3000000,
				.valid_modes_mask = REGULATOR_MODE_NORMAL,
			},
		},
		[PCF50633_REGULATOR_MEMLDO] = {
			.constraints = {
				.min_uV = 1800000,
				.max_uV = 1800000,
				.valid_modes_mask = REGULATOR_MODE_NORMAL,
				.state_mem = {
					.enabled = 1,
				},
			},
		},

	},
	.probe_done = gta02_pmu_attach_child_devices,
	.mbc_event_callback = gta02_pmu_event_callback,
};




#define GTA02_FLASH_BASE	0x18000000 
#define GTA02_FLASH_SIZE	0x200000 

static struct physmap_flash_data gta02_nor_flash_data = {
	.width		= 2,
};

static struct resource gta02_nor_flash_resource = {
	.start		= GTA02_FLASH_BASE,
	.end		= GTA02_FLASH_BASE + GTA02_FLASH_SIZE - 1,
	.flags		= IORESOURCE_MEM,
};

static struct platform_device gta02_nor_flash = {
	.name		= "physmap-flash",
	.id		= 0,
	.dev		= {
		.platform_data	= &gta02_nor_flash_data,
	},
	.resource	= &gta02_nor_flash_resource,
	.num_resources	= 1,
};


struct platform_device s3c24xx_pwm_device = {
	.name		= "s3c24xx_pwm",
	.num_resources	= 0,
};

static struct i2c_board_info gta02_i2c_devs[] __initdata = {
	{
		I2C_BOARD_INFO("pcf50633", 0x73),
		.irq = GTA02_IRQ_PCF50633,
		.platform_data = &gta02_pcf_pdata,
	},
	{
		I2C_BOARD_INFO("wm8753", 0x1a),
	},
};

static struct s3c2410_nand_set gta02_nand_sets[] = {
	[0] = {
		
		.name		= "neo1973-nand",
		.nr_chips	= 1,
		.flash_bbt	= 1,
	},
};



static struct s3c2410_platform_nand gta02_nand_info = {
	.tacls		= 0,
	.twrph0		= 25,
	.twrph1		= 15,
	.nr_sets	= ARRAY_SIZE(gta02_nand_sets),
	.sets		= gta02_nand_sets,
};


static void gta02_udc_command(enum s3c2410_udc_cmd_e cmd)
{
	switch (cmd) {
	case S3C2410_UDC_P_ENABLE:
		pr_debug("%s S3C2410_UDC_P_ENABLE\n", __func__);
		gpio_direction_output(GTA02_GPIO_USB_PULLUP, 1);
		break;
	case S3C2410_UDC_P_DISABLE:
		pr_debug("%s S3C2410_UDC_P_DISABLE\n", __func__);
		gpio_direction_output(GTA02_GPIO_USB_PULLUP, 0);
		break;
	case S3C2410_UDC_P_RESET:
		pr_debug("%s S3C2410_UDC_P_RESET\n", __func__);
		
	}
}


static struct s3c2410_udc_mach_info gta02_udc_cfg = {
	.vbus_draw	= gta02_udc_vbus_draw,
	.udc_command	= gta02_udc_command,

};



static void gta02_bl_set_intensity(int intensity)
{
	struct pcf50633 *pcf = gta02_pcf;
	int old_intensity = pcf50633_reg_read(pcf, PCF50633_REG_LEDOUT);

	
	intensity >>= 2;

	
	if (in_atomic()) {
		printk(KERN_ERR "gta02_bl_set_intensity called while atomic\n");
		return;
	}

	old_intensity = pcf50633_reg_read(pcf, PCF50633_REG_LEDOUT);
	if (intensity == old_intensity)
		return;

	
	pcf50633_reg_write(pcf, PCF50633_REG_LEDDIM, 5);

	if (!(pcf50633_reg_read(pcf, PCF50633_REG_LEDENA) & 3))
		old_intensity = 0;

	
	if (!intensity || !old_intensity)
		pcf50633_reg_write(pcf, PCF50633_REG_LEDENA, 0);

	
	if (!intensity)
		pcf50633_reg_set_bit_mask(pcf, PCF50633_REG_LEDOUT, 0x3f, 2);
	else
		pcf50633_reg_set_bit_mask(pcf, PCF50633_REG_LEDOUT, 0x3f,
					  intensity);

	if (intensity)
		pcf50633_reg_write(pcf, PCF50633_REG_LEDENA, 2);

}

static struct generic_bl_info gta02_bl_info = {
	.name			= "gta02-bl",
	.max_intensity		= 0xff,
	.default_intensity	= 0xff,
	.set_bl_intensity	= gta02_bl_set_intensity,
};

static struct platform_device gta02_bl_dev = {
	.name			= "generic-bl",
	.id			= 1,
	.dev = {
		.platform_data = &gta02_bl_info,
	},
};




static struct s3c2410_hcd_info gta02_usb_info = {
	.port[0]	= {
		.flags	= S3C_HCDFLG_USED,
	},
	.port[1]	= {
		.flags	= 0,
	},
};


static void __init gta02_map_io(void)
{
	s3c24xx_init_io(gta02_iodesc, ARRAY_SIZE(gta02_iodesc));
	s3c24xx_init_clocks(12000000);
	s3c24xx_init_uarts(gta02_uartcfgs, ARRAY_SIZE(gta02_uartcfgs));
}




static struct platform_device *gta02_devices[] __initdata = {
	&s3c_device_usb,
	&s3c_device_wdt,
	&s3c_device_sdi,
	&s3c_device_usbgadget,
	&s3c_device_nand,
	&gta02_nor_flash,
	&s3c24xx_pwm_device,
	&s3c_device_iis,
	&s3c_device_i2c0,
};



static struct platform_device *gta02_devices_pmu_children[] = {
	&gta02_bl_dev,
};




static void gta02_pmu_attach_child_devices(struct pcf50633 *pcf)
{
	int n;

	
	gta02_pcf = pcf;

	for (n = 0; n < ARRAY_SIZE(gta02_devices_pmu_children); n++)
		gta02_devices_pmu_children[n]->dev.parent = pcf->dev;

	platform_add_devices(gta02_devices_pmu_children,
			     ARRAY_SIZE(gta02_devices_pmu_children));
}

static void gta02_poweroff(void)
{
	pcf50633_reg_set_bit_mask(gta02_pcf, PCF50633_REG_OOCSHDWN, 1, 1);
}

static void __init gta02_machine_init(void)
{
	
	panic_blink = gta02_panic_blink;

	s3c_pm_init();

#ifdef CONFIG_CHARGER_PCF50633
	INIT_DELAYED_WORK(&gta02_charger_work, gta02_charger_worker);
#endif

	s3c_device_usb.dev.platform_data = &gta02_usb_info;
	s3c_device_nand.dev.platform_data = &gta02_nand_info;

	s3c24xx_udc_set_platdata(&gta02_udc_cfg);
	s3c_i2c0_set_platdata(NULL);

	i2c_register_board_info(0, gta02_i2c_devs, ARRAY_SIZE(gta02_i2c_devs));

	platform_add_devices(gta02_devices, ARRAY_SIZE(gta02_devices));
	pm_power_off = gta02_poweroff;
}


MACHINE_START(NEO1973_GTA02, "GTA02")
	
	.phys_io	= S3C2410_PA_UART,
	.io_pg_offst	= (((u32)S3C24XX_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S3C2410_SDRAM_PA + 0x100,
	.map_io		= gta02_map_io,
	.init_irq	= s3c24xx_init_irq,
	.init_machine	= gta02_machine_init,
	.timer		= &s3c24xx_timer,
MACHINE_END
