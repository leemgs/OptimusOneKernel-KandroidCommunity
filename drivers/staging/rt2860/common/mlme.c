

#include "../rt_config.h"
#include <stdarg.h>

UCHAR	CISCO_OUI[] = {0x00, 0x40, 0x96};

UCHAR	WPA_OUI[] = {0x00, 0x50, 0xf2, 0x01};
UCHAR	RSN_OUI[] = {0x00, 0x0f, 0xac};
UCHAR   WME_INFO_ELEM[]  = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};
UCHAR   WME_PARM_ELEM[] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};
UCHAR	Ccx2QosInfo[] = {0x00, 0x40, 0x96, 0x04};
UCHAR   RALINK_OUI[]  = {0x00, 0x0c, 0x43};
UCHAR   BROADCOM_OUI[]  = {0x00, 0x90, 0x4c};
UCHAR   WPS_OUI[] = {0x00, 0x50, 0xf2, 0x04};
UCHAR	PRE_N_HT_OUI[]	= {0x00, 0x90, 0x4c};

UCHAR RateSwitchTable[] = {

    0x11, 0x00,  0,  0,  0,						
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 35, 45,
    0x03, 0x00,  3, 20, 45,
    0x04, 0x21,  0, 30, 50,
    0x05, 0x21,  1, 20, 50,
    0x06, 0x21,  2, 20, 50,
    0x07, 0x21,  3, 15, 50,
    0x08, 0x21,  4, 15, 30,
    0x09, 0x21,  5, 10, 25,
    0x0a, 0x21,  6,  8, 25,
    0x0b, 0x21,  7,  8, 25,
    0x0c, 0x20, 12,  15, 30,
    0x0d, 0x20, 13,  8, 20,
    0x0e, 0x20, 14,  8, 20,
    0x0f, 0x20, 15,  8, 25,
    0x10, 0x22, 15,  8, 25,
    0x11, 0x00,  0,  0,  0,
    0x12, 0x00,  0,  0,  0,
    0x13, 0x00,  0,  0,  0,
    0x14, 0x00,  0,  0,  0,
    0x15, 0x00,  0,  0,  0,
    0x16, 0x00,  0,  0,  0,
    0x17, 0x00,  0,  0,  0,
    0x18, 0x00,  0,  0,  0,
    0x19, 0x00,  0,  0,  0,
    0x1a, 0x00,  0,  0,  0,
    0x1b, 0x00,  0,  0,  0,
    0x1c, 0x00,  0,  0,  0,
    0x1d, 0x00,  0,  0,  0,
    0x1e, 0x00,  0,  0,  0,
    0x1f, 0x00,  0,  0,  0,
};

UCHAR RateSwitchTable11B[] = {

    0x04, 0x03,  0,  0,  0,						
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 35, 45,
    0x03, 0x00,  3, 20, 45,
};

UCHAR RateSwitchTable11BG[] = {

    0x0a, 0x00,  0,  0,  0,						
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 35, 45,
    0x03, 0x00,  3, 20, 45,
    0x04, 0x10,  2, 20, 35,
    0x05, 0x10,  3, 16, 35,
    0x06, 0x10,  4, 10, 25,
    0x07, 0x10,  5, 16, 25,
    0x08, 0x10,  6, 10, 25,
    0x09, 0x10,  7, 10, 13,
};

UCHAR RateSwitchTable11G[] = {

    0x08, 0x00,  0,  0,  0,						
    0x00, 0x10,  0, 20, 101,
    0x01, 0x10,  1, 20, 35,
    0x02, 0x10,  2, 20, 35,
    0x03, 0x10,  3, 16, 35,
    0x04, 0x10,  4, 10, 25,
    0x05, 0x10,  5, 16, 25,
    0x06, 0x10,  6, 10, 25,
    0x07, 0x10,  7, 10, 13,
};

UCHAR RateSwitchTable11N1S[] = {

    0x09, 0x00,  0,  0,  0,						
    0x00, 0x21,  0, 30, 101,
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x21,  5, 10, 25,
    0x06, 0x21,  6,  8, 14,
    0x07, 0x21,  7,  8, 14,
    0x08, 0x23,  7,  8, 14,
};

UCHAR RateSwitchTable11N2S[] = {

    0x0a, 0x00,  0,  0,  0,      
    0x00, 0x21,  0, 30, 101,
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x20, 12,  15, 30,
    0x06, 0x20, 13,  8, 20,
    0x07, 0x20, 14,  8, 20,
    0x08, 0x20, 15,  8, 25,
    0x09, 0x22, 15,  8, 25,
};

UCHAR RateSwitchTable11N3S[] = {

    0x0a, 0x00,  0,  0,  0,      
    0x00, 0x21,  0, 30, 101,
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x20, 12,  15, 30,
    0x06, 0x20, 13,  8, 20,
    0x07, 0x20, 14,  8, 20,
    0x08, 0x20, 15,  8, 25,
    0x09, 0x22, 15,  8, 25,
};

UCHAR RateSwitchTable11N2SForABand[] = {

    0x0b, 0x09,  0,  0,  0,						
    0x00, 0x21,  0, 30, 101,
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x21,  5, 15, 30,
    0x06, 0x20, 12,  15, 30,
    0x07, 0x20, 13,  8, 20,
    0x08, 0x20, 14,  8, 20,
    0x09, 0x20, 15,  8, 25,
    0x0a, 0x22, 15,  8, 25,
};

UCHAR RateSwitchTable11N3SForABand[] = { 

    0x0b, 0x09,  0,  0,  0,						
    0x00, 0x21,  0, 30, 101,
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x21,  5, 15, 30,
    0x06, 0x20, 12,  15, 30,
    0x07, 0x20, 13,  8, 20,
    0x08, 0x20, 14,  8, 20,
    0x09, 0x20, 15,  8, 25,
    0x0a, 0x22, 15,  8, 25,
};

UCHAR RateSwitchTable11BGN1S[] = {

    0x0d, 0x00,  0,  0,  0,						
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 35, 45,
    0x03, 0x00,  3, 20, 45,
    0x04, 0x21,  0, 30,101,	
    0x05, 0x21,  1, 20, 50,
    0x06, 0x21,  2, 20, 50,
    0x07, 0x21,  3, 15, 50,
    0x08, 0x21,  4, 15, 30,
    0x09, 0x21,  5, 10, 25,
    0x0a, 0x21,  6,  8, 14,
    0x0b, 0x21,  7,  8, 14,
	0x0c, 0x23,  7,  8, 14,
};

UCHAR RateSwitchTable11BGN2S[] = {

    0x0a, 0x00,  0,  0,  0,						
    0x00, 0x21,  0, 30,101,	
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x20, 12, 15, 30,
    0x06, 0x20, 13,  8, 20,
    0x07, 0x20, 14,  8, 20,
    0x08, 0x20, 15,  8, 25,
    0x09, 0x22, 15,  8, 25,
};

UCHAR RateSwitchTable11BGN3S[] = { 

    0x0a, 0x00,  0,  0,  0,						
    0x00, 0x21,  0, 30,101,	
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 20, 50,
    0x04, 0x21,  4, 15, 50,
    0x05, 0x20, 20, 15, 30,
    0x06, 0x20, 21,  8, 20,
    0x07, 0x20, 22,  8, 20,
    0x08, 0x20, 23,  8, 25,
    0x09, 0x22, 23,  8, 25,
};

UCHAR RateSwitchTable11BGN2SForABand[] = {

    0x0b, 0x09,  0,  0,  0,						
    0x00, 0x21,  0, 30,101,	
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x21,  5, 15, 30,
    0x06, 0x20, 12, 15, 30,
    0x07, 0x20, 13,  8, 20,
    0x08, 0x20, 14,  8, 20,
    0x09, 0x20, 15,  8, 25,
    0x0a, 0x22, 15,  8, 25,
};

UCHAR RateSwitchTable11BGN3SForABand[] = { 

    0x0c, 0x09,  0,  0,  0,						
    0x00, 0x21,  0, 30,101,	
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x21,  5, 15, 30,
    0x06, 0x21, 12, 15, 30,
    0x07, 0x20, 20, 15, 30,
    0x08, 0x20, 21,  8, 20,
    0x09, 0x20, 22,  8, 20,
    0x0a, 0x20, 23,  8, 25,
    0x0b, 0x22, 23,  8, 25,
};

PUCHAR ReasonString[] = {
		 "Reserved",
		 "Unspecified Reason",
		 "Previous Auth no longer valid",
		 "STA is leaving / has left",
		 "DIS-ASSOC due to inactivity",
		 "AP unable to hanle all associations",
		 "class 2 error",
		 "class 3 error",
		 "STA is leaving / has left",
		 "require auth before assoc/re-assoc",
		 "Reserved",
		 "Reserved",
		 "Reserved",
		 "invalid IE",
		 "MIC error",
		 "4-way handshake timeout",
		 "2-way (group key) handshake timeout",
		 "4-way handshake IE diff among AssosReq/Rsp/Beacon",
	
};

extern UCHAR	 OfdmRateToRxwiMCS[];


ULONG BasicRateMask[12]				= {0xfffff001 , 0xfffff003 , 0xfffff007 , 0xfffff00f ,
									  0xfffff01f 	 , 0xfffff03f 	  , 0xfffff07f  , 0xfffff0ff ,
									  0xfffff1ff 	 , 0xfffff3ff 	  , 0xfffff7ff  , 0xffffffff };

UCHAR MULTICAST_ADDR[MAC_ADDR_LEN] = {0x1,  0x00, 0x00, 0x00, 0x00, 0x00};
UCHAR BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};





CHAR RssiSafeLevelForTxRate[] ={  -92, -91, -90, -87, -88, -86, -85, -83, -81, -78, -72, -71, -40, -40 };

UCHAR  RateIdToMbps[]	 = { 1, 2, 5, 11, 6, 9, 12, 18, 24, 36, 48, 54, 72, 100};
USHORT RateIdTo500Kbps[] = { 2, 4, 11, 22, 12, 18, 24, 36, 48, 72, 96, 108, 144, 200};

UCHAR  SsidIe	 = IE_SSID;
UCHAR  SupRateIe = IE_SUPP_RATES;
UCHAR  ExtRateIe = IE_EXT_SUPP_RATES;
UCHAR  HtCapIe = IE_HT_CAP;
UCHAR  AddHtInfoIe = IE_ADD_HT;
UCHAR  NewExtChanIe = IE_SECONDARY_CH_OFFSET;
UCHAR  ErpIe	 = IE_ERP;
UCHAR  DsIe 	 = IE_DS_PARM;
UCHAR  TimIe	 = IE_TIM;
UCHAR  WpaIe	 = IE_WPA;
UCHAR  Wpa2Ie	 = IE_WPA2;
UCHAR  IbssIe	 = IE_IBSS_PARM;
UCHAR  Ccx2Ie	 = IE_CCX_V2;

extern UCHAR	WPA_OUI[];

UCHAR	SES_OUI[] = {0x00, 0x90, 0x4c};

UCHAR	ZeroSsid[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};


RTMP_RF_REGS RF2850RegTable[] = {

		{1,  0x98402ecc, 0x984c0786, 0x9816b455, 0x9800510b},
		{2,  0x98402ecc, 0x984c0786, 0x98168a55, 0x9800519f},
		{3,  0x98402ecc, 0x984c078a, 0x98168a55, 0x9800518b},
		{4,  0x98402ecc, 0x984c078a, 0x98168a55, 0x9800519f},
		{5,  0x98402ecc, 0x984c078e, 0x98168a55, 0x9800518b},
		{6,  0x98402ecc, 0x984c078e, 0x98168a55, 0x9800519f},
		{7,  0x98402ecc, 0x984c0792, 0x98168a55, 0x9800518b},
		{8,  0x98402ecc, 0x984c0792, 0x98168a55, 0x9800519f},
		{9,  0x98402ecc, 0x984c0796, 0x98168a55, 0x9800518b},
		{10, 0x98402ecc, 0x984c0796, 0x98168a55, 0x9800519f},
		{11, 0x98402ecc, 0x984c079a, 0x98168a55, 0x9800518b},
		{12, 0x98402ecc, 0x984c079a, 0x98168a55, 0x9800519f},
		{13, 0x98402ecc, 0x984c079e, 0x98168a55, 0x9800518b},
		{14, 0x98402ecc, 0x984c07a2, 0x98168a55, 0x98005193},

		
		{36, 0x98402ecc, 0x984c099a, 0x98158a55, 0x980ed1a3},
		{38, 0x98402ecc, 0x984c099e, 0x98158a55, 0x980ed193},
		{40, 0x98402ec8, 0x984c0682, 0x98158a55, 0x980ed183},
		{44, 0x98402ec8, 0x984c0682, 0x98158a55, 0x980ed1a3},
		{46, 0x98402ec8, 0x984c0686, 0x98158a55, 0x980ed18b},
		{48, 0x98402ec8, 0x984c0686, 0x98158a55, 0x980ed19b},
		{52, 0x98402ec8, 0x984c068a, 0x98158a55, 0x980ed193},
		{54, 0x98402ec8, 0x984c068a, 0x98158a55, 0x980ed1a3},
		{56, 0x98402ec8, 0x984c068e, 0x98158a55, 0x980ed18b},
		{60, 0x98402ec8, 0x984c0692, 0x98158a55, 0x980ed183},
		{62, 0x98402ec8, 0x984c0692, 0x98158a55, 0x980ed193},
		{64, 0x98402ec8, 0x984c0692, 0x98158a55, 0x980ed1a3}, 

		
		{100, 0x98402ec8, 0x984c06b2, 0x98178a55, 0x980ed783},

		
		
		
		{102, 0x98402ec8, 0x985c06b2, 0x98578a55, 0x980ed793},
		{104, 0x98402ec8, 0x985c06b2, 0x98578a55, 0x980ed1a3},
		{108, 0x98402ecc, 0x985c0a32, 0x98578a55, 0x980ed193},

		{110, 0x98402ecc, 0x984c0a36, 0x98178a55, 0x980ed183},
		{112, 0x98402ecc, 0x984c0a36, 0x98178a55, 0x980ed19b},
		{116, 0x98402ecc, 0x984c0a3a, 0x98178a55, 0x980ed1a3},
		{118, 0x98402ecc, 0x984c0a3e, 0x98178a55, 0x980ed193},
		{120, 0x98402ec4, 0x984c0382, 0x98178a55, 0x980ed183},
		{124, 0x98402ec4, 0x984c0382, 0x98178a55, 0x980ed193},
		{126, 0x98402ec4, 0x984c0382, 0x98178a55, 0x980ed15b}, 
		{128, 0x98402ec4, 0x984c0382, 0x98178a55, 0x980ed1a3},
		{132, 0x98402ec4, 0x984c0386, 0x98178a55, 0x980ed18b},
		{134, 0x98402ec4, 0x984c0386, 0x98178a55, 0x980ed193},
		{136, 0x98402ec4, 0x984c0386, 0x98178a55, 0x980ed19b},
		{140, 0x98402ec4, 0x984c038a, 0x98178a55, 0x980ed183},

		
		{149, 0x98402ec4, 0x984c038a, 0x98178a55, 0x980ed1a7},
		{151, 0x98402ec4, 0x984c038e, 0x98178a55, 0x980ed187},
		{153, 0x98402ec4, 0x984c038e, 0x98178a55, 0x980ed18f},
		{157, 0x98402ec4, 0x984c038e, 0x98178a55, 0x980ed19f},
		{159, 0x98402ec4, 0x984c038e, 0x98178a55, 0x980ed1a7},
		{161, 0x98402ec4, 0x984c0392, 0x98178a55, 0x980ed187},
		{165, 0x98402ec4, 0x984c0392, 0x98178a55, 0x980ed197},

		
		{184, 0x95002ccc, 0x9500491e, 0x9509be55, 0x950c0a0b},
		{188, 0x95002ccc, 0x95004922, 0x9509be55, 0x950c0a13},
		{192, 0x95002ccc, 0x95004926, 0x9509be55, 0x950c0a1b},
		{196, 0x95002ccc, 0x9500492a, 0x9509be55, 0x950c0a23},
		{208, 0x95002ccc, 0x9500493a, 0x9509be55, 0x950c0a13},
		{212, 0x95002ccc, 0x9500493e, 0x9509be55, 0x950c0a1b},
		{216, 0x95002ccc, 0x95004982, 0x9509be55, 0x950c0a23},

		
};
UCHAR	NUM_OF_2850_CHNL = (sizeof(RF2850RegTable) / sizeof(RTMP_RF_REGS));

FREQUENCY_ITEM FreqItems3020[] =
{
	
	
	
	
	
	
	{1,    241,  2,  2},
	{2,    241,	 2,  7},
	{3,    242,	 2,  2},
	{4,    242,	 2,  7},
	{5,    243,	 2,  2},
	{6,    243,	 2,  7},
	{7,    244,	 2,  2},
	{8,    244,	 2,  7},
	{9,    245,	 2,  2},
	{10,   245,	 2,  7},
	{11,   246,	 2,  2},
	{12,   246,	 2,  7},
	{13,   247,	 2,  2},
	{14,   248,	 2,  4},
};
UCHAR	NUM_OF_3020_CHNL=(sizeof(FreqItems3020) / sizeof(FREQUENCY_ITEM));


NDIS_STATUS MlmeInit(
	IN PRTMP_ADAPTER pAd)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("--> MLME Initialize\n"));

	do
	{
		Status = MlmeQueueInit(&pAd->Mlme.Queue);
		if(Status != NDIS_STATUS_SUCCESS)
			break;

		pAd->Mlme.bRunning = FALSE;
		NdisAllocateSpinLock(&pAd->Mlme.TaskLock);

		{
			BssTableInit(&pAd->ScanTab);

			
			AssocStateMachineInit(pAd, &pAd->Mlme.AssocMachine, pAd->Mlme.AssocFunc);
			AuthStateMachineInit(pAd, &pAd->Mlme.AuthMachine, pAd->Mlme.AuthFunc);
			AuthRspStateMachineInit(pAd, &pAd->Mlme.AuthRspMachine, pAd->Mlme.AuthRspFunc);
			SyncStateMachineInit(pAd, &pAd->Mlme.SyncMachine, pAd->Mlme.SyncFunc);
			WpaPskStateMachineInit(pAd, &pAd->Mlme.WpaPskMachine, pAd->Mlme.WpaPskFunc);
			AironetStateMachineInit(pAd, &pAd->Mlme.AironetMachine, pAd->Mlme.AironetFunc);

			
			
			MlmeCntlInit(pAd, &pAd->Mlme.CntlMachine, NULL);
		}

		ActionStateMachineInit(pAd, &pAd->Mlme.ActMachine, pAd->Mlme.ActFunc);

		
		RTMPInitTimer(pAd, &pAd->Mlme.PeriodicTimer, GET_TIMER_FUNCTION(MlmePeriodicExec), pAd, TRUE);

		
		RTMPSetTimer(&pAd->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);

		
		RTMPInitTimer(pAd, &pAd->Mlme.RxAntEvalTimer, GET_TIMER_FUNCTION(AsicRxAntEvalTimeout), pAd, FALSE);

#ifdef RT2860
		{
	        if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
	        {
	            
	    		RTMPInitTimer(pAd, &pAd->Mlme.PsPollTimer, GET_TIMER_FUNCTION(PsPollWakeExec), pAd, FALSE);
	    		RTMPInitTimer(pAd, &pAd->Mlme.RadioOnOffTimer, GET_TIMER_FUNCTION(RadioOnExec), pAd, FALSE);
	        }
		}
#endif
	} while (FALSE);

	DBGPRINT(RT_DEBUG_TRACE, ("<-- MLME Initialize\n"));

	return Status;
}


VOID MlmeHandler(
	IN PRTMP_ADAPTER pAd)
{
	MLME_QUEUE_ELEM 	   *Elem = NULL;

	
	

	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	if(pAd->Mlme.bRunning)
	{
		NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
		return;
	}
	else
	{
		pAd->Mlme.bRunning = TRUE;
	}
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);

	while (!MlmeQueueEmpty(&pAd->Mlme.Queue))
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_MLME_RESET_IN_PROGRESS) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Device Halted or Removed or MlmeRest, exit MlmeHandler! (queue num = %ld)\n", pAd->Mlme.Queue.Num));
			break;
		}

		
		if (MlmeDequeue(&pAd->Mlme.Queue, &Elem))
		{
#ifdef RT2870
			if (Elem->MsgType == MT2_RESET_CONF)
			{
				DBGPRINT_RAW(RT_DEBUG_TRACE, ("!!! reset MLME state machine !!!\n"));
				MlmeRestartStateMachine(pAd);
				Elem->Occupied = FALSE;
				Elem->MsgLen = 0;
				continue;
			}
#endif 

			
			switch (Elem->Machine)
			{
				
				case ASSOC_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.AssocMachine, Elem);
					break;
				case AUTH_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.AuthMachine, Elem);
					break;
				case AUTH_RSP_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.AuthRspMachine, Elem);
					break;
				case SYNC_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.SyncMachine, Elem);
					break;
				case MLME_CNTL_STATE_MACHINE:
					MlmeCntlMachinePerformAction(pAd, &pAd->Mlme.CntlMachine, Elem);
					break;
				case WPA_PSK_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.WpaPskMachine, Elem);
					break;
				case AIRONET_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.AironetMachine, Elem);
					break;
				case ACTION_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.ActMachine, Elem);
					break;




				default:
					DBGPRINT(RT_DEBUG_TRACE, ("ERROR: Illegal machine %ld in MlmeHandler()\n", Elem->Machine));
					break;
			} 

			
			Elem->Occupied = FALSE;
			Elem->MsgLen = 0;

		}
		else {
			DBGPRINT_ERR(("MlmeHandler: MlmeQueue empty\n"));
		}
	}

	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	pAd->Mlme.bRunning = FALSE;
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
}


VOID MlmeHalt(
	IN PRTMP_ADAPTER pAd)
{
	BOOLEAN 	  Cancelled;
#ifdef RT3070
	UINT32		TxPinCfg = 0x00050F0F;
#endif 

	DBGPRINT(RT_DEBUG_TRACE, ("==> MlmeHalt\n"));

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		
		AsicDisableSync(pAd);
	}

	{
		
		RTMPCancelTimer(&pAd->MlmeAux.AssocTimer,		&Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.ReassocTimer,		&Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.DisassocTimer,	&Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.AuthTimer,		&Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer,		&Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer,		&Cancelled);
#ifdef RT2860
	    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
	    {
	   	    RTMPCancelTimer(&pAd->Mlme.PsPollTimer,		&Cancelled);
		    RTMPCancelTimer(&pAd->Mlme.RadioOnOffTimer,		&Cancelled);
		}
#endif
	}

	RTMPCancelTimer(&pAd->Mlme.PeriodicTimer,		&Cancelled);
	RTMPCancelTimer(&pAd->Mlme.RxAntEvalTimer,		&Cancelled);



	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		
		RTMPSetLED(pAd, LED_HALT);
        RTMPSetSignalLED(pAd, -100);	
#ifdef RT2870
        {
            LED_CFG_STRUC LedCfg;
            RTMP_IO_READ32(pAd, LED_CFG, &LedCfg.word);
            LedCfg.field.LedPolar = 0;
            LedCfg.field.RLedMode = 0;
            LedCfg.field.GLedMode = 0;
            LedCfg.field.YLedMode = 0;
            RTMP_IO_WRITE32(pAd, LED_CFG, LedCfg.word);
        }
#endif 
#ifdef RT3070
		
		
		
		if (IS_RT3070(pAd) || IS_RT3071(pAd))
		{
			TxPinCfg &= 0xFFFFF0F0;
			RTUSBWriteMACRegister(pAd, TX_PIN_CFG, TxPinCfg);
		}
#endif 
	}

	RTMPusecDelay(5000);    

	MlmeQueueDestroy(&pAd->Mlme.Queue);
	NdisFreeSpinLock(&pAd->Mlme.TaskLock);

	DBGPRINT(RT_DEBUG_TRACE, ("<== MlmeHalt\n"));
}

VOID MlmeResetRalinkCounters(
	IN  PRTMP_ADAPTER   pAd)
{
	pAd->RalinkCounters.LastOneSecRxOkDataCnt = pAd->RalinkCounters.OneSecRxOkDataCnt;
	
	pAd->RalinkCounters.OneSecBeaconSentCnt = 0;
	pAd->RalinkCounters.OneSecFalseCCACnt = 0;
	pAd->RalinkCounters.OneSecRxFcsErrCnt = 0;
	pAd->RalinkCounters.OneSecRxOkCnt = 0;
	pAd->RalinkCounters.OneSecTxFailCount = 0;
	pAd->RalinkCounters.OneSecTxNoRetryOkCount = 0;
	pAd->RalinkCounters.OneSecTxRetryOkCount = 0;
	pAd->RalinkCounters.OneSecRxOkDataCnt = 0;

	
	pAd->RalinkCounters.OneSecOsTxCount[QID_AC_BE] = 0;
	pAd->RalinkCounters.OneSecOsTxCount[QID_AC_BK] = 0;
	pAd->RalinkCounters.OneSecOsTxCount[QID_AC_VI] = 0;
	pAd->RalinkCounters.OneSecOsTxCount[QID_AC_VO] = 0;
	pAd->RalinkCounters.OneSecDmaDoneCount[QID_AC_BE] = 0;
	pAd->RalinkCounters.OneSecDmaDoneCount[QID_AC_BK] = 0;
	pAd->RalinkCounters.OneSecDmaDoneCount[QID_AC_VI] = 0;
	pAd->RalinkCounters.OneSecDmaDoneCount[QID_AC_VO] = 0;
	pAd->RalinkCounters.OneSecTxDoneCount = 0;
	pAd->RalinkCounters.OneSecRxCount = 0;
	pAd->RalinkCounters.OneSecTxAggregationCount = 0;
	pAd->RalinkCounters.OneSecRxAggregationCount = 0;

	return;
}

unsigned long rx_AMSDU;
unsigned long rx_Total;


#define ADHOC_BEACON_LOST_TIME		(8*OS_HZ)  
VOID MlmePeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	ULONG			TxTotalCnt;
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;

#ifdef RT2860
	
	
	
	
	if(pAd->StaCfg.WepStatus<2)
	{
		pAd->StaCfg.WpaSupplicantUP = 0;
	}
	else
	{
		pAd->StaCfg.WpaSupplicantUP = 1;
	}

	{
	    
		
		if ((pAd->Mlme.PeriodicRound % (MLME_TASK_EXEC_MULTIPLE * 2) == 0) &&
			(pAd->StaCfg.bHardwareRadio == TRUE) &&
			(RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)))
		{
			UINT32				data = 0;

			
			RTMP_IO_FORCE_READ32(pAd, GPIO_CTRL_CFG, &data);
			if (data & 0x04)
			{
				pAd->StaCfg.bHwRadio = TRUE;
			}
			else
			{
				pAd->StaCfg.bHwRadio = FALSE;
			}
			if (pAd->StaCfg.bRadio != (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio))
			{
				pAd->StaCfg.bRadio = (pAd->StaCfg.bHwRadio && pAd->StaCfg.bSwRadio);
				if (pAd->StaCfg.bRadio == TRUE)
				{
					MlmeRadioOn(pAd);
					
					pAd->ExtraInfo = EXTRA_INFO_CLEAR;
				}
				else
				{
					MlmeRadioOff(pAd);
					
					pAd->ExtraInfo = HW_RADIO_OFF;
				}
			}
		}
	}
#endif 

	
	
	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_RADIO_OFF |
								fRTMP_ADAPTER_RADIO_MEASUREMENT |
								fRTMP_ADAPTER_RESET_IN_PROGRESS))))
		return;

#ifdef RT2860
	{
		if ((pAd->RalinkCounters.LastReceivedByteCount == pAd->RalinkCounters.ReceivedByteCount) && (pAd->StaCfg.bRadio == TRUE))
		{
			
			pAd->SameRxByteCount++;
		}
		else
			pAd->SameRxByteCount = 0;

		
		if (pAd->SameRxByteCount == 702)
		{
			pAd->SameRxByteCount = 0;
			AsicResetPBF(pAd);
			AsicResetMAC(pAd);
		}

		
		if (((INFRA_ON(pAd)) && (pAd->SameRxByteCount > 20)) || ((IDLE_ON(pAd)) && (pAd->SameRxByteCount > 600)))
		{
			if ((pAd->StaCfg.bRadio == TRUE) && (pAd->SameRxByteCount < 700))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("--->  SameRxByteCount = %lu !!!!!!!!!!!!!!! \n", pAd->SameRxByteCount));
				pAd->SameRxByteCount = 700;
				AsicResetBBP(pAd);
			}
		}

		
		pAd->RalinkCounters.LastReceivedByteCount = pAd->RalinkCounters.ReceivedByteCount;

		if ((pAd->CheckDmaBusyCount > 3) && (IDLE_ON(pAd)))
		{
			pAd->CheckDmaBusyCount = 0;
			AsicResetFromDMABusy(pAd);
		}
	}
#endif 
	RT28XX_MLME_PRE_SANITY_CHECK(pAd);

	{
		
		if (MONITOR_ON(pAd))
			return;

		if (pAd->Mlme.PeriodicRound & 0x1)
		{
			
			if (((pAd->MACVersion & 0xffff) == 0x0101) &&
				(STA_TGN_WIFI_ON(pAd)) &&
				(pAd->CommonCfg.IOTestParm.bToggle == FALSE))

				{
					RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, 0x24Bf);
					pAd->CommonCfg.IOTestParm.bToggle = TRUE;
				}
				else if ((STA_TGN_WIFI_ON(pAd)) &&
						((pAd->MACVersion & 0xffff) == 0x0101))
				{
					RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, 0x243f);
					pAd->CommonCfg.IOTestParm.bToggle = FALSE;
				}
		}
	}

	pAd->bUpdateBcnCntDone = FALSE;


	pAd->Mlme.PeriodicRound ++;

#ifdef RT3070
	
	NICUpdateFifoStaCounters(pAd);
#endif 
	
	if ((pAd->Mlme.PeriodicRound % 5 == 0) && RTMPAutoRateSwitchCheck(pAd))
	{
		
		{
			if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)
					)
				&& (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE)))
				MlmeDynamicTxRateSwitching(pAd);
		}
	}

	
	if (pAd->Mlme.PeriodicRound %MLME_TASK_EXEC_MULTIPLE == 0)
	{
                pAd->Mlme.OneSecPeriodicRound ++;

		if (rx_Total)
		{

			
			rx_AMSDU = 0;
			rx_Total = 0;
		}

		
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_MEDIA_STATE_CHANGE))
		{
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_MEDIA_STATE_CHANGE);
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
			{
				pAd->IndicateMediaState = NdisMediaStateConnected;
				RTMP_IndicateMediaState(pAd);

			}
			else
			{
				pAd->IndicateMediaState = NdisMediaStateDisconnected;
				RTMP_IndicateMediaState(pAd);
			}
		}

		NdisGetSystemUpTime(&pAd->Mlme.Now32);

		
		
		NICUpdateRawCounters(pAd);

#ifdef RT2870
		RT2870_WatchDog(pAd);
#endif 

   		
		ORIBATimerTimeout(pAd);

		
		if (pAd->Mlme.bEnableAutoAntennaCheck)
		{
			TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount +
							 pAd->RalinkCounters.OneSecTxRetryOkCount +
							 pAd->RalinkCounters.OneSecTxFailCount;

			
			if (TxTotalCnt > 50)
			{
				if (pAd->Mlme.OneSecPeriodicRound % 10 == 0)
				{
					AsicEvaluateRxAnt(pAd);
				}
			}
			else
			{
				if (pAd->Mlme.OneSecPeriodicRound % 3 == 0)
				{
					AsicEvaluateRxAnt(pAd);
				}
			}
		}

		STAMlmePeriodicExec(pAd);

		MlmeResetRalinkCounters(pAd);

		{
#ifdef RT2860
			if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) && (pAd->bPCIclkOff == FALSE))
#endif
			{
				
				
				
				
				
				

				UINT32	MacReg = 0;

				RTMP_IO_READ32(pAd, 0x10F4, &MacReg);
				if (((MacReg & 0x20000000) && (MacReg & 0x80)) || ((MacReg & 0x20000000) && (MacReg & 0x20)))
				{
					RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x1);
					RTMPusecDelay(1);
					RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0xC);

					DBGPRINT(RT_DEBUG_WARN,("Warning, MAC specific condition occurs \n"));
				}
			}
		}

		RT28XX_MLME_HANDLER(pAd);
	}

	pAd->bUpdateBcnCntDone = FALSE;
}

