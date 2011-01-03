#if defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)


#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/backlight.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <mach/board_lge.h>

#define MODULE_NAME  "aat2870bl"
#define CONFIG_BACKLIGHT_LEDS_CLASS

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
#include <linux/leds.h>
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif













#define LCD_LED_MAX 22  
#define LCD_LED_MIN 0  
#define LCD_LED_DIM 1
#define DEFAULT_BRIGHTNESS 12
#define AAT28XX_LDO_NUM 4

#define AAT2862BL_REG_BLM   0x03  
#define AAT2862BL_REG_BLS   0x04  
#define AAT2862BL_REG_LDOAB 0x00  
#define AAT2862BL_REG_LDOCD 0x01  
#define AAT2862BL_REG_LDOEN 0x02  

#define AAT2870BL_REG_BLM   0x01  
#define AAT2870BL_REG_ALS_LEVEL 0x11 
#define AAT2870BL_REG_LDOAB 0x24  
#define AAT2870BL_REG_LDOCD 0x25  
#define AAT2870BL_REG_LDOEN 0x26  

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
#define LEDS_BACKLIGHT_NAME "lcd-backlight"
#endif

#define GPIO_LCD_BL_EN		82
#define GPIO_BL_I2C_SCL		88
#define GPIO_BL_I2C_SDA		89

enum {
	ALC_MODE,
	NORMAL_MODE,
} AAT2870BL_MODE;

enum {
	UNINIT_STATE=-1,
	POWERON_STATE,
	NORMAL_STATE,
	SLEEP_STATE,
	POWEROFF_STATE
} AAT2870BL_STATE;

#define dprintk(fmt, args...) \
	do { \
		if (debug) \
			printk(KERN_INFO "%s:%s: " fmt, MODULE_NAME, __func__, ## args); \
	} while(0);

#define eprintk(fmt, args...)   printk(KERN_ERR "%s:%s: " fmt, MODULE_NAME, __func__, ## args)

struct ldo_vout_struct {
	unsigned char reg;
	unsigned vol;
};

struct aat28xx_ctrl_tbl {
	unsigned char reg;
	unsigned char val;
};

struct aat28xx_reg_addrs {
	unsigned char bl_m;
	unsigned char bl_s;
	unsigned char ldo_ab;
	unsigned char ldo_cd;
	unsigned char ldo_en;
};

struct aat28xx_cmds {
	struct aat28xx_ctrl_tbl *normal;
	struct aat28xx_ctrl_tbl *alc;
	struct aat28xx_ctrl_tbl *sleep;
};

struct aat28xx_driver_data {
	struct i2c_client *client;
	struct backlight_device *bd;
	struct led_classdev *led;
	int gpio;
	int intensity;
	int max_intensity;
	int mode;
	int state;
	int ldo_ref[AAT28XX_LDO_NUM];
	unsigned char reg_ldo_enable;
	unsigned char reg_ldo_vout[2];
	int version;
	struct aat28xx_cmds cmds;
	struct aat28xx_reg_addrs reg_addrs;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
	int has_alc;
	int is_charging;
};


static unsigned int debug = 0;
module_param(debug, uint, 0644);


static struct aat28xx_ctrl_tbl aat2862bl_normal_tbl[] = {
	
	{ 0x03, 0xF2 },  
	{ 0xFF, 0xFE }	 
};


static struct aat28xx_ctrl_tbl aat2862bl_alc_tbl[] = {
	
	{ 0xFF, 0xFE }   
};


static struct aat28xx_ctrl_tbl aat2862bl_sleep_tbl[] = {
	{ 0x03, 0xDF }, 
	{ 0xFF, 0xFE },  	
};


static struct aat28xx_ctrl_tbl aat2870bl_normal_tbl[] = {
	{ 0x00, 0xFF },  
	{ 0x0E, 0x26 },  
	{ 0x0F, 0x06 },  
	{ 0xFF, 0xFE }	 
};


static struct aat28xx_ctrl_tbl aat2870bl_alc_tbl[] = {
	{ 0x12, 0x0C },  
	{ 0x13, 0x0D },
	{ 0x14, 0x0D },
	{ 0x15, 0x0D },
	{ 0x16, 0x0D },
	{ 0x17, 0x0E },
	{ 0x18, 0x0E },
	{ 0x19, 0x0E },
	{ 0x1A, 0x0E },
	{ 0x1B, 0x0F },
	{ 0x1C, 0x0F },
	{ 0x1D, 0x0F },
	{ 0x1E, 0x10 },
	{ 0x1F, 0x11 },
	{ 0x20, 0x11 },
	{ 0x21, 0x11 },

	{ 0x0E, 0x71 },  
	{ 0x0F, 0x01 },  
	{ 0x10, 0x1D },  
	{ 0x00, 0xFF },  		
	{ 0xFF, 0xFE }   
};


static struct aat28xx_ctrl_tbl aat2870bl_sleep_tbl[] = {

	{ 0x0E, 0x26 },  
	{ 0x0F, 0x06 },  
	{ 0x00, 0x00 },  
	{ 0xFF, 0xFE },  	
};

static struct ldo_vout_struct ldo_vout_table[] = {
	{ 0x00, 1200},
	{ 0x01, 1300},
	{ 0x02, 1500},
	{ 0x03, 1600},
	{ 0x04, 1800},
	{ 0x05, 2000},
	{ 0x06, 2200},
	{ 0x07, 2500},
	{ 0x08, 2600},
	{ 0x09, 2700},
	{ 0x0A, 2800},
	{ 0x0B, 2900},
	{ 0x0C, 3000},
	{ 0x0D, 3100},
	{ 0x0E, 3200},
	{ 0x0F, 3300},
	{ 0xFF, 0},
};

static void aat28xx_poweron(struct aat28xx_driver_data *drvdata);


static int aat28xx_setup_version(struct aat28xx_driver_data *drvdata)
{
	if(!drvdata)
		return -ENODEV;

	if(drvdata->version == 2862) {
		drvdata->cmds.normal = aat2862bl_normal_tbl;
		drvdata->cmds.alc = aat2862bl_alc_tbl;
		drvdata->cmds.sleep = aat2862bl_sleep_tbl;
		drvdata->reg_addrs.bl_m = AAT2862BL_REG_BLM;
		drvdata->reg_addrs.bl_s = AAT2862BL_REG_BLS;
		drvdata->reg_addrs.ldo_ab = AAT2862BL_REG_LDOAB;
		drvdata->reg_addrs.ldo_cd = AAT2862BL_REG_LDOCD;
		drvdata->reg_addrs.ldo_en = AAT2862BL_REG_LDOEN;
	}
	else if(drvdata->version == 2870) {
		drvdata->cmds.normal = aat2870bl_normal_tbl;
		drvdata->cmds.alc = aat2870bl_alc_tbl;
		drvdata->cmds.sleep = aat2870bl_sleep_tbl;
		drvdata->reg_addrs.bl_m = AAT2870BL_REG_BLM;
		drvdata->reg_addrs.ldo_ab = AAT2870BL_REG_LDOAB;
		drvdata->reg_addrs.ldo_cd = AAT2870BL_REG_LDOCD;
		drvdata->reg_addrs.ldo_en = AAT2870BL_REG_LDOEN;
	}
	else {
		eprintk("Not supported version!!\n");
		return -ENODEV;
	}

	return 0;
}

static int aat28xx_read(struct i2c_client *client, u8 reg, u8 *pval)
{
	int ret;
	int status = 0;

	if (client == NULL) { 	
		eprintk("client is null\n");
		return -1;
	}

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		status = -EIO;
		eprintk("fail to read(reg=0x%x,val=0x%x)\n", reg,*pval);	
	}

	*pval = ret;
	return status;
}

