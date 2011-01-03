

#include <linux/sched.h>
#include "../rt_config.h"


INT	Show_SSID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_WirelessMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_TxBurst_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_TxPreamble_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_TxPower_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_Channel_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_BGProtection_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_RTSThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_FragThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

#ifdef DOT11_N_SUPPORT
INT	Show_HtBw_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_HtMcs_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_HtGi_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_HtOpMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_HtExtcha_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_HtMpduDensity_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_HtBaWinSize_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_HtRdg_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_HtAmsdu_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_HtAutoBa_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);
#endif 

INT	Show_CountryRegion_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_CountryRegionABand_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_CountryCode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

#ifdef AGGREGATION_SUPPORT
INT	Show_PktAggregate_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);
#endif 

#ifdef WMM_SUPPORT
INT	Show_WmmCapable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);
#endif 

INT	Show_IEEE80211H_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

#ifdef CONFIG_STA_SUPPORT
INT	Show_NetworkType_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

#endif 

INT	Show_AuthMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_EncrypType_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_DefaultKeyID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_Key1_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_Key2_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_Key3_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_Key4_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

INT	Show_WPAPSK_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf);

static struct {
	PSTRING name;
	INT (*show_proc)(PRTMP_ADAPTER pAdapter, PSTRING arg);
} *PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC, RTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC[] = {
	{"SSID",					Show_SSID_Proc},
	{"WirelessMode",			Show_WirelessMode_Proc},
	{"TxBurst",					Show_TxBurst_Proc},
	{"TxPreamble",				Show_TxPreamble_Proc},
	{"TxPower",					Show_TxPower_Proc},
	{"Channel",					Show_Channel_Proc},
	{"BGProtection",			Show_BGProtection_Proc},
	{"RTSThreshold",			Show_RTSThreshold_Proc},
	{"FragThreshold",			Show_FragThreshold_Proc},
#ifdef DOT11_N_SUPPORT
	{"HtBw",					Show_HtBw_Proc},
	{"HtMcs",					Show_HtMcs_Proc},
	{"HtGi",					Show_HtGi_Proc},
	{"HtOpMode",				Show_HtOpMode_Proc},
	{"HtExtcha",				Show_HtExtcha_Proc},
	{"HtMpduDensity",			Show_HtMpduDensity_Proc},
	{"HtBaWinSize",		        Show_HtBaWinSize_Proc},
	{"HtRdg",				Show_HtRdg_Proc},
	{"HtAmsdu",				Show_HtAmsdu_Proc},
	{"HtAutoBa",		        Show_HtAutoBa_Proc},
#endif 
	{"CountryRegion",			Show_CountryRegion_Proc},
	{"CountryRegionABand",		Show_CountryRegionABand_Proc},
	{"CountryCode",				Show_CountryCode_Proc},
#ifdef AGGREGATION_SUPPORT
	{"PktAggregate",			Show_PktAggregate_Proc},
#endif

#ifdef WMM_SUPPORT
	{"WmmCapable",				Show_WmmCapable_Proc},
#endif
	{"IEEE80211H",				Show_IEEE80211H_Proc},
#ifdef CONFIG_STA_SUPPORT
    {"NetworkType",				Show_NetworkType_Proc},
#endif 
	{"AuthMode",				Show_AuthMode_Proc},
	{"EncrypType",				Show_EncrypType_Proc},
	{"DefaultKeyID",			Show_DefaultKeyID_Proc},
	{"Key1",					Show_Key1_Proc},
	{"Key2",					Show_Key2_Proc},
	{"Key3",					Show_Key3_Proc},
	{"Key4",					Show_Key4_Proc},
	{"WPAPSK",					Show_WPAPSK_Proc},
	{NULL, NULL}
};


INT Set_DriverVersion_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		DBGPRINT(RT_DEBUG_TRACE, ("Driver version-%s\n", STA_DRIVER_VERSION));
#endif 


    return TRUE;
}


INT Set_CountryRegion_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	int retval;

#ifdef EXT_BUILD_CHANNEL_LIST
	return -EOPNOTSUPP;
#endif 

	retval = RT_CfgSetCountryRegion(pAd, arg, BAND_24G);
	if (retval == FALSE)
		return FALSE;

	
	BuildChannelList(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_CountryRegion_Proc::(CountryRegion=%d)\n", pAd->CommonCfg.CountryRegion));

	return TRUE;
}



INT Set_CountryRegionABand_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	int retval;

#ifdef EXT_BUILD_CHANNEL_LIST
	return -EOPNOTSUPP;
#endif 

	retval = RT_CfgSetCountryRegion(pAd, arg, BAND_5G);
	if (retval == FALSE)
		return FALSE;

	
	BuildChannelList(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_CountryRegionABand_Proc::(CountryRegion=%d)\n", pAd->CommonCfg.CountryRegionForABand));

	return TRUE;
}



INT	Set_WirelessMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	INT	success = TRUE;

	success = RT_CfgSetWirelessMode(pAd, arg);
	if (success)
	{

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			LONG	WirelessMode = pAd->CommonCfg.PhyMode;

			RTMPSetPhyMode(pAd, WirelessMode);
#ifdef DOT11_N_SUPPORT
			if (WirelessMode >= PHY_11ABGN_MIXED)
			{
				pAd->CommonCfg.BACapability.field.AutoBA = TRUE;
				pAd->CommonCfg.REGBACapability.field.AutoBA = TRUE;
			}
			else
			{
				pAd->CommonCfg.BACapability.field.AutoBA = FALSE;
				pAd->CommonCfg.REGBACapability.field.AutoBA = FALSE;
			}
#endif 
			
			if (pAd->StaCfg.BssType == BSS_ADHOC)
			{
				MlmeUpdateTxRates(pAd, FALSE, 0);
				MakeIbssBeacon(pAd);           
				AsicEnableIbssSync(pAd);       
			}
		}
#endif 

		
#ifdef DOT11_N_SUPPORT
		SetCommonHT(pAd);
#endif 
		DBGPRINT(RT_DEBUG_TRACE, ("Set_WirelessMode_Proc::(=%d)\n", pAd->CommonCfg.PhyMode));
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_WirelessMode_Proc::parameters out of range\n"));
	}

	return success;
}


INT	Set_Channel_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	INT		success = TRUE;
	UCHAR	Channel;

	Channel = (UCHAR) simple_strtol(arg, 0, 10);

	
	if (ChannelSanity(pAd, Channel) == TRUE)
	{
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			pAd->CommonCfg.Channel = Channel;

			if (MONITOR_ON(pAd))
			{
#ifdef DOT11_N_SUPPORT
				N_ChannelCheck(pAd);
				if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED &&
					pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)
				{
					N_SetCenCh(pAd);
						AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
						AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
					DBGPRINT(RT_DEBUG_TRACE, ("BW_40, control_channel(%d), CentralChannel(%d) \n",
								pAd->CommonCfg.Channel, pAd->CommonCfg.CentralChannel));
				}
				else
#endif 
				{
					AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
					AsicLockChannel(pAd, pAd->CommonCfg.Channel);
					DBGPRINT(RT_DEBUG_TRACE, ("BW_20, Channel(%d)\n", pAd->CommonCfg.Channel));
				}
			}
		}
#endif 
		success = TRUE;
	}
	else
	{

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			success = FALSE;
#endif 
	}


	if (success == TRUE)
		DBGPRINT(RT_DEBUG_TRACE, ("Set_Channel_Proc::(Channel=%d)\n", pAd->CommonCfg.Channel));

	return success;
}



INT	Set_ShortSlot_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	int retval;

	retval = RT_CfgSetShortSlot(pAd, arg);
	if (retval == TRUE)
		DBGPRINT(RT_DEBUG_TRACE, ("Set_ShortSlot_Proc::(ShortSlot=%d)\n", pAd->CommonCfg.bUseShortSlotTime));

	return retval;
}



INT	Set_TxPower_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	LONG TxPower;
	INT   success = FALSE;

	TxPower = simple_strtol(arg, 0, 10);
	if (TxPower <= 100)
	{

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			pAd->CommonCfg.TxPowerDefault = TxPower;
			pAd->CommonCfg.TxPowerPercentage = pAd->CommonCfg.TxPowerDefault;
		}
#endif 
		success = TRUE;
	}
	else
		success = FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("Set_TxPower_Proc::(TxPowerPercentage=%ld)\n", pAd->CommonCfg.TxPowerPercentage));

	return success;
}


INT	Set_BGProtection_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	switch (simple_strtol(arg, 0, 10))
	{
		case 0: 
			pAd->CommonCfg.UseBGProtection = 0;
			break;
		case 1: 
			pAd->CommonCfg.UseBGProtection = 1;
			break;
		case 2: 
			pAd->CommonCfg.UseBGProtection = 2;
			break;
		default:  
			return FALSE;
	}


	DBGPRINT(RT_DEBUG_TRACE, ("Set_BGProtection_Proc::(BGProtection=%ld)\n", pAd->CommonCfg.UseBGProtection));

	return TRUE;
}


INT	Set_TxPreamble_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	RT_802_11_PREAMBLE	Preamble;

	Preamble = simple_strtol(arg, 0, 10);


	switch (Preamble)
	{
		case Rt802_11PreambleShort:
			pAd->CommonCfg.TxPreamble = Preamble;
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				MlmeSetTxPreamble(pAd, Rt802_11PreambleShort);
#endif 
			break;
		case Rt802_11PreambleLong:
#ifdef CONFIG_STA_SUPPORT
		case Rt802_11PreambleAuto:
			
			
#endif 
			pAd->CommonCfg.TxPreamble = Preamble;
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				MlmeSetTxPreamble(pAd, Rt802_11PreambleLong);
#endif 
			break;
		default: 
			return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_TxPreamble_Proc::(TxPreamble=%ld)\n", pAd->CommonCfg.TxPreamble));

	return TRUE;
}


INT	Set_RTSThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	 NDIS_802_11_RTS_THRESHOLD           RtsThresh;

	RtsThresh = simple_strtol(arg, 0, 10);

	if((RtsThresh > 0) && (RtsThresh <= MAX_RTS_THRESHOLD))
		pAd->CommonCfg.RtsThreshold  = (USHORT)RtsThresh;
#ifdef CONFIG_STA_SUPPORT
	else if (RtsThresh == 0)
		pAd->CommonCfg.RtsThreshold = MAX_RTS_THRESHOLD;
#endif 
	else
		return FALSE; 

	DBGPRINT(RT_DEBUG_TRACE, ("Set_RTSThreshold_Proc::(RTSThreshold=%d)\n", pAd->CommonCfg.RtsThreshold));

	return TRUE;
}


INT	Set_FragThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	 NDIS_802_11_FRAGMENTATION_THRESHOLD     FragThresh;

	FragThresh = simple_strtol(arg, 0, 10);

	if (FragThresh > MAX_FRAG_THRESHOLD || FragThresh < MIN_FRAG_THRESHOLD)
	{
		
		pAd->CommonCfg.FragmentThreshold = MAX_FRAG_THRESHOLD;
	}
	else if (FragThresh % 2 == 1)
	{
		
		
		pAd->CommonCfg.FragmentThreshold = (USHORT)(FragThresh - 1);
	}
	else
	{
		pAd->CommonCfg.FragmentThreshold = (USHORT)FragThresh;
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (pAd->CommonCfg.FragmentThreshold == MAX_FRAG_THRESHOLD)
			pAd->CommonCfg.bUseZeroToDisableFragment = TRUE;
		else
			pAd->CommonCfg.bUseZeroToDisableFragment = FALSE;
	}
#endif 

	DBGPRINT(RT_DEBUG_TRACE, ("Set_FragThreshold_Proc::(FragThreshold=%d)\n", pAd->CommonCfg.FragmentThreshold));

	return TRUE;
}


INT	Set_TxBurst_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	LONG TxBurst;

	TxBurst = simple_strtol(arg, 0, 10);
	if (TxBurst == 1)
		pAd->CommonCfg.bEnableTxBurst = TRUE;
	else if (TxBurst == 0)
		pAd->CommonCfg.bEnableTxBurst = FALSE;
	else
		return FALSE;  

	DBGPRINT(RT_DEBUG_TRACE, ("Set_TxBurst_Proc::(TxBurst=%d)\n", pAd->CommonCfg.bEnableTxBurst));

	return TRUE;
}

#ifdef AGGREGATION_SUPPORT

