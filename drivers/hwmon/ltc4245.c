

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>


enum ltc4245_cmd {
	LTC4245_STATUS			= 0x00, 
	LTC4245_ALERT			= 0x01,
	LTC4245_CONTROL			= 0x02,
	LTC4245_ON			= 0x03,
	LTC4245_FAULT1			= 0x04,
	LTC4245_FAULT2			= 0x05,
	LTC4245_GPIO			= 0x06,
	LTC4245_ADCADR			= 0x07,

	LTC4245_12VIN			= 0x10,
	LTC4245_12VSENSE		= 0x11,
	LTC4245_12VOUT			= 0x12,
	LTC4245_5VIN			= 0x13,
	LTC4245_5VSENSE			= 0x14,
	LTC4245_5VOUT			= 0x15,
	LTC4245_3VIN			= 0x16,
	LTC4245_3VSENSE			= 0x17,
	LTC4245_3VOUT			= 0x18,
	LTC4245_VEEIN			= 0x19,
	LTC4245_VEESENSE		= 0x1a,
	LTC4245_VEEOUT			= 0x1b,
	LTC4245_GPIOADC1		= 0x1c,
	LTC4245_GPIOADC2		= 0x1d,
	LTC4245_GPIOADC3		= 0x1e,
};

struct ltc4245_data {
	struct device *hwmon_dev;

	struct mutex update_lock;
	bool valid;
	unsigned long last_updated; 

	
	u8 cregs[0x08];

	
	u8 vregs[0x0f];
};

static struct ltc4245_data *ltc4245_update_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ltc4245_data *data = i2c_get_clientdata(client);
	s32 val;
	int i;

	mutex_lock(&data->update_lock);

	if (time_after(jiffies, data->last_updated + HZ) || !data->valid) {

		dev_dbg(&client->dev, "Starting ltc4245 update\n");

		
		for (i = 0; i < ARRAY_SIZE(data->cregs); i++) {
			val = i2c_smbus_read_byte_data(client, i);
			if (unlikely(val < 0))
				data->cregs[i] = 0;
			else
				data->cregs[i] = val;
		}

		
		for (i = 0; i < ARRAY_SIZE(data->vregs); i++) {
			val = i2c_smbus_read_byte_data(client, i+0x10);
			if (unlikely(val < 0))
				data->vregs[i] = 0;
			else
				data->vregs[i] = val;
		}

		data->last_updated = jiffies;
		data->valid = 1;
	}

	mutex_unlock(&data->update_lock);

	return data;
}


static int ltc4245_get_voltage(struct device *dev, u8 reg)
{
	struct ltc4245_data *data = ltc4245_update_device(dev);
	const u8 regval = data->vregs[reg - 0x10];
	u32 voltage = 0;

	switch (reg) {
	case LTC4245_12VIN:
	case LTC4245_12VOUT:
		voltage = regval * 55;
		break;
	case LTC4245_5VIN:
	case LTC4245_5VOUT:
		voltage = regval * 22;
		break;
	case LTC4245_3VIN:
	case LTC4245_3VOUT:
		voltage = regval * 15;
		break;
	case LTC4245_VEEIN:
	case LTC4245_VEEOUT:
		voltage = regval * -55;
		break;
	case LTC4245_GPIOADC1:
	case LTC4245_GPIOADC2:
	case LTC4245_GPIOADC3:
		voltage = regval * 10;
		break;
	default:
		
		WARN_ON_ONCE(1);
		break;
	}

	return voltage;
}


static unsigned int ltc4245_get_current(struct device *dev, u8 reg)
{
	struct ltc4245_data *data = ltc4245_update_device(dev);
	const u8 regval = data->vregs[reg - 0x10];
	unsigned int voltage;
	unsigned int curr;

	

	switch (reg) {
	case LTC4245_12VSENSE:
		voltage = regval * 250; 
		curr = voltage / 50; 
		break;
	case LTC4245_5VSENSE:
		voltage = regval * 125; 
		curr = (voltage * 10) / 35; 
		break;
	case LTC4245_3VSENSE:
		voltage = regval * 125; 
		curr = (voltage * 10) / 25; 
		break;
	case LTC4245_VEESENSE:
		voltage = regval * 250; 
		curr = voltage / 100; 
		break;
	default:
		
		WARN_ON_ONCE(1);
		curr = 0;
		break;
	}

	return curr;
}

static ssize_t ltc4245_show_voltage(struct device *dev,
				    struct device_attribute *da,
				    char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	const int voltage = ltc4245_get_voltage(dev, attr->index);

	return snprintf(buf, PAGE_SIZE, "%d\n", voltage);
}

