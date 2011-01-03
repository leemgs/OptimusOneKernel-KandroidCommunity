
#include "../rt_config.h"


VOID	AironetStateMachineInit(
	IN	PRTMP_ADAPTER		pAd,
	IN	STATE_MACHINE		*S,
	OUT	STATE_MACHINE_FUNC	Trans[])
{
	StateMachineInit(S,	Trans, MAX_AIRONET_STATE, MAX_AIRONET_MSG, (STATE_MACHINE_FUNC)Drop, AIRONET_IDLE, AIRONET_MACHINE_BASE);
	StateMachineSetAction(S, AIRONET_IDLE, MT2_AIRONET_MSG, (STATE_MACHINE_FUNC)AironetMsgAction);
	StateMachineSetAction(S, AIRONET_IDLE, MT2_AIRONET_SCAN_REQ, (STATE_MACHINE_FUNC)AironetRequestAction);
	StateMachineSetAction(S, AIRONET_SCANNING, MT2_AIRONET_SCAN_DONE, (STATE_MACHINE_FUNC)AironetReportAction);
}


VOID	AironetMsgAction(
	IN	PRTMP_ADAPTER	pAd,
	IN	MLME_QUEUE_ELEM	*Elem)
{
	USHORT							Length;
	UCHAR							Index, i;
	PUCHAR							pData;
	PAIRONET_RM_REQUEST_FRAME		pRMReq;
	PRM_REQUEST_ACTION				pReqElem;

	DBGPRINT(RT_DEBUG_TRACE, ("-----> AironetMsgAction\n"));

	
	pRMReq = (PAIRONET_RM_REQUEST_FRAME) &Elem->Msg[LENGTH_802_11];
	pData  = (PUCHAR) &Elem->Msg[LENGTH_802_11];

	
	Length = be2cpu16(pRMReq->IAPP.Length);

	
	if (pAd->StaCfg.CCXEnable != TRUE)
		return;

	
	if (pAd->StaCfg.CCXControl.field.RMEnable != 1)
		return;

	
	DBGPRINT(RT_DEBUG_TRACE, ("IAPP ID & Length %d\n", Length));
	DBGPRINT(RT_DEBUG_TRACE, ("IAPP Type %x\n", pRMReq->IAPP.Type));
	DBGPRINT(RT_DEBUG_TRACE, ("IAPP SubType %x\n", pRMReq->IAPP.SubType));
	DBGPRINT(RT_DEBUG_TRACE, ("IAPP Dialog Token %x\n", pRMReq->IAPP.Token));
	DBGPRINT(RT_DEBUG_TRACE, ("IAPP Activation Delay %x\n", pRMReq->Delay));
	DBGPRINT(RT_DEBUG_TRACE, ("IAPP Measurement Offset %x\n", pRMReq->Offset));

	
	if (pRMReq->IAPP.Type != AIRONET_IAPP_TYPE)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Wrong IAPP type for Cisco Aironet extension\n"));
		return;
	}

	
	
	if (pRMReq->IAPP.SubType != AIRONET_IAPP_SUBTYPE_REQUEST)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Wrong IAPP subtype for Cisco Aironet extension\n"));
		return;
	}

	
	if (! MAC_ADDR_EQUAL(pRMReq->IAPP.DA, ZERO_MAC_ADDR))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Wrong IAPP DA for Cisco Aironet extension, it's not Zero\n"));
		return;
	}

	if (! MAC_ADDR_EQUAL(pRMReq->IAPP.SA, ZERO_MAC_ADDR))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Wrong IAPP SA for Cisco Aironet extension, it's not Zero\n"));
		return;
	}

	
	NdisZeroMemory(pAd->StaCfg.FrameReportBuf, 2048);
	NdisZeroMemory(pAd->StaCfg.BssReportOffset, sizeof(USHORT) * MAX_LEN_OF_BSS_TABLE);
	NdisZeroMemory(pAd->StaCfg.MeasurementRequest, sizeof(RM_REQUEST_ACTION) * 4);

	
	pAd->StaCfg.FrameReportLen   = LENGTH_802_11 + sizeof(AIRONET_IAPP_HEADER);
	DBGPRINT(RT_DEBUG_TRACE, ("FR len = %d\n", pAd->StaCfg.FrameReportLen));
	pAd->StaCfg.LastBssIndex     = 0xff;
	pAd->StaCfg.RMReqCnt         = 0;
	pAd->StaCfg.ParallelReq      = FALSE;
	pAd->StaCfg.ParallelDuration = 0;
	pAd->StaCfg.ParallelChannel  = 0;
	pAd->StaCfg.IAPPToken        = pRMReq->IAPP.Token;
	pAd->StaCfg.CurrentRMReqIdx  = 0;
	pAd->StaCfg.CLBusyBytes      = 0;
	
	for (i = 0; i < 8; i++)
		pAd->StaCfg.RPIDensity[i] = 0;

	Index = 0;

	
	pAd->StaCfg.IAPPToken = pRMReq->IAPP.Token;

	

	
	pData += sizeof(AIRONET_RM_REQUEST_FRAME);
	
	Length -= (sizeof(AIRONET_RM_REQUEST_FRAME) - LENGTH_802_1_H);

	
	
	while (Length > 0)
	{
		pReqElem = (PRM_REQUEST_ACTION) pData;
		switch (pReqElem->ReqElem.Eid)
		{
			case IE_MEASUREMENT_REQUEST:
				
				
				
				

				
				
				
				
				
				
				
				switch (pReqElem->ReqElem.Type)
				{
					case MSRN_TYPE_CHANNEL_LOAD_REQ:
					case MSRN_TYPE_NOISE_HIST_REQ:
					case MSRN_TYPE_BEACON_REQ:
						
						if (pAd->StaCfg.CCXControl.field.DCRMEnable == 0)
						{
							
							if (pReqElem->Measurement.Channel != pAd->CommonCfg.Channel)
								break;
						}
						else
						{
							
							if (pReqElem->Measurement.Channel != pAd->CommonCfg.Channel)
								if (pReqElem->Measurement.Duration > pAd->StaCfg.CCXControl.field.TuLimit)
									break;
						}

						
						NdisMoveMemory(&pAd->StaCfg.MeasurementRequest[Index], pReqElem, sizeof(RM_REQUEST_ACTION));
						Index += 1;
						break;

					case MSRN_TYPE_FRAME_REQ:
						
						
						break;

					default:
						break;
				}

				
				pData  += sizeof(RM_REQUEST_ACTION);
				Length -= sizeof(RM_REQUEST_ACTION);
				break;

			
			case IE_MEASUREMENT_REPORT:
			case IE_AP_TX_POWER:
			case IE_MEASUREMENT_CAPABILITY:
			default:
				return;
		}
	}

	
	pAd->StaCfg.RMReqCnt = Index;

	if (Index)
	{
		MlmeEnqueue(pAd, AIRONET_STATE_MACHINE, MT2_AIRONET_SCAN_REQ, 0, NULL);
		RT28XX_MLME_HANDLER(pAd);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<----- AironetMsgAction\n"));
}


