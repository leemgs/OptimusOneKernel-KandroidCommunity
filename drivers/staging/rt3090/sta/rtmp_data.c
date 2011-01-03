

#include "../rt_config.h"


VOID STARxEAPOLFrameIndicate(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID)
{
	PRT28XX_RXD_STRUC	pRxD = &(pRxBlk->RxD);
	PRXWI_STRUC		pRxWI = pRxBlk->pRxWI;
	UCHAR			*pTmpBuf;


#ifdef WPA_SUPPLICANT_SUPPORT
	if (pAd->StaCfg.WpaSupplicantUP)
	{
		
		
		{
			
			if ( pAd->StaCfg.IEEE8021X == TRUE &&
				 (EAP_CODE_SUCCESS == WpaCheckEapCode(pAd, pRxBlk->pData, pRxBlk->DataSize, LENGTH_802_1_H)))
			{
				PUCHAR	Key;
				UCHAR	CipherAlg;
				int     idx = 0;

				DBGPRINT_RAW(RT_DEBUG_TRACE, ("Receive EAP-SUCCESS Packet\n"));
				
				STA_PORT_SECURED(pAd);

                if (pAd->StaCfg.IEEE8021x_required_keys == FALSE)
                {
                    idx = pAd->StaCfg.DesireSharedKeyId;
                    CipherAlg = pAd->StaCfg.DesireSharedKey[idx].CipherAlg;
					Key = pAd->StaCfg.DesireSharedKey[idx].Key;

                    if (pAd->StaCfg.DesireSharedKey[idx].KeyLen > 0)
				{
#ifdef RTMP_MAC_PCI
						MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[BSSID_WCID];

						
						AsicAddSharedKeyEntry(pAd, BSS0, idx, CipherAlg, Key, NULL, NULL);

						
						RTMPAddWcidAttributeEntry(pAd, BSS0, idx, CipherAlg, NULL);

						
						RTMPAddWcidAttributeEntry(pAd, BSS0, idx, CipherAlg, pEntry);

                        pAd->IndicateMediaState = NdisMediaStateConnected;
                        pAd->ExtraInfo = GENERAL_LINK_UP;
#endif 
						
					pAd->SharedKey[BSS0][idx].CipherAlg = CipherAlg;
						pAd->SharedKey[BSS0][idx].KeyLen = pAd->StaCfg.DesireSharedKey[idx].KeyLen;
						NdisMoveMemory(pAd->SharedKey[BSS0][idx].Key,
									   pAd->StaCfg.DesireSharedKey[idx].Key,
									   pAd->StaCfg.DesireSharedKey[idx].KeyLen);
				}
				}
			}

			Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);
			return;
		}
	}
	else
#endif 
	{
		
		
		
		{
			pTmpBuf = pRxBlk->pData - LENGTH_802_11;
			NdisMoveMemory(pTmpBuf, pRxBlk->pHeader, LENGTH_802_11);
			REPORT_MGMT_FRAME_TO_MLME(pAd, pRxWI->WirelessCliID, pTmpBuf, pRxBlk->DataSize + LENGTH_802_11, pRxWI->RSSI0, pRxWI->RSSI1, pRxWI->RSSI2, pRxD->PlcpSignal);
			DBGPRINT_RAW(RT_DEBUG_TRACE, ("!!! report EAPOL/AIRONET DATA to MLME (len=%d) !!!\n", pRxBlk->DataSize));
		}
	}

	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	return;

}

VOID STARxDataFrameAnnounce(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID)
{

	
	if (!RTMPCheckWPAframe(pAd, pEntry, pRxBlk->pData, pRxBlk->DataSize, FromWhichBSSID))
	{

		{
			
			
			if (pRxBlk->pHeader->FC.Wep)
			{
				
				if (pAd->StaCfg.WepStatus == Ndis802_11EncryptionDisabled)
				{
					
					RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
					return;
				}
			}
			else
			{
				
				if ((pAd->StaCfg.WepStatus != Ndis802_11EncryptionDisabled) &&
					(pAd->StaCfg.PortSecured == WPA_802_1X_PORT_NOT_SECURED))
				{
					
					RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
					return;
				}
			}
		}
		RX_BLK_CLEAR_FLAG(pRxBlk, fRX_EAP);


		if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_ARALINK))
		{
			
			CmmRxnonRalinkFrameIndicate(pAd, pRxBlk, FromWhichBSSID);

		}
		else
		{
			
			CmmRxRalinkFrameIndicate(pAd, pEntry, pRxBlk, FromWhichBSSID);
		}
#ifdef QOS_DLS_SUPPORT
		RX_BLK_CLEAR_FLAG(pRxBlk, fRX_DLS);
#endif 
	}
	else
	{
		RX_BLK_SET_FLAG(pRxBlk, fRX_EAP);
#ifdef DOT11_N_SUPPORT
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU) && (pAd->CommonCfg.bDisableReordering == 0))
		{
			Indicate_AMPDU_Packet(pAd, pRxBlk, FromWhichBSSID);
		}
		else
#endif 
		{
			
			
			STARxEAPOLFrameIndicate(pAd, pEntry, pRxBlk, FromWhichBSSID);
		}
	}
}



BOOLEAN STACheckTkipMICValue(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk)
{
	PHEADER_802_11	pHeader = pRxBlk->pHeader;
	UCHAR			*pData = pRxBlk->pData;
	USHORT			DataSize = pRxBlk->DataSize;
	UCHAR			UserPriority = pRxBlk->UserPriority;
	PCIPHER_KEY		pWpaKey;
	UCHAR			*pDA, *pSA;

	pWpaKey = &pAd->SharedKey[BSS0][pRxBlk->pRxWI->KeyIndex];

	pDA = pHeader->Addr1;
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_INFRA))
	{
		pSA = pHeader->Addr3;
	}
	else
	{
		pSA = pHeader->Addr2;
	}

	if (RTMPTkipCompareMICValue(pAd,
								pData,
								pDA,
								pSA,
								pWpaKey->RxMic,
								UserPriority,
								DataSize) == FALSE)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR,("Rx MIC Value error 2\n"));

#ifdef WPA_SUPPLICANT_SUPPORT
		if (pAd->StaCfg.WpaSupplicantUP)
		{
			WpaSendMicFailureToWpaSupplicant(pAd, (pWpaKey->Type == PAIRWISEKEY) ? TRUE : FALSE);
		}
		else
#endif 
		{
			RTMPReportMicError(pAd, pWpaKey);
		}

		
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return FALSE;
	}

	return TRUE;
}