static ssize_t ltc4245_show_current(struct device *dev,
				    struct device_attribute *da,
				    char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	const unsigned int curr = ltc4245_get_current(dev, attr->index);

	return snprintf(buf, PAGE_SIZE, "%u\n", curr);
}

static ssize_t ltc4245_show_power(struct device *dev,
				  struct device_attribute *da,
				  char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	const unsigned int curr = ltc4245_get_current(dev, attr->index);
	const int output_voltage = ltc4245_get_voltage(dev, attr->index+1);

	
	const unsigned int power = abs(output_voltage * curr);

	return snprintf(buf, PAGE_SIZE, "%u\n", power);
}

static ssize_t ltc4245_show_alarm(struct device *dev,
					  struct device_attribute *da,
					  char *buf)
{
	struct sensor_device_attribute_2 *attr = to_sensor_dev_attr_2(da);
	struct ltc4245_data *data = ltc4245_update_device(dev);
	const u8 reg = data->cregs[attr->index];
	const u32 mask = attr->nr;

	return snprintf(buf, PAGE_SIZE, "%u\n", (reg & mask) ? 1 : 0);
}



#define LTC4245_VOLTAGE(name, ltc4245_cmd_idx) \
	static SENSOR_DEVICE_ATTR(name, S_IRUGO, \
	ltc4245_show_voltage, NULL, ltc4245_cmd_idx)

#define LTC4245_CURRENT(name, ltc4245_cmd_idx) \
	static SENSOR_DEVICE_ATTR(name, S_IRUGO, \
	ltc4245_show_current, NULL, ltc4245_cmd_idx)

#define LTC4245_POWER(name, ltc4245_cmd_idx) \
	static SENSOR_DEVICE_ATTR(name, S_IRUGO, \
	ltc4245_show_power, NULL, ltc4245_cmd_idx)

#define LTC4245_ALARM(name, mask, reg) \
	static SENSOR_DEVICE_ATTR_2(name, S_IRUGO, \
	ltc4245_show_alarm, NULL, (mask), reg)




LTC4245_VOLTAGE(in1_input,			LTC4245_12VIN);
LTC4245_VOLTAGE(in2_input,			LTC4245_5VIN);
LTC4245_VOLTAGE(in3_input,			LTC4245_3VIN);
LTC4245_VOLTAGE(in4_input,			LTC4245_VEEIN);


LTC4245_ALARM(in1_min_alarm,	(1 << 0),	LTC4245_FAULT1);
LTC4245_ALARM(in2_min_alarm,	(1 << 1),	LTC4245_FAULT1);
LTC4245_ALARM(in3_min_alarm,	(1 << 2),	LTC4245_FAULT1);
LTC4245_ALARM(in4_min_alarm,	(1 << 3),	LTC4245_FAULT1);


LTC4245_CURRENT(curr1_input,			LTC4245_12VSENSE);
LTC4245_CURRENT(curr2_input,			LTC4245_5VSENSE);
LTC4245_CURRENT(curr3_input,			LTC4245_3VSENSE);
LTC4245_CURRENT(curr4_input,			LTC4245_VEESENSE);


LTC4245_ALARM(curr1_max_alarm,	(1 << 4),	LTC4245_FAULT1);
LTC4245_ALARM(curr2_max_alarm,	(1 << 5),	LTC4245_FAULT1);
LTC4245_ALARM(curr3_max_alarm,	(1 << 6),	LTC4245_FAULT1);
LTC4245_ALARM(curr4_max_alarm,	(1 << 7),	LTC4245_FAULT1);


LTC4245_VOLTAGE(in5_input,			LTC4245_12VOUT);
LTC4245_VOLTAGE(in6_input,			LTC4245_5VOUT);
LTC4245_VOLTAGE(in7_input,			LTC4245_3VOUT);
LTC4245_VOLTAGE(in8_input,			LTC4245_VEEOUT);


LTC4245_ALARM(in5_min_alarm,	(1 << 0),	LTC4245_FAULT2);
LTC4245_ALARM(in6_min_alarm,	(1 << 1),	LTC4245_FAULT2);
LTC4245_ALARM(in7_min_alarm,	(1 << 2),	LTC4245_FAULT2);
LTC4245_ALARM(in8_min_alarm,	(1 << 3),	LTC4245_FAULT2);


LTC4245_VOLTAGE(in9_input,			LTC4245_GPIOADC1);
LTC4245_VOLTAGE(in10_input,			LTC4245_GPIOADC2);
LTC4245_VOLTAGE(in11_input,			LTC4245_GPIOADC3);


