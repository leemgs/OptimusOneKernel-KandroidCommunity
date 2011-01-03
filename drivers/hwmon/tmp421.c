



#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>


static unsigned short normal_i2c[] = { 0x2a, 0x4c, 0x4d, 0x4e, 0x4f,
				       I2C_CLIENT_END };


I2C_CLIENT_INSMOD_3(tmp421, tmp422, tmp423);


#define TMP421_CONFIG_REG_1			0x09
#define TMP421_CONVERSION_RATE_REG		0x0B
#define TMP421_MANUFACTURER_ID_REG		0xFE
#define TMP421_DEVICE_ID_REG			0xFF

static const u8 TMP421_TEMP_MSB[4]		= { 0x00, 0x01, 0x02, 0x03 };
static const u8 TMP421_TEMP_LSB[4]		= { 0x10, 0x11, 0x12, 0x13 };


#define TMP421_CONFIG_SHUTDOWN			0x40
#define TMP421_CONFIG_RANGE			0x04


#define TMP421_MANUFACTURER_ID			0x55
#define TMP421_DEVICE_ID			0x21
#define TMP422_DEVICE_ID			0x22
#define TMP423_DEVICE_ID			0x23

static const struct i2c_device_id tmp421_id[] = {
	{ "tmp421", tmp421 },
	{ "tmp422", tmp422 },
	{ "tmp423", tmp423 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, tmp421_id);

struct tmp421_data {
	struct device *hwmon_dev;
	struct mutex update_lock;
	char valid;
	unsigned long last_updated;
	int kind;
	u8 config;
	s16 temp[4];
};

static int temp_from_s16(s16 reg)
{
	int temp = reg;

	return (temp * 1000 + 128) / 256;
}

static int temp_from_u16(u16 reg)
{
	int temp = reg;

	
	temp -= 64 * 256;

	return (temp * 1000 + 128) / 256;
}

static struct tmp421_data *tmp421_update_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct tmp421_data *data = i2c_get_clientdata(client);
	int i;

	mutex_lock(&data->update_lock);

	if (time_after(jiffies, data->last_updated + 2 * HZ) || !data->valid) {
		data->config = i2c_smbus_read_byte_data(client,
			TMP421_CONFIG_REG_1);

		for (i = 0; i <= data->kind; i++) {
			data->temp[i] = i2c_smbus_read_byte_data(client,
				TMP421_TEMP_MSB[i]) << 8;
			data->temp[i] |= i2c_smbus_read_byte_data(client,
				TMP421_TEMP_LSB[i]);
		}
		data->last_updated = jiffies;
		data->valid = 1;
	}

	mutex_unlock(&data->update_lock);

	return data;
}

static ssize_t show_temp_value(struct device *dev,
			       struct device_attribute *devattr, char *buf)
{
	int index = to_sensor_dev_attr(devattr)->index;
	struct tmp421_data *data = tmp421_update_device(dev);
	int temp;

	mutex_lock(&data->update_lock);
	if (data->config & TMP421_CONFIG_RANGE)
		temp = temp_from_u16(data->temp[index]);
	else
		temp = temp_from_s16(data->temp[index]);
	mutex_unlock(&data->update_lock);

	return sprintf(buf, "%d\n", temp);
}

static ssize_t show_fault(struct device *dev,
			  struct device_attribute *devattr, char *buf)
{
	int index = to_sensor_dev_attr(devattr)->index;
	struct tmp421_data *data = tmp421_update_device(dev);

	
	if (data->temp[index] & 0x01)
		return sprintf(buf, "1\n");
	else
		return sprintf(buf, "0\n");
}

static mode_t tmp421_is_visible(struct kobject *kobj, struct attribute *a,
				int n)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct tmp421_data *data = dev_get_drvdata(dev);
	struct device_attribute *devattr;
	unsigned int index;

	devattr = container_of(a, struct device_attribute, attr);
	index = to_sensor_dev_attr(devattr)->index;

	if (data->kind > index)
		return a->mode;

	return 0;
}

