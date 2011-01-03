

#include "../rt_config.h"



UCHAR		OUI_WPA_NONE_AKM[4]		= {0x00, 0x50, 0xF2, 0x00};
UCHAR       OUI_WPA_VERSION[4]      = {0x00, 0x50, 0xF2, 0x01};
UCHAR       OUI_WPA_WEP40[4]      = {0x00, 0x50, 0xF2, 0x01};
UCHAR       OUI_WPA_TKIP[4]     = {0x00, 0x50, 0xF2, 0x02};
UCHAR       OUI_WPA_CCMP[4]     = {0x00, 0x50, 0xF2, 0x04};
UCHAR       OUI_WPA_WEP104[4]      = {0x00, 0x50, 0xF2, 0x05};
UCHAR       OUI_WPA_8021X_AKM[4]	= {0x00, 0x50, 0xF2, 0x01};
UCHAR       OUI_WPA_PSK_AKM[4]      = {0x00, 0x50, 0xF2, 0x02};

UCHAR       OUI_WPA2_WEP40[4]   = {0x00, 0x0F, 0xAC, 0x01};
UCHAR       OUI_WPA2_TKIP[4]        = {0x00, 0x0F, 0xAC, 0x02};
UCHAR       OUI_WPA2_CCMP[4]        = {0x00, 0x0F, 0xAC, 0x04};
UCHAR       OUI_WPA2_8021X_AKM[4]   = {0x00, 0x0F, 0xAC, 0x01};
UCHAR       OUI_WPA2_PSK_AKM[4]		= {0x00, 0x0F, 0xAC, 0x02};
UCHAR       OUI_WPA2_WEP104[4]   = {0x00, 0x0F, 0xAC, 0x05};



static VOID	ConstructEapolKeyData(
	IN	PMAC_TABLE_ENTRY	pEntry,
	IN	UCHAR			GroupKeyWepStatus,
	IN	UCHAR			keyDescVer,
	IN	UCHAR			MsgType,
	IN	UCHAR			DefaultKeyIdx,
	IN	UCHAR			*GTK,
	IN	UCHAR			*RSNIE,
	IN	UCHAR			RSNIE_LEN,
	OUT PEAPOL_PACKET   pMsg);

static VOID	CalculateMIC(
	IN	UCHAR			KeyDescVer,
	IN	UCHAR			*PTK,
	OUT PEAPOL_PACKET   pMsg);

static VOID WpaEAPPacketAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

static VOID WpaEAPOLASFAlertAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

static VOID WpaEAPOLLogoffAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

static VOID WpaEAPOLStartAction(
    IN PRTMP_ADAPTER    pAd,
    IN MLME_QUEUE_ELEM  *Elem);

static VOID WpaEAPOLKeyAction(
    IN PRTMP_ADAPTER    pAd,
    IN MLME_QUEUE_ELEM  *Elem);


VOID WpaStateMachineInit(
    IN  PRTMP_ADAPTER   pAd,
    IN  STATE_MACHINE *S,
    OUT STATE_MACHINE_FUNC Trans[])
{
    StateMachineInit(S, (STATE_MACHINE_FUNC *)Trans, MAX_WPA_PTK_STATE, MAX_WPA_MSG, (STATE_MACHINE_FUNC)Drop, WPA_PTK, WPA_MACHINE_BASE);

    StateMachineSetAction(S, WPA_PTK, MT2_EAPPacket, (STATE_MACHINE_FUNC)WpaEAPPacketAction);
    StateMachineSetAction(S, WPA_PTK, MT2_EAPOLStart, (STATE_MACHINE_FUNC)WpaEAPOLStartAction);
    StateMachineSetAction(S, WPA_PTK, MT2_EAPOLLogoff, (STATE_MACHINE_FUNC)WpaEAPOLLogoffAction);
    StateMachineSetAction(S, WPA_PTK, MT2_EAPOLKey, (STATE_MACHINE_FUNC)WpaEAPOLKeyAction);
    StateMachineSetAction(S, WPA_PTK, MT2_EAPOLASFAlert, (STATE_MACHINE_FUNC)WpaEAPOLASFAlertAction);
}


VOID WpaEAPPacketAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
}

VOID WpaEAPOLASFAlertAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
}

VOID WpaEAPOLLogoffAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
}


VOID WpaEAPOLStartAction(
    IN PRTMP_ADAPTER    pAd,
    IN MLME_QUEUE_ELEM  *Elem)
{
    MAC_TABLE_ENTRY     *pEntry;
    PHEADER_802_11      pHeader;

    DBGPRINT(RT_DEBUG_TRACE, ("WpaEAPOLStartAction ===> \n"));

    pHeader = (PHEADER_802_11)Elem->Msg;

    
    if (Elem->MsgLen == 6)
        pEntry = MacTableLookup(pAd, Elem->Msg);
    else
    {
        pEntry = MacTableLookup(pAd, pHeader->Addr2);
    }

    if (pEntry)
    {
		DBGPRINT(RT_DEBUG_TRACE, (" PortSecured(%d), WpaState(%d), AuthMode(%d), PMKID_CacheIdx(%d) \n", pEntry->PortSecured, pEntry->WpaState, pEntry->AuthMode, pEntry->PMKID_CacheIdx));

        if ((pEntry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
			&& (pEntry->WpaState < AS_PTKSTART)
            && ((pEntry->AuthMode == Ndis802_11AuthModeWPAPSK) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK) || ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) && (pEntry->PMKID_CacheIdx != ENTRY_NOT_FOUND))))
        {
            pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
            pEntry->WpaState = AS_INITPSK;
            pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
            NdisZeroMemory(pEntry->R_Counter, sizeof(pEntry->R_Counter));
            pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;

            WPAStart4WayHS(pAd, pEntry, PEER_MSG1_RETRY_EXEC_INTV);
        }
    }
}


VOID WpaEAPOLKeyAction(
    IN PRTMP_ADAPTER    pAd,
    IN MLME_QUEUE_ELEM  *Elem)
{
    MAC_TABLE_ENTRY     *pEntry;
    PHEADER_802_11      pHeader;
    PEAPOL_PACKET       pEapol_packet;
	KEY_INFO			peerKeyInfo;

    DBGPRINT(RT_DEBUG_TRACE, ("WpaEAPOLKeyAction ===>\n"));

    pHeader = (PHEADER_802_11)Elem->Msg;
    pEapol_packet = (PEAPOL_PACKET)&Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];

	NdisZeroMemory((PUCHAR)&peerKeyInfo, sizeof(peerKeyInfo));
	NdisMoveMemory((PUCHAR)&peerKeyInfo, (PUCHAR)&pEapol_packet->KeyDesc.KeyInfo, sizeof(KEY_INFO));

	hex_dump("Received Eapol frame", (unsigned char *)pEapol_packet, (Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H));

	*((USHORT *)&peerKeyInfo) = cpu2le16(*((USHORT *)&peerKeyInfo));

    do
    {
        pEntry = MacTableLookup(pAd, pHeader->Addr2);

		if (!pEntry || ((!pEntry->ValidAsCLI) && (!pEntry->ValidAsApCli)))
            break;

		if (pEntry->AuthMode < Ndis802_11AuthModeWPA)
				break;

		DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPoL-Key frame from STA %02X-%02X-%02X-%02X-%02X-%02X\n", PRINT_MAC(pEntry->Addr)));

        if (((pEapol_packet->ProVer != EAPOL_VER) && (pEapol_packet->ProVer != EAPOL_VER2)) ||
			((pEapol_packet->KeyDesc.Type != WPA1_KEY_DESC) && (pEapol_packet->KeyDesc.Type != WPA2_KEY_DESC)))
        {
            DBGPRINT(RT_DEBUG_ERROR, ("Key descripter does not match with WPA rule\n"));
            break;
        }

		
		
		if ((pEntry->WepStatus == Ndis802_11Encryption2Enabled) && (peerKeyInfo.KeyDescVer != DESC_TYPE_TKIP))
        {
	        DBGPRINT(RT_DEBUG_ERROR, ("Key descripter version not match(TKIP) \n"));
	    break;
	}
		
		
	else if ((pEntry->WepStatus == Ndis802_11Encryption3Enabled) && (peerKeyInfo.KeyDescVer != DESC_TYPE_AES))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Key descripter version not match(AES) \n"));
		break;
	}

		
        if ((pEntry->Sst == SST_ASSOC) && (pEntry->WpaState >= AS_INITPSK))
        {
			
			
			
			
			if (peerKeyInfo.KeyAck == 1)
			{
				
				

				if ((peerKeyInfo.Secure == 0) && (peerKeyInfo.Request == 0) &&
					(peerKeyInfo.Error == 0) && (peerKeyInfo.KeyType == PAIRWISEKEY))
				{
					
					
					
					
					if (peerKeyInfo.KeyMic == 0)
			PeerPairMsg1Action(pAd, pEntry, Elem);
	                else
	                PeerPairMsg3Action(pAd, pEntry, Elem);
				}
				else if ((peerKeyInfo.Secure == 1) &&
						 (peerKeyInfo.KeyMic == 1) &&
						 (peerKeyInfo.Request == 0) &&
						 (peerKeyInfo.Error == 0))
				{
					
					
					
					
					if (peerKeyInfo.KeyType == PAIRWISEKEY)
						PeerPairMsg3Action(pAd, pEntry, Elem);
					else
						PeerGroupMsg1Action(pAd, pEntry, Elem);
				}
			}
			else
			{
				
				
				if ((peerKeyInfo.Request == 0) &&
						 (peerKeyInfo.Error == 0) &&
						 (peerKeyInfo.KeyMic == 1))
				{
					if (peerKeyInfo.Secure == 0 && peerKeyInfo.KeyType == PAIRWISEKEY)
					{
						
						
						
						if (CONV_ARRARY_TO_UINT16(pEapol_packet->KeyDesc.KeyDataLen) == 0)
						{
							PeerPairMsg4Action(pAd, pEntry, Elem);
			}
						else
						{
							PeerPairMsg2Action(pAd, pEntry, Elem);
						}
					}
					else if (peerKeyInfo.Secure == 1 && peerKeyInfo.KeyType == PAIRWISEKEY)
					{
						
						
						PeerPairMsg4Action(pAd, pEntry, Elem);
					}
					else if (peerKeyInfo.Secure == 1 && peerKeyInfo.KeyType == GROUPKEY)
					{
						
						
						PeerGroupMsg2Action(pAd, pEntry, &Elem->Msg[LENGTH_802_11], (Elem->MsgLen - LENGTH_802_11));
					}
				}
			}
        }
    }while(FALSE);
}


