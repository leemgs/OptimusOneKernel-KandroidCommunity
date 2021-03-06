

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/gpio.h>

#include <linux/i2c.h>
#include <linux/i2c/max732x.h>




#define PORT_NONE	0x0	
#define PORT_OUTPUT	0x1	
#define PORT_INPUT	0x2	
#define PORT_OPENDRAIN	0x3	

#define IO_4I4O		0x5AA5	
#define IO_4P4O		0x5FF5	
#define IO_8I		0xAAAA	
#define IO_8P		0xFFFF	
#define IO_8O		0x5555	

#define GROUP_A(x)	((x) & 0xffff)	
#define GROUP_B(x)	((x) << 16)	

static const struct i2c_device_id max732x_id[] = {
	{ "max7319", GROUP_A(IO_8I) },
	{ "max7320", GROUP_B(IO_8O) },
	{ "max7321", GROUP_A(IO_8P) },
	{ "max7322", GROUP_A(IO_4I4O) },
	{ "max7323", GROUP_A(IO_4P4O) },
	{ "max7324", GROUP_A(IO_8I) | GROUP_B(IO_8O) },
	{ "max7325", GROUP_A(IO_8P) | GROUP_B(IO_8O) },
	{ "max7326", GROUP_A(IO_4I4O) | GROUP_B(IO_8O) },
	{ "max7327", GROUP_A(IO_4P4O) | GROUP_B(IO_8O) },
	{ },
};
MODULE_DEVICE_TABLE(i2c, max732x_id);

struct max732x_chip {
	struct gpio_chip gpio_chip;

	struct i2c_client *client;	
	struct i2c_client *client_dummy;
	struct i2c_client *client_group_a;
	struct i2c_client *client_group_b;

	unsigned int	mask_group_a;
	unsigned int	dir_input;
	unsigned int	dir_output;

	struct mutex	lock;
	uint8_t		reg_out[2];
};

static int max732x_write(struct max732x_chip *chip, int group_a, uint8_t val)
{
	struct i2c_client *client;
	int ret;

	client = group_a ? chip->client_group_a : chip->client_group_b;
	ret = i2c_smbus_write_byte(client, val);
	if (ret < 0) {
		dev_err(&client->dev, "failed writing\n");
		return ret;
	}

	return 0;
}

static int max732x_read(struct max732x_chip *chip, int group_a, uint8_t *val)
{
	struct i2c_client *client;
	int ret;

	client = group_a ? chip->client_group_a : chip->client_group_b;
	ret = i2c_smbus_read_byte(client);
	if (ret < 0) {
		dev_err(&client->dev, "failed reading\n");
		return ret;
	}

	*val = (uint8_t)ret;
	return 0;
}

static inline int is_group_a(struct max732x_chip *chip, unsigned off)
{
	return (1u << off) & chip->mask_group_a;
}

static int max732x_gpio_get_value(struct gpio_chip *gc, unsigned off)
{
	struct max732x_chip *chip;
	uint8_t reg_val;
	int ret;

	chip = container_of(gc, struct max732x_chip, gpio_chip);

	ret = max732x_read(chip, is_group_a(chip, off), &reg_val);
	if (ret < 0)
		return 0;

	return reg_val & (1u << (off & 0x7));
}

static void max732x_gpio_set_value(struct gpio_chip *gc, unsigned off, int val)
{
	struct max732x_chip *chip;
	uint8_t reg_out, mask = 1u << (off & 0x7);
	int ret;

	chip = container_of(gc, struct max732x_chip, gpio_chip);

	mutex_lock(&chip->lock);

	reg_out = (off > 7) ? chip->reg_out[1] : chip->reg_out[0];
	reg_out = (val) ? reg_out | mask : reg_out & ~mask;

	ret = max732x_write(chip, is_group_a(chip, off), reg_out);
	if (ret < 0)
		goto out;

	
	if (off > 7)
		chip->reg_out[1] = reg_out;
	else
		chip->reg_out[0] = reg_out;
out:
	mutex_unlock(&chip->lock);
}

static int max732x_gpio_direction_input(struct gpio_chip *gc, unsigned off)
{
	struct max732x_chip *chip;
	unsigned int mask = 1u << off;

	chip = container_of(gc, struct max732x_chip, gpio_chip);

	if ((mask & chip->dir_input) == 0) {
		dev_dbg(&chip->client->dev, "%s port %d is output only\n",
			chip->client->name, off);
		return -EACCES;
	}

	return 0;
}

static int max732x_gpio_direction_output(struct gpio_chip *gc,
		unsigned off, int val)
{
	struct max732x_chip *chip;
	unsigned int mask = 1u << off;

	chip = container_of(gc, struct max732x_chip, gpio_chip);

	if ((mask & chip->dir_output) == 0) {
		dev_dbg(&chip->client->dev, "%s port %d is input only\n",
			chip->client->name, off);
		return -EACCES;
	}

	max732x_gpio_set_value(gc, off, val);
	return 0;
}

