

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
#include "iowpa.h"
#include "aes_ccmp.h"










static int          msglevel                =MSG_LEVEL_INFO;

const BYTE acbyRxRate[MAX_RATE] =
{2, 4, 11, 22, 12, 18, 24, 36, 48, 72, 96, 108};








static BYTE s_byGetRateIdx(IN BYTE byRate);


static
VOID
s_vGetDASA(
    IN  PBYTE pbyRxBufferAddr,
    OUT PUINT pcbHeaderSize,
    OUT PSEthernetHeader psEthHeader
    );

static
VOID
s_vProcessRxMACHeader (
    IN  PSDevice pDevice,
    IN  PBYTE pbyRxBufferAddr,
    IN  UINT cbPacketSize,
    IN  BOOL bIsWEP,
    IN  BOOL bExtIV,
    OUT PUINT pcbHeadSize
    );

static BOOL s_bAPModeRxCtl(
    IN PSDevice pDevice,
    IN PBYTE    pbyFrame,
    IN INT      iSANodeIndex
    );



static BOOL s_bAPModeRxData (
    IN PSDevice pDevice,
    IN struct sk_buff* skb,
    IN UINT     FrameSize,
    IN UINT     cbHeaderOffset,
    IN INT      iSANodeIndex,
    IN INT      iDANodeIndex
    );


static BOOL s_bHandleRxEncryption(
    IN PSDevice     pDevice,
    IN PBYTE        pbyFrame,
    IN UINT         FrameSize,
    IN PBYTE        pbyRsr,
    OUT PBYTE       pbyNewRsr,
    OUT PSKeyItem   *pKeyOut,
    int *       pbExtIV,
    OUT PWORD       pwRxTSC15_0,
    OUT PDWORD      pdwRxTSC47_16
    );

static BOOL s_bHostWepRxEncryption(

    IN PSDevice     pDevice,
    IN PBYTE        pbyFrame,
    IN UINT         FrameSize,
    IN PBYTE        pbyRsr,
    IN BOOL         bOnFly,
    IN PSKeyItem    pKey,
    OUT PBYTE       pbyNewRsr,
    int *       pbExtIV,
    OUT PWORD       pwRxTSC15_0,
    OUT PDWORD      pdwRxTSC47_16

    );




static
VOID
s_vProcessRxMACHeader (
    IN  PSDevice pDevice,
    IN  PBYTE pbyRxBufferAddr,
    IN  UINT cbPacketSize,
    IN  BOOL bIsWEP,
    IN  BOOL bExtIV,
    OUT PUINT pcbHeadSize
    )
{
    PBYTE           pbyRxBuffer;
    UINT            cbHeaderSize = 0;
    PWORD           pwType;
    PS802_11Header  pMACHeader;
    int             ii;


    pMACHeader = (PS802_11Header) (pbyRxBufferAddr + cbHeaderSize);

    s_vGetDASA((PBYTE)pMACHeader, &cbHeaderSize, &pDevice->sRxEthHeader);

    if (bIsWEP) {
        if (bExtIV) {
            
            cbHeaderSize += (WLAN_HDR_ADDR3_LEN + 8);
        } else {
            
            cbHeaderSize += (WLAN_HDR_ADDR3_LEN + 4);
        }
    }
    else {
        cbHeaderSize += WLAN_HDR_ADDR3_LEN;
    };

    pbyRxBuffer = (PBYTE) (pbyRxBufferAddr + cbHeaderSize);
    if (IS_ETH_ADDRESS_EQUAL(pbyRxBuffer, &pDevice->abySNAP_Bridgetunnel[0])) {
        cbHeaderSize += 6;
    }
    else if (IS_ETH_ADDRESS_EQUAL(pbyRxBuffer, &pDevice->abySNAP_RFC1042[0])) {
        cbHeaderSize += 6;
        pwType = (PWORD) (pbyRxBufferAddr + cbHeaderSize);
        if ((*pwType!= TYPE_PKT_IPX) && (*pwType != cpu_to_le16(0xF380))) {
        }
        else {
            cbHeaderSize -= 8;
            pwType = (PWORD) (pbyRxBufferAddr + cbHeaderSize);
            if (bIsWEP) {
                if (bExtIV) {
                    *pwType = htons(cbPacketSize - WLAN_HDR_ADDR3_LEN - 8);    
                } else {
                    *pwType = htons(cbPacketSize - WLAN_HDR_ADDR3_LEN - 4);    
                }
            }
            else {
                *pwType = htons(cbPacketSize - WLAN_HDR_ADDR3_LEN);
            }
        }
    }
    else {
        cbHeaderSize -= 2;
        pwType = (PWORD) (pbyRxBufferAddr + cbHeaderSize);
        if (bIsWEP) {
            if (bExtIV) {
                *pwType = htons(cbPacketSize - WLAN_HDR_ADDR3_LEN - 8);    
            } else {
                *pwType = htons(cbPacketSize - WLAN_HDR_ADDR3_LEN - 4);    
            }
        }
        else {
            *pwType = htons(cbPacketSize - WLAN_HDR_ADDR3_LEN);
        }
    }

    cbHeaderSize -= (U_ETHER_ADDR_LEN * 2);
    pbyRxBuffer = (PBYTE) (pbyRxBufferAddr + cbHeaderSize);
    for(ii=0;ii<U_ETHER_ADDR_LEN;ii++)
        *pbyRxBuffer++ = pDevice->sRxEthHeader.abyDstAddr[ii];
    for(ii=0;ii<U_ETHER_ADDR_LEN;ii++)
        *pbyRxBuffer++ = pDevice->sRxEthHeader.abySrcAddr[ii];

    *pcbHeadSize = cbHeaderSize;
}




static BYTE s_byGetRateIdx (IN BYTE byRate)
{
    BYTE    byRateIdx;

    for (byRateIdx = 0; byRateIdx <MAX_RATE ; byRateIdx++) {
        if (acbyRxRate[byRateIdx%MAX_RATE] == byRate)
            return byRateIdx;
    }
    return 0;
}


static
VOID
s_vGetDASA (
    IN  PBYTE pbyRxBufferAddr,
    OUT PUINT pcbHeaderSize,
    OUT PSEthernetHeader psEthHeader
    )
{
    UINT            cbHeaderSize = 0;
    PS802_11Header  pMACHeader;
    int             ii;

    pMACHeader = (PS802_11Header) (pbyRxBufferAddr + cbHeaderSize);

    if ((pMACHeader->wFrameCtl & FC_TODS) == 0) {
        if (pMACHeader->wFrameCtl & FC_FROMDS) {
            for(ii=0;ii<U_ETHER_ADDR_LEN;ii++) {
                psEthHeader->abyDstAddr[ii] = pMACHeader->abyAddr1[ii];
                psEthHeader->abySrcAddr[ii] = pMACHeader->abyAddr3[ii];
            }
        }
        else {
            
            for(ii=0;ii<U_ETHER_ADDR_LEN;ii++) {
                psEthHeader->abyDstAddr[ii] = pMACHeader->abyAddr1[ii];
                psEthHeader->abySrcAddr[ii] = pMACHeader->abyAddr2[ii];
            }
        }
    }
    else {
        
        if (pMACHeader->wFrameCtl & FC_FROMDS) {
            for(ii=0;ii<U_ETHER_ADDR_LEN;ii++) {
                psEthHeader->abyDstAddr[ii] = pMACHeader->abyAddr3[ii];
                psEthHeader->abySrcAddr[ii] = pMACHeader->abyAddr4[ii];
                cbHeaderSize += 6;
            }
        }
        else {
            for(ii=0;ii<U_ETHER_ADDR_LEN;ii++) {
                psEthHeader->abyDstAddr[ii] = pMACHeader->abyAddr3[ii];
                psEthHeader->abySrcAddr[ii] = pMACHeader->abyAddr2[ii];
            }
        }
    };
    *pcbHeaderSize = cbHeaderSize;
}






