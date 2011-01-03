

#include "../rt_config.h"


#define ADHOC_ENTRY_BEACON_LOST_TIME	(2*OS_HZ)	


VOID SyncStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, Trans, MAX_SYNC_STATE, MAX_SYNC_MSG, (STATE_MACHINE_FUNC)Drop, SYNC_IDLE, SYNC_MACHINE_BASE);

	
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_MLME_SCAN_REQ, (STATE_MACHINE_FUNC)MlmeScanReqAction);
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_MLME_JOIN_REQ, (STATE_MACHINE_FUNC)MlmeJoinReqAction);
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_MLME_START_REQ, (STATE_MACHINE_FUNC)MlmeStartReqAction);
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_PEER_BEACON, (STATE_MACHINE_FUNC)PeerBeacon);
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_PEER_PROBE_REQ, (STATE_MACHINE_FUNC)PeerProbeReqAction);

	
	StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_MLME_SCAN_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenScan);
	StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_MLME_JOIN_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenJoin);
	StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_MLME_START_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenStart);
	StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_PEER_BEACON, (STATE_MACHINE_FUNC)PeerBeaconAtJoinAction);
	StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_BEACON_TIMEOUT, (STATE_MACHINE_FUNC)BeaconTimeoutAtJoinAction);

	
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_MLME_SCAN_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenScan);
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_MLME_JOIN_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenJoin);
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_MLME_START_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenStart);
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_PEER_BEACON, (STATE_MACHINE_FUNC)PeerBeaconAtScanAction);
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_PEER_PROBE_RSP, (STATE_MACHINE_FUNC)PeerBeaconAtScanAction);
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_SCAN_TIMEOUT, (STATE_MACHINE_FUNC)ScanTimeoutAction);

	
	RTMPInitTimer(pAd, &pAd->MlmeAux.BeaconTimer, GET_TIMER_FUNCTION(BeaconTimeout), pAd, FALSE);
	RTMPInitTimer(pAd, &pAd->MlmeAux.ScanTimer, GET_TIMER_FUNCTION(ScanTimeout), pAd, FALSE);
}


VOID BeaconTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	DBGPRINT(RT_DEBUG_TRACE,("SYNC - BeaconTimeout\n"));

	
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
		return;

#ifdef DOT11_N_SUPPORT
	if ((pAd->CommonCfg.BBPCurrentBW == BW_40)
		)
	{
		UCHAR        BBPValue = 0;
		AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
		AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
		BBPValue &= (~0x18);
		BBPValue |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - End of SCAN, restore to 40MHz channel %d, Total BSS[%02d]\n",pAd->CommonCfg.CentralChannel, pAd->ScanTab.BssNr));
	}
#endif 

	MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_BEACON_TIMEOUT, 0, NULL);
	RTMP_MLME_HANDLER(pAd);
}


VOID ScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;


	
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
		return;

	if (MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_SCAN_TIMEOUT, 0, NULL))
	{
	RTMP_MLME_HANDLER(pAd);
}
	else
	{
		
		pAd->MlmeAux.Channel = 0;
		ScanNextChannel(pAd);
		if (pAd->CommonCfg.bWirelessEvent)
		{
			RTMPSendWirelessEvent(pAd, IW_SCAN_ENQUEUE_FAIL_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
		}
	}
}


VOID MlmeScanReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR          Ssid[MAX_LEN_OF_SSID], SsidLen, ScanType, BssType, BBPValue = 0;
	BOOLEAN        TimerCancelled;
	ULONG		   Now;
	USHORT         Status;
	PHEADER_802_11 pHdr80211;
	PUCHAR         pOutBuffer = NULL;
	NDIS_STATUS    NStatus;

	
	
	if ( !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - MlmeScanReqAction before Startup\n"));
		return;
	}

	
	pAd->StaCfg.ScanCnt++;

#ifdef RTMP_MAC_PCI
    if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE)) &&
        (IDLE_ON(pAd)) &&
		(pAd->StaCfg.bRadio == TRUE) &&
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)))
	{
	        if (pAd->StaCfg.PSControl.field.EnableNewPS == FALSE)
		{
			AsicSendCommandToMcu(pAd, 0x31, PowerWakeCID, 0x00, 0x02);
			AsicCheckCommanOk(pAd, PowerWakeCID);
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
			DBGPRINT(RT_DEBUG_TRACE, ("PSM - Issue Wake up command \n"));
		}
		else
		{
		RT28xxPciAsicRadioOn(pAd, GUI_IDLE_POWER_SAVE);
	}
	}
#endif 

	
	if (MlmeScanReqSanity(pAd,
						  Elem->Msg,
						  Elem->MsgLen,
						  &BssType,
						  (PCHAR)Ssid,
						  &SsidLen,
						  &ScanType))
	{

		
		
		
		RTMPSuspendMsduTransmission(pAd);

		
		
		
		
		
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) && (INFRA_ON(pAd)))
		{
			NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);
			if (NStatus	== NDIS_STATUS_SUCCESS)
			{
				pHdr80211 = (PHEADER_802_11) pOutBuffer;
				MgtMacHeaderInit(pAd, pHdr80211, SUBTYPE_NULL_FUNC, 1, pAd->CommonCfg.Bssid, pAd->CommonCfg.Bssid);
				pHdr80211->Duration = 0;
				pHdr80211->FC.Type = BTYPE_DATA;
				pHdr80211->FC.PwrMgmt = PWR_SAVE;

				
				MiniportMMRequest(pAd, 0, pOutBuffer, sizeof(HEADER_802_11));
				DBGPRINT(RT_DEBUG_TRACE, ("MlmeScanReqAction -- Send PSM Data frame for off channel RM\n"));
				MlmeFreeMemory(pAd, pOutBuffer);
				RTMPusecDelay(5000);
			}
		}

		NdisGetSystemUpTime(&Now);
		pAd->StaCfg.LastScanTime = Now;
		
		RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer, &TimerCancelled);
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &TimerCancelled);

		
		pAd->MlmeAux.BssType = BssType;
		pAd->MlmeAux.ScanType = ScanType;
		pAd->MlmeAux.SsidLen = SsidLen;
        NdisZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAd->MlmeAux.Ssid, Ssid, SsidLen);

		
		pAd->MlmeAux.Channel = FirstChannel(pAd);

		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
		BBPValue &= (~0x18);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - BBP R4 to 20MHz.l\n"));
		ScanNextChannel(pAd);
	}
	else
	{
		DBGPRINT_ERR(("SYNC - MlmeScanReqAction() sanity check fail\n"));
		pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
		Status = MLME_INVALID_FORMAT;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status);
	}
}


VOID MlmeJoinReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR        BBPValue = 0;
	BSS_ENTRY    *pBss;
	BOOLEAN       TimerCancelled;
	HEADER_802_11 Hdr80211;
	NDIS_STATUS   NStatus;
	ULONG         FrameLen = 0;
	PUCHAR        pOutBuffer = NULL;
	PUCHAR        pSupRate = NULL;
	UCHAR         SupRateLen;
	PUCHAR        pExtRate = NULL;
	UCHAR         ExtRateLen;
	UCHAR         ASupRate[] = {0x8C, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6C};
	UCHAR         ASupRateLen = sizeof(ASupRate)/sizeof(UCHAR);
	MLME_JOIN_REQ_STRUCT *pInfo = (MLME_JOIN_REQ_STRUCT *)(Elem->Msg);

	DBGPRINT(RT_DEBUG_TRACE, ("SYNC - MlmeJoinReqAction(BSS #%ld)\n", pInfo->BssIdx));

#ifdef RTMP_MAC_PCI
    if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE)) &&
        (IDLE_ON(pAd)) &&
		(pAd->StaCfg.bRadio == TRUE) &&
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)))
	{
		RT28xxPciAsicRadioOn(pAd, GUI_IDLE_POWER_SAVE);
	}
