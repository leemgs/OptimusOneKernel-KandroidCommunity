

#include "../rt_config.h"
#include "action.h"


static VOID ReservedAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);


VOID ActionStateMachineInit(
    IN	PRTMP_ADAPTER	pAd,
    IN  STATE_MACHINE *S,
    OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(S, (STATE_MACHINE_FUNC *)Trans, MAX_ACT_STATE, MAX_ACT_MSG, (STATE_MACHINE_FUNC)Drop, ACT_IDLE, ACT_MACHINE_BASE);

	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_SPECTRUM_CATE, (STATE_MACHINE_FUNC)PeerSpectrumAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_QOS_CATE, (STATE_MACHINE_FUNC)PeerQOSAction);

	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_DLS_CATE, (STATE_MACHINE_FUNC)ReservedAction);

	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_BA_CATE, (STATE_MACHINE_FUNC)PeerBAAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_HT_CATE, (STATE_MACHINE_FUNC)PeerHTAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_ADD_BA_CATE, (STATE_MACHINE_FUNC)MlmeADDBAAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_ORI_DELBA_CATE, (STATE_MACHINE_FUNC)MlmeDELBAAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_REC_DELBA_CATE, (STATE_MACHINE_FUNC)MlmeDELBAAction);

	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_PUBLIC_CATE, (STATE_MACHINE_FUNC)PeerPublicAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_RM_CATE, (STATE_MACHINE_FUNC)PeerRMAction);

	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_QOS_CATE, (STATE_MACHINE_FUNC)MlmeQOSAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_DLS_CATE, (STATE_MACHINE_FUNC)MlmeDLSAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_ACT_INVALID, (STATE_MACHINE_FUNC)MlmeInvalidAction);
}

VOID MlmeADDBAAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)

{
	MLME_ADDBA_REQ_STRUCT *pInfo;
	UCHAR           Addr[6];
	PUCHAR         pOutBuffer = NULL;
	NDIS_STATUS     NStatus;
	ULONG		Idx;
	FRAME_ADDBA_REQ  Frame;
	ULONG		FrameLen;
	BA_ORI_ENTRY			*pBAEntry = NULL;

	pInfo = (MLME_ADDBA_REQ_STRUCT *)Elem->Msg;
	NdisZeroMemory(&Frame, sizeof(FRAME_ADDBA_REQ));

	if(MlmeAddBAReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr))
	{
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  
		if(NStatus != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_TRACE,("BA - MlmeADDBAAction() allocate memory failed \n"));
			return;
		}
		
		Idx = pAd->MacTab.Content[pInfo->Wcid].BAOriWcidArray[pInfo->TID];
		if (Idx == 0)
		{
			MlmeFreeMemory(pAd, pOutBuffer);
			DBGPRINT(RT_DEBUG_ERROR,("BA - MlmeADDBAAction() can't find BAOriEntry \n"));
			return;
		}
		else
		{
			pBAEntry =&pAd->BATable.BAOriEntry[Idx];
		}

		{
			if (ADHOC_ON(pAd))
				ActHeaderInit(pAd, &Frame.Hdr, pInfo->pAddr, pAd->CurrentAddress, pAd->CommonCfg.Bssid);
			else
				ActHeaderInit(pAd, &Frame.Hdr, pAd->CommonCfg.Bssid, pAd->CurrentAddress, pInfo->pAddr);
		}

		Frame.Category = CATEGORY_BA;
		Frame.Action = ADDBA_REQ;
		Frame.BaParm.AMSDUSupported = 0;
		Frame.BaParm.BAPolicy = IMMED_BA;
		Frame.BaParm.TID = pInfo->TID;
		Frame.BaParm.BufSize = pInfo->BaBufSize;
		Frame.Token = pInfo->Token;
		Frame.TimeOutValue = pInfo->TimeOutValue;
		Frame.BaStartSeq.field.FragNum = 0;
		Frame.BaStartSeq.field.StartSeq = pAd->MacTab.Content[pInfo->Wcid].TxSeq[pInfo->TID];

		*(USHORT *)(&Frame.BaParm) = cpu2le16(*(USHORT *)(&Frame.BaParm));
		Frame.TimeOutValue = cpu2le16(Frame.TimeOutValue);
		Frame.BaStartSeq.word = cpu2le16(Frame.BaStartSeq.word);

		MakeOutgoingFrame(pOutBuffer,		   &FrameLen,
		              sizeof(FRAME_ADDBA_REQ), &Frame,
		              END_OF_ARGS);
		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);

		DBGPRINT(RT_DEBUG_TRACE, ("BA - Send ADDBA request. StartSeq = %x,  FrameLen = %ld. BufSize = %d\n", Frame.BaStartSeq.field.StartSeq, FrameLen, Frame.BaParm.BufSize));
    }
}