VOID	AironetRequestAction(
	IN	PRTMP_ADAPTER	pAd,
	IN	MLME_QUEUE_ELEM	*Elem)
{
	PRM_REQUEST_ACTION	pReq;

	
	pReq = (PRM_REQUEST_ACTION) &pAd->StaCfg.MeasurementRequest[pAd->StaCfg.CurrentRMReqIdx];

	
	if (pReq->ReqElem.Type == MSRN_TYPE_CHANNEL_LOAD_REQ)
		
		ChannelLoadRequestAction(pAd, pAd->StaCfg.CurrentRMReqIdx);
	else if (pReq->ReqElem.Type == MSRN_TYPE_NOISE_HIST_REQ)
		
		NoiseHistRequestAction(pAd, pAd->StaCfg.CurrentRMReqIdx);
	else if (pReq->ReqElem.Type == MSRN_TYPE_BEACON_REQ)
		
		BeaconRequestAction(pAd, pAd->StaCfg.CurrentRMReqIdx);
	else
		
		return;

	
	if ((pAd->StaCfg.CurrentRMReqIdx + 1) < pAd->StaCfg.RMReqCnt)
	{
		pReq = (PRM_REQUEST_ACTION) &pAd->StaCfg.MeasurementRequest[pAd->StaCfg.CurrentRMReqIdx + 1];
		
		if ((pReq->ReqElem.Mode & 0x01) && (pReq->Measurement.Channel == pAd->StaCfg.CCXScanChannel))
		{
			
			pAd->StaCfg.ParallelReq = TRUE;
			pAd->StaCfg.CCXScanTime = ((pReq->Measurement.Duration > pAd->StaCfg.CCXScanTime) ?
			(pReq->Measurement.Duration) : (pAd->StaCfg.CCXScanTime));
		}
	}

	
	RT28XX_MLME_HANDLER(pAd);

}



VOID	ChannelLoadRequestAction(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Index)
{
	PRM_REQUEST_ACTION				pReq;
	MLME_SCAN_REQ_STRUCT			ScanReq;
	UCHAR							ZeroSsid[32];
	NDIS_STATUS						NStatus;
	PUCHAR							pOutBuffer = NULL;
	PHEADER_802_11					pNullFrame;

	DBGPRINT(RT_DEBUG_TRACE, ("ChannelLoadRequestAction ----->\n"));

	pReq = (PRM_REQUEST_ACTION) &pAd->StaCfg.MeasurementRequest[Index];
	NdisZeroMemory(ZeroSsid, 32);

	
	
	
	
	
	
	

	
	if ((pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE) && (Index == 0))
		return;

	
	ScanParmFill(pAd, &ScanReq, ZeroSsid, 0, BSS_ANY, SCAN_CISCO_CHANNEL_LOAD);
	MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq);
	pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;

	
	BssTableInit(&pAd->StaCfg.CCXBssTab);
	pAd->StaCfg.ScanCnt        = 0;
	pAd->StaCfg.CCXScanChannel = pReq->Measurement.Channel;
	pAd->StaCfg.CCXScanTime    = pReq->Measurement.Duration;

	DBGPRINT(RT_DEBUG_TRACE, ("Duration %d, Channel %d!\n", pReq->Measurement.Duration, pReq->Measurement.Channel));

	
	if (pAd->StaCfg.CCXScanChannel != pAd->CommonCfg.Channel)
	{
		
		NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  
		if (NStatus	!= NDIS_STATUS_SUCCESS)
			return;

		pNullFrame = (PHEADER_802_11) pOutBuffer;;
		
		MgtMacHeaderInit(pAd, pNullFrame, SUBTYPE_NULL_FUNC, 1, pAd->CommonCfg.Bssid, pAd->CommonCfg.Bssid);
		pNullFrame->Duration 	= 0;
		pNullFrame->FC.Type 	= BTYPE_DATA;
		pNullFrame->FC.PwrMgmt	= PWR_SAVE;

		
		MiniportMMRequest(pAd, 0, pOutBuffer, sizeof(HEADER_802_11));
		MlmeFreeMemory(pAd, pOutBuffer);
		DBGPRINT(RT_DEBUG_TRACE, ("Send PSM Data frame for off channel RM\n"));
		RTMPusecDelay(5000);
	}

	pAd->StaCfg.CCXReqType     = MSRN_TYPE_CHANNEL_LOAD_REQ;
	pAd->StaCfg.CLBusyBytes    = 0;
	
	RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, 0x1010);

	
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_MEASUREMENT);

	pAd->Mlme.AironetMachine.CurrState = AIRONET_SCANNING;

	DBGPRINT(RT_DEBUG_TRACE, ("ChannelLoadRequestAction <-----\n"));
}


