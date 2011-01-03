

#include "../rt_config.h"


typedef struct _RADAR_DURATION_TABLE
{
	ULONG RDDurRegion;
	ULONG RadarSignalDuration;
	ULONG Tolerance;
} RADAR_DURATION_TABLE, *PRADAR_DURATION_TABLE;



UCHAR RdIdleTimeTable[MAX_RD_REGION][4] =
{
	{9, 250, 250, 250},		
#ifdef DFS_FCC_BW40_FIX
	{1, 250, 250, 250},		
#else
	{4, 250, 250, 250},		
#endif
	{4, 250, 250, 250},		
	{15, 250, 250, 250},	
	{4, 250, 250, 250}		
};

#ifdef TONE_RADAR_DETECT_SUPPORT
static void ToneRadarProgram(PRTMP_ADAPTER pAd);
static void ToneRadarEnable(PRTMP_ADAPTER pAd);
#endif 

#ifdef DFS_SUPPORT

VOID BbpRadarDetectionStart(
	IN PRTMP_ADAPTER pAd)
{
	UINT8 RadarPeriod;

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 114, 0x02);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 121, 0x20);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 122, 0x00);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 123, 0x08);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 124, 0x28);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, 125, 0xff);

#ifdef MERGE_ARCH_TEAM
	if ((pAd->CommonCfg.RadarDetect.RDDurRegion == JAP) || (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W53) || (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56))
	{
		pAd->CommonCfg.RadarDetect.RDDurRegion = JAP;
		pAd->CommonCfg.RadarDetect.RDDurRegion = JapRadarType(pAd);
		if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W56)
		{
			pAd->CommonCfg.RadarDetect.DfsSessionTime = 13;
		}
		else if (pAd->CommonCfg.RadarDetect.RDDurRegion == JAP_W53)
		{
			pAd->CommonCfg.RadarDetect.DfsSessionTime = 15;
		}
#ifdef CARRIER_DETECTION_SUPPORT
		pAd->CommonCfg.CarrierDetect.Enable = 1;
#endif 
	}
#endif 

	RadarPeriod = ((UINT)RdIdleTimeTable[pAd->CommonCfg.RadarDetect.RDDurRegion][0] + (UINT)pAd->CommonCfg.RadarDetect.DfsSessionTime) < 250 ?
			(RdIdleTimeTable[pAd->CommonCfg.RadarDetect.RDDurRegion][0] + pAd->CommonCfg.RadarDetect.DfsSessionTime) : 250;

#ifdef MERGE_ARCH_TEAM


#else 
	RTMP_IO_WRITE8(pAd, 0x7020, 0x1d);
	RTMP_IO_WRITE8(pAd, 0x7021, 0x40);
#endif 

	RadarDetectionStart(pAd, 0, RadarPeriod);
	return;
}


VOID BbpRadarDetectionStop(
	IN PRTMP_ADAPTER pAd)
{
	RTMP_IO_WRITE8(pAd, 0x7020, 0x1d);
	RTMP_IO_WRITE8(pAd, 0x7021, 0x60);

	RadarDetectionStop(pAd);
	return;
}


VOID RadarDetectionStart(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN CTSProtect,
	IN UINT8 CTSPeriod)
{
	UINT8 DfsActiveTime = (pAd->CommonCfg.RadarDetect.DfsSessionTime & 0x1f);
	UINT8 CtsProtect = (CTSProtect == 1) ? 0x02 : 0x01; 

	if (CTSProtect != 0)
	{
		switch(pAd->CommonCfg.RadarDetect.RDDurRegion)
		{
		case FCC:
		case JAP_W56:
			CtsProtect = 0x03;
			break;

		case JAP:
			{
				UCHAR RDDurRegion;
				RDDurRegion = JapRadarType(pAd);
				if (RDDurRegion == JAP_W56)
					CtsProtect = 0x03;
				else
					CtsProtect = 0x02;
				break;
			}

		case CE:
		case JAP_W53:
		default:
			CtsProtect = 0x02;
			break;
		}
	}
	else
		CtsProtect = 0x01;


	
	
	
	
	

	
	AsicSendCommandToMcu(pAd, 0x60, 0xff, CTSPeriod, DfsActiveTime | (CtsProtect << 5));
	

	return;
}


VOID RadarDetectionStop(
	IN PRTMP_ADAPTER	pAd)
{
	DBGPRINT(RT_DEBUG_TRACE,("RadarDetectionStop.\n"));
	AsicSendCommandToMcu(pAd, 0x60, 0xff, 0x00, 0x00);	

	return;
}
#endif 