VOID	MngWorkItem(PVOID Context)
{
	PSRxMgmtPacket			pRxMgmtPacket;
	PSDevice	pDevice =  (PSDevice) Context;
	
	spin_lock_irq(&pDevice->lock);
	 while(pDevice->rxManeQueue.packet_num != 0)
	 {
		 pRxMgmtPacket =  DeQueue(pDevice);
        		vMgrRxManagePacket(pDevice, pDevice->pMgmt, pRxMgmtPacket);
	}
	spin_unlock_irq(&pDevice->lock);
}






BOOL
device_receive_frame (
    IN  PSDevice pDevice,
    IN  PSRxDesc pCurrRD
    )
{

    PDEVICE_RD_INFO  pRDInfo = pCurrRD->pRDInfo;
#ifdef	PLICE_DEBUG
	
#endif
    struct net_device_stats* pStats=&pDevice->stats;
    struct sk_buff* skb;
    PSMgmtObject    pMgmt = pDevice->pMgmt;
    PSRxMgmtPacket  pRxPacket = &(pDevice->pMgmt->sRxPacket);
    PS802_11Header  p802_11Header;
    PBYTE           pbyRsr;
    PBYTE           pbyNewRsr;
    PBYTE           pbyRSSI;
    PQWORD          pqwTSFTime;
    PWORD           pwFrameSize;
    PBYTE           pbyFrame;
    BOOL            bDeFragRx = FALSE;
    BOOL            bIsWEP = FALSE;
    UINT            cbHeaderOffset;
    UINT            FrameSize;
    WORD            wEtherType = 0;
    INT             iSANodeIndex = -1;
    INT             iDANodeIndex = -1;
    UINT            ii;
    UINT            cbIVOffset;
    BOOL            bExtIV = FALSE;
    PBYTE           pbyRxSts;
    PBYTE           pbyRxRate;
    PBYTE           pbySQ;
    UINT            cbHeaderSize;
    PSKeyItem       pKey = NULL;
    WORD            wRxTSC15_0 = 0;
    DWORD           dwRxTSC47_16 = 0;
    SKeyItem        STempKey;
    
    DWORD           dwDuration = 0;
    LONG            ldBm = 0;
    LONG            ldBmThreshold = 0;
    PS802_11Header pMACHeader;
 BOOL            bRxeapol_key = FALSE;



    skb = pRDInfo->skb;



#if 1
	pci_unmap_single(pDevice->pcid, pRDInfo->skb_dma,
                     pDevice->rx_buf_sz, PCI_DMA_FROMDEVICE);
#endif

    pwFrameSize = (PWORD)(skb->data + 2);
    FrameSize = cpu_to_le16(pCurrRD->m_rd1RD1.wReqCount) - cpu_to_le16(pCurrRD->m_rd0RD0.wResCount);

    
    
    if ((FrameSize > 2364)||(FrameSize <= 32)) {
        
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"---------- WRONG Length 1 \n");
        return FALSE;
    }

    pbyRxSts = (PBYTE) (skb->data);
    pbyRxRate = (PBYTE) (skb->data + 1);
    pbyRsr = (PBYTE) (skb->data + FrameSize - 1);
    pbyRSSI = (PBYTE) (skb->data + FrameSize - 2);
    pbyNewRsr = (PBYTE) (skb->data + FrameSize - 3);
    pbySQ = (PBYTE) (skb->data + FrameSize - 4);
    pqwTSFTime = (PQWORD) (skb->data + FrameSize - 12);
    pbyFrame = (PBYTE)(skb->data + 4);

    
    FrameSize = cpu_to_le16(*pwFrameSize);

    if ((FrameSize > 2346)|(FrameSize < 14)) { 
                                               
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"---------- WRONG Length 2 \n");
        return FALSE;
    }

#if 1
	
    STAvUpdateRDStatCounter(&pDevice->scStatistic,
                            *pbyRsr,
                            *pbyNewRsr,
                            *pbyRxRate,
                            pbyFrame,
                            FrameSize);

