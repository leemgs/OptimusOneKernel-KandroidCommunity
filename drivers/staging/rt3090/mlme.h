
#ifndef __MLME_H__
#define __MLME_H__

#include "rtmp_dot11.h"

#ifdef CONFIG_STA_SUPPORT
#endif 




#define SUPPORTED_CAPABILITY_INFO   0x0533

#define END_OF_ARGS                 -1
#define LFSR_MASK                   0x80000057
#define MLME_TASK_EXEC_INTV         100       
#define LEAD_TIME                   5
#define MLME_TASK_EXEC_MULTIPLE       10         
#define REORDER_EXEC_INTV		100       



#define CE		0
#define FCC		1
#define JAP		2
#define JAP_W53	3
#define JAP_W56	4
#define MAX_RD_REGION 5

#define BEACON_LOST_TIME            4 * OS_HZ    

#define DLS_TIMEOUT                 1200      
#define AUTH_TIMEOUT                300       
#define ASSOC_TIMEOUT               300       
#define JOIN_TIMEOUT                2000        
#define SHORT_CHANNEL_TIME          90        
#define MIN_CHANNEL_TIME            110        
#define MAX_CHANNEL_TIME            140       
#define	FAST_ACTIVE_SCAN_TIME	    30		  
#define CW_MIN_IN_BITS              4         
#define LINK_DOWN_TIMEOUT           20000      
#define AUTO_WAKEUP_TIMEOUT			70			


#ifdef CONFIG_STA_SUPPORT
#define CW_MAX_IN_BITS              10        
#endif 

#ifdef CONFIG_APSTA_MIXED_SUPPORT
extern UINT32 CW_MAX_IN_BITS;
#endif 




#define RSSI_FOR_MID_TX_POWER       -55  
#define RSSI_FOR_LOW_TX_POWER       -45  
                                        
#define RSSI_FOR_LOWEST_TX_POWER    -30

#define LOW_TX_POWER_DELTA          6    
#define LOWEST_TX_POWER_DELTA       16   

#define RSSI_TRIGGERED_UPON_BELOW_THRESHOLD     0
#define RSSI_TRIGGERED_UPON_EXCCEED_THRESHOLD   1
#define RSSI_THRESHOLD_FOR_ROAMING              25
#define RSSI_DELTA                              5


#define CQI_IS_GOOD(cqi)            ((cqi) >= 50)

#define CQI_IS_POOR(cqi)            (cqi < 50)  
#define CQI_IS_BAD(cqi)             (cqi < 5)
#define CQI_IS_DEAD(cqi)            (cqi == 0)


#define RSSI_WEIGHTING                   50
#define TX_WEIGHTING                     30
#define RX_WEIGHTING                     20








#define BSS_NOT_FOUND                    0xFFFFFFFF


#ifdef CONFIG_STA_SUPPORT
#define MAX_LEN_OF_MLME_QUEUE            40 
#endif 

#define SCAN_PASSIVE                     18		
#define SCAN_ACTIVE                      19		
#define	SCAN_CISCO_PASSIVE				 20		
#define	SCAN_CISCO_ACTIVE				 21		
#define	SCAN_CISCO_NOISE				 22		
#define	SCAN_CISCO_CHANNEL_LOAD			 23		
#define FAST_SCAN_ACTIVE                 24		

#ifdef DOT11N_DRAFT3
#define SCAN_2040_BSS_COEXIST                  26
#endif 


#define MAC_ADDR_IS_GROUP(Addr)       (((Addr[0]) & 0x01))
#define MAC_ADDR_HASH(Addr)            (Addr[0] ^ Addr[1] ^ Addr[2] ^ Addr[3] ^ Addr[4] ^ Addr[5])
#define MAC_ADDR_HASH_INDEX(Addr)      (MAC_ADDR_HASH(Addr) % HASH_TABLE_SIZE)
#define TID_MAC_HASH(Addr,TID)            (TID^Addr[0] ^ Addr[1] ^ Addr[2] ^ Addr[3] ^ Addr[4] ^ Addr[5])
#define TID_MAC_HASH_INDEX(Addr,TID)      (TID_MAC_HASH(Addr,TID) % HASH_TABLE_SIZE)




#define ASIC_LED_ACT_ON(pAd)        RTMP_IO_WRITE32(pAd, MAC_CSR14, 0x00031e46)
#define ASIC_LED_ACT_OFF(pAd)       RTMP_IO_WRITE32(pAd, MAC_CSR14, 0x00001e46)


#define CAP_IS_ESS_ON(x)                 (((x) & 0x0001) != 0)
#define CAP_IS_IBSS_ON(x)                (((x) & 0x0002) != 0)
#define CAP_IS_CF_POLLABLE_ON(x)         (((x) & 0x0004) != 0)
#define CAP_IS_CF_POLL_REQ_ON(x)         (((x) & 0x0008) != 0)
#define CAP_IS_PRIVACY_ON(x)             (((x) & 0x0010) != 0)
#define CAP_IS_SHORT_PREAMBLE_ON(x)      (((x) & 0x0020) != 0)
#define CAP_IS_PBCC_ON(x)                (((x) & 0x0040) != 0)
#define CAP_IS_AGILITY_ON(x)             (((x) & 0x0080) != 0)
#define CAP_IS_SPECTRUM_MGMT(x)          (((x) & 0x0100) != 0)  
#define CAP_IS_QOS(x)                    (((x) & 0x0200) != 0)  
#define CAP_IS_SHORT_SLOT(x)             (((x) & 0x0400) != 0)
#define CAP_IS_APSD(x)                   (((x) & 0x0800) != 0)  
#define CAP_IS_IMMED_BA(x)               (((x) & 0x1000) != 0)  
#define CAP_IS_DSSS_OFDM(x)              (((x) & 0x2000) != 0)
#define CAP_IS_DELAY_BA(x)               (((x) & 0x4000) != 0)  

#define CAP_GENERATE(ess,ibss,priv,s_pre,s_slot,spectrum)  (((ess) ? 0x0001 : 0x0000) | ((ibss) ? 0x0002 : 0x0000) | ((priv) ? 0x0010 : 0x0000) | ((s_pre) ? 0x0020 : 0x0000) | ((s_slot) ? 0x0400 : 0x0000) | ((spectrum) ? 0x0100 : 0x0000))



