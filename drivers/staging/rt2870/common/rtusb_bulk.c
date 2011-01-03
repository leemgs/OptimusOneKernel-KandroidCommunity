 

#include "../rt_config.h"

UCHAR	EpToQueue[6]={FIFO_EDCA, FIFO_EDCA, FIFO_EDCA, FIFO_EDCA, FIFO_EDCA, FIFO_MGMT};



void RTUSB_FILL_BULK_URB (struct urb *pUrb,
	struct usb_device *pUsb_Dev,
	unsigned int bulkpipe,
	void *pTransferBuf,
	int BufSize,
	usb_complete_t Complete,
	void *pContext)
{

	usb_fill_bulk_urb(pUrb, pUsb_Dev, bulkpipe, pTransferBuf, BufSize, (usb_complete_t)Complete, pContext);

}

VOID	RTUSBInitTxDesc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTX_CONTEXT		pTxContext,
	IN	UCHAR			BulkOutPipeId,
	IN	usb_complete_t	Func)
{
	PURB				pUrb;
	PUCHAR				pSrc = NULL;
	POS_COOKIE			pObj = (POS_COOKIE) pAd->OS_Cookie;

	pUrb = pTxContext->pUrb;
	ASSERT(pUrb);

	
	pTxContext->BulkOutPipeId = BulkOutPipeId;

	if (pTxContext->bAggregatible)
	{
		pSrc = &pTxContext->TransferBuffer->Aggregation[2];
	}
	else
	{
		pSrc = (PUCHAR) pTxContext->TransferBuffer->field.WirelessPacket;
	}


	
	RTUSB_FILL_BULK_URB(pUrb,
						pObj->pUsb_Dev,
						usb_sndbulkpipe(pObj->pUsb_Dev, pAd->BulkOutEpAddr[BulkOutPipeId]),
						pSrc,
						pTxContext->BulkOutSize,
						Func,
						pTxContext);

	if (pTxContext->bAggregatible)
		pUrb->transfer_dma	= (pTxContext->data_dma + TX_BUFFER_NORMSIZE + 2);
	else
		pUrb->transfer_dma	= pTxContext->data_dma;

	pUrb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

}

VOID	RTUSBInitHTTxDesc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PHT_TX_CONTEXT	pTxContext,
	IN	UCHAR			BulkOutPipeId,
	IN	ULONG			BulkOutSize,
	IN	usb_complete_t	Func)
{
	PURB				pUrb;
	PUCHAR				pSrc = NULL;
	POS_COOKIE			pObj = (POS_COOKIE) pAd->OS_Cookie;

	pUrb = pTxContext->pUrb;
	ASSERT(pUrb);

	
	pTxContext->BulkOutPipeId = BulkOutPipeId;

	pSrc = &pTxContext->TransferBuffer->field.WirelessPacket[pTxContext->NextBulkOutPosition];


	
	RTUSB_FILL_BULK_URB(pUrb,
						pObj->pUsb_Dev,
						usb_sndbulkpipe(pObj->pUsb_Dev, pAd->BulkOutEpAddr[BulkOutPipeId]),
						pSrc,
						BulkOutSize,
						Func,
						pTxContext);

	pUrb->transfer_dma	= (pTxContext->data_dma + pTxContext->NextBulkOutPosition);
	pUrb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

}

VOID	RTUSBInitRxDesc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PRX_CONTEXT		pRxContext)
{
	PURB				pUrb;
	POS_COOKIE			pObj = (POS_COOKIE) pAd->OS_Cookie;
	ULONG				RX_bulk_size;


	pUrb = pRxContext->pUrb;
	ASSERT(pUrb);

	if ( pAd->BulkInMaxPacketSize == 64)
		RX_bulk_size = 4096;
	else
		RX_bulk_size = MAX_RXBULK_SIZE;

	
	RTUSB_FILL_BULK_URB(pUrb,
						pObj->pUsb_Dev,
						usb_rcvbulkpipe(pObj->pUsb_Dev, pAd->BulkInEpAddr),
						&(pRxContext->TransferBuffer[pAd->NextRxBulkInPosition]),
						RX_bulk_size - (pAd->NextRxBulkInPosition),
						(usb_complete_t)RTUSBBulkRxComplete,
						(void *)pRxContext);

	pUrb->transfer_dma	= pRxContext->data_dma + pAd->NextRxBulkInPosition;
	pUrb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;


}



#define BULK_OUT_LOCK(pLock, IrqFlags)	\
		if(1 )	\
			RTMP_IRQ_LOCK((pLock), IrqFlags);

