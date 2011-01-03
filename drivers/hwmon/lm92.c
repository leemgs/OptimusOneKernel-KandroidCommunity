

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>


static const unsigned short normal_i2c[] = { 0x48, 0x49, 0x4a, 0x4b,
						I2C_CLIENT_END };


I2C_CLIENT_INSMOD_1(lm92);


#define LM92_REG_CONFIG			0x01 
#define LM92_REG_TEMP			0x00 
#define LM92_REG_TEMP_HYST		0x02 
#define LM92_REG_TEMP_CRIT		0x03 
#define LM92_REG_TEMP_LOW		0x04 
#define LM92_REG_TEMP_HIGH		0x05 
#define LM92_REG_MAN_ID			0x07 


static inline int TEMP_FROM_REG(s16 reg)
{
	return reg / 8 * 625 / 10;
}

static inline s16 TEMP_TO_REG(int val)
{
	if (val <= -60000)
		return -60000 * 10 / 625 * 8;
	if (val >= 160000)
		return 160000 * 10 / 625 * 8;
	return val * 10 / 625 * 8;
}


static inline u8 ALARMS_FROM_REG(s16 reg)
{
	return reg & 0x0007;
}


static struct i2c_driver lm92_driver;


struct lm92_data {
	struct device *hwmon_dev;
	struct mutex update_lock;
	char valid; 
	unsigned long last_updated; 

	
	s16 temp1_input, temp1_crit, temp1_min, temp1_max, temp1_hyst;
};




static struct lm92_data *lm92_update_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct lm92_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);

	if (time_after(jiffies, data->last_updated + HZ)
	 || !data->valid) {
		dev_dbg(&client->dev, "Updating lm92 data\n");
		data->temp1_input = swab16(i2c_smbus_read_word_data(client,
				    LM92_REG_TEMP));
		data->temp1_hyst = swab16(i2c_smbus_read_word_data(client,
				    LM92_REG_TEMP_HYST));
		data->temp1_crit = swab16(i2c_smbus_read_word_data(client,
				    LM92_REG_TEMP_CRIT));
		data->temp1_min = swab16(i2c_smbus_read_word_data(client,
				    LM92_REG_TEMP_LOW));
		data->temp1_max = swab16(i2c_smbus_read_word_data(client,
				    LM92_REG_TEMP_HIGH));

		data->last_updated = jiffies;
		data->valid = 1;
	}

	mutex_unlock(&data->update_lock);

	return data;
}

#define show_temp(value) \
static ssize_t show_##value(struct device *dev, struct device_attribute *attr, char *buf) \
{ \
	struct lm92_data *data = lm92_update_device(dev); \
	return sprintf(buf, "%d\n", TEMP_FROM_REG(data->value)); \
}
show_temp(temp1_input);
show_temp(temp1_crit);
show_temp(temp1_min);
show_temp(temp1_max);

#define set_temp(value, reg) \
static ssize_t set_##value(struct device *dev, struct device_attribute *attr, const char *buf, \
	size_t count) \
{ \
	struct i2c_client *client = to_i2c_client(dev); \
	struct lm92_data *data = i2c_get_clientdata(client); \
	long val = simple_strtol(buf, NULL, 10); \
 \
	mutex_lock(&data->update_lock); \
	data->value = TEMP_TO_REG(val); \
	i2c_smbus_write_word_data(client, reg, swab16(data->value)); \
	mutex_unlock(&data->update_lock); \
	return count; \
}
set_temp(temp1_crit, LM92_REG_TEMP_CRIT);
set_temp(temp1_min, LM92_REG_TEMP_LOW);
set_temp(temp1_max, LM92_REG_TEMP_HIGH);

static ssize_t show_temp1_crit_hyst(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct lm92_data *data = lm92_update_device(dev);
	return sprintf(buf, "%d\n", TEMP_FROM_REG(data->temp1_crit)
		       - TEMP_FROM_REG(data->temp1_hyst));
}
static ssize_t show_temp1_max_hyst(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct lm92_data *data = lm92_update_device(dev);
	return sprintf(buf, "%d\n", TEMP_FROM_REG(data->temp1_max)
		       - TEMP_FROM_REG(data->temp1_hyst));
}
static ssize_t show_temp1_min_hyst(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct lm92_data *data = lm92_update_device(dev);
	return sprintf(buf, "%d\n", TEMP_FROM_REG(data->temp1_min)
		       + TEMP_FROM_REG(data->temp1_hyst));
}

