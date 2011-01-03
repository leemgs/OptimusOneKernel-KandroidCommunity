#ifndef _GPIO_KEYS_H
#define _GPIO_KEYS_H

struct gpio_keys_button {
	
	int code;		
	int gpio;
	int active_low;
	char *desc;
	int type;		
	int wakeup;		
	int debounce_interval;	
};

struct gpio_keys_platform_data {
	struct gpio_keys_button *buttons;
	int nbuttons;
	unsigned int rep:1;		
};

#endif
