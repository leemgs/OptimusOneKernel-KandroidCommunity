

#include "../rt_config.h"

#define MAX_TX_IN_TBTT		(16)


UCHAR	SNAP_802_1H[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
UCHAR	SNAP_BRIDGE_TUNNEL[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0xf8};

UCHAR	SNAP_AIRONET[] = {0xaa, 0xaa, 0x03, 0x00, 0x40, 0x96, 0x00, 0x00};
UCHAR	CKIP_LLC_SNAP[] = {0xaa, 0xaa, 0x03, 0x00, 0x40, 0x96, 0x00, 0x02};
UCHAR	EAPOL_LLC_SNAP[]= {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x88, 0x8e};
UCHAR	EAPOL[] = {0x88, 0x8e};
UCHAR   TPID[] = {0x81, 0x00}; 

UCHAR	IPX[] = {0x81, 0x37};
UCHAR	APPLE_TALK[] = {0x80, 0xf3};
UCHAR	RateIdToPlcpSignal[12] = {
	 0, 	1,  	2, 	3, 	
	11,    15,     10,    14, 	
	 9,   13, 	8,    12   }; 

UCHAR	 OfdmSignalToRateId[16] = {
	RATE_54,  RATE_54,	RATE_54,  RATE_54,	
	RATE_54,  RATE_54,	RATE_54,  RATE_54,	
	RATE_48,  RATE_24,	RATE_12,  RATE_6,	
	RATE_54,  RATE_36,	RATE_18,  RATE_9,	
};

UCHAR	 OfdmRateToRxwiMCS[12] = {
	0,  0,	0,  0,
	0,  1,	2,  3,	
	4,  5,	6,  7,	
};
UCHAR	 RxwiMCSToOfdmRate[12] = {
	RATE_6,  RATE_9,	RATE_12,  RATE_18,
	RATE_24,  RATE_36,	RATE_48,  RATE_54,	
	4,  5,	6,  7,	
};

char*   MCSToMbps[] = {"1Mbps","2Mbps","5.5Mbps","11Mbps","06Mbps","09Mbps","12Mbps","18Mbps","24Mbps","36Mbps","48Mbps","54Mbps","MM-0","MM-1","MM-2","MM-3","MM-4","MM-5","MM-6","MM-7","MM-8","MM-9","MM-10","MM-11","MM-12","MM-13","MM-14","MM-15","MM-32","ee1","ee2","ee3"};

UCHAR default_cwmin[]={CW_MIN_IN_BITS, CW_MIN_IN_BITS, CW_MIN_IN_BITS-1, CW_MIN_IN_BITS-2};
UCHAR default_sta_aifsn[]={3,7,2,2};

UCHAR MapUserPriorityToAccessCategory[8] = {QID_AC_BE, QID_AC_BK, QID_AC_BK, QID_AC_BE, QID_AC_VI, QID_AC_VI, QID_AC_VO, QID_AC_VO};



NDIS_STATUS MiniportMMRequest(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			QueIdx,
	IN	PUCHAR			pData,
	IN	UINT			Length)
{
	PNDIS_PACKET	pPacket;
	NDIS_STATUS  	Status = NDIS_STATUS_SUCCESS;
	ULONG	 		FreeNum;
#ifdef RT2860
	unsigned long	IrqFlags = 0;
#endif
	UCHAR			IrqState;
	UCHAR			rtmpHwHdr[TXINFO_SIZE + TXWI_SIZE]; 

	ASSERT(Length <= MGMT_DMA_BUFFER_SIZE);

	QueIdx=3;

	

	IrqState = pAd->irq_disabled;

#ifdef RT2860
	if ((pAd->MACVersion == 0x28600100) && (!IrqState))
		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
#endif
	do
	{
		
		if ( RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) ||
			 RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)||
			 !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		
		

		
		if (pAd->MACVersion == 0x28600100)
		{
			FreeNum = GET_TXRING_FREENO(pAd, QueIdx);
		}
		else
		{
			FreeNum = GET_MGMTRING_FREENO(pAd);
		}

		if ((FreeNum > 0))
		{
			
			NdisZeroMemory(&rtmpHwHdr, (TXINFO_SIZE + TXWI_SIZE));
			Status = RTMPAllocateNdisPacket(pAd, &pPacket, (PUCHAR)&rtmpHwHdr, (TXINFO_SIZE + TXWI_SIZE), pData, Length);
			if (Status != NDIS_STATUS_SUCCESS)
			{
				DBGPRINT(RT_DEBUG_WARN, ("MiniportMMRequest (error:: can't allocate NDIS PACKET)\n"));
				break;
			}

			
			


			Status = MlmeHardTransmit(pAd, QueIdx, pPacket);
			if (Status != NDIS_STATUS_SUCCESS)
				RTMPFreeNdisPacket(pAd, pPacket);
		}
		else
		{
			pAd->RalinkCounters.MgmtRingFullCount++;
			DBGPRINT(RT_DEBUG_ERROR, ("Qidx(%d), not enough space in MgmtRing, MgmtRingFullCount=%ld!\n",
										QueIdx, pAd->RalinkCounters.MgmtRingFullCount));
		}

	} while (FALSE);

#ifdef RT2860
	
	if ((pAd->MACVersion == 0x28600100) && (!IrqState))
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif
	return Status;
}

#ifdef RT2860
NDIS_STATUS MiniportMMRequestUnlock(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			QueIdx,
	IN	PUCHAR			pData,
	IN	UINT			Length)
{
	PNDIS_PACKET	pPacket;
	NDIS_STATUS  Status = NDIS_STATUS_SUCCESS;
	ULONG	 FreeNum;
	TXWI_STRUC		TXWI;
	ULONG	SW_TX_IDX;
	PTXD_STRUC		pTxD;

	QueIdx = 3;
	ASSERT(Length <= MGMT_DMA_BUFFER_SIZE);

	do
	{
		
		if ( RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) ||
			 RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)||
			 !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		
		
		
		if (pAd->MACVersion == 0x28600100)
		{
			FreeNum = GET_TXRING_FREENO(pAd, QueIdx);
			SW_TX_IDX = pAd->TxRing[QueIdx].TxCpuIdx;
			pTxD  = (PTXD_STRUC) pAd->TxRing[QueIdx].Cell[SW_TX_IDX].AllocVa;
		}
		else
		{
			FreeNum = GET_MGMTRING_FREENO(pAd);
			SW_TX_IDX = pAd->MgmtRing.TxCpuIdx;
			pTxD  = (PTXD_STRUC) pAd->MgmtRing.Cell[SW_TX_IDX].AllocVa;
		}
		if ((FreeNum > 0))
		{
			NdisZeroMemory(&TXWI, TXWI_SIZE);
			Status = RTMPAllocateNdisPacket(pAd, &pPacket, (PUCHAR)&TXWI, TXWI_SIZE, pData, Length);
			if (Status != NDIS_STATUS_SUCCESS)
			{
				DBGPRINT(RT_DEBUG_WARN, ("MiniportMMRequest (error:: can't allocate NDIS PACKET)\n"));
				break;
			}

			Status = MlmeHardTransmit(pAd, QueIdx, pPacket);
			if (Status != NDIS_STATUS_SUCCESS)
				RTMPFreeNdisPacket(pAd, pPacket);
		}
		else
		{
			pAd->RalinkCounters.MgmtRingFullCount++;
			DBGPRINT(RT_DEBUG_ERROR, ("Qidx(%d), not enough space in MgmtRing\n", QueIdx));
		}

	} while (FALSE);


	return Status;
}
#endif


NDIS_STATUS MlmeHardTransmit(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			QueIdx,
	IN	PNDIS_PACKET	pPacket)
{
	if (pAd->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE)
	{
		return NDIS_STATUS_FAILURE;
	}

#ifdef RT2860
	if ( pAd->MACVersion == 0x28600100 )
		return MlmeHardTransmitTxRing(pAd,QueIdx,pPacket);
	else
#endif
		return MlmeHardTransmitMgmtRing(pAd,QueIdx,pPacket);

}

