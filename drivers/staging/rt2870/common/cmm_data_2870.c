

#include "../rt_config.h"



static inline NDIS_STATUS RtmpUSBCanDoWrite(
	IN RTMP_ADAPTER		*pAd,
	IN UCHAR			QueIdx,
	IN HT_TX_CONTEXT 	*pHTTXContext)
{
	NDIS_STATUS	canWrite = NDIS_STATUS_RESOURCES;

	if (((pHTTXContext->CurWritePosition) < pHTTXContext->NextBulkOutPosition) && (pHTTXContext->CurWritePosition + LOCAL_TXBUF_SIZE) > pHTTXContext->NextBulkOutPosition)
	{
		DBGPRINT(RT_DEBUG_ERROR,("RtmpUSBCanDoWrite c1!\n"));
		RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << QueIdx));
	}
	else if ((pHTTXContext->CurWritePosition == 8) && (pHTTXContext->NextBulkOutPosition < LOCAL_TXBUF_SIZE))
	{
		DBGPRINT(RT_DEBUG_ERROR,("RtmpUSBCanDoWrite c2!\n"));
		RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << QueIdx));
	}
	else if (pHTTXContext->bCurWriting == TRUE)
	{
		DBGPRINT(RT_DEBUG_ERROR,("RtmpUSBCanDoWrite c3!\n"));
	}
	else
	{
		canWrite = NDIS_STATUS_SUCCESS;
	}


	return canWrite;
}


USHORT RtmpUSB_WriteSubTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	BOOLEAN			bIsLast,
	OUT	USHORT			*FreeNumber)
{

	
	return 0;

}

USHORT	RtmpUSB_WriteFragTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	UCHAR			fragNum,
	OUT	USHORT			*FreeNumber)
{
	HT_TX_CONTEXT	*pHTTXContext;
	USHORT			hwHdrLen;	
	UINT32			fillOffset;
	TXINFO_STRUC	*pTxInfo;
	TXWI_STRUC		*pTxWI;
	PUCHAR			pWirelessPacket = NULL;
	UCHAR			QueIdx;
	NDIS_STATUS		Status;
	unsigned long	IrqFlags;
	UINT32			USBDMApktLen = 0, DMAHdrLen, padding;
	BOOLEAN			TxQLastRound = FALSE;

	
	
	
	QueIdx = pTxBlk->QueIdx;
	pHTTXContext  = &pAd->TxContext[QueIdx];

	RTMP_IRQ_LOCK(&pAd->TxContextQueueLock[QueIdx], IrqFlags);

	pHTTXContext  = &pAd->TxContext[QueIdx];
	fillOffset = pHTTXContext->CurWritePosition;

	if(fragNum == 0)
	{
		
		Status = RtmpUSBCanDoWrite(pAd, QueIdx, pHTTXContext);
		if (Status == NDIS_STATUS_SUCCESS)
		{
			pHTTXContext->bCurWriting = TRUE;

			
			if ((pHTTXContext->ENextBulkOutPosition == pHTTXContext->CurWritePosition))
			{
				pHTTXContext->ENextBulkOutPosition += 8;
				pHTTXContext->CurWritePosition += 8;
				fillOffset += 8;
			}
			pTxBlk->Priv = 0;
			pHTTXContext->CurWriteRealPos = pHTTXContext->CurWritePosition;
		}
		else
		{
			RTMP_IRQ_UNLOCK(&pAd->TxContextQueueLock[QueIdx], IrqFlags);

			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return(Status);
		}
	}
	else
	{
		
		Status = ((pHTTXContext->bCurWriting == TRUE) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_FAILURE);
		if (Status == NDIS_STATUS_SUCCESS)
		{
			fillOffset += pTxBlk->Priv;
		}
		else
		{
			RTMP_IRQ_UNLOCK(&pAd->TxContextQueueLock[QueIdx], IrqFlags);

			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return(Status);
		}
	}

	NdisZeroMemory((PUCHAR)(&pTxBlk->HeaderBuf[0]), TXINFO_SIZE);
	pTxInfo = (PTXINFO_STRUC)(&pTxBlk->HeaderBuf[0]);
	pTxWI= (PTXWI_STRUC)(&pTxBlk->HeaderBuf[TXINFO_SIZE]);

	pWirelessPacket = &pHTTXContext->TransferBuffer->field.WirelessPacket[fillOffset];

	
	
	hwHdrLen = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;

	
	DMAHdrLen = TXWI_SIZE + hwHdrLen;
	USBDMApktLen = DMAHdrLen + pTxBlk->SrcBufLen;
	padding = (4 - (USBDMApktLen % 4)) & 0x03;	
	USBDMApktLen += padding;

	pTxBlk->Priv += (TXINFO_SIZE + USBDMApktLen);

	
	RTMPWriteTxInfo(pAd, pTxInfo, (USHORT)(USBDMApktLen), FALSE, FIFO_EDCA, FALSE ,  FALSE);

	if (fragNum == pTxBlk->TotalFragNum)
	{
		pTxInfo->USBDMATxburst = 0;
		if ((pHTTXContext->CurWritePosition + pTxBlk->Priv + 3906)> MAX_TXBULK_LIMIT)
		{
			pTxInfo->SwUseLastRound = 1;
			TxQLastRound = TRUE;
		}
	}
	else
	{
		pTxInfo->USBDMATxburst = 1;
	}

	NdisMoveMemory(pWirelessPacket, pTxBlk->HeaderBuf, TXINFO_SIZE + TXWI_SIZE + hwHdrLen);
	pWirelessPacket += (TXINFO_SIZE + TXWI_SIZE + hwHdrLen);
	pHTTXContext->CurWriteRealPos += (TXINFO_SIZE + TXWI_SIZE + hwHdrLen);

	RTMP_IRQ_UNLOCK(&pAd->TxContextQueueLock[QueIdx], IrqFlags);

	NdisMoveMemory(pWirelessPacket, pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);

	
	pWirelessPacket += pTxBlk->SrcBufLen;
	NdisZeroMemory(pWirelessPacket, padding + 8);

	if (fragNum == pTxBlk->TotalFragNum)
	{
		RTMP_IRQ_LOCK(&pAd->TxContextQueueLock[QueIdx], IrqFlags);

		
		pHTTXContext->CurWritePosition += pTxBlk->Priv;
		if (TxQLastRound == TRUE)
			pHTTXContext->CurWritePosition = 8;
		pHTTXContext->CurWriteRealPos = pHTTXContext->CurWritePosition;


		
	pHTTXContext->bCurWriting = FALSE;

		RTMP_IRQ_UNLOCK(&pAd->TxContextQueueLock[QueIdx], IrqFlags);

		
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_SUCCESS);
	}


	return(Status);

}


