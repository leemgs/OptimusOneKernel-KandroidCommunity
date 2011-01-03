

#include "rt_config.h"





MODULE_AUTHOR("Paul Lin <paul_lin@ralinktech.com>");
MODULE_DESCRIPTION(RT28xx_CHIP_NAME " Wireless LAN Linux Driver");
MODULE_LICENSE("GPL");
#ifdef MODULE_VERSION
MODULE_VERSION(STA_DRIVER_VERSION);
#endif
MODULE_ALIAS("rt3070sta");



extern INT __devinit rt28xx_probe(IN void *_dev_p, IN void *_dev_id_p,
									IN UINT argc, OUT PRTMP_ADAPTER *ppAd);

struct usb_device_id rtusb_usb_id[] = {
	{ USB_DEVICE(0x148F, 0x2770) }, 
	{ USB_DEVICE(0x1737, 0x0071) }, 
	{ USB_DEVICE(0x1737, 0x0070) }, 
	{ USB_DEVICE(0x148F, 0x2870) }, 
	{ USB_DEVICE(0x148F, 0x3070) }, 
	{ USB_DEVICE(0x148F, 0x3071) }, 
	{ USB_DEVICE(0x148F, 0x3072) }, 
	{ USB_DEVICE(0x0B05, 0x1731) }, 
	{ USB_DEVICE(0x0B05, 0x1732) }, 
	{ USB_DEVICE(0x0B05, 0x1742) }, 
	{ USB_DEVICE(0x0DF6, 0x0017) }, 
	{ USB_DEVICE(0x0DF6, 0x002B) }, 
	{ USB_DEVICE(0x0DF6, 0x002C) }, 
	{ USB_DEVICE(0x0DF6, 0x003E) }, 
	{ USB_DEVICE(0x0DF6, 0x002D) }, 
	{ USB_DEVICE(0x0DF6, 0x0039) }, 
	{ USB_DEVICE(0x0DF6, 0x003F) }, 
	{ USB_DEVICE(0x14B2, 0x3C06) }, 
	{ USB_DEVICE(0x14B2, 0x3C28) }, 
	{ USB_DEVICE(0x2019, 0xED06) }, 
	{ USB_DEVICE(0x2019, 0xED14) }, 
	{ USB_DEVICE(0x2019, 0xAB25) }, 
	{ USB_DEVICE(0x07D1, 0x3C09) }, 
	{ USB_DEVICE(0x07D1, 0x3C11) }, 
	{ USB_DEVICE(0x2001, 0x3C09) }, 
	{ USB_DEVICE(0x2001, 0x3C0A) }, 
	{ USB_DEVICE(0x14B2, 0x3C07) }, 
	{ USB_DEVICE(0x14B2, 0x3C12) }, 
	{ USB_DEVICE(0x050D, 0x8053) }, 
	{ USB_DEVICE(0x050D, 0x815C) }, 
	{ USB_DEVICE(0x050D, 0x825a) }, 
	{ USB_DEVICE(0x14B2, 0x3C23) }, 
	{ USB_DEVICE(0x14B2, 0x3C27) }, 
	{ USB_DEVICE(0x07AA, 0x002F) }, 
	{ USB_DEVICE(0x07AA, 0x003C) }, 
	{ USB_DEVICE(0x07AA, 0x003F) }, 
	{ USB_DEVICE(0x18C5, 0x0012) }, 
	{ USB_DEVICE(0x1044, 0x800B) }, 
	{ USB_DEVICE(0x1044, 0x800D) }, 
	{ USB_DEVICE(0x15A9, 0x0006) }, 
	{ USB_DEVICE(0x083A, 0xB522) }, 
	{ USB_DEVICE(0x083A, 0xA618) }, 
	{ USB_DEVICE(0x083A, 0x8522) }, 
	{ USB_DEVICE(0x083A, 0x7512) }, 
	{ USB_DEVICE(0x083A, 0x7522) }, 
	{ USB_DEVICE(0x083A, 0x7511) }, 
	{ USB_DEVICE(0x0CDE, 0x0022) }, 
	{ USB_DEVICE(0x0586, 0x3416) }, 
	{ USB_DEVICE(0x0CDE, 0x0025) }, 
	{ USB_DEVICE(0x1740, 0x9701) }, 
	{ USB_DEVICE(0x1740, 0x9702) }, 
	{ USB_DEVICE(0x1740, 0x9703) }, 
	{ USB_DEVICE(0x0471, 0x200f) }, 
	{ USB_DEVICE(0x14B2, 0x3C25) }, 
	{ USB_DEVICE(0x13D3, 0x3247) }, 
	{ USB_DEVICE(0x13D3, 0x3273) }, 
	{ USB_DEVICE(0x083A, 0x6618) }, 
	{ USB_DEVICE(0x15c5, 0x0008) }, 
	{ USB_DEVICE(0x0E66, 0x0001) }, 
	{ USB_DEVICE(0x0E66, 0x0003) }, 
	{ USB_DEVICE(0x129B, 0x1828) }, 
	{ USB_DEVICE(0x157E, 0x300E) }, 
	{ USB_DEVICE(0x050d, 0x805c) },
	{ USB_DEVICE(0x1482, 0x3C09) }, 
	{ USB_DEVICE(0x14B2, 0x3C09) }, 
	{ USB_DEVICE(0x04E8, 0x2018) }, 
	{ USB_DEVICE(0x07B8, 0x3070) }, 
	{ USB_DEVICE(0x07B8, 0x3071) }, 
	{ USB_DEVICE(0x07B8, 0x2870) }, 
	{ USB_DEVICE(0x07B8, 0x2770) }, 
	{ USB_DEVICE(0x07B8, 0x3072) }, 
	{ USB_DEVICE(0x7392, 0x7711) }, 
	{ USB_DEVICE(0x5A57, 0x0280) }, 
	{ USB_DEVICE(0x5A57, 0x0282) }, 
	{ USB_DEVICE(0x1A32, 0x0304) }, 
	{ USB_DEVICE(0x0789, 0x0162) }, 
	{ USB_DEVICE(0x0789, 0x0163) }, 
	{ USB_DEVICE(0x0789, 0x0164) }, 
	{ USB_DEVICE(0x7392, 0x7717) }, 
	{ USB_DEVICE(0x1EDA, 0x2310) }, 
	{ USB_DEVICE(0x1737, 0x0077) }, 
	{ } 
};

