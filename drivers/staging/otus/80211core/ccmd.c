










#include "cprecomp.h"
#include "../hal/hpreg.h"


u16_t zfWlanReset(zdev_t *dev);
u32_t zfUpdateRxRate(zdev_t *dev);


extern void zfiUsbRecv(zdev_t *dev, zbuf_t *buf);
extern void zfiUsbRegIn(zdev_t *dev, u32_t *rsp, u16_t rspLen);
extern void zfiUsbOutComplete(zdev_t *dev, zbuf_t *buf, u8_t status, u8_t *hdr);
extern void zfiUsbRegOutComplete(zdev_t *dev);
extern u16_t zfHpReinit(zdev_t *dev, u32_t frequency);




u16_t zfiGlobalDataSize(zdev_t *dev)
{
	u32_t ret;
	ret = (sizeof(struct zsWlanDev));
	zm_assert((ret>>16) == 0);
	return (u16_t)ret;
}




extern u16_t zfiWlanOpen(zdev_t *dev, struct zsCbFuncTbl *cbFuncTbl)
{
	
	u32_t devSize;
	struct zfCbUsbFuncTbl cbUsbFuncTbl;
	zmw_get_wlan_dev(dev);

	zm_debug_msg0("start");

	devSize = sizeof(struct zsWlanDev);
	
	zfZeroMemory((u8_t *)wd, (u16_t)devSize);

#ifdef ZM_ENABLE_AGGREGATION
	zfAggInit(dev);
#endif

	zfCwmInit(dev);

	wd->commTally.RateCtrlTxMPDU = 0;
	wd->commTally.RateCtrlBAFail = 0;
	wd->preambleTypeInUsed = ZM_PREAMBLE_TYPE_SHORT;

	if (cbFuncTbl == NULL) {
		
		zm_assert(0);
	} else {
		if (cbFuncTbl->zfcbRecvEth == NULL) {
			
			zm_assert(0);
		}
		wd->zfcbAuthNotify = cbFuncTbl->zfcbAuthNotify;
		wd->zfcbAuthNotify = cbFuncTbl->zfcbAuthNotify;
		wd->zfcbAsocNotify = cbFuncTbl->zfcbAsocNotify;
		wd->zfcbDisAsocNotify = cbFuncTbl->zfcbDisAsocNotify;
		wd->zfcbApConnectNotify = cbFuncTbl->zfcbApConnectNotify;
		wd->zfcbConnectNotify = cbFuncTbl->zfcbConnectNotify;
		wd->zfcbScanNotify = cbFuncTbl->zfcbScanNotify;
		wd->zfcbMicFailureNotify = cbFuncTbl->zfcbMicFailureNotify;
		wd->zfcbApMicFailureNotify = cbFuncTbl->zfcbApMicFailureNotify;
		wd->zfcbIbssPartnerNotify = cbFuncTbl->zfcbIbssPartnerNotify;
		wd->zfcbMacAddressNotify = cbFuncTbl->zfcbMacAddressNotify;
		wd->zfcbSendCompleteIndication =
					cbFuncTbl->zfcbSendCompleteIndication;
		wd->zfcbRecvEth = cbFuncTbl->zfcbRecvEth;
		wd->zfcbRestoreBufData = cbFuncTbl->zfcbRestoreBufData;
		wd->zfcbRecv80211 = cbFuncTbl->zfcbRecv80211;
#ifdef ZM_ENABLE_CENC
		wd->zfcbCencAsocNotify = cbFuncTbl->zfcbCencAsocNotify;
#endif 
		wd->zfcbClassifyTxPacket = cbFuncTbl->zfcbClassifyTxPacket;
		wd->zfcbHwWatchDogNotify = cbFuncTbl->zfcbHwWatchDogNotify;
	}

	
	cbUsbFuncTbl.zfcbUsbRecv = zfiUsbRecv;
	cbUsbFuncTbl.zfcbUsbRegIn = zfiUsbRegIn;
	cbUsbFuncTbl.zfcbUsbOutComplete = zfiUsbOutComplete;
	cbUsbFuncTbl.zfcbUsbRegOutComplete = zfiUsbRegOutComplete;
	zfwUsbRegisterCallBack(dev, &cbUsbFuncTbl);
	
	wd->macAddr[0] = 0x8000;
	wd->macAddr[1] = 0x0000;
	wd->macAddr[2] = 0x0000;

	wd->regulationTable.regionCode = 0xffff;

	zfHpInit(dev, wd->frequency);

	
	
	
	
	
	
#ifdef ZM_AP_DEBUG
	
#endif

	
	wd->sta.mTxRate = 0x0;
	wd->sta.uTxRate = 0x3;
	wd->sta.mmTxRate = 0x0;
	wd->sta.adapterState = ZM_STA_STATE_DISCONNECT;
	wd->sta.capability[0] = 0x01;
	wd->sta.capability[1] = 0x00;

	wd->sta.preambleTypeHT = 0;
	wd->sta.htCtrlBandwidth = 0;
	wd->sta.htCtrlSTBC = 0;
	wd->sta.htCtrlSG = 0;
	wd->sta.defaultTA = 0;
	
	{
		u8_t Dur = ZM_TIME_ACTIVE_SCAN;
		zfwGetActiveScanDur(dev, &Dur);
		wd->sta.activescanTickPerChannel = Dur / ZM_MS_PER_TICK;

	}
	wd->sta.passiveScanTickPerChannel = ZM_TIME_PASSIVE_SCAN/ZM_MS_PER_TICK;
	wd->sta.bAutoReconnect = TRUE;
	wd->sta.dropUnencryptedPkts = FALSE;

	
	wd->sta.bAllMulticast = 1;

	
	wd->sta.rifsState = ZM_RIFS_STATE_DETECTING;
	wd->sta.rifsLikeFrameCnt = 0;
	wd->sta.rifsCount = 0;

	wd->sta.osRxFilter = 0;
	wd->sta.bSafeMode = 0;

	
	zfResetSupportRate(dev, ZM_DEFAULT_SUPPORT_RATE_DISCONNECT);
	wd->beaconInterval = 100;
	wd->rtsThreshold = 2346;
	wd->fragThreshold = 32767;
	wd->wlanMode = ZM_MODE_INFRASTRUCTURE;
	wd->txMCS = 0xff;    
	wd->dtim = 1;
	
	wd->tick = 1;
	wd->maxTxPower2 = 0xff;
	wd->maxTxPower5 = 0xff;
	wd->supportMode = 0xffffffff;
	wd->ws.adhocMode = ZM_ADHOCBAND_G;
	wd->ws.autoSetFrequency = 0xff;

	
	
	wd->ap.ssidLen[0] = 6;
	wd->ap.ssid[0][0] = 'Z';
	wd->ap.ssid[0][1] = 'D';
	wd->ap.ssid[0][2] = '1';
	wd->ap.ssid[0][3] = '2';
	wd->ap.ssid[0][4] = '2';
	wd->ap.ssid[0][5] = '1';

	
	wd->ws.countryIsoName[0] = 0;
	wd->ws.countryIsoName[1] = 0;
	wd->ws.countryIsoName[2] = '\0';

	
	

	
	wd->swSniffer = 0;
	wd->XLinkMode = 0;

	
#if 1
	
	
	wd->ap.HTCap.Data.ElementID = ZM_WLAN_EID_HT_CAPABILITY;
	wd->ap.HTCap.Data.Length = 26;
	
	wd->ap.HTCap.Data.AMPDUParam |= HTCAP_MaxRxAMPDU3;
	wd->ap.HTCap.Data.MCSSet[0] = 0xFF; 
	wd->ap.HTCap.Data.MCSSet[1] = 0xFF; 

	
	wd->ap.ExtHTCap.Data.ElementID = ZM_WLAN_EID_EXTENDED_HT_CAPABILITY;
	wd->ap.ExtHTCap.Data.Length = 22;
	wd->ap.ExtHTCap.Data.ControlChannel = 6;
	
	wd->ap.ExtHTCap.Data.ChannelInfo |= ExtHtCap_RecomTxWidthSet;
	
	wd->ap.ExtHTCap.Data.OperatingInfo |= 1;

	
	
	wd->sta.HTCap.Data.ElementID = ZM_WLAN_EID_HT_CAPABILITY;
	wd->sta.HTCap.Data.Length = 26;

	
	
	wd->sta.HTCap.Data.HtCapInfo |= HTCAP_SMEnabled;
	wd->sta.HTCap.Data.HtCapInfo |= HTCAP_SupChannelWidthSet;
	wd->sta.HTCap.Data.HtCapInfo |= HTCAP_ShortGIfor40MHz;
	wd->sta.HTCap.Data.HtCapInfo |= HTCAP_DSSSandCCKin40MHz;
#ifndef ZM_DISABLE_AMSDU8K_SUPPORT
	wd->sta.HTCap.Data.HtCapInfo |= HTCAP_MaxAMSDULength;
#endif
	
	wd->sta.HTCap.Data.AMPDUParam |= HTCAP_MaxRxAMPDU3;
	wd->sta.HTCap.Data.MCSSet[0] = 0xFF; 
	wd->sta.HTCap.Data.MCSSet[1] = 0xFF; 
	wd->sta.HTCap.Data.PCO |= HTCAP_TransmissionTime3;
	
	
	wd->sta.ExtHTCap.Data.ElementID = ZM_WLAN_EID_EXTENDED_HT_CAPABILITY;
	wd->sta.ExtHTCap.Data.Length = 22;
	wd->sta.ExtHTCap.Data.ControlChannel = 6;

	
	wd->sta.ExtHTCap.Data.ChannelInfo |= ExtHtCap_ExtChannelOffsetBelow;

	
	
	wd->sta.ExtHTCap.Data.OperatingInfo |= 1;
#endif

#if 0
	
	wd->ap.qosMode[0] = 1;
#endif

	wd->ledStruct.ledMode[0] = 0x2221;
	wd->ledStruct.ledMode[1] = 0x2221;

	zfTimerInit(dev);

	ZM_PERFORMANCE_INIT(dev);

	zfBssInfoCreate(dev);
	zfScanMgrInit(dev);
	zfPowerSavingMgrInit(dev);

#if 0
	
	{
		u32_t key[4] = {0xffffffff, 0xff, 0, 0};
		u16_t addr[3] = {0x8000, 0x01ab, 0x0000};
		
	}
#endif

	
	wd->ws.staWmeEnabled = 1;           
#define ZM_UAPSD_Q_SIZE 32 
	wd->ap.uapsdQ = zfQueueCreate(dev, ZM_UAPSD_Q_SIZE);
	zm_assert(wd->ap.uapsdQ != NULL);
	wd->sta.uapsdQ = zfQueueCreate(dev, ZM_UAPSD_Q_SIZE);
	zm_assert(wd->sta.uapsdQ != NULL);

	

	
	
	zfHpGetMacAddress(dev);

	zfCoreSetFrequency(dev, wd->frequency);

#if ZM_PCI_LOOP_BACK == 1
	zfwWriteReg(dev, ZM_REG_PCI_CONTROL, 6);
#endif 

	
	
	wd->sta.DFSEnable = 1;
	wd->sta.capability[1] |= ZM_BIT_0;

	
	

	
	zfHpStartRecv(dev);

	zm_debug_msg0("end");

	return 0;
}


