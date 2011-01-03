

#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <asm/gpio.h>
#include <mach/vreg.h>
#include <linux/wakelock.h>
#include <mach/msm_i2ckbd.h>
#include <linux/spinlock.h>

#include <mach/board_lge.h>

#define PROXIMITY_DEBUG_PRINT	(1)
#define PROXIMITY_ERROR_PRINT	(1)


enum {
	GP2AP_DEBUG_ERR_CHECK		= 1U << 0,
	GP2AP_DEBUG_USER_ERROR		= 1U << 1,
	GP2AP_DEBUG_FUNC_TRACE		= 1U << 2,
	GP2AP_DEBUG_DEV_STATUS		= 1U << 3,
	GP2AP_DEBUG_DEV_DEBOUNCE	= 1U << 4,
	GP2AP_DEBUG_GEN_INFO		= 1U << 5,
	GP2AP_DEBUG_INTR_INFO		= 1U << 6,
	GP2AP_DEBUG_INTR_DELAY		= 1U << 7,
};

static unsigned int gp2ap_debug_mask = GP2AP_DEBUG_DEV_STATUS | \
					GP2AP_DEBUG_DEV_DEBOUNCE;

module_param_named(debug_mask, gp2ap_debug_mask, int,
		S_IRUGO | S_IWUSR | S_IWGRP);