#ifdef RT2860
NDIS_STATUS MlmeHardTransmitTxRing(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	QueIdx,
	IN	PNDIS_PACKET	pPacket)
{
	PACKET_INFO 	PacketInfo;
	PUCHAR			pSrcBufVA;
	UINT			SrcBufLen;
	PTXD_STRUC		pTxD;
	PHEADER_802_11	pHeader_802_11;
	BOOLEAN 		bAckRequired, bInsertTimestamp;
	ULONG			SrcBufPA;
	UCHAR			MlmeRate;
	ULONG			SwIdx = pAd->TxRing[QueIdx].TxCpuIdx;
	PTXWI_STRUC 	pFirstTxWI;
	ULONG	 FreeNum;
	MAC_TABLE_ENTRY	*pMacEntry = NULL;


	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	if (pSrcBufVA == NULL)
	{
		
		return NDIS_STATUS_FAILURE;
	}

	
	

	FreeNum = GET_TXRING_FREENO(pAd, QueIdx);

	if (FreeNum == 0)
	{
		
		return NDIS_STATUS_FAILURE;
	}

	SwIdx = pAd->TxRing[QueIdx].TxCpuIdx;

	pTxD  = (PTXD_STRUC) pAd->TxRing[QueIdx].Cell[SwIdx].AllocVa;

	if (pAd->TxRing[QueIdx].Cell[SwIdx].pNdisPacket)
	{
		printk("MlmeHardTransmit Error\n");
		return NDIS_STATUS_FAILURE;
	}

	
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
		AsicForceWakeup(pAd, FROM_TX);

	pFirstTxWI	=(PTXWI_STRUC)pSrcBufVA;

	pHeader_802_11 = (PHEADER_802_11) (pSrcBufVA + TXWI_SIZE);
	if (pHeader_802_11->Addr1[0] & 0x01)
	{
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	}
	else
	{
		MlmeRate = pAd->CommonCfg.MlmeRate;
	}

	if ((pHeader_802_11->FC.Type == BTYPE_DATA) &&
		(pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
	{
		pMacEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
	}

	
	if ((pAd->LatchRfRegs.Channel > 14) && (MlmeRate < RATE_6)) 
		MlmeRate = RATE_6;

	
	
	
	
	
	
	

    
	if (pHeader_802_11->FC.Type != BTYPE_DATA)
    {
    	if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_REQ) || !(pAd->CommonCfg.bAPSDCapable && pAd->CommonCfg.APEdcaParm.bAPSDCapable))
    	{
    		pHeader_802_11->FC.PwrMgmt = PWR_ACTIVE;
    	}
    	else
    	{
    		pHeader_802_11->FC.PwrMgmt = pAd->CommonCfg.bAPSDForcePowerSave;
    	}
    }

	bInsertTimestamp = FALSE;
	if (pHeader_802_11->FC.Type == BTYPE_CNTL) 
	{
		bAckRequired = FALSE;
	}
	else 
	{
		if (pHeader_802_11->Addr1[0] & 0x01) 
		{
			bAckRequired = FALSE;
			pHeader_802_11->Duration = 0;
		}
		else
		{
			bAckRequired = TRUE;
			pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, 14);
			if (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP)
			{
				bInsertTimestamp = TRUE;
			}
		}
	}
	pHeader_802_11->Sequence = pAd->Sequence++;
	if (pAd->Sequence > 0xfff)
		pAd->Sequence = 0;
	
	
	if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pAd->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE))
	{
		DBGPRINT(RT_DEBUG_ERROR,("MlmeHardTransmit --> radar detect not in normal mode !!!\n"));
		return (NDIS_STATUS_FAILURE);
	}

	
	
	
	
	

	
	
	


	
	if (pMacEntry == NULL)
	{
		RTMPWriteTxWI(pAd, pFirstTxWI, FALSE, FALSE, bInsertTimestamp, FALSE, bAckRequired, FALSE,
		0, RESERVED_WCID, (SrcBufLen - TXWI_SIZE), PID_MGMT, 0,  (UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS, IFS_BACKOFF, FALSE, &pAd->CommonCfg.MlmeTransmit);
	}
	else
	{
		RTMPWriteTxWI(pAd, pFirstTxWI, FALSE, FALSE,
					bInsertTimestamp, FALSE, bAckRequired, FALSE,
					0, pMacEntry->Aid, (SrcBufLen - TXWI_SIZE),
					pMacEntry->MaxHTPhyMode.field.MCS, 0,
					(UCHAR)pMacEntry->MaxHTPhyMode.field.MCS,
					IFS_BACKOFF, FALSE, &pMacEntry->MaxHTPhyMode);
	}

	pAd->TxRing[QueIdx].Cell[SwIdx].pNdisPacket = pPacket;
	pAd->TxRing[QueIdx].Cell[SwIdx].pNextNdisPacket = NULL;

	SrcBufPA = PCI_MAP_SINGLE(pAd, pSrcBufVA, SrcBufLen, 0, PCI_DMA_TODEVICE);


	RTMPWriteTxDescriptor(pAd, pTxD, TRUE, FIFO_EDCA);
	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 1;
	pTxD->SDLen0 = SrcBufLen;
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = SrcBufPA;
	pTxD->DMADONE = 0;

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

   	
	INC_RING_INDEX(pAd->TxRing[QueIdx].TxCpuIdx, TX_RING_SIZE);

	RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QueIdx*0x10,  pAd->TxRing[QueIdx].TxCpuIdx);

	return NDIS_STATUS_SUCCESS;
}
#endif 

NDIS_STATUS MlmeHardTransmitMgmtRing(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	QueIdx,
	IN	PNDIS_PACKET	pPacket)
{
	PACKET_INFO 	PacketInfo;
	PUCHAR			pSrcBufVA;
	UINT			SrcBufLen;
	PHEADER_802_11	pHeader_802_11;
	BOOLEAN 		bAckRequired, bInsertTimestamp;
	UCHAR			MlmeRate;
	PTXWI_STRUC 	pFirstTxWI;
	MAC_TABLE_ENTRY	*pMacEntry = NULL;

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
		RTMP_SEM_LOCK(&pAd->MgmtRingLock);


	if (pSrcBufVA == NULL)
	{
		RTMP_SEM_UNLOCK(&pAd->MgmtRingLock);
		return NDIS_STATUS_FAILURE;
	}

	
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
#ifdef RT2860
		AsicForceWakeup(pAd, FROM_TX);
#endif
#ifdef RT2870
		AsicForceWakeup(pAd, TRUE);
#endif

	pFirstTxWI = (PTXWI_STRUC)(pSrcBufVA +  TXINFO_SIZE);
	pHeader_802_11 = (PHEADER_802_11) (pSrcBufVA + TXINFO_SIZE + TXWI_SIZE); 

	if (pHeader_802_11->Addr1[0] & 0x01)
	{
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	}
	else
	{
		MlmeRate = pAd->CommonCfg.MlmeRate;
	}

	
	if ((pAd->LatchRfRegs.Channel > 14) && (MlmeRate < RATE_6)) 
		MlmeRate = RATE_6;

	if ((pHeader_802_11->FC.Type == BTYPE_DATA) &&
		(pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
	{
		pMacEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
	}

	{
		
		if (pAd->CommonCfg.PhyMode == PHY_11ABG_MIXED
			|| pAd->CommonCfg.PhyMode == PHY_11ABGN_MIXED
		)
		{
			if (pAd->LatchRfRegs.Channel > 14)
				pAd->CommonCfg.MlmeTransmit.field.MODE = 1;
			else
				pAd->CommonCfg.MlmeTransmit.field.MODE = 0;
		}
	}

	
	
	
	
	
	
	
	

    
	if ((pHeader_802_11->FC.Type != BTYPE_DATA) && (pHeader_802_11->FC.Type != BTYPE_CNTL))
	{
		if ((pAd->StaCfg.Psm == PWR_SAVE) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_ACTION))
			pHeader_802_11->FC.PwrMgmt = PWR_SAVE;
		else
			pHeader_802_11->FC.PwrMgmt = PWR_ACTIVE;
	}

	bInsertTimestamp = FALSE;
	if (pHeader_802_11->FC.Type == BTYPE_CNTL) 
	{
		
		if ((pAd->OpMode == OPMODE_STA) && (pHeader_802_11->FC.SubType == SUBTYPE_PS_POLL))
		{
			pHeader_802_11->FC.PwrMgmt = PWR_SAVE;
		}
		bAckRequired = FALSE;
	}
	else 
	{
		if (pHeader_802_11->Addr1[0] & 0x01) 
		{
			bAckRequired = FALSE;
			pHeader_802_11->Duration = 0;
		}
		else
		{
			bAckRequired = TRUE;
			pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, 14);
			if (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP)
			{
				bInsertTimestamp = TRUE;
			}
		}
	}

	pHeader_802_11->Sequence = pAd->Sequence++;
	if (pAd->Sequence >0xfff)
		pAd->Sequence = 0;

	
	
	if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pAd->CommonCfg.RadarDetect.RDMode != RD_NORMAL_MODE))
	{
		DBGPRINT(RT_DEBUG_ERROR,("MlmeHardTransmit --> radar detect not in normal mode !!!\n"));
		RTMP_SEM_UNLOCK(&pAd->MgmtRingLock);
		return (NDIS_STATUS_FAILURE);
	}

	
	
	
	
	

	
	
	


	if (pMacEntry == NULL)
	{
		RTMPWriteTxWI(pAd, pFirstTxWI, FALSE, FALSE, bInsertTimestamp, FALSE, bAckRequired, FALSE,
		0, RESERVED_WCID, (SrcBufLen - TXINFO_SIZE - TXWI_SIZE), PID_MGMT, 0,  (UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS, IFS_BACKOFF, FALSE, &pAd->CommonCfg.MlmeTransmit);
	}
	else
	{
		RTMPWriteTxWI(pAd, pFirstTxWI, FALSE, FALSE,
					bInsertTimestamp, FALSE, bAckRequired, FALSE,
					0, pMacEntry->Aid, (SrcBufLen - TXINFO_SIZE - TXWI_SIZE),
					pMacEntry->MaxHTPhyMode.field.MCS, 0,
					(UCHAR)pMacEntry->MaxHTPhyMode.field.MCS,
					IFS_BACKOFF, FALSE, &pMacEntry->MaxHTPhyMode);
	}

	
	HAL_KickOutMgmtTx(pAd, QueIdx, pPacket, pSrcBufVA, SrcBufLen);

	
	RTMP_SEM_UNLOCK(&pAd->MgmtRingLock);
	return NDIS_STATUS_SUCCESS;
}




#define DEQUEUE_LOCK(lock, bIntContext, IrqFlags) 				\
			do{													\
				if (bIntContext == FALSE)						\
				RTMP_IRQ_LOCK((lock), IrqFlags);		\
			}while(0)

#define DEQUEUE_UNLOCK(lock, bIntContext, IrqFlags)				\
			do{													\
				if (bIntContext == FALSE)						\
					RTMP_IRQ_UNLOCK((lock), IrqFlags);	\
			}while(0)


