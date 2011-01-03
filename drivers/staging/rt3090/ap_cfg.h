
#ifndef __AP_CFG_H__
#define __AP_CFG_H__


#include "rt_config.h"

INT RTMPAPPrivIoctlSet(
	IN RTMP_ADAPTER *pAd,
	IN struct iwreq *pIoctlCmdStr);

INT RTMPAPPrivIoctlShow(
	IN RTMP_ADAPTER *pAd,
	IN struct iwreq *pIoctlCmdStr);

INT RTMPAPSetInformation(
	IN	PRTMP_ADAPTER	pAd,
	IN	OUT	struct iwreq	*rq,
	IN	INT				cmd);

INT RTMPAPQueryInformation(
	IN	PRTMP_ADAPTER       pAd,
	IN	OUT	struct iwreq    *rq,
	IN	INT                 cmd);

VOID RTMPIoctlStatistics(
	IN PRTMP_ADAPTER pAd,
	IN struct iwreq *wrq);

VOID RTMPIoctlGetMacTable(
	IN PRTMP_ADAPTER pAd,
	IN struct iwreq *wrq);

#ifdef DBG
VOID RTMPAPIoctlBBP(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  struct iwreq    *wrq);

VOID RTMPAPIoctlMAC(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  struct iwreq    *wrq);

VOID RTMPAPIoctlE2PROM(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  struct iwreq    *wrq);

#ifdef RTMP_RF_RW_SUPPORT
VOID RTMPAPIoctlRF(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq);
#endif 

#endif 

VOID RT28XX_IOCTL_MaxRateGet(
	IN	RTMP_ADAPTER			*pAd,
	IN	PHTTRANSMIT_SETTING	pHtPhyMode,
	OUT	UINT32					*pRate);


#ifdef DOT11_N_SUPPORT
VOID RTMPIoctlQueryBaTable(
	IN	PRTMP_ADAPTER	pAd,
	IN	struct iwreq	*wrq);
#endif 

VOID RTMPIoctlStaticWepCopy(
	IN	PRTMP_ADAPTER	pAd,
	IN	struct iwreq	*wrq);

VOID RTMPIoctlRadiusData(
	IN PRTMP_ADAPTER	pAd,
	IN struct iwreq		*wrq);

VOID RTMPIoctlAddWPAKey(
	IN	PRTMP_ADAPTER	pAd,
	IN	struct iwreq	*wrq);

VOID RTMPIoctlAddPMKIDCache(
	IN	PRTMP_ADAPTER	pAd,
	IN	struct iwreq	*wrq);

#endif 