VOID STAMlmePeriodicExec(
	PRTMP_ADAPTER pAd)
{
#ifdef RT2860
	ULONG			    TxTotalCnt;
#endif
#ifdef RT2870
	ULONG	TxTotalCnt;
	int 	i;
#endif

    if (pAd->StaCfg.WpaSupplicantUP == WPA_SUPPLICANT_DISABLE)
    {
    	
    	if (pAd->StaCfg.bBlockAssoc && (pAd->StaCfg.LastMicErrorTime + (60 * OS_HZ) < pAd->Mlme.Now32))
    		pAd->StaCfg.bBlockAssoc = FALSE;
    }

#ifdef RT2860
	
	
	
	
	if(pAd->StaCfg.WepStatus<2)
	{
		pAd->StaCfg.WpaSupplicantUP = 0;
	}
	else
	{
		pAd->StaCfg.WpaSupplicantUP = 1;
	}
#endif

    if ((pAd->PreMediaState != pAd->IndicateMediaState) && (pAd->CommonCfg.bWirelessEvent))
	{
		if (pAd->IndicateMediaState == NdisMediaStateConnected)
		{
			RTMPSendWirelessEvent(pAd, IW_STA_LINKUP_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
		}
		pAd->PreMediaState = pAd->IndicateMediaState;
	}

#ifdef RT2860
	if ((pAd->OpMode == OPMODE_STA) && (IDLE_ON(pAd)) &&
        (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)) &&
		(pAd->Mlme.SyncMachine.CurrState == SYNC_IDLE) &&
		(pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE) &&
		(RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP)) &&
		(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)))
	{
		RT28xxPciAsicRadioOff(pAd, GUI_IDLE_POWER_SAVE, 0);
	}
#endif



   	AsicStaBbpTuning(pAd);

	TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount +
					 pAd->RalinkCounters.OneSecTxRetryOkCount +
					 pAd->RalinkCounters.OneSecTxFailCount;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
	{
		
		MlmeCalculateChannelQuality(pAd, pAd->Mlme.Now32);
	}

	
	
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		AsicAdjustTxPower(pAd);

	if (INFRA_ON(pAd))
	{
		
		
		if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
		MlmeCheckPsmChange(pAd, pAd->Mlme.Now32);

		pAd->RalinkCounters.LastOneSecTotalTxCount = TxTotalCnt;

		if ((pAd->StaCfg.LastBeaconRxTime + 1*OS_HZ < pAd->Mlme.Now32) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) &&
			((TxTotalCnt + pAd->RalinkCounters.OneSecRxOkCnt < 600)))
		{
			RTMPSetAGCInitValue(pAd, BW_20);
			DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - No BEACON. restore R66 to the low bound(%d) \n", (0x2E + GET_LNA_GAIN(pAd))));
		}

        {
    		if (pAd->CommonCfg.bAPSDCapable && pAd->CommonCfg.APEdcaParm.bAPSDCapable)
    		{
    		    
    			if ((pAd->Mlme.OneSecPeriodicRound % 20) == 8)
    				RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate, TRUE);
    		}
    		else
    		{
    		    
    			if ((pAd->Mlme.OneSecPeriodicRound % 10) == 8)
                {
                    if (pAd->CommonCfg.bWmmCapable)
    					RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate, TRUE);
                    else
						RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate, FALSE);
                }
    		}
        }

		if (CQI_IS_DEAD(pAd->Mlme.ChannelQuality))
			{
			DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - No BEACON. Dead CQI. Auto Recovery attempt #%ld\n", pAd->RalinkCounters.BadCQIAutoRecoveryCount));
			pAd->StaCfg.CCXAdjacentAPReportFlag = TRUE;
			pAd->StaCfg.CCXAdjacentAPLinkDownTime = pAd->StaCfg.LastBeaconRxTime;

			
			LinkDown(pAd, FALSE);

            {
                union iwreq_data    wrqu;
                memset(wrqu.ap_addr.sa_data, 0, MAC_ADDR_LEN);
                wireless_send_event(pAd->net_dev, SIOCGIWAP, &wrqu, NULL);
            }

			MlmeAutoReconnectLastSSID(pAd);
		}
		else if (CQI_IS_BAD(pAd->Mlme.ChannelQuality))
		{
			pAd->RalinkCounters.BadCQIAutoRecoveryCount ++;
			DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Bad CQI. Auto Recovery attempt #%ld\n", pAd->RalinkCounters.BadCQIAutoRecoveryCount));
			MlmeAutoReconnectLastSSID(pAd);
		}

		
		if (pAd->StaCfg.bFastRoaming)
		{
			SHORT	dBmToRoam = (SHORT)pAd->StaCfg.dBmToRoam;

			DBGPRINT(RT_DEBUG_TRACE, ("Rssi=%d, dBmToRoam=%d\n", RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.LastRssi0, pAd->StaCfg.RssiSample.LastRssi1, pAd->StaCfg.RssiSample.LastRssi2), (CHAR)dBmToRoam));

			if (RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.LastRssi0, pAd->StaCfg.RssiSample.LastRssi1, pAd->StaCfg.RssiSample.LastRssi2) <= (CHAR)dBmToRoam)
			{
				MlmeCheckForFastRoaming(pAd, pAd->Mlme.Now32);
			}
		}
	}
	else if (ADHOC_ON(pAd))
	{
#ifdef RT2860
		
		
		
		
		
		
		

		
		
		if ((pAd->CommonCfg.Channel <= 14)			   &&
			(pAd->CommonCfg.MaxTxRate <= RATE_11)	   &&
			(pAd->CommonCfg.MaxDesiredRate > RATE_11)  &&
			((pAd->StaCfg.Last11bBeaconRxTime + 5*OS_HZ) < pAd->Mlme.Now32))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - last 11B peer left, update Tx rates\n"));
			NdisMoveMemory(pAd->StaActive.SupRate, pAd->CommonCfg.SupRate, MAX_LEN_OF_SUPPORTED_RATES);
			pAd->StaActive.SupRateLen = pAd->CommonCfg.SupRateLen;
			MlmeUpdateTxRates(pAd, FALSE, 0);
			MakeIbssBeacon(pAd);		
			AsicEnableIbssSync(pAd);	
			pAd->StaCfg.AdhocBOnlyJoined = FALSE;
		}

		if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
		{
			if ((pAd->StaCfg.AdhocBGJoined) &&
				((pAd->StaCfg.Last11gBeaconRxTime + 5 * OS_HZ) < pAd->Mlme.Now32))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - last 11G peer left\n"));
				pAd->StaCfg.AdhocBGJoined = FALSE;
			}

			if ((pAd->StaCfg.Adhoc20NJoined) &&
				((pAd->StaCfg.Last20NBeaconRxTime + 5 * OS_HZ) < pAd->Mlme.Now32))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - last 20MHz N peer left\n"));
				pAd->StaCfg.Adhoc20NJoined = FALSE;
			}
		}
#endif 

		
		if ((pAd->CommonCfg.Channel > 14)
			&& (pAd->CommonCfg.bIEEE80211H == 1)
			&& RadarChannelCheck(pAd, pAd->CommonCfg.Channel))
		{
			RadarDetectPeriodic(pAd);
		}

		
		
		
		if ((pAd->StaCfg.LastBeaconRxTime + ADHOC_BEACON_LOST_TIME < pAd->Mlme.Now32) &&
			OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
		{
			MLME_START_REQ_STRUCT     StartReq;

			DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - excessive BEACON lost, last STA in this IBSS, MediaState=Disconnected\n"));
			LinkDown(pAd, FALSE);

			StartParmFill(pAd, &StartReq, pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
			MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_START_REQ, sizeof(MLME_START_REQ_STRUCT), &StartReq);
			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_START;
		}

#ifdef RT2870
		for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++)
		{
			MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];

			if (pEntry->ValidAsCLI == FALSE)
				continue;

			if (pEntry->LastBeaconRxTime + ADHOC_BEACON_LOST_TIME < pAd->Mlme.Now32)
				MacTableDeleteEntry(pAd, pEntry->Aid, pEntry->Addr);
		}
#endif
	}
	else 
	{

		if (pAd->StaCfg.bScanReqIsFromWebUI &&
            ((pAd->StaCfg.LastScanTime + 30 * OS_HZ) > pAd->Mlme.Now32))
			goto SKIP_AUTO_SCAN_CONN;
        else
            pAd->StaCfg.bScanReqIsFromWebUI = FALSE;

		if ((pAd->StaCfg.bAutoReconnect == TRUE)
			&& RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)
			&& (MlmeValidateSSID(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen) == TRUE))
		{
			if ((pAd->ScanTab.BssNr==0) && (pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE))
			{
				MLME_SCAN_REQ_STRUCT	   ScanReq;

				if ((pAd->StaCfg.LastScanTime + 10 * OS_HZ) < pAd->Mlme.Now32)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("STAMlmePeriodicExec():CNTL - ScanTab.BssNr==0, start a new ACTIVE scan SSID[%s]\n", pAd->MlmeAux.AutoReconnectSsid));
					ScanParmFill(pAd, &ScanReq, pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen, BSS_ANY, SCAN_ACTIVE);
					MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq);
					pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;
					
					pAd->StaCfg.LastScanTime = pAd->Mlme.Now32;
				}
				else if (pAd->StaCfg.BssType == BSS_ADHOC)	
					MlmeAutoReconnectLastSSID(pAd);
			}
			else if (pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE)
			{
				if ((pAd->Mlme.OneSecPeriodicRound % 7) == 0)
				{
					MlmeAutoScan(pAd);
					pAd->StaCfg.LastScanTime = pAd->Mlme.Now32;
				}
				else
				{
						MlmeAutoReconnectLastSSID(pAd);
				}
			}
		}
	}

SKIP_AUTO_SCAN_CONN:

    if ((pAd->MacTab.Content[BSSID_WCID].TXBAbitmap !=0) && (pAd->MacTab.fAnyBASession == FALSE))
	{
		pAd->MacTab.fAnyBASession = TRUE;
		AsicUpdateProtect(pAd, HT_FORCERTSCTS,  ALLN_SETPROTECT, FALSE, FALSE);
	}
	else if ((pAd->MacTab.Content[BSSID_WCID].TXBAbitmap ==0) && (pAd->MacTab.fAnyBASession == TRUE))
	{
		pAd->MacTab.fAnyBASession = FALSE;
		AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode,  ALLN_SETPROTECT, FALSE, FALSE);
	}

	return;
}


VOID LinkDownExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{

	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	pAd->IndicateMediaState = NdisMediaStateDisconnected;
	RTMP_IndicateMediaState(pAd);
    pAd->ExtraInfo = GENERAL_LINK_DOWN;
}


VOID MlmeAutoScan(
	IN PRTMP_ADAPTER pAd)
{
	
	if (pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Driver auto scan\n"));
		MlmeEnqueue(pAd,
					MLME_CNTL_STATE_MACHINE,
					OID_802_11_BSSID_LIST_SCAN,
					0,
					NULL);
		RT28XX_MLME_HANDLER(pAd);
	}
}


VOID MlmeAutoReconnectLastSSID(
	IN PRTMP_ADAPTER pAd)
{


	
	if ((pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE) &&
		(MlmeValidateSSID(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen) == TRUE))
	{
		NDIS_802_11_SSID OidSsid;
		OidSsid.SsidLength = pAd->MlmeAux.AutoReconnectSsidLen;
		NdisMoveMemory(OidSsid.Ssid, pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

		DBGPRINT(RT_DEBUG_TRACE, ("Driver auto reconnect to last OID_802_11_SSID setting - %s, len - %d\n", pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen));
		MlmeEnqueue(pAd,
					MLME_CNTL_STATE_MACHINE,
					OID_802_11_SSID,
					sizeof(NDIS_802_11_SSID),
					&OidSsid);
		RT28XX_MLME_HANDLER(pAd);
	}
}


BOOLEAN MlmeValidateSSID(
	IN PUCHAR	pSsid,
	IN UCHAR	SsidLen)
{
	int	index;

	if (SsidLen > MAX_LEN_OF_SSID)
		return (FALSE);

	
	for (index = 0; index < SsidLen; index++)
	{
		if (pSsid[index] < 0x20)
			return (FALSE);
	}

	
	return (TRUE);
}

VOID MlmeSelectTxRateTable(
	IN PRTMP_ADAPTER		pAd,
	IN PMAC_TABLE_ENTRY		pEntry,
	IN PUCHAR				*ppTable,
	IN PUCHAR				pTableSize,
	IN PUCHAR				pInitTxRateIdx)
{
	do
	{
		
		if (pAd->CommonCfg.TxRateTableSize > 0)
		{
			*ppTable = RateSwitchTable;
			*pTableSize = RateSwitchTable[0];
			*pInitTxRateIdx = RateSwitchTable[1];

			break;
		}

		if ((pAd->OpMode == OPMODE_STA) && ADHOC_ON(pAd))
		{
			if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) &&
#ifdef RT2860
				!pAd->StaCfg.AdhocBOnlyJoined &&
				!pAd->StaCfg.AdhocBGJoined &&
				(pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&
				((pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))
#endif
#ifdef RT2870
				(pEntry->HTCapability.MCSSet[0] == 0xff) &&
				((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))
#endif
			{
				*ppTable = RateSwitchTable11N1S;
				*pTableSize = RateSwitchTable11N1S[0];
				*pInitTxRateIdx = RateSwitchTable11N1S[1];

			}
			else if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) &&
#ifdef RT2860
					!pAd->StaCfg.AdhocBOnlyJoined &&
					!pAd->StaCfg.AdhocBGJoined &&
					(pAd->StaActive.SupportedPhyInfo.MCSSet[0] == 0xff) &&
					(pAd->StaActive.SupportedPhyInfo.MCSSet[1] == 0xff) &&
#endif
#ifdef RT2870
					(pEntry->HTCapability.MCSSet[0] == 0xff) &&
					(pEntry->HTCapability.MCSSet[1] == 0xff) &&
#endif
					(pAd->Antenna.field.TxPath == 2))
			{
				if (pAd->LatchRfRegs.Channel <= 14)
				{
					*ppTable = RateSwitchTable11N2S;
					*pTableSize = RateSwitchTable11N2S[0];
					*pInitTxRateIdx = RateSwitchTable11N2S[1];
				}
				else
				{
					*ppTable = RateSwitchTable11N2SForABand;
					*pTableSize = RateSwitchTable11N2SForABand[0];
					*pInitTxRateIdx = RateSwitchTable11N2SForABand[1];
				}

			}
			else
#ifdef RT2860
				if (pAd->CommonCfg.PhyMode == PHY_11B)
			{
				*ppTable = RateSwitchTable11B;
				*pTableSize = RateSwitchTable11B[0];
				*pInitTxRateIdx = RateSwitchTable11B[1];

			}
	        else if((pAd->LatchRfRegs.Channel <= 14) && (pAd->StaCfg.AdhocBOnlyJoined == TRUE))
#endif
#ifdef RT2870
				if ((pEntry->RateLen == 4)
					&& (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
					)
#endif
			{
				
				*ppTable = RateSwitchTable11B;
				*pTableSize = RateSwitchTable11B[0];
				*pInitTxRateIdx = RateSwitchTable11B[1];

			}
			else if (pAd->LatchRfRegs.Channel <= 14)
			{
				*ppTable = RateSwitchTable11BG;
				*pTableSize = RateSwitchTable11BG[0];
				*pInitTxRateIdx = RateSwitchTable11BG[1];

			}
			else
			{
				*ppTable = RateSwitchTable11G;
				*pTableSize = RateSwitchTable11G[0];
				*pInitTxRateIdx = RateSwitchTable11G[1];

			}
			break;
		}

		if ((pEntry->RateLen == 12) && (pEntry->HTCapability.MCSSet[0] == 0xff) &&
			((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->CommonCfg.TxStream == 1)))
		{
			*ppTable = RateSwitchTable11BGN1S;
			*pTableSize = RateSwitchTable11BGN1S[0];
			*pInitTxRateIdx = RateSwitchTable11BGN1S[1];

			break;
		}

		if ((pEntry->RateLen == 12) && (pEntry->HTCapability.MCSSet[0] == 0xff) &&
			(pEntry->HTCapability.MCSSet[1] == 0xff) && (pAd->CommonCfg.TxStream == 2))
		{
			if (pAd->LatchRfRegs.Channel <= 14)
			{
				*ppTable = RateSwitchTable11BGN2S;
				*pTableSize = RateSwitchTable11BGN2S[0];
				*pInitTxRateIdx = RateSwitchTable11BGN2S[1];

			}
			else
			{
				*ppTable = RateSwitchTable11BGN2SForABand;
				*pTableSize = RateSwitchTable11BGN2SForABand[0];
				*pInitTxRateIdx = RateSwitchTable11BGN2SForABand[1];

			}
			break;
		}

		if ((pEntry->HTCapability.MCSSet[0] == 0xff) && ((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->CommonCfg.TxStream == 1)))
		{
			*ppTable = RateSwitchTable11N1S;
			*pTableSize = RateSwitchTable11N1S[0];
			*pInitTxRateIdx = RateSwitchTable11N1S[1];

			break;
		}

		if ((pEntry->HTCapability.MCSSet[0] == 0xff) && (pEntry->HTCapability.MCSSet[1] == 0xff) && (pAd->CommonCfg.TxStream == 2))
		{
			if (pAd->LatchRfRegs.Channel <= 14)
			{
			*ppTable = RateSwitchTable11N2S;
			*pTableSize = RateSwitchTable11N2S[0];
			*pInitTxRateIdx = RateSwitchTable11N2S[1];
            }
			else
			{
				*ppTable = RateSwitchTable11N2SForABand;
				*pTableSize = RateSwitchTable11N2SForABand[0];
				*pInitTxRateIdx = RateSwitchTable11N2SForABand[1];
			}

			break;
		}

		
		if (pEntry->RateLen == 4)
		{
			*ppTable = RateSwitchTable11B;
			*pTableSize = RateSwitchTable11B[0];
			*pInitTxRateIdx = RateSwitchTable11B[1];

			break;
		}

		
		if ((pEntry->RateLen > 8)
			&& (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
			)
		{
			*ppTable = RateSwitchTable11BG;
			*pTableSize = RateSwitchTable11BG[0];
			*pInitTxRateIdx = RateSwitchTable11BG[1];

			break;
		}

		
		if ((pEntry->RateLen == 8)
			&& (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
			)
		{
			*ppTable = RateSwitchTable11G;
			*pTableSize = RateSwitchTable11G[0];
			*pInitTxRateIdx = RateSwitchTable11G[1];

			break;
		}

		{
			
			if ((pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0))
			{	
				if (pAd->CommonCfg.MaxTxRate <= RATE_11)
				{
					*ppTable = RateSwitchTable11B;
					*pTableSize = RateSwitchTable11B[0];
					*pInitTxRateIdx = RateSwitchTable11B[1];
				}
				else if ((pAd->CommonCfg.MaxTxRate > RATE_11) && (pAd->CommonCfg.MinTxRate > RATE_11))
				{
					*ppTable = RateSwitchTable11G;
					*pTableSize = RateSwitchTable11G[0];
					*pInitTxRateIdx = RateSwitchTable11G[1];

				}
				else
				{
					*ppTable = RateSwitchTable11BG;
					*pTableSize = RateSwitchTable11BG[0];
					*pInitTxRateIdx = RateSwitchTable11BG[1];
				}
				break;
			}

			if (pAd->LatchRfRegs.Channel <= 14)
			{
				if (pAd->CommonCfg.TxStream == 1)
				{
					*ppTable = RateSwitchTable11N1S;
					*pTableSize = RateSwitchTable11N1S[0];
					*pInitTxRateIdx = RateSwitchTable11N1S[1];
					DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode,default use 11N 1S AP \n"));
				}
				else
				{
					*ppTable = RateSwitchTable11N2S;
					*pTableSize = RateSwitchTable11N2S[0];
					*pInitTxRateIdx = RateSwitchTable11N2S[1];
					DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode,default use 11N 2S AP \n"));
				}
			}
			else
			{
				if (pAd->CommonCfg.TxStream == 1)
				{
					*ppTable = RateSwitchTable11N1S;
					*pTableSize = RateSwitchTable11N1S[0];
					*pInitTxRateIdx = RateSwitchTable11N1S[1];
					DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode,default use 11N 1S AP \n"));
				}
				else
				{
					*ppTable = RateSwitchTable11N2SForABand;
					*pTableSize = RateSwitchTable11N2SForABand[0];
					*pInitTxRateIdx = RateSwitchTable11N2SForABand[1];
					DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode,default use 11N 2S AP \n"));
				}
			}

			DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode (SupRateLen=%d, ExtRateLen=%d, MCSSet[0]=0x%x, MCSSet[1]=0x%x)\n",
				pAd->StaActive.SupRateLen, pAd->StaActive.ExtRateLen, pAd->StaActive.SupportedPhyInfo.MCSSet[0], pAd->StaActive.SupportedPhyInfo.MCSSet[1]));
		}
	} while(FALSE);
}


VOID MlmeCheckForRoaming(
	IN PRTMP_ADAPTER pAd,
	IN ULONG	Now32)
{
	USHORT	   i;
	BSS_TABLE  *pRoamTab = &pAd->MlmeAux.RoamTab;
	BSS_ENTRY  *pBss;

	DBGPRINT(RT_DEBUG_TRACE, ("==> MlmeCheckForRoaming\n"));
	
	BssTableInit(pRoamTab);
	for (i = 0; i < pAd->ScanTab.BssNr; i++)
	{
		pBss = &pAd->ScanTab.BssEntry[i];

		if ((pBss->LastBeaconRxTime + BEACON_LOST_TIME) < Now32)
			continue;	 
		if (pBss->Rssi <= RSSI_THRESHOLD_FOR_ROAMING)
			continue;	 
		if (MAC_ADDR_EQUAL(pBss->Bssid, pAd->CommonCfg.Bssid))
			continue;	 
		if (pBss->Rssi < (pAd->StaCfg.RssiSample.LastRssi0 + RSSI_DELTA))
			continue;	 

		
		NdisMoveMemory(&pRoamTab->BssEntry[pRoamTab->BssNr], pBss, sizeof(BSS_ENTRY));
		pRoamTab->BssNr += 1;
	}

	if (pRoamTab->BssNr > 0)
	{
		
		if (pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE)
		{
			pAd->RalinkCounters.PoorCQIRoamingCount ++;
			DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Roaming attempt #%ld\n", pAd->RalinkCounters.PoorCQIRoamingCount));
			MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_MLME_ROAMING_REQ, 0, NULL);
			RT28XX_MLME_HANDLER(pAd);
		}
	}
	DBGPRINT(RT_DEBUG_TRACE, ("<== MlmeCheckForRoaming(# of candidate= %d)\n",pRoamTab->BssNr));
}


VOID MlmeCheckForFastRoaming(
	IN	PRTMP_ADAPTER	pAd,
	IN	ULONG			Now)
{
	USHORT		i;
	BSS_TABLE	*pRoamTab = &pAd->MlmeAux.RoamTab;
	BSS_ENTRY	*pBss;

	DBGPRINT(RT_DEBUG_TRACE, ("==> MlmeCheckForFastRoaming\n"));
	
	BssTableInit(pRoamTab);
	for (i = 0; i < pAd->ScanTab.BssNr; i++)
	{
		pBss = &pAd->ScanTab.BssEntry[i];

        if ((pBss->Rssi <= -50) && (pBss->Channel == pAd->CommonCfg.Channel))
			continue;	 
		if (MAC_ADDR_EQUAL(pBss->Bssid, pAd->CommonCfg.Bssid))
			continue;	 
		if (!SSID_EQUAL(pBss->Ssid, pBss->SsidLen, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen))
			continue;	 
        if (pBss->Rssi < (RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.LastRssi0, pAd->StaCfg.RssiSample.LastRssi1, pAd->StaCfg.RssiSample.LastRssi2) + RSSI_DELTA))
			continue;	 

        DBGPRINT(RT_DEBUG_TRACE, ("LastRssi0 = %d, pBss->Rssi = %d\n", RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.LastRssi0, pAd->StaCfg.RssiSample.LastRssi1, pAd->StaCfg.RssiSample.LastRssi2), pBss->Rssi));
		
		NdisMoveMemory(&pRoamTab->BssEntry[pRoamTab->BssNr], pBss, sizeof(BSS_ENTRY));
		pRoamTab->BssNr += 1;
	}

	if (pRoamTab->BssNr > 0)
	{
		
		if (pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE)
		{
			pAd->RalinkCounters.PoorCQIRoamingCount ++;
			DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Roaming attempt #%ld\n", pAd->RalinkCounters.PoorCQIRoamingCount));
			MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_MLME_ROAMING_REQ, 0, NULL);
			RT28XX_MLME_HANDLER(pAd);
		}
	}
	
	else
	{
		if ((pAd->StaCfg.LastScanTime + 10 * 1000) < Now)
		{
			
			DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Roaming, No eligable entry, try new scan!\n"));
			pAd->StaCfg.ScanCnt = 2;
			pAd->StaCfg.LastScanTime = Now;
			MlmeAutoScan(pAd);
		}
	}

    DBGPRINT(RT_DEBUG_TRACE, ("<== MlmeCheckForFastRoaming (BssNr=%d)\n", pRoamTab->BssNr));
}


VOID MlmeCalculateChannelQuality(
	IN PRTMP_ADAPTER pAd,
	IN ULONG Now32)
{
	ULONG TxOkCnt, TxCnt, TxPER, TxPRR;
	ULONG RxCnt, RxPER;
	UCHAR NorRssi;
	CHAR  MaxRssi;
	ULONG BeaconLostTime = BEACON_LOST_TIME;

	MaxRssi = RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.LastRssi0, pAd->StaCfg.RssiSample.LastRssi1, pAd->StaCfg.RssiSample.LastRssi2);

	
	
	
	TxOkCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount + pAd->RalinkCounters.OneSecTxRetryOkCount;
	TxCnt = TxOkCnt + pAd->RalinkCounters.OneSecTxFailCount;
	if (TxCnt < 5)
	{
		TxPER = 0;
		TxPRR = 0;
	}
	else
	{
		TxPER = (pAd->RalinkCounters.OneSecTxFailCount * 100) / TxCnt;
		TxPRR = ((TxCnt - pAd->RalinkCounters.OneSecTxNoRetryOkCount) * 100) / TxCnt;
	}

	
	
	
	RxCnt = pAd->RalinkCounters.OneSecRxOkCnt + pAd->RalinkCounters.OneSecRxFcsErrCnt;
	if (RxCnt < 5)
		RxPER = 0;
	else
		RxPER = (pAd->RalinkCounters.OneSecRxFcsErrCnt * 100) / RxCnt;

	
	
	
	if (INFRA_ON(pAd) &&
		(pAd->RalinkCounters.OneSecTxNoRetryOkCount < 2) && 
		(pAd->StaCfg.LastBeaconRxTime + BeaconLostTime < Now32))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("BEACON lost > %ld msec with TxOkCnt=%ld -> CQI=0\n", BeaconLostTime, TxOkCnt));
		pAd->Mlme.ChannelQuality = 0;
	}
	else
	{
		
		if (MaxRssi > -40)
			NorRssi = 100;
		else if (MaxRssi < -90)
			NorRssi = 0;
		else
			NorRssi = (MaxRssi + 90) * 2;

		
		pAd->Mlme.ChannelQuality = (RSSI_WEIGHTING * NorRssi +
								   TX_WEIGHTING * (100 - TxPRR) +
								   RX_WEIGHTING* (100 - RxPER)) / 100;
		if (pAd->Mlme.ChannelQuality >= 100)
			pAd->Mlme.ChannelQuality = 100;
	}

}

VOID MlmeSetTxRate(
	IN PRTMP_ADAPTER		pAd,
	IN PMAC_TABLE_ENTRY		pEntry,
	IN PRTMP_TX_RATE_SWITCH	pTxRate)
{
	UCHAR	MaxMode = MODE_OFDM;

	MaxMode = MODE_HTGREENFIELD;

	if (pTxRate->STBC && (pAd->StaCfg.MaxHTPhyMode.field.STBC) && (pAd->Antenna.field.TxPath == 2))
		pAd->StaCfg.HTPhyMode.field.STBC = STBC_USE;
	else
		pAd->StaCfg.HTPhyMode.field.STBC = STBC_NONE;

	if (pTxRate->CurrMCS < MCS_AUTO)
		pAd->StaCfg.HTPhyMode.field.MCS = pTxRate->CurrMCS;

	if (pAd->StaCfg.HTPhyMode.field.MCS > 7)
		pAd->StaCfg.HTPhyMode.field.STBC = STBC_NONE;

   	if (ADHOC_ON(pAd))
	{
		
		pAd->StaCfg.HTPhyMode.field.ShortGI = GI_800;
		pEntry->HTPhyMode.field.STBC	= STBC_NONE;

		
		
		
		pEntry->HTPhyMode.field.MODE	= pTxRate->Mode;
		pEntry->HTPhyMode.field.ShortGI	= pAd->StaCfg.HTPhyMode.field.ShortGI;
		pEntry->HTPhyMode.field.MCS		= pAd->StaCfg.HTPhyMode.field.MCS;

		
		pAd->StaCfg.HTPhyMode.field.MODE = pEntry->HTPhyMode.field.MODE;
	}
	else
	{
		if (pTxRate->Mode <= MaxMode)
			pAd->StaCfg.HTPhyMode.field.MODE = pTxRate->Mode;

		if (pTxRate->ShortGI && (pAd->StaCfg.MaxHTPhyMode.field.ShortGI))
			pAd->StaCfg.HTPhyMode.field.ShortGI = GI_400;
		else
			pAd->StaCfg.HTPhyMode.field.ShortGI = GI_800;

		
		if (pAd->StaCfg.HTPhyMode.field.ShortGI == GI_400)
		{
			if ((pEntry->HTPhyMode.field.BW == BW_20) && (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE)))
				pAd->StaCfg.HTPhyMode.field.ShortGI = GI_800;
			if ((pEntry->HTPhyMode.field.BW == BW_40) && (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE)))
				pAd->StaCfg.HTPhyMode.field.ShortGI = GI_800;
		}

		
		if ((pEntry->HTPhyMode.field.MCS == 0) && (pAd->StaCfg.HTPhyMode.field.MCS != 0))
		{
			pEntry->HTPhyMode.field.MCS		= pAd->StaCfg.HTPhyMode.field.MCS;
			if (pAd->MacTab.fAnyBASession)
			{
				AsicUpdateProtect(pAd, HT_FORCERTSCTS, ALLN_SETPROTECT, TRUE, (BOOLEAN)pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent);
			}
			else
			{
				AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, ALLN_SETPROTECT, TRUE, (BOOLEAN)pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent);
			}
		}
		else if ((pEntry->HTPhyMode.field.MCS == 8) && (pAd->StaCfg.HTPhyMode.field.MCS != 8))
		{
			pEntry->HTPhyMode.field.MCS		= pAd->StaCfg.HTPhyMode.field.MCS;
			if (pAd->MacTab.fAnyBASession)
			{
				AsicUpdateProtect(pAd, HT_FORCERTSCTS, ALLN_SETPROTECT, TRUE, (BOOLEAN)pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent);
			}
			else
			{
				AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, ALLN_SETPROTECT, TRUE, (BOOLEAN)pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent);
			}
		}
		else if ((pEntry->HTPhyMode.field.MCS != 0) && (pAd->StaCfg.HTPhyMode.field.MCS == 0))
		{
			AsicUpdateProtect(pAd, HT_RTSCTS_6M, ALLN_SETPROTECT, TRUE, (BOOLEAN)pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent);

		}
		else if ((pEntry->HTPhyMode.field.MCS != 8) && (pAd->StaCfg.HTPhyMode.field.MCS == 8))
		{
			AsicUpdateProtect(pAd, HT_RTSCTS_6M, ALLN_SETPROTECT, TRUE, (BOOLEAN)pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent);
		}

		pEntry->HTPhyMode.field.STBC	= pAd->StaCfg.HTPhyMode.field.STBC;
		pEntry->HTPhyMode.field.ShortGI	= pAd->StaCfg.HTPhyMode.field.ShortGI;
		pEntry->HTPhyMode.field.MCS		= pAd->StaCfg.HTPhyMode.field.MCS;
		pEntry->HTPhyMode.field.MODE	= pAd->StaCfg.HTPhyMode.field.MODE;

		if ((pAd->StaCfg.MaxHTPhyMode.field.MODE == MODE_HTGREENFIELD) &&
		    pAd->WIFItestbed.bGreenField)
		    pEntry->HTPhyMode.field.MODE = MODE_HTGREENFIELD;
	}

	pAd->LastTxRate = (USHORT)(pEntry->HTPhyMode.word);
}