static UCHAR TxPktClassification(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET  pPacket)
{
	UCHAR			TxFrameType = TX_UNKOWN_FRAME;
	UCHAR			Wcid;
	MAC_TABLE_ENTRY	*pMacEntry = NULL;
	BOOLEAN			bHTRate = FALSE;

	Wcid = RTMP_GET_PACKET_WCID(pPacket);
	if (Wcid == MCAST_WCID)
	{	
		return TX_MCAST_FRAME;
	}

	
	pMacEntry = &pAd->MacTab.Content[Wcid];
	if (RTMP_GET_PACKET_LOWRATE(pPacket))
	{	
		TxFrameType = TX_LEGACY_FRAME;
	}
	else if (IS_HT_RATE(pMacEntry))
	{	

		
		
		bHTRate = TRUE;
		if (RTMP_GET_PACKET_MOREDATA(pPacket) || (pMacEntry->PsMode == PWR_SAVE))
			TxFrameType = TX_LEGACY_FRAME;
#ifdef UAPSD_AP_SUPPORT
		else if (RTMP_GET_PACKET_EOSP(pPacket))
			TxFrameType = TX_LEGACY_FRAME;
#endif 
		else if((pMacEntry->TXBAbitmap & (1<<(RTMP_GET_PACKET_UP(pPacket)))) != 0)
			return TX_AMPDU_FRAME;
		else if(CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_AMSDU_INUSED))
			return TX_AMSDU_FRAME;
		else
			TxFrameType = TX_LEGACY_FRAME;
	}
	else
	{	
		if ((CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE) && pAd->CommonCfg.bAggregationCapable) &&
			(RTMP_GET_PACKET_TXRATE(pPacket) >= RATE_6) &&
			(!(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))))
		{	
			TxFrameType = TX_RALINK_FRAME;
		}
		else
		{
			TxFrameType = TX_LEGACY_FRAME;
		}
	}

	
	if ((RTMP_GET_PACKET_FRAGMENTS(pPacket) > 1) && (TxFrameType == TX_LEGACY_FRAME))
		TxFrameType = TX_FRAG_FRAME;

	return TxFrameType;
}


BOOLEAN RTMP_FillTxBlkInfo(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk)
{
	PACKET_INFO			PacketInfo;
	PNDIS_PACKET		pPacket;
	PMAC_TABLE_ENTRY	pMacEntry = NULL;

	pPacket = pTxBlk->pPacket;
	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);

	pTxBlk->Wcid	 	 		= RTMP_GET_PACKET_WCID(pPacket);
	pTxBlk->apidx		 		= RTMP_GET_PACKET_IF(pPacket);
	pTxBlk->UserPriority 		= RTMP_GET_PACKET_UP(pPacket);
	pTxBlk->FrameGap = IFS_HTTXOP;		

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pTxBlk->pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);
	else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bClearEAPFrame);

	
	TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bForceNonQoS);


	if (pTxBlk->Wcid == MCAST_WCID)
	{
		pTxBlk->pMacEntry = NULL;
		{
#ifdef MCAST_RATE_SPECIFIC
			PUCHAR pDA = GET_OS_PKT_DATAPTR(pPacket);
			if (((*pDA & 0x01) == 0x01) && (*pDA != 0xff))
				pTxBlk->pTransmit = &pAd->CommonCfg.MCastPhyMode;
			else
#endif 
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID].HTPhyMode;
		}

		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);	
		
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAllowFrag);
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
		if (RTMP_GET_PACKET_MOREDATA(pPacket))
		{
			TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);
		}

	}
	else
	{
		pTxBlk->pMacEntry = &pAd->MacTab.Content[pTxBlk->Wcid];
		pTxBlk->pTransmit = &pTxBlk->pMacEntry->HTPhyMode;

		pMacEntry = pTxBlk->pMacEntry;


		
		if (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx] != NORMAL_ACK)
			TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
		else
			TX_BLK_SET_FLAG(pTxBlk, fTX_bAckRequired);

		{
			
#ifdef RT2860
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED))
#endif
#ifdef RT2870
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) &&
				CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
#endif
				TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM);
		}

		if (pTxBlk->TxFrameType == TX_LEGACY_FRAME)
		{
			if ( (RTMP_GET_PACKET_LOWRATE(pPacket)) ||
                ((pAd->OpMode == OPMODE_AP) && (pMacEntry->MaxHTPhyMode.field.MODE == MODE_CCK) && (pMacEntry->MaxHTPhyMode.field.MCS == RATE_1)))
			{	
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID].HTPhyMode;

				
				if (IS_HT_STA(pTxBlk->pMacEntry) &&
					(CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET)) &&
					((pAd->CommonCfg.bRdg == TRUE) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RDG_CAPABLE)))
				{
					TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
					TX_BLK_SET_FLAG(pTxBlk, fTX_bForceNonQoS);
				}
			}

			if ( (IS_HT_RATE(pMacEntry) == FALSE) &&
				(CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE)))
			{	
				TX_BLK_SET_FLAG(pTxBlk, fTX_bPiggyBack);
			}

			if (RTMP_GET_PACKET_MOREDATA(pPacket))
			{
				TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);
			}
#ifdef UAPSD_AP_SUPPORT
			if (RTMP_GET_PACKET_EOSP(pPacket))
			{
				TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP);
			}
#endif 
		}
		else if (pTxBlk->TxFrameType == TX_FRAG_FRAME)
		{
			TX_BLK_SET_FLAG(pTxBlk, fTX_bAllowFrag);
		}

		pMacEntry->DebugTxCount++;
	}

	return TRUE;
}


BOOLEAN CanDoAggregateTransmit(
	IN RTMP_ADAPTER *pAd,
	IN NDIS_PACKET *pPacket,
	IN TX_BLK		*pTxBlk)
{

	

	if (RTMP_GET_PACKET_WCID(pPacket) == MCAST_WCID)
		return FALSE;

	if (RTMP_GET_PACKET_DHCP(pPacket) ||
		RTMP_GET_PACKET_EAPOL(pPacket) ||
		RTMP_GET_PACKET_WAI(pPacket))
		return FALSE;

	if ((pTxBlk->TxFrameType == TX_AMSDU_FRAME) &&
		((pTxBlk->TotalFrameLen + GET_OS_PKT_LEN(pPacket))> (RX_BUFFER_AGGRESIZE - 100)))
	{	
		return FALSE;
	}

	if ((pTxBlk->TxFrameType == TX_RALINK_FRAME) &&
		(pTxBlk->TxPacketList.Number == 2))
	{	
		return FALSE;
	}

	if ((INFRA_ON(pAd)) && (pAd->OpMode == OPMODE_STA)) 
		return TRUE;
	else
		return FALSE;
}



VOID RTMPDeQueuePacket(
	IN  PRTMP_ADAPTER   pAd,
	IN  BOOLEAN         bIntContext,
	IN  UCHAR			QIdx, 
	IN  UCHAR           Max_Tx_Packets)
{
	PQUEUE_ENTRY    pEntry = NULL;
	PNDIS_PACKET 	pPacket;
	NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
	UCHAR           Count=0;
	PQUEUE_HEADER   pQueue;
	ULONG           FreeNumber[NUM_OF_TX_RING];
	UCHAR			QueIdx, sQIdx, eQIdx;
	unsigned long	IrqFlags = 0;
	BOOLEAN			hasTxDesc = FALSE;
	TX_BLK			TxBlk;
	TX_BLK			*pTxBlk;



	if (QIdx == NUM_OF_TX_RING)
	{
		sQIdx = 0;

		eQIdx = 3;	
	}
	else
	{
		sQIdx = eQIdx = QIdx;
	}

	for (QueIdx=sQIdx; QueIdx <= eQIdx; QueIdx++)
	{
		Count=0;

		RT28XX_START_DEQUEUE(pAd, QueIdx, IrqFlags);


		while (1)
		{
			if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS |
										fRTMP_ADAPTER_RADIO_OFF |
										fRTMP_ADAPTER_RESET_IN_PROGRESS |
										fRTMP_ADAPTER_HALT_IN_PROGRESS |
										fRTMP_ADAPTER_NIC_NOT_EXIST))))
			{
				RT28XX_STOP_DEQUEUE(pAd, QueIdx, IrqFlags);
				return;
			}

			if (Count >= Max_Tx_Packets)
				break;

			DEQUEUE_LOCK(&pAd->irq_lock, bIntContext, IrqFlags);
			if (&pAd->TxSwQueue[QueIdx] == NULL)
			{
				DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
				break;
			}
#ifdef RT2860
			FreeNumber[QueIdx] = GET_TXRING_FREENO(pAd, QueIdx);


			if (FreeNumber[QueIdx] <= 5)
			{
				
				RTMPFreeTXDUponTxDmaDone(pAd, QueIdx);
				FreeNumber[QueIdx] = GET_TXRING_FREENO(pAd, QueIdx);
			}
