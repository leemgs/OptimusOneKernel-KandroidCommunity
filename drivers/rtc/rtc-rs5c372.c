

#include <linux/i2c.h>
#include <linux/rtc.h>
#include <linux/bcd.h>

#define DRV_VERSION "0.6"



#define RS5C372_REG_SECS	0
#define RS5C372_REG_MINS	1
#define RS5C372_REG_HOURS	2
#define RS5C372_REG_WDAY	3
#define RS5C372_REG_DAY		4
#define RS5C372_REG_MONTH	5
#define RS5C372_REG_YEAR	6
#define RS5C372_REG_TRIM	7
#	define RS5C372_TRIM_XSL		0x80
#	define RS5C372_TRIM_MASK	0x7F

#define RS5C_REG_ALARM_A_MIN	8			
#define RS5C_REG_ALARM_A_HOURS	9
#define RS5C_REG_ALARM_A_WDAY	10

#define RS5C_REG_ALARM_B_MIN	11			
#define RS5C_REG_ALARM_B_HOURS	12
#define RS5C_REG_ALARM_B_WDAY	13			

#define RS5C_REG_CTRL1		14
#	define RS5C_CTRL1_AALE		(1 << 7)	
#	define RS5C_CTRL1_BALE		(1 << 6)	
#	define RV5C387_CTRL1_24		(1 << 5)
#	define RS5C372A_CTRL1_SL1	(1 << 5)
#	define RS5C_CTRL1_CT_MASK	(7 << 0)
#	define RS5C_CTRL1_CT0		(0 << 0)	
#	define RS5C_CTRL1_CT4		(4 << 0)	
#define RS5C_REG_CTRL2		15
#	define RS5C372_CTRL2_24		(1 << 5)
#	define R2025_CTRL2_XST		(1 << 5)
#	define RS5C_CTRL2_XSTP		(1 << 4)	
#	define RS5C_CTRL2_CTFG		(1 << 2)
#	define RS5C_CTRL2_AAFG		(1 << 1)	
#	define RS5C_CTRL2_BAFG		(1 << 0)	



#define RS5C_ADDR(R)		(((R) << 4) | 0)


enum rtc_type {
	rtc_undef = 0,
	rtc_r2025sd,
	rtc_rs5c372a,
	rtc_rs5c372b,
	rtc_rv5c386,
	rtc_rv5c387a,
};

static const struct i2c_device_id rs5c372_id[] = {
	{ "r2025sd", rtc_r2025sd },
	{ "rs5c372a", rtc_rs5c372a },
	{ "rs5c372b", rtc_rs5c372b },
	{ "rv5c386", rtc_rv5c386 },
	{ "rv5c387a", rtc_rv5c387a },
	{ }
};
MODULE_DEVICE_TABLE(i2c, rs5c372_id);


struct rs5c372 {
	struct i2c_client	*client;
	struct rtc_device	*rtc;
	enum rtc_type		type;
	unsigned		time24:1;
	unsigned		has_irq:1;
	unsigned		smbus:1;
	char			buf[17];
	char			*regs;
};

static int rs5c_get_regs(struct rs5c372 *rs5c)
{
	struct i2c_client	*client = rs5c->client;
	struct i2c_msg		msgs[] = {
		{ client->addr, I2C_M_RD, sizeof rs5c->buf, rs5c->buf },
	};

	
	if (rs5c->smbus) {
		int addr = RS5C_ADDR(RS5C372_REG_SECS);
		int size = sizeof(rs5c->buf) - 1;

		if (i2c_smbus_read_i2c_block_data(client, addr, size,
						  rs5c->buf + 1) != size) {
			dev_warn(&client->dev, "can't read registers\n");
			return -EIO;
		}
	} else {
		if ((i2c_transfer(client->adapter, msgs, 1)) != 1) {
			dev_warn(&client->dev, "can't read registers\n");
			return -EIO;
		}
	}

	dev_dbg(&client->dev,
		"%02x %02x %02x (%02x) %02x %02x %02x (%02x), "
		"%02x %02x %02x, %02x %02x %02x; %02x %02x\n",
		rs5c->regs[0],  rs5c->regs[1],  rs5c->regs[2],  rs5c->regs[3],
		rs5c->regs[4],  rs5c->regs[5],  rs5c->regs[6],  rs5c->regs[7],
		rs5c->regs[8],  rs5c->regs[9],  rs5c->regs[10], rs5c->regs[11],
		rs5c->regs[12], rs5c->regs[13], rs5c->regs[14], rs5c->regs[15]);

	return 0;
}