VOID STAHandleRxDataFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk)
{
	PRT28XX_RXD_STRUC				pRxD = &(pRxBlk->RxD);
	PRXWI_STRUC						pRxWI = pRxBlk->pRxWI;
	PHEADER_802_11					pHeader = pRxBlk->pHeader;
	PNDIS_PACKET					pRxPacket = pRxBlk->pRxPacket;
	BOOLEAN							bFragment = FALSE;
	MAC_TABLE_ENTRY				*pEntry = NULL;
	UCHAR							FromWhichBSSID = BSS0;
	UCHAR                           UserPriority = 0;

	{
		
		if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
		{
			
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

#ifdef QOS_DLS_SUPPORT
		
		if (RTMPRcvFrameDLSCheck(pAd, pHeader, pRxWI->MPDUtotalByteCount, pRxD))
		{
			return;
		}
#endif 

		
		if (pRxD->MyBss == 0)
		{
			{
				
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
				return;
			}
		}

		pAd->RalinkCounters.RxCountSinceLastNULL++;
		if (pAd->CommonCfg.bAPSDCapable && pAd->CommonCfg.APEdcaParm.bAPSDCapable && (pHeader->FC.SubType & 0x08))
		{
			UCHAR *pData;
			DBGPRINT(RT_DEBUG_INFO,("bAPSDCapable\n"));

			
			pData = (PUCHAR)pHeader + LENGTH_802_11;
			if ((*pData >> 4) & 0x01)
			{
				DBGPRINT(RT_DEBUG_INFO,("RxDone- Rcv EOSP frame, driver may fall into sleep\n"));
				pAd->CommonCfg.bInServicePeriod = FALSE;

				
				if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
				{
					USHORT  TbttNumToNextWakeUp;
					USHORT  NextDtim = pAd->StaCfg.DtimPeriod;
					ULONG   Now;

					NdisGetSystemUpTime(&Now);
					NextDtim -= (USHORT)(Now - pAd->StaCfg.LastBeaconRxTime)/pAd->CommonCfg.BeaconPeriod;

					TbttNumToNextWakeUp = pAd->StaCfg.DefaultListenCount;
					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM) && (TbttNumToNextWakeUp > NextDtim))
						TbttNumToNextWakeUp = NextDtim;

					RTMP_SET_PSM_BIT(pAd, PWR_SAVE);
					
					AsicSleepThenAutoWakeup(pAd, TbttNumToNextWakeUp);
				}
			}

			if ((pHeader->FC.MoreData) && (pAd->CommonCfg.bInServicePeriod))
			{
				DBGPRINT(RT_DEBUG_TRACE,("Sending another trigger frame when More Data bit is set to 1\n"));
			}
		}

		
		if ((pHeader->FC.SubType & 0x04)) 
		{
			
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

	    
#ifdef QOS_DLS_SUPPORT
	    if (!pAd->CommonCfg.bDLSCapable)
	    {
#endif 
		if (INFRA_ON(pAd))
		{
			
			if (!RTMPEqualMemory(&pHeader->Addr2, &pAd->CommonCfg.Bssid, 6))
			{
				
	            
	            RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
				return;
			}
		}
		else	
		{
			
			if (!RTMPEqualMemory(&pHeader->Addr3, &pAd->CommonCfg.Bssid, 6))
			{
				
	            
	            RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
				return;
			}
		}
#ifdef QOS_DLS_SUPPORT
	    }
#endif 

		
		
		
		if (pRxWI->WirelessCliID < MAX_LEN_OF_MAC_TABLE)
		{
			pEntry = &pAd->MacTab.Content[pRxWI->WirelessCliID];

		}
		else
		{
			
			
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		
		if (INFRA_ON(pAd))
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_INFRA);
#ifdef QOS_DLS_SUPPORT
			if ((pHeader->FC.FrDs == 0) && (pHeader->FC.ToDs == 0))
				RX_BLK_SET_FLAG(pRxBlk, fRX_DLS);
			else
#endif 
			ASSERT(pRxWI->WirelessCliID == BSSID_WCID);
		}

		
		if ((pEntry->bIAmBadAtheros == FALSE) &&  (pRxD->AMPDU == 1) && (pHeader->FC.Retry ))
		{
			pEntry->bIAmBadAtheros = TRUE;
			pAd->CommonCfg.IOTestParm.bCurrentAtheros = TRUE;
			pAd->CommonCfg.IOTestParm.bLastAtheros = TRUE;
			if (!STA_AES_ON(pAd))
			{
				AsicUpdateProtect(pAd, 8, ALLN_SETPROTECT, TRUE, FALSE);
			}
		}
	}

	pRxBlk->pData = (UCHAR *)pHeader;

	
	
	
	

	
	{
		pRxBlk->pData += LENGTH_802_11;
		pRxBlk->DataSize -= LENGTH_802_11;
	}

	
	if (pHeader->FC.SubType & 0x08)
	{
		RX_BLK_SET_FLAG(pRxBlk, fRX_QOS);
		UserPriority = *(pRxBlk->pData) & 0x0f;
		
		if ((*pRxBlk->pData) & 0x80)
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMSDU);
		}

		
		pRxBlk->pData += 2;
		pRxBlk->DataSize -=2;
	}
	pRxBlk->UserPriority = UserPriority;

	
	if ((pAd->StaCfg.Psm == PWR_SAVE) && (pHeader->FC.MoreData == 1))
	{
		if ((((UserPriority == 0) || (UserPriority == 3)) &&
			pAd->CommonCfg.bAPSDAC_BE == 0) ||
			(((UserPriority == 1) || (UserPriority == 2)) &&
			pAd->CommonCfg.bAPSDAC_BK == 0) ||
			(((UserPriority == 4) || (UserPriority == 5)) &&
			pAd->CommonCfg.bAPSDAC_VI == 0) ||
			(((UserPriority == 6) || (UserPriority == 7)) &&
			pAd->CommonCfg.bAPSDAC_VO == 0))
		{
			
			RTMP_PS_POLL_ENQUEUE(pAd);
		}
	}

	
	if (pHeader->FC.Order)
	{
#ifdef AGGREGATION_SUPPORT
		if ((pRxWI->PHYMODE <= MODE_OFDM) && (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED)))
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_ARALINK);
		}
		else
#endif 
		{
#ifdef DOT11_N_SUPPORT
			RX_BLK_SET_FLAG(pRxBlk, fRX_HTC);
			
			pRxBlk->pData += 4;
			pRxBlk->DataSize -= 4;
#endif 
		}
	}

	
	if (pRxD->L2PAD)
	{
		
		
		RX_BLK_SET_FLAG(pRxBlk, fRX_PAD);
		pRxBlk->pData += 2;
	}

#ifdef DOT11_N_SUPPORT
	if (pRxD->BA)
	{
		RX_BLK_SET_FLAG(pRxBlk, fRX_AMPDU);
	}
#endif 


	
	
	
	if (pRxD->Bcast || pRxD->Mcast)
	{
		INC_COUNTER64(pAd->WlanCounters.MulticastReceivedFrameCount);

		
		if (pHeader->FC.MoreFrag)
		{
			
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		
		if (pHeader->FC.FrDs && MAC_ADDR_EQUAL(pHeader->Addr3, pAd->CurrentAddress))
		{
			
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}

		Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);
		return;
	}
	else if (pRxD->U2M)
	{
		pAd->LastRxRate = (USHORT)((pRxWI->MCS) + (pRxWI->BW <<7) + (pRxWI->ShortGI <<8)+ (pRxWI->PHYMODE <<14)) ;


#ifdef QOS_DLS_SUPPORT
        if (RX_BLK_TEST_FLAG(pRxBlk, fRX_DLS))
		{
			MAC_TABLE_ENTRY *pDlsEntry = NULL;

			pDlsEntry = DlsEntryTableLookupByWcid(pAd, pRxWI->WirelessCliID, pHeader->Addr2, TRUE);
										                        if(pDlsEntry)
			Update_Rssi_Sample(pAd, &pDlsEntry->RssiSample, pRxWI);
		}
		else
#endif 
		if (ADHOC_ON(pAd))
		{
			pEntry = MacTableLookup(pAd, pHeader->Addr2);
			if (pEntry)
				Update_Rssi_Sample(pAd, &pEntry->RssiSample, pRxWI);
		}


		Update_Rssi_Sample(pAd, &pAd->StaCfg.RssiSample, pRxWI);

		pAd->StaCfg.LastSNR0 = (UCHAR)(pRxWI->SNR0);
		pAd->StaCfg.LastSNR1 = (UCHAR)(pRxWI->SNR1);

		pAd->RalinkCounters.OneSecRxOkDataCnt++;


	if (!((pHeader->Frag == 0) && (pHeader->FC.MoreFrag == 0)))
	{
		
		
		bFragment = TRUE;
		pRxPacket = RTMPDeFragmentDataFrame(pAd, pRxBlk);
	}

	if (pRxPacket)
	{
		pEntry = &pAd->MacTab.Content[pRxWI->WirelessCliID];

		
		if (bFragment && (pRxD->Decrypted) && (pEntry->WepStatus == Ndis802_11Encryption2Enabled))
		{
			
			pRxBlk->DataSize -= 8;

			
			if (STACheckTkipMICValue(pAd, pEntry, pRxBlk) == FALSE)
			{
				return;
			}
		}

		STARxDataFrameAnnounce(pAd, pEntry, pRxBlk, FromWhichBSSID);
			return;
	}
	else
	{
		
		
		
		return;
	}
	}

	ASSERT(0);
	
	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
}

VOID STAHandleRxMgmtFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk)
{
	PRT28XX_RXD_STRUC	pRxD = &(pRxBlk->RxD);
	PRXWI_STRUC		pRxWI = pRxBlk->pRxWI;
	PHEADER_802_11	pHeader = pRxBlk->pHeader;
	PNDIS_PACKET	pRxPacket = pRxBlk->pRxPacket;

	do
	{


		
		if ((pAd->StaCfg.Psm == PWR_SAVE) && (pHeader->FC.MoreData == 1))
		{
			
			if (pAd->CommonCfg.bAPSDAC_VO == 0)
			{
				
				RTMP_PS_POLL_ENQUEUE(pAd);
			}
		}

		


		
		if ((pHeader->FC.SubType == SUBTYPE_BEACON) && (MAC_ADDR_EQUAL(&pAd->CommonCfg.Bssid, &pHeader->Addr2))
			&& (pAd->RxAnt.EvaluatePeriod == 0))
		{
			Update_Rssi_Sample(pAd, &pAd->StaCfg.RssiSample, pRxWI);

			pAd->StaCfg.LastSNR0 = (UCHAR)(pRxWI->SNR0);
			pAd->StaCfg.LastSNR1 = (UCHAR)(pRxWI->SNR1);
		}

#ifdef RT30xx
#ifdef ANT_DIVERSITY_SUPPORT
		
		if ((pAd->NicConfig2.field.AntDiversity) &&
			(pAd->CommonCfg.bRxAntDiversity != ANT_DIVERSITY_DISABLE))
		{
			if ((pRxD->U2M) || ((pHeader->FC.SubType == SUBTYPE_BEACON) && (MAC_ADDR_EQUAL(&pAd->CommonCfg.Bssid, &pHeader->Addr2))))
			{
					COLLECT_RX_ANTENNA_AVERAGE_RSSI(pAd, ConvertToRssi(pAd, (UCHAR)pRxWI->RSSI0, RSSI_0), 0); 
					pAd->StaCfg.NumOfAvgRssiSample ++;
			}
		}
#endif 
#endif 

		
		if (pRxWI->MPDUtotalByteCount > MGMT_DMA_BUFFER_SIZE)
		{
			DBGPRINT_ERR(("STAHandleRxMgmtFrame: frame too large, size = %d \n", pRxWI->MPDUtotalByteCount));
			break;
		}

		REPORT_MGMT_FRAME_TO_MLME(pAd, pRxWI->WirelessCliID, pHeader, pRxWI->MPDUtotalByteCount,
									pRxWI->RSSI0, pRxWI->RSSI1, pRxWI->RSSI2, pRxD->PlcpSignal);
	} while (FALSE);

	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
}