VOID MlmeDELBAAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
	MLME_DELBA_REQ_STRUCT *pInfo;
	PUCHAR         pOutBuffer = NULL;
	PUCHAR		   pOutBuffer2 = NULL;
	NDIS_STATUS     NStatus;
	ULONG		Idx;
	FRAME_DELBA_REQ  Frame;
	ULONG		FrameLen;
	FRAME_BAR	FrameBar;

	pInfo = (MLME_DELBA_REQ_STRUCT *)Elem->Msg;
	
	NdisZeroMemory(&Frame, sizeof(FRAME_DELBA_REQ));
	DBGPRINT(RT_DEBUG_TRACE, ("==> MlmeDELBAAction(), Initiator(%d) \n", pInfo->Initiator));

	if(MlmeDelBAReqSanity(pAd, Elem->Msg, Elem->MsgLen))
	{
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  
		if(NStatus != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_ERROR,("BA - MlmeDELBAAction() allocate memory failed 1. \n"));
			return;
		}

		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer2);  
		if(NStatus != NDIS_STATUS_SUCCESS)
		{
			MlmeFreeMemory(pAd, pOutBuffer);
			DBGPRINT(RT_DEBUG_ERROR, ("BA - MlmeDELBAAction() allocate memory failed 2. \n"));
			return;
		}

		
		Idx = pAd->MacTab.Content[pInfo->Wcid].BAOriWcidArray[pInfo->TID];

		BarHeaderInit(pAd, &FrameBar, pAd->MacTab.Content[pInfo->Wcid].Addr, pAd->CurrentAddress);

		FrameBar.StartingSeq.field.FragNum = 0; 
		FrameBar.StartingSeq.field.StartSeq = pAd->MacTab.Content[pInfo->Wcid].TxSeq[pInfo->TID]; 
		FrameBar.BarControl.TID = pInfo->TID; 
		FrameBar.BarControl.ACKPolicy = IMMED_BA; 
		FrameBar.BarControl.Compressed = 1; 
		FrameBar.BarControl.MTID = 0; 

		MakeOutgoingFrame(pOutBuffer2,				&FrameLen,
					  sizeof(FRAME_BAR),	  &FrameBar,
					  END_OF_ARGS);
		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer2, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer2);
		DBGPRINT(RT_DEBUG_TRACE,("BA - MlmeDELBAAction() . Send BAR to refresh peer reordering buffer \n"));

		
		FrameLen = 0;

		{
			if (ADHOC_ON(pAd))
				ActHeaderInit(pAd, &Frame.Hdr, pAd->MacTab.Content[pInfo->Wcid].Addr, pAd->CurrentAddress, pAd->CommonCfg.Bssid);
			else
				ActHeaderInit(pAd, &Frame.Hdr,  pAd->CommonCfg.Bssid, pAd->CurrentAddress, pAd->MacTab.Content[pInfo->Wcid].Addr);
		}

		Frame.Category = CATEGORY_BA;
		Frame.Action = DELBA;
		Frame.DelbaParm.Initiator = pInfo->Initiator;
		Frame.DelbaParm.TID = pInfo->TID;
		Frame.ReasonCode = 39; 
		*(USHORT *)(&Frame.DelbaParm) = cpu2le16(*(USHORT *)(&Frame.DelbaParm));
		Frame.ReasonCode = cpu2le16(Frame.ReasonCode);

		MakeOutgoingFrame(pOutBuffer,               &FrameLen,
		              sizeof(FRAME_DELBA_REQ),    &Frame,
		              END_OF_ARGS);
		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);
		DBGPRINT(RT_DEBUG_TRACE, ("BA - MlmeDELBAAction() . 3 DELBA sent. Initiator(%d)\n", pInfo->Initiator));
    	}
}

VOID MlmeQOSAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
}

VOID MlmeDLSAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
}

VOID MlmeInvalidAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
	
	
}

VOID PeerQOSAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
}

VOID PeerBAAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR	Action = Elem->Msg[LENGTH_802_11+1];

	switch(Action)
	{
		case ADDBA_REQ:
			PeerAddBAReqAction(pAd,Elem);
			break;
		case ADDBA_RESP:
			PeerAddBARspAction(pAd,Elem);
			break;
		case DELBA:
			PeerDelBAAction(pAd,Elem);
			break;
	}
}

