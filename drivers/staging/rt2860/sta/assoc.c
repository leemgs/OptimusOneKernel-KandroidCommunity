
#include "../rt_config.h"

UCHAR	CipherWpaTemplate[] = {
		0xdd, 					
		0x16,					
		0x00, 0x50, 0xf2, 0x01,	
		0x01, 0x00,				
		0x00, 0x50, 0xf2, 0x02,	
		0x01, 0x00,				
		0x00, 0x50, 0xf2, 0x02,	
		0x01, 0x00,				
		0x00, 0x50, 0xf2, 0x01	
		};

UCHAR	CipherWpa2Template[] = {
		0x30,					
		0x14,					
		0x01, 0x00,				
		0x00, 0x0f, 0xac, 0x02,	
		0x01, 0x00,				
		0x00, 0x0f, 0xac, 0x02,	
		0x01, 0x00,				
		0x00, 0x0f, 0xac, 0x02,	
		0x00, 0x00,				
		};

UCHAR	Ccx2IeInfo[] = { 0x00, 0x40, 0x96, 0x03, 0x02};


VOID AssocStateMachineInit(
	IN	PRTMP_ADAPTER	pAd,
	IN  STATE_MACHINE *S,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(S, Trans, MAX_ASSOC_STATE, MAX_ASSOC_MSG, (STATE_MACHINE_FUNC)Drop, ASSOC_IDLE, ASSOC_MACHINE_BASE);

	
	StateMachineSetAction(S, ASSOC_IDLE, MT2_MLME_ASSOC_REQ, (STATE_MACHINE_FUNC)MlmeAssocReqAction);
	StateMachineSetAction(S, ASSOC_IDLE, MT2_MLME_REASSOC_REQ, (STATE_MACHINE_FUNC)MlmeReassocReqAction);
	StateMachineSetAction(S, ASSOC_IDLE, MT2_MLME_DISASSOC_REQ, (STATE_MACHINE_FUNC)MlmeDisassocReqAction);
	StateMachineSetAction(S, ASSOC_IDLE, MT2_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC)PeerDisassocAction);

	
	StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_MLME_ASSOC_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenAssoc);
	StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_MLME_REASSOC_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenReassoc);
	StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_MLME_DISASSOC_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenDisassociate);
	StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC)PeerDisassocAction);
	StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_PEER_ASSOC_RSP, (STATE_MACHINE_FUNC)PeerAssocRspAction);
	
	
	
	
	StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_PEER_REASSOC_RSP, (STATE_MACHINE_FUNC)PeerAssocRspAction);
	StateMachineSetAction(S, ASSOC_WAIT_RSP, MT2_ASSOC_TIMEOUT, (STATE_MACHINE_FUNC)AssocTimeoutAction);

	
	StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_MLME_ASSOC_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenAssoc);
	StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_MLME_REASSOC_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenReassoc);
	StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_MLME_DISASSOC_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenDisassociate);
	StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC)PeerDisassocAction);
	StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_PEER_REASSOC_RSP, (STATE_MACHINE_FUNC)PeerReassocRspAction);
	
	
	
	StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_PEER_ASSOC_RSP, (STATE_MACHINE_FUNC)PeerReassocRspAction);
	StateMachineSetAction(S, REASSOC_WAIT_RSP, MT2_REASSOC_TIMEOUT, (STATE_MACHINE_FUNC)ReassocTimeoutAction);

	
	StateMachineSetAction(S, DISASSOC_WAIT_RSP, MT2_MLME_ASSOC_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenAssoc);
	StateMachineSetAction(S, DISASSOC_WAIT_RSP, MT2_MLME_REASSOC_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenReassoc);
	StateMachineSetAction(S, DISASSOC_WAIT_RSP, MT2_MLME_DISASSOC_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenDisassociate);
	StateMachineSetAction(S, DISASSOC_WAIT_RSP, MT2_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC)PeerDisassocAction);
	StateMachineSetAction(S, DISASSOC_WAIT_RSP, MT2_DISASSOC_TIMEOUT, (STATE_MACHINE_FUNC)DisassocTimeoutAction);

	
	RTMPInitTimer(pAd, &pAd->MlmeAux.AssocTimer, GET_TIMER_FUNCTION(AssocTimeout), pAd, FALSE);
	RTMPInitTimer(pAd, &pAd->MlmeAux.ReassocTimer, GET_TIMER_FUNCTION(ReassocTimeout), pAd, FALSE);
	RTMPInitTimer(pAd, &pAd->MlmeAux.DisassocTimer, GET_TIMER_FUNCTION(DisassocTimeout), pAd, FALSE);
}


VOID AssocTimeout(IN PVOID SystemSpecific1,
				 IN PVOID FunctionContext,
				 IN PVOID SystemSpecific2,
				 IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return;

	MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_ASSOC_TIMEOUT, 0, NULL);
	RT28XX_MLME_HANDLER(pAd);
}


VOID ReassocTimeout(IN PVOID SystemSpecific1,
					IN PVOID FunctionContext,
					IN PVOID SystemSpecific2,
					IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return;

	MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_REASSOC_TIMEOUT, 0, NULL);
	RT28XX_MLME_HANDLER(pAd);
}


VOID DisassocTimeout(IN PVOID SystemSpecific1,
					IN PVOID FunctionContext,
					IN PVOID SystemSpecific2,
					IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return;

	MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_DISASSOC_TIMEOUT, 0, NULL);
	RT28XX_MLME_HANDLER(pAd);
}


