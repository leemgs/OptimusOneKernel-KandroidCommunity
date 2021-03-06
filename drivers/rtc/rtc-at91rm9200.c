

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/completion.h>

#include <asm/uaccess.h>

#include <mach/at91_rtc.h>


#define AT91_RTC_EPOCH		1900UL	

static DECLARE_COMPLETION(at91_rtc_updated);
static unsigned int at91_alarm_year = AT91_RTC_EPOCH;


static void at91_rtc_decodetime(unsigned int timereg, unsigned int calreg,
				struct rtc_time *tm)
{
	unsigned int time, date;

	
	do {
		time = at91_sys_read(timereg);
		date = at91_sys_read(calreg);
	} while ((time != at91_sys_read(timereg)) ||
			(date != at91_sys_read(calreg)));

	tm->tm_sec  = bcd2bin((time & AT91_RTC_SEC) >> 0);
	tm->tm_min  = bcd2bin((time & AT91_RTC_MIN) >> 8);
	tm->tm_hour = bcd2bin((time & AT91_RTC_HOUR) >> 16);

	
	tm->tm_year  = bcd2bin(date & AT91_RTC_CENT) * 100;	
	tm->tm_year += bcd2bin((date & AT91_RTC_YEAR) >> 8);	

	tm->tm_wday = bcd2bin((date & AT91_RTC_DAY) >> 21) - 1;	
	tm->tm_mon  = bcd2bin((date & AT91_RTC_MONTH) >> 16) - 1;
	tm->tm_mday = bcd2bin((date & AT91_RTC_DATE) >> 24);
}