#define BULK_OUT_UNLOCK(pLock, IrqFlags)	\
		if(1 )	\
			RTMP_IRQ_UNLOCK((pLock), IrqFlags);


VOID	RTUSBBulkOutDataPacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			BulkOutPipeId,
	IN	UCHAR			Index)
{

	PHT_TX_CONTEXT	pHTTXContext;
	PURB			pUrb;
	int				ret = 0;
	PTXINFO_STRUC	pTxInfo, pLastTxInfo = NULL;
	PTXWI_STRUC             pTxWI;
	ULONG			TmpBulkEndPos, ThisBulkSize;
	unsigned long	IrqFlags = 0, IrqFlags2 = 0;
	PUCHAR			pWirelessPkt, pAppendant;
	BOOLEAN			bTxQLastRound = FALSE;
	UCHAR			allzero[4]= {0x0,0x0,0x0,0x0};

	BULK_OUT_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
	if ((pAd->BulkOutPending[BulkOutPipeId] == TRUE) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_TX))
	{
		BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
		return;
	}
	pAd->BulkOutPending[BulkOutPipeId] = TRUE;

	if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)
		)
	{
		pAd->BulkOutPending[BulkOutPipeId] = FALSE;
		BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
		return;
	}
	BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);


	pHTTXContext = &(pAd->TxContext[BulkOutPipeId]);

	BULK_OUT_LOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags2);
	if ((pHTTXContext->ENextBulkOutPosition == pHTTXContext->CurWritePosition)
		|| ((pHTTXContext->ENextBulkOutPosition-8) == pHTTXContext->CurWritePosition))
	{
		BULK_OUT_UNLOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags2);

		BULK_OUT_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
		pAd->BulkOutPending[BulkOutPipeId] = FALSE;

		
		RTUSB_CLEAR_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_FRAG << BulkOutPipeId));
		RTUSB_CLEAR_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << BulkOutPipeId));

		BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
		return;
	}

	
	RTUSB_CLEAR_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_FRAG << BulkOutPipeId));
	RTUSB_CLEAR_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << BulkOutPipeId));

	
	
	
	pHTTXContext->NextBulkOutPosition = pHTTXContext->ENextBulkOutPosition;
	ThisBulkSize = 0;
	TmpBulkEndPos = pHTTXContext->NextBulkOutPosition;
	pWirelessPkt = &pHTTXContext->TransferBuffer->field.WirelessPacket[0];

	if ((pHTTXContext->bCopySavePad == TRUE))
	{
		if (RTMPEqualMemory(pHTTXContext->SavedPad, allzero,4))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR,("e1, allzero : %x  %x  %x  %x  %x  %x  %x  %x \n",
				pHTTXContext->SavedPad[0], pHTTXContext->SavedPad[1], pHTTXContext->SavedPad[2],pHTTXContext->SavedPad[3]
				,pHTTXContext->SavedPad[4], pHTTXContext->SavedPad[5], pHTTXContext->SavedPad[6],pHTTXContext->SavedPad[7]));
		}
		NdisMoveMemory(&pWirelessPkt[TmpBulkEndPos], pHTTXContext->SavedPad, 8);
		pHTTXContext->bCopySavePad = FALSE;
		if (pAd->bForcePrintTX == TRUE)
			DBGPRINT(RT_DEBUG_TRACE,("RTUSBBulkOutDataPacket --> COPY PAD. CurWrite = %ld, NextBulk = %ld.   ENextBulk = %ld.\n",   pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition));
	}

	do
	{
		pTxInfo = (PTXINFO_STRUC)&pWirelessPkt[TmpBulkEndPos];
		pTxWI = (PTXWI_STRUC)&pWirelessPkt[TmpBulkEndPos + TXINFO_SIZE];

		if (pAd->bForcePrintTX == TRUE)
			DBGPRINT(RT_DEBUG_TRACE, ("RTUSBBulkOutDataPacket AMPDU = %d.\n",   pTxWI->AMPDU));

		
		
		if ((ThisBulkSize != 0) && (pTxWI->PHYMODE == MODE_CCK))
		{
			if (((ThisBulkSize&0xffff8000) != 0) || ((ThisBulkSize&0x1000) == 0x1000))
			{
				
				pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
				break;
			}
			else if (((pAd->BulkOutMaxPacketSize < 512) && ((ThisBulkSize&0xfffff800) != 0) ) )
			{
				
				
				pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
				break;
			}
		}
		
		else
		{
			if (((ThisBulkSize&0xffff8000) != 0) || ((ThisBulkSize&0x6000) == 0x6000))
			{	
				pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
				break;
			}
			else if (((pAd->BulkOutMaxPacketSize < 512) && ((ThisBulkSize&0xfffff800) != 0) ) )
			{	
				
				pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
				break;
			}
		}

		if (TmpBulkEndPos == pHTTXContext->CurWritePosition)
		{
			pHTTXContext->ENextBulkOutPosition = TmpBulkEndPos;
			break;
		}

		
		if (pTxInfo->QSEL != FIFO_EDCA)
		{
			printk("%s(): ====> pTxInfo->QueueSel(%d)!= FIFO_EDCA!!!!\n", __func__, pTxInfo->QSEL);
			printk("\tCWPos=%ld, NBPos=%ld, ENBPos=%ld, bCopy=%d!\n", pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition, pHTTXContext->bCopySavePad);
			hex_dump("Wrong QSel Pkt:", (PUCHAR)&pWirelessPkt[TmpBulkEndPos], (pHTTXContext->CurWritePosition - pHTTXContext->NextBulkOutPosition));
		}

		if (pTxInfo->USBDMATxPktLen <= 8)
		{
			BULK_OUT_UNLOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags2);
			DBGPRINT(RT_DEBUG_ERROR ,("e2, USBDMATxPktLen==0, Size=%ld, bCSPad=%d, CWPos=%ld, NBPos=%ld, CWRPos=%ld!\n",
					pHTTXContext->BulkOutSize, pHTTXContext->bCopySavePad, pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition, pHTTXContext->CurWriteRealPos));
			{
				DBGPRINT_RAW(RT_DEBUG_ERROR ,("%x  %x  %x  %x  %x  %x  %x  %x \n",
					pHTTXContext->SavedPad[0], pHTTXContext->SavedPad[1], pHTTXContext->SavedPad[2],pHTTXContext->SavedPad[3]
					,pHTTXContext->SavedPad[4], pHTTXContext->SavedPad[5], pHTTXContext->SavedPad[6],pHTTXContext->SavedPad[7]));
			}
			pAd->bForcePrintTX = TRUE;
			BULK_OUT_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
			pAd->BulkOutPending[BulkOutPipeId] = FALSE;
			BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
			
			return;
		}

			
		pAd->RalinkCounters.OneSecTransmittedByteCount +=  pTxWI->MPDUtotalByteCount;
		pAd->RalinkCounters.TransmittedByteCount +=  pTxWI->MPDUtotalByteCount;

		pLastTxInfo = pTxInfo;

		
		pTxInfo->QSEL = FIFO_EDCA;  
		ThisBulkSize += (pTxInfo->USBDMATxPktLen+4);
		TmpBulkEndPos += (pTxInfo->USBDMATxPktLen+4);

		if (TmpBulkEndPos != pHTTXContext->CurWritePosition)
			pTxInfo->USBDMANextVLD = 1;

		if (pTxInfo->SwUseLastRound == 1)
		{
			if (pHTTXContext->CurWritePosition == 8)
				pTxInfo->USBDMANextVLD = 0;
			pTxInfo->SwUseLastRound = 0;

			bTxQLastRound = TRUE;
			pHTTXContext->ENextBulkOutPosition = 8;

			break;
		}
	}while (TRUE);

	
	if (pLastTxInfo)
	{
		pLastTxInfo->USBDMANextVLD = 0;
	}

	
	if ((bTxQLastRound == FALSE) &&
		 (((pHTTXContext->ENextBulkOutPosition == pHTTXContext->CurWritePosition) && (pHTTXContext->CurWriteRealPos > pHTTXContext->CurWritePosition)) ||
		  (pHTTXContext->ENextBulkOutPosition != pHTTXContext->CurWritePosition))
		)
	{
		NdisMoveMemory(pHTTXContext->SavedPad, &pWirelessPkt[pHTTXContext->ENextBulkOutPosition], 8);
		pHTTXContext->bCopySavePad = TRUE;
		if (RTMPEqualMemory(pHTTXContext->SavedPad, allzero,4))
		{
			PUCHAR	pBuf = &pHTTXContext->SavedPad[0];
			DBGPRINT_RAW(RT_DEBUG_ERROR,("WARNING-Zero-3:%02x%02x%02x%02x%02x%02x%02x%02x,CWPos=%ld, CWRPos=%ld, bCW=%d, NBPos=%ld, TBPos=%ld, TBSize=%ld\n",
				pBuf[0], pBuf[1], pBuf[2],pBuf[3],pBuf[4], pBuf[5], pBuf[6],pBuf[7], pHTTXContext->CurWritePosition, pHTTXContext->CurWriteRealPos,
				pHTTXContext->bCurWriting, pHTTXContext->NextBulkOutPosition, TmpBulkEndPos, ThisBulkSize));

			pBuf = &pWirelessPkt[pHTTXContext->CurWritePosition];
			DBGPRINT_RAW(RT_DEBUG_ERROR,("\tCWPos=%02x%02x%02x%02x%02x%02x%02x%02x\n", pBuf[0], pBuf[1], pBuf[2],pBuf[3],pBuf[4], pBuf[5], pBuf[6],pBuf[7]));
		}
		
	}

	if (pAd->bForcePrintTX == TRUE)
		DBGPRINT(RT_DEBUG_TRACE,("BulkOut-A:Size=%ld, CWPos=%ld, NBPos=%ld, ENBPos=%ld, bCopy=%d!\n", ThisBulkSize, pHTTXContext->CurWritePosition, pHTTXContext->NextBulkOutPosition, pHTTXContext->ENextBulkOutPosition, pHTTXContext->bCopySavePad));
	

		
	pAppendant = &pWirelessPkt[TmpBulkEndPos];
	NdisZeroMemory(pAppendant, 8);
		ThisBulkSize += 4;
		pHTTXContext->LastOne = TRUE;
		if ((ThisBulkSize % pAd->BulkOutMaxPacketSize) == 0)
			ThisBulkSize += 4;
	pHTTXContext->BulkOutSize = ThisBulkSize;

	pAd->watchDogTxPendingCnt[BulkOutPipeId] = 1;
	BULK_OUT_UNLOCK(&pAd->TxContextQueueLock[BulkOutPipeId], IrqFlags2);

	
	RTUSBInitHTTxDesc(pAd, pHTTXContext, BulkOutPipeId, ThisBulkSize, (usb_complete_t)RTUSBBulkOutDataPacketComplete);

	pUrb = pHTTXContext->pUrb;
	if((ret = RTUSB_SUBMIT_URB(pUrb))!=0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("RTUSBBulkOutDataPacket: Submit Tx URB failed %d\n", ret));

		BULK_OUT_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
		pAd->BulkOutPending[BulkOutPipeId] = FALSE;
		pAd->watchDogTxPendingCnt[BulkOutPipeId] = 0;
		BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);

		return;
	}

	BULK_OUT_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
	pHTTXContext->IRPPending = TRUE;
	BULK_OUT_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
	pAd->BulkOutReq++;

}