u16_t zfiWlanClose(zdev_t *dev)
{
	zmw_get_wlan_dev(dev);

	zm_msg0_init(ZM_LV_0, "enter");

	wd->state = ZM_WLAN_STATE_CLOSEDED;

	
	zfWlanReset(dev);

	zfHpStopRecv(dev);

	
	
	

	zfHpRelease(dev);

	zfQueueDestroy(dev, wd->ap.uapsdQ);
	zfQueueDestroy(dev, wd->sta.uapsdQ);

	zfBssInfoDestroy(dev);

#ifdef ZM_ENABLE_AGGREGATION
	
	zfAggRxFreeBuf(dev, 1);  
	
#endif

	zm_msg0_init(ZM_LV_0, "exit");

	return 0;
}

void zfGetWrapperSetting(zdev_t *dev)
{
	u8_t bPassive;
	u16_t vapId = 0;

	zmw_get_wlan_dev(dev);

	zmw_declare_for_critical_section();
#if 0
	if ((wd->ws.countryIsoName[0] != 0)
		|| (wd->ws.countryIsoName[1] != 0)
		|| (wd->ws.countryIsoName[2] != '\0')) {
		zfHpGetRegulationTablefromRegionCode(dev,
		zfHpGetRegionCodeFromIsoName(dev, wd->ws.countryIsoName));
	}
#endif
	zmw_enter_critical_section(dev);

	wd->wlanMode = wd->ws.wlanMode;

	
	if (wd->ws.frequency) {
		wd->frequency = wd->ws.frequency;
		wd->ws.frequency = 0;
	} else {
		wd->frequency = zfChGetFirstChannel(dev, &bPassive);

		if (wd->wlanMode == ZM_MODE_IBSS) {
			if (wd->ws.adhocMode == ZM_ADHOCBAND_A)
				wd->frequency = ZM_CH_A_36;
			else
				wd->frequency = ZM_CH_G_6;
		}
	}
#ifdef ZM_AP_DEBUG
	
	wd->frequency = 2437;
	
#endif

	
	switch (wd->ws.preambleType) {
	case ZM_PREAMBLE_TYPE_AUTO:
	case ZM_PREAMBLE_TYPE_SHORT:
	case ZM_PREAMBLE_TYPE_LONG:
		wd->preambleType = wd->ws.preambleType;
		break;
	default:
		wd->preambleType = ZM_PREAMBLE_TYPE_SHORT;
		break;
	}
	wd->ws.preambleType = 0;

	if (wd->wlanMode == ZM_MODE_AP) {
		vapId = zfwGetVapId(dev);

		if (vapId == 0xffff) {
			wd->ap.authAlgo[0] = wd->ws.authMode;
			wd->ap.encryMode[0] = wd->ws.encryMode;
		} else {
			wd->ap.authAlgo[vapId + 1] = wd->ws.authMode;
			wd->ap.encryMode[vapId + 1] = wd->ws.encryMode;
		}
		wd->ws.authMode = 0;
		wd->ws.encryMode = ZM_NO_WEP;

		
		if ((wd->ws.beaconInterval >= 20) &&
					(wd->ws.beaconInterval <= 1000))
			wd->beaconInterval = wd->ws.beaconInterval;
		else
			wd->beaconInterval = 100; 

		if (wd->ws.dtim > 0)
			wd->dtim = wd->ws.dtim;
		else
			wd->dtim = 1;


		wd->ap.qosMode = wd->ws.apWmeEnabled & 0x1;
		wd->ap.uapsdEnabled = (wd->ws.apWmeEnabled & 0x2) >> 1;
	} else {
		wd->sta.authMode = wd->ws.authMode;
		wd->sta.currentAuthMode = wd->ws.authMode;
		wd->sta.wepStatus = wd->ws.wepStatus;

		if (wd->ws.beaconInterval)
			wd->beaconInterval = wd->ws.beaconInterval;
		else
			wd->beaconInterval = 0x64;

		if (wd->wlanMode == ZM_MODE_IBSS) {
			
			

			
			if ((wd->ws.adhocMode == ZM_ADHOCBAND_G) ||
				(wd->ws.adhocMode == ZM_ADHOCBAND_BG) ||
				(wd->ws.adhocMode == ZM_ADHOCBAND_ABG))
				wd->wfc.bIbssGMode = 1;
			else
				wd->wfc.bIbssGMode = 0;

			
			
		}

		
		if (wd->ws.atimWindow)
			wd->sta.atimWindow = wd->ws.atimWindow;
		else {
			
			wd->sta.atimWindow = 0;
		}

		
		wd->sta.dropUnencryptedPkts = wd->ws.dropUnencryptedPkts;
		wd->sta.ibssJoinOnly = wd->ws.ibssJoinOnly;

		if (wd->ws.bDesiredBssid) {
			zfMemoryCopy(wd->sta.desiredBssid,
						wd->ws.desiredBssid, 6);
			wd->sta.bDesiredBssid = TRUE;
			wd->ws.bDesiredBssid = FALSE;
		} else
			wd->sta.bDesiredBssid = FALSE;

		
		if (wd->ws.ssidLen != 0) {
			if ((!zfMemoryIsEqual(wd->ws.ssid, wd->sta.ssid,
				wd->sta.ssidLen)) ||
				(wd->ws.ssidLen != wd->sta.ssidLen) ||
				(wd->sta.authMode == ZM_AUTH_MODE_WPA) ||
				(wd->sta.authMode == ZM_AUTH_MODE_WPAPSK) ||
				(wd->ws.staWmeQosInfo != 0)) {
				
				wd->sta.connectByReasso = FALSE;
				wd->sta.failCntOfReasso = 0;
				wd->sta.pmkidInfo.bssidCount = 0;

				wd->sta.ssidLen = wd->ws.ssidLen;
				zfMemoryCopy(wd->sta.ssid, wd->ws.ssid,
							wd->sta.ssidLen);

				if (wd->sta.ssidLen < 32)
					wd->sta.ssid[wd->sta.ssidLen] = 0;
			}
		} else {
			
			wd->sta.ssid[0] = 0;
			wd->sta.ssidLen = 0;
		}

		wd->sta.wmeEnabled = wd->ws.staWmeEnabled;
		wd->sta.wmeQosInfo = wd->ws.staWmeQosInfo;

	}

	zmw_leave_critical_section(dev);
}

