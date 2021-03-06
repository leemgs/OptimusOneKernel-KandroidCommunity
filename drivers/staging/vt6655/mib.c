

#include "upc.h"
#include "mac.h"
#include "tether.h"
#include "mib.h"
#include "wctl.h"
#include "baseband.h"


static int          msglevel                =MSG_LEVEL_INFO;













void STAvClearAllCounter (PSStatCounter pStatistic)
{
    
	memset(pStatistic, 0, sizeof(SStatCounter));
}



void STAvUpdateIsrStatCounter (PSStatCounter pStatistic, DWORD dwIsr)
{
    
    
    
    

    if (dwIsr == 0) {
        pStatistic->ISRStat.dwIsrUnknown++;
        return;
    }


    if (dwIsr & ISR_TXDMA0)               
        pStatistic->ISRStat.dwIsrTx0OK++;             

    if (dwIsr & ISR_AC0DMA)               
        pStatistic->ISRStat.dwIsrAC0TxOK++;           

    if (dwIsr & ISR_BNTX)                 
        pStatistic->ISRStat.dwIsrBeaconTxOK++;        

    if (dwIsr & ISR_RXDMA0)               
        pStatistic->ISRStat.dwIsrRx0OK++;             

    if (dwIsr & ISR_TBTT)                 
        pStatistic->ISRStat.dwIsrTBTTInt++;           

    if (dwIsr & ISR_SOFTTIMER)            
        pStatistic->ISRStat.dwIsrSTIMERInt++;

    if (dwIsr & ISR_WATCHDOG)             
        pStatistic->ISRStat.dwIsrWatchDog++;

    if (dwIsr & ISR_FETALERR)             
        pStatistic->ISRStat.dwIsrUnrecoverableError++;

    if (dwIsr & ISR_SOFTINT)              
        pStatistic->ISRStat.dwIsrSoftInterrupt++;     

    if (dwIsr & ISR_MIBNEARFULL)          
        pStatistic->ISRStat.dwIsrMIBNearfull++;

    if (dwIsr & ISR_RXNOBUF)              
        pStatistic->ISRStat.dwIsrRxNoBuf++;           

    if (dwIsr & ISR_RXDMA1)               
        pStatistic->ISRStat.dwIsrRx1OK++;             
















    if (dwIsr & ISR_SOFTTIMER1)           
        pStatistic->ISRStat.dwIsrSTIMER1Int++;

}



