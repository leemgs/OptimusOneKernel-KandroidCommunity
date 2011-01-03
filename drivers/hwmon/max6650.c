

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>



static const unsigned short normal_i2c[] = {0x1b, 0x1f, 0x48, 0x4b,
						I2C_CLIENT_END};




static int fan_voltage;

static int prescaler;

static int clock = 254000;

module_param(fan_voltage, int, S_IRUGO);
module_param(prescaler, int, S_IRUGO);
module_param(clock, int, S_IRUGO);

I2C_CLIENT_INSMOD_1(max6650);



#define MAX6650_REG_SPEED	0x00
#define MAX6650_REG_CONFIG	0x02
#define MAX6650_REG_GPIO_DEF	0x04
#define MAX6650_REG_DAC		0x06
#define MAX6650_REG_ALARM_EN	0x08
#define MAX6650_REG_ALARM	0x0A
#define MAX6650_REG_TACH0	0x0C
#define MAX6650_REG_TACH1	0x0E
#define MAX6650_REG_TACH2	0x10
#define MAX6650_REG_TACH3	0x12
#define MAX6650_REG_GPIO_STAT	0x14
#define MAX6650_REG_COUNT	0x16



#define MAX6650_CFG_V12			0x08
#define MAX6650_CFG_PRESCALER_MASK	0x07
#define MAX6650_CFG_PRESCALER_2		0x01
#define MAX6650_CFG_PRESCALER_4		0x02
#define MAX6650_CFG_PRESCALER_8		0x03
#define MAX6650_CFG_PRESCALER_16	0x04
#define MAX6650_CFG_MODE_MASK		0x30
#define MAX6650_CFG_MODE_ON		0x00
#define MAX6650_CFG_MODE_OFF		0x10
#define MAX6650_CFG_MODE_CLOSED_LOOP	0x20
#define MAX6650_CFG_MODE_OPEN_LOOP	0x30
#define MAX6650_COUNT_MASK		0x03



#define MAX6650_ALRM_MAX	0x01
#define MAX6650_ALRM_MIN	0x02
#define MAX6650_ALRM_TACH	0x04
#define MAX6650_ALRM_GPIO1	0x08
#define MAX6650_ALRM_GPIO2	0x10


#define FAN_RPM_MIN 240
#define FAN_RPM_MAX 30000

#define DIV_FROM_REG(reg) (1 << (reg & 7))

static int max6650_probe(struct i2c_client *client,
			 const struct i2c_device_id *id);
static int max6650_detect(struct i2c_client *client, int kind,
			  struct i2c_board_info *info);
static int max6650_init_client(struct i2c_client *client);
static int max6650_remove(struct i2c_client *client);
static struct max6650_data *max6650_update_device(struct device *dev);



static const struct i2c_device_id max6650_id[] = {
	{ "max6650", max6650 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, max6650_id);

static struct i2c_driver max6650_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "max6650",
	},
	.probe		= max6650_probe,
	.remove		= max6650_remove,
	.id_table	= max6650_id,
	.detect		= max6650_detect,
	.address_data	= &addr_data,
};



struct max6650_data
{
	struct device *hwmon_dev;
	struct mutex update_lock;
	char valid; 
	unsigned long last_updated; 

	
	u8 speed;
	u8 config;
	u8 tach[4];
	u8 count;
	u8 dac;
	u8 alarm;
};

static ssize_t get_fan(struct device *dev, struct device_attribute *devattr,
		       char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct max6650_data *data = max6650_update_device(dev);
	int rpm;

	

	rpm = ((data->tach[attr->index] * 120) / DIV_FROM_REG(data->count));
	return sprintf(buf, "%d\n", rpm);
}



static ssize_t get_target(struct device *dev, struct device_attribute *devattr,
			 char *buf)
{
	struct max6650_data *data = max6650_update_device(dev);
	int kscale, ktach, rpm;

	

	kscale = DIV_FROM_REG(data->config);
	ktach = data->speed;
	rpm = 60 * kscale * clock / (256 * (ktach + 1));
	return sprintf(buf, "%d\n", rpm);
}

static ssize_t set_target(struct device *dev, struct device_attribute *devattr,
			 const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct max6650_data *data = i2c_get_clientdata(client);
	int rpm = simple_strtoul(buf, NULL, 10);
	int kscale, ktach;

	rpm = SENSORS_LIMIT(rpm, FAN_RPM_MIN, FAN_RPM_MAX);

	

	mutex_lock(&data->update_lock);

	kscale = DIV_FROM_REG(data->config);
	ktach = ((clock * kscale) / (256 * rpm / 60)) - 1;
	if (ktach < 0)
		ktach = 0;
	if (ktach > 255)
		ktach = 255;
	data->speed = ktach;

	i2c_smbus_write_byte_data(client, MAX6650_REG_SPEED, data->speed);

	mutex_unlock(&data->update_lock);

	return count;
}



