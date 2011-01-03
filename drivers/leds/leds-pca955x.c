

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/workqueue.h>


#define PCA955X_LS_LED_ON	0x0	
#define PCA955X_LS_LED_OFF	0x1	
#define PCA955X_LS_BLINK0	0x2	
#define PCA955X_LS_BLINK1	0x3	

enum pca955x_type {
	pca9550,
	pca9551,
	pca9552,
	pca9553,
};

struct pca955x_chipdef {
	int			bits;
	u8			slv_addr;	
	int			slv_addr_shift;	
};

static struct pca955x_chipdef pca955x_chipdefs[] = {
	[pca9550] = {
		.bits		= 2,
		.slv_addr	=  0x60,
		.slv_addr_shift	= 1,
	},
	[pca9551] = {
		.bits		= 8,
		.slv_addr	=  0x60,
		.slv_addr_shift	= 3,
	},
	[pca9552] = {
		.bits		= 16,
		.slv_addr	=  0x60,
		.slv_addr_shift	= 3,
	},
	[pca9553] = {
		.bits		= 4,
		.slv_addr	=  0x62,
		.slv_addr_shift	= 1,
	},
};

static const struct i2c_device_id pca955x_id[] = {
	{ "pca9550", pca9550 },
	{ "pca9551", pca9551 },
	{ "pca9552", pca9552 },
	{ "pca9553", pca9553 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, pca955x_id);

struct pca955x_led {
	struct pca955x_chipdef	*chipdef;
	struct i2c_client	*client;
	struct work_struct	work;
	spinlock_t		lock;
	enum led_brightness	brightness;
	struct led_classdev	led_cdev;
	int			led_num;	
	char			name[32];
};


static inline int pca95xx_num_input_regs(int bits)
{
	return (bits + 7) / 8;
}


static inline int pca95xx_num_led_regs(int bits)
{
	return (bits + 3)  / 4;
}


static inline u8 pca955x_ledsel(u8 oldval, int led_num, int state)
{
	return (oldval & (~(0x3 << (led_num << 1)))) |
		((state & 0x3) << (led_num << 1));
}


static void pca955x_write_psc(struct i2c_client *client, int n, u8 val)
{
	struct pca955x_led *pca955x = i2c_get_clientdata(client);

	i2c_smbus_write_byte_data(client,
		pca95xx_num_input_regs(pca955x->chipdef->bits) + 2*n,
		val);
}


static void pca955x_write_pwm(struct i2c_client *client, int n, u8 val)
{
	struct pca955x_led *pca955x = i2c_get_clientdata(client);

	i2c_smbus_write_byte_data(client,
		pca95xx_num_input_regs(pca955x->chipdef->bits) + 1 + 2*n,
		val);
}


static void pca955x_write_ls(struct i2c_client *client, int n, u8 val)
{
	struct pca955x_led *pca955x = i2c_get_clientdata(client);

	i2c_smbus_write_byte_data(client,
		pca95xx_num_input_regs(pca955x->chipdef->bits) + 4 + n,
		val);
}


static u8 pca955x_read_ls(struct i2c_client *client, int n)
{
	struct pca955x_led *pca955x = i2c_get_clientdata(client);

	return (u8) i2c_smbus_read_byte_data(client,
		pca95xx_num_input_regs(pca955x->chipdef->bits) + 4 + n);
}

static void pca955x_led_work(struct work_struct *work)
{
	struct pca955x_led *pca955x;
	u8 ls;
	int chip_ls;	
	int ls_led;	

	pca955x = container_of(work, struct pca955x_led, work);
	chip_ls = pca955x->led_num / 4;
	ls_led = pca955x->led_num % 4;

	ls = pca955x_read_ls(pca955x->client, chip_ls);

	switch (pca955x->brightness) {
	case LED_FULL:
		ls = pca955x_ledsel(ls, ls_led, PCA955X_LS_LED_ON);
		break;
	case LED_OFF:
		ls = pca955x_ledsel(ls, ls_led, PCA955X_LS_LED_OFF);
		break;
	case LED_HALF:
		ls = pca955x_ledsel(ls, ls_led, PCA955X_LS_BLINK0);
		break;
	default:
		
		pca955x_write_pwm(pca955x->client, 1, 255-pca955x->brightness);
		ls = pca955x_ledsel(ls, ls_led, PCA955X_LS_BLINK1);
		break;
	}

	pca955x_write_ls(pca955x->client, chip_ls, ls);
}

static void pca955x_led_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct pca955x_led *pca955x;

	pca955x = container_of(led_cdev, struct pca955x_led, led_cdev);

	spin_lock(&pca955x->lock);
	pca955x->brightness = value;

	
	schedule_work(&pca955x->work);

	spin_unlock(&pca955x->lock);
}

