

#include	"rt_config.h"

#ifdef DBG
extern ULONG    RTDebugLevel;
#endif

#define NR_WEP_KEYS 				4
#define WEP_SMALL_KEY_LEN 			(40/8)
#define WEP_LARGE_KEY_LEN 			(104/8)

#define GROUP_KEY_NO                4

extern UCHAR    CipherWpa2Template[];
extern UCHAR    CipherWpaPskTkip[];
extern UCHAR    CipherWpaPskTkipLen;

typedef struct PACKED _RT_VERSION_INFO{
    UCHAR       DriverVersionW;
    UCHAR       DriverVersionX;
    UCHAR       DriverVersionY;
    UCHAR       DriverVersionZ;
    UINT        DriverBuildYear;
    UINT        DriverBuildMonth;
    UINT        DriverBuildDay;
} RT_VERSION_INFO, *PRT_VERSION_INFO;

struct iw_priv_args privtab[] = {
{ RTPRIV_IOCTL_SET,
  IW_PRIV_TYPE_CHAR | 1024, 0,
  "set"},

{ RTPRIV_IOCTL_SHOW, 0, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK,
  ""},
{ RTPRIV_IOCTL_SHOW, IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK,
  ""},

    { SHOW_CONN_STATUS,
	  0, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "connStatus" },
	{ SHOW_DRVIER_VERION,
	  0, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "driverVer" },
    { SHOW_BA_INFO,
	  0, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "bainfo" },
	{ SHOW_DESC_INFO,
	  0, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "descinfo" },
    { RAIO_OFF,
	  0, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "radio_off" },
	{ RAIO_ON,
	  0, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "radio_on" },
	{ SHOW_CFG_VALUE,
	  IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "show" },


{ RTPRIV_IOCTL_STATISTICS,
  0, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK,
  "stat"},
{ RTPRIV_IOCTL_GSITESURVEY,
  0, IW_PRIV_TYPE_CHAR | 1024,
  "get_site_survey"},
};

INT Set_SSID_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg);

#ifdef WMM_SUPPORT
INT	Set_WmmCapable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);
#endif

INT Set_NetworkType_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg);

INT Set_AuthMode_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg);

INT Set_EncrypType_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg);

INT Set_DefaultKeyID_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg);

INT Set_Key1_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg);

INT Set_Key2_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg);

INT Set_Key3_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg);

INT Set_Key4_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg);

INT Set_WPAPSK_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg);


INT Set_PSMode_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg);

INT Set_Wpa_Support(
    IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg);

NDIS_STATUS RTMPWPANoneAddKeyProc(
    IN  PRTMP_ADAPTER   pAd,
    IN	PVOID			pBuf);

INT Set_FragTest_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg);

INT Set_TGnWifiTest_Proc(
    IN  PRTMP_ADAPTER   pAd,
    IN  PUCHAR          arg);

INT Set_LongRetryLimit_Proc(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	PUCHAR			arg);

INT Set_ShortRetryLimit_Proc(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	PUCHAR			arg);

static struct {
	CHAR *name;
	INT (*set_proc)(PRTMP_ADAPTER pAdapter, PUCHAR arg);
} *PRTMP_PRIVATE_SET_PROC, RTMP_PRIVATE_SUPPORT_PROC[] = {
	{"DriverVersion",				Set_DriverVersion_Proc},
	{"CountryRegion",				Set_CountryRegion_Proc},
	{"CountryRegionABand",			Set_CountryRegionABand_Proc},
	{"SSID",						Set_SSID_Proc},
	{"WirelessMode",				Set_WirelessMode_Proc},
	{"TxBurst",					Set_TxBurst_Proc},
	{"TxPreamble",				Set_TxPreamble_Proc},
	{"TxPower",					Set_TxPower_Proc},
	{"Channel",					Set_Channel_Proc},
	{"BGProtection",				Set_BGProtection_Proc},
	{"RTSThreshold",				Set_RTSThreshold_Proc},
	{"FragThreshold",				Set_FragThreshold_Proc},
	{"HtBw",		                Set_HtBw_Proc},
	{"HtMcs",		                Set_HtMcs_Proc},
	{"HtGi",		                Set_HtGi_Proc},
	{"HtOpMode",		            Set_HtOpMode_Proc},
	{"HtExtcha",		            Set_HtExtcha_Proc},
	{"HtMpduDensity",		        Set_HtMpduDensity_Proc},
	{"HtBaWinSize",		        	Set_HtBaWinSize_Proc},
	{"HtRdg",		        		Set_HtRdg_Proc},
	{"HtAmsdu",		        		Set_HtAmsdu_Proc},
	{"HtAutoBa",		        	Set_HtAutoBa_Proc},
	{"HtBaDecline",					Set_BADecline_Proc},
	{"HtProtect",		        	Set_HtProtect_Proc},
	{"HtMimoPs",		        	Set_HtMimoPs_Proc},
#ifdef AGGREGATION_SUPPORT
	{"PktAggregate",				Set_PktAggregate_Proc},
#endif

#ifdef WMM_SUPPORT
	{"WmmCapable",					Set_WmmCapable_Proc},
#endif
	{"IEEE80211H",					Set_IEEE80211H_Proc},
    {"NetworkType",                 Set_NetworkType_Proc},
	{"AuthMode",					Set_AuthMode_Proc},
	{"EncrypType",					Set_EncrypType_Proc},
	{"DefaultKeyID",				Set_DefaultKeyID_Proc},
	{"Key1",						Set_Key1_Proc},
	{"Key2",						Set_Key2_Proc},
	{"Key3",						Set_Key3_Proc},
	{"Key4",						Set_Key4_Proc},
	{"WPAPSK",						Set_WPAPSK_Proc},
	{"ResetCounter",				Set_ResetStatCounter_Proc},
	{"PSMode",                      Set_PSMode_Proc},
#ifdef DBG
	{"Debug",						Set_Debug_Proc},
#endif
    {"WpaSupport",                  Set_Wpa_Support},
	{"FixedTxMode",                 Set_FixedTxMode_Proc},
    {"TGnWifiTest",                 Set_TGnWifiTest_Proc},
    {"ForceGF",		        		Set_ForceGF_Proc},
	{"LongRetry",	        		Set_LongRetryLimit_Proc},
	{"ShortRetry",	        		Set_ShortRetryLimit_Proc},
#ifdef RT2870
	{"efuseFreeNumber",				set_eFuseGetFreeBlockCount_Proc},
	{"efuseDump",					set_eFusedump_Proc},
	{"efuseLoadFromBin",				set_eFuseLoadFromBin_Proc},
#endif
	{NULL,}
};


VOID RTMPAddKey(
	IN	PRTMP_ADAPTER	    pAd,
	IN	PNDIS_802_11_KEY    pKey)
{
	ULONG				KeyIdx;
	MAC_TABLE_ENTRY  	*pEntry;

    DBGPRINT(RT_DEBUG_TRACE, ("RTMPAddKey ------>\n"));

#ifdef RT2860
	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP);
	if (RTMP_TEST_PSFLAG(pAd, fRTMP_PS_SET_PCI_CLK_OFF_COMMAND))
	{
		if (pAd->StaCfg.bRadio == FALSE)
		{
			RTMP_SET_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP);
			return;
		}
		DBGPRINT(RT_DEBUG_TRACE,("RTMPWPAAddKeyProc1==>\n"));
		RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_HALT);
		RTMPusecDelay(6000);
		pAd->bPCIclkOff = FALSE;
	}
#endif

	if (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
	{
		if (pKey->KeyIndex & 0x80000000)
		{
		    if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPANone)
            {
                NdisZeroMemory(pAd->StaCfg.PMK, 32);
                NdisMoveMemory(pAd->StaCfg.PMK, pKey->KeyMaterial, pKey->KeyLength);
                goto end;
            }
		    
		    NdisZeroMemory(&pAd->SharedKey[BSS0][0], sizeof(CIPHER_KEY));
            pAd->SharedKey[BSS0][0].KeyLen = LEN_TKIP_EK;
            NdisMoveMemory(pAd->SharedKey[BSS0][0].Key, pKey->KeyMaterial, LEN_TKIP_EK);

            if (pAd->StaCfg.PairCipher == Ndis802_11Encryption2Enabled)
            {
                NdisMoveMemory(pAd->SharedKey[BSS0][0].RxMic, pKey->KeyMaterial + LEN_TKIP_EK, LEN_TKIP_TXMICK);
                NdisMoveMemory(pAd->SharedKey[BSS0][0].TxMic, pKey->KeyMaterial + LEN_TKIP_EK + LEN_TKIP_TXMICK, LEN_TKIP_RXMICK);
            }
            else
            {
            	NdisMoveMemory(pAd->SharedKey[BSS0][0].TxMic, pKey->KeyMaterial + LEN_TKIP_EK, LEN_TKIP_TXMICK);
                NdisMoveMemory(pAd->SharedKey[BSS0][0].RxMic, pKey->KeyMaterial + LEN_TKIP_EK + LEN_TKIP_TXMICK, LEN_TKIP_RXMICK);
            }

            
        	if (pAd->StaCfg.PairCipher == Ndis802_11Encryption2Enabled)
        		pAd->SharedKey[BSS0][0].CipherAlg = CIPHER_TKIP;
        	else if (pAd->StaCfg.PairCipher == Ndis802_11Encryption3Enabled)
        		pAd->SharedKey[BSS0][0].CipherAlg = CIPHER_AES;
        	else
        		pAd->SharedKey[BSS0][0].CipherAlg = CIPHER_NONE;

            
        	pEntry = &pAd->MacTab.Content[BSSID_WCID];
            NdisMoveMemory(pEntry->PairwiseKey.Key, pAd->SharedKey[BSS0][0].Key, LEN_TKIP_EK);
        	NdisMoveMemory(pEntry->PairwiseKey.RxMic, pAd->SharedKey[BSS0][0].RxMic, LEN_TKIP_RXMICK);
        	NdisMoveMemory(pEntry->PairwiseKey.TxMic, pAd->SharedKey[BSS0][0].TxMic, LEN_TKIP_TXMICK);
        	pEntry->PairwiseKey.CipherAlg = pAd->SharedKey[BSS0][0].CipherAlg;

        	
        	AsicAddSharedKeyEntry(pAd,
        						  BSS0,
        						  0,
        						  pAd->SharedKey[BSS0][0].CipherAlg,
        						  pAd->SharedKey[BSS0][0].Key,
        						  pAd->SharedKey[BSS0][0].TxMic,
        						  pAd->SharedKey[BSS0][0].RxMic);

        	
        	RTMPAddWcidAttributeEntry(pAd,
        							  BSS0,
        							  0,
        							  pAd->SharedKey[BSS0][0].CipherAlg,
        							  pEntry);

            if (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA2)
            {
                
				STA_PORT_SECURED(pAd);

                
                pAd->IndicateMediaState = NdisMediaStateConnected;
            }
		}
        else
        {
            
            pAd->StaCfg.DefaultKeyId = (pKey->KeyIndex & 0xFF);
            NdisZeroMemory(&pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId], sizeof(CIPHER_KEY));
            pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].KeyLen = LEN_TKIP_EK;
            NdisMoveMemory(pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].Key, pKey->KeyMaterial, LEN_TKIP_EK);

            if (pAd->StaCfg.GroupCipher == Ndis802_11Encryption2Enabled)
            {
                NdisMoveMemory(pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].RxMic, pKey->KeyMaterial + LEN_TKIP_EK, LEN_TKIP_TXMICK);
                NdisMoveMemory(pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].TxMic, pKey->KeyMaterial + LEN_TKIP_EK + LEN_TKIP_TXMICK, LEN_TKIP_RXMICK);
            }
            else
            {
            	NdisMoveMemory(pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].TxMic, pKey->KeyMaterial + LEN_TKIP_EK, LEN_TKIP_TXMICK);
                NdisMoveMemory(pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].RxMic, pKey->KeyMaterial + LEN_TKIP_EK + LEN_TKIP_TXMICK, LEN_TKIP_RXMICK);
            }

            
    		pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].CipherAlg = CIPHER_NONE;
    		if (pAd->StaCfg.GroupCipher == Ndis802_11Encryption2Enabled)
    			pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].CipherAlg = CIPHER_TKIP;
    		else if (pAd->StaCfg.GroupCipher == Ndis802_11Encryption3Enabled)
    			pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].CipherAlg = CIPHER_AES;

            
        	AsicAddSharedKeyEntry(pAd,
        						  BSS0,
        						  pAd->StaCfg.DefaultKeyId,
        						  pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].CipherAlg,
        						  pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].Key,
        						  pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].TxMic,
        						  pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].RxMic);

        	
        	RTMPAddWcidAttributeEntry(pAd,
        							  BSS0,
        							  pAd->StaCfg.DefaultKeyId,
        							  pAd->SharedKey[BSS0][pAd->StaCfg.DefaultKeyId].CipherAlg,
        							  NULL);

            
			STA_PORT_SECURED(pAd);

            
            pAd->IndicateMediaState = NdisMediaStateConnected;
        }
	}
	else	
	{
		UCHAR	CipherAlg;
    	PUCHAR	Key;

		if(pKey->KeyLength == 32)
			goto end;

		KeyIdx = pKey->KeyIndex & 0x0fffffff;

		if (KeyIdx < 4)
		{
			
			if (pKey->KeyIndex & 0x80000000)
			{
				pEntry = MacTableLookup(pAd, pKey->BSSID);

				if (pEntry)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("RTMPAddKey: Set Pair-wise Key\n"));

					
 					pEntry->PairwiseKey.KeyLen = (UCHAR)pKey->KeyLength;
					NdisMoveMemory(pEntry->PairwiseKey.Key, &pKey->KeyMaterial, pKey->KeyLength);

					
					if (pKey->KeyLength == 5)
						pEntry->PairwiseKey.CipherAlg = CIPHER_WEP64;
					else
						pEntry->PairwiseKey.CipherAlg = CIPHER_WEP128;

					
					AsicAddPairwiseKeyEntry(
						pAd,
						pEntry->Addr,
						(UCHAR)pEntry->Aid,
                		&pEntry->PairwiseKey);

					
					RTMPAddWcidAttributeEntry(
						pAd,
						BSS0,
						KeyIdx, 
						pEntry->PairwiseKey.CipherAlg,
						pEntry);

				}
			}
			else
            {
				
				pAd->StaCfg.DefaultKeyId = (UCHAR) KeyIdx;

				
				pAd->SharedKey[BSS0][KeyIdx].KeyLen = (UCHAR) pKey->KeyLength;
				NdisMoveMemory(pAd->SharedKey[BSS0][KeyIdx].Key, &pKey->KeyMaterial, pKey->KeyLength);

				
				if (pKey->KeyLength == 5)
					pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_WEP64;
				else
					pAd->SharedKey[BSS0][KeyIdx].CipherAlg = CIPHER_WEP128;

    			CipherAlg = pAd->SharedKey[BSS0][KeyIdx].CipherAlg;
    			Key = pAd->SharedKey[BSS0][KeyIdx].Key;

				
    			AsicAddSharedKeyEntry(pAd, BSS0, KeyIdx, CipherAlg, Key, NULL, NULL);

				
				RTMPAddWcidAttributeEntry(pAd, BSS0, KeyIdx, CipherAlg, NULL);

			}
		}
	}
end:
#ifdef RT2860
	RTMP_SET_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP);
    DBGPRINT(RT_DEBUG_INFO, ("<------ RTMPAddKey\n"));
#endif
	return;
}

char * rtstrchr(const char * s, int c)
{
    for(; *s != (char) c; ++s)
        if (*s == '\0')
            return NULL;
    return (char *) s;
}



int
rt_ioctl_giwname(struct net_device *dev,
		   struct iw_request_info *info,
		   char *name, char *extra)
{

	strncpy(name, RT28xx_CHIP_NAME " Wireless", IFNAMSIZ);
	return 0;
}

