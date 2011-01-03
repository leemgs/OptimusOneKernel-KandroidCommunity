

#ifndef __MAC_PCI_H__
#define __MAC_PCI_H__

#include "rtmp_type.h"
#include "rtmp_mac.h"
#include "rtmp_phy.h"
#include "rtmp_iface.h"
#include "rtmp_dot11.h"






#define NIC_PCI_VENDOR_ID		0x1814
#define PCIBUS_INTEL_VENDOR	0x8086

#if !defined(PCI_CAP_ID_EXP)
#define PCI_CAP_ID_EXP			    0x10
#endif
#if !defined(PCI_EXP_LNKCTL)
#define PCI_EXP_LNKCTL			    0x10
#endif
#if !defined(PCI_CLASS_BRIDGE_PCI)
#define PCI_CLASS_BRIDGE_PCI		0x0604
#endif





#define TXINFO_SIZE						0
#define RTMP_PKT_TAIL_PADDING			0
#define fRTMP_ADAPTER_NEED_STOP_TX	0

#define AUX_CTRL           0x10c




#ifdef RT_BIG_ENDIAN
typedef	struct	PACKED _TXD_STRUC {
	
	UINT32		SDPtr0;
	
	UINT32		DMADONE:1;
	UINT32		LastSec0:1;
	UINT32		SDLen0:14;
	UINT32		Burst:1;
	UINT32		LastSec1:1;
	UINT32		SDLen1:14;
	
	UINT32		SDPtr1;
	
	UINT32		ICO:1;
	UINT32		UCO:1;
	UINT32		TCO:1;
	UINT32		rsv:2;
	UINT32		QSEL:2;	
	UINT32		WIV:1;	
	UINT32		rsv2:24;
}	TXD_STRUC, *PTXD_STRUC;
#else
typedef	struct	PACKED _TXD_STRUC {
	
	UINT32		SDPtr0;
	
	UINT32		SDLen1:14;
	UINT32		LastSec1:1;
	UINT32		Burst:1;
	UINT32		SDLen0:14;
	UINT32		LastSec0:1;
	UINT32		DMADONE:1;
	
	UINT32		SDPtr1;
	
	UINT32		rsv2:24;
	UINT32		WIV:1;	
	UINT32		QSEL:2;	
	UINT32		rsv:2;
	UINT32		TCO:1;	
	UINT32		UCO:1;	
	UINT32		ICO:1;	
}	TXD_STRUC, *PTXD_STRUC;
#endif





#ifdef RT_BIG_ENDIAN
typedef	struct	PACKED _RXD_STRUC{
	
	UINT32		SDP0;
	
	UINT32		DDONE:1;
	UINT32		LS0:1;
	UINT32		SDL0:14;
	UINT32		Rsv:2;
	UINT32		SDL1:14;
	
	UINT32		SDP1;
	
	UINT32		Rsv1:13;
	UINT32		PlcpRssil:1;
	UINT32		PlcpSignal:1;		
	UINT32		Decrypted:1;	
	UINT32		AMPDU:1;
	UINT32		L2PAD:1;
	UINT32		RSSI:1;
	UINT32		HTC:1;
	UINT32		AMSDU:1;		
	UINT32		CipherErr:2;        
	UINT32		Crc:1;              
	UINT32		MyBss:1;	
	UINT32		Bcast:1;            
	UINT32		Mcast:1;            
	UINT32		U2M:1;              
	UINT32		FRAG:1;
	UINT32		NULLDATA:1;
	UINT32		DATA:1;
	UINT32		BA:1;

}	RXD_STRUC, *PRXD_STRUC, RT28XX_RXD_STRUC, *PRT28XX_RXD_STRUC;
#else
typedef	struct	PACKED _RXD_STRUC{
	
	UINT32		SDP0;
	
	UINT32		SDL1:14;
	UINT32		Rsv:2;
	UINT32		SDL0:14;
	UINT32		LS0:1;
	UINT32		DDONE:1;
	
	UINT32		SDP1;
	
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
	UINT32		PlcpSignal:1;		
	UINT32		PlcpRssil:1;
	UINT32		Rsv1:13;
}	RXD_STRUC, *PRXD_STRUC, RT28XX_RXD_STRUC, *PRT28XX_RXD_STRUC;
#endif

