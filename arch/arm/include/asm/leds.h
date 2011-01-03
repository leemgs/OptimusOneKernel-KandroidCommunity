
#ifndef ASM_ARM_LEDS_H
#define ASM_ARM_LEDS_H


typedef enum {
	led_idle_start,
	led_idle_end,
	led_timer,
	led_start,
	led_stop,
	led_claim,		
	led_release,		
	led_start_timer_mode,
	led_stop_timer_mode,
	led_green_on,
	led_green_off,
	led_amber_on,
	led_amber_off,
	led_red_on,
	led_red_off,
	led_blue_on,
	led_blue_off,
	
	led_halted
} led_event_t;



#ifdef CONFIG_LEDS
extern void (*leds_event)(led_event_t);
#else
#define leds_event(e)
#endif

#endif