VOID RTUSBBulkOutDataPacketComplete(purbb_t pUrb, struct pt_regs *pt_regs)
{
	PHT_TX_CONTEXT	pHTTXContext;
	PRTMP_ADAPTER	pAd;
	POS_COOKIE 		pObj;
	UCHAR			BulkOutPipeId;


	pHTTXContext	= (PHT_TX_CONTEXT)pUrb->context;
	pAd 			= pHTTXContext->pAd;
	pObj 			= (POS_COOKIE) pAd->OS_Cookie;

	
	BulkOutPipeId	= pHTTXContext->BulkOutPipeId;
	pAd->BulkOutDataOneSecCount++;

	switch (BulkOutPipeId)
	{
		case 0:
				pObj->ac0_dma_done_task.data = (unsigned long)pUrb;
				tasklet_hi_schedule(&pObj->ac0_dma_done_task);
				break;
		case 1:
				pObj->ac1_dma_done_task.data = (unsigned long)pUrb;
				tasklet_hi_schedule(&pObj->ac1_dma_done_task);
				break;
		case 2:
				pObj->ac2_dma_done_task.data = (unsigned long)pUrb;
				tasklet_hi_schedule(&pObj->ac2_dma_done_task);
				break;
		case 3:
				pObj->ac3_dma_done_task.data = (unsigned long)pUrb;
				tasklet_hi_schedule(&pObj->ac3_dma_done_task);
				break;
		case 4:
				pObj->hcca_dma_done_task.data = (unsigned long)pUrb;
				tasklet_hi_schedule(&pObj->hcca_dma_done_task);
				break;
	}
}