#endif 

	
	RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &TimerCancelled);
	RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer, &TimerCancelled);

	pBss = &pAd->MlmeAux.SsidBssTab.BssEntry[pInfo->BssIdx];

	
	COPY_MAC_ADDR(pAd->MlmeAux.Bssid, pBss->Bssid);

	
	if (pBss->Hidden == 0)
	{
		RTMPZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAd->MlmeAux.Ssid, pBss->Ssid, pBss->SsidLen);
	pAd->MlmeAux.SsidLen = pBss->SsidLen;
	}

	pAd->MlmeAux.BssType = pBss->BssType;
	pAd->MlmeAux.Channel = pBss->Channel;
	pAd->MlmeAux.CentralChannel = pBss->CentralChannel;

#ifdef EXT_BUILD_CHANNEL_LIST
	
	if ((pAd->StaCfg.IEEE80211dClientMode != Rt802_11_D_None) &&
		(pBss->bHasCountryIE == TRUE))
	{
		NdisMoveMemory(&pAd->CommonCfg.CountryCode[0], &pBss->CountryString[0], 2);
		if (pBss->CountryString[2] == 'I')
			pAd->CommonCfg.Geography = IDOR;
		else if (pBss->CountryString[2] == 'O')
			pAd->CommonCfg.Geography = ODOR;
		else
			pAd->CommonCfg.Geography = BOTH;
		BuildChannelListEx(pAd);
	}
#endif 

	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
	BBPValue &= (~0x18);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
	DBGPRINT(RT_DEBUG_TRACE, ("SYNC - BBP R4 to 20MHz.l\n"));

	
	AsicSwitchChannel(pAd, pAd->MlmeAux.Channel, FALSE);
	AsicLockChannel(pAd, pAd->MlmeAux.Channel);


	RTMPSetTimer(&pAd->MlmeAux.BeaconTimer, JOIN_TIMEOUT);

    do
	{
		if (((pAd->CommonCfg.bIEEE80211H == 1) &&
            (pAd->MlmeAux.Channel > 14) &&
             RadarChannelCheck(pAd, pAd->MlmeAux.Channel))
#ifdef CARRIER_DETECTION_SUPPORT 
             || (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
#endif 
            )
		{
			
			
			
			if (pBss->Hidden == 0)
			break;
		}

	
	
	
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus == NDIS_STATUS_SUCCESS)
	{
		if (pAd->MlmeAux.Channel <= 14)
		{
			pSupRate = pAd->CommonCfg.SupRate;
			SupRateLen = pAd->CommonCfg.SupRateLen;
			pExtRate = pAd->CommonCfg.ExtRate;
			ExtRateLen = pAd->CommonCfg.ExtRateLen;
		}
		else
		{
			
			
			
			pSupRate = ASupRate;
			SupRateLen = ASupRateLen;
			ExtRateLen = 0;
		}

		if (pAd->MlmeAux.BssType == BSS_INFRA)
			MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, pAd->MlmeAux.Bssid, pAd->MlmeAux.Bssid);
		else
			MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR, BROADCAST_ADDR);

		MakeOutgoingFrame(pOutBuffer,               &FrameLen,
						  sizeof(HEADER_802_11),    &Hdr80211,
						  1,                        &SsidIe,
						  1,                        &pAd->MlmeAux.SsidLen,
						  pAd->MlmeAux.SsidLen,	    pAd->MlmeAux.Ssid,
						  1,                        &SupRateIe,
						  1,                        &SupRateLen,
						  SupRateLen,               pSupRate,
						  END_OF_ARGS);

		if (ExtRateLen)
		{
			ULONG Tmp;
			MakeOutgoingFrame(pOutBuffer + FrameLen,            &Tmp,
							  1,                                &ExtRateIe,
							  1,                                &ExtRateLen,
							  ExtRateLen,                       pExtRate,
							  END_OF_ARGS);
			FrameLen += Tmp;
		}


		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);
	}
    } while (FALSE);

	DBGPRINT(RT_DEBUG_TRACE, ("SYNC - Switch to ch %d, Wait BEACON from %02x:%02x:%02x:%02x:%02x:%02x\n",
		pBss->Channel, pBss->Bssid[0], pBss->Bssid[1], pBss->Bssid[2], pBss->Bssid[3], pBss->Bssid[4], pBss->Bssid[5]));

	pAd->Mlme.SyncMachine.CurrState = JOIN_WAIT_BEACON;
}


VOID MlmeStartReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR         Ssid[MAX_LEN_OF_SSID], SsidLen;
	BOOLEAN       TimerCancelled;

	
	UCHAR						VarIE[MAX_VIE_LEN];	
	NDIS_802_11_VARIABLE_IEs	*pVIE = NULL;
	LARGE_INTEGER				TimeStamp;
	BOOLEAN Privacy;
	USHORT Status;

	
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;
	TimeStamp.u.LowPart  = 0;
	TimeStamp.u.HighPart = 0;

	if (MlmeStartReqSanity(pAd, Elem->Msg, Elem->MsgLen, (PCHAR)Ssid, &SsidLen))
	{
		
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &TimerCancelled);
		RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer, &TimerCancelled);

		
		
		
		DBGPRINT(RT_DEBUG_TRACE, ("MlmeStartReqAction - Start a new IBSS. All IBSS parameters are decided now.... \n"));
		pAd->MlmeAux.BssType           = BSS_ADHOC;
		NdisMoveMemory(pAd->MlmeAux.Ssid, Ssid, SsidLen);
		pAd->MlmeAux.SsidLen           = SsidLen;

		
		MacAddrRandomBssid(pAd, pAd->MlmeAux.Bssid);
		DBGPRINT(RT_DEBUG_TRACE, ("MlmeStartReqAction - generate a radom number as BSSID \n"));

		Privacy = (pAd->StaCfg.WepStatus == Ndis802_11Encryption1Enabled) ||
				  (pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) ||
				  (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled);
		pAd->MlmeAux.CapabilityInfo    = CAP_GENERATE(0,1,Privacy, (pAd->CommonCfg.TxPreamble == Rt802_11PreambleShort), 1, 0);
		pAd->MlmeAux.BeaconPeriod      = pAd->CommonCfg.BeaconPeriod;
		pAd->MlmeAux.AtimWin           = pAd->StaCfg.AtimWin;
		pAd->MlmeAux.Channel           = pAd->CommonCfg.Channel;

		pAd->CommonCfg.CentralChannel  = pAd->CommonCfg.Channel;
		pAd->MlmeAux.CentralChannel    = pAd->CommonCfg.CentralChannel;

		pAd->MlmeAux.SupRateLen= pAd->CommonCfg.SupRateLen;
		NdisMoveMemory(pAd->MlmeAux.SupRate, pAd->CommonCfg.SupRate, MAX_LEN_OF_SUPPORTED_RATES);
		RTMPCheckRates(pAd, pAd->MlmeAux.SupRate, &pAd->MlmeAux.SupRateLen);
		pAd->MlmeAux.ExtRateLen = pAd->CommonCfg.ExtRateLen;
		NdisMoveMemory(pAd->MlmeAux.ExtRate, pAd->CommonCfg.ExtRate, MAX_LEN_OF_SUPPORTED_RATES);
		RTMPCheckRates(pAd, pAd->MlmeAux.ExtRate, &pAd->MlmeAux.ExtRateLen);
#ifdef DOT11_N_SUPPORT
		if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
		{
			RTMPUpdateHTIE(&pAd->CommonCfg.DesiredHtPhy, &pAd->StaCfg.DesiredHtPhyInfo.MCSSet[0], &pAd->MlmeAux.HtCapability, &pAd->MlmeAux.AddHtInfo);
			pAd->MlmeAux.HtCapabilityLen = sizeof(HT_CAPABILITY_IE);
			
			DBGPRINT(RT_DEBUG_TRACE, ("SYNC -pAd->StaActive.SupportedHtPhy.bHtEnable = TRUE\n"));
		}
		else
