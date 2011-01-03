

#ifndef __RT2870_H__
#define __RT2870_H__


#include <linux/usb.h>



#define BULKAGGRE_ZISE          100
#define RT28XX_DRVDATA_SET(_a)                                             usb_set_intfdata(_a, pAd);
#define RT28XX_PUT_DEVICE                                                  usb_put_dev
#define RTUSB_ALLOC_URB(iso)                                               usb_alloc_urb(iso, GFP_ATOMIC)
#define RTUSB_SUBMIT_URB(pUrb)                                             usb_submit_urb(pUrb, GFP_ATOMIC)
#define	RTUSB_URB_ALLOC_BUFFER(pUsb_Dev, BufSize, pDma_addr)               usb_buffer_alloc(pUsb_Dev, BufSize, GFP_ATOMIC, pDma_addr)
#define	RTUSB_URB_FREE_BUFFER(pUsb_Dev, BufSize, pTransferBuf, Dma_addr)   usb_buffer_free(pUsb_Dev, BufSize, pTransferBuf, Dma_addr)

#define RXBULKAGGRE_ZISE        12
#define MAX_TXBULK_LIMIT        (LOCAL_TXBUF_SIZE*(BULKAGGRE_ZISE-1))
#define MAX_TXBULK_SIZE         (LOCAL_TXBUF_SIZE*BULKAGGRE_ZISE)
#define MAX_RXBULK_SIZE         (LOCAL_TXBUF_SIZE*RXBULKAGGRE_ZISE)
#define MAX_MLME_HANDLER_MEMORY 20
#define	RETRY_LIMIT             10
#define BUFFER_SIZE				2400	
#define	TX_RING					0xa
#define	PRIO_RING				0xc




#define	fRTUSB_BULK_OUT_DATA_NULL				0x00000001
#define fRTUSB_BULK_OUT_RTS						0x00000002
#define	fRTUSB_BULK_OUT_MLME					0x00000004

#define	fRTUSB_BULK_OUT_DATA_NORMAL				0x00010000
#define	fRTUSB_BULK_OUT_DATA_NORMAL_2			0x00020000
#define	fRTUSB_BULK_OUT_DATA_NORMAL_3			0x00040000
#define	fRTUSB_BULK_OUT_DATA_NORMAL_4			0x00080000
#define	fRTUSB_BULK_OUT_DATA_NORMAL_5			0x00100000

#define	fRTUSB_BULK_OUT_PSPOLL					0x00000020
#define	fRTUSB_BULK_OUT_DATA_FRAG				0x00000040
#define	fRTUSB_BULK_OUT_DATA_FRAG_2				0x00000080
#define	fRTUSB_BULK_OUT_DATA_FRAG_3				0x00000100
#define	fRTUSB_BULK_OUT_DATA_FRAG_4				0x00000200

#define	FREE_HTTX_RING(_p, _b, _t)			\
{										\
	if ((_t)->ENextBulkOutPosition == (_t)->CurWritePosition)				\
	{																	\
		(_t)->bRingEmpty = TRUE;			\
	}																	\
	\
}




typedef	struct	PACKED _RXINFO_STRUC {
	UINT32		BA:1;
	UINT32		DATA:1;
	UINT32		NULLDATA:1;
	UINT32		FRAG:1;
	UINT32		U2M:1;              
	UINT32		Mcast:1;            
	UINT32		Bcast:1;            
	UINT32		MyBss:1;  	
	UINT32		Crc:1;              
	UINT32		CipherErr:2;        
	UINT32		AMSDU:1;		
	UINT32		HTC:1;
	UINT32		RSSI:1;
	UINT32		L2PAD:1;
	UINT32		AMPDU:1;		
	UINT32		Decrypted:1;
	UINT32		PlcpRssil:1;
	UINT32		CipherAlg:1;
	UINT32		LastAMSDU:1;
	UINT32		PlcpSignal:12;
}	RXINFO_STRUC, *PRXINFO_STRUC, RT28XX_RXD_STRUC, *PRT28XX_RXD_STRUC;




typedef	struct	_TXINFO_STRUC {
	
	UINT32		USBDMATxPktLen:16;	
	UINT32		rsv:8;
	UINT32		WIV:1;	
	UINT32		QSEL:2;	
	UINT32		SwUseLastRound:1; 
	UINT32		rsv2:2;  
	UINT32		USBDMANextVLD:1;	
	UINT32		USBDMATxburst:1;
}	TXINFO_STRUC, *PTXINFO_STRUC;

