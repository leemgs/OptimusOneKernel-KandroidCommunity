

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon-sysfs.h>
#include <linux/hwmon.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>



static const unsigned short normal_i2c[] = {
	0x18, 0x19, 0x1a, 0x29, 0x2a, 0x2b, 0x4c, 0x4d, 0x4e, I2C_CLIENT_END };



I2C_CLIENT_INSMOD_8(lm90, adm1032, lm99, lm86, max6657, adt7461, max6680,
		    max6646);



#define LM90_REG_R_MAN_ID		0xFE
#define LM90_REG_R_CHIP_ID		0xFF
#define LM90_REG_R_CONFIG1		0x03
#define LM90_REG_W_CONFIG1		0x09
#define LM90_REG_R_CONFIG2		0xBF
#define LM90_REG_W_CONFIG2		0xBF
#define LM90_REG_R_CONVRATE		0x04
#define LM90_REG_W_CONVRATE		0x0A
#define LM90_REG_R_STATUS		0x02
#define LM90_REG_R_LOCAL_TEMP		0x00
#define LM90_REG_R_LOCAL_HIGH		0x05
#define LM90_REG_W_LOCAL_HIGH		0x0B
#define LM90_REG_R_LOCAL_LOW		0x06
#define LM90_REG_W_LOCAL_LOW		0x0C
#define LM90_REG_R_LOCAL_CRIT		0x20
#define LM90_REG_W_LOCAL_CRIT		0x20
#define LM90_REG_R_REMOTE_TEMPH		0x01
#define LM90_REG_R_REMOTE_TEMPL		0x10
#define LM90_REG_R_REMOTE_OFFSH		0x11
#define LM90_REG_W_REMOTE_OFFSH		0x11
#define LM90_REG_R_REMOTE_OFFSL		0x12
#define LM90_REG_W_REMOTE_OFFSL		0x12
#define LM90_REG_R_REMOTE_HIGHH		0x07
#define LM90_REG_W_REMOTE_HIGHH		0x0D
#define LM90_REG_R_REMOTE_HIGHL		0x13
#define LM90_REG_W_REMOTE_HIGHL		0x13
#define LM90_REG_R_REMOTE_LOWH		0x08
#define LM90_REG_W_REMOTE_LOWH		0x0E
#define LM90_REG_R_REMOTE_LOWL		0x14
#define LM90_REG_W_REMOTE_LOWL		0x14
#define LM90_REG_R_REMOTE_CRIT		0x19
#define LM90_REG_W_REMOTE_CRIT		0x19
#define LM90_REG_R_TCRIT_HYST		0x21
#define LM90_REG_W_TCRIT_HYST		0x21



#define MAX6657_REG_R_LOCAL_TEMPL	0x11


#define LM90_FLAG_ADT7461_EXT		0x01	



static int lm90_detect(struct i2c_client *client, int kind,
		       struct i2c_board_info *info);
static int lm90_probe(struct i2c_client *client,
		      const struct i2c_device_id *id);
static void lm90_init_client(struct i2c_client *client);
static int lm90_remove(struct i2c_client *client);
static struct lm90_data *lm90_update_device(struct device *dev);



static const struct i2c_device_id lm90_id[] = {
	{ "adm1032", adm1032 },
	{ "adt7461", adt7461 },
	{ "lm90", lm90 },
	{ "lm86", lm86 },
	{ "lm89", lm86 },
	{ "lm99", lm99 },
	{ "max6646", max6646 },
	{ "max6647", max6646 },
	{ "max6649", max6646 },
	{ "max6657", max6657 },
	{ "max6658", max6657 },
	{ "max6659", max6657 },
	{ "max6680", max6680 },
	{ "max6681", max6680 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, lm90_id);

static struct i2c_driver lm90_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "lm90",
	},
	.probe		= lm90_probe,
	.remove		= lm90_remove,
	.id_table	= lm90_id,
	.detect		= lm90_detect,
	.address_data	= &addr_data,
};



