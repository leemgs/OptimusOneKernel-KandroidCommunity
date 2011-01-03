

#include "../rt_config.h"


extern UCHAR	CISCO_OUI[];

extern UCHAR	WPA_OUI[];
extern UCHAR	RSN_OUI[];
extern UCHAR	WME_INFO_ELEM[];
extern UCHAR	WME_PARM_ELEM[];
extern UCHAR	Ccx2QosInfo[];
extern UCHAR	RALINK_OUI[];
extern UCHAR	BROADCOM_OUI[];
extern UCHAR    WPS_OUI[];


BOOLEAN MlmeAddBAReqSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2)
{
    PMLME_ADDBA_REQ_STRUCT   pInfo;

    pInfo = (MLME_ADDBA_REQ_STRUCT *)Msg;

    if ((MsgLen != sizeof(MLME_ADDBA_REQ_STRUCT)))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeAddBAReqSanity fail - message lenght not correct.\n"));
        return FALSE;
    }

    if ((pInfo->Wcid >= MAX_LEN_OF_MAC_TABLE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeAddBAReqSanity fail - The peer Mac is not associated yet.\n"));
        return FALSE;
    }

	

    if ((pInfo->pAddr[0]&0x01) == 0x01)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeAddBAReqSanity fail - broadcast address not support BA\n"));
        return FALSE;
    }

    return TRUE;
}


BOOLEAN MlmeDelBAReqSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen)
{
	MLME_DELBA_REQ_STRUCT *pInfo;
	pInfo = (MLME_DELBA_REQ_STRUCT *)Msg;

    if ((MsgLen != sizeof(MLME_DELBA_REQ_STRUCT)))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("MlmeDelBAReqSanity fail - message lenght not correct.\n"));
        return FALSE;
    }

    if ((pInfo->Wcid >= MAX_LEN_OF_MAC_TABLE))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("MlmeDelBAReqSanity fail - The peer Mac is not associated yet.\n"));
        return FALSE;
    }

    if ((pInfo->TID & 0xf0))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("MlmeDelBAReqSanity fail - The peer TID is incorrect.\n"));
        return FALSE;
    }

	if (NdisEqualMemory(pAd->MacTab.Content[pInfo->Wcid].Addr, pInfo->Addr, MAC_ADDR_LEN) == 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("MlmeDelBAReqSanity fail - the peer addr dosen't exist.\n"));
        return FALSE;
    }

    return TRUE;
}

BOOLEAN PeerAddBAReqActionSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *pMsg,
    IN ULONG MsgLen,
	OUT PUCHAR pAddr2)
{
	PFRAME_802_11 pFrame = (PFRAME_802_11)pMsg;
	PFRAME_ADDBA_REQ pAddFrame;
	pAddFrame = (PFRAME_ADDBA_REQ)(pMsg);
	if (MsgLen < (sizeof(FRAME_ADDBA_REQ)))
	{
		DBGPRINT(RT_DEBUG_ERROR,("PeerAddBAReqActionSanity: ADDBA Request frame length size = %ld incorrect\n", MsgLen));
		return FALSE;
	}
	
#ifdef UNALIGNMENT_SUPPORT
	{
		BA_PARM		tmpBaParm;

		NdisMoveMemory((PUCHAR)(&tmpBaParm), (PUCHAR)(&pAddFrame->BaParm), sizeof(BA_PARM));
		*(USHORT *)(&tmpBaParm) = cpu2le16(*(USHORT *)(&tmpBaParm));
		NdisMoveMemory((PUCHAR)(&pAddFrame->BaParm), (PUCHAR)(&tmpBaParm), sizeof(BA_PARM));
	}
#else
	*(USHORT *)(&pAddFrame->BaParm) = cpu2le16(*(USHORT *)(&pAddFrame->BaParm));
#endif
	pAddFrame->TimeOutValue = cpu2le16(pAddFrame->TimeOutValue);
	pAddFrame->BaStartSeq.word = cpu2le16(pAddFrame->BaStartSeq.word);

	if (pAddFrame->BaParm.BAPolicy != IMMED_BA)
	{
		DBGPRINT(RT_DEBUG_ERROR,("PeerAddBAReqActionSanity: ADDBA Request Ba Policy[%d] not support\n", pAddFrame->BaParm.BAPolicy));
		DBGPRINT(RT_DEBUG_ERROR,("ADDBA Request. tid=%x, Bufsize=%x, AMSDUSupported=%x \n", pAddFrame->BaParm.TID, pAddFrame->BaParm.BufSize, pAddFrame->BaParm.AMSDUSupported));
		return FALSE;
	}

	
	if (pAddFrame->BaParm.TID &0xfff0)
	{
		DBGPRINT(RT_DEBUG_ERROR,("PeerAddBAReqActionSanity: ADDBA Request incorrect TID = %d\n", pAddFrame->BaParm.TID));
		return FALSE;
	}
	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
	return TRUE;
}

BOOLEAN PeerAddBARspActionSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *pMsg,
    IN ULONG MsgLen)
{
	
	PFRAME_ADDBA_RSP pAddFrame;

	pAddFrame = (PFRAME_ADDBA_RSP)(pMsg);
	if (MsgLen < (sizeof(FRAME_ADDBA_RSP)))
	{
		DBGPRINT(RT_DEBUG_ERROR,("PeerAddBARspActionSanity: ADDBA Response frame length size = %ld incorrect\n", MsgLen));
		return FALSE;
	}
	
#ifdef UNALIGNMENT_SUPPORT
	{
		BA_PARM		tmpBaParm;

		NdisMoveMemory((PUCHAR)(&tmpBaParm), (PUCHAR)(&pAddFrame->BaParm), sizeof(BA_PARM));
		*(USHORT *)(&tmpBaParm) = cpu2le16(*(USHORT *)(&tmpBaParm));
		NdisMoveMemory((PUCHAR)(&pAddFrame->BaParm), (PUCHAR)(&tmpBaParm), sizeof(BA_PARM));
	}
#else
	*(USHORT *)(&pAddFrame->BaParm) = cpu2le16(*(USHORT *)(&pAddFrame->BaParm));
#endif
	pAddFrame->StatusCode = cpu2le16(pAddFrame->StatusCode);
	pAddFrame->TimeOutValue = cpu2le16(pAddFrame->TimeOutValue);

	if (pAddFrame->BaParm.BAPolicy != IMMED_BA)
	{
		DBGPRINT(RT_DEBUG_ERROR,("PeerAddBAReqActionSanity: ADDBA Response Ba Policy[%d] not support\n", pAddFrame->BaParm.BAPolicy));
		return FALSE;
	}

	
	if (pAddFrame->BaParm.TID &0xfff0)
	{
		DBGPRINT(RT_DEBUG_ERROR,("PeerAddBARspActionSanity: ADDBA Response incorrect TID = %d\n", pAddFrame->BaParm.TID));
		return FALSE;
	}
	return TRUE;

}

