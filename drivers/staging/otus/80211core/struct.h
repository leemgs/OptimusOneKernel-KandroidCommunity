

#ifndef _STRUCT_H
#define _STRUCT_H

#include "../oal_marc.h"

#define ZM_SW_LOOP_BACK                     0 
#define ZM_PCI_LOOP_BACK                    0 
#define ZM_PROTOCOL_RESPONSE_SIMULATION     0

#define ZM_RX_FRAME_SIZE               1600

extern const u8_t zg11bRateTbl[4];
extern const u8_t zg11gRateTbl[8];

#define ZM_DRIVER_CORE_MAJOR_VERSION        1
#define ZM_DRIVER_CORE_MINOR_VERSION        1
#define ZM_DRIVER_CORE_BRANCH_MAJOR_VERSION 3
#define ZM_DRIVER_CORE_BRANCH_MINOR_VERSION 39

#ifndef ZM_VTXQ_SIZE
#define ZM_VTXQ_SIZE                        1024 
#endif

#define ZM_VTXQ_SIZE_MASK                   (ZM_VTXQ_SIZE-1)
#define ZM_VMMQ_SIZE                        8 
#define ZM_VMMQ_SIZE_MASK                   (ZM_VMMQ_SIZE-1)

#include "cagg.h"

#define ZM_AGG_POOL_SIZE                    20
#define ZM_RATE_TABLE_SIZE                  32

#define ZM_MAX_BUF_DISCRETE_NUMBER          5












#define ZM_IBSS_PEER_ALIVE_COUNTER     4





#define ZM_BIT_0       0x1
#define ZM_BIT_1       0x2
#define ZM_BIT_2       0x4
#define ZM_BIT_3       0x8
#define ZM_BIT_4       0x10
#define ZM_BIT_5       0x20
#define ZM_BIT_6       0x40
#define ZM_BIT_7       0x80
#define ZM_BIT_8       0x100
#define ZM_BIT_9       0x200
#define ZM_BIT_10      0x400
#define ZM_BIT_11      0x800
#define ZM_BIT_12      0x1000
#define ZM_BIT_13      0x2000
#define ZM_BIT_14      0x4000
#define ZM_BIT_15      0x8000
#define ZM_BIT_16      0x10000
#define ZM_BIT_17      0x20000
#define ZM_BIT_18      0x40000
#define ZM_BIT_19      0x80000
#define ZM_BIT_20      0x100000
#define ZM_BIT_21      0x200000
#define ZM_BIT_22      0x400000
#define ZM_BIT_23      0x800000
#define ZM_BIT_24      0x1000000
#define ZM_BIT_25      0x2000000
#define ZM_BIT_26      0x4000000
#define ZM_BIT_27      0x8000000
#define ZM_BIT_28      0x10000000
#define ZM_BIT_29      0x20000000   
#define ZM_BIT_30      0x40000000
#define ZM_BIT_31      0x80000000





#define ZM_MAC_BYTE_TO_WORD(macb, macw)   macw[0] = macb[0] + (macb[1] << 8); \
                                          macw[1] = macb[2] + (macb[3] << 8); \
                                          macw[2] = macb[4] + (macb[5] << 8);

#define ZM_MAC_WORD_TO_BYTE(macw, macb)   macb[0] = (u8_t) (macw[0] & 0xff); \
                                          macb[1] = (u8_t) (macw[0] >> 8);   \
                                          macb[2] = (u8_t) (macw[1] & 0xff); \
                                          macb[3] = (u8_t) (macw[1] >> 8);   \
                                          macb[4] = (u8_t) (macw[2] & 0xff); \
                                          macb[5] = (u8_t) (macw[2] >> 8);

#define ZM_MAC_0(macw)   ((u8_t)(macw[0] & 0xff))
#define ZM_MAC_1(macw)   ((u8_t)(macw[0] >> 8))
#define ZM_MAC_2(macw)   ((u8_t)(macw[1] & 0xff))
#define ZM_MAC_3(macw)   ((u8_t)(macw[1] >> 8))
#define ZM_MAC_4(macw)   ((u8_t)(macw[2] & 0xff))
#define ZM_MAC_5(macw)   ((u8_t)(macw[2] >> 8))

#define ZM_IS_MULTICAST_OR_BROADCAST(mac) (mac[0] & 0x01)
#define ZM_IS_MULTICAST(mac) ((mac[0] & 0x01) && (((u8_t)mac[0]) != 0xFF))

#define ZM_MAC_EQUAL(mac1, mac2)   ((mac1[0]==mac2[0])&&(mac1[1]==mac2[1])&&(mac1[2]==mac2[2]))
#define ZM_MAC_NOT_EQUAL(mac1, mac2)   ((mac1[0]!=mac2[0])||(mac1[1]!=mac2[1])||(mac1[2]!=mac2[2]))



#define ZM_BYTE_TO_WORD(A, B)   ((A<<8)+B)
#define ZM_ROL32( A, n ) \
        ( ((A) << (n)) | ( ((A)>>(32-(n)))  & ( (1UL << (n)) - 1 ) ) )
#define ZM_ROR32( A, n ) ZM_ROL32( (A), 32-(n) )
#define ZM_LO8(v16)  ((u8_t)((v16) & 0xFF))
#define ZM_HI8(v16)  ((u8_t)(((v16)>>8)&0xFF))

#ifdef ZM_ENABLE_BUFFER_TRACE
extern void zfwBufTrace(zdev_t* dev, zbuf_t *buf, u8_t *functionName);
#define ZM_BUFFER_TRACE(dev, buf)       zfwBufTrace(dev, buf, __func__);
#else
#define ZM_BUFFER_TRACE(dev, buf)
#endif


#define ZM_BSSID_LIST_SCAN         0x01


