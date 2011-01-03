

#include "ttype.h"
#include "tmacro.h"
#include "tether.h"
#include "device.h"
#include "80211hdr.h"
#include "bssdb.h"
#include "wmgr.h"
#include "datarate.h"
#include "desc.h"
#include "wcmd.h"
#include "wpa.h"
#include "baseband.h"
#include "rf.h"
#include "card.h"
#include "mac.h"
#include "wpa2.h"
#include "iowpa.h"










static int          msglevel                =MSG_LEVEL_INFO;




const WORD             awHWRetry0[5][5] = {
                                            {RATE_18M, RATE_18M, RATE_12M, RATE_12M, RATE_12M},
                                            {RATE_24M, RATE_24M, RATE_18M, RATE_12M, RATE_12M},
                                            {RATE_36M, RATE_36M, RATE_24M, RATE_18M, RATE_18M},
                                            {RATE_48M, RATE_48M, RATE_36M, RATE_24M, RATE_24M},
                                            {RATE_54M, RATE_54M, RATE_48M, RATE_36M, RATE_36M}
                                           };
const WORD             awHWRetry1[5][5] = {
                                            {RATE_18M, RATE_18M, RATE_12M, RATE_6M, RATE_6M},
                                            {RATE_24M, RATE_24M, RATE_18M, RATE_6M, RATE_6M},
                                            {RATE_36M, RATE_36M, RATE_24M, RATE_12M, RATE_12M},
                                            {RATE_48M, RATE_48M, RATE_24M, RATE_12M, RATE_12M},
                                            {RATE_54M, RATE_54M, RATE_36M, RATE_18M, RATE_18M}
                                           };





VOID s_vCheckSensitivity(
    IN HANDLE hDeviceContext
    );

#ifdef Calcu_LinkQual
VOID s_uCalculateLinkQual(
    IN HANDLE hDeviceContext
    );
#endif


VOID s_vCheckPreEDThreshold(
    IN HANDLE hDeviceContext
    );











PKnownBSS
BSSpSearchBSSList(
    IN HANDLE hDeviceContext,
    IN PBYTE pbyDesireBSSID,
    IN PBYTE pbyDesireSSID,
    IN CARD_PHY_TYPE  ePhyType
    )
{
    PSDevice        pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    PBYTE           pbyBSSID = NULL;
    PWLAN_IE_SSID   pSSID = NULL;
    PKnownBSS       pCurrBSS = NULL;
    PKnownBSS       pSelect = NULL;
BYTE                 ZeroBSSID[WLAN_BSSID_LEN]={0x00,0x00,0x00,0x00,0x00,0x00};
    UINT            ii = 0;

    if (pbyDesireBSSID != NULL) {
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"BSSpSearchBSSList BSSID[%02X %02X %02X-%02X %02X %02X]\n",
                            *pbyDesireBSSID,*(pbyDesireBSSID+1),*(pbyDesireBSSID+2),
                            *(pbyDesireBSSID+3),*(pbyDesireBSSID+4),*(pbyDesireBSSID+5));
        if ((!IS_BROADCAST_ADDRESS(pbyDesireBSSID)) &&
	     (memcmp(pbyDesireBSSID, ZeroBSSID, 6)!= 0)){
            pbyBSSID = pbyDesireBSSID;
        }
    }
    if (pbyDesireSSID != NULL) {
        if (((PWLAN_IE_SSID)pbyDesireSSID)->len != 0) {
            pSSID = (PWLAN_IE_SSID) pbyDesireSSID;
        }
    }

    if (pbyBSSID != NULL) {
        
        for (ii = 0; ii <MAX_BSS_NUM; ii++) {
            pCurrBSS = &(pMgmt->sBSSList[ii]);
if(pDevice->bLinkPass==FALSE) pCurrBSS->bSelected = FALSE;
            if ((pCurrBSS->bActive) &&
                (pCurrBSS->bSelected == FALSE)) {
                if (IS_ETH_ADDRESS_EQUAL(pCurrBSS->abyBSSID, pbyBSSID)) {
                    if (pSSID != NULL) {
                        
                        if ( !memcmp(pSSID->abySSID,
                            ((PWLAN_IE_SSID)pCurrBSS->abySSID)->abySSID,
                            pSSID->len)) {
                            if ((pMgmt->eConfigMode == WMAC_CONFIG_AUTO) ||
                                ((pMgmt->eConfigMode == WMAC_CONFIG_IBSS_STA) && WLAN_GET_CAP_INFO_IBSS(pCurrBSS->wCapInfo)) ||
                                ((pMgmt->eConfigMode == WMAC_CONFIG_ESS_STA) && WLAN_GET_CAP_INFO_ESS(pCurrBSS->wCapInfo))
                                ) {
                                pCurrBSS->bSelected = TRUE;
                                return(pCurrBSS);
                            }
                        }
                    } else {
                        if ((pMgmt->eConfigMode == WMAC_CONFIG_AUTO) ||
                            ((pMgmt->eConfigMode == WMAC_CONFIG_IBSS_STA) && WLAN_GET_CAP_INFO_IBSS(pCurrBSS->wCapInfo)) ||
                            ((pMgmt->eConfigMode == WMAC_CONFIG_ESS_STA) && WLAN_GET_CAP_INFO_ESS(pCurrBSS->wCapInfo))
                            ) {
                            pCurrBSS->bSelected = TRUE;
                            return(pCurrBSS);
                        }
                    }
                }
            }
        }
    } else {
        
        for (ii = 0; ii <MAX_BSS_NUM; ii++) {
            pCurrBSS = &(pMgmt->sBSSList[ii]);
	
	  pCurrBSS->bSelected = FALSE;
          if (pCurrBSS->bActive) {

                if (pSSID != NULL) {
                    
                    if (! !memcmp(pSSID->abySSID,
                        ((PWLAN_IE_SSID)pCurrBSS->abySSID)->abySSID,
                        pSSID->len) ||
                        (pSSID->len != ((PWLAN_IE_SSID)pCurrBSS->abySSID)->len)) {
                        
                        continue;
                      }
                }
                if (((pMgmt->eConfigMode == WMAC_CONFIG_IBSS_STA) && WLAN_GET_CAP_INFO_ESS(pCurrBSS->wCapInfo)) ||
                    ((pMgmt->eConfigMode == WMAC_CONFIG_ESS_STA) && WLAN_GET_CAP_INFO_IBSS(pCurrBSS->wCapInfo))
                    ){
                    
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"BSS type mismatch.... Config[%d] BSS[0x%04x]\n", pMgmt->eConfigMode, pCurrBSS->wCapInfo);
                    continue;
                }

                if (ePhyType != PHY_TYPE_AUTO) {
                    if (((ePhyType == PHY_TYPE_11A) && (PHY_TYPE_11A != pCurrBSS->eNetworkTypeInUse)) ||
                        ((ePhyType != PHY_TYPE_11A) && (PHY_TYPE_11A == pCurrBSS->eNetworkTypeInUse))) {
                        
                        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Physical type mismatch.... ePhyType[%d] BSS[%d]\n", ePhyType, pCurrBSS->eNetworkTypeInUse);
                        continue;
                    }
                }

                if (pSelect == NULL) {
                    pSelect = pCurrBSS;
                } else {
                    
                    if (pCurrBSS->uRSSI < pSelect->uRSSI) {
                        pSelect = pCurrBSS;
                    }
                }
            }
        }
        if (pSelect != NULL) {
            pSelect->bSelected = TRUE;


            return(pSelect);
        }
    }
    return(NULL);

}





