#if 1



#ifdef CONFIG_MACH_LGE
#define LGE_APDS9801
#endif

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#ifdef LGE_APDS9801
#include <linux/input.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <asm/gpio.h>
#include <mach/board_lge.h>
#endif

#define APDS9801_DRV_NAME	"apds9801"
#define DRIVER_VERSION		"1.0.0"



#define APDS9801_SHUTDOWN_REG		0x80
#define APDS9801_PULSE_REG			0x81
#define APDS9801_CONTROL_REG		0x82
#define APDS9801_THRESHOLD_LOW_REG	0x83
#define APDS9801_THRESHOLD_HIGH_REG	0x84
#define APDS9801_DATA_LOW_REG		0x85
#define APDS9801_DATA_HIGH_REG		0x86
#define APDS9801_INTERRUPT_REG		0x87

#ifdef LGE_APDS9801

#define PROX_INPUT_NEAR 0
#define PROX_INPUT_FAR  1



#if 0
#define DETECTION_THRESHOLD_UPPER	430
#define DETECTION_THRESHOLD_LOWER	370
#else
#define DETECTION_THRESHOLD_UPPER	1000
#define DETECTION_THRESHOLD_LOWER	950
#endif


#define Pulse_4p 	0x00
#define Pulse_8p 	0x01
#define Pulse_12p 	0x02
#define Pulse_16p	0x03
#define Pulse_20p 	0x04
#define Pulse_24p 	0x05
#define Pulse_28p 	0x06
#define Pulse_32p 	0x07

#define DutyCycle_12_5 0x00
#define DutyCycle_25_0 0x01
#define DutyCycle_37_5 0x02
#define DutyCycle_50_0 0x03

#define Freq_25Khz 0x00
#define Freq_50Khz 0x01
#define Freq_100Khz 0x02
#define Freq_200Khz 0x03

#define StopMeasure 0x00
#define StartMeasure 0x01

#define LEDCurrent75mA 0x00
#define LEDCurrent100mA 0x01
#define LEDCurrent125mA 0x02
#define LEDCurrent150mA 0x03

#define Interval_5ms 0x00
#define Interval_20ms 0x01
#define Interval_50ms 0x02
#define Interval_125ms 0x03
#define Interval_250ms 0x04
#define Interval_500ms 0x05
#define Interval_1s 0x06
#define Interval_2s 0x07

#define IntpDisable 0x00
#define IntpEOCEn 0x01
#define IntpThresEn 0x02
#define IntpPosStat 0x20
#define IntpNegStat 0x40

#define SoftwareReset 0x10
#define IntpClrEOC 0x20
#define IntpClrThres 0x40
#define IntpClrBoth 0x60

#endif 



struct apds9801_data {
	struct i2c_client *client;
	struct mutex update_lock;

	unsigned int shutdown;
	unsigned int pulse;
	unsigned int control;
	unsigned int threshold;
	unsigned int interrupt;

#ifdef LGE_APDS9801
	unsigned int enable;
	unsigned int vout;          
	unsigned int last_vout;     
	unsigned int irq;			
	unsigned int debounce;		
	
	struct input_dev    *input_dev;		
	struct delayed_work dwork;			
	spinlock_t          lock;				
#endif
};


#ifdef LGE_APDS9801
static struct workqueue_struct *proximity_wq;

#endif



static int apds9801_set_interrupt(struct i2c_client *client, int interrupt_data)
{
	struct apds9801_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, APDS9801_INTERRUPT_REG, interrupt_data);
	mutex_unlock(&data->update_lock);

	data->interrupt = interrupt_data;

	return ret;
}

static int apds9801_set_threshold(struct i2c_client *client, int threshold)
{
	struct apds9801_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, APDS9801_THRESHOLD_LOW_REG, threshold&0xFF);
	ret = i2c_smbus_write_byte_data(client, APDS9801_THRESHOLD_HIGH_REG, (threshold>>8)&0xFF);
	mutex_unlock(&data->update_lock);
	
	data->threshold = threshold;

	return ret;
}

static int apds9801_set_control(struct i2c_client *client, int control)
{
	struct apds9801_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, APDS9801_CONTROL_REG, control);
	mutex_unlock(&data->update_lock);

	data->control = control;

	return ret;
}

static int apds9801_set_pulse(struct i2c_client *client, int pulse)
{
	struct apds9801_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, APDS9801_PULSE_REG, pulse);
	mutex_unlock(&data->update_lock);

	data->pulse = pulse;

	return ret;
}

static int apds9801_set_shutdown(struct i2c_client *client, int shutdown)
{
	struct apds9801_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, APDS9801_SHUTDOWN_REG, shutdown);
	mutex_unlock(&data->update_lock);

	data->shutdown = shutdown;

	return ret;
}

static int apds9801_set_command(struct i2c_client *client, int command)
{
	struct apds9801_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte(client, command);
	mutex_unlock(&data->update_lock);

	return ret;
}



