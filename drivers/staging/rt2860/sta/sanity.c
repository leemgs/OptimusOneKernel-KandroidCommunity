
#include "../rt_config.h"

extern UCHAR	CISCO_OUI[];

extern UCHAR	WPA_OUI[];
extern UCHAR	RSN_OUI[];
extern UCHAR	WME_INFO_ELEM[];
extern UCHAR	WME_PARM_ELEM[];
extern UCHAR	Ccx2QosInfo[];
extern UCHAR	RALINK_OUI[];
extern UCHAR	BROADCOM_OUI[];


BOOLEAN MlmeStartReqSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT CHAR Ssid[],
    OUT UCHAR *pSsidLen)
{
    MLME_START_REQ_STRUCT *Info;

    Info = (MLME_START_REQ_STRUCT *)(Msg);

    if (Info->SsidLen > MAX_LEN_OF_SSID)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("MlmeStartReqSanity fail - wrong SSID length\n"));
        return FALSE;
    }

    *pSsidLen = Info->SsidLen;
    NdisMoveMemory(Ssid, Info->Ssid, *pSsidLen);

    return TRUE;
}


BOOLEAN PeerAssocRspSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *pMsg,
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2,
    OUT USHORT *pCapabilityInfo,
    OUT USHORT *pStatus,
    OUT USHORT *pAid,
    OUT UCHAR SupRate[],
    OUT UCHAR *pSupRateLen,
    OUT UCHAR ExtRate[],
    OUT UCHAR *pExtRateLen,
    OUT HT_CAPABILITY_IE		*pHtCapability,
    OUT ADD_HT_INFO_IE		*pAddHtInfo,	
    OUT UCHAR			*pHtCapabilityLen,
    OUT UCHAR			*pAddHtInfoLen,
    OUT UCHAR			*pNewExtChannelOffset,
    OUT PEDCA_PARM pEdcaParm,
    OUT UCHAR *pCkipFlag)
{
    CHAR          IeType, *Ptr;
    PFRAME_802_11 pFrame = (PFRAME_802_11)pMsg;
    PEID_STRUCT   pEid;
    ULONG         Length = 0;

	*pNewExtChannelOffset = 0xff;
	*pHtCapabilityLen = 0;
	*pAddHtInfoLen = 0;
    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
    Ptr = pFrame->Octet;
    Length += LENGTH_802_11;

    NdisMoveMemory(pCapabilityInfo, &pFrame->Octet[0], 2);
    Length += 2;
    NdisMoveMemory(pStatus,         &pFrame->Octet[2], 2);
    Length += 2;
    *pCkipFlag = 0;
    *pExtRateLen = 0;
    pEdcaParm->bValid = FALSE;

    if (*pStatus != MLME_SUCCESS)
        return TRUE;

    NdisMoveMemory(pAid, &pFrame->Octet[4], 2);
    Length += 2;

    
    *pAid = (*pAid) & 0x3fff; 

    
    IeType = pFrame->Octet[6];
    *pSupRateLen = pFrame->Octet[7];
    if ((IeType != IE_SUPP_RATES) || (*pSupRateLen > MAX_LEN_OF_SUPPORTED_RATES))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PeerAssocRspSanity fail - wrong SupportedRates IE\n"));
        return FALSE;
    }
    else
        NdisMoveMemory(SupRate, &pFrame->Octet[8], *pSupRateLen);

    Length = Length + 2 + *pSupRateLen;

    
    
    pEid = (PEID_STRUCT) &pFrame->Octet[8 + (*pSupRateLen)];

    
    while ((Length + 2 + pEid->Len) <= MsgLen)
    {
        switch (pEid->Eid)
        {
            case IE_EXT_SUPP_RATES:
                if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
                {
                    NdisMoveMemory(ExtRate, pEid->Octet, pEid->Len);
                    *pExtRateLen = pEid->Len;
                }
                break;

             case IE_HT_CAP:
            case IE_HT_CAP2:
			if (pEid->Len >= SIZE_HT_CAP_IE)  
			{
				NdisMoveMemory(pHtCapability, pEid->Octet, SIZE_HT_CAP_IE);

				*(USHORT *)(&pHtCapability->HtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->HtCapInfo));
				*(USHORT *)(&pHtCapability->ExtHtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->ExtHtCapInfo));

				*pHtCapabilityLen = SIZE_HT_CAP_IE;
			}
			else
			{
				DBGPRINT(RT_DEBUG_WARN, ("PeerAssocRspSanity - wrong IE_HT_CAP. \n"));
			}

		break;
            case IE_ADD_HT:
            case IE_ADD_HT2:
			if (pEid->Len >= sizeof(ADD_HT_INFO_IE))
			{
				
				
				NdisMoveMemory(pAddHtInfo, pEid->Octet, sizeof(ADD_HT_INFO_IE));

				*(USHORT *)(&pAddHtInfo->AddHtInfo2) = cpu2le16(*(USHORT *)(&pAddHtInfo->AddHtInfo2));
				*(USHORT *)(&pAddHtInfo->AddHtInfo3) = cpu2le16(*(USHORT *)(&pAddHtInfo->AddHtInfo3));

				*pAddHtInfoLen = SIZE_ADD_HT_INFO_IE;
			}
			else
			{
				DBGPRINT(RT_DEBUG_WARN, ("PeerAssocRspSanity - wrong IE_ADD_HT. \n"));
			}

		break;
            case IE_SECONDARY_CH_OFFSET:
			if (pEid->Len == 1)
			{
				*pNewExtChannelOffset = pEid->Octet[0];
			}
			else
			{
				DBGPRINT(RT_DEBUG_WARN, ("PeerAssocRspSanity - wrong IE_SECONDARY_CH_OFFSET. \n"));
			}
		break;
            case IE_AIRONET_CKIP:
                
                
                
                if (pEid->Len < (CKIP_NEGOTIATION_LENGTH - 2))
                break;

                
                *pCkipFlag = *(pEid->Octet + 8);
                break;

            case IE_AIRONET_IPADDRESS:
                if (pEid->Len != 0x0A)
                break;

                
                if (NdisEqualMemory(pEid->Octet, CISCO_OUI, 3) == 1)
                    NdisMoveMemory(pAd->StaCfg.AironetIPAddress, pEid->Octet + 4, 4);
                break;

            
            
            case IE_VENDOR_SPECIFIC:
                
                if (NdisEqualMemory(pEid->Octet, WME_PARM_ELEM, 6) && (pEid->Len == 24))
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

                
                else
                {
                    
                    if (pAd->StaCfg.CCXControl.field.Enable == 0)
                        break;
                    if (pEid->Len != 5)
                        break;

                    
                    if (NdisEqualMemory(pEid->Octet, Ccx2IeInfo, 5) == 1)
                        pAd->StaCfg.CCXEnable = TRUE;
                    break;
                }
                break;

            default:
                DBGPRINT(RT_DEBUG_TRACE, ("PeerAssocRspSanity - ignore unrecognized EID = %d\n", pEid->Eid));
                break;
        }

        Length = Length + 2 + pEid->Len;
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);
    }

    
    if (pAd->StaCfg.CCXControl.field.Enable == 1)
        pAd->StaCfg.CCXEnable = TRUE;

    return TRUE;
}


