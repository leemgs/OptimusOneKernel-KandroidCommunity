

#include <linux/kernel.h>
#include <linux/types.h>

#include <linux/gpio_keys.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/serial_core.h>
#include <linux/timer.h>
#include <linux/io.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <mach/fb.h>
#include <mach/leds-gpio.h>
#include <mach/regs-gpio.h>
#include <mach/regs-lcd.h>

#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>

#include <plat/iic.h>
#include <plat/regs-serial.h>

#include <plat/clock.h>
#include <plat/cpu.h>
#include <plat/devs.h>
#include <plat/s3c2410.h>
#include <plat/udc.h>

static struct map_desc n30_iodesc[] __initdata = {
	
};

static struct s3c2410_uartcfg n30_uartcfgs[] = {
	
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = 0x2c5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
	},
	
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.uart_flags  = UPF_CONS_FLOW,
		.ucon	     = 0x2c5,
		.ulcon	     = 0x43,
		.ufcon	     = 0x51,
	},
	
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = 0x2c5,
		.ulcon	     = 0x03,
		.ufcon	     = 0x51,
	},
};

static void n30_udc_pullup(enum s3c2410_udc_cmd_e cmd)
{
	switch (cmd) {
	case S3C2410_UDC_P_ENABLE :
		s3c2410_gpio_setpin(S3C2410_GPB(3), 1);
		break;
	case S3C2410_UDC_P_DISABLE :
		s3c2410_gpio_setpin(S3C2410_GPB(3), 0);
		break;
	case S3C2410_UDC_P_RESET :
		break;
	default:
		break;
	}
}

static struct s3c2410_udc_mach_info n30_udc_cfg __initdata = {
	.udc_command		= n30_udc_pullup,
	.vbus_pin		= S3C2410_GPG(1),
	.vbus_pin_inverted	= 0,
};

static struct gpio_keys_button n30_buttons[] = {
	{
		.gpio		= S3C2410_GPF(0),
		.code		= KEY_POWER,
		.desc		= "Power",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPG(9),
		.code		= KEY_UP,
		.desc		= "Thumbwheel Up",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPG(8),
		.code		= KEY_DOWN,
		.desc		= "Thumbwheel Down",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPG(7),
		.code		= KEY_ENTER,
		.desc		= "Thumbwheel Press",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPF(7),
		.code		= KEY_HOMEPAGE,
		.desc		= "Home",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPF(6),
		.code		= KEY_CALENDAR,
		.desc		= "Calendar",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPF(5),
		.code		= KEY_ADDRESSBOOK,
		.desc		= "Contacts",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPF(4),
		.code		= KEY_MAIL,
		.desc		= "Mail",
		.active_low	= 0,
	},
};

static struct gpio_keys_platform_data n30_button_data = {
	.buttons	= n30_buttons,
	.nbuttons	= ARRAY_SIZE(n30_buttons),
};

static struct platform_device n30_button_device = {
	.name		= "gpio-keys",
	.id		= -1,
	.dev		= {
		.platform_data	= &n30_button_data,
	}
};

static struct gpio_keys_button n35_buttons[] = {
	{
		.gpio		= S3C2410_GPF(0),
		.code		= KEY_POWER,
		.desc		= "Power",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPG(9),
		.code		= KEY_UP,
		.desc		= "Joystick Up",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPG(8),
		.code		= KEY_DOWN,
		.desc		= "Joystick Down",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPG(6),
		.code		= KEY_DOWN,
		.desc		= "Joystick Left",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPG(5),
		.code		= KEY_DOWN,
		.desc		= "Joystick Right",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPG(7),
		.code		= KEY_ENTER,
		.desc		= "Joystick Press",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPF(7),
		.code		= KEY_HOMEPAGE,
		.desc		= "Home",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPF(6),
		.code		= KEY_CALENDAR,
		.desc		= "Calendar",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPF(5),
		.code		= KEY_ADDRESSBOOK,
		.desc		= "Contacts",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPF(4),
		.code		= KEY_MAIL,
		.desc		= "Mail",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPF(3),
		.code		= SW_RADIO,
		.desc		= "GPS Antenna",
		.active_low	= 0,
	},
	{
		.gpio		= S3C2410_GPG(2),
		.code		= SW_HEADPHONE_INSERT,
		.desc		= "Headphone",
		.active_low	= 0,
	},
};