USHORT RtmpUSB_WriteSingleTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	BOOLEAN			bIsLast,
	OUT	USHORT			*FreeNumber)
{
	HT_TX_CONTEXT	*pHTTXContext;
	USHORT			hwHdrLen;
	UINT32			fillOffset;
	TXINFO_STRUC	*pTxInfo;
	TXWI_STRUC		*pTxWI;
	PUCHAR			pWirelessPacket;
	UCHAR			QueIdx;
	unsigned long	IrqFlags;
	NDIS_STATUS		Status;
	UINT32			USBDMApktLen = 0, DMAHdrLen, padding;
	BOOLEAN			bTxQLastRound = FALSE;

	
	


	
	
	
	QueIdx = pTxBlk->QueIdx;

	RTMP_IRQ_LOCK(&pAd->TxContextQueueLock[QueIdx], IrqFlags);
	pHTTXContext  = &pAd->TxContext[QueIdx];
	fillOffset = pHTTXContext->CurWritePosition;



	
	Status = RtmpUSBCanDoWrite(pAd, QueIdx, pHTTXContext);
	if(Status == NDIS_STATUS_SUCCESS)
	{
		pHTTXContext->bCurWriting = TRUE;

		pTxInfo = (PTXINFO_STRUC)(&pTxBlk->HeaderBuf[0]);
		pTxWI= (PTXWI_STRUC)(&pTxBlk->HeaderBuf[TXINFO_SIZE]);

		
		if ((pHTTXContext->ENextBulkOutPosition == pHTTXContext->CurWritePosition))
		{
			pHTTXContext->ENextBulkOutPosition += 8;
			pHTTXContext->CurWritePosition += 8;
			fillOffset += 8;
		}
		pHTTXContext->CurWriteRealPos = pHTTXContext->CurWritePosition;

		pWirelessPacket = &pHTTXContext->TransferBuffer->field.WirelessPacket[fillOffset];

		
		
		hwHdrLen = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;

		
		DMAHdrLen = TXWI_SIZE + hwHdrLen;
		USBDMApktLen = DMAHdrLen + pTxBlk->SrcBufLen;
		padding = (4 - (USBDMApktLen % 4)) & 0x03;	
		USBDMApktLen += padding;

		pTxBlk->Priv = (TXINFO_SIZE + USBDMApktLen);

		
		
		RTMPWriteTxInfo(pAd, pTxInfo, (USHORT)(USBDMApktLen), FALSE, FIFO_EDCA, FALSE ,  FALSE);

		if ((pHTTXContext->CurWritePosition + 3906 + pTxBlk->Priv) > MAX_TXBULK_LIMIT)
		{
			pTxInfo->SwUseLastRound = 1;
			bTxQLastRound = TRUE;
		}
		NdisMoveMemory(pWirelessPacket, pTxBlk->HeaderBuf, TXINFO_SIZE + TXWI_SIZE + hwHdrLen);
		pWirelessPacket += (TXINFO_SIZE + TXWI_SIZE + hwHdrLen);

		
		
		
		
		
		
		
		
		
		
		pHTTXContext->CurWriteRealPos += (TXINFO_SIZE + TXWI_SIZE + hwHdrLen);
		RTMP_IRQ_UNLOCK(&pAd->TxContextQueueLock[QueIdx], IrqFlags);

		NdisMoveMemory(pWirelessPacket, pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
		pWirelessPacket += pTxBlk->SrcBufLen;
		NdisZeroMemory(pWirelessPacket, padding + 8);

		RTMP_IRQ_LOCK(&pAd->TxContextQueueLock[QueIdx], IrqFlags);

		pHTTXContext->CurWritePosition += pTxBlk->Priv;
		if (bTxQLastRound)
			pHTTXContext->CurWritePosition = 8;
		pHTTXContext->CurWriteRealPos = pHTTXContext->CurWritePosition;

	pHTTXContext->bCurWriting = FALSE;
	}


	RTMP_IRQ_UNLOCK(&pAd->TxContextQueueLock[QueIdx], IrqFlags);


	
	RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_SUCCESS);

	return(Status);

}


