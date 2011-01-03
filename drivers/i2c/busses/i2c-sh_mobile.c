

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>
#include <linux/io.h>






































































enum sh_mobile_i2c_op {
	OP_START = 0,
	OP_TX_FIRST,
	OP_TX,
	OP_TX_STOP,
	OP_TX_TO_RX,
	OP_RX,
	OP_RX_STOP,
	OP_RX_STOP_DATA,
};

struct sh_mobile_i2c_data {
	struct device *dev;
	void __iomem *reg;
	struct i2c_adapter adap;

	struct clk *clk;
	u_int8_t iccl;
	u_int8_t icch;

	spinlock_t lock;
	wait_queue_head_t wait;
	struct i2c_msg *msg;
	int pos;
	int sr;
};

#define NORMAL_SPEED		100000 


#define ICDR(pd)		(pd->reg + 0x00)
#define ICCR(pd)		(pd->reg + 0x04)
#define ICSR(pd)		(pd->reg + 0x08)
#define ICIC(pd)		(pd->reg + 0x0c)
#define ICCL(pd)		(pd->reg + 0x10)
#define ICCH(pd)		(pd->reg + 0x14)


#define ICCR_ICE		0x80
#define ICCR_RACK		0x40
#define ICCR_TRS		0x10
#define ICCR_BBSY		0x04
#define ICCR_SCP		0x01

#define ICSR_SCLM		0x80
#define ICSR_SDAM		0x40
#define SW_DONE			0x20
#define ICSR_BUSY		0x10
#define ICSR_AL			0x08
#define ICSR_TACK		0x04
#define ICSR_WAIT		0x02
#define ICSR_DTE		0x01

#define ICIC_ALE		0x08
#define ICIC_TACKE		0x04
#define ICIC_WAITE		0x02
#define ICIC_DTEE		0x01

static void activate_ch(struct sh_mobile_i2c_data *pd)
{
	unsigned long i2c_clk;
	u_int32_t num;
	u_int32_t denom;
	u_int32_t tmp;

	
	pm_runtime_get_sync(pd->dev);
	clk_enable(pd->clk);

	
	i2c_clk = clk_get_rate(pd->clk);

	
	num = i2c_clk * 5;
	denom = NORMAL_SPEED * 9;
	tmp = num * 10 / denom;
	if (tmp % 10 >= 5)
		pd->iccl = (u_int8_t)((num/denom) + 1);
	else
		pd->iccl = (u_int8_t)(num/denom);

	
	num = i2c_clk * 4;
	tmp = num * 10 / denom;
	if (tmp % 10 >= 5)
		pd->icch = (u_int8_t)((num/denom) + 1);
	else
		pd->icch = (u_int8_t)(num/denom);

	
	iowrite8(ioread8(ICCR(pd)) | ICCR_ICE, ICCR(pd));

	
	iowrite8(0, ICIC(pd));

	
	iowrite8(pd->iccl, ICCL(pd));
	iowrite8(pd->icch, ICCH(pd));
}

static void deactivate_ch(struct sh_mobile_i2c_data *pd)
{
	
	iowrite8(0, ICSR(pd));
	iowrite8(0, ICIC(pd));

	
	iowrite8(ioread8(ICCR(pd)) & ~ICCR_ICE, ICCR(pd));

	
	clk_disable(pd->clk);
	pm_runtime_put_sync(pd->dev);
}

static unsigned char i2c_op(struct sh_mobile_i2c_data *pd,
			    enum sh_mobile_i2c_op op, unsigned char data)
{
	unsigned char ret = 0;
	unsigned long flags;

	dev_dbg(pd->dev, "op %d, data in 0x%02x\n", op, data);

	spin_lock_irqsave(&pd->lock, flags);