#endif 
		{
			pAd->MlmeAux.HtCapabilityLen = 0;
			pAd->StaActive.SupportedPhyInfo.bHtEnable = FALSE;
			NdisZeroMemory(&pAd->StaActive.SupportedPhyInfo.MCSSet[0], 16);
		}
		
		NdisZeroMemory(&pAd->MlmeAux.APEdcaParm, sizeof(EDCA_PARM));
		NdisZeroMemory(&pAd->MlmeAux.APQbssLoad, sizeof(QBSS_LOAD_PARM));
		NdisZeroMemory(&pAd->MlmeAux.APQosCapability, sizeof(QOS_CAPABILITY_PARM));

		AsicSwitchChannel(pAd, pAd->MlmeAux.Channel, FALSE);
		AsicLockChannel(pAd, pAd->MlmeAux.Channel);

		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - MlmeStartReqAction(ch= %d,sup rates= %d, ext rates=%d)\n",
			pAd->MlmeAux.Channel, pAd->MlmeAux.SupRateLen, pAd->MlmeAux.ExtRateLen));

		pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
		Status = MLME_SUCCESS;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_START_CONF, 2, &Status);
	}
	else
	{
		DBGPRINT_ERR(("SYNC - MlmeStartReqAction() sanity check fail.\n"));
		pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
		Status = MLME_INVALID_FORMAT;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_START_CONF, 2, &Status);
	}
}


VOID PeerBeaconAtScanAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR           Bssid[MAC_ADDR_LEN], Addr2[MAC_ADDR_LEN];
	UCHAR           Ssid[MAX_LEN_OF_SSID], BssType, Channel, NewChannel,
					SsidLen, DtimCount, DtimPeriod, BcastFlag, MessageToMe;
	CF_PARM         CfParm;
	USHORT          BeaconPeriod, AtimWin, CapabilityInfo;
	PFRAME_802_11   pFrame;
	LARGE_INTEGER   TimeStamp;
	UCHAR           Erp;
	UCHAR		SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR			SupRateLen, ExtRateLen;
	USHORT			LenVIE;
	UCHAR			CkipFlag;
	UCHAR			AironetCellPowerLimit;
	EDCA_PARM       EdcaParm;
	QBSS_LOAD_PARM  QbssLoad;
	QOS_CAPABILITY_PARM QosCapability;
	ULONG						RalinkIe;
	UCHAR						VarIE[MAX_VIE_LEN];		
	NDIS_802_11_VARIABLE_IEs	*pVIE = NULL;
	HT_CAPABILITY_IE		HtCapability;
	ADD_HT_INFO_IE		AddHtInfo;	
	UCHAR			HtCapabilityLen = 0, PreNHtCapabilityLen = 0;
	UCHAR			AddHtInfoLen;
	UCHAR			NewExtChannelOffset = 0xff;


	
	pFrame = (PFRAME_802_11) Elem->Msg;
	
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;
#ifdef DOT11_N_SUPPORT
    RTMPZeroMemory(&HtCapability, sizeof(HtCapability));
	RTMPZeroMemory(&AddHtInfo, sizeof(ADD_HT_INFO_IE));
#endif 

	if (PeerBeaconAndProbeRspSanity(pAd,
								Elem->Msg,
								Elem->MsgLen,
								Elem->Channel,
								Addr2,
								Bssid,
								(PCHAR)Ssid,
								&SsidLen,
								&BssType,
								&BeaconPeriod,
								&Channel,
								&NewChannel,
								&TimeStamp,
								&CfParm,
								&AtimWin,
								&CapabilityInfo,
								&Erp,
								&DtimCount,
								&DtimPeriod,
								&BcastFlag,
								&MessageToMe,
								SupRate,
								&SupRateLen,
								ExtRate,
								&ExtRateLen,
								&CkipFlag,
								&AironetCellPowerLimit,
								&EdcaParm,
								&QbssLoad,
								&QosCapability,
								&RalinkIe,
								&HtCapabilityLen,
								&PreNHtCapabilityLen,
								&HtCapability,
								&AddHtInfoLen,
								&AddHtInfo,
								&NewExtChannelOffset,
								&LenVIE,
								pVIE))
	{
		ULONG Idx;
		CHAR Rssi = 0;

		Idx = BssTableSearch(&pAd->ScanTab, Bssid, Channel);
		if (Idx != BSS_NOT_FOUND)
			Rssi = pAd->ScanTab.BssEntry[Idx].Rssi;

		Rssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, Elem->Rssi0, RSSI_0), ConvertToRssi(pAd, Elem->Rssi1, RSSI_1), ConvertToRssi(pAd, Elem->Rssi2, RSSI_2));


#ifdef DOT11_N_SUPPORT
		if ((HtCapabilityLen > 0) || (PreNHtCapabilityLen > 0))
			HtCapabilityLen = SIZE_HT_CAP_IE;
#endif 

		Idx = BssTableSetEntry(pAd, &pAd->ScanTab, Bssid, (PCHAR)Ssid, SsidLen, BssType, BeaconPeriod,
					  &CfParm, AtimWin, CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen,  &HtCapability,
					 &AddHtInfo, HtCapabilityLen, AddHtInfoLen, NewExtChannelOffset, Channel, Rssi, TimeStamp, CkipFlag,
					 &EdcaParm, &QosCapability, &QbssLoad, LenVIE, pVIE);
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		if (pAd->ChannelList[pAd->CommonCfg.ChannelListIdx].bEffectedChannel == TRUE)
		{
			UCHAR		RegClass;
			PeerBeaconAndProbeRspSanity2(pAd, Elem->Msg, Elem->MsgLen, &RegClass);
			TriEventTableSetEntry(pAd, &pAd->CommonCfg.TriggerEventTab, Bssid, &HtCapability, HtCapabilityLen, RegClass, Channel);
		}
#endif 
#endif 
		if (Idx != BSS_NOT_FOUND)
		{
			NdisMoveMemory(pAd->ScanTab.BssEntry[Idx].PTSF, &Elem->Msg[24], 4);
			NdisMoveMemory(&pAd->ScanTab.BssEntry[Idx].TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
			NdisMoveMemory(&pAd->ScanTab.BssEntry[Idx].TTSF[4], &Elem->TimeStamp.u.LowPart, 4);
		}

	}
	
}


VOID PeerBeaconAtJoinAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR         Bssid[MAC_ADDR_LEN], Addr2[MAC_ADDR_LEN];
	UCHAR         Ssid[MAX_LEN_OF_SSID], SsidLen, BssType, Channel, MessageToMe,
				  DtimCount, DtimPeriod, BcastFlag, NewChannel;
	LARGE_INTEGER TimeStamp;
	USHORT        BeaconPeriod, AtimWin, CapabilityInfo;
	CF_PARM       Cf;
	BOOLEAN       TimerCancelled;
	UCHAR         Erp;
	UCHAR         SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR		  SupRateLen, ExtRateLen;
	UCHAR         CkipFlag;
	USHORT		  LenVIE;
	UCHAR		  AironetCellPowerLimit;
	EDCA_PARM       EdcaParm;
	QBSS_LOAD_PARM  QbssLoad;
	QOS_CAPABILITY_PARM QosCapability;
	USHORT        Status;
	UCHAR						VarIE[MAX_VIE_LEN];		
	NDIS_802_11_VARIABLE_IEs	*pVIE = NULL;
	ULONG           RalinkIe;
	ULONG         Idx;
	HT_CAPABILITY_IE		HtCapability;
	ADD_HT_INFO_IE		AddHtInfo;	
	UCHAR				HtCapabilityLen = 0, PreNHtCapabilityLen = 0;
	UCHAR			AddHtInfoLen;
	UCHAR			NewExtChannelOffset = 0xff;
#ifdef DOT11_N_SUPPORT
	UCHAR			CentralChannel;
	BOOLEAN			bAllowNrate = FALSE;
