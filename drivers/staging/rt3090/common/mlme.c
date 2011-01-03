

#include "../rt_config.h"
#include <stdarg.h>

UCHAR	CISCO_OUI[] = {0x00, 0x40, 0x96};

UCHAR	WPA_OUI[] = {0x00, 0x50, 0xf2, 0x01};
UCHAR	RSN_OUI[] = {0x00, 0x0f, 0xac};
UCHAR	WAPI_OUI[] = {0x00, 0x14, 0x72};
UCHAR   WME_INFO_ELEM[]  = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};
UCHAR   WME_PARM_ELEM[] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};
UCHAR	Ccx2QosInfo[] = {0x00, 0x40, 0x96, 0x04};
UCHAR   RALINK_OUI[]  = {0x00, 0x0c, 0x43};
UCHAR   BROADCOM_OUI[]  = {0x00, 0x90, 0x4c};
UCHAR   WPS_OUI[] = {0x00, 0x50, 0xf2, 0x04};
#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11_N_SUPPORT
UCHAR	PRE_N_HT_OUI[]	= {0x00, 0x90, 0x4c};
#endif 
#endif 

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

#ifdef DOT11_N_SUPPORT
UCHAR RateSwitchTable11N1S[] = {

    0x0c, 0x0a,  0,  0,  0,						
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 25, 45,
    0x03, 0x21,  0, 20, 35,
    0x04, 0x21,  1, 20, 35,
    0x05, 0x21,  2, 20, 35,
    0x06, 0x21,  3, 15, 35,
    0x07, 0x21,  4, 15, 30,
    0x08, 0x21,  5, 10, 25,
    0x09, 0x21,  6,  8, 14,
    0x0a, 0x21,  7,  8, 14,
    0x0b, 0x23,  7,  8, 14,
};

UCHAR RateSwitchTable11N2S[] = {

    0x0e, 0x0c,  0,  0,  0,						
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 25, 45,
    0x03, 0x21,  0, 20, 35,
    0x04, 0x21,  1, 20, 35,
    0x05, 0x21,  2, 20, 35,
    0x06, 0x21,  3, 15, 35,
    0x07, 0x21,  4, 15, 30,
    0x08, 0x20, 11, 15, 30,
    0x09, 0x20, 12, 15, 30,
    0x0a, 0x20, 13,  8, 20,
    0x0b, 0x20, 14,  8, 20,
    0x0c, 0x20, 15,  8, 25,
    0x0d, 0x22, 15,  8, 15,
};

UCHAR RateSwitchTable11N3S[] = {

    0x0b, 0x00,  0,  0,  0,	
    0x00, 0x21,  0, 30, 101,
    0x01, 0x21,  1, 20, 50,
    0x02, 0x21,  2, 20, 50,
    0x03, 0x21,  3, 15, 50,
    0x04, 0x21,  4, 15, 30,
    0x05, 0x20, 11, 15, 30,	
    0x06, 0x20, 12, 15, 30,	
    0x07, 0x20, 13,  8, 20,	
    0x08, 0x20, 14,  8, 20,	
    0x09, 0x20, 15,  8, 25,	
    0x0a, 0x22, 15,  8, 25,	
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

    0x0c, 0x0a,  0,  0,  0,						
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 25, 45,
    0x03, 0x21,  0, 20, 35,
    0x04, 0x21,  1, 20, 35,
    0x05, 0x21,  2, 20, 35,
    0x06, 0x21,  3, 15, 35,
    0x07, 0x21,  4, 15, 30,
    0x08, 0x21,  5, 10, 25,
    0x09, 0x21,  6,  8, 14,
    0x0a, 0x21,  7,  8, 14,
    0x0b, 0x23,  7,  8, 14,
};

UCHAR RateSwitchTable11BGN2S[] = {

    0x0e, 0x0c,  0,  0,  0,						
    0x00, 0x00,  0, 40, 101,
    0x01, 0x00,  1, 40, 50,
    0x02, 0x00,  2, 25, 45,
    0x03, 0x21,  0, 20, 35,
    0x04, 0x21,  1, 20, 35,
    0x05, 0x21,  2, 20, 35,
    0x06, 0x21,  3, 15, 35,
    0x07, 0x21,  4, 15, 30,
    0x08, 0x20, 11, 15, 30,
    0x09, 0x20, 12, 15, 30,
    0x0a, 0x20, 13,  8, 20,
    0x0b, 0x20, 14,  8, 20,
    0x0c, 0x20, 15,  8, 25,
    0x0d, 0x22, 15,  8, 15,
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
#endif 


extern UCHAR	 OfdmRateToRxwiMCS[];


ULONG BasicRateMask[12]				= {0xfffff001 , 0xfffff003 , 0xfffff007 , 0xfffff00f ,
									  0xfffff01f 	 , 0xfffff03f 	  , 0xfffff07f  , 0xfffff0ff ,
									  0xfffff1ff 	 , 0xfffff3ff 	  , 0xfffff7ff  , 0xffffffff };

UCHAR BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};





CHAR RssiSafeLevelForTxRate[] ={  -92, -91, -90, -87, -88, -86, -85, -83, -81, -78, -72, -71, -40, -40 };

UCHAR  RateIdToMbps[]	 = { 1, 2, 5, 11, 6, 9, 12, 18, 24, 36, 48, 54, 72, 100};
USHORT RateIdTo500Kbps[] = { 2, 4, 11, 22, 12, 18, 24, 36, 48, 72, 96, 108, 144, 200};

UCHAR  SsidIe	 = IE_SSID;
UCHAR  SupRateIe = IE_SUPP_RATES;
UCHAR  ExtRateIe = IE_EXT_SUPP_RATES;
#ifdef DOT11_N_SUPPORT
UCHAR  HtCapIe = IE_HT_CAP;
UCHAR  AddHtInfoIe = IE_ADD_HT;
UCHAR  NewExtChanIe = IE_SECONDARY_CH_OFFSET;
#ifdef DOT11N_DRAFT3
UCHAR  ExtHtCapIe = IE_EXT_CAPABILITY;
#endif 
#endif 
UCHAR  ErpIe	 = IE_ERP;
UCHAR  DsIe	 = IE_DS_PARM;
UCHAR  TimIe	 = IE_TIM;
UCHAR  WpaIe	 = IE_WPA;
UCHAR  Wpa2Ie	 = IE_WPA2;
UCHAR  IbssIe	 = IE_IBSS_PARM;
UCHAR  Ccx2Ie	 = IE_CCX_V2;
UCHAR  WapiIe	 = IE_WAPI;

extern UCHAR	WPA_OUI[];

UCHAR	SES_OUI[] = {0x00, 0x90, 0x4c};

UCHAR	ZeroSsid[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};



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

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			BssTableInit(&pAd->ScanTab);

			
			AssocStateMachineInit(pAd, &pAd->Mlme.AssocMachine, pAd->Mlme.AssocFunc);
			AuthStateMachineInit(pAd, &pAd->Mlme.AuthMachine, pAd->Mlme.AuthFunc);
			AuthRspStateMachineInit(pAd, &pAd->Mlme.AuthRspMachine, pAd->Mlme.AuthRspFunc);
			SyncStateMachineInit(pAd, &pAd->Mlme.SyncMachine, pAd->Mlme.SyncFunc);

#ifdef QOS_DLS_SUPPORT
			DlsStateMachineInit(pAd, &pAd->Mlme.DlsMachine, pAd->Mlme.DlsFunc);
#endif 



			
			
			MlmeCntlInit(pAd, &pAd->Mlme.CntlMachine, NULL);
		}
#endif 


		WpaStateMachineInit(pAd, &pAd->Mlme.WpaMachine, pAd->Mlme.WpaFunc);


		ActionStateMachineInit(pAd, &pAd->Mlme.ActMachine, pAd->Mlme.ActFunc);

		
		RTMPInitTimer(pAd, &pAd->Mlme.PeriodicTimer, GET_TIMER_FUNCTION(MlmePeriodicExec), pAd, TRUE);

		
		RTMPSetTimer(&pAd->Mlme.PeriodicTimer, MLME_TASK_EXEC_INTV);

		
		RTMPInitTimer(pAd, &pAd->Mlme.RxAntEvalTimer, GET_TIMER_FUNCTION(AsicRxAntEvalTimeout), pAd, FALSE);


#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
#ifdef RTMP_PCI_SUPPORT
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE))
			{
			    
				RTMPInitTimer(pAd, &pAd->Mlme.PsPollTimer, GET_TIMER_FUNCTION(PsPollWakeExec), pAd, FALSE);
				RTMPInitTimer(pAd, &pAd->Mlme.RadioOnOffTimer, GET_TIMER_FUNCTION(RadioOnExec), pAd, FALSE);
			}
#endif 

			RTMPInitTimer(pAd, &pAd->Mlme.LinkDownTimer, GET_TIMER_FUNCTION(LinkDownExec), pAd, FALSE);

		}
#endif 

	} while (FALSE);

	DBGPRINT(RT_DEBUG_TRACE, ("<-- MLME Initialize\n"));

	return Status;
}


