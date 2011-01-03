

#include "../rt_config.h"


static void rx_done_tasklet(unsigned long data);
static void rt2870_hcca_dma_done_tasklet(unsigned long data);
static void rt2870_ac3_dma_done_tasklet(unsigned long data);
static void rt2870_ac2_dma_done_tasklet(unsigned long data);
static void rt2870_ac1_dma_done_tasklet(unsigned long data);
static void rt2870_ac0_dma_done_tasklet(unsigned long data);
static void rt2870_mgmt_dma_done_tasklet(unsigned long data);
static void rt2870_null_frame_complete_tasklet(unsigned long data);
static void rt2870_rts_frame_complete_tasklet(unsigned long data);
static void rt2870_pspoll_frame_complete_tasklet(unsigned long data);
static void rt2870_dataout_complete_tasklet(unsigned long data);



NDIS_STATUS	NICInitRecv(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR				i;
	NDIS_STATUS			Status = NDIS_STATUS_SUCCESS;
	POS_COOKIE			pObj = (POS_COOKIE) pAd->OS_Cookie;


	DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitRecv\n"));
	pObj = pObj;

	
	pAd->PendingRx = 0;
	pAd->NextRxBulkInReadIndex 	= 0;	
	pAd->NextRxBulkInIndex		= 0 ; 
	pAd->NextRxBulkInPosition 	= 0;

	for (i = 0; i < (RX_RING_SIZE); i++)
	{
		PRX_CONTEXT  pRxContext = &(pAd->RxContext[i]);

		
		pRxContext->pUrb = RTUSB_ALLOC_URB(0);
		if (pRxContext->pUrb == NULL)
		{
			Status = NDIS_STATUS_RESOURCES;
			goto out1;
		}

		
		pRxContext->TransferBuffer = RTUSB_URB_ALLOC_BUFFER(pObj->pUsb_Dev, MAX_RXBULK_SIZE, &pRxContext->data_dma);
		if (pRxContext->TransferBuffer == NULL)
		{
			Status = NDIS_STATUS_RESOURCES;
			goto out1;
		}

		NdisZeroMemory(pRxContext->TransferBuffer, MAX_RXBULK_SIZE);

		pRxContext->pAd	= pAd;
		pRxContext->pIrp = NULL;
		pRxContext->InUse		= FALSE;
		pRxContext->IRPPending	= FALSE;
		pRxContext->Readable	= FALSE;
		
		pRxContext->bRxHandling = FALSE;
		pRxContext->BulkInOffset = 0;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitRecv\n"));
	return Status;

out1:
	for (i = 0; i < (RX_RING_SIZE); i++)
	{
		PRX_CONTEXT  pRxContext = &(pAd->RxContext[i]);

		if (NULL != pRxContext->TransferBuffer)
		{
			RTUSB_URB_FREE_BUFFER(pObj->pUsb_Dev, MAX_RXBULK_SIZE,
								pRxContext->TransferBuffer, pRxContext->data_dma);
			pRxContext->TransferBuffer = NULL;
		}

		if (NULL != pRxContext->pUrb)
		{
			RTUSB_UNLINK_URB(pRxContext->pUrb);
			RTUSB_FREE_URB(pRxContext->pUrb);
			pRxContext->pUrb = NULL;
		}
	}

	return Status;
}



NDIS_STATUS	NICInitTransmit(
	IN	PRTMP_ADAPTER	pAd)
{
#define LM_USB_ALLOC(pObj, Context, TB_Type, BufferSize, Status, msg1, err1, msg2, err2)	\
	Context->pUrb = RTUSB_ALLOC_URB(0);		\
	if (Context->pUrb == NULL) {			\
		DBGPRINT(RT_DEBUG_ERROR, msg1);		\
		Status = NDIS_STATUS_RESOURCES;		\
		goto err1; }						\
											\
	Context->TransferBuffer = 				\
		(TB_Type)RTUSB_URB_ALLOC_BUFFER(pObj->pUsb_Dev, BufferSize, &Context->data_dma);	\
	if (Context->TransferBuffer == NULL) {	\
		DBGPRINT(RT_DEBUG_ERROR, msg2);		\
		Status = NDIS_STATUS_RESOURCES;		\
		goto err2; }

#define LM_URB_FREE(pObj, Context, BufferSize)				\
	if (NULL != Context->pUrb) {							\
		RTUSB_UNLINK_URB(Context->pUrb);					\
		RTUSB_FREE_URB(Context->pUrb);						\
		Context->pUrb = NULL; }								\
	if (NULL != Context->TransferBuffer) {				\
		RTUSB_URB_FREE_BUFFER(pObj->pUsb_Dev, BufferSize,	\
								Context->TransferBuffer,	\
								Context->data_dma);			\
		Context->TransferBuffer = NULL; }

	UCHAR			i, acidx;
	NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
	PTX_CONTEXT		pNullContext   = &(pAd->NullContext);
	PTX_CONTEXT		pPsPollContext = &(pAd->PsPollContext);
	PTX_CONTEXT		pRTSContext    = &(pAd->RTSContext);
	PTX_CONTEXT		pMLMEContext = NULL;

	POS_COOKIE		pObj = (POS_COOKIE) pAd->OS_Cookie;
	PVOID			RingBaseVa;

	RTMP_MGMT_RING  *pMgmtRing;

	DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitTransmit\n"));
	pObj = pObj;

	
	for(acidx = 0; acidx < NUM_OF_TX_RING; acidx++)
	{
		
		InitializeQueueHeader(&pAd->TxSwQueue[acidx]);

		
		pAd->NextBulkOutIndex[acidx] = acidx;
		pAd->BulkOutPending[acidx] = FALSE; 
		
	}

	
	
	
	
	

	
	

	do
	{
		
		
		
		for(acidx=0; acidx<4; acidx++)
		{
			PHT_TX_CONTEXT	pHTTXContext = &(pAd->TxContext[acidx]);

			NdisZeroMemory(pHTTXContext, sizeof(HT_TX_CONTEXT));
			
			LM_USB_ALLOC(pObj, pHTTXContext, PHTTX_BUFFER, sizeof(HTTX_BUFFER), Status,
							("<-- ERROR in Alloc TX TxContext[%d] urb!! \n", acidx),
							done,
							("<-- ERROR in Alloc TX TxContext[%d] HTTX_BUFFER !! \n", acidx),
							out1);

			NdisZeroMemory(pHTTXContext->TransferBuffer->Aggregation, 4);
			pHTTXContext->pAd = pAd;
			pHTTXContext->pIrp = NULL;
			pHTTXContext->IRPPending = FALSE;
			pHTTXContext->NextBulkOutPosition = 0;
			pHTTXContext->ENextBulkOutPosition = 0;
			pHTTXContext->CurWritePosition = 0;
			pHTTXContext->CurWriteRealPos = 0;
			pHTTXContext->BulkOutSize = 0;
			pHTTXContext->BulkOutPipeId = acidx;
			pHTTXContext->bRingEmpty = TRUE;
			pHTTXContext->bCopySavePad = FALSE;

			pAd->BulkOutPending[acidx] = FALSE;
		}


		
		
		
		
		pAd->MgmtDescRing.AllocSize = MGMT_RING_SIZE * sizeof(TX_CONTEXT);
		RTMPAllocateMemory(&pAd->MgmtDescRing.AllocVa, pAd->MgmtDescRing.AllocSize);
		if (pAd->MgmtDescRing.AllocVa == NULL)
		{
			DBGPRINT_ERR(("Failed to allocate a big buffer for MgmtDescRing!\n"));
			Status = NDIS_STATUS_RESOURCES;
			goto out1;
		}
		NdisZeroMemory(pAd->MgmtDescRing.AllocVa, pAd->MgmtDescRing.AllocSize);
		RingBaseVa     = pAd->MgmtDescRing.AllocVa;

		
		pMgmtRing = &pAd->MgmtRing;
		for (i = 0; i < MGMT_RING_SIZE; i++)
		{
			
			pMgmtRing->Cell[i].AllocSize = sizeof(TX_CONTEXT);
			pMgmtRing->Cell[i].AllocVa = RingBaseVa;
			pMgmtRing->Cell[i].pNdisPacket = NULL;
			pMgmtRing->Cell[i].pNextNdisPacket = NULL;

			
			pMLMEContext = (PTX_CONTEXT) pAd->MgmtRing.Cell[i].AllocVa;
			pMLMEContext->pUrb = RTUSB_ALLOC_URB(0);
			if (pMLMEContext->pUrb == NULL)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("<-- ERROR in Alloc TX MLMEContext[%d] urb!! \n", i));
				Status = NDIS_STATUS_RESOURCES;
				goto out2;
			}
			pMLMEContext->pAd = pAd;
			pMLMEContext->pIrp = NULL;
			pMLMEContext->TransferBuffer = NULL;
			pMLMEContext->InUse = FALSE;
			pMLMEContext->IRPPending = FALSE;
			pMLMEContext->bWaitingBulkOut = FALSE;
			pMLMEContext->BulkOutSize = 0;
			pMLMEContext->SelfIdx = i;

			
			RingBaseVa = (PUCHAR) RingBaseVa + sizeof(TX_CONTEXT);
		}
		DBGPRINT(RT_DEBUG_TRACE, ("MGMT Ring: total %d entry allocated\n", i));

		
		pAd->MgmtRing.TxSwFreeIdx = MGMT_RING_SIZE;
		pAd->MgmtRing.TxCpuIdx = 0;
		pAd->MgmtRing.TxDmaIdx = 0;

		
		
		
		for(i=0; i<BEACON_RING_SIZE; i++) 
		{
			PTX_CONTEXT	pBeaconContext = &(pAd->BeaconContext[i]);


			NdisZeroMemory(pBeaconContext, sizeof(TX_CONTEXT));

			
			LM_USB_ALLOC(pObj, pBeaconContext, PTX_BUFFER, sizeof(TX_BUFFER), Status,
							("<-- ERROR in Alloc TX BeaconContext[%d] urb!! \n", i),
							out2,
							("<-- ERROR in Alloc TX BeaconContext[%d] TX_BUFFER !! \n", i),
							out3);

			pBeaconContext->pAd = pAd;
			pBeaconContext->pIrp = NULL;
			pBeaconContext->InUse = FALSE;
			pBeaconContext->IRPPending = FALSE;
		}

		
		
		
		NdisZeroMemory(pNullContext, sizeof(TX_CONTEXT));

		
		LM_USB_ALLOC(pObj, pNullContext, PTX_BUFFER, sizeof(TX_BUFFER), Status,
						("<-- ERROR in Alloc TX NullContext urb!! \n"),
						out3,
						("<-- ERROR in Alloc TX NullContext TX_BUFFER !! \n"),
						out4);

		pNullContext->pAd = pAd;
		pNullContext->pIrp = NULL;
		pNullContext->InUse = FALSE;
		pNullContext->IRPPending = FALSE;

		
		
		
		NdisZeroMemory(pRTSContext, sizeof(TX_CONTEXT));

		
		LM_USB_ALLOC(pObj, pRTSContext, PTX_BUFFER, sizeof(TX_BUFFER), Status,
						("<-- ERROR in Alloc TX RTSContext urb!! \n"),
						out4,
						("<-- ERROR in Alloc TX RTSContext TX_BUFFER !! \n"),
						out5);

		pRTSContext->pAd = pAd;
		pRTSContext->pIrp = NULL;
		pRTSContext->InUse = FALSE;
		pRTSContext->IRPPending = FALSE;

		
		
		
		
		
		LM_USB_ALLOC(pObj, pPsPollContext, PTX_BUFFER, sizeof(TX_BUFFER), Status,
						("<-- ERROR in Alloc TX PsPollContext urb!! \n"),
						out5,
						("<-- ERROR in Alloc TX PsPollContext TX_BUFFER !! \n"),
						out6);

		pPsPollContext->pAd = pAd;
		pPsPollContext->pIrp = NULL;
		pPsPollContext->InUse = FALSE;
		pPsPollContext->IRPPending = FALSE;
		pPsPollContext->bAggregatible = FALSE;
		pPsPollContext->LastOne = TRUE;

	}   while (FALSE);