int rt_ioctl_siwfreq(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_freq *freq, char *extra)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;
	int 	chan = -1;

    
    if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;
    }


	if (freq->e > 1)
		return -EINVAL;

	if((freq->e == 0) && (freq->m <= 1000))
		chan = freq->m;	
	else
		MAP_KHZ_TO_CHANNEL_ID( (freq->m /100) , chan); 

    if (ChannelSanity(pAdapter, chan) == TRUE)
    {
	pAdapter->CommonCfg.Channel = chan;
	DBGPRINT(RT_DEBUG_ERROR, ("==>rt_ioctl_siwfreq::SIOCSIWFREQ[cmd=0x%x] (Channel=%d)\n", SIOCSIWFREQ, pAdapter->CommonCfg.Channel));
    }
    else
        return -EINVAL;

	return 0;
}
int rt_ioctl_giwfreq(struct net_device *dev,
		   struct iw_request_info *info,
		   struct iw_freq *freq, char *extra)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;
	UCHAR ch = pAdapter->CommonCfg.Channel;
	ULONG	m;

	DBGPRINT(RT_DEBUG_TRACE,("==>rt_ioctl_giwfreq  %d\n", ch));

    MAP_CHANNEL_ID_TO_KHZ(ch, m);
	freq->m = m * 100;
	freq->e = 1;
	return 0;
}

int rt_ioctl_siwmode(struct net_device *dev,
		   struct iw_request_info *info,
		   __u32 *mode, char *extra)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;

	
    if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
    	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
       	return -ENETDOWN;
    }

	switch (*mode)
	{
		case IW_MODE_ADHOC:
			Set_NetworkType_Proc(pAdapter, "Adhoc");
			break;
		case IW_MODE_INFRA:
			Set_NetworkType_Proc(pAdapter, "Infra");
			break;
        case IW_MODE_MONITOR:
			Set_NetworkType_Proc(pAdapter, "Monitor");
			break;
		default:
			DBGPRINT(RT_DEBUG_TRACE, ("===>rt_ioctl_siwmode::SIOCSIWMODE (unknown %d)\n", *mode));
			return -EINVAL;
	}

	
	pAdapter->StaCfg.WpaState = SS_NOTUSE;

	return 0;
}

int rt_ioctl_giwmode(struct net_device *dev,
		   struct iw_request_info *info,
		   __u32 *mode, char *extra)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;

	if (ADHOC_ON(pAdapter))
		*mode = IW_MODE_ADHOC;
    else if (INFRA_ON(pAdapter))
		*mode = IW_MODE_INFRA;
    else if (MONITOR_ON(pAdapter))
    {
        *mode = IW_MODE_MONITOR;
    }
    else
        *mode = IW_MODE_AUTO;

	DBGPRINT(RT_DEBUG_TRACE, ("==>rt_ioctl_giwmode(mode=%d)\n", *mode));
	return 0;
}

int rt_ioctl_siwsens(struct net_device *dev,
		   struct iw_request_info *info,
		   char *name, char *extra)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;

	
    	if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    	{
        	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        	return -ENETDOWN;
    	}

	return 0;
}

int rt_ioctl_giwsens(struct net_device *dev,
		   struct iw_request_info *info,
		   char *name, char *extra)
{
	return 0;
}

int rt_ioctl_giwrange(struct net_device *dev,
		   struct iw_request_info *info,
		   struct iw_point *data, char *extra)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;
	struct iw_range *range = (struct iw_range *) extra;
	u16 val;
	int i;

	DBGPRINT(RT_DEBUG_TRACE ,("===>rt_ioctl_giwrange\n"));
	data->length = sizeof(struct iw_range);
	memset(range, 0, sizeof(struct iw_range));

	range->txpower_capa = IW_TXPOW_DBM;

	if (INFRA_ON(pAdapter)||ADHOC_ON(pAdapter))
	{
		range->min_pmp = 1 * 1024;
		range->max_pmp = 65535 * 1024;
		range->min_pmt = 1 * 1024;
		range->max_pmt = 1000 * 1024;
		range->pmp_flags = IW_POWER_PERIOD;
		range->pmt_flags = IW_POWER_TIMEOUT;
		range->pm_capa = IW_POWER_PERIOD | IW_POWER_TIMEOUT |
			IW_POWER_UNICAST_R | IW_POWER_ALL_R;
	}

	range->we_version_compiled = WIRELESS_EXT;
	range->we_version_source = 14;

	range->retry_capa = IW_RETRY_LIMIT;
	range->retry_flags = IW_RETRY_LIMIT;
	range->min_retry = 0;
	range->max_retry = 255;

	range->num_channels =  pAdapter->ChannelListNum;

	val = 0;
	for (i = 1; i <= range->num_channels; i++)
	{
		u32 m;
		range->freq[val].i = pAdapter->ChannelList[i-1].Channel;
		MAP_CHANNEL_ID_TO_KHZ(pAdapter->ChannelList[i-1].Channel, m);
		range->freq[val].m = m * 100; 

		range->freq[val].e = 1;
		val++;
		if (val == IW_MAX_FREQUENCIES)
			break;
	}
	range->num_frequency = val;

	range->max_qual.qual = 100; 
	range->max_qual.level = 0; 
	range->max_qual.noise = 0; 

	
	range->avg_qual.qual = 20;
	range->avg_qual.level = -60;
	range->avg_qual.noise = -95;
	range->sensitivity = 3;

	range->max_encoding_tokens = NR_WEP_KEYS;
	range->num_encoding_sizes = 2;
	range->encoding_size[0] = 5;
	range->encoding_size[1] = 13;

	range->min_rts = 0;
	range->max_rts = 2347;
	range->min_frag = 256;
	range->max_frag = 2346;

	
	range->enc_capa = IW_ENC_CAPA_WPA | IW_ENC_CAPA_WPA2 |
					IW_ENC_CAPA_CIPHER_TKIP | IW_ENC_CAPA_CIPHER_CCMP;

	return 0;
}

int rt_ioctl_siwap(struct net_device *dev,
		      struct iw_request_info *info,
		      struct sockaddr *ap_addr, char *extra)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;
    NDIS_802_11_MAC_ADDRESS Bssid;

	
	if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
       	return -ENETDOWN;
    }

	if (pAdapter->Mlme.CntlMachine.CurrState != CNTL_IDLE)
    {
        RT28XX_MLME_RESET_STATE_MACHINE(pAdapter);
        DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
    }

    
    
    pAdapter->MlmeAux.CurrReqIsFromNdis = FALSE;
	
	pAdapter->MlmeAux.AutoReconnectSsidLen= 32;

    memset(Bssid, 0, MAC_ADDR_LEN);
    memcpy(Bssid, ap_addr->sa_data, MAC_ADDR_LEN);
    MlmeEnqueue(pAdapter,
                MLME_CNTL_STATE_MACHINE,
                OID_802_11_BSSID,
                sizeof(NDIS_802_11_MAC_ADDRESS),
                (VOID *)&Bssid);

    DBGPRINT(RT_DEBUG_TRACE, ("IOCTL::SIOCSIWAP %02x:%02x:%02x:%02x:%02x:%02x\n",
        Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5]));

	return 0;
}

int rt_ioctl_giwap(struct net_device *dev,
		      struct iw_request_info *info,
		      struct sockaddr *ap_addr, char *extra)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;

	if (INFRA_ON(pAdapter) || ADHOC_ON(pAdapter))
	{
		ap_addr->sa_family = ARPHRD_ETHER;
		memcpy(ap_addr->sa_data, &pAdapter->CommonCfg.Bssid, ETH_ALEN);
	}
    
    else if (pAdapter->StaCfg.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE)
    {
        ap_addr->sa_family = ARPHRD_ETHER;
        memcpy(ap_addr->sa_data, &pAdapter->MlmeAux.Bssid, ETH_ALEN);
    }
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("IOCTL::SIOCGIWAP(=EMPTY)\n"));
		return -ENOTCONN;
	}

	return 0;
}


static void set_quality(PRTMP_ADAPTER pAdapter,
                        struct iw_quality *iq,
                        signed char rssi)
{
	__u8 ChannelQuality;

	
	if (rssi >= -50)
		ChannelQuality = 100;
	else if (rssi >= -80) 
		ChannelQuality = (__u8)(24 + ((rssi + 80) * 26)/10);
	else if (rssi >= -90)   
        ChannelQuality = (__u8)((rssi + 90) * 26)/10;
	else
		ChannelQuality = 0;

    iq->qual = (__u8)ChannelQuality;

    iq->level = (__u8)(rssi);
    iq->noise = (pAdapter->BbpWriteLatch[66] > pAdapter->BbpTuning.FalseCcaUpperThreshold) ? ((__u8)pAdapter->BbpTuning.FalseCcaUpperThreshold) : ((__u8) pAdapter->BbpWriteLatch[66]); 	
    iq->noise += 256 - 143;
    iq->updated = pAdapter->iw_stats.qual.updated;
}

int rt_ioctl_iwaplist(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_point *data, char *extra)
{
 	PRTMP_ADAPTER pAdapter = dev->ml_priv;

	struct sockaddr addr[IW_MAX_AP];
	struct iw_quality qual[IW_MAX_AP];
	int i;

   	
    if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
		data->length = 0;
		return 0;
        
	}

	for (i = 0; i <IW_MAX_AP ; i++)
	{
		if (i >=  pAdapter->ScanTab.BssNr)
			break;
		addr[i].sa_family = ARPHRD_ETHER;
			memcpy(addr[i].sa_data, &pAdapter->ScanTab.BssEntry[i].Bssid, MAC_ADDR_LEN);
		set_quality(pAdapter, &qual[i], pAdapter->ScanTab.BssEntry[i].Rssi);
	}
	data->length = i;
	memcpy(extra, &addr, i*sizeof(addr[0]));
	data->flags = 1;		
	memcpy(extra + i*sizeof(addr[0]), &qual, i*sizeof(qual[i]));

	return 0;
}

int rt_ioctl_siwscan(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_point *data, char *extra)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;

	ULONG								Now;
	int Status = NDIS_STATUS_SUCCESS;

	
	if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
		return -ENETDOWN;
	}

	if (MONITOR_ON(pAdapter))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("!!! Driver is in Monitor Mode now !!!\n"));
        return -EINVAL;
    }
#ifdef RT2860
	if ((pAdapter->OpMode == OPMODE_STA) && (IDLE_ON(pAdapter))
		&& (pAdapter->StaCfg.bRadio == TRUE)
		&& (RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_IDLE_RADIO_OFF)))
	{
		RT28xxPciAsicRadioOn(pAdapter, GUI_IDLE_POWER_SAVE);
	}
	
	else if (pAdapter->bPCIclkOff == TRUE)
		return 0;
#endif
	if (pAdapter->StaCfg.WpaSupplicantUP == WPA_SUPPLICANT_ENABLE)
	{
		pAdapter->StaCfg.WpaSupplicantScanCount++;
	}

    pAdapter->StaCfg.bScanReqIsFromWebUI = TRUE;
	if (RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		return 0;
	do{
		Now = jiffies;

		if ((pAdapter->StaCfg.WpaSupplicantUP == WPA_SUPPLICANT_ENABLE) &&
			(pAdapter->StaCfg.WpaSupplicantScanCount > 3))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("!!! WpaSupplicantScanCount > 3\n"));
			Status = NDIS_STATUS_SUCCESS;
			break;
		}

		if ((OPSTATUS_TEST_FLAG(pAdapter, fOP_STATUS_MEDIA_STATE_CONNECTED)) &&
			((pAdapter->StaCfg.AuthMode == Ndis802_11AuthModeWPA) ||
			(pAdapter->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK)) &&
			(pAdapter->StaCfg.PortSecured == WPA_802_1X_PORT_NOT_SECURED))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("!!! Link UP, Port Not Secured! ignore this set::OID_802_11_BSSID_LIST_SCAN\n"));
			Status = NDIS_STATUS_SUCCESS;
			break;
		}

		if (pAdapter->Mlme.CntlMachine.CurrState != CNTL_IDLE)
		{
			RT28XX_MLME_RESET_STATE_MACHINE(pAdapter);
			DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
		}

		
		
		pAdapter->MlmeAux.CurrReqIsFromNdis = FALSE;
		
		pAdapter->StaCfg.ScanCnt = 0;
		pAdapter->StaCfg.LastScanTime = Now;

		MlmeEnqueue(pAdapter,
			MLME_CNTL_STATE_MACHINE,
			OID_802_11_BSSID_LIST_SCAN,
			0,
			NULL);

		Status = NDIS_STATUS_SUCCESS;
		RT28XX_MLME_HANDLER(pAdapter);
	}while(0);
	return 0;
}