VOID	NoiseHistRequestAction(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Index)
{
	PRM_REQUEST_ACTION				pReq;
	MLME_SCAN_REQ_STRUCT			ScanReq;
	UCHAR							ZeroSsid[32], i;
	NDIS_STATUS						NStatus;
	PUCHAR							pOutBuffer = NULL;
	PHEADER_802_11					pNullFrame;

	DBGPRINT(RT_DEBUG_TRACE, ("NoiseHistRequestAction ----->\n"));

	pReq = (PRM_REQUEST_ACTION) &pAd->StaCfg.MeasurementRequest[Index];
	NdisZeroMemory(ZeroSsid, 32);

	
	
	
	
	
	
	

	
	if ((pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE) && (Index == 0))
		return;

	
	ScanParmFill(pAd, &ScanReq, ZeroSsid, 0, BSS_ANY, SCAN_CISCO_NOISE);
	MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq);
	pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;

	
	BssTableInit(&pAd->StaCfg.CCXBssTab);
	pAd->StaCfg.ScanCnt        = 0;
	pAd->StaCfg.CCXScanChannel = pReq->Measurement.Channel;
	pAd->StaCfg.CCXScanTime    = pReq->Measurement.Duration;
	pAd->StaCfg.CCXReqType     = MSRN_TYPE_NOISE_HIST_REQ;

	DBGPRINT(RT_DEBUG_TRACE, ("Duration %d, Channel %d!\n", pReq->Measurement.Duration, pReq->Measurement.Channel));

	
	if (pAd->StaCfg.CCXScanChannel != pAd->CommonCfg.Channel)
	{
		
		NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  
		if (NStatus	!= NDIS_STATUS_SUCCESS)
			return;

		pNullFrame = (PHEADER_802_11) pOutBuffer;
		
		MgtMacHeaderInit(pAd, pNullFrame, SUBTYPE_NULL_FUNC, 1, pAd->CommonCfg.Bssid, pAd->CommonCfg.Bssid);
		pNullFrame->Duration 	= 0;
		pNullFrame->FC.Type  	= BTYPE_DATA;
		pNullFrame->FC.PwrMgmt	= PWR_SAVE;

		
		MiniportMMRequest(pAd, 0, pOutBuffer, sizeof(HEADER_802_11));
		MlmeFreeMemory(pAd, pOutBuffer);
		DBGPRINT(RT_DEBUG_TRACE, ("Send PSM Data frame for off channel RM\n"));
		RTMPusecDelay(5000);
	}

	
	for (i = 0; i < 8; i++)
		pAd->StaCfg.RPIDensity[i] = 0;

	
	RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, 0x1010);

	
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_MEASUREMENT);

	pAd->Mlme.AironetMachine.CurrState = AIRONET_SCANNING;

	DBGPRINT(RT_DEBUG_TRACE, ("NoiseHistRequestAction <-----\n"));
}