#define ZM_CAM_AP                       0x1
#define ZM_CAM_STA                      0x2
#define ZM_CAM_HOST                     0x4


#define ZM_STA_STATE_DISCONNECT           1
#define ZM_STA_STATE_CONNECTING           2
#define ZM_STA_STATE_CONNECTED            3


#define ZM_EVENT_TIMEOUT_SCAN             0x0000
#define ZM_EVENT_TIMEOUT_BG_SCAN          0x0001
#define ZN_EVENT_TIMEOUT_RECONNECT        0x0002
#define ZM_EVENT_TIMEOUT_INIT_SCAN        0x0003
#define ZM_EVENT_TIMEOUT_AUTH             0x0004
#define ZM_EVENT_TIMEOUT_ASSO             0x0005
#define ZM_EVENT_TIMEOUT_AUTO_SCAN        0x0006
#define ZM_EVENT_TIMEOUT_MIC_FAIL         0x0007
#define ZM_EVENT_TIMEOUT_CHECK_AP         0x0008
#define ZM_EVENT_CONNECT                  0x0009
#define ZM_EVENT_INIT_SCAN                0x000a
#define ZM_EVENT_SCAN                     0x000b
#define ZM_EVENT_BG_SCAN                  0x000c
#define ZM_EVENT_DISCONNECT               0x000d
#define ZM_EVENT_WPA_MIC_FAIL             0x000e
#define ZM_EVENT_AP_ALIVE                 0x000f
#define ZM_EVENT_CHANGE_TO_AP             0x0010
#define ZM_EVENT_CHANGE_TO_STA            0x0011
#define ZM_EVENT_IDLE                     0x0012
#define ZM_EVENT_AUTH                     0x0013
#define ZM_EVENT_ASSO_RSP                 0x0014
#define ZM_EVENT_WPA_PK_OK                0x0015
#define ZM_EVENT_WPA_GK_OK                0x0016
#define ZM_EVENT_RCV_BEACON               0x0017
#define ZM_EVENT_RCV_PROBE_RSP            0x0018
#define ZM_EVENT_SEND_DATA                0x0019
#define ZM_EVENT_AUTO_SCAN                0x001a
#define ZM_EVENT_MIC_FAIL1                0x001d
#define ZM_EVENT_MIC_FAIL2                0x001e
#define ZM_EVENT_IBSS_MONITOR             0x001f
#define ZM_EVENT_IN_SCAN                  0x0020
#define ZM_EVENT_CM_TIMER                 0x0021
#define ZM_EVENT_CM_DISCONNECT            0x0022
#define ZM_EVENT_CM_BLOCK_TIMER           0x0023
#define ZM_EVENT_TIMEOUT_ADDBA            0x0024
#define ZM_EVENT_TIMEOUT_PERFORMANCE      0x0025
#define ZM_EVENT_SKIP_COUNTERMEASURE	  0x0026
#define ZM_EVENT_NONE                     0xffff


#define ZM_ACTION_NONE                    0x0000
#define ZM_ACTION_QUEUE_DATA              0x0001
#define ZM_ACTION_DROP_DATA               0x0002


#define ZM_TICK_ZERO                      0
#define ZM_TICK_INIT_SCAN_END             8
#define ZM_TICK_NEXT_BG_SCAN              50
#define ZM_TICK_BG_SCAN_END               8
#define ZM_TICK_AUTH_TIMEOUT              4
#define ZM_TICK_ASSO_TIMEOUT              4
#define ZM_TICK_AUTO_SCAN                 300
#define ZM_TICK_MIC_FAIL_TIMEOUT          6000
#define ZM_TICK_CHECK_AP1                 150
#define ZM_TICK_CHECK_AP2                 350
#define ZM_TICK_CHECK_AP3                 250
#define ZM_TICK_IBSS_MONITOR              160
#define ZM_TICK_IN_SCAN                   4
#define ZM_TICK_CM_TIMEOUT                6000
#define ZM_TICK_CM_DISCONNECT             200
#define ZM_TICK_CM_BLOCK_TIMEOUT          6000


#ifdef NDIS_CM_FOR_XP
#define ZM_TICK_CM_TIMEOUT_OFFSET        2160
#define ZM_TICK_CM_DISCONNECT_OFFSET     72
#define ZM_TICK_CM_BLOCK_TIMEOUT_OFFSET  2160
#else
#define ZM_TICK_CM_TIMEOUT_OFFSET        0
#define ZM_TICK_CM_DISCONNECT_OFFSET     0
#define ZM_TICK_CM_BLOCK_TIMEOUT_OFFSET  0
#endif

#define ZM_TIME_ACTIVE_SCAN               30 
#define ZM_TIME_PASSIVE_SCAN              110 


#define ZM_STA_CONN_STATE_NONE            0
#define ZM_STA_CONN_STATE_AUTH_OPEN       1
#define ZM_STA_CONN_STATE_AUTH_SHARE_1    2
#define ZM_STA_CONN_STATE_AUTH_SHARE_2    3
#define ZM_STA_CONN_STATE_ASSOCIATE       4
#define ZM_STA_CONN_STATE_SSID_NOT_FOUND  5
#define ZM_STA_CONN_STATE_AUTH_COMPLETED  6


#define ZM_STA_WPA_STATE_INIT             0
#define ZM_STA_WPA_STATE_PK_OK            1
#define ZM_STA_WPA_STATE_GK_OK            2


#define ZM_INTERVAL_CONNECT_TIMEOUT          20   


#define ZM_IBSS_PARTNER_LOST                 0
#define ZM_IBSS_PARTNER_ALIVE                1
#define ZM_IBSS_PARTNER_CHECK                2

#define ZM_BCMC_ARRAY_SIZE                  16 
#define ZM_UNI_ARRAY_SIZE                   16 