static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, show_temp_value, NULL, 0);
static SENSOR_DEVICE_ATTR(temp2_input, S_IRUGO, show_temp_value, NULL, 1);
static SENSOR_DEVICE_ATTR(temp2_fault, S_IRUGO, show_fault, NULL, 1);
static SENSOR_DEVICE_ATTR(temp3_input, S_IRUGO, show_temp_value, NULL, 2);
static SENSOR_DEVICE_ATTR(temp3_fault, S_IRUGO, show_fault, NULL, 2);
static SENSOR_DEVICE_ATTR(temp4_input, S_IRUGO, show_temp_value, NULL, 3);
static SENSOR_DEVICE_ATTR(temp4_fault, S_IRUGO, show_fault, NULL, 3);

static struct attribute *tmp421_attr[] = {
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	&sensor_dev_attr_temp2_fault.dev_attr.attr,
	&sensor_dev_attr_temp3_input.dev_attr.attr,
	&sensor_dev_attr_temp3_fault.dev_attr.attr,
	&sensor_dev_attr_temp4_input.dev_attr.attr,
	&sensor_dev_attr_temp4_fault.dev_attr.attr,
	NULL
};

static const struct attribute_group tmp421_group = {
	.attrs = tmp421_attr,
	.is_visible = tmp421_is_visible,
};

static int tmp421_init_client(struct i2c_client *client)
{
	int config, config_orig;

	
	i2c_smbus_write_byte_data(client, TMP421_CONVERSION_RATE_REG, 0x05);

	
	config = i2c_smbus_read_byte_data(client, TMP421_CONFIG_REG_1);
	if (config < 0) {
		dev_err(&client->dev, "Could not read configuration"
			 " register (%d)\n", config);
		return -ENODEV;
	}

	config_orig = config;
	config &= ~TMP421_CONFIG_SHUTDOWN;

	if (config != config_orig) {
		dev_info(&client->dev, "Enable monitoring chip\n");
		i2c_smbus_write_byte_data(client, TMP421_CONFIG_REG_1, config);
	}

	return 0;
}

static int tmp421_detect(struct i2c_client *client, int kind,
			 struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
	const char *names[] = { "TMP421", "TMP422", "TMP423" };

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -ENODEV;

	if (kind <= 0) {
		u8 reg;

		reg = i2c_smbus_read_byte_data(client,
					       TMP421_MANUFACTURER_ID_REG);
		if (reg != TMP421_MANUFACTURER_ID)
			return -ENODEV;

		reg = i2c_smbus_read_byte_data(client,
					       TMP421_DEVICE_ID_REG);
		switch (reg) {
		case TMP421_DEVICE_ID:
			kind = tmp421;
			break;
		case TMP422_DEVICE_ID:
			kind = tmp422;
			break;
		case TMP423_DEVICE_ID:
			kind = tmp423;
			break;
		default:
			return -ENODEV;
		}
	}
	strlcpy(info->type, tmp421_id[kind - 1].name, I2C_NAME_SIZE);
	dev_info(&adapter->dev, "Detected TI %s chip at 0x%02x\n",
		 names[kind - 1], client->addr);

	return 0;
}

static int tmp421_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct tmp421_data *data;
	int err;

	data = kzalloc(sizeof(struct tmp421_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);
	data->kind = id->driver_data;

	err = tmp421_init_client(client);
	if (err)
		goto exit_free;

	err = sysfs_create_group(&client->dev.kobj, &tmp421_group);
	if (err)
		goto exit_free;

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		err = PTR_ERR(data->hwmon_dev);
		data->hwmon_dev = NULL;
		goto exit_remove;
	}
	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &tmp421_group);

exit_free:
	i2c_set_clientdata(client, NULL);
	kfree(data);

	return err;
}

static int tmp421_remove(struct i2c_client *client)
{
	struct tmp421_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &tmp421_group);

	i2c_set_clientdata(client, NULL);
	kfree(data);

	return 0;
}

static struct i2c_driver tmp421_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name	= "tmp421",
	},
	.probe = tmp421_probe,
	.remove = tmp421_remove,
	.id_table = tmp421_id,
	.detect = tmp421_detect,
	.address_data = &addr_data,
};

static int __init tmp421_init(void)
{
	return i2c_add_driver(&tmp421_driver);
}

static void __exit tmp421_exit(void)
{
	i2c_del_driver(&tmp421_driver);
}

MODULE_AUTHOR("Andre Prendel <andre.prendel@gmx.de>");
MODULE_DESCRIPTION("Texas Instruments TMP421/422/423 temperature sensor"
		   " driver");
MODULE_LICENSE("GPL");

module_init(tmp421_init);
module_exit(tmp421_exit);