VOID	BeaconRequestAction(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Index)
{
	PRM_REQUEST_ACTION				pReq;
	NDIS_STATUS						NStatus;
	PUCHAR							pOutBuffer = NULL;
	PHEADER_802_11					pNullFrame;
	MLME_SCAN_REQ_STRUCT			ScanReq;
	UCHAR							ZeroSsid[32];

	DBGPRINT(RT_DEBUG_TRACE, ("BeaconRequestAction ----->\n"));

	pReq = (PRM_REQUEST_ACTION) &pAd->StaCfg.MeasurementRequest[Index];
	NdisZeroMemory(ZeroSsid, 32);

	
	
	
	
	
	
	if (pReq->Measurement.ScanMode == MSRN_SCAN_MODE_PASSIVE)
	{
		
		DBGPRINT(RT_DEBUG_TRACE, ("Passive Scan Mode!\n"));

		
		if ((pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE) && (Index == 0))
			return;

		
		ScanParmFill(pAd, &ScanReq, ZeroSsid, 0, BSS_ANY, SCAN_CISCO_PASSIVE);
		MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq);
		pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;

		
		BssTableInit(&pAd->StaCfg.CCXBssTab);
		pAd->StaCfg.ScanCnt        = 0;
		pAd->StaCfg.CCXScanChannel = pReq->Measurement.Channel;
		pAd->StaCfg.CCXScanTime    = pReq->Measurement.Duration;
		pAd->StaCfg.CCXReqType     = MSRN_TYPE_BEACON_REQ;
		DBGPRINT(RT_DEBUG_TRACE, ("Duration %d!\n", pReq->Measurement.Duration));

		
		if (pAd->StaCfg.CCXScanChannel != pAd->CommonCfg.Channel)
		{
			
			NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  
			if (NStatus	!= NDIS_STATUS_SUCCESS)
				return;

			pNullFrame = (PHEADER_802_11) pOutBuffer;
			
			MgtMacHeaderInit(pAd, pNullFrame, SUBTYPE_NULL_FUNC, 1, pAd->CommonCfg.Bssid, pAd->CommonCfg.Bssid);
			pNullFrame->Duration 	= 0;
			pNullFrame->FC.Type     = BTYPE_DATA;
			pNullFrame->FC.PwrMgmt  = PWR_SAVE;

			
			MiniportMMRequest(pAd, 0, pOutBuffer, sizeof(HEADER_802_11));
			MlmeFreeMemory(pAd, pOutBuffer);
			DBGPRINT(RT_DEBUG_TRACE, ("Send PSM Data frame for off channel RM\n"));
			RTMPusecDelay(5000);
		}

		pAd->Mlme.AironetMachine.CurrState = AIRONET_SCANNING;
	}
	else if (pReq->Measurement.ScanMode == MSRN_SCAN_MODE_ACTIVE)
	{
		
		DBGPRINT(RT_DEBUG_TRACE, ("Active Scan Mode!\n"));

		
		if (pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE)
			return;

		
		ScanParmFill(pAd, &ScanReq, ZeroSsid, 0, BSS_ANY, SCAN_CISCO_ACTIVE);
		MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq);
		pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;

		
		BssTableInit(&pAd->StaCfg.CCXBssTab);
		pAd->StaCfg.ScanCnt        = 0;
		pAd->StaCfg.CCXScanChannel = pReq->Measurement.Channel;
		pAd->StaCfg.CCXScanTime    = pReq->Measurement.Duration;
		pAd->StaCfg.CCXReqType     = MSRN_TYPE_BEACON_REQ;
		DBGPRINT(RT_DEBUG_TRACE, ("Duration %d!\n", pReq->Measurement.Duration));

		
		if (pAd->StaCfg.CCXScanChannel != pAd->CommonCfg.Channel)
		{
			
			NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  
			if (NStatus	!= NDIS_STATUS_SUCCESS)
				return;

			pNullFrame = (PHEADER_802_11) pOutBuffer;
			
			MgtMacHeaderInit(pAd, pNullFrame, SUBTYPE_NULL_FUNC, 1, pAd->CommonCfg.Bssid, pAd->CommonCfg.Bssid);
			pNullFrame->Duration 	= 0;
			pNullFrame->FC.Type     = BTYPE_DATA;
			pNullFrame->FC.PwrMgmt  = PWR_SAVE;

			
			MiniportMMRequest(pAd, 0, pOutBuffer, sizeof(HEADER_802_11));
			MlmeFreeMemory(pAd, pOutBuffer);
			DBGPRINT(RT_DEBUG_TRACE, ("Send PSM Data frame for off channel RM\n"));
			RTMPusecDelay(5000);
		}

		pAd->Mlme.AironetMachine.CurrState = AIRONET_SCANNING;
	}
	else if (pReq->Measurement.ScanMode == MSRN_SCAN_MODE_BEACON_TABLE)
	{
		
		DBGPRINT(RT_DEBUG_TRACE, ("Beacon Report Mode!\n"));

		
		NdisMoveMemory(&pAd->StaCfg.CCXBssTab, &pAd->ScanTab, sizeof(BSS_TABLE));

		
		AironetCreateBeaconReportFromBssTable(pAd);

		
		pAd->Mlme.AironetMachine.CurrState = AIRONET_SCANNING;

		
		
		MlmeEnqueue(pAd, AIRONET_STATE_MACHINE, MT2_AIRONET_SCAN_DONE, 0, NULL);
	}
	else
	{
		
		DBGPRINT(RT_DEBUG_TRACE, ("Wrong Scan Mode!\n"));
	}

	DBGPRINT(RT_DEBUG_TRACE, ("BeaconRequestAction <-----\n"));
}