VOID    RTMPToWirelessSta(
    IN  PRTMP_ADAPTER		pAd,
    IN  PMAC_TABLE_ENTRY	pEntry,
    IN  PUCHAR			pHeader802_3,
    IN  UINT			HdrLen,
    IN  PUCHAR			pData,
    IN  UINT			DataLen,
    IN	BOOLEAN				bClearFrame)
{
    PNDIS_PACKET    pPacket;
    NDIS_STATUS     Status;

	if ((!pEntry) || ((!pEntry->ValidAsCLI) && (!pEntry->ValidAsApCli)))
		return;

    do {
		
		Status = RTMPAllocateNdisPacket(pAd, &pPacket, pHeader802_3, HdrLen, pData, DataLen);
		if (Status != NDIS_STATUS_SUCCESS)
		break;


			if (bClearFrame)
				RTMP_SET_PACKET_CLEAR_EAP_FRAME(pPacket, 1);
			else
				RTMP_SET_PACKET_CLEAR_EAP_FRAME(pPacket, 0);
		{
			RTMP_SET_PACKET_SOURCE(pPacket, PKTSRC_NDIS);

			RTMP_SET_PACKET_NET_DEVICE_MBSSID(pPacket, MAIN_MBSSID);	
			if(pEntry->apidx != 0)
			RTMP_SET_PACKET_NET_DEVICE_MBSSID(pPacket, pEntry->apidx);

		RTMP_SET_PACKET_WCID(pPacket, (UCHAR)pEntry->Aid);
			RTMP_SET_PACKET_MOREDATA(pPacket, FALSE);
		}

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
		    
	        Status = STASendPacket(pAd, pPacket);
	    if (Status == NDIS_STATUS_SUCCESS)
			{
				UCHAR   Index;

				
				
				
				
				
				if((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) &&
					(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)))
				{
					for(Index = 0; Index < 5; Index ++)
						if(pAd->TxSwQueue[Index].Number > 0)
							RTMPDeQueuePacket(pAd, FALSE, Index, MAX_TX_PROCESS);
				}
			}
		}
#endif 

    } while (FALSE);
}


VOID WPAStart4WayHS(
    IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN ULONG			TimeInterval)
{
    UCHAR           Header802_3[14];
    EAPOL_PACKET	EAPOLPKT;
	PUINT8			pBssid = NULL;
	UCHAR			group_cipher = Ndis802_11WEPDisabled;

    DBGPRINT(RT_DEBUG_TRACE, ("===> WPAStart4WayHS\n"));

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS | fRTMP_ADAPTER_HALT_IN_PROGRESS))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("[ERROR]WPAStart4WayHS : The interface is closed...\n"));
		return;
	}


	if (pBssid == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("[ERROR]WPAStart4WayHS : No corresponding Authenticator.\n"));
		return;
    }

	
    if ((pEntry->WpaState > AS_PTKSTART) || (pEntry->WpaState < AS_INITPMK))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("[ERROR]WPAStart4WayHS : Not expect calling\n"));
        return;
    }


	
	ADD_ONE_To_64BIT_VAR(pEntry->R_Counter);

	
	GenRandom(pAd, (UCHAR *)pBssid, pEntry->ANonce);

	
	
	NdisZeroMemory(&EAPOLPKT, sizeof(EAPOL_PACKET));
	ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_PAIR_MSG_1,
					  0,					
					  pEntry->ANonce,
					  NULL,					
					  NULL,					
					  NULL,					
					  0,					
					  &EAPOLPKT);


	
    MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pBssid, EAPOL);
    RTMPToWirelessSta(pAd, pEntry, Header802_3,
					  LENGTH_802_3, (PUCHAR)&EAPOLPKT,
					  CONV_ARRARY_TO_UINT16(EAPOLPKT.Body_Len) + 4,
					  (pEntry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);

	
    RTMPModTimer(&pEntry->RetryTimer, TimeInterval);

	
    pEntry->WpaState = AS_PTKSTART;

	DBGPRINT(RT_DEBUG_TRACE, ("<=== WPAStart4WayHS: send Msg1 of 4-way \n"));

}


VOID PeerPairMsg1Action(
	IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem)
{
	UCHAR				PTK[80];
	UCHAR               Header802_3[14];
	PEAPOL_PACKET		pMsg1;
	UINT			MsgLen;
	EAPOL_PACKET		EAPOLPKT;
	PUINT8				pCurrentAddr = NULL;
	PUINT8				pmk_ptr = NULL;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
	PUINT8				rsnie_ptr = NULL;
	UCHAR				rsnie_len = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("===> PeerPairMsg1Action \n"));

	if ((!pEntry) || ((!pEntry->ValidAsCLI) && (!pEntry->ValidAsApCli)))
		return;

    if (Elem->MsgLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_EAPOL_H + sizeof(KEY_DESCRIPTER) - MAX_LEN_OF_RSNIE - 2))
        return;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		pCurrentAddr = pAd->CurrentAddress;
		pmk_ptr = pAd->StaCfg.PMK;
		group_cipher = pAd->StaCfg.GroupCipher;
		rsnie_ptr = pAd->StaCfg.RSN_IE;
		rsnie_len = pAd->StaCfg.RSNIE_Len;
	}
#endif 

	
	pMsg1 = (PEAPOL_PACKET) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H;

	
	if (PeerWpaMessageSanity(pAd, pMsg1, MsgLen, EAPOL_PAIR_MSG_1, pEntry) == FALSE)
		return;

	
	NdisMoveMemory(pEntry->R_Counter, pMsg1->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);

	
	NdisMoveMemory(pEntry->ANonce, pMsg1->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE);

	
	GenRandom(pAd, (UCHAR *)pCurrentAddr, pEntry->SNonce);

	{
	    
	    WpaDerivePTK(pAd,
				pmk_ptr,
				pEntry->ANonce,
					pEntry->Addr,
					pEntry->SNonce,
					pCurrentAddr,
				    PTK,
				    LEN_PTK);

		
		NdisMoveMemory(pEntry->PTK, PTK, LEN_PTK);
	}

	
	pEntry->WpaState = AS_PTKINIT_NEGOTIATING;

	
	
	NdisZeroMemory(&EAPOLPKT, sizeof(EAPOL_PACKET));
	ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_PAIR_MSG_2,
					  0,				
					  pEntry->SNonce,
					  NULL,				
					  NULL,				
					  (UCHAR *)rsnie_ptr,
					  rsnie_len,
					  &EAPOLPKT);

	
	MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pCurrentAddr, EAPOL);

	RTMPToWirelessSta(pAd, pEntry,
					  Header802_3, sizeof(Header802_3), (PUCHAR)&EAPOLPKT,
					  CONV_ARRARY_TO_UINT16(EAPOLPKT.Body_Len) + 4, TRUE);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== PeerPairMsg1Action: send Msg2 of 4-way \n"));
}



VOID PeerPairMsg2Action(
    IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem)
{
	UCHAR				PTK[80];
    BOOLEAN             Cancelled;
    PHEADER_802_11      pHeader;
	EAPOL_PACKET        EAPOLPKT;
	PEAPOL_PACKET       pMsg2;
	UINT			MsgLen;
    UCHAR               Header802_3[LENGTH_802_3];
	UCHAR				TxTsc[6];
	PUINT8				pBssid = NULL;
	PUINT8				pmk_ptr = NULL;
	PUINT8				gtk_ptr = NULL;
	UCHAR				default_key = 0;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
	PUINT8				rsnie_ptr = NULL;
	UCHAR				rsnie_len = 0;

    DBGPRINT(RT_DEBUG_TRACE, ("===> PeerPairMsg2Action \n"));

    if ((!pEntry) || (!pEntry->ValidAsCLI))
        return;

    if (Elem->MsgLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_EAPOL_H + sizeof(KEY_DESCRIPTER) - MAX_LEN_OF_RSNIE - 2))
        return;

    
    if (pEntry->WpaState < AS_PTKSTART)
        return;



    
	pHeader = (PHEADER_802_11)Elem->Msg;

	
	pMsg2 = (PEAPOL_PACKET)&Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H;

	
	NdisMoveMemory(pEntry->SNonce, pMsg2->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE);

	{
		
		WpaDerivePTK(pAd,
					(UCHAR *)pmk_ptr,
					pEntry->ANonce,			
					(UCHAR *)pBssid,
					pEntry->SNonce,			
					pEntry->Addr,
					PTK,
					LEN_PTK);

	NdisMoveMemory(pEntry->PTK, PTK, LEN_PTK);
	}

	
	if (PeerWpaMessageSanity(pAd, pMsg2, MsgLen, EAPOL_PAIR_MSG_2, pEntry) == FALSE)
		return;

    do
    {
        
		RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);

		
        pEntry->WpaState = AS_PTKINIT_NEGOTIATING;

		
		ADD_ONE_To_64BIT_VAR(pEntry->R_Counter);

		
		NdisZeroMemory(&EAPOLPKT, sizeof(EAPOL_PACKET));
		ConstructEapolMsg(pEntry,
						  group_cipher,
						  EAPOL_PAIR_MSG_3,
						  default_key,
						  pEntry->ANonce,
						  TxTsc,
						  (UCHAR *)gtk_ptr,
						  (UCHAR *)rsnie_ptr,
						  rsnie_len,
						  &EAPOLPKT);

        
        MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pBssid, EAPOL);
        RTMPToWirelessSta(pAd, pEntry, Header802_3, LENGTH_802_3,
						  (PUCHAR)&EAPOLPKT,
						  CONV_ARRARY_TO_UINT16(EAPOLPKT.Body_Len) + 4,
						  (pEntry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);

        pEntry->ReTryCounter = PEER_MSG3_RETRY_TIMER_CTR;
		RTMPSetTimer(&pEntry->RetryTimer, PEER_MSG3_RETRY_EXEC_INTV);

		
        pEntry->WpaState = AS_PTKINIT_NEGOTIATING;
    }while(FALSE);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== PeerPairMsg2Action: send Msg3 of 4-way \n"));
}


