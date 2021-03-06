

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>
#include <asm/io.h>
#include "i2c-parport.h"

#define DEFAULT_BASE 0x378
#define DRVNAME "i2c-parport-light"

static struct platform_device *pdev;

static u16 base;
module_param(base, ushort, 0);
MODULE_PARM_DESC(base, "Base I/O address");



static inline void port_write(unsigned char p, unsigned char d)
{
	outb(d, base+p);
}

static inline unsigned char port_read(unsigned char p)
{
	return inb(base+p);
}



static inline void line_set(int state, const struct lineop *op)
{
	u8 oldval = port_read(op->port);

	
	if ((op->inverted && !state) || (!op->inverted && state))
		port_write(op->port, oldval | op->val);
	else
		port_write(op->port, oldval & ~op->val);
}

static inline int line_get(const struct lineop *op)
{
	u8 oldval = port_read(op->port);

	return ((op->inverted && (oldval & op->val) != op->val)
	    || (!op->inverted && (oldval & op->val) == op->val));
}



static void parport_setscl(void *data, int state)
{
	line_set(state, &adapter_parm[type].setscl);
}

static void parport_setsda(void *data, int state)
{
	line_set(state, &adapter_parm[type].setsda);
}

static int parport_getscl(void *data)
{
	return line_get(&adapter_parm[type].getscl);
}

static int parport_getsda(void *data)
{
	return line_get(&adapter_parm[type].getsda);
}


static struct i2c_algo_bit_data parport_algo_data = {
	.setsda		= parport_setsda,
	.setscl		= parport_setscl,
	.getsda		= parport_getsda,
	.getscl		= parport_getscl,
	.udelay		= 50,
	.timeout	= HZ,
}; 



static struct i2c_adapter parport_adapter = {
	.owner		= THIS_MODULE,
	.class		= I2C_CLASS_HWMON,
	.algo_data	= &parport_algo_data,
	.name		= "Parallel port adapter (light)",
};

static int __devinit i2c_parport_probe(struct platform_device *pdev)
{
	int err;

	
	parport_setsda(NULL, 1);
	parport_setscl(NULL, 1);
	
	if (adapter_parm[type].init.val)
		line_set(1, &adapter_parm[type].init);

	parport_adapter.dev.parent = &pdev->dev;
	err = i2c_bit_add_bus(&parport_adapter);
	if (err)
		dev_err(&pdev->dev, "Unable to register with I2C\n");
	return err;
}

static int __devexit i2c_parport_remove(struct platform_device *pdev)
{
	i2c_del_adapter(&parport_adapter);

	
	if (adapter_parm[type].init.val)
		line_set(0, &adapter_parm[type].init);

	return 0;
}

static struct platform_driver i2c_parport_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= DRVNAME,
	},
	.probe		= i2c_parport_probe,
	.remove		= __devexit_p(i2c_parport_remove),
};

static int __init i2c_parport_device_add(u16 address)
{
	int err;

	pdev = platform_device_alloc(DRVNAME, -1);
	if (!pdev) {
		err = -ENOMEM;
		printk(KERN_ERR DRVNAME ": Device allocation failed\n");
		goto exit;
	}

	err = platform_device_add(pdev);
	if (err) {
		printk(KERN_ERR DRVNAME ": Device addition failed (%d)\n",
		       err);
		goto exit_device_put;
	}

	return 0;

exit_device_put:
	platform_device_put(pdev);
exit:
	return err;
}

static int __init i2c_parport_init(void)
{
	int err;

	if (type < 0) {
		printk(KERN_ERR DRVNAME ": adapter type unspecified\n");
		return -ENODEV;
	}

	if (type >= ARRAY_SIZE(adapter_parm)) {
		printk(KERN_ERR DRVNAME ": invalid type (%d)\n", type);
		return -ENODEV;
	}

	if (base == 0) {
		pr_info(DRVNAME ": using default base 0x%x\n", DEFAULT_BASE);
		base = DEFAULT_BASE;
	}

	if (!request_region(base, 3, DRVNAME))
		return -EBUSY;

        if (!adapter_parm[type].getscl.val)
		parport_algo_data.getscl = NULL;

	
	err = i2c_parport_device_add(base);
	if (err)
		goto exit_release;

	err = platform_driver_register(&i2c_parport_driver);
	if (err)
		goto exit_device;

	return 0;

exit_device:
	platform_device_unregister(pdev);
exit_release:
	release_region(base, 3);
	return err;
}

static void __exit i2c_parport_exit(void)
{
	platform_driver_unregister(&i2c_parport_driver);
	platform_device_unregister(pdev);
	release_region(base, 3);
}

MODULE_AUTHOR("Jean Delvare <khali@linux-fr.org>");
MODULE_DESCRIPTION("I2C bus over parallel port (light)");
MODULE_LICENSE("GPL");

module_init(i2c_parport_init);
module_exit(i2c_parport_exit);