int rt_ioctl_giwscan(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_point *data, char *extra)
{

	PRTMP_ADAPTER pAdapter = dev->ml_priv;
	int i=0;
	char *current_ev = extra, *previous_ev = extra;
	char *end_buf;
	char *current_val, custom[MAX_CUSTOM_LEN] = {0};
	struct iw_event iwe;

	if (RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
    {
		
		return -EAGAIN;
	}

	if (pAdapter->StaCfg.WpaSupplicantUP == WPA_SUPPLICANT_ENABLE)
	{
		pAdapter->StaCfg.WpaSupplicantScanCount = 0;
	}

	if (pAdapter->ScanTab.BssNr == 0)
	{
		data->length = 0;
		return 0;
	}

    if (data->length > 0)
        end_buf = extra + data->length;
    else
        end_buf = extra + IW_SCAN_MAX_DATA;

	for (i = 0; i < pAdapter->ScanTab.BssNr; i++)
	{
		if (current_ev >= end_buf)
			return -E2BIG;

		
		
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWAP;
		iwe.u.ap_addr.sa_family = ARPHRD_ETHER;
		memcpy(iwe.u.ap_addr.sa_data, &pAdapter->ScanTab.BssEntry[i].Bssid, ETH_ALEN);

        previous_ev = current_ev;
		current_ev = iwe_stream_add_event(info, current_ev,end_buf, &iwe, IW_EV_ADDR_LEN);
        if (current_ev == previous_ev)
		return -E2BIG;

		
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWNAME;


	{
		PBSS_ENTRY pBssEntry=&pAdapter->ScanTab.BssEntry[i];
		BOOLEAN isGonly=FALSE;
		int rateCnt=0;

		if (pBssEntry->Channel>14)
		{
			if (pBssEntry->HtCapabilityLen!=0)
				strcpy(iwe.u.name,"802.11a/n");
			else
				strcpy(iwe.u.name,"802.11a");
		}
		else
		{
			
			for (rateCnt=0;rateCnt<pBssEntry->SupRateLen;rateCnt++)
			{
				
				if (pBssEntry->SupRate[rateCnt]==140 || pBssEntry->SupRate[rateCnt]==146 || pBssEntry->SupRate[rateCnt]>=152)
					isGonly=TRUE;
			}

			for (rateCnt=0;rateCnt<pBssEntry->ExtRateLen;rateCnt++)
			{
				if (pBssEntry->ExtRate[rateCnt]==140 || pBssEntry->ExtRate[rateCnt]==146 || pBssEntry->ExtRate[rateCnt]>=152)
					isGonly=TRUE;
			}


			if (pBssEntry->HtCapabilityLen!=0)
			{
				if (isGonly==TRUE)
					strcpy(iwe.u.name,"802.11g/n");
				else
					strcpy(iwe.u.name,"802.11b/g/n");
			}
			else
			{
				if (isGonly==TRUE)
					strcpy(iwe.u.name,"802.11g");
				else
				{
					if (pBssEntry->SupRateLen==4 && pBssEntry->ExtRateLen==0)
						strcpy(iwe.u.name,"802.11b");
					else
						strcpy(iwe.u.name,"802.11b/g");
				}
			}
		}
	}

		previous_ev = current_ev;
		current_ev	 = iwe_stream_add_event(info, current_ev, end_buf, &iwe, IW_EV_ADDR_LEN);
        if (current_ev == previous_ev)
		return -E2BIG;

		
		
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWESSID;
		iwe.u.data.length = pAdapter->ScanTab.BssEntry[i].SsidLen;
		iwe.u.data.flags = 1;

        previous_ev = current_ev;
		current_ev = iwe_stream_add_point(info, current_ev,end_buf, &iwe, pAdapter->ScanTab.BssEntry[i].Ssid);
        if (current_ev == previous_ev)
		return -E2BIG;

		
		
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWMODE;
		if (pAdapter->ScanTab.BssEntry[i].BssType == Ndis802_11IBSS)
		{
			iwe.u.mode = IW_MODE_ADHOC;
		}
		else if (pAdapter->ScanTab.BssEntry[i].BssType == Ndis802_11Infrastructure)
		{
			iwe.u.mode = IW_MODE_INFRA;
		}
		else
		{
			iwe.u.mode = IW_MODE_AUTO;
		}
		iwe.len = IW_EV_UINT_LEN;

        previous_ev = current_ev;
		current_ev = iwe_stream_add_event(info, current_ev, end_buf, &iwe,  IW_EV_UINT_LEN);
        if (current_ev == previous_ev)
		return -E2BIG;

		
		
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWFREQ;
		if (INFRA_ON(pAdapter) || ADHOC_ON(pAdapter))
			iwe.u.freq.m = pAdapter->ScanTab.BssEntry[i].Channel;
		else
			iwe.u.freq.m = pAdapter->ScanTab.BssEntry[i].Channel;
		iwe.u.freq.e = 0;
		iwe.u.freq.i = 0;

		previous_ev = current_ev;
		current_ev = iwe_stream_add_event(info, current_ev, end_buf, &iwe, IW_EV_FREQ_LEN);
        if (current_ev == previous_ev)
		return -E2BIG;

        
        
        memset(&iwe, 0, sizeof(iwe));
    	iwe.cmd = IWEVQUAL;
    	iwe.u.qual.level = 0;
    	iwe.u.qual.noise = 0;
        set_quality(pAdapter, &iwe.u.qual, pAdapter->ScanTab.BssEntry[i].Rssi);
    	current_ev = iwe_stream_add_event(info, current_ev, end_buf, &iwe, IW_EV_QUAL_LEN);
        if (current_ev == previous_ev)
		return -E2BIG;

		
		
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = SIOCGIWENCODE;
		if (CAP_IS_PRIVACY_ON (pAdapter->ScanTab.BssEntry[i].CapabilityInfo ))
			iwe.u.data.flags =IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
		else
			iwe.u.data.flags = IW_ENCODE_DISABLED;

        previous_ev = current_ev;
        current_ev = iwe_stream_add_point(info, current_ev, end_buf,&iwe, (char *)pAdapter->SharedKey[BSS0][(iwe.u.data.flags & IW_ENCODE_INDEX)-1].Key);
        if (current_ev == previous_ev)
		return -E2BIG;

		
		
		if (pAdapter->ScanTab.BssEntry[i].SupRateLen)
        {
            UCHAR tmpRate = pAdapter->ScanTab.BssEntry[i].SupRate[pAdapter->ScanTab.BssEntry[i].SupRateLen-1];
			memset(&iwe, 0, sizeof(iwe));
			iwe.cmd = SIOCGIWRATE;
    		current_val = current_ev + IW_EV_LCP_LEN;
            if (tmpRate == 0x82)
                iwe.u.bitrate.value =  1 * 1000000;
            else if (tmpRate == 0x84)
                iwe.u.bitrate.value =  2 * 1000000;
            else if (tmpRate == 0x8B)
                iwe.u.bitrate.value =  5.5 * 1000000;
            else if (tmpRate == 0x96)
                iwe.u.bitrate.value =  11 * 1000000;
            else
    		    iwe.u.bitrate.value =  (tmpRate/2) * 1000000;

			iwe.u.bitrate.disabled = 0;
			current_val = iwe_stream_add_value(info, current_ev,
				current_val, end_buf, &iwe,
    			IW_EV_PARAM_LEN);

        	if((current_val-current_ev)>IW_EV_LCP_LEN)
            	current_ev = current_val;
        	else
			return -E2BIG;
        }

		
		if (pAdapter->ScanTab.BssEntry[i].WpaIE.IELen > 0)
		{
			memset(&iwe, 0, sizeof(iwe));
			memset(&custom[0], 0, MAX_CUSTOM_LEN);
			memcpy(custom, &(pAdapter->ScanTab.BssEntry[i].WpaIE.IE[0]),
						   pAdapter->ScanTab.BssEntry[i].WpaIE.IELen);
			iwe.cmd = IWEVGENIE;
			iwe.u.data.length = pAdapter->ScanTab.BssEntry[i].WpaIE.IELen;
			current_ev = iwe_stream_add_point(info, current_ev, end_buf, &iwe, custom);
			if (current_ev == previous_ev)
				return -E2BIG;
		}

		
        if (pAdapter->ScanTab.BssEntry[i].RsnIE.IELen > 0)
        {
        	memset(&iwe, 0, sizeof(iwe));
			memset(&custom[0], 0, MAX_CUSTOM_LEN);
			memcpy(custom, &(pAdapter->ScanTab.BssEntry[i].RsnIE.IE[0]),
						   pAdapter->ScanTab.BssEntry[i].RsnIE.IELen);
			iwe.cmd = IWEVGENIE;
			iwe.u.data.length = pAdapter->ScanTab.BssEntry[i].RsnIE.IELen;
			current_ev = iwe_stream_add_point(info, current_ev, end_buf, &iwe, custom);
			if (current_ev == previous_ev)
				return -E2BIG;
        }
	}

	data->length = current_ev - extra;
    pAdapter->StaCfg.bScanReqIsFromWebUI = FALSE;
	DBGPRINT(RT_DEBUG_ERROR ,("===>rt_ioctl_giwscan. %d(%d) BSS returned, data->length = %d\n",i , pAdapter->ScanTab.BssNr, data->length));
	return 0;
}

int rt_ioctl_siwessid(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_point *data, char *essid)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;

	
    if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
       	DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
       	return -ENETDOWN;
    }

	if (data->flags)
	{
		PCHAR	pSsidString = NULL;

		
		if (data->length > (IW_ESSID_MAX_SIZE + 1))
			return -E2BIG;

		pSsidString = (CHAR *) kmalloc(MAX_LEN_OF_SSID+1, MEM_ALLOC_FLAG);
		if (pSsidString)
		{
			NdisZeroMemory(pSsidString, MAX_LEN_OF_SSID+1);
			NdisMoveMemory(pSsidString, essid, data->length);
			if (Set_SSID_Proc(pAdapter, pSsidString) == FALSE)
				return -EINVAL;
		}
		else
			return -ENOMEM;
	}
	else
	{
		
		if (Set_SSID_Proc(pAdapter, "") == FALSE)
			return -EINVAL;
    }
	return 0;
}

int rt_ioctl_giwessid(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_point *data, char *essid)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;

	data->flags = 1;
    if (MONITOR_ON(pAdapter))
    {
        data->length  = 0;
        return 0;
    }

	if (OPSTATUS_TEST_FLAG(pAdapter, fOP_STATUS_MEDIA_STATE_CONNECTED))
	{
		DBGPRINT(RT_DEBUG_TRACE ,("MediaState is connected\n"));
		data->length = pAdapter->CommonCfg.SsidLen;
		memcpy(essid, pAdapter->CommonCfg.Ssid, pAdapter->CommonCfg.SsidLen);
	}
#ifdef RT2870
    
    else if (pAdapter->StaCfg.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE)
    {
        data->length = pAdapter->CommonCfg.SsidLen;
		memcpy(essid, pAdapter->CommonCfg.Ssid, pAdapter->CommonCfg.SsidLen);
	}
#endif 
	else
	{
		data->length  = 0;
		DBGPRINT(RT_DEBUG_TRACE ,("MediaState is not connected, ess\n"));
	}

	return 0;

}

int rt_ioctl_siwnickn(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_point *data, char *nickname)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;

    
    if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
        DBGPRINT(RT_DEBUG_TRACE ,("INFO::Network is down!\n"));
        return -ENETDOWN;
    }

	if (data->length > IW_ESSID_MAX_SIZE)
		return -EINVAL;

	memset(pAdapter->nickname, 0, IW_ESSID_MAX_SIZE + 1);
	memcpy(pAdapter->nickname, nickname, data->length);


	return 0;
}

int rt_ioctl_giwnickn(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_point *data, char *nickname)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;

	if (data->length > strlen(pAdapter->nickname) + 1)
		data->length = strlen(pAdapter->nickname) + 1;
	if (data->length > 0) {
		memcpy(nickname, pAdapter->nickname, data->length-1);
		nickname[data->length-1] = '\0';
	}
	return 0;
}

int rt_ioctl_siwrts(struct net_device *dev,
		       struct iw_request_info *info,
		       struct iw_param *rts, char *extra)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;
	u16 val;

    
    if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        return -ENETDOWN;
    }

	if (rts->disabled)
		val = MAX_RTS_THRESHOLD;
	else if (rts->value < 0 || rts->value > MAX_RTS_THRESHOLD)
		return -EINVAL;
	else if (rts->value == 0)
	    val = MAX_RTS_THRESHOLD;
	else
		val = rts->value;

	if (val != pAdapter->CommonCfg.RtsThreshold)
		pAdapter->CommonCfg.RtsThreshold = val;

	return 0;
}

int rt_ioctl_giwrts(struct net_device *dev,
		       struct iw_request_info *info,
		       struct iw_param *rts, char *extra)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;

	
    	if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    	{
      		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        	return -ENETDOWN;
    	}

	rts->value = pAdapter->CommonCfg.RtsThreshold;
	rts->disabled = (rts->value == MAX_RTS_THRESHOLD);
	rts->fixed = 1;

	return 0;
}

int rt_ioctl_siwfrag(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *frag, char *extra)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;
	u16 val;

	
    	if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    	{
      		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        	return -ENETDOWN;
    	}

	if (frag->disabled)
		val = MAX_FRAG_THRESHOLD;
	else if (frag->value >= MIN_FRAG_THRESHOLD && frag->value <= MAX_FRAG_THRESHOLD)
		val = __cpu_to_le16(frag->value & ~0x1); 
	else if (frag->value == 0)
	    val = MAX_FRAG_THRESHOLD;
	else
		return -EINVAL;

	pAdapter->CommonCfg.FragmentThreshold = val;
	return 0;
}

int rt_ioctl_giwfrag(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *frag, char *extra)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;

	
    	if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    	{
      		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        	return -ENETDOWN;
    	}

	frag->value = pAdapter->CommonCfg.FragmentThreshold;
	frag->disabled = (frag->value == MAX_FRAG_THRESHOLD);
	frag->fixed = 1;

	return 0;
}

#define MAX_WEP_KEY_SIZE 13
#define MIN_WEP_KEY_SIZE 5
int rt_ioctl_siwencode(struct net_device *dev,
			  struct iw_request_info *info,
			  struct iw_point *erq, char *extra)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;

	
    	if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    	{
      		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
        	return -ENETDOWN;
    	}

	if ((erq->length == 0) &&
        (erq->flags & IW_ENCODE_DISABLED))
	{
		pAdapter->StaCfg.PairCipher = Ndis802_11WEPDisabled;
		pAdapter->StaCfg.GroupCipher = Ndis802_11WEPDisabled;
		pAdapter->StaCfg.WepStatus = Ndis802_11WEPDisabled;
        pAdapter->StaCfg.OrigWepStatus = pAdapter->StaCfg.WepStatus;
        pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeOpen;
        goto done;
	} else if (
		 (erq->flags & IW_ENCODE_RESTRICTED || erq->flags & IW_ENCODE_OPEN)) {
		STA_PORT_SECURED(pAdapter);
		pAdapter->StaCfg.PairCipher = Ndis802_11WEPEnabled;
		pAdapter->StaCfg.GroupCipher = Ndis802_11WEPEnabled;
		pAdapter->StaCfg.WepStatus = Ndis802_11WEPEnabled;
        pAdapter->StaCfg.OrigWepStatus = pAdapter->StaCfg.WepStatus;
		if (erq->flags & IW_ENCODE_RESTRICTED)
			pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeShared;
    	else
			pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeOpen;
	}

    if (erq->length > 0)
	{
		int keyIdx = (erq->flags & IW_ENCODE_INDEX) - 1;
		
		if (erq->length > MAX_WEP_KEY_SIZE) {
			return -EINVAL;
		}
		
		if ((keyIdx < 0) || (keyIdx >= NR_WEP_KEYS))
        {
            DBGPRINT(RT_DEBUG_TRACE ,("==>rt_ioctl_siwencode::Wrong keyIdx=%d! Using default key instead (%d)\n",
                                        keyIdx, pAdapter->StaCfg.DefaultKeyId));

            
			keyIdx = pAdapter->StaCfg.DefaultKeyId;
        }
		else
			pAdapter->StaCfg.DefaultKeyId=keyIdx;

        NdisZeroMemory(pAdapter->SharedKey[BSS0][keyIdx].Key,  16);

		if (erq->length == MAX_WEP_KEY_SIZE)
        {
			pAdapter->SharedKey[BSS0][keyIdx].KeyLen = MAX_WEP_KEY_SIZE;
            pAdapter->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_WEP128;
		}
		else if (erq->length == MIN_WEP_KEY_SIZE)
        {
            pAdapter->SharedKey[BSS0][keyIdx].KeyLen = MIN_WEP_KEY_SIZE;
            pAdapter->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_WEP64;
		}
		else
			
			pAdapter->SharedKey[BSS0][keyIdx].KeyLen = 0;

		
		if(!(erq->flags & IW_ENCODE_NOKEY)) {
			
			NdisMoveMemory(pAdapter->SharedKey[BSS0][keyIdx].Key, extra, erq->length);
        }
	}
    else
			{
		
		int index = (erq->flags & IW_ENCODE_INDEX) - 1;
		if ((index >= 0) && (index < 4))
        {
			pAdapter->StaCfg.DefaultKeyId = index;
            }
        else
			
			if (!(erq->flags & IW_ENCODE_MODE)) {
				return -EINVAL;
		}
	}

done:
    DBGPRINT(RT_DEBUG_TRACE ,("==>rt_ioctl_siwencode::erq->flags=%x\n",erq->flags));
	DBGPRINT(RT_DEBUG_TRACE ,("==>rt_ioctl_siwencode::AuthMode=%x\n",pAdapter->StaCfg.AuthMode));
	DBGPRINT(RT_DEBUG_TRACE ,("==>rt_ioctl_siwencode::DefaultKeyId=%x, KeyLen = %d\n",pAdapter->StaCfg.DefaultKeyId , pAdapter->SharedKey[BSS0][pAdapter->StaCfg.DefaultKeyId].KeyLen));
	DBGPRINT(RT_DEBUG_TRACE ,("==>rt_ioctl_siwencode::WepStatus=%x\n",pAdapter->StaCfg.WepStatus));
	return 0;
}

int
rt_ioctl_giwencode(struct net_device *dev,
			  struct iw_request_info *info,
			  struct iw_point *erq, char *key)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;
	int kid;

	
	if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
  		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
    	return -ENETDOWN;
	}

	kid = erq->flags & IW_ENCODE_INDEX;
	DBGPRINT(RT_DEBUG_TRACE, ("===>rt_ioctl_giwencode %d\n", erq->flags & IW_ENCODE_INDEX));

	if (pAdapter->StaCfg.WepStatus == Ndis802_11WEPDisabled)
	{
		erq->length = 0;
		erq->flags = IW_ENCODE_DISABLED;
	}
	else if ((kid > 0) && (kid <=4))
	{
		
		erq->flags = kid ;			
		if (erq->length > pAdapter->SharedKey[BSS0][kid-1].KeyLen)
			erq->length = pAdapter->SharedKey[BSS0][kid-1].KeyLen;
		memcpy(key, pAdapter->SharedKey[BSS0][kid-1].Key, erq->length);
		
		
		if (pAdapter->StaCfg.AuthMode == Ndis802_11AuthModeShared)
			erq->flags |= IW_ENCODE_RESTRICTED;		
		else
			erq->flags |= IW_ENCODE_OPEN;		

	}
	else if (kid == 0)
	{
		if (pAdapter->StaCfg.AuthMode == Ndis802_11AuthModeShared)
			erq->flags |= IW_ENCODE_RESTRICTED;		
		else
			erq->flags |= IW_ENCODE_OPEN;		
		erq->length = pAdapter->SharedKey[BSS0][pAdapter->StaCfg.DefaultKeyId].KeyLen;
		memcpy(key, pAdapter->SharedKey[BSS0][pAdapter->StaCfg.DefaultKeyId].Key, erq->length);
		
		if (pAdapter->StaCfg.AuthMode == Ndis802_11AuthModeShared)
			erq->flags |= IW_ENCODE_RESTRICTED;		
		else
			erq->flags |= IW_ENCODE_OPEN;		
		erq->flags = pAdapter->StaCfg.DefaultKeyId + 1;			
		erq->flags |= IW_ENCODE_ENABLED;	
	}

	return 0;

}

