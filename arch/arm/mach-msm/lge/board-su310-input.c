
#include <linux/types.h>
#include <linux/list.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/gpio_event.h>
#include <linux/keyreset.h>
#include <mach/gpio.h>
#include <mach/vreg.h>
#include <mach/board.h>
#include <mach/board_lge.h>
#include <mach/rpc_server_handset.h>

#include "board-su310.h"
static int prox_power_set(unsigned char onoff);


static unsigned short atcmd_virtual_keycode[ATCMD_VIRTUAL_KEYPAD_ROW][ATCMD_VIRTUAL_KEYPAD_COL] = {
	{KEY_1, 		KEY_8, 				KEY_Q,  	 KEY_I,          KEY_D,      	KEY_HOME,	KEY_B,          KEY_UP},
	{KEY_2, 		KEY_9, 		  		KEY_W,		 KEY_O,       	 KEY_F,		 	KEY_RIGHTSHIFT, 	KEY_N,			KEY_DOWN},
	{KEY_3, 		KEY_0, 		  		KEY_E,		 KEY_P,          KEY_G,      	KEY_Z,        	KEY_M, 			KEY_UNKNOWN},
	{KEY_4, 		KEY_BACK,  			KEY_R,		 KEY_SEARCH,     KEY_H,			KEY_X,    		KEY_LEFTSHIFT,	KEY_UNKNOWN},
	{KEY_5, 		KEY_BACKSPACE, 		KEY_T,		 KEY_LEFTALT,    KEY_J,      	KEY_C,     		KEY_REPLY,    KEY_CAMERA},
	{KEY_6, 		KEY_ENTER,  		KEY_Y,  	 KEY_A,		     KEY_K,			KEY_V,  	    KEY_RIGHT,     	KEY_CAMERAFOCUS},
	{KEY_7, 		KEY_MENU,	KEY_U,  	 KEY_S,    		 KEY_L, 	    KEY_SPACE,      KEY_LEFT,     	KEY_SEND},
	{KEY_UNKNOWN, 	KEY_UNKNOWN,  		KEY_UNKNOWN, KEY_UNKNOWN, 	 KEY_UNKNOWN,	KEY_UNKNOWN,    KEY_FOLDER_MENU,      	KEY_FOLDER_HOME},

};

static struct atcmd_virtual_platform_data atcmd_virtual_pdata = {
	.keypad_row = ATCMD_VIRTUAL_KEYPAD_ROW,
	.keypad_col = ATCMD_VIRTUAL_KEYPAD_COL,
	.keycode = (unsigned char *)atcmd_virtual_keycode,
};

static struct platform_device atcmd_virtual_device = {
	.name = "atcmd_virtual_kbd",
	.id = -1,
	.dev = {
		.platform_data = &atcmd_virtual_pdata,
	},
};



static struct msm_handset_platform_data hs_platform_data = {
	.hs_name = "7k_handset",
	.pwr_key_delay_ms = 500, 
};

static struct platform_device hs_device = {
	.name   = "msm-handset",
	.id     = -1,
	.dev    = {
		.platform_data = &hs_platform_data,
	},
};


#define GPIO_VOL_UP		37
#define GPIO_VOL_DOWN	38

static struct gpio_event_direct_entry su310_keypad_switch_map[] = {
	{ GPIO_VOL_UP,       	KEY_VOLUMEUP   	    },
	{ GPIO_VOL_DOWN,       	KEY_VOLUMEDOWN	    },
};


static int su310_gpio_event_input_func(struct input_dev *input_dev,
			struct gpio_event_info *info, void **data, int func)
{
	int ret, i;


