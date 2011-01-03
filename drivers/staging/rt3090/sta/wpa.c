

#include "../rt_config.h"


void inc_byte_array(UCHAR *counter, int len);


VOID	RTMPReportMicError(
	IN	PRTMP_ADAPTER	pAd,
	IN	PCIPHER_KEY	pWpaKey)
{
	ULONG	Now;
    UCHAR   unicastKey = (pWpaKey->Type == PAIRWISE_KEY ? 1:0);

	
	NdisGetSystemUpTime(&Now);
	if (pAd->StaCfg.MicErrCnt == 0)
	{
		pAd->StaCfg.MicErrCnt++;
		pAd->StaCfg.LastMicErrorTime = Now;
        NdisZeroMemory(pAd->StaCfg.ReplayCounter, 8);
	}
	else if (pAd->StaCfg.MicErrCnt == 1)
	{
		if ((pAd->StaCfg.LastMicErrorTime + (60 * OS_HZ)) < Now)
		{
			
			pAd->StaCfg.LastMicErrorTime = Now;
		}
		else
		{

			if (pAd->CommonCfg.bWirelessEvent)
				RTMPSendWirelessEvent(pAd, IW_COUNTER_MEASURES_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);

			pAd->StaCfg.LastMicErrorTime = Now;
			
			pAd->StaCfg.MicErrCnt++;
			
			
			
			
			
			
			
			
			
			
			
		}
	}
	else
	{
		
		
		;
	}
    MlmeEnqueue(pAd,
				MLME_CNTL_STATE_MACHINE,
				OID_802_11_MIC_FAILURE_REPORT_FRAME,
				1,
				&unicastKey);

    if (pAd->StaCfg.MicErrCnt == 2)
    {
        RTMPSetTimer(&pAd->StaCfg.WpaDisassocAndBlockAssocTimer, 100);
    }
}


#ifdef WPA_SUPPLICANT_SUPPORT
#define	LENGTH_EAP_H    4

INT	    WpaCheckEapCode(
	IN  PRTMP_ADAPTER		pAd,
	IN  PUCHAR				pFrame,
	IN  USHORT				FrameLen,
	IN  USHORT				OffSet)
{

	PUCHAR	pData;
	INT	result = 0;

	if( FrameLen < OffSet + LENGTH_EAPOL_H + LENGTH_EAP_H )
		return result;

	pData = pFrame + OffSet; 

	if(*(pData+1) == EAPPacket)	
	{
		 result = *(pData+4);		
	}

	return result;
}

VOID    WpaSendMicFailureToWpaSupplicant(
    IN  PRTMP_ADAPTER    pAd,
    IN  BOOLEAN          bUnicast)
{
	char custom[IW_CUSTOM_MAX] = {0};

	sprintf(custom, "MLME-MICHAELMICFAILURE.indication");
	if(bUnicast)
		sprintf(custom, "%s unicast", custom);

	RtmpOSWrielessEventSend(pAd, IWEVCUSTOM, -1, NULL, (PUCHAR)custom, strlen(custom));

	return;
}
#endif 

