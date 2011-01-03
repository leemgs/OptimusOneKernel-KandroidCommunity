

#ifndef __WPA2_H__
#define __WPA2_H__

#include "ttype.h"
#include "80211mgr.h"
#include "80211hdr.h"
#include "bssdb.h"


#define MAX_PMKID_CACHE         16

typedef struct tagsPMKIDInfo {
    BYTE    abyBSSID[6];
    BYTE    abyPMKID[16];
} PMKIDInfo, *PPMKIDInfo;

typedef struct tagSPMKIDCache {
    ULONG       BSSIDInfoCount;
    PMKIDInfo   BSSIDInfo[MAX_PMKID_CACHE];
} SPMKIDCache, *PSPMKIDCache;










VOID
WPA2_ClearRSN (
    IN PKnownBSS        pBSSNode
    );

VOID
WPA2vParseRSN (
    IN PKnownBSS        pBSSNode,
    IN PWLAN_IE_RSN     pRSN
    );

UINT
WPA2uSetIEs(
    IN PVOID pMgmtHandle,
    OUT PWLAN_IE_RSN pRSNIEs
    );

#endif 