static int aat28xx_write(struct i2c_client *client, u8 reg, u8 val)
{
	int ret;
	int status = 0;

	if (client == NULL) {	
		eprintk("client is null\n");
		return -1;
	}

	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret != 0) {
		status = -EIO;
		eprintk("fail to write(reg=0x%x,val=0x%x)\n", reg, val);
	}

	return status;
}

static int aat28xx_set_ldos(struct i2c_client *i2c_dev, unsigned num, int enable)
{
	struct aat28xx_driver_data *drvdata = i2c_get_clientdata(i2c_dev);

	if (drvdata) {
		if (enable) drvdata->reg_ldo_enable |= 1 << (num-1);
		else drvdata->reg_ldo_enable &= ~(1 << (num-1));
		
		dprintk("enable ldos, reg:0x13 value:0x%x\n", drvdata->reg_ldo_enable);
		
		return aat28xx_write(i2c_dev, drvdata->reg_addrs.ldo_en, drvdata->reg_ldo_enable);
	}
	return -EIO;
}

static unsigned char aat28xx_ldo_get_vout_val(unsigned vol)
{
	int i = 0;
	do {
		if (ldo_vout_table[i].vol == vol)
			return ldo_vout_table[i].reg;
		else
			i++;
	} while (ldo_vout_table[i].vol != 0);

	return ldo_vout_table[i].reg;
}

static int aat28xx_ldo_set_vout(struct i2c_client *i2c_dev, unsigned num, unsigned char val)
{
	struct aat28xx_driver_data *drvdata = i2c_get_clientdata(i2c_dev);
	unsigned char *next_val;
	unsigned char reg;

	if (drvdata) {
		if (num <= 2) {
			reg = drvdata->reg_addrs.ldo_ab;
			next_val = &drvdata->reg_ldo_vout[0];
		} else {
			reg = drvdata->reg_addrs.ldo_cd;
			next_val = &drvdata->reg_ldo_vout[1];
		}
		if (num % 2) {
			*next_val &= 0x0F;
			val = val << 4;		
		}
		else {
			*next_val &= 0xF0;		
		}
		*next_val |= val;
		dprintk("target register[0x%x], value[0x%x]\n",	reg, *next_val);
		return aat28xx_write(i2c_dev, reg, *next_val);
	}
	return -EIO;
}


int aat28xx_ldo_enable(struct device *dev, unsigned num, unsigned enable)
{
	struct i2c_adapter *adap;
	struct i2c_client *client;
	struct aat28xx_driver_data *drvdata;
	int err = 0;

	dprintk("ldo_no[%d], on/off[%d]\n",num, enable);

	if (num > 0 && num <= AAT28XX_LDO_NUM) {
		if ((adap=dev_get_drvdata(dev)) && (client=i2c_get_adapdata(adap))) {
			drvdata = i2c_get_clientdata(client);
			if (enable) {
				if (drvdata->ldo_ref[num-1]++ == 0) {
					dprintk("ref count = 0, call aat28xx_set_ldos\n");
					err = aat28xx_set_ldos(client, num, enable);
				}
			}
			else {
				if (--drvdata->ldo_ref[num-1] == 0) {
					dprintk("ref count = 0, call aat28xx_set_ldos\n");
					err = aat28xx_set_ldos(client, num, enable);
				}
			}
			return err;
		}
	}
	return -ENODEV;
}
EXPORT_SYMBOL(aat28xx_ldo_enable);


int aat28xx_ldo_set_level(struct device *dev, unsigned num, unsigned vol)
{
	struct i2c_adapter *adap;
	struct i2c_client *client;
	struct aat28xx_driver_data *drvdata;
	unsigned char val;	

	dprintk("ldo_no[%d], level[%d]\n", num, vol);
	if (num > 0 && num <= AAT28XX_LDO_NUM) {
		if ((adap=dev_get_drvdata(dev)) && (client=i2c_get_adapdata(adap))) {

#if (!CONFIG_MACH_MSM7X27_SU310 && !CONFIG_MACH_MSM7X27_LU3100) 
			
			if((num == LDO_CAM_DVDD_NO) && (vol != 0)){			
				drvdata = i2c_get_clientdata(client);

				if (drvdata->state == POWEROFF_STATE) {
					gpio_tlmm_config(GPIO_CFG(GPIO_LCD_BL_EN, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
					gpio_direction_output(GPIO_LCD_BL_EN, 1);
					gpio_tlmm_config(GPIO_CFG(GPIO_BL_I2C_SCL, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
					gpio_direction_output(GPIO_BL_I2C_SCL, 1);
					gpio_tlmm_config(GPIO_CFG(GPIO_BL_I2C_SDA, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
					gpio_direction_output(GPIO_BL_I2C_SDA, 1);	
					mdelay(10);
					aat28xx_poweron(drvdata);		
				} 
			}
#endif
			
			val = aat28xx_ldo_get_vout_val(vol);
			dprintk("vout register value 0x%x for level %d\n", val, vol);
			return aat28xx_ldo_set_vout(client, num, val);
		}
	}
	return -ENODEV;
}
EXPORT_SYMBOL(aat28xx_ldo_set_level);

static int aat28xx_set_table(struct aat28xx_driver_data *drvdata, struct aat28xx_ctrl_tbl *ptbl)
{
	unsigned int i = 0;
	unsigned long delay = 0;

	if (ptbl == NULL) {
		eprintk("input ptr is null\n");
		return -EIO;
	}

	for( ;;) {
		if (ptbl->reg == 0xFF) {
			if (ptbl->val != 0xFE) {
				delay = (unsigned long)ptbl->val;
				udelay(delay);
			}
			else
				break;
		}	
		else {
			if (aat28xx_write(drvdata->client, ptbl->reg, ptbl->val) != 0)
				dprintk("i2c failed addr:%d, value:%d\n", ptbl->reg, ptbl->val);
		}
		ptbl++;
		i++;
	}
	return 0;
}

static void aat28xx_hw_reset(struct aat28xx_driver_data *drvdata)
{
	if (drvdata->client && gpio_is_valid(drvdata->gpio)) {
		gpio_configure(drvdata->gpio, GPIOF_DRIVE_OUTPUT);
		
		gpio_set_value(drvdata->gpio, 0);
		udelay(5);
		gpio_set_value(drvdata->gpio, 1);
		udelay(5);
	}
}

static void aat28xx_go_opmode(struct aat28xx_driver_data *drvdata)
{
	if (!drvdata->has_alc)
		drvdata->mode = NORMAL_MODE;
		
	dprintk("operation mode is %s\n", (drvdata->mode == NORMAL_MODE) ? "normal_mode" : "alc_mode");
	
	switch (drvdata->mode) {
		case NORMAL_MODE:
			aat28xx_set_table(drvdata, drvdata->cmds.normal);
			drvdata->state = NORMAL_STATE;
			break;
		case ALC_MODE:
			aat28xx_set_table(drvdata, drvdata->cmds.alc);
			drvdata->state = NORMAL_STATE;
			break;
		default:
			eprintk("Invalid Mode\n");
			break;
	}
}

static void aat28xx_device_init(struct aat28xx_driver_data *drvdata)
{
	aat28xx_hw_reset(drvdata);
	aat28xx_go_opmode(drvdata);
}

static void aat28xx_poweron(struct aat28xx_driver_data *drvdata)
{
	unsigned int aat28xx_intensity;
	if (!drvdata || drvdata->state != POWEROFF_STATE)
		return;
	
	dprintk("POWER ON \n");

	aat28xx_device_init(drvdata);
	
	if (drvdata->mode == NORMAL_MODE)
	{
		if(drvdata->version == 2862)
		{
			aat28xx_intensity = (~(drvdata->intensity)& 0x1F);	
			aat28xx_intensity |= 0xE0;				
			aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, aat28xx_intensity);
		}
		else
			aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, drvdata->intensity);
	}
}

static void aat28xx_poweroff(struct aat28xx_driver_data *drvdata)
{
	if (!drvdata || drvdata->state == POWEROFF_STATE)
		return;

	dprintk("POWER OFF \n");

	if (drvdata->state == SLEEP_STATE) {
		gpio_direction_output(drvdata->gpio, 0);
		msleep(6);
		drvdata->state = POWEROFF_STATE;
		return;
	}

	gpio_tlmm_config(GPIO_CFG(drvdata->gpio, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	gpio_direction_output(drvdata->gpio, 0);
	mdelay(6);
	drvdata->state = POWEROFF_STATE;
}


static void aat28xx_sleep(struct aat28xx_driver_data *drvdata)
{
	if (!drvdata || drvdata->state == SLEEP_STATE)
		return;

	dprintk("operation mode is %s\n", (drvdata->mode == NORMAL_MODE) ? "normal_mode" : "alc_mode");
	
	switch (drvdata->mode) {
		case NORMAL_MODE:
			drvdata->state = SLEEP_STATE;
			aat28xx_set_table(drvdata, drvdata->cmds.sleep);
			break;

		case ALC_MODE:
			drvdata->state = SLEEP_STATE;
			aat28xx_set_table(drvdata, drvdata->cmds.sleep);
			udelay(500);
			break;

		default:
			eprintk("Invalid Mode\n");
			break;
	}
}

static void aat28xx_wakeup(struct aat28xx_driver_data *drvdata)
{
	unsigned int aat28xx_intensity;

	if (!drvdata || drvdata->state == NORMAL_STATE)
		return;

	dprintk("operation mode is %s\n", (drvdata->mode == NORMAL_MODE) ? "normal_mode" : "alc_mode");

	if (drvdata->state == POWEROFF_STATE) {
		aat28xx_poweron(drvdata);
		
		
	} else if (drvdata->state == SLEEP_STATE) {
		if (drvdata->mode == NORMAL_MODE) {
			if(drvdata->version == 2862) {
				aat28xx_intensity = (~(drvdata->intensity)& 0x1F);	
				aat28xx_intensity |= 0xE0;				
				aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, aat28xx_intensity);
			} else {
				aat28xx_set_table(drvdata, drvdata->cmds.normal);
				aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, drvdata->intensity);
			}
			drvdata->state = NORMAL_STATE;
		} else if (drvdata->mode == ALC_MODE) {
			aat28xx_set_table(drvdata, drvdata->cmds.alc);
			drvdata->state = NORMAL_STATE;
		}
	}
}

static int aat28xx_send_intensity(struct aat28xx_driver_data *drvdata, int next)
{
	int aat2862_bl_next;

	if (next > drvdata->max_intensity)
		next = drvdata->max_intensity;
	if (next < LCD_LED_MIN)
		next = LCD_LED_MIN;
	dprintk("next current is %d\n", next);

	if (drvdata->mode == NORMAL_MODE) {
		if (drvdata->state == NORMAL_STATE && drvdata->intensity != next)
		{
			
			if(drvdata->version == 2862)
			{
				if(next != 0)
				{
					aat2862_bl_next = (~next & 0x1F);	
					aat2862_bl_next |= 0xE0;		
					aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, aat2862_bl_next);
				}
				else
				{	
					aat2862_bl_next = 0xDF;		
					aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, aat2862_bl_next);					
				}
			}
			else	
				aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, next);
		}
		
		drvdata->intensity = next;
	}
	else if(drvdata->mode == ALC_MODE && drvdata->intensity != next && next != LCD_LED_MIN)
	{	
		if (drvdata->state == NORMAL_STATE )
		{
			if (next == LCD_LED_DIM)	{
				aat28xx_set_table(drvdata, drvdata->cmds.normal);
				aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, next);	
			} else if (drvdata->intensity == LCD_LED_DIM) {
				aat28xx_set_table(drvdata, drvdata->cmds.alc);		
			}
			drvdata->intensity = next;		
		}
	} 		
	return 0;
}

