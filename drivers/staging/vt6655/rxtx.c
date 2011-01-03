

#include "device.h"
#include "rxtx.h"
#include "tether.h"
#include "card.h"
#include "bssdb.h"
#include "mac.h"
#include "baseband.h"
#include "michael.h"
#include "tkip.h"
#include "tcrc.h"
#include "wctl.h"
#include "wroute.h"
#include "hostap.h"
#include "rf.h"







static int          msglevel                =MSG_LEVEL_INFO;

#define	PLICE_DEBUG





#define CRITICAL_PACKET_LEN      256    
                                        

const WORD wTimeStampOff[2][MAX_RATE] = {
        {384, 288, 226, 209, 54, 43, 37, 31, 28, 25, 24, 23}, 
        {384, 192, 130, 113, 54, 43, 37, 31, 28, 25, 24, 23}, 
    };

const WORD wFB_Opt0[2][5] = {
        {RATE_12M, RATE_18M, RATE_24M, RATE_36M, RATE_48M}, 
        {RATE_12M, RATE_12M, RATE_18M, RATE_24M, RATE_36M}, 
    };
const WORD wFB_Opt1[2][5] = {
        {RATE_12M, RATE_18M, RATE_24M, RATE_24M, RATE_36M}, 
        {RATE_6M , RATE_6M,  RATE_12M, RATE_12M, RATE_18M}, 
    };


#define RTSDUR_BB       0
#define RTSDUR_BA       1
#define RTSDUR_AA       2
#define CTSDUR_BA       3
#define RTSDUR_BA_F0    4
#define RTSDUR_AA_F0    5
#define RTSDUR_BA_F1    6
#define RTSDUR_AA_F1    7
#define CTSDUR_BA_F0    8
#define CTSDUR_BA_F1    9
#define DATADUR_B       10
#define DATADUR_A       11
#define DATADUR_A_F0    12
#define DATADUR_A_F1    13





static
VOID
s_vFillTxKey(
    IN  PSDevice   pDevice,
    IN  PBYTE      pbyBuf,
    IN  PBYTE      pbyIVHead,
    IN  PSKeyItem  pTransmitKey,
    IN  PBYTE      pbyHdrBuf,
    IN  WORD       wPayloadLen,
    OUT PBYTE      pMICHDR
    );



static
VOID
s_vFillRTSHead(
    IN PSDevice         pDevice,
    IN BYTE             byPktType,
    IN PVOID            pvRTS,
    IN UINT             cbFrameLength,
    IN BOOL             bNeedAck,
    IN BOOL             bDisCRC,
    IN PSEthernetHeader psEthHeader,
    IN WORD             wCurrentRate,
    IN BYTE             byFBOption
    );

static
VOID
s_vGenerateTxParameter(
    IN PSDevice         pDevice,
    IN  BYTE            byPktType,
    IN PVOID            pTxBufHead,
    IN PVOID            pvRrvTime,
    IN PVOID            pvRTS,
    IN PVOID            pvCTS,
    IN UINT             cbFrameSize,
    IN BOOL             bNeedACK,
    IN UINT             uDMAIdx,
    IN PSEthernetHeader psEthHeader,
    IN WORD             wCurrentRate
    );



static void s_vFillFragParameter(
    IN PSDevice pDevice,
    IN PBYTE    pbyBuffer,
    IN UINT     uTxType,
    IN PVOID    pvtdCurr,
    IN WORD     wFragType,
    IN UINT     cbReqCount
    );


static
UINT
s_cbFillTxBufHead (
    IN  PSDevice         pDevice,
    IN  BYTE             byPktType,
    IN  PBYTE            pbyTxBufferAddr,
    IN  UINT             cbFrameBodySize,
    IN  UINT             uDMAIdx,
    IN  PSTxDesc         pHeadTD,
    IN  PSEthernetHeader psEthHeader,
    IN  PBYTE            pPacket,
    IN  BOOL             bNeedEncrypt,
    IN  PSKeyItem        pTransmitKey,
    IN  UINT             uNodeIndex,
    OUT PUINT            puMACfragNum
    );


static
UINT
s_uFillDataHead (
    IN PSDevice pDevice,
    IN BYTE     byPktType,
    IN PVOID    pTxDataHead,
    IN UINT     cbFrameLength,
    IN UINT     uDMAIdx,
    IN BOOL     bNeedAck,
    IN UINT     uFragIdx,
    IN UINT     cbLastFragmentSize,
    IN UINT     uMACfragNum,
    IN BYTE     byFBOption,
    IN WORD     wCurrentRate
    );






static
VOID
s_vFillTxKey (
    IN  PSDevice   pDevice,
    IN  PBYTE      pbyBuf,
    IN  PBYTE      pbyIVHead,
    IN  PSKeyItem  pTransmitKey,
    IN  PBYTE      pbyHdrBuf,
    IN  WORD       wPayloadLen,
    OUT PBYTE      pMICHDR
    )
{
    PDWORD          pdwIV = (PDWORD) pbyIVHead;
    PDWORD          pdwExtIV = (PDWORD) ((PBYTE)pbyIVHead+4);
    WORD            wValue;
    PS802_11Header  pMACHeader = (PS802_11Header)pbyHdrBuf;
    DWORD           dwRevIVCounter;
    BYTE            byKeyIndex = 0;



    
    if (pTransmitKey == NULL)
        return;

    dwRevIVCounter = cpu_to_le32(pDevice->dwIVCounter);
    *pdwIV = pDevice->dwIVCounter;
    byKeyIndex = pTransmitKey->dwKeyIndex & 0xf;

    if (pTransmitKey->byCipherSuite == KEY_CTL_WEP) {
        if (pTransmitKey->uKeyLength == WLAN_WEP232_KEYLEN ){
            memcpy(pDevice->abyPRNG, (PBYTE)&(dwRevIVCounter), 3);
            memcpy(pDevice->abyPRNG+3, pTransmitKey->abyKey, pTransmitKey->uKeyLength);
        } else {
            memcpy(pbyBuf, (PBYTE)&(dwRevIVCounter), 3);
            memcpy(pbyBuf+3, pTransmitKey->abyKey, pTransmitKey->uKeyLength);
            if(pTransmitKey->uKeyLength == WLAN_WEP40_KEYLEN) {
                memcpy(pbyBuf+8, (PBYTE)&(dwRevIVCounter), 3);
                memcpy(pbyBuf+11, pTransmitKey->abyKey, pTransmitKey->uKeyLength);
            }
            memcpy(pDevice->abyPRNG, pbyBuf, 16);
        }
        
        *pdwIV &= WEP_IV_MASK;
        *pdwIV |= (byKeyIndex << 30);
        *pdwIV = cpu_to_le32(*pdwIV);
        pDevice->dwIVCounter++;
        if (pDevice->dwIVCounter > WEP_IV_MASK) {
            pDevice->dwIVCounter = 0;
        }
    } else if (pTransmitKey->byCipherSuite == KEY_CTL_TKIP) {
        pTransmitKey->wTSC15_0++;
        if (pTransmitKey->wTSC15_0 == 0) {
            pTransmitKey->dwTSC47_16++;
        }
        TKIPvMixKey(pTransmitKey->abyKey, pDevice->abyCurrentNetAddr,
                    pTransmitKey->wTSC15_0, pTransmitKey->dwTSC47_16, pDevice->abyPRNG);
        memcpy(pbyBuf, pDevice->abyPRNG, 16);
        
        memcpy(pdwIV, pDevice->abyPRNG, 3);

        *(pbyIVHead+3) = (BYTE)(((byKeyIndex << 6) & 0xc0) | 0x20); 
        
        *pdwExtIV = cpu_to_le32(pTransmitKey->dwTSC47_16);
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"vFillTxKey()---- pdwExtIV: %lx\n", *pdwExtIV);

    } else if (pTransmitKey->byCipherSuite == KEY_CTL_CCMP) {
        pTransmitKey->wTSC15_0++;
        if (pTransmitKey->wTSC15_0 == 0) {
            pTransmitKey->dwTSC47_16++;
        }
        memcpy(pbyBuf, pTransmitKey->abyKey, 16);

        
        *pdwIV = 0;
        *(pbyIVHead+3) = (BYTE)(((byKeyIndex << 6) & 0xc0) | 0x20); 
        *pdwIV |= cpu_to_le16((WORD)(pTransmitKey->wTSC15_0));
        
        *pdwExtIV = cpu_to_le32(pTransmitKey->dwTSC47_16);

        
        *pMICHDR = 0x59;
        *((PBYTE)(pMICHDR+1)) = 0; 
        memcpy(pMICHDR+2, &(pMACHeader->abyAddr2[0]), 6);
        *((PBYTE)(pMICHDR+8)) = HIBYTE(HIWORD(pTransmitKey->dwTSC47_16));
        *((PBYTE)(pMICHDR+9)) = LOBYTE(HIWORD(pTransmitKey->dwTSC47_16));
        *((PBYTE)(pMICHDR+10)) = HIBYTE(LOWORD(pTransmitKey->dwTSC47_16));
        *((PBYTE)(pMICHDR+11)) = LOBYTE(LOWORD(pTransmitKey->dwTSC47_16));
        *((PBYTE)(pMICHDR+12)) = HIBYTE(pTransmitKey->wTSC15_0);
        *((PBYTE)(pMICHDR+13)) = LOBYTE(pTransmitKey->wTSC15_0);
        *((PBYTE)(pMICHDR+14)) = HIBYTE(wPayloadLen);
        *((PBYTE)(pMICHDR+15)) = LOBYTE(wPayloadLen);

        
        *((PBYTE)(pMICHDR+16)) = 0; 
        if (pDevice->bLongHeader) {
            *((PBYTE)(pMICHDR+17)) = 28; 
        } else {
            *((PBYTE)(pMICHDR+17)) = 22; 
        }
        wValue = cpu_to_le16(pMACHeader->wFrameCtl & 0xC78F);
        memcpy(pMICHDR+18, (PBYTE)&wValue, 2); 
        memcpy(pMICHDR+20, &(pMACHeader->abyAddr1[0]), 6);
        memcpy(pMICHDR+26, &(pMACHeader->abyAddr2[0]), 6);

        
        memcpy(pMICHDR+32, &(pMACHeader->abyAddr3[0]), 6);
        wValue = pMACHeader->wSeqCtl;
        wValue &= 0x000F;
        wValue = cpu_to_le16(wValue);
        memcpy(pMICHDR+38, (PBYTE)&wValue, 2); 
        if (pDevice->bLongHeader) {
            memcpy(pMICHDR+40, &(pMACHeader->abyAddr4[0]), 6);
        }
    }
}


static
VOID
s_vSWencryption (
    IN  PSDevice            pDevice,
    IN  PSKeyItem           pTransmitKey,
    IN  PBYTE               pbyPayloadHead,
    IN  WORD                wPayloadSize
    )
{
    UINT   cbICVlen = 4;
    DWORD  dwICV = 0xFFFFFFFFL;
    PDWORD pdwICV;

    if (pTransmitKey == NULL)
        return;

    if (pTransmitKey->byCipherSuite == KEY_CTL_WEP) {
        
        
        dwICV = CRCdwGetCrc32Ex(pbyPayloadHead, wPayloadSize, dwICV);
        pdwICV = (PDWORD)(pbyPayloadHead + wPayloadSize);
        
        *pdwICV = cpu_to_le32(~dwICV);
        
        rc4_init(&pDevice->SBox, pDevice->abyPRNG, pTransmitKey->uKeyLength + 3);
        rc4_encrypt(&pDevice->SBox, pbyPayloadHead, pbyPayloadHead, wPayloadSize+cbICVlen);
        
    } else if (pTransmitKey->byCipherSuite == KEY_CTL_TKIP) {
        
        
        dwICV = CRCdwGetCrc32Ex(pbyPayloadHead, wPayloadSize, dwICV);
        pdwICV = (PDWORD)(pbyPayloadHead + wPayloadSize);
        
        *pdwICV = cpu_to_le32(~dwICV);
        
        rc4_init(&pDevice->SBox, pDevice->abyPRNG, TKIP_KEY_LEN);
        rc4_encrypt(&pDevice->SBox, pbyPayloadHead, pbyPayloadHead, wPayloadSize+cbICVlen);
        
    }
}





static
UINT
s_uGetTxRsvTime (
    IN PSDevice pDevice,
    IN BYTE     byPktType,
    IN UINT     cbFrameLength,
    IN WORD     wRate,
    IN BOOL     bNeedAck
    )
{
    UINT uDataTime, uAckTime;

    uDataTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, cbFrameLength, wRate);
#ifdef	PLICE_DEBUG
	
#endif
    if (byPktType == PK_TYPE_11B) {
        uAckTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, (WORD)pDevice->byTopCCKBasicRate);
    } else {
        uAckTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, (WORD)pDevice->byTopOFDMBasicRate);
    }

    if (bNeedAck) {
        return (uDataTime + pDevice->uSIFS + uAckTime);
    }
    else {
        return uDataTime;
    }
}


static
UINT
s_uGetRTSCTSRsvTime (
    IN PSDevice pDevice,
    IN BYTE byRTSRsvType,
    IN BYTE byPktType,
    IN UINT cbFrameLength,
    IN WORD wCurrentRate
    )
{
    UINT uRrvTime  , uRTSTime, uCTSTime, uAckTime, uDataTime;

    uRrvTime = uRTSTime = uCTSTime = uAckTime = uDataTime = 0;


    uDataTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, cbFrameLength, wCurrentRate);
    if (byRTSRsvType == 0) { 
        uRTSTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 20, pDevice->byTopCCKBasicRate);
        uCTSTime = uAckTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopCCKBasicRate);
    }
    else if (byRTSRsvType == 1){ 
        uRTSTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 20, pDevice->byTopCCKBasicRate);
        uCTSTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopCCKBasicRate);
        uAckTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopOFDMBasicRate);
    }
    else if (byRTSRsvType == 2) { 
        uRTSTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 20, pDevice->byTopOFDMBasicRate);
        uCTSTime = uAckTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopOFDMBasicRate);
    }
    else if (byRTSRsvType == 3) { 
        uCTSTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopCCKBasicRate);
        uAckTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopOFDMBasicRate);
        uRrvTime = uCTSTime + uAckTime + uDataTime + 2*pDevice->uSIFS;
        return uRrvTime;
    }

    
    uRrvTime = uRTSTime + uCTSTime + uAckTime + uDataTime + 3*pDevice->uSIFS;
    return uRrvTime;
}


