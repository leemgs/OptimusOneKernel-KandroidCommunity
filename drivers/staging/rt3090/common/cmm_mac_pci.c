

#ifdef RTMP_MAC_PCI

#include "../rt_config.h"



NDIS_STATUS	RTMPAllocTxRxRingMemory(
	IN	PRTMP_ADAPTER	pAd)
{
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
	ULONG			RingBasePaHigh;
	ULONG			RingBasePaLow;
	PVOID			RingBaseVa;
	INT				index, num;
	PTXD_STRUC		pTxD;
	PRXD_STRUC		pRxD;
	ULONG			ErrorValue = 0;
	PRTMP_TX_RING	pTxRing;
	PRTMP_DMABUF	pDmaBuf;
	PNDIS_PACKET	pPacket;


	DBGPRINT(RT_DEBUG_TRACE, ("--> RTMPAllocTxRxRingMemory\n"));
	do
	{
		
		
		
		
		
		for (num=0; num<NUM_OF_TX_RING; num++)
		{
			ULONG  BufBasePaHigh;
			ULONG  BufBasePaLow;
			PVOID  BufBaseVa;

			
			
			
			pAd->TxDescRing[num].AllocSize = TX_RING_SIZE * TXD_SIZE;
			RTMP_AllocateTxDescMemory(
				pAd,
				num,
				pAd->TxDescRing[num].AllocSize,
				FALSE,
				&pAd->TxDescRing[num].AllocVa,
				&pAd->TxDescRing[num].AllocPa);

			if (pAd->TxDescRing[num].AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			
			NdisZeroMemory(pAd->TxDescRing[num].AllocVa, pAd->TxDescRing[num].AllocSize);

			
			RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->TxDescRing[num].AllocPa);
			RingBasePaLow  = RTMP_GetPhysicalAddressLow (pAd->TxDescRing[num].AllocPa);
			RingBaseVa     = pAd->TxDescRing[num].AllocVa;

			
			
			
			pAd->TxBufSpace[num].AllocSize = TX_RING_SIZE * TX_DMA_1ST_BUFFER_SIZE;
			RTMP_AllocateFirstTxBuffer(
				pAd,
				num,
				pAd->TxBufSpace[num].AllocSize,
				FALSE,
				&pAd->TxBufSpace[num].AllocVa,
				&pAd->TxBufSpace[num].AllocPa);

			if (pAd->TxBufSpace[num].AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			
			NdisZeroMemory(pAd->TxBufSpace[num].AllocVa, pAd->TxBufSpace[num].AllocSize);

			
			BufBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->TxBufSpace[num].AllocPa);
			BufBasePaLow  = RTMP_GetPhysicalAddressLow (pAd->TxBufSpace[num].AllocPa);
			BufBaseVa     = pAd->TxBufSpace[num].AllocVa;

			
			
			
			pTxRing = &pAd->TxRing[num];
			for (index = 0; index < TX_RING_SIZE; index++)
			{
				pTxRing->Cell[index].pNdisPacket = NULL;
				pTxRing->Cell[index].pNextNdisPacket = NULL;
				
				pTxRing->Cell[index].AllocSize = TXD_SIZE;
				pTxRing->Cell[index].AllocVa = RingBaseVa;
				RTMP_SetPhysicalAddressHigh(pTxRing->Cell[index].AllocPa, RingBasePaHigh);
				RTMP_SetPhysicalAddressLow (pTxRing->Cell[index].AllocPa, RingBasePaLow);

				
				pDmaBuf = &pTxRing->Cell[index].DmaBuf;
				pDmaBuf->AllocSize = TX_DMA_1ST_BUFFER_SIZE;
				pDmaBuf->AllocVa = BufBaseVa;
				RTMP_SetPhysicalAddressHigh(pDmaBuf->AllocPa, BufBasePaHigh);
				RTMP_SetPhysicalAddressLow(pDmaBuf->AllocPa, BufBasePaLow);

				
				pTxD = (PTXD_STRUC) pTxRing->Cell[index].AllocVa;
				pTxD->SDPtr0 = BufBasePaLow;
				
				pTxD->DMADONE = 1;
#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
				RingBasePaLow += TXD_SIZE;
				RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

				
				BufBasePaLow += TX_DMA_1ST_BUFFER_SIZE;
				BufBaseVa = (PUCHAR) BufBaseVa + TX_DMA_1ST_BUFFER_SIZE;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("TxRing[%d]: total %d entry allocated\n", num, index));
		}
		if (Status == NDIS_STATUS_RESOURCES)
			break;

		
		
		
		pAd->MgmtDescRing.AllocSize = MGMT_RING_SIZE * TXD_SIZE;
		RTMP_AllocateMgmtDescMemory(
			pAd,
			pAd->MgmtDescRing.AllocSize,
			FALSE,
			&pAd->MgmtDescRing.AllocVa,
			&pAd->MgmtDescRing.AllocPa);

		if (pAd->MgmtDescRing.AllocVa == NULL)
		{
			ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
			DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		
		NdisZeroMemory(pAd->MgmtDescRing.AllocVa, pAd->MgmtDescRing.AllocSize);

		
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->MgmtDescRing.AllocPa);
		RingBasePaLow  = RTMP_GetPhysicalAddressLow (pAd->MgmtDescRing.AllocPa);
		RingBaseVa     = pAd->MgmtDescRing.AllocVa;

		
		
		
		for (index = 0; index < MGMT_RING_SIZE; index++)
		{
			pAd->MgmtRing.Cell[index].pNdisPacket = NULL;
			pAd->MgmtRing.Cell[index].pNextNdisPacket = NULL;
			
			pAd->MgmtRing.Cell[index].AllocSize = TXD_SIZE;
			pAd->MgmtRing.Cell[index].AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(pAd->MgmtRing.Cell[index].AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow (pAd->MgmtRing.Cell[index].AllocPa, RingBasePaLow);

			
			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

			
			pTxD = (PTXD_STRUC) pAd->MgmtRing.Cell[index].AllocVa;
			pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
			
		}
		DBGPRINT(RT_DEBUG_TRACE, ("MGMT Ring: total %d entry allocated\n", index));

		
		
		
		pAd->RxDescRing.AllocSize = RX_RING_SIZE * RXD_SIZE;
		RTMP_AllocateRxDescMemory(
			pAd,
			pAd->RxDescRing.AllocSize,
			FALSE,
			&pAd->RxDescRing.AllocVa,
			&pAd->RxDescRing.AllocPa);

		if (pAd->RxDescRing.AllocVa == NULL)
		{
			ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
			DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		
		NdisZeroMemory(pAd->RxDescRing.AllocVa, pAd->RxDescRing.AllocSize);


		DBGPRINT(RT_DEBUG_OFF,
					("RX DESC %p  size = %ld\n", pAd->RxDescRing.AllocVa, pAd->RxDescRing.AllocSize));

		
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->RxDescRing.AllocPa);
		RingBasePaLow  = RTMP_GetPhysicalAddressLow (pAd->RxDescRing.AllocPa);
		RingBaseVa     = pAd->RxDescRing.AllocVa;

		
		
		
		for (index = 0; index < RX_RING_SIZE; index++)
		{
			
			pAd->RxRing.Cell[index].AllocSize = RXD_SIZE;
			pAd->RxRing.Cell[index].AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(pAd->RxRing.Cell[index].AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow (pAd->RxRing.Cell[index].AllocPa, RingBasePaLow);

			

			
			RingBasePaLow += RXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + RXD_SIZE;

			
			pDmaBuf = &pAd->RxRing.Cell[index].DmaBuf;
			pDmaBuf->AllocSize = RX_BUFFER_AGGRESIZE;
			pPacket = RTMP_AllocateRxPacketBuffer(
				pAd,
				pDmaBuf->AllocSize,
				FALSE,
				&pDmaBuf->AllocVa,
				&pDmaBuf->AllocPa);

			
			pAd->RxRing.Cell[index].pNdisPacket = pPacket;

			
			if (pDmaBuf->AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate RxRing's 1st buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			
			NdisZeroMemory(pDmaBuf->AllocVa, pDmaBuf->AllocSize);

			
			pRxD = (PRXD_STRUC) pAd->RxRing.Cell[index].AllocVa;
			pRxD->SDP0 = RTMP_GetPhysicalAddressLow(pDmaBuf->AllocPa);
			pRxD->DDONE = 0;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#endif
		}

		DBGPRINT(RT_DEBUG_TRACE, ("Rx Ring: total %d entry allocated\n", index));

	}	while (FALSE);


	NdisZeroMemory(&pAd->FragFrame, sizeof(FRAGMENT_FRAME));
	pAd->FragFrame.pFragPacket =  RTMP_AllocateFragPacketBuffer(pAd, RX_BUFFER_NORMSIZE);

	if (pAd->FragFrame.pFragPacket == NULL)
	{
		Status = NDIS_STATUS_RESOURCES;
	}

	if (Status != NDIS_STATUS_SUCCESS)
	{
		
		NdisWriteErrorLogEntry(
			pAd->AdapterHandle,
			NDIS_ERROR_CODE_OUT_OF_RESOURCES,
			1,
			ErrorValue);
	}

	
	{
		DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitTxRxRingAndBacklogQueue\n"));



		
		for(index = 0; index < NUM_OF_TX_RING; index++)
		{
			InitializeQueueHeader(&pAd->TxSwQueue[index]);
			
			pAd->TxRing[index].TxSwFreeIdx = 0;
			pAd->TxRing[index].TxCpuIdx = 0;
			
		}

		
		pAd->RxRing.RxSwReadIdx = 0;
		pAd->RxRing.RxCpuIdx = RX_RING_SIZE - 1;
		


		
		pAd->MgmtRing.TxSwFreeIdx = 0;
		pAd->MgmtRing.TxCpuIdx = 0;

		pAd->PrivateInfo.TxRingFullCnt = 0;

		DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitTxRxRingAndBacklogQueue\n"));
	}

	DBGPRINT_S(Status, ("<-- RTMPAllocTxRxRingMemory, Status=%x\n", Status));
	return Status;
}





VOID	RTMPRingCleanUp(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			RingType)
{
	PTXD_STRUC		pTxD;
	PRXD_STRUC		pRxD;
	PQUEUE_ENTRY	pEntry;
	PNDIS_PACKET	pPacket;
	int				i;
	PRTMP_TX_RING	pTxRing;
	unsigned long	IrqFlags;
	


	DBGPRINT(RT_DEBUG_TRACE,("RTMPRingCleanUp(RingIdx=%d, Pending-NDIS=%ld)\n", RingType, pAd->RalinkCounters.PendingNdisPacketCount));
	switch (RingType)
	{
		case QID_AC_BK:
		case QID_AC_BE:
		case QID_AC_VI:
		case QID_AC_VO:
		

			pTxRing = &pAd->TxRing[RingType];

			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			
			for (i=0; i<TX_RING_SIZE; i++) 
			{
				pTxD  = (PTXD_STRUC) pTxRing->Cell[i].AllocVa;

				pPacket = (PNDIS_PACKET) pTxRing->Cell[i].pNdisPacket;
				
				if (pPacket)
				{
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					pTxRing->Cell[i].pNdisPacket = NULL;
				}

				pPacket = (PNDIS_PACKET) pTxRing->Cell[i].pNextNdisPacket;
				
				if (pPacket)
				{
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					pTxRing->Cell[i].pNextNdisPacket = NULL;
				}
			}

			RTMP_IO_READ32(pAd, TX_DTX_IDX0 + RingType * 0x10, &pTxRing->TxDmaIdx);
			pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
			pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
			RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + RingType * 0x10, pTxRing->TxCpuIdx);

			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			while (pAd->TxSwQueue[RingType].Head != NULL)
			{
				pEntry = RemoveHeadQueue(&pAd->TxSwQueue[RingType]);
				pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				DBGPRINT(RT_DEBUG_TRACE,("Release 1 NDIS packet from s/w backlog queue\n"));
			}
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
			break;

		case QID_MGMT:
			
			NdisAcquireSpinLock(&pAd->MgmtRingLock);

			for (i=0; i<MGMT_RING_SIZE; i++)
			{
				pTxD  = (PTXD_STRUC) pAd->MgmtRing.Cell[i].AllocVa;

				pPacket = (PNDIS_PACKET) pAd->MgmtRing.Cell[i].pNdisPacket;
				
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				pAd->MgmtRing.Cell[i].pNdisPacket = NULL;

				pPacket = (PNDIS_PACKET) pAd->MgmtRing.Cell[i].pNextNdisPacket;
				
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			}
				pAd->MgmtRing.Cell[i].pNextNdisPacket = NULL;

			}

			RTMP_IO_READ32(pAd, TX_MGMTDTX_IDX, &pAd->MgmtRing.TxDmaIdx);
			pAd->MgmtRing.TxSwFreeIdx = pAd->MgmtRing.TxDmaIdx;
			pAd->MgmtRing.TxCpuIdx = pAd->MgmtRing.TxDmaIdx;
			RTMP_IO_WRITE32(pAd, TX_MGMTCTX_IDX, pAd->MgmtRing.TxCpuIdx);

			NdisReleaseSpinLock(&pAd->MgmtRingLock);
			pAd->RalinkCounters.MgmtRingFullCount = 0;
			break;

		case QID_RX:
			
			NdisAcquireSpinLock(&pAd->RxRingLock);

			for (i=0; i<RX_RING_SIZE; i++)
			{
				pRxD  = (PRXD_STRUC) pAd->RxRing.Cell[i].AllocVa;
                pRxD->DDONE = 0 ;
			}

			RTMP_IO_READ32(pAd, RX_DRX_IDX, &pAd->RxRing.RxDmaIdx);
			pAd->RxRing.RxSwReadIdx = pAd->RxRing.RxDmaIdx;
			pAd->RxRing.RxCpuIdx = ((pAd->RxRing.RxDmaIdx == 0) ? (RX_RING_SIZE-1) : (pAd->RxRing.RxDmaIdx-1));
			RTMP_IO_WRITE32(pAd, RX_CRX_IDX, pAd->RxRing.RxCpuIdx);

			NdisReleaseSpinLock(&pAd->RxRingLock);
			break;

		default:
			break;
	}
}


VOID RTMPFreeTxRxRingMemory(
    IN  PRTMP_ADAPTER   pAd)
{
	int index, num , j;
	PRTMP_TX_RING pTxRing;
	PTXD_STRUC	  pTxD;
	PNDIS_PACKET  pPacket;
	unsigned int  IrqFlags;

	

	DBGPRINT(RT_DEBUG_TRACE, ("--> RTMPFreeTxRxRingMemory\n"));

	
	for (index=0; index <NUM_OF_TX_RING; index++)
	{
		PQUEUE_ENTRY pEntry;
		PNDIS_PACKET pPacket;
		PQUEUE_HEADER   pQueue;

		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
		pQueue = &pAd->TxSwQueue[index];
		while (pQueue->Head)
		{
			pEntry = RemoveHeadQueue(pQueue);
			pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		}
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
	}

	
	for (index=0;index< NUM_OF_TX_RING;index++)
	{
		pTxRing = &pAd->TxRing[index];

		for (j=0; j< TX_RING_SIZE; j++)
		{
			pTxD = (PTXD_STRUC) (pTxRing->Cell[j].AllocVa);
			pPacket = pTxRing->Cell[j].pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			
			pTxRing->Cell[j].pNdisPacket = NULL;

			pPacket = pTxRing->Cell[j].pNextNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			
			pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket = NULL;

		}
	}

	for (index = RX_RING_SIZE - 1 ; index >= 0; index--)
	{
		if ((pAd->RxRing.Cell[index].DmaBuf.AllocVa) && (pAd->RxRing.Cell[index].pNdisPacket))
		{
			PCI_UNMAP_SINGLE(pAd, pAd->RxRing.Cell[index].DmaBuf.AllocPa, pAd->RxRing.Cell[index].DmaBuf.AllocSize, PCI_DMA_FROMDEVICE);
			RELEASE_NDIS_PACKET(pAd, pAd->RxRing.Cell[index].pNdisPacket, NDIS_STATUS_SUCCESS);
		}
	}
	NdisZeroMemory(pAd->RxRing.Cell, RX_RING_SIZE * sizeof(RTMP_DMACB));

	if (pAd->RxDescRing.AllocVa)
    {
		RTMP_FreeDescMemory(pAd, pAd->RxDescRing.AllocSize, pAd->RxDescRing.AllocVa, pAd->RxDescRing.AllocPa);
    }
    NdisZeroMemory(&pAd->RxDescRing, sizeof(RTMP_DMABUF));

	if (pAd->MgmtDescRing.AllocVa)
	{
		RTMP_FreeDescMemory(pAd, pAd->MgmtDescRing.AllocSize, pAd->MgmtDescRing.AllocVa, pAd->MgmtDescRing.AllocPa);
	}
	NdisZeroMemory(&pAd->MgmtDescRing, sizeof(RTMP_DMABUF));

	for (num = 0; num < NUM_OF_TX_RING; num++)
	{
	if (pAd->TxBufSpace[num].AllocVa)
		{
			RTMP_FreeFirstTxBuffer(pAd, pAd->TxBufSpace[num].AllocSize, FALSE, pAd->TxBufSpace[num].AllocVa, pAd->TxBufSpace[num].AllocPa);
	    }
	    NdisZeroMemory(&pAd->TxBufSpace[num], sizeof(RTMP_DMABUF));

	if (pAd->TxDescRing[num].AllocVa)
		{
			RTMP_FreeDescMemory(pAd, pAd->TxDescRing[num].AllocSize, pAd->TxDescRing[num].AllocVa, pAd->TxDescRing[num].AllocPa);
	    }
	    NdisZeroMemory(&pAd->TxDescRing[num], sizeof(RTMP_DMABUF));
	}

	if (pAd->FragFrame.pFragPacket)
		RELEASE_NDIS_PACKET(pAd, pAd->FragFrame.pFragPacket, NDIS_STATUS_SUCCESS);

	DBGPRINT(RT_DEBUG_TRACE, ("<-- RTMPFreeTxRxRingMemory\n"));
}




VOID RT28XXDMADisable(
	IN RTMP_ADAPTER			*pAd)
{
	WPDMA_GLO_CFG_STRUC     GloCfg;


	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
	GloCfg.word &= 0xff0;
	GloCfg.field.EnTXWriteBackDDONE =1;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);
}



