

#include <linux/types.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/power_supply.h>
#include <linux/interrupt.h>
#include <asm/setup.h>
#include <mach/gpio.h>
#include <mach/vreg.h>
#include <mach/pmic.h>
#include <mach/msm_battery.h>
#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <asm/io.h>
#include <mach/rpc_server_handset.h>
#include <mach/board_lge.h>
#include "board-su310.h"

static u32 su310_battery_capacity(u32 current_soc)
{
	if(current_soc > 100)
		current_soc = 100;

	return current_soc;
}

static struct msm_psy_batt_pdata msm_psy_batt_data = {
	.voltage_min_design     = 3200,
	.voltage_max_design     = 4200,
	.avail_chg_sources      = AC_CHG | USB_CHG ,
	.batt_technology        = POWER_SUPPLY_TECHNOLOGY_LION,
	.calculate_capacity		= su310_battery_capacity,
};

static struct platform_device msm_batt_device = {
	.name           = "msm-battery",
	.id         = -1,
	.dev.platform_data  = &msm_psy_batt_data,
};


#define GPIO_LIN_MOTOR_PWM		28
#define GPIO_LIN_MOTOR_EN       76

#define GP_MN_CLK_MDIV_REG			0x004C
#define GP_MN_CLK_NDIV_REG			0x0050
#define GP_MN_CLK_DUTY_REG			0x0054


#define GPMN_M_DEFAULT			21
#define GPMN_N_DEFAULT			4500
#define GPMN_D_DEFAULT              3200  
#define PWM_MAX_HALF_DUTY              2560  


#define GPMN_M_MASK				0x01FF
#define GPMN_N_MASK				0x1FFF
#define GPMN_D_MASK				0x1FFF

#define REG_WRITEL(value, reg)	writel(value, (MSM_WEB_BASE+reg))

static struct vreg *s_vreg_vibrator;



#define MAX_BACKLIGHT_LEVEL	16	
#define TUNED_MAX_BACKLIGHT_LEVEL	40	

extern int aat2870bl_ldo_set_level(struct device * dev, unsigned num, unsigned vol);
extern int aat2870bl_ldo_enable(struct device * dev, unsigned num, unsigned enable);

static char *dock_state_string[] = {
	"0",
	"1",
	"2",
};

enum {
	DOCK_STATE_UNDOCKED = 0,
	DOCK_STATE_DESK = 1, 
	DOCK_STATE_CAR = 2, 
	DOCK_STATE_UNKNOWN,
};

enum {
	KIT_DOCKED = 0,
	KIT_UNDOCKED = 1,
};

static void su310_desk_dock_detect_callback(int state)
{
	int ret;

	if (state)
		state = DOCK_STATE_DESK;

	ret = lge_gpio_switch_pass_event("dock", state);

	if (ret)
		printk(KERN_INFO "%s: desk dock event report fail\n", __func__);

	return;
}

static int su310_register_callback(void)
{
	rpc_server_hs_register_callback(su310_desk_dock_detect_callback);

	return 0;
}

static int su310_gpio_carkit_work_func(void)
{
	return DOCK_STATE_UNDOCKED;
}

static char *su310_gpio_carkit_print_state(int state)
{
	return dock_state_string[state];
}

static int su310_gpio_carkit_sysfs_store(const char *buf, size_t size)
{
	int state;

	if (!strncmp(buf, "undock", size-1))
		state = DOCK_STATE_UNDOCKED;
	else if (!strncmp(buf, "desk", size-1))
		state = DOCK_STATE_DESK;
	else if (!strncmp(buf, "car", size-1))
		state = DOCK_STATE_CAR;
	else
		return -EINVAL;

	return state;
}

static unsigned su310_carkit_gpios[] = {
};

static struct lge_gpio_switch_platform_data su310_carkit_data = {
	.name = "dock",
	.gpios = su310_carkit_gpios,
	.num_gpios = ARRAY_SIZE(su310_carkit_gpios),
	.irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	.wakeup_flag = 1,
	.work_func = su310_gpio_carkit_work_func,
	.print_state = su310_gpio_carkit_print_state,
	.sysfs_store = su310_gpio_carkit_sysfs_store,
	.additional_init = su310_register_callback,
};

static struct platform_device su310_carkit_device = {
	.name = "lge-switch-gpio",
	.id = 0,
	.dev = {
		.platform_data = &su310_carkit_data,
	},
};

static struct platform_device msm_device_pmic_leds = {
	.name = "pmic-leds",
	.id = -1,
	.dev.platform_data = "button-backlight",
};


int pm7540_gp3_poweron(void)
{
	int rc;    
	s_vreg_vibrator = vreg_get(NULL, "rftx");
	rc = vreg_set_level(s_vreg_vibrator, 3000);
	if(rc != 0) {
		return -1;
    }
	vreg_enable(s_vreg_vibrator);
	return rc;
}