LTC4245_POWER(power1_input,			LTC4245_12VSENSE);
LTC4245_POWER(power2_input,			LTC4245_5VSENSE);
LTC4245_POWER(power3_input,			LTC4245_3VSENSE);
LTC4245_POWER(power4_input,			LTC4245_VEESENSE);


static struct attribute *ltc4245_attributes[] = {
	&sensor_dev_attr_in1_input.dev_attr.attr,
	&sensor_dev_attr_in2_input.dev_attr.attr,
	&sensor_dev_attr_in3_input.dev_attr.attr,
	&sensor_dev_attr_in4_input.dev_attr.attr,

	&sensor_dev_attr_in1_min_alarm.dev_attr.attr,
	&sensor_dev_attr_in2_min_alarm.dev_attr.attr,
	&sensor_dev_attr_in3_min_alarm.dev_attr.attr,
	&sensor_dev_attr_in4_min_alarm.dev_attr.attr,

	&sensor_dev_attr_curr1_input.dev_attr.attr,
	&sensor_dev_attr_curr2_input.dev_attr.attr,
	&sensor_dev_attr_curr3_input.dev_attr.attr,
	&sensor_dev_attr_curr4_input.dev_attr.attr,

	&sensor_dev_attr_curr1_max_alarm.dev_attr.attr,
	&sensor_dev_attr_curr2_max_alarm.dev_attr.attr,
	&sensor_dev_attr_curr3_max_alarm.dev_attr.attr,
	&sensor_dev_attr_curr4_max_alarm.dev_attr.attr,

	&sensor_dev_attr_in5_input.dev_attr.attr,
	&sensor_dev_attr_in6_input.dev_attr.attr,
	&sensor_dev_attr_in7_input.dev_attr.attr,
	&sensor_dev_attr_in8_input.dev_attr.attr,

	&sensor_dev_attr_in5_min_alarm.dev_attr.attr,
	&sensor_dev_attr_in6_min_alarm.dev_attr.attr,
	&sensor_dev_attr_in7_min_alarm.dev_attr.attr,
	&sensor_dev_attr_in8_min_alarm.dev_attr.attr,

	&sensor_dev_attr_in9_input.dev_attr.attr,
	&sensor_dev_attr_in10_input.dev_attr.attr,
	&sensor_dev_attr_in11_input.dev_attr.attr,

	&sensor_dev_attr_power1_input.dev_attr.attr,
	&sensor_dev_attr_power2_input.dev_attr.attr,
	&sensor_dev_attr_power3_input.dev_attr.attr,
	&sensor_dev_attr_power4_input.dev_attr.attr,

	NULL,
};

static const struct attribute_group ltc4245_group = {
	.attrs = ltc4245_attributes,
};

static int ltc4245_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = client->adapter;
	struct ltc4245_data *data;
	int ret;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -ENODEV;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto out_kzalloc;
	}

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);

	
	i2c_smbus_write_byte_data(client, LTC4245_FAULT1, 0x00);
	i2c_smbus_write_byte_data(client, LTC4245_FAULT2, 0x00);

	
	ret = sysfs_create_group(&client->dev.kobj, &ltc4245_group);
	if (ret)
		goto out_sysfs_create_group;

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		ret = PTR_ERR(data->hwmon_dev);
		goto out_hwmon_device_register;
	}

	return 0;

out_hwmon_device_register:
	sysfs_remove_group(&client->dev.kobj, &ltc4245_group);
out_sysfs_create_group:
	kfree(data);
out_kzalloc:
	return ret;
}

static int ltc4245_remove(struct i2c_client *client)
{
	struct ltc4245_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &ltc4245_group);

	kfree(data);

	return 0;
}

static const struct i2c_device_id ltc4245_id[] = {
	{ "ltc4245", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ltc4245_id);


static struct i2c_driver ltc4245_driver = {
	.driver = {
		.name	= "ltc4245",
	},
	.probe		= ltc4245_probe,
	.remove		= ltc4245_remove,
	.id_table	= ltc4245_id,
};

static int __init ltc4245_init(void)
{
	return i2c_add_driver(&ltc4245_driver);
}

static void __exit ltc4245_exit(void)
{
	i2c_del_driver(&ltc4245_driver);
}

MODULE_AUTHOR("Ira W. Snyder <iws@ovro.caltech.edu>");
MODULE_DESCRIPTION("LTC4245 driver");
MODULE_LICENSE("GPL");

module_init(ltc4245_init);
module_exit(ltc4245_exit);