#define ERP_IS_NON_ERP_PRESENT(x)        (((x) & 0x01) != 0)    
#define ERP_IS_USE_PROTECTION(x)         (((x) & 0x02) != 0)    
#define ERP_IS_USE_BARKER_PREAMBLE(x)    (((x) & 0x04) != 0)    

#define DRS_TX_QUALITY_WORST_BOUND       8
#define DRS_PENALTY                      8

#define BA_NOTUSE	2

#define IMMED_BA	1
#define DELAY_BA	0


#define ORIGINATOR	1
#define RECIPIENT	0


#define ADDBA_RESULTCODE_SUCCESS					0
#define ADDBA_RESULTCODE_REFUSED					37
#define ADDBA_RESULTCODE_INVALID_PARAMETERS			38


#define DELBA_REASONCODE_QSTA_LEAVING				36
#define DELBA_REASONCODE_END_BA						37
#define DELBA_REASONCODE_UNKNOWN_BA					38
#define DELBA_REASONCODE_TIMEOUT					39


#define RESET_ONE_SEC_TX_CNT(__pEntry) \
if (((__pEntry)) != NULL) \
{ \
	(__pEntry)->OneSecTxRetryOkCount = 0; \
	(__pEntry)->OneSecTxFailCount = 0; \
	(__pEntry)->OneSecTxNoRetryOkCount = 0; \
}





typedef struct PACKED {
#ifdef RT_BIG_ENDIAN
	USHORT	LSIGTxopProSup:1;
	USHORT	Forty_Mhz_Intolerant:1;
	USHORT	PSMP:1;
	USHORT	CCKmodein40:1;
	USHORT	AMsduSize:1;
	USHORT	DelayedBA:1;	
	USHORT	RxSTBC:2;
	USHORT	TxSTBC:1;
	USHORT	ShortGIfor40:1;	
	USHORT	ShortGIfor20:1;
	USHORT	GF:1;	
	USHORT	MimoPs:2;
	USHORT	ChannelWidth:1;
	USHORT	AdvCoding:1;
#else
	USHORT	AdvCoding:1;
	USHORT	ChannelWidth:1;
	USHORT	MimoPs:2;
	USHORT	GF:1;	
	USHORT	ShortGIfor20:1;
	USHORT	ShortGIfor40:1;	
	USHORT	TxSTBC:1;
	USHORT	RxSTBC:2;
	USHORT	DelayedBA:1;	
	USHORT	AMsduSize:1;	
	USHORT	CCKmodein40:1;
	USHORT	PSMP:1;
	USHORT	Forty_Mhz_Intolerant:1;
	USHORT	LSIGTxopProSup:1;
#endif	
} HT_CAP_INFO, *PHT_CAP_INFO;


typedef struct PACKED {
#ifdef RT_BIG_ENDIAN
	UCHAR	rsv:3;
	UCHAR	MpduDensity:3;
	UCHAR	MaxRAmpduFactor:2;
#else
	UCHAR	MaxRAmpduFactor:2;
	UCHAR	MpduDensity:3;
	UCHAR	rsv:3;
#endif 
} HT_CAP_PARM, *PHT_CAP_PARM;


typedef struct PACKED {
	UCHAR	MCSSet[10];
	UCHAR	SupRate[2];  
#ifdef RT_BIG_ENDIAN
	UCHAR	rsv:3;
	UCHAR	MpduDensity:1;
	UCHAR	TxStream:2;
	UCHAR	TxRxNotEqual:1;
	UCHAR	TxMCSSetDefined:1;
#else
	UCHAR	TxMCSSetDefined:1;
	UCHAR	TxRxNotEqual:1;
	UCHAR	TxStream:2;
	UCHAR	MpduDensity:1;
	UCHAR	rsv:3;
#endif 
	UCHAR	rsv3[3];
} HT_MCS_SET, *PHT_MCS_SET;


typedef struct PACKED {
#ifdef RT_BIG_ENDIAN
	USHORT	rsv2:4;
	USHORT	RDGSupport:1;	
	USHORT	PlusHTC:1;	
	USHORT	MCSFeedback:2;	
	USHORT	rsv:5;
	USHORT	TranTime:2;
	USHORT	Pco:1;
#else
	USHORT	Pco:1;
	USHORT	TranTime:2;
	USHORT	rsv:5;
	USHORT	MCSFeedback:2;	
	USHORT	PlusHTC:1;	
	USHORT	RDGSupport:1;	
	USHORT	rsv2:4;
#endif 
} EXT_HT_CAP_INFO, *PEXT_HT_CAP_INFO;


typedef struct PACKED _HT_BF_CAP{
#ifdef RT_BIG_ENDIAN
	ULONG	rsv:3;
	ULONG	ChanEstimation:2;
	ULONG	CSIRowBFSup:2;
	ULONG	ComSteerBFAntSup:2;
	ULONG	NoComSteerBFAntSup:2;
	ULONG	CSIBFAntSup:2;
	ULONG	MinGrouping:2;
	ULONG	ExpComBF:2;
	ULONG	ExpNoComBF:2;
	ULONG	ExpCSIFbk:2;
	ULONG	ExpComSteerCapable:1;
	ULONG	ExpNoComSteerCapable:1;
	ULONG	ExpCSICapable:1;
	ULONG	Calibration:2;
	ULONG	ImpTxBFCapable:1;
	ULONG	TxNDPCapable:1;
	ULONG	RxNDPCapable:1;
	ULONG	TxSoundCapable:1;
	ULONG	RxSoundCapable:1;
	ULONG	TxBFRecCapable:1;
#else
	ULONG	TxBFRecCapable:1;
	ULONG	RxSoundCapable:1;
	ULONG	TxSoundCapable:1;
	ULONG	RxNDPCapable:1;
	ULONG	TxNDPCapable:1;
	ULONG	ImpTxBFCapable:1;
	ULONG	Calibration:2;
	ULONG	ExpCSICapable:1;
	ULONG	ExpNoComSteerCapable:1;
	ULONG	ExpComSteerCapable:1;
	ULONG	ExpCSIFbk:2;
	ULONG	ExpNoComBF:2;
	ULONG	ExpComBF:2;
	ULONG	MinGrouping:2;
	ULONG	CSIBFAntSup:2;
	ULONG	NoComSteerBFAntSup:2;
	ULONG	ComSteerBFAntSup:2;
	ULONG	CSIRowBFSup:2;
	ULONG	ChanEstimation:2;
	ULONG	rsv:3;
#endif 
} HT_BF_CAP, *PHT_BF_CAP;


