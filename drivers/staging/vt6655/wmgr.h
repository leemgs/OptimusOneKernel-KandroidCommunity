

#ifndef __WMGR_H__
#define __WMGR_H__

#include "ttype.h"
#include "80211mgr.h"
#include "80211hdr.h"
#include "wcmd.h"
#include "bssdb.h"
#include "wpa2.h"
#include "vntwifi.h"
#include "card.h"






#define PROBE_DELAY                  100  
#define SWITCH_CHANNEL_DELAY         200 
#define WLAN_SCAN_MINITIME           25   
#define WLAN_SCAN_MAXTIME            100  
#define TRIVIAL_SYNC_DIFFERENCE      0    
#define DEFAULT_IBSS_BI              100  

#define WCMD_ACTIVE_SCAN_TIME   50 
#define WCMD_PASSIVE_SCAN_TIME  100 


#define DEFAULT_MSDU_LIFETIME           512  
#define DEFAULT_MSDU_LIFETIME_RES_64us  8000 

#define DEFAULT_MGN_LIFETIME            8    
#define DEFAULT_MGN_LIFETIME_RES_64us   125  

#define MAKE_BEACON_RESERVED            10  


#define TIM_MULTICAST_MASK           0x01
#define TIM_BITMAPOFFSET_MASK        0xFE
#define DEFAULT_DTIM_PERIOD             1

#define AP_LONG_RETRY_LIMIT             4

#define DEFAULT_IBSS_CHANNEL            6  







#define timer_expire(timer,next_tick)   mod_timer(&timer, RUN_AT(next_tick))
typedef void (*TimerFunction)(ULONG);




typedef UCHAR   NDIS_802_11_MAC_ADDRESS[6];
typedef struct _NDIS_802_11_AI_REQFI
{
    USHORT Capabilities;
    USHORT ListenInterval;
    NDIS_802_11_MAC_ADDRESS  CurrentAPAddress;
} NDIS_802_11_AI_REQFI, *PNDIS_802_11_AI_REQFI;

typedef struct _NDIS_802_11_AI_RESFI
{
    USHORT Capabilities;
    USHORT StatusCode;
    USHORT AssociationId;
} NDIS_802_11_AI_RESFI, *PNDIS_802_11_AI_RESFI;

typedef struct _NDIS_802_11_ASSOCIATION_INFORMATION
{
    ULONG                   Length;
    USHORT                  AvailableRequestFixedIEs;
    NDIS_802_11_AI_REQFI    RequestFixedIEs;
    ULONG                   RequestIELength;
    ULONG                   OffsetRequestIEs;
    USHORT                  AvailableResponseFixedIEs;
    NDIS_802_11_AI_RESFI    ResponseFixedIEs;
    ULONG                   ResponseIELength;
    ULONG                   OffsetResponseIEs;
} NDIS_802_11_ASSOCIATION_INFORMATION, *PNDIS_802_11_ASSOCIATION_INFORMATION;



typedef struct tagSAssocInfo {
    NDIS_802_11_ASSOCIATION_INFORMATION     AssocInfo;
    BYTE                                    abyIEs[WLAN_BEACON_FR_MAXLEN+WLAN_BEACON_FR_MAXLEN];
    
    ULONG                                   RequestIELength;
    BYTE                                    abyReqIEs[WLAN_BEACON_FR_MAXLEN];
} SAssocInfo, *PSAssocInfo;









typedef enum tagWMAC_SCAN_TYPE {

    WMAC_SCAN_ACTIVE,
    WMAC_SCAN_PASSIVE,
    WMAC_SCAN_HYBRID

} WMAC_SCAN_TYPE, *PWMAC_SCAN_TYPE;


typedef enum tagWMAC_SCAN_STATE {

    WMAC_NO_SCANNING,
    WMAC_IS_SCANNING,
    WMAC_IS_PROBEPENDING

} WMAC_SCAN_STATE, *PWMAC_SCAN_STATE;













typedef enum tagWMAC_BSS_STATE {

    WMAC_STATE_IDLE,
    WMAC_STATE_STARTED,
    WMAC_STATE_JOINTED,
    WMAC_STATE_AUTHPENDING,
    WMAC_STATE_AUTH,
    WMAC_STATE_ASSOCPENDING,
    WMAC_STATE_ASSOC

} WMAC_BSS_STATE, *PWMAC_BSS_STATE;