#endif 
			
			pQueue = &pAd->TxSwQueue[QueIdx];
			if ((pEntry = pQueue->Head) == NULL)
			{
				DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
				break;
			}

			pTxBlk = &TxBlk;
			NdisZeroMemory((PUCHAR)pTxBlk, sizeof(TX_BLK));
			pTxBlk->QueIdx = QueIdx;

			pPacket = QUEUE_ENTRY_TO_PKT(pEntry);

			
			hasTxDesc = RT28XX_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, FreeNumber[QueIdx], pPacket);
			if (!hasTxDesc)
			{
				pAd->PrivateInfo.TxRingFullCnt++;

				DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);

				break;
			}

			pTxBlk->TxFrameType = TxPktClassification(pAd, pPacket);
			pEntry = RemoveHeadQueue(pQueue);
			pTxBlk->TotalFrameNum++;
			pTxBlk->TotalFragNum += RTMP_GET_PACKET_FRAGMENTS(pPacket);	
			pTxBlk->TotalFrameLen += GET_OS_PKT_LEN(pPacket);
			pTxBlk->pPacket = pPacket;
			InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pPacket));

			if (pTxBlk->TxFrameType == TX_RALINK_FRAME || pTxBlk->TxFrameType == TX_AMSDU_FRAME)
			{
				
				if (NEED_QUEUE_BACK_FOR_AGG(pAd, QueIdx, FreeNumber[QueIdx], pTxBlk->TxFrameType))
				{
					InsertHeadQueue(pQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
					DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
					break;
				}

				do{
					if((pEntry = pQueue->Head) == NULL)
						break;

					
					pPacket = QUEUE_ENTRY_TO_PKT(pEntry);
					FreeNumber[QueIdx] = GET_TXRING_FREENO(pAd, QueIdx);
					hasTxDesc = RT28XX_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, FreeNumber[QueIdx], pPacket);
					if ((hasTxDesc == FALSE) || (CanDoAggregateTransmit(pAd, pPacket, pTxBlk) == FALSE))
						break;

					
					pEntry = RemoveHeadQueue(pQueue);
					ASSERT(pEntry);
					pPacket = QUEUE_ENTRY_TO_PKT(pEntry);
					pTxBlk->TotalFrameNum++;
					pTxBlk->TotalFragNum += RTMP_GET_PACKET_FRAGMENTS(pPacket);	
					pTxBlk->TotalFrameLen += GET_OS_PKT_LEN(pPacket);
					InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pPacket));
				}while(1);

				if (pTxBlk->TxPacketList.Number == 1)
					pTxBlk->TxFrameType = TX_LEGACY_FRAME;
			}

#ifdef RT2870
			DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
#endif 

			Count += pTxBlk->TxPacketList.Number;

			
			Status = STAHardTransmit(pAd, pTxBlk, QueIdx);

#ifdef RT2860
			DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
			
			
				NICUpdateFifoStaCounters(pAd);
#endif
		}

		RT28XX_STOP_DEQUEUE(pAd, QueIdx, IrqFlags);

#ifdef RT2870
		if (!hasTxDesc)
			RTUSBKickBulkOut(pAd);
#endif 
	}

}



USHORT	RTMPCalcDuration(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Rate,
	IN	ULONG			Size)
{
	ULONG	Duration = 0;

	if (Rate < RATE_FIRST_OFDM_RATE) 
	{
		if ((Rate > RATE_1) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
			Duration = 96;	
		else
			Duration = 192; 

		Duration += (USHORT)((Size << 4) / RateIdTo500Kbps[Rate]);
		if ((Size << 4) % RateIdTo500Kbps[Rate])
			Duration ++;
	}
	else if (Rate <= RATE_LAST_OFDM_RATE)
	{
		Duration = 20 + 6;		
		Duration += 4 * (USHORT)((11 + Size * 4) / RateIdTo500Kbps[Rate]);
		if ((11 + Size * 4) % RateIdTo500Kbps[Rate])
			Duration += 4;
	}
	else	
	{
		Duration = 20 + 6;		
	}

	return (USHORT)Duration;
}



VOID RTMPWriteTxWI(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXWI_STRUC 	pOutTxWI,
	IN	BOOLEAN			FRAG,
	IN	BOOLEAN			CFACK,
	IN	BOOLEAN			InsTimestamp,
	IN	BOOLEAN 		AMPDU,
	IN	BOOLEAN 		Ack,
	IN	BOOLEAN 		NSeq,		
	IN	UCHAR			BASize,
	IN	UCHAR			WCID,
	IN	ULONG			Length,
	IN	UCHAR 			PID,
	IN	UCHAR			TID,
	IN	UCHAR			TxRate,
	IN	UCHAR			Txopmode,
	IN	BOOLEAN			CfAck,
	IN	HTTRANSMIT_SETTING	*pTransmit)
{
	PMAC_TABLE_ENTRY	pMac = NULL;
	TXWI_STRUC 		TxWI;
	PTXWI_STRUC 	pTxWI;

	if (WCID < MAX_LEN_OF_MAC_TABLE)
		pMac = &pAd->MacTab.Content[WCID];

	
	
	
	
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	NdisZeroMemory(&TxWI, TXWI_SIZE);
	pTxWI = &TxWI;

	pTxWI->FRAG= FRAG;

	pTxWI->CFACK = CFACK;
	pTxWI->TS= InsTimestamp;
	pTxWI->AMPDU = AMPDU;
	pTxWI->ACK = Ack;
	pTxWI->txop= Txopmode;

	pTxWI->NSEQ = NSeq;
	
	BASize = pAd->CommonCfg.TxBASize;

	if( BASize >7 )
		BASize =7;
	pTxWI->BAWinSize = BASize;
	pTxWI->ShortGI = pTransmit->field.ShortGI;
	pTxWI->STBC = pTransmit->field.STBC;

	pTxWI->WirelessCliID = WCID;
	pTxWI->MPDUtotalByteCount = Length;
	pTxWI->PacketId = PID;

	
	pTxWI->BW = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);

	pTxWI->MCS = pTransmit->field.MCS;
	pTxWI->PHYMODE = pTransmit->field.MODE;
	pTxWI->CFACK = CfAck;

	if (pMac)
	{
		if (pAd->CommonCfg.bMIMOPSEnable)
		{
			if ((pMac->MmpsMode == MMPS_DYNAMIC) && (pTransmit->field.MCS > 7))
			{
				
				pTxWI->MIMOps = 1;
			}
			else if (pMac->MmpsMode == MMPS_STATIC)
			{
				
				if (pTransmit->field.MODE >= MODE_HTMIX && pTransmit->field.MCS > 7)
				{
					pTxWI->MCS = 7;
					pTxWI->MIMOps = 0;
				}
			}
		}
		
		if (pMac->bIAmBadAtheros && (pMac->WepStatus != Ndis802_11WEPDisabled))
		{
			pTxWI->MpduDensity = 7;
		}
		else
		{
			pTxWI->MpduDensity = pMac->MpduDensity;
		}
	}

	pTxWI->PacketId = pTxWI->MCS;
	NdisMoveMemory(pOutTxWI, &TxWI, sizeof(TXWI_STRUC));
}


VOID RTMPWriteTxWI_Data(
	IN	PRTMP_ADAPTER		pAd,
	IN	OUT PTXWI_STRUC		pTxWI,
	IN	TX_BLK				*pTxBlk)
{
	HTTRANSMIT_SETTING	*pTransmit;
	PMAC_TABLE_ENTRY	pMacEntry;
	UCHAR				BASize;

	ASSERT(pTxWI);

	pTransmit = pTxBlk->pTransmit;
	pMacEntry = pTxBlk->pMacEntry;


	
	
	
	
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	NdisZeroMemory(pTxWI, TXWI_SIZE);

	pTxWI->FRAG		= TX_BLK_TEST_FLAG(pTxBlk, fTX_bAllowFrag);
	pTxWI->ACK		= TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired);
	pTxWI->txop		= pTxBlk->FrameGap;

		pTxWI->WirelessCliID		= pTxBlk->Wcid;

	pTxWI->MPDUtotalByteCount	= pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
	pTxWI->CFACK				= TX_BLK_TEST_FLAG(pTxBlk, fTX_bPiggyBack);

	
	pTxWI->BW = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
	pTxWI->AMPDU	= ((pTxBlk->TxFrameType == TX_AMPDU_FRAME) ? TRUE : FALSE);

	
	BASize = pAd->CommonCfg.TxBASize;
	if((pTxBlk->TxFrameType == TX_AMPDU_FRAME) && (pMacEntry))
	{
		UCHAR		RABAOriIdx = 0;	

		RABAOriIdx = pTxBlk->pMacEntry->BAOriWcidArray[pTxBlk->UserPriority];
		BASize = pAd->BATable.BAOriEntry[RABAOriIdx].BAWinSize;
	}

	pTxWI->TxBF = pTransmit->field.TxBF;
	pTxWI->BAWinSize = BASize;
	pTxWI->ShortGI = pTransmit->field.ShortGI;
	pTxWI->STBC = pTransmit->field.STBC;

	pTxWI->MCS = pTransmit->field.MCS;
	pTxWI->PHYMODE = pTransmit->field.MODE;

	if (pMacEntry)
	{
		if ((pMacEntry->MmpsMode == MMPS_DYNAMIC) && (pTransmit->field.MCS > 7))
		{
			
			pTxWI->MIMOps = 1;
		}
		else if (pMacEntry->MmpsMode == MMPS_STATIC)
		{
			
			if (pTransmit->field.MODE >= MODE_HTMIX && pTransmit->field.MCS > 7)
			{
				pTxWI->MCS = 7;
				pTxWI->MIMOps = 0;
			}
		}

		if (pMacEntry->bIAmBadAtheros && (pMacEntry->WepStatus != Ndis802_11WEPDisabled))
		{
			pTxWI->MpduDensity = 7;
		}
		else
		{
			pTxWI->MpduDensity = pMacEntry->MpduDensity;
		}
	}


	
	pTxWI->PacketId = pTxWI->MCS;
}


VOID RTMPWriteTxWI_Cache(
	IN	PRTMP_ADAPTER		pAd,
	IN	OUT PTXWI_STRUC		pTxWI,
	IN	TX_BLK				*pTxBlk)
{
	PHTTRANSMIT_SETTING	pTransmit;
	PMAC_TABLE_ENTRY	pMacEntry;

	
	
	
	pMacEntry = pTxBlk->pMacEntry;
	pTransmit = pTxBlk->pTransmit;

	if (pMacEntry->bAutoTxRateSwitch)
	{
		pTxWI->txop = IFS_HTTXOP;

		
		pTxWI->BW = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
		pTxWI->ShortGI = pTransmit->field.ShortGI;
		pTxWI->STBC = pTransmit->field.STBC;

		pTxWI->MCS = pTransmit->field.MCS;
		pTxWI->PHYMODE = pTransmit->field.MODE;

		
		pTxWI->PacketId = pTransmit->field.MCS;
	}

	pTxWI->AMPDU = ((pMacEntry->NoBADataCountDown == 0) ? TRUE: FALSE);
	pTxWI->MIMOps = 0;

	if (pAd->CommonCfg.bMIMOPSEnable)
	{
		
		if ((pMacEntry->MmpsMode == MMPS_DYNAMIC) && (pTransmit->field.MCS > 7))
		{
			
			pTxWI->MIMOps = 1;
		}
		else if (pMacEntry->MmpsMode == MMPS_STATIC)
		{
			
			if ((pTransmit->field.MODE >= MODE_HTMIX) && (pTransmit->field.MCS > 7))
			{
				pTxWI->MCS = 7;
				pTxWI->MIMOps = 0;
			}
		}
	}


	pTxWI->MPDUtotalByteCount = pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;

}