typedef struct PACKED _HT_AS_CAP{
#ifdef RT_BIG_ENDIAN
	UCHAR	rsv:1;
	UCHAR	TxSoundPPDU:1;
	UCHAR	RxASel:1;
	UCHAR	AntIndFbk:1;
	UCHAR	ExpCSIFbk:1;
	UCHAR	AntIndFbkTxASEL:1;
	UCHAR	ExpCSIFbkTxASEL:1;
	UCHAR	AntSelect:1;
#else
	UCHAR	AntSelect:1;
	UCHAR	ExpCSIFbkTxASEL:1;
	UCHAR	AntIndFbkTxASEL:1;
	UCHAR	ExpCSIFbk:1;
	UCHAR	AntIndFbk:1;
	UCHAR	RxASel:1;
	UCHAR	TxSoundPPDU:1;
	UCHAR	rsv:1;
#endif 
} HT_AS_CAP, *PHT_AS_CAP;


#define SIZE_HT_CAP_IE		26

typedef struct PACKED _HT_CAPABILITY_IE{
	HT_CAP_INFO		HtCapInfo;
	HT_CAP_PARM		HtCapParm;

	UCHAR			MCSSet[16];
	EXT_HT_CAP_INFO	ExtHtCapInfo;
	HT_BF_CAP		TxBFCap;	
	HT_AS_CAP		ASCap;	
} HT_CAPABILITY_IE, *PHT_CAPABILITY_IE;




#define dot11OBSSScanPassiveDwell							20	
#define dot11OBSSScanActiveDwell							10	
#define dot11BSSWidthTriggerScanInterval					300  
#define dot11OBSSScanPassiveTotalPerChannel					200	
#define dot11OBSSScanActiveTotalPerChannel					20	
#define dot11BSSWidthChannelTransactionDelayFactor			5	
																
#define dot11BSSScanActivityThreshold						25	
																
																

typedef struct PACKED _OVERLAP_BSS_SCAN_IE{
	USHORT		ScanPassiveDwell;
	USHORT		ScanActiveDwell;
	USHORT		TriggerScanInt;				
	USHORT		PassiveTalPerChannel;		
	USHORT		ActiveTalPerChannel;		
	USHORT		DelayFactor;				
	USHORT		ScanActThre;				
}OVERLAP_BSS_SCAN_IE, *POVERLAP_BSS_SCAN_IE;



typedef union PACKED _BSS_2040_COEXIST_IE{
 struct PACKED {
 #ifdef RT_BIG_ENDIAN
	UCHAR	rsv:5;
	UCHAR	BSS20WidthReq:1;
	UCHAR	Intolerant40:1;
	UCHAR	InfoReq:1;
 #else
	UCHAR	InfoReq:1;
	UCHAR	Intolerant40:1;			
	UCHAR	BSS20WidthReq:1;		
	UCHAR	rsv:5;
#endif 
    } field;
 UCHAR   word;
} BSS_2040_COEXIST_IE, *PBSS_2040_COEXIST_IE;


typedef struct  _TRIGGER_EVENTA{
	BOOLEAN			bValid;
	UCHAR	BSSID[6];
	UCHAR	RegClass;	
	USHORT	Channel;
	ULONG	CDCounter;   
} TRIGGER_EVENTA, *PTRIGGER_EVENTA;



#define MAX_TRIGGER_EVENT		64
typedef struct  _TRIGGER_EVENT_TAB{
	UCHAR	EventANo;
	TRIGGER_EVENTA	EventA[MAX_TRIGGER_EVENT];
	ULONG			EventBCountDown;	
} TRIGGER_EVENT_TAB, *PTRIGGER_EVENT_TAB;



typedef struct PACKED _EXT_CAP_INFO_ELEMENT{
#ifdef RT_BIG_ENDIAN
	UCHAR	rsv2:5;
	UCHAR	ExtendChannelSwitch:1;
	UCHAR	rsv:1;
	UCHAR	BssCoexistMgmtSupport:1;
#else
	UCHAR	BssCoexistMgmtSupport:1;
	UCHAR	rsv:1;
	UCHAR	ExtendChannelSwitch:1;
	UCHAR	rsv2:5;
#endif 
}EXT_CAP_INFO_ELEMENT, *PEXT_CAP_INFO_ELEMENT;



typedef struct PACKED _BSS_2040_COEXIST_ELEMENT{
	UCHAR					ElementID;		
	UCHAR					Len;
	BSS_2040_COEXIST_IE		BssCoexistIe;
}BSS_2040_COEXIST_ELEMENT, *PBSS_2040_COEXIST_ELEMENT;



typedef struct PACKED _BSS_2040_INTOLERANT_CH_REPORT{
	UCHAR				ElementID;		
	UCHAR				Len;
	UCHAR				RegulatoryClass;
	UCHAR				ChList[0];
}BSS_2040_INTOLERANT_CH_REPORT, *PBSS_2040_INTOLERANT_CH_REPORT;



typedef struct PACKED _CHA_SWITCH_ANNOUNCE_IE{
	UCHAR			SwitchMode;	
	UCHAR			NewChannel;	
	UCHAR			SwitchCount;	
} CHA_SWITCH_ANNOUNCE_IE, *PCHA_SWITCH_ANNOUNCE_IE;



typedef struct PACKED _SEC_CHA_OFFSET_IE{
	UCHAR			SecondaryChannelOffset;	 
} SEC_CHA_OFFSET_IE, *PSEC_CHA_OFFSET_IE;