#if defined(PROXIMITY_DEBUG_PRINT)
#define PROXD(fmt, args...) \
			printk(KERN_INFO "D[%-18s:%5d]" \
				fmt, __FUNCTION__, __LINE__, ##args);
#else
#define PROXD(fmt, args...)	{};
#endif

#if defined(PROXIMITY_ERROR_PRINT)
#define PROXE(fmt, args...) \
			printk(KERN_ERR "E[%-18s:%5d]" \
				fmt, __FUNCTION__, __LINE__, ##args);
#else
#define PROXD(fmt, args...)	{};
#endif

#define GP2AP_NO_INTCLEAR	(0)
#define GP2AP_INTCLEAR		(1 << 7)























#define GP2AP_REG_PROX			(0x00)
#define GP2AP_REG_GAIN			(0x01)
#define GP2AP_REG_HYS			(0x02)
#define GP2AP_REG_CYCLE			(0x03)
#define GP2AP_REG_OPMOD			(0x04)
#define GP2AP_REG_CON			(0x06)

#define GP2AP_ASD_SHIFT(x)		(x << 4)
#define PROX_SENSOR_DETECT_N	(0)

#define	PROX_OPMODE_A		0x0
#define	PROX_OPMODE_B1	0x1
#define	PROX_OPMODE_B2	0x2


struct proximity_gp2ap_device {
	struct i2c_client		*client;		
	struct input_dev		*input_dev;		
	struct delayed_work		dwork;			
	int	irq;			
	int	vout_gpio;		
	int	vout_level;		
	int	sw_mode;		
	int	methods;		
	int	last_vout;		
	int	debounce;		
	u8	cycle;			
	u8	asd;			
	spinlock_t			lock;			
	int op_mode;	
	u8	reg_backup[7];	
};

struct detection_cycle {
	u8 val;
	unsigned char *resp_time;
};

static struct detection_cycle gp2ap_cycle_table[] = {
	{0x04, "8ms"},
	{0x0C, "16ms"},
	{0x14, "32ms"},
	{0x1C, "64ms"},
	{0x24, "128ms"},
	{0x2C, "256ms"},
	{0x34, "512ms"},
	{0x3C, "1024ms"},
};

static struct proximity_gp2ap_device *gp2ap_pdev = NULL;
static struct workqueue_struct *proximity_wq;

enum gp2ap_dev_mode {
	PROX_TMODE_NORMAL = 0,
	PROX_TMODE_INTR,
};

enum gp2ap_dev_status {
	PROX_STAT_SHUTDOWN = 0,
	PROX_STAT_OPERATING,
};

enum gp2ap_input_event {
	PROX_INPUT_NEAR = 0,
	PROX_INPUT_FAR,
};

struct proxi_timestamp {
	u64 start;
	u64 end;
	u64 result_t;
	unsigned long rem;
	char ready;
};

static struct proxi_timestamp time_result;





static s32
prox_i2c_write(u8 addr, u8 value, u8 intclr)
{
	int ret;

	if (GP2AP_DEBUG_ERR_CHECK & gp2ap_debug_mask) {
		if (gp2ap_pdev == NULL) {
			PROXE("gp2ap device is null\n");
			return -1;
		}
	}

	ret = i2c_smbus_write_byte_data(gp2ap_pdev->client, addr | intclr, value);
	if (GP2AP_DEBUG_ERR_CHECK & gp2ap_debug_mask) {
		if (ret != 0) {
			PROXE("caddr(0x%x),addr(%u),ret(%d)\n", gp2ap_pdev->client->addr, addr, ret);
			return ret;
		}
	}

	gp2ap_pdev->reg_backup[addr] = value;

	if (GP2AP_DEBUG_GEN_INFO & gp2ap_debug_mask)
		PROXD("addr(%u),val(%u),intclr(%u)\n", addr, value, intclr);

	return ret;
}

static s32
prox_i2c_read(u8 addr, u8 intclr)
{
	int ret = 0;

	if (GP2AP_DEBUG_ERR_CHECK & gp2ap_debug_mask) {
		if (gp2ap_pdev == NULL) {
			PROXE("gp2ap device is null\n");
			return -1;
		}
	}

	if(addr == 0)
	{
		ret = i2c_smbus_read_word_data(gp2ap_pdev->client, addr | intclr);
		if (GP2AP_DEBUG_ERR_CHECK & gp2ap_debug_mask) {
			if (ret < 0) {
				PROXE("caddr(0x%x), intclr(%d)\n", gp2ap_pdev->client->addr, intclr);
				return -1;
			}
		}
	}
	else
	{
		ret = gp2ap_pdev->reg_backup[addr];
	}

	if (GP2AP_DEBUG_GEN_INFO & gp2ap_debug_mask)
		PROXD("addr(%u),val(0x%08x),intclr(%u)\n", addr, ret, intclr);

	return ret;
}

static int
gp2ap_device_initialise(void)
{
	s32 ret = 0;
	u8 opmod = 0;
	u8 hys = 0;

	if (GP2AP_DEBUG_FUNC_TRACE & gp2ap_debug_mask)
		PROXD("entry\n");

	if (GP2AP_DEBUG_ERR_CHECK & gp2ap_debug_mask) {
		if (gp2ap_pdev == NULL) {
			PROXE("gp2ap device is null\n");
			return -1;
		}
	}

	ret = prox_i2c_write(GP2AP_REG_CON, 0x18, GP2AP_NO_INTCLEAR);
	if (ret < 0)
		goto end_device_init;

	ret = prox_i2c_write(GP2AP_REG_GAIN, 0x08, GP2AP_NO_INTCLEAR);
	if (ret < 0)
		goto end_device_init;

	if(gp2ap_pdev->op_mode == PROX_OPMODE_A)
		hys = 0xC2;
	else if(gp2ap_pdev->op_mode == PROX_OPMODE_B1)
		hys = 0x40;
	else	
		hys = 0x20;

	ret = prox_i2c_write(GP2AP_REG_HYS, hys, GP2AP_NO_INTCLEAR);
	if (ret < 0)
		goto end_device_init;

	ret = prox_i2c_write(GP2AP_REG_CYCLE, gp2ap_cycle_table[gp2ap_pdev->cycle].val, GP2AP_NO_INTCLEAR);
	if (ret < 0)
		goto end_device_init;

	if (gp2ap_pdev->methods)  	
		opmod = (u8)(GP2AP_ASD_SHIFT(gp2ap_pdev->asd) | 0x03);
	else					
		opmod = (u8)(GP2AP_ASD_SHIFT(gp2ap_pdev->asd) | 0x01);

	ret = prox_i2c_write(GP2AP_REG_OPMOD, opmod, GP2AP_NO_INTCLEAR);
	if (ret < 0)
		goto end_device_init;

	ret = prox_i2c_write(GP2AP_REG_CON, 0x00, GP2AP_NO_INTCLEAR);
	if (ret < 0)
		goto end_device_init;

	if (GP2AP_DEBUG_FUNC_TRACE & gp2ap_debug_mask)
		PROXD("exit\n");

 	return ret;

end_device_init:
	PROXE("failed to initailise\n");
	return ret;

}


static irqreturn_t gp2ap_irq_handler(int irq, void *dev_id)
{
	struct proximity_gp2ap_device *pdev = dev_id;
	unsigned long delay;

	if (GP2AP_DEBUG_INTR_DELAY & gp2ap_debug_mask)
	{
		time_result.ready = 1;
		time_result.start= 0;
		time_result.end = 0;
		time_result.result_t  = 0;
		time_result.rem = 0;
		time_result.start = cpu_clock(smp_processor_id());
	}

	spin_lock(&pdev->lock);

	if (GP2AP_DEBUG_INTR_INFO & gp2ap_debug_mask)
		PROXD("\n");

	if (pdev->methods)	
		pdev->vout_level = gpio_get_value(pdev->vout_gpio);

	delay = msecs_to_jiffies(pdev->debounce);
	queue_delayed_work(proximity_wq, &pdev->dwork, delay);

	spin_unlock(&pdev->lock);

	return IRQ_HANDLED;
}

static void
gp2ap_report_event(int state)
{
	int input_state;

	if (GP2AP_DEBUG_FUNC_TRACE & gp2ap_debug_mask)
		PROXD("entry\n");

	input_state = (state == PROX_SENSOR_DETECT_N) ? PROX_INPUT_FAR : PROX_INPUT_NEAR;

	input_report_abs(gp2ap_pdev->input_dev, ABS_DISTANCE, input_state);
	input_sync(gp2ap_pdev->input_dev);

	if (GP2AP_DEBUG_INTR_DELAY & gp2ap_debug_mask)
	{
		if(time_result.ready == 1)
		{
			time_result.end = cpu_clock(smp_processor_id());
			time_result.result_t  = time_result.end -time_result.start;
			time_result.rem = do_div(time_result.result_t , 1000000000);
			PROXD("Proximity interrupt BSP delay time = %2lu.%06lu\n", (unsigned long)time_result.result_t, time_result.rem/1000);
			time_result.ready = 0;
		}
	}

	gp2ap_pdev->last_vout = !input_state;

	if (GP2AP_DEBUG_DEV_STATUS & gp2ap_debug_mask)
		(gp2ap_pdev->last_vout) ? printk(KERN_INFO  "\nPROX:Near\n") : printk(KERN_INFO "\nPROX:Far\n");

	if (GP2AP_DEBUG_FUNC_TRACE & gp2ap_debug_mask)
		PROXD("exit\n");
}

static void
gp2ap_work_func(struct work_struct *work)
{
	struct proximity_gp2ap_device *dev = container_of(work, struct proximity_gp2ap_device, dwork.work);
	struct proximity_platform_data* proxi_pdata = dev->client->dev.platform_data;

	int vo_data;

	if (GP2AP_DEBUG_FUNC_TRACE & gp2ap_debug_mask)
		PROXD("entry\n");

	if (dev->methods) {  	
		
		disable_irq(dev->irq);
		vo_data = prox_i2c_read(GP2AP_REG_PROX, GP2AP_NO_INTCLEAR);
		vo_data = (vo_data >> 8) & 0x1;
	} else { 
		if(gpio_get_value(proxi_pdata->irq_num) == 1) 
			vo_data = PROX_SENSOR_DETECT_N;
		else
			vo_data = !PROX_SENSOR_DETECT_N;
	}

	
	if (dev->last_vout == vo_data) {
		if (GP2AP_DEBUG_DEV_DEBOUNCE & gp2ap_debug_mask)
			PROXD("not change state by debounce\n");

		if (dev->methods) {	
			goto clear_interrupt;
		}
		goto work_func_end;
	}
	gp2ap_report_event(vo_data);

clear_interrupt:
	if (dev->methods) {	
		if (dev->vout_level == 0) {
			if(PROX_OPMODE_B1) {
				(vo_data) ? prox_i2c_write(GP2AP_REG_HYS, 0x20, GP2AP_NO_INTCLEAR) : \
	        	            prox_i2c_write(GP2AP_REG_HYS, 0x40, GP2AP_NO_INTCLEAR);
			}
			else	
			{
				(vo_data) ? prox_i2c_write(GP2AP_REG_HYS, 0x00, GP2AP_NO_INTCLEAR) : \
	        	            prox_i2c_write(GP2AP_REG_HYS, 0x20, GP2AP_NO_INTCLEAR);
			}
			prox_i2c_write(GP2AP_REG_CON, 0x18, GP2AP_NO_INTCLEAR);
		}
		enable_irq(dev->irq);
		prox_i2c_write(GP2AP_REG_CON, 0x00, GP2AP_INTCLEAR);
	}

work_func_end:
	if (GP2AP_DEBUG_FUNC_TRACE & gp2ap_debug_mask)
		PROXD("exit\n");

	return;
}

static int gp2ap_suspend(struct i2c_client *i2c_dev, pm_message_t state);
static int gp2ap_resume(struct i2c_client *i2c_dev);





static ssize_t
gp2ap_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct proximity_gp2ap_device *pdev = i2c_get_clientdata(client);

	return snprintf(buf, PAGE_SIZE, "%d\n", !(pdev->last_vout));
}

static ssize_t
gp2ap_method_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct proximity_gp2ap_device *pdev = i2c_get_clientdata(client);
	unsigned char *string;

	string = (pdev->methods) ? "interrupt" : "normal";

	return snprintf(buf, PAGE_SIZE, "%s\n", string);
}