static
UINT
s_uGetDataDuration (
    IN PSDevice pDevice,
    IN BYTE     byDurType,
    IN UINT     cbFrameLength,
    IN BYTE     byPktType,
    IN WORD     wRate,
    IN BOOL     bNeedAck,
    IN UINT     uFragIdx,
    IN UINT     cbLastFragmentSize,
    IN UINT     uMACfragNum,
    IN BYTE     byFBOption
    )
{
    BOOL bLastFrag = 0;
    UINT uAckTime =0, uNextPktTime = 0;



    if (uFragIdx == (uMACfragNum-1)) {
        bLastFrag = 1;
    }


    switch (byDurType) {

    case DATADUR_B:    
        if (((uMACfragNum == 1)) || (bLastFrag == 1)) {
            if (bNeedAck) {
            	uAckTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopCCKBasicRate);
                return (pDevice->uSIFS + uAckTime);
            } else {
                return 0;
            }
        }
        else {
            if (uFragIdx == (uMACfragNum-2)) {
            	uNextPktTime = s_uGetTxRsvTime(pDevice, byPktType, cbLastFragmentSize, wRate, bNeedAck);
            } else {
                uNextPktTime = s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wRate, bNeedAck);
            }
            if (bNeedAck) {
            	uAckTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopCCKBasicRate);
                return (pDevice->uSIFS + uAckTime + uNextPktTime);
            } else {
                return (pDevice->uSIFS + uNextPktTime);
            }
        }
        break;

    case DATADUR_A:    
        if (((uMACfragNum==1)) || (bLastFrag==1)) {
            if(bNeedAck){
            	uAckTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopOFDMBasicRate);
                return (pDevice->uSIFS + uAckTime);
            } else {
                return 0;
            }
        }
        else {
            if(uFragIdx == (uMACfragNum-2)){
            	uNextPktTime = s_uGetTxRsvTime(pDevice, byPktType, cbLastFragmentSize, wRate, bNeedAck);
            } else {
                uNextPktTime = s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wRate, bNeedAck);
            }
            if(bNeedAck){
            	uAckTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopOFDMBasicRate);
                return (pDevice->uSIFS + uAckTime + uNextPktTime);
            } else {
                return (pDevice->uSIFS + uNextPktTime);
            }
        }
        break;

    case DATADUR_A_F0:    
	    if (((uMACfragNum==1)) || (bLastFrag==1)) {
            if(bNeedAck){
            	uAckTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopOFDMBasicRate);
                return (pDevice->uSIFS + uAckTime);
            } else {
                return 0;
            }
        }
	    else { 
	        if (byFBOption == AUTO_FB_0) {
                if (wRate < RATE_18M)
                    wRate = RATE_18M;
                else if (wRate > RATE_54M)
                    wRate = RATE_54M;

	            if(uFragIdx == (uMACfragNum-2)){
            	    uNextPktTime = s_uGetTxRsvTime(pDevice, byPktType, cbLastFragmentSize, wFB_Opt0[FB_RATE0][wRate-RATE_18M], bNeedAck);
                } else {
                    uNextPktTime = s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wFB_Opt0[FB_RATE0][wRate-RATE_18M], bNeedAck);
                }
	        } else { 
                if (wRate < RATE_18M)
                    wRate = RATE_18M;
                else if (wRate > RATE_54M)
                    wRate = RATE_54M;

	            if(uFragIdx == (uMACfragNum-2)){
            	    uNextPktTime = s_uGetTxRsvTime(pDevice, byPktType, cbLastFragmentSize, wFB_Opt1[FB_RATE0][wRate-RATE_18M], bNeedAck);
                } else {
                    uNextPktTime = s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wFB_Opt1[FB_RATE0][wRate-RATE_18M], bNeedAck);
                }
	        }

	        if(bNeedAck){
            	uAckTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopOFDMBasicRate);
                return (pDevice->uSIFS + uAckTime + uNextPktTime);
            } else {
                return (pDevice->uSIFS + uNextPktTime);
            }
	    }
        break;

    case DATADUR_A_F1:    
        if (((uMACfragNum==1)) || (bLastFrag==1)) {
            if(bNeedAck){
            	uAckTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopOFDMBasicRate);
                return (pDevice->uSIFS + uAckTime);
            } else {
                return 0;
            }
        }
	    else { 
	        if (byFBOption == AUTO_FB_0) {
                if (wRate < RATE_18M)
                    wRate = RATE_18M;
                else if (wRate > RATE_54M)
                    wRate = RATE_54M;

	            if(uFragIdx == (uMACfragNum-2)){
            	    uNextPktTime = s_uGetTxRsvTime(pDevice, byPktType, cbLastFragmentSize, wFB_Opt0[FB_RATE1][wRate-RATE_18M], bNeedAck);
                } else {
                    uNextPktTime = s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wFB_Opt0[FB_RATE1][wRate-RATE_18M], bNeedAck);
                }

	        } else { 
                if (wRate < RATE_18M)
                    wRate = RATE_18M;
                else if (wRate > RATE_54M)
                    wRate = RATE_54M;

	            if(uFragIdx == (uMACfragNum-2)){
            	    uNextPktTime = s_uGetTxRsvTime(pDevice, byPktType, cbLastFragmentSize, wFB_Opt1[FB_RATE1][wRate-RATE_18M], bNeedAck);
                } else {
                    uNextPktTime = s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wFB_Opt1[FB_RATE1][wRate-RATE_18M], bNeedAck);
                }
	        }
	        if(bNeedAck){
            	uAckTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopOFDMBasicRate);
                return (pDevice->uSIFS + uAckTime + uNextPktTime);
            } else {
                return (pDevice->uSIFS + uNextPktTime);
            }
	    }
        break;

    default:
        break;
    }

	ASSERT(FALSE);
	return 0;
}



static
UINT
s_uGetRTSCTSDuration (
    IN PSDevice pDevice,
    IN BYTE byDurType,
    IN UINT cbFrameLength,
    IN BYTE byPktType,
    IN WORD wRate,
    IN BOOL bNeedAck,
    IN BYTE byFBOption
    )
{
    UINT uCTSTime = 0, uDurTime = 0;


    switch (byDurType) {

    case RTSDUR_BB:    
        uCTSTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopCCKBasicRate);
        uDurTime = uCTSTime + 2*pDevice->uSIFS + s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wRate, bNeedAck);
        break;

    case RTSDUR_BA:    
        uCTSTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopCCKBasicRate);
        uDurTime = uCTSTime + 2*pDevice->uSIFS + s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wRate, bNeedAck);
        break;

    case RTSDUR_AA:    
        uCTSTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopOFDMBasicRate);
        uDurTime = uCTSTime + 2*pDevice->uSIFS + s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wRate, bNeedAck);
        break;

    case CTSDUR_BA:    
        uDurTime = pDevice->uSIFS + s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wRate, bNeedAck);
        break;

    case RTSDUR_BA_F0: 
        uCTSTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopCCKBasicRate);
        if ((byFBOption == AUTO_FB_0) && (wRate >= RATE_18M) && (wRate <=RATE_54M)) {
            uDurTime = uCTSTime + 2*pDevice->uSIFS + s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wFB_Opt0[FB_RATE0][wRate-RATE_18M], bNeedAck);
        } else if ((byFBOption == AUTO_FB_1) && (wRate >= RATE_18M) && (wRate <=RATE_54M)) {
            uDurTime = uCTSTime + 2*pDevice->uSIFS + s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wFB_Opt1[FB_RATE0][wRate-RATE_18M], bNeedAck);
        }
        break;

    case RTSDUR_AA_F0: 
        uCTSTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopOFDMBasicRate);
        if ((byFBOption == AUTO_FB_0) && (wRate >= RATE_18M) && (wRate <=RATE_54M)) {
            uDurTime = uCTSTime + 2*pDevice->uSIFS + s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wFB_Opt0[FB_RATE0][wRate-RATE_18M], bNeedAck);
        } else if ((byFBOption == AUTO_FB_1) && (wRate >= RATE_18M) && (wRate <=RATE_54M)) {
            uDurTime = uCTSTime + 2*pDevice->uSIFS + s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wFB_Opt1[FB_RATE0][wRate-RATE_18M], bNeedAck);
        }
        break;

    case RTSDUR_BA_F1: 
        uCTSTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopCCKBasicRate);
        if ((byFBOption == AUTO_FB_0) && (wRate >= RATE_18M) && (wRate <=RATE_54M)) {
            uDurTime = uCTSTime + 2*pDevice->uSIFS + s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wFB_Opt0[FB_RATE1][wRate-RATE_18M], bNeedAck);
        } else if ((byFBOption == AUTO_FB_1) && (wRate >= RATE_18M) && (wRate <=RATE_54M)) {
            uDurTime = uCTSTime + 2*pDevice->uSIFS + s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wFB_Opt1[FB_RATE1][wRate-RATE_18M], bNeedAck);
        }
        break;

    case RTSDUR_AA_F1: 
        uCTSTime = BBuGetFrameTime(pDevice->byPreambleType, byPktType, 14, pDevice->byTopOFDMBasicRate);
        if ((byFBOption == AUTO_FB_0) && (wRate >= RATE_18M) && (wRate <=RATE_54M)) {
            uDurTime = uCTSTime + 2*pDevice->uSIFS + s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wFB_Opt0[FB_RATE1][wRate-RATE_18M], bNeedAck);
        } else if ((byFBOption == AUTO_FB_1) && (wRate >= RATE_18M) && (wRate <=RATE_54M)) {
            uDurTime = uCTSTime + 2*pDevice->uSIFS + s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wFB_Opt1[FB_RATE1][wRate-RATE_18M], bNeedAck);
        }
        break;

    case CTSDUR_BA_F0: 
        if ((byFBOption == AUTO_FB_0) && (wRate >= RATE_18M) && (wRate <=RATE_54M)) {
            uDurTime = pDevice->uSIFS + s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wFB_Opt0[FB_RATE0][wRate-RATE_18M], bNeedAck);
        } else if ((byFBOption == AUTO_FB_1) && (wRate >= RATE_18M) && (wRate <=RATE_54M)) {
            uDurTime = pDevice->uSIFS + s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wFB_Opt1[FB_RATE0][wRate-RATE_18M], bNeedAck);
        }
        break;

    case CTSDUR_BA_F1: 
        if ((byFBOption == AUTO_FB_0) && (wRate >= RATE_18M) && (wRate <=RATE_54M)) {
            uDurTime = pDevice->uSIFS + s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wFB_Opt0[FB_RATE1][wRate-RATE_18M], bNeedAck);
        } else if ((byFBOption == AUTO_FB_1) && (wRate >= RATE_18M) && (wRate <=RATE_54M)) {
            uDurTime = pDevice->uSIFS + s_uGetTxRsvTime(pDevice, byPktType, cbFrameLength, wFB_Opt1[FB_RATE1][wRate-RATE_18M], bNeedAck);
        }
        break;

    default:
        break;
    }

    return uDurTime;

}



static
UINT
s_uFillDataHead (
    IN PSDevice pDevice,
    IN BYTE     byPktType,
    IN PVOID    pTxDataHead,
    IN UINT     cbFrameLength,
    IN UINT     uDMAIdx,
    IN BOOL     bNeedAck,
    IN UINT     uFragIdx,
    IN UINT     cbLastFragmentSize,
    IN UINT     uMACfragNum,
    IN BYTE     byFBOption,
    IN WORD     wCurrentRate
    )
{
    WORD  wLen = 0x0000;

    if (pTxDataHead == NULL) {
        return 0;
    }

    if (byPktType == PK_TYPE_11GB || byPktType == PK_TYPE_11GA) {
        if (byFBOption == AUTO_FB_NONE) {
            PSTxDataHead_g pBuf = (PSTxDataHead_g)pTxDataHead;
            
            BBvCaculateParameter(pDevice, cbFrameLength, wCurrentRate, byPktType,
                (PWORD)&(wLen), (PBYTE)&(pBuf->byServiceField_a), (PBYTE)&(pBuf->bySignalField_a)
            );
            pBuf->wTransmitLength_a = cpu_to_le16(wLen);
            BBvCaculateParameter(pDevice, cbFrameLength, pDevice->byTopCCKBasicRate, PK_TYPE_11B,
                (PWORD)&(wLen), (PBYTE)&(pBuf->byServiceField_b), (PBYTE)&(pBuf->bySignalField_b)
            );
            pBuf->wTransmitLength_b = cpu_to_le16(wLen);
            
            pBuf->wDuration_a = cpu_to_le16((WORD)s_uGetDataDuration(pDevice, DATADUR_A, cbFrameLength,
                                                         byPktType, wCurrentRate, bNeedAck, uFragIdx,
                                                         cbLastFragmentSize, uMACfragNum,
                                                         byFBOption)); 
            pBuf->wDuration_b = cpu_to_le16((WORD)s_uGetDataDuration(pDevice, DATADUR_B, cbFrameLength,
                                                         PK_TYPE_11B, pDevice->byTopCCKBasicRate,
                                                         bNeedAck, uFragIdx, cbLastFragmentSize,
                                                         uMACfragNum, byFBOption)); 

            pBuf->wTimeStampOff_a = cpu_to_le16(wTimeStampOff[pDevice->byPreambleType%2][wCurrentRate%MAX_RATE]);
            pBuf->wTimeStampOff_b = cpu_to_le16(wTimeStampOff[pDevice->byPreambleType%2][pDevice->byTopCCKBasicRate%MAX_RATE]);

            return (pBuf->wDuration_a);
         } else {
            
            PSTxDataHead_g_FB pBuf = (PSTxDataHead_g_FB)pTxDataHead;
            
            BBvCaculateParameter(pDevice, cbFrameLength, wCurrentRate, byPktType,
                (PWORD)&(wLen), (PBYTE)&(pBuf->byServiceField_a), (PBYTE)&(pBuf->bySignalField_a)
            );
            pBuf->wTransmitLength_a = cpu_to_le16(wLen);
            BBvCaculateParameter(pDevice, cbFrameLength, pDevice->byTopCCKBasicRate, PK_TYPE_11B,
                (PWORD)&(wLen), (PBYTE)&(pBuf->byServiceField_b), (PBYTE)&(pBuf->bySignalField_b)
            );
            pBuf->wTransmitLength_b = cpu_to_le16(wLen);
            
            pBuf->wDuration_a = cpu_to_le16((WORD)s_uGetDataDuration(pDevice, DATADUR_A, cbFrameLength, byPktType,
                                         wCurrentRate, bNeedAck, uFragIdx, cbLastFragmentSize, uMACfragNum, byFBOption)); 
            pBuf->wDuration_b = cpu_to_le16((WORD)s_uGetDataDuration(pDevice, DATADUR_B, cbFrameLength, PK_TYPE_11B,
                                         pDevice->byTopCCKBasicRate, bNeedAck, uFragIdx, cbLastFragmentSize, uMACfragNum, byFBOption)); 
            pBuf->wDuration_a_f0 = cpu_to_le16((WORD)s_uGetDataDuration(pDevice, DATADUR_A_F0, cbFrameLength, byPktType,
                                         wCurrentRate, bNeedAck, uFragIdx, cbLastFragmentSize, uMACfragNum, byFBOption)); 
            pBuf->wDuration_a_f1 = cpu_to_le16((WORD)s_uGetDataDuration(pDevice, DATADUR_A_F1, cbFrameLength, byPktType,
                                         wCurrentRate, bNeedAck, uFragIdx, cbLastFragmentSize, uMACfragNum, byFBOption)); 

            pBuf->wTimeStampOff_a = cpu_to_le16(wTimeStampOff[pDevice->byPreambleType%2][wCurrentRate%MAX_RATE]);
            pBuf->wTimeStampOff_b = cpu_to_le16(wTimeStampOff[pDevice->byPreambleType%2][pDevice->byTopCCKBasicRate%MAX_RATE]);

            return (pBuf->wDuration_a);
        } 
    }
    else if (byPktType == PK_TYPE_11A) {
        if ((byFBOption != AUTO_FB_NONE)) {
            
            PSTxDataHead_a_FB pBuf = (PSTxDataHead_a_FB)pTxDataHead;
            
            BBvCaculateParameter(pDevice, cbFrameLength, wCurrentRate, byPktType,
                (PWORD)&(wLen), (PBYTE)&(pBuf->byServiceField), (PBYTE)&(pBuf->bySignalField)
            );
            pBuf->wTransmitLength = cpu_to_le16(wLen);
            

            pBuf->wDuration = cpu_to_le16((WORD)s_uGetDataDuration(pDevice, DATADUR_A, cbFrameLength, byPktType,
                                        wCurrentRate, bNeedAck, uFragIdx, cbLastFragmentSize, uMACfragNum, byFBOption)); 
            pBuf->wDuration_f0 = cpu_to_le16((WORD)s_uGetDataDuration(pDevice, DATADUR_A_F0, cbFrameLength, byPktType,
                                        wCurrentRate, bNeedAck, uFragIdx, cbLastFragmentSize, uMACfragNum, byFBOption)); 
            pBuf->wDuration_f1 = cpu_to_le16((WORD)s_uGetDataDuration(pDevice, DATADUR_A_F1, cbFrameLength, byPktType,
                                        wCurrentRate, bNeedAck, uFragIdx, cbLastFragmentSize, uMACfragNum, byFBOption)); 
            pBuf->wTimeStampOff = cpu_to_le16(wTimeStampOff[pDevice->byPreambleType%2][wCurrentRate%MAX_RATE]);
            return (pBuf->wDuration);
        } else {
            PSTxDataHead_ab pBuf = (PSTxDataHead_ab)pTxDataHead;
            
            BBvCaculateParameter(pDevice, cbFrameLength, wCurrentRate, byPktType,
                (PWORD)&(wLen), (PBYTE)&(pBuf->byServiceField), (PBYTE)&(pBuf->bySignalField)
            );
            pBuf->wTransmitLength = cpu_to_le16(wLen);
            

            pBuf->wDuration = cpu_to_le16((WORD)s_uGetDataDuration(pDevice, DATADUR_A, cbFrameLength, byPktType,
                                                       wCurrentRate, bNeedAck, uFragIdx,
                                                       cbLastFragmentSize, uMACfragNum,
                                                       byFBOption));

            pBuf->wTimeStampOff = cpu_to_le16(wTimeStampOff[pDevice->byPreambleType%2][wCurrentRate%MAX_RATE]);
            return (pBuf->wDuration);
        }
    }
    else {
            PSTxDataHead_ab pBuf = (PSTxDataHead_ab)pTxDataHead;
            
            BBvCaculateParameter(pDevice, cbFrameLength, wCurrentRate, byPktType,
                (PWORD)&(wLen), (PBYTE)&(pBuf->byServiceField), (PBYTE)&(pBuf->bySignalField)
            );
            pBuf->wTransmitLength = cpu_to_le16(wLen);
            
            pBuf->wDuration = cpu_to_le16((WORD)s_uGetDataDuration(pDevice, DATADUR_B, cbFrameLength, byPktType,
                                                       wCurrentRate, bNeedAck, uFragIdx,
                                                       cbLastFragmentSize, uMACfragNum,
                                                       byFBOption));
            pBuf->wTimeStampOff = cpu_to_le16(wTimeStampOff[pDevice->byPreambleType%2][wCurrentRate%MAX_RATE]);
            return (pBuf->wDuration);
    }
    return 0;
}