VOID MlmeDynamicTxRateSwitching(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR					UpRateIdx = 0, DownRateIdx = 0, CurrRateIdx;
	ULONG					i, AccuTxTotalCnt = 0, TxTotalCnt;
	ULONG					TxErrorRatio = 0;
	BOOLEAN					bTxRateChanged, bUpgradeQuality = FALSE;
	PRTMP_TX_RATE_SWITCH	pCurrTxRate, pNextTxRate = NULL;
	PUCHAR					pTable;
	UCHAR					TableSize = 0;
	UCHAR					InitTxRateIdx = 0, TrainUp, TrainDown;
	CHAR					Rssi, RssiOffset = 0;
	TX_STA_CNT1_STRUC		StaTx1;
	TX_STA_CNT0_STRUC		TxStaCnt0;
	ULONG					TxRetransmit = 0, TxSuccess = 0, TxFailCount = 0;
	MAC_TABLE_ENTRY			*pEntry;

	
	
	
   	for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		pEntry = &pAd->MacTab.Content[i];

		
		if (RTMPCheckEntryEnableAutoRateSwitch(pAd, pEntry) == FALSE)
			continue;

		if ((pAd->MacTab.Size == 1) || (pEntry->ValidAsDls))
		{
#ifdef RT2860
			Rssi = RTMPMaxRssi(pAd, (CHAR)pAd->StaCfg.RssiSample.AvgRssi0, (CHAR)pAd->StaCfg.RssiSample.AvgRssi1, (CHAR)pAd->StaCfg.RssiSample.AvgRssi2);
#endif
#ifdef RT2870
			Rssi = RTMPMaxRssi(pAd,
							   pAd->StaCfg.RssiSample.AvgRssi0,
							   pAd->StaCfg.RssiSample.AvgRssi1,
							   pAd->StaCfg.RssiSample.AvgRssi2);
#endif

			
			RTMP_IO_READ32(pAd, TX_STA_CNT0, &TxStaCnt0.word);
			RTMP_IO_READ32(pAd, TX_STA_CNT1, &StaTx1.word);
			pAd->bUpdateBcnCntDone = TRUE;
			TxRetransmit = StaTx1.field.TxRetransmit;
			TxSuccess = StaTx1.field.TxSuccess;
			TxFailCount = TxStaCnt0.field.TxFailCount;
			TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;

			pAd->RalinkCounters.OneSecTxRetryOkCount += StaTx1.field.TxRetransmit;
			pAd->RalinkCounters.OneSecTxNoRetryOkCount += StaTx1.field.TxSuccess;
			pAd->RalinkCounters.OneSecTxFailCount += TxStaCnt0.field.TxFailCount;
			pAd->WlanCounters.TransmittedFragmentCount.u.LowPart += StaTx1.field.TxSuccess;
			pAd->WlanCounters.RetryCount.u.LowPart += StaTx1.field.TxRetransmit;
			pAd->WlanCounters.FailedCount.u.LowPart += TxStaCnt0.field.TxFailCount;

			
			
			
			AccuTxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount +
						 pAd->RalinkCounters.OneSecTxRetryOkCount +
						 pAd->RalinkCounters.OneSecTxFailCount;

			if (TxTotalCnt)
				TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;
		}
		else
		{
#ifdef RT2860
			Rssi = RTMPMaxRssi(pAd, (CHAR)pEntry->RssiSample.AvgRssi0, (CHAR)pEntry->RssiSample.AvgRssi1, (CHAR)pEntry->RssiSample.AvgRssi2);
#endif
#ifdef RT2870
			if (INFRA_ON(pAd) && (i == 1))
				Rssi = RTMPMaxRssi(pAd,
								   pAd->StaCfg.RssiSample.AvgRssi0,
								   pAd->StaCfg.RssiSample.AvgRssi1,
								   pAd->StaCfg.RssiSample.AvgRssi2);
			else
				Rssi = RTMPMaxRssi(pAd,
								   pEntry->RssiSample.AvgRssi0,
								   pEntry->RssiSample.AvgRssi1,
								   pEntry->RssiSample.AvgRssi2);
#endif

			TxTotalCnt = pEntry->OneSecTxNoRetryOkCount +
				 pEntry->OneSecTxRetryOkCount +
				 pEntry->OneSecTxFailCount;

			if (TxTotalCnt)
				TxErrorRatio = ((pEntry->OneSecTxRetryOkCount + pEntry->OneSecTxFailCount) * 100) / TxTotalCnt;
		}

		CurrRateIdx = pEntry->CurrTxRateIndex;

		MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);

		if (CurrRateIdx >= TableSize)
		{
			CurrRateIdx = TableSize - 1;
		}

		
		
		pCurrTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(CurrRateIdx+1)*5];
		if ((pEntry->HTPhyMode.field.MCS != pCurrTxRate->CurrMCS)
			
			)
		{

			
			
			pCurrTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(InitTxRateIdx+1)*5];
			pEntry->CurrTxRateIndex = InitTxRateIdx;
			MlmeSetTxRate(pAd, pEntry, pCurrTxRate);

			
			RESET_ONE_SEC_TX_CNT(pEntry);
			continue;
		}

		
		if ((CurrRateIdx > 0) && (CurrRateIdx < (TableSize - 1)))
		{
			UpRateIdx = CurrRateIdx + 1;
			DownRateIdx = CurrRateIdx -1;
		}
		else if (CurrRateIdx == 0)
		{
			UpRateIdx = CurrRateIdx + 1;
			DownRateIdx = CurrRateIdx;
		}
		else if (CurrRateIdx == (TableSize - 1))
		{
			UpRateIdx = CurrRateIdx;
			DownRateIdx = CurrRateIdx - 1;
		}

		pCurrTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(CurrRateIdx+1)*5];

		if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
		{
			TrainUp		= (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
			TrainDown	= (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
		}
		else
		{
			TrainUp		= pCurrTxRate->TrainUp;
			TrainDown	= pCurrTxRate->TrainDown;
		}

		

		
		
		
		pEntry->LastTimeTxRateChangeAction = pEntry->LastSecTxRateChangeAction;



		
		
		
		
		if (TxTotalCnt <= 15)
		{
			CHAR	idx = 0;
			UCHAR	TxRateIdx;
			
			UCHAR	MCS0 = 0, MCS1 = 0, MCS2 = 0, MCS3 = 0, MCS4 = 0,  MCS5 =0, MCS6 = 0, MCS7 = 0;
	        UCHAR	MCS12 = 0, MCS13 = 0, MCS14 = 0, MCS15 = 0;
			UCHAR	MCS20 = 0, MCS21 = 0, MCS22 = 0, MCS23 = 0; 

			
			while (idx < pTable[0])
			{
				pCurrTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(idx+1)*5];

				if (pCurrTxRate->CurrMCS == MCS_0)
				{
					MCS0 = idx;
				}
				else if (pCurrTxRate->CurrMCS == MCS_1)
				{
					MCS1 = idx;
				}
				else if (pCurrTxRate->CurrMCS == MCS_2)
				{
					MCS2 = idx;
				}
				else if (pCurrTxRate->CurrMCS == MCS_3)
				{
					MCS3 = idx;
				}
				else if (pCurrTxRate->CurrMCS == MCS_4)
				{
					MCS4 = idx;
				}
	            else if (pCurrTxRate->CurrMCS == MCS_5)
	            {
	                MCS5 = idx;
	            }
	            else if (pCurrTxRate->CurrMCS == MCS_6)
	            {
	                MCS6 = idx;
	            }
				
				else if ((pCurrTxRate->CurrMCS == MCS_7) && (pCurrTxRate->ShortGI == GI_800))	
				{
					MCS7 = idx;
				}
				else if (pCurrTxRate->CurrMCS == MCS_12)
				{
					MCS12 = idx;
				}
				else if (pCurrTxRate->CurrMCS == MCS_13)
				{
					MCS13 = idx;
				}
				else if (pCurrTxRate->CurrMCS == MCS_14)
				{
					MCS14 = idx;
				}
				else if ((pCurrTxRate->CurrMCS == MCS_15) && (pCurrTxRate->ShortGI == GI_800))	
				{
					MCS15 = idx;
				}
				else if (pCurrTxRate->CurrMCS == MCS_20) 
				{
					MCS20 = idx;
				}
				else if (pCurrTxRate->CurrMCS == MCS_21)
				{
					MCS21 = idx;
				}
				else if (pCurrTxRate->CurrMCS == MCS_22)
				{
					MCS22 = idx;
				}
				else if (pCurrTxRate->CurrMCS == MCS_23)
				{
					MCS23 = idx;
				}
				idx ++;
			}

			if (pAd->LatchRfRegs.Channel <= 14)
			{
				if (pAd->NicConfig2.field.ExternalLNAForG)
				{
					RssiOffset = 2;
				}
				else
				{
					RssiOffset = 5;
				}
			}
			else
			{
				if (pAd->NicConfig2.field.ExternalLNAForA)
				{
					RssiOffset = 5;
				}
				else
				{
					RssiOffset = 8;
				}
			}

			
			if ((pTable == RateSwitchTable11BGN3S) ||
				(pTable == RateSwitchTable11N3S) ||
				(pTable == RateSwitchTable))
			{
				if (MCS23 && (Rssi >= -70))
					TxRateIdx = MCS15;
				else if (MCS22 && (Rssi >= -72))
					TxRateIdx = MCS14;
        	    else if (MCS21 && (Rssi >= -76))
					TxRateIdx = MCS13;
				else if (MCS20 && (Rssi >= -78))
					TxRateIdx = MCS12;
			else if (MCS4 && (Rssi >= -82))
				TxRateIdx = MCS4;
			else if (MCS3 && (Rssi >= -84))
				TxRateIdx = MCS3;
			else if (MCS2 && (Rssi >= -86))
				TxRateIdx = MCS2;
			else if (MCS1 && (Rssi >= -88))
				TxRateIdx = MCS1;
			else
				TxRateIdx = MCS0;
		}
		else if ((pTable == RateSwitchTable11BGN2S) || (pTable == RateSwitchTable11BGN2SForABand) ||(pTable == RateSwitchTable11N2S) ||(pTable == RateSwitchTable11N2SForABand)) 
			{
				if (MCS15 && (Rssi >= (-70+RssiOffset)))
					TxRateIdx = MCS15;
				else if (MCS14 && (Rssi >= (-72+RssiOffset)))
					TxRateIdx = MCS14;
				else if (MCS13 && (Rssi >= (-76+RssiOffset)))
					TxRateIdx = MCS13;
				else if (MCS12 && (Rssi >= (-78+RssiOffset)))
					TxRateIdx = MCS12;
				else if (MCS4 && (Rssi >= (-82+RssiOffset)))
					TxRateIdx = MCS4;
				else if (MCS3 && (Rssi >= (-84+RssiOffset)))
					TxRateIdx = MCS3;
				else if (MCS2 && (Rssi >= (-86+RssiOffset)))
					TxRateIdx = MCS2;
				else if (MCS1 && (Rssi >= (-88+RssiOffset)))
					TxRateIdx = MCS1;
				else
					TxRateIdx = MCS0;
			}
			else if ((pTable == RateSwitchTable11BGN1S) || (pTable == RateSwitchTable11N1S))
			{
				if (MCS7 && (Rssi > (-72+RssiOffset)))
					TxRateIdx = MCS7;
				else if (MCS6 && (Rssi > (-74+RssiOffset)))
					TxRateIdx = MCS6;
				else if (MCS5 && (Rssi > (-77+RssiOffset)))
					TxRateIdx = MCS5;
				else if (MCS4 && (Rssi > (-79+RssiOffset)))
					TxRateIdx = MCS4;
				else if (MCS3 && (Rssi > (-81+RssiOffset)))
					TxRateIdx = MCS3;
				else if (MCS2 && (Rssi > (-83+RssiOffset)))
					TxRateIdx = MCS2;
				else if (MCS1 && (Rssi > (-86+RssiOffset)))
					TxRateIdx = MCS1;
				else
					TxRateIdx = MCS0;
			}
			else
			{
				if (MCS7 && (Rssi > -70))
					TxRateIdx = MCS7;
				else if (MCS6 && (Rssi > -74))
					TxRateIdx = MCS6;
				else if (MCS5 && (Rssi > -78))
					TxRateIdx = MCS5;
				else if (MCS4 && (Rssi > -82))
					TxRateIdx = MCS4;
				else if (MCS4 == 0)	
					TxRateIdx = MCS3;
				else if (MCS3 && (Rssi > -85))
					TxRateIdx = MCS3;
				else if (MCS2 && (Rssi > -87))
					TxRateIdx = MCS2;
				else if (MCS1 && (Rssi > -90))
					TxRateIdx = MCS1;
				else
					TxRateIdx = MCS0;
			}

			{
				pEntry->CurrTxRateIndex = TxRateIdx;
				pNextTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(pEntry->CurrTxRateIndex+1)*5];
				MlmeSetTxRate(pAd, pEntry, pNextTxRate);
			}

			NdisZeroMemory(pEntry->TxQuality, sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH);
			NdisZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);
			pEntry->fLastSecAccordingRSSI = TRUE;
			
			RESET_ONE_SEC_TX_CNT(pEntry);

			continue;
		}

		if (pEntry->fLastSecAccordingRSSI == TRUE)
		{
			pEntry->fLastSecAccordingRSSI = FALSE;
			pEntry->LastSecTxRateChangeAction = 0;
			
			RESET_ONE_SEC_TX_CNT(pEntry);

			continue;
		}

		do
		{
			BOOLEAN	bTrainUpDown = FALSE;

			pEntry->CurrTxRateStableTime ++;

			
			if (TxErrorRatio >= TrainDown)
			{
				bTrainUpDown = TRUE;
				pEntry->TxQuality[CurrRateIdx] = DRS_TX_QUALITY_WORST_BOUND;
			}
			
			else if (TxErrorRatio <= TrainUp)
			{
				bTrainUpDown = TRUE;
				bUpgradeQuality = TRUE;
				if (pEntry->TxQuality[CurrRateIdx])
					pEntry->TxQuality[CurrRateIdx] --;  

				if (pEntry->TxRateUpPenalty)
					pEntry->TxRateUpPenalty --;
				else if (pEntry->TxQuality[UpRateIdx])
					pEntry->TxQuality[UpRateIdx] --;    
			}

			pEntry->PER[CurrRateIdx] = (UCHAR)TxErrorRatio;

			if (bTrainUpDown)
			{
				
				if ((CurrRateIdx != DownRateIdx) && (pEntry->TxQuality[CurrRateIdx] >= DRS_TX_QUALITY_WORST_BOUND))
				{
					pEntry->CurrTxRateIndex = DownRateIdx;
				}
				else if ((CurrRateIdx != UpRateIdx) && (pEntry->TxQuality[UpRateIdx] <= 0))
				{
					pEntry->CurrTxRateIndex = UpRateIdx;
				}
			}
		} while (FALSE);

		
		if (pEntry->CurrTxRateIndex > CurrRateIdx)
		{
			pEntry->CurrTxRateStableTime = 0;
			pEntry->TxRateUpPenalty = 0;
			pEntry->LastSecTxRateChangeAction = 1; 
			NdisZeroMemory(pEntry->TxQuality, sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH);
			NdisZeroMemory(pEntry->PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);

			
			
			
			if (!pAd->StaCfg.StaQuickResponeForRateUpTimerRunning)
			{
				RTMPSetTimer(&pAd->StaCfg.StaQuickResponeForRateUpTimer, 100);

				pAd->StaCfg.StaQuickResponeForRateUpTimerRunning = TRUE;
			}
			bTxRateChanged = TRUE;
		}
		
		else if (pEntry->CurrTxRateIndex < CurrRateIdx)
		{
			pEntry->CurrTxRateStableTime = 0;
			pEntry->TxRateUpPenalty = 0;           
			pEntry->LastSecTxRateChangeAction = 2; 
			pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0;
			pEntry->PER[pEntry->CurrTxRateIndex] = 0;

			
			
			
			if (!pAd->StaCfg.StaQuickResponeForRateUpTimerRunning)
			{
				RTMPSetTimer(&pAd->StaCfg.StaQuickResponeForRateUpTimer, 100);

				pAd->StaCfg.StaQuickResponeForRateUpTimerRunning = TRUE;
			}
			bTxRateChanged = TRUE;
		}
		else
		{
			pEntry->LastSecTxRateChangeAction = 0; 
			bTxRateChanged = FALSE;
		}

		pEntry->LastTxOkCount = TxSuccess;

		
		RESET_ONE_SEC_TX_CNT(pEntry);

		pNextTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(pEntry->CurrTxRateIndex+1)*5];
		if (bTxRateChanged && pNextTxRate)
		{
			MlmeSetTxRate(pAd, pEntry, pNextTxRate);
		}
	}
}


VOID StaQuickResponeForRateUpExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER			pAd = (PRTMP_ADAPTER)FunctionContext;
	UCHAR					UpRateIdx = 0, DownRateIdx = 0, CurrRateIdx = 0;
	ULONG					TxTotalCnt;
	ULONG					TxErrorRatio = 0;
#ifdef RT2860
	BOOLEAN					bTxRateChanged = TRUE; 
#endif
#ifdef RT2870
	BOOLEAN					bTxRateChanged; 
#endif
	PRTMP_TX_RATE_SWITCH	pCurrTxRate, pNextTxRate = NULL;
	PUCHAR					pTable;
	UCHAR					TableSize = 0;
	UCHAR					InitTxRateIdx = 0, TrainUp, TrainDown;
	TX_STA_CNT1_STRUC		StaTx1;
	TX_STA_CNT0_STRUC		TxStaCnt0;
	CHAR					Rssi, ratio;
	ULONG					TxRetransmit = 0, TxSuccess = 0, TxFailCount = 0;
	MAC_TABLE_ENTRY			*pEntry;
	ULONG					i;

	pAd->StaCfg.StaQuickResponeForRateUpTimerRunning = FALSE;

    
    
    
	for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		pEntry = &pAd->MacTab.Content[i];

		
		if (RTMPCheckEntryEnableAutoRateSwitch(pAd, pEntry) == FALSE)
			continue;

#ifdef RT2860
		
	    if (pAd->Antenna.field.TxPath > 1)
			Rssi = (pAd->StaCfg.RssiSample.AvgRssi0 + pAd->StaCfg.RssiSample.AvgRssi1) >> 1;
		else
			Rssi = pAd->StaCfg.RssiSample.AvgRssi0;
#endif
#ifdef RT2870
		if (INFRA_ON(pAd) && (i == 1))
			Rssi = RTMPMaxRssi(pAd,
							   pAd->StaCfg.RssiSample.AvgRssi0,
							   pAd->StaCfg.RssiSample.AvgRssi1,
							   pAd->StaCfg.RssiSample.AvgRssi2);
		else
			Rssi = RTMPMaxRssi(pAd,
							   pEntry->RssiSample.AvgRssi0,
							   pEntry->RssiSample.AvgRssi1,
							   pEntry->RssiSample.AvgRssi2);
#endif

		CurrRateIdx = pAd->CommonCfg.TxRateIndex;

			MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);

		
		if ((CurrRateIdx > 0) && (CurrRateIdx < (TableSize - 1)))
		{
			UpRateIdx = CurrRateIdx + 1;
			DownRateIdx = CurrRateIdx -1;
		}
		else if (CurrRateIdx == 0)
		{
			UpRateIdx = CurrRateIdx + 1;
			DownRateIdx = CurrRateIdx;
		}
		else if (CurrRateIdx == (TableSize - 1))
		{
			UpRateIdx = CurrRateIdx;
			DownRateIdx = CurrRateIdx - 1;
		}

		pCurrTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(CurrRateIdx+1)*5];

		if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
		{
			TrainUp		= (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
			TrainDown	= (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
		}
		else
		{
			TrainUp		= pCurrTxRate->TrainUp;
			TrainDown	= pCurrTxRate->TrainDown;
		}

		if (pAd->MacTab.Size == 1)
		{
			
			RTMP_IO_READ32(pAd, TX_STA_CNT0, &TxStaCnt0.word);
			RTMP_IO_READ32(pAd, TX_STA_CNT1, &StaTx1.word);

			TxRetransmit = StaTx1.field.TxRetransmit;
			TxSuccess = StaTx1.field.TxSuccess;
			TxFailCount = TxStaCnt0.field.TxFailCount;
			TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;

			pAd->RalinkCounters.OneSecTxRetryOkCount += StaTx1.field.TxRetransmit;
			pAd->RalinkCounters.OneSecTxNoRetryOkCount += StaTx1.field.TxSuccess;
			pAd->RalinkCounters.OneSecTxFailCount += TxStaCnt0.field.TxFailCount;
			pAd->WlanCounters.TransmittedFragmentCount.u.LowPart += StaTx1.field.TxSuccess;
			pAd->WlanCounters.RetryCount.u.LowPart += StaTx1.field.TxRetransmit;
			pAd->WlanCounters.FailedCount.u.LowPart += TxStaCnt0.field.TxFailCount;

			if (TxTotalCnt)
				TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;
		}
		else
		{
			TxTotalCnt = pEntry->OneSecTxNoRetryOkCount +
				 pEntry->OneSecTxRetryOkCount +
				 pEntry->OneSecTxFailCount;

			if (TxTotalCnt)
				TxErrorRatio = ((pEntry->OneSecTxRetryOkCount + pEntry->OneSecTxFailCount) * 100) / TxTotalCnt;
		}


		
		
		
		
		if (TxTotalCnt <= 12)
		{
			NdisZeroMemory(pAd->DrsCounters.TxQuality, sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH);
			NdisZeroMemory(pAd->DrsCounters.PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);

			if ((pAd->DrsCounters.LastSecTxRateChangeAction == 1) && (CurrRateIdx != DownRateIdx))
			{
				pAd->CommonCfg.TxRateIndex = DownRateIdx;
				pAd->DrsCounters.TxQuality[CurrRateIdx] = DRS_TX_QUALITY_WORST_BOUND;
			}
			else if ((pAd->DrsCounters.LastSecTxRateChangeAction == 2) && (CurrRateIdx != UpRateIdx))
			{
				pAd->CommonCfg.TxRateIndex = UpRateIdx;
			}

			DBGPRINT_RAW(RT_DEBUG_TRACE,("QuickDRS: TxTotalCnt <= 15, train back to original rate \n"));
			return;
		}

		do
		{
			ULONG OneSecTxNoRetryOKRationCount;

			if (pAd->DrsCounters.LastTimeTxRateChangeAction == 0)
				ratio = 5;
			else
				ratio = 4;

			
			if (TxErrorRatio >= TrainDown)
			{
				pAd->DrsCounters.TxQuality[CurrRateIdx] = DRS_TX_QUALITY_WORST_BOUND;
			}

			pAd->DrsCounters.PER[CurrRateIdx] = (UCHAR)TxErrorRatio;

			OneSecTxNoRetryOKRationCount = (TxSuccess * ratio);

			
			if ((pAd->DrsCounters.LastSecTxRateChangeAction == 1) && (CurrRateIdx != DownRateIdx))
			{
				if ((pAd->DrsCounters.LastTxOkCount + 2) >= OneSecTxNoRetryOKRationCount)
				{
					pAd->CommonCfg.TxRateIndex = DownRateIdx;
					pAd->DrsCounters.TxQuality[CurrRateIdx] = DRS_TX_QUALITY_WORST_BOUND;

				}

			}
			else if ((pAd->DrsCounters.LastSecTxRateChangeAction == 2) && (CurrRateIdx != UpRateIdx))
			{
				if ((TxErrorRatio >= 50) || (TxErrorRatio >= TrainDown))
				{

				}
				else if ((pAd->DrsCounters.LastTxOkCount + 2) >= OneSecTxNoRetryOKRationCount)
				{
					pAd->CommonCfg.TxRateIndex = UpRateIdx;
				}
			}
		}while (FALSE);

		
		if (pAd->CommonCfg.TxRateIndex > CurrRateIdx)
		{
			pAd->DrsCounters.TxRateUpPenalty = 0;
			NdisZeroMemory(pAd->DrsCounters.TxQuality, sizeof(USHORT) * MAX_STEP_OF_TX_RATE_SWITCH);
			NdisZeroMemory(pAd->DrsCounters.PER, sizeof(UCHAR) * MAX_STEP_OF_TX_RATE_SWITCH);
#ifdef RT2870
			bTxRateChanged = TRUE;
#endif
		}
		
		else if (pAd->CommonCfg.TxRateIndex < CurrRateIdx)
		{
			DBGPRINT_RAW(RT_DEBUG_TRACE,("QuickDRS: --TX rate from %d to %d \n", CurrRateIdx, pAd->CommonCfg.TxRateIndex));

			pAd->DrsCounters.TxRateUpPenalty = 0;           
			pAd->DrsCounters.TxQuality[pAd->CommonCfg.TxRateIndex] = 0;
			pAd->DrsCounters.PER[pAd->CommonCfg.TxRateIndex] = 0;
#ifdef RT2870
			bTxRateChanged = TRUE;
#endif
		}
		else
		{
			bTxRateChanged = FALSE;
		}

		pNextTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(pAd->CommonCfg.TxRateIndex+1)*5];
		if (bTxRateChanged && pNextTxRate)
		{
			MlmeSetTxRate(pAd, pEntry, pNextTxRate);
		}
	}
}


VOID MlmeCheckPsmChange(
	IN PRTMP_ADAPTER pAd,
	IN ULONG	Now32)
{
	ULONG	PowerMode;

	
	
	
	
	
	
#ifdef NDIS51_MINIPORT
	if (pAd->StaCfg.WindowsPowerProfile == NdisPowerProfileBattery)
		PowerMode = pAd->StaCfg.WindowsBatteryPowerMode;
	else
#endif
		PowerMode = pAd->StaCfg.WindowsPowerMode;

	if (INFRA_ON(pAd) &&
		(PowerMode != Ndis802_11PowerModeCAM) &&
		(pAd->StaCfg.Psm == PWR_ACTIVE) &&
#ifdef RT2860
		RTMP_TEST_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP))
#else
		(pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE))
#endif
	{
		
		
		
		if (((pAd->StaCfg.HTPhyMode.field.MCS <= 3) &&
				(pAd->RalinkCounters.OneSecRxOkDataCnt < (ULONG)100)) ||
			((pAd->StaCfg.HTPhyMode.field.MCS > 3) &&
			(pAd->RalinkCounters.OneSecRxOkDataCnt < (ULONG)400)))
		{
				
			NdisGetSystemUpTime(&pAd->Mlme.LastSendNULLpsmTime);
			pAd->RalinkCounters.RxCountSinceLastNULL = 0;
			MlmeSetPsmBit(pAd, PWR_SAVE);
			if (!(pAd->CommonCfg.bAPSDCapable && pAd->CommonCfg.APEdcaParm.bAPSDCapable))
			{
				RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate, FALSE);
			}
			else
			{
				RTMPSendNullFrame(pAd, pAd->CommonCfg.TxRate, TRUE);
			}
		}
	}
}



VOID MlmeSetPsmBit(
	IN PRTMP_ADAPTER pAd,
	IN USHORT psm)
{
	AUTO_RSP_CFG_STRUC csr4;

	pAd->StaCfg.Psm = psm;
	RTMP_IO_READ32(pAd, AUTO_RSP_CFG, &csr4.word);
	csr4.field.AckCtsPsmBit = (psm == PWR_SAVE)? 1:0;
	RTMP_IO_WRITE32(pAd, AUTO_RSP_CFG, csr4.word);

	DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetPsmBit = %d\n", psm));
}


VOID MlmeSetTxPreamble(
	IN PRTMP_ADAPTER pAd,
	IN USHORT TxPreamble)
{
	AUTO_RSP_CFG_STRUC csr4;

	
	
	
	
	

	RTMP_IO_READ32(pAd, AUTO_RSP_CFG, &csr4.word);
	if (TxPreamble == Rt802_11PreambleLong)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetTxPreamble (= LONG PREAMBLE)\n"));
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
		csr4.field.AutoResponderPreamble = 0;
	}
	else
	{
		
		DBGPRINT(RT_DEBUG_TRACE, ("MlmeSetTxPreamble (= SHORT PREAMBLE)\n"));
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
		csr4.field.AutoResponderPreamble = 1;
	}

	RTMP_IO_WRITE32(pAd, AUTO_RSP_CFG, csr4.word);
}