VOID MlmeHandler(
	IN PRTMP_ADAPTER pAd)
{
	MLME_QUEUE_ELEM		   *Elem = NULL;
#ifdef APCLI_SUPPORT
	SHORT apcliIfIndex;
#endif 

	
	

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

#ifdef RALINK_ATE
		if(ATE_ON(pAd))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("The driver is in ATE mode now in MlmeHandler\n"));
			break;
		}
#endif 

		
		if (MlmeDequeue(&pAd->Mlme.Queue, &Elem))
		{

			
			switch (Elem->Machine)
			{
				
#ifdef CONFIG_STA_SUPPORT
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

#ifdef QOS_DLS_SUPPORT
				case DLS_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.DlsMachine, Elem);
					break;
#endif 

#endif 

				case ACTION_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.ActMachine, Elem);
					break;

				case WPA_STATE_MACHINE:
					StateMachinePerformAction(pAd, &pAd->Mlme.WpaMachine, Elem);
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
	BOOLEAN		  Cancelled;

	DBGPRINT(RT_DEBUG_TRACE, ("==> MlmeHalt\n"));

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		
		AsicDisableSync(pAd);
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef QOS_DLS_SUPPORT
		UCHAR		i;
#endif 
		
		RTMPCancelTimer(&pAd->MlmeAux.AssocTimer,		&Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.ReassocTimer,		&Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.DisassocTimer,	&Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.AuthTimer,		&Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer,		&Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer,		&Cancelled);


#ifdef RTMP_MAC_PCI
	    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE)
			&&(pAd->StaCfg.PSControl.field.EnableNewPS == TRUE))
	    {
		    RTMPCancelTimer(&pAd->Mlme.PsPollTimer,		&Cancelled);
		    RTMPCancelTimer(&pAd->Mlme.RadioOnOffTimer,		&Cancelled);
		}
#endif 

#ifdef QOS_DLS_SUPPORT
		for (i=0; i<MAX_NUM_OF_DLS_ENTRY; i++)
		{
			RTMPCancelTimer(&pAd->StaCfg.DLSEntry[i].Timer, &Cancelled);
		}
#endif 
		RTMPCancelTimer(&pAd->Mlme.LinkDownTimer,		&Cancelled);

	}
#endif 

	RTMPCancelTimer(&pAd->Mlme.PeriodicTimer,		&Cancelled);
	RTMPCancelTimer(&pAd->Mlme.RxAntEvalTimer,		&Cancelled);



	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_CHIP_OP *pChipOps = &pAd->chipOps;

		
		RTMPSetLED(pAd, LED_HALT);
		RTMPSetSignalLED(pAd, -100);	

		if (pChipOps->AsicHaltAction)
			pChipOps->AsicHaltAction(pAd);
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
	pAd->RalinkCounters.OneSecReceivedByteCount = 0;
	pAd->RalinkCounters.OneSecTransmittedByteCount = 0;

	
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



#define ADHOC_BEACON_LOST_TIME		(8*OS_HZ)  
VOID MlmePeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	ULONG			TxTotalCnt;
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;
	SHORT	realavgrssi;

#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_MAC_PCI
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
	    
		
		if ((pAd->Mlme.PeriodicRound % (MLME_TASK_EXEC_MULTIPLE * 2) == 0) && (pAd->StaCfg.bHardwareRadio == TRUE) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
			)
		{
			UINT32				data = 0;

			
#ifndef RT3090
			RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &data);
#endif 

#ifdef RT3090


if ((pAd->OpMode == OPMODE_STA) && (IDLE_ON(pAd))
	&& (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
	&& (pAd->StaCfg.PSControl.field.EnablePSinIdle == TRUE))
	{
	
	RTMPPCIeLinkCtrlValueRestore(pAd, 0);
	RTMP_IO_FORCE_READ32(pAd, GPIO_CTRL_CFG, &data);
	RTMPPCIeLinkCtrlSetting(pAd, 3);
	}
else
	RTMP_IO_FORCE_READ32(pAd, GPIO_CTRL_CFG, &data);
#endif 


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
#endif 

	
	
	if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_RADIO_OFF |
								fRTMP_ADAPTER_RADIO_MEASUREMENT |
								fRTMP_ADAPTER_RESET_IN_PROGRESS))))
		return;

	RTMP_MLME_PRE_SANITY_CHECK(pAd);

#ifdef RALINK_ATE
	
	if (ATE_ON(pAd))
	{
		if (pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE != (MLME_TASK_EXEC_MULTIPLE - 1))
	{
			pAd->Mlme.PeriodicRound ++;
			return;
		}
	}
#endif 

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
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
#endif 

	pAd->bUpdateBcnCntDone = FALSE;


	pAd->Mlme.PeriodicRound ++;


	
	if ((pAd->Mlme.PeriodicRound % 5 == 0) && RTMPAutoRateSwitchCheck(pAd))
	{
#ifdef CONFIG_STA_SUPPORT
		
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)
					)
				&& (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE)))
				MlmeDynamicTxRateSwitching(pAd);
		}
#endif 
	}

	
	if (pAd->Mlme.PeriodicRound %MLME_TASK_EXEC_MULTIPLE == 0)
	{
                pAd->Mlme.OneSecPeriodicRound ++;

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
			
			
            NICUpdateRawCounters(pAd);
			if (pAd->ate.bRxFER == 1)
			{
				pAd->ate.RxTotalCnt += pAd->ate.RxCntPerSec;
			    ate_print(KERN_EMERG "MlmePeriodicExec: Rx packet cnt = %d/%d\n", pAd->ate.RxCntPerSec, pAd->ate.RxTotalCnt);
				pAd->ate.RxCntPerSec = 0;

				if (pAd->ate.RxAntennaSel == 0)
					ate_print(KERN_EMERG "MlmePeriodicExec: Rx AvgRssi0=%d, AvgRssi1=%d, AvgRssi2=%d\n\n",
						pAd->ate.AvgRssi0, pAd->ate.AvgRssi1, pAd->ate.AvgRssi2);
				else
					ate_print(KERN_EMERG "MlmePeriodicExec: Rx AvgRssi=%d\n\n", pAd->ate.AvgRssi0);
			}
			MlmeResetRalinkCounters(pAd);



			return;
	}
#endif 



		

		
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


#ifdef DOT11_N_SUPPORT
		
		ORIBATimerTimeout(pAd);
#endif 

		
		
		
	
	
	
	

		
#ifdef ANT_DIVERSITY_SUPPORT
		if ((pAd->NicConfig2.field.AntDiversity) &&
			(pAd->CommonCfg.bRxAntDiversity == ANT_DIVERSITY_ENABLE) &&
			(!pAd->EepromAccess))
			AsicAntennaSelect(pAd, pAd->MlmeAux.Channel);
		else if(pAd->CommonCfg.bRxAntDiversity == ANT_FIX_ANT1 || pAd->CommonCfg.bRxAntDiversity == ANT_FIX_ANT2)
		{
#ifdef CONFIG_STA_SUPPORT
			realavgrssi = (pAd->RxAnt.Pair1AvgRssi[pAd->RxAnt.Pair1PrimaryRxAnt] >> 3);
#endif 
			DBGPRINT(RT_DEBUG_TRACE,("Ant-realrssi0(%d), Lastrssi0(%d), EvaluateStableCnt=%d\n", realavgrssi, pAd->RxAnt.Pair1LastAvgRssi, pAd->RxAnt.EvaluateStableCnt));
		}
		else
#endif 
		{
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
		}

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			STAMlmePeriodicExec(pAd);
#endif 

		MlmeResetRalinkCounters(pAd);

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
#ifdef RTMP_MAC_PCI
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
#endif 

		RTMP_MLME_HANDLER(pAd);
	}


	pAd->bUpdateBcnCntDone = FALSE;
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