static ssize_t apds9801_store_command(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	int ret;

	ret = apds9801_set_command(client, val);

	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(command, S_IWUSR,
		   NULL, apds9801_store_command);

static ssize_t apds9801_show_shutdown(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", data->shutdown);
}

static ssize_t apds9801_store_shutdown(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	int ret;

	if (val < 0 || val > 1)
		return -EINVAL;

	ret = apds9801_set_shutdown(client, val);

	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(shutdown, S_IWUSR | S_IRUGO,
		   apds9801_show_shutdown, apds9801_store_shutdown);

static ssize_t apds9801_show_pulse(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", data->pulse);
}

static ssize_t apds9801_store_pulse(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	int ret;

	if (data->shutdown == 0)
		return -EBUSY;

	ret = apds9801_set_pulse(client, val);

	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(pulse, S_IWUSR | S_IRUGO,
		   apds9801_show_pulse, apds9801_store_pulse);

static ssize_t apds9801_show_control(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", data->control);
}

static ssize_t apds9801_store_control(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	int ret;

	if (data->shutdown == 0)
		return -EBUSY;

	ret = apds9801_set_control(client, val);

	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(control, S_IWUSR | S_IRUGO,
		   apds9801_show_control, apds9801_store_control);

static ssize_t apds9801_show_threshold(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", data->threshold);
}

static ssize_t apds9801_store_threshold(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	int ret;

	if (data->shutdown == 0)
		return -EBUSY;

	ret = apds9801_set_threshold(client, val);

	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(threshold, S_IWUSR | S_IRUGO,
		   apds9801_show_threshold, apds9801_store_threshold);

static ssize_t apds9801_show_adc(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);
	int adc_low, adc_high, adc_data;

	mutex_lock(&data->update_lock);
	adc_low = i2c_smbus_read_byte_data(client, APDS9801_DATA_LOW_REG);
	adc_high = i2c_smbus_read_byte_data(client, APDS9801_DATA_HIGH_REG);
	mutex_unlock(&data->update_lock);

	adc_data = (adc_high<<8) | adc_low;
	
	return sprintf(buf, "%d\n", adc_data);
}

static DEVICE_ATTR(adc, S_IRUGO,
		   apds9801_show_adc, NULL);

static ssize_t apds9801_show_interrupt(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);
	int interrupt_data;

	mutex_lock(&data->update_lock);
	interrupt_data = i2c_smbus_read_byte_data(client, APDS9801_INTERRUPT_REG);
	mutex_unlock(&data->update_lock);

	return sprintf(buf, "%d\n", interrupt_data);
}

static ssize_t apds9801_store_interrupt(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	int ret;

	if (data->shutdown == 0)
		return -EBUSY;

	ret = apds9801_set_interrupt(client, val);

	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(interrupt, S_IWUSR | S_IRUGO,
		   apds9801_show_interrupt, apds9801_store_interrupt);

#ifdef LGE_APDS9801

static void
apds9801_report_event(struct apds9801_data *data, int state)
{
	input_report_abs(data->input_dev, ABS_DISTANCE, state);
	input_sync(data->input_dev);
	
	data->last_vout = state;
	(data->last_vout) ? printk(KERN_INFO "\nPROX:Far\n") : printk(KERN_INFO "\nPROX:Near\n");
}

static void
apds9801_work_func(struct work_struct *work)
{
	struct apds9801_data *data = container_of(work, struct apds9801_data, dwork.work);
	struct i2c_client *client = data->client;
	int interrupt_data;
	
	disable_irq(data->irq);

	mutex_lock(&data->update_lock);
	interrupt_data = i2c_smbus_read_byte_data(client, APDS9801_INTERRUPT_REG);
	mutex_unlock(&data->update_lock);

	if (interrupt_data & IntpNegStat) {
		
		data->vout = PROX_INPUT_FAR;
		apds9801_set_threshold(client, DETECTION_THRESHOLD_UPPER);	
	}
	else if (interrupt_data & IntpPosStat) {
		
		data->vout = PROX_INPUT_NEAR;
		apds9801_set_threshold(client, DETECTION_THRESHOLD_LOWER);	
	}
	else {
		
		printk(KERN_INFO "PROX: dummy data\n");

		
		apds9801_set_command(data->client, IntpClrBoth);
		enable_irq(data->irq);		
		return;
	}
	
	
	if (data->last_vout != data->vout) {
		apds9801_report_event(data, data->vout);
	}
	
	
	apds9801_set_command(data->client, IntpClrBoth);
	enable_irq(data->irq);

	return;
}


static int apds9801_irq_handler(int irq, void *dev_id)
{
	struct apds9801_data *data = dev_id;
	unsigned long delay;

	spin_lock(&data->lock);
	delay = msecs_to_jiffies(data->debounce);
	queue_delayed_work(proximity_wq, &data->dwork, delay);
	spin_unlock(&data->lock);
	
	return IRQ_HANDLED;
}

static int apds9801_ps_start(struct i2c_client *client)
{
	int err = -1;
	struct apds9801_data *data = i2c_get_clientdata(client);

	
	
	apds9801_set_command(client, SoftwareReset);
	apds9801_set_shutdown(client, 1);
	apds9801_set_command(client, IntpClrBoth);


#if 0
	apds9801_set_pulse(client, (Pulse_4p << 5 | DutyCycle_50_0 << 2 | Freq_50Khz));
	apds9801_set_control(client, (StartMeasure << 5 | LEDCurrent150mA << 3 | Interval_50ms));
#else
    apds9801_set_pulse(client, (Pulse_32p << 5 | DutyCycle_50_0 << 2 | Freq_200Khz));
    apds9801_set_control(client, (StartMeasure << 5 | LEDCurrent150mA << 3 | Interval_50ms));
#endif


	
	apds9801_set_threshold(client, DETECTION_THRESHOLD_UPPER);
	apds9801_set_interrupt(client, IntpThresEn);

	INIT_DELAYED_WORK(&data->dwork, apds9801_work_func);
	proximity_wq = create_singlethread_workqueue("proximity_wq");
	if (!proximity_wq) {
		dev_err(&client->dev, "failed to create singlethread workqueue\n");
		goto err_create_wq;
	}

	
	err = request_irq(data->irq, apds9801_irq_handler, IRQF_TRIGGER_FALLING, 
		"proximity_irq", data);
	if (err < 0) {
		dev_err(&client->dev, "failed to register irq\n");
		goto err_irq_request;
	}
	
	return 0;

err_irq_request:
	destroy_workqueue(proximity_wq);
err_create_wq:
	return err;
}

static int apds9801_ps_stop(struct i2c_client *client)
{
	struct apds9801_data *data = i2c_get_clientdata(client);

	free_irq(data->irq, data);
	destroy_workqueue(proximity_wq);

	apds9801_set_shutdown(client, 0);
	return 0;
}

static ssize_t
apds9801_show_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *pdev = i2c_get_clientdata(client);

	return snprintf(buf, PAGE_SIZE, "%d\n", (pdev->last_vout == PROX_INPUT_NEAR)? PROX_INPUT_NEAR : PROX_INPUT_FAR);
}
static DEVICE_ATTR(show, S_IRUGO | S_IWUSR, apds9801_show_show, NULL);

static ssize_t
apds9801_show_enable(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);
	
	return snprintf(buf, PAGE_SIZE, "%d\n", data->enable);
}

static ssize_t
apds9801_store_enable(struct device *dev, 
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);
	pm_message_t dummy_state; 
	int enable;
	
	dummy_state.event = 0;

	sscanf(buf, "%d", &enable);

	if (enable != 0 && enable != 1) {
		printk(KERN_INFO "Usage: echo [0 | 1] > enable");
		printk(KERN_INFO " 0: disable\n");
		printk(KERN_INFO " 1: enable\n");
		return count;
	}

	if (enable == data->enable) {
		printk(KERN_INFO "mode is already %d\n", data->enable);
		return count;
	}
	else {
		struct proximity_platform_data *pdata = data->client->dev.platform_data;
		data->enable = enable;
		
		if (enable) {
			unsigned long delay;			
			apds9801_ps_start(client);
			
			apds9801_report_event(data, PROX_INPUT_FAR);
			data->last_vout = PROX_INPUT_FAR;
			printk(KERN_INFO "apds9801_ps_start\n");
		}
		else {
			apds9801_ps_stop(client);
			printk(KERN_INFO "apds9801_ps_stop\n");
		}
	}

	return count;
}

static DEVICE_ATTR(enable, S_IWUSR | S_IRUGO,
		   apds9801_show_enable, apds9801_store_enable);

#endif 

static struct attribute *apds9801_attributes[] = {
	&dev_attr_command.attr,
	&dev_attr_shutdown.attr,
	&dev_attr_pulse.attr,
	&dev_attr_control.attr,
	&dev_attr_threshold.attr,
	&dev_attr_adc.attr,
	&dev_attr_interrupt.attr,
#ifdef LGE_APDS9801
	&dev_attr_enable.attr,
	&dev_attr_show.attr,
#endif	
	NULL
};

static const struct attribute_group apds9801_attr_group = {
	.attrs = apds9801_attributes,
};



static int apds9801_init_client(struct i2c_client *client)
{
	struct apds9801_data *data = i2c_get_clientdata(client);
	int err;

	err = apds9801_set_shutdown(client, 0);

	if (err < 0)
		return err;

	mdelay(1);

	mutex_lock(&data->update_lock);
	err = i2c_smbus_read_byte_data(client, APDS9801_SHUTDOWN_REG);
	mutex_unlock(&data->update_lock);

	if (err != 0)
		return -ENODEV;

	data->shutdown = 0;

	return 0;
}



static struct i2c_driver apds9801_driver;
static int __devinit apds9801_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct apds9801_data *data;
	int err = 0;
#ifdef LGE_APDS9801
	struct proximity_platform_data	*pdata;
#endif

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE)) {
		err = -EIO;
		goto exit;
	}

	data = kzalloc(sizeof(struct apds9801_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}
	data->client = client;
	i2c_set_clientdata(client, data);

#ifdef LGE_APDS9801
	pdata = data->client->dev.platform_data;
	if (NULL == pdata) {
		dev_err(&client->dev, "failed to get platform data\n");
		goto exit_kfree;
	}

	data->irq = gpio_to_irq(pdata->irq_num);
	data->debounce = 0;
	data->last_vout = -1;
	
	
	pdata->power(1);
#endif 

	mutex_init(&data->update_lock);

	
	err = apds9801_init_client(client);
	if (err)
		goto exit_kfree;

	
	err = sysfs_create_group(&client->dev.kobj, &apds9801_attr_group);
	if (err)
		goto exit_kfree;

	dev_info(&client->dev, "support ver. %s enabled\n", DRIVER_VERSION);

#ifdef LGE_APDS9801
	
	data->input_dev = input_allocate_device();
	if (NULL == data->input_dev) {
		dev_err(&client->dev, "failed to allocation\n");
		goto exit_kfree;
	}

	
	data->input_dev->name = "proximity";
	

	set_bit(EV_SYN, data->input_dev->evbit); 
	set_bit(EV_ABS, data->input_dev->evbit);
	input_set_abs_params(data->input_dev, ABS_DISTANCE, 0, 1, 0, 0);

	
	err = input_register_device(data->input_dev);
	if (err < 0) {
		dev_err(&client->dev, "failed to register input\n");
		goto err_input_register_device;
	}

	device_init_wakeup(&client->dev, 1);

	spin_lock_init(&data->lock);

#endif 

	return 0;

err_irq_request:
err_input_register_device:
	input_free_device(data->input_dev);
exit_kfree:
	kfree(data);
exit:
	return err;
}