#ifdef BIG_ENDIAN
typedef union _TX_ATTENUATION_CTRL_STRUC
{
	struct
	{
		ULONG	Reserve1:20;
		ULONG	PCIE_PHY_TX_ATTEN_EN:1;
		ULONG	PCIE_PHY_TX_ATTEN_VALUE:3;
		ULONG	Reserve2:7;
		ULONG	RF_ISOLATION_ENABLE:1;
	} field;

	ULONG	word;
} TX_ATTENUATION_CTRL_STRUC, *PTX_ATTENUATION_CTRL_STRUC;
#else
typedef union _TX_ATTENUATION_CTRL_STRUC {
	struct
	{
		ULONG	RF_ISOLATION_ENABLE:1;
		ULONG	Reserve2:7;
		ULONG	PCIE_PHY_TX_ATTEN_VALUE:3;
		ULONG	PCIE_PHY_TX_ATTEN_EN:1;
		ULONG	Reserve1:20;
	} field;

	ULONG	word;
} TX_ATTENUATION_CTRL_STRUC, *PTX_ATTENUATION_CTRL_STRUC;
#endif



#define FIRMWARE_IMAGE_BASE     0x2000
#define MAX_FIRMWARE_IMAGE_SIZE 0x2000    



#define RTMP_WRITE_FIRMWARE(_pAd, _pFwImage, _FwLen)			\
	do{								\
		ULONG	_i, _firm;					\
		RTMP_IO_WRITE32(_pAd, PBF_SYS_CTRL, 0x10000);		\
									\
		for(_i=0; _i<_FwLen; _i+=4)				\
		{							\
			_firm = _pFwImage[_i] +				\
			   (_pFwImage[_i+3] << 24) +			\
			   (_pFwImage[_i+2] << 16) +			\
			   (_pFwImage[_i+1] << 8);			\
			RTMP_IO_WRITE32(_pAd, FIRMWARE_IMAGE_BASE + _i, _firm);	\
		}							\
		RTMP_IO_WRITE32(_pAd, PBF_SYS_CTRL, 0x00000);		\
		RTMP_IO_WRITE32(_pAd, PBF_SYS_CTRL, 0x00001);		\
									\
					\
		RTMP_IO_WRITE32(_pAd, H2M_BBP_AGENT, 0);		\
		RTMP_IO_WRITE32(_pAd, H2M_MAILBOX_CSR, 0);		\
	}while(0)



#define RTMP_START_DEQUEUE(pAd, QueIdx, irqFlags)		do{}while(0)
#define RTMP_STOP_DEQUEUE(pAd, QueIdx, irqFlags)		do{}while(0)


#define RTMP_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, freeNum, pPacket) \
		((freeNum) >= (ULONG)(pTxBlk->TotalFragNum + RTMP_GET_PACKET_FRAGMENTS(pPacket) + 3)) 
#define RTMP_RELEASE_DESC_RESOURCE(pAd, QueIdx)	\
		do{}while(0)

#define NEED_QUEUE_BACK_FOR_AGG(pAd, QueIdx, freeNum, _TxFrameType) \
		(((freeNum != (TX_RING_SIZE-1)) && (pAd->TxSwQueue[QueIdx].Number == 0)) || (freeNum<3))
		


#define HAL_KickOutMgmtTx(_pAd, _QueIdx, _pPacket, _pSrcBufVA, _SrcBufLen)	\
			RtmpPCIMgmtKickOut(_pAd, _QueIdx, _pPacket, _pSrcBufVA, _SrcBufLen)

#define HAL_WriteSubTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)	\
		

#define HAL_WriteTxResource(pAd, pTxBlk,bIsLast, pFreeNumber)	\
			RtmpPCI_WriteSingleTxResource(pAd, pTxBlk, bIsLast, pFreeNumber)

#define HAL_WriteFragTxResource(pAd, pTxBlk, fragNum, pFreeNumber) \
			RtmpPCI_WriteFragTxResource(pAd, pTxBlk, fragNum, pFreeNumber)

