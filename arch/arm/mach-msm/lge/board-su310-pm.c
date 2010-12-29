

#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/gpio.h>
#include <mach/board.h>
#include <mach/board_lge.h>
#include "board-su310.h"




int su310_pwrsink_suspend_noirq(struct device *dev)
{
	printk(KERN_INFO"%s: configure gpio for suspend\n", __func__);

#if 0
	camera_power_mutex_lock();

	if(camera_power_state == CAM_POWER_ON)
	{
		camera_power_mutex_unlock();
		return 0;
	}
#endif 

	gpio_tlmm_config(GPIO_CFG(GPIO_LCD_BL_EN, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), GPIO_ENABLE);

	gpio_tlmm_config(GPIO_CFG(GPIO_BL_I2C_SCL, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);

	gpio_tlmm_config(GPIO_CFG(GPIO_BL_I2C_SDA, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);



	gpio_tlmm_config(GPIO_CFG(GPIO_LCD_MAKER_LOW, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	gpio_direction_output(GPIO_LCD_MAKER_LOW, 0);

	gpio_tlmm_config(GPIO_CFG(GPIO_LCD_RESET_N, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), GPIO_ENABLE);
	gpio_direction_output(GPIO_LCD_RESET_N, 0);

#if 0
	lcd_bl_power_state = BL_POWER_SUSPEND;
	camera_power_mutex_unlock();
#endif 

	return 0;
}

int su310_pwrsink_resume_noirq(struct device *dev)
{
	printk(KERN_INFO"%s: configure gpio for resume\n", __func__);

#if 0
	camera_power_mutex_lock();

	if(camera_power_state == CAM_POWER_ON || lcd_bl_power_state == BL_POWER_RESUME)
	{
		camera_power_mutex_unlock();
		return 0;
	}
#endif 

	gpio_tlmm_config(GPIO_CFG(GPIO_LCD_BL_EN, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	gpio_direction_output(GPIO_LCD_BL_EN, 1);

	gpio_tlmm_config(GPIO_CFG(GPIO_BL_I2C_SCL, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	gpio_direction_output(GPIO_BL_I2C_SCL, 1);

	gpio_tlmm_config(GPIO_CFG(GPIO_BL_I2C_SDA, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	gpio_direction_output(GPIO_BL_I2C_SDA, 1);



	gpio_tlmm_config(GPIO_CFG(GPIO_LCD_MAKER_LOW, 0, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), GPIO_ENABLE);

	gpio_tlmm_config(GPIO_CFG(GPIO_LCD_RESET_N, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	gpio_direction_output(GPIO_LCD_RESET_N, 0);

#if 0
	lcd_bl_power_state = BL_POWER_RESUME;

	camera_power_mutex_unlock();
#endif 

	return 0;
}

#if 0
void thunderg_pwrsink_resume()
{
	gpio_tlmm_config(GPIO_CFG(GPIO_LCD_BL_EN, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	gpio_direction_output(GPIO_LCD_BL_EN, 1);

	gpio_tlmm_config(GPIO_CFG(GPIO_BL_I2C_SCL, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	gpio_direction_output(GPIO_BL_I2C_SCL, 1);

	gpio_tlmm_config(GPIO_CFG(GPIO_BL_I2C_SDA, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	gpio_direction_output(GPIO_BL_I2C_SDA, 1);



	gpio_tlmm_config(GPIO_CFG(GPIO_LCD_MAKER_LOW, 0, GPIO_INPUT, GPIO_PULL_UP, GPIO_2MA), GPIO_ENABLE);

	gpio_tlmm_config(GPIO_CFG(GPIO_LCD_RESET_N, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	gpio_direction_output(GPIO_LCD_RESET_N, 0);

	lcd_bl_power_state = BL_POWER_RESUME;
}
#endif 

static struct dev_pm_ops su310_pwrsink_data = {
	.suspend_noirq = su310_pwrsink_suspend_noirq,
	.resume_noirq = su310_pwrsink_resume_noirq,
};

static struct platform_device su310_pwrsink_device = {
	.name = "lge-pwrsink",
	.id = -1,
	.dev = {
		.platform_data = &su310_pwrsink_data,
	},
};

void __init lge_add_pm_devices(void)
{



	platform_device_register(&su310_pwrsink_device);
}