typedef struct {
	BOOLEAN			bHtEnable;	 
	BOOLEAN			bPreNHt;	 
	
	UCHAR			MCSSet[16];
} RT_HT_PHY_INFO, *PRT_HT_PHY_INFO;



typedef struct {
#ifdef RT_BIG_ENDIAN
	USHORT	rsv:5;
	USHORT	AmsduSize:1;	
	USHORT	AmsduEnable:1;	
	USHORT	RxSTBC:2;	
	USHORT	TxSTBC:1;
	USHORT	ShortGIfor40:1;	
	USHORT	ShortGIfor20:1;
	USHORT	GF:1;	
	USHORT	MimoPs:2;
	USHORT	ChannelWidth:1;
#else
	USHORT	ChannelWidth:1;
	USHORT	MimoPs:2;
	USHORT	GF:1;	
	USHORT	ShortGIfor20:1;
	USHORT	ShortGIfor40:1;	
	USHORT	TxSTBC:1;
	USHORT	RxSTBC:2;	
	USHORT	AmsduEnable:1;	
	USHORT	AmsduSize:1;	
	USHORT	rsv:5;
#endif

	
#ifdef RT_BIG_ENDIAN
	UCHAR	RecomWidth:1;
	UCHAR	ExtChanOffset:2;	
	UCHAR	MpduDensity:3;
	UCHAR	MaxRAmpduFactor:2;
#else
	UCHAR	MaxRAmpduFactor:2;
	UCHAR	MpduDensity:3;
	UCHAR	ExtChanOffset:2;	
	UCHAR	RecomWidth:1;
#endif

#ifdef RT_BIG_ENDIAN
	USHORT	rsv2:11;
	USHORT	OBSS_NonHTExist:1;
	USHORT	rsv3:1;
	USHORT	NonGfPresent:1;
	USHORT	OperaionMode:2;
#else
	USHORT	OperaionMode:2;
	USHORT	NonGfPresent:1;
	USHORT	rsv3:1;
	USHORT	OBSS_NonHTExist:1;
	USHORT	rsv2:11;
#endif

	
	UCHAR	NewExtChannelOffset;
	
	UCHAR	BSSCoexist2040;
} RT_HT_CAPABILITY, *PRT_HT_CAPABILITY;


typedef struct PACKED {
#ifdef RT_BIG_ENDIAN
	UCHAR	SerInterGranu:3;
	UCHAR	S_PSMPSup:1;
	UCHAR	RifsMode:1;
	UCHAR	RecomWidth:1;
	UCHAR	ExtChanOffset:2;
#else
	UCHAR	ExtChanOffset:2;
	UCHAR	RecomWidth:1;
	UCHAR	RifsMode:1;
	UCHAR	S_PSMPSup:1;	 
	UCHAR	SerInterGranu:3;	 
#endif
} ADD_HTINFO, *PADD_HTINFO;

typedef struct PACKED{
#ifdef RT_BIG_ENDIAN
	USHORT	rsv2:11;
	USHORT	OBSS_NonHTExist:1;
	USHORT	rsv:1;
	USHORT	NonGfPresent:1;
	USHORT	OperaionMode:2;
#else
	USHORT	OperaionMode:2;
	USHORT	NonGfPresent:1;
	USHORT	rsv:1;
	USHORT	OBSS_NonHTExist:1;
	USHORT	rsv2:11;
#endif
} ADD_HTINFO2, *PADD_HTINFO2;



typedef struct PACKED{
#ifdef RT_BIG_ENDIAN
	USHORT	rsv:4;
	USHORT	PcoPhase:1;
	USHORT	PcoActive:1;
	USHORT	LsigTxopProt:1;
	USHORT	STBCBeacon:1;
	USHORT	DualCTSProtect:1;
	USHORT	DualBeacon:1;
	USHORT	StbcMcs:6;
#else
	USHORT	StbcMcs:6;
	USHORT	DualBeacon:1;
	USHORT	DualCTSProtect:1;
	USHORT	STBCBeacon:1;
	USHORT	LsigTxopProt:1;	
	USHORT	PcoActive:1;
	USHORT	PcoPhase:1;
	USHORT	rsv:4;
#endif 
} ADD_HTINFO3, *PADD_HTINFO3;

#define SIZE_ADD_HT_INFO_IE		22
typedef struct  PACKED{
	UCHAR				ControlChan;
	ADD_HTINFO			AddHtInfo;
	ADD_HTINFO2			AddHtInfo2;
	ADD_HTINFO3			AddHtInfo3;
	UCHAR				MCSSet[16];		
} ADD_HT_INFO_IE, *PADD_HT_INFO_IE;

typedef struct  PACKED{
	UCHAR				NewExtChanOffset;
} NEW_EXT_CHAN_IE, *PNEW_EXT_CHAN_IE;

typedef struct PACKED _FRAME_802_11 {
    HEADER_802_11   Hdr;
    UCHAR            Octet[1];
}   FRAME_802_11, *PFRAME_802_11;


typedef struct PACKED _MA_BODY {
    UCHAR            Category;
    UCHAR            Action;
    UCHAR            Octet[1];
}   MA_BODY, *PMA_BODY;

typedef	struct	PACKED _HEADER_802_3	{
    UCHAR           DAAddr1[MAC_ADDR_LEN];
    UCHAR           SAAddr2[MAC_ADDR_LEN];
    UCHAR           Octet[2];
}	HEADER_802_3, *PHEADER_802_3;


typedef struct PACKED{
#ifdef RT_BIG_ENDIAN
    USHORT      TID:4;	
    USHORT      Initiator:1;	
    USHORT      Rsv:11;	
#else
    USHORT      Rsv:11;	
    USHORT      Initiator:1;	
    USHORT      TID:4;	
#endif 
} DELBA_PARM, *PDELBA_PARM;


typedef struct PACKED {
#ifdef RT_BIG_ENDIAN
    USHORT      BufSize:10;	
    USHORT      TID:4;	
    USHORT      BAPolicy:1;	
    USHORT      AMSDUSupported:1;	
#else
    USHORT      AMSDUSupported:1;	
    USHORT      BAPolicy:1;	
    USHORT      TID:4;	
    USHORT      BufSize:10;	
#endif 
} BA_PARM, *PBA_PARM;