static ssize_t get_pwm(struct device *dev, struct device_attribute *devattr,
		       char *buf)
{
	int pwm;
	struct max6650_data *data = max6650_update_device(dev);

	
	if (data->config & MAX6650_CFG_V12)
		pwm = 255 - (255 * (int)data->dac)/180;
	else
		pwm = 255 - (255 * (int)data->dac)/76;

	if (pwm < 0)
		pwm = 0;

	return sprintf(buf, "%d\n", pwm);
}

static ssize_t set_pwm(struct device *dev, struct device_attribute *devattr,
			const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct max6650_data *data = i2c_get_clientdata(client);
	int pwm = simple_strtoul(buf, NULL, 10);

	pwm = SENSORS_LIMIT(pwm, 0, 255);

	mutex_lock(&data->update_lock);

	if (data->config & MAX6650_CFG_V12)
		data->dac = 180 - (180 * pwm)/255;
	else
		data->dac = 76 - (76 * pwm)/255;

	i2c_smbus_write_byte_data(client, MAX6650_REG_DAC, data->dac);

	mutex_unlock(&data->update_lock);

	return count;
}



static ssize_t get_enable(struct device *dev, struct device_attribute *devattr,
			  char *buf)
{
	struct max6650_data *data = max6650_update_device(dev);
	int mode = (data->config & MAX6650_CFG_MODE_MASK) >> 4;
	int sysfs_modes[4] = {0, 1, 2, 1};

	return sprintf(buf, "%d\n", sysfs_modes[mode]);
}

static ssize_t set_enable(struct device *dev, struct device_attribute *devattr,
			  const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct max6650_data *data = i2c_get_clientdata(client);
	int mode = simple_strtoul(buf, NULL, 10);
	int max6650_modes[3] = {0, 3, 2};

	if ((mode < 0)||(mode > 2)) {
		dev_err(&client->dev,
			"illegal value for pwm1_enable (%d)\n", mode);
		return -EINVAL;
	}

	mutex_lock(&data->update_lock);

	data->config = i2c_smbus_read_byte_data(client, MAX6650_REG_CONFIG);
	data->config = (data->config & ~MAX6650_CFG_MODE_MASK)
		       | (max6650_modes[mode] << 4);

	i2c_smbus_write_byte_data(client, MAX6650_REG_CONFIG, data->config);

	mutex_unlock(&data->update_lock);

	return count;
}



static ssize_t get_div(struct device *dev, struct device_attribute *devattr,
		       char *buf)
{
	struct max6650_data *data = max6650_update_device(dev);

	return sprintf(buf, "%d\n", DIV_FROM_REG(data->count));
}

static ssize_t set_div(struct device *dev, struct device_attribute *devattr,
		       const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct max6650_data *data = i2c_get_clientdata(client);
	int div = simple_strtoul(buf, NULL, 10);

	mutex_lock(&data->update_lock);
	switch (div) {
	case 1:
		data->count = 0;
		break;
	case 2:
		data->count = 1;
		break;
	case 4:
		data->count = 2;
		break;
	case 8:
		data->count = 3;
		break;
	default:
		mutex_unlock(&data->update_lock);
		dev_err(&client->dev,
			"illegal value for fan divider (%d)\n", div);
		return -EINVAL;
	}

	i2c_smbus_write_byte_data(client, MAX6650_REG_COUNT, data->count);
	mutex_unlock(&data->update_lock);

	return count;
}



static ssize_t get_alarm(struct device *dev, struct device_attribute *devattr,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct max6650_data *data = max6650_update_device(dev);
	struct i2c_client *client = to_i2c_client(dev);
	int alarm = 0;

	if (data->alarm & attr->index) {
		mutex_lock(&data->update_lock);
		alarm = 1;
		data->alarm &= ~attr->index;
		data->alarm |= i2c_smbus_read_byte_data(client,
							MAX6650_REG_ALARM);
		mutex_unlock(&data->update_lock);
	}

	return sprintf(buf, "%d\n", alarm);
}

static SENSOR_DEVICE_ATTR(fan1_input, S_IRUGO, get_fan, NULL, 0);
static SENSOR_DEVICE_ATTR(fan2_input, S_IRUGO, get_fan, NULL, 1);
static SENSOR_DEVICE_ATTR(fan3_input, S_IRUGO, get_fan, NULL, 2);
static SENSOR_DEVICE_ATTR(fan4_input, S_IRUGO, get_fan, NULL, 3);
static DEVICE_ATTR(fan1_target, S_IWUSR | S_IRUGO, get_target, set_target);
static DEVICE_ATTR(fan1_div, S_IWUSR | S_IRUGO, get_div, set_div);
static DEVICE_ATTR(pwm1_enable, S_IWUSR | S_IRUGO, get_enable, set_enable);
static DEVICE_ATTR(pwm1, S_IWUSR | S_IRUGO, get_pwm, set_pwm);
static SENSOR_DEVICE_ATTR(fan1_max_alarm, S_IRUGO, get_alarm, NULL,
			  MAX6650_ALRM_MAX);