#ifdef CONFIG_STA_SUPPORT
		if ((pAd->OpMode == OPMODE_STA) && ADHOC_ON(pAd))
		{
#ifdef DOT11_N_SUPPORT
			if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) &&
				(pEntry->HTCapability.MCSSet[0] == 0xff) &&
				((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->Antenna.field.TxPath == 1)))
			{
				*ppTable = RateSwitchTable11N1S;
				*pTableSize = RateSwitchTable11N1S[0];
				*pInitTxRateIdx = RateSwitchTable11N1S[1];

			}
			else if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) &&
					(pEntry->HTCapability.MCSSet[0] == 0xff) &&
					(pEntry->HTCapability.MCSSet[1] == 0xff) &&
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
#endif 
				if ((pEntry->RateLen == 4)
#ifdef DOT11_N_SUPPORT
					&& (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif 
					)
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
#endif 

#ifdef DOT11_N_SUPPORT
		
		
		if (((pEntry->RateLen == 12) || (pAd->OpMode == OPMODE_STA)) && (pEntry->HTCapability.MCSSet[0] == 0xff) &&
			((pEntry->HTCapability.MCSSet[1] == 0x00) || (pAd->CommonCfg.TxStream == 1)))
		{
			*ppTable = RateSwitchTable11BGN1S;
			*pTableSize = RateSwitchTable11BGN1S[0];
			*pInitTxRateIdx = RateSwitchTable11BGN1S[1];

			break;
		}

		
		
		if (((pEntry->RateLen == 12) || (pAd->OpMode == OPMODE_STA)) && (pEntry->HTCapability.MCSSet[0] == 0xff) &&
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
#endif 
		
		if ((pEntry->RateLen == 4 || pAd->CommonCfg.PhyMode==PHY_11B)
#ifdef DOT11_N_SUPPORT
		
		
#endif 
			)
		{
			*ppTable = RateSwitchTable11B;
			*pTableSize = RateSwitchTable11B[0];
			*pInitTxRateIdx = RateSwitchTable11B[1];

			break;
		}

		
		if ((pEntry->RateLen > 8)
#ifdef DOT11_N_SUPPORT
			&& (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif 
			)
		{
			*ppTable = RateSwitchTable11BG;
			*pTableSize = RateSwitchTable11BG[0];
			*pInitTxRateIdx = RateSwitchTable11BG[1];

			break;
		}

		
		if ((pEntry->RateLen == 8)
#ifdef DOT11_N_SUPPORT
			&& (pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0)
#endif 
			)
		{
			*ppTable = RateSwitchTable11G;
			*pTableSize = RateSwitchTable11G[0];
			*pInitTxRateIdx = RateSwitchTable11G[1];

			break;
		}
#ifdef DOT11_N_SUPPORT
#endif 

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
#ifdef DOT11_N_SUPPORT
			
			if ((pEntry->HTCapability.MCSSet[0] == 0) && (pEntry->HTCapability.MCSSet[1] == 0))
#endif 
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
#ifdef DOT11_N_SUPPORT
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
#endif 
			DBGPRINT_RAW(RT_DEBUG_ERROR,("DRS: unkown mode (SupRateLen=%d, ExtRateLen=%d, MCSSet[0]=0x%x, MCSSet[1]=0x%x)\n",
				pAd->StaActive.SupRateLen, pAd->StaActive.ExtRateLen, pAd->StaActive.SupportedPhyInfo.MCSSet[0], pAd->StaActive.SupportedPhyInfo.MCSSet[1]));
		}
#endif 
	} while(FALSE);
}


#ifdef CONFIG_STA_SUPPORT
VOID STAMlmePeriodicExec(
	PRTMP_ADAPTER pAd)
{
	ULONG			    TxTotalCnt;
	int	i;




	
#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	return;
#endif 

#ifdef RALINK_ATE
	
	ASSERT(!(ATE_ON(pAd)));
	if (ATE_ON(pAd))
		return;
#endif 

#ifdef PCIE_PS_SUPPORT


if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_PCIE_DEVICE))
	{
	if ((pAd->OpMode == OPMODE_STA) && (IDLE_ON(pAd))
		&& (pAd->Mlme.SyncMachine.CurrState == SYNC_IDLE)
		&& (pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE)
		&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)))
		{
		if (IS_RT3090(pAd)|| IS_RT3572(pAd) || IS_RT3390(pAd))
			{
			if (pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
				{
				DBGPRINT(RT_DEBUG_TRACE, ("%s::%d\n",__FUNCTION__,__LINE__));

				RT28xxPciAsicRadioOff(pAd, GUI_IDLE_POWER_SAVE, 0);
				}
			else
				{
				DBGPRINT(RT_DEBUG_TRACE, ("%s::%d\n",__FUNCTION__,__LINE__));
				AsicSendCommandToMcu(pAd, 0x30, PowerSafeCID, 0xff, 0x2);
				
				AsicCheckCommanOk(pAd, PowerSafeCID);
				RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
				DBGPRINT(RT_DEBUG_TRACE, ("PSM - rt30xx Issue Sleep command)\n"));
				}
			}
		else if (pAd->Mlme.OneSecPeriodicRound > 180)
			{
			if (pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
				{
				DBGPRINT(RT_DEBUG_TRACE, ("%s::%d\n",__FUNCTION__,__LINE__));
				RT28xxPciAsicRadioOff(pAd, GUI_IDLE_POWER_SAVE, 0);
				}
			else
				{
				DBGPRINT(RT_DEBUG_TRACE, ("%s::%d\n",__FUNCTION__,__LINE__));
				AsicSendCommandToMcu(pAd, 0x30, PowerSafeCID, 0xff, 0x02);
				
				AsicCheckCommanOk(pAd, PowerSafeCID);
				RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
				DBGPRINT(RT_DEBUG_TRACE, ("PSM -  rt28xx Issue Sleep command)\n"));
				}
			}
		}
	else
		{
		DBGPRINT(RT_DEBUG_TRACE,("STAMlmePeriodicExec MMCHK - CommonCfg.Ssid[%d]=%c%c%c%c... MlmeAux.Ssid[%d]=%c%c%c%c...\n",
			pAd->CommonCfg.SsidLen, pAd->CommonCfg.Ssid[0], pAd->CommonCfg.Ssid[1], pAd->CommonCfg.Ssid[2], pAd->CommonCfg.Ssid[3],
			pAd->MlmeAux.SsidLen, pAd->MlmeAux.Ssid[0], pAd->MlmeAux.Ssid[1], pAd->MlmeAux.Ssid[2], pAd->MlmeAux.Ssid[3]));
		}
	}
#endif 


#ifdef WPA_SUPPLICANT_SUPPORT
    if (pAd->StaCfg.WpaSupplicantUP == WPA_SUPPLICANT_DISABLE)
#endif 
    {
	
		if (pAd->StaCfg.bBlockAssoc &&
			RTMP_TIME_AFTER(pAd->Mlme.Now32, pAd->StaCfg.LastMicErrorTime + (60*OS_HZ)))
		pAd->StaCfg.bBlockAssoc = FALSE;
    }

    if ((pAd->PreMediaState != pAd->IndicateMediaState) && (pAd->CommonCfg.bWirelessEvent))
	{
		if (pAd->IndicateMediaState == NdisMediaStateConnected)
		{
			RTMPSendWirelessEvent(pAd, IW_STA_LINKUP_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
		}
		pAd->PreMediaState = pAd->IndicateMediaState;
	}




	if (pAd->CommonCfg.PSPXlink && ADHOC_ON(pAd))
	{
	}
	else
	{
	AsicStaBbpTuning(pAd);
	}

	TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount +
					 pAd->RalinkCounters.OneSecTxRetryOkCount +
					 pAd->RalinkCounters.OneSecTxFailCount;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
	{
		
		MlmeCalculateChannelQuality(pAd, NULL, pAd->Mlme.Now32);
	}

	
	
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	AsicAdjustTxPower(pAd);

	if (INFRA_ON(pAd))
	{
#ifdef QOS_DLS_SUPPORT
		
		RTMPCheckDLSTimeOut(pAd);
#endif 

		
		
		if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
		MlmeCheckPsmChange(pAd, pAd->Mlme.Now32);

		pAd->RalinkCounters.LastOneSecTotalTxCount = TxTotalCnt;

		if ((RTMP_TIME_AFTER(pAd->Mlme.Now32, pAd->StaCfg.LastBeaconRxTime + (1*OS_HZ))) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) &&
			(((TxTotalCnt + pAd->RalinkCounters.OneSecRxOkCnt) < 600)))
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

			
			LinkDown(pAd, FALSE);

#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
		
		if (pAd->StaCfg.WpaSupplicantUP) {
			RtmpOSWrielessEventSend(pAd, IWEVCUSTOM, RT_DISASSOC_EVENT_FLAG, NULL, NULL, 0);
		}
#endif 
#endif 

#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
		RtmpOSWrielessEventSend(pAd, SIOCGIWAP, -1, NULL, NULL, 0);
#endif 

			
			MlmeAutoReconnectLastSSID(pAd);
		}
		else if (CQI_IS_BAD(pAd->Mlme.ChannelQuality))
		{
			pAd->RalinkCounters.BadCQIAutoRecoveryCount ++;
			DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Bad CQI. Auto Recovery attempt #%ld\n", pAd->RalinkCounters.BadCQIAutoRecoveryCount));
			MlmeAutoReconnectLastSSID(pAd);
		}

		if (pAd->StaCfg.bAutoRoaming)
		{
			BOOLEAN	rv = FALSE;
			CHAR	dBmToRoam = pAd->StaCfg.dBmToRoam;
			CHAR	MaxRssi = RTMPMaxRssi(pAd,
										  pAd->StaCfg.RssiSample.LastRssi0,
										  pAd->StaCfg.RssiSample.LastRssi1,
										  pAd->StaCfg.RssiSample.LastRssi2);

			
			if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) &&
				(pAd->Mlme.SyncMachine.CurrState == SYNC_IDLE) &&
				(MaxRssi <= dBmToRoam))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Rssi=%d, dBmToRoam=%d\n", MaxRssi, (CHAR)dBmToRoam));


				
				if (rv == FALSE)
					rv = MlmeCheckForFastRoaming(pAd);

				if (rv == FALSE)
				{
					if ((pAd->StaCfg.LastScanTime + 10 * OS_HZ) < pAd->Mlme.Now32)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Roaming, No eligable entry, try new scan!\n"));
						pAd->StaCfg.ScanCnt = 2;
						pAd->StaCfg.LastScanTime = pAd->Mlme.Now32;
						MlmeAutoScan(pAd);
					}
				}
			}
		}
	}
	else if (ADHOC_ON(pAd))
	{

		
		
		
		if (RTMP_TIME_AFTER(pAd->Mlme.Now32, pAd->StaCfg.LastBeaconRxTime + ADHOC_BEACON_LOST_TIME) &&
			OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
		{
			MLME_START_REQ_STRUCT     StartReq;

			DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - excessive BEACON lost, last STA in this IBSS, MediaState=Disconnected\n"));
			LinkDown(pAd, FALSE);

			StartParmFill(pAd, &StartReq, (CHAR *)pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen);
			MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_START_REQ, sizeof(MLME_START_REQ_STRUCT), &StartReq);
			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_START;
		}

		for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++)
		{
			MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];

			if (pEntry->ValidAsCLI == FALSE)
				continue;

			if (RTMP_TIME_AFTER(pAd->Mlme.Now32, pEntry->LastBeaconRxTime + ADHOC_BEACON_LOST_TIME))
				MacTableDeleteEntry(pAd, pEntry->Aid, pEntry->Addr);
		}
	}
	else 
	{

		if (pAd->StaCfg.bScanReqIsFromWebUI &&
			RTMP_TIME_BEFORE(pAd->Mlme.Now32, pAd->StaCfg.LastScanTime + (30 * OS_HZ)))
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

				if (RTMP_TIME_AFTER(pAd->Mlme.Now32, pAd->StaCfg.LastScanTime + (10 * OS_HZ)))
				{
					DBGPRINT(RT_DEBUG_TRACE, ("STAMlmePeriodicExec():CNTL - ScanTab.BssNr==0, start a new ACTIVE scan SSID[%s]\n", pAd->MlmeAux.AutoReconnectSsid));
					ScanParmFill(pAd, &ScanReq, (PSTRING) pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen, BSS_ANY, SCAN_ACTIVE);
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
#ifdef CARRIER_DETECTION_SUPPORT 
					if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
					{
						if ((pAd->Mlme.OneSecPeriodicRound % 5) == 1)
							MlmeAutoReconnectLastSSID(pAd);
					}
					else
#endif 
						MlmeAutoReconnectLastSSID(pAd);
				}
			}
		}
	}