u16_t zfWlanEnable(zdev_t *dev)
{
	u8_t bssid[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
	u16_t i;

	zmw_get_wlan_dev(dev);

	zmw_declare_for_critical_section();

	if (wd->wlanMode == ZM_MODE_UNKNOWN) {
		zm_debug_msg0("Unknown Mode...Skip...");
		return 0;
	}

	if (wd->wlanMode == ZM_MODE_AP) {
		u16_t vapId;

		vapId = zfwGetVapId(dev);

		if (vapId == 0xffff) {
			
			zfApInitStaTbl(dev);

			
			wd->bRate = 0xf;
			wd->gRate = 0xff;
			wd->bRateBasic = 0xf;
			wd->gRateBasic = 0x0;
			
			wd->ap.apBitmap = 1;
			wd->ap.beaconCounter = 0;
			

			wd->ap.hideSsid[0] = 0;
			wd->ap.staAgingTimeSec = 10*60;
			wd->ap.staProbingTimeSec = 60;

			for (i = 0; i < ZM_MAX_AP_SUPPORT; i++)
				wd->ap.bcmcHead[i] = wd->ap.bcmcTail[i] = 0;

			

			
			wd->bRateBasic = wd->ws.bRateBasic;
			wd->gRateBasic = wd->ws.gRateBasic;
			wd->bgMode = wd->ws.bgMode;
			if ((wd->ws.ssidLen <= 32) && (wd->ws.ssidLen != 0)) {
				wd->ap.ssidLen[0] = wd->ws.ssidLen;
				for (i = 0; i < wd->ws.ssidLen; i++)
					wd->ap.ssid[0][i] = wd->ws.ssid[i];
				wd->ws.ssidLen = 0; 
			}

			if (wd->ap.encryMode[0] == 0)
				wd->ap.capab[0] = 0x001;
			else
				wd->ap.capab[0] = 0x011;
			
			if (wd->ap.wlanType[0] != ZM_WLAN_TYPE_PURE_B)
				wd->ap.capab[0] |= 0x400;

			
		} else {
#if 0
			
			wd->ap.apBitmap = 0x3;
			wd->ap.capab[1] = 0x401;
			wd->ap.ssidLen[1] = 4;
			wd->ap.ssid[1][0] = 'v';
			wd->ap.ssid[1][1] = 'a';
			wd->ap.ssid[1][2] = 'p';
			wd->ap.ssid[1][3] = '1';
			wd->ap.authAlgo[1] = wd->ws.authMode;
			wd->ap.encryMode[1] = wd->ws.encryMode;
			wd->ap.vapNumber = 2;
#else
			
			wd->ap.apBitmap = 0x1 | (0x01 << (vapId+1));

			if ((wd->ws.ssidLen <= 32) && (wd->ws.ssidLen != 0)) {
				wd->ap.ssidLen[vapId+1] = wd->ws.ssidLen;
				for (i = 0; i < wd->ws.ssidLen; i++)
					wd->ap.ssid[vapId+1][i] =
								wd->ws.ssid[i];
				wd->ws.ssidLen = 0; 
			}

			if (wd->ap.encryMode[vapId+1] == 0)
				wd->ap.capab[vapId+1] = 0x401;
			else
				wd->ap.capab[vapId+1] = 0x411;

			wd->ap.authAlgo[vapId+1] = wd->ws.authMode;
			wd->ap.encryMode[vapId+1] = wd->ws.encryMode;

			
			
#endif
		}

		wd->ap.vapNumber++;

		zfCoreSetFrequency(dev, wd->frequency);

		zfInitMacApMode(dev);

		
		zfApSetProtectionMode(dev, 0);

		zfApSendBeacon(dev);
	} else { 

		zfScanMgrScanStop(dev, ZM_SCAN_MGR_SCAN_INTERNAL);
		zfScanMgrScanStop(dev, ZM_SCAN_MGR_SCAN_EXTERNAL);

		zmw_enter_critical_section(dev);
		wd->sta.oppositeCount = 0;    
		
		
		zfStaInitOppositeInfo(dev);
		zmw_leave_critical_section(dev);

		zfStaResetStatus(dev, 0);

		if ((wd->sta.cmDisallowSsidLength != 0) &&
		(wd->sta.ssidLen == wd->sta.cmDisallowSsidLength) &&
		(zfMemoryIsEqual(wd->sta.ssid, wd->sta.cmDisallowSsid,
		wd->sta.ssidLen)) &&
		(wd->sta.wepStatus == ZM_ENCRYPTION_TKIP)) {
			zm_debug_msg0("countermeasures disallow association");
		} else {
			switch (wd->wlanMode) {
			case ZM_MODE_IBSS:
				
				if (wd->sta.authMode == ZM_AUTH_MODE_WPA2PSK)
					zfHpSetApStaMode(dev,
					ZM_HAL_80211_MODE_IBSS_WPA2PSK);
				else
					zfHpSetApStaMode(dev,
					ZM_HAL_80211_MODE_IBSS_GENERAL);

				zm_msg0_mm(ZM_LV_0, "ZM_MODE_IBSS");
				zfIbssConnectNetwork(dev);
				break;

			case ZM_MODE_INFRASTRUCTURE:
				
				zfHpSetApStaMode(dev, ZM_HAL_80211_MODE_STA);

				zfInfraConnectNetwork(dev);
				break;

			case ZM_MODE_PSEUDO:
				
				zfHpSetApStaMode(dev, ZM_HAL_80211_MODE_STA);

				zfUpdateBssid(dev, bssid);
				zfCoreSetFrequency(dev, wd->frequency);
				break;

			default:
				break;
			}
		}

	}


	
	if (wd->wlanMode == ZM_MODE_PSEUDO) {
		
		zfWlanReset(dev);

		if (wd->zfcbConnectNotify != NULL)
			wd->zfcbConnectNotify(dev, ZM_STATUS_MEDIA_CONNECT,
								wd->sta.bssid);
		zfChangeAdapterState(dev, ZM_STA_STATE_CONNECTED);
	}


	if (wd->wlanMode == ZM_MODE_AP) {
		if (wd->zfcbConnectNotify != NULL)
			wd->zfcbConnectNotify(dev, ZM_STATUS_MEDIA_CONNECT,
								wd->sta.bssid);
		
	}

	
	if (wd->sta.EnableHT) {
		u32_t oneTxStreamCap;
		oneTxStreamCap = (zfHpCapability(dev) &
						ZM_HP_CAP_11N_ONE_TX_STREAM);
		if (oneTxStreamCap)
			wd->CurrentTxRateKbps = 135000;
		else
			wd->CurrentTxRateKbps = 270000;
		wd->CurrentRxRateKbps = 270000;
	} else {
		wd->CurrentTxRateKbps = 54000;
		wd->CurrentRxRateKbps = 54000;
	}

	wd->state = ZM_WLAN_STATE_ENABLED;

	return 0;
}


u16_t zfiWlanEnable(zdev_t *dev)
{
	u16_t ret;

	zmw_get_wlan_dev(dev);

	zm_msg0_mm(ZM_LV_1, "Enable Wlan");

	zfGetWrapperSetting(dev);

	zfZeroMemory((u8_t *) &wd->trafTally, sizeof(struct zsTrafTally));

	
	if (wd->sta.cmMicFailureCount == 1) {
		zfTimerCancel(dev, ZM_EVENT_CM_TIMER);
		wd->sta.cmMicFailureCount = 0;
	}

	zfFlushVtxq(dev);
	if ((wd->queueFlushed & 0x10) != 0)
		zfHpUsbReset(dev);

	ret = zfWlanEnable(dev);

	return ret;
}

u16_t zfiWlanDisable(zdev_t *dev, u8_t ResetKeyCache)
{
	u16_t  i;
	u8_t isConnected;

	zmw_get_wlan_dev(dev);

#ifdef ZM_ENABLE_IBSS_WPA2PSK
	zmw_declare_for_critical_section();
#endif
	wd->state = ZM_WLAN_STATE_DISABLED;

	zm_msg0_mm(ZM_LV_1, "Disable Wlan");

	if (wd->wlanMode != ZM_MODE_AP) {
		isConnected = zfStaIsConnected(dev);

		if ((wd->wlanMode == ZM_MODE_INFRASTRUCTURE) &&
			(wd->sta.currentAuthMode != ZM_AUTH_MODE_WPA2)) {
			
			if (isConnected) {
				
				zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_DEAUTH,
						wd->sta.bssid, 3, 0, 0);
				
			}
		}

		
		if (wd->wlanMode == ZM_MODE_IBSS) {
			wd->sta.ibssBssIsCreator = 0;
			zfTimerCancel(dev, ZM_EVENT_IBSS_MONITOR);
			zfStaIbssMonitoring(dev, 1);
		}

#ifdef ZM_ENABLE_IBSS_WPA2PSK
		zmw_enter_critical_section(dev);
		wd->sta.ibssWpa2Psk = 0;
		zmw_leave_critical_section(dev);
#endif

		wd->sta.wpaState = ZM_STA_WPA_STATE_INIT;

		
		wd->sta.connectTimeoutCount = 0;

		
		wd->sta.connectState = ZM_STA_CONN_STATE_NONE;

		
		wd->sta.leapEnabled = 0;

		
		if (wd->sta.rifsState == ZM_RIFS_STATE_DETECTED)
			zfHpDisableRifs(dev);
		wd->sta.rifsState = ZM_RIFS_STATE_DETECTING;
		wd->sta.rifsLikeFrameCnt = 0;
		wd->sta.rifsCount = 0;

		wd->sta.osRxFilter = 0;
		wd->sta.bSafeMode = 0;

		zfChangeAdapterState(dev, ZM_STA_STATE_DISCONNECT);
		if (ResetKeyCache)
			zfHpResetKeyCache(dev);

		if (isConnected) {
			if (wd->zfcbConnectNotify != NULL)
				wd->zfcbConnectNotify(dev,
				ZM_STATUS_MEDIA_CONNECTION_DISABLED,
				wd->sta.bssid);
		} else {
			if (wd->zfcbConnectNotify != NULL)
				wd->zfcbConnectNotify(dev,
				ZM_STATUS_MEDIA_DISABLED, wd->sta.bssid);
		}
	} else { 
		for (i = 0; i < ZM_MAX_STA_SUPPORT; i++) {
			
			if (wd->ap.staTable[i].valid == 1) {
				
				zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_DEAUTH,
				wd->ap.staTable[i].addr, 3, 0, 0);
			}
		}

		if (ResetKeyCache)
			zfHpResetKeyCache(dev);

		wd->ap.vapNumber--;
	}

	
	zfHpDisableBeacon(dev);

	
	zfFlushVtxq(dev);
	
	zfApFlushBufferedPsFrame(dev);
	
	zfAgingDefragList(dev, 1);