static SENSOR_DEVICE_ATTR(fan1_min_alarm, S_IRUGO, get_alarm, NULL,
			  MAX6650_ALRM_MIN);
static SENSOR_DEVICE_ATTR(fan1_fault, S_IRUGO, get_alarm, NULL,
			  MAX6650_ALRM_TACH);
static SENSOR_DEVICE_ATTR(gpio1_alarm, S_IRUGO, get_alarm, NULL,
			  MAX6650_ALRM_GPIO1);
static SENSOR_DEVICE_ATTR(gpio2_alarm, S_IRUGO, get_alarm, NULL,
			  MAX6650_ALRM_GPIO2);

static mode_t max6650_attrs_visible(struct kobject *kobj, struct attribute *a,
				    int n)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct i2c_client *client = to_i2c_client(dev);
	u8 alarm_en = i2c_smbus_read_byte_data(client, MAX6650_REG_ALARM_EN);
	struct device_attribute *devattr;

	

	devattr = container_of(a, struct device_attribute, attr);
	if (devattr == &sensor_dev_attr_fan1_max_alarm.dev_attr
	 || devattr == &sensor_dev_attr_fan1_min_alarm.dev_attr
	 || devattr == &sensor_dev_attr_fan1_fault.dev_attr
	 || devattr == &sensor_dev_attr_gpio1_alarm.dev_attr
	 || devattr == &sensor_dev_attr_gpio2_alarm.dev_attr) {
		if (!(alarm_en & to_sensor_dev_attr(devattr)->index))
			return 0;
	}

	return a->mode;
}

static struct attribute *max6650_attrs[] = {
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,
	&dev_attr_fan1_target.attr,
	&dev_attr_fan1_div.attr,
	&dev_attr_pwm1_enable.attr,
	&dev_attr_pwm1.attr,
	&sensor_dev_attr_fan1_max_alarm.dev_attr.attr,
	&sensor_dev_attr_fan1_min_alarm.dev_attr.attr,
	&sensor_dev_attr_fan1_fault.dev_attr.attr,
	&sensor_dev_attr_gpio1_alarm.dev_attr.attr,
	&sensor_dev_attr_gpio2_alarm.dev_attr.attr,
	NULL
};

static struct attribute_group max6650_attr_grp = {
	.attrs = max6650_attrs,
	.is_visible = max6650_attrs_visible,
};




static int max6650_detect(struct i2c_client *client, int kind,
			  struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
	int address = client->addr;

	dev_dbg(&adapter->dev, "max6650_detect called, kind = %d\n", kind);

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_dbg(&adapter->dev, "max6650: I2C bus doesn't support "
					"byte read mode, skipping.\n");
		return -ENODEV;
	}

	

	if ((kind < 0) &&
	   (  (i2c_smbus_read_byte_data(client, MAX6650_REG_CONFIG) & 0xC0)
	    ||(i2c_smbus_read_byte_data(client, MAX6650_REG_GPIO_STAT) & 0xE0)
	    ||(i2c_smbus_read_byte_data(client, MAX6650_REG_ALARM_EN) & 0xE0)
	    ||(i2c_smbus_read_byte_data(client, MAX6650_REG_ALARM) & 0xE0)
	    ||(i2c_smbus_read_byte_data(client, MAX6650_REG_COUNT) & 0xFC))) {
		dev_dbg(&adapter->dev,
			"max6650: detection failed at 0x%02x.\n", address);
		return -ENODEV;
	}

	dev_info(&adapter->dev, "max6650: chip found at 0x%02x.\n", address);

	strlcpy(info->type, "max6650", I2C_NAME_SIZE);

	return 0;
}

static int max6650_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct max6650_data *data;
	int err;

	if (!(data = kzalloc(sizeof(struct max6650_data), GFP_KERNEL))) {
		dev_err(&client->dev, "out of memory.\n");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);

	
	err = max6650_init_client(client);
	if (err)
		goto err_free;

	err = sysfs_create_group(&client->dev.kobj, &max6650_attr_grp);
	if (err)
		goto err_free;

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (!IS_ERR(data->hwmon_dev))
		return 0;

	err = PTR_ERR(data->hwmon_dev);
	dev_err(&client->dev, "error registering hwmon device.\n");
	sysfs_remove_group(&client->dev.kobj, &max6650_attr_grp);