VOID RT28XXDMAEnable(
	IN RTMP_ADAPTER			*pAd)
{
	WPDMA_GLO_CFG_STRUC	GloCfg;
	int i = 0;

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x4);
	do
	{
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0)  && (GloCfg.field.RxDMABusy == 0))
			break;

		DBGPRINT(RT_DEBUG_TRACE, ("==>  DMABusy\n"));
		RTMPusecDelay(1000);
		i++;
	}while ( i <200);

	RTMPusecDelay(50);

	GloCfg.field.EnTXWriteBackDDONE = 1;
	GloCfg.field.WPDMABurstSIZE = 2;
	GloCfg.field.EnableRxDMA = 1;
	GloCfg.field.EnableTxDMA = 1;

	DBGPRINT(RT_DEBUG_TRACE, ("<== WRITE DMA offset 0x208 = 0x%x\n", GloCfg.word));
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);

}


BOOLEAN AsicCheckCommanOk(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Command)
{
	UINT32	CmdStatus = 0, CID = 0, i;
	UINT32	ThisCIDMask = 0;

	i = 0;
	do
	{
		RTMP_IO_READ32(pAd, H2M_MAILBOX_CID, &CID);
		
		if ((CID & CID0MASK) == Command)
		{
			ThisCIDMask = CID0MASK;
			break;
		}
		else if ((((CID & CID1MASK)>>8) & 0xff) == Command)
		{
			ThisCIDMask = CID1MASK;
			break;
		}
		else if ((((CID & CID2MASK)>>16) & 0xff) == Command)
		{
			ThisCIDMask = CID2MASK;
			break;
		}
		else if ((((CID & CID3MASK)>>24) & 0xff) == Command)
		{
			ThisCIDMask = CID3MASK;
			break;
		}

		RTMPusecDelay(100);
		i++;
	}while (i < 200);

	
	RTMP_IO_READ32(pAd, H2M_MAILBOX_STATUS, &CmdStatus);

	
	if (i < 200)
	{
		
		if (((CmdStatus & ThisCIDMask) == 0x1) || ((CmdStatus & ThisCIDMask) == 0x100)
			|| ((CmdStatus & ThisCIDMask) == 0x10000) || ((CmdStatus & ThisCIDMask) == 0x1000000))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("--> AsicCheckCommanOk CID = 0x%x, CmdStatus= 0x%x \n", CID, CmdStatus));
			RTMP_IO_WRITE32(pAd, H2M_MAILBOX_STATUS, 0xffffffff);
			RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CID, 0xffffffff);
			return TRUE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("--> AsicCheckCommanFail1 CID = 0x%x, CmdStatus= 0x%x \n", CID, CmdStatus));
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("--> AsicCheckCommanFail2 Timeout Command = %d, CmdStatus= 0x%x \n", Command, CmdStatus));
	}
	
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_STATUS, 0xffffffff);
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CID, 0xffffffff);

	return FALSE;
}



