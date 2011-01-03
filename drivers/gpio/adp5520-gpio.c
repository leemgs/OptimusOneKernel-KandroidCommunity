

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mfd/adp5520.h>

#include <linux/gpio.h>

struct adp5520_gpio {
	struct device *master;
	struct gpio_chip gpio_chip;
	unsigned char lut[ADP5520_MAXGPIOS];
	unsigned long output;
};

static int adp5520_gpio_get_value(struct gpio_chip *chip, unsigned off)
{
	struct adp5520_gpio *dev;
	uint8_t reg_val;

	dev = container_of(chip, struct adp5520_gpio, gpio_chip);

	

	if (test_bit(off, &dev->output))
		adp5520_read(dev->master, GPIO_OUT, &reg_val);
	else
		adp5520_read(dev->master, GPIO_IN, &reg_val);

	return !!(reg_val & dev->lut[off]);
}

static void adp5520_gpio_set_value(struct gpio_chip *chip,
		unsigned off, int val)
{
	struct adp5520_gpio *dev;
	dev = container_of(chip, struct adp5520_gpio, gpio_chip);

	if (val)
		adp5520_set_bits(dev->master, GPIO_OUT, dev->lut[off]);
	else
		adp5520_clr_bits(dev->master, GPIO_OUT, dev->lut[off]);
}

static int adp5520_gpio_direction_input(struct gpio_chip *chip, unsigned off)
{
	struct adp5520_gpio *dev;
	dev = container_of(chip, struct adp5520_gpio, gpio_chip);

	clear_bit(off, &dev->output);

	return adp5520_clr_bits(dev->master, GPIO_CFG_2, dev->lut[off]);
}

static int adp5520_gpio_direction_output(struct gpio_chip *chip,
		unsigned off, int val)
{
	struct adp5520_gpio *dev;
	int ret = 0;
	dev = container_of(chip, struct adp5520_gpio, gpio_chip);

	set_bit(off, &dev->output);

	if (val)
		ret |= adp5520_set_bits(dev->master, GPIO_OUT, dev->lut[off]);
	else
		ret |= adp5520_clr_bits(dev->master, GPIO_OUT, dev->lut[off]);

	ret |= adp5520_set_bits(dev->master, GPIO_CFG_2, dev->lut[off]);

	return ret;
}

static int __devinit adp5520_gpio_probe(struct platform_device *pdev)
{
	struct adp5520_gpio_platfrom_data *pdata = pdev->dev.platform_data;
	struct adp5520_gpio *dev;
	struct gpio_chip *gc;
	int ret, i, gpios;
	unsigned char ctl_mask = 0;

	if (pdata == NULL) {
		dev_err(&pdev->dev, "missing platform data\n");
		return -ENODEV;
	}

	if (pdev->id != ID_ADP5520) {
		dev_err(&pdev->dev, "only ADP5520 supports GPIO\n");
		return -ENODEV;
	}

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (dev == NULL) {
		dev_err(&pdev->dev, "failed to alloc memory\n");
		return -ENOMEM;
	}

	dev->master = pdev->dev.parent;

	for (gpios = 0, i = 0; i < ADP5520_MAXGPIOS; i++)
		if (pdata->gpio_en_mask & (1 << i))
			dev->lut[gpios++] = 1 << i;

	if (gpios < 1) {
		ret = -EINVAL;
		goto err;
	}

	gc = &dev->gpio_chip;
	gc->direction_input  = adp5520_gpio_direction_input;
	gc->direction_output = adp5520_gpio_direction_output;
	gc->get = adp5520_gpio_get_value;
	gc->set = adp5520_gpio_set_value;
	gc->can_sleep = 1;

	gc->base = pdata->gpio_start;
	gc->ngpio = gpios;
	gc->label = pdev->name;
	gc->owner = THIS_MODULE;

	ret = adp5520_clr_bits(dev->master, GPIO_CFG_1,
		pdata->gpio_en_mask);

	if (pdata->gpio_en_mask & GPIO_C3)
		ctl_mask |= C3_MODE;

	if (pdata->gpio_en_mask & GPIO_R3)
		ctl_mask |= R3_MODE;

	if (ctl_mask)
		ret = adp5520_set_bits(dev->master, LED_CONTROL,
			ctl_mask);

	ret |= adp5520_set_bits(dev->master, GPIO_PULLUP,
		pdata->gpio_pullup_mask);

	if (ret) {
		dev_err(&pdev->dev, "failed to write\n");
		goto err;
	}

	ret = gpiochip_add(&dev->gpio_chip);
	if (ret)
		goto err;

	platform_set_drvdata(pdev, dev);
	return 0;

err:
	kfree(dev);
	return ret;
}

static int __devexit adp5520_gpio_remove(struct platform_device *pdev)
{
	struct adp5520_gpio *dev;
	int ret;

	dev = platform_get_drvdata(pdev);
	ret = gpiochip_remove(&dev->gpio_chip);
	if (ret) {
		dev_err(&pdev->dev, "%s failed, %d\n",
				"gpiochip_remove()", ret);
		return ret;
	}

	kfree(dev);
	return 0;
}

static struct platform_driver adp5520_gpio_driver = {
	.driver	= {
		.name	= "adp5520-gpio",
		.owner	= THIS_MODULE,
	},
	.probe		= adp5520_gpio_probe,
	.remove		= __devexit_p(adp5520_gpio_remove),
};

static int __init adp5520_gpio_init(void)
{
	return platform_driver_register(&adp5520_gpio_driver);
}
module_init(adp5520_gpio_init);

static void __exit adp5520_gpio_exit(void)
{
	platform_driver_unregister(&adp5520_gpio_driver);
}
module_exit(adp5520_gpio_exit);

MODULE_AUTHOR("Michael Hennerich <hennerich@blackfin.uclinux.org>");
MODULE_DESCRIPTION("GPIO ADP5520 Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:adp5520-gpio");
