
#include "../rt_config.h"

UCHAR	CipherSuiteWpaNoneTkip[] = {
		0x00, 0x50, 0xf2, 0x01,	
		0x01, 0x00,				
		0x00, 0x50, 0xf2, 0x02,	
		0x01, 0x00,				
		0x00, 0x50, 0xf2, 0x02,	
		0x01, 0x00,				
		0x00, 0x50, 0xf2, 0x00	
		};
UCHAR	CipherSuiteWpaNoneTkipLen = (sizeof(CipherSuiteWpaNoneTkip) / sizeof(UCHAR));

UCHAR	CipherSuiteWpaNoneAes[] = {
		0x00, 0x50, 0xf2, 0x01,	
		0x01, 0x00,				
		0x00, 0x50, 0xf2, 0x04,	
		0x01, 0x00,				
		0x00, 0x50, 0xf2, 0x04,	
		0x01, 0x00,				
		0x00, 0x50, 0xf2, 0x00	
		};
UCHAR	CipherSuiteWpaNoneAesLen = (sizeof(CipherSuiteWpaNoneAes) / sizeof(UCHAR));





#define COPY_SETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(_pAd)                                 \
{                                                                                       \
	(_pAd)->CommonCfg.SsidLen = (_pAd)->MlmeAux.SsidLen;                                \
	NdisMoveMemory((_pAd)->CommonCfg.Ssid, (_pAd)->MlmeAux.Ssid, (_pAd)->MlmeAux.SsidLen); \
	COPY_MAC_ADDR((_pAd)->CommonCfg.Bssid, (_pAd)->MlmeAux.Bssid);                      \
	(_pAd)->CommonCfg.Channel = (_pAd)->MlmeAux.Channel;                                \
	(_pAd)->CommonCfg.CentralChannel = (_pAd)->MlmeAux.CentralChannel;                  \
	(_pAd)->StaActive.Aid = (_pAd)->MlmeAux.Aid;                                        \
	(_pAd)->StaActive.AtimWin = (_pAd)->MlmeAux.AtimWin;                                \
	(_pAd)->StaActive.CapabilityInfo = (_pAd)->MlmeAux.CapabilityInfo;                  \
	(_pAd)->CommonCfg.BeaconPeriod = (_pAd)->MlmeAux.BeaconPeriod;                      \
	(_pAd)->StaActive.CfpMaxDuration = (_pAd)->MlmeAux.CfpMaxDuration;                  \
	(_pAd)->StaActive.CfpPeriod = (_pAd)->MlmeAux.CfpPeriod;                            \
	(_pAd)->StaActive.SupRateLen = (_pAd)->MlmeAux.SupRateLen;                          \
	NdisMoveMemory((_pAd)->StaActive.SupRate, (_pAd)->MlmeAux.SupRate, (_pAd)->MlmeAux.SupRateLen);\
	(_pAd)->StaActive.ExtRateLen = (_pAd)->MlmeAux.ExtRateLen;                          \
	NdisMoveMemory((_pAd)->StaActive.ExtRate, (_pAd)->MlmeAux.ExtRate, (_pAd)->MlmeAux.ExtRateLen);\
	NdisMoveMemory(&(_pAd)->CommonCfg.APEdcaParm, &(_pAd)->MlmeAux.APEdcaParm, sizeof(EDCA_PARM));\
	NdisMoveMemory(&(_pAd)->CommonCfg.APQosCapability, &(_pAd)->MlmeAux.APQosCapability, sizeof(QOS_CAPABILITY_PARM));\
	NdisMoveMemory(&(_pAd)->CommonCfg.APQbssLoad, &(_pAd)->MlmeAux.APQbssLoad, sizeof(QBSS_LOAD_PARM));\
	COPY_MAC_ADDR((_pAd)->MacTab.Content[BSSID_WCID].Addr, (_pAd)->MlmeAux.Bssid);      \
	(_pAd)->MacTab.Content[BSSID_WCID].Aid = (_pAd)->MlmeAux.Aid;                       \
	(_pAd)->MacTab.Content[BSSID_WCID].PairwiseKey.CipherAlg = (_pAd)->StaCfg.PairCipher;\
	COPY_MAC_ADDR((_pAd)->MacTab.Content[BSSID_WCID].PairwiseKey.BssId, (_pAd)->MlmeAux.Bssid);\
	(_pAd)->MacTab.Content[BSSID_WCID].RateLen = (_pAd)->StaActive.SupRateLen + (_pAd)->StaActive.ExtRateLen;\
}


VOID MlmeCntlInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE *S,
	OUT STATE_MACHINE_FUNC Trans[])
{
	
	
	pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;
}


VOID MlmeCntlMachinePerformAction(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE *S,
	IN MLME_QUEUE_ELEM *Elem)
{
	switch(pAd->Mlme.CntlMachine.CurrState)
	{
		case CNTL_IDLE:
			{
				CntlIdleProc(pAd, Elem);
			}
			break;
		case CNTL_WAIT_DISASSOC:
			CntlWaitDisassocProc(pAd, Elem);
			break;
		case CNTL_WAIT_JOIN:
			CntlWaitJoinProc(pAd, Elem);
			break;

		
		
		
		
		
		
		case CNTL_WAIT_REASSOC:
			CntlWaitReassocProc(pAd, Elem);
			break;

		case CNTL_WAIT_START:
			CntlWaitStartProc(pAd, Elem);
			break;
		case CNTL_WAIT_AUTH:
			CntlWaitAuthProc(pAd, Elem);
			break;
		case CNTL_WAIT_AUTH2:
			CntlWaitAuthProc2(pAd, Elem);
			break;
		case CNTL_WAIT_ASSOC:
			CntlWaitAssocProc(pAd, Elem);
			break;

		case CNTL_WAIT_OID_LIST_SCAN:
			if(Elem->MsgType == MT2_SCAN_CONF)
			{
				
				
				RTMPResumeMsduTransmission(pAd);
				if (pAd->StaCfg.CCXReqType != MSRN_TYPE_UNUSED)
				{
					
					MlmeEnqueue(pAd, AIRONET_STATE_MACHINE, MT2_AIRONET_SCAN_DONE, 0, NULL);
				}
				pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;

                
				
				
				if (pAd->bLedOnScanning)
				{
					pAd->bLedOnScanning = FALSE;
					RTMPSetLED(pAd, pAd->LedStatus);
				}
			}
			break;

		case CNTL_WAIT_OID_DISASSOC:
			if (Elem->MsgType == MT2_DISASSOC_CONF)
			{
				LinkDown(pAd, FALSE);
				pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;
			}
			break;
#ifdef RT2870
		
		
		
		
		
		case CNTL_WAIT_SCAN_FOR_CONNECT:
			if(Elem->MsgType == MT2_SCAN_CONF)
			{
				
				
				RTMPResumeMsduTransmission(pAd);
#ifdef CCX_SUPPORT
				if (pAd->StaCfg.CCXReqType != MSRN_TYPE_UNUSED)
				{
					
					MlmeEnqueue(pAd, AIRONET_STATE_MACHINE, MT2_AIRONET_SCAN_DONE, 0, NULL);
				}
#endif 
				pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;

				
				
				
				BssTableSsidSort(pAd, &pAd->MlmeAux.SsidBssTab, pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
				if (pAd->MlmeAux.SsidBssTab.BssNr > 0)
				{
					MlmeAutoReconnectLastSSID(pAd);
				}
			}
			break;
#endif 
		default:
			DBGPRINT_ERR(("!ERROR! CNTL - Illegal message type(=%ld)", Elem->MsgType));
			break;
	}
}



VOID CntlIdleProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	MLME_DISASSOC_REQ_STRUCT   DisassocReq;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
		return;

	switch(Elem->MsgType)
	{
		case OID_802_11_SSID:
			CntlOidSsidProc(pAd, Elem);
			break;

		case OID_802_11_BSSID:
			CntlOidRTBssidProc(pAd,Elem);
			break;

		case OID_802_11_BSSID_LIST_SCAN:
			CntlOidScanProc(pAd,Elem);
			break;

		case OID_802_11_DISASSOCIATE:
			DisassocParmFill(pAd, &DisassocReq, pAd->CommonCfg.Bssid, REASON_DISASSOC_STA_LEAVING);
			MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);
			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_DISASSOC;

            if (pAd->StaCfg.WpaSupplicantUP != WPA_SUPPLICANT_ENABLE_WITH_WEB_UI)
            {
    			
    			
    			pAd->MlmeAux.AutoReconnectSsidLen= 32;
    			NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
            }
			break;

		case MT2_MLME_ROAMING_REQ:
			CntlMlmeRoamingProc(pAd, Elem);
			break;

        case OID_802_11_MIC_FAILURE_REPORT_FRAME:
            WpaMicFailureReportFrame(pAd, Elem);
            break;

		default:
			DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Illegal message in CntlIdleProc(MsgType=%ld)\n",Elem->MsgType));
			break;
	}
}

VOID CntlOidScanProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	MLME_SCAN_REQ_STRUCT       ScanReq;
	ULONG                      BssIdx = BSS_NOT_FOUND;
	BSS_ENTRY                  CurrBss;

	
	
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
	{
		BssIdx = BssSsidTableSearch(&pAd->ScanTab, pAd->CommonCfg.Bssid, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen, pAd->CommonCfg.Channel);
		if (BssIdx != BSS_NOT_FOUND)
		{
			NdisMoveMemory(&CurrBss, &pAd->ScanTab.BssEntry[BssIdx], sizeof(BSS_ENTRY));
		}
	}

	
	BssTableInit(&pAd->ScanTab);
	if (BssIdx != BSS_NOT_FOUND)
	{
		
		
		
		
		
		NdisMoveMemory(&pAd->ScanTab.BssEntry[0], &CurrBss, sizeof(BSS_ENTRY));
		pAd->ScanTab.BssNr = 1;
	}

	ScanParmFill(pAd, &ScanReq, "", 0, BSS_ANY, SCAN_ACTIVE);
	MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ,
		sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq);
	pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;
}


VOID CntlOidSsidProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	PNDIS_802_11_SSID          pOidSsid = (NDIS_802_11_SSID *)Elem->Msg;
	MLME_DISASSOC_REQ_STRUCT   DisassocReq;
	ULONG					   Now;

#ifdef RT2860
	
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
		AsicForceWakeup(pAd, RTMP_HALT);