done:
	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitTransmit\n"));

	return Status;

	
out6:
	LM_URB_FREE(pObj, pPsPollContext, sizeof(TX_BUFFER));

out5:
	LM_URB_FREE(pObj, pRTSContext, sizeof(TX_BUFFER));

out4:
	LM_URB_FREE(pObj, pNullContext, sizeof(TX_BUFFER));

out3:
	for(i=0; i<BEACON_RING_SIZE; i++)
	{
		PTX_CONTEXT	pBeaconContext = &(pAd->BeaconContext[i]);
		if (pBeaconContext)
			LM_URB_FREE(pObj, pBeaconContext, sizeof(TX_BUFFER));
	}

out2:
	if (pAd->MgmtDescRing.AllocVa)
	{
		pMgmtRing = &pAd->MgmtRing;
	for(i=0; i<MGMT_RING_SIZE; i++)
	{
		pMLMEContext = (PTX_CONTEXT) pAd->MgmtRing.Cell[i].AllocVa;
		if (pMLMEContext)
			LM_URB_FREE(pObj, pMLMEContext, sizeof(TX_BUFFER));
	}
		NdisFreeMemory(pAd->MgmtDescRing.AllocVa, pAd->MgmtDescRing.AllocSize, 0);
		pAd->MgmtDescRing.AllocVa = NULL;
	}