BOOLEAN PeerDelBAActionSanity(
    IN PRTMP_ADAPTER pAd,
    IN UCHAR Wcid,
    IN VOID *pMsg,
    IN ULONG MsgLen )
{
	
	PFRAME_DELBA_REQ  pDelFrame;
	if (MsgLen != (sizeof(FRAME_DELBA_REQ)))
		return FALSE;

	if (Wcid >= MAX_LEN_OF_MAC_TABLE)
		return FALSE;

	pDelFrame = (PFRAME_DELBA_REQ)(pMsg);

	*(USHORT *)(&pDelFrame->DelbaParm) = cpu2le16(*(USHORT *)(&pDelFrame->DelbaParm));
	pDelFrame->ReasonCode = cpu2le16(pDelFrame->ReasonCode);

	if (pDelFrame->DelbaParm.TID &0xfff0)
		return FALSE;

	return TRUE;
}


BOOLEAN PeerBeaconAndProbeRspSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    IN UCHAR  MsgChannel,
    OUT PUCHAR pAddr2,
    OUT PUCHAR pBssid,
    OUT CHAR Ssid[],
    OUT UCHAR *pSsidLen,
    OUT UCHAR *pBssType,
    OUT USHORT *pBeaconPeriod,
    OUT UCHAR *pChannel,
    OUT UCHAR *pNewChannel,
    OUT LARGE_INTEGER *pTimestamp,
    OUT CF_PARM *pCfParm,
    OUT USHORT *pAtimWin,
    OUT USHORT *pCapabilityInfo,
    OUT UCHAR *pErp,
    OUT UCHAR *pDtimCount,
    OUT UCHAR *pDtimPeriod,
    OUT UCHAR *pBcastFlag,
    OUT UCHAR *pMessageToMe,
    OUT UCHAR SupRate[],
    OUT UCHAR *pSupRateLen,
    OUT UCHAR ExtRate[],
    OUT UCHAR *pExtRateLen,
    OUT	UCHAR *pCkipFlag,
    OUT	UCHAR *pAironetCellPowerLimit,
    OUT PEDCA_PARM       pEdcaParm,
    OUT PQBSS_LOAD_PARM  pQbssLoad,
    OUT PQOS_CAPABILITY_PARM pQosCapability,
    OUT ULONG *pRalinkIe,
    OUT UCHAR		 *pHtCapabilityLen,
#ifdef CONFIG_STA_SUPPORT
    OUT UCHAR		 *pPreNHtCapabilityLen,
