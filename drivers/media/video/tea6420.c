 


#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/i2c.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-i2c-drv.h>
#include "tea6420.h"

MODULE_AUTHOR("Michael Hunold <michael@mihu.de>");
MODULE_DESCRIPTION("tea6420 driver");
MODULE_LICENSE("GPL");

static int debug;
module_param(debug, int, 0644);

MODULE_PARM_DESC(debug, "Debug level (0-1)");



static int tea6420_s_routing(struct v4l2_subdev *sd,
			     u32 i, u32 o, u32 config)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int g = (o >> 4) & 0xf;
	u8 byte;
	int ret;

	o &= 0xf;
	v4l2_dbg(1, debug, sd, "i=%d, o=%d, g=%d\n", i, o, g);

	
	if (i < 1 || i > 6 || o < 1 || o > 4 || g < 0 || g > 6 || g % 2 != 0)
		return -EINVAL;

	byte = ((o - 1) << 5);
	byte |= (i - 1);

	
	switch (g) {
	case 0:
		byte |= (3 << 3);
		break;
	case 2:
		byte |= (2 << 3);
		break;
	case 4:
		byte |= (1 << 3);
		break;
	case 6:
		break;
	}

	ret = i2c_smbus_write_byte(client, byte);
	if (ret) {
		v4l2_dbg(1, debug, sd,
			"i2c_smbus_write_byte() failed, ret:%d\n", ret);
		return -EIO;
	}
	return 0;
}

static int tea6420_g_chip_ident(struct v4l2_subdev *sd, struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_TEA6420, 0);
}



static const struct v4l2_subdev_core_ops tea6420_core_ops = {
	.g_chip_ident = tea6420_g_chip_ident,
};

static const struct v4l2_subdev_audio_ops tea6420_audio_ops = {
	.s_routing = tea6420_s_routing,
};

static const struct v4l2_subdev_ops tea6420_ops = {
	.core = &tea6420_core_ops,
	.audio = &tea6420_audio_ops,
};

static int tea6420_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	struct v4l2_subdev *sd;
	int err, i;

	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_WRITE_BYTE))
		return -EIO;

	v4l_info(client, "chip found @ 0x%x (%s)\n",
			client->addr << 1, client->adapter->name);

	sd = kmalloc(sizeof(struct v4l2_subdev), GFP_KERNEL);
	if (sd == NULL)
		return -ENOMEM;
	v4l2_i2c_subdev_init(sd, client, &tea6420_ops);

	
	err = 0;
	for (i = 1; i < 5; i++)
		err += tea6420_s_routing(sd, 6, i, 0);
	if (err) {
		v4l_dbg(1, debug, client, "could not initialize tea6420\n");
		return -ENODEV;
	}
	return 0;
}

static int tea6420_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	v4l2_device_unregister_subdev(sd);
	kfree(sd);
	return 0;
}

static const struct i2c_device_id tea6420_id[] = {
	{ "tea6420", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, tea6420_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = "tea6420",
	.probe = tea6420_probe,
	.remove = tea6420_remove,
	.id_table = tea6420_id,
};
