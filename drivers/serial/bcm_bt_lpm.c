

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/hrtimer.h>
#include <linux/irq.h>
#include <linux/serial_core.h>
#include <mach/bcm_bt_lpm.h>
#include <asm/gpio.h>



struct bcm_bt_lpm {
	unsigned int gpio_wake;
	unsigned int gpio_host_wake;

	int wake;
	int host_wake;

	struct hrtimer enter_lpm_timer;
	ktime_t enter_lpm_delay;

	struct uart_port *uport;

	void (*request_clock_off_locked)(struct uart_port *uport);
	void (*request_clock_on_locked)(struct uart_port *uport);
} bt_lpm;

static void set_wake_locked(int wake)
{
	if (wake == bt_lpm.wake)
		return;
	bt_lpm.wake = wake;

	if (wake || bt_lpm.host_wake)
		bt_lpm.request_clock_on_locked(bt_lpm.uport);
	else
		bt_lpm.request_clock_off_locked(bt_lpm.uport);

	gpio_set_value(bt_lpm.gpio_wake, wake);
}

static enum hrtimer_restart enter_lpm(struct hrtimer *timer) {
	unsigned long flags;

	spin_lock_irqsave(&bt_lpm.uport->lock, flags);
	set_wake_locked(0);
	spin_unlock_irqrestore(&bt_lpm.uport->lock, flags);

	return HRTIMER_NORESTART;
}

void bcm_bt_lpm_exit_lpm_locked(struct uart_port *uport) {
	bt_lpm.uport = uport;

	hrtimer_try_to_cancel(&bt_lpm.enter_lpm_timer);

	set_wake_locked(1);

	hrtimer_start(&bt_lpm.enter_lpm_timer, bt_lpm.enter_lpm_delay,
			HRTIMER_MODE_REL);
}
EXPORT_SYMBOL(bcm_bt_lpm_exit_lpm_locked);

static void update_host_wake_locked(int host_wake)
{
	if (host_wake == bt_lpm.host_wake)
		return;
	bt_lpm.host_wake = host_wake;

	if (bt_lpm.wake || host_wake)
		bt_lpm.request_clock_on_locked(bt_lpm.uport);
	else
		bt_lpm.request_clock_off_locked(bt_lpm.uport);
}

static irqreturn_t host_wake_isr(int irq, void *dev)
{
	int host_wake;
	unsigned long flags;

	host_wake = gpio_get_value(bt_lpm.gpio_host_wake);
	set_irq_type(irq, host_wake ? IRQF_TRIGGER_LOW : IRQF_TRIGGER_HIGH);

	if (!bt_lpm.uport) {
		bt_lpm.host_wake = host_wake;
		return IRQ_HANDLED;
	}

	spin_lock_irqsave(&bt_lpm.uport->lock, flags);

	update_host_wake_locked(host_wake);

	spin_unlock_irqrestore(&bt_lpm.uport->lock, flags);

	return IRQ_HANDLED;
}

static int bcm_bt_lpm_probe(struct platform_device *pdev)
{
	int irq;
	int ret;
	struct bcm_bt_lpm_platform_data *pdata = pdev->dev.platform_data;

	if (bt_lpm.request_clock_off_locked != NULL) {
		printk(KERN_ERR "Cannot register two bcm_bt_lpm drivers\n");
		return -EINVAL;
	}

	bt_lpm.gpio_wake = pdata->gpio_wake;
	bt_lpm.gpio_host_wake = pdata->gpio_host_wake;
	bt_lpm.request_clock_off_locked = pdata->request_clock_off_locked;
	bt_lpm.request_clock_on_locked = pdata->request_clock_on_locked;

	hrtimer_init(&bt_lpm.enter_lpm_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	bt_lpm.enter_lpm_delay = ktime_set(1, 0);  
	bt_lpm.enter_lpm_timer.function = enter_lpm;

	gpio_set_value(bt_lpm.gpio_wake, 0);
	bt_lpm.host_wake = 0;

	irq = gpio_to_irq(bt_lpm.gpio_host_wake);
	ret = request_irq(irq, host_wake_isr, IRQF_TRIGGER_HIGH,
			"bt host_wake", NULL);
	if (ret)
		return ret;
	ret = set_irq_wake(irq, 1);
	if (ret)
		return ret;

	return 0;
}

static struct platform_driver bcm_bt_lpm_driver = {
	.probe = bcm_bt_lpm_probe,
	.driver = {
		.name = "bcm_bt_lpm",
		.owner = THIS_MODULE,
	},
};

static int __init bcm_bt_lpm_init(void)
{
	return platform_driver_register(&bcm_bt_lpm_driver);
}

module_init(bcm_bt_lpm_init);
MODULE_DESCRIPTION("Broadcom Bluetooth low power mode driver");
MODULE_AUTHOR("Nick Pelly <npelly@google.com>");
MODULE_LICENSE("GPL");
