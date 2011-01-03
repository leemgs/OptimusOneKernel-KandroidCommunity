

#ifndef __MIB_H__
#define __MIB_H__

#include "ttype.h"
#include "tether.h"
#include "desc.h"







typedef struct tagSUSBCounter {
    DWORD dwCrc;

} SUSBCounter, *PSUSBCounter;








typedef struct tagSDot11Counters {

    ULONGLONG   TransmittedFragmentCount;
    ULONGLONG   MulticastTransmittedFrameCount;
    ULONGLONG   FailedCount;
    ULONGLONG   RetryCount;
    ULONGLONG   MultipleRetryCount;
    ULONGLONG   RTSSuccessCount;
    ULONGLONG   RTSFailureCount;
    ULONGLONG   ACKFailureCount;
    ULONGLONG   FrameDuplicateCount;
    ULONGLONG   ReceivedFragmentCount;
    ULONGLONG   MulticastReceivedFrameCount;
    ULONGLONG   FCSErrorCount;
    ULONGLONG   TKIPLocalMICFailures;
    ULONGLONG   TKIPRemoteMICFailures;
    ULONGLONG   TKIPICVErrors;
    ULONGLONG   TKIPCounterMeasuresInvoked;
    ULONGLONG   TKIPReplays;
    ULONGLONG   CCMPFormatErrors;
    ULONGLONG   CCMPReplays;
    ULONGLONG   CCMPDecryptErrors;
    ULONGLONG   FourWayHandshakeFailures;




} SDot11Counters, *PSDot11Counters;





typedef struct tagSMib2Counter {
    LONG    ifIndex;
    char    ifDescr[256];               
                                        
    LONG    ifType;
    LONG    ifMtu;
    DWORD   ifSpeed;
    BYTE    ifPhysAddress[U_ETHER_ADDR_LEN];
    LONG    ifAdminStatus;
    LONG    ifOperStatus;
    DWORD   ifLastChange;
    DWORD   ifInOctets;
    DWORD   ifInUcastPkts;
    DWORD   ifInNUcastPkts;
    DWORD   ifInDiscards;
    DWORD   ifInErrors;
    DWORD   ifInUnknownProtos;
    DWORD   ifOutOctets;
    DWORD   ifOutUcastPkts;
    DWORD   ifOutNUcastPkts;
    DWORD   ifOutDiscards;
    DWORD   ifOutErrors;
    DWORD   ifOutQLen;
    DWORD   ifSpecific;
} SMib2Counter, *PSMib2Counter;



#define WIRELESSLANIEEE80211b      6           


#define UP                  1           
#define DOWN                2           
#define TESTING             3           





typedef struct tagSRmonCounter {
    LONG    etherStatsIndex;
    DWORD   etherStatsDataSource;
    DWORD   etherStatsDropEvents;
    DWORD   etherStatsOctets;
    DWORD   etherStatsPkts;
    DWORD   etherStatsBroadcastPkts;
    DWORD   etherStatsMulticastPkts;
    DWORD   etherStatsCRCAlignErrors;
    DWORD   etherStatsUndersizePkts;
    DWORD   etherStatsOversizePkts;
    DWORD   etherStatsFragments;
    DWORD   etherStatsJabbers;
    DWORD   etherStatsCollisions;
    DWORD   etherStatsPkt64Octets;
    DWORD   etherStatsPkt65to127Octets;
    DWORD   etherStatsPkt128to255Octets;
    DWORD   etherStatsPkt256to511Octets;
    DWORD   etherStatsPkt512to1023Octets;
    DWORD   etherStatsPkt1024to1518Octets;
    DWORD   etherStatsOwners;
    DWORD   etherStatsStatus;
} SRmonCounter, *PSRmonCounter;