struct lm90_data {
	struct device *hwmon_dev;
	struct mutex update_lock;
	char valid; 
	unsigned long last_updated; 
	int kind;
	int flags;

	
	s8 temp8[4];	
	s16 temp11[5];	
	u8 temp_hyst;
	u8 alarms; 
};



static inline int temp_from_s8(s8 val)
{
	return val * 1000;
}

static inline int temp_from_u8(u8 val)
{
	return val * 1000;
}

static inline int temp_from_s16(s16 val)
{
	return val / 32 * 125;
}

static inline int temp_from_u16(u16 val)
{
	return val / 32 * 125;
}

static s8 temp_to_s8(long val)
{
	if (val <= -128000)
		return -128;
	if (val >= 127000)
		return 127;
	if (val < 0)
		return (val - 500) / 1000;
	return (val + 500) / 1000;
}

static u8 temp_to_u8(long val)
{
	if (val <= 0)
		return 0;
	if (val >= 255000)
		return 255;
	return (val + 500) / 1000;
}

static s16 temp_to_s16(long val)
{
	if (val <= -128000)
		return 0x8000;
	if (val >= 127875)
		return 0x7FE0;
	if (val < 0)
		return (val - 62) / 125 * 32;
	return (val + 62) / 125 * 32;
}

static u8 hyst_to_reg(long val)
{
	if (val <= 0)
		return 0;
	if (val >= 30500)
		return 31;
	return (val + 500) / 1000;
}


static inline int temp_from_u8_adt7461(struct lm90_data *data, u8 val)
{
	if (data->flags & LM90_FLAG_ADT7461_EXT)
		return (val - 64) * 1000;
	else
		return temp_from_s8(val);
}

static inline int temp_from_u16_adt7461(struct lm90_data *data, u16 val)
{
	if (data->flags & LM90_FLAG_ADT7461_EXT)
		return (val - 0x4000) / 64 * 250;
	else
		return temp_from_s16(val);
}

static u8 temp_to_u8_adt7461(struct lm90_data *data, long val)
{
	if (data->flags & LM90_FLAG_ADT7461_EXT) {
		if (val <= -64000)
			return 0;
		if (val >= 191000)
			return 0xFF;
		return (val + 500 + 64000) / 1000;
	} else {
		if (val <= 0)
			return 0;
		if (val >= 127000)
			return 127;
		return (val + 500) / 1000;
	}
}

static u16 temp_to_u16_adt7461(struct lm90_data *data, long val)
{
	if (data->flags & LM90_FLAG_ADT7461_EXT) {
		if (val <= -64000)
			return 0;
		if (val >= 191750)
			return 0xFFC0;
		return (val + 64000 + 125) / 250 * 64;
	} else {
		if (val <= 0)
			return 0;
		if (val >= 127750)
			return 0x7FC0;
		return (val + 125) / 250 * 64;
	}
}



static ssize_t show_temp8(struct device *dev, struct device_attribute *devattr,
			  char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct lm90_data *data = lm90_update_device(dev);
	int temp;

	if (data->kind == adt7461)
		temp = temp_from_u8_adt7461(data, data->temp8[attr->index]);
	else if (data->kind == max6646)
		temp = temp_from_u8(data->temp8[attr->index]);
	else
		temp = temp_from_s8(data->temp8[attr->index]);

	
	if (data->kind == lm99 && attr->index == 3)
		temp += 16000;

	return sprintf(buf, "%d\n", temp);
}

static ssize_t set_temp8(struct device *dev, struct device_attribute *devattr,
			 const char *buf, size_t count)
{
	static const u8 reg[4] = {
		LM90_REG_W_LOCAL_LOW,
		LM90_REG_W_LOCAL_HIGH,
		LM90_REG_W_LOCAL_CRIT,
		LM90_REG_W_REMOTE_CRIT,
	};

	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct lm90_data *data = i2c_get_clientdata(client);
	long val = simple_strtol(buf, NULL, 10);
	int nr = attr->index;

	
	if (data->kind == lm99 && attr->index == 3)
		val -= 16000;

	mutex_lock(&data->update_lock);
	if (data->kind == adt7461)
		data->temp8[nr] = temp_to_u8_adt7461(data, val);
	else if (data->kind == max6646)
		data->temp8[nr] = temp_to_u8(val);
	else
		data->temp8[nr] = temp_to_s8(val);
	i2c_smbus_write_byte_data(client, reg[nr], data->temp8[nr]);
	mutex_unlock(&data->update_lock);
	return count;
}

