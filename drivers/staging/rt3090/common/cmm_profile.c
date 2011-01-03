

#include "../rt_config.h"


#define ETH_MAC_ADDR_STR_LEN 17  


BOOLEAN rtstrmactohex(PSTRING s1, PSTRING s2)
{
	int i = 0;
	PSTRING ptokS = s1, ptokE = s1;

	if (strlen(s1) != ETH_MAC_ADDR_STR_LEN)
		return FALSE;

	while((*ptokS) != '\0')
	{
		if((ptokE = strchr(ptokS, ':')) != NULL)
			*ptokE++ = '\0';
		if ((strlen(ptokS) != 2) || (!isxdigit(*ptokS)) || (!isxdigit(*(ptokS+1))))
			break; 
		AtoH(ptokS, (PUCHAR)&s2[i++], 1);
		ptokS = ptokE;
		if (i == 6)
			break; 
	}

	return ( i == 6 ? TRUE : FALSE);

}



BOOLEAN rtstrcasecmp(PSTRING s1, PSTRING s2)
{
	PSTRING p1 = s1, p2 = s2;

	if (strlen(s1) != strlen(s2))
		return FALSE;

	while(*p1 != '\0')
	{
		if((*p1 != *p2) && ((*p1 ^ *p2) != 0x20))
			return FALSE;
		p1++;
		p2++;
	}

	return TRUE;
}


PSTRING rtstrstruncasecmp(PSTRING s1, PSTRING s2)
{
	INT l1, l2, i;
	char temp1, temp2;

	l2 = strlen(s2);
	if (!l2)
		return (char *) s1;

	l1 = strlen(s1);

	while (l1 >= l2)
	{
		l1--;

		for(i=0; i<l2; i++)
		{
			temp1 = *(s1+i);
			temp2 = *(s2+i);

			if (('a' <= temp1) && (temp1 <= 'z'))
				temp1 = 'A'+(temp1-'a');
			if (('a' <= temp2) && (temp2 <= 'z'))
				temp2 = 'A'+(temp2-'a');

			if (temp1 != temp2)
				break;
		}

		if (i == l2)
			return (char *) s1;

		s1++;
	}

	return NULL; 
}



 
PSTRING rtstrstr(PSTRING s1,const PSTRING s2)
{
	INT l1, l2;

	l2 = strlen(s2);
	if (!l2)
		return s1;

	l1 = strlen(s1);

	while (l1 >= l2)
	{
		l1--;
		if (!memcmp(s1,s2,l2))
			return s1;
		s1++;
	}

	return NULL;
}


PSTRING __rstrtok;
PSTRING rstrtok(PSTRING s,const PSTRING ct)
{
	PSTRING sbegin, send;

	sbegin  = s ? s : __rstrtok;
	if (!sbegin)
	{
		return NULL;
	}

	sbegin += strspn(sbegin,ct);
	if (*sbegin == '\0')
	{
		__rstrtok = NULL;
		return( NULL );
	}

	send = strpbrk( sbegin, ct);
	if (send && *send != '\0')
		*send++ = '\0';

	__rstrtok = send;

	return (sbegin);
}


INT delimitcnt(PSTRING s,PSTRING ct)
{
	INT count = 0;
	
	PSTRING token = s;

	for ( ;; )
	{
		token = strpbrk(token, ct); 

        if ( token == NULL )
		{
			
			break;
		}
		
	    ++token;

		

		
	    ++count;
	}
    return count;
}


int rtinet_aton(PSTRING cp, unsigned int *addr)
{
	unsigned int	val;
	int		base, n;
	STRING		c;
	unsigned int    parts[4];
	unsigned int    *pp = parts;

	for (;;)
    {
         
         val = 0;
         base = 10;
         if (*cp == '0')
         {
             if (*++cp == 'x' || *cp == 'X')
                 base = 16, cp++;
             else
                 base = 8;
         }
         while ((c = *cp) != '\0')
         {
             if (isdigit((unsigned char) c))
             {
                 val = (val * base) + (c - '0');
                 cp++;
                 continue;
             }
             if (base == 16 && isxdigit((unsigned char) c))
             {
                 val = (val << 4) +
                     (c + 10 - (islower((unsigned char) c) ? 'a' : 'A'));
                 cp++;
                 continue;
             }
             break;
         }
         if (*cp == '.')
         {
             
             if (pp >= parts + 3 || val > 0xff)
                 return 0;
             *pp++ = val, cp++;
         }
         else
             break;
     }

     
     while (*cp)
         if (!isspace((unsigned char) *cp++))
             return 0;

     
     n = pp - parts + 1;
     switch (n)
     {

         case 1:         
             break;

         case 2:         
             if (val > 0xffffff)
                 return 0;
             val |= parts[0] << 24;
             break;

         case 3:         
             if (val > 0xffff)
                 return 0;
             val |= (parts[0] << 24) | (parts[1] << 16);
             break;

         case 4:         
             if (val > 0xff)
                 return 0;
             val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
             break;
     }

     *addr = htonl(val);
     return 1;

}


PSTRING RTMPFindSection(
    IN  PSTRING   buffer)
{
    STRING temp_buf[32];
    PSTRING  ptr;

    strcpy(temp_buf, "Default");

    if((ptr = rtstrstr(buffer, temp_buf)) != NULL)
            return (ptr+strlen("\n"));
        else
            return NULL;
}


INT RTMPGetKeyParameter(
    IN PSTRING key,
    OUT PSTRING dest,
    IN INT destsize,
    IN PSTRING buffer,
    IN BOOLEAN bTrimSpace)
{
	PSTRING pMemBuf, temp_buf1 = NULL, temp_buf2 = NULL;
	PSTRING start_ptr, end_ptr;
	PSTRING ptr;
	PSTRING offset = NULL;
	INT  len, keyLen;


	keyLen = strlen(key);
	os_alloc_mem(NULL, (PUCHAR *)&pMemBuf, MAX_PARAM_BUFFER_SIZE  * 2);
	if (pMemBuf == NULL)
		return (FALSE);

	memset(pMemBuf, 0, MAX_PARAM_BUFFER_SIZE * 2);
	temp_buf1 = pMemBuf;
	temp_buf2 = (PSTRING)(pMemBuf + MAX_PARAM_BUFFER_SIZE);


	
	if((offset = RTMPFindSection(buffer)) == NULL)
	{
		os_free_mem(NULL, (PUCHAR)pMemBuf);
		return (FALSE);
	}

	strcpy(temp_buf1, "\n");
	strcat(temp_buf1, key);
	strcat(temp_buf1, "=");

	
	if((start_ptr=rtstrstr(offset, temp_buf1)) == NULL)
	{
		os_free_mem(NULL, (PUCHAR)pMemBuf);
		return (FALSE);
	}

	start_ptr += strlen("\n");
	if((end_ptr = rtstrstr(start_ptr, "\n"))==NULL)
		end_ptr = start_ptr+strlen(start_ptr);

	if (end_ptr<start_ptr)
	{
		os_free_mem(NULL, (PUCHAR)pMemBuf);
		return (FALSE);
	}

	NdisMoveMemory(temp_buf2, start_ptr, end_ptr-start_ptr);
	temp_buf2[end_ptr-start_ptr]='\0';
	if((start_ptr=rtstrstr(temp_buf2, "=")) == NULL)
	{
		os_free_mem(NULL, (PUCHAR)pMemBuf);
		return (FALSE);
	}
	ptr = (start_ptr +1);
	
	while(*start_ptr != 0x00)
	{
		if( ((*ptr == ' ') && bTrimSpace) || (*ptr == '\t') )
			ptr++;
		else
			break;
	}
	len = strlen(start_ptr);

	memset(dest, 0x00, destsize);
	strncpy(dest, ptr, ((len >= destsize) ? destsize: len));

	os_free_mem(NULL, (PUCHAR)pMemBuf);

	return TRUE;
}