static struct gpio_keys_platform_data n35_button_data = {
	.buttons	= n35_buttons,
	.nbuttons	= ARRAY_SIZE(n35_buttons),
};

static struct platform_device n35_button_device = {
	.name		= "gpio-keys",
	.id		= -1,
	.num_resources	= 0,
	.dev		= {
		.platform_data	= &n35_button_data,
	}
};


static struct s3c24xx_led_platdata n30_blue_led_pdata = {
	.name		= "blue_led",
	.gpio		= S3C2410_GPG(6),
	.def_trigger	= "",
};


static struct s3c24xx_led_platdata n30_warning_led_pdata = {
	.name		= "warning_led",
	.flags          = S3C24XX_LEDF_ACTLOW,
	.gpio		= S3C2410_GPD(9),
	.def_trigger	= "",
};

static struct platform_device n30_blue_led = {
	.name		= "s3c24xx_led",
	.id		= 1,
	.dev		= {
		.platform_data	= &n30_blue_led_pdata,
	},
};

static struct platform_device n30_warning_led = {
	.name		= "s3c24xx_led",
	.id		= 2,
	.dev		= {
		.platform_data	= &n30_warning_led_pdata,
	},
};

static struct s3c2410fb_display n30_display __initdata = {
	.type		= S3C2410_LCDCON1_TFT,
	.width		= 240,
	.height		= 320,
	.pixclock	= 170000,

	.xres		= 240,
	.yres		= 320,
	.bpp		= 16,
	.left_margin	= 3,
	.right_margin	= 40,
	.hsync_len	= 40,
	.upper_margin	= 2,
	.lower_margin	= 3,
	.vsync_len	= 2,

	.lcdcon5 = S3C2410_LCDCON5_INVVLINE | S3C2410_LCDCON5_INVVFRAME,
};

static struct s3c2410fb_mach_info n30_fb_info __initdata = {
	.displays	= &n30_display,
	.num_displays	= 1,
	.default_display = 0,
	.lpcsel		= 0x06,
};

static struct platform_device *n30_devices[] __initdata = {
	&s3c_device_lcd,
	&s3c_device_wdt,
	&s3c_device_i2c0,
	&s3c_device_iis,
	&s3c_device_usb,
	&s3c_device_usbgadget,
	&n30_button_device,
	&n30_blue_led,
	&n30_warning_led,
};

static struct platform_device *n35_devices[] __initdata = {
	&s3c_device_lcd,
	&s3c_device_wdt,
	&s3c_device_i2c0,
	&s3c_device_iis,
	&s3c_device_usbgadget,
	&n35_button_device,
};

static struct s3c2410_platform_i2c n30_i2ccfg = {
	.flags		= 0,
	.slave_addr	= 0x10,
	.frequency	= 10*1000,
};