static int at91_rtc_readtime(struct device *dev, struct rtc_time *tm)
{
	at91_rtc_decodetime(AT91_RTC_TIMR, AT91_RTC_CALR, tm);
	tm->tm_yday = rtc_year_days(tm->tm_mday, tm->tm_mon, tm->tm_year);
	tm->tm_year = tm->tm_year - 1900;

	pr_debug("%s(): %4d-%02d-%02d %02d:%02d:%02d\n", __func__,
		1900 + tm->tm_year, tm->tm_mon, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	return 0;
}


static int at91_rtc_settime(struct device *dev, struct rtc_time *tm)
{
	unsigned long cr;

	pr_debug("%s(): %4d-%02d-%02d %02d:%02d:%02d\n", __func__,
		1900 + tm->tm_year, tm->tm_mon, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	
	cr = at91_sys_read(AT91_RTC_CR);
	at91_sys_write(AT91_RTC_CR, cr | AT91_RTC_UPDCAL | AT91_RTC_UPDTIM);

	at91_sys_write(AT91_RTC_IER, AT91_RTC_ACKUPD);
	wait_for_completion(&at91_rtc_updated);	
	at91_sys_write(AT91_RTC_IDR, AT91_RTC_ACKUPD);

	at91_sys_write(AT91_RTC_TIMR,
			  bin2bcd(tm->tm_sec) << 0
			| bin2bcd(tm->tm_min) << 8
			| bin2bcd(tm->tm_hour) << 16);

	at91_sys_write(AT91_RTC_CALR,
			  bin2bcd((tm->tm_year + 1900) / 100)	
			| bin2bcd(tm->tm_year % 100) << 8	
			| bin2bcd(tm->tm_mon + 1) << 16		
			| bin2bcd(tm->tm_wday + 1) << 21	
			| bin2bcd(tm->tm_mday) << 24);

	
	cr = at91_sys_read(AT91_RTC_CR);
	at91_sys_write(AT91_RTC_CR, cr & ~(AT91_RTC_UPDCAL | AT91_RTC_UPDTIM));

	return 0;
}


static int at91_rtc_readalarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct rtc_time *tm = &alrm->time;

	at91_rtc_decodetime(AT91_RTC_TIMALR, AT91_RTC_CALALR, tm);
	tm->tm_yday = rtc_year_days(tm->tm_mday, tm->tm_mon, tm->tm_year);
	tm->tm_year = at91_alarm_year - 1900;

	alrm->enabled = (at91_sys_read(AT91_RTC_IMR) & AT91_RTC_ALARM)
			? 1 : 0;

	pr_debug("%s(): %4d-%02d-%02d %02d:%02d:%02d\n", __func__,
		1900 + tm->tm_year, tm->tm_mon, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	return 0;
}


static int at91_rtc_setalarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct rtc_time tm;

	at91_rtc_decodetime(AT91_RTC_TIMR, AT91_RTC_CALR, &tm);

	at91_alarm_year = tm.tm_year;

	tm.tm_hour = alrm->time.tm_hour;
	tm.tm_min = alrm->time.tm_min;
	tm.tm_sec = alrm->time.tm_sec;

	at91_sys_write(AT91_RTC_IDR, AT91_RTC_ALARM);
	at91_sys_write(AT91_RTC_TIMALR,
		  bin2bcd(tm.tm_sec) << 0
		| bin2bcd(tm.tm_min) << 8
		| bin2bcd(tm.tm_hour) << 16
		| AT91_RTC_HOUREN | AT91_RTC_MINEN | AT91_RTC_SECEN);
	at91_sys_write(AT91_RTC_CALALR,
		  bin2bcd(tm.tm_mon + 1) << 16		
		| bin2bcd(tm.tm_mday) << 24
		| AT91_RTC_DATEEN | AT91_RTC_MTHEN);

	if (alrm->enabled) {
		at91_sys_write(AT91_RTC_SCCR, AT91_RTC_ALARM);
		at91_sys_write(AT91_RTC_IER, AT91_RTC_ALARM);
	}

	pr_debug("%s(): %4d-%02d-%02d %02d:%02d:%02d\n", __func__,
		at91_alarm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour,
		tm.tm_min, tm.tm_sec);

	return 0;
}


static int at91_rtc_ioctl(struct device *dev, unsigned int cmd,
			unsigned long arg)
{
	int ret = 0;

	pr_debug("%s(): cmd=%08x, arg=%08lx.\n", __func__, cmd, arg);

	
	switch (cmd) {
	case RTC_AIE_OFF:	
		at91_sys_write(AT91_RTC_IDR, AT91_RTC_ALARM);
		break;
	case RTC_AIE_ON:	
		at91_sys_write(AT91_RTC_SCCR, AT91_RTC_ALARM);
		at91_sys_write(AT91_RTC_IER, AT91_RTC_ALARM);
		break;
	case RTC_UIE_OFF:	
		at91_sys_write(AT91_RTC_IDR, AT91_RTC_SECEV);
		break;
	case RTC_UIE_ON:	
		at91_sys_write(AT91_RTC_SCCR, AT91_RTC_SECEV);
		at91_sys_write(AT91_RTC_IER, AT91_RTC_SECEV);
		break;
	default:
		ret = -ENOIOCTLCMD;
		break;
	}

	return ret;
}


static int at91_rtc_proc(struct device *dev, struct seq_file *seq)
{
	unsigned long imr = at91_sys_read(AT91_RTC_IMR);

	seq_printf(seq, "update_IRQ\t: %s\n",
			(imr & AT91_RTC_ACKUPD) ? "yes" : "no");
	seq_printf(seq, "periodic_IRQ\t: %s\n",
			(imr & AT91_RTC_SECEV) ? "yes" : "no");

	return 0;
}


static irqreturn_t at91_rtc_interrupt(int irq, void *dev_id)
{
	struct platform_device *pdev = dev_id;
	struct rtc_device *rtc = platform_get_drvdata(pdev);
	unsigned int rtsr;
	unsigned long events = 0;

	rtsr = at91_sys_read(AT91_RTC_SR) & at91_sys_read(AT91_RTC_IMR);
	if (rtsr) {		
		if (rtsr & AT91_RTC_ALARM)
			events |= (RTC_AF | RTC_IRQF);
		if (rtsr & AT91_RTC_SECEV)
			events |= (RTC_UF | RTC_IRQF);
		if (rtsr & AT91_RTC_ACKUPD)
			complete(&at91_rtc_updated);

		at91_sys_write(AT91_RTC_SCCR, rtsr);	

		rtc_update_irq(rtc, 1, events);

		pr_debug("%s(): num=%ld, events=0x%02lx\n", __func__,
			events >> 8, events & 0x000000FF);

		return IRQ_HANDLED;
	}
	return IRQ_NONE;		
}

static const struct rtc_class_ops at91_rtc_ops = {
	.ioctl		= at91_rtc_ioctl,
	.read_time	= at91_rtc_readtime,
	.set_time	= at91_rtc_settime,
	.read_alarm	= at91_rtc_readalarm,
	.set_alarm	= at91_rtc_setalarm,
	.proc		= at91_rtc_proc,
};


static int __init at91_rtc_probe(struct platform_device *pdev)
{
	struct rtc_device *rtc;
	int ret;

	at91_sys_write(AT91_RTC_CR, 0);
	at91_sys_write(AT91_RTC_MR, 0);		

	
	at91_sys_write(AT91_RTC_IDR, AT91_RTC_ACKUPD | AT91_RTC_ALARM |
					AT91_RTC_SECEV | AT91_RTC_TIMEV |
					AT91_RTC_CALEV);

	ret = request_irq(AT91_ID_SYS, at91_rtc_interrupt,
				IRQF_SHARED,
				"at91_rtc", pdev);
	if (ret) {
		printk(KERN_ERR "at91_rtc: IRQ %d already in use.\n",
				AT91_ID_SYS);
		return ret;
	}

	
	if (!device_can_wakeup(&pdev->dev))
		device_init_wakeup(&pdev->dev, 1);

	rtc = rtc_device_register(pdev->name, &pdev->dev,
				&at91_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc)) {
		free_irq(AT91_ID_SYS, pdev);
		return PTR_ERR(rtc);
	}
	platform_set_drvdata(pdev, rtc);

	printk(KERN_INFO "AT91 Real Time Clock driver.\n");
	return 0;
}


