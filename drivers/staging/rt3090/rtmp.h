
#ifndef __RTMP_H__
#define __RTMP_H__

#include "link_list.h"
#include "spectrum_def.h"

#include "rtmp_dot11.h"

#ifdef MLME_EX
#include "mlme_ex_def.h"
#endif 

#ifdef CONFIG_STA_SUPPORT
#endif 

#undef AP_WSC_INCLUDED
#undef STA_WSC_INCLUDED
#undef WSC_INCLUDED


#ifdef CONFIG_STA_SUPPORT
#endif 

#if defined(AP_WSC_INCLUDED) || defined(STA_WSC_INCLUDED)
#define WSC_INCLUDED
#endif





#include "rtmp_chip.h"



typedef struct _RTMP_ADAPTER		RTMP_ADAPTER;
typedef struct _RTMP_ADAPTER		*PRTMP_ADAPTER;

typedef struct _RTMP_CHIP_OP_ RTMP_CHIP_OP;








#define MAX_DATAMM_RETRY	3
#define MGMT_USE_QUEUE_FLAG	0x80


#define	MAXSEQ		(0xFFF)


#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_STA_SUPPORT)
#define IF_DEV_CONFIG_OPMODE_ON_AP(_pAd)	if(_pAd->OpMode == OPMODE_AP)
#define IF_DEV_CONFIG_OPMODE_ON_STA(_pAd)	if(_pAd->OpMode == OPMODE_STA)
#else
#define IF_DEV_CONFIG_OPMODE_ON_AP(_pAd)
#define IF_DEV_CONFIG_OPMODE_ON_STA(_pAd)
#endif

extern  unsigned char   SNAP_AIRONET[];
extern  unsigned char   CISCO_OUI[];
extern  UCHAR	BaSizeArray[4];

extern UCHAR BROADCAST_ADDR[MAC_ADDR_LEN];
extern UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN];
extern ULONG BIT32[32];
extern UCHAR BIT8[8];
extern char* CipherName[];
extern char* MCSToMbps[];
extern UCHAR	 RxwiMCSToOfdmRate[12];
extern UCHAR SNAP_802_1H[6];
extern UCHAR SNAP_BRIDGE_TUNNEL[6];
extern UCHAR SNAP_AIRONET[8];
extern UCHAR CKIP_LLC_SNAP[8];
extern UCHAR EAPOL_LLC_SNAP[8];
extern UCHAR EAPOL[2];
extern UCHAR IPX[2];
extern UCHAR APPLE_TALK[2];
extern UCHAR RateIdToPlcpSignal[12]; 
extern UCHAR	 OfdmRateToRxwiMCS[];
extern UCHAR OfdmSignalToRateId[16] ;
extern UCHAR default_cwmin[4];
extern UCHAR default_cwmax[4];
extern UCHAR default_sta_aifsn[4];
extern UCHAR MapUserPriorityToAccessCategory[8];

extern USHORT RateUpPER[];
extern USHORT RateDownPER[];
extern UCHAR  Phy11BNextRateDownward[];
extern UCHAR  Phy11BNextRateUpward[];
extern UCHAR  Phy11BGNextRateDownward[];
extern UCHAR  Phy11BGNextRateUpward[];
extern UCHAR  Phy11ANextRateDownward[];
extern UCHAR  Phy11ANextRateUpward[];
extern CHAR   RssiSafeLevelForTxRate[];
extern UCHAR  RateIdToMbps[];
extern USHORT RateIdTo500Kbps[];

extern UCHAR  CipherSuiteWpaNoneTkip[];
extern UCHAR  CipherSuiteWpaNoneTkipLen;

extern UCHAR  CipherSuiteWpaNoneAes[];
extern UCHAR  CipherSuiteWpaNoneAesLen;

extern UCHAR  SsidIe;
extern UCHAR  SupRateIe;
extern UCHAR  ExtRateIe;

#ifdef DOT11_N_SUPPORT
extern UCHAR  HtCapIe;
extern UCHAR  AddHtInfoIe;
extern UCHAR  NewExtChanIe;
#ifdef DOT11N_DRAFT3
extern UCHAR  ExtHtCapIe;
#endif 
#endif 

extern UCHAR  ErpIe;
extern UCHAR  DsIe;
extern UCHAR  TimIe;
extern UCHAR  WpaIe;
extern UCHAR  Wpa2Ie;
extern UCHAR  IbssIe;
extern UCHAR  Ccx2Ie;
extern UCHAR  WapiIe;

extern UCHAR  WPA_OUI[];
extern UCHAR  RSN_OUI[];
extern UCHAR  WAPI_OUI[];
extern UCHAR  WME_INFO_ELEM[];
extern UCHAR  WME_PARM_ELEM[];
extern UCHAR  Ccx2QosInfo[];
extern UCHAR  Ccx2IeInfo[];
extern UCHAR  RALINK_OUI[];
extern UCHAR  PowerConstraintIE[];


extern UCHAR  RateSwitchTable[];
extern UCHAR  RateSwitchTable11B[];
extern UCHAR  RateSwitchTable11G[];
extern UCHAR  RateSwitchTable11BG[];

#ifdef DOT11_N_SUPPORT
extern UCHAR  RateSwitchTable11BGN1S[];
extern UCHAR  RateSwitchTable11BGN2S[];
extern UCHAR  RateSwitchTable11BGN2SForABand[];
extern UCHAR  RateSwitchTable11N1S[];
extern UCHAR  RateSwitchTable11N2S[];
extern UCHAR  RateSwitchTable11N2SForABand[];

#ifdef CONFIG_STA_SUPPORT
extern UCHAR  PRE_N_HT_OUI[];
#endif 
#endif 


#ifdef RALINK_ATE
typedef	struct _ATE_INFO {
	UCHAR	Mode;
	CHAR	TxPower0;
	CHAR	TxPower1;
	CHAR    TxAntennaSel;
	CHAR    RxAntennaSel;
	TXWI_STRUC  TxWI;	  
	USHORT	QID;
	UCHAR	Addr1[MAC_ADDR_LEN];
	UCHAR	Addr2[MAC_ADDR_LEN];
	UCHAR	Addr3[MAC_ADDR_LEN];
	UCHAR	Channel;
	UINT32	TxLength;
	UINT32	TxCount;
	UINT32	TxDoneCount; 
	UINT32	RFFreqOffset;
	BOOLEAN	bRxFER;		
	BOOLEAN	bQATxStart; 
	BOOLEAN	bQARxStart;	
#ifdef RTMP_MAC_PCI
	BOOLEAN	bFWLoading;	
#endif 
	UINT32	RxTotalCnt;
	UINT32	RxCntPerSec;

	CHAR	LastSNR0;             
	CHAR    LastSNR1;             
	CHAR    LastRssi0;            
	CHAR    LastRssi1;            
	CHAR    LastRssi2;            
	CHAR    AvgRssi0;             
	CHAR    AvgRssi1;             
	CHAR    AvgRssi2;             
	SHORT   AvgRssi0X8;           
	SHORT   AvgRssi1X8;           
	SHORT   AvgRssi2X8;           

	UINT32	NumOfAvgRssiSample;


#ifdef RALINK_28xx_QA
	
	USHORT		HLen; 
	USHORT		PLen; 
	UCHAR		Header[32]; 
	UCHAR		Pattern[32]; 
	USHORT		DLen; 
	USHORT		seq;
	UINT32		CID;
	RTMP_OS_PID	AtePid;
	
	UINT32		U2M;
	UINT32		OtherData;
	UINT32		Beacon;
	UINT32		OtherCount;
	UINT32		TxAc0;
	UINT32		TxAc1;
	UINT32		TxAc2;
	UINT32		TxAc3;
	
	UINT32		TxMgmt;
	UINT32		RSSI0;
	UINT32		RSSI1;
	UINT32		RSSI2;
	UINT32		SNR0;
	UINT32		SNR1;
	
	UCHAR		TxStatus;
#endif 
}	ATE_INFO, *PATE_INFO;

#ifdef RALINK_28xx_QA
struct ate_racfghdr {
	UINT32		magic_no;
	USHORT		command_type;
	USHORT		command_id;
	USHORT		length;
	USHORT		sequence;
	USHORT		status;
	UCHAR		data[2046];
}  __attribute__((packed));
#endif 
#endif 


typedef struct	_RSSI_SAMPLE {
	CHAR			LastRssi0;             
	CHAR			LastRssi1;             
	CHAR			LastRssi2;             
	CHAR			AvgRssi0;
	CHAR			AvgRssi1;
	CHAR			AvgRssi2;
	SHORT			AvgRssi0X8;
	SHORT			AvgRssi1X8;
	SHORT			AvgRssi2X8;
} RSSI_SAMPLE;




typedef struct  _QUEUE_ENTRY    {
	struct _QUEUE_ENTRY     *Next;
}   QUEUE_ENTRY, *PQUEUE_ENTRY;


typedef struct  _QUEUE_HEADER   {
	PQUEUE_ENTRY    Head;
	PQUEUE_ENTRY    Tail;
	ULONG           Number;
}   QUEUE_HEADER, *PQUEUE_HEADER;

#define InitializeQueueHeader(QueueHeader)              \
{                                                       \
	(QueueHeader)->Head = (QueueHeader)->Tail = NULL;   \
	(QueueHeader)->Number = 0;                          \
}

#define RemoveHeadQueue(QueueHeader)                \
(QueueHeader)->Head;                                \
{                                                   \
	PQUEUE_ENTRY pNext;                             \
	if ((QueueHeader)->Head != NULL)				\
	{												\
		pNext = (QueueHeader)->Head->Next;          \
		(QueueHeader)->Head->Next = NULL;		\
		(QueueHeader)->Head = pNext;                \
		if (pNext == NULL)                          \
			(QueueHeader)->Tail = NULL;             \
		(QueueHeader)->Number--;                    \
	}												\
}

#define InsertHeadQueue(QueueHeader, QueueEntry)            \
{                                                           \
		((PQUEUE_ENTRY)QueueEntry)->Next = (QueueHeader)->Head; \
		(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
		if ((QueueHeader)->Tail == NULL)                        \
			(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);   \
		(QueueHeader)->Number++;                                \
}

#define InsertTailQueue(QueueHeader, QueueEntry)				\
{                                                               \
	((PQUEUE_ENTRY)QueueEntry)->Next = NULL;                    \
	if ((QueueHeader)->Tail)                                    \
		(QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry); \
	else                                                        \
		(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
	(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);           \
	(QueueHeader)->Number++;                                    \
}

#define InsertTailQueueAc(pAd, pEntry, QueueHeader, QueueEntry)			\
{																		\
	((PQUEUE_ENTRY)QueueEntry)->Next = NULL;							\
	if ((QueueHeader)->Tail)											\
		(QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry);			\
	else																\
		(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);				\
	(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);					\
	(QueueHeader)->Number++;											\
}






#define RTMP_SET_FLAG(_M, _F)       ((_M)->Flags |= (_F))
#define RTMP_CLEAR_FLAG(_M, _F)     ((_M)->Flags &= ~(_F))
#define RTMP_CLEAR_FLAGS(_M)        ((_M)->Flags = 0)
#define RTMP_TEST_FLAG(_M, _F)      (((_M)->Flags & (_F)) != 0)
#define RTMP_TEST_FLAGS(_M, _F)     (((_M)->Flags & (_F)) == (_F))

#define RTMP_SET_PSFLAG(_M, _F)       ((_M)->PSFlags |= (_F))
#define RTMP_CLEAR_PSFLAG(_M, _F)     ((_M)->PSFlags &= ~(_F))
#define RTMP_CLEAR_PSFLAGS(_M)        ((_M)->PSFlags = 0)
#define RTMP_TEST_PSFLAG(_M, _F)      (((_M)->PSFlags & (_F)) != 0)
#define RTMP_TEST_PSFLAGS(_M, _F)     (((_M)->PSFlags & (_F)) == (_F))

#define OPSTATUS_SET_FLAG(_pAd, _F)     ((_pAd)->CommonCfg.OpStatusFlags |= (_F))
#define OPSTATUS_CLEAR_FLAG(_pAd, _F)   ((_pAd)->CommonCfg.OpStatusFlags &= ~(_F))
#define OPSTATUS_TEST_FLAG(_pAd, _F)    (((_pAd)->CommonCfg.OpStatusFlags & (_F)) != 0)

#define CLIENT_STATUS_SET_FLAG(_pEntry,_F)      ((_pEntry)->ClientStatusFlags |= (_F))
#define CLIENT_STATUS_CLEAR_FLAG(_pEntry,_F)    ((_pEntry)->ClientStatusFlags &= ~(_F))
#define CLIENT_STATUS_TEST_FLAG(_pEntry,_F)     (((_pEntry)->ClientStatusFlags & (_F)) != 0)

#define RX_FILTER_SET_FLAG(_pAd, _F)    ((_pAd)->CommonCfg.PacketFilter |= (_F))
#define RX_FILTER_CLEAR_FLAG(_pAd, _F)  ((_pAd)->CommonCfg.PacketFilter &= ~(_F))
#define RX_FILTER_TEST_FLAG(_pAd, _F)   (((_pAd)->CommonCfg.PacketFilter & (_F)) != 0)

#ifdef CONFIG_STA_SUPPORT
#define STA_NO_SECURITY_ON(_p)          (_p->StaCfg.WepStatus == Ndis802_11EncryptionDisabled)
#define STA_WEP_ON(_p)                  (_p->StaCfg.WepStatus == Ndis802_11Encryption1Enabled)
#define STA_TKIP_ON(_p)                 (_p->StaCfg.WepStatus == Ndis802_11Encryption2Enabled)
#define STA_AES_ON(_p)                  (_p->StaCfg.WepStatus == Ndis802_11Encryption3Enabled)

#define STA_TGN_WIFI_ON(_p)             (_p->StaCfg.bTGnWifiTest == TRUE)
#endif 

#define CKIP_KP_ON(_p)				((((_p)->StaCfg.CkipFlag) & 0x10) && ((_p)->StaCfg.bCkipCmicOn == TRUE))
#define CKIP_CMIC_ON(_p)			((((_p)->StaCfg.CkipFlag) & 0x08) && ((_p)->StaCfg.bCkipCmicOn == TRUE))


#define INC_RING_INDEX(_idx, _RingSize)    \
{                                          \
    (_idx) = (_idx+1) % (_RingSize);       \
}


#ifdef DOT11_N_SUPPORT

#define COPY_HTSETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(_pAd)                                 \
{                                                                                       \
	_pAd->StaActive.SupportedHtPhy.ChannelWidth = _pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth;      \
	_pAd->StaActive.SupportedHtPhy.MimoPs = _pAd->MlmeAux.HtCapability.HtCapInfo.MimoPs;      \
	_pAd->StaActive.SupportedHtPhy.GF = _pAd->MlmeAux.HtCapability.HtCapInfo.GF;      \
	_pAd->StaActive.SupportedHtPhy.ShortGIfor20 = _pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor20;      \
	_pAd->StaActive.SupportedHtPhy.ShortGIfor40 = _pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor40;      \
	_pAd->StaActive.SupportedHtPhy.TxSTBC = _pAd->MlmeAux.HtCapability.HtCapInfo.TxSTBC;      \
	_pAd->StaActive.SupportedHtPhy.RxSTBC = _pAd->MlmeAux.HtCapability.HtCapInfo.RxSTBC;      \
	_pAd->StaActive.SupportedHtPhy.ExtChanOffset = _pAd->MlmeAux.AddHtInfo.AddHtInfo.ExtChanOffset;      \
	_pAd->StaActive.SupportedHtPhy.RecomWidth = _pAd->MlmeAux.AddHtInfo.AddHtInfo.RecomWidth;      \
	_pAd->StaActive.SupportedHtPhy.OperaionMode = _pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode;      \
	_pAd->StaActive.SupportedHtPhy.NonGfPresent = _pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent;      \
	NdisMoveMemory((_pAd)->MacTab.Content[BSSID_WCID].HTCapability.MCSSet, (_pAd)->StaActive.SupportedPhyInfo.MCSSet, sizeof(UCHAR) * 16);\
}

#define COPY_AP_HTSETTINGS_FROM_BEACON(_pAd, _pHtCapability)                                 \
{                                                                                       \
	_pAd->MacTab.Content[BSSID_WCID].AMsduSize = (UCHAR)(_pHtCapability->HtCapInfo.AMsduSize);	\
	_pAd->MacTab.Content[BSSID_WCID].MmpsMode= (UCHAR)(_pHtCapability->HtCapInfo.MimoPs);	\
	_pAd->MacTab.Content[BSSID_WCID].MaxRAmpduFactor = (UCHAR)(_pHtCapability->HtCapParm.MaxRAmpduFactor);	\
}
#endif 





















#define NIC_MAX_PHYS_BUF_COUNT              8

typedef struct _RTMP_SCATTER_GATHER_ELEMENT {
    PVOID		Address;
    ULONG		Length;
    PULONG		Reserved;
} RTMP_SCATTER_GATHER_ELEMENT, *PRTMP_SCATTER_GATHER_ELEMENT;


typedef struct _RTMP_SCATTER_GATHER_LIST {
    ULONG  NumberOfElements;
    PULONG Reserved;
    RTMP_SCATTER_GATHER_ELEMENT Elements[NIC_MAX_PHYS_BUF_COUNT];
} RTMP_SCATTER_GATHER_LIST, *PRTMP_SCATTER_GATHER_LIST;





#ifndef min
#define min(_a, _b)     (((_a) < (_b)) ? (_a) : (_b))
#endif

#ifndef max
#define max(_a, _b)     (((_a) > (_b)) ? (_a) : (_b))
#endif

#define GET_LNA_GAIN(_pAd)	((_pAd->LatchRfRegs.Channel <= 14) ? (_pAd->BLNAGain) : ((_pAd->LatchRfRegs.Channel <= 64) ? (_pAd->ALNAGain0) : ((_pAd->LatchRfRegs.Channel <= 128) ? (_pAd->ALNAGain1) : (_pAd->ALNAGain2))))

#define INC_COUNTER64(Val)          (Val.QuadPart++)

#define INFRA_ON(_p)                (OPSTATUS_TEST_FLAG(_p, fOP_STATUS_INFRA_ON))
#define ADHOC_ON(_p)                (OPSTATUS_TEST_FLAG(_p, fOP_STATUS_ADHOC_ON))
#define MONITOR_ON(_p)              (((_p)->StaCfg.BssType) == BSS_MONITOR)
#define IDLE_ON(_p)                 (!INFRA_ON(_p) && !ADHOC_ON(_p))


#define LEAP_ON(_p)                 (((_p)->StaCfg.LeapAuthMode) == CISCO_AuthModeLEAP)
#define LEAP_CCKM_ON(_p)            ((((_p)->StaCfg.LeapAuthMode) == CISCO_AuthModeLEAP) && ((_p)->StaCfg.LeapAuthInfo.CCKM == TRUE))


#define EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(_pBufVA, _pExtraLlcSnapEncap)		\
{																\
	if (((*(_pBufVA + 12) << 8) + *(_pBufVA + 13)) > 1500)		\
	{															\
		_pExtraLlcSnapEncap = SNAP_802_1H;						\
		if (NdisEqualMemory(IPX, _pBufVA + 12, 2) ||			\
			NdisEqualMemory(APPLE_TALK, _pBufVA + 12, 2))		\
		{														\
			_pExtraLlcSnapEncap = SNAP_BRIDGE_TUNNEL;			\
		}														\
	}															\
	else														\
	{															\
		_pExtraLlcSnapEncap = NULL;								\
	}															\
}


#define EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(_pBufVA, _pExtraLlcSnapEncap)	\
{																\
	if (((*(_pBufVA) << 8) + *(_pBufVA + 1)) > 1500)			\
	{															\
		_pExtraLlcSnapEncap = SNAP_802_1H;						\
		if (NdisEqualMemory(IPX, _pBufVA, 2) ||					\
			NdisEqualMemory(APPLE_TALK, _pBufVA, 2))			\
		{														\
			_pExtraLlcSnapEncap = SNAP_BRIDGE_TUNNEL;			\
		}														\
	}															\
	else														\
	{															\
		_pExtraLlcSnapEncap = NULL;								\
	}															\
}


#define MAKE_802_3_HEADER(_p, _pMac1, _pMac2, _pType)                   \
{                                                                       \
    NdisMoveMemory(_p, _pMac1, MAC_ADDR_LEN);                           \
    NdisMoveMemory((_p + MAC_ADDR_LEN), _pMac2, MAC_ADDR_LEN);          \
    NdisMoveMemory((_p + MAC_ADDR_LEN * 2), _pType, LENGTH_802_3_TYPE); \
}








#define CONVERT_TO_802_3(_p8023hdr, _pDA, _pSA, _pData, _DataSize, _pRemovedLLCSNAP)      \
{                                                                       \
    char LLC_Len[2];                                                    \
                                                                        \
    _pRemovedLLCSNAP = NULL;                                            \
    if (NdisEqualMemory(SNAP_802_1H, _pData, 6)  ||                     \
        NdisEqualMemory(SNAP_BRIDGE_TUNNEL, _pData, 6))                 \
    {                                                                   \
        PUCHAR pProto = _pData + 6;                                     \
                                                                        \
        if ((NdisEqualMemory(IPX, pProto, 2) || NdisEqualMemory(APPLE_TALK, pProto, 2)) &&  \
            NdisEqualMemory(SNAP_802_1H, _pData, 6))                    \
        {                                                               \
            LLC_Len[0] = (UCHAR)(_DataSize / 256);                      \
            LLC_Len[1] = (UCHAR)(_DataSize % 256);                      \
            MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, LLC_Len);          \
        }                                                               \
        else                                                            \
        {                                                               \
            MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, pProto);           \
            _pRemovedLLCSNAP = _pData;                                  \
            _DataSize -= LENGTH_802_1_H;                                \
            _pData += LENGTH_802_1_H;                                   \
        }                                                               \
    }                                                                   \
    else                                                                \
    {                                                                   \
        LLC_Len[0] = (UCHAR)(_DataSize / 256);                          \
        LLC_Len[1] = (UCHAR)(_DataSize % 256);                          \
        MAKE_802_3_HEADER(_p8023hdr, _pDA, _pSA, LLC_Len);              \
    }                                                                   \
}





#ifdef RTMP_MAC_PCI
#define REPORT_MGMT_FRAME_TO_MLME(_pAd, Wcid, _pFrame, _FrameSize, _Rssi0, _Rssi1, _Rssi2, _PlcpSignal)        \
{                                                                                       \
    UINT32 High32TSF, Low32TSF;                                                          \
    RTMP_IO_READ32(_pAd, TSF_TIMER_DW1, &High32TSF);                                       \
    RTMP_IO_READ32(_pAd, TSF_TIMER_DW0, &Low32TSF);                                        \
    MlmeEnqueueForRecv(_pAd, Wcid, High32TSF, Low32TSF, (UCHAR)_Rssi0, (UCHAR)_Rssi1,(UCHAR)_Rssi2,_FrameSize, _pFrame, (UCHAR)_PlcpSignal);   \
}
#endif 

#define MAC_ADDR_EQUAL(pAddr1,pAddr2)           RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), MAC_ADDR_LEN)
#define SSID_EQUAL(ssid1, len1, ssid2, len2)    ((len1==len2) && (RTMPEqualMemory(ssid1, ssid2, len1)))




#define JapanChannelCheck(channel)  ((channel == 52) || (channel == 56) || (channel == 60) || (channel == 64))

#ifdef CONFIG_STA_SUPPORT
#define STA_EXTRA_SETTING(_pAd)

#define STA_PORT_SECURED(_pAd) \
{ \
	BOOLEAN	Cancelled; \
	(_pAd)->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED; \
	NdisAcquireSpinLock(&((_pAd)->MacTabLock)); \
	(_pAd)->MacTab.Content[BSSID_WCID].PortSecured = (_pAd)->StaCfg.PortSecured; \
	(_pAd)->MacTab.Content[BSSID_WCID].PrivacyFilter = Ndis802_11PrivFilterAcceptAll;\
	NdisReleaseSpinLock(&(_pAd)->MacTabLock); \
	RTMPCancelTimer(&((_pAd)->Mlme.LinkDownTimer), &Cancelled);\
	STA_EXTRA_SETTING(_pAd); \
}
#endif 