#ifdef ZM_ENABLE_AGGREGATION
	
	zfAggRxFreeBuf(dev, 0);  
	
#endif

	
	zfZeroMemory((u8_t *)wd->sta.oppositeInfo,
			sizeof(struct zsOppositeInfo) * ZM_MAX_OPPOSITE_COUNT);

	
	if (wd->sta.SWEncryptEnable != 0) {
		zm_debug_msg0("Disable software encryption");
		zfStaDisableSWEncryption(dev);
	}

	
	

	return 0;
}

u16_t zfiWlanSuspend(zdev_t *dev)
{
	zmw_get_wlan_dev(dev);
	zmw_declare_for_critical_section();

	
	zmw_enter_critical_section(dev);
	wd->halState = ZM_HAL_STATE_INIT;
	zmw_leave_critical_section(dev);

	return 0;
}

u16_t zfiWlanResume(zdev_t *dev, u8_t doReconn)
{
	u16_t ret;
	zmw_get_wlan_dev(dev);
	zmw_declare_for_critical_section();

	
	zfHpReinit(dev, wd->frequency);

	
	zfCoreSetFrequencyExV2(dev, wd->frequency, wd->BandWidth40,
		wd->ExtOffset, NULL, 1);

	zfHpSetMacAddress(dev, wd->macAddr, 0);

	
	zfHpStartRecv(dev);

	zfFlushVtxq(dev);

	if (wd->wlanMode != ZM_MODE_INFRASTRUCTURE &&
			wd->wlanMode != ZM_MODE_IBSS)
		return 1;

	zm_msg0_mm(ZM_LV_1, "Resume Wlan");
	if ((zfStaIsConnected(dev)) || (zfStaIsConnecting(dev))) {
		if (doReconn == 1) {
			zm_msg0_mm(ZM_LV_1, "Re-connect...");
			zmw_enter_critical_section(dev);
			wd->sta.connectByReasso = FALSE;
			zmw_leave_critical_section(dev);

			zfWlanEnable(dev);
		} else if (doReconn == 0)
			zfHpSetRollCallTable(dev);
	}

	ret = 0;

	return ret;
}
