INT RTMPGetKeyParameterWithOffset(
    IN  PSTRING   key,
    OUT PSTRING   dest,
    OUT	USHORT	*end_offset,
    IN  INT     destsize,
    IN  PSTRING   buffer,
    IN	BOOLEAN	bTrimSpace)
{
    PSTRING temp_buf1 = NULL;
    PSTRING temp_buf2 = NULL;
    PSTRING start_ptr;
    PSTRING end_ptr;
    PSTRING ptr;
    PSTRING offset = 0;
    INT  len;

	if (*end_offset >= MAX_INI_BUFFER_SIZE)
		return (FALSE);

	os_alloc_mem(NULL, (PUCHAR *)&temp_buf1, MAX_PARAM_BUFFER_SIZE);

	if(temp_buf1 == NULL)
        return (FALSE);

	os_alloc_mem(NULL, (PUCHAR *)&temp_buf2, MAX_PARAM_BUFFER_SIZE);
	if(temp_buf2 == NULL)
	{
		os_free_mem(NULL, (PUCHAR)temp_buf1);
        return (FALSE);
	}

    
	if(*end_offset == 0)
    {
		if ((offset = RTMPFindSection(buffer)) == NULL)
		{
			os_free_mem(NULL, (PUCHAR)temp_buf1);
		os_free_mem(NULL, (PUCHAR)temp_buf2);
	    return (FALSE);
		}
    }
	else
		offset = buffer + (*end_offset);

    strcpy(temp_buf1, "\n");
    strcat(temp_buf1, key);
    strcat(temp_buf1, "=");

    
    if((start_ptr=rtstrstr(offset, temp_buf1))==NULL)
    {
		os_free_mem(NULL, (PUCHAR)temp_buf1);
	os_free_mem(NULL, (PUCHAR)temp_buf2);
        return (FALSE);
    }

    start_ptr+=strlen("\n");
    if((end_ptr=rtstrstr(start_ptr, "\n"))==NULL)
       end_ptr=start_ptr+strlen(start_ptr);

    if (end_ptr<start_ptr)
    {
		os_free_mem(NULL, (PUCHAR)temp_buf1);
	os_free_mem(NULL, (PUCHAR)temp_buf2);
        return (FALSE);
    }

	*end_offset = end_ptr - buffer;

    NdisMoveMemory(temp_buf2, start_ptr, end_ptr-start_ptr);
    temp_buf2[end_ptr-start_ptr]='\0';
    len = strlen(temp_buf2);
    strcpy(temp_buf1, temp_buf2);
    if((start_ptr=rtstrstr(temp_buf1, "=")) == NULL)
    {
		os_free_mem(NULL, (PUCHAR)temp_buf1);
	os_free_mem(NULL, (PUCHAR)temp_buf2);
        return (FALSE);
    }

    strcpy(temp_buf2, start_ptr+1);
    ptr = temp_buf2;
    
    while(*ptr != 0x00)
    {
        if((bTrimSpace && (*ptr == ' ')) || (*ptr == '\t') )
            ptr++;
        else
           break;
    }

    len = strlen(ptr);
    memset(dest, 0x00, destsize);
    strncpy(dest, ptr, len >= destsize ?  destsize: len);

	os_free_mem(NULL, (PUCHAR)temp_buf1);
    os_free_mem(NULL, (PUCHAR)temp_buf2);
    return TRUE;
}


static int rtmp_parse_key_buffer_from_file(IN  PRTMP_ADAPTER pAd,IN  PSTRING buffer,IN  ULONG KeyType,IN  INT BSSIdx,IN  INT KeyIdx)
{
	PSTRING		keybuff;
	
	ULONG		KeyLen;
	
	CIPHER_KEY	*pSharedKey;

	keybuff = buffer;
	KeyLen = strlen(keybuff);
	pSharedKey = &pAd->SharedKey[BSSIdx][KeyIdx];

	if(((KeyType != 0) && (KeyType != 1)) ||
	    ((KeyType == 0) && (KeyLen != 10) && (KeyLen != 26)) ||
	    ((KeyType== 1) && (KeyLen != 5) && (KeyLen != 13)))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Key%dStr is Invalid key length(%ld) or Type(%ld)\n",
								KeyIdx+1, KeyLen, KeyType));
		return FALSE;
	}
	else
	{
		return RT_CfgSetWepKey(pAd, buffer, pSharedKey, KeyIdx);
	}

}


static void rtmp_read_key_parms_from_file(IN  PRTMP_ADAPTER pAd, PSTRING tmpbuf, PSTRING buffer)
{
	STRING		tok_str[16];
	PSTRING		macptr;
	INT			i = 0, idx;
	ULONG		KeyType[MAX_MBSSID_NUM];
	ULONG		KeyIdx;

	NdisZeroMemory(KeyType, sizeof(KeyType));

	
	if(RTMPGetKeyParameter("DefaultKeyID", tmpbuf, 25, buffer, TRUE))
	{

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			KeyIdx = simple_strtol(tmpbuf, 0, 10);
			if((KeyIdx >= 1 ) && (KeyIdx <= 4))
				pAd->StaCfg.DefaultKeyId = (UCHAR) (KeyIdx - 1);
			else
				pAd->StaCfg.DefaultKeyId = 0;

			DBGPRINT(RT_DEBUG_TRACE, ("DefaultKeyID(0~3)=%d\n", pAd->StaCfg.DefaultKeyId));
		}
#endif 
	}


	for (idx = 0; idx < 4; idx++)
	{
		sprintf(tok_str, "Key%dType", idx + 1);
		
		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, TRUE))
		{
		    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		    {
				
				if (i < MAX_MBSSID_NUM)
					KeyType[i] = simple_strtol(macptr, 0, 10);
		    }

#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
				sprintf(tok_str, "Key%dStr", idx + 1);
				if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, FALSE))
				{
					rtmp_parse_key_buffer_from_file(pAd, tmpbuf, KeyType[BSS0], BSS0, idx);
				}
			}
#endif 
		}
	}
}



#ifdef CONFIG_STA_SUPPORT
static void rtmp_read_sta_wmm_parms_from_file(IN  PRTMP_ADAPTER pAd, char *tmpbuf, char *buffer)
{
	PSTRING					macptr;
	INT						i=0;
	BOOLEAN					bWmmEnable = FALSE;

	
	if(RTMPGetKeyParameter("WmmCapable", tmpbuf, 32, buffer, TRUE))
	{
		if(simple_strtol(tmpbuf, 0, 10) != 0) 
		{
			pAd->CommonCfg.bWmmCapable = TRUE;
			bWmmEnable = TRUE;
		}
		else 
		{
			pAd->CommonCfg.bWmmCapable = FALSE;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("WmmCapable=%d\n", pAd->CommonCfg.bWmmCapable));
	}

#ifdef QOS_DLS_SUPPORT
	
	if(RTMPGetKeyParameter("DLSCapable", tmpbuf, 32, buffer, TRUE))
	{
		if(simple_strtol(tmpbuf, 0, 10) != 0)  
		{
			pAd->CommonCfg.bDLSCapable = TRUE;
		}
		else 
		{
			pAd->CommonCfg.bDLSCapable = FALSE;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("bDLSCapable=%d\n", pAd->CommonCfg.bDLSCapable));
	}
#endif 

	
	if(RTMPGetKeyParameter("AckPolicy", tmpbuf, 32, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			pAd->CommonCfg.AckPolicy[i] = (UCHAR)simple_strtol(macptr, 0, 10);

			DBGPRINT(RT_DEBUG_TRACE, ("AckPolicy[%d]=%d\n", i, pAd->CommonCfg.AckPolicy[i]));
		}
	}

	if (bWmmEnable)
	{
		
		if(RTMPGetKeyParameter("APSDCapable", tmpbuf, 10, buffer, TRUE))
		{
			if(simple_strtol(tmpbuf, 0, 10) != 0)  
				pAd->CommonCfg.bAPSDCapable = TRUE;
			else
				pAd->CommonCfg.bAPSDCapable = FALSE;

			DBGPRINT(RT_DEBUG_TRACE, ("APSDCapable=%d\n", pAd->CommonCfg.bAPSDCapable));
		}

		
		if(RTMPGetKeyParameter("MaxSPLength", tmpbuf, 10, buffer, TRUE))
		{
			pAd->CommonCfg.MaxSPLength = simple_strtol(tmpbuf, 0, 10);

			DBGPRINT(RT_DEBUG_TRACE, ("MaxSPLength=%d\n", pAd->CommonCfg.MaxSPLength));
		}

		
		if(RTMPGetKeyParameter("APSDAC", tmpbuf, 32, buffer, TRUE))
		{
			BOOLEAN apsd_ac[4];

			for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
			{
				apsd_ac[i] = (BOOLEAN)simple_strtol(macptr, 0, 10);

				DBGPRINT(RT_DEBUG_TRACE, ("APSDAC%d  %d\n", i,  apsd_ac[i]));
			}

			pAd->CommonCfg.bAPSDAC_BE = apsd_ac[0];
			pAd->CommonCfg.bAPSDAC_BK = apsd_ac[1];
			pAd->CommonCfg.bAPSDAC_VI = apsd_ac[2];
			pAd->CommonCfg.bAPSDAC_VO = apsd_ac[3];

			pAd->CommonCfg.bACMAPSDTr[0] = apsd_ac[0];
			pAd->CommonCfg.bACMAPSDTr[1] = apsd_ac[1];
			pAd->CommonCfg.bACMAPSDTr[2] = apsd_ac[2];
			pAd->CommonCfg.bACMAPSDTr[3] = apsd_ac[3];
		}
	}

}
#endif 


