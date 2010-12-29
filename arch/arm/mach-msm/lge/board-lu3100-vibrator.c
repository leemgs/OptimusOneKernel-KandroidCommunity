

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/workqueue.h>  
#include <asm/io.h>
#include <mach/gpio.h>
#include <mach/msm_iomap.h>
#include <mach/timed_output.h>
#include <mach/vreg.h>
#include <mach/board_lge.h> 
#include "board-lu3100.h"

#define DEBUG_TS 0 
#if DEBUG_TS
#define DMSG(fmt, args...)  printk(KERN_ERR fmt, ##args) 
#else
#define DMSG(fmt, args...) do{} while(0)
#endif


static struct work_struct work_vibrator_on;
static struct work_struct work_vibrator_off;
static struct hrtimer vibe_timer;
static unsigned int vibe_timeout = 0;


#define GPIO_LIN_MOTOR_PWM		28
#define GPIO_LIN_MOTOR_EN       76

#define GP_MN_CLK_MDIV			0x004C
#define GP_MN_CLK_NDIV			0x0050
#define GP_MN_CLK_DUTY			0x0054


#define GPMN_M_DEFAULT			21
#define GPMN_N_DEFAULT			4500
#define GPMN_D_DEFAULT              3200  
#define PWM_MULTIPLIER              2560  


#define GPMN_M_MASK				0x01FF
#define GPMN_N_MASK				0x1FFF
#define GPMN_D_MASK				0x1FFF

#define REG_WRITEL(value, reg)	writel(value, (MSM_WEB_BASE+reg))

static atomic_t s_vibstate = ATOMIC_INIT(0);

static atomic_t s_vibrator = ATOMIC_INIT(0);
static atomic_t s_amp = ATOMIC_INIT(100);

static struct vreg *s_vreg_vibrator;

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

static int vibrator_motor_power_switch(int on)
{
    if(on) {
        return pm7540_gp3_poweron();
    } else {
        return pm7540_gp3_poweroff();
    }
}

static void vibrator_disable(void)
{
	
	REG_WRITEL((GPMN_D_DEFAULT & GPMN_D_MASK), GP_MN_CLK_DUTY);
	gpio_set_value(GPIO_LIN_MOTOR_EN, 0);
	gpio_tlmm_config(GPIO_CFG(GPIO_LIN_MOTOR_PWM, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	
	
    vibrator_motor_power_switch(0);

    atomic_set(&s_vibstate, 0);
}

static void vibrator_enable(void)
{
	
	gpio_tlmm_config(GPIO_CFG(GPIO_LIN_MOTOR_PWM, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), GPIO_ENABLE);

    REG_WRITEL((GPMN_M_DEFAULT & GPMN_M_MASK), GP_MN_CLK_MDIV);
    REG_WRITEL((~(GPMN_N_DEFAULT - GPMN_M_DEFAULT) & GPMN_N_MASK), GP_MN_CLK_NDIV);

    gpio_set_value(GPIO_LIN_MOTOR_EN, 1);

    vibrator_motor_power_switch(1);

    atomic_set(&s_vibstate, 1);
}

static void vibrator_set(int amp)
{
    int gain;

    if(amp == 0) {
        if(atomic_read(&s_vibstate)) {
	        vibrator_disable();
        }
    }
    else {
        if(!atomic_read(&s_vibstate)) {
	        vibrator_enable();
        }

        gain = ((PWM_MULTIPLIER * amp) >> 8) + GPMN_D_DEFAULT;
        REG_WRITEL((gain & GPMN_D_MASK), GP_MN_CLK_DUTY);
    }
}

static void vibrator_work_func(struct work_struct *work)
{
	vibrator_set(0);
}

static ssize_t vibrator_amp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int amp = atomic_read(&s_amp);
    return sprintf(buf, "%d\n", amp);
}

static ssize_t vibrator_amp_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    int value;

    sscanf(buf, "%d", &value);
    atomic_set(&s_amp, value);

    return size;
}

static DEVICE_ATTR(amp, S_IRUGO | S_IWUSR, vibrator_amp_show, vibrator_amp_store);

static void hrtimer_vibrator_on(struct work_struct *work)
{
	vibrator_set(atomic_read(&s_amp));
	hrtimer_start(&vibe_timer,
		      ktime_set(vibe_timeout / 1000, (vibe_timeout % 1000) * 1000000),
		      HRTIMER_MODE_REL);
}

static void hrtimer_vibrator_off(struct work_struct *work)
{
	vibrator_set(0);
}

static void timed_vibrator_on(struct timed_output_dev *sdev)
{
	schedule_work(&work_vibrator_on);
}

static void timed_vibrator_off(struct timed_output_dev *sdev)
{
	schedule_work(&work_vibrator_off);
}

static void timed_vibrator_enable(struct timed_output_dev *dev, int value)
{
	hrtimer_cancel(&vibe_timer);

	if (value == 0)
		timed_vibrator_off(dev);
	else 
	{
		value = (value > 15000 ? 15000 : value);
		value = (value < 20 ? 20 : value);
		timed_vibrator_on(dev);
	}
	vibe_timeout = value;
}

static int timed_vibrator_get_time(struct timed_output_dev *dev)
{
	if (hrtimer_active(&vibe_timer)) {
		ktime_t r = hrtimer_get_remaining(&vibe_timer);
		return r.tv.sec * 1000 + r.tv.nsec / 1000000;
	} else
		return 0;
}

static enum hrtimer_restart timed_vibrator_timer_func(struct hrtimer *timer)
{
	
	vibrator_set(0);
	return HRTIMER_NORESTART;
}

static struct timed_output_dev timed_vibrator = {
	.name = "vibrator",
	.get_time = timed_vibrator_get_time,
	.enable = timed_vibrator_enable,
};

void __init lge_add_init_timed_vibrator(void)
{
	int ret;
	
	INIT_WORK(&work_vibrator_on, hrtimer_vibrator_on);
	INIT_WORK(&work_vibrator_off, hrtimer_vibrator_off);

	hrtimer_init(&vibe_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	vibe_timer.function = timed_vibrator_timer_func;

	ret = timed_output_dev_register(&timed_vibrator);
	if(ret < 0) {
		printk("timed_output_dev_register error!\n");
		return;
	}

	ret = device_create_file(timed_vibrator.dev, &dev_attr_amp);
	if (ret < 0) {
		printk("device_create_file error!\n");
		timed_output_dev_unregister(&timed_vibrator);
		return;
	}




	gpio_request(GPIO_LIN_MOTOR_EN, "gpio_lin_motor_en");
	gpio_request(GPIO_LIN_MOTOR_PWM, "gpio_lin_motor_pwm");

	
	REG_WRITEL((GPMN_M_DEFAULT & GPMN_M_MASK), GP_MN_CLK_MDIV);
	REG_WRITEL((~(GPMN_N_DEFAULT - GPMN_M_DEFAULT) & GPMN_N_MASK), GP_MN_CLK_NDIV);

	
	gpio_set_value(GPIO_LIN_MOTOR_EN, 0);
}

MODULE_AUTHOR("----------------------@------->");
MODULE_DESCRIPTION("ALESSI vibrator driver for Android");
MODULE_LICENSE("GPL");