out1:
	for (acidx = 0; acidx < 4; acidx++)
	{
		PHT_TX_CONTEXT pTxContext = &(pAd->TxContext[acidx]);
		if (pTxContext)
			LM_URB_FREE(pObj, pTxContext, sizeof(HTTX_BUFFER));
	}

	

	return Status;
}



NDIS_STATUS	RTMPAllocTxRxRingMemory(
	IN	PRTMP_ADAPTER	pAd)
{

	NDIS_STATUS		Status;
	INT				num;


	DBGPRINT(RT_DEBUG_TRACE, ("--> RTMPAllocTxRxRingMemory\n"));


	do
	{
		
		NdisAllocateSpinLock(&pAd->CmdQLock);
		NdisAcquireSpinLock(&pAd->CmdQLock);
		RTUSBInitializeCmdQ(&pAd->CmdQ);
		NdisReleaseSpinLock(&pAd->CmdQLock);


		NdisAllocateSpinLock(&pAd->MLMEBulkOutLock);
		
		NdisAllocateSpinLock(&pAd->BulkOutLock[0]);
		NdisAllocateSpinLock(&pAd->BulkOutLock[1]);
		NdisAllocateSpinLock(&pAd->BulkOutLock[2]);
		NdisAllocateSpinLock(&pAd->BulkOutLock[3]);
		NdisAllocateSpinLock(&pAd->BulkOutLock[4]);
		NdisAllocateSpinLock(&pAd->BulkOutLock[5]);
		NdisAllocateSpinLock(&pAd->BulkInLock);

		for (num = 0; num < NUM_OF_TX_RING; num++)
		{
			NdisAllocateSpinLock(&pAd->TxContextQueueLock[num]);
		}











		
		
		


		
		
		
		Status = NICInitTransmit(pAd);
		if (Status != NDIS_STATUS_SUCCESS)
			break;

		
		
		
		Status = NICInitRecv(pAd);
		if (Status != NDIS_STATUS_SUCCESS)
			break;

		pAd->PendingIoCount = 1;

	} while (FALSE);

	NdisZeroMemory(&pAd->FragFrame, sizeof(FRAGMENT_FRAME));
	pAd->FragFrame.pFragPacket =  RTMP_AllocateFragPacketBuffer(pAd, RX_BUFFER_NORMSIZE);

	if (pAd->FragFrame.pFragPacket == NULL)
	{
		Status = NDIS_STATUS_RESOURCES;
	}

	DBGPRINT_S(Status, ("<-- RTMPAllocTxRxRingMemory, Status=%x\n", Status));
	return Status;
}



VOID	RTMPFreeTxRxRingMemory(
	IN	PRTMP_ADAPTER	pAd)
{
#define LM_URB_FREE(pObj, Context, BufferSize)				\
	if (NULL != Context->pUrb) {							\
		RTUSB_UNLINK_URB(Context->pUrb);					\
		RTUSB_FREE_URB(Context->pUrb);						\
		Context->pUrb = NULL; }								\
	if (NULL != Context->TransferBuffer) {					\
		RTUSB_URB_FREE_BUFFER(pObj->pUsb_Dev, BufferSize,	\
								Context->TransferBuffer,	\
								Context->data_dma);			\
		Context->TransferBuffer = NULL; }


	UINT                i, acidx;
	PTX_CONTEXT			pNullContext   = &pAd->NullContext;
	PTX_CONTEXT			pPsPollContext = &pAd->PsPollContext;
	PTX_CONTEXT			pRTSContext    = &pAd->RTSContext;

	
	POS_COOKIE			pObj = (POS_COOKIE) pAd->OS_Cookie;


	DBGPRINT(RT_DEBUG_ERROR, ("---> RTMPFreeTxRxRingMemory\n"));
	pObj = pObj;

	
	for(i=0; i<(RX_RING_SIZE); i++)
	{
		PRX_CONTEXT  pRxContext = &(pAd->RxContext[i]);
		if (pRxContext)
			LM_URB_FREE(pObj, pRxContext, MAX_RXBULK_SIZE);
	}

	
	LM_URB_FREE(pObj, pPsPollContext, sizeof(TX_BUFFER));

	
	LM_URB_FREE(pObj, pNullContext, sizeof(TX_BUFFER));

	
	LM_URB_FREE(pObj, pRTSContext, sizeof(TX_BUFFER));


	
	for(i=0; i<BEACON_RING_SIZE; i++)
	{
		PTX_CONTEXT	pBeaconContext = &(pAd->BeaconContext[i]);
		if (pBeaconContext)
			LM_URB_FREE(pObj, pBeaconContext, sizeof(TX_BUFFER));
	}


	
	for(i = 0; i < MGMT_RING_SIZE; i++)
	{
		PTX_CONTEXT pMLMEContext = (PTX_CONTEXT)pAd->MgmtRing.Cell[i].AllocVa;
		
		if (NULL != pAd->MgmtRing.Cell[i].pNdisPacket)
		{
			RTMPFreeNdisPacket(pAd, pAd->MgmtRing.Cell[i].pNdisPacket);
			pAd->MgmtRing.Cell[i].pNdisPacket = NULL;
			pMLMEContext->TransferBuffer = NULL;
		}

		if (pMLMEContext)
		{
			if (NULL != pMLMEContext->pUrb)
			{
				RTUSB_UNLINK_URB(pMLMEContext->pUrb);
				RTUSB_FREE_URB(pMLMEContext->pUrb);
				pMLMEContext->pUrb = NULL;
			}
		}
	}
	if (pAd->MgmtDescRing.AllocVa)
		NdisFreeMemory(pAd->MgmtDescRing.AllocVa, pAd->MgmtDescRing.AllocSize, 0);


	
		for(acidx=0; acidx<4; acidx++)
		{
		PHT_TX_CONTEXT pHTTXContext = &(pAd->TxContext[acidx]);
			if (pHTTXContext)
				LM_URB_FREE(pObj, pHTTXContext, sizeof(HTTX_BUFFER));
		}

	if (pAd->FragFrame.pFragPacket)
		RELEASE_NDIS_PACKET(pAd, pAd->FragFrame.pFragPacket, NDIS_STATUS_SUCCESS);

	for(i=0; i<6; i++)
	{
		NdisFreeSpinLock(&pAd->BulkOutLock[i]);
	}

	NdisFreeSpinLock(&pAd->BulkInLock);
	NdisFreeSpinLock(&pAd->MLMEBulkOutLock);

	NdisFreeSpinLock(&pAd->CmdQLock);

	
	RTUSB_CLEAR_BULK_FLAG(pAd, 0xffffffff);








	DBGPRINT(RT_DEBUG_ERROR, ("<--- ReleaseAdapter\n"));
}