#ifdef DOT11_N_SUPPORT
static void HTParametersHook(
	IN	PRTMP_ADAPTER pAd,
	IN	PSTRING		  pValueStr,
	IN	PSTRING		  pInput)
{

	long Value;

    if (RTMPGetKeyParameter("HT_PROTECT", pValueStr, 25, pInput, TRUE))
    {
        Value = simple_strtol(pValueStr, 0, 10);
        if (Value == 0)
        {
            pAd->CommonCfg.bHTProtect = FALSE;
        }
        else
        {
            pAd->CommonCfg.bHTProtect = TRUE;
        }
        DBGPRINT(RT_DEBUG_TRACE, ("HT: Protection  = %s\n", (Value==0) ? "Disable" : "Enable"));
    }

    if (RTMPGetKeyParameter("HT_MIMOPSEnable", pValueStr, 25, pInput, TRUE))
    {
        Value = simple_strtol(pValueStr, 0, 10);
        if (Value == 0)
        {
            pAd->CommonCfg.bMIMOPSEnable = FALSE;
        }
        else
        {
            pAd->CommonCfg.bMIMOPSEnable = TRUE;
        }
        DBGPRINT(RT_DEBUG_TRACE, ("HT: MIMOPSEnable  = %s\n", (Value==0) ? "Disable" : "Enable"));
    }


    if (RTMPGetKeyParameter("HT_MIMOPSMode", pValueStr, 25, pInput, TRUE))
    {
        Value = simple_strtol(pValueStr, 0, 10);
        if (Value > MMPS_ENABLE)
        {
			pAd->CommonCfg.BACapability.field.MMPSmode = MMPS_ENABLE;
        }
        else
        {
            
            pAd->CommonCfg.BACapability.field.MMPSmode = MMPS_ENABLE;
			
        }
        DBGPRINT(RT_DEBUG_TRACE, ("HT: MIMOPS Mode  = %d\n", (INT) Value));
    }

    if (RTMPGetKeyParameter("HT_BADecline", pValueStr, 25, pInput, TRUE))
    {
        Value = simple_strtol(pValueStr, 0, 10);
        if (Value == 0)
        {
            pAd->CommonCfg.bBADecline = FALSE;
        }
        else
        {
            pAd->CommonCfg.bBADecline = TRUE;
        }
        DBGPRINT(RT_DEBUG_TRACE, ("HT: BA Decline  = %s\n", (Value==0) ? "Disable" : "Enable"));
    }


    if (RTMPGetKeyParameter("HT_DisableReordering", pValueStr, 25, pInput, TRUE))
    {
        Value = simple_strtol(pValueStr, 0, 10);
        if (Value == 0)
        {
            pAd->CommonCfg.bDisableReordering = FALSE;
        }
        else
        {
            pAd->CommonCfg.bDisableReordering = TRUE;
        }
        DBGPRINT(RT_DEBUG_TRACE, ("HT: DisableReordering  = %s\n", (Value==0) ? "Disable" : "Enable"));
    }

    if (RTMPGetKeyParameter("HT_AutoBA", pValueStr, 25, pInput, TRUE))
    {
        Value = simple_strtol(pValueStr, 0, 10);
        if (Value == 0)
        {
            pAd->CommonCfg.BACapability.field.AutoBA = FALSE;
			pAd->CommonCfg.BACapability.field.Policy = BA_NOTUSE;
        }
        else
        {
            pAd->CommonCfg.BACapability.field.AutoBA = TRUE;
			pAd->CommonCfg.BACapability.field.Policy = IMMED_BA;
        }
        pAd->CommonCfg.REGBACapability.field.AutoBA = pAd->CommonCfg.BACapability.field.AutoBA;
        DBGPRINT(RT_DEBUG_TRACE, ("HT: Auto BA  = %s\n", (Value==0) ? "Disable" : "Enable"));
    }

	
    if (RTMPGetKeyParameter("HT_HTC", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value == 0)
		{
			pAd->HTCEnable = FALSE;
		}
		else
		{
            pAd->HTCEnable = TRUE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("HT: Tx +HTC frame = %s\n", (Value==0) ? "Disable" : "Enable"));
	}

	
	if (RTMPGetKeyParameter("HT_LinkAdapt", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value == 0)
		{
			pAd->bLinkAdapt = FALSE;
		}
		else
		{
			pAd->HTCEnable = TRUE;
			pAd->bLinkAdapt = TRUE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("HT: Link Adaptation Control = %s\n", (Value==0) ? "Disable" : "Enable(+HTC)"));
	}

	
    if (RTMPGetKeyParameter("HT_RDG", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value == 0)
		{
			pAd->CommonCfg.bRdg = FALSE;
		}
		else
		{
			pAd->HTCEnable = TRUE;
            pAd->CommonCfg.bRdg = TRUE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("HT: RDG = %s\n", (Value==0) ? "Disable" : "Enable(+HTC)"));
	}




	
    if (RTMPGetKeyParameter("HT_AMSDU", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value == 0)
		{
			pAd->CommonCfg.BACapability.field.AmsduEnable = FALSE;
		}
		else
		{
            pAd->CommonCfg.BACapability.field.AmsduEnable = TRUE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("HT: Tx A-MSDU = %s\n", (Value==0) ? "Disable" : "Enable"));
	}

	
    if (RTMPGetKeyParameter("HT_MpduDensity", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value <=7 && Value >= 0)
		{
			pAd->CommonCfg.BACapability.field.MpduDensity = Value;
			DBGPRINT(RT_DEBUG_TRACE, ("HT: MPDU Density = %d\n", (INT) Value));
		}
		else
		{
			pAd->CommonCfg.BACapability.field.MpduDensity = 4;
			DBGPRINT(RT_DEBUG_TRACE, ("HT: MPDU Density = %d (Default)\n", 4));
		}
	}

	
    if (RTMPGetKeyParameter("HT_BAWinSize", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value >=1 && Value <= 64)
		{
			pAd->CommonCfg.REGBACapability.field.RxBAWinLimit = Value;
			pAd->CommonCfg.BACapability.field.RxBAWinLimit = Value;
			DBGPRINT(RT_DEBUG_TRACE, ("HT: BA Windw Size = %d\n", (INT) Value));
		}
		else
		{
            pAd->CommonCfg.REGBACapability.field.RxBAWinLimit = 64;
			pAd->CommonCfg.BACapability.field.RxBAWinLimit = 64;
			DBGPRINT(RT_DEBUG_TRACE, ("HT: BA Windw Size = 64 (Defualt)\n"));
		}

	}

	
	if (RTMPGetKeyParameter("HT_GI", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == GI_400)
		{
			pAd->CommonCfg.RegTransmitSetting.field.ShortGI = GI_400;
		}
		else
		{
			pAd->CommonCfg.RegTransmitSetting.field.ShortGI = GI_800;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("HT: Guard Interval = %s\n", (Value==GI_400) ? "400" : "800" ));
	}

	
	if (RTMPGetKeyParameter("HT_OpMode", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == HTMODE_GF)
		{

			pAd->CommonCfg.RegTransmitSetting.field.HTMODE  = HTMODE_GF;
		}
		else
		{
			pAd->CommonCfg.RegTransmitSetting.field.HTMODE  = HTMODE_MM;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("HT: Operate Mode = %s\n", (Value==HTMODE_GF) ? "Green Field" : "Mixed Mode" ));
	}

	
	if (RTMPGetKeyParameter("FixedTxMode", pValueStr, 25, pInput, TRUE))
	{
		UCHAR	fix_tx_mode;

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			fix_tx_mode = FIXED_TXMODE_HT;

			if (strcmp(pValueStr, "OFDM") == 0 || strcmp(pValueStr, "ofdm") == 0)
			{
				fix_tx_mode = FIXED_TXMODE_OFDM;
			}
			else if (strcmp(pValueStr, "CCK") == 0 || strcmp(pValueStr, "cck") == 0)
			{
		        fix_tx_mode = FIXED_TXMODE_CCK;
			}
			else if (strcmp(pValueStr, "HT") == 0 || strcmp(pValueStr, "ht") == 0)
			{
		        fix_tx_mode = FIXED_TXMODE_HT;
		}
		else
		{
				Value = simple_strtol(pValueStr, 0, 10);
				
				
				
				if (Value == FIXED_TXMODE_CCK || Value == FIXED_TXMODE_OFDM)
					fix_tx_mode = Value;
				else
					fix_tx_mode = FIXED_TXMODE_HT;
		}

			pAd->StaCfg.DesiredTransmitSetting.field.FixedTxMode = fix_tx_mode;
			DBGPRINT(RT_DEBUG_TRACE, ("Fixed Tx Mode = %d\n", fix_tx_mode));

		}
#endif 
	}


	
	if (RTMPGetKeyParameter("HT_BW", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == BW_40)
		{
			pAd->CommonCfg.RegTransmitSetting.field.BW  = BW_40;
		}
		else
		{
            pAd->CommonCfg.RegTransmitSetting.field.BW  = BW_20;
		}

#ifdef MCAST_RATE_SPECIFIC
		pAd->CommonCfg.MCastPhyMode.field.BW = pAd->CommonCfg.RegTransmitSetting.field.BW;
#endif 

		DBGPRINT(RT_DEBUG_TRACE, ("HT: Channel Width = %s\n", (Value==BW_40) ? "40 MHz" : "20 MHz" ));
	}

	if (RTMPGetKeyParameter("HT_EXTCHA", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == 0)
		{

			pAd->CommonCfg.RegTransmitSetting.field.EXTCHA  = EXTCHA_BELOW;
		}
		else
		{
            pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_ABOVE;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("HT: Ext Channel = %s\n", (Value==0) ? "BELOW" : "ABOVE" ));
	}

	
	if (RTMPGetKeyParameter("HT_MCS", pValueStr, 50, pInput, TRUE))
	{

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			Value = simple_strtol(pValueStr, 0, 10);


			if ((Value >= 0 && Value <= 23) || (Value == 32)) 
		{
				pAd->StaCfg.DesiredTransmitSetting.field.MCS  = Value;
				pAd->StaCfg.bAutoTxRateSwitch = FALSE;
				DBGPRINT(RT_DEBUG_TRACE, ("HT: MCS = %d\n", pAd->StaCfg.DesiredTransmitSetting.field.MCS));
		}
		else
		{
				pAd->StaCfg.DesiredTransmitSetting.field.MCS  = MCS_AUTO;
				pAd->StaCfg.bAutoTxRateSwitch = TRUE;
				DBGPRINT(RT_DEBUG_TRACE, ("HT: MCS = AUTO\n"));
		}
	}
