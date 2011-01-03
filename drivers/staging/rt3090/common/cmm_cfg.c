

#include "../rt_config.h"


char* GetPhyMode(
	int Mode)
{
	switch(Mode)
	{
		case MODE_CCK:
			return "CCK";

		case MODE_OFDM:
			return "OFDM";
#ifdef DOT11_N_SUPPORT
		case MODE_HTMIX:
			return "HTMIX";

		case MODE_HTGREENFIELD:
			return "GREEN";
#endif 
		default:
			return "N/A";
	}
}


char* GetBW(
	int BW)
{
	switch(BW)
	{
		case BW_10:
			return "10M";

		case BW_20:
			return "20M";
#ifdef DOT11_N_SUPPORT
		case BW_40:
			return "40M";
#endif 
		default:
			return "N/A";
	}
}



INT RT_CfgSetCountryRegion(
	IN PRTMP_ADAPTER	pAd,
	IN PSTRING			arg,
	IN INT				band)
{
	LONG region, regionMax;
	UCHAR *pCountryRegion;

	region = simple_strtol(arg, 0, 10);

	if (band == BAND_24G)
	{
		pCountryRegion = &pAd->CommonCfg.CountryRegion;
		regionMax = REGION_MAXIMUM_BG_BAND;
	}
	else
	{
		pCountryRegion = &pAd->CommonCfg.CountryRegionForABand;
		regionMax = REGION_MAXIMUM_A_BAND;
	}

	
	
	if (*pCountryRegion & 0x80)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("CfgSetCountryRegion():CountryRegion in eeprom was programmed\n"));
		return FALSE;
	}

	if((region >= 0) && (region <= REGION_MAXIMUM_BG_BAND))
	{
		*pCountryRegion= (UCHAR) region;
	}
	else if ((region == REGION_31_BG_BAND) && (band == BAND_24G))
	{
		*pCountryRegion = (UCHAR) region;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("CfgSetCountryRegion():region(%ld) out of range!\n", region));
		return FALSE;
	}

	return TRUE;

}



INT RT_CfgSetWirelessMode(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	INT		MaxPhyMode = PHY_11G;
	LONG	WirelessMode;

#ifdef DOT11_N_SUPPORT
	MaxPhyMode = PHY_11N_5G;
#endif 

	WirelessMode = simple_strtol(arg, 0, 10);
	if (WirelessMode <= MaxPhyMode)
	{
		pAd->CommonCfg.PhyMode = WirelessMode;
		pAd->CommonCfg.DesiredPhyMode = WirelessMode;
		return TRUE;
	}

	return FALSE;

}


INT RT_CfgSetShortSlot(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	LONG ShortSlot;

	ShortSlot = simple_strtol(arg, 0, 10);

	if (ShortSlot == 1)
		pAd->CommonCfg.bUseShortSlotTime = TRUE;
	else if (ShortSlot == 0)
		pAd->CommonCfg.bUseShortSlotTime = FALSE;
	else
		return FALSE;  

	return TRUE;
}



INT	RT_CfgSetWepKey(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			keyString,
	IN	CIPHER_KEY		*pSharedKey,
	IN	INT				keyIdx)
{
	INT				KeyLen;
	INT				i;
	UCHAR			CipherAlg = CIPHER_NONE;
	BOOLEAN			bKeyIsHex = FALSE;

	
	memset(pSharedKey, 0, sizeof(CIPHER_KEY));
	KeyLen = strlen(keyString);
	switch (KeyLen)
	{
		case 5: 
		case 13: 
			bKeyIsHex = FALSE;
			pSharedKey->KeyLen = KeyLen;
			NdisMoveMemory(pSharedKey->Key, keyString, KeyLen);
			break;

		case 10: 
		case 26: 
			for(i=0; i < KeyLen; i++)
			{
				if( !isxdigit(*(keyString+i)) )
					return FALSE;  
			}
			bKeyIsHex = TRUE;
			pSharedKey->KeyLen = KeyLen/2 ;
			AtoH(keyString, pSharedKey->Key, pSharedKey->KeyLen);
			break;

		default: 
			DBGPRINT(RT_DEBUG_TRACE, ("RT_CfgSetWepKey(keyIdx=%d):Invalid argument (arg=%s)\n", keyIdx, keyString));
			return FALSE;
	}

	pSharedKey->CipherAlg = ((KeyLen % 5) ? CIPHER_WEP128 : CIPHER_WEP64);
	DBGPRINT(RT_DEBUG_TRACE, ("RT_CfgSetWepKey:(KeyIdx=%d,type=%s, Alg=%s)\n",
						keyIdx, (bKeyIsHex == FALSE ? "Ascii" : "Hex"), CipherName[CipherAlg]));

	return TRUE;
}



INT RT_CfgSetWPAPSKKey(
	IN RTMP_ADAPTER	*pAd,
	IN PSTRING		keyString,
	IN UCHAR		*pHashStr,
	IN INT			hashStrLen,
	OUT PUCHAR		pPMKBuf)
{
	int keyLen;
	UCHAR keyMaterial[40];

	keyLen = strlen(keyString);
	if ((keyLen < 8) || (keyLen > 64))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("WPAPSK Key length(%d) error, required 8 ~ 64 characters!(keyStr=%s)\n",
									keyLen, keyString));
		return FALSE;
	}

	memset(pPMKBuf, 0, 32);
	if (keyLen == 64)
	{
	    AtoH(keyString, pPMKBuf, 32);
	}
	else
	{
	    PasswordHash(keyString, pHashStr, hashStrLen, keyMaterial);
	    NdisMoveMemory(pPMKBuf, keyMaterial, 32);
	}

	return TRUE;
}
