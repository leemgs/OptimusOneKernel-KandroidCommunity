

#include "cprecomp.h"

void zfScanMgrInit(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    wd->sta.scanMgr.scanReqs[0] = 0;
    wd->sta.scanMgr.scanReqs[1] = 0;

    wd->sta.scanMgr.currScanType = ZM_SCAN_MGR_SCAN_NONE;
    wd->sta.scanMgr.scanStartDelay = 3;
    
}

u8_t zfScanMgrScanStart(zdev_t* dev, u8_t scanType)
{
    u8_t i;

    zmw_get_wlan_dev(dev);

    zm_debug_msg1("scanType = ", scanType);

    zmw_declare_for_critical_section();

    if ( scanType != ZM_SCAN_MGR_SCAN_INTERNAL &&
         scanType != ZM_SCAN_MGR_SCAN_EXTERNAL )
    {
        zm_debug_msg0("unknown scanType");
        return 1;
    }
    else if (zfStaIsConnecting(dev))
    {
        zm_debug_msg0("reject scan request due to connecting");
        return 1;
    }

    i = scanType - 1;

    zmw_enter_critical_section(dev);

    if ( wd->sta.scanMgr.scanReqs[i] == 1 )
    {
        zm_debug_msg1("scan rescheduled", scanType);
        goto scan_done;
    }

    wd->sta.scanMgr.scanReqs[i] = 1;
    zm_debug_msg1("scan scheduled: ", scanType);

    
    
    
    if ( wd->sta.scanMgr.currScanType == ZM_SCAN_MGR_SCAN_NONE )
    {
        goto schedule_scan;
    }
    else if ( wd->sta.scanMgr.currScanType == ZM_SCAN_MGR_SCAN_INTERNAL &&
              scanType == ZM_SCAN_MGR_SCAN_EXTERNAL )
    {
        
        zfTimerCancel(dev, ZM_EVENT_SCAN);

        
        wd->sta.bScheduleScan = FALSE;

        zfTimerCancel(dev, ZM_EVENT_TIMEOUT_SCAN);
        zfTimerCancel(dev, ZM_EVENT_IN_SCAN);

        wd->sta.bChannelScan = FALSE;
        goto schedule_scan;
    }
    else
    {
        zm_debug_msg0("Scan is busy...waiting later to start\n");
    }

    zmw_leave_critical_section(dev);
    return 0;

scan_done:
    zmw_leave_critical_section(dev);
    return 1;

schedule_scan:

    wd->sta.bScheduleScan = TRUE;

    zfTimerSchedule(dev, ZM_EVENT_SCAN, wd->sta.scanMgr.scanStartDelay);
    wd->sta.scanMgr.scanStartDelay = 3;
    
    wd->sta.scanMgr.currScanType = scanType;
    zmw_leave_critical_section(dev);

    if ((zfStaIsConnected(dev)) && (!zfPowerSavingMgrIsSleeping(dev)))
    {
        zfSendNullData(dev, 1);
    }
    return 0;
}

void zfScanMgrScanStop(zdev_t* dev, u8_t scanType)
{
    u8_t scanNotifyRequired = 0;
    u8_t theOtherScan = ZM_SCAN_MGR_SCAN_NONE;

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);

    if ( wd->sta.scanMgr.currScanType == ZM_SCAN_MGR_SCAN_NONE )
    {
        zm_assert(wd->sta.scanMgr.scanReqs[0] == 0);
        zm_assert(wd->sta.scanMgr.scanReqs[1] == 0);
        goto done;
    }

    switch(scanType)
    {
        case ZM_SCAN_MGR_SCAN_EXTERNAL:
            scanNotifyRequired = 1;
            theOtherScan = ZM_SCAN_MGR_SCAN_INTERNAL;
            break;

        case ZM_SCAN_MGR_SCAN_INTERNAL:
            theOtherScan = ZM_SCAN_MGR_SCAN_EXTERNAL;
            break;

        default:
            goto done;
    }

    if ( wd->sta.scanMgr.currScanType != scanType )
    {
        goto stop_done;
    }

    zfTimerCancel(dev, ZM_EVENT_SCAN);

    
    wd->sta.bScheduleScan = FALSE;

    zfTimerCancel(dev, ZM_EVENT_TIMEOUT_SCAN);
    zfTimerCancel(dev, ZM_EVENT_IN_SCAN);

    wd->sta.bChannelScan = FALSE;
    wd->sta.scanFrequency = 0;

    if ( wd->sta.scanMgr.scanReqs[theOtherScan - 1] )
    {
        wd->sta.scanMgr.currScanType = theOtherScan;

        
        zfTimerSchedule(dev, ZM_EVENT_SCAN, 100);
    }
    else
    {
        wd->sta.scanMgr.currScanType = ZM_SCAN_MGR_SCAN_NONE;
    }