USHORT RtmpUSB_WriteMultiTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	UCHAR			frameNum,
	OUT	USHORT			*FreeNumber)
{
	HT_TX_CONTEXT	*pHTTXContext;
	USHORT			hwHdrLen;	
	UINT32			fillOffset;
	TXINFO_STRUC	*pTxInfo;
	TXWI_STRUC		*pTxWI;
	PUCHAR			pWirelessPacket = NULL;
	UCHAR			QueIdx;
	NDIS_STATUS		Status;
	unsigned long	IrqFlags;
	

	
	
	
	QueIdx = pTxBlk->QueIdx;
	pHTTXContext  = &pAd->TxContext[QueIdx];

	RTMP_IRQ_LOCK(&pAd->TxContextQueueLock[QueIdx], IrqFlags);

	if(frameNum == 0)
	{
		
		Status = RtmpUSBCanDoWrite(pAd, QueIdx, pHTTXContext);
		if (Status == NDIS_STATUS_SUCCESS)
		{
			pHTTXContext->bCurWriting = TRUE;

			pTxInfo = (PTXINFO_STRUC)(&pTxBlk->HeaderBuf[0]);
			pTxWI= (PTXWI_STRUC)(&pTxBlk->HeaderBuf[TXINFO_SIZE]);


			
			if ((pHTTXContext->ENextBulkOutPosition == pHTTXContext->CurWritePosition))
			{

				pHTTXContext->CurWritePosition += 8;
				pHTTXContext->ENextBulkOutPosition += 8;
			}
			fillOffset = pHTTXContext->CurWritePosition;
			pHTTXContext->CurWriteRealPos = pHTTXContext->CurWritePosition;

			pWirelessPacket = &pHTTXContext->TransferBuffer->field.WirelessPacket[fillOffset];

			
			
			
			if (pTxBlk->TxFrameType == TX_AMSDU_FRAME)
				
				hwHdrLen = pTxBlk->MpduHeaderLen-LENGTH_AMSDU_SUBFRAMEHEAD + pTxBlk->HdrPadLen + LENGTH_AMSDU_SUBFRAMEHEAD;
			else if (pTxBlk->TxFrameType == TX_RALINK_FRAME)
				
				hwHdrLen = pTxBlk->MpduHeaderLen-LENGTH_ARALINK_HEADER_FIELD + pTxBlk->HdrPadLen + LENGTH_ARALINK_HEADER_FIELD;
			else
				
				hwHdrLen = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;

			
			pTxBlk->Priv = TXINFO_SIZE + TXWI_SIZE + hwHdrLen;

			
			RTMPWriteTxInfo(pAd, pTxInfo, (USHORT)(pTxBlk->Priv), FALSE, FIFO_EDCA, FALSE ,  FALSE);

			
			NdisMoveMemory(pWirelessPacket, pTxBlk->HeaderBuf, pTxBlk->Priv);
			pHTTXContext->CurWriteRealPos += pTxBlk->Priv;
			pWirelessPacket += pTxBlk->Priv;
		}
	}
	else
	{	

		Status = ((pHTTXContext->bCurWriting == TRUE) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_FAILURE);
		if (Status == NDIS_STATUS_SUCCESS)
		{
			fillOffset =  (pHTTXContext->CurWritePosition + pTxBlk->Priv);
			pWirelessPacket = &pHTTXContext->TransferBuffer->field.WirelessPacket[fillOffset];

			
			NdisMoveMemory(pWirelessPacket, pTxBlk->HeaderBuf, pTxBlk->MpduHeaderLen);
			pWirelessPacket += (pTxBlk->MpduHeaderLen);
			pTxBlk->Priv += pTxBlk->MpduHeaderLen;
		}
		else
		{	
			DBGPRINT(RT_DEBUG_ERROR, ("WriteMultiTxResource():bCurWriting is FALSE when handle sub-sequent frames.\n"));
			Status = NDIS_STATUS_FAILURE;
		}
	}


	
	
	
	
	
	
	
	
	
	
	RTMP_IRQ_UNLOCK(&pAd->TxContextQueueLock[QueIdx], IrqFlags);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR,("WriteMultiTxResource: CWPos = %ld, NBOutPos = %ld.\n", pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition));
		goto done;
	}

	
	NdisMoveMemory(pWirelessPacket, pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	pWirelessPacket += pTxBlk->SrcBufLen;
	pTxBlk->Priv += pTxBlk->SrcBufLen;

done:
	
	RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_SUCCESS);

	return(Status);

}