#define ZM_MAX_DEFRAG_ENTRIES               4  
#define ZM_DEFRAG_AGING_TIME_SEC            5  

#define ZM_MAX_WPAIE_SIZE                   128

#define ZM_USER_KEY_DEFAULT                 64
#define ZM_USER_KEY_PK                      0                
#define ZM_USER_KEY_GK                      1                

#define ZM_WLAN_TYPE_PURE_B                 2
#define ZM_WLAN_TYPE_PURE_G                 1
#define ZM_WLAN_TYPE_MIXED                  0


#define ZM_HAL_STATE_INIT                   0
#define ZM_HAL_STATE_RUNNING                1


#define ZM_All11N_AP                        0x01
#define ZM_XR_AP                            0x02
#define ZM_SuperG_AP                        0x04


#define ZM_MPDU_DENSITY_NONE                0
#define ZM_MPDU_DENSITY_1_8US               1
#define ZM_MPDU_DENSITY_1_4US               2
#define ZM_MPDU_DENSITY_1_2US               3
#define ZM_MPDU_DENSITY_1US                 4
#define ZM_MPDU_DENSITY_2US                 5
#define ZM_MPDU_DENSITY_4US                 6
#define ZM_MPDU_DENSITY_8US                 7


#define ZM_SW_TKIP_ENCRY_EN                0x01
#define ZM_SW_TKIP_DECRY_EN                0x02
#define ZM_SW_WEP_ENCRY_EN                 0x04
#define ZM_SW_WEP_DECRY_EN                 0x08


#define ZM_DEFAULT_SUPPORT_RATE_ZERO       0x0
#define ZM_DEFAULT_SUPPORT_RATE_DISCONNECT 0x1
#define ZM_DEFAULT_SUPPORT_RATE_IBSS_B     0x2
#define ZM_DEFAULT_SUPPORT_RATE_IBSS_AG    0x3


struct zsTkipSeed
{
    u8_t   tk[32];     
    u8_t   ta[6];
    u16_t  ttak[5];
    u16_t  ppk[6];
    u16_t  iv16,iv16tmp;
    u32_t  iv32,iv32tmp;
};

struct zsMicVar
{
    u32_t  k0, k1;        
    u32_t  left, right;   
    u32_t  m;             
    u16_t  nBytes;        
};

struct zsDefragEntry
{
    u8_t    fragCount;
    u8_t    addr[6];
    u16_t   seqNum;
    zbuf_t* fragment[8];
    u32_t   tick;
};

struct zsDefragList
{
    struct zsDefragEntry   defragEntry[ZM_MAX_DEFRAG_ENTRIES];
    u8_t                   replaceNum;
};

#define ZM_MAX_OPPOSITE_COUNT      16
#define ZM_MAX_TX_SAMPLES          15
#define ZM_TX_RATE_DOWN_CRITERIA   80
#define ZM_TX_RATE_UP_CRITERIA    200


#define ZM_MAX_PROBE_HIDDEN_SSID_SIZE 2
struct zsSsidList
{
    u8_t            ssid[32];
    u8_t            ssidLen;
};

struct zsWrapperSetting
{
    u8_t            bDesiredBssid;
    u8_t            desiredBssid[6];
    u16_t           bssid[3];
    u8_t            ssid[32];
    u8_t            ssidLen;
    u8_t            authMode;
    u8_t            wepStatus;
    u8_t            encryMode;
    u8_t            wlanMode;
    u16_t           frequency;
    u16_t           beaconInterval;
    u8_t            dtim;
    u8_t            preambleType;
    u16_t           atimWindow;

    struct zsSsidList probingSsidList[ZM_MAX_PROBE_HIDDEN_SSID_SIZE];

    u8_t            dropUnencryptedPkts;
    u8_t            ibssJoinOnly;
    u32_t           adhocMode;
    u8_t            countryIsoName[4];
    u16_t           autoSetFrequency;

    
    u8_t            bRateBasic;
    u8_t            gRateBasic;
    u32_t           nRateBasic;
    u8_t            bgMode;

    
    u8_t            staWmeEnabled;
    u8_t            staWmeQosInfo;
    u8_t            apWmeEnabled;


    
};

struct zsWrapperFeatureCtrl
{
    u8_t           bIbssGMode;
};

#define  ZM_MAX_PS_STA            16
#define  ZM_PS_QUEUE_SIZE         32

struct zsStaPSEntity
{
    u8_t           bUsed;
    u8_t           macAddr[6];
    u8_t           bDataQueued;
};

struct zsStaPSList
{
    u8_t           count;
    struct zsStaPSEntity    entity[ZM_MAX_PS_STA];
};

#define ZM_MAX_TIMER_COUNT   32


struct zsTimerEntry
{
    u16_t   event;
    u32_t   timer;
    struct zsTimerEntry *pre;
    struct zsTimerEntry *next;
};

struct zsTimerList
{
    u8_t   freeCount;
    struct zsTimerEntry list[ZM_MAX_TIMER_COUNT];
    struct zsTimerEntry *head;
    struct zsTimerEntry *tail;
};


#define ZM_MAX_MULTICAST_LIST_SIZE     64

struct zsMulticastAddr
{
    u8_t addr[6];
};

struct zsMulticastList
{
    u8_t   size;
    struct zsMulticastAddr macAddr[ZM_MAX_MULTICAST_LIST_SIZE];
};

enum ieee80211_cwm_mode {
    CWM_MODE20,
    CWM_MODE2040,
    CWM_MODE40,
    CWM_MODEMAX

};

enum ieee80211_cwm_extprotspacing {
    CWM_EXTPROTSPACING20,
    CWM_EXTPROTSPACING25,
    CWM_EXTPROTSPACINGMAX
};

enum ieee80211_cwm_width {
    CWM_WIDTH20,
    CWM_WIDTH40
};