stop_done:
    wd->sta.scanMgr.scanReqs[scanType - 1] = 0;

    zmw_leave_critical_section(dev);

    
    if ((zfStaIsConnected(dev)) && (!zfPowerSavingMgrIsSleeping(dev)))
    {
        zfSendNullData(dev, 0);
    }

    if ( scanNotifyRequired )
    {
        zm_debug_msg0("Scan notify after reset");
        if (wd->zfcbScanNotify != NULL)
        {
            wd->zfcbScanNotify(dev, NULL);
        }
    }

    return;

done:
    zmw_leave_critical_section(dev);
    return;
}

void zfScanMgrScanAck(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);

    wd->sta.scanMgr.scanStartDelay = 3;
    

    zmw_leave_critical_section(dev);
    return;
}

extern void zfStaReconnect(zdev_t* dev);

static void zfScanSendProbeRequest(zdev_t* dev)
{
    u8_t k;
    u16_t  dst[3] = { 0xffff, 0xffff, 0xffff };

    zmw_get_wlan_dev(dev);

    
    if (zfStaIsConnected(dev))
    {
        wd->sta.rxBeaconCount++;
    }

    if ( wd->sta.bPassiveScan )
    {
        return;
    }
    
    if (wd->sta.DFSEnable)
    {
        if (zfHpIsDfsChannel(dev, wd->sta.scanFrequency))
        {
            return;
        }
    }

    zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_PROBEREQ, dst, 0, 0, 0);

    if ( wd->sta.disableProbingWithSsid )
    {
        return;
    }

    for (k=1; k<=ZM_MAX_PROBE_HIDDEN_SSID_SIZE; k++)
    {
        if ( wd->ws.probingSsidList[k-1].ssidLen != 0 )
        {
            zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_PROBEREQ, dst, k, 0, 0);
        }
    }
}

static void zfScanMgrEventSetFreqCompleteCb(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();



    zmw_enter_critical_section(dev);
    zfTimerSchedule(dev, ZM_EVENT_IN_SCAN, ZM_TICK_IN_SCAN);
    if (wd->sta.bPassiveScan)
    {
        zfTimerSchedule(dev, ZM_EVENT_TIMEOUT_SCAN, wd->sta.passiveScanTickPerChannel);
    }
    else
    {
        zfTimerSchedule(dev, ZM_EVENT_TIMEOUT_SCAN, wd->sta.activescanTickPerChannel);
    }
    zmw_leave_critical_section(dev);

    zfScanSendProbeRequest(dev);
}


static void zfScanMgrEventScanCompleteCb(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    if ((zfStaIsConnected(dev)) && (!zfPowerSavingMgrIsSleeping(dev)))
    {
        zfSendNullData(dev, 0);
    }
    return;
}


void zfScanMgrScanEventRetry(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    if ( !wd->sta.bChannelScan )
    {
        return;
    }

    if ( !wd->sta.bPassiveScan )
    {
        zfScanSendProbeRequest(dev);
        #if 0
        zmw_enter_critical_section(dev);
        zfTimerSchedule(dev, ZM_EVENT_IN_SCAN, ZM_TICK_IN_SCAN);
        zmw_leave_critical_section(dev);
        #endif
    }
}