static unsigned rs5c_reg2hr(struct rs5c372 *rs5c, unsigned reg)
{
	unsigned	hour;

	if (rs5c->time24)
		return bcd2bin(reg & 0x3f);

	hour = bcd2bin(reg & 0x1f);
	if (hour == 12)
		hour = 0;
	if (reg & 0x20)
		hour += 12;
	return hour;
}

static unsigned rs5c_hr2reg(struct rs5c372 *rs5c, unsigned hour)
{
	if (rs5c->time24)
		return bin2bcd(hour);

	if (hour > 12)
		return 0x20 | bin2bcd(hour - 12);
	if (hour == 12)
		return 0x20 | bin2bcd(12);
	if (hour == 0)
		return bin2bcd(12);
	return bin2bcd(hour);
}

static int rs5c372_get_datetime(struct i2c_client *client, struct rtc_time *tm)
{
	struct rs5c372	*rs5c = i2c_get_clientdata(client);
	int		status = rs5c_get_regs(rs5c);

	if (status < 0)
		return status;

	tm->tm_sec = bcd2bin(rs5c->regs[RS5C372_REG_SECS] & 0x7f);
	tm->tm_min = bcd2bin(rs5c->regs[RS5C372_REG_MINS] & 0x7f);
	tm->tm_hour = rs5c_reg2hr(rs5c, rs5c->regs[RS5C372_REG_HOURS]);

	tm->tm_wday = bcd2bin(rs5c->regs[RS5C372_REG_WDAY] & 0x07);
	tm->tm_mday = bcd2bin(rs5c->regs[RS5C372_REG_DAY] & 0x3f);

	
	tm->tm_mon = bcd2bin(rs5c->regs[RS5C372_REG_MONTH] & 0x1f) - 1;

	
	tm->tm_year = bcd2bin(rs5c->regs[RS5C372_REG_YEAR]) + 100;

	dev_dbg(&client->dev, "%s: tm is secs=%d, mins=%d, hours=%d, "
		"mday=%d, mon=%d, year=%d, wday=%d\n",
		__func__,
		tm->tm_sec, tm->tm_min, tm->tm_hour,
		tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);

	
	return rtc_valid_tm(tm);
}

static int rs5c372_set_datetime(struct i2c_client *client, struct rtc_time *tm)
{
	struct rs5c372	*rs5c = i2c_get_clientdata(client);
	unsigned char	buf[8];
	int		addr;

	dev_dbg(&client->dev, "%s: tm is secs=%d, mins=%d, hours=%d "
		"mday=%d, mon=%d, year=%d, wday=%d\n",
		__func__,
		tm->tm_sec, tm->tm_min, tm->tm_hour,
		tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);

	addr   = RS5C_ADDR(RS5C372_REG_SECS);
	buf[0] = bin2bcd(tm->tm_sec);
	buf[1] = bin2bcd(tm->tm_min);
	buf[2] = rs5c_hr2reg(rs5c, tm->tm_hour);
	buf[3] = bin2bcd(tm->tm_wday);
	buf[4] = bin2bcd(tm->tm_mday);
	buf[5] = bin2bcd(tm->tm_mon + 1);
	buf[6] = bin2bcd(tm->tm_year - 100);

	if (i2c_smbus_write_i2c_block_data(client, addr, sizeof(buf), buf) < 0) {
		dev_err(&client->dev, "%s: write error\n", __func__);
		return -EIO;
	}

	return 0;
}