VOID PeerPairMsg3Action(
    IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem)
{
	PHEADER_802_11		pHeader;
	UCHAR               Header802_3[14];
	EAPOL_PACKET		EAPOLPKT;
	PEAPOL_PACKET		pMsg3;
	UINT			MsgLen;
	PUINT8				pCurrentAddr = NULL;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;

	DBGPRINT(RT_DEBUG_TRACE, ("===> PeerPairMsg3Action \n"));

	if ((!pEntry) || ((!pEntry->ValidAsCLI) && (!pEntry->ValidAsApCli)))
		return;

    if (Elem->MsgLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_EAPOL_H + sizeof(KEY_DESCRIPTER) - MAX_LEN_OF_RSNIE - 2))
		return;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		pCurrentAddr = pAd->CurrentAddress;
		group_cipher = pAd->StaCfg.GroupCipher;

	}
#endif 

	
	pHeader	= (PHEADER_802_11) Elem->Msg;
	pMsg3 = (PEAPOL_PACKET) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H;

	
	if (PeerWpaMessageSanity(pAd, pMsg3, MsgLen, EAPOL_PAIR_MSG_3, pEntry) == FALSE)
		return;

	
	NdisMoveMemory(pEntry->R_Counter, pMsg3->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);

	
	if (!NdisEqualMemory(pEntry->ANonce, pMsg3->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE))
	{
		return;
	}

	
	NdisZeroMemory(&EAPOLPKT, sizeof(EAPOL_PACKET));
	ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_PAIR_MSG_4,
					  0,					
					  NULL,					
					  NULL,					
					  NULL,					
					  NULL,					
					  0,
					  &EAPOLPKT);

	
	pEntry->WpaState = AS_PTKINITDONE;

	
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		PCIPHER_KEY pSharedKey;

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

		
		pEntry = &pAd->MacTab.Content[BSSID_WCID];
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

	}
#endif 

	
	if (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK ||
		pEntry->AuthMode == Ndis802_11AuthModeWPA2)
	{
		pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
		pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;

#ifdef CONFIG_STA_SUPPORT
		STA_PORT_SECURED(pAd);
	    
	    pAd->IndicateMediaState = NdisMediaStateConnected;
#endif 
		DBGPRINT(RT_DEBUG_TRACE, ("PeerPairMsg3Action: AuthMode(%s) PairwiseCipher(%s) GroupCipher(%s) \n",
									GetAuthMode(pEntry->AuthMode),
									GetEncryptType(pEntry->WepStatus),
									GetEncryptType(group_cipher)));
	}
	else
	{
	}

	
	MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pCurrentAddr, EAPOL);
	RTMPToWirelessSta(pAd, pEntry,
					  Header802_3, sizeof(Header802_3),
					  (PUCHAR)&EAPOLPKT,
					  CONV_ARRARY_TO_UINT16(EAPOLPKT.Body_Len) + 4, TRUE);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== PeerPairMsg3Action: send Msg4 of 4-way \n"));
}


VOID PeerPairMsg4Action(
    IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem)
{
	PEAPOL_PACKET		pMsg4;
    PHEADER_802_11      pHeader;
    UINT		MsgLen;
    BOOLEAN             Cancelled;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;

    DBGPRINT(RT_DEBUG_TRACE, ("===> PeerPairMsg4Action\n"));

    do
    {
        if ((!pEntry) || (!pEntry->ValidAsCLI))
            break;

        if (Elem->MsgLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_EAPOL_H + sizeof(KEY_DESCRIPTER) - MAX_LEN_OF_RSNIE - 2 ) )
            break;

        if (pEntry->WpaState < AS_PTKINIT_NEGOTIATING)
            break;


        
        pHeader = (PHEADER_802_11)Elem->Msg;

		
		pMsg4 = (PEAPOL_PACKET)&Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
		MsgLen = Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H;

        
		if (PeerWpaMessageSanity(pAd, pMsg4, MsgLen, EAPOL_PAIR_MSG_4, pEntry) == FALSE)
			break;

        
        NdisZeroMemory(&pEntry->PairwiseKey, sizeof(CIPHER_KEY));

		
		AsicUpdateWCIDIVEIV(pAd, pEntry->Aid, 1, 0);

        pEntry->PairwiseKey.KeyLen = LEN_TKIP_EK;
        NdisMoveMemory(pEntry->PairwiseKey.Key, &pEntry->PTK[32], LEN_TKIP_EK);
        NdisMoveMemory(pEntry->PairwiseKey.RxMic, &pEntry->PTK[TKIP_AP_RXMICK_OFFSET], LEN_TKIP_RXMICK);
        NdisMoveMemory(pEntry->PairwiseKey.TxMic, &pEntry->PTK[TKIP_AP_TXMICK_OFFSET], LEN_TKIP_TXMICK);

		
        {
            pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
            if (pEntry->WepStatus == Ndis802_11Encryption2Enabled)
                pEntry->PairwiseKey.CipherAlg = CIPHER_TKIP;
            else if (pEntry->WepStatus == Ndis802_11Encryption3Enabled)
                pEntry->PairwiseKey.CipherAlg = CIPHER_AES;

			
            AsicAddPairwiseKeyEntry(
                pAd,
                pEntry->Addr,
                (UCHAR)pEntry->Aid,
                &pEntry->PairwiseKey);

			
			RTMPAddWcidAttributeEntry(
				pAd,
				pEntry->apidx,
				0,
				pEntry->PairwiseKey.CipherAlg,
				pEntry);
        }

        
        pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
        pEntry->WpaState = AS_PTKINITDONE;
		pEntry->PortSecured = WPA_802_1X_PORT_SECURED;


		if (pEntry->AuthMode == Ndis802_11AuthModeWPA2 ||
			pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)
		{
			pEntry->GTKState = REKEY_ESTABLISHED;
			RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);


			
			if (pAd->CommonCfg.bWirelessEvent)
				RTMPSendWirelessEvent(pAd, IW_SET_KEY_DONE_WPA2_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0);

	        DBGPRINT(RT_DEBUG_OFF, ("AP SETKEYS DONE - WPA2, AuthMode(%d)=%s, WepStatus(%d)=%s, GroupWepStatus(%d)=%s\n\n",
									pEntry->AuthMode, GetAuthMode(pEntry->AuthMode),
									pEntry->WepStatus, GetEncryptType(pEntry->WepStatus),
									group_cipher,
									GetEncryptType(group_cipher)));
		}
		else
		{
		
	        WPAStart2WayGroupHS(pAd, pEntry);

		pEntry->ReTryCounter = GROUP_MSG1_RETRY_TIMER_CTR;
			RTMPModTimer(&pEntry->RetryTimer, PEER_MSG3_RETRY_EXEC_INTV);
		}
    }while(FALSE);

}


VOID WPAStart2WayGroupHS(
    IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry)
{
    UCHAR               Header802_3[14];
	UCHAR				TxTsc[6];
    EAPOL_PACKET	EAPOLPKT;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
	UCHAR				default_key = 0;
	PUINT8				gnonce_ptr = NULL;
	PUINT8				gtk_ptr = NULL;
	PUINT8				pBssid = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("===> WPAStart2WayGroupHS\n"));

    if ((!pEntry) || (!pEntry->ValidAsCLI))
        return;


    do
    {
        
		ADD_ONE_To_64BIT_VAR(pEntry->R_Counter);

		
		NdisZeroMemory(&EAPOLPKT, sizeof(EAPOL_PACKET));
		ConstructEapolMsg(pEntry,
						  group_cipher,
						  EAPOL_GROUP_MSG_1,
						  default_key,
						  (UCHAR *)gnonce_ptr,
						  TxTsc,
						  (UCHAR *)gtk_ptr,
						  NULL,
						  0,
						  &EAPOLPKT);

		
        MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pBssid, EAPOL);
        RTMPToWirelessSta(pAd, pEntry,
						  Header802_3, LENGTH_802_3,
						  (PUCHAR)&EAPOLPKT,
						  CONV_ARRARY_TO_UINT16(EAPOLPKT.Body_Len) + 4, FALSE);



    }while (FALSE);

    DBGPRINT(RT_DEBUG_TRACE, ("<=== WPAStart2WayGroupHS : send out Group Message 1 \n"));

    return;
}


VOID	PeerGroupMsg1Action(
	IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem)
{
    UCHAR               Header802_3[14];
	EAPOL_PACKET		EAPOLPKT;
	PEAPOL_PACKET		pGroup;
	UINT			MsgLen;
	BOOLEAN             Cancelled;
	UCHAR				default_key = 0;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
	PUINT8				pCurrentAddr = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("===> PeerGroupMsg1Action \n"));

	if ((!pEntry) || ((!pEntry->ValidAsCLI) && (!pEntry->ValidAsApCli)))
        return;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		pCurrentAddr = pAd->CurrentAddress;
		group_cipher = pAd->StaCfg.GroupCipher;
		default_key = pAd->StaCfg.DefaultKeyId;
	}
#endif 

	
	pGroup = (PEAPOL_PACKET) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H;

	
	if (PeerWpaMessageSanity(pAd, pGroup, MsgLen, EAPOL_GROUP_MSG_1, pEntry) == FALSE)
		return;

	
	RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);

	
	NdisMoveMemory(pEntry->R_Counter, pGroup->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);

	
	NdisZeroMemory(&EAPOLPKT, sizeof(EAPOL_PACKET));
	ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_GROUP_MSG_2,
					  default_key,
					  NULL,					
					  NULL,					
					  NULL,					
					  NULL,					
					  0,
					  &EAPOLPKT);

    
	pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
	pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;

#ifdef CONFIG_STA_SUPPORT
	STA_PORT_SECURED(pAd);
    
    pAd->IndicateMediaState = NdisMediaStateConnected;