typedef struct  _RTMP_DMABUF
{
	ULONG                   AllocSize;
	PVOID                   AllocVa;            
	NDIS_PHYSICAL_ADDRESS   AllocPa;            
} RTMP_DMABUF, *PRTMP_DMABUF;










typedef struct _RTMP_DMACB
{
	ULONG                   AllocSize;          
	PVOID                   AllocVa;            
	NDIS_PHYSICAL_ADDRESS   AllocPa;            
	PNDIS_PACKET pNdisPacket;
	PNDIS_PACKET pNextNdisPacket;

	RTMP_DMABUF             DmaBuf;             
} RTMP_DMACB, *PRTMP_DMACB;


typedef struct _RTMP_TX_RING
{
	RTMP_DMACB  Cell[TX_RING_SIZE];
	UINT32		TxCpuIdx;
	UINT32		TxDmaIdx;
	UINT32		TxSwFreeIdx;	
} RTMP_TX_RING, *PRTMP_TX_RING;

typedef struct _RTMP_RX_RING
{
	RTMP_DMACB  Cell[RX_RING_SIZE];
	UINT32		RxCpuIdx;
	UINT32		RxDmaIdx;
	INT32		RxSwReadIdx;	
} RTMP_RX_RING, *PRTMP_RX_RING;

typedef struct _RTMP_MGMT_RING
{
	RTMP_DMACB  Cell[MGMT_RING_SIZE];
	UINT32		TxCpuIdx;
	UINT32		TxDmaIdx;
	UINT32		TxSwFreeIdx; 
} RTMP_MGMT_RING, *PRTMP_MGMT_RING;





typedef struct _COUNTER_802_3
{
	
	ULONG       GoodTransmits;
	ULONG       GoodReceives;
	ULONG       TxErrors;
	ULONG       RxErrors;
	ULONG       RxNoBuffer;

	
	ULONG       RcvAlignmentErrors;
	ULONG       OneCollision;
	ULONG       MoreCollisions;

} COUNTER_802_3, *PCOUNTER_802_3;

typedef struct _COUNTER_802_11 {
	ULONG           Length;
	LARGE_INTEGER   LastTransmittedFragmentCount;
	LARGE_INTEGER   TransmittedFragmentCount;
	LARGE_INTEGER   MulticastTransmittedFrameCount;
	LARGE_INTEGER   FailedCount;
	LARGE_INTEGER   RetryCount;
	LARGE_INTEGER   MultipleRetryCount;
	LARGE_INTEGER   RTSSuccessCount;
	LARGE_INTEGER   RTSFailureCount;
	LARGE_INTEGER   ACKFailureCount;
	LARGE_INTEGER   FrameDuplicateCount;
	LARGE_INTEGER   ReceivedFragmentCount;
	LARGE_INTEGER   MulticastReceivedFrameCount;
	LARGE_INTEGER   FCSErrorCount;
} COUNTER_802_11, *PCOUNTER_802_11;

typedef struct _COUNTER_RALINK {
	ULONG           TransmittedByteCount;   
	ULONG           ReceivedByteCount;      
	ULONG           BeenDisassociatedCount;
	ULONG           BadCQIAutoRecoveryCount;
	ULONG           PoorCQIRoamingCount;
	ULONG           MgmtRingFullCount;
	ULONG           RxCountSinceLastNULL;
	ULONG           RxCount;
	ULONG           RxRingErrCount;
	ULONG           KickTxCount;
	ULONG           TxRingErrCount;
	LARGE_INTEGER   RealFcsErrCount;
	ULONG           PendingNdisPacketCount;

	ULONG           OneSecOsTxCount[NUM_OF_TX_RING];
	ULONG           OneSecDmaDoneCount[NUM_OF_TX_RING];
	UINT32          OneSecTxDoneCount;
	ULONG           OneSecRxCount;
	UINT32          OneSecTxAggregationCount;
	UINT32          OneSecRxAggregationCount;
	UINT32          OneSecReceivedByteCount;
	UINT32			OneSecFrameDuplicateCount;

	UINT32          OneSecTransmittedByteCount;   
	UINT32          OneSecTxNoRetryOkCount;
	UINT32          OneSecTxRetryOkCount;
	UINT32          OneSecTxFailCount;
	UINT32          OneSecFalseCCACnt;      
	UINT32          OneSecRxOkCnt;          
	UINT32          OneSecRxOkDataCnt;      
	UINT32          OneSecRxFcsErrCnt;      
	UINT32          OneSecBeaconSentCnt;
	UINT32          LastOneSecTotalTxCount; 
	UINT32          LastOneSecRxOkDataCnt;  
	ULONG		DuplicateRcv;
	ULONG		TxAggCount;
	ULONG		TxNonAggCount;
	ULONG		TxAgg1MPDUCount;
	ULONG		TxAgg2MPDUCount;
	ULONG		TxAgg3MPDUCount;
	ULONG		TxAgg4MPDUCount;
	ULONG		TxAgg5MPDUCount;
	ULONG		TxAgg6MPDUCount;
	ULONG		TxAgg7MPDUCount;
	ULONG		TxAgg8MPDUCount;
	ULONG		TxAgg9MPDUCount;
	ULONG		TxAgg10MPDUCount;
	ULONG		TxAgg11MPDUCount;
	ULONG		TxAgg12MPDUCount;
	ULONG		TxAgg13MPDUCount;
	ULONG		TxAgg14MPDUCount;
	ULONG		TxAgg15MPDUCount;
	ULONG		TxAgg16MPDUCount;

	LARGE_INTEGER       TransmittedOctetsInAMSDU;
	LARGE_INTEGER       TransmittedAMSDUCount;
	LARGE_INTEGER       ReceivedOctesInAMSDUCount;
	LARGE_INTEGER       ReceivedAMSDUCount;
	LARGE_INTEGER       TransmittedAMPDUCount;
	LARGE_INTEGER       TransmittedMPDUsInAMPDUCount;
	LARGE_INTEGER       TransmittedOctetsInAMPDUCount;
	LARGE_INTEGER       MPDUInReceivedAMPDUCount;
} COUNTER_RALINK, *PCOUNTER_RALINK;


typedef struct _COUNTER_DRS {
	
	USHORT          TxQuality[MAX_STEP_OF_TX_RATE_SWITCH];
	UCHAR           PER[MAX_STEP_OF_TX_RATE_SWITCH];
	UCHAR           TxRateUpPenalty;      
	ULONG           CurrTxRateStableTime; 
	BOOLEAN         fNoisyEnvironment;
	BOOLEAN         fLastSecAccordingRSSI;
	UCHAR           LastSecTxRateChangeAction; 
	UCHAR			LastTimeTxRateChangeAction; 
	ULONG			LastTxOkCount;
} COUNTER_DRS, *PCOUNTER_DRS;





typedef struct _CIPHER_KEY {
	UCHAR   Key[16];            
	UCHAR   RxMic[8];			
	UCHAR   TxMic[8];
	UCHAR   TxTsc[6];           
	UCHAR   RxTsc[6];           
	UCHAR   CipherAlg;          
	UCHAR   KeyLen;
#ifdef CONFIG_STA_SUPPORT
	UCHAR   BssId[6];
#endif 
            
	UCHAR   Type;               
} CIPHER_KEY, *PCIPHER_KEY;



typedef struct PACKED _RT_802_11_WPA_REKEY {
	ULONG ReKeyMethod;          
	ULONG ReKeyInterval;        
} RT_WPA_REKEY,*PRT_WPA_REKEY, RT_802_11_WPA_REKEY, *PRT_802_11_WPA_REKEY;



typedef struct {
	UCHAR        Addr[MAC_ADDR_LEN];
	UCHAR        ErrorCode[2];  
							
							
							
	BOOLEAN      Reported;
} ROGUEAP_ENTRY, *PROGUEAP_ENTRY;

typedef struct {
	UCHAR               RogueApNr;
	ROGUEAP_ENTRY       RogueApEntry[MAX_LEN_OF_BSS_TABLE];
} ROGUEAP_TABLE, *PROGUEAP_TABLE;




typedef struct  _CISCO_IAPP_CONTENT_
{
	USHORT     Length;        
	UCHAR      MessageType;      
	UCHAR      FunctionCode;     
	UCHAR      DestinaionMAC[MAC_ADDR_LEN];
	UCHAR      SourceMAC[MAC_ADDR_LEN];
	USHORT     Tag;           
	USHORT     TagLength;     
	UCHAR      OUI[4];           
	UCHAR      PreviousAP[MAC_ADDR_LEN];       
	USHORT     Channel;
	USHORT     SsidLen;
	UCHAR      Ssid[MAX_LEN_OF_SSID];
	USHORT     Seconds;          
} CISCO_IAPP_CONTENT, *PCISCO_IAPP_CONTENT;



typedef struct  _FRAGMENT_FRAME {
	PNDIS_PACKET    pFragPacket;
	ULONG       RxSize;
	USHORT      Sequence;
	USHORT      LastFrag;
	ULONG       Flags;          
} FRAGMENT_FRAME, *PFRAGMENT_FRAME;





typedef struct  _PACKET_INFO    {
	UINT            PhysicalBufferCount;    
	UINT            BufferCount ;           
	UINT            TotalPacketLength ;     
	PNDIS_BUFFER    pFirstBuffer;           
} PACKET_INFO, *PPACKET_INFO;





typedef struct  _ARCFOUR
{
	UINT            X;
	UINT            Y;
	UCHAR           STATE[256];
} ARCFOURCONTEXT, *PARCFOURCONTEXT;





typedef struct  _TKIP_KEY_INFO  {
	UINT        nBytesInM;  
	ULONG       IV16;
	ULONG       IV32;
	ULONG       K0;         
	ULONG       K1;         
	ULONG       L;          
	ULONG       R;          
	ULONG       M;          
	UCHAR       RC4KEY[16];
	UCHAR       MIC[8];
} TKIP_KEY_INFO, *PTKIP_KEY_INFO;





typedef struct  __PRIVATE_STRUC {
	UINT       SystemResetCnt;         
	UINT       TxRingFullCnt;          
	UINT       PhyRxErrCnt;            
	
	UINT       FCSCRC32;
	ARCFOURCONTEXT  WEPCONTEXT;
	
	TKIP_KEY_INFO   Tx;
	TKIP_KEY_INFO   Rx;
} PRIVATE_STRUC, *PPRIVATE_STRUC;




typedef struct _BBP_R66_TUNING {
	BOOLEAN     bEnable;
	USHORT      FalseCcaLowerThreshold;  
	USHORT      FalseCcaUpperThreshold;  
	UCHAR       R66Delta;
	UCHAR       R66CurrentValue;
	BOOLEAN		R66LowerUpperSelect; 
} BBP_R66_TUNING, *PBBP_R66_TUNING;


typedef struct _CHANNEL_TX_POWER {
	USHORT     RemainingTimeForUse;		
	UCHAR      Channel;
#ifdef DOT11N_DRAFT3
	BOOLEAN       bEffectedChannel;	
#endif 
	CHAR       Power;
	CHAR       Power2;
	UCHAR      MaxTxPwr;
	UCHAR      DfsReq;
} CHANNEL_TX_POWER, *PCHANNEL_TX_POWER;


typedef struct _CHANNEL_11J_TX_POWER {
	UCHAR      Channel;
	UCHAR      BW;	
	CHAR       Power;
	CHAR       Power2;
	USHORT     RemainingTimeForUse;		
} CHANNEL_11J_TX_POWER, *PCHANNEL_11J_TX_POWER;

typedef struct _SOFT_RX_ANT_DIVERSITY_STRUCT {
	UCHAR     EvaluatePeriod;		 
	UCHAR     EvaluateStableCnt;
	UCHAR     Pair1PrimaryRxAnt;     
	UCHAR     Pair1SecondaryRxAnt;   
	UCHAR     Pair2PrimaryRxAnt;     
	UCHAR     Pair2SecondaryRxAnt;   
#ifdef CONFIG_STA_SUPPORT
	SHORT     Pair1AvgRssi[2];       
	SHORT     Pair2AvgRssi[2];       
#endif 
	SHORT     Pair1LastAvgRssi;      
	SHORT     Pair2LastAvgRssi;      
	ULONG     RcvPktNumWhenEvaluate;
	BOOLEAN   FirstPktArrivedWhenEvaluate;
	RALINK_TIMER_STRUCT    RxAntDiversityTimer;
} SOFT_RX_ANT_DIVERSITY, *PSOFT_RX_ANT_DIVERSITY;



typedef struct _RADAR_DETECT_STRUCT {
    
	UCHAR		CSCount;			
	UCHAR		CSPeriod;			
	UCHAR		RDCount;			
	UCHAR		RDMode;				
	UCHAR		RDDurRegion;		
	UCHAR		BBPR16;
	UCHAR		BBPR17;
	UCHAR		BBPR18;
	UCHAR		BBPR21;
	UCHAR		BBPR22;
	UCHAR		BBPR64;
	ULONG		InServiceMonitorCount; 
	UINT8		DfsSessionTime;
#ifdef DFS_FCC_BW40_FIX
	CHAR		DfsSessionFccOff;
#endif
	BOOLEAN		bFastDfs;
	UINT8		ChMovingTime;
	UINT8		LongPulseRadarTh;
#ifdef MERGE_ARCH_TEAM
	CHAR		AvgRssiReq;
	ULONG		DfsLowerLimit;
	ULONG		DfsUpperLimit;
	UINT8		FixDfsLimit;
	ULONG		upperlimit;
	ULONG		lowerlimit;
#endif 
} RADAR_DETECT_STRUCT, *PRADAR_DETECT_STRUCT;

#ifdef CARRIER_DETECTION_SUPPORT
typedef enum CD_STATE_n
{
	CD_NORMAL,
	CD_SILENCE,
	CD_MAX_STATE
} CD_STATE;

#ifdef TONE_RADAR_DETECT_SUPPORT
#define CARRIER_DETECT_RECHECK_TIME			3


#ifdef CARRIER_SENSE_NEW_ALGO
#define CARRIER_DETECT_CRITIRIA				400
#define CARRIER_DETECT_STOP_RATIO				0x11
#define	CARRIER_DETECT_STOP_RATIO_OLD_3090			2
#endif 


#define CARRIER_DETECT_STOP_RECHECK_TIME		4
#define CARRIER_DETECT_CRITIRIA_A				230
#define CARRIER_DETECT_DELTA					7
#define CARRIER_DETECT_DIV_FLAG				0
#ifdef RT3090
#define CARRIER_DETECT_THRESHOLD_3090A			0x1fffffff
#endif 
#ifdef RT3390
#define CARRIER_DETECT_THRESHOLD			0x0fffffff
#endif 
#ifndef RT3390
#define CARRIER_DETECT_THRESHOLD			0x0fffffff
#endif 
#endif 

typedef struct CARRIER_DETECTION_s
{
	BOOLEAN					Enable;
	UINT8					CDSessionTime;
	UINT8					CDPeriod;
	CD_STATE				CD_State;
#ifdef TONE_RADAR_DETECT_SUPPORT
	UINT8					delta;
	UINT8					div_flag;
	UINT32					threshold;
	UINT8					recheck;
	UINT8					recheck1;
	UINT8					recheck2;
	UINT32					TimeStamp;
	UINT32					criteria;
	UINT32					CarrierDebug;
	ULONG					idle_time;
	ULONG					busy_time;
	ULONG					Debug;
#endif 
}CARRIER_DETECTION_STRUCT, *PCARRIER_DETECTION_STRUCT;
#endif 


#ifdef NEW_DFS
typedef struct _NewDFSDebug
{
	UCHAR channel;
	ULONG wait_time;
	UCHAR delta_delay_range;
	UCHAR delta_delay_step;
	UCHAR EL_range;
	UCHAR EL_step;
	UCHAR EH_range;
	UCHAR EH_step;
	UCHAR WL_range;
	UCHAR WL_step;
	UCHAR WH_range;
	UCHAR WH_step;
	ULONG T_expected;
	ULONG T_margin;
	UCHAR start;
	ULONG count;
	ULONG idx;

}NewDFSDebug, *pNewDFSDebug;

#define NEW_DFS_FCC_5_ENT_NUM		5
#define NEW_DFS_DBG_PORT_ENT_NUM_POWER	8
#define NEW_DFS_DBG_PORT_ENT_NUM		(1 << NEW_DFS_DBG_PORT_ENT_NUM_POWER)  
#define NEW_DFS_DBG_PORT_MASK	0xff


#define NEW_DFS_MPERIOD_ENT_NUM_POWER		8
#define NEW_DFS_MPERIOD_ENT_NUM		(1 << NEW_DFS_MPERIOD_ENT_NUM_POWER)	 
#define NEW_DFS_MAX_CHANNEL	4

typedef struct _NewDFSDebugPort{
	ULONG counter;
	ULONG timestamp;
	USHORT width;
	USHORT start_idx; 
	USHORT end_idx; 
}NewDFSDebugPort, *pNewDFSDebugPort;


typedef struct _NewDFSMPeriod{
	USHORT	idx;
	USHORT width;
	USHORT	idx2;
	USHORT width2;
	ULONG period;
}NewDFSMPeriod, *pNewDFSMPeriod;

#endif 


typedef enum _ABGBAND_STATE_ {
	UNKNOWN_BAND,
	BG_BAND,
	A_BAND,
} ABGBAND_STATE;

#ifdef RTMP_MAC_PCI
#ifdef CONFIG_STA_SUPPORT

typedef	union	_PS_CONTROL	{
	struct	{
		ULONG		EnablePSinIdle:1;			
		ULONG		EnableNewPS:1;		
		ULONG		rt30xxPowerMode:2;			
		ULONG		rt30xxFollowHostASPM:1;			
		ULONG		rt30xxForceASPMTest:1;			
		ULONG		rsv:26;			
	}	field;
	ULONG			word;
}	PS_CONTROL, *PPS_CONTROL;
#endif 
#endif 

typedef struct _MLME_STRUCT {
#ifdef CONFIG_STA_SUPPORT
	
	STATE_MACHINE           CntlMachine;
	STATE_MACHINE           AssocMachine;
	STATE_MACHINE           AuthMachine;
	STATE_MACHINE           AuthRspMachine;
	STATE_MACHINE           SyncMachine;
	STATE_MACHINE           WpaPskMachine;
	STATE_MACHINE           LeapMachine;
	STATE_MACHINE_FUNC      AssocFunc[ASSOC_FUNC_SIZE];
	STATE_MACHINE_FUNC      AuthFunc[AUTH_FUNC_SIZE];
	STATE_MACHINE_FUNC      AuthRspFunc[AUTH_RSP_FUNC_SIZE];
	STATE_MACHINE_FUNC      SyncFunc[SYNC_FUNC_SIZE];
#endif 
	STATE_MACHINE_FUNC      ActFunc[ACT_FUNC_SIZE];
	
	STATE_MACHINE           ActMachine;


#ifdef QOS_DLS_SUPPORT
	STATE_MACHINE			DlsMachine;
	STATE_MACHINE_FUNC      DlsFunc[DLS_FUNC_SIZE];
#endif 


	
	STATE_MACHINE           WpaMachine;
	STATE_MACHINE_FUNC      WpaFunc[WPA_FUNC_SIZE];



	ULONG                   ChannelQuality;  
	ULONG                   Now32;           
	ULONG                   LastSendNULLpsmTime;

	BOOLEAN                 bRunning;
	NDIS_SPIN_LOCK          TaskLock;
	MLME_QUEUE              Queue;

	UINT                    ShiftReg;

	RALINK_TIMER_STRUCT     PeriodicTimer;
	RALINK_TIMER_STRUCT     APSDPeriodicTimer;
	RALINK_TIMER_STRUCT     LinkDownTimer;
	RALINK_TIMER_STRUCT     LinkUpTimer;
#ifdef RTMP_MAC_PCI
    UCHAR                   bPsPollTimerRunning;
    RALINK_TIMER_STRUCT     PsPollTimer;
	RALINK_TIMER_STRUCT     RadioOnOffTimer;
#endif 
	ULONG                   PeriodicRound;
	ULONG                   OneSecPeriodicRound;

	UCHAR					RealRxPath;
	BOOLEAN					bLowThroughput;
	BOOLEAN					bEnableAutoAntennaCheck;
	RALINK_TIMER_STRUCT		RxAntEvalTimer;

#ifdef RT30xx
	UCHAR CaliBW40RfR24;
	UCHAR CaliBW20RfR24;
#endif 

} MLME_STRUCT, *PMLME_STRUCT;


#ifdef DOT11_N_SUPPORT

struct reordering_mpdu
{
	struct reordering_mpdu	*next;
	PNDIS_PACKET			pPacket;		
	int						Sequence;		
	BOOLEAN					bAMSDU;
};

struct reordering_list
{
	struct reordering_mpdu *next;
	int	qlen;
};

struct reordering_mpdu_pool
{
	PVOID					mem;
	NDIS_SPIN_LOCK			lock;
	struct reordering_list	freelist;
};

typedef enum _REC_BLOCKACK_STATUS
{
    Recipient_NONE=0,
	Recipient_USED,
	Recipient_HandleRes,
    Recipient_Accept
} REC_BLOCKACK_STATUS, *PREC_BLOCKACK_STATUS;

typedef enum _ORI_BLOCKACK_STATUS
{
    Originator_NONE=0,
	Originator_USED,
    Originator_WaitRes,
    Originator_Done
} ORI_BLOCKACK_STATUS, *PORI_BLOCKACK_STATUS;

typedef struct _BA_ORI_ENTRY{
	UCHAR   Wcid;
	UCHAR   TID;
	UCHAR   BAWinSize;
	UCHAR   Token;

	USHORT	Sequence;
	USHORT	TimeOutValue;
	ORI_BLOCKACK_STATUS  ORI_BA_Status;
	RALINK_TIMER_STRUCT ORIBATimer;
	PVOID	pAdapter;
} BA_ORI_ENTRY, *PBA_ORI_ENTRY;

typedef struct _BA_REC_ENTRY {
	UCHAR   Wcid;
	UCHAR   TID;
	UCHAR   BAWinSize;	
	
	
	USHORT		LastIndSeq;

	USHORT		TimeOutValue;
	RALINK_TIMER_STRUCT RECBATimer;
	ULONG		LastIndSeqAtTimer;
	ULONG		nDropPacket;
	ULONG		rcvSeq;
	REC_BLOCKACK_STATUS  REC_BA_Status;

	
	
	NDIS_SPIN_LOCK          RxReRingLock;                 

	PVOID	pAdapter;
	struct reordering_list	list;
} BA_REC_ENTRY, *PBA_REC_ENTRY;


typedef struct {
	ULONG		numAsRecipient;		
	ULONG		numAsOriginator;	
	ULONG		numDoneOriginator;	
	BA_ORI_ENTRY       BAOriEntry[MAX_LEN_OF_BA_ORI_TABLE];
	BA_REC_ENTRY       BARecEntry[MAX_LEN_OF_BA_REC_TABLE];
} BA_TABLE, *PBA_TABLE;


typedef struct  PACKED _OID_BA_REC_ENTRY{
	UCHAR   MACAddr[MAC_ADDR_LEN];
	UCHAR   BaBitmap;   
	UCHAR   rsv;
	UCHAR   BufSize[8];
	REC_BLOCKACK_STATUS	REC_BA_Status[8];
} OID_BA_REC_ENTRY, *POID_BA_REC_ENTRY;


typedef struct  PACKED _OID_BA_ORI_ENTRY{
	UCHAR   MACAddr[MAC_ADDR_LEN];
	UCHAR   BaBitmap;  
	UCHAR   rsv;
	UCHAR   BufSize[8];
	ORI_BLOCKACK_STATUS  ORI_BA_Status[8];
} OID_BA_ORI_ENTRY, *POID_BA_ORI_ENTRY;

typedef struct _QUERYBA_TABLE{
	OID_BA_ORI_ENTRY       BAOriEntry[32];
	OID_BA_REC_ENTRY       BARecEntry[32];
	UCHAR   OriNum;
	UCHAR   RecNum;
} QUERYBA_TABLE, *PQUERYBA_TABLE;

