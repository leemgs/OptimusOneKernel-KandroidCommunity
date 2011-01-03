

#ifndef __BSSDB_H__
#define __BSSDB_H__

#include <linux/skbuff.h>
#include "80211hdr.h"
#include "80211mgr.h"
#include "card.h"



#define MAX_NODE_NUM             64
#define MAX_BSS_NUM              42
#define LOST_BEACON_COUNT      	 10   
#define MAX_PS_TX_BUF            32   
#define ADHOC_LOST_BEACON_COUNT  30   
#define MAX_INACTIVE_COUNT       300  

#define USE_PROTECT_PERIOD       10   
#define ERP_RECOVER_COUNT        30   
#define BSS_CLEAR_COUNT           1

#define RSSI_STAT_COUNT          10
#define MAX_CHECK_RSSI_COUNT     8


#define WLAN_STA_AUTH            BIT0
#define WLAN_STA_ASSOC           BIT1
#define WLAN_STA_PS              BIT2
#define WLAN_STA_TIM             BIT3

#define WLAN_STA_PERM            BIT4



#define WLAN_STA_AUTHORIZED      BIT5

#define MAX_RATE            12

#define MAX_WPA_IE_LEN      64













typedef enum _NDIS_802_11_NETWORK_TYPE
{
    Ndis802_11FH,
    Ndis802_11DS,
    Ndis802_11OFDM5,
    Ndis802_11OFDM24,
    Ndis802_11NetworkTypeMax    
} NDIS_802_11_NETWORK_TYPE, *PNDIS_802_11_NETWORK_TYPE;


typedef struct tagSERPObject {
    BOOL    bERPExist;
    BYTE    byERP;
}ERPObject, *PERPObject;


typedef struct tagSRSNCapObject {
    BOOL    bRSNCapExist;
    WORD    wRSNCap;
}SRSNCapObject, *PSRSNCapObject;


#pragma pack(1)
typedef struct tagKnownBSS {
    
    BOOL            bActive;
    BYTE            abyBSSID[WLAN_BSSID_LEN];
    UINT            uChannel;
    BYTE            abySuppRates[WLAN_IEHDR_LEN + WLAN_RATES_MAXLEN + 1];
    BYTE            abyExtSuppRates[WLAN_IEHDR_LEN + WLAN_RATES_MAXLEN + 1];
    UINT            uRSSI;
    BYTE            bySQ;
    WORD            wBeaconInterval;
    WORD            wCapInfo;
    BYTE            abySSID[WLAN_IEHDR_LEN + WLAN_SSID_MAXLEN + 1];
    BYTE            byRxRate;


    BYTE            byRSSIStatCnt;
    LONG            ldBmMAX;
    LONG            ldBmAverage[RSSI_STAT_COUNT];
    LONG            ldBmAverRange;
    
    BOOL            bSelected;

    
    BOOL            bWPAValid;
    BYTE            byGKType;
    BYTE            abyPKType[4];
    WORD            wPKCount;
    BYTE            abyAuthType[4];
    WORD            wAuthCount;
    BYTE            byDefaultK_as_PK;
    BYTE            byReplayIdx;
    

    
    BOOL            bWPA2Valid;
    BYTE            byCSSGK;
    WORD            wCSSPKCount;
    BYTE            abyCSSPK[4];
    WORD            wAKMSSAuthCount;
    BYTE            abyAKMSSAuthType[4];

    
    BYTE            byWPAIE[MAX_WPA_IE_LEN];
    BYTE            byRSNIE[MAX_WPA_IE_LEN];
    WORD            wWPALen;
    WORD            wRSNLen;

    
    UINT            uClearCount;

    UINT            uIELength;
    QWORD           qwBSSTimestamp;
    QWORD           qwLocalTSF;     


    CARD_PHY_TYPE   eNetworkTypeInUse;

    ERPObject       sERP;
    SRSNCapObject   sRSNCapObj;
    BYTE            abyIEs[1024];   

}__attribute__ ((__packed__))
KnownBSS , *PKnownBSS;


#pragma pack()

typedef enum tagNODE_STATE {
    NODE_FREE,
    NODE_AGED,
    NODE_KNOWN,
    NODE_AUTH,
    NODE_ASSOC
} NODE_STATE, *PNODE_STATE;



