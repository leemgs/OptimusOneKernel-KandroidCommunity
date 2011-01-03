

#include "ttype.h"
#include "mac.h"
#include "device.h"
#include "wmgr.h"
#include "power.h"
#include "wcmd.h"
#include "rxtx.h"
#include "card.h"









static int          msglevel                =MSG_LEVEL_INFO;











VOID
PSvEnablePowerSaving(
    IN HANDLE hDeviceContext,
    IN WORD wListenInterval
    )
{
    PSDevice        pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    WORD            wAID = pMgmt->wCurrAID | BIT14 | BIT15;

    
    VNSvOutPortW(pDevice->PortOffset + MAC_REG_PWBT, C_PWBT);
    if (pDevice->eOPMode != OP_MODE_ADHOC) {
        
        VNSvOutPortW(pDevice->PortOffset + MAC_REG_AIDATIM, wAID);
    } else {
    	
        MACvWriteATIMW(pDevice->PortOffset, pMgmt->wCurrATIMWindow);
    }
    
    MACvRegBitsOn(pDevice->PortOffset, MAC_REG_PSCFG, PSCFG_AUTOSLEEP);
    
    MACvRegBitsOn(pDevice->PortOffset, MAC_REG_TFTCTL, TFTCTL_HWUTSF);

    if (wListenInterval >= 2) {
        
        MACvRegBitsOff(pDevice->PortOffset, MAC_REG_PSCTL, PSCTL_ALBCN);
        
        
        MACvRegBitsOn(pDevice->PortOffset, MAC_REG_PSCTL, PSCTL_LNBCN);
        pMgmt->wCountToWakeUp = wListenInterval;
    }
    else {
        
        MACvRegBitsOn(pDevice->PortOffset, MAC_REG_PSCTL, PSCTL_ALBCN);
        
        pMgmt->wCountToWakeUp = 0;
    }

    
    MACvRegBitsOn(pDevice->PortOffset, MAC_REG_PSCTL, PSCTL_PSEN);
    pDevice->bEnablePSMode = TRUE;

    if (pDevice->eOPMode == OP_MODE_ADHOC) {

    }
    
    else if (pDevice->eOPMode == OP_MODE_INFRASTRUCTURE) {
        PSbSendNullPacket(pDevice);
    }
    pDevice->bPWBitOn = TRUE;
    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "PS:Power Saving Mode Enable... \n");
    return;
}








VOID
PSvDisablePowerSaving(
    IN HANDLE hDeviceContext
    )
{
    PSDevice        pDevice = (PSDevice)hDeviceContext;


    
    MACbPSWakeup(pDevice->PortOffset);
    
    MACvRegBitsOff(pDevice->PortOffset, MAC_REG_PSCFG, PSCFG_AUTOSLEEP);
    
    MACvRegBitsOff(pDevice->PortOffset, MAC_REG_TFTCTL, TFTCTL_HWUTSF);
    
    MACvRegBitsOn(pDevice->PortOffset, MAC_REG_PSCTL, PSCTL_ALBCN);

    pDevice->bEnablePSMode = FALSE;

    if (pDevice->eOPMode == OP_MODE_INFRASTRUCTURE) {
        PSbSendNullPacket(pDevice);
    }
    pDevice->bPWBitOn = FALSE;
    return;
}





BOOL
PSbConsiderPowerDown(
    IN HANDLE hDeviceContext,
    IN BOOL bCheckRxDMA,
    IN BOOL bCheckCountToWakeUp
    )
{
    PSDevice        pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    UINT            uIdx;

    
    if (MACbIsRegBitsOn(pDevice->PortOffset, MAC_REG_PSCTL, PSCTL_PS))
        return TRUE;

    if (pMgmt->eCurrMode != WMAC_MODE_IBSS_STA) {
        
        if (pMgmt->bInTIMWake)
            return FALSE;
    }

    
    if (pDevice->bCmdRunning)
        return FALSE;

    
    MACvRegBitsOn(pDevice->PortOffset, MAC_REG_PSCTL, PSCTL_PSEN);

    
    for (uIdx = 0; uIdx < TYPE_MAXTD; uIdx ++) {
        if (pDevice->iTDUsed[uIdx] != 0)
            return FALSE;
    }

    
    if (bCheckRxDMA &&
        ((pDevice->dwIsr& ISR_RXDMA0) != 0) &&
        ((pDevice->dwIsr & ISR_RXDMA1) != 0)){
        return FALSE;
    };

    if (pMgmt->eCurrMode != WMAC_MODE_IBSS_STA) {
        if (bCheckCountToWakeUp &&
           (pMgmt->wCountToWakeUp == 0 || pMgmt->wCountToWakeUp == 1)) {
             return FALSE;
        }
    }

    
    MACvRegBitsOn(pDevice->PortOffset, MAC_REG_PSCTL, PSCTL_GO2DOZE);
    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "Go to Doze ZZZZZZZZZZZZZZZ\n");
    return TRUE;
}







