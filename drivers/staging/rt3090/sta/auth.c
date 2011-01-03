

#include "../rt_config.h"




void AuthStateMachineInit(
    IN PRTMP_ADAPTER pAd,
    IN STATE_MACHINE *Sm,
    OUT STATE_MACHINE_FUNC Trans[])
{
    StateMachineInit(Sm, Trans, MAX_AUTH_STATE, MAX_AUTH_MSG, (STATE_MACHINE_FUNC)Drop, AUTH_REQ_IDLE, AUTH_MACHINE_BASE);

    
    StateMachineSetAction(Sm, AUTH_REQ_IDLE, MT2_MLME_AUTH_REQ, (STATE_MACHINE_FUNC)MlmeAuthReqAction);

    
    StateMachineSetAction(Sm, AUTH_WAIT_SEQ2, MT2_MLME_AUTH_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenAuth);
    StateMachineSetAction(Sm, AUTH_WAIT_SEQ2, MT2_PEER_AUTH_EVEN, (STATE_MACHINE_FUNC)PeerAuthRspAtSeq2Action);
    StateMachineSetAction(Sm, AUTH_WAIT_SEQ2, MT2_AUTH_TIMEOUT, (STATE_MACHINE_FUNC)AuthTimeoutAction);

    
    StateMachineSetAction(Sm, AUTH_WAIT_SEQ4, MT2_MLME_AUTH_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenAuth);
    StateMachineSetAction(Sm, AUTH_WAIT_SEQ4, MT2_PEER_AUTH_EVEN, (STATE_MACHINE_FUNC)PeerAuthRspAtSeq4Action);
    StateMachineSetAction(Sm, AUTH_WAIT_SEQ4, MT2_AUTH_TIMEOUT, (STATE_MACHINE_FUNC)AuthTimeoutAction);

	RTMPInitTimer(pAd, &pAd->MlmeAux.AuthTimer, GET_TIMER_FUNCTION(AuthTimeout), pAd, FALSE);
}


VOID AuthTimeout(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3)
{
    RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

    DBGPRINT(RT_DEBUG_TRACE,("AUTH - AuthTimeout\n"));

	
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return;

	
	if (pAd->Mlme.AuthMachine.CurrState == AUTH_WAIT_SEQ2)
		Cls2errAction(pAd, pAd->MlmeAux.Bssid);


    MlmeEnqueue(pAd, AUTH_STATE_MACHINE, MT2_AUTH_TIMEOUT, 0, NULL);
    RTMP_MLME_HANDLER(pAd);
}



VOID MlmeAuthReqAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
	if (AUTH_ReqSend(pAd, Elem, &pAd->MlmeAux.AuthTimer, "AUTH", 1, NULL, 0))
        pAd->Mlme.AuthMachine.CurrState = AUTH_WAIT_SEQ2;
    else
    {
		USHORT Status;

        pAd->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
        Status = MLME_INVALID_FORMAT;
        MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status);
    }
}