#define HAL_WriteMultiTxResource(pAd, pTxBlk,frameNum, pFreeNumber)	\
			RtmpPCI_WriteMultiTxResource(pAd, pTxBlk, frameNum, pFreeNumber)

#define HAL_FinalWriteTxResource(_pAd, _pTxBlk, _TotalMPDUSize, _FirstTxIdx)	\
			RtmpPCI_FinalWriteTxResource(_pAd, _pTxBlk, _TotalMPDUSize, _FirstTxIdx)

#define HAL_LastTxIdx(_pAd, _QueIdx,_LastTxIdx) \
			

#define HAL_KickOutTx(_pAd, _pTxBlk, _QueIdx)	\
			RTMP_IO_WRITE32((_pAd), TX_CTX_IDX0+((_QueIdx)*0x10), (_pAd)->TxRing[(_QueIdx)].TxCpuIdx)


#define HAL_KickOutNullFrameTx(_pAd, _QueIdx, _pNullFrame, _frameLen)	\
			MiniportMMRequest(_pAd, _QueIdx, _pNullFrame, _frameLen)

#define GET_TXRING_FREENO(_pAd, _QueIdx) \
	(_pAd->TxRing[_QueIdx].TxSwFreeIdx > _pAd->TxRing[_QueIdx].TxCpuIdx)	? \
			(_pAd->TxRing[_QueIdx].TxSwFreeIdx - _pAd->TxRing[_QueIdx].TxCpuIdx - 1) \
			 :	\
			(_pAd->TxRing[_QueIdx].TxSwFreeIdx + TX_RING_SIZE - _pAd->TxRing[_QueIdx].TxCpuIdx - 1);


#define GET_MGMTRING_FREENO(_pAd) \
	(_pAd->MgmtRing.TxSwFreeIdx > _pAd->MgmtRing.TxCpuIdx)	? \
			(_pAd->MgmtRing.TxSwFreeIdx - _pAd->MgmtRing.TxCpuIdx - 1) \
			 :	\
			(_pAd->MgmtRing.TxSwFreeIdx + MGMT_RING_SIZE - _pAd->MgmtRing.TxCpuIdx - 1);







#define RTMP_STA_ENTRY_MAC_RESET(pAd, Wcid)	\
	AsicDelWcidTab(pAd, Wcid);


#define RTMP_STA_ENTRY_ADD(pAd, pEntry)		\
	AsicUpdateRxWCIDTable(pAd, pEntry->Aid, pEntry->Addr);



#define RTMP_UPDATE_PROTECT(pAd)	\
	AsicUpdateProtect(pAd, 0, (ALLN_SETPROTECT), TRUE, 0);



#define RTMP_STA_ENTRY_KEY_DEL(pAd, BssIdx, Wcid)	\
	AsicRemovePairwiseKeyEntry(pAd, BssIdx, (UCHAR)Wcid);


#define RTMP_STA_SECURITY_INFO_ADD(pAd, apidx, KeyID, pEntry)		\
	RTMPAddWcidAttributeEntry(pAd, apidx, KeyID,			\
							pAd->SharedKey[apidx][KeyID].CipherAlg, pEntry);

#define RTMP_SECURITY_KEY_ADD(pAd, apidx, KeyID, pEntry)		\
	{		\
		AsicAddSharedKeyEntry(pAd, apidx, KeyID,					\
						  pAd->SharedKey[apidx][KeyID].CipherAlg,		\
						  pAd->SharedKey[apidx][KeyID].Key,				\
						  pAd->SharedKey[apidx][KeyID].TxMic,			\
						  pAd->SharedKey[apidx][KeyID].RxMic);			\
					\
		RTMPAddWcidAttributeEntry(pAd, apidx, KeyID,					\
						  pAd->SharedKey[apidx][KeyID].CipherAlg,		\
						  pEntry); }