NDIS_STATUS AdapterBlockAllocateMemory(
	IN PVOID	handle,
	OUT	PVOID	*ppAd)
{
	PUSB_DEV	usb_dev;
	POS_COOKIE	pObj = (POS_COOKIE) handle;


	usb_dev = pObj->pUsb_Dev;

	pObj->MLMEThr_pid	= NULL;
	pObj->RTUSBCmdThr_pid	= NULL;

	*ppAd = (PVOID)vmalloc(sizeof(RTMP_ADAPTER));

	if (*ppAd)
	{
		NdisZeroMemory(*ppAd, sizeof(RTMP_ADAPTER));
		((PRTMP_ADAPTER)*ppAd)->OS_Cookie = handle;
		return (NDIS_STATUS_SUCCESS);
	}
	else
	{
		return (NDIS_STATUS_FAILURE);
	}
}



NDIS_STATUS	 CreateThreads(
	IN	struct net_device *net_dev)
{
	PRTMP_ADAPTER pAd = net_dev->ml_priv;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	pid_t pid_number;

	

	init_MUTEX_LOCKED(&(pAd->mlme_semaphore));
	init_completion (&pAd->mlmeComplete);

	init_MUTEX_LOCKED(&(pAd->RTUSBCmd_semaphore));
	init_completion (&pAd->CmdQComplete);

	init_MUTEX_LOCKED(&(pAd->RTUSBTimer_semaphore));
	init_completion (&pAd->TimerQComplete);

	
	pObj->MLMEThr_pid = NULL;
	pid_number = kernel_thread(MlmeThread, pAd, CLONE_VM);
	if (pid_number < 0)
	{
		printk (KERN_WARNING "%s: unable to start Mlme thread\n",pAd->net_dev->name);
		return NDIS_STATUS_FAILURE;
	}

	pObj->MLMEThr_pid = find_get_pid(pid_number);

	
	wait_for_completion(&(pAd->mlmeComplete));

	
	pObj->RTUSBCmdThr_pid = NULL;
	pid_number = kernel_thread(RTUSBCmdThread, pAd, CLONE_VM);
	if (pid_number < 0)
	{
		printk (KERN_WARNING "%s: unable to start RTUSBCmd thread\n",pAd->net_dev->name);
		return NDIS_STATUS_FAILURE;
	}

	pObj->RTUSBCmdThr_pid = find_get_pid(pid_number);

	wait_for_completion(&(pAd->CmdQComplete));

	pObj->TimerQThr_pid = NULL;
	pid_number = kernel_thread(TimerQThread, pAd, CLONE_VM);
	if (pid_number < 0)
	{
		printk (KERN_WARNING "%s: unable to start TimerQThread\n",pAd->net_dev->name);
		return NDIS_STATUS_FAILURE;
	}

	pObj->TimerQThr_pid = find_get_pid(pid_number);

	
	wait_for_completion(&(pAd->TimerQComplete));

	
	tasklet_init(&pObj->rx_done_task, rx_done_tasklet, (ULONG)pAd);
	tasklet_init(&pObj->mgmt_dma_done_task, rt2870_mgmt_dma_done_tasklet, (unsigned long)pAd);
	tasklet_init(&pObj->ac0_dma_done_task, rt2870_ac0_dma_done_tasklet, (unsigned long)pAd);
	tasklet_init(&pObj->ac1_dma_done_task, rt2870_ac1_dma_done_tasklet, (unsigned long)pAd);
	tasklet_init(&pObj->ac2_dma_done_task, rt2870_ac2_dma_done_tasklet, (unsigned long)pAd);
	tasklet_init(&pObj->ac3_dma_done_task, rt2870_ac3_dma_done_tasklet, (unsigned long)pAd);
	tasklet_init(&pObj->hcca_dma_done_task, rt2870_hcca_dma_done_tasklet, (unsigned long)pAd);
	tasklet_init(&pObj->tbtt_task, tbtt_tasklet, (unsigned long)pAd);
	tasklet_init(&pObj->null_frame_complete_task, rt2870_null_frame_complete_tasklet, (unsigned long)pAd);
	tasklet_init(&pObj->rts_frame_complete_task, rt2870_rts_frame_complete_tasklet, (unsigned long)pAd);
	tasklet_init(&pObj->pspoll_frame_complete_task, rt2870_pspoll_frame_complete_tasklet, (unsigned long)pAd);

	return NDIS_STATUS_SUCCESS;
}