typedef enum tagWMAC_CURRENT_MODE {

    WMAC_MODE_STANDBY,
    WMAC_MODE_ESS_STA,
    WMAC_MODE_IBSS_STA,
    WMAC_MODE_ESS_AP

} WMAC_CURRENT_MODE, *PWMAC_CURRENT_MODE;





typedef struct tagSTxMgmtPacket {

    PUWLAN_80211HDR     p80211Header;
    UINT                cbMPDULen;
    UINT                cbPayloadLen;

} STxMgmtPacket, *PSTxMgmtPacket;



typedef struct tagSRxMgmtPacket {

    PUWLAN_80211HDR     p80211Header;
    QWORD               qwLocalTSF;
    UINT                cbMPDULen;
    UINT                cbPayloadLen;
    UINT                uRSSI;
    BYTE                bySQ;
    BYTE                byRxRate;
    BYTE                byRxChannel;

} SRxMgmtPacket, *PSRxMgmtPacket;



typedef struct tagSMgmtObject
{

    PVOID                   pAdapter;
    
    BYTE                    abyMACAddr[WLAN_ADDR_LEN];

    
    WMAC_CONFIG_MODE        eConfigMode; 
    CARD_PHY_TYPE           eCurrentPHYMode;
    CARD_PHY_TYPE           eConfigPHYMode;


    
    WMAC_CURRENT_MODE       eCurrMode;   
    WMAC_BSS_STATE          eCurrState;  

    PKnownBSS               pCurrBSS;
    BYTE                    byCSSGK;
    BYTE                    byCSSPK;




    
    UINT                    uCurrChannel;
    BYTE                    abyCurrSuppRates[WLAN_IEHDR_LEN + WLAN_RATES_MAXLEN + 1];
    BYTE                    abyCurrExtSuppRates[WLAN_IEHDR_LEN + WLAN_RATES_MAXLEN + 1];
    BYTE                    abyCurrSSID[WLAN_IEHDR_LEN + WLAN_SSID_MAXLEN + 1];
    BYTE                    abyCurrBSSID[WLAN_BSSID_LEN];
    WORD                    wCurrCapInfo;
    WORD                    wCurrAID;
    WORD                    wCurrATIMWindow;
    WORD                    wCurrBeaconPeriod;
    BOOL                    bIsDS;
    BYTE                    byERPContext;

    CMD_STATE               eCommandState;
    UINT                    uScanChannel;

    
    BYTE                    abyDesireSSID[WLAN_IEHDR_LEN + WLAN_SSID_MAXLEN + 1];
    BYTE                    abyDesireBSSID[WLAN_BSSID_LEN];

    
  
    WORD                    wIBSSBeaconPeriod;
    WORD                    wIBSSATIMWindow;
    UINT                    uIBSSChannel;
    BYTE                    abyIBSSSuppRates[WLAN_IEHDR_LEN + WLAN_RATES_MAXLEN + 1];
    BYTE                    byAPBBType;
    BYTE                    abyWPAIE[MAX_WPA_IE_LEN];
    WORD                    wWPAIELen;

    UINT                    uAssocCount;
    BOOL                    bMoreData;

    
    WMAC_SCAN_STATE         eScanState;
    WMAC_SCAN_TYPE          eScanType;
    UINT                    uScanStartCh;
    UINT                    uScanEndCh;
    WORD                    wScanSteps;
    UINT                    uScanBSSType;
    
    BYTE                    abyScanSSID[WLAN_IEHDR_LEN + WLAN_SSID_MAXLEN + 1];
    BYTE                    abyScanBSSID[WLAN_BSSID_LEN];

    
    WMAC_AUTHENTICATION_MODE eAuthenMode;
    WMAC_ENCRYPTION_MODE    eEncryptionMode;
    BOOL                    bShareKeyAlgorithm;
    BYTE                    abyChallenge[WLAN_CHALLENGE_LEN];
    BOOL                    bPrivacyInvoked;

    
    BOOL                    bInTIM;
    BOOL                    bMulticastTIM;
    BYTE                    byDTIMCount;
    BYTE                    byDTIMPeriod;

    
    WMAC_POWER_MODE         ePSMode;
    WORD                    wListenInterval;
    WORD                    wCountToWakeUp;
    BOOL                    bInTIMWake;
    PBYTE                   pbyPSPacketPool;
    BYTE                    byPSPacketPool[sizeof(STxMgmtPacket) + WLAN_NULLDATA_FR_MAXLEN];
    BOOL                    bRxBeaconInTBTTWake;
    BYTE                    abyPSTxMap[MAX_NODE_NUM + 1];

    
    UINT                    uCmdBusy;
    UINT                    uCmdHostAPBusy;

    
    PBYTE                   pbyMgmtPacketPool;
    BYTE                    byMgmtPacketPool[sizeof(STxMgmtPacket) + WLAN_A3FR_MAXLEN];


    
    struct timer_list	    sTimerSecondCallback;

    
    SRxMgmtPacket           sRxPacket;

    
    KnownBSS                sBSSList[MAX_BSS_NUM];



    
    
    
    KnownNodeDB             sNodeDBTable[MAX_NODE_NUM + 1];



    
    SPMKIDCache             gsPMKIDCache;
    BOOL                    bRoaming;

    



    
    SAssocInfo              sAssocInfo;


    
    BOOL                    b11hEnable;
    BOOL                    bSwitchChannel;
    BYTE                    byNewChannel;
    PWLAN_IE_MEASURE_REP    pCurrMeasureEIDRep;
    UINT                    uLengthOfRepEIDs;
    BYTE                    abyCurrentMSRReq[sizeof(STxMgmtPacket) + WLAN_A3FR_MAXLEN];
    BYTE                    abyCurrentMSRRep[sizeof(STxMgmtPacket) + WLAN_A3FR_MAXLEN];
    BYTE                    abyIECountry[WLAN_A3FR_MAXLEN];
    BYTE                    abyIBSSDFSOwner[6];
    BYTE                    byIBSSDFSRecovery;

    struct sk_buff  skb;

} SMgmtObject, *PSMgmtObject;








