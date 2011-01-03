
#ifndef __PMIC8058_PWM_H__
#define __PMIC8058_PWM_H__


#define	PM_PWM_PERIOD_MAX		(274 * USEC_PER_SEC)

#define	PM_PWM_PERIOD_MIN		7 

struct pm8058_pwm_pdata {
	int 	(*config)(struct pwm_device *pwm, int ch, int on);
	int 	(*enable)(struct pwm_device *pwm, int ch, int on);
};

#define	PM_PWM_LUT_SIZE			64
#define	PM_PWM_LUT_DUTY_TIME_MAX	512	
#define	PM_PWM_LUT_PAUSE_MAX		(7000 * PM_PWM_LUT_DUTY_TIME_MAX)


#define	PM_PWM_LUT_LOOP		0x01
#define	PM_PWM_LUT_RAMP_UP	0x02
#define	PM_PWM_LUT_REVERSE	0x04
#define	PM_PWM_LUT_PAUSE_HI_EN	0x10
#define	PM_PWM_LUT_PAUSE_LO_EN	0x20

#define	PM_PWM_LUT_NO_TABLE	0x100


#define	PM_PWM_LED_0		0
#define	PM_PWM_LED_1		1
#define	PM_PWM_LED_2		2
#define	PM_PWM_LED_KPD		3
#define	PM_PWM_LED_FLASH	4


#define	PM_PWM_CONF_NONE	0x0
#define	PM_PWM_CONF_PWM1	0x1
#define	PM_PWM_CONF_PWM2	0x2
#define	PM_PWM_CONF_PWM3	0x3
#define	PM_PWM_CONF_DTEST1	0x4
#define	PM_PWM_CONF_DTEST2	0x5
#define	PM_PWM_CONF_DTEST3	0x6
#define	PM_PWM_CONF_DTEST4	0x7


int pm8058_pwm_lut_config(struct pwm_device *pwm, int period_us,
			  int duty_pct[], int duty_time_ms, int start_idx,
			  int len, int pause_lo, int pause_hi, int flags);


int pm8058_pwm_lut_enable(struct pwm_device *pwm, int start);

int pm8058_pwm_set_dtest(struct pwm_device *pwm, int enable);

int pm8058_pwm_config_led(struct pwm_device *pwm, int id,
			  int mode, int max_current);

#if !defined(CONFIG_PMIC8058_PWM)
inline struct pwm_device *pwm_request(int pwm_id, const char *label)
{
	return NULL;
}

inline void pwm_free(struct pwm_device *pwm)
{
}

inline int pwm_config(struct pwm_device *pwm, int duty_ns, int period_ns)
{
	return 0;
}

inline int pwm_enable(struct pwm_device *pwm)
{
	return 0;
}

inline void pwm_disable(struct pwm_device *pwm)
{
}

inline int pwm_set_dtest(struct pwm_device *pwm, int enable)
{
	return 0;
}
#endif

#endif 
