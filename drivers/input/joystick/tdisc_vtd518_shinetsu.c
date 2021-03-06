

#include <linux/module.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/input/tdisc_shinetsu.h>

MODULE_LICENSE("GPL v2");
MODULE_VERSION("0.1");
MODULE_DESCRIPTION("Shinetsu Touchdisc driver");
MODULE_ALIAS("platform:tdisc-shinetsu");

#define TDSIC_BLK_READ_CMD		0x00
#define TDISC_READ_DELAY		msecs_to_jiffies(25)
#define X_MAX				(32)
#define X_MIN				(-32)
#define Y_MAX				(32)
#define Y_MIN				(-32)
#define PRESSURE_MAX			(32)
#define PRESSURE_MIN			(0)
#define TDISC_USER_ACTIVE_MASK		0x40
#define TDISC_NORTH_SWITCH_MASK		0x20
#define TDISC_SOUTH_SWITCH_MASK		0x10
#define TDISC_EAST_SWITCH_MASK		0x08
#define TDISC_WEST_SWITCH_MASK		0x04
#define TDISC_CENTER_SWITCH		0x01

#define DRIVER_NAME			"tdisc-shinetsu"
#define DEVICE_NAME			"vtd518"
#define TDISC_NAME			"tdisc_shinetsu"
#define TDISC_INT			"tdisc_interrupt"

struct tdisc_data {
	struct input_dev  *tdisc_device;
	struct i2c_client *clientp;
	struct tdisc_platform_data *pdata;
	struct delayed_work tdisc_work;
};

static void process_tdisc_data(struct tdisc_data *dd, u8 *data)
{
	int i;

	
	if (!(data[7] & TDISC_USER_ACTIVE_MASK)) {
		pr_debug(" TDISC ! No Data to report ! False positive \n");
		return;
	}

	for (i = 0; i < 8 ; i++)
		pr_debug(" Data[%d] = %x \n", i, data[i]);

	
	input_report_rel(dd->tdisc_device, REL_X, (s8) data[0]);
	input_report_rel(dd->tdisc_device, REL_Y, (s8) data[1]);

	
	
	input_report_abs(dd->tdisc_device, ABS_X, data[2]);
	input_report_abs(dd->tdisc_device, ABS_Y, data[3]);
	input_report_abs(dd->tdisc_device, ABS_PRESSURE, data[4]);

	
	input_report_rel(dd->tdisc_device, REL_WHEEL, (s8) data[6]);

	
	input_report_key(dd->tdisc_device, KEY_UP,
				(data[7] & TDISC_NORTH_SWITCH_MASK));

	input_report_key(dd->tdisc_device, KEY_DOWN,
				(data[7] & TDISC_SOUTH_SWITCH_MASK));

	input_report_key(dd->tdisc_device, KEY_RIGHT,
				 (data[7] & TDISC_EAST_SWITCH_MASK));

	input_report_key(dd->tdisc_device, KEY_LEFT,
				 (data[7] & TDISC_WEST_SWITCH_MASK));

	input_report_key(dd->tdisc_device, KEY_ENTER,
				 (data[7] & TDISC_CENTER_SWITCH));

	input_sync(dd->tdisc_device);
}


static void tdisc_work_f(struct work_struct *work)
{
	int rc;
	u8 data[8];
	struct tdisc_data	*dd =
		container_of(work, struct tdisc_data, tdisc_work.work);

	
	rc = gpio_get_value(dd->pdata->tdisc_gpio);
	if (rc < 0) {
		enable_irq(dd->clientp->irq);
		return;
	}

	pr_debug("%s: TDISC gpio_get_value = %d\n", __func__, rc);
	if (rc == 0) {
		
		rc = i2c_smbus_read_i2c_block_data(dd->clientp,
				TDSIC_BLK_READ_CMD, 8, data);
		if (rc < 0) {
			pr_err("%s:I2C read failed,trying again\n", __func__);
			rc = i2c_smbus_read_i2c_block_data(dd->clientp,
						TDSIC_BLK_READ_CMD, 8, data);
			if (rc < 0) {
				pr_err("%s:I2C read failed again, exiting\n",
								 __func__);
				goto fail_i2c_read;
			}
		}
		pr_debug("%s: TDISC: I2C read success\n", __func__);
		process_tdisc_data(dd, data);
	} else {
		
		enable_irq(dd->clientp->irq);
		return;
	}

fail_i2c_read:
	schedule_delayed_work(&dd->tdisc_work, TDISC_READ_DELAY);
}