#endif

  pMACHeader=(PS802_11Header)((PBYTE) (skb->data)+8);

	if (pDevice->bMeasureInProgress == TRUE) {
        if ((*pbyRsr & RSR_CRCOK) != 0) {
            pDevice->byBasicMap |= 0x01;
        }
        dwDuration = (FrameSize << 4);
        dwDuration /= acbyRxRate[*pbyRxRate%MAX_RATE];
        if (*pbyRxRate <= RATE_11M) {
            if (*pbyRxSts & 0x01) {
                
                dwDuration += 192;
            } else {
                
                dwDuration += 96;
            }
        } else {
            dwDuration += 16;
        }
        RFvRSSITodBm(pDevice, *pbyRSSI, &ldBm);
        ldBmThreshold = -57;
        for (ii = 7; ii > 0;) {
            if (ldBm > ldBmThreshold) {
                break;
            }
            ldBmThreshold -= 5;
            ii--;
        }
        pDevice->dwRPIs[ii] += dwDuration;
        return FALSE;
    }

    if (!IS_MULTICAST_ADDRESS(pbyFrame) && !IS_BROADCAST_ADDRESS(pbyFrame)) {
        if (WCTLbIsDuplicate(&(pDevice->sDupRxCache), (PS802_11Header) (skb->data + 4))) {
            pDevice->s802_11Counter.FrameDuplicateCount++;
            return FALSE;
        }
    }


    
    s_vGetDASA(skb->data+4, &cbHeaderSize, &pDevice->sRxEthHeader);

    
    if (IS_ETH_ADDRESS_EQUAL((PBYTE)&(pDevice->sRxEthHeader.abySrcAddr[0]), pDevice->abyCurrentNetAddr))
        return FALSE;

    if ((pMgmt->eCurrMode == WMAC_MODE_ESS_AP) || (pMgmt->eCurrMode == WMAC_MODE_IBSS_STA)) {
        if (IS_CTL_PSPOLL(pbyFrame) || !IS_TYPE_CONTROL(pbyFrame)) {
            p802_11Header = (PS802_11Header) (pbyFrame);
            
            if (BSSDBbIsSTAInNodeDB(pMgmt, (PBYTE)(p802_11Header->abyAddr2), &iSANodeIndex)) {
                pMgmt->sNodeDBTable[iSANodeIndex].ulLastRxJiffer = jiffies;
                pMgmt->sNodeDBTable[iSANodeIndex].uInActiveCount = 0;
            }
        }
    }

    if (pMgmt->eCurrMode == WMAC_MODE_ESS_AP) {
        if (s_bAPModeRxCtl(pDevice, pbyFrame, iSANodeIndex) == TRUE) {
            return FALSE;
        }
    }


    if (IS_FC_WEP(pbyFrame)) {
        BOOL     bRxDecryOK = FALSE;

        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"rx WEP pkt\n");
        bIsWEP = TRUE;
        if ((pDevice->bEnableHostWEP) && (iSANodeIndex >= 0)) {
            pKey = &STempKey;
            pKey->byCipherSuite = pMgmt->sNodeDBTable[iSANodeIndex].byCipherSuite;
            pKey->dwKeyIndex = pMgmt->sNodeDBTable[iSANodeIndex].dwKeyIndex;
            pKey->uKeyLength = pMgmt->sNodeDBTable[iSANodeIndex].uWepKeyLength;
            pKey->dwTSC47_16 = pMgmt->sNodeDBTable[iSANodeIndex].dwTSC47_16;
            pKey->wTSC15_0 = pMgmt->sNodeDBTable[iSANodeIndex].wTSC15_0;
            memcpy(pKey->abyKey,
                &pMgmt->sNodeDBTable[iSANodeIndex].abyWepKey[0],
                pKey->uKeyLength
                );

            bRxDecryOK = s_bHostWepRxEncryption(pDevice,
                                                pbyFrame,
                                                FrameSize,
                                                pbyRsr,
                                                pMgmt->sNodeDBTable[iSANodeIndex].bOnFly,
                                                pKey,
                                                pbyNewRsr,
                                                &bExtIV,
                                                &wRxTSC15_0,
                                                &dwRxTSC47_16);
        } else {
            bRxDecryOK = s_bHandleRxEncryption(pDevice,
                                                pbyFrame,
                                                FrameSize,
                                                pbyRsr,
                                                pbyNewRsr,
                                                &pKey,
                                                &bExtIV,
                                                &wRxTSC15_0,
                                                &dwRxTSC47_16);
        }

        if (bRxDecryOK) {
            if ((*pbyNewRsr & NEWRSR_DECRYPTOK) == 0) {
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ICV Fail\n");
                if ( (pDevice->pMgmt->eAuthenMode == WMAC_AUTH_WPA) ||
                    (pDevice->pMgmt->eAuthenMode == WMAC_AUTH_WPAPSK) ||
                    (pDevice->pMgmt->eAuthenMode == WMAC_AUTH_WPANONE) ||
                    (pDevice->pMgmt->eAuthenMode == WMAC_AUTH_WPA2) ||
                    (pDevice->pMgmt->eAuthenMode == WMAC_AUTH_WPA2PSK)) {

                    if ((pKey != NULL) && (pKey->byCipherSuite == KEY_CTL_TKIP)) {
                        pDevice->s802_11Counter.TKIPICVErrors++;
                    } else if ((pKey != NULL) && (pKey->byCipherSuite == KEY_CTL_CCMP)) {
                        pDevice->s802_11Counter.CCMPDecryptErrors++;
                    } else if ((pKey != NULL) && (pKey->byCipherSuite == KEY_CTL_WEP)) {

                    }
                }
                return FALSE;
            }
        } else {
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"WEP Func Fail\n");
            return FALSE;
        }
        if ((pKey != NULL) && (pKey->byCipherSuite == KEY_CTL_CCMP))
            FrameSize -= 8;         
        else
            FrameSize -= 4;         
    }


    
    
    
    
    FrameSize -= U_CRC_LEN;

    if (( !(*pbyRsr & (RSR_ADDRBROAD | RSR_ADDRMULTI))) && 
        (IS_FRAGMENT_PKT((skb->data+4)))
        ) {
        
        bDeFragRx = WCTLbHandleFragment(pDevice, (PS802_11Header) (skb->data+4), FrameSize, bIsWEP, bExtIV);
        pDevice->s802_11Counter.ReceivedFragmentCount++;
        if (bDeFragRx) {
            
            skb = pDevice->sRxDFCB[pDevice->uCurrentDFCBIdx].skb;
            FrameSize = pDevice->sRxDFCB[pDevice->uCurrentDFCBIdx].cbFrameLength;

        }
        else {
            return FALSE;
        }
    }



    if ((IS_TYPE_DATA((skb->data+4))) == FALSE) {
        

        if (IS_TYPE_MGMT((skb->data+4))) {
            PBYTE pbyData1;
            PBYTE pbyData2;

            pRxPacket->p80211Header = (PUWLAN_80211HDR)(skb->data+4);
            pRxPacket->cbMPDULen = FrameSize;
            pRxPacket->uRSSI = *pbyRSSI;
            pRxPacket->bySQ = *pbySQ;
            HIDWORD(pRxPacket->qwLocalTSF) = cpu_to_le32(HIDWORD(*pqwTSFTime));
            LODWORD(pRxPacket->qwLocalTSF) = cpu_to_le32(LODWORD(*pqwTSFTime));
            if (bIsWEP) {
                
                pbyData1 = WLAN_HDR_A3_DATA_PTR(skb->data+4);
                pbyData2 = WLAN_HDR_A3_DATA_PTR(skb->data+4) + 4;
                for (ii = 0; ii < (FrameSize - 4); ii++) {
                    *pbyData1 = *pbyData2;
                     pbyData1++;
                     pbyData2++;
                }
            }
            pRxPacket->byRxRate = s_byGetRateIdx(*pbyRxRate);
            pRxPacket->byRxChannel = (*pbyRxSts) >> 2;



#ifdef	THREAD
		EnQueue(pDevice,pRxPacket);

		
		
			
#else

#ifdef	TASK_LET
		EnQueue(pDevice,pRxPacket);
		tasklet_schedule(&pDevice->RxMngWorkItem);
#else

	vMgrRxManagePacket((HANDLE)pDevice, pDevice->pMgmt, pRxPacket);
           
#endif

#endif

			
            
            if (pDevice->bEnableHostapd) {
	            skb->dev = pDevice->apdev;
	            skb->data += 4;
	            skb->tail += 4;
                     skb_put(skb, FrameSize);
		skb_reset_mac_header(skb);
	            skb->pkt_type = PACKET_OTHERHOST;
    	        skb->protocol = htons(ETH_P_802_2);
	            memset(skb->cb, 0, sizeof(skb->cb));
	            netif_rx(skb);
                return TRUE;
	        }
        }
        else {
            
        };
        return FALSE;
    }
    else {
        if (pMgmt->eCurrMode == WMAC_MODE_ESS_AP) {
            
            if ( !(*pbyRsr & RSR_BSSIDOK)) {
                if (bDeFragRx) {
                    if (!device_alloc_frag_buf(pDevice, &pDevice->sRxDFCB[pDevice->uCurrentDFCBIdx])) {
                        DBG_PRT(MSG_LEVEL_ERR,KERN_ERR "%s: can not alloc more frag bufs\n",
                        pDevice->dev->name);
                    }
                }
                return FALSE;
            }
        }
        else {
            
            if ((pDevice->bLinkPass == FALSE) ||
                !(*pbyRsr & RSR_BSSIDOK)) {
                if (bDeFragRx) {
                    if (!device_alloc_frag_buf(pDevice, &pDevice->sRxDFCB[pDevice->uCurrentDFCBIdx])) {
                        DBG_PRT(MSG_LEVEL_ERR,KERN_ERR "%s: can not alloc more frag bufs\n",
                        pDevice->dev->name);
                    }
                }
                return FALSE;
            }
   
   	  {
   	    BYTE  Protocol_Version;    
	    BYTE  Packet_Type;           
              if (bIsWEP)
                  cbIVOffset = 8;
              else
                  cbIVOffset = 0;
              wEtherType = (skb->data[cbIVOffset + 8 + 24 + 6] << 8) |
                          skb->data[cbIVOffset + 8 + 24 + 6 + 1];
	      Protocol_Version = skb->data[cbIVOffset + 8 + 24 + 6 + 1 +1];
	      Packet_Type = skb->data[cbIVOffset + 8 + 24 + 6 + 1 +1+1];
	     if (wEtherType == ETH_P_PAE) {         
                  if(((Protocol_Version==1) ||(Protocol_Version==2)) &&
		     (Packet_Type==3)) {  
                        bRxeapol_key = TRUE;
                  }
	      }
   	  }
    
        }
    }





    if (pDevice->bEnablePSMode) {
        if (IS_FC_MOREDATA((skb->data+4))) {
            if (*pbyRsr & RSR_ADDROK) {
                
            }
        }
        else {
            if (pDevice->pMgmt->bInTIMWake == TRUE) {
                pDevice->pMgmt->bInTIMWake = FALSE;
            }
        }
    };

    
    if (pDevice->bDiversityEnable && (FrameSize>50) &&
        (pDevice->eOPMode == OP_MODE_INFRASTRUCTURE) &&
        (pDevice->bLinkPass == TRUE)) {
	
		BBvAntennaDiversity(pDevice, s_byGetRateIdx(*pbyRxRate), 0);
    }


    if (pDevice->byLocalID != REV_ID_VT3253_B1) {
        pDevice->uCurrRSSI = *pbyRSSI;
    }
    pDevice->byCurrSQ = *pbySQ;

    if ((*pbyRSSI != 0) &&
        (pMgmt->pCurrBSS!=NULL)) {
        RFvRSSITodBm(pDevice, *pbyRSSI, &ldBm);
        
        pMgmt->pCurrBSS->byRSSIStatCnt++;
        pMgmt->pCurrBSS->byRSSIStatCnt %= RSSI_STAT_COUNT;
        pMgmt->pCurrBSS->ldBmAverage[pMgmt->pCurrBSS->byRSSIStatCnt] = ldBm;
        for(ii=0;ii<RSSI_STAT_COUNT;ii++) {
            if (pMgmt->pCurrBSS->ldBmAverage[ii] != 0) {
            pMgmt->pCurrBSS->ldBmMAX = max(pMgmt->pCurrBSS->ldBmAverage[ii], ldBm);
            }
        }
    }

    

    if ((pMgmt->eCurrMode == WMAC_MODE_ESS_AP) && (pDevice->bEnable8021x == TRUE)){
        BYTE    abyMacHdr[24];

        
        if (bIsWEP)
            cbIVOffset = 8;
        else
            cbIVOffset = 0;
        wEtherType = (skb->data[cbIVOffset + 4 + 24 + 6] << 8) |
                    skb->data[cbIVOffset + 4 + 24 + 6 + 1];

	    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"wEtherType = %04x \n", wEtherType);
        if (wEtherType == ETH_P_PAE) {
            skb->dev = pDevice->apdev;

            if (bIsWEP == TRUE) {
                
                memcpy(&abyMacHdr[0], (skb->data + 4), 24);
                memcpy((skb->data + 4 + cbIVOffset), &abyMacHdr[0], 24);
            }
            skb->data +=  (cbIVOffset + 4);
            skb->tail +=  (cbIVOffset + 4);
            skb_put(skb, FrameSize);
	    skb_reset_mac_header(skb);

	skb->pkt_type = PACKET_OTHERHOST;
            skb->protocol = htons(ETH_P_802_2);
            memset(skb->cb, 0, sizeof(skb->cb));
            netif_rx(skb);
            return TRUE;

}
        
        if (!(pMgmt->sNodeDBTable[iSANodeIndex].dwFlags & WLAN_STA_AUTHORIZED))
            return FALSE;
    }


    if ((pKey != NULL) && (pKey->byCipherSuite == KEY_CTL_TKIP)) {
        if (bIsWEP) {
            FrameSize -= 8;  
        }
    }

    
    
    if ((pKey != NULL) && (pKey->byCipherSuite == KEY_CTL_TKIP)) {
        if (bIsWEP) {
            PDWORD          pdwMIC_L;
            PDWORD          pdwMIC_R;
            DWORD           dwMIC_Priority;
            DWORD           dwMICKey0 = 0, dwMICKey1 = 0;
            DWORD           dwLocalMIC_L = 0;
            DWORD           dwLocalMIC_R = 0;
            viawget_wpa_header *wpahdr;


            if (pMgmt->eCurrMode == WMAC_MODE_ESS_AP) {
                dwMICKey0 = cpu_to_le32(*(PDWORD)(&pKey->abyKey[24]));
                dwMICKey1 = cpu_to_le32(*(PDWORD)(&pKey->abyKey[28]));
            }
            else {
                if (pDevice->pMgmt->eAuthenMode == WMAC_AUTH_WPANONE) {
                    dwMICKey0 = cpu_to_le32(*(PDWORD)(&pKey->abyKey[16]));
                    dwMICKey1 = cpu_to_le32(*(PDWORD)(&pKey->abyKey[20]));
                } else if ((pKey->dwKeyIndex & BIT28) == 0) {
                    dwMICKey0 = cpu_to_le32(*(PDWORD)(&pKey->abyKey[16]));
                    dwMICKey1 = cpu_to_le32(*(PDWORD)(&pKey->abyKey[20]));
                } else {
                    dwMICKey0 = cpu_to_le32(*(PDWORD)(&pKey->abyKey[24]));
                    dwMICKey1 = cpu_to_le32(*(PDWORD)(&pKey->abyKey[28]));
                }
            }

            MIC_vInit(dwMICKey0, dwMICKey1);
            MIC_vAppend((PBYTE)&(pDevice->sRxEthHeader.abyDstAddr[0]), 12);
            dwMIC_Priority = 0;
            MIC_vAppend((PBYTE)&dwMIC_Priority, 4);
            
            MIC_vAppend((PBYTE)(skb->data + 4 + WLAN_HDR_ADDR3_LEN + 8),
                        FrameSize - WLAN_HDR_ADDR3_LEN - 8);
            MIC_vGetMIC(&dwLocalMIC_L, &dwLocalMIC_R);
            MIC_vUnInit();

            pdwMIC_L = (PDWORD)(skb->data + 4 + FrameSize);
            pdwMIC_R = (PDWORD)(skb->data + 4 + FrameSize + 4);
            
            
            


            if ((cpu_to_le32(*pdwMIC_L) != dwLocalMIC_L) || (cpu_to_le32(*pdwMIC_R) != dwLocalMIC_R) ||
                (pDevice->bRxMICFail == TRUE)) {
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"MIC comparison is fail!\n");
                pDevice->bRxMICFail = FALSE;
                
                pDevice->s802_11Counter.TKIPLocalMICFailures++;
                if (bDeFragRx) {
                    if (!device_alloc_frag_buf(pDevice, &pDevice->sRxDFCB[pDevice->uCurrentDFCBIdx])) {
                        DBG_PRT(MSG_LEVEL_ERR,KERN_ERR "%s: can not alloc more frag bufs\n",
                            pDevice->dev->name);
                    }
                }
               
       #ifdef WPA_SUPPLICANT_DRIVER_WEXT_SUPPORT
				
				
				{
					union iwreq_data wrqu;
					struct iw_michaelmicfailure ev;
					int keyidx = pbyFrame[cbHeaderSize+3] >> 6; 
					memset(&ev, 0, sizeof(ev));
					ev.flags = keyidx & IW_MICFAILURE_KEY_ID;
					if ((pMgmt->eCurrMode == WMAC_MODE_ESS_STA) &&
							(pMgmt->eCurrState == WMAC_STATE_ASSOC) &&
								(*pbyRsr & (RSR_ADDRBROAD | RSR_ADDRMULTI)) == 0) {
						ev.flags |= IW_MICFAILURE_PAIRWISE;
					} else {
						ev.flags |= IW_MICFAILURE_GROUP;
					}

					ev.src_addr.sa_family = ARPHRD_ETHER;
					memcpy(ev.src_addr.sa_data, pMACHeader->abyAddr2, ETH_ALEN);
					memset(&wrqu, 0, sizeof(wrqu));
					wrqu.data.length = sizeof(ev);
					wireless_send_event(pDevice->dev, IWEVMICHAELMICFAILURE, &wrqu, (char *)&ev);

				}
         #endif


                if ((pDevice->bWPADEVUp) && (pDevice->skb != NULL)) {
                     wpahdr = (viawget_wpa_header *)pDevice->skb->data;
                     if ((pDevice->pMgmt->eCurrMode == WMAC_MODE_ESS_STA) &&
                         (pDevice->pMgmt->eCurrState == WMAC_STATE_ASSOC) &&
                         (*pbyRsr & (RSR_ADDRBROAD | RSR_ADDRMULTI)) == 0) {
                         
                         wpahdr->type = VIAWGET_PTK_MIC_MSG;
                     } else {
                         
                         wpahdr->type = VIAWGET_GTK_MIC_MSG;
                     }
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

                return FALSE;

            }
        }
    } 

    

    if ((pKey != NULL) && ((pKey->byCipherSuite == KEY_CTL_TKIP) ||
                           (pKey->byCipherSuite == KEY_CTL_CCMP))) {
        if (bIsWEP) {
            WORD        wLocalTSC15_0 = 0;
            DWORD       dwLocalTSC47_16 = 0;
            ULONGLONG       RSC = 0;
            
            RSC = *((ULONGLONG *) &(pKey->KeyRSC));
            wLocalTSC15_0 = (WORD) RSC;
            dwLocalTSC47_16 = (DWORD) (RSC>>16);

            RSC = dwRxTSC47_16;
            RSC <<= 16;
            RSC += wRxTSC15_0;
            memcpy(&(pKey->KeyRSC), &RSC,  sizeof(QWORD));

            if ( (pDevice->sMgmtObj.eCurrMode == WMAC_MODE_ESS_STA) &&
                 (pDevice->sMgmtObj.eCurrState == WMAC_STATE_ASSOC)) {
                
                if ( (wRxTSC15_0 < wLocalTSC15_0) &&
                     (dwRxTSC47_16 <= dwLocalTSC47_16) &&
                     !((dwRxTSC47_16 == 0) && (dwLocalTSC47_16 == 0xFFFFFFFF))) {
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"TSC is illegal~~!\n ");
                    if (pKey->byCipherSuite == KEY_CTL_TKIP)
                        
                        pDevice->s802_11Counter.TKIPReplays++;
                    else
                        
                        pDevice->s802_11Counter.CCMPReplays++;

                    if (bDeFragRx) {
                        if (!device_alloc_frag_buf(pDevice, &pDevice->sRxDFCB[pDevice->uCurrentDFCBIdx])) {
                            DBG_PRT(MSG_LEVEL_ERR,KERN_ERR "%s: can not alloc more frag bufs\n",
                                pDevice->dev->name);
                        }
                    }
                    return FALSE;
                }
            }
        }
    } 



    if ((pKey != NULL) && (bIsWEP)) {

    }


    s_vProcessRxMACHeader(pDevice, (PBYTE)(skb->data+4), FrameSize, bIsWEP, bExtIV, &cbHeaderOffset);
    FrameSize -= cbHeaderOffset;
    cbHeaderOffset += 4;        

    
    if (FrameSize < 15)
        return FALSE;

    if (pMgmt->eCurrMode == WMAC_MODE_ESS_AP) {
        if (s_bAPModeRxData(pDevice,
                            skb,
                            FrameSize,
                            cbHeaderOffset,
                            iSANodeIndex,
                            iDANodeIndex
                            ) == FALSE) {

            if (bDeFragRx) {
                if (!device_alloc_frag_buf(pDevice, &pDevice->sRxDFCB[pDevice->uCurrentDFCBIdx])) {
                    DBG_PRT(MSG_LEVEL_ERR,KERN_ERR "%s: can not alloc more frag bufs\n",
                    pDevice->dev->name);
                }
            }
            return FALSE;
        }







    }

	skb->data += cbHeaderOffset;
	skb->tail += cbHeaderOffset;
    skb_put(skb, FrameSize);
    skb->protocol=eth_type_trans(skb, skb->dev);


	


    skb->ip_summed=CHECKSUM_NONE;
    pStats->rx_bytes +=skb->len;
    pStats->rx_packets++;
    netif_rx(skb);

    if (bDeFragRx) {
        if (!device_alloc_frag_buf(pDevice, &pDevice->sRxDFCB[pDevice->uCurrentDFCBIdx])) {
            DBG_PRT(MSG_LEVEL_ERR,KERN_ERR "%s: can not alloc more frag bufs\n",
                pDevice->dev->name);
        }
        return FALSE;
    }

    return TRUE;
}