VOID RTMPWriteTxDescriptor(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXD_STRUC		pTxD,
	IN	BOOLEAN 		bWIV,
	IN	UCHAR			QueueSEL)
{
	
	
	
	
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);

	pTxD->WIV	= (bWIV) ? 1: 0;
	pTxD->QSEL= (QueueSEL);
	if (pAd->bGenOneHCCA == TRUE)
		pTxD->QSEL= FIFO_HCCA;
	pTxD->DMADONE = 0;
}







BOOLEAN TxFrameIsAggregatible(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pPrevAddr1,
	IN	PUCHAR			p8023hdr)
{

	
	if ((p8023hdr[12] == 0x88) && (p8023hdr[13] == 0x8e))
		return FALSE;

	
	if (p8023hdr[0] & 0x01)
		return FALSE;

	if (INFRA_ON(pAd)) 
		return TRUE;
	else if ((pPrevAddr1 == NULL) || MAC_ADDR_EQUAL(pPrevAddr1, p8023hdr)) 
		return TRUE;
	else
		return FALSE;
}



BOOLEAN PeerIsAggreOn(
	IN	PRTMP_ADAPTER	pAd,
	IN	ULONG		   TxRate,
	IN	PMAC_TABLE_ENTRY pMacEntry)
{
	ULONG	AFlags = (fCLIENT_STATUS_AMSDU_INUSED | fCLIENT_STATUS_AGGREGATION_CAPABLE);

	if (pMacEntry != NULL && CLIENT_STATUS_TEST_FLAG(pMacEntry, AFlags))
	{
		if (pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)
		{
			return TRUE;
		}

#ifdef AGGREGATION_SUPPORT
		if (TxRate >= RATE_6 && pAd->CommonCfg.bAggregationCapable && (!(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))))
		{	
			return TRUE;
		}
#endif 
	}

	return FALSE;

}



PQUEUE_HEADER	RTMPCheckTxSwQueue(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR			pQueIdx)
{

	ULONG	Number;

	Number = pAd->TxSwQueue[QID_AC_BK].Number
			 + pAd->TxSwQueue[QID_AC_BE].Number
			 + pAd->TxSwQueue[QID_AC_VI].Number
			 + pAd->TxSwQueue[QID_AC_VO].Number
			 + pAd->TxSwQueue[QID_HCCA].Number;

	if (pAd->TxSwQueue[QID_AC_VO].Head != NULL)
	{
		*pQueIdx = QID_AC_VO;
		return (&pAd->TxSwQueue[QID_AC_VO]);
	}
	else if (pAd->TxSwQueue[QID_AC_VI].Head != NULL)
	{
		*pQueIdx = QID_AC_VI;
		return (&pAd->TxSwQueue[QID_AC_VI]);
	}
	else if (pAd->TxSwQueue[QID_AC_BE].Head != NULL)
	{
		*pQueIdx = QID_AC_BE;
		return (&pAd->TxSwQueue[QID_AC_BE]);
	}
	else if (pAd->TxSwQueue[QID_AC_BK].Head != NULL)
	{
		*pQueIdx = QID_AC_BK;
		return (&pAd->TxSwQueue[QID_AC_BK]);
	}
	else if (pAd->TxSwQueue[QID_HCCA].Head != NULL)
	{
		*pQueIdx = QID_HCCA;
		return (&pAd->TxSwQueue[QID_HCCA]);
	}

	
	*pQueIdx = QID_AC_BK;

	return (NULL);
}

#ifdef RT2860
BOOLEAN  RTMPFreeTXDUponTxDmaDone(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			QueIdx)
{
	PRTMP_TX_RING pTxRing;
	PTXD_STRUC	  pTxD;
	PNDIS_PACKET  pPacket;
	UCHAR	FREE = 0;
	TXD_STRUC	TxD, *pOriTxD;
	
	BOOLEAN			bReschedule = FALSE;


	ASSERT(QueIdx < NUM_OF_TX_RING);
	pTxRing = &pAd->TxRing[QueIdx];

	RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QueIdx * RINGREG_DIFF, &pTxRing->TxDmaIdx);
	while (pTxRing->TxSwFreeIdx != pTxRing->TxDmaIdx)
	{
		
		
			NICUpdateFifoStaCounters(pAd);

		
		FREE++;
                pTxD = (PTXD_STRUC) (pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);
		pOriTxD = pTxD;
                NdisMoveMemory(&TxD, pTxD, sizeof(TXD_STRUC));
		pTxD = &TxD;

		pTxD->DMADONE = 0;


		{
			pPacket = pTxRing->Cell[pTxRing->TxSwFreeIdx].pNdisPacket;
			if (pPacket)
			{
#ifdef CONFIG_5VT_ENHANCE
				if (RTMP_GET_PACKET_5VT(pPacket))
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, 16, PCI_DMA_TODEVICE);
				else
#endif 
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			
			pTxRing->Cell[pTxRing->TxSwFreeIdx].pNdisPacket = NULL;

			pPacket = pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket;

			ASSERT(pPacket == NULL);
			if (pPacket)
			{
#ifdef CONFIG_5VT_ENHANCE
				if (RTMP_GET_PACKET_5VT(pPacket))
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, 16, PCI_DMA_TODEVICE);
				else
#endif 
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			
			pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket = NULL;
		}


		pAd->RalinkCounters.TransmittedByteCount +=  (pTxD->SDLen1 + pTxD->SDLen0);
		pAd->RalinkCounters.OneSecDmaDoneCount[QueIdx] ++;
		INC_RING_INDEX(pTxRing->TxSwFreeIdx, TX_RING_SIZE);
		
		RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QueIdx * RINGREG_DIFF ,  &pTxRing->TxDmaIdx);

        NdisMoveMemory(pOriTxD, pTxD, sizeof(TXD_STRUC));
	}


	return  bReschedule;

}



BOOLEAN	RTMPHandleTxRingDmaDoneInterrupt(
	IN	PRTMP_ADAPTER	pAd,
	IN	INT_SOURCE_CSR_STRUC TxRingBitmap)
{
    unsigned long	IrqFlags;
	BOOLEAN			bReschedule = FALSE;

	

	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);

	if (TxRingBitmap.field.Ac0DmaDone)
		bReschedule = RTMPFreeTXDUponTxDmaDone(pAd, QID_AC_BE);

	if (TxRingBitmap.field.HccaDmaDone)
		bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd, QID_HCCA);

	if (TxRingBitmap.field.Ac3DmaDone)
		bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd, QID_AC_VO);

	if (TxRingBitmap.field.Ac2DmaDone)
		bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd, QID_AC_VI);

	if (TxRingBitmap.field.Ac1DmaDone)
		bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd, QID_AC_BK);

	
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

	
	RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);

	return  bReschedule;
}



VOID	RTMPHandleMgmtRingDmaDoneInterrupt(
	IN	PRTMP_ADAPTER	pAd)
{
	PTXD_STRUC	 pTxD;
	PNDIS_PACKET pPacket;
	UCHAR	FREE = 0;
	PRTMP_MGMT_RING pMgmtRing = &pAd->MgmtRing;

	NdisAcquireSpinLock(&pAd->MgmtRingLock);

	RTMP_IO_READ32(pAd, TX_MGMTDTX_IDX, &pMgmtRing->TxDmaIdx);
	while (pMgmtRing->TxSwFreeIdx!= pMgmtRing->TxDmaIdx)
	{
		FREE++;
		pTxD = (PTXD_STRUC) (pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocVa);
		pTxD->DMADONE = 0;
		pPacket = pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNdisPacket;


		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
		pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNdisPacket = NULL;

		pPacket = pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNextNdisPacket;
		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
		pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNextNdisPacket = NULL;
		INC_RING_INDEX(pMgmtRing->TxSwFreeIdx, MGMT_RING_SIZE);
	}
	NdisReleaseSpinLock(&pAd->MgmtRingLock);

}



VOID	RTMPHandleTBTTInterrupt(
	IN PRTMP_ADAPTER pAd)
{
	{
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
		{
		}
	}
}



VOID	RTMPHandlePreTBTTInterrupt(
	IN PRTMP_ADAPTER pAd)
{
	{
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("RTMPHandlePreTBTTInterrupt...\n"));
		}
	}


}

VOID	RTMPHandleRxCoherentInterrupt(
	IN	PRTMP_ADAPTER	pAd)
{
	WPDMA_GLO_CFG_STRUC	GloCfg;

	if (pAd == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("====> pAd is NULL, return.\n"));
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPHandleRxCoherentInterrupt \n"));

	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG , &GloCfg.word);

	GloCfg.field.EnTXWriteBackDDONE = 0;
	GloCfg.field.EnableRxDMA = 0;
	GloCfg.field.EnableTxDMA = 0;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);

	RTMPRingCleanUp(pAd, QID_AC_BE);
	RTMPRingCleanUp(pAd, QID_AC_BK);
	RTMPRingCleanUp(pAd, QID_AC_VI);
	RTMPRingCleanUp(pAd, QID_AC_VO);
	RTMPRingCleanUp(pAd, QID_HCCA);
	RTMPRingCleanUp(pAd, QID_MGMT);
	RTMPRingCleanUp(pAd, QID_RX);

	RTMPEnableRxTx(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPHandleRxCoherentInterrupt \n"));
}
#endif 


