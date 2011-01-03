

#ifndef __CHLIST_H__
#define __CHLIST_H__

#include "rtmp_type.h"
#include "rtmp_def.h"


#define ODOR			0
#define IDOR			1
#define BOTH			2

#define BAND_5G         0
#define BAND_24G        1
#define BAND_BOTH       2

typedef struct _CH_DESP {
	UCHAR FirstChannel;
	UCHAR NumOfCh;
	CHAR MaxTxPwr;			
	UCHAR Geography;			
	BOOLEAN DfsReq;			
} CH_DESP, *PCH_DESP;

typedef struct _CH_REGION {
	UCHAR CountReg[3];
	UCHAR DfsType;			
	CH_DESP ChDesp[10];
} CH_REGION, *PCH_REGION;

static CH_REGION ChRegion[] =
{
		{	
			"AG",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  23, BOTH, FALSE},	
				{ 52,  4,  23, BOTH, FALSE},	
				{ 100, 11, 30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"AR",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 52,  4,  24, BOTH, FALSE},	
				{ 149, 4,  30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"AW",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  23, BOTH, FALSE},	
				{ 52,  4,  23, BOTH, FALSE},	
				{ 100, 11, 30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"AU",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  23, BOTH, FALSE},	
				{ 52,  4,  24, BOTH, FALSE},	
				{ 149, 5,  30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"AT",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  23, IDOR, TRUE},		
				{ 52,  4,  23, IDOR, TRUE},		
				{ 100, 11, 30, BOTH, TRUE},		
				{ 0},							
			}
		},

		{	
			"BS",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  23, BOTH, FALSE},	
				{ 52,  4,  24, BOTH, FALSE},	
				{ 149, 5,  30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"BB",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  23, BOTH, FALSE},	
				{ 52,  4,  24, BOTH, FALSE},	
				{ 100, 11, 30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"BM",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  23, BOTH, FALSE},	
				{ 52,  4,  24, BOTH, FALSE},	
				{ 100, 11, 30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"BR",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  23, BOTH, FALSE},	
				{ 52,  4,  24, BOTH, FALSE},	
				{ 100, 11, 24, BOTH, FALSE},	
				{ 149, 5,  30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"BE",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  18, IDOR, FALSE},	
				{ 52,  4,  18, IDOR, FALSE},	
				{ 0},							
			}
		},

		{	
			"BG",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  23, IDOR, FALSE},	
				{ 52,  4,  23, IDOR, TRUE},	
				{ 100, 11, 30, ODOR, TRUE},	
				{ 0},							
			}
		},

		{	
			"CA",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  23, BOTH, FALSE},	
				{ 52,  4,  23, BOTH, FALSE},	
				{ 149, 5,  30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"KY",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  23, BOTH, FALSE},	
				{ 52,  4,  24, BOTH, FALSE},	
				{ 100, 11, 30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"CL",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  20, BOTH, FALSE},	
				{ 52,  4,  20, BOTH, FALSE},	
				{ 149, 5,  20, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"CN",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 149, 4,  27, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"CO",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  17, BOTH, FALSE},	
				{ 52,  4,  24, BOTH, FALSE},	
				{ 100, 11, 30, BOTH, FALSE},	
				{ 149, 5,  30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"CR",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  17, BOTH, FALSE},	
				{ 52,  4,  24, BOTH, FALSE},	
				{ 149, 4,  30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"CY",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  23, IDOR, FALSE},	
				{ 52,  4,  24, IDOR, TRUE},		
				{ 100, 11, 30, BOTH, TRUE},		
				{ 0},							
			}
		},

		{	
			"CZ",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  23, IDOR, FALSE},	
				{ 52,  4,  23, IDOR, TRUE},		
				{ 0},							
			}
		},

		{	
			"DK",
			CE,
			{
				{ 1,   13, 20, BOTH, FALSE},	
				{ 36,  4,  23, IDOR, FALSE},	
				{ 52,  4,  23, IDOR, TRUE},		
				{ 100, 11, 30, BOTH, TRUE},		
				{ 0},							
			}
		},

		{	
			"DO",
			CE,
			{
				{ 1,   0,  20, BOTH, FALSE},	
				{ 149, 4,  20, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"EC",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 100, 11,  27, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"SV",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,  4,   23, IDOR, FALSE},	
				{ 52,  4,   30, BOTH, TRUE},	
				{ 149, 4,   36, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"FI",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,  4,   23, IDOR, FALSE},	
				{ 52,  4,   23, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"FR",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,  4,   23, IDOR, FALSE},	
				{ 52,  4,   23, IDOR, TRUE},	
				{ 0},							
			}
		},

		{	
			"DE",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,  4,   23, IDOR, FALSE},	
				{ 52,  4,   23, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"GR",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,  4,   23, IDOR, FALSE},	
				{ 52,  4,   23, IDOR, TRUE},	
				{ 100, 11,  30, ODOR, TRUE},	
				{ 0},							
			}
		},

		{	
			"GU",
			CE,
			{
				{ 1,   11,  20, BOTH, FALSE},	
				{ 36,  4,   17, BOTH, FALSE},	
				{ 52,  4,   24, BOTH, FALSE},	
				{ 100, 11,  30, BOTH, FALSE},	
				{ 149,  5,  30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"GT",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,  4,   17, BOTH, FALSE},	
				{ 52,  4,   24, BOTH, FALSE},	
				{ 149,  4,  30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"HT",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,  4,   17, BOTH, FALSE},	
				{ 52,  4,   24, BOTH, FALSE},	
				{ 149,  4,  30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"HN",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 149,  4,  27, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"HK",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  23, IDOR, FALSE},	
				{ 52,   4,  23, IDOR, FALSE},	
				{ 149,  4,  30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"HU",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  23, IDOR, FALSE},	
				{ 52,   4,  23, IDOR, TRUE},	
				{ 0},							
			}
		},

		{	
			"IS",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  23, IDOR, FALSE},	
				{ 52,   4,  23, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"IN",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 149, 	4,  24, IDOR, FALSE},	
				{ 0},							
			}
		},

		{	
			"ID",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 149, 	4,  27, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"IE",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36, 	4,  23, IDOR, FALSE},	
				{ 52, 	4,  23, IDOR, TRUE},	
				{ 100, 11,  30, ODOR, TRUE},	
				{ 0},							
			}
		},

		{	
			"IL",
			CE,
			{
				{ 1,    3,  20, IDOR, FALSE},	
				{ 4, 	6,  20, BOTH, FALSE},	
				{ 10, 	4,  20, IDOR, FALSE},	
				{ 0},							
			}
		},

		{	
			"IT",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36, 	4,  23, IDOR, FALSE},	
				{ 52, 	4,  23, IDOR, TRUE},	
				{ 100, 11,  30, ODOR, TRUE},	
				{ 0},							
			}
		},

		{	
			"JP",
			JAP,
			{
				{ 1,   14,  20, BOTH, FALSE},	
				{ 36, 	4,  23, IDOR, FALSE},	
				{ 0},							
			}
		},

		{	
			"JO",
			CE,
			{
				{ 1,   13,  20, IDOR, FALSE},	
				{ 36, 	4,  23, IDOR, FALSE},	
				{ 149, 	4,  23, IDOR, FALSE},	
				{ 0},							
			}
		},

		{	
			"LV",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36, 	4,  23, IDOR, FALSE},	
				{ 52, 	4,  23, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"LI",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 52, 	4,  23, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"LT",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36, 	4,  23, IDOR, FALSE},	
				{ 52, 	4,  23, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"LU",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36, 	4,  23, IDOR, FALSE},	
				{ 52, 	4,  23, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"MY",
			CE,
			{
				{ 36, 	4,  23, BOTH, FALSE},	
				{ 52, 	4,  23, BOTH, FALSE},	
				{ 149,  5,  20, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"MT",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36, 	4,  23, IDOR, FALSE},	
				{ 52, 	4,  23, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"MA",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36, 	4,  24, IDOR, FALSE},	
				{ 0},							
			}
		},

		{	
			"MX",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36, 	4,  23, BOTH, FALSE},	
				{ 52, 	4,  24, BOTH, FALSE},	
				{ 149,  5,  30, IDOR, FALSE},	
				{ 0},							
			}
		},

		{	
			"NL",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36, 	4,  23, IDOR, FALSE},	
				{ 52, 	4,  24, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"NZ",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36, 	4,  24, BOTH, FALSE},	
				{ 52, 	4,  24, BOTH, FALSE},	
				{ 149,  4,  30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"NO",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36, 	4,  24, IDOR, FALSE},	
				{ 52, 	4,  24, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"PE",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 149,  4,  27, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"PT",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  23, IDOR, FALSE},	
				{ 52,   4,  23, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"PL",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  23, IDOR, FALSE},	
				{ 52,   4,  23, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"RO",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  23, IDOR, FALSE},	
				{ 52,   4,  23, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"RU",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 149,  4,  20, IDOR, FALSE},	
				{ 0},							
			}
		},

		{	
			"SA",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  23, BOTH, FALSE},	
				{ 52,   4,  23, BOTH, FALSE},	
				{ 149,  4,  23, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"CS",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"SG",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  23, BOTH, FALSE},	
				{ 52,   4,  23, BOTH, FALSE},	
				{ 149,  4,  20, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"SK",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  23, IDOR, FALSE},	
				{ 52,   4,  23, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"SI",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  23, IDOR, FALSE},	
				{ 52,   4,  23, IDOR, TRUE},	
				{ 0},							
			}
		},

		{	
			"ZA",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  23, BOTH, FALSE},	
				{ 52,   4,  23, IDOR, FALSE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 149,  4,  30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"KR",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  20, BOTH, FALSE},	
				{ 52,   4,  20, BOTH, FALSE},	
				{ 100,  8,  20, BOTH, FALSE},	
				{ 149,  4,  20, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"ES",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  17, IDOR, FALSE},	
				{ 52,   4,  23, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"SE",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  23, IDOR, FALSE},	
				{ 52,   4,  23, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"CH",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  23, IDOR, TRUE},	
				{ 52,   4,  23, IDOR, TRUE},	
				{ 0},							
			}
		},

		{	
			"TW",
			CE,
			{
				{ 1,   11,  30, BOTH, FALSE},	
				{ 52,   4,  23, IDOR, FALSE},	
				{ 0},							
			}
		},

		{	
			"TR",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  23, BOTH, FALSE},	
				{ 52,   4,  23, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"GB",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 36,   4,  23, IDOR, FALSE},	
				{ 52,   4,  23, IDOR, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 0},							
			}
		},

		{	
			"UA",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"AE",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"US",
			CE,
			{
				{ 1,   11,  30, BOTH, FALSE},	
				{ 36,   4,  17, IDOR, FALSE},	
				{ 52,   4,  24, BOTH, TRUE},	
				{ 100, 11,  30, BOTH, TRUE},	
				{ 149,  5,  30, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"VE",
			CE,
			{
				{ 1,   13,  20, BOTH, FALSE},	
				{ 149,  4,  27, BOTH, FALSE},	
				{ 0},							
			}
		},

		{	
			"",
			CE,
			{
				{ 1,   11,  20, BOTH, FALSE},	
				{ 36,   4,  20, BOTH, FALSE},	
				{ 52,   4,  20, BOTH, FALSE},	
				{ 100, 11,  20, BOTH, FALSE},	
				{ 149,  5,  20, BOTH, FALSE},	
				{ 0},							
			}
		},
};

static inline PCH_REGION GetChRegion(
	IN PUCHAR CntryCode)
{
	INT loop = 0;
	PCH_REGION pChRegion = NULL;

	while (strcmp(ChRegion[loop].CountReg, "") != 0)
	{
		if (strncmp(ChRegion[loop].CountReg, CntryCode, 2) == 0)
		{
			pChRegion = &ChRegion[loop];
			break;
		}
		loop++;
	}

	if (pChRegion == NULL)
		pChRegion = &ChRegion[loop];
	return pChRegion;
}

static inline VOID ChBandCheck(
	IN UCHAR PhyMode,
	OUT PUCHAR pChType)
{
	switch(PhyMode)
	{
		case PHY_11A:
		case PHY_11AN_MIXED:
			*pChType = BAND_5G;
			break;
		case PHY_11ABG_MIXED:
		case PHY_11AGN_MIXED:
		case PHY_11ABGN_MIXED:
			*pChType = BAND_BOTH;
			break;

		default:
			*pChType = BAND_24G;
			break;
	}
}

static inline UCHAR FillChList(
	IN PRTMP_ADAPTER pAd,
	IN PCH_DESP pChDesp,
	IN UCHAR Offset,
	IN UCHAR increment)
{
	INT i, j, l;
	UCHAR channel;

	j = Offset;
	for (i = 0; i < pChDesp->NumOfCh; i++)
	{
		channel = pChDesp->FirstChannel + i * increment;
		for (l=0; l<MAX_NUM_OF_CHANNELS; l++)
		{
			if (channel == pAd->TxPower[l].Channel)
			{
				pAd->ChannelList[j].Power = pAd->TxPower[l].Power;
				pAd->ChannelList[j].Power2 = pAd->TxPower[l].Power2;
				break;
			}
		}
		if (l == MAX_NUM_OF_CHANNELS)
			continue;

		pAd->ChannelList[j].Channel = pChDesp->FirstChannel + i * increment;
		pAd->ChannelList[j].MaxTxPwr = pChDesp->MaxTxPwr;
		pAd->ChannelList[j].DfsReq = pChDesp->DfsReq;
		j++;
	}
	pAd->ChannelListNum = j;

	return j;
}

static inline VOID CreateChList(
	IN PRTMP_ADAPTER pAd,
	IN PCH_REGION pChRegion,
	IN UCHAR Geography)
{
	INT i;
	UCHAR offset = 0;
	PCH_DESP pChDesp;
	UCHAR ChType;
	UCHAR increment;

	if (pChRegion == NULL)
		return;

	ChBandCheck(pAd->CommonCfg.PhyMode, &ChType);

	for (i=0; i<10; i++)
	{
		pChDesp = &pChRegion->ChDesp[i];
		if (pChDesp->FirstChannel == 0)
			break;

		if (ChType == BAND_5G)
		{
			if (pChDesp->FirstChannel <= 14)
				continue;
		}
		else if (ChType == BAND_24G)
		{
			if (pChDesp->FirstChannel > 14)
				continue;
		}

		if ((pChDesp->Geography == BOTH)
			|| (pChDesp->Geography == Geography))
        {
			if (pChDesp->FirstChannel > 14)
                increment = 4;
            else
                increment = 1;
			offset = FillChList(pAd, pChDesp, offset, increment);
        }
	}
}

static inline VOID BuildChannelListEx(
	IN PRTMP_ADAPTER pAd)
{
	PCH_REGION pChReg;

	pChReg = GetChRegion(pAd->CommonCfg.CountryCode);
	CreateChList(pAd, pChReg, pAd->CommonCfg.Geography);
}

static inline VOID BuildBeaconChList(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pBuf,
	OUT	PULONG pBufLen)
{
	INT i;
	ULONG TmpLen;
	PCH_REGION pChRegion;
	PCH_DESP pChDesp;
	UCHAR ChType;

	pChRegion = GetChRegion(pAd->CommonCfg.CountryCode);

	if (pChRegion == NULL)
		return;

	ChBandCheck(pAd->CommonCfg.PhyMode, &ChType);
	*pBufLen = 0;

	for (i=0; i<10; i++)
	{
		pChDesp = &pChRegion->ChDesp[i];
		if (pChDesp->FirstChannel == 0)
			break;

		if (ChType == BAND_5G)
		{
			if (pChDesp->FirstChannel <= 14)
				continue;
		}
		else if (ChType == BAND_24G)
		{
			if (pChDesp->FirstChannel > 14)
				continue;
		}

		if ((pChDesp->Geography == BOTH)
			|| (pChDesp->Geography == pAd->CommonCfg.Geography))
		{
			MakeOutgoingFrame(pBuf + *pBufLen,		&TmpLen,
								1,                 	&pChDesp->FirstChannel,
								1,                 	&pChDesp->NumOfCh,
								1,                 	&pChDesp->MaxTxPwr,
								END_OF_ARGS);
			*pBufLen += TmpLen;
		}
	}
}

static inline BOOLEAN IsValidChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel)