#endif

	
	NdisZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
	NdisMoveMemory(pAd->MlmeAux.Ssid, pOidSsid->Ssid, pOidSsid->SsidLength);
	pAd->MlmeAux.SsidLen = (UCHAR)pOidSsid->SsidLength;
	NdisZeroMemory(pAd->MlmeAux.Bssid, MAC_ADDR_LEN);
	pAd->MlmeAux.BssType = pAd->StaCfg.BssType;


	
	
	
	NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, MAX_LEN_OF_SSID);
	NdisMoveMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
	pAd->MlmeAux.AutoReconnectSsidLen = pAd->MlmeAux.SsidLen;

	
	
	BssTableSsidSort(pAd, &pAd->MlmeAux.SsidBssTab, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);

	DBGPRINT(RT_DEBUG_TRACE, ("CntlOidSsidProc():CNTL - %d BSS of %d BSS match the desire (%d)SSID - %s\n",
			pAd->MlmeAux.SsidBssTab.BssNr, pAd->ScanTab.BssNr, pAd->MlmeAux.SsidLen, pAd->MlmeAux.Ssid));
	NdisGetSystemUpTime(&Now);

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) &&
		(pAd->CommonCfg.SsidLen == pAd->MlmeAux.SsidBssTab.BssEntry[0].SsidLen) &&
		NdisEqualMemory(pAd->CommonCfg.Ssid, pAd->MlmeAux.SsidBssTab.BssEntry[0].Ssid, pAd->CommonCfg.SsidLen) &&
		MAC_ADDR_EQUAL(pAd->CommonCfg.Bssid, pAd->MlmeAux.SsidBssTab.BssEntry[0].Bssid))
	{
		
		

		
		if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) ||
			 (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK) ||
			 (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) ||
			 (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)
			 ) &&
			(pAd->StaCfg.PortSecured == WPA_802_1X_PORT_NOT_SECURED))
		{
			
			
			DBGPRINT(RT_DEBUG_TRACE, ("CntlOidSsidProc():CNTL - disassociate with current AP...\n"));
			DisassocParmFill(pAd, &DisassocReq, pAd->CommonCfg.Bssid, REASON_DISASSOC_STA_LEAVING);
			MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ,
						sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);
			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
		}
		else if (pAd->bConfigChanged == TRUE)
		{
			
			DBGPRINT(RT_DEBUG_TRACE, ("CntlOidSsidProc():CNTL - disassociate with current AP Because config changed...\n"));
			DisassocParmFill(pAd, &DisassocReq, pAd->CommonCfg.Bssid, REASON_DISASSOC_STA_LEAVING);
			MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ,
						sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);
			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
		}
		else
		{
			
			DBGPRINT(RT_DEBUG_TRACE, ("CntlOidSsidProc():CNTL - already with this BSSID. ignore this SET_SSID request\n"));
			
			
			
			
			if (INFRA_ON(pAd))
			{
				
				
				
				
				pAd->IndicateMediaState = NdisMediaStateConnected;
				RTMP_IndicateMediaState(pAd);
                pAd->ExtraInfo = GENERAL_LINK_UP;   
			}

			pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;

            {
                union iwreq_data    wrqu;

                memset(wrqu.ap_addr.sa_data, 0, MAC_ADDR_LEN);
                memcpy(wrqu.ap_addr.sa_data, pAd->MlmeAux.Bssid, MAC_ADDR_LEN);
                wireless_send_event(pAd->net_dev, SIOCGIWAP, &wrqu, NULL);

            }
		}
	}
	else if (INFRA_ON(pAd))
	{
		
		
		
		
		
		
		if (!SSID_EQUAL(pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen))
		{
			
			
			
			pAd->MlmeAux.CurrReqIsFromNdis = TRUE;
		}
		
		
		
		
		
		
		DBGPRINT(RT_DEBUG_TRACE, ("CntlOidSsidProc():CNTL - disassociate with current AP...\n"));
		DisassocParmFill(pAd, &DisassocReq, pAd->CommonCfg.Bssid, REASON_DISASSOC_STA_LEAVING);
		MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ,
					sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);
		pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
	}
	else
	{
		if (ADHOC_ON(pAd))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("CntlOidSsidProc():CNTL - drop current ADHOC\n"));
			LinkDown(pAd, FALSE);
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);
			pAd->IndicateMediaState = NdisMediaStateDisconnected;
			RTMP_IndicateMediaState(pAd);
            pAd->ExtraInfo = GENERAL_LINK_DOWN;
			DBGPRINT(RT_DEBUG_TRACE, ("CntlOidSsidProc():NDIS_STATUS_MEDIA_DISCONNECT Event C!\n"));
		}

		if ((pAd->MlmeAux.SsidBssTab.BssNr == 0) &&
			(pAd->StaCfg.bAutoReconnect == TRUE) &&
			(pAd->MlmeAux.BssType == BSS_INFRA) &&
			(MlmeValidateSSID(pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen) == TRUE)
			)
		{
			MLME_SCAN_REQ_STRUCT       ScanReq;

			DBGPRINT(RT_DEBUG_TRACE, ("CntlOidSsidProc():CNTL - No matching BSS, start a new scan\n"));
			ScanParmFill(pAd, &ScanReq, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, BSS_ANY, SCAN_ACTIVE);
			MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq);
			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;
			
			pAd->StaCfg.LastScanTime = Now;
		}
		else
		{
			pAd->MlmeAux.BssIdx = 0;
			IterateOnBssTab(pAd);
		}
	}
}



VOID CntlOidRTBssidProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	ULONG       BssIdx;
	PUCHAR      pOidBssid = (PUCHAR)Elem->Msg;
	MLME_DISASSOC_REQ_STRUCT    DisassocReq;
	MLME_JOIN_REQ_STRUCT        JoinReq;

	
	COPY_MAC_ADDR(pAd->MlmeAux.Bssid, pOidBssid);
	pAd->MlmeAux.BssType = pAd->StaCfg.BssType;

	
	
	
	NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, MAX_LEN_OF_SSID);
	pAd->MlmeAux.AutoReconnectSsidLen = pAd->MlmeAux.SsidLen;
	NdisMoveMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);

	
	BssIdx = BssTableSearch(&pAd->ScanTab, pOidBssid, pAd->MlmeAux.Channel);
	if (BssIdx == BSS_NOT_FOUND)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CNTL - BSSID not found. reply NDIS_STATUS_NOT_ACCEPTED\n"));
		pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;
		return;
	}

	
	
	pAd->MlmeAux.BssIdx = 0;
	pAd->MlmeAux.SsidBssTab.BssNr = 1;
	NdisMoveMemory(&pAd->MlmeAux.SsidBssTab.BssEntry[0], &pAd->ScanTab.BssEntry[BssIdx], sizeof(BSS_ENTRY));

	
	
	
	
	
	
	
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) &&
		MAC_ADDR_EQUAL(pAd->CommonCfg.Bssid, pOidBssid))
	{
		
		DBGPRINT(RT_DEBUG_TRACE, ("CNTL - already in this BSSID. ignore this SET_BSSID request\n"));
		pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;

            {
                union iwreq_data    wrqu;

                memset(wrqu.ap_addr.sa_data, 0, MAC_ADDR_LEN);
                memcpy(wrqu.ap_addr.sa_data, pAd->MlmeAux.Bssid, MAC_ADDR_LEN);
                wireless_send_event(pAd->net_dev, SIOCGIWAP, &wrqu, NULL);

            }
	}
	else
	{
		if (INFRA_ON(pAd))
		{
			
			DBGPRINT(RT_DEBUG_TRACE, ("CNTL - disassociate with current AP ...\n"));
			DisassocParmFill(pAd, &DisassocReq, pAd->CommonCfg.Bssid, REASON_DISASSOC_STA_LEAVING);
			MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ,
						sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);

			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
		}
		else
		{
			if (ADHOC_ON(pAd))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("CNTL - drop current ADHOC\n"));
				LinkDown(pAd, FALSE);
				OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);
				pAd->IndicateMediaState = NdisMediaStateDisconnected;
				RTMP_IndicateMediaState(pAd);
                pAd->ExtraInfo = GENERAL_LINK_DOWN;
				DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_MEDIA_DISCONNECT Event C!\n"));
			}

			
			pAd->StaCfg.WepStatus   = pAd->StaCfg.OrigWepStatus;
			pAd->StaCfg.PairCipher  = pAd->StaCfg.OrigWepStatus;
			pAd->StaCfg.GroupCipher = pAd->StaCfg.OrigWepStatus;

			
			
			
			if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK))
			{
				pAd->StaCfg.GroupCipher = pAd->ScanTab.BssEntry[BssIdx].WPA.GroupCipher;

				if (pAd->StaCfg.WepStatus == pAd->ScanTab.BssEntry[BssIdx].WPA.PairCipher)
					pAd->StaCfg.PairCipher = pAd->ScanTab.BssEntry[BssIdx].WPA.PairCipher;
				else if (pAd->ScanTab.BssEntry[BssIdx].WPA.PairCipherAux != Ndis802_11WEPDisabled)
					pAd->StaCfg.PairCipher = pAd->ScanTab.BssEntry[BssIdx].WPA.PairCipherAux;
				else	
					pAd->StaCfg.PairCipher = Ndis802_11Encryption2Enabled;
			}
			else if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
			{
				pAd->StaCfg.GroupCipher = pAd->ScanTab.BssEntry[BssIdx].WPA2.GroupCipher;

				if (pAd->StaCfg.WepStatus == pAd->ScanTab.BssEntry[BssIdx].WPA2.PairCipher)
					pAd->StaCfg.PairCipher = pAd->ScanTab.BssEntry[BssIdx].WPA2.PairCipher;
				else if (pAd->ScanTab.BssEntry[BssIdx].WPA2.PairCipherAux != Ndis802_11WEPDisabled)
					pAd->StaCfg.PairCipher = pAd->ScanTab.BssEntry[BssIdx].WPA2.PairCipherAux;
				else	
					pAd->StaCfg.PairCipher = Ndis802_11Encryption2Enabled;

				
				pAd->StaCfg.RsnCapability = pAd->ScanTab.BssEntry[BssIdx].WPA2.RsnCapability;
			}

			
			pAd->StaCfg.bMixCipher = (pAd->StaCfg.PairCipher == pAd->StaCfg.GroupCipher) ? FALSE : TRUE;
			if (pAd->StaCfg.bMixCipher == TRUE)
			{
				
				RTMPMakeRSNIE(pAd, pAd->StaCfg.AuthMode, pAd->StaCfg.WepStatus, 0);
			}
			
			DBGPRINT(RT_DEBUG_TRACE, ("CNTL - joining %02x:%02x:%02x:%02x:%02x:%02x ...\n",
				pOidBssid[0],pOidBssid[1],pOidBssid[2],pOidBssid[3],pOidBssid[4],pOidBssid[5]));

			JoinParmFill(pAd, &JoinReq, pAd->MlmeAux.BssIdx);
			MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_JOIN_REQ, sizeof(MLME_JOIN_REQ_STRUCT), &JoinReq);

			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_JOIN;
		}
	}
}