	switch (op) {
	case OP_START: 
		iowrite8(0x94, ICCR(pd));
		break;
	case OP_TX_FIRST: 
		iowrite8(ICIC_WAITE | ICIC_ALE | ICIC_TACKE, ICIC(pd));
		iowrite8(data, ICDR(pd));
		break;
	case OP_TX: 
		iowrite8(data, ICDR(pd));
		break;
	case OP_TX_STOP: 
		iowrite8(data, ICDR(pd));
		iowrite8(0x90, ICCR(pd));
		break;
	case OP_TX_TO_RX: 
		iowrite8(0x81, ICCR(pd));
		break;
	case OP_RX: 
		ret = ioread8(ICDR(pd));
		break;
	case OP_RX_STOP: 
		iowrite8(ICIC_DTEE | ICIC_WAITE | ICIC_ALE | ICIC_TACKE,
			 ICIC(pd));
		iowrite8(0xc0, ICCR(pd));
		break;
	case OP_RX_STOP_DATA: 
		iowrite8(ICIC_DTEE | ICIC_WAITE | ICIC_ALE | ICIC_TACKE,
			 ICIC(pd));
		ret = ioread8(ICDR(pd));
		iowrite8(0xc0, ICCR(pd));
		break;
	}

	spin_unlock_irqrestore(&pd->lock, flags);

	dev_dbg(pd->dev, "op %d, data out 0x%02x\n", op, ret);
	return ret;
}

static int sh_mobile_i2c_is_first_byte(struct sh_mobile_i2c_data *pd)
{
	if (pd->pos == -1)
		return 1;

	return 0;
}

static int sh_mobile_i2c_is_last_byte(struct sh_mobile_i2c_data *pd)
{
	if (pd->pos == (pd->msg->len - 1))
		return 1;

	return 0;
}

static void sh_mobile_i2c_get_data(struct sh_mobile_i2c_data *pd,
				   unsigned char *buf)
{
	switch (pd->pos) {
	case -1:
		*buf = (pd->msg->addr & 0x7f) << 1;
		*buf |= (pd->msg->flags & I2C_M_RD) ? 1 : 0;
		break;
	default:
		*buf = pd->msg->buf[pd->pos];
	}
}

static int sh_mobile_i2c_isr_tx(struct sh_mobile_i2c_data *pd)
{
	unsigned char data;

	if (pd->pos == pd->msg->len)
		return 1;

	sh_mobile_i2c_get_data(pd, &data);

	if (sh_mobile_i2c_is_last_byte(pd))
		i2c_op(pd, OP_TX_STOP, data);
	else if (sh_mobile_i2c_is_first_byte(pd))
		i2c_op(pd, OP_TX_FIRST, data);
	else
		i2c_op(pd, OP_TX, data);

	pd->pos++;
	return 0;
}

static int sh_mobile_i2c_isr_rx(struct sh_mobile_i2c_data *pd)
{
	unsigned char data;
	int real_pos;

	do {
		if (pd->pos <= -1) {
			sh_mobile_i2c_get_data(pd, &data);

			if (sh_mobile_i2c_is_first_byte(pd))
				i2c_op(pd, OP_TX_FIRST, data);
			else
				i2c_op(pd, OP_TX, data);
			break;
		}

		if (pd->pos == 0) {
			i2c_op(pd, OP_TX_TO_RX, 0);
			break;
		}

		real_pos = pd->pos - 2;

		if (pd->pos == pd->msg->len) {
			if (real_pos < 0) {
				i2c_op(pd, OP_RX_STOP, 0);
				break;
			}
			data = i2c_op(pd, OP_RX_STOP_DATA, 0);
		} else
			data = i2c_op(pd, OP_RX, 0);

		if (real_pos >= 0)
			pd->msg->buf[real_pos] = data;
	} while (0);

	pd->pos++;
	return pd->pos == (pd->msg->len + 2);
}

static irqreturn_t sh_mobile_i2c_isr(int irq, void *dev_id)
{
	struct platform_device *dev = dev_id;
	struct sh_mobile_i2c_data *pd = platform_get_drvdata(dev);
	unsigned char sr;
	int wakeup;

	sr = ioread8(ICSR(pd));
	pd->sr |= sr; 

	dev_dbg(pd->dev, "i2c_isr 0x%02x 0x%02x %s %d %d!\n", sr, pd->sr,
	       (pd->msg->flags & I2C_M_RD) ? "read" : "write",
	       pd->pos, pd->msg->len);

	if (sr & (ICSR_AL | ICSR_TACK)) {
		
		iowrite8(sr & ~(ICSR_AL | ICSR_TACK), ICSR(pd));
		wakeup = 0;
	} else if (pd->msg->flags & I2C_M_RD)
		wakeup = sh_mobile_i2c_isr_rx(pd);
	else
		wakeup = sh_mobile_i2c_isr_tx(pd);

	if (sr & ICSR_WAIT) 
		iowrite8(sr & ~ICSR_WAIT, ICSR(pd));

	if (wakeup) {
		pd->sr |= SW_DONE;
		wake_up(&pd->wait);
	}

	return IRQ_HANDLED;
}