#endif 
	}

	
    if (RTMPGetKeyParameter("HT_STBC", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value == STBC_USE)
		{
			pAd->CommonCfg.RegTransmitSetting.field.STBC = STBC_USE;
		}
		else
		{
			pAd->CommonCfg.RegTransmitSetting.field.STBC = STBC_NONE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("HT: STBC = %d\n", pAd->CommonCfg.RegTransmitSetting.field.STBC));
	}

	
	if (RTMPGetKeyParameter("HT_40MHZ_INTOLERANT", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value == 0)
		{
			pAd->CommonCfg.bForty_Mhz_Intolerant = FALSE;
		}
		else
		{
			pAd->CommonCfg.bForty_Mhz_Intolerant = TRUE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("HT: 40MHZ INTOLERANT = %d\n", pAd->CommonCfg.bForty_Mhz_Intolerant));
	}
	
	if(RTMPGetKeyParameter("HT_TxStream", pValueStr, 10, pInput, TRUE))
	{
		switch (simple_strtol(pValueStr, 0, 10))
		{
			case 1:
				pAd->CommonCfg.TxStream = 1;
				break;
			case 2:
				pAd->CommonCfg.TxStream = 2;
				break;
			case 3: 
			default:
				pAd->CommonCfg.TxStream = 3;

				if (pAd->MACVersion < RALINK_2883_VERSION)
					pAd->CommonCfg.TxStream = 2; 
				break;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("HT: Tx Stream = %d\n", pAd->CommonCfg.TxStream));
	}
	
	if(RTMPGetKeyParameter("HT_RxStream", pValueStr, 10, pInput, TRUE))
	{
		switch (simple_strtol(pValueStr, 0, 10))
		{
			case 1:
				pAd->CommonCfg.RxStream = 1;
				break;
			case 2:
				pAd->CommonCfg.RxStream = 2;
				break;
			case 3:
			default:
				pAd->CommonCfg.RxStream = 3;

				if (pAd->MACVersion < RALINK_2883_VERSION)
					pAd->CommonCfg.RxStream = 2; 
				break;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("HT: Rx Stream = %d\n", pAd->CommonCfg.RxStream));
	}
	
	
	if(RTMPGetKeyParameter("GreenAP", pValueStr, 10, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);
		if (Value == 0)
		{
			pAd->CommonCfg.bGreenAPEnable = FALSE;
		}
		else
		{
			pAd->CommonCfg.bGreenAPEnable = TRUE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("HT: Green AP= %d\n", pAd->CommonCfg.bGreenAPEnable));
	}

	
	if (RTMPGetKeyParameter("HT_DisallowTKIP", pValueStr, 25, pInput, TRUE))
	{
		Value = simple_strtol(pValueStr, 0, 10);

		if (Value == 1)
		{
			pAd->CommonCfg.HT_DisallowTKIP = TRUE;
		}
		else
		{
			pAd->CommonCfg.HT_DisallowTKIP = FALSE;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("HT: Disallow TKIP mode = %s\n", (pAd->CommonCfg.HT_DisallowTKIP == TRUE) ? "ON" : "OFF" ));
	}


	
}
#endif 


NDIS_STATUS	RTMPSetProfileParameters(
	IN RTMP_ADAPTER *pAd,
	IN PSTRING	pBuffer)
{
	PSTRING					tmpbuf;
	ULONG					RtsThresh;
	ULONG					FragThresh;
	PSTRING					macptr;
	INT						i = 0, retval;
	tmpbuf = kmalloc(MAX_PARAM_BUFFER_SIZE, MEM_ALLOC_FLAG);
	if(tmpbuf == NULL)
		return NDIS_STATUS_FAILURE;

	do
	{
		
		
		if(RTMPGetKeyParameter("CountryRegion", tmpbuf, 25, pBuffer, TRUE))
		{
			retval = RT_CfgSetCountryRegion(pAd, tmpbuf, BAND_24G);
			DBGPRINT(RT_DEBUG_TRACE, ("CountryRegion=%d\n", pAd->CommonCfg.CountryRegion));
		}
		
		if(RTMPGetKeyParameter("CountryRegionABand", tmpbuf, 25, pBuffer, TRUE))
		{
			retval = RT_CfgSetCountryRegion(pAd, tmpbuf, BAND_5G);
			DBGPRINT(RT_DEBUG_TRACE, ("CountryRegionABand=%d\n", pAd->CommonCfg.CountryRegionForABand));
		}
#ifdef RTMP_EFUSE_SUPPORT
#ifdef RT30xx
		
		if(RTMPGetKeyParameter("EfuseBufferMode", tmpbuf, 25, pBuffer, TRUE))
		{
			pAd->bEEPROMFile = (UCHAR) simple_strtol(tmpbuf, 0, 10);
			DBGPRINT(RT_DEBUG_TRACE, ("EfuseBufferMode=%d\n", pAd->bUseEfuse));
		}
#endif 
#endif 
		
		if(RTMPGetKeyParameter("CountryCode", tmpbuf, 25, pBuffer, TRUE))
		{
			NdisMoveMemory(pAd->CommonCfg.CountryCode, tmpbuf , 2);
#ifdef CONFIG_STA_SUPPORT
#ifdef EXT_BUILD_CHANNEL_LIST
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				NdisMoveMemory(pAd->StaCfg.StaOriCountryCode, tmpbuf , 2);
#endif 
#endif 
			if (strlen((PSTRING) pAd->CommonCfg.CountryCode) != 0)
			{
				pAd->CommonCfg.bCountryFlag = TRUE;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("CountryCode=%s\n", pAd->CommonCfg.CountryCode));
		}
		
		if(RTMPGetKeyParameter("ChannelGeography", tmpbuf, 25, pBuffer, TRUE))
		{
			UCHAR Geography = (UCHAR) simple_strtol(tmpbuf, 0, 10);
			if (Geography <= BOTH)
			{
				pAd->CommonCfg.Geography = Geography;
				pAd->CommonCfg.CountryCode[2] =
					(pAd->CommonCfg.Geography == BOTH) ? ' ' : ((pAd->CommonCfg.Geography == IDOR) ? 'I' : 'O');
#ifdef CONFIG_STA_SUPPORT
#ifdef EXT_BUILD_CHANNEL_LIST
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
					pAd->StaCfg.StaOriGeography = pAd->CommonCfg.Geography;
#endif 
#endif 
				DBGPRINT(RT_DEBUG_TRACE, ("ChannelGeography=%d\n", pAd->CommonCfg.Geography));
			}
		}
		else
		{
			pAd->CommonCfg.Geography = BOTH;
			pAd->CommonCfg.CountryCode[2] = ' ';
		}


#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			
			if (RTMPGetKeyParameter("SSID", tmpbuf, 256, pBuffer, FALSE))
			{
				if (strlen(tmpbuf) <= 32)
				{
						pAd->CommonCfg.SsidLen = (UCHAR) strlen(tmpbuf);
					NdisZeroMemory(pAd->CommonCfg.Ssid, NDIS_802_11_LENGTH_SSID);
					NdisMoveMemory(pAd->CommonCfg.Ssid, tmpbuf, pAd->CommonCfg.SsidLen);
					pAd->MlmeAux.AutoReconnectSsidLen = pAd->CommonCfg.SsidLen;
					NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, NDIS_802_11_LENGTH_SSID);
					NdisMoveMemory(pAd->MlmeAux.AutoReconnectSsid, tmpbuf, pAd->MlmeAux.AutoReconnectSsidLen);
					pAd->MlmeAux.SsidLen = pAd->CommonCfg.SsidLen;
					NdisZeroMemory(pAd->MlmeAux.Ssid, NDIS_802_11_LENGTH_SSID);
					NdisMoveMemory(pAd->MlmeAux.Ssid, tmpbuf, pAd->MlmeAux.SsidLen);
					DBGPRINT(RT_DEBUG_TRACE, ("%s::(SSID=%s)\n", __FUNCTION__, tmpbuf));
				}
			}
		}
#endif 

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			
			if (RTMPGetKeyParameter("NetworkType", tmpbuf, 25, pBuffer, TRUE))
			{
				pAd->bConfigChanged = TRUE;
				if (strcmp(tmpbuf, "Adhoc") == 0)
					pAd->StaCfg.BssType = BSS_ADHOC;
				else 
					pAd->StaCfg.BssType = BSS_INFRA;
				
				pAd->StaCfg.WpaState = SS_NOTUSE;
				DBGPRINT(RT_DEBUG_TRACE, ("%s::(NetworkType=%d)\n", __FUNCTION__, pAd->StaCfg.BssType));
			}
		}
#ifdef RTMP_MAC_PCI
		
		if(RTMPGetKeyParameter("NewPCIePS", tmpbuf, 10, pBuffer, TRUE))
		{
			UCHAR temp_buffer = (UCHAR) simple_strtol(tmpbuf, 0, 10);
			if(temp_buffer>0)
				pAd->StaCfg.PSControl.field.EnableNewPS=TRUE;
				else
					pAd->StaCfg.PSControl.field.EnableNewPS=FALSE;
			DBGPRINT(RT_DEBUG_TRACE, ("NewPCIePS=%d\n", pAd->StaCfg.PSControl.field.EnableNewPS));
		}