VOID CntlMlmeRoamingProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	
	
	
	DBGPRINT(RT_DEBUG_TRACE,("CNTL - Roaming in MlmeAux.RoamTab...\n"));

	NdisMoveMemory(&pAd->MlmeAux.SsidBssTab, &pAd->MlmeAux.RoamTab, sizeof(pAd->MlmeAux.RoamTab));
	pAd->MlmeAux.SsidBssTab.BssNr = pAd->MlmeAux.RoamTab.BssNr;

	BssTableSortByRssi(&pAd->MlmeAux.SsidBssTab);
	pAd->MlmeAux.BssIdx = 0;
	IterateOnBssTab(pAd);
}


VOID CntlWaitDisassocProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	MLME_START_REQ_STRUCT     StartReq;

	if (Elem->MsgType == MT2_DISASSOC_CONF)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Dis-associate successful\n"));

	    if (pAd->CommonCfg.bWirelessEvent)
		{
			RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
		}

		LinkDown(pAd, FALSE);

		
		if ((pAd->MlmeAux.SsidBssTab.BssNr==0) && (pAd->StaCfg.BssType == BSS_ADHOC))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("CNTL - No matching BSS, start a new ADHOC (Ssid=%s)...\n",pAd->MlmeAux.Ssid));
			StartParmFill(pAd, &StartReq, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
			MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_START_REQ, sizeof(MLME_START_REQ_STRUCT), &StartReq);
			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_START;
		}
		
		else
		{
			pAd->MlmeAux.BssIdx = 0;

			IterateOnBssTab(pAd);
		}
	}
}


VOID CntlWaitJoinProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT                      Reason;
	MLME_AUTH_REQ_STRUCT        AuthReq;

	if (Elem->MsgType == MT2_JOIN_CONF)
	{
		NdisMoveMemory(&Reason, Elem->Msg, sizeof(USHORT));
		if (Reason == MLME_SUCCESS)
		{
			
			if (pAd->MlmeAux.BssType == BSS_ADHOC)
			{
			    
				
				
				
				if ( (pAd->CommonCfg.bIEEE80211H == 1) &&
                      RadarChannelCheck(pAd, pAd->CommonCfg.Channel)
				   )
				{
					pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;
					DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Channel=%d, Join adhoc on W53(52,56,60,64) Channels are not accepted\n", pAd->CommonCfg.Channel));
					return;
				}

				LinkUp(pAd, BSS_ADHOC);
				pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;
				DBGPRINT(RT_DEBUG_TRACE, ("CNTL - join the IBSS = %02x:%02x:%02x:%02x:%02x:%02x ...\n",
				pAd->CommonCfg.Bssid[0],pAd->CommonCfg.Bssid[1],pAd->CommonCfg.Bssid[2],
				pAd->CommonCfg.Bssid[3],pAd->CommonCfg.Bssid[4],pAd->CommonCfg.Bssid[5]));

                pAd->IndicateMediaState = NdisMediaStateConnected;
                pAd->ExtraInfo = GENERAL_LINK_UP;
			}
			
			else
			{
				{
					
					if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeShared) ||
						(pAd->StaCfg.AuthMode == Ndis802_11AuthModeAutoSwitch))
					{
						AuthParmFill(pAd, &AuthReq, pAd->MlmeAux.Bssid, Ndis802_11AuthModeShared);
					}
					else
					{
						AuthParmFill(pAd, &AuthReq, pAd->MlmeAux.Bssid, Ndis802_11AuthModeOpen);
					}
				}
				MlmeEnqueue(pAd, AUTH_STATE_MACHINE, MT2_MLME_AUTH_REQ,
							sizeof(MLME_AUTH_REQ_STRUCT), &AuthReq);

				pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_AUTH;
			}
		}
		else
		{
			
			pAd->MlmeAux.BssIdx++;
			IterateOnBssTab(pAd);
		}
	}
}



VOID CntlWaitStartProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT      Result;

	if (Elem->MsgType == MT2_START_CONF)
	{
		NdisMoveMemory(&Result, Elem->Msg, sizeof(USHORT));
		if (Result == MLME_SUCCESS)
		{
		    
			
			
			
			if ( (pAd->CommonCfg.bIEEE80211H == 1) &&
                  RadarChannelCheck(pAd, pAd->CommonCfg.Channel)
			   )
			{
				pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;
				DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Channel=%d, Start adhoc on W53(52,56,60,64) Channels are not accepted\n", pAd->CommonCfg.Channel));
				return;
			}

			if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
			{
				N_ChannelCheck(pAd);
				SetCommonHT(pAd);
				NdisMoveMemory(&pAd->MlmeAux.AddHtInfo, &pAd->CommonCfg.AddHTInfo, sizeof(ADD_HT_INFO_IE));
				RTMPCheckHt(pAd, BSSID_WCID, &pAd->CommonCfg.HtCapability, &pAd->CommonCfg.AddHTInfo);
				pAd->StaActive.SupportedPhyInfo.bHtEnable = TRUE;
				NdisZeroMemory(&pAd->StaActive.SupportedPhyInfo.MCSSet[0], 16);
				NdisMoveMemory(&pAd->StaActive.SupportedPhyInfo.MCSSet[0], &pAd->CommonCfg.HtCapability.MCSSet[0], 16);
				COPY_HTSETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(pAd);

				if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) &&
					(pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset == EXTCHA_ABOVE))
				{
					pAd->MlmeAux.CentralChannel = pAd->CommonCfg.Channel + 2;
				}
				else if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) &&
						 (pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset == EXTCHA_BELOW))
				{
					pAd->MlmeAux.CentralChannel = pAd->CommonCfg.Channel - 2;
				}
			}
			else
			{
				pAd->StaActive.SupportedPhyInfo.bHtEnable = FALSE;
			}
			LinkUp(pAd, BSS_ADHOC);
			pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;
			
			if ((pAd->CommonCfg.Channel > 14 )
				&& (pAd->CommonCfg.bIEEE80211H == 1)
				&& RadarChannelCheck(pAd, pAd->CommonCfg.Channel))
			{
				pAd->CommonCfg.RadarDetect.RDMode = RD_SILENCE_MODE;
				pAd->CommonCfg.RadarDetect.RDCount = 0;
			}

			DBGPRINT(RT_DEBUG_TRACE, ("CNTL - start a new IBSS = %02x:%02x:%02x:%02x:%02x:%02x ...\n",
				pAd->CommonCfg.Bssid[0],pAd->CommonCfg.Bssid[1],pAd->CommonCfg.Bssid[2],
				pAd->CommonCfg.Bssid[3],pAd->CommonCfg.Bssid[4],pAd->CommonCfg.Bssid[5]));
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Start IBSS fail. BUG!!!!!\n"));
			pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;
		}
	}
}


VOID CntlWaitAuthProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT                       Reason;
	MLME_ASSOC_REQ_STRUCT        AssocReq;
	MLME_AUTH_REQ_STRUCT         AuthReq;

	if (Elem->MsgType == MT2_AUTH_CONF)
	{
		NdisMoveMemory(&Reason, Elem->Msg, sizeof(USHORT));
		if (Reason == MLME_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("CNTL - AUTH OK\n"));
			AssocParmFill(pAd, &AssocReq, pAd->MlmeAux.Bssid, pAd->MlmeAux.CapabilityInfo,
						  ASSOC_TIMEOUT, pAd->StaCfg.DefaultListenCount);

			{
				MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_MLME_ASSOC_REQ,
							sizeof(MLME_ASSOC_REQ_STRUCT), &AssocReq);

				pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_ASSOC;
			}
		}
		else
		{
			
			
			
			DBGPRINT(RT_DEBUG_TRACE, ("CNTL - AUTH FAIL, try again...\n"));

			{
				if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeShared) ||
					(pAd->StaCfg.AuthMode == Ndis802_11AuthModeAutoSwitch))
				{
					
					AuthParmFill(pAd, &AuthReq, pAd->MlmeAux.Bssid, Ndis802_11AuthModeShared);
				}
				else
				{
					AuthParmFill(pAd, &AuthReq, pAd->MlmeAux.Bssid, Ndis802_11AuthModeOpen);
				}
			}
			MlmeEnqueue(pAd, AUTH_STATE_MACHINE, MT2_MLME_AUTH_REQ,
						sizeof(MLME_AUTH_REQ_STRUCT), &AuthReq);

			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_AUTH2;
		}
	}
}


VOID CntlWaitAuthProc2(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT                       Reason;
	MLME_ASSOC_REQ_STRUCT        AssocReq;
	MLME_AUTH_REQ_STRUCT         AuthReq;

	if (Elem->MsgType == MT2_AUTH_CONF)
	{
		NdisMoveMemory(&Reason, Elem->Msg, sizeof(USHORT));
		if (Reason == MLME_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("CNTL - AUTH OK\n"));
			AssocParmFill(pAd, &AssocReq, pAd->MlmeAux.Bssid, pAd->MlmeAux.CapabilityInfo,
						  ASSOC_TIMEOUT, pAd->StaCfg.DefaultListenCount);
			MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_MLME_ASSOC_REQ,
						sizeof(MLME_ASSOC_REQ_STRUCT), &AssocReq);

			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_ASSOC;
		}
		else
		{
			if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeAutoSwitch) &&
				 (pAd->MlmeAux.Alg == Ndis802_11AuthModeShared))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("CNTL - AUTH FAIL, try OPEN system...\n"));
				AuthParmFill(pAd, &AuthReq, pAd->MlmeAux.Bssid, Ndis802_11AuthModeOpen);
				MlmeEnqueue(pAd, AUTH_STATE_MACHINE, MT2_MLME_AUTH_REQ,
							sizeof(MLME_AUTH_REQ_STRUCT), &AuthReq);

				pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_AUTH2;
			}
			else
			{
				
				DBGPRINT(RT_DEBUG_TRACE, ("CNTL - AUTH FAIL, give up; try next BSS\n"));
				pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE; 
				pAd->MlmeAux.BssIdx++;
				IterateOnBssTab(pAd);
			}
		}
	}
}


VOID CntlWaitAssocProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT      Reason;

	if (Elem->MsgType == MT2_ASSOC_CONF)
	{
		NdisMoveMemory(&Reason, Elem->Msg, sizeof(USHORT));
		if (Reason == MLME_SUCCESS)
		{
			LinkUp(pAd, BSS_INFRA);
			pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;
			DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Association successful on BSS #%ld\n",pAd->MlmeAux.BssIdx));

			if (pAd->CommonCfg.bWirelessEvent)
			{
				RTMPSendWirelessEvent(pAd, IW_ASSOC_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
			}
		}
		else
		{
			
			DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Association fails on BSS #%ld\n",pAd->MlmeAux.BssIdx));
			pAd->MlmeAux.BssIdx++;
			IterateOnBssTab(pAd);
		}
	}
}