VOID UpdateBasicRateBitmap(
    IN  PRTMP_ADAPTER   pAdapter)
{
    INT  i, j;
                  
    UCHAR rate[] = { 2, 4,  11, 22, 12, 18, 24, 36, 48, 72, 96, 108 };
    UCHAR *sup_p = pAdapter->CommonCfg.SupRate;
    UCHAR *ext_p = pAdapter->CommonCfg.ExtRate;
    ULONG bitmap = pAdapter->CommonCfg.BasicRateBitmap;


    
    
	if (pAdapter->CommonCfg.Channel > 14)
        pAdapter->CommonCfg.BasicRateBitmap = 0x150; 
    

    if (pAdapter->CommonCfg.BasicRateBitmap > 4095)
    {
        
        return;
    } 

    for(i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
    {
        sup_p[i] &= 0x7f;
        ext_p[i] &= 0x7f;
    } 

    for(i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
    {
        if (bitmap & (1 << i))
        {
            for(j=0; j<MAX_LEN_OF_SUPPORTED_RATES; j++)
            {
                if (sup_p[j] == rate[i])
                    sup_p[j] |= 0x80;
                
            } 

            for(j=0; j<MAX_LEN_OF_SUPPORTED_RATES; j++)
            {
                if (ext_p[j] == rate[i])
                    ext_p[j] |= 0x80;
                
            } 
        } 
    } 
} 





VOID MlmeUpdateTxRates(
	IN PRTMP_ADAPTER 		pAd,
	IN 	BOOLEAN		 		bLinkUp,
	IN	UCHAR				apidx)
{
	int i, num;
	UCHAR Rate = RATE_6, MaxDesire = RATE_1, MaxSupport = RATE_1;
	UCHAR MinSupport = RATE_54;
	ULONG BasicRateBitmap = 0;
	UCHAR CurrBasicRate = RATE_1;
	UCHAR *pSupRate, SupRateLen, *pExtRate, ExtRateLen;
	PHTTRANSMIT_SETTING		pHtPhy = NULL;
	PHTTRANSMIT_SETTING		pMaxHtPhy = NULL;
	PHTTRANSMIT_SETTING		pMinHtPhy = NULL;
	BOOLEAN 				*auto_rate_cur_p;
	UCHAR					HtMcs = MCS_AUTO;

	
	UpdateBasicRateBitmap(pAd);

	num = 0;
	auto_rate_cur_p = NULL;
	for (i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
	{
		switch (pAd->CommonCfg.DesireRate[i] & 0x7f)
		{
			case 2:  Rate = RATE_1;   num++;   break;
			case 4:  Rate = RATE_2;   num++;   break;
			case 11: Rate = RATE_5_5; num++;   break;
			case 22: Rate = RATE_11;  num++;   break;
			case 12: Rate = RATE_6;   num++;   break;
			case 18: Rate = RATE_9;   num++;   break;
			case 24: Rate = RATE_12;  num++;   break;
			case 36: Rate = RATE_18;  num++;   break;
			case 48: Rate = RATE_24;  num++;   break;
			case 72: Rate = RATE_36;  num++;   break;
			case 96: Rate = RATE_48;  num++;   break;
			case 108: Rate = RATE_54; num++;   break;
			
		}
		if (MaxDesire < Rate)  MaxDesire = Rate;
	}



	{
		pHtPhy 		= &pAd->StaCfg.HTPhyMode;
		pMaxHtPhy	= &pAd->StaCfg.MaxHTPhyMode;
		pMinHtPhy	= &pAd->StaCfg.MinHTPhyMode;

		auto_rate_cur_p = &pAd->StaCfg.bAutoTxRateSwitch;
		HtMcs 		= pAd->StaCfg.DesiredTransmitSetting.field.MCS;

		if ((pAd->StaCfg.BssType == BSS_ADHOC) &&
			(pAd->CommonCfg.PhyMode == PHY_11B) &&
			(MaxDesire > RATE_11))
		{
			MaxDesire = RATE_11;
		}
	}

	pAd->CommonCfg.MaxDesiredRate = MaxDesire;
	pMinHtPhy->word = 0;
	pMaxHtPhy->word = 0;
	pHtPhy->word = 0;

	
	
	if (num <= 1)
	{
		*auto_rate_cur_p = FALSE;
	}
	else
	{
		*auto_rate_cur_p = TRUE;
	}

#if 1
	if (HtMcs != MCS_AUTO)
	{
		*auto_rate_cur_p = FALSE;
	}
	else
	{
		*auto_rate_cur_p = TRUE;
	}
#endif

	if ((ADHOC_ON(pAd) || INFRA_ON(pAd)) && (pAd->OpMode == OPMODE_STA))
	{
		pSupRate = &pAd->StaActive.SupRate[0];
		pExtRate = &pAd->StaActive.ExtRate[0];
		SupRateLen = pAd->StaActive.SupRateLen;
		ExtRateLen = pAd->StaActive.ExtRateLen;
	}
	else
	{
		pSupRate = &pAd->CommonCfg.SupRate[0];
		pExtRate = &pAd->CommonCfg.ExtRate[0];
		SupRateLen = pAd->CommonCfg.SupRateLen;
		ExtRateLen = pAd->CommonCfg.ExtRateLen;
	}

	
	for (i=0; i<SupRateLen; i++)
	{
		switch (pSupRate[i] & 0x7f)
		{
			case 2:   Rate = RATE_1;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0001;	 break;
			case 4:   Rate = RATE_2;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0002;	 break;
			case 11:  Rate = RATE_5_5;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0004;	 break;
			case 22:  Rate = RATE_11;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0008;	 break;
			case 12:  Rate = RATE_6;	  BasicRateBitmap |= 0x0010;  break;
			case 18:  Rate = RATE_9;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0020;	 break;
			case 24:  Rate = RATE_12;	  BasicRateBitmap |= 0x0040;  break;
			case 36:  Rate = RATE_18;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0080;	 break;
			case 48:  Rate = RATE_24;	  BasicRateBitmap |= 0x0100;  break;
			case 72:  Rate = RATE_36;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0200;	 break;
			case 96:  Rate = RATE_48;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0400;	 break;
			case 108: Rate = RATE_54;	if (pSupRate[i] & 0x80) BasicRateBitmap |= 0x0800;	 break;
			default:  Rate = RATE_1;	break;
		}
		if (MaxSupport < Rate)	MaxSupport = Rate;

		if (MinSupport > Rate) MinSupport = Rate;
	}

	for (i=0; i<ExtRateLen; i++)
	{
		switch (pExtRate[i] & 0x7f)
		{
			case 2:   Rate = RATE_1;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0001;	 break;
			case 4:   Rate = RATE_2;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0002;	 break;
			case 11:  Rate = RATE_5_5;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0004;	 break;
			case 22:  Rate = RATE_11;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0008;	 break;
			case 12:  Rate = RATE_6;	  BasicRateBitmap |= 0x0010;  break;
			case 18:  Rate = RATE_9;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0020;	 break;
			case 24:  Rate = RATE_12;	  BasicRateBitmap |= 0x0040;  break;
			case 36:  Rate = RATE_18;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0080;	 break;
			case 48:  Rate = RATE_24;	  BasicRateBitmap |= 0x0100;  break;
			case 72:  Rate = RATE_36;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0200;	 break;
			case 96:  Rate = RATE_48;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0400;	 break;
			case 108: Rate = RATE_54;	if (pExtRate[i] & 0x80) BasicRateBitmap |= 0x0800;	 break;
			default:  Rate = RATE_1;	break;
		}
		if (MaxSupport < Rate)	MaxSupport = Rate;

		if (MinSupport > Rate) MinSupport = Rate;
	}

	RTMP_IO_WRITE32(pAd, LEGACY_BASIC_RATE, BasicRateBitmap);

	
	
	for (i=0; i<MAX_LEN_OF_SUPPORTED_RATES; i++)
	{
		if (BasicRateBitmap & (0x01 << i))
			CurrBasicRate = (UCHAR)i;
		pAd->CommonCfg.ExpectedACKRate[i] = CurrBasicRate;
	}

	DBGPRINT(RT_DEBUG_TRACE,("MlmeUpdateTxRates[MaxSupport = %d] = MaxDesire %d Mbps\n", RateIdToMbps[MaxSupport], RateIdToMbps[MaxDesire]));
	
	if (MaxSupport < MaxDesire)
		pAd->CommonCfg.MaxTxRate = MaxSupport;
	else
		pAd->CommonCfg.MaxTxRate = MaxDesire;

	pAd->CommonCfg.MinTxRate = MinSupport;
	if (*auto_rate_cur_p)
	{
		short dbm = 0;

		dbm = pAd->StaCfg.RssiSample.AvgRssi0 - pAd->BbpRssiToDbmDelta;

		if (bLinkUp == TRUE)
			pAd->CommonCfg.TxRate = RATE_24;
		else
			pAd->CommonCfg.TxRate = pAd->CommonCfg.MaxTxRate;

		if (dbm < -75)
			pAd->CommonCfg.TxRate = RATE_11;
		else if (dbm < -70)
			pAd->CommonCfg.TxRate = RATE_24;

		
		if (pAd->CommonCfg.TxRate > pAd->CommonCfg.MaxTxRate)
			pAd->CommonCfg.TxRate = pAd->CommonCfg.MaxTxRate;

		pAd->CommonCfg.TxRateIndex = 0;
	}
	else
	{
		pAd->CommonCfg.TxRate = pAd->CommonCfg.MaxTxRate;
		pHtPhy->field.MCS	= (pAd->CommonCfg.MaxTxRate > 3) ? (pAd->CommonCfg.MaxTxRate - 4) : pAd->CommonCfg.MaxTxRate;
		pHtPhy->field.MODE	= (pAd->CommonCfg.MaxTxRate > 3) ? MODE_OFDM : MODE_CCK;

		pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.STBC	= pHtPhy->field.STBC;
		pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.ShortGI	= pHtPhy->field.ShortGI;
		pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MCS		= pHtPhy->field.MCS;
		pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE	= pHtPhy->field.MODE;
	}

	if (pAd->CommonCfg.TxRate <= RATE_11)
	{
		pMaxHtPhy->field.MODE = MODE_CCK;
		pMaxHtPhy->field.MCS = pAd->CommonCfg.TxRate;
		pMinHtPhy->field.MCS = pAd->CommonCfg.MinTxRate;
	}
	else
	{
		pMaxHtPhy->field.MODE = MODE_OFDM;
		pMaxHtPhy->field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.TxRate];
		if (pAd->CommonCfg.MinTxRate >= RATE_6 && (pAd->CommonCfg.MinTxRate <= RATE_54))
			{pMinHtPhy->field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MinTxRate];}
		else
			{pMinHtPhy->field.MCS = pAd->CommonCfg.MinTxRate;}
	}

	pHtPhy->word = (pMaxHtPhy->word);
	if (bLinkUp && (pAd->OpMode == OPMODE_STA))
	{
			pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word = pHtPhy->word;
			pAd->MacTab.Content[BSSID_WCID].MaxHTPhyMode.word = pMaxHtPhy->word;
			pAd->MacTab.Content[BSSID_WCID].MinHTPhyMode.word = pMinHtPhy->word;
	}
	else
	{
		switch (pAd->CommonCfg.PhyMode)
		{
			case PHY_11BG_MIXED:
			case PHY_11B:
			case PHY_11BGN_MIXED:
				pAd->CommonCfg.MlmeRate = RATE_1;
				pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_CCK;
				pAd->CommonCfg.MlmeTransmit.field.MCS = RATE_1;
				pAd->CommonCfg.RtsRate = RATE_11;
				break;
			case PHY_11G:
			case PHY_11A:
			case PHY_11AGN_MIXED:
			case PHY_11GN_MIXED:
			case PHY_11N_2_4G:
			case PHY_11AN_MIXED:
			case PHY_11N_5G:
				pAd->CommonCfg.MlmeRate = RATE_6;
				pAd->CommonCfg.RtsRate = RATE_6;
				pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
				pAd->CommonCfg.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
				break;
			case PHY_11ABG_MIXED:
			case PHY_11ABGN_MIXED:
				if (pAd->CommonCfg.Channel <= 14)
				{
					pAd->CommonCfg.MlmeRate = RATE_1;
					pAd->CommonCfg.RtsRate = RATE_1;
					pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_CCK;
					pAd->CommonCfg.MlmeTransmit.field.MCS = RATE_1;
				}
				else
				{
					pAd->CommonCfg.MlmeRate = RATE_6;
					pAd->CommonCfg.RtsRate = RATE_6;
					pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
					pAd->CommonCfg.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
				}
				break;
			default: 
				pAd->CommonCfg.MlmeRate = RATE_6;
                        	pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
				pAd->CommonCfg.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
				pAd->CommonCfg.RtsRate = RATE_1;
				break;
		}
		
		
		
		pAd->MacTab.Content[MCAST_WCID].HTPhyMode.word = pAd->CommonCfg.MlmeTransmit.word;
		if (pAd->CommonCfg.MlmeTransmit.field.MODE == MODE_OFDM)
			pAd->MacTab.Content[MCAST_WCID].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[RATE_24];
		else
			pAd->MacTab.Content[MCAST_WCID].HTPhyMode.field.MCS = RATE_1;
		pAd->CommonCfg.BasicMlmeRate = pAd->CommonCfg.MlmeRate;
	}

	DBGPRINT(RT_DEBUG_TRACE, (" MlmeUpdateTxRates (MaxDesire=%d, MaxSupport=%d, MaxTxRate=%d, MinRate=%d, Rate Switching =%d)\n",
			 RateIdToMbps[MaxDesire], RateIdToMbps[MaxSupport], RateIdToMbps[pAd->CommonCfg.MaxTxRate], RateIdToMbps[pAd->CommonCfg.MinTxRate],
			 *auto_rate_cur_p));
	DBGPRINT(RT_DEBUG_TRACE, (" MlmeUpdateTxRates (TxRate=%d, RtsRate=%d, BasicRateBitmap=0x%04lx)\n",
			 RateIdToMbps[pAd->CommonCfg.TxRate], RateIdToMbps[pAd->CommonCfg.RtsRate], BasicRateBitmap));
	DBGPRINT(RT_DEBUG_TRACE, ("MlmeUpdateTxRates (MlmeTransmit=0x%x, MinHTPhyMode=%x, MaxHTPhyMode=0x%x, HTPhyMode=0x%x)\n",
			 pAd->CommonCfg.MlmeTransmit.word, pAd->MacTab.Content[BSSID_WCID].MinHTPhyMode.word ,pAd->MacTab.Content[BSSID_WCID].MaxHTPhyMode.word ,pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word ));
}


VOID MlmeUpdateHtTxRates(
	IN PRTMP_ADAPTER 		pAd,
	IN	UCHAR				apidx)
{
	UCHAR	StbcMcs; 
	CHAR 	i; 
	RT_HT_CAPABILITY 	*pRtHtCap = NULL;
	RT_HT_PHY_INFO		*pActiveHtPhy = NULL;
	ULONG		BasicMCS;
	UCHAR j, bitmask;
	PRT_HT_PHY_INFO			pDesireHtPhy = NULL;
	PHTTRANSMIT_SETTING		pHtPhy = NULL;
	PHTTRANSMIT_SETTING		pMaxHtPhy = NULL;
	PHTTRANSMIT_SETTING		pMinHtPhy = NULL;
	BOOLEAN 				*auto_rate_cur_p;

	DBGPRINT(RT_DEBUG_TRACE,("MlmeUpdateHtTxRates===> \n"));

	auto_rate_cur_p = NULL;

	{
		pDesireHtPhy	= &pAd->StaCfg.DesiredHtPhyInfo;
		pActiveHtPhy	= &pAd->StaCfg.DesiredHtPhyInfo;
		pHtPhy 		= &pAd->StaCfg.HTPhyMode;
		pMaxHtPhy	= &pAd->StaCfg.MaxHTPhyMode;
		pMinHtPhy	= &pAd->StaCfg.MinHTPhyMode;

		auto_rate_cur_p = &pAd->StaCfg.bAutoTxRateSwitch;
	}

	if ((ADHOC_ON(pAd) || INFRA_ON(pAd)) && (pAd->OpMode == OPMODE_STA))
	{
		if (pAd->StaActive.SupportedPhyInfo.bHtEnable == FALSE)
			return;

		pRtHtCap = &pAd->StaActive.SupportedHtPhy;
		pActiveHtPhy = &pAd->StaActive.SupportedPhyInfo;
		StbcMcs = (UCHAR)pAd->MlmeAux.AddHtInfo.AddHtInfo3.StbcMcs;
		BasicMCS =pAd->MlmeAux.AddHtInfo.MCSSet[0]+(pAd->MlmeAux.AddHtInfo.MCSSet[1]<<8)+(StbcMcs<<16);
		if ((pAd->CommonCfg.DesiredHtPhy.TxSTBC) && (pRtHtCap->RxSTBC) && (pAd->Antenna.field.TxPath == 2))
			pMaxHtPhy->field.STBC = STBC_USE;
		else
			pMaxHtPhy->field.STBC = STBC_NONE;
	}
	else
	{
		if (pDesireHtPhy->bHtEnable == FALSE)
			return;

		pRtHtCap = &pAd->CommonCfg.DesiredHtPhy;
		StbcMcs = (UCHAR)pAd->CommonCfg.AddHTInfo.AddHtInfo3.StbcMcs;
		BasicMCS = pAd->CommonCfg.AddHTInfo.MCSSet[0]+(pAd->CommonCfg.AddHTInfo.MCSSet[1]<<8)+(StbcMcs<<16);
		if ((pAd->CommonCfg.DesiredHtPhy.TxSTBC) && (pRtHtCap->RxSTBC) && (pAd->Antenna.field.TxPath == 2))
			pMaxHtPhy->field.STBC = STBC_USE;
		else
			pMaxHtPhy->field.STBC = STBC_NONE;
	}

	
	if ((pRtHtCap->GF) && (pAd->CommonCfg.DesiredHtPhy.GF))
		pMaxHtPhy->field.MODE = MODE_HTGREENFIELD;
	else
		pMaxHtPhy->field.MODE = MODE_HTMIX;

    if ((pAd->CommonCfg.DesiredHtPhy.ChannelWidth) && (pRtHtCap->ChannelWidth))
		pMaxHtPhy->field.BW = BW_40;
	else
		pMaxHtPhy->field.BW = BW_20;

    if (pMaxHtPhy->field.BW == BW_20)
		pMaxHtPhy->field.ShortGI = (pAd->CommonCfg.DesiredHtPhy.ShortGIfor20 & pRtHtCap->ShortGIfor20);
	else
		pMaxHtPhy->field.ShortGI = (pAd->CommonCfg.DesiredHtPhy.ShortGIfor40 & pRtHtCap->ShortGIfor40);

	for (i=23; i>=0; i--) 
	{
		j = i/8;
		bitmask = (1<<(i-(j*8)));

		if ((pActiveHtPhy->MCSSet[j] & bitmask) && (pDesireHtPhy->MCSSet[j] & bitmask))
		{
			pMaxHtPhy->field.MCS = i;
			break;
		}

		if (i==0)
			break;
	}

	
	pMinHtPhy->field.BW = BW_20;
	pMinHtPhy->field.MCS = 0;
	pMinHtPhy->field.STBC = 0;
	pMinHtPhy->field.ShortGI = 0;
	
	if ( (pAd->OpMode == OPMODE_STA) && (pDesireHtPhy->MCSSet[0] != 0xff))
	{
		if (pDesireHtPhy->MCSSet[4] != 0)
		{
			pMaxHtPhy->field.MCS = 32;
			pMinHtPhy->field.MCS = 32;
			DBGPRINT(RT_DEBUG_TRACE,("MlmeUpdateHtTxRates<=== Use Fixed MCS = %d\n",pMinHtPhy->field.MCS));
		}

		for (i=23; (CHAR)i >= 0; i--) 
		{
			j = i/8;
			bitmask = (1<<(i-(j*8)));
			if ( (pDesireHtPhy->MCSSet[j] & bitmask) && (pActiveHtPhy->MCSSet[j] & bitmask))
			{
				pMaxHtPhy->field.MCS = i;
				pMinHtPhy->field.MCS = i;
				break;
			}
			if (i==0)
				break;
		}
	}

	
	pHtPhy->field.STBC = pMaxHtPhy->field.STBC;
	pHtPhy->field.BW = pMaxHtPhy->field.BW;
	pHtPhy->field.MODE = pMaxHtPhy->field.MODE;
	pHtPhy->field.MCS = pMaxHtPhy->field.MCS;
	pHtPhy->field.ShortGI = pMaxHtPhy->field.ShortGI;

	
	if (pDesireHtPhy->MCSSet[0] != 0xff)
		*auto_rate_cur_p = FALSE;
	else
		*auto_rate_cur_p = TRUE;

	DBGPRINT(RT_DEBUG_TRACE, (" MlmeUpdateHtTxRates<---.AMsduSize = %d  \n", pAd->CommonCfg.DesiredHtPhy.AmsduSize ));
	DBGPRINT(RT_DEBUG_TRACE,("TX: MCS[0] = %x (choose %d), BW = %d, ShortGI = %d, MODE = %d,  \n", pActiveHtPhy->MCSSet[0],pHtPhy->field.MCS,
		pHtPhy->field.BW, pHtPhy->field.ShortGI, pHtPhy->field.MODE));
	DBGPRINT(RT_DEBUG_TRACE,("MlmeUpdateHtTxRates<=== \n"));
}


VOID MlmeRadioOff(
	IN PRTMP_ADAPTER pAd)
{
	RT28XX_MLME_RADIO_OFF(pAd);
}


VOID MlmeRadioOn(
	IN PRTMP_ADAPTER pAd)
{
	RT28XX_MLME_RADIO_ON(pAd);
}







VOID BssTableInit(
	IN BSS_TABLE *Tab)
{
	int i;

	Tab->BssNr = 0;
    Tab->BssOverlapNr = 0;
	for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++)
	{
		NdisZeroMemory(&Tab->BssEntry[i], sizeof(BSS_ENTRY));
		Tab->BssEntry[i].Rssi = -127;	
	}
}

VOID BATableInit(
	IN PRTMP_ADAPTER pAd,
    IN BA_TABLE *Tab)
{
	int i;

	Tab->numAsOriginator = 0;
	Tab->numAsRecipient = 0;
	NdisAllocateSpinLock(&pAd->BATabLock);
	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++)
	{
		Tab->BARecEntry[i].REC_BA_Status = Recipient_NONE;
		NdisAllocateSpinLock(&(Tab->BARecEntry[i].RxReRingLock));
	}
	for (i = 0; i < MAX_LEN_OF_BA_ORI_TABLE; i++)
	{
		Tab->BAOriEntry[i].ORI_BA_Status = Originator_NONE;
	}
}


ULONG BssTableSearch(
	IN BSS_TABLE *Tab,
	IN PUCHAR	 pBssid,
	IN UCHAR	 Channel)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr; i++)
	{
		
		
		
		
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			 ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid))
		{
			return i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}

ULONG BssSsidTableSearch(
	IN BSS_TABLE *Tab,
	IN PUCHAR	 pBssid,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr; i++)
	{
		
		
		
		
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			 ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid) &&
			SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen))
		{
			return i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}

ULONG BssTableSearchWithSSID(
	IN BSS_TABLE *Tab,
	IN PUCHAR	 Bssid,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr; i++)
	{
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(&(Tab->BssEntry[i].Bssid), Bssid) &&
			(SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen) ||
			(NdisEqualMemory(pSsid, ZeroSsid, SsidLen)) ||
			(NdisEqualMemory(Tab->BssEntry[i].Ssid, ZeroSsid, Tab->BssEntry[i].SsidLen))))
		{
			return i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}


VOID BssTableDeleteEntry(
	IN OUT	BSS_TABLE *Tab,
	IN		PUCHAR	  pBssid,
	IN		UCHAR	  Channel)
{
	UCHAR i, j;

	for (i = 0; i < Tab->BssNr; i++)
	{
		if ((Tab->BssEntry[i].Channel == Channel) &&
			(MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid)))
		{
			for (j = i; j < Tab->BssNr - 1; j++)
			{
				NdisMoveMemory(&(Tab->BssEntry[j]), &(Tab->BssEntry[j + 1]), sizeof(BSS_ENTRY));
			}
			NdisZeroMemory(&(Tab->BssEntry[Tab->BssNr - 1]), sizeof(BSS_ENTRY));
			Tab->BssNr -= 1;
			return;
		}
	}
}


VOID BATableDeleteORIEntry(
	IN OUT	PRTMP_ADAPTER pAd,
	IN		BA_ORI_ENTRY	*pBAORIEntry)
{

	if (pBAORIEntry->ORI_BA_Status != Originator_NONE)
	{
		NdisAcquireSpinLock(&pAd->BATabLock);
		if (pBAORIEntry->ORI_BA_Status == Originator_Done)
		{
			pAd->BATable.numAsOriginator -= 1;
			DBGPRINT(RT_DEBUG_TRACE, ("BATableDeleteORIEntry numAsOriginator= %ld\n", pAd->BATable.numAsRecipient));
			
		}
		pAd->MacTab.Content[pBAORIEntry->Wcid].TXBAbitmap &= (~(1<<(pBAORIEntry->TID) ));	
		pAd->MacTab.Content[pBAORIEntry->Wcid].BAOriWcidArray[pBAORIEntry->TID] = 0;	
		pBAORIEntry->ORI_BA_Status = Originator_NONE;
		pBAORIEntry->Token = 1;
		
		NdisReleaseSpinLock(&pAd->BATabLock);
	}
}


VOID BssEntrySet(
	IN PRTMP_ADAPTER	pAd,
	OUT BSS_ENTRY *pBss,
	IN PUCHAR pBssid,
	IN CHAR Ssid[],
	IN UCHAR SsidLen,
	IN UCHAR BssType,
	IN USHORT BeaconPeriod,
	IN PCF_PARM pCfParm,
	IN USHORT AtimWin,
	IN USHORT CapabilityInfo,
	IN UCHAR SupRate[],
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN ADD_HT_INFO_IE *pAddHtInfo,	
	IN UCHAR			HtCapabilityLen,
	IN UCHAR			AddHtInfoLen,
	IN UCHAR			NewExtChanOffset,
	IN UCHAR Channel,
	IN CHAR Rssi,
	IN LARGE_INTEGER TimeStamp,
	IN UCHAR CkipFlag,
	IN PEDCA_PARM pEdcaParm,
	IN PQOS_CAPABILITY_PARM pQosCapability,
	IN PQBSS_LOAD_PARM pQbssLoad,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE)
{
	COPY_MAC_ADDR(pBss->Bssid, pBssid);
	
	pBss->Hidden = 1;
	if (SsidLen > 0)
	{
		
		
		
		
		if (NdisEqualMemory(Ssid, ZeroSsid, SsidLen) == 0)
		{
		    NdisZeroMemory(pBss->Ssid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pBss->Ssid, Ssid, SsidLen);
			pBss->SsidLen = SsidLen;
			pBss->Hidden = 0;
		}
	}
	else
		pBss->SsidLen = 0;
	pBss->BssType = BssType;
	pBss->BeaconPeriod = BeaconPeriod;
	if (BssType == BSS_INFRA)
	{
		if (pCfParm->bValid)
		{
			pBss->CfpCount = pCfParm->CfpCount;
			pBss->CfpPeriod = pCfParm->CfpPeriod;
			pBss->CfpMaxDuration = pCfParm->CfpMaxDuration;
			pBss->CfpDurRemaining = pCfParm->CfpDurRemaining;
		}
	}
	else
	{
		pBss->AtimWin = AtimWin;
	}

	pBss->CapabilityInfo = CapabilityInfo;
	
	
	pBss->Privacy = CAP_IS_PRIVACY_ON(pBss->CapabilityInfo);
	ASSERT(SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES);
	if (SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES)
		NdisMoveMemory(pBss->SupRate, SupRate, SupRateLen);
	else
		NdisMoveMemory(pBss->SupRate, SupRate, MAX_LEN_OF_SUPPORTED_RATES);
	pBss->SupRateLen = SupRateLen;
	ASSERT(ExtRateLen <= MAX_LEN_OF_SUPPORTED_RATES);
	NdisMoveMemory(pBss->ExtRate, ExtRate, ExtRateLen);
	NdisMoveMemory(&pBss->HtCapability, pHtCapability, HtCapabilityLen);
	NdisMoveMemory(&pBss->AddHtInfo, pAddHtInfo, AddHtInfoLen);
	pBss->NewExtChanOffset = NewExtChanOffset;
	pBss->ExtRateLen = ExtRateLen;
	pBss->Channel = Channel;
	pBss->CentralChannel = Channel;
	pBss->Rssi = Rssi;
	
	pBss->CkipFlag = CkipFlag;

	
	NdisMoveMemory(pBss->FixIEs.Timestamp, &TimeStamp, 8);
	pBss->FixIEs.BeaconInterval = BeaconPeriod;
	pBss->FixIEs.Capabilities = CapabilityInfo;

	
	if (LengthVIE != 0)
	{
		pBss->VarIELen = LengthVIE;
		NdisMoveMemory(pBss->VarIEs, pVIE, pBss->VarIELen);
	}
	else
	{
		pBss->VarIELen = 0;
	}

	pBss->AddHtInfoLen = 0;
	pBss->HtCapabilityLen = 0;

	if (HtCapabilityLen> 0)
	{
		pBss->HtCapabilityLen = HtCapabilityLen;
		NdisMoveMemory(&pBss->HtCapability, pHtCapability, HtCapabilityLen);
		if (AddHtInfoLen > 0)
		{
			pBss->AddHtInfoLen = AddHtInfoLen;
			NdisMoveMemory(&pBss->AddHtInfo, pAddHtInfo, AddHtInfoLen);

	 			if ((pAddHtInfo->ControlChan > 2)&& (pAddHtInfo->AddHtInfo.ExtChanOffset == EXTCHA_BELOW) && (pHtCapability->HtCapInfo.ChannelWidth == BW_40))
	 			{
	 				pBss->CentralChannel = pAddHtInfo->ControlChan - 2;
	 			}
	 			else if ((pAddHtInfo->AddHtInfo.ExtChanOffset == EXTCHA_ABOVE) && (pHtCapability->HtCapInfo.ChannelWidth == BW_40))
				{
		 				pBss->CentralChannel = pAddHtInfo->ControlChan + 2;
				}
		}
	}

	BssCipherParse(pBss);

	
	if (pEdcaParm)
		NdisMoveMemory(&pBss->EdcaParm, pEdcaParm, sizeof(EDCA_PARM));
	else
		pBss->EdcaParm.bValid = FALSE;
	if (pQosCapability)
		NdisMoveMemory(&pBss->QosCapability, pQosCapability, sizeof(QOS_CAPABILITY_PARM));
	else
		pBss->QosCapability.bValid = FALSE;
	if (pQbssLoad)
		NdisMoveMemory(&pBss->QbssLoad, pQbssLoad, sizeof(QBSS_LOAD_PARM));
	else
		pBss->QbssLoad.bValid = FALSE;

	{
		PEID_STRUCT     pEid;
		USHORT          Length = 0;


		NdisZeroMemory(&pBss->WpaIE.IE[0], MAX_CUSTOM_LEN);
		NdisZeroMemory(&pBss->RsnIE.IE[0], MAX_CUSTOM_LEN);

		pEid = (PEID_STRUCT) pVIE;

		while ((Length + 2 + (USHORT)pEid->Len) <= LengthVIE)
		{
			switch(pEid->Eid)
			{
				case IE_WPA:
					if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4))
					{
						if ((pEid->Len + 2) > MAX_CUSTOM_LEN)
						{
							pBss->WpaIE.IELen = 0;
							break;
						}
						pBss->WpaIE.IELen = pEid->Len + 2;
						NdisMoveMemory(pBss->WpaIE.IE, pEid, pBss->WpaIE.IELen);
					}
					break;
                case IE_RSN:
                    if (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3))
					{
						if ((pEid->Len + 2) > MAX_CUSTOM_LEN)
						{
							pBss->RsnIE.IELen = 0;
							break;
						}
						pBss->RsnIE.IELen = pEid->Len + 2;
						NdisMoveMemory(pBss->RsnIE.IE, pEid, pBss->RsnIE.IELen);
			}
				break;
            }
			Length = Length + 2 + (USHORT)pEid->Len;  
			pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);
		}
	}
}


ULONG BssTableSetEntry(
	IN	PRTMP_ADAPTER	pAd,
	OUT BSS_TABLE *Tab,
	IN PUCHAR pBssid,
	IN CHAR Ssid[],
	IN UCHAR SsidLen,
	IN UCHAR BssType,
	IN USHORT BeaconPeriod,
	IN CF_PARM *CfParm,
	IN USHORT AtimWin,
	IN USHORT CapabilityInfo,
	IN UCHAR SupRate[],
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN ADD_HT_INFO_IE *pAddHtInfo,	
	IN UCHAR			HtCapabilityLen,
	IN UCHAR			AddHtInfoLen,
	IN UCHAR			NewExtChanOffset,
	IN UCHAR ChannelNo,
	IN CHAR Rssi,
	IN LARGE_INTEGER TimeStamp,
	IN UCHAR CkipFlag,
	IN PEDCA_PARM pEdcaParm,
	IN PQOS_CAPABILITY_PARM pQosCapability,
	IN PQBSS_LOAD_PARM pQbssLoad,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE)
{
	ULONG	Idx;

	Idx = BssTableSearchWithSSID(Tab, pBssid,  Ssid, SsidLen, ChannelNo);
	if (Idx == BSS_NOT_FOUND)
	{
		if (Tab->BssNr >= MAX_LEN_OF_BSS_TABLE)
	    {
			
			
			
			
			
			if(!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
			{
				if (MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, pBssid) ||
					SSID_EQUAL(pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, Ssid, SsidLen))
				{
					Idx = Tab->BssOverlapNr;
					BssEntrySet(pAd, &Tab->BssEntry[Idx], pBssid, Ssid, SsidLen, BssType, BeaconPeriod, CfParm, AtimWin,
						CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen,pHtCapability, pAddHtInfo,HtCapabilityLen, AddHtInfoLen,
						NewExtChanOffset, ChannelNo, Rssi, TimeStamp, CkipFlag, pEdcaParm, pQosCapability, pQbssLoad, LengthVIE, pVIE);
                    Tab->BssOverlapNr = (Tab->BssOverlapNr++) % MAX_LEN_OF_BSS_TABLE;
				}
				return Idx;
			}
			else
			{
			return BSS_NOT_FOUND;
			}
		}
		Idx = Tab->BssNr;
		BssEntrySet(pAd, &Tab->BssEntry[Idx], pBssid, Ssid, SsidLen, BssType, BeaconPeriod, CfParm, AtimWin,
					CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen,pHtCapability, pAddHtInfo,HtCapabilityLen, AddHtInfoLen,
					NewExtChanOffset, ChannelNo, Rssi, TimeStamp, CkipFlag, pEdcaParm, pQosCapability, pQbssLoad, LengthVIE, pVIE);
		Tab->BssNr++;
	}
	else
	{
		
		if ((SSID_EQUAL(Ssid, SsidLen, Tab->BssEntry[Idx].Ssid, Tab->BssEntry[Idx].SsidLen)) ||
			(NdisEqualMemory(Tab->BssEntry[Idx].Ssid, ZeroSsid, Tab->BssEntry[Idx].SsidLen)))
		{
		BssEntrySet(pAd, &Tab->BssEntry[Idx], pBssid, Ssid, SsidLen, BssType, BeaconPeriod,CfParm, AtimWin,
					CapabilityInfo, SupRate, SupRateLen, ExtRate, ExtRateLen,pHtCapability, pAddHtInfo,HtCapabilityLen, AddHtInfoLen,
					NewExtChanOffset, ChannelNo, Rssi, TimeStamp, CkipFlag, pEdcaParm, pQosCapability, pQbssLoad, LengthVIE, pVIE);
		}
	}

	return Idx;
}