#endif 

	
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;
    RTMPZeroMemory(&HtCapability, sizeof(HtCapability));
	RTMPZeroMemory(&AddHtInfo, sizeof(ADD_HT_INFO_IE));


	if (PeerBeaconAndProbeRspSanity(pAd,
								Elem->Msg,
								Elem->MsgLen,
								Elem->Channel,
								Addr2,
								Bssid,
								(PCHAR)Ssid,
								&SsidLen,
								&BssType,
								&BeaconPeriod,
								&Channel,
								&NewChannel,
								&TimeStamp,
								&Cf,
								&AtimWin,
								&CapabilityInfo,
								&Erp,
								&DtimCount,
								&DtimPeriod,
								&BcastFlag,
								&MessageToMe,
								SupRate,
								&SupRateLen,
								ExtRate,
								&ExtRateLen,
								&CkipFlag,
								&AironetCellPowerLimit,
								&EdcaParm,
								&QbssLoad,
								&QosCapability,
								&RalinkIe,
								&HtCapabilityLen,
								&PreNHtCapabilityLen,
								&HtCapability,
								&AddHtInfoLen,
								&AddHtInfo,
								&NewExtChannelOffset,
								&LenVIE,
								pVIE))
	{
		
		if ((BssType == BSS_ADHOC) && (pAd->CommonCfg.PhyMode == PHY_11G) && ((SupRateLen+ExtRateLen)< 12))
			return;

		
		
		
		
		
		
		if (MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, Bssid))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("SYNC - receive desired BEACON at JoinWaitBeacon... Channel = %d\n", Channel));
			RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer, &TimerCancelled);

			
			pAd->StaCfg.RssiSample.LastRssi0	= ConvertToRssi(pAd, Elem->Rssi0, RSSI_0);
			pAd->StaCfg.RssiSample.LastRssi1	= ConvertToRssi(pAd, Elem->Rssi1, RSSI_1);
			pAd->StaCfg.RssiSample.LastRssi2	= ConvertToRssi(pAd, Elem->Rssi2, RSSI_2);
			pAd->StaCfg.RssiSample.AvgRssi0	= pAd->StaCfg.RssiSample.LastRssi0;
			pAd->StaCfg.RssiSample.AvgRssi0X8	= pAd->StaCfg.RssiSample.AvgRssi0 << 3;
			pAd->StaCfg.RssiSample.AvgRssi1	= pAd->StaCfg.RssiSample.LastRssi1;
			pAd->StaCfg.RssiSample.AvgRssi1X8	= pAd->StaCfg.RssiSample.AvgRssi1 << 3;
			pAd->StaCfg.RssiSample.AvgRssi2	= pAd->StaCfg.RssiSample.LastRssi2;
			pAd->StaCfg.RssiSample.AvgRssi2X8	= pAd->StaCfg.RssiSample.AvgRssi2 << 3;

			
			
			
			
			if (pAd->MlmeAux.SsidLen == 0)
			{
				NdisMoveMemory(pAd->MlmeAux.Ssid, Ssid, SsidLen);
				pAd->MlmeAux.SsidLen = SsidLen;
			}
			else
			{
				Idx = BssSsidTableSearch(&pAd->ScanTab, Bssid, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, Channel);

				if (Idx == BSS_NOT_FOUND)
				{
					CHAR Rssi = 0;
					Rssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, Elem->Rssi0, RSSI_0), ConvertToRssi(pAd, Elem->Rssi1, RSSI_1), ConvertToRssi(pAd, Elem->Rssi2, RSSI_2));
					Idx = BssTableSetEntry(pAd, &pAd->ScanTab, Bssid, (CHAR *) Ssid, SsidLen, BssType, BeaconPeriod,
										&Cf, AtimWin, CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen,  &HtCapability,
										&AddHtInfo, HtCapabilityLen, AddHtInfoLen, NewExtChannelOffset, Channel, Rssi, TimeStamp, CkipFlag,
										&EdcaParm, &QosCapability, &QbssLoad, LenVIE, pVIE);
					if (Idx != BSS_NOT_FOUND)
					{
						NdisMoveMemory(pAd->ScanTab.BssEntry[Idx].PTSF, &Elem->Msg[24], 4);
						NdisMoveMemory(&pAd->ScanTab.BssEntry[Idx].TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
						NdisMoveMemory(&pAd->ScanTab.BssEntry[Idx].TTSF[4], &Elem->TimeStamp.u.LowPart, 4);
						CapabilityInfo = pAd->ScanTab.BssEntry[Idx].CapabilityInfo;
					}
				}
				else
				{
					
					
					
					CapabilityInfo = pAd->ScanTab.BssEntry[Idx].CapabilityInfo;
				}
			}
			NdisMoveMemory(pAd->MlmeAux.Bssid, Bssid, MAC_ADDR_LEN);
			pAd->MlmeAux.CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;
			pAd->MlmeAux.BssType = BssType;
			pAd->MlmeAux.BeaconPeriod = BeaconPeriod;
			pAd->MlmeAux.Channel = Channel;
			pAd->MlmeAux.AtimWin = AtimWin;
			pAd->MlmeAux.CfpPeriod = Cf.CfpPeriod;
			pAd->MlmeAux.CfpMaxDuration = Cf.CfpMaxDuration;
			pAd->MlmeAux.APRalinkIe = RalinkIe;

			
			
			pAd->MlmeAux.SupRateLen = SupRateLen;
			NdisMoveMemory(pAd->MlmeAux.SupRate, SupRate, SupRateLen);
			RTMPCheckRates(pAd, pAd->MlmeAux.SupRate, &pAd->MlmeAux.SupRateLen);
			pAd->MlmeAux.ExtRateLen = ExtRateLen;
			NdisMoveMemory(pAd->MlmeAux.ExtRate, ExtRate, ExtRateLen);
			RTMPCheckRates(pAd, pAd->MlmeAux.ExtRate, &pAd->MlmeAux.ExtRateLen);

            NdisZeroMemory(pAd->StaActive.SupportedPhyInfo.MCSSet, 16);


#ifdef DOT11_N_SUPPORT
			if (((pAd->StaCfg.WepStatus != Ndis802_11WEPEnabled) && (pAd->StaCfg.WepStatus != Ndis802_11Encryption2Enabled))
				|| (pAd->CommonCfg.HT_DisallowTKIP == FALSE))
			{
				bAllowNrate = TRUE;
			}

			pAd->MlmeAux.NewExtChannelOffset = NewExtChannelOffset;
			pAd->MlmeAux.HtCapabilityLen = HtCapabilityLen;

			RTMPZeroMemory(&pAd->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
			
			if (((HtCapabilityLen > 0) || (PreNHtCapabilityLen > 0)) &&
				((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (bAllowNrate)))
			{
				RTMPMoveMemory(&pAd->MlmeAux.AddHtInfo, &AddHtInfo, SIZE_ADD_HT_INFO_IE);

				
				NdisMoveMemory(pAd->StaActive.SupportedPhyInfo.MCSSet, HtCapability.MCSSet, 16);
				pAd->MlmeAux.NewExtChannelOffset = NewExtChannelOffset;
				pAd->MlmeAux.HtCapabilityLen = SIZE_HT_CAP_IE;
				pAd->StaActive.SupportedPhyInfo.bHtEnable = TRUE;
				if (PreNHtCapabilityLen > 0)
					pAd->StaActive.SupportedPhyInfo.bPreNHt = TRUE;
				RTMPCheckHt(pAd, BSSID_WCID, &HtCapability, &AddHtInfo);
				
				DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAtJoinAction! (MpduDensity=%d, MaxRAmpduFactor=%d, BW=%d)\n",
					pAd->StaActive.SupportedHtPhy.MpduDensity, pAd->StaActive.SupportedHtPhy.MaxRAmpduFactor, HtCapability.HtCapInfo.ChannelWidth));

				if (AddHtInfoLen > 0)
				{
					CentralChannel = AddHtInfo.ControlChan;
					
					if ((AddHtInfo.ControlChan > 2)&& (AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_BELOW) && (HtCapability.HtCapInfo.ChannelWidth == BW_40))
					{
						CentralChannel = AddHtInfo.ControlChan - 2;
					}
					else if ((AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_ABOVE) && (HtCapability.HtCapInfo.ChannelWidth == BW_40))
					{
						CentralChannel = AddHtInfo.ControlChan + 2;
					}

                    
					if (pAd->MlmeAux.CentralChannel != CentralChannel)
						DBGPRINT(RT_DEBUG_ERROR, ("PeerBeaconAtJoinAction HT===>Beacon Central Channel = %d, Control Channel = %d. Mlmeaux CentralChannel = %d\n", CentralChannel, AddHtInfo.ControlChan, pAd->MlmeAux.CentralChannel));

					DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAtJoinAction HT===>Central Channel = %d, Control Channel = %d,  .\n", CentralChannel, AddHtInfo.ControlChan));

				}

			}
			else