static ssize_t show_temp11(struct device *dev, struct device_attribute *devattr,
			   char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct lm90_data *data = lm90_update_device(dev);
	int temp;

	if (data->kind == adt7461)
		temp = temp_from_u16_adt7461(data, data->temp11[attr->index]);
	else if (data->kind == max6646)
		temp = temp_from_u16(data->temp11[attr->index]);
	else
		temp = temp_from_s16(data->temp11[attr->index]);

	
	if (data->kind == lm99 &&  attr->index <= 2)
		temp += 16000;

	return sprintf(buf, "%d\n", temp);
}

static ssize_t set_temp11(struct device *dev, struct device_attribute *devattr,
			  const char *buf, size_t count)
{
	static const u8 reg[6] = {
		LM90_REG_W_REMOTE_LOWH,
		LM90_REG_W_REMOTE_LOWL,
		LM90_REG_W_REMOTE_HIGHH,
		LM90_REG_W_REMOTE_HIGHL,
		LM90_REG_W_REMOTE_OFFSH,
		LM90_REG_W_REMOTE_OFFSL,
	};

	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct lm90_data *data = i2c_get_clientdata(client);
	long val = simple_strtol(buf, NULL, 10);
	int nr = attr->index;

	
	if (data->kind == lm99 && attr->index <= 2)
		val -= 16000;

	mutex_lock(&data->update_lock);
	if (data->kind == adt7461)
		data->temp11[nr] = temp_to_u16_adt7461(data, val);
	else if (data->kind == max6657 || data->kind == max6680)
		data->temp11[nr] = temp_to_s8(val) << 8;
	else if (data->kind == max6646)
		data->temp11[nr] = temp_to_u8(val) << 8;
	else
		data->temp11[nr] = temp_to_s16(val);

	i2c_smbus_write_byte_data(client, reg[(nr - 1) * 2],
				  data->temp11[nr] >> 8);
	if (data->kind != max6657 && data->kind != max6680
	    && data->kind != max6646)
		i2c_smbus_write_byte_data(client, reg[(nr - 1) * 2 + 1],
					  data->temp11[nr] & 0xff);
	mutex_unlock(&data->update_lock);
	return count;
}

static ssize_t show_temphyst(struct device *dev, struct device_attribute *devattr,
			     char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct lm90_data *data = lm90_update_device(dev);
	int temp;

	if (data->kind == adt7461)
		temp = temp_from_u8_adt7461(data, data->temp8[attr->index]);
	else if (data->kind == max6646)
		temp = temp_from_u8(data->temp8[attr->index]);
	else
		temp = temp_from_s8(data->temp8[attr->index]);

	
	if (data->kind == lm99 && attr->index == 3)
		temp += 16000;

	return sprintf(buf, "%d\n", temp - temp_from_s8(data->temp_hyst));
}

static ssize_t set_temphyst(struct device *dev, struct device_attribute *dummy,
			    const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct lm90_data *data = i2c_get_clientdata(client);
	long val = simple_strtol(buf, NULL, 10);
	int temp;

	mutex_lock(&data->update_lock);
	if (data->kind == adt7461)
		temp = temp_from_u8_adt7461(data, data->temp8[2]);
	else if (data->kind == max6646)
		temp = temp_from_u8(data->temp8[2]);
	else
		temp = temp_from_s8(data->temp8[2]);

	data->temp_hyst = hyst_to_reg(temp - val);
	i2c_smbus_write_byte_data(client, LM90_REG_W_TCRIT_HYST,
				  data->temp_hyst);
	mutex_unlock(&data->update_lock);
	return count;
}