static int start_ch(struct sh_mobile_i2c_data *pd, struct i2c_msg *usr_msg)
{
	if (usr_msg->len == 0 && (usr_msg->flags & I2C_M_RD)) {
		dev_err(pd->dev, "Unsupported zero length i2c read\n");
		return -EIO;
	}

	
	iowrite8(ioread8(ICCR(pd)) & ~ICCR_ICE, ICCR(pd));

	
	iowrite8(ioread8(ICCR(pd)) | ICCR_ICE, ICCR(pd));

	
	iowrite8(pd->iccl, ICCL(pd));
	iowrite8(pd->icch, ICCH(pd));

	pd->msg = usr_msg;
	pd->pos = -1;
	pd->sr = 0;

	
	iowrite8(ICIC_WAITE | ICIC_ALE | ICIC_TACKE | ICIC_DTEE, ICIC(pd));
	return 0;
}

static int sh_mobile_i2c_xfer(struct i2c_adapter *adapter,
			      struct i2c_msg *msgs,
			      int num)
{
	struct sh_mobile_i2c_data *pd = i2c_get_adapdata(adapter);
	struct i2c_msg	*msg;
	int err = 0;
	u_int8_t val;
	int i, k, retry_count;

	activate_ch(pd);

	
	for (i = 0; i < num; i++) {
		msg = &msgs[i];

		err = start_ch(pd, msg);
		if (err)
			break;

		i2c_op(pd, OP_START, 0);

		
		k = wait_event_timeout(pd->wait,
				       pd->sr & (ICSR_TACK | SW_DONE),
				       5 * HZ);
		if (!k)
			dev_err(pd->dev, "Transfer request timed out\n");

		retry_count = 1000;
again:
		val = ioread8(ICSR(pd));

		dev_dbg(pd->dev, "val 0x%02x pd->sr 0x%02x\n", val, pd->sr);

		
		if (val & ICSR_BUSY) {
			udelay(10);
			if (retry_count--)
				goto again;

			err = -EIO;
			dev_err(pd->dev, "Polling timed out\n");
			break;
		}

		
		if ((val | pd->sr) & (ICSR_TACK | ICSR_AL)) {
			err = -EIO;
			break;
		}
	}

	deactivate_ch(pd);

	if (!err)
		err = num;
	return err;
}

static u32 sh_mobile_i2c_func(struct i2c_adapter *adapter)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static struct i2c_algorithm sh_mobile_i2c_algorithm = {
	.functionality	= sh_mobile_i2c_func,
	.master_xfer	= sh_mobile_i2c_xfer,
};

static int sh_mobile_i2c_hook_irqs(struct platform_device *dev, int hook)
{
	struct resource *res;
	int ret = -ENXIO;
	int q, m;
	int k = 0;
	int n = 0;

	while ((res = platform_get_resource(dev, IORESOURCE_IRQ, k))) {
		for (n = res->start; hook && n <= res->end; n++) {
			if (request_irq(n, sh_mobile_i2c_isr, IRQF_DISABLED,
					dev_name(&dev->dev), dev))
				goto rollback;
		}
		k++;
	}

	if (hook)
		return k > 0 ? 0 : -ENOENT;

	k--;
	ret = 0;

 rollback:
	for (q = k; k >= 0; k--) {
		for (m = n; m >= res->start; m--)
			free_irq(m, dev);

		res = platform_get_resource(dev, IORESOURCE_IRQ, k - 1);
		m = res->end;
	}

	return ret;
}