#endif 
			{
				
				if ((HtCapabilityLen == 0) && (PreNHtCapabilityLen == 0))
					pAd->MlmeAux.CentralChannel = pAd->MlmeAux.Channel;

				pAd->StaActive.SupportedPhyInfo.bHtEnable = FALSE;
				pAd->MlmeAux.NewExtChannelOffset = 0xff;
				RTMPZeroMemory(&pAd->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
				pAd->MlmeAux.HtCapabilityLen = 0;
				RTMPZeroMemory(&pAd->MlmeAux.AddHtInfo, SIZE_ADD_HT_INFO_IE);
			}

			RTMPUpdateMlmeRate(pAd);

			
			if ((pAd->CommonCfg.bWmmCapable)
#ifdef DOT11_N_SUPPORT
				 || (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
#endif 
				)
			{
				NdisMoveMemory(&pAd->MlmeAux.APEdcaParm, &EdcaParm, sizeof(EDCA_PARM));
				NdisMoveMemory(&pAd->MlmeAux.APQbssLoad, &QbssLoad, sizeof(QBSS_LOAD_PARM));
				NdisMoveMemory(&pAd->MlmeAux.APQosCapability, &QosCapability, sizeof(QOS_CAPABILITY_PARM));
			}
			else
			{
				NdisZeroMemory(&pAd->MlmeAux.APEdcaParm, sizeof(EDCA_PARM));
				NdisZeroMemory(&pAd->MlmeAux.APQbssLoad, sizeof(QBSS_LOAD_PARM));
				NdisZeroMemory(&pAd->MlmeAux.APQosCapability, sizeof(QOS_CAPABILITY_PARM));
			}

			DBGPRINT(RT_DEBUG_TRACE, ("SYNC - after JOIN, SupRateLen=%d, ExtRateLen=%d\n",
				pAd->MlmeAux.SupRateLen, pAd->MlmeAux.ExtRateLen));

			if (AironetCellPowerLimit != 0xFF)
			{
				
				ChangeToCellPowerLimit(pAd, AironetCellPowerLimit);
			}
			else  
				pAd->CommonCfg.TxPowerPercentage = pAd->CommonCfg.TxPowerDefault;

			pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
			Status = MLME_SUCCESS;
			MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_JOIN_CONF, 2, &Status);
		}
		
	}
	
}


VOID PeerBeacon(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR         Bssid[MAC_ADDR_LEN], Addr2[MAC_ADDR_LEN];
	CHAR          Ssid[MAX_LEN_OF_SSID];
	CF_PARM       CfParm;
	UCHAR         SsidLen, MessageToMe=0, BssType, Channel, NewChannel, index=0;
	UCHAR         DtimCount=0, DtimPeriod=0, BcastFlag=0;
	USHORT        CapabilityInfo, AtimWin, BeaconPeriod;
	LARGE_INTEGER TimeStamp;
	USHORT        TbttNumToNextWakeUp;
	UCHAR         Erp;
	UCHAR         SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR		  SupRateLen, ExtRateLen;
	UCHAR		  CkipFlag;
	USHORT        LenVIE;
	UCHAR		  AironetCellPowerLimit;
	EDCA_PARM       EdcaParm;
	QBSS_LOAD_PARM  QbssLoad;
	QOS_CAPABILITY_PARM QosCapability;
	ULONG           RalinkIe;
	
	UCHAR						VarIE[MAX_VIE_LEN];		
	NDIS_802_11_VARIABLE_IEs	*pVIE = NULL;
	HT_CAPABILITY_IE		HtCapability;
	ADD_HT_INFO_IE		AddHtInfo;	
	UCHAR			HtCapabilityLen, PreNHtCapabilityLen;
	UCHAR			AddHtInfoLen;
	UCHAR			NewExtChannelOffset = 0xff;


#ifdef RALINK_ATE
    if (ATE_ON(pAd))
    {
		return;
    }
#endif 

	if (!(INFRA_ON(pAd) || ADHOC_ON(pAd)
		))
		return;

	
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;
    RTMPZeroMemory(&HtCapability, sizeof(HtCapability));
	RTMPZeroMemory(&AddHtInfo, sizeof(ADD_HT_INFO_IE));

	if (PeerBeaconAndProbeRspSanity(pAd,
								Elem->Msg,
								Elem->MsgLen,
								Elem->Channel,
								Addr2,
								Bssid,
								Ssid,
								&SsidLen,
								&BssType,
								&BeaconPeriod,
								&Channel,
								&NewChannel,
								&TimeStamp,
								&CfParm,
								&AtimWin,
								&CapabilityInfo,
								&Erp,
								&DtimCount,
								&DtimPeriod,
								&BcastFlag,
								&MessageToMe,
								SupRate,
								&SupRateLen,
								ExtRate,
								&ExtRateLen,
								&CkipFlag,
								&AironetCellPowerLimit,
								&EdcaParm,
								&QbssLoad,
								&QosCapability,
								&RalinkIe,
								&HtCapabilityLen,
								&PreNHtCapabilityLen,
								&HtCapability,
								&AddHtInfoLen,
								&AddHtInfo,
								&NewExtChannelOffset,
								&LenVIE,
								pVIE))
	{
		BOOLEAN is_my_bssid, is_my_ssid;
		ULONG   Bssidx, Now;
		BSS_ENTRY *pBss;
		CHAR		RealRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, Elem->Rssi0, RSSI_0), ConvertToRssi(pAd, Elem->Rssi1, RSSI_1), ConvertToRssi(pAd, Elem->Rssi2, RSSI_2));

		is_my_bssid = MAC_ADDR_EQUAL(Bssid, pAd->CommonCfg.Bssid)? TRUE : FALSE;
		is_my_ssid = SSID_EQUAL(Ssid, SsidLen, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen)? TRUE:FALSE;


		
		if ((! is_my_ssid) && (! is_my_bssid))
			return;

		
		if (pAd->Mlme.CntlMachine.CurrState == CNTL_WAIT_DISASSOC)
			return;

#ifdef DOT11_N_SUPPORT
		
		if (AddHtInfoLen != 0)
			Channel = AddHtInfo.ControlChan;

		if ((HtCapabilityLen > 0) || (PreNHtCapabilityLen > 0))
			HtCapabilityLen = SIZE_HT_CAP_IE;