static int __devinit max732x_setup_gpio(struct max732x_chip *chip,
					const struct i2c_device_id *id,
					unsigned gpio_start)
{
	struct gpio_chip *gc = &chip->gpio_chip;
	uint32_t id_data = id->driver_data;
	int i, port = 0;

	for (i = 0; i < 16; i++, id_data >>= 2) {
		unsigned int mask = 1 << port;

		switch (id_data & 0x3) {
		case PORT_OUTPUT:
			chip->dir_output |= mask;
			break;
		case PORT_INPUT:
			chip->dir_input |= mask;
			break;
		case PORT_OPENDRAIN:
			chip->dir_output |= mask;
			chip->dir_input |= mask;
			break;
		default:
			continue;
		}

		if (i < 8)
			chip->mask_group_a |= mask;
		port++;
	}

	if (chip->dir_input)
		gc->direction_input = max732x_gpio_direction_input;
	if (chip->dir_output) {
		gc->direction_output = max732x_gpio_direction_output;
		gc->set = max732x_gpio_set_value;
	}
	gc->get = max732x_gpio_get_value;
	gc->can_sleep = 1;

	gc->base = gpio_start;
	gc->ngpio = port;
	gc->label = chip->client->name;
	gc->owner = THIS_MODULE;

	return port;
}

static int __devinit max732x_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct max732x_platform_data *pdata;
	struct max732x_chip *chip;
	struct i2c_client *c;
	uint16_t addr_a, addr_b;
	int ret, nr_port;

	pdata = client->dev.platform_data;
	if (pdata == NULL) {
		dev_dbg(&client->dev, "no platform data\n");
		return -EINVAL;
	}

	chip = kzalloc(sizeof(struct max732x_chip), GFP_KERNEL);
	if (chip == NULL)
		return -ENOMEM;
	chip->client = client;

	nr_port = max732x_setup_gpio(chip, id, pdata->gpio_base);

	addr_a = (client->addr & 0x0f) | 0x60;
	addr_b = (client->addr & 0x0f) | 0x50;

	switch (client->addr & 0x70) {
	case 0x60:
		chip->client_group_a = client;
		if (nr_port > 7) {
			c = i2c_new_dummy(client->adapter, addr_b);
			chip->client_group_b = chip->client_dummy = c;
		}
		break;
	case 0x50:
		chip->client_group_b = client;
		if (nr_port > 7) {
			c = i2c_new_dummy(client->adapter, addr_a);
			chip->client_group_a = chip->client_dummy = c;
		}
		break;
	default:
		dev_err(&client->dev, "invalid I2C address specified %02x\n",
				client->addr);
		ret = -EINVAL;
		goto out_failed;
	}

	mutex_init(&chip->lock);

	max732x_read(chip, is_group_a(chip, 0), &chip->reg_out[0]);
	if (nr_port > 7)
		max732x_read(chip, is_group_a(chip, 8), &chip->reg_out[1]);

	ret = gpiochip_add(&chip->gpio_chip);
	if (ret)
		goto out_failed;

	if (pdata->setup) {
		ret = pdata->setup(client, chip->gpio_chip.base,
				chip->gpio_chip.ngpio, pdata->context);
		if (ret < 0)
			dev_warn(&client->dev, "setup failed, %d\n", ret);
	}

	i2c_set_clientdata(client, chip);
	return 0;

out_failed:
	kfree(chip);
	return ret;
}

static int __devexit max732x_remove(struct i2c_client *client)
{
	struct max732x_platform_data *pdata = client->dev.platform_data;
	struct max732x_chip *chip = i2c_get_clientdata(client);
	int ret;

	if (pdata->teardown) {
		ret = pdata->teardown(client, chip->gpio_chip.base,
				chip->gpio_chip.ngpio, pdata->context);
		if (ret < 0) {
			dev_err(&client->dev, "%s failed, %d\n",
					"teardown", ret);
			return ret;
		}
	}

	ret = gpiochip_remove(&chip->gpio_chip);
	if (ret) {
		dev_err(&client->dev, "%s failed, %d\n",
				"gpiochip_remove()", ret);
		return ret;
	}

	
	if (chip->client_dummy)
		i2c_unregister_device(chip->client_dummy);

	kfree(chip);
	return 0;
}

static struct i2c_driver max732x_driver = {
	.driver = {
		.name	= "max732x",
		.owner	= THIS_MODULE,
	},
	.probe		= max732x_probe,
	.remove		= __devexit_p(max732x_remove),
	.id_table	= max732x_id,
};

static int __init max732x_init(void)
{
	return i2c_add_driver(&max732x_driver);
}

subsys_initcall(max732x_init);

static void __exit max732x_exit(void)
{
	i2c_del_driver(&max732x_driver);
}
module_exit(max732x_exit);

MODULE_AUTHOR("Eric Miao <eric.miao@marvell.com>");
MODULE_DESCRIPTION("GPIO expander driver for MAX732X");
MODULE_LICENSE("GPL");