INT	Set_PktAggregate_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	LONG aggre;

	aggre = simple_strtol(arg, 0, 10);

	if (aggre == 1)
		pAd->CommonCfg.bAggregationCapable = TRUE;
	else if (aggre == 0)
		pAd->CommonCfg.bAggregationCapable = FALSE;
	else
		return FALSE;  


	DBGPRINT(RT_DEBUG_TRACE, ("Set_PktAggregate_Proc::(AGGRE=%d)\n", pAd->CommonCfg.bAggregationCapable));

	return TRUE;
}
#endif


#ifdef INF_AMAZON_PPA
INT	Set_INF_AMAZON_SE_PPA_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg)
{
	ULONG aggre;
	UINT status;

	aggre = simple_strtol(arg, 0, 10);

	if (aggre == 1)
	{
		if(pAd->PPAEnable==TRUE)
		{
			printk("INF_AMAZON_SE_PPA already enabled \n");
		}
		else
		{
			if (ppa_hook_directpath_register_dev_fn)
			{
				UINT32 g_if_id;

				if (pAd->pDirectpathCb == NULL)
				{
					 pAd->pDirectpathCb = (PPA_DIRECTPATH_CB *) kmalloc (sizeof(PPA_DIRECTPATH_CB), GFP_ATOMIC);
				        printk("Realloc memory for  pDirectpathCb ??\n");
				}

				
				pAd->pDirectpathCb->rx_fn = NULL;
				pAd->pDirectpathCb->stop_tx_fn = NULL;
				pAd->pDirectpathCb->start_tx_fn = NULL;

				status = ppa_hook_directpath_register_dev_fn(&g_if_id, pAd->net_dev, pAd->pDirectpathCb, PPA_F_DIRECTPATH_ETH_IF);

				if(status==1)
				{
					pAd->g_if_id=g_if_id;
					printk("register INF_AMAZON_SE_PPA success :ret:%d id:%d:%d\n",status,pAd->g_if_id,g_if_id);
					pAd->PPAEnable=TRUE;
				}
				else
				{
					printk("register INF_AMAZON_SE_PPA fail :ret:%d\n",status);
				}

			}
			else
			{
				printk("INF_AMAZON_SE_PPA enable fail : there is no INF_AMAZON_SE_PPA module . \n");
			}
		}


	}
	else if (aggre == 0)
	{
		if(pAd->PPAEnable==FALSE)
		{

printk("INF_AMAZON_SE_PPA already disable \n");
		}
		else
		{
			if (ppa_hook_directpath_register_dev_fn)
			{
				UINT32 g_if_id;
				g_if_id=pAd->g_if_id;
				printk("g_if_id=%d \n",pAd->g_if_id);
				status=ppa_hook_directpath_register_dev_fn(&g_if_id, pAd->net_dev, NULL, PPA_F_DIRECTPATH_DEREGISTER);

				if(status==1)
				{
					pAd->g_if_id=0;
					printk("unregister INF_AMAZON_SE_PPA success :ret:%d\n",status);
					pAd->PPAEnable=FALSE;
				}
				else
				{
					printk("unregister INF_AMAZON_SE_PPA fail :ret:%d\n",status);
				}

			}
			else
			{
				printk("INF_AMAZON_SE_PPA enable fail : there is no INF_AMAZON_SE_PPA module . \n");
			}
		}

	}
	else
	{
		printk("Invalid argument %d \n",aggre);
		return FALSE;  
	}

	return TRUE;

}
#endif 



INT	Set_IEEE80211H_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
    LONG ieee80211h;

	ieee80211h = simple_strtol(arg, 0, 10);

	if (ieee80211h == 1)
		pAd->CommonCfg.bIEEE80211H = TRUE;
	else if (ieee80211h == 0)
		pAd->CommonCfg.bIEEE80211H = FALSE;
	else
		return FALSE;  

	DBGPRINT(RT_DEBUG_TRACE, ("Set_IEEE80211H_Proc::(IEEE80211H=%d)\n", pAd->CommonCfg.bIEEE80211H));

	return TRUE;
}


#ifdef DBG

INT	Set_Debug_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	DBGPRINT(RT_DEBUG_TRACE, ("==> Set_Debug_Proc *******************\n"));

    if(simple_strtol(arg, 0, 10) <= RT_DEBUG_LOUD)
        RTDebugLevel = simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_TRACE, ("<== Set_Debug_Proc(RTDebugLevel = %ld)\n", RTDebugLevel));

	return TRUE;
}
#endif

INT	Show_DescInfo_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
#ifdef RTMP_MAC_PCI
	INT i, QueIdx=0;

        PRT28XX_RXD_STRUC pRxD;
        PTXD_STRUC pTxD;
	PRTMP_TX_RING	pTxRing = &pAd->TxRing[QueIdx];
	PRTMP_MGMT_RING	pMgmtRing = &pAd->MgmtRing;
	PRTMP_RX_RING	pRxRing = &pAd->RxRing;

	for(i=0;i<TX_RING_SIZE;i++)
	{
	    pTxD = (PTXD_STRUC) pTxRing->Cell[i].AllocVa;
	    DBGPRINT(RT_DEBUG_OFF, ("Desc #%d\n",i));
	    hex_dump("Tx Descriptor", (PUCHAR)pTxD, 16);
	    DBGPRINT(RT_DEBUG_OFF, ("pTxD->DMADONE = %x\n", pTxD->DMADONE));
	}
	DBGPRINT(RT_DEBUG_OFF, ("---------------------------------------------------\n"));
	for(i=0;i<MGMT_RING_SIZE;i++)
	{
	    pTxD = (PTXD_STRUC) pMgmtRing->Cell[i].AllocVa;
	    DBGPRINT(RT_DEBUG_OFF, ("Desc #%d\n",i));
	    hex_dump("Mgmt Descriptor", (PUCHAR)pTxD, 16);
	    DBGPRINT(RT_DEBUG_OFF, ("pMgmt->DMADONE = %x\n", pTxD->DMADONE));
	}
	DBGPRINT(RT_DEBUG_OFF, ("---------------------------------------------------\n"));
	for(i=0;i<RX_RING_SIZE;i++)
	{
	    pRxD = (PRT28XX_RXD_STRUC) pRxRing->Cell[i].AllocVa;
	    DBGPRINT(RT_DEBUG_OFF, ("Desc #%d\n",i));
	    hex_dump("Rx Descriptor", (PUCHAR)pRxD, 16);
	    DBGPRINT(RT_DEBUG_OFF, ("pRxD->DDONE = %x\n", pRxD->DDONE));
	}
#endif 

	return TRUE;
}


INT	Set_ResetStatCounter_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	
	

	DBGPRINT(RT_DEBUG_TRACE, ("==>Set_ResetStatCounter_Proc\n"));

	
	NICUpdateRawCounters(pAd);

	NdisZeroMemory(&pAd->WlanCounters, sizeof(COUNTER_802_11));
	NdisZeroMemory(&pAd->Counters8023, sizeof(COUNTER_802_3));
	NdisZeroMemory(&pAd->RalinkCounters, sizeof(COUNTER_RALINK));

	


	return TRUE;
}



BOOLEAN RTMPCheckStrPrintAble(
    IN  CHAR *pInPutStr,
    IN  UCHAR strLen)
{
    UCHAR i=0;

    for (i=0; i<strLen; i++)
    {
        if ((pInPutStr[i] < 0x21) ||
            (pInPutStr[i] > 0x7E))
            return FALSE;
    }

    return TRUE;
}


#ifdef CONFIG_STA_SUPPORT
VOID    RTMPSetDesiredRates(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  LONG            Rates)
{
    NDIS_802_11_RATES aryRates;

    memset(&aryRates, 0x00, sizeof(NDIS_802_11_RATES));
    switch (pAdapter->CommonCfg.PhyMode)
    {
        case PHY_11A: 
            switch (Rates)
            {
                case 6000000: 
                    aryRates[0] = 0x0c; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_0;
                    break;
                case 9000000: 
                    aryRates[0] = 0x12; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_1;
                    break;
                case 12000000: 
                    aryRates[0] = 0x18; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_2;
                    break;
                case 18000000: 
                    aryRates[0] = 0x24; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_3;
                    break;
                case 24000000: 
                    aryRates[0] = 0x30; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_4;
                    break;
                case 36000000: 
                    aryRates[0] = 0x48; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_5;
                    break;
                case 48000000: 
                    aryRates[0] = 0x60; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_6;
                    break;
                case 54000000: 
                    aryRates[0] = 0x6c; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_7;
                    break;
                case -1: 
                default:
                    aryRates[0] = 0x6c; 
                    aryRates[1] = 0x60; 
                    aryRates[2] = 0x48; 
                    aryRates[3] = 0x30; 
                    aryRates[4] = 0x24; 
                    aryRates[5] = 0x18; 
                    aryRates[6] = 0x12; 
                    aryRates[7] = 0x0c; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_AUTO;
                    break;
            }
            break;
        case PHY_11BG_MIXED: 
        case PHY_11B: 
        case PHY_11ABG_MIXED: 
        default:
            switch (Rates)
            {
                case 1000000: 
                    aryRates[0] = 0x02;
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_0;
                    break;
                case 2000000: 
                    aryRates[0] = 0x04;
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_1;
                    break;
                case 5000000: 
                    aryRates[0] = 0x0b; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_2;
                    break;
                case 11000000: 
                    aryRates[0] = 0x16; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_3;
                    break;
                case 6000000: 
                    aryRates[0] = 0x0c; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_0;
                    break;
                case 9000000: 
                    aryRates[0] = 0x12; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_1;
                    break;
                case 12000000: 
                    aryRates[0] = 0x18; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_2;
                    break;
                case 18000000: 
                    aryRates[0] = 0x24; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_3;
                    break;
                case 24000000: 
                    aryRates[0] = 0x30; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_4;
                    break;
                case 36000000: 
                    aryRates[0] = 0x48; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_5;
                    break;
                case 48000000: 
                    aryRates[0] = 0x60; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_6;
                    break;
                case 54000000: 
                    aryRates[0] = 0x6c; 
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_7;
                    break;
                case -1: 
                default:
                    if (pAdapter->CommonCfg.PhyMode == PHY_11B)
                    { 
                        aryRates[0] = 0x16; 
                        aryRates[1] = 0x0b; 
                        aryRates[2] = 0x04; 
                        aryRates[3] = 0x02; 
                    }
                    else
                    { 
                        aryRates[0] = 0x6c; 
                        aryRates[1] = 0x60; 
                        aryRates[2] = 0x48; 
                        aryRates[3] = 0x30; 
                        aryRates[4] = 0x16; 
                        aryRates[5] = 0x0b; 
                        aryRates[6] = 0x04; 
                        aryRates[7] = 0x02; 
                    }
                    pAdapter->StaCfg.DesiredTransmitSetting.field.MCS = MCS_AUTO;
                    break;
            }
            break;
    }

    NdisZeroMemory(pAdapter->CommonCfg.DesireRate, MAX_LEN_OF_SUPPORTED_RATES);
    NdisMoveMemory(pAdapter->CommonCfg.DesireRate, &aryRates, sizeof(NDIS_802_11_RATES));
    DBGPRINT(RT_DEBUG_TRACE, (" RTMPSetDesiredRates (%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x)\n",
        pAdapter->CommonCfg.DesireRate[0],pAdapter->CommonCfg.DesireRate[1],
        pAdapter->CommonCfg.DesireRate[2],pAdapter->CommonCfg.DesireRate[3],
        pAdapter->CommonCfg.DesireRate[4],pAdapter->CommonCfg.DesireRate[5],
        pAdapter->CommonCfg.DesireRate[6],pAdapter->CommonCfg.DesireRate[7] ));
    
    MlmeUpdateTxRates(pAdapter, FALSE, 0);
}