#endif 
    OUT HT_CAPABILITY_IE *pHtCapability,
	OUT UCHAR		 *AddHtInfoLen,
	OUT ADD_HT_INFO_IE *AddHtInfo,
	OUT UCHAR *NewExtChannelOffset,		
    OUT USHORT *LengthVIE,
    OUT	PNDIS_802_11_VARIABLE_IEs pVIE)
{
    UCHAR				*Ptr;
#ifdef CONFIG_STA_SUPPORT
	UCHAR				TimLen;
#endif 
    PFRAME_802_11		pFrame;
    PEID_STRUCT         pEid;
    UCHAR				SubType;
    UCHAR				Sanity;
    
    
    ULONG				Length = 0;

	
	
	
	UCHAR			CtrlChannel = 0;

    
    Sanity = 0;

    *pAtimWin = 0;
    *pErp = 0;
    *pDtimCount = 0;
    *pDtimPeriod = 0;
    *pBcastFlag = 0;
    *pMessageToMe = 0;
    *pExtRateLen = 0;
    *pCkipFlag = 0;			        
    *pAironetCellPowerLimit = 0xFF;  
    *LengthVIE = 0;					
    *pHtCapabilityLen = 0;					
#ifdef CONFIG_STA_SUPPORT
	if (pAd->OpMode == OPMODE_STA)
		*pPreNHtCapabilityLen = 0;					
#endif 
    *AddHtInfoLen = 0;					
    *pRalinkIe = 0;
    *pNewChannel = 0;
    *NewExtChannelOffset = 0xff;	
    pCfParm->bValid = FALSE;        
    pQbssLoad->bValid = FALSE;      
    pEdcaParm->bValid = FALSE;      
    pQosCapability->bValid = FALSE; 

    pFrame = (PFRAME_802_11)Msg;

    
    SubType = (UCHAR)pFrame->Hdr.FC.SubType;

    
    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
    COPY_MAC_ADDR(pBssid, pFrame->Hdr.Addr3);



    Ptr = pFrame->Octet;
    Length += LENGTH_802_11;

    
    NdisMoveMemory(pTimestamp, Ptr, TIMESTAMP_LEN);

	pTimestamp->u.LowPart = cpu2le32(pTimestamp->u.LowPart);
	pTimestamp->u.HighPart = cpu2le32(pTimestamp->u.HighPart);

    Ptr += TIMESTAMP_LEN;
    Length += TIMESTAMP_LEN;

    
    NdisMoveMemory(pBeaconPeriod, Ptr, 2);
    Ptr += 2;
    Length += 2;

    
    NdisMoveMemory(pCapabilityInfo, Ptr, 2);
    Ptr += 2;
    Length += 2;

    if (CAP_IS_ESS_ON(*pCapabilityInfo))
        *pBssType = BSS_INFRA;
    else
        *pBssType = BSS_ADHOC;

    pEid = (PEID_STRUCT) Ptr;

    
    while ((Length + 2 + pEid->Len) <= MsgLen)
    {
        
        
        
        if ((*LengthVIE + pEid->Len + 2) >= MAX_VIE_LEN)
        {
            DBGPRINT(RT_DEBUG_WARN, ("PeerBeaconAndProbeRspSanity - Variable IEs out of resource [len(=%d) > MAX_VIE_LEN(=%d)]\n",
                    (*LengthVIE + pEid->Len + 2), MAX_VIE_LEN));
            break;
        }

        switch(pEid->Eid)
        {
            case IE_SSID:
                
                if (Sanity & 0x1)
                    break;
                if(pEid->Len <= MAX_LEN_OF_SSID)
                {
                    NdisMoveMemory(Ssid, pEid->Octet, pEid->Len);
                    *pSsidLen = pEid->Len;
                    Sanity |= 0x1;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAndProbeRspSanity - wrong IE_SSID (len=%d)\n",pEid->Len));
                    return FALSE;
                }
                break;

            case IE_SUPP_RATES:
                if(pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    Sanity |= 0x2;
                    NdisMoveMemory(SupRate, pEid->Octet, pEid->Len);
                    *pSupRateLen = pEid->Len;

                    
                    
                    
                    
                    
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAndProbeRspSanity - wrong IE_SUPP_RATES (len=%d)\n",pEid->Len));
                    return FALSE;
                }
                break;

            case IE_HT_CAP:
			if (pEid->Len >= SIZE_HT_CAP_IE)  
			{
				NdisMoveMemory(pHtCapability, pEid->Octet, sizeof(HT_CAPABILITY_IE));
				*pHtCapabilityLen = SIZE_HT_CAP_IE;	

				*(USHORT *)(&pHtCapability->HtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
				{
					EXT_HT_CAP_INFO extHtCapInfo;
					NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&pHtCapability->ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
					*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
					NdisMoveMemory((PUCHAR)(&pHtCapability->ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
				}
#else
				*(USHORT *)(&pHtCapability->ExtHtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->ExtHtCapInfo));
#endif 

#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				{
					*pPreNHtCapabilityLen = 0;	

					Ptr = (PUCHAR) pVIE;
					NdisMoveMemory(Ptr + *LengthVIE, &pEid->Eid, pEid->Len + 2);
					*LengthVIE += (pEid->Len + 2);
				}
#endif 
			}
			else
			{
				DBGPRINT(RT_DEBUG_WARN, ("PeerBeaconAndProbeRspSanity - wrong IE_HT_CAP. pEid->Len = %d\n", pEid->Len));
			}

		break;
            case IE_ADD_HT:
			if (pEid->Len >= sizeof(ADD_HT_INFO_IE))
			{
				
				
				NdisMoveMemory(AddHtInfo, pEid->Octet, sizeof(ADD_HT_INFO_IE));
				*AddHtInfoLen = SIZE_ADD_HT_INFO_IE;

				CtrlChannel = AddHtInfo->ControlChan;

				*(USHORT *)(&AddHtInfo->AddHtInfo2) = cpu2le16(*(USHORT *)(&AddHtInfo->AddHtInfo2));
				*(USHORT *)(&AddHtInfo->AddHtInfo3) = cpu2le16(*(USHORT *)(&AddHtInfo->AddHtInfo3));

#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				{
			                Ptr = (PUCHAR) pVIE;
			                NdisMoveMemory(Ptr + *LengthVIE, &pEid->Eid, pEid->Len + 2);
			                *LengthVIE += (pEid->Len + 2);
				}
#endif 
			}
			else
			{
				DBGPRINT(RT_DEBUG_WARN, ("PeerBeaconAndProbeRspSanity - wrong IE_ADD_HT. \n"));
			}

		break;
            case IE_SECONDARY_CH_OFFSET:
			if (pEid->Len == 1)
			{
				*NewExtChannelOffset = pEid->Octet[0];
			}
			else
			{
				DBGPRINT(RT_DEBUG_WARN, ("PeerBeaconAndProbeRspSanity - wrong IE_SECONDARY_CH_OFFSET. \n"));
			}

		break;
            case IE_FH_PARM:
                DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAndProbeRspSanity(IE_FH_PARM) \n"));
                break;

            case IE_DS_PARM:
                if(pEid->Len == 1)
                {
                    *pChannel = *pEid->Octet;
#ifdef CONFIG_STA_SUPPORT
					IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
					{
						if (ChannelSanity(pAd, *pChannel) == 0)
						{

							return FALSE;
						}
					}
#endif 
                    Sanity |= 0x4;
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAndProbeRspSanity - wrong IE_DS_PARM (len=%d)\n",pEid->Len));
                    return FALSE;
                }
                break;

            case IE_CF_PARM:
                if(pEid->Len == 6)
                {
                    pCfParm->bValid = TRUE;
                    pCfParm->CfpCount = pEid->Octet[0];
                    pCfParm->CfpPeriod = pEid->Octet[1];
                    pCfParm->CfpMaxDuration = pEid->Octet[2] + 256 * pEid->Octet[3];
                    pCfParm->CfpDurRemaining = pEid->Octet[4] + 256 * pEid->Octet[5];
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAndProbeRspSanity - wrong IE_CF_PARM\n"));
                    return FALSE;
                }
                break;

            case IE_IBSS_PARM:
                if(pEid->Len == 2)
                {
                    NdisMoveMemory(pAtimWin, pEid->Octet, pEid->Len);
                }
                else
                {
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAndProbeRspSanity - wrong IE_IBSS_PARM\n"));
                    return FALSE;
                }
                break;

#ifdef CONFIG_STA_SUPPORT
            case IE_TIM:
                if(INFRA_ON(pAd) && SubType == SUBTYPE_BEACON)
                {
                    GetTimBit((PCHAR)pEid, pAd->StaActive.Aid, &TimLen, pBcastFlag, pDtimCount, pDtimPeriod, pMessageToMe);
                }
                break;
#endif 
            case IE_CHANNEL_SWITCH_ANNOUNCEMENT:
                if(pEid->Len == 3)
                {
			*pNewChannel = pEid->Octet[1];	
                }
                break;

            
            
            
            
            case IE_VENDOR_SPECIFIC:
                
                
                
                
                if (NdisEqualMemory(pEid->Octet, RALINK_OUI, 3) && (pEid->Len == 7))
                {
                    
                    if (pEid->Octet[3] != 0)
					*pRalinkIe = pEid->Octet[3];
				else
					*pRalinkIe = 0xf0000000; 
                }
#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11_N_SUPPORT
		

                
                
                else if ((*pHtCapabilityLen == 0) && NdisEqualMemory(pEid->Octet, PRE_N_HT_OUI, 3) && (pEid->Len >= 4) && (pAd->OpMode == OPMODE_STA))
                {
                    if ((pEid->Octet[3] == OUI_PREN_HT_CAP) && (pEid->Len >= 30) && (*pHtCapabilityLen == 0))
                    {
                        NdisMoveMemory(pHtCapability, &pEid->Octet[4], sizeof(HT_CAPABILITY_IE));
                        *pPreNHtCapabilityLen = SIZE_HT_CAP_IE;
                    }

                    if ((pEid->Octet[3] == OUI_PREN_ADD_HT) && (pEid->Len >= 26))
                    {
                        NdisMoveMemory(AddHtInfo, &pEid->Octet[4], sizeof(ADD_HT_INFO_IE));
                        *AddHtInfoLen = SIZE_ADD_HT_INFO_IE;
                    }
                }
#endif 
#endif 
                else if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4))
                {
                    
                    Ptr = (PUCHAR) pVIE;
                    NdisMoveMemory(Ptr + *LengthVIE, &pEid->Eid, pEid->Len + 2);
                    *LengthVIE += (pEid->Len + 2);
                }
                else if (NdisEqualMemory(pEid->Octet, WME_PARM_ELEM, 6) && (pEid->Len == 24))
                {
                    PUCHAR ptr;
                    int i;

                    
                    pEdcaParm->bValid          = TRUE;
                    pEdcaParm->bQAck           = FALSE; 
                    pEdcaParm->bQueueRequest   = FALSE; 
                    pEdcaParm->bTxopRequest    = FALSE; 
                    pEdcaParm->EdcaUpdateCount = pEid->Octet[6] & 0x0f;
                    pEdcaParm->bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;
                    ptr = &pEid->Octet[8];
                    for (i=0; i<4; i++)
                    {
                        UCHAR aci = (*ptr & 0x60) >> 5; 
                        pEdcaParm->bACM[aci]  = (((*ptr) & 0x10) == 0x10);   
                        pEdcaParm->Aifsn[aci] = (*ptr) & 0x0f;               
                        pEdcaParm->Cwmin[aci] = *(ptr+1) & 0x0f;             
                        pEdcaParm->Cwmax[aci] = *(ptr+1) >> 4;               
                        pEdcaParm->Txop[aci]  = *(ptr+2) + 256 * (*(ptr+3)); 
                        ptr += 4; 
                    }
                }
                else if (NdisEqualMemory(pEid->Octet, WME_INFO_ELEM, 6) && (pEid->Len == 7))
                {
                    
                    pEdcaParm->bValid          = TRUE;
                    pEdcaParm->bQAck           = FALSE; 
                    pEdcaParm->bQueueRequest   = FALSE; 
                    pEdcaParm->bTxopRequest    = FALSE; 
                    pEdcaParm->EdcaUpdateCount = pEid->Octet[6] & 0x0f;
                    pEdcaParm->bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;

                    
                    pEdcaParm->bACM[QID_AC_BE]  = 0;
                    pEdcaParm->Aifsn[QID_AC_BE] = 3;
                    pEdcaParm->Cwmin[QID_AC_BE] = CW_MIN_IN_BITS;
                    pEdcaParm->Cwmax[QID_AC_BE] = CW_MAX_IN_BITS;
                    pEdcaParm->Txop[QID_AC_BE]  = 0;

                    pEdcaParm->bACM[QID_AC_BK]  = 0;
                    pEdcaParm->Aifsn[QID_AC_BK] = 7;
                    pEdcaParm->Cwmin[QID_AC_BK] = CW_MIN_IN_BITS;
                    pEdcaParm->Cwmax[QID_AC_BK] = CW_MAX_IN_BITS;
                    pEdcaParm->Txop[QID_AC_BK]  = 0;

                    pEdcaParm->bACM[QID_AC_VI]  = 0;
                    pEdcaParm->Aifsn[QID_AC_VI] = 2;
                    pEdcaParm->Cwmin[QID_AC_VI] = CW_MIN_IN_BITS-1;
                    pEdcaParm->Cwmax[QID_AC_VI] = CW_MAX_IN_BITS;
                    pEdcaParm->Txop[QID_AC_VI]  = 96;   

                    pEdcaParm->bACM[QID_AC_VO]  = 0;
                    pEdcaParm->Aifsn[QID_AC_VO] = 2;
                    pEdcaParm->Cwmin[QID_AC_VO] = CW_MIN_IN_BITS-2;
                    pEdcaParm->Cwmax[QID_AC_VO] = CW_MAX_IN_BITS-1;
                    pEdcaParm->Txop[QID_AC_VO]  = 48;   
                }
#ifdef CONFIG_STA_SUPPORT
#endif 


                break;

            case IE_EXT_SUPP_RATES:
                if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    NdisMoveMemory(ExtRate, pEid->Octet, pEid->Len);
                    *pExtRateLen = pEid->Len;

                    
                    
                    
                    
                    
                }
                break;

            case IE_ERP:
                if (pEid->Len == 1)
                {
                    *pErp = (UCHAR)pEid->Octet[0];
                }
                break;

            case IE_AIRONET_CKIP:
                
                
                
                if (pEid->Len < (CKIP_NEGOTIATION_LENGTH - 2))
                    break;

                
                *pCkipFlag = *(pEid->Octet + 8);
                break;

            case IE_AP_TX_POWER:
                
                
                if (pEid->Len != 0x06)
                    break;

                
                if (NdisEqualMemory(pEid->Octet, CISCO_OUI, 3) == 1)
                    *pAironetCellPowerLimit = *(pEid->Octet + 4);
                break;

            
            case IE_RSN:
                
                if (RTMPEqualMemory(pEid->Octet + 2, RSN_OUI, 3))
                {
                    
                    Ptr = (PUCHAR) pVIE;
                    NdisMoveMemory(Ptr + *LengthVIE, &pEid->Eid, pEid->Len + 2);
                    *LengthVIE += (pEid->Len + 2);
                }
                break;