#endif 

	DBGPRINT(RT_DEBUG_TRACE, ("PeerGroupMsg1Action: AuthMode(%s) PairwiseCipher(%s) GroupCipher(%s) \n",
									GetAuthMode(pEntry->AuthMode),
									GetEncryptType(pEntry->WepStatus),
									GetEncryptType(group_cipher)));

	
	MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pCurrentAddr, EAPOL);
	RTMPToWirelessSta(pAd, pEntry,
					  Header802_3, sizeof(Header802_3),
					  (PUCHAR)&EAPOLPKT,
					  CONV_ARRARY_TO_UINT16(EAPOLPKT.Body_Len) + 4, FALSE);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== PeerGroupMsg1Action: sned group message 2\n"));
}


VOID PeerGroupMsg2Action(
    IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN VOID             *Msg,
    IN UINT             MsgLen)
{
    UINT		Len;
    PUCHAR		pData;
    BOOLEAN		Cancelled;
	PEAPOL_PACKET       pMsg2;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;

	DBGPRINT(RT_DEBUG_TRACE, ("===> PeerGroupMsg2Action \n"));

    do
    {
        if ((!pEntry) || (!pEntry->ValidAsCLI))
            break;

        if (MsgLen < (LENGTH_802_1_H + LENGTH_EAPOL_H + sizeof(KEY_DESCRIPTER) - MAX_LEN_OF_RSNIE - 2))
            break;

        if (pEntry->WpaState != AS_PTKINITDONE)
            break;


        pData = (PUCHAR)Msg;
		pMsg2 = (PEAPOL_PACKET) (pData + LENGTH_802_1_H);
        Len = MsgLen - LENGTH_802_1_H;

		
		if (PeerWpaMessageSanity(pAd, pMsg2, Len, EAPOL_GROUP_MSG_2, pEntry) == FALSE)
            break;

        

		RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
        pEntry->GTKState = REKEY_ESTABLISHED;

		if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
		{
			
			if (pAd->CommonCfg.bWirelessEvent)
				RTMPSendWirelessEvent(pAd, IW_SET_KEY_DONE_WPA2_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0);

			DBGPRINT(RT_DEBUG_OFF, ("AP SETKEYS DONE - WPA2, AuthMode(%d)=%s, WepStatus(%d)=%s, GroupWepStatus(%d)=%s\n\n",
										pEntry->AuthMode, GetAuthMode(pEntry->AuthMode),
										pEntry->WepStatus, GetEncryptType(pEntry->WepStatus),
										group_cipher, GetEncryptType(group_cipher)));
		}
		else
		{
			
			if (pAd->CommonCfg.bWirelessEvent)
				RTMPSendWirelessEvent(pAd, IW_SET_KEY_DONE_WPA1_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0);

		DBGPRINT(RT_DEBUG_OFF, ("AP SETKEYS DONE - WPA1, AuthMode(%d)=%s, WepStatus(%d)=%s, GroupWepStatus(%d)=%s\n\n",
										pEntry->AuthMode, GetAuthMode(pEntry->AuthMode),
										pEntry->WepStatus, GetEncryptType(pEntry->WepStatus),
										group_cipher, GetEncryptType(group_cipher)));
		}
    }while(FALSE);
}


BOOLEAN	WpaMsgTypeSubst(
	IN	UCHAR	EAPType,
	OUT	INT		*MsgType)
{
	switch (EAPType)
	{
		case EAPPacket:
			*MsgType = MT2_EAPPacket;
			break;
		case EAPOLStart:
			*MsgType = MT2_EAPOLStart;
			break;
		case EAPOLLogoff:
			*MsgType = MT2_EAPOLLogoff;
			break;
		case EAPOLKey:
			*MsgType = MT2_EAPOLKey;
			break;
		case EAPOLASFAlert:
			*MsgType = MT2_EAPOLASFAlert;
			break;
		default:
			return FALSE;
	}
	return TRUE;
}


VOID	PRF(
	IN	UCHAR	*key,
	IN	INT		key_len,
	IN	UCHAR	*prefix,
	IN	INT		prefix_len,
	IN	UCHAR	*data,
	IN	INT		data_len,
	OUT	UCHAR	*output,
	IN	INT		len)
{
	INT		i;
    UCHAR   *input;
	INT		currentindex = 0;
	INT		total_len;

	
	os_alloc_mem(NULL, (PUCHAR *)&input, 1024);

    if (input == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!PRF: no memory!!!\n"));
        return;
    }

	
	NdisMoveMemory(input, prefix, prefix_len);

	
	input[prefix_len] =	0;

	
	NdisMoveMemory(&input[prefix_len + 1], data, data_len);
	total_len =	prefix_len + 1 + data_len;

	
	
	input[total_len] = 0;
	total_len++;

	
	
	for	(i = 0;	i <	(len + 19) / 20; i++)
	{
		HMAC_SHA1(key, key_len, input, total_len, &output[currentindex], SHA1_DIGEST_SIZE);
		currentindex +=	20;

		
		input[total_len - 1]++;
	}
    os_free_mem(NULL, input);
}



static void F(char *password, unsigned char *ssid, int ssidlength, int iterations, int count, unsigned char *output)
{
    unsigned char digest[36], digest1[SHA1_DIGEST_SIZE];
    int i, j;

    
    memcpy(digest, ssid, ssidlength);
    digest[ssidlength] = (unsigned char)((count>>24) & 0xff);
    digest[ssidlength+1] = (unsigned char)((count>>16) & 0xff);
    digest[ssidlength+2] = (unsigned char)((count>>8) & 0xff);
    digest[ssidlength+3] = (unsigned char)(count & 0xff);
    HMAC_SHA1((unsigned char*) password, (int) strlen(password), digest, ssidlength+4, digest1, SHA1_DIGEST_SIZE); 

    
    memcpy(output, digest1, SHA1_DIGEST_SIZE);

    for (i = 1; i < iterations; i++)
    {
        
        HMAC_SHA1((unsigned char*) password, (int) strlen(password), digest1, SHA1_DIGEST_SIZE, digest, SHA1_DIGEST_SIZE); 
        memcpy(digest1, digest, SHA1_DIGEST_SIZE);

        
        for (j = 0; j < SHA1_DIGEST_SIZE; j++)
        {
            output[j] ^= digest[j];
        }
    }
}


int PasswordHash(PSTRING password, PUCHAR ssid, INT ssidlength, PUCHAR output)
{
    if ((strlen(password) > 63) || (ssidlength > 32))
        return 0;

    F(password, ssid, ssidlength, 4096, 1, output);
    F(password, ssid, ssidlength, 4096, 2, &output[SHA1_DIGEST_SIZE]);
    return 1;
}




VOID WpaDerivePTK(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	*PMK,
	IN	UCHAR	*ANonce,
	IN	UCHAR	*AA,
	IN	UCHAR	*SNonce,
	IN	UCHAR	*SA,
	OUT	UCHAR	*output,
	IN	UINT	len)
{
	UCHAR	concatenation[76];
	UINT	CurrPos = 0;
	UCHAR	temp[32];
	UCHAR	Prefix[] = {'P', 'a', 'i', 'r', 'w', 'i', 's', 'e', ' ', 'k', 'e', 'y', ' ',
						'e', 'x', 'p', 'a', 'n', 's', 'i', 'o', 'n'};

	
	NdisZeroMemory(temp, sizeof(temp));
	NdisZeroMemory(concatenation, 76);

	
	if (RTMPCompareMemory(SA, AA, 6) == 1)
		NdisMoveMemory(concatenation, AA, 6);
	else
		NdisMoveMemory(concatenation, SA, 6);
	CurrPos += 6;

	
	if (RTMPCompareMemory(SA, AA, 6) == 1)
		NdisMoveMemory(&concatenation[CurrPos], SA, 6);
	else
		NdisMoveMemory(&concatenation[CurrPos], AA, 6);

	
	
	NdisMoveMemory(temp, &concatenation[CurrPos], MAC_ADDR_LEN);
	CurrPos += 6;

	
	if (RTMPCompareMemory(ANonce, SNonce, 32) == 0)
		NdisMoveMemory(&concatenation[CurrPos], temp, 32);	
	else if (RTMPCompareMemory(ANonce, SNonce, 32) == 1)
		NdisMoveMemory(&concatenation[CurrPos], SNonce, 32);
	else
		NdisMoveMemory(&concatenation[CurrPos], ANonce, 32);
	CurrPos += 32;

	
	if (RTMPCompareMemory(ANonce, SNonce, 32) == 0)
		NdisMoveMemory(&concatenation[CurrPos], temp, 32);	
	else if (RTMPCompareMemory(ANonce, SNonce, 32) == 1)
		NdisMoveMemory(&concatenation[CurrPos], ANonce, 32);
	else
		NdisMoveMemory(&concatenation[CurrPos], SNonce, 32);
	CurrPos += 32;

	hex_dump("concatenation=", concatenation, 76);

	
	PRF(PMK, LEN_MASTER_KEY, Prefix, 22, concatenation, 76, output, len);

}


VOID	GenRandom(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			*macAddr,
	OUT	UCHAR			*random)
{
	INT		i, curr;
	UCHAR	local[80], KeyCounter[32];
	UCHAR	result[80];
	ULONG	CurrentTime;
	UCHAR	prefix[] = {'I', 'n', 'i', 't', ' ', 'C', 'o', 'u', 'n', 't', 'e', 'r'};

	
	NdisZeroMemory(result, 80);
	NdisZeroMemory(local, 80);
	NdisZeroMemory(KeyCounter, 32);

	for	(i = 0;	i <	32;	i++)
	{
		
		COPY_MAC_ADDR(local, macAddr);
		curr =	MAC_ADDR_LEN;

		
		NdisGetSystemUpTime(&CurrentTime);
		NdisMoveMemory(&local[curr],  &CurrentTime,	sizeof(CurrentTime));
		curr +=	sizeof(CurrentTime);

		
		NdisMoveMemory(&local[curr],  result, 32);
		curr +=	32;

		
		NdisMoveMemory(&local[curr],  &i,  2);
		curr +=	2;

		
		PRF(KeyCounter, 32, prefix,12, local, curr, result, 32);
	}

	NdisMoveMemory(random, result,	32);
}