INT const               rtusb_usb_id_len = sizeof(rtusb_usb_id) / sizeof(struct usb_device_id);
MODULE_DEVICE_TABLE(usb, rtusb_usb_id);

#ifndef PF_NOFREEZE
#define PF_NOFREEZE  0
#endif


#ifdef CONFIG_PM
static int rt2870_suspend(struct usb_interface *intf, pm_message_t state);
static int rt2870_resume(struct usb_interface *intf);
#endif 






static int rtusb_probe (struct usb_interface *intf,
						const struct usb_device_id *id);
static void rtusb_disconnect(struct usb_interface *intf);

struct usb_driver rtusb_driver = {
	.name="rt2870",
	.probe=rtusb_probe,
	.disconnect=rtusb_disconnect,
	.id_table=rtusb_usb_id,

#ifdef CONFIG_PM
	suspend:	rt2870_suspend,
	resume:		rt2870_resume,
#endif
	};

#ifdef CONFIG_PM

VOID RT2860RejectPendingPackets(
	IN	PRTMP_ADAPTER	pAd)
{
	
	
}

static int rt2870_suspend(
	struct usb_interface *intf,
	pm_message_t state)
{
	struct net_device *net_dev;
	PRTMP_ADAPTER pAd = usb_get_intfdata(intf);


	DBGPRINT(RT_DEBUG_TRACE, ("===> rt2870_suspend()\n"));
	net_dev = pAd->net_dev;
	netif_device_detach (net_dev);

	pAd->PM_FlgSuspend = 1;
	if (netif_running(net_dev)) {
		RTUSBCancelPendingBulkInIRP(pAd);
		RTUSBCancelPendingBulkOutIRP(pAd);
	}
	DBGPRINT(RT_DEBUG_TRACE, ("<=== rt2870_suspend()\n"));
	return 0;
}

static int rt2870_resume(
	struct usb_interface *intf)
{
	struct net_device *net_dev;
	PRTMP_ADAPTER pAd = usb_get_intfdata(intf);


	DBGPRINT(RT_DEBUG_TRACE, ("===> rt2870_resume()\n"));

	pAd->PM_FlgSuspend = 0;
	net_dev = pAd->net_dev;
	netif_device_attach (net_dev);
	netif_start_queue(net_dev);
	netif_carrier_on(net_dev);
	netif_wake_queue(net_dev);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== rt2870_resume()\n"));
	return 0;
}
#endif 



INT __init rtusb_init(void)
{
	printk("rtusb init --->\n");
	return usb_register(&rtusb_driver);
}


VOID __exit rtusb_exit(void)
{
	usb_deregister(&rtusb_driver);
	printk("<--- rtusb exit\n");
}

module_init(rtusb_init);
module_exit(rtusb_exit);









INT MlmeThread(
	IN void *Context)
{
	PRTMP_ADAPTER	pAd = (PRTMP_ADAPTER)Context;
	POS_COOKIE	pObj;
	int status;

	pObj = (POS_COOKIE)pAd->OS_Cookie;

	rtmp_os_thread_init("rt2870MlmeThread", (PVOID)&(pAd->mlmeComplete));

	while (pAd->mlme_kill == 0)
	{
		
		
		status = down_interruptible(&(pAd->mlme_semaphore));

		
		

		if (!pAd->PM_FlgSuspend)
		MlmeHandler(pAd);

		
		
		if (status != 0)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}
	}

	
	DBGPRINT(RT_DEBUG_TRACE,( "<---%s\n",__func__));

	pObj->MLMEThr_pid = NULL;

	complete_and_exit (&pAd->mlmeComplete, 0);
	return 0;

}



INT RTUSBCmdThread(
	IN void * Context)
{
	PRTMP_ADAPTER	pAd = (PRTMP_ADAPTER)Context;
	POS_COOKIE		pObj;
	int status;

	pObj = (POS_COOKIE)pAd->OS_Cookie;

	rtmp_os_thread_init("rt2870CmdThread", (PVOID)&(pAd->CmdQComplete));

	NdisAcquireSpinLock(&pAd->CmdQLock);
	pAd->CmdQ.CmdQState = RT2870_THREAD_RUNNING;
	NdisReleaseSpinLock(&pAd->CmdQLock);

	while (pAd->CmdQ.CmdQState == RT2870_THREAD_RUNNING)
	{
		
		
		status = down_interruptible(&(pAd->RTUSBCmd_semaphore));

		if (pAd->CmdQ.CmdQState == RT2870_THREAD_STOPED)
			break;

		if (status != 0)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}
		
		

		if (!pAd->PM_FlgSuspend)
		CMDHandler(pAd);

		
		
	}

	if (!pAd->PM_FlgSuspend)
	{	
		CmdQElmt	*pCmdQElmt = NULL;

		NdisAcquireSpinLock(&pAd->CmdQLock);
		pAd->CmdQ.CmdQState = RT2870_THREAD_STOPED;
		while(pAd->CmdQ.size)
		{
			RTUSBDequeueCmd(&pAd->CmdQ, &pCmdQElmt);
			if (pCmdQElmt)
			{
				if (pCmdQElmt->CmdFromNdis == TRUE)
				{
					if (pCmdQElmt->buffer != NULL)
						NdisFreeMemory(pCmdQElmt->buffer, pCmdQElmt->bufferlength, 0);

					NdisFreeMemory(pCmdQElmt, sizeof(CmdQElmt), 0);
				}
				else
				{
					if ((pCmdQElmt->buffer != NULL) && (pCmdQElmt->bufferlength != 0))
						NdisFreeMemory(pCmdQElmt->buffer, pCmdQElmt->bufferlength, 0);
		            {
						NdisFreeMemory(pCmdQElmt, sizeof(CmdQElmt), 0);
					}
				}
			}
		}

		NdisReleaseSpinLock(&pAd->CmdQLock);
	}
	
	DBGPRINT(RT_DEBUG_TRACE,( "<---RTUSBCmdThread\n"));

	pObj->RTUSBCmdThr_pid = NULL;

	complete_and_exit (&pAd->CmdQComplete, 0);
	return 0;

}