VOID PeerPublicAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	if (Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
		return;
}


static VOID ReservedAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR Category;

	if (Elem->MsgLen <= LENGTH_802_11)
	{
		return;
	}

	Category = Elem->Msg[LENGTH_802_11];
	DBGPRINT(RT_DEBUG_TRACE,("Rcv reserved category(%d) Action Frame\n", Category));
	hex_dump("Reserved Action Frame", &Elem->Msg[0], Elem->MsgLen);
}

VOID PeerRMAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)

{
	return;
}

static VOID respond_ht_information_exchange_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PUCHAR			pOutBuffer = NULL;
	NDIS_STATUS		NStatus;
	ULONG			FrameLen;
	FRAME_HT_INFO	HTINFOframe, *pFrame;
	UCHAR   		*pAddr;


	
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	 

	if (NStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE,("ACTION - respond_ht_information_exchange_action() allocate memory failed \n"));
		return;
	}

	
	pFrame = (FRAME_HT_INFO *) &Elem->Msg[0];
	pAddr = pFrame->Hdr.Addr2;

	NdisZeroMemory(&HTINFOframe, sizeof(FRAME_HT_INFO));
	
	{
		if (ADHOC_ON(pAd))
			ActHeaderInit(pAd, &HTINFOframe.Hdr, pAddr, pAd->CurrentAddress, pAd->CommonCfg.Bssid);
		else
			ActHeaderInit(pAd, &HTINFOframe.Hdr, pAd->CommonCfg.Bssid, pAd->CurrentAddress, pAddr);
	}

	HTINFOframe.Category = CATEGORY_HT;
	HTINFOframe.Action = HT_INFO_EXCHANGE;
	HTINFOframe.HT_Info.Request = 0;
	HTINFOframe.HT_Info.Forty_MHz_Intolerant = pAd->CommonCfg.HtCapability.HtCapInfo.Forty_Mhz_Intolerant;
	HTINFOframe.HT_Info.STA_Channel_Width	 = pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth;

	MakeOutgoingFrame(pOutBuffer,					&FrameLen,
					  sizeof(FRAME_HT_INFO),	&HTINFOframe,
					  END_OF_ARGS);

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);
}

VOID PeerHTAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR	Action = Elem->Msg[LENGTH_802_11+1];

	if (Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
		return;

	switch(Action)
	{
		case NOTIFY_BW_ACTION:
			DBGPRINT(RT_DEBUG_TRACE,("ACTION - HT Notify Channel bandwidth action----> \n"));

			if(pAd->StaActive.SupportedPhyInfo.bHtEnable == FALSE)
			{
				
				
				
				DBGPRINT(RT_DEBUG_TRACE,("ACTION -Ignore HT Notify Channel BW when link as legacy mode. BW = %d---> \n",
								Elem->Msg[LENGTH_802_11+2] ));
				break;
			}

			if (Elem->Msg[LENGTH_802_11+2] == 0)	
				pAd->MacTab.Content[Elem->Wcid].HTPhyMode.field.BW = 0;

			break;
		case SMPS_ACTION:
			
 			DBGPRINT(RT_DEBUG_TRACE,("ACTION - SMPS action----> \n"));
			if (((Elem->Msg[LENGTH_802_11+2]&0x1) == 0))
			{
				pAd->MacTab.Content[Elem->Wcid].MmpsMode = MMPS_ENABLE;
			}
			else if (((Elem->Msg[LENGTH_802_11+2]&0x2) == 0))
			{
				pAd->MacTab.Content[Elem->Wcid].MmpsMode = MMPS_STATIC;
			}
			else
			{
				pAd->MacTab.Content[Elem->Wcid].MmpsMode = MMPS_DYNAMIC;
			}

			DBGPRINT(RT_DEBUG_TRACE,("Aid(%d) MIMO PS = %d\n", Elem->Wcid, pAd->MacTab.Content[Elem->Wcid].MmpsMode));
			
			break;

		case SETPCO_ACTION:
			break;
		case MIMO_CHA_MEASURE_ACTION:
			break;
		case HT_INFO_EXCHANGE:
			{
				HT_INFORMATION_OCTET	*pHT_info;

				pHT_info = (HT_INFORMATION_OCTET *) &Elem->Msg[LENGTH_802_11+2];
    				
    				DBGPRINT(RT_DEBUG_TRACE,("ACTION - HT Information Exchange action----> \n"));
    				if (pHT_info->Request)
    				{
    					respond_ht_information_exchange_action(pAd, Elem);
    				}
			}
    			break;
	}
}



