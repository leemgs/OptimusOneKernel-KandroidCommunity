
#ifndef __RTMP_H__
#define __RTMP_H__

#include "spectrum_def.h"

#include "aironet.h"

#define VIRTUAL_IF_INC(__pAd) ((__pAd)->VirtualIfCnt++)
#define VIRTUAL_IF_DEC(__pAd) ((__pAd)->VirtualIfCnt--)
#define VIRTUAL_IF_NUM(__pAd) ((__pAd)->VirtualIfCnt)

#ifdef RT2870



typedef struct __TX_BUFFER{
	union	{
		UCHAR			WirelessPacket[TX_BUFFER_NORMSIZE];
		HEADER_802_11	NullFrame;
		PSPOLL_FRAME	PsPollPacket;
		RTS_FRAME		RTSFrame;
	}field;
	UCHAR			Aggregation[4];  
} TX_BUFFER, *PTX_BUFFER;

typedef struct __HTTX_BUFFER{
	union	{
		UCHAR			WirelessPacket[MAX_TXBULK_SIZE];
		HEADER_802_11	NullFrame;
		PSPOLL_FRAME	PsPollPacket;
		RTS_FRAME		RTSFrame;
	}field;
	UCHAR			Aggregation[4];  
} HTTX_BUFFER, *PHTTX_BUFFER;



typedef struct _TX_CONTEXT
{
	PVOID			pAd;		
	PURB			pUrb;			
	PIRP			pIrp;			
									
	PTX_BUFFER		TransferBuffer;	
	ULONG			BulkOutSize;
	UCHAR			BulkOutPipeId;
	UCHAR			SelfIdx;
	BOOLEAN			InUse;
	BOOLEAN			bWaitingBulkOut; 
	BOOLEAN			bFullForBulkOut; 
	BOOLEAN			IRPPending;
	BOOLEAN			LastOne;
	BOOLEAN			bAggregatible;
	UCHAR			Header_802_3[LENGTH_802_3];
	UCHAR			Rsv[2];
	ULONG			DataOffset;
	UINT			TxRate;
	dma_addr_t		data_dma;		

}	TX_CONTEXT, *PTX_CONTEXT, **PPTX_CONTEXT;



typedef struct _HT_TX_CONTEXT
{
	PVOID			pAd;		
	PURB			pUrb;			
	PIRP			pIrp;			
									
	PHTTX_BUFFER	TransferBuffer;	
	ULONG			BulkOutSize;	
	UCHAR			BulkOutPipeId;
	BOOLEAN			IRPPending;
	BOOLEAN			LastOne;
	BOOLEAN			bCurWriting;
	BOOLEAN			bRingEmpty;
	BOOLEAN			bCopySavePad;
	UCHAR			SavedPad[8];
	UCHAR			Header_802_3[LENGTH_802_3];
	ULONG			CurWritePosition;		
	ULONG			CurWriteRealPos;		
	ULONG			NextBulkOutPosition;	
	ULONG			ENextBulkOutPosition;	
	UINT			TxRate;
	dma_addr_t		data_dma;		
}	HT_TX_CONTEXT, *PHT_TX_CONTEXT, **PPHT_TX_CONTEXT;






typedef struct _RX_CONTEXT
{
	PUCHAR				TransferBuffer;
	PVOID				pAd;
	PIRP				pIrp;
	PURB				pUrb;
	
	ULONG				BulkInOffset;	
	BOOLEAN				bRxHandling;	
	BOOLEAN				InUse;			
	BOOLEAN				Readable;		
	BOOLEAN				IRPPending;		
	atomic_t			IrpLock;
	NDIS_SPIN_LOCK		RxContextLock;
	dma_addr_t			data_dma;		
}	RX_CONTEXT, *PRX_CONTEXT;
#endif 





#ifdef  NDIS50_MINIPORT
#define RTMP_NDIS_MAJOR_VERSION     5
#define RTMP_NDIS_MINOR_VERSION     0
#endif

#ifdef  NDIS51_MINIPORT
#define RTMP_NDIS_MAJOR_VERSION     5
#define RTMP_NDIS_MINOR_VERSION     1
#endif

extern  char    NIC_VENDOR_DESC[];
extern  int     NIC_VENDOR_DESC_LEN;

extern  unsigned char   SNAP_AIRONET[];
extern  unsigned char   CipherSuiteCiscoCCKM[];
extern  unsigned char   CipherSuiteCiscoCCKMLen;
extern	unsigned char	CipherSuiteCiscoCCKM24[];
extern	unsigned char	CipherSuiteCiscoCCKM24Len;
extern  unsigned char   CipherSuiteCCXTkip[];
extern  unsigned char   CipherSuiteCCXTkipLen;
extern  unsigned char   CISCO_OUI[];
extern  UCHAR	BaSizeArray[4];

extern UCHAR BROADCAST_ADDR[MAC_ADDR_LEN];
extern UCHAR MULTICAST_ADDR[MAC_ADDR_LEN];
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

extern UCHAR  HtCapIe;
extern UCHAR  AddHtInfoIe;
extern UCHAR  NewExtChanIe;

extern UCHAR  ErpIe;
extern UCHAR  DsIe;
extern UCHAR  TimIe;
extern UCHAR  WpaIe;
extern UCHAR  Wpa2Ie;
extern UCHAR  IbssIe;
extern UCHAR  Ccx2Ie;

extern UCHAR  WPA_OUI[];
extern UCHAR  RSN_OUI[];
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

extern UCHAR  RateSwitchTable11BGN1S[];
extern UCHAR  RateSwitchTable11BGN2S[];
extern UCHAR  RateSwitchTable11BGN2SForABand[];
extern UCHAR  RateSwitchTable11N1S[];
extern UCHAR  RateSwitchTable11N2S[];
extern UCHAR  RateSwitchTable11N2SForABand[];

extern UCHAR  PRE_N_HT_OUI[];

#define	MAXSEQ		(0xFFF)

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
	int 	qlen;
};

struct reordering_mpdu_pool
{
	PVOID					mem;
	NDIS_SPIN_LOCK			lock;
	struct reordering_list 	freelist;
};