static void RT2870_TimerQ_Handle(RTMP_ADAPTER *pAd)
{
	int status;
	RALINK_TIMER_STRUCT	*pTimer;
	RT2870_TIMER_ENTRY	*pEntry;
	unsigned long	irqFlag;

	while(!pAd->TimerFunc_kill)
	{

		pTimer = NULL;

		status = down_interruptible(&(pAd->RTUSBTimer_semaphore));

		if (pAd->TimerQ.status == RT2870_THREAD_STOPED)
			break;

		
		while(pAd->TimerQ.pQHead)
		{
			RTMP_IRQ_LOCK(&pAd->TimerQLock, irqFlag);
			pEntry = pAd->TimerQ.pQHead;
			if (pEntry)
			{
				pTimer = pEntry->pRaTimer;

				
				pAd->TimerQ.pQHead = pEntry->pNext;
				if (pEntry == pAd->TimerQ.pQTail)
					pAd->TimerQ.pQTail = NULL;

				
				pEntry->pNext = pAd->TimerQ.pQPollFreeList;
				pAd->TimerQ.pQPollFreeList = pEntry;
			}
			RTMP_IRQ_UNLOCK(&pAd->TimerQLock, irqFlag);

			if (pTimer)
			{
				if (pTimer->handle != NULL)
				if (!pAd->PM_FlgSuspend)
					pTimer->handle(NULL, (PVOID) pTimer->cookie, NULL, pTimer);
				if ((pTimer->Repeat) && (pTimer->State == FALSE))
					RTMP_OS_Add_Timer(&pTimer->TimerObj, pTimer->TimerValue);
			}
		}

		if (status != 0)
		{
			pAd->TimerQ.status = RT2870_THREAD_STOPED;
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}
	}
}


INT TimerQThread(
	IN OUT PVOID Context)
{
	PRTMP_ADAPTER	pAd;
	POS_COOKIE	pObj;

	pAd = (PRTMP_ADAPTER)Context;
	pObj = (POS_COOKIE) pAd->OS_Cookie;

	rtmp_os_thread_init("rt2870TimerQHandle", (PVOID)&(pAd->TimerQComplete));

	RT2870_TimerQ_Handle(pAd);

	
	DBGPRINT(RT_DEBUG_TRACE,( "<---%s\n",__func__));

	pObj->TimerQThr_pid = NULL;

	complete_and_exit(&pAd->TimerQComplete, 0);
	return 0;

}


RT2870_TIMER_ENTRY *RT2870_TimerQ_Insert(
	IN RTMP_ADAPTER *pAd,
	IN RALINK_TIMER_STRUCT *pTimer)
{
	RT2870_TIMER_ENTRY *pQNode = NULL, *pQTail;
	unsigned long irqFlags;


	RTMP_IRQ_LOCK(&pAd->TimerQLock, irqFlags);
	if (pAd->TimerQ.status & RT2870_THREAD_CAN_DO_INSERT)
	{
		if(pAd->TimerQ.pQPollFreeList)
		{
			pQNode = pAd->TimerQ.pQPollFreeList;
			pAd->TimerQ.pQPollFreeList = pQNode->pNext;

			pQNode->pRaTimer = pTimer;
			pQNode->pNext = NULL;

			pQTail = pAd->TimerQ.pQTail;
			if (pAd->TimerQ.pQTail != NULL)
				pQTail->pNext = pQNode;
			pAd->TimerQ.pQTail = pQNode;
			if (pAd->TimerQ.pQHead == NULL)
				pAd->TimerQ.pQHead = pQNode;
		}
		RTMP_IRQ_UNLOCK(&pAd->TimerQLock, irqFlags);

		if (pQNode)
			up(&pAd->RTUSBTimer_semaphore);
			
	}
	else
	{
		RTMP_IRQ_UNLOCK(&pAd->TimerQLock, irqFlags);
	}
	return pQNode;
}


BOOLEAN RT2870_TimerQ_Remove(
	IN RTMP_ADAPTER *pAd,
	IN RALINK_TIMER_STRUCT *pTimer)
{
	RT2870_TIMER_ENTRY *pNode, *pPrev = NULL;
	unsigned long irqFlags;

	RTMP_IRQ_LOCK(&pAd->TimerQLock, irqFlags);
	if (pAd->TimerQ.status >= RT2870_THREAD_INITED)
	{
		pNode = pAd->TimerQ.pQHead;
		while (pNode)
		{
			if (pNode->pRaTimer == pTimer)
				break;
			pPrev = pNode;
			pNode = pNode->pNext;
		}

		
		if (pNode)
		{
			if (pNode == pAd->TimerQ.pQHead)
				pAd->TimerQ.pQHead = pNode->pNext;
			if (pNode == pAd->TimerQ.pQTail)
				pAd->TimerQ.pQTail = pPrev;
			if (pPrev != NULL)
				pPrev->pNext = pNode->pNext;

			
			pNode->pNext = pAd->TimerQ.pQPollFreeList;
			pAd->TimerQ.pQPollFreeList = pNode;
		}
	}
	RTMP_IRQ_UNLOCK(&pAd->TimerQLock, irqFlags);

	return TRUE;
}