typedef	union	_BACAP_STRUC	{
#ifdef RT_BIG_ENDIAN
	struct	{
		UINT32     :4;
		UINT32     b2040CoexistScanSup:1;		
		UINT32     bHtAdhoc:1;			
		UINT32     MMPSmode:2;	
		UINT32     AmsduSize:1;	
		UINT32     AmsduEnable:1;	
		UINT32		MpduDensity:3;
		UINT32		Policy:2;	
		UINT32		AutoBA:1;	
		UINT32		TxBAWinLimit:8;
		UINT32		RxBAWinLimit:8;
	}	field;
#else
	struct	{
		UINT32		RxBAWinLimit:8;
		UINT32		TxBAWinLimit:8;
		UINT32		AutoBA:1;	
		UINT32		Policy:2;	
		UINT32		MpduDensity:3;
		UINT32		AmsduEnable:1;	
		UINT32		AmsduSize:1;	
		UINT32		MMPSmode:2;	
		UINT32		bHtAdhoc:1;			
		UINT32		b2040CoexistScanSup:1;		
		UINT32		:4;
	}	field;
#endif
	UINT32			word;
} BACAP_STRUC, *PBACAP_STRUC;


typedef struct {
	BOOLEAN		IsRecipient;
	UCHAR   MACAddr[MAC_ADDR_LEN];
	UCHAR   TID;
	UCHAR   nMSDU;
	USHORT   TimeOut;
	BOOLEAN bAllTid;  
} OID_ADD_BA_ENTRY, *POID_ADD_BA_ENTRY;


#ifdef DOT11N_DRAFT3
typedef enum _BSS2040COEXIST_FLAG{
	BSS_2040_COEXIST_DISABLE = 0,
	BSS_2040_COEXIST_TIMER_FIRED  = 1,
	BSS_2040_COEXIST_INFO_SYNC = 2,
	BSS_2040_COEXIST_INFO_NOTIFY = 4,
}BSS2040COEXIST_FLAG;
#endif 

#define IS_HT_STA(_pMacEntry)	\
	(_pMacEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX)

#define IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

#define PEER_IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

#endif 



typedef	struct	_IOT_STRUC	{
	UCHAR			Threshold[2];
	UCHAR			ReorderTimeOutNum[MAX_LEN_OF_BA_REC_TABLE];	
	UCHAR			RefreshNum[MAX_LEN_OF_BA_REC_TABLE];	
	ULONG			OneSecInWindowCount;
	ULONG			OneSecFrameDuplicateCount;
	ULONG			OneSecOutWindowCount;
	UCHAR			DelOriAct;
	UCHAR			DelRecAct;
	UCHAR			RTSShortProt;
	UCHAR			RTSLongProt;
	BOOLEAN			bRTSLongProtOn;
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN			bLastAtheros;
    BOOLEAN			bCurrentAtheros;
    BOOLEAN         bNowAtherosBurstOn;
	BOOLEAN			bNextDisableRxBA;
    BOOLEAN			bToggle;
#endif 
} IOT_STRUC, *PIOT_STRUC;



typedef union _REG_TRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
 struct {
         UINT32  rsv:13;
		 UINT32  EXTCHA:2;
		 UINT32  HTMODE:1;
		 UINT32  TRANSNO:2;
		 UINT32  STBC:1; 
		 UINT32  ShortGI:1;
		 UINT32  BW:1; 
		 UINT32  TxBF:1; 
		 UINT32  rsv0:10;
		 
         
    } field;
#else
 struct {
         
         
		 UINT32  rsv0:10;
		 UINT32  TxBF:1;
         UINT32  BW:1; 
         UINT32  ShortGI:1;
         UINT32  STBC:1; 
         UINT32  TRANSNO:2;
         UINT32  HTMODE:1;
         UINT32  EXTCHA:2;
         UINT32  rsv:13;
    } field;
#endif
 UINT32   word;
} REG_TRANSMIT_SETTING, *PREG_TRANSMIT_SETTING;


typedef union  _DESIRED_TRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
	struct	{
			USHORT		rsv:3;
			USHORT		FixedTxMode:2;			
			USHORT		PhyMode:4;
			USHORT		MCS:7;                 
	}	field;
#else
	struct	{
			USHORT		MCS:7;			
			USHORT		PhyMode:4;
			USHORT		FixedTxMode:2;			
			USHORT		rsv:3;
	}	field;
#endif
	USHORT		word;
 } DESIRED_TRANSMIT_SETTING, *PDESIRED_TRANSMIT_SETTING;





#define WLAN_MAX_NUM_OF_TIM			((MAX_LEN_OF_MAC_TABLE >> 3) + 1) 
#define WLAN_CT_TIM_BCMC_OFFSET		0 


#define WLAN_MR_TIM_BCMC_CLEAR(apidx) \
	pAd->ApCfg.MBSSID[apidx].TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] &= ~BIT8[0];


#define WLAN_MR_TIM_BCMC_SET(apidx) \
	pAd->ApCfg.MBSSID[apidx].TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] |= BIT8[0];


#define WLAN_MR_TIM_BIT_CLEAR(ad_p, apidx, wcid) \
	{	UCHAR tim_offset = wcid >> 3; \
		UCHAR bit_offset = wcid & 0x7; \
		ad_p->ApCfg.MBSSID[apidx].TimBitmaps[tim_offset] &= (~BIT8[bit_offset]); }


#define WLAN_MR_TIM_BIT_SET(ad_p, apidx, wcid) \
	{	UCHAR tim_offset = wcid >> 3; \
		UCHAR bit_offset = wcid & 0x7; \
		ad_p->ApCfg.MBSSID[apidx].TimBitmaps[tim_offset] |= BIT8[bit_offset]; }



typedef struct _COMMON_CONFIG {

	BOOLEAN		bCountryFlag;
	UCHAR		CountryCode[3];
	UCHAR		Geography;
	UCHAR       CountryRegion;      
	UCHAR       CountryRegionForABand;	
	UCHAR       PhyMode;            
	UCHAR       DesiredPhyMode;            
	USHORT      Dsifs;              
	ULONG       PacketFilter;       
	UINT8		RegulatoryClass[MAX_NUM_OF_REGULATORY_CLASS];

	CHAR        Ssid[MAX_LEN_OF_SSID]; 
	UCHAR       SsidLen;               
	UCHAR       LastSsidLen;               
	CHAR        LastSsid[MAX_LEN_OF_SSID]; 
	UCHAR		LastBssid[MAC_ADDR_LEN];

	UCHAR       Bssid[MAC_ADDR_LEN];
	USHORT      BeaconPeriod;
	UCHAR       Channel;
	UCHAR       CentralChannel;	

	UCHAR       SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR       SupRateLen;
	UCHAR       ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR       ExtRateLen;
	UCHAR       DesireRate[MAX_LEN_OF_SUPPORTED_RATES];      
	UCHAR       MaxDesiredRate;
	UCHAR       ExpectedACKRate[MAX_LEN_OF_SUPPORTED_RATES];

	ULONG       BasicRateBitmap;        

	BOOLEAN		bAPSDCapable;
	BOOLEAN		bInServicePeriod;
	BOOLEAN		bAPSDAC_BE;
	BOOLEAN		bAPSDAC_BK;
	BOOLEAN		bAPSDAC_VI;
	BOOLEAN		bAPSDAC_VO;

	
	BOOLEAN		bACMAPSDBackup[4]; 
	BOOLEAN		bACMAPSDTr[4]; 

	BOOLEAN		bNeedSendTriggerFrame;
	BOOLEAN		bAPSDForcePowerSave;	
	ULONG		TriggerTimerCount;
	UCHAR		MaxSPLength;
	UCHAR		BBPCurrentBW;	
	
	
	REG_TRANSMIT_SETTING        RegTransmitSetting; 
	
	UCHAR       TxRate;                 
	UCHAR       MaxTxRate;              
	UCHAR       TxRateIndex;            
	UCHAR       TxRateTableSize;        
	
	UCHAR       MinTxRate;              
	UCHAR       RtsRate;                
	HTTRANSMIT_SETTING	MlmeTransmit;   
	UCHAR       MlmeRate;               
	UCHAR       BasicMlmeRate;          

	USHORT      RtsThreshold;           
	USHORT      FragmentThreshold;      

	UCHAR       TxPower;                
	ULONG       TxPowerPercentage;      
	ULONG       TxPowerDefault;         
	UINT8		PwrConstraint;

#ifdef DOT11_N_SUPPORT
	BACAP_STRUC        BACapability; 
	BACAP_STRUC        REGBACapability; 
#endif 
	IOT_STRUC		IOTestParm;	
	ULONG       TxPreamble;             
	BOOLEAN     bUseZeroToDisableFragment;     
	ULONG       UseBGProtection;        
	BOOLEAN     bUseShortSlotTime;      
	BOOLEAN     bEnableTxBurst;         
	BOOLEAN     bAggregationCapable;      
	BOOLEAN     bPiggyBackCapable;		
	BOOLEAN     bIEEE80211H;			
	ULONG		DisableOLBCDetect;		

#ifdef DOT11_N_SUPPORT
	BOOLEAN				bRdg;
#endif 
	BOOLEAN             bWmmCapable;        
	QOS_CAPABILITY_PARM APQosCapability;    
	EDCA_PARM           APEdcaParm;         
	QBSS_LOAD_PARM      APQbssLoad;         
	UCHAR               AckPolicy[4];       
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN				bDLSCapable;		
#endif 
	
	
	
	
	ULONG               OpStatusFlags;

	BOOLEAN				NdisRadioStateOff; 
	ABGBAND_STATE		BandState;		
#ifdef ANT_DIVERSITY_SUPPORT
	UCHAR				bRxAntDiversity; 
#endif 

	
	RADAR_DETECT_STRUCT	RadarDetect;

#ifdef CARRIER_DETECTION_SUPPORT
	CARRIER_DETECTION_STRUCT		CarrierDetect;
#endif 

#ifdef DOT11_N_SUPPORT
	
	UCHAR			BASize;		
	
	RT_HT_CAPABILITY	DesiredHtPhy;
	HT_CAPABILITY_IE		HtCapability;
	ADD_HT_INFO_IE		AddHTInfo;	
	
	
	NEW_EXT_CHAN_IE	NewExtChanOffset;	

#ifdef DOT11N_DRAFT3
	UCHAR					Bss2040CoexistFlag;		
	RALINK_TIMER_STRUCT	Bss2040CoexistTimer;

	
	BSS_2040_COEXIST_IE		BSS2040CoexistInfo;
	
	USHORT					Dot11OBssScanPassiveDwell;				
	USHORT					Dot11OBssScanActiveDwell;				
	USHORT					Dot11BssWidthTriggerScanInt;			
	USHORT					Dot11OBssScanPassiveTotalPerChannel;	
	USHORT					Dot11OBssScanActiveTotalPerChannel;	
	USHORT					Dot11BssWidthChanTranDelayFactor;
	USHORT					Dot11OBssScanActivityThre;				

	ULONG					Dot11BssWidthChanTranDelay;			
	ULONG					CountDownCtr;	

	NDIS_SPIN_LOCK          TriggerEventTabLock;
	BSS_2040_COEXIST_IE		LastBSSCoexist2040;
	BSS_2040_COEXIST_IE		BSSCoexist2040;
	TRIGGER_EVENT_TAB		TriggerEventTab;
	UCHAR					ChannelListIdx;
	
	BOOLEAN					bOverlapScanning;
#endif 

    BOOLEAN                 bHTProtect;
    BOOLEAN                 bMIMOPSEnable;
    BOOLEAN					bBADecline;

	BOOLEAN					bGreenAPEnable;
	BOOLEAN					bBlockAntDivforGreenAP;

	BOOLEAN					bDisableReordering;
	BOOLEAN					bForty_Mhz_Intolerant;
	BOOLEAN					bExtChannelSwitchAnnouncement;
	BOOLEAN					bRcvBSSWidthTriggerEvents;
	ULONG					LastRcvBSSWidthTriggerEventsTime;

	UCHAR					TxBASize;
#endif 

	
	BOOLEAN				bWirelessEvent;
	BOOLEAN				bWiFiTest;				

	
	UCHAR				TxStream;
	UCHAR				RxStream;

	
#ifdef MCAST_RATE_SPECIFIC
	UCHAR				McastTransmitMcs;
	UCHAR				McastTransmitPhyMode;
#endif 

	BOOLEAN			bHardwareRadio;     



	NDIS_SPIN_LOCK			MeasureReqTabLock;
	PMEASURE_REQ_TAB		pMeasureReqTab;

	NDIS_SPIN_LOCK			TpcReqTabLock;
	PTPC_REQ_TAB			pTpcReqTab;

	
#ifdef MCAST_RATE_SPECIFIC
	HTTRANSMIT_SETTING		MCastPhyMode;
#endif 

#ifdef SINGLE_SKU
	UINT16					DefineMaxTxPwr;
#endif 


	BOOLEAN				PSPXlink;  


#if defined(RT305x)||defined(RT30xx)
	
	UCHAR	HighPowerPatchDisabled;
#endif

	BOOLEAN		HT_DisallowTKIP;		
} COMMON_CONFIG, *PCOMMON_CONFIG;


#ifdef CONFIG_STA_SUPPORT


typedef struct _STA_ADMIN_CONFIG {
	
	
	
	
	
	
	UCHAR       BssType;              
	USHORT      AtimWin;          

	
	
	
	
	
	UCHAR       RssiTrigger;
	UCHAR       RssiTriggerMode;      
	USHORT      DefaultListenCount;   
	ULONG       WindowsPowerMode;           
	ULONG       WindowsBatteryPowerMode;    
	BOOLEAN     bWindowsACCAMEnable;        
	BOOLEAN     bAutoReconnect;         
	ULONG       WindowsPowerProfile;    

	
	USHORT      Psm;                  
	USHORT      DisassocReason;
	UCHAR       DisassocSta[MAC_ADDR_LEN];
	USHORT      DeauthReason;
	UCHAR       DeauthSta[MAC_ADDR_LEN];
	USHORT      AuthFailReason;
	UCHAR       AuthFailSta[MAC_ADDR_LEN];

	NDIS_802_11_PRIVACY_FILTER          PrivacyFilter;  
	NDIS_802_11_AUTHENTICATION_MODE     AuthMode;       
	NDIS_802_11_WEP_STATUS              WepStatus;
	NDIS_802_11_WEP_STATUS				OrigWepStatus;	

	
	NDIS_802_11_ENCRYPTION_STATUS		GroupCipher;		
	NDIS_802_11_ENCRYPTION_STATUS		PairCipher;			
	BOOLEAN								bMixCipher;			
	USHORT								RsnCapability;

	NDIS_802_11_WEP_STATUS              GroupKeyWepStatus;

	UCHAR		WpaPassPhrase[64];		
	UINT		WpaPassPhraseLen;		
	UCHAR		PMK[32];                
	UCHAR       PTK[64];                
	UCHAR		GTK[32];				
	BSSID_INFO	SavedPMK[PMKID_NO];
	UINT		SavedPMKNum;			

	UCHAR		DefaultKeyId;


	
	UCHAR       PortSecured;

	
	ULONG       LastMicErrorTime;   
	ULONG       MicErrCnt;          
	BOOLEAN     bBlockAssoc;        
	
	WPA_STATE   WpaState;           
	UCHAR       ReplayCounter[8];
	UCHAR       ANonce[32];         
	UCHAR       SNonce[32];         

	UCHAR       LastSNR0;             
	UCHAR       LastSNR1;            
	RSSI_SAMPLE RssiSample;
	ULONG       NumOfAvgRssiSample;

	ULONG       LastBeaconRxTime;     
	ULONG       Last11bBeaconRxTime;  
	ULONG		Last11gBeaconRxTime;	
	ULONG		Last20NBeaconRxTime;	

	ULONG       LastScanTime;       
	ULONG       ScanCnt;            
	BOOLEAN     bSwRadio;           
	BOOLEAN     bHwRadio;           
	BOOLEAN     bRadio;             
	BOOLEAN     bHardwareRadio;     
	BOOLEAN     bShowHiddenSSID;    

	
	
	NDIS_802_11_ASSOCIATION_INFORMATION     AssocInfo;
	USHORT       ReqVarIELen;                
	UCHAR       ReqVarIEs[MAX_VIE_LEN];		
	USHORT       ResVarIELen;                
	UCHAR       ResVarIEs[MAX_VIE_LEN];

	UCHAR       RSNIE_Len;
	UCHAR       RSN_IE[MAX_LEN_OF_RSNIE];	

	ULONG               CLBusyBytes;                
	USHORT              RPIDensity[8];              

	UCHAR               RMReqCnt;                   
	UCHAR               CurrentRMReqIdx;            
	BOOLEAN             ParallelReq;                
													
	USHORT              ParallelDuration;           
	UCHAR               ParallelChannel;            
	USHORT              IAPPToken;                  
	
	UCHAR               NHFactor;                   
	UCHAR               CLFactor;                   

	RALINK_TIMER_STRUCT	StaQuickResponeForRateUpTimer;
	BOOLEAN				StaQuickResponeForRateUpTimerRunning;

	UCHAR			DtimCount;      
	UCHAR			DtimPeriod;     

#ifdef QOS_DLS_SUPPORT
	RT_802_11_DLS		DLSEntry[MAX_NUM_OF_DLS_ENTRY];
	UCHAR				DlsReplayCounter[8];
#endif 
	
	
	BOOLEAN				WhqlTest;
	

    RALINK_TIMER_STRUCT WpaDisassocAndBlockAssocTimer;
    
	BOOLEAN		        bAutoRoaming;       
	CHAR		        dBmToRoam;          

#ifdef WPA_SUPPLICANT_SUPPORT
    BOOLEAN             IEEE8021X;
    BOOLEAN             IEEE8021x_required_keys;
    CIPHER_KEY	        DesireSharedKey[4];	
    UCHAR               DesireSharedKeyId;

    
    
    
    UCHAR               WpaSupplicantUP;
	UCHAR				WpaSupplicantScanCount;
	BOOLEAN				bRSN_IE_FromWpaSupplicant;
#endif 

    CHAR                dev_name[16];
    USHORT              OriDevType;

    BOOLEAN             bTGnWifiTest;
	BOOLEAN			    bScanReqIsFromWebUI;

	HTTRANSMIT_SETTING				HTPhyMode, MaxHTPhyMode, MinHTPhyMode;
	DESIRED_TRANSMIT_SETTING	DesiredTransmitSetting;
	RT_HT_PHY_INFO					DesiredHtPhyInfo;
	BOOLEAN							bAutoTxRateSwitch;

#ifdef RTMP_MAC_PCI
    UCHAR       BBPR3;
	
	
	
	PS_CONTROL				PSControl;
#endif 

#ifdef EXT_BUILD_CHANNEL_LIST
	UCHAR				IEEE80211dClientMode;
	UCHAR				StaOriCountryCode[3];
	UCHAR				StaOriGeography;
#endif 



	BOOLEAN				bAutoConnectByBssid;
	ULONG				BeaconLostTime;	
	BOOLEAN			bForceTxBurst;          
} STA_ADMIN_CONFIG, *PSTA_ADMIN_CONFIG;







typedef struct _STA_ACTIVE_CONFIG {
	USHORT      Aid;
	USHORT      AtimWin;                
	USHORT      CapabilityInfo;
	USHORT      CfpMaxDuration;
	USHORT      CfpPeriod;

	
	
	UCHAR       SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR       ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR       SupRateLen;
	UCHAR       ExtRateLen;
	
	RT_HT_PHY_INFO		SupportedPhyInfo;
	RT_HT_CAPABILITY	SupportedHtPhy;
} STA_ACTIVE_CONFIG, *PSTA_ACTIVE_CONFIG;



#endif 



typedef struct _MAC_TABLE_ENTRY {
	
	BOOLEAN		ValidAsCLI;		
	BOOLEAN		ValidAsWDS;	
	BOOLEAN		ValidAsApCli;   
	BOOLEAN		ValidAsMesh;
	BOOLEAN		ValidAsDls;	
	BOOLEAN		isCached;
	BOOLEAN		bIAmBadAtheros;	

	UCHAR		EnqueueEapolStartTimerRunning;  
	
	
	UCHAR           CMTimerRunning;
	UCHAR           apidx;			
	UCHAR           RSNIE_Len;
	UCHAR           RSN_IE[MAX_LEN_OF_RSNIE];
	UCHAR           ANonce[LEN_KEY_DESC_NONCE];
	UCHAR           SNonce[LEN_KEY_DESC_NONCE];
	UCHAR           R_Counter[LEN_KEY_DESC_REPLAY];
	UCHAR           PTK[64];
	UCHAR           ReTryCounter;
	RALINK_TIMER_STRUCT                 RetryTimer;
	RALINK_TIMER_STRUCT					EnqueueStartForPSKTimer;	
	NDIS_802_11_AUTHENTICATION_MODE     AuthMode;   
	NDIS_802_11_WEP_STATUS              WepStatus;
	NDIS_802_11_WEP_STATUS              GroupKeyWepStatus;
	AP_WPA_STATE    WpaState;
	GTK_STATE       GTKState;
	USHORT          PortSecured;
	NDIS_802_11_PRIVACY_FILTER  PrivacyFilter;      
	CIPHER_KEY      PairwiseKey;
	PVOID           pAd;
	INT				PMKID_CacheIdx;
	UCHAR			PMKID[LEN_PMKID];


	UCHAR           Addr[MAC_ADDR_LEN];
	UCHAR           PsMode;
	SST             Sst;
	AUTH_STATE      AuthState; 
	BOOLEAN			IsReassocSta;	
	USHORT          Aid;
	USHORT          CapabilityInfo;
	UCHAR           LastRssi;
	ULONG           NoDataIdleCount;
	UINT16			StationKeepAliveCount; 
	ULONG           PsQIdleCount;
	QUEUE_HEADER    PsQueue;

	UINT32			StaConnectTime;		


#ifdef DOT11_N_SUPPORT
	BOOLEAN			bSendBAR;
	USHORT			NoBADataCountDown;

	UINT32			CachedBuf[16];		
	UINT			TxBFCount; 
#endif 
	UINT			FIFOCount;
	UINT			DebugFIFOCount;
	UINT			DebugTxCount;
    BOOLEAN			bDlsInit;





	UINT			MatchWDSTabIdx;
	UCHAR           MaxSupportedRate;
	UCHAR           CurrTxRate;
	UCHAR           CurrTxRateIndex;
	
	USHORT          TxQuality[MAX_STEP_OF_TX_RATE_SWITCH];

	UINT32			OneSecTxNoRetryOkCount;
	UINT32          OneSecTxRetryOkCount;
	UINT32          OneSecTxFailCount;
	UINT32			ContinueTxFailCnt;
	UINT32          CurrTxRateStableTime; 
	UCHAR           TxRateUpPenalty;      
#ifdef WDS_SUPPORT
	BOOLEAN		LockEntryTx; 
	UINT32		TimeStamp_toTxRing;
#endif 





#ifdef CONFIG_STA_SUPPORT
#ifdef QOS_DLS_SUPPORT
	UINT			MatchDlsEntryIdx; 
#endif 
#endif 

	BOOLEAN         fNoisyEnvironment;
	BOOLEAN			fLastSecAccordingRSSI;
	UCHAR           LastSecTxRateChangeAction; 
	CHAR			LastTimeTxRateChangeAction; 
	ULONG			LastTxOkCount;
	UCHAR           PER[MAX_STEP_OF_TX_RATE_SWITCH];

	
	
	
	
	ULONG           ClientStatusFlags;

	HTTRANSMIT_SETTING	HTPhyMode, MaxHTPhyMode, MinHTPhyMode;

#ifdef DOT11_N_SUPPORT
	
	USHORT		RXBAbitmap;	
	USHORT		TXBAbitmap;	
	USHORT		TXAutoBAbitmap;
	USHORT		BADeclineBitmap;
	USHORT		BARecWcidArray[NUM_OF_TID];	
	USHORT		BAOriWcidArray[NUM_OF_TID]; 
	USHORT		BAOriSequence[NUM_OF_TID]; 

	
	UCHAR		MpduDensity;
	UCHAR		MaxRAmpduFactor;
	UCHAR		AMsduSize;
	UCHAR		MmpsMode;	

	HT_CAPABILITY_IE		HTCapability;

#ifdef DOT11N_DRAFT3
	UCHAR		BSS2040CoexistenceMgmtSupport;
#endif 
#endif 

	BOOLEAN		bAutoTxRateSwitch;

	UCHAR       RateLen;
	struct _MAC_TABLE_ENTRY *pNext;
    USHORT      TxSeq[NUM_OF_TID];
	USHORT		NonQosDataSeq;

	RSSI_SAMPLE	RssiSample;

	UINT32			TXMCSExpected[16];
	UINT32			TXMCSSuccessful[16];
	UINT32			TXMCSFailed[16];
	UINT32			TXMCSAutoFallBack[16][16];

#ifdef CONFIG_STA_SUPPORT
	ULONG			LastBeaconRxTime;
#endif 



	ULONG AssocDeadLine;



	ULONG ChannelQuality;  

} MAC_TABLE_ENTRY, *PMAC_TABLE_ENTRY;

