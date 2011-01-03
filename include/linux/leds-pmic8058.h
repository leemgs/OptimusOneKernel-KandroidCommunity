
#ifndef __LEDS_PMIC8058_H__
#define __LEDS_PMIC8058_H__

enum pmic8058_leds {
	PMIC8058_ID_LED_KB_LIGHT = 1,
	PMIC8058_ID_LED_0,
	PMIC8058_ID_LED_1,
	PMIC8058_ID_LED_2,
	PMIC8058_ID_FLASH_LED_0,
	PMIC8058_ID_FLASH_LED_1,
};

struct pmic8058_led {
	const char	*name;
	const char	*default_trigger;
	unsigned	max_brightness;
	int		id;
};

struct pmic8058_leds_platform_data {
	int	num_leds;
	struct pmic8058_led *leds;
};

int pm8058_set_flash_led_current(enum pmic8058_leds leds, unsigned mA);

#endif 