VOID
BSSvClearBSSList(
    IN HANDLE hDeviceContext,
    IN BOOL bKeepCurrBSSID
    )
{
    PSDevice     pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    UINT            ii;

    for (ii = 0; ii < MAX_BSS_NUM; ii++) {
        if (bKeepCurrBSSID) {
            if (pMgmt->sBSSList[ii].bActive &&
                IS_ETH_ADDRESS_EQUAL(pMgmt->sBSSList[ii].abyBSSID, pMgmt->abyCurrBSSID)) {
               
                continue;
            }
        }

        if ((pMgmt->sBSSList[ii].bActive) && (pMgmt->sBSSList[ii].uClearCount < BSS_CLEAR_COUNT)) {
             pMgmt->sBSSList[ii].uClearCount ++;
             continue;
        }

        pMgmt->sBSSList[ii].bActive = FALSE;
        memset(&pMgmt->sBSSList[ii], 0, sizeof(KnownBSS));
    }
    BSSvClearAnyBSSJoinRecord(pDevice);

    return;
}




PKnownBSS
BSSpAddrIsInBSSList(
    IN HANDLE hDeviceContext,
    IN PBYTE abyBSSID,
    IN PWLAN_IE_SSID pSSID
    )
{
    PSDevice     pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    PKnownBSS       pBSSList = NULL;
    UINT            ii;

    for (ii = 0; ii < MAX_BSS_NUM; ii++) {
        pBSSList = &(pMgmt->sBSSList[ii]);
        if (pBSSList->bActive) {
            if (IS_ETH_ADDRESS_EQUAL(pBSSList->abyBSSID, abyBSSID)) {


                if (pSSID->len == ((PWLAN_IE_SSID)pBSSList->abySSID)->len){
                    if (memcmp(pSSID->abySSID,
                            ((PWLAN_IE_SSID)pBSSList->abySSID)->abySSID,
                            pSSID->len) == 0)
                        return pBSSList;
                }
            }
        }
    }

    return NULL;
};





BOOL
BSSbInsertToBSSList (
    IN HANDLE hDeviceContext,
    IN PBYTE abyBSSIDAddr,
    IN QWORD qwTimestamp,
    IN WORD wBeaconInterval,
    IN WORD wCapInfo,
    IN BYTE byCurrChannel,
    IN PWLAN_IE_SSID pSSID,
    IN PWLAN_IE_SUPP_RATES pSuppRates,
    IN PWLAN_IE_SUPP_RATES pExtSuppRates,
    IN PERPObject psERP,
    IN PWLAN_IE_RSN pRSN,
    IN PWLAN_IE_RSN_EXT pRSNWPA,
    IN PWLAN_IE_COUNTRY pIE_Country,
    IN PWLAN_IE_QUIET pIE_Quiet,
    IN UINT uIELength,
    IN PBYTE pbyIEs,
    IN HANDLE pRxPacketContext
    )
{

    PSDevice     pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    PSRxMgmtPacket  pRxPacket = (PSRxMgmtPacket)pRxPacketContext;
    PKnownBSS       pBSSList = NULL;
    UINT            ii;
    BOOL            bParsingQuiet = FALSE;
    PWLAN_IE_QUIET  pQuiet = NULL;



    pBSSList = (PKnownBSS)&(pMgmt->sBSSList[0]);

    for (ii = 0; ii < MAX_BSS_NUM; ii++) {
        pBSSList = (PKnownBSS)&(pMgmt->sBSSList[ii]);
        if (!pBSSList->bActive)
                break;
    }

    if (ii == MAX_BSS_NUM){
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "Get free KnowBSS node failed.\n");
        return FALSE;
    }
    
    pBSSList->bActive = TRUE;
    memcpy( pBSSList->abyBSSID, abyBSSIDAddr, WLAN_BSSID_LEN);
    HIDWORD(pBSSList->qwBSSTimestamp) = cpu_to_le32(HIDWORD(qwTimestamp));
    LODWORD(pBSSList->qwBSSTimestamp) = cpu_to_le32(LODWORD(qwTimestamp));
    pBSSList->wBeaconInterval = cpu_to_le16(wBeaconInterval);
    pBSSList->wCapInfo = cpu_to_le16(wCapInfo);
    pBSSList->uClearCount = 0;

    if (pSSID->len > WLAN_SSID_MAXLEN)
        pSSID->len = WLAN_SSID_MAXLEN;
    memcpy( pBSSList->abySSID, pSSID, pSSID->len + WLAN_IEHDR_LEN);

    pBSSList->uChannel = byCurrChannel;

    if (pSuppRates->len > WLAN_RATES_MAXLEN)
        pSuppRates->len = WLAN_RATES_MAXLEN;
    memcpy( pBSSList->abySuppRates, pSuppRates, pSuppRates->len + WLAN_IEHDR_LEN);

    if (pExtSuppRates != NULL) {
        if (pExtSuppRates->len > WLAN_RATES_MAXLEN)
            pExtSuppRates->len = WLAN_RATES_MAXLEN;
        memcpy(pBSSList->abyExtSuppRates, pExtSuppRates, pExtSuppRates->len + WLAN_IEHDR_LEN);
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"BSSbInsertToBSSList: pExtSuppRates->len = %d\n", pExtSuppRates->len);

    } else {
        memset(pBSSList->abyExtSuppRates, 0, WLAN_IEHDR_LEN + WLAN_RATES_MAXLEN + 1);
    }
    pBSSList->sERP.byERP = psERP->byERP;
    pBSSList->sERP.bERPExist = psERP->bERPExist;

    
    if (pBSSList->uChannel > CB_MAX_CHANNEL_24G) {
        pBSSList->eNetworkTypeInUse = PHY_TYPE_11A;
    } else {
        if (pBSSList->sERP.bERPExist == TRUE) {
            pBSSList->eNetworkTypeInUse = PHY_TYPE_11G;
        } else {
            pBSSList->eNetworkTypeInUse = PHY_TYPE_11B;
        }
    }

    pBSSList->byRxRate = pRxPacket->byRxRate;
    pBSSList->qwLocalTSF = pRxPacket->qwLocalTSF;
    pBSSList->uRSSI = pRxPacket->uRSSI;
    pBSSList->bySQ = pRxPacket->bySQ;

   if ((pMgmt->eCurrMode == WMAC_MODE_ESS_STA) &&
        (pMgmt->eCurrState == WMAC_STATE_ASSOC)) {
        
        if (pBSSList == pMgmt->pCurrBSS) {
            bParsingQuiet = TRUE;
        }
    }

    WPA_ClearRSN(pBSSList);

    if (pRSNWPA != NULL) {
        UINT uLen = pRSNWPA->len + 2;

        if (uLen <= (uIELength - (UINT)(ULONG_PTR)((PBYTE)pRSNWPA - pbyIEs))) {
            pBSSList->wWPALen = uLen;
            memcpy(pBSSList->byWPAIE, pRSNWPA, uLen);
            WPA_ParseRSN(pBSSList, pRSNWPA);
        }
    }

    WPA2_ClearRSN(pBSSList);

    if (pRSN != NULL) {
        UINT uLen = pRSN->len + 2;
        if (uLen <= (uIELength - (UINT)(ULONG_PTR)((PBYTE)pRSN - pbyIEs))) {
            pBSSList->wRSNLen = uLen;
            memcpy(pBSSList->byRSNIE, pRSN, uLen);
            WPA2vParseRSN(pBSSList, pRSN);
        }
    }

    if ((pMgmt->eAuthenMode == WMAC_AUTH_WPA2) || (pBSSList->bWPA2Valid == TRUE)) {

        PSKeyItem  pTransmitKey = NULL;
        BOOL       bIs802_1x = FALSE;

        for (ii = 0; ii < pBSSList->wAKMSSAuthCount; ii ++) {
            if (pBSSList->abyAKMSSAuthType[ii] == WLAN_11i_AKMSS_802_1X) {
                bIs802_1x = TRUE;
                break;
            }
        }
        if ((bIs802_1x == TRUE) && (pSSID->len == ((PWLAN_IE_SSID)pMgmt->abyDesireSSID)->len) &&
            ( !memcmp(pSSID->abySSID, ((PWLAN_IE_SSID)pMgmt->abyDesireSSID)->abySSID, pSSID->len))) {

            bAdd_PMKID_Candidate((HANDLE)pDevice, pBSSList->abyBSSID, &pBSSList->sRSNCapObj);

            if ((pDevice->bLinkPass == TRUE) && (pMgmt->eCurrState == WMAC_STATE_ASSOC)) {
                if ((KeybGetTransmitKey(&(pDevice->sKey), pDevice->abyBSSID, PAIRWISE_KEY, &pTransmitKey) == TRUE) ||
                    (KeybGetTransmitKey(&(pDevice->sKey), pDevice->abyBSSID, GROUP_KEY, &pTransmitKey) == TRUE)) {
                    pDevice->gsPMKIDCandidate.StatusType = Ndis802_11StatusType_PMKID_CandidateList;
                    pDevice->gsPMKIDCandidate.Version = 1;

                }

            }
        }
    }

    if (pDevice->bUpdateBBVGA) {
        
        pBSSList->byRSSIStatCnt = 0;
        RFvRSSITodBm(pDevice, (BYTE)(pRxPacket->uRSSI), &pBSSList->ldBmMAX);
        pBSSList->ldBmAverage[0] = pBSSList->ldBmMAX;
        for (ii = 1; ii < RSSI_STAT_COUNT; ii++)
            pBSSList->ldBmAverage[ii] = 0;
    }

    if ((pIE_Country != NULL) &&
        (pMgmt->b11hEnable == TRUE)) {
        CARDvSetCountryInfo(pMgmt->pAdapter,
                            pBSSList->eNetworkTypeInUse,
                            pIE_Country);
    }

    if ((bParsingQuiet == TRUE) && (pIE_Quiet != NULL)) {
        if ((((PWLAN_IE_QUIET)pIE_Quiet)->len == 8) &&
            (((PWLAN_IE_QUIET)pIE_Quiet)->byQuietCount != 0)) {
            
            if (pQuiet == NULL) {
                pQuiet = (PWLAN_IE_QUIET)pIE_Quiet;
                CARDbSetQuiet(  pMgmt->pAdapter,
                                TRUE,
                                pQuiet->byQuietCount,
                                pQuiet->byQuietPeriod,
                                *((PWORD)pQuiet->abyQuietDuration),
                                *((PWORD)pQuiet->abyQuietOffset)
                                );
            } else {
                pQuiet = (PWLAN_IE_QUIET)pIE_Quiet;
                CARDbSetQuiet(  pMgmt->pAdapter,
                                FALSE,
                                pQuiet->byQuietCount,
                                pQuiet->byQuietPeriod,
                                *((PWORD)pQuiet->abyQuietDuration),
                                *((PWORD)pQuiet->abyQuietOffset)
                                );
            }
        }
    }

    if ((bParsingQuiet == TRUE) &&
        (pQuiet != NULL)) {
        CARDbStartQuiet(pMgmt->pAdapter);
    }

    pBSSList->uIELength = uIELength;
    if (pBSSList->uIELength > WLAN_BEACON_FR_MAXLEN)
        pBSSList->uIELength = WLAN_BEACON_FR_MAXLEN;
    memcpy(pBSSList->abyIEs, pbyIEs, pBSSList->uIELength);

    return TRUE;
}