typedef union PACKED {
    struct PACKED {
#ifdef RT_BIG_ENDIAN
    USHORT      StartSeq:12;   
	USHORT      FragNum:4;	
#else
    USHORT      FragNum:4;	
	USHORT      StartSeq:12;   
#endif 
    }   field;
    USHORT           word;
} BASEQ_CONTROL, *PBASEQ_CONTROL;



typedef struct PACKED {
#ifdef RT_BIG_ENDIAN
    USHORT      TID:4;
    USHORT      Rsv:9;
    USHORT      Compressed:1;
    USHORT      MTID:1;		
    USHORT      ACKPolicy:1; 
#else
    USHORT      ACKPolicy:1; 
    USHORT      MTID:1;		
    USHORT      Compressed:1;
    USHORT      Rsv:9;
    USHORT      TID:4;
#endif 
} BA_CONTROL, *PBA_CONTROL;


typedef struct PACKED {
#ifdef RT_BIG_ENDIAN
    USHORT      TID:4;
    USHORT      Rsv1:9;
    USHORT      Compressed:1;
    USHORT      MTID:1;		
    USHORT      ACKPolicy:1;
#else
    USHORT      ACKPolicy:1; 
    USHORT      MTID:1;		
    USHORT      Compressed:1;
    USHORT      Rsv1:9;
    USHORT      TID:4;
#endif 
} BAR_CONTROL, *PBAR_CONTROL;


typedef struct PACKED {
#ifdef RT_BIG_ENDIAN
    USHORT      NumTID:4;
    USHORT      Rsv1:9;
    USHORT      Compressed:1;
    USHORT      MTID:1;
    USHORT      ACKPolicy:1;
#else
    USHORT      ACKPolicy:1;
    USHORT      MTID:1;
    USHORT      Compressed:1;
    USHORT      Rsv1:9;
    USHORT      NumTID:4;
#endif 
} MTBAR_CONTROL, *PMTBAR_CONTROL;

typedef struct PACKED {
#ifdef RT_BIG_ENDIAN
    USHORT      TID:4;
    USHORT      Rsv1:12;
#else
    USHORT      Rsv1:12;
    USHORT      TID:4;
#endif 
} PER_TID_INFO, *PPER_TID_INFO;

typedef struct {
	PER_TID_INFO      PerTID;
	BASEQ_CONTROL	 BAStartingSeq;
} EACH_TID, *PEACH_TID;



typedef struct PACKED _FRAME_BA_REQ {
	FRAME_CONTROL   FC;
	USHORT          Duration;
	UCHAR           Addr1[MAC_ADDR_LEN];
	UCHAR           Addr2[MAC_ADDR_LEN];
	BAR_CONTROL  BARControl;
	BASEQ_CONTROL	 BAStartingSeq;
}   FRAME_BA_REQ, *PFRAME_BA_REQ;

typedef struct PACKED _FRAME_MTBA_REQ {
	FRAME_CONTROL   FC;
	USHORT          Duration;
	UCHAR           Addr1[MAC_ADDR_LEN];
	UCHAR           Addr2[MAC_ADDR_LEN];
	MTBAR_CONTROL  MTBARControl;
	PER_TID_INFO	PerTIDInfo;
	BASEQ_CONTROL	 BAStartingSeq;
}   FRAME_MTBA_REQ, *PFRAME_MTBA_REQ;


typedef struct PACKED _FRAME_MTBA {
	FRAME_CONTROL   FC;
	USHORT          Duration;
	UCHAR           Addr1[MAC_ADDR_LEN];
	UCHAR           Addr2[MAC_ADDR_LEN];
	BA_CONTROL  BAControl;
	BASEQ_CONTROL	 BAStartingSeq;
	UCHAR		BitMap[8];
}   FRAME_MTBA, *PFRAME_MTBA;

typedef struct PACKED _FRAME_PSMP_ACTION {
	HEADER_802_11   Hdr;
	UCHAR	Category;
	UCHAR	Action;
	UCHAR	Psmp;	
}   FRAME_PSMP_ACTION, *PFRAME_PSMP_ACTION;

typedef struct PACKED _FRAME_ACTION_HDR {
	HEADER_802_11   Hdr;
	UCHAR	Category;
	UCHAR	Action;
}   FRAME_ACTION_HDR, *PFRAME_ACTION_HDR;



typedef struct PACKED _CHAN_SWITCH_ANNOUNCE {
	UCHAR					ElementID;	
	UCHAR					Len;
	CHA_SWITCH_ANNOUNCE_IE	CSAnnounceIe;
}   CHAN_SWITCH_ANNOUNCE, *PCHAN_SWITCH_ANNOUNCE;



typedef struct PACKED _SECOND_CHAN_OFFSET {
	UCHAR				ElementID;		
	UCHAR				Len;
	SEC_CHA_OFFSET_IE	SecChOffsetIe;
}   SECOND_CHAN_OFFSET, *PSECOND_CHAN_OFFSET;


typedef struct PACKED _FRAME_SPETRUM_CS {
	HEADER_802_11   Hdr;
	UCHAR	Category;
	UCHAR	Action;
	CHAN_SWITCH_ANNOUNCE	CSAnnounce;
	SECOND_CHAN_OFFSET		SecondChannel;
}   FRAME_SPETRUM_CS, *PFRAME_SPETRUM_CS;


typedef struct PACKED _FRAME_ADDBA_REQ {
	HEADER_802_11   Hdr;
	UCHAR	Category;
	UCHAR	Action;
	UCHAR	Token;	
	BA_PARM		BaParm;	      
	USHORT		TimeOutValue;	
	BASEQ_CONTROL	BaStartSeq; 
}   FRAME_ADDBA_REQ, *PFRAME_ADDBA_REQ;

typedef struct PACKED _FRAME_ADDBA_RSP {
	HEADER_802_11   Hdr;
	UCHAR	Category;
	UCHAR	Action;
	UCHAR	Token;
	USHORT	StatusCode;
	BA_PARM		BaParm; 
	USHORT		TimeOutValue;
}   FRAME_ADDBA_RSP, *PFRAME_ADDBA_RSP;