{
	INT i;

	for (i = 0; i < pAd->ChannelListNum; i++)
	{
		if (pAd->ChannelList[i].Channel == channel)
			break;
	}

	if (i == pAd->ChannelListNum)
		return FALSE;
	else
		return TRUE;
}


static inline UCHAR GetExtCh(
	IN UCHAR Channel,
	IN UCHAR Direction)
{
	CHAR ExtCh;

	if (Direction == EXTCHA_ABOVE)
		ExtCh = Channel + 4;
	else
		ExtCh = (Channel - 4) > 0 ? (Channel - 4) : 0;

	return ExtCh;
}


static inline VOID N_ChannelCheck(
	IN PRTMP_ADAPTER pAd)
{
	
	UCHAR Channel = pAd->CommonCfg.Channel;

	if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (pAd->CommonCfg.RegTransmitSetting.field.BW  == BW_40))
	{
		if (Channel > 14)
		{
			if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
			    (Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157))
			{
				pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_ABOVE;
			}
			else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) || (Channel == 112) ||
					(Channel == 120) || (Channel == 128) || (Channel == 136) || (Channel == 153) || (Channel == 161))
			{
				pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_BELOW;
			}
			else
			{
				pAd->CommonCfg.RegTransmitSetting.field.BW  = BW_20;
			}
		}
		else
		{
			do
			{
				UCHAR ExtCh;
				UCHAR Dir = pAd->CommonCfg.RegTransmitSetting.field.EXTCHA;
				ExtCh = GetExtCh(Channel, Dir);
				if (IsValidChannel(pAd, ExtCh))
					break;

				Dir = (Dir == EXTCHA_ABOVE) ? EXTCHA_BELOW : EXTCHA_ABOVE;
				ExtCh = GetExtCh(Channel, Dir);
				if (IsValidChannel(pAd, ExtCh))
				{
					pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = Dir;
					break;
				}
				pAd->CommonCfg.RegTransmitSetting.field.BW  = BW_20;
			} while(FALSE);

			if (Channel == 14)
			{
				pAd->CommonCfg.RegTransmitSetting.field.BW  = BW_20;
				
			}
		}
	}


}


static inline VOID N_SetCenCh(
	IN PRTMP_ADAPTER pAd)
{
	if (pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)
	{
		if (pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
		{
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel + 2;
		}
		else
		{
			if (pAd->CommonCfg.Channel == 14)
				pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 1;
			else
				pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 2;
		}
	}
	else
	{
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
	}
}

static inline UINT8 GetCuntryMaxTxPwr(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 channel)
{
	int i;
	for (i = 0; i < pAd->ChannelListNum; i++)
	{
		if (pAd->ChannelList[i].Channel == channel)
			break;
	}

	if (i == pAd->ChannelListNum)
		return 0xff;
	else
		return pAd->ChannelList[i].MaxTxPwr;
}
#endif 