#ifdef CONFIG_STA_SUPPORT
#ifdef EXT_BUILD_CHANNEL_LIST
			case IE_COUNTRY:
				Ptr = (PUCHAR) pVIE;
                NdisMoveMemory(Ptr + *LengthVIE, &pEid->Eid, pEid->Len + 2);
                *LengthVIE += (pEid->Len + 2);
				break;
#endif 
#endif 


            default:
                break;
        }

        Length = Length + 2 + pEid->Len;  
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);
    }

    
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		UCHAR LatchRfChannel = MsgChannel;
		if ((pAd->LatchRfRegs.Channel > 14) && ((Sanity & 0x4) == 0))
		{
			if (CtrlChannel != 0)
				*pChannel = CtrlChannel;
			else
				*pChannel = LatchRfChannel;
			Sanity |= 0x4;
		}
	}
#endif 

	if (Sanity != 0x7)
	{
		DBGPRINT(RT_DEBUG_LOUD, ("PeerBeaconAndProbeRspSanity - missing field, Sanity=0x%02x\n", Sanity));
		return FALSE;
	}
	else
	{
		return TRUE;
	}

}

#ifdef DOT11N_DRAFT3

BOOLEAN PeerBeaconAndProbeRspSanity2(
	IN PRTMP_ADAPTER pAd,
	IN VOID *Msg,
	IN ULONG MsgLen,
	OUT UCHAR	*RegClass)
{
	CHAR				*Ptr;
	PFRAME_802_11		pFrame;
	PEID_STRUCT			pEid;
	ULONG				Length = 0;

	pFrame = (PFRAME_802_11)Msg;

	*RegClass = 0;
	Ptr = (PCHAR) pFrame->Octet;
	Length += LENGTH_802_11;

	
	Ptr += TIMESTAMP_LEN;
	Length += TIMESTAMP_LEN;

	
	Ptr += 2;
	Length += 2;

	
	Ptr += 2;
	Length += 2;

	pEid = (PEID_STRUCT) Ptr;

	
	while ((Length + 2 + pEid->Len) <= MsgLen)
	{
		switch(pEid->Eid)
		{
			case IE_SUPP_REG_CLASS:
				if(pEid->Len > 0)
				{
					*RegClass = *pEid->Octet;
				}
				else
				{
					DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAndProbeRspSanity - wrong IE_SSID (len=%d)\n",pEid->Len));
					return FALSE;
				}
				break;
		}

		Length = Length + 2 + pEid->Len;  
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);
	}

	return TRUE;

}
#endif 