#if defined(CONFIG_RTC_INTF_PROC) || defined(CONFIG_RTC_INTF_PROC_MODULE)
#define	NEED_TRIM
#endif

#if defined(CONFIG_RTC_INTF_SYSFS) || defined(CONFIG_RTC_INTF_SYSFS_MODULE)
#define	NEED_TRIM
#endif

#ifdef	NEED_TRIM
static int rs5c372_get_trim(struct i2c_client *client, int *osc, int *trim)
{
	struct rs5c372 *rs5c372 = i2c_get_clientdata(client);
	u8 tmp = rs5c372->regs[RS5C372_REG_TRIM];

	if (osc)
		*osc = (tmp & RS5C372_TRIM_XSL) ? 32000 : 32768;

	if (trim) {
		dev_dbg(&client->dev, "%s: raw trim=%x\n", __func__, tmp);
		tmp &= RS5C372_TRIM_MASK;
		if (tmp & 0x3e) {
			int t = tmp & 0x3f;

			if (tmp & 0x40)
				t = (~t | (s8)0xc0) + 1;
			else
				t = t - 1;

			tmp = t * 2;
		} else
			tmp = 0;
		*trim = tmp;
	}

	return 0;
}
#endif

static int rs5c372_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	return rs5c372_get_datetime(to_i2c_client(dev), tm);
}

static int rs5c372_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	return rs5c372_set_datetime(to_i2c_client(dev), tm);
}

#if defined(CONFIG_RTC_INTF_DEV) || defined(CONFIG_RTC_INTF_DEV_MODULE)

static int
rs5c_rtc_ioctl(struct device *dev, unsigned int cmd, unsigned long arg)
{
	struct i2c_client	*client = to_i2c_client(dev);
	struct rs5c372		*rs5c = i2c_get_clientdata(client);
	unsigned char		buf;
	int			status, addr;

	buf = rs5c->regs[RS5C_REG_CTRL1];
	switch (cmd) {
	case RTC_UIE_OFF:
	case RTC_UIE_ON:
		
		if (rs5c->type == rtc_rs5c372a
				&& (buf & RS5C372A_CTRL1_SL1))
			return -ENOIOCTLCMD;
	case RTC_AIE_OFF:
	case RTC_AIE_ON:
		
		if (!rs5c->has_irq)
			return -ENOIOCTLCMD;
		break;
	default:
		return -ENOIOCTLCMD;
	}

	status = rs5c_get_regs(rs5c);
	if (status < 0)
		return status;

	addr = RS5C_ADDR(RS5C_REG_CTRL1);
	switch (cmd) {
	case RTC_AIE_OFF:	
		buf &= ~RS5C_CTRL1_AALE;
		break;
	case RTC_AIE_ON:	
		buf |= RS5C_CTRL1_AALE;
		break;
	case RTC_UIE_OFF:	
		buf &= ~RS5C_CTRL1_CT_MASK;
		break;
	case RTC_UIE_ON:	
		buf &= ~RS5C_CTRL1_CT_MASK;
		buf |= RS5C_CTRL1_CT4;
		break;
	}

	if (i2c_smbus_write_byte_data(client, addr, buf) < 0) {
		printk(KERN_WARNING "%s: can't update alarm\n",
			rs5c->rtc->name);
		status = -EIO;
	} else
		rs5c->regs[RS5C_REG_CTRL1] = buf;

	return status;
}

#else
#define	rs5c_rtc_ioctl	NULL
#endif