typedef struct 	_RSSI_SAMPLE {
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

#define InsertTailQueue(QueueHeader, QueueEntry)                \
{                                                               \
	((PQUEUE_ENTRY)QueueEntry)->Next = NULL;                    \
	if ((QueueHeader)->Tail)                                    \
		(QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry); \
	else                                                        \
		(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
	(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);           \
	(QueueHeader)->Number++;                                    \
}




#define RTMP_SET_FLAG(_M, _F)       ((_M)->Flags |= (_F))
#define RTMP_CLEAR_FLAG(_M, _F)     ((_M)->Flags &= ~(_F))
#define RTMP_CLEAR_FLAGS(_M)        ((_M)->Flags = 0)
#define RTMP_TEST_FLAG(_M, _F)      (((_M)->Flags & (_F)) != 0)
#define RTMP_TEST_FLAGS(_M, _F)     (((_M)->Flags & (_F)) == (_F))

#ifdef RT2860

#define RTMP_SET_PSFLAG(_M, _F)       ((_M)->PSFlags |= (_F))
#define RTMP_CLEAR_PSFLAG(_M, _F)     ((_M)->PSFlags &= ~(_F))
#define RTMP_CLEAR_PSFLAGS(_M)        ((_M)->PSFlags = 0)
#define RTMP_TEST_PSFLAG(_M, _F)      (((_M)->PSFlags & (_F)) != 0)
#define RTMP_TEST_PSFLAGS(_M, _F)     (((_M)->PSFlags & (_F)) == (_F))
#endif

#define OPSTATUS_SET_FLAG(_pAd, _F)     ((_pAd)->CommonCfg.OpStatusFlags |= (_F))
#define OPSTATUS_CLEAR_FLAG(_pAd, _F)   ((_pAd)->CommonCfg.OpStatusFlags &= ~(_F))
#define OPSTATUS_TEST_FLAG(_pAd, _F)    (((_pAd)->CommonCfg.OpStatusFlags & (_F)) != 0)

#define CLIENT_STATUS_SET_FLAG(_pEntry,_F)      ((_pEntry)->ClientStatusFlags |= (_F))
#define CLIENT_STATUS_CLEAR_FLAG(_pEntry,_F)    ((_pEntry)->ClientStatusFlags &= ~(_F))
#define CLIENT_STATUS_TEST_FLAG(_pEntry,_F)     (((_pEntry)->ClientStatusFlags & (_F)) != 0)

#define RX_FILTER_SET_FLAG(_pAd, _F)    ((_pAd)->CommonCfg.PacketFilter |= (_F))
#define RX_FILTER_CLEAR_FLAG(_pAd, _F)  ((_pAd)->CommonCfg.PacketFilter &= ~(_F))
#define RX_FILTER_TEST_FLAG(_pAd, _F)   (((_pAd)->CommonCfg.PacketFilter & (_F)) != 0)

#define STA_NO_SECURITY_ON(_p)          (_p->StaCfg.WepStatus == Ndis802_11EncryptionDisabled)
#define STA_WEP_ON(_p)                  (_p->StaCfg.WepStatus == Ndis802_11Encryption1Enabled)
#define STA_TKIP_ON(_p)                 (_p->StaCfg.WepStatus == Ndis802_11Encryption2Enabled)
#define STA_AES_ON(_p)                  (_p->StaCfg.WepStatus == Ndis802_11Encryption3Enabled)

#define STA_TGN_WIFI_ON(_p)             (_p->StaCfg.bTGnWifiTest == TRUE)

#define CKIP_KP_ON(_p)				((((_p)->StaCfg.CkipFlag) & 0x10) && ((_p)->StaCfg.bCkipCmicOn == TRUE))
#define CKIP_CMIC_ON(_p)			((((_p)->StaCfg.CkipFlag) & 0x08) && ((_p)->StaCfg.bCkipCmicOn == TRUE))


#define INC_RING_INDEX(_idx, _RingSize)    \
{                                          \
    (_idx) = (_idx+1) % (_RingSize);       \
}

#ifdef RT2870

#define IS_RT3090(_pAd)				((((_pAd)->MACVersion & 0xffff0000) == 0x30710000) || (((_pAd)->MACVersion & 0xffff0000) == 0x30900000))
#else
#define IS_RT3090(_pAd)				0
#endif
#define IS_RT3070(_pAd)				(((_pAd)->MACVersion & 0xffff0000) == 0x30700000)
#ifdef RT2870
#define IS_RT3071(_pAd)				(((_pAd)->MACVersion & 0xffff0000) == 0x30710000)
#define IS_RT30xx(_pAd)				(((_pAd)->MACVersion & 0xfff00000) == 0x30700000)
#endif

#define RING_PACKET_INIT(_TxRing, _idx)    \
{                                          \
    _TxRing->Cell[_idx].pNdisPacket = NULL;                              \
    _TxRing->Cell[_idx].pNextNdisPacket = NULL;                              \
}

#define TXDT_INIT(_TxD)    \
{                                          \
	NdisZeroMemory(_TxD, TXD_SIZE);	\
	_TxD->DMADONE = 1;                              \
}


#define RING_SET_LASTDS(_TxD, _IsSD0)    \
{                                          \
    if (_IsSD0) {_TxD->LastSec0 = 1;}     \
    else {_TxD->LastSec1 = 1;}     \
}





#define INC_TX_TSC(_tsc)                                \
{                                                       \
    int i=0;                                            \
    while (++_tsc[i] == 0x0)                            \
    {                                                   \
        i++;                                            \
        if (i == 6)                                     \
            break;                                      \
    }                                                   \
}


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



















#define MAX_BUSY_COUNT  100         

#ifdef RT2860
#define RTMP_RF_IO_WRITE32(_A, _V)                  \
{                                                   \
    PHY_CSR4_STRUC  Value;                          \
    ULONG           BusyCnt = 0;                    \
    if ((_A)->bPCIclkOff) 	                \
    {												\
        return;										\
    }                                               \
    do {                                            \
        RTMP_IO_READ32(_A, RF_CSR_CFG0, &Value.word);  \
        if (Value.field.Busy == IDLE)               \
            break;                                  \
        BusyCnt++;                                  \
    }   while (BusyCnt < MAX_BUSY_COUNT);           \
    if (BusyCnt < MAX_BUSY_COUNT)                   \
    {                                               \
        RTMP_IO_WRITE32(_A, RF_CSR_CFG0, _V);          \
    }                                               \
}

#define BBP_IO_READ8_BY_REG_ID(_A, _I, _pV)        \
{                                                       \
    BBP_CSR_CFG_STRUC  BbpCsr;                             \
    int             i, k;                               \
    for (i=0; i<MAX_BUSY_COUNT; i++)                    \
    {                                                   \
        RTMP_IO_READ32(_A, BBP_CSR_CFG, &BbpCsr.word);     \
        if (BbpCsr.field.Busy == BUSY)                  \
        {                                               \
            continue;                                   \
        }                                               \
        BbpCsr.word = 0;                                \
        BbpCsr.field.fRead = 1;                         \
        BbpCsr.field.BBP_RW_MODE = 1;                         \
        BbpCsr.field.Busy = 1;                          \
        BbpCsr.field.RegNum = _I;                       \
        RTMP_IO_WRITE32(_A, BBP_CSR_CFG, BbpCsr.word);     \
        for (k=0; k<MAX_BUSY_COUNT; k++)                \
        {                                               \
            RTMP_IO_READ32(_A, BBP_CSR_CFG, &BbpCsr.word); \
            if (BbpCsr.field.Busy == IDLE)              \
                break;                                  \
        }                                               \
        if ((BbpCsr.field.Busy == IDLE) &&              \
            (BbpCsr.field.RegNum == _I))                \
        {                                               \
            *(_pV) = (UCHAR)BbpCsr.field.Value;         \
            break;                                      \
        }                                               \
    }                                                   \
    if (BbpCsr.field.Busy == BUSY)                      \
    {                                                   \
        DBGPRINT_ERR(("DFS BBP read R%d fail\n", _I));      \
        *(_pV) = (_A)->BbpWriteLatch[_I];               \
    }                                                   \
}



#define RTMP_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV)        \
{                                                       \
    BBP_CSR_CFG_STRUC  BbpCsr;                             \
    int             i, k;                               \
    if ((_A)->bPCIclkOff == FALSE)                     \
    {                                                   \
    for (i=0; i<MAX_BUSY_COUNT; i++)                    \
    {                                                   \
		RTMP_IO_READ32(_A, H2M_BBP_AGENT, &BbpCsr.word);				\
        if (BbpCsr.field.Busy == BUSY)                  \
        {                                               \
            continue;                                   \
        }                                               \
        BbpCsr.word = 0;                                \
        BbpCsr.field.fRead = 1;                         \
        BbpCsr.field.BBP_RW_MODE = 1;                         \
        BbpCsr.field.Busy = 1;                          \
        BbpCsr.field.RegNum = _I;                       \
		RTMP_IO_WRITE32(_A, H2M_BBP_AGENT, BbpCsr.word);				\
		AsicSendCommandToMcu(_A, 0x80, 0xff, 0x0, 0x0);					\
		RTMPusecDelay(1000);							\
        for (k=0; k<MAX_BUSY_COUNT; k++)                \
        {                                               \
			RTMP_IO_READ32(_A, H2M_BBP_AGENT, &BbpCsr.word);			\
            if (BbpCsr.field.Busy == IDLE)              \
                break;                                  \
        }                                               \
        if ((BbpCsr.field.Busy == IDLE) &&              \
            (BbpCsr.field.RegNum == _I))                \
        {                                               \
            *(_pV) = (UCHAR)BbpCsr.field.Value;         \
            break;                                      \
        }                                               \
    }                                                   \
    if (BbpCsr.field.Busy == BUSY)                      \
    {                                                   \
		DBGPRINT_ERR(("BBP read R%d=0x%x fail\n", _I, BbpCsr.word));	\
        *(_pV) = (_A)->BbpWriteLatch[_I];               \
		RTMP_IO_READ32(_A, H2M_BBP_AGENT, &BbpCsr.word);				\
		BbpCsr.field.Busy = 0;                          \
		RTMP_IO_WRITE32(_A, H2M_BBP_AGENT, BbpCsr.word);				\
    }                                                   \
    }                   \
}

#define BBP_IO_WRITE8_BY_REG_ID(_A, _I, _V)        \
{                                                       \
    BBP_CSR_CFG_STRUC  BbpCsr;                             \
    int             BusyCnt;                            \
    for (BusyCnt=0; BusyCnt<MAX_BUSY_COUNT; BusyCnt++)  \
    {                                                   \
        RTMP_IO_READ32(_A, BBP_CSR_CFG, &BbpCsr.word);     \
        if (BbpCsr.field.Busy == BUSY)                  \
            continue;                                   \
        BbpCsr.word = 0;                                \
        BbpCsr.field.fRead = 0;                         \
        BbpCsr.field.BBP_RW_MODE = 1;                         \
        BbpCsr.field.Busy = 1;                          \
        BbpCsr.field.Value = _V;                        \
        BbpCsr.field.RegNum = _I;                       \
        RTMP_IO_WRITE32(_A, BBP_CSR_CFG, BbpCsr.word);     \
        (_A)->BbpWriteLatch[_I] = _V;                   \
        break;                                          \
    }                                                   \
    if (BusyCnt == MAX_BUSY_COUNT)                      \
    {                                                   \
        DBGPRINT_ERR(("BBP write R%d fail\n", _I));     \
    }                                                   \
}


#define RTMP_BBP_IO_WRITE8_BY_REG_ID(_A, _I, _V)        \
{                                                       \
    BBP_CSR_CFG_STRUC  BbpCsr;                             \
    int             BusyCnt;                            \
    if ((_A)->bPCIclkOff == FALSE)                     \
    {                                                   \
    for (BusyCnt=0; BusyCnt<MAX_BUSY_COUNT; BusyCnt++)  \
    {                                                   \
		RTMP_IO_READ32(_A, H2M_BBP_AGENT, &BbpCsr.word);				\
        if (BbpCsr.field.Busy == BUSY)                  \
            continue;                                   \
        BbpCsr.word = 0;                                \
        BbpCsr.field.fRead = 0;                         \
        BbpCsr.field.BBP_RW_MODE = 1;                         \
        BbpCsr.field.Busy = 1;                          \
        BbpCsr.field.Value = _V;                        \
        BbpCsr.field.RegNum = _I;                       \
		RTMP_IO_WRITE32(_A, H2M_BBP_AGENT, BbpCsr.word);				\
		AsicSendCommandToMcu(_A, 0x80, 0xff, 0x0, 0x0);					\
            if (_A->OpMode == OPMODE_AP)                    \
		RTMPusecDelay(1000);							\
        (_A)->BbpWriteLatch[_I] = _V;                   \
        break;                                          \
    }                                                   \
    if (BusyCnt == MAX_BUSY_COUNT)                      \
    {                                                   \
		DBGPRINT_ERR(("BBP write R%d=0x%x fail\n", _I, BbpCsr.word));	\
		RTMP_IO_READ32(_A, H2M_BBP_AGENT, &BbpCsr.word);				\
		BbpCsr.field.Busy = 0;                          \
		RTMP_IO_WRITE32(_A, H2M_BBP_AGENT, BbpCsr.word);				\
    }                                                   \
    }                                                   \
}
#endif 
#ifdef RT2870
#define RTMP_RF_IO_WRITE32(_A, _V)                 RTUSBWriteRFRegister(_A, _V)
#define RTMP_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV)   RTUSBReadBBPRegister(_A, _I, _pV)
#define RTMP_BBP_IO_WRITE8_BY_REG_ID(_A, _I, _V)   RTUSBWriteBBPRegister(_A, _I, _V)

#define BBP_IO_WRITE8_BY_REG_ID(_A, _I, _V)			RTUSBWriteBBPRegister(_A, _I, _V)
#define BBP_IO_READ8_BY_REG_ID(_A, _I, _pV)   		RTUSBReadBBPRegister(_A, _I, _pV)
#endif 

#define     MAP_CHANNEL_ID_TO_KHZ(ch, khz)  {               \
                switch (ch)                                 \
                {                                           \
                    case 1:     khz = 2412000;   break;     \
                    case 2:     khz = 2417000;   break;     \
                    case 3:     khz = 2422000;   break;     \
                    case 4:     khz = 2427000;   break;     \
                    case 5:     khz = 2432000;   break;     \
                    case 6:     khz = 2437000;   break;     \
                    case 7:     khz = 2442000;   break;     \
                    case 8:     khz = 2447000;   break;     \
                    case 9:     khz = 2452000;   break;     \
                    case 10:    khz = 2457000;   break;     \
                    case 11:    khz = 2462000;   break;     \
                    case 12:    khz = 2467000;   break;     \
                    case 13:    khz = 2472000;   break;     \
                    case 14:    khz = 2484000;   break;     \
                    case 36:    khz = 5180000;   break;     \
                    case 40:    khz = 5200000;   break;     \
                    case 44:    khz = 5220000;   break;     \
                    case 48:    khz = 5240000;   break;     \
                    case 52:    khz = 5260000;   break;     \
                    case 56:    khz = 5280000;   break;     \
                    case 60:    khz = 5300000;   break;     \
                    case 64:    khz = 5320000;   break;     \
                    case 149:   khz = 5745000;   break;     \
                    case 153:   khz = 5765000;   break;     \
                    case 157:   khz = 5785000;   break;     \
                    case 161:   khz = 5805000;   break;     \
                    case 165:   khz = 5825000;   break;     \
                    case 100:   khz = 5500000;   break;     \
                    case 104:   khz = 5520000;   break;     \
                    case 108:   khz = 5540000;   break;     \
                    case 112:   khz = 5560000;   break;     \
                    case 116:   khz = 5580000;   break;     \
                    case 120:   khz = 5600000;   break;     \
                    case 124:   khz = 5620000;   break;     \
                    case 128:   khz = 5640000;   break;     \
                    case 132:   khz = 5660000;   break;     \
                    case 136:   khz = 5680000;   break;     \
                    case 140:   khz = 5700000;   break;     \
                    case 34:     khz = 5170000;   break;   \
                    case 38:     khz = 5190000;   break;   \
                    case 42:     khz = 5210000;   break;   \
                    case 46:     khz = 5230000;   break;   \
                    case 184:    khz = 4920000;   break;   \
                    case 188:    khz = 4940000;   break;   \
                    case 192:    khz = 4960000;   break;   \
                    case 196:    khz = 4980000;   break;   \
                    case 208:    khz = 5040000;   break;   \
                    case 212:    khz = 5060000;   break;   \
                    case 216:    khz = 5080000;   break;   \
                    default:    khz = 2412000;   break;     \
                }                                           \
            }

#define     MAP_KHZ_TO_CHANNEL_ID(khz, ch)  {               \
                switch (khz)                                \
                {                                           \
                    case 2412000:    ch = 1;     break;     \
                    case 2417000:    ch = 2;     break;     \
                    case 2422000:    ch = 3;     break;     \
                    case 2427000:    ch = 4;     break;     \
                    case 2432000:    ch = 5;     break;     \
                    case 2437000:    ch = 6;     break;     \
                    case 2442000:    ch = 7;     break;     \
                    case 2447000:    ch = 8;     break;     \
                    case 2452000:    ch = 9;     break;     \
                    case 2457000:    ch = 10;    break;     \
                    case 2462000:    ch = 11;    break;     \
                    case 2467000:    ch = 12;    break;     \
                    case 2472000:    ch = 13;    break;     \
                    case 2484000:    ch = 14;    break;     \
                    case 5180000:    ch = 36;    break;     \
                    case 5200000:    ch = 40;    break;     \
                    case 5220000:    ch = 44;    break;     \
                    case 5240000:    ch = 48;    break;     \
                    case 5260000:    ch = 52;    break;     \
                    case 5280000:    ch = 56;    break;     \
                    case 5300000:    ch = 60;    break;     \
                    case 5320000:    ch = 64;    break;     \
                    case 5745000:    ch = 149;   break;     \
                    case 5765000:    ch = 153;   break;     \
                    case 5785000:    ch = 157;   break;     \
                    case 5805000:    ch = 161;   break;     \
                    case 5825000:    ch = 165;   break;     \
                    case 5500000:    ch = 100;   break;     \
                    case 5520000:    ch = 104;   break;     \
                    case 5540000:    ch = 108;   break;     \
                    case 5560000:    ch = 112;   break;     \
                    case 5580000:    ch = 116;   break;     \
                    case 5600000:    ch = 120;   break;     \
                    case 5620000:    ch = 124;   break;     \
                    case 5640000:    ch = 128;   break;     \
                    case 5660000:    ch = 132;   break;     \
                    case 5680000:    ch = 136;   break;     \
                    case 5700000:    ch = 140;   break;     \
                    case 5170000:    ch = 34;     break;   \
                    case 5190000:    ch = 38;     break;   \
                    case 5210000:    ch = 42;     break;   \
                    case 5230000:    ch = 46;     break;   \
                    case 4920000:    ch = 184;   break;   \
                    case 4940000:    ch = 188;   break;   \
                    case 4960000:    ch = 192;   break;   \
                    case 4980000:    ch = 196;   break;   \
                    case 5040000:    ch = 208;   break;   \
                    case 5060000:    ch = 212;   break;   \
                    case 5080000:    ch = 216;   break;   \
                    default:         ch = 1;     break;     \
                }                                           \
            }




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
		if (NdisEqualMemory(IPX, _pBufVA + 12, 2) || 			\
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
		if (NdisEqualMemory(IPX, _pBufVA, 2) || 				\
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

#define SWITCH_AB( _pAA, _pBB)    \
{                                                                           \
    PVOID pCC;                                                          \
    pCC = _pBB;                                                 \
    _pBB = _pAA;                                                 \
    _pAA = pCC;                                                 \
}




#ifdef RT2860
#define REPORT_MGMT_FRAME_TO_MLME(_pAd, Wcid, _pFrame, _FrameSize, _Rssi0, _Rssi1, _Rssi2, _PlcpSignal)        \
{                                                                                       \
    UINT32 High32TSF, Low32TSF;                                                          \
    RTMP_IO_READ32(_pAd, TSF_TIMER_DW1, &High32TSF);                                       \
    RTMP_IO_READ32(_pAd, TSF_TIMER_DW0, &Low32TSF);                                        \
    MlmeEnqueueForRecv(_pAd, Wcid, High32TSF, Low32TSF, (UCHAR)_Rssi0, (UCHAR)_Rssi1,(UCHAR)_Rssi2,_FrameSize, _pFrame, (UCHAR)_PlcpSignal);   \
}
#endif
#ifdef RT2870
#define REPORT_MGMT_FRAME_TO_MLME(_pAd, Wcid, _pFrame, _FrameSize, _Rssi0, _Rssi1, _Rssi2, _PlcpSignal)        \
{                                                                                       \
    UINT32 High32TSF=0, Low32TSF=0;                                                          \
    MlmeEnqueueForRecv(_pAd, Wcid, High32TSF, Low32TSF, (UCHAR)_Rssi0, (UCHAR)_Rssi1,(UCHAR)_Rssi2,_FrameSize, _pFrame, (UCHAR)_PlcpSignal);   \
}
#endif 



#define COLLECT_RX_ANTENNA_AVERAGE_RSSI(_pAd, _rssi1, _rssi2)					\
{																				\
	SHORT	AvgRssi;															\
	UCHAR	UsedAnt;															\
	if (_pAd->RxAnt.EvaluatePeriod == 0)									\
	{																		\
		UsedAnt = _pAd->RxAnt.Pair1PrimaryRxAnt;							\
		AvgRssi = _pAd->RxAnt.Pair1AvgRssi[UsedAnt];						\
		if (AvgRssi < 0)													\
			AvgRssi = AvgRssi - (AvgRssi >> 3) + _rssi1;					\
		else																\
			AvgRssi = _rssi1 << 3;											\
		_pAd->RxAnt.Pair1AvgRssi[UsedAnt] = AvgRssi;						\
	}																		\
	else																	\
	{																		\
		UsedAnt = _pAd->RxAnt.Pair1SecondaryRxAnt;							\
		AvgRssi = _pAd->RxAnt.Pair1AvgRssi[UsedAnt];						\
		if ((AvgRssi < 0) && (_pAd->RxAnt.FirstPktArrivedWhenEvaluate))		\
			AvgRssi = AvgRssi - (AvgRssi >> 3) + _rssi1;					\
		else																\
		{																	\
			_pAd->RxAnt.FirstPktArrivedWhenEvaluate = TRUE;					\
			AvgRssi = _rssi1 << 3;											\
		}																	\
		_pAd->RxAnt.Pair1AvgRssi[UsedAnt] = AvgRssi;						\
		_pAd->RxAnt.RcvPktNumWhenEvaluate++;								\
	}																		\
}

#define NDIS_QUERY_BUFFER(_NdisBuf, _ppVA, _pBufLen)                    \
    NdisQueryBuffer(_NdisBuf, _ppVA, _pBufLen)

#define MAC_ADDR_EQUAL(pAddr1,pAddr2)           RTMPEqualMemory((PVOID)(pAddr1), (PVOID)(pAddr2), MAC_ADDR_LEN)
#define SSID_EQUAL(ssid1, len1, ssid2, len2)    ((len1==len2) && (RTMPEqualMemory(ssid1, ssid2, len1)))




#define JapanChannelCheck(channel)  ((channel == 52) || (channel == 56) || (channel == 60) || (channel == 64))

#ifdef RT2860
#define STA_PORT_SECURED(_pAd) \
{ \
	_pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED; \
	RTMP_SET_PSFLAG(_pAd, fRTMP_PS_CAN_GO_SLEEP); \
	NdisAcquireSpinLock(&(_pAd)->MacTabLock); \
	_pAd->MacTab.Content[BSSID_WCID].PortSecured = _pAd->StaCfg.PortSecured; \
	NdisReleaseSpinLock(&(_pAd)->MacTabLock); \
}
#endif
#ifdef RT2870
#define STA_PORT_SECURED(_pAd) \
{ \
	_pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED; \
	NdisAcquireSpinLock(&_pAd->MacTabLock); \
	_pAd->MacTab.Content[BSSID_WCID].PortSecured = _pAd->StaCfg.PortSecured; \
	NdisReleaseSpinLock(&_pAd->MacTabLock); \
}
#endif




typedef struct  _RTMP_REG_PAIR
{
	ULONG   Register;
	ULONG   Value;
} RTMP_REG_PAIR, *PRTMP_REG_PAIR;

typedef struct  _REG_PAIR
{
	UCHAR   Register;
	UCHAR   Value;
} REG_PAIR, *PREG_PAIR;




typedef struct  _RTMP_RF_REGS
{
	UCHAR   Channel;
	ULONG   R1;
	ULONG   R2;
	ULONG   R3;
	ULONG   R4;
} RTMP_RF_REGS, *PRTMP_RF_REGS;

typedef struct _FREQUENCY_ITEM {
	UCHAR	Channel;
	UCHAR	N;
	UCHAR	R;
	UCHAR	K;
} FREQUENCY_ITEM, *PFREQUENCY_ITEM;





typedef struct  _RTMP_DMABUF
{
	ULONG                   AllocSize;
	PVOID                   AllocVa;            
	NDIS_PHYSICAL_ADDRESS   AllocPa;            
} RTMP_DMABUF, *PRTMP_DMABUF;


typedef	union	_HEADER_802_11_SEQ{
    struct {
	USHORT			Frag:4;
	USHORT			Sequence:12;
    }   field;
    USHORT           value;
}	HEADER_802_11_SEQ, *PHEADER_802_11_SEQ;





typedef struct  _RTMP_REORDERBUF
{
	BOOLEAN			IsFull;
	PVOID                   AllocVa;            
	UCHAR			Header802_3[14];
	HEADER_802_11_SEQ			Sequence;	
	UCHAR 		DataOffset;
	USHORT 		Datasize;
	ULONG                   AllocSize;
#ifdef RT2860
	NDIS_PHYSICAL_ADDRESS   AllocPa;            
#endif
#ifdef RT2870
	PUCHAR					AllocPa;
#endif 
}   RTMP_REORDERBUF, *PRTMP_REORDERBUF;









typedef struct _RTMP_DMACB
{
	ULONG                   AllocSize;          
	PVOID                   AllocVa;            
	NDIS_PHYSICAL_ADDRESS   AllocPa;            
	PNDIS_PACKET pNdisPacket;
	PNDIS_PACKET pNextNdisPacket;

	RTMP_DMABUF             DmaBuf;             
} RTMP_DMACB, *PRTMP_DMACB;

typedef struct _RTMP_TX_BUF
{
	PQUEUE_ENTRY    Next;
	UCHAR           Index;
	ULONG                   AllocSize;          
	PVOID                   AllocVa;            
	NDIS_PHYSICAL_ADDRESS   AllocPa;            
} RTMP_TXBUF, *PRTMP_TXBUF;

typedef struct _RTMP_RX_BUF
{
	BOOLEAN           InUse;
	ULONG           	ByBaRecIndex;
	RTMP_REORDERBUF	MAP_RXBuf[MAX_RX_REORDERBUF];
} RTMP_RXBUF, *PRTMP_RXBUF;
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
#ifdef RT2860
	ULONG           LastReceivedByteCount;
#endif
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

	UINT32   		OneSecFrameDuplicateCount;

#ifdef RT2870
	ULONG           OneSecTransmittedByteCount;   
#endif 

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

typedef struct _PID_COUNTER {
	ULONG           TxAckRequiredCount;      
	ULONG           TxAggreCount;
	ULONG           TxSuccessCount; 
	ULONG		LastSuccessRate;
} PID_COUNTER, *PPID_COUNTER;

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




typedef struct  _ARCFOUR
{
	UINT            X;
	UINT            Y;
	UCHAR           STATE[256];
} ARCFOURCONTEXT, *PARCFOURCONTEXT;


typedef struct  _RECEIVE_SETTING {
	USHORT   	NumOfRX:2;                 
	USHORT		Mode:2;	
	USHORT		ShortGI:1;
	USHORT		STBC:2;	
	USHORT		rsv:3;
	USHORT		OFDM:1;
	USHORT		MIMO:1;
 } RECEIVE_SETTING, *PRECEIVE_SETTING;


typedef struct  _WEP_KEY {
	UCHAR   KeyLen;                     
	UCHAR   Key[MAX_LEN_OF_KEY];        
} WEP_KEY, *PWEP_KEY;

typedef struct _CIPHER_KEY {
	UCHAR   Key[16];            
	UCHAR   RxMic[8];			
	UCHAR   TxMic[8];
	UCHAR   TxTsc[6];           
	UCHAR   RxTsc[6];           
	UCHAR   CipherAlg;          
	UCHAR   KeyLen;
	UCHAR   BssId[6];
            
	UCHAR   Type;               
} CIPHER_KEY, *PCIPHER_KEY;

typedef struct _BBP_TUNING_STRUCT {
	BOOLEAN     Enable;
	UCHAR       FalseCcaCountUpperBound;  
	UCHAR       FalseCcaCountLowerBound;  
	UCHAR       R17LowerBound;            
	UCHAR       R17UpperBound;            
	UCHAR       CurrentR17Value;
} BBP_TUNING, *PBBP_TUNING;

typedef struct _SOFT_RX_ANT_DIVERSITY_STRUCT {
	UCHAR     EvaluatePeriod;		 
#ifdef RT2870
	UCHAR     EvaluateStableCnt;
#endif
	UCHAR     Pair1PrimaryRxAnt;     
	UCHAR     Pair1SecondaryRxAnt;   
	UCHAR     Pair2PrimaryRxAnt;     
	UCHAR     Pair2SecondaryRxAnt;   
	SHORT     Pair1AvgRssi[2];       
	SHORT     Pair2AvgRssi[2];       
	SHORT     Pair1LastAvgRssi;      
	SHORT     Pair2LastAvgRssi;      
	ULONG     RcvPktNumWhenEvaluate;
	BOOLEAN   FirstPktArrivedWhenEvaluate;
	RALINK_TIMER_STRUCT    RxAntDiversityTimer;
} SOFT_RX_ANT_DIVERSITY, *PSOFT_RX_ANT_DIVERSITY;

typedef struct _LEAP_AUTH_INFO {
	BOOLEAN         Enabled;        
	BOOLEAN         CCKM;           
	UCHAR           Reserve[2];
	UCHAR           UserName[256];  
	ULONG           UserNameLen;
	UCHAR           Password[256];  
	ULONG           PasswordLen;
} LEAP_AUTH_INFO, *PLEAP_AUTH_INFO;

typedef struct {
	UCHAR        Addr[MAC_ADDR_LEN];
	UCHAR        ErrorCode[2];  
								
								
								
	BOOLEAN      Reported;
} ROGUEAP_ENTRY, *PROGUEAP_ENTRY;

typedef struct {
	UCHAR               RogueApNr;
	ROGUEAP_ENTRY       RogueApEntry[MAX_LEN_OF_BSS_TABLE];
} ROGUEAP_TABLE, *PROGUEAP_TABLE;

typedef struct {
	BOOLEAN     Enable;
	UCHAR       Delta;
	BOOLEAN     PlusSign;
} CCK_TX_POWER_CALIBRATE, *PCCK_TX_POWER_CALIBRATE;




typedef struct  _TUPLE_CACHE    {
	BOOLEAN         Valid;
	UCHAR           MacAddress[MAC_ADDR_LEN];
	USHORT          Sequence;
	USHORT          Frag;
} TUPLE_CACHE, *PTUPLE_CACHE;




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

typedef enum _ABGBAND_STATE_ {
	UNKNOWN_BAND,
	BG_BAND,
	A_BAND,
} ABGBAND_STATE;

typedef struct _MLME_STRUCT {
	
	STATE_MACHINE           CntlMachine;
	STATE_MACHINE           AssocMachine;
	STATE_MACHINE           AuthMachine;
	STATE_MACHINE           AuthRspMachine;
	STATE_MACHINE           SyncMachine;
	STATE_MACHINE           WpaPskMachine;
	STATE_MACHINE           LeapMachine;
	STATE_MACHINE           AironetMachine;
	STATE_MACHINE_FUNC      AssocFunc[ASSOC_FUNC_SIZE];
	STATE_MACHINE_FUNC      AuthFunc[AUTH_FUNC_SIZE];
	STATE_MACHINE_FUNC      AuthRspFunc[AUTH_RSP_FUNC_SIZE];
	STATE_MACHINE_FUNC      SyncFunc[SYNC_FUNC_SIZE];
	STATE_MACHINE_FUNC      WpaPskFunc[WPA_PSK_FUNC_SIZE];
	STATE_MACHINE_FUNC      AironetFunc[AIRONET_FUNC_SIZE];
	STATE_MACHINE_FUNC      ActFunc[ACT_FUNC_SIZE];
	
	STATE_MACHINE           ActMachine;

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
#ifdef RT2860
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

#ifdef RT2870
	UCHAR CaliBW40RfR24;
	UCHAR CaliBW20RfR24;
#endif 
} MLME_STRUCT, *PMLME_STRUCT;


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
	BOOLEAN		bFastDfs;
	UINT8		ChMovingTime;
	UINT8		LongPulseRadarTh;
} RADAR_DETECT_STRUCT, *PRADAR_DETECT_STRUCT;

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
	struct	{
		UINT32		RxBAWinLimit:8;
		UINT32		TxBAWinLimit:8;
		UINT32		AutoBA:1;	
		UINT32		Policy:2;	
		UINT32		MpduDensity:3;
		UINT32       	AmsduEnable:1;	
		UINT32       	AmsduSize:1;	
		UINT32       	MMPSmode:2;	
		UINT32       	bHtAdhoc:1;			
		UINT32       	b2040CoexistScanSup:1;		
		UINT32       	:4;
	}	field;
	UINT32			word;
} BACAP_STRUC, *PBACAP_STRUC;


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
	BOOLEAN			bLastAtheros;
    BOOLEAN			bCurrentAtheros;
    BOOLEAN         bNowAtherosBurstOn;
	BOOLEAN			bNextDisableRxBA;
    BOOLEAN			bToggle;
} IOT_STRUC, *PIOT_STRUC;


