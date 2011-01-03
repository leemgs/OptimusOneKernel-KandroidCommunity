

#include "ap_autoChSel_cmm.h"

#ifndef __AUTOCHSELECT_H__
#define __AUTOCHSELECT_H__

#ifdef AUTO_CH_SELECT_ENHANCE
#define AP_AUTO_CH_SEL(__P, __O)	New_APAutoSelectChannel((__P), (__O))
#else
#define AP_AUTO_CH_SEL(__P, __O)	APAutoSelectChannel((__P), (__O))
#endif


ULONG AutoChBssInsertEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pBssid,
	IN CHAR Ssid[],
	IN UCHAR SsidLen,
	IN UCHAR ChannelNo,
	IN UCHAR ExtChOffset,
	IN CHAR Rssi);

void AutoChBssTableInit(
	IN PRTMP_ADAPTER pAd);

void ChannelInfoInit(
	IN PRTMP_ADAPTER pAd);

void AutoChBssTableDestroy(
	IN PRTMP_ADAPTER pAd);

void ChannelInfoDestroy(
	IN PRTMP_ADAPTER pAd);

UCHAR New_APAutoSelectChannel(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN Optimal);

UCHAR APAutoSelectChannel(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN Optimal);

#endif 