u8_t zfScanMgrScanEventTimeout(zdev_t* dev)
{
    u16_t   nextScanFrequency = 0;
    u8_t    temp;

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);
    if ( wd->sta.scanFrequency == 0 )
    {
        zmw_leave_critical_section(dev);
        return -1;
    }

    nextScanFrequency = zfChGetNextChannel(dev, wd->sta.scanFrequency,
                                           &wd->sta.bPassiveScan);

    if ( (nextScanFrequency == 0xffff)
         || (wd->sta.scanFrequency == zfChGetLastChannel(dev, &temp)) )
    {
        u8_t currScanType;
        u8_t isExternalScan = 0;
        u8_t isInternalScan = 0;

        
        wd->sta.scanFrequency = 0;

        zm_debug_msg1("scan 1 type: ", wd->sta.scanMgr.currScanType);
        zm_debug_msg1("scan channel count = ", wd->regulationTable.allowChannelCnt);

        
        zfTimerCancel(dev, ZM_EVENT_TIMEOUT_SCAN);

        if ( wd->sta.bChannelScan == FALSE )
        {
            zm_debug_msg0("WOW!! scan is cancelled\n");
            zmw_leave_critical_section(dev);
            goto report_scan_result;
        }


        currScanType = wd->sta.scanMgr.currScanType;
        switch(currScanType)
        {
            case ZM_SCAN_MGR_SCAN_EXTERNAL:
                isExternalScan = 1;

                if ( wd->sta.scanMgr.scanReqs[ZM_SCAN_MGR_SCAN_INTERNAL - 1] )
                {
                    wd->sta.scanMgr.scanReqs[ZM_SCAN_MGR_SCAN_INTERNAL - 1] = 0;
                    isInternalScan = 1;
                }

                break;

            case ZM_SCAN_MGR_SCAN_INTERNAL:
                isInternalScan = 1;

                if ( wd->sta.scanMgr.scanReqs[ZM_SCAN_MGR_SCAN_EXTERNAL - 1] )
                {
                    
                    
                    zm_assert(0);
                }

                break;

            default:
                zm_assert(0);
                break;
        }

        wd->sta.scanMgr.scanReqs[currScanType - 1] = 0;
        wd->sta.scanMgr.scanStartDelay = 100;
        wd->sta.scanMgr.currScanType = ZM_SCAN_MGR_SCAN_NONE;
        zmw_leave_critical_section(dev);

        
        zfCoreSetFrequencyEx(dev, wd->frequency, wd->BandWidth40,
                wd->ExtOffset, zfScanMgrEventScanCompleteCb);

        wd->sta.bChannelScan = FALSE;

        #if 1
        if (zfStaIsConnected(dev))
        { 
            zfHpFinishSiteSurvey(dev, 1);
            zmw_enter_critical_section(dev);
            wd->sta.ibssSiteSurveyStatus = 2;
            wd->tickIbssReceiveBeacon = 0;
            wd->sta.ibssReceiveBeaconCount = 0;
            zmw_leave_critical_section(dev);

            
            
            if( wd->sta.rifsState == ZM_RIFS_STATE_DETECTED )
            {
                zfHpEnableRifs(dev, ((wd->sta.currentFrequency<3000)?1:0), wd->sta.EnableHT, wd->sta.HT2040);
            }
        }
        else
        {
            zfHpFinishSiteSurvey(dev, 0);
            zmw_enter_critical_section(dev);
            wd->sta.ibssSiteSurveyStatus = 0;
            zmw_leave_critical_section(dev);
        }
        #endif

report_scan_result:
        
        
        
        
        

        if ( isExternalScan )
        {
            if (wd->zfcbScanNotify != NULL)
            {
                wd->zfcbScanNotify(dev, NULL);
            }
        }

        if ( isInternalScan )
        {
            
            zfStaReconnect(dev);
        }

        return 0;
    }
    else
    {
        wd->sta.scanFrequency = nextScanFrequency;

        
        zfTimerCancel(dev, ZM_EVENT_IN_SCAN);
        zmw_leave_critical_section(dev);

        zm_debug_msg0("scan 2");
        zfCoreSetFrequencyV2(dev, wd->sta.scanFrequency, zfScanMgrEventSetFreqCompleteCb);

        return 1;
    }
}

void zfScanMgrScanEventStart(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    if ( wd->sta.bChannelScan )
    {
        return;
    }

    zfPowerSavingMgrWakeup(dev);

    zmw_enter_critical_section(dev);

    if ( wd->sta.scanMgr.currScanType == ZM_SCAN_MGR_SCAN_NONE )
    {
        goto no_scan;
    }

    
    zfBssInfoRefresh(dev, 0);
    wd->sta.bChannelScan = TRUE;
    wd->sta.bScheduleScan = FALSE;
    zfTimerCancel(dev, ZM_EVENT_IN_SCAN);
    zfTimerCancel(dev, ZM_EVENT_TIMEOUT_SCAN);

    
    wd->sta.scanFrequency = zfChGetFirstChannel(dev, &wd->sta.bPassiveScan);
    zmw_leave_critical_section(dev);

    
    
    
    
    



    #if 1
    if (zfStaIsConnected(dev))
    {
        zfHpBeginSiteSurvey(dev, 1);
        zmw_enter_critical_section(dev);
        wd->sta.ibssSiteSurveyStatus = 1;
        zmw_leave_critical_section(dev);
    }
    else
    {
        zfHpBeginSiteSurvey(dev, 0);
        zmw_enter_critical_section(dev);
        wd->sta.ibssSiteSurveyStatus = 0;
        zmw_leave_critical_section(dev);
    }
    #endif

    zm_debug_msg0("scan 0");
    zfCoreSetFrequencyV2(dev, wd->sta.scanFrequency, zfScanMgrEventSetFreqCompleteCb);

    return;

no_scan:
    zmw_leave_critical_section(dev);
    return;
}
