

#include "ttype.h"
#include "tmacro.h"
#include "mac.h"
#include "80211mgr.h"
#include "bssdb.h"
#include "datarate.h"
#include "card.h"
#include "baseband.h"
#include "srom.h"
#include "rf.h"












static int          msglevel                =MSG_LEVEL_INFO;
const BYTE acbyIERate[MAX_RATE] =
{0x02, 0x04, 0x0B, 0x16, 0x0C, 0x12, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6C};

#define AUTORATE_TXOK_CNT       0x0400
#define AUTORATE_TXFAIL_CNT     0x0064
#define AUTORATE_TIMEOUT        10



VOID s_vResetCounter (
    IN PKnownNodeDB psNodeDBTable
    );



VOID
s_vResetCounter (
    IN PKnownNodeDB psNodeDBTable
    )
{
    BYTE            ii;

    
    for(ii=0;ii<=MAX_RATE;ii++) {
        psNodeDBTable->uTxOk[ii] = 0;
        psNodeDBTable->uTxFail[ii] = 0;
    }
}








BYTE
DATARATEbyGetRateIdx (
    IN BYTE byRate
    )
{
    BYTE    ii;

    
    byRate = byRate & 0x7F;

    for (ii = 0; ii < MAX_RATE; ii ++) {
        if (acbyIERate[ii] == byRate)
            return ii;
    }
    return 0;
}




#define AUTORATE_TXCNT_THRESHOLD        20
#define AUTORATE_INC_THRESHOLD          30





WORD
RATEwGetRateIdx(
    IN BYTE byRate
    )
{
    WORD    ii;

    
    byRate = byRate & 0x7F;

    for (ii = 0; ii < MAX_RATE; ii ++) {
        if (acbyIERate[ii] == byRate)
            return ii;
    }
    return 0;
}


VOID
RATEvParseMaxRate (
    IN PVOID pDeviceHandler,
    IN PWLAN_IE_SUPP_RATES pItemRates,
    IN PWLAN_IE_SUPP_RATES pItemExtRates,
    IN BOOL bUpdateBasicRate,
    OUT PWORD pwMaxBasicRate,
    OUT PWORD pwMaxSuppRate,
    OUT PWORD pwSuppRate,
    OUT PBYTE pbyTopCCKRate,
    OUT PBYTE pbyTopOFDMRate
    )
{
PSDevice  pDevice = (PSDevice) pDeviceHandler;
UINT  ii;
BYTE  byHighSuppRate = 0;
BYTE  byRate = 0;
WORD  wOldBasicRate = pDevice->wBasicRate;
UINT  uRateLen;


    if (pItemRates == NULL)
        return;

    *pwSuppRate = 0;
    uRateLen = pItemRates->len;

    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ParseMaxRate Len: %d\n", uRateLen);
    if (pDevice->byBBType != BB_TYPE_11B) {
        if (uRateLen > WLAN_RATES_MAXLEN)
            uRateLen = WLAN_RATES_MAXLEN;
    } else {
        if (uRateLen > WLAN_RATES_MAXLEN_11B)
            uRateLen = WLAN_RATES_MAXLEN_11B;
    }

    for (ii = 0; ii < uRateLen; ii++) {
    	byRate = (BYTE)(pItemRates->abyRates[ii]);
        if (WLAN_MGMT_IS_BASICRATE(byRate) &&
            (bUpdateBasicRate == TRUE))  {
            
            CARDbAddBasicRate((PVOID)pDevice, RATEwGetRateIdx(byRate));
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ParseMaxRate AddBasicRate: %d\n", RATEwGetRateIdx(byRate));
        }
        byRate = (BYTE)(pItemRates->abyRates[ii]&0x7F);
        if (byHighSuppRate == 0)
            byHighSuppRate = byRate;
        if (byRate > byHighSuppRate)
            byHighSuppRate = byRate;
        *pwSuppRate |= (1<<RATEwGetRateIdx(byRate));
    }
    if ((pItemExtRates != NULL) && (pItemExtRates->byElementID == WLAN_EID_EXTSUPP_RATES) &&
        (pDevice->byBBType != BB_TYPE_11B)) {

        UINT  uExtRateLen = pItemExtRates->len;

        if (uExtRateLen > WLAN_RATES_MAXLEN)
            uExtRateLen = WLAN_RATES_MAXLEN;

        for (ii = 0; ii < uExtRateLen ; ii++) {
            byRate = (BYTE)(pItemExtRates->abyRates[ii]);
            
            if (WLAN_MGMT_IS_BASICRATE(pItemExtRates->abyRates[ii])) {
            	
                CARDbAddBasicRate((PVOID)pDevice, RATEwGetRateIdx(byRate));
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ParseMaxRate AddBasicRate: %d\n", RATEwGetRateIdx(byRate));
            }
            byRate = (BYTE)(pItemExtRates->abyRates[ii]&0x7F);
            if (byHighSuppRate == 0)
                byHighSuppRate = byRate;
            if (byRate > byHighSuppRate)
                byHighSuppRate = byRate;
            *pwSuppRate |= (1<<RATEwGetRateIdx(byRate));
            
        }
    } 

    if ((pDevice->byPacketType == PK_TYPE_11GB) && CARDbIsOFDMinBasicRate((PVOID)pDevice)) {
        pDevice->byPacketType = PK_TYPE_11GA;
    }

    *pbyTopCCKRate = pDevice->byTopCCKBasicRate;
    *pbyTopOFDMRate = pDevice->byTopOFDMBasicRate;
    *pwMaxSuppRate = RATEwGetRateIdx(byHighSuppRate);
    if ((pDevice->byPacketType==PK_TYPE_11B) || (pDevice->byPacketType==PK_TYPE_11GB))
       *pwMaxBasicRate = pDevice->byTopCCKBasicRate;
    else
       *pwMaxBasicRate = pDevice->byTopOFDMBasicRate;
    if (wOldBasicRate != pDevice->wBasicRate)
        CARDvSetRSPINF((PVOID)pDevice, pDevice->byBBType);

     DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Exit ParseMaxRate\n");
}