static VOID RTMPMakeRsnIeCipher(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			ElementID,
	IN	UINT			WepStatus,
	IN	BOOLEAN			bMixCipher,
	IN	UCHAR			FlexibleCipher,
	OUT	PUCHAR			pRsnIe,
	OUT	UCHAR			*rsn_len)
{
	UCHAR	PairwiseCnt;

	*rsn_len = 0;

	
	if (ElementID == Wpa2Ie)
	{
		RSNIE2	*pRsnie_cipher = (RSNIE2*)pRsnIe;

		
		pRsnie_cipher->version = 1;

        switch (WepStatus)
        {
		
            case Ndis802_11Encryption2Enabled:
                NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_TKIP, 4);
                pRsnie_cipher->ucount = 1;
                NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA2_TKIP, 4);
                *rsn_len = sizeof(RSNIE2);
                break;

			
            case Ndis802_11Encryption3Enabled:
				if (bMixCipher)
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_TKIP, 4);
				else
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CCMP, 4);
                pRsnie_cipher->ucount = 1;
                NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA2_CCMP, 4);
                *rsn_len = sizeof(RSNIE2);
                break;

			
            case Ndis802_11Encryption4Enabled:
                NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_TKIP, 4);

				PairwiseCnt = 1;
				
				if (MIX_CIPHER_WPA2_TKIP_ON(FlexibleCipher))
				{
			NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA2_TKIP, 4);
					
					if (MIX_CIPHER_WPA2_AES_ON(FlexibleCipher))
					{
				NdisMoveMemory(pRsnie_cipher->ucast[0].oui + 4, OUI_WPA2_CCMP, 4);
						PairwiseCnt = 2;
					}
				}
				else
				{
					
					NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA2_CCMP, 4);
				}

                pRsnie_cipher->ucount = PairwiseCnt;
                *rsn_len = sizeof(RSNIE2) + (4 * (PairwiseCnt - 1));
                break;
        }

#ifdef CONFIG_STA_SUPPORT
		if ((pAd->OpMode == OPMODE_STA) &&
			(pAd->StaCfg.GroupCipher != Ndis802_11Encryption2Enabled) &&
			(pAd->StaCfg.GroupCipher != Ndis802_11Encryption3Enabled))
		{
			UINT	GroupCipher = pAd->StaCfg.GroupCipher;
			switch(GroupCipher)
			{
				case Ndis802_11GroupWEP40Enabled:
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_WEP40, 4);
					break;
				case Ndis802_11GroupWEP104Enabled:
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_WEP104, 4);
					break;
			}
		}
#endif 

		
		pRsnie_cipher->version = cpu2le16(pRsnie_cipher->version);
	    pRsnie_cipher->ucount = cpu2le16(pRsnie_cipher->ucount);
	}
	else
	{
		RSNIE	*pRsnie_cipher = (RSNIE*)pRsnIe;

		
		NdisMoveMemory(pRsnie_cipher->oui, OUI_WPA_VERSION, 4);
        pRsnie_cipher->version = 1;

		switch (WepStatus)
		{
			
            case Ndis802_11Encryption2Enabled:
                NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_TKIP, 4);
                pRsnie_cipher->ucount = 1;
                NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA_TKIP, 4);
                *rsn_len = sizeof(RSNIE);
                break;

			
            case Ndis802_11Encryption3Enabled:
				if (bMixCipher)
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_TKIP, 4);
				else
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_CCMP, 4);
                pRsnie_cipher->ucount = 1;
                NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA_CCMP, 4);
                *rsn_len = sizeof(RSNIE);
                break;

			
            case Ndis802_11Encryption4Enabled:
                NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_TKIP, 4);

				PairwiseCnt = 1;
				
				if (MIX_CIPHER_WPA_TKIP_ON(FlexibleCipher))
				{
			NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA_TKIP, 4);
					
					if (MIX_CIPHER_WPA_AES_ON(FlexibleCipher))
					{
				NdisMoveMemory(pRsnie_cipher->ucast[0].oui + 4, OUI_WPA_CCMP, 4);
						PairwiseCnt = 2;
					}
				}
				else
				{
					
					NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA_CCMP, 4);
				}

                pRsnie_cipher->ucount = PairwiseCnt;
                *rsn_len = sizeof(RSNIE) + (4 * (PairwiseCnt - 1));
                break;
        }

#ifdef CONFIG_STA_SUPPORT
		if ((pAd->OpMode == OPMODE_STA) &&
			(pAd->StaCfg.GroupCipher != Ndis802_11Encryption2Enabled) &&
			(pAd->StaCfg.GroupCipher != Ndis802_11Encryption3Enabled))
		{
			UINT	GroupCipher = pAd->StaCfg.GroupCipher;
			switch(GroupCipher)
			{
				case Ndis802_11GroupWEP40Enabled:
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_WEP40, 4);
					break;
				case Ndis802_11GroupWEP104Enabled:
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_WEP104, 4);
					break;
			}
		}
#endif 

		
		pRsnie_cipher->version = cpu2le16(pRsnie_cipher->version);
	    pRsnie_cipher->ucount = cpu2le16(pRsnie_cipher->ucount);
	}
}


static VOID RTMPMakeRsnIeAKM(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			ElementID,
	IN	UINT			AuthMode,
	IN	UCHAR			apidx,
	OUT	PUCHAR			pRsnIe,
	OUT	UCHAR			*rsn_len)
{
	RSNIE_AUTH		*pRsnie_auth;
	UCHAR			AkmCnt = 1;		

	pRsnie_auth = (RSNIE_AUTH*)(pRsnIe + (*rsn_len));

	
	if (ElementID == Wpa2Ie)
	{

		switch (AuthMode)
        {
            case Ndis802_11AuthModeWPA2:
            case Ndis802_11AuthModeWPA1WPA2:
			NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA2_8021X_AKM, 4);
                break;

            case Ndis802_11AuthModeWPA2PSK:
            case Ndis802_11AuthModeWPA1PSKWPA2PSK:
			NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA2_PSK_AKM, 4);
                break;
			default:
				AkmCnt = 0;
				break;

        }
	}
	else
	{
		switch (AuthMode)
        {
            case Ndis802_11AuthModeWPA:
            case Ndis802_11AuthModeWPA1WPA2:
                NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA_8021X_AKM, 4);
                break;

            case Ndis802_11AuthModeWPAPSK:
            case Ndis802_11AuthModeWPA1PSKWPA2PSK:
                NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA_PSK_AKM, 4);
                break;

			case Ndis802_11AuthModeWPANone:
                NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA_NONE_AKM, 4);
                break;
			default:
				AkmCnt = 0;
				break;
        }
	}

	pRsnie_auth->acount = AkmCnt;
	pRsnie_auth->acount = cpu2le16(pRsnie_auth->acount);

	
	(*rsn_len) += (sizeof(RSNIE_AUTH) + (4 * (AkmCnt - 1)));

}


static VOID RTMPMakeRsnIeCap(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			ElementID,
	IN	UCHAR			apidx,
	OUT	PUCHAR			pRsnIe,
	OUT	UCHAR			*rsn_len)
{
	RSN_CAPABILITIES    *pRSN_Cap;

	
	if (ElementID == WpaIe)
		return;

	pRSN_Cap = (RSN_CAPABILITIES*)(pRsnIe + (*rsn_len));


	pRSN_Cap->word = cpu2le16(pRSN_Cap->word);

	(*rsn_len) += sizeof(RSN_CAPABILITIES);	

}