static
VOID
s_vFillRTSHead (
    IN PSDevice         pDevice,
    IN BYTE             byPktType,
    IN PVOID            pvRTS,
    IN UINT             cbFrameLength,
    IN BOOL             bNeedAck,
    IN BOOL             bDisCRC,
    IN PSEthernetHeader psEthHeader,
    IN WORD             wCurrentRate,
    IN BYTE             byFBOption
    )
{
    UINT uRTSFrameLen = 20;
    WORD  wLen = 0x0000;

    if (pvRTS == NULL)
    	return;

    if (bDisCRC) {
        
        
        uRTSFrameLen -= 4;
    }

    
    
    if (byPktType == PK_TYPE_11GB || byPktType == PK_TYPE_11GA) {
        if (byFBOption == AUTO_FB_NONE) {
            PSRTS_g pBuf = (PSRTS_g)pvRTS;
            
            BBvCaculateParameter(pDevice, uRTSFrameLen, pDevice->byTopCCKBasicRate, PK_TYPE_11B,
                (PWORD)&(wLen), (PBYTE)&(pBuf->byServiceField_b), (PBYTE)&(pBuf->bySignalField_b)
            );
            pBuf->wTransmitLength_b = cpu_to_le16(wLen);
            BBvCaculateParameter(pDevice, uRTSFrameLen, pDevice->byTopOFDMBasicRate, byPktType,
                (PWORD)&(wLen), (PBYTE)&(pBuf->byServiceField_a), (PBYTE)&(pBuf->bySignalField_a)
            );
            pBuf->wTransmitLength_a = cpu_to_le16(wLen);
            
            pBuf->wDuration_bb = cpu_to_le16((WORD)s_uGetRTSCTSDuration(pDevice, RTSDUR_BB, cbFrameLength, PK_TYPE_11B, pDevice->byTopCCKBasicRate, bNeedAck, byFBOption));    
            pBuf->wDuration_aa = cpu_to_le16((WORD)s_uGetRTSCTSDuration(pDevice, RTSDUR_AA, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption)); 
            pBuf->wDuration_ba = cpu_to_le16((WORD)s_uGetRTSCTSDuration(pDevice, RTSDUR_BA, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption)); 

            pBuf->Data.wDurationID = pBuf->wDuration_aa;
            
            pBuf->Data.wFrameControl = TYPE_CTL_RTS;
            if ((pDevice->eOPMode == OP_MODE_ADHOC) ||
                (pDevice->eOPMode == OP_MODE_AP)) {
                memcpy(&(pBuf->Data.abyRA[0]), &(psEthHeader->abyDstAddr[0]), U_ETHER_ADDR_LEN);
            }
            else {
                memcpy(&(pBuf->Data.abyRA[0]), &(pDevice->abyBSSID[0]), U_ETHER_ADDR_LEN);
            }
            if (pDevice->eOPMode == OP_MODE_AP) {
                memcpy(&(pBuf->Data.abyTA[0]), &(pDevice->abyBSSID[0]), U_ETHER_ADDR_LEN);
            }
            else {
                memcpy(&(pBuf->Data.abyTA[0]), &(psEthHeader->abySrcAddr[0]), U_ETHER_ADDR_LEN);
            }
        }
        else {
           PSRTS_g_FB pBuf = (PSRTS_g_FB)pvRTS;
            
            BBvCaculateParameter(pDevice, uRTSFrameLen, pDevice->byTopCCKBasicRate, PK_TYPE_11B,
                (PWORD)&(wLen), (PBYTE)&(pBuf->byServiceField_b), (PBYTE)&(pBuf->bySignalField_b)
            );
            pBuf->wTransmitLength_b = cpu_to_le16(wLen);
            BBvCaculateParameter(pDevice, uRTSFrameLen, pDevice->byTopOFDMBasicRate, byPktType,
                (PWORD)&(wLen), (PBYTE)&(pBuf->byServiceField_a), (PBYTE)&(pBuf->bySignalField_a)
            );
            pBuf->wTransmitLength_a = cpu_to_le16(wLen);

            
            pBuf->wDuration_bb = cpu_to_le16((WORD)s_uGetRTSCTSDuration(pDevice, RTSDUR_BB, cbFrameLength, PK_TYPE_11B, pDevice->byTopCCKBasicRate, bNeedAck, byFBOption));    
            pBuf->wDuration_aa = cpu_to_le16((WORD)s_uGetRTSCTSDuration(pDevice, RTSDUR_AA, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption)); 
            pBuf->wDuration_ba = cpu_to_le16((WORD)s_uGetRTSCTSDuration(pDevice, RTSDUR_BA, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption)); 
            pBuf->wRTSDuration_ba_f0 = cpu_to_le16((WORD)s_uGetRTSCTSDuration(pDevice, RTSDUR_BA_F0, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption));    
            pBuf->wRTSDuration_aa_f0 = cpu_to_le16((WORD)s_uGetRTSCTSDuration(pDevice, RTSDUR_AA_F0, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption));    
            pBuf->wRTSDuration_ba_f1 = cpu_to_le16((WORD)s_uGetRTSCTSDuration(pDevice, RTSDUR_BA_F1, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption));    
            pBuf->wRTSDuration_aa_f1 = cpu_to_le16((WORD)s_uGetRTSCTSDuration(pDevice, RTSDUR_AA_F1, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption));    
            pBuf->Data.wDurationID = pBuf->wDuration_aa;
            
            pBuf->Data.wFrameControl = TYPE_CTL_RTS;

            if ((pDevice->eOPMode == OP_MODE_ADHOC) ||
                (pDevice->eOPMode == OP_MODE_AP)) {
                memcpy(&(pBuf->Data.abyRA[0]), &(psEthHeader->abyDstAddr[0]), U_ETHER_ADDR_LEN);
            }
            else {
                memcpy(&(pBuf->Data.abyRA[0]), &(pDevice->abyBSSID[0]), U_ETHER_ADDR_LEN);
            }

            if (pDevice->eOPMode == OP_MODE_AP) {
                memcpy(&(pBuf->Data.abyTA[0]), &(pDevice->abyBSSID[0]), U_ETHER_ADDR_LEN);
            }
            else {
                memcpy(&(pBuf->Data.abyTA[0]), &(psEthHeader->abySrcAddr[0]), U_ETHER_ADDR_LEN);
            }

        } 
    }
    else if (byPktType == PK_TYPE_11A) {
        if (byFBOption == AUTO_FB_NONE) {
            PSRTS_ab pBuf = (PSRTS_ab)pvRTS;
            
            BBvCaculateParameter(pDevice, uRTSFrameLen, pDevice->byTopOFDMBasicRate, byPktType,
                (PWORD)&(wLen), (PBYTE)&(pBuf->byServiceField), (PBYTE)&(pBuf->bySignalField)
            );
            pBuf->wTransmitLength = cpu_to_le16(wLen);
            
            pBuf->wDuration = cpu_to_le16((WORD)s_uGetRTSCTSDuration(pDevice, RTSDUR_AA, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption)); 
    	    pBuf->Data.wDurationID = pBuf->wDuration;
            
            pBuf->Data.wFrameControl = TYPE_CTL_RTS;

            if ((pDevice->eOPMode == OP_MODE_ADHOC) ||
                (pDevice->eOPMode == OP_MODE_AP)) {
                memcpy(&(pBuf->Data.abyRA[0]), &(psEthHeader->abyDstAddr[0]), U_ETHER_ADDR_LEN);
            }
            else {
                memcpy(&(pBuf->Data.abyRA[0]), &(pDevice->abyBSSID[0]), U_ETHER_ADDR_LEN);
            }

            if (pDevice->eOPMode == OP_MODE_AP) {
                memcpy(&(pBuf->Data.abyTA[0]), &(pDevice->abyBSSID[0]), U_ETHER_ADDR_LEN);
            }
            else {
                memcpy(&(pBuf->Data.abyTA[0]), &(psEthHeader->abySrcAddr[0]), U_ETHER_ADDR_LEN);
            }

        }
        else {
            PSRTS_a_FB pBuf = (PSRTS_a_FB)pvRTS;
            
            BBvCaculateParameter(pDevice, uRTSFrameLen, pDevice->byTopOFDMBasicRate, byPktType,
                (PWORD)&(wLen), (PBYTE)&(pBuf->byServiceField), (PBYTE)&(pBuf->bySignalField)
            );
            pBuf->wTransmitLength = cpu_to_le16(wLen);
            
            pBuf->wDuration = cpu_to_le16((WORD)s_uGetRTSCTSDuration(pDevice, RTSDUR_AA, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption)); 
    	    pBuf->wRTSDuration_f0 = cpu_to_le16((WORD)s_uGetRTSCTSDuration(pDevice, RTSDUR_AA_F0, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption)); 
    	    pBuf->wRTSDuration_f1 = cpu_to_le16((WORD)s_uGetRTSCTSDuration(pDevice, RTSDUR_AA_F1, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption)); 
    	    pBuf->Data.wDurationID = pBuf->wDuration;
    	    
            pBuf->Data.wFrameControl = TYPE_CTL_RTS;

            if ((pDevice->eOPMode == OP_MODE_ADHOC) ||
                (pDevice->eOPMode == OP_MODE_AP)) {
                memcpy(&(pBuf->Data.abyRA[0]), &(psEthHeader->abyDstAddr[0]), U_ETHER_ADDR_LEN);
            }
            else {
                memcpy(&(pBuf->Data.abyRA[0]), &(pDevice->abyBSSID[0]), U_ETHER_ADDR_LEN);
            }
            if (pDevice->eOPMode == OP_MODE_AP) {
                memcpy(&(pBuf->Data.abyTA[0]), &(pDevice->abyBSSID[0]), U_ETHER_ADDR_LEN);
            }
            else {
                memcpy(&(pBuf->Data.abyTA[0]), &(psEthHeader->abySrcAddr[0]), U_ETHER_ADDR_LEN);
            }
        }
    }
    else if (byPktType == PK_TYPE_11B) {
        PSRTS_ab pBuf = (PSRTS_ab)pvRTS;
        
        BBvCaculateParameter(pDevice, uRTSFrameLen, pDevice->byTopCCKBasicRate, PK_TYPE_11B,
            (PWORD)&(wLen), (PBYTE)&(pBuf->byServiceField), (PBYTE)&(pBuf->bySignalField)
        );
        pBuf->wTransmitLength = cpu_to_le16(wLen);
        
        pBuf->wDuration = cpu_to_le16((WORD)s_uGetRTSCTSDuration(pDevice, RTSDUR_BB, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption)); 
        pBuf->Data.wDurationID = pBuf->wDuration;
        
        pBuf->Data.wFrameControl = TYPE_CTL_RTS;


        if ((pDevice->eOPMode == OP_MODE_ADHOC) ||
            (pDevice->eOPMode == OP_MODE_AP)) {
            memcpy(&(pBuf->Data.abyRA[0]), &(psEthHeader->abyDstAddr[0]), U_ETHER_ADDR_LEN);
        }
        else {
            memcpy(&(pBuf->Data.abyRA[0]), &(pDevice->abyBSSID[0]), U_ETHER_ADDR_LEN);
        }

        if (pDevice->eOPMode == OP_MODE_AP) {
            memcpy(&(pBuf->Data.abyTA[0]), &(pDevice->abyBSSID[0]), U_ETHER_ADDR_LEN);
        }
        else {
            memcpy(&(pBuf->Data.abyTA[0]), &(psEthHeader->abySrcAddr[0]), U_ETHER_ADDR_LEN);
        }
    }
}

static
VOID
s_vFillCTSHead (
    IN PSDevice pDevice,
    IN UINT     uDMAIdx,
    IN BYTE     byPktType,
    IN PVOID    pvCTS,
    IN UINT     cbFrameLength,
    IN BOOL     bNeedAck,
    IN BOOL     bDisCRC,
    IN WORD     wCurrentRate,
    IN BYTE     byFBOption
    )
{
    UINT uCTSFrameLen = 14;
    WORD  wLen = 0x0000;

    if (pvCTS == NULL) {
        return;
    }

    if (bDisCRC) {
        
        
        uCTSFrameLen -= 4;
    }

    if (byPktType == PK_TYPE_11GB || byPktType == PK_TYPE_11GA) {
        if (byFBOption != AUTO_FB_NONE && uDMAIdx != TYPE_ATIMDMA && uDMAIdx != TYPE_BEACONDMA) {
            
            PSCTS_FB pBuf = (PSCTS_FB)pvCTS;
            
            BBvCaculateParameter(pDevice, uCTSFrameLen, pDevice->byTopCCKBasicRate, PK_TYPE_11B,
                (PWORD)&(wLen), (PBYTE)&(pBuf->byServiceField_b), (PBYTE)&(pBuf->bySignalField_b)
            );


            pBuf->wTransmitLength_b = cpu_to_le16(wLen);

            pBuf->wDuration_ba = (WORD)s_uGetRTSCTSDuration(pDevice, CTSDUR_BA, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption); 
            pBuf->wDuration_ba += pDevice->wCTSDuration;
            pBuf->wDuration_ba = cpu_to_le16(pBuf->wDuration_ba);
            
            pBuf->wCTSDuration_ba_f0 = (WORD)s_uGetRTSCTSDuration(pDevice, CTSDUR_BA_F0, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption); 
            pBuf->wCTSDuration_ba_f0 += pDevice->wCTSDuration;
            pBuf->wCTSDuration_ba_f0 = cpu_to_le16(pBuf->wCTSDuration_ba_f0);
            
            pBuf->wCTSDuration_ba_f1 = (WORD)s_uGetRTSCTSDuration(pDevice, CTSDUR_BA_F1, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption); 
            pBuf->wCTSDuration_ba_f1 += pDevice->wCTSDuration;
            pBuf->wCTSDuration_ba_f1 = cpu_to_le16(pBuf->wCTSDuration_ba_f1);
            
            pBuf->Data.wDurationID = pBuf->wDuration_ba;
            pBuf->Data.wFrameControl = TYPE_CTL_CTS;
            pBuf->Data.wReserved = 0x0000;
            memcpy(&(pBuf->Data.abyRA[0]), &(pDevice->abyCurrentNetAddr[0]), U_ETHER_ADDR_LEN);

        } else { 
            PSCTS pBuf = (PSCTS)pvCTS;
            
            BBvCaculateParameter(pDevice, uCTSFrameLen, pDevice->byTopCCKBasicRate, PK_TYPE_11B,
                (PWORD)&(wLen), (PBYTE)&(pBuf->byServiceField_b), (PBYTE)&(pBuf->bySignalField_b)
            );
            pBuf->wTransmitLength_b = cpu_to_le16(wLen);
            
            pBuf->wDuration_ba = cpu_to_le16((WORD)s_uGetRTSCTSDuration(pDevice, CTSDUR_BA, cbFrameLength, byPktType, wCurrentRate, bNeedAck, byFBOption)); 
            pBuf->wDuration_ba += pDevice->wCTSDuration;
            pBuf->wDuration_ba = cpu_to_le16(pBuf->wDuration_ba);

            
            pBuf->Data.wDurationID = pBuf->wDuration_ba;
            pBuf->Data.wFrameControl = TYPE_CTL_CTS;
            pBuf->Data.wReserved = 0x0000;
            memcpy(&(pBuf->Data.abyRA[0]), &(pDevice->abyCurrentNetAddr[0]), U_ETHER_ADDR_LEN);
        }
    }
}