static ssize_t set_temp1_crit_hyst(struct device *dev, struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct lm92_data *data = i2c_get_clientdata(client);
	long val = simple_strtol(buf, NULL, 10);

	mutex_lock(&data->update_lock);
	data->temp1_hyst = TEMP_FROM_REG(data->temp1_crit) - val;
	i2c_smbus_write_word_data(client, LM92_REG_TEMP_HYST,
				  swab16(TEMP_TO_REG(data->temp1_hyst)));
	mutex_unlock(&data->update_lock);
	return count;
}

static ssize_t show_alarms(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct lm92_data *data = lm92_update_device(dev);
	return sprintf(buf, "%d\n", ALARMS_FROM_REG(data->temp1_input));
}

static ssize_t show_alarm(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	int bitnr = to_sensor_dev_attr(attr)->index;
	struct lm92_data *data = lm92_update_device(dev);
	return sprintf(buf, "%d\n", (data->temp1_input >> bitnr) & 1);
}

static DEVICE_ATTR(temp1_input, S_IRUGO, show_temp1_input, NULL);
static DEVICE_ATTR(temp1_crit, S_IWUSR | S_IRUGO, show_temp1_crit,
	set_temp1_crit);
static DEVICE_ATTR(temp1_crit_hyst, S_IWUSR | S_IRUGO, show_temp1_crit_hyst,
	set_temp1_crit_hyst);
static DEVICE_ATTR(temp1_min, S_IWUSR | S_IRUGO, show_temp1_min,
	set_temp1_min);
static DEVICE_ATTR(temp1_min_hyst, S_IRUGO, show_temp1_min_hyst, NULL);
static DEVICE_ATTR(temp1_max, S_IWUSR | S_IRUGO, show_temp1_max,
	set_temp1_max);
static DEVICE_ATTR(temp1_max_hyst, S_IRUGO, show_temp1_max_hyst, NULL);
static DEVICE_ATTR(alarms, S_IRUGO, show_alarms, NULL);
static SENSOR_DEVICE_ATTR(temp1_crit_alarm, S_IRUGO, show_alarm, NULL, 2);
static SENSOR_DEVICE_ATTR(temp1_min_alarm, S_IRUGO, show_alarm, NULL, 0);
static SENSOR_DEVICE_ATTR(temp1_max_alarm, S_IRUGO, show_alarm, NULL, 1);




static void lm92_init_client(struct i2c_client *client)
{
	u8 config;

	
	config = i2c_smbus_read_byte_data(client, LM92_REG_CONFIG);
	if (config & 0x01)
		i2c_smbus_write_byte_data(client, LM92_REG_CONFIG,
					  config & 0xFE);
}


static int max6635_check(struct i2c_client *client)
{
	u16 temp_low, temp_high, temp_hyst, temp_crit;
	u8 conf;
	int i;

	
	temp_low = i2c_smbus_read_word_data(client, LM92_REG_TEMP_LOW);
	if (i2c_smbus_read_word_data(client, LM92_REG_MAN_ID) != temp_low)
		return 0;
	temp_high = i2c_smbus_read_word_data(client, LM92_REG_TEMP_HIGH);
	if (i2c_smbus_read_word_data(client, LM92_REG_MAN_ID) != temp_high)
		return 0;
	
	
	if ((temp_low & 0x7f00) || (temp_high & 0x7f00))
		return 0;
	temp_hyst = i2c_smbus_read_word_data(client, LM92_REG_TEMP_HYST);
	temp_crit = i2c_smbus_read_word_data(client, LM92_REG_TEMP_CRIT);
	if ((temp_hyst & 0x7f00) || (temp_crit & 0x7f00))
		return 0;

	
	conf = i2c_smbus_read_byte_data(client, LM92_REG_CONFIG);
	for (i=16; i<96; i*=2) {
		if (temp_hyst != i2c_smbus_read_word_data(client,
		 		 LM92_REG_TEMP_HYST + i - 16)
		 || temp_crit != i2c_smbus_read_word_data(client,
		 		 LM92_REG_TEMP_CRIT + i)
		 || temp_low != i2c_smbus_read_word_data(client,
				LM92_REG_TEMP_LOW + i + 16)
		 || temp_high != i2c_smbus_read_word_data(client,
		 		 LM92_REG_TEMP_HIGH + i + 32)
		 || conf != i2c_smbus_read_byte_data(client,
		 	    LM92_REG_CONFIG + i))
			return 0;
	}

	return 1;
}