VOID	RTMPAddBSSIDCipher(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				Aid,
	IN	PNDIS_802_11_KEY	pKey,
	IN  UCHAR   			CipherAlg)
{
	PUCHAR		pTxMic, pRxMic;
	BOOLEAN 	bKeyRSC, bAuthenticator; 

	UCHAR		i;
	ULONG		WCIDAttri;
	USHORT	 	offset;
	UCHAR		KeyIdx, IVEIV[8];
	UINT32		Value;

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPAddBSSIDCipher==> Aid = %d\n",Aid));

	
	bKeyRSC 	   = (pKey->KeyIndex & 0x20000000) ? TRUE : FALSE;

	
	bAuthenticator = (pKey->KeyIndex & 0x10000000) ? TRUE : FALSE;
	KeyIdx = (UCHAR)pKey->KeyIndex&0xff;

	if (KeyIdx > 4)
		return;


	if (pAd->MacTab.Content[Aid].PairwiseKey.CipherAlg == CIPHER_TKIP)
	{	if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPANone)
		{
			
			pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
			pRxMic = pTxMic;
		}
		else if (bAuthenticator == TRUE)
		{
			pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
			pRxMic = (PUCHAR) (&pKey->KeyMaterial) + 24;
		}
		else
		{
			pRxMic = (PUCHAR) (&pKey->KeyMaterial) + 16;
			pTxMic = (PUCHAR) (&pKey->KeyMaterial) + 24;
		}

		offset = PAIRWISE_KEY_TABLE_BASE + (Aid * HW_KEY_ENTRY_SIZE) + 0x10;
		for (i=0; i<8; )
		{
			Value = *(pTxMic+i);
			Value += (*(pTxMic+i+1)<<8);
			Value += (*(pTxMic+i+2)<<16);
			Value += (*(pTxMic+i+3)<<24);
			RTUSBWriteMACRegister(pAd, offset+i, Value);
			i+=4;
		}

		offset = PAIRWISE_KEY_TABLE_BASE + (Aid * HW_KEY_ENTRY_SIZE) + 0x18;
		for (i=0; i<8; )
		{
			Value = *(pRxMic+i);
			Value += (*(pRxMic+i+1)<<8);
			Value += (*(pRxMic+i+2)<<16);
			Value += (*(pRxMic+i+3)<<24);
			RTUSBWriteMACRegister(pAd, offset+i, Value);
			i+=4;
		}

		
		NdisMoveMemory(pAd->MacTab.Content[Aid].PairwiseKey.RxMic, pRxMic, 8);
		NdisMoveMemory(pAd->MacTab.Content[Aid].PairwiseKey.TxMic, pTxMic, 8);

		DBGPRINT(RT_DEBUG_TRACE,
				("	TxMIC  = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x \n",
				pTxMic[0],pTxMic[1],pTxMic[2],pTxMic[3],
				pTxMic[4],pTxMic[5],pTxMic[6],pTxMic[7]));
		DBGPRINT(RT_DEBUG_TRACE,
				("	RxMIC  = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x \n",
				pRxMic[0],pRxMic[1],pRxMic[2],pRxMic[3],
				pRxMic[4],pRxMic[5],pRxMic[6],pRxMic[7]));
	}

	
	pAd->MacTab.Content[BSSID_WCID].PairwiseKey.KeyLen= (UCHAR)pKey->KeyLength;
	NdisMoveMemory(pAd->MacTab.Content[Aid].PairwiseKey.Key, &pKey->KeyMaterial, pKey->KeyLength);

	
	if (bKeyRSC == TRUE)
		NdisMoveMemory(pAd->MacTab.Content[Aid].PairwiseKey.RxTsc, &pKey->KeyRSC, 6);
	else
		NdisZeroMemory(pAd->MacTab.Content[Aid].PairwiseKey.RxTsc, 6);

	
	pAd->MacTab.Content[Aid].PairwiseKey.TxTsc[0] = 1;
	pAd->MacTab.Content[Aid].PairwiseKey.TxTsc[1] = 0;
	pAd->MacTab.Content[Aid].PairwiseKey.TxTsc[2] = 0;
	pAd->MacTab.Content[Aid].PairwiseKey.TxTsc[3] = 0;
	pAd->MacTab.Content[Aid].PairwiseKey.TxTsc[4] = 0;
	pAd->MacTab.Content[Aid].PairwiseKey.TxTsc[5] = 0;

	CipherAlg = pAd->MacTab.Content[Aid].PairwiseKey.CipherAlg;

	offset = PAIRWISE_KEY_TABLE_BASE + (Aid * HW_KEY_ENTRY_SIZE);
	RTUSBMultiWrite(pAd, (USHORT) offset, pKey->KeyMaterial,
				((pKey->KeyLength == LEN_TKIP_KEY) ? 16 : (USHORT)pKey->KeyLength));

	offset = SHARED_KEY_TABLE_BASE + (KeyIdx * HW_KEY_ENTRY_SIZE);
	RTUSBMultiWrite(pAd, (USHORT) offset, pKey->KeyMaterial, (USHORT)pKey->KeyLength);

	offset = PAIRWISE_IVEIV_TABLE_BASE + (Aid * HW_IVEIV_ENTRY_SIZE);
	NdisZeroMemory(IVEIV, 8);

	
	if ((CipherAlg == CIPHER_TKIP) ||
		(CipherAlg == CIPHER_TKIP_NO_MIC) ||
		(CipherAlg == CIPHER_AES))
	{
		IVEIV[3] = 0x20; 
	}
	
	
	else
	{
		IVEIV[3] |= (KeyIdx<< 6);
	}
	RTUSBMultiWrite(pAd, (USHORT) offset, IVEIV, 8);

	
	if ((CipherAlg == CIPHER_TKIP) ||
		(CipherAlg == CIPHER_TKIP_NO_MIC) ||
		(CipherAlg == CIPHER_AES))
	{
		WCIDAttri = (CipherAlg<<1)|SHAREDKEYTABLE;
	}
	else
		WCIDAttri = (CipherAlg<<1)|SHAREDKEYTABLE;

	offset = MAC_WCID_ATTRIBUTE_BASE + (Aid* HW_WCID_ATTRI_SIZE);
	RTUSBWriteMACRegister(pAd, offset, WCIDAttri);
	RTUSBReadMACRegister(pAd, offset, &Value);

	DBGPRINT(RT_DEBUG_TRACE, ("BSSID_WCID : offset = %x, WCIDAttri = %lx\n",
			offset, WCIDAttri));

	
	
	

	DBGPRINT(RT_DEBUG_ERROR, ("AddBSSIDasWCIDEntry: Alg=%s, KeyLength = %d\n",
			CipherName[CipherAlg], pKey->KeyLength));
	DBGPRINT(RT_DEBUG_TRACE, ("Key [idx=%x] [KeyLen = %d]\n",
			pKey->KeyIndex, pKey->KeyLength));
	for(i=0; i<pKey->KeyLength; i++)
		DBGPRINT_RAW(RT_DEBUG_TRACE,(" %x:", pKey->KeyMaterial[i]));
	DBGPRINT(RT_DEBUG_TRACE,("	 \n"));
}