static BOOL s_bAPModeRxCtl (
    IN PSDevice pDevice,
    IN PBYTE    pbyFrame,
    IN INT      iSANodeIndex
    )
{
    PS802_11Header      p802_11Header;
    CMD_STATUS          Status;
    PSMgmtObject        pMgmt = pDevice->pMgmt;


    if (IS_CTL_PSPOLL(pbyFrame) || !IS_TYPE_CONTROL(pbyFrame)) {

        p802_11Header = (PS802_11Header) (pbyFrame);
        if (!IS_TYPE_MGMT(pbyFrame)) {

            
            
            if (iSANodeIndex > 0) {
                
                if (pMgmt->sNodeDBTable[iSANodeIndex].eNodeState < NODE_AUTH) {
                    
                    
                    vMgrDeAuthenBeginSta(pDevice,
                                         pMgmt,
                                         (PBYTE)(p802_11Header->abyAddr2),
                                         (WLAN_MGMT_REASON_CLASS2_NONAUTH),
                                         &Status
                                         );
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "dpc: send vMgrDeAuthenBeginSta 1\n");
                    return TRUE;
                };
                if (pMgmt->sNodeDBTable[iSANodeIndex].eNodeState < NODE_ASSOC) {
                    
                    
                    vMgrDisassocBeginSta(pDevice,
                                         pMgmt,
                                         (PBYTE)(p802_11Header->abyAddr2),
                                         (WLAN_MGMT_REASON_CLASS3_NONASSOC),
                                         &Status
                                         );
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "dpc: send vMgrDisassocBeginSta 2\n");
                    return TRUE;
                };

                if (pMgmt->sNodeDBTable[iSANodeIndex].bPSEnable) {
                    
                    if (IS_CTL_PSPOLL(pbyFrame)) {
                        pMgmt->sNodeDBTable[iSANodeIndex].bRxPSPoll = TRUE;
                        bScheduleCommand((HANDLE)pDevice, WLAN_CMD_RX_PSPOLL, NULL);
                        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "dpc: WLAN_CMD_RX_PSPOLL 1\n");
                    }
                    else {
                        
                        
                        if (!IS_FC_POWERMGT(pbyFrame)) {
                            pMgmt->sNodeDBTable[iSANodeIndex].bPSEnable = FALSE;
                            pMgmt->sNodeDBTable[iSANodeIndex].bRxPSPoll = TRUE;
                            bScheduleCommand((HANDLE)pDevice, WLAN_CMD_RX_PSPOLL, NULL);
                            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "dpc: WLAN_CMD_RX_PSPOLL 2\n");
                        }
                    }
                }
                else {
                   if (IS_FC_POWERMGT(pbyFrame)) {
                       pMgmt->sNodeDBTable[iSANodeIndex].bPSEnable = TRUE;
                       
                       pMgmt->sNodeDBTable[0].bPSEnable = TRUE;
                   }
                   else {
                      
                      if (pMgmt->sNodeDBTable[iSANodeIndex].wEnQueueCnt > 0) {
                          pMgmt->sNodeDBTable[iSANodeIndex].bPSEnable = FALSE;
                          pMgmt->sNodeDBTable[iSANodeIndex].bRxPSPoll = TRUE;
                          bScheduleCommand((HANDLE)pDevice, WLAN_CMD_RX_PSPOLL, NULL);
                         DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "dpc: WLAN_CMD_RX_PSPOLL 3\n");

                      }
                   }
                }
            }
            else {
                  vMgrDeAuthenBeginSta(pDevice,
                                       pMgmt,
                                       (PBYTE)(p802_11Header->abyAddr2),
                                       (WLAN_MGMT_REASON_CLASS2_NONAUTH),
                                       &Status
                                       );
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "dpc: send vMgrDeAuthenBeginSta 3\n");
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "BSSID:%02x-%02x-%02x=%02x-%02x-%02x \n",
                                p802_11Header->abyAddr3[0],
                                p802_11Header->abyAddr3[1],
                                p802_11Header->abyAddr3[2],
                                p802_11Header->abyAddr3[3],
                                p802_11Header->abyAddr3[4],
                                p802_11Header->abyAddr3[5]
                               );
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "ADDR2:%02x-%02x-%02x=%02x-%02x-%02x \n",
                                p802_11Header->abyAddr2[0],
                                p802_11Header->abyAddr2[1],
                                p802_11Header->abyAddr2[2],
                                p802_11Header->abyAddr2[3],
                                p802_11Header->abyAddr2[4],
                                p802_11Header->abyAddr2[5]
                               );
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "ADDR1:%02x-%02x-%02x=%02x-%02x-%02x \n",
                                p802_11Header->abyAddr1[0],
                                p802_11Header->abyAddr1[1],
                                p802_11Header->abyAddr1[2],
                                p802_11Header->abyAddr1[3],
                                p802_11Header->abyAddr1[4],
                                p802_11Header->abyAddr1[5]
                               );
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "dpc: wFrameCtl= %x\n", p802_11Header->wFrameCtl );
                    VNSvInPortB(pDevice->PortOffset + MAC_REG_RCR, &(pDevice->byRxMode));
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "dpc:pDevice->byRxMode = %x\n", pDevice->byRxMode );
                    return TRUE;
            }
        }
    }
    return FALSE;

}

