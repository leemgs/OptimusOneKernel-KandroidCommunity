

#include "../rt_config.h"



#define	BG_BAND_REGION_0_START	0			
#define	BG_BAND_REGION_0_SIZE	11
#define	BG_BAND_REGION_1_START	0			
#define	BG_BAND_REGION_1_SIZE	13
#define	BG_BAND_REGION_2_START	9			
#define	BG_BAND_REGION_2_SIZE	2
#define	BG_BAND_REGION_3_START	9			
#define	BG_BAND_REGION_3_SIZE	4
#define	BG_BAND_REGION_4_START	13			
#define	BG_BAND_REGION_4_SIZE	1
#define	BG_BAND_REGION_5_START	0			
#define	BG_BAND_REGION_5_SIZE	14
#define	BG_BAND_REGION_6_START	2			
#define	BG_BAND_REGION_6_SIZE	7
#define	BG_BAND_REGION_7_START	4			
#define	BG_BAND_REGION_7_SIZE	9
#define	BG_BAND_REGION_31_START	0			
#define	BG_BAND_REGION_31_SIZE	14


UCHAR A_BAND_REGION_0_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165};
UCHAR A_BAND_REGION_1_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};
UCHAR A_BAND_REGION_2_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64};
UCHAR A_BAND_REGION_3_CHANNEL_LIST[]={52, 56, 60, 64, 149, 153, 157, 161};
UCHAR A_BAND_REGION_4_CHANNEL_LIST[]={149, 153, 157, 161, 165};
UCHAR A_BAND_REGION_5_CHANNEL_LIST[]={149, 153, 157, 161};
UCHAR A_BAND_REGION_6_CHANNEL_LIST[]={36, 40, 44, 48};
UCHAR A_BAND_REGION_7_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165, 169, 173};
UCHAR A_BAND_REGION_8_CHANNEL_LIST[]={52, 56, 60, 64};
UCHAR A_BAND_REGION_9_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165};
UCHAR A_BAND_REGION_10_CHANNEL_LIST[]={36, 40, 44, 48, 149, 153, 157, 161, 165};
UCHAR A_BAND_REGION_11_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 149, 153, 157, 161};
UCHAR A_BAND_REGION_12_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};
UCHAR A_BAND_REGION_13_CHANNEL_LIST[]={52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161};
UCHAR A_BAND_REGION_14_CHANNEL_LIST[]={36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 136, 140, 149, 153, 157, 161, 165};
UCHAR A_BAND_REGION_15_CHANNEL_LIST[]={149, 153, 157, 161, 165, 169, 173};



UCHAR BaSizeArray[4] = {8,16,32,64};