void zfiWlanFlushAllQueuedBuffers(zdev_t *dev)
{
	
	zfFlushVtxq(dev);
	
	zfApFlushBufferedPsFrame(dev);
	
	zfAgingDefragList(dev, 1);
}


u16_t zfiWlanScan(zdev_t *dev)
{
	u16_t ret = 1;
	zmw_get_wlan_dev(dev);

	zm_debug_msg0("");

	zmw_declare_for_critical_section();

	zmw_enter_critical_section(dev);

	if (wd->wlanMode == ZM_MODE_AP) {
		wd->heartBeatNotification |= ZM_BSSID_LIST_SCAN;
		wd->sta.scanFrequency = 0;
		
		ret = 0;
	} else {
#if 0
		if (!zfStaBlockWlanScan(dev)) {
			zm_debug_msg0("scan request");
			
			ret = 0;
			goto start_scan;
		}
#else
		goto start_scan;
#endif
	}

	zmw_leave_critical_section(dev);

	return ret;

start_scan:
	zmw_leave_critical_section(dev);

	if (wd->ledStruct.LEDCtrlFlagFromReg & ZM_LED_CTRL_FLAG_ALPHA) {
		
		wd->ledStruct.LEDCtrlFlag |= ZM_LED_CTRL_FLAG_ALPHA;
	}

	ret = zfScanMgrScanStart(dev, ZM_SCAN_MGR_SCAN_EXTERNAL);

	zm_debug_msg1("ret = ", ret);

	return ret;
}


















u16_t zcRateToMCS[] =
    {0xff, 0, 1, 2, 3, 0xb, 0xf, 0xa, 0xe, 0x9, 0xd, 0x8, 0xc};
u16_t zcRateToMT[] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1};

u16_t zfiWlanSetTxRate(zdev_t *dev, u16_t rate)
{
	
	zmw_get_wlan_dev(dev);

	if (rate <= 12) {
		wd->txMCS = zcRateToMCS[rate];
		wd->txMT = zcRateToMT[rate];
		return ZM_SUCCESS;
	} else if ((rate <= 28) || (rate == 13 + 32)) {
		wd->txMCS = rate - 12 - 1;
		wd->txMT = 2;
		return ZM_SUCCESS;
	}

	return ZM_ERR_INVALID_TX_RATE;
}

const u32_t zcRateIdToKbps40M[] =
{
	1000, 2000, 5500, 11000, 
	6000, 9000, 12000, 18000, 
	24000, 36000, 48000, 54000, 
	13500, 27000, 40500, 54000, 
	81000, 108000, 121500, 135000, 
	27000, 54000, 81000, 108000, 
	162000, 216000, 243000, 270000, 
	270000, 300000, 150000  
};

const u32_t zcRateIdToKbps20M[] =
{
	1000, 2000, 5500, 11000, 
	6000, 9000, 12000, 18000, 
	24000, 36000, 48000, 54000, 
	6500, 13000, 19500, 26000, 
	39000, 52000, 58500, 65000, 
	13000, 26000, 39000, 52000, 
	78000, 104000, 117000, 130000, 
	130000, 144400, 72200  
};

