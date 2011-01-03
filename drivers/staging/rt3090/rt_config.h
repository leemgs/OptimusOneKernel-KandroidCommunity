
#ifndef	__RT_CONFIG_H__
#define	__RT_CONFIG_H__

#include "rtmp_type.h"
#include "rtmp_os.h"

#include "rtmp_def.h"
#include "rtmp_chip.h"
#include "rtmp_timer.h"

#include "oid.h"
#include "mlme.h"
#include "wpa.h"
#include "crypt_md5.h"
#include "crypt_sha2.h"
#include "crypt_hmac.h"
#include "rtmp.h"
#include "ap.h"
#include "dfs.h"
#include "chlist.h"
#include "spectrum.h"

#ifdef MLME_EX
#include	"mlme_ex_def.h"
#include	"mlme_ex.h"
#endif 

#include "eeprom.h"
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
#include "rtmp_mcu.h"
#endif



#undef AP_WSC_INCLUDED
#undef STA_WSC_INCLUDED
#undef WSC_INCLUDED



#ifdef CONFIG_STA_SUPPORT
#endif 

#ifdef BLOCK_NET_IF
#include "netif_block.h"
#endif 

#ifdef IGMP_SNOOP_SUPPORT
#include "igmp_snoop.h"
#endif 

#ifdef RALINK_ATE
#include "rt_ate.h"
#endif 

#ifdef RALINK_28xx_QA
#ifndef RALINK_ATE
#error "For supporting QA GUI, please set HAS_ATE=y and HAS_28xx_QA=y."
#endif 
#endif 




#if defined(AP_WSC_INCLUDED) || defined(STA_WSC_INCLUDED)
#define WSC_INCLUDED
#endif


#ifdef CONFIG_STA_SUPPORT
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
#ifndef WPA_SUPPLICANT_SUPPORT
#error "Build for being controlled by NetworkManager or wext, please set HAS_WPA_SUPPLICANT=y and HAS_NATIVE_WPA_SUPPLICANT_SUPPORT=y"
#endif 
#endif 

#endif 



#ifdef IKANOS_VX_1X0
#include "vr_ikans.h"
#endif 



#endif	