BOOLEAN MlmeScanReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID *Msg,
	IN ULONG MsgLen,
	OUT UCHAR *pBssType,
	OUT CHAR Ssid[],
	OUT UCHAR *pSsidLen,
	OUT UCHAR *pScanType)
{
	MLME_SCAN_REQ_STRUCT *Info;

	Info = (MLME_SCAN_REQ_STRUCT *)(Msg);
	*pBssType = Info->BssType;
	*pSsidLen = Info->SsidLen;
	NdisMoveMemory(Ssid, Info->Ssid, *pSsidLen);
	*pScanType = Info->ScanType;

	if ((*pBssType == BSS_INFRA || *pBssType == BSS_ADHOC || *pBssType == BSS_ANY)
		&& (*pScanType == SCAN_ACTIVE || *pScanType == SCAN_PASSIVE
#ifdef CONFIG_STA_SUPPORT
#endif 
		))
	{
		return TRUE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("MlmeScanReqSanity fail - wrong BssType or ScanType\n"));
		return FALSE;
	}
}


UCHAR ChannelSanity(
    IN PRTMP_ADAPTER pAd,
    IN UCHAR channel)
{
    int i;

    for (i = 0; i < pAd->ChannelListNum; i ++)
    {
        if (channel == pAd->ChannelList[i].Channel)
            return 1;
    }
    return 0;
}


BOOLEAN PeerDeauthSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2,
    OUT USHORT *pReason)
{
    PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;

    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
    NdisMoveMemory(pReason, &pFrame->Octet[0], 2);

    return TRUE;
}


BOOLEAN PeerAuthSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pAddr,
    OUT USHORT *pAlg,
    OUT USHORT *pSeq,
    OUT USHORT *pStatus,
    CHAR *pChlgText)
{
    PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;

    COPY_MAC_ADDR(pAddr,   pFrame->Hdr.Addr2);
    NdisMoveMemory(pAlg,    &pFrame->Octet[0], 2);
    NdisMoveMemory(pSeq,    &pFrame->Octet[2], 2);
    NdisMoveMemory(pStatus, &pFrame->Octet[4], 2);

    if (*pAlg == AUTH_MODE_OPEN)
    {
        if (*pSeq == 1 || *pSeq == 2)
        {
            return TRUE;
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("PeerAuthSanity fail - wrong Seg#\n"));
            return FALSE;
        }
    }
    else if (*pAlg == AUTH_MODE_KEY)
    {
        if (*pSeq == 1 || *pSeq == 4)
        {
            return TRUE;
        }
        else if (*pSeq == 2 || *pSeq == 3)
        {
            NdisMoveMemory(pChlgText, &pFrame->Octet[8], CIPHER_TEXT_LEN);
            return TRUE;
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE, ("PeerAuthSanity fail - wrong Seg#\n"));
            return FALSE;
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PeerAuthSanity fail - wrong algorithm\n"));
        return FALSE;
    }
}


BOOLEAN MlmeAuthReqSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pAddr,
    OUT ULONG *pTimeout,
    OUT USHORT *pAlg)
{
    MLME_AUTH_REQ_STRUCT *pInfo;

    pInfo  = (MLME_AUTH_REQ_STRUCT *)Msg;
    COPY_MAC_ADDR(pAddr, pInfo->Addr);
    *pTimeout = pInfo->Timeout;
    *pAlg = pInfo->Alg;

    if (((*pAlg == AUTH_MODE_KEY) ||(*pAlg == AUTH_MODE_OPEN)
	) &&
        ((*pAddr & 0x01) == 0))
    {
        return TRUE;
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeAuthReqSanity fail - wrong algorithm\n"));
        return FALSE;
    }
}


BOOLEAN MlmeAssocReqSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pApAddr,
    OUT USHORT *pCapabilityInfo,
    OUT ULONG *pTimeout,
    OUT USHORT *pListenIntv)
{
    MLME_ASSOC_REQ_STRUCT *pInfo;

    pInfo = (MLME_ASSOC_REQ_STRUCT *)Msg;
    *pTimeout = pInfo->Timeout;                             
    COPY_MAC_ADDR(pApAddr, pInfo->Addr);                   
    *pCapabilityInfo = pInfo->CapabilityInfo;               
    *pListenIntv = pInfo->ListenIntv;

    return TRUE;
}