VOID BssTableSsidSort(
	IN	PRTMP_ADAPTER	pAd,
	OUT BSS_TABLE *OutTab,
	IN	CHAR Ssid[],
	IN	UCHAR SsidLen)
{
	INT i;
	BssTableInit(OutTab);

	for (i = 0; i < pAd->ScanTab.BssNr; i++)
	{
		BSS_ENTRY *pInBss = &pAd->ScanTab.BssEntry[i];
		BOOLEAN	bIsHiddenApIncluded = FALSE;

		if (((pAd->CommonCfg.bIEEE80211H == 1) &&
            (pAd->MlmeAux.Channel > 14) &&
             RadarChannelCheck(pAd, pInBss->Channel))
            )
		{
			if (pInBss->Hidden)
				bIsHiddenApIncluded = TRUE;
		}

		if ((pInBss->BssType == pAd->StaCfg.BssType) &&
			(SSID_EQUAL(Ssid, SsidLen, pInBss->Ssid, pInBss->SsidLen) || bIsHiddenApIncluded))
		{
			BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];

			
			if ((pInBss->HtCapabilityLen == 0) &&
				((pAd->CommonCfg.PhyMode == PHY_11N_2_4G) || (pAd->CommonCfg.PhyMode == PHY_11N_5G)))
			{
				DBGPRINT(RT_DEBUG_TRACE,("STA is in N-only Mode, this AP don't have Ht capability in Beacon.\n"));
				continue;
			}

			
			
			if (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
			{
				
				if ((pAd->StaCfg.AuthMode != pInBss->AuthMode) && (pAd->StaCfg.AuthMode != pInBss->AuthModeAux))
					
					continue;

				
				if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK))
				{
					
					if (pInBss->WPA.bMixMode == FALSE)
						if (pAd->StaCfg.WepStatus != pInBss->WPA.GroupCipher)
							continue;

					
					if (pInBss->WPA.GroupCipher != Ndis802_11GroupWEP40Enabled &&
					    pInBss->WPA.GroupCipher != Ndis802_11GroupWEP104Enabled &&
					    pAd->StaCfg.WepStatus < pInBss->WPA.GroupCipher)
						continue;

					
					
					
					if ((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) &&
						(pAd->StaCfg.WepStatus != pInBss->WPA.PairCipher) &&
						(pAd->StaCfg.WepStatus != pInBss->WPA.PairCipherAux))
						continue;
				}
				else if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
				{
					
					if (pInBss->WPA2.bMixMode == FALSE)
						if (pAd->StaCfg.WepStatus != pInBss->WPA2.GroupCipher)
							continue;

					
					if (pInBss->WPA2.GroupCipher != Ndis802_11GroupWEP40Enabled &&
					    pInBss->WPA2.GroupCipher != Ndis802_11GroupWEP104Enabled &&
					    pAd->StaCfg.WepStatus < pInBss->WPA2.GroupCipher)
						continue;

					
					
					
					if ((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) &&
						(pAd->StaCfg.WepStatus != pInBss->WPA2.PairCipher) &&
						(pAd->StaCfg.WepStatus != pInBss->WPA2.PairCipherAux))
						continue;
				}
			}
			
			
			else if (pAd->StaCfg.WepStatus != pInBss->WepStatus)
			{
				DBGPRINT(RT_DEBUG_TRACE,("StaCfg.WepStatus=%d, while pInBss->WepStatus=%d\n", pAd->StaCfg.WepStatus, pInBss->WepStatus));
				
				
				
				if (!pInBss->bSES)
					continue;
			}

			
			
			
			if (SsidLen == 0)
				continue;

			
			
			if ((pInBss->CentralChannel != pInBss->Channel) &&
				(pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40))
			{
				if (RTMPCheckChannel(pAd, pInBss->CentralChannel, pInBss->Channel) == FALSE)
				{
					pAd->CommonCfg.RegTransmitSetting.field.BW = BW_20;
					SetCommonHT(pAd);
					pAd->CommonCfg.RegTransmitSetting.field.BW = BW_40;
				}
				else
				{
					if (pAd->CommonCfg.DesiredHtPhy.ChannelWidth == BAND_WIDTH_20)
					{
						SetCommonHT(pAd);
					}
				}
			}

			
			NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));

			OutTab->BssNr++;
		}
		else if ((pInBss->BssType == pAd->StaCfg.BssType) && (SsidLen == 0))
		{
			BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];

			
			if ((pInBss->HtCapabilityLen == 0) &&
				((pAd->CommonCfg.PhyMode == PHY_11N_2_4G) || (pAd->CommonCfg.PhyMode == PHY_11N_5G)))
			{
				DBGPRINT(RT_DEBUG_TRACE,("STA is in N-only Mode, this AP don't have Ht capability in Beacon.\n"));
				continue;
			}

			
			
			if (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
			{
				
				if ((pAd->StaCfg.AuthMode != pInBss->AuthMode) && (pAd->StaCfg.AuthMode != pInBss->AuthModeAux))
					
					continue;

				
				if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK))
				{
					
					if (pInBss->WPA.bMixMode == FALSE)
						if (pAd->StaCfg.WepStatus != pInBss->WPA.GroupCipher)
							continue;

					
					if (pAd->StaCfg.WepStatus < pInBss->WPA.GroupCipher)
						continue;

					
					
					
					if ((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) &&
						(pAd->StaCfg.WepStatus != pInBss->WPA.PairCipher) &&
						(pAd->StaCfg.WepStatus != pInBss->WPA.PairCipherAux))
						continue;
				}
				else if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK))
				{
					
					if (pInBss->WPA2.bMixMode == FALSE)
						if (pAd->StaCfg.WepStatus != pInBss->WPA2.GroupCipher)
							continue;

					
					if (pAd->StaCfg.WepStatus < pInBss->WPA2.GroupCipher)
						continue;

					
					
					
					if ((pAd->StaCfg.WepStatus == Ndis802_11Encryption2Enabled) &&
						(pAd->StaCfg.WepStatus != pInBss->WPA2.PairCipher) &&
						(pAd->StaCfg.WepStatus != pInBss->WPA2.PairCipherAux))
						continue;
				}
			}
			
			
			else if (pAd->StaCfg.WepStatus != pInBss->WepStatus)
					continue;

			
			
			if ((pInBss->CentralChannel != pInBss->Channel) &&
				(pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40))
			{
				if (RTMPCheckChannel(pAd, pInBss->CentralChannel, pInBss->Channel) == FALSE)
				{
					pAd->CommonCfg.RegTransmitSetting.field.BW = BW_20;
					SetCommonHT(pAd);
					pAd->CommonCfg.RegTransmitSetting.field.BW = BW_40;
				}
			}

			
			NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));

			OutTab->BssNr++;
		}

		if (OutTab->BssNr >= MAX_LEN_OF_BSS_TABLE)
			break;
	}

	BssTableSortByRssi(OutTab);
}



VOID BssTableSortByRssi(
	IN OUT BSS_TABLE *OutTab)
{
	INT 	  i, j;
	BSS_ENTRY TmpBss;

	for (i = 0; i < OutTab->BssNr - 1; i++)
	{
		for (j = i+1; j < OutTab->BssNr; j++)
		{
			if (OutTab->BssEntry[j].Rssi > OutTab->BssEntry[i].Rssi)
			{
				NdisMoveMemory(&TmpBss, &OutTab->BssEntry[j], sizeof(BSS_ENTRY));
				NdisMoveMemory(&OutTab->BssEntry[j], &OutTab->BssEntry[i], sizeof(BSS_ENTRY));
				NdisMoveMemory(&OutTab->BssEntry[i], &TmpBss, sizeof(BSS_ENTRY));
			}
		}
	}
}

VOID BssCipherParse(
	IN OUT	PBSS_ENTRY	pBss)
{
	PEID_STRUCT 		 pEid;
	PUCHAR				pTmp;
	PRSN_IE_HEADER_STRUCT			pRsnHeader;
	PCIPHER_SUITE_STRUCT			pCipher;
	PAKM_SUITE_STRUCT				pAKM;
	USHORT							Count;
	INT								Length;
	NDIS_802_11_ENCRYPTION_STATUS	TmpCipher;

	
	
	
	if (pBss->Privacy)
	{
		pBss->WepStatus 	= Ndis802_11WEPEnabled;
	}
	else
	{
		pBss->WepStatus 	= Ndis802_11WEPDisabled;
	}
	
	pBss->AuthMode		= Ndis802_11AuthModeOpen;
	pBss->AuthModeAux	= Ndis802_11AuthModeOpen;

	
	pBss->WPA.PairCipher	= Ndis802_11WEPDisabled;
	pBss->WPA.PairCipherAux = Ndis802_11WEPDisabled;
	pBss->WPA.GroupCipher	= Ndis802_11WEPDisabled;
	pBss->WPA.RsnCapability = 0;
	pBss->WPA.bMixMode		= FALSE;

	
	pBss->WPA2.PairCipher	 = Ndis802_11WEPDisabled;
	pBss->WPA2.PairCipherAux = Ndis802_11WEPDisabled;
	pBss->WPA2.GroupCipher	 = Ndis802_11WEPDisabled;
	pBss->WPA2.RsnCapability = 0;
	pBss->WPA2.bMixMode 	 = FALSE;


	Length = (INT) pBss->VarIELen;

	while (Length > 0)
	{
		
		pTmp = ((PUCHAR) pBss->VarIEs) + pBss->VarIELen - Length;
		pEid = (PEID_STRUCT) pTmp;
		switch (pEid->Eid)
		{
			case IE_WPA:
				
				if ( NdisEqualMemory((pTmp+8), CISCO_OUI, 3))
				{
					pTmp   += 11;
					switch (*pTmp)
					{
						case 1:
						case 5:	
							pBss->WepStatus = Ndis802_11Encryption1Enabled;
							pBss->WPA.PairCipher = Ndis802_11Encryption1Enabled;
							pBss->WPA.GroupCipher = Ndis802_11Encryption1Enabled;
							break;
						case 2:
							pBss->WepStatus = Ndis802_11Encryption2Enabled;
							pBss->WPA.PairCipher = Ndis802_11Encryption1Enabled;
							pBss->WPA.GroupCipher = Ndis802_11Encryption1Enabled;
							break;
						case 4:
							pBss->WepStatus = Ndis802_11Encryption3Enabled;
							pBss->WPA.PairCipher = Ndis802_11Encryption1Enabled;
							pBss->WPA.GroupCipher = Ndis802_11Encryption1Enabled;
							break;
						default:
							break;
					}

					
					break;
				}
				else if (NdisEqualMemory(pEid->Octet, SES_OUI, 3) && (pEid->Len == 7))
				{
					pBss->bSES = TRUE;
					break;
				}
				else if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4) != 1)
				{
					
					break;
				}
				
				
				
				
				pTmp   += 11;

				
				
				
				
				
				
				
				
				
				switch (*pTmp)
				{
					case 1:
						pBss->WPA.GroupCipher = Ndis802_11GroupWEP40Enabled;
						break;
					case 5:
						pBss->WPA.GroupCipher = Ndis802_11GroupWEP104Enabled;
						break;
					case 2:
						pBss->WPA.GroupCipher = Ndis802_11Encryption2Enabled;
						break;
					case 4:
						pBss->WPA.GroupCipher = Ndis802_11Encryption3Enabled;
						break;
					default:
						break;
				}
				
				pTmp   += 1;

				
				
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);

				
				while (Count > 0)
				{
					
					pTmp += 3;
					TmpCipher = Ndis802_11WEPDisabled;
					switch (*pTmp)
					{
						case 1:
						case 5: 
							TmpCipher = Ndis802_11Encryption1Enabled;
							break;
						case 2:
							TmpCipher = Ndis802_11Encryption2Enabled;
							break;
						case 4:
							TmpCipher = Ndis802_11Encryption3Enabled;
							break;
						default:
							break;
					}
					if (TmpCipher > pBss->WPA.PairCipher)
					{
						
						pBss->WPA.PairCipherAux = pBss->WPA.PairCipher;
						pBss->WPA.PairCipher	= TmpCipher;
					}
					else
					{
						pBss->WPA.PairCipherAux = TmpCipher;
					}
					pTmp++;
					Count--;
				}

				
				
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);
				pTmp   += 3;

				switch (*pTmp)
				{
					case 1:
						
						if (pBss->AuthMode == Ndis802_11AuthModeOpen)
							pBss->AuthMode = Ndis802_11AuthModeWPA;
						else
							pBss->AuthModeAux = Ndis802_11AuthModeWPA;
						break;
					case 2:
						
						if (pBss->AuthMode == Ndis802_11AuthModeOpen)
							pBss->AuthMode = Ndis802_11AuthModeWPAPSK;
						else
							pBss->AuthModeAux = Ndis802_11AuthModeWPAPSK;
						break;
					default:
						break;
				}
				pTmp   += 1;

				
				if (pBss->BssType == BSS_ADHOC)
				{
					pBss->AuthMode	  = Ndis802_11AuthModeWPANone;
					pBss->AuthModeAux = Ndis802_11AuthModeWPANone;
					pBss->WepStatus   = pBss->WPA.GroupCipher;
					if (pBss->WPA.PairCipherAux == Ndis802_11WEPDisabled)
						pBss->WPA.PairCipherAux = pBss->WPA.GroupCipher;
				}
				else
					pBss->WepStatus   = pBss->WPA.PairCipher;

				
				if (pBss->WPA.GroupCipher != pBss->WPA.PairCipher)
					pBss->WPA.bMixMode = TRUE;

				break;

			case IE_RSN:
				pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;

				
				if (le2cpu16(pRsnHeader->Version) != 1)
					break;
				pTmp   += sizeof(RSN_IE_HEADER_STRUCT);

				
				pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
				if (!RTMPEqualMemory(pTmp, RSN_OUI, 3))
					break;

				
				switch (pCipher->Type)
				{
					case 1:
						pBss->WPA2.GroupCipher = Ndis802_11GroupWEP40Enabled;
						break;
					case 5:
						pBss->WPA2.GroupCipher = Ndis802_11GroupWEP104Enabled;
						break;
					case 2:
						pBss->WPA2.GroupCipher = Ndis802_11Encryption2Enabled;
						break;
					case 4:
						pBss->WPA2.GroupCipher = Ndis802_11Encryption3Enabled;
						break;
					default:
						break;
				}
				
				pTmp   += sizeof(CIPHER_SUITE_STRUCT);

				
				
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);

				
				
				while (Count > 0)
				{
					
					pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
					TmpCipher = Ndis802_11WEPDisabled;
					switch (pCipher->Type)
					{
						case 1:
						case 5: 
							TmpCipher = Ndis802_11Encryption1Enabled;
							break;
						case 2:
							TmpCipher = Ndis802_11Encryption2Enabled;
							break;
						case 4:
							TmpCipher = Ndis802_11Encryption3Enabled;
							break;
						default:
							break;
					}
					if (TmpCipher > pBss->WPA2.PairCipher)
					{
						
						pBss->WPA2.PairCipherAux = pBss->WPA2.PairCipher;
						pBss->WPA2.PairCipher	 = TmpCipher;
					}
					else
					{
						pBss->WPA2.PairCipherAux = TmpCipher;
					}
					pTmp += sizeof(CIPHER_SUITE_STRUCT);
					Count--;
				}

				
				
				Count = (pTmp[1]<<8) + pTmp[0];
				pTmp   += sizeof(USHORT);

				
				pAKM = (PAKM_SUITE_STRUCT) pTmp;
				if (!RTMPEqualMemory(pTmp, RSN_OUI, 3))
					break;

				switch (pAKM->Type)
				{
					case 1:
						
						if (pBss->AuthMode == Ndis802_11AuthModeOpen)
							pBss->AuthMode = Ndis802_11AuthModeWPA2;
						else
							pBss->AuthModeAux = Ndis802_11AuthModeWPA2;
						break;
					case 2:
						
						if (pBss->AuthMode == Ndis802_11AuthModeOpen)
							pBss->AuthMode = Ndis802_11AuthModeWPA2PSK;
						else
							pBss->AuthModeAux = Ndis802_11AuthModeWPA2PSK;
						break;
					default:
						break;
				}
				pTmp   += (Count * sizeof(AKM_SUITE_STRUCT));

				
				if (pBss->BssType == BSS_ADHOC)
				{
					pBss->AuthMode = Ndis802_11AuthModeWPANone;
					pBss->AuthModeAux = Ndis802_11AuthModeWPANone;
					pBss->WPA.PairCipherAux = pBss->WPA2.PairCipherAux;
					pBss->WPA.GroupCipher	= pBss->WPA2.GroupCipher;
					pBss->WepStatus 		= pBss->WPA.GroupCipher;
					if (pBss->WPA.PairCipherAux == Ndis802_11WEPDisabled)
						pBss->WPA.PairCipherAux = pBss->WPA.GroupCipher;
				}
				pBss->WepStatus   = pBss->WPA2.PairCipher;

				
				
				pBss->WPA2.RsnCapability = (pTmp[1]<<8) + pTmp[0];
				pTmp += sizeof(USHORT);

				
				if (pBss->WPA2.GroupCipher != pBss->WPA2.PairCipher)
					pBss->WPA2.bMixMode = TRUE;

				break;
			default:
				break;
		}
		Length -= (pEid->Len + 2);
	}
}






VOID MacAddrRandomBssid(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pAddr)
{
	INT i;

	for (i = 0; i < MAC_ADDR_LEN; i++)
	{
		pAddr[i] = RandomByte(pAd);
	}

	pAddr[0] = (pAddr[0] & 0xfe) | 0x02;  
}


VOID MgtMacHeaderInit(
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
	pHdr80211->FC.ToDs = ToDs;
	COPY_MAC_ADDR(pHdr80211->Addr1, pDA);

	COPY_MAC_ADDR(pHdr80211->Addr2, pAd->CurrentAddress);

	COPY_MAC_ADDR(pHdr80211->Addr3, pBssid);
}






ULONG MakeOutgoingFrame(
	OUT CHAR *Buffer,
	OUT ULONG *FrameLen, ...)
{
	CHAR   *p;
	int 	leng;
	ULONG	TotLeng;
	va_list Args;

	
	TotLeng = 0;
	va_start(Args, FrameLen);
	do
	{
		leng = va_arg(Args, int);
		if (leng == END_OF_ARGS)
		{
			break;
		}
		p = va_arg(Args, PVOID);
		NdisMoveMemory(&Buffer[TotLeng], p, leng);
		TotLeng = TotLeng + leng;
	} while(TRUE);

	va_end(Args); 
	*FrameLen = TotLeng;
	return TotLeng;
}






NDIS_STATUS MlmeQueueInit(
	IN MLME_QUEUE *Queue)
{
	INT i;

	NdisAllocateSpinLock(&Queue->Lock);

	Queue->Num	= 0;
	Queue->Head = 0;
	Queue->Tail = 0;

	for (i = 0; i < MAX_LEN_OF_MLME_QUEUE; i++)
	{
		Queue->Entry[i].Occupied = FALSE;
		Queue->Entry[i].MsgLen = 0;
		NdisZeroMemory(Queue->Entry[i].Msg, MGMT_DMA_BUFFER_SIZE);
	}

	return NDIS_STATUS_SUCCESS;
}


BOOLEAN MlmeEnqueue(
	IN	PRTMP_ADAPTER	pAd,
	IN ULONG Machine,
	IN ULONG MsgType,
	IN ULONG MsgLen,
	IN VOID *Msg)
{
	INT Tail;
	MLME_QUEUE	*Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;

	
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return FALSE;

	
	if (MsgLen > MGMT_DMA_BUFFER_SIZE)
	{
		DBGPRINT_ERR(("MlmeEnqueue: msg too large, size = %ld \n", MsgLen));
		return FALSE;
	}

	if (MlmeQueueFull(Queue))
	{
		return FALSE;
	}

	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;
	Queue->Tail++;
	Queue->Num++;
	if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE)
	{
		Queue->Tail = 0;
	}

	Queue->Entry[Tail].Wcid = RESERVED_WCID;
	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen  = MsgLen;

	if (Msg != NULL)
	{
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);
	}

	NdisReleaseSpinLock(&(Queue->Lock));
	return TRUE;
}


BOOLEAN MlmeEnqueueForRecv(
	IN	PRTMP_ADAPTER	pAd,
	IN ULONG Wcid,
	IN ULONG TimeStampHigh,
	IN ULONG TimeStampLow,
	IN UCHAR Rssi0,
	IN UCHAR Rssi1,
	IN UCHAR Rssi2,
	IN ULONG MsgLen,
	IN VOID *Msg,
	IN UCHAR Signal)
{
	INT 		 Tail, Machine;
	PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;
	INT		 MsgType;
	MLME_QUEUE	*Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;

	
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		DBGPRINT_ERR(("MlmeEnqueueForRecv: fRTMP_ADAPTER_HALT_IN_PROGRESS\n"));
		return FALSE;
	}

	
	if (MsgLen > MGMT_DMA_BUFFER_SIZE)
	{
		DBGPRINT_ERR(("MlmeEnqueueForRecv: frame too large, size = %ld \n", MsgLen));
		return FALSE;
	}

	if (MlmeQueueFull(Queue))
	{
		return FALSE;
	}

	{
		if (!MsgTypeSubst(pAd, pFrame, &Machine, &MsgType))
		{
			DBGPRINT_ERR(("MlmeEnqueueForRecv: un-recongnized mgmt->subtype=%d\n",pFrame->Hdr.FC.SubType));
			return FALSE;
		}
	}

	
	NdisAcquireSpinLock(&(Queue->Lock));
	Tail = Queue->Tail;
	Queue->Tail++;
	Queue->Num++;
	if (Queue->Tail == MAX_LEN_OF_MLME_QUEUE)
	{
		Queue->Tail = 0;
	}
	Queue->Entry[Tail].Occupied = TRUE;
	Queue->Entry[Tail].Machine = Machine;
	Queue->Entry[Tail].MsgType = MsgType;
	Queue->Entry[Tail].MsgLen  = MsgLen;
	Queue->Entry[Tail].TimeStamp.u.LowPart = TimeStampLow;
	Queue->Entry[Tail].TimeStamp.u.HighPart = TimeStampHigh;
	Queue->Entry[Tail].Rssi0 = Rssi0;
	Queue->Entry[Tail].Rssi1 = Rssi1;
	Queue->Entry[Tail].Rssi2 = Rssi2;
	Queue->Entry[Tail].Signal = Signal;
	Queue->Entry[Tail].Wcid = (UCHAR)Wcid;

	Queue->Entry[Tail].Channel = pAd->LatchRfRegs.Channel;

	if (Msg != NULL)
	{
		NdisMoveMemory(Queue->Entry[Tail].Msg, Msg, MsgLen);
	}

	NdisReleaseSpinLock(&(Queue->Lock));

	RT28XX_MLME_HANDLER(pAd);

	return TRUE;
}



BOOLEAN MlmeDequeue(
	IN MLME_QUEUE *Queue,
	OUT MLME_QUEUE_ELEM **Elem)
{
	NdisAcquireSpinLock(&(Queue->Lock));
	*Elem = &(Queue->Entry[Queue->Head]);
	Queue->Num--;
	Queue->Head++;
	if (Queue->Head == MAX_LEN_OF_MLME_QUEUE)
	{
		Queue->Head = 0;
	}
	NdisReleaseSpinLock(&(Queue->Lock));
	return TRUE;
}


VOID	MlmeRestartStateMachine(
	IN	PRTMP_ADAPTER	pAd)
{
#ifdef RT2860
	MLME_QUEUE_ELEM		*Elem = NULL;
#endif
	BOOLEAN				Cancelled;

	DBGPRINT(RT_DEBUG_TRACE, ("MlmeRestartStateMachine \n"));

#ifdef RT2860
	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	if(pAd->Mlme.bRunning)
	{
		NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
		return;
	}
	else
	{
		pAd->Mlme.bRunning = TRUE;
	}
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);

	
	while (!MlmeQueueEmpty(&pAd->Mlme.Queue))
	{
		
		if (MlmeDequeue(&pAd->Mlme.Queue, &Elem))
		{
			
			Elem->Occupied = FALSE;
			Elem->MsgLen = 0;

		}
		else {
			DBGPRINT_ERR(("MlmeRestartStateMachine: MlmeQueue empty\n"));
		}
	}
#endif 

	{
		
		
		RTMPCancelTimer(&pAd->MlmeAux.AssocTimer,	  &Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.ReassocTimer,   &Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.DisassocTimer,  &Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.AuthTimer,	   &Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer,	   &Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer,	   &Cancelled);
	}

	
	AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
	AsicLockChannel(pAd, pAd->CommonCfg.Channel);

	
	RTMPResumeMsduTransmission(pAd);

	{
		
		pAd->Mlme.CntlMachine.CurrState    = CNTL_IDLE;
		pAd->Mlme.AssocMachine.CurrState   = ASSOC_IDLE;
		pAd->Mlme.AuthMachine.CurrState    = AUTH_REQ_IDLE;
		pAd->Mlme.AuthRspMachine.CurrState = AUTH_RSP_IDLE;
		pAd->Mlme.SyncMachine.CurrState    = SYNC_IDLE;
		pAd->Mlme.ActMachine.CurrState    = ACT_IDLE;
	}

#ifdef RT2860
	
	NdisAcquireSpinLock(&pAd->Mlme.TaskLock);
	pAd->Mlme.bRunning = FALSE;
	NdisReleaseSpinLock(&pAd->Mlme.TaskLock);
#endif
}


BOOLEAN MlmeQueueEmpty(
	IN MLME_QUEUE *Queue)
{
	BOOLEAN Ans;

	NdisAcquireSpinLock(&(Queue->Lock));
	Ans = (Queue->Num == 0);
	NdisReleaseSpinLock(&(Queue->Lock));

	return Ans;
}


BOOLEAN MlmeQueueFull(
	IN MLME_QUEUE *Queue)
{
	BOOLEAN Ans;

	NdisAcquireSpinLock(&(Queue->Lock));
	Ans = (Queue->Num == MAX_LEN_OF_MLME_QUEUE || Queue->Entry[Queue->Tail].Occupied);
	NdisReleaseSpinLock(&(Queue->Lock));

	return Ans;
}


VOID MlmeQueueDestroy(
	IN MLME_QUEUE *pQueue)
{
	NdisAcquireSpinLock(&(pQueue->Lock));
	pQueue->Num  = 0;
	pQueue->Head = 0;
	pQueue->Tail = 0;
	NdisReleaseSpinLock(&(pQueue->Lock));
	NdisFreeSpinLock(&(pQueue->Lock));
}


BOOLEAN MsgTypeSubst(
	IN PRTMP_ADAPTER  pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType)
{
	USHORT	Seq;
	UCHAR	EAPType;
	PUCHAR	pData;

	
	pData = (PUCHAR) pFrame + LENGTH_802_11;

	
	if (pFrame->Hdr.FC.Type == BTYPE_DATA)
	{
		if (NdisEqualMemory(SNAP_AIRONET, pData, LENGTH_802_1_H))
		{
			
			*Machine = AIRONET_STATE_MACHINE;
			*MsgType = MT2_AIRONET_MSG;
			return (TRUE);
		}
		{
			*Machine = WPA_PSK_STATE_MACHINE;
			EAPType = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
			return(WpaMsgTypeSubst(EAPType, MsgType));
		}
	}

	switch (pFrame->Hdr.FC.SubType)
	{
		case SUBTYPE_ASSOC_REQ:
			*Machine = ASSOC_STATE_MACHINE;
			*MsgType = MT2_PEER_ASSOC_REQ;
			break;
		case SUBTYPE_ASSOC_RSP:
			*Machine = ASSOC_STATE_MACHINE;
			*MsgType = MT2_PEER_ASSOC_RSP;
			break;
		case SUBTYPE_REASSOC_REQ:
			*Machine = ASSOC_STATE_MACHINE;
			*MsgType = MT2_PEER_REASSOC_REQ;
			break;
		case SUBTYPE_REASSOC_RSP:
			*Machine = ASSOC_STATE_MACHINE;
			*MsgType = MT2_PEER_REASSOC_RSP;
			break;
		case SUBTYPE_PROBE_REQ:
			*Machine = SYNC_STATE_MACHINE;
			*MsgType = MT2_PEER_PROBE_REQ;
			break;
		case SUBTYPE_PROBE_RSP:
			*Machine = SYNC_STATE_MACHINE;
			*MsgType = MT2_PEER_PROBE_RSP;
			break;
		case SUBTYPE_BEACON:
			*Machine = SYNC_STATE_MACHINE;
			*MsgType = MT2_PEER_BEACON;
			break;
		case SUBTYPE_ATIM:
			*Machine = SYNC_STATE_MACHINE;
			*MsgType = MT2_PEER_ATIM;
			break;
		case SUBTYPE_DISASSOC:
			*Machine = ASSOC_STATE_MACHINE;
			*MsgType = MT2_PEER_DISASSOC_REQ;
			break;
		case SUBTYPE_AUTH:
			
			NdisMoveMemory(&Seq, &pFrame->Octet[2], sizeof(USHORT));
			if (Seq == 1 || Seq == 3)
			{
				*Machine = AUTH_RSP_STATE_MACHINE;
				*MsgType = MT2_PEER_AUTH_ODD;
			}
			else if (Seq == 2 || Seq == 4)
			{
				*Machine = AUTH_STATE_MACHINE;
				*MsgType = MT2_PEER_AUTH_EVEN;
			}
			else
			{
				return FALSE;
			}
			break;
		case SUBTYPE_DEAUTH:
			*Machine = AUTH_RSP_STATE_MACHINE;
			*MsgType = MT2_PEER_DEAUTH;
			break;
		case SUBTYPE_ACTION:
			*Machine = ACTION_STATE_MACHINE;
			
			if ((pFrame->Octet[0]&0x7F) > MAX_PEER_CATE_MSG)
			{
				*MsgType = MT2_ACT_INVALID;
			}
			else
			{
				*MsgType = (pFrame->Octet[0]&0x7F);
			}
			break;
		default:
			return FALSE;
			break;
	}

	return TRUE;
}






VOID StateMachineInit(
	IN STATE_MACHINE *S,
	IN STATE_MACHINE_FUNC Trans[],
	IN ULONG StNr,
	IN ULONG MsgNr,
	IN STATE_MACHINE_FUNC DefFunc,
	IN ULONG InitState,
	IN ULONG Base)
{
	ULONG i, j;

	
	S->NrState = StNr;
	S->NrMsg   = MsgNr;
	S->Base    = Base;

	S->TransFunc  = Trans;

	
	for (i = 0; i < StNr; i++)
	{
		for (j = 0; j < MsgNr; j++)
		{
			S->TransFunc[i * MsgNr + j] = DefFunc;
		}
	}

	
	S->CurrState = InitState;
}


VOID StateMachineSetAction(
	IN STATE_MACHINE *S,
	IN ULONG St,
	IN ULONG Msg,
	IN STATE_MACHINE_FUNC Func)
{
	ULONG MsgIdx;

	MsgIdx = Msg - S->Base;

	if (St < S->NrState && MsgIdx < S->NrMsg)
	{
		
		S->TransFunc[St * S->NrMsg + MsgIdx] = Func;
	}
}


VOID StateMachinePerformAction(
	IN	PRTMP_ADAPTER	pAd,
	IN STATE_MACHINE *S,
	IN MLME_QUEUE_ELEM *Elem)
{
	(*(S->TransFunc[S->CurrState * S->NrMsg + Elem->MsgType - S->Base]))(pAd, Elem);
}


VOID Drop(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
}






VOID LfsrInit(
	IN PRTMP_ADAPTER pAd,
	IN ULONG Seed)
{
	if (Seed == 0)
		pAd->Mlme.ShiftReg = 1;
	else
		pAd->Mlme.ShiftReg = Seed;
}


UCHAR RandomByte(
	IN PRTMP_ADAPTER pAd)
{
	ULONG i;
	UCHAR R, Result;

	R = 0;

	if (pAd->Mlme.ShiftReg == 0)
	NdisGetSystemUpTime((ULONG *)&pAd->Mlme.ShiftReg);

	for (i = 0; i < 8; i++)
	{
		if (pAd->Mlme.ShiftReg & 0x00000001)
		{
			pAd->Mlme.ShiftReg = ((pAd->Mlme.ShiftReg ^ LFSR_MASK) >> 1) | 0x80000000;
			Result = 1;
		}
		else
		{
			pAd->Mlme.ShiftReg = pAd->Mlme.ShiftReg >> 1;
			Result = 0;
		}
		R = (R << 1) | Result;
	}

	return R;
}

VOID AsicUpdateAutoFallBackTable(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pRateTable)
{
	UCHAR					i;
	HT_FBK_CFG0_STRUC		HtCfg0;
	HT_FBK_CFG1_STRUC		HtCfg1;
	LG_FBK_CFG0_STRUC		LgCfg0;
	LG_FBK_CFG1_STRUC		LgCfg1;
	PRTMP_TX_RATE_SWITCH	pCurrTxRate, pNextTxRate;

	
	HtCfg0.word = 0x65432100;
	HtCfg1.word = 0xedcba988;
	LgCfg0.word = 0xedcba988;
	LgCfg1.word = 0x00002100;

	pNextTxRate = (PRTMP_TX_RATE_SWITCH)pRateTable+1;
	for (i = 1; i < *((PUCHAR) pRateTable); i++)
	{
		pCurrTxRate = (PRTMP_TX_RATE_SWITCH)pRateTable+1+i;
		switch (pCurrTxRate->Mode)
		{
			case 0:		
				break;
			case 1:		
				{
					switch(pCurrTxRate->CurrMCS)
					{
						case 0:
							LgCfg0.field.OFDMMCS0FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 1:
							LgCfg0.field.OFDMMCS1FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 2:
							LgCfg0.field.OFDMMCS2FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 3:
							LgCfg0.field.OFDMMCS3FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 4:
							LgCfg0.field.OFDMMCS4FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 5:
							LgCfg0.field.OFDMMCS5FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 6:
							LgCfg0.field.OFDMMCS6FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
						case 7:
							LgCfg0.field.OFDMMCS7FBK = (pNextTxRate->Mode == MODE_OFDM) ? (pNextTxRate->CurrMCS+8): pNextTxRate->CurrMCS;
							break;
					}
				}
				break;
			case 2:		
			case 3:		
				{
					if ((pNextTxRate->Mode >= MODE_HTMIX) && (pCurrTxRate->CurrMCS != pNextTxRate->CurrMCS))
					{
						switch(pCurrTxRate->CurrMCS)
						{
							case 0:
								HtCfg0.field.HTMCS0FBK = pNextTxRate->CurrMCS;
								break;
							case 1:
								HtCfg0.field.HTMCS1FBK = pNextTxRate->CurrMCS;
								break;
							case 2:
								HtCfg0.field.HTMCS2FBK = pNextTxRate->CurrMCS;
								break;
							case 3:
								HtCfg0.field.HTMCS3FBK = pNextTxRate->CurrMCS;
								break;
							case 4:
								HtCfg0.field.HTMCS4FBK = pNextTxRate->CurrMCS;
								break;
							case 5:
								HtCfg0.field.HTMCS5FBK = pNextTxRate->CurrMCS;
								break;
							case 6:
								HtCfg0.field.HTMCS6FBK = pNextTxRate->CurrMCS;
								break;
							case 7:
								HtCfg0.field.HTMCS7FBK = pNextTxRate->CurrMCS;
								break;
							case 8:
								HtCfg1.field.HTMCS8FBK = pNextTxRate->CurrMCS;
								break;
							case 9:
								HtCfg1.field.HTMCS9FBK = pNextTxRate->CurrMCS;
								break;
							case 10:
								HtCfg1.field.HTMCS10FBK = pNextTxRate->CurrMCS;
								break;
							case 11:
								HtCfg1.field.HTMCS11FBK = pNextTxRate->CurrMCS;
								break;
							case 12:
								HtCfg1.field.HTMCS12FBK = pNextTxRate->CurrMCS;
								break;
							case 13:
								HtCfg1.field.HTMCS13FBK = pNextTxRate->CurrMCS;
								break;
							case 14:
								HtCfg1.field.HTMCS14FBK = pNextTxRate->CurrMCS;
								break;
							case 15:
								HtCfg1.field.HTMCS15FBK = pNextTxRate->CurrMCS;
								break;
							default:
								DBGPRINT(RT_DEBUG_ERROR, ("AsicUpdateAutoFallBackTable: not support CurrMCS=%d\n", pCurrTxRate->CurrMCS));
						}
					}
				}
				break;
		}

		pNextTxRate = pCurrTxRate;
	}

	RTMP_IO_WRITE32(pAd, HT_FBK_CFG0, HtCfg0.word);
	RTMP_IO_WRITE32(pAd, HT_FBK_CFG1, HtCfg1.word);
	RTMP_IO_WRITE32(pAd, LG_FBK_CFG0, LgCfg0.word);
	RTMP_IO_WRITE32(pAd, LG_FBK_CFG1, LgCfg1.word);
}