enum ieee80211_cwm_extprotmode {
    CWM_EXTPROTNONE,  
    CWM_EXTPROTCTSONLY,   
    CWM_EXTPROTRTSCTS,    
    CWM_EXTPROTMAX
};

struct ieee80211_cwm {

    
    enum ieee80211_cwm_mode         cw_mode;            
    u8_t                            cw_extoffset;       
    enum ieee80211_cwm_extprotmode  cw_extprotmode;     
    enum ieee80211_cwm_extprotspacing cw_extprotspacing;
    u32_t                           cw_enable;          
    u32_t                           cw_extbusythreshold;

    
    enum ieee80211_cwm_width        cw_width;           
};



struct zsStaTable
{
    u32_t time;     
    
    u16_t addr[3];  
    u16_t state;    
    
    struct zsRcCell rcCell;

    u8_t valid;     
    u8_t psMode;    
    u8_t staType;   
    u8_t qosType;   
    u8_t qosInfo;   
    u8_t vap;       
    u8_t encryMode; 
    u8_t keyIdx;
    struct zsMicVar     txMicKey;
    struct zsMicVar     rxMicKey;
    u16_t iv16;
    u32_t iv32;
#ifdef ZM_ENABLE_CENC
    
    u8_t cencKeyIdx;
    u32_t txiv[4];
    u32_t rxiv[4];
#endif 
};

struct zdStructWds
{
    u8_t    wdsBitmap;                      
    u8_t    encryMode[ZM_MAX_WDS_SUPPORT];  
    u16_t   macAddr[ZM_MAX_WDS_SUPPORT][3]; 
};

    
#define HTCAP_AdvCodingCap          0x0001
#define HTCAP_SupChannelWidthSet    0x0002
#define HTCAP_DynamicSMPS           0x0004
#define HTCAP_SMEnabled             0x000C
#define HTCAP_GreenField            0x0010
#define HTCAP_ShortGIfor20MHz       0x0020
#define HTCAP_ShortGIfor40MHz       0x0040
#define HTCAP_TxSTBC                0x0080
#define HTCAP_RxOneStream           0x0100
#define HTCAP_RxTwoStream           0x0200
#define HTCAP_RxThreeStream         0x0300
#define HTCAP_DelayedBlockACK       0x0400
#define HTCAP_MaxAMSDULength        0x0800
#define HTCAP_DSSSandCCKin40MHz     0x1000
#define HTCAP_PSMPSup               0x2000
#define HTCAP_STBCControlFrameSup   0x4000
#define HTCAP_LSIGTXOPProtectionSUP 0x8000
    
#define HTCAP_MaxRxAMPDU0           0x00
#define HTCAP_MaxRxAMPDU1           0x01
#define HTCAP_MaxRxAMPDU2           0x02
#define HTCAP_MaxRxAMPDU3           0x03
    
#define HTCAP_PCO                   0x01
#define HTCAP_TransmissionTime1     0x02
#define HTCAP_TransmissionTime2     0x04
#define HTCAP_TransmissionTime3     0x06
    
#define HTCAP_PlusHTCSupport        0x04
#define HTCAP_RDResponder           0x08
    
#define HTCAP_TxBFCapable           0x01
#define HTCAP_RxStaggeredSoundCap   0x02
#define HTCAP_TxStaggeredSoundCap   0x04
#define HTCAP_RxZLFCapable          0x08
#define HTCAP_TxZLFCapable          0x10
#define HTCAP_ImplicitTxBFCapable   0x20
    
#define HTCAP_ExplicitCSITxBFCap    0x01
#define HTCAP_ExpUncompSteerMatrCap 0x02
    
#define HTCAP_AntennaSelectionCap       0x01
#define HTCAP_ExplicitCSITxASCap        0x02
#define HTCAP_AntennaIndFeeTxASCap      0x04
#define HTCAP_ExplicitCSIFeedbackCap    0x08
#define HTCAP_AntennaIndFeedbackCap     0x10
#define HTCAP_RxASCap                   0x20
#define HTCAP_TxSoundPPDUsCap           0x40



struct zsHTCapability
{
    u8_t ElementID;
    u8_t Length;
    
    u16_t HtCapInfo;
    u8_t AMPDUParam;
    u8_t MCSSet[16];    
    
    u8_t PCO;
    u8_t MCSFeedBack;

    u8_t TxBFCap[4];
    u8_t AselCap;
};

union zuHTCapability
{
    struct zsHTCapability Data;
    u8_t Byte[28];
};

    
#define ExtHtCap_ExtChannelOffsetAbove  0x01
#define ExtHtCap_ExtChannelOffsetBelow  0x03
#define ExtHtCap_RecomTxWidthSet        0x04
#define ExtHtCap_RIFSMode               0x08
#define ExtHtCap_ControlAccessOnly      0x10
    
#define ExtHtCap_NonGFDevicePresent     0x0004
    
#define ExtHtCap_DualBeacon             0x0040
#define ExtHtCap_DualSTBCProtection     0x0080
#define ExtHtCap_SecondaryBeacon        0x0100
#define ExtHtCap_LSIGTXOPProtectFullSup 0x0200
#define ExtHtCap_PCOActive              0x0400
#define ExtHtCap_PCOPhase               0x0800


struct zsExtHTCapability
{
    u8_t    ElementID;
    u8_t    Length;
    u8_t    ControlChannel;
    u8_t    ChannelInfo;
    u16_t   OperatingInfo;
    u16_t   BeaconInfo;
    
    u8_t    MCSSet[16];
};

union zuExtHTCapability
{
    struct zsExtHTCapability Data;
    u8_t Byte[24];
};

struct InformationElementSta {
    struct zsHTCapability       HtCap;
    struct zsExtHTCapability    HtInfo;
};

struct InformationElementAp {
    struct zsHTCapability       HtCap;
};