VOID	RTUSBBulkOutNullFrame(
	IN	PRTMP_ADAPTER	pAd)
{
	PTX_CONTEXT		pNullContext = &(pAd->NullContext);
	PURB			pUrb;
	int				ret = 0;
	unsigned long	IrqFlags;

	RTMP_IRQ_LOCK(&pAd->BulkOutLock[0], IrqFlags);
	if ((pAd->BulkOutPending[0] == TRUE) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_TX))
	{
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], IrqFlags);
		return;
	}
	pAd->BulkOutPending[0] = TRUE;
	pAd->watchDogTxPendingCnt[0] = 1;
	pNullContext->IRPPending = TRUE;
	RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], IrqFlags);

	
	pAd->RalinkCounters.TransmittedByteCount +=  pNullContext->BulkOutSize;


	
	RTUSB_CLEAR_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NULL);

	
	RTUSBInitTxDesc(pAd, pNullContext, 0, (usb_complete_t)RTUSBBulkOutNullFrameComplete);

	pUrb = pNullContext->pUrb;
	if((ret = RTUSB_SUBMIT_URB(pUrb))!=0)
	{
		RTMP_IRQ_LOCK(&pAd->BulkOutLock[0], IrqFlags);
		pAd->BulkOutPending[0] = FALSE;
		pAd->watchDogTxPendingCnt[0] = 0;
		pNullContext->IRPPending = FALSE;
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], IrqFlags);

		DBGPRINT(RT_DEBUG_ERROR, ("RTUSBBulkOutNullFrame: Submit Tx URB failed %d\n", ret));
		return;
	}

}