static
VOID
s_vGenerateTxParameter (
    IN PSDevice         pDevice,
    IN BYTE             byPktType,
    IN PVOID            pTxBufHead,
    IN PVOID            pvRrvTime,
    IN PVOID            pvRTS,
    IN PVOID            pvCTS,
    IN UINT             cbFrameSize,
    IN BOOL             bNeedACK,
    IN UINT             uDMAIdx,
    IN PSEthernetHeader psEthHeader,
    IN WORD             wCurrentRate
    )
{
    UINT cbMACHdLen = WLAN_HDR_ADDR3_LEN; 
    WORD wFifoCtl;
    BOOL bDisCRC = FALSE;
    BYTE byFBOption = AUTO_FB_NONE;


    
    PSTxBufHead pFifoHead = (PSTxBufHead)pTxBufHead;
    pFifoHead->wReserved = wCurrentRate;
    wFifoCtl = pFifoHead->wFIFOCtl;

    if (wFifoCtl & FIFOCTL_CRCDIS) {
        bDisCRC = TRUE;
    }

    if (wFifoCtl & FIFOCTL_AUTO_FB_0) {
        byFBOption = AUTO_FB_0;
    }
    else if (wFifoCtl & FIFOCTL_AUTO_FB_1) {
        byFBOption = AUTO_FB_1;
    }

    if (pDevice->bLongHeader)
        cbMACHdLen = WLAN_HDR_ADDR3_LEN + 6;

    if (byPktType == PK_TYPE_11GB || byPktType == PK_TYPE_11GA) {

        if (pvRTS != NULL) { 
            
            if (pvRrvTime) {
                PSRrvTime_gRTS pBuf = (PSRrvTime_gRTS)pvRrvTime;
                pBuf->wRTSTxRrvTime_aa = cpu_to_le16((WORD)s_uGetRTSCTSRsvTime(pDevice, 2, byPktType, cbFrameSize, wCurrentRate));
                pBuf->wRTSTxRrvTime_ba = cpu_to_le16((WORD)s_uGetRTSCTSRsvTime(pDevice, 1, byPktType, cbFrameSize, wCurrentRate));
                pBuf->wRTSTxRrvTime_bb = cpu_to_le16((WORD)s_uGetRTSCTSRsvTime(pDevice, 0, byPktType, cbFrameSize, wCurrentRate));
                pBuf->wTxRrvTime_a = cpu_to_le16((WORD) s_uGetTxRsvTime(pDevice, byPktType, cbFrameSize, wCurrentRate, bNeedACK));
                pBuf->wTxRrvTime_b = cpu_to_le16((WORD) s_uGetTxRsvTime(pDevice, PK_TYPE_11B, cbFrameSize, pDevice->byTopCCKBasicRate, bNeedACK));
            }
            
            s_vFillRTSHead(pDevice, byPktType, pvRTS, cbFrameSize, bNeedACK, bDisCRC, psEthHeader, wCurrentRate, byFBOption);
        }
        else {

            
            if (pvRrvTime) {
                PSRrvTime_gCTS pBuf = (PSRrvTime_gCTS)pvRrvTime;
                pBuf->wTxRrvTime_a = cpu_to_le16((WORD)s_uGetTxRsvTime(pDevice, byPktType, cbFrameSize, wCurrentRate, bNeedACK));
                pBuf->wTxRrvTime_b = cpu_to_le16((WORD)s_uGetTxRsvTime(pDevice, PK_TYPE_11B, cbFrameSize, pDevice->byTopCCKBasicRate, bNeedACK));
                pBuf->wCTSTxRrvTime_ba = cpu_to_le16((WORD)s_uGetRTSCTSRsvTime(pDevice, 3, byPktType, cbFrameSize, wCurrentRate));
            }


            
            s_vFillCTSHead(pDevice, uDMAIdx, byPktType, pvCTS, cbFrameSize, bNeedACK, bDisCRC, wCurrentRate, byFBOption);
        }
    }
    else if (byPktType == PK_TYPE_11A) {

        if (pvRTS != NULL) {
            
            if (pvRrvTime) {
                PSRrvTime_ab pBuf = (PSRrvTime_ab)pvRrvTime;
                pBuf->wRTSTxRrvTime = cpu_to_le16((WORD)s_uGetRTSCTSRsvTime(pDevice, 2, byPktType, cbFrameSize, wCurrentRate));
                pBuf->wTxRrvTime = cpu_to_le16((WORD)s_uGetTxRsvTime(pDevice, byPktType, cbFrameSize, wCurrentRate, bNeedACK));
            }
            
            s_vFillRTSHead(pDevice, byPktType, pvRTS, cbFrameSize, bNeedACK, bDisCRC, psEthHeader, wCurrentRate, byFBOption);
        }
        else if (pvRTS == NULL) {
            
            if (pvRrvTime) {
                PSRrvTime_ab pBuf = (PSRrvTime_ab)pvRrvTime;
                pBuf->wTxRrvTime = cpu_to_le16((WORD)s_uGetTxRsvTime(pDevice, PK_TYPE_11A, cbFrameSize, wCurrentRate, bNeedACK)); 
            }
        }
    }
    else if (byPktType == PK_TYPE_11B) {

        if ((pvRTS != NULL)) {
            
            if (pvRrvTime) {
                PSRrvTime_ab pBuf = (PSRrvTime_ab)pvRrvTime;
                pBuf->wRTSTxRrvTime = cpu_to_le16((WORD)s_uGetRTSCTSRsvTime(pDevice, 0, byPktType, cbFrameSize, wCurrentRate));
                pBuf->wTxRrvTime = cpu_to_le16((WORD)s_uGetTxRsvTime(pDevice, PK_TYPE_11B, cbFrameSize, wCurrentRate, bNeedACK));
            }
            
            s_vFillRTSHead(pDevice, byPktType, pvRTS, cbFrameSize, bNeedACK, bDisCRC, psEthHeader, wCurrentRate, byFBOption);
        }
        else { 
            
            if (pvRrvTime) {
                PSRrvTime_ab pBuf = (PSRrvTime_ab)pvRrvTime;
                pBuf->wTxRrvTime = cpu_to_le16((WORD)s_uGetTxRsvTime(pDevice, PK_TYPE_11B, cbFrameSize, wCurrentRate, bNeedACK)); 
            }
        }
    }
    
}

static
VOID
s_vFillFragParameter(
    IN PSDevice pDevice,
    IN PBYTE    pbyBuffer,
    IN UINT     uTxType,
    IN PVOID    pvtdCurr,
    IN WORD     wFragType,
    IN UINT     cbReqCount
    )
{
    PSTxBufHead pTxBufHead = (PSTxBufHead) pbyBuffer;
    

    if (uTxType == TYPE_SYNCDMA) {
        
        PSTxSyncDesc ptdCurr = (PSTxSyncDesc)pvtdCurr;

         
        ptdCurr->m_wFIFOCtl = pTxBufHead->wFIFOCtl;
        ptdCurr->m_wTimeStamp = pTxBufHead->wTimeStamp;
        
        ptdCurr->m_td1TD1.wReqCount = cpu_to_le16((WORD)(cbReqCount));
        if (wFragType == FRAGCTL_ENDFRAG) { 
            ptdCurr->m_td1TD1.byTCR |= (TCR_STP | TCR_EDP | EDMSDU);
        }
        else {
            ptdCurr->m_td1TD1.byTCR |= (TCR_STP | TCR_EDP);
        }
    }
    else {
        
        PSTxDesc ptdCurr = (PSTxDesc)pvtdCurr;
        
        ptdCurr->m_td1TD1.wReqCount = cpu_to_le16((WORD)(cbReqCount));
        if (wFragType == FRAGCTL_ENDFRAG) { 
            ptdCurr->m_td1TD1.byTCR |= (TCR_STP | TCR_EDP | EDMSDU);
        }
        else {
            ptdCurr->m_td1TD1.byTCR |= (TCR_STP | TCR_EDP);
        }
    }

    pTxBufHead->wFragCtl |= (WORD)wFragType;

    
}