VOID	RTMPSuspendMsduTransmission(
	IN	PRTMP_ADAPTER	pAd)
{
	DBGPRINT(RT_DEBUG_TRACE,("SCANNING, suspend MSDU transmission ...\n"));


	
	
	
	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R66, &pAd->BbpTuning.R66CurrentValue);

	
	RTMPSetAGCInitValue(pAd, BW_20);

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
}



VOID RTMPResumeMsduTransmission(
	IN	PRTMP_ADAPTER	pAd)
{
	DBGPRINT(RT_DEBUG_TRACE,("SCAN done, resume MSDU transmission ...\n"));

	
	
	if (pAd->BbpTuning.R66CurrentValue == 0)
	{
		pAd->BbpTuning.R66CurrentValue = 0x38;
		DBGPRINT_ERR(("RTMPResumeMsduTransmission, R66CurrentValue=0...\n"));
	}

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, pAd->BbpTuning.R66CurrentValue);

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
	RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
}


UINT deaggregate_AMSDU_announce(
	IN	PRTMP_ADAPTER	pAd,
	PNDIS_PACKET		pPacket,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize)
{
	USHORT 			PayloadSize;
	USHORT 			SubFrameSize;
	PHEADER_802_3 	pAMSDUsubheader;
	UINT			nMSDU;
    UCHAR			Header802_3[14];

	PUCHAR			pPayload, pDA, pSA, pRemovedLLCSNAP;
	PNDIS_PACKET	pClonePacket;



	nMSDU = 0;

	while (DataSize > LENGTH_802_3)
	{

		nMSDU++;

		pAMSDUsubheader = (PHEADER_802_3)pData;
		PayloadSize = pAMSDUsubheader->Octet[1] + (pAMSDUsubheader->Octet[0]<<8);
		SubFrameSize = PayloadSize + LENGTH_802_3;


		if ((DataSize < SubFrameSize) || (PayloadSize > 1518 ))
		{
			break;
		}

		pPayload = pData + LENGTH_802_3;
		pDA = pData;
		pSA = pData + MAC_ADDR_LEN;

		
        CONVERT_TO_802_3(Header802_3, pDA, pSA, pPayload, PayloadSize, pRemovedLLCSNAP);

		if ((Header802_3[12] == 0x88) && (Header802_3[13] == 0x8E) )
		{
		    
		   MLME_QUEUE_ELEM *Elem = (MLME_QUEUE_ELEM *) kmalloc(sizeof(MLME_QUEUE_ELEM), MEM_ALLOC_FLAG);
		   if (Elem == NULL)
			return;
		   memmove(Elem->Msg+(LENGTH_802_11 + LENGTH_802_1_H), pPayload, PayloadSize);
		   Elem->MsgLen = LENGTH_802_11 + LENGTH_802_1_H + PayloadSize;
		   WpaEAPOLKeyAction(pAd, Elem);
		   kfree(Elem);
		}

		{
	        	if (pRemovedLLCSNAP)
	        	{
	    			pPayload -= LENGTH_802_3;
	    			PayloadSize += LENGTH_802_3;
	    			NdisMoveMemory(pPayload, &Header802_3[0], LENGTH_802_3);
	        	}
		}

		pClonePacket = ClonePacket(pAd, pPacket, pPayload, PayloadSize);
		if (pClonePacket)
		{
			ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pClonePacket, RTMP_GET_PACKET_IF(pPacket));
		}


		
		
		SubFrameSize = (SubFrameSize+3)&(~0x3);


		if (SubFrameSize > 1528 || SubFrameSize < 32)
		{
			break;
		}

		if (DataSize > SubFrameSize)
		{
			pData += SubFrameSize;
			DataSize -= SubFrameSize;
		}
		else
		{
			
			DataSize = 0;
		}
	}

	
	RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);

	return nMSDU;
}


UINT BA_Reorder_AMSDU_Annnounce(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket)
{
	PUCHAR			pData;
	USHORT			DataSize;
	UINT			nMSDU = 0;

	pData = (PUCHAR) GET_OS_PKT_DATAPTR(pPacket);
	DataSize = (USHORT) GET_OS_PKT_LEN(pPacket);

	nMSDU = deaggregate_AMSDU_announce(pAd, pPacket, pData, DataSize);

	return nMSDU;
}



MAC_TABLE_ENTRY *MacTableLookup(
	IN PRTMP_ADAPTER pAd,
	PUCHAR pAddr)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->MacTab.Hash[HashIdx];

	while (pEntry && (pEntry->ValidAsCLI || pEntry->ValidAsWDS || pEntry->ValidAsApCli || pEntry->ValidAsMesh))
	{
		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
		{
			break;
		}
		else
			pEntry = pEntry->pNext;
	}

	return pEntry;
}

MAC_TABLE_ENTRY *MacTableInsertEntry(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR			pAddr,
	IN	UCHAR			apidx,
	IN BOOLEAN	CleanAll)
{
	UCHAR HashIdx;
	int i, FirstWcid;
	MAC_TABLE_ENTRY *pEntry = NULL, *pCurrEntry;

	
	if (pAd->MacTab.Size >= MAX_LEN_OF_MAC_TABLE)
		return NULL;

	FirstWcid = 1;

	if (pAd->StaCfg.BssType == BSS_INFRA)
		FirstWcid = 2;

	
	NdisAcquireSpinLock(&pAd->MacTabLock);
	for (i = FirstWcid; i< MAX_LEN_OF_MAC_TABLE; i++)   
	{
		
		if ((pAd->MacTab.Content[i].ValidAsCLI == FALSE) &&
			(pAd->MacTab.Content[i].ValidAsWDS == FALSE) &&
			(pAd->MacTab.Content[i].ValidAsApCli== FALSE) &&
			(pAd->MacTab.Content[i].ValidAsMesh == FALSE)
			)
		{
			pEntry = &pAd->MacTab.Content[i];
			if (CleanAll == TRUE)
			{
				pEntry->MaxSupportedRate = RATE_11;
				pEntry->CurrTxRate = RATE_11;
				NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
				pEntry->PairwiseKey.KeyLen = 0;
				pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
			}
			{
				{
					pEntry->ValidAsCLI = TRUE;
					pEntry->ValidAsWDS = FALSE;
					pEntry->ValidAsApCli = FALSE;
					pEntry->ValidAsMesh = FALSE;
					pEntry->ValidAsDls = FALSE;
				}
			}

			pEntry->bIAmBadAtheros = FALSE;
			pEntry->pAd = pAd;
			pEntry->CMTimerRunning = FALSE;
			pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
			pEntry->RSNIE_Len = 0;
			NdisZeroMemory(pEntry->R_Counter, sizeof(pEntry->R_Counter));
			pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;

			if (pEntry->ValidAsMesh)
				pEntry->apidx = (apidx - MIN_NET_DEVICE_FOR_MESH);
			else if (pEntry->ValidAsApCli)
				pEntry->apidx = (apidx - MIN_NET_DEVICE_FOR_APCLI);
			else if (pEntry->ValidAsWDS)
				pEntry->apidx = (apidx - MIN_NET_DEVICE_FOR_WDS);
			else
				pEntry->apidx = apidx;

			{
				{
					pEntry->AuthMode = pAd->StaCfg.AuthMode;
					pEntry->WepStatus = pAd->StaCfg.WepStatus;
					pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
#ifdef RT2860
					AsicRemovePairwiseKeyEntry(pAd, pEntry->apidx, (UCHAR)i);
#endif
				}
			}

			pEntry->GTKState = REKEY_NEGOTIATING;
			pEntry->PairwiseKey.KeyLen = 0;
			pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;

#ifdef RT2860
			if ((pAd->OpMode == OPMODE_STA) &&
				(pAd->StaCfg.BssType == BSS_ADHOC))
				pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
			else
#endif
			pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

			pEntry->PMKID_CacheIdx = ENTRY_NOT_FOUND;
			COPY_MAC_ADDR(pEntry->Addr, pAddr);
			pEntry->Sst = SST_NOT_AUTH;
			pEntry->AuthState = AS_NOT_AUTH;
			pEntry->Aid = (USHORT)i;  
			pEntry->CapabilityInfo = 0;
			pEntry->PsMode = PWR_ACTIVE;
			pEntry->PsQIdleCount = 0;
			pEntry->NoDataIdleCount = 0;
			pEntry->ContinueTxFailCnt = 0;
			InitializeQueueHeader(&pEntry->PsQueue);


			pAd->MacTab.Size ++;
			
			RT28XX_STA_ENTRY_ADD(pAd, pEntry);



			DBGPRINT(RT_DEBUG_TRACE, ("MacTableInsertEntry - allocate entry #%d, Total= %d\n",i, pAd->MacTab.Size));
			break;
		}
	}

	
	if (pEntry)
	{
		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		if (pAd->MacTab.Hash[HashIdx] == NULL)
		{
			pAd->MacTab.Hash[HashIdx] = pEntry;
		}
		else
		{
			pCurrEntry = pAd->MacTab.Hash[HashIdx];
			while (pCurrEntry->pNext != NULL)
				pCurrEntry = pCurrEntry->pNext;
			pCurrEntry->pNext = pEntry;
		}
	}

	NdisReleaseSpinLock(&pAd->MacTabLock);
	return pEntry;
}