static int aat28xx_get_intensity(struct aat28xx_driver_data *drvdata)
{
	return drvdata->intensity;
}

static int aat28xx_get_alc_level(struct aat28xx_driver_data *drvdata)
{
	u8 level = 0;

	if (drvdata->mode == ALC_MODE) {
		if(aat28xx_read(drvdata->client, AAT2870BL_REG_ALS_LEVEL, &level))
			eprintk("Error while read register(0x13).\n");
		dprintk("alc level: %d\n", level);
	}
	else {
		eprintk("Current mode is not ALC mode\n");
	}

	
	return (level >> 3);
}

#ifdef CONFIG_PM
#ifdef CONFIG_HAS_EARLYSUSPEND
static void aat28xx_early_suspend(struct early_suspend * h)
{	
	struct aat28xx_driver_data *drvdata = container_of(h, struct aat28xx_driver_data,
						    early_suspend);

	dprintk("start\n");
	aat28xx_sleep(drvdata);

	return;
}

static void aat28xx_late_resume(struct early_suspend * h)
{	
	struct aat28xx_driver_data *drvdata = container_of(h, struct aat28xx_driver_data,
						    early_suspend);

	dprintk("start\n");
	if (drvdata->is_charging) 
		mdelay(300);
	mdelay(100);
	aat28xx_wakeup(drvdata);

	return;
}
#else
static int aat28xx_suspend(struct i2c_client *i2c_dev, pm_message_t state)
{
	struct aat28xx_driver_data *drvdata = i2c_get_clientdata(i2c_dev);
	aat28xx_sleep(drvdata);
	return 0;
}

static int aat28xx_resume(struct i2c_client *i2c_dev)
{
	struct aat28xx_driver_data *drvdata = i2c_get_clientdata(i2c_dev);
	aat28xx_wakeup(drvdata);
	return 0;
}
#endif	
#else
#define aat28xx_suspend	NULL
#define aat28xx_resume	NULL
#endif	

void aat28xx_switch_mode(struct device *dev, int next_mode)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev);
	unsigned int aat28xx_intensity;

	if (!drvdata || drvdata->mode == next_mode || !drvdata->has_alc)
		return;

	if (next_mode == ALC_MODE)
		aat28xx_set_table(drvdata, drvdata->cmds.alc);
	else if (next_mode == NORMAL_MODE) {
		aat28xx_set_table(drvdata, drvdata->cmds.normal);

		if(drvdata->version == 2862) {
			aat28xx_intensity = (~(drvdata->intensity)& 0x1F);	
			aat28xx_intensity |= 0xE0;				
			aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, aat28xx_intensity);
		} else {
			aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, drvdata->intensity);
		}
	} else {
		printk(KERN_ERR "%s: invalid mode(%d)!!!\n", __func__, next_mode);
		return;
	}

	drvdata->mode = next_mode;
	return;
}

ssize_t aat28xx_show_alc(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev->parent);
	int r;

	if (!drvdata) return 0;

	r = snprintf(buf, PAGE_SIZE, "%s\n", (drvdata->mode == ALC_MODE) ? "1":"0");
	
	return r;
}

ssize_t aat28xx_store_alc(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int alc;
	int next_mode;

	if (!count)
		return -EINVAL;

	sscanf(buf, "%d", &alc);

	if (alc)
		next_mode = ALC_MODE;
	else
		next_mode = NORMAL_MODE;

	aat28xx_switch_mode(dev->parent, next_mode);

	return count;
}

