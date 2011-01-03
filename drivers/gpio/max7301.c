

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/spi/spi.h>
#include <linux/spi/max7301.h>
#include <linux/gpio.h>

#define DRIVER_NAME "max7301"


#define PIN_CONFIG_MASK 0x03
#define PIN_CONFIG_IN_PULLUP 0x03
#define PIN_CONFIG_IN_WO_PULLUP 0x02
#define PIN_CONFIG_OUT 0x01

#define PIN_NUMBER 28



struct max7301 {
	struct mutex	lock;
	u8		port_config[8];	
	u32		out_level;	
	struct gpio_chip chip;
	struct spi_device *spi;
};


static int max7301_write(struct spi_device *spi, unsigned int reg, unsigned int val)
{
	u16 word = ((reg & 0x7F) << 8) | (val & 0xFF);
	return spi_write(spi, (const u8 *)&word, sizeof(word));
}


static int max7301_read(struct spi_device *spi, unsigned int reg)
{
	int ret;
	u16 word;

	word = 0x8000 | (reg << 8);
	ret = spi_write(spi, (const u8 *)&word, sizeof(word));
	if (ret)
		return ret;
	
	ret = spi_read(spi, (u8 *)&word, sizeof(word));
	if (ret)
		return ret;
	return word & 0xff;
}

static int max7301_direction_input(struct gpio_chip *chip, unsigned offset)
{
	struct max7301 *ts = container_of(chip, struct max7301, chip);
	u8 *config;
	int ret;

	
	offset += 4;

	config = &ts->port_config[offset >> 2];

	mutex_lock(&ts->lock);

	
	*config = (*config & ~(3 << (offset & 3))) | (1 << (offset & 3));

	ret = max7301_write(ts->spi, 0x08 + (offset >> 2), *config);

	mutex_unlock(&ts->lock);

	return ret;
}

static int __max7301_set(struct max7301 *ts, unsigned offset, int value)
{
	if (value) {
		ts->out_level |= 1 << offset;
		return max7301_write(ts->spi, 0x20 + offset, 0x01);
	} else {
		ts->out_level &= ~(1 << offset);
		return max7301_write(ts->spi, 0x20 + offset, 0x00);
	}
}

static int max7301_direction_output(struct gpio_chip *chip, unsigned offset,
				    int value)
{
	struct max7301 *ts = container_of(chip, struct max7301, chip);
	u8 *config;
	int ret;

	
	offset += 4;

	config = &ts->port_config[offset >> 2];

	mutex_lock(&ts->lock);

	*config = (*config & ~(3 << (offset & 3))) | (1 << (offset & 3));

	ret = __max7301_set(ts, offset, value);

	if (!ret)
		ret = max7301_write(ts->spi, 0x08 + (offset >> 2), *config);

	mutex_unlock(&ts->lock);

	return ret;
}

static int max7301_get(struct gpio_chip *chip, unsigned offset)
{
	struct max7301 *ts = container_of(chip, struct max7301, chip);
	int config, level = -EINVAL;

	
	offset += 4;

	mutex_lock(&ts->lock);

	config = (ts->port_config[offset >> 2] >> ((offset & 3) * 2)) & 3;

	switch (config) {
	case 1:
		
		level =  !!(ts->out_level & (1 << offset));
		break;
	case 2:
	case 3:
		
		level = max7301_read(ts->spi, 0x20 + offset) & 0x01;
	}
	mutex_unlock(&ts->lock);

	return level;
}

static void max7301_set(struct gpio_chip *chip, unsigned offset, int value)
{
	struct max7301 *ts = container_of(chip, struct max7301, chip);

	
	offset += 4;

	mutex_lock(&ts->lock);

	__max7301_set(ts, offset, value);

	mutex_unlock(&ts->lock);
}

static int __devinit max7301_probe(struct spi_device *spi)
{
	struct max7301 *ts;
	struct max7301_platform_data *pdata;
	int i, ret;

	pdata = spi->dev.platform_data;
	if (!pdata || !pdata->base) {
		dev_dbg(&spi->dev, "incorrect or missing platform data\n");
		return -EINVAL;
	}

	
	spi->bits_per_word = 16;

	ret = spi_setup(spi);
	if (ret < 0)
		return ret;

	ts = kzalloc(sizeof(struct max7301), GFP_KERNEL);
	if (!ts)
		return -ENOMEM;

	mutex_init(&ts->lock);

	dev_set_drvdata(&spi->dev, ts);

	
	max7301_write(spi, 0x04, 0x01);

	ts->spi = spi;

	ts->chip.label = DRIVER_NAME,

	ts->chip.direction_input = max7301_direction_input;
	ts->chip.get = max7301_get;
	ts->chip.direction_output = max7301_direction_output;
	ts->chip.set = max7301_set;

	ts->chip.base = pdata->base;
	ts->chip.ngpio = PIN_NUMBER;
	ts->chip.can_sleep = 1;
	ts->chip.dev = &spi->dev;
	ts->chip.owner = THIS_MODULE;

	
	for (i = 1; i < 8; i++) {
		int j;
		
		max7301_write(spi, 0x08 + i, 0xAA);
		ts->port_config[i] = 0xAA;
		for (j = 0; j < 4; j++) {
			int offset = (i - 1) * 4 + j;
			ret = max7301_direction_input(&ts->chip, offset);
			if (ret)
				goto exit_destroy;
		}
	}

	ret = gpiochip_add(&ts->chip);
	if (ret)
		goto exit_destroy;

	return ret;

exit_destroy:
	dev_set_drvdata(&spi->dev, NULL);
	mutex_destroy(&ts->lock);
	kfree(ts);
	return ret;
}

static int __devexit max7301_remove(struct spi_device *spi)
{
	struct max7301 *ts;
	int ret;

	ts = dev_get_drvdata(&spi->dev);
	if (ts == NULL)
		return -ENODEV;

	dev_set_drvdata(&spi->dev, NULL);

	
	max7301_write(spi, 0x04, 0x00);

	ret = gpiochip_remove(&ts->chip);
	if (!ret) {
		mutex_destroy(&ts->lock);
		kfree(ts);
	} else
		dev_err(&spi->dev, "Failed to remove the GPIO controller: %d\n",
			ret);

	return ret;
}

static struct spi_driver max7301_driver = {
	.driver = {
		.name		= DRIVER_NAME,
		.owner		= THIS_MODULE,
	},
	.probe		= max7301_probe,
	.remove		= __devexit_p(max7301_remove),
};

static int __init max7301_init(void)
{
	return spi_register_driver(&max7301_driver);
}

subsys_initcall(max7301_init);

static void __exit max7301_exit(void)
{
	spi_unregister_driver(&max7301_driver);
}
module_exit(max7301_exit);

MODULE_AUTHOR("Juergen Beisert");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("MAX7301 SPI based GPIO-Expander");
MODULE_ALIAS("spi:" DRIVER_NAME);