int pm7540_gp3_poweroff(void)
{
	int rc;    
	s_vreg_vibrator = vreg_get(NULL, "rftx");
	rc = vreg_set_level(s_vreg_vibrator, 0);
	vreg_disable(s_vreg_vibrator);
	return rc;
}

int su310_vibrator_power_set(int on)
{
    if(on) {
		gpio_set_value(GPIO_LIN_MOTOR_EN, 1);
        return pm7540_gp3_poweron();
    } else {
    	gpio_set_value(GPIO_LIN_MOTOR_EN, 0);
        return pm7540_gp3_poweroff();
    }
}


int su310_vibrator_pwn_set(int enable, int amp)
{
	int gain = ((PWM_MAX_HALF_DUTY*amp) >> 8)+ GPMN_D_DEFAULT;

	REG_WRITEL((GPMN_M_DEFAULT & GPMN_M_MASK), GP_MN_CLK_MDIV_REG);
	REG_WRITEL((~( GPMN_N_DEFAULT - GPMN_M_DEFAULT )&GPMN_N_MASK), GP_MN_CLK_NDIV_REG);
		
	if (enable) {		
		
		gpio_tlmm_config(GPIO_CFG(GPIO_LIN_MOTOR_PWM, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA), GPIO_ENABLE);
		REG_WRITEL((gain & GPMN_D_MASK), GP_MN_CLK_DUTY_REG);
		gpio_direction_output(GPIO_LIN_MOTOR_PWM, 1);
	} else {
		REG_WRITEL(GPMN_D_DEFAULT, GP_MN_CLK_DUTY_REG);
		
		gpio_tlmm_config(GPIO_CFG(GPIO_LIN_MOTOR_PWM, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), GPIO_ENABLE);
		gpio_direction_output(GPIO_LIN_MOTOR_PWM, 0);		
	}
	
	return 0;
}

int su310_vibrator_ic_enable_set(int enable)
{
	
	return 0;
}

static struct android_vibrator_platform_data su310_vibrator_data = {
	.enable_status = 0,	
	.power_set = su310_vibrator_power_set,
	.pwn_set = su310_vibrator_pwn_set,
	.ic_enable_set = su310_vibrator_ic_enable_set,
};

static struct platform_device android_vibrator_device = {
	.name   = "android-vibrator",
	.id = -1,
	.dev = {
		.platform_data = &su310_vibrator_data,
	},
};


static char *ear_state_string[] = {
	"0",
	"1",
};

enum {
	EAR_STATE_EJECT = 0,
	EAR_STATE_INJECT = 1, 
};

enum {
	EAR_EJECT = 0,
	EAR_INJECT = 1,
};

static int su310_gpio_earsense_work_func(void)
{
	int state;
	int gpio_value;
	
	gpio_value = gpio_get_value(GPIO_EAR_SENSE);
	printk(KERN_INFO"%s: ear sense detected : %s\n", __func__, 
			gpio_value?"injected":"ejected");
	if (gpio_value == EAR_EJECT) {
		state = EAR_STATE_EJECT;
		gpio_set_value(GPIO_HS_MIC_BIAS_EN, 0);
	} else {
		state = EAR_STATE_INJECT;
		gpio_set_value(GPIO_HS_MIC_BIAS_EN, 1);
	}

	return state;
}

static char *su310_gpio_earsense_print_state(int state)
{
	return ear_state_string[state];
}

static int su310_gpio_earsense_sysfs_store(const char *buf, size_t size)
{
	int state;

	if (!strncmp(buf, "eject", size - 1))
		state = EAR_STATE_EJECT;
	else if (!strncmp(buf, "inject", size - 1))
		state = EAR_STATE_INJECT;
	else
		return -EINVAL;

	return state;
}

static unsigned su310_earsense_gpios[] = {
	GPIO_EAR_SENSE,
};

static struct lge_gpio_switch_platform_data su310_earsense_data = {
	.name = "h2w",
	.gpios = su310_earsense_gpios,
	.num_gpios = ARRAY_SIZE(su310_earsense_gpios),
	.irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	.wakeup_flag = 1,
	.work_func = su310_gpio_earsense_work_func,
	.print_state = su310_gpio_earsense_print_state,
	.sysfs_store = su310_gpio_earsense_sysfs_store,
};

static struct platform_device su310_earsense_device = {
	.name   = "lge-switch-gpio",
	.id = 1,
	.dev = {
		.platform_data = &su310_earsense_data,
	},
};


static struct platform_device *su310_misc_devices[] __initdata = {
	&msm_batt_device,
	&android_vibrator_device,
	&su310_carkit_device,

	

};


void __init lge_add_misc_devices(void)
{
	platform_add_devices(su310_misc_devices, ARRAY_SIZE(su310_misc_devices));
	platform_device_register(&msm_device_pmic_leds);
}