#define TXINFO_SIZE				4
#define RXINFO_SIZE				4
#define TXPADDING_SIZE			11




typedef	struct	_MGMT_STRUC	{
	BOOLEAN		Valid;
	PUCHAR		pBuffer;
	ULONG		Length;
}	MGMT_STRUC, *PMGMT_STRUC;



#define RT28xx_EEPROM_READ16(pAd, offset, var)					\
	do {														\
		RTUSBReadEEPROM(pAd, offset, (PUCHAR)&(var), 2);		\
		if(!pAd->bUseEfuse)										\
		var = le2cpu16(var);									\
	}while(0)

#define RT28xx_EEPROM_WRITE16(pAd, offset, var)					\
	do{															\
		USHORT _tmpVar=var;										\
		if(!pAd->bUseEfuse)									\
		_tmpVar = cpu2le16(var);								\
		RTUSBWriteEEPROM(pAd, offset, (PUCHAR)&(_tmpVar), 2);	\
	}while(0)


#define RT28XX_TASK_THREAD_INIT(pAd, Status)		\
	Status = CreateThreads(net_dev);



#define RT28XX_WRITE_FIRMWARE(_pAd, _pFwImage, _FwLen)		\
	RTUSBFirmwareWrite(_pAd, _pFwImage, _FwLen)


#define RT28XX_START_DEQUEUE(pAd, QueIdx, irqFlags)				\
			{													\
				RTMP_IRQ_LOCK(&pAd->DeQueueLock[QueIdx], irqFlags);		\
				if (pAd->DeQueueRunning[QueIdx])						\
				{														\
					RTMP_IRQ_UNLOCK(&pAd->DeQueueLock[QueIdx], irqFlags);\
					printk("DeQueueRunning[%d]= TRUE!\n", QueIdx);		\
					continue;											\
				}														\
				else													\
				{														\
					pAd->DeQueueRunning[QueIdx] = TRUE;					\
					RTMP_IRQ_UNLOCK(&pAd->DeQueueLock[QueIdx], irqFlags);\
				}														\
			}
#define RT28XX_STOP_DEQUEUE(pAd, QueIdx, irqFlags)						\
			do{															\
				RTMP_IRQ_LOCK(&pAd->DeQueueLock[QueIdx], irqFlags);		\
				pAd->DeQueueRunning[QueIdx] = FALSE;					\
				RTMP_IRQ_UNLOCK(&pAd->DeQueueLock[QueIdx], irqFlags);	\
			}while(0)


#define	RT28XX_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, freeNum, pPacket) \
		(RTUSBFreeDescriptorRequest(pAd, pTxBlk->QueIdx, (pTxBlk->TotalFrameLen + GET_OS_PKT_LEN(pPacket))) == NDIS_STATUS_SUCCESS)

#define RT28XX_RELEASE_DESC_RESOURCE(pAd, QueIdx)			\
		do{}while(0)

#define NEED_QUEUE_BACK_FOR_AGG(_pAd, _QueIdx, _freeNum, _TxFrameType) 		\
		((_TxFrameType == TX_RALINK_FRAME) && (RTUSBNeedQueueBackForAgg(_pAd, _QueIdx)))



#define fRTMP_ADAPTER_NEED_STOP_TX		\
		(fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_HALT_IN_PROGRESS |	\
		 fRTMP_ADAPTER_RESET_IN_PROGRESS | fRTMP_ADAPTER_BULKOUT_RESET | \
		 fRTMP_ADAPTER_RADIO_OFF | fRTMP_ADAPTER_REMOVE_IN_PROGRESS)


#define HAL_WriteSubTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)	\
			RtmpUSB_WriteSubTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)

#define HAL_WriteTxResource(pAd, pTxBlk,bIsLast, pFreeNumber)	\
			RtmpUSB_WriteSingleTxResource(pAd, pTxBlk,bIsLast, pFreeNumber)

#define HAL_WriteFragTxResource(pAd, pTxBlk, fragNum, pFreeNumber) \
			RtmpUSB_WriteFragTxResource(pAd, pTxBlk, fragNum, pFreeNumber)

#define HAL_WriteMultiTxResource(pAd, pTxBlk,frameNum, pFreeNumber)	\
			RtmpUSB_WriteMultiTxResource(pAd, pTxBlk,frameNum, pFreeNumber)

#define HAL_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, TxIdx)	\
			RtmpUSB_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, TxIdx)

