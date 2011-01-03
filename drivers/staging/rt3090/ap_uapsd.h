





#define UAPSD_TIMING_RECORD_MAX				1000
#define UAPSD_TIMING_RECORD_DISPLAY_TIMES	10

#define UAPSD_TIMING_RECORD_ISR				1
#define UAPSD_TIMING_RECORD_TASKLET			2
#define UAPSD_TIMING_RECORD_TRG_RCV			3
#define UAPSD_TIMING_RECORD_MOVE2TX			4
#define UAPSD_TIMING_RECORD_TX2AIR			5

#define UAPSD_TIMING_CTRL_STOP				0
#define UAPSD_TIMING_CTRL_START				1
#define UAPSD_TIMING_CTRL_SUSPEND			2

#define UAPSD_TIMESTAMP_GET(__pAd, __TimeStamp)			\
	{													\
		UINT32 __CSR=0;	UINT64 __Value64;				\
		RTMP_IO_READ32((__pAd), TSF_TIMER_DW0, &__CSR);	\
		__TimeStamp = (UINT64)__CSR;					\
		RTMP_IO_READ32((__pAd), TSF_TIMER_DW1, &__CSR);	\
		__Value64 = (UINT64)__CSR;						\
		__TimeStamp |= (__Value64 << 32);				\
	}

#ifdef LINUX
#define UAPSD_TIME_GET(__pAd, __Time)					\
		__Time = jiffies
#endif 


#ifdef UAPSD_TIMING_RECORD_FUNC
#define UAPSD_TIMING_RECORD_START()				\
	UAPSD_TimingRecordCtrl(UAPSD_TIMING_CTRL_START);
#define UAPSD_TIMING_RECORD_STOP()				\
	UAPSD_TimingRecordCtrl(UAPSD_TIMING_CTRL_STOP);
#define UAPSD_TIMING_RECORD(__pAd, __Type)		\
	UAPSD_TimingRecord(__pAd, __Type);
#define UAPSD_TIMING_RECORD_INDEX(__LoopIndex)	\
	UAPSD_TimeingRecordLoopIndex(__LoopIndex);
#else

#define UAPSD_TIMING_RECORD_START()
#define UAPSD_TIMING_RECORD_STOP()
#define UAPSD_TIMING_RECORD(__pAd, __type)
#define UAPSD_TIMING_RECORD_INDEX(__LoopIndex)
#endif 


#ifndef MODULE_WMM_UAPSD

#define UAPSD_EXTERN			extern




#define UAPSD_MR_QOS_NULL_HANDLE(__pAd, __pData, __pPacket)					\
	{																		\
		PHEADER_802_11 __pHeader = (PHEADER_802_11)(__pData);				\
		MAC_TABLE_ENTRY *__pEntry;											\
		if (__pHeader->FC.SubType == SUBTYPE_QOS_NULL)						\
		{																	\
			RTMP_SET_PACKET_QOS_NULL((__pPacket));							\
			__pEntry = MacTableLookup((__pAd), __pHeader->Addr1);			\
			if (__pEntry != NULL)											\
			{																\
				RTMP_SET_PACKET_WCID((__pPacket), __pEntry->Aid);			\
			}																\
		}																	\
		else																\
		{																	\
			RTMP_SET_PACKET_NON_QOS_NULL((__pPacket));						\
		}																	\
	}


#define UAPSD_MR_ENTRY_INIT(__pEntry)										\
	{																		\
		UINT16	__IdAc;														\
		for(__IdAc=0; __IdAc<WMM_NUM_OF_AC; __IdAc++)						\
			InitializeQueueHeader(&(__pEntry)->UAPSDQueue[__IdAc]);			\
		(__pEntry)->UAPSDTxNum = 0;											\
		(__pEntry)->pUAPSDEOSPFrame = NULL;									\
		(__pEntry)->bAPSDFlagSPStart = 0;									\
		(__pEntry)->bAPSDFlagEOSPOK = 0;									\
		(__pEntry)->MaxSPLength = 0;										\
	}


#define UAPSD_MR_ENTRY_RESET(__pAd, __pEntry)								\
	{																		\
		MAC_TABLE_ENTRY *__pSta;											\
		UINT32 __IdAc;														\
		__pSta = (__pEntry);												\
												\
		for(__IdAc=0; __IdAc<WMM_NUM_OF_AC; __IdAc++)						\
			APCleanupPsQueue((__pAd), &__pSta->UAPSDQueue[__IdAc]);		\
														\
		__pSta->UAPSDTxNum = 0;												\
		if (__pSta->pUAPSDEOSPFrame != NULL) {								\
			RELEASE_NDIS_PACKET((__pAd),									\
							QUEUE_ENTRY_TO_PACKET(__pSta->pUAPSDEOSPFrame),	\
							NDIS_STATUS_FAILURE);							\
			__pSta->pUAPSDEOSPFrame = NULL; }								\
		__pSta->bAPSDFlagSPStart = 0;										\
		__pSta->bAPSDFlagEOSPOK = 0; }


#define UAPSD_MR_IE_FILL(__QosCtrlField, __pAd)								\
		(__QosCtrlField) |= ((__pAd)->CommonCfg.bAPSDCapable) ? 0x80 : 0x00;