static BOOL s_bHandleRxEncryption (
    IN PSDevice     pDevice,
    IN PBYTE        pbyFrame,
    IN UINT         FrameSize,
    IN PBYTE        pbyRsr,
    OUT PBYTE       pbyNewRsr,
    OUT PSKeyItem   *pKeyOut,
    int *       pbExtIV,
    OUT PWORD       pwRxTSC15_0,
    OUT PDWORD      pdwRxTSC47_16
    )
{
    UINT            PayloadLen = FrameSize;
    PBYTE           pbyIV;
    BYTE            byKeyIdx;
    PSKeyItem       pKey = NULL;
    BYTE            byDecMode = KEY_CTL_WEP;
    PSMgmtObject    pMgmt = pDevice->pMgmt;


    *pwRxTSC15_0 = 0;
    *pdwRxTSC47_16 = 0;

    pbyIV = pbyFrame + WLAN_HDR_ADDR3_LEN;
    if ( WLAN_GET_FC_TODS(*(PWORD)pbyFrame) &&
         WLAN_GET_FC_FROMDS(*(PWORD)pbyFrame) ) {
         pbyIV += 6;             
         PayloadLen -= 6;
    }
    byKeyIdx = (*(pbyIV+3) & 0xc0);
    byKeyIdx >>= 6;
    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"\nKeyIdx: %d\n", byKeyIdx);

    if ((pMgmt->eAuthenMode == WMAC_AUTH_WPA) ||
        (pMgmt->eAuthenMode == WMAC_AUTH_WPAPSK) ||
        (pMgmt->eAuthenMode == WMAC_AUTH_WPANONE) ||
        (pMgmt->eAuthenMode == WMAC_AUTH_WPA2) ||
        (pMgmt->eAuthenMode == WMAC_AUTH_WPA2PSK)) {
        if (((*pbyRsr & (RSR_ADDRBROAD | RSR_ADDRMULTI)) == 0) &&
            (pDevice->pMgmt->byCSSPK != KEY_CTL_NONE)) {
            
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"unicast pkt\n");
            if (KeybGetKey(&(pDevice->sKey), pDevice->abyBSSID, 0xFFFFFFFF, &pKey) == TRUE) {
                if (pDevice->pMgmt->byCSSPK == KEY_CTL_TKIP)
                    byDecMode = KEY_CTL_TKIP;
                else if (pDevice->pMgmt->byCSSPK == KEY_CTL_CCMP)
                    byDecMode = KEY_CTL_CCMP;
            }
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"unicast pkt: %d, %p\n", byDecMode, pKey);
        } else {
            
            KeybGetKey(&(pDevice->sKey), pDevice->abyBSSID, byKeyIdx, &pKey);
            if (pDevice->pMgmt->byCSSGK == KEY_CTL_TKIP)
                byDecMode = KEY_CTL_TKIP;
            else if (pDevice->pMgmt->byCSSGK == KEY_CTL_CCMP)
                byDecMode = KEY_CTL_CCMP;
            DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"group pkt: %d, %d, %p\n", byKeyIdx, byDecMode, pKey);
        }
    }
    
    if (pKey == NULL) {
        
        KeybGetKey(&(pDevice->sKey), pDevice->abyBroadcastAddr, byKeyIdx, &pKey);
        if (pDevice->pMgmt->byCSSGK == KEY_CTL_TKIP)
            byDecMode = KEY_CTL_TKIP;
        else if (pDevice->pMgmt->byCSSGK == KEY_CTL_CCMP)
            byDecMode = KEY_CTL_CCMP;
    }
    *pKeyOut = pKey;

    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"AES:%d %d %d\n", pDevice->pMgmt->byCSSPK, pDevice->pMgmt->byCSSGK, byDecMode);

    if (pKey == NULL) {
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"pKey == NULL\n");
        if (byDecMode == KEY_CTL_WEP) {

        } else if (pDevice->bLinkPass == TRUE) {

        }
        return FALSE;
    }
    if (byDecMode != pKey->byCipherSuite) {
        if (byDecMode == KEY_CTL_WEP) {

        } else if (pDevice->bLinkPass == TRUE) {

        }
        *pKeyOut = NULL;
        return FALSE;
    }
    if (byDecMode == KEY_CTL_WEP) {
        
        if ((pDevice->byLocalID <= REV_ID_VT3253_A1) ||
            (((PSKeyTable)(pKey->pvKeyTable))->bSoftWEP == TRUE)) {
            
            
            

            PayloadLen -= (WLAN_HDR_ADDR3_LEN + 4 + 4); 
            memcpy(pDevice->abyPRNG, pbyIV, 3);
            memcpy(pDevice->abyPRNG + 3, pKey->abyKey, pKey->uKeyLength);
            rc4_init(&pDevice->SBox, pDevice->abyPRNG, pKey->uKeyLength + 3);
            rc4_encrypt(&pDevice->SBox, pbyIV+4, pbyIV+4, PayloadLen);

            if (ETHbIsBufferCrc32Ok(pbyIV+4, PayloadLen)) {
                *pbyNewRsr |= NEWRSR_DECRYPTOK;
            }
        }
    } else if ((byDecMode == KEY_CTL_TKIP) ||
               (byDecMode == KEY_CTL_CCMP)) {
        

        PayloadLen -= (WLAN_HDR_ADDR3_LEN + 8 + 4); 
        *pdwRxTSC47_16 = cpu_to_le32(*(PDWORD)(pbyIV + 4));
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ExtIV: %lx\n",*pdwRxTSC47_16);
        if (byDecMode == KEY_CTL_TKIP) {
            *pwRxTSC15_0 = cpu_to_le16(MAKEWORD(*(pbyIV+2), *pbyIV));
        } else {
            *pwRxTSC15_0 = cpu_to_le16(*(PWORD)pbyIV);
        }
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"TSC0_15: %x\n", *pwRxTSC15_0);

        if ((byDecMode == KEY_CTL_TKIP) &&
            (pDevice->byLocalID <= REV_ID_VT3253_A1)) {
            
            
            PS802_11Header  pMACHeader = (PS802_11Header) (pbyFrame);
            TKIPvMixKey(pKey->abyKey, pMACHeader->abyAddr2, *pwRxTSC15_0, *pdwRxTSC47_16, pDevice->abyPRNG);
            rc4_init(&pDevice->SBox, pDevice->abyPRNG, TKIP_KEY_LEN);
            rc4_encrypt(&pDevice->SBox, pbyIV+8, pbyIV+8, PayloadLen);
            if (ETHbIsBufferCrc32Ok(pbyIV+8, PayloadLen)) {
                *pbyNewRsr |= NEWRSR_DECRYPTOK;
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ICV OK!\n");
            } else {
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ICV FAIL!!!\n");
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"PayloadLen = %d\n", PayloadLen);
            }
        }
    }

    if ((*(pbyIV+3) & 0x20) != 0)
        *pbExtIV = TRUE;
    return TRUE;
}