#define ZM_MAX_FREQ_REQ_QUEUE  32
typedef void (*zfpFreqChangeCompleteCb)(zdev_t* dev);

struct zsWlanDevFreqControl
{
    u16_t                     freqReqQueue[ZM_MAX_FREQ_REQ_QUEUE];
    u8_t                     freqReqBw40[ZM_MAX_FREQ_REQ_QUEUE];
    u8_t                     freqReqExtOffset[ZM_MAX_FREQ_REQ_QUEUE];
    zfpFreqChangeCompleteCb   freqChangeCompCb[ZM_MAX_FREQ_REQ_QUEUE];
    u8_t                      freqReqQueueHead;
    u8_t                      freqReqQueueTail;
};

struct zsWlanDevAp
{
    u16_t   protectedObss;    
    u16_t   staAgingTimeSec;  
                              
    u16_t   staProbingTimeSec;
                              
    u8_t    authSharing;      
    u8_t    bStaAssociated;   
    u8_t    gStaAssociated;   
    u8_t    nStaAssociated;   
    u16_t   protectionMode;   
    u16_t   staPowerSaving;   



    zbuf_t*  uniArray[ZM_UNI_ARRAY_SIZE]; 
    u16_t   uniHead;
    u16_t   uniTail;

    
    union zuHTCapability HTCap; 

    
    union zuExtHTCapability ExtHTCap; 

    
    struct zsStaTable staTable[ZM_MAX_STA_SUPPORT];

    
    struct zdStructWds wds;
    
    u8_t wpaIe[ZM_MAX_AP_SUPPORT][ZM_MAX_WPAIE_SIZE];
    u8_t wpaLen[ZM_MAX_AP_SUPPORT];
    u8_t stawpaIe[ZM_MAX_AP_SUPPORT][ZM_MAX_WPAIE_SIZE];
    u8_t stawpaLen[ZM_MAX_AP_SUPPORT];
    u8_t wpaSupport[ZM_MAX_AP_SUPPORT];

    
    u8_t bcKeyIndex[ZM_MAX_AP_SUPPORT];
    u8_t bcHalKeyIdx[ZM_MAX_AP_SUPPORT];
    struct zsMicVar     bcMicKey[ZM_MAX_AP_SUPPORT];
    u16_t iv16[ZM_MAX_AP_SUPPORT];
    u32_t iv32[ZM_MAX_AP_SUPPORT];

#ifdef ZM_ENABLE_CENC
    
    u32_t txiv[ZM_MAX_AP_SUPPORT][4];
#endif 

    
    u8_t    beaconCounter;
    u8_t    vapNumber;
    u8_t    apBitmap;                         
    u8_t    hideSsid[ZM_MAX_AP_SUPPORT];
    u8_t    authAlgo[ZM_MAX_AP_SUPPORT];
    u8_t    ssid[ZM_MAX_AP_SUPPORT][32];      
    u8_t    ssidLen[ZM_MAX_AP_SUPPORT];       
    u8_t    encryMode[ZM_MAX_AP_SUPPORT];
    u8_t    wepStatus[ZM_MAX_AP_SUPPORT];
    u16_t   capab[ZM_MAX_AP_SUPPORT];         
    u8_t    timBcmcBit[ZM_MAX_AP_SUPPORT];    
    u8_t    wlanType[ZM_MAX_AP_SUPPORT];

    
    zbuf_t*  bcmcArray[ZM_MAX_AP_SUPPORT][ZM_BCMC_ARRAY_SIZE];
    u16_t   bcmcHead[ZM_MAX_AP_SUPPORT];
    u16_t   bcmcTail[ZM_MAX_AP_SUPPORT];

    u8_t                    qosMode;                          
    u8_t                    uapsdEnabled;
    struct zsQueue*         uapsdQ;

    u8_t                    challengeText[128];

    struct InformationElementAp ie[ZM_MAX_STA_SUPPORT];


};

#define ZM_MAX_BLOCKING_AP_LIST_SIZE    4 
struct zsBlockingAp
{
    u8_t addr[6];
    u8_t weight;
};

#define ZM_SCAN_MGR_SCAN_NONE           0
#define ZM_SCAN_MGR_SCAN_INTERNAL       1
#define ZM_SCAN_MGR_SCAN_EXTERNAL       2

struct zsWlanDevStaScanMgr
{
    u8_t                    scanReqs[2];
    u8_t                    currScanType;
    u8_t                    scanStartDelay;
};

#define ZM_PS_MSG_STATE_ACTIVE          0
#define ZM_PS_MSG_STATE_SLEEP           1
#define ZM_PS_MSG_STATE_T1              2
#define ZM_PS_MSG_STATE_T2              3
#define ZM_PS_MSG_STATE_S1              4

#define ZM_PS_MAX_SLEEP_PERIODS         3       

struct zsWlanDevStaPSMgr
{
    u8_t                    state;
    u8_t                    isSleepAllowed;
    u8_t                    maxSleepPeriods;
    u8_t                    ticks;
    u32_t                   lastTxUnicastFrm;
    u32_t                   lastTxMulticastFrm;
    u32_t                   lastTxBroadcastFrm;
    u8_t                    tempWakeUp; 
    u16_t                   sleepAllowedtick;
};

struct zsWlanDevSta
{
    u32_t                   beaconTxCnt;  
    u8_t                    txBeaconInd;  
    u16_t                   beaconCnt;    
    u16_t                   bssid[3];     
    u8_t                    ssid[32];     
    u8_t                    ssidLen;      
    u8_t                    mTxRate;      
    u8_t                    uTxRate;      
    u8_t                    mmTxRate;     
    u8_t                    bChannelScan;
    u8_t                    bScheduleScan;