BOOLEAN PeerDisassocSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2,
    OUT USHORT *pReason)
{
    PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;

    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
    NdisMoveMemory(pReason, &pFrame->Octet[0], 2);

    return TRUE;
}


NDIS_802_11_NETWORK_TYPE NetworkTypeInUseSanity(
    IN PBSS_ENTRY pBss)
{
	NDIS_802_11_NETWORK_TYPE	NetWorkType;
	UCHAR						rate, i;

	NetWorkType = Ndis802_11DS;

	if (pBss->Channel <= 14)
	{
		
		
		
		for (i = 0; i < pBss->SupRateLen; i++)
		{
			rate = pBss->SupRate[i] & 0x7f; 
			if ((rate == 2) || (rate == 4) || (rate == 11) || (rate == 22))
			{
				continue;
			}
			else
			{
				
				
				
				NetWorkType = Ndis802_11OFDM24;
				break;
			}
		}

		
		
		
		if (NetWorkType != Ndis802_11OFDM24)
		{
			for (i = 0; i < pBss->ExtRateLen; i++)
			{
				rate = pBss->SupRate[i] & 0x7f; 
				if ((rate == 2) || (rate == 4) || (rate == 11) || (rate == 22))
				{
					continue;
				}
				else
				{
					
					
					
					NetWorkType = Ndis802_11OFDM24;
					break;
				}
			}
		}
	}
	else
	{
		NetWorkType = Ndis802_11OFDM5;
	}

    if (pBss->HtCapabilityLen != 0)
    {
        if (NetWorkType == Ndis802_11OFDM5)
            NetWorkType = Ndis802_11OFDM5_N;
        else
            NetWorkType = Ndis802_11OFDM24_N;
    }

	return NetWorkType;
}


BOOLEAN PeerWpaMessageSanity(
    IN	PRTMP_ADAPTER		pAd,
    IN	PEAPOL_PACKET		pMsg,
    IN	ULONG				MsgLen,
    IN	UCHAR				MsgType,
    IN	MAC_TABLE_ENTRY		*pEntry)
{
	UCHAR			mic[LEN_KEY_DESC_MIC], digest[80], KEYDATA[MAX_LEN_OF_RSNIE];
	BOOLEAN			bReplayDiff = FALSE;
	BOOLEAN			bWPA2 = FALSE;
	KEY_INFO		EapolKeyInfo;
	UCHAR			GroupKeyIndex = 0;


	NdisZeroMemory(mic, sizeof(mic));
	NdisZeroMemory(digest, sizeof(digest));
	NdisZeroMemory(KEYDATA, sizeof(KEYDATA));
	NdisZeroMemory((PUCHAR)&EapolKeyInfo, sizeof(EapolKeyInfo));

	NdisMoveMemory((PUCHAR)&EapolKeyInfo, (PUCHAR)&pMsg->KeyDesc.KeyInfo, sizeof(KEY_INFO));

	*((USHORT *)&EapolKeyInfo) = cpu2le16(*((USHORT *)&EapolKeyInfo));

	
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
		bWPA2 = TRUE;

	
	if ((MsgType > EAPOL_GROUP_MSG_2) || (MsgType < EAPOL_PAIR_MSG_1))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("The message type is invalid(%d)! \n", MsgType));
		return FALSE;
	}

	
	if (MsgType == EAPOL_PAIR_MSG_1 || MsgType == EAPOL_PAIR_MSG_3 || MsgType == EAPOL_GROUP_MSG_1)	
    {
	
		
		UCHAR	ZeroReplay[LEN_KEY_DESC_REPLAY];

        NdisZeroMemory(ZeroReplay, LEN_KEY_DESC_REPLAY);
		if ((RTMPCompareMemory(pMsg->KeyDesc.ReplayCounter, pEntry->R_Counter, LEN_KEY_DESC_REPLAY) != 1) &&
			(RTMPCompareMemory(pMsg->KeyDesc.ReplayCounter, ZeroReplay, LEN_KEY_DESC_REPLAY) != 0))
	{
			bReplayDiff = TRUE;
	}
	}
	else if (MsgType == EAPOL_PAIR_MSG_2 || MsgType == EAPOL_PAIR_MSG_4 || MsgType == EAPOL_GROUP_MSG_2)	
	{
		
	if (!NdisEqualMemory(pMsg->KeyDesc.ReplayCounter, pEntry->R_Counter, LEN_KEY_DESC_REPLAY))
	{
			bReplayDiff = TRUE;
	}
	}

	
	if (bReplayDiff)
	{
		
		if (pAd->CommonCfg.bWirelessEvent)
			RTMPSendWirelessEvent(pAd, IW_REPLAY_COUNTER_DIFF_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0);

		if (MsgType < EAPOL_GROUP_MSG_1)
		{
		DBGPRINT(RT_DEBUG_ERROR, ("Replay Counter Different in pairwise msg %d of 4-way handshake!\n", MsgType));
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Replay Counter Different in group msg %d of 2-way handshake!\n", (MsgType - EAPOL_PAIR_MSG_4)));
		}

		hex_dump("Receive replay counter ", pMsg->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);
		hex_dump("Current replay counter ", pEntry->R_Counter, LEN_KEY_DESC_REPLAY);
        return FALSE;
	}

	
	if (MsgType != EAPOL_PAIR_MSG_1)
	{
		UCHAR			rcvd_mic[LEN_KEY_DESC_MIC];

		
		NdisMoveMemory(rcvd_mic, pMsg->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
		NdisZeroMemory(pMsg->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);

        if (EapolKeyInfo.KeyDescVer == DESC_TYPE_TKIP)	
        {
            HMAC_MD5(pEntry->PTK, LEN_EAP_MICK, (PUCHAR)pMsg, MsgLen, mic, MD5_DIGEST_SIZE);
        }
        else if (EapolKeyInfo.KeyDescVer == DESC_TYPE_AES)	
        {
            HMAC_SHA1(pEntry->PTK, LEN_EAP_MICK, (PUCHAR)pMsg, MsgLen, digest, SHA1_DIGEST_SIZE);
            NdisMoveMemory(mic, digest, LEN_KEY_DESC_MIC);
        }

        if (!NdisEqualMemory(rcvd_mic, mic, LEN_KEY_DESC_MIC))
        {
			
			if (pAd->CommonCfg.bWirelessEvent)
				RTMPSendWirelessEvent(pAd, IW_MIC_DIFF_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0);

			if (MsgType < EAPOL_GROUP_MSG_1)
			{
		DBGPRINT(RT_DEBUG_ERROR, ("MIC Different in pairwise msg %d of 4-way handshake!\n", MsgType));
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("MIC Different in group msg %d of 2-way handshake!\n", (MsgType - EAPOL_PAIR_MSG_4)));
			}

			hex_dump("Received MIC", rcvd_mic, LEN_KEY_DESC_MIC);
			hex_dump("Desired  MIC", mic, LEN_KEY_DESC_MIC);

			return FALSE;
        }
	}

	
	
	
	
	if (CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen) > 0)
	{
		
		if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1))
		{
			if(
				(EapolKeyInfo.KeyDescVer == DESC_TYPE_AES))
			{
				
				AES_GTK_KEY_UNWRAP(&pEntry->PTK[16], KEYDATA,
									CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen),
									pMsg->KeyDesc.KeyData);
			}
			else
			{
				INT	i;
				UCHAR   Key[32];
				
				
				NdisMoveMemory(Key, pMsg->KeyDesc.KeyIv, 16);
				NdisMoveMemory(&Key[16], &pEntry->PTK[16], 16);
				ARCFOUR_INIT(&pAd->PrivateInfo.WEPCONTEXT, Key, 32);
				
				for(i = 0; i < 256; i++)
					ARCFOUR_BYTE(&pAd->PrivateInfo.WEPCONTEXT);
				
				ARCFOUR_DECRYPT(&pAd->PrivateInfo.WEPCONTEXT, KEYDATA,
								pMsg->KeyDesc.KeyData,
								CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen));
			}

			if (!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1))
				GroupKeyIndex = EapolKeyInfo.KeyIndex;

		}
		else if ((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3 && !bWPA2))
		{
			NdisMoveMemory(KEYDATA, pMsg->KeyDesc.KeyData, CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen));
		}
		else
		{

			return TRUE;
		}

		
		
		
		
		if (!RTMPParseEapolKeyData(pAd, KEYDATA,
								  CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen),
								  GroupKeyIndex, MsgType, bWPA2, pEntry))
		{
			return FALSE;
		}
	}

	return TRUE;

}