typedef struct PACKED _FRAME_DELBA_REQ {
	HEADER_802_11   Hdr;
	UCHAR	Category;
	UCHAR	Action;
	DELBA_PARM		DelbaParm;
	USHORT	ReasonCode;
}   FRAME_DELBA_REQ, *PFRAME_DELBA_REQ;



typedef struct PACKED _FRAME_BAR {
	FRAME_CONTROL   FC;
	USHORT          Duration;
	UCHAR           Addr1[MAC_ADDR_LEN];
	UCHAR           Addr2[MAC_ADDR_LEN];
	BAR_CONTROL		BarControl;
	BASEQ_CONTROL	StartingSeq;
}   FRAME_BAR, *PFRAME_BAR;


typedef struct PACKED _FRAME_BA {
	FRAME_CONTROL   FC;
	USHORT          Duration;
	UCHAR           Addr1[MAC_ADDR_LEN];
	UCHAR           Addr2[MAC_ADDR_LEN];
	BAR_CONTROL		BarControl;
	BASEQ_CONTROL	StartingSeq;
	UCHAR		bitmask[8];
}   FRAME_BA, *PFRAME_BA;



typedef struct PACKED _FRAME_RM_REQ_ACTION {
	HEADER_802_11   Hdr;
	UCHAR	Category;
	UCHAR	Action;
	UCHAR	Token;
	USHORT	Repetition;
	UCHAR   data[0];
}   FRAME_RM_REQ_ACTION, *PFRAME_RM_REQ_ACTION;

typedef struct PACKED {
	UCHAR		ID;
	UCHAR		Length;
	UCHAR		ChannelSwitchMode;
	UCHAR		NewRegClass;
	UCHAR		NewChannelNum;
	UCHAR		ChannelSwitchCount;
} HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE, *PHT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE;






#define SEQ_STEPONE(_SEQ1, _SEQ2, _Limit)	((_SEQ1 == ((_SEQ2+1) & _Limit)))
#define SEQ_SMALLER(_SEQ1, _SEQ2, _Limit)	(((_SEQ1-_SEQ2) & ((_Limit+1)>>1)))
#define SEQ_LARGER(_SEQ1, _SEQ2, _Limit)	((_SEQ1 != _SEQ2) && !(((_SEQ1-_SEQ2) & ((_Limit+1)>>1))))
#define SEQ_WITHIN_WIN(_SEQ1, _SEQ2, _WIN, _Limit) (SEQ_LARGER(_SEQ1, _SEQ2, _Limit) &&  \
												SEQ_SMALLER(_SEQ1, ((_SEQ2+_WIN+1)&_Limit), _Limit))




typedef struct PACKED {
    BOOLEAN     bValid;         
    UCHAR       CfpCount;
    UCHAR       CfpPeriod;
    USHORT      CfpMaxDuration;
    USHORT      CfpDurRemaining;
} CF_PARM, *PCF_PARM;

typedef	struct	_CIPHER_SUITE	{
	NDIS_802_11_ENCRYPTION_STATUS	PairCipher;		
	NDIS_802_11_ENCRYPTION_STATUS	PairCipherAux;	
	NDIS_802_11_ENCRYPTION_STATUS	GroupCipher;	
	USHORT							RsnCapability;	
	BOOLEAN							bMixMode;		
}	CIPHER_SUITE, *PCIPHER_SUITE;


typedef struct {
    BOOLEAN     bValid;         
    BOOLEAN     bAdd;         
    BOOLEAN     bQAck;
    BOOLEAN     bQueueRequest;
    BOOLEAN     bTxopRequest;
    BOOLEAN     bAPSDCapable;

    UCHAR       EdcaUpdateCount;
    UCHAR       Aifsn[4];       
    UCHAR       Cwmin[4];
    UCHAR       Cwmax[4];
    USHORT      Txop[4];      
    BOOLEAN     bACM[4];      
} EDCA_PARM, *PEDCA_PARM;


typedef struct {
    BOOLEAN     bValid;                     
    USHORT      StaNum;
    UCHAR       ChannelUtilization;
    USHORT      RemainingAdmissionControl;  
} QBSS_LOAD_PARM, *PQBSS_LOAD_PARM;


typedef struct PACKED {
#ifdef RT_BIG_ENDIAN
	UCHAR		Rsv2:1;
	UCHAR		MaxSPLength:2;
	UCHAR		Rsv1:1;
	UCHAR		UAPSD_AC_BE:1;
	UCHAR		UAPSD_AC_BK:1;
	UCHAR		UAPSD_AC_VI:1;
	UCHAR		UAPSD_AC_VO:1;
#else
    UCHAR		UAPSD_AC_VO:1;
	UCHAR		UAPSD_AC_VI:1;
	UCHAR		UAPSD_AC_BK:1;
	UCHAR		UAPSD_AC_BE:1;
	UCHAR		Rsv1:1;
	UCHAR		MaxSPLength:2;
	UCHAR		Rsv2:1;
#endif 
} QBSS_STA_INFO_PARM, *PQBSS_STA_INFO_PARM;


typedef struct PACKED {
#ifdef RT_BIG_ENDIAN
	UCHAR		UAPSD:1;
	UCHAR		Rsv:3;
    UCHAR		ParamSetCount:4;
#else
    UCHAR		ParamSetCount:4;
	UCHAR		Rsv:3;
	UCHAR		UAPSD:1;
#endif 
} QBSS_AP_INFO_PARM, *PQBSS_AP_INFO_PARM;



typedef struct {
    BOOLEAN     bValid;                     
    BOOLEAN     bQAck;
    BOOLEAN     bQueueRequest;
    BOOLEAN     bTxopRequest;

    UCHAR       EdcaUpdateCount;
} QOS_CAPABILITY_PARM, *PQOS_CAPABILITY_PARM;

#ifdef CONFIG_STA_SUPPORT
typedef struct {
    UCHAR       IELen;
    UCHAR       IE[MAX_CUSTOM_LEN];
} WPA_IE_;
#endif 