VOID RT28xx_UpdateBeaconToAsic(
	IN RTMP_ADAPTER		*pAd,
	IN INT				apidx,
	IN ULONG			FrameLen,
	IN ULONG			UpdatePos)
{
	ULONG				CapInfoPos = 0;
	UCHAR			*ptr, *ptr_update, *ptr_capinfo;
	UINT			i;
	BOOLEAN			bBcnReq = FALSE;
	UCHAR			bcn_idx = 0;


	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s() : No valid Interface be found.\n", __FUNCTION__));
		return;
	}

	
	
	
	
	if (bBcnReq == FALSE)
	{
		
		
		for(i=0; i<TXWI_SIZE; i+=4)
			RTMP_IO_WRITE32(pAd, pAd->BeaconOffset[bcn_idx] + i, 0x00);
	}
	else
	{
		ptr = (PUCHAR)&pAd->BeaconTxWI;
#ifdef RT_BIG_ENDIAN
		RTMPWIEndianChange(ptr, TYPE_TXWI);
#endif
		for (i=0; i<TXWI_SIZE; i+=4)  
		{
			UINT32 longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
			RTMP_IO_WRITE32(pAd, pAd->BeaconOffset[bcn_idx] + i, longptr);
			ptr += 4;
		}

		
		for (i = CapInfoPos; i < (CapInfoPos+2); i++)
		{
			RTMP_IO_WRITE8(pAd, pAd->BeaconOffset[bcn_idx] + TXWI_SIZE + i, *ptr_capinfo);
			ptr_capinfo ++;
		}

		if (FrameLen > UpdatePos)
		{
			for (i= UpdatePos; i< (FrameLen); i++)
			{
				RTMP_IO_WRITE8(pAd, pAd->BeaconOffset[bcn_idx] + TXWI_SIZE + i, *ptr_update);
				ptr_update ++;
			}
		}

	}

}