static ssize_t show_alarms(struct device *dev, struct device_attribute *dummy,
			   char *buf)
{
	struct lm90_data *data = lm90_update_device(dev);
	return sprintf(buf, "%d\n", data->alarms);
}

static ssize_t show_alarm(struct device *dev, struct device_attribute
			  *devattr, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct lm90_data *data = lm90_update_device(dev);
	int bitnr = attr->index;

	return sprintf(buf, "%d\n", (data->alarms >> bitnr) & 1);
}

static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, show_temp11, NULL, 4);
static SENSOR_DEVICE_ATTR(temp2_input, S_IRUGO, show_temp11, NULL, 0);
static SENSOR_DEVICE_ATTR(temp1_min, S_IWUSR | S_IRUGO, show_temp8,
	set_temp8, 0);
static SENSOR_DEVICE_ATTR(temp2_min, S_IWUSR | S_IRUGO, show_temp11,
	set_temp11, 1);
static SENSOR_DEVICE_ATTR(temp1_max, S_IWUSR | S_IRUGO, show_temp8,
	set_temp8, 1);
static SENSOR_DEVICE_ATTR(temp2_max, S_IWUSR | S_IRUGO, show_temp11,
	set_temp11, 2);
static SENSOR_DEVICE_ATTR(temp1_crit, S_IWUSR | S_IRUGO, show_temp8,
	set_temp8, 2);
static SENSOR_DEVICE_ATTR(temp2_crit, S_IWUSR | S_IRUGO, show_temp8,
	set_temp8, 3);
static SENSOR_DEVICE_ATTR(temp1_crit_hyst, S_IWUSR | S_IRUGO, show_temphyst,
	set_temphyst, 2);
static SENSOR_DEVICE_ATTR(temp2_crit_hyst, S_IRUGO, show_temphyst, NULL, 3);
static SENSOR_DEVICE_ATTR(temp2_offset, S_IWUSR | S_IRUGO, show_temp11,
	set_temp11, 3);


static SENSOR_DEVICE_ATTR(temp1_crit_alarm, S_IRUGO, show_alarm, NULL, 0);
static SENSOR_DEVICE_ATTR(temp2_crit_alarm, S_IRUGO, show_alarm, NULL, 1);
static SENSOR_DEVICE_ATTR(temp2_fault, S_IRUGO, show_alarm, NULL, 2);
static SENSOR_DEVICE_ATTR(temp2_min_alarm, S_IRUGO, show_alarm, NULL, 3);
static SENSOR_DEVICE_ATTR(temp2_max_alarm, S_IRUGO, show_alarm, NULL, 4);
static SENSOR_DEVICE_ATTR(temp1_min_alarm, S_IRUGO, show_alarm, NULL, 5);
static SENSOR_DEVICE_ATTR(temp1_max_alarm, S_IRUGO, show_alarm, NULL, 6);

static DEVICE_ATTR(alarms, S_IRUGO, show_alarms, NULL);

static struct attribute *lm90_attributes[] = {
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	&sensor_dev_attr_temp1_min.dev_attr.attr,
	&sensor_dev_attr_temp2_min.dev_attr.attr,
	&sensor_dev_attr_temp1_max.dev_attr.attr,
	&sensor_dev_attr_temp2_max.dev_attr.attr,
	&sensor_dev_attr_temp1_crit.dev_attr.attr,
	&sensor_dev_attr_temp2_crit.dev_attr.attr,
	&sensor_dev_attr_temp1_crit_hyst.dev_attr.attr,
	&sensor_dev_attr_temp2_crit_hyst.dev_attr.attr,

	&sensor_dev_attr_temp1_crit_alarm.dev_attr.attr,
	&sensor_dev_attr_temp2_crit_alarm.dev_attr.attr,
	&sensor_dev_attr_temp2_fault.dev_attr.attr,
	&sensor_dev_attr_temp2_min_alarm.dev_attr.attr,
	&sensor_dev_attr_temp2_max_alarm.dev_attr.attr,
	&sensor_dev_attr_temp1_min_alarm.dev_attr.attr,
	&sensor_dev_attr_temp1_max_alarm.dev_attr.attr,
	&dev_attr_alarms.attr,
	NULL
};