VOID MlmeAssocReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR			ApAddr[6];
	HEADER_802_11	AssocHdr;
	UCHAR			Ccx2Len = 5;
	UCHAR			WmeIe[9] = {IE_VENDOR_SPECIFIC, 0x07, 0x00, 0x50, 0xf2, 0x02, 0x00, 0x01, 0x00};
	USHORT			ListenIntv;
	ULONG			Timeout;
	USHORT			CapabilityInfo;
	BOOLEAN			TimerCancelled;
	PUCHAR			pOutBuffer = NULL;
	NDIS_STATUS		NStatus;
	ULONG			FrameLen = 0;
	ULONG			tmp;
	USHORT			VarIesOffset;
	UCHAR			CkipFlag;
	UCHAR			CkipNegotiationBuffer[CKIP_NEGOTIATION_LENGTH];
	UCHAR			AironetCkipIe = IE_AIRONET_CKIP;
	UCHAR			AironetCkipLen = CKIP_NEGOTIATION_LENGTH;
	UCHAR			AironetIPAddressIE = IE_AIRONET_IPADDRESS;
	UCHAR			AironetIPAddressLen = AIRONET_IPADDRESS_LENGTH;
	UCHAR			AironetIPAddressBuffer[AIRONET_IPADDRESS_LENGTH] = {0x00, 0x40, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00};
	USHORT			Status;

	
	if (pAd->StaCfg.bBlockAssoc == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Block Assoc request durning WPA block period!\n"));
		pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
		Status = MLME_STATE_MACHINE_REJECT;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2, &Status);
	}
	
	else if (MlmeAssocReqSanity(pAd, Elem->Msg, Elem->MsgLen, ApAddr, &CapabilityInfo, &Timeout, &ListenIntv))
	{
		RTMPCancelTimer(&pAd->MlmeAux.AssocTimer, &TimerCancelled);
		COPY_MAC_ADDR(pAd->MlmeAux.Bssid, ApAddr);

		
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
		if (NStatus != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_TRACE,("ASSOC - MlmeAssocReqAction() allocate memory failed \n"));
			pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
			Status = MLME_FAIL_NO_RESOURCE;
			MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2, &Status);
			return;
		}

		
		pAd->StaCfg.AssocInfo.Length = sizeof(NDIS_802_11_ASSOCIATION_INFORMATION);
		
		pAd->StaCfg.AssocInfo.AvailableRequestFixedIEs =
			NDIS_802_11_AI_REQFI_CAPABILITIES | NDIS_802_11_AI_REQFI_LISTENINTERVAL;
		pAd->StaCfg.AssocInfo.RequestFixedIEs.Capabilities = CapabilityInfo;
		pAd->StaCfg.AssocInfo.RequestFixedIEs.ListenInterval = ListenIntv;
		
		
		pAd->StaCfg.AssocInfo.OffsetRequestIEs = sizeof(NDIS_802_11_ASSOCIATION_INFORMATION);

        NdisZeroMemory(pAd->StaCfg.ReqVarIEs, MAX_VIE_LEN);
		
		VarIesOffset = 0;
		NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, &SsidIe, 1);
		VarIesOffset += 1;
		NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, &pAd->MlmeAux.SsidLen, 1);
		VarIesOffset += 1;
		NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
		VarIesOffset += pAd->MlmeAux.SsidLen;

		
		NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, &SupRateIe, 1);
		VarIesOffset += 1;
		NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, &pAd->MlmeAux.SupRateLen, 1);
		VarIesOffset += 1;
		NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, pAd->MlmeAux.SupRate, pAd->MlmeAux.SupRateLen);
		VarIesOffset += pAd->MlmeAux.SupRateLen;
		

        if ((pAd->CommonCfg.Channel > 14) &&
            (pAd->CommonCfg.bIEEE80211H == TRUE))
            CapabilityInfo |= 0x0100;

		DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Send ASSOC request...\n"));
		MgtMacHeaderInit(pAd, &AssocHdr, SUBTYPE_ASSOC_REQ, 0, ApAddr, ApAddr);

		
		MakeOutgoingFrame(pOutBuffer,				&FrameLen,
						  sizeof(HEADER_802_11),	&AssocHdr,
						  2,						&CapabilityInfo,
						  2,						&ListenIntv,
						  1,						&SsidIe,
						  1,						&pAd->MlmeAux.SsidLen,
						  pAd->MlmeAux.SsidLen, 	pAd->MlmeAux.Ssid,
						  1,						&SupRateIe,
						  1,						&pAd->MlmeAux.SupRateLen,
						  pAd->MlmeAux.SupRateLen,  pAd->MlmeAux.SupRate,
						  END_OF_ARGS);

		if (pAd->MlmeAux.ExtRateLen != 0)
		{
			MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
							  1,                        &ExtRateIe,
							  1,                        &pAd->MlmeAux.ExtRateLen,
							  pAd->MlmeAux.ExtRateLen,  pAd->MlmeAux.ExtRate,
							  END_OF_ARGS);
			FrameLen += tmp;
		}

		
		if ((pAd->MlmeAux.HtCapabilityLen > 0) && (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
		{
			ULONG TmpLen;
			UCHAR HtLen;
			UCHAR BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
			if (pAd->StaActive.SupportedPhyInfo.bPreNHt == TRUE)
			{
				HtLen = SIZE_HT_CAP_IE + 4;
				MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
							  1,                                &WpaIe,
							  1,                                &HtLen,
							  4,                                &BROADCOM[0],
							 pAd->MlmeAux.HtCapabilityLen,          &pAd->MlmeAux.HtCapability,
							  END_OF_ARGS);
			}
			else
			{
				MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
							  1,                                &HtCapIe,
							  1,                                &pAd->MlmeAux.HtCapabilityLen,
							 pAd->MlmeAux.HtCapabilityLen,          &pAd->MlmeAux.HtCapability,
							  END_OF_ARGS);
			}
			FrameLen += TmpLen;
		}

		
		
		
		
		
		
		
		
		if (pAd->CommonCfg.bAggregationCapable)
		{
			if ((pAd->CommonCfg.bPiggyBackCapable) && ((pAd->MlmeAux.APRalinkIe & 0x00000003) == 3))
			{
				ULONG TmpLen;
				UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x03, 0x00, 0x00, 0x00};
				MakeOutgoingFrame(pOutBuffer+FrameLen,           &TmpLen,
								  9,                             RalinkIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
			else if (pAd->MlmeAux.APRalinkIe & 0x00000001)
			{
				ULONG TmpLen;
				UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x01, 0x00, 0x00, 0x00};
				MakeOutgoingFrame(pOutBuffer+FrameLen,           &TmpLen,
								  9,                             RalinkIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
		}
		else
		{
			ULONG TmpLen;
			UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x06, 0x00, 0x00, 0x00};
			MakeOutgoingFrame(pOutBuffer+FrameLen,		 &TmpLen,
							  9,						 RalinkIe,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}

		if (pAd->MlmeAux.APEdcaParm.bValid)
		{
			if (pAd->CommonCfg.bAPSDCapable && pAd->MlmeAux.APEdcaParm.bAPSDCapable)
			{
				QBSS_STA_INFO_PARM QosInfo;

				NdisZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));
				QosInfo.UAPSD_AC_BE = pAd->CommonCfg.bAPSDAC_BE;
				QosInfo.UAPSD_AC_BK = pAd->CommonCfg.bAPSDAC_BK;
				QosInfo.UAPSD_AC_VI = pAd->CommonCfg.bAPSDAC_VI;
				QosInfo.UAPSD_AC_VO = pAd->CommonCfg.bAPSDAC_VO;
				QosInfo.MaxSPLength = pAd->CommonCfg.MaxSPLength;
				WmeIe[8] |= *(PUCHAR)&QosInfo;
			}
			else
			{
                
                
			}

			MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
							  9,                        &WmeIe[0],
							  END_OF_ARGS);
			FrameLen += tmp;
		}

		
		
		
		
		
		
		if (((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK) ||
            (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK) ||
            (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) ||
            (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2)
			)
            )
		{
			UCHAR RSNIe = IE_WPA;

			if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK) ||
                (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2))
			{
				RSNIe = IE_WPA2;
			}

			if (pAd->StaCfg.WpaSupplicantUP != 1)
            RTMPMakeRSNIE(pAd, pAd->StaCfg.AuthMode, pAd->StaCfg.WepStatus, BSS0);

            
			if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2)
			{
			    INT     idx;
                BOOLEAN FoundPMK = FALSE;
				
				for (idx = 0; idx < PMKID_NO; idx++)
				{
					if (NdisEqualMemory(ApAddr, &pAd->StaCfg.SavedPMK[idx].BSSID, 6))
					{
						FoundPMK = TRUE;
						break;
					}
				}

				if (FoundPMK)
				{
					
					*(PUSHORT) &pAd->StaCfg.RSN_IE[pAd->StaCfg.RSNIE_Len] = 1;
					NdisMoveMemory(&pAd->StaCfg.RSN_IE[pAd->StaCfg.RSNIE_Len + 2], &pAd->StaCfg.SavedPMK[idx].PMKID, 16);
                    pAd->StaCfg.RSNIE_Len += 18;
				}
			}

			if (pAd->StaCfg.WpaSupplicantUP == 1)
			{
				MakeOutgoingFrame(pOutBuffer + FrameLen,    		&tmp,
		                        	pAd->StaCfg.RSNIE_Len,			pAd->StaCfg.RSN_IE,
		                        	END_OF_ARGS);
			}
			else
			{
				MakeOutgoingFrame(pOutBuffer + FrameLen,    		&tmp,
				              		1,                              &RSNIe,
		                        	1,                              &pAd->StaCfg.RSNIE_Len,
		                        	pAd->StaCfg.RSNIE_Len,			pAd->StaCfg.RSN_IE,
		                        	END_OF_ARGS);
			}

			FrameLen += tmp;

			if (pAd->StaCfg.WpaSupplicantUP != 1)
			{
	            
	            NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, &RSNIe, 1);
	            VarIesOffset += 1;
	            NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, &pAd->StaCfg.RSNIE_Len, 1);
	            VarIesOffset += 1;
			}
			NdisMoveMemory(pAd->StaCfg.ReqVarIEs + VarIesOffset, pAd->StaCfg.RSN_IE, pAd->StaCfg.RSNIE_Len);
			VarIesOffset += pAd->StaCfg.RSNIE_Len;

			
			pAd->StaCfg.ReqVarIELen = VarIesOffset;
		}

		
		CkipFlag = pAd->StaCfg.CkipFlag;
		if (CkipFlag != 0)
		{
			NdisZeroMemory(CkipNegotiationBuffer, CKIP_NEGOTIATION_LENGTH);
			CkipNegotiationBuffer[2] = 0x66;
			
			CkipNegotiationBuffer[8] = 0x18;
			CkipNegotiationBuffer[CKIP_NEGOTIATION_LENGTH - 1] = 0x22;
			CkipFlag = 0x18;

			MakeOutgoingFrame(pOutBuffer + FrameLen, 	&tmp,
						1,						  		&AironetCkipIe,
						1,						  		&AironetCkipLen,
						AironetCkipLen, 		  		CkipNegotiationBuffer,
						END_OF_ARGS);
			FrameLen += tmp;
		}

		
		if (pAd->StaCfg.CCXControl.field.Enable == 1)
		{

			
			
			
			
			MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
						1,							&AironetIPAddressIE,
						1,							&AironetIPAddressLen,
						AironetIPAddressLen,		AironetIPAddressBuffer,
						1,							&Ccx2Ie,
						1,							&Ccx2Len,
						Ccx2Len,				    Ccx2IeInfo,
						END_OF_ARGS);
			FrameLen += tmp;

			
			
			pAd->StaCfg.ReqVarIELen = VarIesOffset;
			pAd->StaCfg.AssocInfo.RequestIELength = VarIesOffset;

			
			pAd->StaCfg.AssocInfo.OffsetResponseIEs = sizeof(NDIS_802_11_ASSOCIATION_INFORMATION) + pAd->StaCfg.ReqVarIELen;
			
		}


		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);

		RTMPSetTimer(&pAd->MlmeAux.AssocTimer, Timeout);
		pAd->Mlme.AssocMachine.CurrState = ASSOC_WAIT_RSP;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE,("ASSOC - MlmeAssocReqAction() sanity check failed. BUG!!!!!! \n"));
		pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
		Status = MLME_INVALID_FORMAT;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2, &Status);
	}

}