u32_t zfiWlanQueryTxRate(zdev_t *dev)
{
	u8_t rateId = 0xff;
	zmw_get_wlan_dev(dev);
	zmw_declare_for_critical_section();

	
	if ((wd->wlanMode == ZM_MODE_INFRASTRUCTURE) &&
						(zfStaIsConnected(dev))) {
		zmw_enter_critical_section(dev);
		
		if (wd->txMCS == 0xff) {
			if ((wd->sta.oppositeInfo[0].rcCell.flag &
							ZM_RC_TRAINED_BIT) == 0)
				rateId = wd->sta.oppositeInfo[0].rcCell. \
				operationRateSet[wd->sta.oppositeInfo[0]. \
				rcCell.operationRateCount-1];
			else
				rateId = wd->sta.oppositeInfo[0].rcCell. \
				operationRateSet[wd->sta.oppositeInfo[0]. \
				rcCell.currentRateIndex];
		}
		zmw_leave_critical_section(dev);
	}

	if (rateId != 0xff) {
		if (wd->sta.htCtrlBandwidth)
			return zcRateIdToKbps40M[rateId];
		else
			return zcRateIdToKbps20M[rateId];
	} else
		return wd->CurrentTxRateKbps;
}

void zfWlanUpdateRxRate(zdev_t *dev, struct zsAdditionInfo *addInfo)
{
	u32_t rxRateKbps;
	zmw_get_wlan_dev(dev);
	

	
	
	
	
	
	
	
	
	if ((addInfo->Tail.Data.RxMacStatus & 0x10) == 0) {
		
		wd->modulationType = addInfo->Tail.Data.RxMacStatus & 0x03;
		switch (wd->modulationType) {
		
		case 0x0:
			wd->rateField = addInfo->PlcpHeader[0] & 0xff;
			wd->rxInfo = 0;
			break;
		
		case 0x1:
			wd->rateField = addInfo->PlcpHeader[0] & 0x0f;
			wd->rxInfo = 0;
			break;
		
		case 0x2:
			wd->rateField = addInfo->PlcpHeader[3];
			wd->rxInfo = addInfo->PlcpHeader[6];
			break;
		default:
			break;
		}

		rxRateKbps = zfUpdateRxRate(dev);
		if (wd->CurrentRxRateUpdated == 1) {
			if (rxRateKbps > wd->CurrentRxRateKbps)
				wd->CurrentRxRateKbps = rxRateKbps;
		} else {
			wd->CurrentRxRateKbps = rxRateKbps;
			wd->CurrentRxRateUpdated = 1;
		}
	}
}

#if 0
u16_t zcIndextoRateBG[16] = {1000, 2000, 5500, 11000, 0, 0, 0, 0, 48000,
			24000, 12000, 6000, 54000, 36000, 18000, 9000};
u32_t zcIndextoRateN20L[16] = {6500, 13000, 19500, 26000, 39000, 52000, 58500,
			65000, 13000, 26000, 39000, 52000, 78000, 104000,
			117000, 130000};
u32_t zcIndextoRateN20S[16] = {7200, 14400, 21700, 28900, 43300, 57800, 65000,
			72200, 14400, 28900, 43300, 57800, 86700, 115600,
			130000, 144400};
u32_t zcIndextoRateN40L[16] = {13500, 27000, 40500, 54000, 81000, 108000,
			121500, 135000, 27000, 54000, 81000, 108000,
			162000, 216000, 243000, 270000};
u32_t zcIndextoRateN40S[16] = {15000, 30000, 45000, 60000, 90000, 120000,
			135000, 150000, 30000, 60000, 90000, 120000,
			180000, 240000, 270000, 300000};
#endif

extern u16_t zcIndextoRateBG[16];
extern u32_t zcIndextoRateN20L[16];
extern u32_t zcIndextoRateN20S[16];
extern u32_t zcIndextoRateN40L[16];
extern u32_t zcIndextoRateN40S[16];

u32_t zfiWlanQueryRxRate(zdev_t *dev)
{
	zmw_get_wlan_dev(dev);

	wd->CurrentRxRateUpdated = 0;
	return wd->CurrentRxRateKbps;
}

u32_t zfUpdateRxRate(zdev_t *dev)
{
	u8_t mcs, bandwidth;
	u32_t rxRateKbps = 130000;
	zmw_get_wlan_dev(dev);

	switch (wd->modulationType) {
	
	case 0x0:
		switch (wd->rateField) {
		case 0x0a:
			rxRateKbps = 1000;
			break;
		case 0x14:
			rxRateKbps = 2000;

		case 0x37:
			rxRateKbps = 5500;
			break;
		case 0x6e:
			rxRateKbps = 11000;
			break;
		default:
			break;
		}
		break;
	
	case 0x1:
		if (wd->rateField <= 15)
			rxRateKbps = zcIndextoRateBG[wd->rateField];
		break;
	
	case 0x2:
		mcs = wd->rateField & 0x7F;
		bandwidth = wd->rateField & 0x80;
		if (mcs <= 15) {
			if (bandwidth != 0) {
				if ((wd->rxInfo & 0x80) != 0) {
					
					rxRateKbps = zcIndextoRateN40S[mcs];
				} else {
					
					rxRateKbps = zcIndextoRateN40L[mcs];
				}
			} else {
				if ((wd->rxInfo & 0x80) != 0) {
					
					rxRateKbps = zcIndextoRateN20S[mcs];
				} else {
					
					rxRateKbps = zcIndextoRateN20L[mcs];
				}
			}
		}
		break;
	default:
		break;
	}
	

	
	return rxRateKbps;
}


u16_t zfiWlanGetStatistics(zdev_t *dev)
{
	
	return 0;
}

u16_t zfiWlanReset(zdev_t *dev)
{
	zmw_get_wlan_dev(dev);

	wd->state = ZM_WLAN_STATE_DISABLED;

	return zfWlanReset(dev);
}


u16_t zfWlanReset(zdev_t *dev)
{
	u8_t isConnected;
	zmw_get_wlan_dev(dev);

	zmw_declare_for_critical_section();

	zm_debug_msg0("zfWlanReset");

	isConnected = zfStaIsConnected(dev);

	
	{
		if ((wd->wlanMode == ZM_MODE_INFRASTRUCTURE) &&
		(wd->sta.currentAuthMode != ZM_AUTH_MODE_WPA2)) {
			
			if (isConnected) {
				
				zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_DEAUTH,
						wd->sta.bssid, 3, 0, 0);
				
			}
		}
	}

	zfChangeAdapterState(dev, ZM_STA_STATE_DISCONNECT);
	zfHpResetKeyCache(dev);

	if (isConnected) {
		
		if (wd->zfcbConnectNotify != NULL)
			wd->zfcbConnectNotify(dev,
			ZM_STATUS_MEDIA_CONNECTION_RESET, wd->sta.bssid);
	} else {
		if (wd->zfcbConnectNotify != NULL)
			wd->zfcbConnectNotify(dev, ZM_STATUS_MEDIA_RESET,
								wd->sta.bssid);
	}

	
	zfHpDisableBeacon(dev);

	
	zfAgingDefragList(dev, 1);

	
	zfFlushVtxq(dev);