#define HAL_LastTxIdx(pAd, QueIdx,TxIdx) \
			

#define HAL_KickOutTx(pAd, pTxBlk, QueIdx)	\
			RtmpUSBDataKickOut(pAd, pTxBlk, QueIdx)


#define HAL_KickOutMgmtTx(pAd, QueIdx, pPacket, pSrcBufVA, SrcBufLen)	\
			RtmpUSBMgmtKickOut(pAd, QueIdx, pPacket, pSrcBufVA, SrcBufLen)

#define HAL_KickOutNullFrameTx(_pAd, _QueIdx, _pNullFrame, _frameLen)	\
			RtmpUSBNullFrameKickOut(_pAd, _QueIdx, _pNullFrame, _frameLen)

#define RTMP_PKT_TAIL_PADDING 	11 

extern UCHAR EpToQueue[6];


#ifdef RT2870
#define GET_TXRING_FREENO(_pAd, _QueIdx) 	(_QueIdx) 
#define GET_MGMTRING_FREENO(_pAd) 			(_pAd->MgmtRing.TxSwFreeIdx)
#endif 





#define RT28XX_RV_ALL_BUF_END(bBulkReceive)		\
		\
			\
	if (bBulkReceive == TRUE)	RTUSBBulkReceive(pAd);





#define RT28XX_STA_ENTRY_MAC_RESET(pAd, Wcid)					\
	{	RT_SET_ASIC_WCID	SetAsicWcid;						\
		SetAsicWcid.WCID = Wcid;								\
		SetAsicWcid.SetTid = 0xffffffff;						\
		SetAsicWcid.DeleteTid = 0xffffffff;						\
		RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_SET_ASIC_WCID, 	\
				&SetAsicWcid, sizeof(RT_SET_ASIC_WCID));	}


#define RT28XX_STA_ENTRY_ADD(pAd, pEntry)							\
	RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_SET_CLIENT_MAC_ENTRY, 	\
							pEntry, sizeof(MAC_TABLE_ENTRY));



#define RT28XX_STA_ENTRY_KEY_DEL(pAd, BssIdx, Wcid)


#define RT28XX_STA_SECURITY_INFO_ADD(pAd, apidx, KeyID, pEntry)						\
	{	RT28XX_STA_ENTRY_MAC_RESET(pAd, pEntry->Aid);								\
		if (pEntry->Aid >= 1) {														\
			RT_SET_ASIC_WCID_ATTRI	SetAsicWcidAttri;								\
			SetAsicWcidAttri.WCID = pEntry->Aid;									\
			if ((pEntry->AuthMode <= Ndis802_11AuthModeAutoSwitch) &&				\
				(pEntry->WepStatus == Ndis802_11Encryption1Enabled))				\
			{																		\
				SetAsicWcidAttri.Cipher = pAd->SharedKey[apidx][KeyID].CipherAlg;	\
			}																		\
			else if (pEntry->AuthMode == Ndis802_11AuthModeWPANone)					\
			{																		\
				SetAsicWcidAttri.Cipher = pAd->SharedKey[apidx][KeyID].CipherAlg;	\
			}																		\
			else SetAsicWcidAttri.Cipher = 0;										\
            DBGPRINT(RT_DEBUG_TRACE, ("aid cipher = %ld\n",SetAsicWcidAttri.Cipher));       \
			RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_SET_ASIC_WCID_CIPHER, 			\
							&SetAsicWcidAttri, sizeof(RT_SET_ASIC_WCID_ATTRI)); } }


#define RT28XX_ADD_BA_SESSION_TO_ASIC(_pAd, _Aid, _TID)					\
		do{																\
			RT_SET_ASIC_WCID	SetAsicWcid;							\
			SetAsicWcid.WCID = (_Aid);									\
			SetAsicWcid.SetTid = (0x10000<<(_TID));						\
			SetAsicWcid.DeleteTid = 0xffffffff;							\
			RTUSBEnqueueInternalCmd((_pAd), CMDTHREAD_SET_ASIC_WCID, &SetAsicWcid, sizeof(RT_SET_ASIC_WCID));	\
		}while(0)