ssize_t aat28xx_show_reg(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev);
	int len = 0;
	unsigned char val;

	len += snprintf(buf,       PAGE_SIZE,       "\nAAT2870 Registers is following..\n");
	aat28xx_read(drvdata->client, 0x00, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[CH_EN(0x00)] = 0x%x\n", val);
	aat28xx_read(drvdata->client, 0x01, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[BLM(0x01)] = 0x%x\n", val);
	aat28xx_read(drvdata->client, 0x0E, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[ALS(0x0E)] = 0x%x\n", val);	
	aat28xx_read(drvdata->client, 0x0F, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[SBIAS(0x0F)] = 0x%x\n", val);
	aat28xx_read(drvdata->client, 0x10, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[ALS_GAIN(0x10)] = 0x%x\n", val);
	aat28xx_read(drvdata->client, 0x11, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[AMBIENT_LEVEL(0x11)] = 0x%x\n", val);

	return len;
}

ssize_t aat28xx_show_drvstat(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev->parent);
	int len = 0;

	len += snprintf(buf,       PAGE_SIZE,       "\nAAT2870 Backlight Driver Status is following..\n");
	len += snprintf(buf + len, PAGE_SIZE - len, "mode                   = %3d\n", drvdata->mode);
	len += snprintf(buf + len, PAGE_SIZE - len, "state                  = %3d\n", drvdata->state);
	len += snprintf(buf + len, PAGE_SIZE - len, "current intensity      = %3d\n", drvdata->intensity);

	return len;
}

ssize_t aat28xx_show_alc_level(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev->parent);
	int alc_level;
	int r;

	if (!drvdata)
		return 0;

	alc_level = aat28xx_get_alc_level(drvdata);
	if (drvdata->has_alc) {
		r = snprintf(buf, 40, "%d\n", alc_level);
	} else {
		r = snprintf(buf, 40, "%d\n", -1); 
	}

	return r;
}

ssize_t aat28xx_show_onoff(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev->parent);
	int r;

	if (!drvdata) return 0;

	r = snprintf(buf, PAGE_SIZE, "%s\n", (drvdata->state == POWEROFF_STATE) ? "0":"1");
	
	return r;
}

ssize_t aat28xx_store_onoff(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev->parent);
	int onoff;

	if (!count)
		return -EINVAL;

	sscanf(buf, "%d", &onoff);


	if (onoff)	{
		mdelay(100);
		aat28xx_wakeup(drvdata);
	}else{
		aat28xx_sleep(drvdata);
	}
	return count;
}

ssize_t aat28xx_show_chargingmode(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev->parent);
	int r;

	if (!drvdata) return 0;

	r = snprintf(buf, PAGE_SIZE, "%s\n", (drvdata->is_charging) ? "1":"0");
	
	return r;
}

ssize_t aat28xx_store_chargingmode(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev->parent);
	int bChargingmode;

	if (!count)
		return -EINVAL;

	sscanf(buf, "%d", &bChargingmode);

	drvdata->is_charging = bChargingmode;
	return count;
}

DEVICE_ATTR(alc, 0664, aat28xx_show_alc, aat28xx_store_alc);
DEVICE_ATTR(reg, 0444, aat28xx_show_reg, NULL);
DEVICE_ATTR(drvstat, 0444, aat28xx_show_drvstat, NULL);
DEVICE_ATTR(alc_level, 0444, aat28xx_show_alc_level, NULL);
DEVICE_ATTR(onoff, 0666, aat28xx_show_onoff, aat28xx_store_onoff);
DEVICE_ATTR(chargingmode, 0666, aat28xx_show_chargingmode, aat28xx_store_chargingmode);

static int aat28xx_set_brightness(struct backlight_device *bd)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(bd->dev.parent);
	return aat28xx_send_intensity(drvdata, bd->props.brightness);
}

static int aat28xx_get_brightness(struct backlight_device *bd)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(bd->dev.parent);
	return aat28xx_get_intensity(drvdata);
}

static struct backlight_ops aat28xx_ops = {
	.get_brightness = aat28xx_get_brightness,
	.update_status  = aat28xx_set_brightness,
};


#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
static void leds_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(led_cdev->dev->parent);
	int brightness;
	int next;

	if (!drvdata) {
		eprintk("Error getting drvier data\n");
		return;
	}

	brightness = aat28xx_get_intensity(drvdata);

	next = value * drvdata->max_intensity / LED_FULL;
	dprintk("input brightness value=%d]\n", next);

	if (brightness != next) {
		dprintk("brightness[current=%d, next=%d]\n", brightness, next);
		aat28xx_send_intensity(drvdata, next);
	}
}

static struct led_classdev aat28xx_led_dev = {
	.name = LEDS_BACKLIGHT_NAME,
	.brightness_set = leds_brightness_set,
};
#endif

static int __init aat28xx_probe(struct i2c_client *i2c_dev, const struct i2c_device_id *i2c_dev_id)
{
	struct aat28xx_platform_data *pdata;
	struct aat28xx_driver_data *drvdata;
	struct backlight_device *bd;
	int err;

	dprintk("start, client addr=0x%x\n", i2c_dev->addr);

	pdata = i2c_dev->dev.platform_data;
	if(!pdata)
		return -EINVAL;
		
	drvdata = kzalloc(sizeof(struct aat28xx_driver_data), GFP_KERNEL);
	if (!drvdata) {
		dev_err(&i2c_dev->dev, "failed to allocate memory\n");
		return -ENOMEM;
	}

	if (pdata && pdata->platform_init)
		pdata->platform_init();

	drvdata->client = i2c_dev;
	drvdata->gpio = pdata->gpio;
	drvdata->max_intensity = LCD_LED_MAX;
	if (pdata->max_current > 0)
		drvdata->max_intensity = pdata->max_current;
	drvdata->intensity = LCD_LED_MIN;
	drvdata->mode = NORMAL_MODE;
	drvdata->state = UNINIT_STATE;
	drvdata->has_alc = 1;
	drvdata->is_charging = 0;
	drvdata->version = pdata->version;

	if(aat28xx_setup_version(drvdata) != 0) {
		eprintk("Error while requesting gpio %d\n", drvdata->gpio);
		kfree(drvdata);
		return -ENODEV;
	}		
	if (drvdata->gpio && gpio_request(drvdata->gpio, "aat28xx_en") != 0) {
		eprintk("Error while requesting gpio %d\n", drvdata->gpio);
		kfree(drvdata);
		return -ENODEV;
	}

	bd = backlight_device_register("aat28xx-bl", &i2c_dev->dev, NULL, &aat28xx_ops);
	if (bd == NULL) {
		eprintk("entering aat28xx probe function error \n");
		if (gpio_is_valid(drvdata->gpio))
			gpio_free(drvdata->gpio);
		kfree(drvdata);
		return -1;
	}
	bd->props.power = FB_BLANK_UNBLANK;
	bd->props.brightness = drvdata->intensity;
	bd->props.max_brightness = drvdata->max_intensity;
	drvdata->bd = bd;

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
	if (led_classdev_register(&i2c_dev->dev, &aat28xx_led_dev) == 0) {
		eprintk("Registering led class dev successfully.\n");
		drvdata->led = &aat28xx_led_dev;
		err = device_create_file(drvdata->led->dev, &dev_attr_alc);
		err = device_create_file(drvdata->led->dev, &dev_attr_reg);
		err = device_create_file(drvdata->led->dev, &dev_attr_drvstat);
		err = device_create_file(drvdata->led->dev, &dev_attr_alc_level);
		err = device_create_file(drvdata->led->dev, &dev_attr_onoff);
		err = device_create_file(drvdata->led->dev, &dev_attr_chargingmode);		
	}
#endif

	i2c_set_clientdata(i2c_dev, drvdata);
	i2c_set_adapdata(i2c_dev->adapter, i2c_dev);

	aat28xx_device_init(drvdata);
	aat28xx_send_intensity(drvdata, DEFAULT_BRIGHTNESS);

#ifdef CONFIG_HAS_EARLYSUSPEND
	drvdata->early_suspend.suspend = aat28xx_early_suspend;
	drvdata->early_suspend.resume = aat28xx_late_resume;
	drvdata->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 40;
	register_early_suspend(&drvdata->early_suspend);
#endif

	eprintk("done\n");
	return 0;
}