	if (func == GPIO_EVENT_FUNC_INIT) {




		gpio_tlmm_config(GPIO_CFG(GPIO_VOL_UP, 0, GPIO_INPUT, GPIO_PULL_UP,
					GPIO_2MA), GPIO_ENABLE);
		gpio_tlmm_config(GPIO_CFG(GPIO_VOL_DOWN, 0, GPIO_INPUT, GPIO_PULL_UP,
					GPIO_2MA), GPIO_ENABLE);
		
		enable_irq_wake(MSM_GPIO_TO_INT(GPIO_VOL_UP));
		enable_irq_wake(MSM_GPIO_TO_INT(GPIO_VOL_DOWN));
		
		ret = gpio_event_input_func(input_dev, info, data, func);
	
		return ret;
	}
}

static int su310_gpio_keypad_power(
                const struct gpio_event_platform_data *pdata, bool on)
{
	

	return 0;
}

static struct gpio_event_input_info su310_keypad_switch_info = {
	.info.func = su310_gpio_event_input_func,
	.flags = 0,
	.type = EV_KEY,
	.keymap = su310_keypad_switch_map,
	.keymap_size = ARRAY_SIZE(su310_keypad_switch_map)
};

static struct gpio_event_info *su310_keypad_info[] = {
	&su310_keypad_switch_info.info,
};

static struct gpio_event_platform_data su310_keypad_data = {
	.name		= "su310_keypad",
	.info		= su310_keypad_info,
	.info_count	= ARRAY_SIZE(su310_keypad_info),
	.power = su310_gpio_keypad_power,
};

struct platform_device keypad_device_su310= {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &su310_keypad_data,
	},
};


static int su310_reset_keys_up[] = {
	KEY_HOME,
	0
};

static struct keyreset_platform_data su310_reset_keys_pdata = {
	.keys_up = su310_reset_keys_up,
	.keys_down = {
		
		KEY_SEARCH,
		KEY_VOLUMEDOWN,
		KEY_MENU,
		0
	},
};

struct platform_device su310_reset_keys_device = {
	.name = KEYRESET_NAME,
	.dev.platform_data = &su310_reset_keys_pdata,
};



#ifdef CONFIG_MSM_RPCSERVER_TESTMODE
struct testmode_input_platform_data {
	const char *name;
};

static struct testmode_input_platform_data testmode_input_pdata = {
	.name = "testmode_input",
};

static struct platform_device testmode_input_device = {
	.name = "testmode_input",
	.id = -1,
	.dev    = {
		.platform_data = &testmode_input_pdata
	},
};
#endif



static struct platform_device *su310_input_devices[] __initdata = {
	&hs_device,
	&keypad_device_su310,
	&su310_reset_keys_device,
	&atcmd_virtual_device,

#ifdef CONFIG_MSM_RPCSERVER_TESTMODE
	&testmode_input_device,
#endif

};


static struct gpio_i2c_pin ts_i2c_pin[] = {
	[0] = {
		.sda_pin	= TS_GPIO_I2C_SDA,
		.scl_pin	= TS_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= TS_GPIO_IRQ,
	},
};

static struct i2c_gpio_platform_data ts_i2c_pdata = {
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.udelay				= 2,
};

static struct platform_device ts_i2c_device = {
	.name	= "i2c-gpio",
	.dev.platform_data = &ts_i2c_pdata,
};

static int ts_set_vreg(unsigned char onoff)
{
	struct vreg *vreg_touch;
	int rc;

	printk("[Touch] %s() onoff:%d\n",__FUNCTION__, onoff);

	vreg_touch = vreg_get(0, "gp3");

	if(IS_ERR(vreg_touch)) {
		printk("[Touch] vreg_get fail : touch\n");
		return -1;
	}

	if (onoff) {
		rc = vreg_set_level(vreg_touch, 3050);
		if (rc != 0) {
			printk("[Touch] vreg_set_level failed\n");
			return -1;
		}
		vreg_enable(vreg_touch);
	} else
		vreg_disable(vreg_touch);

	return 0;
}

static struct touch_platform_data ts_pdata = {
	.ts_x_min = TS_X_MIN,
	.ts_x_max = TS_X_MAX,
	.ts_y_min = TS_Y_MIN,
	.ts_y_max = TS_Y_MAX,
	.power 	  = ts_set_vreg,
	.irq 	  = TS_GPIO_IRQ,
	.scl      = TS_GPIO_I2C_SCL,
	.sda      = TS_GPIO_I2C_SDA,
};