VOID CntlWaitReassocProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT      Result;

	if (Elem->MsgType == MT2_REASSOC_CONF)
	{
		NdisMoveMemory(&Result, Elem->Msg, sizeof(USHORT));
		if (Result == MLME_SUCCESS)
		{
			
			
			
			LinkUp(pAd, BSS_INFRA);

			
			if (pAd->CommonCfg.bWirelessEvent)
				RTMPSendWirelessEvent(pAd, IW_ASSOC_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);

			pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;
			DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Re-assocition successful on BSS #%ld\n", pAd->MlmeAux.RoamIdx));
		}
		else
		{
			
			DBGPRINT(RT_DEBUG_TRACE, ("CNTL - Re-assocition fails on BSS #%ld\n", pAd->MlmeAux.RoamIdx));
			pAd->MlmeAux.RoamIdx++;
			IterateOnBssTab2(pAd);
		}
	}
}


#ifdef RT2870
VOID	AdhocTurnOnQos(
	IN  PRTMP_ADAPTER pAd)
{
#define AC0_DEF_TXOP		0
#define AC1_DEF_TXOP		0
#define AC2_DEF_TXOP		94
#define AC3_DEF_TXOP		47

	
	if (pAd->CommonCfg.APEdcaParm.bValid == FALSE)
	{
		pAd->CommonCfg.APEdcaParm.bValid = TRUE;
		pAd->CommonCfg.APEdcaParm.Aifsn[0] = 3;
		pAd->CommonCfg.APEdcaParm.Aifsn[1] = 7;
		pAd->CommonCfg.APEdcaParm.Aifsn[2] = 1;
		pAd->CommonCfg.APEdcaParm.Aifsn[3] = 1;

		pAd->CommonCfg.APEdcaParm.Cwmin[0] = 4;
		pAd->CommonCfg.APEdcaParm.Cwmin[1] = 4;
		pAd->CommonCfg.APEdcaParm.Cwmin[2] = 3;
		pAd->CommonCfg.APEdcaParm.Cwmin[3] = 2;

		pAd->CommonCfg.APEdcaParm.Cwmax[0] = 10;
		pAd->CommonCfg.APEdcaParm.Cwmax[1] = 6;
		pAd->CommonCfg.APEdcaParm.Cwmax[2] = 4;
		pAd->CommonCfg.APEdcaParm.Cwmax[3] = 3;

		pAd->CommonCfg.APEdcaParm.Txop[0]  = 0;
		pAd->CommonCfg.APEdcaParm.Txop[1]  = 0;
		pAd->CommonCfg.APEdcaParm.Txop[2]  = AC2_DEF_TXOP;
		pAd->CommonCfg.APEdcaParm.Txop[3]  = AC3_DEF_TXOP;
	}
	AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm);
}
#endif 


VOID LinkUp(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR BssType)
{
	ULONG	Now;
	UINT32	Data;
	BOOLEAN	Cancelled;
	UCHAR	Value = 0, idx;
	MAC_TABLE_ENTRY *pEntry = NULL, *pCurrEntry;

#ifdef RT2860
	if (RTMP_TEST_PSFLAG(pAd, fRTMP_PS_SET_PCI_CLK_OFF_COMMAND))
	{
		RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_HALT);
		RTMPusecDelay(6000);
		pAd->bPCIclkOff = FALSE;
	}
#endif

	pEntry = &pAd->MacTab.Content[BSSID_WCID];

	
	
	
	
	
	
	
	
	
	RTMPCancelTimer(&pAd->MlmeAux.DisassocTimer,  &Cancelled);

	COPY_SETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(pAd);

	COPY_HTSETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(pAd);

	
	
	
	
	SwitchBetweenWepAndCkip(pAd);

#ifdef RT2860
	
	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
	if(pAd->Antenna.field.RxPath == 3)
	{
		Value |= (0x10);
	}
	else if(pAd->Antenna.field.RxPath == 2)
	{
		Value |= (0x8);
	}
	else if(pAd->Antenna.field.RxPath == 1)
	{
		Value |= (0x0);
	}
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);
	pAd->StaCfg.BBPR3 = Value;
#endif 

	if (BssType == BSS_ADHOC)
	{
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_ADHOC_ON);
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_INFRA_ON);

		if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) &&
			(pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset == EXTCHA_ABOVE))
		{
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel + 2;
		}
		else if ((pAd->CommonCfg.Channel > 2) &&
				 (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) &&
				 (pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset == EXTCHA_BELOW))
		{
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 2;
		}
#ifdef RT2870
		if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
			AdhocTurnOnQos(pAd);
#endif

		DBGPRINT(RT_DEBUG_TRACE, ("!!!Adhoc LINK UP !!! \n" ));
	}
	else
	{
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_INFRA_ON);
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADHOC_ON);

		DBGPRINT(RT_DEBUG_TRACE, ("!!!Infra LINK UP !!! \n" ));
	}

	
	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
	Value &= (~0x01);
	Value |= pAd->CommonCfg.RegTransmitSetting.field.TxBF;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);

	
    if ((pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel) && (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
	{
		
		pAd->CommonCfg.BBPCurrentBW = BW_40;
		AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
		AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
		Value &= (~0x18);
		Value |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);

		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
		Value &= (~0x20);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);
#ifdef RT2860
        pAd->StaCfg.BBPR3 = Value;
#endif

		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Data);
		Data &= 0xfffffffe;
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Data);

		if (pAd->MACVersion == 0x28600100)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x1A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x16);
            DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
		}

		DBGPRINT(RT_DEBUG_TRACE, ("!!!40MHz Lower LINK UP !!! Control Channel at Below. Central = %d \n", pAd->CommonCfg.CentralChannel ));
	}
	else if ((pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel) && (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
    {
	    
		pAd->CommonCfg.BBPCurrentBW = BW_40;
		AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
	    AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
		Value &= (~0x18);
		Value |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);

		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Data);
		Data |= 0x1;
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Data);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
	    Value |= (0x20);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);
#ifdef RT2860
        pAd->StaCfg.BBPR3 = Value;
#endif

		if (pAd->MACVersion == 0x28600100)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x1A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x16);
			    DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
		}

	    DBGPRINT(RT_DEBUG_TRACE, ("!!! 40MHz Upper LINK UP !!! Control Channel at UpperCentral = %d \n", pAd->CommonCfg.CentralChannel ));
    }
    else
    {
	    pAd->CommonCfg.BBPCurrentBW = BW_20;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
		AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
		AsicLockChannel(pAd, pAd->CommonCfg.Channel);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
		Value &= (~0x18);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);

		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Data);
		Data &= 0xfffffffe;
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Data);

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
		Value &= (~0x20);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);
#ifdef RT2860
        pAd->StaCfg.BBPR3 = Value;