static const struct attribute_group lm90_group = {
	.attrs = lm90_attributes,
};


static ssize_t show_pec(struct device *dev, struct device_attribute *dummy,
			char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	return sprintf(buf, "%d\n", !!(client->flags & I2C_CLIENT_PEC));
}

static ssize_t set_pec(struct device *dev, struct device_attribute *dummy,
		       const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	long val = simple_strtol(buf, NULL, 10);

	switch (val) {
	case 0:
		client->flags &= ~I2C_CLIENT_PEC;
		break;
	case 1:
		client->flags |= I2C_CLIENT_PEC;
		break;
	default:
		return -EINVAL;
	}

	return count;
}

static DEVICE_ATTR(pec, S_IWUSR | S_IRUGO, show_pec, set_pec);




static inline s32 adm1032_write_byte(struct i2c_client *client, u8 value)
{
	return i2c_smbus_xfer(client->adapter, client->addr,
			      client->flags & ~I2C_CLIENT_PEC,
			      I2C_SMBUS_WRITE, value, I2C_SMBUS_BYTE, NULL);
}


static int lm90_read_reg(struct i2c_client* client, u8 reg, u8 *value)
{
	int err;

 	if (client->flags & I2C_CLIENT_PEC) {
 		err = adm1032_write_byte(client, reg);
 		if (err >= 0)
 			err = i2c_smbus_read_byte(client);
 	} else
 		err = i2c_smbus_read_byte_data(client, reg);

	if (err < 0) {
		dev_warn(&client->dev, "Register %#02x read failed (%d)\n",
			 reg, err);
		return err;
	}
	*value = err;

	return 0;
}