typedef struct _MAC_TABLE {
	USHORT			Size;
	MAC_TABLE_ENTRY *Hash[HASH_TABLE_SIZE];
	MAC_TABLE_ENTRY Content[MAX_LEN_OF_MAC_TABLE];
	QUEUE_HEADER    McastPsQueue;
	ULONG           PsQIdleCount;
	BOOLEAN         fAnyStationInPsm;
	BOOLEAN         fAnyStationBadAtheros;	
	BOOLEAN			fAnyTxOPForceDisable;	
	BOOLEAN			fAllStationAsRalink;	
#ifdef DOT11_N_SUPPORT
	BOOLEAN         fAnyStationIsLegacy;	
	BOOLEAN         fAnyStationNonGF;		
	BOOLEAN         fAnyStation20Only;		
	BOOLEAN			fAnyStationMIMOPSDynamic; 
	BOOLEAN         fAnyBASession;   


#endif 
} MAC_TABLE, *PMAC_TABLE;




#ifdef BLOCK_NET_IF
typedef struct _BLOCK_QUEUE_ENTRY
{
	BOOLEAN SwTxQueueBlockFlag;
	LIST_HEADER NetIfList;
} BLOCK_QUEUE_ENTRY, *PBLOCK_QUEUE_ENTRY;
#endif 


struct wificonf
{
	BOOLEAN	bShortGI;
	BOOLEAN bGreenField;
};


typedef struct _RTMP_DEV_INFO_
{
	UCHAR			chipName[16];
	RTMP_INF_TYPE	infType;
}RTMP_DEV_INFO;


#ifdef DBG_DIAGNOSE
#define DIAGNOSE_TIME	10   
typedef struct _RtmpDiagStrcut_
{	
	unsigned char		inited;
	unsigned char	qIdx;
	unsigned char	ArrayStartIdx;
	unsigned char		ArrayCurIdx;
	
	USHORT			TxDataCnt[DIAGNOSE_TIME];
	USHORT			TxFailCnt[DIAGNOSE_TIME];

	USHORT			TxDescCnt[DIAGNOSE_TIME][24]; 

	USHORT			TxMcsCnt[DIAGNOSE_TIME][24]; 
	USHORT			TxSWQueCnt[DIAGNOSE_TIME][9];		

	USHORT			TxAggCnt[DIAGNOSE_TIME];
	USHORT			TxNonAggCnt[DIAGNOSE_TIME];

	USHORT			TxAMPDUCnt[DIAGNOSE_TIME][24]; 
	USHORT			TxRalinkCnt[DIAGNOSE_TIME];			
	USHORT			TxAMSDUCnt[DIAGNOSE_TIME];			

	
	USHORT			RxDataCnt[DIAGNOSE_TIME];			
	USHORT			RxCrcErrCnt[DIAGNOSE_TIME];

	USHORT			RxMcsCnt[DIAGNOSE_TIME][24]; 
}RtmpDiagStruct;
#endif 


struct _RTMP_CHIP_OP_
{
	
	int (*eeinit)(RTMP_ADAPTER *pAd);										
	int (*eeread)(RTMP_ADAPTER *pAd, USHORT offset, PUSHORT pValue);				
	int (*eewrite)(RTMP_ADAPTER *pAd, USHORT offset, USHORT value);;				

	
	int (*loadFirmware)(RTMP_ADAPTER *pAd);								
	int (*eraseFirmware)(RTMP_ADAPTER *pAd);								
	int (*sendCommandToMcu)(RTMP_ADAPTER *pAd, UCHAR cmd, UCHAR token, UCHAR arg0, UCHAR arg1);;	

	
	REG_PAIR *pRFRegTable;
	void (*AsicRfInit)(RTMP_ADAPTER *pAd);
	void (*AsicRfTurnOn)(RTMP_ADAPTER *pAd);
	void (*AsicRfTurnOff)(RTMP_ADAPTER *pAd);
	void (*AsicReverseRfFromSleepMode)(RTMP_ADAPTER *pAd);
	void (*AsicHaltAction)(RTMP_ADAPTER *pAd);
};





struct _RTMP_ADAPTER
{
	PVOID					OS_Cookie;	
	PNET_DEV				net_dev;
	ULONG					VirtualIfCnt;

	RTMP_CHIP_OP			chipOps;
	USHORT					ThisTbttNumToNextWakeUp;

#ifdef INF_AMAZON_PPA
	UINT32  g_if_id;
	BOOLEAN	PPAEnable;
	PPA_DIRECTPATH_CB       *pDirectpathCb;
#endif 

#ifdef RTMP_MAC_PCI



	PUCHAR                  CSRBaseAddress;     
	unsigned int			irq_num;

	USHORT		            LnkCtrlBitMask;
	USHORT		            RLnkCtrlConfiguration;
	USHORT                  RLnkCtrlOffset;
	USHORT		            HostLnkCtrlConfiguration;
	USHORT                  HostLnkCtrlOffset;
	USHORT		            PCIePowerSaveLevel;
	ULONG				Rt3xxHostLinkCtrl;	
	ULONG				Rt3xxRalinkLinkCtrl;	
	USHORT				DeviceID;           
	ULONG				AccessBBPFailCount;
	BOOLEAN					bPCIclkOff;						
	BOOLEAN					bPCIclkOffDisableTx;			

	BOOLEAN					brt30xxBanMcuCmd;	
	BOOLEAN					b3090ESpecialChip;	
	ULONG					CheckDmaBusyCount;  

	UINT					int_enable_reg;
	UINT					int_disable_mask;
	UINT					int_pending;


	RTMP_DMABUF             TxBufSpace[NUM_OF_TX_RING]; 
	RTMP_DMABUF             RxDescRing;                 
	RTMP_DMABUF             TxDescRing[NUM_OF_TX_RING];	
	RTMP_TX_RING            TxRing[NUM_OF_TX_RING];		
#endif 


	NDIS_SPIN_LOCK		irq_lock;
	UCHAR				irq_disabled;










	
	RTMP_INF_TYPE			infType;




	RTMP_OS_TASK			mlmeTask;
#ifdef RTMP_TIMER_TASK_SUPPORT
	
	RTMP_TIMER_TASK_QUEUE	TimerQ;
	NDIS_SPIN_LOCK			TimerQLock;
	RTMP_OS_TASK			timerTask;
#endif 





	BOOLEAN                 DeQueueRunning[NUM_OF_TX_RING];  
	NDIS_SPIN_LOCK          DeQueueLock[NUM_OF_TX_RING];


	
	QUEUE_HEADER            TxSwQueue[NUM_OF_TX_RING];  
	NDIS_SPIN_LOCK          TxSwQueueLock[NUM_OF_TX_RING];	

	RTMP_DMABUF             MgmtDescRing;			
	RTMP_MGMT_RING          MgmtRing;
	NDIS_SPIN_LOCK          MgmtRingLock;			






#ifdef RTMP_MAC_PCI
	RTMP_RX_RING            RxRing;
	NDIS_SPIN_LOCK          RxRingLock;                 
#ifdef RT3090
	NDIS_SPIN_LOCK          McuCmdLock;              
#endif 
#endif 






	UINT32			MACVersion;		

	
	
	
	ULONG				EepromVersion;          
	ULONG				FirmwareVersion;        
	USHORT				EEPROMDefaultValue[NUM_EEPROM_BBP_PARMS];
	UCHAR				EEPROMAddressNum;       
	BOOLEAN				EepromAccess;
	UCHAR				EFuseTag;


	
	
	
#ifdef MERGE_ARCH_TEAM
	UCHAR                   BbpWriteLatch[256];     
#else
	UCHAR                   BbpWriteLatch[140];     
#endif 
	CHAR					BbpRssiToDbmDelta;		
	BBP_R66_TUNING          BbpTuning;

	
	
	
	UCHAR                   RfIcType;       
	ULONG                   RfFreqOffset;   
	RTMP_RF_REGS            LatchRfRegs;    

	EEPROM_ANTENNA_STRUC    Antenna;                            
	EEPROM_NIC_CONFIG2_STRUC    NicConfig2;

	
	
	SOFT_RX_ANT_DIVERSITY   RxAnt;

	UCHAR                   RFProgSeq;
	CHANNEL_TX_POWER        TxPower[MAX_NUM_OF_CHANNELS];       
	CHANNEL_TX_POWER        ChannelList[MAX_NUM_OF_CHANNELS];   
	CHANNEL_11J_TX_POWER    TxPower11J[MAX_NUM_OF_11JCHANNELS];       
	CHANNEL_11J_TX_POWER    ChannelList11J[MAX_NUM_OF_11JCHANNELS];   

	UCHAR                   ChannelListNum;                     
	UCHAR					Bbp94;
	BOOLEAN					BbpForCCK;
	ULONG		Tx20MPwrCfgABand[5];
	ULONG		Tx20MPwrCfgGBand[5];
	ULONG		Tx40MPwrCfgABand[5];
	ULONG		Tx40MPwrCfgGBand[5];

	BOOLEAN     bAutoTxAgcA;                
	UCHAR	    TssiRefA;					
	UCHAR	    TssiPlusBoundaryA[5];		
	UCHAR	    TssiMinusBoundaryA[5];		
	UCHAR	    TxAgcStepA;					
	CHAR		TxAgcCompensateA;			

	BOOLEAN     bAutoTxAgcG;                
	UCHAR	    TssiRefG;					
	UCHAR	    TssiPlusBoundaryG[5];		
	UCHAR	    TssiMinusBoundaryG[5];		
	UCHAR	    TxAgcStepG;					
	CHAR		TxAgcCompensateG;			

	CHAR		BGRssiOffset0;				
	CHAR		BGRssiOffset1;				
	CHAR		BGRssiOffset2;				

	CHAR		ARssiOffset0;				
	CHAR		ARssiOffset1;				
	CHAR		ARssiOffset2;				

	CHAR		BLNAGain;					
	CHAR		ALNAGain0;					
	CHAR		ALNAGain1;					
	CHAR		ALNAGain2;					
#ifdef RT30xx
	
	UCHAR		Bbp25;
	UCHAR		Bbp26;

	UCHAR		TxMixerGain24G;				
	UCHAR		TxMixerGain5G;
#endif 
	
	
	
	MCU_LEDCS_STRUC		LedCntl;
	USHORT				Led1;	
	USHORT				Led2;	
	USHORT				Led3;	
	UCHAR				LedIndicatorStrength;
	UCHAR				RssiSingalstrengthOffet;
	BOOLEAN				bLedOnScanning;
	UCHAR				LedStatus;




	
	TXWI_STRUC			BeaconTxWI;
	PUCHAR						BeaconBuf;
	USHORT						BeaconOffset[HW_BEACON_MAX_COUNT];

	
	PSPOLL_FRAME			PsPollFrame;
	HEADER_802_11			NullFrame;








#ifdef CONFIG_STA_SUPPORT
	
	
	
	
	STA_ADMIN_CONFIG        StaCfg;		
	STA_ACTIVE_CONFIG       StaActive;		
	CHAR                    nickname[IW_ESSID_MAX_SIZE+1]; 
	NDIS_MEDIA_STATE        PreMediaState;
#endif 


	
	UCHAR                   OpMode;                     

	NDIS_MEDIA_STATE        IndicateMediaState;			


	

	
	BOOLEAN                 bLocalAdminMAC;             
	UCHAR                   PermanentAddress[MAC_ADDR_LEN];    
	UCHAR                   CurrentAddress[MAC_ADDR_LEN];      

	
	
	
	COMMON_CONFIG           CommonCfg;
	MLME_STRUCT             Mlme;

	
	MLME_AUX                MlmeAux;           
	BSS_TABLE               ScanTab;           

	
	MAC_TABLE                 MacTab;     
	NDIS_SPIN_LOCK          MacTabLock;

#ifdef DOT11_N_SUPPORT
	BA_TABLE			BATable;
	NDIS_SPIN_LOCK          BATabLock;
	RALINK_TIMER_STRUCT RECBATimer;
#endif 

	
	CIPHER_KEY              SharedKey[MAX_MBSSID_NUM][4]; 

	
	FRAGMENT_FRAME          FragFrame;                  

	
	COUNTER_802_3           Counters8023;               
	COUNTER_802_11          WlanCounters;               
	COUNTER_RALINK          RalinkCounters;             
	COUNTER_DRS             DrsCounters;                
	PRIVATE_STRUC           PrivateInfo;                

	
	ULONG                   Flags;                      
	ULONG                   PSFlags;                    

	
	USHORT                  Sequence;

	
	
	ULONG                   LinkDownTime;
	
	ULONG                   LastRxRate;
	ULONG                   LastTxRate;
	
	BOOLEAN                 bConfigChanged;         
	

	ULONG                   ExtraInfo;              
	ULONG                   SystemErrorBitmap;      

	
	ULONG                   MacIcVersion;           
	

	
	
	
	RT_802_11_EVENT_TABLE   EventTab;


	BOOLEAN		HTCEnable;

	
	
	

	BOOLEAN						bUpdateBcnCntDone;
	ULONG						watchDogMacDeadlock;	
	
	
	
	
	BOOLEAN		bBanAllBaSetup;
	BOOLEAN		bPromiscuous;

	
	
	
	
	
	
	
	
	
	BOOLEAN		bLinkAdapt;
	BOOLEAN		bForcePrintTX;
	BOOLEAN		bForcePrintRX;
	
	BOOLEAN		bStaFifoTest;
	BOOLEAN		bProtectionTest;
	
	BOOLEAN		bBroadComHT;
	
	ULONG		BulkOutReq;
	ULONG		BulkOutComplete;
	ULONG		BulkOutCompleteOther;
	ULONG		BulkOutCompleteCancel;	
	ULONG		BulkInReq;
	ULONG		BulkInComplete;
	ULONG		BulkInCompleteFail;
	

    struct wificonf			WIFItestbed;

#ifdef RALINK_ATE
	ATE_INFO				ate;
#endif 

#ifdef DOT11_N_SUPPORT
	struct reordering_mpdu_pool mpdu_blk_pool;
#endif 

	ULONG					OneSecondnonBEpackets;		

#ifdef LINUX
#if WIRELESS_EXT >= 12
    struct iw_statistics    iw_stats;
#endif

	struct net_device_stats	stats;
#endif 

#ifdef BLOCK_NET_IF
	BLOCK_QUEUE_ENTRY		blockQueueTab[NUM_OF_TX_RING];
#endif 



#ifdef MULTIPLE_CARD_SUPPORT
	INT32					MC_RowID;
	STRING					MC_FileName[256];
#endif 

	ULONG					TbttTickCount;
#ifdef PCI_MSI_SUPPORT
	BOOLEAN					HaveMsi;
#endif 


	UCHAR					is_on;

#define TIME_BASE			(1000000/OS_HZ)
#define TIME_ONE_SECOND		(1000000/TIME_BASE)
	UCHAR					flg_be_adjust;
	ULONG					be_adjust_last_time;

#ifdef NINTENDO_AP
	NINDO_CTRL_BLOCK		nindo_ctrl_block;
#endif 


#ifdef IKANOS_VX_1X0
	struct IKANOS_TX_INFO	IkanosTxInfo;
	struct IKANOS_TX_INFO	IkanosRxInfo[MAX_MBSSID_NUM + MAX_WDS_ENTRY + MAX_APCLI_NUM + MAX_MESH_NUM];
#endif 


#ifdef DBG_DIAGNOSE
	RtmpDiagStruct	DiagStruct;
#endif 


	UINT8					FlgCtsEnabled;
	UINT8					PM_FlgSuspend;

#ifdef RT30xx
#ifdef RTMP_EFUSE_SUPPORT
	BOOLEAN		bUseEfuse;
	BOOLEAN		bEEPROMFile;
	BOOLEAN		bFroceEEPROMBuffer;
	UCHAR		EEPROMImage[1024];
#endif 
#endif 

#ifdef CONFIG_STA_SUPPORT
#endif 

};



#ifdef TONE_RADAR_DETECT_SUPPORT
#define DELAYINTMASK		0x0013fffb
#define INTMASK				0x0013fffb
#define IndMask				0x0013fffc
#define RadarInt			0x00100000
#else
#define DELAYINTMASK		0x0003fffb
#define INTMASK				0x0003fffb
#define IndMask				0x0003fffc
#endif 

#define RxINT				0x00000005	
#define TxDataInt			0x000000fa	
#define TxMgmtInt			0x00000102	
#define TxCoherent			0x00020000	
#define RxCoherent			0x00010000	
#define McuCommand			0x00000200	
#define PreTBTTInt			0x00001000	
#define TBTTInt				0x00000800		
#define GPTimeOutInt			0x00008000		
#define AutoWakeupInt		0x00004000		
#define FifoStaFullInt			0x00002000	



typedef struct _RX_BLK_
{

	RT28XX_RXD_STRUC	RxD;
	PRXWI_STRUC			pRxWI;
	PHEADER_802_11		pHeader;
	PNDIS_PACKET		pRxPacket;
	UCHAR				*pData;
	USHORT				DataSize;
	USHORT				Flags;
	UCHAR				UserPriority;	
} RX_BLK;


#define RX_BLK_SET_FLAG(_pRxBlk, _flag)		(_pRxBlk->Flags |= _flag)
#define RX_BLK_TEST_FLAG(_pRxBlk, _flag)	(_pRxBlk->Flags & _flag)
#define RX_BLK_CLEAR_FLAG(_pRxBlk, _flag)	(_pRxBlk->Flags &= ~(_flag))


#define fRX_WDS			0x0001
#define fRX_AMSDU		0x0002
#define fRX_ARALINK		0x0004
#define fRX_HTC			0x0008
#define fRX_PAD			0x0010
#define fRX_AMPDU		0x0020
#define fRX_QOS			0x0040
#define fRX_INFRA		0x0080
#define fRX_EAP			0x0100
#define fRX_MESH		0x0200
#define fRX_APCLI		0x0400
#define fRX_DLS			0x0800
#define fRX_WPI			0x1000

#define LENGTH_AMSDU_SUBFRAMEHEAD	14
#define LENGTH_ARALINK_SUBFRAMEHEAD	14
#define LENGTH_ARALINK_HEADER_FIELD	 2



#define TX_UNKOWN_FRAME		0x00
#define TX_MCAST_FRAME			0x01
#define TX_LEGACY_FRAME		0x02
#define TX_AMPDU_FRAME		0x04
#define TX_AMSDU_FRAME		0x08
#define TX_RALINK_FRAME		0x10
#define TX_FRAG_FRAME			0x20



typedef struct _TX_BLK_
{
	UCHAR				QueIdx;
	UCHAR				TxFrameType;				
	UCHAR				TotalFrameNum;				
	USHORT				TotalFragNum;				
	USHORT				TotalFrameLen;				

	QUEUE_HEADER		TxPacketList;
	MAC_TABLE_ENTRY		*pMacEntry;					
	HTTRANSMIT_SETTING	*pTransmit;

	
	PNDIS_PACKET		pPacket;
	PUCHAR				pSrcBufHeader;				
	PUCHAR				pSrcBufData;				
	UINT				SrcBufLen;					
	PUCHAR				pExtraLlcSnapEncap;			
	UCHAR				HeaderBuf[96];				
	UCHAR				MpduHeaderLen;				
	UCHAR				HdrPadLen;					
	UCHAR				apidx;						
	UCHAR				Wcid;						
	UCHAR				UserPriority;				
	UCHAR				FrameGap;					
	UCHAR				MpduReqNum;					
	UCHAR				TxRate;						
	UCHAR				CipherAlg;					
	PCIPHER_KEY			pKey;



	USHORT				Flags;						

	
	ULONG				Priv;						

} TX_BLK, *PTX_BLK;


#define fTX_bRtsRequired			0x0001	
#define fTX_bAckRequired			0x0002	
#define fTX_bPiggyBack			0x0004	
#define fTX_bHTRate				0x0008	
#define fTX_bForceNonQoS		0x0010	
#define fTX_bAllowFrag			0x0020	
#define fTX_bMoreData			0x0040	
#define fTX_bWMM				0x0080	
#define fTX_bClearEAPFrame		0x0100



#ifdef CONFIG_STA_SUPPORT
#endif 




#define TX_BLK_SET_FLAG(_pTxBlk, _flag)		(_pTxBlk->Flags |= _flag)
#define TX_BLK_TEST_FLAG(_pTxBlk, _flag)	(((_pTxBlk->Flags & _flag) == _flag) ? 1 : 0)
#define TX_BLK_CLEAR_FLAG(_pTxBlk, _flag)	(_pTxBlk->Flags &= ~(_flag))




#ifdef RT_BIG_ENDIAN


static inline VOID	RTMPWIEndianChange(
	IN	PUCHAR			pData,
	IN	ULONG			DescriptorType)
{
	int size;
	int i;

	size = ((DescriptorType == TYPE_TXWI) ? TXWI_SIZE : RXWI_SIZE);

	if(DescriptorType == TYPE_TXWI)
	{
		*((UINT32 *)(pData)) = SWAP32(*((UINT32 *)(pData)));		
		*((UINT32 *)(pData + 4)) = SWAP32(*((UINT32 *)(pData+4)));	
	}
	else
	{
		for(i=0; i < size/4 ; i++)
			*(((UINT32 *)pData) +i) = SWAP32(*(((UINT32 *)pData)+i));
	}
}


#ifdef RTMP_MAC_PCI
static inline VOID	WriteBackToDescriptor(
	IN  PUCHAR			Dest,
	IN	PUCHAR			Src,
    IN  BOOLEAN			DoEncrypt,
	IN  ULONG           DescriptorType)
{
	UINT32 *p1, *p2;

	p1 = ((UINT32 *)Dest);
	p2 = ((UINT32 *)Src);

	*p1 = *p2;
	*(p1+2) = *(p2+2);
	*(p1+3) = *(p2+3);
	*(p1+1) = *(p2+1); 
}
#endif 



#ifdef RTMP_MAC_PCI
static inline VOID	RTMPDescriptorEndianChange(
	IN	PUCHAR			pData,
	IN	ULONG			DescriptorType)
{
	*((UINT32 *)(pData)) = SWAP32(*((UINT32 *)(pData)));		
	*((UINT32 *)(pData + 8)) = SWAP32(*((UINT32 *)(pData+8)));	
	*((UINT32 *)(pData +12)) = SWAP32(*((UINT32 *)(pData + 12)));	
	*((UINT32 *)(pData + 4)) = SWAP32(*((UINT32 *)(pData + 4)));				
}
#endif 


static inline VOID	RTMPFrameEndianChange(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pData,
	IN	ULONG			Dir,
	IN	BOOLEAN			FromRxDoneInt)
{
	PHEADER_802_11 pFrame;
	PUCHAR	pMacHdr;

	
	if(Dir == DIR_READ)
	{
		*(USHORT *)pData = SWAP16(*(USHORT *)pData);
	}

	pFrame = (PHEADER_802_11) pData;
	pMacHdr = (PUCHAR) pFrame;

	
	*(USHORT *)(pMacHdr + 2) = SWAP16(*(USHORT *)(pMacHdr + 2));

	
	*(USHORT *)(pMacHdr + 22) = SWAP16(*(USHORT *)(pMacHdr + 22));

	if(pFrame->FC.Type == BTYPE_MGMT)
	{
		switch(pFrame->FC.SubType)
		{
			case SUBTYPE_ASSOC_REQ:
			case SUBTYPE_REASSOC_REQ:
				
				pMacHdr += sizeof(HEADER_802_11);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				break;

			case SUBTYPE_ASSOC_RSP:
			case SUBTYPE_REASSOC_RSP:
				
				pMacHdr += sizeof(HEADER_802_11);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				
				pMacHdr += 2;
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				break;

			case SUBTYPE_AUTH:
				
				
				if(!FromRxDoneInt && pFrame->FC.Wep == 1)
					break;
				else
				{
					
					pMacHdr += sizeof(HEADER_802_11);
					*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

					
					pMacHdr += 2;
					*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

					
					pMacHdr += 2;
					*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				}
				break;

			case SUBTYPE_BEACON:
			case SUBTYPE_PROBE_RSP:
				
				pMacHdr += (sizeof(HEADER_802_11) + TIMESTAMP_LEN);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);

				
				pMacHdr += sizeof(USHORT);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				break;

			case SUBTYPE_DEAUTH:
			case SUBTYPE_DISASSOC:
				
				pMacHdr += sizeof(HEADER_802_11);
				*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
				break;
		}
	}
	else if( pFrame->FC.Type == BTYPE_DATA )
	{
	}
	else if(pFrame->FC.Type == BTYPE_CNTL)
	{
		switch(pFrame->FC.SubType)
		{
			case SUBTYPE_BLOCK_ACK_REQ:
				{
					PFRAME_BA_REQ pBAReq = (PFRAME_BA_REQ)pFrame;
					*(USHORT *)(&pBAReq->BARControl) = SWAP16(*(USHORT *)(&pBAReq->BARControl));
					pBAReq->BAStartingSeq.word = SWAP16(pBAReq->BAStartingSeq.word);
				}
				break;
			case SUBTYPE_BLOCK_ACK:
				
				*(UINT32 *)(&pFrame->Addr3[0]) = SWAP32(*(UINT32 *)(&pFrame->Addr3[0]));
				break;

			case SUBTYPE_ACK:
				
				*(UINT32 *)(&pFrame->Addr2[0])=	SWAP32(*(UINT32 *)(&pFrame->Addr2[0]));
				break;
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,("Invalid Frame Type!!!\n"));
	}

	
	if(Dir == DIR_WRITE)
	{
		*(USHORT *)pData = SWAP16(*(USHORT *)pData);
	}
}
#endif 