#endif

		if (pAd->MACVersion == 0x28600100)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x16);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x08);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x11);
            DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
		}

	    DBGPRINT(RT_DEBUG_TRACE, ("!!! 20MHz LINK UP !!! \n" ));
    }

	RTMPSetAGCInitValue(pAd, pAd->CommonCfg.BBPCurrentBW);
	
	
	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R66, &pAd->BbpTuning.R66CurrentValue);

	DBGPRINT(RT_DEBUG_TRACE, ("!!! LINK UP !!! (BssType=%d, AID=%d, ssid=%s, Channel=%d, CentralChannel = %d)\n",
		BssType, pAd->StaActive.Aid, pAd->CommonCfg.Ssid, pAd->CommonCfg.Channel, pAd->CommonCfg.CentralChannel));

	DBGPRINT(RT_DEBUG_TRACE, ("!!! LINK UP !!! (Density =%d, )\n", pAd->MacTab.Content[BSSID_WCID].MpduDensity));

		AsicSetBssid(pAd, pAd->CommonCfg.Bssid);

	AsicSetSlotTime(pAd, TRUE);
	AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm);

	
	AsicUpdateProtect(pAd, 0, (OFDMSETPROTECT | CCKSETPROTECT), TRUE, FALSE);

	if ((pAd->StaActive.SupportedPhyInfo.bHtEnable == TRUE))
	{
		
    	if (pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent == 1)
    	{
    		AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode,  ALLN_SETPROTECT, FALSE, TRUE);
    	}
    	else
   			AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode,  ALLN_SETPROTECT, FALSE, FALSE);
	}

	NdisZeroMemory(&pAd->DrsCounters, sizeof(COUNTER_DRS));

	NdisGetSystemUpTime(&Now);
	pAd->StaCfg.LastBeaconRxTime = Now;   

	if ((pAd->CommonCfg.TxPreamble != Rt802_11PreambleLong) &&
		CAP_IS_SHORT_PREAMBLE_ON(pAd->StaActive.CapabilityInfo))
	{
		MlmeSetTxPreamble(pAd, Rt802_11PreambleShort);
	}

	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED);

	if (pAd->CommonCfg.RadarDetect.RDMode == RD_SILENCE_MODE)
	{
	}
	pAd->CommonCfg.RadarDetect.RDMode = RD_NORMAL_MODE;

	if (BssType == BSS_ADHOC)
	{
		MakeIbssBeacon(pAd);
		if ((pAd->CommonCfg.Channel > 14)
			&& (pAd->CommonCfg.bIEEE80211H == 1)
			&& RadarChannelCheck(pAd, pAd->CommonCfg.Channel))
		{
			; 
		}
		else
		{
			AsicEnableIbssSync(pAd);
		}

		
		
		RTMP_IO_WRITE32(pAd, MAC_WCID_BASE, 0x00);
		RTMP_IO_WRITE32(pAd, 0x1808, 0x00);

		
		

		if (pAd->StaCfg.WepStatus == Ndis802_11WEPEnabled)
		{
			PUCHAR	Key;
			UCHAR 	CipherAlg;

			for (idx=0; idx < SHARE_KEY_NUM; idx++)
        	{
				CipherAlg = pAd->SharedKey[BSS0][idx].CipherAlg;
    			Key = pAd->SharedKey[BSS0][idx].Key;

				if (pAd->SharedKey[BSS0][idx].KeyLen > 0)
				{
					
    				AsicAddSharedKeyEntry(pAd, BSS0, idx, CipherAlg, Key, NULL, NULL);

                    if (idx == pAd->StaCfg.DefaultKeyId)
					{
						
						RTMPAddWcidAttributeEntry(pAd, BSS0, idx, CipherAlg, NULL);
					}
				}


			}
		}
		
		
		else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPANone)
		{
			pAd->StaCfg.DefaultKeyId = 0;	

            NdisZeroMemory(&pAd->SharedKey[BSS0][0], sizeof(CIPHER_KEY));
							pAd->SharedKey[BSS0][0].KeyLen = LEN_TKIP_EK;
			NdisMoveMemory(pAd->SharedKey[BSS0][0].Key, pAd->StaCfg.PMK, LEN_TKIP_EK);

            if (pAd->StaCfg.PairCipher == Ndis802_11Encryption2Enabled)
            {
    			NdisMoveMemory(pAd->SharedKey[BSS0][0].RxMic, &pAd->StaCfg.PMK[16], LEN_TKIP_RXMICK);
    			NdisMoveMemory(pAd->SharedKey[BSS0][0].TxMic, &pAd->StaCfg.PMK[16], LEN_TKIP_TXMICK);
            }

			
			if (pAd->StaCfg.PairCipher == Ndis802_11Encryption2Enabled)
				pAd->SharedKey[BSS0][0].CipherAlg = CIPHER_TKIP;
			else if (pAd->StaCfg.PairCipher == Ndis802_11Encryption3Enabled)
				pAd->SharedKey[BSS0][0].CipherAlg = CIPHER_AES;
			else
            {
                DBGPRINT(RT_DEBUG_TRACE, ("Unknow Cipher (=%d), set Cipher to AES\n", pAd->StaCfg.PairCipher));
				pAd->SharedKey[BSS0][0].CipherAlg = CIPHER_AES;
            }

			
			AsicAddSharedKeyEntry(pAd,
								  BSS0,
								  0,
								  pAd->SharedKey[BSS0][0].CipherAlg,
								  pAd->SharedKey[BSS0][0].Key,
								  pAd->SharedKey[BSS0][0].TxMic,
								  pAd->SharedKey[BSS0][0].RxMic);

            
			RTMPAddWcidAttributeEntry(pAd, BSS0, 0, pAd->SharedKey[BSS0][0].CipherAlg, NULL);

		}

	}
	else 
	{
		
		while (Cancelled == TRUE)
		{
			if (pAd->CommonCfg.LastSsidLen == pAd->CommonCfg.SsidLen)
			{
				if (RTMPCompareMemory(pAd->CommonCfg.LastSsid, pAd->CommonCfg.Ssid, pAd->CommonCfg.LastSsidLen) == 0)
				{
					
					break;
				}
			}
			
			pAd->IndicateMediaState = NdisMediaStateDisconnected;
			RTMP_IndicateMediaState(pAd);
            pAd->ExtraInfo = GENERAL_LINK_DOWN;
			DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_MEDIA_DISCONNECT Event AA!\n"));
			break;
		}

		
		
		
		
		if ((pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA))
		{
			ULONG 		IV;

			
			RTMPWPARemoveAllKeys(pAd);
			pAd->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			pAd->StaCfg.PrivacyFilter = Ndis802_11PrivFilter8021xWEP;

			
			
			IV = 0;
			IV |= (pAd->StaCfg.DefaultKeyId << 30);
			AsicUpdateWCIDIVEIV(pAd, BSSID_WCID, IV, 0);

#ifdef RT2860
			RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP);
#endif
		}
		
		
		

		
		
		

		ComposePsPoll(pAd);
		ComposeNullFrame(pAd);

			AsicEnableBssSync(pAd);

		
		AsicUpdateRxWCIDTable(pAd, BSSID_WCID, pAd->CommonCfg.Bssid);

		NdisAcquireSpinLock(&pAd->MacTabLock);
		
		{
			UCHAR HashIdx;

			
			HashIdx = MAC_ADDR_HASH_INDEX(pAd->CommonCfg.Bssid);
			if (pAd->MacTab.Hash[HashIdx] == NULL)
			{
				pAd->MacTab.Hash[HashIdx] = pEntry;
			}
			else
			{
				pCurrEntry = pAd->MacTab.Hash[HashIdx];
				while (pCurrEntry->pNext != NULL)
					pCurrEntry = pCurrEntry->pNext;
				pCurrEntry->pNext = pEntry;
			}
		}
		NdisReleaseSpinLock(&pAd->MacTabLock);


		
        if (((pAd->StaCfg.WpaSupplicantUP)&&
             (pAd->StaCfg.WepStatus == Ndis802_11WEPEnabled)&&
             (pAd->StaCfg.PortSecured == WPA_802_1X_PORT_SECURED)) ||
            ((pAd->StaCfg.WpaSupplicantUP == WPA_SUPPLICANT_DISABLE)&&
              (pAd->StaCfg.WepStatus == Ndis802_11WEPEnabled)))
		{
			PUCHAR	Key;
			UCHAR 	CipherAlg;

			for (idx=0; idx < SHARE_KEY_NUM; idx++)
        	{
				CipherAlg = pAd->SharedKey[BSS0][idx].CipherAlg;
    			Key = pAd->SharedKey[BSS0][idx].Key;

				if (pAd->SharedKey[BSS0][idx].KeyLen > 0)
				{
					
    				AsicAddSharedKeyEntry(pAd, BSS0, idx, CipherAlg, Key, NULL, NULL);

					if (idx == pAd->StaCfg.DefaultKeyId)
					{
						
						RTMPAddWcidAttributeEntry(pAd, BSS0, idx, CipherAlg, NULL);

						
						RTMPAddWcidAttributeEntry(pAd, BSS0, idx, CipherAlg, pEntry);
					}
				}
			}
		}

		
		
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);

        
		if (pAd->StaCfg.AuthMode < Ndis802_11AuthModeWPA)
		{
			pAd->IndicateMediaState = NdisMediaStateConnected;
			pAd->ExtraInfo = GENERAL_LINK_UP;
#ifdef RT2870
			RTMP_IndicateMediaState(pAd);
#endif
		}
        
#ifdef RT2860
		RTMP_IndicateMediaState(pAd);
#endif

		
        NdisAcquireSpinLock(&pAd->MacTabLock);
		RTMPMoveMemory(pAd->MacTab.Content[BSSID_WCID].Addr, pAd->CommonCfg.Bssid, MAC_ADDR_LEN);
		pAd->MacTab.Content[BSSID_WCID].Aid = BSSID_WCID;
		pAd->MacTab.Content[BSSID_WCID].pAd = pAd;
		pAd->MacTab.Content[BSSID_WCID].ValidAsCLI = TRUE;	
		pAd->MacTab.Size = 1;	
		pAd->MacTab.Content[BSSID_WCID].Sst = SST_ASSOC;
		pAd->MacTab.Content[BSSID_WCID].AuthState = SST_ASSOC;
		pAd->MacTab.Content[BSSID_WCID].AuthMode = pAd->StaCfg.AuthMode;
		pAd->MacTab.Content[BSSID_WCID].WepStatus = pAd->StaCfg.WepStatus;
        NdisReleaseSpinLock(&pAd->MacTabLock);

		DBGPRINT(RT_DEBUG_TRACE, ("!!! LINK UP !!!  ClientStatusFlags=%lx)\n",
			pAd->MacTab.Content[BSSID_WCID].ClientStatusFlags));

		MlmeUpdateTxRates(pAd, TRUE, BSS0);
		MlmeUpdateHtTxRates(pAd, BSS0);
		DBGPRINT(RT_DEBUG_TRACE, ("!!! LINK UP !! (StaActive.bHtEnable =%d, )\n", pAd->StaActive.SupportedPhyInfo.bHtEnable));

		if (pAd->CommonCfg.bAggregationCapable)
		{
			if ((pAd->CommonCfg.bPiggyBackCapable) && (pAd->MlmeAux.APRalinkIe & 0x00000003) == 3)
			{

				OPSTATUS_SET_FLAG(pAd, fOP_STATUS_PIGGYBACK_INUSED);
				OPSTATUS_SET_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED);
                RTMPSetPiggyBack(pAd, TRUE);
				DBGPRINT(RT_DEBUG_TRACE, ("Turn on Piggy-Back\n"));
			}
			else if (pAd->MlmeAux.APRalinkIe & 0x00000001)
			{
				OPSTATUS_SET_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED);
			}
		}

		if (pAd->MlmeAux.APRalinkIe != 0x0)
		{
			if (CLIENT_STATUS_TEST_FLAG(&pAd->MacTab.Content[BSSID_WCID], fCLIENT_STATUS_RDG_CAPABLE))
			{
				AsicEnableRDG(pAd);
			}

			OPSTATUS_SET_FLAG(pAd, fCLIENT_STATUS_RALINK_CHIPSET);
			CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[BSSID_WCID], fCLIENT_STATUS_RALINK_CHIPSET);
		}
		else
		{
			OPSTATUS_CLEAR_FLAG(pAd, fCLIENT_STATUS_RALINK_CHIPSET);
			CLIENT_STATUS_CLEAR_FLAG(&pAd->MacTab.Content[BSSID_WCID], fCLIENT_STATUS_RALINK_CHIPSET);
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_MEDIA_CONNECT Event B!.BACapability = %x. ClientStatusFlags = %lx\n", pAd->CommonCfg.BACapability.word, pAd->MacTab.Content[BSSID_WCID].ClientStatusFlags));

	
	RTMPSetLED(pAd, LED_LINK_UP);

	pAd->Mlme.PeriodicRound = 0;
	pAd->Mlme.OneSecPeriodicRound = 0;
	pAd->bConfigChanged = FALSE;        
	pAd->ExtraInfo = GENERAL_LINK_UP;   

	
	{
		PUCHAR					pTable;
		UCHAR					TableSize = 0;

		MlmeSelectTxRateTable(pAd, &pAd->MacTab.Content[BSSID_WCID], &pTable, &TableSize, &pAd->CommonCfg.TxRateIndex);
		AsicUpdateAutoFallBackTable(pAd, pTable);
	}

	NdisAcquireSpinLock(&pAd->MacTabLock);
    pEntry->HTPhyMode.word = pAd->StaCfg.HTPhyMode.word;
    pEntry->MaxHTPhyMode.word = pAd->StaCfg.HTPhyMode.word;
	if (pAd->StaCfg.bAutoTxRateSwitch == FALSE)
	{
		pEntry->bAutoTxRateSwitch = FALSE;

		if (pEntry->HTPhyMode.field.MCS == 32)
			pEntry->HTPhyMode.field.ShortGI = GI_800;

		if ((pEntry->HTPhyMode.field.MCS > MCS_7) || (pEntry->HTPhyMode.field.MCS == 32))
			pEntry->HTPhyMode.field.STBC = STBC_NONE;

		
		if (pEntry->HTPhyMode.field.MODE <= MODE_OFDM)
			RTMPUpdateLegacyTxSetting((UCHAR)pAd->StaCfg.DesiredTransmitSetting.field.FixedTxMode, pEntry);
	}
	else
		pEntry->bAutoTxRateSwitch = TRUE;
	NdisReleaseSpinLock(&pAd->MacTabLock);

	
	pAd->LastTxRate = (USHORT)(pEntry->HTPhyMode.word);
	
	if (pAd->StaActive.SupportedPhyInfo.MCSSet[0] != 0x00)
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &Value);
		Value &= (~0x18);
		if (pAd->Antenna.field.TxPath == 2)
		{
		    Value |= 0x10;
		}
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, Value);
	}
	else
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &Value);
		Value &= (~0x18);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, Value);
	}

	if (pAd->StaActive.SupportedPhyInfo.bHtEnable == FALSE)
	{
	}
	else if (pEntry->MaxRAmpduFactor == 0)
	{
	    
	    
		RTMP_IO_WRITE32(pAd, MAX_LEN_CFG, 0x0fff);
	}

	
	
	
	
	
	
	
	
	
	
	if (
		!(pAd->CommonCfg.RxStream == 1 && pAd->CommonCfg.TxStream == 1) &&
		(((pAd->StaActive.SupportedPhyInfo.bHtEnable == FALSE) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED))
		|| ((pAd->StaActive.SupportedPhyInfo.bHtEnable == TRUE) && (pAd->CommonCfg.BACapability.field.Policy == BA_NOTUSE))))
	{
		RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);
		Data  &= 0xFFFFFF00;
		RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);

		RTMP_IO_WRITE32(pAd, PBF_MAX_PCNT, 0x1F3F7F9F);
		DBGPRINT(RT_DEBUG_TRACE, ("Txburst 1\n"));
	}
	else
	if (pAd->CommonCfg.bEnableTxBurst)
	{
		RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);
		Data  &= 0xFFFFFF00;
		Data  |= 0x60;
		RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);
		pAd->CommonCfg.IOTestParm.bNowAtherosBurstOn = TRUE;

		RTMP_IO_WRITE32(pAd, PBF_MAX_PCNT, 0x1F3FBF9F);
		DBGPRINT(RT_DEBUG_TRACE, ("Txburst 2\n"));
	}
	else
	{
		RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);
		Data  &= 0xFFFFFF00;
		RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);

		RTMP_IO_WRITE32(pAd, PBF_MAX_PCNT, 0x1F3F7F9F);
		DBGPRINT(RT_DEBUG_TRACE, ("Txburst 3\n"));
	}

	
	if ((pAd->CommonCfg.IOTestParm.bLastAtheros == TRUE) && ((STA_WEP_ON(pAd))||(STA_TKIP_ON(pAd))))
	{
		pAd->CommonCfg.IOTestParm.bNextDisableRxBA = TRUE;
		if (pAd->CommonCfg.bEnableTxBurst)
		{
		    UINT32 MACValue = 0;
			
			
			RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &MACValue);
			MACValue  &= 0xFFFFFF00;
			RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, MACValue);
			pAd->CommonCfg.IOTestParm.bNowAtherosBurstOn = FALSE;
		}
	}
	else
	{
		pAd->CommonCfg.IOTestParm.bNextDisableRxBA = FALSE;
	}

	pAd->CommonCfg.IOTestParm.bLastAtheros = FALSE;
	COPY_MAC_ADDR(pAd->CommonCfg.LastBssid, pAd->CommonCfg.Bssid);
	DBGPRINT(RT_DEBUG_TRACE, ("!!!pAd->bNextDisableRxBA= %d \n", pAd->CommonCfg.IOTestParm.bNextDisableRxBA));
	
	
	

    if (pAd->StaCfg.WepStatus <= Ndis802_11WEPDisabled)
    {
        pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED;
		pAd->StaCfg.PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
	}

	NdisAcquireSpinLock(&pAd->MacTabLock);
	pEntry->PortSecured = pAd->StaCfg.PortSecured;
	NdisReleaseSpinLock(&pAd->MacTabLock);

    
	
	
	
	if (INFRA_ON(pAd) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) && STA_TKIP_ON(pAd))
	{
		RTMP_IO_WRITE32(pAd, RX_PARSER_CFG, 0x01);
	}
	else
	{
		RTMP_IO_WRITE32(pAd, RX_PARSER_CFG, 0x00);
	}

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
#ifdef RT2860
	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_GO_TO_SLEEP_NOW);