NDIS_STATUS RTMPWPARemoveKeyProc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PVOID			pBuf)
{
	PNDIS_802_11_REMOVE_KEY pKey;
	ULONG					KeyIdx;
	NDIS_STATUS			Status = NDIS_STATUS_FAILURE;
	BOOLEAN		bTxKey;			
	BOOLEAN		bPairwise;		
	BOOLEAN		bKeyRSC;		
								
	BOOLEAN		bAuthenticator; 
	INT		i;

	DBGPRINT(RT_DEBUG_TRACE,("---> RTMPWPARemoveKeyProc\n"));

	pKey = (PNDIS_802_11_REMOVE_KEY) pBuf;
	KeyIdx = pKey->KeyIndex & 0xff;
	
	bTxKey		   = (pKey->KeyIndex & 0x80000000) ? TRUE : FALSE;
	
	bPairwise	   = (pKey->KeyIndex & 0x40000000) ? TRUE : FALSE;
	
	bKeyRSC		   = (pKey->KeyIndex & 0x20000000) ? TRUE : FALSE;
	
	bAuthenticator = (pKey->KeyIndex & 0x10000000) ? TRUE : FALSE;

	
	if (bTxKey == TRUE)
		return(NDIS_STATUS_INVALID_DATA);

	
	if (bPairwise)
	{
		
		
		for (i = 0; i < SHARE_KEY_NUM; i++)
		{
			if (MAC_ADDR_EQUAL(pAd->SharedKey[BSS0][i].BssId, pKey->BSSID))
			{
				DBGPRINT(RT_DEBUG_TRACE,("RTMPWPARemoveKeyProc(KeyIdx=%d)\n", i));
				pAd->SharedKey[BSS0][i].KeyLen = 0;
				pAd->SharedKey[BSS0][i].CipherAlg = CIPHER_NONE;
				AsicRemoveSharedKeyEntry(pAd, BSS0, (UCHAR)i);
				Status = NDIS_STATUS_SUCCESS;
				break;
			}
		}
	}
	
	else
	{
		
		
		DBGPRINT(RT_DEBUG_TRACE,("RTMPWPARemoveKeyProc(KeyIdx=%ld)\n", KeyIdx));
		pAd->SharedKey[BSS0][KeyIdx].KeyLen = 0;
		pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_NONE;
		AsicRemoveSharedKeyEntry(pAd, BSS0, (UCHAR)KeyIdx);
		Status = NDIS_STATUS_SUCCESS;
	}

	return (Status);
}
#endif 


#ifdef CONFIG_STA_SUPPORT

VOID	RTMPWPARemoveAllKeys(
	IN	PRTMP_ADAPTER	pAd)
{

	UCHAR	i;

	DBGPRINT(RT_DEBUG_TRACE,("RTMPWPARemoveAllKeys(AuthMode=%d, WepStatus=%d)\n", pAd->StaCfg.AuthMode, pAd->StaCfg.WepStatus));
	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP);
	
	
	if (pAd->StaCfg.AuthMode < Ndis802_11AuthModeWPA)
		return;

	
	
	if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPANone)
		return;

	
	AsicRemovePairwiseKeyEntry(pAd, BSS0, BSSID_WCID);

	
	for (i = 0; i < SHARE_KEY_NUM; i++)
    {
		DBGPRINT(RT_DEBUG_TRACE,("remove %s key #%d\n", CipherName[pAd->SharedKey[BSS0][i].CipherAlg], i));
		NdisZeroMemory(&pAd->SharedKey[BSS0][i], sizeof(CIPHER_KEY));

		AsicRemoveSharedKeyEntry(pAd, BSS0, i);
	}
	RTMP_SET_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP);

}
#endif 




VOID	RTMPSetPhyMode(
	IN	PRTMP_ADAPTER	pAd,
	IN	ULONG phymode)
{
	INT i;
	

	
	
	pAd->CommonCfg.PhyMode = (UCHAR)phymode;

	DBGPRINT(RT_DEBUG_TRACE,("RTMPSetPhyMode : PhyMode=%d, channel=%d \n", pAd->CommonCfg.PhyMode, pAd->CommonCfg.Channel));
#ifdef EXT_BUILD_CHANNEL_LIST
	BuildChannelListEx(pAd);
#else
	BuildChannelList(pAd);
#endif 

	
	for (i = 0; i < pAd->ChannelListNum; i++)
	{
		if (pAd->CommonCfg.Channel == pAd->ChannelList[i].Channel)
			break;
	}

	if (i == pAd->ChannelListNum)
	{
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			pAd->CommonCfg.Channel = FirstChannel(pAd);
#endif 
		DBGPRINT(RT_DEBUG_ERROR, ("RTMPSetPhyMode: channel is out of range, use first channel=%d \n", pAd->CommonCfg.Channel));
	}

	NdisZeroMemory(pAd->CommonCfg.SupRate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(pAd->CommonCfg.ExtRate, MAX_LEN_OF_SUPPORTED_RATES);
	NdisZeroMemory(pAd->CommonCfg.DesireRate, MAX_LEN_OF_SUPPORTED_RATES);
	switch (phymode) {
		case PHY_11B:
			pAd->CommonCfg.SupRate[0]  = 0x82;	  
			pAd->CommonCfg.SupRate[1]  = 0x84;	  
			pAd->CommonCfg.SupRate[2]  = 0x8B;	  
			pAd->CommonCfg.SupRate[3]  = 0x96;	  
			pAd->CommonCfg.SupRateLen  = 4;
			pAd->CommonCfg.ExtRateLen  = 0;
			pAd->CommonCfg.DesireRate[0]  = 2;	   
			pAd->CommonCfg.DesireRate[1]  = 4;	   
			pAd->CommonCfg.DesireRate[2]  = 11;    
			pAd->CommonCfg.DesireRate[3]  = 22;    
			
			break;

		case PHY_11G:
		case PHY_11BG_MIXED:
		case PHY_11ABG_MIXED:
#ifdef DOT11_N_SUPPORT
		case PHY_11N_2_4G:
		case PHY_11ABGN_MIXED:
		case PHY_11BGN_MIXED:
		case PHY_11GN_MIXED:
#endif 
			pAd->CommonCfg.SupRate[0]  = 0x82;	  
			pAd->CommonCfg.SupRate[1]  = 0x84;	  
			pAd->CommonCfg.SupRate[2]  = 0x8B;	  
			pAd->CommonCfg.SupRate[3]  = 0x96;	  
			pAd->CommonCfg.SupRate[4]  = 0x12;	  
			pAd->CommonCfg.SupRate[5]  = 0x24;	  
			pAd->CommonCfg.SupRate[6]  = 0x48;	  
			pAd->CommonCfg.SupRate[7]  = 0x6c;	  
			pAd->CommonCfg.SupRateLen  = 8;
			pAd->CommonCfg.ExtRate[0]  = 0x0C;	  
			pAd->CommonCfg.ExtRate[1]  = 0x18;	  
			pAd->CommonCfg.ExtRate[2]  = 0x30;	  
			pAd->CommonCfg.ExtRate[3]  = 0x60;	  
			pAd->CommonCfg.ExtRateLen  = 4;
			pAd->CommonCfg.DesireRate[0]  = 2;	   
			pAd->CommonCfg.DesireRate[1]  = 4;	   
			pAd->CommonCfg.DesireRate[2]  = 11;    
			pAd->CommonCfg.DesireRate[3]  = 22;    
			pAd->CommonCfg.DesireRate[4]  = 12;    
			pAd->CommonCfg.DesireRate[5]  = 18;    
			pAd->CommonCfg.DesireRate[6]  = 24;    
			pAd->CommonCfg.DesireRate[7]  = 36;    
			pAd->CommonCfg.DesireRate[8]  = 48;    
			pAd->CommonCfg.DesireRate[9]  = 72;    
			pAd->CommonCfg.DesireRate[10] = 96;    
			pAd->CommonCfg.DesireRate[11] = 108;   
			break;

		case PHY_11A:
#ifdef DOT11_N_SUPPORT
		case PHY_11AN_MIXED:
		case PHY_11AGN_MIXED:
		case PHY_11N_5G:
#endif 
			pAd->CommonCfg.SupRate[0]  = 0x8C;	  
			pAd->CommonCfg.SupRate[1]  = 0x12;	  
			pAd->CommonCfg.SupRate[2]  = 0x98;	  
			pAd->CommonCfg.SupRate[3]  = 0x24;	  
			pAd->CommonCfg.SupRate[4]  = 0xb0;	  
			pAd->CommonCfg.SupRate[5]  = 0x48;	  
			pAd->CommonCfg.SupRate[6]  = 0x60;	  
			pAd->CommonCfg.SupRate[7]  = 0x6c;	  
			pAd->CommonCfg.SupRateLen  = 8;
			pAd->CommonCfg.ExtRateLen  = 0;
			pAd->CommonCfg.DesireRate[0]  = 12;    
			pAd->CommonCfg.DesireRate[1]  = 18;    
			pAd->CommonCfg.DesireRate[2]  = 24;    
			pAd->CommonCfg.DesireRate[3]  = 36;    
			pAd->CommonCfg.DesireRate[4]  = 48;    
			pAd->CommonCfg.DesireRate[5]  = 72;    
			pAd->CommonCfg.DesireRate[6]  = 96;    
			pAd->CommonCfg.DesireRate[7]  = 108;   
			
			break;

		default:
			break;
	}


	pAd->CommonCfg.BandState = UNKNOWN_BAND;
}


#ifdef DOT11_N_SUPPORT

VOID	RTMPSetHT(
	IN	PRTMP_ADAPTER	pAd,
	IN	OID_SET_HT_PHYMODE *pHTPhyMode)
{
	
	UINT32	Value = 0;
	UCHAR	BBPValue = 0;
	UCHAR	BBP3Value = 0;
	UCHAR	RxStream = pAd->CommonCfg.RxStream;

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPSetHT : HT_mode(%d), ExtOffset(%d), MCS(%d), BW(%d), STBC(%d), SHORTGI(%d)\n",
										pHTPhyMode->HtMode, pHTPhyMode->ExtOffset,
										pHTPhyMode->MCS, pHTPhyMode->BW,
										pHTPhyMode->STBC, pHTPhyMode->SHORTGI));

	
	RTMPZeroMemory(&pAd->CommonCfg.HtCapability, sizeof(pAd->CommonCfg.HtCapability));
	RTMPZeroMemory(&pAd->CommonCfg.AddHTInfo, sizeof(pAd->CommonCfg.AddHTInfo));
	RTMPZeroMemory(&pAd->CommonCfg.NewExtChanOffset, sizeof(pAd->CommonCfg.NewExtChanOffset));
	RTMPZeroMemory(&pAd->CommonCfg.DesiredHtPhy, sizeof(pAd->CommonCfg.DesiredHtPhy));

	if (pAd->CommonCfg.bRdg)
	{
		pAd->CommonCfg.HtCapability.ExtHtCapInfo.PlusHTC = 1;
		pAd->CommonCfg.HtCapability.ExtHtCapInfo.RDGSupport = 1;
	}
	else
	{
		pAd->CommonCfg.HtCapability.ExtHtCapInfo.PlusHTC = 0;
		pAd->CommonCfg.HtCapability.ExtHtCapInfo.RDGSupport = 0;
	}

	pAd->CommonCfg.HtCapability.HtCapParm.MaxRAmpduFactor = 3;
	pAd->CommonCfg.DesiredHtPhy.MaxRAmpduFactor = 3;

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPSetHT : RxBAWinLimit = %d\n", pAd->CommonCfg.BACapability.field.RxBAWinLimit));

	
	pAd->CommonCfg.DesiredHtPhy.AmsduEnable = (USHORT)pAd->CommonCfg.BACapability.field.AmsduEnable;
	pAd->CommonCfg.DesiredHtPhy.AmsduSize = (UCHAR)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.DesiredHtPhy.MimoPs = (UCHAR)pAd->CommonCfg.BACapability.field.MMPSmode;
	pAd->CommonCfg.DesiredHtPhy.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;

	pAd->CommonCfg.HtCapability.HtCapInfo.AMsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.HtCapability.HtCapInfo.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	pAd->CommonCfg.HtCapability.HtCapParm.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPSetHT : AMsduSize = %d, MimoPs = %d, MpduDensity = %d, MaxRAmpduFactor = %d\n",
													pAd->CommonCfg.DesiredHtPhy.AmsduSize,
													pAd->CommonCfg.DesiredHtPhy.MimoPs,
													pAd->CommonCfg.DesiredHtPhy.MpduDensity,
													pAd->CommonCfg.DesiredHtPhy.MaxRAmpduFactor));

	if(pHTPhyMode->HtMode == HTMODE_GF)
	{
		pAd->CommonCfg.HtCapability.HtCapInfo.GF = 1;
		pAd->CommonCfg.DesiredHtPhy.GF = 1;
	}
	else
		pAd->CommonCfg.DesiredHtPhy.GF = 0;

	
	switch (RxStream)
	{
		case 1:
			pAd->CommonCfg.HtCapability.MCSSet[0] =  0xff;
			pAd->CommonCfg.HtCapability.MCSSet[1] =  0x00;
			break;

		case 2:
			pAd->CommonCfg.HtCapability.MCSSet[0] =  0xff;
			pAd->CommonCfg.HtCapability.MCSSet[1] =  0xff;
			break;

		case 3: 
			pAd->CommonCfg.HtCapability.MCSSet[0] =  0xff;
			pAd->CommonCfg.HtCapability.MCSSet[1] =  0xff;
			pAd->CommonCfg.HtCapability.MCSSet[2] =  0xff;
			break;
	}

	if (pAd->CommonCfg.bForty_Mhz_Intolerant && (pAd->CommonCfg.Channel <= 14) && (pHTPhyMode->BW == BW_40) )
	{
		pHTPhyMode->BW = BW_20;
		pAd->CommonCfg.HtCapability.HtCapInfo.Forty_Mhz_Intolerant = 1;
	}

	if(pHTPhyMode->BW == BW_40)
	{
		pAd->CommonCfg.HtCapability.MCSSet[4] = 0x1; 
		pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = 1;
		if (pAd->CommonCfg.Channel <= 14)
			pAd->CommonCfg.HtCapability.HtCapInfo.CCKmodein40 = 1;

		pAd->CommonCfg.DesiredHtPhy.ChannelWidth = 1;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 1;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = (pHTPhyMode->ExtOffset == EXTCHA_BELOW)? (EXTCHA_BELOW): EXTCHA_ABOVE;
		
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBP3Value);
		if ((pHTPhyMode->ExtOffset == EXTCHA_BELOW))
		{
			Value |= 0x1;
			BBP3Value |= (0x20);
			RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);
		}
		else if ((pHTPhyMode->ExtOffset == EXTCHA_ABOVE))
		{
			Value &= 0xfe;
			BBP3Value &= (~0x20);
			RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);
		}

		
		
		if ((pAd->OpMode == OPMODE_AP) || INFRA_ON(pAd) || ADHOC_ON(pAd)
			)
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
			BBPValue &= (~0x18);
			BBPValue |= 0x10;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);

			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBP3Value);
			pAd->CommonCfg.BBPCurrentBW = BW_40;
		}
	}
	else
	{
		pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = 0;
		pAd->CommonCfg.DesiredHtPhy.ChannelWidth = 0;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 0;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = EXTCHA_NONE;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
		
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
			BBPValue &= (~0x18);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
			pAd->CommonCfg.BBPCurrentBW = BW_20;
		}
	}

	if(pHTPhyMode->STBC == STBC_USE)
	{
		pAd->CommonCfg.HtCapability.HtCapInfo.TxSTBC = 1;
		pAd->CommonCfg.DesiredHtPhy.TxSTBC = 1;
		pAd->CommonCfg.HtCapability.HtCapInfo.RxSTBC = 1;
		pAd->CommonCfg.DesiredHtPhy.RxSTBC = 1;
	}
	else
	{
		pAd->CommonCfg.DesiredHtPhy.TxSTBC = 0;
		pAd->CommonCfg.DesiredHtPhy.RxSTBC = 0;
	}

	if(pHTPhyMode->SHORTGI == GI_400)
	{
		pAd->CommonCfg.HtCapability.HtCapInfo.ShortGIfor20 = 1;
		pAd->CommonCfg.HtCapability.HtCapInfo.ShortGIfor40 = 1;
		pAd->CommonCfg.DesiredHtPhy.ShortGIfor20 = 1;
		pAd->CommonCfg.DesiredHtPhy.ShortGIfor40 = 1;
	}
	else
	{
		pAd->CommonCfg.HtCapability.HtCapInfo.ShortGIfor20 = 0;
		pAd->CommonCfg.HtCapability.HtCapInfo.ShortGIfor40 = 0;
		pAd->CommonCfg.DesiredHtPhy.ShortGIfor20 = 0;
		pAd->CommonCfg.DesiredHtPhy.ShortGIfor40 = 0;
	}

	
	pAd->CommonCfg.HtCapability.ExtHtCapInfo.MCSFeedback = MCSFBK_NONE; 
	pAd->CommonCfg.AddHTInfo.ControlChan = pAd->CommonCfg.Channel;
	

	
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

		pAd->CommonCfg.APEdcaParm.Cwmax[0] = 6;
		pAd->CommonCfg.APEdcaParm.Cwmax[1] = 10;
		pAd->CommonCfg.APEdcaParm.Cwmax[2] = 4;
		pAd->CommonCfg.APEdcaParm.Cwmax[3] = 3;

		pAd->CommonCfg.APEdcaParm.Txop[0]  = 0;
		pAd->CommonCfg.APEdcaParm.Txop[1]  = 0;
		pAd->CommonCfg.APEdcaParm.Txop[2]  = 94;
		pAd->CommonCfg.APEdcaParm.Txop[3]  = 47;
	}
	AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm);


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
		RTMPSetIndividualHT(pAd, 0);
		}