VOID	AironetReportAction(
	IN	PRTMP_ADAPTER	pAd,
	IN	MLME_QUEUE_ELEM	*Elem)
{
	PRM_REQUEST_ACTION	pReq;
	ULONG				Now32;

    NdisGetSystemUpTime(&Now32);
	pAd->StaCfg.LastBeaconRxTime = Now32;

	pReq = (PRM_REQUEST_ACTION) &pAd->StaCfg.MeasurementRequest[pAd->StaCfg.CurrentRMReqIdx];

	DBGPRINT(RT_DEBUG_TRACE, ("AironetReportAction ----->\n"));

	
	if (pReq->ReqElem.Type == MSRN_TYPE_CHANNEL_LOAD_REQ)
		
		ChannelLoadReportAction(pAd, pAd->StaCfg.CurrentRMReqIdx);
	else if (pReq->ReqElem.Type == MSRN_TYPE_NOISE_HIST_REQ)
		
		NoiseHistReportAction(pAd, pAd->StaCfg.CurrentRMReqIdx);
	else if (pReq->ReqElem.Type == MSRN_TYPE_BEACON_REQ)
		
		BeaconReportAction(pAd, pAd->StaCfg.CurrentRMReqIdx);
	else
		
		;

	
	pAd->StaCfg.CurrentRMReqIdx++;

	
	if (pAd->StaCfg.ParallelReq == TRUE)
	{
		pReq = (PRM_REQUEST_ACTION) &pAd->StaCfg.MeasurementRequest[pAd->StaCfg.CurrentRMReqIdx];

		
		if (pReq->ReqElem.Type == MSRN_TYPE_CHANNEL_LOAD_REQ)
			
			ChannelLoadReportAction(pAd, pAd->StaCfg.CurrentRMReqIdx);
		else if (pReq->ReqElem.Type == MSRN_TYPE_NOISE_HIST_REQ)
			
			NoiseHistReportAction(pAd, pAd->StaCfg.CurrentRMReqIdx);

		pAd->StaCfg.ParallelReq = FALSE;
		pAd->StaCfg.CurrentRMReqIdx++;
	}

	if (pAd->StaCfg.CurrentRMReqIdx >= pAd->StaCfg.RMReqCnt)
	{
		
		AironetFinalReportAction(pAd);
		pAd->Mlme.AironetMachine.CurrState = AIRONET_IDLE;
	}
	else
	{
		pReq = (PRM_REQUEST_ACTION) &pAd->StaCfg.MeasurementRequest[pAd->StaCfg.CurrentRMReqIdx];

		if (pReq->Measurement.Channel != pAd->CommonCfg.Channel)
		{
			RTMPusecDelay(100000);
		}

		
		MlmeEnqueue(pAd, AIRONET_STATE_MACHINE, MT2_AIRONET_SCAN_REQ, 0, NULL);
		RT28XX_MLME_HANDLER(pAd);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("AironetReportAction <-----\n"));
}


VOID	AironetFinalReportAction(
	IN	PRTMP_ADAPTER	pAd)
{
	PUCHAR					pDest;
	PAIRONET_IAPP_HEADER	pIAPP;
	PHEADER_802_11			pHeader;
	UCHAR					AckRate = RATE_2;
	USHORT					AckDuration = 0;
	NDIS_STATUS				NStatus;
	PUCHAR					pOutBuffer = NULL;
	ULONG					FrameLen = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("AironetFinalReportAction ----->\n"));

	
	pDest = &pAd->StaCfg.FrameReportBuf[LENGTH_802_11];

	
	pIAPP = (PAIRONET_IAPP_HEADER) pDest;

	
	NdisMoveMemory(pIAPP->CiscoSnapHeader, SNAP_AIRONET, LENGTH_802_1_H);

	
	pIAPP->Length  = cpu2be16(pAd->StaCfg.FrameReportLen - LENGTH_802_11 - LENGTH_802_1_H);

	
	if (be2cpu16(pIAPP->Length) <= 18)
		return;

	
	pIAPP->Type    = AIRONET_IAPP_TYPE;

	
	pIAPP->SubType = AIRONET_IAPP_SUBTYPE_REPORT;

	
	
	COPY_MAC_ADDR(pIAPP->DA, pAd->CommonCfg.Bssid);

	
	COPY_MAC_ADDR(pIAPP->SA, pAd->CurrentAddress);

	
	pIAPP->Token = pAd->StaCfg.IAPPToken;

	
	
	pHeader = (PHEADER_802_11) pAd->StaCfg.FrameReportBuf;
	pAd->Sequence ++;
	WpaMacHeaderInit(pAd, pHeader, 0, pAd->CommonCfg.Bssid);

	
	AckRate     = pAd->CommonCfg.ExpectedACKRate[pAd->CommonCfg.MlmeRate];
	AckDuration = RTMPCalcDuration(pAd, AckRate, 14);
	pHeader->Duration = pAd->CommonCfg.Dsifs + AckDuration;

	
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  
	if (NStatus	!= NDIS_STATUS_SUCCESS)
		return;

	
	MakeOutgoingFrame(pOutBuffer,                       &FrameLen,
	                  pAd->StaCfg.FrameReportLen, pAd->StaCfg.FrameReportBuf,
		              END_OF_ARGS);

	
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

	pAd->StaCfg.CCXReqType = MSRN_TYPE_UNUSED;

	DBGPRINT(RT_DEBUG_TRACE, ("AironetFinalReportAction <-----\n"));
}