VOID	WpaMicFailureReportFrame(
	IN  PRTMP_ADAPTER   pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PUCHAR              pOutBuffer = NULL;
	UCHAR               Header802_3[14];
	ULONG               FrameLen = 0;
	EAPOL_PACKET        Packet;
	UCHAR               Mic[16];
    BOOLEAN             bUnicast;

	DBGPRINT(RT_DEBUG_TRACE, ("WpaMicFailureReportFrame ----->\n"));

    bUnicast = (Elem->Msg[0] == 1 ? TRUE:FALSE);
	pAd->Sequence = ((pAd->Sequence) + 1) & (MAX_SEQ_NUMBER);

	
	MAKE_802_3_HEADER(Header802_3, pAd->CommonCfg.Bssid, pAd->CurrentAddress, EAPOL);

	NdisZeroMemory(&Packet, sizeof(Packet));
	Packet.ProVer	= EAPOL_VER;
	Packet.ProType	= EAPOLKey;

	Packet.KeyDesc.Type = WPA1_KEY_DESC;

    
    Packet.KeyDesc.KeyInfo.Request = 1;

	if(pAd->StaCfg.WepStatus  == Ndis802_11Encryption3Enabled)
	{
		Packet.KeyDesc.KeyInfo.KeyDescVer = 2;
	}
	else	  
	{
		Packet.KeyDesc.KeyInfo.KeyDescVer = 1;
	}

    Packet.KeyDesc.KeyInfo.KeyType = (bUnicast ? PAIRWISEKEY : GROUPKEY);

	
	Packet.KeyDesc.KeyInfo.KeyMic  = 1;

    
	Packet.KeyDesc.KeyInfo.Error  = 1;

	
	SET_UINT16_TO_ARRARY(Packet.Body_Len, LEN_EAPOL_KEY_MSG)

	
	NdisMoveMemory(Packet.KeyDesc.ReplayCounter, pAd->StaCfg.ReplayCounter, LEN_KEY_DESC_REPLAY);
    inc_byte_array(pAd->StaCfg.ReplayCounter, 8);

	
	*((USHORT *)&Packet.KeyDesc.KeyInfo) = cpu2le16(*((USHORT *)&Packet.KeyDesc.KeyInfo));


	MlmeAllocateMemory(pAd, (PUCHAR *)&pOutBuffer);  
	if(pOutBuffer == NULL)
	{
		return;
	}

	
	
	MakeOutgoingFrame(pOutBuffer,               &FrameLen,
		              CONV_ARRARY_TO_UINT16(Packet.Body_Len) + 4,   &Packet,
		              END_OF_ARGS);

	
	NdisZeroMemory(Mic, sizeof(Mic));
	if(pAd->StaCfg.WepStatus  == Ndis802_11Encryption3Enabled)
	{	
        UCHAR digest[20] = {0};
		HMAC_SHA1(pAd->StaCfg.PTK, LEN_EAP_MICK, pOutBuffer, FrameLen, digest, SHA1_DIGEST_SIZE);
		NdisMoveMemory(Mic, digest, LEN_KEY_DESC_MIC);
	}
	else
	{	
		HMAC_MD5(pAd->StaCfg.PTK,  LEN_EAP_MICK, pOutBuffer, FrameLen, Mic, MD5_DIGEST_SIZE);
	}
	NdisMoveMemory(Packet.KeyDesc.KeyMic, Mic, LEN_KEY_DESC_MIC);

	
	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[BSSID_WCID],
					  Header802_3, LENGTH_802_3,
					  (PUCHAR)&Packet,
					  CONV_ARRARY_TO_UINT16(Packet.Body_Len) + 4, FALSE);

	MlmeFreeMemory(pAd, (PUCHAR)pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE, ("WpaMicFailureReportFrame <-----\n"));
}


void inc_byte_array(UCHAR *counter, int len)
{
	int pos = len - 1;
	while (pos >= 0) {
		counter[pos]++;
		if (counter[pos] != 0)
			break;
		pos--;
	}
}

VOID WpaDisassocApAndBlockAssoc(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3)
{
    RTMP_ADAPTER                *pAd = (PRTMP_ADAPTER)FunctionContext;
    MLME_DISASSOC_REQ_STRUCT    DisassocReq;

	
	DBGPRINT(RT_DEBUG_TRACE, ("RTMPReportMicError - disassociate with current AP after sending second continuous EAPOL frame\n"));
	DisassocParmFill(pAd, &DisassocReq, pAd->CommonCfg.Bssid, REASON_MIC_FAILURE);
	MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ, sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);

	pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
	pAd->StaCfg.bBlockAssoc = TRUE;
}