static BOOL s_bHostWepRxEncryption (
    IN PSDevice     pDevice,
    IN PBYTE        pbyFrame,
    IN UINT         FrameSize,
    IN PBYTE        pbyRsr,
    IN BOOL         bOnFly,
    IN PSKeyItem    pKey,
    OUT PBYTE       pbyNewRsr,
    int *       pbExtIV,
    OUT PWORD       pwRxTSC15_0,
    OUT PDWORD      pdwRxTSC47_16
    )
{
    UINT            PayloadLen = FrameSize;
    PBYTE           pbyIV;
    BYTE            byKeyIdx;
    BYTE            byDecMode = KEY_CTL_WEP;
    PS802_11Header  pMACHeader;



    *pwRxTSC15_0 = 0;
    *pdwRxTSC47_16 = 0;

    pbyIV = pbyFrame + WLAN_HDR_ADDR3_LEN;
    if ( WLAN_GET_FC_TODS(*(PWORD)pbyFrame) &&
         WLAN_GET_FC_FROMDS(*(PWORD)pbyFrame) ) {
         pbyIV += 6;             
         PayloadLen -= 6;
    }
    byKeyIdx = (*(pbyIV+3) & 0xc0);
    byKeyIdx >>= 6;
    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"\nKeyIdx: %d\n", byKeyIdx);


    if (pDevice->pMgmt->byCSSGK == KEY_CTL_TKIP)
        byDecMode = KEY_CTL_TKIP;
    else if (pDevice->pMgmt->byCSSGK == KEY_CTL_CCMP)
        byDecMode = KEY_CTL_CCMP;

    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"AES:%d %d %d\n", pDevice->pMgmt->byCSSPK, pDevice->pMgmt->byCSSGK, byDecMode);

    if (byDecMode != pKey->byCipherSuite) {
        if (byDecMode == KEY_CTL_WEP) {

        } else if (pDevice->bLinkPass == TRUE) {

        }
        return FALSE;
    }

    if (byDecMode == KEY_CTL_WEP) {
        
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"byDecMode == KEY_CTL_WEP \n");
        if ((pDevice->byLocalID <= REV_ID_VT3253_A1) ||
            (((PSKeyTable)(pKey->pvKeyTable))->bSoftWEP == TRUE) ||
            (bOnFly == FALSE)) {
            
            
            
            

            PayloadLen -= (WLAN_HDR_ADDR3_LEN + 4 + 4); 
            memcpy(pDevice->abyPRNG, pbyIV, 3);
            memcpy(pDevice->abyPRNG + 3, pKey->abyKey, pKey->uKeyLength);
            rc4_init(&pDevice->SBox, pDevice->abyPRNG, pKey->uKeyLength + 3);
            rc4_encrypt(&pDevice->SBox, pbyIV+4, pbyIV+4, PayloadLen);

            if (ETHbIsBufferCrc32Ok(pbyIV+4, PayloadLen)) {
                *pbyNewRsr |= NEWRSR_DECRYPTOK;
            }
        }
    } else if ((byDecMode == KEY_CTL_TKIP) ||
               (byDecMode == KEY_CTL_CCMP)) {
        

        PayloadLen -= (WLAN_HDR_ADDR3_LEN + 8 + 4); 
        *pdwRxTSC47_16 = cpu_to_le32(*(PDWORD)(pbyIV + 4));
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ExtIV: %lx\n",*pdwRxTSC47_16);

        if (byDecMode == KEY_CTL_TKIP) {
            *pwRxTSC15_0 = cpu_to_le16(MAKEWORD(*(pbyIV+2), *pbyIV));
        } else {
            *pwRxTSC15_0 = cpu_to_le16(*(PWORD)pbyIV);
        }
        DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"TSC0_15: %x\n", *pwRxTSC15_0);

        if (byDecMode == KEY_CTL_TKIP) {

            if ((pDevice->byLocalID <= REV_ID_VT3253_A1) || (bOnFly == FALSE)) {
                
                
                
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"soft KEY_CTL_TKIP \n");
                pMACHeader = (PS802_11Header) (pbyFrame);
                TKIPvMixKey(pKey->abyKey, pMACHeader->abyAddr2, *pwRxTSC15_0, *pdwRxTSC47_16, pDevice->abyPRNG);
                rc4_init(&pDevice->SBox, pDevice->abyPRNG, TKIP_KEY_LEN);
                rc4_encrypt(&pDevice->SBox, pbyIV+8, pbyIV+8, PayloadLen);
                if (ETHbIsBufferCrc32Ok(pbyIV+8, PayloadLen)) {
                    *pbyNewRsr |= NEWRSR_DECRYPTOK;
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ICV OK!\n");
                } else {
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"ICV FAIL!!!\n");
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"PayloadLen = %d\n", PayloadLen);
                }
            }
        }

        if (byDecMode == KEY_CTL_CCMP) {
            if (bOnFly == FALSE) {
                
                
                DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"soft KEY_CTL_CCMP\n");
                if (AESbGenCCMP(pKey->abyKey, pbyFrame, FrameSize)) {
                    *pbyNewRsr |= NEWRSR_DECRYPTOK;
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"CCMP MIC compare OK!\n");
                } else {
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO"CCMP MIC fail!\n");
                }
            }
        }

    }

    if ((*(pbyIV+3) & 0x20) != 0)
        *pbExtIV = TRUE;
    return TRUE;
}



