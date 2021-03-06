

#include <typedefs.h>
#include <bcmutils.h>
#include <sdio.h>	
#include <bcmsdbus.h>	
#include <sdiovar.h>	

#include <linux/sched.h>	

#include <linux/mmc/core.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#if defined(CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>
#endif 

#if !defined(SDIO_VENDOR_ID_BROADCOM)
#define SDIO_VENDOR_ID_BROADCOM		0x02d0
#endif 

#define SDIO_DEVICE_ID_BROADCOM_DEFAULT	0x0000

#if !defined(SDIO_DEVICE_ID_BROADCOM_4325_SDGWB)
#define SDIO_DEVICE_ID_BROADCOM_4325_SDGWB	0x0492	
#endif 
#if !defined(SDIO_DEVICE_ID_BROADCOM_4325)
#define SDIO_DEVICE_ID_BROADCOM_4325		0x0493	
#endif 
#if !defined(SDIO_DEVICE_ID_BROADCOM_4329)
#define SDIO_DEVICE_ID_BROADCOM_4329		0x4329
#endif 

#include <bcmsdh_sdmmc.h>
#include <dhd_dbg.h>

#if defined(CONFIG_BRCM_GPIO_INTR) && defined(CONFIG_HAS_EARLYSUSPEND)
#include <mach/gpio.h>
#include <linux/irq.h>
#endif	


#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
#include <linux/mmc/host.h>
#include <linux/wakelock.h>

#define GPIO_WLAN_HOST_WAKE CONFIG_BCM4325_GPIO_WL_HOSTWAKEUP

struct dhd_wifisleep_info {
	unsigned host_wake;
	unsigned host_wake_irq;
};

static struct dhd_wifisleep_info *dhd_wifi_sleep;

struct wake_lock wlan_host_wakelock; 
struct wake_lock wlan_host_wakelock_resume;
int dhd_suspend_context = FALSE;

extern int del_wl_timers(void);

extern void register_mmc_card_pm(struct early_suspend *);
extern void unregister_mmc_card_pm(void);

#endif 


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && defined(CONFIG_PM_SLEEP)
extern volatile bool dhd_mmc_suspend;
#endif

extern void sdioh_sdmmc_devintr_off(sdioh_info_t *sd);
extern void sdioh_sdmmc_devintr_on(sdioh_info_t *sd);

int sdio_function_init(void);
void sdio_function_cleanup(void);

#define DESCRIPTION "bcmsdh_sdmmc Driver"
#define AUTHOR "Broadcom Corporation"


static int clockoverride = 0;

module_param(clockoverride, int, 0644);
MODULE_PARM_DESC(clockoverride, "SDIO card clock override");

PBCMSDH_SDMMC_INSTANCE gInstance;


#define BCMSDH_SDMMC_MAX_DEVICES 1

extern int bcmsdh_probe(struct device *dev);
extern int bcmsdh_remove(struct device *dev);
struct device sdmmc_dev;

#if defined(CONFIG_HAS_EARLYSUSPEND)
extern int dhdsdio_bussleep(void *bus, bool sleep);
extern bool dhdsdio_dpc(void *bus);
extern int dhd_os_proto_block(void *pub);
extern int dhd_os_proto_unblock(void * pub);
extern void *dhd_es_get_dhd_pub(void);
extern void *dhd_es_get_dhd_bus_sdh(void);
static int dhd_register_early_suspend(void);
static void dhd_unregister_early_suspend(void);

#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)

void dhd_early_suspend(struct early_suspend *h);
void dhd_late_resume(struct early_suspend *h);
static struct early_suspend early_suspend_data = {
	.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 20,
	.suspend = dhd_early_suspend,
	.resume = dhd_late_resume
};
#endif 

DECLARE_WAIT_QUEUE_HEAD(bussleep_wake);
typedef struct dhd_early_suspend {

#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)	
	int wait_driver_load; 
	bool skip;
#endif 
	bool state;
	bool drv_loaded;
	struct dhd_bus_t *bus;
} dhd_early_suspend_t;

#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
dhd_early_suspend_t dhd_early_suspend_ctrl = { 0, 0, 0, 0, 0};
#else 
dhd_early_suspend_t dhd_early_suspend_ctrl = { 0, 0, 0};
#endif 
#endif 