VOID RTMPMakeRSNIE(
    IN  PRTMP_ADAPTER   pAd,
    IN  UINT            AuthMode,
    IN  UINT            WepStatus,
	IN	UCHAR			apidx)
{
	PUCHAR		pRsnIe = NULL;			
	UCHAR		*rsnielen_cur_p = 0;	
	UCHAR		*rsnielen_ex_cur_p = 0;	
	UCHAR		PrimaryRsnie;
	BOOLEAN		bMixCipher = FALSE;	
	UCHAR		p_offset;
	WPA_MIX_PAIR_CIPHER		FlexibleCipher = WPA_TKIPAES_WPA2_TKIPAES;	

	rsnielen_cur_p = NULL;
	rsnielen_ex_cur_p = NULL;

	{
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
#ifdef WPA_SUPPLICANT_SUPPORT
			if (pAd->StaCfg.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE)
			{
				if (AuthMode < Ndis802_11AuthModeWPA)
					return;
			}
			else
#endif 
			{
				
				
				if ((AuthMode != Ndis802_11AuthModeWPAPSK) &&
					(AuthMode != Ndis802_11AuthModeWPA2PSK) &&
					(AuthMode != Ndis802_11AuthModeWPANone)
					)
					return;
			}

			DBGPRINT(RT_DEBUG_TRACE,("==> RTMPMakeRSNIE(STA)\n"));

			
			pAd->StaCfg.RSNIE_Len = 0;
			NdisZeroMemory(pAd->StaCfg.RSN_IE, MAX_LEN_OF_RSNIE);

			
			rsnielen_cur_p = &pAd->StaCfg.RSNIE_Len;
			pRsnIe = pAd->StaCfg.RSN_IE;

			bMixCipher = pAd->StaCfg.bMixCipher;
		}
#endif 
	}

	
	if ((AuthMode == Ndis802_11AuthModeWPA) ||
		(AuthMode == Ndis802_11AuthModeWPAPSK) ||
		(AuthMode == Ndis802_11AuthModeWPANone) ||
		(AuthMode == Ndis802_11AuthModeWPA1WPA2) ||
		(AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
		PrimaryRsnie = WpaIe;
	else
		PrimaryRsnie = Wpa2Ie;

	{
		
		
		RTMPMakeRsnIeCipher(pAd, PrimaryRsnie, WepStatus, bMixCipher, FlexibleCipher, pRsnIe, &p_offset);

		
		RTMPMakeRsnIeAKM(pAd, PrimaryRsnie, AuthMode, apidx, pRsnIe, &p_offset);

		
		RTMPMakeRsnIeCap(pAd, PrimaryRsnie, apidx, pRsnIe, &p_offset);
	}

	
	*rsnielen_cur_p = p_offset;

	hex_dump("The primary RSNIE", pRsnIe, (*rsnielen_cur_p));


}


BOOLEAN RTMPCheckWPAframe(
    IN PRTMP_ADAPTER    pAd,
    IN PMAC_TABLE_ENTRY	pEntry,
    IN PUCHAR           pData,
    IN ULONG            DataByteCount,
	IN UCHAR			FromWhichBSSID)
{
	ULONG	Body_len;
	BOOLEAN Cancelled;


    if(DataByteCount < (LENGTH_802_1_H + LENGTH_EAPOL_H))
        return FALSE;


	
    if (NdisEqualMemory(SNAP_802_1H, pData, 6) ||
        
        NdisEqualMemory(SNAP_BRIDGE_TUNNEL, pData, 6))
    {
        pData += 6;
    }
	
    if (NdisEqualMemory(EAPOL, pData, 2))

    {
        pData += 2;
    }
    else
        return FALSE;

    switch (*(pData+1))
    {
        case EAPPacket:
			Body_len = (*(pData+2)<<8) | (*(pData+3));
            DBGPRINT(RT_DEBUG_TRACE, ("Receive EAP-Packet frame, TYPE = 0, Length = %ld\n", Body_len));
            break;
        case EAPOLStart:
            DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOL-Start frame, TYPE = 1 \n"));
			if (pEntry->EnqueueEapolStartTimerRunning != EAPOL_START_DISABLE)
            {
		DBGPRINT(RT_DEBUG_TRACE, ("Cancel the EnqueueEapolStartTimerRunning \n"));
                RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
                pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
            }
            break;
        case EAPOLLogoff:
            DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOLLogoff frame, TYPE = 2 \n"));
            break;
        case EAPOLKey:
			Body_len = (*(pData+2)<<8) | (*(pData+3));
            DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOL-Key frame, TYPE = 3, Length = %ld\n", Body_len));
            break;
        case EAPOLASFAlert:
            DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOLASFAlert frame, TYPE = 4 \n"));
            break;
        default:
            return FALSE;

    }
    return TRUE;
}


PSTRING GetEapolMsgType(CHAR msg)
{
    if(msg == EAPOL_PAIR_MSG_1)
        return "Pairwise Message 1";
    else if(msg == EAPOL_PAIR_MSG_2)
        return "Pairwise Message 2";
	else if(msg == EAPOL_PAIR_MSG_3)
        return "Pairwise Message 3";
	else if(msg == EAPOL_PAIR_MSG_4)
        return "Pairwise Message 4";
	else if(msg == EAPOL_GROUP_MSG_1)
        return "Group Message 1";
	else if(msg == EAPOL_GROUP_MSG_2)
        return "Group Message 2";
    else
	return "Invalid Message";
}



BOOLEAN RTMPCheckRSNIE(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pData,
	IN  UCHAR           DataLen,
	IN  MAC_TABLE_ENTRY *pEntry,
	OUT	UCHAR			*Offset)
{
	PUCHAR              pVIE;
	UCHAR               len;
	PEID_STRUCT         pEid;
	BOOLEAN				result = FALSE;

	pVIE = pData;
	len	 = DataLen;
	*Offset = 0;

	while (len > sizeof(RSNIE2))
	{
		pEid = (PEID_STRUCT) pVIE;
		
		if ((pEid->Eid == IE_WPA) && (NdisEqualMemory(pEid->Octet, WPA_OUI, 4)))
		{
			if ((pEntry->AuthMode == Ndis802_11AuthModeWPA || pEntry->AuthMode == Ndis802_11AuthModeWPAPSK) &&
				(NdisEqualMemory(pVIE, pEntry->RSN_IE, pEntry->RSNIE_Len)) &&
				(pEntry->RSNIE_Len == (pEid->Len + 2)))
			{
					result = TRUE;
			}

			*Offset += (pEid->Len + 2);
		}
		
		else if ((pEid->Eid == IE_RSN) && (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3)))
		{
			if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2 || pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK) &&
				(pEid->Eid == pEntry->RSN_IE[0]) &&
				((pEid->Len + 2) >= pEntry->RSNIE_Len) &&
				(NdisEqualMemory(pEid->Octet, &pEntry->RSN_IE[2], pEntry->RSNIE_Len - 2)))
			{

					result = TRUE;
			}

			*Offset += (pEid->Len + 2);
		}
		else
		{
			break;
		}

		pVIE += (pEid->Len + 2);
		len  -= (pEid->Len + 2);
	}


	return result;

}



BOOLEAN RTMPParseEapolKeyData(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pKeyData,
	IN  UCHAR           KeyDataLen,
	IN	UCHAR			GroupKeyIndex,
	IN	UCHAR			MsgType,
	IN	BOOLEAN			bWPA2,
	IN  MAC_TABLE_ENTRY *pEntry)
{
    PKDE_ENCAP          pKDE = NULL;
    PUCHAR              pMyKeyData = pKeyData;
    UCHAR               KeyDataLength = KeyDataLen;
    UCHAR               GTKLEN = 0;
	UCHAR				DefaultIdx = 0;
	UCHAR				skip_offset;

	
	if (MsgType == EAPOL_PAIR_MSG_2 || MsgType == EAPOL_PAIR_MSG_3)
    {
		
		if (!RTMPCheckRSNIE(pAd, pKeyData, KeyDataLen, pEntry, &skip_offset))
		{
			
			if (pAd->CommonCfg.bWirelessEvent)
				RTMPSendWirelessEvent(pAd, IW_RSNIE_DIFF_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0);

		DBGPRINT(RT_DEBUG_ERROR, ("RSN_IE Different in msg %d of 4-way handshake!\n", MsgType));
			hex_dump("Receive RSN_IE ", pKeyData, KeyDataLen);
			hex_dump("Desired RSN_IE ", pEntry->RSN_IE, pEntry->RSNIE_Len);

			return FALSE;
	}
	else
		{
			if (bWPA2 && MsgType == EAPOL_PAIR_MSG_3)
			{
				WpaShowAllsuite(pMyKeyData, skip_offset);

				
				pMyKeyData += skip_offset;
				KeyDataLength -= skip_offset;
				DBGPRINT(RT_DEBUG_TRACE, ("RTMPParseEapolKeyData ==> WPA2/WPA2PSK RSN IE matched in Msg 3, Length(%d) \n", skip_offset));
			}
			else
				return TRUE;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE,("RTMPParseEapolKeyData ==> KeyDataLength %d without RSN_IE \n", KeyDataLength));
	


	
	if (bWPA2 && (MsgType == EAPOL_PAIR_MSG_3 || MsgType == EAPOL_GROUP_MSG_1))
	{
		if (KeyDataLength >= 8)	
	{
		pKDE = (PKDE_ENCAP) pMyKeyData;


			DefaultIdx = pKDE->GTKEncap.Kid;

			
			if (KeyDataLength < (pKDE->Len + 2))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ERROR: The len from KDE is too short \n"));
			return FALSE;
		}

			
			GTKLEN = pKDE->Len -6;
			if (GTKLEN < LEN_AES_KEY)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ERROR: GTK Key length is too short (%d) \n", GTKLEN));
			return FALSE;
			}

	}
		else
	{
			DBGPRINT(RT_DEBUG_ERROR, ("ERROR: KDE format length is too short \n"));
	        return FALSE;
	}

		DBGPRINT(RT_DEBUG_TRACE, ("GTK in KDE format ,DefaultKeyID=%d, KeyLen=%d \n", DefaultIdx, GTKLEN));
		
		pMyKeyData += 8;
		KeyDataLength -= 8;

	}
	else if (!bWPA2 && MsgType == EAPOL_GROUP_MSG_1)
	{
		DefaultIdx = GroupKeyIndex;
		DBGPRINT(RT_DEBUG_TRACE, ("GTK DefaultKeyID=%d \n", DefaultIdx));
	}

	
	if (DefaultIdx < 1 || DefaultIdx > 3)
    {
	DBGPRINT(RT_DEBUG_ERROR, ("ERROR: GTK Key index(%d) is invalid in %s %s \n", DefaultIdx, ((bWPA2) ? "WPA2" : "WPA"), GetEapolMsgType(MsgType)));
        return FALSE;
    }


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		PCIPHER_KEY pSharedKey;

		
		NdisMoveMemory(pAd->StaCfg.GTK, pMyKeyData, 32);
		pAd->StaCfg.DefaultKeyId = DefaultIdx;

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
#endif 

	return TRUE;

}