typedef struct tagKnownNodeDB {
    
    BOOL            bActive;
    BYTE            abyMACAddr[WLAN_ADDR_LEN];
    BYTE            abyCurrSuppRates[WLAN_IEHDR_LEN + WLAN_RATES_MAXLEN];
    BYTE            abyCurrExtSuppRates[WLAN_IEHDR_LEN + WLAN_RATES_MAXLEN];
    WORD            wTxDataRate;
    BOOL            bShortPreamble;
    BOOL            bERPExist;
    BOOL            bShortSlotTime;
    UINT            uInActiveCount;
    WORD            wMaxBasicRate;     
    WORD            wMaxSuppRate;      
    WORD            wSuppRate;
    BYTE            byTopOFDMBasicRate;
    BYTE            byTopCCKBasicRate; 

    
    struct sk_buff_head sTxPSQueue;
    WORD            wCapInfo;
    WORD            wListenInterval;
    WORD            wAID;
    NODE_STATE      eNodeState;
    BOOL            bPSEnable;
    BOOL            bRxPSPoll;
    BYTE            byAuthSequence;
    ULONG           ulLastRxJiffer;
    BYTE            bySuppRate;
    DWORD           dwFlags;
    WORD            wEnQueueCnt;

    BOOL            bOnFly;
    ULONGLONG       KeyRSC;
    BYTE            byKeyIndex;
    DWORD           dwKeyIndex;
    BYTE            byCipherSuite;
    DWORD           dwTSC47_16;
    WORD            wTSC15_0;
    UINT            uWepKeyLength;
    BYTE            abyWepKey[WLAN_WEPMAX_KEYLEN];
    
    
    BOOL            bIsInFallback;
    UINT            uAverageRSSI;
    UINT            uRateRecoveryTimeout;
    UINT            uRatePollTimeout;
    UINT            uTxFailures;
    UINT            uTxAttempts;

    UINT            uTxRetry;
    UINT            uFailureRatio;
    UINT            uRetryRatio;
    UINT            uTxOk[MAX_RATE+1];
    UINT            uTxFail[MAX_RATE+1];
    UINT            uTimeCount;

} KnownNodeDB, *PKnownNodeDB;






PKnownBSS
BSSpSearchBSSList(
    IN HANDLE hDeviceContext,
    IN PBYTE pbyDesireBSSID,
    IN PBYTE pbyDesireSSID,
    IN CARD_PHY_TYPE ePhyType
    );

PKnownBSS
BSSpAddrIsInBSSList(
    IN HANDLE hDeviceContext,
    IN PBYTE abyBSSID,
    IN PWLAN_IE_SSID pSSID
    );

VOID
BSSvClearBSSList(
    IN HANDLE hDeviceContext,
    IN BOOL bKeepCurrBSSID
    );

BOOL
BSSbInsertToBSSList(
    IN HANDLE hDeviceContext,
    IN PBYTE abyBSSIDAddr,
    IN QWORD qwTimestamp,
    IN WORD wBeaconInterval,
    IN WORD wCapInfo,
    IN BYTE byCurrChannel,
    IN PWLAN_IE_SSID pSSID,
    IN PWLAN_IE_SUPP_RATES pSuppRates,
    IN PWLAN_IE_SUPP_RATES pExtSuppRates,
    IN PERPObject psERP,
    IN PWLAN_IE_RSN pRSN,
    IN PWLAN_IE_RSN_EXT pRSNWPA,
    IN PWLAN_IE_COUNTRY pIE_Country,
    IN PWLAN_IE_QUIET pIE_Quiet,
    IN UINT uIELength,
    IN PBYTE pbyIEs,
    IN HANDLE pRxPacketContext
    );


BOOL
BSSbUpdateToBSSList(
    IN HANDLE hDeviceContext,
    IN QWORD qwTimestamp,
    IN WORD wBeaconInterval,
    IN WORD wCapInfo,
    IN BYTE byCurrChannel,
    IN BOOL bChannelHit,
    IN PWLAN_IE_SSID pSSID,
    IN PWLAN_IE_SUPP_RATES pSuppRates,
    IN PWLAN_IE_SUPP_RATES pExtSuppRates,
    IN PERPObject psERP,
    IN PWLAN_IE_RSN pRSN,
    IN PWLAN_IE_RSN_EXT pRSNWPA,
    IN PWLAN_IE_COUNTRY pIE_Country,
    IN PWLAN_IE_QUIET pIE_Quiet,
    IN PKnownBSS pBSSList,
    IN UINT uIELength,
    IN PBYTE pbyIEs,
    IN HANDLE pRxPacketContext
    );


BOOL
BSSDBbIsSTAInNodeDB(
    IN HANDLE hDeviceContext,
    IN PBYTE abyDstAddr,
    OUT PUINT puNodeIndex
    );

VOID
BSSvCreateOneNode(
    IN HANDLE hDeviceContext,
    OUT PUINT puNodeIndex
    );

VOID
BSSvUpdateAPNode(
    IN HANDLE hDeviceContext,
    IN PWORD pwCapInfo,
    IN PWLAN_IE_SUPP_RATES pItemRates,
    IN PWLAN_IE_SUPP_RATES pExtSuppRates
    );


VOID
BSSvSecondCallBack(
    IN HANDLE hDeviceContext
    );


VOID
BSSvUpdateNodeTxCounter(
    IN HANDLE hDeviceContext,
    IN BYTE        byTsr0,
    IN BYTE        byTsr1,
    IN PBYTE       pbyBuffer,
    IN UINT        uFIFOHeaderSize
    );

VOID
BSSvRemoveOneNode(
    IN HANDLE hDeviceContext,
    IN UINT uNodeIndex
    );

VOID
BSSvAddMulticastNode(
    IN HANDLE hDeviceContext
    );


VOID
BSSvClearNodeDBTable(
    IN HANDLE hDeviceContext,
    IN UINT uStartIndex
    );

VOID
BSSvClearAnyBSSJoinRecord(
    IN HANDLE hDeviceContext
    );

#endif 