    u8_t                    InternalScanReq;
    u16_t                   activescanTickPerChannel;
    u16_t                   passiveScanTickPerChannel;
    u16_t                   scanFrequency;
    u32_t                   connPowerInHalfDbm;

    u16_t                   currentFrequency;
    u16_t                   currentBw40;
    u16_t                   currentExtOffset;

    u8_t                    bPassiveScan;

    struct zsBlockingAp     blockingApList[ZM_MAX_BLOCKING_AP_LIST_SIZE];

    
    struct zsBssInfo*       bssInfoArray[ZM_MAX_BSS];
    struct zsBssList        bssList;
    u8_t                    bssInfoArrayHead;
    u8_t                    bssInfoArrayTail;
    u8_t                    bssInfoFreeCount;

    u8_t                    authMode;
    u8_t                    currentAuthMode;
    u8_t                    wepStatus;
    u8_t                    encryMode;
    u8_t                    keyId;
#ifdef ZM_ENABLE_IBSS_WPA2PSK
    u8_t                    ibssWpa2Psk;
#endif
#ifdef ZM_ENABLE_CENC
    u8_t                    cencKeyId; 
#endif 
    u8_t                    dropUnencryptedPkts;
    u8_t                    ibssJoinOnly;
    u8_t                    adapterState;
    u8_t                    oldAdapterState;
    u8_t                    connectState;
    u8_t                    connectRetry;
    u8_t                    wpaState;
    u8_t                    wpaIe[ZM_MAX_IE_SIZE + 2];
    u8_t                    rsnIe[ZM_MAX_IE_SIZE + 2];
    u8_t                    challengeText[255+2];
    u8_t                    capability[2];
    
    
    u16_t                   aid;
    u32_t                   mgtFrameCount;
    u8_t                    bProtectionMode;
	u32_t					NonNAPcount;
	u8_t					RTSInAGGMode;
    u32_t                   connectTimer;
    u16_t                   atimWindow;
    u8_t                    desiredBssid[6];
    u8_t                    bDesiredBssid;
    struct zsTkipSeed       txSeed;
    struct zsTkipSeed       rxSeed[4];
    struct zsMicVar         txMicKey;
    struct zsMicVar         rxMicKey[4];
    u16_t                   iv16;
    u32_t                   iv32;
    struct zsOppositeInfo   oppositeInfo[ZM_MAX_OPPOSITE_COUNT];
    u8_t                    oppositeCount;
    u8_t                    bssNotFoundCount;     
    u16_t                   rxBeaconCount;
    u8_t                    beaconMissState;
    u32_t                   rxBeaconTotal;
    u8_t                    bIsSharedKey;
    u8_t                    connectTimeoutCount;

    u8_t                    recvAtim;

    
    struct zsWlanDevStaScanMgr scanMgr;
    struct zsWlanDevStaPSMgr   psMgr;

    
    
    
    zfpStaRxSecurityCheckCb pStaRxSecurityCheckCb;

    
    u8_t                    apWmeCapability; 
                                             
    u8_t                    wmeParameterSetCount;

    u8_t                    wmeEnabled;
    #define ZM_STA_WME_ENABLE_BIT       0x1
    #define ZM_STA_UAPSD_ENABLE_BIT     0x2
    u8_t                    wmeQosInfo;

    u8_t                    wmeConnected;
    u8_t                    qosInfo;
    struct zsQueue*         uapsdQ;

    
    u8_t                    cmMicFailureCount;
    u8_t                    cmDisallowSsidLength;
    u8_t                    cmDisallowSsid[32];

    
    u8_t                    powerSaveMode;
    zbuf_t*                 staPSDataQueue[ZM_PS_QUEUE_SIZE];
    u8_t                    staPSDataCount;

    
    
    struct zsStaPSList      staPSList;
    
    zbuf_t*                  ibssPSDataQueue[ZM_PS_QUEUE_SIZE];
    u8_t                    ibssPSDataCount;
    u8_t                    ibssPrevPSDataCount;
    u8_t                    bIbssPSEnable;
    
    u16_t                   ibssAtimTimer;

    
    struct zsPmkidInfo      pmkidInfo;

    
    struct zsMulticastList  multicastList;

    
    
    
    u8_t                    bAllMulticast;

    
    u8_t                    connectByReasso;
    u8_t                    failCntOfReasso;

	
    u8_t                    preambleTypeHT;  
	u8_t                    htCtrlBandwidth;
	u8_t                    htCtrlSTBC;
	u8_t                    htCtrlSG;
    u8_t                    defaultTA;

    u8_t                    connection_11b;

    u8_t                    EnableHT;
    u8_t                    SG40;
    u8_t                    HT2040;
    
    u8_t                    wpaSupport;
    u8_t                    wpaLen;

    
    u8_t                    ibssDelayedInd;
    struct zsPartnerNotifyEvent ibssDelayedIndEvent;
    u8_t                    ibssPartnerStatus;

    u8_t                    bAutoReconnect;

    u8_t                    flagFreqChanging;
    u8_t                    flagKeyChanging;
    struct zsBssInfo        ibssBssDesc;
    u8_t                    ibssBssIsCreator;
    u16_t                   ibssReceiveBeaconCount;
    u8_t                    ibssSiteSurveyStatus;

    u8_t                    disableProbingWithSsid;
#ifdef ZM_ENABLE_CENC
    
    u8_t                    cencIe[ZM_MAX_IE_SIZE + 2];
#endif 
    u32_t txiv[4];  
    u32_t rxiv[4];  
    u32_t rxivGK[4];
    u8_t  wepKey[4][32];    
	u8_t  SWEncryMode[4];

    
    u8_t b802_11D;

    
    u8_t TPCEnable;
    u8_t DFSEnable;
    u8_t DFSDisableTx;

    
    u8_t athOwlAp;

    
    u8_t enableDrvBA;

    
    union zuHTCapability HTCap; 

    
    union zuExtHTCapability ExtHTCap; 