BOOLEAN MacTableDeleteEntry(
	IN PRTMP_ADAPTER pAd,
	IN USHORT wcid,
	IN PUCHAR pAddr)
{
	USHORT HashIdx;
	MAC_TABLE_ENTRY *pEntry, *pPrevEntry, *pProbeEntry;
	BOOLEAN Cancelled;

	if (wcid >= MAX_LEN_OF_MAC_TABLE)
		return FALSE;

	NdisAcquireSpinLock(&pAd->MacTabLock);

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = &pAd->MacTab.Content[wcid];

	if (pEntry && (pEntry->ValidAsCLI || pEntry->ValidAsApCli || pEntry->ValidAsWDS || pEntry->ValidAsMesh
		))
	{
		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
		{

			
			RT28XX_STA_ENTRY_MAC_RESET(pAd, wcid);

			
			BASessionTearDownALL(pAd, pEntry->Aid);

			pPrevEntry = NULL;
			pProbeEntry = pAd->MacTab.Hash[HashIdx];
			ASSERT(pProbeEntry);

			
			do
			{
				if (pProbeEntry == pEntry)
				{
					if (pPrevEntry == NULL)
					{
						pAd->MacTab.Hash[HashIdx] = pEntry->pNext;
					}
					else
					{
						pPrevEntry->pNext = pEntry->pNext;
					}
					break;
				}

				pPrevEntry = pProbeEntry;
				pProbeEntry = pProbeEntry->pNext;
			} while (pProbeEntry);

			
			ASSERT(pProbeEntry != NULL);

			RT28XX_STA_ENTRY_KEY_DEL(pAd, BSS0, wcid);


		if (pEntry->EnqueueEapolStartTimerRunning != EAPOL_START_DISABLE)
		{
			RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
			pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
		}


   			NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
			pAd->MacTab.Size --;
			DBGPRINT(RT_DEBUG_TRACE, ("MacTableDeleteEntry1 - Total= %d\n", pAd->MacTab.Size));
		}
		else
		{
			printk("\n%s: Impossible Wcid = %d !!!!!\n", __func__, wcid);
		}
	}

	NdisReleaseSpinLock(&pAd->MacTabLock);

	
	if (pAd->MacTab.Size == 0)
	{
		pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 0;
#ifdef RT2860
		AsicUpdateProtect(pAd, 0 , (ALLN_SETPROTECT), TRUE, 0 );
#else
		
		
		RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_UPDATE_PROTECT, NULL, 0);
#endif
	}

	return TRUE;
}



VOID MacTableReset(
	IN  PRTMP_ADAPTER  pAd)
{
	int         i;

	DBGPRINT(RT_DEBUG_TRACE, ("MacTableReset\n"));
	

	for (i=1; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
#ifdef RT2860
		RT28XX_STA_ENTRY_MAC_RESET(pAd, i);
#endif
		if (pAd->MacTab.Content[i].ValidAsCLI == TRUE)
	   {
			
			BASessionTearDownALL(pAd, i);

			pAd->MacTab.Content[i].ValidAsCLI = FALSE;



#ifdef RT2870
			NdisZeroMemory(pAd->MacTab.Content[i].Addr, 6);
			RT28XX_STA_ENTRY_MAC_RESET(pAd, i);
#endif 

			
		}
	}

	return;
}


VOID AssocParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_ASSOC_REQ_STRUCT *AssocReq,
	IN PUCHAR                     pAddr,
	IN USHORT                     CapabilityInfo,
	IN ULONG                      Timeout,
	IN USHORT                     ListenIntv)
{
	COPY_MAC_ADDR(AssocReq->Addr, pAddr);
	
	AssocReq->CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO; 
	AssocReq->Timeout = Timeout;
	AssocReq->ListenIntv = ListenIntv;
}



VOID DisassocParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_DISASSOC_REQ_STRUCT *DisassocReq,
	IN PUCHAR pAddr,
	IN USHORT Reason)
{
	COPY_MAC_ADDR(DisassocReq->Addr, pAddr);
	DisassocReq->Reason = Reason;
}




BOOLEAN RTMPCheckDHCPFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket)
{
	PACKET_INFO 	PacketInfo;
	ULONG			NumberOfBytesRead = 0;
	ULONG			CurrentOffset = 0;
	PVOID			pVirtualAddress = NULL;
	UINT			NdisBufferLength;
	PUCHAR			pSrc;
	USHORT			Protocol;
	UCHAR			ByteOffset36 = 0;
	UCHAR			ByteOffset38 = 0;
	BOOLEAN 		ReadFirstParm = TRUE;

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, (PUCHAR *)&pVirtualAddress, &NdisBufferLength);

	NumberOfBytesRead += NdisBufferLength;
	pSrc = (PUCHAR) pVirtualAddress;
	Protocol = *(pSrc + 12) * 256 + *(pSrc + 13);

	
	
	
	while (NumberOfBytesRead <= PacketInfo.TotalPacketLength)
	{
		if ((NumberOfBytesRead >= 35) && (ReadFirstParm == TRUE))
		{
			CurrentOffset = 35 - (NumberOfBytesRead - NdisBufferLength);
			ByteOffset36 = *(pSrc + CurrentOffset);
			ReadFirstParm = FALSE;
		}

		if (NumberOfBytesRead >= 37)
		{
			CurrentOffset = 37 - (NumberOfBytesRead - NdisBufferLength);
			ByteOffset38 = *(pSrc + CurrentOffset);
			
			break;
		}
		return FALSE;
	}

	
	if ((ByteOffset36 != 0x44) || (ByteOffset38 != 0x43))
		{
		
		
		
		
		
		if (Protocol != 0x0806 )
			return FALSE;
		}

	return TRUE;
}


BOOLEAN RTMPCheckEtherType(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket)
{
	USHORT	TypeLen;
	UCHAR	Byte0, Byte1;
	PUCHAR	pSrcBuf;
	UINT32	pktLen;
	UINT16 	srcPort, dstPort;
	BOOLEAN	status = TRUE;


	pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
	pktLen = GET_OS_PKT_LEN(pPacket);

	ASSERT(pSrcBuf);

	RTMP_SET_PACKET_SPECIFIC(pPacket, 0);

	
	TypeLen = (pSrcBuf[12] << 8) + pSrcBuf[13];

	pSrcBuf += LENGTH_802_3;	

	if (TypeLen <= 1500)
	{	
		
		if (pSrcBuf[0] == 0xAA && pSrcBuf[1] == 0xAA && pSrcBuf[2] == 0x03)
		{
			Sniff2BytesFromNdisBuffer(pSrcBuf, 6, &Byte0, &Byte1);
			RTMP_SET_PACKET_LLCSNAP(pPacket, 1);
			TypeLen = (USHORT)((Byte0 << 8) + Byte1);
			pSrcBuf += 8; 
		}
		else
		{
			
		}
	}

	
	if (TypeLen == 0x8100)
	{
		

		

		RTMP_SET_PACKET_VLAN(pPacket, 1);
		Sniff2BytesFromNdisBuffer(pSrcBuf, 2, &Byte0, &Byte1);
		TypeLen = (USHORT)((Byte0 << 8) + Byte1);

		pSrcBuf += 4; 
	}

	switch (TypeLen)
	{
		case 0x0800:
			{
				ASSERT((pktLen > 34));
				if (*(pSrcBuf + 9) == 0x11)
				{	
					ASSERT((pktLen > 34));	

					pSrcBuf += 20;	
					srcPort = OS_NTOHS(*((UINT16 *)pSrcBuf));
					dstPort = OS_NTOHS(*((UINT16 *)(pSrcBuf +2)));

					if ((srcPort==0x44 && dstPort==0x43) || (srcPort==0x43 && dstPort==0x44))
					{	
						RTMP_SET_PACKET_DHCP(pPacket, 1);
					}
				}
			}
			break;
		case 0x0806:
			{
				
				RTMP_SET_PACKET_DHCP(pPacket, 1);
			}
			break;
		case 0x888e:
			{
				
				RTMP_SET_PACKET_EAPOL(pPacket, 1);
			}
			break;
		default:
			status = FALSE;
			break;
	}

	return status;

}



VOID Update_Rssi_Sample(
	IN PRTMP_ADAPTER	pAd,
	IN RSSI_SAMPLE		*pRssi,
	IN PRXWI_STRUC		pRxWI)
		{
	CHAR	rssi0 = pRxWI->RSSI0;
	CHAR	rssi1 = pRxWI->RSSI1;
	CHAR	rssi2 = pRxWI->RSSI2;

	if (rssi0 != 0)
	{
		pRssi->LastRssi0	= ConvertToRssi(pAd, (CHAR)rssi0, RSSI_0);
		pRssi->AvgRssi0X8	= (pRssi->AvgRssi0X8 - pRssi->AvgRssi0) + pRssi->LastRssi0;
		pRssi->AvgRssi0	= pRssi->AvgRssi0X8 >> 3;
	}

	if (rssi1 != 0)
	{
		pRssi->LastRssi1	= ConvertToRssi(pAd, (CHAR)rssi1, RSSI_1);
		pRssi->AvgRssi1X8	= (pRssi->AvgRssi1X8 - pRssi->AvgRssi1) + pRssi->LastRssi1;
		pRssi->AvgRssi1	= pRssi->AvgRssi1X8 >> 3;
	}

	if (rssi2 != 0)
	{
		pRssi->LastRssi2	= ConvertToRssi(pAd, (CHAR)rssi2, RSSI_2);
		pRssi->AvgRssi2X8  = (pRssi->AvgRssi2X8 - pRssi->AvgRssi2) + pRssi->LastRssi2;
		pRssi->AvgRssi2 = pRssi->AvgRssi2X8 >> 3;
	}
}