static int __devexit apds9801_remove(struct i2c_client *client)
{
	device_init_wakeup(&client->dev, 0);

	sysfs_remove_group(&client->dev.kobj, &apds9801_attr_group);

	
	apds9801_set_shutdown(client, 0);

	kfree(i2c_get_clientdata(client));

	return 0;
}

#ifdef CONFIG_PM

static int apds9801_suspend(struct device *device)
{
#ifdef LGE_APDS9801
	struct i2c_client *client = i2c_verify_client(device);
	struct apds9801_data *data = i2c_get_clientdata(client);
	struct proximity_platform_data *pdata = client->dev.platform_data;
	
	if (data->enable)
	{
		if (device_may_wakeup(&client->dev))
			enable_irq_wake(data->irq);	
	} else {
		apds9801_set_shutdown(client, 0);	
		pdata->power(0);
	}	
	
	return 0;
#else	
	return apds9801_set_shutdown(client, 0);
#endif	
}

static int apds9801_resume(struct device *device)
{
#ifdef LGE_APDS9801
	struct i2c_client *client = i2c_verify_client(device);
	struct apds9801_data *data = i2c_get_clientdata(client);
	struct proximity_platform_data *pdata = client->dev.platform_data;	

	if (data->enable)
	{
		if (device_may_wakeup(&client->dev))
			disable_irq_wake(data->irq);	
	} else {
		pdata->power(1);
		apds9801_set_shutdown(client, 0);	
	}

	return 0;
#else
	return apds9801_set_shutdown(client, 1);
#endif
}