#endif 

		
		
		
		Bssidx = BssTableSearch(&pAd->ScanTab, Bssid, Channel);
		if (Bssidx == BSS_NOT_FOUND)
		{
			
			Bssidx = BssTableSetEntry(pAd, &pAd->ScanTab, Bssid, Ssid, SsidLen, BssType, BeaconPeriod,
						 &CfParm, AtimWin, CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen,
						&HtCapability, &AddHtInfo,HtCapabilityLen,AddHtInfoLen,NewExtChannelOffset, Channel,
						RealRssi, TimeStamp, CkipFlag, &EdcaParm, &QosCapability,
						&QbssLoad, LenVIE, pVIE);
			if (Bssidx == BSS_NOT_FOUND) 
				return;

			NdisMoveMemory(pAd->ScanTab.BssEntry[Bssidx].PTSF, &Elem->Msg[24], 4);
			NdisMoveMemory(&pAd->ScanTab.BssEntry[Bssidx].TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
			NdisMoveMemory(&pAd->ScanTab.BssEntry[Bssidx].TTSF[4], &Elem->TimeStamp.u.LowPart, 4);



		}

		if ((pAd->CommonCfg.bIEEE80211H == 1) && (NewChannel != 0) && (Channel != NewChannel))
		{
			
			
			AsicSwitchChannel(pAd, 1, FALSE);
			AsicLockChannel(pAd, 1);
		    LinkDown(pAd, FALSE);
			MlmeQueueInit(&pAd->Mlme.Queue);
			BssTableInit(&pAd->ScanTab);
		    RTMPusecDelay(1000000);		

			
			for (index = 0 ; index < pAd->ChannelListNum; index++)
			{
				if (pAd->ChannelList[index].Channel == NewChannel)
				{
					pAd->ScanTab.BssEntry[Bssidx].Channel = NewChannel;
					pAd->CommonCfg.Channel = NewChannel;
					AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
					AsicLockChannel(pAd, pAd->CommonCfg.Channel);
					DBGPRINT(RT_DEBUG_TRACE, ("PeerBeacon - STA receive channel switch announcement IE (New Channel =%d)\n", NewChannel));
					break;
				}
			}

			if (index >= pAd->ChannelListNum)
			{
				DBGPRINT_ERR(("PeerBeacon(can not find New Channel=%d in ChannelList[%d]\n", pAd->CommonCfg.Channel, pAd->ChannelListNum));
			}
		}

		
		
		if ((! is_my_bssid) && ADHOC_ON(pAd))
		{
			INT	i;

			
			if (pAd->StaCfg.WepStatus != pAd->ScanTab.BssEntry[Bssidx].WepStatus)
			{
				return;
			}

			
			for (i = 0; i < 6; i++)
			{
				if (Bssid[i] > pAd->CommonCfg.Bssid[i])
				{
					DBGPRINT(RT_DEBUG_TRACE, ("SYNC - merge to the IBSS with bigger BSSID=%02x:%02x:%02x:%02x:%02x:%02x\n",
						Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5]));
					AsicDisableSync(pAd);
					COPY_MAC_ADDR(pAd->CommonCfg.Bssid, Bssid);
					AsicSetBssid(pAd, pAd->CommonCfg.Bssid);
					MakeIbssBeacon(pAd);        
					AsicEnableIbssSync(pAd);    
					is_my_bssid = TRUE;
					break;
				}
				else if (Bssid[i] < pAd->CommonCfg.Bssid[i])
					break;
			}
		}


		NdisGetSystemUpTime(&Now);
		pBss = &pAd->ScanTab.BssEntry[Bssidx];
		pBss->Rssi = RealRssi;       
		pBss->LastBeaconRxTime = Now;   

		
		
		
		if (is_my_bssid)
		{
			RXWI_STRUC	RxWI;

			pAd->StaCfg.DtimCount = DtimCount;
			pAd->StaCfg.DtimPeriod = DtimPeriod;
			pAd->StaCfg.LastBeaconRxTime = Now;


			RxWI.RSSI0 = Elem->Rssi0;
			RxWI.RSSI1 = Elem->Rssi1;
			RxWI.RSSI2 = Elem->Rssi2;

			Update_Rssi_Sample(pAd, &pAd->StaCfg.RssiSample, &RxWI);
			if (AironetCellPowerLimit != 0xFF)
			{
				
				
				
				
				ChangeToCellPowerLimit(pAd, AironetCellPowerLimit);
			}
			else
			{
				
				
				
				
				pAd->CommonCfg.TxPowerPercentage = pAd->CommonCfg.TxPowerDefault;
			}

			if (ADHOC_ON(pAd) && (CAP_IS_IBSS_ON(CapabilityInfo)))
			{
				UCHAR			MaxSupportedRateIn500Kbps = 0;
				UCHAR			idx;
				MAC_TABLE_ENTRY *pEntry;

				
			    for (idx=0; idx<SupRateLen; idx++)
				{
			        if (MaxSupportedRateIn500Kbps < (SupRate[idx] & 0x7f))
			            MaxSupportedRateIn500Kbps = SupRate[idx] & 0x7f;
						}

				for (idx=0; idx<ExtRateLen; idx++)
			    {
			        if (MaxSupportedRateIn500Kbps < (ExtRate[idx] & 0x7f))
			            MaxSupportedRateIn500Kbps = ExtRate[idx] & 0x7f;
					}

				
				pEntry = MacTableLookup(pAd, Addr2);

				
				
				if ((ADHOC_ON(pAd) && (Elem->Wcid == RESERVED_WCID)) ||
					(pEntry && ((pEntry->LastBeaconRxTime + ADHOC_ENTRY_BEACON_LOST_TIME) < Now)))
						{
					if (pEntry == NULL)
						
						pEntry = MacTableInsertEntry(pAd, Addr2, BSS0, FALSE);

					if (StaAddMacTableEntry(pAd,
											pEntry,
											MaxSupportedRateIn500Kbps,
											&HtCapability,
											HtCapabilityLen,
											&AddHtInfo,
											AddHtInfoLen,
											CapabilityInfo) == FALSE)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("ADHOC - Add Entry failed.\n"));
						return;
					}

					if (pEntry &&
						(Elem->Wcid == RESERVED_WCID))
				{
						idx = pAd->StaCfg.DefaultKeyId;
						RTMP_STA_SECURITY_INFO_ADD(pAd, BSS0, idx, pEntry);
				}
				}

				if (pEntry && pEntry->ValidAsCLI)
					pEntry->LastBeaconRxTime = Now;

				
				if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
				{
					OPSTATUS_SET_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);

					pAd->IndicateMediaState = NdisMediaStateConnected;
					RTMP_IndicateMediaState(pAd);
	                pAd->ExtraInfo = GENERAL_LINK_UP;
					AsicSetBssid(pAd, pAd->CommonCfg.Bssid);

					
					
					
					
					Bssidx = BssTableSearch(&pAd->ScanTab, Bssid, Channel);
					if (Bssidx == BSS_NOT_FOUND)
					{
						Bssidx = BssTableSetEntry(pAd, &pAd->ScanTab, Bssid, Ssid, SsidLen, BssType, BeaconPeriod,
									&CfParm, AtimWin, CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen, &HtCapability,
									&AddHtInfo, HtCapabilityLen, AddHtInfoLen, NewExtChannelOffset, Channel, RealRssi, TimeStamp, 0,
									&EdcaParm, &QosCapability, &QbssLoad, LenVIE, pVIE);
					}
					DBGPRINT(RT_DEBUG_TRACE, ("ADHOC  fOP_STATUS_MEDIA_STATE_CONNECTED.\n"));
				}
			}

			if (INFRA_ON(pAd))
			{
				BOOLEAN bUseShortSlot, bUseBGProtection;

				
				
				
				

				
				bUseShortSlot = CAP_IS_SHORT_SLOT(CapabilityInfo);
				if (bUseShortSlot != OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED))
					AsicSetSlotTime(pAd, bUseShortSlot);

				bUseBGProtection = (pAd->CommonCfg.UseBGProtection == 1) ||    
								   ((pAd->CommonCfg.UseBGProtection == 0) && ERP_IS_USE_PROTECTION(Erp));

				if (pAd->CommonCfg.Channel > 14) 
					bUseBGProtection = FALSE;

				if (bUseBGProtection != OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					if (bUseBGProtection)
					{
						OPSTATUS_SET_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
						AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, (OFDMSETPROTECT|CCKSETPROTECT|ALLN_SETPROTECT),FALSE,(pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent == 1));
					}
					else
					{
						OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
						AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, (OFDMSETPROTECT|CCKSETPROTECT|ALLN_SETPROTECT),TRUE,(pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent == 1));
					}

					DBGPRINT(RT_DEBUG_WARN, ("SYNC - AP changed B/G protection to %d\n", bUseBGProtection));
				}