void STAvUpdateRDStatCounter (PSStatCounter pStatistic,
                              BYTE byRSR, BYTE byNewRSR, BYTE byRxRate,
                              PBYTE pbyBuffer, UINT cbFrameLength)
{
    
    PS802_11Header pHeader = (PS802_11Header)pbyBuffer;

    if (byRSR & RSR_ADDROK)
        pStatistic->dwRsrADDROk++;
    if (byRSR & RSR_CRCOK) {
        pStatistic->dwRsrCRCOk++;

        pStatistic->ullRsrOK++;

        if (cbFrameLength >= U_ETHER_ADDR_LEN) {
            
            if (byRSR & RSR_ADDRBROAD) {
                pStatistic->ullRxBroadcastFrames++;
                pStatistic->ullRxBroadcastBytes += (ULONGLONG)cbFrameLength;
            }
            else if (byRSR & RSR_ADDRMULTI) {
                pStatistic->ullRxMulticastFrames++;
                pStatistic->ullRxMulticastBytes += (ULONGLONG)cbFrameLength;
            }
            else {
                pStatistic->ullRxDirectedFrames++;
                pStatistic->ullRxDirectedBytes += (ULONGLONG)cbFrameLength;
            }
        }
    }

    if(byRxRate==22) {
        pStatistic->CustomStat.ullRsr11M++;
        if(byRSR & RSR_CRCOK) {
            pStatistic->CustomStat.ullRsr11MCRCOk++;
        }
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"11M: ALL[%d], OK[%d]:[%02x]\n", (INT)pStatistic->CustomStat.ullRsr11M, (INT)pStatistic->CustomStat.ullRsr11MCRCOk, byRSR);
    }
    else if(byRxRate==11) {
        pStatistic->CustomStat.ullRsr5M++;
        if(byRSR & RSR_CRCOK) {
            pStatistic->CustomStat.ullRsr5MCRCOk++;
        }
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO" 5M: ALL[%d], OK[%d]:[%02x]\n", (INT)pStatistic->CustomStat.ullRsr5M, (INT)pStatistic->CustomStat.ullRsr5MCRCOk, byRSR);
    }
    else if(byRxRate==4) {
        pStatistic->CustomStat.ullRsr2M++;
        if(byRSR & RSR_CRCOK) {
            pStatistic->CustomStat.ullRsr2MCRCOk++;
        }
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO" 2M: ALL[%d], OK[%d]:[%02x]\n", (INT)pStatistic->CustomStat.ullRsr2M, (INT)pStatistic->CustomStat.ullRsr2MCRCOk, byRSR);
    }
    else if(byRxRate==2){
        pStatistic->CustomStat.ullRsr1M++;
        if(byRSR & RSR_CRCOK) {
            pStatistic->CustomStat.ullRsr1MCRCOk++;
        }
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO" 1M: ALL[%d], OK[%d]:[%02x]\n", (INT)pStatistic->CustomStat.ullRsr1M, (INT)pStatistic->CustomStat.ullRsr1MCRCOk, byRSR);
    }
    else if(byRxRate==12){
        pStatistic->CustomStat.ullRsr6M++;
        if(byRSR & RSR_CRCOK) {
            pStatistic->CustomStat.ullRsr6MCRCOk++;
        }
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO" 6M: ALL[%d], OK[%d]\n", (INT)pStatistic->CustomStat.ullRsr6M, (INT)pStatistic->CustomStat.ullRsr6MCRCOk);
    }
    else if(byRxRate==18){
        pStatistic->CustomStat.ullRsr9M++;
        if(byRSR & RSR_CRCOK) {
            pStatistic->CustomStat.ullRsr9MCRCOk++;
        }
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO" 9M: ALL[%d], OK[%d]\n", (INT)pStatistic->CustomStat.ullRsr9M, (INT)pStatistic->CustomStat.ullRsr9MCRCOk);
    }
    else if(byRxRate==24){
        pStatistic->CustomStat.ullRsr12M++;
        if(byRSR & RSR_CRCOK) {
            pStatistic->CustomStat.ullRsr12MCRCOk++;
        }
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"12M: ALL[%d], OK[%d]\n", (INT)pStatistic->CustomStat.ullRsr12M, (INT)pStatistic->CustomStat.ullRsr12MCRCOk);
    }
    else if(byRxRate==36){
        pStatistic->CustomStat.ullRsr18M++;
        if(byRSR & RSR_CRCOK) {
            pStatistic->CustomStat.ullRsr18MCRCOk++;
        }
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"18M: ALL[%d], OK[%d]\n", (INT)pStatistic->CustomStat.ullRsr18M, (INT)pStatistic->CustomStat.ullRsr18MCRCOk);
    }
    else if(byRxRate==48){
        pStatistic->CustomStat.ullRsr24M++;
        if(byRSR & RSR_CRCOK) {
            pStatistic->CustomStat.ullRsr24MCRCOk++;
        }
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"24M: ALL[%d], OK[%d]\n", (INT)pStatistic->CustomStat.ullRsr24M, (INT)pStatistic->CustomStat.ullRsr24MCRCOk);
    }
    else if(byRxRate==72){
        pStatistic->CustomStat.ullRsr36M++;
        if(byRSR & RSR_CRCOK) {
            pStatistic->CustomStat.ullRsr36MCRCOk++;
        }
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"36M: ALL[%d], OK[%d]\n", (INT)pStatistic->CustomStat.ullRsr36M, (INT)pStatistic->CustomStat.ullRsr36MCRCOk);
    }
    else if(byRxRate==96){
        pStatistic->CustomStat.ullRsr48M++;
        if(byRSR & RSR_CRCOK) {
            pStatistic->CustomStat.ullRsr48MCRCOk++;
        }
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"48M: ALL[%d], OK[%d]\n", (INT)pStatistic->CustomStat.ullRsr48M, (INT)pStatistic->CustomStat.ullRsr48MCRCOk);
    }
    else if(byRxRate==108){
        pStatistic->CustomStat.ullRsr54M++;
        if(byRSR & RSR_CRCOK) {
            pStatistic->CustomStat.ullRsr54MCRCOk++;
        }
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"54M: ALL[%d], OK[%d]\n", (INT)pStatistic->CustomStat.ullRsr54M, (INT)pStatistic->CustomStat.ullRsr54MCRCOk);
    }
    else {
    	DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"Unknown: Total[%d], CRCOK[%d]\n", (INT)pStatistic->dwRsrRxPacket+1, (INT)pStatistic->dwRsrCRCOk);
    }

    if (byRSR & RSR_BSSIDOK)
        pStatistic->dwRsrBSSIDOk++;

    if (byRSR & RSR_BCNSSIDOK)
        pStatistic->dwRsrBCNSSIDOk++;
    if (byRSR & RSR_IVLDLEN)  
        pStatistic->dwRsrLENErr++;
    if (byRSR & RSR_IVLDTYP)  
        pStatistic->dwRsrTYPErr++;
    if (byRSR & (RSR_IVLDTYP | RSR_IVLDLEN))
        pStatistic->dwRsrErr++;

    if (byNewRSR & NEWRSR_DECRYPTOK)
        pStatistic->dwNewRsrDECRYPTOK++;
    if (byNewRSR & NEWRSR_CFPIND)
        pStatistic->dwNewRsrCFP++;
    if (byNewRSR & NEWRSR_HWUTSF)
        pStatistic->dwNewRsrUTSF++;
    if (byNewRSR & NEWRSR_BCNHITAID)
        pStatistic->dwNewRsrHITAID++;
    if (byNewRSR & NEWRSR_BCNHITAID0)
        pStatistic->dwNewRsrHITAID0++;

    
    pStatistic->dwRsrRxPacket++;
    pStatistic->dwRsrRxOctet += cbFrameLength;


    if (IS_TYPE_DATA(pbyBuffer)) {
        pStatistic->dwRsrRxData++;
    } else if (IS_TYPE_MGMT(pbyBuffer)){
        pStatistic->dwRsrRxManage++;
    } else if (IS_TYPE_CONTROL(pbyBuffer)){
        pStatistic->dwRsrRxControl++;
    }

    if (byRSR & RSR_ADDRBROAD)
        pStatistic->dwRsrBroadcast++;
    else if (byRSR & RSR_ADDRMULTI)
        pStatistic->dwRsrMulticast++;
    else
        pStatistic->dwRsrDirected++;

    if (WLAN_GET_FC_MOREFRAG(pHeader->wFrameCtl))
        pStatistic->dwRsrRxFragment++;

    if (cbFrameLength < MIN_PACKET_LEN + 4) {
        pStatistic->dwRsrRunt++;
    }
    else if (cbFrameLength == MIN_PACKET_LEN + 4) {
        pStatistic->dwRsrRxFrmLen64++;
    }
    else if ((65 <= cbFrameLength) && (cbFrameLength <= 127)) {
        pStatistic->dwRsrRxFrmLen65_127++;
    }
    else if ((128 <= cbFrameLength) && (cbFrameLength <= 255)) {
        pStatistic->dwRsrRxFrmLen128_255++;
    }
    else if ((256 <= cbFrameLength) && (cbFrameLength <= 511)) {
        pStatistic->dwRsrRxFrmLen256_511++;
    }
    else if ((512 <= cbFrameLength) && (cbFrameLength <= 1023)) {
        pStatistic->dwRsrRxFrmLen512_1023++;
    }
    else if ((1024 <= cbFrameLength) && (cbFrameLength <= MAX_PACKET_LEN + 4)) {
        pStatistic->dwRsrRxFrmLen1024_1518++;
    } else if (cbFrameLength > MAX_PACKET_LEN + 4) {
        pStatistic->dwRsrLong++;
    }

}