static int bcmsdh_sdmmc_probe(struct sdio_func *func,
                              const struct sdio_device_id *id)
{
	int ret = 0;
	static struct sdio_func sdio_func_0;
	sd_trace(("bcmsdh_sdmmc: %s Enter\n", __FUNCTION__));
	sd_trace(("sdio_bcmsdh: func->class=%x\n", func->class));
	sd_trace(("sdio_vendor: 0x%04x\n", func->vendor));
	sd_trace(("sdio_device: 0x%04x\n", func->device));
	sd_trace(("Function#: 0x%04x\n", func->num));

	if (func->num == 1) {
		sdio_func_0.num = 0;
		sdio_func_0.card = func->card;
		gInstance->func[0] = &sdio_func_0;
		if(func->device == 0x4) { 
			gInstance->func[2] = NULL;
			sd_trace(("NIC found, calling bcmsdh_probe...\n"));
			ret = bcmsdh_probe(&sdmmc_dev);
		}
	}

	gInstance->func[func->num] = func;

	if (func->num == 2) {
		sd_trace(("F2 found, calling bcmsdh_probe...\n"));
		ret = bcmsdh_probe(&sdmmc_dev);
	}

	return ret;
}

static void bcmsdh_sdmmc_remove(struct sdio_func *func)
{
	sd_trace(("bcmsdh_sdmmc: %s Enter\n", __FUNCTION__));
	sd_info(("sdio_bcmsdh: func->class=%x\n", func->class));
	sd_info(("sdio_vendor: 0x%04x\n", func->vendor));
	sd_info(("sdio_device: 0x%04x\n", func->device));
	sd_info(("Function#: 0x%04x\n", func->num));

	if (func->num == 2) {
		sd_trace(("F2 found, calling bcmsdh_probe...\n"));
		bcmsdh_remove(&sdmmc_dev);
	}
}



static const struct sdio_device_id bcmsdh_sdmmc_ids[] = {
	{ SDIO_DEVICE(SDIO_VENDOR_ID_BROADCOM, SDIO_DEVICE_ID_BROADCOM_DEFAULT) },
	{ SDIO_DEVICE(SDIO_VENDOR_ID_BROADCOM, SDIO_DEVICE_ID_BROADCOM_4325_SDGWB) },
	{ SDIO_DEVICE(SDIO_VENDOR_ID_BROADCOM, SDIO_DEVICE_ID_BROADCOM_4325) },
	{ SDIO_DEVICE(SDIO_VENDOR_ID_BROADCOM, SDIO_DEVICE_ID_BROADCOM_4329) },
	{ 				},
};

MODULE_DEVICE_TABLE(sdio, bcmsdh_sdmmc_ids);

static struct sdio_driver bcmsdh_sdmmc_driver = {
	.probe		= bcmsdh_sdmmc_probe,
	.remove		= bcmsdh_sdmmc_remove,
	.name		= "bcmsdh_sdmmc",
	.id_table	= bcmsdh_sdmmc_ids,
	};

struct sdos_info {
	sdioh_info_t *sd;
	spinlock_t lock;
};


int
sdioh_sdmmc_osinit(sdioh_info_t *sd)
{
	struct sdos_info *sdos;

	sdos = (struct sdos_info*)MALLOC(sd->osh, sizeof(struct sdos_info));
	sd->sdos_info = (void*)sdos;
	if (sdos == NULL)
		return BCME_NOMEM;

	sdos->sd = sd;
	spin_lock_init(&sdos->lock);
	return BCME_OK;
}

void
sdioh_sdmmc_osfree(sdioh_info_t *sd)
{
	struct sdos_info *sdos;
	ASSERT(sd && sd->sdos_info);

	sdos = (struct sdos_info *)sd->sdos_info;
	MFREE(sd->osh, sdos, sizeof(struct sdos_info));
}

#if defined(CONFIG_HAS_EARLYSUSPEND)
void
dhd_es_set_dhd_bus(void *bus)
{
	dhd_early_suspend_ctrl.bus = bus;
}