static inline VOID ConvertMulticastIP2MAC(
	IN PUCHAR pIpAddr,
	IN PUCHAR *ppMacAddr,
	IN UINT16 ProtoType)
{
	if (pIpAddr == NULL)
		return;

	if (ppMacAddr == NULL || *ppMacAddr == NULL)
		return;

	switch (ProtoType)
	{
		case ETH_P_IPV6:

			*(*ppMacAddr) = 0x33;
			*(*ppMacAddr + 1) = 0x33;
			*(*ppMacAddr + 2) = pIpAddr[12];
			*(*ppMacAddr + 3) = pIpAddr[13];
			*(*ppMacAddr + 4) = pIpAddr[14];
			*(*ppMacAddr + 5) = pIpAddr[15];
			break;

		case ETH_P_IP:
		default:

			*(*ppMacAddr) = 0x01;
			*(*ppMacAddr + 1) = 0x00;
			*(*ppMacAddr + 2) = 0x5e;
			*(*ppMacAddr + 3) = pIpAddr[1] & 0x7f;
			*(*ppMacAddr + 4) = pIpAddr[2];
			*(*ppMacAddr + 5) = pIpAddr[3];
			break;
	}

	return;
}


char *GetPhyMode(int Mode);
char* GetBW(int BW);



BOOLEAN RTMPCheckForHang(
	IN  NDIS_HANDLE MiniportAdapterContext);

VOID  RTMPHalt(
	IN  NDIS_HANDLE MiniportAdapterContext);




NDIS_STATUS RTMPAllocAdapterBlock(
	IN PVOID			handle,
	OUT PRTMP_ADAPTER   *ppAdapter);

NDIS_STATUS RTMPAllocTxRxRingMemory(
	IN  PRTMP_ADAPTER   pAd);

NDIS_STATUS RTMPFindAdapter(
	IN  PRTMP_ADAPTER   pAd,
	IN  NDIS_HANDLE     WrapperConfigurationContext);

NDIS_STATUS	RTMPReadParametersHook(
	IN	PRTMP_ADAPTER pAd);

NDIS_STATUS	RTMPSetProfileParameters(
	IN RTMP_ADAPTER *pAd,
	IN PSTRING		pBuffer);

INT RTMPGetKeyParameter(
    IN PSTRING key,
    OUT PSTRING dest,
    IN INT destsize,
    IN PSTRING buffer,
    IN BOOLEAN bTrimSpace);

VOID RTMPFreeAdapter(
	IN  PRTMP_ADAPTER   pAd);

NDIS_STATUS NICReadRegParameters(
	IN  PRTMP_ADAPTER       pAd,
	IN  NDIS_HANDLE         WrapperConfigurationContext);

#ifdef RTMP_RF_RW_SUPPORT
VOID NICInitRFRegisters(
	IN PRTMP_ADAPTER pAd);

VOID RtmpChipOpsRFHook(
	IN RTMP_ADAPTER *pAd);

NDIS_STATUS	RT30xxWriteRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			regID,
	IN	UCHAR			value);

NDIS_STATUS	RT30xxReadRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			regID,
	IN	PUCHAR			pValue);
#endif 

VOID NICReadEEPROMParameters(
	IN  PRTMP_ADAPTER       pAd,
	IN	PUCHAR				mac_addr);

VOID NICInitAsicFromEEPROM(
	IN  PRTMP_ADAPTER       pAd);


NDIS_STATUS NICInitializeAdapter(
	IN  PRTMP_ADAPTER   pAd,
	IN   BOOLEAN    bHardReset);

NDIS_STATUS NICInitializeAsic(
	IN  PRTMP_ADAPTER   pAd,
	IN  BOOLEAN		bHardReset);

VOID NICIssueReset(
	IN  PRTMP_ADAPTER   pAd);

VOID RTMPRingCleanUp(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           RingType);

VOID RxTest(
	IN  PRTMP_ADAPTER   pAd);

NDIS_STATUS DbgSendPacket(
	IN  PRTMP_ADAPTER   pAd,
	IN  PNDIS_PACKET    pPacket);

VOID UserCfgInit(
	IN  PRTMP_ADAPTER   pAd);

VOID NICResetFromError(
	IN  PRTMP_ADAPTER   pAd);

NDIS_STATUS NICLoadFirmware(
	IN  PRTMP_ADAPTER   pAd);

VOID NICEraseFirmware(
	IN PRTMP_ADAPTER pAd);

NDIS_STATUS NICLoadRateSwitchingParams(
	IN PRTMP_ADAPTER pAd);

BOOLEAN NICCheckForHang(
	IN  PRTMP_ADAPTER   pAd);

VOID NICUpdateFifoStaCounters(
	IN PRTMP_ADAPTER pAd);

VOID NICUpdateRawCounters(
	IN  PRTMP_ADAPTER   pAd);

VOID RTMPZeroMemory(
	IN  PVOID   pSrc,
	IN  ULONG   Length);

ULONG RTMPCompareMemory(
	IN  PVOID   pSrc1,
	IN  PVOID   pSrc2,
	IN  ULONG   Length);

VOID RTMPMoveMemory(
	OUT PVOID   pDest,
	IN  PVOID   pSrc,
	IN  ULONG   Length);

VOID AtoH(
	PSTRING	src,
	PUCHAR dest,
	int		destlen);

UCHAR BtoH(
	char ch);

VOID RTMPPatchMacBbpBug(
	IN  PRTMP_ADAPTER   pAd);

VOID RTMPPatchCardBus(
	IN	PRTMP_ADAPTER	pAdapter);

VOID RTMPPatchRalinkCardBus(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	ULONG			Bus);

ULONG RTMPReadCBConfig(
	IN	ULONG	Bus,
	IN	ULONG	Slot,
	IN	ULONG	Func,
	IN	ULONG	Offset);

VOID RTMPWriteCBConfig(
	IN	ULONG	Bus,
	IN	ULONG	Slot,
	IN	ULONG	Func,
	IN	ULONG	Offset,
	IN	ULONG	Value);

VOID RTMPInitTimer(
	IN  PRTMP_ADAPTER           pAd,
	IN  PRALINK_TIMER_STRUCT    pTimer,
	IN  PVOID                   pTimerFunc,
	IN	PVOID					pData,
	IN  BOOLEAN                 Repeat);

VOID RTMPSetTimer(
	IN  PRALINK_TIMER_STRUCT    pTimer,
	IN  ULONG                   Value);


VOID RTMPModTimer(
	IN	PRALINK_TIMER_STRUCT	pTimer,
	IN	ULONG					Value);

VOID RTMPCancelTimer(
	IN  PRALINK_TIMER_STRUCT    pTimer,
	OUT BOOLEAN                 *pCancelled);

VOID RTMPSetLED(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Status);

VOID RTMPSetSignalLED(
	IN PRTMP_ADAPTER	pAd,
	IN NDIS_802_11_RSSI Dbm);


VOID RTMPEnableRxTx(
	IN PRTMP_ADAPTER	pAd);




VOID ActionStateMachineInit(
    IN	PRTMP_ADAPTER	pAd,
    IN  STATE_MACHINE *S,
    OUT STATE_MACHINE_FUNC Trans[]);

VOID MlmeADDBAAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeDELBAAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeDLSAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeInvalidAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID MlmeQOSAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

#ifdef DOT11_N_SUPPORT
VOID PeerAddBAReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerAddBARspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerDelBAAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerBAAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);
#endif 

VOID SendPSMPAction(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Wcid,
	IN UCHAR			Psmp);


VOID PeerRMAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerPublicAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

#ifdef CONFIG_STA_SUPPORT
VOID StaPublicAction(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Bss2040Coexist);
#endif 


VOID PeerBSSTranAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

#ifdef DOT11_N_SUPPORT
VOID PeerHTAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);
#endif 

VOID PeerQOSAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

#ifdef QOS_DLS_SUPPORT
VOID PeerDLSAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);
#endif 

#ifdef CONFIG_STA_SUPPORT
#ifdef QOS_DLS_SUPPORT
VOID DlsParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_DLS_REQ_STRUCT *pDlsReq,
	IN PRT_802_11_DLS pDls,
	IN USHORT reason);
#endif 
#endif 

#ifdef DOT11_N_SUPPORT
VOID RECBATimerTimeout(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3);

VOID ORIBATimerTimeout(
	IN	PRTMP_ADAPTER	pAd);

VOID SendRefreshBAR(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry);

#ifdef DOT11N_DRAFT3
VOID SendBSS2040CoexistMgmtAction(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	Wcid,
	IN	UCHAR	apidx,
	IN	UCHAR	InfoReq);

VOID SendNotifyBWActionFrame(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR  Wcid,
	IN UCHAR apidx);

BOOLEAN ChannelSwitchSanityCheck(
	IN	PRTMP_ADAPTER	pAd,
	IN    UCHAR  Wcid,
	IN    UCHAR  NewChannel,
	IN    UCHAR  Secondary);

VOID ChannelSwitchAction(
	IN	PRTMP_ADAPTER	pAd,
	IN    UCHAR  Wcid,
	IN    UCHAR  Channel,
	IN    UCHAR  Secondary);

ULONG BuildIntolerantChannelRep(
	IN	PRTMP_ADAPTER	pAd,
	IN    PUCHAR  pDest);

VOID Update2040CoexistFrameAndNotify(
	IN	PRTMP_ADAPTER	pAd,
	IN    UCHAR  Wcid,
	IN	BOOLEAN	bAddIntolerantCha);

VOID Send2040CoexistAction(
	IN	PRTMP_ADAPTER	pAd,
	IN    UCHAR  Wcid,
	IN	BOOLEAN	bAddIntolerantCha);
#endif 
#endif 

VOID ActHeaderInit(
    IN	PRTMP_ADAPTER	pAd,
    IN OUT PHEADER_802_11 pHdr80211,
    IN PUCHAR Addr1,
    IN PUCHAR Addr2,
    IN PUCHAR Addr3);

VOID BarHeaderInit(
	IN	PRTMP_ADAPTER	pAd,
	IN OUT PFRAME_BAR pCntlBar,
	IN PUCHAR pDA,
	IN PUCHAR pSA);

VOID InsertActField(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Category,
	IN UINT8 ActCode);

BOOLEAN QosBADataParse(
	IN PRTMP_ADAPTER	pAd,
	IN BOOLEAN bAMSDU,
	IN PUCHAR p8023Header,
	IN UCHAR	WCID,
	IN UCHAR	TID,
	IN USHORT Sequence,
	IN UCHAR DataOffset,
	IN USHORT Datasize,
	IN UINT   CurRxIndex);

#ifdef DOT11_N_SUPPORT
BOOLEAN CntlEnqueueForRecv(
    IN	PRTMP_ADAPTER	pAd,
	IN ULONG Wcid,
    IN ULONG MsgLen,
	IN PFRAME_BA_REQ pMsg);

VOID BaAutoManSwitch(
	IN	PRTMP_ADAPTER	pAd);
#endif 

VOID HTIOTCheck(
	IN	PRTMP_ADAPTER	pAd,
	IN    UCHAR     BatRecIdx);




BOOLEAN RTMPHandleRxDoneInterrupt(
	IN  PRTMP_ADAPTER   pAd);

VOID RTMPHandleTxDoneInterrupt(
	IN  PRTMP_ADAPTER   pAd);

BOOLEAN RTMPHandleTxRingDmaDoneInterrupt(
	IN  PRTMP_ADAPTER   pAd,
	IN  INT_SOURCE_CSR_STRUC TxRingBitmap);

VOID RTMPHandleMgmtRingDmaDoneInterrupt(
	IN  PRTMP_ADAPTER   pAd);

VOID RTMPHandleTBTTInterrupt(
	IN  PRTMP_ADAPTER   pAd);

VOID RTMPHandlePreTBTTInterrupt(
	IN  PRTMP_ADAPTER   pAd);

void RTMPHandleTwakeupInterrupt(
	IN PRTMP_ADAPTER pAd);

VOID	RTMPHandleRxCoherentInterrupt(
	IN	PRTMP_ADAPTER	pAd);


BOOLEAN TxFrameIsAggregatible(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pPrevAddr1,
	IN  PUCHAR          p8023hdr);

BOOLEAN PeerIsAggreOn(
    IN  PRTMP_ADAPTER   pAd,
    IN  ULONG          TxRate,
    IN  PMAC_TABLE_ENTRY pMacEntry);


NDIS_STATUS Sniff2BytesFromNdisBuffer(
	IN  PNDIS_BUFFER    pFirstBuffer,
	IN  UCHAR           DesiredOffset,
	OUT PUCHAR          pByte0,
	OUT PUCHAR          pByte1);

NDIS_STATUS STASendPacket(
	IN  PRTMP_ADAPTER   pAd,
	IN  PNDIS_PACKET    pPacket);

VOID STASendPackets(
	IN  NDIS_HANDLE     MiniportAdapterContext,
	IN  PPNDIS_PACKET   ppPacketArray,
	IN  UINT            NumberOfPackets);

VOID RTMPDeQueuePacket(
	IN  PRTMP_ADAPTER   pAd,
	IN	BOOLEAN			bIntContext,
	IN  UCHAR			QueIdx,
	IN	UCHAR			Max_Tx_Packets);

NDIS_STATUS	RTMPHardTransmit(
	IN PRTMP_ADAPTER	pAd,
	IN PNDIS_PACKET		pPacket,
	IN  UCHAR			QueIdx,
	OUT	PULONG			pFreeTXDLeft);

NDIS_STATUS	STAHardTransmit(
	IN PRTMP_ADAPTER	pAd,
	IN TX_BLK			*pTxBlk,
	IN  UCHAR			QueIdx);

VOID STARxEAPOLFrameIndicate(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

NDIS_STATUS RTMPFreeTXDRequest(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           RingType,
	IN  UCHAR           NumberRequired,
	IN	PUCHAR          FreeNumberIs);

NDIS_STATUS MlmeHardTransmit(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR	QueIdx,
	IN  PNDIS_PACKET    pPacket);

NDIS_STATUS MlmeHardTransmitMgmtRing(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR	QueIdx,
	IN  PNDIS_PACKET    pPacket);

#ifdef RTMP_MAC_PCI
NDIS_STATUS MlmeHardTransmitTxRing(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR	QueIdx,
	IN  PNDIS_PACKET    pPacket);

NDIS_STATUS MlmeDataHardTransmit(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	QueIdx,
	IN	PNDIS_PACKET	pPacket);

VOID RTMPWriteTxDescriptor(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXD_STRUC		pTxD,
	IN	BOOLEAN			bWIV,
	IN	UCHAR			QSEL);
#endif 

USHORT  RTMPCalcDuration(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           Rate,
	IN  ULONG           Size);

VOID RTMPWriteTxWI(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXWI_STRUC		pTxWI,
	IN  BOOLEAN		FRAG,
	IN  BOOLEAN		CFACK,
	IN  BOOLEAN		InsTimestamp,
	IN	BOOLEAN			AMPDU,
	IN	BOOLEAN			Ack,
	IN	BOOLEAN			NSeq,		
	IN	UCHAR			BASize,
	IN	UCHAR			WCID,
	IN	ULONG			Length,
	IN  UCHAR		PID,
	IN	UCHAR			TID,
	IN	UCHAR			TxRate,
	IN	UCHAR			Txopmode,
	IN	BOOLEAN			CfAck,
	IN	HTTRANSMIT_SETTING	*pTransmit);


VOID RTMPWriteTxWI_Data(
	IN	PRTMP_ADAPTER		pAd,
	IN	OUT PTXWI_STRUC		pTxWI,
	IN	TX_BLK				*pTxBlk);


VOID RTMPWriteTxWI_Cache(
	IN	PRTMP_ADAPTER		pAd,
	IN	OUT PTXWI_STRUC		pTxWI,
	IN	TX_BLK				*pTxBlk);

VOID RTMPSuspendMsduTransmission(
	IN  PRTMP_ADAPTER   pAd);

VOID RTMPResumeMsduTransmission(
	IN  PRTMP_ADAPTER   pAd);

NDIS_STATUS MiniportMMRequest(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			QueIdx,
	IN	PUCHAR			pData,
	IN  UINT            Length);




VOID RTMPSendNullFrame(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           TxRate,
	IN	BOOLEAN			bQosNull);

VOID RTMPSendDisassociationFrame(
	IN	PRTMP_ADAPTER	pAd);

VOID RTMPSendRTSFrame(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pDA,
	IN	unsigned int	NextMpduSize,
	IN  UCHAR           TxRate,
	IN  UCHAR           RTSRate,
	IN  USHORT          AckDuration,
	IN  UCHAR           QueIdx,
	IN  UCHAR			FrameGap);


NDIS_STATUS RTMPApplyPacketFilter(
	IN  PRTMP_ADAPTER   pAd,
	IN  PRT28XX_RXD_STRUC      pRxD,
	IN  PHEADER_802_11  pHeader);

PQUEUE_HEADER   RTMPCheckTxSwQueue(
	IN  PRTMP_ADAPTER   pAd,
	OUT UCHAR           *QueIdx);

#ifdef CONFIG_STA_SUPPORT
VOID RTMPReportMicError(
	IN  PRTMP_ADAPTER   pAd,
	IN  PCIPHER_KEY     pWpaKey);

VOID	WpaMicFailureReportFrame(
	IN  PRTMP_ADAPTER    pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID    WpaDisassocApAndBlockAssoc(
    IN  PVOID SystemSpecific1,
    IN  PVOID FunctionContext,
    IN  PVOID SystemSpecific2,
    IN  PVOID SystemSpecific3);

VOID WpaStaPairwiseKeySetting(
	IN	PRTMP_ADAPTER	pAd);

VOID WpaStaGroupKeySetting(
	IN	PRTMP_ADAPTER	pAd);

#endif 

NDIS_STATUS RTMPCloneNdisPacket(
	IN  PRTMP_ADAPTER   pAd,
	IN	BOOLEAN    pInsAMSDUHdr,
	IN  PNDIS_PACKET    pInPacket,
	OUT PNDIS_PACKET   *ppOutPacket);

NDIS_STATUS RTMPAllocateNdisPacket(
	IN  PRTMP_ADAPTER   pAd,
	IN  PNDIS_PACKET    *pPacket,
	IN  PUCHAR          pHeader,
	IN  UINT            HeaderLen,
	IN  PUCHAR          pData,
	IN  UINT            DataLen);

VOID RTMPFreeNdisPacket(
	IN  PRTMP_ADAPTER   pAd,
	IN  PNDIS_PACKET    pPacket);

BOOLEAN RTMPFreeTXDUponTxDmaDone(
	IN PRTMP_ADAPTER    pAd,
	IN UCHAR            QueIdx);

BOOLEAN RTMPCheckDHCPFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket);


BOOLEAN RTMPCheckEtherType(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket);


VOID RTMPCckBbpTuning(
	IN	PRTMP_ADAPTER	pAd,
	IN	UINT			TxRate);




VOID RTMPInitWepEngine(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pKey,
	IN  UCHAR           KeyId,
	IN  UCHAR           KeyLen,
	IN  PUCHAR          pDest);

VOID RTMPEncryptData(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pSrc,
	IN  PUCHAR          pDest,
	IN  UINT            Len);

BOOLEAN	RTMPDecryptData(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	PUCHAR			pSrc,
	IN	UINT			Len,
	IN	UINT			idx);

BOOLEAN	RTMPSoftDecryptWEP(
	IN PRTMP_ADAPTER	pAd,
	IN PUCHAR			pData,
	IN ULONG			DataByteCnt,
	IN PCIPHER_KEY		pGroupKey);

VOID RTMPSetICV(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pDest);

VOID ARCFOUR_INIT(
	IN  PARCFOURCONTEXT Ctx,
	IN  PUCHAR          pKey,
	IN  UINT            KeyLen);

UCHAR   ARCFOUR_BYTE(
	IN  PARCFOURCONTEXT     Ctx);

VOID ARCFOUR_DECRYPT(
	IN  PARCFOURCONTEXT Ctx,
	IN  PUCHAR          pDest,
	IN  PUCHAR          pSrc,
	IN  UINT            Len);

VOID ARCFOUR_ENCRYPT(
	IN  PARCFOURCONTEXT Ctx,
	IN  PUCHAR          pDest,
	IN  PUCHAR          pSrc,
	IN  UINT            Len);

VOID WPAARCFOUR_ENCRYPT(
	IN  PARCFOURCONTEXT Ctx,
	IN  PUCHAR          pDest,
	IN  PUCHAR          pSrc,
	IN  UINT            Len);

UINT RTMP_CALC_FCS32(
	IN  UINT   Fcs,
	IN  PUCHAR  Cp,
	IN  INT     Len);







VOID AsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd);

VOID	AsicUpdateProtect(
	IN		PRTMP_ADAPTER	pAd,
	IN		USHORT			OperaionMode,
	IN		UCHAR			SetMask,
	IN		BOOLEAN			bDisableBGProtect,
	IN		BOOLEAN			bNonGFExist);

VOID AsicSwitchChannel(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			Channel,
	IN	BOOLEAN			bScan);

VOID AsicLockChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel) ;

VOID AsicAntennaSelect(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           Channel);

VOID AsicAntennaSetting(
	IN	PRTMP_ADAPTER	pAd,
	IN	ABGBAND_STATE	BandState);

VOID AsicRfTuningExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

#ifdef CONFIG_STA_SUPPORT

VOID AsicResetBBPAgent(
	IN PRTMP_ADAPTER pAd);

VOID AsicSleepThenAutoWakeup(
	IN  PRTMP_ADAPTER   pAd,
	IN  USHORT TbttNumToNextWakeUp);

VOID AsicForceSleep(
	IN PRTMP_ADAPTER pAd);

VOID AsicForceWakeup(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN    bFromTx);
#endif 

VOID AsicSetBssid(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR pBssid);

VOID AsicSetMcastWC(
	IN PRTMP_ADAPTER pAd);


VOID AsicDelWcidTab(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	Wcid);

VOID AsicEnableRDG(
	IN PRTMP_ADAPTER pAd);

VOID AsicDisableRDG(
	IN PRTMP_ADAPTER pAd);

VOID AsicDisableSync(
	IN  PRTMP_ADAPTER   pAd);

VOID AsicEnableBssSync(
	IN  PRTMP_ADAPTER   pAd);

VOID AsicEnableIbssSync(
	IN  PRTMP_ADAPTER   pAd);

VOID AsicSetEdcaParm(
	IN PRTMP_ADAPTER pAd,
	IN PEDCA_PARM    pEdcaParm);

VOID AsicSetSlotTime(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bUseShortSlotTime);


VOID AsicAddSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         BssIndex,
	IN UCHAR         KeyIdx,
	IN UCHAR         CipherAlg,
	IN PUCHAR        pKey,
	IN PUCHAR        pTxMic,
	IN PUCHAR        pRxMic);

VOID AsicRemoveSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         BssIndex,
	IN UCHAR         KeyIdx);