SKIP_AUTO_SCAN_CONN:

#ifdef DOT11_N_SUPPORT
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
#endif 


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040))
		TriEventCounterMaintenance(pAd);
#endif 
#endif 

	return;
}


VOID LinkDownExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	if (pAd != NULL)
	{
		MLME_DISASSOC_REQ_STRUCT   DisassocReq;

		if ((pAd->StaCfg.PortSecured == WPA_802_1X_PORT_NOT_SECURED) &&
			(INFRA_ON(pAd)))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("LinkDownExec(): disassociate with current AP...\n"));
			DisassocParmFill(pAd, &DisassocReq, pAd->CommonCfg.Bssid, REASON_DISASSOC_STA_LEAVING);
			MlmeEnqueue(pAd, ASSOC_STATE_MACHINE, MT2_MLME_DISASSOC_REQ,
						sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq);
			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;

			pAd->IndicateMediaState = NdisMediaStateDisconnected;
			RTMP_IndicateMediaState(pAd);
		    pAd->ExtraInfo = GENERAL_LINK_DOWN;
		}
	}
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
					pAd->MlmeAux.AutoReconnectSsidLen,
					pAd->MlmeAux.AutoReconnectSsid);
		RTMP_MLME_HANDLER(pAd);
	}
}


VOID MlmeAutoReconnectLastSSID(
	IN PRTMP_ADAPTER pAd)
{
	if (pAd->StaCfg.bAutoConnectByBssid)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Driver auto reconnect to last OID_802_11_BSSID setting - %02X:%02X:%02X:%02X:%02X:%02X\n",
									pAd->MlmeAux.Bssid[0],
									pAd->MlmeAux.Bssid[1],
									pAd->MlmeAux.Bssid[2],
									pAd->MlmeAux.Bssid[3],
									pAd->MlmeAux.Bssid[4],
									pAd->MlmeAux.Bssid[5]));

		pAd->MlmeAux.Channel = pAd->CommonCfg.Channel;
		MlmeEnqueue(pAd,
			 MLME_CNTL_STATE_MACHINE,
			 OID_802_11_BSSID,
			 MAC_ADDR_LEN,
			 pAd->MlmeAux.Bssid);

		pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;

		RTMP_MLME_HANDLER(pAd);
	}
	
	else if ((pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE) &&
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
		RTMP_MLME_HANDLER(pAd);
	}
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

		if ((pBss->LastBeaconRxTime + pAd->StaCfg.BeaconLostTime) < Now32)
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
			RTMP_MLME_HANDLER(pAd);
		}
	}
	DBGPRINT(RT_DEBUG_TRACE, ("<== MlmeCheckForRoaming(# of candidate= %d)\n",pRoamTab->BssNr));
}


BOOLEAN MlmeCheckForFastRoaming(
	IN	PRTMP_ADAPTER	pAd)
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

	DBGPRINT(RT_DEBUG_TRACE, ("<== MlmeCheckForFastRoaming (BssNr=%d)\n", pRoamTab->BssNr));
	if (pRoamTab->BssNr > 0)
	{
		
		if (pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE)
		{
			pAd->RalinkCounters.PoorCQIRoamingCount ++;
			DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - Roaming attempt #%ld\n", pAd->RalinkCounters.PoorCQIRoamingCount));
			MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_MLME_ROAMING_REQ, 0, NULL);
			RTMP_MLME_HANDLER(pAd);
			return TRUE;
		}
	}

	return FALSE;
}

VOID MlmeSetTxRate(
	IN PRTMP_ADAPTER		pAd,
	IN PMAC_TABLE_ENTRY		pEntry,
	IN PRTMP_TX_RATE_SWITCH	pTxRate)
{
	UCHAR	MaxMode = MODE_OFDM;

#ifdef DOT11_N_SUPPORT
	MaxMode = MODE_HTGREENFIELD;

	if (pTxRate->STBC && (pAd->StaCfg.MaxHTPhyMode.field.STBC) && (pAd->Antenna.field.TxPath == 2))
		pAd->StaCfg.HTPhyMode.field.STBC = STBC_USE;
	else
#endif 
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

#ifdef DOT11_N_SUPPORT
        if (pTxRate->ShortGI && (pAd->StaCfg.MaxHTPhyMode.field.ShortGI))
			pAd->StaCfg.HTPhyMode.field.ShortGI = GI_400;
		else
#endif 
			pAd->StaCfg.HTPhyMode.field.ShortGI = GI_800;

#ifdef DOT11_N_SUPPORT
		
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
#endif 

		pEntry->HTPhyMode.field.STBC	= pAd->StaCfg.HTPhyMode.field.STBC;
		pEntry->HTPhyMode.field.ShortGI	= pAd->StaCfg.HTPhyMode.field.ShortGI;
		pEntry->HTPhyMode.field.MCS		= pAd->StaCfg.HTPhyMode.field.MCS;
		pEntry->HTPhyMode.field.MODE	= pAd->StaCfg.HTPhyMode.field.MODE;
#ifdef DOT11_N_SUPPORT
        if ((pAd->StaCfg.MaxHTPhyMode.field.MODE == MODE_HTGREENFIELD) &&
            pAd->WIFItestbed.bGreenField)
            pEntry->HTPhyMode.field.MODE = MODE_HTGREENFIELD;
#endif 
    }

    pAd->LastTxRate = (USHORT)(pEntry->HTPhyMode.word);
}


VOID MlmeDynamicTxRateSwitching(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR					UpRateIdx = 0, DownRateIdx = 0, CurrRateIdx;
	ULONG					i, AccuTxTotalCnt = 0, TxTotalCnt;
	ULONG					TxErrorRatio = 0;
	BOOLEAN					bTxRateChanged = FALSE, bUpgradeQuality = FALSE;
	PRTMP_TX_RATE_SWITCH	pCurrTxRate, pNextTxRate = NULL;
	PUCHAR					pTable;
	UCHAR					TableSize = 0;
	UCHAR					InitTxRateIdx = 0, TrainUp, TrainDown;
	CHAR					Rssi, RssiOffset = 0;
	TX_STA_CNT1_STRUC		StaTx1;
	TX_STA_CNT0_STRUC		TxStaCnt0;
	ULONG					TxRetransmit = 0, TxSuccess = 0, TxFailCount = 0;
	MAC_TABLE_ENTRY			*pEntry;
	RSSI_SAMPLE				*pRssi = &pAd->StaCfg.RssiSample;

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
		return;
	}