VOID Indicate_Legacy_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID)
{
	PNDIS_PACKET	pRxPacket = pRxBlk->pRxPacket;
	UCHAR			Header802_3[LENGTH_802_3];

	
	
	
	
	RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);

	if (pRxBlk->DataSize > MAX_RX_PKT_LEN)
	{

		
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}


	STATS_INC_RX_PACKETS(pAd, FromWhichBSSID);

#ifdef RT2870
	if (pAd->CommonCfg.bDisableReordering == 0)
	{
		PBA_REC_ENTRY		pBAEntry;
		ULONG				Now32;
		UCHAR				Wcid = pRxBlk->pRxWI->WirelessCliID;
		UCHAR				TID = pRxBlk->pRxWI->TID;
		USHORT				Idx;

#define REORDERING_PACKET_TIMEOUT		((100 * HZ)/1000)	

		if (Wcid < MAX_LEN_OF_MAC_TABLE)
		{
			Idx = pAd->MacTab.Content[Wcid].BARecWcidArray[TID];
			if (Idx != 0)
			{
				pBAEntry = &pAd->BATable.BARecEntry[Idx];
				
				NdisGetSystemUpTime(&Now32);
				if ((pBAEntry->list.qlen > 0) &&
					 RTMP_TIME_AFTER((unsigned long)Now32, (unsigned long)(pBAEntry->LastIndSeqAtTimer+(REORDERING_PACKET_TIMEOUT)))
	   				)
				{
					printk("Indicate_Legacy_Packet():flush reordering_timeout_mpdus! RxWI->Flags=%d, pRxWI.TID=%d, RxD->AMPDU=%d!\n", pRxBlk->Flags, pRxBlk->pRxWI->TID, pRxBlk->RxD.AMPDU);
					hex_dump("Dump the legacy Packet:", GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket), 64);
					ba_flush_reordering_timeout_mpdus(pAd, pBAEntry, Now32);
				}
			}
		}
	}
#endif 

	wlan_802_11_to_802_3_packet(pAd, pRxBlk, Header802_3, FromWhichBSSID);

	
	
	
	ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pRxPacket, FromWhichBSSID);
}



VOID CmmRxnonRalinkFrameIndicate(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID)
{
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU) && (pAd->CommonCfg.bDisableReordering == 0))
	{
		Indicate_AMPDU_Packet(pAd, pRxBlk, FromWhichBSSID);
	}
	else
	{
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU))
		{
			
			Indicate_AMSDU_Packet(pAd, pRxBlk, FromWhichBSSID);
		}
		else
		{
			Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);
		}
	}
}


VOID CmmRxRalinkFrameIndicate(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID)
{
	UCHAR			Header802_3[LENGTH_802_3];
	UINT16			Msdu2Size;
	UINT16 			Payload1Size, Payload2Size;
	PUCHAR 			pData2;
	PNDIS_PACKET	pPacket2 = NULL;



	Msdu2Size = *(pRxBlk->pData) + (*(pRxBlk->pData+1) << 8);

	if ((Msdu2Size <= 1536) && (Msdu2Size < pRxBlk->DataSize))
	{
		
		pRxBlk->pData += 2;
		pRxBlk->DataSize -= 2;
	}
	else
	{
		
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	
	RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);

	ASSERT(pRxBlk->pRxPacket);

	
	pAd->RalinkCounters.OneSecRxAggregationCount ++;
	Payload1Size = pRxBlk->DataSize - Msdu2Size;
	Payload2Size = Msdu2Size - LENGTH_802_3;

	pData2 = pRxBlk->pData + Payload1Size + LENGTH_802_3;

	pPacket2 = duplicate_pkt(pAd, (pData2-LENGTH_802_3), LENGTH_802_3, pData2, Payload2Size, FromWhichBSSID);

	if (!pPacket2)
	{
		
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	
	pRxBlk->DataSize = Payload1Size;
	wlan_802_11_to_802_3_packet(pAd, pRxBlk, Header802_3, FromWhichBSSID);

	ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pRxBlk->pRxPacket, FromWhichBSSID);

	if (pPacket2)
	{
		ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pPacket2, FromWhichBSSID);
	}
}


#define RESET_FRAGFRAME(_fragFrame) \
	{								\
		_fragFrame.RxSize = 0;		\
		_fragFrame.Sequence = 0;	\
		_fragFrame.LastFrag = 0;	\
		_fragFrame.Flags = 0;		\
	}


PNDIS_PACKET RTMPDeFragmentDataFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk)
{
	PHEADER_802_11	pHeader = pRxBlk->pHeader;
	PNDIS_PACKET	pRxPacket = pRxBlk->pRxPacket;
	UCHAR			*pData = pRxBlk->pData;
	USHORT			DataSize = pRxBlk->DataSize;
	PNDIS_PACKET	pRetPacket = NULL;
	UCHAR			*pFragBuffer = NULL;
	BOOLEAN 		bReassDone = FALSE;
	UCHAR			HeaderRoom = 0;


	ASSERT(pHeader);

	HeaderRoom = pData - (UCHAR *)pHeader;

	
	if (pHeader->Frag == 0)		
	{
		
		if (pHeader->FC.MoreFrag)
		{
			ASSERT(pAd->FragFrame.pFragPacket);
			pFragBuffer = GET_OS_PKT_DATAPTR(pAd->FragFrame.pFragPacket);
			pAd->FragFrame.RxSize   = DataSize + HeaderRoom;
			NdisMoveMemory(pFragBuffer,	 pHeader, pAd->FragFrame.RxSize);
			pAd->FragFrame.Sequence = pHeader->Sequence;
			pAd->FragFrame.LastFrag = pHeader->Frag;	   
			ASSERT(pAd->FragFrame.LastFrag == 0);
			goto done;	
		}
	}
	else	
	{
		if ((pHeader->Sequence != pAd->FragFrame.Sequence) ||
			(pHeader->Frag != (pAd->FragFrame.LastFrag + 1)))
		{
			
			
			RESET_FRAGFRAME(pAd->FragFrame);
			DBGPRINT(RT_DEBUG_ERROR, ("Fragment is not the same sequence or out of fragment number order.\n"));
			goto done; 
		}
		else if ((pAd->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE)
		{
			
			
			RESET_FRAGFRAME(pAd->FragFrame);
			DBGPRINT(RT_DEBUG_ERROR, ("Fragment frame is too large, it exeeds the maximum frame size.\n"));
			goto done; 
		}

        
		
		
		
		if (NdisEqualMemory(pData, SNAP_802_1H, sizeof(SNAP_802_1H)))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Find another LLC at Middle or End fragment(SN=%d, Frag=%d)\n", pHeader->Sequence, pHeader->Frag));
			goto done; 
		}

		pFragBuffer = GET_OS_PKT_DATAPTR(pAd->FragFrame.pFragPacket);

		
		NdisMoveMemory((pFragBuffer + pAd->FragFrame.RxSize), pData, DataSize);
		pAd->FragFrame.RxSize  += DataSize;
		pAd->FragFrame.LastFrag = pHeader->Frag;	   

		
		if (pHeader->FC.MoreFrag == FALSE)
		{
			bReassDone = TRUE;
		}
	}

done:
	
	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);

	
	
	if (bReassDone)
	{
		PNDIS_PACKET pNewFragPacket;

		
		pNewFragPacket = RTMP_AllocateFragPacketBuffer(pAd, RX_BUFFER_NORMSIZE);
		if (pNewFragPacket)
		{
			
			pRetPacket = pAd->FragFrame.pFragPacket;
			pAd->FragFrame.pFragPacket = pNewFragPacket;
			pRxBlk->pHeader = (PHEADER_802_11) GET_OS_PKT_DATAPTR(pRetPacket);
			pRxBlk->pData = (UCHAR *)pRxBlk->pHeader + HeaderRoom;
			pRxBlk->DataSize = pAd->FragFrame.RxSize - HeaderRoom;
			pRxBlk->pRxPacket = pRetPacket;
		}
		else
		{
			RESET_FRAGFRAME(pAd->FragFrame);
		}
	}

	return pRetPacket;
}


VOID Indicate_AMSDU_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID)
{
	UINT			nMSDU;

	update_os_packet_info(pAd, pRxBlk, FromWhichBSSID);
	RTMP_SET_PACKET_IF(pRxBlk->pRxPacket, FromWhichBSSID);
	nMSDU = deaggregate_AMSDU_announce(pAd, pRxBlk->pRxPacket, pRxBlk->pData, pRxBlk->DataSize);
}

VOID Indicate_EAPOL_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	{
		pEntry = &pAd->MacTab.Content[BSSID_WCID];
		STARxEAPOLFrameIndicate(pAd, pEntry, pRxBlk, FromWhichBSSID);
		return;
	}

	if (pEntry == NULL)
	{
		DBGPRINT(RT_DEBUG_WARN, ("Indicate_EAPOL_Packet: drop and release the invalid packet.\n"));
		
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}
}

#define BCN_TBTT_OFFSET		64	
VOID ReSyncBeaconTime(
	IN  PRTMP_ADAPTER   pAd)
{

	UINT32  Offset;


	Offset = (pAd->TbttTickCount) % (BCN_TBTT_OFFSET);

	pAd->TbttTickCount++;

	
	
	
	
	if (Offset == (BCN_TBTT_OFFSET-2))
	{
		BCN_TIME_CFG_STRUC csr;
		RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
		csr.field.BeaconInterval = (pAd->CommonCfg.BeaconPeriod << 4) - 1 ;	
		RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
	}
	else
	{
		if (Offset == (BCN_TBTT_OFFSET-1))
		{
			BCN_TIME_CFG_STRUC csr;

			RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
			csr.field.BeaconInterval = (pAd->CommonCfg.BeaconPeriod) << 4; 
			RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
		}
	}
}