BOOL
BSSbUpdateToBSSList (
    IN HANDLE hDeviceContext,
    IN QWORD qwTimestamp,
    IN WORD wBeaconInterval,
    IN WORD wCapInfo,
    IN BYTE byCurrChannel,
    IN BOOL bChannelHit,
    IN PWLAN_IE_SSID pSSID,
    IN PWLAN_IE_SUPP_RATES pSuppRates,
    IN PWLAN_IE_SUPP_RATES pExtSuppRates,
    IN PERPObject psERP,
    IN PWLAN_IE_RSN pRSN,
    IN PWLAN_IE_RSN_EXT pRSNWPA,
    IN PWLAN_IE_COUNTRY pIE_Country,
    IN PWLAN_IE_QUIET pIE_Quiet,
    IN PKnownBSS pBSSList,
    IN UINT uIELength,
    IN PBYTE pbyIEs,
    IN HANDLE pRxPacketContext
    )
{
    int             ii;
    PSDevice        pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    PSRxMgmtPacket  pRxPacket = (PSRxMgmtPacket)pRxPacketContext;
    LONG            ldBm;
    BOOL            bParsingQuiet = FALSE;
    PWLAN_IE_QUIET  pQuiet = NULL;



    if (pBSSList == NULL)
        return FALSE;


    HIDWORD(pBSSList->qwBSSTimestamp) = cpu_to_le32(HIDWORD(qwTimestamp));
    LODWORD(pBSSList->qwBSSTimestamp) = cpu_to_le32(LODWORD(qwTimestamp));
    pBSSList->wBeaconInterval = cpu_to_le16(wBeaconInterval);
    pBSSList->wCapInfo = cpu_to_le16(wCapInfo);
    pBSSList->uClearCount = 0;
    pBSSList->uChannel = byCurrChannel;


    if (pSSID->len > WLAN_SSID_MAXLEN)
        pSSID->len = WLAN_SSID_MAXLEN;

    if ((pSSID->len != 0) && (pSSID->abySSID[0] != 0))
        memcpy(pBSSList->abySSID, pSSID, pSSID->len + WLAN_IEHDR_LEN);
    memcpy(pBSSList->abySuppRates, pSuppRates,pSuppRates->len + WLAN_IEHDR_LEN);

    if (pExtSuppRates != NULL) {
        memcpy(pBSSList->abyExtSuppRates, pExtSuppRates,pExtSuppRates->len + WLAN_IEHDR_LEN);
    } else {
        memset(pBSSList->abyExtSuppRates, 0, WLAN_IEHDR_LEN + WLAN_RATES_MAXLEN + 1);
    }
    pBSSList->sERP.byERP = psERP->byERP;
    pBSSList->sERP.bERPExist = psERP->bERPExist;

    
    if (pBSSList->uChannel > CB_MAX_CHANNEL_24G) {
        pBSSList->eNetworkTypeInUse = PHY_TYPE_11A;
    } else {
        if (pBSSList->sERP.bERPExist == TRUE) {
            pBSSList->eNetworkTypeInUse = PHY_TYPE_11G;
        } else {
            pBSSList->eNetworkTypeInUse = PHY_TYPE_11B;
        }
    }

    pBSSList->byRxRate = pRxPacket->byRxRate;
    pBSSList->qwLocalTSF = pRxPacket->qwLocalTSF;
    if(bChannelHit)
        pBSSList->uRSSI = pRxPacket->uRSSI;
    pBSSList->bySQ = pRxPacket->bySQ;

   if ((pMgmt->eCurrMode == WMAC_MODE_ESS_STA) &&
        (pMgmt->eCurrState == WMAC_STATE_ASSOC)) {
        
        if (pBSSList == pMgmt->pCurrBSS) {
            bParsingQuiet = TRUE;
        }
    }

   WPA_ClearRSN(pBSSList);         

    if (pRSNWPA != NULL) {
        UINT uLen = pRSNWPA->len + 2;
        if (uLen <= (uIELength - (UINT)(ULONG_PTR)((PBYTE)pRSNWPA - pbyIEs))) {
            pBSSList->wWPALen = uLen;
            memcpy(pBSSList->byWPAIE, pRSNWPA, uLen);
            WPA_ParseRSN(pBSSList, pRSNWPA);
        }
    }

   WPA2_ClearRSN(pBSSList);  

    if (pRSN != NULL) {
        UINT uLen = pRSN->len + 2;
        if (uLen <= (uIELength - (UINT)(ULONG_PTR)((PBYTE)pRSN - pbyIEs))) {
            pBSSList->wRSNLen = uLen;
            memcpy(pBSSList->byRSNIE, pRSN, uLen);
            WPA2vParseRSN(pBSSList, pRSN);
        }
    }

    if (pRxPacket->uRSSI != 0) {
        RFvRSSITodBm(pDevice, (BYTE)(pRxPacket->uRSSI), &ldBm);
        
        pBSSList->byRSSIStatCnt++;
        pBSSList->byRSSIStatCnt %= RSSI_STAT_COUNT;
        pBSSList->ldBmAverage[pBSSList->byRSSIStatCnt] = ldBm;
        for(ii=0;ii<RSSI_STAT_COUNT;ii++) {
            if (pBSSList->ldBmAverage[ii] != 0) {
                pBSSList->ldBmMAX = max(pBSSList->ldBmAverage[ii], ldBm);
            }
        }
    }

    if ((pIE_Country != NULL) &&
        (pMgmt->b11hEnable == TRUE)) {
        CARDvSetCountryInfo(pMgmt->pAdapter,
                            pBSSList->eNetworkTypeInUse,
                            pIE_Country);
    }

    if ((bParsingQuiet == TRUE) && (pIE_Quiet != NULL)) {
        if ((((PWLAN_IE_QUIET)pIE_Quiet)->len == 8) &&
            (((PWLAN_IE_QUIET)pIE_Quiet)->byQuietCount != 0)) {
            
            if (pQuiet == NULL) {
                pQuiet = (PWLAN_IE_QUIET)pIE_Quiet;
                CARDbSetQuiet(  pMgmt->pAdapter,
                                TRUE,
                                pQuiet->byQuietCount,
                                pQuiet->byQuietPeriod,
                                *((PWORD)pQuiet->abyQuietDuration),
                                *((PWORD)pQuiet->abyQuietOffset)
                                );
            } else {
                pQuiet = (PWLAN_IE_QUIET)pIE_Quiet;
                CARDbSetQuiet(  pMgmt->pAdapter,
                                FALSE,
                                pQuiet->byQuietCount,
                                pQuiet->byQuietPeriod,
                                *((PWORD)pQuiet->abyQuietDuration),
                                *((PWORD)pQuiet->abyQuietOffset)
                                );
            }
        }
    }

    if ((bParsingQuiet == TRUE) &&
        (pQuiet != NULL)) {
        CARDbStartQuiet(pMgmt->pAdapter);
    }

    pBSSList->uIELength = uIELength;
    if (pBSSList->uIELength > WLAN_BEACON_FR_MAXLEN)
        pBSSList->uIELength = WLAN_BEACON_FR_MAXLEN;
    memcpy(pBSSList->abyIEs, pbyIEs, pBSSList->uIELength);

    return TRUE;
}