static
UINT
s_cbFillTxBufHead (
    IN  PSDevice         pDevice,
    IN  BYTE             byPktType,
    IN  PBYTE            pbyTxBufferAddr,
    IN  UINT             cbFrameBodySize,
    IN  UINT             uDMAIdx,
    IN  PSTxDesc         pHeadTD,
    IN  PSEthernetHeader psEthHeader,
    IN  PBYTE            pPacket,
    IN  BOOL             bNeedEncrypt,
    IN  PSKeyItem        pTransmitKey,
    IN  UINT             uNodeIndex,
    OUT PUINT            puMACfragNum
    )
{
    UINT           cbMACHdLen;
    UINT           cbFrameSize;
    UINT           cbFragmentSize; 
    UINT           cbFragPayloadSize;
    UINT           cbLastFragmentSize; 
    UINT           cbLastFragPayloadSize;
    UINT           uFragIdx;
    PBYTE          pbyPayloadHead;
    PBYTE          pbyIVHead;
    PBYTE          pbyMacHdr;
    WORD           wFragType; 
    UINT           uDuration;
    PBYTE          pbyBuffer;


    UINT           cbIVlen = 0;
    UINT           cbICVlen = 0;
    UINT           cbMIClen = 0;
    UINT           cbFCSlen = 4;
    UINT           cb802_1_H_len = 0;
    UINT           uLength = 0;
    UINT           uTmpLen = 0;


    UINT           cbMICHDR = 0;
    DWORD          dwMICKey0, dwMICKey1;
    DWORD          dwMIC_Priority;
    PDWORD         pdwMIC_L;
    PDWORD         pdwMIC_R;
    DWORD          dwSafeMIC_L, dwSafeMIC_R; 
    BOOL           bMIC2Frag = FALSE;
    UINT           uMICFragLen = 0;
    UINT           uMACfragNum = 1;
    UINT           uPadding = 0;
    UINT           cbReqCount = 0;

    BOOL           bNeedACK;
    BOOL           bRTS;
    BOOL           bIsAdhoc;
    PBYTE          pbyType;
    PSTxDesc       ptdCurr;
    PSTxBufHead    psTxBufHd = (PSTxBufHead) pbyTxBufferAddr;

    UINT           cbHeaderLength = 0;
    PVOID          pvRrvTime;
    PSMICHDRHead   pMICHDR;
    PVOID          pvRTS;
    PVOID          pvCTS;
    PVOID          pvTxDataHd;
    WORD           wTxBufSize;   
    UINT           uTotalCopyLength = 0;
    BYTE           byFBOption = AUTO_FB_NONE;
    BOOL           bIsWEP256 = FALSE;
    PSMgmtObject    pMgmt = pDevice->pMgmt;


    pvRrvTime = pMICHDR = pvRTS = pvCTS = pvTxDataHd = NULL;

    
    if ((pDevice->eOPMode == OP_MODE_ADHOC) ||
        (pDevice->eOPMode == OP_MODE_AP)) {

        if (IS_MULTICAST_ADDRESS(&(psEthHeader->abyDstAddr[0])) ||
            IS_BROADCAST_ADDRESS(&(psEthHeader->abyDstAddr[0]))) {
            bNeedACK = FALSE;
        }
        else {
            bNeedACK = TRUE;
        }
        bIsAdhoc = TRUE;
    }
    else {
        
        bNeedACK = TRUE;
        bIsAdhoc = FALSE;
    }

    if (pDevice->bLongHeader)
        cbMACHdLen = WLAN_HDR_ADDR3_LEN + 6;
    else
        cbMACHdLen = WLAN_HDR_ADDR3_LEN;


    if ((bNeedEncrypt == TRUE) && (pTransmitKey != NULL)) {
        if (pTransmitKey->byCipherSuite == KEY_CTL_WEP) {
            cbIVlen = 4;
            cbICVlen = 4;
            if (pTransmitKey->uKeyLength == WLAN_WEP232_KEYLEN) {
                bIsWEP256 = TRUE;
            }
        }
        if (pTransmitKey->byCipherSuite == KEY_CTL_TKIP) {
            cbIVlen = 8;
            cbMIClen = 8;
            cbICVlen = 4;
        }
        if (pTransmitKey->byCipherSuite == KEY_CTL_CCMP) {
            cbIVlen = 8;
            cbICVlen = 8;
            cbMICHDR = sizeof(SMICHDRHead);
        }
        if (pDevice->byLocalID > REV_ID_VT3253_A1) {
            
            uPadding = 4 - (cbMACHdLen%4);
            uPadding %= 4;
        }
    }


    cbFrameSize = cbMACHdLen + cbIVlen + (cbFrameBodySize + cbMIClen) + cbICVlen + cbFCSlen;

    if ((bNeedACK == FALSE) ||
        (cbFrameSize < pDevice->wRTSThreshold) ||
        ((cbFrameSize >= pDevice->wFragmentationThreshold) && (pDevice->wFragmentationThreshold <= pDevice->wRTSThreshold))
        ) {
        bRTS = FALSE;
    }
    else {
        bRTS = TRUE;
        psTxBufHd->wFIFOCtl |= (FIFOCTL_RTS | FIFOCTL_LRETRY);
    }
    
    
    
    if (psTxBufHd->wFIFOCtl & FIFOCTL_AUTO_FB_0) {
        byFBOption = AUTO_FB_0;
    }
    else if (psTxBufHd->wFIFOCtl & FIFOCTL_AUTO_FB_1) {
        byFBOption = AUTO_FB_1;
    }

    
    
    wTxBufSize = sizeof(STxBufHead);
    if (byPktType == PK_TYPE_11GB || byPktType == PK_TYPE_11GA) {

        if (byFBOption == AUTO_FB_NONE) {
            if (bRTS == TRUE) {
                pvRrvTime = (PSRrvTime_gRTS) (pbyTxBufferAddr + wTxBufSize);
                pMICHDR = (PSMICHDRHead) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gRTS));
                pvRTS = (PSRTS_g) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gRTS) + cbMICHDR);
                pvCTS = NULL;
                pvTxDataHd = (PSTxDataHead_g) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gRTS) + cbMICHDR + sizeof(SRTS_g));
                cbHeaderLength = wTxBufSize + sizeof(SRrvTime_gRTS) + cbMICHDR + sizeof(SRTS_g) + sizeof(STxDataHead_g);
            }
            else { 
                pvRrvTime = (PSRrvTime_gCTS) (pbyTxBufferAddr + wTxBufSize);
                pMICHDR = (PSMICHDRHead) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gCTS));
                pvRTS = NULL;
                pvCTS = (PSCTS) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gCTS) + cbMICHDR);
                pvTxDataHd = (PSTxDataHead_g) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gCTS) + cbMICHDR + sizeof(SCTS));
                cbHeaderLength = wTxBufSize + sizeof(SRrvTime_gCTS) + cbMICHDR + sizeof(SCTS) + sizeof(STxDataHead_g);
            }
        } else {
            
            if (bRTS == TRUE) {
                pvRrvTime = (PSRrvTime_gRTS) (pbyTxBufferAddr + wTxBufSize);
                pMICHDR = (PSMICHDRHead) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gRTS));
                pvRTS = (PSRTS_g_FB) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gRTS) + cbMICHDR);
                pvCTS = NULL;
                pvTxDataHd = (PSTxDataHead_g_FB) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gRTS) + cbMICHDR + sizeof(SRTS_g_FB));
                cbHeaderLength = wTxBufSize + sizeof(SRrvTime_gRTS) + cbMICHDR + sizeof(SRTS_g_FB) + sizeof(STxDataHead_g_FB);
            }
            else { 
                pvRrvTime = (PSRrvTime_gCTS) (pbyTxBufferAddr + wTxBufSize);
                pMICHDR = (PSMICHDRHead) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gCTS));
                pvRTS = NULL;
                pvCTS = (PSCTS_FB) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gCTS) + cbMICHDR);
                pvTxDataHd = (PSTxDataHead_g_FB) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gCTS) + cbMICHDR + sizeof(SCTS_FB));
                cbHeaderLength = wTxBufSize + sizeof(SRrvTime_gCTS) + cbMICHDR + sizeof(SCTS_FB) + sizeof(STxDataHead_g_FB);
            }
        } 
    }
    else {

        if (byFBOption == AUTO_FB_NONE) {
            if (bRTS == TRUE) {
                pvRrvTime = (PSRrvTime_ab) (pbyTxBufferAddr + wTxBufSize);
                pMICHDR = (PSMICHDRHead) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_ab));
                pvRTS = (PSRTS_ab) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_ab) + cbMICHDR);
                pvCTS = NULL;
                pvTxDataHd = (PSTxDataHead_ab) (pbyTxBufferAddr + wTxBufSize + sizeof(PSRrvTime_ab) + cbMICHDR + sizeof(SRTS_ab));
                cbHeaderLength = wTxBufSize + sizeof(PSRrvTime_ab) + cbMICHDR + sizeof(SRTS_ab) + sizeof(STxDataHead_ab);
            }
            else { 
                pvRrvTime = (PSRrvTime_ab) (pbyTxBufferAddr + wTxBufSize);
                pMICHDR = (PSMICHDRHead) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_ab));
                pvRTS = NULL;
                pvCTS = NULL;
                pvTxDataHd = (PSTxDataHead_ab) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_ab) + cbMICHDR);
                cbHeaderLength = wTxBufSize + sizeof(SRrvTime_ab) + cbMICHDR + sizeof(STxDataHead_ab);
            }
        } else {
            
            if (bRTS == TRUE) {
                pvRrvTime = (PSRrvTime_ab) (pbyTxBufferAddr + wTxBufSize);
                pMICHDR = (PSMICHDRHead) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_ab));
                pvRTS = (PSRTS_a_FB) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_ab) + cbMICHDR);
                pvCTS = NULL;
                pvTxDataHd = (PSTxDataHead_a_FB) (pbyTxBufferAddr + wTxBufSize + sizeof(PSRrvTime_ab) + cbMICHDR + sizeof(SRTS_a_FB));
                cbHeaderLength = wTxBufSize + sizeof(PSRrvTime_ab) + cbMICHDR + sizeof(SRTS_a_FB) + sizeof(STxDataHead_a_FB);
            }
            else { 
                pvRrvTime = (PSRrvTime_ab) (pbyTxBufferAddr + wTxBufSize);
                pMICHDR = (PSMICHDRHead) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_ab));
                pvRTS = NULL;
                pvCTS = NULL;
                pvTxDataHd = (PSTxDataHead_a_FB) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_ab) + cbMICHDR);
                cbHeaderLength = wTxBufSize + sizeof(SRrvTime_ab) + cbMICHDR + sizeof(STxDataHead_a_FB);
            }
        } 
    }
    memset((PVOID)(pbyTxBufferAddr + wTxBufSize), 0, (cbHeaderLength - wTxBufSize));


    if ((bNeedEncrypt == TRUE) && (pTransmitKey != NULL) && (pTransmitKey->byCipherSuite == KEY_CTL_TKIP)) {
        if (pDevice->pMgmt->eAuthenMode == WMAC_AUTH_WPANONE) {
            dwMICKey0 = *(PDWORD)(&pTransmitKey->abyKey[16]);
            dwMICKey1 = *(PDWORD)(&pTransmitKey->abyKey[20]);
        }
        else if ((pTransmitKey->dwKeyIndex & AUTHENTICATOR_KEY) != 0) {
            dwMICKey0 = *(PDWORD)(&pTransmitKey->abyKey[16]);
            dwMICKey1 = *(PDWORD)(&pTransmitKey->abyKey[20]);
        }
        else {
            dwMICKey0 = *(PDWORD)(&pTransmitKey->abyKey[24]);
            dwMICKey1 = *(PDWORD)(&pTransmitKey->abyKey[28]);
        }
        
        MIC_vInit(dwMICKey0, dwMICKey1);
        MIC_vAppend((PBYTE)&(psEthHeader->abyDstAddr[0]), 12);
        dwMIC_Priority = 0;
        MIC_vAppend((PBYTE)&dwMIC_Priority, 4);
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"MIC KEY: %lX, %lX\n", dwMICKey0, dwMICKey1);
    }



    pbyMacHdr = (PBYTE)(pbyTxBufferAddr + cbHeaderLength);
    pbyPayloadHead = (PBYTE)(pbyMacHdr + cbMACHdLen + uPadding + cbIVlen);
    pbyIVHead = (PBYTE)(pbyMacHdr + cbMACHdLen + uPadding);

    if ((cbFrameSize > pDevice->wFragmentationThreshold) && (bNeedACK == TRUE) && (bIsWEP256 == FALSE)) {
        
        
        cbFragmentSize = pDevice->wFragmentationThreshold;
        cbFragPayloadSize = cbFragmentSize - cbMACHdLen - cbIVlen - cbICVlen - cbFCSlen;
        
        uMACfragNum = (WORD) ((cbFrameBodySize + cbMIClen) / cbFragPayloadSize);
        cbLastFragPayloadSize = (cbFrameBodySize + cbMIClen) % cbFragPayloadSize;
        if (cbLastFragPayloadSize == 0) {
            cbLastFragPayloadSize = cbFragPayloadSize;
        } else {
            uMACfragNum++;
        }
        
        cbLastFragmentSize = cbMACHdLen + cbLastFragPayloadSize + cbIVlen + cbICVlen + cbFCSlen;

        for (uFragIdx = 0; uFragIdx < uMACfragNum; uFragIdx ++) {
            if (uFragIdx == 0) {
                
                
                
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Start Fragmentation...\n");
                wFragType = FRAGCTL_STAFRAG;


                
                s_vGenerateTxParameter(pDevice, byPktType, (PVOID)psTxBufHd, pvRrvTime, pvRTS, pvCTS,
                                       cbFragmentSize, bNeedACK, uDMAIdx, psEthHeader, pDevice->wCurrentRate);
                
                uDuration = s_uFillDataHead(pDevice, byPktType, pvTxDataHd, cbFragmentSize, uDMAIdx, bNeedACK,
                                            uFragIdx, cbLastFragmentSize, uMACfragNum, byFBOption, pDevice->wCurrentRate);
                
                vGenerateMACHeader(pDevice, pbyMacHdr, (WORD)uDuration, psEthHeader, bNeedEncrypt,
                                   wFragType, uDMAIdx, uFragIdx);

                if (bNeedEncrypt == TRUE) {
                    
                    s_vFillTxKey(pDevice, (PBYTE)(psTxBufHd->adwTxKey), pbyIVHead, pTransmitKey,
                                 pbyMacHdr, (WORD)cbFragPayloadSize, (PBYTE)pMICHDR);
                    
                    if (pDevice->bEnableHostWEP) {
                        pMgmt->sNodeDBTable[uNodeIndex].dwTSC47_16 = pTransmitKey->dwTSC47_16;
                        pMgmt->sNodeDBTable[uNodeIndex].wTSC15_0 = pTransmitKey->wTSC15_0;
                    }
                }


                
                if (ntohs(psEthHeader->wType) > MAX_DATA_LEN) {
                    if ((psEthHeader->wType == TYPE_PKT_IPX) ||
                        (psEthHeader->wType == cpu_to_le16(0xF380))) {
                        memcpy((PBYTE) (pbyPayloadHead), &pDevice->abySNAP_Bridgetunnel[0], 6);
                    }
                    else {
                        memcpy((PBYTE) (pbyPayloadHead), &pDevice->abySNAP_RFC1042[0], 6);
                    }
                    pbyType = (PBYTE) (pbyPayloadHead + 6);
                    memcpy(pbyType, &(psEthHeader->wType), sizeof(WORD));
                    cb802_1_H_len = 8;
                }

                cbReqCount = cbHeaderLength + cbMACHdLen + uPadding + cbIVlen + cbFragPayloadSize;
                
                
                
                
                
                
                
                
                



                
                pbyBuffer = (PBYTE)pHeadTD->pTDInfo->buf;

                uLength = cbHeaderLength + cbMACHdLen + uPadding + cbIVlen + cb802_1_H_len;
                
                memcpy(pbyBuffer, (PVOID)psTxBufHd, uLength);

                
                memcpy((pbyBuffer + uLength), (pPacket + 14), (cbFragPayloadSize - cb802_1_H_len));


                uTotalCopyLength += cbFragPayloadSize - cb802_1_H_len;

                if ((bNeedEncrypt == TRUE) && (pTransmitKey != NULL) && (pTransmitKey->byCipherSuite == KEY_CTL_TKIP)) {
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Start MIC: %d\n", cbFragPayloadSize);
                    MIC_vAppend((pbyBuffer + uLength - cb802_1_H_len), cbFragPayloadSize);

                }

                
                
                
                if ((pDevice->byLocalID <= REV_ID_VT3253_A1)) {
                    if (bNeedEncrypt) {
                        s_vSWencryption(pDevice, pTransmitKey, (pbyBuffer + uLength - cb802_1_H_len), (WORD)cbFragPayloadSize);
                        cbReqCount += cbICVlen;
                    }
                }

                ptdCurr = (PSTxDesc)pHeadTD;
                
                
                
                
                
                
                
                s_vFillFragParameter(pDevice, pbyBuffer, uDMAIdx, (PVOID)ptdCurr, wFragType, cbReqCount);

                ptdCurr->pTDInfo->dwReqCount = cbReqCount - uPadding;
                ptdCurr->pTDInfo->dwHeaderLength = cbHeaderLength;
                ptdCurr->pTDInfo->skb_dma = ptdCurr->pTDInfo->buf_dma;
                ptdCurr->buff_addr = cpu_to_le32(ptdCurr->pTDInfo->skb_dma);
                pDevice->iTDUsed[uDMAIdx]++;
                pHeadTD = ptdCurr->next;
            }
            else if (uFragIdx == (uMACfragNum-1)) {
                
                
                
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Last Fragmentation...\n");
                

                wFragType = FRAGCTL_ENDFRAG;

                
                s_vGenerateTxParameter(pDevice, byPktType, (PVOID)psTxBufHd, pvRrvTime, pvRTS, pvCTS,
                                       cbLastFragmentSize, bNeedACK, uDMAIdx, psEthHeader, pDevice->wCurrentRate);
                
                uDuration = s_uFillDataHead(pDevice, byPktType, pvTxDataHd, cbLastFragmentSize, uDMAIdx, bNeedACK,
                                            uFragIdx, cbLastFragmentSize, uMACfragNum, byFBOption, pDevice->wCurrentRate);

                
                vGenerateMACHeader(pDevice, pbyMacHdr, (WORD)uDuration, psEthHeader, bNeedEncrypt,
                                   wFragType, uDMAIdx, uFragIdx);

                if (bNeedEncrypt == TRUE) {
                    
                    s_vFillTxKey(pDevice, (PBYTE)(psTxBufHd->adwTxKey), pbyIVHead, pTransmitKey,
                                 pbyMacHdr, (WORD)cbLastFragPayloadSize, (PBYTE)pMICHDR);

                    if (pDevice->bEnableHostWEP) {
                        pMgmt->sNodeDBTable[uNodeIndex].dwTSC47_16 = pTransmitKey->dwTSC47_16;
                        pMgmt->sNodeDBTable[uNodeIndex].wTSC15_0 = pTransmitKey->wTSC15_0;
                    }

                }


                cbReqCount = cbHeaderLength + cbMACHdLen + uPadding + cbIVlen + cbLastFragPayloadSize;
                
                
                



                pbyBuffer = (PBYTE)pHeadTD->pTDInfo->buf;
                

                uLength = cbHeaderLength + cbMACHdLen + uPadding + cbIVlen;

                
                memcpy(pbyBuffer, (PVOID)psTxBufHd, uLength);

                
                if (bMIC2Frag == FALSE) {

                    memcpy((pbyBuffer + uLength),
                             (pPacket + 14 + uTotalCopyLength),
                             (cbLastFragPayloadSize - cbMIClen)
                             );
                    
                    uTmpLen = cbLastFragPayloadSize - cbMIClen;

                }
                if ((bNeedEncrypt == TRUE) && (pTransmitKey != NULL) && (pTransmitKey->byCipherSuite == KEY_CTL_TKIP)) {
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"LAST: uMICFragLen:%d, cbLastFragPayloadSize:%d, uTmpLen:%d\n",
                                   uMICFragLen, cbLastFragPayloadSize, uTmpLen);

                    if (bMIC2Frag == FALSE) {
                        if (uTmpLen != 0)
                            MIC_vAppend((pbyBuffer + uLength), uTmpLen);
                        pdwMIC_L = (PDWORD)(pbyBuffer + uLength + uTmpLen);
                        pdwMIC_R = (PDWORD)(pbyBuffer + uLength + uTmpLen + 4);
                        MIC_vGetMIC(pdwMIC_L, pdwMIC_R);
                        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Last MIC:%lX, %lX\n", *pdwMIC_L, *pdwMIC_R);
                    } else {
                        if (uMICFragLen >= 4) {
                            memcpy((pbyBuffer + uLength), ((PBYTE)&dwSafeMIC_R + (uMICFragLen - 4)),
                                     (cbMIClen - uMICFragLen));
                            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"LAST: uMICFragLen >= 4: %X, %d\n",
                                           *(PBYTE)((PBYTE)&dwSafeMIC_R + (uMICFragLen - 4)),
                                           (cbMIClen - uMICFragLen));

                        } else {
                            memcpy((pbyBuffer + uLength), ((PBYTE)&dwSafeMIC_L + uMICFragLen),
                                     (4 - uMICFragLen));
                            memcpy((pbyBuffer + uLength + (4 - uMICFragLen)), &dwSafeMIC_R, 4);
                            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"LAST: uMICFragLen < 4: %X, %d\n",
                                           *(PBYTE)((PBYTE)&dwSafeMIC_R + uMICFragLen - 4),
                                           (cbMIClen - uMICFragLen));
                        }
                        
                    }
                    MIC_vUnInit();
                } else {
                    ASSERT(uTmpLen == (cbLastFragPayloadSize - cbMIClen));
                }


                
                
                
                if ((pDevice->byLocalID <= REV_ID_VT3253_A1)) {
                    if (bNeedEncrypt) {
                        s_vSWencryption(pDevice, pTransmitKey, (pbyBuffer + uLength), (WORD)cbLastFragPayloadSize);
                        cbReqCount += cbICVlen;
                    }
                }

                ptdCurr = (PSTxDesc)pHeadTD;

                
                
                
                
                
                
                


                s_vFillFragParameter(pDevice, pbyBuffer, uDMAIdx, (PVOID)ptdCurr, wFragType, cbReqCount);

                ptdCurr->pTDInfo->dwReqCount = cbReqCount - uPadding;
                ptdCurr->pTDInfo->dwHeaderLength = cbHeaderLength;
                ptdCurr->pTDInfo->skb_dma = ptdCurr->pTDInfo->buf_dma;
                ptdCurr->buff_addr = cpu_to_le32(ptdCurr->pTDInfo->skb_dma);
                pDevice->iTDUsed[uDMAIdx]++;
                pHeadTD = ptdCurr->next;

            }
            else {
                
                
                
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Middle Fragmentation...\n");
                

                wFragType = FRAGCTL_MIDFRAG;

                
                s_vGenerateTxParameter(pDevice, byPktType, (PVOID)psTxBufHd, pvRrvTime, pvRTS, pvCTS,
                                       cbFragmentSize, bNeedACK, uDMAIdx, psEthHeader, pDevice->wCurrentRate);
                
                uDuration = s_uFillDataHead(pDevice, byPktType, pvTxDataHd, cbFragmentSize, uDMAIdx, bNeedACK,
                                            uFragIdx, cbLastFragmentSize, uMACfragNum, byFBOption, pDevice->wCurrentRate);

                
                vGenerateMACHeader(pDevice, pbyMacHdr, (WORD)uDuration, psEthHeader, bNeedEncrypt,
                                   wFragType, uDMAIdx, uFragIdx);


                if (bNeedEncrypt == TRUE) {
                    
                    s_vFillTxKey(pDevice, (PBYTE)(psTxBufHd->adwTxKey), pbyIVHead, pTransmitKey,
                                 pbyMacHdr, (WORD)cbFragPayloadSize, (PBYTE)pMICHDR);

                    if (pDevice->bEnableHostWEP) {
                        pMgmt->sNodeDBTable[uNodeIndex].dwTSC47_16 = pTransmitKey->dwTSC47_16;
                        pMgmt->sNodeDBTable[uNodeIndex].wTSC15_0 = pTransmitKey->wTSC15_0;
                    }
                }

                cbReqCount = cbHeaderLength + cbMACHdLen + uPadding + cbIVlen + cbFragPayloadSize;
                
                
                
                
                
                
                
                
                


                pbyBuffer = (PBYTE)pHeadTD->pTDInfo->buf;
                


                uLength = cbHeaderLength + cbMACHdLen + uPadding + cbIVlen;

                
                memcpy(pbyBuffer, (PVOID)psTxBufHd, uLength);

                
                memcpy((pbyBuffer + uLength),
                         (pPacket + 14 + uTotalCopyLength),
                         cbFragPayloadSize
                        );
                uTmpLen = cbFragPayloadSize;

                uTotalCopyLength += uTmpLen;

                if ((bNeedEncrypt == TRUE) && (pTransmitKey != NULL) && (pTransmitKey->byCipherSuite == KEY_CTL_TKIP)) {

                    MIC_vAppend((pbyBuffer + uLength), uTmpLen);

                    if (uTmpLen < cbFragPayloadSize) {
                        bMIC2Frag = TRUE;
                        uMICFragLen = cbFragPayloadSize - uTmpLen;
                        ASSERT(uMICFragLen < cbMIClen);

                        pdwMIC_L = (PDWORD)(pbyBuffer + uLength + uTmpLen);
                        pdwMIC_R = (PDWORD)(pbyBuffer + uLength + uTmpLen + 4);
                        MIC_vGetMIC(pdwMIC_L, pdwMIC_R);
                        dwSafeMIC_L = *pdwMIC_L;
                        dwSafeMIC_R = *pdwMIC_R;

                        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"MIDDLE: uMICFragLen:%d, cbFragPayloadSize:%d, uTmpLen:%d\n",
                                       uMICFragLen, cbFragPayloadSize, uTmpLen);
                        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Fill MIC in Middle frag [%d]\n", uMICFragLen);
                        
                        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Get MIC:%lX, %lX\n", *pdwMIC_L, *pdwMIC_R);
                    }
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Middle frag len: %d\n", uTmpLen);
                    

                } else {
                    ASSERT(uTmpLen == (cbFragPayloadSize));
                }

                if ((pDevice->byLocalID <= REV_ID_VT3253_A1)) {
                    if (bNeedEncrypt) {
                        s_vSWencryption(pDevice, pTransmitKey, (pbyBuffer + uLength), (WORD)cbFragPayloadSize);
                        cbReqCount += cbICVlen;
                    }
                }

                ptdCurr = (PSTxDesc)pHeadTD;

                
                
                
                
                
                
                

                s_vFillFragParameter(pDevice, pbyBuffer, uDMAIdx, (PVOID)ptdCurr, wFragType, cbReqCount);

                ptdCurr->pTDInfo->dwReqCount = cbReqCount - uPadding;
                ptdCurr->pTDInfo->dwHeaderLength = cbHeaderLength;
                ptdCurr->pTDInfo->skb_dma = ptdCurr->pTDInfo->buf_dma;
                ptdCurr->buff_addr = cpu_to_le32(ptdCurr->pTDInfo->skb_dma);
                pDevice->iTDUsed[uDMAIdx]++;
                pHeadTD = ptdCurr->next;
            }
        }  
    }
    else {
        
        
        
        
        
        wFragType = FRAGCTL_NONFRAG;

        
        psTxBufHd->wFragCtl |= (WORD)wFragType;

        
        s_vGenerateTxParameter(pDevice, byPktType, (PVOID)psTxBufHd, pvRrvTime, pvRTS, pvCTS,
                               cbFrameSize, bNeedACK, uDMAIdx, psEthHeader, pDevice->wCurrentRate);
        
        uDuration = s_uFillDataHead(pDevice, byPktType, pvTxDataHd, cbFrameSize, uDMAIdx, bNeedACK,
                                    0, 0, uMACfragNum, byFBOption, pDevice->wCurrentRate);

        
        vGenerateMACHeader(pDevice, pbyMacHdr, (WORD)uDuration, psEthHeader, bNeedEncrypt,
                           wFragType, uDMAIdx, 0);

        if (bNeedEncrypt == TRUE) {
            
            s_vFillTxKey(pDevice, (PBYTE)(psTxBufHd->adwTxKey), pbyIVHead, pTransmitKey,
                         pbyMacHdr, (WORD)cbFrameBodySize, (PBYTE)pMICHDR);

            if (pDevice->bEnableHostWEP) {
                pMgmt->sNodeDBTable[uNodeIndex].dwTSC47_16 = pTransmitKey->dwTSC47_16;
                pMgmt->sNodeDBTable[uNodeIndex].wTSC15_0 = pTransmitKey->wTSC15_0;
            }
        }

        
        if (ntohs(psEthHeader->wType) > MAX_DATA_LEN) {
            if ((psEthHeader->wType == TYPE_PKT_IPX) ||
                (psEthHeader->wType == cpu_to_le16(0xF380))) {
                memcpy((PBYTE) (pbyPayloadHead), &pDevice->abySNAP_Bridgetunnel[0], 6);
            }
            else {
                memcpy((PBYTE) (pbyPayloadHead), &pDevice->abySNAP_RFC1042[0], 6);
            }
            pbyType = (PBYTE) (pbyPayloadHead + 6);
            memcpy(pbyType, &(psEthHeader->wType), sizeof(WORD));
            cb802_1_H_len = 8;
        }

        cbReqCount = cbHeaderLength + cbMACHdLen + uPadding + cbIVlen + (cbFrameBodySize + cbMIClen);
        
        
        
        
        
        
        
        

        pbyBuffer = (PBYTE)pHeadTD->pTDInfo->buf;
        

        uLength = cbHeaderLength + cbMACHdLen + uPadding + cbIVlen + cb802_1_H_len;

        
        memcpy(pbyBuffer, (PVOID)psTxBufHd, uLength);

        
        memcpy((pbyBuffer + uLength),
                 (pPacket + 14),
                 cbFrameBodySize - cb802_1_H_len
                 );

        if ((bNeedEncrypt == TRUE) && (pTransmitKey != NULL) && (pTransmitKey->byCipherSuite == KEY_CTL_TKIP)){

            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Length:%d, %d\n", cbFrameBodySize - cb802_1_H_len, uLength);
            

            MIC_vAppend((pbyBuffer + uLength - cb802_1_H_len), cbFrameBodySize);

            pdwMIC_L = (PDWORD)(pbyBuffer + uLength - cb802_1_H_len + cbFrameBodySize);
            pdwMIC_R = (PDWORD)(pbyBuffer + uLength - cb802_1_H_len + cbFrameBodySize + 4);

            MIC_vGetMIC(pdwMIC_L, pdwMIC_R);
            MIC_vUnInit();


            if (pDevice->bTxMICFail == TRUE) {
                *pdwMIC_L = 0;
                *pdwMIC_R = 0;
                pDevice->bTxMICFail = FALSE;
            }

            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"uLength: %d, %d\n", uLength, cbFrameBodySize);
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"cbReqCount:%d, %d, %d, %d\n", cbReqCount, cbHeaderLength, uPadding, cbIVlen);
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"MIC:%lx, %lx\n", *pdwMIC_L, *pdwMIC_R);


        }


        if ((pDevice->byLocalID <= REV_ID_VT3253_A1)){
            if (bNeedEncrypt) {
                s_vSWencryption(pDevice, pTransmitKey, (pbyBuffer + uLength - cb802_1_H_len),
                                (WORD)(cbFrameBodySize + cbMIClen));
                cbReqCount += cbICVlen;
            }
        }


        ptdCurr = (PSTxDesc)pHeadTD;

        ptdCurr->pTDInfo->dwReqCount = cbReqCount - uPadding;
        ptdCurr->pTDInfo->dwHeaderLength = cbHeaderLength;
        ptdCurr->pTDInfo->skb_dma = ptdCurr->pTDInfo->buf_dma;
        ptdCurr->buff_addr = cpu_to_le32(ptdCurr->pTDInfo->skb_dma);
  	    
        ptdCurr->m_td1TD1.byTCR |= (TCR_STP | TCR_EDP | EDMSDU);
        ptdCurr->m_td1TD1.wReqCount = cpu_to_le16((WORD)(cbReqCount));

        pDevice->iTDUsed[uDMAIdx]++;





    }
    *puMACfragNum = uMACfragNum;
    
    return cbHeaderLength;
}