static void __init n30_hwinit(void)
{
	
	if (machine_is_n30())
		__raw_writel(0x007fffff, S3C2410_GPACON);
	if (machine_is_n35())
		__raw_writel(0x007fefff, S3C2410_GPACON);
	__raw_writel(0x00000000, S3C2410_GPADAT);

	
	__raw_writel(0x00154556, S3C2410_GPBCON);
	__raw_writel(0x00000750, S3C2410_GPBDAT);
	__raw_writel(0x00000073, S3C2410_GPBUP);

	
	__raw_writel(0xaaa80618, S3C2410_GPCCON);
	__raw_writel(0x0000014c, S3C2410_GPCDAT);
	__raw_writel(0x0000fef2, S3C2410_GPCUP);

	
	__raw_writel(0xaa95aaa4, S3C2410_GPDCON);
	__raw_writel(0x00000601, S3C2410_GPDDAT);
	__raw_writel(0x0000fbfe, S3C2410_GPDUP);

	
	__raw_writel(0xa56aaaaa, S3C2410_GPECON);
	__raw_writel(0x0000efc5, S3C2410_GPEDAT);
	__raw_writel(0x0000f81f, S3C2410_GPEUP);

	
	__raw_writel(0x0000aaaa, S3C2410_GPFCON);
	__raw_writel(0x00000000, S3C2410_GPFDAT);
	__raw_writel(0x000000ff, S3C2410_GPFUP);

	
	if (machine_is_n30())
		__raw_writel(0xff0a956a, S3C2410_GPGCON);
	if (machine_is_n35())
		__raw_writel(0xff4aa92a, S3C2410_GPGCON);
	__raw_writel(0x0000e800, S3C2410_GPGDAT);
	__raw_writel(0x0000f86f, S3C2410_GPGUP);

	
	__raw_writel(0x0028aaaa, S3C2410_GPHCON);
	__raw_writel(0x000005ef, S3C2410_GPHDAT);
	__raw_writel(0x0000063f, S3C2410_GPHUP);
}

static void __init n30_map_io(void)
{
	s3c24xx_init_io(n30_iodesc, ARRAY_SIZE(n30_iodesc));
	n30_hwinit();
	s3c24xx_init_clocks(0);
	s3c24xx_init_uarts(n30_uartcfgs, ARRAY_SIZE(n30_uartcfgs));
}

static void __init n30_init_irq(void)
{
	s3c24xx_init_irq();
}



static void __init n30_init(void)
{
	s3c24xx_fb_set_platdata(&n30_fb_info);
	s3c_device_i2c0.dev.platform_data = &n30_i2ccfg;
	s3c24xx_udc_set_platdata(&n30_udc_cfg);

	

	s3c2410_modify_misccr(S3C2410_MISCCR_USBHOST |
			      S3C2410_MISCCR_USBSUSPND0 |
			      S3C2410_MISCCR_USBSUSPND1, 0x0);

	if (machine_is_n30()) {
		
		s3c2410_modify_misccr(S3C2410_MISCCR_USBHOST |
				      S3C2410_MISCCR_USBSUSPND0 |
				      S3C2410_MISCCR_USBSUSPND1, 0x0);

		platform_add_devices(n30_devices, ARRAY_SIZE(n30_devices));
	}

	if (machine_is_n35()) {
		
		s3c2410_modify_misccr(S3C2410_MISCCR_USBHOST |
				      S3C2410_MISCCR_USBSUSPND0 |
				      S3C2410_MISCCR_USBSUSPND1,
				      S3C2410_MISCCR_USBSUSPND1);

		platform_add_devices(n35_devices, ARRAY_SIZE(n35_devices));
	}
}

MACHINE_START(N30, "Acer-N30")
	
	.phys_io	= S3C2410_PA_UART,
	.io_pg_offst	= (((u32)S3C24XX_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S3C2410_SDRAM_PA + 0x100,
	.timer		= &s3c24xx_timer,
	.init_machine	= n30_init,
	.init_irq	= n30_init_irq,
	.map_io		= n30_map_io,
MACHINE_END

MACHINE_START(N35, "Acer-N35")
	
	.phys_io	= S3C2410_PA_UART,
	.io_pg_offst	= (((u32)S3C24XX_VA_UART) >> 18) & 0xfffc,
	.boot_params	= S3C2410_SDRAM_PA + 0x100,
	.timer		= &s3c24xx_timer,
	.init_machine	= n30_init,
	.init_irq	= n30_init_irq,
	.map_io		= n30_map_io,
MACHINE_END