static ssize_t
gp2ap_method_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct proximity_gp2ap_device *pdev = i2c_get_clientdata(client);
	pm_message_t dummy_state;
	unsigned char string[10];
	int method;

	dummy_state.event = 0;

	sscanf(buf, "%s", string);

	if (!strncmp(string, "normal", 6))
		method = 0;
	else if (!strncmp(string, "interrupt", 6))
		method = 1;
	else {
		printk(KERN_INFO "Usage: echo [normal | interrupt] > method\n");
		printk(KERN_INFO " normal   : normal    mode\n");
		printk(KERN_INFO " interrupt: interrupt mode\n");
		return count;
	}

	if (pdev->methods != method) {
		gp2ap_suspend(client, dummy_state);
		pdev->methods = method;
		gp2ap_resume(client);
		(pdev->methods) ? printk(KERN_INFO "interrupt\n") : printk(KERN_INFO "normal\n");
	}
	else {
		printk(KERN_INFO "sw mode is already %s\n", string);
		return count;
	}

	return count;
}

static ssize_t
gp2ap_cycle_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct proximity_gp2ap_device *pdev = i2c_get_clientdata(client);

	return snprintf(buf, PAGE_SIZE, "%s\n", gp2ap_cycle_table[pdev->cycle].resp_time);
}