BOOL
BSSDBbIsSTAInNodeDB(
    IN PVOID pMgmtObject,
    IN PBYTE abyDstAddr,
    OUT PUINT puNodeIndex
    )
{
    PSMgmtObject    pMgmt = (PSMgmtObject) pMgmtObject;
    UINT            ii;

    
    for (ii = 1; ii < (MAX_NODE_NUM + 1); ii++) {
        if (pMgmt->sNodeDBTable[ii].bActive) {
            if (IS_ETH_ADDRESS_EQUAL(abyDstAddr, pMgmt->sNodeDBTable[ii].abyMACAddr)) {
                *puNodeIndex = ii;
                return TRUE;
            }
        }
    }

   return FALSE;
};




VOID
BSSvCreateOneNode(
    IN HANDLE hDeviceContext,
    OUT PUINT puNodeIndex
    )
{

    PSDevice     pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    UINT            ii;
    UINT            BigestCount = 0;
    UINT            SelectIndex;
    struct sk_buff  *skb;
    
    
    SelectIndex = 1;
    for (ii = 1; ii < (MAX_NODE_NUM + 1); ii++) {
        if (pMgmt->sNodeDBTable[ii].bActive) {
            if (pMgmt->sNodeDBTable[ii].uInActiveCount > BigestCount) {
                BigestCount = pMgmt->sNodeDBTable[ii].uInActiveCount;
                SelectIndex = ii;
            }
        }
        else {
            break;
        }
    }

    
    if ( ii == (MAX_NODE_NUM + 1)) {
        *puNodeIndex = SelectIndex;
        DBG_PRT(MSG_LEVEL_NOTICE, KERN_INFO "Replace inactive node = %d\n", SelectIndex);
        
        if (pMgmt->sNodeDBTable[*puNodeIndex].sTxPSQueue.next != NULL) {
      	    while ((skb = skb_dequeue(&pMgmt->sNodeDBTable[*puNodeIndex].sTxPSQueue)) != NULL)
            dev_kfree_skb(skb);
        }
    }
    else {
        *puNodeIndex = ii;
    }

    memset(&pMgmt->sNodeDBTable[*puNodeIndex], 0, sizeof(KnownNodeDB));
    pMgmt->sNodeDBTable[*puNodeIndex].bActive = TRUE;
    pMgmt->sNodeDBTable[*puNodeIndex].uRatePollTimeout = FALLBACK_POLL_SECOND;
    
    skb_queue_head_init(&pMgmt->sNodeDBTable[*puNodeIndex].sTxPSQueue);
    pMgmt->sNodeDBTable[*puNodeIndex].byAuthSequence = 0;
    pMgmt->sNodeDBTable[*puNodeIndex].wEnQueueCnt = 0;
    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "Create node index = %d\n", ii);
    return;
};




VOID
BSSvRemoveOneNode(
    IN HANDLE hDeviceContext,
    IN UINT uNodeIndex
    )
{

    PSDevice        pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    BYTE            byMask[8] = {1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80};
    struct sk_buff  *skb;


    while ((skb = skb_dequeue(&pMgmt->sNodeDBTable[uNodeIndex].sTxPSQueue)) != NULL)
            dev_kfree_skb(skb);
    
    memset(&pMgmt->sNodeDBTable[uNodeIndex], 0, sizeof(KnownNodeDB));
    
    pMgmt->abyPSTxMap[pMgmt->sNodeDBTable[uNodeIndex].wAID >> 3] &=  ~byMask[pMgmt->sNodeDBTable[uNodeIndex].wAID & 7];

    return;
};


VOID
BSSvUpdateAPNode(
    IN HANDLE hDeviceContext,
    IN PWORD pwCapInfo,
    IN PWLAN_IE_SUPP_RATES pSuppRates,
    IN PWLAN_IE_SUPP_RATES pExtSuppRates
    )
{
    PSDevice     pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    UINT            uRateLen = WLAN_RATES_MAXLEN;

    memset(&pMgmt->sNodeDBTable[0], 0, sizeof(KnownNodeDB));

    pMgmt->sNodeDBTable[0].bActive = TRUE;
    if (pDevice->eCurrentPHYType == PHY_TYPE_11B) {
        uRateLen = WLAN_RATES_MAXLEN_11B;
    }
    pMgmt->abyCurrSuppRates[1] = RATEuSetIE((PWLAN_IE_SUPP_RATES)pSuppRates,
                                            (PWLAN_IE_SUPP_RATES)pMgmt->abyCurrSuppRates,
                                            uRateLen);
    pMgmt->abyCurrExtSuppRates[1] = RATEuSetIE((PWLAN_IE_SUPP_RATES)pExtSuppRates,
                                            (PWLAN_IE_SUPP_RATES)pMgmt->abyCurrExtSuppRates,
                                            uRateLen);
    RATEvParseMaxRate((PVOID) pDevice,
                       (PWLAN_IE_SUPP_RATES)pMgmt->abyCurrSuppRates,
                       (PWLAN_IE_SUPP_RATES)pMgmt->abyCurrExtSuppRates,
                       TRUE,
                       &(pMgmt->sNodeDBTable[0].wMaxBasicRate),
                       &(pMgmt->sNodeDBTable[0].wMaxSuppRate),
                       &(pMgmt->sNodeDBTable[0].wSuppRate),
                       &(pMgmt->sNodeDBTable[0].byTopCCKBasicRate),
                       &(pMgmt->sNodeDBTable[0].byTopOFDMBasicRate)
                      );
    memcpy(pMgmt->sNodeDBTable[0].abyMACAddr, pMgmt->abyCurrBSSID, WLAN_ADDR_LEN);
    pMgmt->sNodeDBTable[0].wTxDataRate = pMgmt->sNodeDBTable[0].wMaxSuppRate;
    pMgmt->sNodeDBTable[0].bShortPreamble = WLAN_GET_CAP_INFO_SHORTPREAMBLE(*pwCapInfo);
    pMgmt->sNodeDBTable[0].uRatePollTimeout = FALLBACK_POLL_SECOND;
#ifdef	PLICE_DEBUG
	printk("BSSvUpdateAPNode:MaxSuppRate is %d\n",pMgmt->sNodeDBTable[0].wMaxSuppRate);
#endif
    
    
    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pMgmt->sNodeDBTable[0].wTxDataRate = %d \n", pMgmt->sNodeDBTable[0].wTxDataRate);

};