static int __devexit aat28xx_remove(struct i2c_client *i2c_dev)
{
	struct aat28xx_driver_data *drvdata = i2c_get_clientdata(i2c_dev);

	aat28xx_send_intensity(drvdata, 0);

	backlight_device_unregister(drvdata->bd);
	led_classdev_unregister(drvdata->led);
	i2c_set_clientdata(i2c_dev, NULL);
	if (gpio_is_valid(drvdata->gpio))
		gpio_free(drvdata->gpio);
	kfree(drvdata);

	return 0;
}

static struct i2c_device_id aat28xx_idtable[] = {
	{ MODULE_NAME, 0 },
};

MODULE_DEVICE_TABLE(i2c, aat28xx_idtable);

#ifndef CONFIG_HAS_EARLYSUSPEND
static struct dev_pm_ops aat28xx_pm_ops = {
       .suspend = aat28xx_suspend,
       .resume = aat28xx_resume,
};
#endif

static struct i2c_driver aat28xx_driver = {
	.probe 		= aat28xx_probe,
	.remove 	= aat28xx_remove,
	.id_table 	= aat28xx_idtable,
	.driver = {
		.name = MODULE_NAME,
		.owner = THIS_MODULE,
#if defined(CONFIG_PM)
#ifndef CONFIG_HAS_EARLYSUSPEND
		.pm	= &aat28xx_pm_ops,
#endif
#endif		
	},
};

static int __init aat28xx_init(void)
{
	printk("AAT28XX init start\n");
	return i2c_add_driver(&aat28xx_driver);
}

static void __exit aat28xx_exit(void)
{
	i2c_del_driver(&aat28xx_driver);
}

module_init(aat28xx_init);
module_exit(aat28xx_exit);

MODULE_DESCRIPTION("Backlight driver for ANALOGIC TECH AAT28XX");
MODULE_AUTHOR("---------------------------");
MODULE_LICENSE("GPL");
#else


#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/backlight.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <mach/board_lge.h>

#define MODULE_NAME  "aat2870bl"
#define CONFIG_BACKLIGHT_LEDS_CLASS

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
#include <linux/leds.h>
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif













#ifdef CONFIG_MACH_MSM7X27_THUNDERA 
#define LCD_LED_MAX 14 
#define LCD_LED_MIN 4  
#else
#define LCD_LED_MAX 21 
#define LCD_LED_MIN 0  
#endif
#define DEFAULT_BRIGHTNESS 13
#define AAT28XX_LDO_NUM 4

#define AAT2862BL_REG_BLM   0x03  
#define AAT2862BL_REG_BLS   0x04  
#define AAT2862BL_REG_FADE	0x07  
#define AAT2862BL_REG_LDOAB 0x00  
#define AAT2862BL_REG_LDOCD 0x01  
#define AAT2862BL_REG_LDOEN 0x02  

#define AAT2870BL_REG_BLM   0x01  
#define AAT2870BL_REG_LDOAB 0x24  
#define AAT2870BL_REG_LDOCD 0x25  
#define AAT2870BL_REG_LDOEN 0x26  

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
#define LEDS_BACKLIGHT_NAME "lcd-backlight"
#endif

enum {
	ALC_MODE,
	NORMAL_MODE,
} AAT2870BL_MODE;

enum {
	UNINIT_STATE=-1,
	POWERON_STATE,
	NORMAL_STATE,
	SLEEP_STATE,
	POWEROFF_STATE
} AAT2870BL_STATE;

#define dprintk(fmt, args...) \
	do { \
		if (debug) \
			printk(KERN_INFO "%s:%s: " fmt, MODULE_NAME, __func__, ## args); \
	} while(0);

#define eprintk(fmt, args...)   printk(KERN_ERR "%s:%s: " fmt, MODULE_NAME, __func__, ## args)

struct ldo_vout_struct {
	unsigned char reg;
	unsigned vol;
};

struct aat28xx_ctrl_tbl {
	unsigned char reg;
	unsigned char val;
};

struct aat28xx_reg_addrs {
	unsigned char bl_m;
	unsigned char bl_s;
	unsigned char fade;
	unsigned char ldo_ab;
	unsigned char ldo_cd;
	unsigned char ldo_en;
};

struct aat28xx_cmds {
	struct aat28xx_ctrl_tbl *normal;
	struct aat28xx_ctrl_tbl *alc;
	struct aat28xx_ctrl_tbl *sleep;
};

struct aat28xx_driver_data {
	struct i2c_client *client;
	struct backlight_device *bd;
	struct led_classdev *led;
	int gpio;
	int intensity;
	int max_intensity;
	int mode;
	int state;
	int ldo_ref[AAT28XX_LDO_NUM];
	unsigned char reg_ldo_enable;
	unsigned char reg_ldo_vout[2];
	int version;
	struct aat28xx_cmds cmds;
	struct aat28xx_reg_addrs reg_addrs;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};


static unsigned int debug = 0;
module_param(debug, uint, 0644);


static struct aat28xx_ctrl_tbl aat2862bl_normal_tbl[] = {
#ifdef CONFIG_MACH_MSM7X27_THUNDERA 
	
	 { 0x03, 0xE9 },  
#else
	
	{ 0x03, 0xD2 },  
#endif
	{ 0xFF, 0xFE }	 
};


static struct aat28xx_ctrl_tbl aat2862bl_alc_tbl[] = {
	
	{ 0xFF, 0xFE }   
};


static struct aat28xx_ctrl_tbl aat2862bl_sleep_tbl[] = {
	{ 0x03, 0xDF }, 
	{ 0xFF, 0xFE },  	
};


static struct aat28xx_ctrl_tbl aat2870bl_normal_tbl[] = {
	{ 0x00, 0xFF },  
	{ 0x0E, 0x26 },  
	{ 0x0F, 0x06 },  
	{ 0xFF, 0xFE }	 
};


static struct aat28xx_ctrl_tbl aat2870bl_alc_tbl[] = {
	{ 0x00, 0xFF },  	
	{ 0x0E, 0x27 },  
	{ 0x0F, 0x07 },  
	{ 0x10, 0x00 },  
	{ 0xFF, 0xFE }   
};


static struct aat28xx_ctrl_tbl aat2870bl_sleep_tbl[] = {

	{ 0x0E, 0x26 },  
	{ 0x0F, 0x06 },  
	{ 0x00, 0x00 },  
	{ 0xFF, 0xFE },  	
};

static struct ldo_vout_struct ldo_vout_table[] = {
	{ 0x00, 1200},
	{ 0x01, 1300},
	{ 0x02, 1500},
	{ 0x03, 1600},
	{ 0x04, 1800},
	{ 0x05, 2000},
	{ 0x06, 2200},
	{ 0x07, 2500},
	{ 0x08, 2600},
	{ 0x09, 2700},
	{ 0x0A, 2800},
	{ 0x0B, 2900},
	{ 0x0C, 3000},
	{ 0x0D, 3100},
	{ 0x0E, 3200},
	{ 0x0F, 3300},
	{ 0xFF, 0},
};


static int aat28xx_setup_version(struct aat28xx_driver_data *drvdata)
{
	if(!drvdata)
		return -ENODEV;

	if(drvdata->version == 2862) {
		drvdata->cmds.normal = aat2862bl_normal_tbl;
		drvdata->cmds.alc = aat2862bl_alc_tbl;
		drvdata->cmds.sleep = aat2862bl_sleep_tbl;
		drvdata->reg_addrs.bl_m = AAT2862BL_REG_BLM;
		drvdata->reg_addrs.bl_s = AAT2862BL_REG_BLS;
		drvdata->reg_addrs.fade = AAT2862BL_REG_FADE;
		drvdata->reg_addrs.ldo_ab = AAT2862BL_REG_LDOAB;
		drvdata->reg_addrs.ldo_cd = AAT2862BL_REG_LDOCD;
		drvdata->reg_addrs.ldo_en = AAT2862BL_REG_LDOEN;
	}
	else if(drvdata->version == 2870) {
		drvdata->cmds.normal = aat2870bl_normal_tbl;
		drvdata->cmds.alc = aat2870bl_alc_tbl;
		drvdata->cmds.sleep = aat2870bl_sleep_tbl;
		drvdata->reg_addrs.bl_m = AAT2870BL_REG_BLM;
		drvdata->reg_addrs.ldo_ab = AAT2870BL_REG_LDOAB;
		drvdata->reg_addrs.ldo_cd = AAT2870BL_REG_LDOCD;
		drvdata->reg_addrs.ldo_en = AAT2870BL_REG_LDOEN;
	}
	else {
		eprintk("Not supported version!!\n");
		return -ENODEV;
	}

	return 0;
}

static int aat28xx_read(struct i2c_client *client, u8 reg, u8 *pval)
{
	int ret;
	int status = 0;

	if (client == NULL) { 	
		eprintk("client is null\n");
		return -1;
	}

	ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0) {
		status = -EIO;
		eprintk("fail to read(reg=0x%x,val=0x%x)\n", reg,*pval);	
	}

	*pval = ret;
	return status;
}

