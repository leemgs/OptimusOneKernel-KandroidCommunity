

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/platform_device.h>

#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>

#include <asm/io.h>

struct simtec_i2c_data {
	struct resource		*ioarea;
	void __iomem		*reg;
	struct i2c_adapter	 adap;
	struct i2c_algo_bit_data bit;
};

#define CMD_SET_SDA	(1<<2)
#define CMD_SET_SCL	(1<<3)

#define STATE_SDA	(1<<0)
#define STATE_SCL	(1<<1)



static void simtec_i2c_setsda(void *pw, int state)
{
	struct simtec_i2c_data *pd = pw;
	writeb(CMD_SET_SDA | (state ? STATE_SDA : 0), pd->reg);
}

static void simtec_i2c_setscl(void *pw, int state)
{
	struct simtec_i2c_data *pd = pw;
	writeb(CMD_SET_SCL | (state ? STATE_SCL : 0), pd->reg);
}

static int simtec_i2c_getsda(void *pw)
{
	struct simtec_i2c_data *pd = pw;
	return readb(pd->reg) & STATE_SDA ? 1 : 0;
}

static int simtec_i2c_getscl(void *pw)
{
	struct simtec_i2c_data *pd = pw;
	return readb(pd->reg) & STATE_SCL ? 1 : 0;
}



static int simtec_i2c_probe(struct platform_device *dev)
{
	struct simtec_i2c_data *pd;
	struct resource *res;
	int size;
	int ret;

	pd = kzalloc(sizeof(struct simtec_i2c_data), GFP_KERNEL);
	if (pd == NULL) {
		dev_err(&dev->dev, "cannot allocate private data\n");
		return -ENOMEM;
	}

	platform_set_drvdata(dev, pd);

	res = platform_get_resource(dev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&dev->dev, "cannot find IO resource\n");
		ret = -ENOENT;
		goto err;
	}

	size = resource_size(res);

	pd->ioarea = request_mem_region(res->start, size, dev->name);
	if (pd->ioarea == NULL) {
		dev_err(&dev->dev, "cannot request IO\n");
		ret = -ENXIO;
		goto err;
	}

	pd->reg = ioremap(res->start, size);
	if (pd->reg == NULL) {
		dev_err(&dev->dev, "cannot map IO\n");
		ret = -ENXIO;
		goto err_res;
	}

	

	pd->adap.owner = THIS_MODULE;
	pd->adap.algo_data = &pd->bit;
	pd->adap.dev.parent = &dev->dev;

	strlcpy(pd->adap.name, "Simtec I2C", sizeof(pd->adap.name));

	pd->bit.data = pd;
	pd->bit.setsda = simtec_i2c_setsda;
	pd->bit.setscl = simtec_i2c_setscl;
	pd->bit.getsda = simtec_i2c_getsda;
	pd->bit.getscl = simtec_i2c_getscl;
	pd->bit.timeout = HZ;
	pd->bit.udelay = 20;

	ret = i2c_bit_add_bus(&pd->adap);
	if (ret)
		goto err_all;

	return 0;

 err_all:
	iounmap(pd->reg);

 err_res:
	release_resource(pd->ioarea);
	kfree(pd->ioarea);

 err:
	kfree(pd);
	return ret;
}

static int simtec_i2c_remove(struct platform_device *dev)
{
	struct simtec_i2c_data *pd = platform_get_drvdata(dev);

	i2c_del_adapter(&pd->adap);

	iounmap(pd->reg);
	release_resource(pd->ioarea);
	kfree(pd->ioarea);
	kfree(pd);

	return 0;
}





MODULE_ALIAS("platform:simtec-i2c");

static struct platform_driver simtec_i2c_driver = {
	.driver		= {
		.name		= "simtec-i2c",
		.owner		= THIS_MODULE,
	},
	.probe		= simtec_i2c_probe,
	.remove		= simtec_i2c_remove,
};

static int __init i2c_adap_simtec_init(void)
{
	return platform_driver_register(&simtec_i2c_driver);
}

static void __exit i2c_adap_simtec_exit(void)
{
	platform_driver_unregister(&simtec_i2c_driver);
}

module_init(i2c_adap_simtec_init);
module_exit(i2c_adap_simtec_exit);

MODULE_DESCRIPTION("Simtec Generic I2C Bus driver");
MODULE_AUTHOR("Ben Dooks <ben@simtec.co.uk>");
MODULE_LICENSE("GPL");