VOID AsicUpdateWCIDAttribute(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN UCHAR		BssIndex,
	IN UCHAR        CipherAlg,
	IN BOOLEAN		bUsePairewiseKeyTable);

VOID AsicUpdateWCIDIVEIV(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN ULONG        uIV,
	IN ULONG        uEIV);

VOID AsicUpdateRxWCIDTable(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN PUCHAR        pAddr);

VOID AsicAddKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN UCHAR		BssIndex,
	IN UCHAR		KeyIdx,
	IN PCIPHER_KEY	pCipherKey,
	IN BOOLEAN		bUsePairewiseKeyTable,
	IN BOOLEAN		bTxKey);

VOID AsicAddPairwiseKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR        pAddr,
	IN UCHAR		WCID,
	IN CIPHER_KEY		 *pCipherKey);

VOID AsicRemovePairwiseKeyEntry(
	IN PRTMP_ADAPTER  pAd,
	IN UCHAR		 BssIdx,
	IN UCHAR		 Wcid);

BOOLEAN AsicSendCommandToMcu(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         Command,
	IN UCHAR         Token,
	IN UCHAR         Arg0,
	IN UCHAR         Arg1);


#ifdef RTMP_MAC_PCI
BOOLEAN AsicCheckCommanOk(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Command);
#endif 

VOID MacAddrRandomBssid(
	IN  PRTMP_ADAPTER   pAd,
	OUT PUCHAR pAddr);

VOID MgtMacHeaderInit(
	IN  PRTMP_ADAPTER     pAd,
	IN OUT PHEADER_802_11 pHdr80211,
	IN UCHAR SubType,
	IN UCHAR ToDs,
	IN PUCHAR pDA,
	IN PUCHAR pBssid);

VOID MlmeRadioOff(
	IN PRTMP_ADAPTER pAd);

VOID MlmeRadioOn(
	IN PRTMP_ADAPTER pAd);


VOID BssTableInit(
	IN BSS_TABLE *Tab);

#ifdef DOT11_N_SUPPORT
VOID BATableInit(
	IN PRTMP_ADAPTER pAd,
    IN BA_TABLE *Tab);
#endif 

ULONG BssTableSearch(
	IN BSS_TABLE *Tab,
	IN PUCHAR pBssid,
	IN UCHAR Channel);

ULONG BssSsidTableSearch(
	IN BSS_TABLE *Tab,
	IN PUCHAR    pBssid,
	IN PUCHAR    pSsid,
	IN UCHAR     SsidLen,
	IN UCHAR     Channel);

ULONG BssTableSearchWithSSID(
	IN BSS_TABLE *Tab,
	IN PUCHAR    Bssid,
	IN PUCHAR    pSsid,
	IN UCHAR     SsidLen,
	IN UCHAR     Channel);

ULONG BssSsidTableSearchBySSID(
	IN BSS_TABLE *Tab,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen);

VOID BssTableDeleteEntry(
	IN OUT  PBSS_TABLE pTab,
	IN      PUCHAR pBssid,
	IN      UCHAR Channel);

#ifdef DOT11_N_SUPPORT
VOID BATableDeleteORIEntry(
	IN OUT	PRTMP_ADAPTER pAd,
	IN		BA_ORI_ENTRY	*pBAORIEntry);

VOID BATableDeleteRECEntry(
	IN OUT	PRTMP_ADAPTER pAd,
	IN		BA_REC_ENTRY	*pBARECEntry);

VOID BATableTearORIEntry(
	IN OUT	PRTMP_ADAPTER pAd,
	IN		UCHAR TID,
	IN		UCHAR Wcid,
	IN		BOOLEAN bForceDelete,
	IN		BOOLEAN ALL);

VOID BATableTearRECEntry(
	IN OUT	PRTMP_ADAPTER pAd,
	IN		UCHAR TID,
	IN		UCHAR WCID,
	IN		BOOLEAN ALL);
#endif 

VOID  BssEntrySet(
	IN  PRTMP_ADAPTER   pAd,
	OUT PBSS_ENTRY pBss,
	IN PUCHAR pBssid,
	IN CHAR Ssid[],
	IN UCHAR SsidLen,
	IN UCHAR BssType,
	IN USHORT BeaconPeriod,
	IN PCF_PARM CfParm,
	IN USHORT AtimWin,
	IN USHORT CapabilityInfo,
	IN UCHAR SupRate[],
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN ADD_HT_INFO_IE *pAddHtInfo,	
	IN UCHAR			HtCapabilityLen,
	IN UCHAR			AddHtInfoLen,
	IN UCHAR			NewExtChanOffset,
	IN UCHAR Channel,
	IN CHAR Rssi,
	IN LARGE_INTEGER TimeStamp,
	IN UCHAR CkipFlag,
	IN PEDCA_PARM pEdcaParm,
	IN PQOS_CAPABILITY_PARM pQosCapability,
	IN PQBSS_LOAD_PARM pQbssLoad,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE);

ULONG  BssTableSetEntry(
	IN  PRTMP_ADAPTER   pAd,
	OUT PBSS_TABLE pTab,
	IN PUCHAR pBssid,
	IN CHAR Ssid[],
	IN UCHAR SsidLen,
	IN UCHAR BssType,
	IN USHORT BeaconPeriod,
	IN CF_PARM *CfParm,
	IN USHORT AtimWin,
	IN USHORT CapabilityInfo,
	IN UCHAR SupRate[],
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN ADD_HT_INFO_IE *pAddHtInfo,	
	IN UCHAR			HtCapabilityLen,
	IN UCHAR			AddHtInfoLen,
	IN UCHAR			NewExtChanOffset,
	IN UCHAR Channel,
	IN CHAR Rssi,
	IN LARGE_INTEGER TimeStamp,
	IN UCHAR CkipFlag,
	IN PEDCA_PARM pEdcaParm,
	IN PQOS_CAPABILITY_PARM pQosCapability,
	IN PQBSS_LOAD_PARM pQbssLoad,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE);

#ifdef DOT11_N_SUPPORT
VOID BATableInsertEntry(
    IN	PRTMP_ADAPTER	pAd,
	IN USHORT Aid,
    IN USHORT		TimeOutValue,
	IN USHORT		StartingSeq,
    IN UCHAR TID,
	IN UCHAR BAWinSize,
	IN UCHAR OriginatorStatus,
    IN BOOLEAN IsRecipient);

#ifdef DOT11N_DRAFT3
VOID Bss2040CoexistTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);


VOID  TriEventInit(
	IN	PRTMP_ADAPTER	pAd);

ULONG TriEventTableSetEntry(
	IN	PRTMP_ADAPTER	pAd,
	OUT TRIGGER_EVENT_TAB *Tab,
	IN PUCHAR pBssid,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR			HtCapabilityLen,
	IN UCHAR			RegClass,
	IN UCHAR ChannelNo);

VOID TriEventCounterMaintenance(
	IN	PRTMP_ADAPTER	pAd);
#endif 
#endif 

VOID BssTableSsidSort(
	IN  PRTMP_ADAPTER   pAd,
	OUT BSS_TABLE *OutTab,
	IN  CHAR Ssid[],
	IN  UCHAR SsidLen);

VOID  BssTableSortByRssi(
	IN OUT BSS_TABLE *OutTab);

VOID BssCipherParse(
	IN OUT  PBSS_ENTRY  pBss);

NDIS_STATUS  MlmeQueueInit(
	IN MLME_QUEUE *Queue);

VOID  MlmeQueueDestroy(
	IN MLME_QUEUE *Queue);

BOOLEAN MlmeEnqueue(
	IN PRTMP_ADAPTER pAd,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg);

BOOLEAN MlmeEnqueueForRecv(
	IN  PRTMP_ADAPTER   pAd,
	IN ULONG Wcid,
	IN ULONG TimeStampHigh,
	IN ULONG TimeStampLow,
	IN UCHAR Rssi0,
	IN UCHAR Rssi1,
	IN UCHAR Rssi2,
	IN ULONG MsgLen,
	IN PVOID Msg,
	IN UCHAR Signal);


BOOLEAN MlmeDequeue(
	IN MLME_QUEUE *Queue,
	OUT MLME_QUEUE_ELEM **Elem);

VOID    MlmeRestartStateMachine(
	IN  PRTMP_ADAPTER   pAd);

BOOLEAN  MlmeQueueEmpty(
	IN MLME_QUEUE *Queue);

BOOLEAN  MlmeQueueFull(
	IN MLME_QUEUE *Queue);

BOOLEAN  MsgTypeSubst(
	IN PRTMP_ADAPTER pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType);

VOID StateMachineInit(
	IN STATE_MACHINE *Sm,
	IN STATE_MACHINE_FUNC Trans[],
	IN ULONG StNr,
	IN ULONG MsgNr,
	IN STATE_MACHINE_FUNC DefFunc,
	IN ULONG InitState,
	IN ULONG Base);

VOID StateMachineSetAction(
	IN STATE_MACHINE *S,
	IN ULONG St,
	ULONG Msg,
	IN STATE_MACHINE_FUNC F);

VOID StateMachinePerformAction(
	IN  PRTMP_ADAPTER   pAd,
	IN STATE_MACHINE *S,
	IN MLME_QUEUE_ELEM *Elem);

VOID Drop(
	IN  PRTMP_ADAPTER   pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID AssocStateMachineInit(
	IN  PRTMP_ADAPTER   pAd,
	IN  STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID ReassocTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID AssocTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID DisassocTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);


VOID MlmeDisassocReqAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeAssocReqAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeReassocReqAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeDisassocReqAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerAssocRspAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerReassocRspAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerDisassocAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID DisassocTimeoutAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID AssocTimeoutAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID  ReassocTimeoutAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID  Cls3errAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR pAddr);

VOID  InvalidStateWhenAssoc(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID  InvalidStateWhenReassoc(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenDisassociate(
	IN  PRTMP_ADAPTER pAd,
	IN  MLME_QUEUE_ELEM *Elem);


VOID  ComposePsPoll(
	IN  PRTMP_ADAPTER   pAd);

VOID  ComposeNullFrame(
	IN  PRTMP_ADAPTER pAd);

VOID  AssocPostProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR pAddr2,
	IN  USHORT CapabilityInfo,
	IN  USHORT Aid,
	IN  UCHAR SupRate[],
	IN  UCHAR SupRateLen,
	IN  UCHAR ExtRate[],
	IN  UCHAR ExtRateLen,
	IN PEDCA_PARM pEdcaParm,
	IN HT_CAPABILITY_IE		*pHtCapability,
	IN  UCHAR HtCapabilityLen,
	IN ADD_HT_INFO_IE		*pAddHtInfo);

VOID AuthStateMachineInit(
	IN  PRTMP_ADAPTER   pAd,
	IN PSTATE_MACHINE sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID AuthTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID MlmeAuthReqAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerAuthRspAtSeq2Action(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerAuthRspAtSeq4Action(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID AuthTimeoutAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID Cls2errAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR pAddr);

VOID MlmeDeauthReqAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenAuth(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);



VOID AuthRspStateMachineInit(
	IN  PRTMP_ADAPTER   pAd,
	IN  PSTATE_MACHINE Sm,
	IN  STATE_MACHINE_FUNC Trans[]);

VOID PeerDeauthAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerAuthSimpleRspGenAndSend(
	IN  PRTMP_ADAPTER   pAd,
	IN  PHEADER_802_11  pHdr80211,
	IN  USHORT Alg,
	IN  USHORT Seq,
	IN  USHORT Reason,
	IN  USHORT Status);





#ifdef CONFIG_STA_SUPPORT
#ifdef QOS_DLS_SUPPORT
void DlsStateMachineInit(
    IN PRTMP_ADAPTER pAd,
    IN STATE_MACHINE *Sm,
    OUT STATE_MACHINE_FUNC Trans[]);

VOID MlmeDlsReqAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerDlsReqAction(
    IN PRTMP_ADAPTER	pAd,
    IN MLME_QUEUE_ELEM	*Elem);

VOID PeerDlsRspAction(
    IN PRTMP_ADAPTER	pAd,
    IN MLME_QUEUE_ELEM	*Elem);

VOID MlmeDlsTearDownAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID PeerDlsTearDownAction(
    IN PRTMP_ADAPTER	pAd,
    IN MLME_QUEUE_ELEM	*Elem);

VOID RTMPCheckDLSTimeOut(
	IN PRTMP_ADAPTER	pAd);

BOOLEAN RTMPRcvFrameDLSCheck(
	IN PRTMP_ADAPTER	pAd,
	IN PHEADER_802_11	pHeader,
	IN ULONG			Len,
	IN PRT28XX_RXD_STRUC	pRxD);

INT	RTMPCheckDLSFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN  PUCHAR          pDA);

VOID RTMPSendDLSTearDownFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN  PUCHAR          pDA);

NDIS_STATUS RTMPSendSTAKeyRequest(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pDA);

NDIS_STATUS RTMPSendSTAKeyHandShake(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pDA);

VOID DlsTimeoutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

BOOLEAN MlmeDlsReqSanity(
	IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PRT_802_11_DLS *pDLS,
    OUT PUSHORT pReason);

INT Set_DlsEntryInfo_Display_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR arg);

MAC_TABLE_ENTRY *MacTableInsertDlsEntry(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR	pAddr,
	IN  UINT	DlsEntryIdx);

BOOLEAN MacTableDeleteDlsEntry(
	IN PRTMP_ADAPTER pAd,
	IN USHORT wcid,
	IN PUCHAR pAddr);

MAC_TABLE_ENTRY *DlsEntryTableLookup(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	pAddr,
	IN BOOLEAN	bResetIdelCount);

MAC_TABLE_ENTRY *DlsEntryTableLookupByWcid(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	wcid,
	IN PUCHAR	pAddr,
	IN BOOLEAN	bResetIdelCount);

INT	Set_DlsAddEntry_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_DlsTearDownEntry_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);
#endif 
#endif 

#ifdef QOS_DLS_SUPPORT
BOOLEAN PeerDlsReqSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pDA,
    OUT PUCHAR pSA,
    OUT USHORT *pCapabilityInfo,
    OUT USHORT *pDlsTimeout,
    OUT UCHAR *pRatesLen,
    OUT UCHAR Rates[],
    OUT UCHAR *pHtCapabilityLen,
    OUT HT_CAPABILITY_IE *pHtCapability);

BOOLEAN PeerDlsRspSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pDA,
    OUT PUCHAR pSA,
    OUT USHORT *pCapabilityInfo,
    OUT USHORT *pStatus,
    OUT UCHAR *pRatesLen,
    OUT UCHAR Rates[],
    OUT UCHAR *pHtCapabilityLen,
    OUT HT_CAPABILITY_IE *pHtCapability);

BOOLEAN PeerDlsTearDownSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pDA,
    OUT PUCHAR pSA,
    OUT USHORT *pReason);
#endif 



VOID SyncStateMachineInit(
	IN  PRTMP_ADAPTER   pAd,
	IN  STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID BeaconTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID ScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID MlmeScanReqAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenScan(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenJoin(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenStart(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeacon(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID EnqueueProbeRequest(
	IN PRTMP_ADAPTER pAd);

BOOLEAN ScanRunning(
		IN PRTMP_ADAPTER pAd);


VOID MlmeCntlInit(
	IN  PRTMP_ADAPTER   pAd,
	IN  STATE_MACHINE *S,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID MlmeCntlMachinePerformAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  STATE_MACHINE *S,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlIdleProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlOidScanProc(
	IN  PRTMP_ADAPTER pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlOidSsidProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM * Elem);

VOID CntlOidRTBssidProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM * Elem);

VOID CntlMlmeRoamingProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM * Elem);

VOID CntlWaitDisassocProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitJoinProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitReassocProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitStartProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitAuthProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitAuthProc2(
	IN  PRTMP_ADAPTER pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID CntlWaitAssocProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

#ifdef QOS_DLS_SUPPORT
VOID CntlOidDLSSetupProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);
#endif 

VOID LinkUp(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR BssType);

VOID LinkDown(
	IN  PRTMP_ADAPTER   pAd,
	IN  BOOLEAN         IsReqFromAP);

VOID IterateOnBssTab(
	IN  PRTMP_ADAPTER   pAd);

VOID IterateOnBssTab2(
	IN  PRTMP_ADAPTER   pAd);;

VOID JoinParmFill(
	IN  PRTMP_ADAPTER   pAd,
	IN  OUT MLME_JOIN_REQ_STRUCT *JoinReq,
	IN  ULONG BssIdx);

VOID AssocParmFill(
	IN  PRTMP_ADAPTER   pAd,
	IN OUT MLME_ASSOC_REQ_STRUCT *AssocReq,
	IN  PUCHAR pAddr,
	IN  USHORT CapabilityInfo,
	IN  ULONG Timeout,
	IN  USHORT ListenIntv);

VOID ScanParmFill(
	IN  PRTMP_ADAPTER   pAd,
	IN  OUT MLME_SCAN_REQ_STRUCT *ScanReq,
	IN  STRING Ssid[],
	IN  UCHAR SsidLen,
	IN  UCHAR BssType,
	IN  UCHAR ScanType);

VOID DisassocParmFill(
	IN  PRTMP_ADAPTER   pAd,
	IN  OUT MLME_DISASSOC_REQ_STRUCT *DisassocReq,
	IN  PUCHAR pAddr,
	IN  USHORT Reason);

VOID StartParmFill(
	IN  PRTMP_ADAPTER   pAd,
	IN  OUT MLME_START_REQ_STRUCT *StartReq,
	IN  CHAR Ssid[],
	IN  UCHAR SsidLen);

VOID AuthParmFill(
	IN  PRTMP_ADAPTER   pAd,
	IN  OUT MLME_AUTH_REQ_STRUCT *AuthReq,
	IN  PUCHAR pAddr,
	IN  USHORT Alg);

VOID EnqueuePsPoll(
	IN  PRTMP_ADAPTER   pAd);

VOID EnqueueBeaconFrame(
	IN  PRTMP_ADAPTER   pAd);

VOID MlmeJoinReqAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeScanReqAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID MlmeStartReqAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID ScanTimeoutAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID BeaconTimeoutAtJoinAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeaconAtScanAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeaconAtJoinAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerBeacon(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID PeerProbeReqAction(
	IN  PRTMP_ADAPTER pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID ScanNextChannel(
	IN  PRTMP_ADAPTER   pAd);

ULONG MakeIbssBeacon(
	IN  PRTMP_ADAPTER   pAd);

VOID CCXAdjacentAPReport(
	IN  PRTMP_ADAPTER   pAd);

BOOLEAN MlmeScanReqSanity(
	IN  PRTMP_ADAPTER   pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT UCHAR *BssType,
	OUT CHAR ssid[],
	OUT UCHAR *SsidLen,
	OUT UCHAR *ScanType);

BOOLEAN PeerBeaconAndProbeRspSanity(
	IN  PRTMP_ADAPTER   pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	IN  UCHAR MsgChannel,
	OUT PUCHAR pAddr2,
	OUT PUCHAR pBssid,
	OUT CHAR Ssid[],
	OUT UCHAR *pSsidLen,
	OUT UCHAR *pBssType,
	OUT USHORT *pBeaconPeriod,
	OUT UCHAR *pChannel,
	OUT UCHAR *pNewChannel,
	OUT LARGE_INTEGER *pTimestamp,
	OUT CF_PARM *pCfParm,
	OUT USHORT *pAtimWin,
	OUT USHORT *pCapabilityInfo,
	OUT UCHAR *pErp,
	OUT UCHAR *pDtimCount,
	OUT UCHAR *pDtimPeriod,
	OUT UCHAR *pBcastFlag,
	OUT UCHAR *pMessageToMe,
	OUT UCHAR SupRate[],
	OUT UCHAR *pSupRateLen,
	OUT UCHAR ExtRate[],
	OUT UCHAR *pExtRateLen,
	OUT	UCHAR *pCkipFlag,
	OUT	UCHAR *pAironetCellPowerLimit,
	OUT PEDCA_PARM       pEdcaParm,
	OUT PQBSS_LOAD_PARM  pQbssLoad,
	OUT PQOS_CAPABILITY_PARM pQosCapability,
	OUT ULONG *pRalinkIe,
	OUT UCHAR		 *pHtCapabilityLen,
#ifdef CONFIG_STA_SUPPORT
	OUT UCHAR		 *pPreNHtCapabilityLen,
#endif 
	OUT HT_CAPABILITY_IE *pHtCapability,
	OUT UCHAR		 *AddHtInfoLen,
	OUT ADD_HT_INFO_IE *AddHtInfo,
	OUT UCHAR *NewExtChannel,
	OUT USHORT *LengthVIE,
	OUT PNDIS_802_11_VARIABLE_IEs pVIE);

BOOLEAN PeerAddBAReqActionSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *pMsg,
    IN ULONG MsgLen,
	OUT PUCHAR pAddr2);

BOOLEAN PeerAddBARspActionSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *pMsg,
    IN ULONG MsgLen);

BOOLEAN PeerDelBAActionSanity(
    IN PRTMP_ADAPTER pAd,
    IN UCHAR Wcid,
    IN VOID *pMsg,
    IN ULONG MsgLen);

BOOLEAN MlmeAssocReqSanity(
	IN  PRTMP_ADAPTER   pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pApAddr,
	OUT USHORT *CapabilityInfo,
	OUT ULONG *Timeout,
	OUT USHORT *ListenIntv);

BOOLEAN MlmeAuthReqSanity(
	IN  PRTMP_ADAPTER   pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr,
	OUT ULONG *Timeout,
	OUT USHORT *Alg);

BOOLEAN MlmeStartReqSanity(
	IN  PRTMP_ADAPTER   pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT CHAR Ssid[],
	OUT UCHAR *Ssidlen);

BOOLEAN PeerAuthSanity(
	IN  PRTMP_ADAPTER   pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr,
	OUT USHORT *Alg,
	OUT USHORT *Seq,
	OUT USHORT *Status,
	OUT CHAR ChlgText[]);

BOOLEAN PeerAssocRspSanity(
	IN  PRTMP_ADAPTER   pAd,
    IN VOID *pMsg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr2,
	OUT USHORT *pCapabilityInfo,
	OUT USHORT *pStatus,
	OUT USHORT *pAid,
	OUT UCHAR SupRate[],
	OUT UCHAR *pSupRateLen,
	OUT UCHAR ExtRate[],
	OUT UCHAR *pExtRateLen,
    OUT HT_CAPABILITY_IE		*pHtCapability,
    OUT ADD_HT_INFO_IE		*pAddHtInfo,	
    OUT UCHAR			*pHtCapabilityLen,
    OUT UCHAR			*pAddHtInfoLen,
    OUT UCHAR			*pNewExtChannelOffset,
	OUT PEDCA_PARM pEdcaParm,
	OUT UCHAR *pCkipFlag);

BOOLEAN PeerDisassocSanity(
	IN  PRTMP_ADAPTER   pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr2,
	OUT USHORT *Reason);

BOOLEAN PeerWpaMessageSanity(
    IN	PRTMP_ADAPTER		pAd,
    IN	PEAPOL_PACKET		pMsg,
    IN	ULONG				MsgLen,
    IN	UCHAR				MsgType,
    IN	MAC_TABLE_ENTRY		*pEntry);

BOOLEAN PeerDeauthSanity(
	IN  PRTMP_ADAPTER   pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr2,
	OUT USHORT *Reason);

BOOLEAN PeerProbeReqSanity(
	IN  PRTMP_ADAPTER   pAd,
	IN  VOID *Msg,
	IN  ULONG MsgLen,
	OUT PUCHAR pAddr2,
	OUT CHAR Ssid[],
	OUT UCHAR *pSsidLen);

BOOLEAN GetTimBit(
	IN  CHAR *Ptr,
	IN  USHORT Aid,
	OUT UCHAR *TimLen,
	OUT UCHAR *BcastFlag,
	OUT UCHAR *DtimCount,
	OUT UCHAR *DtimPeriod,
	OUT UCHAR *MessageToMe);

UCHAR ChannelSanity(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel);

NDIS_802_11_NETWORK_TYPE NetworkTypeInUseSanity(
	IN PBSS_ENTRY pBss);


BOOLEAN MlmeDelBAReqSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen);

BOOLEAN MlmeAddBAReqSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2);

ULONG MakeOutgoingFrame(
	OUT UCHAR *Buffer,
	OUT ULONG *Length, ...);

VOID  LfsrInit(
	IN  PRTMP_ADAPTER   pAd,
	IN  ULONG Seed);

UCHAR RandomByte(
	IN  PRTMP_ADAPTER   pAd);

VOID AsicUpdateAutoFallBackTable(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pTxRate);

VOID  MlmePeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID LinkDownExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID LinkUpExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID STAMlmePeriodicExec(
	PRTMP_ADAPTER pAd);

VOID MlmeAutoScan(
	IN PRTMP_ADAPTER pAd);

VOID MlmeAutoReconnectLastSSID(
	IN PRTMP_ADAPTER pAd);

BOOLEAN MlmeValidateSSID(
	IN PUCHAR pSsid,
	IN UCHAR  SsidLen);

VOID MlmeCheckForRoaming(
	IN PRTMP_ADAPTER pAd,
	IN ULONG    Now32);

BOOLEAN MlmeCheckForFastRoaming(
	IN  PRTMP_ADAPTER   pAd);

VOID MlmeDynamicTxRateSwitching(
	IN PRTMP_ADAPTER pAd);

VOID MlmeSetTxRate(
	IN PRTMP_ADAPTER		pAd,
	IN PMAC_TABLE_ENTRY		pEntry,
	IN PRTMP_TX_RATE_SWITCH	pTxRate);

VOID MlmeSelectTxRateTable(
	IN PRTMP_ADAPTER		pAd,
	IN PMAC_TABLE_ENTRY		pEntry,
	IN PUCHAR				*ppTable,
	IN PUCHAR				pTableSize,
	IN PUCHAR				pInitTxRateIdx);

VOID MlmeCalculateChannelQuality(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN ULONG Now);

VOID MlmeCheckPsmChange(
	IN PRTMP_ADAPTER pAd,
	IN ULONG    Now32);

VOID MlmeSetPsmBit(
	IN PRTMP_ADAPTER pAd,
	IN USHORT psm);

VOID MlmeSetTxPreamble(
	IN PRTMP_ADAPTER pAd,
	IN USHORT TxPreamble);

VOID UpdateBasicRateBitmap(
	IN	PRTMP_ADAPTER	pAd);

VOID MlmeUpdateTxRates(
	IN PRTMP_ADAPTER	pAd,
	IN	BOOLEAN			bLinkUp,
	IN	UCHAR			apidx);

#ifdef DOT11_N_SUPPORT
VOID MlmeUpdateHtTxRates(
	IN PRTMP_ADAPTER		pAd,
	IN	UCHAR				apidx);
#endif 

VOID    RTMPCheckRates(
	IN      PRTMP_ADAPTER   pAd,
	IN OUT  UCHAR           SupRate[],
	IN OUT  UCHAR           *SupRateLen);

#ifdef CONFIG_STA_SUPPORT
BOOLEAN RTMPCheckChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		CentralChannel,
	IN UCHAR		Channel);
#endif 

BOOLEAN		RTMPCheckHt(
	IN		PRTMP_ADAPTER	pAd,
	IN		UCHAR	Wcid,
	IN OUT	HT_CAPABILITY_IE			*pHtCapability,
	IN OUT	ADD_HT_INFO_IE			*pAddHtInfo);

VOID StaQuickResponeForRateUpExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID AsicBbpTuning1(
	IN PRTMP_ADAPTER pAd);

VOID AsicBbpTuning2(
	IN PRTMP_ADAPTER pAd);

VOID RTMPUpdateMlmeRate(
	IN PRTMP_ADAPTER	pAd);

CHAR RTMPMaxRssi(
	IN PRTMP_ADAPTER	pAd,
	IN CHAR				Rssi0,
	IN CHAR				Rssi1,
	IN CHAR				Rssi2);

#ifdef RT30xx
VOID AsicSetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant);