VOID
BSSvAddMulticastNode(
    IN HANDLE hDeviceContext
    )
{
    PSDevice     pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject    pMgmt = pDevice->pMgmt;

    if (!pDevice->bEnableHostWEP)
        memset(&pMgmt->sNodeDBTable[0], 0, sizeof(KnownNodeDB));
    memset(pMgmt->sNodeDBTable[0].abyMACAddr, 0xff, WLAN_ADDR_LEN);
    pMgmt->sNodeDBTable[0].bActive = TRUE;
    pMgmt->sNodeDBTable[0].bPSEnable = FALSE;
    skb_queue_head_init(&pMgmt->sNodeDBTable[0].sTxPSQueue);
    RATEvParseMaxRate((PVOID) pDevice,
                      (PWLAN_IE_SUPP_RATES)pMgmt->abyCurrSuppRates,
                      (PWLAN_IE_SUPP_RATES)pMgmt->abyCurrExtSuppRates,
                      TRUE,
                      &(pMgmt->sNodeDBTable[0].wMaxBasicRate),
                      &(pMgmt->sNodeDBTable[0].wMaxSuppRate),
                       &(pMgmt->sNodeDBTable[0].wSuppRate),
                      &(pMgmt->sNodeDBTable[0].byTopCCKBasicRate),
                      &(pMgmt->sNodeDBTable[0].byTopOFDMBasicRate)
                     );
    pMgmt->sNodeDBTable[0].wTxDataRate = pMgmt->sNodeDBTable[0].wMaxBasicRate;
#ifdef	PLICE_DEBUG
	printk("BSSvAddMultiCastNode:pMgmt->sNodeDBTable[0].wTxDataRate is %d\n",pMgmt->sNodeDBTable[0].wTxDataRate);
#endif
    pMgmt->sNodeDBTable[0].uRatePollTimeout = FALLBACK_POLL_SECOND;

};






 
 #ifdef FOR_LED_ON_NOTEBOOK