VOID 	AsicUpdateProtect(
	IN		PRTMP_ADAPTER	pAd,
	IN 		USHORT			OperationMode,
	IN 		UCHAR			SetMask,
	IN		BOOLEAN			bDisableBGProtect,
	IN		BOOLEAN			bNonGFExist)
{
	PROT_CFG_STRUC	ProtCfg, ProtCfg4;
	UINT32 Protect[6];
	USHORT			offset;
	UCHAR			i;
	UINT32 MacReg = 0;

	if (!(pAd->CommonCfg.bHTProtect) && (OperationMode != 8))
	{
		return;
	}

	if (pAd->BATable.numAsOriginator)
	{
		
		
		
		SetMask = ALLN_SETPROTECT;
		OperationMode = 8;
	}

	
	RTMP_IO_READ32(pAd, TX_RTS_CFG, &MacReg);
	MacReg &= 0xFF0000FF;

	
        if ((
			(pAd->CommonCfg.BACapability.field.AmsduEnable) ||
			(pAd->CommonCfg.bAggregationCapable == TRUE))
            && pAd->CommonCfg.RtsThreshold == MAX_RTS_THRESHOLD)
        {
			MacReg |= (0x1000 << 8);
        }
        else
        {
			MacReg |= (pAd->CommonCfg.RtsThreshold << 8);
        }

	RTMP_IO_WRITE32(pAd, TX_RTS_CFG, MacReg);

	
	RTMPZeroMemory(Protect, sizeof(Protect));
	ProtCfg4.word = 0;
	ProtCfg.word = 0;
	ProtCfg.field.TxopAllowGF40 = 1;
	ProtCfg.field.TxopAllowGF20 = 1;
	ProtCfg.field.TxopAllowMM40 = 1;
	ProtCfg.field.TxopAllowMM20 = 1;
	ProtCfg.field.TxopAllowOfdm = 1;
	ProtCfg.field.TxopAllowCck = 1;
	ProtCfg.field.RTSThEn = 1;
	ProtCfg.field.ProtectNav = ASIC_SHORTNAV;

	
	if (pAd->CommonCfg.Channel > 14)
		ProtCfg.field.ProtectRate = 0x4000;
	ProtCfg.field.ProtectRate |= pAd->CommonCfg.RtsRate;

	
	if (bDisableBGProtect)
	{
		
		ProtCfg.field.ProtectCtrl = 0;
		Protect[0] = ProtCfg.word;
		Protect[1] = ProtCfg.word;
	}
	else
	{
		
		ProtCfg.field.ProtectCtrl = 0;			
		Protect[0] = ProtCfg.word;
		ProtCfg.field.ProtectCtrl = ASIC_CTS;	
		Protect[1] = ProtCfg.word;
	}

	
	if ((SetMask & ALLN_SETPROTECT) != 0)
	{
		switch(OperationMode)
		{
			case 0x0:
				
				
				
				
				

				
				
				
				
				
				
				Protect[2] = 0x01744004;

				
				
				
				
				
				
				Protect[3] = 0x03f44084;

				
				
				
				
				
				
				Protect[4] = 0x01744004;

				
				
				
				
				
				
				Protect[5] = 0x03f44084;

				if (bNonGFExist)
				{
					
					
					Protect[4] = 0x01754004;
					Protect[5] = 0x03f54084;
				}
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = FALSE;
				break;

 			case 1:
				
				
				ProtCfg.word = 0x01744004;	
				ProtCfg4.word = 0x03f44084; 
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	
					ProtCfg4.word = 0x03f40003; 
				}
				
				ProtCfg.field.ProtectCtrl = ASIC_RTS;
				ProtCfg.field.ProtectNav = ASIC_SHORTNAV;
				ProtCfg4.field.ProtectCtrl = ASIC_RTS;
				ProtCfg4.field.ProtectNav = ASIC_SHORTNAV;
				Protect[2] = ProtCfg.word;
				Protect[3] = ProtCfg4.word;
				Protect[4] = ProtCfg.word;
				Protect[5] = ProtCfg4.word;
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = TRUE;
				break;

			case 2:
				
				ProtCfg.word = 0x01744004;  
				ProtCfg4.word = 0x03f44084; 

				
				ProtCfg4.field.ProtectCtrl = ASIC_RTS;
				ProtCfg4.field.ProtectNav = ASIC_SHORTNAV;
				Protect[2] = ProtCfg.word;
				Protect[3] = ProtCfg4.word;
				if (bNonGFExist)
				{
					ProtCfg.field.ProtectCtrl = ASIC_RTS;
					ProtCfg.field.ProtectNav = ASIC_SHORTNAV;
				}
				Protect[4] = ProtCfg.word;
				Protect[5] = ProtCfg4.word;

				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = FALSE;
				break;

			case 3:
				
				
				ProtCfg.word = 0x01744004;	
				ProtCfg4.word = 0x03f44084;
				
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					ProtCfg.word = 0x01740003;	
					ProtCfg4.word = 0x03f40003; 
				}
				
				ProtCfg.field.ProtectCtrl = ASIC_RTS;
				ProtCfg.field.ProtectNav = ASIC_SHORTNAV;
				ProtCfg4.field.ProtectCtrl = ASIC_RTS;
				ProtCfg4.field.ProtectNav = ASIC_SHORTNAV;
				Protect[2] = ProtCfg.word;
				Protect[3] = ProtCfg4.word;
				Protect[4] = ProtCfg.word;
				Protect[5] = ProtCfg4.word;
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = TRUE;
				break;

			case 8:
				
				Protect[2] = 0x01754004;
				Protect[3] = 0x03f54084;
				Protect[4] = 0x01754004;
				Protect[5] = 0x03f54084;
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = TRUE;
				break;
		}
	}

	offset = CCK_PROT_CFG;
	for (i = 0;i < 6;i++)
	{
		if ((SetMask & (1<< i)))
		{
			RTMP_IO_WRITE32(pAd, offset + i*4, Protect[i]);
		}
	}
}

#ifdef RT2870

VOID RT30xxLoadRFNormalModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RFValue;

	
	RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
	RFValue = (RFValue & (~0x0C)) | 0x31;
	RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

	
	RT30xxReadRFRegister(pAd, RF_R15, &RFValue);
	RFValue &= (~0x08);
	RT30xxWriteRFRegister(pAd, RF_R15, RFValue);

	
	RT30xxReadRFRegister(pAd, RF_R17, &RFValue);
	RFValue &= (~0x08);
	
	if (((pAd->MACVersion & 0xffff) >= 0x0211) && (pAd->NicConfig2.field.ExternalLNAForG == 0))
	{
		RFValue |= 0x20;
	}
	RT30xxWriteRFRegister(pAd, RF_R17, RFValue);

	
	RT30xxReadRFRegister(pAd, RF_R20, &RFValue);
	RFValue &= (~0x08);
	RT30xxWriteRFRegister(pAd, RF_R20, RFValue);

	
	RT30xxReadRFRegister(pAd, RF_R21, &RFValue);
	RFValue &= (~0x08);
	RT30xxWriteRFRegister(pAd, RF_R21, RFValue);

	
	RT30xxReadRFRegister(pAd, RF_R27, &RFValue);
	if ((pAd->MACVersion & 0xffff) < 0x0211)
		RFValue = (RFValue & (~0x77)) | 0x3;
	else
		RFValue = (RFValue & (~0x77));
	RT30xxWriteRFRegister(pAd, RF_R27, RFValue);
	
}


VOID RT30xxLoadRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RFValue;
	UINT32 MACValue;

	
	RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
	RFValue &= (~0x01);
	RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

	
	RT30xxReadRFRegister(pAd, RF_R07, &RFValue);
	RFValue &= (~0x30);
	RT30xxWriteRFRegister(pAd, RF_R07, RFValue);

	
	RT30xxReadRFRegister(pAd, RF_R09, &RFValue);
	RFValue &= (~0x0E);
	RT30xxWriteRFRegister(pAd, RF_R09, RFValue);

	
	RT30xxReadRFRegister(pAd, RF_R21, &RFValue);
	RFValue &= (~0x80);
	RT30xxWriteRFRegister(pAd, RF_R21, RFValue);

	
	RT30xxReadRFRegister(pAd, RF_R27, &RFValue);
	RFValue |= 0x77;
	RT30xxWriteRFRegister(pAd, RF_R27, RFValue);

	RTMP_IO_READ32(pAd, LDO_CFG0, &MACValue);
	MACValue |= 0x1D000000;
	RTMP_IO_WRITE32(pAd, LDO_CFG0, MACValue);
}


VOID RT30xxReverseRFSleepModeSetup(
	IN PRTMP_ADAPTER 	pAd)
{
	UCHAR RFValue;
	UINT32 MACValue;

	
	RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
	RFValue |= 0x01;
	RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

	
	RT30xxReadRFRegister(pAd, RF_R07, &RFValue);
	RFValue |= 0x30;
	RT30xxWriteRFRegister(pAd, RF_R07, RFValue);

	
	RT30xxReadRFRegister(pAd, RF_R09, &RFValue);
	RFValue |= 0x0E;
	RT30xxWriteRFRegister(pAd, RF_R09, RFValue);

	
	RT30xxReadRFRegister(pAd, RF_R21, &RFValue);
	RFValue |= 0x80;
	RT30xxWriteRFRegister(pAd, RF_R21, RFValue);

	
	RT30xxReadRFRegister(pAd, RF_R27, &RFValue);
	if ((pAd->MACVersion & 0xffff) < 0x0211)
		RFValue = (RFValue & (~0x77)) | 0x3;
	else
		RFValue = (RFValue & (~0x77));
	RT30xxWriteRFRegister(pAd, RF_R27, RFValue);

	
	if ((pAd->NicConfig2.field.DACTestBit == 1) && ((pAd->MACVersion & 0xffff) < 0x0211))
	{
		
		RTMP_IO_READ32(pAd, LDO_CFG0, &MACValue);
		MACValue = ((MACValue & 0xE0FFFFFF) | 0x0D000000);
		RTMP_IO_WRITE32(pAd, LDO_CFG0, MACValue);
	}
	else
	{
		RTMP_IO_READ32(pAd, LDO_CFG0, &MACValue);
		MACValue = ((MACValue & 0xE0FFFFFF) | 0x01000000);
		RTMP_IO_WRITE32(pAd, LDO_CFG0, MACValue);
	}
}
#endif


VOID AsicSwitchChannel(
					  IN PRTMP_ADAPTER pAd,
	IN	UCHAR			Channel,
	IN	BOOLEAN			bScan)
{
	ULONG			R2 = 0, R3 = DEFAULT_RF_TX_POWER, R4 = 0;
	CHAR    TxPwer = 0, TxPwer2 = DEFAULT_RF_TX_POWER; 
	UCHAR	index;
	UINT32 	Value = 0; 
	RTMP_RF_REGS *RFRegTable;

	
	
	
	
	for (index = 0; index < MAX_NUM_OF_CHANNELS; index++)
	{
		if (Channel == pAd->TxPower[index].Channel)
	{
			TxPwer = pAd->TxPower[index].Power;
			TxPwer2 = pAd->TxPower[index].Power2;
			break;
		}
	}

	if (index == MAX_NUM_OF_CHANNELS)
		DBGPRINT(RT_DEBUG_ERROR, ("AsicSwitchChannel: Can't find the Channel#%d \n", Channel));

#ifdef RT2870
	
	if ((IS_RT3070(pAd) || IS_RT3090(pAd)) && (
	     (pAd->RfIcType == RFIC_3022) || (pAd->RfIcType == RFIC_3021) ||
	     (pAd->RfIcType == RFIC_3020) || (pAd->RfIcType == RFIC_2020)))
	{
		
		UCHAR RFValue;

		for (index = 0; index < NUM_OF_3020_CHNL; index++)
		{
			if (Channel == FreqItems3020[index].Channel)
			{
				
				RT30xxWriteRFRegister(pAd, RF_R02, FreqItems3020[index].N);
				RT30xxWriteRFRegister(pAd, RF_R03, FreqItems3020[index].K);

				RT30xxReadRFRegister(pAd, RF_R06, &RFValue);
				RFValue = (RFValue & 0xFC) | FreqItems3020[index].R;
				RT30xxWriteRFRegister(pAd, RF_R06, RFValue);

				
				RT30xxReadRFRegister(pAd, RF_R12, &RFValue);
				RFValue = (RFValue & 0xE0) | TxPwer;
				RT30xxWriteRFRegister(pAd, RF_R12, RFValue);

				
				RT30xxReadRFRegister(pAd, RF_R13, &RFValue);
				RFValue = (RFValue & 0xE0) | TxPwer2;
				RT30xxWriteRFRegister(pAd, RF_R13, RFValue);

				
				RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
				
				
				RFValue &= 0x03;	
				if (pAd->Antenna.field.TxPath == 1)
					RFValue |= 0xA0;
				else if (pAd->Antenna.field.TxPath == 2)
					RFValue |= 0x80;
				if (pAd->Antenna.field.RxPath == 1)
					RFValue |= 0x50;
				else if (pAd->Antenna.field.RxPath == 2)
					RFValue |= 0x40;
				RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

				
				RT30xxReadRFRegister(pAd, RF_R23, &RFValue);
				RFValue = (RFValue & 0x80) | pAd->RfFreqOffset;
				RT30xxWriteRFRegister(pAd, RF_R23, RFValue);

				
				if (!bScan && (pAd->CommonCfg.BBPCurrentBW == BW_40))
				{
					RFValue = pAd->Mlme.CaliBW40RfR24;
					
				}
				else
				{
					RFValue = pAd->Mlme.CaliBW20RfR24;
				}
				RT30xxWriteRFRegister(pAd, RF_R24, RFValue);
				RT30xxWriteRFRegister(pAd, RF_R31, RFValue);

				
				RT30xxReadRFRegister(pAd, RF_R07, &RFValue);
				RFValue = RFValue | 0x1;
				RT30xxWriteRFRegister(pAd, RF_R07, RFValue);

				
				pAd->LatchRfRegs.Channel = Channel;

				DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
					Channel,
					pAd->RfIcType,
					TxPwer,
					TxPwer2,
					pAd->Antenna.field.TxPath,
					FreqItems3020[index].N,
					FreqItems3020[index].K,
					FreqItems3020[index].R));
				break;
			}
		}

		DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
			Channel,
			pAd->RfIcType,
			TxPwer,
			TxPwer2,
			pAd->Antenna.field.TxPath,
			FreqItems3020[index].N,
			FreqItems3020[index].K,
			FreqItems3020[index].R));
	}
	else
#endif 
	{
		RFRegTable = RF2850RegTable;

		switch (pAd->RfIcType)
		{
			case RFIC_2820:
			case RFIC_2850:
			case RFIC_2720:
			case RFIC_2750:

			for (index = 0; index < NUM_OF_2850_CHNL; index++)
			{
				if (Channel == RFRegTable[index].Channel)
				{
					R2 = RFRegTable[index].R2;
					if (pAd->Antenna.field.TxPath == 1)
					{
						R2 |= 0x4000;	
					}

					if (pAd->Antenna.field.RxPath == 2)
					{
						R2 |= 0x40;	
					}
					else if (pAd->Antenna.field.RxPath == 1)
					{
						R2 |= 0x20040;	
					}

					if (Channel > 14)
					{
						
						R3 = (RFRegTable[index].R3 & 0xffffc1ff);
						R4 = (RFRegTable[index].R4 & (~0x001f87c0)) | (pAd->RfFreqOffset << 15);

						
						
						if ((TxPwer >= -7) && (TxPwer < 0))
						{
							TxPwer = (7+TxPwer);
							TxPwer = (TxPwer > 0xF) ? (0xF) : (TxPwer);
							R3 |= (TxPwer << 10);
							DBGPRINT(RT_DEBUG_ERROR, ("AsicSwitchChannel: TxPwer=%d \n", TxPwer));
						}
						else
						{
							TxPwer = (TxPwer > 0xF) ? (0xF) : (TxPwer);
							R3 |= (TxPwer << 10) | (1 << 9);
						}

						
						if ((TxPwer2 >= -7) && (TxPwer2 < 0))
						{
							TxPwer2 = (7+TxPwer2);
							TxPwer2 = (TxPwer2 > 0xF) ? (0xF) : (TxPwer2);
							R4 |= (TxPwer2 << 7);
							DBGPRINT(RT_DEBUG_ERROR, ("AsicSwitchChannel: TxPwer2=%d \n", TxPwer2));
						}
						else
						{
							TxPwer2 = (TxPwer2 > 0xF) ? (0xF) : (TxPwer2);
							R4 |= (TxPwer2 << 7) | (1 << 6);
						}
					}
					else
					{
						R3 = (RFRegTable[index].R3 & 0xffffc1ff) | (TxPwer << 9); 
					R4 = (RFRegTable[index].R4 & (~0x001f87c0)) | (pAd->RfFreqOffset << 15) | (TxPwer2 <<6);
					}

					
					if (!bScan && (pAd->CommonCfg.BBPCurrentBW == BW_40))
					{
						R4 |=0x200000;
					}

					
					pAd->LatchRfRegs.Channel = Channel;
					pAd->LatchRfRegs.R1 = RFRegTable[index].R1;
					pAd->LatchRfRegs.R2 = R2;
					pAd->LatchRfRegs.R3 = R3;
					pAd->LatchRfRegs.R4 = R4;

					
					RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
					RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
					RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 & (~0x04)));
					RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

					RTMPusecDelay(200);

					
					RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
					RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
					RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 | 0x04));
					RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

					RTMPusecDelay(200);

					
					RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
					RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
					RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 & (~0x04)));
					RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

					break;
				}
			}
			break;

			default:
			break;
		}
	}

	
	if (Channel <= 14)
	{
	    ULONG	TxPinCfg = 0x00050F0A;

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);
		

		
	    if (pAd->NicConfig2.field.ExternalLNAForG)
	    {
	        RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
	    }
	    else
	    {
	        RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x84);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x50);
	    }

		
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x04);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

        
		if (pAd->Antenna.field.TxPath == 1)
		{
			TxPinCfg &= 0xFFFFFFF3;
		}
		if (pAd->Antenna.field.RxPath == 1)
		{
			TxPinCfg &= 0xFFFFF3FF;
		}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
	}
	else
	{
	    ULONG	TxPinCfg = 0x00050F05;

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0xF2);

		
		if (pAd->NicConfig2.field.ExternalLNAForA)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x46);
		}
		else
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R75, 0x50);
		}

		
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x02);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

        
		if (pAd->Antenna.field.TxPath == 1)
		{
			TxPinCfg &= 0xFFFFFFF3;
	}
		if (pAd->Antenna.field.RxPath == 1)
		{
			TxPinCfg &= 0xFFFFF3FF;
	}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
	}

    
	
	if (bScan)
		RTMPSetAGCInitValue(pAd, BW_20);
	else
		RTMPSetAGCInitValue(pAd, pAd->CommonCfg.BBPCurrentBW);

	
	
	
	
	
	RTMPusecDelay(1000);

	DBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, Pwr0=%lu, Pwr1=%lu, %dT) to , R1=0x%08lx, R2=0x%08lx, R3=0x%08lx, R4=0x%08lx\n",
							  Channel,
							  pAd->RfIcType,
							  (R3 & 0x00003e00) >> 9,
							  (R4 & 0x000007c0) >> 6,
							  pAd->Antenna.field.TxPath,
							  pAd->LatchRfRegs.R1,
							  pAd->LatchRfRegs.R2,
							  pAd->LatchRfRegs.R3,
							  pAd->LatchRfRegs.R4));
}


VOID AsicLockChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel)
{
}

VOID AsicRfTuningExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
}


VOID AsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd)
{
	INT			i, j;
	CHAR		DeltaPwr = 0;
	BOOLEAN		bAutoTxAgc = FALSE;
	UCHAR		TssiRef, *pTssiMinusBoundary, *pTssiPlusBoundary, TxAgcStep;
	UCHAR		BbpR1 = 0, BbpR49 = 0, idx;
	PCHAR		pTxAgcCompensate;
	ULONG		TxPwr[5];
	CHAR		Value;

#ifdef RT2860
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE)
		|| (pAd->bPCIclkOff == TRUE)
		|| RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)
		|| RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		return;
#endif

	if (pAd->CommonCfg.BBPCurrentBW == BW_40)
	{
		if (pAd->CommonCfg.CentralChannel > 14)
		{
			TxPwr[0] = pAd->Tx40MPwrCfgABand[0];
			TxPwr[1] = pAd->Tx40MPwrCfgABand[1];
			TxPwr[2] = pAd->Tx40MPwrCfgABand[2];
			TxPwr[3] = pAd->Tx40MPwrCfgABand[3];
			TxPwr[4] = pAd->Tx40MPwrCfgABand[4];
		}
		else
		{
			TxPwr[0] = pAd->Tx40MPwrCfgGBand[0];
			TxPwr[1] = pAd->Tx40MPwrCfgGBand[1];
			TxPwr[2] = pAd->Tx40MPwrCfgGBand[2];
			TxPwr[3] = pAd->Tx40MPwrCfgGBand[3];
			TxPwr[4] = pAd->Tx40MPwrCfgGBand[4];
		}
	}
	else
	{
		if (pAd->CommonCfg.Channel > 14)
		{
			TxPwr[0] = pAd->Tx20MPwrCfgABand[0];
			TxPwr[1] = pAd->Tx20MPwrCfgABand[1];
			TxPwr[2] = pAd->Tx20MPwrCfgABand[2];
			TxPwr[3] = pAd->Tx20MPwrCfgABand[3];
			TxPwr[4] = pAd->Tx20MPwrCfgABand[4];
		}
		else
		{
			TxPwr[0] = pAd->Tx20MPwrCfgGBand[0];
			TxPwr[1] = pAd->Tx20MPwrCfgGBand[1];
			TxPwr[2] = pAd->Tx20MPwrCfgGBand[2];
			TxPwr[3] = pAd->Tx20MPwrCfgGBand[3];
			TxPwr[4] = pAd->Tx20MPwrCfgGBand[4];
		}
	}

	
	if (pAd->Mlme.OneSecPeriodicRound % 4 == 0)
	{
		if (pAd->CommonCfg.Channel <= 14)
		{
			
			bAutoTxAgc         = pAd->bAutoTxAgcG;
			TssiRef            = pAd->TssiRefG;
			pTssiMinusBoundary = &pAd->TssiMinusBoundaryG[0];
			pTssiPlusBoundary  = &pAd->TssiPlusBoundaryG[0];
			TxAgcStep          = pAd->TxAgcStepG;
			pTxAgcCompensate   = &pAd->TxAgcCompensateG;
		}
		else
		{
			
			bAutoTxAgc         = pAd->bAutoTxAgcA;
			TssiRef            = pAd->TssiRefA;
			pTssiMinusBoundary = &pAd->TssiMinusBoundaryA[0];
			pTssiPlusBoundary  = &pAd->TssiPlusBoundaryA[0];
			TxAgcStep          = pAd->TxAgcStepA;
			pTxAgcCompensate   = &pAd->TxAgcCompensateA;
		}

		if (bAutoTxAgc)
		{
			
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49);

			
			
			

			
			
			

			
			
			

			if (BbpR49 > pTssiMinusBoundary[1])
			{
				
				
				for (idx = 1; idx < 5; idx++)
				{
					if (BbpR49 <= pTssiMinusBoundary[idx])  
						break;
				}
				
				*pTxAgcCompensate = -(TxAgcStep * (idx-1));

				DeltaPwr += (*pTxAgcCompensate);
				DBGPRINT(RT_DEBUG_TRACE, ("-- Tx Power, BBP R1=%x, TssiRef=%x, TxAgcStep=%x, step = -%d\n",
					BbpR49, TssiRef, TxAgcStep, idx-1));
			}
			else if (BbpR49 < pTssiPlusBoundary[1])
			{
				
				
				for (idx = 1; idx < 5; idx++)
				{
					if (BbpR49 >= pTssiPlusBoundary[idx])   
						break;
				}
				
				*pTxAgcCompensate = TxAgcStep * (idx-1);
				DeltaPwr += (*pTxAgcCompensate);
				DBGPRINT(RT_DEBUG_TRACE, ("++ Tx Power, BBP R1=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
					BbpR49, TssiRef, TxAgcStep, idx-1));
			}
			else
			{
				*pTxAgcCompensate = 0;
				DBGPRINT(RT_DEBUG_TRACE, ("   Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
					BbpR49, TssiRef, TxAgcStep, 0));
			}
		}
	}
	else
	{
		if (pAd->CommonCfg.Channel <= 14)
		{
			bAutoTxAgc         = pAd->bAutoTxAgcG;
			pTxAgcCompensate   = &pAd->TxAgcCompensateG;
		}
		else
		{
			bAutoTxAgc         = pAd->bAutoTxAgcA;
			pTxAgcCompensate   = &pAd->TxAgcCompensateA;
		}

		if (bAutoTxAgc)
			DeltaPwr += (*pTxAgcCompensate);
	}

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpR1);
	BbpR1 &= 0xFC;

	
	
	
	if (pAd->CommonCfg.TxPowerPercentage == 0xffffffff)       
		;
	else if (pAd->CommonCfg.TxPowerPercentage > 90)  
		;
	else if (pAd->CommonCfg.TxPowerPercentage > 60)  
	{
		DeltaPwr -= 1;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 30)  
	{
		DeltaPwr -= 3;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 15)  
	{
		BbpR1 |= 0x01;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 9)   
	{
		BbpR1 |= 0x01;
		DeltaPwr -= 3;
	}
	else                                           
	{
		BbpR1 |= 0x02;
	}

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);

	
	for(i=0; i<5; i++)
	{
		if (TxPwr[i] != 0xffffffff)
		{
			for (j=0; j<8; j++)
			{
				Value = (CHAR)((TxPwr[i] >> j*4) & 0x0F); 

				if ((Value + DeltaPwr) < 0)
				{
					Value = 0; 
				}
				else if ((Value + DeltaPwr) > 0xF)
				{
					Value = 0xF; 
				}
				else
				{
					Value += DeltaPwr; 
				}

				
				TxPwr[i] = (TxPwr[i] & ~(0x0000000F << j*4)) | (Value << j*4);
			}

			
			
			
			RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + i*4, TxPwr[i]);
		}
	}

}


VOID AsicSleepThenAutoWakeup(
	IN PRTMP_ADAPTER pAd,
	IN USHORT TbttNumToNextWakeUp)
{
    RT28XX_STA_SLEEP_THEN_AUTO_WAKEUP(pAd, TbttNumToNextWakeUp);
}


VOID AsicForceSleep(
	IN PRTMP_ADAPTER pAd)
{

}


VOID AsicForceWakeup(
	IN PRTMP_ADAPTER pAd,
#ifdef RT2860
	IN UCHAR    	 Level)
#endif
#ifdef RT2870
	IN BOOLEAN    bFromTx)
#endif
{
    DBGPRINT(RT_DEBUG_TRACE, ("--> AsicForceWakeup \n"));
#ifdef RT2860
    RT28XX_STA_FORCE_WAKEUP(pAd, Level);
#endif
#ifdef RT2870
    RT28XX_STA_FORCE_WAKEUP(pAd, bFromTx);
#endif
}


VOID AsicSetBssid(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pBssid)
{
	ULONG		  Addr4;
	DBGPRINT(RT_DEBUG_TRACE, ("==============> AsicSetBssid %x:%x:%x:%x:%x:%x\n",
		pBssid[0],pBssid[1],pBssid[2],pBssid[3], pBssid[4],pBssid[5]));

	Addr4 = (ULONG)(pBssid[0])		 |
			(ULONG)(pBssid[1] << 8)  |
			(ULONG)(pBssid[2] << 16) |
			(ULONG)(pBssid[3] << 24);
	RTMP_IO_WRITE32(pAd, MAC_BSSID_DW0, Addr4);

	Addr4 = 0;
	
	Addr4 = (ULONG)(pBssid[4]) | (ULONG)(pBssid[5] << 8);

	RTMP_IO_WRITE32(pAd, MAC_BSSID_DW1, Addr4);
}

VOID AsicSetMcastWC(
	IN PRTMP_ADAPTER pAd)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[MCAST_WCID];
	USHORT		offset;

	pEntry->Sst        = SST_ASSOC;
	pEntry->Aid        = MCAST_WCID;	
	pEntry->PsMode     = PWR_ACTIVE;
	pEntry->CurrTxRate = pAd->CommonCfg.MlmeRate;
	offset = MAC_WCID_BASE + BSS0Mcast_WCID * HW_WCID_ENTRY_SIZE;
}


VOID AsicDelWcidTab(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	Wcid)
{
	ULONG		  Addr0 = 0x0, Addr1 = 0x0;
	ULONG 		offset;

	DBGPRINT(RT_DEBUG_TRACE, ("AsicDelWcidTab==>Wcid = 0x%x\n",Wcid));
	offset = MAC_WCID_BASE + Wcid * HW_WCID_ENTRY_SIZE;
	RTMP_IO_WRITE32(pAd, offset, Addr0);
	offset += 4;
	RTMP_IO_WRITE32(pAd, offset, Addr1);
}


VOID AsicEnableRDG(
	IN PRTMP_ADAPTER pAd)
{
	TX_LINK_CFG_STRUC	TxLinkCfg;
	UINT32				Data = 0;

	RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);
	TxLinkCfg.field.TxRDGEn = 1;
	RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);

	RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);
	Data  &= 0xFFFFFF00;
	Data  |= 0x80;
	RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);

	
}


VOID AsicDisableRDG(
	IN PRTMP_ADAPTER pAd)
{
	TX_LINK_CFG_STRUC	TxLinkCfg;
	UINT32				Data = 0;


	RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);
	TxLinkCfg.field.TxRDGEn = 0;
	RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);

	RTMP_IO_READ32(pAd, EDCA_AC0_CFG, &Data);

	Data  &= 0xFFFFFF00;
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE)
		&& (pAd->MacTab.fAnyStationMIMOPSDynamic == FALSE)
	)
	{
		
		if (pAd->CommonCfg.bEnableTxBurst)
			Data |= 0x20;
	}
	RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Data);
}


VOID AsicDisableSync(
	IN PRTMP_ADAPTER pAd)
{
	BCN_TIME_CFG_STRUC csr;

	DBGPRINT(RT_DEBUG_TRACE, ("--->Disable TSF synchronization\n"));

	
	
	
	pAd->TbttTickCount = 0;
	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
	csr.field.bBeaconGen = 0;
	csr.field.bTBTTEnable = 0;
	csr.field.TsfSyncMode = 0;
	csr.field.bTsfTicking = 0;
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);

}


VOID AsicEnableBssSync(
	IN PRTMP_ADAPTER pAd)
{
	BCN_TIME_CFG_STRUC csr;

	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableBssSync(INFRA mode)\n"));

	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);

	{
		csr.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; 
		csr.field.bTsfTicking = 1;
		csr.field.TsfSyncMode = 1; 
		csr.field.bBeaconGen  = 0; 
		csr.field.bTBTTEnable = 1;
	}

	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
}


VOID AsicEnableIbssSync(
	IN PRTMP_ADAPTER pAd)
{
	BCN_TIME_CFG_STRUC csr9;
	PUCHAR			ptr;
	UINT i;

	DBGPRINT(RT_DEBUG_TRACE, ("--->AsicEnableIbssSync(ADHOC mode. MPDUtotalByteCount = %d)\n", pAd->BeaconTxWI.MPDUtotalByteCount));

	RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr9.word);
	csr9.field.bBeaconGen = 0;
	csr9.field.bTBTTEnable = 0;
	csr9.field.bTsfTicking = 0;
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr9.word);