static int aat28xx_write(struct i2c_client *client, u8 reg, u8 val)
{
	int ret;
	int status = 0;

	if (client == NULL) {	
		eprintk("client is null\n");
		return -1;
	}

	ret = i2c_smbus_write_byte_data(client, reg, val);
	if (ret != 0) {
		status = -EIO;
		eprintk("fail to write(reg=0x%x,val=0x%x)\n", reg, val);
	}

	return status;
}

static int aat28xx_set_ldos(struct i2c_client *i2c_dev, unsigned num, int enable)
{
	struct aat28xx_driver_data *drvdata = i2c_get_clientdata(i2c_dev);

	if (drvdata) {
		if (enable) drvdata->reg_ldo_enable |= 1 << (num-1);
		else drvdata->reg_ldo_enable &= ~(1 << (num-1));
		
		dprintk("enable ldos, reg:0x13 value:0x%x\n", drvdata->reg_ldo_enable);
		
		return aat28xx_write(i2c_dev, drvdata->reg_addrs.ldo_en, drvdata->reg_ldo_enable);
	}
	return -EIO;
}

static unsigned char aat28xx_ldo_get_vout_val(unsigned vol)
{
	int i = 0;
	do {
		if (ldo_vout_table[i].vol == vol)
			return ldo_vout_table[i].reg;
		else
			i++;
	} while (ldo_vout_table[i].vol != 0);

	return ldo_vout_table[i].reg;
}

static int aat28xx_ldo_set_vout(struct i2c_client *i2c_dev, unsigned num, unsigned char val)
{
	struct aat28xx_driver_data *drvdata = i2c_get_clientdata(i2c_dev);
	unsigned char *next_val;
	unsigned char reg;

	if (drvdata) {
		if (num <= 2) {
			reg = drvdata->reg_addrs.ldo_ab;
			next_val = &drvdata->reg_ldo_vout[0];
		} else {
			reg = drvdata->reg_addrs.ldo_cd;
			next_val = &drvdata->reg_ldo_vout[1];
		}
		if (num % 2) {
			*next_val &= 0x0F;
			val = val << 4;		
		}
		else {
			*next_val &= 0xF0;		
		}
		*next_val |= val;
		dprintk("target register[0x%x], value[0x%x]\n",	reg, *next_val);
		return aat28xx_write(i2c_dev, reg, *next_val);
	}
	return -EIO;
}


int aat28xx_ldo_enable(struct device *dev, unsigned num, unsigned enable)
{
	struct i2c_adapter *adap;
	struct i2c_client *client;
	struct aat28xx_driver_data *drvdata;
	int err = 0;

	dprintk("ldo_no[%d], on/off[%d]\n",num, enable);

	if (num > 0 && num <= AAT28XX_LDO_NUM) {
		if ((adap=dev_get_drvdata(dev)) && (client=i2c_get_adapdata(adap))) {
			drvdata = i2c_get_clientdata(client);
			if (enable) {
				if (drvdata->ldo_ref[num-1]++ == 0) {
					dprintk("ref count = 0, call aat28xx_set_ldos\n");
					err = aat28xx_set_ldos(client, num, enable);
				}
			}
			else {
				if (--drvdata->ldo_ref[num-1] == 0) {
					dprintk("ref count = 0, call aat28xx_set_ldos\n");
					err = aat28xx_set_ldos(client, num, enable);
				}
			}
			return err;
		}
	}
	return -ENODEV;
}
EXPORT_SYMBOL(aat28xx_ldo_enable);


int aat28xx_ldo_set_level(struct device *dev, unsigned num, unsigned vol)
{
	struct i2c_adapter *adap;
	struct i2c_client *client;
	unsigned char val;

	dprintk("ldo_no[%d], level[%d]\n", num, vol);
	if (num > 0 && num <= AAT28XX_LDO_NUM) {
		if ((adap=dev_get_drvdata(dev)) && (client=i2c_get_adapdata(adap))) {
			val = aat28xx_ldo_get_vout_val(vol);
			dprintk("vout register value 0x%x for level %d\n", val, vol);
			return aat28xx_ldo_set_vout(client, num, val);
		}
	}
	return -ENODEV;
}
EXPORT_SYMBOL(aat28xx_ldo_set_level);

static int aat28xx_set_table(struct aat28xx_driver_data *drvdata, struct aat28xx_ctrl_tbl *ptbl)
{
	unsigned int i = 0;
	unsigned long delay = 0;

	if (ptbl == NULL) {
		eprintk("input ptr is null\n");
		return -EIO;
	}

	for( ;;) {
		if (ptbl->reg == 0xFF) {
			if (ptbl->val != 0xFE) {
				delay = (unsigned long)ptbl->val;
				udelay(delay);
			}
			else
				break;
		}	
		else {
			if (aat28xx_write(drvdata->client, ptbl->reg, ptbl->val) != 0)
				dprintk("i2c failed addr:%d, value:%d\n", ptbl->reg, ptbl->val);
		}
		ptbl++;
		i++;
	}
	return 0;
}

static void aat28xx_hw_reset(struct aat28xx_driver_data *drvdata)
{
	if (drvdata->client && gpio_is_valid(drvdata->gpio)) {
		gpio_configure(drvdata->gpio, GPIOF_DRIVE_OUTPUT);
		
		gpio_set_value(drvdata->gpio, 0);
		udelay(5);
		gpio_set_value(drvdata->gpio, 1);
		udelay(5);
	}
}

static void aat28xx_go_opmode(struct aat28xx_driver_data *drvdata)
{
	dprintk("operation mode is %s\n", (drvdata->mode == NORMAL_MODE) ? "normal_mode" : "alc_mode");
	
	switch (drvdata->mode) {
		case NORMAL_MODE:
			aat28xx_set_table(drvdata, drvdata->cmds.normal);
			drvdata->state = NORMAL_STATE;
			break;
		case ALC_MODE:
			
			
			
			
		default:
			eprintk("Invalid Mode\n");
			break;
	}
}