VOID RTUSBBulkOutNullFrameComplete(purbb_t pUrb, struct pt_regs *pt_regs)
{
	PRTMP_ADAPTER		pAd;
	PTX_CONTEXT			pNullContext;
	NTSTATUS			Status;
	POS_COOKIE			pObj;


	pNullContext	= (PTX_CONTEXT)pUrb->context;
	pAd 			= pNullContext->pAd;
	Status 			= pUrb->status;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	pObj->null_frame_complete_task.data = (unsigned long)pUrb;
	tasklet_hi_schedule(&pObj->null_frame_complete_task);
}


VOID	RTUSBBulkOutMLMEPacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Index)
{
	PTX_CONTEXT		pMLMEContext;
	PURB			pUrb;
	int				ret = 0;
	unsigned long	IrqFlags;

	pMLMEContext = (PTX_CONTEXT)pAd->MgmtRing.Cell[pAd->MgmtRing.TxDmaIdx].AllocVa;
	pUrb = pMLMEContext->pUrb;

	if ((pAd->MgmtRing.TxSwFreeIdx >= MGMT_RING_SIZE) ||
		(pMLMEContext->InUse == FALSE) ||
		(pMLMEContext->bWaitingBulkOut == FALSE))
	{


		
		RTUSB_CLEAR_BULK_FLAG(pAd, fRTUSB_BULK_OUT_MLME);

		return;
	}


	RTMP_IRQ_LOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);
	if ((pAd->BulkOutPending[MGMTPIPEIDX] == TRUE) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_TX))
	{
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);
		return;
	}

	pAd->BulkOutPending[MGMTPIPEIDX] = TRUE;
	pAd->watchDogTxPendingCnt[MGMTPIPEIDX] = 1;
	pMLMEContext->IRPPending = TRUE;
	pMLMEContext->bWaitingBulkOut = FALSE;
	RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);

	
	pAd->RalinkCounters.TransmittedByteCount +=  pMLMEContext->BulkOutSize;

	
	RTUSB_CLEAR_BULK_FLAG(pAd, fRTUSB_BULK_OUT_MLME);

	
	RTUSBInitTxDesc(pAd, pMLMEContext, MGMTPIPEIDX, (usb_complete_t)RTUSBBulkOutMLMEPacketComplete);

	
	pUrb->transfer_dma	= 0;
	pUrb->transfer_flags &= (~URB_NO_TRANSFER_DMA_MAP);

	pUrb = pMLMEContext->pUrb;
	if((ret = RTUSB_SUBMIT_URB(pUrb))!=0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("RTUSBBulkOutMLMEPacket: Submit MLME URB failed %d\n", ret));
		RTMP_IRQ_LOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);
		pAd->BulkOutPending[MGMTPIPEIDX] = FALSE;
		pAd->watchDogTxPendingCnt[MGMTPIPEIDX] = 0;
		pMLMEContext->IRPPending = FALSE;
		pMLMEContext->bWaitingBulkOut = TRUE;
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);

		return;
	}

	

}