void RT2870_TimerQ_Exit(RTMP_ADAPTER *pAd)
{
	RT2870_TIMER_ENTRY *pTimerQ;
	unsigned long irqFlags;

	RTMP_IRQ_LOCK(&pAd->TimerQLock, irqFlags);
	while (pAd->TimerQ.pQHead)
	{
		pTimerQ = pAd->TimerQ.pQHead;
		pAd->TimerQ.pQHead = pTimerQ->pNext;
		
	}
	pAd->TimerQ.pQPollFreeList = NULL;
	os_free_mem(pAd, pAd->TimerQ.pTimerQPoll);
	pAd->TimerQ.pQTail = NULL;
	pAd->TimerQ.pQHead = NULL;
	pAd->TimerQ.status = RT2870_THREAD_STOPED;
	RTMP_IRQ_UNLOCK(&pAd->TimerQLock, irqFlags);

}


void RT2870_TimerQ_Init(RTMP_ADAPTER *pAd)
{
	int 	i;
	RT2870_TIMER_ENTRY *pQNode, *pEntry;
	unsigned long irqFlags;

	NdisAllocateSpinLock(&pAd->TimerQLock);

	RTMP_IRQ_LOCK(&pAd->TimerQLock, irqFlags);
	NdisZeroMemory(&pAd->TimerQ, sizeof(pAd->TimerQ));
	

	
	

	os_alloc_mem(pAd, &pAd->TimerQ.pTimerQPoll, sizeof(RT2870_TIMER_ENTRY) * TIMER_QUEUE_SIZE_MAX);
	if (pAd->TimerQ.pTimerQPoll)
	{
		pEntry = NULL;
		pQNode = (RT2870_TIMER_ENTRY *)pAd->TimerQ.pTimerQPoll;
		for (i = 0 ;i <TIMER_QUEUE_SIZE_MAX; i++)
		{
			pQNode->pNext = pEntry;
			pEntry = pQNode;
			pQNode++;
		}
		pAd->TimerQ.pQPollFreeList = pEntry;
		pAd->TimerQ.pQHead = NULL;
		pAd->TimerQ.pQTail = NULL;
		pAd->TimerQ.status = RT2870_THREAD_INITED;
	}
	RTMP_IRQ_UNLOCK(&pAd->TimerQLock, irqFlags);
}


VOID RT2870_WatchDog(IN RTMP_ADAPTER *pAd)
{
	PHT_TX_CONTEXT		pHTTXContext;
	int 					idx;
	ULONG				irqFlags;
	PURB		   		pUrb;
	BOOLEAN				needDumpSeq = FALSE;
	UINT32          	MACValue;


	idx = 0;
	RTMP_IO_READ32(pAd, TXRXQ_PCNT, &MACValue);
	if ((MACValue & 0xff) !=0 )
	{
		DBGPRINT(RT_DEBUG_TRACE, ("TX QUEUE 0 Not EMPTY(Value=0x%0x). !!!!!!!!!!!!!!!\n", MACValue));
		RTMP_IO_WRITE32(pAd, PBF_CFG, 0xf40012);
		while((MACValue &0xff) != 0 && (idx++ < 10))
		{
		        RTMP_IO_READ32(pAd, TXRXQ_PCNT, &MACValue);
		        NdisMSleep(1);
		}
		RTMP_IO_WRITE32(pAd, PBF_CFG, 0xf40006);
	}


	idx = 0;
	if ((MACValue & 0xff00) !=0 )
	{
		DBGPRINT(RT_DEBUG_TRACE, ("TX QUEUE 1 Not EMPTY(Value=0x%0x). !!!!!!!!!!!!!!!\n", MACValue));
		RTMP_IO_WRITE32(pAd, PBF_CFG, 0xf4000a);
		while((MACValue &0xff00) != 0 && (idx++ < 10))
		{
			RTMP_IO_READ32(pAd, TXRXQ_PCNT, &MACValue);
			NdisMSleep(1);
		}
		RTMP_IO_WRITE32(pAd, PBF_CFG, 0xf40006);
	}

	if (pAd->watchDogRxOverFlowCnt >= 2)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Maybe the Rx Bulk-In hanged! Cancel the pending Rx bulks request!\n"));
		if ((!RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
									fRTMP_ADAPTER_BULKIN_RESET |
									fRTMP_ADAPTER_HALT_IN_PROGRESS |
									fRTMP_ADAPTER_NIC_NOT_EXIST))))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Call CMDTHREAD_RESET_BULK_IN to cancel the pending Rx Bulk!\n"));
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BULKIN_RESET);
			RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_RESET_BULK_IN, NULL, 0);
			needDumpSeq = TRUE;
		}
		pAd->watchDogRxOverFlowCnt = 0;
	}


	for (idx = 0; idx < NUM_OF_TX_RING; idx++)
	{
		pUrb = NULL;

		RTMP_IRQ_LOCK(&pAd->BulkOutLock[idx], irqFlags);
		if ((pAd->BulkOutPending[idx] == TRUE) && pAd->watchDogTxPendingCnt)
		{
			pAd->watchDogTxPendingCnt[idx]++;

			if ((pAd->watchDogTxPendingCnt[idx] > 2) &&
				 (!RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS | fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_BULKOUT_RESET)))
				)
			{
				
				pHTTXContext = (PHT_TX_CONTEXT)(&pAd->TxContext[idx]);
				if (pHTTXContext->IRPPending)
				{	
					pUrb = pHTTXContext->pUrb;
				}
				else if (idx == MGMTPIPEIDX)
				{
					PTX_CONTEXT pMLMEContext, pNULLContext, pPsPollContext;

					
					pMLMEContext = (PTX_CONTEXT)(pAd->MgmtRing.Cell[pAd->MgmtRing.TxDmaIdx].AllocVa);
					pPsPollContext = (PTX_CONTEXT)(&pAd->PsPollContext);
					pNULLContext = (PTX_CONTEXT)(&pAd->NullContext);

					if (pMLMEContext->IRPPending)
					{
						ASSERT(pMLMEContext->IRPPending);
						pUrb = pMLMEContext->pUrb;
					}
					else if (pNULLContext->IRPPending)
					{
						ASSERT(pNULLContext->IRPPending);
						pUrb = pNULLContext->pUrb;
					}
					else if (pPsPollContext->IRPPending)
					{
						ASSERT(pPsPollContext->IRPPending);
						pUrb = pPsPollContext->pUrb;
					}
				}

				RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[idx], irqFlags);

				DBGPRINT(RT_DEBUG_TRACE, ("Maybe the Tx Bulk-Out hanged! Cancel the pending Tx bulks request of idx(%d)!\n", idx));
				if (pUrb)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("Unlink the pending URB!\n"));
					
					RTUSB_UNLINK_URB(pUrb);
					
					RTMPusecDelay(200);
					needDumpSeq = TRUE;
				}
				else
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Unkonw bulkOut URB maybe hanged!!!!!!!!!!!!\n"));
				}
			}
			else
			{
				RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[idx], irqFlags);
			}
		}
		else
		{
			RTMP_IRQ_UNLOCK(&pAd->BulkOutLock[idx], irqFlags);
		}
	}

	
	if((needDumpSeq == TRUE) && (pAd->CommonCfg.bDisableReordering == 0))
	{
		USHORT				Idx;
		PBA_REC_ENTRY		pBAEntry = NULL;
		UCHAR				count = 0;
		struct reordering_mpdu *mpdu_blk;

		Idx = pAd->MacTab.Content[BSSID_WCID].BARecWcidArray[0];

		pBAEntry = &pAd->BATable.BARecEntry[Idx];
		if((pBAEntry->list.qlen > 0) && (pBAEntry->list.next != NULL))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("NICUpdateRawCounters():The Queueing pkt in reordering buffer:\n"));
			NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
			mpdu_blk = pBAEntry->list.next;
			while (mpdu_blk)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("\t%d:Seq-%d, bAMSDU-%d!\n", count, mpdu_blk->Sequence, mpdu_blk->bAMSDU));
				mpdu_blk = mpdu_blk->next;
				count++;
			}

			DBGPRINT(RT_DEBUG_TRACE, ("\npBAEntry->LastIndSeq=%d!\n", pBAEntry->LastIndSeq));
			NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
		}
	}
}