void *
dhd_es_get_dhd_bus(void)
{
	return dhd_early_suspend_ctrl.bus;
}


#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
static int
dhd_es_lock_dhd_bus(void)
{



	void *bus;
	bus = dhd_es_get_dhd_pub();
	if( bus )
		dhd_os_proto_block(bus);


	return 0;
}

static int
dhd_es_unlock_dhd_bus(void)
{



	void *bus;
	bus = dhd_es_get_dhd_pub();
	if( bus )
		dhd_os_proto_unblock(bus);


	return 0;
}
#endif 


bool
dhd_early_suspend_state(void)
{
	return dhd_early_suspend_ctrl.state;
}


#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
static void dhd_enable_sdio_irq(int enable)
{


	struct mmc_card *card;
	struct mmc_host *host;

	if(gInstance->func[0] == NULL){
		printk("%s: gInstance->func[0] is NULL\n",__func__);	
		return;
	}
	
	card = gInstance->func[0]->card;
	host = card->host;


	if (enable == TRUE )
		host->ops->enable_sdio_irq(host, 1); 
	else if (enable == FALSE)
		host->ops->enable_sdio_irq(host, 0); 
}

static int dhd_suspend(void)
{
	int bus_state;
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	int max_tries = 3;
	int gpio = 0;	
#endif  	


	DHD_TRACE(("%s: SUSPEND Enter\n", __FUNCTION__));
	if (NULL != dhd_early_suspend_ctrl.bus) {
		dhd_es_lock_dhd_bus();
#if !defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)		
		dhd_early_suspend_ctrl.state = TRUE;
#endif 
		do {
			bus_state = dhdsdio_bussleep(dhd_early_suspend_ctrl.bus, TRUE);
			if (bus_state == BCME_BUSY)
			{
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
				
				wait_event_timeout(bussleep_wake, FALSE, HZ/4);
				DHD_TRACE(("%s in loop\n", __FUNCTION__));
#else 
				
				wait_event_timeout(bussleep_wake, FALSE, HZ/20);
				DHD_TRACE(("%s in loop\n", __FUNCTION__));
#endif 
			}
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)			
		} while ((bus_state == BCME_BUSY) && (max_tries-- > 0));
#else 		
		} while (bus_state == BCME_BUSY);
#endif 

#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)	
		if(max_tries <= 0)
		{
			printk(KERN_ERR "[WiFi] BUS BUSY!! Couldn't sleep.\n"); 	
			dhd_es_unlock_dhd_bus();
			
			return -1;
		}

		dhd_early_suspend_ctrl.state = TRUE;
		gpio = gpio_get_value(GPIO_WLAN_HOST_WAKE);
		DHD_TRACE(("%s: SUSPEND Done gpio->%d\n", __FUNCTION__, gpio));
#else 
		DHD_TRACE(("%s: SUSPEND Done\n", __FUNCTION__));
#endif 
	} else {
		DHD_ERROR(("%s: no bus.. \n", __FUNCTION__));
	}
	return 0;
}
static int dhd_resume(void)
{
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	int gpio = 0;




	dhd_enable_sdio_irq(FALSE);
	dhd_suspend_context = FALSE;
#endif 


	DHD_TRACE(("%s: RESUME Enter\n", __FUNCTION__));
	if (NULL != dhd_early_suspend_ctrl.bus) {
		dhd_early_suspend_ctrl.state = FALSE;
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)



		wake_lock(&wlan_host_wakelock_resume);

#endif 
		dhdsdio_dpc(dhd_early_suspend_ctrl.bus);
		dhd_es_unlock_dhd_bus();
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)		
		gpio = gpio_get_value(GPIO_WLAN_HOST_WAKE);
		DHD_TRACE(("%s: RESUME Done gpio->%d\n", __FUNCTION__, gpio));

		
		wake_unlock(&wlan_host_wakelock_resume);

#else 
		DHD_TRACE(("%s: RESUME Done\n", __FUNCTION__));
#endif 
	} else {
		DHD_ERROR(("%s: no bus.. \n", __FUNCTION__));
	}
	return 0;
}
#endif 

#endif 