#define UAPSD_MR_IS_NOT_TIM_BIT_NEEDED_HANDLED(__pMacEntry, __QueIdx)		\
		(CLIENT_STATUS_TEST_FLAG((__pMacEntry), fCLIENT_STATUS_APSD_CAPABLE) && \
			(!(__pMacEntry)->bAPSDDeliverEnabledPerAC[QID_AC_VO] ||			\
			!(__pMacEntry)->bAPSDDeliverEnabledPerAC[QID_AC_VI] ||			\
			!(__pMacEntry)->bAPSDDeliverEnabledPerAC[QID_AC_BE] ||			\
			!(__pMacEntry)->bAPSDDeliverEnabledPerAC[QID_AC_BK]) &&			\
		(__pMacEntry)->bAPSDDeliverEnabledPerAC[__QueIdx])


#define UAPSD_MR_IS_UAPSD_AC(__pMacEntry, __AcId)							\
		(CLIENT_STATUS_TEST_FLAG((__pMacEntry), fCLIENT_STATUS_APSD_CAPABLE) &&	\
			((0 <= (__AcId)) && ((__AcId) < WMM_NUM_OF_AC)) && 	\
			(__pMacEntry)->bAPSDDeliverEnabledPerAC[(__AcId)])


#define UAPSD_MR_IS_ALL_AC_UAPSD(__FlgIsActive, __pMacEntry)				\
		(((__FlgIsActive) == FALSE) && ((__pMacEntry)->bAPSDAllAC == 1))


#define UAPSD_MR_SP_SUSPEND(__pAd)											\
		(__pAd)->bAPSDFlagSPSuspend = 1;


#define UAPSD_MR_SP_RESUME(__pAd)											\
		(__pAd)->bAPSDFlagSPSuspend = 0;


#ifdef RTMP_MAC_PCI

#define UAPSD_MR_MIX_PS_POLL_RCV(__pAd, __pMacEntry)						\
		if ((__pMacEntry)->bAPSDFlagSpRoughUse == 0)						\
		{																	\
			if ((__pMacEntry)->bAPSDFlagSPStart == 0)						\
			{																\
				if ((__pMacEntry)->bAPSDFlagLegacySent == 1)				\
					NICUpdateFifoStaCounters((__pAd));						\
				(__pMacEntry)->bAPSDFlagLegacySent = 1;						\
			}																\
			else															\
			{																\
				(__pMacEntry)->UAPSDTxNum ++;								\
			}																\
		}
#endif 


#else

#define UAPSD_EXTERN
#define UAPSD_QOS_NULL_QUE_ID	0x7f

#ifdef RTMP_MAC_PCI



#define UAPSD_SP_ACCURATE		
#endif 

#define UAPSD_EPT_SP_INT		(100000/(1000000/OS_HZ)) 

#endif 



#define MAX_PACKETS_IN_UAPSD_QUEUE	16	




UAPSD_EXTERN VOID UAPSD_Init(
	IN	PRTMP_ADAPTER		pAd);



UAPSD_EXTERN VOID UAPSD_Release(
	IN	PRTMP_ADAPTER		pAd);



UAPSD_EXTERN VOID UAPSD_FreeAll(
	IN	PRTMP_ADAPTER		pAd);



UAPSD_EXTERN VOID UAPSD_SP_Close(
    IN  PRTMP_ADAPTER       pAd,
	IN	MAC_TABLE_ENTRY		*pEntry);



UAPSD_EXTERN VOID UAPSD_AllPacketDeliver(
	IN	PRTMP_ADAPTER		pAd,
	IN	MAC_TABLE_ENTRY		*pEntry);



UAPSD_EXTERN VOID UAPSD_AssocParse(
	IN	PRTMP_ADAPTER		pAd,
	IN	MAC_TABLE_ENTRY		*pEntry,
	IN	UCHAR				*pElm);



UAPSD_EXTERN VOID UAPSD_PacketEnqueue(
	IN	PRTMP_ADAPTER		pAd,
	IN	MAC_TABLE_ENTRY		*pEntry,
	IN	PNDIS_PACKET		pPacket,
	IN	UINT32				IdAc);



UAPSD_EXTERN VOID UAPSD_QoSNullTxMgmtTxDoneHandle(
	IN	PRTMP_ADAPTER		pAd,
	IN	PNDIS_PACKET		pPacket,
	IN	UCHAR				*pDstMac);



UAPSD_EXTERN VOID UAPSD_QueueMaintenance(
	IN	PRTMP_ADAPTER		pAd,
	IN	MAC_TABLE_ENTRY		*pEntry);



UAPSD_EXTERN VOID UAPSD_SP_AUE_Handle(
	IN RTMP_ADAPTER		*pAd,
    IN MAC_TABLE_ENTRY	*pEntry,
	IN UCHAR			FlgSuccess);



UAPSD_EXTERN VOID UAPSD_SP_CloseInRVDone(
	IN	PRTMP_ADAPTER		pAd);



UAPSD_EXTERN VOID UAPSD_SP_PacketCheck(
	IN	PRTMP_ADAPTER		pAd,
	IN	PNDIS_PACKET		pPacket,
	IN	UCHAR				*pDstMac);


#ifdef UAPSD_TIMING_RECORD_FUNC

UAPSD_EXTERN VOID UAPSD_TimingRecordCtrl(
	IN	UINT32				Flag);


UAPSD_EXTERN VOID UAPSD_TimingRecord(
	IN	PRTMP_ADAPTER		pAd,
	IN	UINT32				Type);


UAPSD_EXTERN VOID UAPSD_TimeingRecordLoopIndex(
	IN	UINT32				LoopIndex);
#endif 



UAPSD_EXTERN VOID UAPSD_TriggerFrameHandle(
	IN	PRTMP_ADAPTER		pAd,
	IN	MAC_TABLE_ENTRY		*pEntry,
	IN	UCHAR				UpOfFrame);