void
STAvUpdateRDStatCounterEx (
    PSStatCounter   pStatistic,
    BYTE            byRSR,
    BYTE            byNewRSR,
    BYTE            byRxRate,
    PBYTE           pbyBuffer,
    UINT            cbFrameLength
    )
{
    STAvUpdateRDStatCounter(
                    pStatistic,
                    byRSR,
                    byNewRSR,
                    byRxRate,
                    pbyBuffer,
                    cbFrameLength
                    );

    
    pStatistic->dwCntRxFrmLength = cbFrameLength;
    
    memcpy(pStatistic->abyCntRxPattern, (PBYTE)pbyBuffer, 10);
}



void
STAvUpdateTDStatCounter (
    PSStatCounter   pStatistic,
    BYTE            byTSR0,
    BYTE            byTSR1,
    PBYTE           pbyBuffer,
    UINT            cbFrameLength,
    UINT            uIdx
    )
{
    PWLAN_80211HDR_A4   pHeader;
    PBYTE               pbyDestAddr;
    BYTE                byTSR0_NCR = byTSR0 & TSR0_NCR;



    pHeader = (PWLAN_80211HDR_A4) pbyBuffer;
    if (WLAN_GET_FC_TODS(pHeader->wFrameCtl) == 0) {
        pbyDestAddr = &(pHeader->abyAddr1[0]);
    }
    else {
        pbyDestAddr = &(pHeader->abyAddr3[0]);
    }
    
    pStatistic->dwTsrTxPacket[uIdx]++;
    pStatistic->dwTsrTxOctet[uIdx] += cbFrameLength;

    if (byTSR0_NCR != 0) {
        pStatistic->dwTsrRetry[uIdx]++;
        pStatistic->dwTsrTotalRetry[uIdx] += byTSR0_NCR;

        if (byTSR0_NCR == 1)
            pStatistic->dwTsrOnceRetry[uIdx]++;
        else
            pStatistic->dwTsrMoreThanOnceRetry[uIdx]++;
    }

    if ((byTSR1&(TSR1_TERR|TSR1_RETRYTMO|TSR1_TMO|ACK_DATA)) == 0) {
        pStatistic->ullTsrOK[uIdx]++;
        pStatistic->CustomStat.ullTsrAllOK =
            (pStatistic->ullTsrOK[TYPE_AC0DMA] + pStatistic->ullTsrOK[TYPE_TXDMA0]);
        
        if (IS_BROADCAST_ADDRESS(pbyDestAddr)) {
            pStatistic->ullTxBroadcastFrames[uIdx]++;
            pStatistic->ullTxBroadcastBytes[uIdx] += (ULONGLONG)cbFrameLength;
        }
        else if (IS_MULTICAST_ADDRESS(pbyDestAddr)) {
            pStatistic->ullTxMulticastFrames[uIdx]++;
            pStatistic->ullTxMulticastBytes[uIdx] += (ULONGLONG)cbFrameLength;
        }
        else {
            pStatistic->ullTxDirectedFrames[uIdx]++;
            pStatistic->ullTxDirectedBytes[uIdx] += (ULONGLONG)cbFrameLength;
        }
    }
    else {
        if (byTSR1 & TSR1_TERR)
            pStatistic->dwTsrErr[uIdx]++;
        if (byTSR1 & TSR1_RETRYTMO)
            pStatistic->dwTsrRetryTimeout[uIdx]++;
        if (byTSR1 & TSR1_TMO)
            pStatistic->dwTsrTransmitTimeout[uIdx]++;
        if (byTSR1 & ACK_DATA)
            pStatistic->dwTsrACKData[uIdx]++;
    }

    if (IS_BROADCAST_ADDRESS(pbyDestAddr))
        pStatistic->dwTsrBroadcast[uIdx]++;
    else if (IS_MULTICAST_ADDRESS(pbyDestAddr))
        pStatistic->dwTsrMulticast[uIdx]++;
    else
        pStatistic->dwTsrDirected[uIdx]++;

}