static int rs5c_read_alarm(struct device *dev, struct rtc_wkalrm *t)
{
	struct i2c_client	*client = to_i2c_client(dev);
	struct rs5c372		*rs5c = i2c_get_clientdata(client);
	int			status;

	status = rs5c_get_regs(rs5c);
	if (status < 0)
		return status;

	
	t->time.tm_sec = 0;
	t->time.tm_min = bcd2bin(rs5c->regs[RS5C_REG_ALARM_A_MIN] & 0x7f);
	t->time.tm_hour = rs5c_reg2hr(rs5c, rs5c->regs[RS5C_REG_ALARM_A_HOURS]);
	t->time.tm_mday = -1;
	t->time.tm_mon = -1;
	t->time.tm_year = -1;
	t->time.tm_wday = -1;
	t->time.tm_yday = -1;
	t->time.tm_isdst = -1;

	
	t->enabled = !!(rs5c->regs[RS5C_REG_CTRL1] & RS5C_CTRL1_AALE);
	t->pending = !!(rs5c->regs[RS5C_REG_CTRL2] & RS5C_CTRL2_AAFG);

	return 0;
}

static int rs5c_set_alarm(struct device *dev, struct rtc_wkalrm *t)
{
	struct i2c_client	*client = to_i2c_client(dev);
	struct rs5c372		*rs5c = i2c_get_clientdata(client);
	int			status, addr, i;
	unsigned char		buf[3];

	
	if (t->time.tm_mday != -1
			|| t->time.tm_mon != -1
			|| t->time.tm_year != -1)
		return -EINVAL;

	

	
	status = rs5c_get_regs(rs5c);
	if (status < 0)
		return status;
	if (rs5c->regs[RS5C_REG_CTRL1] & RS5C_CTRL1_AALE) {
		addr = RS5C_ADDR(RS5C_REG_CTRL1);
		buf[0] = rs5c->regs[RS5C_REG_CTRL1] & ~RS5C_CTRL1_AALE;
		if (i2c_smbus_write_byte_data(client, addr, buf[0]) < 0) {
			pr_debug("%s: can't disable alarm\n", rs5c->rtc->name);
			return -EIO;
		}
		rs5c->regs[RS5C_REG_CTRL1] = buf[0];
	}

	
	buf[0] = bin2bcd(t->time.tm_min);
	buf[1] = rs5c_hr2reg(rs5c, t->time.tm_hour);
	buf[2] = 0x7f;	

	for (i = 0; i < sizeof(buf); i++) {
		addr = RS5C_ADDR(RS5C_REG_ALARM_A_MIN + i);
		if (i2c_smbus_write_byte_data(client, addr, buf[i]) < 0) {
			pr_debug("%s: can't set alarm time\n", rs5c->rtc->name);
			return -EIO;
		}
	}

	
	if (t->enabled) {
		addr = RS5C_ADDR(RS5C_REG_CTRL1);
		buf[0] = rs5c->regs[RS5C_REG_CTRL1] | RS5C_CTRL1_AALE;
		if (i2c_smbus_write_byte_data(client, addr, buf[0]) < 0)
			printk(KERN_WARNING "%s: can't enable alarm\n",
				rs5c->rtc->name);
		rs5c->regs[RS5C_REG_CTRL1] = buf[0];
	}

	return 0;
}

#if defined(CONFIG_RTC_INTF_PROC) || defined(CONFIG_RTC_INTF_PROC_MODULE)

static int rs5c372_rtc_proc(struct device *dev, struct seq_file *seq)
{
	int err, osc, trim;

	err = rs5c372_get_trim(to_i2c_client(dev), &osc, &trim);
	if (err == 0) {
		seq_printf(seq, "crystal\t\t: %d.%03d KHz\n",
				osc / 1000, osc % 1000);
		seq_printf(seq, "trim\t\t: %d\n", trim);
	}

	return 0;
}

#else
#define	rs5c372_rtc_proc	NULL
#endif

static const struct rtc_class_ops rs5c372_rtc_ops = {
	.proc		= rs5c372_rtc_proc,
	.ioctl		= rs5c_rtc_ioctl,
	.read_time	= rs5c372_rtc_read_time,
	.set_time	= rs5c372_rtc_set_time,
	.read_alarm	= rs5c_read_alarm,
	.set_alarm	= rs5c_set_alarm,
};