VOID MlmeReassocReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR			ApAddr[6];
	HEADER_802_11	ReassocHdr;
	UCHAR			Ccx2Len = 5;
	UCHAR			WmeIe[9] = {IE_VENDOR_SPECIFIC, 0x07, 0x00, 0x50, 0xf2, 0x02, 0x00, 0x01, 0x00};
	USHORT			CapabilityInfo, ListenIntv;
	ULONG			Timeout;
	ULONG			FrameLen = 0;
	BOOLEAN			TimerCancelled;
	NDIS_STATUS		NStatus;
	ULONG			tmp;
	PUCHAR			pOutBuffer = NULL;
	USHORT			Status;

	
	if (pAd->StaCfg.bBlockAssoc == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Block ReAssoc request durning WPA block period!\n"));
		pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
		Status = MLME_STATE_MACHINE_REJECT;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_REASSOC_CONF, 2, &Status);
	}
	
	else if(MlmeAssocReqSanity(pAd, Elem->Msg, Elem->MsgLen, ApAddr, &CapabilityInfo, &Timeout, &ListenIntv))
	{
		RTMPCancelTimer(&pAd->MlmeAux.ReassocTimer, &TimerCancelled);

		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  
		if(NStatus != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_TRACE,("ASSOC - MlmeReassocReqAction() allocate memory failed \n"));
			pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
			Status = MLME_FAIL_NO_RESOURCE;
			MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_REASSOC_CONF, 2, &Status);
			return;
		}

		COPY_MAC_ADDR(pAd->MlmeAux.Bssid, ApAddr);

		
		DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Send RE-ASSOC request...\n"));
		MgtMacHeaderInit(pAd, &ReassocHdr, SUBTYPE_REASSOC_REQ, 0, ApAddr, ApAddr);
		MakeOutgoingFrame(pOutBuffer,               &FrameLen,
						  sizeof(HEADER_802_11),    &ReassocHdr,
						  2,                        &CapabilityInfo,
						  2,                        &ListenIntv,
						  MAC_ADDR_LEN,             ApAddr,
						  1,                        &SsidIe,
						  1,                        &pAd->MlmeAux.SsidLen,
						  pAd->MlmeAux.SsidLen,     pAd->MlmeAux.Ssid,
						  1,                        &SupRateIe,
						  1,						&pAd->MlmeAux.SupRateLen,
						  pAd->MlmeAux.SupRateLen,  pAd->MlmeAux.SupRate,
						  END_OF_ARGS);

		if (pAd->MlmeAux.ExtRateLen != 0)
		{
			MakeOutgoingFrame(pOutBuffer + FrameLen,        &tmp,
							  1,                            &ExtRateIe,
							  1,                            &pAd->MlmeAux.ExtRateLen,
							  pAd->MlmeAux.ExtRateLen,	    pAd->MlmeAux.ExtRate,
							  END_OF_ARGS);
			FrameLen += tmp;
		}

		if (pAd->MlmeAux.APEdcaParm.bValid)
		{
			if (pAd->CommonCfg.bAPSDCapable && pAd->MlmeAux.APEdcaParm.bAPSDCapable)
			{
				QBSS_STA_INFO_PARM QosInfo;

				NdisZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));
				QosInfo.UAPSD_AC_BE = pAd->CommonCfg.bAPSDAC_BE;
				QosInfo.UAPSD_AC_BK = pAd->CommonCfg.bAPSDAC_BK;
				QosInfo.UAPSD_AC_VI = pAd->CommonCfg.bAPSDAC_VI;
				QosInfo.UAPSD_AC_VO = pAd->CommonCfg.bAPSDAC_VO;
				QosInfo.MaxSPLength = pAd->CommonCfg.MaxSPLength;
				WmeIe[8] |= *(PUCHAR)&QosInfo;
			}

			MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
							  9,                        &WmeIe[0],
							  END_OF_ARGS);
			FrameLen += tmp;
		}

		
		if ((pAd->MlmeAux.HtCapabilityLen > 0) && (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
		{
			ULONG TmpLen;
			UCHAR HtLen;
			UCHAR BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
			if (pAd->StaActive.SupportedPhyInfo.bPreNHt == TRUE)
			{
				HtLen = SIZE_HT_CAP_IE + 4;
				MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
							  1,                                &WpaIe,
							  1,                                &HtLen,
							  4,                                &BROADCOM[0],
							 pAd->MlmeAux.HtCapabilityLen,          &pAd->MlmeAux.HtCapability,
							  END_OF_ARGS);
			}
			else
			{
				MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
							  1,                                &HtCapIe,
							  1,                                &pAd->MlmeAux.HtCapabilityLen,
							 pAd->MlmeAux.HtCapabilityLen,          &pAd->MlmeAux.HtCapability,
							  END_OF_ARGS);
			}
			FrameLen += TmpLen;
		}

		
		
		
		
		
		
		
		
		if (pAd->CommonCfg.bAggregationCapable)
		{
			if ((pAd->CommonCfg.bPiggyBackCapable) && ((pAd->MlmeAux.APRalinkIe & 0x00000003) == 3))
			{
				ULONG TmpLen;
				UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x03, 0x00, 0x00, 0x00};
				MakeOutgoingFrame(pOutBuffer+FrameLen,           &TmpLen,
								  9,                             RalinkIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
			else if (pAd->MlmeAux.APRalinkIe & 0x00000001)
			{
				ULONG TmpLen;
				UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x01, 0x00, 0x00, 0x00};
				MakeOutgoingFrame(pOutBuffer+FrameLen,           &TmpLen,
								  9,                             RalinkIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
		}
		else
		{
			ULONG TmpLen;
			UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x04, 0x00, 0x00, 0x00};
			MakeOutgoingFrame(pOutBuffer+FrameLen,		 &TmpLen,
							  9,						 RalinkIe,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}

		
		if (pAd->StaCfg.CCXControl.field.Enable == 1)
		{
			
			
			
			MakeOutgoingFrame(pOutBuffer + FrameLen, &tmp,
						1,							&Ccx2Ie,
						1,							&Ccx2Len,
						Ccx2Len,				    Ccx2IeInfo,
						END_OF_ARGS);
			FrameLen += tmp;
		}

		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);

		RTMPSetTimer(&pAd->MlmeAux.ReassocTimer, Timeout); 
		pAd->Mlme.AssocMachine.CurrState = REASSOC_WAIT_RSP;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE,("ASSOC - MlmeReassocReqAction() sanity check failed. BUG!!!! \n"));
		pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
		Status = MLME_INVALID_FORMAT;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_REASSOC_CONF, 2, &Status);
	}
}