VOID
vGenerateFIFOHeader (
    IN  PSDevice         pDevice,
    IN  BYTE             byPktType,
    IN  PBYTE            pbyTxBufferAddr,
    IN  BOOL             bNeedEncrypt,
    IN  UINT             cbPayloadSize,
    IN  UINT             uDMAIdx,
    IN  PSTxDesc         pHeadTD,
    IN  PSEthernetHeader psEthHeader,
    IN  PBYTE            pPacket,
    IN  PSKeyItem        pTransmitKey,
    IN  UINT             uNodeIndex,
    OUT PUINT            puMACfragNum,
    OUT PUINT            pcbHeaderSize
    )
{
    UINT            wTxBufSize;       
    BOOL            bNeedACK;
    BOOL            bIsAdhoc;
    WORD            cbMacHdLen;
    PSTxBufHead     pTxBufHead = (PSTxBufHead) pbyTxBufferAddr;

    wTxBufSize = sizeof(STxBufHead);

    memset(pTxBufHead, 0, wTxBufSize);
    

    if ((pDevice->eOPMode == OP_MODE_ADHOC) ||
        (pDevice->eOPMode == OP_MODE_AP)) {
        if (IS_MULTICAST_ADDRESS(&(psEthHeader->abyDstAddr[0])) ||
            IS_BROADCAST_ADDRESS(&(psEthHeader->abyDstAddr[0]))) {
            bNeedACK = FALSE;
            pTxBufHead->wFIFOCtl = pTxBufHead->wFIFOCtl & (~FIFOCTL_NEEDACK);
        }
        else {
            bNeedACK = TRUE;
            pTxBufHead->wFIFOCtl |= FIFOCTL_NEEDACK;
        }
        bIsAdhoc = TRUE;
    }
    else {
        
        bNeedACK = TRUE;
        pTxBufHead->wFIFOCtl |= FIFOCTL_NEEDACK;
        bIsAdhoc = FALSE;
    }


    pTxBufHead->wFIFOCtl |= FIFOCTL_TMOEN;
    pTxBufHead->wTimeStamp = cpu_to_le16(DEFAULT_MSDU_LIFETIME_RES_64us);

    
    if (pDevice->bLongHeader)
        pTxBufHead->wFIFOCtl |= FIFOCTL_LHEAD;

    

    pTxBufHead->wFIFOCtl |= FIFOCTL_GENINT;


    
    if (TYPE_TXDMA0 == uDMAIdx) {
        pTxBufHead->wFIFOCtl |= FIFOCTL_ISDMA0;
    }

    
    if (pDevice->bLongHeader) {
        cbMacHdLen = WLAN_HDR_ADDR3_LEN + 6;
    } else {
        cbMacHdLen = WLAN_HDR_ADDR3_LEN;
    }
    pTxBufHead->wFragCtl |= cpu_to_le16((WORD)(cbMacHdLen << 10));

    
    if (byPktType == PK_TYPE_11A) {
        ;
    }
    else if (byPktType == PK_TYPE_11B) {
        pTxBufHead->wFIFOCtl |= FIFOCTL_11B;
    }
    else if (byPktType == PK_TYPE_11GB) {
        pTxBufHead->wFIFOCtl |= FIFOCTL_11GB;
    }
    else if (byPktType == PK_TYPE_11GA) {
        pTxBufHead->wFIFOCtl |= FIFOCTL_11GA;
    }
    
    if (pDevice->bGrpAckPolicy == TRUE) {
        pTxBufHead->wFIFOCtl |=	FIFOCTL_GRPACK;
    }

    
    if (pDevice->wCurrentRate >= RATE_18M) {
        if (pDevice->byAutoFBCtrl == AUTO_FB_0) {
            pTxBufHead->wFIFOCtl |= FIFOCTL_AUTO_FB_0;
        } else if (pDevice->byAutoFBCtrl == AUTO_FB_1) {
            pTxBufHead->wFIFOCtl |= FIFOCTL_AUTO_FB_1;
        }
    }

    
    pDevice->bAES = FALSE;

    
    if (pDevice->byLocalID > REV_ID_VT3253_A1) {
        if ((bNeedEncrypt) && (pTransmitKey != NULL))  { 
            if (pTransmitKey->byCipherSuite == KEY_CTL_TKIP) {
                pTxBufHead->wFragCtl |= FRAGCTL_TKIP;
            }
            else if (pTransmitKey->byCipherSuite == KEY_CTL_WEP) { 
                if (pTransmitKey->uKeyLength != WLAN_WEP232_KEYLEN)
                    pTxBufHead->wFragCtl |= FRAGCTL_LEGACY;
            }
            else if (pTransmitKey->byCipherSuite == KEY_CTL_CCMP) { 
                pTxBufHead->wFragCtl |= FRAGCTL_AES;
            }
        }
    }

#ifdef	PLICE_DEBUG
	

	
	
	
	
	

	RFbSetPower(pDevice, pDevice->wCurrentRate, pDevice->byCurrentCh);
#endif
		
		
		pTxBufHead->byTxPower = pDevice->byCurPwr;





    *pcbHeaderSize = s_cbFillTxBufHead(pDevice, byPktType, pbyTxBufferAddr, cbPayloadSize,
                                   uDMAIdx, pHeadTD, psEthHeader, pPacket, bNeedEncrypt,
                                   pTransmitKey, uNodeIndex, puMACfragNum);

    return;
}






VOID
vGenerateMACHeader (
    IN PSDevice         pDevice,
    IN PBYTE            pbyBufferAddr,
    IN WORD             wDuration,
    IN PSEthernetHeader psEthHeader,
    IN BOOL             bNeedEncrypt,
    IN WORD             wFragType,
    IN UINT             uDMAIdx,
    IN UINT             uFragIdx
    )
{
    PS802_11Header  pMACHeader = (PS802_11Header)pbyBufferAddr;

    memset(pMACHeader, 0, (sizeof(S802_11Header)));  

    if (uDMAIdx == TYPE_ATIMDMA) {
    	pMACHeader->wFrameCtl = TYPE_802_11_ATIM;
    } else {
        pMACHeader->wFrameCtl = TYPE_802_11_DATA;
    }

    if (pDevice->eOPMode == OP_MODE_AP) {
        memcpy(&(pMACHeader->abyAddr1[0]), &(psEthHeader->abyDstAddr[0]), U_ETHER_ADDR_LEN);
        memcpy(&(pMACHeader->abyAddr2[0]), &(pDevice->abyBSSID[0]), U_ETHER_ADDR_LEN);
        memcpy(&(pMACHeader->abyAddr3[0]), &(psEthHeader->abySrcAddr[0]), U_ETHER_ADDR_LEN);
        pMACHeader->wFrameCtl |= FC_FROMDS;
    }
    else {
        if (pDevice->eOPMode == OP_MODE_ADHOC) {
            memcpy(&(pMACHeader->abyAddr1[0]), &(psEthHeader->abyDstAddr[0]), U_ETHER_ADDR_LEN);
            memcpy(&(pMACHeader->abyAddr2[0]), &(psEthHeader->abySrcAddr[0]), U_ETHER_ADDR_LEN);
            memcpy(&(pMACHeader->abyAddr3[0]), &(pDevice->abyBSSID[0]), U_ETHER_ADDR_LEN);
        }
        else {
            memcpy(&(pMACHeader->abyAddr3[0]), &(psEthHeader->abyDstAddr[0]), U_ETHER_ADDR_LEN);
            memcpy(&(pMACHeader->abyAddr2[0]), &(psEthHeader->abySrcAddr[0]), U_ETHER_ADDR_LEN);
            memcpy(&(pMACHeader->abyAddr1[0]), &(pDevice->abyBSSID[0]), U_ETHER_ADDR_LEN);
            pMACHeader->wFrameCtl |= FC_TODS;
        }
    }

    if (bNeedEncrypt)
        pMACHeader->wFrameCtl |= cpu_to_le16((WORD)WLAN_SET_FC_ISWEP(1));

    pMACHeader->wDurationID = cpu_to_le16(wDuration);

    if (pDevice->bLongHeader) {
        PWLAN_80211HDR_A4 pMACA4Header  = (PWLAN_80211HDR_A4) pbyBufferAddr;
        pMACHeader->wFrameCtl |= (FC_TODS | FC_FROMDS);
        memcpy(pMACA4Header->abyAddr4, pDevice->abyBSSID, WLAN_ADDR_LEN);
    }
    pMACHeader->wSeqCtl = cpu_to_le16(pDevice->wSeqCounter << 4);

    
    pMACHeader->wSeqCtl |= cpu_to_le16((WORD)uFragIdx);

    if ((wFragType == FRAGCTL_ENDFRAG) || (wFragType == FRAGCTL_NONFRAG)) {
        pDevice->wSeqCounter++;
        if (pDevice->wSeqCounter > 0x0fff)
            pDevice->wSeqCounter = 0;
    }

    if ((wFragType == FRAGCTL_STAFRAG) || (wFragType == FRAGCTL_MIDFRAG)) { 
        pMACHeader->wFrameCtl |= FC_MOREFRAG;
    }
}