static int __devinit pca955x_probe(struct i2c_client *client,
					const struct i2c_device_id *id)
{
	struct pca955x_led *pca955x;
	struct pca955x_chipdef *chip;
	struct i2c_adapter *adapter;
	struct led_platform_data *pdata;
	int i, err;

	chip = &pca955x_chipdefs[id->driver_data];
	adapter = to_i2c_adapter(client->dev.parent);
	pdata = client->dev.platform_data;

	
	if ((client->addr & ~((1 << chip->slv_addr_shift) - 1)) !=
	    chip->slv_addr) {
		dev_err(&client->dev, "invalid slave address %02x\n",
				client->addr);
		return -ENODEV;
	}

	printk(KERN_INFO "leds-pca955x: Using %s %d-bit LED driver at "
			"slave address 0x%02x\n",
			id->name, chip->bits, client->addr);

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -EIO;

	if (pdata) {
		if (pdata->num_leds != chip->bits) {
			dev_err(&client->dev, "board info claims %d LEDs"
					" on a %d-bit chip\n",
					pdata->num_leds, chip->bits);
			return -ENODEV;
		}
	}

	pca955x = kzalloc(sizeof(*pca955x) * chip->bits, GFP_KERNEL);
	if (!pca955x)
		return -ENOMEM;

	i2c_set_clientdata(client, pca955x);

	for (i = 0; i < chip->bits; i++) {
		pca955x[i].chipdef = chip;
		pca955x[i].client = client;
		pca955x[i].led_num = i;

		
		if (pdata) {
			if (pdata->leds[i].name)
				snprintf(pca955x[i].name,
					 sizeof(pca955x[i].name), "pca955x:%s",
					 pdata->leds[i].name);
			if (pdata->leds[i].default_trigger)
				pca955x[i].led_cdev.default_trigger =
					pdata->leds[i].default_trigger;
		} else {
			snprintf(pca955x[i].name, sizeof(pca955x[i].name),
				 "pca955x:%d", i);
		}

		spin_lock_init(&pca955x[i].lock);

		pca955x[i].led_cdev.name = pca955x[i].name;
		pca955x[i].led_cdev.brightness_set = pca955x_led_set;

		INIT_WORK(&pca955x[i].work, pca955x_led_work);

		err = led_classdev_register(&client->dev, &pca955x[i].led_cdev);
		if (err < 0)
			goto exit;
	}

	
	for (i = 0; i < pca95xx_num_led_regs(chip->bits); i++)
		pca955x_write_ls(client, i, 0x55);

	
	pca955x_write_pwm(client, 0, 255-LED_HALF);

	
	pca955x_write_pwm(client, 1, 0);

	
	pca955x_write_psc(client, 0, 0);
	pca955x_write_psc(client, 1, 0);

	return 0;

exit:
	while (i--) {
		led_classdev_unregister(&pca955x[i].led_cdev);
		cancel_work_sync(&pca955x[i].work);
	}

	kfree(pca955x);
	i2c_set_clientdata(client, NULL);

	return err;
}

static int __devexit pca955x_remove(struct i2c_client *client)
{
	struct pca955x_led *pca955x = i2c_get_clientdata(client);
	int i;

	for (i = 0; i < pca955x->chipdef->bits; i++) {
		led_classdev_unregister(&pca955x[i].led_cdev);
		cancel_work_sync(&pca955x[i].work);
	}

	kfree(pca955x);
	i2c_set_clientdata(client, NULL);

	return 0;
}

static struct i2c_driver pca955x_driver = {
	.driver = {
		.name	= "leds-pca955x",
		.owner	= THIS_MODULE,
	},
	.probe	= pca955x_probe,
	.remove	= __devexit_p(pca955x_remove),
	.id_table = pca955x_id,
};

static int __init pca955x_leds_init(void)
{
	return i2c_add_driver(&pca955x_driver);
}

static void __exit pca955x_leds_exit(void)
{
	i2c_del_driver(&pca955x_driver);
}

module_init(pca955x_leds_init);
module_exit(pca955x_leds_exit);

MODULE_AUTHOR("Nate Case <ncase@xes-inc.com>");
MODULE_DESCRIPTION("PCA955x LED driver");
MODULE_LICENSE("GPL v2");