VOID ORIBATimerTimeout(
	IN	PRTMP_ADAPTER	pAd)
{
	MAC_TABLE_ENTRY	*pEntry;
	INT			i, total;
	UCHAR			TID;

	total = pAd->MacTab.Size * NUM_OF_TID;

	for (i = 1; ((i <MAX_LEN_OF_BA_ORI_TABLE) && (total > 0)) ; i++)
	{
		if  (pAd->BATable.BAOriEntry[i].ORI_BA_Status == Originator_Done)
		{
			pEntry = &pAd->MacTab.Content[pAd->BATable.BAOriEntry[i].Wcid];
			TID = pAd->BATable.BAOriEntry[i].TID;

			ASSERT(pAd->BATable.BAOriEntry[i].Wcid < MAX_LEN_OF_MAC_TABLE);
		}
		total --;
	}
}


VOID SendRefreshBAR(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry)
{
	FRAME_BAR		FrameBar;
	ULONG			FrameLen;
	NDIS_STATUS 	NStatus;
	PUCHAR			pOutBuffer = NULL;
	USHORT			Sequence;
	UCHAR			i, TID;
	USHORT			idx;
	BA_ORI_ENTRY	*pBAEntry;

	for (i = 0; i <NUM_OF_TID; i++)
	{
		idx = pEntry->BAOriWcidArray[i];
		if (idx == 0)
		{
			continue;
		}
		pBAEntry = &pAd->BATable.BAOriEntry[idx];

		if  (pBAEntry->ORI_BA_Status == Originator_Done)
		{
			TID = pBAEntry->TID;

			ASSERT(pBAEntry->Wcid < MAX_LEN_OF_MAC_TABLE);

			NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  
			if(NStatus != NDIS_STATUS_SUCCESS)
			{
				DBGPRINT(RT_DEBUG_ERROR,("BA - MlmeADDBAAction() allocate memory failed \n"));
				return;
			}

			Sequence = pEntry->TxSeq[TID];

			BarHeaderInit(pAd, &FrameBar, pEntry->Addr, pAd->CurrentAddress);

			FrameBar.StartingSeq.field.FragNum = 0; 
			FrameBar.StartingSeq.field.StartSeq = Sequence; 
			FrameBar.BarControl.TID = TID; 

			MakeOutgoingFrame(pOutBuffer,				&FrameLen,
							  sizeof(FRAME_BAR),	  &FrameBar,
							  END_OF_ARGS);

			MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);

			MlmeFreeMemory(pAd, pOutBuffer);
		}
	}
}

VOID ActHeaderInit(
    IN	PRTMP_ADAPTER	pAd,
    IN OUT PHEADER_802_11 pHdr80211,
    IN PUCHAR Addr1,
    IN PUCHAR Addr2,
    IN PUCHAR Addr3)
{
    NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
    pHdr80211->FC.Type = BTYPE_MGMT;
    pHdr80211->FC.SubType = SUBTYPE_ACTION;

    COPY_MAC_ADDR(pHdr80211->Addr1, Addr1);
	COPY_MAC_ADDR(pHdr80211->Addr2, Addr2);
    COPY_MAC_ADDR(pHdr80211->Addr3, Addr3);
}

VOID BarHeaderInit(
	IN	PRTMP_ADAPTER	pAd,
	IN OUT PFRAME_BAR pCntlBar,
	IN PUCHAR pDA,
	IN PUCHAR pSA)
{
	NdisZeroMemory(pCntlBar, sizeof(FRAME_BAR));
	pCntlBar->FC.Type = BTYPE_CNTL;
	pCntlBar->FC.SubType = SUBTYPE_BLOCK_ACK_REQ;
   	pCntlBar->BarControl.MTID = 0;
	pCntlBar->BarControl.Compressed = 1;
	pCntlBar->BarControl.ACKPolicy = 0;


	pCntlBar->Duration = 16 + RTMPCalcDuration(pAd, RATE_1, sizeof(FRAME_BA));

	COPY_MAC_ADDR(pCntlBar->Addr1, pDA);
	COPY_MAC_ADDR(pCntlBar->Addr2, pSA);
}



VOID InsertActField(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Category,
	IN UINT8 ActCode)
{
	ULONG TempLen;

	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&Category,
						1,				&ActCode,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}