#endif
}


VOID LinkDown(
	IN PRTMP_ADAPTER pAd,
	IN  BOOLEAN      IsReqFromAP)
{
	UCHAR			    i, ByteValue = 0;
#ifdef RT2860
	BOOLEAN		Cancelled;
#endif

	
	if (MONITOR_ON(pAd))
		return;

#ifdef RT2860
	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_GO_TO_SLEEP_NOW);
	RTMPCancelTimer(&pAd->Mlme.PsPollTimer,		&Cancelled);

	
	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP);
#endif

    if (pAd->CommonCfg.bWirelessEvent)
	{
		RTMPSendWirelessEvent(pAd, IW_STA_LINKDOWN_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("!!! LINK DOWN !!!\n"));
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED);

#ifdef RT2860
    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
    {
	    BOOLEAN Cancelled;
        pAd->Mlme.bPsPollTimerRunning = FALSE;
        RTMPCancelTimer(&pAd->Mlme.PsPollTimer,	&Cancelled);
    }

    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE) ||
		RTMP_TEST_PSFLAG(pAd, fRTMP_PS_SET_PCI_CLK_OFF_COMMAND) ||
		RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
    {
		AsicForceWakeup(pAd, RTMP_HALT);
        OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
    }

    pAd->bPCIclkOff = FALSE;
#endif
	if (ADHOC_ON(pAd))		
	{
		DBGPRINT(RT_DEBUG_TRACE, ("!!! LINK DOWN 1!!!\n"));

		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADHOC_ON);
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);
		pAd->IndicateMediaState = NdisMediaStateDisconnected;
		RTMP_IndicateMediaState(pAd);
        pAd->ExtraInfo = GENERAL_LINK_DOWN;
		BssTableDeleteEntry(&pAd->ScanTab, pAd->CommonCfg.Bssid, pAd->CommonCfg.Channel);
		DBGPRINT(RT_DEBUG_TRACE, ("!!! MacTab.Size=%d !!!\n", pAd->MacTab.Size));
	}
	else					
	{
		DBGPRINT(RT_DEBUG_TRACE, ("!!! LINK DOWN 2!!!\n"));

		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_INFRA_ON);
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);

		
		pAd->CommonCfg.LastSsidLen = pAd->CommonCfg.SsidLen;
		NdisMoveMemory(pAd->CommonCfg.LastSsid, pAd->CommonCfg.Ssid, pAd->CommonCfg.LastSsidLen);
		COPY_MAC_ADDR(pAd->CommonCfg.LastBssid, pAd->CommonCfg.Bssid);
		if (pAd->MlmeAux.CurrReqIsFromNdis == TRUE)
		{
			pAd->IndicateMediaState = NdisMediaStateDisconnected;
			RTMP_IndicateMediaState(pAd);
            pAd->ExtraInfo = GENERAL_LINK_DOWN;
			DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_MEDIA_DISCONNECT Event A!\n"));
			pAd->MlmeAux.CurrReqIsFromNdis = FALSE;
		}
		else
		{
            
			
			
			
			
			
			BssTableDeleteEntry(&pAd->ScanTab, pAd->CommonCfg.Bssid, pAd->CommonCfg.Channel);
		}

		
		
		
		
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);

		if (pAd->StaCfg.CCXAdjacentAPReportFlag == TRUE)
		{
			
			
			
			
			pAd->StaCfg.CCXAdjacentAPChannel = pAd->CommonCfg.Channel;
			pAd->StaCfg.CCXAdjacentAPSsidLen = pAd->CommonCfg.SsidLen;
			NdisMoveMemory(pAd->StaCfg.CCXAdjacentAPSsid, pAd->CommonCfg.Ssid, pAd->StaCfg.CCXAdjacentAPSsidLen);
			COPY_MAC_ADDR(pAd->StaCfg.CCXAdjacentAPBssid, pAd->CommonCfg.Bssid);
		}
	}

	for (i=1; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		if (pAd->MacTab.Content[i].ValidAsCLI == TRUE)
			MacTableDeleteEntry(pAd, pAd->MacTab.Content[i].Aid, pAd->MacTab.Content[i].Addr);
	}

	pAd->StaCfg.CCXQosECWMin	= 4;
	pAd->StaCfg.CCXQosECWMax	= 10;

	AsicSetSlotTime(pAd, TRUE); 
	AsicSetEdcaParm(pAd, NULL);

	
	RTMPSetLED(pAd, LED_LINK_DOWN);
    pAd->LedIndicatorStregth = 0xF0;
    RTMPSetSignalLED(pAd, -100);	

		AsicDisableSync(pAd);

	pAd->Mlme.PeriodicRound = 0;
	pAd->Mlme.OneSecPeriodicRound = 0;

	if (pAd->StaCfg.BssType == BSS_INFRA)
	{
		
		NdisZeroMemory(pAd->CommonCfg.Bssid, MAC_ADDR_LEN);
		NdisZeroMemory(pAd->CommonCfg.Ssid, MAX_LEN_OF_SSID);
		pAd->CommonCfg.SsidLen = 0;
	}

	NdisZeroMemory(&pAd->MlmeAux.HtCapability, sizeof(HT_CAPABILITY_IE));
	NdisZeroMemory(&pAd->MlmeAux.AddHtInfo, sizeof(ADD_HT_INFO_IE));
	pAd->MlmeAux.HtCapabilityLen = 0;
	pAd->MlmeAux.NewExtChannelOffset = 0xff;

	
	if (pAd->StaCfg.WpaState != SS_NOTUSE)
	{
		pAd->StaCfg.WpaState = SS_START;
		
		NdisZeroMemory(pAd->StaCfg.ReplayCounter, 8);
	}


	
	
	
	
	if ((IsReqFromAP) && (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA))
	{
		
		RTMPWPARemoveAllKeys(pAd);
	}

	

	
	
	if (pAd->StaCfg.WpaSupplicantUP &&
		(pAd->StaCfg.WepStatus == Ndis802_11WEPEnabled) &&
		(pAd->StaCfg.IEEE8021X == FALSE))
	{
		pAd->StaCfg.PortSecured = WPA_802_1X_PORT_SECURED;
	}
	else
	{
		pAd->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		pAd->StaCfg.PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
	}

	NdisAcquireSpinLock(&pAd->MacTabLock);
	pAd->MacTab.Content[BSSID_WCID].PortSecured = pAd->StaCfg.PortSecured;
	NdisReleaseSpinLock(&pAd->MacTabLock);

	pAd->StaCfg.MicErrCnt = 0;

	
	pAd->StaCfg.bCkipOn = FALSE;
	pAd->StaCfg.CCXEnable = FALSE;

    pAd->IndicateMediaState = NdisMediaStateDisconnected;
	
	pAd->ExtraInfo = GENERAL_LINK_DOWN;

