

#ifndef __ASM_ARCH_MSM_RPC_HSUSB_H
#define __ASM_ARCH_MSM_RPC_HSUSB_H

#include <mach/msm_rpcrouter.h>
#include <mach/msm_otg.h>
#include <mach/msm_hsusb.h>

#if defined(CONFIG_MSM_ONCRPCROUTER) && !defined(CONFIG_ARCH_MSM8X60)
int msm_hsusb_rpc_connect(void);
int msm_hsusb_phy_reset(void);
int msm_hsusb_vbus_powerup(void);
int msm_hsusb_vbus_shutdown(void);
int msm_hsusb_send_productID(uint32_t product_id);
int msm_hsusb_send_serial_number(char *serial_number);
int msm_hsusb_is_serial_num_null(uint32_t val);
int msm_hsusb_reset_rework_installed(void);
int msm_hsusb_enable_pmic_ulpidata0(void);
int msm_hsusb_disable_pmic_ulpidata0(void);
int msm_hsusb_rpc_close(void);

int msm_chg_rpc_connect(void);
int msm_chg_usb_charger_connected(uint32_t type);
int msm_chg_usb_i_is_available(uint32_t sample);
int msm_chg_usb_i_is_not_available(void);
int msm_chg_usb_charger_disconnected(void);
int msm_chg_rpc_close(void);

#ifdef CONFIG_USB_GADGET_MSM_72K
int hsusb_chg_init(int connect);
void hsusb_chg_vbus_draw(unsigned mA);
void hsusb_chg_connected(enum chg_type chgtype);
#endif


int msm_fsusb_rpc_init(struct msm_otg_ops *ops);
int msm_fsusb_init_phy(void);
int msm_fsusb_reset_phy(void);
int msm_fsusb_suspend_phy(void);
int msm_fsusb_resume_phy(void);
int msm_fsusb_rpc_close(void);
int msm_fsusb_remote_dev_disconnected(void);
int msm_fsusb_set_remote_wakeup(void);
void msm_fsusb_rpc_deinit(void);
#if defined(CONFIG_MACH_MSM7X27_ALOHAV) || defined(CONFIG_MACH_MSM7X27_THUNDERC)


int msm_hsusb_get_charger_type(void);
#endif


#if defined(CONFIG_USB_SUPPORT_LGDRIVER_GSM) || \
	defined(CONFIG_USB_SUPPORT_LGE_GADGET_GSM)
int msm_hsusb_detect_chg_type(void);
#endif


#if defined(CONFIG_USB_SUPPORT_LGE_SERIAL_FROM_ARM9_IMEI)

typedef struct {
	
	u8 ue_imei[9];
} __attribute__((packed)) nv_ue_imei_type;

int msm_nv_imei_get(unsigned char* nv_imei_ptr);

typedef enum {
	NV_READ_F,          
	NV_WRITE_F,         
	NV_PEEK_F,          
	NV_POKE_F,          
	NV_FREE_F,          
	NV_CHKPNT_DIS_F,    
	NV_CHKPNT_ENA_F,    
	NV_OTASP_COMMIT_F,  
	NV_REPLACE_F,       
	NV_INCREMENT_F,     
	NV_FUNC_ENUM_PAD = 0x7FFF     
#ifdef FEATURE_RPC
	, NV_FUNC_ENUM_MAX = 0x7fffffff 
#endif
} nv_func_enum_type;

typedef enum {
	NV_ESN_I                          = 0,
	NV_UE_IMEI_I                      = 550,
#ifdef FEATURE_NV_RPC_SUPPORT
	NV_ITEMS_ENUM_MAX           = 0x7fffffff
#endif
} nv_items_enum_type;

typedef enum {
	NV_DONE_S,          
	NV_BUSY_S,          
	NV_BADCMD_S,        
	NV_FULL_S,          
	NV_FAIL_S,          
	NV_NOTACTIVE_S,     
	NV_BADPARM_S,       
	NV_READONLY_S,      
	NV_BADTG_S,         
	NV_NOMEM_S,         
	NV_NOTALLOC_S,      
	NV_STAT_ENUM_PAD = 0x7FFF     
#ifdef FEATURE_RPC
	,NV_STAT_ENUM_MAX = 0x7FFFFFFF     
#endif 
} nv_stat_enum_type;

#endif  

#else
static inline int msm_hsusb_rpc_connect(void) { return 0; }
static inline int msm_hsusb_phy_reset(void) { return 0; }
static inline int msm_hsusb_vbus_powerup(void) { return 0; }
static inline int msm_hsusb_vbus_shutdown(void) { return 0; }
static inline int msm_hsusb_send_productID(uint32_t product_id) { return 0; }
static inline int msm_hsusb_send_serial_number(char *serial_number)
{ return 0; }
static inline int msm_hsusb_is_serial_num_null(uint32_t val) { return 0; }
static inline int msm_hsusb_reset_rework_installed(void) { return 0; }
static inline int msm_hsusb_enable_pmic_ulpidata0(void) { return 0; }
static inline int msm_hsusb_disable_pmic_ulpidata0(void) { return 0; }
static inline int msm_hsusb_rpc_close(void) { return 0; }

static inline int msm_chg_rpc_connect(void) { return 0; }
static inline int msm_chg_usb_charger_connected(uint32_t type) { return 0; }
static inline int msm_chg_usb_i_is_available(uint32_t sample) { return 0; }
static inline int msm_chg_usb_i_is_not_available(void) { return 0; }
static inline int msm_chg_usb_charger_disconnected(void) { return 0; }
static inline int msm_chg_rpc_close(void) { return 0; }

#ifdef CONFIG_USB_GADGET_MSM_72K
static inline int hsusb_chg_init(int connect) { return 0; }
static inline void hsusb_chg_vbus_draw(unsigned mA) { }
static inline void hsusb_chg_connected(enum chg_type chgtype) { }
#endif

static inline int msm_fsusb_rpc_init(struct msm_otg_ops *ops) { return 0; }
static inline int msm_fsusb_init_phy(void) { return 0; }
static inline int msm_fsusb_reset_phy(void) { return 0; }
static inline int msm_fsusb_suspend_phy(void) { return 0; }
static inline int msm_fsusb_resume_phy(void) { return 0; }
static inline int msm_fsusb_rpc_close(void) { return 0; }
static inline int msm_fsusb_remote_dev_disconnected(void) { return 0; }
static inline int msm_fsusb_set_remote_wakeup(void) { return 0; }
static inline void msm_fsusb_rpc_deinit(void) { }
#endif 

#endif