VOID RTUSBBulkOutMLMEPacketComplete(purbb_t pUrb, struct pt_regs *pt_regs)
{
	PTX_CONTEXT			pMLMEContext;
	PRTMP_ADAPTER		pAd;
	NTSTATUS			Status;
	POS_COOKIE 			pObj;
	int					index;

	
	pMLMEContext	= (PTX_CONTEXT)pUrb->context;
	pAd 			= pMLMEContext->pAd;
	pObj 			= (POS_COOKIE)pAd->OS_Cookie;
	Status			= pUrb->status;
	index 			= pMLMEContext->SelfIdx;

	pObj->mgmt_dma_done_task.data = (unsigned long)pUrb;
	tasklet_hi_schedule(&pObj->mgmt_dma_done_task);
}



VOID	RTUSBBulkOutPsPoll(
	IN	PRTMP_ADAPTER	pAd)
{
	PTX_CONTEXT		pPsPollContext = &(pAd->PsPollContext);
	PURB			pUrb;
	int				ret = 0;
	unsigned long	IrqFlags;

	RTMP_IRQ_LOCK(&pAd->BulkOutLock[0], IrqFlags);
	if ((pAd->BulkOutPending[0] == TRUE) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_TX))
	{
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], IrqFlags);
		return;
	}
	pAd->BulkOutPending[0] = TRUE;
	pAd->watchDogTxPendingCnt[0] = 1;
	pPsPollContext->IRPPending = TRUE;
	RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], IrqFlags);


	
	RTUSB_CLEAR_BULK_FLAG(pAd, fRTUSB_BULK_OUT_PSPOLL);

	
	RTUSBInitTxDesc(pAd, pPsPollContext, MGMTPIPEIDX, (usb_complete_t)RTUSBBulkOutPsPollComplete);

	pUrb = pPsPollContext->pUrb;
	if((ret = RTUSB_SUBMIT_URB(pUrb))!=0)
	{
		RTMP_IRQ_LOCK(&pAd->BulkOutLock[0], IrqFlags);
		pAd->BulkOutPending[0] = FALSE;
		pAd->watchDogTxPendingCnt[0] = 0;
		pPsPollContext->IRPPending = FALSE;
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], IrqFlags);

		DBGPRINT(RT_DEBUG_ERROR, ("RTUSBBulkOutPsPoll: Submit Tx URB failed %d\n", ret));
		return;
	}

}


VOID RTUSBBulkOutPsPollComplete(purbb_t pUrb,struct pt_regs *pt_regs)
{
	PRTMP_ADAPTER		pAd;
	PTX_CONTEXT			pPsPollContext;
	NTSTATUS			Status;
	POS_COOKIE			pObj;


	pPsPollContext= (PTX_CONTEXT)pUrb->context;
	pAd = pPsPollContext->pAd;
	Status = pUrb->status;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	pObj->pspoll_frame_complete_task.data = (unsigned long)pUrb;
	tasklet_hi_schedule(&pObj->pspoll_frame_complete_task);
}

VOID DoBulkIn(IN RTMP_ADAPTER *pAd)
{
	PRX_CONTEXT		pRxContext;
	PURB			pUrb;
	int				ret = 0;
	unsigned long	IrqFlags;

	RTMP_IRQ_LOCK(&pAd->BulkInLock, IrqFlags);
	pRxContext = &(pAd->RxContext[pAd->NextRxBulkInIndex]);
	if ((pAd->PendingRx > 0) || (pRxContext->Readable == TRUE) || (pRxContext->InUse == TRUE))
	{
		RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);
		return;
	}
	pRxContext->InUse = TRUE;
	pRxContext->IRPPending = TRUE;
	pAd->PendingRx++;
	pAd->BulkInReq++;
	RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);

	
	NdisZeroMemory(pRxContext->TransferBuffer, pRxContext->BulkInOffset);
	RTUSBInitRxDesc(pAd, pRxContext);

	pUrb = pRxContext->pUrb;
	if ((ret = RTUSB_SUBMIT_URB(pUrb))!=0)
	{	

		RTMP_IRQ_LOCK(&pAd->BulkInLock, IrqFlags);
		pRxContext->InUse = FALSE;
		pRxContext->IRPPending = FALSE;
		pAd->PendingRx--;
		pAd->BulkInReq--;
		RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);
		DBGPRINT(RT_DEBUG_ERROR, ("RTUSBBulkReceive: Submit Rx URB failed %d\n", ret));
	}
	else
	{	
		ASSERT((pRxContext->InUse == pRxContext->IRPPending));
		
	}
}