typedef struct {
    UCHAR   Bssid[MAC_ADDR_LEN];
    UCHAR   Channel;
	UCHAR   CentralChannel;	
    UCHAR   BssType;
    USHORT  AtimWin;
    USHORT  BeaconPeriod;

    UCHAR   SupRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR   SupRateLen;
    UCHAR   ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
    UCHAR   ExtRateLen;
	HT_CAPABILITY_IE HtCapability;
	UCHAR			HtCapabilityLen;
	ADD_HT_INFO_IE AddHtInfo;	
	UCHAR			AddHtInfoLen;
	UCHAR			NewExtChanOffset;
	CHAR    Rssi;
    UCHAR   Privacy;			
	UCHAR	Hidden;

    USHORT  DtimPeriod;
    USHORT  CapabilityInfo;

    USHORT  CfpCount;
    USHORT  CfpPeriod;
    USHORT  CfpMaxDuration;
    USHORT  CfpDurRemaining;
    UCHAR   SsidLen;
    CHAR    Ssid[MAX_LEN_OF_SSID];

    ULONG   LastBeaconRxTime; 

	BOOLEAN	bSES;

	
	CIPHER_SUITE					WPA;			
	CIPHER_SUITE					WPA2;			

	
	NDIS_802_11_FIXED_IEs	FixIEs;
	NDIS_802_11_AUTHENTICATION_MODE	AuthModeAux;	
	NDIS_802_11_AUTHENTICATION_MODE	AuthMode;
	NDIS_802_11_WEP_STATUS	WepStatus;				
	USHORT					VarIELen;				
	UCHAR					VarIEs[MAX_VIE_LEN];

	
    UCHAR   CkipFlag;

	
	UCHAR	PTSF[4];		
	UCHAR	TTSF[8];		

    
	EDCA_PARM           EdcaParm;
	QOS_CAPABILITY_PARM QosCapability;
	QBSS_LOAD_PARM      QbssLoad;
#ifdef CONFIG_STA_SUPPORT
    WPA_IE_     WpaIE;
    WPA_IE_     RsnIE;
#ifdef EXT_BUILD_CHANNEL_LIST
	UCHAR		CountryString[3];
	BOOLEAN		bHasCountryIE;
#endif 
#endif 

} BSS_ENTRY, *PBSS_ENTRY;

typedef struct {
    UCHAR           BssNr;
    UCHAR           BssOverlapNr;
    BSS_ENTRY       BssEntry[MAX_LEN_OF_BSS_TABLE];
} BSS_TABLE, *PBSS_TABLE;


typedef struct _MLME_QUEUE_ELEM {
    ULONG             Machine;
    ULONG             MsgType;
    ULONG             MsgLen;
    UCHAR             Msg[MGMT_DMA_BUFFER_SIZE];
    LARGE_INTEGER     TimeStamp;
    UCHAR             Rssi0;
    UCHAR             Rssi1;
    UCHAR             Rssi2;
    UCHAR             Signal;
    UCHAR             Channel;
    UCHAR             Wcid;
    BOOLEAN           Occupied;
#ifdef MLME_EX
	USHORT            Idx;
#endif 
} MLME_QUEUE_ELEM, *PMLME_QUEUE_ELEM;

typedef struct _MLME_QUEUE {
    ULONG             Num;
    ULONG             Head;
    ULONG             Tail;
    NDIS_SPIN_LOCK   Lock;
    MLME_QUEUE_ELEM  Entry[MAX_LEN_OF_MLME_QUEUE];
} MLME_QUEUE, *PMLME_QUEUE;

typedef VOID (*STATE_MACHINE_FUNC)(VOID *Adaptor, MLME_QUEUE_ELEM *Elem);

typedef struct _STATE_MACHINE {
    ULONG                           Base;
    ULONG                           NrState;
    ULONG                           NrMsg;
    ULONG                           CurrState;
    STATE_MACHINE_FUNC             *TransFunc;
} STATE_MACHINE, *PSTATE_MACHINE;








typedef struct _MLME_AUX {
    UCHAR               BssType;
    UCHAR               Ssid[MAX_LEN_OF_SSID];
    UCHAR               SsidLen;
    UCHAR               Bssid[MAC_ADDR_LEN];
	UCHAR				AutoReconnectSsid[MAX_LEN_OF_SSID];
	UCHAR				AutoReconnectSsidLen;
    USHORT              Alg;
    UCHAR               ScanType;
    UCHAR               Channel;
	UCHAR               CentralChannel;
    USHORT              Aid;
    USHORT              CapabilityInfo;
    USHORT              BeaconPeriod;
    USHORT              CfpMaxDuration;
    USHORT              CfpPeriod;
    USHORT              AtimWin;

	
	
	UCHAR		        SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR		        ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR		        SupRateLen;
	UCHAR		        ExtRateLen;
	HT_CAPABILITY_IE		HtCapability;
	UCHAR				HtCapabilityLen;
	ADD_HT_INFO_IE		AddHtInfo;	
	UCHAR			NewExtChannelOffset;
	

    
    QOS_CAPABILITY_PARM APQosCapability;    
    EDCA_PARM           APEdcaParm;         
    QBSS_LOAD_PARM      APQbssLoad;         

    
    ULONG               APRalinkIe;

    BSS_TABLE           SsidBssTab;     
    BSS_TABLE           RoamTab;        
    ULONG               BssIdx;
    ULONG               RoamIdx;

	BOOLEAN				CurrReqIsFromNdis;

    RALINK_TIMER_STRUCT BeaconTimer, ScanTimer;
    RALINK_TIMER_STRUCT AuthTimer;
    RALINK_TIMER_STRUCT AssocTimer, ReassocTimer, DisassocTimer;

#ifdef CONFIG_STA_SUPPORT
#endif 
} MLME_AUX, *PMLME_AUX;

typedef struct _MLME_ADDBA_REQ_STRUCT{
	UCHAR   Wcid;	
	UCHAR   pAddr[MAC_ADDR_LEN];
	UCHAR   BaBufSize;
	USHORT	TimeOutValue;
	UCHAR   TID;
	UCHAR   Token;
	USHORT	BaStartSeq;
} MLME_ADDBA_REQ_STRUCT, *PMLME_ADDBA_REQ_STRUCT;