static ssize_t
gp2ap_cycle_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct proximity_gp2ap_device *pdev = i2c_get_clientdata(client);
	int cycle;

	sscanf(buf, "%d", &cycle);

	if ((cycle < 0) || (cycle > ARRAY_SIZE(gp2ap_cycle_table))) {
		printk(KERN_INFO "Usage: echo [0 ~ %d] > cycle\n", ARRAY_SIZE(gp2ap_cycle_table));
		printk(KERN_INFO " 0(8ms),1(16ms),2(32ms),3(64ms),4(128ms)\n");
		printk(KERN_INFO " 5(256ms),6(512ms),7(1024ms)\n");
		return count;
	}

	if (pdev->cycle != cycle)
		pdev->cycle = cycle;
	else {
		printk(KERN_INFO "cycle is already %d\n", pdev->cycle);
		return count;
	}

	disable_irq(pdev->irq);
	prox_i2c_write(GP2AP_REG_CON, 0x18, GP2AP_NO_INTCLEAR);
	prox_i2c_write(GP2AP_REG_CYCLE, gp2ap_cycle_table[pdev->cycle].val, GP2AP_NO_INTCLEAR);
	prox_i2c_write(GP2AP_REG_CON, 0x00, GP2AP_NO_INTCLEAR);
	enable_irq(pdev->irq);

	return count;
}

