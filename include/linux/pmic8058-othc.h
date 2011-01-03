

#ifndef __PMIC8058_OTHC_H__
#define __PMIC8058_OTHC_H__

enum othc_headset_type {
	OTHC_HEADSET_NO,
	OTHC_HEADSET_NC,
};


enum othc_micbias_enable {
	
	OTHC_SIGNAL_OFF,
	
	OTHC_SIGNAL_TCXO,
	
	OTHC_SIGNAL_PWM_TCXO,
	
	OTHC_SIGNAL_ALWAYS_ON,
};


enum othc_micbias {
	OTHC_MICBIAS_0,
	OTHC_MICBIAS_1,
	OTHC_MICBIAS_2,
	OTHC_MICBIAS_MAX,
};

enum othc_micbias_capability {
	
	OTHC_MICBIAS,
	
	OTHC_MICBIAS_HSED,
};


struct othc_hsed_config {
	enum othc_headset_type othc_headset;
	u16 othc_lowcurr_thresh_uA;
	u16 othc_highcurr_thresh_uA;
	u32 othc_hyst_prediv_us;
	u32 othc_period_clkdiv_us;
	u32 othc_hyst_clk_us;
	u32 othc_period_clk_us;
	int othc_nc_gpio;
	int othc_wakeup;
	int (*othc_nc_gpio_setup)(void);
};

struct pmic8058_othc_config_pdata {
	enum othc_micbias micbias_select;
	enum othc_micbias_enable micbias_enable;
	enum othc_micbias_capability micbias_capability;
	struct othc_hsed_config *hsed_config;
};

int pm8058_micbias_enable(enum othc_micbias micbias,
			enum othc_micbias_enable enable);

#endif 