#define RT28XX_DEL_BA_SESSION_FROM_ASIC(_pAd, _Wcid, _TID)				\
		do{																\
			RT_SET_ASIC_WCID	SetAsicWcid;							\
			SetAsicWcid.WCID = (_Wcid);									\
			SetAsicWcid.SetTid = (0xffffffff);							\
			SetAsicWcid.DeleteTid = (0x10000<<(_TID) );					\
			RTUSBEnqueueInternalCmd((_pAd), CMDTHREAD_SET_ASIC_WCID, &SetAsicWcid, sizeof(RT_SET_ASIC_WCID));	\
		}while(0)



#define RT28XX_HANDLE_DEV_ASSIGN(handle, dev_p)			\
	((POS_COOKIE)handle)->pUsb_Dev = dev_p;


#define RT28XX_UNMAP()
#define RT28XX_IRQ_REQUEST(net_dev)
#define RT28XX_IRQ_RELEASE(net_dev)
#define RT28XX_IRQ_INIT(pAd)
#define RT28XX_IRQ_ENABLE(pAd)



#define RT28XX_MLME_HANDLER(pAd)			RTUSBMlmeUp(pAd)

#define RT28XX_MLME_PRE_SANITY_CHECK(pAd)								\
	{	if ((pAd->CommonCfg.bHardwareRadio == TRUE) && 					\
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&		\
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))) {	\
			RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_CHECK_GPIO, NULL, 0); } }

#define RT28XX_MLME_STA_QUICK_RSP_WAKE_UP(pAd)	\
	{	RTUSBEnqueueInternalCmd(pAd, CMDTHREAD_QKERIODIC_EXECUT, NULL, 0);	\
		RTUSBMlmeUp(pAd); }

#define RT28XX_MLME_RESET_STATE_MACHINE(pAd)	\
		        MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_RESET_CONF, 0, NULL);	\
		        RTUSBMlmeUp(pAd);

#define RT28XX_HANDLE_COUNTER_MEASURE(_pAd, _pEntry)		\
	{	RTUSBEnqueueInternalCmd(_pAd, CMDTHREAD_802_11_COUNTER_MEASURE, _pEntry, sizeof(MAC_TABLE_ENTRY));	\
		RTUSBMlmeUp(_pAd);									\
	}



#define RT28XX_PS_POLL_ENQUEUE(pAd)						\
	{	RTUSB_SET_BULK_FLAG(pAd, fRTUSB_BULK_OUT_PSPOLL);	\
		RTUSBKickBulkOut(pAd); }

#define RT28xx_CHIP_NAME            "RTxx70"

#define USB_CYC_CFG                 0x02a4
#define STATUS_SUCCESS				0x00
#define STATUS_UNSUCCESSFUL 		0x01
#define NT_SUCCESS(status)			(((status) > 0) ? (1):(0))
#define InterlockedIncrement 	 	atomic_inc
#define NdisInterlockedIncrement 	atomic_inc
#define InterlockedDecrement		atomic_dec
#define NdisInterlockedDecrement 	atomic_dec
#define InterlockedExchange			atomic_set

#define NdisMCancelTimer			RTMPCancelTimer
#define NdisAllocMemory(_ptr, _size, _flag)	\
									do{_ptr = kmalloc((_size),(_flag));}while(0)
#define NdisFreeMemory(a, b, c) 	kfree((a))
#define NdisMSleep					RTMPusecDelay		


#define USBD_TRANSFER_DIRECTION_OUT		0
#define USBD_TRANSFER_DIRECTION_IN		0
#define USBD_SHORT_TRANSFER_OK			0
#define PURB			purbb_t

#define RTUSB_FREE_URB(pUrb)	usb_free_urb(pUrb)




typedef struct usb_device	* PUSB_DEV;


typedef struct urb *purbb_t;
typedef struct usb_ctrlrequest devctrlrequest;
#define PIRP		PVOID
#define PMDL		PVOID
#define NDIS_OID	UINT
#ifndef USB_ST_NOERROR
#define USB_ST_NOERROR     0
#endif


#define CONTROL_TIMEOUT_JIFFIES ( (100 * HZ) / 1000)
#define UNLINK_TIMEOUT_MS		3


#define RTUSB_UNLINK_URB(pUrb)		usb_kill_urb(pUrb)


VOID RTUSBBulkOutDataPacketComplete(purbb_t purb, struct pt_regs *pt_regs);
VOID RTUSBBulkOutMLMEPacketComplete(purbb_t pUrb, struct pt_regs *pt_regs);
VOID RTUSBBulkOutNullFrameComplete(purbb_t pUrb, struct pt_regs *pt_regs);
VOID RTUSBBulkOutRTSFrameComplete(purbb_t pUrb, struct pt_regs *pt_regs);
VOID RTUSBBulkOutPsPollComplete(purbb_t pUrb, struct pt_regs *pt_regs);
VOID RTUSBBulkRxComplete(purbb_t pUrb, struct pt_regs *pt_regs);