typedef struct tagSCustomCounters {
    ULONG       Length;

    ULONGLONG   ullTsrAllOK;

    ULONGLONG   ullRsr11M;
    ULONGLONG   ullRsr5M;
    ULONGLONG   ullRsr2M;
    ULONGLONG   ullRsr1M;

    ULONGLONG   ullRsr11MCRCOk;
    ULONGLONG   ullRsr5MCRCOk;
    ULONGLONG   ullRsr2MCRCOk;
    ULONGLONG   ullRsr1MCRCOk;

    ULONGLONG   ullRsr54M;
    ULONGLONG   ullRsr48M;
    ULONGLONG   ullRsr36M;
    ULONGLONG   ullRsr24M;
    ULONGLONG   ullRsr18M;
    ULONGLONG   ullRsr12M;
    ULONGLONG   ullRsr9M;
    ULONGLONG   ullRsr6M;

    ULONGLONG   ullRsr54MCRCOk;
    ULONGLONG   ullRsr48MCRCOk;
    ULONGLONG   ullRsr36MCRCOk;
    ULONGLONG   ullRsr24MCRCOk;
    ULONGLONG   ullRsr18MCRCOk;
    ULONGLONG   ullRsr12MCRCOk;
    ULONGLONG   ullRsr9MCRCOk;
    ULONGLONG   ullRsr6MCRCOk;

} SCustomCounters, *PSCustomCounters;





typedef struct tagSISRCounters {
    ULONG   Length;

    DWORD   dwIsrTx0OK;
    DWORD   dwIsrAC0TxOK;
    DWORD   dwIsrBeaconTxOK;
    DWORD   dwIsrRx0OK;
    DWORD   dwIsrTBTTInt;
    DWORD   dwIsrSTIMERInt;
    DWORD   dwIsrWatchDog;
    DWORD   dwIsrUnrecoverableError;
    DWORD   dwIsrSoftInterrupt;
    DWORD   dwIsrMIBNearfull;
    DWORD   dwIsrRxNoBuf;

    DWORD   dwIsrUnknown;               

    DWORD   dwIsrRx1OK;
    DWORD   dwIsrATIMTxOK;
    DWORD   dwIsrSYNCTxOK;
    DWORD   dwIsrCFPEnd;
    DWORD   dwIsrATIMEnd;
    DWORD   dwIsrSYNCFlushOK;
    DWORD   dwIsrSTIMER1Int;
    
} SISRCounters, *PSISRCounters;



#define VALID               1           
#define CREATE_REQUEST      2           
#define UNDER_CREATION      3           
#define INVALID             4           





typedef struct tagSTxPktInfo {
    BYTE    byBroadMultiUni;
    WORD    wLength;
    WORD    wFIFOCtl;
    BYTE    abyDestAddr[U_ETHER_ADDR_LEN];
} STxPktInfo, *PSTxPktInfo;


#define MAX_RATE            12