#define fRTMP_ADAPTER_NEED_STOP_RX		\
		(fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_HALT_IN_PROGRESS |	\
		 fRTMP_ADAPTER_RADIO_OFF | fRTMP_ADAPTER_RESET_IN_PROGRESS | \
		 fRTMP_ADAPTER_REMOVE_IN_PROGRESS | fRTMP_ADAPTER_BULKIN_RESET)

#define fRTMP_ADAPTER_NEED_STOP_HANDLE_RX	\
		(fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_HALT_IN_PROGRESS |	\
		 fRTMP_ADAPTER_RADIO_OFF | fRTMP_ADAPTER_RESET_IN_PROGRESS | \
		 fRTMP_ADAPTER_REMOVE_IN_PROGRESS)

VOID	RTUSBBulkReceive(
	IN	PRTMP_ADAPTER	pAd)
{
	PRX_CONTEXT		pRxContext;
	unsigned long	IrqFlags;


	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_HANDLE_RX))
		return;

	while(1)
	{

		RTMP_IRQ_LOCK(&pAd->BulkInLock, IrqFlags);
		pRxContext = &(pAd->RxContext[pAd->NextRxBulkInReadIndex]);
		if (((pRxContext->InUse == FALSE) && (pRxContext->Readable == TRUE)) &&
			(pRxContext->bRxHandling == FALSE))
		{
			pRxContext->bRxHandling = TRUE;
			RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);

			
			STARxDoneInterruptHandle(pAd, TRUE);

			
			RTMP_IRQ_LOCK(&pAd->BulkInLock, IrqFlags);
			pRxContext->BulkInOffset = 0;
			pRxContext->Readable = FALSE;
			pRxContext->bRxHandling = FALSE;
			pAd->ReadPosition = 0;
			pAd->TransferBufferLength = 0;
			INC_RING_INDEX(pAd->NextRxBulkInReadIndex, RX_RING_SIZE);
			RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);

		}
		else
		{
			RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);
			break;
		}
	}

	if (!(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NEED_STOP_RX)))
		DoBulkIn(pAd);

}



VOID RTUSBBulkRxComplete(purbb_t pUrb, struct pt_regs *pt_regs)
{
	
	
	
	PRX_CONTEXT		pRxContext;
	PRTMP_ADAPTER	pAd;
	POS_COOKIE 		pObj;


	pRxContext	= (PRX_CONTEXT)pUrb->context;
	pAd 		= pRxContext->pAd;
	pObj 		= (POS_COOKIE) pAd->OS_Cookie;

	pObj->rx_done_task.data = (unsigned long)pUrb;
	tasklet_hi_schedule(&pObj->rx_done_task);

}




VOID	RTUSBKickBulkOut(
	IN	PRTMP_ADAPTER pAd)
{
	
	if (!RTMP_TEST_FLAG(pAd ,fRTMP_ADAPTER_NEED_STOP_TX)
		)
	{
		
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_PSPOLL))
		{
			RTUSBBulkOutPsPoll(pAd);
		}

		
		else if ((RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_MLME)) &&
				 (pAd->MgmtRing.TxSwFreeIdx < MGMT_RING_SIZE))
		{
			RTUSBBulkOutMLMEPacket(pAd, pAd->MgmtRing.TxDmaIdx);
		}

		
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL))
		{
			if (((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
				(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
				))
			{
				RTUSBBulkOutDataPacket(pAd, 0, pAd->NextBulkOutIndex[0]);
			}
		}
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL_2))
		{
			if (((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
				(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
				))
			{
				RTUSBBulkOutDataPacket(pAd, 1, pAd->NextBulkOutIndex[1]);
			}
		}
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL_3))
		{
			if (((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
				(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
				))
			{
				RTUSBBulkOutDataPacket(pAd, 2, pAd->NextBulkOutIndex[2]);
			}
		}
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL_4))
		{
			if (((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
				(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
				))
			{
				RTUSBBulkOutDataPacket(pAd, 3, pAd->NextBulkOutIndex[3]);
			}
		}

		
		if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL_5))
		{
			if (((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) ||
				(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
				))
			{
			}
		}

		
		else if (RTUSB_TEST_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NULL))
		{
			if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
			{
				RTUSBBulkOutNullFrame(pAd);
			}
		}

		
		else
		{

		}
	}
}


