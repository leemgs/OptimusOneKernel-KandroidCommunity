

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

void s3c64xx_setup_sdhci0_cfg_gpio(struct platform_device *dev, int width)
{
	unsigned int gpio;
	unsigned int end;

	end = S3C64XX_GPG(2 + width);

	
	for (gpio = S3C64XX_GPG(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(2));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}

	s3c_gpio_setpull(S3C64XX_GPG(6), S3C_GPIO_PULL_UP);
	s3c_gpio_cfgpin(S3C64XX_GPG(6), S3C_GPIO_SFN(2));
}

void s3c64xx_setup_sdhci1_cfg_gpio(struct platform_device *dev, int width)
{
	unsigned int gpio;
	unsigned int end;

	end = S3C64XX_GPH(2 + width);

	
	for (gpio = S3C64XX_GPH(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(2));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}

	s3c_gpio_setpull(S3C64XX_GPG(6), S3C_GPIO_PULL_UP);
	s3c_gpio_cfgpin(S3C64XX_GPG(6), S3C_GPIO_SFN(3));
}