VOID RtmpUSB_FinalWriteTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	USHORT			totalMPDUSize,
	IN	USHORT			TxIdx)
{
	UCHAR			QueIdx;
	HT_TX_CONTEXT	*pHTTXContext;
	UINT32			fillOffset;
	TXINFO_STRUC	*pTxInfo;
	TXWI_STRUC		*pTxWI;
	UINT32			USBDMApktLen, padding;
	unsigned long	IrqFlags;
	PUCHAR			pWirelessPacket;

	QueIdx = pTxBlk->QueIdx;
	pHTTXContext  = &pAd->TxContext[QueIdx];

	RTMP_IRQ_LOCK(&pAd->TxContextQueueLock[QueIdx], IrqFlags);

	if (pHTTXContext->bCurWriting == TRUE)
	{
		fillOffset = pHTTXContext->CurWritePosition;
		if (((pHTTXContext->ENextBulkOutPosition == pHTTXContext->CurWritePosition) || ((pHTTXContext->ENextBulkOutPosition-8) == pHTTXContext->CurWritePosition))
			&& (pHTTXContext->bCopySavePad == TRUE))
			pWirelessPacket = (PUCHAR)(&pHTTXContext->SavedPad[0]);
		else
			pWirelessPacket = (PUCHAR)(&pHTTXContext->TransferBuffer->field.WirelessPacket[fillOffset]);

		
		
		
		
		pTxInfo = (PTXINFO_STRUC)(pWirelessPacket);

		
		USBDMApktLen = pTxBlk->Priv - TXINFO_SIZE;
		padding = (4 - (USBDMApktLen % 4)) & 0x03;	
		USBDMApktLen += padding;

		pTxInfo->USBDMATxPktLen = USBDMApktLen;

		
		
		
		pTxWI= (PTXWI_STRUC)(pWirelessPacket + TXINFO_SIZE);
		pTxWI->MPDUtotalByteCount = totalMPDUSize;

		
		
		
		pHTTXContext->CurWritePosition += (TXINFO_SIZE + USBDMApktLen);
		if ((pHTTXContext->CurWritePosition + 3906)> MAX_TXBULK_LIMIT)
		{	
			pHTTXContext->CurWritePosition = 8;
			pTxInfo->SwUseLastRound = 1;
		}
		pHTTXContext->CurWriteRealPos = pHTTXContext->CurWritePosition;


		
		
		
		pWirelessPacket = (&pHTTXContext->TransferBuffer->field.WirelessPacket[fillOffset + pTxBlk->Priv]);
		NdisZeroMemory(pWirelessPacket, padding + 8);

		
		pHTTXContext->bCurWriting = FALSE;

	}
	else
	{	
		DBGPRINT(RT_DEBUG_ERROR, ("FinalWriteTxResource():bCurWriting is FALSE when handle last frames.\n"));
	}

	RTMP_IRQ_UNLOCK(&pAd->TxContextQueueLock[QueIdx], IrqFlags);

}