static ssize_t
gp2ap_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct proximity_gp2ap_device *pdev = i2c_get_clientdata(client);

	return snprintf(buf, PAGE_SIZE, "%d\n", pdev->sw_mode);
}

static ssize_t
gp2ap_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct proximity_gp2ap_device *pdev = i2c_get_clientdata(client);
	pm_message_t dummy_state;
	int mode;

	dummy_state.event = 0;

	sscanf(buf, "%d", &mode);

	if ((mode != PROX_STAT_SHUTDOWN) && (mode != PROX_STAT_OPERATING)) {
		printk(KERN_INFO "Usage: echo [0 | 1] > enable");
		printk(KERN_INFO " 0: disable\n");
		printk(KERN_INFO " 1: enable\n");
		return count;
	}

	if (mode == pdev->sw_mode) {
		printk(KERN_INFO "mode is already %d\n", pdev->sw_mode);
		return count;
	}
	else {
		if (mode) {
			gp2ap_resume(client);
			printk(KERN_INFO "Power On Enable\n");
		}
		else {
			gp2ap_suspend(client, dummy_state);
			printk(KERN_INFO "Power Off Disable\n");
		}
	}

	return count;
}

static ssize_t
gp2ap_debounce_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct proximity_gp2ap_device *pdev = i2c_get_clientdata(client);

	return snprintf(buf, PAGE_SIZE, "%d ms\n", pdev->debounce);
}

static ssize_t
gp2ap_debounce_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct proximity_gp2ap_device *pdev = i2c_get_clientdata(client);
	int debounce;

	sscanf(buf, "%d", &debounce);

	if (pdev->debounce != debounce)
		pdev->debounce = debounce;

	return count;
}

static ssize_t
gp2ap_asd_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct proximity_gp2ap_device *pdev = i2c_get_clientdata(client);

	return snprintf(buf, PAGE_SIZE, "%d(0:disable, 1:enable)\n", pdev->asd);
}

static ssize_t
gp2ap_asd_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct proximity_gp2ap_device *pdev = i2c_get_clientdata(client);
	int asd;
	u8 opmod;

	sscanf(buf, "%d", &asd);

	if ((asd < 0) || (asd > 1)) {
		printk(KERN_INFO "Usaage: echo [0 | 1] > asd\n");
		printk(KERN_INFO " 0: disable\n");
		printk(KERN_INFO " 1: enable\n");
		return count;
	}

	if (pdev->asd != asd) {
		opmod = (u8)(GP2AP_ASD_SHIFT(asd) | (pdev->methods << 1) | pdev->sw_mode);
		disable_irq(pdev->irq);
		prox_i2c_write(GP2AP_REG_CON, 0x18, GP2AP_NO_INTCLEAR);
		prox_i2c_write(GP2AP_REG_OPMOD, opmod, GP2AP_NO_INTCLEAR);
		prox_i2c_write(GP2AP_REG_CON, 0x00, GP2AP_NO_INTCLEAR);
		enable_irq(pdev->irq);
		pdev->asd = asd;
	}
	else
		printk(KERN_INFO "ASD is already %d\n", pdev->asd);

	return count;
}


void proximity_test_mode_get_value( int *value )
{
	pm_message_t dummy_state;
	if (gp2ap_pdev == NULL) {
		PROXE("gp2ap device is null\n");
		return -1;
	}

	gp2ap_resume(gp2ap_pdev->client);

	struct proximity_gp2ap_device *pdev = i2c_get_clientdata(gp2ap_pdev->client);
		*value = !(pdev->last_vout);

	gp2ap_suspend(gp2ap_pdev->client, dummy_state);
}

EXPORT_SYMBOL(proximity_test_mode_get_value);