#define RTUSBMlmeUp(pAd)	        \
{								    \
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;	\
	if (pid_nr(pObj->MLMEThr_pid) > 0) \
        up(&(pAd->mlme_semaphore)); \
}

#define RTUSBCMDUp(pAd)	                \
{									    \
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;	\
	if (pid_nr(pObj->RTUSBCmdThr_pid) > 0) \
	    up(&(pAd->RTUSBCmd_semaphore)); \
}

static inline NDIS_STATUS RTMPAllocateMemory(
	OUT PVOID *ptr,
	IN size_t size)
{
	*ptr = kmalloc(size, GFP_ATOMIC);
	if(*ptr)
		return NDIS_STATUS_SUCCESS;
	else
		return NDIS_STATUS_RESOURCES;
}


#define	BEACON_RING_SIZE                2
#define DEVICE_VENDOR_REQUEST_OUT       0x40
#define DEVICE_VENDOR_REQUEST_IN        0xc0
#define INTERFACE_VENDOR_REQUEST_OUT    0x41
#define INTERFACE_VENDOR_REQUEST_IN     0xc1
#define MGMTPIPEIDX						0	

#define BULKOUT_MGMT_RESET_FLAG				0x80

#define RTUSB_SET_BULK_FLAG(_M, _F)				((_M)->BulkFlags |= (_F))
#define RTUSB_CLEAR_BULK_FLAG(_M, _F)			((_M)->BulkFlags &= ~(_F))
#define RTUSB_TEST_BULK_FLAG(_M, _F)			(((_M)->BulkFlags & (_F)) != 0)

#define EnqueueCmd(cmdq, cmdqelmt)		\
{										\
	if (cmdq->size == 0)				\
		cmdq->head = cmdqelmt;			\
	else								\
		cmdq->tail->next = cmdqelmt;	\
	cmdq->tail = cmdqelmt;				\
	cmdqelmt->next = NULL;				\
	cmdq->size++;						\
}

typedef struct   _RT_SET_ASIC_WCID {
	ULONG WCID;          
	ULONG SetTid;        
	ULONG DeleteTid;        
	UCHAR Addr[MAC_ADDR_LEN];	
} RT_SET_ASIC_WCID,*PRT_SET_ASIC_WCID;

typedef struct   _RT_SET_ASIC_WCID_ATTRI {
	ULONG	WCID;          
	ULONG	Cipher;        
	UCHAR	Addr[ETH_LENGTH_OF_ADDRESS];
} RT_SET_ASIC_WCID_ATTRI,*PRT_SET_ASIC_WCID_ATTRI;

typedef struct _MLME_MEMORY_STRUCT {
	PVOID                           AllocVa;    
	struct _MLME_MEMORY_STRUCT      *Next;      
}   MLME_MEMORY_STRUCT, *PMLME_MEMORY_STRUCT;

typedef struct  _MLME_MEMORY_HANDLER {
	BOOLEAN                 MemRunning;         
	UINT                    MemoryCount;        
	UINT                    InUseCount;         
	UINT                    UnUseCount;         
	INT                    PendingCount;       
	PMLME_MEMORY_STRUCT     pInUseHead;         
	PMLME_MEMORY_STRUCT     pInUseTail;         
	PMLME_MEMORY_STRUCT     pUnUseHead;         
	PMLME_MEMORY_STRUCT     pUnUseTail;         
	PULONG                  MemFreePending[MAX_MLME_HANDLER_MEMORY];   
}   MLME_MEMORY_HANDLER, *PMLME_MEMORY_HANDLER;

typedef	struct _CmdQElmt	{
	UINT				command;
	PVOID				buffer;
	ULONG				bufferlength;
	BOOLEAN				CmdFromNdis;
	BOOLEAN				SetOperation;
	struct _CmdQElmt	*next;
}	CmdQElmt, *PCmdQElmt;

typedef	struct	_CmdQ	{
	UINT		size;
	CmdQElmt	*head;
	CmdQElmt	*tail;
	UINT32		CmdQState;
}CmdQ, *PCmdQ;