static int sh_mobile_i2c_probe(struct platform_device *dev)
{
	struct sh_mobile_i2c_data *pd;
	struct i2c_adapter *adap;
	struct resource *res;
	char clk_name[8];
	int size;
	int ret;

	pd = kzalloc(sizeof(struct sh_mobile_i2c_data), GFP_KERNEL);
	if (pd == NULL) {
		dev_err(&dev->dev, "cannot allocate private data\n");
		return -ENOMEM;
	}

	snprintf(clk_name, sizeof(clk_name), "i2c%d", dev->id);
	pd->clk = clk_get(&dev->dev, clk_name);
	if (IS_ERR(pd->clk)) {
		dev_err(&dev->dev, "cannot get clock \"%s\"\n", clk_name);
		ret = PTR_ERR(pd->clk);
		goto err;
	}

	ret = sh_mobile_i2c_hook_irqs(dev, 1);
	if (ret) {
		dev_err(&dev->dev, "cannot request IRQ\n");
		goto err_clk;
	}

	pd->dev = &dev->dev;
	platform_set_drvdata(dev, pd);

	res = platform_get_resource(dev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&dev->dev, "cannot find IO resource\n");
		ret = -ENOENT;
		goto err_irq;
	}

	size = resource_size(res);

	pd->reg = ioremap(res->start, size);
	if (pd->reg == NULL) {
		dev_err(&dev->dev, "cannot map IO\n");
		ret = -ENXIO;
		goto err_irq;
	}

	
	pm_suspend_ignore_children(&dev->dev, true);
	pm_runtime_enable(&dev->dev);

	
	adap = &pd->adap;
	i2c_set_adapdata(adap, pd);

	adap->owner = THIS_MODULE;
	adap->algo = &sh_mobile_i2c_algorithm;
	adap->dev.parent = &dev->dev;
	adap->retries = 5;
	adap->nr = dev->id;

	strlcpy(adap->name, dev->name, sizeof(adap->name));

	spin_lock_init(&pd->lock);
	init_waitqueue_head(&pd->wait);

	ret = i2c_add_numbered_adapter(adap);
	if (ret < 0) {
		dev_err(&dev->dev, "cannot add numbered adapter\n");
		goto err_all;
	}

	return 0;

 err_all:
	iounmap(pd->reg);
 err_irq:
	sh_mobile_i2c_hook_irqs(dev, 0);
 err_clk:
	clk_put(pd->clk);
 err:
	kfree(pd);
	return ret;
}

static int sh_mobile_i2c_remove(struct platform_device *dev)
{
	struct sh_mobile_i2c_data *pd = platform_get_drvdata(dev);

	i2c_del_adapter(&pd->adap);
	iounmap(pd->reg);
	sh_mobile_i2c_hook_irqs(dev, 0);
	clk_put(pd->clk);
	pm_runtime_disable(&dev->dev);
	kfree(pd);
	return 0;
}

static int sh_mobile_i2c_runtime_nop(struct device *dev)
{
	
	return 0;
}

static struct dev_pm_ops sh_mobile_i2c_dev_pm_ops = {
	.runtime_suspend = sh_mobile_i2c_runtime_nop,
	.runtime_resume = sh_mobile_i2c_runtime_nop,
};

static struct platform_driver sh_mobile_i2c_driver = {
	.driver		= {
		.name		= "i2c-sh_mobile",
		.owner		= THIS_MODULE,
		.pm		= &sh_mobile_i2c_dev_pm_ops,
	},
	.probe		= sh_mobile_i2c_probe,
	.remove		= sh_mobile_i2c_remove,
};

static int __init sh_mobile_i2c_adap_init(void)
{
	return platform_driver_register(&sh_mobile_i2c_driver);
}

static void __exit sh_mobile_i2c_adap_exit(void)
{
	platform_driver_unregister(&sh_mobile_i2c_driver);
}

subsys_initcall(sh_mobile_i2c_adap_init);
module_exit(sh_mobile_i2c_adap_exit);

MODULE_DESCRIPTION("SuperH Mobile I2C Bus Controller driver");
MODULE_AUTHOR("Magnus Damm");
MODULE_LICENSE("GPL v2");