#endif 

}


VOID	RTMPSetIndividualHT(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				apidx)
{
	PRT_HT_PHY_INFO		pDesired_ht_phy = NULL;
	UCHAR	TxStream = pAd->CommonCfg.TxStream;
	UCHAR	DesiredMcs	= MCS_AUTO;

	do
	{

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			pDesired_ht_phy = &pAd->StaCfg.DesiredHtPhyInfo;
			DesiredMcs = pAd->StaCfg.DesiredTransmitSetting.field.MCS;
			
				break;
		}
#endif 
	} while (FALSE);

	if (pDesired_ht_phy == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("RTMPSetIndividualHT: invalid apidx(%d)\n", apidx));
		return;
	}
	RTMPZeroMemory(pDesired_ht_phy, sizeof(RT_HT_PHY_INFO));

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPSetIndividualHT : Desired MCS = %d\n", DesiredMcs));
	
	if ((TxStream == 1) && ((DesiredMcs >= MCS_8) && (DesiredMcs <= MCS_15)))
	{
		DBGPRINT(RT_DEBUG_WARN, ("RTMPSetIndividualHT: MCS(%d) is invalid in 1S, reset it as MCS_7\n", DesiredMcs));
		DesiredMcs = MCS_7;
	}

	if ((pAd->CommonCfg.DesiredHtPhy.ChannelWidth == BW_20) && (DesiredMcs == MCS_32))
	{
		DBGPRINT(RT_DEBUG_WARN, ("RTMPSetIndividualHT: MCS_32 is only supported in 40-MHz, reset it as MCS_0\n"));
		DesiredMcs = MCS_0;
	}

	pDesired_ht_phy->bHtEnable = TRUE;

	
	switch (TxStream)
	{
		case 1:
			if (DesiredMcs == MCS_AUTO)
			{
				pDesired_ht_phy->MCSSet[0]= 0xff;
				pDesired_ht_phy->MCSSet[1]= 0x00;
			}
			else if (DesiredMcs <= MCS_7)
			{
				pDesired_ht_phy->MCSSet[0]= 1<<DesiredMcs;
				pDesired_ht_phy->MCSSet[1]= 0x00;
			}
			break;

		case 2:
			if (DesiredMcs == MCS_AUTO)
			{
				pDesired_ht_phy->MCSSet[0]= 0xff;
				pDesired_ht_phy->MCSSet[1]= 0xff;
			}
			else if (DesiredMcs <= MCS_15)
			{
				ULONG mode;

				mode = DesiredMcs / 8;
				if (mode < 2)
					pDesired_ht_phy->MCSSet[mode] = (1 << (DesiredMcs - mode * 8));
			}
			break;

		case 3: 
			if (DesiredMcs == MCS_AUTO)
			{
				
				pDesired_ht_phy->MCSSet[0]= 0xff;
				pDesired_ht_phy->MCSSet[1]= 0xff;
				pDesired_ht_phy->MCSSet[2]= 0xff;
			}
			else if (DesiredMcs <= MCS_23)
			{
				ULONG mode;

				mode = DesiredMcs / 8;
				if (mode < 3)
					pDesired_ht_phy->MCSSet[mode] = (1 << (DesiredMcs - mode * 8));
			}
			break;
	}

	if(pAd->CommonCfg.DesiredHtPhy.ChannelWidth == BW_40)
	{
		if (DesiredMcs == MCS_AUTO || DesiredMcs == MCS_32)
			pDesired_ht_phy->MCSSet[4] = 0x1;
	}

	
    if (pAd->OpMode == OPMODE_STA)
        MlmeUpdateHtTxRates(pAd, BSS0);
    else
	    MlmeUpdateHtTxRates(pAd, apidx);
}



VOID	RTMPUpdateHTIE(
	IN	RT_HT_CAPABILITY	*pRtHt,
	IN		UCHAR				*pMcsSet,
	OUT		HT_CAPABILITY_IE *pHtCapability,
	OUT		ADD_HT_INFO_IE		*pAddHtInfo)
{
	RTMPZeroMemory(pHtCapability, sizeof(HT_CAPABILITY_IE));
	RTMPZeroMemory(pAddHtInfo, sizeof(ADD_HT_INFO_IE));

		pHtCapability->HtCapInfo.ChannelWidth = pRtHt->ChannelWidth;
		pHtCapability->HtCapInfo.MimoPs = pRtHt->MimoPs;
		pHtCapability->HtCapInfo.GF = pRtHt->GF;
		pHtCapability->HtCapInfo.ShortGIfor20 = pRtHt->ShortGIfor20;
		pHtCapability->HtCapInfo.ShortGIfor40 = pRtHt->ShortGIfor40;
		pHtCapability->HtCapInfo.TxSTBC = pRtHt->TxSTBC;
		pHtCapability->HtCapInfo.RxSTBC = pRtHt->RxSTBC;
		pHtCapability->HtCapInfo.AMsduSize = pRtHt->AmsduSize;
		pHtCapability->HtCapParm.MaxRAmpduFactor = pRtHt->MaxRAmpduFactor;
		pHtCapability->HtCapParm.MpduDensity = pRtHt->MpduDensity;

		pAddHtInfo->AddHtInfo.ExtChanOffset = pRtHt->ExtChanOffset ;
		pAddHtInfo->AddHtInfo.RecomWidth = pRtHt->RecomWidth;
		pAddHtInfo->AddHtInfo2.OperaionMode = pRtHt->OperaionMode;
		pAddHtInfo->AddHtInfo2.NonGfPresent = pRtHt->NonGfPresent;
		RTMPMoveMemory(pAddHtInfo->MCSSet, pMcsSet, 4); 

        DBGPRINT(RT_DEBUG_TRACE,("RTMPUpdateHTIE <== \n"));
}
#endif 


VOID	RTMPAddWcidAttributeEntry(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			BssIdx,
	IN	UCHAR			KeyIdx,
	IN	UCHAR			CipherAlg,
	IN	MAC_TABLE_ENTRY *pEntry)
{
	UINT32		WCIDAttri = 0;
	USHORT		offset;
	UCHAR		IVEIV = 0;
	USHORT		Wcid = 0;

	{
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			if (BssIdx > BSS0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("RTMPAddWcidAttributeEntry: The BSS-index(%d) is out of range for Infra link. \n", BssIdx));
				return;
			}

			
			
			
			if (pEntry)
				Wcid = pEntry->Aid;
			else
				Wcid = MCAST_WCID;
		}
#endif 
	}

	
	offset = MAC_WCID_ATTRIBUTE_BASE + (Wcid * HW_WCID_ATTRI_SIZE);

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (pEntry && pEntry->ValidAsMesh)
			WCIDAttri = (CipherAlg<<1) | PAIRWISEKEYTABLE;
#ifdef QOS_DLS_SUPPORT
		else if ((pEntry) && (pEntry->ValidAsDls) &&
					((CipherAlg == CIPHER_TKIP) ||
					(CipherAlg == CIPHER_TKIP_NO_MIC) ||
					(CipherAlg == CIPHER_AES) ||
					(CipherAlg == CIPHER_NONE)))
			WCIDAttri = (CipherAlg<<1) | PAIRWISEKEYTABLE;
#endif 
		else
		WCIDAttri = (CipherAlg<<1) | SHAREDKEYTABLE;
	}
#endif 

	RTMP_IO_WRITE32(pAd, offset, WCIDAttri);


	
	offset = MAC_IVEIV_TABLE_BASE + (Wcid * HW_IVEIV_ENTRY_SIZE);

	
	if ((CipherAlg == CIPHER_TKIP) || (CipherAlg == CIPHER_TKIP_NO_MIC) || (CipherAlg == CIPHER_AES))
	{
		
		IVEIV = (KeyIdx <<6) | 0x20;
	}
	else
	{
		
		IVEIV = (KeyIdx << 6);
	}

	
#ifdef RTMP_MAC_PCI
	RTMP_IO_WRITE8(pAd, offset+3, IVEIV);