VOID MlmeDisassocReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PMLME_DISASSOC_REQ_STRUCT pDisassocReq;
	HEADER_802_11         DisassocHdr;
	PHEADER_802_11        pDisassocHdr;
	PUCHAR                pOutBuffer = NULL;
	ULONG                 FrameLen = 0;
	NDIS_STATUS           NStatus;
	BOOLEAN               TimerCancelled;
	ULONG                 Timeout = 0;
	USHORT                Status;

	
	pDisassocReq = (PMLME_DISASSOC_REQ_STRUCT)(Elem->Msg);

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  
	if (NStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - MlmeDisassocReqAction() allocate memory failed\n"));
		pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
		Status = MLME_FAIL_NO_RESOURCE;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_DISASSOC_CONF, 2, &Status);
		return;
	}



	RTMPCancelTimer(&pAd->MlmeAux.DisassocTimer, &TimerCancelled);

	DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Send DISASSOC request[BSSID::%02x:%02x:%02x:%02x:%02x:%02x (Reason=%d)\n",
				pDisassocReq->Addr[0], pDisassocReq->Addr[1], pDisassocReq->Addr[2],
				pDisassocReq->Addr[3], pDisassocReq->Addr[4], pDisassocReq->Addr[5], pDisassocReq->Reason));
	MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pDisassocReq->Addr, pDisassocReq->Addr);	
	MakeOutgoingFrame(pOutBuffer,           &FrameLen,
					  sizeof(HEADER_802_11),&DisassocHdr,
					  2,                    &pDisassocReq->Reason,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);

	
	
	
	pDisassocHdr = (PHEADER_802_11)pOutBuffer;
	pDisassocHdr->FC.SubType = SUBTYPE_DEAUTH;
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	pAd->StaCfg.DisassocReason = REASON_DISASSOC_STA_LEAVING;
	COPY_MAC_ADDR(pAd->StaCfg.DisassocSta, pDisassocReq->Addr);

	RTMPSetTimer(&pAd->MlmeAux.DisassocTimer, Timeout); 
	pAd->Mlme.AssocMachine.CurrState = DISASSOC_WAIT_RSP;

    {
        union iwreq_data    wrqu;
        memset(wrqu.ap_addr.sa_data, 0, MAC_ADDR_LEN);
        wireless_send_event(pAd->net_dev, SIOCGIWAP, &wrqu, NULL);
    }
}


VOID PeerAssocRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT        CapabilityInfo, Status, Aid;
	UCHAR         SupRate[MAX_LEN_OF_SUPPORTED_RATES], SupRateLen;
	UCHAR         ExtRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRateLen;
	UCHAR         Addr2[MAC_ADDR_LEN];
	BOOLEAN       TimerCancelled;
	UCHAR         CkipFlag;
	EDCA_PARM     EdcaParm;
	HT_CAPABILITY_IE		HtCapability;
	ADD_HT_INFO_IE		AddHtInfo;	
	UCHAR			HtCapabilityLen;
	UCHAR			AddHtInfoLen;
	UCHAR			NewExtChannelOffset = 0xff;

	if (PeerAssocRspSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &CapabilityInfo, &Status, &Aid, SupRate, &SupRateLen, ExtRate, &ExtRateLen,
		&HtCapability,&AddHtInfo, &HtCapabilityLen,&AddHtInfoLen,&NewExtChannelOffset, &EdcaParm, &CkipFlag))
	{
		
		if(MAC_ADDR_EQUAL(Addr2, pAd->MlmeAux.Bssid))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("PeerAssocRspAction():ASSOC - receive ASSOC_RSP to me (status=%d)\n", Status));
			DBGPRINT(RT_DEBUG_TRACE, ("PeerAssocRspAction():MacTable [%d].AMsduSize = %d. ClientStatusFlags = 0x%lx \n",Elem->Wcid, pAd->MacTab.Content[BSSID_WCID].AMsduSize, pAd->MacTab.Content[BSSID_WCID].ClientStatusFlags));
			RTMPCancelTimer(&pAd->MlmeAux.AssocTimer, &TimerCancelled);
			if(Status == MLME_SUCCESS)
			{
#ifdef RT2860
				
				AssocPostProc(pAd, Addr2, CapabilityInfo, Aid, SupRate, SupRateLen, ExtRate, ExtRateLen,
					&EdcaParm, &HtCapability, HtCapabilityLen, &AddHtInfo);

                {
                    union iwreq_data    wrqu;
                    wext_notify_event_assoc(pAd);

                    memset(wrqu.ap_addr.sa_data, 0, MAC_ADDR_LEN);
                    memcpy(wrqu.ap_addr.sa_data, pAd->MlmeAux.Bssid, MAC_ADDR_LEN);
                    wireless_send_event(pAd->net_dev, SIOCGIWAP, &wrqu, NULL);

                }
#endif
#ifdef RT2870
				UCHAR			MaxSupportedRateIn500Kbps = 0;
				UCHAR			idx;

				
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
				
				AssocPostProc(pAd, Addr2, CapabilityInfo, Aid, SupRate, SupRateLen, ExtRate, ExtRateLen,
					&EdcaParm, &HtCapability, HtCapabilityLen, &AddHtInfo);

				StaAddMacTableEntry(pAd, &pAd->MacTab.Content[BSSID_WCID], MaxSupportedRateIn500Kbps, &HtCapability, HtCapabilityLen, CapabilityInfo);
#endif
				pAd->StaCfg.CkipFlag = CkipFlag;
				if (CkipFlag & 0x18)
				{
					NdisZeroMemory(pAd->StaCfg.TxSEQ, 4);
					NdisZeroMemory(pAd->StaCfg.RxSEQ, 4);
					NdisZeroMemory(pAd->StaCfg.CKIPMIC, 4);
					pAd->StaCfg.GIV[0] = RandomByte(pAd);
					pAd->StaCfg.GIV[1] = RandomByte(pAd);
					pAd->StaCfg.GIV[2] = RandomByte(pAd);
					pAd->StaCfg.bCkipOn = TRUE;
					DBGPRINT(RT_DEBUG_TRACE, ("<CCX> pAd->StaCfg.CkipFlag = 0x%02x\n", pAd->StaCfg.CkipFlag));
				}
			}
			else
			{
			}
			pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
			MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2, &Status);
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - PeerAssocRspAction() sanity check fail\n"));
	}
}