VOID RTMPFilterCalibration(
	IN PRTMP_ADAPTER pAd);

#ifdef RTMP_EFUSE_SUPPORT

INT set_eFuseGetFreeBlockCount_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT set_eFusedump_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT set_eFuseLoadFromBin_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

VOID eFusePhysicalReadRegisters(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	OUT	USHORT* pData);

int RtmpEfuseSupportCheck(
	IN RTMP_ADAPTER *pAd);

INT set_eFuseBufferModeWriteBack_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT eFuseLoadEEPROM(
	IN PRTMP_ADAPTER pAd);

INT eFuseWriteEeeppromBuf(
	IN PRTMP_ADAPTER pAd);

VOID eFuseGetFreeBlockCount(IN PRTMP_ADAPTER pAd,
	PUINT EfuseFreeBlock);

INT eFuse_init(
	IN PRTMP_ADAPTER pAd);

NTSTATUS eFuseRead(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	OUT	PUCHAR			pData,
	IN	USHORT			Length);

NTSTATUS eFuseWrite(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	PUCHAR			pData,
	IN	USHORT			length);

#endif 


VOID RT30xxLoadRFNormalModeSetup(
	IN PRTMP_ADAPTER	pAd);

VOID RT30xxLoadRFSleepModeSetup(
	IN PRTMP_ADAPTER	pAd);

VOID RT30xxReverseRFSleepModeSetup(
	IN PRTMP_ADAPTER	pAd);



#ifdef RT3090
VOID NICInitRT3090RFRegisters(
	IN RTMP_ADAPTER *pAd);
#endif 

VOID RT30xxHaltAction(
	IN PRTMP_ADAPTER	pAd);

VOID RT30xxSetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant);
#endif 
#ifdef RT33xx
VOID RT33xxLoadRFNormalModeSetup(
	IN PRTMP_ADAPTER	pAd);

VOID RT33xxLoadRFSleepModeSetup(
	IN PRTMP_ADAPTER	pAd);

VOID RT33xxReverseRFSleepModeSetup(
	IN PRTMP_ADAPTER	pAd);

#ifdef RT3370
VOID NICInitRT3370RFRegisters(
	IN RTMP_ADAPTER *pAd);
#endif 

#ifdef RT3390
VOID NICInitRT3390RFRegisters(
	IN RTMP_ADAPTER *pAd);
#endif 

VOID RT33xxHaltAction(
	IN PRTMP_ADAPTER	pAd);

VOID RT33xxSetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant);

#endif 



VOID AsicEvaluateRxAnt(
	IN PRTMP_ADAPTER	pAd);

VOID AsicRxAntEvalTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID APSDPeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

BOOLEAN RTMPCheckEntryEnableAutoRateSwitch(
	IN PRTMP_ADAPTER    pAd,
	IN PMAC_TABLE_ENTRY	pEntry);

UCHAR RTMPStaFixedTxMode(
	IN PRTMP_ADAPTER    pAd,
	IN PMAC_TABLE_ENTRY	pEntry);

VOID RTMPUpdateLegacyTxSetting(
		UCHAR				fixed_tx_mode,
		PMAC_TABLE_ENTRY	pEntry);

BOOLEAN RTMPAutoRateSwitchCheck(
	IN PRTMP_ADAPTER    pAd);

NDIS_STATUS MlmeInit(
	IN  PRTMP_ADAPTER   pAd);

VOID MlmeHandler(
	IN  PRTMP_ADAPTER   pAd);

VOID MlmeHalt(
	IN  PRTMP_ADAPTER   pAd);

VOID MlmeResetRalinkCounters(
	IN  PRTMP_ADAPTER   pAd);

VOID BuildChannelList(
	IN PRTMP_ADAPTER pAd);

UCHAR FirstChannel(
	IN  PRTMP_ADAPTER   pAd);

UCHAR NextChannel(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR channel);

VOID ChangeToCellPowerLimit(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         AironetCellPowerLimit);




VOID    RTMPInitTkipEngine(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pTKey,
	IN  UCHAR           KeyId,
	IN  PUCHAR          pTA,
	IN  PUCHAR          pMICKey,
	IN  PUCHAR          pTSC,
	OUT PULONG          pIV16,
	OUT PULONG          pIV32);

VOID    RTMPInitMICEngine(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pKey,
	IN  PUCHAR          pDA,
	IN  PUCHAR          pSA,
	IN  UCHAR           UserPriority,
	IN  PUCHAR          pMICKey);

BOOLEAN RTMPTkipCompareMICValue(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pSrc,
	IN  PUCHAR          pDA,
	IN  PUCHAR          pSA,
	IN  PUCHAR          pMICKey,
	IN	UCHAR			UserPriority,
	IN  UINT            Len);

VOID    RTMPCalculateMICValue(
	IN  PRTMP_ADAPTER   pAd,
	IN  PNDIS_PACKET    pPacket,
	IN  PUCHAR          pEncap,
	IN  PCIPHER_KEY     pKey,
	IN	UCHAR			apidx);

BOOLEAN RTMPTkipCompareMICValueWithLLC(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pLLC,
	IN  PUCHAR          pSrc,
	IN  PUCHAR          pDA,
	IN  PUCHAR          pSA,
	IN  PUCHAR          pMICKey,
	IN  UINT            Len);

VOID    RTMPTkipAppendByte(
	IN  PTKIP_KEY_INFO  pTkip,
	IN  UCHAR           uChar);

VOID    RTMPTkipAppend(
	IN  PTKIP_KEY_INFO  pTkip,
	IN  PUCHAR          pSrc,
	IN  UINT            nBytes);

VOID    RTMPTkipGetMIC(
	IN  PTKIP_KEY_INFO  pTkip);

BOOLEAN RTMPSoftDecryptTKIP(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	pData,
	IN ULONG	DataByteCnt,
	IN UCHAR    UserPriority,
	IN PCIPHER_KEY	pWpaKey);

BOOLEAN RTMPSoftDecryptAES(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR	pData,
	IN ULONG	DataByteCnt,
	IN PCIPHER_KEY	pWpaKey);






INT RT_CfgSetCountryRegion(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg,
	IN INT				band);

INT RT_CfgSetWirelessMode(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT RT_CfgSetShortSlot(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	RT_CfgSetWepKey(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			keyString,
	IN	CIPHER_KEY		*pSharedKey,
	IN	INT				keyIdx);

INT RT_CfgSetWPAPSKKey(
	IN RTMP_ADAPTER	*pAd,
	IN PSTRING		keyString,
	IN UCHAR		*pHashStr,
	IN INT			hashStrLen,
	OUT PUCHAR		pPMKBuf);






NDIS_STATUS RTMPWPARemoveKeyProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  PVOID           pBuf);

VOID    RTMPWPARemoveAllKeys(
	IN  PRTMP_ADAPTER   pAd);

BOOLEAN RTMPCheckStrPrintAble(
    IN  CHAR *pInPutStr,
    IN  UCHAR strLen);

VOID    RTMPSetPhyMode(
	IN  PRTMP_ADAPTER   pAd,
	IN  ULONG phymode);

VOID	RTMPUpdateHTIE(
	IN	RT_HT_CAPABILITY	*pRtHt,
	IN		UCHAR				*pMcsSet,
	OUT		HT_CAPABILITY_IE *pHtCapability,
	OUT		ADD_HT_INFO_IE		*pAddHtInfo);

VOID	RTMPAddWcidAttributeEntry(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			BssIdx,
	IN	UCHAR			KeyIdx,
	IN	UCHAR			CipherAlg,
	IN	MAC_TABLE_ENTRY *pEntry);

PSTRING GetEncryptType(
	CHAR enc);

PSTRING GetAuthMode(
	CHAR auth);


VOID RTMPIndicateWPA2Status(
	IN  PRTMP_ADAPTER  pAdapter);

VOID	RTMPOPModeSwitching(
	IN	PRTMP_ADAPTER	pAd);


#ifdef DOT11_N_SUPPORT
VOID	RTMPSetHT(
	IN	PRTMP_ADAPTER	pAd,
	IN	OID_SET_HT_PHYMODE *pHTPhyMode);

VOID	RTMPSetIndividualHT(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				apidx);
#endif 

VOID RTMPSendWirelessEvent(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Event_flag,
	IN	PUCHAR			pAddr,
	IN  UCHAR			BssIdx,
	IN	CHAR			Rssi);

VOID	NICUpdateCntlCounters(
	IN	PRTMP_ADAPTER	pAd,
	IN	PHEADER_802_11	pHeader,
	IN    UCHAR			SubType,
	IN	PRXWI_STRUC	pRxWI);

VOID    DBGPRINT_TX_RING(
	IN PRTMP_ADAPTER  pAd,
	IN UCHAR          QueIdx);

VOID DBGPRINT_RX_RING(
	IN PRTMP_ADAPTER  pAd);

CHAR    ConvertToRssi(
	IN PRTMP_ADAPTER  pAd,
	IN CHAR				Rssi,
	IN UCHAR    RssiNumber);


#ifdef DOT11N_DRAFT3
VOID BuildEffectedChannelList(
	IN PRTMP_ADAPTER pAd);
#endif 


VOID APAsicEvaluateRxAnt(
	IN PRTMP_ADAPTER	pAd);

#ifdef ANT_DIVERSITY_SUPPORT
VOID	APAsicAntennaAvg(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	              AntSelect,
	IN	SHORT	              *RssiAvg);
#endif 

VOID APAsicRxAntEvalTimeout(
	IN PRTMP_ADAPTER	pAd);


VOID	RTMPToWirelessSta(
    IN  PRTMP_ADAPTER		pAd,
    IN  PMAC_TABLE_ENTRY	pEntry,
    IN  PUCHAR			pHeader802_3,
    IN  UINT			HdrLen,
    IN  PUCHAR			pData,
    IN  UINT			DataLen,
    IN	BOOLEAN				bClearFrame);

VOID WpaDerivePTK(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR   *PMK,
	IN  UCHAR   *ANonce,
	IN  UCHAR   *AA,
	IN  UCHAR   *SNonce,
	IN  UCHAR   *SA,
	OUT UCHAR   *output,
	IN  UINT    len);

VOID    GenRandom(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			*macAddr,
	OUT	UCHAR			*random);

BOOLEAN RTMPCheckWPAframe(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN PUCHAR			pData,
	IN ULONG			DataByteCount,
	IN UCHAR			FromWhichBSSID);

VOID AES_GTK_KEY_UNWRAP(
	IN  UCHAR   *key,
	OUT UCHAR   *plaintext,
	IN	UINT32	c_len,
	IN  UCHAR   *ciphertext);

BOOLEAN RTMPParseEapolKeyData(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pKeyData,
	IN  UCHAR           KeyDataLen,
	IN	UCHAR			GroupKeyIndex,
	IN	UCHAR			MsgType,
	IN	BOOLEAN			bWPA2,
	IN  MAC_TABLE_ENTRY *pEntry);

VOID	ConstructEapolMsg(
	IN	PMAC_TABLE_ENTRY	pEntry,
    IN	UCHAR				GroupKeyWepStatus,
    IN	UCHAR				MsgType,
    IN	UCHAR				DefaultKeyIdx,
	IN	UCHAR				*KeyNonce,
	IN	UCHAR				*TxRSC,
	IN	UCHAR				*GTK,
	IN	UCHAR				*RSNIE,
	IN	UCHAR				RSNIE_Len,
    OUT PEAPOL_PACKET       pMsg);

NDIS_STATUS	RTMPSoftDecryptBroadCastData(
	IN	PRTMP_ADAPTER					pAd,
	IN	RX_BLK							*pRxBlk,
	IN  NDIS_802_11_ENCRYPTION_STATUS	GroupCipher,
	IN  PCIPHER_KEY						pShard_key);

VOID RTMPMakeRSNIE(
	IN  PRTMP_ADAPTER   pAd,
	IN  UINT            AuthMode,
	IN  UINT            WepStatus,
	IN	UCHAR			apidx);




VOID RTMPGetTxTscFromAsic(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			apidx,
	OUT	PUCHAR			pTxTsc);

VOID APInstallPairwiseKey(
	PRTMP_ADAPTER		pAd,
	PMAC_TABLE_ENTRY	pEntry);

MAC_TABLE_ENTRY *PACInquiry(
	IN  PRTMP_ADAPTER   pAd,
	IN  ULONG           Wcid);

UINT	APValidateRSNIE(
	IN PRTMP_ADAPTER    pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PUCHAR			pRsnIe,
	IN UCHAR			rsnie_len);

VOID HandleCounterMeasure(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY  *pEntry);

VOID WPAStart4WayHS(
	IN  PRTMP_ADAPTER   pAd,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN	ULONG			TimeInterval);

VOID WPAStart2WayGroupHS(
	IN  PRTMP_ADAPTER   pAd,
	IN  MAC_TABLE_ENTRY *pEntry);

VOID PeerPairMsg1Action(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerPairMsg2Action(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerPairMsg3Action(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerPairMsg4Action(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerGroupMsg1Action(
	IN  PRTMP_ADAPTER    pAd,
	IN  PMAC_TABLE_ENTRY pEntry,
    IN  MLME_QUEUE_ELEM  *Elem);

VOID PeerGroupMsg2Action(
	IN  PRTMP_ADAPTER    pAd,
	IN  PMAC_TABLE_ENTRY pEntry,
	IN  VOID             *Msg,
	IN  UINT             MsgLen);

VOID CMTimerExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID WPARetryExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID EnqueueStartForPSKExec(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3);

VOID RTMPHandleSTAKey(
    IN PRTMP_ADAPTER    pAdapter,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem);

VOID PairDisAssocAction(
	IN  PRTMP_ADAPTER    pAd,
	IN  PMAC_TABLE_ENTRY pEntry,
	IN  USHORT           Reason);

VOID MlmeDeAuthAction(
	IN  PRTMP_ADAPTER    pAd,
	IN  PMAC_TABLE_ENTRY pEntry,
	IN  USHORT           Reason);

VOID GREKEYPeriodicExec(
	IN  PVOID   SystemSpecific1,
	IN  PVOID   FunctionContext,
	IN  PVOID   SystemSpecific2,
	IN  PVOID   SystemSpecific3);

VOID WpaDeriveGTK(
	IN  UCHAR   *PMK,
	IN  UCHAR   *GNonce,
	IN  UCHAR   *AA,
	OUT UCHAR   *output,
	IN  UINT    len);

VOID AES_GTK_KEY_WRAP(
	IN UCHAR *key,
	IN UCHAR *plaintext,
	IN UINT32 p_len,
	OUT UCHAR *ciphertext);

VOID AES_128_CMAC(
	IN	PUCHAR	key,
	IN	PUCHAR	input,
	IN	INT		len,
	OUT	PUCHAR	mac);

VOID    WpaSend(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          pPacket,
    IN  ULONG           Len);

VOID RTMPAddPMKIDCache(
	IN  PRTMP_ADAPTER		pAd,
	IN	INT						apidx,
	IN	PUCHAR				pAddr,
	IN	UCHAR					*PMKID,
	IN	UCHAR					*PMK);

INT RTMPSearchPMKIDCache(
	IN  PRTMP_ADAPTER   pAd,
	IN	INT				apidx,
	IN	PUCHAR		pAddr);

VOID RTMPDeletePMKIDCache(
	IN  PRTMP_ADAPTER   pAd,
	IN	INT				apidx,
	IN  INT				idx);

VOID RTMPMaintainPMKIDCache(
	IN  PRTMP_ADAPTER   pAd);

VOID	RTMPSendTriggerFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	PVOID			pBuffer,
	IN	ULONG			Length,
	IN  UCHAR           TxRate,
	IN	BOOLEAN			bQosNull);





VOID RTMP_SetPeriodicTimer(
	IN	NDIS_MINIPORT_TIMER *pTimer,
	IN	unsigned long timeout);

VOID RTMP_OS_Init_Timer(
	IN	PRTMP_ADAPTER pAd,
	IN	NDIS_MINIPORT_TIMER *pTimer,
	IN	TIMER_FUNCTION function,
	IN	PVOID data);

VOID RTMP_OS_Add_Timer(
	IN	NDIS_MINIPORT_TIMER	*pTimer,
	IN	unsigned long timeout);

VOID RTMP_OS_Mod_Timer(
	IN	NDIS_MINIPORT_TIMER	*pTimer,
	IN	unsigned long timeout);


VOID RTMP_OS_Del_Timer(
	IN	NDIS_MINIPORT_TIMER	*pTimer,
	OUT	BOOLEAN				 *pCancelled);


VOID RTMP_OS_Release_Packet(
	IN	PRTMP_ADAPTER pAd,
	IN	PQUEUE_ENTRY  pEntry);

VOID RTMPusecDelay(
	IN	ULONG	usec);

NDIS_STATUS os_alloc_mem(
	IN	RTMP_ADAPTER *pAd,
	OUT	UCHAR **mem,
	IN	ULONG  size);

NDIS_STATUS os_free_mem(
	IN	PRTMP_ADAPTER pAd,
	IN	PVOID mem);


void RTMP_AllocateSharedMemory(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress);

VOID RTMPFreeTxRxRingMemory(
    IN  PRTMP_ADAPTER   pAd);

NDIS_STATUS AdapterBlockAllocateMemory(
	IN PVOID	handle,
	OUT	PVOID	*ppAd);

void RTMP_AllocateTxDescMemory(
	IN	PRTMP_ADAPTER pAd,
	IN	UINT	Index,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress);

void RTMP_AllocateFirstTxBuffer(
	IN	PRTMP_ADAPTER pAd,
	IN	UINT	Index,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress);

void RTMP_FreeFirstTxBuffer(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	IN	PVOID	VirtualAddress,
	IN	NDIS_PHYSICAL_ADDRESS PhysicalAddress);

void RTMP_AllocateMgmtDescMemory(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress);

void RTMP_AllocateRxDescMemory(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress);

void RTMP_FreeDescMemory(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	PVOID	VirtualAddress,
	IN	NDIS_PHYSICAL_ADDRESS PhysicalAddress);

PNDIS_PACKET RtmpOSNetPktAlloc(
	IN RTMP_ADAPTER *pAd,
	IN int size);

PNDIS_PACKET RTMP_AllocateRxPacketBuffer(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS PhysicalAddress);

PNDIS_PACKET RTMP_AllocateTxPacketBuffer(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress);

PNDIS_PACKET RTMP_AllocateFragPacketBuffer(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length);

void RTMP_QueryPacketInfo(
	IN  PNDIS_PACKET pPacket,
	OUT PACKET_INFO  *pPacketInfo,
	OUT PUCHAR		 *pSrcBufVA,
	OUT	UINT		 *pSrcBufLen);

void RTMP_QueryNextPacketInfo(
	IN  PNDIS_PACKET *ppPacket,
	OUT PACKET_INFO  *pPacketInfo,
	OUT PUCHAR		 *pSrcBufVA,
	OUT	UINT		 *pSrcBufLen);


BOOLEAN RTMP_FillTxBlkInfo(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk);


PRTMP_SCATTER_GATHER_LIST
rt_get_sg_list_from_packet(PNDIS_PACKET pPacket, RTMP_SCATTER_GATHER_LIST *sg);


 void announce_802_3_packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket);


UINT BA_Reorder_AMSDU_Annnounce(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket);


UINT Handle_AMSDU_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN  UCHAR			FromWhichBSSID);


void convert_802_11_to_802_3_packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	PUCHAR			p8023hdr,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN  UCHAR			FromWhichBSSID);


PNET_DEV get_netdev_from_bssid(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			FromWhichBSSID);


PNDIS_PACKET duplicate_pkt(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pHeader802_3,
    IN  UINT            HdrLen,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN	UCHAR			FromWhichBSSID);


PNDIS_PACKET duplicate_pkt_with_TKIP_MIC(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pOldPkt);

PNDIS_PACKET duplicate_pkt_with_VLAN(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pHeader802_3,
    IN  UINT            HdrLen,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN	UCHAR			FromWhichBSSID);


UCHAR VLAN_8023_Header_Copy(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pHeader802_3,
	IN	UINT            HdrLen,
	OUT PUCHAR			pData,
	IN	UCHAR			FromWhichBSSID);

#ifdef DOT11_N_SUPPORT
void ba_flush_reordering_timeout_mpdus(
	IN PRTMP_ADAPTER	pAd,
	IN PBA_REC_ENTRY	pBAEntry,
	IN ULONG			Now32);


VOID BAOriSessionSetUp(
			IN PRTMP_ADAPTER    pAd,
			IN MAC_TABLE_ENTRY	*pEntry,
			IN UCHAR			TID,
			IN USHORT			TimeOut,
			IN ULONG			DelayTime,
			IN BOOLEAN		isForced);

VOID BASessionTearDownALL(
	IN OUT	PRTMP_ADAPTER pAd,
	IN		UCHAR Wcid);
#endif 

BOOLEAN OS_Need_Clone_Packet(void);


VOID build_tx_packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	PUCHAR	pFrame,
	IN	ULONG	FrameLen);


