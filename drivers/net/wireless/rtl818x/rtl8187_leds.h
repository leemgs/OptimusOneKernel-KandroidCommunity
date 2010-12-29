

#ifndef RTL8187_LED_H
#define RTL8187_LED_H

#ifdef CONFIG_RTL8187_LEDS

#define RTL8187_LED_MAX_NAME_LEN	21

#include <linux/leds.h>
#include <linux/types.h>

enum {
	LED_PIN_LED0,
	LED_PIN_LED1,
	LED_PIN_GPIO0,
	LED_PIN_HW
};

enum {
	EEPROM_CID_RSVD0 = 0x00,
	EEPROM_CID_RSVD1 = 0xFF,
	EEPROM_CID_ALPHA0 = 0x01,
	EEPROM_CID_SERCOMM_PS = 0x02,
	EEPROM_CID_HW = 0x03,
	EEPROM_CID_TOSHIBA = 0x04,
	EEPROM_CID_QMI = 0x07,
	EEPROM_CID_DELL = 0x08
};

struct rtl8187_led {
	struct ieee80211_hw *dev;
	
	struct led_classdev led_dev;
	
	u8 ledpin;
	
	char name[RTL8187_LED_MAX_NAME_LEN + 1];
};

void rtl8187_leds_init(struct ieee80211_hw *dev, u16 code);
void rtl8187_leds_exit(struct ieee80211_hw *dev);

#endif 

#endif 