#ifdef CONFIG_STA_SUPPORT
VOID RT28xxPciStaAsicForceWakeup(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN       bFromTx)
{
    AUTO_WAKEUP_STRUC	AutoWakeupCfg;

    if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
        return;

    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WAKEUP_NOW))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("waking up now!\n"));
        return;
    }

    OPSTATUS_SET_FLAG(pAd, fOP_STATUS_WAKEUP_NOW);

    RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_GO_TO_SLEEP_NOW);

    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE)
		&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
    {
        
	if (bFromTx == TRUE
			&&(pAd->Mlme.bPsPollTimerRunning == TRUE))
	{
            pAd->Mlme.bPsPollTimerRunning = FALSE;
		RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_WAKEUP);
		RTMPusecDelay(3000);
            DBGPRINT(RT_DEBUG_TRACE, ("=======AsicForceWakeup===bFromTx\n"));
	}

		AutoWakeupCfg.word = 0;
		RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);

        if (RT28xxPciAsicRadioOn(pAd, DOT11POWERSAVE))
        {
#ifdef PCIE_PS_SUPPORT
			
			if ((IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd)) && IS_VERSION_AFTER_F(pAd))
			{
				RTMP_CHIP_OP *pChipOps = &pAd->chipOps;

				if (pChipOps->AsicReverseRfFromSleepMode)
					pChipOps->AsicReverseRfFromSleepMode(pAd);
			}
			else