#define RT2870_RXDMALEN_FIELD_SIZE			4
PNDIS_PACKET GetPacketFromRxRing(
	IN		PRTMP_ADAPTER		pAd,
	OUT		PRT28XX_RXD_STRUC	pSaveRxD,
	OUT		BOOLEAN				*pbReschedule,
	IN OUT	UINT32				*pRxPending)
{
	PRX_CONTEXT		pRxContext;
	PNDIS_PACKET	pSkb;
	PUCHAR			pData;
	ULONG			ThisFrameLen;
	ULONG			RxBufferLength;
	PRXWI_STRUC		pRxWI;

	pRxContext = &pAd->RxContext[pAd->NextRxBulkInReadIndex];
	if ((pRxContext->Readable == FALSE) || (pRxContext->InUse == TRUE))
		return NULL;

	RxBufferLength = pRxContext->BulkInOffset - pAd->ReadPosition;
	if (RxBufferLength < (RT2870_RXDMALEN_FIELD_SIZE + sizeof(RXWI_STRUC) + sizeof(RXINFO_STRUC)))
	{
		goto label_null;
	}

	pData = &pRxContext->TransferBuffer[pAd->ReadPosition]; 
	
	ThisFrameLen = *pData + (*(pData+1)<<8);
    if (ThisFrameLen == 0)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("BIRIdx(%d): RXDMALen is zero.[%ld], BulkInBufLen = %ld)\n",
								pAd->NextRxBulkInReadIndex, ThisFrameLen, pRxContext->BulkInOffset));
		goto label_null;
	}
	if ((ThisFrameLen&0x3) != 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("BIRIdx(%d): RXDMALen not multiple of 4.[%ld], BulkInBufLen = %ld)\n",
								pAd->NextRxBulkInReadIndex, ThisFrameLen, pRxContext->BulkInOffset));
		goto label_null;
	}

	if ((ThisFrameLen + 8)> RxBufferLength)	
	{
		DBGPRINT(RT_DEBUG_TRACE,("BIRIdx(%d):FrameLen(0x%lx) outranges. BulkInLen=0x%lx, remaining RxBufLen=0x%lx, ReadPos=0x%lx\n",
						pAd->NextRxBulkInReadIndex, ThisFrameLen, pRxContext->BulkInOffset, RxBufferLength, pAd->ReadPosition));

		
		goto label_null;
	}

	
	pData += RT2870_RXDMALEN_FIELD_SIZE;
	pRxWI = (PRXWI_STRUC)pData;

	if (pRxWI->MPDUtotalByteCount > ThisFrameLen)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s():pRxWIMPDUtotalByteCount(%d) large than RxDMALen(%ld)\n",
									__func__, pRxWI->MPDUtotalByteCount, ThisFrameLen));
		goto label_null;
	}

	
	pSkb = dev_alloc_skb(ThisFrameLen);
	if (pSkb == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s():Cannot Allocate sk buffer for this Bulk-In buffer!\n", __func__));
		goto label_null;
	}

	
	memcpy(skb_put(pSkb, ThisFrameLen), pData, ThisFrameLen);
	RTPKT_TO_OSPKT(pSkb)->dev = get_netdev_from_bssid(pAd, BSS0);
	RTMP_SET_PACKET_SOURCE(OSPKT_TO_RTPKT(pSkb), PKTSRC_NDIS);

	
	*pSaveRxD = *(PRXINFO_STRUC)(pData + ThisFrameLen);

	
	pAd->ReadPosition += (ThisFrameLen + RT2870_RXDMALEN_FIELD_SIZE + RXINFO_SIZE);	

	return pSkb;

label_null:

	return NULL;
}



static void rx_done_tasklet(unsigned long data)
{
	purbb_t 			pUrb;
	PRX_CONTEXT			pRxContext;
	PRTMP_ADAPTER		pAd;
	NTSTATUS			Status;
	unsigned int		IrqFlags;

	pUrb		= (purbb_t)data;
	pRxContext	= (PRX_CONTEXT)pUrb->context;
	pAd 		= pRxContext->pAd;
	Status = pUrb->status;


	RTMP_IRQ_LOCK(&pAd->BulkInLock, IrqFlags);
	pRxContext->InUse = FALSE;
	pRxContext->IRPPending = FALSE;
	pRxContext->BulkInOffset += pUrb->actual_length;
	
	pAd->PendingRx--;

	if (Status == USB_ST_NOERROR)
	{
		pAd->BulkInComplete++;
		pAd->NextRxBulkInPosition = 0;
		if (pRxContext->BulkInOffset)	
		{
			pRxContext->Readable = TRUE;
			INC_RING_INDEX(pAd->NextRxBulkInIndex, RX_RING_SIZE);
		}
		RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);
	}
	else	 
	{
		pAd->BulkInCompleteFail++;
		
		pRxContext->Readable = FALSE;
		RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);

		
		if ((!RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
									fRTMP_ADAPTER_BULKIN_RESET |
									fRTMP_ADAPTER_HALT_IN_PROGRESS |
									fRTMP_ADAPTER_NIC_NOT_EXIST))))
		{

			DBGPRINT_RAW(RT_DEBUG_ERROR, ("Bulk In Failed. Status=%d, BIIdx=0x%x, BIRIdx=0x%x, actual_length= 0x%x\n",
							Status, pAd->NextRxBulkInIndex, pAd->NextRxBulkInReadIndex, pRxContext->pUrb->actual_length));

			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);
			RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_IN, NULL, 0);
		}
	}

	ASSERT((pRxContext->InUse == pRxContext->IRPPending));

	RTUSBBulkReceive(pAd);

	return;

}


static void rt2870_mgmt_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER 	pAd;
	PTX_CONTEXT		pMLMEContext;
	int				index;
	PNDIS_PACKET	pPacket;
	purbb_t			pUrb;
	NTSTATUS		Status;
	unsigned long	IrqFlags;


	pUrb			= (purbb_t)data;
	pMLMEContext	= (PTX_CONTEXT)pUrb->context;
	pAd 			= pMLMEContext->pAd;
	Status			= pUrb->status;
	index 			= pMLMEContext->SelfIdx;

	ASSERT((pAd->MgmtRing.TxDmaIdx == index));

	RTMP_IRQ_LOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);


	if (Status != USB_ST_NOERROR)
	{
		
		if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("Bulk Out MLME Failed, Status=%d!\n", Status));
			
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
			pAd->bulkResetPipeid = (MGMTPIPEIDX | BULKOUT_MGMT_RESET_FLAG);
		}
	}

	pAd->BulkOutPending[MGMTPIPEIDX] = FALSE;
	RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[MGMTPIPEIDX], IrqFlags);

	RTMP_IRQ_LOCK(&pAd->MLMEBulkOutLock, IrqFlags);
	
	pMLMEContext->IRPPending = FALSE;
	pMLMEContext->InUse = FALSE;
	pMLMEContext->bWaitingBulkOut = FALSE;
	pMLMEContext->BulkOutSize = 0;

	pPacket = pAd->MgmtRing.Cell[index].pNdisPacket;
	pAd->MgmtRing.Cell[index].pNdisPacket = NULL;

	
	INC_RING_INDEX(pAd->MgmtRing.TxDmaIdx, MGMT_RING_SIZE);
	pAd->MgmtRing.TxSwFreeIdx++;
	RTMP_IRQ_UNLOCK(&pAd->MLMEBulkOutLock, IrqFlags);

	
	if (pPacket)
		RTMPFreeNdisPacket(pAd, pPacket);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST))))
	{
		
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET) &&
			((pAd->bulkResetPipeid & BULKOUT_MGMT_RESET_FLAG) == BULKOUT_MGMT_RESET_FLAG))
		{	
			RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		}
		else
		{

			
			
			if (pAd->MgmtRing.TxSwFreeIdx < MGMT_RING_SIZE )
			{
				RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_MLME);
			}
				RTUSBKickBulkOut(pAd);
			}
		}

}