#else

#define apds9801_suspend	NULL
#define apds9801_resume		NULL

#endif 


void proximity_test_mode_get_value( int *value )
{

}

EXPORT_SYMBOL(proximity_test_mode_get_value);


static const struct i2c_device_id apds9801_id[] = {
	{ "apds9801", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, apds9801_id);

#if defined(CONFIG_PM)
static struct dev_pm_ops apds9801_pm_ops = {
       .suspend = apds9801_suspend,
       .resume = apds9801_resume,
};
#endif

static struct i2c_driver apds9801_driver = {
	.driver = {
		.name	= APDS9801_DRV_NAME,
		.owner	= THIS_MODULE,
#if defined(CONFIG_PM)
		.pm	= &apds9801_pm_ops,
#endif		
	},
	.probe	= apds9801_probe,
	.remove	= __devexit_p(apds9801_remove),
	.id_table = apds9801_id,
};

static int __init apds9801_init(void)
{
	return i2c_add_driver(&apds9801_driver);
}

static void __exit apds9801_exit(void)
{
	i2c_del_driver(&apds9801_driver);
}

MODULE_AUTHOR("Lee Kai Koon <kai-koon.lee@avagotech.com>");
MODULE_DESCRIPTION("APDS9801 ambient light + proximity sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(apds9801_init);
module_exit(apds9801_exit);
#else



#ifdef CONFIG_MACH_LGE
#define LGE_APDS9801
#endif

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#ifdef LGE_APDS9801
#include <linux/input.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <asm/gpio.h>
#include <mach/board_lge.h>
#endif

#define APDS9801_DRV_NAME	"apds9801"
#define DRIVER_VERSION		"1.0.0"



#define APDS9801_SHUTDOWN_REG		0x80
#define APDS9801_PULSE_REG			0x81
#define APDS9801_CONTROL_REG		0x82
#define APDS9801_THRESHOLD_LOW_REG	0x83
#define APDS9801_THRESHOLD_HIGH_REG	0x84
#define APDS9801_DATA_LOW_REG		0x85
#define APDS9801_DATA_HIGH_REG		0x86
#define APDS9801_INTERRUPT_REG		0x87

#ifdef LGE_APDS9801

#define PROX_INPUT_NEAR 0
#define PROX_INPUT_FAR  1


#define DETECTION_THRESHOLD	0x195


#if defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)
#if 0

#define PS_HSYTERESIS	100
#define PS_SW_PERSISTENCE	5
#else
int PS_HSYTERESIS = 100;
int PS_SW_PERSISTENCE = 5;
#endif
#endif


#define Pulse_4p 	0x00
#define Pulse_8p 	0x01
#define Pulse_12p 	0x02
#define Pulse_16p	0x03
#define Pulse_20p 	0x04
#define Pulse_24p 	0x05
#define Pulse_28p 	0x06
#define Pulse_32p 	0x07

#define DutyCycle_12_5 0x00
#define DutyCycle_25_0 0x01
#define DutyCycle_37_5 0x02
#define DutyCycle_50_0 0x03

#define Freq_25Khz 0x00
#define Freq_50Khz 0x01
#define Freq_100Khz 0x02
#define Freq_200Khz 0x03

#define StopMeasure 0x00
#define StartMeasure 0x01

#define LEDCurrent75mA 0x00
#define LEDCurrent100mA 0x01
#define LEDCurrent125mA 0x02
#define LEDCurrent150mA 0x03

#define Interval_5ms 0x00
#define Interval_20ms 0x01
#define Interval_50ms 0x02
#define Interval_125ms 0x03
#define Interval_250ms 0x04
#define Interval_500ms 0x05
#define Interval_1s 0x06
#define Interval_2s 0x07

#define IntpDisable 0x00
#define IntpEOCEn 0x01
#define IntpThresEn 0x02
#define IntpPosStat 0x20
#define IntpNegStat 0x40

#define SoftwareReset 0x10
#define IntpClrEOC 0x20
#define IntpClrThres 0x40
#define IntpClrBoth 0x60

#endif 



struct apds9801_data {
	struct i2c_client *client;
	struct mutex update_lock;

	unsigned int shutdown;
	unsigned int pulse;
	unsigned int control;
	unsigned int threshold;
	unsigned int interrupt;

#ifdef LGE_APDS9801
	unsigned int enable;
	unsigned int vout;          
	unsigned int last_vout;     
	unsigned int irq;			
	unsigned int debounce;		
	
	struct input_dev    *input_dev;		
	struct delayed_work dwork;			
	spinlock_t          lock;				


#if defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)
	unsigned int positive_persistence;	
	unsigned int negative_persistence;	
	unsigned int detection_threshold;
#endif

#endif
};


#ifdef LGE_APDS9801
static struct workqueue_struct *proximity_wq;

#endif



static int apds9801_set_interrupt(struct i2c_client *client, int interrupt_data)
{
	struct apds9801_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, APDS9801_INTERRUPT_REG, interrupt_data);
	mutex_unlock(&data->update_lock);

	data->interrupt = interrupt_data;

	return ret;
}

static int apds9801_set_threshold(struct i2c_client *client, int threshold)
{
	struct apds9801_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, APDS9801_THRESHOLD_LOW_REG, threshold&0xFF);
	ret = i2c_smbus_write_byte_data(client, APDS9801_THRESHOLD_HIGH_REG, (threshold>>8)&0xFF);
	mutex_unlock(&data->update_lock);
	
	data->threshold = threshold;


#if defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)
	data->detection_threshold = threshold;	
#endif


	return ret;
}