#if defined(CONFIG_RTC_INTF_SYSFS) || defined(CONFIG_RTC_INTF_SYSFS_MODULE)

static ssize_t rs5c372_sysfs_show_trim(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int err, trim;

	err = rs5c372_get_trim(to_i2c_client(dev), NULL, &trim);
	if (err)
		return err;

	return sprintf(buf, "%d\n", trim);
}
static DEVICE_ATTR(trim, S_IRUGO, rs5c372_sysfs_show_trim, NULL);

static ssize_t rs5c372_sysfs_show_osc(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int err, osc;

	err = rs5c372_get_trim(to_i2c_client(dev), &osc, NULL);
	if (err)
		return err;

	return sprintf(buf, "%d.%03d KHz\n", osc / 1000, osc % 1000);
}
static DEVICE_ATTR(osc, S_IRUGO, rs5c372_sysfs_show_osc, NULL);

static int rs5c_sysfs_register(struct device *dev)
{
	int err;

	err = device_create_file(dev, &dev_attr_trim);
	if (err)
		return err;
	err = device_create_file(dev, &dev_attr_osc);
	if (err)
		device_remove_file(dev, &dev_attr_trim);

	return err;
}

static void rs5c_sysfs_unregister(struct device *dev)
{
	device_remove_file(dev, &dev_attr_trim);
	device_remove_file(dev, &dev_attr_osc);
}

#else
static int rs5c_sysfs_register(struct device *dev)
{
	return 0;
}

static void rs5c_sysfs_unregister(struct device *dev)
{
	
}
#endif	

static struct i2c_driver rs5c372_driver;

static int rs5c_oscillator_setup(struct rs5c372 *rs5c372)
{
	unsigned char buf[2];
	int addr, i, ret = 0;

	if (rs5c372->type == rtc_r2025sd) {
		if (!(rs5c372->regs[RS5C_REG_CTRL2] & R2025_CTRL2_XST))
			return ret;
		rs5c372->regs[RS5C_REG_CTRL2] &= ~R2025_CTRL2_XST;
	} else {
		if (!(rs5c372->regs[RS5C_REG_CTRL2] & RS5C_CTRL2_XSTP))
			return ret;
		rs5c372->regs[RS5C_REG_CTRL2] &= ~RS5C_CTRL2_XSTP;
	}

	addr   = RS5C_ADDR(RS5C_REG_CTRL1);
	buf[0] = rs5c372->regs[RS5C_REG_CTRL1];
	buf[1] = rs5c372->regs[RS5C_REG_CTRL2];

	
	switch (rs5c372->type) {
	case rtc_rs5c372a:
	case rtc_rs5c372b:
		buf[1] |= RS5C372_CTRL2_24;
		rs5c372->time24 = 1;
		break;
	case rtc_r2025sd:
	case rtc_rv5c386:
	case rtc_rv5c387a:
		buf[0] |= RV5C387_CTRL1_24;
		rs5c372->time24 = 1;
		break;
	default:
		
		break;
	}

	for (i = 0; i < sizeof(buf); i++) {
		addr = RS5C_ADDR(RS5C_REG_CTRL1 + i);
		ret = i2c_smbus_write_byte_data(rs5c372->client, addr, buf[i]);
		if (unlikely(ret < 0))
			return ret;
	}

	rs5c372->regs[RS5C_REG_CTRL1] = buf[0];
	rs5c372->regs[RS5C_REG_CTRL2] = buf[1];

	return 0;
}