#endif 
			{
			
				
				if (INFRA_ON(pAd) && (pAd->CommonCfg.CentralChannel != pAd->CommonCfg.Channel)
					&& (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
				{
					
					AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
					AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
				}
				else
				{
					
					AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
					AsicLockChannel(pAd, pAd->CommonCfg.Channel);
				}
			}
        }
#ifdef PCIE_PS_SUPPORT
		
		
		if (((IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd)) && IS_VERSION_AFTER_F(pAd)) && IS_VERSION_AFTER_F(pAd)
			&& (pAd->StaCfg.PSControl.field.rt30xxPowerMode == 3)
			&& (pAd->StaCfg.PSControl.field.EnableNewPS == TRUE))
			{
			DBGPRINT(RT_DEBUG_TRACE, ("<==RT28xxPciStaAsicForceWakeup::Release the MCU Lock(3090)\n"));
			RTMP_SEM_LOCK(&pAd->McuCmdLock);
			pAd->brt30xxBanMcuCmd = FALSE;
			RTMP_SEM_UNLOCK(&pAd->McuCmdLock);
			}
#endif 
    }
    else
    {
        
         DBGPRINT(RT_DEBUG_TRACE, ("<==RT28xxPciStaAsicForceWakeup::Original PCI Power Saving\n"));
        AsicSendCommandToMcu(pAd, 0x31, 0xff, 0x00, 0x02);
        AutoWakeupCfg.word = 0;
	    RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);
    }

    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WAKEUP_NOW);
    DBGPRINT(RT_DEBUG_TRACE, ("<=======RT28xxPciStaAsicForceWakeup\n"));
}


VOID RT28xxPciStaAsicSleepThenAutoWakeup(
	IN PRTMP_ADAPTER pAd,
	IN USHORT TbttNumToNextWakeUp)
{
	BOOLEAN brc;

	if (pAd->StaCfg.bRadio == FALSE)
	{
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
		return;
	}
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE)
		&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
	{
		ULONG	Now = 0;
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WAKEUP_NOW))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("waking up now!\n"));
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
			return;
		}

		NdisGetSystemUpTime(&Now);
		
		
		if (((pAd->Mlme.LastSendNULLpsmTime + 8) >= Now) && (pAd->Mlme.LastSendNULLpsmTime <= Now))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Now = %lu, LastSendNULLpsmTime=%lu :  RxCountSinceLastNULL = %lu. \n", Now, pAd->Mlme.LastSendNULLpsmTime, pAd->RalinkCounters.RxCountSinceLastNULL));
			return;
		}
		else if ((pAd->RalinkCounters.RxCountSinceLastNULL > 0) && ((pAd->Mlme.LastSendNULLpsmTime + pAd->CommonCfg.BeaconPeriod) >= Now))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Now = %lu, LastSendNULLpsmTime=%lu: RxCountSinceLastNULL = %lu > 0 \n", Now, pAd->Mlme.LastSendNULLpsmTime,  pAd->RalinkCounters.RxCountSinceLastNULL));
			return;
		}

		brc = RT28xxPciAsicRadioOff(pAd, DOT11POWERSAVE, TbttNumToNextWakeUp);
		if (brc==TRUE)
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_DOZE);
	}
	else
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
		AsicSendCommandToMcu(pAd, 0x30, 0xff, 0xff, 0x00);   
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_DOZE);
		DBGPRINT(RT_DEBUG_TRACE, ("<-- %s, TbttNumToNextWakeUp=%d \n", __FUNCTION__, TbttNumToNextWakeUp));
	}

}


VOID PsPollWakeExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	unsigned long flags;

    DBGPRINT(RT_DEBUG_TRACE,("-->PsPollWakeExec \n"));

	RTMP_INT_LOCK(&pAd->irq_lock, flags);
    if (pAd->Mlme.bPsPollTimerRunning)
    {
	    RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_WAKEUP);
    }
    pAd->Mlme.bPsPollTimerRunning = FALSE;
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
#ifdef PCIE_PS_SUPPORT
	
	
	if (((IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd)) && IS_VERSION_AFTER_F(pAd))
	&& (pAd->StaCfg.PSControl.field.rt30xxPowerMode == 3)
	&& (pAd->StaCfg.PSControl.field.EnableNewPS == TRUE))
	{
		DBGPRINT(RT_DEBUG_TRACE,("<--PsPollWakeExec::3090 calls AsicForceWakeup(pAd, DOT11POWERSAVE) in advance \n"));
		AsicForceWakeup(pAd, DOT11POWERSAVE);
	}

#endif 
}

VOID  RadioOnExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	WPDMA_GLO_CFG_STRUC	DmaCfg;
	BOOLEAN				Cancelled;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
	{
		DBGPRINT(RT_DEBUG_TRACE,("-->RadioOnExec() return on fOP_STATUS_DOZE == TRUE; \n"));

#ifdef RTMP_PCI_SUPPORT
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE)
			&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
		RTMPSetTimer(&pAd->Mlme.RadioOnOffTimer, 10);
#endif 
		return;
	}

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
		DBGPRINT(RT_DEBUG_TRACE,("-->RadioOnExec() return on SCAN_IN_PROGRESS; \n"));


#ifdef RTMP_PCI_SUPPORT
if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE)
	&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
		RTMPSetTimer(&pAd->Mlme.RadioOnOffTimer, 10);
#endif 
		return;
	}

#ifdef RTMP_PCI_SUPPORT
if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE)
	&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
	{
	pAd->Mlme.bPsPollTimerRunning = FALSE;
	RTMPCancelTimer(&pAd->Mlme.PsPollTimer,	&Cancelled);
	}