#endif 

	DBGPRINT(RT_DEBUG_TRACE,("RTMPAddWcidAttributeEntry: WCID #%d, KeyIndex #%d, Alg=%s\n",Wcid, KeyIdx, CipherName[CipherAlg]));
	DBGPRINT(RT_DEBUG_TRACE,("	WCIDAttri = 0x%x \n",  WCIDAttri));

}


PSTRING GetEncryptType(CHAR enc)
{
    if(enc == Ndis802_11WEPDisabled)
        return "NONE";
    if(enc == Ndis802_11WEPEnabled)
	return "WEP";
    if(enc == Ndis802_11Encryption2Enabled)
	return "TKIP";
    if(enc == Ndis802_11Encryption3Enabled)
	return "AES";
	if(enc == Ndis802_11Encryption4Enabled)
	return "TKIPAES";
    else
	return "UNKNOW";
}

PSTRING GetAuthMode(CHAR auth)
{
    if(auth == Ndis802_11AuthModeOpen)
	return "OPEN";
    if(auth == Ndis802_11AuthModeShared)
	return "SHARED";
	if(auth == Ndis802_11AuthModeAutoSwitch)
	return "AUTOWEP";
    if(auth == Ndis802_11AuthModeWPA)
	return "WPA";
    if(auth == Ndis802_11AuthModeWPAPSK)
	return "WPAPSK";
    if(auth == Ndis802_11AuthModeWPANone)
	return "WPANONE";
    if(auth == Ndis802_11AuthModeWPA2)
	return "WPA2";
    if(auth == Ndis802_11AuthModeWPA2PSK)
	return "WPA2PSK";
	if(auth == Ndis802_11AuthModeWPA1WPA2)
	return "WPA1WPA2";
	if(auth == Ndis802_11AuthModeWPA1PSKWPA2PSK)
	return "WPA1PSKWPA2PSK";

	return "UNKNOW";
}



#define	LINE_LEN	(4+33+20+23+9+7+3)	
#ifdef CONFIG_STA_SUPPORT
#endif 
VOID	RTMPCommSiteSurveyData(
	IN  PSTRING		msg,
	IN  PBSS_ENTRY	pBss)
{
	INT         Rssi = 0;
	UINT        Rssi_Quality = 0;
	NDIS_802_11_NETWORK_TYPE    wireless_mode;
	CHAR		Ssid[MAX_LEN_OF_SSID +1];
	STRING		SecurityStr[32] = {0};
	NDIS_802_11_ENCRYPTION_STATUS	ap_cipher = Ndis802_11EncryptionDisabled;
	NDIS_802_11_AUTHENTICATION_MODE	ap_auth_mode = Ndis802_11AuthModeOpen;

	memset(Ssid, 0 ,(MAX_LEN_OF_SSID +1));

		
		sprintf(msg+strlen(msg),"%-4d", pBss->Channel);
		
		memcpy(Ssid, pBss->Ssid, pBss->SsidLen);
		Ssid[pBss->SsidLen] = '\0';
		sprintf(msg+strlen(msg),"%-33s", Ssid);
		
		sprintf(msg+strlen(msg),"%02x:%02x:%02x:%02x:%02x:%02x   ",
			pBss->Bssid[0],
			pBss->Bssid[1],
			pBss->Bssid[2],
			pBss->Bssid[3],
			pBss->Bssid[4],
			pBss->Bssid[5]);

	
	if ((Ndis802_11AuthModeWPA <= pBss->AuthMode) &&
		(pBss->AuthMode <= Ndis802_11AuthModeWPA1PSKWPA2PSK))
	{
		if (pBss->AuthModeAux == Ndis802_11AuthModeWPANone)
		{
			ap_auth_mode = pBss->AuthMode;
			if (pBss->WPA.PairCipherAux == Ndis802_11WEPDisabled)
				ap_cipher = pBss->WPA.PairCipher;
			else
				ap_cipher = Ndis802_11Encryption4Enabled;
		}
		else if (pBss->AuthModeAux == Ndis802_11AuthModeOpen)
		{
			ap_auth_mode = pBss->AuthMode;
			if ((ap_auth_mode == Ndis802_11AuthModeWPA) ||
				(ap_auth_mode == Ndis802_11AuthModeWPAPSK))
			{
				if (pBss->WPA.PairCipherAux == Ndis802_11WEPDisabled)
					ap_cipher = pBss->WPA.PairCipher;
				else
					ap_cipher = Ndis802_11Encryption4Enabled;
			}
			else if ((ap_auth_mode == Ndis802_11AuthModeWPA2) ||
					 (ap_auth_mode == Ndis802_11AuthModeWPA2PSK))
			{
				if (pBss->WPA2.PairCipherAux == Ndis802_11WEPDisabled)
					ap_cipher = pBss->WPA2.PairCipher;
				else
					ap_cipher = Ndis802_11Encryption4Enabled;
			}
		}
		else if ((pBss->AuthMode == Ndis802_11AuthModeWPAPSK) ||
				 (pBss->AuthMode == Ndis802_11AuthModeWPA2PSK))
		{
			if ((pBss->AuthModeAux == Ndis802_11AuthModeWPAPSK) ||
				(pBss->AuthModeAux == Ndis802_11AuthModeWPA2PSK))
				ap_auth_mode = Ndis802_11AuthModeWPA1PSKWPA2PSK;
			else
				ap_auth_mode = pBss->AuthMode;

			if (pBss->WPA.PairCipher != pBss->WPA2.PairCipher)
				ap_cipher = Ndis802_11Encryption4Enabled;
			else if ((pBss->WPA.PairCipher == pBss->WPA2.PairCipher) &&
					 (pBss->WPA.PairCipherAux != pBss->WPA2.PairCipherAux))
				ap_cipher = Ndis802_11Encryption4Enabled;
			else if ((pBss->WPA.PairCipher == pBss->WPA2.PairCipher) &&
					 (pBss->WPA.PairCipherAux == pBss->WPA2.PairCipherAux) &&
					 (pBss->WPA.PairCipherAux != Ndis802_11WEPDisabled))
				ap_cipher = Ndis802_11Encryption4Enabled;
			else if ((pBss->WPA.PairCipher == pBss->WPA2.PairCipher) &&
					 (pBss->WPA.PairCipherAux == pBss->WPA2.PairCipherAux) &&
					 (pBss->WPA.PairCipherAux == Ndis802_11WEPDisabled))
				ap_cipher = pBss->WPA.PairCipher;
		}
		else if ((pBss->AuthMode == Ndis802_11AuthModeWPA) ||
				 (pBss->AuthMode == Ndis802_11AuthModeWPA2))
		{
			if ((pBss->AuthModeAux == Ndis802_11AuthModeWPA) ||
				(pBss->AuthMode == Ndis802_11AuthModeWPA2))
				ap_auth_mode = Ndis802_11AuthModeWPA1WPA2;
			else
				ap_auth_mode = pBss->AuthMode;

			if (pBss->WPA.PairCipher != pBss->WPA2.PairCipher)
				ap_cipher = Ndis802_11Encryption4Enabled;
			else if ((pBss->WPA.PairCipher == pBss->WPA2.PairCipher) &&
					 (pBss->WPA.PairCipherAux != pBss->WPA2.PairCipherAux))
				ap_cipher = Ndis802_11Encryption4Enabled;
			else if ((pBss->WPA.PairCipher == pBss->WPA2.PairCipher) &&
					 (pBss->WPA.PairCipherAux == pBss->WPA2.PairCipherAux) &&
					 (pBss->WPA.PairCipherAux != Ndis802_11WEPDisabled))
				ap_cipher = Ndis802_11Encryption4Enabled;
			else if ((pBss->WPA.PairCipher == pBss->WPA2.PairCipher) &&
					 (pBss->WPA.PairCipherAux == pBss->WPA2.PairCipherAux) &&
					 (pBss->WPA.PairCipherAux == Ndis802_11WEPDisabled))
				ap_cipher = pBss->WPA.PairCipher;
		}

		sprintf(SecurityStr, "%s/%s", GetAuthMode((CHAR)ap_auth_mode), GetEncryptType((CHAR)ap_cipher));
	}
	else
	{
		ap_auth_mode = pBss->AuthMode;
		ap_cipher = pBss->WepStatus;
		if (ap_cipher == Ndis802_11WEPDisabled)
			sprintf(SecurityStr, "NONE");
		else if (ap_cipher == Ndis802_11WEPEnabled)
			sprintf(SecurityStr, "WEP");
		else
			sprintf(SecurityStr, "%s/%s", GetAuthMode((CHAR)ap_auth_mode), GetEncryptType((CHAR)ap_cipher));
	}

	sprintf(msg+strlen(msg), "%-23s", SecurityStr);

		
		Rssi = (INT)pBss->Rssi;
		if (Rssi >= -50)
			Rssi_Quality = 100;
		else if (Rssi >= -80)    
			Rssi_Quality = (UINT)(24 + ((Rssi + 80) * 26)/10);
		else if (Rssi >= -90)   
			Rssi_Quality = (UINT)(((Rssi + 90) * 26)/10);
		else    
			Rssi_Quality = 0;
		sprintf(msg+strlen(msg),"%-9d", Rssi_Quality);
		
		wireless_mode = NetworkTypeInUseSanity(pBss);
		if (wireless_mode == Ndis802_11FH ||
			wireless_mode == Ndis802_11DS)
			sprintf(msg+strlen(msg),"%-7s", "11b");
		else if (wireless_mode == Ndis802_11OFDM5)
			sprintf(msg+strlen(msg),"%-7s", "11a");
		else if (wireless_mode == Ndis802_11OFDM5_N)
			sprintf(msg+strlen(msg),"%-7s", "11a/n");
		else if (wireless_mode == Ndis802_11OFDM24)
			sprintf(msg+strlen(msg),"%-7s", "11b/g");
		else if (wireless_mode == Ndis802_11OFDM24_N)
			sprintf(msg+strlen(msg),"%-7s", "11b/g/n");
		else
			sprintf(msg+strlen(msg),"%-7s", "unknow");
		
		if (pBss->BssType == BSS_ADHOC)
			sprintf(msg+strlen(msg),"%-3s", " Ad");
		else
			sprintf(msg+strlen(msg),"%-3s", " In");

        sprintf(msg+strlen(msg),"\n");

	return;
}

VOID RTMPIoctlGetSiteSurvey(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq)
{
	PSTRING		msg;
	INT		i=0;
	INT			WaitCnt;
	INT		Status=0;
    INT         max_len = LINE_LEN;
	PBSS_ENTRY	pBss;

#ifdef CONFIG_STA_SUPPORT
#endif 

	os_alloc_mem(NULL, (PUCHAR *)&msg, sizeof(CHAR)*((MAX_LEN_OF_BSS_TABLE)*max_len));

	if (msg == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("RTMPIoctlGetSiteSurvey - msg memory alloc fail.\n"));
		return;
	}

	memset(msg, 0 ,(MAX_LEN_OF_BSS_TABLE)*max_len );
	sprintf(msg,"%s","\n");
	sprintf(msg+strlen(msg),"%-4s%-33s%-20s%-23s%-9s%-7s%-3s\n",
	    "Ch", "SSID", "BSSID", "Security", "Siganl(%)", "W-Mode", " NT");

#ifdef CONFIG_STA_SUPPORT

#endif 

	WaitCnt = 0;
#ifdef CONFIG_STA_SUPPORT
	pAdapter->StaCfg.bScanReqIsFromWebUI = TRUE;
	while ((ScanRunning(pAdapter) == TRUE) && (WaitCnt++ < 200))
		OS_WAIT(500);
#endif 

	for(i=0; i<pAdapter->ScanTab.BssNr ;i++)
	{
		pBss = &pAdapter->ScanTab.BssEntry[i];

		if( pBss->Channel==0)
			break;

		if((strlen(msg)+max_len ) >= IW_SCAN_MAX_DATA)
			break;


		RTMPCommSiteSurveyData(msg, pBss);

#ifdef CONFIG_STA_SUPPORT

#endif 
	}

#ifdef CONFIG_STA_SUPPORT
	pAdapter->StaCfg.bScanReqIsFromWebUI = FALSE;
#endif 
	wrq->u.data.length = strlen(msg);
	Status = copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length);

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPIoctlGetSiteSurvey - wrq->u.data.length = %d\n", wrq->u.data.length));
	os_free_mem(NULL, (PUCHAR)msg);
}