BOOL cc=FALSE;
UINT status;
#endif
VOID
BSSvSecondCallBack(
    IN  HANDLE hDeviceContext
    )
{
    PSDevice        pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    UINT            ii;
    PWLAN_IE_SSID   pItemSSID, pCurrSSID;
    UINT            uSleepySTACnt = 0;
    UINT            uNonShortSlotSTACnt = 0;
    UINT            uLongPreambleSTACnt = 0;
    viawget_wpa_header* wpahdr;  

    spin_lock_irq(&pDevice->lock);

    pDevice->uAssocCount = 0;

    pDevice->byERPFlag &=
        ~(WLAN_SET_ERP_BARKER_MODE(1) | WLAN_SET_ERP_NONERP_PRESENT(1));
 
#ifdef FOR_LED_ON_NOTEBOOK
MACvGPIOIn(pDevice->PortOffset, &pDevice->byGPIO);
if ((( !(pDevice->byGPIO & GPIO0_DATA)&&(pDevice->bHWRadioOff == FALSE))||((pDevice->byGPIO & GPIO0_DATA)&&(pDevice->bHWRadioOff == TRUE)))&&(cc==FALSE)){
cc=TRUE;
}
else if(cc==TRUE){

if(pDevice->bHWRadioOff == TRUE){
            if ( !(pDevice->byGPIO & GPIO0_DATA))

{if(status==1) goto start;
status=1;
CARDbRadioPowerOff(pDevice);
                pMgmt->sNodeDBTable[0].bActive = FALSE;
                pMgmt->eCurrMode = WMAC_MODE_STANDBY;
                pMgmt->eCurrState = WMAC_STATE_IDLE;
                
                pDevice->bLinkPass = FALSE;

}
  if (pDevice->byGPIO &GPIO0_DATA)

{if(status==2) goto start;
status=2;
CARDbRadioPowerOn(pDevice);
} }
else{
            if (pDevice->byGPIO & GPIO0_DATA)

{if(status==3) goto start;
status=3;
CARDbRadioPowerOff(pDevice);
                pMgmt->sNodeDBTable[0].bActive = FALSE;
                pMgmt->eCurrMode = WMAC_MODE_STANDBY;
                pMgmt->eCurrState = WMAC_STATE_IDLE;
                
                pDevice->bLinkPass = FALSE;

}
  if ( !(pDevice->byGPIO & GPIO0_DATA))

{if(status==4) goto start;
status=4;
CARDbRadioPowerOn(pDevice);
} }
}
start:
#endif


    if (pDevice->wUseProtectCntDown > 0) {
        pDevice->wUseProtectCntDown --;
    }
    else {
        
        pDevice->byERPFlag &= ~(WLAN_SET_ERP_USE_PROTECTION(1));
    }

{
       pDevice->byReAssocCount++;
   if((pDevice->byReAssocCount > 10) && (pDevice->bLinkPass != TRUE)) {  
                     printk("Re-association timeout!!!\n");
		   pDevice->byReAssocCount = 0;
                     #ifdef WPA_SUPPLICANT_DRIVER_WEXT_SUPPORT
                    
                        {
                  	union iwreq_data  wrqu;
                  	memset(&wrqu, 0, sizeof (wrqu));
                          wrqu.ap_addr.sa_family = ARPHRD_ETHER;
                  	PRINT_K("wireless_send_event--->SIOCGIWAP(disassociated)\n");
                  	wireless_send_event(pDevice->dev, SIOCGIWAP, &wrqu, NULL);
                       }
                    #endif
     }
   else if(pDevice->bLinkPass == TRUE)
   	pDevice->byReAssocCount = 0;
}

#ifdef Calcu_LinkQual
   s_uCalculateLinkQual((HANDLE)pDevice);
#endif

    for (ii = 0; ii < (MAX_NODE_NUM + 1); ii++) {

        if (pMgmt->sNodeDBTable[ii].bActive) {
            
            pMgmt->sNodeDBTable[ii].uInActiveCount++;

            if (ii > 0) {
                if (pMgmt->sNodeDBTable[ii].uInActiveCount > MAX_INACTIVE_COUNT) {
                    BSSvRemoveOneNode(pDevice, ii);
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO
                        "Inactive timeout [%d] sec, STA index = [%d] remove\n", MAX_INACTIVE_COUNT, ii);
                    continue;
                }

                if (pMgmt->sNodeDBTable[ii].eNodeState >= NODE_ASSOC) {

                    pDevice->uAssocCount++;

                    
                    if (pMgmt->sNodeDBTable[ii].uInActiveCount < ERP_RECOVER_COUNT) {
                        if (!pMgmt->sNodeDBTable[ii].bShortPreamble) {
                            pDevice->byERPFlag |= WLAN_SET_ERP_BARKER_MODE(1);
                            uLongPreambleSTACnt ++;
                        }
                        if (!pMgmt->sNodeDBTable[ii].bERPExist) {
                            pDevice->byERPFlag |= WLAN_SET_ERP_NONERP_PRESENT(1);
                            pDevice->byERPFlag |= WLAN_SET_ERP_USE_PROTECTION(1);
                        }
                        if (!pMgmt->sNodeDBTable[ii].bShortSlotTime)
                            uNonShortSlotSTACnt++;
                    }
                }

                
                if (pMgmt->sNodeDBTable[ii].bPSEnable)
                    uSleepySTACnt++;


            }

            
            if (!pDevice->bFixRate) {

                if (ii > 0) {
                    
                    RATEvTxRateFallBack((PVOID)pDevice, &(pMgmt->sNodeDBTable[ii]));
                }
                else {
                    
                    if (pMgmt->eCurrMode == WMAC_MODE_ESS_STA)
#ifdef	PLICE_DEBUG
		printk("SecondCallback:Before:TxDataRate is %d\n",pMgmt->sNodeDBTable[0].wTxDataRate);
#endif
                        RATEvTxRateFallBack((PVOID)pDevice, &(pMgmt->sNodeDBTable[ii]));
#ifdef	PLICE_DEBUG
		printk("SecondCallback:After:TxDataRate is %d\n",pMgmt->sNodeDBTable[0].wTxDataRate);
#endif

		}

            }

            
            if (pMgmt->sNodeDBTable[ii].wEnQueueCnt != 0) {
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "Index= %d, Queue = %d pending \n",
                           ii, pMgmt->sNodeDBTable[ii].wEnQueueCnt);
                if ((ii >0) && (pMgmt->sNodeDBTable[ii].wEnQueueCnt > 15)) {
                    BSSvRemoveOneNode(pDevice, ii);
                    DBG_PRT(MSG_LEVEL_NOTICE, KERN_INFO "Pending many queues PS STA Index = %d remove \n", ii);
                    continue;
                }
            }
        }

    }


    if ((pMgmt->eCurrMode == WMAC_MODE_ESS_AP) && (pDevice->eCurrentPHYType == PHY_TYPE_11G)) {

        
        if (WLAN_GET_ERP_USE_PROTECTION(pDevice->byERPFlag)) {
            if (!pDevice->bProtectMode) {
                MACvEnableProtectMD(pDevice->PortOffset);
                pDevice->bProtectMode = TRUE;
            }
        }
        else {
            if (pDevice->bProtectMode) {
                MACvDisableProtectMD(pDevice->PortOffset);
                pDevice->bProtectMode = FALSE;
            }
        }
        

        if (uNonShortSlotSTACnt > 0) {
            if (pDevice->bShortSlotTime) {
                pDevice->bShortSlotTime = FALSE;
                BBvSetShortSlotTime(pDevice);
                vUpdateIFS((PVOID)pDevice);
            }
        }
        else {
            if (!pDevice->bShortSlotTime) {
                pDevice->bShortSlotTime = TRUE;
                BBvSetShortSlotTime(pDevice);
                vUpdateIFS((PVOID)pDevice);
            }
        }

        

        if (uLongPreambleSTACnt > 0) {
            if (!pDevice->bBarkerPreambleMd) {
                MACvEnableBarkerPreambleMd(pDevice->PortOffset);
                pDevice->bBarkerPreambleMd = TRUE;
            }
        }
        else {
            if (pDevice->bBarkerPreambleMd) {
                MACvDisableBarkerPreambleMd(pDevice->PortOffset);
                pDevice->bBarkerPreambleMd = FALSE;
            }
        }

    }


    
    if (pMgmt->eCurrMode == WMAC_MODE_ESS_AP) {
        if (uSleepySTACnt > 0)
            pMgmt->sNodeDBTable[0].bPSEnable = TRUE;
        else
            pMgmt->sNodeDBTable[0].bPSEnable = FALSE;
    }

    pItemSSID = (PWLAN_IE_SSID)pMgmt->abyDesireSSID;
    pCurrSSID = (PWLAN_IE_SSID)pMgmt->abyCurrSSID;

    if ((pMgmt->eCurrMode == WMAC_MODE_STANDBY) ||
        (pMgmt->eCurrMode == WMAC_MODE_ESS_STA)) {

        if (pMgmt->sNodeDBTable[0].bActive) { 
           
            
            
            

            if (pDevice->bUpdateBBVGA) {
               
               s_vCheckPreEDThreshold((HANDLE)pDevice);
            }

    	    if ((pMgmt->sNodeDBTable[0].uInActiveCount >= (LOST_BEACON_COUNT/2)) &&
    	        (pDevice->byBBVGACurrent != pDevice->abyBBVGA[0]) ) {
    	        pDevice->byBBVGANew = pDevice->abyBBVGA[0];
                bScheduleCommand((HANDLE) pDevice, WLAN_CMD_CHANGE_BBSENSITIVITY, NULL);
    	    }

        	if (pMgmt->sNodeDBTable[0].uInActiveCount >= LOST_BEACON_COUNT) {
                pMgmt->sNodeDBTable[0].bActive = FALSE;
                pMgmt->eCurrMode = WMAC_MODE_STANDBY;
                pMgmt->eCurrState = WMAC_STATE_IDLE;
                netif_stop_queue(pDevice->dev);
                pDevice->bLinkPass = FALSE;
                pDevice->bRoaming = TRUE;
                DBG_PRT(MSG_LEVEL_NOTICE, KERN_INFO "Lost AP beacon [%d] sec, disconnected !\n", pMgmt->sNodeDBTable[0].uInActiveCount);
        if ((pDevice->bWPADEVUp) && (pDevice->skb != NULL)) {
             wpahdr = (viawget_wpa_header *)pDevice->skb->data;
             wpahdr->type = VIAWGET_DISASSOC_MSG;
             wpahdr->resp_ie_len = 0;
             wpahdr->req_ie_len = 0;
             skb_put(pDevice->skb, sizeof(viawget_wpa_header));
             pDevice->skb->dev = pDevice->wpadev;
	     skb_reset_mac_header(pDevice->skb);
             pDevice->skb->pkt_type = PACKET_HOST;
             pDevice->skb->protocol = htons(ETH_P_802_2);
             memset(pDevice->skb->cb, 0, sizeof(pDevice->skb->cb));
             netif_rx(pDevice->skb);
             pDevice->skb = dev_alloc_skb((int)pDevice->rx_buf_sz);
         };
   #ifdef WPA_SUPPLICANT_DRIVER_WEXT_SUPPORT
  
      {
	union iwreq_data  wrqu;
	memset(&wrqu, 0, sizeof (wrqu));
        wrqu.ap_addr.sa_family = ARPHRD_ETHER;
	PRINT_K("wireless_send_event--->SIOCGIWAP(disassociated)\n");
	wireless_send_event(pDevice->dev, SIOCGIWAP, &wrqu, NULL);
     }
  #endif
	    }
        }
        else if (pItemSSID->len != 0) {
            if (pDevice->uAutoReConnectTime < 10) {
                pDevice->uAutoReConnectTime++;
	       #ifdef WPA_SUPPLICANT_DRIVER_WEXT_SUPPORT
                
                if(pDevice->bWPASuppWextEnabled ==TRUE)
		 pDevice->uAutoReConnectTime = 0;
	     #endif
            }
            else {
	   
	      if(pDevice->bWPADEVUp)
	          pDevice->eEncryptionStatus = pDevice->eOldEncryptionStatus;

                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "Roaming ...\n");
                BSSvClearBSSList((HANDLE)pDevice, pDevice->bLinkPass);
 	      pMgmt->eScanType = WMAC_SCAN_ACTIVE;
                bScheduleCommand((HANDLE) pDevice, WLAN_CMD_BSSID_SCAN, pMgmt->abyDesireSSID);
                bScheduleCommand((HANDLE) pDevice, WLAN_CMD_SSID, pMgmt->abyDesireSSID);
                pDevice->uAutoReConnectTime = 0;
            }
        }
    }

    if (pMgmt->eCurrMode == WMAC_MODE_IBSS_STA) {
        
        if ((pMgmt->eCurrState == WMAC_STATE_STARTED) && (pCurrSSID->len == 0)) {
            if (pDevice->uAutoReConnectTime < 10) {
                pDevice->uAutoReConnectTime++;
            }
            else {
                DBG_PRT(MSG_LEVEL_NOTICE, KERN_INFO "Adhoc re-scaning ...\n");
	      pMgmt->eScanType = WMAC_SCAN_ACTIVE;
                bScheduleCommand((HANDLE) pDevice, WLAN_CMD_BSSID_SCAN, NULL);
                bScheduleCommand((HANDLE) pDevice, WLAN_CMD_SSID, NULL);
                pDevice->uAutoReConnectTime = 0;
            };
        }
        if (pMgmt->eCurrState == WMAC_STATE_JOINTED) {

            if (pDevice->bUpdateBBVGA) {
               
               s_vCheckPreEDThreshold((HANDLE)pDevice);
            }
        	if (pMgmt->sNodeDBTable[0].uInActiveCount >=ADHOC_LOST_BEACON_COUNT) {
        	    DBG_PRT(MSG_LEVEL_NOTICE, KERN_INFO "Lost other STA beacon [%d] sec, started !\n", pMgmt->sNodeDBTable[0].uInActiveCount);
                pMgmt->sNodeDBTable[0].uInActiveCount = 0;
                pMgmt->eCurrState = WMAC_STATE_STARTED;
                netif_stop_queue(pDevice->dev);
                pDevice->bLinkPass = FALSE;
            }
        }
    }

    spin_unlock_irq(&pDevice->lock);

    pMgmt->sTimerSecondCallback.expires = RUN_AT(HZ);
    add_timer(&pMgmt->sTimerSecondCallback);
    return;
}