CMD_STATUS csMgmt_xmit(PSDevice pDevice, PSTxMgmtPacket pPacket) {

    PSTxDesc        pFrstTD;
    BYTE            byPktType;
    PBYTE           pbyTxBufferAddr;
    PVOID           pvRTS;
    PSCTS           pCTS;
    PVOID           pvTxDataHd;
    UINT            uDuration;
    UINT            cbReqCount;
    PS802_11Header  pMACHeader;
    UINT            cbHeaderSize;
    UINT            cbFrameBodySize;
    BOOL            bNeedACK;
    BOOL            bIsPSPOLL = FALSE;
    PSTxBufHead     pTxBufHead;
    UINT            cbFrameSize;
    UINT            cbIVlen = 0;
    UINT            cbICVlen = 0;
    UINT            cbMIClen = 0;
    UINT            cbFCSlen = 4;
    UINT            uPadding = 0;
    WORD            wTxBufSize;
    UINT            cbMacHdLen;
    SEthernetHeader sEthHeader;
    PVOID           pvRrvTime;
    PVOID           pMICHDR;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    WORD            wCurrentRate = RATE_1M;


    if (AVAIL_TD(pDevice, TYPE_TXDMA0) <= 0) {
        return CMD_STATUS_RESOURCES;
    }

    pFrstTD = pDevice->apCurrTD[TYPE_TXDMA0];
    pbyTxBufferAddr = (PBYTE)pFrstTD->pTDInfo->buf;
    cbFrameBodySize = pPacket->cbPayloadLen;
    pTxBufHead = (PSTxBufHead) pbyTxBufferAddr;
    wTxBufSize = sizeof(STxBufHead);
    memset(pTxBufHead, 0, wTxBufSize);

    if (pDevice->eCurrentPHYType == PHY_TYPE_11A) {
        wCurrentRate = RATE_6M;
        byPktType = PK_TYPE_11A;
    } else {
        wCurrentRate = RATE_1M;
        byPktType = PK_TYPE_11B;
    }

    
    
    
    
    if (pDevice->pMgmt->eScanState != WMAC_NO_SCANNING) {

		RFbSetPower(pDevice, wCurrentRate, pDevice->byCurrentCh);
    } else {
        RFbSetPower(pDevice, wCurrentRate, pMgmt->uCurrChannel);
    }
    pTxBufHead->byTxPower = pDevice->byCurPwr;
    
    if (pDevice->byFOETuning) {
        if ((pPacket->p80211Header->sA3.wFrameCtl & TYPE_DATE_NULL) == TYPE_DATE_NULL) {
            wCurrentRate = RATE_24M;
            byPktType = PK_TYPE_11GA;
        }
    }

    
    if (byPktType == PK_TYPE_11A) {
        pTxBufHead->wFIFOCtl = 0;
    }
    else if (byPktType == PK_TYPE_11B) {
        pTxBufHead->wFIFOCtl |= FIFOCTL_11B;
    }
    else if (byPktType == PK_TYPE_11GB) {
        pTxBufHead->wFIFOCtl |= FIFOCTL_11GB;
    }
    else if (byPktType == PK_TYPE_11GA) {
        pTxBufHead->wFIFOCtl |= FIFOCTL_11GA;
    }

    pTxBufHead->wFIFOCtl |= FIFOCTL_TMOEN;
    pTxBufHead->wTimeStamp = cpu_to_le16(DEFAULT_MGN_LIFETIME_RES_64us);


    if (IS_MULTICAST_ADDRESS(&(pPacket->p80211Header->sA3.abyAddr1[0])) ||
        IS_BROADCAST_ADDRESS(&(pPacket->p80211Header->sA3.abyAddr1[0]))) {
        bNeedACK = FALSE;
    }
    else {
        bNeedACK = TRUE;
        pTxBufHead->wFIFOCtl |= FIFOCTL_NEEDACK;
    };

    if ((pMgmt->eCurrMode == WMAC_MODE_ESS_AP) ||
        (pMgmt->eCurrMode == WMAC_MODE_IBSS_STA) ) {

        pTxBufHead->wFIFOCtl |= FIFOCTL_LRETRY;
        
        
        
        
        
        
        
    }

    pTxBufHead->wFIFOCtl |= (FIFOCTL_GENINT | FIFOCTL_ISDMA0);

    if ((pPacket->p80211Header->sA4.wFrameCtl & TYPE_SUBTYPE_MASK) == TYPE_CTL_PSPOLL) {
        bIsPSPOLL = TRUE;
        cbMacHdLen = WLAN_HDR_ADDR2_LEN;
    } else {
        cbMacHdLen = WLAN_HDR_ADDR3_LEN;
    }

    
    pTxBufHead->wFragCtl |= cpu_to_le16((WORD)(cbMacHdLen << 10));

    
    
    
    pDevice->bAES = FALSE;  

    if (WLAN_GET_FC_ISWEP(pPacket->p80211Header->sA4.wFrameCtl) != 0) {
        if (pDevice->eEncryptionStatus == Ndis802_11Encryption1Enabled) {
            cbIVlen = 4;
            cbICVlen = 4;
    	    pTxBufHead->wFragCtl |= FRAGCTL_LEGACY;
        }
        else if (pDevice->eEncryptionStatus == Ndis802_11Encryption2Enabled) {
            cbIVlen = 8;
            cbMIClen = 8;
            cbICVlen = 4;
    	    pTxBufHead->wFragCtl |= FRAGCTL_TKIP;
    	    
            
            
        }
        else if (pDevice->eEncryptionStatus == Ndis802_11Encryption3Enabled) {
            cbIVlen = 8;
            cbICVlen = 8;
            pTxBufHead->wFragCtl |= FRAGCTL_AES;
            pDevice->bAES = TRUE;
        }
        
        uPadding = 4 - (cbMacHdLen%4);
        uPadding %= 4;
    }

    cbFrameSize = cbMacHdLen + cbFrameBodySize + cbIVlen + cbMIClen + cbICVlen + cbFCSlen;

    
    if (pDevice->bGrpAckPolicy == TRUE) {
        pTxBufHead->wFIFOCtl |=	FIFOCTL_GRPACK;
    }
    

    
    if (byPktType == PK_TYPE_11GB || byPktType == PK_TYPE_11GA) {

        pvRrvTime = (PSRrvTime_gCTS) (pbyTxBufferAddr + wTxBufSize);
        pMICHDR = NULL;
        pvRTS = NULL;
        pCTS = (PSCTS) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gCTS));
        pvTxDataHd = (PSTxDataHead_g) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gCTS) + sizeof(SCTS));
        cbHeaderSize = wTxBufSize + sizeof(SRrvTime_gCTS) + sizeof(SCTS) + sizeof(STxDataHead_g);
    }
    else { 
        pvRrvTime = (PSRrvTime_ab) (pbyTxBufferAddr + wTxBufSize);
        pMICHDR = NULL;
        pvRTS = NULL;
        pCTS = NULL;
        pvTxDataHd = (PSTxDataHead_ab) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_ab));
        cbHeaderSize = wTxBufSize + sizeof(SRrvTime_ab) + sizeof(STxDataHead_ab);
    }

    memset((PVOID)(pbyTxBufferAddr + wTxBufSize), 0, (cbHeaderSize - wTxBufSize));

    memcpy(&(sEthHeader.abyDstAddr[0]), &(pPacket->p80211Header->sA3.abyAddr1[0]), U_ETHER_ADDR_LEN);
    memcpy(&(sEthHeader.abySrcAddr[0]), &(pPacket->p80211Header->sA3.abyAddr2[0]), U_ETHER_ADDR_LEN);
    
    
    
    pTxBufHead->wFragCtl |= (WORD)FRAGCTL_NONFRAG;


    
    s_vGenerateTxParameter(pDevice, byPktType, pbyTxBufferAddr, pvRrvTime, pvRTS, pCTS,
                           cbFrameSize, bNeedACK, TYPE_TXDMA0, &sEthHeader, wCurrentRate);

    
    uDuration = s_uFillDataHead(pDevice, byPktType, pvTxDataHd, cbFrameSize, TYPE_TXDMA0, bNeedACK,
                                0, 0, 1, AUTO_FB_NONE, wCurrentRate);

    pMACHeader = (PS802_11Header) (pbyTxBufferAddr + cbHeaderSize);

    cbReqCount = cbHeaderSize + cbMacHdLen + uPadding + cbIVlen + cbFrameBodySize;

    if (WLAN_GET_FC_ISWEP(pPacket->p80211Header->sA4.wFrameCtl) != 0) {
        PBYTE           pbyIVHead;
        PBYTE           pbyPayloadHead;
        PBYTE           pbyBSSID;
        PSKeyItem       pTransmitKey = NULL;

        pbyIVHead = (PBYTE)(pbyTxBufferAddr + cbHeaderSize + cbMacHdLen + uPadding);
        pbyPayloadHead = (PBYTE)(pbyTxBufferAddr + cbHeaderSize + cbMacHdLen + uPadding + cbIVlen);

        
        
        

        
        
        
        
        
        
        
        
        
        do {
            if ((pDevice->eOPMode == OP_MODE_INFRASTRUCTURE) &&
                (pDevice->bLinkPass == TRUE)) {
                pbyBSSID = pDevice->abyBSSID;
                
                if (KeybGetTransmitKey(&(pDevice->sKey), pbyBSSID, PAIRWISE_KEY, &pTransmitKey) == FALSE) {
                    
                    if(KeybGetTransmitKey(&(pDevice->sKey), pbyBSSID, GROUP_KEY, &pTransmitKey) == TRUE) {
                        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Get GTK.\n");
                        break;
                    }
                } else {
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Get PTK.\n");
                    break;
                }
            }
            
            pbyBSSID = pDevice->abyBroadcastAddr;
            if(KeybGetTransmitKey(&(pDevice->sKey), pbyBSSID, GROUP_KEY, &pTransmitKey) == FALSE) {
                pTransmitKey = NULL;
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"KEY is NULL. OP Mode[%d]\n", pDevice->eOPMode);
            } else {
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Get GTK.\n");
            }
        } while(FALSE);
        
        s_vFillTxKey(pDevice, (PBYTE)(pTxBufHead->adwTxKey), pbyIVHead, pTransmitKey,
                     (PBYTE)pMACHeader, (WORD)cbFrameBodySize, NULL);

        memcpy(pMACHeader, pPacket->p80211Header, cbMacHdLen);
        memcpy(pbyPayloadHead, ((PBYTE)(pPacket->p80211Header) + cbMacHdLen),
                 cbFrameBodySize);
    }
    else {
        
        memcpy(pMACHeader, pPacket->p80211Header, pPacket->cbMPDULen);
    }

    pMACHeader->wSeqCtl = cpu_to_le16(pDevice->wSeqCounter << 4);
    pDevice->wSeqCounter++ ;
    if (pDevice->wSeqCounter > 0x0fff)
        pDevice->wSeqCounter = 0;

    if (bIsPSPOLL) {
        
        
        
        
        
        if (byPktType == PK_TYPE_11GB || byPktType == PK_TYPE_11GA) {
            ((PSTxDataHead_g)pvTxDataHd)->wDuration_a = cpu_to_le16(pPacket->p80211Header->sA2.wDurationID);
            ((PSTxDataHead_g)pvTxDataHd)->wDuration_b = cpu_to_le16(pPacket->p80211Header->sA2.wDurationID);
        } else {
            ((PSTxDataHead_ab)pvTxDataHd)->wDuration = cpu_to_le16(pPacket->p80211Header->sA2.wDurationID);
        }
    }


    
    
    pFrstTD->m_td1TD1.byTCR = (TCR_STP | TCR_EDP | EDMSDU);
    pFrstTD->pTDInfo->skb_dma = pFrstTD->pTDInfo->buf_dma;
    pFrstTD->m_td1TD1.wReqCount = cpu_to_le16((WORD)(cbReqCount));
    pFrstTD->buff_addr = cpu_to_le32(pFrstTD->pTDInfo->skb_dma);
    pFrstTD->pTDInfo->byFlags = 0;

    if (MACbIsRegBitsOn(pDevice->PortOffset, MAC_REG_PSCTL, PSCTL_PS)) {
        
        MACbPSWakeup(pDevice->PortOffset);
    }
    pDevice->bPWBitOn = FALSE;

    wmb();
    pFrstTD->m_td0TD0.f1Owner = OWNED_BY_NIC;
    wmb();

    pDevice->iTDUsed[TYPE_TXDMA0]++;

    if (AVAIL_TD(pDevice, TYPE_TXDMA0) <= 1) {
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO " available td0 <= 1\n");
    }

    pDevice->apCurrTD[TYPE_TXDMA0] = pFrstTD->next;
#ifdef	PLICE_DEBUG
		
#endif

#ifdef TxInSleep
  pDevice->nTxDataTimeCout=0; 
  #endif

    
    MACvTransmit0(pDevice->PortOffset);

    return CMD_STATUS_PENDING;

}


CMD_STATUS csBeacon_xmit(PSDevice pDevice, PSTxMgmtPacket pPacket) {

    BYTE             byPktType;
    PBYTE            pbyBuffer = (PBYTE)pDevice->tx_beacon_bufs;
    UINT             cbFrameSize = pPacket->cbMPDULen + WLAN_FCS_LEN;
    UINT             cbHeaderSize = 0;
    WORD             wTxBufSize = sizeof(STxShortBufHead);
    PSTxShortBufHead pTxBufHead = (PSTxShortBufHead) pbyBuffer;
    PSTxDataHead_ab  pTxDataHead = (PSTxDataHead_ab) (pbyBuffer + wTxBufSize);
    PS802_11Header   pMACHeader;
    WORD             wCurrentRate;
    WORD             wLen = 0x0000;


    memset(pTxBufHead, 0, wTxBufSize);

    if (pDevice->eCurrentPHYType == PHY_TYPE_11A) {
        wCurrentRate = RATE_6M;
        byPktType = PK_TYPE_11A;
    } else {
        wCurrentRate = RATE_2M;
        byPktType = PK_TYPE_11B;
    }

    
    pDevice->byPreambleType = PREAMBLE_LONG;

    

    pTxBufHead->wFIFOCtl |= FIFOCTL_GENINT;


    
    if (byPktType == PK_TYPE_11A) {
        pTxDataHead->wDuration = cpu_to_le16((WORD)s_uGetDataDuration(pDevice, DATADUR_A, cbFrameSize, byPktType,
                                                          wCurrentRate, FALSE, 0, 0, 1, AUTO_FB_NONE));
    }
    else if (byPktType == PK_TYPE_11B) {
        pTxBufHead->wFIFOCtl |= FIFOCTL_11B;
        pTxDataHead->wDuration = cpu_to_le16((WORD)s_uGetDataDuration(pDevice, DATADUR_B, cbFrameSize, byPktType,
                                                          wCurrentRate, FALSE, 0, 0, 1, AUTO_FB_NONE));
    }

    BBvCaculateParameter(pDevice, cbFrameSize, wCurrentRate, byPktType,
        (PWORD)&(wLen), (PBYTE)&(pTxDataHead->byServiceField), (PBYTE)&(pTxDataHead->bySignalField)
    );
    pTxDataHead->wTransmitLength = cpu_to_le16(wLen);
    
    pTxDataHead->wTimeStampOff = cpu_to_le16(wTimeStampOff[pDevice->byPreambleType%2][wCurrentRate%MAX_RATE]);
    cbHeaderSize = wTxBufSize + sizeof(STxDataHead_ab);

   
    pMACHeader = (PS802_11Header)(pbyBuffer + cbHeaderSize);
    memcpy(pMACHeader, pPacket->p80211Header, pPacket->cbMPDULen);

    pMACHeader->wDurationID = 0;
    pMACHeader->wSeqCtl = cpu_to_le16(pDevice->wSeqCounter << 4);
    pDevice->wSeqCounter++ ;
    if (pDevice->wSeqCounter > 0x0fff)
        pDevice->wSeqCounter = 0;

    
    pDevice->wBCNBufLen = pPacket->cbMPDULen + cbHeaderSize;

    MACvSetCurrBCNTxDescAddr(pDevice->PortOffset, (pDevice->tx_beacon_dma));

    MACvSetCurrBCNLength(pDevice->PortOffset, pDevice->wBCNBufLen);
    
    MACvRegBitsOn(pDevice->PortOffset, MAC_REG_TCR, TCR_AUTOBCNTX);
    
    MACvTransmitBCN(pDevice->PortOffset);

    return CMD_STATUS_PENDING;
}



UINT
cbGetFragCount (
    IN  PSDevice         pDevice,
    IN  PSKeyItem        pTransmitKey,
    IN  UINT             cbFrameBodySize,
    IN  PSEthernetHeader psEthHeader
    )
{
    UINT           cbMACHdLen;
    UINT           cbFrameSize;
    UINT           cbFragmentSize; 
    UINT           cbFragPayloadSize;
    UINT           cbLastFragPayloadSize;
    UINT           cbIVlen = 0;
    UINT           cbICVlen = 0;
    UINT           cbMIClen = 0;
    UINT           cbFCSlen = 4;
    UINT           uMACfragNum = 1;
    BOOL           bNeedACK;



    if ((pDevice->eOPMode == OP_MODE_ADHOC) ||
        (pDevice->eOPMode == OP_MODE_AP)) {
        if (IS_MULTICAST_ADDRESS(&(psEthHeader->abyDstAddr[0])) ||
            IS_BROADCAST_ADDRESS(&(psEthHeader->abyDstAddr[0]))) {
            bNeedACK = FALSE;
        }
        else {
            bNeedACK = TRUE;
        }
    }
    else {
        
        bNeedACK = TRUE;
    }

    if (pDevice->bLongHeader)
        cbMACHdLen = WLAN_HDR_ADDR3_LEN + 6;
    else
        cbMACHdLen = WLAN_HDR_ADDR3_LEN;


    if (pDevice->bEncryptionEnable == TRUE) {

        if (pTransmitKey == NULL) {
            if ((pDevice->eEncryptionStatus == Ndis802_11Encryption1Enabled) ||
                (pDevice->pMgmt->eAuthenMode < WMAC_AUTH_WPA)) {
                cbIVlen = 4;
                cbICVlen = 4;
            } else if (pDevice->eEncryptionStatus == Ndis802_11Encryption2Enabled) {
                cbIVlen = 8;
                cbMIClen = 8;
                cbICVlen = 4;
            } else if (pDevice->eEncryptionStatus == Ndis802_11Encryption3Enabled) {
                cbIVlen = 8;
                cbICVlen = 8;
            }
        } else if (pTransmitKey->byCipherSuite == KEY_CTL_WEP) {
            cbIVlen = 4;
            cbICVlen = 4;
        } else if (pTransmitKey->byCipherSuite == KEY_CTL_TKIP) {
            cbIVlen = 8;
            cbMIClen = 8;
            cbICVlen = 4;
        } else if (pTransmitKey->byCipherSuite == KEY_CTL_CCMP) {
            cbIVlen = 8;
            cbICVlen = 8;
        }
    }

    cbFrameSize = cbMACHdLen + cbIVlen + (cbFrameBodySize + cbMIClen) + cbICVlen + cbFCSlen;

    if ((cbFrameSize > pDevice->wFragmentationThreshold) && (bNeedACK == TRUE)) {
        
        cbFragmentSize = pDevice->wFragmentationThreshold;
        cbFragPayloadSize = cbFragmentSize - cbMACHdLen - cbIVlen - cbICVlen - cbFCSlen;
        uMACfragNum = (WORD) ((cbFrameBodySize + cbMIClen) / cbFragPayloadSize);
        cbLastFragPayloadSize = (cbFrameBodySize + cbMIClen) % cbFragPayloadSize;
        if (cbLastFragPayloadSize == 0) {
            cbLastFragPayloadSize = cbFragPayloadSize;
        } else {
            uMACfragNum++;
        }
    }
    return uMACfragNum;
}