#endif 
	if (pAd->StaCfg.bRadio == TRUE)
	{
		pAd->bPCIclkOff = FALSE;
		RTMPRingCleanUp(pAd, QID_AC_BK);
		RTMPRingCleanUp(pAd, QID_AC_BE);
		RTMPRingCleanUp(pAd, QID_AC_VI);
		RTMPRingCleanUp(pAd, QID_AC_VO);
		
		RTMPRingCleanUp(pAd, QID_MGMT);
		RTMPRingCleanUp(pAd, QID_RX);

		
		AsicSendCommandToMcu(pAd, 0x31, PowerWakeCID, 0x00, 0x02);
		
		AsicCheckCommanOk(pAd, PowerWakeCID);

		
		
		RTMP_ASIC_INTERRUPT_ENABLE(pAd);

		
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &DmaCfg.word);
		DmaCfg.field.EnableTxDMA = 1;
		RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, DmaCfg.word);

		
		if (INFRA_ON(pAd) && (pAd->CommonCfg.CentralChannel != pAd->CommonCfg.Channel)
			&& (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
		{
			
			AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
			AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
		}
		else
		{
			
			AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
			AsicLockChannel(pAd, pAd->CommonCfg.Channel);
		}

		if (pChipOps->AsicReverseRfFromSleepMode)
			pChipOps->AsicReverseRfFromSleepMode(pAd);
#ifdef PCIE_PS_SUPPORT
#ifdef CONFIG_STA_SUPPORT


if ((IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd)) && IS_VERSION_AFTER_F(pAd)
	&& (pAd->StaCfg.PSControl.field.rt30xxPowerMode == 3)
	&& (pAd->StaCfg.PSControl.field.EnableNewPS == TRUE))
	{
	RTMP_SEM_LOCK(&pAd->McuCmdLock);
	pAd->brt30xxBanMcuCmd = FALSE;
	RTMP_SEM_UNLOCK(&pAd->McuCmdLock);
	}
#endif 
#endif 
		
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

		
		RTMPSetLED(pAd, LED_RADIO_ON);

        if (pAd->StaCfg.Psm == PWR_ACTIVE)
        {
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, pAd->StaCfg.BBPR3);
        }
	}
	else
	{
		RT28xxPciAsicRadioOff(pAd, GUIRADIO_OFF, 0);
	}
}
#endif 



BOOLEAN RT28xxPciAsicRadioOn(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR     Level)
{
    
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN				Cancelled;
#endif 
    

	if (pAd->OpMode == OPMODE_AP && Level==DOT11POWERSAVE)
		return FALSE;

#ifdef CONFIG_STA_SUPPORT
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE))
	{
		if (pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
		{
	    pAd->Mlme.bPsPollTimerRunning = FALSE;
		RTMPCancelTimer(&pAd->Mlme.PsPollTimer,	&Cancelled);
		}
		if ((pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)&&
		((Level == GUIRADIO_OFF) || (Level == GUI_IDLE_POWER_SAVE))
		||(RTMP_TEST_PSFLAG(pAd, fRTMP_PS_SET_PCI_CLK_OFF_COMMAND)))
		{
			
			
			
		{
			DBGPRINT(RT_DEBUG_TRACE, ("RT28xxPciAsicRadioOn ()\n"));
			
			RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_WAKEUP);
			RTMPusecDelay(6000);
		}
	}
	}

#ifdef PCIE_PS_SUPPORT
if (!(((IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd)) && IS_VERSION_AFTER_F(pAd)
	&& (pAd->StaCfg.PSControl.field.rt30xxPowerMode == 3)
	&& (pAd->StaCfg.PSControl.field.EnableNewPS == TRUE))))
#endif 
	{
    pAd->bPCIclkOff = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("PSM :309xbPCIclkOff == %d\n", pAd->bPCIclkOff));

	}
#endif 
	
	AsicSendCommandToMcu(pAd, 0x31, PowerWakeCID, 0x00, 0x02);
	pAd->bPCIclkOff = FALSE;
	
	AsicCheckCommanOk(pAd, PowerWakeCID);
	RTMP_ASIC_INTERRUPT_ENABLE(pAd);


	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
	if (Level == GUI_IDLE_POWER_SAVE)
	{
#ifdef  PCIE_PS_SUPPORT

			
			if ((IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd)))
			{
				RTMP_CHIP_OP *pChipOps = &pAd->chipOps;

				if (pChipOps->AsicReverseRfFromSleepMode)
					pChipOps->AsicReverseRfFromSleepMode(pAd);
#ifdef CONFIG_STA_SUPPORT
				
				
				if ((IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd)) && IS_VERSION_AFTER_F(pAd)
					&& (pAd->StaCfg.PSControl.field.rt30xxPowerMode == 3)
					&& (pAd->StaCfg.PSControl.field.EnableNewPS == TRUE))
					{
						RTMP_SEM_LOCK(&pAd->McuCmdLock);
						pAd->brt30xxBanMcuCmd = FALSE;
						RTMP_SEM_UNLOCK(&pAd->McuCmdLock);
					}
#endif 
			}
			else
			
#endif 
			{
			
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				{
				if (INFRA_ON(pAd) && (pAd->CommonCfg.CentralChannel != pAd->CommonCfg.Channel)
					&& (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
				{
					
					AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
					AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
				}
				else
				{
					
					AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
					AsicLockChannel(pAd, pAd->CommonCfg.Channel);
				}
				}
#endif 
			}
	}
        return TRUE;

}