VOID PeerAuthRspAtSeq2Action(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    UCHAR         Addr2[MAC_ADDR_LEN];
    USHORT        Seq, Status, RemoteStatus, Alg;
    UCHAR         ChlgText[CIPHER_TEXT_LEN];
    UCHAR         CyperChlgText[CIPHER_TEXT_LEN + 8 + 8];
    UCHAR         Element[2];
    HEADER_802_11 AuthHdr;
    BOOLEAN       TimerCancelled;
    PUCHAR        pOutBuffer = NULL;
    NDIS_STATUS   NStatus;
    ULONG         FrameLen = 0;
    USHORT        Status2;

    if (PeerAuthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Alg, &Seq, &Status, (PCHAR)ChlgText))
    {
        if (MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, Addr2) && Seq == 2)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Receive AUTH_RSP seq#2 to me (Alg=%d, Status=%d)\n", Alg, Status));
            RTMPCancelTimer(&pAd->MlmeAux.AuthTimer, &TimerCancelled);

            if (Status == MLME_SUCCESS)
            {
                
                if (pAd->MlmeAux.Alg == Ndis802_11AuthModeOpen)
                {
                    pAd->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
                    MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status);
                }
                else
                {
                    
                    Seq++;
                    RemoteStatus = MLME_SUCCESS;

					
                    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
                    if(NStatus != NDIS_STATUS_SUCCESS)
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("AUTH - PeerAuthRspAtSeq2Action() allocate memory fail\n"));
                        pAd->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
                        Status2 = MLME_FAIL_NO_RESOURCE;
                        MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status2);
                        return;
                    }

                    DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Send AUTH request seq#3...\n"));
                    MgtMacHeaderInit(pAd, &AuthHdr, SUBTYPE_AUTH, 0, Addr2, pAd->MlmeAux.Bssid);
                    AuthHdr.FC.Wep = 1;
                    
                    RTMPInitWepEngine(
			pAd,
			pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].Key,
			pAd->StaCfg.DefaultKeyId,
			pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].KeyLen,
			CyperChlgText);

					Alg = cpu2le16(*(USHORT *)&Alg);
					Seq = cpu2le16(*(USHORT *)&Seq);
					RemoteStatus= cpu2le16(*(USHORT *)&RemoteStatus);

					RTMPEncryptData(pAd, (PUCHAR) &Alg, CyperChlgText + 4, 2);
					RTMPEncryptData(pAd, (PUCHAR) &Seq, CyperChlgText + 6, 2);
					RTMPEncryptData(pAd, (PUCHAR) &RemoteStatus, CyperChlgText + 8, 2);
					Element[0] = 16;
					Element[1] = 128;
					RTMPEncryptData(pAd, Element, CyperChlgText + 10, 2);
					RTMPEncryptData(pAd, ChlgText, CyperChlgText + 12, 128);
					RTMPSetICV(pAd, CyperChlgText + 140);
                    MakeOutgoingFrame(pOutBuffer,               &FrameLen,
                                      sizeof(HEADER_802_11),    &AuthHdr,
                                      CIPHER_TEXT_LEN + 16,     CyperChlgText,
                                      END_OF_ARGS);
                    MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
			MlmeFreeMemory(pAd, pOutBuffer);

                    RTMPSetTimer(&pAd->MlmeAux.AuthTimer, AUTH_TIMEOUT);
                    pAd->Mlme.AuthMachine.CurrState = AUTH_WAIT_SEQ4;
                }
            }
            else
            {
                pAd->StaCfg.AuthFailReason = Status;
                COPY_MAC_ADDR(pAd->StaCfg.AuthFailSta, Addr2);
                pAd->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
                MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status);
            }
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("AUTH - PeerAuthSanity() sanity check fail\n"));
    }
}


VOID PeerAuthRspAtSeq4Action(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    UCHAR         Addr2[MAC_ADDR_LEN];
    USHORT        Alg, Seq, Status;
    CHAR          ChlgText[CIPHER_TEXT_LEN];
    BOOLEAN       TimerCancelled;

    if(PeerAuthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Alg, &Seq, &Status, ChlgText))
    {
        if(MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, Addr2) && Seq == 4)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Receive AUTH_RSP seq#4 to me\n"));
            RTMPCancelTimer(&pAd->MlmeAux.AuthTimer, &TimerCancelled);

            if (Status != MLME_SUCCESS)
            {
                pAd->StaCfg.AuthFailReason = Status;
                COPY_MAC_ADDR(pAd->StaCfg.AuthFailSta, Addr2);
            }

            pAd->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
            MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status);
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("AUTH - PeerAuthRspAtSeq4Action() sanity check fail\n"));
    }
}


VOID MlmeDeauthReqAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    MLME_DEAUTH_REQ_STRUCT *pInfo;
    HEADER_802_11 DeauthHdr;
    PUCHAR        pOutBuffer = NULL;
    NDIS_STATUS   NStatus;
    ULONG         FrameLen = 0;
    USHORT        Status;

    pInfo = (MLME_DEAUTH_REQ_STRUCT *)Elem->Msg;

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  
    if (NStatus != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("AUTH - MlmeDeauthReqAction() allocate memory fail\n"));
        pAd->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
        Status = MLME_FAIL_NO_RESOURCE;
        MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_DEAUTH_CONF, 2, &Status);
        return;
    }


    DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Send DE-AUTH request (Reason=%d)...\n", pInfo->Reason));
    MgtMacHeaderInit(pAd, &DeauthHdr, SUBTYPE_DEAUTH, 0, pInfo->Addr, pAd->MlmeAux.Bssid);
    MakeOutgoingFrame(pOutBuffer,           &FrameLen,
                      sizeof(HEADER_802_11),&DeauthHdr,
                      2,                    &pInfo->Reason,
                      END_OF_ARGS);
    MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

    pAd->StaCfg.DeauthReason = pInfo->Reason;
    COPY_MAC_ADDR(pAd->StaCfg.DeauthSta, pInfo->Addr);
    pAd->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
    Status = MLME_SUCCESS;
    MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_DEAUTH_CONF, 2, &Status);

	
	if (pAd->CommonCfg.bWirelessEvent)
		RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
}