BOOLEAN RadarChannelCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	INT		i;
	BOOLEAN result = FALSE;

	for (i=0; i<pAd->ChannelListNum; i++)
	{
		if (Ch == pAd->ChannelList[i].Channel)
		{
			result = pAd->ChannelList[i].DfsReq;
			break;
		}
	}

	return result;
}

#ifdef DFS_SUPPORT

ULONG JapRadarType(
	IN PRTMP_ADAPTER pAd)
{
	ULONG		i;
	const UCHAR	Channel[15]={52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};

	if (pAd->CommonCfg.RadarDetect.RDDurRegion != JAP)
	{
		return pAd->CommonCfg.RadarDetect.RDDurRegion;
	}

	for (i=0; i<15; i++)
	{
		if (pAd->CommonCfg.Channel == Channel[i])
		{
			break;
		}
	}

	if (i < 4)
		return JAP_W53;
	else if (i < 15)
		return JAP_W56;
	else
		return JAP; 

}

ULONG RTMPBbpReadRadarDuration(
	IN PRTMP_ADAPTER	pAd)
{
	UINT8 byteValue = 0;
	ULONG result;

	BBP_IO_READ8_BY_REG_ID(pAd, BBP_R115, &byteValue);

	result = 0;
	switch (byteValue)
	{
	case 1: 
	case 2: 
		result = RTMPReadRadarDuration(pAd);
		break;

	case 0: 
	default:

		result = 0;
		break;
	}

	return result;
}

ULONG RTMPReadRadarDuration(
	IN PRTMP_ADAPTER	pAd)
{
	ULONG result = 0;

#ifdef DFS_SUPPORT
	UINT8 duration1 = 0, duration2 = 0, duration3 = 0;


	BBP_IO_READ8_BY_REG_ID(pAd, BBP_R116, &duration1);
	BBP_IO_READ8_BY_REG_ID(pAd, BBP_R117, &duration2);
	BBP_IO_READ8_BY_REG_ID(pAd, BBP_R118, &duration3);
	result = (duration1 << 16) + (duration2 << 8) + duration3;
#endif 

	return result;

}

VOID RTMPCleanRadarDuration(
	IN PRTMP_ADAPTER	pAd)
{
	return;
}


VOID ApRadarDetectPeriodic(
	IN PRTMP_ADAPTER pAd)
{
	INT	i;

	pAd->CommonCfg.RadarDetect.InServiceMonitorCount++;

	for (i=0; i<pAd->ChannelListNum; i++)
	{

		if (pAd->ChannelList[i].RemainingTimeForUse > 0)
		{
			pAd->ChannelList[i].RemainingTimeForUse --;
			if ((pAd->Mlme.PeriodicRound%5) == 0)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("RadarDetectPeriodic - ch=%d, RemainingTimeForUse=%d\n", pAd->ChannelList[i].Channel, pAd->ChannelList[i].RemainingTimeForUse));
			}
		}
	}

	
	if ((pAd->CommonCfg.Channel > 14)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& RadarChannelCheck(pAd, pAd->CommonCfg.Channel))
	{
		RadarDetectPeriodic(pAd);
	}

	return;
}



VOID RadarDetectPeriodic(
	IN PRTMP_ADAPTER	pAd)
{

	
	if (pAd->CommonCfg.RadarDetect.RDMode != RD_SILENCE_MODE)
			return;



	
	if (pAd->CommonCfg.RadarDetect.RDCount++ > pAd->CommonCfg.RadarDetect.ChMovingTime)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Not found radar signal, start send beacon and radar detection in service monitor\n\n"));
		BbpRadarDetectionStop(pAd);


		AsicEnableBssSync(pAd);
		pAd->CommonCfg.RadarDetect.RDMode = RD_NORMAL_MODE;



		return;
	}

	return;
}
#endif 

#ifdef DFS_SUPPORT

INT Set_ChMovingTime_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg)
{
	UINT8 Value;

	Value = (UINT8) simple_strtol(arg, 0, 10);

	pAd->CommonCfg.RadarDetect.ChMovingTime = Value;

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: %d\n", __FUNCTION__,
		pAd->CommonCfg.RadarDetect.ChMovingTime));

	return TRUE;
}

INT Set_LongPulseRadarTh_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING arg)
{
	UINT8 Value;

	Value = (UINT8) simple_strtol(arg, 0, 10) > 10 ? 10 : simple_strtol(arg, 0, 10);

	pAd->CommonCfg.RadarDetect.LongPulseRadarTh = Value;

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: %d\n", __FUNCTION__,
		pAd->CommonCfg.RadarDetect.LongPulseRadarTh));

	return TRUE;
}
#endif 