#endif 
#ifdef RT3090
		

		if(RTMPGetKeyParameter("PCIePowerLevel", tmpbuf, 10, pBuffer, TRUE))
		{
			pAd->StaCfg.PSControl.field.rt30xxPowerMode = (UCHAR) simple_strtol(tmpbuf, 0, 10);
			DBGPRINT(RT_DEBUG_TRACE, ("PCIePowerLevel=%d\n", pAd->StaCfg.PSControl.field.rt30xxPowerMode));
		}
		
		if(RTMPGetKeyParameter("FollowHostASPM", tmpbuf, 10, pBuffer, TRUE))
		{
			UCHAR temp_buffer = (UCHAR) simple_strtol(tmpbuf, 0, 10);

			if(temp_buffer>0)
				pAd->StaCfg.PSControl.field.rt30xxFollowHostASPM=TRUE;
				else
					pAd->StaCfg.PSControl.field.rt30xxFollowHostASPM=FALSE;
			DBGPRINT(RT_DEBUG_TRACE, ("rt30xxFollowHostASPM=%d\n", pAd->StaCfg.PSControl.field.rt30xxFollowHostASPM));
		}
		
		if(RTMPGetKeyParameter("ForceTestASPM", tmpbuf, 10, pBuffer, TRUE))
		{
			UCHAR temp_buffer = (UCHAR) simple_strtol(tmpbuf, 0, 10);

			if(temp_buffer>0)
				pAd->StaCfg.PSControl.field.rt30xxForceASPMTest=TRUE;
				else
					pAd->StaCfg.PSControl.field.rt30xxForceASPMTest=FALSE;
			DBGPRINT(RT_DEBUG_TRACE, ("rt30xxForceASPM=%d\n", pAd->StaCfg.PSControl.field.rt30xxForceASPMTest));
		}
#endif 
#endif 
		
		if(RTMPGetKeyParameter("Channel", tmpbuf, 10, pBuffer, TRUE))
		{
			pAd->CommonCfg.Channel = (UCHAR) simple_strtol(tmpbuf, 0, 10);
			DBGPRINT(RT_DEBUG_TRACE, ("Channel=%d\n", pAd->CommonCfg.Channel));
		}
		
		if(RTMPGetKeyParameter("WirelessMode", tmpbuf, 10, pBuffer, TRUE))
		{
			RT_CfgSetWirelessMode(pAd, tmpbuf);
			DBGPRINT(RT_DEBUG_TRACE, ("PhyMode=%d\n", pAd->CommonCfg.PhyMode));
		}
	    
		if(RTMPGetKeyParameter("BasicRate", tmpbuf, 10, pBuffer, TRUE))
		{
			pAd->CommonCfg.BasicRateBitmap = (ULONG) simple_strtol(tmpbuf, 0, 10);
			DBGPRINT(RT_DEBUG_TRACE, ("BasicRate=%ld\n", pAd->CommonCfg.BasicRateBitmap));
		}
		
		if(RTMPGetKeyParameter("BeaconPeriod", tmpbuf, 10, pBuffer, TRUE))
		{
			pAd->CommonCfg.BeaconPeriod = (USHORT) simple_strtol(tmpbuf, 0, 10);
			DBGPRINT(RT_DEBUG_TRACE, ("BeaconPeriod=%d\n", pAd->CommonCfg.BeaconPeriod));
		}
	    
		if(RTMPGetKeyParameter("TxPower", tmpbuf, 10, pBuffer, TRUE))
		{
			pAd->CommonCfg.TxPowerPercentage = (ULONG) simple_strtol(tmpbuf, 0, 10);
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				pAd->CommonCfg.TxPowerDefault = pAd->CommonCfg.TxPowerPercentage;
#endif 
			DBGPRINT(RT_DEBUG_TRACE, ("TxPower=%ld\n", pAd->CommonCfg.TxPowerPercentage));
		}
		
		if(RTMPGetKeyParameter("BGProtection", tmpbuf, 10, pBuffer, TRUE))
		{
	
	
	
			switch (simple_strtol(tmpbuf, 0, 10))
			{
				case 1: 
					pAd->CommonCfg.UseBGProtection = 1;
					break;
				case 2: 
					pAd->CommonCfg.UseBGProtection = 2;
					break;
				case 0: 
				default:
					pAd->CommonCfg.UseBGProtection = 0;
					break;
			}
	
			DBGPRINT(RT_DEBUG_TRACE, ("BGProtection=%ld\n", pAd->CommonCfg.UseBGProtection));
		}
		
		if(RTMPGetKeyParameter("DisableOLBC", tmpbuf, 10, pBuffer, TRUE))
		{
			switch (simple_strtol(tmpbuf, 0, 10))
			{
				case 1: 
					pAd->CommonCfg.DisableOLBCDetect = 1;
					break;
				case 0: 
					pAd->CommonCfg.DisableOLBCDetect = 0;
					break;
				default:
					pAd->CommonCfg.DisableOLBCDetect= 0;
					break;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("OLBCDetection=%ld\n", pAd->CommonCfg.DisableOLBCDetect));
		}
		
		if(RTMPGetKeyParameter("TxPreamble", tmpbuf, 10, pBuffer, TRUE))
		{
			switch (simple_strtol(tmpbuf, 0, 10))
			{
				case Rt802_11PreambleShort:
					pAd->CommonCfg.TxPreamble = Rt802_11PreambleShort;
					break;
				case Rt802_11PreambleLong:
				default:
					pAd->CommonCfg.TxPreamble = Rt802_11PreambleLong;
					break;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("TxPreamble=%ld\n", pAd->CommonCfg.TxPreamble));
		}
		
		if(RTMPGetKeyParameter("RTSThreshold", tmpbuf, 10, pBuffer, TRUE))
		{
			RtsThresh = simple_strtol(tmpbuf, 0, 10);
			if( (RtsThresh >= 1) && (RtsThresh <= MAX_RTS_THRESHOLD) )
				pAd->CommonCfg.RtsThreshold  = (USHORT)RtsThresh;
			else
				pAd->CommonCfg.RtsThreshold = MAX_RTS_THRESHOLD;

			DBGPRINT(RT_DEBUG_TRACE, ("RTSThreshold=%d\n", pAd->CommonCfg.RtsThreshold));
		}
		
		if(RTMPGetKeyParameter("FragThreshold", tmpbuf, 10, pBuffer, TRUE))
		{
			FragThresh = simple_strtol(tmpbuf, 0, 10);
			pAd->CommonCfg.bUseZeroToDisableFragment = FALSE;

			if (FragThresh > MAX_FRAG_THRESHOLD || FragThresh < MIN_FRAG_THRESHOLD)
			{ 
				pAd->CommonCfg.FragmentThreshold = MAX_FRAG_THRESHOLD;
				pAd->CommonCfg.bUseZeroToDisableFragment = TRUE;
			}
			else if (FragThresh % 2 == 1)
			{
				
				
				pAd->CommonCfg.FragmentThreshold = (USHORT)(FragThresh - 1);
			}
			else
			{
				pAd->CommonCfg.FragmentThreshold = (USHORT)FragThresh;
			}
			
			DBGPRINT(RT_DEBUG_TRACE, ("FragThreshold=%d\n", pAd->CommonCfg.FragmentThreshold));
		}
		
		if(RTMPGetKeyParameter("TxBurst", tmpbuf, 10, pBuffer, TRUE))
		{
	
	
	
			if(simple_strtol(tmpbuf, 0, 10) != 0)  
				pAd->CommonCfg.bEnableTxBurst = TRUE;
			else 
				pAd->CommonCfg.bEnableTxBurst = FALSE;
	
			DBGPRINT(RT_DEBUG_TRACE, ("TxBurst=%d\n", pAd->CommonCfg.bEnableTxBurst));
		}

#ifdef AGGREGATION_SUPPORT
		
		if(RTMPGetKeyParameter("PktAggregate", tmpbuf, 10, pBuffer, TRUE))
		{
			if(simple_strtol(tmpbuf, 0, 10) != 0)  
				pAd->CommonCfg.bAggregationCapable = TRUE;
			else 
				pAd->CommonCfg.bAggregationCapable = FALSE;
#ifdef PIGGYBACK_SUPPORT
			pAd->CommonCfg.bPiggyBackCapable = pAd->CommonCfg.bAggregationCapable;
#endif 
			DBGPRINT(RT_DEBUG_TRACE, ("PktAggregate=%d\n", pAd->CommonCfg.bAggregationCapable));
		}
#else
		pAd->CommonCfg.bAggregationCapable = FALSE;
		pAd->CommonCfg.bPiggyBackCapable = FALSE;
#endif 

		

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			rtmp_read_sta_wmm_parms_from_file(pAd, tmpbuf, pBuffer);
#endif 

		
		if(RTMPGetKeyParameter("ShortSlot", tmpbuf, 10, pBuffer, TRUE))
		{
			RT_CfgSetShortSlot(pAd, tmpbuf);
			DBGPRINT(RT_DEBUG_TRACE, ("ShortSlot=%d\n", pAd->CommonCfg.bUseShortSlotTime));
		}
		
		if(RTMPGetKeyParameter("IEEE80211H", tmpbuf, 10, pBuffer, TRUE))
		{
		    for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		    {
				if(simple_strtol(macptr, 0, 10) != 0)  
					pAd->CommonCfg.bIEEE80211H = TRUE;
				else 
					pAd->CommonCfg.bIEEE80211H = FALSE;

				DBGPRINT(RT_DEBUG_TRACE, ("IEEE80211H=%d\n", pAd->CommonCfg.bIEEE80211H));
		    }
		}
		
		if(RTMPGetKeyParameter("CSPeriod", tmpbuf, 10, pBuffer, TRUE))
		{
		    if(simple_strtol(tmpbuf, 0, 10) != 0)
				pAd->CommonCfg.RadarDetect.CSPeriod = simple_strtol(tmpbuf, 0, 10);
			else
				pAd->CommonCfg.RadarDetect.CSPeriod = 0;

				DBGPRINT(RT_DEBUG_TRACE, ("CSPeriod=%d\n", pAd->CommonCfg.RadarDetect.CSPeriod));
		}