VOID
BSSvUpdateNodeTxCounter(
    IN HANDLE      hDeviceContext,
    IN BYTE        byTsr0,
    IN BYTE        byTsr1,
    IN PBYTE       pbyBuffer,
    IN UINT        uFIFOHeaderSize
    )
{
    PSDevice        pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    UINT            uNodeIndex = 0;
    BYTE            byTxRetry = (byTsr0 & TSR0_NCR);
    PSTxBufHead     pTxBufHead;
    PS802_11Header  pMACHeader;
    WORD            wRate;
    WORD            wFallBackRate = RATE_1M;
    BYTE            byFallBack;
    UINT            ii;


	
	
	

    pTxBufHead = (PSTxBufHead) pbyBuffer;
    if (pTxBufHead->wFIFOCtl & FIFOCTL_AUTO_FB_0) {
        byFallBack = AUTO_FB_0;
    } else if (pTxBufHead->wFIFOCtl & FIFOCTL_AUTO_FB_1) {
        byFallBack = AUTO_FB_1;
    } else {
        byFallBack = AUTO_FB_NONE;
    }
    wRate = pTxBufHead->wReserved; 
    



	

    
    if (pTxBufHead->wFIFOCtl & FIFOCTL_NEEDACK) {
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"wRate %04X, byTsr0 %02X, byTsr1 %02X\n", wRate, byTsr0, byTsr1);
        if (pMgmt->eCurrMode == WMAC_MODE_ESS_STA) {
            pMgmt->sNodeDBTable[0].uTxAttempts += 1;
            if ((byTsr1 & TSR1_TERR) == 0) {
                
                pMgmt->sNodeDBTable[0].uTxOk[MAX_RATE]++;
                if ( (byFallBack == AUTO_FB_NONE) ||
                     (wRate < RATE_18M) ) {
                    wFallBackRate = wRate;
                } else if (byFallBack == AUTO_FB_0) {

				  if (byTxRetry < 5)
				
					wFallBackRate = awHWRetry0[wRate-RATE_18M][byTxRetry];
			
			
		else
                        wFallBackRate = awHWRetry0[wRate-RATE_18M][4];
			
		} else if (byFallBack == AUTO_FB_1) {
                    if (byTxRetry < 5)
                        wFallBackRate = awHWRetry1[wRate-RATE_18M][byTxRetry];
                    else
                        wFallBackRate = awHWRetry1[wRate-RATE_18M][4];
                }
                pMgmt->sNodeDBTable[0].uTxOk[wFallBackRate]++;
            } else {
                pMgmt->sNodeDBTable[0].uTxFailures ++;
            }
            pMgmt->sNodeDBTable[0].uTxRetry += byTxRetry;
            if (byTxRetry != 0) {
                pMgmt->sNodeDBTable[0].uTxFail[MAX_RATE]+=byTxRetry;
                if ( (byFallBack == AUTO_FB_NONE) ||
                     (wRate < RATE_18M) ) {
                    pMgmt->sNodeDBTable[0].uTxFail[wRate]+=byTxRetry;
                } else if (byFallBack == AUTO_FB_0) {

				   for(ii=0;ii<byTxRetry;ii++)
		
		{
                        if (ii < 5)
                        	{


						wFallBackRate = awHWRetry0[wRate-RATE_18M][ii];
					
				
                        	}
			else
				{
			wFallBackRate = awHWRetry0[wRate-RATE_18M][4];
			
				
				}
						pMgmt->sNodeDBTable[0].uTxFail[wFallBackRate]++;
                    }
                } else if (byFallBack == AUTO_FB_1) {
                    for(ii=0;ii<byTxRetry;ii++) {
                        if (ii < 5)
                            wFallBackRate = awHWRetry1[wRate-RATE_18M][ii];
                        else
                            wFallBackRate = awHWRetry1[wRate-RATE_18M][4];
                        pMgmt->sNodeDBTable[0].uTxFail[wFallBackRate]++;
                    }
                }
            }
        };

        if ((pMgmt->eCurrMode == WMAC_MODE_IBSS_STA) ||
            (pMgmt->eCurrMode == WMAC_MODE_ESS_AP)) {

            pMACHeader = (PS802_11Header)(pbyBuffer + uFIFOHeaderSize);

            if (BSSDBbIsSTAInNodeDB((HANDLE)pMgmt, &(pMACHeader->abyAddr1[0]), &uNodeIndex)){
                pMgmt->sNodeDBTable[uNodeIndex].uTxAttempts += 1;
                if ((byTsr1 & TSR1_TERR) == 0) {
                    
                    pMgmt->sNodeDBTable[uNodeIndex].uTxOk[MAX_RATE]++;
                    if ( (byFallBack == AUTO_FB_NONE) ||
                         (wRate < RATE_18M) ) {
                        wFallBackRate = wRate;
                    } else if (byFallBack == AUTO_FB_0) {
                        if (byTxRetry < 5)
                            wFallBackRate = awHWRetry0[wRate-RATE_18M][byTxRetry];
                        else
                            wFallBackRate = awHWRetry0[wRate-RATE_18M][4];
                    } else if (byFallBack == AUTO_FB_1) {
                        if (byTxRetry < 5)
                            wFallBackRate = awHWRetry1[wRate-RATE_18M][byTxRetry];
                        else
                            wFallBackRate = awHWRetry1[wRate-RATE_18M][4];
                    }
                    pMgmt->sNodeDBTable[uNodeIndex].uTxOk[wFallBackRate]++;
                } else {
                    pMgmt->sNodeDBTable[uNodeIndex].uTxFailures ++;
                }
                pMgmt->sNodeDBTable[uNodeIndex].uTxRetry += byTxRetry;
                if (byTxRetry != 0) {
                    pMgmt->sNodeDBTable[uNodeIndex].uTxFail[MAX_RATE]+=byTxRetry;
                    if ( (byFallBack == AUTO_FB_NONE) ||
                         (wRate < RATE_18M) ) {
                        pMgmt->sNodeDBTable[uNodeIndex].uTxFail[wRate]+=byTxRetry;
                    } else if (byFallBack == AUTO_FB_0) {
                        for(ii=0;ii<byTxRetry;ii++) {
                            if (ii < 5)
                                wFallBackRate = awHWRetry0[wRate-RATE_18M][ii];
                            else
                                wFallBackRate = awHWRetry0[wRate-RATE_18M][4];
                            pMgmt->sNodeDBTable[uNodeIndex].uTxFail[wFallBackRate]++;
                        }
                    } else if (byFallBack == AUTO_FB_1) {
                        for(ii=0;ii<byTxRetry;ii++) {
                            if (ii < 5)
                                wFallBackRate = awHWRetry1[wRate-RATE_18M][ii];
                            else
                                wFallBackRate = awHWRetry1[wRate-RATE_18M][4];
                            pMgmt->sNodeDBTable[uNodeIndex].uTxFail[wFallBackRate]++;
                        }
                    }
                }
            };
        }
    };

    return;


}