#ifdef RT2860
    pAd->StaCfg.AdhocBOnlyJoined = FALSE;
	pAd->StaCfg.AdhocBGJoined = FALSE;
	pAd->StaCfg.Adhoc20NJoined = FALSE;
#endif
    pAd->StaActive.SupportedPhyInfo.bHtEnable = FALSE;

	
	NdisZeroMemory(pAd->StaCfg.AironetIPAddress, 4);
#ifdef RT2870
	pAd->bUsbTxBulkAggre = FALSE;
#endif 

	
	NdisZeroMemory(&pAd->StaCfg.AssocInfo, sizeof(NDIS_802_11_ASSOCIATION_INFORMATION));
	pAd->StaCfg.AssocInfo.Length = sizeof(NDIS_802_11_ASSOCIATION_INFORMATION);
	pAd->StaCfg.ReqVarIELen = 0;
	pAd->StaCfg.ResVarIELen = 0;

	
	
	
	pAd->StaCfg.RssiSample.AvgRssi0 = 0;
	pAd->StaCfg.RssiSample.AvgRssi0X8 = 0;
	pAd->StaCfg.RssiSample.AvgRssi1 = 0;
	pAd->StaCfg.RssiSample.AvgRssi1X8 = 0;
	pAd->StaCfg.RssiSample.AvgRssi2 = 0;
	pAd->StaCfg.RssiSample.AvgRssi2X8 = 0;

	
	pAd->CommonCfg.MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	pAd->CommonCfg.RtsRate = pAd->CommonCfg.BasicMlmeRate;

	
	
	
	if (pAd->CommonCfg.BBPCurrentBW == BW_40)
	{
		pAd->CommonCfg.BBPCurrentBW = BW_20;
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &ByteValue);
		ByteValue &= (~0x18);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, ByteValue);
	}

	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &ByteValue);
	ByteValue &= (~0x18);
	if (pAd->Antenna.field.TxPath == 2)
	{
		ByteValue |= 0x10;
	}
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, ByteValue);

	RTMPSetPiggyBack(pAd,FALSE);
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_PIGGYBACK_INUSED);

	pAd->CommonCfg.BACapability.word = pAd->CommonCfg.REGBACapability.word;

	
	AsicUpdateProtect(pAd, 0, (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), TRUE, FALSE);
	AsicDisableRDG(pAd);
	pAd->CommonCfg.IOTestParm.bCurrentAtheros = FALSE;
	pAd->CommonCfg.IOTestParm.bNowAtherosBurstOn = FALSE;

	RTMP_IO_WRITE32(pAd, MAX_LEN_CFG, 0x1fff);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);

#ifdef RT2860
	
	RTMP_SET_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP);
#endif

	{
		union iwreq_data    wrqu;
		memset(wrqu.ap_addr.sa_data, 0, MAC_ADDR_LEN);
		wireless_send_event(pAd->net_dev, SIOCGIWAP, &wrqu, NULL);
	}

	if (IS_RT3090(pAd))
	{
		UINT32				macdata;
		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &ByteValue);
		ByteValue &= ~(0x04);	
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, ByteValue);

		
		RTMP_IO_READ32(pAd, 0x1210, &macdata);
		macdata &= ~(0x09);	
		RTMP_IO_WRITE32(pAd, 0x1210, macdata);
	}
}


VOID IterateOnBssTab(
	IN PRTMP_ADAPTER pAd)
{
	MLME_START_REQ_STRUCT   StartReq;
	MLME_JOIN_REQ_STRUCT    JoinReq;
	ULONG                   BssIdx;

	
	pAd->StaCfg.WepStatus   = pAd->StaCfg.OrigWepStatus;
	pAd->StaCfg.PairCipher  = pAd->StaCfg.OrigWepStatus;
	pAd->StaCfg.GroupCipher = pAd->StaCfg.OrigWepStatus;

	BssIdx = pAd->MlmeAux.BssIdx;
	if (BssIdx < pAd->MlmeAux.SsidBssTab.BssNr)
	{
		
		
		
		if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK))
		{
			pAd->StaCfg.GroupCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA.GroupCipher;

			if (pAd->StaCfg.WepStatus == pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA.PairCipher)
				pAd->StaCfg.PairCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA.PairCipher;
			else if (pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA.PairCipherAux != Ndis802_11WEPDisabled)
				pAd->StaCfg.PairCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA.PairCipherAux;
			else	
				pAd->StaCfg.PairCipher = Ndis802_11Encryption2Enabled;
		}
		else if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
		{
			pAd->StaCfg.GroupCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.GroupCipher;

			if (pAd->StaCfg.WepStatus == pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.PairCipher)
				pAd->StaCfg.PairCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.PairCipher;
			else if (pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.PairCipherAux != Ndis802_11WEPDisabled)
				pAd->StaCfg.PairCipher = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.PairCipherAux;
			else	
				pAd->StaCfg.PairCipher = Ndis802_11Encryption2Enabled;

			
			pAd->StaCfg.RsnCapability = pAd->MlmeAux.SsidBssTab.BssEntry[BssIdx].WPA2.RsnCapability;
		}

		
		pAd->StaCfg.bMixCipher = (pAd->StaCfg.PairCipher == pAd->StaCfg.GroupCipher) ? FALSE : TRUE;
		if (pAd->StaCfg.bMixCipher == TRUE)
		{
			
			RTMPMakeRSNIE(pAd, pAd->StaCfg.AuthMode, pAd->StaCfg.WepStatus, 0);
		}

		DBGPRINT(RT_DEBUG_TRACE, ("CNTL - iterate BSS %ld of %d\n", BssIdx, pAd->MlmeAux.SsidBssTab.BssNr));
		JoinParmFill(pAd, &JoinReq, BssIdx);
		MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_JOIN_REQ, sizeof(MLME_JOIN_REQ_STRUCT),
					&JoinReq);
		pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_JOIN;
	}
	else if (pAd->StaCfg.BssType == BSS_ADHOC)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CNTL - All BSS fail; start a new ADHOC (Ssid=%s)...\n",pAd->MlmeAux.Ssid));
		StartParmFill(pAd, &StartReq, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
		MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_START_REQ, sizeof(MLME_START_REQ_STRUCT), &StartReq);
		pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_START;
	}
	else 
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CNTL - All roaming failed, stay @ ch #%d\n", pAd->CommonCfg.Channel));
		AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
		AsicLockChannel(pAd, pAd->CommonCfg.Channel);
		pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;
	}
}



VOID IterateOnBssTab2(
	IN PRTMP_ADAPTER pAd)
{
	MLME_REASSOC_REQ_STRUCT ReassocReq;
	ULONG                   BssIdx;
	BSS_ENTRY               *pBss;

	BssIdx = pAd->MlmeAux.RoamIdx;
	pBss = &pAd->MlmeAux.RoamTab.BssEntry[BssIdx];

	if (BssIdx < pAd->MlmeAux.RoamTab.BssNr)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CNTL - iterate BSS %ld of %d\n", BssIdx, pAd->MlmeAux.RoamTab.BssNr));

		AsicSwitchChannel(pAd, pBss->Channel, FALSE);
		AsicLockChannel(pAd, pBss->Channel);

		
		AssocParmFill(pAd, &ReassocReq, pBss->Bssid, pBss->CapabilityInfo,
					  ASSOC_TIMEOUT, pAd->StaCfg.DefaultListenCount);
		MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_MLME_REASSOC_REQ,
					sizeof(MLME_REASSOC_REQ_STRUCT), &ReassocReq);

		pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_REASSOC;
	}
	else 
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CNTL - All fast roaming failed, back to ch #%d\n",pAd->CommonCfg.Channel));
		AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
		AsicLockChannel(pAd, pAd->CommonCfg.Channel);
		pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;
	}
}


VOID JoinParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_JOIN_REQ_STRUCT *JoinReq,
	IN ULONG BssIdx)
{
	JoinReq->BssIdx = BssIdx;
}


VOID ScanParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_SCAN_REQ_STRUCT *ScanReq,
	IN CHAR Ssid[],
	IN UCHAR SsidLen,
	IN UCHAR BssType,
	IN UCHAR ScanType)
{
    NdisZeroMemory(ScanReq->Ssid, MAX_LEN_OF_SSID);
	ScanReq->SsidLen = SsidLen;
	NdisMoveMemory(ScanReq->Ssid, Ssid, SsidLen);
	ScanReq->BssType = BssType;
	ScanReq->ScanType = ScanType;
}


VOID StartParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_START_REQ_STRUCT *StartReq,
	IN CHAR Ssid[],
	IN UCHAR SsidLen)
{
	ASSERT(SsidLen <= MAX_LEN_OF_SSID);
	NdisMoveMemory(StartReq->Ssid, Ssid, SsidLen);
	StartReq->SsidLen = SsidLen;
}


VOID AuthParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_AUTH_REQ_STRUCT *AuthReq,
	IN PUCHAR pAddr,
	IN USHORT Alg)
{
	COPY_MAC_ADDR(AuthReq->Addr, pAddr);
	AuthReq->Alg = Alg;
	AuthReq->Timeout = AUTH_TIMEOUT;
}




#ifdef RT2870

VOID MlmeCntlConfirm(
	IN PRTMP_ADAPTER pAd,
	IN ULONG MsgType,
	IN USHORT Msg)
{
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MsgType, sizeof(USHORT), &Msg);
}
#endif