typedef enum _RT_802_11_CIPHER_SUITE_TYPE {
	Cipher_Type_NONE,
	Cipher_Type_WEP40,
	Cipher_Type_TKIP,
	Cipher_Type_RSVD,
	Cipher_Type_CCMP,
	Cipher_Type_WEP104
} RT_802_11_CIPHER_SUITE_TYPE, *PRT_802_11_CIPHER_SUITE_TYPE;





typedef	struct	_CMDHandler_TLV	{
	USHORT		Offset;
	USHORT		Length;
	UCHAR		DataFirst;
}	CMDHandler_TLV, *PCMDHandler_TLV;


#define CMDTHREAD_VENDOR_RESET                      0x0D730101	
#define CMDTHREAD_VENDOR_UNPLUG                     0x0D730102	
#define CMDTHREAD_VENDOR_SWITCH_FUNCTION            0x0D730103	
#define CMDTHREAD_MULTI_WRITE_MAC                   0x0D730107	
#define CMDTHREAD_MULTI_READ_MAC                    0x0D730108	
#define CMDTHREAD_VENDOR_EEPROM_WRITE               0x0D73010A	
#define CMDTHREAD_VENDOR_EEPROM_READ                0x0D73010B	
#define CMDTHREAD_VENDOR_ENTER_TESTMODE             0x0D73010C	
#define CMDTHREAD_VENDOR_EXIT_TESTMODE              0x0D73010D	
#define CMDTHREAD_VENDOR_WRITE_BBP                  0x0D730119	
#define CMDTHREAD_VENDOR_READ_BBP                   0x0D730118	
#define CMDTHREAD_VENDOR_WRITE_RF                   0x0D73011A	
#define CMDTHREAD_VENDOR_FLIP_IQ                    0x0D73011D	
#define CMDTHREAD_RESET_BULK_OUT                    0x0D730210	
#define CMDTHREAD_RESET_BULK_IN                     0x0D730211	
#define CMDTHREAD_SET_PSM_BIT_SAVE                  0x0D730212	
#define CMDTHREAD_SET_RADIO                         0x0D730214	
#define CMDTHREAD_UPDATE_TX_RATE                    0x0D730216	
#define CMDTHREAD_802_11_ADD_KEY_WEP                0x0D730218	
#define CMDTHREAD_RESET_FROM_ERROR                  0x0D73021A	
#define CMDTHREAD_LINK_DOWN                         0x0D73021B	
#define CMDTHREAD_RESET_FROM_NDIS                   0x0D73021C	
#define CMDTHREAD_CHECK_GPIO                        0x0D730215	
#define CMDTHREAD_FORCE_WAKE_UP                     0x0D730222	
#define CMDTHREAD_SET_BW                            0x0D730225	
#define CMDTHREAD_SET_ASIC_WCID                     0x0D730226	
#define CMDTHREAD_SET_ASIC_WCID_CIPHER              0x0D730227	
#define CMDTHREAD_QKERIODIC_EXECUT                  0x0D73023D	
#define CMDTHREAD_SET_CLIENT_MAC_ENTRY              0x0D73023E	
#define CMDTHREAD_802_11_QUERY_HARDWARE_REGISTER    0x0D710105	
#define CMDTHREAD_802_11_SET_PHY_MODE               0x0D79010C	
#define CMDTHREAD_802_11_SET_STA_CONFIG             0x0D790111	
#define CMDTHREAD_802_11_SET_PREAMBLE               0x0D790101	
#define CMDTHREAD_802_11_COUNTER_MEASURE			0x0D790102	
#define CMDTHREAD_UPDATE_PROTECT					0x0D790103	

#define WPA1AKMBIT	    0x01
#define WPA2AKMBIT	    0x02
#define WPA1PSKAKMBIT   0x04
#define WPA2PSKAKMBIT   0x08
#define TKIPBIT         0x01
#define CCMPBIT         0x02


#define RT28XX_STA_FORCE_WAKEUP(pAd, bFromTx) \
    RT28xxUsbStaAsicForceWakeup(pAd, bFromTx);

#define RT28XX_STA_SLEEP_THEN_AUTO_WAKEUP(pAd, TbttNumToNextWakeUp) \
    RT28xxUsbStaAsicSleepThenAutoWakeup(pAd, TbttNumToNextWakeUp);

#define RT28XX_MLME_RADIO_ON(pAd) \
    RT28xxUsbMlmeRadioOn(pAd);

#define RT28XX_MLME_RADIO_OFF(pAd) \
    RT28xxUsbMlmeRadioOFF(pAd);

#endif 