VOID PeerReassocRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT      CapabilityInfo;
	USHORT      Status;
	USHORT      Aid;
	UCHAR       SupRate[MAX_LEN_OF_SUPPORTED_RATES], SupRateLen;
	UCHAR       ExtRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRateLen;
	UCHAR       Addr2[MAC_ADDR_LEN];
	UCHAR       CkipFlag;
	BOOLEAN     TimerCancelled;
	EDCA_PARM   EdcaParm;
	HT_CAPABILITY_IE		HtCapability;
	ADD_HT_INFO_IE		AddHtInfo;	
	UCHAR			HtCapabilityLen;
	UCHAR			AddHtInfoLen;
	UCHAR			NewExtChannelOffset = 0xff;

	if(PeerAssocRspSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &CapabilityInfo, &Status, &Aid, SupRate, &SupRateLen, ExtRate, &ExtRateLen,
								&HtCapability,	&AddHtInfo, &HtCapabilityLen, &AddHtInfoLen,&NewExtChannelOffset, &EdcaParm, &CkipFlag))
	{
		if(MAC_ADDR_EQUAL(Addr2, pAd->MlmeAux.Bssid)) 
		{
			DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - receive REASSOC_RSP to me (status=%d)\n", Status));
			RTMPCancelTimer(&pAd->MlmeAux.ReassocTimer, &TimerCancelled);

			if(Status == MLME_SUCCESS)
			{
				
				AssocPostProc(pAd, Addr2, CapabilityInfo, Aid, SupRate, SupRateLen, ExtRate, ExtRateLen,
					 &EdcaParm, &HtCapability, HtCapabilityLen, &AddHtInfo);

                {
                    union iwreq_data    wrqu;
                    wext_notify_event_assoc(pAd);

                    memset(wrqu.ap_addr.sa_data, 0, MAC_ADDR_LEN);
                    memcpy(wrqu.ap_addr.sa_data, pAd->MlmeAux.Bssid, MAC_ADDR_LEN);
                    wireless_send_event(pAd->net_dev, SIOCGIWAP, &wrqu, NULL);

                }

			}

			{
				
				pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
				MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_REASSOC_CONF, 2, &Status);
			}
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - PeerReassocRspAction() sanity check fail\n"));
	}

}


VOID AssocPostProc(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr2,
	IN USHORT CapabilityInfo,
	IN USHORT Aid,
	IN UCHAR SupRate[],
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN PEDCA_PARM pEdcaParm,
	IN HT_CAPABILITY_IE		*pHtCapability,
	IN UCHAR HtCapabilityLen,
	IN ADD_HT_INFO_IE		*pAddHtInfo)	
{
	ULONG Idx;

	pAd->MlmeAux.BssType = BSS_INFRA;
	COPY_MAC_ADDR(pAd->MlmeAux.Bssid, pAddr2);
	pAd->MlmeAux.Aid = Aid;
	pAd->MlmeAux.CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;

	
	if ((HtCapabilityLen > 0) && (pEdcaParm->bValid == FALSE))
	{
		pEdcaParm->bValid = TRUE;
		pEdcaParm->Aifsn[0] = 3;
		pEdcaParm->Aifsn[1] = 7;
		pEdcaParm->Aifsn[2] = 2;
		pEdcaParm->Aifsn[3] = 2;

		pEdcaParm->Cwmin[0] = 4;
		pEdcaParm->Cwmin[1] = 4;
		pEdcaParm->Cwmin[2] = 3;
		pEdcaParm->Cwmin[3] = 2;

		pEdcaParm->Cwmax[0] = 10;
		pEdcaParm->Cwmax[1] = 10;
		pEdcaParm->Cwmax[2] = 4;
		pEdcaParm->Cwmax[3] = 3;

		pEdcaParm->Txop[0]  = 0;
		pEdcaParm->Txop[1]  = 0;
		pEdcaParm->Txop[2]  = 96;
		pEdcaParm->Txop[3]  = 48;

	}

	NdisMoveMemory(&pAd->MlmeAux.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));

	
	pAd->MlmeAux.SupRateLen = SupRateLen;
	NdisMoveMemory(pAd->MlmeAux.SupRate, SupRate, SupRateLen);
	RTMPCheckRates(pAd, pAd->MlmeAux.SupRate, &pAd->MlmeAux.SupRateLen);

	
	pAd->MlmeAux.ExtRateLen = ExtRateLen;
	NdisMoveMemory(pAd->MlmeAux.ExtRate, ExtRate, ExtRateLen);
	RTMPCheckRates(pAd, pAd->MlmeAux.ExtRate, &pAd->MlmeAux.ExtRateLen);

	if (HtCapabilityLen > 0)
	{
		RTMPCheckHt(pAd, BSSID_WCID, pHtCapability, pAddHtInfo);
	}
	DBGPRINT(RT_DEBUG_TRACE, ("AssocPostProc===>  AP.AMsduSize = %d. ClientStatusFlags = 0x%lx \n", pAd->MacTab.Content[BSSID_WCID].AMsduSize, pAd->MacTab.Content[BSSID_WCID].ClientStatusFlags));

	DBGPRINT(RT_DEBUG_TRACE, ("AssocPostProc===>    (Mmps=%d, AmsduSize=%d, )\n",
		pAd->MacTab.Content[BSSID_WCID].MmpsMode, pAd->MacTab.Content[BSSID_WCID].AMsduSize));

	
	Idx = BssTableSearch(&pAd->ScanTab, pAddr2, pAd->MlmeAux.Channel);
	if (Idx == BSS_NOT_FOUND)
	{
		DBGPRINT_ERR(("ASSOC - Can't find BSS after receiving Assoc response\n"));
	}
	else
	{
		
		pAd->MacTab.Content[BSSID_WCID].RSNIE_Len = 0;
		NdisZeroMemory(pAd->MacTab.Content[BSSID_WCID].RSN_IE, MAX_LEN_OF_RSNIE);

		
		if ((pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA) && (pAd->ScanTab.BssEntry[Idx].VarIELen != 0))
		{
			PUCHAR              pVIE;
			USHORT              len;
			PEID_STRUCT         pEid;

			pVIE = pAd->ScanTab.BssEntry[Idx].VarIEs;
			len	 = pAd->ScanTab.BssEntry[Idx].VarIELen;

			while (len > 0)
			{
				pEid = (PEID_STRUCT) pVIE;
				
				if ((pEid->Eid == IE_WPA) && (NdisEqualMemory(pEid->Octet, WPA_OUI, 4))
					&& (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA || pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK))
				{
					NdisMoveMemory(pAd->MacTab.Content[BSSID_WCID].RSN_IE, pVIE, (pEid->Len + 2));
					pAd->MacTab.Content[BSSID_WCID].RSNIE_Len = (pEid->Len + 2);
					DBGPRINT(RT_DEBUG_TRACE, ("AssocPostProc===> Store RSN_IE for WPA SM negotiation \n"));
				}
				
				else if ((pEid->Eid == IE_RSN) && (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3))
					&& (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2 || pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
				{
					NdisMoveMemory(pAd->MacTab.Content[BSSID_WCID].RSN_IE, pVIE, (pEid->Len + 2));
					pAd->MacTab.Content[BSSID_WCID].RSNIE_Len = (pEid->Len + 2);
					DBGPRINT(RT_DEBUG_TRACE, ("AssocPostProc===> Store RSN_IE for WPA2 SM negotiation \n"));
				}

				pVIE += (pEid->Len + 2);
				len  -= (pEid->Len + 2);
			}
		}

		if (pAd->MacTab.Content[BSSID_WCID].RSNIE_Len == 0)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("AssocPostProc===> no RSN_IE \n"));
		}
		else
		{
			hex_dump("RSN_IE", pAd->MacTab.Content[BSSID_WCID].RSN_IE, pAd->MacTab.Content[BSSID_WCID].RSNIE_Len);
		}
	}
}