static int lm90_detect(struct i2c_client *new_client, int kind,
		       struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = new_client->adapter;
	int address = new_client->addr;
	const char *name = "";

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -ENODEV;

	

	
	if (kind == 0)
		kind = lm90;

	if (kind < 0) { 
		int man_id, chip_id, reg_config1, reg_convrate;

		if ((man_id = i2c_smbus_read_byte_data(new_client,
						LM90_REG_R_MAN_ID)) < 0
		 || (chip_id = i2c_smbus_read_byte_data(new_client,
						LM90_REG_R_CHIP_ID)) < 0
		 || (reg_config1 = i2c_smbus_read_byte_data(new_client,
						LM90_REG_R_CONFIG1)) < 0
		 || (reg_convrate = i2c_smbus_read_byte_data(new_client,
						LM90_REG_R_CONVRATE)) < 0)
			return -ENODEV;
		
		if ((address == 0x4C || address == 0x4D)
		 && man_id == 0x01) { 
			int reg_config2;

			if ((reg_config2 = i2c_smbus_read_byte_data(new_client,
						LM90_REG_R_CONFIG2)) < 0)
				return -ENODEV;

			if ((reg_config1 & 0x2A) == 0x00
			 && (reg_config2 & 0xF8) == 0x00
			 && reg_convrate <= 0x09) {
				if (address == 0x4C
				 && (chip_id & 0xF0) == 0x20) { 
					kind = lm90;
				} else
				if ((chip_id & 0xF0) == 0x30) { 
					kind = lm99;
					dev_info(&adapter->dev,
						 "Assuming LM99 chip at "
						 "0x%02x\n", address);
					dev_info(&adapter->dev,
						 "If it is an LM89, pass "
						 "force_lm86=%d,0x%02x when "
						 "loading the lm90 driver\n",
						 i2c_adapter_id(adapter),
						 address);
				} else
				if (address == 0x4C
				 && (chip_id & 0xF0) == 0x10) { 
					kind = lm86;
				}
			}
		} else
		if ((address == 0x4C || address == 0x4D)
		 && man_id == 0x41) { 
			if ((chip_id & 0xF0) == 0x40 
			 && (reg_config1 & 0x3F) == 0x00
			 && reg_convrate <= 0x0A) {
				kind = adm1032;
			} else
			if (chip_id == 0x51 
			 && (reg_config1 & 0x1B) == 0x00
			 && reg_convrate <= 0x0A) {
				kind = adt7461;
			}
		} else
		if (man_id == 0x4D) { 
			
			if (chip_id == man_id
			 && (address == 0x4C || address == 0x4D)
			 && (reg_config1 & 0x1F) == (man_id & 0x0F)
			 && reg_convrate <= 0x09) {
			 	kind = max6657;
			} else
			
			if (chip_id == 0x01
			 && (reg_config1 & 0x03) == 0x00
			 && reg_convrate <= 0x07) {
			 	kind = max6680;
			} else
			
			if (chip_id == 0x59
			 && (reg_config1 & 0x3f) == 0x00
			 && reg_convrate <= 0x07) {
				kind = max6646;
			}
		}

		if (kind <= 0) { 
			dev_dbg(&adapter->dev,
				"Unsupported chip at 0x%02x (man_id=0x%02X, "
				"chip_id=0x%02X)\n", address, man_id, chip_id);
			return -ENODEV;
		}
	}

	
	if (kind == lm90) {
		name = "lm90";
	} else if (kind == adm1032) {
		name = "adm1032";
		
		if (i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
			info->flags |= I2C_CLIENT_PEC;
	} else if (kind == lm99) {
		name = "lm99";
	} else if (kind == lm86) {
		name = "lm86";
	} else if (kind == max6657) {
		name = "max6657";
	} else if (kind == max6680) {
		name = "max6680";
	} else if (kind == adt7461) {
		name = "adt7461";
	} else if (kind == max6646) {
		name = "max6646";
	}
	strlcpy(info->type, name, I2C_NAME_SIZE);

	return 0;
}

static int lm90_probe(struct i2c_client *new_client,
		      const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(new_client->dev.parent);
	struct lm90_data *data;
	int err;

	data = kzalloc(sizeof(struct lm90_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}
	i2c_set_clientdata(new_client, data);
	mutex_init(&data->update_lock);

	
	data->kind = id->driver_data;
	if (data->kind == adm1032) {
		if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
			new_client->flags &= ~I2C_CLIENT_PEC;
	}

	
	lm90_init_client(new_client);

	
	if ((err = sysfs_create_group(&new_client->dev.kobj, &lm90_group)))
		goto exit_free;
	if (new_client->flags & I2C_CLIENT_PEC) {
		if ((err = device_create_file(&new_client->dev,
					      &dev_attr_pec)))
			goto exit_remove_files;
	}
	if (data->kind != max6657 && data->kind != max6646) {
		if ((err = device_create_file(&new_client->dev,
				&sensor_dev_attr_temp2_offset.dev_attr)))
			goto exit_remove_files;
	}

	data->hwmon_dev = hwmon_device_register(&new_client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		err = PTR_ERR(data->hwmon_dev);
		goto exit_remove_files;
	}

	return 0;

exit_remove_files:
	sysfs_remove_group(&new_client->dev.kobj, &lm90_group);
	device_remove_file(&new_client->dev, &dev_attr_pec);
exit_free:
	kfree(data);
exit:
	return err;
}

static void lm90_init_client(struct i2c_client *client)
{
	u8 config, config_orig;
	struct lm90_data *data = i2c_get_clientdata(client);

	
	i2c_smbus_write_byte_data(client, LM90_REG_W_CONVRATE,
				  5); 
	if (lm90_read_reg(client, LM90_REG_R_CONFIG1, &config) < 0) {
		dev_warn(&client->dev, "Initialization failed!\n");
		return;
	}
	config_orig = config;

	
	if (data->kind == adt7461) {
		if (config & 0x04)
			data->flags |= LM90_FLAG_ADT7461_EXT;
	}

	
	if (data->kind == max6680) {
		config |= 0x18;
	}

	config &= 0xBF;	
	if (config != config_orig) 
		i2c_smbus_write_byte_data(client, LM90_REG_W_CONFIG1, config);
}

static int lm90_remove(struct i2c_client *client)
{
	struct lm90_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &lm90_group);
	device_remove_file(&client->dev, &dev_attr_pec);
	if (data->kind != max6657 && data->kind != max6646)
		device_remove_file(&client->dev,
				   &sensor_dev_attr_temp2_offset.dev_attr);

	kfree(data);
	return 0;
}

static int lm90_read16(struct i2c_client *client, u8 regh, u8 regl, u16 *value)
{
	int err;
	u8 oldh, newh, l;

	
	if ((err = lm90_read_reg(client, regh, &oldh))
	 || (err = lm90_read_reg(client, regl, &l))
	 || (err = lm90_read_reg(client, regh, &newh)))
		return err;
	if (oldh != newh) {
		err = lm90_read_reg(client, regl, &l);
		if (err)
			return err;
	}
	*value = (newh << 8) | l;

	return 0;
}

static struct lm90_data *lm90_update_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct lm90_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);

	if (time_after(jiffies, data->last_updated + HZ * 2) || !data->valid) {
		u8 h, l;

		dev_dbg(&client->dev, "Updating lm90 data.\n");
		lm90_read_reg(client, LM90_REG_R_LOCAL_LOW, &data->temp8[0]);
		lm90_read_reg(client, LM90_REG_R_LOCAL_HIGH, &data->temp8[1]);
		lm90_read_reg(client, LM90_REG_R_LOCAL_CRIT, &data->temp8[2]);
		lm90_read_reg(client, LM90_REG_R_REMOTE_CRIT, &data->temp8[3]);
		lm90_read_reg(client, LM90_REG_R_TCRIT_HYST, &data->temp_hyst);

		if (data->kind == max6657 || data->kind == max6646) {
			lm90_read16(client, LM90_REG_R_LOCAL_TEMP,
				    MAX6657_REG_R_LOCAL_TEMPL,
				    &data->temp11[4]);
		} else {
			if (lm90_read_reg(client, LM90_REG_R_LOCAL_TEMP,
					  &h) == 0)
				data->temp11[4] = h << 8;
		}
		lm90_read16(client, LM90_REG_R_REMOTE_TEMPH,
			    LM90_REG_R_REMOTE_TEMPL, &data->temp11[0]);

		if (lm90_read_reg(client, LM90_REG_R_REMOTE_LOWH, &h) == 0) {
			data->temp11[1] = h << 8;
			if (data->kind != max6657 && data->kind != max6680
			 && data->kind != max6646
			 && lm90_read_reg(client, LM90_REG_R_REMOTE_LOWL,
					  &l) == 0)
				data->temp11[1] |= l;
		}
		if (lm90_read_reg(client, LM90_REG_R_REMOTE_HIGHH, &h) == 0) {
			data->temp11[2] = h << 8;
			if (data->kind != max6657 && data->kind != max6680
			 && data->kind != max6646
			 && lm90_read_reg(client, LM90_REG_R_REMOTE_HIGHL,
					  &l) == 0)
				data->temp11[2] |= l;
		}

		if (data->kind != max6657 && data->kind != max6646) {
			if (lm90_read_reg(client, LM90_REG_R_REMOTE_OFFSH,
					  &h) == 0
			 && lm90_read_reg(client, LM90_REG_R_REMOTE_OFFSL,
					  &l) == 0)
				data->temp11[3] = (h << 8) | l;
		}
		lm90_read_reg(client, LM90_REG_R_STATUS, &data->alarms);

		data->last_updated = jiffies;
		data->valid = 1;
	}

	mutex_unlock(&data->update_lock);

	return data;
}

static int __init sensors_lm90_init(void)
{
	return i2c_add_driver(&lm90_driver);
}

static void __exit sensors_lm90_exit(void)
{
	i2c_del_driver(&lm90_driver);
}

MODULE_AUTHOR("Jean Delvare <khali@linux-fr.org>");
MODULE_DESCRIPTION("LM90/ADM1032 driver");
MODULE_LICENSE("GPL");

module_init(sensors_lm90_init);
module_exit(sensors_lm90_exit);