    struct InformationElementSta   ie;

#define ZM_CACHED_FRAMEBODY_SIZE   200
    u8_t                    asocReqFrameBody[ZM_CACHED_FRAMEBODY_SIZE];
    u16_t                   asocReqFrameBodySize;
    u8_t                    asocRspFrameBody[ZM_CACHED_FRAMEBODY_SIZE];
    u16_t                   asocRspFrameBodySize;
    u8_t                    beaconFrameBody[ZM_CACHED_FRAMEBODY_SIZE];
    u16_t                   beaconFrameBodySize;

    u8_t                    ac0PriorityHigherThanAc2;
    u8_t                    SWEncryptEnable;

    u8_t                    leapEnabled;

    u32_t                    TotalNumberOfReceivePackets;
    u32_t                    TotalNumberOfReceiveBytes;
    u32_t                    avgSizeOfReceivePackets;

    u32_t                    ReceivedPacketRateCounter;
    u32_t                    ReceivedPktRatePerSecond;

    
#define    ZM_RIFS_STATE_DETECTING    0
#define    ZM_RIFS_STATE_DETECTED     1
#define    ZM_RIFS_TIMER_TIMEOUT      4480          
    u8_t                    rifsState;
    u8_t                    rifsLikeFrameCnt;
    u16_t                   rifsLikeFrameSequence[3];
    u32_t                   rifsTimer;
    u32_t                   rifsCount;

    
    u32_t  osRxFilter;

    u8_t   bSafeMode;

    u32_t  ibssAdditionalIESize;
    u8_t   ibssAdditionalIE[256];
}; 

#define ZM_CMD_QUEUE_SIZE                   256  

#define ZM_OID_READ                         1
#define ZM_OID_WRITE                        2
#define ZM_OID_INTERNAL_WRITE               3
#define ZM_CMD_SET_FREQUENCY                4
#define ZM_CMD_SET_KEY                      5
#define ZM_CWM_READ                         6
#define ZM_MAC_READ                         7
#define ZM_ANI_READ                         8
#define ZM_EEPROM_READ                      9
#define ZM_EEPROM_WRITE                     0x0A
#define ZM_OID_CHAN							0x30
#define ZM_OID_SYNTH						0x32
#define ZM_OID_TALLY						0x81
#define ZM_OID_TALLY_APD					0x82

#define ZM_OID_DKTX_STATUS                  0x92
#define ZM_OID_FLASH_CHKSUM                 0xD0
#define ZM_OID_FLASH_READ                   0xD1
#define ZM_OID_FLASH_PROGRAM                0xD2
#define ZM_OID_FW_DL_INIT                   0xD3


#define ZM_CMD_ECHO         0x80
#define ZM_CMD_TALLY        0x81
#define ZM_CMD_TALLY_APD    0x82
#define ZM_CMD_CONFIG       0x83
#define ZM_CMD_RREG         0x00
#define ZM_CMD_WREG         0x01
#define ZM_CMD_RMEM         0x02
#define ZM_CMD_WMEM         0x03
#define ZM_CMD_BITAND       0x04
#define ZM_CMD_BITOR        0x05
#define ZM_CMD_EKEY         0x28
#define ZM_CMD_DKEY         0x29
#define ZM_CMD_FREQUENCY    0x30
#define ZM_CMD_RF_INIT      0x31
#define ZM_CMD_SYNTH        0x32
#define ZM_CMD_FREQ_STRAT   0x33
#define ZM_CMD_RESET        0x90
#define ZM_CMD_DKRESET      0x91
#define ZM_CMD_DKTX_STATUS  0x92
#define ZM_CMD_FDC          0xA0
#define ZM_CMD_WREEPROM     0xB0
#define ZM_CMD_WFLASH       0xB0
#define ZM_CMD_FLASH_ERASE  0xB1
#define ZM_CMD_FLASH_PROG   0xB2
#define ZM_CMD_FLASH_CHKSUM 0xB3
#define ZM_CMD_FLASH_READ   0xB4
#define ZM_CMD_FW_DL_INIT   0xB5
#define ZM_CMD_MEM_WREEPROM 0xBB



#define ZM_FILTER_TABLE_COL                 2 

#define ZM_FILTER_TABLE_ROW                 8 


struct zsRxFilter
{
    u16_t addr[3];
    u16_t seq;
    u8_t up;
};

struct zsWlanDev
{
    
    struct zsWlanDevAp ap;
    
    struct zsWlanDevSta sta;
    
    struct zsWrapperSetting ws;
    
    struct zsWrapperFeatureCtrl wfc;
    
    struct zsTrafTally trafTally;
    
    struct zsCommTally commTally;
    
    struct zsRxFilter rxFilterTbl[ZM_FILTER_TABLE_COL][ZM_FILTER_TABLE_ROW];
    
    struct zsRegulationTable  regulationTable;

    
    struct zsWlanDevFreqControl freqCtrl;

    enum devState state;

    u8_t  halState;
    u8_t  wlanMode;         
    u16_t macAddr[3];       
    u16_t beaconInterval;   
    u8_t dtim;              
    u8_t            CurrentDtimCount;
    u8_t  preambleType;
    u8_t  preambleTypeInUsed;
    u8_t  maxTxPower2;	    
    u8_t  maxTxPower5;	    
    u8_t  connectMode;
    u32_t supportMode;

    u8_t bRate;             
    u8_t bRateBasic;        
    u8_t gRate;             
    u8_t gRateBasic;        
    
    u8_t channelIndex;

    
    u8_t    BandWidth40;
    u8_t    ExtOffset;      
    u16_t   frequency;      

    u8_t erpElement;        

    u8_t disableSelfCts;    
    u8_t bgMode;

	
    u32_t enableProtectionMode;   
	u32_t checksumTest;     
	u32_t rxPacketDump;     