VOID
vDMA0_tx_80211(PSDevice  pDevice, struct sk_buff *skb, PBYTE pbMPDU, UINT cbMPDULen) {

    PSTxDesc        pFrstTD;
    BYTE            byPktType;
    PBYTE           pbyTxBufferAddr;
    PVOID           pvRTS;
    PVOID           pvCTS;
    PVOID           pvTxDataHd;
    UINT            uDuration;
    UINT            cbReqCount;
    PS802_11Header  pMACHeader;
    UINT            cbHeaderSize;
    UINT            cbFrameBodySize;
    BOOL            bNeedACK;
    BOOL            bIsPSPOLL = FALSE;
    PSTxBufHead     pTxBufHead;
    UINT            cbFrameSize;
    UINT            cbIVlen = 0;
    UINT            cbICVlen = 0;
    UINT            cbMIClen = 0;
    UINT            cbFCSlen = 4;
    UINT            uPadding = 0;
    UINT            cbMICHDR = 0;
    UINT            uLength = 0;
    DWORD           dwMICKey0, dwMICKey1;
    DWORD           dwMIC_Priority;
    PDWORD          pdwMIC_L;
    PDWORD          pdwMIC_R;
    WORD            wTxBufSize;
    UINT            cbMacHdLen;
    SEthernetHeader sEthHeader;
    PVOID           pvRrvTime;
    PVOID           pMICHDR;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    WORD            wCurrentRate = RATE_1M;
    PUWLAN_80211HDR  p80211Header;
    UINT             uNodeIndex = 0;
    BOOL            bNodeExist = FALSE;
    SKeyItem        STempKey;
    PSKeyItem       pTransmitKey = NULL;
    PBYTE           pbyIVHead;
    PBYTE           pbyPayloadHead;
    PBYTE           pbyMacHdr;

    UINT            cbExtSuppRate = 0;



    pvRrvTime = pMICHDR = pvRTS = pvCTS = pvTxDataHd = NULL;

    if(cbMPDULen <= WLAN_HDR_ADDR3_LEN) {
       cbFrameBodySize = 0;
    }
    else {
       cbFrameBodySize = cbMPDULen - WLAN_HDR_ADDR3_LEN;
    }
    p80211Header = (PUWLAN_80211HDR)pbMPDU;


    pFrstTD = pDevice->apCurrTD[TYPE_TXDMA0];
    pbyTxBufferAddr = (PBYTE)pFrstTD->pTDInfo->buf;
    pTxBufHead = (PSTxBufHead) pbyTxBufferAddr;
    wTxBufSize = sizeof(STxBufHead);
    memset(pTxBufHead, 0, wTxBufSize);

    if (pDevice->eCurrentPHYType == PHY_TYPE_11A) {
        wCurrentRate = RATE_6M;
        byPktType = PK_TYPE_11A;
    } else {
        wCurrentRate = RATE_1M;
        byPktType = PK_TYPE_11B;
    }

    
    
    
    
    if (pDevice->pMgmt->eScanState != WMAC_NO_SCANNING) {
        RFbSetPower(pDevice, wCurrentRate, pDevice->byCurrentCh);
    } else {
        RFbSetPower(pDevice, wCurrentRate, pMgmt->uCurrChannel);
    }
    pTxBufHead->byTxPower = pDevice->byCurPwr;

    
    if (pDevice->byFOETuning) {
        if ((p80211Header->sA3.wFrameCtl & TYPE_DATE_NULL) == TYPE_DATE_NULL) {
            wCurrentRate = RATE_24M;
            byPktType = PK_TYPE_11GA;
        }
    }

    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"vDMA0_tx_80211: p80211Header->sA3.wFrameCtl = %x \n", p80211Header->sA3.wFrameCtl);

    
    if (byPktType == PK_TYPE_11A) {
        pTxBufHead->wFIFOCtl = 0;
    }
    else if (byPktType == PK_TYPE_11B) {
        pTxBufHead->wFIFOCtl |= FIFOCTL_11B;
    }
    else if (byPktType == PK_TYPE_11GB) {
        pTxBufHead->wFIFOCtl |= FIFOCTL_11GB;
    }
    else if (byPktType == PK_TYPE_11GA) {
        pTxBufHead->wFIFOCtl |= FIFOCTL_11GA;
    }

    pTxBufHead->wFIFOCtl |= FIFOCTL_TMOEN;
    pTxBufHead->wTimeStamp = cpu_to_le16(DEFAULT_MGN_LIFETIME_RES_64us);


    if (IS_MULTICAST_ADDRESS(&(p80211Header->sA3.abyAddr1[0])) ||
        IS_BROADCAST_ADDRESS(&(p80211Header->sA3.abyAddr1[0]))) {
        bNeedACK = FALSE;
        if (pDevice->bEnableHostWEP) {
            uNodeIndex = 0;
            bNodeExist = TRUE;
        };
    }
    else {
        if (pDevice->bEnableHostWEP) {
            if (BSSDBbIsSTAInNodeDB(pDevice->pMgmt, (PBYTE)(p80211Header->sA3.abyAddr1), &uNodeIndex))
                bNodeExist = TRUE;
        };
        bNeedACK = TRUE;
        pTxBufHead->wFIFOCtl |= FIFOCTL_NEEDACK;
    };

    if ((pMgmt->eCurrMode == WMAC_MODE_ESS_AP) ||
        (pMgmt->eCurrMode == WMAC_MODE_IBSS_STA) ) {

        pTxBufHead->wFIFOCtl |= FIFOCTL_LRETRY;
        
        

        
        
        
        
        
    }

    pTxBufHead->wFIFOCtl |= (FIFOCTL_GENINT | FIFOCTL_ISDMA0);

    if ((p80211Header->sA4.wFrameCtl & TYPE_SUBTYPE_MASK) == TYPE_CTL_PSPOLL) {
        bIsPSPOLL = TRUE;
        cbMacHdLen = WLAN_HDR_ADDR2_LEN;
    } else {
        cbMacHdLen = WLAN_HDR_ADDR3_LEN;
    }

    
    if (WLAN_GET_FC_FSTYPE(p80211Header->sA4.wFrameCtl) == WLAN_FSTYPE_ASSOCRESP) {

        if (((PWLAN_IE_SUPP_RATES)pMgmt->abyCurrSuppRates)->len != 0) {
            cbExtSuppRate += ((PWLAN_IE_SUPP_RATES)pMgmt->abyCurrSuppRates)->len + WLAN_IEHDR_LEN;
         }

        if (((PWLAN_IE_SUPP_RATES)pMgmt->abyCurrExtSuppRates)->len != 0) {
            cbExtSuppRate += ((PWLAN_IE_SUPP_RATES)pMgmt->abyCurrExtSuppRates)->len + WLAN_IEHDR_LEN;
         }

         if (cbExtSuppRate >0) {
            cbFrameBodySize = WLAN_ASSOCRESP_OFF_SUPP_RATES;
         }
    }


    
    pTxBufHead->wFragCtl |= cpu_to_le16((WORD)cbMacHdLen << 10);

    
    
    
    pDevice->bAES = FALSE;  


    if (WLAN_GET_FC_ISWEP(p80211Header->sA4.wFrameCtl) != 0) {
        if (pDevice->eEncryptionStatus == Ndis802_11Encryption1Enabled) {
            cbIVlen = 4;
            cbICVlen = 4;
    	    pTxBufHead->wFragCtl |= FRAGCTL_LEGACY;
        }
        else if (pDevice->eEncryptionStatus == Ndis802_11Encryption2Enabled) {
            cbIVlen = 8;
            cbMIClen = 8;
            cbICVlen = 4;
    	    pTxBufHead->wFragCtl |= FRAGCTL_TKIP;
    	    
            
            
        }
        else if (pDevice->eEncryptionStatus == Ndis802_11Encryption3Enabled) {
            cbIVlen = 8;
            cbICVlen = 8;
            cbMICHDR = sizeof(SMICHDRHead);
            pTxBufHead->wFragCtl |= FRAGCTL_AES;
            pDevice->bAES = TRUE;
        }
        
        uPadding = 4 - (cbMacHdLen%4);
        uPadding %= 4;
    }

    cbFrameSize = cbMacHdLen + cbFrameBodySize + cbIVlen + cbMIClen + cbICVlen + cbFCSlen + cbExtSuppRate;

    
    if (pDevice->bGrpAckPolicy == TRUE) {
        pTxBufHead->wFIFOCtl |=	FIFOCTL_GRPACK;
    }
    


    if (byPktType == PK_TYPE_11GB || byPktType == PK_TYPE_11GA) {

        pvRrvTime = (PSRrvTime_gCTS) (pbyTxBufferAddr + wTxBufSize);
        pMICHDR = (PSMICHDRHead) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gCTS));
        pvRTS = NULL;
        pvCTS = (PSCTS) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gCTS) + cbMICHDR);
        pvTxDataHd = (PSTxDataHead_g) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_gCTS) + cbMICHDR + sizeof(SCTS));
        cbHeaderSize = wTxBufSize + sizeof(SRrvTime_gCTS) + cbMICHDR + sizeof(SCTS) + sizeof(STxDataHead_g);

    }
    else {

        pvRrvTime = (PSRrvTime_ab) (pbyTxBufferAddr + wTxBufSize);
        pMICHDR = (PSMICHDRHead) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_ab));
        pvRTS = NULL;
        pvCTS = NULL;
        pvTxDataHd = (PSTxDataHead_ab) (pbyTxBufferAddr + wTxBufSize + sizeof(SRrvTime_ab) + cbMICHDR);
        cbHeaderSize = wTxBufSize + sizeof(SRrvTime_ab) + cbMICHDR + sizeof(STxDataHead_ab);

    }

    memset((PVOID)(pbyTxBufferAddr + wTxBufSize), 0, (cbHeaderSize - wTxBufSize));
    memcpy(&(sEthHeader.abyDstAddr[0]), &(p80211Header->sA3.abyAddr1[0]), U_ETHER_ADDR_LEN);
    memcpy(&(sEthHeader.abySrcAddr[0]), &(p80211Header->sA3.abyAddr2[0]), U_ETHER_ADDR_LEN);
    
    
    
    pTxBufHead->wFragCtl |= (WORD)FRAGCTL_NONFRAG;


    
    s_vGenerateTxParameter(pDevice, byPktType, pbyTxBufferAddr, pvRrvTime, pvRTS, pvCTS,
                           cbFrameSize, bNeedACK, TYPE_TXDMA0, &sEthHeader, wCurrentRate);

    
    uDuration = s_uFillDataHead(pDevice, byPktType, pvTxDataHd, cbFrameSize, TYPE_TXDMA0, bNeedACK,
                                0, 0, 1, AUTO_FB_NONE, wCurrentRate);

    pMACHeader = (PS802_11Header) (pbyTxBufferAddr + cbHeaderSize);

    cbReqCount = cbHeaderSize + cbMacHdLen + uPadding + cbIVlen + (cbFrameBodySize + cbMIClen) + cbExtSuppRate;

    pbyMacHdr = (PBYTE)(pbyTxBufferAddr + cbHeaderSize);
    pbyPayloadHead = (PBYTE)(pbyMacHdr + cbMacHdLen + uPadding + cbIVlen);
    pbyIVHead = (PBYTE)(pbyMacHdr + cbMacHdLen + uPadding);

    
    memcpy(pbyMacHdr, pbMPDU, cbMacHdLen);

    
    pMACHeader->wFrameCtl &= cpu_to_le16(0xfffc);
    memcpy(pbyPayloadHead, (pbMPDU + cbMacHdLen), cbFrameBodySize);

    
    if (WLAN_GET_FC_FSTYPE(p80211Header->sA4.wFrameCtl) == WLAN_FSTYPE_ASSOCRESP) {
        if (cbExtSuppRate != 0) {
            if (((PWLAN_IE_SUPP_RATES)pMgmt->abyCurrSuppRates)->len != 0)
                memcpy((pbyPayloadHead + cbFrameBodySize),
                        pMgmt->abyCurrSuppRates,
                        ((PWLAN_IE_SUPP_RATES)pMgmt->abyCurrSuppRates)->len + WLAN_IEHDR_LEN
                       );
             if (((PWLAN_IE_SUPP_RATES)pMgmt->abyCurrExtSuppRates)->len != 0)
                memcpy((pbyPayloadHead + cbFrameBodySize) + ((PWLAN_IE_SUPP_RATES)pMgmt->abyCurrSuppRates)->len + WLAN_IEHDR_LEN,
                        pMgmt->abyCurrExtSuppRates,
                        ((PWLAN_IE_SUPP_RATES)pMgmt->abyCurrExtSuppRates)->len + WLAN_IEHDR_LEN
                       );
         }
    }

    
    if (WLAN_GET_FC_ISWEP(p80211Header->sA4.wFrameCtl) != 0) {

        if (pDevice->bEnableHostWEP) {
            pTransmitKey = &STempKey;
            pTransmitKey->byCipherSuite = pMgmt->sNodeDBTable[uNodeIndex].byCipherSuite;
            pTransmitKey->dwKeyIndex = pMgmt->sNodeDBTable[uNodeIndex].dwKeyIndex;
            pTransmitKey->uKeyLength = pMgmt->sNodeDBTable[uNodeIndex].uWepKeyLength;
            pTransmitKey->dwTSC47_16 = pMgmt->sNodeDBTable[uNodeIndex].dwTSC47_16;
            pTransmitKey->wTSC15_0 = pMgmt->sNodeDBTable[uNodeIndex].wTSC15_0;
            memcpy(pTransmitKey->abyKey,
                &pMgmt->sNodeDBTable[uNodeIndex].abyWepKey[0],
                pTransmitKey->uKeyLength
                );
        }

        if ((pTransmitKey != NULL) && (pTransmitKey->byCipherSuite == KEY_CTL_TKIP)) {

            dwMICKey0 = *(PDWORD)(&pTransmitKey->abyKey[16]);
            dwMICKey1 = *(PDWORD)(&pTransmitKey->abyKey[20]);

            
            MIC_vInit(dwMICKey0, dwMICKey1);
            MIC_vAppend((PBYTE)&(sEthHeader.abyDstAddr[0]), 12);
            dwMIC_Priority = 0;
            MIC_vAppend((PBYTE)&dwMIC_Priority, 4);
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"DMA0_tx_8021:MIC KEY: %lX, %lX\n", dwMICKey0, dwMICKey1);

            uLength = cbHeaderSize + cbMacHdLen + uPadding + cbIVlen;

            MIC_vAppend((pbyTxBufferAddr + uLength), cbFrameBodySize);

            pdwMIC_L = (PDWORD)(pbyTxBufferAddr + uLength + cbFrameBodySize);
            pdwMIC_R = (PDWORD)(pbyTxBufferAddr + uLength + cbFrameBodySize + 4);

            MIC_vGetMIC(pdwMIC_L, pdwMIC_R);
            MIC_vUnInit();

            if (pDevice->bTxMICFail == TRUE) {
                *pdwMIC_L = 0;
                *pdwMIC_R = 0;
                pDevice->bTxMICFail = FALSE;
            }

            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"uLength: %d, %d\n", uLength, cbFrameBodySize);
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"cbReqCount:%d, %d, %d, %d\n", cbReqCount, cbHeaderSize, uPadding, cbIVlen);
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"MIC:%lx, %lx\n", *pdwMIC_L, *pdwMIC_R);

        }


        s_vFillTxKey(pDevice, (PBYTE)(pTxBufHead->adwTxKey), pbyIVHead, pTransmitKey,
                     pbyMacHdr, (WORD)cbFrameBodySize, (PBYTE)pMICHDR);

        if (pDevice->bEnableHostWEP) {
            pMgmt->sNodeDBTable[uNodeIndex].dwTSC47_16 = pTransmitKey->dwTSC47_16;
            pMgmt->sNodeDBTable[uNodeIndex].wTSC15_0 = pTransmitKey->wTSC15_0;
        }

        if ((pDevice->byLocalID <= REV_ID_VT3253_A1)) {
            s_vSWencryption(pDevice, pTransmitKey, pbyPayloadHead, (WORD)(cbFrameBodySize + cbMIClen));
        }
    }

    pMACHeader->wSeqCtl = cpu_to_le16(pDevice->wSeqCounter << 4);
    pDevice->wSeqCounter++ ;
    if (pDevice->wSeqCounter > 0x0fff)
        pDevice->wSeqCounter = 0;


    if (bIsPSPOLL) {
        
        
        
        
        
        if (byPktType == PK_TYPE_11GB || byPktType == PK_TYPE_11GA) {
            ((PSTxDataHead_g)pvTxDataHd)->wDuration_a = cpu_to_le16(p80211Header->sA2.wDurationID);
            ((PSTxDataHead_g)pvTxDataHd)->wDuration_b = cpu_to_le16(p80211Header->sA2.wDurationID);
        } else {
            ((PSTxDataHead_ab)pvTxDataHd)->wDuration = cpu_to_le16(p80211Header->sA2.wDurationID);
        }
    }


    
    
    pFrstTD->pTDInfo->skb = skb;
    pFrstTD->m_td1TD1.byTCR = (TCR_STP | TCR_EDP | EDMSDU);
    pFrstTD->pTDInfo->skb_dma = pFrstTD->pTDInfo->buf_dma;
    pFrstTD->m_td1TD1.wReqCount = cpu_to_le16(cbReqCount);
    pFrstTD->buff_addr = cpu_to_le32(pFrstTD->pTDInfo->skb_dma);
    pFrstTD->pTDInfo->byFlags = 0;
    pFrstTD->pTDInfo->byFlags |= TD_FLAGS_PRIV_SKB;

    if (MACbIsRegBitsOn(pDevice->PortOffset, MAC_REG_PSCTL, PSCTL_PS)) {
        
        MACbPSWakeup(pDevice->PortOffset);
    }
    pDevice->bPWBitOn = FALSE;

    wmb();
    pFrstTD->m_td0TD0.f1Owner = OWNED_BY_NIC;
    wmb();

    pDevice->iTDUsed[TYPE_TXDMA0]++;

    if (AVAIL_TD(pDevice, TYPE_TXDMA0) <= 1) {
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO " available td0 <= 1\n");
    }

    pDevice->apCurrTD[TYPE_TXDMA0] = pFrstTD->next;

    
    MACvTransmit0(pDevice->PortOffset);

    return;
}