static struct device_attribute gp2ap_device_attrs[] = {
	__ATTR(show, S_IRUGO | S_IWUSR, gp2ap_status_show, NULL),
	__ATTR(method, S_IRUGO | S_IWUSR, gp2ap_method_show, gp2ap_method_store),
	__ATTR(cycle, S_IRUGO | S_IWUSR, gp2ap_cycle_show, gp2ap_cycle_store),
	__ATTR(enable, S_IRUGO | S_IWUSR, gp2ap_enable_show, gp2ap_enable_store),
	__ATTR(debounce, S_IRUGO | S_IWUSR, gp2ap_debounce_show, gp2ap_debounce_store),
	__ATTR(asd, S_IRUGO | S_IWUSR, gp2ap_asd_show, gp2ap_asd_store),
};





static int
gp2ap_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	int i;
	struct proximity_platform_data	*pdata;
	pm_message_t dummy_state;

	if (GP2AP_DEBUG_FUNC_TRACE & gp2ap_debug_mask)
		PROXD("entry\n");

	if (GP2AP_DEBUG_ERR_CHECK & gp2ap_debug_mask) {
		if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)){
			PROXE("it is not support I2C_FUNC_I2C.\n");
			return -ENODEV;
		}
	}

	gp2ap_pdev = kzalloc(sizeof(struct proximity_gp2ap_device), GFP_KERNEL);
	if (GP2AP_DEBUG_ERR_CHECK & gp2ap_debug_mask) {
		if (gp2ap_pdev == NULL) {
			PROXE("failed to allocation\n");
			return -ENOMEM;
		}
	}

	INIT_DELAYED_WORK(&gp2ap_pdev->dwork, gp2ap_work_func);

	gp2ap_pdev->client = client;

	i2c_set_clientdata(gp2ap_pdev->client, gp2ap_pdev);

	
	gp2ap_pdev->input_dev = input_allocate_device();
	if (GP2AP_DEBUG_ERR_CHECK & gp2ap_debug_mask) {
		if (NULL == gp2ap_pdev->input_dev) {
			dev_err(&client->dev, "failed to allocation\n");
			goto err_input_allocate_device;
		}
	}

	
	gp2ap_pdev->input_dev->name = "proximity";
	gp2ap_pdev->input_dev->phys = "proximity/input2";

	set_bit(EV_SYN, gp2ap_pdev->input_dev->evbit); 
	set_bit(EV_ABS, gp2ap_pdev->input_dev->evbit);
	input_set_abs_params(gp2ap_pdev->input_dev, ABS_DISTANCE, 0, 1, 0, 0);

	
	ret = input_register_device(gp2ap_pdev->input_dev);
	if (GP2AP_DEBUG_ERR_CHECK & gp2ap_debug_mask) {
		if (ret < 0) {
			PROXE("failed to register input\n");
			goto err_input_register_device;
		}
	}

	pdata = gp2ap_pdev->client->dev.platform_data;
	if (GP2AP_DEBUG_ERR_CHECK & gp2ap_debug_mask) {
		if (pdata == NULL) {
			PROXE("failed to get platform data\n");
			goto err_gp2ap_initialise;
		}
	}

	
	gp2ap_pdev->vout_gpio = pdata->irq_num;
	gp2ap_pdev->irq = gpio_to_irq(pdata->irq_num);
	gp2ap_pdev->op_mode = pdata->operation_mode;
	gp2ap_pdev->methods = pdata->methods;
	gp2ap_pdev->last_vout = -1;
	gp2ap_pdev->debounce = pdata->debounce;
	gp2ap_pdev->asd = 0;		
	gp2ap_pdev->cycle = pdata->cycle;		

	spin_lock_init(&gp2ap_pdev->lock);

	
	pdata->power(1);
	udelay(60);

	
	ret = gp2ap_device_initialise();
	if (GP2AP_DEBUG_ERR_CHECK & gp2ap_debug_mask) {
		if (ret < 0) {
			PROXE("failed to init\n");
			goto err_gp2ap_initialise;
		}
	}

	if (GP2AP_DEBUG_GEN_INFO & gp2ap_debug_mask)
		PROXD("vout gpio(%d), irq(%d)\n", gp2ap_pdev->vout_gpio, gp2ap_pdev->irq);

	
	ret = request_irq(gp2ap_pdev->irq, gp2ap_irq_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
		"proximity_irq", gp2ap_pdev);
	if (GP2AP_DEBUG_ERR_CHECK & gp2ap_debug_mask) {
		if (ret < 0) {
			PROXE("failed to register irq\n");
			goto err_irq_request;
		}
	}

	gp2ap_pdev->sw_mode = PROX_STAT_OPERATING;

	if (GP2AP_DEBUG_GEN_INFO & gp2ap_debug_mask)
		PROXD("i2c client addr(0x%x)\n", gp2ap_pdev->client->addr);

	ret = set_irq_wake(gp2ap_pdev->irq, 1);
        if (ret)
		set_irq_wake(gp2ap_pdev->irq, 0);
	
	for (i = 0; i < ARRAY_SIZE(gp2ap_device_attrs); i++) {
		ret = device_create_file(&client->dev, &gp2ap_device_attrs[i]);
		if (ret) {
			goto err_device_create_file;
		}
	}

	gp2ap_suspend(gp2ap_pdev->client, dummy_state);

	if (GP2AP_DEBUG_FUNC_TRACE & gp2ap_debug_mask)
		PROXD("exit\n");

	return 0;