#ifdef DOT11_N_SUPPORT
				
				if ((AddHtInfoLen != 0) &&
					((AddHtInfo.AddHtInfo2.OperaionMode != pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode) ||
					(AddHtInfo.AddHtInfo2.NonGfPresent != pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent)))
				{
					pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent = AddHtInfo.AddHtInfo2.NonGfPresent;
					pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode = AddHtInfo.AddHtInfo2.OperaionMode;
					if (pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent == 1)
				{
						AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, ALLN_SETPROTECT, FALSE, TRUE);
					}
					else
						AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, ALLN_SETPROTECT, FALSE, FALSE);

					DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP changed N OperaionMode to %d\n", pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode));
				}
#endif 

				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED) &&
					ERP_IS_USE_BARKER_PREAMBLE(Erp))
				{
					MlmeSetTxPreamble(pAd, Rt802_11PreambleLong);
					DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP forced to use LONG preamble\n"));
				}

				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED)    &&
					(EdcaParm.bValid == TRUE)                          &&
					(EdcaParm.EdcaUpdateCount != pAd->CommonCfg.APEdcaParm.EdcaUpdateCount))
				{
					DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP change EDCA parameters(from %d to %d)\n",
						pAd->CommonCfg.APEdcaParm.EdcaUpdateCount,
						EdcaParm.EdcaUpdateCount));
					AsicSetEdcaParm(pAd, &EdcaParm);
				}

				
				NdisMoveMemory(&pAd->CommonCfg.APQbssLoad, &QbssLoad, sizeof(QBSS_LOAD_PARM));
				NdisMoveMemory(&pAd->CommonCfg.APQosCapability, &QosCapability, sizeof(QOS_CAPABILITY_PARM));
			}

			
			if ((INFRA_ON(pAd) && (pAd->StaCfg.Psm == PWR_SAVE)) || (pAd->CommonCfg.bAPSDForcePowerSave))
			{
				UCHAR FreeNumber;
				
				
				
				
				
				if (MessageToMe)
				{
#ifdef RTMP_MAC_PCI
					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE))
					{
						
						if (pAd->Antenna.field.RxPath > 1)
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, pAd->StaCfg.BBPR3);
						
					}
#endif 
					if (pAd->CommonCfg.bAPSDCapable && pAd->CommonCfg.APEdcaParm.bAPSDCapable &&
						pAd->CommonCfg.bAPSDAC_BE && pAd->CommonCfg.bAPSDAC_BK && pAd->CommonCfg.bAPSDAC_VI && pAd->CommonCfg.bAPSDAC_VO)
					{
						pAd->CommonCfg.bNeedSendTriggerFrame = TRUE;
					}
					else
						RTMP_PS_POLL_ENQUEUE(pAd);
				}
				else if (BcastFlag && (DtimCount == 0) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM))
				{
#ifdef RTMP_MAC_PCI
					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE))
					{
						if (pAd->Antenna.field.RxPath > 1)
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, pAd->StaCfg.BBPR3);
					}
#endif 
				}
				else if ((pAd->TxSwQueue[QID_AC_BK].Number != 0)													||
						(pAd->TxSwQueue[QID_AC_BE].Number != 0)														||
						(pAd->TxSwQueue[QID_AC_VI].Number != 0)														||
						(pAd->TxSwQueue[QID_AC_VO].Number != 0)														||
						(RTMPFreeTXDRequest(pAd, QID_AC_BK, TX_RING_SIZE - 1, &FreeNumber) != NDIS_STATUS_SUCCESS)	||
						(RTMPFreeTXDRequest(pAd, QID_AC_BE, TX_RING_SIZE - 1, &FreeNumber) != NDIS_STATUS_SUCCESS)	||
						(RTMPFreeTXDRequest(pAd, QID_AC_VI, TX_RING_SIZE - 1, &FreeNumber) != NDIS_STATUS_SUCCESS)	||
						(RTMPFreeTXDRequest(pAd, QID_AC_VO, TX_RING_SIZE - 1, &FreeNumber) != NDIS_STATUS_SUCCESS)	||
						(RTMPFreeTXDRequest(pAd, QID_MGMT, MGMT_RING_SIZE - 1, &FreeNumber) != NDIS_STATUS_SUCCESS))
				{
					
					
#ifdef RTMP_MAC_PCI
					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE))
					{
						if (pAd->Antenna.field.RxPath > 1)
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, pAd->StaCfg.BBPR3);
					}
#endif 
				}
				else
				{
					if ((pAd->CommonCfg.bACMAPSDTr[QID_AC_VO]) ||
						(pAd->CommonCfg.bACMAPSDTr[QID_AC_VI]) ||
						(pAd->CommonCfg.bACMAPSDTr[QID_AC_BK]) ||
						(pAd->CommonCfg.bACMAPSDTr[QID_AC_BE]))
					{
						
					}
					else
					{
						USHORT NextDtim = DtimCount;


						if (NextDtim == 0)
							NextDtim = DtimPeriod;

						TbttNumToNextWakeUp = pAd->StaCfg.DefaultListenCount;
						if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM) && (TbttNumToNextWakeUp > NextDtim))
							TbttNumToNextWakeUp = NextDtim;

						if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
						{
							
							pAd->ThisTbttNumToNextWakeUp = TbttNumToNextWakeUp;
		                                        AsicSleepThenAutoWakeup(pAd, pAd->ThisTbttNumToNextWakeUp);

						}
					}
				}
			}
		}
		
	}
	
}


VOID PeerProbeReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR         Addr2[MAC_ADDR_LEN];
	CHAR          Ssid[MAX_LEN_OF_SSID];
	UCHAR         SsidLen;
#ifdef DOT11_N_SUPPORT
	UCHAR		  HtLen, AddHtLen, NewExtLen;
#endif 
	HEADER_802_11 ProbeRspHdr;
	NDIS_STATUS   NStatus;
	PUCHAR        pOutBuffer = NULL;
	ULONG         FrameLen = 0;
	LARGE_INTEGER FakeTimestamp;
	UCHAR         DsLen = 1, IbssLen = 2;
	UCHAR         LocalErpIe[3] = {IE_ERP, 1, 0};
	BOOLEAN       Privacy;
	USHORT        CapabilityInfo;
	UCHAR		  RSNIe = IE_WPA;

	if (! ADHOC_ON(pAd))
		return;

	if (PeerProbeReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, Ssid, &SsidLen))
	{
		if ((SsidLen == 0) || SSID_EQUAL(Ssid, SsidLen, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen))
		{
			
			NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  
			if (NStatus != NDIS_STATUS_SUCCESS)
				return;

			

			Privacy = (pAd->StaCfg.WepStatus == Ndis802_11Encryption1Enabled) ||
					  (pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) ||
					  (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled);
			CapabilityInfo = CAP_GENERATE(0, 1, Privacy, (pAd->CommonCfg.TxPreamble == Rt802_11PreambleShort), 0, 0);

			MakeOutgoingFrame(pOutBuffer,                   &FrameLen,
							  sizeof(HEADER_802_11),        &ProbeRspHdr,
							  TIMESTAMP_LEN,                &FakeTimestamp,
							  2,                            &pAd->CommonCfg.BeaconPeriod,
							  2,                            &CapabilityInfo,
							  1,                            &SsidIe,
							  1,                            &pAd->CommonCfg.SsidLen,
							  pAd->CommonCfg.SsidLen,       pAd->CommonCfg.Ssid,
							  1,                            &SupRateIe,
							  1,                            &pAd->StaActive.SupRateLen,
							  pAd->StaActive.SupRateLen,    pAd->StaActive.SupRate,
							  1,                            &DsIe,
							  1,                            &DsLen,
							  1,                            &pAd->CommonCfg.Channel,
							  1,                            &IbssIe,
							  1,                            &IbssLen,
							  2,                            &pAd->StaActive.AtimWin,
							  END_OF_ARGS);

			if (pAd->StaActive.ExtRateLen)
			{
				ULONG tmp;
				MakeOutgoingFrame(pOutBuffer + FrameLen,        &tmp,
								  3,                            LocalErpIe,
								  1,                            &ExtRateIe,
								  1,                            &pAd->StaActive.ExtRateLen,
								  pAd->StaActive.ExtRateLen,    &pAd->StaActive.ExtRate,
								  END_OF_ARGS);
				FrameLen += tmp;
			}

			
			if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPANone)
			{
				ULONG tmp;
				MakeOutgoingFrame(pOutBuffer + FrameLen,		&tmp,
									1,                              &RSNIe,
									1,				&pAd->StaCfg.RSNIE_Len,
									pAd->StaCfg.RSNIE_Len,		pAd->StaCfg.RSN_IE,
									END_OF_ARGS);
				FrameLen += tmp;
			}
#ifdef DOT11_N_SUPPORT
			if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
			{
				ULONG TmpLen;
				UCHAR	BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
				HtLen = sizeof(pAd->CommonCfg.HtCapability);
				AddHtLen = sizeof(pAd->CommonCfg.AddHTInfo);
				NewExtLen = 1;
				
				if (pAd->bBroadComHT == TRUE)
				{
					MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
								  1,                                &WpaIe,
								  4,                                &BROADCOM[0],
								 pAd->MlmeAux.HtCapabilityLen,          &pAd->MlmeAux.HtCapability,
								  END_OF_ARGS);
				}
				else
				{
				MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
								  1,                                &HtCapIe,
								  1,                                &HtLen,
								 sizeof(HT_CAPABILITY_IE),          &pAd->CommonCfg.HtCapability,
								  1,                                &AddHtInfoIe,
								  1,                                &AddHtLen,
								 sizeof(ADD_HT_INFO_IE),          &pAd->CommonCfg.AddHTInfo,
								  1,                                &NewExtChanIe,
								  1,                                &NewExtLen,
								 sizeof(NEW_EXT_CHAN_IE),          &pAd->CommonCfg.NewExtChanOffset,
								  END_OF_ARGS);
				}
				FrameLen += TmpLen;
			}