#ifdef MERGE_ARCH_TEAM
		
		if(RTMPGetKeyParameter("DfsLowerLimit", tmpbuf, 10, pBuffer, TRUE))
		{
			if(simple_strtol(tmpbuf, 0, 10) != 0)
				pAd->CommonCfg.RadarDetect.DfsLowerLimit = simple_strtol(tmpbuf, 0, 10);

			DBGPRINT(RT_DEBUG_TRACE, ("DfsLowerLimit=%ld\n", pAd->CommonCfg.RadarDetect.DfsLowerLimit));
		}

		
		if(RTMPGetKeyParameter("DfsUpperLimit", tmpbuf, 10, pBuffer, TRUE))
		{
			if(simple_strtol(tmpbuf, 0, 10) != 0)
				pAd->CommonCfg.RadarDetect.DfsUpperLimit = simple_strtol(tmpbuf, 0, 10);

			DBGPRINT(RT_DEBUG_TRACE, ("DfsUpperLimit=%ld\n", pAd->CommonCfg.RadarDetect.DfsUpperLimit));
		}

		
		if(RTMPGetKeyParameter("FixDfsLimit", tmpbuf, 10, pBuffer, TRUE))
		{
		    if(simple_strtol(tmpbuf, 0, 10) != 0)
				pAd->CommonCfg.RadarDetect.FixDfsLimit = TRUE;
			else
				pAd->CommonCfg.RadarDetect.FixDfsLimit = FALSE;

			DBGPRINT(RT_DEBUG_TRACE, ("FixDfsLimit=%d\n", pAd->CommonCfg.RadarDetect.FixDfsLimit));
		}

		
		if(RTMPGetKeyParameter("LongPulseRadarTh", tmpbuf, 10, pBuffer, TRUE))
		{
		    if(simple_strtol(tmpbuf, 0, 10) != 0)
				pAd->CommonCfg.RadarDetect.LongPulseRadarTh = simple_strtol(tmpbuf, 0, 10);

			DBGPRINT(RT_DEBUG_TRACE, ("LongPulseRadarTh=%d\n", pAd->CommonCfg.RadarDetect.LongPulseRadarTh));
		}

		
		if(RTMPGetKeyParameter("AvgRssiReq", tmpbuf, 10, pBuffer, TRUE))
		{
			if(simple_strtol(tmpbuf, 0, 10) != 0)
				pAd->CommonCfg.RadarDetect.AvgRssiReq = simple_strtol(tmpbuf, 0, 10);

			DBGPRINT(RT_DEBUG_TRACE, ("AvgRssiReq=%d\n", pAd->CommonCfg.RadarDetect.AvgRssiReq));
		}

#endif 

		
		if(RTMPGetKeyParameter("RDRegion", tmpbuf, 128, pBuffer, TRUE))
		{
						RADAR_DETECT_STRUCT	*pRadarDetect = &pAd->CommonCfg.RadarDetect;
			if ((strncmp(tmpbuf, "JAP_W53", 7) == 0) || (strncmp(tmpbuf, "jap_w53", 7) == 0))
			{
							pRadarDetect->RDDurRegion = JAP_W53;
							pRadarDetect->DfsSessionTime = 15;
			}
			else if ((strncmp(tmpbuf, "JAP_W56", 7) == 0) || (strncmp(tmpbuf, "jap_w56", 7) == 0))
			{
							pRadarDetect->RDDurRegion = JAP_W56;
							pRadarDetect->DfsSessionTime = 13;
			}
			else if ((strncmp(tmpbuf, "JAP", 3) == 0) || (strncmp(tmpbuf, "jap", 3) == 0))
			{
							pRadarDetect->RDDurRegion = JAP;
							pRadarDetect->DfsSessionTime = 5;
			}
			else  if ((strncmp(tmpbuf, "FCC", 3) == 0) || (strncmp(tmpbuf, "fcc", 3) == 0))
			{
							pRadarDetect->RDDurRegion = FCC;
							pRadarDetect->DfsSessionTime = 5;
#ifdef DFS_FCC_BW40_FIX
							pRadarDetect->DfsSessionFccOff = 0;
#endif 
			}
			else if ((strncmp(tmpbuf, "CE", 2) == 0) || (strncmp(tmpbuf, "ce", 2) == 0))
			{
							pRadarDetect->RDDurRegion = CE;
							pRadarDetect->DfsSessionTime = 13;
			}
			else
			{
							pRadarDetect->RDDurRegion = CE;
							pRadarDetect->DfsSessionTime = 13;
			}

						DBGPRINT(RT_DEBUG_TRACE, ("RDRegion=%d\n", pRadarDetect->RDDurRegion));
		}
		else
		{
			pAd->CommonCfg.RadarDetect.RDDurRegion = CE;
			pAd->CommonCfg.RadarDetect.DfsSessionTime = 13;
		}

		
		if(RTMPGetKeyParameter("WirelessEvent", tmpbuf, 10, pBuffer, TRUE))
		{
#if WIRELESS_EXT >= 15
		    if(simple_strtol(tmpbuf, 0, 10) != 0)
				pAd->CommonCfg.bWirelessEvent = simple_strtol(tmpbuf, 0, 10);
			else
				pAd->CommonCfg.bWirelessEvent = 0;	
#else
			pAd->CommonCfg.bWirelessEvent = 0;	
#endif
				DBGPRINT(RT_DEBUG_TRACE, ("WirelessEvent=%d\n", pAd->CommonCfg.bWirelessEvent));
		}
		if(RTMPGetKeyParameter("WiFiTest", tmpbuf, 10, pBuffer, TRUE))
		{
		    if(simple_strtol(tmpbuf, 0, 10) != 0)
				pAd->CommonCfg.bWiFiTest= simple_strtol(tmpbuf, 0, 10);
			else
				pAd->CommonCfg.bWiFiTest = 0;	

				DBGPRINT(RT_DEBUG_TRACE, ("WiFiTest=%d\n", pAd->CommonCfg.bWiFiTest));
		}
		
		if(RTMPGetKeyParameter("AuthMode", tmpbuf, 128, pBuffer, TRUE))
		{
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
				if ((strcmp(tmpbuf, "WEPAUTO") == 0) || (strcmp(tmpbuf, "wepauto") == 0))
	                            pAd->StaCfg.AuthMode = Ndis802_11AuthModeAutoSwitch;
	                        else if ((strcmp(tmpbuf, "SHARED") == 0) || (strcmp(tmpbuf, "shared") == 0))
	                            pAd->StaCfg.AuthMode = Ndis802_11AuthModeShared;
	                        else if ((strcmp(tmpbuf, "WPAPSK") == 0) || (strcmp(tmpbuf, "wpapsk") == 0))
	                            pAd->StaCfg.AuthMode = Ndis802_11AuthModeWPAPSK;
	                        else if ((strcmp(tmpbuf, "WPANONE") == 0) || (strcmp(tmpbuf, "wpanone") == 0))
	                            pAd->StaCfg.AuthMode = Ndis802_11AuthModeWPANone;
	                        else if ((strcmp(tmpbuf, "WPA2PSK") == 0) || (strcmp(tmpbuf, "wpa2psk") == 0))
							    pAd->StaCfg.AuthMode = Ndis802_11AuthModeWPA2PSK;
#ifdef WPA_SUPPLICANT_SUPPORT
							else if ((strcmp(tmpbuf, "WPA") == 0) || (strcmp(tmpbuf, "wpa") == 0))
			                    pAd->StaCfg.AuthMode = Ndis802_11AuthModeWPA;
							else if ((strcmp(tmpbuf, "WPA2") == 0) || (strcmp(tmpbuf, "wpa2") == 0))
							    pAd->StaCfg.AuthMode = Ndis802_11AuthModeWPA2;
#endif 
	                        else
	                            pAd->StaCfg.AuthMode = Ndis802_11AuthModeOpen;

	                        pAd->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;

				DBGPRINT(RT_DEBUG_TRACE, ("%s::(EncrypType=%d)\n", __FUNCTION__, pAd->StaCfg.WepStatus));
			}
#endif 
		}
		
		if(RTMPGetKeyParameter("EncrypType", tmpbuf, 128, pBuffer, TRUE))
		{

#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
				if ((strcmp(tmpbuf, "WEP") == 0) || (strcmp(tmpbuf, "wep") == 0))
					pAd->StaCfg.WepStatus	= Ndis802_11WEPEnabled;
				else if ((strcmp(tmpbuf, "TKIP") == 0) || (strcmp(tmpbuf, "tkip") == 0))
					pAd->StaCfg.WepStatus	= Ndis802_11Encryption2Enabled;
				else if ((strcmp(tmpbuf, "AES") == 0) || (strcmp(tmpbuf, "aes") == 0))
					pAd->StaCfg.WepStatus	= Ndis802_11Encryption3Enabled;
				else
					pAd->StaCfg.WepStatus	= Ndis802_11WEPDisabled;

				
				pAd->StaCfg.PairCipher		= pAd->StaCfg.WepStatus;
				pAd->StaCfg.GroupCipher		= pAd->StaCfg.WepStatus;
				pAd->StaCfg.OrigWepStatus	= pAd->StaCfg.WepStatus;
				pAd->StaCfg.bMixCipher		= FALSE;

				
				DBGPRINT(RT_DEBUG_TRACE, ("%s::(EncrypType=%d)\n", __FUNCTION__, pAd->StaCfg.WepStatus));
			}