static BOOL s_bAPModeRxData (
    IN PSDevice pDevice,
    IN struct sk_buff* skb,
    IN UINT     FrameSize,
    IN UINT     cbHeaderOffset,
    IN INT      iSANodeIndex,
    IN INT      iDANodeIndex
    )
{
    PSMgmtObject        pMgmt = pDevice->pMgmt;
    BOOL                bRelayAndForward = FALSE;
    BOOL                bRelayOnly = FALSE;
    BYTE                byMask[8] = {1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80};
    WORD                wAID;


    struct sk_buff* skbcpy = NULL;

    if (FrameSize > CB_MAX_BUF_SIZE)
        return FALSE;
    
    if(IS_MULTICAST_ADDRESS((PBYTE)(skb->data+cbHeaderOffset))) {
       if (pMgmt->sNodeDBTable[0].bPSEnable) {

           skbcpy = dev_alloc_skb((int)pDevice->rx_buf_sz);

        
           if (skbcpy == NULL) {
               DBG_PRT(MSG_LEVEL_NOTICE, KERN_INFO "relay multicast no skb available \n");
           }
           else {
               skbcpy->dev = pDevice->dev;
               skbcpy->len = FrameSize;
               memcpy(skbcpy->data, skb->data+cbHeaderOffset, FrameSize);
               skb_queue_tail(&(pMgmt->sNodeDBTable[0].sTxPSQueue), skbcpy);

               pMgmt->sNodeDBTable[0].wEnQueueCnt++;
               
               pMgmt->abyPSTxMap[0] |= byMask[0];
           }
       }
       else {
           bRelayAndForward = TRUE;
       }
    }
    else {
        
        if (BSSDBbIsSTAInNodeDB(pMgmt, (PBYTE)(skb->data+cbHeaderOffset), &iDANodeIndex)) {
            if (pMgmt->sNodeDBTable[iDANodeIndex].eNodeState >= NODE_ASSOC) {
                if (pMgmt->sNodeDBTable[iDANodeIndex].bPSEnable) {
                    

	                skb->data += cbHeaderOffset;
	                skb->tail += cbHeaderOffset;
                    skb_put(skb, FrameSize);
                    skb_queue_tail(&pMgmt->sNodeDBTable[iDANodeIndex].sTxPSQueue, skb);
                    pMgmt->sNodeDBTable[iDANodeIndex].wEnQueueCnt++;
                    wAID = pMgmt->sNodeDBTable[iDANodeIndex].wAID;
                    pMgmt->abyPSTxMap[wAID >> 3] |=  byMask[wAID & 7];
                    DBG_PRT(MSG_LEVEL_DEBUG, KERN_INFO "relay: index= %d, pMgmt->abyPSTxMap[%d]= %d\n",
                               iDANodeIndex, (wAID >> 3), pMgmt->abyPSTxMap[wAID >> 3]);
                    return TRUE;
                }
                else {
                    bRelayOnly = TRUE;
                }
            }
        };
    }

    if (bRelayOnly || bRelayAndForward) {
        
        if (bRelayAndForward)
            iDANodeIndex = 0;

        if ((pDevice->uAssocCount > 1) && (iDANodeIndex >= 0)) {
            ROUTEbRelay(pDevice, (PBYTE)(skb->data + cbHeaderOffset), FrameSize, (UINT)iDANodeIndex);
        }

        if (bRelayOnly)
            return FALSE;
    }
    
    if (pDevice->uAssocCount == 0)
        return FALSE;

    return TRUE;
}