static int
rt_ioctl_setparam(struct net_device *dev, struct iw_request_info *info,
			 void *w, char *extra)
{
	PRTMP_ADAPTER pAdapter = dev->ml_priv;
	POS_COOKIE pObj = (POS_COOKIE)pAdapter->OS_Cookie;
	char *this_char = extra;
	char *value;
	int  Status=0;

	{
		pObj->ioctl_if_type = INT_MAIN;
        pObj->ioctl_if = MAIN_MBSSID;
	}

	
    	if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    	{
      		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
			return -ENETDOWN;
    	}

	if (!*this_char)
		return -EINVAL;

	if ((value = rtstrchr(this_char, '=')) != NULL)
	    *value++ = 0;

	if (!value)
	    return -EINVAL;

	
    if (!*value && (strcmp(this_char, "SSID") != 0))
        return -EINVAL;

	for (PRTMP_PRIVATE_SET_PROC = RTMP_PRIVATE_SUPPORT_PROC; PRTMP_PRIVATE_SET_PROC->name; PRTMP_PRIVATE_SET_PROC++)
	{
	    if (strcmp(this_char, PRTMP_PRIVATE_SET_PROC->name) == 0)
	    {
	        if(!PRTMP_PRIVATE_SET_PROC->set_proc(pAdapter, value))
	        {	
			    Status = -EINVAL;
	        }
		    break;	
	    }
	}

	if(PRTMP_PRIVATE_SET_PROC->name == NULL)
	{  
	    Status = -EINVAL;
	    DBGPRINT(RT_DEBUG_TRACE, ("===>rt_ioctl_setparam:: (iwpriv) Not Support Set Command [%s=%s]\n", this_char, value));
	}

    return Status;
}


static int
rt_private_get_statistics(struct net_device *dev, struct iw_request_info *info,
		struct iw_point *wrq, char *extra)
{
	INT				Status = 0;
    PRTMP_ADAPTER   pAd = dev->ml_priv;

    if (extra == NULL)
    {
        wrq->length = 0;
        return -EIO;
    }

    memset(extra, 0x00, IW_PRIV_SIZE_MASK);
    sprintf(extra, "\n\n");

	{
    sprintf(extra+strlen(extra), "Tx success                      = %ld\n", (ULONG)pAd->WlanCounters.TransmittedFragmentCount.QuadPart);
    sprintf(extra+strlen(extra), "Tx success without retry        = %ld\n", (ULONG)pAd->WlanCounters.TransmittedFragmentCount.QuadPart - (ULONG)pAd->WlanCounters.RetryCount.QuadPart);
	}
    sprintf(extra+strlen(extra), "Tx success after retry          = %ld\n", (ULONG)pAd->WlanCounters.RetryCount.QuadPart);
    sprintf(extra+strlen(extra), "Tx fail to Rcv ACK after retry  = %ld\n", (ULONG)pAd->WlanCounters.FailedCount.QuadPart);
    sprintf(extra+strlen(extra), "RTS Success Rcv CTS             = %ld\n", (ULONG)pAd->WlanCounters.RTSSuccessCount.QuadPart);
    sprintf(extra+strlen(extra), "RTS Fail Rcv CTS                = %ld\n", (ULONG)pAd->WlanCounters.RTSFailureCount.QuadPart);

    sprintf(extra+strlen(extra), "Rx success                      = %ld\n", (ULONG)pAd->WlanCounters.ReceivedFragmentCount.QuadPart);
    sprintf(extra+strlen(extra), "Rx with CRC                     = %ld\n", (ULONG)pAd->WlanCounters.FCSErrorCount.QuadPart);
    sprintf(extra+strlen(extra), "Rx drop due to out of resource  = %ld\n", (ULONG)pAd->Counters8023.RxNoBuffer);
    sprintf(extra+strlen(extra), "Rx duplicate frame              = %ld\n", (ULONG)pAd->WlanCounters.FrameDuplicateCount.QuadPart);

    sprintf(extra+strlen(extra), "False CCA (one second)          = %ld\n", (ULONG)pAd->RalinkCounters.OneSecFalseCCACnt);
	{
    	sprintf(extra+strlen(extra), "RSSI-A                          = %ld\n", (LONG)(pAd->StaCfg.RssiSample.LastRssi0 - pAd->BbpRssiToDbmDelta));
        sprintf(extra+strlen(extra), "RSSI-B (if available)           = %ld\n", (LONG)(pAd->StaCfg.RssiSample.LastRssi1 - pAd->BbpRssiToDbmDelta));
        sprintf(extra+strlen(extra), "RSSI-C (if available)           = %ld\n\n", (LONG)(pAd->StaCfg.RssiSample.LastRssi2 - pAd->BbpRssiToDbmDelta));
	}
    sprintf(extra+strlen(extra), "WpaSupplicantUP                 = %d\n\n", pAd->StaCfg.WpaSupplicantUP);

    wrq->length = strlen(extra) + 1; 
    DBGPRINT(RT_DEBUG_TRACE, ("<== rt_private_get_statistics, wrq->length = %d\n", wrq->length));

    return Status;
}

void	getBaInfo(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pOutBuf)
{
	INT i, j;
	BA_ORI_ENTRY *pOriBAEntry;
	BA_REC_ENTRY *pRecBAEntry;

	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (((pEntry->ValidAsCLI || pEntry->ValidAsApCli) && (pEntry->Sst == SST_ASSOC))
			|| (pEntry->ValidAsWDS) || (pEntry->ValidAsMesh))
		{
			sprintf(pOutBuf + strlen(pOutBuf), "\n%02X:%02X:%02X:%02X:%02X:%02X (Aid = %d) (AP) -\n",
				pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
				pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5], pEntry->Aid);

			sprintf(pOutBuf, "%s[Recipient]\n", pOutBuf);
			for (j=0; j < NUM_OF_TID; j++)
			{
				if (pEntry->BARecWcidArray[j] != 0)
				{
					pRecBAEntry =&pAd->BATable.BARecEntry[pEntry->BARecWcidArray[j]];
					sprintf(pOutBuf + strlen(pOutBuf), "TID=%d, BAWinSize=%d, LastIndSeq=%d, ReorderingPkts=%d\n", j, pRecBAEntry->BAWinSize, pRecBAEntry->LastIndSeq, pRecBAEntry->list.qlen);
				}
			}
			sprintf(pOutBuf, "%s\n", pOutBuf);

			sprintf(pOutBuf, "%s[Originator]\n", pOutBuf);
			for (j=0; j < NUM_OF_TID; j++)
			{
				if (pEntry->BAOriWcidArray[j] != 0)
				{
					pOriBAEntry =&pAd->BATable.BAOriEntry[pEntry->BAOriWcidArray[j]];
					sprintf(pOutBuf + strlen(pOutBuf), "TID=%d, BAWinSize=%d, StartSeq=%d, CurTxSeq=%d\n", j, pOriBAEntry->BAWinSize, pOriBAEntry->Sequence, pEntry->TxSeq[j]);
				}
			}
			sprintf(pOutBuf, "%s\n\n", pOutBuf);
		}
        if (strlen(pOutBuf) > (IW_PRIV_SIZE_MASK - 30))
                break;
	}

	return;
}

static int
rt_private_show(struct net_device *dev, struct iw_request_info *info,
		struct iw_point *wrq, char *extra)
{
    INT				Status = 0;
    PRTMP_ADAPTER pAd = dev->ml_priv;
    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
    u32             subcmd = wrq->flags;

    if (extra == NULL)
    {
        wrq->length = 0;
        return -EIO;
    }
    memset(extra, 0x00, IW_PRIV_SIZE_MASK);

	{
		pObj->ioctl_if_type = INT_MAIN;
        pObj->ioctl_if = MAIN_MBSSID;
	}

    switch(subcmd)
    {

        case SHOW_CONN_STATUS:
            if (MONITOR_ON(pAd))
            {
                if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED &&
                    pAd->CommonCfg.RegTransmitSetting.field.BW)
                    sprintf(extra, "Monitor Mode(CentralChannel %d)\n", pAd->CommonCfg.CentralChannel);
                else
                    sprintf(extra, "Monitor Mode(Channel %d)\n", pAd->CommonCfg.Channel);
            }
            else
            {
                if (pAd->IndicateMediaState == NdisMediaStateConnected)
            	{
            	    if (INFRA_ON(pAd))
                    {
                    sprintf(extra, "Connected(AP: %s[%02X:%02X:%02X:%02X:%02X:%02X])\n",
                                    pAd->CommonCfg.Ssid,
                                    pAd->CommonCfg.Bssid[0],
                                    pAd->CommonCfg.Bssid[1],
                                    pAd->CommonCfg.Bssid[2],
                                    pAd->CommonCfg.Bssid[3],
                                    pAd->CommonCfg.Bssid[4],
                                    pAd->CommonCfg.Bssid[5]);
            		DBGPRINT(RT_DEBUG_TRACE ,("Ssid=%s ,Ssidlen = %d\n",pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen));
            	}
                    else if (ADHOC_ON(pAd))
                        sprintf(extra, "Connected\n");
            	}
            	else
            	{
            	    sprintf(extra, "Disconnected\n");
            		DBGPRINT(RT_DEBUG_TRACE ,("ConnStatus is not connected\n"));
            	}
            }
            wrq->length = strlen(extra) + 1; 
            break;
        case SHOW_DRVIER_VERION:
            sprintf(extra, "Driver version-%s, %s %s\n", STA_DRIVER_VERSION, __DATE__, __TIME__ );
            wrq->length = strlen(extra) + 1; 
            break;
        case SHOW_BA_INFO:
            getBaInfo(pAd, extra);
            wrq->length = strlen(extra) + 1; 
            break;
		case SHOW_DESC_INFO:
			{
				Show_DescInfo_Proc(pAd, NULL);
				wrq->length = 0; 
			}
			break;
        case RAIO_OFF:
            if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
            {
                sprintf(extra, "Scanning\n");
                wrq->length = strlen(extra) + 1; 
                break;
            }
            pAd->StaCfg.bSwRadio = FALSE;
            if (pAd->StaCfg.bRadio != (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio))
            {
                pAd->StaCfg.bRadio = (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio);
                if (pAd->StaCfg.bRadio == FALSE)
                {
                    MlmeRadioOff(pAd);
                    
					pAd->ExtraInfo = SW_RADIO_OFF;
                }
            }
            sprintf(extra, "Radio Off\n");
            wrq->length = strlen(extra) + 1; 
            break;
        case RAIO_ON:
#ifdef RT2870
            if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
            {
                sprintf(extra, "Scanning\n");
                wrq->length = strlen(extra) + 1; 
                break;
            }
#endif
            pAd->StaCfg.bSwRadio = TRUE;
            
            {
                pAd->StaCfg.bRadio = (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio);
                if (pAd->StaCfg.bRadio == TRUE)
                {
                    MlmeRadioOn(pAd);
                    
					pAd->ExtraInfo = EXTRA_INFO_CLEAR;
                }
            }
            sprintf(extra, "Radio On\n");
            wrq->length = strlen(extra) + 1; 
            break;

		case SHOW_CFG_VALUE:
			{
				Status = RTMPShowCfgValue(pAd, wrq->pointer, extra);
				if (Status == 0)
					wrq->length = strlen(extra) + 1; 
			}
			break;
        default:
            DBGPRINT(RT_DEBUG_TRACE, ("%s - unknow subcmd = %d\n", __func__, subcmd));
            break;
    }

    return Status;
}

int rt_ioctl_siwmlme(struct net_device *dev,
			   struct iw_request_info *info,
			   union iwreq_data *wrqu,
			   char *extra)
{
	PRTMP_ADAPTER   pAd = dev->ml_priv;
	struct iw_mlme *pMlme = (struct iw_mlme *)wrqu->data.pointer;
	MLME_QUEUE_ELEM				MsgElem;
	MLME_DISASSOC_REQ_STRUCT	DisAssocReq;
	MLME_DEAUTH_REQ_STRUCT      DeAuthReq;

	DBGPRINT(RT_DEBUG_TRACE, ("====> %s\n", __func__));

	if (pMlme == NULL)
		return -EINVAL;

	switch(pMlme->cmd)
	{
#ifdef IW_MLME_DEAUTH
		case IW_MLME_DEAUTH:
			DBGPRINT(RT_DEBUG_TRACE, ("====> %s - IW_MLME_DEAUTH\n", __func__));
			COPY_MAC_ADDR(DeAuthReq.Addr, pAd->CommonCfg.Bssid);
			DeAuthReq.Reason = pMlme->reason_code;
			MsgElem.MsgLen = sizeof(MLME_DEAUTH_REQ_STRUCT);
			NdisMoveMemory(MsgElem.Msg, &DeAuthReq, sizeof(MLME_DEAUTH_REQ_STRUCT));
			MlmeDeauthReqAction(pAd, &MsgElem);
			if (INFRA_ON(pAd))
			{
			    LinkDown(pAd, FALSE);
			    pAd->Mlme.AssocMachine.CurrState = ASSOC_IDLE;
			}
			break;
#endif 
#ifdef IW_MLME_DISASSOC
		case IW_MLME_DISASSOC:
			DBGPRINT(RT_DEBUG_TRACE, ("====> %s - IW_MLME_DISASSOC\n", __func__));
			COPY_MAC_ADDR(DisAssocReq.Addr, pAd->CommonCfg.Bssid);
			DisAssocReq.Reason =  pMlme->reason_code;

			MsgElem.Machine = ASSOC_STATE_MACHINE;
			MsgElem.MsgType = MT2_MLME_DISASSOC_REQ;
			MsgElem.MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
			NdisMoveMemory(MsgElem.Msg, &DisAssocReq, sizeof(MLME_DISASSOC_REQ_STRUCT));

			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_DISASSOC;
			MlmeDisassocReqAction(pAd, &MsgElem);
			break;
#endif 
		default:
			DBGPRINT(RT_DEBUG_TRACE, ("====> %s - Unknow Command\n", __func__));
			break;
	}

	return 0;
}

int rt_ioctl_siwauth(struct net_device *dev,
			  struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	PRTMP_ADAPTER   pAdapter = dev->ml_priv;
	struct iw_param *param = &wrqu->param;

    
	if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
  		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
    	return -ENETDOWN;
	}
	switch (param->flags & IW_AUTH_INDEX) {
    	case IW_AUTH_WPA_VERSION:
            if (param->value == IW_AUTH_WPA_VERSION_WPA)
            {
                pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeWPAPSK;
				if (pAdapter->StaCfg.BssType == BSS_ADHOC)
					pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeWPANone;
            }
            else if (param->value == IW_AUTH_WPA_VERSION_WPA2)
                pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeWPA2PSK;

            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_WPA_VERSION - param->value = %d!\n", __func__, param->value));
            break;
    	case IW_AUTH_CIPHER_PAIRWISE:
            if (param->value == IW_AUTH_CIPHER_NONE)
            {
                pAdapter->StaCfg.WepStatus = Ndis802_11WEPDisabled;
                pAdapter->StaCfg.OrigWepStatus = pAdapter->StaCfg.WepStatus;
                pAdapter->StaCfg.PairCipher = Ndis802_11WEPDisabled;
            }
            else if (param->value == IW_AUTH_CIPHER_WEP40 ||
                     param->value == IW_AUTH_CIPHER_WEP104)
            {
                pAdapter->StaCfg.WepStatus = Ndis802_11WEPEnabled;
                pAdapter->StaCfg.OrigWepStatus = pAdapter->StaCfg.WepStatus;
                pAdapter->StaCfg.PairCipher = Ndis802_11WEPEnabled;
                pAdapter->StaCfg.IEEE8021X = FALSE;
            }
            else if (param->value == IW_AUTH_CIPHER_TKIP)
            {
                pAdapter->StaCfg.WepStatus = Ndis802_11Encryption2Enabled;
                pAdapter->StaCfg.OrigWepStatus = pAdapter->StaCfg.WepStatus;
                pAdapter->StaCfg.PairCipher = Ndis802_11Encryption2Enabled;
            }
            else if (param->value == IW_AUTH_CIPHER_CCMP)
            {
                pAdapter->StaCfg.WepStatus = Ndis802_11Encryption3Enabled;
                pAdapter->StaCfg.OrigWepStatus = pAdapter->StaCfg.WepStatus;
                pAdapter->StaCfg.PairCipher = Ndis802_11Encryption3Enabled;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_CIPHER_PAIRWISE - param->value = %d!\n", __func__, param->value));
            break;
    	case IW_AUTH_CIPHER_GROUP:
            if (param->value == IW_AUTH_CIPHER_NONE)
            {
                pAdapter->StaCfg.GroupCipher = Ndis802_11WEPDisabled;
            }
            else if (param->value == IW_AUTH_CIPHER_WEP40 ||
                     param->value == IW_AUTH_CIPHER_WEP104)
            {
                pAdapter->StaCfg.GroupCipher = Ndis802_11WEPEnabled;
            }
            else if (param->value == IW_AUTH_CIPHER_TKIP)
            {
                pAdapter->StaCfg.GroupCipher = Ndis802_11Encryption2Enabled;
            }
            else if (param->value == IW_AUTH_CIPHER_CCMP)
            {
                pAdapter->StaCfg.GroupCipher = Ndis802_11Encryption3Enabled;
            }
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_CIPHER_GROUP - param->value = %d!\n", __func__, param->value));
            break;
    	case IW_AUTH_KEY_MGMT:
            if (param->value == IW_AUTH_KEY_MGMT_802_1X)
            {
                if (pAdapter->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK)
                {
                    pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeWPA;
                    pAdapter->StaCfg.IEEE8021X = FALSE;
                }
                else if (pAdapter->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)
                {
                    pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeWPA2;
                    pAdapter->StaCfg.IEEE8021X = FALSE;
                }
                else
                    
                    pAdapter->StaCfg.IEEE8021X = TRUE;
            }
            else if (param->value == 0)
            {
				STA_PORT_SECURED(pAdapter);
            }
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_KEY_MGMT - param->value = %d!\n", __func__, param->value));
            break;
    	case IW_AUTH_RX_UNENCRYPTED_EAPOL:
            break;
    	case IW_AUTH_PRIVACY_INVOKED:
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_PRIVACY_INVOKED - param->value = %d!\n", __func__, param->value));
    		break;
    	case IW_AUTH_DROP_UNENCRYPTED:
            if (param->value != 0)
                pAdapter->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			else
			{
				STA_PORT_SECURED(pAdapter);
			}
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_WPA_VERSION - param->value = %d!\n", __func__, param->value));
    		break;
    	case IW_AUTH_80211_AUTH_ALG:
			if (param->value & IW_AUTH_ALG_SHARED_KEY)
            {
				pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeShared;
			}
            else if (param->value & IW_AUTH_ALG_OPEN_SYSTEM)
            {
				pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeOpen;
			}
            else
				return -EINVAL;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_80211_AUTH_ALG - param->value = %d!\n", __func__, param->value));
			break;
    	case IW_AUTH_WPA_ENABLED:
    		DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_AUTH_WPA_ENABLED - Driver supports WPA!(param->value = %d)\n", __func__, param->value));
    		break;
    	default:
    		return -EOPNOTSUPP;
}

	return 0;
}