VOID STAHandleRxControlFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk)
{
#ifdef DOT11_N_SUPPORT
	PRXWI_STRUC		pRxWI = pRxBlk->pRxWI;
#endif 
	PHEADER_802_11	pHeader = pRxBlk->pHeader;
	PNDIS_PACKET	pRxPacket = pRxBlk->pRxPacket;

	switch (pHeader->FC.SubType)
	{
		case SUBTYPE_BLOCK_ACK_REQ:
#ifdef DOT11_N_SUPPORT
			{
				CntlEnqueueForRecv(pAd, pRxWI->WirelessCliID, (pRxWI->MPDUtotalByteCount), (PFRAME_BA_REQ)pHeader);
			}
			break;
#endif 
		case SUBTYPE_BLOCK_ACK:
		case SUBTYPE_ACK:
		default:
			break;
	}

	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
}



BOOLEAN STARxDoneInterruptHandle(
	IN	PRTMP_ADAPTER	pAd,
	IN	BOOLEAN			argc)
{
	NDIS_STATUS			Status;
	UINT32			RxProcessed, RxPending;
	BOOLEAN			bReschedule = FALSE;
	RT28XX_RXD_STRUC	*pRxD;
	UCHAR			*pData;
	PRXWI_STRUC		pRxWI;
	PNDIS_PACKET	pRxPacket;
	PHEADER_802_11	pHeader;
	RX_BLK			RxCell;

	RxProcessed = RxPending = 0;

	
	while (1)
	{

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF |
								fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST) ||
			!RTMP_TEST_FLAG(pAd,fRTMP_ADAPTER_START_UP))
		{
			break;
		}

#ifdef RTMP_MAC_PCI
		if (RxProcessed++ > MAX_RX_PROCESS_CNT)
		{
			
			bReschedule = TRUE;
			break;
		}
#endif 

		RxProcessed ++; 

		
		
		
		
		
		
		
		pRxPacket = GetPacketFromRxRing(pAd, &(RxCell.RxD), &bReschedule, &RxPending);
		if (pRxPacket == NULL)
		{
			
			break;
		}

		
		pRxD = &(RxCell.RxD);
		
		pData	= GET_OS_PKT_DATAPTR(pRxPacket);
		pRxWI	= (PRXWI_STRUC) pData;
		pHeader = (PHEADER_802_11) (pData+RXWI_SIZE) ;

#ifdef RT_BIG_ENDIAN
	    RTMPFrameEndianChange(pAd, (PUCHAR)pHeader, DIR_READ, TRUE);
		RTMPWIEndianChange((PUCHAR)pRxWI, TYPE_RXWI);
#endif

		
		RxCell.pRxWI = pRxWI;
		RxCell.pHeader = pHeader;
		RxCell.pRxPacket = pRxPacket;
		RxCell.pData = (UCHAR *) pHeader;
		RxCell.DataSize = pRxWI->MPDUtotalByteCount;
		RxCell.Flags = 0;

		
		pAd->RalinkCounters.ReceivedByteCount +=  pRxWI->MPDUtotalByteCount;
		pAd->RalinkCounters.OneSecReceivedByteCount +=  pRxWI->MPDUtotalByteCount;
		pAd->RalinkCounters.RxCount ++;

		INC_COUNTER64(pAd->WlanCounters.ReceivedFragmentCount);

		if (pRxWI->MPDUtotalByteCount < 14)
			Status = NDIS_STATUS_FAILURE;

        if (MONITOR_ON(pAd))
		{
            send_monitor_packets(pAd, &RxCell);
			break;
		}

		
#ifdef RALINK_ATE
		if (ATE_ON(pAd))
		{
			pAd->ate.RxCntPerSec++;
			ATESampleRssi(pAd, pRxWI);
#ifdef RALINK_28xx_QA
			if (pAd->ate.bQARxStart == TRUE)
			{
				
				ATE_QA_Statistics(pAd, pRxWI, pRxD,	pHeader);
			}
#endif 
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
			continue;
		}
#endif 

		
		Status = RTMPCheckRxError(pAd, pHeader, pRxWI, pRxD);

		
		if (Status == NDIS_STATUS_SUCCESS)
		{
			switch (pHeader->FC.Type)
			{
				
				case BTYPE_DATA:
				{
					
					STAHandleRxDataFrame(pAd, &RxCell);
				}
				break;
				
				case BTYPE_MGMT:
				{
					STAHandleRxMgmtFrame(pAd, &RxCell);
				}
				break;
				
				case BTYPE_CNTL:
				{
					STAHandleRxControlFrame(pAd, &RxCell);
				}
				break;
				
				default:
					RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
					break;
			}
		}
		else
		{
			pAd->Counters8023.RxErrors++;
			
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		}
	}

	return bReschedule;
}


VOID	RTMPHandleTwakeupInterrupt(
	IN PRTMP_ADAPTER pAd)
{
	AsicForceWakeup(pAd, FALSE);
}


VOID STASendPackets(
	IN	NDIS_HANDLE		MiniportAdapterContext,
	IN	PPNDIS_PACKET	ppPacketArray,
	IN	UINT			NumberOfPackets)
{
	UINT			Index;
	PRTMP_ADAPTER	pAd = (PRTMP_ADAPTER) MiniportAdapterContext;
	PNDIS_PACKET	pPacket;
	BOOLEAN			allowToSend = FALSE;


	for (Index = 0; Index < NumberOfPackets; Index++)
	{
		pPacket = ppPacketArray[Index];

		do
		{

			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) ||
				RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
				RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
			{
				
					break;
			}
			else if (!INFRA_ON(pAd) && !ADHOC_ON(pAd))
			{
				
					break;
			}
			else
			{
				
				
#ifdef QOS_DLS_SUPPORT
				MAC_TABLE_ENTRY *pEntry;
				PUCHAR pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);

				pEntry = MacTableLookup(pAd, pSrcBufVA);
				if (pEntry && (pEntry->ValidAsDls == TRUE))
				{
					RTMP_SET_PACKET_WCID(pPacket, pEntry->Aid);
				}
				else
#endif 
				RTMP_SET_PACKET_WCID(pPacket, 0); 
				RTMP_SET_PACKET_SOURCE(pPacket, PKTSRC_NDIS);
				NDIS_SET_PACKET_STATUS(pPacket, NDIS_STATUS_PENDING);
				pAd->RalinkCounters.PendingNdisPacketCount++;

				allowToSend = TRUE;
			}
		} while(FALSE);

		if (allowToSend == TRUE)
			STASendPacket(pAd, pPacket);
		else
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}

	
	RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);

}



NDIS_STATUS STASendPacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket)
{
	PACKET_INFO	PacketInfo;
	PUCHAR			pSrcBufVA;
	UINT			SrcBufLen;
	UINT			AllowFragSize;
	UCHAR			NumberOfFrag;
	UCHAR			RTSRequired;
	UCHAR			QueIdx, UserPriority;
	MAC_TABLE_ENTRY *pEntry = NULL;
	unsigned int	IrqFlags;
	UCHAR			FlgIsIP = 0;
	UCHAR			Rate;

	
	
	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	if (pSrcBufVA == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR,("STASendPacket --> pSrcBufVA == NULL !!!SrcBufLen=%x\n",SrcBufLen));
		
		
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}


	if (SrcBufLen < 14)
	{
		DBGPRINT(RT_DEBUG_ERROR,("STASendPacket --> Ndis Packet buffer error !!!\n"));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return (NDIS_STATUS_FAILURE);
	}

	
	
	{
		if(INFRA_ON(pAd))
		{
#ifdef QOS_DLS_SUPPORT
			USHORT	tmpWcid;

			tmpWcid = RTMP_GET_PACKET_WCID(pPacket);
			if (VALID_WCID(tmpWcid) &&
				(pAd->MacTab.Content[tmpWcid].ValidAsDls== TRUE))
			{
				pEntry = &pAd->MacTab.Content[tmpWcid];
				Rate = pAd->MacTab.Content[tmpWcid].CurrTxRate;
			}
			else
#endif 
			{
			pEntry = &pAd->MacTab.Content[BSSID_WCID];
			RTMP_SET_PACKET_WCID(pPacket, BSSID_WCID);
			Rate = pAd->CommonCfg.TxRate;
		}
		}
		else if (ADHOC_ON(pAd))
		{
			if (*pSrcBufVA & 0x01)
			{
				RTMP_SET_PACKET_WCID(pPacket, MCAST_WCID);
				pEntry = &pAd->MacTab.Content[MCAST_WCID];
			}
			else
			{
				pEntry = MacTableLookup(pAd, pSrcBufVA);
			}
			Rate = pAd->CommonCfg.TxRate;
		}
	}

	if (!pEntry)
	{
		DBGPRINT(RT_DEBUG_ERROR,("STASendPacket->Cannot find pEntry(%2x:%2x:%2x:%2x:%2x:%2x) in MacTab!\n", PRINT_MAC(pSrcBufVA)));
		
		
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (ADHOC_ON(pAd)
		)
	{
		RTMP_SET_PACKET_WCID(pPacket, (UCHAR)pEntry->Aid);
	}

	
	
	
	RTMPCheckEtherType(pAd, pPacket);



	
	
	
	if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) ||
		 (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK) ||
		 (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) ||
		 (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef WPA_SUPPLICANT_SUPPORT
		  || (pAd->StaCfg.IEEE8021X == TRUE)
#endif 
		  )
		  && ((pAd->StaCfg.PortSecured == WPA_802_1X_PORT_NOT_SECURED) || (pAd->StaCfg.MicErrCnt >= 2))
		  && (RTMP_GET_PACKET_EAPOL(pPacket)== FALSE)
		  )
	{
		DBGPRINT(RT_DEBUG_TRACE,("STASendPacket --> Drop packet before port secured !!!\n"));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);

		return (NDIS_STATUS_FAILURE);
	}


	
	
	
	
	


	if (*pSrcBufVA & 0x01)	
		NumberOfFrag = 1;
	else if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED))
		NumberOfFrag = 1;	
	else if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_AMSDU_INUSED))
		NumberOfFrag = 1;	