typedef union _REG_TRANSMIT_SETTING {
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
 UINT32   word;
} REG_TRANSMIT_SETTING, *PREG_TRANSMIT_SETTING;

typedef union  _DESIRED_TRANSMIT_SETTING {
	struct	{
			USHORT   	MCS:7;                 	
			USHORT		PhyMode:4;
			USHORT	 	FixedTxMode:2;			
			USHORT		rsv:3;
	}	field;
	USHORT		word;
 } DESIRED_TRANSMIT_SETTING, *PDESIRED_TRANSMIT_SETTING;

typedef struct {
	BOOLEAN		IsRecipient;
	UCHAR   MACAddr[MAC_ADDR_LEN];
	UCHAR   TID;
	UCHAR   nMSDU;
	USHORT   TimeOut;
	BOOLEAN bAllTid;  
} OID_ADD_BA_ENTRY, *POID_ADD_BA_ENTRY;




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

#ifdef RT2870
#define BEACON_BITMAP_MASK		0xff
typedef struct _BEACON_SYNC_STRUCT_
{
	UCHAR        			BeaconBuf[HW_BEACON_MAX_COUNT][HW_BEACON_OFFSET];
	UCHAR					BeaconTxWI[HW_BEACON_MAX_COUNT][TXWI_SIZE];
	ULONG 					TimIELocationInBeacon[HW_BEACON_MAX_COUNT];
	ULONG					CapabilityInfoLocationInBeacon[HW_BEACON_MAX_COUNT];
	BOOLEAN					EnableBeacon;		
	UCHAR					BeaconBitMap;		
	UCHAR					DtimBitOn;			
}BEACON_SYNC_STRUCT;
#endif 