static int apds9801_set_control(struct i2c_client *client, int control)
{
	struct apds9801_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, APDS9801_CONTROL_REG, control);
	mutex_unlock(&data->update_lock);

	data->control = control;

	return ret;
}

static int apds9801_set_pulse(struct i2c_client *client, int pulse)
{
	struct apds9801_data *data = i2c_get_clientdata(client);
	int ret;
	
	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, APDS9801_PULSE_REG, pulse);
	mutex_unlock(&data->update_lock);

	data->pulse = pulse;

	return ret;
}

static int apds9801_set_shutdown(struct i2c_client *client, int shutdown)
{
	struct apds9801_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte_data(client, APDS9801_SHUTDOWN_REG, shutdown);
	mutex_unlock(&data->update_lock);

	data->shutdown = shutdown;

	return ret;
}

static int apds9801_set_command(struct i2c_client *client, int command)
{
	struct apds9801_data *data = i2c_get_clientdata(client);
	int ret;

	mutex_lock(&data->update_lock);
	ret = i2c_smbus_write_byte(client, command);
	mutex_unlock(&data->update_lock);

	return ret;
}



static ssize_t apds9801_store_command(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	int ret;

	ret = apds9801_set_command(client, val);

	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(command, S_IWUSR,
		   NULL, apds9801_store_command);

static ssize_t apds9801_show_shutdown(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", data->shutdown);
}

static ssize_t apds9801_store_shutdown(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	int ret;

	if (val < 0 || val > 1)
		return -EINVAL;

	ret = apds9801_set_shutdown(client, val);

	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(shutdown, S_IWUSR | S_IRUGO,
		   apds9801_show_shutdown, apds9801_store_shutdown);

static ssize_t apds9801_show_pulse(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", data->pulse);
}

static ssize_t apds9801_store_pulse(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	int ret;

	if (data->shutdown == 0)
		return -EBUSY;

	ret = apds9801_set_pulse(client, val);

	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(pulse, S_IWUSR | S_IRUGO,
		   apds9801_show_pulse, apds9801_store_pulse);

static ssize_t apds9801_show_control(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", data->control);
}

static ssize_t apds9801_store_control(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	int ret;

	if (data->shutdown == 0)
		return -EBUSY;

	ret = apds9801_set_control(client, val);

	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(control, S_IWUSR | S_IRUGO,
		   apds9801_show_control, apds9801_store_control);

static ssize_t apds9801_show_threshold(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", data->threshold);
}

static ssize_t apds9801_store_threshold(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	int ret;

	if (data->shutdown == 0)
		return -EBUSY;

	ret = apds9801_set_threshold(client, val);

	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(threshold, S_IWUSR | S_IRUGO,
		   apds9801_show_threshold, apds9801_store_threshold);

static ssize_t apds9801_show_adc(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);
	int adc_low, adc_high, adc_data;

	mutex_lock(&data->update_lock);
	adc_low = i2c_smbus_read_byte_data(client, APDS9801_DATA_LOW_REG);
	adc_high = i2c_smbus_read_byte_data(client, APDS9801_DATA_HIGH_REG);
	mutex_unlock(&data->update_lock);

	adc_data = (adc_high<<8) | adc_low;
	
	return sprintf(buf, "%d\n", adc_data);
}

static DEVICE_ATTR(adc, S_IRUGO,
		   apds9801_show_adc, NULL);

static ssize_t apds9801_show_interrupt(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);
	int interrupt_data;

	mutex_lock(&data->update_lock);
	interrupt_data = i2c_smbus_read_byte_data(client, APDS9801_INTERRUPT_REG);
	mutex_unlock(&data->update_lock);

	return sprintf(buf, "%d\n", interrupt_data);
}

static ssize_t apds9801_store_interrupt(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	int ret;

	if (data->shutdown == 0)
		return -EBUSY;

	ret = apds9801_set_interrupt(client, val);

	if (ret < 0)
		return ret;

	return count;
}

static DEVICE_ATTR(interrupt, S_IWUSR | S_IRUGO,
		   apds9801_show_interrupt, apds9801_store_interrupt);

#ifdef LGE_APDS9801

static void
apds9801_report_event(struct apds9801_data *data, int state)
{
	input_report_abs(data->input_dev, ABS_DISTANCE, state);
	input_sync(data->input_dev);
	
	data->last_vout = state;
	(data->last_vout) ? printk(KERN_INFO "\nPROX:Far\n") : printk(KERN_INFO "\nPROX:Near\n");
}

static void
apds9801_work_func(struct work_struct *work)
{
	struct apds9801_data *data = container_of(work, struct apds9801_data, dwork.work);
	struct i2c_client *client = data->client;
	int interrupt_data;

#if defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)
	int adc_low, adc_high, adc_data;
	unsigned long delay;
#endif

	
	disable_irq(data->irq);

	mutex_lock(&data->update_lock);
	interrupt_data = i2c_smbus_read_byte_data(client, APDS9801_INTERRUPT_REG);
	mutex_unlock(&data->update_lock);

	if (interrupt_data & IntpNegStat) {
		


#if defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)
			
			data->negative_persistence++;
			data->positive_persistence=0;	
#else
			data->vout = PROX_INPUT_FAR;
#endif

	}
	else if (interrupt_data & IntpPosStat) {
		


#if defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)
		
		data->positive_persistence++;
		data->negative_persistence = 0;	
#else
		data->vout = PROX_INPUT_NEAR;
#endif

	}
	else {
		


#if defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)
		mutex_lock(&data->update_lock);
		adc_low = i2c_smbus_read_byte_data(client, APDS9801_DATA_LOW_REG);
		adc_high = i2c_smbus_read_byte_data(client, APDS9801_DATA_HIGH_REG);
		mutex_unlock(&data->update_lock);

		adc_data = (adc_high<<8) | adc_low;

		if (data->positive_persistence) {
			

			if (adc_data > data->detection_threshold) {
				data->positive_persistence++;
				if (data->positive_persistence >= PS_SW_PERSISTENCE) {

					
					mutex_lock(&data->update_lock);
					i2c_smbus_write_byte_data(client, APDS9801_THRESHOLD_LOW_REG, (data->threshold-PS_HSYTERESIS)&0xFF);
					i2c_smbus_write_byte_data(client, APDS9801_THRESHOLD_HIGH_REG, ((data->threshold-PS_HSYTERESIS)>>8)&0xFF);
					mutex_unlock(&data->update_lock);

					data->detection_threshold = data->threshold-PS_HSYTERESIS;
					data->positive_persistence = 0;

					data->vout = PROX_INPUT_NEAR;
					if (data->last_vout != data->vout) {
						apds9801_report_event(data, data->vout);
					}
				}
			}
			else {
				
				data->positive_persistence = 0;
			}

		}
		else if (data->negative_persistence) {
			if (adc_data < data->detection_threshold) {
				data->negative_persistence++;
				if (data->negative_persistence >= PS_SW_PERSISTENCE) {

					
					mutex_lock(&data->update_lock);
					i2c_smbus_write_byte_data(client, APDS9801_THRESHOLD_LOW_REG, data->threshold&0xFF);
					i2c_smbus_write_byte_data(client, APDS9801_THRESHOLD_HIGH_REG, (data->threshold>>8)&0xFF);
					mutex_unlock(&data->update_lock);

					data->detection_threshold = data->threshold;
					data->negative_persistence = 0;

					data->vout = PROX_INPUT_FAR;
					if (data->last_vout != data->vout) {
						apds9801_report_event(data, data->vout);
					}
				}
			}
			else {
				
				data->negative_persistence = 0;
			}
		}
#else
		printk(KERN_INFO "PROX: dummy data\n");

		
		apds9801_set_command(data->client, IntpClrBoth);
		enable_irq(data->irq);		
		return;
#endif

	}
	

#if defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)
#else
	
	if (data->last_vout != data->vout) {
		apds9801_report_event(data, data->vout);
	}
#endif

	
	
	apds9801_set_command(data->client, IntpClrBoth);
	enable_irq(data->irq);


#if defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)
	if (data->positive_persistence || data->negative_persistence) {
		delay = msecs_to_jiffies(50);
		queue_delayed_work(proximity_wq, &data->dwork, delay);
	}
#endif


	return;
}


static int apds9801_irq_handler(int irq, void *dev_id)
{
	struct apds9801_data *data = dev_id;
	unsigned long delay;

	spin_lock(&data->lock);
	delay = msecs_to_jiffies(data->debounce);


#if defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)
	

	
#endif


	queue_delayed_work(proximity_wq, &data->dwork, delay);
	spin_unlock(&data->lock);
	
	return IRQ_HANDLED;
}

static int apds9801_ps_start(struct i2c_client *client)
{
	int err = -1;
	struct apds9801_data *data = i2c_get_clientdata(client);

	
	
	apds9801_set_command(client, SoftwareReset);
	apds9801_set_shutdown(client, 1);
	apds9801_set_command(client, IntpClrBoth);
	apds9801_set_pulse(client, (Pulse_20p << 5 | DutyCycle_25_0 << 2 | Freq_100Khz));
	apds9801_set_control(client, (StartMeasure << 5 | LEDCurrent100mA << 3 | Interval_50ms));

	
	apds9801_set_threshold(client, DETECTION_THRESHOLD);
	apds9801_set_interrupt(client, IntpThresEn);

	INIT_DELAYED_WORK(&data->dwork, apds9801_work_func);
	proximity_wq = create_singlethread_workqueue("proximity_wq");
	if (!proximity_wq) {
		dev_err(&client->dev, "failed to create singlethread workqueue\n");
		goto err_create_wq;
	}

	
	err = request_irq(data->irq, apds9801_irq_handler, IRQF_TRIGGER_FALLING, 
		"proximity_irq", data);
	if (err < 0) {
		dev_err(&client->dev, "failed to register irq\n");
		goto err_irq_request;
	}
	
	return 0;

err_irq_request:
	destroy_workqueue(proximity_wq);
err_create_wq:
	return err;
}

static int apds9801_ps_stop(struct i2c_client *client)
{
	struct apds9801_data *data = i2c_get_clientdata(client);

	free_irq(data->irq, data);
	destroy_workqueue(proximity_wq);

	apds9801_set_shutdown(client, 0);
	return 0;
}

static ssize_t
apds9801_show_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *pdev = i2c_get_clientdata(client);

	return snprintf(buf, PAGE_SIZE, "%d\n", (pdev->last_vout == PROX_INPUT_NEAR)? PROX_INPUT_NEAR : PROX_INPUT_FAR);
}
static DEVICE_ATTR(show, S_IRUGO | S_IWUSR, apds9801_show_show, NULL);

static ssize_t
apds9801_show_enable(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);
	
	return snprintf(buf, PAGE_SIZE, "%d\n", data->enable);
}

static ssize_t
apds9801_store_enable(struct device *dev, 
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct apds9801_data *data = i2c_get_clientdata(client);
	pm_message_t dummy_state; 
	int enable;
	
	dummy_state.event = 0;

	sscanf(buf, "%d", &enable);

	if (enable != 0 && enable != 1) {
		printk(KERN_INFO "Usage: echo [0 | 1] > enable");
		printk(KERN_INFO " 0: disable\n");
		printk(KERN_INFO " 1: enable\n");
		return count;
	}

	if (enable == data->enable) {
		printk(KERN_INFO "mode is already %d\n", data->enable);
		return count;
	}
	else {
		struct proximity_platform_data *pdata = data->client->dev.platform_data;
		data->enable = enable;
		
		if (enable) {
			unsigned long delay;			


#if defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)
			data->positive_persistence = 0;	
			data->negative_persistence = 0;	
#endif


			apds9801_ps_start(client);

			
			apds9801_report_event(data, PROX_INPUT_FAR);
			data->last_vout = PROX_INPUT_FAR;
			printk(KERN_INFO "apds9801_ps_start\n");
		}
		else {
			apds9801_ps_stop(client);
			printk(KERN_INFO "apds9801_ps_stop\n");
		}
	}

	return count;
}

static DEVICE_ATTR(enable, S_IWUSR | S_IRUGO,
		   apds9801_show_enable, apds9801_store_enable);

#endif 


#if defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)
static ssize_t
apds9801_show_PS_HSYTERESIS(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", PS_HSYTERESIS);
}

static ssize_t
apds9801_store_PS_HSYTERESIS(struct device *dev, 
		struct device_attribute *attr, const char *buf, size_t count)
{
	int tmp_ps_hsyteresis;

	sscanf(buf, "%d", &tmp_ps_hsyteresis);
	PS_HSYTERESIS = tmp_ps_hsyteresis;

	printk(KERN_INFO "PS_HSYTERESIS ----------------------> %d\n", PS_HSYTERESIS);
	return count;
}

static DEVICE_ATTR(PS_HSYTERESIS, S_IWUSR | S_IRUGO,
		   apds9801_show_PS_HSYTERESIS, apds9801_store_PS_HSYTERESIS);

static ssize_t
apds9801_show_PS_SW_PERSISTENCE(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", PS_SW_PERSISTENCE);
}

static ssize_t
apds9801_store_PS_SW_PERSISTENCE(struct device *dev, 
		struct device_attribute *attr, const char *buf, size_t count)
{
	int tmp_ps_sw_persistence;

	sscanf(buf, "%d", &tmp_ps_sw_persistence);
	PS_SW_PERSISTENCE = tmp_ps_sw_persistence;

	printk(KERN_INFO "PS_SW_PERSISTENCE ----------------------> %d\n", PS_SW_PERSISTENCE);
	return count;
}

static DEVICE_ATTR(PS_SW_PERSISTENCE, S_IWUSR | S_IRUGO,
		   apds9801_show_PS_SW_PERSISTENCE, apds9801_store_PS_SW_PERSISTENCE);
#endif


static struct attribute *apds9801_attributes[] = {
	&dev_attr_command.attr,
	&dev_attr_shutdown.attr,
	&dev_attr_pulse.attr,
	&dev_attr_control.attr,
	&dev_attr_threshold.attr,
	&dev_attr_adc.attr,
	&dev_attr_interrupt.attr,
#ifdef LGE_APDS9801
	&dev_attr_enable.attr,
	&dev_attr_show.attr,
#endif	

#if defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)
	&dev_attr_PS_HSYTERESIS.attr,
	&dev_attr_PS_SW_PERSISTENCE.attr,
#endif

	NULL
};

static const struct attribute_group apds9801_attr_group = {
	.attrs = apds9801_attributes,
};



static int apds9801_init_client(struct i2c_client *client)
{
	struct apds9801_data *data = i2c_get_clientdata(client);
	int err;

	err = apds9801_set_shutdown(client, 0);

	if (err < 0)
		return err;

	mdelay(1);

	mutex_lock(&data->update_lock);
	err = i2c_smbus_read_byte_data(client, APDS9801_SHUTDOWN_REG);
	mutex_unlock(&data->update_lock);

	if (err != 0)
		return -ENODEV;

	data->shutdown = 0;

	return 0;
}



static struct i2c_driver apds9801_driver;
static int __devinit apds9801_probe(struct i2c_client *client,
				   const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct apds9801_data *data;
	int err = 0;
#ifdef LGE_APDS9801
	struct proximity_platform_data	*pdata;
#endif

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE)) {
		err = -EIO;
		goto exit;
	}

	data = kzalloc(sizeof(struct apds9801_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}
	data->client = client;
	i2c_set_clientdata(client, data);

#ifdef LGE_APDS9801
	pdata = data->client->dev.platform_data;
	if (NULL == pdata) {
		dev_err(&client->dev, "failed to get platform data\n");
		goto exit_kfree;
	}

	data->irq = gpio_to_irq(pdata->irq_num);
	data->debounce = 0;
	data->last_vout = -1;
	
	
	pdata->power(1);
#endif 

	mutex_init(&data->update_lock);

	
	err = apds9801_init_client(client);
	if (err)
		goto exit_kfree;

	
	err = sysfs_create_group(&client->dev.kobj, &apds9801_attr_group);
	if (err)
		goto exit_kfree;

	dev_info(&client->dev, "support ver. %s enabled\n", DRIVER_VERSION);

#ifdef LGE_APDS9801
	
	data->input_dev = input_allocate_device();
	if (NULL == data->input_dev) {
		dev_err(&client->dev, "failed to allocation\n");
		goto exit_kfree;
	}

	
	data->input_dev->name = "proximity";
	

	set_bit(EV_SYN, data->input_dev->evbit); 
	set_bit(EV_ABS, data->input_dev->evbit);
	input_set_abs_params(data->input_dev, ABS_DISTANCE, 0, 1, 0, 0);

	
	err = input_register_device(data->input_dev);
	if (err < 0) {
		dev_err(&client->dev, "failed to register input\n");
		goto err_input_register_device;
	}

	device_init_wakeup(&client->dev, 1);

	spin_lock_init(&data->lock);

#endif 

	return 0;

err_irq_request:
err_input_register_device:
	input_free_device(data->input_dev);
exit_kfree:
	kfree(data);
exit:
	return err;
}