int rt_ioctl_giwauth(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	PRTMP_ADAPTER   pAdapter = dev->ml_priv;
	struct iw_param *param = &wrqu->param;

    
	if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
  		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
    	return -ENETDOWN;
    }

	switch (param->flags & IW_AUTH_INDEX) {
	case IW_AUTH_DROP_UNENCRYPTED:
        param->value = (pAdapter->StaCfg.WepStatus == Ndis802_11WEPDisabled) ? 0 : 1;
		break;

	case IW_AUTH_80211_AUTH_ALG:
        param->value = (pAdapter->StaCfg.AuthMode == Ndis802_11AuthModeShared) ? IW_AUTH_ALG_SHARED_KEY : IW_AUTH_ALG_OPEN_SYSTEM;
		break;

	case IW_AUTH_WPA_ENABLED:
		param->value = (pAdapter->StaCfg.AuthMode >= Ndis802_11AuthModeWPA) ? 1 : 0;
		break;

	default:
		return -EOPNOTSUPP;
	}
    DBGPRINT(RT_DEBUG_TRACE, ("rt_ioctl_giwauth::param->value = %d!\n", param->value));
	return 0;
}

void fnSetCipherKey(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  INT             keyIdx,
    IN  UCHAR           CipherAlg,
    IN  BOOLEAN         bGTK,
    IN  struct iw_encode_ext *ext)
{
#ifdef RT2860
	RTMP_CLEAR_PSFLAG(pAdapter, fRTMP_PS_CAN_GO_SLEEP);
	if (RTMP_TEST_PSFLAG(pAdapter, fRTMP_PS_SET_PCI_CLK_OFF_COMMAND))
	{
		if (pAdapter->StaCfg.bRadio == FALSE)
		{
			RTMP_SET_PSFLAG(pAdapter, fRTMP_PS_CAN_GO_SLEEP);
			return;
		}
		DBGPRINT(RT_DEBUG_TRACE,("RTMPWPAAddKeyProc1==>\n"));
		RTMPPCIeLinkCtrlValueRestore(pAdapter, RESTORE_HALT);
		RTMPusecDelay(6000);
		pAdapter->bPCIclkOff = FALSE;
	}
#endif
    NdisZeroMemory(&pAdapter->SharedKey[BSS0][keyIdx], sizeof(CIPHER_KEY));
    pAdapter->SharedKey[BSS0][keyIdx].KeyLen = LEN_TKIP_EK;
    NdisMoveMemory(pAdapter->SharedKey[BSS0][keyIdx].Key, ext->key, LEN_TKIP_EK);
    NdisMoveMemory(pAdapter->SharedKey[BSS0][keyIdx].TxMic, ext->key + LEN_TKIP_EK, LEN_TKIP_TXMICK);
    NdisMoveMemory(pAdapter->SharedKey[BSS0][keyIdx].RxMic, ext->key + LEN_TKIP_EK + LEN_TKIP_TXMICK, LEN_TKIP_RXMICK);
    pAdapter->SharedKey[BSS0][keyIdx].CipherAlg = CipherAlg;

    
	AsicAddSharedKeyEntry(pAdapter,
						  BSS0,
						  keyIdx,
						  pAdapter->SharedKey[BSS0][keyIdx].CipherAlg,
						  pAdapter->SharedKey[BSS0][keyIdx].Key,
						  pAdapter->SharedKey[BSS0][keyIdx].TxMic,
						  pAdapter->SharedKey[BSS0][keyIdx].RxMic);

    if (bGTK)
        
    	RTMPAddWcidAttributeEntry(pAdapter,
    							  BSS0,
    							  keyIdx,
    							  pAdapter->SharedKey[BSS0][keyIdx].CipherAlg,
    							  NULL);
    else
        
    	RTMPAddWcidAttributeEntry(pAdapter,
    							  BSS0,
    							  keyIdx,
    							  pAdapter->SharedKey[BSS0][keyIdx].CipherAlg,
    							  &pAdapter->MacTab.Content[BSSID_WCID]);
#ifdef RT2860
	RTMP_SET_PSFLAG(pAdapter, fRTMP_PS_CAN_GO_SLEEP);
#endif
}

int rt_ioctl_siwencodeext(struct net_device *dev,
			   struct iw_request_info *info,
			   union iwreq_data *wrqu,
			   char *extra)
			{
    PRTMP_ADAPTER   pAdapter = dev->ml_priv;
	struct iw_point *encoding = &wrqu->encoding;
	struct iw_encode_ext *ext = (struct iw_encode_ext *)extra;
    int keyIdx, alg = ext->alg;

    
	if(!RTMP_TEST_FLAG(pAdapter, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
  		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
    	return -ENETDOWN;
	}

    if (encoding->flags & IW_ENCODE_DISABLED)
	{
        keyIdx = (encoding->flags & IW_ENCODE_INDEX) - 1;
        
	    AsicRemovePairwiseKeyEntry(pAdapter, BSS0, BSSID_WCID);
        pAdapter->SharedKey[BSS0][keyIdx].KeyLen = 0;
		pAdapter->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_NONE;
		AsicRemoveSharedKeyEntry(pAdapter, 0, (UCHAR)keyIdx);
        NdisZeroMemory(&pAdapter->SharedKey[BSS0][keyIdx], sizeof(CIPHER_KEY));
        DBGPRINT(RT_DEBUG_TRACE, ("%s::Remove all keys!(encoding->flags = %x)\n", __func__, encoding->flags));
    }
					else
    {
        
    	keyIdx = (encoding->flags & IW_ENCODE_INDEX) - 1;
    	if((keyIdx < 0) || (keyIdx >= NR_WEP_KEYS))
    		return -EINVAL;

        if (ext->ext_flags & IW_ENCODE_EXT_SET_TX_KEY)
        {
            pAdapter->StaCfg.DefaultKeyId = keyIdx;
            DBGPRINT(RT_DEBUG_TRACE, ("%s::DefaultKeyId = %d\n", __func__, pAdapter->StaCfg.DefaultKeyId));
        }

        switch (alg) {
    		case IW_ENCODE_ALG_NONE:
                DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_ENCODE_ALG_NONE\n", __func__));
    			break;
    		case IW_ENCODE_ALG_WEP:
                DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_ENCODE_ALG_WEP - ext->key_len = %d, keyIdx = %d\n", __func__, ext->key_len, keyIdx));
    			if (ext->key_len == MAX_WEP_KEY_SIZE)
                {
        			pAdapter->SharedKey[BSS0][keyIdx].KeyLen = MAX_WEP_KEY_SIZE;
                    pAdapter->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_WEP128;
				}
        		else if (ext->key_len == MIN_WEP_KEY_SIZE)
                {
                    pAdapter->SharedKey[BSS0][keyIdx].KeyLen = MIN_WEP_KEY_SIZE;
                    pAdapter->SharedKey[BSS0][keyIdx].CipherAlg = CIPHER_WEP64;
			}
        		else
                    return -EINVAL;

                NdisZeroMemory(pAdapter->SharedKey[BSS0][keyIdx].Key,  16);
			    NdisMoveMemory(pAdapter->SharedKey[BSS0][keyIdx].Key, ext->key, ext->key_len);
				if (pAdapter->StaCfg.GroupCipher == Ndis802_11GroupWEP40Enabled ||
					pAdapter->StaCfg.GroupCipher == Ndis802_11GroupWEP104Enabled)
				{
					
					AsicAddSharedKeyEntry(pAdapter, BSS0, keyIdx, pAdapter->SharedKey[BSS0][keyIdx].CipherAlg, pAdapter->SharedKey[BSS0][keyIdx].Key, NULL, NULL);

					
					RTMPAddWcidAttributeEntry(pAdapter, BSS0, keyIdx, pAdapter->SharedKey[BSS0][keyIdx].CipherAlg, NULL);

					STA_PORT_SECURED(pAdapter);

    				
    				pAdapter->IndicateMediaState = NdisMediaStateConnected;
				}
    			break;
            case IW_ENCODE_ALG_TKIP:
                DBGPRINT(RT_DEBUG_TRACE, ("%s::IW_ENCODE_ALG_TKIP - keyIdx = %d, ext->key_len = %d\n", __func__, keyIdx, ext->key_len));
                if (ext->key_len == 32)
                {
                    if (ext->ext_flags & IW_ENCODE_EXT_SET_TX_KEY)
                    {
                        fnSetCipherKey(pAdapter, keyIdx, CIPHER_TKIP, FALSE, ext);
                        if (pAdapter->StaCfg.AuthMode >= Ndis802_11AuthModeWPA2)
                        {
                            STA_PORT_SECURED(pAdapter);
                        }
		}
                    else if (ext->ext_flags & IW_ENCODE_EXT_GROUP_KEY)
                    {
                        fnSetCipherKey(pAdapter, keyIdx, CIPHER_TKIP, TRUE, ext);

                        
            	        STA_PORT_SECURED(pAdapter);
                    }
                }
                else
                    return -EINVAL;
                break;
            case IW_ENCODE_ALG_CCMP:
                if (ext->ext_flags & IW_ENCODE_EXT_SET_TX_KEY)
		{
                    fnSetCipherKey(pAdapter, keyIdx, CIPHER_AES, FALSE, ext);
                    if (pAdapter->StaCfg.AuthMode >= Ndis802_11AuthModeWPA2)
                    	STA_PORT_SECURED(pAdapter);
                }
                else if (ext->ext_flags & IW_ENCODE_EXT_GROUP_KEY)
                {
                    fnSetCipherKey(pAdapter, keyIdx, CIPHER_AES, TRUE, ext);

                    
        	        STA_PORT_SECURED(pAdapter);
                }
                break;
    		default:
    			return -EINVAL;
		}
    }

    return 0;
}

int
rt_ioctl_giwencodeext(struct net_device *dev,
			  struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	PRTMP_ADAPTER pAd = dev->ml_priv;
	PCHAR pKey = NULL;
	struct iw_point *encoding = &wrqu->encoding;
	struct iw_encode_ext *ext = (struct iw_encode_ext *)extra;
	int idx, max_key_len;

	DBGPRINT(RT_DEBUG_TRACE ,("===> rt_ioctl_giwencodeext\n"));

	max_key_len = encoding->length - sizeof(*ext);
	if (max_key_len < 0)
		return -EINVAL;

	idx = encoding->flags & IW_ENCODE_INDEX;
	if (idx)
	{
		if (idx < 1 || idx > 4)
			return -EINVAL;
		idx--;

		if ((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) ||
			(pAd->StaCfg.WepStatus == Ndis802_11Encryption3Enabled))
		{
			if (idx != pAd->StaCfg.DefaultKeyId)
			{
				ext->key_len = 0;
				return 0;
			}
		}
	}
	else
		idx = pAd->StaCfg.DefaultKeyId;

	encoding->flags = idx + 1;
	memset(ext, 0, sizeof(*ext));

	ext->key_len = 0;
	switch(pAd->StaCfg.WepStatus) {
		case Ndis802_11WEPDisabled:
			ext->alg = IW_ENCODE_ALG_NONE;
			encoding->flags |= IW_ENCODE_DISABLED;
			break;
		case Ndis802_11WEPEnabled:
			ext->alg = IW_ENCODE_ALG_WEP;
			if (pAd->SharedKey[BSS0][idx].KeyLen > max_key_len)
				return -E2BIG;
			else
			{
				ext->key_len = pAd->SharedKey[BSS0][idx].KeyLen;
				pKey = &(pAd->SharedKey[BSS0][idx].Key[0]);
			}
			break;
		case Ndis802_11Encryption2Enabled:
		case Ndis802_11Encryption3Enabled:
			if (pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled)
				ext->alg = IW_ENCODE_ALG_TKIP;
			else
				ext->alg = IW_ENCODE_ALG_CCMP;

			if (max_key_len < 32)
				return -E2BIG;
			else
			{
				ext->key_len = 32;
				pKey = &pAd->StaCfg.PMK[0];
			}
			break;
		default:
			return -EINVAL;
	}

	if (ext->key_len && pKey)
	{
		encoding->flags |= IW_ENCODE_ENABLED;
		memcpy(ext->key, pKey, ext->key_len);
	}

	return 0;
}

int rt_ioctl_siwgenie(struct net_device *dev,
			  struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	PRTMP_ADAPTER   pAd = dev->ml_priv;

	if (wrqu->data.length > MAX_LEN_OF_RSNIE ||
	    (wrqu->data.length && extra == NULL))
		return -EINVAL;

	if (wrqu->data.length)
	{
		pAd->StaCfg.RSNIE_Len = wrqu->data.length;
		NdisMoveMemory(&pAd->StaCfg.RSN_IE[0], extra, pAd->StaCfg.RSNIE_Len);
	}
	else
	{
		pAd->StaCfg.RSNIE_Len = 0;
		NdisZeroMemory(&pAd->StaCfg.RSN_IE[0], MAX_LEN_OF_RSNIE);
	}

	return 0;
}

int rt_ioctl_giwgenie(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	PRTMP_ADAPTER   pAd = dev->ml_priv;

	if ((pAd->StaCfg.RSNIE_Len == 0) ||
		(pAd->StaCfg.AuthMode < Ndis802_11AuthModeWPA))
	{
		wrqu->data.length = 0;
		return 0;
	}

	if (pAd->StaCfg.WpaSupplicantUP == WPA_SUPPLICANT_ENABLE)
	{
	if (wrqu->data.length < pAd->StaCfg.RSNIE_Len)
		return -E2BIG;

	wrqu->data.length = pAd->StaCfg.RSNIE_Len;
	memcpy(extra, &pAd->StaCfg.RSN_IE[0], pAd->StaCfg.RSNIE_Len);
	}
	else
	{
		UCHAR RSNIe = IE_WPA;

		if (wrqu->data.length < (pAd->StaCfg.RSNIE_Len + 2)) 
			return -E2BIG;
		wrqu->data.length = pAd->StaCfg.RSNIE_Len + 2;

		if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK) ||
            (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2))
			RSNIe = IE_RSN;

		extra[0] = (char)RSNIe;
		extra[1] = pAd->StaCfg.RSNIE_Len;
		memcpy(extra+2, &pAd->StaCfg.RSN_IE[0], pAd->StaCfg.RSNIE_Len);
	}

	return 0;
}

