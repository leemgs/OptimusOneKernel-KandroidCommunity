

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#include <bcmutils.h>

#ifndef BCMDONGLEHOST
#include <wlc_cfg.h>
#else
#include <dngl_stats.h>
#include <dhd.h>
#endif

#include <wlioctl.h>
#include <wl_iw.h>

#if defined(CONFIG_LGE_BCM432X_PATCH)
#include <asm/gpio.h>
#include <linux/interrupt.h>
#endif 


#ifndef BCMDONGLEHOST
#include <wlc_pub.h>
#include <wl_dbg.h>
#else
#define WL_ERROR(x) printf x
#define WL_TRACE(x)
#endif

#ifdef CUSTOMER_HW
extern  void bcm_wlan_power_off(int);
extern  void bcm_wlan_power_on(int);
#endif 

#if defined(OOB_INTR_ONLY)

#ifdef CUSTOMER_HW3
#include <mach/gpio.h>
#endif


static int dhd_oob_gpio_num = -1; 

module_param(dhd_oob_gpio_num, int, 0644);
MODULE_PARM_DESC(dhd_oob_gpio_num, "DHD oob gpio number");

int dhd_customer_oob_irq_map(void)
{
int  host_oob_irq = 0;
#if defined(CUSTOM_OOB_GPIO_NUM)
	if (dhd_oob_gpio_num < 0) {
		dhd_oob_gpio_num = CUSTOM_OOB_GPIO_NUM;
	}
#endif

	if (dhd_oob_gpio_num < 0) {
		WL_ERROR(("%s: ERROR customer specific Host GPIO is NOT defined \n",
		__FUNCTION__));
		return (dhd_oob_gpio_num);
	}

	WL_ERROR(("%s: customer specific Host GPIO number is (%d)\n",
	         __FUNCTION__, dhd_oob_gpio_num));

#if defined CUSTOMER_HW
	host_oob_irq = MSM_GPIO_TO_INT(dhd_oob_gpio_num);
#elif defined CUSTOMER_HW3
	gpio_request(dhd_oob_gpio_num, "oob irq");
	host_oob_irq = gpio_to_irq(dhd_oob_gpio_num);
	gpio_direction_input(dhd_oob_gpio_num);
#endif

	return (host_oob_irq);
}
#endif 


#if defined(CONFIG_LGE_BCM432X_PATCH)

void
dhd_customer_gpio_wlan_ctrl(int onoff, int irq_detect_ctrl)
#else 

void
dhd_customer_gpio_wlan_ctrl(int onoff)
#endif 

{
	switch (onoff) {
		case WLAN_RESET_OFF:
			WL_TRACE(("%s: call customer specific GPIO to insert WLAN RESET\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			bcm_wlan_power_off(2);
#endif 
			WL_ERROR(("=========== WLAN placed in RESET ========\n"));

#if defined(CONFIG_LGE_BCM432X_PATCH)
			if (gpio_get_value(CONFIG_BCM4325_GPIO_WL_RESET)) {
				if(irq_detect_ctrl)
					disable_irq(gpio_to_irq(CONFIG_BCM4325_GPIO_WL_RESET));
				gpio_set_value(CONFIG_BCM4325_GPIO_WL_RESET, 0);
			}
#endif 

		break;

		case WLAN_RESET_ON:
			WL_TRACE(("%s: callc customer specific GPIO to remove WLAN RESET\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			bcm_wlan_power_on(2);
#endif 
			WL_ERROR(("=========== WLAN going back to live  ========\n"));

#if defined(CONFIG_LGE_BCM432X_PATCH)
			if (!gpio_get_value(CONFIG_BCM4325_GPIO_WL_RESET)) { 
				gpio_set_value(CONFIG_BCM4325_GPIO_WL_RESET, 1);
				if(irq_detect_ctrl)
					enable_irq(gpio_to_irq(CONFIG_BCM4325_GPIO_WL_RESET));
			}
#endif 
		
		break;

		case WLAN_POWER_OFF:
			WL_TRACE(("%s: call customer specific GPIO to turn off WL_REG_ON\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			bcm_wlan_power_off(1);
#endif 

#if defined(CONFIG_LGE_BCM432X_PATCH)
#ifdef CONFIG_BCM4325_GPIO_WL_REGON
			if (!gpio_get_value(CONFIG_BCM4325_GPIO_BT_RESET)) {
				gpio_set_value(CONFIG_BCM4325_GPIO_WL_REGON, 0);
			}
#endif 

#endif 

		break;

		case WLAN_POWER_ON:
			WL_TRACE(("%s: call customer specific GPIO to turn on WL_REG_ON\n",
				__FUNCTION__));
#ifdef CUSTOMER_HW
			bcm_wlan_power_on(1);
#endif 

#if defined(CONFIG_LGE_BCM432X_PATCH)

#ifdef CONFIG_BCM4325_GPIO_WL_REGON
			if (!gpio_get_value(CONFIG_BCM4325_GPIO_WL_REGON)) { 
				gpio_set_value(CONFIG_BCM4325_GPIO_WL_REGON, 1);
				mdelay(150);
			}
#endif 

#else 
			
			OSL_DELAY(500);
#endif 

		break;
	}
}