VOID ComposePsPoll(
	IN PRTMP_ADAPTER pAd)
{
#ifdef RT2870
	PTXINFO_STRUC		pTxInfo;
	PTXWI_STRUC		pTxWI;

	DBGPRINT(RT_DEBUG_TRACE, ("ComposePsPoll\n"));
#endif
	NdisZeroMemory(&pAd->PsPollFrame, sizeof(PSPOLL_FRAME));

#ifdef RT2870
	pAd->PsPollFrame.FC.PwrMgmt = 0;
#endif
	pAd->PsPollFrame.FC.Type = BTYPE_CNTL;
	pAd->PsPollFrame.FC.SubType = SUBTYPE_PS_POLL;
	pAd->PsPollFrame.Aid = pAd->StaActive.Aid | 0xC000;
	COPY_MAC_ADDR(pAd->PsPollFrame.Bssid, pAd->CommonCfg.Bssid);
	COPY_MAC_ADDR(pAd->PsPollFrame.Ta, pAd->CurrentAddress);

#ifdef RT2870
	RTMPZeroMemory(&pAd->PsPollContext.TransferBuffer->field.WirelessPacket[0], 100);
	pTxInfo = (PTXINFO_STRUC)&pAd->PsPollContext.TransferBuffer->field.WirelessPacket[0];
	RTMPWriteTxInfo(pAd, pTxInfo, (USHORT)(sizeof(PSPOLL_FRAME)+TXWI_SIZE), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);
	pTxWI = (PTXWI_STRUC)&pAd->PsPollContext.TransferBuffer->field.WirelessPacket[TXINFO_SIZE];
	RTMPWriteTxWI(pAd, pTxWI, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, 0, BSSID_WCID, (sizeof(PSPOLL_FRAME)),
		0,  0, (UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS, IFS_BACKOFF, FALSE, &pAd->CommonCfg.MlmeTransmit);
	RTMPMoveMemory(&pAd->PsPollContext.TransferBuffer->field.WirelessPacket[TXWI_SIZE+TXINFO_SIZE], &pAd->PsPollFrame, sizeof(PSPOLL_FRAME));
	
	pAd->PsPollContext.BulkOutSize =  TXINFO_SIZE + TXWI_SIZE + sizeof(PSPOLL_FRAME) + 4;
#endif
}


VOID ComposeNullFrame(
	IN PRTMP_ADAPTER pAd)
{
#ifdef RT2870
	PTXINFO_STRUC		pTxInfo;
	PTXWI_STRUC		pTxWI;
#endif

	NdisZeroMemory(&pAd->NullFrame, sizeof(HEADER_802_11));
	pAd->NullFrame.FC.Type = BTYPE_DATA;
	pAd->NullFrame.FC.SubType = SUBTYPE_NULL_FUNC;
	pAd->NullFrame.FC.ToDs = 1;
	COPY_MAC_ADDR(pAd->NullFrame.Addr1, pAd->CommonCfg.Bssid);
	COPY_MAC_ADDR(pAd->NullFrame.Addr2, pAd->CurrentAddress);
	COPY_MAC_ADDR(pAd->NullFrame.Addr3, pAd->CommonCfg.Bssid);
#ifdef RT2870
	RTMPZeroMemory(&pAd->NullContext.TransferBuffer->field.WirelessPacket[0], 100);
	pTxInfo = (PTXINFO_STRUC)&pAd->NullContext.TransferBuffer->field.WirelessPacket[0];
	RTMPWriteTxInfo(pAd, pTxInfo, (USHORT)(sizeof(HEADER_802_11)+TXWI_SIZE), TRUE, EpToQueue[MGMTPIPEIDX], FALSE,  FALSE);
	pTxWI = (PTXWI_STRUC)&pAd->NullContext.TransferBuffer->field.WirelessPacket[TXINFO_SIZE];
	RTMPWriteTxWI(pAd, pTxWI, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, 0, BSSID_WCID, (sizeof(HEADER_802_11)),
		0, 0, (UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS, IFS_BACKOFF, FALSE, &pAd->CommonCfg.MlmeTransmit);
	RTMPMoveMemory(&pAd->NullContext.TransferBuffer->field.WirelessPacket[TXWI_SIZE+TXINFO_SIZE], &pAd->NullFrame, sizeof(HEADER_802_11));
	pAd->NullContext.BulkOutSize =  TXINFO_SIZE + TXWI_SIZE + sizeof(pAd->NullFrame) + 4;
#endif
}





ULONG MakeIbssBeacon(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR         DsLen = 1, IbssLen = 2;
	UCHAR         LocalErpIe[3] = {IE_ERP, 1, 0x04};
	HEADER_802_11 BcnHdr;
	USHORT        CapabilityInfo;
	LARGE_INTEGER FakeTimestamp;
	ULONG         FrameLen = 0;
	PTXWI_STRUC	  pTxWI = &pAd->BeaconTxWI;
	CHAR         *pBeaconFrame = pAd->BeaconBuf;
	BOOLEAN       Privacy;
	UCHAR         SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR         SupRateLen = 0;
	UCHAR         ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR         ExtRateLen = 0;
	UCHAR         RSNIe = IE_WPA;

	if ((pAd->CommonCfg.PhyMode == PHY_11B) && (pAd->CommonCfg.Channel <= 14))
	{
		SupRate[0] = 0x82; 
		SupRate[1] = 0x84; 
		SupRate[2] = 0x8b; 
		SupRate[3] = 0x96; 
		SupRateLen = 4;
		ExtRateLen = 0;
	}
	else if (pAd->CommonCfg.Channel > 14)
	{
		SupRate[0]  = 0x8C;    
		SupRate[1]  = 0x12;    
		SupRate[2]  = 0x98;    
		SupRate[3]  = 0x24;    
		SupRate[4]  = 0xb0;    
		SupRate[5]  = 0x48;    
		SupRate[6]  = 0x60;    
		SupRate[7]  = 0x6c;    
		SupRateLen  = 8;
		ExtRateLen  = 0;

		
		
		
		pAd->CommonCfg.MlmeRate = RATE_6;
		pAd->CommonCfg.RtsRate = RATE_6;
		pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
		pAd->CommonCfg.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
		pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MODE = MODE_OFDM;
		pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
	}
	else
	{
		SupRate[0] = 0x82; 
		SupRate[1] = 0x84; 
		SupRate[2] = 0x8b; 
		SupRate[3] = 0x96; 
		SupRateLen = 4;

		ExtRate[0]  = 0x0C;    
		ExtRate[1]  = 0x12;    
		ExtRate[2]  = 0x18;    
		ExtRate[3]  = 0x24;    
		ExtRate[4]  = 0x30;    
		ExtRate[5]  = 0x48;    
		ExtRate[6]  = 0x60;    
		ExtRate[7]  = 0x6c;    
		ExtRateLen  = 8;
	}

	pAd->StaActive.SupRateLen = SupRateLen;
	NdisMoveMemory(pAd->StaActive.SupRate, SupRate, SupRateLen);
	pAd->StaActive.ExtRateLen = ExtRateLen;
	NdisMoveMemory(pAd->StaActive.ExtRate, ExtRate, ExtRateLen);

	
	MgtMacHeaderInit(pAd, &BcnHdr, SUBTYPE_BEACON, 0, BROADCAST_ADDR, pAd->CommonCfg.Bssid);
	Privacy = (pAd->StaCfg.WepStatus == Ndis802_11Encryption1Enabled) ||
			  (pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) ||
			  (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled);
	CapabilityInfo = CAP_GENERATE(0, 1, Privacy, (pAd->CommonCfg.TxPreamble == Rt802_11PreambleShort), 0, 0);

	MakeOutgoingFrame(pBeaconFrame,                &FrameLen,
					  sizeof(HEADER_802_11),           &BcnHdr,
					  TIMESTAMP_LEN,                   &FakeTimestamp,
					  2,                               &pAd->CommonCfg.BeaconPeriod,
					  2,                               &CapabilityInfo,
					  1,                               &SsidIe,
					  1,                               &pAd->CommonCfg.SsidLen,
					  pAd->CommonCfg.SsidLen,          pAd->CommonCfg.Ssid,
					  1,                               &SupRateIe,
					  1,                               &SupRateLen,
					  SupRateLen,                      SupRate,
					  1,                               &DsIe,
					  1,                               &DsLen,
					  1,                               &pAd->CommonCfg.Channel,
					  1,                               &IbssIe,
					  1,                               &IbssLen,
					  2,                               &pAd->StaActive.AtimWin,
					  END_OF_ARGS);

	
	if (ExtRateLen)
	{
		ULONG	tmp;

		MakeOutgoingFrame(pBeaconFrame + FrameLen,         &tmp,
						  3,                               LocalErpIe,
						  1,                               &ExtRateIe,
						  1,                               &ExtRateLen,
						  ExtRateLen,                      ExtRate,
						  END_OF_ARGS);
		FrameLen += tmp;
	}

	
	if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPANone)
	{
		ULONG tmp;
        RTMPMakeRSNIE(pAd, pAd->StaCfg.AuthMode, pAd->StaCfg.WepStatus, BSS0);

		MakeOutgoingFrame(pBeaconFrame + FrameLen,        	&tmp,
						  1,                              	&RSNIe,
						  1,                            	&pAd->StaCfg.RSNIE_Len,
						  pAd->StaCfg.RSNIE_Len,      		pAd->StaCfg.RSN_IE,
						  END_OF_ARGS);
		FrameLen += tmp;
	}

	if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
	{
		ULONG TmpLen;
		UCHAR HtLen, HtLen1;

		
		HtLen = sizeof(pAd->CommonCfg.HtCapability);
		HtLen1 = sizeof(pAd->CommonCfg.AddHTInfo);

		MakeOutgoingFrame(pBeaconFrame+FrameLen,	&TmpLen,
						  1,						&HtCapIe,
						  1,						&HtLen,
						  HtLen,					&pAd->CommonCfg.HtCapability,
						  1,						&AddHtInfoIe,
						  1,						&HtLen1,
						  HtLen1,					&pAd->CommonCfg.AddHTInfo,
						  END_OF_ARGS);

		FrameLen += TmpLen;
	}

	
    if (pAd->CommonCfg.Channel > 14)
    {
	RTMPWriteTxWI(pAd, pTxWI, FALSE, FALSE,  TRUE, FALSE, FALSE, TRUE, 0, 0xff, FrameLen,
		PID_MGMT, PID_BEACON, RATE_1, IFS_HTTXOP, FALSE, &pAd->CommonCfg.MlmeTransmit);
    }
    else
    {
        
		HTTRANSMIT_SETTING Transmit;
        Transmit.word = 0;
        RTMPWriteTxWI(pAd, pTxWI, FALSE, FALSE,  TRUE, FALSE, FALSE, TRUE, 0, 0xff, FrameLen,
    		PID_MGMT, PID_BEACON, RATE_1, IFS_HTTXOP, FALSE, &Transmit);
    }

    DBGPRINT(RT_DEBUG_TRACE, ("MakeIbssBeacon (len=%ld), SupRateLen=%d, ExtRateLen=%d, Channel=%d, PhyMode=%d\n",
					FrameLen, SupRateLen, ExtRateLen, pAd->CommonCfg.Channel, pAd->CommonCfg.PhyMode));
	return FrameLen;
}