SDIOH_API_RC
sdioh_interrupt_set(sdioh_info_t *sd, bool enable)
{
	ulong flags;
	struct sdos_info *sdos;

	sd_trace(("%s: %s\n", __FUNCTION__, enable ? "Enabling" : "Disabling"));

	sdos = (struct sdos_info *)sd->sdos_info;
	ASSERT(sdos);

#if !defined(OOB_INTR_ONLY)
	if (enable && !(sd->intr_handler && sd->intr_handler_arg)) {
		sd_err(("%s: no handler registered, will not enable\n", __FUNCTION__));
		return SDIOH_API_RC_FAIL;
	}
#endif 

	
	spin_lock_irqsave(&sdos->lock, flags);

	sd->client_intr_enabled = enable;
	if (enable) {
		sdioh_sdmmc_devintr_on(sd);
	} else {
		sdioh_sdmmc_devintr_off(sd);
	}

	spin_unlock_irqrestore(&sdos->lock, flags);

	return SDIOH_API_RC_SUCCESS;
}


#ifdef BCMSDH_MODULE
static int __init
bcmsdh_module_init(void)
{
	int error = 0;
	sdio_function_init();
	return error;
}

static void __exit
bcmsdh_module_cleanup(void)
{
	sdio_function_cleanup();
}

module_init(bcmsdh_module_init);
module_exit(bcmsdh_module_cleanup);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_AUTHOR(AUTHOR);

#endif 

int sdio_function_init(void)
{
	int error = 0;
	sd_trace(("bcmsdh_sdmmc: %s Enter\n", __FUNCTION__));

	gInstance = kzalloc(sizeof(BCMSDH_SDMMC_INSTANCE), GFP_KERNEL);
	if (!gInstance)
		return -ENOMEM;

	bzero(&sdmmc_dev, sizeof(sdmmc_dev));
	error = sdio_register_driver(&bcmsdh_sdmmc_driver);

#if defined(CONFIG_HAS_EARLYSUSPEND)
	if (!error) {
		dhd_register_early_suspend();
		DHD_TRACE(("%s: registered with Android PM\n", __FUNCTION__));
	}
#endif	

	return error;
}


extern int bcmsdh_remove(struct device *dev);
void sdio_function_cleanup(void)
{
	sd_trace(("%s Enter\n", __FUNCTION__));

#if defined(CONFIG_HAS_EARLYSUSPEND)


#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	dhd_enable_sdio_irq(FALSE);
#endif

	dhd_unregister_early_suspend();


#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	dhd_suspend_context = TRUE; 
#endif

#endif	

	sdio_unregister_driver(&bcmsdh_sdmmc_driver);

	if (gInstance)
		kfree(gInstance);
}

#if defined(CONFIG_BRCM_LGE_WL_ARPOFFLOAD)
extern int dhd_config_arp_offload(void *bus, bool flag);
#endif	


#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP) && defined(CONFIG_BRCM_LGE_WL_ARPOFFLOAD)
extern int dhdsdio_set_dtim(void *bus, int enalbe);
#endif


#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP) && defined(CONFIG_BRCM_LGE_WL_PKTFILTER)
extern int dhdsdio_enable_filters(void *bus);
extern int dhdsdio_disable_filters(void *bus);
#endif	


#if defined(CONFIG_HAS_EARLYSUSPEND)

#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
void dhd_early_suspend(struct early_suspend *h)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && defined(CONFIG_PM_SLEEP)
	dhd_mmc_suspend = FALSE;
#endif


	DHD_TRACE(("%s: enter\n", __FUNCTION__));

	dhd_suspend_context = TRUE;
	
	
	del_wl_timers();

#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP) && defined(CONFIG_BRCM_LGE_WL_ARPOFFLOAD)
	dhdsdio_set_dtim(dhd_early_suspend_ctrl.bus, TRUE);
#endif


#if defined(CONFIG_BRCM_LGE_WL_ARPOFFLOAD)
	
	dhd_config_arp_offload(dhd_early_suspend_ctrl.bus , TRUE);
#endif	

	

#if defined(CONFIG_BRCM_LGE_WL_PKTFILTER)
	dhdsdio_enable_filters(dhd_early_suspend_ctrl.bus);