VOID RtmpUSBDataLastTxIdx(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			QueIdx,
	IN	USHORT			TxIdx)
{
	
}



VOID RtmpUSBDataKickOut(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	UCHAR			QueIdx)
{
	RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << QueIdx));
	RTUSBKickBulkOut(pAd);

}



int RtmpUSBMgmtKickOut(
	IN RTMP_ADAPTER 	*pAd,
	IN UCHAR 			QueIdx,
	IN PNDIS_PACKET		pPacket,
	IN PUCHAR			pSrcBufVA,
	IN UINT 			SrcBufLen)
{
	PTXINFO_STRUC	pTxInfo;
	ULONG			BulkOutSize;
	UCHAR			padLen;
	PUCHAR			pDest;
	ULONG			SwIdx = pAd->MgmtRing.TxCpuIdx;
	PTX_CONTEXT		pMLMEContext = (PTX_CONTEXT)pAd->MgmtRing.Cell[SwIdx].AllocVa;
	unsigned long	IrqFlags;


	pTxInfo = (PTXINFO_STRUC)(pSrcBufVA);

	
	BulkOutSize = SrcBufLen;
	BulkOutSize = (BulkOutSize + 3) & (~3);
	RTMPWriteTxInfo(pAd, pTxInfo, (USHORT)(BulkOutSize - TXINFO_SIZE), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);

	BulkOutSize += 4; 

	
	if ((BulkOutSize % pAd->BulkOutMaxPacketSize) == 0)
		BulkOutSize += 4;

	padLen = BulkOutSize - SrcBufLen;
	ASSERT((padLen <= RTMP_PKT_TAIL_PADDING));

	
	pDest = (PUCHAR)(pSrcBufVA + SrcBufLen);
	skb_put(GET_OS_PKT_TYPE(pPacket), padLen);
	NdisZeroMemory(pDest, padLen);

	RTMP_IRQ_LOCK(&pAd->MLMEBulkOutLock, IrqFlags);

	pAd->MgmtRing.Cell[pAd->MgmtRing.TxCpuIdx].pNdisPacket = pPacket;
	pMLMEContext->TransferBuffer = (PTX_BUFFER)(GET_OS_PKT_DATAPTR(pPacket));

	
	pMLMEContext->BulkOutSize = BulkOutSize;
	pMLMEContext->InUse = TRUE;
	pMLMEContext->bWaitingBulkOut = TRUE;


	
	

	
	

	
	

	
	pAd->MgmtRing.TxSwFreeIdx--;
	INC_RING_INDEX(pAd->MgmtRing.TxCpuIdx, MGMT_RING_SIZE);

	RTMP_IRQ_UNLOCK(&pAd->MLMEBulkOutLock, IrqFlags);

	RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_MLME);
	
	RTUSBKickBulkOut(pAd);

	return 0;
}