VOID
BSSvClearNodeDBTable(
    IN HANDLE hDeviceContext,
    IN UINT uStartIndex
    )

{
    PSDevice     pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    struct sk_buff  *skb;
    UINT            ii;

    for (ii = uStartIndex; ii < (MAX_NODE_NUM + 1); ii++) {
        if (pMgmt->sNodeDBTable[ii].bActive) {
            
            if (pMgmt->sNodeDBTable[ii].sTxPSQueue.next != NULL) {
                while ((skb = skb_dequeue(&pMgmt->sNodeDBTable[ii].sTxPSQueue)) != NULL){
                        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "PS skb != NULL %d\n", ii);
                        dev_kfree_skb(skb);
                }
            }
            memset(&pMgmt->sNodeDBTable[ii], 0, sizeof(KnownNodeDB));
        }
    }

    return;
};


VOID s_vCheckSensitivity(
    IN HANDLE hDeviceContext
    )
{
    PSDevice        pDevice = (PSDevice)hDeviceContext;
    PKnownBSS       pBSSList = NULL;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    int             ii;

    if ((pDevice->byLocalID <= REV_ID_VT3253_A1) && (pDevice->byRFType == RF_RFMD2959) &&
        (pMgmt->eCurrMode == WMAC_MODE_IBSS_STA)) {
        return;
    }

    if ((pMgmt->eCurrState == WMAC_STATE_ASSOC) ||
        ((pMgmt->eCurrMode == WMAC_MODE_IBSS_STA) && (pMgmt->eCurrState == WMAC_STATE_JOINTED))) {
        pBSSList = BSSpAddrIsInBSSList(pDevice, pMgmt->abyCurrBSSID, (PWLAN_IE_SSID)pMgmt->abyCurrSSID);
        if (pBSSList != NULL) {
            
            LONG    LocalldBmAverage = 0;
            LONG    uNumofdBm = 0;
            for (ii = 0; ii < RSSI_STAT_COUNT; ii++) {
                if (pBSSList->ldBmAverage[ii] != 0) {
                    uNumofdBm ++;
                    LocalldBmAverage += pBSSList->ldBmAverage[ii];
                }
            }
            if (uNumofdBm > 0) {
                LocalldBmAverage = LocalldBmAverage/uNumofdBm;
                for (ii=0;ii<BB_VGA_LEVEL;ii++) {
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"LocalldBmAverage:%ld, %ld %02x\n", LocalldBmAverage, pDevice->ldBmThreshold[ii], pDevice->abyBBVGA[ii]);
                    if (LocalldBmAverage < pDevice->ldBmThreshold[ii]) {
                	    pDevice->byBBVGANew = pDevice->abyBBVGA[ii];
                        break;
                    }
                }
                if (pDevice->byBBVGANew != pDevice->byBBVGACurrent) {
                    pDevice->uBBVGADiffCount++;
                    if (pDevice->uBBVGADiffCount >= BB_VGA_CHANGE_THRESHOLD)
                        bScheduleCommand((HANDLE) pDevice, WLAN_CMD_CHANGE_BBSENSITIVITY, NULL);
                } else {
                    pDevice->uBBVGADiffCount = 0;
                }
            }
        }
    }
}


VOID
BSSvClearAnyBSSJoinRecord (
    IN HANDLE hDeviceContext
    )
{
    PSDevice        pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    UINT            ii;

    for (ii = 0; ii < MAX_BSS_NUM; ii++) {
        pMgmt->sBSSList[ii].bSelected = FALSE;
    }
    return;
}

#ifdef Calcu_LinkQual
VOID s_uCalculateLinkQual(
    IN HANDLE hDeviceContext
    )
{
   PSDevice        pDevice = (PSDevice)hDeviceContext;
   ULONG TxOkRatio, TxCnt;
   ULONG RxOkRatio,RxCnt;
   ULONG RssiRatio;
   long ldBm;

TxCnt = pDevice->scStatistic.TxNoRetryOkCount +
	      pDevice->scStatistic.TxRetryOkCount +
	      pDevice->scStatistic.TxFailCount;
RxCnt = pDevice->scStatistic.RxFcsErrCnt +
	      pDevice->scStatistic.RxOkCnt;
TxOkRatio = (TxCnt < 6) ? 4000:((pDevice->scStatistic.TxNoRetryOkCount * 4000) / TxCnt);
RxOkRatio = (RxCnt < 6) ? 2000:((pDevice->scStatistic.RxOkCnt * 2000) / RxCnt);

if(pDevice->bLinkPass !=TRUE)
{
 
   pDevice->scStatistic.LinkQuality = 0;
   pDevice->scStatistic.SignalStren = 0;
}
else
{
   RFvRSSITodBm(pDevice, (BYTE)(pDevice->uCurrRSSI), &ldBm);
   if(-ldBm < 50)  {
   	RssiRatio = 4000;
     }
   else if(-ldBm > 90) {
   	RssiRatio = 0;
     }
   else {
   	RssiRatio = (40-(-ldBm-50))*4000/40;
     }
   pDevice->scStatistic.SignalStren = RssiRatio/40;
   pDevice->scStatistic.LinkQuality = (RssiRatio+TxOkRatio+RxOkRatio)/100;
}
   pDevice->scStatistic.RxFcsErrCnt = 0;
   pDevice->scStatistic.RxOkCnt = 0;
   pDevice->scStatistic.TxFailCount = 0;
   pDevice->scStatistic.TxNoRetryOkCount = 0;
   pDevice->scStatistic.TxRetryOkCount = 0;
   return;
}
#endif

VOID s_vCheckPreEDThreshold(
    IN HANDLE hDeviceContext
    )
{
    PSDevice        pDevice = (PSDevice)hDeviceContext;
    PKnownBSS       pBSSList = NULL;
    PSMgmtObject    pMgmt = &(pDevice->sMgmtObj);

    if ((pMgmt->eCurrState == WMAC_STATE_ASSOC) ||
        ((pMgmt->eCurrMode == WMAC_MODE_IBSS_STA) && (pMgmt->eCurrState == WMAC_STATE_JOINTED))) {
        pBSSList = BSSpAddrIsInBSSList(pDevice, pMgmt->abyCurrBSSID, (PWLAN_IE_SSID)pMgmt->abyCurrSSID);
        if (pBSSList != NULL) {
            pDevice->byBBPreEDRSSI = (BYTE) (~(pBSSList->ldBmAverRange) + 1);
            
        }
    }
    return;
}