#ifdef RT2860
	
	ptr = (PUCHAR)&pAd->BeaconTxWI;
	for (i=0; i<TXWI_SIZE; i+=4)  
	{
		UINT32 longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
		RTMP_IO_WRITE32(pAd, HW_BEACON_BASE0 + i, longptr);
		ptr += 4;
	}

	
	ptr = pAd->BeaconBuf;
	for (i=0; i< pAd->BeaconTxWI.MPDUtotalByteCount; i+=4)
	{
		UINT32 longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
		RTMP_IO_WRITE32(pAd, HW_BEACON_BASE0 + TXWI_SIZE + i, longptr);
		ptr +=4;
	}
#endif
#ifdef RT2870
	
	ptr = (PUCHAR)&pAd->BeaconTxWI;
	for (i=0; i<TXWI_SIZE; i+=2)  
	{
		RTUSBMultiWrite(pAd, HW_BEACON_BASE0 + i, ptr, 2);
		ptr += 2;
	}

	
	ptr = pAd->BeaconBuf;
	for (i=0; i< pAd->BeaconTxWI.MPDUtotalByteCount; i+=2)
	{
		RTUSBMultiWrite(pAd, HW_BEACON_BASE0 + TXWI_SIZE + i, ptr, 2);
		ptr +=2;
	}
#endif 

	
	csr9.field.BeaconInterval = pAd->CommonCfg.BeaconPeriod << 4; 
	csr9.field.bTsfTicking = 1;
	csr9.field.TsfSyncMode = 2; 
	csr9.field.bTBTTEnable = 1;
	csr9.field.bBeaconGen = 1;
	RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr9.word);
}


VOID AsicSetEdcaParm(
	IN PRTMP_ADAPTER pAd,
	IN PEDCA_PARM	 pEdcaParm)
{
	EDCA_AC_CFG_STRUC   Ac0Cfg, Ac1Cfg, Ac2Cfg, Ac3Cfg;
	AC_TXOP_CSR0_STRUC csr0;
	AC_TXOP_CSR1_STRUC csr1;
	AIFSN_CSR_STRUC    AifsnCsr;
	CWMIN_CSR_STRUC    CwminCsr;
	CWMAX_CSR_STRUC    CwmaxCsr;
	int i;

	Ac0Cfg.word = 0;
	Ac1Cfg.word = 0;
	Ac2Cfg.word = 0;
	Ac3Cfg.word = 0;
	if ((pEdcaParm == NULL) || (pEdcaParm->bValid == FALSE))
	{
		DBGPRINT(RT_DEBUG_TRACE,("AsicSetEdcaParm\n"));
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WMM_INUSED);
		for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
		{
			if (pAd->MacTab.Content[i].ValidAsCLI || pAd->MacTab.Content[i].ValidAsApCli)
				CLIENT_STATUS_CLEAR_FLAG(&pAd->MacTab.Content[i], fCLIENT_STATUS_WMM_CAPABLE);
		}

		
		
		
		if( pAd->CommonCfg.bEnableTxBurst )
		{
			
			Ac0Cfg.field.AcTxop = 0x20; 
		}
		else
			Ac0Cfg.field.AcTxop = 0;	
		Ac0Cfg.field.Cwmin = CW_MIN_IN_BITS;
		Ac0Cfg.field.Cwmax = CW_MAX_IN_BITS;
		Ac0Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Ac0Cfg.word);

		Ac1Cfg.field.AcTxop = 0;	
		Ac1Cfg.field.Cwmin = CW_MIN_IN_BITS;
		Ac1Cfg.field.Cwmax = CW_MAX_IN_BITS;
		Ac1Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, Ac1Cfg.word);

		if (pAd->CommonCfg.PhyMode == PHY_11B)
		{
			Ac2Cfg.field.AcTxop = 192;	
			Ac3Cfg.field.AcTxop = 96;	
		}
		else
		{
			Ac2Cfg.field.AcTxop = 96;	
			Ac3Cfg.field.AcTxop = 48;	
		}
		Ac2Cfg.field.Cwmin = CW_MIN_IN_BITS;
		Ac2Cfg.field.Cwmax = CW_MAX_IN_BITS;
		Ac2Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, Ac2Cfg.word);
		Ac3Cfg.field.Cwmin = CW_MIN_IN_BITS;
		Ac3Cfg.field.Cwmax = CW_MAX_IN_BITS;
		Ac3Cfg.field.Aifsn = 2;
		RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, Ac3Cfg.word);

		
		
		
		csr0.field.Ac0Txop = 0;		
		csr0.field.Ac1Txop = 0;		
		RTMP_IO_WRITE32(pAd, WMM_TXOP0_CFG, csr0.word);
		if (pAd->CommonCfg.PhyMode == PHY_11B)
		{
			csr1.field.Ac2Txop = 192;		
			csr1.field.Ac3Txop = 96;		
		}
		else
		{
			csr1.field.Ac2Txop = 96;		
			csr1.field.Ac3Txop = 48;		
		}
		RTMP_IO_WRITE32(pAd, WMM_TXOP1_CFG, csr1.word);

		CwminCsr.word = 0;
		CwminCsr.field.Cwmin0 = CW_MIN_IN_BITS;
		CwminCsr.field.Cwmin1 = CW_MIN_IN_BITS;
		CwminCsr.field.Cwmin2 = CW_MIN_IN_BITS;
		CwminCsr.field.Cwmin3 = CW_MIN_IN_BITS;
		RTMP_IO_WRITE32(pAd, WMM_CWMIN_CFG, CwminCsr.word);

		CwmaxCsr.word = 0;
		CwmaxCsr.field.Cwmax0 = CW_MAX_IN_BITS;
		CwmaxCsr.field.Cwmax1 = CW_MAX_IN_BITS;
		CwmaxCsr.field.Cwmax2 = CW_MAX_IN_BITS;
		CwmaxCsr.field.Cwmax3 = CW_MAX_IN_BITS;
		RTMP_IO_WRITE32(pAd, WMM_CWMAX_CFG, CwmaxCsr.word);

		RTMP_IO_WRITE32(pAd, WMM_AIFSN_CFG, 0x00002222);

		NdisZeroMemory(&pAd->CommonCfg.APEdcaParm, sizeof(EDCA_PARM));
	}
	else
	{
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_WMM_INUSED);
		
		
		
		
		
		
		
		

		Ac0Cfg.field.AcTxop =  pEdcaParm->Txop[QID_AC_BE];
		Ac0Cfg.field.Cwmin= pEdcaParm->Cwmin[QID_AC_BE];
		Ac0Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_BE];
		Ac0Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_BE]; 

		Ac1Cfg.field.AcTxop =  pEdcaParm->Txop[QID_AC_BK];
		Ac1Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_BK]; 
		Ac1Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_BK];
		Ac1Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_BK]; 

		Ac2Cfg.field.AcTxop = (pEdcaParm->Txop[QID_AC_VI] * 6) / 10;
		Ac2Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_VI];
		Ac2Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_VI];
		Ac2Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_VI];

		{
			
			if (pAd->CommonCfg.bWiFiTest &&
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
				Ac2Cfg.field.Aifsn -= 1;

			
			
			if (STA_TGN_WIFI_ON(pAd) &&
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
			{
				Ac0Cfg.field.Aifsn = 3;
				Ac2Cfg.field.AcTxop = 5;
			}

#ifdef RT2870
			if (pAd->RfIcType == RFIC_3020 || pAd->RfIcType == RFIC_2020)
			{
				
				Ac2Cfg.field.Aifsn = 5;
			}
#endif
		}

		Ac3Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_VO];
		Ac3Cfg.field.Cwmin = pEdcaParm->Cwmin[QID_AC_VO];
		Ac3Cfg.field.Cwmax = pEdcaParm->Cwmax[QID_AC_VO];
		Ac3Cfg.field.Aifsn = pEdcaParm->Aifsn[QID_AC_VO];


		if (pAd->CommonCfg.bWiFiTest)
		{
			if (Ac3Cfg.field.AcTxop == 102)
			{
			Ac0Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_BE] ? pEdcaParm->Txop[QID_AC_BE] : 10;
				Ac0Cfg.field.Aifsn  = pEdcaParm->Aifsn[QID_AC_BE]-1; 
			Ac1Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_BK];
				Ac1Cfg.field.Aifsn  = pEdcaParm->Aifsn[QID_AC_BK];
			Ac2Cfg.field.AcTxop = pEdcaParm->Txop[QID_AC_VI];
			} 
		}


		RTMP_IO_WRITE32(pAd, EDCA_AC0_CFG, Ac0Cfg.word);
		RTMP_IO_WRITE32(pAd, EDCA_AC1_CFG, Ac1Cfg.word);
		RTMP_IO_WRITE32(pAd, EDCA_AC2_CFG, Ac2Cfg.word);
		RTMP_IO_WRITE32(pAd, EDCA_AC3_CFG, Ac3Cfg.word);


		
		
		
		csr0.field.Ac0Txop = Ac0Cfg.field.AcTxop;
		csr0.field.Ac1Txop = Ac1Cfg.field.AcTxop;
		RTMP_IO_WRITE32(pAd, WMM_TXOP0_CFG, csr0.word);

		csr1.field.Ac2Txop = Ac2Cfg.field.AcTxop;
		csr1.field.Ac3Txop = Ac3Cfg.field.AcTxop;
		RTMP_IO_WRITE32(pAd, WMM_TXOP1_CFG, csr1.word);

		CwminCsr.word = 0;
		CwminCsr.field.Cwmin0 = pEdcaParm->Cwmin[QID_AC_BE];
		CwminCsr.field.Cwmin1 = pEdcaParm->Cwmin[QID_AC_BK];
		CwminCsr.field.Cwmin2 = pEdcaParm->Cwmin[QID_AC_VI];

		CwminCsr.field.Cwmin3 = pEdcaParm->Cwmin[QID_AC_VO] - 1; 

		RTMP_IO_WRITE32(pAd, WMM_CWMIN_CFG, CwminCsr.word);

		CwmaxCsr.word = 0;
		CwmaxCsr.field.Cwmax0 = pEdcaParm->Cwmax[QID_AC_BE];
		CwmaxCsr.field.Cwmax1 = pEdcaParm->Cwmax[QID_AC_BK];
		CwmaxCsr.field.Cwmax2 = pEdcaParm->Cwmax[QID_AC_VI];
		CwmaxCsr.field.Cwmax3 = pEdcaParm->Cwmax[QID_AC_VO];
		RTMP_IO_WRITE32(pAd, WMM_CWMAX_CFG, CwmaxCsr.word);

		AifsnCsr.word = 0;
		AifsnCsr.field.Aifsn0 = Ac0Cfg.field.Aifsn; 
		AifsnCsr.field.Aifsn1 = Ac1Cfg.field.Aifsn; 
		AifsnCsr.field.Aifsn2 = Ac2Cfg.field.Aifsn; 

		{
			
			if (pAd->CommonCfg.bWiFiTest &&
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
				AifsnCsr.field.Aifsn2 = Ac2Cfg.field.Aifsn - 4;

			
			
			if (STA_TGN_WIFI_ON(pAd) &&
				pEdcaParm->Aifsn[QID_AC_VI] == 10)
			{
				AifsnCsr.field.Aifsn0 = 3;
				AifsnCsr.field.Aifsn2 = 7;
			}
#ifdef RT2870
			if (INFRA_ON(pAd))
				CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[BSSID_WCID], fCLIENT_STATUS_WMM_CAPABLE);
#endif
		}

		AifsnCsr.field.Aifsn3 = Ac3Cfg.field.Aifsn - 1; 
#ifdef RT2870
		if (pAd->RfIcType == RFIC_3020 || pAd->RfIcType == RFIC_2020)
			AifsnCsr.field.Aifsn2 = 0x2; 
#endif
		RTMP_IO_WRITE32(pAd, WMM_AIFSN_CFG, AifsnCsr.word);

		NdisMoveMemory(&pAd->CommonCfg.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));
		if (!ADHOC_ON(pAd))
		{
			DBGPRINT(RT_DEBUG_TRACE,("EDCA [#%d]: AIFSN CWmin CWmax  TXOP(us)  ACM\n", pEdcaParm->EdcaUpdateCount));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_BE      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[0],
									 pEdcaParm->Cwmin[0],
									 pEdcaParm->Cwmax[0],
									 pEdcaParm->Txop[0]<<5,
									 pEdcaParm->bACM[0]));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_BK      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[1],
									 pEdcaParm->Cwmin[1],
									 pEdcaParm->Cwmax[1],
									 pEdcaParm->Txop[1]<<5,
									 pEdcaParm->bACM[1]));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_VI      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[2],
									 pEdcaParm->Cwmin[2],
									 pEdcaParm->Cwmax[2],
									 pEdcaParm->Txop[2]<<5,
									 pEdcaParm->bACM[2]));
			DBGPRINT(RT_DEBUG_TRACE,("     AC_VO      %2d     %2d     %2d      %4d     %d\n",
									 pEdcaParm->Aifsn[3],
									 pEdcaParm->Cwmin[3],
									 pEdcaParm->Cwmax[3],
									 pEdcaParm->Txop[3]<<5,
									 pEdcaParm->bACM[3]));
		}
	}
}


VOID 	AsicSetSlotTime(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bUseShortSlotTime)
{
	ULONG	SlotTime;
	UINT32	RegValue = 0;

	if (pAd->CommonCfg.Channel > 14)
		bUseShortSlotTime = TRUE;

	if (bUseShortSlotTime)
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
	else
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);

	SlotTime = (bUseShortSlotTime)? 9 : 20;

	{
		
		if (((pAd->StaActive.SupportedPhyInfo.bHtEnable == FALSE) && (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED)))
			|| ((pAd->StaActive.SupportedPhyInfo.bHtEnable == TRUE) && (pAd->CommonCfg.BACapability.field.Policy == BA_NOTUSE))
			)
		{
			
			
		}
		else if (pAd->CommonCfg.bEnableTxBurst)
			SlotTime = 9;
	}

	
	
	
	
	
	if (pAd->StaCfg.BssType == BSS_ADHOC)
		SlotTime = 20;

	RTMP_IO_READ32(pAd, BKOFF_SLOT_CFG, &RegValue);
	RegValue = RegValue & 0xFFFFFF00;

	RegValue |= SlotTime;

	RTMP_IO_WRITE32(pAd, BKOFF_SLOT_CFG, RegValue);
}


VOID AsicAddSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 BssIndex,
	IN UCHAR		 KeyIdx,
	IN UCHAR		 CipherAlg,
	IN PUCHAR		 pKey,
	IN PUCHAR		 pTxMic,
	IN PUCHAR		 pRxMic)
{
	ULONG offset; 
	SHAREDKEY_MODE_STRUC csr1;
#ifdef RT2860
	INT   i;
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("AsicAddSharedKeyEntry BssIndex=%d, KeyIdx=%d\n", BssIndex,KeyIdx));


	DBGPRINT(RT_DEBUG_TRACE,("AsicAddSharedKeyEntry: %s key #%d\n", CipherName[CipherAlg], BssIndex*4 + KeyIdx));
	DBGPRINT_RAW(RT_DEBUG_TRACE, (" 	Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
		pKey[0],pKey[1],pKey[2],pKey[3],pKey[4],pKey[5],pKey[6],pKey[7],pKey[8],pKey[9],pKey[10],pKey[11],pKey[12],pKey[13],pKey[14],pKey[15]));
	if (pRxMic)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE, (" 	Rx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pRxMic[0],pRxMic[1],pRxMic[2],pRxMic[3],pRxMic[4],pRxMic[5],pRxMic[6],pRxMic[7]));
	}
	if (pTxMic)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE, (" 	Tx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pTxMic[0],pTxMic[1],pTxMic[2],pTxMic[3],pTxMic[4],pTxMic[5],pTxMic[6],pTxMic[7]));
	}

	
	
	

	offset = SHARED_KEY_TABLE_BASE + (4*BssIndex + KeyIdx)*HW_KEY_ENTRY_SIZE;
#ifdef RT2860
	for (i=0; i<MAX_LEN_OF_SHARE_KEY; i++)
	{
		RTMP_IO_WRITE8(pAd, offset + i, pKey[i]);
	}
#endif
#ifdef RT2870
	RTUSBMultiWrite(pAd, offset, pKey, MAX_LEN_OF_SHARE_KEY);
#endif
	offset += MAX_LEN_OF_SHARE_KEY;
	if (pTxMic)
	{
#ifdef RT2860
		for (i=0; i<8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset + i, pTxMic[i]);
		}
#endif
#ifdef RT2870
		RTUSBMultiWrite(pAd, offset, pTxMic, 8);
#endif
	}

	offset += 8;
	if (pRxMic)
	{
#ifdef RT2860
		for (i=0; i<8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset + i, pRxMic[i]);
		}
#endif
#ifdef RT2870
		RTUSBMultiWrite(pAd, offset, pRxMic, 8);
#endif
	}


	
	
	
	RTMP_IO_READ32(pAd, SHARED_KEY_MODE_BASE+4*(BssIndex/2), &csr1.word);
	DBGPRINT(RT_DEBUG_TRACE,("Read: SHARED_KEY_MODE_BASE at this Bss[%d] KeyIdx[%d]= 0x%x \n", BssIndex,KeyIdx, csr1.word));
	if ((BssIndex%2) == 0)
	{
		if (KeyIdx == 0)
			csr1.field.Bss0Key0CipherAlg = CipherAlg;
		else if (KeyIdx == 1)
			csr1.field.Bss0Key1CipherAlg = CipherAlg;
		else if (KeyIdx == 2)
			csr1.field.Bss0Key2CipherAlg = CipherAlg;
		else
			csr1.field.Bss0Key3CipherAlg = CipherAlg;
	}
	else
	{
		if (KeyIdx == 0)
			csr1.field.Bss1Key0CipherAlg = CipherAlg;
		else if (KeyIdx == 1)
			csr1.field.Bss1Key1CipherAlg = CipherAlg;
		else if (KeyIdx == 2)
			csr1.field.Bss1Key2CipherAlg = CipherAlg;
		else
			csr1.field.Bss1Key3CipherAlg = CipherAlg;
	}
	DBGPRINT(RT_DEBUG_TRACE,("Write: SHARED_KEY_MODE_BASE at this Bss[%d] = 0x%x \n", BssIndex, csr1.word));
	RTMP_IO_WRITE32(pAd, SHARED_KEY_MODE_BASE+4*(BssIndex/2), csr1.word);

}


VOID AsicRemoveSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 BssIndex,
	IN UCHAR		 KeyIdx)
{
	
	SHAREDKEY_MODE_STRUC csr1;

	DBGPRINT(RT_DEBUG_TRACE,("AsicRemoveSharedKeyEntry: #%d \n", BssIndex*4 + KeyIdx));

	RTMP_IO_READ32(pAd, SHARED_KEY_MODE_BASE+4*(BssIndex/2), &csr1.word);
	if ((BssIndex%2) == 0)
	{
		if (KeyIdx == 0)
			csr1.field.Bss0Key0CipherAlg = 0;
		else if (KeyIdx == 1)
			csr1.field.Bss0Key1CipherAlg = 0;
		else if (KeyIdx == 2)
			csr1.field.Bss0Key2CipherAlg = 0;
		else
			csr1.field.Bss0Key3CipherAlg = 0;
	}
	else
	{
		if (KeyIdx == 0)
			csr1.field.Bss1Key0CipherAlg = 0;
		else if (KeyIdx == 1)
			csr1.field.Bss1Key1CipherAlg = 0;
		else if (KeyIdx == 2)
			csr1.field.Bss1Key2CipherAlg = 0;
		else
			csr1.field.Bss1Key3CipherAlg = 0;
	}
	DBGPRINT(RT_DEBUG_TRACE,("Write: SHARED_KEY_MODE_BASE at this Bss[%d] = 0x%x \n", BssIndex, csr1.word));
	RTMP_IO_WRITE32(pAd, SHARED_KEY_MODE_BASE+4*(BssIndex/2), csr1.word);
	ASSERT(BssIndex < 4);
	ASSERT(KeyIdx < 4);

}


VOID AsicUpdateWCIDAttribute(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN UCHAR		BssIndex,
	IN UCHAR        CipherAlg,
	IN BOOLEAN		bUsePairewiseKeyTable)
{
	ULONG   WCIDAttri = 0, offset;

	
	
	
	
	offset = MAC_WCID_ATTRIBUTE_BASE + (WCID * HW_WCID_ATTRI_SIZE);
	WCIDAttri = (BssIndex << 4) | (CipherAlg << 1) | (bUsePairewiseKeyTable);
	RTMP_IO_WRITE32(pAd, offset, WCIDAttri);
}

VOID AsicUpdateWCIDIVEIV(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN ULONG        uIV,
	IN ULONG        uEIV)
{
	ULONG	offset;

	offset = MAC_IVEIV_TABLE_BASE + (WCID * HW_IVEIV_ENTRY_SIZE);

	RTMP_IO_WRITE32(pAd, offset, uIV);
	RTMP_IO_WRITE32(pAd, offset + 4, uEIV);
}

VOID AsicUpdateRxWCIDTable(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN PUCHAR        pAddr)
{
	ULONG offset;
	ULONG Addr;

	offset = MAC_WCID_BASE + (WCID * HW_WCID_ENTRY_SIZE);
	Addr = pAddr[0] + (pAddr[1] << 8) +(pAddr[2] << 16) +(pAddr[3] << 24);
	RTMP_IO_WRITE32(pAd, offset, Addr);
	Addr = pAddr[4] + (pAddr[5] << 8);
	RTMP_IO_WRITE32(pAd, offset + 4, Addr);
}



VOID AsicAddKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN UCHAR		BssIndex,
	IN UCHAR		KeyIdx,
	IN PCIPHER_KEY	pCipherKey,
	IN BOOLEAN		bUsePairewiseKeyTable,
	IN BOOLEAN		bTxKey)
{
	ULONG	offset;
	UCHAR	IV4 = 0;
	PUCHAR		pKey = pCipherKey->Key;
	PUCHAR		pTxMic = pCipherKey->TxMic;
	PUCHAR		pRxMic = pCipherKey->RxMic;
	PUCHAR		pTxtsc = pCipherKey->TxTsc;
	UCHAR		CipherAlg = pCipherKey->CipherAlg;
	SHAREDKEY_MODE_STRUC csr1;
#ifdef RT2860
	UCHAR		i;
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("==> AsicAddKeyEntry\n"));
	
	
	
	if (bUsePairewiseKeyTable)
		offset = PAIRWISE_KEY_TABLE_BASE + (WCID * HW_KEY_ENTRY_SIZE);
	else
		offset = SHARED_KEY_TABLE_BASE + (4 * BssIndex + KeyIdx) * HW_KEY_ENTRY_SIZE;

	
	
	
	
#ifdef RT2860
	for (i = 0; i < MAX_LEN_OF_PEER_KEY; i++)
	{
		RTMP_IO_WRITE8(pAd, offset + i, pKey[i]);
	}
#endif
#ifdef RT2870
	RTUSBMultiWrite(pAd, offset, pKey, MAX_LEN_OF_PEER_KEY);
#endif
	offset += MAX_LEN_OF_PEER_KEY;

	
	
	
	if (pTxMic)
	{
#ifdef RT2860
		for (i = 0; i < 8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset + i, pTxMic[i]);
		}
#endif
#ifdef RT2870
		RTUSBMultiWrite(pAd, offset, pTxMic, 8);
#endif
	}
	offset += LEN_TKIP_TXMICK;

	if (pRxMic)
	{
#ifdef RT2860
		for (i = 0; i < 8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset + i, pRxMic[i]);
		}
#endif
#ifdef RT2870
		RTUSBMultiWrite(pAd, offset, pRxMic, 8);
#endif
	}


	
	
	
	
	if (bTxKey)
	{
#ifdef RT2860
		offset = MAC_IVEIV_TABLE_BASE + (WCID * HW_IVEIV_ENTRY_SIZE);
		
		
		
		RTMP_IO_WRITE8(pAd, offset, pTxtsc[1]);
		RTMP_IO_WRITE8(pAd, offset + 1, ((pTxtsc[1] | 0x20) & 0x7f));
		RTMP_IO_WRITE8(pAd, offset + 2, pTxtsc[0]);

		IV4 = (KeyIdx << 6);
		if ((CipherAlg == CIPHER_TKIP) || (CipherAlg == CIPHER_TKIP_NO_MIC) ||(CipherAlg == CIPHER_AES))
			IV4 |= 0x20;  

		RTMP_IO_WRITE8(pAd, offset + 3, IV4);

		
		
		
		offset += 4;
		for (i = 0; i < 4; i++)
		{
			RTMP_IO_WRITE8(pAd, offset + i, pTxtsc[i + 2]);
		}

#endif
#ifdef RT2870
		UINT32 tmpVal;

		
		
		
		IV4 = (KeyIdx << 6);
		if ((CipherAlg == CIPHER_TKIP) || (CipherAlg == CIPHER_TKIP_NO_MIC) ||(CipherAlg == CIPHER_AES))
			IV4 |= 0x20;  

		tmpVal = pTxtsc[1] + (((pTxtsc[1] | 0x20) & 0x7f) << 8) + (pTxtsc[0] << 16) + (IV4 << 24);
		RTMP_IO_WRITE32(pAd, offset, tmpVal);

		
		
		
		offset += 4;
		RTMP_IO_WRITE32(pAd, offset, *(PUINT32)&pCipherKey->TxTsc[2]);
#endif 
		AsicUpdateWCIDAttribute(pAd, WCID, BssIndex, CipherAlg, bUsePairewiseKeyTable);
	}

	if (!bUsePairewiseKeyTable)
	{
		
		
		
		RTMP_IO_READ32(pAd, SHARED_KEY_MODE_BASE + 4 * (BssIndex / 2), &csr1.word);
		if ((BssIndex % 2) == 0)
		{
			if (KeyIdx == 0)
				csr1.field.Bss0Key0CipherAlg = CipherAlg;
			else if (KeyIdx == 1)
				csr1.field.Bss0Key1CipherAlg = CipherAlg;
			else if (KeyIdx == 2)
				csr1.field.Bss0Key2CipherAlg = CipherAlg;
			else
				csr1.field.Bss0Key3CipherAlg = CipherAlg;
		}
		else
		{
			if (KeyIdx == 0)
				csr1.field.Bss1Key0CipherAlg = CipherAlg;
			else if (KeyIdx == 1)
				csr1.field.Bss1Key1CipherAlg = CipherAlg;
			else if (KeyIdx == 2)
				csr1.field.Bss1Key2CipherAlg = CipherAlg;
			else
				csr1.field.Bss1Key3CipherAlg = CipherAlg;
		}
		RTMP_IO_WRITE32(pAd, SHARED_KEY_MODE_BASE + 4 * (BssIndex / 2), csr1.word);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<== AsicAddKeyEntry\n"));
}



VOID AsicAddPairwiseKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR        pAddr,
	IN UCHAR		WCID,
	IN CIPHER_KEY		 *pCipherKey)
{
	INT i;
	ULONG 		offset;
	PUCHAR		 pKey = pCipherKey->Key;
	PUCHAR		 pTxMic = pCipherKey->TxMic;
	PUCHAR		 pRxMic = pCipherKey->RxMic;
#ifdef DBG
	UCHAR		CipherAlg = pCipherKey->CipherAlg;
#endif 

	
	offset = PAIRWISE_KEY_TABLE_BASE + (WCID * HW_KEY_ENTRY_SIZE);
#ifdef RT2860
	for (i=0; i<MAX_LEN_OF_PEER_KEY; i++)
	{
		RTMP_IO_WRITE8(pAd, offset + i, pKey[i]);
	}
#endif
#ifdef RT2870
	RTUSBMultiWrite(pAd, offset, &pCipherKey->Key[0], MAX_LEN_OF_PEER_KEY);
#endif 
	for (i=0; i<MAX_LEN_OF_PEER_KEY; i+=4)
	{
		UINT32 Value;
		RTMP_IO_READ32(pAd, offset + i, &Value);
	}

	offset += MAX_LEN_OF_PEER_KEY;

	
	if (pTxMic)
	{
#ifdef RT2860
		for (i=0; i<8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset+i, pTxMic[i]);
		}
#endif
#ifdef RT2870
		RTUSBMultiWrite(pAd, offset, &pCipherKey->TxMic[0], 8);
#endif 
	}
	offset += 8;
	if (pRxMic)
	{
#ifdef RT2860
		for (i=0; i<8; i++)
		{
			RTMP_IO_WRITE8(pAd, offset+i, pRxMic[i]);
		}
#endif
#ifdef RT2870
		RTUSBMultiWrite(pAd, offset, &pCipherKey->RxMic[0], 8);
#endif 
	}

	DBGPRINT(RT_DEBUG_TRACE,("AsicAddPairwiseKeyEntry: WCID #%d Alg=%s\n",WCID, CipherName[CipherAlg]));
	DBGPRINT(RT_DEBUG_TRACE,("	Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
		pKey[0],pKey[1],pKey[2],pKey[3],pKey[4],pKey[5],pKey[6],pKey[7],pKey[8],pKey[9],pKey[10],pKey[11],pKey[12],pKey[13],pKey[14],pKey[15]));
	if (pRxMic)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("	Rx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pRxMic[0],pRxMic[1],pRxMic[2],pRxMic[3],pRxMic[4],pRxMic[5],pRxMic[6],pRxMic[7]));
	}
	if (pTxMic)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("	Tx MIC Key = %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
			pTxMic[0],pTxMic[1],pTxMic[2],pTxMic[3],pTxMic[4],pTxMic[5],pTxMic[6],pTxMic[7]));
	}
}

VOID AsicRemovePairwiseKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 BssIdx,
	IN UCHAR		 Wcid)
{
	ULONG		WCIDAttri;
	USHORT		offset;

	
	offset = MAC_WCID_ATTRIBUTE_BASE + (Wcid * HW_WCID_ATTRI_SIZE);
	WCIDAttri = (BssIdx<<4) | PAIRWISEKEYTABLE;
	RTMP_IO_WRITE32(pAd, offset, WCIDAttri);
}

BOOLEAN AsicSendCommandToMcu(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Command,
	IN UCHAR		 Token,
	IN UCHAR		 Arg0,
	IN UCHAR		 Arg1)
{
	HOST_CMD_CSR_STRUC	H2MCmd;
	H2M_MAILBOX_STRUC	H2MMailbox;
	ULONG				i = 0;

	do
	{
		RTMP_IO_READ32(pAd, H2M_MAILBOX_CSR, &H2MMailbox.word);
		if (H2MMailbox.field.Owner == 0)
			break;

		RTMPusecDelay(2);
	} while(i++ < 100);

	if (i > 100)
	{
		{
#ifdef RT2860
			UINT32 Data;

			
			RTMP_IO_READ32(pAd, PBF_SYS_CTRL, &Data);
			Data |= 0x2;
			RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, Data);

			
			
			RTMPRingCleanUp(pAd, QID_AC_BK);
			RTMPRingCleanUp(pAd, QID_AC_BE);
			RTMPRingCleanUp(pAd, QID_AC_VI);
			RTMPRingCleanUp(pAd, QID_AC_VO);
			RTMPRingCleanUp(pAd, QID_HCCA);
			RTMPRingCleanUp(pAd, QID_MGMT);
			RTMPRingCleanUp(pAd, QID_RX);

			
			RTMP_IO_READ32(pAd, PBF_SYS_CTRL, &Data);
			Data &= 0xfffffffd;
			RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, Data);
#endif 
		DBGPRINT_ERR(("H2M_MAILBOX still hold by MCU. command fail\n"));
		}
		
#ifdef RT2870
		return FALSE;
#endif
	}

	H2MMailbox.field.Owner	  = 1;	   
	H2MMailbox.field.CmdToken = Token;
	H2MMailbox.field.HighByte = Arg1;
	H2MMailbox.field.LowByte  = Arg0;
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CSR, H2MMailbox.word);

	H2MCmd.word 			  = 0;
	H2MCmd.field.HostCommand  = Command;
	RTMP_IO_WRITE32(pAd, HOST_CMD_CSR, H2MCmd.word);

	if (Command != 0x80)
	{
	}

	return TRUE;
}