#endif	

	
	if(dhd_suspend() < 0) {
		dhd_enable_sdio_irq(TRUE); 
		DHD_ERROR(("%s: dhd_suspend() failed\n", __FUNCTION__));
		return;
	} 
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && defined(CONFIG_PM_SLEEP)	
	dhd_mmc_suspend = TRUE;
#endif
#if defined(CONFIG_BRCM_GPIO_INTR)
	enable_irq(dhd_wifi_sleep->host_wake_irq);
#endif 

}

void dhd_late_resume(struct early_suspend *h)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) && defined(CONFIG_PM_SLEEP)
	dhd_mmc_suspend = FALSE;
#endif


	DHD_TRACE(("%s: enter\n", __FUNCTION__));

	if(dhd_suspend_context == TRUE ){
#if defined(CONFIG_BRCM_GPIO_INTR)
	disable_irq(dhd_wifi_sleep->host_wake_irq);
#endif 
	
	dhd_resume();
	}else 
		printk("%s: Do not dhd_suspend mode setting.\n",__FUNCTION__);


#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP) && defined(CONFIG_BRCM_LGE_WL_ARPOFFLOAD)
	dhdsdio_set_dtim(dhd_early_suspend_ctrl.bus, FALSE);
#endif


#if defined(CONFIG_BRCM_LGE_WL_ARPOFFLOAD)
	
	dhd_config_arp_offload(dhd_early_suspend_ctrl.bus, FALSE);
#endif	



#if defined (CONFIG_BRCM_LGE_WL_PKTFILTER)
	dhdsdio_disable_filters(dhd_early_suspend_ctrl.bus);
#endif 



	return;
}
EXPORT_SYMBOL(dhd_early_suspend);
EXPORT_SYMBOL(dhd_late_resume);
#endif 


#if defined(CONFIG_BRCM_GPIO_INTR)


#if !defined(CONFIG_LGE_BCM432X_PATCH)
#define GPIO_WLAN_HOST_WAKE 0

struct dhd_wifisleep_info {
	unsigned host_wake;
	unsigned host_wake_irq;
};

static struct dhd_wifisleep_info *dhd_wifi_sleep;
#endif 



static int
dhd_enable_hwakeup(void)
{
	int ret;

	ret = set_irq_wake(dhd_wifi_sleep->host_wake_irq, 1);

	if (ret < 0) {
		DHD_ERROR(("Couldn't enable WLAN_HOST_WAKE as wakeup interrupt"));
		free_irq(dhd_wifi_sleep->host_wake_irq, NULL);
	}
	else 
		printk("[yoohoo] dhd_enable_hwakeup : succeed irq %d\n", dhd_wifi_sleep->host_wake_irq);

	return ret;
}


static void
dhd_disable_hwakeup(void)
{

	if (set_irq_wake(dhd_wifi_sleep->host_wake_irq, 0))
		DHD_ERROR(("Couldn't disable hostwake IRQ wakeup mode\n"));
}



static irqreturn_t
dhd_hostwakeup_isr(int irq, void *dev_id)
{
	int gpio = 0;

	gpio = gpio_get_value(GPIO_WLAN_HOST_WAKE);
	printk(KERN_ERR "[%s] HostWakeup Get GPIO %d: %d\n", 
		__func__, GPIO_WLAN_HOST_WAKE, gpio);
	gpio_set_value(GPIO_WLAN_HOST_WAKE, 0);

#if !defined(CONFIG_LGE_BCM432X_PATCH)
	set_irq_type(dhd_wifi_sleep->host_wake_irq, gpio ? IRQF_TRIGGER_LOW : IRQF_TRIGGER_HIGH);
#endif 

	if (!gpio) {
		DHD_INFO(("[WiFi] complete on host-wakeup \n"));

		
		return IRQ_HANDLED;
	}

	
	return IRQ_HANDLED;
}