    u8_t enableAggregation; 
    u8_t enableWDS;         
	u8_t enableTxPathMode;  
    u8_t enableHALDbgInfo;  

    u32_t forceTxTPC;       

    u16_t seq[4];
    u16_t mmseq;

    
    u32_t tick;
    u16_t tickIbssSendBeacon;
    u16_t tickIbssReceiveBeacon;

    
    u16_t rtsThreshold;

    
    u16_t fragThreshold;

    
    u16_t txMCS;
    u16_t txMT;
    u32_t CurrentTxRateKbps; 
    
    u32_t CurrentRxRateKbps; 
    u8_t CurrentRxRateUpdated;
    u8_t modulationType;
    u8_t rxInfo;
    u16_t rateField;

    
    struct zsTimerList  timerList;
    u8_t                bTimerReady;

    
    struct zsDefragList defragTable;

    
    

    
    u8_t SignalStrength; 
    u8_t SignalQuality;  



    
    zbuf_t* vtxq[4][ZM_VTXQ_SIZE];
    u16_t vtxqHead[4];
    u16_t vtxqTail[4];
    u16_t qosDropIpFrag[4];

    
    zbuf_t* vmmq[ZM_VMMQ_SIZE];
    u16_t vmmqHead;
    u16_t vmmqTail;

    u8_t                vtxqPushing;

    
    struct aggQueue    *aggQPool[ZM_AGG_POOL_SIZE];
    u8_t                aggInitiated;
    u8_t                addbaComplete;
    u8_t                addbaCount;
    u8_t                aggState;
    u8_t                destLock;
    struct aggSta       aggSta[ZM_MAX_STA_SUPPORT];
    struct agg_tid_rx  *tid_rx[ZM_AGG_POOL_SIZE];
    struct aggTally     agg_tal;
    struct destQ        destQ;
    struct baw_enabler *baw_enabler;
    struct ieee80211_cwm    cwm;
    u16_t               reorder;
    u16_t               seq_debug;
    
    u32_t txMPDU[ZM_RATE_TABLE_SIZE];
    u32_t txFail[ZM_RATE_TABLE_SIZE];
    u32_t PER[ZM_RATE_TABLE_SIZE];
    u16_t probeCount;
    u16_t probeSuccessCount;
    u16_t probeInterval;
    u16_t success_probing;
    

    
    u32_t               swSniffer;   
    u32_t               XLinkMode;

    
    
    u32_t               modeMDKEnable;

    u32_t               heartBeatNotification;

    
    void* hpPrivate;

    
    
    
    

    struct zsLedStruct      ledStruct;

    
    u8_t aniEnable;
    u16_t txq_threshold;

	
	u8_t	TKIP_Group_KeyChanging;

	u8_t    dynamicSIFSEnable;

	u8_t    queueFlushed;

    u16_t (*zfcbAuthNotify)(zdev_t* dev, u16_t* macAddr);
    u16_t (*zfcbAsocNotify)(zdev_t* dev, u16_t* macAddr, u8_t* body, u16_t bodySize, u16_t port);
    u16_t (*zfcbDisAsocNotify)(zdev_t* dev, u8_t* macAddr, u16_t port);
    u16_t (*zfcbApConnectNotify)(zdev_t* dev, u8_t* macAddr, u16_t port);
    void (*zfcbConnectNotify)(zdev_t* dev, u16_t status, u16_t* bssid);
    void (*zfcbScanNotify)(zdev_t* dev, struct zsScanResult* result);
    void (*zfcbMicFailureNotify)(zdev_t* dev, u16_t* addr, u16_t status);
    void (*zfcbApMicFailureNotify)(zdev_t* dev, u8_t* addr, zbuf_t* buf);
    void (*zfcbIbssPartnerNotify)(zdev_t* dev, u16_t status, struct zsPartnerNotifyEvent *event);
    void (*zfcbMacAddressNotify)(zdev_t* dev, u8_t* addr);
    void (*zfcbSendCompleteIndication)(zdev_t* dev, zbuf_t* buf);
    void (*zfcbRecvEth)(zdev_t* dev, zbuf_t* buf, u16_t port);
    void (*zfcbRecv80211)(zdev_t* dev, zbuf_t* buf, struct zsAdditionInfo* addInfo);
    void (*zfcbRestoreBufData)(zdev_t* dev, zbuf_t* buf);
#ifdef ZM_ENABLE_CENC
    u16_t (*zfcbCencAsocNotify)(zdev_t* dev, u16_t* macAddr, u8_t* body,
            u16_t bodySize, u16_t port);
#endif 
    u8_t (*zfcbClassifyTxPacket)(zdev_t* dev, zbuf_t* buf);
    void (*zfcbHwWatchDogNotify)(zdev_t* dev);
};


struct zsWlanKey
{
    u8_t key;
};





#define zmw_tx_buf_readb(dev, buf, offset) zmw_buf_readb(dev, buf, offset)
#define zmw_tx_buf_readh(dev, buf, offset) zmw_buf_readh(dev, buf, offset)
#define zmw_tx_buf_writeb(dev, buf, offset, value) zmw_buf_writeb(dev, buf, offset, value)
#define zmw_tx_buf_writeh(dev, buf, offset, value) zmw_buf_writeh(dev, buf, offset, value)


#define zmw_rx_buf_readb(dev, buf, offset) zmw_buf_readb(dev, buf, offset)
#define zmw_rx_buf_readh(dev, buf, offset) zmw_buf_readh(dev, buf, offset)
#define zmw_rx_buf_writeb(dev, buf, offset, value) zmw_buf_writeb(dev, buf, offset, value)
#define zmw_rx_buf_writeh(dev, buf, offset, value) zmw_buf_writeh(dev, buf, offset, value)

#endif 