#ifdef DOT11_N_SUPPORT
	else if ((pAd->StaCfg.HTPhyMode.field.MODE == MODE_HTMIX) || (pAd->StaCfg.HTPhyMode.field.MODE == MODE_HTGREENFIELD))
		NumberOfFrag = 1;	
#endif 
	else
	{
		
		
		
		
		
		
		
		

		AllowFragSize = (pAd->CommonCfg.FragmentThreshold) - LENGTH_802_11 - LENGTH_CRC;
		NumberOfFrag = ((PacketInfo.TotalPacketLength - LENGTH_802_3 + LENGTH_802_1_H) / AllowFragSize) + 1;
		
		if (((PacketInfo.TotalPacketLength - LENGTH_802_3 + LENGTH_802_1_H) % AllowFragSize) == 0)
		{
			NumberOfFrag--;
		}
	}

	
	RTMP_SET_PACKET_FRAGMENTS(pPacket, NumberOfFrag);


	
	
	
	

	if (NumberOfFrag > 1)
		RTSRequired = (pAd->CommonCfg.FragmentThreshold > pAd->CommonCfg.RtsThreshold) ? 1 : 0;
	else
		RTSRequired = (PacketInfo.TotalPacketLength > pAd->CommonCfg.RtsThreshold) ? 1 : 0;

	
	RTMP_SET_PACKET_RTS(pPacket, RTSRequired);
	RTMP_SET_PACKET_TXRATE(pPacket, pAd->CommonCfg.TxRate);

	
	
	
	UserPriority = 0;
	QueIdx		 = QID_AC_BE;
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) &&
		CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
	{
		USHORT Protocol;
		UCHAR  LlcSnapLen = 0, Byte0, Byte1;
		do
		{
			
			Protocol = (USHORT)((pSrcBufVA[12] << 8) + pSrcBufVA[13]);
			if (Protocol <= 1500)
			{
				
				if (Sniff2BytesFromNdisBuffer(PacketInfo.pFirstBuffer, LENGTH_802_3 + 6, &Byte0, &Byte1) != NDIS_STATUS_SUCCESS)
					break;

				Protocol = (USHORT)((Byte0 << 8) + Byte1);
				LlcSnapLen = 8;
			}

			
			if (Protocol != 0x0800)
				break;

			
			if (Sniff2BytesFromNdisBuffer(PacketInfo.pFirstBuffer, LENGTH_802_3 + LlcSnapLen, &Byte0, &Byte1) != NDIS_STATUS_SUCCESS)
				break;

			
			if ((Byte0 & 0xf0) != 0x40)
				break;

			FlgIsIP = 1;
			UserPriority = (Byte1 & 0xe0) >> 5;
			QueIdx = MapUserPriorityToAccessCategory[UserPriority];

			
			
			
			if (pAd->CommonCfg.APEdcaParm.bACM[QueIdx])
			{
				UserPriority = 0;
				QueIdx		 = QID_AC_BE;
			}
		} while (FALSE);
	}

	RTMP_SET_PACKET_UP(pPacket, UserPriority);



	
	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
	if (pAd->TxSwQueue[QueIdx].Number >= MAX_PACKETS_IN_QUEUE)
	{
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#ifdef BLOCK_NET_IF
		StopNetIfQueue(pAd, QueIdx, pPacket);
#endif 
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);

		return NDIS_STATUS_FAILURE;
	}
	else
	{
		InsertTailQueueAc(pAd, pEntry, &pAd->TxSwQueue[QueIdx], PACKET_TO_QUEUE_ENTRY(pPacket));
	}
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

#ifdef DOT11_N_SUPPORT
    if ((pAd->CommonCfg.BACapability.field.AutoBA == TRUE)&&
        IS_HT_STA(pEntry))
	{
	    
		if (((pEntry->TXBAbitmap & (1<<UserPriority)) == 0) &&
            ((pEntry->BADeclineBitmap & (1<<UserPriority)) == 0) &&
            (pEntry->PortSecured == WPA_802_1X_PORT_SECURED)
			 
			 
			 
			 
			 && ((pEntry->ValidAsCLI && pAd->MlmeAux.APRalinkIe != 0x0) ||
				 (pEntry->WepStatus != Ndis802_11WEPEnabled && pEntry->WepStatus != Ndis802_11Encryption2Enabled))
			)
		{
			BAOriSessionSetUp(pAd, pEntry, UserPriority, 0, 10, FALSE);
		}
	}
#endif 

	pAd->RalinkCounters.OneSecOsTxCount[QueIdx]++; 
	return NDIS_STATUS_SUCCESS;
}



#ifdef RTMP_MAC_PCI
NDIS_STATUS RTMPFreeTXDRequest(
	IN		PRTMP_ADAPTER	pAd,
	IN		UCHAR			QueIdx,
	IN		UCHAR			NumberRequired,
	IN		PUCHAR			FreeNumberIs)
{
	ULONG		FreeNumber = 0;
	NDIS_STATUS	Status = NDIS_STATUS_FAILURE;

	switch (QueIdx)
	{
		case QID_AC_BK:
		case QID_AC_BE:
		case QID_AC_VI:
		case QID_AC_VO:
		
			if (pAd->TxRing[QueIdx].TxSwFreeIdx > pAd->TxRing[QueIdx].TxCpuIdx)
				FreeNumber = pAd->TxRing[QueIdx].TxSwFreeIdx - pAd->TxRing[QueIdx].TxCpuIdx - 1;
			else
				FreeNumber = pAd->TxRing[QueIdx].TxSwFreeIdx + TX_RING_SIZE - pAd->TxRing[QueIdx].TxCpuIdx - 1;

			if (FreeNumber >= NumberRequired)
				Status = NDIS_STATUS_SUCCESS;
			break;

		case QID_MGMT:
			if (pAd->MgmtRing.TxSwFreeIdx > pAd->MgmtRing.TxCpuIdx)
				FreeNumber = pAd->MgmtRing.TxSwFreeIdx - pAd->MgmtRing.TxCpuIdx - 1;
			else
				FreeNumber = pAd->MgmtRing.TxSwFreeIdx + MGMT_RING_SIZE - pAd->MgmtRing.TxCpuIdx - 1;

			if (FreeNumber >= NumberRequired)
				Status = NDIS_STATUS_SUCCESS;
			break;

		default:
			DBGPRINT(RT_DEBUG_ERROR,("RTMPFreeTXDRequest::Invalid QueIdx(=%d)\n", QueIdx));
			break;
	}
	*FreeNumberIs = (UCHAR)FreeNumber;

	return (Status);
}
#endif 



VOID RTMPSendDisassociationFrame(
	IN	PRTMP_ADAPTER	pAd)
{
}