static int
dhd_register_hwakeup(void)
{
	int ret;

	dhd_wifi_sleep = kzalloc(sizeof(struct dhd_wifisleep_info), GFP_KERNEL);
	if (!dhd_wifi_sleep)
		return -ENOMEM;

	dhd_wifi_sleep->host_wake = GPIO_WLAN_HOST_WAKE;

	printk(KERN_ERR "[yoohoo] dhd_register_hwakeup : start \n");


#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	
   	wake_lock_init(&wlan_host_wakelock, WAKE_LOCK_SUSPEND, "WLAN_HOST_WAKE");
   	wake_lock_init(&wlan_host_wakelock_resume, WAKE_LOCK_SUSPEND, "WLAN_HOST_WAKE_RESUME");	
#endif 


	ret = gpio_request(dhd_wifi_sleep->host_wake, "wifi_hostwakeup");
	if (ret < 0) {
		DHD_ERROR(("[WiFi] Failed to get gpio_request \n"));
		gpio_free(dhd_wifi_sleep->host_wake);
		return 0;
	}


#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	
	ret = gpio_direction_output(dhd_wifi_sleep->host_wake, 0);
	if (ret < 0) {
		printk(KERN_ERR "[WiFi] Failed to get direction out\n");
	}
	
#endif 


	ret = gpio_direction_input(dhd_wifi_sleep->host_wake);
	if (ret < 0) {
		DHD_ERROR(("[WiFi] Failed to get direction  \n"));
		return 0;
	}

	dhd_wifi_sleep->host_wake_irq = gpio_to_irq(dhd_wifi_sleep->host_wake);

	if (dhd_wifi_sleep->host_wake_irq  < 0) {
		DHD_ERROR(("[WiFi] Failed to get irq  \n"));
		return 0;
	}


#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	ret = request_irq(dhd_wifi_sleep->host_wake_irq, dhd_hostwakeup_isr,

		IRQF_DISABLED | IRQF_TRIGGER_RISING , "wifi_hostwakeup", NULL); 
#else 
	ret = request_irq(dhd_wifi_sleep->host_wake_irq, dhd_hostwakeup_isr,
		IRQF_DISABLED | IRQF_TRIGGER_HIGH, "wifi_hostwakeup", NULL);
#endif 

	if (ret) {
		DHD_ERROR(("[WiFi] Failed to get HostWakeUp IRQ \n"));
		free_irq(dhd_wifi_sleep->host_wake_irq, 0);
		return ret;
		
	}
	else {
		DHD_INFO(("[WiFi] install HostWakeup IRQ \n"));
		printk (KERN_ERR "[yoohoo] dhd_register_hwakeup : OK\n");
	}


#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)

	set_irq_type(dhd_wifi_sleep->host_wake_irq, IRQ_TYPE_EDGE_RISING);
#if	defined(CONFIG_BRCM_GPIO_INTR)
	disable_irq(dhd_wifi_sleep->host_wake_irq);
#endif 
#endif 

	return ret;
}

static void
dhd_unregister_hwakeup(void)
{

	dhd_disable_hwakeup();
	free_irq(dhd_wifi_sleep->host_wake_irq, NULL);
	gpio_free(dhd_wifi_sleep->host_wake);
	kfree(dhd_wifi_sleep);
}
#endif 

static int
dhd_register_early_suspend(void)
{
	
#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	dhd_early_suspend_ctrl.drv_loaded = TRUE;

	dhd_early_suspend_ctrl.wait_driver_load = jiffies;
	register_mmc_card_pm(&early_suspend_data);

#if defined(CONFIG_BRCM_GPIO_INTR)
	
	dhd_register_hwakeup();
	dhd_enable_hwakeup();
	printk(KERN_ERR "[%s] HostWakeup Get GPIO %d: %d\n",
			__func__, GPIO_WLAN_HOST_WAKE, gpio_get_value(GPIO_WLAN_HOST_WAKE));
#endif	

	return 0;
#else	
	return 0;
#endif	

}

static void
dhd_unregister_early_suspend(void)
{

#if defined(CONFIG_BRCM_LGE_WL_HOSTWAKEUP)
	if (dhd_early_suspend_ctrl.drv_loaded == FALSE)
		return;
	
#if defined(CONFIG_BRCM_GPIO_INTR)
	
	dhd_unregister_hwakeup();
#endif	

	unregister_mmc_card_pm();

	
	wake_lock_destroy(&wlan_host_wakelock);
	wake_lock_destroy(&wlan_host_wakelock_resume);
#else	
	return;
#endif 

}
#endif	
