

#include <linux/timer.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/workqueue.h>

#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include "touch_mcs6000_down_ioctl.h"
#include "touch_mcs6000_ioctl.h"
#include <linux/i2c-gpio.h>
#include <mach/board_lge.h>

#include <mach/vreg.h>
struct vreg {
	const char *name;
	unsigned id;
	int status;
	unsigned refcnt;
};

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>

static struct early_suspend ts_early_suspend;
static void mcs6000_early_suspend(struct early_suspend *h);
static void mcs6000_late_resume(struct early_suspend *h);
#endif

#if defined(CONFIG_MACH_MSM7X27_ALESSI) || defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_KU3100) || defined(CONFIG_MACH_MSM7X27_LU3100)
#define LG_FW_HARDKEY_BLOCK
#endif
#define LG_FW_MULTI_TOUCH
#define LG_FW_TOUCH_SOFT_KEY 1
#define TOUCH_SEARCH    247
#define TOUCH_BACK      248




#define TS_POLLING_TIME 0 

#define DEBUG_TS 0 
#if DEBUG_TS
#define DMSG(fmt, args...) printk(KERN_DEBUG fmt, ##args)
#else
#define DMSG(fmt, args...) do{} while(0)
#endif

#define ON 	1
#define OFF 	0

#define PRESSED 	1
#define RELEASED 	0

#define MCS6000_TS_INPUT_INFO					0x10
#define MCS6000_TS_XY_HIGH						0x11
#define MCS6000_TS_X_LOW						0x12
#define MCS6000_TS_Y_LOW						0x13
#define MCS6000_TS_Z				 			0x14
#define MCS6000_TS_XY2_HIGH				 		0x15
#define MCS6000_TS_X2_LOW					 	0x16
#define MCS6000_TS_Y2_LOW	  					0x17
#define MCS6000_TS_Z2			 				0x18
#define MCS6000_TS_KEY_STRENGTH  				0x19
#define MCS6000_TS_FW_VERSION			 		0x20
#define MCS6000_TS_HW_REVISION					0x21

#define MCS6000_TS_MAX_HW_VERSION				0x40
#define MCS6000_TS_MAX_FW_VERSION				0x20

struct mcs6000_ts_device {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct delayed_work work;
#ifdef LG_FW_HARDKEY_BLOCK
	struct hrtimer touch_timer;
	bool hardkey_block;
#endif
	int num_irq;
	int intr_gpio;
	int scl_gpio;
	int sda_gpio;
	bool pendown;
	int (*power)(unsigned char onoff);
	struct workqueue_struct *ts_wq;
};

static struct input_dev *mcs6000_ts_input = NULL;
static struct mcs6000_ts_device mcs6000_ts_dev; 
static int is_downloading = 0;
static int is_touch_suspend = 0;
static int KEY_MENU_TOUCHED ; 
static int KEY_HOME_TOUCHED;
static int KEY_BACK_TOUCHED;
static int KEY_SEARCH_TOUCHED;

#define READ_NUM 8 

enum{
	NON_TOUCHED_STATE,
	SINGLE_POINT_TOUCH,
	MULTI_POINT_TOUCH,
	MAX_TOUCH_TYPE
};