static int rs5c372_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int err = 0;
	int smbus_mode = 0;
	struct rs5c372 *rs5c372;
	struct rtc_time tm;

	dev_dbg(&client->dev, "%s\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C |
			I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_I2C_BLOCK)) {
		
		if (i2c_check_functionality(client->adapter,
				I2C_FUNC_SMBUS_BYTE_DATA |
				I2C_FUNC_SMBUS_I2C_BLOCK))
			smbus_mode = 1;
		else {
			
			err = -ENODEV;
			goto exit;
		}
	}

	if (!(rs5c372 = kzalloc(sizeof(struct rs5c372), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit;
	}

	rs5c372->client = client;
	i2c_set_clientdata(client, rs5c372);
	rs5c372->type = id->driver_data;

	
	rs5c372->regs = &rs5c372->buf[1];
	rs5c372->smbus = smbus_mode;

	err = rs5c_get_regs(rs5c372);
	if (err < 0)
		goto exit_kfree;

	
	switch (rs5c372->type) {
	case rtc_rs5c372a:
	case rtc_rs5c372b:
		
		if (rs5c372->regs[RS5C_REG_CTRL2] & RS5C372_CTRL2_24)
			rs5c372->time24 = 1;
		break;
	case rtc_r2025sd:
	case rtc_rv5c386:
	case rtc_rv5c387a:
		if (rs5c372->regs[RS5C_REG_CTRL1] & RV5C387_CTRL1_24)
			rs5c372->time24 = 1;
		
		break;
	default:
		dev_err(&client->dev, "unknown RTC type\n");
		goto exit_kfree;
	}

	
	err = rs5c_oscillator_setup(rs5c372);
	if (unlikely(err < 0)) {
		dev_err(&client->dev, "setup error\n");
		goto exit_kfree;
	}

	if (rs5c372_get_datetime(client, &tm) < 0)
		dev_warn(&client->dev, "clock needs to be set\n");

	dev_info(&client->dev, "%s found, %s, driver version " DRV_VERSION "\n",
			({ char *s; switch (rs5c372->type) {
			case rtc_r2025sd:	s = "r2025sd"; break;
			case rtc_rs5c372a:	s = "rs5c372a"; break;
			case rtc_rs5c372b:	s = "rs5c372b"; break;
			case rtc_rv5c386:	s = "rv5c386"; break;
			case rtc_rv5c387a:	s = "rv5c387a"; break;
			default:		s = "chip"; break;
			}; s;}),
			rs5c372->time24 ? "24hr" : "am/pm"
			);

	

	rs5c372->rtc = rtc_device_register(rs5c372_driver.driver.name,
				&client->dev, &rs5c372_rtc_ops, THIS_MODULE);

	if (IS_ERR(rs5c372->rtc)) {
		err = PTR_ERR(rs5c372->rtc);
		goto exit_kfree;
	}

	err = rs5c_sysfs_register(&client->dev);
	if (err)
		goto exit_devreg;

	return 0;

exit_devreg:
	rtc_device_unregister(rs5c372->rtc);

exit_kfree:
	kfree(rs5c372);

exit:
	return err;
}

static int rs5c372_remove(struct i2c_client *client)
{
	struct rs5c372 *rs5c372 = i2c_get_clientdata(client);

	rtc_device_unregister(rs5c372->rtc);
	rs5c_sysfs_unregister(&client->dev);
	kfree(rs5c372);
	return 0;
}

static struct i2c_driver rs5c372_driver = {
	.driver		= {
		.name	= "rtc-rs5c372",
	},
	.probe		= rs5c372_probe,
	.remove		= rs5c372_remove,
	.id_table	= rs5c372_id,
};

static __init int rs5c372_init(void)
{
	return i2c_add_driver(&rs5c372_driver);
}

static __exit void rs5c372_exit(void)
{
	i2c_del_driver(&rs5c372_driver);
}

module_init(rs5c372_init);
module_exit(rs5c372_exit);

MODULE_AUTHOR(
		"Pavel Mironchik <pmironchik@optifacio.net>, "
		"Alessandro Zummo <a.zummo@towertech.it>, "
		"Paul Mundt <lethal@linux-sh.org>");
MODULE_DESCRIPTION("Ricoh RS5C372 RTC driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