VOID RtmpUSBNullFrameKickOut(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR		QueIdx,
	IN UCHAR		*pNullFrame,
	IN UINT32		frameLen)
{
	if (pAd->NullContext.InUse == FALSE)
	{
		PTX_CONTEXT		pNullContext;
		PTXINFO_STRUC	pTxInfo;
		PTXWI_STRUC		pTxWI;
		PUCHAR			pWirelessPkt;

		pNullContext = &(pAd->NullContext);

		
		pNullContext->InUse = TRUE;
		pWirelessPkt = (PUCHAR)&pNullContext->TransferBuffer->field.WirelessPacket[0];

		RTMPZeroMemory(&pWirelessPkt[0], 100);
		pTxInfo = (PTXINFO_STRUC)&pWirelessPkt[0];
		RTMPWriteTxInfo(pAd, pTxInfo, (USHORT)(sizeof(HEADER_802_11)+TXWI_SIZE), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);
		pTxInfo->QSEL = FIFO_EDCA;
		pTxWI = (PTXWI_STRUC)&pWirelessPkt[TXINFO_SIZE];
		RTMPWriteTxWI(pAd, pTxWI,  FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, 0, BSSID_WCID, (sizeof(HEADER_802_11)),
			0, 0, (UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS, IFS_HTTXOP, FALSE, &pAd->CommonCfg.MlmeTransmit);
		RTMPMoveMemory(&pWirelessPkt[TXWI_SIZE+TXINFO_SIZE], &pAd->NullFrame, sizeof(HEADER_802_11));
		pAd->NullContext.BulkOutSize =  TXINFO_SIZE + TXWI_SIZE + sizeof(pAd->NullFrame) + 4;

		
		
		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - send NULL Frame @%d Mbps...\n", RateIdToMbps[pAd->CommonCfg.TxRate]));
		RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NULL);

		
		RTUSBKickBulkOut(pAd);
	}

}


NDIS_STATUS	RTMPCheckRxError(
	IN	PRTMP_ADAPTER	pAd,
	IN	PHEADER_802_11	pHeader,
	IN	PRXWI_STRUC	pRxWI,
	IN	PRT28XX_RXD_STRUC	pRxINFO)
{
	PCIPHER_KEY pWpaKey;
	INT	dBm;

	if (pAd->bPromiscuous == TRUE)
		return(NDIS_STATUS_SUCCESS);
	if(pRxINFO == NULL)
		return(NDIS_STATUS_FAILURE);

	
	if (pRxINFO->Crc)
	{
		
		dBm = (INT) (pRxWI->RSSI0) - pAd->BbpRssiToDbmDelta;
		if (dBm <= -87)
			pAd->StaCfg.RPIDensity[0] += 1;
		else if (dBm <= -82)
			pAd->StaCfg.RPIDensity[1] += 1;
		else if (dBm <= -77)
			pAd->StaCfg.RPIDensity[2] += 1;
		else if (dBm <= -72)
			pAd->StaCfg.RPIDensity[3] += 1;
		else if (dBm <= -67)
			pAd->StaCfg.RPIDensity[4] += 1;
		else if (dBm <= -62)
			pAd->StaCfg.RPIDensity[5] += 1;
		else if (dBm <= -57)
			pAd->StaCfg.RPIDensity[6] += 1;
		else if (dBm > -57)
			pAd->StaCfg.RPIDensity[7] += 1;

		return(NDIS_STATUS_FAILURE);
	}

	
	pAd->StaCfg.CLBusyBytes += (pRxWI->MPDUtotalByteCount+ 14);

	
	if (pHeader->FC.ToDs)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("Err;FC.ToDs\n"));
		return NDIS_STATUS_FAILURE;
	}

	
	if (pRxWI->MPDUtotalByteCount > MAX_AGGREGATION_SIZE)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("received packet too long\n"));
		return NDIS_STATUS_FAILURE;
	}

	
	
	
	

	
	if (pRxINFO->Decrypted && pRxINFO->CipherErr)
	{

		
		
		
		if ((pRxINFO->CipherErr == 2) && pRxINFO->MyBss)
		{
			pWpaKey = &pAd->SharedKey[BSS0][pRxWI->KeyIndex];
			RTMPReportMicError(pAd, pWpaKey);
			DBGPRINT_RAW(RT_DEBUG_ERROR,("Rx MIC Value error\n"));
		}

		if (pRxINFO->Decrypted &&
			(pAd->SharedKey[BSS0][pRxWI->KeyIndex].CipherAlg == CIPHER_AES) &&
			(pHeader->Sequence == pAd->FragFrame.Sequence))
		{
			
			
			
			return(NDIS_STATUS_SUCCESS);
		}

		return(NDIS_STATUS_FAILURE);
	}

	return(NDIS_STATUS_SUCCESS);
}