VOID	ChannelLoadReportAction(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Index)
{
	PMEASUREMENT_REPORT_ELEMENT	pReport;
	PCHANNEL_LOAD_REPORT		pLoad;
	PUCHAR						pDest;
	UCHAR						CCABusyFraction;

	DBGPRINT(RT_DEBUG_TRACE, ("ChannelLoadReportAction ----->\n"));

	
	RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, STANORMAL); 

	
	pDest = (PUCHAR) &pAd->StaCfg.FrameReportBuf[pAd->StaCfg.FrameReportLen];
	pReport = (PMEASUREMENT_REPORT_ELEMENT) pDest;

	
	pReport->Eid    = IE_MEASUREMENT_REPORT;
	
	pReport->Length = 9;
	pReport->Token  = pAd->StaCfg.MeasurementRequest[Index].ReqElem.Token;
	pReport->Mode   = pAd->StaCfg.MeasurementRequest[Index].ReqElem.Mode;
	pReport->Type   = MSRN_TYPE_CHANNEL_LOAD_REQ;

	
	pDest += sizeof(MEASUREMENT_REPORT_ELEMENT);
	pLoad  = (PCHANNEL_LOAD_REPORT) pDest;
	pLoad->Channel  = pAd->StaCfg.MeasurementRequest[Index].Measurement.Channel;
	pLoad->Spare    = 0;
	pLoad->Duration = pAd->StaCfg.MeasurementRequest[Index].Measurement.Duration;

	
	
	
	
	
	CCABusyFraction = (UCHAR) (pAd->StaCfg.CLBusyBytes / pAd->StaCfg.CLFactor / pLoad->Duration);
	if (CCABusyFraction < 10)
			CCABusyFraction = (UCHAR) (pAd->StaCfg.CLBusyBytes / 3 / pLoad->Duration) + 1;

	pLoad->CCABusy = CCABusyFraction;
	DBGPRINT(RT_DEBUG_TRACE, ("CLBusyByte %ld, Duration %d, Result, %d\n", pAd->StaCfg.CLBusyBytes, pLoad->Duration, CCABusyFraction));

	DBGPRINT(RT_DEBUG_TRACE, ("FrameReportLen %d\n", pAd->StaCfg.FrameReportLen));
	pAd->StaCfg.FrameReportLen += (sizeof(MEASUREMENT_REPORT_ELEMENT) + sizeof(CHANNEL_LOAD_REPORT));
	DBGPRINT(RT_DEBUG_TRACE, ("FrameReportLen %d\n", pAd->StaCfg.FrameReportLen));

	
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_MEASUREMENT);

	
	pAd->Mlme.AironetMachine.CurrState = AIRONET_IDLE;

	DBGPRINT(RT_DEBUG_TRACE, ("ChannelLoadReportAction <-----\n"));
}


VOID	NoiseHistReportAction(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Index)
{
	PMEASUREMENT_REPORT_ELEMENT	pReport;
	PNOISE_HIST_REPORT			pNoise;
	PUCHAR						pDest;
	UCHAR						i,NoiseCnt;
	USHORT						TotalRPICnt, TotalRPISum;

	DBGPRINT(RT_DEBUG_TRACE, ("NoiseHistReportAction ----->\n"));

	
	RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, STANORMAL); 
	
	pDest = (PUCHAR) &pAd->StaCfg.FrameReportBuf[pAd->StaCfg.FrameReportLen];
	pReport = (PMEASUREMENT_REPORT_ELEMENT) pDest;

	
	pReport->Eid    = IE_MEASUREMENT_REPORT;
	
	pReport->Length = 16;
	pReport->Token  = pAd->StaCfg.MeasurementRequest[Index].ReqElem.Token;
	pReport->Mode   = pAd->StaCfg.MeasurementRequest[Index].ReqElem.Mode;
	pReport->Type   = MSRN_TYPE_NOISE_HIST_REQ;

	
	pDest += sizeof(MEASUREMENT_REPORT_ELEMENT);
	pNoise  = (PNOISE_HIST_REPORT) pDest;
	pNoise->Channel  = pAd->StaCfg.MeasurementRequest[Index].Measurement.Channel;
	pNoise->Spare    = 0;
	pNoise->Duration = pAd->StaCfg.MeasurementRequest[Index].Measurement.Duration;
	
	
	
	
	
	TotalRPICnt = pNoise->Duration * pAd->StaCfg.NHFactor / 10;
	TotalRPISum = 0;

	for (i = 0; i < 8; i++)
	{
		TotalRPISum += pAd->StaCfg.RPIDensity[i];
		DBGPRINT(RT_DEBUG_TRACE, ("RPI %d Conuts %d\n", i, pAd->StaCfg.RPIDensity[i]));
	}

	
	
	if (TotalRPISum > TotalRPICnt)
		TotalRPICnt = TotalRPISum + pNoise->Duration / 20;

	DBGPRINT(RT_DEBUG_TRACE, ("Total RPI Conuts %d\n", TotalRPICnt));

	
	NoiseCnt = 0;
	for (i = 1; i < 8; i++)
	{
		pNoise->Density[i] = (UCHAR) (pAd->StaCfg.RPIDensity[i] * 255 / TotalRPICnt);
		if ((pNoise->Density[i] == 0) && (pAd->StaCfg.RPIDensity[i] != 0))
			pNoise->Density[i]++;
		NoiseCnt += pNoise->Density[i];
		DBGPRINT(RT_DEBUG_TRACE, ("Reported RPI[%d]  = 0x%02x\n", i, pNoise->Density[i]));
	}

	
	pNoise->Density[0] = 0xff - NoiseCnt;
	DBGPRINT(RT_DEBUG_TRACE, ("Reported RPI[0]  = 0x%02x\n", pNoise->Density[0]));

	pAd->StaCfg.FrameReportLen += (sizeof(MEASUREMENT_REPORT_ELEMENT) + sizeof(NOISE_HIST_REPORT));

	
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_MEASUREMENT);

	
	pAd->Mlme.AironetMachine.CurrState = AIRONET_IDLE;

	DBGPRINT(RT_DEBUG_TRACE, ("NoiseHistReportAction <-----\n"));
}


VOID	BeaconReportAction(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Index)
{
	DBGPRINT(RT_DEBUG_TRACE, ("BeaconReportAction ----->\n"));

	
	
	

	
	pAd->StaCfg.LastBssIndex = 0xff;

	
	pAd->Mlme.AironetMachine.CurrState = AIRONET_IDLE;

	DBGPRINT(RT_DEBUG_TRACE, ("BeaconReportAction <-----\n"));
}