VOID PeerDisassocAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR         Addr2[MAC_ADDR_LEN];
	USHORT        Reason;

	DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - PeerDisassocAction()\n"));
	if(PeerDisassocSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Reason))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - PeerDisassocAction() Reason = %d\n", Reason));
		if (INFRA_ON(pAd) && MAC_ADDR_EQUAL(pAd->CommonCfg.Bssid, Addr2))
		{

			if (pAd->CommonCfg.bWirelessEvent)
			{
				RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
			}

			
			
			
			NdisGetSystemUpTime(&pAd->StaCfg.CCXAdjacentAPLinkDownTime);
			pAd->StaCfg.CCXAdjacentAPReportFlag = TRUE;
			LinkDown(pAd, TRUE);
			pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;

            {
                union iwreq_data    wrqu;
                memset(wrqu.ap_addr.sa_data, 0, MAC_ADDR_LEN);
                wireless_send_event(pAd->net_dev, SIOCGIWAP, &wrqu, NULL);
            }
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - PeerDisassocAction() sanity check fail\n"));
	}

}


VOID AssocTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT  Status;
	DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - AssocTimeoutAction\n"));
	pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
	Status = MLME_REJ_TIMEOUT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2, &Status);
}


VOID ReassocTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT  Status;
	DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - ReassocTimeoutAction\n"));
	pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
	Status = MLME_REJ_TIMEOUT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_REASSOC_CONF, 2, &Status);
}


VOID DisassocTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT  Status;
	DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - DisassocTimeoutAction\n"));
	pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
	Status = MLME_SUCCESS;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_DISASSOC_CONF, 2, &Status);
}

VOID InvalidStateWhenAssoc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT  Status;
	DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - InvalidStateWhenAssoc(state=%ld), reset ASSOC state machine\n",
		pAd->Mlme.AssocMachine.CurrState));
	pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
	Status = MLME_STATE_MACHINE_REJECT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_ASSOC_CONF, 2, &Status);
}

VOID InvalidStateWhenReassoc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT Status;
	DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - InvalidStateWhenReassoc(state=%ld), reset ASSOC state machine\n",
		pAd->Mlme.AssocMachine.CurrState));
	pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
	Status = MLME_STATE_MACHINE_REJECT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_REASSOC_CONF, 2, &Status);
}

VOID InvalidStateWhenDisassociate(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT Status;
	DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - InvalidStateWhenDisassoc(state=%ld), reset ASSOC state machine\n",
		pAd->Mlme.AssocMachine.CurrState));
	pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
	Status = MLME_STATE_MACHINE_REJECT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_DISASSOC_CONF, 2, &Status);
}