VOID	ConstructEapolMsg(
	IN	PMAC_TABLE_ENTRY	pEntry,
    IN	UCHAR				GroupKeyWepStatus,
    IN	UCHAR				MsgType,
    IN	UCHAR				DefaultKeyIdx,
	IN	UCHAR				*KeyNonce,
	IN	UCHAR				*TxRSC,
	IN	UCHAR				*GTK,
	IN	UCHAR				*RSNIE,
	IN	UCHAR				RSNIE_Len,
    OUT PEAPOL_PACKET       pMsg)
{
	BOOLEAN	bWPA2 = FALSE;
	UCHAR	KeyDescVer;

	
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) ||
		(pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
		bWPA2 = TRUE;

    
    pMsg->ProVer = EAPOL_VER;
    pMsg->ProType = EAPOLKey;

	
	SET_UINT16_TO_ARRARY(pMsg->Body_Len, LEN_EAPOL_KEY_MSG);

	
	if (bWPA2)
		pMsg->KeyDesc.Type = WPA2_KEY_DESC;
	else
		pMsg->KeyDesc.Type = WPA1_KEY_DESC;

	
	{
		
		
		KeyDescVer = (((pEntry->WepStatus == Ndis802_11Encryption3Enabled) ||
					(GroupKeyWepStatus == Ndis802_11Encryption3Enabled)) ? (DESC_TYPE_AES) : (DESC_TYPE_TKIP));
	}

	pMsg->KeyDesc.KeyInfo.KeyDescVer = KeyDescVer;

	
	if (MsgType >= EAPOL_GROUP_MSG_1)
		pMsg->KeyDesc.KeyInfo.KeyType = GROUPKEY;
	else
		pMsg->KeyDesc.KeyInfo.KeyType = PAIRWISEKEY;

	
	if (!bWPA2 && (MsgType >= EAPOL_GROUP_MSG_1))
		pMsg->KeyDesc.KeyInfo.KeyIndex = DefaultKeyIdx;

	if (MsgType == EAPOL_PAIR_MSG_3)
		pMsg->KeyDesc.KeyInfo.Install = 1;

	if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3) || (MsgType == EAPOL_GROUP_MSG_1))
		pMsg->KeyDesc.KeyInfo.KeyAck = 1;

	if (MsgType != EAPOL_PAIR_MSG_1)
		pMsg->KeyDesc.KeyInfo.KeyMic = 1;

	if ((bWPA2 && (MsgType >= EAPOL_PAIR_MSG_3)) ||
		(!bWPA2 && (MsgType >= EAPOL_GROUP_MSG_1)))
    {
	pMsg->KeyDesc.KeyInfo.Secure = 1;
    }

	if (bWPA2 && ((MsgType == EAPOL_PAIR_MSG_3) ||
		(MsgType == EAPOL_GROUP_MSG_1)))
    {
        pMsg->KeyDesc.KeyInfo.EKD_DL = 1;
    }

	
	*(USHORT *)(&pMsg->KeyDesc.KeyInfo) = cpu2le16(*(USHORT *)(&pMsg->KeyDesc.KeyInfo));

	
	{
		if (MsgType >= EAPOL_GROUP_MSG_1)
		{
			
			pMsg->KeyDesc.KeyLength[1] = ((GroupKeyWepStatus == Ndis802_11Encryption2Enabled) ? TKIP_GTK_LENGTH : LEN_AES_KEY);
		}
		else
		{
			
			pMsg->KeyDesc.KeyLength[1] = ((pEntry->WepStatus == Ndis802_11Encryption2Enabled) ? LEN_TKIP_KEY : LEN_AES_KEY);
		}
	}

	
    NdisMoveMemory(pMsg->KeyDesc.ReplayCounter, pEntry->R_Counter, LEN_KEY_DESC_REPLAY);

	
	
	
	
	if ((MsgType <= EAPOL_PAIR_MSG_3) || ((!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1))))
	NdisMoveMemory(pMsg->KeyDesc.KeyNonce, KeyNonce, LEN_KEY_DESC_NONCE);

	
	if (!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1))
	{
		
		NdisMoveMemory(pMsg->KeyDesc.KeyIv, &KeyNonce[16], LEN_KEY_DESC_IV);
        pMsg->KeyDesc.KeyIv[15] += 2;
	}

    
    
	if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1))
	{
        NdisMoveMemory(pMsg->KeyDesc.KeyRsc, TxRSC, 6);
	}

	
    NdisZeroMemory(pMsg->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);

	ConstructEapolKeyData(pEntry,
						  GroupKeyWepStatus,
						  KeyDescVer,
						  MsgType,
						  DefaultKeyIdx,
						  GTK,
						  RSNIE,
						  RSNIE_Len,
						  pMsg);

	
	if (MsgType != EAPOL_PAIR_MSG_1)
	{
		CalculateMIC(KeyDescVer, pEntry->PTK, pMsg);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("===> ConstructEapolMsg for %s %s\n", ((bWPA2) ? "WPA2" : "WPA"), GetEapolMsgType(MsgType)));
	DBGPRINT(RT_DEBUG_TRACE, ("	     Body length = %d \n", CONV_ARRARY_TO_UINT16(pMsg->Body_Len)));
	DBGPRINT(RT_DEBUG_TRACE, ("	     Key length  = %d \n", CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyLength)));


}


VOID	ConstructEapolKeyData(
	IN	PMAC_TABLE_ENTRY	pEntry,
	IN	UCHAR			GroupKeyWepStatus,
	IN	UCHAR			keyDescVer,
	IN	UCHAR			MsgType,
	IN	UCHAR			DefaultKeyIdx,
	IN	UCHAR			*GTK,
	IN	UCHAR			*RSNIE,
	IN	UCHAR			RSNIE_LEN,
	OUT PEAPOL_PACKET   pMsg)
{
	UCHAR		*mpool, *Key_Data, *Rc4GTK;
	UCHAR       ekey[(LEN_KEY_DESC_IV+LEN_EAP_EK)];
	ULONG		data_offset;
	BOOLEAN		bWPA2Capable = FALSE;
	PRTMP_ADAPTER	pAd = pEntry->pAd;
	BOOLEAN		GTK_Included = FALSE;

	
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) ||
		(pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
		bWPA2Capable = TRUE;

	if (MsgType == EAPOL_PAIR_MSG_1 ||
		MsgType == EAPOL_PAIR_MSG_4 ||
		MsgType == EAPOL_GROUP_MSG_2)
		return;

	
	os_alloc_mem(NULL, (PUCHAR *)&mpool, 1500);

    if (mpool == NULL)
		return;

	
	Rc4GTK = (UCHAR *) ROUND_UP(mpool, 4);
	
	Key_Data = (UCHAR *) ROUND_UP(Rc4GTK + 512, 4);

	NdisZeroMemory(Key_Data, 512);
	SET_UINT16_TO_ARRARY(pMsg->KeyDesc.KeyDataLen, 0);
	data_offset = 0;

	
	if (RSNIE_LEN && ((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3)))
	{
		PUINT8	pmkid_ptr = NULL;
		UINT8	pmkid_len = 0;


		RTMPInsertRSNIE(&Key_Data[data_offset],
						(PULONG)&data_offset,
						RSNIE,
						RSNIE_LEN,
						pmkid_ptr,
						pmkid_len);
	}


	
	if (bWPA2Capable && ((MsgType == EAPOL_PAIR_MSG_3) || (MsgType == EAPOL_GROUP_MSG_1)))
	{
		
        Key_Data[data_offset + 0] = 0xDD;

		if (GroupKeyWepStatus == Ndis802_11Encryption3Enabled)
		{
			Key_Data[data_offset + 1] = 0x16;
		}
		else
		{
			Key_Data[data_offset + 1] = 0x26;
		}

        Key_Data[data_offset + 2] = 0x00;
        Key_Data[data_offset + 3] = 0x0F;
        Key_Data[data_offset + 4] = 0xAC;
        Key_Data[data_offset + 5] = 0x01;

		
        Key_Data[data_offset + 6] = (DefaultKeyIdx & 0x03);
        Key_Data[data_offset + 7] = 0x00;	

		data_offset += 8;
	}


	
	
	if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2Capable) || (MsgType == EAPOL_GROUP_MSG_1))
	{
		
		if (GroupKeyWepStatus == Ndis802_11Encryption3Enabled)
		{
			NdisMoveMemory(&Key_Data[data_offset], GTK, LEN_AES_KEY);
			data_offset += LEN_AES_KEY;
		}
		else
		{
			NdisMoveMemory(&Key_Data[data_offset], GTK, TKIP_GTK_LENGTH);
			data_offset += TKIP_GTK_LENGTH;
		}

		GTK_Included = TRUE;
	}


	
	
	if (GTK_Included)
	{
		

		if (
			(keyDescVer == DESC_TYPE_AES))
		{
			UCHAR	remainder = 0;
			UCHAR	pad_len = 0;

			
			
			

			
			
			
			
			if ((remainder = data_offset & 0x07) != 0)
			{
				INT		i;

				pad_len = (8 - remainder);
				Key_Data[data_offset] = 0xDD;
				for (i = 1; i < pad_len; i++)
					Key_Data[data_offset + i] = 0;

				data_offset += pad_len;
			}

			AES_GTK_KEY_WRAP(&pEntry->PTK[16], Key_Data, data_offset, Rc4GTK);
            
            data_offset += 8;
		}
		else
		{
			

			
			
			pAd->PrivateInfo.FCSCRC32 = PPPINITFCS32;   

			
			NdisMoveMemory(ekey, pMsg->KeyDesc.KeyIv, LEN_KEY_DESC_IV);
			NdisMoveMemory(&ekey[LEN_KEY_DESC_IV], &pEntry->PTK[16], LEN_EAP_EK);
			ARCFOUR_INIT(&pAd->PrivateInfo.WEPCONTEXT, ekey, sizeof(ekey));  
			pAd->PrivateInfo.FCSCRC32 = RTMP_CALC_FCS32(pAd->PrivateInfo.FCSCRC32, Key_Data, data_offset);
			WPAARCFOUR_ENCRYPT(&pAd->PrivateInfo.WEPCONTEXT, Rc4GTK, Key_Data, data_offset);
		}

		NdisMoveMemory(pMsg->KeyDesc.KeyData, Rc4GTK, data_offset);
	}
	else
	{
		NdisMoveMemory(pMsg->KeyDesc.KeyData, Key_Data, data_offset);
	}

	
	SET_UINT16_TO_ARRARY(pMsg->KeyDesc.KeyDataLen, data_offset);
	INC_UINT16_TO_ARRARY(pMsg->Body_Len, data_offset);

	os_free_mem(NULL, mpool);

}


static VOID	CalculateMIC(
	IN	UCHAR			KeyDescVer,
	IN	UCHAR			*PTK,
	OUT PEAPOL_PACKET   pMsg)
{
    UCHAR   *OutBuffer;
	ULONG	FrameLen = 0;
	UCHAR	mic[LEN_KEY_DESC_MIC];
	UCHAR	digest[80];

	
	os_alloc_mem(NULL, (PUCHAR *)&OutBuffer, 512);

    if (OutBuffer == NULL)
    {
		DBGPRINT(RT_DEBUG_ERROR, ("!!!CalculateMIC: no memory!!!\n"));
		return;
    }

	
    MakeOutgoingFrame(OutBuffer,		&FrameLen,
                      CONV_ARRARY_TO_UINT16(pMsg->Body_Len) + 4,	pMsg,
                      END_OF_ARGS);

	NdisZeroMemory(mic, sizeof(mic));

	
    if (KeyDescVer == DESC_TYPE_AES)
	{
		HMAC_SHA1(PTK, LEN_EAP_MICK, OutBuffer,  FrameLen, digest, SHA1_DIGEST_SIZE);
		NdisMoveMemory(mic, digest, LEN_KEY_DESC_MIC);
	}
	else
	{
		HMAC_MD5(PTK,  LEN_EAP_MICK, OutBuffer, FrameLen, mic, MD5_DIGEST_SIZE);
	}

	
	NdisMoveMemory(pMsg->KeyDesc.KeyMic, mic, LEN_KEY_DESC_MIC);

	os_free_mem(NULL, OutBuffer);
}


