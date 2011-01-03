
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


		printk("RX DESC %p  size = %ld\n", pAd->RxDescRing.AllocVa,
					pAd->RxDescRing.AllocSize);

		
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

	DBGPRINT_S(Status, ("<-- RTMPAllocTxRxRingMemory, Status=%x\n", Status));
	return Status;
}



VOID	NICInitTxRxRingAndBacklogQueue(
	IN	PRTMP_ADAPTER	pAd)
{
	
	int i;

	DBGPRINT(RT_DEBUG_TRACE, ("<--> NICInitTxRxRingAndBacklogQueue\n"));

	
	InitializeQueueHeader(&pAd->TxSwQueue[QID_AC_BE]);
	InitializeQueueHeader(&pAd->TxSwQueue[QID_AC_BK]);
	InitializeQueueHeader(&pAd->TxSwQueue[QID_AC_VI]);
	InitializeQueueHeader(&pAd->TxSwQueue[QID_AC_VO]);
	InitializeQueueHeader(&pAd->TxSwQueue[QID_HCCA]);

	
	pAd->RxRing.RxSwReadIdx = 0;
	pAd->RxRing.RxCpuIdx = RX_RING_SIZE - 1;

	
		for (i=0; i<NUM_OF_TX_RING; i++)
		{
			pAd->TxRing[i].TxSwFreeIdx = 0;
			pAd->TxRing[i].TxCpuIdx = 0;
		}

	
	pAd->MgmtRing.TxSwFreeIdx = 0;
	pAd->MgmtRing.TxCpuIdx = 0;

	pAd->PrivateInfo.TxRingFullCnt       = 0;
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
		case QID_HCCA:
			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			pTxRing = &pAd->TxRing[RingType];

			
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


NDIS_STATUS AdapterBlockAllocateMemory(
	IN PVOID	handle,
	OUT	PVOID	*ppAd)
{
	PPCI_DEV pci_dev;
	dma_addr_t	*phy_addr;
	POS_COOKIE pObj = (POS_COOKIE) handle;

	pci_dev = pObj->pci_dev;
	phy_addr = &pObj->pAd_pa;

	*ppAd = (PVOID)vmalloc(sizeof(RTMP_ADAPTER)); 

	if (*ppAd)
	{
		NdisZeroMemory(*ppAd, sizeof(RTMP_ADAPTER));
		((PRTMP_ADAPTER)*ppAd)->OS_Cookie = handle;
		return (NDIS_STATUS_SUCCESS);
	} else {
		return (NDIS_STATUS_FAILURE);
	}
}


void RTMP_AllocateTxDescMemory(
	IN	PRTMP_ADAPTER pAd,
	IN	UINT	Index,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;

	*VirtualAddress = (PVOID)PCI_ALLOC_CONSISTENT(pObj->pci_dev,sizeof(char)*Length, PhysicalAddress);

}

void RTMP_AllocateMgmtDescMemory(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;

	*VirtualAddress = (PVOID)PCI_ALLOC_CONSISTENT(pObj->pci_dev,sizeof(char)*Length, PhysicalAddress);

}

void RTMP_AllocateRxDescMemory(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;

	*VirtualAddress = (PVOID)PCI_ALLOC_CONSISTENT(pObj->pci_dev,sizeof(char)*Length, PhysicalAddress);

}

void RTMP_FreeRxDescMemory(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	PVOID	VirtualAddress,
	IN	NDIS_PHYSICAL_ADDRESS PhysicalAddress)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;

	PCI_FREE_CONSISTENT(pObj->pci_dev, Length, VirtualAddress, PhysicalAddress);
}


void RTMP_AllocateFirstTxBuffer(
	IN	PRTMP_ADAPTER pAd,
	IN	UINT	Index,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;

	*VirtualAddress = (PVOID)PCI_ALLOC_CONSISTENT(pObj->pci_dev,sizeof(char)*Length, PhysicalAddress);
}



void RTMP_AllocateSharedMemory(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;

	*VirtualAddress = (PVOID)PCI_ALLOC_CONSISTENT(pObj->pci_dev,sizeof(char)*Length, PhysicalAddress);
}