static void aat28xx_device_init(struct aat28xx_driver_data *drvdata)
{

	if (system_state == SYSTEM_BOOTING) {
		aat28xx_go_opmode(drvdata);
		return;
	}
	aat28xx_hw_reset(drvdata);
	aat28xx_go_opmode(drvdata);
}

static void aat28xx_poweron(struct aat28xx_driver_data *drvdata)
{
	unsigned int aat28xx_intensity;
	if (!drvdata || drvdata->state != POWEROFF_STATE)
		return;
	
	dprintk("POWER ON \n");

	aat28xx_device_init(drvdata);
	
	if (drvdata->mode == NORMAL_MODE)
	{
		if(drvdata->version == 2862)
		{
			aat28xx_intensity = (~(drvdata->intensity)& 0x1F);	
			aat28xx_intensity |= 0xE0;				
			aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, aat28xx_intensity);
		}
		else
			aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, drvdata->intensity);
	}
}

static void aat28xx_poweroff(struct aat28xx_driver_data *drvdata)
{
	if (!drvdata || drvdata->state == POWEROFF_STATE)
		return;

	dprintk("POWER OFF \n");

	if (drvdata->state == SLEEP_STATE) {
		gpio_direction_output(drvdata->gpio, 0);
		msleep(6);
		drvdata->state = POWEROFF_STATE;
		return;
	}

	gpio_tlmm_config(GPIO_CFG(drvdata->gpio, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	gpio_direction_output(drvdata->gpio, 0);
	mdelay(6);
	drvdata->state = POWEROFF_STATE;
}


static void aat28xx_sleep(struct aat28xx_driver_data *drvdata)
{
	if (!drvdata || drvdata->state == SLEEP_STATE)
		return;

	dprintk("operation mode is %s\n", (drvdata->mode == NORMAL_MODE) ? "normal_mode" : "alc_mode");
	
	switch (drvdata->mode) {
		case NORMAL_MODE:
			drvdata->state = SLEEP_STATE;
			aat28xx_set_table(drvdata, drvdata->cmds.sleep);
			break;

		case ALC_MODE:
			
			
			
			
			

		default:
			eprintk("Invalid Mode\n");
			break;
	}
}

static void aat28xx_wakeup(struct aat28xx_driver_data *drvdata)
{
	unsigned int aat28xx_intensity;

	if (!drvdata || drvdata->state == NORMAL_STATE)
		return;

	dprintk("operation mode is %s\n", (drvdata->mode == NORMAL_MODE) ? "normal_mode" : "alc_mode");

	if (drvdata->state == POWEROFF_STATE) {
		aat28xx_poweron(drvdata);
		
		
	} else if (drvdata->state == SLEEP_STATE) {
		if (drvdata->mode == NORMAL_MODE) {
			if(drvdata->version == 2862) {
				
				aat28xx_write(drvdata->client, drvdata->reg_addrs.fade, 0x00);	
				aat28xx_intensity = (~(drvdata->intensity)& 0x1F);	
				aat28xx_intensity |= 0xA0;							
				aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, aat28xx_intensity);
				aat28xx_write(drvdata->client, drvdata->reg_addrs.fade, 0x08);	
			} else {
				aat28xx_set_table(drvdata, drvdata->cmds.normal);
				aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, drvdata->intensity);
			}
			drvdata->state = NORMAL_STATE;
		} else if (drvdata->mode == ALC_MODE) {
			
			
			
		}
	}
}

static int aat28xx_send_intensity(struct aat28xx_driver_data *drvdata, int next)
{
	int aat2862_bl_next;

	if (drvdata->mode == NORMAL_MODE) {
		if (next > drvdata->max_intensity)
			next = drvdata->max_intensity;
		if (next < LCD_LED_MIN)
			next = LCD_LED_MIN;
		dprintk("next current is %d\n", next);

		if (drvdata->state == NORMAL_STATE && drvdata->intensity != next)
		{
			
			if(drvdata->version == 2862)
			{
				if(next != 0)
				{
					aat2862_bl_next = (~next & 0x1F);	
					aat2862_bl_next |= 0xE0;		
					aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, aat2862_bl_next);
				}
				else
				{	
					aat2862_bl_next = 0xDF;		
					aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, aat2862_bl_next);					
				}
			}
			else	
				aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, next);
		}
		
		drvdata->intensity = next;
	}
	else {
		dprintk("A manual setting for intensity is only permitted in normal mode\n");
	}

	return 0;
}

static int aat28xx_get_intensity(struct aat28xx_driver_data *drvdata)
{
	return drvdata->intensity;
}


#ifdef CONFIG_PM
#ifdef CONFIG_HAS_EARLYSUSPEND
static void aat28xx_early_suspend(struct early_suspend * h)
{	
	struct aat28xx_driver_data *drvdata = container_of(h, struct aat28xx_driver_data,
						    early_suspend);

	dprintk("start\n");
	aat28xx_sleep(drvdata);

	return;
}

static void aat28xx_late_resume(struct early_suspend * h)
{	
	struct aat28xx_driver_data *drvdata = container_of(h, struct aat28xx_driver_data,
						    early_suspend);

	dprintk("start\n");
	aat28xx_wakeup(drvdata);

	return;
}
#else
static int aat28xx_suspend(struct i2c_client *i2c_dev, pm_message_t state)
{
	struct aat28xx_driver_data *drvdata = i2c_get_clientdata(i2c_dev);
	aat28xx_sleep(drvdata);
	return 0;
}

static int aat28xx_resume(struct i2c_client *i2c_dev)
{
	struct aat28xx_driver_data *drvdata = i2c_get_clientdata(i2c_dev);
	aat28xx_wakeup(drvdata);
	return 0;
}
#endif	
#else
#define aat28xx_suspend	NULL
#define aat28xx_resume	NULL
#endif	

void aat28xx_switch_mode(struct device *dev, int next_mode)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev);
	unsigned int aat28xx_intensity;

	if (!drvdata || drvdata->mode == next_mode)
		return;

	if (next_mode == ALC_MODE) {
		
		
	}
	else if (next_mode == NORMAL_MODE) {
		aat28xx_set_table(drvdata, drvdata->cmds.alc);

		if(drvdata->version == 2862) {
			aat28xx_intensity = (~(drvdata->intensity)& 0x1F);	
			aat28xx_intensity |= 0xE0;				
			aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, aat28xx_intensity);
		} else {
			aat28xx_write(drvdata->client, drvdata->reg_addrs.bl_m, drvdata->intensity);
		}
	} else {
		printk(KERN_ERR "%s: invalid mode(%d)!!!\n", __func__, next_mode);
		return;
	}

	drvdata->mode = next_mode;
	return;
}

ssize_t aat28xx_show_alc(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev->parent);
	int r;

	if (!drvdata) return 0;

	r = snprintf(buf, PAGE_SIZE, "%s\n", (drvdata->mode == ALC_MODE) ? "1":"0");
	
	return r;
}

ssize_t aat28xx_store_alc(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int alc;
	int next_mode;

	if (!count)
		return -EINVAL;

	sscanf(buf, "%d", &alc);

	if (alc)
		next_mode = ALC_MODE;
	else
		next_mode = NORMAL_MODE;

	aat28xx_switch_mode(dev->parent, next_mode);

	return count;
}