VOID Cls3errAction(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR        pAddr)
{
	HEADER_802_11         DisassocHdr;
	PHEADER_802_11        pDisassocHdr;
	PUCHAR                pOutBuffer = NULL;
	ULONG                 FrameLen = 0;
	NDIS_STATUS           NStatus;
	USHORT                Reason = REASON_CLS3ERR;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  
	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Class 3 Error, Send DISASSOC frame\n"));
	MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pAddr, pAd->CommonCfg.Bssid);	
	MakeOutgoingFrame(pOutBuffer,           &FrameLen,
					  sizeof(HEADER_802_11),&DisassocHdr,
					  2,                    &Reason,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);

	
	
	
	pDisassocHdr = (PHEADER_802_11)pOutBuffer;
	pDisassocHdr->FC.SubType = SUBTYPE_DEAUTH;
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	pAd->StaCfg.DisassocReason = REASON_CLS3ERR;
	COPY_MAC_ADDR(pAd->StaCfg.DisassocSta, pAddr);
}

 
VOID SwitchBetweenWepAndCkip(
	IN PRTMP_ADAPTER pAd)
{
	int            i;
	SHAREDKEY_MODE_STRUC  csr1;

	
	
	if (pAd->StaCfg.bCkipOn && (pAd->StaCfg.CkipFlag & 0x10))
	{
		
		RTMP_IO_READ32(pAd, SHARED_KEY_MODE_BASE, &csr1.word);
		if (csr1.field.Bss0Key0CipherAlg == CIPHER_WEP64)
			csr1.field.Bss0Key0CipherAlg = CIPHER_CKIP64;
		else if (csr1.field.Bss0Key0CipherAlg == CIPHER_WEP128)
			csr1.field.Bss0Key0CipherAlg = CIPHER_CKIP128;

		if (csr1.field.Bss0Key1CipherAlg == CIPHER_WEP64)
			csr1.field.Bss0Key1CipherAlg = CIPHER_CKIP64;
		else if (csr1.field.Bss0Key1CipherAlg == CIPHER_WEP128)
			csr1.field.Bss0Key1CipherAlg = CIPHER_CKIP128;

		if (csr1.field.Bss0Key2CipherAlg == CIPHER_WEP64)
			csr1.field.Bss0Key2CipherAlg = CIPHER_CKIP64;
		else if (csr1.field.Bss0Key2CipherAlg == CIPHER_WEP128)
			csr1.field.Bss0Key2CipherAlg = CIPHER_CKIP128;

		if (csr1.field.Bss0Key3CipherAlg == CIPHER_WEP64)
			csr1.field.Bss0Key3CipherAlg = CIPHER_CKIP64;
		else if (csr1.field.Bss0Key3CipherAlg == CIPHER_WEP128)
			csr1.field.Bss0Key3CipherAlg = CIPHER_CKIP128;
		RTMP_IO_WRITE32(pAd, SHARED_KEY_MODE_BASE, csr1.word);
		DBGPRINT(RT_DEBUG_TRACE, ("SwitchBetweenWepAndCkip: modify BSS0 cipher to %s\n", CipherName[csr1.field.Bss0Key0CipherAlg]));

		
		for (i=0; i<SHARE_KEY_NUM; i++)
		{
			if (pAd->SharedKey[BSS0][i].CipherAlg == CIPHER_WEP64)
				pAd->SharedKey[BSS0][i].CipherAlg = CIPHER_CKIP64;
			else if (pAd->SharedKey[BSS0][i].CipherAlg == CIPHER_WEP128)
				pAd->SharedKey[BSS0][i].CipherAlg = CIPHER_CKIP128;
		}
	}

	
	
	else
	{
		
		RTMP_IO_READ32(pAd, SHARED_KEY_MODE_BASE, &csr1.word);
		if (csr1.field.Bss0Key0CipherAlg == CIPHER_CKIP64)
			csr1.field.Bss0Key0CipherAlg = CIPHER_WEP64;
		else if (csr1.field.Bss0Key0CipherAlg == CIPHER_CKIP128)
			csr1.field.Bss0Key0CipherAlg = CIPHER_WEP128;

		if (csr1.field.Bss0Key1CipherAlg == CIPHER_CKIP64)
			csr1.field.Bss0Key1CipherAlg = CIPHER_WEP64;
		else if (csr1.field.Bss0Key1CipherAlg == CIPHER_CKIP128)
			csr1.field.Bss0Key1CipherAlg = CIPHER_WEP128;

		if (csr1.field.Bss0Key2CipherAlg == CIPHER_CKIP64)
			csr1.field.Bss0Key2CipherAlg = CIPHER_WEP64;
		else if (csr1.field.Bss0Key2CipherAlg == CIPHER_CKIP128)
			csr1.field.Bss0Key2CipherAlg = CIPHER_WEP128;

		if (csr1.field.Bss0Key3CipherAlg == CIPHER_CKIP64)
			csr1.field.Bss0Key3CipherAlg = CIPHER_WEP64;
		else if (csr1.field.Bss0Key3CipherAlg == CIPHER_CKIP128)
			csr1.field.Bss0Key3CipherAlg = CIPHER_WEP128;

		
		for (i=0; i<SHARE_KEY_NUM; i++)
		{
			if (pAd->SharedKey[BSS0][i].CipherAlg == CIPHER_CKIP64)
				pAd->SharedKey[BSS0][i].CipherAlg = CIPHER_WEP64;
			else if (pAd->SharedKey[BSS0][i].CipherAlg == CIPHER_CKIP128)
				pAd->SharedKey[BSS0][i].CipherAlg = CIPHER_WEP128;
		}

		
		
		
		
		
		
		if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPANone)
		{
			for (i = 0; i < SHARE_KEY_NUM; i++)
			{
				if (pAd->SharedKey[BSS0][i].KeyLen != 0)
				{
					if (pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled)
					{
						pAd->SharedKey[BSS0][i].CipherAlg = CIPHER_TKIP;
					}
					else if (pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled)
					{
						pAd->SharedKey[BSS0][i].CipherAlg = CIPHER_AES;
					}
				}
				else
				{
					pAd->SharedKey[BSS0][i].CipherAlg = CIPHER_NONE;
				}
			}

			csr1.field.Bss0Key0CipherAlg = pAd->SharedKey[BSS0][0].CipherAlg;
			csr1.field.Bss0Key1CipherAlg = pAd->SharedKey[BSS0][1].CipherAlg;
			csr1.field.Bss0Key2CipherAlg = pAd->SharedKey[BSS0][2].CipherAlg;
			csr1.field.Bss0Key3CipherAlg = pAd->SharedKey[BSS0][3].CipherAlg;
		}
		RTMP_IO_WRITE32(pAd, SHARED_KEY_MODE_BASE, csr1.word);
		DBGPRINT(RT_DEBUG_TRACE, ("SwitchBetweenWepAndCkip: modify BSS0 cipher to %s\n", CipherName[csr1.field.Bss0Key0CipherAlg]));
	}
}

int wext_notify_event_assoc(
	IN  RTMP_ADAPTER *pAd)
{
    union iwreq_data    wrqu;
    char custom[IW_CUSTOM_MAX] = {0};

    if (pAd->StaCfg.ReqVarIELen <= IW_CUSTOM_MAX)
    {
        wrqu.data.length = pAd->StaCfg.ReqVarIELen;
        memcpy(custom, pAd->StaCfg.ReqVarIEs, pAd->StaCfg.ReqVarIELen);
        wireless_send_event(pAd->net_dev, IWEVASSOCREQIE, &wrqu, custom);
    }
    else
        DBGPRINT(RT_DEBUG_TRACE, ("pAd->StaCfg.ReqVarIELen > MAX_CUSTOM_LEN\n"));

	return 0;

}