NDIS_STATUS	RTMPSoftDecryptBroadCastData(
	IN	PRTMP_ADAPTER					pAd,
	IN	RX_BLK							*pRxBlk,
	IN  NDIS_802_11_ENCRYPTION_STATUS	GroupCipher,
	IN  PCIPHER_KEY						pShard_key)
{
	PRXWI_STRUC			pRxWI = pRxBlk->pRxWI;



	
	if (GroupCipher == Ndis802_11Encryption1Enabled)
    {
		if (RTMPSoftDecryptWEP(pAd, pRxBlk->pData, pRxWI->MPDUtotalByteCount, pShard_key))
		{

			
			pRxWI->MPDUtotalByteCount -= 8;
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ERROR : Software decrypt WEP data fails.\n"));
			
			return NDIS_STATUS_FAILURE;
		}
	}
	
	else if (GroupCipher == Ndis802_11Encryption2Enabled)
	{
		if (RTMPSoftDecryptTKIP(pAd, pRxBlk->pData, pRxWI->MPDUtotalByteCount, 0, pShard_key))
		{

			
			pRxWI->MPDUtotalByteCount -= 20;
		}
        else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ERROR : RTMPSoftDecryptTKIP Failed\n"));
			
			return NDIS_STATUS_FAILURE;
        }
	}
	
	else if (GroupCipher == Ndis802_11Encryption3Enabled)
	{
		if (RTMPSoftDecryptAES(pAd, pRxBlk->pData, pRxWI->MPDUtotalByteCount , pShard_key))
		{

			
			pRxWI->MPDUtotalByteCount -= 16;
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ERROR : RTMPSoftDecryptAES Failed\n"));
			
			return NDIS_STATUS_FAILURE;
		}
	}
	else
	{
		
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;

}


PUINT8	GetSuiteFromRSNIE(
		IN	PUINT8	rsnie,
		IN	UINT	rsnie_len,
		IN	UINT8	type,
		OUT	UINT8	*count)
{
	PEID_STRUCT pEid;
	INT			len;
	PUINT8		pBuf;
	INT			offset = 0;
	PRSNIE_AUTH	pAkm;
	UINT16		acount;
	BOOLEAN		isWPA2 = FALSE;

	pEid = (PEID_STRUCT)rsnie;
	len = rsnie_len - 2;	
	pBuf = (PUINT8)&pEid->Octet[0];



	
	*count = 0;

	
	if ((len <= 0) || (pEid->Len != len))
	{
		DBGPRINT_ERR(("%s : The length is invalid\n", __FUNCTION__));
		return NULL;
	}

	
	if (pEid->Eid == IE_WPA)
	{
		PRSNIE	pRsnie = (PRSNIE)pBuf;
		UINT16 ucount;

		if (len < sizeof(RSNIE))
		{
			DBGPRINT_ERR(("%s : The length is too short for WPA\n", __FUNCTION__));
			return NULL;
		}

		
		ucount = cpu2le16(pRsnie->ucount);
		if (ucount > 2)
		{
			DBGPRINT_ERR(("%s : The count(%d) of pairwise cipher is invlaid\n",
											__FUNCTION__, ucount));
			return NULL;
		}

		
		if (type == GROUP_SUITE)
		{
			*count = 1;
			return pRsnie->mcast;
		}
		
		else if (type == PAIRWISE_SUITE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s : The count of pairwise cipher is %d\n",
										__FUNCTION__, ucount));
			*count = ucount;
			return pRsnie->ucast[0].oui;
		}

		offset = sizeof(RSNIE) + (4 * (ucount - 1));

	}
	else if (pEid->Eid == IE_RSN)
	{
		PRSNIE2	pRsnie = (PRSNIE2)pBuf;
		UINT16 ucount;

		isWPA2 = TRUE;

		if (len < sizeof(RSNIE2))
		{
			DBGPRINT_ERR(("%s : The length is too short for WPA2\n", __FUNCTION__));
			return NULL;
		}

		
		ucount = cpu2le16(pRsnie->ucount);
		if (ucount > 2)
		{
			DBGPRINT_ERR(("%s : The count(%d) of pairwise cipher is invlaid\n",
											__FUNCTION__, ucount));
			return NULL;
		}

		
		if (type == GROUP_SUITE)
		{
			*count = 1;
			return pRsnie->mcast;
		}
		
		else if (type == PAIRWISE_SUITE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s : The count of pairwise cipher is %d\n",
										__FUNCTION__, ucount));
			*count = ucount;
			return pRsnie->ucast[0].oui;
		}

		offset = sizeof(RSNIE2) + (4 * (ucount - 1));

	}
	else
	{
		DBGPRINT_ERR(("%s : Unknown IE (%d)\n", __FUNCTION__, pEid->Eid));
		return NULL;
	}

	
	pBuf += offset;
	len -= offset;

	if (len < sizeof(RSNIE_AUTH))
	{
		DBGPRINT_ERR(("%s : The length of RSNIE is too short\n", __FUNCTION__));
		return NULL;
	}

	
	pAkm = (PRSNIE_AUTH)pBuf;

	
	acount = cpu2le16(pAkm->acount);
	if (acount > 2)
	{
		DBGPRINT_ERR(("%s : The count(%d) of AKM is invlaid\n",
										__FUNCTION__, acount));
		return NULL;
	}

	
	if (type == AKM_SUITE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s : The count of AKM is %d\n",
									__FUNCTION__, acount));
		*count = acount;
		return pAkm->auth[0].oui;
	}
	offset = sizeof(RSNIE_AUTH) + (4 * (acount - 1));

	pBuf += offset;
	len -= offset;

	
	if (len >= (sizeof(RSN_CAPABILITIES) + 2 + LEN_PMKID))
	{
		
		pBuf += (sizeof(RSN_CAPABILITIES) + 2);
		len -= (sizeof(RSN_CAPABILITIES) + 2);

		
		if (type == PMKID_LIST)
		{
			*count = 1;
			return pBuf;
		}
	}
	else
	{
		DBGPRINT_ERR(("%s : it can't get any more information beyond AKM \n", __FUNCTION__));
		return NULL;
	}

	*count = 0;
	
	return NULL;

}

VOID WpaShowAllsuite(
	IN	PUINT8	rsnie,
	IN	UINT	rsnie_len)
{
	PUINT8 pSuite = NULL;
	UINT8 count;

	hex_dump("RSNIE", rsnie, rsnie_len);

	
	if ((pSuite = GetSuiteFromRSNIE(rsnie, rsnie_len, GROUP_SUITE, &count)) != NULL)
	{
		hex_dump("group cipher", pSuite, 4*count);
	}

	
	if ((pSuite = GetSuiteFromRSNIE(rsnie, rsnie_len, PAIRWISE_SUITE, &count)) != NULL)
	{
		hex_dump("pairwise cipher", pSuite, 4*count);
	}

	
	if ((pSuite = GetSuiteFromRSNIE(rsnie, rsnie_len, AKM_SUITE, &count)) != NULL)
	{
		hex_dump("AKM suite", pSuite, 4*count);
	}

	
	if ((pSuite = GetSuiteFromRSNIE(rsnie, rsnie_len, PMKID_LIST, &count)) != NULL)
	{
		hex_dump("PMKID", pSuite, LEN_PMKID);
	}

}

VOID RTMPInsertRSNIE(
	IN PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUINT8 rsnie_ptr,
	IN UINT8  rsnie_len,
	IN PUINT8 pmkid_ptr,
	IN UINT8  pmkid_len)
{
	PUCHAR	pTmpBuf;
	ULONG	TempLen = 0;
	UINT8	extra_len = 0;
	UINT16	pmk_count = 0;
	UCHAR	ie_num;
	UINT8	total_len = 0;
    UCHAR	WPA2_OUI[3]={0x00,0x0F,0xAC};

	pTmpBuf = pFrameBuf;

	
	if (pmkid_len > 0 && ((pmkid_len & 0x0f) == 0))
	{
		extra_len = sizeof(UINT16) + pmkid_len;

		pmk_count = (pmkid_len >> 4);
		pmk_count = cpu2le16(pmk_count);
	}
	else
	{
		DBGPRINT(RT_DEBUG_WARN, ("%s : The length is PMKID-List is invalid (%d), so don't insert it.\n",
									__FUNCTION__, pmkid_len));
	}

	if (rsnie_len != 0)
	{
		ie_num = IE_WPA;
		total_len = rsnie_len;

		if (NdisEqualMemory(rsnie_ptr + 2, WPA2_OUI, sizeof(WPA2_OUI)))
		{
			ie_num = IE_RSN;
			total_len += extra_len;
		}

		
		MakeOutgoingFrame(pTmpBuf,			&TempLen,
						  1,				&ie_num,
						  1,				&total_len,
						  rsnie_len,		rsnie_ptr,
						  END_OF_ARGS);

		pTmpBuf += TempLen;
		*pFrameLen = *pFrameLen + TempLen;

		if (ie_num == IE_RSN)
		{
			
			if (extra_len > 0)
			{
				MakeOutgoingFrame(pTmpBuf,					&TempLen,
								  2,						&pmk_count,
								  pmkid_len,				pmkid_ptr,
								  END_OF_ARGS);

				pTmpBuf += TempLen;
				*pFrameLen = *pFrameLen + TempLen;
			}
		}
	}

	return;
}