#define	MAC_LINE_LEN	(14+4+4+10+10+10+6+6)	
VOID RTMPIoctlGetMacTable(
	IN PRTMP_ADAPTER pAd,
	IN struct iwreq *wrq)
{
	INT i;
	RT_802_11_MAC_TABLE MacTab;
	char *msg;

	MacTab.Num = 0;
	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		if (pAd->MacTab.Content[i].ValidAsCLI && (pAd->MacTab.Content[i].Sst == SST_ASSOC))
		{
			COPY_MAC_ADDR(MacTab.Entry[MacTab.Num].Addr, &pAd->MacTab.Content[i].Addr);
			MacTab.Entry[MacTab.Num].Aid = (UCHAR)pAd->MacTab.Content[i].Aid;
			MacTab.Entry[MacTab.Num].Psm = pAd->MacTab.Content[i].PsMode;
#ifdef DOT11_N_SUPPORT
			MacTab.Entry[MacTab.Num].MimoPs = pAd->MacTab.Content[i].MmpsMode;
#endif 

			
			MacTab.Entry[MacTab.Num].AvgRssi0 = pAd->MacTab.Content[i].RssiSample.AvgRssi0;
			MacTab.Entry[MacTab.Num].AvgRssi1 = pAd->MacTab.Content[i].RssiSample.AvgRssi1;
			MacTab.Entry[MacTab.Num].AvgRssi2 = pAd->MacTab.Content[i].RssiSample.AvgRssi2;

			
			MacTab.Entry[MacTab.Num].ConnectedTime = pAd->MacTab.Content[i].StaConnectTime;
			MacTab.Entry[MacTab.Num].TxRate.field.MCS = pAd->MacTab.Content[i].HTPhyMode.field.MCS;
			MacTab.Entry[MacTab.Num].TxRate.field.BW = pAd->MacTab.Content[i].HTPhyMode.field.BW;
			MacTab.Entry[MacTab.Num].TxRate.field.ShortGI = pAd->MacTab.Content[i].HTPhyMode.field.ShortGI;
			MacTab.Entry[MacTab.Num].TxRate.field.STBC = pAd->MacTab.Content[i].HTPhyMode.field.STBC;
			MacTab.Entry[MacTab.Num].TxRate.field.rsv = pAd->MacTab.Content[i].HTPhyMode.field.rsv;
			MacTab.Entry[MacTab.Num].TxRate.field.MODE = pAd->MacTab.Content[i].HTPhyMode.field.MODE;
			MacTab.Entry[MacTab.Num].TxRate.word = pAd->MacTab.Content[i].HTPhyMode.word;

			MacTab.Num += 1;
		}
	}
	wrq->u.data.length = sizeof(RT_802_11_MAC_TABLE);
	if (copy_to_user(wrq->u.data.pointer, &MacTab, wrq->u.data.length))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));
	}

	msg = kmalloc(sizeof(CHAR)*(MAX_LEN_OF_MAC_TABLE*MAC_LINE_LEN), MEM_ALLOC_FLAG);
	if (msg == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Alloc memory failed\n", __FUNCTION__));
		return;
	}
	memset(msg, 0 ,MAX_LEN_OF_MAC_TABLE*MAC_LINE_LEN );
	sprintf(msg,"%s","\n");
	sprintf(msg+strlen(msg),"%-14s%-4s%-4s%-10s%-10s%-10s%-6s%-6s\n",
		"MAC", "AID", "PSM", "LDT", "RxB", "TxB","CTxR", "LTxR");

	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (pEntry->ValidAsCLI && (pEntry->Sst == SST_ASSOC))
		{
			if((strlen(msg)+MAC_LINE_LEN ) >= (MAX_LEN_OF_MAC_TABLE*MAC_LINE_LEN) )
				break;
			sprintf(msg+strlen(msg),"%02x%02x%02x%02x%02x%02x  ",
				pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
				pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]);
			sprintf(msg+strlen(msg),"%-4d", (int)pEntry->Aid);
			sprintf(msg+strlen(msg),"%-4d", (int)pEntry->PsMode);
			sprintf(msg+strlen(msg),"%-10d",0); 
			sprintf(msg+strlen(msg),"%-10d",0); 
			sprintf(msg+strlen(msg),"%-10d",0); 
			sprintf(msg+strlen(msg),"%-6d",RateIdToMbps[pAd->MacTab.Content[i].CurrTxRate]);
			sprintf(msg+strlen(msg),"%-6d\n",0); 
		}
	}
	
	
	
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s", msg));
	}

	kfree(msg);
}


#ifdef DOT11_N_SUPPORT
INT	Set_BASetup_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
    UCHAR mac[6], tid;
	PSTRING token;
	STRING sepValue[] = ":", DASH = '-';
	INT i;
    MAC_TABLE_ENTRY *pEntry;


	

	if(strlen(arg) < 19)  
		return FALSE;

	token = strchr(arg, DASH);
	if ((token != NULL) && (strlen(token)>1))
	{
		tid = (UCHAR) simple_strtol((token+1), 0, 10);
		if (tid > 15)
			return FALSE;

		*token = '\0';
		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
		{
			if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
				return FALSE;
			AtoH(token, (&mac[i]), 1);
		}
		if(i != 6)
			return FALSE;

		DBGPRINT(RT_DEBUG_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x\n",
								mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], tid));

	    pEntry = MacTableLookup(pAd, (PUCHAR) mac);

	if (pEntry) {
		DBGPRINT(RT_DEBUG_OFF, ("\nSetup BA Session: Tid = %d\n", tid));
	        BAOriSessionSetUp(pAd, pEntry, tid, 0, 100, TRUE);
	}

		return TRUE;
	}

	return FALSE;

}

INT	Set_BADecline_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG bBADecline;

	bBADecline = simple_strtol(arg, 0, 10);

	if (bBADecline == 0)
	{
		pAd->CommonCfg.bBADecline = FALSE;
	}
	else if (bBADecline == 1)
	{
		pAd->CommonCfg.bBADecline = TRUE;
	}
	else
	{
		return FALSE; 
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_BADecline_Proc::(BADecline=%d)\n", pAd->CommonCfg.bBADecline));

	return TRUE;
}

INT	Set_BAOriTearDown_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
    UCHAR mac[6], tid;
	PSTRING token;
	STRING sepValue[] = ":", DASH = '-';
	INT i;
    MAC_TABLE_ENTRY *pEntry;

    

    if(strlen(arg) < 19)  
		return FALSE;

	token = strchr(arg, DASH);
	if ((token != NULL) && (strlen(token)>1))
	{
		tid = simple_strtol((token+1), 0, 10);
		if (tid > NUM_OF_TID)
			return FALSE;

		*token = '\0';
		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
		{
			if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
				return FALSE;
			AtoH(token, (&mac[i]), 1);
		}
		if(i != 6)
			return FALSE;

	    DBGPRINT(RT_DEBUG_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x",
								mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], tid));

	    pEntry = MacTableLookup(pAd, (PUCHAR) mac);

	    if (pEntry) {
	        DBGPRINT(RT_DEBUG_OFF, ("\nTear down Ori BA Session: Tid = %d\n", tid));
	        BAOriSessionTearDown(pAd, pEntry->Aid, tid, FALSE, TRUE);
	    }

		return TRUE;
	}

	return FALSE;

}

INT	Set_BARecTearDown_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
    UCHAR mac[6], tid;
	PSTRING token;
	STRING sepValue[] = ":", DASH = '-';
	INT i;
    MAC_TABLE_ENTRY *pEntry;

    

    if(strlen(arg) < 19)  
		return FALSE;

	token = strchr(arg, DASH);
	if ((token != NULL) && (strlen(token)>1))
	{
		tid = simple_strtol((token+1), 0, 10);
		if (tid > NUM_OF_TID)
			return FALSE;

		*token = '\0';
		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
		{
			if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
				return FALSE;
			AtoH(token, (&mac[i]), 1);
		}
		if(i != 6)
			return FALSE;

		DBGPRINT(RT_DEBUG_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x",
								mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], tid));

		pEntry = MacTableLookup(pAd, (PUCHAR) mac);

		if (pEntry) {
		    DBGPRINT(RT_DEBUG_OFF, ("\nTear down Rec BA Session: Tid = %d\n", tid));
		    BARecSessionTearDown(pAd, pEntry->Aid, tid, FALSE);
		}

		return TRUE;
	}

	return FALSE;

}

INT	Set_HtBw_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG HtBw;

	HtBw = simple_strtol(arg, 0, 10);
	if (HtBw == BW_40)
		pAd->CommonCfg.RegTransmitSetting.field.BW  = BW_40;
	else if (HtBw == BW_20)
		pAd->CommonCfg.RegTransmitSetting.field.BW  = BW_20;
	else
		return FALSE;  

	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtBw_Proc::(HtBw=%d)\n", pAd->CommonCfg.RegTransmitSetting.field.BW));

	return TRUE;
}

INT	Set_HtMcs_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG HtMcs, Mcs_tmp;
#ifdef CONFIG_STA_SUPPORT
    BOOLEAN bAutoRate = FALSE;
#endif 

	Mcs_tmp = simple_strtol(arg, 0, 10);

	if (Mcs_tmp <= 15 || Mcs_tmp == 32)
		HtMcs = Mcs_tmp;
	else
		HtMcs = MCS_AUTO;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		pAd->StaCfg.DesiredTransmitSetting.field.MCS = HtMcs;
		pAd->StaCfg.bAutoTxRateSwitch = (HtMcs == MCS_AUTO) ? TRUE:FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("Set_HtMcs_Proc::(HtMcs=%d, bAutoTxRateSwitch = %d)\n",
						pAd->StaCfg.DesiredTransmitSetting.field.MCS, pAd->StaCfg.bAutoTxRateSwitch));

		if ((pAd->CommonCfg.PhyMode < PHY_11ABGN_MIXED) ||
			(pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE < MODE_HTMIX))
		{
	        if ((pAd->StaCfg.DesiredTransmitSetting.field.MCS != MCS_AUTO) &&
				(HtMcs >= 0 && HtMcs <= 3) &&
	            (pAd->StaCfg.DesiredTransmitSetting.field.FixedTxMode == FIXED_TXMODE_CCK))
			{
				RTMPSetDesiredRates(pAd, (LONG) (RateIdToMbps[HtMcs] * 1000000));
			}
	        else if ((pAd->StaCfg.DesiredTransmitSetting.field.MCS != MCS_AUTO) &&
					(HtMcs >= 0 && HtMcs <= 7) &&
			(pAd->StaCfg.DesiredTransmitSetting.field.FixedTxMode == FIXED_TXMODE_OFDM))
			{
				RTMPSetDesiredRates(pAd, (LONG) (RateIdToMbps[HtMcs+4] * 1000000));
			}
			else
				bAutoRate = TRUE;

			if (bAutoRate)
			{
	            pAd->StaCfg.DesiredTransmitSetting.field.MCS = MCS_AUTO;
				RTMPSetDesiredRates(pAd, -1);
			}
	        DBGPRINT(RT_DEBUG_TRACE, ("Set_HtMcs_Proc::(FixedTxMode=%d)\n",pAd->StaCfg.DesiredTransmitSetting.field.FixedTxMode));
		}
        if (ADHOC_ON(pAd))
            return TRUE;
	}
#endif 

	SetCommonHT(pAd);

	return TRUE;
}

INT	Set_HtGi_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG HtGi;

	HtGi = simple_strtol(arg, 0, 10);

	if ( HtGi == GI_400)
		pAd->CommonCfg.RegTransmitSetting.field.ShortGI = GI_400;
	else if ( HtGi == GI_800 )
		pAd->CommonCfg.RegTransmitSetting.field.ShortGI = GI_800;
	else
		return FALSE; 

	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtGi_Proc::(ShortGI=%d)\n",pAd->CommonCfg.RegTransmitSetting.field.ShortGI));

	return TRUE;
}


INT	Set_HtTxBASize_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UCHAR Size;

	Size = simple_strtol(arg, 0, 10);

	if (Size <=0 || Size >=64)
	{
		Size = 8;
	}
	pAd->CommonCfg.TxBASize = Size-1;
	DBGPRINT(RT_DEBUG_ERROR, ("Set_HtTxBASize ::(TxBASize= %d)\n", Size));

	return TRUE;
}

INT	Set_HtDisallowTKIP_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);

	if (Value == 1)
	{
		pAd->CommonCfg.HT_DisallowTKIP = TRUE;
	}
	else
	{
		pAd->CommonCfg.HT_DisallowTKIP = FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtDisallowTKIP_Proc ::%s\n",
				(pAd->CommonCfg.HT_DisallowTKIP == TRUE) ? "enabled" : "disabled"));

	return TRUE;
}