err_free:
	kfree(data);
	return err;
}

static int max6650_remove(struct i2c_client *client)
{
	struct max6650_data *data = i2c_get_clientdata(client);

	sysfs_remove_group(&client->dev.kobj, &max6650_attr_grp);
	hwmon_device_unregister(data->hwmon_dev);
	kfree(data);
	return 0;
}

static int max6650_init_client(struct i2c_client *client)
{
	struct max6650_data *data = i2c_get_clientdata(client);
	int config;
	int err = -EIO;

	config = i2c_smbus_read_byte_data(client, MAX6650_REG_CONFIG);

	if (config < 0) {
		dev_err(&client->dev, "Error reading config, aborting.\n");
		return err;
	}

	switch (fan_voltage) {
		case 0:
			break;
		case 5:
			config &= ~MAX6650_CFG_V12;
			break;
		case 12:
			config |= MAX6650_CFG_V12;
			break;
		default:
			dev_err(&client->dev,
				"illegal value for fan_voltage (%d)\n",
				fan_voltage);
	}

	dev_info(&client->dev, "Fan voltage is set to %dV.\n",
		 (config & MAX6650_CFG_V12) ? 12 : 5);

	switch (prescaler) {
		case 0:
			break;
		case 1:
			config &= ~MAX6650_CFG_PRESCALER_MASK;
			break;
		case 2:
			config = (config & ~MAX6650_CFG_PRESCALER_MASK)
				 | MAX6650_CFG_PRESCALER_2;
			break;
		case  4:
			config = (config & ~MAX6650_CFG_PRESCALER_MASK)
				 | MAX6650_CFG_PRESCALER_4;
			break;
		case  8:
			config = (config & ~MAX6650_CFG_PRESCALER_MASK)
				 | MAX6650_CFG_PRESCALER_8;
			break;
		case 16:
			config = (config & ~MAX6650_CFG_PRESCALER_MASK)
				 | MAX6650_CFG_PRESCALER_16;
			break;
		default:
			dev_err(&client->dev,
				"illegal value for prescaler (%d)\n",
				prescaler);
	}

	dev_info(&client->dev, "Prescaler is set to %d.\n",
		 1 << (config & MAX6650_CFG_PRESCALER_MASK));

	

	if ((config & MAX6650_CFG_MODE_MASK) == MAX6650_CFG_MODE_OFF) {
		dev_dbg(&client->dev, "Change mode to open loop, full off.\n");
		config = (config & ~MAX6650_CFG_MODE_MASK)
			 | MAX6650_CFG_MODE_OPEN_LOOP;
		if (i2c_smbus_write_byte_data(client, MAX6650_REG_DAC, 255)) {
			dev_err(&client->dev, "DAC write error, aborting.\n");
			return err;
		}
	}

	if (i2c_smbus_write_byte_data(client, MAX6650_REG_CONFIG, config)) {
		dev_err(&client->dev, "Config write error, aborting.\n");
		return err;
	}

	data->config = config;
	data->count = i2c_smbus_read_byte_data(client, MAX6650_REG_COUNT);

	return 0;
}

static const u8 tach_reg[] = {
	MAX6650_REG_TACH0,
	MAX6650_REG_TACH1,
	MAX6650_REG_TACH2,
	MAX6650_REG_TACH3,
};

static struct max6650_data *max6650_update_device(struct device *dev)
{
	int i;
	struct i2c_client *client = to_i2c_client(dev);
	struct max6650_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);

	if (time_after(jiffies, data->last_updated + HZ) || !data->valid) {
		data->speed = i2c_smbus_read_byte_data(client,
						       MAX6650_REG_SPEED);
		data->config = i2c_smbus_read_byte_data(client,
							MAX6650_REG_CONFIG);
		for (i = 0; i < 4; i++) {
			data->tach[i] = i2c_smbus_read_byte_data(client,
								 tach_reg[i]);
		}
		data->count = i2c_smbus_read_byte_data(client,
							MAX6650_REG_COUNT);
		data->dac = i2c_smbus_read_byte_data(client, MAX6650_REG_DAC);

		
		data->alarm |= i2c_smbus_read_byte_data(client,
							MAX6650_REG_ALARM);

		data->last_updated = jiffies;
		data->valid = 1;
	}

	mutex_unlock(&data->update_lock);

	return data;
}

static int __init sensors_max6650_init(void)
{
	return i2c_add_driver(&max6650_driver);
}

static void __exit sensors_max6650_exit(void)
{
	i2c_del_driver(&max6650_driver);
}

MODULE_AUTHOR("Hans J. Koch");
MODULE_DESCRIPTION("MAX6650 sensor driver");
MODULE_LICENSE("GPL");

module_init(sensors_max6650_init);
module_exit(sensors_max6650_exit);