typedef struct _MULTISSID_STRUCT {
	UCHAR								Bssid[MAC_ADDR_LEN];
    UCHAR                               SsidLen;
    CHAR                                Ssid[MAX_LEN_OF_SSID];
    USHORT                              CapabilityInfo;

    PNET_DEV                   			MSSIDDev;

	NDIS_802_11_AUTHENTICATION_MODE     AuthMode;
	NDIS_802_11_WEP_STATUS              WepStatus;
	NDIS_802_11_WEP_STATUS				GroupKeyWepStatus;
	WPA_MIX_PAIR_CIPHER					WpaMixPairCipher;

	ULONG								TxCount;
	ULONG								RxCount;
	ULONG								ReceivedByteCount;
	ULONG								TransmittedByteCount;
	ULONG								RxErrorCount;
	ULONG								RxDropCount;

	HTTRANSMIT_SETTING					HTPhyMode, MaxHTPhyMode, MinHTPhyMode;
	RT_HT_PHY_INFO						DesiredHtPhyInfo;
	DESIRED_TRANSMIT_SETTING        	DesiredTransmitSetting; 
	BOOLEAN								bAutoTxRateSwitch;

	UCHAR                               DefaultKeyId;

	UCHAR								TxRate;       
	UCHAR     							DesiredRates[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR								DesiredRatesIndex;
	UCHAR     							MaxTxRate;            

	UCHAR								TimBitmaps[WLAN_MAX_NUM_OF_TIM];

    
    UCHAR                               GMK[32];
    UCHAR                               PMK[32];
	UCHAR								GTK[32];
    BOOLEAN                             IEEE8021X;
    BOOLEAN                             PreAuth;
    UCHAR                               GNonce[32];
    UCHAR                               PortSecured;
    NDIS_802_11_PRIVACY_FILTER          PrivacyFilter;
    UCHAR                               BANClass3Data;
    ULONG                               IsolateInterStaTraffic;

    UCHAR                               RSNIE_Len[2];
    UCHAR                               RSN_IE[2][MAX_LEN_OF_RSNIE];


    UCHAR                   			TimIELocationInBeacon;
    UCHAR                   			CapabilityInfoLocationInBeacon;
    
	
    CHAR                                BeaconBuf[MAX_BEACON_SIZE]; 

    BOOLEAN                             bHideSsid;
	UINT16								StationKeepAliveTime; 

    USHORT                              VLAN_VID;
    USHORT                              VLAN_Priority;

    RT_802_11_ACL						AccessControlList;

	
    BOOLEAN								bWmmCapable;	
    BOOLEAN								bDLSCapable;	

	UCHAR           					DlsPTK[64];		

	
	UCHAR								radius_srv_num;
	RADIUS_SRV_INFO						radius_srv_info[MAX_RADIUS_SRV_NUM];

#ifdef RTL865X_SOC
	unsigned int						mylinkid;
#endif


	UINT32					RcvdConflictSsidCount;
	UINT32					RcvdSpoofedAssocRespCount;
	UINT32					RcvdSpoofedReassocRespCount;
	UINT32					RcvdSpoofedProbeRespCount;
	UINT32					RcvdSpoofedBeaconCount;
	UINT32					RcvdSpoofedDisassocCount;
	UINT32					RcvdSpoofedAuthCount;
	UINT32					RcvdSpoofedDeauthCount;
	UINT32					RcvdSpoofedUnknownMgmtCount;
	UINT32					RcvdReplayAttackCount;

	CHAR					RssiOfRcvdConflictSsid;
	CHAR					RssiOfRcvdSpoofedAssocResp;
	CHAR					RssiOfRcvdSpoofedReassocResp;
	CHAR					RssiOfRcvdSpoofedProbeResp;
	CHAR					RssiOfRcvdSpoofedBeacon;
	CHAR					RssiOfRcvdSpoofedDisassoc;
	CHAR					RssiOfRcvdSpoofedAuth;
	CHAR					RssiOfRcvdSpoofedDeauth;
	CHAR					RssiOfRcvdSpoofedUnknownMgmt;
	CHAR					RssiOfRcvdReplayAttack;

	BOOLEAN					bBcnSntReq;
	UCHAR					BcnBufIdx;
} MULTISSID_STRUCT, *PMULTISSID_STRUCT;


typedef struct _COMMON_CONFIG {

	BOOLEAN		bCountryFlag;
	UCHAR		CountryCode[3];
	UCHAR		Geography;
	UCHAR       CountryRegion;      
	UCHAR       CountryRegionForABand;	
	UCHAR       PhyMode;            
	USHORT      Dsifs;              
	ULONG       PacketFilter;       

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

	BACAP_STRUC        BACapability; 
	BACAP_STRUC        REGBACapability; 

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

	BOOLEAN				bRdg;

	BOOLEAN             bWmmCapable;        
	QOS_CAPABILITY_PARM APQosCapability;    
	EDCA_PARM           APEdcaParm;         
	QBSS_LOAD_PARM      APQbssLoad;         
	UCHAR               AckPolicy[4];       
	BOOLEAN				bDLSCapable;		
	
	
	
	
	ULONG               OpStatusFlags;

	BOOLEAN				NdisRadioStateOff; 
	ABGBAND_STATE		BandState;		

	
	RADAR_DETECT_STRUCT	RadarDetect;

	
	UCHAR			BASize;		
	
	RT_HT_CAPABILITY	DesiredHtPhy;
	HT_CAPABILITY_IE		HtCapability;
	ADD_HT_INFO_IE		AddHTInfo;	
	
	
	NEW_EXT_CHAN_IE	NewExtChanOffset;	

    BOOLEAN                 bHTProtect;
    BOOLEAN                 bMIMOPSEnable;
    BOOLEAN					bBADecline;
	BOOLEAN					bDisableReordering;
	BOOLEAN					bForty_Mhz_Intolerant;
	BOOLEAN					bExtChannelSwitchAnnouncement;
	BOOLEAN					bRcvBSSWidthTriggerEvents;
	ULONG					LastRcvBSSWidthTriggerEventsTime;

	UCHAR					TxBASize;

	
	BOOLEAN				bWirelessEvent;
	BOOLEAN				bWiFiTest;				

	
	UCHAR				TxStream;
	UCHAR				RxStream;

	
#ifdef MCAST_RATE_SPECIFIC
	UCHAR				McastTransmitMcs;
	UCHAR				McastTransmitPhyMode;
#endif 

	BOOLEAN     		bHardwareRadio;     

#ifdef RT2870
	BOOLEAN     		bMultipleIRP;       
	UCHAR       		NumOfBulkInIRP;     
 	RT_HT_CAPABILITY	SupportedHtPhy;
	ULONG				MaxPktOneTxBulk;
	UCHAR				TxBulkFactor;
	UCHAR				RxBulkFactor;

	BEACON_SYNC_STRUCT	*pBeaconSync;
	RALINK_TIMER_STRUCT	BeaconUpdateTimer;
	UINT32				BeaconAdjust;
	UINT32				BeaconFactor;
	UINT32				BeaconRemain;
#endif 


 	NDIS_SPIN_LOCK			MeasureReqTabLock;
	PMEASURE_REQ_TAB		pMeasureReqTab;

	NDIS_SPIN_LOCK			TpcReqTabLock;
	PTPC_REQ_TAB			pTpcReqTab;

	
#ifdef MCAST_RATE_SPECIFIC
	HTTRANSMIT_SETTING		MCastPhyMode;
#endif 
} COMMON_CONFIG, *PCOMMON_CONFIG;



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
#ifdef RT2860
    BOOLEAN		AdhocBOnlyJoined;	
    BOOLEAN		AdhocBGJoined;		
    BOOLEAN		Adhoc20NJoined;		
#endif
	
	
	NDIS_802_11_ASSOCIATION_INFORMATION     AssocInfo;
	USHORT       ReqVarIELen;                
	UCHAR       ReqVarIEs[MAX_VIE_LEN];		
	USHORT       ResVarIELen;                
	UCHAR       ResVarIEs[MAX_VIE_LEN];

	UCHAR       RSNIE_Len;
	UCHAR       RSN_IE[MAX_LEN_OF_RSNIE];	

	
	BOOLEAN             bCkipOn;
	BOOLEAN             bCkipCmicOn;
	UCHAR               CkipFlag;
	UCHAR               GIV[3];  
	UCHAR               RxSEQ[4];
	UCHAR               TxSEQ[4];
	UCHAR               CKIPMIC[4];
	UCHAR               LeapAuthMode;
	LEAP_AUTH_INFO      LeapAuthInfo;
	UCHAR               HashPwd[16];
	UCHAR               NetworkChallenge[8];
	UCHAR               NetworkChallengeResponse[24];
	UCHAR               PeerChallenge[8];

	UCHAR               PeerChallengeResponse[24];
	UCHAR               SessionKey[16]; 
	RALINK_TIMER_STRUCT LeapAuthTimer;
	ROGUEAP_TABLE       RogueApTab;   

	
	CCX_CONTROL         CCXControl;                 
	BOOLEAN             CCXEnable;                  
	UCHAR               CCXScanChannel;             
	USHORT              CCXScanTime;                
	UCHAR               CCXReqType;                 
	BSS_TABLE           CCXBssTab;                  
	UCHAR               FrameReportBuf[2048];       
	USHORT              FrameReportLen;             
	ULONG               CLBusyBytes;                
	USHORT              RPIDensity[8];              
	
	
	USHORT              BssReportOffset[MAX_LEN_OF_BSS_TABLE];
	USHORT              BeaconToken;                
	ULONG               LastBssIndex;               
	RM_REQUEST_ACTION   MeasurementRequest[16];     
	UCHAR               RMReqCnt;                   
	UCHAR               CurrentRMReqIdx;            
	BOOLEAN             ParallelReq;                
													
	USHORT              ParallelDuration;           
	UCHAR               ParallelChannel;            
	USHORT              IAPPToken;                  
	UCHAR               CCXQosECWMin;               
	UCHAR               CCXQosECWMax;               
	
	UCHAR               NHFactor;                   
	UCHAR               CLFactor;                   

	UCHAR               KRK[16];        
	UCHAR               BTK[32];        
	BOOLEAN             CCKMLinkUpFlag;
	ULONG               CCKMRN;    
	LARGE_INTEGER       CCKMBeaconAtJoinTimeStamp;  
	UCHAR               AironetCellPowerLimit;      
	UCHAR               AironetIPAddress[4];        
	BOOLEAN             CCXAdjacentAPReportFlag;    
	CHAR                CCXAdjacentAPSsid[MAX_LEN_OF_SSID]; 
	UCHAR               CCXAdjacentAPSsidLen;               
	UCHAR               CCXAdjacentAPBssid[MAC_ADDR_LEN];         
	USHORT              CCXAdjacentAPChannel;
	ULONG               CCXAdjacentAPLinkDownTime;  

	RALINK_TIMER_STRUCT	StaQuickResponeForRateUpTimer;
	BOOLEAN				StaQuickResponeForRateUpTimerRunning;

	UCHAR           	DtimCount;      
	UCHAR           	DtimPeriod;     

	
	
	BOOLEAN				WhqlTest;
	

    RALINK_TIMER_STRUCT WpaDisassocAndBlockAssocTimer;
    
	BOOLEAN		        bFastRoaming;       
	CHAR		        dBmToRoam;          

    BOOLEAN             IEEE8021X;
    BOOLEAN             IEEE8021x_required_keys;
    CIPHER_KEY	        DesireSharedKey[4];	
    UCHAR               DesireSharedKeyId;

    
    
    
    UCHAR               WpaSupplicantUP;
	UCHAR				WpaSupplicantScanCount;

    CHAR                dev_name[16];
    USHORT              OriDevType;

    BOOLEAN             bTGnWifiTest;
	BOOLEAN			    bScanReqIsFromWebUI;

	HTTRANSMIT_SETTING				HTPhyMode, MaxHTPhyMode, MinHTPhyMode;
	DESIRED_TRANSMIT_SETTING       	DesiredTransmitSetting;
	RT_HT_PHY_INFO					DesiredHtPhyInfo;
	BOOLEAN							bAutoTxRateSwitch;

#ifdef RT2860
    UCHAR       BBPR3;
#endif
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

#ifdef RT2870

typedef struct   RT_ADD_PAIRWISE_KEY_ENTRY {
        NDIS_802_11_MAC_ADDRESS         MacAddr;
        USHORT                          MacTabMatchWCID;        
        CIPHER_KEY                      CipherKey;
} RT_ADD_PAIRWISE_KEY_ENTRY,*PRT_ADD_PAIRWISE_KEY_ENTRY;
#endif 



typedef struct _AP_MLME_AUX {
	UCHAR               Addr[MAC_ADDR_LEN];
	USHORT              Alg;
	CHAR                Challenge[CIPHER_TEXT_LEN];
} AP_MLME_AUX, *PAP_MLME_AUX;


typedef struct PACKED _RT_802_11_WPA_REKEY {
	ULONG ReKeyMethod;          
	ULONG ReKeyInterval;        
} RT_WPA_REKEY,*PRT_WPA_REKEY, RT_802_11_WPA_REKEY, *PRT_802_11_WPA_REKEY;

typedef struct _MAC_TABLE_ENTRY {
	
	BOOLEAN		ValidAsCLI;		
	BOOLEAN		ValidAsWDS;	
	BOOLEAN		ValidAsApCli;   
	BOOLEAN		ValidAsMesh;
	BOOLEAN		ValidAsDls;	
	BOOLEAN		isCached;
	BOOLEAN		bIAmBadAtheros;	

	UCHAR         	EnqueueEapolStartTimerRunning;  
	
	
	UCHAR           CMTimerRunning;
	UCHAR           apidx;			
	UCHAR           RSNIE_Len;
	UCHAR           RSN_IE[MAX_LEN_OF_RSNIE];
	UCHAR           ANonce[LEN_KEY_DESC_NONCE];
	UCHAR           R_Counter[LEN_KEY_DESC_REPLAY];
	UCHAR           PTK[64];
	UCHAR           ReTryCounter;
	RALINK_TIMER_STRUCT                 RetryTimer;
	RALINK_TIMER_STRUCT					EnqueueStartForPSKTimer;	
	NDIS_802_11_AUTHENTICATION_MODE     AuthMode;   
	NDIS_802_11_WEP_STATUS              WepStatus;
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

	BOOLEAN			bSendBAR;
	USHORT			NoBADataCountDown;

	UINT32   		CachedBuf[16];		
	UINT			TxBFCount; 
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


	BOOLEAN         fNoisyEnvironment;
	BOOLEAN			fLastSecAccordingRSSI;
	UCHAR           LastSecTxRateChangeAction; 
	CHAR			LastTimeTxRateChangeAction; 
	ULONG			LastTxOkCount;
	UCHAR           PER[MAX_STEP_OF_TX_RATE_SWITCH];

	
	
	
	
	ULONG           ClientStatusFlags;

	HTTRANSMIT_SETTING	HTPhyMode, MaxHTPhyMode, MinHTPhyMode;

	
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
#ifdef RT2870
	ULONG   		LastBeaconRxTime;
#endif
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
#ifdef RT2870
	BOOLEAN			fAllStationAsRalink; 	
#endif
	BOOLEAN         fAnyStationIsLegacy;	
	BOOLEAN         fAnyStationNonGF;		
	BOOLEAN         fAnyStation20Only;		
	BOOLEAN			fAnyStationMIMOPSDynamic; 
	BOOLEAN         fAnyBASession;   
} MAC_TABLE, *PMAC_TABLE;

#define IS_HT_STA(_pMacEntry)	\
	(_pMacEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX)

#define IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

#define PEER_IS_HT_RATE(_pMacEntry)	\
	(_pMacEntry->HTPhyMode.field.MODE >= MODE_HTMIX)

typedef struct _WDS_ENTRY {
	BOOLEAN         Valid;
	UCHAR           Addr[MAC_ADDR_LEN];
	ULONG           NoDataIdleCount;
	struct _WDS_ENTRY *pNext;
} WDS_ENTRY, *PWDS_ENTRY;

typedef struct  _WDS_TABLE_ENTRY {
	USHORT			Size;
	UCHAR           WdsAddr[MAC_ADDR_LEN];
	WDS_ENTRY       *Hash[HASH_TABLE_SIZE];
	WDS_ENTRY       Content[MAX_LEN_OF_MAC_TABLE];
	UCHAR           MaxSupportedRate;
	UCHAR           CurrTxRate;
	USHORT          TxQuality[MAX_LEN_OF_SUPPORTED_RATES];
	USHORT          OneSecTxOkCount;
	USHORT          OneSecTxRetryOkCount;
	USHORT          OneSecTxFailCount;
	ULONG           CurrTxRateStableTime; 
	UCHAR           TxRateUpPenalty;      
} WDS_TABLE_ENTRY, *PWDS_TABLE_ENTRY;

typedef struct _RT_802_11_WDS_ENTRY {
	PNET_DEV			dev;
	UCHAR				Valid;
	UCHAR				PhyMode;
	UCHAR				PeerWdsAddr[MAC_ADDR_LEN];
	UCHAR				MacTabMatchWCID;	
	NDIS_802_11_WEP_STATUS  WepStatus;
	UCHAR					KeyIdx;
	CIPHER_KEY          	WdsKey;
	HTTRANSMIT_SETTING				HTPhyMode, MaxHTPhyMode, MinHTPhyMode;
	RT_HT_PHY_INFO					DesiredHtPhyInfo;
	BOOLEAN							bAutoTxRateSwitch;
	DESIRED_TRANSMIT_SETTING       	DesiredTransmitSetting; 
} RT_802_11_WDS_ENTRY, *PRT_802_11_WDS_ENTRY;

typedef struct _WDS_TABLE {
	UCHAR               Mode;
	ULONG               Size;
	RT_802_11_WDS_ENTRY	WdsEntry[MAX_WDS_ENTRY];
} WDS_TABLE, *PWDS_TABLE;

typedef struct _APCLI_STRUCT {
	PNET_DEV				dev;
#ifdef RTL865X_SOC
	unsigned int            mylinkid;
#endif
	BOOLEAN                 Enable;	
	BOOLEAN                 Valid;	
	UCHAR					MacTabWCID;	
	UCHAR                   SsidLen;
	CHAR                    Ssid[MAX_LEN_OF_SSID];

	UCHAR                   CfgSsidLen;
	CHAR                    CfgSsid[MAX_LEN_OF_SSID];
	UCHAR                   CfgApCliBssid[ETH_LENGTH_OF_ADDRESS];
	UCHAR                   CurrentAddress[ETH_LENGTH_OF_ADDRESS];

	ULONG                   ApCliRcvBeaconTime;

	ULONG                   CtrlCurrState;
	ULONG                   SyncCurrState;
	ULONG                   AuthCurrState;
	ULONG                   AssocCurrState;
	ULONG					WpaPskCurrState;

	USHORT                  AuthReqCnt;
	USHORT                  AssocReqCnt;

	ULONG                   ClientStatusFlags;
	UCHAR                   MpduDensity;

	NDIS_802_11_AUTHENTICATION_MODE     AuthMode;   
	NDIS_802_11_WEP_STATUS              WepStatus;

	
	NDIS_802_11_ENCRYPTION_STATUS		GroupCipher;		
	NDIS_802_11_ENCRYPTION_STATUS		PairCipher;			
	BOOLEAN								bMixCipher;			
	USHORT								RsnCapability;

	UCHAR		PSK[100];				
	UCHAR       PSKLen;
	UCHAR       PMK[32];                
	UCHAR		GTK[32];				

	CIPHER_KEY      SharedKey[SHARE_KEY_NUM];
	UCHAR           DefaultKeyId;

	
	UCHAR		RSN_IE[MAX_LEN_OF_RSNIE];  
	UCHAR		RSNIE_Len;

	
	ULONG       LastMicErrorTime;   
	BOOLEAN                 bBlockAssoc; 

	
	UCHAR       	SNonce[32];         
	UCHAR			GNonce[32];			

	HTTRANSMIT_SETTING				HTPhyMode, MaxHTPhyMode, MinHTPhyMode;
	RT_HT_PHY_INFO					DesiredHtPhyInfo;
	BOOLEAN							bAutoTxRateSwitch;
	DESIRED_TRANSMIT_SETTING       	DesiredTransmitSetting; 
} APCLI_STRUCT, *PAPCLI_STRUCT;



struct wificonf
{
	BOOLEAN	bShortGI;
	BOOLEAN bGreenField;
};




typedef struct _INF_PCI_CONFIG
{
	PUCHAR                  CSRBaseAddress;     
}INF_PCI_CONFIG;

typedef struct _INF_USB_CONFIG
{
	UINT                BulkInEpAddr;		
	UINT                BulkOutEpAddr[6];	

}INF_USB_CONFIG;






typedef struct _RTMP_ADAPTER
{
	PVOID					OS_Cookie;	
	PNET_DEV				net_dev;
	ULONG					VirtualIfCnt;

#ifdef RT2860
    USHORT		            LnkCtrlBitMask;
    USHORT		            RLnkCtrlConfiguration;
    USHORT                  RLnkCtrlOffset;
    USHORT		            HostLnkCtrlConfiguration;
    USHORT                  HostLnkCtrlOffset;
	USHORT		            PCIePowerSaveLevel;
   	BOOLEAN					bPCIclkOff;						
   	ULONG					CheckDmaBusyCount;  
   	USHORT					ThisTbttNumToNextWakeUp;
	ULONG					SameRxByteCount;





	PUCHAR                  CSRBaseAddress;     

	UINT					int_enable_reg;
	UINT					int_disable_mask;
	UINT					int_pending;


	RTMP_DMABUF             TxBufSpace[NUM_OF_TX_RING]; 
	RTMP_DMABUF             RxDescRing;                 
	RTMP_DMABUF             TxDescRing[NUM_OF_TX_RING]; 	
	RTMP_TX_RING            TxRing[NUM_OF_TX_RING];     	
#endif

	NDIS_SPIN_LOCK          irq_lock;
	UCHAR                   irq_disabled;

#ifdef RT2870



	struct usb_config_descriptor		*config;
	UINT								BulkInEpAddr;		
	UINT								BulkOutEpAddr[6];	

	UINT								NumberOfPipes;
	USHORT								BulkOutMaxPacketSize;
	USHORT								BulkInMaxPacketSize;

	
	LONG                    	PendingIoCount;
	ULONG						BulkFlags;
	BOOLEAN                     bUsbTxBulkAggre;	


	
	RT2870_TIMER_QUEUE		TimerQ;
	NDIS_SPIN_LOCK			TimerQLock;


	
	CmdQ					CmdQ;
	NDIS_SPIN_LOCK			CmdQLock;				

	BOOLEAN					TimerFunc_kill;
	BOOLEAN					mlme_kill;


	
	struct semaphore			mlme_semaphore;			
	struct semaphore			RTUSBCmd_semaphore;		
	struct semaphore			RTUSBTimer_semaphore;
	struct completion			TimerQComplete;
	struct completion			mlmeComplete;
	struct completion			CmdQComplete;
	wait_queue_head_t			*wait;
#endif 



	






	BOOLEAN                 DeQueueRunning[NUM_OF_TX_RING];  
	NDIS_SPIN_LOCK          DeQueueLock[NUM_OF_TX_RING];

#ifdef RT2870
	
	NDIS_SPIN_LOCK			BulkOutLock[6];			
	NDIS_SPIN_LOCK			MLMEBulkOutLock;	

	HT_TX_CONTEXT			TxContext[NUM_OF_TX_RING];
	NDIS_SPIN_LOCK			TxContextQueueLock[NUM_OF_TX_RING];		

	
	UCHAR					NextBulkOutIndex[4];	

	BOOLEAN					BulkOutPending[6];	
	UCHAR					bulkResetPipeid;
	BOOLEAN					MgmtBulkPending;
	ULONG					bulkResetReq[6];
#endif 

	
	QUEUE_HEADER            TxSwQueue[NUM_OF_TX_RING];  
	NDIS_SPIN_LOCK          TxSwQueueLock[NUM_OF_TX_RING];	

	RTMP_DMABUF             MgmtDescRing;               	
	RTMP_MGMT_RING          MgmtRing;
	NDIS_SPIN_LOCK          MgmtRingLock;               	





#ifdef RT2860
	RTMP_RX_RING            RxRing;
	NDIS_SPIN_LOCK          RxRingLock;                 
#endif
#ifdef RT2870
	RX_CONTEXT				RxContext[RX_RING_SIZE];  
	NDIS_SPIN_LOCK			BulkInLock;				
	UCHAR					PendingRx;				
	UCHAR					NextRxBulkInIndex;		
	UCHAR					NextRxBulkInReadIndex;	
	ULONG					NextRxBulkInPosition;   
	ULONG					TransferBufferLength;	
	ULONG					ReadPosition;			
#endif 





	UINT32               	MACVersion;      	

	
	
	
	ULONG                   EepromVersion;          
	UCHAR                   EEPROMAddressNum;       
	USHORT                  EEPROMDefaultValue[NUM_EEPROM_BBP_PARMS];
#ifdef RT2870
	BOOLEAN                 EepromAccess;
#endif
	ULONG                   FirmwareVersion;        

	
	
	
	UCHAR                   BbpWriteLatch[140];     
	UCHAR                   BbpRssiToDbmDelta;
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

	
	
	
	MCU_LEDCS_STRUC		LedCntl;
	USHORT				Led1;	
	USHORT				Led2;	
	USHORT				Led3;	
	UCHAR				LedIndicatorStregth;
	UCHAR				RssiSingalstrengthOffet;
    BOOLEAN				bLedOnScanning;
	UCHAR				LedStatus;




	
	TXWI_STRUC              	BeaconTxWI;
	PUCHAR						BeaconBuf;
	USHORT						BeaconOffset[HW_BEACON_MAX_COUNT];

	
	PSPOLL_FRAME            	PsPollFrame;
	HEADER_802_11           	NullFrame;

#ifdef RT2870
	TX_CONTEXT				BeaconContext[BEACON_RING_SIZE];
	TX_CONTEXT				NullContext;
	TX_CONTEXT				PsPollContext;
	TX_CONTEXT				RTSContext;
#endif 








	
	
	
	
	STA_ADMIN_CONFIG        StaCfg;           
	STA_ACTIVE_CONFIG       StaActive;         
	CHAR                    nickname[IW_ESSID_MAX_SIZE+1]; 
	NDIS_MEDIA_STATE        PreMediaState;


	
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

	BA_TABLE			BATable;

	NDIS_SPIN_LOCK          BATabLock;
	RALINK_TIMER_STRUCT RECBATimer;

	
	CIPHER_KEY              SharedKey[MAX_MBSSID_NUM][4]; 

		
	FRAGMENT_FRAME          FragFrame;                  

	
	COUNTER_802_3           Counters8023;               
	COUNTER_802_11          WlanCounters;               
	COUNTER_RALINK          RalinkCounters;             
	COUNTER_DRS             DrsCounters;                
	PRIVATE_STRUC           PrivateInfo;                

	
	ULONG                   Flags;                      
#ifdef RT2860
	ULONG                   PSFlags;                    
#endif

	
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

	
	
	
#ifdef RT2870
	ULONG						BulkOutDataOneSecCount;
	ULONG						BulkInDataOneSecCount;
	ULONG						BulkLastOneSecCount; 
	ULONG						watchDogRxCnt;
	ULONG						watchDogRxOverFlowCnt;
	ULONG						watchDogTxPendingCnt[NUM_OF_TX_RING];
#endif 

	BOOLEAN						bUpdateBcnCntDone;
	ULONG						watchDogMacDeadlock;	
	
	
	
	BOOLEAN		bBanAllBaSetup;
	BOOLEAN		bPromiscuous;

	
	
	
	ULONG		rtsaccu[30];
	ULONG		ctsaccu[30];
	ULONG		cfendaccu[30];
	ULONG		bacontent[16];
	ULONG		rxint[RX_RING_SIZE+1];
	UCHAR		rcvba[60];
	BOOLEAN		bLinkAdapt;
	BOOLEAN		bForcePrintTX;
	BOOLEAN		bForcePrintRX;
	BOOLEAN		bDisablescanning;		
	BOOLEAN		bStaFifoTest;
	BOOLEAN		bProtectionTest;
	BOOLEAN		bHCCATest;
	BOOLEAN		bGenOneHCCA;
	BOOLEAN		bBroadComHT;
	
	ULONG		BulkOutReq;
	ULONG		BulkOutComplete;
	ULONG		BulkOutCompleteOther;
	ULONG		BulkOutCompleteCancel;	
	ULONG		BulkInReq;
	ULONG		BulkInComplete;
	ULONG		BulkInCompleteFail;
	

    struct wificonf			WIFItestbed;

	struct reordering_mpdu_pool mpdu_blk_pool;

	ULONG					OneSecondnonBEpackets;		

    struct iw_statistics    iw_stats;

	struct net_device_stats	stats;

	ULONG					TbttTickCount;
#ifdef PCI_MSI_SUPPORT
	BOOLEAN					HaveMsi;
#endif 


	UCHAR					is_on;

#define TIME_BASE			(1000000/OS_HZ)
#define TIME_ONE_SECOND		(1000000/TIME_BASE)
	UCHAR					flg_be_adjust;
	ULONG					be_adjust_last_time;



	UINT8					PM_FlgSuspend;

#ifdef RT2870
	BOOLEAN		bUseEfuse;
#endif
} RTMP_ADAPTER, *PRTMP_ADAPTER;




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

#define DELAYINTMASK		0x0003fffb
#define INTMASK				0x0003fffb
#define IndMask				0x0003fffc
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
#define fRX_AMSDU       0x0002
#define fRX_ARALINK     0x0004
#define fRX_HTC         0x0008
#define fRX_PAD         0x0010
#define fRX_AMPDU       0x0020
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

#define TX_UNKOWN_FRAME			0x00
#define TX_MCAST_FRAME			0x01
#define TX_LEGACY_FRAME			0x02
#define TX_AMPDU_FRAME			0x04
#define TX_AMSDU_FRAME			0x08
#define TX_RALINK_FRAME			0x10
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
	UCHAR				HeaderBuf[80];				
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


#define fTX_bRtsRequired		0x0001	
#define fTX_bAckRequired       	0x0002	
#define fTX_bPiggyBack     		0x0004	
#define fTX_bHTRate         	0x0008	
#define fTX_bForceNonQoS       	0x0010	
#define fTX_bAllowFrag       	0x0020	
#define fTX_bMoreData			0x0040	
#define fTX_bWMM				0x0080	

#define fTX_bClearEAPFrame		0x0100

#define TX_BLK_ASSIGN_FLAG(_pTxBlk, _flag, value)	\
		do {										\
			if (value) 								\
				(_pTxBlk->Flags |= _flag) 			\
			else 									\
				(_pTxBlk->Flags &= ~(_flag))		\
		}while(0)

#define TX_BLK_SET_FLAG(_pTxBlk, _flag)		(_pTxBlk->Flags |= _flag)
#define TX_BLK_TEST_FLAG(_pTxBlk, _flag)	(((_pTxBlk->Flags & _flag) == _flag) ? 1 : 0)
#define TX_BLK_CLEAR_FLAG(_pTxBlk, _flag)	(_pTxBlk->Flags &= ~(_flag))







#ifdef RT2860




__inline    VOID    NICDisableInterrupt(
    IN  PRTMP_ADAPTER   pAd)
{
	RTMP_IO_WRITE32(pAd, INT_MASK_CSR, 0x0);     
	
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
}

__inline    VOID    NICEnableInterrupt(
    IN  PRTMP_ADAPTER   pAd)
{
	
	
	
	
	
    
	
	
	{
		RTMP_IO_WRITE32(pAd, INT_MASK_CSR, pAd->int_enable_reg );     
	}
	
	

	
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
}

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
#endif 




NDIS_STATUS RTMPAllocAdapterBlock(
	IN PVOID			handle,
	OUT PRTMP_ADAPTER   *ppAdapter
	);

NDIS_STATUS RTMPAllocTxRxRingMemory(
	IN  PRTMP_ADAPTER   pAd
	);

NDIS_STATUS	RTMPReadParametersHook(
	IN	PRTMP_ADAPTER pAd
	);

VOID RTMPFreeAdapter(
	IN  PRTMP_ADAPTER   pAd
	);

NDIS_STATUS NICReadRegParameters(
	IN  PRTMP_ADAPTER       pAd,
	IN  NDIS_HANDLE         WrapperConfigurationContext
	);

#ifdef RT2870
VOID NICInitRT30xxRFRegisters(
	IN PRTMP_ADAPTER pAd);
#endif 

VOID NICReadEEPROMParameters(
	IN  PRTMP_ADAPTER       pAd,
	IN	PUCHAR				mac_addr);

VOID NICInitAsicFromEEPROM(
	IN  PRTMP_ADAPTER       pAd);

VOID NICInitTxRxRingAndBacklogQueue(
	IN  PRTMP_ADAPTER   pAd);

NDIS_STATUS NICInitializeAdapter(
	IN  PRTMP_ADAPTER   pAd,
	IN   BOOLEAN    bHardReset);

NDIS_STATUS NICInitializeAsic(
	IN  PRTMP_ADAPTER   pAd,
	IN  BOOLEAN		bHardReset);
#ifdef RT2860
VOID NICRestoreBBPValue(
	IN PRTMP_ADAPTER pAd);
#endif
VOID NICIssueReset(
	IN  PRTMP_ADAPTER   pAd);

VOID RTMPRingCleanUp(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           RingType);

VOID UserCfgInit(
	IN  PRTMP_ADAPTER   pAd);

VOID NICResetFromError(
	IN  PRTMP_ADAPTER   pAd);

VOID NICEraseFirmware(
	IN PRTMP_ADAPTER pAd);

NDIS_STATUS NICLoadFirmware(
	IN  PRTMP_ADAPTER   pAd);

NDIS_STATUS NICLoadRateSwitchingParams(
	IN PRTMP_ADAPTER pAd);

BOOLEAN NICCheckForHang(
	IN  PRTMP_ADAPTER   pAd);

VOID NICUpdateFifoStaCounters(
	IN PRTMP_ADAPTER pAd);

VOID NICUpdateRawCounters(
	IN  PRTMP_ADAPTER   pAd);

ULONG	RTMPNotAllZero(
	IN	PVOID	pSrc1,
	IN	ULONG	Length);

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
	char	*src,
	UCHAR	*dest,
	int		destlen);