INT	Set_HtOpMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{

	ULONG Value;

	Value = simple_strtol(arg, 0, 10);

	if (Value == HTMODE_GF)
		pAd->CommonCfg.RegTransmitSetting.field.HTMODE  = HTMODE_GF;
	else if ( Value == HTMODE_MM )
		pAd->CommonCfg.RegTransmitSetting.field.HTMODE  = HTMODE_MM;
	else
		return FALSE; 

	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtOpMode_Proc::(HtOpMode=%d)\n",pAd->CommonCfg.RegTransmitSetting.field.HTMODE));

	return TRUE;

}

INT	Set_HtStbc_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{

	ULONG Value;

	Value = simple_strtol(arg, 0, 10);

	if (Value == STBC_USE)
		pAd->CommonCfg.RegTransmitSetting.field.STBC = STBC_USE;
	else if ( Value == STBC_NONE )
		pAd->CommonCfg.RegTransmitSetting.field.STBC = STBC_NONE;
	else
		return FALSE; 

	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_Stbc_Proc::(HtStbc=%d)\n",pAd->CommonCfg.RegTransmitSetting.field.STBC));

	return TRUE;
}

INT	Set_HtHtc_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{

	ULONG Value;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->HTCEnable = FALSE;
	else if ( Value ==1 )
        pAd->HTCEnable = TRUE;
	else
		return FALSE; 

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtHtc_Proc::(HtHtc=%d)\n",pAd->HTCEnable));

	return TRUE;
}

INT	Set_HtExtcha_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{

	ULONG Value;

	Value = simple_strtol(arg, 0, 10);

	if (Value == 0)
		pAd->CommonCfg.RegTransmitSetting.field.EXTCHA  = EXTCHA_BELOW;
	else if ( Value ==1 )
        pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_ABOVE;
	else
		return FALSE; 

	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtExtcha_Proc::(HtExtcha=%d)\n",pAd->CommonCfg.RegTransmitSetting.field.EXTCHA));

	return TRUE;
}

INT	Set_HtMpduDensity_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);

	if (Value <=7 && Value >= 0)
		pAd->CommonCfg.BACapability.field.MpduDensity = Value;
	else
		pAd->CommonCfg.BACapability.field.MpduDensity = 4;

	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtMpduDensity_Proc::(HtMpduDensity=%d)\n",pAd->CommonCfg.BACapability.field.MpduDensity));

	return TRUE;
}

INT	Set_HtBaWinSize_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);


	if (Value >=1 && Value <= 64)
	{
		pAd->CommonCfg.REGBACapability.field.RxBAWinLimit = Value;
		pAd->CommonCfg.BACapability.field.RxBAWinLimit = Value;
	}
	else
	{
        pAd->CommonCfg.REGBACapability.field.RxBAWinLimit = 64;
		pAd->CommonCfg.BACapability.field.RxBAWinLimit = 64;
	}

	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtBaWinSize_Proc::(HtBaWinSize=%d)\n",pAd->CommonCfg.BACapability.field.RxBAWinLimit));

	return TRUE;
}

INT	Set_HtRdg_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);

	if (Value == 0)
		pAd->CommonCfg.bRdg = FALSE;
	else if ( Value ==1 )
	{
		pAd->HTCEnable = TRUE;
        pAd->CommonCfg.bRdg = TRUE;
	}
	else
		return FALSE; 

	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtRdg_Proc::(HtRdg=%d)\n",pAd->CommonCfg.bRdg));

	return TRUE;
}

INT	Set_HtLinkAdapt_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->bLinkAdapt = FALSE;
	else if ( Value ==1 )
	{
			pAd->HTCEnable = TRUE;
			pAd->bLinkAdapt = TRUE;
	}
	else
		return FALSE; 

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtLinkAdapt_Proc::(HtLinkAdapt=%d)\n",pAd->bLinkAdapt));

	return TRUE;
}

INT	Set_HtAmsdu_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->CommonCfg.BACapability.field.AmsduEnable = FALSE;
	else if ( Value == 1 )
        pAd->CommonCfg.BACapability.field.AmsduEnable = TRUE;
	else
		return FALSE; 

	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtAmsdu_Proc::(HtAmsdu=%d)\n",pAd->CommonCfg.BACapability.field.AmsduEnable));

	return TRUE;
}

INT	Set_HtAutoBa_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
	{
		pAd->CommonCfg.BACapability.field.AutoBA = FALSE;
		pAd->CommonCfg.BACapability.field.Policy = BA_NOTUSE;
	}
    else if (Value == 1)
    {
		pAd->CommonCfg.BACapability.field.AutoBA = TRUE;
		pAd->CommonCfg.BACapability.field.Policy = IMMED_BA;
    }
	else
		return FALSE; 

    pAd->CommonCfg.REGBACapability.field.AutoBA = pAd->CommonCfg.BACapability.field.AutoBA;
	pAd->CommonCfg.REGBACapability.field.Policy = pAd->CommonCfg.BACapability.field.Policy;
	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtAutoBa_Proc::(HtAutoBa=%d)\n",pAd->CommonCfg.BACapability.field.AutoBA));

	return TRUE;

}

INT	Set_HtProtect_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->CommonCfg.bHTProtect = FALSE;
    else if (Value == 1)
		pAd->CommonCfg.bHTProtect = TRUE;
	else
		return FALSE; 

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtProtect_Proc::(HtProtect=%d)\n",pAd->CommonCfg.bHTProtect));

	return TRUE;
}

INT	Set_SendPSMPAction_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
    UCHAR mac[6], mode;
	PSTRING token;
	STRING sepValue[] = ":", DASH = '-';
	INT i;
    MAC_TABLE_ENTRY *pEntry;

    

    if(strlen(arg) < 19)  
		return FALSE;

	token = strchr(arg, DASH);
	if ((token != NULL) && (strlen(token)>1))
	{
		mode = simple_strtol((token+1), 0, 10);
		if (mode > MMPS_ENABLE)
			return FALSE;

		*token = '\0';
		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
		{
			if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
				return FALSE;
			AtoH(token, (&mac[i]), 1);
		}
		if(i != 6)
			return FALSE;

		DBGPRINT(RT_DEBUG_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x",
								mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mode));

		pEntry = MacTableLookup(pAd, mac);

		if (pEntry) {
		    DBGPRINT(RT_DEBUG_OFF, ("\nSendPSMPAction MIPS mode = %d\n", mode));
		    SendPSMPAction(pAd, pEntry->Aid, mode);
		}

		return TRUE;
	}

	return FALSE;


}

INT	Set_HtMIMOPSmode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);

	if (Value <=3 && Value >= 0)
		pAd->CommonCfg.BACapability.field.MMPSmode = Value;
	else
		pAd->CommonCfg.BACapability.field.MMPSmode = 3;

	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtMIMOPSmode_Proc::(MIMOPS mode=%d)\n",pAd->CommonCfg.BACapability.field.MMPSmode));

	return TRUE;
}


INT	Set_ForceShortGI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->WIFItestbed.bShortGI = FALSE;
	else if (Value == 1)
		pAd->WIFItestbed.bShortGI = TRUE;
	else
		return FALSE; 

	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ForceShortGI_Proc::(ForceShortGI=%d)\n", pAd->WIFItestbed.bShortGI));

	return TRUE;
}



INT	Set_ForceGF_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->WIFItestbed.bGreenField = FALSE;
	else if (Value == 1)
		pAd->WIFItestbed.bGreenField = TRUE;
	else
		return FALSE; 

	SetCommonHT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ForceGF_Proc::(ForceGF=%d)\n", pAd->WIFItestbed.bGreenField));

	return TRUE;
}

INT	Set_HtMimoPs_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);
	if (Value == 0)
		pAd->CommonCfg.bMIMOPSEnable = FALSE;
	else if (Value == 1)
		pAd->CommonCfg.bMIMOPSEnable = TRUE;
	else
		return FALSE; 

	DBGPRINT(RT_DEBUG_TRACE, ("Set_HtMimoPs_Proc::(HtMimoPs=%d)\n",pAd->CommonCfg.bMIMOPSEnable));

	return TRUE;
}
#endif 


#ifdef DOT11_N_SUPPORT
INT	SetCommonHT(
	IN	PRTMP_ADAPTER	pAd)
{
	OID_SET_HT_PHYMODE		SetHT;

	if (pAd->CommonCfg.PhyMode < PHY_11ABGN_MIXED)
		return FALSE;

	SetHT.PhyMode = pAd->CommonCfg.PhyMode;
	SetHT.TransmitNo = ((UCHAR)pAd->Antenna.field.TxPath);
	SetHT.HtMode = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.HTMODE;
	SetHT.ExtOffset = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.EXTCHA;
	SetHT.MCS = MCS_AUTO;
	SetHT.BW = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.BW;
	SetHT.STBC = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.STBC;
	SetHT.SHORTGI = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.ShortGI;

	RTMPSetHT(pAd, &SetHT);

	return TRUE;
}
#endif 

INT	Set_FixedTxMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UCHAR	fix_tx_mode = FIXED_TXMODE_HT;

	if (strcmp(arg, "OFDM") == 0 || strcmp(arg, "ofdm") == 0)
	{
		fix_tx_mode = FIXED_TXMODE_OFDM;
	}
	else if (strcmp(arg, "CCK") == 0 || strcmp(arg, "cck") == 0)
	{
        fix_tx_mode = FIXED_TXMODE_CCK;
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pAd->StaCfg.DesiredTransmitSetting.field.FixedTxMode = fix_tx_mode;
#endif 

	DBGPRINT(RT_DEBUG_TRACE, ("Set_FixedTxMode_Proc::(FixedTxMode=%d)\n", fix_tx_mode));

	return TRUE;
}

#ifdef CONFIG_APSTA_MIXED_SUPPORT
INT	Set_OpMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ULONG Value;

	Value = simple_strtol(arg, 0, 10);

#ifdef RTMP_MAC_PCI
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
#endif 
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Can not switch operate mode on interface up !! \n"));
		return FALSE;
	}

	if (Value == 0)
		pAd->OpMode = OPMODE_STA;
	else if (Value == 1)
		pAd->OpMode = OPMODE_AP;
	else
		return FALSE; 

	DBGPRINT(RT_DEBUG_TRACE, ("Set_OpMode_Proc::(OpMode=%s)\n", pAd->OpMode == 1 ? "AP Mode" : "STA Mode"));

	return TRUE;
}
#endif 



INT Set_LongRetryLimit_Proc(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	PSTRING			arg)
{
	TX_RTY_CFG_STRUC	tx_rty_cfg;
	UCHAR				LongRetryLimit = (UCHAR)simple_strtol(arg, 0, 10);

	RTMP_IO_READ32(pAdapter, TX_RTY_CFG, &tx_rty_cfg.word);
	tx_rty_cfg.field.LongRtyLimit = LongRetryLimit;
	RTMP_IO_WRITE32(pAdapter, TX_RTY_CFG, tx_rty_cfg.word);
	DBGPRINT(RT_DEBUG_TRACE, ("IF Set_LongRetryLimit_Proc::(tx_rty_cfg=0x%x)\n", tx_rty_cfg.word));
	return TRUE;
}

INT Set_ShortRetryLimit_Proc(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	PSTRING			arg)
{
	TX_RTY_CFG_STRUC	tx_rty_cfg;
	UCHAR				ShortRetryLimit = (UCHAR)simple_strtol(arg, 0, 10);

	RTMP_IO_READ32(pAdapter, TX_RTY_CFG, &tx_rty_cfg.word);
	tx_rty_cfg.field.ShortRtyLimit = ShortRetryLimit;
	RTMP_IO_WRITE32(pAdapter, TX_RTY_CFG, tx_rty_cfg.word);
	DBGPRINT(RT_DEBUG_TRACE, ("IF Set_ShortRetryLimit_Proc::(tx_rty_cfg=0x%x)\n", tx_rty_cfg.word));
	return TRUE;
}



PSTRING RTMPGetRalinkAuthModeStr(
    IN  NDIS_802_11_AUTHENTICATION_MODE authMode)
{
	switch(authMode)
	{
		case Ndis802_11AuthModeOpen:
			return "OPEN";
		case Ndis802_11AuthModeWPAPSK:
			return "WPAPSK";
		case Ndis802_11AuthModeShared:
			return "SHARED";
		case Ndis802_11AuthModeWPA:
			return "WPA";
		case Ndis802_11AuthModeWPA2:
			return "WPA2";
		case Ndis802_11AuthModeWPA2PSK:
			return "WPA2PSK";
        case Ndis802_11AuthModeWPA1PSKWPA2PSK:
			return "WPAPSKWPA2PSK";
        case Ndis802_11AuthModeWPA1WPA2:
			return "WPA1WPA2";
		case Ndis802_11AuthModeWPANone:
			return "WPANONE";
		default:
			return "UNKNOW";
	}
}