VOID BuildChannelList(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR i, j, index=0, num=0;
	PUCHAR	pChannelList = NULL;

	NdisZeroMemory(pAd->ChannelList, MAX_NUM_OF_CHANNELS * sizeof(CHANNEL_TX_POWER));

	
	if ((pAd->CommonCfg.PhyMode != PHY_11A)
#ifdef DOT11_N_SUPPORT
		&& (pAd->CommonCfg.PhyMode != PHY_11AN_MIXED) && (pAd->CommonCfg.PhyMode != PHY_11N_5G)
#endif 
	)
	{
		switch (pAd->CommonCfg.CountryRegion  & 0x7f)
		{
			case REGION_0_BG_BAND:	
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_0_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_0_SIZE);
				index += BG_BAND_REGION_0_SIZE;
				break;
			case REGION_1_BG_BAND:	
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_1_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_1_SIZE);
				index += BG_BAND_REGION_1_SIZE;
				break;
			case REGION_2_BG_BAND:	
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_2_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_2_SIZE);
				index += BG_BAND_REGION_2_SIZE;
				break;
			case REGION_3_BG_BAND:	
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_3_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_3_SIZE);
				index += BG_BAND_REGION_3_SIZE;
				break;
			case REGION_4_BG_BAND:	
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_4_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_4_SIZE);
				index += BG_BAND_REGION_4_SIZE;
				break;
			case REGION_5_BG_BAND:	
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_5_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_5_SIZE);
				index += BG_BAND_REGION_5_SIZE;
				break;
			case REGION_6_BG_BAND:	
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_6_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_6_SIZE);
				index += BG_BAND_REGION_6_SIZE;
				break;
			case REGION_7_BG_BAND:  
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_7_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_7_SIZE);
				index += BG_BAND_REGION_7_SIZE;
				break;
			case REGION_31_BG_BAND:	
				NdisMoveMemory(&pAd->ChannelList[index], &pAd->TxPower[BG_BAND_REGION_31_START], sizeof(CHANNEL_TX_POWER) * BG_BAND_REGION_31_SIZE);
				index += BG_BAND_REGION_31_SIZE;
				break;
			default:            
				break;
		}
		for (i=0; i<index; i++)
			pAd->ChannelList[i].MaxTxPwr = 20;
	}

	if ((pAd->CommonCfg.PhyMode == PHY_11A) || (pAd->CommonCfg.PhyMode == PHY_11ABG_MIXED)
#ifdef DOT11_N_SUPPORT
		|| (pAd->CommonCfg.PhyMode == PHY_11ABGN_MIXED) || (pAd->CommonCfg.PhyMode == PHY_11AN_MIXED)
		|| (pAd->CommonCfg.PhyMode == PHY_11AGN_MIXED) || (pAd->CommonCfg.PhyMode == PHY_11N_5G)
#endif 
	)
	{
		switch (pAd->CommonCfg.CountryRegionForABand & 0x7f)
		{
			case REGION_0_A_BAND:
				num = sizeof(A_BAND_REGION_0_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_0_CHANNEL_LIST;
				break;
			case REGION_1_A_BAND:
				num = sizeof(A_BAND_REGION_1_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_1_CHANNEL_LIST;
				break;
			case REGION_2_A_BAND:
				num = sizeof(A_BAND_REGION_2_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_2_CHANNEL_LIST;
				break;
			case REGION_3_A_BAND:
				num = sizeof(A_BAND_REGION_3_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_3_CHANNEL_LIST;
				break;
			case REGION_4_A_BAND:
				num = sizeof(A_BAND_REGION_4_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_4_CHANNEL_LIST;
				break;
			case REGION_5_A_BAND:
				num = sizeof(A_BAND_REGION_5_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_5_CHANNEL_LIST;
				break;
			case REGION_6_A_BAND:
				num = sizeof(A_BAND_REGION_6_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_6_CHANNEL_LIST;
				break;
			case REGION_7_A_BAND:
				num = sizeof(A_BAND_REGION_7_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_7_CHANNEL_LIST;
				break;
			case REGION_8_A_BAND:
				num = sizeof(A_BAND_REGION_8_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_8_CHANNEL_LIST;
				break;
			case REGION_9_A_BAND:
				num = sizeof(A_BAND_REGION_9_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_9_CHANNEL_LIST;
				break;
			case REGION_10_A_BAND:
				num = sizeof(A_BAND_REGION_10_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_10_CHANNEL_LIST;
				break;
			case REGION_11_A_BAND:
				num = sizeof(A_BAND_REGION_11_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_11_CHANNEL_LIST;
				break;
			case REGION_12_A_BAND:
				num = sizeof(A_BAND_REGION_12_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_12_CHANNEL_LIST;
				break;
			case REGION_13_A_BAND:
				num = sizeof(A_BAND_REGION_13_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_13_CHANNEL_LIST;
				break;
			case REGION_14_A_BAND:
				num = sizeof(A_BAND_REGION_14_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_14_CHANNEL_LIST;
				break;
			case REGION_15_A_BAND:
				num = sizeof(A_BAND_REGION_15_CHANNEL_LIST)/sizeof(UCHAR);
				pChannelList = A_BAND_REGION_15_CHANNEL_LIST;
				break;
			default:            
				DBGPRINT(RT_DEBUG_WARN,("countryregion=%d not support", pAd->CommonCfg.CountryRegionForABand));
				break;
		}

		if (num != 0)
		{
			UCHAR RadarCh[15]={52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};
			for (i=0; i<num; i++)
			{
				for (j=0; j<MAX_NUM_OF_CHANNELS; j++)
				{
					if (pChannelList[i] == pAd->TxPower[j].Channel)
						NdisMoveMemory(&pAd->ChannelList[index+i], &pAd->TxPower[j], sizeof(CHANNEL_TX_POWER));
					}
				for (j=0; j<15; j++)
				{
					if (pChannelList[i] == RadarCh[j])
						pAd->ChannelList[index+i].DfsReq = TRUE;
				}
				pAd->ChannelList[index+i].MaxTxPwr = 20;
			}
			index += num;
		}
	}

	pAd->ChannelListNum = index;
	DBGPRINT(RT_DEBUG_TRACE,("country code=%d/%d, RFIC=%d, PHY mode=%d, support %d channels\n",
		pAd->CommonCfg.CountryRegion, pAd->CommonCfg.CountryRegionForABand, pAd->RfIcType, pAd->CommonCfg.PhyMode, pAd->ChannelListNum));
#ifdef DBG
	for (i=0;i<pAd->ChannelListNum;i++)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE,("BuildChannel # %d :: Pwr0 = %d, Pwr1 =%d, \n ", pAd->ChannelList[i].Channel, pAd->ChannelList[i].Power, pAd->ChannelList[i].Power2));
	}
#endif
}


UCHAR FirstChannel(
	IN PRTMP_ADAPTER pAd)
{
	return pAd->ChannelList[0].Channel;
}


UCHAR NextChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel)
{
	int i;
	UCHAR next_channel = 0;

	for (i = 0; i < (pAd->ChannelListNum - 1); i++)
		if (channel == pAd->ChannelList[i].Channel)
		{
			next_channel = pAd->ChannelList[i+1].Channel;
			break;
	}
	return next_channel;
}


VOID ChangeToCellPowerLimit(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         AironetCellPowerLimit)
{
	
	
	if (AironetCellPowerLimit == 0xFF)
		return;

	if (AironetCellPowerLimit < 6) 
		pAd->CommonCfg.TxPowerPercentage = 6;
	else if (AironetCellPowerLimit < 9)
		pAd->CommonCfg.TxPowerPercentage = 10;
	else if (AironetCellPowerLimit < 12)
		pAd->CommonCfg.TxPowerPercentage = 25;
	else if (AironetCellPowerLimit < 14)
		pAd->CommonCfg.TxPowerPercentage = 50;
	else if (AironetCellPowerLimit < 15)
		pAd->CommonCfg.TxPowerPercentage = 75;
	else
		pAd->CommonCfg.TxPowerPercentage = 100; 

	if (pAd->CommonCfg.TxPowerPercentage > pAd->CommonCfg.TxPowerDefault)
		pAd->CommonCfg.TxPowerPercentage = pAd->CommonCfg.TxPowerDefault;

}

CHAR	ConvertToRssi(
	IN PRTMP_ADAPTER	pAd,
	IN CHAR				Rssi,
	IN UCHAR			RssiNumber)
{
	UCHAR	RssiOffset, LNAGain;

	
	if (Rssi == 0)
		return -99;

	LNAGain = GET_LNA_GAIN(pAd);
    if (pAd->LatchRfRegs.Channel > 14)
    {
        if (RssiNumber == 0)
			RssiOffset = pAd->ARssiOffset0;
		else if (RssiNumber == 1)
			RssiOffset = pAd->ARssiOffset1;
		else
			RssiOffset = pAd->ARssiOffset2;
    }
    else
    {
        if (RssiNumber == 0)
			RssiOffset = pAd->BGRssiOffset0;
		else if (RssiNumber == 1)
			RssiOffset = pAd->BGRssiOffset1;
		else
			RssiOffset = pAd->BGRssiOffset2;
    }

    return (-12 - RssiOffset - LNAGain - Rssi);
}


VOID ScanNextChannel(
	IN PRTMP_ADAPTER pAd)
{
	HEADER_802_11   Hdr80211;
	PUCHAR          pOutBuffer = NULL;
	NDIS_STATUS     NStatus;
	ULONG           FrameLen = 0;
	UCHAR           SsidLen = 0, ScanType = pAd->MlmeAux.ScanType, BBPValue = 0;
#ifdef CONFIG_STA_SUPPORT
	USHORT          Status;
	PHEADER_802_11  pHdr80211;
#endif 
	UINT			ScanTimeIn5gChannel = SHORT_CHANNEL_TIME;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (MONITOR_ON(pAd))
			return;
	}
#endif 

#ifdef RALINK_ATE
	
	if (ATE_ON(pAd))
		return;
#endif 

	if (pAd->MlmeAux.Channel == 0)
	{
		if ((pAd->CommonCfg.BBPCurrentBW == BW_40)
#ifdef CONFIG_STA_SUPPORT
			&& (INFRA_ON(pAd)
				|| (pAd->OpMode == OPMODE_AP))
#endif 
			)
		{
			AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
			AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
			BBPValue &= (~0x18);
			BBPValue |= 0x10;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
			DBGPRINT(RT_DEBUG_TRACE, ("SYNC - End of SCAN, restore to 40MHz channel %d, Total BSS[%02d]\n",pAd->CommonCfg.CentralChannel, pAd->ScanTab.BssNr));
		}
		else
		{
			AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
			AsicLockChannel(pAd, pAd->CommonCfg.Channel);
			DBGPRINT(RT_DEBUG_TRACE, ("SYNC - End of SCAN, restore to channel %d, Total BSS[%02d]\n",pAd->CommonCfg.Channel, pAd->ScanTab.BssNr));
		}

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			
			
			
			
			
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) && (INFRA_ON(pAd)))
			{
				NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);
				if (NStatus	== NDIS_STATUS_SUCCESS)
				{
					pHdr80211 = (PHEADER_802_11) pOutBuffer;
					MgtMacHeaderInit(pAd, pHdr80211, SUBTYPE_NULL_FUNC, 1, pAd->CommonCfg.Bssid, pAd->CommonCfg.Bssid);
					pHdr80211->Duration = 0;
					pHdr80211->FC.Type = BTYPE_DATA;
					pHdr80211->FC.PwrMgmt = (pAd->StaCfg.Psm == PWR_SAVE);

					
					MiniportMMRequest(pAd, 0, pOutBuffer, sizeof(HEADER_802_11));
					DBGPRINT(RT_DEBUG_TRACE, ("MlmeScanReqAction -- Send PSM Data frame\n"));
					MlmeFreeMemory(pAd, pOutBuffer);
					RTMPusecDelay(5000);
				}
			}

			pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
			Status = MLME_SUCCESS;
			MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status);
		}
#endif 


		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
	}
	else
	{
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
		
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
			AsicForceWakeup(pAd, TRUE);

			
			if (pAd->StaCfg.Psm == PWR_SAVE)
				RTMP_SET_PSM_BIT(pAd, PWR_ACTIVE);
		}
#endif 

		AsicSwitchChannel(pAd, pAd->MlmeAux.Channel, TRUE);
		AsicLockChannel(pAd, pAd->MlmeAux.Channel);

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			if (pAd->MlmeAux.Channel > 14)
			{
				if ((pAd->CommonCfg.bIEEE80211H == 1) && RadarChannelCheck(pAd, pAd->MlmeAux.Channel))
				{
					ScanType = SCAN_PASSIVE;
					ScanTimeIn5gChannel = MIN_CHANNEL_TIME;
				}
			}

#ifdef CARRIER_DETECTION_SUPPORT 
			
			if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
			{
				ScanType = SCAN_PASSIVE;
				ScanTimeIn5gChannel = MIN_CHANNEL_TIME;
			}
#endif 
		}

#endif 

		
		if ((pAd->MlmeAux.Channel <= 14) && (pAd->MlmeAux.Channel >= 12) && ((pAd->CommonCfg.CountryRegion & 0x7f) == REGION_31_BG_BAND))
		{
			ScanType = SCAN_PASSIVE;
		}

		
		
		if (ScanType == FAST_SCAN_ACTIVE)
			RTMPSetTimer(&pAd->MlmeAux.ScanTimer, FAST_ACTIVE_SCAN_TIME);
		else 
		{
			if ((pAd->CommonCfg.PhyMode == PHY_11ABG_MIXED)
#ifdef DOT11_N_SUPPORT
				|| (pAd->CommonCfg.PhyMode == PHY_11ABGN_MIXED) || (pAd->CommonCfg.PhyMode == PHY_11AGN_MIXED)
#endif 
			)
			{
				if (pAd->MlmeAux.Channel > 14)
					RTMPSetTimer(&pAd->MlmeAux.ScanTimer, ScanTimeIn5gChannel);
				else
				RTMPSetTimer(&pAd->MlmeAux.ScanTimer, MIN_CHANNEL_TIME);
			}
			else
				RTMPSetTimer(&pAd->MlmeAux.ScanTimer, MAX_CHANNEL_TIME);
		}

		if ((ScanType == SCAN_ACTIVE)
			|| (ScanType == FAST_SCAN_ACTIVE)
			)
		{
			NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  
			if (NStatus != NDIS_STATUS_SUCCESS)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("SYNC - ScanNextChannel() allocate memory fail\n"));
#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				{
					pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
					Status = MLME_FAIL_NO_RESOURCE;
					MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status);
				}
#endif 

				return;
			}

			
			if ((ScanType == SCAN_ACTIVE) || (ScanType == FAST_SCAN_ACTIVE)
				)
				SsidLen = pAd->MlmeAux.SsidLen;
			else
				SsidLen = 0;

#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR, BROADCAST_ADDR);
#endif 


			MakeOutgoingFrame(pOutBuffer,               &FrameLen,
							  sizeof(HEADER_802_11),    &Hdr80211,
							  1,                        &SsidIe,
							  1,                        &SsidLen,
							  SsidLen,			        pAd->MlmeAux.Ssid,
							  1,                        &SupRateIe,
							  1,                        &pAd->CommonCfg.SupRateLen,
							  pAd->CommonCfg.SupRateLen,  pAd->CommonCfg.SupRate,
							  END_OF_ARGS);

			if (pAd->CommonCfg.ExtRateLen)
			{
				ULONG Tmp;
				MakeOutgoingFrame(pOutBuffer + FrameLen,            &Tmp,
								  1,                                &ExtRateIe,
								  1,                                &pAd->CommonCfg.ExtRateLen,
								  pAd->CommonCfg.ExtRateLen,          pAd->CommonCfg.ExtRate,
								  END_OF_ARGS);
				FrameLen += Tmp;
			}

#ifdef DOT11_N_SUPPORT
			if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
			{
				ULONG	Tmp;
				UCHAR	HtLen;
				UCHAR	BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
#ifdef RT_BIG_ENDIAN
				HT_CAPABILITY_IE HtCapabilityTmp;
#endif
				if (pAd->bBroadComHT == TRUE)
				{
					HtLen = pAd->MlmeAux.HtCapabilityLen + 4;
#ifdef RT_BIG_ENDIAN
					NdisMoveMemory(&HtCapabilityTmp, &pAd->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
					*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
					{
						EXT_HT_CAP_INFO extHtCapInfo;

						NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
						*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
						NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
					}
#else
					*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif 

					MakeOutgoingFrame(pOutBuffer + FrameLen,          &Tmp,
									1,                                &WpaIe,
									1,                                &HtLen,
									4,                                &BROADCOM[0],
									pAd->MlmeAux.HtCapabilityLen,     &HtCapabilityTmp,
									END_OF_ARGS);
#else
					MakeOutgoingFrame(pOutBuffer + FrameLen,          &Tmp,
									1,                                &WpaIe,
									1,                                &HtLen,
									4,                                &BROADCOM[0],
									pAd->MlmeAux.HtCapabilityLen,     &pAd->MlmeAux.HtCapability,
									END_OF_ARGS);
#endif 
				}
				else
				{
					HtLen = pAd->MlmeAux.HtCapabilityLen;
#ifdef RT_BIG_ENDIAN
					NdisMoveMemory(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, SIZE_HT_CAP_IE);
					*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
					{
						EXT_HT_CAP_INFO extHtCapInfo;

						NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
						*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
						NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
					}
#else
					*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif 

					MakeOutgoingFrame(pOutBuffer + FrameLen,          &Tmp,
									1,                                &HtCapIe,
									1,                                &HtLen,
									HtLen,                            &HtCapabilityTmp,
									END_OF_ARGS);
#else
					MakeOutgoingFrame(pOutBuffer + FrameLen,          &Tmp,
									1,                                &HtCapIe,
									1,                                &HtLen,
									HtLen,                            &pAd->CommonCfg.HtCapability,
									END_OF_ARGS);
#endif 
				}
				FrameLen += Tmp;

#ifdef DOT11N_DRAFT3
				if (pAd->CommonCfg.BACapability.field.b2040CoexistScanSup == 1)
				{
					ULONG		Tmp;
					HtLen = 1;
					MakeOutgoingFrame(pOutBuffer + FrameLen,            &Tmp,
									  1,					&ExtHtCapIe,
									  1,					&HtLen,
									  1,				&pAd->CommonCfg.BSSCoexist2040.word,
									  END_OF_ARGS);

					FrameLen += Tmp;
				}
#endif 
			}
#endif 


			MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
			MlmeFreeMemory(pAd, pOutBuffer);
		}

		

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			pAd->Mlme.SyncMachine.CurrState = SCAN_LISTEN;
#endif 

	}
}

VOID MgtProbReqMacHeaderInit(
	IN	PRTMP_ADAPTER	pAd,
	IN OUT PHEADER_802_11 pHdr80211,
	IN UCHAR SubType,
	IN UCHAR ToDs,
	IN PUCHAR pDA,
	IN PUCHAR pBssid)
{
	NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));

	pHdr80211->FC.Type = BTYPE_MGMT;
	pHdr80211->FC.SubType = SubType;
	if (SubType == SUBTYPE_ACK)
		pHdr80211->FC.Type = BTYPE_CNTL;
	pHdr80211->FC.ToDs = ToDs;
	COPY_MAC_ADDR(pHdr80211->Addr1, pDA);
	COPY_MAC_ADDR(pHdr80211->Addr2, pAd->CurrentAddress);
	COPY_MAC_ADDR(pHdr80211->Addr3, pBssid);
}