#endif 
			MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
			MlmeFreeMemory(pAd, pOutBuffer);
		}
	}
}

VOID BeaconTimeoutAtJoinAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT Status;
	DBGPRINT(RT_DEBUG_TRACE, ("SYNC - BeaconTimeoutAtJoinAction\n"));
	pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
	Status = MLME_REJ_TIMEOUT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_JOIN_CONF, 2, &Status);
}


VOID ScanTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	pAd->MlmeAux.Channel = NextChannel(pAd, pAd->MlmeAux.Channel);

	
	if ((pAd->MlmeAux.ScanType == SCAN_CISCO_ACTIVE) ||
		(pAd->MlmeAux.ScanType == SCAN_CISCO_PASSIVE) ||
		(pAd->MlmeAux.ScanType == SCAN_CISCO_NOISE) ||
		(pAd->MlmeAux.ScanType == SCAN_CISCO_CHANNEL_LOAD))
		pAd->MlmeAux.Channel = 0;

	
	ScanNextChannel(pAd);
}


VOID InvalidStateWhenScan(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT Status;
	DBGPRINT(RT_DEBUG_TRACE, ("AYNC - InvalidStateWhenScan(state=%ld). Reset SYNC machine\n", pAd->Mlme.SyncMachine.CurrState));
	pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
	Status = MLME_STATE_MACHINE_REJECT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status);
}


VOID InvalidStateWhenJoin(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT Status;
	DBGPRINT(RT_DEBUG_TRACE, ("InvalidStateWhenJoin(state=%ld). Reset SYNC machine\n", pAd->Mlme.SyncMachine.CurrState));
	pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
	Status = MLME_STATE_MACHINE_REJECT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_JOIN_CONF, 2, &Status);
}


VOID InvalidStateWhenStart(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT Status;
	DBGPRINT(RT_DEBUG_TRACE, ("InvalidStateWhenStart(state=%ld). Reset SYNC machine\n", pAd->Mlme.SyncMachine.CurrState));
	pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
	Status = MLME_STATE_MACHINE_REJECT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_START_CONF, 2, &Status);
}


VOID EnqueuePsPoll(
	IN PRTMP_ADAPTER pAd)
{
#ifdef RALINK_ATE
    if (ATE_ON(pAd))
    {
		return;
    }
#endif 


	if (pAd->StaCfg.WindowsPowerMode == Ndis802_11PowerModeLegacy_PSP)
	pAd->PsPollFrame.FC.PwrMgmt = PWR_SAVE;
	MiniportMMRequest(pAd, 0, (PUCHAR)&pAd->PsPollFrame, sizeof(PSPOLL_FRAME));
}



VOID EnqueueProbeRequest(
	IN PRTMP_ADAPTER pAd)
{
	NDIS_STATUS     NState;
	PUCHAR          pOutBuffer;
	ULONG           FrameLen = 0;
	HEADER_802_11   Hdr80211;

	DBGPRINT(RT_DEBUG_TRACE, ("force out a ProbeRequest ...\n"));

	NState = MlmeAllocateMemory(pAd, &pOutBuffer);  
	if (NState == NDIS_STATUS_SUCCESS)
	{
		MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR, BROADCAST_ADDR);

		
		MakeOutgoingFrame(pOutBuffer,                     &FrameLen,
						  sizeof(HEADER_802_11),          &Hdr80211,
						  1,                              &SsidIe,
						  1,                              &pAd->CommonCfg.SsidLen,
						  pAd->CommonCfg.SsidLen,		  pAd->CommonCfg.Ssid,
						  1,                              &SupRateIe,
						  1,                              &pAd->StaActive.SupRateLen,
						  pAd->StaActive.SupRateLen,      pAd->StaActive.SupRate,
						  END_OF_ARGS);
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);
	}

}

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
VOID BuildEffectedChannelList(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR		EChannel[11];
	UCHAR		i, j, k;
	UCHAR		UpperChannel = 0, LowerChannel = 0;

	RTMPZeroMemory(EChannel, 11);
	i = 0;
	
	if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
	{
		UpperChannel = pAd->CommonCfg.Channel;
		LowerChannel = pAd->CommonCfg.CentralChannel;
	}
	else if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
	{
		UpperChannel = pAd->CommonCfg.CentralChannel;
		LowerChannel = pAd->CommonCfg.Channel;
	}
	else
	{
		return;
	}

	
	if (LowerChannel > 1)
	{
		EChannel[0] = LowerChannel - 1;
		i = 1;
		if (LowerChannel > 2)
		{
			EChannel[1] = LowerChannel - 2;
			i = 2;
			if (LowerChannel > 3)
			{
				EChannel[2] = LowerChannel - 3;
				i = 3;
			}
		}
	}
	
	for (k = LowerChannel;k < UpperChannel;k++)
	{
		EChannel[i] = k;
		i++;
	}
	
	if (LowerChannel < 11)
	{
		EChannel[i] = UpperChannel + 1;
		i++;
		if (LowerChannel < 10)
		{
			EChannel[i] = LowerChannel + 2;
			i++;
			if (LowerChannel < 9)
			{
				EChannel[i] = LowerChannel + 3;
				i++;
			}
		}
	}
	
	for (j = 0;j < i;j++)
	{
		for (k = 0;k < pAd->ChannelListNum;k++)
		{
			if (pAd->ChannelList[k].Channel == EChannel[j])
			{
				pAd->ChannelList[k].bEffectedChannel = TRUE;
				DBGPRINT(RT_DEBUG_TRACE,(" EffectedChannel( =%d)\n", EChannel[j]));
				break;
			}
		}
	}
}
#endif 
#endif 

BOOLEAN ScanRunning(
		IN PRTMP_ADAPTER pAd)
{
	return (pAd->Mlme.SyncMachine.CurrState == SCAN_LISTEN) ? TRUE : FALSE;
}
