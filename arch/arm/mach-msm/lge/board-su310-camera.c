
#include <linux/types.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <asm/setup.h>
#include <mach/gpio.h>
#include <mach/board.h>
#include <mach/camera.h>
#include <mach/msm_iomap.h>
#include <mach/vreg.h>
#include <mach/board_lge.h>

#include "board-su310.h"

int mclk_rate = 24000000;

extern int aat2870bl_ldo_set_level(struct device * dev, unsigned num, unsigned vol);
extern int aat2870bl_ldo_enable(struct device * dev, unsigned num, unsigned enable);

struct i2c_board_info i2c_devices[1] = {
#if defined (CONFIG_ISX006)
	{
		I2C_BOARD_INFO("isx006", CAM_I2C_SLAVE_ADDR),
	},
#endif
};

#if defined (CONFIG_MSM_CAMERA)
static uint32_t camera_off_gpio_table[] = {
	
	GPIO_CFG(4,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(5,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(6,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(7,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(8,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(9,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(10, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(11, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(12, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(13, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(14, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(GPIO_CAM_MCLK, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), 
};

static uint32_t camera_on_gpio_table[] = {
	
	GPIO_CFG(4,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(5,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(6,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(7,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(8,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(9,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(10, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(11, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(12, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_16MA), 
	GPIO_CFG(13, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(14, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), 
	GPIO_CFG(GPIO_CAM_MCLK, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_16MA), 
};

static void config_gpio_table(uint32_t *table, int len)
{
	int n, rc;
	for (n = 0; n < len; n++) {
		rc = gpio_tlmm_config(table[n], GPIO_ENABLE);
		if (rc) {
			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, table[n], rc);
			break;
		}
	}
}

void config_camera_on_gpios(void)
{
	config_gpio_table(camera_on_gpio_table,
		ARRAY_SIZE(camera_on_gpio_table));
}

void config_camera_off_gpios(void)
{
	config_gpio_table(camera_off_gpio_table,
		ARRAY_SIZE(camera_off_gpio_table));
}

int camera_power_on (void)
{
	struct device *dev = su310_backlight_dev();
	printk("dev = %d\n",dev);

	



    aat28xx_ldo_set_level(dev, 1, 1200);

	aat28xx_ldo_set_level(dev, 4, 2700);

	aat28xx_ldo_set_level(dev, 2, 2700);

	aat28xx_ldo_set_level(dev, 3, 2800);

	
	aat28xx_ldo_enable(dev, 1, 1);	
	mdelay(1);

	aat28xx_ldo_enable(dev, 4, 1);	
	mdelay(1);
	aat28xx_ldo_enable(dev, 2, 1);	
	mdelay(1);
	aat28xx_ldo_enable(dev, 3, 1);	
	mdelay(5);

#if 0
	msm_camio_clk_rate_set(24000000);
	mdelay(5);
	msm_camio_camif_pad_reg_reset();

	mdelay(5);

	gpio_set_value(0, 1);	
	
#endif
	return 0;
}

int camera_power_off (void)
{
	struct device *dev = su310_backlight_dev();


	






    aat28xx_ldo_set_level(dev, 3, 0);
	aat28xx_ldo_set_level(dev, 2, 0);
	aat28xx_ldo_set_level(dev, 4, 0);
	aat28xx_ldo_set_level(dev, 1, 0);
	aat28xx_ldo_enable(dev, 3, 0);	
	aat28xx_ldo_enable(dev, 2, 0);
	aat28xx_ldo_enable(dev, 4, 0);

	aat28xx_ldo_enable(dev, 1, 0);






	return 0;
}

static struct msm_camera_device_platform_data msm_camera_device_data = {
	.camera_gpio_on  = config_camera_on_gpios,
	.camera_gpio_off = config_camera_off_gpios,
	.ioext.mdcphy = MSM_MDC_PHYS,
	.ioext.mdcsz  = MSM_MDC_SIZE,
	.ioext.appphy = MSM_CLK_CTL_PHYS,
	.ioext.appsz  = MSM_CLK_CTL_SIZE,
	.camera_power_on = camera_power_on,
	.camera_power_off = camera_power_off,
};

#if defined (CONFIG_ISX006)
static struct msm_camera_sensor_flash_data flash_none = {
	.flash_type = MSM_CAMERA_FLASH_NONE,
};

static struct msm_camera_sensor_info msm_camera_sensor_isx006_data = {
	.sensor_name    = "isx006",
	.sensor_reset   = GPIO_CAM_RESET,
	.sensor_pwd     = GPIO_CAM_PWDN,
	.vcm_pwd        = 0,
	.vcm_enable		= 0,
	.pdata          = &msm_camera_device_data,
	.flash_data		= &flash_none,
};

static struct platform_device msm_camera_sensor_isx006 = {
	.name      = "msm_camera_isx006",
	.dev       = {
		.platform_data = &msm_camera_sensor_isx006_data,
	},
};
#endif

#endif

static struct platform_device *su310_camera_devices[] __initdata = {
#if defined (CONFIG_ISX006)
	&msm_camera_sensor_isx006,
#endif
};

void __init lge_add_camera_devices(void)
{
	platform_add_devices(su310_camera_devices, ARRAY_SIZE(su310_camera_devices));
}