VOID BAOriSessionTearDown(
	IN OUT	PRTMP_ADAPTER	pAd,
	IN		UCHAR			Wcid,
	IN		UCHAR			TID,
	IN		BOOLEAN			bPassive,
	IN		BOOLEAN			bForceSend);

VOID BARecSessionTearDown(
	IN OUT	PRTMP_ADAPTER	pAd,
	IN		UCHAR			Wcid,
	IN		UCHAR			TID,
	IN		BOOLEAN			bPassive);

BOOLEAN ba_reordering_resource_init(PRTMP_ADAPTER pAd, int num);
void ba_reordering_resource_release(PRTMP_ADAPTER pAd);



#ifdef NINTENDO_AP
VOID	InitNINTENDO_TABLE(
	IN PRTMP_ADAPTER pAd);

UCHAR	CheckNINTENDO_TABLE(
	IN PRTMP_ADAPTER pAd,
	PCHAR pDS_Ssid,
	UCHAR DS_SsidLen,
	PUCHAR pDS_Addr);

UCHAR	DelNINTENDO_ENTRY(
	IN	PRTMP_ADAPTER pAd,
	UCHAR * pDS_Addr);

VOID	RTMPIoctlNintendoCapable(
	IN	PRTMP_ADAPTER	pAd,
	IN	struct iwreq	*wrq);

VOID	RTMPIoctlNintendoGetTable(
	IN	PRTMP_ADAPTER	pAd,
	IN	struct iwreq	*wrq);

VOID	RTMPIoctlNintendoSetTable(
	IN	PRTMP_ADAPTER	pAd,
	IN	struct iwreq	*wrq);

#endif 

BOOLEAN rtstrmactohex(
	IN PSTRING s1,
	IN PSTRING s2);

BOOLEAN rtstrcasecmp(
	IN PSTRING s1,
	IN PSTRING s2);

PSTRING rtstrstruncasecmp(
	IN PSTRING s1,
	IN PSTRING s2);

PSTRING rtstrstr(
	IN	const PSTRING s1,
	IN	const PSTRING s2);

PSTRING rstrtok(
	IN PSTRING s,
	IN const PSTRING ct);

int rtinet_aton(
	const PSTRING cp,
	unsigned int *addr);


INT Set_DriverVersion_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT Set_CountryRegion_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT Set_CountryRegionABand_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT Set_WirelessMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT Set_Channel_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_ShortSlot_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_TxPower_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT Set_BGProtection_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PSTRING			arg);

INT Set_TxPreamble_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PSTRING			arg);

INT Set_RTSThreshold_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PSTRING			arg);

INT Set_FragThreshold_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PSTRING			arg);

INT Set_TxBurst_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PSTRING			arg);

#ifdef AGGREGATION_SUPPORT
INT	Set_PktAggregate_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PSTRING			arg);
#endif 

#ifdef INF_AMAZON_PPA
INT	Set_INF_AMAZON_SE_PPA_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

#endif 

INT	Set_IEEE80211H_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

#ifdef DBG
INT	Set_Debug_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);
#endif

INT	Show_DescInfo_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_ResetStatCounter_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

#ifdef DOT11_N_SUPPORT
INT	Set_BASetup_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_BADecline_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_BAOriTearDown_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_BARecTearDown_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtBw_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtMcs_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtGi_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtOpMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtStbc_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtHtc_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtExtcha_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtMpduDensity_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtBaWinSize_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtRdg_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtLinkAdapt_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtAmsdu_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtAutoBa_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtProtect_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtMimoPs_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);


INT	Set_ForceShortGI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_ForceGF_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	SetCommonHT(
	IN	PRTMP_ADAPTER	pAd);

INT	Set_SendPSMPAction_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtMIMOPSmode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);


INT	Set_HtTxBASize_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_HtDisallowTKIP_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

#endif 



#ifdef CONFIG_STA_SUPPORT

VOID RTMPSendDLSTearDownFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pDA);

#ifdef DOT11_N_SUPPORT

VOID QueryBATABLE(
	IN  PRTMP_ADAPTER pAd,
	OUT PQUERYBA_TABLE pBAT);
#endif 

#ifdef WPA_SUPPLICANT_SUPPORT
INT	    WpaCheckEapCode(
	IN  PRTMP_ADAPTER	pAd,
	IN  PUCHAR				pFrame,
	IN  USHORT				FrameLen,
	IN  USHORT				OffSet);

VOID    WpaSendMicFailureToWpaSupplicant(
    IN  PRTMP_ADAPTER       pAd,
    IN  BOOLEAN             bUnicast);

VOID    SendAssocIEsToWpaSupplicant(
    IN  PRTMP_ADAPTER       pAd);
#endif 

#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
int wext_notify_event_assoc(
	IN  RTMP_ADAPTER *pAd);
#endif 

#endif 



#ifdef DOT11_N_SUPPORT
VOID Handle_BSS_Width_Trigger_Events(
	IN PRTMP_ADAPTER pAd);

void build_ext_channel_switch_ie(
	IN PRTMP_ADAPTER pAd,
	IN HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE *pIE);
#endif 


BOOLEAN APRxDoneInterruptHandle(
	IN	PRTMP_ADAPTER	pAd);

BOOLEAN STARxDoneInterruptHandle(
	IN	PRTMP_ADAPTER	pAd,
	IN	BOOLEAN			argc);

#ifdef DOT11_N_SUPPORT

VOID Indicate_AMPDU_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);


VOID Indicate_AMSDU_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);
#endif 


VOID Indicate_Legacy_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

VOID Indicate_EAPOL_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

void  update_os_packet_info(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

void wlan_802_11_to_802_3_packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	PUCHAR			pHeader802_3,
	IN  UCHAR			FromWhichBSSID);

UINT deaggregate_AMSDU_announce(
	IN	PRTMP_ADAPTER	pAd,
	PNDIS_PACKET		pPacket,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize);


#ifdef CONFIG_STA_SUPPORT

#define  RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(_pRxBlk, _pHeader802_3)	\
{																				\
	PUCHAR _pRemovedLLCSNAP = NULL, _pDA, _pSA;                                 \
																				\
	if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_MESH))                                    \
	{                                                                           \
		_pDA = _pRxBlk->pHeader->Addr3;                                         \
		_pSA = (PUCHAR)_pRxBlk->pHeader + sizeof(HEADER_802_11);                \
	}                                                                           \
	else                                                                        \
	{                                                                           \
		if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_INFRA))				\
		{                                                                       \
			_pDA = _pRxBlk->pHeader->Addr1;                                     \
		if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_DLS))									\
			_pSA = _pRxBlk->pHeader->Addr2;										\
		else																	\
			_pSA = _pRxBlk->pHeader->Addr3;                                     \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			_pDA = _pRxBlk->pHeader->Addr1;                                     \
			_pSA = _pRxBlk->pHeader->Addr2;                                     \
		}                                                                       \
	}                                                                           \
																				\
	CONVERT_TO_802_3(_pHeader802_3, _pDA, _pSA, _pRxBlk->pData,				\
		_pRxBlk->DataSize, _pRemovedLLCSNAP);                                   \
}
#endif 


BOOLEAN APFowardWirelessStaToWirelessSta(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	ULONG			FromWhichBSSID);

VOID Announce_or_Forward_802_3_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	UCHAR			FromWhichBSSID);

VOID Sta_Announce_or_Forward_802_3_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	UCHAR			FromWhichBSSID);


#ifdef CONFIG_STA_SUPPORT
#define ANNOUNCE_OR_FORWARD_802_3_PACKET(_pAd, _pPacket, _FromWhichBSS)\
			Sta_Announce_or_Forward_802_3_Packet(_pAd, _pPacket, _FromWhichBSS);
			
#endif 


PNDIS_PACKET DuplicatePacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	UCHAR			FromWhichBSSID);


PNDIS_PACKET ClonePacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize);



VOID CmmRxnonRalinkFrameIndicate(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

VOID CmmRxRalinkFrameIndicate(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

VOID Update_Rssi_Sample(
	IN PRTMP_ADAPTER	pAd,
	IN RSSI_SAMPLE		*pRssi,
	IN PRXWI_STRUC		pRxWI);

PNDIS_PACKET GetPacketFromRxRing(
	IN		PRTMP_ADAPTER	pAd,
	OUT		PRT28XX_RXD_STRUC		pSaveRxD,
	OUT		BOOLEAN			*pbReschedule,
	IN OUT	UINT32			*pRxPending);

PNDIS_PACKET RTMPDeFragmentDataFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk);



VOID RTMPIoctlGetSiteSurvey(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq);





#ifdef SNMP_SUPPORT

typedef struct _DefaultKeyIdxValue
{
	UCHAR	KeyIdx;
	UCHAR	Value[16];
} DefaultKeyIdxValue, *PDefaultKeyIdxValue;
#endif


#ifdef CONFIG_STA_SUPPORT
enum {
	DIDmsg_lnxind_wlansniffrm		= 0x00000044,
	DIDmsg_lnxind_wlansniffrm_hosttime	= 0x00010044,
	DIDmsg_lnxind_wlansniffrm_mactime	= 0x00020044,
	DIDmsg_lnxind_wlansniffrm_channel	= 0x00030044,
	DIDmsg_lnxind_wlansniffrm_rssi		= 0x00040044,
	DIDmsg_lnxind_wlansniffrm_sq		= 0x00050044,
	DIDmsg_lnxind_wlansniffrm_signal	= 0x00060044,
	DIDmsg_lnxind_wlansniffrm_noise		= 0x00070044,
	DIDmsg_lnxind_wlansniffrm_rate		= 0x00080044,
	DIDmsg_lnxind_wlansniffrm_istx		= 0x00090044,
	DIDmsg_lnxind_wlansniffrm_frmlen	= 0x000A0044
};
enum {
	P80211ENUM_msgitem_status_no_value	= 0x00
};
enum {
	P80211ENUM_truth_false			= 0x00,
	P80211ENUM_truth_true			= 0x01
};


typedef struct {
        UINT32 did;
        UINT16 status;
        UINT16 len;
        UINT32 data;
} p80211item_uint32_t;

typedef struct {
        UINT32 msgcode;
        UINT32 msglen;
#define WLAN_DEVNAMELEN_MAX 16
        UINT8 devname[WLAN_DEVNAMELEN_MAX];
        p80211item_uint32_t hosttime;
        p80211item_uint32_t mactime;
        p80211item_uint32_t channel;
        p80211item_uint32_t rssi;
        p80211item_uint32_t sq;
        p80211item_uint32_t signal;
        p80211item_uint32_t noise;
        p80211item_uint32_t rate;
        p80211item_uint32_t istx;
        p80211item_uint32_t frmlen;
} wlan_ng_prism2_header;


typedef struct PACKED _ieee80211_radiotap_header {
    UINT8	it_version;	
    UINT8	it_pad;
    UINT16     it_len;         
    UINT32   it_present;	
}ieee80211_radiotap_header ;

enum ieee80211_radiotap_type {
    IEEE80211_RADIOTAP_TSFT = 0,
    IEEE80211_RADIOTAP_FLAGS = 1,
    IEEE80211_RADIOTAP_RATE = 2,
    IEEE80211_RADIOTAP_CHANNEL = 3,
    IEEE80211_RADIOTAP_FHSS = 4,
    IEEE80211_RADIOTAP_DBM_ANTSIGNAL = 5,
    IEEE80211_RADIOTAP_DBM_ANTNOISE = 6,
    IEEE80211_RADIOTAP_LOCK_QUALITY = 7,
    IEEE80211_RADIOTAP_TX_ATTENUATION = 8,
    IEEE80211_RADIOTAP_DB_TX_ATTENUATION = 9,
    IEEE80211_RADIOTAP_DBM_TX_POWER = 10,
    IEEE80211_RADIOTAP_ANTENNA = 11,
    IEEE80211_RADIOTAP_DB_ANTSIGNAL = 12,
    IEEE80211_RADIOTAP_DB_ANTNOISE = 13
};

#define WLAN_RADIOTAP_PRESENT (			\
	(1 << IEEE80211_RADIOTAP_TSFT)	|	\
	(1 << IEEE80211_RADIOTAP_FLAGS) |	\
	(1 << IEEE80211_RADIOTAP_RATE)  |	\
	 0)

typedef struct _wlan_radiotap_header {
	ieee80211_radiotap_header wt_ihdr;
	INT64 wt_tsft;
	UINT8 wt_flags;
	UINT8 wt_rate;
} wlan_radiotap_header;


void send_monitor_packets(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk);


VOID    RTMPSetDesiredRates(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  LONG            Rates);
#endif 

INT	Set_FixedTxMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

#ifdef CONFIG_APSTA_MIXED_SUPPORT
INT	Set_OpMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);
#endif 

INT Set_LongRetryLimit_Proc(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	PSTRING			arg);

INT Set_ShortRetryLimit_Proc(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	PSTRING			arg);

BOOLEAN RT28XXChipsetCheck(
	IN void *_dev_p);


VOID RT28XXDMADisable(
	IN RTMP_ADAPTER			*pAd);

VOID RT28XXDMAEnable(
	IN RTMP_ADAPTER			*pAd);

VOID RT28xx_UpdateBeaconToAsic(
	IN RTMP_ADAPTER * pAd,
	IN INT apidx,
	IN ULONG BeaconLen,
	IN ULONG UpdatePos);

int rt28xx_init(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING pDefaultMac,
	IN PSTRING pHostName);

BOOLEAN RT28XXSecurityKeyAdd(
	IN		PRTMP_ADAPTER		pAd,
	IN		ULONG				apidx,
	IN		ULONG				KeyIdx,
	IN		MAC_TABLE_ENTRY		*pEntry);

NDIS_STATUS RtmpNetTaskInit(
	IN RTMP_ADAPTER *pAd);

VOID RtmpNetTaskExit(
	IN PRTMP_ADAPTER pAd);

NDIS_STATUS RtmpMgmtTaskInit(
	IN RTMP_ADAPTER *pAd);

VOID RtmpMgmtTaskExit(
	IN RTMP_ADAPTER *pAd);

void tbtt_tasklet(unsigned long data);


PNET_DEV RtmpPhyNetDevInit(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_OS_NETDEV_OP_HOOK *pNetHook);

BOOLEAN RtmpPhyNetDevExit(
	IN RTMP_ADAPTER *pAd,
	IN PNET_DEV net_dev);

INT RtmpRaDevCtrlInit(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_INF_TYPE infType);

BOOLEAN RtmpRaDevCtrlExit(
	IN RTMP_ADAPTER *pAd);


#ifdef RTMP_MAC_PCI



USHORT RtmpPCI_WriteTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	BOOLEAN			bIsLast,
	OUT	USHORT			*FreeNumber);

USHORT RtmpPCI_WriteSingleTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	BOOLEAN			bIsLast,
	OUT	USHORT			*FreeNumber);

USHORT RtmpPCI_WriteMultiTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	UCHAR			frameNum,
	OUT	USHORT			*FreeNumber);

USHORT	RtmpPCI_WriteFragTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	UCHAR			fragNum,
	OUT	USHORT			*FreeNumber);

USHORT RtmpPCI_WriteSubTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	BOOLEAN			bIsLast,
	OUT	USHORT			*FreeNumber);

VOID RtmpPCI_FinalWriteTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	USHORT			totalMPDUSize,
	IN	USHORT			FirstTxIdx);

VOID RtmpPCIDataLastTxIdx(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			QueIdx,
	IN	USHORT			LastTxIdx);

VOID RtmpPCIDataKickOut(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	UCHAR			QueIdx);


int RtmpPCIMgmtKickOut(
	IN RTMP_ADAPTER		*pAd,
	IN UCHAR			QueIdx,
	IN PNDIS_PACKET		pPacket,
	IN PUCHAR			pSrcBufVA,
	IN UINT				SrcBufLen);


NDIS_STATUS RTMPCheckRxError(
	IN  PRTMP_ADAPTER   pAd,
	IN  PHEADER_802_11  pHeader,
	IN	PRXWI_STRUC	pRxWI,
	IN  PRT28XX_RXD_STRUC      pRxD);

BOOLEAN RT28xxPciAsicRadioOff(
	IN PRTMP_ADAPTER    pAd,
	IN UCHAR            Level,
	IN USHORT           TbttNumToNextWakeUp);

BOOLEAN RT28xxPciAsicRadioOn(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR     Level);

#ifdef CONFIG_STA_SUPPORT
VOID RTMPInitPCIeLinkCtrlValue(
	IN	PRTMP_ADAPTER	pAd);

VOID RTMPFindHostPCIDev(
    IN	PRTMP_ADAPTER	pAd);

VOID RTMPPCIeLinkCtrlValueRestore(
	IN	PRTMP_ADAPTER	pAd,
	IN   UCHAR		Level);

VOID RTMPPCIeLinkCtrlSetting(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT		Max);

VOID RTMPrt3xSetPCIePowerLinkCtrl(
	IN	PRTMP_ADAPTER	pAd);


VOID RT28xxPciStaAsicForceWakeup(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN       bFromTx);

VOID RT28xxPciStaAsicSleepThenAutoWakeup(
	IN PRTMP_ADAPTER pAd,
	IN USHORT TbttNumToNextWakeUp);

VOID PsPollWakeExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID  RadioOnExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
#endif 

VOID RT28xxPciMlmeRadioOn(
	IN PRTMP_ADAPTER pAd);

VOID RT28xxPciMlmeRadioOFF(
	IN PRTMP_ADAPTER pAd);
#endif 

VOID AsicTurnOffRFClk(
	IN PRTMP_ADAPTER    pAd,
	IN	UCHAR           Channel);

VOID AsicTurnOnRFClk(
	IN PRTMP_ADAPTER	pAd,
	IN	UCHAR			Channel);



#ifdef RTMP_TIMER_TASK_SUPPORT
INT RtmpTimerQThread(
	IN OUT PVOID Context);

RTMP_TIMER_TASK_ENTRY *RtmpTimerQInsert(
	IN RTMP_ADAPTER *pAd,
	IN RALINK_TIMER_STRUCT *pTimer);

BOOLEAN RtmpTimerQRemove(
	IN RTMP_ADAPTER *pAd,
	IN RALINK_TIMER_STRUCT *pTimer);

void RtmpTimerQExit(
	IN RTMP_ADAPTER *pAd);

void RtmpTimerQInit(
	IN RTMP_ADAPTER *pAd);
#endif 





VOID QBSS_LoadInit(
	IN		RTMP_ADAPTER	*pAd);

UINT32 QBSS_LoadElementAppend(
	IN		RTMP_ADAPTER	*pAd,
	OUT		UINT8			*buf_p);

VOID QBSS_LoadUpdate(
	IN		RTMP_ADAPTER	*pAd);


INT RTMPShowCfgValue(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			pName,
	IN	PSTRING			pBuf);

PSTRING RTMPGetRalinkAuthModeStr(
    IN  NDIS_802_11_AUTHENTICATION_MODE authMode);

PSTRING RTMPGetRalinkEncryModeStr(
    IN  USHORT encryMode);


#ifdef CONFIG_STA_SUPPORT
VOID AsicStaBbpTuning(
	IN PRTMP_ADAPTER pAd);

BOOLEAN StaAddMacTableEntry(
	IN  PRTMP_ADAPTER		pAd,
	IN  PMAC_TABLE_ENTRY	pEntry,
	IN  UCHAR				MaxSupportedRateIn500Kbps,
	IN  HT_CAPABILITY_IE	*pHtCapability,
	IN  UCHAR				HtCapabilityLen,
	IN  ADD_HT_INFO_IE		*pAddHtInfo,
	IN  UCHAR				AddHtInfoLen,
	IN  USHORT			CapabilityInfo);


BOOLEAN	AUTH_ReqSend(
	IN  PRTMP_ADAPTER		pAd,
	IN  PMLME_QUEUE_ELEM	pElem,
	IN  PRALINK_TIMER_STRUCT pAuthTimer,
	IN  PSTRING				pSMName,
	IN  USHORT				SeqNo,
	IN  PUCHAR				pNewElement,
	IN  ULONG				ElementLen);
#endif 

void RTMP_IndicateMediaState(
	IN	PRTMP_ADAPTER	pAd);

VOID ReSyncBeaconTime(
	IN  PRTMP_ADAPTER   pAd);

VOID RTMPSetAGCInitValue(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			BandWidth);

int rt28xx_close(IN PNET_DEV dev);
int rt28xx_open(IN PNET_DEV dev);


#define VIRTUAL_IF_INC(__pAd) ((__pAd)->VirtualIfCnt++)
#define VIRTUAL_IF_DEC(__pAd) ((__pAd)->VirtualIfCnt--)
#define VIRTUAL_IF_NUM(__pAd) ((__pAd)->VirtualIfCnt)


#ifdef LINUX
__inline INT VIRTUAL_IF_UP(PRTMP_ADAPTER pAd)
{
	if (VIRTUAL_IF_NUM(pAd) == 0)
	{
		if (rt28xx_open(pAd->net_dev) != 0)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("rt28xx_open return fail!\n"));
			return -1;
		}
	}
	else
	{
	}
	VIRTUAL_IF_INC(pAd);
	return 0;
}

__inline VOID VIRTUAL_IF_DOWN(PRTMP_ADAPTER pAd)
{
	VIRTUAL_IF_DEC(pAd);
	if (VIRTUAL_IF_NUM(pAd) == 0)
		rt28xx_close(pAd->net_dev);
	return;
}
#endif 





int RtmpOSWrielessEventSend(
	IN RTMP_ADAPTER *pAd,
	IN UINT32		eventType,
	IN INT			flags,
	IN PUCHAR		pSrcMac,
	IN PUCHAR		pData,
	IN UINT32		dataLen);

int RtmpOSNetDevAddrSet(
	IN PNET_DEV pNetDev,
	IN PUCHAR	pMacAddr);

int RtmpOSNetDevAttach(
	IN PNET_DEV pNetDev,
	IN RTMP_OS_NETDEV_OP_HOOK *pDevOpHook);

void RtmpOSNetDevClose(
	IN PNET_DEV pNetDev);

void RtmpOSNetDevDetach(
	IN PNET_DEV pNetDev);

INT RtmpOSNetDevAlloc(
	IN PNET_DEV *pNewNetDev,
	IN UINT32	privDataSize);

void RtmpOSNetDevFree(
	IN PNET_DEV pNetDev);

PNET_DEV RtmpOSNetDevGetByName(
	IN PNET_DEV pNetDev,
	IN PSTRING	pDevName);

void RtmpOSNetDeviceRefPut(
	IN PNET_DEV pNetDev);

PNET_DEV RtmpOSNetDevCreate(
	IN RTMP_ADAPTER *pAd,
	IN INT			devType,
	IN INT			devNum,
	IN INT			privMemSize,
	IN PSTRING		pNamePrefix);


void RtmpOSTaskCustomize(
	IN RTMP_OS_TASK *pTask);

INT RtmpOSTaskNotifyToExit(
	IN RTMP_OS_TASK *pTask);

NDIS_STATUS RtmpOSTaskKill(
	IN RTMP_OS_TASK *pTask);

NDIS_STATUS RtmpOSTaskInit(
	IN RTMP_OS_TASK *pTask,
	PSTRING			 pTaskName,
	VOID			 *pPriv);

NDIS_STATUS RtmpOSTaskAttach(
	IN RTMP_OS_TASK *pTask,
	IN int (*fn)(void *),
	IN void *arg);



RTMP_OS_FD RtmpOSFileOpen(
	IN char *pPath,
	IN int flag,
	IN int mode);

int RtmpOSFileClose(
	IN RTMP_OS_FD osfd);

void RtmpOSFileSeek(
	IN RTMP_OS_FD osfd,
	IN int offset);

int RtmpOSFileRead(
	IN RTMP_OS_FD osfd,
	IN char *pDataPtr,
	IN int readLen);

int RtmpOSFileWrite(
	IN RTMP_OS_FD osfd,
	IN char *pDataPtr,
	IN int writeLen);

void RtmpOSFSInfoChange(
	IN RTMP_OS_FS_INFO *pOSFSInfo,
	IN BOOLEAN bSet);


#endif  