VOID	RTMPSendNullFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			TxRate,
	IN	BOOLEAN			bQosNull)
{
	UCHAR	NullFrame[48];
	ULONG	Length;
	PHEADER_802_11	pHeader_802_11;


#ifdef RALINK_ATE
	if(ATE_ON(pAd))
	{
		return;
	}
#endif 

    
    if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) ||
         (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK) ||
         (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) ||
         (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef WPA_SUPPLICANT_SUPPORT
			  || (pAd->StaCfg.IEEE8021X == TRUE)
#endif
        ) &&
       (pAd->StaCfg.PortSecured == WPA_802_1X_PORT_NOT_SECURED))
	{
		return;
	}

	NdisZeroMemory(NullFrame, 48);
	Length = sizeof(HEADER_802_11);

	pHeader_802_11 = (PHEADER_802_11) NullFrame;

	pHeader_802_11->FC.Type = BTYPE_DATA;
	pHeader_802_11->FC.SubType = SUBTYPE_NULL_FUNC;
	pHeader_802_11->FC.ToDs = 1;
	COPY_MAC_ADDR(pHeader_802_11->Addr1, pAd->CommonCfg.Bssid);
	COPY_MAC_ADDR(pHeader_802_11->Addr2, pAd->CurrentAddress);
	COPY_MAC_ADDR(pHeader_802_11->Addr3, pAd->CommonCfg.Bssid);

	if (pAd->CommonCfg.bAPSDForcePowerSave)
	{
		pHeader_802_11->FC.PwrMgmt = PWR_SAVE;
	}
	else
	{
		pHeader_802_11->FC.PwrMgmt = (pAd->StaCfg.Psm == PWR_SAVE) ? 1: 0;
	}
	pHeader_802_11->Duration = pAd->CommonCfg.Dsifs + RTMPCalcDuration(pAd, TxRate, 14);

	pAd->Sequence++;
	pHeader_802_11->Sequence = pAd->Sequence;

	
	if (bQosNull)
	{
		pHeader_802_11->FC.SubType = SUBTYPE_QOS_NULL;

		
		NullFrame[Length]	=  0;
		NullFrame[Length+1] =  0;
		Length += 2;
	}

	HAL_KickOutNullFrameTx(pAd, 0, NullFrame, Length);

}


VOID	RTMPSendRTSFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pDA,
	IN	unsigned int	NextMpduSize,
	IN	UCHAR			TxRate,
	IN	UCHAR			RTSRate,
	IN	USHORT			AckDuration,
	IN	UCHAR			QueIdx,
	IN	UCHAR			FrameGap)
{
}












VOID STAFindCipherAlgorithm(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk)
{
	NDIS_802_11_ENCRYPTION_STATUS	Cipher;				
	UCHAR							CipherAlg = CIPHER_NONE;		
	UCHAR							KeyIdx = 0xff;
	PUCHAR							pSrcBufVA;
	PCIPHER_KEY						pKey = NULL;

	pSrcBufVA = GET_OS_PKT_DATAPTR(pTxBlk->pPacket);

	{
	    
	    if ((*pSrcBufVA & 0x01) && (ADHOC_ON(pAd)))
	        Cipher = pAd->StaCfg.GroupCipher; 
	    else
	        Cipher = pAd->StaCfg.PairCipher; 

		if (RTMP_GET_PACKET_EAPOL(pTxBlk->pPacket))
		{
			ASSERT(pAd->SharedKey[BSS0][0].CipherAlg <= CIPHER_CKIP128);

			
			if (!(TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame)) && (pAd->SharedKey[BSS0][0].CipherAlg) &&
				(pAd->SharedKey[BSS0][0].KeyLen))
			{
				CipherAlg = pAd->SharedKey[BSS0][0].CipherAlg;
				KeyIdx = 0;
			}
		}
		else if (Cipher == Ndis802_11Encryption1Enabled)
		{
				KeyIdx = pAd->StaCfg.DefaultKeyId;
		}
		else if ((Cipher == Ndis802_11Encryption2Enabled) ||
				 (Cipher == Ndis802_11Encryption3Enabled))
		{
			if ((*pSrcBufVA & 0x01) && (ADHOC_ON(pAd))) 
				KeyIdx = pAd->StaCfg.DefaultKeyId;
			else if (pAd->SharedKey[BSS0][0].KeyLen)
				KeyIdx = 0;
			else
				KeyIdx = pAd->StaCfg.DefaultKeyId;
		}

		if (KeyIdx == 0xff)
			CipherAlg = CIPHER_NONE;
		else if ((Cipher == Ndis802_11EncryptionDisabled) || (pAd->SharedKey[BSS0][KeyIdx].KeyLen == 0))
			CipherAlg = CIPHER_NONE;
#ifdef WPA_SUPPLICANT_SUPPORT
	    else if ( pAd->StaCfg.WpaSupplicantUP &&
	             (Cipher == Ndis802_11Encryption1Enabled) &&
	             (pAd->StaCfg.IEEE8021X == TRUE) &&
	             (pAd->StaCfg.PortSecured == WPA_802_1X_PORT_NOT_SECURED))
	        CipherAlg = CIPHER_NONE;
#endif 
		else
		{
			
			CipherAlg = pAd->SharedKey[BSS0][KeyIdx].CipherAlg;
			pKey = &pAd->SharedKey[BSS0][KeyIdx];
		}
	}

	pTxBlk->CipherAlg = CipherAlg;
	pTxBlk->pKey = pKey;
}


VOID STABuildCommon802_11Header(
	IN  PRTMP_ADAPTER   pAd,
	IN  TX_BLK          *pTxBlk)
{

	HEADER_802_11	*pHeader_802_11;
#ifdef QOS_DLS_SUPPORT
	BOOLEAN	bDLSFrame = FALSE;
	INT	DlsEntryIndex = 0;
#endif 

	
	
	

	
	pTxBlk->MpduHeaderLen = sizeof(HEADER_802_11);

	pHeader_802_11 = (HEADER_802_11 *) &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWI_SIZE];

	NdisZeroMemory(pHeader_802_11, sizeof(HEADER_802_11));

	pHeader_802_11->FC.FrDs = 0;
	pHeader_802_11->FC.Type = BTYPE_DATA;
	pHeader_802_11->FC.SubType = ((TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) ? SUBTYPE_QDATA : SUBTYPE_DATA);

#ifdef QOS_DLS_SUPPORT
	if (INFRA_ON(pAd))
	{
		
		
		DlsEntryIndex = RTMPCheckDLSFrame(pAd, pTxBlk->pSrcBufHeader);
		if (DlsEntryIndex >= 0)
			bDLSFrame = TRUE;
		else
			bDLSFrame = FALSE;
	}
#endif 

    if (pTxBlk->pMacEntry)
	{
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bForceNonQoS))
		{
			pHeader_802_11->Sequence = pTxBlk->pMacEntry->NonQosDataSeq;
			pTxBlk->pMacEntry->NonQosDataSeq = (pTxBlk->pMacEntry->NonQosDataSeq+1) & MAXSEQ;
		}
		else
		{
			{
	    pHeader_802_11->Sequence = pTxBlk->pMacEntry->TxSeq[pTxBlk->UserPriority];
	    pTxBlk->pMacEntry->TxSeq[pTxBlk->UserPriority] = (pTxBlk->pMacEntry->TxSeq[pTxBlk->UserPriority]+1) & MAXSEQ;
	}
	}
	}
	else
	{
		pHeader_802_11->Sequence = pAd->Sequence;
		pAd->Sequence = (pAd->Sequence+1) & MAXSEQ; 
	}

	pHeader_802_11->Frag = 0;

	pHeader_802_11->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);

	{
		if (INFRA_ON(pAd))
		{
#ifdef QOS_DLS_SUPPORT
			if (bDLSFrame)
			{
				COPY_MAC_ADDR(pHeader_802_11->Addr1, pTxBlk->pSrcBufHeader);
				COPY_MAC_ADDR(pHeader_802_11->Addr2, pAd->CurrentAddress);
				COPY_MAC_ADDR(pHeader_802_11->Addr3, pAd->CommonCfg.Bssid);
				pHeader_802_11->FC.ToDs = 0;
			}
			else
#endif 
			{
			COPY_MAC_ADDR(pHeader_802_11->Addr1, pAd->CommonCfg.Bssid);
			COPY_MAC_ADDR(pHeader_802_11->Addr2, pAd->CurrentAddress);
			COPY_MAC_ADDR(pHeader_802_11->Addr3, pTxBlk->pSrcBufHeader);
			pHeader_802_11->FC.ToDs = 1;
		}
		}
		else if (ADHOC_ON(pAd))
		{
			COPY_MAC_ADDR(pHeader_802_11->Addr1, pTxBlk->pSrcBufHeader);
			COPY_MAC_ADDR(pHeader_802_11->Addr2, pAd->CurrentAddress);
			COPY_MAC_ADDR(pHeader_802_11->Addr3, pAd->CommonCfg.Bssid);
			pHeader_802_11->FC.ToDs = 0;
		}
	}

	if (pTxBlk->CipherAlg != CIPHER_NONE)
		pHeader_802_11->FC.Wep = 1;

	
	
	
	if (pAd->CommonCfg.bAPSDForcePowerSave)
	pHeader_802_11->FC.PwrMgmt = PWR_SAVE;
	else
	pHeader_802_11->FC.PwrMgmt = (pAd->StaCfg.Psm == PWR_SAVE);
}