VOID	AironetAddBeaconReport(
	IN	PRTMP_ADAPTER		pAd,
	IN	ULONG				Index,
	IN	PMLME_QUEUE_ELEM	pElem)
{
	PVOID						pMsg;
	PUCHAR						pSrc, pDest;
	UCHAR						ReqIdx;
	ULONG						MsgLen;
	USHORT						Length;
	PFRAME_802_11				pFrame;
	PMEASUREMENT_REPORT_ELEMENT	pReport;
	PEID_STRUCT			        pEid;
	PBEACON_REPORT				pBeaconReport;
	PBSS_ENTRY					pBss;

	
	pMsg   = pElem->Msg;
	MsgLen = pElem->MsgLen;
	pFrame = (PFRAME_802_11) pMsg;
	pSrc   = pFrame->Octet;				
	pBss   = (PBSS_ENTRY) &pAd->StaCfg.CCXBssTab.BssEntry[Index];
	ReqIdx = pAd->StaCfg.CurrentRMReqIdx;

	
	if ((Index <= pAd->StaCfg.LastBssIndex) && (pAd->StaCfg.LastBssIndex != 0xff))
	{
		pDest  = (PUCHAR) &pAd->StaCfg.FrameReportBuf[pAd->StaCfg.BssReportOffset[Index]];
		
		pDest += sizeof(MEASUREMENT_REPORT_ELEMENT);
		pBeaconReport = (PBEACON_REPORT) pDest;

		
		
		pBeaconReport->RxPower += pAd->BbpRssiToDbmDelta;
		
		pBeaconReport->RxPower  = (pBeaconReport->RxPower + pBss->Rssi) / 2;
		
		pBeaconReport->RxPower -= pAd->BbpRssiToDbmDelta;

		DBGPRINT(RT_DEBUG_TRACE, ("Bssid %02x:%02x:%02x:%02x:%02x:%02x ",
			pBss->Bssid[0], pBss->Bssid[1], pBss->Bssid[2],
			pBss->Bssid[3], pBss->Bssid[4], pBss->Bssid[5]));
		DBGPRINT(RT_DEBUG_TRACE, ("RxPower[%ld] Rssi %d, Avg Rssi %d\n", Index, (pBss->Rssi - pAd->BbpRssiToDbmDelta), pBeaconReport->RxPower - 256));
		DBGPRINT(RT_DEBUG_TRACE, ("FrameReportLen = %d\n", pAd->StaCfg.BssReportOffset[Index]));

		

		
		return;
	}

	
	pAd->StaCfg.LastBssIndex = Index;

	
	
	pDest = (PUCHAR) &pAd->StaCfg.FrameReportBuf[pAd->StaCfg.FrameReportLen];

	
	pAd->StaCfg.BssReportOffset[Index] = pAd->StaCfg.FrameReportLen;

	
	pReport = (PMEASUREMENT_REPORT_ELEMENT) pDest;
	pReport->Eid = IE_MEASUREMENT_REPORT;
	pReport->Length = 0;
	pReport->Token  = pAd->StaCfg.MeasurementRequest[ReqIdx].ReqElem.Token;
	pReport->Mode   = pAd->StaCfg.MeasurementRequest[ReqIdx].ReqElem.Mode;
	pReport->Type   = MSRN_TYPE_BEACON_REQ;
	Length          = sizeof(MEASUREMENT_REPORT_ELEMENT);
	pDest          += sizeof(MEASUREMENT_REPORT_ELEMENT);

	
	pBeaconReport = (PBEACON_REPORT) pDest;
	pDest        += sizeof(BEACON_REPORT);
	Length       += sizeof(BEACON_REPORT);

	
	pBeaconReport->Channel        = pBss->Channel;
	pBeaconReport->Spare          = 0;
	pBeaconReport->Duration       = pAd->StaCfg.MeasurementRequest[ReqIdx].Measurement.Duration;
	pBeaconReport->PhyType        = ((pBss->SupRateLen+pBss->ExtRateLen > 4) ? PHY_ERP : PHY_DSS);
	
	pBeaconReport->RxPower        = pBss->Rssi - pAd->BbpRssiToDbmDelta;

	DBGPRINT(RT_DEBUG_TRACE, ("Bssid %02x:%02x:%02x:%02x:%02x:%02x ",
		pBss->Bssid[0], pBss->Bssid[1], pBss->Bssid[2],
		pBss->Bssid[3], pBss->Bssid[4], pBss->Bssid[5]));
	DBGPRINT(RT_DEBUG_TRACE, ("RxPower[%ld], Rssi %d\n", Index, pBeaconReport->RxPower - 256));
	DBGPRINT(RT_DEBUG_TRACE, ("FrameReportLen = %d\n", pAd->StaCfg.FrameReportLen));

	pBeaconReport->BeaconInterval = pBss->BeaconPeriod;
	COPY_MAC_ADDR(pBeaconReport->BSSID, pFrame->Hdr.Addr3);
	NdisMoveMemory(pBeaconReport->ParentTSF, pSrc, 4);
	NdisMoveMemory(pBeaconReport->TargetTSF, &pElem->TimeStamp.u.LowPart, 4);
	NdisMoveMemory(&pBeaconReport->TargetTSF[4], &pElem->TimeStamp.u.HighPart, 4);

	
	pSrc += (TIMESTAMP_LEN + 2);
	pBeaconReport->CapabilityInfo = *(USHORT *)pSrc;

	
	pSrc += 2;
	pEid = (PEID_STRUCT) pSrc;

	
	while (((PUCHAR) pEid + pEid->Len + 1) < ((PUCHAR) pFrame + MsgLen))
	{
		
		
		
		switch (pEid->Eid)
		{
			case IE_SSID:
			case IE_SUPP_RATES:
			case IE_FH_PARM:
			case IE_DS_PARM:
			case IE_CF_PARM:
			case IE_IBSS_PARM:
				NdisMoveMemory(pDest, pEid, pEid->Len + 2);
				pDest  += (pEid->Len + 2);
				Length += (pEid->Len + 2);
				break;

			case IE_MEASUREMENT_CAPABILITY:
				
				
				
				if (pEid->Len != 6)
					break;
				
				if (NdisEqualMemory(CISCO_OUI, (pSrc + 2), 3))
				{
					
					NdisMoveMemory(pDest, pEid, pEid->Len + 2);
					pDest  += (pEid->Len + 2);
					Length += (pEid->Len + 2);
				}
				break;

			case IE_TIM:
				if (pEid->Len > 4)
				{
					
					NdisMoveMemory(pDest, pEid, 6);
					pDest  += 6;
					Length += 6;
				}
				else
				{
					NdisMoveMemory(pDest, pEid, pEid->Len + 2);
					pDest  += (pEid->Len + 2);
					Length += (pEid->Len + 2);
				}
				break;

			default:
				break;
		}
		
		pSrc += (2 + pEid->Len);
		pEid = (PEID_STRUCT) pSrc;
	}

	
	pReport->Length = Length - 4;

	
	pAd->StaCfg.FrameReportLen += Length;
	DBGPRINT(RT_DEBUG_TRACE, ("FR len = %d\n", pAd->StaCfg.FrameReportLen));
}