#endif 

	
	
	
	for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		pEntry = &pAd->MacTab.Content[i];

	
		if (RTMPCheckEntryEnableAutoRateSwitch(pAd, pEntry) == FALSE)
			continue;

		if ((pAd->MacTab.Size == 1) || (pEntry->ValidAsDls))
		{
			Rssi = RTMPMaxRssi(pAd,
							   pRssi->AvgRssi0,
							   pRssi->AvgRssi1,
							   pRssi->AvgRssi2);

			
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
			if (INFRA_ON(pAd) && (i == 1))
				Rssi = RTMPMaxRssi(pAd,
								   pRssi->AvgRssi0,
								   pRssi->AvgRssi1,
								   pRssi->AvgRssi2);
			else
				Rssi = RTMPMaxRssi(pAd,
								   pEntry->RssiSample.AvgRssi0,
								   pEntry->RssiSample.AvgRssi1,
								   pEntry->RssiSample.AvgRssi2);

			TxTotalCnt = pEntry->OneSecTxNoRetryOkCount +
				 pEntry->OneSecTxRetryOkCount +
				 pEntry->OneSecTxFailCount;

			if (TxTotalCnt)
				TxErrorRatio = ((pEntry->OneSecTxRetryOkCount + pEntry->OneSecTxFailCount) * 100) / TxTotalCnt;
		}

		if (TxTotalCnt)
		{
			
			if (TxErrorRatio == 100)
			{
				TX_RTY_CFG_STRUC	TxRtyCfg,TxRtyCfgtmp;
				ULONG	Index;
				ULONG	MACValue;

				RTMP_IO_READ32(pAd, TX_RTY_CFG, &TxRtyCfg.word);
				TxRtyCfgtmp.word = TxRtyCfg.word;
				TxRtyCfg.field.LongRtyLimit = 0x0;
				TxRtyCfg.field.ShortRtyLimit = 0x0;
				RTMP_IO_WRITE32(pAd, TX_RTY_CFG, TxRtyCfg.word);

				RTMPusecDelay(1);

				Index = 0;
				MACValue = 0;
				do
				{
					RTMP_IO_READ32(pAd, TXRXQ_PCNT, &MACValue);
					if ((MACValue & 0xffffff) == 0)
						break;
					Index++;
					RTMPusecDelay(1000);
				}while((Index < 330)&&(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)));

				RTMP_IO_READ32(pAd, TX_RTY_CFG, &TxRtyCfg.word);
				TxRtyCfg.field.LongRtyLimit = TxRtyCfgtmp.field.LongRtyLimit;
				TxRtyCfg.field.ShortRtyLimit = TxRtyCfgtmp.field.ShortRtyLimit;
				RTMP_IO_WRITE32(pAd, TX_RTY_CFG, TxRtyCfg.word);
			}
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

#ifdef DOT11_N_SUPPORT
		if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
		{
			TrainUp		= (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
			TrainDown	= (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
		}
		else
#endif 
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
#ifdef DOT11_N_SUPPORT
			
			if ((pTable == RateSwitchTable11BGN3S) ||
				(pTable == RateSwitchTable11N3S) ||
				(pTable == RateSwitchTable))
			{
				if (MCS23 && (Rssi >= -70))
					TxRateIdx = MCS23;
				else if (MCS22 && (Rssi >= -72))
					TxRateIdx = MCS22;
		    else if (MCS21 && (Rssi >= -76))
					TxRateIdx = MCS21;
				else if (MCS20 && (Rssi >= -78))
					TxRateIdx = MCS20;
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
#endif 
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

		{
			UCHAR tmpTxRate;

			
			if (!bTxRateChanged && (pAd->RalinkCounters.OneSecReceivedByteCount > (pAd->RalinkCounters.OneSecTransmittedByteCount * 5)))
			{
				tmpTxRate = DownRateIdx;
				DBGPRINT_RAW(RT_DEBUG_TRACE,("DRS: Rx(%d) is 5 times larger than Tx(%d), use low rate (curr=%d, tmp=%d)\n",
					pAd->RalinkCounters.OneSecReceivedByteCount, pAd->RalinkCounters.OneSecTransmittedByteCount, pEntry->CurrTxRateIndex, tmpTxRate));
			}
			else
			{
				tmpTxRate = pEntry->CurrTxRateIndex;
			}

			pNextTxRate = (PRTMP_TX_RATE_SWITCH) &pTable[(tmpTxRate+1)*5];
			if (bTxRateChanged && pNextTxRate)
			{
				MlmeSetTxRate(pAd, pEntry, pNextTxRate);
			}
		}
		
		RESET_ONE_SEC_TX_CNT(pEntry);
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
	BOOLEAN					bTxRateChanged; 
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

#ifdef DOT11_N_SUPPORT
	if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
	{
		TrainUp		= (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
		TrainDown	= (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
	}
	else
#endif 
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
			bTxRateChanged = TRUE;
	}
	
	else if (pAd->CommonCfg.TxRateIndex < CurrRateIdx)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE,("QuickDRS: --TX rate from %d to %d \n", CurrRateIdx, pAd->CommonCfg.TxRateIndex));

		pAd->DrsCounters.TxRateUpPenalty = 0;           
		pAd->DrsCounters.TxQuality[pAd->CommonCfg.TxRateIndex] = 0;
		pAd->DrsCounters.PER[pAd->CommonCfg.TxRateIndex] = 0;
			bTxRateChanged = TRUE;
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

	
	
	
	
	
	
	PowerMode = pAd->StaCfg.WindowsPowerMode;

	if (INFRA_ON(pAd) &&
		(PowerMode != Ndis802_11PowerModeCAM) &&
		(pAd->StaCfg.Psm == PWR_ACTIVE) &&

		(pAd->Mlme.CntlMachine.CurrState == CNTL_IDLE)&&
		RTMP_TEST_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP)
		 )
	{
		NdisGetSystemUpTime(&pAd->Mlme.LastSendNULLpsmTime);
		pAd->RalinkCounters.RxCountSinceLastNULL = 0;
		RTMP_SET_PSM_BIT(pAd, PWR_SAVE);
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
#endif 


VOID MlmeCalculateChannelQuality(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pMacEntry,
	IN ULONG Now32)
{
	ULONG TxOkCnt, TxCnt, TxPER, TxPRR;
	ULONG RxCnt, RxPER;
	UCHAR NorRssi;
	CHAR  MaxRssi;
	RSSI_SAMPLE *pRssiSample = NULL;
	UINT32 OneSecTxNoRetryOkCount = 0;
	UINT32 OneSecTxRetryOkCount = 0;
	UINT32 OneSecTxFailCount = 0;
	UINT32 OneSecRxOkCnt = 0;
	UINT32 OneSecRxFcsErrCnt = 0;
	ULONG ChannelQuality = 0;  
#ifdef CONFIG_STA_SUPPORT
	ULONG BeaconLostTime = pAd->StaCfg.BeaconLostTime;
#endif 

#ifdef CONFIG_STA_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT 
	
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
	{
		BeaconLostTime = pAd->StaCfg.BeaconLostTime + (pAd->StaCfg.BeaconLostTime/2);
	}
#endif 
#endif 

#ifdef CONFIG_STA_SUPPORT
	if (pAd->OpMode == OPMODE_STA)
	{
		pRssiSample = &pAd->StaCfg.RssiSample;
		OneSecTxNoRetryOkCount = pAd->RalinkCounters.OneSecTxNoRetryOkCount;
		OneSecTxRetryOkCount = pAd->RalinkCounters.OneSecTxRetryOkCount;
		OneSecTxFailCount = pAd->RalinkCounters.OneSecTxFailCount;
		OneSecRxOkCnt = pAd->RalinkCounters.OneSecRxOkCnt;
		OneSecRxFcsErrCnt = pAd->RalinkCounters.OneSecRxFcsErrCnt;
	}
#endif 

	MaxRssi = RTMPMaxRssi(pAd, pRssiSample->LastRssi0,
								pRssiSample->LastRssi1,
								pRssiSample->LastRssi2);

	
	
	
	TxOkCnt = OneSecTxNoRetryOkCount + OneSecTxRetryOkCount;
	TxCnt = TxOkCnt + OneSecTxFailCount;
	if (TxCnt < 5)
	{
		TxPER = 0;
		TxPRR = 0;
	}
	else
	{
		TxPER = (OneSecTxFailCount * 100) / TxCnt;
		TxPRR = ((TxCnt - OneSecTxNoRetryOkCount) * 100) / TxCnt;
	}

	
	
	
	RxCnt = OneSecRxOkCnt + OneSecRxFcsErrCnt;
	if (RxCnt < 5)
		RxPER = 0;
	else
		RxPER = (OneSecRxFcsErrCnt * 100) / RxCnt;

	
	
	
#ifdef CONFIG_STA_SUPPORT
	if ((pAd->OpMode == OPMODE_STA) &&
		INFRA_ON(pAd) &&
		(OneSecTxNoRetryOkCount < 2) && 
		((pAd->StaCfg.LastBeaconRxTime + BeaconLostTime) < Now32))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("BEACON lost > %ld msec with TxOkCnt=%ld -> CQI=0\n", BeaconLostTime, TxOkCnt));
		ChannelQuality = 0;
	}
	else
#endif 
	{
		
		if (MaxRssi > -40)
			NorRssi = 100;
		else if (MaxRssi < -90)
			NorRssi = 0;
		else
			NorRssi = (MaxRssi + 90) * 2;

		
		ChannelQuality = (RSSI_WEIGHTING * NorRssi +
								   TX_WEIGHTING * (100 - TxPRR) +
								   RX_WEIGHTING* (100 - RxPER)) / 100;
	}


#ifdef CONFIG_STA_SUPPORT
	if (pAd->OpMode == OPMODE_STA)
		pAd->Mlme.ChannelQuality = (ChannelQuality > 100) ? 100 : ChannelQuality;
#endif 


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
	IN PRTMP_ADAPTER		pAd,
	IN	BOOLEAN				bLinkUp,
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
	BOOLEAN					*auto_rate_cur_p;
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




#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		pHtPhy		= &pAd->StaCfg.HTPhyMode;
		pMaxHtPhy	= &pAd->StaCfg.MaxHTPhyMode;
		pMinHtPhy	= &pAd->StaCfg.MinHTPhyMode;

		auto_rate_cur_p = &pAd->StaCfg.bAutoTxRateSwitch;
		HtMcs		= pAd->StaCfg.DesiredTransmitSetting.field.MCS;

		if ((pAd->StaCfg.BssType == BSS_ADHOC) &&
			(pAd->CommonCfg.PhyMode == PHY_11B) &&
		(MaxDesire > RATE_11))
		{
			MaxDesire = RATE_11;
		}
	}
#endif 

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

	if (HtMcs != MCS_AUTO)
	{
		
		
		*auto_rate_cur_p = FALSE;
	}
	else
	{
		
		
		*auto_rate_cur_p = TRUE;
	}

#ifdef CONFIG_STA_SUPPORT
	if ((ADHOC_ON(pAd) || INFRA_ON(pAd)) && (pAd->OpMode == OPMODE_STA))
	{
		pSupRate = &pAd->StaActive.SupRate[0];
		pExtRate = &pAd->StaActive.ExtRate[0];
		SupRateLen = pAd->StaActive.SupRateLen;
		ExtRateLen = pAd->StaActive.ExtRateLen;
	}
	else
#endif 
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
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			dbm = pAd->StaCfg.RssiSample.AvgRssi0 - pAd->BbpRssiToDbmDelta;
#endif 
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
#ifdef DOT11_N_SUPPORT
			case PHY_11BGN_MIXED:
#endif 
				pAd->CommonCfg.MlmeRate = RATE_1;
				pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_CCK;
				pAd->CommonCfg.MlmeTransmit.field.MCS = RATE_1;


				pAd->CommonCfg.RtsRate = RATE_11;



				break;
			case PHY_11G:
			case PHY_11A:
#ifdef DOT11_N_SUPPORT
			case PHY_11AGN_MIXED:
			case PHY_11GN_MIXED:
			case PHY_11N_2_4G:
			case PHY_11AN_MIXED:
			case PHY_11N_5G:
#endif 
				pAd->CommonCfg.MlmeRate = RATE_6;
				pAd->CommonCfg.RtsRate = RATE_6;
				pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
				pAd->CommonCfg.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
				break;
			case PHY_11ABG_MIXED:
#ifdef DOT11_N_SUPPORT
			case PHY_11ABGN_MIXED:
#endif 
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

#ifdef DOT11_N_SUPPORT

VOID MlmeUpdateHtTxRates(
	IN PRTMP_ADAPTER		pAd,
	IN	UCHAR				apidx)
{
	UCHAR	StbcMcs; 
	CHAR	i; 
	RT_HT_CAPABILITY	*pRtHtCap = NULL;
	RT_HT_PHY_INFO		*pActiveHtPhy = NULL;
	ULONG		BasicMCS;
	UCHAR j, bitmask;
	PRT_HT_PHY_INFO			pDesireHtPhy = NULL;
	PHTTRANSMIT_SETTING		pHtPhy = NULL;
	PHTTRANSMIT_SETTING		pMaxHtPhy = NULL;
	PHTTRANSMIT_SETTING		pMinHtPhy = NULL;
	BOOLEAN					*auto_rate_cur_p;

	DBGPRINT(RT_DEBUG_TRACE,("MlmeUpdateHtTxRates===> \n"));

	auto_rate_cur_p = NULL;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		pDesireHtPhy	= &pAd->StaCfg.DesiredHtPhyInfo;
		pActiveHtPhy	= &pAd->StaCfg.DesiredHtPhyInfo;
		pHtPhy		= &pAd->StaCfg.HTPhyMode;
		pMaxHtPhy	= &pAd->StaCfg.MaxHTPhyMode;
		pMinHtPhy	= &pAd->StaCfg.MinHTPhyMode;

		auto_rate_cur_p = &pAd->StaCfg.bAutoTxRateSwitch;
	}
#endif 

#ifdef CONFIG_STA_SUPPORT
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
#endif 
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

	if (pDesireHtPhy->MCSSet[4] != 0)
	{
		pMaxHtPhy->field.MCS = 32;
	}

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
	
#ifdef CONFIG_STA_SUPPORT
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
#endif 


	
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


VOID BATableInit(
	IN PRTMP_ADAPTER pAd,
    IN BA_TABLE *Tab)
{
	int i;

	Tab->numAsOriginator = 0;
	Tab->numAsRecipient = 0;
	Tab->numDoneOriginator = 0;
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
#endif 


VOID MlmeRadioOff(
	IN PRTMP_ADAPTER pAd)
{
	RTMP_MLME_RADIO_OFF(pAd);
}


VOID MlmeRadioOn(
	IN PRTMP_ADAPTER pAd)
{
	RTMP_MLME_RADIO_ON(pAd);
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


ULONG BssSsidTableSearchBySSID(
	IN BSS_TABLE *Tab,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr; i++)
	{
		if (SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen))
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

#ifdef DOT11_N_SUPPORT

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
#endif 


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
#ifdef DOT11_N_SUPPORT
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
#endif 

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

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		PEID_STRUCT     pEid;
		USHORT          Length = 0;


		NdisZeroMemory(&pBss->WpaIE.IE[0], MAX_CUSTOM_LEN);
		NdisZeroMemory(&pBss->RsnIE.IE[0], MAX_CUSTOM_LEN);
#ifdef EXT_BUILD_CHANNEL_LIST
		NdisZeroMemory(&pBss->CountryString[0], 3);
		pBss->bHasCountryIE = FALSE;
#endif 
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
#ifdef EXT_BUILD_CHANNEL_LIST
				case IE_COUNTRY:
					NdisMoveMemory(&pBss->CountryString[0], pEid->Octet, 3);
					pBss->bHasCountryIE = TRUE;
					break;
#endif 
            }
			Length = Length + 2 + (USHORT)pEid->Len;  
			pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);
		}
	}