static void _rtusb_disconnect(struct usb_device *dev, PRTMP_ADAPTER pAd)
{
	struct net_device	*net_dev = NULL;


	DBGPRINT(RT_DEBUG_ERROR, ("rtusb_disconnect: unregister usbnet usb-%s-%s\n",
				dev->bus->bus_name, dev->devpath));
	if (!pAd)
	{
		usb_put_dev(dev);

		printk("rtusb_disconnect: pAd == NULL!\n");
		return;
	}
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);



	
	udelay(1);




	net_dev = pAd->net_dev;
	if (pAd->net_dev != NULL)
	{
		printk("rtusb_disconnect: unregister_netdev(), dev->name=%s!\n", net_dev->name);
		unregister_netdev (pAd->net_dev);
	}
	udelay(1);
	flush_scheduled_work();
	udelay(1);

	
	free_netdev(net_dev);

	
	RTMPFreeAdapter(pAd);

	
	usb_put_dev(dev);
	udelay(1);

	DBGPRINT(RT_DEBUG_ERROR, (" RTUSB disconnect successfully\n"));
}



static int rtusb_probe (struct usb_interface *intf,
						const struct usb_device_id *id)
{
	PRTMP_ADAPTER pAd;
	return (int)rt28xx_probe((void *)intf, (void *)id, 0, &pAd);
}


static void rtusb_disconnect(struct usb_interface *intf)
{
	struct usb_device   *dev = interface_to_usbdev(intf);
	PRTMP_ADAPTER       pAd;


	pAd = usb_get_intfdata(intf);
	usb_set_intfdata(intf, NULL);

	_rtusb_disconnect(dev, pAd);
}