typedef struct _MLME_DELBA_REQ_STRUCT{
	UCHAR   Wcid;	
	UCHAR     Addr[MAC_ADDR_LEN];
	UCHAR   TID;
	UCHAR	Initiator;
} MLME_DELBA_REQ_STRUCT, *PMLME_DELBA_REQ_STRUCT;


typedef struct _MLME_ASSOC_REQ_STRUCT{
    UCHAR     Addr[MAC_ADDR_LEN];
    USHORT    CapabilityInfo;
    USHORT    ListenIntv;
    ULONG     Timeout;
} MLME_ASSOC_REQ_STRUCT, *PMLME_ASSOC_REQ_STRUCT, MLME_REASSOC_REQ_STRUCT, *PMLME_REASSOC_REQ_STRUCT;

typedef struct _MLME_DISASSOC_REQ_STRUCT{
    UCHAR     Addr[MAC_ADDR_LEN];
    USHORT    Reason;
} MLME_DISASSOC_REQ_STRUCT, *PMLME_DISASSOC_REQ_STRUCT;

typedef struct _MLME_AUTH_REQ_STRUCT {
    UCHAR        Addr[MAC_ADDR_LEN];
    USHORT       Alg;
    ULONG        Timeout;
} MLME_AUTH_REQ_STRUCT, *PMLME_AUTH_REQ_STRUCT;

typedef struct _MLME_DEAUTH_REQ_STRUCT {
    UCHAR        Addr[MAC_ADDR_LEN];
    USHORT       Reason;
} MLME_DEAUTH_REQ_STRUCT, *PMLME_DEAUTH_REQ_STRUCT;

typedef struct {
    ULONG      BssIdx;
} MLME_JOIN_REQ_STRUCT;

typedef struct _MLME_SCAN_REQ_STRUCT {
    UCHAR      Bssid[MAC_ADDR_LEN];
    UCHAR      BssType;
    UCHAR      ScanType;
    UCHAR      SsidLen;
    CHAR       Ssid[MAX_LEN_OF_SSID];
} MLME_SCAN_REQ_STRUCT, *PMLME_SCAN_REQ_STRUCT;

typedef struct _MLME_START_REQ_STRUCT {
    CHAR        Ssid[MAX_LEN_OF_SSID];
    UCHAR       SsidLen;
} MLME_START_REQ_STRUCT, *PMLME_START_REQ_STRUCT;

#ifdef CONFIG_STA_SUPPORT
#ifdef QOS_DLS_SUPPORT

typedef struct _RT_802_11_DLS {
	USHORT						TimeOut;		
	USHORT						CountDownTimer;	
	NDIS_802_11_MAC_ADDRESS		MacAddr;		
	UCHAR						Status;			
	BOOLEAN						Valid;			
	RALINK_TIMER_STRUCT			Timer;			
	USHORT						Sequence;
	USHORT						MacTabMatchWCID;	
	BOOLEAN						bHTCap;
	PVOID						pAd;
} RT_802_11_DLS, *PRT_802_11_DLS;

typedef struct _MLME_DLS_REQ_STRUCT {
    PRT_802_11_DLS	pDLS;
    USHORT			Reason;
} MLME_DLS_REQ_STRUCT, *PMLME_DLS_REQ_STRUCT;
#endif 
#endif 

typedef struct PACKED {
    UCHAR   Eid;
    UCHAR   Len;
    UCHAR   Octet[1];
} EID_STRUCT,*PEID_STRUCT, BEACON_EID_STRUCT, *PBEACON_EID_STRUCT;

typedef struct PACKED _RTMP_TX_RATE_SWITCH
{
	UCHAR   ItemNo;
#ifdef RT_BIG_ENDIAN
	UCHAR	Rsv2:2;
	UCHAR	Mode:2;
	UCHAR	Rsv1:1;
	UCHAR	BW:1;
	UCHAR	ShortGI:1;
	UCHAR	STBC:1;
#else
	UCHAR	STBC:1;
	UCHAR	ShortGI:1;
	UCHAR	BW:1;
	UCHAR	Rsv1:1;
	UCHAR	Mode:2;
	UCHAR	Rsv2:2;
#endif
	UCHAR   CurrMCS;
	UCHAR   TrainUp;
	UCHAR   TrainDown;
} RRTMP_TX_RATE_SWITCH, *PRTMP_TX_RATE_SWITCH;


#define TBTT_PRELOAD_TIME       384        
#define DEFAULT_DTIM_PERIOD     1






#define MAC_TABLE_AGEOUT_TIME			300			
#define MAC_TABLE_ASSOC_TIMEOUT			5			
#define MAC_TABLE_FULL(Tab)				((Tab).size == MAX_LEN_OF_MAC_TABLE)


#define MAC_ENTRY_LIFE_CHECK_CNT		20			


typedef enum _Sst {
    SST_NOT_AUTH,   
    SST_AUTH,       
    SST_ASSOC       
} SST;


typedef enum _AuthState {
    AS_NOT_AUTH,
    AS_AUTH_OPEN,       
    AS_AUTH_KEY,        
    AS_AUTHENTICATING   
} AUTH_STATE;


typedef enum _ApWpaState {
    AS_NOTUSE,              
    AS_DISCONNECT,          
    AS_DISCONNECTED,        
    AS_INITIALIZE,          
    AS_AUTHENTICATION,      
    AS_AUTHENTICATION2,     
    AS_INITPMK,             
    AS_INITPSK,             
    AS_PTKSTART,            
    AS_PTKINIT_NEGOTIATING, 
    AS_PTKINITDONE,         
    AS_UPDATEKEYS,          
    AS_INTEGRITY_FAILURE,   
    AS_KEYUPDATE,           
} AP_WPA_STATE;


typedef enum _GTKState {
    REKEY_NEGOTIATING,
    REKEY_ESTABLISHED,
    KEYERROR,
} GTK_STATE;


typedef enum _WpaGTKState {
    SETKEYS,
    SETKEYS_DONE,
} WPA_GTK_STATE;



#endif	