static struct attribute *lm92_attributes[] = {
	&dev_attr_temp1_input.attr,
	&dev_attr_temp1_crit.attr,
	&dev_attr_temp1_crit_hyst.attr,
	&dev_attr_temp1_min.attr,
	&dev_attr_temp1_min_hyst.attr,
	&dev_attr_temp1_max.attr,
	&dev_attr_temp1_max_hyst.attr,
	&dev_attr_alarms.attr,
	&sensor_dev_attr_temp1_crit_alarm.dev_attr.attr,
	&sensor_dev_attr_temp1_min_alarm.dev_attr.attr,
	&sensor_dev_attr_temp1_max_alarm.dev_attr.attr,
	NULL
};

static const struct attribute_group lm92_group = {
	.attrs = lm92_attributes,
};


static int lm92_detect(struct i2c_client *new_client, int kind,
		       struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = new_client->adapter;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA
					    | I2C_FUNC_SMBUS_WORD_DATA))
		return -ENODEV;

	
	if (kind < 0) {
		u8 config = i2c_smbus_read_byte_data(new_client,
			     LM92_REG_CONFIG);
		u16 man_id = i2c_smbus_read_word_data(new_client,
			     LM92_REG_MAN_ID);

		if ((config & 0xe0) == 0x00
		 && man_id == 0x0180) {
			pr_info("lm92: Found National Semiconductor LM92 chip\n");
	 		kind = lm92;
		} else
		if (max6635_check(new_client)) {
			pr_info("lm92: Found Maxim MAX6635 chip\n");
			kind = lm92; 
		}
		else
			return -ENODEV;
	}

	strlcpy(info->type, "lm92", I2C_NAME_SIZE);

	return 0;
}

static int lm92_probe(struct i2c_client *new_client,
		      const struct i2c_device_id *id)
{
	struct lm92_data *data;
	int err;

	data = kzalloc(sizeof(struct lm92_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(new_client, data);
	data->valid = 0;
	mutex_init(&data->update_lock);

	
	lm92_init_client(new_client);

	
	if ((err = sysfs_create_group(&new_client->dev.kobj, &lm92_group)))
		goto exit_free;

	data->hwmon_dev = hwmon_device_register(&new_client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		err = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}

	return 0;

exit_remove:
	sysfs_remove_group(&new_client->dev.kobj, &lm92_group);
exit_free:
	kfree(data);
exit:
	return err;
}

static int lm92_remove(struct i2c_client *client)
{
	struct lm92_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &lm92_group);

	kfree(data);
	return 0;
}




static const struct i2c_device_id lm92_id[] = {
	{ "lm92", lm92 },
	
	{ }
};
MODULE_DEVICE_TABLE(i2c, lm92_id);

static struct i2c_driver lm92_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "lm92",
	},
	.probe		= lm92_probe,
	.remove		= lm92_remove,
	.id_table	= lm92_id,
	.detect		= lm92_detect,
	.address_data	= &addr_data,
};

static int __init sensors_lm92_init(void)
{
	return i2c_add_driver(&lm92_driver);
}

static void __exit sensors_lm92_exit(void)
{
	i2c_del_driver(&lm92_driver);
}

MODULE_AUTHOR("Jean Delvare <khali@linux-fr.org>");
MODULE_DESCRIPTION("LM92/MAX6635 driver");
MODULE_LICENSE("GPL");

module_init(sensors_lm92_init);
module_exit(sensors_lm92_exit);