VOID WpaStaPairwiseKeySetting(
	IN	PRTMP_ADAPTER	pAd)
{
	PCIPHER_KEY pSharedKey;
	PMAC_TABLE_ENTRY pEntry;

	pEntry = &pAd->MacTab.Content[BSSID_WCID];

	
	pSharedKey = &pAd->SharedKey[BSS0][0];

	NdisMoveMemory(pAd->StaCfg.PTK, pEntry->PTK, LEN_PTK);

	
	NdisZeroMemory(pSharedKey, sizeof(CIPHER_KEY));
	pSharedKey->KeyLen = LEN_TKIP_EK;
    NdisMoveMemory(pSharedKey->Key, &pAd->StaCfg.PTK[32], LEN_TKIP_EK);
	NdisMoveMemory(pSharedKey->RxMic, &pAd->StaCfg.PTK[48], LEN_TKIP_RXMICK);
	NdisMoveMemory(pSharedKey->TxMic, &pAd->StaCfg.PTK[48+LEN_TKIP_RXMICK], LEN_TKIP_TXMICK);

	
	if (pAd->StaCfg.PairCipher == Ndis802_11Encryption2Enabled)
		pSharedKey->CipherAlg = CIPHER_TKIP;
	else if (pAd->StaCfg.PairCipher == Ndis802_11Encryption3Enabled)
		pSharedKey->CipherAlg = CIPHER_AES;
	else
		pSharedKey->CipherAlg = CIPHER_NONE;

	
	NdisMoveMemory(pEntry->PairwiseKey.Key, &pAd->StaCfg.PTK[32], LEN_TKIP_EK);
	NdisMoveMemory(pEntry->PairwiseKey.RxMic, &pAd->StaCfg.PTK[48], LEN_TKIP_RXMICK);
	NdisMoveMemory(pEntry->PairwiseKey.TxMic, &pAd->StaCfg.PTK[48+LEN_TKIP_RXMICK], LEN_TKIP_TXMICK);
	pEntry->PairwiseKey.CipherAlg = pSharedKey->CipherAlg;

	
	AsicAddSharedKeyEntry(pAd,
						  BSS0,
						  0,
						  pSharedKey->CipherAlg,
						  pSharedKey->Key,
						  pSharedKey->TxMic,
						  pSharedKey->RxMic);

	
	RTMPAddWcidAttributeEntry(pAd,
							  BSS0,
							  0,
							  pSharedKey->CipherAlg,
							  pEntry);
	STA_PORT_SECURED(pAd);
	pAd->IndicateMediaState = NdisMediaStateConnected;

	DBGPRINT(RT_DEBUG_TRACE, ("%s : AID(%d) port secured\n", __FUNCTION__, pEntry->Aid));

}

VOID WpaStaGroupKeySetting(
	IN	PRTMP_ADAPTER	pAd)
{
	PCIPHER_KEY		pSharedKey;

	pSharedKey = &pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId];

	
	NdisZeroMemory(pSharedKey, sizeof(CIPHER_KEY));
	pSharedKey->KeyLen = LEN_TKIP_EK;
	NdisMoveMemory(pSharedKey->Key, pAd->StaCfg.GTK, LEN_TKIP_EK);
	NdisMoveMemory(pSharedKey->RxMic, &pAd->StaCfg.GTK[16], LEN_TKIP_RXMICK);
	NdisMoveMemory(pSharedKey->TxMic, &pAd->StaCfg.GTK[24], LEN_TKIP_TXMICK);

	
	pSharedKey->CipherAlg = CIPHER_NONE;
	if (pAd->StaCfg.GroupCipher == Ndis802_11Encryption2Enabled)
		pSharedKey->CipherAlg = CIPHER_TKIP;
	else if (pAd->StaCfg.GroupCipher == Ndis802_11Encryption3Enabled)
		pSharedKey->CipherAlg = CIPHER_AES;
	else if (pAd->StaCfg.GroupCipher == Ndis802_11GroupWEP40Enabled)
		pSharedKey->CipherAlg = CIPHER_WEP64;
	else if (pAd->StaCfg.GroupCipher == Ndis802_11GroupWEP104Enabled)
		pSharedKey->CipherAlg = CIPHER_WEP128;

	
	AsicAddSharedKeyEntry(pAd,
						  BSS0,
						  pAd->StaCfg.DefaultKeyId,
						  pSharedKey->CipherAlg,
						  pSharedKey->Key,
						  pSharedKey->TxMic,
						  pSharedKey->RxMic);

	
	RTMPAddWcidAttributeEntry(pAd,
							  BSS0,
							  pAd->StaCfg.DefaultKeyId,
							  pSharedKey->CipherAlg,
							  NULL);

}