#define RTMP_ADD_BA_SESSION_TO_ASIC(_pAd, _Aid, _TID)	\
		do{					\
			UINT32	_Value = 0, _Offset;					\
			_Offset = MAC_WCID_BASE + (_Aid) * HW_WCID_ENTRY_SIZE + 4;	\
			RTMP_IO_READ32((_pAd), _Offset, &_Value);\
			_Value |= (0x10000<<(_TID));	\
			RTMP_IO_WRITE32((_pAd), _Offset, _Value);\
		}while(0)




#define RTMP_DEL_BA_SESSION_FROM_ASIC(_pAd, _Wcid, _TID)				\
		do{								\
			UINT32	_Value = 0, _Offset;				\
			_Offset = MAC_WCID_BASE + (_Wcid) * HW_WCID_ENTRY_SIZE + 4;	\
			RTMP_IO_READ32((_pAd), _Offset, &_Value);			\
			_Value &= (~(0x10000 << (_TID)));				\
			RTMP_IO_WRITE32((_pAd), _Offset, _Value);			\
		}while(0)








#define RTMP_ASIC_INTERRUPT_DISABLE(_pAd)		\
	do{			\
		RTMP_IO_WRITE32((_pAd), INT_MASK_CSR, 0x0);     	\
		RTMP_CLEAR_FLAG((_pAd), fRTMP_ADAPTER_INTERRUPT_ACTIVE);		\
	}while(0)

#define RTMP_ASIC_INTERRUPT_ENABLE(_pAd)\
	do{				\
		RTMP_IO_WRITE32((_pAd), INT_MASK_CSR, (_pAd)->int_enable_reg );     	\
		RTMP_SET_FLAG((_pAd), fRTMP_ADAPTER_INTERRUPT_ACTIVE);	\
	}while(0)


#define RTMP_IRQ_INIT(pAd)	\
	{	pAd->int_enable_reg = ((DELAYINTMASK) |		\
					(RxINT|TxDataInt|TxMgmtInt)) & ~(0x03);	\
		pAd->int_disable_mask = 0;						\
		pAd->int_pending = 0; }

#define RTMP_IRQ_ENABLE(pAd)					\
	{				\
		RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, 0xffffffff);\
		RTMP_ASIC_INTERRUPT_ENABLE(pAd); }



#define RTMP_MLME_HANDLER(pAd)			MlmeHandler(pAd)

#define RTMP_MLME_PRE_SANITY_CHECK(pAd)

#define RTMP_MLME_STA_QUICK_RSP_WAKE_UP(pAd)	\
		RTMPSetTimer(&pAd->StaCfg.StaQuickResponeForRateUpTimer, 100);

#define RTMP_MLME_RESET_STATE_MACHINE(pAd)	\
		MlmeRestartStateMachine(pAd)

#define RTMP_HANDLE_COUNTER_MEASURE(_pAd, _pEntry)\
		HandleCounterMeasure(_pAd, _pEntry)


#define RTMP_PS_POLL_ENQUEUE(pAd)				EnqueuePsPoll(pAd)



#define RESTORE_HALT		1
#define RESTORE_WAKEUP		2
#define RESTORE_CLOSE           3

#define PowerSafeCID		1
#define PowerRadioOffCID	2
#define PowerWakeCID		3
#define CID0MASK		0x000000ff
#define CID1MASK		0x0000ff00
#define CID2MASK		0x00ff0000
#define CID3MASK		0xff000000


#ifdef CONFIG_STA_SUPPORT
#define RTMP_STA_FORCE_WAKEUP(pAd, bFromTx) \
    RT28xxPciStaAsicForceWakeup(pAd, bFromTx);

#define RTMP_STA_SLEEP_THEN_AUTO_WAKEUP(pAd, TbttNumToNextWakeUp) \
    RT28xxPciStaAsicSleepThenAutoWakeup(pAd, TbttNumToNextWakeUp);

#define RTMP_SET_PSM_BIT(_pAd, _val) \
	MlmeSetPsmBit(_pAd, _val);
#endif 

#define RTMP_MLME_RADIO_ON(pAd) \
    RT28xxPciMlmeRadioOn(pAd);

#define RTMP_MLME_RADIO_OFF(pAd) \
    RT28xxPciMlmeRadioOFF(pAd);

#endif 