void
vMgrObjectInit(
    IN  HANDLE hDeviceContext
    );

void
vMgrTimerInit(
    IN  HANDLE hDeviceContext
    );

VOID
vMgrObjectReset(
    IN  HANDLE hDeviceContext
    );

void
vMgrAssocBeginSta(
    IN  HANDLE hDeviceContext,
    IN  PSMgmtObject pMgmt,
    OUT PCMD_STATUS pStatus
    );

VOID
vMgrReAssocBeginSta(
    IN  HANDLE hDeviceContext,
    IN  PSMgmtObject pMgmt,
    OUT PCMD_STATUS pStatus
    );

VOID
vMgrDisassocBeginSta(
    IN  HANDLE hDeviceContext,
    IN  PSMgmtObject pMgmt,
    IN  PBYTE  abyDestAddress,
    IN  WORD    wReason,
    OUT PCMD_STATUS pStatus
    );

VOID
vMgrAuthenBeginSta(
    IN  HANDLE hDeviceContext,
    IN  PSMgmtObject pMgmt,
    OUT PCMD_STATUS pStatus
    );

VOID
vMgrCreateOwnIBSS(
    IN  HANDLE hDeviceContext,
    OUT PCMD_STATUS pStatus
    );

VOID
vMgrJoinBSSBegin(
    IN  HANDLE hDeviceContext,
    OUT PCMD_STATUS pStatus
    );

VOID
vMgrRxManagePacket(
    IN  HANDLE hDeviceContext,
    IN  PSMgmtObject pMgmt,
    IN  PSRxMgmtPacket pRxPacket
    );



VOID
vMgrDeAuthenBeginSta(
    IN  HANDLE hDeviceContext,
    IN  PSMgmtObject  pMgmt,
    IN  PBYTE   abyDestAddress,
    IN  WORD    wReason,
    OUT PCMD_STATUS pStatus
    );

BOOL
bMgrPrepareBeaconToSend(
    IN  HANDLE hDeviceContext,
    IN  PSMgmtObject pMgmt
    );


BOOL
bAdd_PMKID_Candidate (
    IN HANDLE    hDeviceContext,
    IN PBYTE          pbyBSSID,
    IN PSRSNCapObject psRSNCapObj
    );

VOID
vFlush_PMKID_Candidate (
    IN HANDLE hDeviceContext
    );

#endif 