void Send_Touch( unsigned int x, unsigned int y)
{
#ifdef LG_FW_MULTI_TOUCH
	input_report_abs(mcs6000_ts_dev.input_dev, ABS_MT_TOUCH_MAJOR, 1);
	input_report_abs(mcs6000_ts_dev.input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(mcs6000_ts_dev.input_dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(mcs6000_ts_dev.input_dev);
	input_sync(mcs6000_ts_dev.input_dev);
	
	input_report_abs(mcs6000_ts_dev.input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_report_abs(mcs6000_ts_dev.input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(mcs6000_ts_dev.input_dev, ABS_MT_POSITION_Y, y);
	input_mt_sync(mcs6000_ts_dev.input_dev);
	input_sync(mcs6000_ts_dev.input_dev);
#else
	mcs6000_ts_event_touch( x, y , &mcs6000_ts_dev) ;
	input_report_abs(mcs6000_ts_dev.input_dev, ABS_X, x);
	input_report_abs(mcs6000_ts_dev.input_dev, ABS_Y, y);
	input_report_key(mcs6000_ts_dev.input_dev, BTN_TOUCH, 0);
	input_sync(mcs6000_ts_dev.input_dev);
#endif
}
EXPORT_SYMBOL(Send_Touch);



static __inline void mcs6000_key_event_touch(int touch_reg,  int value,  struct mcs6000_ts_device *dev)
{
	unsigned int keycode;

#if defined(CONFIG_MACH_MSM7X27_ALESSI) || defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_KU3100) || defined(CONFIG_MACH_MSM7X27_LU3100)
	if (touch_reg == KEY_BACK_TOUCHED) {
#ifdef LG_FW_AUDIO_HAPTIC_TOUCH_SOFT_KEY
		keycode = TOUCH_BACK;
#else
		keycode = KEY_BACK;
#endif
	} else if (touch_reg == KEY_MENU_TOUCHED) {
#ifdef LG_FW_AUDIO_HAPTIC_TOUCH_SOFT_KEY
		keycode = TOUCH_MENU;
#else
		keycode = KEY_MENU; 
#endif
	} else if (touch_reg == KEY_HOME_TOUCHED) {
#ifdef LG_FW_AUDIO_HAPTIC_TOUCH_SOFT_KEY
		keycode = TOUCH_HOME;
#else
		keycode = KEY_HOME;
#endif
	} else if (touch_reg == KEY_SEARCH_TOUCHED) {
	#ifdef LG_FW_AUDIO_HAPTIC_TOUCH_SOFT_KEY
		keycode = TOUCH_SEARCH;
	#else
		keycode = KEY_SEARCH;
	#endif
	}
	else {
		printk("%s Not available touch key reg. %d\n", __FUNCTION__, touch_reg);
		return;
	} 
#endif
	input_report_key(dev->input_dev, keycode, value);
	input_sync(dev->input_dev);

	DMSG("%s Touch Key Code %d, Value %d\n", __FUNCTION__, keycode, value);

	return;
}

#ifdef LG_FW_MULTI_TOUCH
static __inline void mcs6000_multi_ts_event_touch(int x1, int y1, int x2, int y2, int value,
		struct mcs6000_ts_device *dev)
{
	int report = 0;

	DMSG("touch (%d,%d) (%d,%d) PRESSED=%d\n", x1, y1, x2, y2, value);

	if ((x1 >= 0) && (y1 >= 0)) {
		input_report_abs(dev->input_dev, ABS_MT_TOUCH_MAJOR, value);
		input_report_abs(dev->input_dev, ABS_MT_POSITION_X, x1);
		input_report_abs(dev->input_dev, ABS_MT_POSITION_Y, y1);
		input_mt_sync(dev->input_dev);
		report = 1;
	}

	if ((x2 >= 0) && (y2 >= 0)) {
		input_report_abs(dev->input_dev, ABS_MT_TOUCH_MAJOR, value);
		input_report_abs(dev->input_dev, ABS_MT_POSITION_X, x2);
		input_report_abs(dev->input_dev, ABS_MT_POSITION_Y, y2);
		input_mt_sync(dev->input_dev);
		report = 1;
	}

	if (report != 0) {
		input_sync(dev->input_dev);
	} else {
		printk(KERN_WARNING "%s: Not Available touch data x1=%d, y1=%d, x2=%d, y2=%d\n", 
				__FUNCTION__,  x1, y1, x2, y2);
	}
	return;
}

#else

static __inline void mcs6000_single_ts_event_touch(unsigned int x, unsigned int y, int value,
				   struct mcs6000_ts_device *dev)
{
	int report = 0;

	DMSG("touch (%d,%d) PRESSED=%d\n", x, y, value);

	if ((x >= 0) && (y >= 0)) {
		input_report_abs(dev->input_dev, ABS_X, x);
		input_report_abs(dev->input_dev, ABS_Y, y);
		report = 1;
	}

	if (report != 0) {
		input_report_key(dev->input_dev, BTN_TOUCH, value);
		input_sync(dev->input_dev);
	} else {
		DMSG(KERN_WARNING "%s: Not Available touch data x=%d, y=%d\n", __FUNCTION__, x, y); 
	}

	return;
}

static __inline void mcs6000_single_ts_event_release(struct mcs6000_ts_device *dev)
{
	input_report_key(dev->input_dev, BTN_TOUCH, 0);
	input_sync(dev->input_dev);

	return;
}
#endif 

#define to_delayed_work(_work)  container_of(_work, struct delayed_work, work)

#ifdef LG_FW_HARDKEY_BLOCK
static enum hrtimer_restart timed_touch_timer_func(struct hrtimer *timer)
{
	mcs6000_ts_dev.hardkey_block = 0;	
	return HRTIMER_NORESTART;
}
#endif

static void mcs6000_work(struct work_struct *work)
{
	int x1=0, y1 = 0;
#ifdef LG_FW_MULTI_TOUCH
	int x2=0, y2 = 0;
	static int pre_x1, pre_x2, pre_y1, pre_y2;
	static unsigned int s_input_type = NON_TOUCHED_STATE;
#endif
	unsigned int input_type;
	unsigned int key_touch;	
	unsigned char read_buf[READ_NUM];

	static int key_pressed = 0;
	static int touch_pressed = 0;

	struct mcs6000_ts_device *dev 
		= container_of(to_delayed_work(work), struct mcs6000_ts_device, work);

	dev->pendown = !gpio_get_value(dev->intr_gpio);

	
	if ( i2c_smbus_read_i2c_block_data(dev->client, MCS6000_TS_INPUT_INFO, READ_NUM, read_buf) < 0) {
		printk(KERN_ERR "%s touch ic read error\n", __FUNCTION__);
		goto touch_retry;
	}

	input_type = read_buf[0] & 0x0f;
	key_touch = (read_buf[0] & 0xf0) >> 4;

	x1 = y1 =0;
#ifdef LG_FW_MULTI_TOUCH
	x2 = y2 = 0;
#endif
	x1 = (read_buf[1] & 0xf0) << 4;
	y1 = (read_buf[1] & 0x0f) << 8;

	x1 |= read_buf[2];	
	y1 |= read_buf[3];		


#if defined(CONFIG_MACH_MSM7X27_ALESSI) || defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_KU3100) || defined(CONFIG_MACH_MSM7X27_LU3100)
	if(dev->input_dev->id.version < 0x12){
	x1 = x1 * 150 / 225;
	y1 = y1 * 160 / 267; 

	x1 = 320 - x1;
	y1 = 480 - y1;
	}
#endif


#ifdef LG_FW_MULTI_TOUCH
	if(input_type == MULTI_POINT_TOUCH) {
		s_input_type = input_type;
		x2 = (read_buf[5] & 0xf0) << 4;
		y2 = (read_buf[5] & 0x0f) << 8;
		x2 |= read_buf[6];
		y2 |= read_buf[7];
		
#if defined(CONFIG_MACH_MSM7X27_ALESSI) || defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_KU3100) || defined(CONFIG_MACH_MSM7X27_LU3100)
	if(dev->input_dev->id.version < 0x12){
		x2 = x2 * 150 / 225;
		y2 = y2 * 160 / 267; 

		x2 = 320 - x2;
		y2 = 480 - y2;
	}
#endif
		
	}
#endif

	if (dev->pendown) { 
#ifdef LG_FW_HARDKEY_BLOCK
		if(dev->hardkey_block == 0)
#endif
		if(key_touch) {
			mcs6000_key_event_touch(key_touch, PRESSED, dev);
			key_pressed = key_touch;
		}

		if(input_type) {
			touch_pressed = 1;

			
			if(key_pressed) {
				mcs6000_key_event_touch(key_pressed, RELEASED, dev);
				key_pressed = 0;
			}
#ifdef LG_FW_MULTI_TOUCH
			if(input_type == MULTI_POINT_TOUCH) {
				mcs6000_multi_ts_event_touch(x1, y1, x2, y2, PRESSED, dev);
				pre_x1 = x1;
				pre_y1 = y1;
				pre_x2 = x2;
				pre_y2 = y2;
			}
			else if(input_type == SINGLE_POINT_TOUCH) {
				mcs6000_multi_ts_event_touch(x1, y1, -1, -1, PRESSED, dev);
				s_input_type = SINGLE_POINT_TOUCH;				
			}
#else
			if(input_type == SINGLE_POINT_TOUCH) {
				mcs6000_single_ts_event_touch(x1, y1, PRESSED, dev);
			}
#endif
#ifdef LG_FW_HARDKEY_BLOCK
			dev->hardkey_block = 1;
#endif				
		}
	} 
	else { 
		if(key_pressed) {
			mcs6000_key_event_touch(key_pressed, RELEASED, dev);
			key_pressed = 0;
		}

		if(touch_pressed) {
#ifdef LG_FW_MULTI_TOUCH
			if(s_input_type == MULTI_POINT_TOUCH) {
				DMSG("%s: multi touch release...(%d, %d), (%d, %d)\n", __FUNCTION__,pre_x1,pre_y1,pre_x2,pre_y2);
				mcs6000_multi_ts_event_touch(pre_x1, pre_y1, pre_x2, pre_y2, RELEASED, dev);
				s_input_type = NON_TOUCHED_STATE; 
				pre_x1 = -1; pre_y1 = -1; pre_x2 = -1; pre_y2 = -1;
			} else {
				DMSG("%s: single touch release... %d, %d\n", __FUNCTION__, x1, y1);
				mcs6000_multi_ts_event_touch(x1, y1, -1, -1, RELEASED, dev);
			}
#else
			DMSG("%s: single release... %d, %d\n", __FUNCTION__, x1, y1);
			mcs6000_single_ts_event_touch (x1, y1, RELEASED, dev);
			touch_pressed = 0;
#endif
#ifdef LG_FW_HARDKEY_BLOCK
			hrtimer_cancel(&dev->touch_timer);
			hrtimer_start(&dev->touch_timer, ktime_set(0, 800),
			HRTIMER_MODE_REL);
#endif
		}
	}

touch_retry:
	if (dev->pendown) {
		
		queue_delayed_work(dev->ts_wq, &dev->work,msecs_to_jiffies(TS_POLLING_TIME));
	} else {
		enable_irq(dev->num_irq);
		DMSG("%s: irq enable\n", __FUNCTION__);
	}
}

static irqreturn_t mcs6000_ts_irq_handler(int irq, void *handle)
{
	struct mcs6000_ts_device *dev = handle;

	if (gpio_get_value(dev->intr_gpio) == 0) {
		disable_irq_nosync(dev->num_irq);
		DMSG("%s: irq disable\n", __FUNCTION__);
		
		queue_delayed_work(dev->ts_wq, &dev->work,msecs_to_jiffies(TS_POLLING_TIME));
	}

	return IRQ_HANDLED;
}

static int mcs6000_ts_on(void)
{
	struct mcs6000_ts_device *dev = NULL;
	int ret = 0;

	dev = &mcs6000_ts_dev;

	ret = dev->power(ON);
	if(ret < 0)	{
		printk(KERN_ERR "mcs6000_ts_on power on failed\n");
		goto err_power_failed;				
	}
	msleep(10);

err_power_failed:
	return ret;
}

void mcs6000_firmware_info(unsigned char* fw_ver, unsigned char* hw_ver)
{
	unsigned char data;
	struct mcs6000_ts_device *dev = NULL;
	int try_cnt = 0;
	dev = &mcs6000_ts_dev;

#if 0
	msleep(200);
	data = i2c_smbus_read_byte_data(dev->client, MCS6000_TS_FW_VERSION);
	printk(KERN_INFO "MCS6000 F/W Version [0x%x]\n", data);
	dev->input_dev->id.version = data;
	data = i2c_smbus_read_byte_data(dev->client, MCS6000_TS_HW_REVISION);
	printk(KERN_INFO "MCS6000 H/W Revision [0x%x]\n", data);
	dev->input_dev->id.product= data ;
#else
	
	do {
		data = i2c_smbus_read_byte_data(dev->client, MCS6000_TS_FW_VERSION);
		msleep(10);
		try_cnt ++;
	} while (data > MCS6000_TS_MAX_FW_VERSION && try_cnt < 10);

	printk(KERN_INFO "MCS6000 F/W Version [0x%x]\n", data);
	dev->input_dev->id.version = data;
	*fw_ver = data;

	try_cnt = 0;
	do {
		data = i2c_smbus_read_byte_data(dev->client, MCS6000_TS_HW_REVISION);
		msleep(10);
		try_cnt ++;
	} while (data > MCS6000_TS_MAX_HW_VERSION && try_cnt < 10);

	printk(KERN_INFO "MCS6000 H/W Revision [0x%x]\n", data);
	dev->input_dev->id.product= data ;
	*hw_ver = data;
#endif
}

static __inline int mcs6000_ts_ioctl_down_i2c_write(unsigned char addr,
				    unsigned char val)
{
	int err = 0;
	struct i2c_client *client;
	struct i2c_msg msg;

	client = mcs6000_ts_dev.client;
	if (client == NULL) {
		DMSG("\n%s: i2c client error \n", __FUNCTION__);
		return -1;
	}
	msg.addr = addr;
	msg.flags = 0;
	msg.len = 1;
	msg.buf = &val;

	if ((err = i2c_transfer(client->adapter, &msg, 1)) < 0) {
		DMSG("\n%s: i2c write error [%d]\n", __FUNCTION__, err);
	}

	return err;
}

static __inline int mcs6000_ts_ioctl_down_i2c_read(unsigned char addr,
				   unsigned char *ret)
{
	int err = 0;
	struct i2c_client *client;
	struct i2c_msg msg;

	client = mcs6000_ts_dev.client;
	if (client == NULL) {
		DMSG("\n%s: i2c client drror \n", __FUNCTION__);
		return -1;
	}
	msg.addr = addr;
	msg.flags = 1;
	msg.len = 1;
	msg.buf = ret;

	if ((err = i2c_transfer(client->adapter, &msg, 1)) < 0) {
		DMSG("\n%s: i2c read error [%d]\n", __FUNCTION__, err);
	}

	return err;
}

int mcs6000_ts_ioctl_down(struct inode *inode, struct file *flip, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	struct mcs6000_ts_down_ioctl_i2c_type client_data;
	struct mcs6000_ts_device *dev = NULL;

	dev = &mcs6000_ts_dev;

	
	if (_IOC_NR(cmd) >= MCS6000_TS_DOWN_IOCTL_MAXNR)
		return -EINVAL;

	switch (cmd) {
		case MCS6000_TS_DOWN_IOCTL_VDD_HIGH:
			err = dev->power(ON);
			if( err < 0 )
				printk(KERN_INFO"%s: Power On Fail....\n", __FUNCTION__);
			break;

		case MCS6000_TS_DOWN_IOCTL_VDD_LOW:
			err = dev->power(OFF);
			if( err < 0 )
				printk(KERN_INFO"%s: Power Down Fail..\n",  __FUNCTION__);
			break;

		case MCS6000_TS_DOWN_IOCTL_INTR_HIGH:
			gpio_configure(dev->intr_gpio, GPIOF_DRIVE_OUTPUT | GPIOF_OUTPUT_HIGH);
			break;
		case MCS6000_TS_DOWN_IOCTL_INTR_LOW:
			gpio_configure(dev->intr_gpio, GPIOF_DRIVE_OUTPUT | GPIOF_OUTPUT_LOW);
			break;
		case MCS6000_TS_DOWN_IOCTL_INTR_OUT:
			gpio_configure(dev->intr_gpio, GPIOF_DRIVE_OUTPUT);
			break;
		case MCS6000_TS_DOWN_IOCTL_INTR_IN:
			gpio_configure(dev->intr_gpio, GPIOF_INPUT);
			break;

		case MCS6000_TS_DOWN_IOCTL_SCL_HIGH:
			gpio_configure(dev->scl_gpio, GPIOF_DRIVE_OUTPUT | GPIOF_OUTPUT_HIGH);
			break;
		case MCS6000_TS_DOWN_IOCTL_SCL_LOW:
			gpio_configure(dev->scl_gpio, GPIOF_DRIVE_OUTPUT | GPIOF_OUTPUT_LOW);
			break;
		case MCS6000_TS_DOWN_IOCTL_SDA_HIGH:
			gpio_configure(dev->sda_gpio, GPIOF_DRIVE_OUTPUT | GPIOF_OUTPUT_HIGH);
			break;
		case MCS6000_TS_DOWN_IOCTL_SDA_LOW:
			gpio_configure(dev->sda_gpio, GPIOF_DRIVE_OUTPUT | GPIOF_OUTPUT_LOW);
			break;
		case MCS6000_TS_DOWN_IOCTL_SCL_OUT:
			gpio_configure(dev->scl_gpio, GPIOF_DRIVE_OUTPUT);
			break;
		case MCS6000_TS_DOWN_IOCTL_SDA_OUT:
			gpio_configure(dev->sda_gpio, GPIOF_DRIVE_OUTPUT);
			break;

		case MCS6000_TS_DOWN_IOCTL_I2C_ENABLE:
			
			break;
		case MCS6000_TS_DOWN_IOCTL_I2C_DISABLE:
			
			break;

		case MCS6000_TS_DOWN_IOCTL_I2C_READ:
			if (copy_from_user(&client_data, (struct mcs6000_ts_down_ioctl_i2c_type *)arg,
						sizeof(struct mcs6000_ts_down_ioctl_i2c_type))) {
				printk(KERN_INFO "%s: copyfromuser error\n", __FUNCTION__);
				return -EFAULT;
			}

			if (0 > mcs6000_ts_ioctl_down_i2c_read( (unsigned char)client_data.addr,
						(unsigned char *)&client_data.data)) {
				err = -EIO;
			}

			if (copy_to_user((void *)arg, (const void *)&client_data,
						sizeof(struct mcs6000_ts_down_ioctl_i2c_type))) {
				printk(KERN_INFO "%s: copytouser error\n",
						__FUNCTION__);
				err = -EFAULT;
			}
			break;
		case MCS6000_TS_DOWN_IOCTL_I2C_WRITE:
			if (copy_from_user(&client_data, (struct mcs6000_ts_down_ioctl_i2c_type *)arg,
						sizeof(struct mcs6000_ts_down_ioctl_i2c_type))) {
				printk(KERN_INFO "%s: copyfromuser error\n", __FUNCTION__);
				return -EFAULT;
			}

			if (0 > mcs6000_ts_ioctl_down_i2c_write((unsigned char)client_data.addr,
						(unsigned char)client_data.data)) {
				err = -EIO;
			}
			break;
		case MCS6000_TS_DOWN_IOCTL_SELECT_TS_TYPE:
			break;
		default:
			err = -EINVAL;
			break;
	}

	if (err < 0)
		printk(KERN_ERR "\n==== Touch DONW IOCTL Fail....%d\n",_IOC_NR(cmd));

	return err;
}

static int mcs6000_ts_ioctl(struct inode *inode, struct file *flip,
		     unsigned int cmd, unsigned long arg)
{
	int err = -1;
	

	switch (_IOC_TYPE(cmd)) {
		case MCS6000_TS_DOWN_IOCTL_MAGIC:
			err = mcs6000_ts_ioctl_down(inode, flip, cmd, arg);
			break;
		case MCS6000_TS_IOCTL_MAGIC :
			switch(cmd){
				case MCS6000_TS_IOCTL_FW_VER:
				{
					unsigned char fw_ver, hw_ver;
					mcs6000_firmware_info(&fw_ver, &hw_ver);
					err = fw_ver;
					break;
				}
				case MCS6000_TS_IOCTL_MAIN_ON:
				case MCS6000_TS_IOCTL_MAIN_OFF:
					break;
			}
			break;
		default:
			printk(KERN_ERR "%s unknow ioctl\n", __FUNCTION__);
			err = -EINVAL;
			break;
	}
	return err;
}

static int mcs6000_ioctl_open(struct inode *inode, struct file *flip) {
	if(is_touch_suspend == 0) {
		disable_irq(mcs6000_ts_dev.num_irq);
		printk(KERN_INFO "touch download start : irq disabled by ioctl\n");
	}
	is_downloading = 1;
	return 0;
}

static int mcs6000_ioctl_release(struct inode *inode, struct file *flip) {
	if(is_touch_suspend == 1) {
		mcs6000_ts_dev.power(OFF);
		printk(KERN_INFO "touch download done : power off by ioctl\n");
	} else {
		enable_irq(mcs6000_ts_dev.num_irq);
		printk(KERN_INFO "touch download done : irq enabled by ioctl\n");
	}
	is_downloading = 0;
	return 0;
}

static struct file_operations mcs6000_ts_ioctl_fops = {
	.owner = THIS_MODULE,
	.ioctl = mcs6000_ts_ioctl,
	.open  = mcs6000_ioctl_open,
	.release = mcs6000_ioctl_release,
};

static struct miscdevice mcs6000_ts_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mcs6000-touch",
	.fops = &mcs6000_ts_ioctl_fops,
};

static ssize_t read_touch_version(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int r;
	unsigned char hw_ver, fw_ver;

	if (is_downloading == 1) {
		r = sprintf(buf,"Now, MCS6000 Firmware Update is going, check it later\n ");
		return r;
	}

	mcs6000_firmware_info(&fw_ver, &hw_ver);
	r = sprintf(buf,"MCS6000 Touch Version HW:%02x FW:%02x\n",hw_ver, fw_ver);

	return r;
}

static ssize_t read_touch_dl_status(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int r;

	r = sprintf(buf,"MCS6000 Download Status %d\n",is_downloading);
	return r;
}

static ssize_t read_touch_status(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int r;
	int int_status;
	struct mcs6000_ts_device *dev_tmp ;
	struct vreg *vreg_touch;

	dev_tmp = &mcs6000_ts_dev;
	vreg_touch = vreg_get(0, "synt");
	
	int_status = gpio_get_value(dev_tmp->intr_gpio);
	r = sprintf(buf,"MCS6000 interrupt Pin [%d] , power Status [%d]\n",int_status,vreg_touch->refcnt);
	return r;
}

static ssize_t write_touch_control(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int cmd,err;
	struct mcs6000_ts_device *dev_tmp;

	dev_tmp = &mcs6000_ts_dev;
	sscanf(buf, "%d", &cmd);

	switch (cmd){
		case 1:	
			err = gpio_direction_input(dev_tmp->intr_gpio);
			if (err < 0) {
				printk(KERN_ERR "%s: gpio input direction fail\n", __FUNCTION__);
				return err;
			}
			gpio_set_value(dev_tmp->intr_gpio, 1 );
			break;
		case 2:	
			err = gpio_direction_input(dev_tmp->intr_gpio);
			if (err < 0) {
				printk(KERN_ERR "%s: gpio input direction fail\n", __FUNCTION__);
				return err;
			}
			gpio_set_value(dev_tmp->intr_gpio, 0 );
			break;
		case 3:	
			dev_tmp->power(ON);
			break;
		case 4:	
			dev_tmp->power(OFF);
			break;
		default :
			break;
	}
	return size;
}

static DEVICE_ATTR(touch_control, S_IRUGO|S_IWUSR,NULL,write_touch_control);
static DEVICE_ATTR(touch_status, S_IRUGO,read_touch_status, NULL);
static DEVICE_ATTR(version, S_IRUGO ,read_touch_version, NULL);
static DEVICE_ATTR(dl_status, S_IRUGO,read_touch_dl_status, NULL);

int mcs6000_create_file(struct input_dev *pdev)
{
	int ret;

	ret = device_create_file(&pdev->dev, &dev_attr_version);
	if (ret) {
		printk( KERN_DEBUG "LG_FW : dev_attr_version create fail\n");
		device_remove_file(&pdev->dev, &dev_attr_version);
		return ret;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_dl_status);
	if (ret) {
		printk( KERN_DEBUG "LG_FW : dev_attr_dl_status create fail\n");
		device_remove_file(&pdev->dev, &dev_attr_dl_status);
		return ret;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_touch_status);
	if (ret) {
		printk( KERN_DEBUG "LG_FW : dev_attr_touch_status create fail\n");
		device_remove_file(&pdev->dev, &dev_attr_touch_status);
		return ret;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_touch_control);
	if (ret) {
		printk( KERN_DEBUG "LG_FW : dev_attr_touch_control create fail\n");
		device_remove_file(&pdev->dev, &dev_attr_touch_control);
		return ret;
	}

	return ret;
}

int mcs6000_remove_file(struct input_dev *pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_version);
	device_remove_file(&pdev->dev, &dev_attr_dl_status);
	device_remove_file(&pdev->dev, &dev_attr_touch_status);
	device_remove_file(&pdev->dev, &dev_attr_touch_control);
	return 0;
}
static int mcs6000_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err = 0;
	struct touch_platform_data *ts_pdata;
	struct mcs6000_ts_device *dev;
	unsigned char fw_ver, hw_ver;

	DMSG("%s: start...\n", __FUNCTION__);

	ts_pdata = client->dev.platform_data;

#ifdef LG_FW_MULTI_TOUCH
	input_set_abs_params(mcs6000_ts_input, ABS_MT_POSITION_X, ts_pdata->ts_x_min, ts_pdata->ts_x_max, 0, 0);
	input_set_abs_params(mcs6000_ts_input, ABS_MT_POSITION_Y, ts_pdata->ts_y_min, ts_pdata->ts_y_max, 0, 0);
#else	
	input_set_abs_params(mcs6000_ts_input, ABS_X, ts_pdata->ts_x_min, ts_pdata->ts_x_max, 0, 0);
	input_set_abs_params(mcs6000_ts_input, ABS_Y, ts_pdata->ts_y_min, ts_pdata->ts_y_max, 0, 0);
#endif

	dev = &mcs6000_ts_dev;

	INIT_DELAYED_WORK(&dev->work, mcs6000_work);

	dev->power = ts_pdata->power;	
	dev->num_irq = client->irq;
	dev->intr_gpio	= (client->irq) - NR_MSM_IRQS ;
	dev->sda_gpio = ts_pdata->sda;
	dev->scl_gpio  = ts_pdata->scl;

#if defined(CONFIG_MACH_MSM7X27_SU310) || defined(CONFIG_MACH_MSM7X27_LU3100)
#if defined(CONFIG_MACH_MSM7X27_SU310)
	if(lge_bd_rev < LGE_REV_C)
#elif defined(CONFIG_MACH_MSM7X27_LU3100)
	if(lge_bd_rev < LGE_REV_B)
#endif
	{
		KEY_MENU_TOUCHED 		= 2; 
		KEY_HOME_TOUCHED 		= 3;
		KEY_BACK_TOUCHED 		= 1;
		KEY_SEARCH_TOUCHED	= 4;
	}
	else
	{
		KEY_MENU_TOUCHED 		= 1; 
		KEY_HOME_TOUCHED 		= 2;
		KEY_BACK_TOUCHED 		= 3;
		KEY_SEARCH_TOUCHED	= 4;		
	}
#endif
#ifdef LG_FW_HARDKEY_BLOCK
	hrtimer_init(&dev->touch_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	dev->touch_timer.function = timed_touch_timer_func;
#endif

	dev->input_dev = mcs6000_ts_input;
	DMSG("mcs6000 dev->num_irq is %d , dev->intr_gpio is %d\n", dev->num_irq,dev->intr_gpio);

	dev->client = client;
	i2c_set_clientdata(client, dev);

	if (!(err = i2c_check_functionality(client->adapter, I2C_FUNC_I2C))) {
		printk(KERN_ERR "%s: fucntionality check failed\n",
				__FUNCTION__);
		return err;
	}

	err = gpio_direction_input(dev->intr_gpio);
	if (err < 0) {
		printk(KERN_ERR "%s: gpio input direction fail\n", __FUNCTION__);
		return err;
	}

	
	err = request_threaded_irq(dev->num_irq, NULL, mcs6000_ts_irq_handler,
			IRQF_TRIGGER_LOW | IRQF_ONESHOT, "mcs6000_ts", dev);

	if (err < 0) {
		printk(KERN_ERR "%s: request_irq failed\n", __FUNCTION__);
		return err;
	}

	disable_irq(dev->num_irq);
	mcs6000_ts_on();
	enable_irq(dev->num_irq);

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts_early_suspend.suspend = mcs6000_early_suspend;
	ts_early_suspend.resume = mcs6000_late_resume;
	ts_early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN +1 ;
	register_early_suspend(&ts_early_suspend);
#endif
	mcs6000_firmware_info(&fw_ver, &hw_ver);
	mcs6000_create_file(mcs6000_ts_input);  
	DMSG(KERN_INFO "%s: ts driver probed\n", __FUNCTION__);

	return 0;
}

static int mcs6000_ts_remove(struct i2c_client *client)
{
	struct mcs6000_ts_device *dev = i2c_get_clientdata(client);

	free_irq(dev->num_irq, dev);
	i2c_set_clientdata(client, NULL);

	return 0;
}

#ifndef CONFIG_HAS_EARLYSUSPEND
static int mcs6000_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct mcs6000_ts_device *dev = i2c_get_clientdata(client);

	if(is_downloading == 0)
	{
		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
		disable_irq(dev->num_irq);
		DMSG("%s: irq disable\n", __FUNCTION__);
		dev->power(OFF);
	}
	is_touch_suspend = 1;

	return 0;
}

static int mcs6000_ts_resume(struct i2c_client *client)
{
	struct mcs6000_ts_device *dev = i2c_get_clientdata(client);

	if(is_downloading == 0)
	{
		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
		dev->power(ON);
		enable_irq(dev->num_irq);
		DMSG("%s: irq enable\n", __FUNCTION__);
	}
	is_touch_suspend = 0;

	return 0;
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mcs6000_early_suspend(struct early_suspend * h)
{	
	struct mcs6000_ts_device *dev = &mcs6000_ts_dev;

	if(is_downloading == 0)
	{
		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
		disable_irq(dev->num_irq);
		DMSG("%s: irq disable\n", __FUNCTION__);
		dev->power(OFF);
	}
	is_touch_suspend = 1;
}

static void mcs6000_late_resume(struct early_suspend * h)
{	
	struct mcs6000_ts_device *dev = &mcs6000_ts_dev;

	if(is_downloading == 0)
	{
		DMSG(KERN_INFO"%s: start! \n", __FUNCTION__);
		mcs6000_ts_on();
		enable_irq(dev->num_irq);
		DMSG("%s: irq enable\n", __FUNCTION__);
	}
	is_touch_suspend = 0;
}
#endif

static const struct i2c_device_id mcs6000_ts_id[] = {
	{ "touch_mcs6000", 1 },	
	{ }
};


static struct i2c_driver mcs6000_i2c_ts_driver = {
	.probe = mcs6000_ts_probe,
	.remove = mcs6000_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = mcs6000_ts_suspend,
	.resume  = mcs6000_ts_resume,
#endif
	.id_table = mcs6000_ts_id,
	.driver = {
		.name = "touch_mcs6000",
		.owner = THIS_MODULE,
	},
};

static int __devinit mcs6000_ts_init(void)
{
	int err = 0;
	struct mcs6000_ts_device *dev;
	dev = &mcs6000_ts_dev;

	memset(&mcs6000_ts_dev, 0, sizeof(struct mcs6000_ts_device));

	mcs6000_ts_input = input_allocate_device();
	if (mcs6000_ts_input == NULL) {
		printk(KERN_ERR "%s: input_allocate: not enough memory\n",
				__FUNCTION__);
		err = -ENOMEM;
		goto err_input_allocate;
	}

	mcs6000_ts_input->name = "touch_mcs6000";

	set_bit(EV_SYN, 	 mcs6000_ts_input->evbit);
	set_bit(EV_KEY, 	 mcs6000_ts_input->evbit);
	set_bit(EV_ABS, 	 mcs6000_ts_input->evbit);
#ifdef LG_FW_MULTI_TOUCH
	set_bit(ABS_MT_TOUCH_MAJOR, mcs6000_ts_input->absbit);
#else
	set_bit(BTN_TOUCH, mcs6000_ts_input->keybit);
#endif
#if defined(LG_FW_TOUCH_SOFT_KEY) && defined(LG_FW_AUDIO_HAPTIC_TOUCH_SOFT_KEY)
	set_bit(TOUCH_BACK, mcs6000_ts_input->keybit);
	set_bit(TOUCH_MENU, mcs6000_ts_input->keybit);
	set_bit(TOUCH_HOME, mcs6000_ts_input->keybit);
	set_bit(TOUCH_SEARCH, mcs6000_ts_input->keybit);
#else
	set_bit(KEY_BACK, mcs6000_ts_input->keybit);
	set_bit(KEY_MENU, mcs6000_ts_input->keybit);
	set_bit(KEY_HOME, mcs6000_ts_input->keybit);
	set_bit(KEY_SEARCH, mcs6000_ts_input->keybit);
#endif

	err = input_register_device(mcs6000_ts_input);
	if (err < 0) {
		printk(KERN_ERR "%s: Fail to register device\n", __FUNCTION__);
		goto err_input_register;
	}

	err = i2c_add_driver(&mcs6000_i2c_ts_driver);
	if (err < 0) {
		printk(KERN_ERR "%s: failed to probe i2c \n", __FUNCTION__);
		goto err_i2c_add_driver;
	}

	err = misc_register(&mcs6000_ts_misc_dev);
	if (err < 0) {
		printk(KERN_ERR "%s: failed to misc register\n", __FUNCTION__);
		goto err_misc_register;
	}

	dev->ts_wq = create_singlethread_workqueue("ts_wq");
	return err;

err_misc_register:
	misc_deregister(&mcs6000_ts_misc_dev);
err_i2c_add_driver:
	i2c_del_driver(&mcs6000_i2c_ts_driver);
err_input_register:
	input_unregister_device(mcs6000_ts_input);
err_input_allocate:
	input_free_device(mcs6000_ts_input);
	mcs6000_ts_input = NULL;
	return err;
}

static void __exit mcs6000_ts_exit(void)
{
	mcs6000_remove_file(mcs6000_ts_input);
	i2c_del_driver(&mcs6000_i2c_ts_driver);
	input_unregister_device(mcs6000_ts_input);
	input_free_device(mcs6000_ts_input);

	printk(KERN_INFO "touchscreen driver was unloaded!\nHave a nice day!\n");
}

module_init(mcs6000_ts_init);
module_exit(mcs6000_ts_exit);

MODULE_DESCRIPTION("MELFAS MCS6000 Touchscreen Driver");
MODULE_LICENSE("GPL");