#ifdef RT2870
BOOLEAN StaAddMacTableEntry(
	IN  PRTMP_ADAPTER		pAd,
	IN  PMAC_TABLE_ENTRY	pEntry,
	IN  UCHAR				MaxSupportedRateIn500Kbps,
	IN  HT_CAPABILITY_IE	*pHtCapability,
	IN  UCHAR				HtCapabilityLen,
	IN  USHORT        		CapabilityInfo)
{
	UCHAR            MaxSupportedRate = RATE_11;

	if (ADHOC_ON(pAd))
		CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

	switch (MaxSupportedRateIn500Kbps)
    {
        case 108: MaxSupportedRate = RATE_54;   break;
        case 96:  MaxSupportedRate = RATE_48;   break;
        case 72:  MaxSupportedRate = RATE_36;   break;
        case 48:  MaxSupportedRate = RATE_24;   break;
        case 36:  MaxSupportedRate = RATE_18;   break;
        case 24:  MaxSupportedRate = RATE_12;   break;
        case 18:  MaxSupportedRate = RATE_9;    break;
        case 12:  MaxSupportedRate = RATE_6;    break;
        case 22:  MaxSupportedRate = RATE_11;   break;
        case 11:  MaxSupportedRate = RATE_5_5;  break;
        case 4:   MaxSupportedRate = RATE_2;    break;
        case 2:   MaxSupportedRate = RATE_1;    break;
        default:  MaxSupportedRate = RATE_11;   break;
    }

    if ((pAd->CommonCfg.PhyMode == PHY_11G) && (MaxSupportedRate < RATE_FIRST_OFDM_RATE))
        return FALSE;

	
	if (((pAd->CommonCfg.PhyMode == PHY_11N_2_4G) || (pAd->CommonCfg.PhyMode == PHY_11N_5G))&& (HtCapabilityLen == 0))
		return FALSE;

	if (!pEntry)
        return FALSE;

	NdisAcquireSpinLock(&pAd->MacTabLock);
	if (pEntry)
	{
		pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
		if ((MaxSupportedRate < RATE_FIRST_OFDM_RATE) ||
			(pAd->CommonCfg.PhyMode == PHY_11B))
		{
			pEntry->RateLen = 4;
			if (MaxSupportedRate >= RATE_FIRST_OFDM_RATE)
				MaxSupportedRate = RATE_11;
		}
		else
			pEntry->RateLen = 12;

		pEntry->MaxHTPhyMode.word = 0;
		pEntry->MinHTPhyMode.word = 0;
		pEntry->HTPhyMode.word = 0;
		pEntry->MaxSupportedRate = MaxSupportedRate;
		if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
		{
			pEntry->MaxHTPhyMode.field.MODE = MODE_CCK;
			pEntry->MaxHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
			pEntry->MinHTPhyMode.field.MODE = MODE_CCK;
			pEntry->MinHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
			pEntry->HTPhyMode.field.MODE = MODE_CCK;
			pEntry->HTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		}
		else
		{
			pEntry->MaxHTPhyMode.field.MODE = MODE_OFDM;
			pEntry->MaxHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
			pEntry->MinHTPhyMode.field.MODE = MODE_OFDM;
			pEntry->MinHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
			pEntry->HTPhyMode.field.MODE = MODE_OFDM;
			pEntry->HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		}
		pEntry->CapabilityInfo = CapabilityInfo;
		CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
		CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
	}

	
	if ((HtCapabilityLen != 0) && (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
	{
		UCHAR	j, bitmask; 
		CHAR    i;

		if (ADHOC_ON(pAd))
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
		if ((pHtCapability->HtCapInfo.GF) && (pAd->CommonCfg.DesiredHtPhy.GF))
		{
			pEntry->MaxHTPhyMode.field.MODE = MODE_HTGREENFIELD;
		}
		else
		{
			pEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
			pAd->MacTab.fAnyStationNonGF = TRUE;
			pAd->CommonCfg.AddHTInfo.AddHtInfo2.NonGfPresent = 1;
		}

		if ((pHtCapability->HtCapInfo.ChannelWidth) && (pAd->CommonCfg.DesiredHtPhy.ChannelWidth))
		{
			pEntry->MaxHTPhyMode.field.BW= BW_40;
			pEntry->MaxHTPhyMode.field.ShortGI = ((pAd->CommonCfg.DesiredHtPhy.ShortGIfor40)&(pHtCapability->HtCapInfo.ShortGIfor40));
		}
		else
		{
			pEntry->MaxHTPhyMode.field.BW = BW_20;
			pEntry->MaxHTPhyMode.field.ShortGI = ((pAd->CommonCfg.DesiredHtPhy.ShortGIfor20)&(pHtCapability->HtCapInfo.ShortGIfor20));
			pAd->MacTab.fAnyStation20Only = TRUE;
		}

		
		if (pAd->MACVersion >= RALINK_2883_VERSION && pAd->MACVersion < RALINK_3070_VERSION)
			pEntry->MaxHTPhyMode.field.TxBF = pAd->CommonCfg.RegTransmitSetting.field.TxBF;

		
		for (i=23; i>=0; i--) 
		{
			j = i/8;
			bitmask = (1<<(i-(j*8)));
			if ((pAd->StaCfg.DesiredHtPhyInfo.MCSSet[j] & bitmask) && (pHtCapability->MCSSet[j] & bitmask))
			{
				pEntry->MaxHTPhyMode.field.MCS = i;
				break;
			}
			if (i==0)
				break;
		}


		if (pAd->StaCfg.DesiredTransmitSetting.field.MCS != MCS_AUTO)
		{
			if (pAd->StaCfg.DesiredTransmitSetting.field.MCS == 32)
			{
				
				pEntry->MaxHTPhyMode.field.BW = 1;
				pEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
				pEntry->MaxHTPhyMode.field.STBC = 0;
				pEntry->MaxHTPhyMode.field.ShortGI = 0;
				pEntry->MaxHTPhyMode.field.MCS = 32;
			}
			else if (pEntry->MaxHTPhyMode.field.MCS > pAd->StaCfg.HTPhyMode.field.MCS)
			{
				
				pEntry->MaxHTPhyMode.field.MCS = pAd->StaCfg.HTPhyMode.field.MCS;
			}
		}

		pEntry->MaxHTPhyMode.field.STBC = (pHtCapability->HtCapInfo.RxSTBC & (pAd->CommonCfg.DesiredHtPhy.TxSTBC));
		pEntry->MpduDensity = pHtCapability->HtCapParm.MpduDensity;
		pEntry->MaxRAmpduFactor = pHtCapability->HtCapParm.MaxRAmpduFactor;
		pEntry->MmpsMode = (UCHAR)pHtCapability->HtCapInfo.MimoPs;
		pEntry->AMsduSize = (UCHAR)pHtCapability->HtCapInfo.AMsduSize;
		pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;

		if (pAd->CommonCfg.DesiredHtPhy.AmsduEnable && (pAd->CommonCfg.REGBACapability.field.AutoBA == FALSE))
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_AMSDU_INUSED);
		if (pHtCapability->HtCapInfo.ShortGIfor20)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE);
		if (pHtCapability->HtCapInfo.ShortGIfor40)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE);
		if (pHtCapability->HtCapInfo.TxSTBC)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_TxSTBC_CAPABLE);
		if (pHtCapability->HtCapInfo.RxSTBC)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RxSTBC_CAPABLE);
		if (pHtCapability->ExtHtCapInfo.PlusHTC)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_HTC_CAPABLE);
		if (pAd->CommonCfg.bRdg && pHtCapability->ExtHtCapInfo.RDGSupport)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE);
		if (pHtCapability->ExtHtCapInfo.MCSFeedback == 0x03)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_MCSFEEDBACK_CAPABLE);
	}
	else
	{
		pAd->MacTab.fAnyStationIsLegacy = TRUE;
	}

	NdisMoveMemory(&pEntry->HTCapability, pHtCapability, sizeof(HT_CAPABILITY_IE));

	pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;
	pEntry->CurrTxRate = pEntry->MaxSupportedRate;

	
	if (pAd->StaCfg.bAutoTxRateSwitch == TRUE)
	{
		PUCHAR					pTable;
		UCHAR					TableSize = 0;

		MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &pEntry->CurrTxRateIndex);
		pEntry->bAutoTxRateSwitch = TRUE;
	}
	else
	{
		pEntry->HTPhyMode.field.MODE	= pAd->StaCfg.HTPhyMode.field.MODE;
		pEntry->HTPhyMode.field.MCS	= pAd->StaCfg.HTPhyMode.field.MCS;
		pEntry->bAutoTxRateSwitch = FALSE;

		
		RTMPUpdateLegacyTxSetting((UCHAR)pAd->StaCfg.DesiredTransmitSetting.field.FixedTxMode, pEntry);
	}

	pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
	pEntry->Sst = SST_ASSOC;
	pEntry->AuthState = AS_AUTH_OPEN;
	pEntry->AuthMode = pAd->StaCfg.AuthMode;
	pEntry->WepStatus = pAd->StaCfg.WepStatus;

	NdisReleaseSpinLock(&pAd->MacTabLock);

    {
        union iwreq_data    wrqu;
        wext_notify_event_assoc(pAd);

        memset(wrqu.ap_addr.sa_data, 0, MAC_ADDR_LEN);
        memcpy(wrqu.ap_addr.sa_data, pAd->MlmeAux.Bssid, MAC_ADDR_LEN);
        wireless_send_event(pAd->net_dev, SIOCGIWAP, &wrqu, NULL);

    }
	return TRUE;
}
#endif 