UCHAR BtoH(
	char ch);

VOID RTMPPatchMacBbpBug(
	IN  PRTMP_ADAPTER   pAd);

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
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR			Status);

VOID RTMPSetSignalLED(
	IN PRTMP_ADAPTER 	pAd,
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

VOID PeerHTAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID PeerQOSAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

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

BOOLEAN CntlEnqueueForRecv(
    IN	PRTMP_ADAPTER	pAd,
	IN ULONG Wcid,
    IN ULONG MsgLen,
	IN PFRAME_BA_REQ pMsg);




BOOLEAN RTMPHandleRxDoneInterrupt(
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
	IN 	PUCHAR          FreeNumberIs);

NDIS_STATUS MlmeHardTransmit(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR	QueIdx,
	IN  PNDIS_PACKET    pPacket);

NDIS_STATUS MlmeHardTransmitMgmtRing(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR	QueIdx,
	IN  PNDIS_PACKET    pPacket);

NDIS_STATUS MlmeHardTransmitTxRing(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR	QueIdx,
	IN  PNDIS_PACKET    pPacket);

USHORT  RTMPCalcDuration(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           Rate,
	IN  ULONG           Size);

VOID RTMPWriteTxWI(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXWI_STRUC		pTxWI,
	IN  BOOLEAN    		FRAG,
	IN  BOOLEAN    		CFACK,
	IN  BOOLEAN    		InsTimestamp,
	IN	BOOLEAN			AMPDU,
	IN	BOOLEAN			Ack,
	IN	BOOLEAN			NSeq,		
	IN	UCHAR			BASize,
	IN	UCHAR			WCID,
	IN	ULONG			Length,
	IN  UCHAR      		PID,
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

VOID RTMPWriteTxDescriptor(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXD_STRUC		pTxD,
	IN	BOOLEAN			bWIV,
	IN	UCHAR			QSEL);

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

PQUEUE_HEADER   RTMPCheckTxSwQueue(
	IN  PRTMP_ADAPTER   pAd,
	OUT UCHAR           *QueIdx);

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

BOOLEAN	RTMPSoftDecryptWEP(
	IN PRTMP_ADAPTER 	pAd,
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

VOID 	AsicUpdateProtect(
	IN		PRTMP_ADAPTER	pAd,
	IN 		USHORT			OperaionMode,
	IN 		UCHAR			SetMask,
	IN		BOOLEAN			bDisableBGProtect,
	IN		BOOLEAN			bNonGFExist);

VOID AsicSwitchChannel(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			Channel,
	IN	BOOLEAN			bScan);

VOID AsicLockChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel) ;

VOID AsicRfTuningExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID AsicSleepThenAutoWakeup(
	IN  PRTMP_ADAPTER   pAd,
	IN  USHORT TbttNumToNextWakeUp);

VOID AsicForceSleep(
	IN PRTMP_ADAPTER pAd);

VOID AsicForceWakeup(
	IN PRTMP_ADAPTER pAd,
#ifdef RT2860
	IN UCHAR    	 Level);
#endif
#ifdef RT2870
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
#ifdef RT2860
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

VOID BATableInit(
	IN PRTMP_ADAPTER pAd,
    IN BA_TABLE *Tab);

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

VOID BssTableDeleteEntry(
	IN OUT  PBSS_TABLE pTab,
	IN      PUCHAR pBssid,
	IN      UCHAR Channel);

VOID BATableDeleteORIEntry(
	IN OUT	PRTMP_ADAPTER pAd,
	IN		BA_ORI_ENTRY	*pBAORIEntry);

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

VOID BATableInsertEntry(
    IN	PRTMP_ADAPTER	pAd,
	IN USHORT Aid,
    IN USHORT		TimeOutValue,
	IN USHORT		StartingSeq,
    IN UCHAR TID,
	IN UCHAR BAWinSize,
	IN UCHAR OriginatorStatus,
    IN BOOLEAN IsRecipient);

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

VOID SwitchBetweenWepAndCkip(
	IN PRTMP_ADAPTER pAd);

VOID  InvalidStateWhenAssoc(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID  InvalidStateWhenReassoc(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenDisassociate(
	IN  PRTMP_ADAPTER pAd,
	IN  MLME_QUEUE_ELEM *Elem);

#ifdef RT2870
VOID MlmeCntlConfirm(
	IN PRTMP_ADAPTER pAd,
	IN ULONG MsgType,
	IN USHORT Msg);
#endif 

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

VOID InvalidStateWhenScan(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenJoin(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID InvalidStateWhenStart(
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
	IN  CHAR Ssid[],
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
	OUT UCHAR		 *pPreNHtCapabilityLen,
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
	OUT CHAR *Buffer,
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

VOID MlmeCheckForFastRoaming(
	IN  PRTMP_ADAPTER   pAd,
	IN  ULONG           Now);

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
	IN PRTMP_ADAPTER 	pAd,
	IN 	BOOLEAN		 	bLinkUp,
	IN	UCHAR			apidx);

VOID MlmeUpdateHtTxRates(
	IN PRTMP_ADAPTER 		pAd,
	IN	UCHAR				apidx);

VOID    RTMPCheckRates(
	IN      PRTMP_ADAPTER   pAd,
	IN OUT  UCHAR           SupRate[],
	IN OUT  UCHAR           *SupRateLen);

BOOLEAN RTMPCheckChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		CentralChannel,
	IN UCHAR		Channel);

BOOLEAN 	RTMPCheckHt(
	IN		PRTMP_ADAPTER	pAd,
	IN		UCHAR	Wcid,
	IN OUT	HT_CAPABILITY_IE			*pHtCapability,
	IN OUT	ADD_HT_INFO_IE			*pAddHtInfo);

VOID StaQuickResponeForRateUpExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID RTMPUpdateMlmeRate(
	IN PRTMP_ADAPTER	pAd);

CHAR RTMPMaxRssi(
	IN PRTMP_ADAPTER	pAd,
	IN CHAR				Rssi0,
	IN CHAR				Rssi1,
	IN CHAR				Rssi2);

VOID AsicSetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant);

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

USHORT RTMP_EEPROM_READ16(
	IN  PRTMP_ADAPTER   pAd,
	IN  USHORT Offset);

VOID RTMP_EEPROM_WRITE16(
	IN  PRTMP_ADAPTER   pAd,
	IN  USHORT Offset,
	IN  USHORT Data);




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
	IN 	UCHAR		 	KeyIdx,
	IN 	UCHAR		 	CipherAlg,
	IN 	MAC_TABLE_ENTRY *pEntry);

CHAR *GetEncryptType(
	CHAR enc);

CHAR *GetAuthMode(
	CHAR auth);

VOID RTMPIoctlGetSiteSurvey(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq);

VOID RTMPIoctlGetMacTable(
	IN PRTMP_ADAPTER pAd,
	IN struct iwreq *wrq);

VOID    RTMPAddBSSIDCipher(
    IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR	Aid,
    IN  PNDIS_802_11_KEY    pKey,
    IN  UCHAR   CipherAlg);

VOID	RTMPSetHT(
	IN	PRTMP_ADAPTER	pAd,
	IN	OID_SET_HT_PHYMODE *pHTPhyMode);

VOID	RTMPSetIndividualHT(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				apidx);

VOID RTMPSendWirelessEvent(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Event_flag,
	IN	PUCHAR 			pAddr,
	IN  UCHAR			BssIdx,
	IN	CHAR			Rssi);




BOOLEAN WpaMsgTypeSubst(
	IN  UCHAR   EAPType,
	OUT INT		*MsgType);

VOID WpaPskStateMachineInit(
	IN  PRTMP_ADAPTER       pAd,
	IN  STATE_MACHINE       *S,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID WpaEAPOLKeyAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID    WpaPairMsg1Action(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID    WpaPairMsg3Action(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID    WpaGroupMsg1Action(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID    WpaMacHeaderInit(
	IN      PRTMP_ADAPTER   pAd,
	IN OUT  PHEADER_802_11  pHdr80211,
	IN      UCHAR           wep,
	IN      PUCHAR          pAddr1);

VOID    Wpa2PairMsg1Action(
    IN  PRTMP_ADAPTER   pAd,
    IN  MLME_QUEUE_ELEM *Elem);

VOID    Wpa2PairMsg3Action(
    IN  PRTMP_ADAPTER   pAd,
    IN  MLME_QUEUE_ELEM *Elem);

BOOLEAN ParseKeyData(
    IN  PRTMP_ADAPTER   pAd,
    IN  PUCHAR          pKeyData,
    IN  UCHAR           KeyDataLen,
	IN	UCHAR			bPairewise);

VOID    RTMPToWirelessSta(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pHeader802_3,
    IN  UINT            HdrLen,
	IN  PUCHAR          pData,
    IN  UINT            DataLen,
    IN	BOOLEAN			is4wayFrame);

VOID    HMAC_SHA1(
	IN  UCHAR   *text,
	IN  UINT    text_len,
	IN  UCHAR   *key,
	IN  UINT    key_len,
	IN  UCHAR   *digest);

VOID    PRF(
	IN  UCHAR   *key,
	IN  INT     key_len,
	IN  UCHAR   *prefix,
	IN  INT     prefix_len,
	IN  UCHAR   *data,
	IN  INT     data_len,
	OUT UCHAR   *output,
	IN  INT     len);

VOID    CCKMPRF(
	IN  UCHAR   *key,
	IN  INT     key_len,
	IN  UCHAR   *data,
	IN  INT     data_len,
	OUT UCHAR   *output,
	IN  INT     len);

VOID WpaCountPTK(
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




VOID    AironetStateMachineInit(
	IN  PRTMP_ADAPTER       pAd,
	IN  STATE_MACHINE       *S,
	OUT STATE_MACHINE_FUNC  Trans[]);

VOID    AironetMsgAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID    AironetRequestAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID    ChannelLoadRequestAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           Index);

VOID    NoiseHistRequestAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           Index);

VOID    BeaconRequestAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           Index);

VOID    AironetReportAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem);

VOID    ChannelLoadReportAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           Index);

VOID    NoiseHistReportAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           Index);

VOID    AironetFinalReportAction(
	IN  PRTMP_ADAPTER   pAd);

VOID    BeaconReportAction(
	IN  PRTMP_ADAPTER   pAd,
	IN  UCHAR           Index);

VOID    AironetAddBeaconReport(
	IN  PRTMP_ADAPTER       pAd,
	IN  ULONG               Index,
	IN  PMLME_QUEUE_ELEM    pElem);

VOID    AironetCreateBeaconReportFromBssTable(
	IN  PRTMP_ADAPTER       pAd);

CHAR    ConvertToRssi(
	IN PRTMP_ADAPTER  pAd,
	IN CHAR				Rssi,
	IN UCHAR    RssiNumber);




BOOLEAN RTMPCheckWPAframe(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN PUCHAR 			pData,
	IN ULONG 			DataByteCount,
	IN UCHAR			FromWhichBSSID);

VOID AES_GTK_KEY_UNWRAP(
	IN  UCHAR   *key,
	OUT UCHAR   *plaintext,
	IN	UCHAR	c_len,
	IN  UCHAR   *ciphertext);

VOID RTMPMakeRSNIE(
	IN  PRTMP_ADAPTER   pAd,
	IN  UINT            AuthMode,
	IN  UINT            WepStatus,
	IN	UCHAR			apidx);





VOID HandleCounterMeasure(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY  *pEntry);


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
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR *mem,
	IN	ULONG  size);

NDIS_STATUS os_free_mem(
	IN	PRTMP_ADAPTER pAd,
	IN	PUCHAR mem);


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

BOOLEAN rtstrmactohex(
	IN char *s1,
	IN char *s2);

BOOLEAN rtstrcasecmp(
	IN char *s1,
	IN char *s2);

char *rtstrstruncasecmp(
	IN char *s1,
	IN char *s2);

char    *rtstrstr(
	IN	const char * s1,
	IN	const char * s2);

char *rstrtok(
	IN char * s,
	IN const char * ct);

int rtinet_aton(
	const char *cp,
	unsigned int *addr);


INT Set_DriverVersion_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT Set_CountryRegion_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT Set_CountryRegionABand_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT Set_WirelessMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT Set_Channel_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_ShortSlot_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_TxPower_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT Set_BGProtection_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PUCHAR			arg);