#ifdef DOT11_N_SUPPORT
VOID STABuildCache802_11Header(
	IN RTMP_ADAPTER		*pAd,
	IN TX_BLK			*pTxBlk,
	IN UCHAR			*pHeader)
{
	MAC_TABLE_ENTRY	*pMacEntry;
	PHEADER_802_11	pHeader80211;

	pHeader80211 = (PHEADER_802_11)pHeader;
	pMacEntry = pTxBlk->pMacEntry;

	
	
	

	
	pTxBlk->MpduHeaderLen = sizeof(HEADER_802_11);

	
	pHeader80211->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);

	
	pHeader80211->Sequence = pMacEntry->TxSeq[pTxBlk->UserPriority];
    pMacEntry->TxSeq[pTxBlk->UserPriority] = (pMacEntry->TxSeq[pTxBlk->UserPriority]+1) & MAXSEQ;

	{
		
		
#ifdef QOS_DLS_SUPPORT
		BOOLEAN	bDLSFrame = FALSE;
		INT	DlsEntryIndex = 0;

		DlsEntryIndex = RTMPCheckDLSFrame(pAd, pTxBlk->pSrcBufHeader);
		if (DlsEntryIndex >= 0)
			bDLSFrame = TRUE;
		else
			bDLSFrame = FALSE;
#endif 

		
#ifdef QOS_DLS_SUPPORT
		if (bDLSFrame)
		{
			COPY_MAC_ADDR(pHeader80211->Addr1, pTxBlk->pSrcBufHeader);
			COPY_MAC_ADDR(pHeader80211->Addr3, pAd->CommonCfg.Bssid);
			pHeader80211->FC.ToDs = 0;
		}
		else
#endif 
		if (ADHOC_ON(pAd))
			COPY_MAC_ADDR(pHeader80211->Addr3, pAd->CommonCfg.Bssid);
		else
		COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);
	}

	
	
	
	if (pAd->CommonCfg.bAPSDForcePowerSave)
	pHeader80211->FC.PwrMgmt = PWR_SAVE;
	else
	pHeader80211->FC.PwrMgmt = (pAd->StaCfg.Psm == PWR_SAVE);
}
#endif 

static inline PUCHAR STA_Build_ARalink_Frame_Header(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK		*pTxBlk)
{
	PUCHAR			pHeaderBufPtr;
	HEADER_802_11	*pHeader_802_11;
	PNDIS_PACKET	pNextPacket;
	UINT32			nextBufLen;
	PQUEUE_ENTRY	pQEntry;

	STAFindCipherAlgorithm(pAd, pTxBlk);
	STABuildCommon802_11Header(pAd, pTxBlk);


	pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWI_SIZE];
	pHeader_802_11 = (HEADER_802_11 *) pHeaderBufPtr;

	
	pHeader_802_11->FC.Order = 1;

	
	pHeaderBufPtr += pTxBlk->MpduHeaderLen;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM))
	{
		
		
		
		*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);

		*(pHeaderBufPtr+1) = 0;
		pHeaderBufPtr +=2;
		pTxBlk->MpduHeaderLen += 2;
	}

	
	pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
	pHeaderBufPtr = (PUCHAR)ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);

	
	
	pQEntry = pTxBlk->TxPacketList.Head;
	pNextPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	nextBufLen = GET_OS_PKT_LEN(pNextPacket);
	if (RTMP_GET_PACKET_VLAN(pNextPacket))
		nextBufLen -= LENGTH_802_1Q;

	*pHeaderBufPtr = (UCHAR)nextBufLen & 0xff;
	*(pHeaderBufPtr+1) = (UCHAR)(nextBufLen >> 8);

	pHeaderBufPtr += 2;
	pTxBlk->MpduHeaderLen += 2;

	return pHeaderBufPtr;

}

#ifdef DOT11_N_SUPPORT
static inline PUCHAR STA_Build_AMSDU_Frame_Header(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK		*pTxBlk)
{
	PUCHAR			pHeaderBufPtr;
	HEADER_802_11	*pHeader_802_11;


	STAFindCipherAlgorithm(pAd, pTxBlk);
	STABuildCommon802_11Header(pAd, pTxBlk);

	pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWI_SIZE];
	pHeader_802_11 = (HEADER_802_11 *) pHeaderBufPtr;

	
	pHeaderBufPtr += pTxBlk->MpduHeaderLen;

	
	
	
	*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);

	
	
	
	*pHeaderBufPtr |= 0x80;

	*(pHeaderBufPtr+1) = 0;
	pHeaderBufPtr +=2;
	pTxBlk->MpduHeaderLen += 2;

	

	
	
	
	
	
	
	pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
	pHeaderBufPtr = (PUCHAR) ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);

	return pHeaderBufPtr;

}


VOID STA_AMPDU_Frame_Tx(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk)
{
	HEADER_802_11	*pHeader_802_11;
	PUCHAR			pHeaderBufPtr;
	USHORT			FreeNumber;
	MAC_TABLE_ENTRY	*pMacEntry;
	BOOLEAN			bVLANPkt;
	PQUEUE_ENTRY	pQEntry;

	ASSERT(pTxBlk);

	while(pTxBlk->TxPacketList.Head)
	{
		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		if ( RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE)
		{
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			continue;
		}

		bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

		pMacEntry = pTxBlk->pMacEntry;
		if (pMacEntry->isCached)
		{
			
			NdisMoveMemory((PUCHAR)&pTxBlk->HeaderBuf[TXINFO_SIZE], (PUCHAR)&pMacEntry->CachedBuf[0], TXWI_SIZE + sizeof(HEADER_802_11));
			pHeaderBufPtr = (PUCHAR)(&pTxBlk->HeaderBuf[TXINFO_SIZE + TXWI_SIZE]);
			STABuildCache802_11Header(pAd, pTxBlk, pHeaderBufPtr);
		}
		else
		{
			STAFindCipherAlgorithm(pAd, pTxBlk);
			STABuildCommon802_11Header(pAd, pTxBlk);

			pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWI_SIZE];
		}


		pHeader_802_11 = (HEADER_802_11 *) pHeaderBufPtr;

		
		pHeaderBufPtr += pTxBlk->MpduHeaderLen;

		
		
		
		*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
		*(pHeaderBufPtr+1) = 0;
		pHeaderBufPtr +=2;
		pTxBlk->MpduHeaderLen += 2;

		
		
		
		
		if ((pAd->CommonCfg.bRdg == TRUE) && CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_RDG_CAPABLE))
		{
			if (pMacEntry->isCached == FALSE)
			{
				
				pHeader_802_11->FC.Order = 1;

				NdisZeroMemory(pHeaderBufPtr, 4);
				*(pHeaderBufPtr+3) |= 0x80;
			}
			pHeaderBufPtr += 4;
			pTxBlk->MpduHeaderLen += 4;
		}

		
		ASSERT(pTxBlk->MpduHeaderLen >= 24);

		
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
		pTxBlk->SrcBufLen  -= LENGTH_802_3;

		
		if (bVLANPkt)
		{
			pTxBlk->pSrcBufData	+= LENGTH_802_1Q;
			pTxBlk->SrcBufLen	-= LENGTH_802_1Q;
		}

		
		
		
		
		
		
		pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
		pHeaderBufPtr = (PUCHAR) ROUND_UP(pHeaderBufPtr, 4);
		pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);

		{

			
			
			
			EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData-2, pTxBlk->pExtraLlcSnapEncap);
			if (pTxBlk->pExtraLlcSnapEncap)
			{
				NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
				pHeaderBufPtr += 6;
				
				NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufData-2, 2);
				pHeaderBufPtr += 2;
				pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
			}

		}

		if (pMacEntry->isCached)
		{
            RTMPWriteTxWI_Cache(pAd, (PTXWI_STRUC)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);
		}
		else
		{
			RTMPWriteTxWI_Data(pAd, (PTXWI_STRUC)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);

			NdisZeroMemory((PUCHAR)(&pMacEntry->CachedBuf[0]), sizeof(pMacEntry->CachedBuf));
			NdisMoveMemory((PUCHAR)(&pMacEntry->CachedBuf[0]), (PUCHAR)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (pHeaderBufPtr - (PUCHAR)(&pTxBlk->HeaderBuf[TXINFO_SIZE])));
			pMacEntry->isCached = TRUE;
		}

		
		{
			pAd->RalinkCounters.TransmittedMPDUsInAMPDUCount.u.LowPart ++;
			pAd->RalinkCounters.TransmittedOctetsInAMPDUCount.QuadPart += pTxBlk->SrcBufLen;
		}

		

		HAL_WriteTxResource(pAd, pTxBlk, TRUE, &FreeNumber);

		
		
		
                if (!RTMP_TEST_PSFLAG(pAd, fRTMP_PS_DISABLE_TX))
		HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;
	}

}