VOID RT28xxThreadTerminate(
	IN RTMP_ADAPTER *pAd)
{
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT			ret;


	
	RTMPusecDelay(50000);

	
	
	
	RTUSBCancelPendingIRPs(pAd);

	

	if (pid_nr(pObj->TimerQThr_pid) > 0)
	{
		POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;

		printk("Terminate the TimerQThr_pid=%d!\n", pid_nr(pObj->TimerQThr_pid));
		mb();
		pAd->TimerFunc_kill = 1;
		mb();
		ret = kill_pid(pObj->TimerQThr_pid, SIGTERM, 1);
		if (ret)
		{
			printk(KERN_WARNING "%s: unable to stop TimerQThread, pid=%d, ret=%d!\n",
					pAd->net_dev->name, pid_nr(pObj->TimerQThr_pid), ret);
		}
		else
		{
			wait_for_completion(&pAd->TimerQComplete);
			pObj->TimerQThr_pid = NULL;
		}
	}

	if (pid_nr(pObj->MLMEThr_pid) > 0)
	{
		printk("Terminate the MLMEThr_pid=%d!\n", pid_nr(pObj->MLMEThr_pid));
		mb();
		pAd->mlme_kill = 1;
		
		mb();
		ret = kill_pid(pObj->MLMEThr_pid, SIGTERM, 1);
		if (ret)
		{
			printk (KERN_WARNING "%s: unable to Mlme thread, pid=%d, ret=%d!\n",
					pAd->net_dev->name, pid_nr(pObj->MLMEThr_pid), ret);
		}
		else
		{
			
			wait_for_completion (&pAd->mlmeComplete);
			pObj->MLMEThr_pid = NULL;
		}
	}

	if (pid_nr(pObj->RTUSBCmdThr_pid) > 0)
	{
		printk("Terminate the RTUSBCmdThr_pid=%d!\n", pid_nr(pObj->RTUSBCmdThr_pid));
		mb();
		NdisAcquireSpinLock(&pAd->CmdQLock);
		pAd->CmdQ.CmdQState = RT2870_THREAD_STOPED;
		NdisReleaseSpinLock(&pAd->CmdQLock);
		mb();
		
		ret = kill_pid(pObj->RTUSBCmdThr_pid, SIGTERM, 1);
		if (ret)
		{
			printk(KERN_WARNING "%s: unable to RTUSBCmd thread, pid=%d, ret=%d!\n",
					pAd->net_dev->name, pid_nr(pObj->RTUSBCmdThr_pid), ret);
		}
		else
		{
			
			wait_for_completion (&pAd->CmdQComplete);
			pObj->RTUSBCmdThr_pid = NULL;
		}
	}

	
	pAd->mlme_kill = 0;
	pAd->CmdQ.CmdQState = RT2870_THREAD_UNKNOWN;
	pAd->TimerFunc_kill = 0;
}


void kill_thread_task(IN PRTMP_ADAPTER pAd)
{
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	tasklet_kill(&pObj->rx_done_task);
	tasklet_kill(&pObj->mgmt_dma_done_task);
	tasklet_kill(&pObj->ac0_dma_done_task);
	tasklet_kill(&pObj->ac1_dma_done_task);
	tasklet_kill(&pObj->ac2_dma_done_task);
	tasklet_kill(&pObj->ac3_dma_done_task);
	tasklet_kill(&pObj->hcca_dma_done_task);
	tasklet_kill(&pObj->tbtt_task);

}



BOOLEAN RT28XXChipsetCheck(
	IN void *_dev_p)
{
	struct usb_interface *intf = (struct usb_interface *)_dev_p;
	struct usb_device *dev_p = interface_to_usbdev(intf);
	UINT32 i;


	for(i=0; i<rtusb_usb_id_len; i++)
	{
		if (dev_p->descriptor.idVendor == rtusb_usb_id[i].idVendor &&
			dev_p->descriptor.idProduct == rtusb_usb_id[i].idProduct)
		{
			printk("rt2870: idVendor = 0x%x, idProduct = 0x%x\n",
					dev_p->descriptor.idVendor, dev_p->descriptor.idProduct);
			break;
		}
	}

	if (i == rtusb_usb_id_len)
	{
		printk("rt2870: Error! Device Descriptor not matching!\n");
		return FALSE;
	}

	return TRUE;
}



BOOLEAN RT28XXNetDevInit(
	IN void 				*_dev_p,
	IN struct  net_device	*net_dev,
	IN RTMP_ADAPTER 		*pAd)
{
	struct usb_interface *intf = (struct usb_interface *)_dev_p;
	struct usb_device *dev_p = interface_to_usbdev(intf);


	pAd->config = &dev_p->config->desc;
	return TRUE;
}



BOOLEAN RT28XXProbePostConfig(
	IN void 				*_dev_p,
	IN RTMP_ADAPTER 		*pAd,
	IN INT32				interface)
{
	struct usb_interface *intf = (struct usb_interface *)_dev_p;
	struct usb_host_interface *iface_desc;
	ULONG BulkOutIdx;
	UINT32 i;


	
	iface_desc = intf->cur_altsetting;

	
	pAd->NumberOfPipes = iface_desc->desc.bNumEndpoints;
	DBGPRINT(RT_DEBUG_TRACE,
			("NumEndpoints=%d\n", iface_desc->desc.bNumEndpoints));

	
	BulkOutIdx = 0;

	for(i=0; i<pAd->NumberOfPipes; i++)
	{
		if ((iface_desc->endpoint[i].desc.bmAttributes ==
				USB_ENDPOINT_XFER_BULK) &&
			((iface_desc->endpoint[i].desc.bEndpointAddress &
				USB_ENDPOINT_DIR_MASK) == USB_DIR_IN))
		{
			pAd->BulkInEpAddr = iface_desc->endpoint[i].desc.bEndpointAddress;
			pAd->BulkInMaxPacketSize = iface_desc->endpoint[i].desc.wMaxPacketSize;

			DBGPRINT_RAW(RT_DEBUG_TRACE,
				("BULK IN MaximumPacketSize = %d\n", pAd->BulkInMaxPacketSize));
			DBGPRINT_RAW(RT_DEBUG_TRACE,
				("EP address = 0x%2x\n", iface_desc->endpoint[i].desc.bEndpointAddress));
		}
		else if ((iface_desc->endpoint[i].desc.bmAttributes ==
					USB_ENDPOINT_XFER_BULK) &&
				((iface_desc->endpoint[i].desc.bEndpointAddress &
					USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT))
		{
			
			
			pAd->BulkOutEpAddr[BulkOutIdx++] = iface_desc->endpoint[i].desc.bEndpointAddress;
			pAd->BulkOutMaxPacketSize = iface_desc->endpoint[i].desc.wMaxPacketSize;

			DBGPRINT_RAW(RT_DEBUG_TRACE,
				("BULK OUT MaximumPacketSize = %d\n", pAd->BulkOutMaxPacketSize));
			DBGPRINT_RAW(RT_DEBUG_TRACE,
				("EP address = 0x%2x  \n", iface_desc->endpoint[i].desc.bEndpointAddress));
		}
	}

	if (!(pAd->BulkInEpAddr && pAd->BulkOutEpAddr[0]))
	{
		printk("%s: Could not find both bulk-in and bulk-out endpoints\n", __func__);
		return FALSE;
	}

	return TRUE;
}



VOID RT28XXDMADisable(
	IN RTMP_ADAPTER 		*pAd)
{
	
}




VOID RT28XXDMAEnable(
	IN RTMP_ADAPTER 		*pAd)
{
	WPDMA_GLO_CFG_STRUC	GloCfg;
	USB_DMA_CFG_STRUC	UsbCfg;
	int					i = 0;


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
	GloCfg.field.EnableRxDMA = 1;
	GloCfg.field.EnableTxDMA = 1;
	DBGPRINT(RT_DEBUG_TRACE, ("<== WRITE DMA offset 0x208 = 0x%x\n", GloCfg.word));
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);

	UsbCfg.word = 0;
	UsbCfg.field.phyclear = 0;
	
	if (pAd->BulkInMaxPacketSize == 512)
			UsbCfg.field.RxBulkAggEn = 1;
	
	UsbCfg.field.RxBulkAggLmt = (MAX_RXBULK_SIZE /1024)-3;
	UsbCfg.field.RxBulkAggTOut = 0x80; 
	UsbCfg.field.RxBulkEn = 1;
	UsbCfg.field.TxBulkEn = 1;

	RTUSBWriteMACRegister(pAd, USB_DMA_CFG, UsbCfg.word);

}