void
STAvUpdateTDStatCounterEx (
    PSStatCounter   pStatistic,
    PBYTE           pbyBuffer,
    DWORD           cbFrameLength
    )
{
    UINT    uPktLength;

    uPktLength = (UINT)cbFrameLength;

    
    pStatistic->dwCntTxBufLength = uPktLength;
    
    memcpy(pStatistic->abyCntTxPattern, pbyBuffer, 16);
}



void
STAvUpdate802_11Counter(
    PSDot11Counters         p802_11Counter,
    PSStatCounter           pStatistic,
    DWORD                   dwCounter
    )
{
    
    p802_11Counter->MulticastTransmittedFrameCount = (ULONGLONG) (pStatistic->dwTsrBroadcast[TYPE_AC0DMA] +
                                                                  pStatistic->dwTsrBroadcast[TYPE_TXDMA0] +
                                                                  pStatistic->dwTsrMulticast[TYPE_AC0DMA] +
                                                                  pStatistic->dwTsrMulticast[TYPE_TXDMA0]);
    p802_11Counter->FailedCount = (ULONGLONG) (pStatistic->dwTsrErr[TYPE_AC0DMA] + pStatistic->dwTsrErr[TYPE_TXDMA0]);
    p802_11Counter->RetryCount = (ULONGLONG) (pStatistic->dwTsrRetry[TYPE_AC0DMA] + pStatistic->dwTsrRetry[TYPE_TXDMA0]);
    p802_11Counter->MultipleRetryCount = (ULONGLONG) (pStatistic->dwTsrMoreThanOnceRetry[TYPE_AC0DMA] +
                                                          pStatistic->dwTsrMoreThanOnceRetry[TYPE_TXDMA0]);
    
    p802_11Counter->RTSSuccessCount += (ULONGLONG)  (dwCounter & 0x000000ff);
    p802_11Counter->RTSFailureCount += (ULONGLONG) ((dwCounter & 0x0000ff00) >> 8);
    p802_11Counter->ACKFailureCount += (ULONGLONG) ((dwCounter & 0x00ff0000) >> 16);
    p802_11Counter->FCSErrorCount +=   (ULONGLONG) ((dwCounter & 0xff000000) >> 24);
    
    p802_11Counter->MulticastReceivedFrameCount = (ULONGLONG) (pStatistic->dwRsrBroadcast +
                                                               pStatistic->dwRsrMulticast);
}


void
STAvClear802_11Counter(PSDot11Counters p802_11Counter)
{
    
	memset(p802_11Counter, 0, sizeof(SDot11Counters));
}