PSTRING RTMPGetRalinkEncryModeStr(
    IN  USHORT encryMode)
{
	switch(encryMode)
	{
		case Ndis802_11WEPDisabled:
			return "NONE";
		case Ndis802_11WEPEnabled:
			return "WEP";
		case Ndis802_11Encryption2Enabled:
			return "TKIP";
		case Ndis802_11Encryption3Enabled:
			return "AES";
        case Ndis802_11Encryption4Enabled:
			return "TKIPAES";
		default:
			return "UNKNOW";
	}
}

INT RTMPShowCfgValue(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			pName,
	IN	PSTRING			pBuf)
{
	INT	Status = 0;

	for (PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC = RTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC; PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name; PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC++)
	{
		if (!strcmp(pName, PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name))
		{
			if(PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->show_proc(pAd, pBuf))
				Status = -EINVAL;
			break;  
		}
	}

	if(PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name == NULL)
	{
		sprintf(pBuf, "\n");
		for (PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC = RTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC; PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name; PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC++)
			sprintf(pBuf, "%s%s\n", pBuf, PRTMP_PRIVATE_STA_SHOW_CFG_VALUE_PROC->name);
	}

	return Status;
}

INT	Show_SSID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		sprintf(pBuf, "\t%s", pAd->CommonCfg.Ssid);
#endif 
	return 0;
}

INT	Show_WirelessMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	switch(pAd->CommonCfg.PhyMode)
	{
		case PHY_11BG_MIXED:
			sprintf(pBuf, "\t11B/G");
			break;
		case PHY_11B:
			sprintf(pBuf, "\t11B");
			break;
		case PHY_11A:
			sprintf(pBuf, "\t11A");
			break;
		case PHY_11ABG_MIXED:
			sprintf(pBuf, "\t11A/B/G");
			break;
		case PHY_11G:
			sprintf(pBuf, "\t11G");
			break;
#ifdef DOT11_N_SUPPORT
		case PHY_11ABGN_MIXED:
			sprintf(pBuf, "\t11A/B/G/N");
			break;
		case PHY_11N_2_4G:
			sprintf(pBuf, "\t11N only with 2.4G");
			break;
		case PHY_11GN_MIXED:
			sprintf(pBuf, "\t11G/N");
			break;
		case PHY_11AN_MIXED:
			sprintf(pBuf, "\t11A/N");
			break;
		case PHY_11BGN_MIXED:
			sprintf(pBuf, "\t11B/G/N");
			break;
		case PHY_11AGN_MIXED:
			sprintf(pBuf, "\t11A/G/N");
			break;
		case PHY_11N_5G:
			sprintf(pBuf, "\t11N only with 5G");
			break;
#endif 
		default:
			sprintf(pBuf, "\tUnknow Value(%d)", pAd->CommonCfg.PhyMode);
			break;
	}
	return 0;
}


INT	Show_TxBurst_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	sprintf(pBuf, "\t%s", pAd->CommonCfg.bEnableTxBurst ? "TRUE":"FALSE");
	return 0;
}

INT	Show_TxPreamble_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	switch(pAd->CommonCfg.TxPreamble)
	{
		case Rt802_11PreambleShort:
			sprintf(pBuf, "\tShort");
			break;
		case Rt802_11PreambleLong:
			sprintf(pBuf, "\tLong");
			break;
		case Rt802_11PreambleAuto:
			sprintf(pBuf, "\tAuto");
			break;
		default:
			sprintf(pBuf, "\tUnknow Value(%lu)", pAd->CommonCfg.TxPreamble);
			break;
	}

	return 0;
}

INT	Show_TxPower_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	sprintf(pBuf, "\t%lu", pAd->CommonCfg.TxPowerPercentage);
	return 0;
}

INT	Show_Channel_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	sprintf(pBuf, "\t%d", pAd->CommonCfg.Channel);
	return 0;
}

INT	Show_BGProtection_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	switch(pAd->CommonCfg.UseBGProtection)
	{
		case 1: 
			sprintf(pBuf, "\tON");
			break;
		case 2: 
			sprintf(pBuf, "\tOFF");
			break;
		case 0: 
			sprintf(pBuf, "\tAuto");
			break;
		default:
			sprintf(pBuf, "\tUnknow Value(%lu)", pAd->CommonCfg.UseBGProtection);
			break;
	}
	return 0;
}

INT	Show_RTSThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	sprintf(pBuf, "\t%u", pAd->CommonCfg.RtsThreshold);
	return 0;
}

INT	Show_FragThreshold_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	sprintf(pBuf, "\t%u", pAd->CommonCfg.FragmentThreshold);
	return 0;
}

#ifdef DOT11_N_SUPPORT
INT	Show_HtBw_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	if (pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)
	{
		sprintf(pBuf, "\t40 MHz");
	}
	else
	{
        sprintf(pBuf, "\t20 MHz");
	}
	return 0;
}

INT	Show_HtMcs_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		sprintf(pBuf, "\t%u", pAd->StaCfg.DesiredTransmitSetting.field.MCS);
#endif 
	return 0;
}

INT	Show_HtGi_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	switch(pAd->CommonCfg.RegTransmitSetting.field.ShortGI)
	{
		case GI_400:
			sprintf(pBuf, "\tGI_400");
			break;
		case GI_800:
			sprintf(pBuf, "\tGI_800");
			break;
		default:
			sprintf(pBuf, "\tUnknow Value(%u)", pAd->CommonCfg.RegTransmitSetting.field.ShortGI);
			break;
	}
	return 0;
}

INT	Show_HtOpMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	switch(pAd->CommonCfg.RegTransmitSetting.field.HTMODE)
	{
		case HTMODE_GF:
			sprintf(pBuf, "\tGF");
			break;
		case HTMODE_MM:
			sprintf(pBuf, "\tMM");
			break;
		default:
			sprintf(pBuf, "\tUnknow Value(%u)", pAd->CommonCfg.RegTransmitSetting.field.HTMODE);
			break;
	}
	return 0;
}

INT	Show_HtExtcha_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	switch(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA)
	{
		case EXTCHA_BELOW:
			sprintf(pBuf, "\tBelow");
			break;
		case EXTCHA_ABOVE:
			sprintf(pBuf, "\tAbove");
			break;
		default:
			sprintf(pBuf, "\tUnknow Value(%u)", pAd->CommonCfg.RegTransmitSetting.field.EXTCHA);
			break;
	}
	return 0;
}


INT	Show_HtMpduDensity_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	sprintf(pBuf, "\t%u", pAd->CommonCfg.BACapability.field.MpduDensity);
	return 0;
}

INT	Show_HtBaWinSize_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	sprintf(pBuf, "\t%u", pAd->CommonCfg.BACapability.field.RxBAWinLimit);
	return 0;
}

INT	Show_HtRdg_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	sprintf(pBuf, "\t%s", pAd->CommonCfg.bRdg ? "TRUE":"FALSE");
	return 0;
}

INT	Show_HtAmsdu_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	sprintf(pBuf, "\t%s", pAd->CommonCfg.BACapability.field.AmsduEnable ? "TRUE":"FALSE");
	return 0;
}

INT	Show_HtAutoBa_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	sprintf(pBuf, "\t%s", pAd->CommonCfg.BACapability.field.AutoBA ? "TRUE":"FALSE");
	return 0;
}
#endif 

INT	Show_CountryRegion_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	sprintf(pBuf, "\t%d", pAd->CommonCfg.CountryRegion);
	return 0;
}

INT	Show_CountryRegionABand_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	sprintf(pBuf, "\t%d", pAd->CommonCfg.CountryRegionForABand);
	return 0;
}

INT	Show_CountryCode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	sprintf(pBuf, "\t%s", pAd->CommonCfg.CountryCode);
	return 0;
}

#ifdef AGGREGATION_SUPPORT
INT	Show_PktAggregate_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	sprintf(pBuf, "\t%s", pAd->CommonCfg.bAggregationCapable ? "TRUE":"FALSE");
	return 0;
}
#endif 

#ifdef WMM_SUPPORT
INT	Show_WmmCapable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		sprintf(pBuf, "\t%s", pAd->CommonCfg.bWmmCapable ? "TRUE":"FALSE");
#endif 

	return 0;
}
#endif 

INT	Show_IEEE80211H_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	sprintf(pBuf, "\t%s", pAd->CommonCfg.bIEEE80211H ? "TRUE":"FALSE");
	return 0;
}

#ifdef CONFIG_STA_SUPPORT
INT	Show_NetworkType_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	switch(pAd->StaCfg.BssType)
	{
		case BSS_ADHOC:
			sprintf(pBuf, "\tAdhoc");
			break;
		case BSS_INFRA:
			sprintf(pBuf, "\tInfra");
			break;
		case BSS_ANY:
			sprintf(pBuf, "\tAny");
			break;
		case BSS_MONITOR:
			sprintf(pBuf, "\tMonitor");
			break;
		default:
			sprintf(pBuf, "\tUnknow Value(%d)", pAd->StaCfg.BssType);
			break;
	}
	return 0;
}


#endif 

INT	Show_AuthMode_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	NDIS_802_11_AUTHENTICATION_MODE	AuthMode = Ndis802_11AuthModeOpen;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		AuthMode = pAd->StaCfg.AuthMode;
#endif 

	if ((AuthMode >= Ndis802_11AuthModeOpen) &&
		(AuthMode <= Ndis802_11AuthModeWPA1PSKWPA2PSK))
		sprintf(pBuf, "\t%s", RTMPGetRalinkAuthModeStr(AuthMode));
	else
		sprintf(pBuf, "\tUnknow Value(%d)", AuthMode);

	return 0;
}

INT	Show_EncrypType_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	NDIS_802_11_WEP_STATUS	WepStatus = Ndis802_11WEPDisabled;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		WepStatus = pAd->StaCfg.WepStatus;
#endif 

	if ((WepStatus >= Ndis802_11WEPEnabled) &&
		(WepStatus <= Ndis802_11Encryption4KeyAbsent))
		sprintf(pBuf, "\t%s", RTMPGetRalinkEncryModeStr(WepStatus));
	else
		sprintf(pBuf, "\tUnknow Value(%d)", WepStatus);

	return 0;
}

INT	Show_DefaultKeyID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	UCHAR DefaultKeyId = 0;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		DefaultKeyId = pAd->StaCfg.DefaultKeyId;
#endif 

	sprintf(pBuf, "\t%d", DefaultKeyId);

	return 0;
}

INT	Show_WepKey_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN  INT				KeyIdx,
	OUT	PSTRING			pBuf)
{
	UCHAR   Key[16] = {0}, KeyLength = 0;
	INT		index = BSS0;

	KeyLength = pAd->SharedKey[index][KeyIdx].KeyLen;
	NdisMoveMemory(Key, pAd->SharedKey[index][KeyIdx].Key, KeyLength);

	
    if (RTMPCheckStrPrintAble((PCHAR)Key, KeyLength))
        sprintf(pBuf, "\t%s", Key);
    else
    {
        int idx;
        sprintf(pBuf, "\t");
        for (idx = 0; idx < KeyLength; idx++)
            sprintf(pBuf+strlen(pBuf), "%02X", Key[idx]);
    }
	return 0;
}

INT	Show_Key1_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	Show_WepKey_Proc(pAd, 0, pBuf);
	return 0;
}

INT	Show_Key2_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	Show_WepKey_Proc(pAd, 1, pBuf);
	return 0;
}

INT	Show_Key3_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	Show_WepKey_Proc(pAd, 2, pBuf);
	return 0;
}

INT	Show_Key4_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	Show_WepKey_Proc(pAd, 3, pBuf);
	return 0;
}

INT	Show_WPAPSK_Proc(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PSTRING			pBuf)
{
	INT	idx;
	UCHAR	PMK[32] = {0};


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		NdisMoveMemory(PMK, pAd->StaCfg.PMK, 32);
#endif 

    sprintf(pBuf, "\tPMK = ");
    for (idx = 0; idx < 32; idx++)
        sprintf(pBuf+strlen(pBuf), "%02X", PMK[idx]);

	return 0;
}