static irqreturn_t tdisc_interrupt(int irq, void *dev_id)
{
	
	struct tdisc_data *dd = dev_id;

	pr_debug("%s: TDISC IRQ ! :-)\n", __func__);

	
	disable_irq_nosync(dd->clientp->irq);
	schedule_delayed_work(&dd->tdisc_work, 0);
	return IRQ_HANDLED;
}

static int tdisc_open(struct input_dev *dev)
{
	int rc;
	struct tdisc_data *dd = input_get_drvdata(dev);

	if (!dd->clientp) {
		
		pr_err("%s: no i2c adapter present \n", __func__);
		return  -ENODEV;
	}

	
	if (dd->pdata->tdisc_enable != NULL) {
		rc = dd->pdata->tdisc_enable();
		if (rc)
			goto fail_open;
	}

	rc = request_irq(dd->clientp->irq, &tdisc_interrupt,
			 IRQF_TRIGGER_FALLING, TDISC_INT, dd);
	if (rc) {
		pr_err("%s: request IRQ failed \n", __func__);
		goto fail_irq_open;
	}

	return 0;

fail_irq_open:
	if (dd->pdata->tdisc_disable != NULL)
		dd->pdata->tdisc_disable();
fail_open:
	return rc;
}

static void tdisc_close(struct input_dev *dev)
{
	struct tdisc_data *dd = input_get_drvdata(dev);

	free_irq(dd->clientp->irq, dd);
	cancel_delayed_work_sync(&dd->tdisc_work);
	if (dd->pdata->tdisc_disable != NULL)
		dd->pdata->tdisc_disable();
}

static int __devexit tdisc_remove(struct i2c_client *client)
{
	struct tdisc_data		*dd;

	dd = i2c_get_clientdata(client);
	input_unregister_device(dd->tdisc_device);
	if (dd->pdata->tdisc_release != NULL)
		dd->pdata->tdisc_release();
	i2c_set_clientdata(client, NULL);
	kfree(dd);

	return 0;
}

#ifdef CONFIG_PM

static int tdisc_suspend(struct device *dev)
{
	struct tdisc_data *dd;

	dd = dev_get_drvdata(dev);
	if (device_may_wakeup(&dd->clientp->dev))
		enable_irq_wake(dd->clientp->irq);

	return 0;
}

static int tdisc_resume(struct device *dev)
{
	struct tdisc_data *dd;

	dd = dev_get_drvdata(dev);
	if (device_may_wakeup(&dd->clientp->dev))
		disable_irq_wake(dd->clientp->irq);

	return 0;
}

static struct dev_pm_ops tdisc_pm_ops = {
	.suspend = tdisc_suspend,
	.resume  = tdisc_resume,
};
#endif