static int __devexit apds9801_remove(struct i2c_client *client)
{
	device_init_wakeup(&client->dev, 0);

	sysfs_remove_group(&client->dev.kobj, &apds9801_attr_group);

	
	apds9801_set_shutdown(client, 0);

	kfree(i2c_get_clientdata(client));

	return 0;
}

#ifdef CONFIG_PM

static int apds9801_suspend(struct device *device)
{
#ifdef LGE_APDS9801
	struct i2c_client *client = i2c_verify_client(device);
	struct apds9801_data *data = i2c_get_clientdata(client);
	struct proximity_platform_data *pdata = client->dev.platform_data;
	
	if (data->enable)
	{
		if (device_may_wakeup(&client->dev))
			enable_irq_wake(data->irq);	
	} else {
		apds9801_set_shutdown(client, 0);	
		pdata->power(0);
	}	
	
	return 0;
#else	
	return apds9801_set_shutdown(client, 0);
#endif	
}

static int apds9801_resume(struct device *device)
{
#ifdef LGE_APDS9801
	struct i2c_client *client = i2c_verify_client(device);
	struct apds9801_data *data = i2c_get_clientdata(client);
	struct proximity_platform_data *pdata = client->dev.platform_data;	

	if (data->enable)
	{
		if (device_may_wakeup(&client->dev))
			disable_irq_wake(data->irq);	
	} else {
		pdata->power(1);
		apds9801_set_shutdown(client, 0);	
	}

	return 0;
#else
	return apds9801_set_shutdown(client, 1);
#endif
}