VOID	RTUSBCleanUpDataBulkOutQueue(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR			Idx;
	PHT_TX_CONTEXT	pTxContext;

	DBGPRINT(RT_DEBUG_TRACE, ("--->CleanUpDataBulkOutQueue\n"));

	for (Idx = 0; Idx < 4; Idx++)
	{
		pTxContext = &pAd->TxContext[Idx];

		pTxContext->CurWritePosition = pTxContext->NextBulkOutPosition;
		pTxContext->LastOne = FALSE;
		NdisAcquireSpinLock(&pAd->BulkOutLock[Idx]);
		pAd->BulkOutPending[Idx] = FALSE;
		NdisReleaseSpinLock(&pAd->BulkOutLock[Idx]);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<---CleanUpDataBulkOutQueue\n"));
}


VOID	RTUSBCleanUpMLMEBulkOutQueue(
	IN	PRTMP_ADAPTER	pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("--->CleanUpMLMEBulkOutQueue\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("<---CleanUpMLMEBulkOutQueue\n"));
}



VOID	RTUSBCancelPendingIRPs(
	IN	PRTMP_ADAPTER	pAd)
{
	RTUSBCancelPendingBulkInIRP(pAd);
	RTUSBCancelPendingBulkOutIRP(pAd);
}


VOID	RTUSBCancelPendingBulkInIRP(
	IN	PRTMP_ADAPTER	pAd)
{
	PRX_CONTEXT		pRxContext;
	UINT			i;

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("--->RTUSBCancelPendingBulkInIRP\n"));
	for ( i = 0; i < (RX_RING_SIZE); i++)
	{
		pRxContext = &(pAd->RxContext[i]);
		if(pRxContext->IRPPending == TRUE)
		{
			RTUSB_UNLINK_URB(pRxContext->pUrb);
			pRxContext->IRPPending = FALSE;
			pRxContext->InUse = FALSE;
			
			
		}
	}
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("<---RTUSBCancelPendingBulkInIRP\n"));
}



VOID	RTUSBCancelPendingBulkOutIRP(
	IN	PRTMP_ADAPTER	pAd)
{
	PHT_TX_CONTEXT		pHTTXContext;
	PTX_CONTEXT			pMLMEContext;
	PTX_CONTEXT			pBeaconContext;
	PTX_CONTEXT			pNullContext;
	PTX_CONTEXT			pPsPollContext;
	PTX_CONTEXT			pRTSContext;
	UINT				i, Idx;








	for (Idx = 0; Idx < 4; Idx++)
	{
		pHTTXContext = &(pAd->TxContext[Idx]);

		if (pHTTXContext->IRPPending == TRUE)
		{

			
			
			
			

			RTUSB_UNLINK_URB(pHTTXContext->pUrb);

			
			RTMPusecDelay(200);
		}

		pAd->BulkOutPending[Idx] = FALSE;
	}

	
	for (i = 0; i < MGMT_RING_SIZE; i++)
	{
		pMLMEContext = (PTX_CONTEXT)pAd->MgmtRing.Cell[i].AllocVa;
		if(pMLMEContext && (pMLMEContext->IRPPending == TRUE))
		{

			
			
			
			

			RTUSB_UNLINK_URB(pMLMEContext->pUrb);
			pMLMEContext->IRPPending = FALSE;

			
			RTMPusecDelay(200);
		}
	}
	pAd->BulkOutPending[MGMTPIPEIDX] = FALSE;
	


	for (i = 0; i < BEACON_RING_SIZE; i++)
	{
		pBeaconContext = &(pAd->BeaconContext[i]);

		if(pBeaconContext->IRPPending == TRUE)
		{

			
			
			
			

			RTUSB_UNLINK_URB(pBeaconContext->pUrb);

			
			RTMPusecDelay(200);
		}
	}

	pNullContext = &(pAd->NullContext);
	if (pNullContext->IRPPending == TRUE)
		RTUSB_UNLINK_URB(pNullContext->pUrb);

	pRTSContext = &(pAd->RTSContext);
	if (pRTSContext->IRPPending == TRUE)
		RTUSB_UNLINK_URB(pRTSContext->pUrb);

	pPsPollContext = &(pAd->PsPollContext);
	if (pPsPollContext->IRPPending == TRUE)
		RTUSB_UNLINK_URB(pPsPollContext->pUrb);

	for (Idx = 0; Idx < 4; Idx++)
	{
		NdisAcquireSpinLock(&pAd->BulkOutLock[Idx]);
		pAd->BulkOutPending[Idx] = FALSE;
		NdisReleaseSpinLock(&pAd->BulkOutLock[Idx]);
	}
}

