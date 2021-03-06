

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>

#include "../w1.h"
#include "../w1_int.h"
#include "../w1_log.h"


#define MXC_W1_RESET_TIMEOUT 500


#define MXC_W1_CONTROL          0x00
#define MXC_W1_TIME_DIVIDER     0x02
#define MXC_W1_RESET            0x04
#define MXC_W1_COMMAND          0x06
#define MXC_W1_TXRX             0x08
#define MXC_W1_INTERRUPT        0x0A
#define MXC_W1_INTERRUPT_EN     0x0C

struct mxc_w1_device {
	void __iomem *regs;
	unsigned int clkdiv;
	struct clk *clk;
	struct w1_bus_master bus_master;
};


static u8 mxc_w1_ds2_reset_bus(void *data)
{
	u8 reg_val;
	unsigned int timeout_cnt = 0;
	struct mxc_w1_device *dev = data;

	__raw_writeb(0x80, (dev->regs + MXC_W1_CONTROL));

	while (1) {
		reg_val = __raw_readb(dev->regs + MXC_W1_CONTROL);

		if (((reg_val >> 7) & 0x1) == 0 ||
		    timeout_cnt > MXC_W1_RESET_TIMEOUT)
			break;
		else
			timeout_cnt++;

		udelay(100);
	}
	return (reg_val >> 7) & 0x1;
}


static u8 mxc_w1_ds2_touch_bit(void *data, u8 bit)
{
	struct mxc_w1_device *mdev = data;
	void __iomem *ctrl_addr = mdev->regs + MXC_W1_CONTROL;
	unsigned int timeout_cnt = 400; 

	__raw_writeb((1 << (5 - bit)), ctrl_addr);

	while (timeout_cnt--) {
		if (!((__raw_readb(ctrl_addr) >> (5 - bit)) & 0x1))
			break;

		udelay(1);
	}

	return ((__raw_readb(ctrl_addr)) >> 3) & 0x1;
}

static int __init mxc_w1_probe(struct platform_device *pdev)
{
	struct mxc_w1_device *mdev;
	struct resource *res;
	int err = 0;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENODEV;

	mdev = kzalloc(sizeof(struct mxc_w1_device), GFP_KERNEL);
	if (!mdev)
		return -ENOMEM;

	mdev->clk = clk_get(&pdev->dev, "owire");
	if (!mdev->clk) {
		err = -ENODEV;
		goto failed_clk;
	}

	mdev->clkdiv = (clk_get_rate(mdev->clk) / 1000000) - 1;

	res = request_mem_region(res->start, resource_size(res),
				"mxc_w1");
	if (!res) {
		err = -EBUSY;
		goto failed_req;
	}

	mdev->regs = ioremap(res->start, resource_size(res));
	if (!mdev->regs) {
		printk(KERN_ERR "Cannot map frame buffer registers\n");
		goto failed_ioremap;
	}

	clk_enable(mdev->clk);
	__raw_writeb(mdev->clkdiv, mdev->regs + MXC_W1_TIME_DIVIDER);

	mdev->bus_master.data = mdev;
	mdev->bus_master.reset_bus = mxc_w1_ds2_reset_bus;
	mdev->bus_master.touch_bit = mxc_w1_ds2_touch_bit;

	err = w1_add_master_device(&mdev->bus_master);

	if (err)
		goto failed_add;

	platform_set_drvdata(pdev, mdev);
	return 0;

failed_add:
	iounmap(mdev->regs);
failed_ioremap:
	release_mem_region(res->start, resource_size(res));
failed_req:
	clk_put(mdev->clk);
failed_clk:
	kfree(mdev);
	return err;
}


static int mxc_w1_remove(struct platform_device *pdev)
{
	struct mxc_w1_device *mdev = platform_get_drvdata(pdev);
	struct resource *res;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	w1_remove_master_device(&mdev->bus_master);

	iounmap(mdev->regs);
	release_mem_region(res->start, resource_size(res));
	clk_disable(mdev->clk);
	clk_put(mdev->clk);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver mxc_w1_driver = {
	.driver = {
		   .name = "mxc_w1",
	},
	.probe = mxc_w1_probe,
	.remove = mxc_w1_remove,
};

static int __init mxc_w1_init(void)
{
	return platform_driver_register(&mxc_w1_driver);
}

static void mxc_w1_exit(void)
{
	platform_driver_unregister(&mxc_w1_driver);
}

module_init(mxc_w1_init);
module_exit(mxc_w1_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Freescale Semiconductors Inc");
MODULE_DESCRIPTION("Driver for One-Wire on MXC");