BOOLEAN RT28xxPciAsicRadioOff(
	IN PRTMP_ADAPTER    pAd,
	IN UCHAR            Level,
	IN USHORT           TbttNumToNextWakeUp)
{
#ifdef CONFIG_STA_SUPPORT
	WPDMA_GLO_CFG_STRUC	DmaCfg;
	UCHAR		i, tempBBP_R3 = 0;
#endif 
	BOOLEAN		brc = FALSE, Cancelled;
    UINT32		TbTTTime = 0;
	UINT32		PsPollTime = 0;
    ULONG		BeaconPeriodTime;
    UINT32		RxDmaIdx, RxCpuIdx;
	DBGPRINT(RT_DEBUG_TRACE, ("AsicRadioOff ===> Lv= %d, TxCpuIdx = %d, TxDmaIdx = %d. RxCpuIdx = %d, RxDmaIdx = %d.\n", Level,pAd->TxRing[0].TxCpuIdx, pAd->TxRing[0].TxDmaIdx, pAd->RxRing.RxCpuIdx, pAd->RxRing.RxDmaIdx));

	if (pAd->OpMode == OPMODE_AP && Level==DOT11POWERSAVE)
		return FALSE;

    
	RTMP_IO_READ32(pAd, RX_DRX_IDX , &RxDmaIdx);
	RTMP_IO_READ32(pAd, RX_CRX_IDX , &RxCpuIdx);
	if ((RxDmaIdx > RxCpuIdx) && ((RxDmaIdx - RxCpuIdx) > RX_RING_SIZE/3))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("AsicRadioOff ===> return1. RxDmaIdx = %d ,  RxCpuIdx = %d. \n", RxDmaIdx, RxCpuIdx));
		return FALSE;
	}
	else if ((RxCpuIdx >= RxDmaIdx) && ((RxCpuIdx - RxDmaIdx) < RX_RING_SIZE/3))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("AsicRadioOff ===> return2.  RxCpuIdx = %d. RxDmaIdx = %d ,  \n", RxCpuIdx, RxDmaIdx));
		return FALSE;
	}

    
	
	RTMP_SET_PSFLAG(pAd, fRTMP_PS_DISABLE_TX);
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE)
		&& pAd->OpMode == OPMODE_STA
#ifdef CONFIG_STA_SUPPORT
		&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE
#endif 
		)
	{

	    RTMPCancelTimer(&pAd->Mlme.RadioOnOffTimer,	&Cancelled);
	    RTMPCancelTimer(&pAd->Mlme.PsPollTimer,	&Cancelled);

	    if (Level == DOT11POWERSAVE)
		{
			RTMP_IO_READ32(pAd, TBTT_TIMER, &TbTTTime);
			TbTTTime &= 0x1ffff;
			
			
	        if  (((64*TbTTTime) <((LEAD_TIME*1024) + 40000)) && (TbttNumToNextWakeUp == 0))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("TbTTTime = 0x%x , give up this sleep. \n", TbTTTime));
	            OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
	            
	            RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_DISABLE_TX);
				return FALSE;
			}
			else
			{
				PsPollTime = (64*TbTTTime- LEAD_TIME*1024)/1000;
#ifdef PCIE_PS_SUPPORT
#ifdef CONFIG_STA_SUPPORT
				if ((IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd)) && IS_VERSION_AFTER_F(pAd)
				&& (pAd->StaCfg.PSControl.field.rt30xxPowerMode == 3)
				&& (pAd->StaCfg.PSControl.field.EnableNewPS == TRUE))
				{
							PsPollTime -= 5;
				}
				else
#endif 
#endif 
				PsPollTime -= 3;

	            BeaconPeriodTime = pAd->CommonCfg.BeaconPeriod*102/100;
				if (TbttNumToNextWakeUp > 0)
					PsPollTime += ((TbttNumToNextWakeUp -1) * BeaconPeriodTime);

	            pAd->Mlme.bPsPollTimerRunning = TRUE;
				RTMPSetTimer(&pAd->Mlme.PsPollTimer, PsPollTime);
			}
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("RT28xxPciAsicRadioOff::Level!=DOT11POWERSAVE \n"));
	}

	pAd->bPCIclkOffDisableTx = FALSE;
    RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);

#ifdef CONFIG_STA_SUPPORT
    
	if (pAd->Antenna.field.RxPath > 1 && pAd->OpMode == OPMODE_STA)
	{
    tempBBP_R3 = (pAd->StaCfg.BBPR3 & 0xE7);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, tempBBP_R3);
	}
#endif 

	
	if ((INFRA_ON(pAd) || pAd->OpMode == OPMODE_AP) && (pAd->CommonCfg.CentralChannel != pAd->CommonCfg.Channel)
		&& (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
	{
		
		AsicTurnOffRFClk(pAd, pAd->CommonCfg.CentralChannel);
	}
	else
	{
		
		AsicTurnOffRFClk(pAd, pAd->CommonCfg.Channel);
	}

	if (Level != RTMP_HALT)
	{
		
    
	RTMP_IO_WRITE32(pAd, INT_MASK_CSR, AutoWakeupInt);
	}
	else
	{
		RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	}


	RTMP_IO_WRITE32(pAd, RX_CRX_IDX, pAd->RxRing.RxCpuIdx);
	
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_STATUS, 0xffffffff);
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CID, 0xffffffff);
	
	AsicSendCommandToMcu(pAd, 0x30, PowerSafeCID, 0xff, 0x1);
	
	
	brc = AsicCheckCommanOk(pAd, PowerSafeCID);

	
	
	if ((Level == DOT11POWERSAVE) && (brc == TRUE))
	{
		AsicSendCommandToMcu(pAd, 0x35, PowerRadioOffCID, 0, 0x00);	
		
		AsicCheckCommanOk(pAd, PowerRadioOffCID);
	}
	else if (brc == TRUE)
	{
		AsicSendCommandToMcu(pAd, 0x35, PowerRadioOffCID, 1, 0x00);	
		
		AsicCheckCommanOk(pAd, PowerRadioOffCID);
	}

#ifdef CONFIG_STA_SUPPORT
	
	i = 0;
	do
	{
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &DmaCfg.word);
		if ((DmaCfg.field.RxDMABusy == 0) && (DmaCfg.field.TxDMABusy == 0))
			break;
		RTMPusecDelay(20);
		i++;
	}while(i < 50);

	
#endif 


RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0x1280);


#ifdef PCIE_PS_SUPPORT
#ifdef CONFIG_STA_SUPPORT
if ((IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd)) && IS_VERSION_AFTER_F(pAd)
	&& (pAd->StaCfg.PSControl.field.rt30xxPowerMode == 3)
	&& (pAd->StaCfg.PSControl.field.EnableNewPS == TRUE))
	{
	DBGPRINT(RT_DEBUG_TRACE, ("RT28xxPciAsicRadioOff::3090 return to skip the following TbttNumToNextWakeUp setting for 279x\n"));
	pAd->bPCIclkOff = TRUE;
	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_DISABLE_TX);
	
	return brc;
	}