INT Set_TxPreamble_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PUCHAR			arg);

INT Set_RTSThreshold_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PUCHAR			arg);

INT Set_FragThreshold_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PUCHAR			arg);

INT Set_TxBurst_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PUCHAR			arg);

#ifdef AGGREGATION_SUPPORT
INT	Set_PktAggregate_Proc(
	IN  PRTMP_ADAPTER		pAd,
	IN  PUCHAR			arg);
#endif

INT	Set_IEEE80211H_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

#ifdef DBG
INT	Set_Debug_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);
#endif

INT	Show_DescInfo_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_ResetStatCounter_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_BASetup_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_BADecline_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_BAOriTearDown_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_BARecTearDown_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_HtBw_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_HtMcs_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_HtGi_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_HtOpMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_HtStbc_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_HtHtc_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_HtExtcha_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_HtMpduDensity_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_HtBaWinSize_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_HtRdg_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_HtLinkAdapt_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_HtAmsdu_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_HtAutoBa_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_HtProtect_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_HtMimoPs_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);


INT	Set_ForceShortGI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_ForceGF_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	SetCommonHT(
	IN	PRTMP_ADAPTER	pAd);

INT	Set_SendPSMPAction_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	Set_HtMIMOPSmode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);


INT	Set_HtTxBASize_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT	    WpaCheckEapCode(
	IN  PRTMP_ADAPTER   	pAd,
	IN  PUCHAR				pFrame,
	IN  USHORT				FrameLen,
	IN  USHORT				OffSet);