VOID	AironetCreateBeaconReportFromBssTable(
	IN	PRTMP_ADAPTER		pAd)
{
	PMEASUREMENT_REPORT_ELEMENT	pReport;
	PBEACON_REPORT				pBeaconReport;
	UCHAR						Index, ReqIdx;
	USHORT						Length;
	PUCHAR						pDest;
	PBSS_ENTRY					pBss;

	
	ReqIdx = pAd->StaCfg.CurrentRMReqIdx;

	for (Index = 0; Index < pAd->StaCfg.CCXBssTab.BssNr; Index++)
	{
		
		
		pDest  = (PUCHAR) &pAd->StaCfg.FrameReportBuf[pAd->StaCfg.FrameReportLen];
		pBss   = (PBSS_ENTRY) &pAd->StaCfg.CCXBssTab.BssEntry[Index];
		Length = 0;

		
		pReport         = (PMEASUREMENT_REPORT_ELEMENT) pDest;
		pReport->Eid    = IE_MEASUREMENT_REPORT;
		pReport->Length = 0;
		pReport->Token  = pAd->StaCfg.MeasurementRequest[ReqIdx].ReqElem.Token;
		pReport->Mode   = pAd->StaCfg.MeasurementRequest[ReqIdx].ReqElem.Mode;
		pReport->Type   = MSRN_TYPE_BEACON_REQ;
		Length          = sizeof(MEASUREMENT_REPORT_ELEMENT);
		pDest          += sizeof(MEASUREMENT_REPORT_ELEMENT);

		
		pBeaconReport = (PBEACON_REPORT) pDest;
		pDest        += sizeof(BEACON_REPORT);
		Length       += sizeof(BEACON_REPORT);

		
		pBeaconReport->Channel        = pBss->Channel;
		pBeaconReport->Spare          = 0;
		pBeaconReport->Duration       = pAd->StaCfg.MeasurementRequest[ReqIdx].Measurement.Duration;
		pBeaconReport->PhyType        = ((pBss->SupRateLen+pBss->ExtRateLen > 4) ? PHY_ERP : PHY_DSS);
		pBeaconReport->RxPower        = pBss->Rssi - pAd->BbpRssiToDbmDelta;
		pBeaconReport->BeaconInterval = pBss->BeaconPeriod;
		pBeaconReport->CapabilityInfo = pBss->CapabilityInfo;
		COPY_MAC_ADDR(pBeaconReport->BSSID, pBss->Bssid);
		NdisMoveMemory(pBeaconReport->ParentTSF, pBss->PTSF, 4);
		NdisMoveMemory(pBeaconReport->TargetTSF, pBss->TTSF, 8);

		
		*pDest++ = 0x00;
		*pDest++ = pBss->SsidLen;
		NdisMoveMemory(pDest, pBss->Ssid, pBss->SsidLen);
		pDest  += pBss->SsidLen;
		Length += (2 + pBss->SsidLen);

		
		*pDest++ = 0x01;
		*pDest++ = pBss->SupRateLen;
		NdisMoveMemory(pDest, pBss->SupRate, pBss->SupRateLen);
		pDest  += pBss->SupRateLen;
		Length += (2 + pBss->SupRateLen);

		
		*pDest++ = 0x03;
		*pDest++ = 1;
		*pDest++ = pBss->Channel;
		Length  += 3;

		
		if (pBss->BssType == BSS_ADHOC)
		{
			*pDest++ = 0x06;
			*pDest++ = 2;
			*(PUSHORT) pDest = pBss->AtimWin;
			pDest   += 2;
			Length  += 4;
		}

		
		pReport->Length = Length - 4;

		
		pAd->StaCfg.FrameReportLen += Length;
	}
}