VOID RT28xx_UpdateBeaconToAsic(
	IN RTMP_ADAPTER		*pAd,
	IN INT				apidx,
	IN ULONG			FrameLen,
	IN ULONG			UpdatePos)
{
	PUCHAR        	pBeaconFrame = NULL;
	UCHAR  			*ptr;
	UINT  			i, padding;
	BEACON_SYNC_STRUCT	*pBeaconSync = pAd->CommonCfg.pBeaconSync;
	UINT32			longValue;
	BOOLEAN			bBcnReq = FALSE;
	UCHAR			bcn_idx = 0;


	if (pBeaconFrame == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR,("pBeaconFrame is NULL!\n"));
		return;
	}

	if (pBeaconSync == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR,("pBeaconSync is NULL!\n"));
		return;
	}

	
	
	
	if (bBcnReq == FALSE)
	{
		
		
		for(i=0; i<TXWI_SIZE; i+=4) {
			RTMP_IO_WRITE32(pAd, pAd->BeaconOffset[bcn_idx] + i, 0x00);
		}
		pBeaconSync->BeaconBitMap &= (~(BEACON_BITMAP_MASK & (1 << bcn_idx)));
		NdisZeroMemory(pBeaconSync->BeaconTxWI[bcn_idx], TXWI_SIZE);
	}
	else
	{
		ptr = (PUCHAR)&pAd->BeaconTxWI;

		if (NdisEqualMemory(pBeaconSync->BeaconTxWI[bcn_idx], &pAd->BeaconTxWI, TXWI_SIZE) == FALSE)
		{	
			pBeaconSync->BeaconBitMap &= (~(BEACON_BITMAP_MASK & (1 << bcn_idx)));
			NdisMoveMemory(pBeaconSync->BeaconTxWI[bcn_idx], &pAd->BeaconTxWI, TXWI_SIZE);
		}

		if ((pBeaconSync->BeaconBitMap & (1 << bcn_idx)) != (1 << bcn_idx))
		{
			for (i=0; i<TXWI_SIZE; i+=4)  
			{
				longValue =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
				RTMP_IO_WRITE32(pAd, pAd->BeaconOffset[bcn_idx] + i, longValue);
				ptr += 4;
			}
		}

		ptr = pBeaconSync->BeaconBuf[bcn_idx];
		padding = (FrameLen & 0x01);
		NdisZeroMemory((PUCHAR)(pBeaconFrame + FrameLen), padding);
		FrameLen += padding;
		for (i = 0 ; i < FrameLen ; i += 2)
		{
			if (NdisEqualMemory(ptr, pBeaconFrame, 2) == FALSE)
			{
				NdisMoveMemory(ptr, pBeaconFrame, 2);
				
				
				RTUSBMultiWrite(pAd, pAd->BeaconOffset[bcn_idx] + TXWI_SIZE + i, ptr, 2);
			}
			ptr +=2;
			pBeaconFrame += 2;
		}

		pBeaconSync->BeaconBitMap |= (1 << bcn_idx);

		
	}

}


VOID RT2870_BssBeaconStop(
	IN RTMP_ADAPTER *pAd)
{
	BEACON_SYNC_STRUCT	*pBeaconSync;
	int i, offset;
	BOOLEAN	Cancelled = TRUE;

	pBeaconSync = pAd->CommonCfg.pBeaconSync;
	if (pBeaconSync && pBeaconSync->EnableBeacon)
	{
		INT NumOfBcn;

		NumOfBcn = MAX_MESH_NUM;

		RTMPCancelTimer(&pAd->CommonCfg.BeaconUpdateTimer, &Cancelled);

		for(i=0; i<NumOfBcn; i++)
		{
			NdisZeroMemory(pBeaconSync->BeaconBuf[i], HW_BEACON_OFFSET);
			NdisZeroMemory(pBeaconSync->BeaconTxWI[i], TXWI_SIZE);

			for (offset=0; offset<HW_BEACON_OFFSET; offset+=4)
				RTMP_IO_WRITE32(pAd, pAd->BeaconOffset[i] + offset, 0x00);

			pBeaconSync->CapabilityInfoLocationInBeacon[i] = 0;
			pBeaconSync->TimIELocationInBeacon[i] = 0;
		}
		pBeaconSync->BeaconBitMap = 0;
		pBeaconSync->DtimBitOn = 0;
	}
}