typedef struct tagSStatCounter {
    
    
    

    SISRCounters ISRStat;

    
    
    DWORD   dwRsrFrmAlgnErr;
    DWORD   dwRsrErr;
    DWORD   dwRsrCRCErr;
    DWORD   dwRsrCRCOk;
    DWORD   dwRsrBSSIDOk;
    DWORD   dwRsrADDROk;
    DWORD   dwRsrBCNSSIDOk;
    DWORD   dwRsrLENErr;
    DWORD   dwRsrTYPErr;

    DWORD   dwNewRsrDECRYPTOK;
    DWORD   dwNewRsrCFP;
    DWORD   dwNewRsrUTSF;
    DWORD   dwNewRsrHITAID;
    DWORD   dwNewRsrHITAID0;

    DWORD   dwRsrLong;
    DWORD   dwRsrRunt;

    DWORD   dwRsrRxControl;
    DWORD   dwRsrRxData;
    DWORD   dwRsrRxManage;

    DWORD   dwRsrRxPacket;
    DWORD   dwRsrRxOctet;
    DWORD   dwRsrBroadcast;
    DWORD   dwRsrMulticast;
    DWORD   dwRsrDirected;
    
    ULONGLONG   ullRsrOK;

    
    ULONGLONG   ullRxBroadcastBytes;
    ULONGLONG   ullRxMulticastBytes;
    ULONGLONG   ullRxDirectedBytes;
    ULONGLONG   ullRxBroadcastFrames;
    ULONGLONG   ullRxMulticastFrames;
    ULONGLONG   ullRxDirectedFrames;

    DWORD   dwRsrRxFragment;
    DWORD   dwRsrRxFrmLen64;
    DWORD   dwRsrRxFrmLen65_127;
    DWORD   dwRsrRxFrmLen128_255;
    DWORD   dwRsrRxFrmLen256_511;
    DWORD   dwRsrRxFrmLen512_1023;
    DWORD   dwRsrRxFrmLen1024_1518;

    
    
    DWORD   dwTsrTotalRetry;        
    DWORD   dwTsrOnceRetry;         
    DWORD   dwTsrMoreThanOnceRetry; 
    DWORD   dwTsrRetry;             
                                         
    DWORD   dwTsrACKData;
    DWORD   dwTsrErr;
    DWORD   dwAllTsrOK;
    DWORD   dwTsrRetryTimeout;
    DWORD   dwTsrTransmitTimeout;

    DWORD   dwTsrTxPacket;
    DWORD   dwTsrTxOctet;
    DWORD   dwTsrBroadcast;
    DWORD   dwTsrMulticast;
    DWORD   dwTsrDirected;

    
    DWORD   dwCntRxFrmLength;
    DWORD   dwCntTxBufLength;

    BYTE    abyCntRxPattern[16];
    BYTE    abyCntTxPattern[16];



    
    DWORD   dwCntRxDataErr;             
    DWORD   dwCntDecryptErr;            
    DWORD   dwCntRxICVErr;              


    
    ULONGLONG   ullTsrOK;

    
    ULONGLONG   ullTxBroadcastFrames;
    ULONGLONG   ullTxMulticastFrames;
    ULONGLONG   ullTxDirectedFrames;
    ULONGLONG   ullTxBroadcastBytes;
    ULONGLONG   ullTxMulticastBytes;
    ULONGLONG   ullTxDirectedBytes;

    
    DWORD   dwTxOk[MAX_RATE+1];
    DWORD   dwTxFail[MAX_RATE+1];
    DWORD   dwTxRetryCount[8];

    STxPktInfo  abyTxPktInfo[16];

    SUSBCounter USB_EP0Stat;
    SUSBCounter USB_BulkInStat;
    SUSBCounter USB_BulkOutStat;
    SUSBCounter USB_InterruptStat;

    SCustomCounters CustomStat;

   #ifdef Calcu_LinkQual
       
    ULONG TxNoRetryOkCount;         
    ULONG TxRetryOkCount;              
    ULONG TxFailCount;                      
      
    ULONG RxOkCnt;                          
    ULONG RxFcsErrCnt;                    
      
    ULONG SignalStren;
    ULONG LinkQuality;
   #endif

} SStatCounter, *PSStatCounter;

#define NTSTATUS        int







void STAvClearAllCounter(PSStatCounter pStatistic);

void STAvUpdateIsrStatCounter (PSStatCounter pStatistic, BYTE byIsr0, BYTE byIsr1);

void STAvUpdateRDStatCounter(PSStatCounter pStatistic,
                              BYTE byRSR, BYTE byNewRSR, BYTE byRxSts, BYTE byRxRate,
                              PBYTE pbyBuffer, UINT cbFrameLength);

void STAvUpdateRDStatCounterEx(PSStatCounter pStatistic,
                              BYTE byRSR, BYTE byNewRSR, BYTE byRxSts, BYTE byRxRate,
                              PBYTE pbyBuffer, UINT cbFrameLength);


void
STAvUpdateTDStatCounter (
    PSStatCounter   pStatistic,
    BYTE            byPktNum,
    BYTE            byRate,
    BYTE            byTSR
    );


void
STAvUpdate802_11Counter(
    PSDot11Counters         p802_11Counter,
    PSStatCounter           pStatistic,
    BYTE                    byRTSSuccess,
    BYTE                    byRTSFail,
    BYTE                    byACKFail,
    BYTE                    byFCSErr
    );

void STAvClear802_11Counter(PSDot11Counters p802_11Counter);

void
STAvUpdateUSBCounter(
    PSUSBCounter    pUsbCounter,
    NTSTATUS        ntStatus
    );

#endif 