#endif 
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

	Idx = BssTableSearchWithSSID(Tab, pBssid,  (UCHAR *)Ssid, SsidLen, ChannelNo);
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

#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
VOID  TriEventInit(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR		i;

	for (i = 0;i < MAX_TRIGGER_EVENT;i++)
		pAd->CommonCfg.TriggerEventTab.EventA[i].bValid = FALSE;

	pAd->CommonCfg.TriggerEventTab.EventANo = 0;
	pAd->CommonCfg.TriggerEventTab.EventBCountDown = 0;
}

ULONG TriEventTableSetEntry(
	IN	PRTMP_ADAPTER	pAd,
	OUT TRIGGER_EVENT_TAB *Tab,
	IN PUCHAR pBssid,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR			HtCapabilityLen,
	IN UCHAR			RegClass,
	IN UCHAR ChannelNo)
{
	
	if (HtCapabilityLen == 0)
	{
		if (Tab->EventANo < MAX_TRIGGER_EVENT)
		{
			RTMPMoveMemory(Tab->EventA[Tab->EventANo].BSSID, pBssid, 6);
			Tab->EventA[Tab->EventANo].bValid = TRUE;
			Tab->EventA[Tab->EventANo].Channel = ChannelNo;
			Tab->EventA[Tab->EventANo].CDCounter = pAd->CommonCfg.Dot11BssWidthChanTranDelay;
			if (RegClass != 0)
			{
				
				Tab->EventA[Tab->EventANo].RegClass = RegClass;
			}
			else
			{
				
				if (pAd->StaActive.SupportedHtPhy.bHtEnable == TRUE)
				{
					if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
					{
						Tab->EventA[Tab->EventANo].RegClass = 32;
					}
					else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
						Tab->EventA[Tab->EventANo].RegClass = 33;
				}
				else
					Tab->EventA[Tab->EventANo].RegClass = ??;

			}

			Tab->EventANo ++;
		}
	}
	else if (pHtCapability->HtCapInfo.Intolerant40)
	{
		Tab->EventBCountDown = pAd->CommonCfg.Dot11BssWidthChanTranDelay;
	}

}