static int __exit at91_rtc_remove(struct platform_device *pdev)
{
	struct rtc_device *rtc = platform_get_drvdata(pdev);

	
	at91_sys_write(AT91_RTC_IDR, AT91_RTC_ACKUPD | AT91_RTC_ALARM |
					AT91_RTC_SECEV | AT91_RTC_TIMEV |
					AT91_RTC_CALEV);
	free_irq(AT91_ID_SYS, pdev);

	rtc_device_unregister(rtc);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

#ifdef CONFIG_PM



static u32 at91_rtc_imr;

static int at91_rtc_suspend(struct device *dev)
{
	
	at91_rtc_imr = at91_sys_read(AT91_RTC_IMR)
			& (AT91_RTC_ALARM|AT91_RTC_SECEV);
	if (at91_rtc_imr) {
		if (device_may_wakeup(dev))
			enable_irq_wake(AT91_ID_SYS);
		else
			at91_sys_write(AT91_RTC_IDR, at91_rtc_imr);
	}
	return 0;
}

static int at91_rtc_resume(struct device *dev)
{
	if (at91_rtc_imr) {
		if (device_may_wakeup(dev))
			disable_irq_wake(AT91_ID_SYS);
		else
			at91_sys_write(AT91_RTC_IER, at91_rtc_imr);
	}
	return 0;
}

static const struct dev_pm_ops at91_rtc_pm = {
	.suspend =	at91_rtc_suspend,
	.resume =	at91_rtc_resume,
};

#define at91_rtc_pm_ptr	&at91_rtc_pm

#else
#define at91_rtc_pm_ptr	NULL
#endif

static struct platform_driver at91_rtc_driver = {
	.remove		= __exit_p(at91_rtc_remove),
	.driver		= {
		.name	= "at91_rtc",
		.owner	= THIS_MODULE,
		.pm	= at91_rtc_pm_ptr,
	},
};

static int __init at91_rtc_init(void)
{
	return platform_driver_probe(&at91_rtc_driver, at91_rtc_probe);
}

static void __exit at91_rtc_exit(void)
{
	platform_driver_unregister(&at91_rtc_driver);
}

module_init(at91_rtc_init);
module_exit(at91_rtc_exit);

MODULE_AUTHOR("Rick Bronson");
MODULE_DESCRIPTION("RTC driver for Atmel AT91RM9200");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:at91_rtc");
