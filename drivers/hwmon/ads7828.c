

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>


#define ADS7828_NCH 8 
#define ADS7828_CMD_SD_SE 0x80 
#define ADS7828_CMD_SD_DIFF 0x00 
#define ADS7828_CMD_PD0 0x0 
#define ADS7828_CMD_PD1 0x04 
#define ADS7828_CMD_PD2 0x08 
#define ADS7828_CMD_PD3 0x0C 
#define ADS7828_INT_VREF_MV 2500 


static const unsigned short normal_i2c[] = { 0x48, 0x49, 0x4a, 0x4b,
	I2C_CLIENT_END };


I2C_CLIENT_INSMOD_1(ads7828);


static int se_input = 1; 
static int int_vref = 1; 
static int vref_mv = ADS7828_INT_VREF_MV; 
module_param(se_input, bool, S_IRUGO);
module_param(int_vref, bool, S_IRUGO);
module_param(vref_mv, int, S_IRUGO);


static u8 ads7828_cmd_byte; 
static unsigned int ads7828_lsb_resol; 


struct ads7828_data {
	struct device *hwmon_dev;
	struct mutex update_lock; 
	char valid; 
	unsigned long last_updated; 
	u16 adc_input[ADS7828_NCH]; 
};


static int ads7828_detect(struct i2c_client *client, int kind,
			  struct i2c_board_info *info);
static int ads7828_probe(struct i2c_client *client,
			 const struct i2c_device_id *id);


static u16 ads7828_read_value(struct i2c_client *client, u8 reg)
{
	return swab16(i2c_smbus_read_word_data(client, reg));
}

static inline u8 channel_cmd_byte(int ch)
{
	
	u8 cmd = (((ch>>1) | (ch&0x01)<<2)<<4);
	cmd |= ads7828_cmd_byte;
	return cmd;
}


static struct ads7828_data *ads7828_update_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ads7828_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);

	if (time_after(jiffies, data->last_updated + HZ + HZ / 2)
			|| !data->valid) {
		unsigned int ch;
		dev_dbg(&client->dev, "Starting ads7828 update\n");

		for (ch = 0; ch < ADS7828_NCH; ch++) {
			u8 cmd = channel_cmd_byte(ch);
			data->adc_input[ch] = ads7828_read_value(client, cmd);
		}
		data->last_updated = jiffies;
		data->valid = 1;
	}

	mutex_unlock(&data->update_lock);

	return data;
}


static ssize_t show_in(struct device *dev, struct device_attribute *da,
	char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct ads7828_data *data = ads7828_update_device(dev);
	
	return sprintf(buf, "%d\n", (data->adc_input[attr->index] *
		ads7828_lsb_resol)/1000);
}

#define in_reg(offset)\
static SENSOR_DEVICE_ATTR(in##offset##_input, S_IRUGO, show_in,\
	NULL, offset)

in_reg(0);
in_reg(1);
in_reg(2);
in_reg(3);
in_reg(4);
in_reg(5);
in_reg(6);
in_reg(7);

static struct attribute *ads7828_attributes[] = {
	&sensor_dev_attr_in0_input.dev_attr.attr,
	&sensor_dev_attr_in1_input.dev_attr.attr,
	&sensor_dev_attr_in2_input.dev_attr.attr,
	&sensor_dev_attr_in3_input.dev_attr.attr,
	&sensor_dev_attr_in4_input.dev_attr.attr,
	&sensor_dev_attr_in5_input.dev_attr.attr,
	&sensor_dev_attr_in6_input.dev_attr.attr,
	&sensor_dev_attr_in7_input.dev_attr.attr,
	NULL
};

static const struct attribute_group ads7828_group = {
	.attrs = ads7828_attributes,
};

static int ads7828_remove(struct i2c_client *client)
{
	struct ads7828_data *data = i2c_get_clientdata(client);
	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &ads7828_group);
	kfree(i2c_get_clientdata(client));
	return 0;
}

static const struct i2c_device_id ads7828_id[] = {
	{ "ads7828", ads7828 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ads7828_id);


static struct i2c_driver ads7828_driver = {
	.class = I2C_CLASS_HWMON,
	.driver = {
		.name = "ads7828",
	},
	.probe = ads7828_probe,
	.remove = ads7828_remove,
	.id_table = ads7828_id,
	.detect = ads7828_detect,
	.address_data = &addr_data,
};


static int ads7828_detect(struct i2c_client *client, int kind,
			  struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;

	
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_READ_WORD_DATA))
		return -ENODEV;

	
	if (kind < 0) {
		int ch;
		for (ch = 0; ch < ADS7828_NCH; ch++) {
			u16 in_data;
			u8 cmd = channel_cmd_byte(ch);
			in_data = ads7828_read_value(client, cmd);
			if (in_data & 0xF000) {
				printk(KERN_DEBUG
				"%s : Doesn't look like an ads7828 device\n",
				__func__);
				return -ENODEV;
			}
		}
	}
	strlcpy(info->type, "ads7828", I2C_NAME_SIZE);

	return 0;
}

static int ads7828_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct ads7828_data *data;
	int err;

	data = kzalloc(sizeof(struct ads7828_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);

	
	err = sysfs_create_group(&client->dev.kobj, &ads7828_group);
	if (err)
		goto exit_free;

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		err = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &ads7828_group);
exit_free:
	kfree(data);
exit:
	return err;
}

static int __init sensors_ads7828_init(void)
{
	
	ads7828_cmd_byte = se_input ?
		ADS7828_CMD_SD_SE : ADS7828_CMD_SD_DIFF;
	ads7828_cmd_byte |= int_vref ?
		ADS7828_CMD_PD3 : ADS7828_CMD_PD1;

	
	ads7828_lsb_resol = (vref_mv*1000)/4096;

	return i2c_add_driver(&ads7828_driver);
}

static void __exit sensors_ads7828_exit(void)
{
	i2c_del_driver(&ads7828_driver);
}

MODULE_AUTHOR("Steve Hardy <steve@linuxrealtime.co.uk>");
MODULE_DESCRIPTION("ADS7828 driver");
MODULE_LICENSE("GPL");

module_init(sensors_ads7828_init);
module_exit(sensors_ads7828_exit);