VOID RT28xxUsbStaAsicForceWakeup(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN       bFromTx)
{
    AUTO_WAKEUP_STRUC	AutoWakeupCfg;

	AutoWakeupCfg.word = 0;
	RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);

	AsicSendCommandToMcu(pAd, 0x31, 0xff, 0x00, 0x02);

	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
}

VOID RT28xxUsbStaAsicSleepThenAutoWakeup(
	IN PRTMP_ADAPTER pAd,
	IN USHORT TbttNumToNextWakeUp)
{
	AUTO_WAKEUP_STRUC	AutoWakeupCfg;

	
	if (TbttNumToNextWakeUp == 0)
		TbttNumToNextWakeUp = 1;

	AutoWakeupCfg.word = 0;
	RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);

	AutoWakeupCfg.field.NumofSleepingTbtt = TbttNumToNextWakeUp - 1;
	AutoWakeupCfg.field.EnableAutoWakeup = 1;
	AutoWakeupCfg.field.AutoLeadTime = 5;
	RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);

	AsicSendCommandToMcu(pAd, 0x30, 0xff, 0xff, 0x02);   

	OPSTATUS_SET_FLAG(pAd, fOP_STATUS_DOZE);

}

VOID RT28xxUsbMlmeRadioOn(
	IN PRTMP_ADAPTER pAd)
{
    DBGPRINT(RT_DEBUG_TRACE,("RT28xxUsbMlmeRadioOn()\n"));

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
		return;

    	AsicSendCommandToMcu(pAd, 0x31, 0xff, 0x00, 0x02);
		RTMPusecDelay(10000);

	NICResetFromError(pAd);

	
	RTMPEnableRxTx(pAd);

#ifdef RT3070
	if (IS_RT3071(pAd))
	{
		RT30xxReverseRFSleepModeSetup(pAd);
	}
#endif 

	
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

	RTUSBBulkReceive(pAd);

	
	RTMPSetLED(pAd, LED_RADIO_ON);
}

VOID RT28xxUsbMlmeRadioOFF(
	IN PRTMP_ADAPTER pAd)
{
	WPDMA_GLO_CFG_STRUC	GloCfg;
	UINT32	Value, i;

	DBGPRINT(RT_DEBUG_TRACE,("RT28xxUsbMlmeRadioOFF()\n"));

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
		return;

	
	RTMPSetLED(pAd, LED_RADIO_OFF);
	
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

	{
		
		if (INFRA_ON(pAd) || ADHOC_ON(pAd))
			LinkDown(pAd, FALSE);
		RTMPusecDelay(10000);

		
		
		BssTableInit(&pAd->ScanTab);
	}

	if (pAd->CommonCfg.BBPCurrentBW == BW_40)
	{
		
		AsicTurnOffRFClk(pAd, pAd->CommonCfg.CentralChannel);
	}
	else
	{
		
		AsicTurnOffRFClk(pAd, pAd->CommonCfg.Channel);
	}

	
	RTUSBReadMACRegister(pAd, WPDMA_GLO_CFG, &GloCfg.word);	   
	GloCfg.field.EnableTxDMA = 0;
	GloCfg.field.EnableRxDMA = 0;
	RTUSBWriteMACRegister(pAd, WPDMA_GLO_CFG, GloCfg.word);	   

	
	i = 0;
	do
	{
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0) && (GloCfg.field.RxDMABusy == 0))
			break;

		RTMPusecDelay(1000);
	}while (i++ < 100);

	
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
	Value &= (0xfffffff3);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);

	AsicSendCommandToMcu(pAd, 0x30, 0xff, 0xff, 0x02);
}