static void rt2870_hcca_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER		pAd;
	PHT_TX_CONTEXT		pHTTXContext;
	UCHAR				BulkOutPipeId = 4;
	purbb_t				pUrb;

	DBGPRINT_RAW(RT_DEBUG_ERROR, ("--->hcca_dma_done_tasklet\n"));

	pUrb			= (purbb_t)data;
	pHTTXContext	= (PHT_TX_CONTEXT)pUrb->context;
	pAd				= pHTTXContext->pAd;

	rt2870_dataout_complete_tasklet((unsigned long)pUrb);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST))))
	{
		
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))
		{
			RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		}
		else
		{	pHTTXContext = &pAd->TxContext[BulkOutPipeId];
			if ((pAd->TxSwQueue[BulkOutPipeId].Number > 0) &&
				
				(pAd->DeQueueRunning[BulkOutPipeId] == FALSE) &&
				(pHTTXContext->bCurWriting == FALSE))
			{
				RTMPDeQueuePacket(pAd, FALSE, BulkOutPipeId, MAX_TX_PROCESS);
			}

			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL<<4);
			RTUSBKickBulkOut(pAd);
		}
	}

	DBGPRINT_RAW(RT_DEBUG_ERROR, ("<---hcca_dma_done_tasklet\n"));
}


static void rt2870_ac3_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER		pAd;
	PHT_TX_CONTEXT		pHTTXContext;
	UCHAR				BulkOutPipeId = 3;
	purbb_t				pUrb;


	pUrb			= (purbb_t)data;
	pHTTXContext	= (PHT_TX_CONTEXT)pUrb->context;
	pAd				= pHTTXContext->pAd;

	rt2870_dataout_complete_tasklet((unsigned long)pUrb);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST))))
	{
		
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))
		{
			RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		}
		else
		{	pHTTXContext = &pAd->TxContext[BulkOutPipeId];
			if ((pAd->TxSwQueue[BulkOutPipeId].Number > 0) &&
				
				(pAd->DeQueueRunning[BulkOutPipeId] == FALSE) &&
				(pHTTXContext->bCurWriting == FALSE))
			{
				RTMPDeQueuePacket(pAd, FALSE, BulkOutPipeId, MAX_TX_PROCESS);
			}

			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL<<3);
			RTUSBKickBulkOut(pAd);
		}
	}


		return;
}


static void rt2870_ac2_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER		pAd;
	PHT_TX_CONTEXT		pHTTXContext;
	UCHAR				BulkOutPipeId = 2;
	purbb_t				pUrb;


	pUrb			= (purbb_t)data;
	pHTTXContext	= (PHT_TX_CONTEXT)pUrb->context;
	pAd				= pHTTXContext->pAd;

	rt2870_dataout_complete_tasklet((unsigned long)pUrb);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST))))
	{
		
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))
		{
			RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		}
		else
		{	pHTTXContext = &pAd->TxContext[BulkOutPipeId];
			if ((pAd->TxSwQueue[BulkOutPipeId].Number > 0) &&
				
				(pAd->DeQueueRunning[BulkOutPipeId] == FALSE) &&
				(pHTTXContext->bCurWriting == FALSE))
			{
				RTMPDeQueuePacket(pAd, FALSE, BulkOutPipeId, MAX_TX_PROCESS);
			}

			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL<<2);
			RTUSBKickBulkOut(pAd);
		}
	}

		return;
}


static void rt2870_ac1_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER		pAd;
	PHT_TX_CONTEXT		pHTTXContext;
	UCHAR				BulkOutPipeId = 1;
	purbb_t				pUrb;


	pUrb			= (purbb_t)data;
	pHTTXContext	= (PHT_TX_CONTEXT)pUrb->context;
	pAd				= pHTTXContext->pAd;

	rt2870_dataout_complete_tasklet((unsigned long)pUrb);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST))))
	{
		
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))
		{
			RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		}
		else
		{	pHTTXContext = &pAd->TxContext[BulkOutPipeId];
			if ((pAd->TxSwQueue[BulkOutPipeId].Number > 0) &&
				
				(pAd->DeQueueRunning[BulkOutPipeId] == FALSE) &&
				(pHTTXContext->bCurWriting == FALSE))
			{
				RTMPDeQueuePacket(pAd, FALSE, BulkOutPipeId, MAX_TX_PROCESS);
			}

			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL<<1);
			RTUSBKickBulkOut(pAd);
		}
	}


	return;
}


static void rt2870_ac0_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER		pAd;
	PHT_TX_CONTEXT		pHTTXContext;
	UCHAR				BulkOutPipeId = 0;
	purbb_t				pUrb;


	pUrb			= (purbb_t)data;
	pHTTXContext	= (PHT_TX_CONTEXT)pUrb->context;
	pAd				= pHTTXContext->pAd;

	rt2870_dataout_complete_tasklet((unsigned long)pUrb);

	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST))))
	{
		
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))
		{
			RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		}
		else
		{	pHTTXContext = &pAd->TxContext[BulkOutPipeId];
			if ((pAd->TxSwQueue[BulkOutPipeId].Number > 0) &&
				
				(pAd->DeQueueRunning[BulkOutPipeId] == FALSE) &&
				(pHTTXContext->bCurWriting == FALSE))
			{
				RTMPDeQueuePacket(pAd, FALSE, BulkOutPipeId, MAX_TX_PROCESS);
			}

			RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_DATA_NORMAL);
			RTUSBKickBulkOut(pAd);
		}
	}


	return;

}