VOID TriEventCounterMaintenance(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR		i;
	BOOLEAN			bNotify = FALSE;
	for (i = 0;i < MAX_TRIGGER_EVENT;i++)
	{
		if (pAd->CommonCfg.TriggerEventTab.EventA[i].bValid && (pAd->CommonCfg.TriggerEventTab.EventA[i].CDCounter > 0))
		{
			pAd->CommonCfg.TriggerEventTab.EventA[i].CDCounter--;
			if (pAd->CommonCfg.TriggerEventTab.EventA[i].CDCounter == 0)
			{
				pAd->CommonCfg.TriggerEventTab.EventA[i].bValid = FALSE;
				pAd->CommonCfg.TriggerEventTab.EventANo --;
				
				bNotify = TRUE;
			}
		}
	}
	if (pAd->CommonCfg.TriggerEventTab.EventBCountDown > 0)
	{
		pAd->CommonCfg.TriggerEventTab.EventBCountDown--;
		if (pAd->CommonCfg.TriggerEventTab.EventBCountDown == 0)
			bNotify = TRUE;
	}

	if (bNotify == TRUE)
		Update2040CoexistFrameAndNotify(pAd, BSSID_WCID, TRUE);
}
#endif 
#endif 


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
#ifdef CARRIER_DETECTION_SUPPORT 
             || (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
#endif 
            )
{
			if (pInBss->Hidden)
				bIsHiddenApIncluded = TRUE;
}

		if ((pInBss->BssType == pAd->StaCfg.BssType) &&
			(SSID_EQUAL(Ssid, SsidLen, pInBss->Ssid, pInBss->SsidLen) || bIsHiddenApIncluded))
		{
			BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];


#ifdef EXT_BUILD_CHANNEL_LIST
			
			if ((pAd->StaCfg.IEEE80211dClientMode == Rt802_11_D_Strict) &&
				(pInBss->bHasCountryIE == FALSE))
			{
				DBGPRINT(RT_DEBUG_TRACE,("StaCfg.IEEE80211dClientMode == Rt802_11_D_Strict, but this AP doesn't have country IE.\n"));
				continue;
			}
#endif 

#ifdef DOT11_N_SUPPORT
			
			if ((pInBss->HtCapabilityLen == 0) &&
				((pAd->CommonCfg.PhyMode == PHY_11N_2_4G) || (pAd->CommonCfg.PhyMode == PHY_11N_5G)))
			{
				DBGPRINT(RT_DEBUG_TRACE,("STA is in N-only Mode, this AP don't have Ht capability in Beacon.\n"));
				continue;
			}
#endif 

			
			
			if (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
			{
				
				if ((pAd->StaCfg.AuthMode != pInBss->AuthMode) && (pAd->StaCfg.AuthMode != pInBss->AuthModeAux))
					
					continue;

				
				if ((pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA) || (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK))
				{
					
					if (pInBss->WPA.bMixMode == FALSE)
						if (pAd->StaCfg.WepStatus != pInBss->WPA.GroupCipher)
							continue;

					
					if ((pAd->StaCfg.WepStatus < pInBss->WPA.GroupCipher) &&
						(pInBss->WPA.GroupCipher != Ndis802_11GroupWEP40Enabled) &&
						(pInBss->WPA.GroupCipher != Ndis802_11GroupWEP104Enabled))
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

					
					if ((pAd->StaCfg.WepStatus < pInBss->WPA.GroupCipher) &&
						(pInBss->WPA2.GroupCipher != Ndis802_11GroupWEP40Enabled) &&
						(pInBss->WPA2.GroupCipher != Ndis802_11GroupWEP104Enabled))
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

#ifdef DOT11_N_SUPPORT
			
			
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
#endif 

			
			NdisMoveMemory(pOutBss, pInBss, sizeof(BSS_ENTRY));

			OutTab->BssNr++;
		}
		else if ((pInBss->BssType == pAd->StaCfg.BssType) && (SsidLen == 0))
		{
			BSS_ENTRY *pOutBss = &OutTab->BssEntry[OutTab->BssNr];


#ifdef DOT11_N_SUPPORT
			
			if ((pInBss->HtCapabilityLen == 0) &&
				((pAd->CommonCfg.PhyMode == PHY_11N_2_4G) || (pAd->CommonCfg.PhyMode == PHY_11N_5G)))
			{
				DBGPRINT(RT_DEBUG_TRACE,("STA is in N-only Mode, this AP don't have Ht capability in Beacon.\n"));
				continue;
			}
#endif 

			
			
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

#ifdef DOT11_N_SUPPORT
			
			
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
#endif 

			
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
	INT	  i, j;
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
#endif 


VOID BssCipherParse(
	IN OUT	PBSS_ENTRY	pBss)
{
	PEID_STRUCT		 pEid;
	PUCHAR				pTmp;
	PRSN_IE_HEADER_STRUCT			pRsnHeader;
	PCIPHER_SUITE_STRUCT			pCipher;
	PAKM_SUITE_STRUCT				pAKM;
	USHORT							Count;
	INT								Length;
	NDIS_802_11_ENCRYPTION_STATUS	TmpCipher;

	
	
	
	if (pBss->Privacy)
	{
		pBss->WepStatus		= Ndis802_11WEPEnabled;
	}
	else
	{
		pBss->WepStatus		= Ndis802_11WEPDisabled;
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
	pBss->WPA2.bMixMode	 = FALSE;


	Length = (INT) pBss->VarIELen;

	while (Length > 0)
	{
		
		pTmp = ((PUCHAR) pBss->VarIEs) + pBss->VarIELen - Length;
		pEid = (PEID_STRUCT) pTmp;
		switch (pEid->Eid)
		{
			case IE_WPA:
				if (NdisEqualMemory(pEid->Octet, SES_OUI, 3) && (pEid->Len == 7))
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

				
				
				while (Count > 0)
				{
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
							if (pBss->AuthMode == Ndis802_11AuthModeOpen)
								pBss->AuthMode = Ndis802_11AuthModeMax;
							else
								pBss->AuthModeAux = Ndis802_11AuthModeMax;
							break;
					}
					pTmp   += (Count * sizeof(AKM_SUITE_STRUCT));
					Count--;
				}

				
				if (pBss->BssType == BSS_ADHOC)
				{
					pBss->AuthMode = Ndis802_11AuthModeWPANone;
					pBss->AuthModeAux = Ndis802_11AuthModeWPANone;
					pBss->WPA.PairCipherAux = pBss->WPA2.PairCipherAux;
					pBss->WPA.GroupCipher	= pBss->WPA2.GroupCipher;
					pBss->WepStatus			= pBss->WPA.GroupCipher;
					
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
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		COPY_MAC_ADDR(pHdr80211->Addr2, pAd->CurrentAddress);
#endif 
	COPY_MAC_ADDR(pHdr80211->Addr3, pBssid);
}






ULONG MakeOutgoingFrame(
	OUT UCHAR *Buffer,
	OUT ULONG *FrameLen, ...)
{
	UCHAR   *p;
	int	leng;
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
	INT		 Tail, Machine;
	PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;
	INT		 MsgType;
	MLME_QUEUE	*Queue = (MLME_QUEUE *)&pAd->Mlme.Queue;

#ifdef RALINK_ATE
	
	if(ATE_ON(pAd))
		return FALSE;
#endif 

	
	
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

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (!MsgTypeSubst(pAd, pFrame, &Machine, &MsgType))
		{
			DBGPRINT_ERR(("MlmeEnqueueForRecv: un-recongnized mgmt->subtype=%d\n",pFrame->Hdr.FC.SubType));
			return FALSE;
		}
	}
#endif 

	
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

	RTMP_MLME_HANDLER(pAd);

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
#ifdef RTMP_MAC_PCI
	MLME_QUEUE_ELEM		*Elem = NULL;
#endif 
#ifdef CONFIG_STA_SUPPORT
	BOOLEAN				Cancelled;
#endif 

	DBGPRINT(RT_DEBUG_TRACE, ("MlmeRestartStateMachine \n"));

#ifdef RTMP_MAC_PCI
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

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef QOS_DLS_SUPPORT
		UCHAR i;
#endif 
		
		
		RTMPCancelTimer(&pAd->MlmeAux.AssocTimer,	  &Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.ReassocTimer,   &Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.DisassocTimer,  &Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.AuthTimer,	   &Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer,	   &Cancelled);
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer,	   &Cancelled);

#ifdef QOS_DLS_SUPPORT
		for (i=0; i<MAX_NUM_OF_DLS_ENTRY; i++)
		{
			RTMPCancelTimer(&pAd->StaCfg.DLSEntry[i].Timer, &Cancelled);
		}
#endif 
	}
#endif 

	
	AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
	AsicLockChannel(pAd, pAd->CommonCfg.Channel);

	
	RTMPResumeMsduTransmission(pAd);

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		
		pAd->Mlme.CntlMachine.CurrState    = CNTL_IDLE;
		pAd->Mlme.AssocMachine.CurrState   = ASSOC_IDLE;
		pAd->Mlme.AuthMachine.CurrState    = AUTH_REQ_IDLE;
		pAd->Mlme.AuthRspMachine.CurrState = AUTH_RSP_IDLE;
		pAd->Mlme.SyncMachine.CurrState    = SYNC_IDLE;
		pAd->Mlme.ActMachine.CurrState    = ACT_IDLE;
#ifdef QOS_DLS_SUPPORT
		pAd->Mlme.DlsMachine.CurrState    = DLS_IDLE;
#endif 
	}
#endif 

#ifdef RTMP_MAC_PCI
	
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



#ifdef CONFIG_STA_SUPPORT
BOOLEAN MsgTypeSubst(
	IN PRTMP_ADAPTER  pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType)
{
	USHORT	Seq, Alg;
	UCHAR	EAPType;
	PUCHAR	pData;

	
	pData = (PUCHAR) pFrame + LENGTH_802_11;

	
	if (pFrame->Hdr.FC.Type == BTYPE_DATA)
	{
		{
	        *Machine = WPA_STATE_MACHINE;
			EAPType = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
	        return (WpaMsgTypeSubst(EAPType, (INT *) MsgType));
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
			NdisMoveMemory(&Alg, &pFrame->Octet[0], sizeof(USHORT));
			if (Seq == 1 || Seq == 3)
			{
				*Machine = AUTH_RSP_STATE_MACHINE;
				*MsgType = MT2_PEER_AUTH_ODD;
			}
			else if (Seq == 2 || Seq == 4)
			{
				if (Alg == AUTH_MODE_OPEN || Alg == AUTH_MODE_KEY)
				{
					*Machine = AUTH_STATE_MACHINE;
					*MsgType = MT2_PEER_AUTH_EVEN;
				}
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
#endif 






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

#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11_N_SUPPORT
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


BOOLEAN		RTMPCheckHt(
	IN	PRTMP_ADAPTER			pAd,
	IN	UCHAR					Wcid,
	IN	HT_CAPABILITY_IE		*pHtCapability,
	IN	ADD_HT_INFO_IE			*pAddHtInfo)
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
#endif 
#endif 


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
#ifdef DOT11_N_SUPPORT
		case PHY_11ABGN_MIXED:
		case PHY_11BGN_MIXED:
#endif 
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
#ifdef DOT11_N_SUPPORT
		case PHY_11N_2_4G:	
		case PHY_11GN_MIXED:
		case PHY_11AGN_MIXED:
		case PHY_11AN_MIXED:
		case PHY_11N_5G:
#endif 
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



VOID AsicEvaluateRxAnt(
	IN PRTMP_ADAPTER	pAd)
{
#ifdef CONFIG_STA_SUPPORT
	UCHAR	BBPR3 = 0;
#endif 

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		return;
#endif 

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS	|
							fRTMP_ADAPTER_HALT_IN_PROGRESS	|
							fRTMP_ADAPTER_RADIO_OFF			|
							fRTMP_ADAPTER_NIC_NOT_EXIST		|
							fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) ||
							OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE)
#ifdef RT3090
							|| (pAd->bPCIclkOff == TRUE)
#endif 
#ifdef ANT_DIVERSITY_SUPPORT
							|| (pAd->EepromAccess)
#endif 
							)
		return;

#ifdef ANT_DIVERSITY_SUPPORT
	if ((pAd->NicConfig2.field.AntDiversity) && (pAd->CommonCfg.bRxAntDiversity == ANT_DIVERSITY_ENABLE))
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
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
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
#ifdef RTMP_MAC_PCI
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			pAd->StaCfg.BBPR3 = BBPR3;
#endif 
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
#endif 
	}
}


VOID AsicRxAntEvalTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER	*pAd = (RTMP_ADAPTER *)FunctionContext;
	BOOLEAN			bSwapAnt = FALSE;
#ifdef CONFIG_STA_SUPPORT
	UCHAR			BBPR3 = 0;
	CHAR			larger = -127, rssi0, rssi1, rssi2;
#endif 

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
		return;
#endif 

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS	|
							fRTMP_ADAPTER_HALT_IN_PROGRESS	|
							fRTMP_ADAPTER_RADIO_OFF			|
							fRTMP_ADAPTER_NIC_NOT_EXIST) ||
							OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE)
#ifdef RT3090
							|| (pAd->bPCIclkOff == TRUE)
#endif 
#ifdef ANT_DIVERSITY_SUPPORT
							|| (pAd->EepromAccess)
#endif 
							)
		return;