#define AUTORATE_TXCNT_THRESHOLD        20
#define AUTORATE_INC_THRESHOLD          30

VOID
RATEvTxRateFallBack (
    IN PVOID pDeviceHandler,
    IN PKnownNodeDB psNodeDBTable
    )
{
PSDevice        pDevice = (PSDevice) pDeviceHandler;
PSMgmtObject    pMgmt = &(pDevice->sMgmtObj);
#if 1  
WORD            wIdxDownRate = 0;
UINT            ii;
BOOL            bAutoRate[MAX_RATE]    = {TRUE,TRUE,TRUE,TRUE,FALSE,FALSE,TRUE,TRUE,TRUE,TRUE,TRUE,TRUE};
DWORD           dwThroughputTbl[MAX_RATE] = {10, 20, 55, 110, 60, 90, 120, 180, 240, 360, 480, 540};
DWORD           dwThroughput = 0;
WORD            wIdxUpRate = 0;
DWORD           dwTxDiff = 0;

    if (pMgmt->eScanState != WMAC_NO_SCANNING) {
        
        return;
    }
    psNodeDBTable->uTimeCount ++;

    if (psNodeDBTable->uTxFail[MAX_RATE] > psNodeDBTable->uTxOk[MAX_RATE])
        dwTxDiff = psNodeDBTable->uTxFail[MAX_RATE] - psNodeDBTable->uTxOk[MAX_RATE];

    if ((psNodeDBTable->uTxOk[MAX_RATE] < AUTORATE_TXOK_CNT) &&
        (dwTxDiff < AUTORATE_TXFAIL_CNT) &&
        (psNodeDBTable->uTimeCount < AUTORATE_TIMEOUT)) {
        return;
    }

    if (psNodeDBTable->uTimeCount >= AUTORATE_TIMEOUT) {
        psNodeDBTable->uTimeCount = 0;
    }

    for(ii=0;ii<MAX_RATE;ii++) {
        if (psNodeDBTable->wSuppRate & (0x0001<<ii)) {
            if (bAutoRate[ii] == TRUE) {
                wIdxUpRate = (WORD) ii;
            }
        } else {
            bAutoRate[ii] = FALSE;
        }
    }

    for(ii=0;ii<=psNodeDBTable->wTxDataRate;ii++) {
        if ( (psNodeDBTable->uTxOk[ii] != 0) ||
             (psNodeDBTable->uTxFail[ii] != 0) ) {
            dwThroughputTbl[ii] *= psNodeDBTable->uTxOk[ii];
            if (ii < RATE_11M) {
                psNodeDBTable->uTxFail[ii] *= 4;
            }
            dwThroughputTbl[ii] /= (psNodeDBTable->uTxOk[ii] + psNodeDBTable->uTxFail[ii]);
        }
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Rate %d,Ok: %d, Fail:%d, Throughput:%d\n",
                       ii, (int)psNodeDBTable->uTxOk[ii], (int)psNodeDBTable->uTxFail[ii], (int)dwThroughputTbl[ii]);
    }
    dwThroughput = dwThroughputTbl[psNodeDBTable->wTxDataRate];

    wIdxDownRate = psNodeDBTable->wTxDataRate;
    for(ii = psNodeDBTable->wTxDataRate; ii > 0;) {
        ii--;
        if ( (dwThroughputTbl[ii] > dwThroughput) &&
             (bAutoRate[ii]==TRUE) ) {
            dwThroughput = dwThroughputTbl[ii];
            wIdxDownRate = (WORD) ii;
        }
    }
    psNodeDBTable->wTxDataRate = wIdxDownRate;
    if (psNodeDBTable->uTxOk[MAX_RATE]) {
        if (psNodeDBTable->uTxOk[MAX_RATE] >
           (psNodeDBTable->uTxFail[MAX_RATE] * 4) ) {
            psNodeDBTable->wTxDataRate = wIdxUpRate;
        }
    }else { 
        if (psNodeDBTable->uTxFail[MAX_RATE] == 0)
            psNodeDBTable->wTxDataRate = wIdxUpRate;
    }

    if (pDevice->byBBType == BB_TYPE_11A) {
        if (psNodeDBTable->wTxDataRate <= RATE_11M)
            psNodeDBTable->wTxDataRate = RATE_6M;
    }
    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"uTxOk[MAX_RATE] %d, uTxFail[MAX_RATE]:%d\n",(int)psNodeDBTable->uTxOk[MAX_RATE], (int)psNodeDBTable->uTxFail[MAX_RATE]);
    s_vResetCounter(psNodeDBTable);
    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Rate: %d, U:%d, D:%d\n", (int)psNodeDBTable->wTxDataRate, (int)wIdxUpRate, (int)wIdxDownRate);
    return;