#endif 
		}


#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			if(RTMPGetKeyParameter("WPAPSK", tmpbuf, 512, pBuffer, FALSE))
			{
				int     ret = TRUE;

				tmpbuf[strlen(tmpbuf)] = '\0'; 

				if ((pAd->StaCfg.AuthMode != Ndis802_11AuthModeWPAPSK) &&
					(pAd->StaCfg.AuthMode != Ndis802_11AuthModeWPA2PSK) &&
					(pAd->StaCfg.AuthMode != Ndis802_11AuthModeWPANone)
					)
				{
					ret = FALSE;
				}
				else
				{
					ret = RT_CfgSetWPAPSKKey(pAd, tmpbuf, (PUCHAR)pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen, pAd->StaCfg.PMK);
				}

				if (ret == TRUE)
				{
			RTMPZeroMemory(pAd->StaCfg.WpaPassPhrase, 64);
			RTMPMoveMemory(pAd->StaCfg.WpaPassPhrase, tmpbuf, strlen(tmpbuf));
					pAd->StaCfg.WpaPassPhraseLen= strlen(tmpbuf);

					if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK) ||
						(pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
					{
						
						pAd->StaCfg.WpaState = SS_START;
					}
					else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPANone)
					{
						pAd->StaCfg.WpaState = SS_NOTUSE;
					}
					DBGPRINT(RT_DEBUG_TRACE, ("%s::(WPAPSK=%s)\n", __FUNCTION__, tmpbuf));
				}
			}
		}
#endif 

		
		rtmp_read_key_parms_from_file(pAd, tmpbuf, pBuffer);


		
		

#ifdef DOT11_N_SUPPORT
		HTParametersHook(pAd, tmpbuf, pBuffer);
#endif 


#ifdef CARRIER_DETECTION_SUPPORT
			
			if(RTMPGetKeyParameter("CarrierDetect", tmpbuf, 128, pBuffer, TRUE))
			{
				if ((strncmp(tmpbuf, "0", 1) == 0))
					pAd->CommonCfg.CarrierDetect.Enable = FALSE;
				else if ((strncmp(tmpbuf, "1", 1) == 0))
					pAd->CommonCfg.CarrierDetect.Enable = TRUE;
				else
					pAd->CommonCfg.CarrierDetect.Enable = FALSE;

				DBGPRINT(RT_DEBUG_TRACE, ("CarrierDetect.Enable=%d\n", pAd->CommonCfg.CarrierDetect.Enable));
			}
			else
				pAd->CommonCfg.CarrierDetect.Enable = FALSE;
#endif 

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			
			if (RTMPGetKeyParameter("PSMode", tmpbuf, 10, pBuffer, TRUE))
			{
				if (pAd->StaCfg.BssType == BSS_INFRA)
				{
					if ((strcmp(tmpbuf, "MAX_PSP") == 0) || (strcmp(tmpbuf, "max_psp") == 0))
					{
						
						
						
						OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
						if (pAd->StaCfg.bWindowsACCAMEnable == FALSE)
							pAd->StaCfg.WindowsPowerMode = Ndis802_11PowerModeMAX_PSP;
						pAd->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeMAX_PSP;
						pAd->StaCfg.DefaultListenCount = 5;
					}
					else if ((strcmp(tmpbuf, "Fast_PSP") == 0) || (strcmp(tmpbuf, "fast_psp") == 0)
						|| (strcmp(tmpbuf, "FAST_PSP") == 0))
					{
						
						
						
						OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
						if (pAd->StaCfg.bWindowsACCAMEnable == FALSE)
							pAd->StaCfg.WindowsPowerMode = Ndis802_11PowerModeFast_PSP;
						pAd->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeFast_PSP;
						pAd->StaCfg.DefaultListenCount = 3;
					}
					else if ((strcmp(tmpbuf, "Legacy_PSP") == 0) || (strcmp(tmpbuf, "legacy_psp") == 0)
						|| (strcmp(tmpbuf, "LEGACY_PSP") == 0))
					{
						
						
						
						OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
						if (pAd->StaCfg.bWindowsACCAMEnable == FALSE)
							pAd->StaCfg.WindowsPowerMode = Ndis802_11PowerModeLegacy_PSP;
						pAd->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeLegacy_PSP;
						pAd->StaCfg.DefaultListenCount = 3;
					}
					else
					{ 
						
						RTMP_SET_PSM_BIT(pAd, PWR_ACTIVE);
						OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
						if (pAd->StaCfg.bWindowsACCAMEnable == FALSE)
							pAd->StaCfg.WindowsPowerMode = Ndis802_11PowerModeCAM;
						pAd->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;
					}
					DBGPRINT(RT_DEBUG_TRACE, ("PSMode=%ld\n", pAd->StaCfg.WindowsPowerMode));
				}
			}
			
			if (RTMPGetKeyParameter("AutoRoaming", tmpbuf, 32, pBuffer, TRUE))
			{
				if (simple_strtol(tmpbuf, 0, 10) == 0)
					pAd->StaCfg.bAutoRoaming = FALSE;
				else
					pAd->StaCfg.bAutoRoaming = TRUE;

				DBGPRINT(RT_DEBUG_TRACE, ("AutoRoaming=%d\n", pAd->StaCfg.bAutoRoaming));
			}
			
			if (RTMPGetKeyParameter("RoamThreshold", tmpbuf, 32, pBuffer, TRUE))
			{
				long lInfo = simple_strtol(tmpbuf, 0, 10);

				if (lInfo > 90 || lInfo < 60)
					pAd->StaCfg.dBmToRoam = -70;
				else
					pAd->StaCfg.dBmToRoam = (CHAR)(-1)*lInfo;

				DBGPRINT(RT_DEBUG_TRACE, ("RoamThreshold=%d  dBm\n", pAd->StaCfg.dBmToRoam));
			}

			if(RTMPGetKeyParameter("TGnWifiTest", tmpbuf, 10, pBuffer, TRUE))
			{
				if(simple_strtol(tmpbuf, 0, 10) == 0)
					pAd->StaCfg.bTGnWifiTest = FALSE;
				else
					pAd->StaCfg.bTGnWifiTest = TRUE;
					DBGPRINT(RT_DEBUG_TRACE, ("TGnWifiTest=%d\n", pAd->StaCfg.bTGnWifiTest));
			}

			
			if (RTMPGetKeyParameter("BeaconLostTime", tmpbuf, 32, pBuffer, TRUE))
			{
				ULONG lInfo = (ULONG)simple_strtol(tmpbuf, 0, 10);

				if ((lInfo != 0) && (lInfo <= 60))
					pAd->StaCfg.BeaconLostTime = (lInfo * OS_HZ);
				DBGPRINT(RT_DEBUG_TRACE, ("BeaconLostTime=%ld \n", pAd->StaCfg.BeaconLostTime));
			}


		}
#endif 



#ifdef RT30xx
#ifdef ANT_DIVERSITY_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			if(RTMPGetKeyParameter("AntDiversity", tmpbuf, 10, pBuffer, TRUE))
			{
				for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
				{
					UCHAR Ant = simple_strtol(tmpbuf, 0, 10);
					if(Ant < 3)
						pAd->CommonCfg.bRxAntDiversity = Ant;
					else
						pAd->CommonCfg.bRxAntDiversity = ANT_DIVERSITY_DISABLE;

					DBGPRINT(RT_DEBUG_ERROR, ("AntDiversity=%d\n", pAd->CommonCfg.bRxAntDiversity));
				}
			}
		}
#endif 
#endif 

	}while(0);


	kfree(tmpbuf);

	return NDIS_STATUS_SUCCESS;

}


#ifdef MULTIPLE_CARD_SUPPORT

UINT8  MC_CardUsed[MAX_NUM_OF_MULTIPLE_CARD];

static UINT8  MC_CardMac[MAX_NUM_OF_MULTIPLE_CARD][6];