int rt_ioctl_siwpmksa(struct net_device *dev,
			   struct iw_request_info *info,
			   union iwreq_data *wrqu,
			   char *extra)
{
	PRTMP_ADAPTER   pAd = dev->ml_priv;
	struct iw_pmksa *pPmksa = (struct iw_pmksa *)wrqu->data.pointer;
	INT	CachedIdx = 0, idx = 0;

	if (pPmksa == NULL)
		return -EINVAL;

	DBGPRINT(RT_DEBUG_TRACE ,("===> rt_ioctl_siwpmksa\n"));
	switch(pPmksa->cmd)
	{
		case IW_PMKSA_FLUSH:
			NdisZeroMemory(pAd->StaCfg.SavedPMK, sizeof(BSSID_INFO)*PMKID_NO);
			DBGPRINT(RT_DEBUG_TRACE ,("rt_ioctl_siwpmksa - IW_PMKSA_FLUSH\n"));
			break;
		case IW_PMKSA_REMOVE:
			for (CachedIdx = 0; CachedIdx < pAd->StaCfg.SavedPMKNum; CachedIdx++)
			{
		        
		        if (NdisEqualMemory(pPmksa->bssid.sa_data, pAd->StaCfg.SavedPMK[CachedIdx].BSSID, MAC_ADDR_LEN))
		        {
		        	NdisZeroMemory(pAd->StaCfg.SavedPMK[CachedIdx].BSSID, MAC_ADDR_LEN);
					NdisZeroMemory(pAd->StaCfg.SavedPMK[CachedIdx].PMKID, 16);
					for (idx = CachedIdx; idx < (pAd->StaCfg.SavedPMKNum - 1); idx++)
					{
						NdisMoveMemory(&pAd->StaCfg.SavedPMK[idx].BSSID[0], &pAd->StaCfg.SavedPMK[idx+1].BSSID[0], MAC_ADDR_LEN);
						NdisMoveMemory(&pAd->StaCfg.SavedPMK[idx].PMKID[0], &pAd->StaCfg.SavedPMK[idx+1].PMKID[0], 16);
					}
					pAd->StaCfg.SavedPMKNum--;
			        break;
		        }
	        }

			DBGPRINT(RT_DEBUG_TRACE ,("rt_ioctl_siwpmksa - IW_PMKSA_REMOVE\n"));
			break;
		case IW_PMKSA_ADD:
			for (CachedIdx = 0; CachedIdx < pAd->StaCfg.SavedPMKNum; CachedIdx++)
			{
		        
		        if (NdisEqualMemory(pPmksa->bssid.sa_data, pAd->StaCfg.SavedPMK[CachedIdx].BSSID, MAC_ADDR_LEN))
			        break;
	        }

	        
	        if (CachedIdx < PMKID_NO)
	        {
		        DBGPRINT(RT_DEBUG_OFF, ("Update PMKID, idx = %d\n", CachedIdx));
		        NdisMoveMemory(&pAd->StaCfg.SavedPMK[CachedIdx].BSSID[0], pPmksa->bssid.sa_data, MAC_ADDR_LEN);
				NdisMoveMemory(&pAd->StaCfg.SavedPMK[CachedIdx].PMKID[0], pPmksa->pmkid, 16);
		        pAd->StaCfg.SavedPMKNum++;
	        }
	        
	        else
	        {
		        
		        CachedIdx = (pPmksa->bssid.sa_data[5] % PMKID_NO);
		        DBGPRINT(RT_DEBUG_OFF, ("Update PMKID, idx = %d\n", CachedIdx));
		        NdisMoveMemory(&pAd->StaCfg.SavedPMK[CachedIdx].BSSID[0], pPmksa->bssid.sa_data, MAC_ADDR_LEN);
				NdisMoveMemory(&pAd->StaCfg.SavedPMK[CachedIdx].PMKID[0], pPmksa->pmkid, 16);
	        }

			DBGPRINT(RT_DEBUG_TRACE ,("rt_ioctl_siwpmksa - IW_PMKSA_ADD\n"));
			break;
		default:
			DBGPRINT(RT_DEBUG_TRACE ,("rt_ioctl_siwpmksa - Unknow Command!!\n"));
			break;
	}

	return 0;
}

int rt_ioctl_siwrate(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
    PRTMP_ADAPTER   pAd = dev->ml_priv;
    UINT32          rate = wrqu->bitrate.value, fixed = wrqu->bitrate.fixed;

    
	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
  		DBGPRINT(RT_DEBUG_TRACE, ("rt_ioctl_siwrate::Network is down!\n"));
    	return -ENETDOWN;
	}

    DBGPRINT(RT_DEBUG_TRACE, ("rt_ioctl_siwrate::(rate = %d, fixed = %d)\n", rate, fixed));
    
    if (rate == -1)
    {
		
		pAd->StaCfg.DesiredTransmitSetting.field.MCS = MCS_AUTO;
		pAd->StaCfg.bAutoTxRateSwitch = TRUE;
		if ((pAd->CommonCfg.PhyMode <= PHY_11G) ||
		    (pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE <= MODE_OFDM))
			RTMPSetDesiredRates(pAd, -1);

		SetCommonHT(pAd);
    }
    else
    {
        if (fixed)
        {
        	pAd->StaCfg.bAutoTxRateSwitch = FALSE;
            if ((pAd->CommonCfg.PhyMode <= PHY_11G) ||
                (pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE <= MODE_OFDM))
                RTMPSetDesiredRates(pAd, rate);
            else
            {
                pAd->StaCfg.DesiredTransmitSetting.field.MCS = MCS_AUTO;
                SetCommonHT(pAd);
            }
            DBGPRINT(RT_DEBUG_TRACE, ("rt_ioctl_siwrate::(HtMcs=%d)\n",pAd->StaCfg.DesiredTransmitSetting.field.MCS));
        }
        else
        {
            
            return -EOPNOTSUPP;
        }
    }

    return 0;
}

int rt_ioctl_giwrate(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
    PRTMP_ADAPTER   pAd = dev->ml_priv;
    int rate_index = 0, rate_count = 0;
    HTTRANSMIT_SETTING ht_setting;
    __s32 ralinkrate[] =
	{2,  4,   11,  22, 
	12, 18,   24,  36, 48, 72, 96, 108, 
	13, 26,   39,  52,  78, 104, 117, 130, 26,  52,  78, 104, 156, 208, 234, 260, 
	39, 78,  117, 156, 234, 312, 351, 390,										  
	27, 54,   81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540, 
	81, 162, 243, 324, 486, 648, 729, 810,										  
	14, 29,   43,  57,  87, 115, 130, 144, 29, 59,   87, 115, 173, 230, 260, 288, 
	43, 87,  130, 173, 260, 317, 390, 433,										  
	30, 60,   90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600, 
	90, 180, 270, 360, 540, 720, 810, 900};										  

    rate_count = sizeof(ralinkrate)/sizeof(__s32);
    
	if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
  		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
    	return -ENETDOWN;
	}

    if ((pAd->StaCfg.bAutoTxRateSwitch == FALSE) &&
        (INFRA_ON(pAd)) &&
        ((pAd->CommonCfg.PhyMode <= PHY_11G) || (pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE <= MODE_OFDM)))
        ht_setting.word = pAd->StaCfg.HTPhyMode.word;
    else
        ht_setting.word = pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word;

    if (ht_setting.field.MODE >= MODE_HTMIX)
    {
    	rate_index = 12 + ((UCHAR)ht_setting.field.BW *24) + ((UCHAR)ht_setting.field.ShortGI *48) + ((UCHAR)ht_setting.field.MCS);
    }
    else
    if (ht_setting.field.MODE == MODE_OFDM)
    	rate_index = (UCHAR)(ht_setting.field.MCS) + 4;
    else if (ht_setting.field.MODE == MODE_CCK)
    	rate_index = (UCHAR)(ht_setting.field.MCS);

    if (rate_index < 0)
        rate_index = 0;

    if (rate_index > rate_count)
        rate_index = rate_count;

    wrqu->bitrate.value = ralinkrate[rate_index] * 500000;
    wrqu->bitrate.disabled = 0;

    return 0;
}

static const iw_handler rt_handler[] =
{
	(iw_handler) NULL,			            
	(iw_handler) rt_ioctl_giwname,			
	(iw_handler) NULL,			            
	(iw_handler) NULL,			            
	(iw_handler) rt_ioctl_siwfreq,		    
	(iw_handler) rt_ioctl_giwfreq,		    
	(iw_handler) rt_ioctl_siwmode,		    
	(iw_handler) rt_ioctl_giwmode,		    
	(iw_handler) NULL,		                
	(iw_handler) NULL,		                
	(iw_handler) NULL ,		
	(iw_handler) rt_ioctl_giwrange,		    
	(iw_handler) NULL ,		
	(iw_handler) NULL ,    
	(iw_handler) NULL ,		
	(iw_handler) rt28xx_get_wireless_stats ,    
	(iw_handler) NULL,		                
	(iw_handler) NULL,		                
	(iw_handler) NULL,				        
	(iw_handler) NULL,				        
	(iw_handler) rt_ioctl_siwap,            
	(iw_handler) rt_ioctl_giwap,		    
	(iw_handler) rt_ioctl_siwmlme,	        
	(iw_handler) rt_ioctl_iwaplist,		    
	(iw_handler) rt_ioctl_siwscan,		    
	(iw_handler) rt_ioctl_giwscan,		    
	(iw_handler) rt_ioctl_siwessid,		    
	(iw_handler) rt_ioctl_giwessid,		    
	(iw_handler) rt_ioctl_siwnickn,		    
	(iw_handler) rt_ioctl_giwnickn,		    
	(iw_handler) NULL,				        
	(iw_handler) NULL,				        
	(iw_handler) rt_ioctl_siwrate,          
	(iw_handler) rt_ioctl_giwrate,          
	(iw_handler) rt_ioctl_siwrts,		    
	(iw_handler) rt_ioctl_giwrts,		    
	(iw_handler) rt_ioctl_siwfrag,		    
	(iw_handler) rt_ioctl_giwfrag,		    
	(iw_handler) NULL,		                
	(iw_handler) NULL,		                
	(iw_handler) NULL,		                
	(iw_handler) NULL,		                
	(iw_handler) rt_ioctl_siwencode,		
	(iw_handler) rt_ioctl_giwencode,		
	(iw_handler) NULL,		                
	(iw_handler) NULL,		                
	(iw_handler) NULL,						
	(iw_handler) NULL,						
    (iw_handler) rt_ioctl_siwgenie,         
	(iw_handler) rt_ioctl_giwgenie,         
	(iw_handler) rt_ioctl_siwauth,		    
	(iw_handler) rt_ioctl_giwauth,		    
	(iw_handler) rt_ioctl_siwencodeext,	    
	(iw_handler) rt_ioctl_giwencodeext,		
	(iw_handler) rt_ioctl_siwpmksa,         
};

static const iw_handler rt_priv_handlers[] = {
	(iw_handler) NULL, 
	(iw_handler) NULL, 
	(iw_handler) rt_ioctl_setparam, 
	(iw_handler) NULL, 
	(iw_handler) NULL, 
	(iw_handler) NULL, 
	(iw_handler) NULL, 
	(iw_handler) NULL, 
	(iw_handler) NULL, 
	(iw_handler) rt_private_get_statistics, 
	(iw_handler) NULL, 
	(iw_handler) NULL, 
	(iw_handler) NULL, 
	(iw_handler) NULL, 
	(iw_handler) NULL, 
	(iw_handler) NULL, 
	(iw_handler) NULL, 
	(iw_handler) rt_private_show, 
    (iw_handler) NULL, 
	(iw_handler) NULL, 
	(iw_handler) NULL, 
	(iw_handler) NULL, 
	(iw_handler) NULL, 
};

const struct iw_handler_def rt28xx_iw_handler_def =
{
#define	N(a)	(sizeof (a) / sizeof (a[0]))
	.standard	= (iw_handler *) rt_handler,
	.num_standard	= sizeof(rt_handler) / sizeof(iw_handler),
	.private	= (iw_handler *) rt_priv_handlers,
	.num_private		= N(rt_priv_handlers),
	.private_args	= (struct iw_priv_args *) privtab,
	.num_private_args	= N(privtab),
#if IW_HANDLER_VERSION >= 7
    .get_wireless_stats = rt28xx_get_wireless_stats,
#endif
};

INT rt28xx_sta_ioctl(
	IN	struct net_device	*net_dev,
	IN	OUT	struct ifreq	*rq,
	IN	INT					cmd)
{
	RTMP_ADAPTER *pAd = net_dev->ml_priv;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct iwreq        *wrq = (struct iwreq *) rq;
	BOOLEAN				StateMachineTouched = FALSE;
	INT					Status = NDIS_STATUS_SUCCESS;

    
    if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
    {
        {
            DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
		    return -ENETDOWN;
        }
    }

	{	
		pObj->ioctl_if_type = INT_MAIN;
		pObj->ioctl_if = MAIN_MBSSID;
	}

	switch(cmd)
	{
        case SIOCGIFHWADDR:
			DBGPRINT(RT_DEBUG_TRACE, ("IOCTL::SIOCGIFHWADDR\n"));
			memcpy(wrq->u.name, pAd->CurrentAddress, ETH_ALEN);
			break;
		case SIOCGIWNAME:
        {
        	char *name=&wrq->u.name[0];
        	rt_ioctl_giwname(net_dev, NULL, name, NULL);
			break;
		}
		case SIOCGIWESSID:  
        {
        	struct iw_point *essid=&wrq->u.essid;
        	rt_ioctl_giwessid(net_dev, NULL, essid, essid->pointer);
			break;
		}
		case SIOCSIWESSID:  
        {
        	struct iw_point	*essid=&wrq->u.essid;
        	rt_ioctl_siwessid(net_dev, NULL, essid, essid->pointer);
			break;
		}
		case SIOCSIWNWID:   
		case SIOCGIWNWID:   
			Status = -EOPNOTSUPP;
			break;
		case SIOCSIWFREQ:   
        {
        	struct iw_freq *freq=&wrq->u.freq;
        	rt_ioctl_siwfreq(net_dev, NULL, freq, NULL);
			break;
		}
		case SIOCGIWFREQ:   
        {
        	struct iw_freq *freq=&wrq->u.freq;
        	rt_ioctl_giwfreq(net_dev, NULL, freq, NULL);
			break;
		}
		case SIOCSIWNICKN: 
        {
        	struct iw_point *data=&wrq->u.data;
        	rt_ioctl_siwnickn(net_dev, NULL, data, NULL);
			break;
		}
		case SIOCGIWNICKN: 
        {
        	struct iw_point *data=&wrq->u.data;
        	rt_ioctl_giwnickn(net_dev, NULL, data, NULL);
			break;
		}
		case SIOCGIWRATE:   
		    rt_ioctl_giwrate(net_dev, NULL, &wrq->u, NULL);
            break;
	    case SIOCSIWRATE:  
	        rt_ioctl_siwrate(net_dev, NULL, &wrq->u, NULL);
            break;
        case SIOCGIWRTS:  
        {
        	struct iw_param *rts=&wrq->u.rts;
        	rt_ioctl_giwrts(net_dev, NULL, rts, NULL);
			break;
		}
        case SIOCSIWRTS:  
        {
        	struct iw_param *rts=&wrq->u.rts;
        	rt_ioctl_siwrts(net_dev, NULL, rts, NULL);
			break;
		}
        case SIOCGIWFRAG:  
        {
        	struct iw_param *frag=&wrq->u.frag;
        	rt_ioctl_giwfrag(net_dev, NULL, frag, NULL);
			break;
		}
        case SIOCSIWFRAG:  
        {
        	struct iw_param *frag=&wrq->u.frag;
        	rt_ioctl_siwfrag(net_dev, NULL, frag, NULL);
			break;
		}
        case SIOCGIWENCODE:  
        {
        	struct iw_point *erq=&wrq->u.encoding;
        	if(erq->pointer)
        		rt_ioctl_giwencode(net_dev, NULL, erq, erq->pointer);
			break;
		}
        case SIOCSIWENCODE:  
        {
        	struct iw_point *erq=&wrq->u.encoding;
        	if(erq->pointer)
        		rt_ioctl_siwencode(net_dev, NULL, erq, erq->pointer);
			break;
		}
		case SIOCGIWAP:     
        {
        	struct sockaddr *ap_addr=&wrq->u.ap_addr;
        	rt_ioctl_giwap(net_dev, NULL, ap_addr, ap_addr->sa_data);
			break;
		}
	    case SIOCSIWAP:  
        {
        	struct sockaddr *ap_addr=&wrq->u.ap_addr;
        	rt_ioctl_siwap(net_dev, NULL, ap_addr, ap_addr->sa_data);
			break;
		}
		case SIOCGIWMODE:   
        {
        	__u32 *mode=&wrq->u.mode;
        	rt_ioctl_giwmode(net_dev, NULL, mode, NULL);
			break;
		}
		case SIOCSIWMODE:   
        {
        	__u32 *mode=&wrq->u.mode;
        	rt_ioctl_siwmode(net_dev, NULL, mode, NULL);
			break;
		}
		case SIOCGIWSENS:   
		case SIOCSIWSENS:	
		case SIOCGIWPOWER:  
		case SIOCSIWPOWER:  
		case SIOCGIWTXPOW:  
		case SIOCSIWTXPOW:  
		case SIOCGIWRANGE:	
		case SIOCGIWRETRY:	
		case SIOCSIWRETRY:	
		case RT_PRIV_IOCTL:
		case RT_PRIV_IOCTL_EXT:
			Status = -EOPNOTSUPP;
			break;
		case SIOCGIWPRIV:
			if (wrq->u.data.pointer)
			{
				if ( access_ok(VERIFY_WRITE, wrq->u.data.pointer, sizeof(privtab)) != TRUE)
					break;
				wrq->u.data.length = sizeof(privtab) / sizeof(privtab[0]);
				if (copy_to_user(wrq->u.data.pointer, privtab, sizeof(privtab)))
					Status = -EFAULT;
			}
			break;
		case RTPRIV_IOCTL_SET:
			if(access_ok(VERIFY_READ, wrq->u.data.pointer, wrq->u.data.length) != TRUE)
				break;
			rt_ioctl_setparam(net_dev, NULL, NULL, wrq->u.data.pointer);
			break;
		case RTPRIV_IOCTL_GSITESURVEY:
			RTMPIoctlGetSiteSurvey(pAd, wrq);
		    break;
        case SIOCETHTOOL:
                break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("IOCTL::unknown IOCTL's cmd = 0x%08x\n", cmd));
			Status = -EOPNOTSUPP;
			break;
	}

    if(StateMachineTouched) 
    	RT28XX_MLME_HANDLER(pAd);

	return Status;
}