#else  
WORD            wIdxUpRate = 0;
BOOL            bAutoRate[MAX_RATE]    = {TRUE,TRUE,TRUE,TRUE,FALSE,FALSE,TRUE,TRUE,TRUE,TRUE,TRUE,TRUE};
UINT            ii;
long  ldBm;

    if (pMgmt->eScanState != WMAC_NO_SCANNING) {
        
        return;
    }

    for(ii=0;ii<MAX_RATE;ii++) {
        if (psNodeDBTable->wSuppRate & (0x0001<<ii)) {
            if (bAutoRate[ii] == TRUE) {
                wIdxUpRate = (WORD) ii;
            }
        } else {
            bAutoRate[ii] = FALSE;
        }
    }

         RFvRSSITodBm(pDevice, (BYTE)(pDevice->uCurrRSSI), &ldBm);

	if (ldBm > -55) {
		if ( psNodeDBTable->wSuppRate & (0x0001<<RATE_54M) )  
      		{
	  		psNodeDBTable->wTxDataRate = RATE_54M;
		}
		else{ 
	  		psNodeDBTable->wTxDataRate = RATE_11M;
		}
	}

if (wIdxUpRate == RATE_54M ) {     
		if (ldBm > -56 )
			psNodeDBTable->wTxDataRate = RATE_54M;
		else if (ldBm > -61 )
			psNodeDBTable->wTxDataRate = RATE_48M;
		else if (ldBm > -66 )
			psNodeDBTable->wTxDataRate = RATE_36M;
		else if (ldBm > -72 )
			psNodeDBTable->wTxDataRate = RATE_24M;
		else if (ldBm > -80 )
			psNodeDBTable->wTxDataRate = RATE_5M;
		else {
			psNodeDBTable->wTxDataRate = RATE_1M;
			
		}
	}
	else {  
		if (ldBm > -65 )
			psNodeDBTable->wTxDataRate = RATE_11M;
		else if (ldBm > -75 )
			psNodeDBTable->wTxDataRate = RATE_5M;
		else
			psNodeDBTable->wTxDataRate = RATE_1M;
	}

   return;
#endif
}


BYTE
RATEuSetIE (
    IN PWLAN_IE_SUPP_RATES pSrcRates,
    IN PWLAN_IE_SUPP_RATES pDstRates,
    IN UINT                uRateLen
    )
{
    UINT ii, uu, uRateCnt = 0;

    if ((pSrcRates == NULL) || (pDstRates == NULL))
        return 0;

    if (pSrcRates->len == 0)
        return 0;

    for (ii = 0; ii < uRateLen; ii++) {
        for (uu = 0; uu < pSrcRates->len; uu++) {
            if ((pSrcRates->abyRates[uu] & 0x7F) == acbyIERate[ii]) {
                pDstRates->abyRates[uRateCnt ++] = pSrcRates->abyRates[uu];
                break;
            }
        }
    }
    return (BYTE)uRateCnt;
}