#ifdef ZM_ENABLE_AGGREGATION
	
	zfAggRxFreeBuf(dev, 0);  
	
#endif

	zfStaRefreshBlockList(dev, 1);

	zmw_enter_critical_section(dev);

	zfTimerCancel(dev, ZM_EVENT_IBSS_MONITOR);
	zfTimerCancel(dev, ZM_EVENT_CM_BLOCK_TIMER);
	zfTimerCancel(dev, ZM_EVENT_CM_DISCONNECT);

	wd->sta.connectState = ZM_STA_CONN_STATE_NONE;
	wd->sta.connectByReasso = FALSE;
	wd->sta.cmDisallowSsidLength = 0;
	wd->sta.bAutoReconnect = 0;
	wd->sta.InternalScanReq = 0;
	wd->sta.encryMode = ZM_NO_WEP;
	wd->sta.wepStatus = ZM_ENCRYPTION_WEP_DISABLED;
	wd->sta.wpaState = ZM_STA_WPA_STATE_INIT;
	wd->sta.cmMicFailureCount = 0;
	wd->sta.ibssBssIsCreator = 0;
#ifdef ZM_ENABLE_IBSS_WPA2PSK
	wd->sta.ibssWpa2Psk = 0;
#endif
	
	wd->sta.connectTimeoutCount = 0;

	
	wd->sta.leapEnabled = 0;

	
	if (wd->sta.rifsState == ZM_RIFS_STATE_DETECTED)
		zfHpDisableRifs(dev);
	wd->sta.rifsState = ZM_RIFS_STATE_DETECTING;
	wd->sta.rifsLikeFrameCnt = 0;
	wd->sta.rifsCount = 0;

	wd->sta.osRxFilter = 0;
	wd->sta.bSafeMode = 0;

	
	zfZeroMemory((u8_t *)wd->sta.oppositeInfo,
			sizeof(struct zsOppositeInfo) * ZM_MAX_OPPOSITE_COUNT);

	zmw_leave_critical_section(dev);

	zfScanMgrScanStop(dev, ZM_SCAN_MGR_SCAN_INTERNAL);
	zfScanMgrScanStop(dev, ZM_SCAN_MGR_SCAN_EXTERNAL);

	
	if (wd->sta.SWEncryptEnable != 0) {
		zm_debug_msg0("Disable software encryption");
		zfStaDisableSWEncryption(dev);
	}

	
	

	
	if (wd->wlanMode != ZM_MODE_PSEUDO)
		wd->wlanMode = ZM_MODE_INFRASTRUCTURE;

	return 0;
}


u16_t zfiWlanDeauth(zdev_t *dev, u16_t *macAddr, u16_t reason)
{
	zmw_get_wlan_dev(dev);

	if (wd->wlanMode == ZM_MODE_AP) {
		

		

		

		zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_DEAUTH, macAddr,
								reason, 0, 0);
	} else
		zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_DEAUTH,
						wd->sta.bssid, 3, 0, 0);

	
	return 0;
}






void zfiWlanSetAllMulticast(zdev_t *dev, u32_t setting)
{
	zmw_get_wlan_dev(dev);
	zm_msg1_mm(ZM_LV_0, "sta.bAllMulticast = ", setting);
	wd->sta.bAllMulticast = (u8_t)setting;
}



void zfiWlanSetHTCtrl(zdev_t *dev, u32_t *setting, u32_t forceTxTPC)
{
	zmw_get_wlan_dev(dev);

	wd->preambleType        = (u8_t)setting[0];
	wd->sta.preambleTypeHT  = (u8_t)setting[1];
	wd->sta.htCtrlBandwidth = (u8_t)setting[2];
	wd->sta.htCtrlSTBC      = (u8_t)setting[3];
	wd->sta.htCtrlSG        = (u8_t)setting[4];
	wd->sta.defaultTA       = (u8_t)setting[5];
	wd->enableAggregation   = (u8_t)setting[6];
	wd->enableWDS           = (u8_t)setting[7];

	wd->forceTxTPC          = forceTxTPC;
}


void zfiWlanQueryHTCtrl(zdev_t *dev, u32_t *setting, u32_t *forceTxTPC)
{
	zmw_get_wlan_dev(dev);

	setting[0] = wd->preambleType;
	setting[1] = wd->sta.preambleTypeHT;
	setting[2] = wd->sta.htCtrlBandwidth;
	setting[3] = wd->sta.htCtrlSTBC;
	setting[4] = wd->sta.htCtrlSG;
	setting[5] = wd->sta.defaultTA;
	setting[6] = wd->enableAggregation;
	setting[7] = wd->enableWDS;

	*forceTxTPC = wd->forceTxTPC;
}

void zfiWlanDbg(zdev_t *dev, u8_t setting)
{
	zmw_get_wlan_dev(dev);

	wd->enableHALDbgInfo = setting;
}


void zfiWlanSetRxPacketDump(zdev_t *dev, u32_t setting)
{
	zmw_get_wlan_dev(dev);
	if (setting)
		wd->rxPacketDump = 1;   
	else
		wd->rxPacketDump = 0;   
}