BOOLEAN PeerProbeReqSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2,
    OUT CHAR Ssid[],
    OUT UCHAR *pSsidLen)
{
    UCHAR         Idx;
    UCHAR	      RateLen;
    CHAR          IeType;
    PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;

    COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);

    if ((pFrame->Octet[0] != IE_SSID) || (pFrame->Octet[1] > MAX_LEN_OF_SSID))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PeerProbeReqSanity fail - wrong SSID IE(Type=%d,Len=%d)\n",pFrame->Octet[0],pFrame->Octet[1]));
        return FALSE;
    }

    *pSsidLen = pFrame->Octet[1];
    NdisMoveMemory(Ssid, &pFrame->Octet[2], *pSsidLen);

    Idx = *pSsidLen + 2;

    
    IeType = pFrame->Octet[Idx];
    RateLen = pFrame->Octet[Idx + 1];
    if (IeType != IE_SUPP_RATES)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("PeerProbeReqSanity fail - wrong SupportRates IE(Type=%d,Len=%d)\n",pFrame->Octet[Idx],pFrame->Octet[Idx+1]));
        return FALSE;
    }
    else
    {
        if ((pAd->CommonCfg.PhyMode == PHY_11G) && (RateLen < 8))
            return (FALSE);
    }

    return TRUE;
}


BOOLEAN GetTimBit(
    IN CHAR *Ptr,
    IN USHORT Aid,
    OUT UCHAR *TimLen,
    OUT UCHAR *BcastFlag,
    OUT UCHAR *DtimCount,
    OUT UCHAR *DtimPeriod,
    OUT UCHAR *MessageToMe)
{
    UCHAR          BitCntl, N1, N2, MyByte, MyBit;
    CHAR          *IdxPtr;

    IdxPtr = Ptr;

    IdxPtr ++;
    *TimLen = *IdxPtr;

    
    IdxPtr ++;
    *DtimCount = *IdxPtr;

    
    IdxPtr++;
    *DtimPeriod = *IdxPtr;

    
    IdxPtr++;
    BitCntl = *IdxPtr;

    if ((*DtimCount == 0) && (BitCntl & 0x01))
        *BcastFlag = TRUE;
    else
        *BcastFlag = FALSE;

    
    N1 = BitCntl & 0xfe;    
    N2 = *TimLen - 4 + N1;  

    if ((Aid < (N1 << 3)) || (Aid >= ((N2 + 1) << 3)))
        *MessageToMe = FALSE;
    else
    {
        MyByte = (Aid >> 3) - N1;                       
        MyBit = Aid % 16 - ((MyByte & 0x01)? 8:0);

        IdxPtr += (MyByte + 1);

        
        

        if (*IdxPtr & (0x01 << MyBit))
            *MessageToMe = TRUE;
        else
            *MessageToMe = FALSE;
    }

    return TRUE;
}