err_device_create_file:
	while(--i >= 0)
		device_remove_file(&client->dev, &gp2ap_device_attrs[i]);
err_irq_request:
	input_unregister_device(gp2ap_pdev->input_dev);
err_gp2ap_initialise:
err_input_register_device:
	input_free_device(gp2ap_pdev->input_dev);
err_input_allocate_device:
	kfree(gp2ap_pdev);

	return ret;
}

static int
gp2ap_i2c_remove(struct i2c_client *client)
{
	struct proximity_gp2ap_device *pdev = i2c_get_clientdata(client);
	int i;

	if (GP2AP_DEBUG_FUNC_TRACE & gp2ap_debug_mask)
		PROXD("entry\n");

	set_irq_wake(gp2ap_pdev->irq, 0);

	free_irq(pdev->irq, NULL);
	input_unregister_device(pdev->input_dev);
	input_free_device(pdev->input_dev);
	kfree(gp2ap_pdev);

	for (i = 0; i < ARRAY_SIZE(gp2ap_device_attrs); i++)
		device_remove_file(&client->dev, &gp2ap_device_attrs[i]);

	if (GP2AP_DEBUG_FUNC_TRACE & gp2ap_debug_mask)
		PROXD("exit\n");

	return 0;
}

static int
gp2ap_suspend(struct i2c_client *i2c_dev, pm_message_t state)
{
	struct proximity_gp2ap_device *pdev = i2c_get_clientdata(i2c_dev);
	struct proximity_platform_data *pdata = pdev->client->dev.platform_data;
	int ret;

	if (GP2AP_DEBUG_FUNC_TRACE & gp2ap_debug_mask)
		PROXD("entry\n");

	
	if (!pdev->sw_mode)
		return 0;

	disable_irq(pdev->irq);

	
	if (pdev->methods) 	
		ret	= prox_i2c_write(GP2AP_REG_OPMOD, (GP2AP_ASD_SHIFT(1) | 0x02), GP2AP_NO_INTCLEAR);
	else				
		ret	= prox_i2c_write(GP2AP_REG_OPMOD, (GP2AP_ASD_SHIFT(1) | 0x00), GP2AP_NO_INTCLEAR);

	if (GP2AP_DEBUG_ERR_CHECK & gp2ap_debug_mask) {
		if (ret < 0) {
			PROXE("failed to write\n");
		}
	}

	pdev->sw_mode = PROX_STAT_SHUTDOWN;

	cancel_delayed_work_sync(&pdev->dwork);
	flush_workqueue(proximity_wq);

	
	pdata->power(0);

	set_irq_wake(pdev->irq, 0);

	if (GP2AP_DEBUG_FUNC_TRACE & gp2ap_debug_mask)
		PROXD("exit\n");

	return 0;
}