INT Set_SSID_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg)
{
    NDIS_802_11_SSID                    Ssid, *pSsid=NULL;
    BOOLEAN                             StateMachineTouched = FALSE;
    int                                 success = TRUE;

    if( strlen(arg) <= MAX_LEN_OF_SSID)
    {
        NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
        if (strlen(arg) != 0)
        {
            NdisMoveMemory(Ssid.Ssid, arg, strlen(arg));
            Ssid.SsidLength = strlen(arg);
        }
        else   
        {
            Ssid.SsidLength = 0;
		    memcpy(Ssid.Ssid, "", 0);
			pAdapter->StaCfg.BssType = BSS_INFRA;
			pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeOpen;
	        pAdapter->StaCfg.WepStatus  = Ndis802_11EncryptionDisabled;
		}
        pSsid = &Ssid;

        if (pAdapter->Mlme.CntlMachine.CurrState != CNTL_IDLE)
        {
            RT28XX_MLME_RESET_STATE_MACHINE(pAdapter);
            DBGPRINT(RT_DEBUG_TRACE, ("!!! MLME busy, reset MLME state machine !!!\n"));
        }

        pAdapter->MlmeAux.CurrReqIsFromNdis = TRUE;
        pAdapter->StaCfg.bScanReqIsFromWebUI = FALSE;
		pAdapter->bConfigChanged = TRUE;

        MlmeEnqueue(pAdapter,
                    MLME_CNTL_STATE_MACHINE,
                    OID_802_11_SSID,
                    sizeof(NDIS_802_11_SSID),
                    (VOID *)pSsid);

        StateMachineTouched = TRUE;
        DBGPRINT(RT_DEBUG_TRACE, ("Set_SSID_Proc::(Len=%d,Ssid=%s)\n", Ssid.SsidLength, Ssid.Ssid));
    }
    else
        success = FALSE;

    if (StateMachineTouched) 
    	RT28XX_MLME_HANDLER(pAdapter);

    return success;
}

#ifdef WMM_SUPPORT

INT	Set_WmmCapable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg)
{
	BOOLEAN	bWmmCapable;

	bWmmCapable = simple_strtol(arg, 0, 10);

	if ((bWmmCapable == 1)
#ifdef RT2870
		&& (pAd->NumberOfPipes >= 5)
#endif 
		)
		pAd->CommonCfg.bWmmCapable = TRUE;
	else if (bWmmCapable == 0)
		pAd->CommonCfg.bWmmCapable = FALSE;
	else
		return FALSE;  

	DBGPRINT(RT_DEBUG_TRACE, ("Set_WmmCapable_Proc::(bWmmCapable=%d)\n",
		pAd->CommonCfg.bWmmCapable));

	return TRUE;
}
#endif 


INT Set_NetworkType_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg)
{
    UINT32	Value = 0;

    if (strcmp(arg, "Adhoc") == 0)
	{
		if (pAdapter->StaCfg.BssType != BSS_ADHOC)
		{
			
			pAdapter->bConfigChanged = TRUE;
            if (MONITOR_ON(pAdapter))
            {
                RTMP_IO_WRITE32(pAdapter, RX_FILTR_CFG, STANORMAL);
                RTMP_IO_READ32(pAdapter, MAC_SYS_CTRL, &Value);
				Value &= (~0x80);
				RTMP_IO_WRITE32(pAdapter, MAC_SYS_CTRL, Value);
                OPSTATUS_CLEAR_FLAG(pAdapter, fOP_STATUS_MEDIA_STATE_CONNECTED);
                pAdapter->StaCfg.bAutoReconnect = TRUE;
                LinkDown(pAdapter, FALSE);
            }
			if (INFRA_ON(pAdapter))
			{
				
				
				
				pAdapter->MlmeAux.AutoReconnectSsidLen= 32;
				NdisZeroMemory(pAdapter->MlmeAux.AutoReconnectSsid, pAdapter->MlmeAux.AutoReconnectSsidLen);

				LinkDown(pAdapter, FALSE);

				DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_MEDIA_DISCONNECT Event BB!\n"));
			}
		}
		pAdapter->StaCfg.BssType = BSS_ADHOC;
        pAdapter->net_dev->type = pAdapter->StaCfg.OriDevType;
		DBGPRINT(RT_DEBUG_TRACE, ("===>Set_NetworkType_Proc::(AD-HOC)\n"));
	}
    else if (strcmp(arg, "Infra") == 0)
	{
		if (pAdapter->StaCfg.BssType != BSS_INFRA)
		{
			
			pAdapter->bConfigChanged = TRUE;
            if (MONITOR_ON(pAdapter))
            {
                RTMP_IO_WRITE32(pAdapter, RX_FILTR_CFG, STANORMAL);
                RTMP_IO_READ32(pAdapter, MAC_SYS_CTRL, &Value);
				Value &= (~0x80);
				RTMP_IO_WRITE32(pAdapter, MAC_SYS_CTRL, Value);
                OPSTATUS_CLEAR_FLAG(pAdapter, fOP_STATUS_MEDIA_STATE_CONNECTED);
                pAdapter->StaCfg.bAutoReconnect = TRUE;
                LinkDown(pAdapter, FALSE);
            }
			if (ADHOC_ON(pAdapter))
			{
				
				
				pAdapter->MlmeAux.AutoReconnectSsidLen= 32;
				NdisZeroMemory(pAdapter->MlmeAux.AutoReconnectSsid, pAdapter->MlmeAux.AutoReconnectSsidLen);

				LinkDown(pAdapter, FALSE);
			}
		}
		pAdapter->StaCfg.BssType = BSS_INFRA;
        pAdapter->net_dev->type = pAdapter->StaCfg.OriDevType;
		DBGPRINT(RT_DEBUG_TRACE, ("===>Set_NetworkType_Proc::(INFRA)\n"));

        pAdapter->StaCfg.BssType = BSS_INFRA;
	}
    else if (strcmp(arg, "Monitor") == 0)
    {
		UCHAR	bbpValue = 0;
		BCN_TIME_CFG_STRUC csr;
		OPSTATUS_CLEAR_FLAG(pAdapter, fOP_STATUS_INFRA_ON);
        OPSTATUS_CLEAR_FLAG(pAdapter, fOP_STATUS_ADHOC_ON);
		OPSTATUS_SET_FLAG(pAdapter, fOP_STATUS_MEDIA_STATE_CONNECTED);
		
		pAdapter->StaCfg.bAutoReconnect = FALSE;
		
		RT28XX_MLME_RESET_STATE_MACHINE(pAdapter);
		DBGPRINT(RT_DEBUG_TRACE, ("fOP_STATUS_MEDIA_STATE_CONNECTED \n"));
        if (pAdapter->CommonCfg.CentralChannel == 0)
        {
            if (pAdapter->CommonCfg.PhyMode == PHY_11AN_MIXED)
                pAdapter->CommonCfg.CentralChannel = 36;
            else
                pAdapter->CommonCfg.CentralChannel = 6;
        }
        else
            N_ChannelCheck(pAdapter);

	if (pAdapter->CommonCfg.PhyMode >= PHY_11ABGN_MIXED &&
            pAdapter->CommonCfg.RegTransmitSetting.field.BW == BW_40 &&
            pAdapter->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
		{
			
			RTMP_BBP_IO_READ8_BY_REG_ID(pAdapter, BBP_R4, &bbpValue);
			bbpValue &= (~0x18);
			bbpValue |= 0x10;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAdapter, BBP_R4, bbpValue);
			pAdapter->CommonCfg.BBPCurrentBW = BW_40;
			
			RTMP_BBP_IO_READ8_BY_REG_ID(pAdapter, BBP_R3, &bbpValue);
			bbpValue &= (~0x20);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAdapter, BBP_R3, bbpValue);

			RTMP_IO_READ32(pAdapter, TX_BAND_CFG, &Value);
			Value &= 0xfffffffe;
			RTMP_IO_WRITE32(pAdapter, TX_BAND_CFG, Value);
			pAdapter->CommonCfg.CentralChannel = pAdapter->CommonCfg.Channel + 2;
            AsicSwitchChannel(pAdapter, pAdapter->CommonCfg.CentralChannel, FALSE);
		    AsicLockChannel(pAdapter, pAdapter->CommonCfg.CentralChannel);
            DBGPRINT(RT_DEBUG_TRACE, ("BW_40 ,control_channel(%d), CentralChannel(%d) \n",
                                       pAdapter->CommonCfg.Channel,
                                       pAdapter->CommonCfg.CentralChannel));
		}
		else if (pAdapter->CommonCfg.PhyMode >= PHY_11ABGN_MIXED &&
                 pAdapter->CommonCfg.RegTransmitSetting.field.BW == BW_40 &&
                 pAdapter->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_BELOW)
		{
			
			RTMP_BBP_IO_READ8_BY_REG_ID(pAdapter, BBP_R4, &bbpValue);
			bbpValue &= (~0x18);
			bbpValue |= 0x10;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAdapter, BBP_R4, bbpValue);
			pAdapter->CommonCfg.BBPCurrentBW = BW_40;
			RTMP_IO_READ32(pAdapter, TX_BAND_CFG, &Value);
			Value |= 0x1;
			RTMP_IO_WRITE32(pAdapter, TX_BAND_CFG, Value);

			RTMP_BBP_IO_READ8_BY_REG_ID(pAdapter, BBP_R3, &bbpValue);
			bbpValue |= (0x20);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAdapter, BBP_R3, bbpValue);
			pAdapter->CommonCfg.CentralChannel = pAdapter->CommonCfg.Channel - 2;
            AsicSwitchChannel(pAdapter, pAdapter->CommonCfg.CentralChannel, FALSE);
		    AsicLockChannel(pAdapter, pAdapter->CommonCfg.CentralChannel);
            DBGPRINT(RT_DEBUG_TRACE, ("BW_40 ,control_channel(%d), CentralChannel(%d) \n",
                                       pAdapter->CommonCfg.Channel,
                                       pAdapter->CommonCfg.CentralChannel));
		}
		else
		{
			
			RTMP_BBP_IO_READ8_BY_REG_ID(pAdapter, BBP_R4, &bbpValue);
			bbpValue &= (~0x18);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAdapter, BBP_R4, bbpValue);
			pAdapter->CommonCfg.BBPCurrentBW = BW_20;
			AsicSwitchChannel(pAdapter, pAdapter->CommonCfg.Channel, FALSE);
			AsicLockChannel(pAdapter, pAdapter->CommonCfg.Channel);
			DBGPRINT(RT_DEBUG_TRACE, ("BW_20, Channel(%d)\n", pAdapter->CommonCfg.Channel));
		}
		
		RTMP_IO_WRITE32(pAdapter, RX_FILTR_CFG, 0x3);
		
		
		
		
		
		RTMP_IO_READ32(pAdapter, BCN_TIME_CFG, &csr.word);
		csr.field.bBeaconGen = 0;
		csr.field.bTBTTEnable = 0;
		csr.field.TsfSyncMode = 0;
		RTMP_IO_WRITE32(pAdapter, BCN_TIME_CFG, csr.word);

		pAdapter->StaCfg.BssType = BSS_MONITOR;
        pAdapter->net_dev->type = ARPHRD_IEEE80211_PRISM; 
		DBGPRINT(RT_DEBUG_TRACE, ("===>Set_NetworkType_Proc::(MONITOR)\n"));
    }

    
    pAdapter->StaCfg.WpaState = SS_NOTUSE;

    DBGPRINT(RT_DEBUG_TRACE, ("Set_NetworkType_Proc::(NetworkType=%d)\n", pAdapter->StaCfg.BssType));

    return TRUE;
}


INT Set_AuthMode_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg)
{
    if ((strcmp(arg, "WEPAUTO") == 0) || (strcmp(arg, "wepauto") == 0))
        pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeAutoSwitch;
    else if ((strcmp(arg, "OPEN") == 0) || (strcmp(arg, "open") == 0))
        pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeOpen;
    else if ((strcmp(arg, "SHARED") == 0) || (strcmp(arg, "shared") == 0))
        pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeShared;
    else if ((strcmp(arg, "WPAPSK") == 0) || (strcmp(arg, "wpapsk") == 0))
        pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeWPAPSK;
    else if ((strcmp(arg, "WPANONE") == 0) || (strcmp(arg, "wpanone") == 0))
        pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeWPANone;
    else if ((strcmp(arg, "WPA2PSK") == 0) || (strcmp(arg, "wpa2psk") == 0))
        pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeWPA2PSK;
    else if ((strcmp(arg, "WPA") == 0) || (strcmp(arg, "wpa") == 0))
        pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeWPA;
    else if ((strcmp(arg, "WPA2") == 0) || (strcmp(arg, "wpa2") == 0))
        pAdapter->StaCfg.AuthMode = Ndis802_11AuthModeWPA2;
    else
        return FALSE;

    pAdapter->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;

    DBGPRINT(RT_DEBUG_TRACE, ("Set_AuthMode_Proc::(AuthMode=%d)\n", pAdapter->StaCfg.AuthMode));

    return TRUE;
}