static void rt2870_null_frame_complete_tasklet(unsigned long data)
{
	PRTMP_ADAPTER	pAd;
	PTX_CONTEXT		pNullContext;
	purbb_t			pUrb;
	NTSTATUS		Status;
	unsigned long	irqFlag;


	pUrb			= (purbb_t)data;
	pNullContext	= (PTX_CONTEXT)pUrb->context;
	pAd 			= pNullContext->pAd;
	Status 			= pUrb->status;

	
	RTMP_IRQ_LOCK(&pAd->BulkOutLock[0], irqFlag);
	pNullContext->IRPPending 	= FALSE;
	pNullContext->InUse 		= FALSE;
	pAd->BulkOutPending[0] = FALSE;
	pAd->watchDogTxPendingCnt[0] = 0;

	if (Status == USB_ST_NOERROR)
	{
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], irqFlag);

		RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
	}
	else	
	{
		if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("Bulk Out Null Frame Failed, ReasonCode=%d!\n", Status));
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
			pAd->bulkResetPipeid = (MGMTPIPEIDX | BULKOUT_MGMT_RESET_FLAG);
			RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], irqFlag);
			RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		}
		else
		{
			RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], irqFlag);
		}
	}

	
	
	RTUSBKickBulkOut(pAd);

}


static void rt2870_rts_frame_complete_tasklet(unsigned long data)
{
	PRTMP_ADAPTER	pAd;
	PTX_CONTEXT		pRTSContext;
	purbb_t			pUrb;
	NTSTATUS		Status;
	unsigned long	irqFlag;


	pUrb		= (purbb_t)data;
	pRTSContext	= (PTX_CONTEXT)pUrb->context;
	pAd			= pRTSContext->pAd;
	Status		= pUrb->status;

	
	RTMP_IRQ_LOCK(&pAd->BulkOutLock[0], irqFlag);
	pRTSContext->IRPPending = FALSE;
	pRTSContext->InUse		= FALSE;

	if (Status == USB_ST_NOERROR)
	{
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], irqFlag);
		RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
	}
	else	
	{
		if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("Bulk Out RTS Frame Failed\n"));
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
			pAd->bulkResetPipeid = (MGMTPIPEIDX | BULKOUT_MGMT_RESET_FLAG);
			RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], irqFlag);
			RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		}
		else
		{
			RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[0], irqFlag);
		}
	}

	RTMP_SEM_LOCK(&pAd->BulkOutLock[pRTSContext->BulkOutPipeId]);
	pAd->BulkOutPending[pRTSContext->BulkOutPipeId] = FALSE;
	RTMP_SEM_UNLOCK(&pAd->BulkOutLock[pRTSContext->BulkOutPipeId]);

	
	
	RTUSBKickBulkOut(pAd);

}


static void rt2870_pspoll_frame_complete_tasklet(unsigned long data)
{
	PRTMP_ADAPTER	pAd;
	PTX_CONTEXT		pPsPollContext;
	purbb_t			pUrb;
	NTSTATUS		Status;


	pUrb			= (purbb_t)data;
	pPsPollContext	= (PTX_CONTEXT)pUrb->context;
	pAd				= pPsPollContext->pAd;
	Status			= pUrb->status;

	
	pPsPollContext->IRPPending	= FALSE;
	pPsPollContext->InUse		= FALSE;
	pAd->watchDogTxPendingCnt[0] = 0;

	if (Status == USB_ST_NOERROR)
	{
		RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
	}
	else 
	{
		if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("Bulk Out PSPoll Failed\n"));
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
			pAd->bulkResetPipeid = (MGMTPIPEIDX | BULKOUT_MGMT_RESET_FLAG);
			RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_OUT, NULL, 0);
		}
	}

	RTMP_SEM_LOCK(&pAd->BulkOutLock[0]);
	pAd->BulkOutPending[0] = FALSE;
	RTMP_SEM_UNLOCK(&pAd->BulkOutLock[0]);

	
	
	RTUSBKickBulkOut(pAd);

}


static void rt2870_dataout_complete_tasklet(unsigned long data)
{
	PRTMP_ADAPTER		pAd;
	purbb_t				pUrb;
	POS_COOKIE			pObj;
	PHT_TX_CONTEXT		pHTTXContext;
	UCHAR				BulkOutPipeId;
	NTSTATUS			Status;
	unsigned long		IrqFlags;


	pUrb			= (purbb_t)data;
	pHTTXContext	= (PHT_TX_CONTEXT)pUrb->context;
	pAd				= pHTTXContext->pAd;
	pObj 			= (POS_COOKIE) pAd->OS_Cookie;
	Status			= pUrb->status;

	
	BulkOutPipeId = pHTTXContext->BulkOutPipeId;
	pAd->BulkOutDataOneSecCount++;

	
	

	RTMP_IRQ_LOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);
	pAd->BulkOutPending[BulkOutPipeId] = FALSE;
	pHTTXContext->IRPPending = FALSE;
	pAd->watchDogTxPendingCnt[BulkOutPipeId] = 0;

	if (Status == USB_ST_NOERROR)
	{
		pAd->BulkOutComplete++;

		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);

		pAd->Counters8023.GoodTransmits++;
		
		FREE_HTTX_RING(pAd, BulkOutPipeId, pHTTXContext);
		


	}
	else	
	{
		PUCHAR	pBuf;

		pAd->BulkOutCompleteOther++;

		pBuf = &pHTTXContext->TransferBuffer->field.WirelessPacket[pHTTXContext->NextBulkOutPosition];

		if (!RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
									fRTMP_ADAPTER_HALT_IN_PROGRESS |
									fRTMP_ADAPTER_NIC_NOT_EXIST |
									fRTMP_ADAPTER_BULKOUT_RESET)))
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET);
			pAd->bulkResetPipeid = BulkOutPipeId;
			pAd->bulkResetReq[BulkOutPipeId] = pAd->BulkOutReq;
		}
		RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[BulkOutPipeId], IrqFlags);

		DBGPRINT_RAW(RT_DEBUG_ERROR, ("BulkOutDataPacket failed: ReasonCode=%d!\n", Status));
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("\t>>BulkOut Req=0x%lx, Complete=0x%lx, Other=0x%lx\n", pAd->BulkOutReq, pAd->BulkOutComplete, pAd->BulkOutCompleteOther));
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("\t>>BulkOut Header:%x %x %x %x %x %x %x %x\n", pBuf[0], pBuf[1], pBuf[2], pBuf[3], pBuf[4], pBuf[5], pBuf[6], pBuf[7]));
		

	}

	
	
	
	
	
	if ((pHTTXContext->ENextBulkOutPosition != pHTTXContext->CurWritePosition) &&
		(pHTTXContext->ENextBulkOutPosition != (pHTTXContext->CurWritePosition+8)) &&
		!RTUSB_TEST_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_FRAG << BulkOutPipeId)))
	{
		
		RTUSB_SET_BULK_FLAG(pAd, (fRTUSB_BULK_OUT_DATA_NORMAL << BulkOutPipeId));
	}
	

	
	
	RTUSBKickBulkOut(pAd);
}