#ifdef ANT_DIVERSITY_SUPPORT
	if ((pAd->NicConfig2.field.AntDiversity) && (pAd->CommonCfg.bRxAntDiversity == ANT_DIVERSITY_ENABLE))
	{
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			if ((pAd->RxAnt.RcvPktNumWhenEvaluate != 0) && (pAd->RxAnt.Pair1AvgRssi[pAd->RxAnt.Pair1SecondaryRxAnt] >= pAd->RxAnt.Pair1AvgRssi[pAd->RxAnt.Pair1PrimaryRxAnt]))
				bSwapAnt = TRUE;
#endif 
		if (bSwapAnt == TRUE)
			{
				UCHAR			temp;

				
				
				
				
				
				temp = pAd->RxAnt.Pair1PrimaryRxAnt;
				pAd->RxAnt.Pair1PrimaryRxAnt = pAd->RxAnt.Pair1SecondaryRxAnt;
				pAd->RxAnt.Pair1SecondaryRxAnt = temp;

#ifdef CONFIG_STA_SUPPORT
				pAd->RxAnt.Pair1LastAvgRssi = (pAd->RxAnt.Pair1AvgRssi[pAd->RxAnt.Pair1SecondaryRxAnt] >> 3);
#endif 

			}
			else
			{
				
				AsicSetRxAnt(pAd, pAd->RxAnt.Pair1PrimaryRxAnt);
				pAd->RxAnt.EvaluateStableCnt ++;
			}

			pAd->RxAnt.EvaluatePeriod = 0; 

#ifdef CONFIG_STA_SUPPORT
			DBGPRINT(RT_DEBUG_TRACE,("AsicRxAntEvalAction::After Eval(fix in #%d), <%d, %d>, RcvPktNumWhenEvaluate=%ld\n",
					pAd->RxAnt.Pair1PrimaryRxAnt, (pAd->RxAnt.Pair1AvgRssi[0] >> 3), (pAd->RxAnt.Pair1AvgRssi[1] >> 3), pAd->RxAnt.RcvPktNumWhenEvaluate));
#endif 
		}
		else
#endif 
	{
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
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
#ifdef RTMP_MAC_PCI
			pAd->StaCfg.BBPR3 = BBPR3;
#endif 
		}
#endif 
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


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		
		if (pEntry && (pEntry->ValidAsCLI) && (pEntry->Sst == SST_ASSOC))
		{
		result = pAd->StaCfg.bAutoTxRateSwitch;
	}
		else
			result = FALSE;

#ifdef QOS_DLS_SUPPORT
		if (pEntry && (pEntry->ValidAsDls))
			result = pAd->StaCfg.bAutoTxRateSwitch;
#endif 
	}
#endif 



	return result;
}


BOOLEAN RTMPAutoRateSwitchCheck(
	IN PRTMP_ADAPTER    pAd)
{

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (pAd->StaCfg.bAutoTxRateSwitch)
			return TRUE;
	}
#endif 
	return FALSE;
}



UCHAR RTMPStaFixedTxMode(
	IN PRTMP_ADAPTER    pAd,
	IN PMAC_TABLE_ENTRY	pEntry)
{
	UCHAR	tx_mode = FIXED_TXMODE_HT;


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		tx_mode = (UCHAR)pAd->StaCfg.DesiredTransmitSetting.field.FixedTxMode;
	}
#endif 

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

#ifdef CONFIG_STA_SUPPORT

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
#ifdef RTMP_MAC_PCI
		&& (pAd->bPCIclkOff == FALSE)
#endif 
		)
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R66, &OrigR66Value);
		R66 = OrigR66Value;

		if (pAd->Antenna.field.RxPath > 1)
			Rssi = (pAd->StaCfg.RssiSample.AvgRssi0 + pAd->StaCfg.RssiSample.AvgRssi1) >> 1;
		else
			Rssi = pAd->StaCfg.RssiSample.AvgRssi0;

		if (pAd->LatchRfRegs.Channel <= 14)
		{	
#ifdef RT30xx
			
			

			if (IS_RT3070(pAd)||IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd))
			{
				if (Rssi > RSSI_FOR_MID_LOW_SENSIBILITY)
				{
					R66 = 0x1C + 2*GET_LNA_GAIN(pAd) + 0x20;
					if (OrigR66Value != R66)
					{
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
					}
				}
				else
				{
					R66 = 0x1C + 2*GET_LNA_GAIN(pAd);
					if (OrigR66Value != R66)
					{
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
					}
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
#endif 

VOID RTMPSetAGCInitValue(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			BandWidth)
{
	UCHAR	R66 = 0x30;

	if (pAd->LatchRfRegs.Channel <= 14)
	{	
#ifdef RT30xx
		

		if (IS_RT3070(pAd)||IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd))
		{
			R66 = 0x1C + 2*GET_LNA_GAIN(pAd);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
		}
		else
#endif 
		{
			R66 = 0x2E + GET_LNA_GAIN(pAd);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
		}
	}
	else
	{	
		{
			if (BandWidth == BW_20)
			{
				R66 = (UCHAR)(0x32 + (GET_LNA_GAIN(pAd)*5)/3);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
			}
#ifdef DOT11_N_SUPPORT
			else
			{
				R66 = (UCHAR)(0x3A + (GET_LNA_GAIN(pAd)*5)/3);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
			}
#endif 
		}
	}

}