VOID    WpaSendMicFailureToWpaSupplicant(
    IN  PRTMP_ADAPTER       pAd,
    IN  BOOLEAN             bUnicast);

int wext_notify_event_assoc(
	IN  RTMP_ADAPTER *pAd);

BOOLEAN STARxDoneInterruptHandle(
	IN	PRTMP_ADAPTER	pAd,
	IN	BOOLEAN			argc);


VOID Indicate_AMPDU_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);


VOID Indicate_AMSDU_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);


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
		if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_INFRA))                              	\
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
	CONVERT_TO_802_3(_pHeader802_3, _pDA, _pSA, _pRxBlk->pData, 				\
		_pRxBlk->DataSize, _pRemovedLLCSNAP);                                   \
}

VOID Sta_Announce_or_Forward_802_3_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	UCHAR			FromWhichBSSID);

#define ANNOUNCE_OR_FORWARD_802_3_PACKET(_pAd, _pPacket, _FromWhichBSS)\
			Sta_Announce_or_Forward_802_3_Packet(_pAd, _pPacket, _FromWhichBSS);
			

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

PNDIS_PACKET RTMPDeFragmentDataFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk);


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
	(1 << IEEE80211_RADIOTAP_RATE)  | 	\
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


struct iw_statistics *rt28xx_get_wireless_stats(
    IN struct net_device *net_dev);

VOID    RTMPSetDesiredRates(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  LONG            Rates);

INT	Set_FixedTxMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

static inline char* GetPhyMode(
	int Mode)
{
	switch(Mode)
	{
		case MODE_CCK:
			return "CCK";

		case MODE_OFDM:
			return "OFDM";
		case MODE_HTMIX:
			return "HTMIX";

		case MODE_HTGREENFIELD:
			return "GREEN";
		default:
			return "N/A";
	}
}


static inline char* GetBW(
	int BW)
{
	switch(BW)
	{
		case BW_10:
			return "10M";

		case BW_20:
			return "20M";
		case BW_40:
			return "40M";
		default:
			return "N/A";
	}
}


VOID RT28xxThreadTerminate(
	IN RTMP_ADAPTER *pAd);

BOOLEAN RT28XXChipsetCheck(
	IN void *_dev_p);

BOOLEAN RT28XXNetDevInit(
	IN void 				*_dev_p,
	IN struct  net_device	*net_dev,
	IN RTMP_ADAPTER 		*pAd);

BOOLEAN RT28XXProbePostConfig(
	IN void 				*_dev_p,
	IN RTMP_ADAPTER 		*pAd,
	IN INT32				argc);

VOID RT28XXDMADisable(
	IN RTMP_ADAPTER 		*pAd);

VOID RT28XXDMAEnable(
	IN RTMP_ADAPTER 		*pAd);

VOID RT28xx_UpdateBeaconToAsic(
	IN RTMP_ADAPTER * pAd,
	IN INT apidx,
	IN ULONG BeaconLen,
	IN ULONG UpdatePos);

INT rt28xx_sta_ioctl(
	IN	struct net_device	*net_dev,
	IN	OUT	struct ifreq	*rq,
	IN	INT			cmd);


PNDIS_PACKET GetPacketFromRxRing(
	IN		PRTMP_ADAPTER	pAd,
	OUT		PRT28XX_RXD_STRUC	pSaveRxD,
	OUT		BOOLEAN			*pbReschedule,
	IN OUT	UINT32			*pRxPending);


void kill_thread_task(PRTMP_ADAPTER pAd);

void tbtt_tasklet(unsigned long data);

#ifdef RT2860



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
	IN RTMP_ADAPTER 	*pAd,
	IN UCHAR 			QueIdx,
	IN PNDIS_PACKET		pPacket,
	IN PUCHAR			pSrcBufVA,
	IN UINT 			SrcBufLen);


NDIS_STATUS RTMPCheckRxError(
	IN  PRTMP_ADAPTER   pAd,
	IN  PHEADER_802_11  pHeader,
	IN	PRXWI_STRUC	pRxWI,
	IN  PRT28XX_RXD_STRUC      pRxD);

VOID RTMPInitPCIeLinkCtrlValue(
	IN	PRTMP_ADAPTER	pAd);

VOID RTMPFindHostPCIDev(
    IN	PRTMP_ADAPTER	pAd);

