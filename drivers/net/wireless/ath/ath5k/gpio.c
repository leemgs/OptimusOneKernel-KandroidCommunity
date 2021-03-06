



#include "ath5k.h"
#include "reg.h"
#include "debug.h"
#include "base.h"


void ath5k_hw_set_ledstate(struct ath5k_hw *ah, unsigned int state)
{
	u32 led;
	
	u32 led_5210;

	ATH5K_TRACE(ah->ah_sc);

	
	if (ah->ah_version != AR5K_AR5210)
		AR5K_REG_DISABLE_BITS(ah, AR5K_PCICFG,
			AR5K_PCICFG_LEDMODE |  AR5K_PCICFG_LED);
	else
		AR5K_REG_DISABLE_BITS(ah, AR5K_PCICFG, AR5K_PCICFG_LED);

	
	switch (state) {
	case AR5K_LED_SCAN:
	case AR5K_LED_AUTH:
		led = AR5K_PCICFG_LEDMODE_PROP | AR5K_PCICFG_LED_PEND;
		led_5210 = AR5K_PCICFG_LED_PEND | AR5K_PCICFG_LED_BCTL;
		break;

	case AR5K_LED_INIT:
		led = AR5K_PCICFG_LEDMODE_PROP | AR5K_PCICFG_LED_NONE;
		led_5210 = AR5K_PCICFG_LED_PEND;
		break;

	case AR5K_LED_ASSOC:
	case AR5K_LED_RUN:
		led = AR5K_PCICFG_LEDMODE_PROP | AR5K_PCICFG_LED_ASSOC;
		led_5210 = AR5K_PCICFG_LED_ASSOC;
		break;

	default:
		led = AR5K_PCICFG_LEDMODE_PROM | AR5K_PCICFG_LED_NONE;
		led_5210 = AR5K_PCICFG_LED_PEND;
		break;
	}

	
	if (ah->ah_version != AR5K_AR5210)
		AR5K_REG_ENABLE_BITS(ah, AR5K_PCICFG, led);
	else
		AR5K_REG_ENABLE_BITS(ah, AR5K_PCICFG, led_5210);
}


int ath5k_hw_set_gpio_input(struct ath5k_hw *ah, u32 gpio)
{
	ATH5K_TRACE(ah->ah_sc);
	if (gpio >= AR5K_NUM_GPIO)
		return -EINVAL;

	ath5k_hw_reg_write(ah,
		(ath5k_hw_reg_read(ah, AR5K_GPIOCR) & ~AR5K_GPIOCR_OUT(gpio))
		| AR5K_GPIOCR_IN(gpio), AR5K_GPIOCR);

	return 0;
}


int ath5k_hw_set_gpio_output(struct ath5k_hw *ah, u32 gpio)
{
	ATH5K_TRACE(ah->ah_sc);
	if (gpio >= AR5K_NUM_GPIO)
		return -EINVAL;

	ath5k_hw_reg_write(ah,
		(ath5k_hw_reg_read(ah, AR5K_GPIOCR) & ~AR5K_GPIOCR_OUT(gpio))
		| AR5K_GPIOCR_OUT(gpio), AR5K_GPIOCR);

	return 0;
}


u32 ath5k_hw_get_gpio(struct ath5k_hw *ah, u32 gpio)
{
	ATH5K_TRACE(ah->ah_sc);
	if (gpio >= AR5K_NUM_GPIO)
		return 0xffffffff;

	
	return ((ath5k_hw_reg_read(ah, AR5K_GPIODI) & AR5K_GPIODI_M) >> gpio) &
		0x1;
}


int ath5k_hw_set_gpio(struct ath5k_hw *ah, u32 gpio, u32 val)
{
	u32 data;
	ATH5K_TRACE(ah->ah_sc);

	if (gpio >= AR5K_NUM_GPIO)
		return -EINVAL;

	
	data = ath5k_hw_reg_read(ah, AR5K_GPIODO);

	data &= ~(1 << gpio);
	data |= (val & 1) << gpio;

	ath5k_hw_reg_write(ah, data, AR5K_GPIODO);

	return 0;
}


void ath5k_hw_set_gpio_intr(struct ath5k_hw *ah, unsigned int gpio,
		u32 interrupt_level)
{
	u32 data;

	ATH5K_TRACE(ah->ah_sc);
	if (gpio >= AR5K_NUM_GPIO)
		return;

	
	data = (ath5k_hw_reg_read(ah, AR5K_GPIOCR) &
		~(AR5K_GPIOCR_INT_SEL(gpio) | AR5K_GPIOCR_INT_SELH |
		AR5K_GPIOCR_INT_ENA | AR5K_GPIOCR_OUT(gpio))) |
		(AR5K_GPIOCR_INT_SEL(gpio) | AR5K_GPIOCR_INT_ENA);

	ath5k_hw_reg_write(ah, interrupt_level ? data :
		(data | AR5K_GPIOCR_INT_SELH), AR5K_GPIOCR);

	ah->ah_imr |= AR5K_IMR_GPIO;

	
	AR5K_REG_ENABLE_BITS(ah, AR5K_PIMR, AR5K_IMR_GPIO);
}