static struct i2c_board_info ts_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("touch_mcs6000", TS_I2C_SLAVE_ADDR),
		.type = "touch_mcs6000",
		.platform_data = &ts_pdata,
	},
};

static void __init su310_init_i2c_touch(int bus_num)
{
	ts_i2c_device.id = bus_num;

	init_gpio_i2c_pin(&ts_i2c_pdata, ts_i2c_pin[0],	&ts_i2c_bdinfo[0]);
	i2c_register_board_info(bus_num, &ts_i2c_bdinfo[0], 1);
	platform_device_register(&ts_i2c_device);
}


static int kr3dh_config_gpio(int config)
{
	if (config) {	
		gpio_tlmm_config(GPIO_CFG(ACCEL_GPIO_I2C_SCL, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
		gpio_tlmm_config(GPIO_CFG(ACCEL_GPIO_I2C_SDA, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	} else {		
		gpio_tlmm_config(GPIO_CFG(ACCEL_GPIO_I2C_SCL, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
		gpio_tlmm_config(GPIO_CFG(ACCEL_GPIO_I2C_SDA, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	}

	return 0;
}

static int kr_init(void)
{
	return 0;
}

static void kr_exit(void)
{
}

static int power_on(void)
{
	return 0;
}

static int power_off(void)
{
	return 0;
}

struct kr3dh_platform_data kr3dh_data = {
	.poll_interval = 100,
	.min_interval = 0,
	.g_range = 0x00,
	.axis_map_x = 0,
	.axis_map_y = 1,
	.axis_map_z = 2,

	.negate_x = 0,
	.negate_y = 0,
	.negate_z = 0,

	.power_on = power_on,
	.power_off = power_off,
	.kr_init = kr_init,
	.kr_exit = kr_exit,
	.gpio_config = kr3dh_config_gpio,
};

static struct gpio_i2c_pin accel_i2c_pin[] = {
	[0] = {
		.sda_pin	= ACCEL_GPIO_I2C_SDA,
		.scl_pin	= ACCEL_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= ACCEL_GPIO_INT,
	},
};

static struct i2c_gpio_platform_data accel_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device accel_i2c_device = {
	.name = "i2c-gpio",
	.dev.platform_data = &accel_i2c_pdata,
};

static struct i2c_board_info accel_i2c_bdinfo[] = {
	[1] = {
		I2C_BOARD_INFO("KR3DH", ACCEL_I2C_ADDRESS_H),
		.type = "KR3DH",
		.platform_data = &kr3dh_data,
	},
	[0] = {
		I2C_BOARD_INFO("KR3DM", ACCEL_I2C_ADDRESS),
		.type = "KR3DM",
		.platform_data = &kr3dh_data,
	},
};

static void __init su310_init_i2c_acceleration(int bus_num)
{
	accel_i2c_device.id = bus_num;


	if(lge_bd_rev <= LGE_REV_B)
	{
		kr3dh_data.axis_map_x = 1;
		kr3dh_data.axis_map_y = 0;
		kr3dh_data.axis_map_z = 2;

		kr3dh_data.negate_x = 1;
		kr3dh_data.negate_y = 0;
		kr3dh_data.negate_z = 0;
	}


	init_gpio_i2c_pin(&accel_i2c_pdata, accel_i2c_pin[0], &accel_i2c_bdinfo[0]);

	
	if (lge_bd_rev < LGE_REV_D)
		i2c_register_board_info(bus_num, &accel_i2c_bdinfo[0], 1);
	else
		i2c_register_board_info(bus_num, &accel_i2c_bdinfo[1], 1);

	platform_device_register(&accel_i2c_device);
}


static int ecom_power_save_on = 0;
static int prox_power_save_on = 0;

static int ecom_power_set(unsigned char onoff)
{
	int ret = 0;
	struct vreg *vreg_power;
	int err;
	int flag_on = !!onoff;

	if (ecom_power_save_on == flag_on)
		return ret;
	printk("[Ecompass] %s() onoff:%d\n",__FUNCTION__, onoff);

	ecom_power_save_on = flag_on;
	prox_power_save_on = flag_on;

	vreg_power = vreg_get(0, "mmc");

	if (onoff) {
		vreg_set_level(vreg_power, 2500);
		vreg_enable(vreg_power);
	} 
	else {
	


	
	}

	return ret;
}

static struct ecom_platform_data ecom_pdata = {
	.pin_int        	= ECOM_GPIO_INT,
	.pin_rst		= 0,
	.power          	= ecom_power_set,
};



static int prox_power_set(unsigned char onoff)
{
	int ret = 0;
	int flag_on = !!onoff;
	struct vreg *vreg = vreg_get(0, "mmc");

	if (prox_power_save_on == flag_on)
		return ret;
	printk("[Proxi] %s() onoff:%d\n",__FUNCTION__, onoff);

	prox_power_save_on = flag_on;	
		ecom_power_save_on = flag_on;

	if (onoff) {
		vreg_set_level(vreg, 2500);
		vreg_enable(vreg);
	} else {
	


	
	}

	return ret;
}

static struct proximity_platform_data proxi_pdata = {
	.irq_num	= PROXI_GPIO_DOUT,
	.power		= prox_power_set,
	.methods		= 0,
	.operation_mode		= 0,
	.debounce	 = 0,
	.cycle = 2,
};

static struct i2c_board_info prox_ecom_i2c_bdinfo[] = {
	[0] = {
		I2C_BOARD_INFO("apds9801", PROXI_I2C_ADDRESS),
		.type = "apds9801",
		.platform_data = &proxi_pdata,
	},
	[1] = {
		I2C_BOARD_INFO("ami304_sensor", ECOM_I2C_ADDRESS),
		.type = "ami304_sensor",
		.platform_data = &ecom_pdata,
	},
};

static struct gpio_i2c_pin proxi_ecom_i2c_pin[] = {
	[0] = {
		.sda_pin	= PROX_COMPASS_GPIO_I2C_SDA,
		.scl_pin	= PROX_COMPASS_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= PROXI_GPIO_DOUT,
	},
	[1] = {
		.sda_pin	= PROX_COMPASS_GPIO_I2C_SDA,
		.scl_pin	= PROX_COMPASS_GPIO_I2C_SCL,
		.reset_pin	= 0,
		.irq_pin	= ECOM_GPIO_INT,
	},
};

static struct i2c_gpio_platform_data proxi_ecom_i2c_pdata = {
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.udelay = 2,
};

static struct platform_device proxi_ecom_i2c_device = {
        .name = "i2c-gpio",
        .dev.platform_data = &proxi_ecom_i2c_pdata,
};


static void __init su310_init_i2c_prox_ecom(int bus_num)
{
	proxi_ecom_i2c_device.id = bus_num;

	init_gpio_i2c_pin(&proxi_ecom_i2c_pdata, proxi_ecom_i2c_pin[0], &prox_ecom_i2c_bdinfo[0]);
	init_gpio_i2c_pin(&proxi_ecom_i2c_pdata, proxi_ecom_i2c_pin[1], &prox_ecom_i2c_bdinfo[1]);

	i2c_register_board_info(bus_num, &prox_ecom_i2c_bdinfo[0], 2);
	platform_device_register(&proxi_ecom_i2c_device);
}


void __init lge_add_input_devices(void)
{
	platform_add_devices(su310_input_devices, ARRAY_SIZE(su310_input_devices));

	lge_add_gpio_i2c_device(su310_init_i2c_touch);
	lge_add_gpio_i2c_device(su310_init_i2c_acceleration);
	lge_add_gpio_i2c_device(su310_init_i2c_prox_ecom);
}

