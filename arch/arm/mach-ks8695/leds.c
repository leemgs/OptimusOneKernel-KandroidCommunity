

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <asm/leds.h>
#include <mach/devices.h>
#include <mach/gpio.h>


static inline void ks8695_led_on(unsigned int led)
{
	gpio_set_value(led, 0);
}

static inline void ks8695_led_off(unsigned int led)
{
	gpio_set_value(led, 1);
}

static inline void ks8695_led_toggle(unsigned int led)
{
	unsigned long is_off = gpio_get_value(led);
	if (is_off)
		ks8695_led_on(led);
	else
		ks8695_led_off(led);
}



static void ks8695_leds_event(led_event_t evt)
{
	unsigned long flags;

	local_irq_save(flags);

	switch(evt) {
	case led_start:		
		ks8695_led_on(ks8695_leds_cpu);
		break;

	case led_stop:		
		ks8695_led_off(ks8695_leds_cpu);
		break;

#ifdef CONFIG_LEDS_TIMER
	case led_timer:		
		ks8695_led_toggle(ks8695_leds_timer);
		break;
#endif

#ifdef CONFIG_LEDS_CPU
	case led_idle_start:	
		ks8695_led_off(ks8695_leds_cpu);
		break;

	case led_idle_end:	
		ks8695_led_on(ks8695_leds_cpu);
		break;
#endif

	default:
		break;
	}

	local_irq_restore(flags);
}


static int __init leds_init(void)
{
	if ((ks8695_leds_timer == -1) || (ks8695_leds_cpu == -1))
		return -ENODEV;

	leds_event = ks8695_leds_event;

	leds_event(led_start);
	return 0;
}

__initcall(leds_init);
