
#ifndef __AP_H__
#define __AP_H__








BOOLEAN APBridgeToWirelessSta(
    IN  PRTMP_ADAPTER   pAd,
    IN  PUCHAR          pHeader,
    IN  UINT            HdrLen,
    IN  PUCHAR          pData,
    IN  UINT            DataLen,
    IN  ULONG           fromwdsidx);


VOID	APSendPackets(
	IN	NDIS_HANDLE		MiniportAdapterContext,
	IN	PPNDIS_PACKET	ppPacketArray,
	IN	UINT			NumberOfPackets);

NDIS_STATUS APSendPacket(
    IN  PRTMP_ADAPTER   pAd,
    IN  PNDIS_PACKET    pPacket);


NDIS_STATUS APHardTransmit(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk,
	IN	UCHAR			QueIdx);

VOID APRxEAPOLFrameIndicate(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

NDIS_STATUS APCheckRxError(
	IN	PRTMP_ADAPTER	pAd,
	IN	PRT28XX_RXD_STRUC		pRxD,
	IN	UCHAR			Wcid);

BOOLEAN APCheckClass2Class3Error(
    IN  PRTMP_ADAPTER   pAd,
	IN ULONG Wcid,
	IN  PHEADER_802_11  pHeader);

VOID APHandleRxPsPoll(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pAddr,
	IN	USHORT			Aid,
    IN	BOOLEAN			isActive);

VOID    RTMPDescriptorEndianChange(
    IN  PUCHAR          pData,
    IN  ULONG           DescriptorType);

VOID    RTMPFrameEndianChange(
    IN  PRTMP_ADAPTER   pAd,
    IN  PUCHAR          pData,
    IN  ULONG           Dir,
    IN  BOOLEAN         FromRxDoneInt);



VOID APAssocStateMachineInit(
    IN  PRTMP_ADAPTER   pAd,
    IN  STATE_MACHINE *S,
    OUT STATE_MACHINE_FUNC Trans[]);

VOID  APPeerAssocReqAction(
    IN  PRTMP_ADAPTER   pAd,
    IN  MLME_QUEUE_ELEM *Elem);

VOID  APPeerReassocReqAction(
    IN  PRTMP_ADAPTER   pAd,
    IN  MLME_QUEUE_ELEM *Elem);

VOID  APPeerDisassocReqAction(
    IN  PRTMP_ADAPTER   pAd,
    IN  MLME_QUEUE_ELEM *Elem);

VOID MbssKickOutStas(
	IN PRTMP_ADAPTER pAd,
	IN INT apidx,
	IN USHORT Reason);

VOID APMlmeKickOutSta(
    IN PRTMP_ADAPTER pAd,
	IN PUCHAR pStaAddr,
	IN UCHAR Wcid,
	IN USHORT Reason);

VOID APMlmeDisassocReqAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID  APCls3errAction(
    IN  PRTMP_ADAPTER   pAd,
	IN	ULONG Wcid,
    IN	PHEADER_802_11	pHeader);


USHORT APBuildAssociation(
    IN PRTMP_ADAPTER pAd,
    IN MAC_TABLE_ENTRY *pEntry,
    IN USHORT CapabilityInfo,
    IN UCHAR  MaxSupportedRateIn500Kbps,
    IN UCHAR  *RSN,
    IN UCHAR  *pRSNLen,
    IN BOOLEAN bWmmCapable,
    IN ULONG  RalinkIe,
#ifdef DOT11N_DRAFT3
    IN EXT_CAP_INFO_ELEMENT ExtCapInfo,
#endif 
	IN HT_CAPABILITY_IE		*pHtCapability,
	IN UCHAR		 HtCapabilityLen,
    OUT USHORT *pAid);





void APAuthStateMachineInit(
    IN PRTMP_ADAPTER pAd,
    IN STATE_MACHINE *Sm,
    OUT STATE_MACHINE_FUNC Trans[]);

VOID APCls2errAction(
    IN PRTMP_ADAPTER pAd,
	IN	ULONG Wcid,
    IN	PHEADER_802_11	pHeader);




VOID APMakeBssBeacon(
    IN  PRTMP_ADAPTER   pAd,
	IN	INT				apidx);

VOID  APUpdateBeaconFrame(
    IN  PRTMP_ADAPTER   pAd,
	IN	INT				apidx);

VOID APMakeAllBssBeacon(
    IN  PRTMP_ADAPTER   pAd);

VOID  APUpdateAllBeaconFrame(
    IN  PRTMP_ADAPTER   pAd);




VOID APSyncStateMachineInit(
    IN PRTMP_ADAPTER pAd,
    IN STATE_MACHINE *Sm,
    OUT STATE_MACHINE_FUNC Trans[]);

VOID APScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID APInvalidStateWhenScan(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID APScanTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID APPeerProbeReqAction(
    IN  PRTMP_ADAPTER pAd,
    IN  MLME_QUEUE_ELEM *Elem);

VOID APPeerBeaconAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID APMlmeScanReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID APPeerBeaconAtScanAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID APScanCnclAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID ApSiteSurvey(
	IN	PRTMP_ADAPTER		pAd,
	IN	PNDIS_802_11_SSID	pSsid,
	IN	UCHAR				ScanType);

VOID SupportRate(
	IN PUCHAR SupRate,
	IN UCHAR SupRateLen,
	IN PUCHAR ExtRate,
	IN UCHAR ExtRateLen,
	OUT PUCHAR *Rates,
	OUT PUCHAR RatesLen,
	OUT PUCHAR pMaxSupportRate);


BOOLEAN ApScanRunning(
	IN PRTMP_ADAPTER pAd);

#ifdef DOT11N_DRAFT3
VOID APOverlappingBSSScan(
	IN RTMP_ADAPTER *pAd);
#endif 


VOID WpaStateMachineInit(
    IN  PRTMP_ADAPTER   pAd,
    IN  STATE_MACHINE *Sm,
    OUT STATE_MACHINE_FUNC Trans[]);


VOID APMlmePeriodicExec(
    IN  PRTMP_ADAPTER   pAd);

VOID APMlmeSelectTxRateTable(
	IN PRTMP_ADAPTER		pAd,
	IN PMAC_TABLE_ENTRY		pEntry,
	IN PUCHAR				*ppTable,
	IN PUCHAR				pTableSize,
	IN PUCHAR				pInitTxRateIdx);

VOID APMlmeSetTxRate(
	IN PRTMP_ADAPTER		pAd,
	IN PMAC_TABLE_ENTRY		pEntry,
	IN PRTMP_TX_RATE_SWITCH	pTxRate);

VOID APMlmeDynamicTxRateSwitching(
    IN PRTMP_ADAPTER pAd);

VOID APQuickResponeForRateUpExec(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3);

BOOLEAN APMsgTypeSubst(
    IN PRTMP_ADAPTER pAd,
    IN PFRAME_802_11 pFrame,
    OUT INT *Machine,
    OUT INT *MsgType);

VOID APQuickResponeForRateUpExec(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3);


VOID RTMPSetPiggyBack(
	IN PRTMP_ADAPTER	pAd,
	IN BOOLEAN			bPiggyBack);

VOID APAsicEvaluateRxAnt(
	IN PRTMP_ADAPTER	pAd);

VOID APAsicRxAntEvalTimeout(
	IN PRTMP_ADAPTER	pAd);



VOID APSwitchChannel(
	IN PRTMP_ADAPTER pAd,
	IN INT Channel);

NDIS_STATUS APInitialize(
    IN  PRTMP_ADAPTER   pAd);

VOID APShutdown(
    IN PRTMP_ADAPTER    pAd);

VOID APStartUp(
    IN  PRTMP_ADAPTER   pAd);

VOID APStop(
    IN  PRTMP_ADAPTER   pAd);

VOID APCleanupPsQueue(
    IN  PRTMP_ADAPTER   pAd,
    IN  PQUEUE_HEADER   pQueue);

VOID MacTableReset(
    IN  PRTMP_ADAPTER   pAd);

MAC_TABLE_ENTRY *MacTableInsertEntry(
    IN  PRTMP_ADAPTER   pAd,
    IN  PUCHAR          pAddr,
	IN	UCHAR			apidx,
	IN BOOLEAN	CleanAll);

BOOLEAN MacTableDeleteEntry(
    IN  PRTMP_ADAPTER   pAd,
	IN USHORT wcid,
    IN  PUCHAR          pAddr);

MAC_TABLE_ENTRY *MacTableLookup(
    IN  PRTMP_ADAPTER   pAd,
    IN  PUCHAR          pAddr);

VOID MacTableMaintenance(
    IN PRTMP_ADAPTER pAd);

UINT32 MacTableAssocStaNumGet(
	IN PRTMP_ADAPTER pAd);

MAC_TABLE_ENTRY *APSsPsInquiry(
    IN  PRTMP_ADAPTER   pAd,
    IN  PUCHAR          pAddr,
    OUT SST             *Sst,
    OUT USHORT          *Aid,
    OUT UCHAR           *PsMode,
    OUT UCHAR           *Rate);

BOOLEAN APPsIndicate(
    IN  PRTMP_ADAPTER   pAd,
    IN  PUCHAR          pAddr,
	IN ULONG Wcid,
    IN  UCHAR           Psm);

VOID ApLogEvent(
    IN PRTMP_ADAPTER    pAd,
    IN PUCHAR           pAddr,
    IN USHORT           Event);

#ifdef DOT11_N_SUPPORT
VOID APUpdateOperationMode(
    IN PRTMP_ADAPTER pAd);
#endif 

VOID APUpdateCapabilityAndErpIe(
	IN PRTMP_ADAPTER pAd);

BOOLEAN ApCheckAccessControlList(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR        pAddr,
	IN UCHAR         Apidx);

VOID ApUpdateAccessControlList(
    IN PRTMP_ADAPTER pAd,
    IN UCHAR         Apidx);

VOID ApEnqueueNullFrame(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR        pAddr,
	IN UCHAR         TxRate,
	IN UCHAR         PID,
	IN UCHAR         apidx,
    IN BOOLEAN       bQosNull,
    IN BOOLEAN       bEOSP,
    IN UCHAR         OldUP);

VOID ApSendFrame(
    IN  PRTMP_ADAPTER   pAd,
    IN  PVOID           pBuffer,
    IN  ULONG           Length,
    IN  UCHAR           TxRate,
    IN  UCHAR           PID);

VOID ApEnqueueAckFrame(
    IN PRTMP_ADAPTER pAd,
    IN PUCHAR        pAddr,
    IN UCHAR         TxRate,
	IN UCHAR         apidx);




BOOLEAN PeerAssocReqCmmSanity(
    IN PRTMP_ADAPTER pAd,
	IN BOOLEAN isRessoc,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2,
    OUT USHORT *pCapabilityInfo,
    OUT USHORT *pListenInterval,
    OUT PUCHAR pApAddr,
    OUT UCHAR *pSsidLen,
    OUT char *Ssid,
    OUT UCHAR *pRatesLen,
    OUT UCHAR Rates[],
    OUT UCHAR *RSN,
    OUT UCHAR *pRSNLen,
    OUT BOOLEAN *pbWmmCapable,
    OUT ULONG  *pRalinkIe,
#ifdef DOT11N_DRAFT3
    OUT EXT_CAP_INFO_ELEMENT	*pExtCapInfo,
#endif 
    OUT UCHAR		 *pHtCapabilityLen,
    OUT HT_CAPABILITY_IE *pHtCapability);


BOOLEAN PeerDisassocReqSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2,
    OUT USHORT *Reason);

BOOLEAN PeerDeauthReqSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2,
    OUT USHORT *Reason);

BOOLEAN APPeerAuthSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
	OUT PUCHAR pAddr1,
    OUT PUCHAR pAddr2,
    OUT USHORT *Alg,
    OUT USHORT *Seq,
    OUT USHORT *Status,
    OUT CHAR *ChlgText
	);

BOOLEAN APPeerProbeReqSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2,
    OUT CHAR Ssid[],
    OUT UCHAR *SsidLen);

BOOLEAN APPeerBeaconAndProbeRspSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2,
    OUT PUCHAR pBssid,
    OUT CHAR Ssid[],
    OUT UCHAR *SsidLen,
    OUT UCHAR *BssType,
    OUT USHORT *BeaconPeriod,
    OUT UCHAR *Channel,
    OUT LARGE_INTEGER *Timestamp,
    OUT USHORT *CapabilityInfo,
    OUT UCHAR Rate[],
    OUT UCHAR *RateLen,
    OUT BOOLEAN *ExtendedRateIeExist,
    OUT UCHAR *Erp);
#if defined(RT30xx) || defined(RT305x)
VOID EnableAPMIMOPS(
    IN PRTMP_ADAPTER pAd);

VOID DisableAPMIMOPS(
    IN PRTMP_ADAPTER pAd);
#endif
#endif  