static const struct i2c_device_id tdisc_id[] = {
	{ DEVICE_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, tdisc_id);

static int __devinit tdisc_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int			rc = -1;
	int	x_max, x_min, y_max, y_min, pressure_min, pressure_max;
	struct tdisc_platform_data  *pd;
	struct tdisc_data           *dd;

	
	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_READ_I2C_BLOCK))
		return -ENODEV;

	dd = kzalloc(sizeof *dd, GFP_KERNEL);
	if (!dd) {
		rc = -ENOMEM;
		goto probe_exit;
	}

	i2c_set_clientdata(client, dd);
	dd->clientp = client;
	pd = client->dev.platform_data;
	if (!pd) {
		pr_err("%s: platform data not set \n", __func__);
		rc = -EFAULT;
		goto probe_free_exit;
	}

	dd->pdata = pd;

	dd->tdisc_device = input_allocate_device();
	if (!dd->tdisc_device) {
		rc = -ENOMEM;
		goto probe_free_exit;
	}

	input_set_drvdata(dd->tdisc_device, dd);
	dd->tdisc_device->open       = tdisc_open;
	dd->tdisc_device->close      = tdisc_close;
	dd->tdisc_device->name       = TDISC_NAME;
	dd->tdisc_device->id.bustype = BUS_I2C;
	dd->tdisc_device->id.product = 1;
	dd->tdisc_device->id.version = 1;

	if (pd->tdisc_abs) {
		x_max = pd->tdisc_abs->x_max;
		x_min = pd->tdisc_abs->x_min;
		y_max = pd->tdisc_abs->y_max;
		y_min = pd->tdisc_abs->y_min;
		pressure_max = pd->tdisc_abs->pressure_max;
		pressure_min = pd->tdisc_abs->pressure_min;
	} else {
		x_max = X_MAX;
		x_min = X_MIN;
		y_max = Y_MAX;
		y_min = Y_MIN;
		pressure_max = PRESSURE_MAX;
		pressure_min = PRESSURE_MIN;
	}

	
	input_set_capability(dd->tdisc_device, EV_REL, REL_X);
	input_set_capability(dd->tdisc_device, EV_REL, REL_Y);

	
	input_set_capability(dd->tdisc_device, EV_ABS, ABS_X);
	input_set_capability(dd->tdisc_device, EV_ABS, ABS_Y);
	input_set_capability(dd->tdisc_device, EV_ABS, ABS_PRESSURE);

	input_set_abs_params(dd->tdisc_device, ABS_X, x_min, x_max, 0, 0);
	input_set_abs_params(dd->tdisc_device, ABS_Y, y_min, y_max, 0, 0);
	input_set_abs_params(dd->tdisc_device, ABS_PRESSURE, pressure_min,
							pressure_max, 0, 0);

	
	input_set_capability(dd->tdisc_device, EV_REL, REL_WHEEL);
	input_set_capability(dd->tdisc_device, EV_KEY, KEY_LEFT);
	input_set_capability(dd->tdisc_device, EV_KEY, KEY_RIGHT);
	input_set_capability(dd->tdisc_device, EV_KEY, KEY_UP);
	input_set_capability(dd->tdisc_device, EV_KEY, KEY_DOWN);
	input_set_capability(dd->tdisc_device, EV_KEY, KEY_ENTER);

	
	if (dd->pdata->tdisc_setup != NULL) {
		rc = dd->pdata->tdisc_setup();
		if (rc) {
			pr_err("%s: Setup failed \n", __func__);
			goto probe_unreg_free_exit;
		}
	}

	
	device_init_wakeup(&dd->clientp->dev, dd->pdata->tdisc_wakeup);

	INIT_DELAYED_WORK(&dd->tdisc_work, tdisc_work_f);

	rc = input_register_device(dd->tdisc_device);
	if (rc) {
		pr_err("%s: input register device failed \n", __func__);
		rc = -EINVAL;
		goto probe_register_fail;
	}

	return 0;

probe_register_fail:
	if (dd->pdata->tdisc_release != NULL)
		dd->pdata->tdisc_release();
probe_unreg_free_exit:
	input_free_device(dd->tdisc_device);
probe_free_exit:
	i2c_set_clientdata(client, NULL);
	kfree(dd);
probe_exit:
	return rc;
}

static struct i2c_driver tdisc_driver = {
	.driver = {
		.name   = DRIVER_NAME,
		.owner  = THIS_MODULE,
#ifdef CONFIG_PM
		.pm = &tdisc_pm_ops,
#endif
	},
	.probe   = tdisc_probe,
	.remove  =  __devexit_p(tdisc_remove),
	.id_table = tdisc_id,
};

static int __init tdisc_init(void)
{
	int rc;

	rc = i2c_add_driver(&tdisc_driver);
	if (rc)
		pr_err("%s: i2c add driver failed \n", __func__);
	return rc;
}

static void __exit tdisc_exit(void)
{
	i2c_del_driver(&tdisc_driver);
}

module_init(tdisc_init);
module_exit(tdisc_exit);