#ifdef CONFIG_STA_SUPPORT
#ifdef QOS_DLS_SUPPORT
BOOLEAN MlmeDlsReqSanity(
	IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PRT_802_11_DLS *pDLS,
    OUT PUSHORT pReason)
{
	MLME_DLS_REQ_STRUCT *pInfo;

    pInfo = (MLME_DLS_REQ_STRUCT *)Msg;

	*pDLS = pInfo->pDLS;
	*pReason = pInfo->Reason;

	return TRUE;
}
#endif 
#endif 

#ifdef QOS_DLS_SUPPORT
BOOLEAN PeerDlsReqSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pDA,
    OUT PUCHAR pSA,
    OUT USHORT *pCapabilityInfo,
    OUT USHORT *pDlsTimeout,
    OUT UCHAR *pRatesLen,
    OUT UCHAR Rates[],
	OUT UCHAR *pHtCapabilityLen,
    OUT HT_CAPABILITY_IE *pHtCapability)
{
	CHAR            *Ptr;
    PFRAME_802_11	Fr = (PFRAME_802_11)Msg;
	PEID_STRUCT  eid_ptr;

    
    *pCapabilityInfo	= 0;
    *pDlsTimeout	= 0;
	*pHtCapabilityLen = 0;

    Ptr = (PCHAR)Fr->Octet;

	
    Ptr += 2;

    
    NdisMoveMemory(pDA, Ptr, MAC_ADDR_LEN);
    Ptr += MAC_ADDR_LEN;

    
    NdisMoveMemory(pSA, Ptr, MAC_ADDR_LEN);
    Ptr += MAC_ADDR_LEN;

    
    NdisMoveMemory(pCapabilityInfo, Ptr, 2);
    Ptr += 2;

    
    NdisMoveMemory(pDlsTimeout, Ptr, 2);
    Ptr += 2;

	
	eid_ptr = (PEID_STRUCT) &Fr->Octet[18];

	while (((UCHAR*)eid_ptr + eid_ptr->Len + 1) < ((UCHAR*)Fr + MsgLen))
	{
		switch(eid_ptr->Eid)
		{
			case IE_SUPP_RATES:
                if ((eid_ptr->Len <= MAX_LEN_OF_SUPPORTED_RATES) && (eid_ptr->Len > 0))
                {
                    NdisMoveMemory(Rates, eid_ptr->Octet, eid_ptr->Len);
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerDlsReqSanity - IE_SUPP_RATES., Len=%d. Rates[0]=%x\n",eid_ptr->Len, Rates[0]));
                    DBGPRINT(RT_DEBUG_TRACE, ("Rates[1]=%x %x %x %x %x %x %x\n", Rates[1], Rates[2], Rates[3], Rates[4], Rates[5], Rates[6], Rates[7]));
                    *pRatesLen = eid_ptr->Len;
                }
                else
                {
                    *pRatesLen = 8;
					Rates[0] = 0x82;
					Rates[1] = 0x84;
					Rates[2] = 0x8b;
					Rates[3] = 0x96;
					Rates[4] = 0x12;
					Rates[5] = 0x24;
					Rates[6] = 0x48;
					Rates[7] = 0x6c;
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerDlsReqSanity - wrong IE_SUPP_RATES., Len=%d\n",eid_ptr->Len));
                }
				break;

			case IE_EXT_SUPP_RATES:
                if (eid_ptr->Len + *pRatesLen <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    NdisMoveMemory(&Rates[*pRatesLen], eid_ptr->Octet, eid_ptr->Len);
                    *pRatesLen = (*pRatesLen) + eid_ptr->Len;
                }
                else
                {
                    NdisMoveMemory(&Rates[*pRatesLen], eid_ptr->Octet, MAX_LEN_OF_SUPPORTED_RATES - (*pRatesLen));
                    *pRatesLen = MAX_LEN_OF_SUPPORTED_RATES;
                }
				break;

			case IE_HT_CAP:
				if (eid_ptr->Len >= sizeof(HT_CAPABILITY_IE))
				{
					NdisMoveMemory(pHtCapability, eid_ptr->Octet, sizeof(HT_CAPABILITY_IE));

					*(USHORT *)(&pHtCapability->HtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
					{
						EXT_HT_CAP_INFO extHtCapInfo;

						NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&pHtCapability->ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
						*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
						NdisMoveMemory((PUCHAR)(&pHtCapability->ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
					}
#else
					*(USHORT *)(&pHtCapability->ExtHtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->ExtHtCapInfo));
#endif 
					*pHtCapabilityLen = sizeof(HT_CAPABILITY_IE);

					DBGPRINT(RT_DEBUG_TRACE, ("PeerDlsReqSanity - IE_HT_CAP\n"));
				}
				else
				{
					DBGPRINT(RT_DEBUG_TRACE, ("PeerDlsReqSanity - wrong IE_HT_CAP.eid_ptr->Len = %d\n", eid_ptr->Len));
				}
				break;

			default:
				break;
		}

		eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);
	}

    return TRUE;
}

BOOLEAN PeerDlsRspSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pDA,
    OUT PUCHAR pSA,
    OUT USHORT *pCapabilityInfo,
    OUT USHORT *pStatus,
    OUT UCHAR *pRatesLen,
    OUT UCHAR Rates[],
    OUT UCHAR *pHtCapabilityLen,
    OUT HT_CAPABILITY_IE *pHtCapability)
{
    CHAR            *Ptr;
    PFRAME_802_11	Fr = (PFRAME_802_11)Msg;
	PEID_STRUCT  eid_ptr;

    
    *pStatus		= 0;
    *pCapabilityInfo	= 0;
	*pHtCapabilityLen = 0;

    Ptr = (PCHAR)Fr->Octet;

	
    Ptr += 2;

	
    NdisMoveMemory(pStatus, Ptr, 2);
    Ptr += 2;

    
    NdisMoveMemory(pDA, Ptr, MAC_ADDR_LEN);
    Ptr += MAC_ADDR_LEN;

    
    NdisMoveMemory(pSA, Ptr, MAC_ADDR_LEN);
    Ptr += MAC_ADDR_LEN;

	if (pStatus == 0)
	{
	    
	    NdisMoveMemory(pCapabilityInfo, Ptr, 2);
	    Ptr += 2;
	}

	
	eid_ptr = (PEID_STRUCT) &Fr->Octet[18];

	while (((UCHAR*)eid_ptr + eid_ptr->Len + 1) < ((UCHAR*)Fr + MsgLen))
	{
		switch(eid_ptr->Eid)
		{
			case IE_SUPP_RATES:
                if ((eid_ptr->Len <= MAX_LEN_OF_SUPPORTED_RATES) && (eid_ptr->Len > 0))
                {
                    NdisMoveMemory(Rates, eid_ptr->Octet, eid_ptr->Len);
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerDlsRspSanity - IE_SUPP_RATES., Len=%d. Rates[0]=%x\n",eid_ptr->Len, Rates[0]));
                    DBGPRINT(RT_DEBUG_TRACE, ("Rates[1]=%x %x %x %x %x %x %x\n", Rates[1], Rates[2], Rates[3], Rates[4], Rates[5], Rates[6], Rates[7]));
                    *pRatesLen = eid_ptr->Len;
                }
                else
                {
                    *pRatesLen = 8;
					Rates[0] = 0x82;
					Rates[1] = 0x84;
					Rates[2] = 0x8b;
					Rates[3] = 0x96;
					Rates[4] = 0x12;
					Rates[5] = 0x24;
					Rates[6] = 0x48;
					Rates[7] = 0x6c;
                    DBGPRINT(RT_DEBUG_TRACE, ("PeerDlsRspSanity - wrong IE_SUPP_RATES., Len=%d\n",eid_ptr->Len));
                }
				break;

			case IE_EXT_SUPP_RATES:
                if (eid_ptr->Len + *pRatesLen <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    NdisMoveMemory(&Rates[*pRatesLen], eid_ptr->Octet, eid_ptr->Len);
                    *pRatesLen = (*pRatesLen) + eid_ptr->Len;
                }
                else
                {
                    NdisMoveMemory(&Rates[*pRatesLen], eid_ptr->Octet, MAX_LEN_OF_SUPPORTED_RATES - (*pRatesLen));
                    *pRatesLen = MAX_LEN_OF_SUPPORTED_RATES;
                }
				break;

			case IE_HT_CAP:
				if (eid_ptr->Len >= sizeof(HT_CAPABILITY_IE))
				{
					NdisMoveMemory(pHtCapability, eid_ptr->Octet, sizeof(HT_CAPABILITY_IE));

					*(USHORT *)(&pHtCapability->HtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
					{
						EXT_HT_CAP_INFO extHtCapInfo;

						NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&pHtCapability->ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
						*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
						NdisMoveMemory((PUCHAR)(&pHtCapability->ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
					}
#else
					*(USHORT *)(&pHtCapability->ExtHtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->ExtHtCapInfo));
#endif 
					*pHtCapabilityLen = sizeof(HT_CAPABILITY_IE);

					DBGPRINT(RT_DEBUG_TRACE, ("PeerDlsRspSanity - IE_HT_CAP\n"));
				}
				else
				{
					DBGPRINT(RT_DEBUG_TRACE, ("PeerDlsRspSanity - wrong IE_HT_CAP.eid_ptr->Len = %d\n", eid_ptr->Len));
				}
				break;

			default:
				break;
		}

		eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);
	}

    return TRUE;
}

BOOLEAN PeerDlsTearDownSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pDA,
    OUT PUCHAR pSA,
    OUT USHORT *pReason)
{
    CHAR            *Ptr;
    PFRAME_802_11	Fr = (PFRAME_802_11)Msg;

    
    *pReason	= 0;

    Ptr = (PCHAR)Fr->Octet;

	
    Ptr += 2;

    
    NdisMoveMemory(pDA, Ptr, MAC_ADDR_LEN);
    Ptr += MAC_ADDR_LEN;

    
    NdisMoveMemory(pSA, Ptr, MAC_ADDR_LEN);
    Ptr += MAC_ADDR_LEN;

	
    NdisMoveMemory(pReason, Ptr, 2);
    Ptr += 2;

    return TRUE;
}
#endif 