VOID RT2870_BssBeaconStart(
	IN RTMP_ADAPTER *pAd)
{
	int apidx;
	BEACON_SYNC_STRUCT	*pBeaconSync;


	pBeaconSync = pAd->CommonCfg.pBeaconSync;
	if (pBeaconSync && pBeaconSync->EnableBeacon)
	{
		INT NumOfBcn;

		NumOfBcn = MAX_MESH_NUM;

		for(apidx=0; apidx<NumOfBcn; apidx++)
		{
			UCHAR CapabilityInfoLocationInBeacon = 0;
			UCHAR TimIELocationInBeacon = 0;

			NdisZeroMemory(pBeaconSync->BeaconBuf[apidx], HW_BEACON_OFFSET);
			pBeaconSync->CapabilityInfoLocationInBeacon[apidx] = CapabilityInfoLocationInBeacon;
			pBeaconSync->TimIELocationInBeacon[apidx] = TimIELocationInBeacon;
			NdisZeroMemory(pBeaconSync->BeaconTxWI[apidx], TXWI_SIZE);
		}
		pBeaconSync->BeaconBitMap = 0;
		pBeaconSync->DtimBitOn = 0;
		pAd->CommonCfg.BeaconUpdateTimer.Repeat = TRUE;

		pAd->CommonCfg.BeaconAdjust = 0;
		pAd->CommonCfg.BeaconFactor = 0xffffffff / (pAd->CommonCfg.BeaconPeriod << 10);
		pAd->CommonCfg.BeaconRemain = (0xffffffff % (pAd->CommonCfg.BeaconPeriod << 10)) + 1;
		printk(RT28xx_CHIP_NAME "_BssBeaconStart:BeaconFactor=%d, BeaconRemain=%d!\n", pAd->CommonCfg.BeaconFactor, pAd->CommonCfg.BeaconRemain);
		RTMPSetTimer(&pAd->CommonCfg.BeaconUpdateTimer, pAd->CommonCfg.BeaconPeriod);

	}
}


VOID RT2870_BssBeaconInit(
	IN RTMP_ADAPTER *pAd)
{
	BEACON_SYNC_STRUCT	*pBeaconSync;
	int i;

	NdisAllocMemory(pAd->CommonCfg.pBeaconSync, sizeof(BEACON_SYNC_STRUCT), MEM_ALLOC_FLAG);
	if (pAd->CommonCfg.pBeaconSync)
	{
		pBeaconSync = pAd->CommonCfg.pBeaconSync;
		NdisZeroMemory(pBeaconSync, sizeof(BEACON_SYNC_STRUCT));
		for(i=0; i < HW_BEACON_MAX_COUNT; i++)
		{
			NdisZeroMemory(pBeaconSync->BeaconBuf[i], HW_BEACON_OFFSET);
			pBeaconSync->CapabilityInfoLocationInBeacon[i] = 0;
			pBeaconSync->TimIELocationInBeacon[i] = 0;
			NdisZeroMemory(pBeaconSync->BeaconTxWI[i], TXWI_SIZE);
		}
		pBeaconSync->BeaconBitMap = 0;

		
		pBeaconSync->EnableBeacon = TRUE;
	}
}


VOID RT2870_BssBeaconExit(
	IN RTMP_ADAPTER *pAd)
{
	BEACON_SYNC_STRUCT	*pBeaconSync;
	BOOLEAN	Cancelled = TRUE;
	int i;

	if (pAd->CommonCfg.pBeaconSync)
	{
		pBeaconSync = pAd->CommonCfg.pBeaconSync;
		pBeaconSync->EnableBeacon = FALSE;
		RTMPCancelTimer(&pAd->CommonCfg.BeaconUpdateTimer, &Cancelled);
		pBeaconSync->BeaconBitMap = 0;

		for(i=0; i<HW_BEACON_MAX_COUNT; i++)
		{
			NdisZeroMemory(pBeaconSync->BeaconBuf[i], HW_BEACON_OFFSET);
			pBeaconSync->CapabilityInfoLocationInBeacon[i] = 0;
			pBeaconSync->TimIELocationInBeacon[i] = 0;
			NdisZeroMemory(pBeaconSync->BeaconTxWI[i], TXWI_SIZE);
		}

		NdisFreeMemory(pAd->CommonCfg.pBeaconSync, HW_BEACON_OFFSET * HW_BEACON_MAX_COUNT, 0);
		pAd->CommonCfg.pBeaconSync = NULL;
	}
}

VOID BeaconUpdateExec(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER	pAd = (PRTMP_ADAPTER)FunctionContext;
	LARGE_INTEGER	tsfTime_a;
	UINT32			delta, remain, remain_low, remain_high;


	ReSyncBeaconTime(pAd);



	RTMP_IO_READ32(pAd, TSF_TIMER_DW0, &tsfTime_a.u.LowPart);
	RTMP_IO_READ32(pAd, TSF_TIMER_DW1, &tsfTime_a.u.HighPart);


	
	remain_high = pAd->CommonCfg.BeaconRemain * tsfTime_a.u.HighPart;
	remain_low = tsfTime_a.u.LowPart % (pAd->CommonCfg.BeaconPeriod << 10);
	remain = (remain_high + remain_low)%(pAd->CommonCfg.BeaconPeriod << 10);
	delta = (pAd->CommonCfg.BeaconPeriod << 10) - remain;

	pAd->CommonCfg.BeaconUpdateTimer.TimerValue = (delta >> 10) + 10;

}