void zfiWlanResetTally(zdev_t *dev)
{
	zmw_get_wlan_dev(dev);

	zmw_declare_for_critical_section();

	zmw_enter_critical_section(dev);

	wd->commTally.txUnicastFrm = 0;		
	wd->commTally.txMulticastFrm = 0;	
	wd->commTally.txUnicastOctets = 0;	
	wd->commTally.txMulticastOctets = 0;	
	wd->commTally.txFrmUpperNDIS = 0;
	wd->commTally.txFrmDrvMgt = 0;
	wd->commTally.RetryFailCnt = 0;
	wd->commTally.Hw_TotalTxFrm = 0;	
	wd->commTally.Hw_RetryCnt = 0;		
	wd->commTally.Hw_UnderrunCnt = 0;
	wd->commTally.DriverRxFrmCnt = 0;
	wd->commTally.rxUnicastFrm = 0;		
	wd->commTally.rxMulticastFrm = 0;	
	wd->commTally.NotifyNDISRxFrmCnt = 0;
	wd->commTally.rxUnicastOctets = 0;	
	wd->commTally.rxMulticastOctets = 0;	
	wd->commTally.DriverDiscardedFrm = 0;	
	wd->commTally.LessThanDataMinLen = 0;
	wd->commTally.GreaterThanMaxLen = 0;
	wd->commTally.DriverDiscardedFrmCauseByMulticastList = 0;
	wd->commTally.DriverDiscardedFrmCauseByFrmCtrl = 0;
	wd->commTally.rxNeedFrgFrm = 0;		
	wd->commTally.DriverRxMgtFrmCnt = 0;
	wd->commTally.rxBroadcastFrm = 0;
	wd->commTally.rxBroadcastOctets = 0;
	wd->commTally.Hw_TotalRxFrm = 0;
	wd->commTally.Hw_CRC16Cnt = 0;		
	wd->commTally.Hw_CRC32Cnt = 0;		
	wd->commTally.Hw_DecrypErr_UNI = 0;
	wd->commTally.Hw_DecrypErr_Mul = 0;
	wd->commTally.Hw_RxFIFOOverrun = 0;
	wd->commTally.Hw_RxTimeOut = 0;
	wd->commTally.LossAP = 0;

	wd->commTally.Tx_MPDU = 0;
	wd->commTally.BA_Fail = 0;
	wd->commTally.Hw_Tx_AMPDU = 0;
	wd->commTally.Hw_Tx_MPDU = 0;

	wd->commTally.txQosDropCount[0] = 0;
	wd->commTally.txQosDropCount[1] = 0;
	wd->commTally.txQosDropCount[2] = 0;
	wd->commTally.txQosDropCount[3] = 0;
	wd->commTally.txQosDropCount[4] = 0;

	wd->commTally.Hw_RxMPDU = 0;
	wd->commTally.Hw_RxDropMPDU = 0;
	wd->commTally.Hw_RxDelMPDU = 0;

	wd->commTally.Hw_RxPhyMiscError = 0;
	wd->commTally.Hw_RxPhyXRError = 0;
	wd->commTally.Hw_RxPhyOFDMError = 0;
	wd->commTally.Hw_RxPhyCCKError = 0;
	wd->commTally.Hw_RxPhyHTError = 0;
	wd->commTally.Hw_RxPhyTotalCount = 0;

#if (defined(GCCK) && defined(OFDM))
	wd->commTally.rx11bDataFrame = 0;
	wd->commTally.rxOFDMDataFrame = 0;
#endif

	zmw_leave_critical_section(dev);
}


void zfiWlanQueryTally(zdev_t *dev, struct zsCommTally *tally)
{
	zmw_get_wlan_dev(dev);

	zmw_declare_for_critical_section();

	zmw_enter_critical_section(dev);
	zfMemoryCopy((u8_t *)tally, (u8_t *)&wd->commTally,
						sizeof(struct zsCommTally));
	zmw_leave_critical_section(dev);
}

void zfiWlanQueryTrafTally(zdev_t *dev, struct zsTrafTally *tally)
{
	zmw_get_wlan_dev(dev);

	zmw_declare_for_critical_section();

	zmw_enter_critical_section(dev);
	zfMemoryCopy((u8_t *)tally, (u8_t *)&wd->trafTally,
						sizeof(struct zsTrafTally));
	zmw_leave_critical_section(dev);
}

void zfiWlanQueryMonHalRxInfo(zdev_t *dev, struct zsMonHalRxInfo *monHalRxInfo)
{
	zfHpQueryMonHalRxInfo(dev, (u8_t *)monHalRxInfo);
}


void zfiDKEnable(zdev_t *dev, u32_t enable)
{
	zmw_get_wlan_dev(dev);

	wd->modeMDKEnable = enable;
	zm_debug_msg1("modeMDKEnable = ", wd->modeMDKEnable);
}


u32_t zfiWlanQueryPacketTypePromiscuous(zdev_t *dev)
{
	zmw_get_wlan_dev(dev);

	return wd->swSniffer;
}


void zfiWlanSetPacketTypePromiscuous(zdev_t *dev, u32_t setValue)
{
	zmw_get_wlan_dev(dev);

	wd->swSniffer = setValue;
	zm_msg1_mm(ZM_LV_0, "wd->swSniffer ", wd->swSniffer);
	if (setValue) {
		
		zfHpSetSnifferMode(dev, 1);
		zm_msg0_mm(ZM_LV_1, "enalbe sniffer mode");
	} else {
		zfHpSetSnifferMode(dev, 0);
		zm_msg0_mm(ZM_LV_0, "disalbe sniffer mode");
	}
}

void zfiWlanSetXLinkMode(zdev_t *dev, u32_t setValue)
{
	zmw_get_wlan_dev(dev);

	wd->XLinkMode = setValue;
	if (setValue) {
		
		zfHpSetSnifferMode(dev, 1);
	} else
		zfHpSetSnifferMode(dev, 0);
}

extern void zfStaChannelManagement(zdev_t *dev, u8_t scan);

void zfiSetChannelManagement(zdev_t *dev, u32_t setting)
{
	zmw_get_wlan_dev(dev);

	switch (setting) {
	case 1:
		wd->sta.EnableHT = 1;
		wd->BandWidth40 = 1;
		wd->ExtOffset   = 1;
		break;
	case 3:
		wd->sta.EnableHT = 1;
		wd->BandWidth40 = 1;
		wd->ExtOffset   = 3;
		break;
	case 0:
		wd->sta.EnableHT = 1;
		wd->BandWidth40 = 0;
		wd->ExtOffset   = 0;
		break;
	default:
		wd->BandWidth40 = 0;
		wd->ExtOffset   = 0;
		break;
	}

	zfCoreSetFrequencyEx(dev, wd->frequency, wd->BandWidth40,
							wd->ExtOffset, NULL);
}

void zfiSetRifs(zdev_t *dev, u16_t setting)
{
	zmw_get_wlan_dev(dev);

	wd->sta.ie.HtInfo.ChannelInfo |= ExtHtCap_RIFSMode;
	wd->sta.EnableHT = 1;

	switch (setting) {
	case 0:
		wd->sta.HT2040 = 0;
		
		break;
	case 1:
		wd->sta.HT2040 = 1;
		
		break;
	default:
		wd->sta.HT2040 = 0;
		
		break;
	}
}

void zfiCheckRifs(zdev_t *dev)
{
	zmw_get_wlan_dev(dev);

	if (wd->sta.ie.HtInfo.ChannelInfo & ExtHtCap_RIFSMode)
		;
		
}

void zfiSetReorder(zdev_t *dev, u16_t value)
{
	zmw_get_wlan_dev(dev);

	wd->reorder = value;
}

void zfiSetSeqDebug(zdev_t *dev, u16_t value)
{
	zmw_get_wlan_dev(dev);

	wd->seq_debug = value;
}