VOID STA_AMSDU_Frame_Tx(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk)
{
	PUCHAR			pHeaderBufPtr;
	USHORT			FreeNumber;
	USHORT			subFramePayloadLen = 0;	
	USHORT			totalMPDUSize=0;
	UCHAR			*subFrameHeader;
	UCHAR			padding = 0;
	USHORT			FirstTx = 0, LastTxIdx = 0;
	BOOLEAN			bVLANPkt;
	int			frameNum = 0;
	PQUEUE_ENTRY	pQEntry;


	ASSERT(pTxBlk);

	ASSERT((pTxBlk->TxPacketList.Number > 1));

	while(pTxBlk->TxPacketList.Head)
	{
		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE)
		{
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			continue;
		}

		bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

		
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
		pTxBlk->SrcBufLen  -= LENGTH_802_3;

		
		if (bVLANPkt)
		{
			pTxBlk->pSrcBufData	+= LENGTH_802_1Q;
			pTxBlk->SrcBufLen	-= LENGTH_802_1Q;
		}

		if (frameNum == 0)
		{
			pHeaderBufPtr = STA_Build_AMSDU_Frame_Header(pAd, pTxBlk);

			
			RTMPWriteTxWI_Data(pAd, (PTXWI_STRUC)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);
		}
		else
		{
			pHeaderBufPtr = &pTxBlk->HeaderBuf[0];
			padding = ROUND_UP(LENGTH_AMSDU_SUBFRAMEHEAD + subFramePayloadLen, 4) - (LENGTH_AMSDU_SUBFRAMEHEAD + subFramePayloadLen);
			NdisZeroMemory(pHeaderBufPtr, padding + LENGTH_AMSDU_SUBFRAMEHEAD);
			pHeaderBufPtr += padding;
			pTxBlk->MpduHeaderLen = padding;
		}

		
		
		
		
		subFrameHeader = pHeaderBufPtr;
		subFramePayloadLen = pTxBlk->SrcBufLen;

		NdisMoveMemory(subFrameHeader, pTxBlk->pSrcBufHeader, 12);


		pHeaderBufPtr += LENGTH_AMSDU_SUBFRAMEHEAD;
		pTxBlk->MpduHeaderLen += LENGTH_AMSDU_SUBFRAMEHEAD;


		
		
		
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData-2, pTxBlk->pExtraLlcSnapEncap);

		subFramePayloadLen = pTxBlk->SrcBufLen;

		if (pTxBlk->pExtraLlcSnapEncap)
		{
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
			pHeaderBufPtr += 6;
			
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufData-2, 2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
			subFramePayloadLen += LENGTH_802_1_H;
		}

		
		subFrameHeader[12] = (subFramePayloadLen & 0xFF00) >> 8;
		subFrameHeader[13] = subFramePayloadLen & 0xFF;

		totalMPDUSize += pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;

		if (frameNum ==0)
			FirstTx = HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum, &FreeNumber);
		else
			LastTxIdx = HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum, &FreeNumber);

		frameNum++;

		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;

		
		{
			pAd->RalinkCounters.TransmittedAMSDUCount.u.LowPart ++;
			pAd->RalinkCounters.TransmittedOctetsInAMSDU.QuadPart += totalMPDUSize;
		}

	}

	HAL_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, FirstTx);
	HAL_LastTxIdx(pAd, pTxBlk->QueIdx, LastTxIdx);

	
	
	
        if (!RTMP_TEST_PSFLAG(pAd, fRTMP_PS_DISABLE_TX))
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
}
#endif 

VOID STA_Legacy_Frame_Tx(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk)
{
	HEADER_802_11	*pHeader_802_11;
	PUCHAR			pHeaderBufPtr;
	USHORT			FreeNumber;
	BOOLEAN			bVLANPkt;
	PQUEUE_ENTRY	pQEntry;

	ASSERT(pTxBlk);


	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE)
	{
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return;
	}

	if (pTxBlk->TxFrameType == TX_MCAST_FRAME)
	{
		INC_COUNTER64(pAd->WlanCounters.MulticastTransmittedFrameCount);
	}

	if (RTMP_GET_PACKET_RTS(pTxBlk->pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bRtsRequired);
	else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bRtsRequired);

	bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

	if (pTxBlk->TxRate < pAd->CommonCfg.MinTxRate)
		pTxBlk->TxRate = pAd->CommonCfg.MinTxRate;

	STAFindCipherAlgorithm(pAd, pTxBlk);
	STABuildCommon802_11Header(pAd, pTxBlk);


	
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
	pTxBlk->SrcBufLen  -= LENGTH_802_3;

	
	if (bVLANPkt)
	{
		pTxBlk->pSrcBufData	+= LENGTH_802_1Q;
		pTxBlk->SrcBufLen	-= LENGTH_802_1Q;
	}

	pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWI_SIZE];
	pHeader_802_11 = (HEADER_802_11 *) pHeaderBufPtr;

	
	pHeaderBufPtr += pTxBlk->MpduHeaderLen;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM))
	{
		
		
		
		*(pHeaderBufPtr) = ((pTxBlk->UserPriority & 0x0F) | (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx]<<5));
		*(pHeaderBufPtr+1) = 0;
		pHeaderBufPtr +=2;
		pTxBlk->MpduHeaderLen += 2;
	}

	
	pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
	pHeaderBufPtr = (PUCHAR) ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);

	{

		
		
		
		
		
		
		
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(pTxBlk->pSrcBufHeader, pTxBlk->pExtraLlcSnapEncap);
		if (pTxBlk->pExtraLlcSnapEncap)
		{
			UCHAR vlan_size;

			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
			pHeaderBufPtr += 6;
			
			vlan_size =  (bVLANPkt) ? LENGTH_802_1Q : 0;
			
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufHeader+12+vlan_size, 2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
		}

	}

	
	
	
	

	RTMPWriteTxWI_Data(pAd, (PTXWI_STRUC)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);

	

	HAL_WriteTxResource(pAd, pTxBlk, TRUE, &FreeNumber);

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

	
	
	
	if (!RTMP_TEST_PSFLAG(pAd, fRTMP_PS_DISABLE_TX))
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
}


VOID STA_ARalink_Frame_Tx(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk)
{
	PUCHAR			pHeaderBufPtr;
	USHORT			FreeNumber;
	USHORT			totalMPDUSize=0;
	USHORT			FirstTx, LastTxIdx;
	int			frameNum = 0;
	BOOLEAN			bVLANPkt;
	PQUEUE_ENTRY	pQEntry;


	ASSERT(pTxBlk);

	ASSERT((pTxBlk->TxPacketList.Number== 2));


	FirstTx = LastTxIdx = 0;  
	while(pTxBlk->TxPacketList.Head)
	{
		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

		if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE)
		{
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			continue;
		}

		bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

		
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
		pTxBlk->SrcBufLen  -= LENGTH_802_3;

		
		if (bVLANPkt)
		{
			pTxBlk->pSrcBufData	+= LENGTH_802_1Q;
			pTxBlk->SrcBufLen	-= LENGTH_802_1Q;
		}

		if (frameNum == 0)
		{	

			pHeaderBufPtr = STA_Build_ARalink_Frame_Header(pAd, pTxBlk);

			
			
			RTMPWriteTxWI_Data(pAd, (PTXWI_STRUC)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);


			
			
			
			EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData-2, pTxBlk->pExtraLlcSnapEncap);

			if (pTxBlk->pExtraLlcSnapEncap)
			{
				NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
				pHeaderBufPtr += 6;
				
				NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufData-2, 2);
				pHeaderBufPtr += 2;
				pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
			}
		}
		else
		{	

			pHeaderBufPtr = &pTxBlk->HeaderBuf[0];
			pTxBlk->MpduHeaderLen = 0;

			
			
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufHeader, 12);
			pHeaderBufPtr += 12;
			
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufData-2, 2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen = LENGTH_ARALINK_SUBFRAMEHEAD;
		}

		totalMPDUSize += pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;

		
		if (frameNum ==0)
			FirstTx = HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum, &FreeNumber);
		else
			LastTxIdx = HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum, &FreeNumber);

		frameNum++;

		pAd->RalinkCounters.OneSecTxAggregationCount++;
		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;

	}

	HAL_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, FirstTx);
	HAL_LastTxIdx(pAd, pTxBlk->QueIdx, LastTxIdx);

	
	
	
	if (!RTMP_TEST_PSFLAG(pAd, fRTMP_PS_DISABLE_TX))
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

}