VOID AuthTimeoutAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    USHORT Status;
    DBGPRINT(RT_DEBUG_TRACE, ("AUTH - AuthTimeoutAction\n"));
    pAd->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
    Status = MLME_REJ_TIMEOUT;
    MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status);
}


VOID InvalidStateWhenAuth(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
    USHORT Status;
    DBGPRINT(RT_DEBUG_TRACE, ("AUTH - InvalidStateWhenAuth (state=%ld), reset AUTH state machine\n", pAd->Mlme.AuthMachine.CurrState));
    pAd->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
    Status = MLME_STATE_MACHINE_REJECT;
    MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status);
}


VOID Cls2errAction(
    IN PRTMP_ADAPTER pAd,
    IN PUCHAR pAddr)
{
    HEADER_802_11 DeauthHdr;
    PUCHAR        pOutBuffer = NULL;
    NDIS_STATUS   NStatus;
    ULONG         FrameLen = 0;
    USHORT        Reason = REASON_CLS2ERR;

    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  
    if (NStatus != NDIS_STATUS_SUCCESS)
        return;

    DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Class 2 error, Send DEAUTH frame...\n"));
    MgtMacHeaderInit(pAd, &DeauthHdr, SUBTYPE_DEAUTH, 0, pAddr, pAd->MlmeAux.Bssid);
    MakeOutgoingFrame(pOutBuffer,           &FrameLen,
                      sizeof(HEADER_802_11),&DeauthHdr,
                      2,                    &Reason,
                      END_OF_ARGS);
    MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

    pAd->StaCfg.DeauthReason = Reason;
    COPY_MAC_ADDR(pAd->StaCfg.DeauthSta, pAddr);
}

BOOLEAN	AUTH_ReqSend(
	IN  PRTMP_ADAPTER		pAd,
	IN  PMLME_QUEUE_ELEM	pElem,
	IN  PRALINK_TIMER_STRUCT pAuthTimer,
	IN  PSTRING				pSMName,
	IN  USHORT				SeqNo,
	IN  PUCHAR				pNewElement,
	IN  ULONG				ElementLen)
{
	USHORT             Alg, Seq, Status;
	UCHAR              Addr[6];
    ULONG              Timeout;
    HEADER_802_11      AuthHdr;
    BOOLEAN            TimerCancelled;
    NDIS_STATUS        NStatus;
    PUCHAR             pOutBuffer = NULL;
    ULONG              FrameLen = 0, tmp = 0;

	
	if (pAd->StaCfg.bBlockAssoc == TRUE)
	{
        DBGPRINT(RT_DEBUG_TRACE, ("%s - Block Auth request durning WPA block period!\n", pSMName));
        pAd->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
        Status = MLME_STATE_MACHINE_REJECT;
        MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status);
	}
    else if(MlmeAuthReqSanity(pAd, pElem->Msg, pElem->MsgLen, Addr, &Timeout, &Alg))
    {
	
		RTMPCancelTimer(pAuthTimer, &TimerCancelled);

        COPY_MAC_ADDR(pAd->MlmeAux.Bssid, Addr);
        pAd->MlmeAux.Alg  = Alg;
        Seq = SeqNo;
        Status = MLME_SUCCESS;

        NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  
        if(NStatus != NDIS_STATUS_SUCCESS)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("%s - MlmeAuthReqAction(Alg:%d) allocate memory failed\n", pSMName, Alg));
            pAd->Mlme.AuthMachine.CurrState = AUTH_REQ_IDLE;
            Status = MLME_FAIL_NO_RESOURCE;
            MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status);
            return FALSE;
        }

        DBGPRINT(RT_DEBUG_TRACE, ("%s - Send AUTH request seq#1 (Alg=%d)...\n", pSMName, Alg));
        MgtMacHeaderInit(pAd, &AuthHdr, SUBTYPE_AUTH, 0, Addr, pAd->MlmeAux.Bssid);
        MakeOutgoingFrame(pOutBuffer,           &FrameLen,
                          sizeof(HEADER_802_11),&AuthHdr,
                          2,                    &Alg,
                          2,                    &Seq,
                          2,                    &Status,
                          END_OF_ARGS);

		if (pNewElement && ElementLen)
		{
			MakeOutgoingFrame(pOutBuffer+FrameLen,	&tmp,
							  ElementLen,			pNewElement,
				  END_OF_ARGS);
			FrameLen += tmp;
		}

        MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

		RTMPSetTimer(pAuthTimer, Timeout);
		return TRUE;
    }
    else
    {
        DBGPRINT_ERR(("%s - MlmeAuthReqAction() sanity check failed\n", pSMName));
		return FALSE;
    }

	return TRUE;
}