ssize_t aat28xx_show_reg(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev);
	int len = 0;
	unsigned char val;

	len += snprintf(buf,       PAGE_SIZE,       "\nAAT2870 Registers is following..\n");
	aat28xx_read(drvdata->client, 0x00, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[CH_EN(0x00)] = 0x%x\n", val);
	aat28xx_read(drvdata->client, 0x01, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[BLM(0x01)] = 0x%x\n", val);
	aat28xx_read(drvdata->client, 0x0E, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[ALS(0x0E)] = 0x%x\n", val);	
	aat28xx_read(drvdata->client, 0x0F, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[SBIAS(0x0F)] = 0x%x\n", val);
	aat28xx_read(drvdata->client, 0x10, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[ALS_GAIN(0x10)] = 0x%x\n", val);
	aat28xx_read(drvdata->client, 0x11, &val);
	len += snprintf(buf + len, PAGE_SIZE - len, "[AMBIENT_LEVEL(0x11)] = 0x%x\n", val);

	return len;
}

ssize_t aat28xx_show_drvstat(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(dev->parent);
	int len = 0;

	len += snprintf(buf,       PAGE_SIZE,       "\nAAT2870 Backlight Driver Status is following..\n");
	len += snprintf(buf + len, PAGE_SIZE - len, "mode                   = %3d\n", drvdata->mode);
	len += snprintf(buf + len, PAGE_SIZE - len, "state                  = %3d\n", drvdata->state);
	len += snprintf(buf + len, PAGE_SIZE - len, "current intensity      = %3d\n", drvdata->intensity);

	return len;
}

DEVICE_ATTR(alc, 0664, aat28xx_show_alc, aat28xx_store_alc);
DEVICE_ATTR(reg, 0444, aat28xx_show_reg, NULL);
DEVICE_ATTR(drvstat, 0444, aat28xx_show_drvstat, NULL);

static int aat28xx_set_brightness(struct backlight_device *bd)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(bd->dev.parent);
	return aat28xx_send_intensity(drvdata, bd->props.brightness);
}

static int aat28xx_get_brightness(struct backlight_device *bd)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(bd->dev.parent);
	return aat28xx_get_intensity(drvdata);
}

static struct backlight_ops aat28xx_ops = {
	.get_brightness = aat28xx_get_brightness,
	.update_status  = aat28xx_set_brightness,
};


#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
static void leds_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct aat28xx_driver_data *drvdata = dev_get_drvdata(led_cdev->dev->parent);
	int brightness;
	int next;

	if (!drvdata) {
		eprintk("Error getting drvier data\n");
		return;
	}

	brightness = aat28xx_get_intensity(drvdata);

	next = value * drvdata->max_intensity / LED_FULL;
	dprintk("input brightness value=%d]\n", next);

	if (brightness != next) {
		dprintk("brightness[current=%d, next=%d]\n", brightness, next);
		aat28xx_send_intensity(drvdata, next);
	}
}

static struct led_classdev aat28xx_led_dev = {
	.name = LEDS_BACKLIGHT_NAME,
	.brightness_set = leds_brightness_set,
};
#endif

static int __init aat28xx_probe(struct i2c_client *i2c_dev, const struct i2c_device_id *i2c_dev_id)
{
	struct aat28xx_platform_data *pdata;
	struct aat28xx_driver_data *drvdata;
	struct backlight_device *bd;
	int err;

	dprintk("start, client addr=0x%x\n", i2c_dev->addr);

	pdata = i2c_dev->dev.platform_data;
	if(!pdata)
		return -EINVAL;
		
	drvdata = kzalloc(sizeof(struct aat28xx_driver_data), GFP_KERNEL);
	if (!drvdata) {
		dev_err(&i2c_dev->dev, "failed to allocate memory\n");
		return -ENOMEM;
	}

	if (pdata && pdata->platform_init)
		pdata->platform_init();

	drvdata->client = i2c_dev;
	drvdata->gpio = pdata->gpio;
	drvdata->max_intensity = LCD_LED_MAX;
	if (pdata->max_current > 0)
		drvdata->max_intensity = pdata->max_current;
	drvdata->intensity = LCD_LED_MIN;
	drvdata->mode = NORMAL_MODE;
	drvdata->state = UNINIT_STATE;
	drvdata->version = pdata->version;

	if(aat28xx_setup_version(drvdata) != 0) {
		eprintk("Error while requesting gpio %d\n", drvdata->gpio);
		kfree(drvdata);
		return -ENODEV;
	}		
	if (drvdata->gpio && gpio_request(drvdata->gpio, "aat28xx_en") != 0) {
		eprintk("Error while requesting gpio %d\n", drvdata->gpio);
		kfree(drvdata);
		return -ENODEV;
	}

	bd = backlight_device_register("aat28xx-bl", &i2c_dev->dev, NULL, &aat28xx_ops);
	if (bd == NULL) {
		eprintk("entering aat28xx probe function error \n");
		if (gpio_is_valid(drvdata->gpio))
			gpio_free(drvdata->gpio);
		kfree(drvdata);
		return -1;
	}
	bd->props.power = FB_BLANK_UNBLANK;
	bd->props.brightness = drvdata->intensity;
	bd->props.max_brightness = drvdata->max_intensity;
	drvdata->bd = bd;

#ifdef CONFIG_BACKLIGHT_LEDS_CLASS
	if (led_classdev_register(&i2c_dev->dev, &aat28xx_led_dev) == 0) {
		eprintk("Registering led class dev successfully.\n");
		drvdata->led = &aat28xx_led_dev;
		err = device_create_file(drvdata->led->dev, &dev_attr_alc);
		err = device_create_file(drvdata->led->dev, &dev_attr_reg);
		err = device_create_file(drvdata->led->dev, &dev_attr_drvstat);
	}
#endif

	i2c_set_clientdata(i2c_dev, drvdata);
	i2c_set_adapdata(i2c_dev->adapter, i2c_dev);

	aat28xx_device_init(drvdata);
	aat28xx_send_intensity(drvdata, DEFAULT_BRIGHTNESS);

#ifdef CONFIG_HAS_EARLYSUSPEND
	drvdata->early_suspend.suspend = aat28xx_early_suspend;
	drvdata->early_suspend.resume = aat28xx_late_resume;
	drvdata->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 40;
	register_early_suspend(&drvdata->early_suspend);
#endif

	eprintk("done\n");
	return 0;
}

static int __devexit aat28xx_remove(struct i2c_client *i2c_dev)
{
	struct aat28xx_driver_data *drvdata = i2c_get_clientdata(i2c_dev);

	aat28xx_send_intensity(drvdata, 0);

	backlight_device_unregister(drvdata->bd);
	led_classdev_unregister(drvdata->led);
	i2c_set_clientdata(i2c_dev, NULL);
	if (gpio_is_valid(drvdata->gpio))
		gpio_free(drvdata->gpio);
	kfree(drvdata);

	return 0;
}

static struct i2c_device_id aat28xx_idtable[] = {
	{ MODULE_NAME, 0 },
};

MODULE_DEVICE_TABLE(i2c, aat28xx_idtable);

static struct i2c_driver aat28xx_driver = {
	.probe 		= aat28xx_probe,
	.remove 	= aat28xx_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend 	= aat28xx_suspend,
	.resume 	= aat28xx_resume,
#endif
	.id_table 	= aat28xx_idtable,
	.driver = {
		.name = MODULE_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init aat28xx_init(void)
{
	printk("AAT28XX init start\n");
	return i2c_add_driver(&aat28xx_driver);
}

static void __exit aat28xx_exit(void)
{
	i2c_del_driver(&aat28xx_driver);
}

module_init(aat28xx_init);
module_exit(aat28xx_exit);

MODULE_DESCRIPTION("Backlight driver for ANALOGIC TECH AAT28XX");
MODULE_AUTHOR("---------------------------------");
MODULE_LICENSE("GPL");
#endif