#endif 
#endif 
	if (Level == DOT11POWERSAVE)
	{
		AUTO_WAKEUP_STRUC	AutoWakeupCfg;
		

		
		if (TbttNumToNextWakeUp == 0)
			TbttNumToNextWakeUp = 1;

		AutoWakeupCfg.word = 0;
		RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);

		
		AutoWakeupCfg.field.NumofSleepingTbtt = TbttNumToNextWakeUp - 1;
		AutoWakeupCfg.field.EnableAutoWakeup = 1;
		AutoWakeupCfg.field.AutoLeadTime = LEAD_TIME;
		RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);
	}

#ifdef CONFIG_STA_SUPPORT
	
	if (Level == RTMP_HALT && pAd->OpMode == OPMODE_STA)
	{
		if ((brc == TRUE) && (i < 50))
			RTMPPCIeLinkCtrlSetting(pAd, 1);
	}
	
	else if (pAd->OpMode == OPMODE_STA)
	{
		if ((brc == TRUE) && (i < 50))
			RTMPPCIeLinkCtrlSetting(pAd, 3);
	}
#endif 

	
	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_DISABLE_TX);
	return TRUE;
}




VOID RT28xxPciMlmeRadioOn(
	IN PRTMP_ADAPTER pAd)
{
    if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
		return;

    DBGPRINT(RT_DEBUG_TRACE,("%s===>\n", __FUNCTION__));

    if ((pAd->OpMode == OPMODE_AP) ||
        ((pAd->OpMode == OPMODE_STA)
        && (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE)
#ifdef CONFIG_STA_SUPPORT
        ||pAd->StaCfg.PSControl.field.EnableNewPS == FALSE
#endif 
        )))
    {
	RT28xxPciAsicRadioOn(pAd, GUI_IDLE_POWER_SAVE);
	

	RTMPRingCleanUp(pAd, QID_AC_BK);
	RTMPRingCleanUp(pAd, QID_AC_BE);
	RTMPRingCleanUp(pAd, QID_AC_VI);
	RTMPRingCleanUp(pAd, QID_AC_VO);
	
	RTMPRingCleanUp(pAd, QID_MGMT);
	RTMPRingCleanUp(pAd, QID_RX);

	
	RTMPEnableRxTx(pAd);

	
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
	    
	    RTMPSetLED(pAd, LED_RADIO_ON);
    }

#ifdef CONFIG_STA_SUPPORT
    if ((pAd->OpMode == OPMODE_STA) &&
        (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE))
        &&(pAd->StaCfg.PSControl.field.EnableNewPS == TRUE))
    {
        BOOLEAN		Cancelled;

	RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_WAKEUP);

        pAd->Mlme.bPsPollTimerRunning = FALSE;
	RTMPCancelTimer(&pAd->Mlme.PsPollTimer,	&Cancelled);
	RTMPCancelTimer(&pAd->Mlme.RadioOnOffTimer,	&Cancelled);
	RTMPSetTimer(&pAd->Mlme.RadioOnOffTimer, 40);
    }
#endif 
}


VOID RT28xxPciMlmeRadioOFF(
	IN PRTMP_ADAPTER pAd)
{
	BOOLEAN brc=TRUE;

    if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
	return;

#ifdef CONFIG_STA_SUPPORT
	
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		if (INFRA_ON(pAd) || ADHOC_ON(pAd))
		{
			MLME_DISASSOC_REQ_STRUCT DisReq;
			MLME_QUEUE_ELEM *pMsgElem = (MLME_QUEUE_ELEM *) kmalloc(sizeof(MLME_QUEUE_ELEM), MEM_ALLOC_FLAG);

			if (pMsgElem)
			{
				COPY_MAC_ADDR(&DisReq.Addr, pAd->CommonCfg.Bssid);
				DisReq.Reason =  REASON_DISASSOC_STA_LEAVING;

				pMsgElem->Machine = ASSOC_STATE_MACHINE;
				pMsgElem->MsgType = MT2_MLME_DISASSOC_REQ;
				pMsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
				NdisMoveMemory(pMsgElem->Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));

				MlmeDisassocReqAction(pAd, pMsgElem);
				kfree(pMsgElem);

				RTMPusecDelay(1000);
			}
		}
	}
#endif 

    DBGPRINT(RT_DEBUG_TRACE,("%s===>\n", __FUNCTION__));

	
	
	
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
    {
	BOOLEAN		Cancelled;
	if (pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
		{
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
			RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &Cancelled);
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
	}
			
			if (pAd->StaCfg.bRadio == TRUE)
			{
				DBGPRINT(RT_DEBUG_TRACE,("-->MlmeRadioOff() return on bRadio == TRUE; \n"));
				return;
			}
			
			if (IDLE_ON(pAd) &&
				(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)))
			{
				RT28xxPciAsicRadioOn(pAd, GUI_IDLE_POWER_SAVE);
			}
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE))
        {
            BOOLEAN Cancelled;
            pAd->Mlme.bPsPollTimerRunning = FALSE;
            RTMPCancelTimer(&pAd->Mlme.PsPollTimer,	&Cancelled);
	        RTMPCancelTimer(&pAd->Mlme.RadioOnOffTimer,	&Cancelled);
        }
		}

        
        if (INFRA_ON(pAd) || ADHOC_ON(pAd))
            LinkDown(pAd, FALSE);
        RTMPusecDelay(10000);
        
        
        BssTableInit(&pAd->ScanTab);

        
    }
#endif 
	
	RTMPSetLED(pAd, LED_RADIO_OFF);

#ifdef CONFIG_STA_SUPPORT



if (pAd->OpMode == OPMODE_STA&&
	OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE)&&
	pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
	{
	RTMPSetTimer(&pAd->Mlme.RadioOnOffTimer, 10);
	}
else
#endif 
{


	brc=RT28xxPciAsicRadioOff(pAd, GUIRADIO_OFF, 0);

	if (brc==FALSE)
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s call RT28xxPciAsicRadioOff fail !!\n", __FUNCTION__));
	}
}

}

#endif 