VOID STA_Fragment_Frame_Tx(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK		*pTxBlk)
{
	HEADER_802_11	*pHeader_802_11;
	PUCHAR			pHeaderBufPtr;
	USHORT			FreeNumber;
	UCHAR			fragNum = 0;
	PACKET_INFO		PacketInfo;
	USHORT			EncryptionOverhead = 0;
	UINT32			FreeMpduSize, SrcRemainingBytes;
	USHORT			AckDuration;
	UINT			NextMpduSize;
	BOOLEAN			bVLANPkt;
	PQUEUE_ENTRY	pQEntry;
	HTTRANSMIT_SETTING	*pTransmit;


	ASSERT(pTxBlk);

	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE)
	{
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return;
	}

	ASSERT(TX_BLK_TEST_FLAG(pTxBlk, fTX_bAllowFrag));
	bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);

	STAFindCipherAlgorithm(pAd, pTxBlk);
	STABuildCommon802_11Header(pAd, pTxBlk);

	if (pTxBlk->CipherAlg == CIPHER_TKIP)
	{
		pTxBlk->pPacket = duplicate_pkt_with_TKIP_MIC(pAd, pTxBlk->pPacket);
		if (pTxBlk->pPacket == NULL)
			return;
		RTMP_QueryPacketInfo(pTxBlk->pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
	}

	
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
	pTxBlk->SrcBufLen  -= LENGTH_802_3;


	
	if (bVLANPkt)
	{
		pTxBlk->pSrcBufData	+= LENGTH_802_1Q;
		pTxBlk->SrcBufLen	-= LENGTH_802_1Q;
	}

	pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWI_SIZE];
	pHeader_802_11 = (HEADER_802_11 *)pHeaderBufPtr;


	
	pHeaderBufPtr += pTxBlk->MpduHeaderLen;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM))
	{
		
		
		
		*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);

		*(pHeaderBufPtr+1) = 0;
		pHeaderBufPtr +=2;
		pTxBlk->MpduHeaderLen += 2;
	}

	
	
	
	
	pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
	pHeaderBufPtr = (PUCHAR) ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);



	
	
	
	
	
	
	
	EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(pTxBlk->pSrcBufHeader, pTxBlk->pExtraLlcSnapEncap);
	if (pTxBlk->pExtraLlcSnapEncap)
	{
		UCHAR vlan_size;

		NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
		pHeaderBufPtr += 6;
		
		vlan_size =  (bVLANPkt) ? LENGTH_802_1Q : 0;
		
		NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufHeader+12+vlan_size, 2);
		pHeaderBufPtr += 2;
		pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
	}


	
	
	
	if (pTxBlk->CipherAlg == CIPHER_TKIP)
	{
		RTMPCalculateMICValue(pAd, pTxBlk->pPacket, pTxBlk->pExtraLlcSnapEncap, pTxBlk->pKey, 0);

		
		
		NdisMoveMemory(pTxBlk->pSrcBufData + pTxBlk->SrcBufLen, &pAd->PrivateInfo.Tx.MIC[0], 8);
		
		pTxBlk->SrcBufLen += 8;
		pTxBlk->TotalFrameLen += 8;
		pTxBlk->CipherAlg = CIPHER_TKIP_NO_MIC;
	}

	
	
	
	
	if ((pTxBlk->CipherAlg == CIPHER_WEP64) || (pTxBlk->CipherAlg == CIPHER_WEP128))
		EncryptionOverhead = 8; 
	else if (pTxBlk->CipherAlg == CIPHER_TKIP_NO_MIC)
		EncryptionOverhead = 12;
	else if (pTxBlk->CipherAlg == CIPHER_TKIP)
		EncryptionOverhead = 20;
	else if (pTxBlk->CipherAlg == CIPHER_AES)
		EncryptionOverhead = 16;	
	else
		EncryptionOverhead = 0;

	pTransmit = pTxBlk->pTransmit;
	
	if (pTransmit->field.MODE == MODE_CCK)
		pTxBlk->TxRate = pTransmit->field.MCS;
	else if (pTransmit->field.MODE == MODE_OFDM)
		pTxBlk->TxRate = pTransmit->field.MCS + RATE_FIRST_OFDM_RATE;
	else
		pTxBlk->TxRate = RATE_6_5;

	
	if (pTxBlk->TxRate <= RATE_LAST_OFDM_RATE)
		AckDuration = RTMPCalcDuration(pAd, pAd->CommonCfg.ExpectedACKRate[pTxBlk->TxRate], 14);
	else
		AckDuration = RTMPCalcDuration(pAd, RATE_6_5, 14);

	
	SrcRemainingBytes = pTxBlk->SrcBufLen;

	pTxBlk->TotalFragNum = 0xff;

	do {

		FreeMpduSize = pAd->CommonCfg.FragmentThreshold - LENGTH_CRC;

		FreeMpduSize -= pTxBlk->MpduHeaderLen;

		if (SrcRemainingBytes <= FreeMpduSize)
		{	

			pTxBlk->SrcBufLen = SrcRemainingBytes;

			pHeader_802_11->FC.MoreFrag = 0;
			pHeader_802_11->Duration = pAd->CommonCfg.Dsifs + AckDuration;

			
			pTxBlk->TotalFragNum = fragNum;
		}
		else
		{	

			pTxBlk->SrcBufLen = FreeMpduSize;

			NextMpduSize = min(((UINT)SrcRemainingBytes - pTxBlk->SrcBufLen), ((UINT)pAd->CommonCfg.FragmentThreshold));
			pHeader_802_11->FC.MoreFrag = 1;
			pHeader_802_11->Duration = (3 * pAd->CommonCfg.Dsifs) + (2 * AckDuration) + RTMPCalcDuration(pAd, pTxBlk->TxRate, NextMpduSize + EncryptionOverhead);
		}

		if (fragNum == 0)
			pTxBlk->FrameGap = IFS_HTTXOP;
		else
			pTxBlk->FrameGap = IFS_SIFS;

		RTMPWriteTxWI_Data(pAd, (PTXWI_STRUC)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), pTxBlk);

		HAL_WriteFragTxResource(pAd, pTxBlk, fragNum, &FreeNumber);

		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;

		

		
		if (fragNum == 0 && pTxBlk->pExtraLlcSnapEncap)
			pTxBlk->MpduHeaderLen -= LENGTH_802_1_H;

		fragNum++;
		SrcRemainingBytes -= pTxBlk->SrcBufLen;
		pTxBlk->pSrcBufData += pTxBlk->SrcBufLen;

		pHeader_802_11->Frag++;	 

	}while(SrcRemainingBytes > 0);

	
	
	
	if (!RTMP_TEST_PSFLAG(pAd, fRTMP_PS_DISABLE_TX))
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
}


#define RELEASE_FRAMES_OF_TXBLK(_pAd, _pTxBlk, _pQEntry, _Status)										\
		while(_pTxBlk->TxPacketList.Head)														\
		{																						\
			_pQEntry = RemoveHeadQueue(&_pTxBlk->TxPacketList);									\
			RELEASE_NDIS_PACKET(_pAd, QUEUE_ENTRY_TO_PACKET(_pQEntry), _Status);	\
		}



NDIS_STATUS STAHardTransmit(
	IN PRTMP_ADAPTER	pAd,
	IN TX_BLK			*pTxBlk,
	IN	UCHAR			QueIdx)
{
	NDIS_PACKET		*pPacket;
	PQUEUE_ENTRY	pQEntry;

	
	
	
	
	ASSERT(pTxBlk->TxPacketList.Number);
	if (pTxBlk->TxPacketList.Head == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("pTxBlk->TotalFrameNum == %ld!\n", pTxBlk->TxPacketList.Number));
		return NDIS_STATUS_FAILURE;
	}

	pPacket = QUEUE_ENTRY_TO_PACKET(pTxBlk->TxPacketList.Head);


	
	
	
	
	
	
	if ((pAd->StaCfg.Psm == PWR_SAVE) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
	{
	    DBGPRINT_RAW(RT_DEBUG_INFO, ("AsicForceWakeup At HardTx\n"));
#ifdef RTMP_MAC_PCI
		AsicForceWakeup(pAd, TRUE);
#endif 
	}

	
	if ((!(pAd->CommonCfg.bAPSDCapable && pAd->CommonCfg.APEdcaParm.bAPSDCapable) && (pAd->CommonCfg.bAPSDForcePowerSave == FALSE))
		|| (RTMP_GET_PACKET_EAPOL(pTxBlk->pPacket))
		|| (RTMP_GET_PACKET_WAI(pTxBlk->pPacket)))
	{
		if ((pAd->StaCfg.Psm == PWR_SAVE) &&
            (pAd->StaCfg.WindowsPowerMode == Ndis802_11PowerModeFast_PSP))
			RTMP_SET_PSM_BIT(pAd, PWR_ACTIVE);
	}

	switch (pTxBlk->TxFrameType)
	{
#ifdef DOT11_N_SUPPORT
		case TX_AMPDU_FRAME:
				STA_AMPDU_Frame_Tx(pAd, pTxBlk);
			break;
		case TX_AMSDU_FRAME:
				STA_AMSDU_Frame_Tx(pAd, pTxBlk);
			break;
#endif 
		case TX_LEGACY_FRAME:
				STA_Legacy_Frame_Tx(pAd, pTxBlk);
			break;
		case TX_MCAST_FRAME:
				STA_Legacy_Frame_Tx(pAd, pTxBlk);
			break;
		case TX_RALINK_FRAME:
				STA_ARalink_Frame_Tx(pAd, pTxBlk);
			break;
		case TX_FRAG_FRAME:
				STA_Fragment_Frame_Tx(pAd, pTxBlk);
			break;
		default:
			{
				
				DBGPRINT(RT_DEBUG_ERROR, ("Send a pacekt was not classified!! It should not happen!\n"));
				while(pTxBlk->TxPacketList.Number)
				{
					pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
					pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
					if (pPacket)
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
			}
			break;
	}

	return (NDIS_STATUS_SUCCESS);

}

ULONG  HashBytesPolynomial(UCHAR *value, unsigned int len)
{
   unsigned char *word = value;
   unsigned int ret = 0;
   unsigned int i;

   for(i=0; i < len; i++)
   {
	  int mod = i % 32;
	  ret ^=(unsigned int) (word[i]) << mod;
	  ret ^=(unsigned int) (word[i]) >> (32 - mod);
   }
   return ret;
}

VOID Sta_Announce_or_Forward_802_3_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	UCHAR			FromWhichBSSID)
{
	if (TRUE
		)
	{
		announce_802_3_packet(pAd, pPacket);
	}
	else
	{
		
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}
}
