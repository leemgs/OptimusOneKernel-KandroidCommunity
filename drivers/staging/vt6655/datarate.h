
#ifndef __DATARATE_H__
#define __DATARATE_H__



#define FALLBACK_PKT_COLLECT_TR_H  50   
#define FALLBACK_PKT_COLLECT_TR_L  10   
#define FALLBACK_POLL_SECOND       5    
#define FALLBACK_RECOVER_SECOND    30   
#define FALLBACK_THRESHOLD         15   
#define UPGRADE_THRESHOLD          5    
#define UPGRADE_CNT_THRD           3    
#define RETRY_TIMES_THRD_H         2    
#define RETRY_TIMES_THRD_L         1    














VOID
RATEvParseMaxRate(
    IN PVOID pDeviceHandler,
    IN PWLAN_IE_SUPP_RATES pItemRates,
    IN PWLAN_IE_SUPP_RATES pItemExtRates,
    IN BOOL bUpdateBasicRate,
    OUT PWORD pwMaxBasicRate,
    OUT PWORD pwMaxSuppRate,
    OUT PWORD pwSuppRate,
    OUT PBYTE pbyTopCCKRate,
    OUT PBYTE pbyTopOFDMRate
    );

VOID
RATEvTxRateFallBack(
    IN PVOID pDeviceHandler,
    IN PKnownNodeDB psNodeDBTable
    );

BYTE
RATEuSetIE(
    IN PWLAN_IE_SUPP_RATES pSrcRates,
    IN PWLAN_IE_SUPP_RATES pDstRates,
    IN UINT                uRateLen
    );

WORD
wGetRateIdx(
    IN BYTE byRate
    );


BYTE
DATARATEbyGetRateIdx(
    IN BYTE byRate
    );


#endif 