#ifdef RT2860
BOOLEAN AsicCheckCommanOk(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Command)
{
	UINT32	CmdStatus = 0, CID = 0, i;
	UINT32	ThisCIDMask = 0;

	i = 0;
	do
	{
		RTMP_IO_READ32(pAd, H2M_MAILBOX_CID, &CID);
		
		if ((CID & CID0MASK) == Command)
		{
			ThisCIDMask = CID0MASK;
			break;
		}
		else if ((((CID & CID1MASK)>>8) & 0xff) == Command)
		{
			ThisCIDMask = CID1MASK;
			break;
		}
		else if ((((CID & CID2MASK)>>16) & 0xff) == Command)
		{
			ThisCIDMask = CID2MASK;
			break;
		}
		else if ((((CID & CID3MASK)>>24) & 0xff) == Command)
		{
			ThisCIDMask = CID3MASK;
			break;
		}

		RTMPusecDelay(100);
		i++;
	}while (i < 200);

	
	RTMP_IO_READ32(pAd, H2M_MAILBOX_STATUS, &CmdStatus);

	
	if (i < 200)
	{
		
		if (((CmdStatus & ThisCIDMask) == 0x1) || ((CmdStatus & ThisCIDMask) == 0x100)
			|| ((CmdStatus & ThisCIDMask) == 0x10000) || ((CmdStatus & ThisCIDMask) == 0x1000000))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("--> AsicCheckCommanOk CID = 0x%x, CmdStatus= 0x%x \n", CID, CmdStatus));
			RTMP_IO_WRITE32(pAd, H2M_MAILBOX_STATUS, 0xffffffff);
			RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CID, 0xffffffff);
			return TRUE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("--> AsicCheckCommanFail1 CID = 0x%x, CmdStatus= 0x%x \n", CID, CmdStatus));
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("--> AsicCheckCommanFail2 Timeout Command = %d, CmdStatus= 0x%x \n", Command, CmdStatus));
	}
	
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_STATUS, 0xffffffff);
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CID, 0xffffffff);

	return FALSE;
}
#endif 


VOID	RTMPCheckRates(
	IN		PRTMP_ADAPTER	pAd,
	IN OUT	UCHAR			SupRate[],
	IN OUT	UCHAR			*SupRateLen)
{
	UCHAR	RateIdx, i, j;
	UCHAR	NewRate[12], NewRateLen;

	NewRateLen = 0;

	if (pAd->CommonCfg.PhyMode == PHY_11B)
		RateIdx = 4;
	else
		RateIdx = 12;

	
	for (i = 0; i < *SupRateLen; i++)
		for (j = 0; j < RateIdx; j++)
			if ((SupRate[i] & 0x7f) == RateIdTo500Kbps[j])
				NewRate[NewRateLen++] = SupRate[i];

	*SupRateLen = NewRateLen;
	NdisMoveMemory(SupRate, NewRate, NewRateLen);
}

BOOLEAN RTMPCheckChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		CentralChannel,
	IN UCHAR		Channel)
{
	UCHAR		k;
	UCHAR		UpperChannel = 0, LowerChannel = 0;
	UCHAR		NoEffectChannelinList = 0;

	
	if (CentralChannel < Channel)
	{
		UpperChannel = Channel;
		if (CentralChannel > 2)
			LowerChannel = CentralChannel - 2;
		else
			return FALSE;
	}
	else if (CentralChannel > Channel)
	{
		UpperChannel = CentralChannel + 2;
		LowerChannel = Channel;
	}

	for (k = 0;k < pAd->ChannelListNum;k++)
	{
		if (pAd->ChannelList[k].Channel == UpperChannel)
		{
			NoEffectChannelinList ++;
		}
		if (pAd->ChannelList[k].Channel == LowerChannel)
		{
			NoEffectChannelinList ++;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE,("Total Channel in Channel List = [%d]\n", NoEffectChannelinList));
	if (NoEffectChannelinList == 2)
		return TRUE;
	else
		return FALSE;
}


BOOLEAN 	RTMPCheckHt(
	IN	PRTMP_ADAPTER			pAd,
	IN	UCHAR					Wcid,
	IN 	HT_CAPABILITY_IE		*pHtCapability,
	IN 	ADD_HT_INFO_IE			*pAddHtInfo)
{
	if (Wcid >= MAX_LEN_OF_MAC_TABLE)
		return FALSE;

	
	if (pAd->CommonCfg.DesiredHtPhy.AmsduEnable)
		CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[Wcid], fCLIENT_STATUS_AMSDU_INUSED);
	
	if (pHtCapability->HtCapInfo.ShortGIfor20)
		CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[Wcid], fCLIENT_STATUS_SGI20_CAPABLE);
	if (pHtCapability->HtCapInfo.ShortGIfor40)
		CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[Wcid], fCLIENT_STATUS_SGI40_CAPABLE);
	if (pHtCapability->HtCapInfo.TxSTBC)
		CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[Wcid], fCLIENT_STATUS_TxSTBC_CAPABLE);
	if (pHtCapability->HtCapInfo.RxSTBC)
		CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[Wcid], fCLIENT_STATUS_RxSTBC_CAPABLE);
	if (pAd->CommonCfg.bRdg && pHtCapability->ExtHtCapInfo.RDGSupport)
	{
		CLIENT_STATUS_SET_FLAG(&pAd->MacTab.Content[Wcid], fCLIENT_STATUS_RDG_CAPABLE);
	}

	if (Wcid < MAX_LEN_OF_MAC_TABLE)
	{
		pAd->MacTab.Content[Wcid].MpduDensity = pHtCapability->HtCapParm.MpduDensity;
	}

	
	pAd->MlmeAux.HtCapability.MCSSet[4] = 0x1;
    switch (pAd->CommonCfg.RxStream)
	{
		case 1:
			pAd->MlmeAux.HtCapability.MCSSet[0] = 0xff;
			pAd->MlmeAux.HtCapability.MCSSet[1] = 0x00;
            pAd->MlmeAux.HtCapability.MCSSet[2] = 0x00;
            pAd->MlmeAux.HtCapability.MCSSet[3] = 0x00;
			break;
		case 2:
			pAd->MlmeAux.HtCapability.MCSSet[0] = 0xff;
			pAd->MlmeAux.HtCapability.MCSSet[1] = 0xff;
            pAd->MlmeAux.HtCapability.MCSSet[2] = 0x00;
            pAd->MlmeAux.HtCapability.MCSSet[3] = 0x00;
			break;
		case 3:
			pAd->MlmeAux.HtCapability.MCSSet[0] = 0xff;
			pAd->MlmeAux.HtCapability.MCSSet[1] = 0xff;
            pAd->MlmeAux.HtCapability.MCSSet[2] = 0xff;
            pAd->MlmeAux.HtCapability.MCSSet[3] = 0x00;
			break;
	}

	pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth = pAddHtInfo->AddHtInfo.RecomWidth & pAd->CommonCfg.DesiredHtPhy.ChannelWidth;

    DBGPRINT(RT_DEBUG_TRACE, ("RTMPCheckHt:: HtCapInfo.ChannelWidth=%d, RecomWidth=%d, DesiredHtPhy.ChannelWidth=%d, BW40MAvailForA/G=%d/%d, PhyMode=%d \n",
		pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth, pAddHtInfo->AddHtInfo.RecomWidth, pAd->CommonCfg.DesiredHtPhy.ChannelWidth,
		pAd->NicConfig2.field.BW40MAvailForA, pAd->NicConfig2.field.BW40MAvailForG, pAd->CommonCfg.PhyMode));

	pAd->MlmeAux.HtCapability.HtCapInfo.GF =  pHtCapability->HtCapInfo.GF &pAd->CommonCfg.DesiredHtPhy.GF;

	
	pAd->MlmeAux.HtCapability.HtCapInfo.AMsduSize =  pAd->CommonCfg.DesiredHtPhy.AmsduSize;
	pAd->MlmeAux.HtCapability.HtCapInfo.MimoPs =  pAd->CommonCfg.DesiredHtPhy.MimoPs;
	pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor20 =  (pAd->CommonCfg.DesiredHtPhy.ShortGIfor20) & (pHtCapability->HtCapInfo.ShortGIfor20);
	pAd->MlmeAux.HtCapability.HtCapInfo.ShortGIfor40 =  (pAd->CommonCfg.DesiredHtPhy.ShortGIfor40) & (pHtCapability->HtCapInfo.ShortGIfor40);
	pAd->MlmeAux.HtCapability.HtCapInfo.TxSTBC =  (pAd->CommonCfg.DesiredHtPhy.TxSTBC)&(pHtCapability->HtCapInfo.RxSTBC);
	pAd->MlmeAux.HtCapability.HtCapInfo.RxSTBC =  (pAd->CommonCfg.DesiredHtPhy.RxSTBC)&(pHtCapability->HtCapInfo.TxSTBC);
	pAd->MlmeAux.HtCapability.HtCapParm.MaxRAmpduFactor = pAd->CommonCfg.DesiredHtPhy.MaxRAmpduFactor;
    pAd->MlmeAux.HtCapability.HtCapParm.MpduDensity = pAd->CommonCfg.HtCapability.HtCapParm.MpduDensity;
	pAd->MlmeAux.HtCapability.ExtHtCapInfo.PlusHTC = pHtCapability->ExtHtCapInfo.PlusHTC;
	pAd->MacTab.Content[Wcid].HTCapability.ExtHtCapInfo.PlusHTC = pHtCapability->ExtHtCapInfo.PlusHTC;
	if (pAd->CommonCfg.bRdg)
	{
		pAd->MlmeAux.HtCapability.ExtHtCapInfo.RDGSupport = pHtCapability->ExtHtCapInfo.RDGSupport;
        pAd->MlmeAux.HtCapability.ExtHtCapInfo.PlusHTC = 1;
	}

    if (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_20)
        pAd->MlmeAux.HtCapability.MCSSet[4] = 0x0;  

	COPY_AP_HTSETTINGS_FROM_BEACON(pAd, pHtCapability);
	return TRUE;
}


VOID RTMPUpdateMlmeRate(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR	MinimumRate;
	UCHAR	ProperMlmeRate; 
	UCHAR	i, j, RateIdx = 12; 
	BOOLEAN	bMatch = FALSE;

	switch (pAd->CommonCfg.PhyMode)
	{
		case PHY_11B:
			ProperMlmeRate = RATE_11;
			MinimumRate = RATE_1;
			break;
		case PHY_11BG_MIXED:
		case PHY_11ABGN_MIXED:
		case PHY_11BGN_MIXED:
			if ((pAd->MlmeAux.SupRateLen == 4) &&
				(pAd->MlmeAux.ExtRateLen == 0))
				
				ProperMlmeRate = RATE_11;
			else
				ProperMlmeRate = RATE_24;

			if (pAd->MlmeAux.Channel <= 14)
				MinimumRate = RATE_1;
			else
				MinimumRate = RATE_6;
			break;
		case PHY_11A:
		case PHY_11N_2_4G:	
		case PHY_11GN_MIXED:
		case PHY_11AGN_MIXED:
		case PHY_11AN_MIXED:
		case PHY_11N_5G:
			ProperMlmeRate = RATE_24;
			MinimumRate = RATE_6;
			break;
		case PHY_11ABG_MIXED:
			ProperMlmeRate = RATE_24;
			if (pAd->MlmeAux.Channel <= 14)
			   MinimumRate = RATE_1;
			else
				MinimumRate = RATE_6;
			break;
		default: 
			ProperMlmeRate = RATE_1;
			MinimumRate = RATE_1;
			break;
	}

	for (i = 0; i < pAd->MlmeAux.SupRateLen; i++)
	{
		for (j = 0; j < RateIdx; j++)
		{
			if ((pAd->MlmeAux.SupRate[i] & 0x7f) == RateIdTo500Kbps[j])
			{
				if (j == ProperMlmeRate)
				{
					bMatch = TRUE;
					break;
				}
			}
		}

		if (bMatch)
			break;
	}

	if (bMatch == FALSE)
	{
		for (i = 0; i < pAd->MlmeAux.ExtRateLen; i++)
		{
			for (j = 0; j < RateIdx; j++)
			{
				if ((pAd->MlmeAux.ExtRate[i] & 0x7f) == RateIdTo500Kbps[j])
				{
					if (j == ProperMlmeRate)
					{
						bMatch = TRUE;
						break;
					}
				}
			}

			if (bMatch)
				break;
		}
	}

	if (bMatch == FALSE)
	{
		ProperMlmeRate = MinimumRate;
	}

	pAd->CommonCfg.MlmeRate = MinimumRate;
	pAd->CommonCfg.RtsRate = ProperMlmeRate;
	if (pAd->CommonCfg.MlmeRate >= RATE_6)
	{
		pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
		pAd->CommonCfg.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
		pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MODE = MODE_OFDM;
		pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
	}
	else
	{
		pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_CCK;
		pAd->CommonCfg.MlmeTransmit.field.MCS = pAd->CommonCfg.MlmeRate;
		pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MODE = MODE_CCK;
		pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MCS = pAd->CommonCfg.MlmeRate;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPUpdateMlmeRate ==>   MlmeTransmit = 0x%x  \n" , pAd->CommonCfg.MlmeTransmit.word));
}

CHAR RTMPMaxRssi(
	IN PRTMP_ADAPTER	pAd,
	IN CHAR				Rssi0,
	IN CHAR				Rssi1,
	IN CHAR				Rssi2)
{
	CHAR	larger = -127;

	if ((pAd->Antenna.field.RxPath == 1) && (Rssi0 != 0))
	{
		larger = Rssi0;
	}

	if ((pAd->Antenna.field.RxPath >= 2) && (Rssi1 != 0))
	{
		larger = max(Rssi0, Rssi1);
	}

	if ((pAd->Antenna.field.RxPath == 3) && (Rssi2 != 0))
	{
		larger = max(larger, Rssi2);
	}

	if (larger == -127)
		larger = 0;

	return larger;
}

#ifdef RT2870




VOID AsicSetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant)
{
	UINT32	Value;
	UINT32	x;

	if ((pAd->EepromAccess)										||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))	||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))	||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))			||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
		return;
	}

	
	if (Ant == 0)
	{
		
		RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
		x |= (EESK);
		RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

		RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &Value);
		Value &= ~(0x0808);
		RTMP_IO_WRITE32(pAd, GPIO_CTRL_CFG, Value);
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("AsicSetRxAnt, switch to main antenna\n"));
	}
	else
	{
		
		RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
		x &= ~(EESK);
		RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

		RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &Value);
		Value &= ~(0x0808);
		Value |= 0x08;
		RTMP_IO_WRITE32(pAd, GPIO_CTRL_CFG, Value);
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("AsicSetRxAnt, switch to aux antenna\n"));
	}
}
#endif


VOID AsicEvaluateRxAnt(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR	BBPR3 = 0;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS |
				fRTMP_ADAPTER_HALT_IN_PROGRESS |
				fRTMP_ADAPTER_RADIO_OFF |
				fRTMP_ADAPTER_NIC_NOT_EXIST |
				fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)
				|| OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE)
#ifdef RT2870
				|| (pAd->EepromAccess)
#endif
				)
			return;

#ifdef RT30xx
	
	
	
	if (pAd->NicConfig2.field.AntDiversity)
	{
		DBGPRINT(RT_DEBUG_TRACE,("AntDiv - before evaluate Pair1-Ant (%d,%d)\n",
			pAd->RxAnt.Pair1PrimaryRxAnt, pAd->RxAnt.Pair1SecondaryRxAnt));

		AsicSetRxAnt(pAd, pAd->RxAnt.Pair1SecondaryRxAnt);

		pAd->RxAnt.EvaluatePeriod = 1; 
		pAd->RxAnt.FirstPktArrivedWhenEvaluate = FALSE;
		pAd->RxAnt.RcvPktNumWhenEvaluate = 0;

		
		
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
			RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 100);
		else
			RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 300);
	}
	else
#endif
	{
		if (pAd->StaCfg.Psm == PWR_SAVE)
			return;

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
		BBPR3 &= (~0x18);
		if(pAd->Antenna.field.RxPath == 3)
		{
			BBPR3 |= (0x10);
		}
		else if(pAd->Antenna.field.RxPath == 2)
		{
			BBPR3 |= (0x8);
		}
		else if(pAd->Antenna.field.RxPath == 1)
		{
			BBPR3 |= (0x0);
		}
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);

#ifdef RT2860
		pAd->StaCfg.BBPR3 = BBPR3;
#endif
	}

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)
		)
	{
		ULONG	TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount +
								pAd->RalinkCounters.OneSecTxRetryOkCount +
								pAd->RalinkCounters.OneSecTxFailCount;

			
		if (TxTotalCnt > 50)
		{
			RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 20);
			pAd->Mlme.bLowThroughput = FALSE;
		}
		else
		{
			RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 300);
			pAd->Mlme.bLowThroughput = TRUE;
		}
	}
}


VOID AsicRxAntEvalTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER	*pAd = (RTMP_ADAPTER *)FunctionContext;
	UCHAR			BBPR3 = 0;
	CHAR			larger = -127, rssi0, rssi1, rssi2;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS	|
							fRTMP_ADAPTER_HALT_IN_PROGRESS	|
							fRTMP_ADAPTER_RADIO_OFF			|
							fRTMP_ADAPTER_NIC_NOT_EXIST)
							|| OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE)
#ifdef RT2870
							|| (pAd->EepromAccess)
#endif
							)
		return;

	{
#ifdef RT30xx
		if (pAd->NicConfig2.field.AntDiversity)
		{
			if ((pAd->RxAnt.RcvPktNumWhenEvaluate != 0) && (pAd->RxAnt.Pair1AvgRssi[pAd->RxAnt.Pair1SecondaryRxAnt] >= pAd->RxAnt.Pair1AvgRssi[pAd->RxAnt.Pair1PrimaryRxAnt]))
			{
				UCHAR			temp;

				
				
				
				
				
				temp = pAd->RxAnt.Pair1PrimaryRxAnt;
				pAd->RxAnt.Pair1PrimaryRxAnt = pAd->RxAnt.Pair1SecondaryRxAnt;
				pAd->RxAnt.Pair1SecondaryRxAnt = temp;

				pAd->RxAnt.Pair1LastAvgRssi = (pAd->RxAnt.Pair1AvgRssi[pAd->RxAnt.Pair1SecondaryRxAnt] >> 3);
				pAd->RxAnt.EvaluateStableCnt = 0;
			}
			else
			{
				
				AsicSetRxAnt(pAd, pAd->RxAnt.Pair1PrimaryRxAnt);
				pAd->RxAnt.EvaluateStableCnt ++;
			}

			pAd->RxAnt.EvaluatePeriod = 0; 

			DBGPRINT(RT_DEBUG_TRACE,("AsicRxAntEvalAction::After Eval(fix in #%d), <%d, %d>, RcvPktNumWhenEvaluate=%ld\n",
					pAd->RxAnt.Pair1PrimaryRxAnt, (pAd->RxAnt.Pair1AvgRssi[0] >> 3), (pAd->RxAnt.Pair1AvgRssi[1] >> 3), pAd->RxAnt.RcvPktNumWhenEvaluate));
		}
		else
#endif
		{
			if (pAd->StaCfg.Psm == PWR_SAVE)
				return;

			
			if (pAd->Mlme.bLowThroughput == TRUE)
			{
				rssi0 = pAd->StaCfg.RssiSample.LastRssi0;
				rssi1 = pAd->StaCfg.RssiSample.LastRssi1;
				rssi2 = pAd->StaCfg.RssiSample.LastRssi2;
			}
			else
			{
				rssi0 = pAd->StaCfg.RssiSample.AvgRssi0;
				rssi1 = pAd->StaCfg.RssiSample.AvgRssi1;
				rssi2 = pAd->StaCfg.RssiSample.AvgRssi2;
			}

			if(pAd->Antenna.field.RxPath == 3)
			{
				larger = max(rssi0, rssi1);

				if (larger > (rssi2 + 20))
					pAd->Mlme.RealRxPath = 2;
				else
					pAd->Mlme.RealRxPath = 3;
			}
			else if(pAd->Antenna.field.RxPath == 2)
			{
				if (rssi0 > (rssi1 + 20))
					pAd->Mlme.RealRxPath = 1;
				else
					pAd->Mlme.RealRxPath = 2;
			}

			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
			BBPR3 &= (~0x18);
			if(pAd->Mlme.RealRxPath == 3)
			{
				BBPR3 |= (0x10);
			}
			else if(pAd->Mlme.RealRxPath == 2)
			{
				BBPR3 |= (0x8);
			}
			else if(pAd->Mlme.RealRxPath == 1)
			{
				BBPR3 |= (0x0);
			}
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);
#ifdef RT2860
			pAd->StaCfg.BBPR3 = BBPR3;
#endif
		}
	}
}

VOID APSDPeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
		return;

	pAd->CommonCfg.TriggerTimerCount++;

}


VOID RTMPSetPiggyBack(
    IN PRTMP_ADAPTER    pAd,
    IN BOOLEAN          bPiggyBack)
{
	TX_LINK_CFG_STRUC  TxLinkCfg;

	RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);

	TxLinkCfg.field.TxCFAckEn = bPiggyBack;
	RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);
}


BOOLEAN RTMPCheckEntryEnableAutoRateSwitch(
	IN PRTMP_ADAPTER    pAd,
	IN PMAC_TABLE_ENTRY	pEntry)
{
	BOOLEAN		result = TRUE;

	{
		
		if (pEntry && (pEntry->ValidAsCLI) && (pEntry->Sst == SST_ASSOC))
		{
			result = pAd->StaCfg.bAutoTxRateSwitch;
		}
		else
			result = FALSE;
	}

	return result;
}


BOOLEAN RTMPAutoRateSwitchCheck(
	IN PRTMP_ADAPTER    pAd)
{
	if (pAd->StaCfg.bAutoTxRateSwitch)
		return TRUE;

	return FALSE;
}



UCHAR RTMPStaFixedTxMode(
	IN PRTMP_ADAPTER    pAd,
	IN PMAC_TABLE_ENTRY	pEntry)
{
	UCHAR	tx_mode = FIXED_TXMODE_HT;

	tx_mode = (UCHAR)pAd->StaCfg.DesiredTransmitSetting.field.FixedTxMode;

	return tx_mode;
}


VOID RTMPUpdateLegacyTxSetting(
		UCHAR				fixed_tx_mode,
		PMAC_TABLE_ENTRY	pEntry)
{
	HTTRANSMIT_SETTING TransmitSetting;

	if (fixed_tx_mode == FIXED_TXMODE_HT)
		return;

	TransmitSetting.word = 0;

	TransmitSetting.field.MODE = pEntry->HTPhyMode.field.MODE;
	TransmitSetting.field.MCS = pEntry->HTPhyMode.field.MCS;

	if (fixed_tx_mode == FIXED_TXMODE_CCK)
	{
		TransmitSetting.field.MODE = MODE_CCK;
		
		if (TransmitSetting.field.MCS > MCS_3)
			TransmitSetting.field.MCS = MCS_3;
	}
	else
	{
		TransmitSetting.field.MODE = MODE_OFDM;
		
		if (TransmitSetting.field.MCS > MCS_7)
			TransmitSetting.field.MCS = MCS_7;
	}

	if (pEntry->HTPhyMode.field.MODE >= TransmitSetting.field.MODE)
	{
		pEntry->HTPhyMode.word = TransmitSetting.word;
		DBGPRINT(RT_DEBUG_TRACE, ("RTMPUpdateLegacyTxSetting : wcid-%d, MODE=%s, MCS=%d \n",
				pEntry->Aid, GetPhyMode(pEntry->HTPhyMode.field.MODE), pEntry->HTPhyMode.field.MCS));
	}
}


VOID AsicStaBbpTuning(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR	OrigR66Value = 0, R66;
	CHAR	Rssi;

	
	if (pAd->MACVersion == 0x28600100)
		return;

	
	
	
	if (pAd->Mlme.CntlMachine.CurrState != CNTL_IDLE)  
		return;

	if ((pAd->OpMode == OPMODE_STA)
		&& (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)
			)
		&& !(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
#ifdef RT2860
		&& (pAd->bPCIclkOff == FALSE))
#endif
#ifdef RT2870
		)
#endif
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R66, &OrigR66Value);
		R66 = OrigR66Value;

		if (pAd->Antenna.field.RxPath > 1)
			Rssi = (pAd->StaCfg.RssiSample.AvgRssi0 + pAd->StaCfg.RssiSample.AvgRssi1) >> 1;
		else
			Rssi = pAd->StaCfg.RssiSample.AvgRssi0;

		if (pAd->LatchRfRegs.Channel <= 14)
		{	
#ifdef RT2870
			
			
			if (IS_RT30xx(pAd))
			{
				if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
				{
					R66 = 0x1C + 2*GET_LNA_GAIN(pAd) + 0x20;
					if (OrigR66Value != R66)
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
				}
				else
				{
					R66 = 0x1C + 2*GET_LNA_GAIN(pAd);
					if (OrigR66Value != R66)
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
				}
			}
			else
#endif 
			{
				if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
				{
					R66 = (0x2E + GET_LNA_GAIN(pAd)) + 0x10;
					if (OrigR66Value != R66)
					{
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
					}
				}
				else
				{
					R66 = 0x2E + GET_LNA_GAIN(pAd);
					if (OrigR66Value != R66)
					{
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
					}
				}
			}
		}
		else
		{	
			if (pAd->CommonCfg.BBPCurrentBW == BW_20)
			{
				if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
				{
					R66 = 0x32 + (GET_LNA_GAIN(pAd)*5)/3 + 0x10;
					if (OrigR66Value != R66)
					{
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
					}
				}
				else
				{
					R66 = 0x32 + (GET_LNA_GAIN(pAd)*5)/3;
					if (OrigR66Value != R66)
					{
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
					}
				}
			}
			else
			{
				if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
				{
					R66 = 0x3A + (GET_LNA_GAIN(pAd)*5)/3 + 0x10;
					if (OrigR66Value != R66)
					{
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
					}
				}
				else
				{
					R66 = 0x3A + (GET_LNA_GAIN(pAd)*5)/3;
					if (OrigR66Value != R66)
					{
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
					}
				}
			}
		}


	}
}

#ifdef RT2860
VOID AsicResetFromDMABusy(
	IN PRTMP_ADAPTER pAd)
{
	UINT32		Data;
	BOOLEAN		bCtrl = FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("--->  AsicResetFromDMABusy  !!!!!!!!!!!!!!!!!!!!!!! \n"));

	
	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP);
	if (RTMP_TEST_PSFLAG(pAd, fRTMP_PS_SET_PCI_CLK_OFF_COMMAND))
	{
		DBGPRINT(RT_DEBUG_TRACE,("AsicResetFromDMABusy==>\n"));
		RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_HALT);
		RTMPusecDelay(6000);
		pAd->bPCIclkOff = FALSE;
		bCtrl = TRUE;
	}
	
	RTMP_IO_READ32(pAd, PBF_SYS_CTRL, &Data);
	Data |= 0x2;
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, Data);

	
	
	RTMPRingCleanUp(pAd, QID_AC_BK);
	RTMPRingCleanUp(pAd, QID_AC_BE);
	RTMPRingCleanUp(pAd, QID_AC_VI);
	RTMPRingCleanUp(pAd, QID_AC_VO);
	RTMPRingCleanUp(pAd, QID_HCCA);
	RTMPRingCleanUp(pAd, QID_MGMT);
	RTMPRingCleanUp(pAd, QID_RX);

	
	RTMP_IO_READ32(pAd, PBF_SYS_CTRL, &Data);
	Data &= 0xfffffffd;
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, Data);

	
	if ((bCtrl == TRUE) && (pAd->StaCfg.bRadio == FALSE))
		RTMPPCIeLinkCtrlSetting(pAd, 3);

	RTMP_SET_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_HALT_IN_PROGRESS);
	DBGPRINT(RT_DEBUG_TRACE, ("<---  AsicResetFromDMABusy !!!!!!!!!!!!!!!!!!!!!!!  \n"));
}

VOID AsicResetBBP(
	IN PRTMP_ADAPTER pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("--->  Asic HardReset BBP  !!!!!!!!!!!!!!!!!!!!!!! \n"));

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x0);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x2);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0xc);

	
	NICRestoreBBPValue(pAd);
	DBGPRINT(RT_DEBUG_TRACE, ("<---  Asic HardReset BBP !!!!!!!!!!!!!!!!!!!!!!!  \n"));
}

VOID AsicResetMAC(
	IN PRTMP_ADAPTER pAd)
{
	ULONG		Data;

	DBGPRINT(RT_DEBUG_TRACE, ("--->  AsicResetMAC   !!!! \n"));
	RTMP_IO_READ32(pAd, PBF_SYS_CTRL, &Data);
	Data |= 0x4;
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, Data);
	Data &= 0xfffffffb;
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, Data);

	DBGPRINT(RT_DEBUG_TRACE, ("<---  AsicResetMAC   !!!! \n"));
}

VOID AsicResetPBF(
	IN PRTMP_ADAPTER pAd)
{
	ULONG		Value1, Value2;
	ULONG		Data;

	RTMP_IO_READ32(pAd, TXRXQ_PCNT, &Value1);
	RTMP_IO_READ32(pAd, PBF_DBG, &Value2);

	Value2 &= 0xff;
	
	if ((Value1 + Value2) < 0xff)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("--->  Asic HardReset PBF !!!! \n"));
		RTMP_IO_READ32(pAd, PBF_SYS_CTRL, &Data);
		Data |= 0x8;
		RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, Data);
		Data &= 0xfffffff7;
		RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, Data);

		DBGPRINT(RT_DEBUG_TRACE, ("<---  Asic HardReset PBF !!!! \n"));
	}
}
#endif 

VOID RTMPSetAGCInitValue(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			BandWidth)
{
	UCHAR	R66 = 0x30;

	if (pAd->LatchRfRegs.Channel <= 14)
	{	
		R66 = 0x2E + GET_LNA_GAIN(pAd);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
	}
	else
	{	
		if (BandWidth == BW_20)
		{
			R66 = (UCHAR)(0x32 + (GET_LNA_GAIN(pAd)*5)/3);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
		}
		else
		{
			R66 = (UCHAR)(0x3A + (GET_LNA_GAIN(pAd)*5)/3);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
		}
	}

}

VOID AsicTurnOffRFClk(
	IN PRTMP_ADAPTER pAd,
	IN	UCHAR		Channel)
{

	
	UINT32			R1 = 0, R2 = 0, R3 = 0;
	UCHAR			index;
	RTMP_RF_REGS	*RFRegTable;

	
	if (IS_RT3090(pAd))
	{
		RT30xxLoadRFSleepModeSetup(pAd);  
		return;
	}

	RFRegTable = RF2850RegTable;

	switch (pAd->RfIcType)
	{
		case RFIC_2820:
		case RFIC_2850:
		case RFIC_2720:
		case RFIC_2750:

			for (index = 0; index < NUM_OF_2850_CHNL; index++)
			{
				if (Channel == RFRegTable[index].Channel)
				{
					R1 = RFRegTable[index].R1 & 0xffffdfff;
					R2 = RFRegTable[index].R2 & 0xfffbffff;
					R3 = RFRegTable[index].R3 & 0xfff3ffff;

					RTMP_RF_IO_WRITE32(pAd, R1);
					RTMP_RF_IO_WRITE32(pAd, R2);

					
					
					
					if (1)
					{
						RTMP_RF_IO_WRITE32(pAd, R3);

						DBGPRINT(RT_DEBUG_TRACE, ("AsicTurnOffRFClk#%d(RF=%d, ) , R2=0x%08x,  R3 = 0x%08x \n",
							Channel, pAd->RfIcType, R2, R3));
					}
					else
						DBGPRINT(RT_DEBUG_TRACE, ("AsicTurnOffRFClk#%d(RF=%d, ) , R2=0x%08x \n",
							Channel, pAd->RfIcType, R2));
					break;
				}
			}
			break;

		default:
			break;
	}
}


VOID AsicTurnOnRFClk(
	IN PRTMP_ADAPTER pAd,
	IN	UCHAR			Channel)
{

	
	UINT32			R1 = 0, R2 = 0, R3 = 0;
	UCHAR			index;
	RTMP_RF_REGS	*RFRegTable;

	
	if (IS_RT3090(pAd))
		return;

	RFRegTable = RF2850RegTable;

	switch (pAd->RfIcType)
	{
		case RFIC_2820:
		case RFIC_2850:
		case RFIC_2720:
		case RFIC_2750:

			for (index = 0; index < NUM_OF_2850_CHNL; index++)
			{
				if (Channel == RFRegTable[index].Channel)
				{
					R3 = pAd->LatchRfRegs.R3;
					R3 &= 0xfff3ffff;
					R3 |= 0x00080000;
					RTMP_RF_IO_WRITE32(pAd, R3);

					R1 = RFRegTable[index].R1;
					RTMP_RF_IO_WRITE32(pAd, R1);

					R2 = RFRegTable[index].R2;
					if (pAd->Antenna.field.TxPath == 1)
					{
						R2 |= 0x4000;	
					}

					if (pAd->Antenna.field.RxPath == 2)
					{
						R2 |= 0x40;	
					}
					else if (pAd->Antenna.field.RxPath == 1)
					{
						R2 |= 0x20040;	
					}
					RTMP_RF_IO_WRITE32(pAd, R2);

					break;
				}
			}
			break;

		default:
			break;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("AsicTurnOnRFClk#%d(RF=%d, ) , R2=0x%08x\n",
		Channel,
		pAd->RfIcType,
		R2));
}