VOID
PSvSendPSPOLL(
    IN HANDLE hDeviceContext
    )
{
    PSDevice            pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject        pMgmt = pDevice->pMgmt;
    PSTxMgmtPacket      pTxPacket = NULL;


    memset(pMgmt->pbyPSPacketPool, 0, sizeof(STxMgmtPacket) + WLAN_HDR_ADDR2_LEN);
    pTxPacket = (PSTxMgmtPacket)pMgmt->pbyPSPacketPool;
    pTxPacket->p80211Header = (PUWLAN_80211HDR)((PBYTE)pTxPacket + sizeof(STxMgmtPacket));
    pTxPacket->p80211Header->sA2.wFrameCtl = cpu_to_le16(
         (
         WLAN_SET_FC_FTYPE(WLAN_TYPE_CTL) |
         WLAN_SET_FC_FSTYPE(WLAN_FSTYPE_PSPOLL) |
         WLAN_SET_FC_PWRMGT(0)
         ));
    pTxPacket->p80211Header->sA2.wDurationID = pMgmt->wCurrAID | BIT14 | BIT15;
    memcpy(pTxPacket->p80211Header->sA2.abyAddr1, pMgmt->abyCurrBSSID, WLAN_ADDR_LEN);
    memcpy(pTxPacket->p80211Header->sA2.abyAddr2, pMgmt->abyMACAddr, WLAN_ADDR_LEN);
    pTxPacket->cbMPDULen = WLAN_HDR_ADDR2_LEN;
    pTxPacket->cbPayloadLen = 0;
    
    if (csMgmt_xmit(pDevice, pTxPacket) != CMD_STATUS_PENDING) {
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "Send PS-Poll packet failed..\n");
    }
    else {

    };

    return;
}




BOOL
PSbSendNullPacket(
    IN HANDLE hDeviceContext
    )
{
    PSDevice            pDevice = (PSDevice)hDeviceContext;
    PSTxMgmtPacket      pTxPacket = NULL;
    PSMgmtObject        pMgmt = pDevice->pMgmt;
    UINT                uIdx;


    if (pDevice->bLinkPass == FALSE) {
        return FALSE;
    }
    #ifdef TxInSleep
     if ((pDevice->bEnablePSMode == FALSE) &&
	  (pDevice->fTxDataInSleep == FALSE)){
        return FALSE;
    }
#else
    if (pDevice->bEnablePSMode == FALSE) {
        return FALSE;
    }
#endif
    if (pDevice->bEnablePSMode) {
        for (uIdx = 0; uIdx < TYPE_MAXTD; uIdx ++) {
            if (pDevice->iTDUsed[uIdx] != 0)
                return FALSE;
        }
    }

    memset(pMgmt->pbyPSPacketPool, 0, sizeof(STxMgmtPacket) + WLAN_NULLDATA_FR_MAXLEN);
    pTxPacket = (PSTxMgmtPacket)pMgmt->pbyPSPacketPool;
    pTxPacket->p80211Header = (PUWLAN_80211HDR)((PBYTE)pTxPacket + sizeof(STxMgmtPacket));

    if (pDevice->bEnablePSMode) {

        pTxPacket->p80211Header->sA3.wFrameCtl = cpu_to_le16(
             (
            WLAN_SET_FC_FTYPE(WLAN_TYPE_DATA) |
            WLAN_SET_FC_FSTYPE(WLAN_FSTYPE_NULL) |
            WLAN_SET_FC_PWRMGT(1)
            ));
    }
    else {
        pTxPacket->p80211Header->sA3.wFrameCtl = cpu_to_le16(
             (
            WLAN_SET_FC_FTYPE(WLAN_TYPE_DATA) |
            WLAN_SET_FC_FSTYPE(WLAN_FSTYPE_NULL) |
            WLAN_SET_FC_PWRMGT(0)
            ));
    }

    if(pMgmt->eCurrMode != WMAC_MODE_IBSS_STA) {
        pTxPacket->p80211Header->sA3.wFrameCtl |= cpu_to_le16((WORD)WLAN_SET_FC_TODS(1));
    }

    memcpy(pTxPacket->p80211Header->sA3.abyAddr1, pMgmt->abyCurrBSSID, WLAN_ADDR_LEN);
    memcpy(pTxPacket->p80211Header->sA3.abyAddr2, pMgmt->abyMACAddr, WLAN_ADDR_LEN);
    memcpy(pTxPacket->p80211Header->sA3.abyAddr3, pMgmt->abyCurrBSSID, WLAN_BSSID_LEN);
    pTxPacket->cbMPDULen = WLAN_HDR_ADDR3_LEN;
    pTxPacket->cbPayloadLen = 0;
    
    if (csMgmt_xmit(pDevice, pTxPacket) != CMD_STATUS_PENDING) {
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "Send Null Packet failed !\n");
        return FALSE;
    }
    else {


    }


    return TRUE ;
}



BOOL
PSbIsNextTBTTWakeUp(
    IN HANDLE hDeviceContext
    )
{

    PSDevice         pDevice = (PSDevice)hDeviceContext;
    PSMgmtObject        pMgmt = pDevice->pMgmt;
    BOOL                bWakeUp = FALSE;

    if (pMgmt->wListenInterval >= 2) {
        if (pMgmt->wCountToWakeUp == 0) {
            pMgmt->wCountToWakeUp = pMgmt->wListenInterval;
        }

        pMgmt->wCountToWakeUp --;

        if (pMgmt->wCountToWakeUp == 1) {
            
            MACvRegBitsOn(pDevice->PortOffset, MAC_REG_PSCTL, PSCTL_LNBCN);
            bWakeUp = TRUE;
        }

    }

    return bWakeUp;
}