#else

#define apds9801_suspend	NULL
#define apds9801_resume		NULL

#endif 


void proximity_test_mode_get_value( int *value )
{

}

EXPORT_SYMBOL(proximity_test_mode_get_value);


static const struct i2c_device_id apds9801_id[] = {
	{ "apds9801", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, apds9801_id);

#if defined(CONFIG_PM)
static struct dev_pm_ops apds9801_pm_ops = {
       .suspend = apds9801_suspend,
       .resume = apds9801_resume,
};
#endif

static struct i2c_driver apds9801_driver = {
	.driver = {
		.name	= APDS9801_DRV_NAME,
		.owner	= THIS_MODULE,
#if defined(CONFIG_PM)
		.pm	= &apds9801_pm_ops,
#endif		
	},
	.probe	= apds9801_probe,
	.remove	= __devexit_p(apds9801_remove),
	.id_table = apds9801_id,
};

static int __init apds9801_init(void)
{
	return i2c_add_driver(&apds9801_driver);
}

static void __exit apds9801_exit(void)
{
	i2c_del_driver(&apds9801_driver);
}

MODULE_AUTHOR("Lee Kai Koon <kai-koon.lee@avagotech.com>");
MODULE_DESCRIPTION("APDS9801 ambient light + proximity sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(apds9801_init);
module_exit(apds9801_exit);
#endif