VOID RTMPFreeTxRxRingMemory(
    IN  PRTMP_ADAPTER   pAd)
{
	int index, num , j;
	PRTMP_TX_RING pTxRing;
	PTXD_STRUC	  pTxD;
	PNDIS_PACKET  pPacket;
	unsigned int  IrqFlags;

	POS_COOKIE pObj =(POS_COOKIE) pAd->OS_Cookie;

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
			PCI_UNMAP_SINGLE(pObj->pci_dev, pAd->RxRing.Cell[index].DmaBuf.AllocPa, pAd->RxRing.Cell[index].DmaBuf.AllocSize, PCI_DMA_FROMDEVICE);
			RELEASE_NDIS_PACKET(pAd, pAd->RxRing.Cell[index].pNdisPacket, NDIS_STATUS_SUCCESS);
		}
	}
	NdisZeroMemory(pAd->RxRing.Cell, RX_RING_SIZE * sizeof(RTMP_DMACB));

	if (pAd->RxDescRing.AllocVa)
    {
    	PCI_FREE_CONSISTENT(pObj->pci_dev, pAd->RxDescRing.AllocSize, pAd->RxDescRing.AllocVa, pAd->RxDescRing.AllocPa);
    }
    NdisZeroMemory(&pAd->RxDescRing, sizeof(RTMP_DMABUF));

	if (pAd->MgmtDescRing.AllocVa)
	{
		PCI_FREE_CONSISTENT(pObj->pci_dev, pAd->MgmtDescRing.AllocSize,	pAd->MgmtDescRing.AllocVa, pAd->MgmtDescRing.AllocPa);
	}
	NdisZeroMemory(&pAd->MgmtDescRing, sizeof(RTMP_DMABUF));

	for (num = 0; num < NUM_OF_TX_RING; num++)
	{
    	if (pAd->TxBufSpace[num].AllocVa)
	    {
	    	PCI_FREE_CONSISTENT(pObj->pci_dev, pAd->TxBufSpace[num].AllocSize, pAd->TxBufSpace[num].AllocVa, pAd->TxBufSpace[num].AllocPa);
	    }
	    NdisZeroMemory(&pAd->TxBufSpace[num], sizeof(RTMP_DMABUF));

    	if (pAd->TxDescRing[num].AllocVa)
	    {
	    	PCI_FREE_CONSISTENT(pObj->pci_dev, pAd->TxDescRing[num].AllocSize, pAd->TxDescRing[num].AllocVa, pAd->TxDescRing[num].AllocPa);
	    }
	    NdisZeroMemory(&pAd->TxDescRing[num], sizeof(RTMP_DMABUF));
	}

	if (pAd->FragFrame.pFragPacket)
		RELEASE_NDIS_PACKET(pAd, pAd->FragFrame.pFragPacket, NDIS_STATUS_SUCCESS);

	DBGPRINT(RT_DEBUG_TRACE, ("<-- RTMPFreeTxRxRingMemory\n"));
}



PNDIS_PACKET RTMP_AllocateRxPacketBuffer(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress)
{
	PNDIS_PACKET pkt;

	pkt = RTPKT_TO_OSPKT(DEV_ALLOC_SKB(Length));

	if (pkt == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, ("can't allocate rx %ld size packet\n",Length));
	}

	if (pkt) {
		RTMP_SET_PACKET_SOURCE(pkt, PKTSRC_NDIS);
		*VirtualAddress = (PVOID) RTPKT_TO_OSPKT(pkt)->data;
		*PhysicalAddress = PCI_MAP_SINGLE(pAd, *VirtualAddress, Length, -1, PCI_DMA_FROMDEVICE);
	} else {
		*VirtualAddress = (PVOID) NULL;
		*PhysicalAddress = (NDIS_PHYSICAL_ADDRESS) NULL;
	}

	return (PNDIS_PACKET) pkt;
}