static void
gp2ap_report_init_staus(struct proximity_gp2ap_device *pdev)
{
	int vo_data;

	msleep(100);
	vo_data = prox_i2c_read(GP2AP_REG_PROX, GP2AP_NO_INTCLEAR);
	vo_data = (vo_data >> 8) & 0x1;
	gp2ap_report_event(vo_data);
}

static int
gp2ap_resume(struct i2c_client *i2c_dev)
{
	struct proximity_gp2ap_device *pdev = i2c_get_clientdata(i2c_dev);
	struct proximity_platform_data *pdata = pdev->client->dev.platform_data;
	int ret;
	int addr;

	if (GP2AP_DEBUG_FUNC_TRACE & gp2ap_debug_mask)
		PROXD("entry\n");

	if (pdev->sw_mode)
		return 0;

	
	pdata->power(1);

	pdev->last_vout = -1;

	udelay(60);

	
	if (pdev->methods) 	
		pdev->reg_backup[GP2AP_REG_OPMOD] = 0x03;
	else				
		pdev->reg_backup[GP2AP_REG_OPMOD] = 0x01;

	for(addr = 0; addr < 6; addr++)
	{
		if(pdev->reg_backup[addr] != 0)
		{
			ret = prox_i2c_write(addr, pdev->reg_backup[addr], GP2AP_NO_INTCLEAR);

			if (ret < 0) {
				PROXE("%s failed to write - addr:%d\n", __func__, addr);
				pdata->power(0);
				return -1;
			}
		}
		else
			continue;
	}

	
        gp2ap_report_event(PROX_SENSOR_DETECT_N);
	pdev->last_vout = -1;

	enable_irq(pdev->irq);

	
	if (pdev->methods)
		prox_i2c_write(GP2AP_REG_CON, 0x00, GP2AP_INTCLEAR);

	
	gp2ap_report_init_staus(pdev);

	pdev->sw_mode = PROX_STAT_OPERATING;

	ret = set_irq_wake(pdev->irq, 1);
        if (ret)
		set_irq_wake(pdev->irq, 0);

	if (GP2AP_DEBUG_FUNC_TRACE & gp2ap_debug_mask)
		PROXD("exit\n");

	return 0;
}

static const struct i2c_device_id gp2ap_i2c_ids[] = {
		{"proximity_gp2ap", 0 },
		{ },
};

MODULE_DEVICE_TABLE(i2c, prox_ids);

static struct i2c_driver gp2ap_i2c_driver = {
	.probe		= gp2ap_i2c_probe,
	.remove		= gp2ap_i2c_remove,
	.id_table	= gp2ap_i2c_ids,
	.driver = {
		.name	= "proximity_gp2ap",
		.owner	= THIS_MODULE,
	},
};

static void __exit gp2ap_i2c_exit(void)
{
	i2c_del_driver(&gp2ap_i2c_driver);
	if (proximity_wq)
		destroy_workqueue(proximity_wq);
}

static int __init gp2ap_i2c_init(void)
{
	int ret;

	if (GP2AP_DEBUG_FUNC_TRACE & gp2ap_debug_mask)
		PROXD("entry\n");

	proximity_wq = create_singlethread_workqueue("proximity_wq");
	if (GP2AP_DEBUG_ERR_CHECK & gp2ap_debug_mask) {
		if (!proximity_wq) {
			PROXE("failed to create singlethread workqueue\n");
			return -ENOMEM;
		}
	}

	ret = i2c_add_driver(&gp2ap_i2c_driver);
	if (GP2AP_DEBUG_ERR_CHECK & gp2ap_debug_mask) {
		if (ret < 0) {
			PROXE("failed to i2c_add_driver \n");
			destroy_workqueue(proximity_wq);
			return ret;
		}
	}

	if (GP2AP_DEBUG_FUNC_TRACE & gp2ap_debug_mask)
		PROXD("entry\n");

	return 0;
}

late_initcall(gp2ap_i2c_init);
module_exit(gp2ap_i2c_exit);

MODULE_DESCRIPTION("gp2ap00200f driver");
MODULE_LICENSE("GPL");