BOOLEAN RTMP_CardInfoRead(
	IN	PRTMP_ADAPTER pAd)
{
#define MC_SELECT_CARDID		0	
#define MC_SELECT_MAC			1	
#define MC_SELECT_CARDTYPE		2	

#define LETTER_CASE_TRANSLATE(txt_p, card_id)			\
	{	UINT32 _len; char _char;						\
		for(_len=0; _len<strlen(card_id); _len++) {		\
			_char = *(txt_p + _len);					\
			if (('A' <= _char) && (_char <= 'Z'))		\
				*(txt_p+_len) = 'a'+(_char-'A');		\
		} }

	RTMP_OS_FD srcf;
	INT retval;
	PSTRING buffer, tmpbuf;
	STRING card_id_buf[30], RFIC_word[30];
	BOOLEAN flg_match_ok = FALSE;
	INT32 card_select_method;
	INT32 card_free_id, card_nouse_id, card_same_mac_id, card_match_id;
	EEPROM_ANTENNA_STRUC antenna;
	USHORT addr01, addr23, addr45;
	UINT8 mac[6];
	UINT32 data, card_index;
	UCHAR *start_ptr;
	RTMP_OS_FS_INFO osFSInfo;

	
	buffer = kmalloc(MAX_INI_BUFFER_SIZE, MEM_ALLOC_FLAG);
	if (buffer == NULL)
		return FALSE;

	tmpbuf = kmalloc(MAX_PARAM_BUFFER_SIZE, MEM_ALLOC_FLAG);
	if(tmpbuf == NULL)
	{
		kfree(buffer);
		return NDIS_STATUS_FAILURE;
	}

	
	RTMP_IO_READ32(pAd, E2PROM_CSR, &data);

	if ((data & 0x30) == 0)
		pAd->EEPROMAddressNum = 6;	
	else if ((data & 0x30) == 0x10)
		pAd->EEPROMAddressNum = 8;	
	else
		pAd->EEPROMAddressNum = 8;	

	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC1_OFFSET, antenna.word);

	if ((antenna.field.RfIcType == RFIC_2850) ||
		(antenna.field.RfIcType == RFIC_2750))
	{
		
		strcpy(RFIC_word, "abgn");
	}
	else
	{
		
		strcpy(RFIC_word, "bgn");
	}

	
	RT28xx_EEPROM_READ16(pAd, 0x04, addr01);
	RT28xx_EEPROM_READ16(pAd, 0x06, addr23);
	RT28xx_EEPROM_READ16(pAd, 0x08, addr45);

	mac[0] = (UCHAR)(addr01 & 0xff);
	mac[1] = (UCHAR)(addr01 >> 8);
	mac[2] = (UCHAR)(addr23 & 0xff);
	mac[3] = (UCHAR)(addr23 >> 8);
	mac[4] = (UCHAR)(addr45 & 0xff);
	mac[5] = (UCHAR)(addr45 >> 8);

	DBGPRINT(RT_DEBUG_TRACE, ("mac addr=%02x:%02x:%02x:%02x:%02x:%02x!\n", PRINT_MAC(mac)));

	RtmpOSFSInfoChange(&osFSInfo, TRUE);
	
	srcf = RtmpOSFileOpen(CARD_INFO_PATH, O_RDONLY, 0);
	if (IS_FILE_OPEN_ERR(srcf))
	{
		
			DBGPRINT(RT_DEBUG_TRACE,
				("--> Error opening %s\n", CARD_INFO_PATH));
		goto  free_resource;
	}

	
	memset(buffer, 0x00, MAX_INI_BUFFER_SIZE);
	retval = RtmpOSFileRead(srcf, buffer, MAX_INI_BUFFER_SIZE);
	if (retval < 0)
	{
		
			DBGPRINT(RT_DEBUG_TRACE,
				("--> Read %s error %d\n", CARD_INFO_PATH, -retval));
	}
	else
	{
		
		memset(tmpbuf, 0x00, MAX_PARAM_BUFFER_SIZE);
		card_select_method = MC_SELECT_CARDTYPE; 

		if (RTMPGetKeyParameter("SELECT", tmpbuf, 256, buffer, TRUE))
		{
			if (strcmp(tmpbuf, "CARDID") == 0)
				card_select_method = MC_SELECT_CARDID;
			else if (strcmp(tmpbuf, "MAC") == 0)
				card_select_method = MC_SELECT_MAC;
			else if (strcmp(tmpbuf, "CARDTYPE") == 0)
				card_select_method = MC_SELECT_CARDTYPE;
		}

		DBGPRINT(RT_DEBUG_TRACE,
				("MC> Card Selection = %d\n", card_select_method));

		
		card_free_id = -1;
		card_nouse_id = -1;
		card_same_mac_id = -1;
		card_match_id = -1;

		
		for(card_index=0;
			card_index<MAX_NUM_OF_MULTIPLE_CARD;
			card_index++)
		{
			if ((*(UINT32 *)&MC_CardMac[card_index][0] == 0) &&
				(*(UINT16 *)&MC_CardMac[card_index][4] == 0))
			{
				
				MC_CardUsed[card_index] = 0;

				if (card_free_id < 0)
					card_free_id = card_index; 
			}
			else
			{
				if (memcmp(MC_CardMac[card_index], mac, 6) == 0)
				{
					
					if (card_same_mac_id < 0)
						card_same_mac_id = card_index; 
				}
				else
				{
					
					if ((MC_CardUsed[card_index] == 0) &&
						(card_nouse_id < 0))
					{
						card_nouse_id = card_index; 
					}
				}
			}
		}

		DBGPRINT(RT_DEBUG_TRACE,
				("MC> Free = %d, Same = %d, NOUSE = %d\n",
				card_free_id, card_same_mac_id, card_nouse_id));

		if ((card_same_mac_id >= 0) &&
			((card_select_method == MC_SELECT_CARDID) ||
			(card_select_method == MC_SELECT_CARDTYPE)))
		{
			
			card_match_id = card_same_mac_id;

			if (card_select_method == MC_SELECT_CARDTYPE)
			{
				
				sprintf(card_id_buf, "%02dCARDTYPE%s",
						card_match_id, RFIC_word);

				if ((start_ptr = (PUCHAR)rtstrstruncasecmp(buffer, card_id_buf)) != NULL)
				{
					
					LETTER_CASE_TRANSLATE(start_ptr, card_id_buf);
				}
			}
		}
		else
		{
			
			switch(card_select_method)
			{
				case MC_SELECT_CARDID: 
				default:
					if (card_free_id >= 0)
						card_match_id = card_free_id;
					else
						card_match_id = card_nouse_id;
					break;

				case MC_SELECT_MAC: 
					sprintf(card_id_buf, "MAC%02x:%02x:%02x:%02x:%02x:%02x",
							mac[0], mac[1], mac[2],
							mac[3], mac[4], mac[5]);

					
					if ((start_ptr = (PUCHAR)rtstrstruncasecmp(buffer, card_id_buf)) != NULL)
					{
						LETTER_CASE_TRANSLATE(start_ptr, card_id_buf);

						
						start_ptr -= 2;
						card_id_buf[0] = *(start_ptr);
						card_id_buf[1] = *(start_ptr+1);
						card_id_buf[2] = 0x00;

						card_match_id = simple_strtol(card_id_buf, 0, 10);
					}
					break;

				case MC_SELECT_CARDTYPE: 
					card_nouse_id = -1;

					for(card_index=0;
						card_index<MAX_NUM_OF_MULTIPLE_CARD;
						card_index++)
					{
						sprintf(card_id_buf, "%02dCARDTYPE%s",
								card_index, RFIC_word);

						if ((start_ptr = (PUCHAR)rtstrstruncasecmp(buffer,
													card_id_buf)) != NULL)
						{
							LETTER_CASE_TRANSLATE(start_ptr, card_id_buf);

							if (MC_CardUsed[card_index] == 0)
							{
								
								if ((*(UINT32 *)&MC_CardMac[card_index][0] == 0) &&
									(*(UINT16 *)&MC_CardMac[card_index][4] == 0))
								{
									
									card_match_id = card_index;
									break;
								}
								else
								{
									
									if (card_nouse_id < 0)
										card_nouse_id = card_index;
								}
							}
						}
					}

					
					if (card_match_id < 0)
						card_match_id = card_nouse_id;
					break;
			}
		}

		if (card_match_id >= 0)
		{
			
			switch(card_select_method)
			{
				case MC_SELECT_CARDID: 
					sprintf(card_id_buf, "%02dCARDID", card_match_id);
					break;

				case MC_SELECT_MAC: 
					sprintf(card_id_buf,
							"%02dmac%02x:%02x:%02x:%02x:%02x:%02x",
							card_match_id,
							mac[0], mac[1], mac[2],
							mac[3], mac[4], mac[5]);
					break;

				case MC_SELECT_CARDTYPE: 
				default:
					sprintf(card_id_buf, "%02dcardtype%s",
							card_match_id, RFIC_word);
					break;
			}

			DBGPRINT(RT_DEBUG_TRACE, ("Search Keyword = %s\n", card_id_buf));

			
			if (RTMPGetKeyParameter(card_id_buf, tmpbuf, 256, buffer, TRUE))
			{
				if (strlen(tmpbuf) < sizeof(pAd->MC_FileName))
				{
					
					pAd->MC_RowID = card_match_id; 
					MC_CardUsed[card_match_id] = 1;
					memcpy(MC_CardMac[card_match_id], mac, sizeof(mac));

					
					NdisMoveMemory(pAd->MC_FileName, tmpbuf , strlen(tmpbuf));
					pAd->MC_FileName[strlen(tmpbuf)] = '\0';
					flg_match_ok = TRUE;

					DBGPRINT(RT_DEBUG_TRACE,
							("Card Profile Name = %s\n", pAd->MC_FileName));
				}
				else
				{
					DBGPRINT(RT_DEBUG_ERROR,
							("Card Profile Name length too large!\n"));
				}
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR,
						("Can not find search key word in card.dat!\n"));
			}

			if ((flg_match_ok != TRUE) &&
				(card_match_id < MAX_NUM_OF_MULTIPLE_CARD))
			{
				MC_CardUsed[card_match_id] = 0;
				memset(MC_CardMac[card_match_id], 0, sizeof(mac));
			}
		} 
	}


	
	retval = RtmpOSFileClose(srcf);

free_resource:
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	kfree(buffer);
	kfree(tmpbuf);

	return flg_match_ok;
}
#endif 