INT Set_EncrypType_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg)
{
    if ((strcmp(arg, "NONE") == 0) || (strcmp(arg, "none") == 0))
    {
        if (pAdapter->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
            return TRUE;    

        pAdapter->StaCfg.WepStatus     = Ndis802_11WEPDisabled;
        pAdapter->StaCfg.PairCipher    = Ndis802_11WEPDisabled;
	    pAdapter->StaCfg.GroupCipher   = Ndis802_11WEPDisabled;
    }
    else if ((strcmp(arg, "WEP") == 0) || (strcmp(arg, "wep") == 0))
    {
        if (pAdapter->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
            return TRUE;    

        pAdapter->StaCfg.WepStatus     = Ndis802_11WEPEnabled;
        pAdapter->StaCfg.PairCipher    = Ndis802_11WEPEnabled;
	    pAdapter->StaCfg.GroupCipher   = Ndis802_11WEPEnabled;
    }
    else if ((strcmp(arg, "TKIP") == 0) || (strcmp(arg, "tkip") == 0))
    {
        if (pAdapter->StaCfg.AuthMode < Ndis802_11AuthModeWPA)
            return TRUE;    

        pAdapter->StaCfg.WepStatus     = Ndis802_11Encryption2Enabled;
        pAdapter->StaCfg.PairCipher    = Ndis802_11Encryption2Enabled;
	    pAdapter->StaCfg.GroupCipher   = Ndis802_11Encryption2Enabled;
    }
    else if ((strcmp(arg, "AES") == 0) || (strcmp(arg, "aes") == 0))
    {
        if (pAdapter->StaCfg.AuthMode < Ndis802_11AuthModeWPA)
            return TRUE;    

        pAdapter->StaCfg.WepStatus     = Ndis802_11Encryption3Enabled;
        pAdapter->StaCfg.PairCipher    = Ndis802_11Encryption3Enabled;
	    pAdapter->StaCfg.GroupCipher   = Ndis802_11Encryption3Enabled;
    }
    else
        return FALSE;

    pAdapter->StaCfg.OrigWepStatus = pAdapter->StaCfg.WepStatus;

    DBGPRINT(RT_DEBUG_TRACE, ("Set_EncrypType_Proc::(EncrypType=%d)\n", pAdapter->StaCfg.WepStatus));

    return TRUE;
}


INT Set_DefaultKeyID_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg)
{
    ULONG                               KeyIdx;

    KeyIdx = simple_strtol(arg, 0, 10);
    if((KeyIdx >= 1 ) && (KeyIdx <= 4))
        pAdapter->StaCfg.DefaultKeyId = (UCHAR) (KeyIdx - 1 );
    else
        return FALSE;  

    DBGPRINT(RT_DEBUG_TRACE, ("Set_DefaultKeyID_Proc::(DefaultKeyID=%d)\n", pAdapter->StaCfg.DefaultKeyId));

    return TRUE;
}


INT Set_Key1_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg)
{
    int                                 KeyLen;
    int                                 i;
    UCHAR                               CipherAlg=CIPHER_WEP64;

    if (pAdapter->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
        return TRUE;    

    KeyLen = strlen(arg);

    switch (KeyLen)
    {
        case 5: 
            pAdapter->SharedKey[BSS0][0].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][0].Key, arg, KeyLen);
            CipherAlg = CIPHER_WEP64;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key1_Proc::(Key1=%s and type=%s)\n", arg, "Ascii"));
            break;
        case 10: 
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(arg+i)) )
                    return FALSE;  
            }
            pAdapter->SharedKey[BSS0][0].KeyLen = KeyLen / 2 ;
            AtoH(arg, pAdapter->SharedKey[BSS0][0].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP64;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key1_Proc::(Key1=%s and type=%s)\n", arg, "Hex"));
            break;
        case 13: 
            pAdapter->SharedKey[BSS0][0].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][0].Key, arg, KeyLen);
            CipherAlg = CIPHER_WEP128;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key1_Proc::(Key1=%s and type=%s)\n", arg, "Ascii"));
            break;
        case 26: 
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(arg+i)) )
                    return FALSE;  
            }
            pAdapter->SharedKey[BSS0][0].KeyLen = KeyLen / 2 ;
            AtoH(arg, pAdapter->SharedKey[BSS0][0].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP128;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key1_Proc::(Key1=%s and type=%s)\n", arg, "Hex"));
            break;
        default: 
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key1_Proc::Invalid argument (=%s)\n", arg));
            return FALSE;
    }

    pAdapter->SharedKey[BSS0][0].CipherAlg = CipherAlg;

    
    if (pAdapter->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
        ;   
    else    
    {
        AsicAddSharedKeyEntry(pAdapter,
                              0,
                              0,
                              pAdapter->SharedKey[BSS0][0].CipherAlg,
                              pAdapter->SharedKey[BSS0][0].Key,
                              NULL,
                              NULL);
    }

    return TRUE;
}

INT Set_Key2_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg)
{
    int                                 KeyLen;
    int                                 i;
    UCHAR                               CipherAlg=CIPHER_WEP64;

    if (pAdapter->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
        return TRUE;    

    KeyLen = strlen(arg);

    switch (KeyLen)
    {
        case 5: 
            pAdapter->SharedKey[BSS0][1].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][1].Key, arg, KeyLen);
            CipherAlg = CIPHER_WEP64;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key2_Proc::(Key2=%s and type=%s)\n", arg, "Ascii"));
            break;
        case 10: 
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(arg+i)) )
                    return FALSE;  
            }
            pAdapter->SharedKey[BSS0][1].KeyLen = KeyLen / 2 ;
            AtoH(arg, pAdapter->SharedKey[BSS0][1].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP64;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key2_Proc::(Key2=%s and type=%s)\n", arg, "Hex"));
            break;
        case 13: 
            pAdapter->SharedKey[BSS0][1].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][1].Key, arg, KeyLen);
            CipherAlg = CIPHER_WEP128;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key2_Proc::(Key2=%s and type=%s)\n", arg, "Ascii"));
            break;
        case 26: 
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(arg+i)) )
                    return FALSE;  
            }
            pAdapter->SharedKey[BSS0][1].KeyLen = KeyLen / 2 ;
            AtoH(arg, pAdapter->SharedKey[BSS0][1].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP128;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key2_Proc::(Key2=%s and type=%s)\n", arg, "Hex"));
            break;
        default: 
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key2_Proc::Invalid argument (=%s)\n", arg));
            return FALSE;
    }
    pAdapter->SharedKey[BSS0][1].CipherAlg = CipherAlg;

    
    if (pAdapter->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
        ;   
    else    
    {
        AsicAddSharedKeyEntry(pAdapter,
                              0,
                              1,
                              pAdapter->SharedKey[BSS0][1].CipherAlg,
                              pAdapter->SharedKey[BSS0][1].Key,
                              NULL,
                              NULL);
    }

    return TRUE;
}

INT Set_Key3_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg)
{
    int                                 KeyLen;
    int                                 i;
    UCHAR                               CipherAlg=CIPHER_WEP64;

    if (pAdapter->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
        return TRUE;    

    KeyLen = strlen(arg);

    switch (KeyLen)
    {
        case 5: 
            pAdapter->SharedKey[BSS0][2].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][2].Key, arg, KeyLen);
            CipherAlg = CIPHER_WEP64;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key3_Proc::(Key3=%s and type=Ascii)\n", arg));
            break;
        case 10: 
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(arg+i)) )
                    return FALSE;  
            }
            pAdapter->SharedKey[BSS0][2].KeyLen = KeyLen / 2 ;
            AtoH(arg, pAdapter->SharedKey[BSS0][2].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP64;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key3_Proc::(Key3=%s and type=Hex)\n", arg));
            break;
        case 13: 
            pAdapter->SharedKey[BSS0][2].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][2].Key, arg, KeyLen);
            CipherAlg = CIPHER_WEP128;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key3_Proc::(Key3=%s and type=Ascii)\n", arg));
            break;
        case 26: 
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(arg+i)) )
                    return FALSE;  
            }
            pAdapter->SharedKey[BSS0][2].KeyLen = KeyLen / 2 ;
            AtoH(arg, pAdapter->SharedKey[BSS0][2].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP128;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key3_Proc::(Key3=%s and type=Hex)\n", arg));
            break;
        default: 
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key3_Proc::Invalid argument (=%s)\n", arg));
            return FALSE;
    }
    pAdapter->SharedKey[BSS0][2].CipherAlg = CipherAlg;

    
    if (pAdapter->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
        ;   
    else    
    {
        AsicAddSharedKeyEntry(pAdapter,
                              0,
                              2,
                              pAdapter->SharedKey[BSS0][2].CipherAlg,
                              pAdapter->SharedKey[BSS0][2].Key,
                              NULL,
                              NULL);
    }

    return TRUE;
}

INT Set_Key4_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg)
{
    int                                 KeyLen;
    int                                 i;
    UCHAR                               CipherAlg=CIPHER_WEP64;

    if (pAdapter->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
        return TRUE;    

    KeyLen = strlen(arg);

    switch (KeyLen)
    {
        case 5: 
            pAdapter->SharedKey[BSS0][3].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][3].Key, arg, KeyLen);
            CipherAlg = CIPHER_WEP64;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key4_Proc::(Key4=%s and type=%s)\n", arg, "Ascii"));
            break;
        case 10: 
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(arg+i)) )
                    return FALSE;  
            }
            pAdapter->SharedKey[BSS0][3].KeyLen = KeyLen / 2 ;
            AtoH(arg, pAdapter->SharedKey[BSS0][3].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP64;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key4_Proc::(Key4=%s and type=%s)\n", arg, "Hex"));
            break;
        case 13: 
            pAdapter->SharedKey[BSS0][3].KeyLen = KeyLen;
            memcpy(pAdapter->SharedKey[BSS0][3].Key, arg, KeyLen);
            CipherAlg = CIPHER_WEP128;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key4_Proc::(Key4=%s and type=%s)\n", arg, "Ascii"));
            break;
        case 26: 
            for(i=0; i < KeyLen; i++)
            {
                if( !isxdigit(*(arg+i)) )
                    return FALSE;  
            }
            pAdapter->SharedKey[BSS0][3].KeyLen = KeyLen / 2 ;
            AtoH(arg, pAdapter->SharedKey[BSS0][3].Key, KeyLen / 2);
            CipherAlg = CIPHER_WEP128;
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key4_Proc::(Key4=%s and type=%s)\n", arg, "Hex"));
            break;
        default: 
            DBGPRINT(RT_DEBUG_TRACE, ("Set_Key4_Proc::Invalid argument (=%s)\n", arg));
            return FALSE;
    }
    pAdapter->SharedKey[BSS0][3].CipherAlg = CipherAlg;

    
    if (pAdapter->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
        ;   
    else    
    {
        AsicAddSharedKeyEntry(pAdapter,
                              0,
                              3,
                              pAdapter->SharedKey[BSS0][3].CipherAlg,
                              pAdapter->SharedKey[BSS0][3].Key,
                              NULL,
                              NULL);
    }

    return TRUE;
}


INT Set_WPAPSK_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg)
{
    UCHAR                   keyMaterial[40];

    if ((pAdapter->StaCfg.AuthMode != Ndis802_11AuthModeWPAPSK) &&
        (pAdapter->StaCfg.AuthMode != Ndis802_11AuthModeWPA2PSK) &&
	    (pAdapter->StaCfg.AuthMode != Ndis802_11AuthModeWPANone)
		)
        return TRUE;    

    DBGPRINT(RT_DEBUG_TRACE, ("Set_WPAPSK_Proc::(WPAPSK=%s)\n", arg));

    NdisZeroMemory(keyMaterial, 40);

    if ((strlen(arg) < 8) || (strlen(arg) > 64))
    {
        DBGPRINT(RT_DEBUG_TRACE, ("Set failed!!(WPAPSK=%s), WPAPSK key-string required 8 ~ 64 characters \n", arg));
        return FALSE;
    }

    if (strlen(arg) == 64)
    {
        AtoH(arg, keyMaterial, 32);
        NdisMoveMemory(pAdapter->StaCfg.PMK, keyMaterial, 32);

    }
    else
    {
        PasswordHash((char *)arg, pAdapter->MlmeAux.Ssid, pAdapter->MlmeAux.SsidLen, keyMaterial);
        NdisMoveMemory(pAdapter->StaCfg.PMK, keyMaterial, 32);
    }



    if(pAdapter->StaCfg.BssType == BSS_ADHOC &&
       pAdapter->StaCfg.AuthMode == Ndis802_11AuthModeWPANone)
    {
         pAdapter->StaCfg.WpaState = SS_NOTUSE;
    }
    else
    {
        
        pAdapter->StaCfg.WpaState = SS_START;
    }

    return TRUE;
}


INT Set_PSMode_Proc(
    IN  PRTMP_ADAPTER   pAdapter,
    IN  PUCHAR          arg)
{
    if (pAdapter->StaCfg.BssType == BSS_INFRA)
    {
        if ((strcmp(arg, "Max_PSP") == 0) ||
			(strcmp(arg, "max_psp") == 0) ||
			(strcmp(arg, "MAX_PSP") == 0))
        {
            
            
            if (pAdapter->StaCfg.bWindowsACCAMEnable == FALSE)
                pAdapter->StaCfg.WindowsPowerMode = Ndis802_11PowerModeMAX_PSP;
            pAdapter->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeMAX_PSP;
            OPSTATUS_SET_FLAG(pAdapter, fOP_STATUS_RECEIVE_DTIM);
            pAdapter->StaCfg.DefaultListenCount = 5;

        }
        else if ((strcmp(arg, "Fast_PSP") == 0) ||
				 (strcmp(arg, "fast_psp") == 0) ||
                 (strcmp(arg, "FAST_PSP") == 0))
        {
            
            
            OPSTATUS_SET_FLAG(pAdapter, fOP_STATUS_RECEIVE_DTIM);
            if (pAdapter->StaCfg.bWindowsACCAMEnable == FALSE)
                pAdapter->StaCfg.WindowsPowerMode = Ndis802_11PowerModeFast_PSP;
            pAdapter->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeFast_PSP;
            pAdapter->StaCfg.DefaultListenCount = 3;
        }
        else if ((strcmp(arg, "Legacy_PSP") == 0) ||
                 (strcmp(arg, "legacy_psp") == 0) ||
                 (strcmp(arg, "LEGACY_PSP") == 0))
        {
            
            
            OPSTATUS_SET_FLAG(pAdapter, fOP_STATUS_RECEIVE_DTIM);
            if (pAdapter->StaCfg.bWindowsACCAMEnable == FALSE)
                pAdapter->StaCfg.WindowsPowerMode = Ndis802_11PowerModeLegacy_PSP;
            pAdapter->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeLegacy_PSP;
            pAdapter->StaCfg.DefaultListenCount = 3;
        }
        else
        {
            
            
            MlmeSetPsmBit(pAdapter, PWR_ACTIVE);
            OPSTATUS_SET_FLAG(pAdapter, fOP_STATUS_RECEIVE_DTIM);
            if (pAdapter->StaCfg.bWindowsACCAMEnable == FALSE)
                pAdapter->StaCfg.WindowsPowerMode = Ndis802_11PowerModeCAM;
            pAdapter->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;
        }

        DBGPRINT(RT_DEBUG_TRACE, ("Set_PSMode_Proc::(PSMode=%ld)\n", pAdapter->StaCfg.WindowsPowerMode));
    }
    else
        return FALSE;


    return TRUE;
}


INT Set_Wpa_Support(
    IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			arg)
{

    if ( simple_strtol(arg, 0, 10) == 0)
        pAd->StaCfg.WpaSupplicantUP = WPA_SUPPLICANT_DISABLE;
    else if ( simple_strtol(arg, 0, 10) == 1)
        pAd->StaCfg.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE;
    else if ( simple_strtol(arg, 0, 10) == 2)
        pAd->StaCfg.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE_WITH_WEB_UI;
    else
        pAd->StaCfg.WpaSupplicantUP = WPA_SUPPLICANT_DISABLE;

    DBGPRINT(RT_DEBUG_TRACE, ("Set_Wpa_Support::(WpaSupplicantUP=%d)\n", pAd->StaCfg.WpaSupplicantUP));

    return TRUE;
}

INT Set_TGnWifiTest_Proc(
    IN  PRTMP_ADAPTER   pAd,
    IN  PUCHAR          arg)
{
    if (simple_strtol(arg, 0, 10) == 0)
        pAd->StaCfg.bTGnWifiTest = FALSE;
    else
        pAd->StaCfg.bTGnWifiTest = TRUE;

    DBGPRINT(RT_DEBUG_TRACE, ("IF Set_TGnWifiTest_Proc::(bTGnWifiTest=%d)\n", pAd->StaCfg.bTGnWifiTest));
	return TRUE;
}

INT Set_LongRetryLimit_Proc(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	PUCHAR			arg)
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
	IN	PUCHAR			arg)
{
	TX_RTY_CFG_STRUC	tx_rty_cfg;
	UCHAR				ShortRetryLimit = (UCHAR)simple_strtol(arg, 0, 10);

	RTMP_IO_READ32(pAdapter, TX_RTY_CFG, &tx_rty_cfg.word);
	tx_rty_cfg.field.ShortRtyLimit = ShortRetryLimit;
	RTMP_IO_WRITE32(pAdapter, TX_RTY_CFG, tx_rty_cfg.word);
	DBGPRINT(RT_DEBUG_TRACE, ("IF Set_ShortRetryLimit_Proc::(tx_rty_cfg=0x%x)\n", tx_rty_cfg.word));
	return TRUE;
}