VOID Invalid_Remaining_Packet(
	IN	PRTMP_ADAPTER pAd,
	IN	 ULONG VirtualAddress)
{
	NDIS_PHYSICAL_ADDRESS PhysicalAddress;

	PhysicalAddress = PCI_MAP_SINGLE(pAd, (void *)(VirtualAddress+1600), RX_BUFFER_NORMSIZE-1600, -1, PCI_DMA_FROMDEVICE);
}

PNDIS_PACKET GetPacketFromRxRing(
	IN		PRTMP_ADAPTER	pAd,
	OUT		PRT28XX_RXD_STRUC	pSaveRxD,
	OUT		BOOLEAN			*pbReschedule,
	IN OUT	UINT32			*pRxPending)
{
	PRXD_STRUC				pRxD;
	PNDIS_PACKET			pRxPacket = NULL;
	PNDIS_PACKET			pNewPacket;
	PVOID					AllocVa;
	NDIS_PHYSICAL_ADDRESS	AllocPa;
	BOOLEAN					bReschedule = FALSE;

	RTMP_SEM_LOCK(&pAd->RxRingLock);

	if (*pRxPending == 0)
	{
		
		RTMP_IO_READ32(pAd, RX_DRX_IDX , &pAd->RxRing.RxDmaIdx);

		if (pAd->RxRing.RxSwReadIdx == pAd->RxRing.RxDmaIdx)
		{
			
			bReschedule = FALSE;
			goto done;
		}

		
		if (pAd->RxRing.RxDmaIdx > pAd->RxRing.RxSwReadIdx)
			*pRxPending = pAd->RxRing.RxDmaIdx - pAd->RxRing.RxSwReadIdx;
		else
			*pRxPending	= pAd->RxRing.RxDmaIdx + RX_RING_SIZE - pAd->RxRing.RxSwReadIdx;

	}

	
	pRxD = (PRXD_STRUC) pAd->RxRing.Cell[pAd->RxRing.RxSwReadIdx].AllocVa;

	if (pRxD->DDONE == 0)
	{
		*pRxPending = 0;
		
		bReschedule = TRUE;
		goto done;
	}


	
	NdisMoveMemory(pSaveRxD, pRxD, RXD_SIZE);

	pNewPacket = RTMP_AllocateRxPacketBuffer(pAd, RX_BUFFER_AGGRESIZE, FALSE, &AllocVa, &AllocPa);

	if (pNewPacket)
	{
		
		PCI_UNMAP_SINGLE(pAd, pAd->RxRing.Cell[pAd->RxRing.RxSwReadIdx].DmaBuf.AllocPa,
					 pAd->RxRing.Cell[pAd->RxRing.RxSwReadIdx].DmaBuf.AllocSize, PCI_DMA_FROMDEVICE);
		pRxPacket = pAd->RxRing.Cell[pAd->RxRing.RxSwReadIdx].pNdisPacket;

		pAd->RxRing.Cell[pAd->RxRing.RxSwReadIdx].DmaBuf.AllocSize	= RX_BUFFER_AGGRESIZE;
		pAd->RxRing.Cell[pAd->RxRing.RxSwReadIdx].pNdisPacket		= (PNDIS_PACKET) pNewPacket;
		pAd->RxRing.Cell[pAd->RxRing.RxSwReadIdx].DmaBuf.AllocVa	= AllocVa;
		pAd->RxRing.Cell[pAd->RxRing.RxSwReadIdx].DmaBuf.AllocPa	= AllocPa;
		
		pRxD->SDP0 = AllocPa;
	}
	else
	{
		
		pRxPacket = NULL;
		bReschedule = TRUE;
	}

	pRxD->DDONE = 0;

	
	*pRxPending = *pRxPending - 1;

	
	INC_RING_INDEX(pAd->RxRing.RxSwReadIdx, RX_RING_SIZE);

	pAd->RxRing.RxCpuIdx = (pAd->RxRing.RxSwReadIdx == 0) ? (RX_RING_SIZE-1) : (pAd->RxRing.RxSwReadIdx-1);
	RTMP_IO_WRITE32(pAd, RX_CRX_IDX, pAd->RxRing.RxCpuIdx);

done:
	RTMP_SEM_UNLOCK(&pAd->RxRingLock);
	*pbReschedule = bReschedule;
	return pRxPacket;
}


