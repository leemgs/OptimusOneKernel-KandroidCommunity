

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netfilter/x_tables.h>
#include <linux/leds.h>
#include <linux/mutex.h>

#include <linux/netfilter/xt_LED.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Adam Nielsen <a.nielsen@shikadi.net>");
MODULE_DESCRIPTION("Xtables: trigger LED devices on packet match");


struct xt_led_info_internal {
	struct led_trigger netfilter_led_trigger;
	struct timer_list timer;
};

static unsigned int
led_tg(struct sk_buff *skb, const struct xt_target_param *par)
{
	const struct xt_led_info *ledinfo = par->targinfo;
	struct xt_led_info_internal *ledinternal = ledinfo->internal_data;

	
	if ((ledinfo->delay > 0) && ledinfo->always_blink &&
	    timer_pending(&ledinternal->timer))
		led_trigger_event(&ledinternal->netfilter_led_trigger,LED_OFF);

	led_trigger_event(&ledinternal->netfilter_led_trigger, LED_FULL);

	
	if (ledinfo->delay > 0) {
		mod_timer(&ledinternal->timer,
			  jiffies + msecs_to_jiffies(ledinfo->delay));

	
	} else if (ledinfo->delay == 0) {
		led_trigger_event(&ledinternal->netfilter_led_trigger, LED_OFF);
	}

	

	return XT_CONTINUE;
}

static void led_timeout_callback(unsigned long data)
{
	struct xt_led_info *ledinfo = (struct xt_led_info *)data;
	struct xt_led_info_internal *ledinternal = ledinfo->internal_data;

	led_trigger_event(&ledinternal->netfilter_led_trigger, LED_OFF);
}

static bool led_tg_check(const struct xt_tgchk_param *par)
{
	struct xt_led_info *ledinfo = par->targinfo;
	struct xt_led_info_internal *ledinternal;
	int err;

	if (ledinfo->id[0] == '\0') {
		printk(KERN_ERR KBUILD_MODNAME ": No 'id' parameter given.\n");
		return false;
	}

	ledinternal = kzalloc(sizeof(struct xt_led_info_internal), GFP_KERNEL);
	if (!ledinternal) {
		printk(KERN_CRIT KBUILD_MODNAME ": out of memory\n");
		return false;
	}

	ledinternal->netfilter_led_trigger.name = ledinfo->id;

	err = led_trigger_register(&ledinternal->netfilter_led_trigger);
	if (err) {
		printk(KERN_CRIT KBUILD_MODNAME
			": led_trigger_register() failed\n");
		if (err == -EEXIST)
			printk(KERN_ERR KBUILD_MODNAME
				": Trigger name is already in use.\n");
		goto exit_alloc;
	}

	
	if (ledinfo->delay > 0)
		setup_timer(&ledinternal->timer, led_timeout_callback,
			    (unsigned long)ledinfo);

	ledinfo->internal_data = ledinternal;

	return true;

exit_alloc:
	kfree(ledinternal);

	return false;
}

static void led_tg_destroy(const struct xt_tgdtor_param *par)
{
	const struct xt_led_info *ledinfo = par->targinfo;
	struct xt_led_info_internal *ledinternal = ledinfo->internal_data;

	if (ledinfo->delay > 0)
		del_timer_sync(&ledinternal->timer);

	led_trigger_unregister(&ledinternal->netfilter_led_trigger);
	kfree(ledinternal);
}

static struct xt_target led_tg_reg __read_mostly = {
	.name		= "LED",
	.revision	= 0,
	.family		= NFPROTO_UNSPEC,
	.target		= led_tg,
	.targetsize	= XT_ALIGN(sizeof(struct xt_led_info)),
	.checkentry	= led_tg_check,
	.destroy	= led_tg_destroy,
	.me		= THIS_MODULE,
};

static int __init led_tg_init(void)
{
	return xt_register_target(&led_tg_reg);
}

static void __exit led_tg_exit(void)
{
	xt_unregister_target(&led_tg_reg);
}

module_init(led_tg_init);
module_exit(led_tg_exit);