VOID RTMPPCIeLinkCtrlValueRestore(
	IN	PRTMP_ADAPTER	pAd,
	IN   UCHAR		Level);

VOID RTMPPCIeLinkCtrlSetting(
	IN	PRTMP_ADAPTER	pAd,
	IN 	USHORT		Max);

VOID RT28xxPciAsicRadioOff(
	IN PRTMP_ADAPTER    pAd,
	IN UCHAR            Level,
	IN USHORT           TbttNumToNextWakeUp);

BOOLEAN RT28xxPciAsicRadioOn(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR     Level);

VOID RT28xxPciStaAsicForceWakeup(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR    	 Level);

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

VOID RT28xxPciMlmeRadioOn(
	IN PRTMP_ADAPTER pAd);

VOID RT28xxPciMlmeRadioOFF(
	IN PRTMP_ADAPTER pAd);
#endif 

VOID AsicTurnOffRFClk(
	IN PRTMP_ADAPTER    pAd,
	IN	UCHAR           Channel);

VOID AsicTurnOnRFClk(
	IN PRTMP_ADAPTER 	pAd,
	IN	UCHAR			Channel);

NTSTATUS RT30xxWriteRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			RegID,
	IN	UCHAR			Value);

NTSTATUS RT30xxReadRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			RegID,
	IN	PUCHAR			pValue);

UCHAR eFuseReadRegisters(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	OUT	USHORT* pData);

VOID eFuseReadPhysical(
	IN	PRTMP_ADAPTER	pAd,
  	IN	PUSHORT lpInBuffer,
  	IN	ULONG nInBufferSize,
  	OUT	PUSHORT lpOutBuffer,
  	IN	ULONG nOutBufferSize
);

NTSTATUS eFuseRead(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	OUT	PUCHAR			pData,
	IN	USHORT			Length);

VOID eFusePhysicalWriteRegisters(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	OUT	USHORT* pData);

NTSTATUS eFuseWriteRegisters(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	IN	USHORT* pData);

VOID eFuseWritePhysical(
	IN	PRTMP_ADAPTER	pAd,
  	PUSHORT lpInBuffer,
	ULONG nInBufferSize,
  	PUCHAR lpOutBuffer,
  	ULONG nOutBufferSize
);

NTSTATUS eFuseWrite(
   	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	PUCHAR			pData,
	IN	USHORT			length);

INT set_eFuseGetFreeBlockCount_Proc(
   	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT set_eFusedump_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

INT set_eFuseLoadFromBin_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

NTSTATUS eFuseWriteRegistersFromBin(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	IN	USHORT* pData);

VOID eFusePhysicalReadRegisters(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT Offset,
	IN	USHORT Length,
	OUT	USHORT* pData);

VOID RT30xxLoadRFNormalModeSetup(
	IN PRTMP_ADAPTER 	pAd);

VOID RT30xxLoadRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd);

VOID RT30xxReverseRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd);

#ifdef RT2870



VOID	RTUSBInitTxDesc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTX_CONTEXT		pTxContext,
	IN	UCHAR			BulkOutPipeId,
	IN	usb_complete_t	Func);

VOID	RTUSBInitHTTxDesc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PHT_TX_CONTEXT	pTxContext,
	IN	UCHAR			BulkOutPipeId,
	IN	ULONG			BulkOutSize,
	IN	usb_complete_t	Func);

VOID RTUSBCleanUpDataBulkOutQueue(
	IN	PRTMP_ADAPTER	pAd);

VOID RTUSBCancelPendingBulkOutIRP(
	IN	PRTMP_ADAPTER	pAd);

VOID RTUSBBulkOutDataPacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			BulkOutPipeId,
	IN	UCHAR			Index);

VOID RTUSBBulkOutNullFrame(
	IN	PRTMP_ADAPTER	pAd);

VOID RTUSBBulkOutRTSFrame(
	IN	PRTMP_ADAPTER	pAd);

VOID RTUSBCancelPendingBulkInIRP(
	IN	PRTMP_ADAPTER	pAd);

VOID RTUSBCancelPendingIRPs(
	IN	PRTMP_ADAPTER	pAd);

VOID RTUSBBulkOutMLMEPacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Index);

VOID RTUSBBulkOutPsPoll(
	IN	PRTMP_ADAPTER	pAd);

VOID RTUSBCleanUpMLMEBulkOutQueue(
	IN	PRTMP_ADAPTER	pAd);

VOID RTUSBKickBulkOut(
	IN	PRTMP_ADAPTER pAd);

VOID	RTUSBBulkReceive(
	IN	PRTMP_ADAPTER	pAd);

VOID DoBulkIn(
	IN RTMP_ADAPTER *pAd);

VOID RTUSBInitRxDesc(
	IN	PRTMP_ADAPTER	pAd,
	IN  PRX_CONTEXT		pRxContext);




NTSTATUS RTUSBMultiRead(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	OUT	PUCHAR			pData,
	IN	USHORT			length);

NTSTATUS RTUSBMultiWrite(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	PUCHAR			pData,
	IN	USHORT			length);

NTSTATUS RTUSBMultiWrite_OneByte(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	PUCHAR			pData);

NTSTATUS RTUSBReadBBPRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Id,
	IN	PUCHAR			pValue);

NTSTATUS RTUSBWriteBBPRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Id,
	IN	UCHAR			Value);

NTSTATUS RTUSBWriteRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UINT32			Value);

NTSTATUS RTUSB_VendorRequest(
	IN	PRTMP_ADAPTER	pAd,
	IN	UINT32			TransferFlags,
	IN	UCHAR			ReservedBits,
	IN	UCHAR			Request,
	IN	USHORT			Value,
	IN	USHORT			Index,
	IN	PVOID			TransferBuffer,
	IN	UINT32			TransferBufferLength);

NTSTATUS RTUSBReadEEPROM(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	OUT	PUCHAR			pData,
	IN	USHORT			length);

NTSTATUS RTUSBWriteEEPROM(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	PUCHAR			pData,
	IN	USHORT			length);

VOID RTUSBPutToSleep(
	IN	PRTMP_ADAPTER	pAd);

NTSTATUS RTUSBWakeUp(
	IN	PRTMP_ADAPTER	pAd);

VOID RTUSBInitializeCmdQ(
	IN	PCmdQ	cmdq);

NDIS_STATUS	RTUSBEnqueueCmdFromNdis(
	IN	PRTMP_ADAPTER	pAd,
	IN	NDIS_OID		Oid,
	IN	BOOLEAN			SetInformation,
	IN	PVOID			pInformationBuffer,
	IN	UINT32			InformationBufferLength);

NDIS_STATUS RTUSBEnqueueInternalCmd(
	IN	PRTMP_ADAPTER	pAd,
	IN NDIS_OID			Oid,
	IN PVOID			pInformationBuffer,
	IN UINT32			InformationBufferLength);

VOID RTUSBDequeueCmd(
	IN	PCmdQ		cmdq,
	OUT	PCmdQElmt	*pcmdqelmt);

INT RTUSBCmdThread(
	IN OUT PVOID Context);

INT TimerQThread(
	IN OUT PVOID Context);

RT2870_TIMER_ENTRY *RT2870_TimerQ_Insert(
	IN RTMP_ADAPTER *pAd,
	IN RALINK_TIMER_STRUCT *pTimer);

BOOLEAN RT2870_TimerQ_Remove(
	IN RTMP_ADAPTER *pAd,
	IN RALINK_TIMER_STRUCT *pTimer);

void RT2870_TimerQ_Exit(
	IN RTMP_ADAPTER *pAd);

void RT2870_TimerQ_Init(
	IN RTMP_ADAPTER *pAd);

VOID RT2870_BssBeaconExit(
	IN RTMP_ADAPTER *pAd);

VOID RT2870_BssBeaconStop(
	IN RTMP_ADAPTER *pAd);

VOID RT2870_BssBeaconStart(
	IN RTMP_ADAPTER * pAd);

VOID RT2870_BssBeaconInit(
	IN RTMP_ADAPTER *pAd);

VOID RT2870_WatchDog(
	IN RTMP_ADAPTER *pAd);

NTSTATUS RTUSBWriteMACRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	IN	UINT32			Value);

NTSTATUS RTUSBReadMACRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Offset,
	OUT	PUINT32			pValue);

NTSTATUS RTUSBSingleWrite(
	IN 	RTMP_ADAPTER 	*pAd,
	IN	USHORT			Offset,
	IN	USHORT			Value);

NTSTATUS RTUSBFirmwareRun(
	IN	PRTMP_ADAPTER	pAd);

NTSTATUS RTUSBFirmwareWrite(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR		pFwImage,
	IN ULONG		FwLen);

NTSTATUS	RTUSBFirmwareOpmode(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUINT32			pValue);

NTSTATUS	RTUSBVenderReset(
	IN	PRTMP_ADAPTER	pAd);

VOID CMDHandler(
    IN PRTMP_ADAPTER pAd);


NDIS_STATUS	 CreateThreads(
	IN	struct net_device *net_dev );


VOID MacTableInitialize(
	IN  PRTMP_ADAPTER   pAd);

VOID MlmeSetPsm(
	IN PRTMP_ADAPTER pAd,
	IN USHORT psm);

NDIS_STATUS RTMPWPAAddKeyProc(
	IN  PRTMP_ADAPTER   pAd,
	IN  PVOID           pBuf);

VOID AsicRxAntEvalAction(
	IN PRTMP_ADAPTER pAd);

NDIS_STATUS	RTMPCheckRxError(
	IN	PRTMP_ADAPTER	pAd,
	IN	PHEADER_802_11	pHeader,
	IN	PRXWI_STRUC	pRxWI,
	IN	PRT28XX_RXD_STRUC	pRxINFO);

INT MlmeThread(
	IN PVOID Context);




NDIS_STATUS	RTUSBFreeDescriptorRequest(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			BulkOutPipeId,
	IN	UINT32			NumberRequired);


BOOLEAN	RTUSBNeedQueueBackForAgg(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR		BulkOutPipeId);


VOID RTMPWriteTxInfo(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXINFO_STRUC 	pTxInfo,
	IN	  USHORT		USBDMApktLen,
	IN	  BOOLEAN		bWiv,
	IN	  UCHAR			QueueSel,
	IN	  UCHAR			NextValid,
	IN	  UCHAR			TxBurst);




USHORT RtmpUSB_WriteSubTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	BOOLEAN			bIsLast,
	OUT	USHORT			*FreeNumber);

USHORT RtmpUSB_WriteSingleTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	BOOLEAN			bIsLast,
	OUT	USHORT			*FreeNumber);

USHORT	RtmpUSB_WriteFragTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	UCHAR			fragNum,
	OUT	USHORT			*FreeNumber);

USHORT RtmpUSB_WriteMultiTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	UCHAR			frameNum,
	OUT	USHORT			*FreeNumber);

VOID RtmpUSB_FinalWriteTxResource(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	USHORT			totalMPDUSize,
#ifdef RT2860
	IN	USHORT			FirstTxIdx);
#endif
	IN	USHORT			TxIdx);

VOID RtmpUSBDataLastTxIdx(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			QueIdx,
	IN	USHORT			TxIdx);

VOID RtmpUSBDataKickOut(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	UCHAR			QueIdx);


int RtmpUSBMgmtKickOut(
	IN RTMP_ADAPTER 	*pAd,
	IN UCHAR 			QueIdx,
	IN PNDIS_PACKET		pPacket,
	IN PUCHAR			pSrcBufVA,
	IN UINT 			SrcBufLen);

VOID RtmpUSBNullFrameKickOut(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR		QueIdx,
	IN UCHAR		*pNullFrame,
	IN UINT32		frameLen);

VOID RT28xxUsbStaAsicForceWakeup(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN       bFromTx);

VOID RT28xxUsbStaAsicSleepThenAutoWakeup(
	IN PRTMP_ADAPTER pAd,
	IN USHORT TbttNumToNextWakeUp);

VOID RT28xxUsbMlmeRadioOn(
	IN PRTMP_ADAPTER pAd);

VOID RT28xxUsbMlmeRadioOFF(
	IN PRTMP_ADAPTER pAd);
#endif 

INT RTMPShowCfgValue(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pName,
	IN	PUCHAR			pBuf);

PCHAR   RTMPGetRalinkAuthModeStr(
    IN  NDIS_802_11_AUTHENTICATION_MODE authMode);

PCHAR   RTMPGetRalinkEncryModeStr(
    IN  USHORT encryMode);

VOID AsicStaBbpTuning(
	IN PRTMP_ADAPTER pAd);

#ifdef RT2860
VOID AsicResetFromDMABusy(
	IN PRTMP_ADAPTER pAd);

VOID AsicResetBBP(
	IN PRTMP_ADAPTER pAd);

VOID AsicResetMAC(
	IN PRTMP_ADAPTER pAd);

VOID AsicResetPBF(
	IN PRTMP_ADAPTER pAd);
#endif
#ifdef RT2870
BOOLEAN StaAddMacTableEntry(
	IN  PRTMP_ADAPTER		pAd,
	IN  PMAC_TABLE_ENTRY	pEntry,
	IN  UCHAR				MaxSupportedRateIn500Kbps,
	IN  HT_CAPABILITY_IE	*pHtCapability,
	IN  UCHAR				HtCapabilityLen,
	IN  USHORT        		CapabilityInfo);
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

__inline INT VIRTUAL_IF_UP(PRTMP_ADAPTER pAd)
{
	if (VIRTUAL_IF_NUM(pAd) == 0)
	{
		if (rt28xx_open(pAd->net_dev) != 0)
			return -1;
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

