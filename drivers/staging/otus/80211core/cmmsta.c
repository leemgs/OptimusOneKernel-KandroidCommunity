

#include "cprecomp.h"
#include "ratectrl.h"
#include "../hal/hpreg.h"


u8_t   zgWpaRadiusOui[] = { 0x00, 0x50, 0xf2, 0x01 };
u8_t   zgWpaAesOui[] = { 0x00, 0x50, 0xf2, 0x04 };
u8_t   zgWpa2RadiusOui[] = { 0x00, 0x0f, 0xac, 0x01 };
u8_t   zgWpa2AesOui[] = { 0x00, 0x0f, 0xac, 0x04 };

const u16_t zcCwTlb[16] = {   0,    1,    3,    7,   15,   31,   63,  127,
                            255,  511, 1023, 2047, 4095, 4095, 4095, 4095};

void zfStaStartConnectCb(zdev_t* dev);


















void zfStaPutApIntoBlockingList(zdev_t* dev, u8_t* bssid, u8_t weight)
{
    u16_t i, j;
    zmw_get_wlan_dev(dev);
    zmw_declare_for_critical_section();

    if (weight > 0)
    {
        zmw_enter_critical_section(dev);
        
        for (i=0; i<ZM_MAX_BLOCKING_AP_LIST_SIZE; i++)
        {
            for (j=0; j<6; j++)
            {
                if(wd->sta.blockingApList[i].addr[j]!= bssid[j])
                {
                    break;
                }
            }

            if(j==6)
            {
                break;
            }
        }
        
        if (i == ZM_MAX_BLOCKING_AP_LIST_SIZE)
        {
            for (i=0; i<ZM_MAX_BLOCKING_AP_LIST_SIZE; i++)
            {
                if (wd->sta.blockingApList[i].weight == 0)
                {
                    break;
                }
            }
        }

        
        if (i == ZM_MAX_BLOCKING_AP_LIST_SIZE)
        {
            i = bssid[5] & (ZM_MAX_BLOCKING_AP_LIST_SIZE-1);
        }

        
        for (j=0; j<6; j++)
        {
            wd->sta.blockingApList[i].addr[j] = bssid[j];
        }

        wd->sta.blockingApList[i].weight = weight;
        zmw_leave_critical_section(dev);
    }

    return;
}



















u16_t zfStaIsApInBlockingList(zdev_t* dev, u8_t* bssid)
{
    u16_t i, j;
    zmw_get_wlan_dev(dev);
    

    
    for (i=0; i<ZM_MAX_BLOCKING_AP_LIST_SIZE; i++)
    {
        if (wd->sta.blockingApList[i].weight != 0)
        {
            for (j=0; j<6; j++)
            {
                if (wd->sta.blockingApList[i].addr[j] != bssid[j])
                {
                    break;
                }
            }
            if (j == 6)
            {
                
                return TRUE;
            }
        }
    }
    
    return FALSE;
}


















void zfStaRefreshBlockList(zdev_t* dev, u16_t flushFlag)
{
    u16_t i;
    zmw_get_wlan_dev(dev);
    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);
    for (i=0; i<ZM_MAX_BLOCKING_AP_LIST_SIZE; i++)
    {
        if (wd->sta.blockingApList[i].weight != 0)
        {
            if (flushFlag != 0)
            {
                wd->sta.blockingApList[i].weight = 0;
            }
            else
            {
                wd->sta.blockingApList[i].weight--;
            }
        }
    }
    zmw_leave_critical_section(dev);
    return;
}



















void zfStaConnectFail(zdev_t* dev, u16_t reason, u16_t* bssid, u8_t weight)
{
    zmw_get_wlan_dev(dev);

    
    zfChangeAdapterState(dev, ZM_STA_STATE_DISCONNECT);

    
    

    
    if (wd->zfcbConnectNotify != NULL)
    {
        wd->zfcbConnectNotify(dev, reason, bssid);
    }

    
    zfStaPutApIntoBlockingList(dev, (u8_t *)bssid, weight);

    
    if ( wd->sta.bAutoReconnect )
    {
        zm_debug_msg0("Start internal scan...");
        zfScanMgrScanStop(dev, ZM_SCAN_MGR_SCAN_INTERNAL);
        zfScanMgrScanStart(dev, ZM_SCAN_MGR_SCAN_INTERNAL);
    }
}

u8_t zfiWlanIBSSGetPeerStationsCount(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    return wd->sta.oppositeCount;
}

u8_t zfiWlanIBSSIteratePeerStations(zdev_t* dev, u8_t numToIterate, zfpIBSSIteratePeerStationCb callback, void *ctx)
{
    u8_t oppositeCount;
    u8_t i;
    u8_t index = 0;

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);

    oppositeCount = wd->sta.oppositeCount;
    if ( oppositeCount > numToIterate )
    {
        oppositeCount = numToIterate;
    }

    for(i=0; i < ZM_MAX_OPPOSITE_COUNT; i++)
    {
        if ( oppositeCount == 0 )
        {
            break;
        }

        if ( wd->sta.oppositeInfo[i].valid == 0 )
        {
            continue;
        }

        callback(dev, &wd->sta.oppositeInfo[i], ctx, index++);
        oppositeCount--;

    }

    zmw_leave_critical_section(dev);

    return index;
}


s8_t zfStaFindFreeOpposite(zdev_t* dev, u16_t *sa, int *pFoundIdx)
{
    int oppositeCount;
    int i;

    zmw_get_wlan_dev(dev);

    oppositeCount = wd->sta.oppositeCount;

    for(i=0; i < ZM_MAX_OPPOSITE_COUNT; i++)
    {
        if ( oppositeCount == 0 )
        {
            break;
        }

        if ( wd->sta.oppositeInfo[i].valid == 0 )
        {
            continue;
        }

        oppositeCount--;
        if ( zfMemoryIsEqual((u8_t*) sa, wd->sta.oppositeInfo[i].macAddr, 6) )
        {
            
            wd->sta.oppositeInfo[i].aliveCounter = ZM_IBSS_PEER_ALIVE_COUNTER;

            
            return 1;
        }
    }

    
    if ( wd->sta.oppositeCount == ZM_MAX_OPPOSITE_COUNT )
    {
        return -1;
    }

    
    for(i=0; i < ZM_MAX_OPPOSITE_COUNT; i++)
    {
        if ( wd->sta.oppositeInfo[i].valid == 0 )
        {
            break;
        }
    }

    *pFoundIdx = i;
    return 0;
}

s8_t zfStaFindOppositeByMACAddr(zdev_t* dev, u16_t *sa, u8_t *pFoundIdx)
{
    u32_t oppositeCount;
    u32_t i;

    zmw_get_wlan_dev(dev);

    oppositeCount = wd->sta.oppositeCount;

    for(i=0; i < ZM_MAX_OPPOSITE_COUNT; i++)
    {
        if ( oppositeCount == 0 )
        {
            break;
        }

        if ( wd->sta.oppositeInfo[i].valid == 0 )
        {
            continue;
        }

        oppositeCount--;
        if ( zfMemoryIsEqual((u8_t*) sa, wd->sta.oppositeInfo[i].macAddr, 6) )
        {
            *pFoundIdx = (u8_t)i;

            return 0;
        }
    }

    *pFoundIdx = 0;
    return 1;
}

static void zfStaInitCommonOppositeInfo(zdev_t* dev, int i)
{
    zmw_get_wlan_dev(dev);

    
    wd->sta.oppositeInfo[i].valid = 1;
    wd->sta.oppositeInfo[i].aliveCounter = ZM_IBSS_PEER_ALIVE_COUNTER;
    wd->sta.oppositeCount++;

#ifdef ZM_ENABLE_IBSS_WPA2PSK
    
    wd->sta.oppositeInfo[i].camIdx = 0xff;  
    wd->sta.oppositeInfo[i].pkInstalled = 0;
    wd->sta.oppositeInfo[i].wpaState = ZM_STA_WPA_STATE_INIT ;  
#endif
}

int zfStaSetOppositeInfoFromBSSInfo(zdev_t* dev, struct zsBssInfo* pBssInfo)
{
    int i;
    u8_t*  dst;
    u16_t  sa[3];
    int res;
    u32_t oneTxStreamCap;

    zmw_get_wlan_dev(dev);

    zfMemoryCopy((u8_t*) sa, pBssInfo->macaddr, 6);

    res = zfStaFindFreeOpposite(dev, sa, &i);
    if ( res != 0 )
    {
        goto zlReturn;
    }

    dst = wd->sta.oppositeInfo[i].macAddr;
    zfMemoryCopy(dst, (u8_t *)sa, 6);

    oneTxStreamCap = (zfHpCapability(dev) & ZM_HP_CAP_11N_ONE_TX_STREAM);

    if (pBssInfo->extSupportedRates[1] != 0)
    {
        
        if (pBssInfo->frequency < 3000)
        {
            
            if (pBssInfo->EnableHT == 1)
                zfRateCtrlInitCell(dev, &wd->sta.oppositeInfo[i].rcCell, (oneTxStreamCap!=0)?3:2, 1, pBssInfo->SG40);
            else
                zfRateCtrlInitCell(dev, &wd->sta.oppositeInfo[i].rcCell, 1, 1, pBssInfo->SG40);
        }
        else
        {
            
            if (pBssInfo->EnableHT == 1)
                zfRateCtrlInitCell(dev, &wd->sta.oppositeInfo[i].rcCell, (oneTxStreamCap!=0)?3:2, 0, pBssInfo->SG40);
            else
                zfRateCtrlInitCell(dev, &wd->sta.oppositeInfo[i].rcCell, 1, 0, pBssInfo->SG40);
        }
    }
    else
    {
        
        if (pBssInfo->frequency < 3000)
        {
            
            if (pBssInfo->EnableHT == 1)
                zfRateCtrlInitCell(dev, &wd->sta.oppositeInfo[i].rcCell, (oneTxStreamCap!=0)?3:2, 1, pBssInfo->SG40);
            else
                zfRateCtrlInitCell(dev, &wd->sta.oppositeInfo[i].rcCell, 0, 1, pBssInfo->SG40);
        }
        else
        {
            
            if (pBssInfo->EnableHT == 1)
                zfRateCtrlInitCell(dev, &wd->sta.oppositeInfo[i].rcCell, (oneTxStreamCap!=0)?3:2, 0, pBssInfo->SG40);
            else
                zfRateCtrlInitCell(dev, &wd->sta.oppositeInfo[i].rcCell, 1, 0, pBssInfo->SG40);
        }
    }


    zfStaInitCommonOppositeInfo(dev, i);
zlReturn:
    return 0;
}

int zfStaSetOppositeInfoFromRxBuf(zdev_t* dev, zbuf_t* buf)
{
    int   i;
    u8_t*  dst;
    u16_t  sa[3];
    int res = 0;
    u16_t  offset;
    u8_t   bSupportExtRate;
    u32_t rtsctsRate = 0xffffffff; 
    u32_t oneTxStreamCap;

    zmw_get_wlan_dev(dev);
    zmw_declare_for_critical_section();

    sa[0] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A2_OFFSET);
    sa[1] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A2_OFFSET+2);
    sa[2] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A2_OFFSET+4);

    zmw_enter_critical_section(dev);

    res = zfStaFindFreeOpposite(dev, sa, &i);
    if ( res != 0 )
    {
        goto zlReturn;
    }

    dst = wd->sta.oppositeInfo[i].macAddr;
    zfCopyFromRxBuffer(dev, buf, dst, ZM_WLAN_HEADER_A2_OFFSET, 6);

    if ( (wd->sta.currentFrequency < 3000) && !(wd->supportMode & (ZM_WIRELESS_MODE_24_54|ZM_WIRELESS_MODE_24_N)) )
    {
        bSupportExtRate = 0;
    } else {
        bSupportExtRate = 1;
    }

    if ( (bSupportExtRate == 1)
         && (wd->sta.currentFrequency < 3000)
         && (wd->wlanMode == ZM_MODE_IBSS)
         && (wd->wfc.bIbssGMode == 0) )
    {
        bSupportExtRate = 0;
    }

    wd->sta.connection_11b = 0;
    oneTxStreamCap = (zfHpCapability(dev) & ZM_HP_CAP_11N_ONE_TX_STREAM);

    if ( ((offset = zfFindElement(dev, buf, ZM_WLAN_EID_EXTENDED_RATE)) != 0xffff)
         && (bSupportExtRate == 1) )
    {
        
        if (wd->sta.currentFrequency < 3000)
        {
            
            if (wd->sta.EnableHT == 1)
            {
                
                zfRateCtrlInitCell(dev, &wd->sta.oppositeInfo[i].rcCell, (oneTxStreamCap!=0)?3:2, 1, wd->sta.SG40);
            }
            else
            {
                
                zfRateCtrlInitCell(dev, &wd->sta.oppositeInfo[i].rcCell, 1, 1, wd->sta.SG40);
            }
            rtsctsRate = 0x00001bb; 
        }
        else
        {
            
            if (wd->sta.EnableHT == 1)
            {
                
                zfRateCtrlInitCell(dev, &wd->sta.oppositeInfo[i].rcCell, (oneTxStreamCap!=0)?3:2, 0, wd->sta.SG40);
            }
            else
            {
                
                zfRateCtrlInitCell(dev, &wd->sta.oppositeInfo[i].rcCell, 1, 0, wd->sta.SG40);
            }
            rtsctsRate = 0x10b01bb; 
        }
    }
    else
    {
        
        if (wd->sta.currentFrequency < 3000)
        {
            
            if (wd->sta.EnableHT == 1)
            {
                
                zfRateCtrlInitCell(dev, &wd->sta.oppositeInfo[i].rcCell, (oneTxStreamCap!=0)?3:2, 1, wd->sta.SG40);
                rtsctsRate = 0x00001bb; 
            }
            else
            {
                
                zfRateCtrlInitCell(dev, &wd->sta.oppositeInfo[i].rcCell, 0, 1, wd->sta.SG40);
                rtsctsRate = 0x0; 
                wd->sta.connection_11b = 1;
            }
        }
        else
        {
            
            if (wd->sta.EnableHT == 1)
            {
                
                zfRateCtrlInitCell(dev, &wd->sta.oppositeInfo[i].rcCell, (oneTxStreamCap!=0)?3:2, 0, wd->sta.SG40);
            }
            else
            {
                
                zfRateCtrlInitCell(dev, &wd->sta.oppositeInfo[i].rcCell, 1, 0, wd->sta.SG40);
            }
            rtsctsRate = 0x10b01bb; 
        }
    }

    zfStaInitCommonOppositeInfo(dev, i);

zlReturn:
    zmw_leave_critical_section(dev);

    if (rtsctsRate != 0xffffffff)
    {
        zfHpSetRTSCTSRate(dev, rtsctsRate);
    }
    return res;
}

void zfStaProtErpMonitor(zdev_t* dev, zbuf_t* buf)
{
    u16_t   offset;
    u8_t    erp;
    u8_t    bssid[6];

    zmw_get_wlan_dev(dev);

    if ( (wd->wlanMode == ZM_MODE_INFRASTRUCTURE)&&(zfStaIsConnected(dev)) )
    {
        ZM_MAC_WORD_TO_BYTE(wd->sta.bssid, bssid);

        if (zfRxBufferEqualToStr(dev, buf, bssid, ZM_WLAN_HEADER_A2_OFFSET, 6))
        {
            if ( (offset=zfFindElement(dev, buf, ZM_WLAN_EID_ERP)) != 0xffff )
            {
                erp = zmw_rx_buf_readb(dev, buf, offset+2);

                if ( erp & ZM_BIT_1 )
                {
                    
                    if (wd->sta.bProtectionMode == FALSE)
                    {
                        wd->sta.bProtectionMode = TRUE;
                        zfHpSetSlotTime(dev, 0);
                    }
                }
                else
                {
                    
                    if (wd->sta.bProtectionMode == TRUE)
                    {
                        wd->sta.bProtectionMode = FALSE;
                        zfHpSetSlotTime(dev, 1);
                    }
                }
            }
        }
		
		
			if ((offset = zfFindElement(dev, buf, ZM_WLAN_EID_HT_CAPABILITY)) != 0xffff)
			{}
			else if ((offset = zfFindElement(dev, buf, ZM_WLAN_PREN2_EID_HTCAPABILITY)) != 0xffff)
			{}
			else
			{wd->sta.NonNAPcount++;}
    }
}

void zfStaUpdateWmeParameter(zdev_t* dev, zbuf_t* buf)
{
    u16_t   tmp;
    u16_t   aifs[5];
    u16_t   cwmin[5];
    u16_t   cwmax[5];
    u16_t   txop[5];
    u8_t    acm;
    u8_t    ac;
    u16_t   len;
    u16_t   i;
   	u16_t   offset;
    u8_t    rxWmeParameterSetCount;

    zmw_get_wlan_dev(dev);

    
    
    if (wd->sta.wmeConnected != 0)
    {
        
        if ((offset = zfFindWifiElement(dev, buf, 2, 1)) != 0xffff)
        {
            if ((len = zmw_rx_buf_readb(dev, buf, offset+1)) >= 7)
            {
                rxWmeParameterSetCount=zmw_rx_buf_readb(dev, buf, offset+8);
                if (rxWmeParameterSetCount != wd->sta.wmeParameterSetCount)
                {
                    zm_msg0_mm(ZM_LV_0, "wmeParameterSetCount changed!");
                    wd->sta.wmeParameterSetCount = rxWmeParameterSetCount;
                    
                    acm = 0xf;
                    for (i=0; i<4; i++)
                    {
                        if (len >= (8+(i*4)+4))
                        {
                            tmp=zmw_rx_buf_readb(dev, buf, offset+10+i*4);
                            ac = (tmp >> 5) & 0x3;
                            if ((tmp & 0x10) == 0)
                            {
                                acm &= (~(1<<ac));
                            }
                            aifs[ac] = ((tmp & 0xf) * 9) + 10;
                            tmp=zmw_rx_buf_readb(dev, buf, offset+11+i*4);
                            
                            cwmin[ac] = zcCwTlb[(tmp & 0xf)];
                            cwmax[ac] = zcCwTlb[(tmp >> 4)];
                            txop[ac]=zmw_rx_buf_readh(dev, buf,
                                    offset+12+i*4);
                        }
                    }

                    if ((acm & 0x4) != 0)
                    {
                        cwmin[2] = cwmin[0];
                        cwmax[2] = cwmax[0];
                        aifs[2] = aifs[0];
                        txop[2] = txop[0];
                    }
                    if ((acm & 0x8) != 0)
                    {
                        cwmin[3] = cwmin[2];
                        cwmax[3] = cwmax[2];
                        aifs[3] = aifs[2];
                        txop[3] = txop[2];
                    }
                    cwmin[4] = 3;
                    cwmax[4] = 7;
                    aifs[4] = 28;

                    if ((cwmin[2]+aifs[2]) > ((cwmin[0]+aifs[0])+1))
                    {
                        wd->sta.ac0PriorityHigherThanAc2 = 1;
                    }
                    else
                    {
                        wd->sta.ac0PriorityHigherThanAc2 = 0;
                    }
                    zfHpUpdateQosParameter(dev, cwmin, cwmax, aifs, txop);
                }
            }
        }
    } 
}

void zfStaUpdateDot11HDFS(zdev_t* dev, zbuf_t* buf)
{
    
    u16_t   offset;

    zmw_get_wlan_dev(dev);

    

    
    if ( (offset = zfFindElement(dev, buf, ZM_WLAN_EID_CHANNEL_SWITCH_ANNOUNCE)) == 0xffff )
    {
        
        return;
    }
    else if ( zmw_rx_buf_readb(dev, buf, offset+1) == 0x3 )
    {
        zm_debug_msg0("EID(Channel Switch Announcement) found");

        
        

        
        
        if (zmw_rx_buf_readb(dev, buf, offset+2) == 0x1 )
        {
        	
        	
        	if (wd->sta.DFSDisableTx != TRUE)
        	{
                
                
                wd->sta.DFSDisableTx = TRUE;
                
                zfHpStartRecv(dev);
            }
        	
        	
        	
        	
        }

        if (zmw_rx_buf_readb(dev, buf, offset+4) <= 0x2 )
        {
        	
        	
        	
        	
        	
        	

            zfHpDeleteAllowChannel(dev, wd->sta.currentFrequency);
        	wd->frequency = zfChNumToFreq(dev, zmw_rx_buf_readb(dev, buf, offset+3), 0);
        	
        	zm_debug_msg1("CWY - jump to frequency = ", wd->frequency);
        	zfCoreSetFrequency(dev, wd->frequency);
        	wd->sta.DFSDisableTx = FALSE;
            
            if (zfStaIsConnected(dev))
            {
                wd->sta.rxBeaconCount = 1 << 6; 
            }
        	

        	
        	
        	
        	
        	
        	
        	
        	
        	
        	
        }
    }

}

void zfStaUpdateDot11HTPC(zdev_t* dev, zbuf_t* buf)
{
}


void zfStaIbssPSCheckState(zdev_t* dev, zbuf_t* buf)
{
    u8_t   i, frameCtrl;

    zmw_get_wlan_dev(dev);

    if ( !zfStaIsConnected(dev) )
    {
        return;
    }

    if ( wd->wlanMode != ZM_MODE_IBSS )
    {
        return ;
    }

    
    if ( !zfRxBufferEqualToStr(dev, buf, (u8_t*) wd->sta.bssid,
                               ZM_WLAN_HEADER_A3_OFFSET, 6) )
    {
        return;
    }

    frameCtrl = zmw_rx_buf_readb(dev, buf, 1);

    
    if ( frameCtrl & ZM_BIT_4 )
    {
        for(i=1; i<ZM_MAX_PS_STA; i++)
        {
            if ( !wd->sta.staPSList.entity[i].bUsed )
            {
                continue;
            }

            
            if ( zfRxBufferEqualToStr(dev, buf,
                                      wd->sta.staPSList.entity[i].macAddr,
                                      ZM_WLAN_HEADER_A2_OFFSET, 6) )
            {
                return;
            }
        }

        for(i=1; i<ZM_MAX_PS_STA; i++)
        {
            if ( !wd->sta.staPSList.entity[i].bUsed )
            {
                wd->sta.staPSList.entity[i].bUsed = TRUE;
                wd->sta.staPSList.entity[i].bDataQueued = FALSE;
                break;
            }
        }

        if ( i == ZM_MAX_PS_STA )
        {
            
            return;
        }

        zfCopyFromRxBuffer(dev, buf, wd->sta.staPSList.entity[i].macAddr,
                           ZM_WLAN_HEADER_A2_OFFSET, 6);

        if ( wd->sta.staPSList.count == 0 )
        {
            
            
        }

        wd->sta.staPSList.count++;
    }
    else if ( wd->sta.staPSList.count )
    {
        for(i=1; i<ZM_MAX_PS_STA; i++)
        {
            if ( wd->sta.staPSList.entity[i].bUsed )
            {
                if ( zfRxBufferEqualToStr(dev, buf,
                                          wd->sta.staPSList.entity[i].macAddr,
                                          ZM_WLAN_HEADER_A2_OFFSET, 6) )
                {
                    wd->sta.staPSList.entity[i].bUsed = FALSE;
                    wd->sta.staPSList.count--;

                    if ( wd->sta.staPSList.entity[i].bDataQueued )
                    {
                        
                    }
                }
            }
        }

        if ( wd->sta.staPSList.count == 0 )
        {
            
            
        }

    }
}


u8_t zfStaIbssPSQueueData(zdev_t* dev, zbuf_t* buf)
{
    u8_t   i;
    u16_t  da[3];

    zmw_get_wlan_dev(dev);

    if ( !zfStaIsConnected(dev) )
    {
        return 0;
    }

    if ( wd->wlanMode != ZM_MODE_IBSS )
    {
        return 0;
    }

    if ( wd->sta.staPSList.count == 0 && wd->sta.powerSaveMode <= ZM_STA_PS_NONE )
    {
        return 0;
    }

    
#ifdef ZM_ENABLE_NATIVE_WIFI
    da[0] = zmw_tx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET);
    da[1] = zmw_tx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET + 2);
    da[2] = zmw_tx_buf_readh(dev, buf, ZM_WLAN_HEADER_A1_OFFSET + 4);
#else
    da[0] = zmw_tx_buf_readh(dev, buf, 0);
    da[1] = zmw_tx_buf_readh(dev, buf, 2);
    da[2] = zmw_tx_buf_readh(dev, buf, 4);
#endif

    if ( ZM_IS_MULTICAST_OR_BROADCAST(da) )
    {
        wd->sta.staPSList.entity[0].bDataQueued = TRUE;
        wd->sta.ibssPSDataQueue[wd->sta.ibssPSDataCount++] = buf;
        return 1;
    }

    

    for(i=1; i<ZM_MAX_PS_STA; i++)
    {
        if ( zfMemoryIsEqual(wd->sta.staPSList.entity[i].macAddr,
                             (u8_t*) da, 6) )
        {
            wd->sta.staPSList.entity[i].bDataQueued = TRUE;
            wd->sta.ibssPSDataQueue[wd->sta.ibssPSDataCount++] = buf;

            return 1;
        }
    }

#if 0
    if ( wd->sta.powerSaveMode > ZM_STA_PS_NONE )
    {
        wd->sta.staPSDataQueue[wd->sta.staPSDataCount++] = buf;

        return 1;
    }
#endif

    return 0;
}


void zfStaIbssPSSend(zdev_t* dev)
{
    u8_t   i;
    u16_t  bcastAddr[3] = {0xffff, 0xffff, 0xffff};

    zmw_get_wlan_dev(dev);

    if ( !zfStaIsConnected(dev) )
    {
        return ;
    }

    if ( wd->wlanMode != ZM_MODE_IBSS )
    {
        return ;
    }

    for(i=0; i<ZM_MAX_PS_STA; i++)
    {
        if ( wd->sta.staPSList.entity[i].bDataQueued )
        {
            if ( i == 0 )
            {
                zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_ATIM,
                              bcastAddr,
                              0, 0, 0);
            }
            else if ( wd->sta.staPSList.entity[i].bUsed )
            {
                
                zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_ATIM,
                              (u16_t*) wd->sta.staPSList.entity[i].macAddr,
                              0, 0, 0);
            }

            wd->sta.staPSList.entity[i].bDataQueued = FALSE;
        }
    }

    for(i=0; i<wd->sta.ibssPSDataCount; i++)
    {
        zfTxSendEth(dev, wd->sta.ibssPSDataQueue[i], 0,
                    ZM_EXTERNAL_ALLOC_BUF, 0);
    }

    wd->sta.ibssPrevPSDataCount = wd->sta.ibssPSDataCount;
    wd->sta.ibssPSDataCount = 0;
}


void zfStaReconnect(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);
    zmw_declare_for_critical_section();

    if ( wd->wlanMode != ZM_MODE_INFRASTRUCTURE &&
         wd->wlanMode != ZM_MODE_IBSS )
    {
        return;
    }

    if ( (zfStaIsConnected(dev))||(zfStaIsConnecting(dev)) )
    {
        return;
    }

    if ( wd->sta.bChannelScan )
    {
        return;
    }

    
    if ( (wd->wlanMode == ZM_MODE_INFRASTRUCTURE) && (wd->ws.ssidLen == 0))
    {
        zm_debug_msg0("zfStaReconnect: NOT Support!! Set SSID to any BSS");
        
        zmw_enter_critical_section(dev);
        wd->sta.ssid[0] = 0;
        wd->sta.ssidLen = 0;
        zmw_leave_critical_section(dev);
    }

    
    zfFlushVtxq(dev);
    zfWlanEnable(dev);
    zfScanMgrScanAck(dev);
}

void zfStaTimer100ms(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    if ( (wd->tick % 10) == 0 )
    {
        zfPushVtxq(dev);

    }
}


void zfStaCheckRxBeacon(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    if (( wd->wlanMode == ZM_MODE_INFRASTRUCTURE ) && (zfStaIsConnected(dev)))
    {
        if (wd->beaconInterval == 0)
        {
            wd->beaconInterval = 100;
        }
        if ( (wd->tick % ((wd->beaconInterval * 10) / ZM_MS_PER_TICK)) == 0 )
        {
            
            if (wd->sta.rxBeaconCount == 0)
            {
                if (wd->sta.beaconMissState == 1)
                {
            	
            	zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_DEAUTH, wd->sta.bssid, 3, 0, 0);
                
                zfStaConnectFail(dev, ZM_STATUS_MEDIA_DISCONNECT_BEACON_MISS,
                        wd->sta.bssid, 0);
                }
                else
                {
                    wd->sta.beaconMissState = 1;
                    
                    zfCoreSetFrequencyExV2(dev, wd->frequency, wd->BandWidth40,
                            wd->ExtOffset, NULL, 1);
                }
            }
            else
            {
                wd->sta.beaconMissState = 0;
            }
            wd->sta.rxBeaconCount = 0;
        }
    }
}



void zfStaCheckConnectTimeout(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);
    zmw_declare_for_critical_section();

    if ( wd->wlanMode != ZM_MODE_INFRASTRUCTURE )
    {
        return;
    }

    if ( !zfStaIsConnecting(dev) )
    {
        return;
    }

    zmw_enter_critical_section(dev);
    if ( (wd->sta.connectState == ZM_STA_CONN_STATE_AUTH_OPEN)||
         (wd->sta.connectState == ZM_STA_CONN_STATE_AUTH_SHARE_1)||
         (wd->sta.connectState == ZM_STA_CONN_STATE_AUTH_SHARE_2)||
         (wd->sta.connectState == ZM_STA_CONN_STATE_ASSOCIATE) )
    {
        if ( (wd->tick - wd->sta.connectTimer) > ZM_INTERVAL_CONNECT_TIMEOUT )
        {
            if ( wd->sta.connectByReasso )
            {
                wd->sta.failCntOfReasso++;
                if ( wd->sta.failCntOfReasso > 2 )
                {
                    wd->sta.connectByReasso = FALSE;
                }
            }

            wd->sta.connectState = ZM_STA_CONN_STATE_NONE;
            zm_debug_msg1("connect timeout, state = ", wd->sta.connectState);
            
            goto failed;
        }
    }

    zmw_leave_critical_section(dev);
    return;

failed:
    zmw_leave_critical_section(dev);
    if(wd->sta.authMode == ZM_AUTH_MODE_AUTO)
	{ 
            wd->sta.connectTimeoutCount++;
	}
    zfStaConnectFail(dev, ZM_STATUS_MEDIA_DISCONNECT_TIMEOUT, wd->sta.bssid, 2);
    return;
}

void zfMmStaTimeTick(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    
    if (wd->wlanMode != ZM_MODE_AP && !wd->swSniffer)
    {
        if ( wd->tick & 1 )
        {
            zfTimerCheckAndHandle(dev);
        }

        zfStaCheckRxBeacon(dev);
        zfStaTimer100ms(dev);
        zfStaCheckConnectTimeout(dev);
        zfPowerSavingMgrMain(dev);
    }

#ifdef ZM_ENABLE_AGGREGATION
    
    zfAggScanAndClear(dev, wd->tick);
#endif
}

void zfStaSendBeacon(zdev_t* dev)
{
    zbuf_t* buf;
    u16_t offset, seq;

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    

    
    if ((buf = zfwBufAllocate(dev, 1024)) == NULL)
    {
        zm_debug_msg0("Allocate beacon buffer failed");
        return;
    }

    offset = 0;
    
    
    zmw_tx_buf_writeh(dev, buf, offset, 0x0080);
    offset+=2;
    
    zmw_tx_buf_writeh(dev, buf, offset, 0x0000);
    offset+=2;
    
    zmw_tx_buf_writeh(dev, buf, offset, 0xffff);
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, 0xffff);
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, 0xffff);
    offset+=2;
    
    zmw_tx_buf_writeh(dev, buf, offset, wd->macAddr[0]);
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, wd->macAddr[1]);
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, wd->macAddr[2]);
    offset+=2;
    
    zmw_tx_buf_writeh(dev, buf, offset, wd->sta.bssid[0]);
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, wd->sta.bssid[1]);
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, wd->sta.bssid[2]);
    offset+=2;

    
    zmw_enter_critical_section(dev);
    seq = ((wd->mmseq++)<<4);
    zmw_leave_critical_section(dev);
    zmw_tx_buf_writeh(dev, buf, offset, seq);
    offset+=2;

    
    offset+=8;

    
    zmw_tx_buf_writeh(dev, buf, offset, wd->beaconInterval);
    offset+=2;

    
    zmw_tx_buf_writeb(dev, buf, offset++, wd->sta.capability[0]);
    zmw_tx_buf_writeb(dev, buf, offset++, wd->sta.capability[1]);

    
    offset = zfStaAddIeSsid(dev, buf, offset);

    if(wd->frequency <= ZM_CH_G_14)  
    {

    	
    	offset = zfMmAddIeSupportRate(dev, buf, offset,
                                  		ZM_WLAN_EID_SUPPORT_RATE, ZM_RATE_SET_CCK);

    	
    	offset = zfMmAddIeDs(dev, buf, offset);

    	offset = zfStaAddIeIbss(dev, buf, offset);

        if( wd->wfc.bIbssGMode
            && (wd->supportMode & (ZM_WIRELESS_MODE_24_54|ZM_WIRELESS_MODE_24_N)) )    
        {
      	    
       	    wd->erpElement = 0;
       	    offset = zfMmAddIeErp(dev, buf, offset);
       	}

       	
        
        if ( wd->sta.authMode == ZM_AUTH_MODE_WPA2PSK )
        {
            offset = zfwStaAddIeWpaRsn(dev, buf, offset, ZM_WLAN_FRAME_TYPE_AUTH);
        }

        if( wd->wfc.bIbssGMode
            && (wd->supportMode & (ZM_WIRELESS_MODE_24_54|ZM_WIRELESS_MODE_24_N)) )    
        {
            
            
       	    offset = zfMmAddIeSupportRate(dev, buf, offset,
                                   		    ZM_WLAN_EID_EXTENDED_RATE, ZM_RATE_SET_OFDM);
	    }
    }
    else    
    {
        
    	offset = zfMmAddIeSupportRate(dev, buf, offset,
        	                            ZM_WLAN_EID_SUPPORT_RATE, ZM_RATE_SET_OFDM);

        
    	offset = zfMmAddIeDs(dev, buf, offset);

    	offset = zfStaAddIeIbss(dev, buf, offset);

        
        
        if ( wd->sta.authMode == ZM_AUTH_MODE_WPA2PSK )
        {
            offset = zfwStaAddIeWpaRsn(dev, buf, offset, ZM_WLAN_FRAME_TYPE_AUTH);
        }
    }

    if ( wd->wlanMode != ZM_MODE_IBSS )
    {
        
        
        offset = zfMmAddHTCapability(dev, buf, offset);

        
        offset = zfMmAddExtendedHTCapability(dev, buf, offset);
    }

    if ( wd->sta.ibssAdditionalIESize )
        offset = zfStaAddIbssAdditionalIE(dev, buf, offset);

    
    
    zfHpSendBeacon(dev, buf, offset);

    
    
}

void zfStaSignalStatistic(zdev_t* dev, u8_t SignalStrength, u8_t SignalQuality) 
{
    zmw_get_wlan_dev(dev);

    
    wd->SignalStrength = (wd->SignalStrength * 7 + SignalStrength * 3)/10;
    wd->SignalQuality = (wd->SignalQuality * 7 + SignalQuality * 3)/10;

}

struct zsBssInfo* zfStaFindBssInfo(zdev_t* dev, zbuf_t* buf, struct zsWlanProbeRspFrameHeader *pProbeRspHeader)
{
    u8_t    i;
    u8_t    j;
    u8_t    k;
    u8_t    isMatched, length, channel;
    u16_t   offset, frequency;
    struct zsBssInfo* pBssInfo;

    zmw_get_wlan_dev(dev);

    if ((pBssInfo = wd->sta.bssList.head) == NULL)
    {
        return NULL;
    }

    for( i=0; i<wd->sta.bssList.bssCount; i++ )
    {
        

        
        for( j=0; j<6; j++ )
        {
            if ( pBssInfo->bssid[j] != pProbeRspHeader->bssid[j] )
            {
                break;
            }
        }

		
        if (j == 6)
        {
            if (pProbeRspHeader->ssid[1] <= 32)
            {
                
                isMatched = 1;
				if((pProbeRspHeader->ssid[1] != 0) && (pBssInfo->ssid[1] != 0))
				{
                for( k=1; k<pProbeRspHeader->ssid[1] + 1; k++ )
                {
                    if ( pBssInfo->ssid[k] != pProbeRspHeader->ssid[k] )
                    {
                        isMatched = 0;
                        break;
                    }
                }
            }
            }
            else
            {
                isMatched = 0;
            }
        }
        else
        {
            isMatched = 0;
        }

        
        
        if (isMatched) {
            if ((offset = zfFindElement(dev, buf, ZM_WLAN_EID_DS)) != 0xffff) {
                if ((length = zmw_rx_buf_readb(dev, buf, offset+1)) == 1) {
                    channel = zmw_rx_buf_readb(dev, buf, offset+2);
                    if (zfHpIsAllowedChannel(dev, zfChNumToFreq(dev, channel, 0)) == 0) {
                        frequency = 0;
                    } else {
                        frequency = zfChNumToFreq(dev, channel, 0);;
                    }
                } else {
                    frequency = 0;
                }
            } else {
                frequency = wd->sta.currentFrequency;
            }

            if (frequency != 0) {
                if ( ((frequency > 3000) && (pBssInfo->frequency > 3000))
                     || ((frequency < 3000) && (pBssInfo->frequency < 3000)) ) {
                    
                    break;
                }
            }
        }

        pBssInfo = pBssInfo->next;
    }

    if ( i == wd->sta.bssList.bssCount )
    {
        pBssInfo = NULL;
    }

    return pBssInfo;
}

u8_t zfStaInitBssInfo(zdev_t* dev, zbuf_t* buf,
        struct zsWlanProbeRspFrameHeader *pProbeRspHeader,
        struct zsBssInfo* pBssInfo, struct zsAdditionInfo* AddInfo, u8_t type)
{
    u8_t    length, channel, is5G;
    u16_t   i, offset;
    u8_t    apQosInfo;
    u16_t    eachIElength = 0;
    u16_t   accumulateLen = 0;

    zmw_get_wlan_dev(dev);

    if ((type == 1) && ((pBssInfo->flag & ZM_BSS_INFO_VALID_BIT) != 0))
    {
        goto zlUpdateRssi;
    }

    
    if ( (offset = zfFindElement(dev, buf, ZM_WLAN_EID_SSID)) == 0xffff )
    {
        zm_debug_msg0("EID(SSID) not found");
        goto zlError;
    }

    length = zmw_rx_buf_readb(dev, buf, offset+1);

	{
		u8_t Show_Flag = 0;
		zfwGetShowZeroLengthSSID(dev, &Show_Flag);

		if(Show_Flag)
		{
			if (length > ZM_MAX_SSID_LENGTH )
			{
				zm_debug_msg0("EID(SSID) is invalid");
				goto zlError;
			}
		}
		else
		{
    if ( length == 0 || length > ZM_MAX_SSID_LENGTH )
    {
        zm_debug_msg0("EID(SSID) is invalid");
        goto zlError;
    }

		}
	}
    zfCopyFromRxBuffer(dev, buf, pBssInfo->ssid, offset, length+2);

    
    if ( (offset = zfFindElement(dev, buf, ZM_WLAN_EID_DS)) != 0xffff )
    {
        length = zmw_rx_buf_readb(dev, buf, offset+1);
        if ( length != 1 )
        {
            zm_msg0_mm(ZM_LV_0, "Abnormal DS Param Set IE");
            goto zlError;
        }
        channel = zmw_rx_buf_readb(dev, buf, offset+2);

        if (zfHpIsAllowedChannel(dev, zfChNumToFreq(dev, channel, 0)) == 0)
        {
            goto zlError2;
        }

        pBssInfo->frequency = zfChNumToFreq(dev, channel, 0); 
        pBssInfo->channel = channel;


    }
    else
    {
        
        pBssInfo->frequency = wd->sta.currentFrequency;
        pBssInfo->channel = zfChFreqToNum(wd->sta.currentFrequency, &is5G);
    }

    
    pBssInfo->securityType = ZM_SECURITY_TYPE_NONE;

    
    for( i=0; i<6; i++ )
    {
        pBssInfo->macaddr[i] = pProbeRspHeader->sa[i];
    }

    
    for( i=0; i<6; i++ )
    {
        pBssInfo->bssid[i] = pProbeRspHeader->bssid[i];
    }

    
    for( i=0; i<8; i++ )
    {
        pBssInfo->timeStamp[i] = pProbeRspHeader->timeStamp[i];
    }

    
    pBssInfo->beaconInterval[0] = pProbeRspHeader->beaconInterval[0];
    pBssInfo->beaconInterval[1] = pProbeRspHeader->beaconInterval[1];

    
    pBssInfo->capability[0] = pProbeRspHeader->capability[0];
    pBssInfo->capability[1] = pProbeRspHeader->capability[1];

    
    offset = 36;            
    pBssInfo->frameBodysize = zfwBufGetSize(dev, buf)-offset;
    if (pBssInfo->frameBodysize > (ZM_MAX_PROBE_FRAME_BODY_SIZE-1))
    {
        pBssInfo->frameBodysize = ZM_MAX_PROBE_FRAME_BODY_SIZE-1;
    }
    accumulateLen = 0;
    do
    {
        eachIElength = zmw_rx_buf_readb(dev, buf, offset + accumulateLen+1) + 2;  

        if ( (eachIElength >= 2)
             && ((accumulateLen + eachIElength) <= pBssInfo->frameBodysize) )
        {
            zfCopyFromRxBuffer(dev, buf, pBssInfo->frameBody+accumulateLen, offset+accumulateLen, eachIElength);
            accumulateLen+=(u16_t)eachIElength;
        }
        else
        {
            zm_msg0_mm(ZM_LV_1, "probersp frameBodysize abnormal");
            break;
        }
    }
    while(accumulateLen < pBssInfo->frameBodysize);
    pBssInfo->frameBodysize = accumulateLen;

    
    if ( (offset = zfFindElement(dev, buf, ZM_WLAN_EID_SUPPORT_RATE)) == 0xffff )
    {
        zm_debug_msg0("EID(supported rates) not found");
        goto zlError;
    }

    length = zmw_rx_buf_readb(dev, buf, offset+1);
    if ( length == 0 || length > ZM_MAX_SUPP_RATES_IE_SIZE)
    {
        zm_msg0_mm(ZM_LV_0, "Supported rates IE length abnormal");
        goto zlError;
    }
    zfCopyFromRxBuffer(dev, buf, pBssInfo->supportedRates, offset, length+2);



    
    if ( (offset = zfFindElement(dev, buf, ZM_WLAN_EID_COUNTRY)) != 0xffff )
    {
        length = zmw_rx_buf_readb(dev, buf, offset+1);
        if (length > ZM_MAX_COUNTRY_INFO_SIZE)
        {
            length = ZM_MAX_COUNTRY_INFO_SIZE;
        }
        zfCopyFromRxBuffer(dev, buf, pBssInfo->countryInfo, offset, length+2);
        
        if (wd->sta.b802_11D)
        {
            zfHpGetRegulationTablefromISO(dev, (u8_t *)&pBssInfo->countryInfo, 3);
            
            wd->sta.b802_11D = 0;
        }
    }

    
    if ( (offset = zfFindElement(dev, buf, ZM_WLAN_EID_ERP)) != 0xffff )
    {
        pBssInfo->erp = zmw_rx_buf_readb(dev, buf, offset+2);
    }

    
    if ( (offset = zfFindElement(dev, buf, ZM_WLAN_EID_EXTENDED_RATE)) != 0xffff )
    {
        length = zmw_rx_buf_readb(dev, buf, offset+1);
        if (length > ZM_MAX_SUPP_RATES_IE_SIZE)
        {
            zm_msg0_mm(ZM_LV_0, "Extended rates IE length abnormal");
            goto zlError;
        }
        zfCopyFromRxBuffer(dev, buf, pBssInfo->extSupportedRates, offset, length+2);
    }
    else
    {
        pBssInfo->extSupportedRates[0] = 0;
        pBssInfo->extSupportedRates[1] = 0;
    }

    
    if ( (offset = zfFindElement(dev, buf, ZM_WLAN_EID_WPA_IE)) != 0xffff )
    {
        length = zmw_rx_buf_readb(dev, buf, offset+1);
        if (length > ZM_MAX_IE_SIZE)
        {
            length = ZM_MAX_IE_SIZE;
        }
        zfCopyFromRxBuffer(dev, buf, pBssInfo->wpaIe, offset, length+2);
        pBssInfo->securityType = ZM_SECURITY_TYPE_WPA;
    }
    else
    {
        pBssInfo->wpaIe[1] = 0;
    }

    
    if ((offset = zfFindWifiElement(dev, buf, 4, 0xff)) != 0xffff)
    {
        length = zmw_rx_buf_readb(dev, buf, offset+1);
        if (length > ZM_MAX_WPS_IE_SIZE )
        {
            length = ZM_MAX_WPS_IE_SIZE;
        }
        zfCopyFromRxBuffer(dev, buf, pBssInfo->wscIe, offset, length+2);
    }
    else
    {
        pBssInfo->wscIe[1] = 0;
    }

    
    if ((offset = zfFindSuperGElement(dev, buf, ZM_WLAN_EID_VENDOR_PRIVATE)) != 0xffff)
    {
        pBssInfo->apCap |= ZM_SuperG_AP;
    }

    
    if ((offset = zfFindXRElement(dev, buf, ZM_WLAN_EID_VENDOR_PRIVATE)) != 0xffff)
    {
        pBssInfo->apCap |= ZM_XR_AP;
    }

    
    if ( (offset = zfFindElement(dev, buf, ZM_WLAN_EID_RSN_IE)) != 0xffff )
    {
        length = zmw_rx_buf_readb(dev, buf, offset+1);
        if (length > ZM_MAX_IE_SIZE)
        {
            length = ZM_MAX_IE_SIZE;
        }
        zfCopyFromRxBuffer(dev, buf, pBssInfo->rsnIe, offset, length+2);
        pBssInfo->securityType = ZM_SECURITY_TYPE_WPA;
    }
    else
    {
        pBssInfo->rsnIe[1] = 0;
    }
#ifdef ZM_ENABLE_CENC
    
    if ( (offset = zfFindElement(dev, buf, ZM_WLAN_EID_CENC_IE)) != 0xffff )
    {
        length = zmw_rx_buf_readb(dev, buf, offset+1);
        if (length > ZM_MAX_IE_SIZE )
        {
            length = ZM_MAX_IE_SIZE;
        }
        zfCopyFromRxBuffer(dev, buf, pBssInfo->cencIe, offset, length+2);
        pBssInfo->securityType = ZM_SECURITY_TYPE_CENC;
        pBssInfo->capability[0] &= 0xffef;
    }
    else
    {
        pBssInfo->cencIe[1] = 0;
    }
#endif 
    
    
    {
        if ((offset = zfFindWifiElement(dev, buf, 2, 1)) != 0xffff)
        {
            apQosInfo = zmw_rx_buf_readb(dev, buf, offset+8) & 0x80;
            pBssInfo->wmeSupport = 1 | apQosInfo;
        }
        else if ((offset = zfFindWifiElement(dev, buf, 2, 0)) != 0xffff)
        {
            apQosInfo = zmw_rx_buf_readb(dev, buf, offset+8) & 0x80;
            pBssInfo->wmeSupport = 1  | apQosInfo;
        }
        else
        {
            pBssInfo->wmeSupport = 0;
        }
    }
    
    if ((offset = zfFindElement(dev, buf, ZM_WLAN_EID_HT_CAPABILITY)) != 0xffff)
    {
        
        pBssInfo->EnableHT = 1;
        if (zmw_rx_buf_readb(dev, buf, offset+1) & 0x02)
        {
            pBssInfo->enableHT40 = 1;
        }
        else
        {
            pBssInfo->enableHT40 = 0;
        }

        if (zmw_rx_buf_readb(dev, buf, offset+1) & 0x40)
        {
            pBssInfo->SG40 = 1;
        }
        else
        {
            pBssInfo->SG40 = 0;
        }
    }
    else if ((offset = zfFindElement(dev, buf, ZM_WLAN_PREN2_EID_HTCAPABILITY)) != 0xffff)
    {
        
        pBssInfo->EnableHT = 1;
        pBssInfo->apCap |= ZM_All11N_AP;
        if (zmw_rx_buf_readb(dev, buf, offset+2) & 0x02)
        {
            pBssInfo->enableHT40 = 1;
        }
        else
        {
            pBssInfo->enableHT40 = 0;
        }

        if (zmw_rx_buf_readb(dev, buf, offset+2) & 0x40)
        {
            pBssInfo->SG40 = 1;
        }
        else
        {
            pBssInfo->SG40 = 0;
        }
    }
    else
    {
        pBssInfo->EnableHT = 0;
    }
    
    if ((offset = zfFindElement(dev, buf, ZM_WLAN_EID_EXTENDED_HT_CAPABILITY)) != 0xffff)
    {
        
        pBssInfo->extChOffset = zmw_rx_buf_readb(dev, buf, offset+2) & 0x03;
    }
    else if ((offset = zfFindElement(dev, buf, ZM_WLAN_PREN2_EID_HTINFORMATION)) != 0xffff)
    {
        
        pBssInfo->extChOffset = zmw_rx_buf_readb(dev, buf, offset+3) & 0x03;
    }
    else
    {
        pBssInfo->extChOffset = 0;
    }

    if ( (pBssInfo->enableHT40 == 1)
         && ((pBssInfo->extChOffset != 1) && (pBssInfo->extChOffset != 3)) )
    {
        pBssInfo->enableHT40 = 0;
    }

    if (pBssInfo->enableHT40 == 1)
    {
        if (zfHpIsAllowedChannel(dev, pBssInfo->frequency+((pBssInfo->extChOffset==1)?20:-20)) == 0)
        {
            
            pBssInfo->EnableHT = 0;
            pBssInfo->enableHT40 = 0;
            pBssInfo->extChOffset = 0;
        }
    }

    
    if ( ((offset = zfFindElement(dev, buf, ZM_WLAN_EID_EXTENDED_HT_CAPABILITY)) != 0xffff)&&
        ((offset = zfFindBrdcmMrvlRlnkExtCap(dev, buf)) == 0xffff))

    {
        pBssInfo->athOwlAp = 1;
    }
    else
    {
        pBssInfo->athOwlAp = 0;
    }

    
    if ( (pBssInfo->EnableHT == 1) 
         && ((offset = zfFindBroadcomExtCap(dev, buf)) != 0xffff) )
    {
        pBssInfo->broadcomHTAp = 1;
    }
    else
    {
        pBssInfo->broadcomHTAp = 0;
    }

    
    if ((offset = zfFindMarvelExtCap(dev, buf)) != 0xffff)
    {
        pBssInfo->marvelAp = 1;
    }
    else
    {
        pBssInfo->marvelAp = 0;
    }

    
    if ( (offset = zfFindElement(dev, buf, ZM_WLAN_EID_IBSS)) != 0xffff )
    {
        pBssInfo->atimWindow = zmw_rx_buf_readh(dev, buf,offset+2);
    }

    
    if (pBssInfo->frequency > 3000) {
        if (wd->supportMode & ZM_WIRELESS_MODE_5_N) {
#if 0
            if (wd->supportMode & ZM_WIRELESS_MODE_5_54) {
                
                
            } else {
                
                
                if (!pBssInfo->EnableHT) {
                    goto zlError2;
                }
            }
#endif
        } else {
            if (wd->supportMode & ZM_WIRELESS_MODE_5_54) {
                
                
                pBssInfo->EnableHT = 0;
                pBssInfo->enableHT40 = 0;
                pBssInfo->apCap &= (~ZM_All11N_AP);
                pBssInfo->extChOffset = 0;
                pBssInfo->frameBodysize = zfRemoveElement(dev, pBssInfo->frameBody,
                            pBssInfo->frameBodysize, ZM_WLAN_EID_HT_CAPABILITY);
                pBssInfo->frameBodysize = zfRemoveElement(dev, pBssInfo->frameBody,
                            pBssInfo->frameBodysize, ZM_WLAN_PREN2_EID_HTCAPABILITY);
                pBssInfo->frameBodysize = zfRemoveElement(dev, pBssInfo->frameBody,
                            pBssInfo->frameBodysize, ZM_WLAN_EID_EXTENDED_HT_CAPABILITY);
                pBssInfo->frameBodysize = zfRemoveElement(dev, pBssInfo->frameBody,
                            pBssInfo->frameBodysize, ZM_WLAN_PREN2_EID_HTINFORMATION);
            } else {
                
                goto zlError2;
            }
        }
    } else {
        if (wd->supportMode & ZM_WIRELESS_MODE_24_N) {
#if 0
            if (wd->supportMode & ZM_WIRELESS_MODE_24_54) {
                if (wd->supportMode & ZM_WIRELESS_MODE_24_11) {
                    
                    
                } else {
                    
                    
                    if ( (!pBssInfo->EnableHT)
                         && (pBssInfo->extSupportedRates[1] == 0) ) {
                         goto zlError2;
                    }
                }
            } else {
                if (wd->supportMode & ZM_WIRELESS_MODE_24_11) {
                    
                    
                    if ( !pBssInfo->EnableHT ) {
                        if ( zfIsGOnlyMode(dev, pBssInfo->frequency, pBssInfo->supportedRates)
                             || zfIsGOnlyMode(dev, pBssInfo->frequency, pBssInfo->extSupportedRates) ) {
                            goto zlError2;
                        } else {
                            zfGatherBMode(dev, pBssInfo->supportedRates,
                                          pBssInfo->extSupportedRates);
                            pBssInfo->erp = 0;

                            pBssInfo->frameBodysize = zfRemoveElement(dev,
                                pBssInfo->frameBody, pBssInfo->frameBodysize,
                                ZM_WLAN_EID_ERP);
                            pBssInfo->frameBodysize = zfRemoveElement(dev,
                                pBssInfo->frameBody, pBssInfo->frameBodysize,
                                ZM_WLAN_EID_EXTENDED_RATE);

                            pBssInfo->frameBodysize = zfUpdateElement(dev,
                                pBssInfo->frameBody, pBssInfo->frameBodysize,
                                pBssInfo->supportedRates);
                        }
                    }
                } else {
                    
                    
                    if (!pBssInfo->EnableHT) {
                        goto zlError2;
                    }
                }
            }
#endif
        } else {
            
            pBssInfo->EnableHT = 0;
            pBssInfo->enableHT40 = 0;
            pBssInfo->apCap &= (~ZM_All11N_AP);
            pBssInfo->extChOffset = 0;
            pBssInfo->frameBodysize = zfRemoveElement(dev, pBssInfo->frameBody,
                        pBssInfo->frameBodysize, ZM_WLAN_EID_HT_CAPABILITY);
            pBssInfo->frameBodysize = zfRemoveElement(dev, pBssInfo->frameBody,
                        pBssInfo->frameBodysize, ZM_WLAN_PREN2_EID_HTCAPABILITY);
            pBssInfo->frameBodysize = zfRemoveElement(dev, pBssInfo->frameBody,
                        pBssInfo->frameBodysize, ZM_WLAN_EID_EXTENDED_HT_CAPABILITY);
            pBssInfo->frameBodysize = zfRemoveElement(dev, pBssInfo->frameBody,
                        pBssInfo->frameBodysize, ZM_WLAN_PREN2_EID_HTINFORMATION);

            if (wd->supportMode & ZM_WIRELESS_MODE_24_54) {
#if 0
                if (wd->supportMode & ZM_WIRELESS_MODE_24_11) {
                    
                    
                } else {
                    
                    
                    
                    if (pBssInfo->extSupportedRates[1] == 0) {
                         goto zlError2;
                    }
                }
#endif
            } else {
                if (wd->supportMode & ZM_WIRELESS_MODE_24_11) {
                    
                    
                    if ( zfIsGOnlyMode(dev, pBssInfo->frequency, pBssInfo->supportedRates)
                         || zfIsGOnlyMode(dev, pBssInfo->frequency, pBssInfo->extSupportedRates) ) {
                        goto zlError2;
                    } else {
                        zfGatherBMode(dev, pBssInfo->supportedRates,
                                          pBssInfo->extSupportedRates);
                        pBssInfo->erp = 0;

                        pBssInfo->frameBodysize = zfRemoveElement(dev,
                            pBssInfo->frameBody, pBssInfo->frameBodysize,
                            ZM_WLAN_EID_ERP);
                        pBssInfo->frameBodysize = zfRemoveElement(dev,
                            pBssInfo->frameBody, pBssInfo->frameBodysize,
                            ZM_WLAN_EID_EXTENDED_RATE);

                        pBssInfo->frameBodysize = zfUpdateElement(dev,
                            pBssInfo->frameBody, pBssInfo->frameBodysize,
                            pBssInfo->supportedRates);
                    }
                } else {
                    
                    goto zlError2;
                }
            }
        }
    }

    pBssInfo->flag |= ZM_BSS_INFO_VALID_BIT;

zlUpdateRssi:
    
    pBssInfo->tick = wd->tick;

    
    if ( (offset = zfFindElement(dev, buf, ZM_WLAN_EID_ERP)) != 0xffff )
    {
        pBssInfo->erp = zmw_rx_buf_readb(dev, buf, offset+2);
    }

    if( (s8_t)pBssInfo->signalStrength < (s8_t)AddInfo->Tail.Data.SignalStrength1 )
    {
        
        pBssInfo->signalStrength = (u8_t)AddInfo->Tail.Data.SignalStrength1;
        
        pBssInfo->signalQuality = (u8_t)(AddInfo->Tail.Data.SignalStrength1 * 2);

        
        pBssInfo->sortValue = zfComputeBssInfoWeightValue(dev,
                                               (pBssInfo->supportedRates[6] + pBssInfo->extSupportedRates[0]),
                                               pBssInfo->EnableHT,
                                               pBssInfo->enableHT40,
                                               pBssInfo->signalStrength);
    }

    return 0;

zlError:

    return 1;

zlError2:

    return 2;
}

void zfStaProcessBeacon(zdev_t* dev, zbuf_t* buf, struct zsAdditionInfo* AddInfo) 
{
    
    struct zsWlanBeaconFrameHeader*  pBeaconHeader;
    struct zsBssInfo* pBssInfo;
    u8_t   pBuf[sizeof(struct zsWlanBeaconFrameHeader)];
    u8_t   bssid[6];
    int    res;

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    
    zfStaProtErpMonitor(dev, buf);  

    if (zfStaIsConnected(dev))
    {
        ZM_MAC_WORD_TO_BYTE(wd->sta.bssid, bssid);

        if ( wd->wlanMode == ZM_MODE_INFRASTRUCTURE )
        {
            if ( zfRxBufferEqualToStr(dev, buf, bssid, ZM_WLAN_HEADER_A2_OFFSET, 6) )
            {
                zfPowerSavingMgrProcessBeacon(dev, buf);
                zfStaUpdateWmeParameter(dev, buf);
                if (wd->sta.DFSEnable)
                    zfStaUpdateDot11HDFS(dev, buf);
                if (wd->sta.TPCEnable)
                    zfStaUpdateDot11HTPC(dev, buf);
                
                zfStaSignalStatistic(dev, AddInfo->Tail.Data.SignalStrength1,
                        AddInfo->Tail.Data.SignalQuality); 
                wd->sta.rxBeaconCount++;
            }
        }
        else if ( wd->wlanMode == ZM_MODE_IBSS )
        {
            if ( zfRxBufferEqualToStr(dev, buf, bssid, ZM_WLAN_HEADER_A3_OFFSET, 6) )
            {
                int res;
                struct zsPartnerNotifyEvent event;

                zm_debug_msg0("20070916 Receive opposite Beacon!");
                zmw_enter_critical_section(dev);
                wd->sta.ibssReceiveBeaconCount++;
                zmw_leave_critical_section(dev);

                res = zfStaSetOppositeInfoFromRxBuf(dev, buf);
                if ( res == 0 )
                {
                    
                    zfInitPartnerNotifyEvent(dev, buf, &event);
                    if (wd->zfcbIbssPartnerNotify != NULL)
                    {
                        wd->zfcbIbssPartnerNotify(dev, 1, &event);
                    }
                }
                
                zfStaSignalStatistic(dev, AddInfo->Tail.Data.SignalStrength1,
                        AddInfo->Tail.Data.SignalQuality); 
            }
            
            
            
#if 0
            else if ( wd->sta.oppositeCount == 0 )
            {   
                if ( (offset = zfFindElement(dev, buf, ZM_WLAN_EID_SSID)) != 0xffff )
                {
                    if ( (wd->sta.ssidLen == zmw_buf_readb(dev, buf, offset+1))&&
                         (zfRxBufferEqualToStr(dev, buf, wd->sta.ssid,
                                               offset+2, wd->sta.ssidLen)) )
                    {
                        capabilityInfo = zmw_buf_readh(dev, buf, 34);

                        if ( capabilityInfo & ZM_BIT_1 )
                        {
                            if ( (wd->sta.capability[0] & ZM_BIT_4) ==
                                 (capabilityInfo & ZM_BIT_4) )
                            {
                                zm_debug_msg0("IBSS merge");
                                zfCopyFromRxBuffer(dev, buf, bssid,
                                                   ZM_WLAN_HEADER_A3_OFFSET, 6);
                                zfUpdateBssid(dev, bssid);
                            }
                        }
                    }
                }
            }
#endif
        }
    }

    
    if ( !wd->sta.bChannelScan )
    {
        goto zlReturn;
    }

    zfCopyFromRxBuffer(dev, buf, pBuf, 0, sizeof(struct zsWlanBeaconFrameHeader));
    pBeaconHeader = (struct zsWlanBeaconFrameHeader*) pBuf;

    zmw_enter_critical_section(dev);

    

    pBssInfo = zfStaFindBssInfo(dev, buf, pBeaconHeader);

    if ( pBssInfo == NULL )
    {
        
        pBssInfo = zfBssInfoAllocate(dev);
        if (pBssInfo != NULL)
        {
            res = zfStaInitBssInfo(dev, buf, pBeaconHeader, pBssInfo, AddInfo, 0);
            
            if ( res != 0 )
            {
                zfBssInfoFree(dev, pBssInfo);
            }
            else
            {
                zfBssInfoInsertToList(dev, pBssInfo);
            }
        }
    }
    else
    {
        res = zfStaInitBssInfo(dev, buf, pBeaconHeader, pBssInfo, AddInfo, 1);
        if (res == 2)
        {
            zfBssInfoRemoveFromList(dev, pBssInfo);
            zfBssInfoFree(dev, pBssInfo);
        }
        else if ( wd->wlanMode == ZM_MODE_IBSS )
        {
            int idx;

            
            zfStaFindFreeOpposite(dev, (u16_t *)pBssInfo->macaddr, &idx);
        }
    }

    zmw_leave_critical_section(dev);

zlReturn:

    return;
}


void zfAuthFreqCompleteCb(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);
    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);

    if (wd->sta.connectState == ZM_STA_CONN_STATE_AUTH_COMPLETED)
    {
        zm_debug_msg0("ZM_STA_CONN_STATE_ASSOCIATE");
        wd->sta.connectTimer = wd->tick;
        wd->sta.connectState = ZM_STA_CONN_STATE_ASSOCIATE;
    }

    zmw_leave_critical_section(dev);
    return;
}





















void zfStaProcessAuth(zdev_t* dev, zbuf_t* buf, u16_t* src, u16_t apId)
{
    struct zsWlanAuthFrameHeader* pAuthFrame;
    u8_t  pBuf[sizeof(struct zsWlanAuthFrameHeader)];
    u32_t p1, p2;

    zmw_get_wlan_dev(dev);
    zmw_declare_for_critical_section();

    if ( !zfStaIsConnecting(dev) )
    {
        return;
    }

    pAuthFrame = (struct zsWlanAuthFrameHeader*) pBuf;
    zfCopyFromRxBuffer(dev, buf, pBuf, 0, sizeof(struct zsWlanAuthFrameHeader));

    if ( wd->sta.connectState == ZM_STA_CONN_STATE_AUTH_OPEN )
    {
        if ( (zmw_le16_to_cpu(pAuthFrame->seq) == 2)&&
             (zmw_le16_to_cpu(pAuthFrame->algo) == 0)&&
             (zmw_le16_to_cpu(pAuthFrame->status) == 0) )
        {

            zmw_enter_critical_section(dev);
            wd->sta.connectTimer = wd->tick;
            zm_debug_msg0("ZM_STA_CONN_STATE_AUTH_COMPLETED");
            wd->sta.connectState = ZM_STA_CONN_STATE_AUTH_COMPLETED;
            zmw_leave_critical_section(dev);

            
            
            zfCoreSetFrequencyEx(dev, wd->frequency, wd->BandWidth40,
                    wd->ExtOffset, zfAuthFreqCompleteCb);

            
            if ( wd->sta.connectByReasso )
            {
                zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_REASOCREQ,
                              wd->sta.bssid, 0, 0, 0);
            }
            else
            {
                zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_ASOCREQ,
                              wd->sta.bssid, 0, 0, 0);
            }


        }
        else
        {
            zm_debug_msg1("authentication failed, status = ",
                          pAuthFrame->status);

            if (wd->sta.authMode == ZM_AUTH_MODE_AUTO)
            {
                wd->sta.bIsSharedKey = 1;
                zfStaStartConnect(dev, wd->sta.bIsSharedKey);
            }
            else
            {
                zm_debug_msg0("ZM_STA_STATE_DISCONNECT");
                zfStaConnectFail(dev, ZM_STATUS_MEDIA_DISCONNECT_AUTH_FAILED, wd->sta.bssid, 3);
            }
        }
    }
    else if ( wd->sta.connectState == ZM_STA_CONN_STATE_AUTH_SHARE_1 )
    {
        if ( (zmw_le16_to_cpu(pAuthFrame->algo) == 1) &&
             (zmw_le16_to_cpu(pAuthFrame->seq) == 2) &&
             (zmw_le16_to_cpu(pAuthFrame->status) == 0))
              
        {
            zfMemoryCopy(wd->sta.challengeText, pAuthFrame->challengeText,
                         pAuthFrame->challengeText[1]+2);

            
            p1 = 0x30001;
            p2 = 0;
            zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_AUTH,
                          wd->sta.bssid, p1, p2, 0);

            zmw_enter_critical_section(dev);
            wd->sta.connectTimer = wd->tick;

            zm_debug_msg0("ZM_STA_SUB_STATE_AUTH_SHARE_2");
            wd->sta.connectState = ZM_STA_CONN_STATE_AUTH_SHARE_2;
            zmw_leave_critical_section(dev);
        }
        else
        {
            zm_debug_msg1("authentication failed, status = ",
                          pAuthFrame->status);

            zm_debug_msg0("ZM_STA_STATE_DISCONNECT");
            zfStaConnectFail(dev, ZM_STATUS_MEDIA_DISCONNECT_AUTH_FAILED, wd->sta.bssid, 3);
        }
    }
    else if ( wd->sta.connectState == ZM_STA_CONN_STATE_AUTH_SHARE_2 )
    {
        if ( (zmw_le16_to_cpu(pAuthFrame->algo) == 1)&&
             (zmw_le16_to_cpu(pAuthFrame->seq) == 4)&&
             (zmw_le16_to_cpu(pAuthFrame->status) == 0) )
        {
            
            
            zfCoreSetFrequencyEx(dev, wd->frequency, wd->BandWidth40,
                    wd->ExtOffset, NULL);

            
            zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_ASOCREQ,
                          wd->sta.bssid, 0, 0, 0);

            zmw_enter_critical_section(dev);
            wd->sta.connectTimer = wd->tick;

            zm_debug_msg0("ZM_STA_SUB_STATE_ASSOCIATE");
            wd->sta.connectState = ZM_STA_CONN_STATE_ASSOCIATE;
            zmw_leave_critical_section(dev);
        }
        else
        {
            zm_debug_msg1("authentication failed, status = ",
                          pAuthFrame->status);

            zm_debug_msg0("ZM_STA_STATE_DISCONNECT");
            zfStaConnectFail(dev, ZM_STATUS_MEDIA_DISCONNECT_AUTH_FAILED, wd->sta.bssid, 3);
        }
    }
    else
    {
        zm_debug_msg0("unknown case");
    }
}

void zfStaProcessAsocReq(zdev_t* dev, zbuf_t* buf, u16_t* src, u16_t apId)
{

    return;
}

void zfStaProcessAsocRsp(zdev_t* dev, zbuf_t* buf)
{
    struct zsWlanAssoFrameHeader* pAssoFrame;
    u8_t  pBuf[sizeof(struct zsWlanAssoFrameHeader)];
    u16_t offset;
    u32_t i;
    u32_t oneTxStreamCap;

    zmw_get_wlan_dev(dev);

    if ( !zfStaIsConnecting(dev) )
    {
        return;
    }

    pAssoFrame = (struct zsWlanAssoFrameHeader*) pBuf;
    zfCopyFromRxBuffer(dev, buf, pBuf, 0, sizeof(struct zsWlanAssoFrameHeader));

    if ( wd->sta.connectState == ZM_STA_CONN_STATE_ASSOCIATE )
    {
        if ( pAssoFrame->status == 0 )
        {
            zm_debug_msg0("ZM_STA_STATE_CONNECTED");

            if (wd->sta.EnableHT == 1)
            {
                wd->sta.wmeConnected = 1;
            }
            if ((wd->sta.wmeEnabled & ZM_STA_WME_ENABLE_BIT) != 0) 
            {
                
                if ((offset = zfFindWifiElement(dev, buf, 2, 1)) != 0xffff)
                {
                    zm_debug_msg0("WME enable");
                    wd->sta.wmeConnected = 1;
                    if ((wd->sta.wmeEnabled & ZM_STA_UAPSD_ENABLE_BIT) != 0)
                    {
                        if ((zmw_rx_buf_readb(dev, buf, offset+8) & 0x80) != 0)
                        {
                            zm_debug_msg0("UAPSD enable");
                            wd->sta.qosInfo = wd->sta.wmeQosInfo;
                        }
                    }

                    zfStaUpdateWmeParameter(dev, buf);
                }
            }


            
            wd->sta.asocRspFrameBodySize = zfwBufGetSize(dev, buf)-24;
            if (wd->sta.asocRspFrameBodySize > ZM_CACHED_FRAMEBODY_SIZE)
            {
                wd->sta.asocRspFrameBodySize = ZM_CACHED_FRAMEBODY_SIZE;
            }
            for (i=0; i<wd->sta.asocRspFrameBodySize; i++)
            {
                wd->sta.asocRspFrameBody[i] = zmw_rx_buf_readb(dev, buf, i+24);
            }

            zfStaStoreAsocRspIe(dev, buf);
            if (wd->sta.EnableHT &&
                ((wd->sta.ie.HtCap.HtCapInfo & HTCAP_SupChannelWidthSet) != 0) &&
                (wd->ExtOffset != 0))
            {
                wd->sta.htCtrlBandwidth = 1;
            }
            else
            {
                wd->sta.htCtrlBandwidth = 0;
            }

            
            
            

            if (wd->sta.EnableHT == 1)
            {
                wd->addbaComplete = 0;

                if ((wd->sta.SWEncryptEnable & ZM_SW_TKIP_ENCRY_EN) == 0 &&
                    (wd->sta.SWEncryptEnable & ZM_SW_WEP_ENCRY_EN) == 0)
                {
                    wd->addbaCount = 1;
                    zfAggSendAddbaRequest(dev, wd->sta.bssid, 0, 0);
                    zfTimerSchedule(dev, ZM_EVENT_TIMEOUT_ADDBA, 100);
                }
            }

            
            if(wd->sta.ie.HtInfo.ChannelInfo & ExtHtCap_RIFSMode)
            {
                wd->sta.HT2040 = 1;

            }

            wd->sta.aid = pAssoFrame->aid & 0x3fff;
            wd->sta.oppositeCount = 0;    
            zfStaSetOppositeInfoFromRxBuf(dev, buf);

            wd->sta.rxBeaconCount = 16;

            zfChangeAdapterState(dev, ZM_STA_STATE_CONNECTED);
            wd->sta.connPowerInHalfDbm = zfHpGetTransmitPower(dev);
            if (wd->zfcbConnectNotify != NULL)
            {
                if (wd->sta.EnableHT != 0) 
            	{
    		        oneTxStreamCap = (zfHpCapability(dev) & ZM_HP_CAP_11N_ONE_TX_STREAM);
    		        if (wd->sta.htCtrlBandwidth == 1) 
    		        {
    					if(oneTxStreamCap) 
    				    {
    				        if (wd->sta.SG40)
    				        {
    				            wd->CurrentTxRateKbps = 150000;
    						    wd->CurrentRxRateKbps = 300000;
    				        }
    				        else
    				        {
    				            wd->CurrentTxRateKbps = 135000;
    						    wd->CurrentRxRateKbps = 270000;
    				        }
    				    }
    				    else 
    				    {
    				        if (wd->sta.SG40)
    				        {
    				            wd->CurrentTxRateKbps = 300000;
    						    wd->CurrentRxRateKbps = 300000;
    				        }
    				        else
    				        {
    				            wd->CurrentTxRateKbps = 270000;
    						    wd->CurrentRxRateKbps = 270000;
    				        }
    				    }
    		        }
    		        else 
    		        {
    		            if(oneTxStreamCap) 
    				    {
    				        wd->CurrentTxRateKbps = 650000;
    						wd->CurrentRxRateKbps = 130000;
    				    }
    				    else 
    				    {
    				        wd->CurrentTxRateKbps = 130000;
    					    wd->CurrentRxRateKbps = 130000;
    				    }
    		        }
                }
                else 
                {
                    if (wd->sta.connection_11b != 0)
                    {
                        wd->CurrentTxRateKbps = 11000;
    			        wd->CurrentRxRateKbps = 11000;
                    }
                    else
                    {
                        wd->CurrentTxRateKbps = 54000;
    			        wd->CurrentRxRateKbps = 54000;
    			    }
                }


                wd->zfcbConnectNotify(dev, ZM_STATUS_MEDIA_CONNECT, wd->sta.bssid);
            }
            wd->sta.connectByReasso = TRUE;
            wd->sta.failCntOfReasso = 0;

            zfPowerSavingMgrConnectNotify(dev);

            
            
            
            
            
            
            
        }
        else
        {
            zm_debug_msg1("association failed, status = ",
                          pAssoFrame->status);

            zm_debug_msg0("ZM_STA_STATE_DISCONNECT");
            wd->sta.connectByReasso = FALSE;
            zfStaConnectFail(dev, ZM_STATUS_MEDIA_DISCONNECT_ASOC_FAILED, wd->sta.bssid, 3);
        }
    }

}

void zfStaStoreAsocRspIe(zdev_t* dev, zbuf_t* buf)
{
    u16_t offset;
    u32_t i;
    u16_t length;
    u8_t  *htcap;
    u8_t  asocBw40 = 0;
    u8_t  asocExtOffset = 0;

    zmw_get_wlan_dev(dev);

    for (i=0; i<wd->sta.asocRspFrameBodySize; i++)
    {
        wd->sta.asocRspFrameBody[i] = zmw_rx_buf_readb(dev, buf, i+24);
    }

    
    if (    ((wd->sta.currentFrequency > 3000) && !(wd->supportMode & ZM_WIRELESS_MODE_5_N))
         || ((wd->sta.currentFrequency < 3000) && !(wd->supportMode & ZM_WIRELESS_MODE_24_N)) )
    {
        
        htcap = (u8_t *)&wd->sta.ie.HtCap;
        for (i=0; i<28; i++)
        {
            htcap[i] = 0;
        }
        wd->BandWidth40 = 0;
        wd->ExtOffset = 0;
        return;
    }

    if ((offset = zfFindElement(dev, buf, ZM_WLAN_EID_HT_CAPABILITY)) != 0xffff)
    {
        
        zm_debug_msg0("atheros pre n");
        htcap = (u8_t *)&wd->sta.ie.HtCap;
        htcap[0] = zmw_rx_buf_readb(dev, buf, offset);
        htcap[1] = 26;
        for (i=1; i<=26; i++)
        {
            htcap[i+1] = zmw_rx_buf_readb(dev, buf, offset + i);
            zm_msg2_mm(ZM_LV_1, "ASOC:  HT Capabilities, htcap=", htcap[i+1]);
        }
    }
    else if ((offset = zfFindElement(dev, buf, ZM_WLAN_PREN2_EID_HTCAPABILITY)) != 0xffff)
    {
        
        zm_debug_msg0("pre n 2.0 standard");
        htcap = (u8_t *)&wd->sta.ie.HtCap;
        for (i=0; i<28; i++)
        {
            htcap[i] = zmw_rx_buf_readb(dev, buf, offset + i);
            zm_msg2_mm(ZM_LV_1, "ASOC:  HT Capabilities, htcap=", htcap[i]);
        }
    }
    else
    {
        
        htcap = (u8_t *)&wd->sta.ie.HtCap;
        for (i=0; i<28; i++)
        {
            htcap[i] = 0;
        }
        wd->BandWidth40 = 0;
        wd->ExtOffset = 0;
        return;
    }

    asocBw40 = (u8_t)((wd->sta.ie.HtCap.HtCapInfo & HTCAP_SupChannelWidthSet) >> 1);

    
    if ((offset = zfFindElement(dev, buf, ZM_WLAN_EID_EXTENDED_HT_CAPABILITY)) != 0xffff)
    {
        
        zm_debug_msg0("atheros pre n HTINFO");
        length = 22;
        htcap = (u8_t *)&wd->sta.ie.HtInfo;
        htcap[0] = zmw_rx_buf_readb(dev, buf, offset);
        htcap[1] = 22;
        for (i=1; i<=22; i++)
        {
            htcap[i+1] = zmw_rx_buf_readb(dev, buf, offset + i);
            zm_msg2_mm(ZM_LV_1, "ASOC:  HT Info, htinfo=", htcap[i+1]);
        }
    }
    else if ((offset = zfFindElement(dev, buf, ZM_WLAN_PREN2_EID_HTINFORMATION)) != 0xffff)
    {
        
        zm_debug_msg0("pre n 2.0 standard HTINFO");
        length = zmw_rx_buf_readb(dev, buf, offset + 1);
        htcap = (u8_t *)&wd->sta.ie.HtInfo;
        for (i=0; i<24; i++)
        {
            htcap[i] = zmw_rx_buf_readb(dev, buf, offset + i);
            zm_msg2_mm(ZM_LV_1, "ASOC:  HT Info, htinfo=", htcap[i]);
        }
    }
    else
    {
        zm_debug_msg0("no HTINFO");
        htcap = (u8_t *)&wd->sta.ie.HtInfo;
        for (i=0; i<24; i++)
        {
            htcap[i] = 0;
        }
    }
    asocExtOffset = wd->sta.ie.HtInfo.ChannelInfo & ExtHtCap_ExtChannelOffsetBelow;

    if ((wd->sta.EnableHT == 1) && (asocBw40 == 1) && ((asocExtOffset == 1) || (asocExtOffset == 3)))
    {
        wd->BandWidth40 = asocBw40;
        wd->ExtOffset = asocExtOffset;
    }
    else
    {
        wd->BandWidth40 = 0;
        wd->ExtOffset = 0;
    }

    return;
}

void zfStaProcessDeauth(zdev_t* dev, zbuf_t* buf)
{
    u16_t apMacAddr[3];

    zmw_get_wlan_dev(dev);
    zmw_declare_for_critical_section();

    
    if ( wd->wlanMode == ZM_MODE_INFRASTRUCTURE )
    {
        apMacAddr[0] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A3_OFFSET);
        apMacAddr[1] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A3_OFFSET+2);
        apMacAddr[2] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A3_OFFSET+4);
  	if ((apMacAddr[0] == wd->sta.bssid[0]) && (apMacAddr[1] == wd->sta.bssid[1]) && (apMacAddr[2] == wd->sta.bssid[2]))
        {
            if (zfwBufGetSize(dev, buf) >= 24+2) 
            {
                if ( zfStaIsConnected(dev) )
                {
                    zfStaConnectFail(dev, ZM_STATUS_MEDIA_DISCONNECT_DEAUTH, wd->sta.bssid, 2);
                }
                else if (zfStaIsConnecting(dev))
                {
                    zfStaConnectFail(dev, ZM_STATUS_MEDIA_DISCONNECT_AUTH_FAILED, wd->sta.bssid, 3);
                }
                else
                {
                }
            }
        }
    }
    else if ( wd->wlanMode == ZM_MODE_IBSS )
    {
        u16_t peerMacAddr[3];
        u8_t  peerIdx;
        s8_t  res;

        if ( zfStaIsConnected(dev) )
        {
            peerMacAddr[0] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A2_OFFSET);
            peerMacAddr[1] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A2_OFFSET+2);
            peerMacAddr[2] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A2_OFFSET+4);

            zmw_enter_critical_section(dev);
            res = zfStaFindOppositeByMACAddr(dev, peerMacAddr, &peerIdx);
            if ( res == 0 )
            {
                wd->sta.oppositeInfo[peerIdx].aliveCounter = 0;
            }
            zmw_leave_critical_section(dev);
        }
    }
}

void zfStaProcessDisasoc(zdev_t* dev, zbuf_t* buf)
{
    u16_t apMacAddr[3];

    zmw_get_wlan_dev(dev);

    
    if ( wd->wlanMode == ZM_MODE_INFRASTRUCTURE )
    {
        apMacAddr[0] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A3_OFFSET);
        apMacAddr[1] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A3_OFFSET+2);
        apMacAddr[2] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A3_OFFSET+4);

        if ((apMacAddr[0] == wd->sta.bssid[0]) && (apMacAddr[1] == wd->sta.bssid[1]) && (apMacAddr[2] == wd->sta.bssid[2]))
        {
            if (zfwBufGetSize(dev, buf) >= 24+2) 
            {
                if ( zfStaIsConnected(dev) )
                {
                    zfStaConnectFail(dev, ZM_STATUS_MEDIA_DISCONNECT_DISASOC, wd->sta.bssid, 2);
                }
                else
                {
                    zfStaConnectFail(dev, ZM_STATUS_MEDIA_DISCONNECT_ASOC_FAILED, wd->sta.bssid, 3);
                }
            }
        }
    }
}



















void zfStaProcessProbeReq(zdev_t* dev, zbuf_t* buf, u16_t* src)
{
    u16_t offset;
    u8_t len;
    u16_t i, j;
    u16_t sendFlag;

    zmw_get_wlan_dev(dev);

    
    if ((wd->wlanMode != ZM_MODE_AP) || (wd->wlanMode != ZM_MODE_IBSS))
    {
        zm_msg0_mm(ZM_LV_3, "Ignore probe req");
        return;
    }

    
    if ((offset = zfFindElement(dev, buf, ZM_WLAN_EID_SSID)) == 0xffff)
    {
        zm_msg0_mm(ZM_LV_3, "probe req SSID not found");
        return;
    }

    len = zmw_rx_buf_readb(dev, buf, offset+1);

    for (i=0; i<ZM_MAX_AP_SUPPORT; i++)
    {
        if ((wd->ap.apBitmap & (i<<i)) != 0)
        {
            sendFlag = 0;
            
            if ((len == 0) && (wd->ap.hideSsid[i] == 0))
            {
                sendFlag = 1;
            }
            
            else if (wd->ap.ssidLen[i] == len)
            {
                for (j=0; j<len; j++)
                {
                    if (zmw_rx_buf_readb(dev, buf, offset+1+j)
                            != wd->ap.ssid[i][j])
                    {
                        break;
                    }
                }
                if (j == len)
                {
                    sendFlag = 1;
                }
            }
            if (sendFlag == 1)
            {
                
                zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_PROBERSP, src, i, 0, 0);
            }
        }
    }
}

void zfStaProcessProbeRsp(zdev_t* dev, zbuf_t* buf, struct zsAdditionInfo* AddInfo)
{
    
    
    
    
    #if 0
    zmw_get_wlan_dev(dev);

    if ( !wd->sta.bChannelScan )
    {
        return;
    }
    #endif

    zfProcessProbeRsp(dev, buf, AddInfo);
}

void zfIBSSSetupBssDesc(zdev_t *dev)
{
#ifdef ZM_ENABLE_IBSS_WPA2PSK
    u8_t i;
#endif
    struct zsBssInfo *pBssInfo;
    u16_t offset = 0;

    zmw_get_wlan_dev(dev);

    pBssInfo = &wd->sta.ibssBssDesc;
    zfZeroMemory((u8_t *)pBssInfo, sizeof(struct zsBssInfo));

    pBssInfo->signalStrength = 100;

    zfMemoryCopy((u8_t *)pBssInfo->macaddr, (u8_t *)wd->macAddr,6);
    zfMemoryCopy((u8_t *)pBssInfo->bssid, (u8_t *)wd->sta.bssid, 6);

    pBssInfo->beaconInterval[0] = (u8_t)(wd->beaconInterval) ;
    pBssInfo->beaconInterval[1] = (u8_t)((wd->beaconInterval) >> 8) ;

    pBssInfo->capability[0] = wd->sta.capability[0];
    pBssInfo->capability[1] = wd->sta.capability[1];

    pBssInfo->ssid[0] = ZM_WLAN_EID_SSID;
    pBssInfo->ssid[1] = wd->sta.ssidLen;
    zfMemoryCopy((u8_t *)&pBssInfo->ssid[2], (u8_t *)wd->sta.ssid, wd->sta.ssidLen);
    zfMemoryCopy((u8_t *)&pBssInfo->frameBody[offset], (u8_t *)pBssInfo->ssid,
                 wd->sta.ssidLen + 2);
    offset += wd->sta.ssidLen + 2;

    

    
    pBssInfo->channel = zfChFreqToNum(wd->frequency, NULL);
    pBssInfo->frequency = wd->frequency;
    pBssInfo->atimWindow = wd->sta.atimWindow;

#ifdef ZM_ENABLE_IBSS_WPA2PSK
    if ( wd->sta.authMode == ZM_AUTH_MODE_WPA2PSK )
    {
        u8_t rsn[64]=
        {
                    
                    0x30,
                    
                    0x14,
                    
                    0x01, 0x00,
                    
                    0x00, 0x0f, 0xac, 0x04,
                    
                    0x01, 0x00,
                    
                    0x00, 0x0f, 0xac, 0x02,
                    
                    0x01, 0x00,
                    
                    0x00, 0x0f, 0xac, 0x02,
                    
                    0x00, 0x00
        };

        
        zfMemoryCopy(rsn+4, zgWpa2AesOui, 4);

        if ( wd->sta.wepStatus == ZM_ENCRYPTION_AES )
        {
            
            zfMemoryCopy(rsn+10, zgWpa2AesOui, 4);
        }

        
        pBssInfo->frameBody[offset++] = ZM_WLAN_EID_RSN_IE ;

        
        pBssInfo->frameBody[offset++] = rsn[1] ;

        
        for(i=0; i<rsn[1]; i++)
        {
            pBssInfo->frameBody[offset++] = rsn[i+2] ;
        }

        zfMemoryCopy(pBssInfo->rsnIe, rsn, rsn[1]+2);
    }
#endif
}

void zfIbssConnectNetwork(zdev_t* dev)
{
    struct zsBssInfo* pBssInfo;
    struct zsBssInfo tmpBssInfo;
    u8_t   macAddr[6], bssid[6], bssNotFound = TRUE;
    u16_t  i, j=100;
    u16_t  k;
    struct zsPartnerNotifyEvent event;
    u32_t  channelFlags;
    u16_t  oppositeWepStatus;

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    
    zfChangeAdapterState(dev, ZM_STA_STATE_CONNECTING);
    zfPowerSavingMgrWakeup(dev);

    
    zfUpdateDefaultQosParameter(dev, 0);

    wd->sta.bProtectionMode = FALSE;
    zfHpSetSlotTime(dev, 1);

    
    wd->sta.capability[0] &= ~ZM_BIT_0;
    
    wd->sta.capability[0] |= ZM_BIT_1;
    
    wd->sta.capability[1] &= ~ZM_BIT_2;

    wd->sta.wmeConnected = 0;
    wd->sta.psMgr.tempWakeUp = 0;
    wd->sta.qosInfo = 0;
    wd->sta.EnableHT = 0;
    wd->BandWidth40 = 0;
    wd->ExtOffset = 0;

    if ( wd->sta.bssList.bssCount )
    {
        
        zfBssInfoReorderList(dev);

        zmw_enter_critical_section(dev);

        pBssInfo = wd->sta.bssList.head;

        for(i=0; i<wd->sta.bssList.bssCount; i++)
        {
            
            if ( pBssInfo->capability[0] & ZM_BIT_4 )
            { 

                oppositeWepStatus = ZM_ENCRYPTION_WEP_ENABLED;

                if ( pBssInfo->rsnIe[1] != 0 )
                {
                    if ( (pBssInfo->rsnIe[7] == 0x01) || (pBssInfo->rsnIe[7] == 0x05) )
                    { 

                        oppositeWepStatus = ZM_ENCRYPTION_WEP_ENABLED;
                    }
                    else if ( pBssInfo->rsnIe[7] == 0x02 )
                    { 

                        oppositeWepStatus = ZM_ENCRYPTION_TKIP;
                    }
                    else if ( pBssInfo->rsnIe[7] == 0x04 )
                    { 

                        oppositeWepStatus = ZM_ENCRYPTION_AES;
                    }
                }
            }
            else
            {

                oppositeWepStatus = ZM_ENCRYPTION_WEP_DISABLED;
            }

            if ( (zfMemoryIsEqual(&(pBssInfo->ssid[2]), wd->sta.ssid,
                                  wd->sta.ssidLen))&&
                 (wd->sta.ssidLen == pBssInfo->ssid[1])&&
                 (oppositeWepStatus == wd->sta.wepStatus) )
            {
                
                if (pBssInfo->frequency > 3000) {
                    if (   (pBssInfo->EnableHT == 1)
                        || (pBssInfo->apCap & ZM_All11N_AP) ) 
                    {
                        channelFlags = CHANNEL_A_HT;
                        if (pBssInfo->enableHT40 == 1) {
                            channelFlags |= CHANNEL_HT40;
                        }
                    } else {
                        channelFlags = CHANNEL_A;
                    }
                } else {
                    if (   (pBssInfo->EnableHT == 1)
                        || (pBssInfo->apCap & ZM_All11N_AP) ) 
                    {
                        channelFlags = CHANNEL_G_HT;
                        if(pBssInfo->enableHT40 == 1) {
                            channelFlags |= CHANNEL_HT40;
                        }
                    } else {
                        if (pBssInfo->extSupportedRates[1] == 0) {
                            channelFlags = CHANNEL_B;
                        } else {
                            channelFlags = CHANNEL_G;
                        }
                    }
                }

                if (   ((channelFlags == CHANNEL_B) && (wd->connectMode & ZM_BIT_0))
                    || ((channelFlags == CHANNEL_G) && (wd->connectMode & ZM_BIT_1))
                    || ((channelFlags == CHANNEL_A) && (wd->connectMode & ZM_BIT_2))
                    || ((channelFlags & CHANNEL_HT20) && (wd->connectMode & ZM_BIT_3)) )
                {
                    pBssInfo = pBssInfo->next;
                    continue;
                }

                
                if (zfHpIsDfsChannelNCS(dev, pBssInfo->frequency))
                {
                    zm_debug_msg0("Bypass DFS channel");
                    continue;
                }

                
                if ( pBssInfo->capability[0] & ZM_BIT_1 )
                {
                    
                    j = i;
                    break;
                }
            }

            pBssInfo = pBssInfo->next;
        }

        if ((j < wd->sta.bssList.bssCount) && (pBssInfo != NULL))
        {
            zfwMemoryCopy((u8_t*)&tmpBssInfo, (u8_t*)(pBssInfo), sizeof(struct zsBssInfo));
            pBssInfo = &tmpBssInfo;
        }
        else
        {
            pBssInfo = NULL;
        }

        zmw_leave_critical_section(dev);

        
        if (pBssInfo != NULL)
        {
            int res;

            zm_debug_msg0("IBSS found");

            
            zmw_enter_critical_section(dev);
            wd->sta.bssNotFoundCount = 0;
            zmw_leave_critical_section(dev);

            bssNotFound = FALSE;
            wd->sta.atimWindow = pBssInfo->atimWindow;
            wd->frequency = pBssInfo->frequency;
            
            zfCoreSetFrequency(dev, wd->frequency);
            zfUpdateBssid(dev, pBssInfo->bssid);
            zfResetSupportRate(dev, ZM_DEFAULT_SUPPORT_RATE_ZERO);
            zfUpdateSupportRate(dev, pBssInfo->supportedRates);
            zfUpdateSupportRate(dev, pBssInfo->extSupportedRates);
            wd->beaconInterval = pBssInfo->beaconInterval[0] +
                                 (((u16_t) pBssInfo->beaconInterval[1]) << 8);

            if (wd->beaconInterval == 0)
            {
                wd->beaconInterval = 100;
            }

            
            if ( pBssInfo->rsnIe[1] != 0 )
            {
                zfMemoryCopy(wd->sta.rsnIe, pBssInfo->rsnIe,
                             pBssInfo->rsnIe[1]+2);

#ifdef ZM_ENABLE_IBSS_WPA2PSK
                
                zmw_enter_critical_section(dev);
                wd->sta.ibssWpa2Psk = 1;
                zmw_leave_critical_section(dev);
#endif
            }
            else
            {
                wd->sta.rsnIe[1] = 0;
            }

            
            if ( pBssInfo->capability[0] & ZM_BIT_4 )
            {
                wd->sta.capability[0] |= ZM_BIT_4;
            }
            else
            {
                wd->sta.capability[0] &= ~ZM_BIT_4;
            }

            
            wd->preambleTypeInUsed = wd->preambleType;
            if ( wd->preambleTypeInUsed == ZM_PREAMBLE_TYPE_AUTO )
            {
                if (pBssInfo->capability[0] & ZM_BIT_5)
                {
                    wd->preambleTypeInUsed = ZM_PREAMBLE_TYPE_SHORT;
                }
                else
                {
                    wd->preambleTypeInUsed = ZM_PREAMBLE_TYPE_LONG;
                }
            }

            if (wd->preambleTypeInUsed == ZM_PREAMBLE_TYPE_LONG)
            {
                wd->sta.capability[0] &= ~ZM_BIT_5;
            }
            else
            {
                wd->sta.capability[0] |= ZM_BIT_5;
            }

            wd->sta.beaconFrameBodySize = pBssInfo->frameBodysize + 12;

            if (wd->sta.beaconFrameBodySize > ZM_CACHED_FRAMEBODY_SIZE)
            {
                wd->sta.beaconFrameBodySize = ZM_CACHED_FRAMEBODY_SIZE;
            }

            for (k=0; k<8; k++)
            {
                wd->sta.beaconFrameBody[k] = pBssInfo->timeStamp[k];
            }
            wd->sta.beaconFrameBody[8] = pBssInfo->beaconInterval[0];
            wd->sta.beaconFrameBody[9] = pBssInfo->beaconInterval[1];
            wd->sta.beaconFrameBody[10] = pBssInfo->capability[0];
            wd->sta.beaconFrameBody[11] = pBssInfo->capability[1];
            
            for (k=0; k<pBssInfo->frameBodysize; k++)
            {
                wd->sta.beaconFrameBody[k+12] = pBssInfo->frameBody[k];
            }

            zmw_enter_critical_section(dev);
            res = zfStaSetOppositeInfoFromBSSInfo(dev, pBssInfo);
            if ( res == 0 )
            {
                zfMemoryCopy(event.bssid, (u8_t *)(pBssInfo->bssid), 6);
                zfMemoryCopy(event.peerMacAddr, (u8_t *)(pBssInfo->macaddr), 6);
            }
            zmw_leave_critical_section(dev);

            
            goto connect_done;
        }
    }

    
    if ( bssNotFound )
    {
#ifdef ZM_ENABLE_IBSS_WPA2PSK
        u16_t offset ;
#endif
        if ( wd->sta.ibssJoinOnly )
        {
            zm_debug_msg0("IBSS join only...retry...");
            goto retry_ibss;
        }

        if(wd->sta.bssNotFoundCount<2)
        {
            zmw_enter_critical_section(dev);
            zm_debug_msg1("IBSS not found, do sitesurvey!!  bssNotFoundCount=", wd->sta.bssNotFoundCount);
            wd->sta.bssNotFoundCount++;
            zmw_leave_critical_section(dev);
            goto retry_ibss;
        }
        else
        {
            zmw_enter_critical_section(dev);
            
            wd->sta.bssNotFoundCount = 0;
            zmw_leave_critical_section(dev);
        }


        if (zfHpIsDfsChannel(dev, wd->frequency))
        {
            wd->frequency = zfHpFindFirstNonDfsChannel(dev, wd->frequency > 3000);
        }

        if( wd->ws.autoSetFrequency == 0 )
        { 
            zm_debug_msg1("Create Ad Hoc Network Band ", wd->ws.adhocMode);
            wd->frequency = zfFindCleanFrequency(dev, wd->ws.adhocMode);
            wd->ws.autoSetFrequency = 0xff;
        }
        zm_debug_msg1("IBSS not found, created one in channel ", wd->frequency);

        wd->sta.ibssBssIsCreator = 1;

        
        zfCoreSetFrequency(dev, wd->frequency);
        if (wd->sta.bDesiredBssid == TRUE)
        {
            for (k=0; k<6; k++)
            {
                bssid[k] = wd->sta.desiredBssid[k];
            }
        }
        else
        {
            #if 1
            macAddr[0] = (wd->macAddr[0] & 0xff);
            macAddr[1] = (wd->macAddr[0] >> 8);
            macAddr[2] = (wd->macAddr[1] & 0xff);
            macAddr[3] = (wd->macAddr[1] >> 8);
            macAddr[4] = (wd->macAddr[2] & 0xff);
            macAddr[5] = (wd->macAddr[2] >> 8);
            zfGenerateRandomBSSID(dev, (u8_t *)wd->macAddr, (u8_t *)bssid);
            #else
            for (k=0; k<6; k++)
            {
                bssid[k] = (u8_t) zfGetRandomNumber(dev, 0);
            }
            bssid[0] &= ~ZM_BIT_0;
            bssid[0] |= ZM_BIT_1;
            #endif
        }

        zfUpdateBssid(dev, bssid);
        

        
        if(wd->frequency <= ZM_CH_G_14)  
        {
            if ( wd->wfc.bIbssGMode
                 && (wd->supportMode & (ZM_WIRELESS_MODE_24_54|ZM_WIRELESS_MODE_24_N)) )
            {
                zfResetSupportRate(dev, ZM_DEFAULT_SUPPORT_RATE_IBSS_AG);
            }
            else
            {
                zfResetSupportRate(dev, ZM_DEFAULT_SUPPORT_RATE_IBSS_B);
            }
        } else {
            zfResetSupportRate(dev, ZM_DEFAULT_SUPPORT_RATE_IBSS_AG);
        }

        if ( wd->sta.wepStatus == ZM_ENCRYPTION_WEP_DISABLED )
        {
            wd->sta.capability[0] &= ~ZM_BIT_4;
        }
        else
        {
            wd->sta.capability[0] |= ZM_BIT_4;
        }

        wd->preambleTypeInUsed = wd->preambleType;
        if (wd->preambleTypeInUsed == ZM_PREAMBLE_TYPE_LONG)
        {
            wd->sta.capability[0] &= ~ZM_BIT_5;
        }
        else
        {
            wd->preambleTypeInUsed = ZM_PREAMBLE_TYPE_SHORT;
            wd->sta.capability[0] |= ZM_BIT_5;
        }

        zfIBSSSetupBssDesc(dev);

#ifdef ZM_ENABLE_IBSS_WPA2PSK

        
        offset = 0 ;

        
        offset += 8 ;

        
        wd->sta.beaconFrameBody[offset++] = (u8_t)(wd->beaconInterval) ;
        wd->sta.beaconFrameBody[offset++] = (u8_t)((wd->beaconInterval) >> 8) ;

        
        wd->sta.beaconFrameBody[offset++] = wd->sta.capability[0] ;
        wd->sta.beaconFrameBody[offset++] = wd->sta.capability[1] ;
        #if 0
        
        
        wd->sta.beaconFrameBody[offset++] = ZM_WLAN_EID_SSID ;
        
        wd->sta.beaconFrameBody[offset++] = wd->sta.ssidLen ;
        
        for(i=0; i<wd->sta.ssidLen; i++)
        {
            wd->sta.beaconFrameBody[offset++] = wd->sta.ssid[i] ;
        }

        
        rateSet = ZM_RATE_SET_CCK ;
        if ( (rateSet == ZM_RATE_SET_OFDM)&&((wd->gRate & 0xff) == 0) )
        {
            offset += 0 ;
        }
        else
        {
            
            wd->sta.beaconFrameBody[offset++] = ZM_WLAN_EID_SUPPORT_RATE ;

            
            lenOffset = offset++;

            
            for (i=0; i<4; i++)
            {
                if ((wd->bRate & (0x1<<i)) == (0x1<<i))
                {
                    wd->sta.beaconFrameBody[offset++] =
                	    zg11bRateTbl[i]+((wd->bRateBasic & (0x1<<i))<<(7-i)) ;
                    len++;
                }
            }

            
            wd->sta.beaconFrameBody[lenOffset] = len ;
        }

        
        
        wd->sta.beaconFrameBody[offset++] = ZM_WLAN_EID_DS ;

        
        wd->sta.beaconFrameBody[offset++] = 1 ;

        
        wd->sta.beaconFrameBody[offset++] =
         	zfChFreqToNum(wd->frequency, NULL) ;

        
        
        wd->sta.beaconFrameBody[offset++] = ZM_WLAN_EID_IBSS ;

        
        wd->sta.beaconFrameBody[offset++] = 2 ;

        
        wd->sta.beaconFrameBody[offset] = wd->sta.atimWindow ;
        offset += 2 ;

        
        if ( wd->wfc.bIbssGMode
             && (wd->supportMode & (ZM_WIRELESS_MODE_24_54|ZM_WIRELESS_MODE_24_N)) )
        {
            
            wd->erpElement = 0;
            
            wd->sta.beaconFrameBody[offset++] = ZM_WLAN_EID_ERP ;

            
            wd->sta.beaconFrameBody[offset++] = 1 ;

            
            wd->sta.beaconFrameBody[offset++] = wd->erpElement ;

            
            if ( (rateSet == ZM_RATE_SET_OFDM)&&((wd->gRate & 0xff) == 0) )
            {
                offset += 0 ;
            }
            else
            {
                len = 0 ;

                
                wd->sta.beaconFrameBody[offset++] = ZM_WLAN_EID_EXTENDED_RATE ;

                
                lenOffset = offset++ ;

                
                for (i=0; i<8; i++)
                {
                    if ((wd->gRate & (0x1<<i)) == (0x1<<i))
                    {
                        wd->sta.beaconFrameBody[offset++] =
                                     zg11gRateTbl[i]+((wd->gRateBasic & (0x1<<i))<<(7-i));
                        len++;
                    }
                }

                
            	  wd->sta.beaconFrameBody[lenOffset] = len ;
            }
        }
        #endif

        
        if ( wd->sta.authMode == ZM_AUTH_MODE_WPA2PSK )
        {
            u8_t frameType = ZM_WLAN_FRAME_TYPE_AUTH ;
            u8_t    rsn[64]=
            {
                        
                        0x30,
                        
                        0x14,
                        
                        0x01, 0x00,
                        
                        0x00, 0x0f, 0xac, 0x04,
                        
                        0x01, 0x00,
                        
                        0x00, 0x0f, 0xac, 0x02,
                        
                        0x01, 0x00,
                        
                        0x00, 0x0f, 0xac, 0x02,
                        
                        0x00, 0x00
            };

            
            zfMemoryCopy(rsn+4, zgWpa2AesOui, 4);

            if ( wd->sta.wepStatus == ZM_ENCRYPTION_AES )
            {
                
                zfMemoryCopy(rsn+10, zgWpa2AesOui, 4);
            }

            
            wd->sta.beaconFrameBody[offset++] = ZM_WLAN_EID_RSN_IE ;

            
            wd->sta.beaconFrameBody[offset++] = rsn[1] ;

            
            for(i=0; i<rsn[1]; i++)
                wd->sta.beaconFrameBody[offset++] = rsn[i+2] ;

            zfMemoryCopy(wd->sta.rsnIe, rsn, rsn[1]+2);

#ifdef ZM_ENABLE_IBSS_WPA2PSK
            
            zmw_enter_critical_section(dev);
            wd->sta.ibssWpa2Psk = 1;
            zmw_leave_critical_section(dev);
#endif
        }

        #if 0
        
        {
            u8_t OUI[3] = { 0x0 , 0x90 , 0x4C } ;

            wd->sta.beaconFrameBody[offset++] = ZM_WLAN_EID_WPA_IE ;

            wd->sta.beaconFrameBody[offset++] = wd->sta.HTCap.Data.Length + 4 ;

            for (i = 0; i < 3; i++)
            {
                wd->sta.beaconFrameBody[offset++] = OUI[i] ;
            }

            wd->sta.beaconFrameBody[offset++] = wd->sta.HTCap.Data.ElementID ;

            for (i = 0; i < 26; i++)
            {
                wd->sta.beaconFrameBody[offset++] = wd->sta.HTCap.Byte[i+2] ;
            }
        }

        
        {
            u8_t OUI[3] = { 0x0 , 0x90 , 0x4C } ;

            wd->sta.beaconFrameBody[offset++] = ZM_WLAN_EID_WPA_IE ;

            wd->sta.beaconFrameBody[offset++] = wd->sta.ExtHTCap.Data.Length + 4 ;

            for (i = 0; i < 3; i++)
            {
                wd->sta.beaconFrameBody[offset++] = OUI[i] ;
            }

            wd->sta.beaconFrameBody[offset++] = wd->sta.ExtHTCap.Data.ElementID ;

            for (i = 0; i < 22; i++)
            {
                wd->sta.beaconFrameBody[offset++] = wd->sta.ExtHTCap.Byte[i+2] ;
            }
        }
        #endif

        wd->sta.beaconFrameBodySize = offset ;

        if (wd->sta.beaconFrameBodySize > ZM_CACHED_FRAMEBODY_SIZE)
        {
            wd->sta.beaconFrameBodySize = ZM_CACHED_FRAMEBODY_SIZE;
        }

        
        

        printk("The capability info 1 = %02x\n", wd->sta.capability[0]) ;
        printk("The capability info 2 = %02x\n", wd->sta.capability[1]) ;
        for(k=0; k<wd->sta.beaconFrameBodySize; k++)
        {
	     printk("%02x ", wd->sta.beaconFrameBody[k]) ;
        }
        #if 0
        zmw_enter_critical_section(dev);
        zfMemoryCopy(event.bssid, (u8_t *)bssid, 6);
        zfMemoryCopy(event.peerMacAddr, (u8_t *)wd->macAddr, 6);
        zmw_leave_critical_section(dev);
        #endif
#endif

        
        
        
    }
    else
    {
        wd->sta.ibssBssIsCreator = 0;
    }

connect_done:
    zfHpEnableBeacon(dev, ZM_MODE_IBSS, wd->beaconInterval, wd->dtim, (u8_t)wd->sta.atimWindow);
    zfStaSendBeacon(dev); 
    zfHpSetAtimWindow(dev, wd->sta.atimWindow);

    
    zmw_enter_critical_section(dev);
    zfTimerSchedule(dev, ZM_EVENT_IBSS_MONITOR, ZM_TICK_IBSS_MONITOR);
    zmw_leave_critical_section(dev);


    if (wd->zfcbConnectNotify != NULL)
    {
        wd->zfcbConnectNotify(dev, ZM_STATUS_MEDIA_CONNECT, wd->sta.bssid);
    }
    zfChangeAdapterState(dev, ZM_STA_STATE_CONNECTED);
    wd->sta.connPowerInHalfDbm = zfHpGetTransmitPower(dev);

#ifdef ZM_ENABLE_IBSS_DELAYED_JOIN_INDICATION
    if ( !bssNotFound )
    {
        wd->sta.ibssDelayedInd = 1;
        zfMemoryCopy((u8_t *)&wd->sta.ibssDelayedIndEvent, (u8_t *)&event, sizeof(struct zsPartnerNotifyEvent));
    }
#else
    if ( !bssNotFound )
    {
        if (wd->zfcbIbssPartnerNotify != NULL)
        {
            wd->zfcbIbssPartnerNotify(dev, 1, &event);
        }
    }
#endif

    return;

retry_ibss:
    zfChangeAdapterState(dev, ZM_STA_STATE_CONNECTING);
    zfStaConnectFail(dev, ZM_STATUS_MEDIA_DISCONNECT_NOT_FOUND, wd->sta.bssid, 0);
    return;
}

void zfStaProcessAtim(zdev_t* dev, zbuf_t* buf)
{
    zmw_get_wlan_dev(dev);

    zm_debug_msg0("Receiving Atim window notification");

    wd->sta.recvAtim = 1;
}

static struct zsBssInfo* zfInfraFindAPToConnect(zdev_t* dev,
        struct zsBssInfo* candidateBss)
{
    struct zsBssInfo* pBssInfo;
    struct zsBssInfo* pNowBssInfo=NULL;
    u16_t i;
    u16_t ret, apWepStatus;
    u32_t k;
    u32_t channelFlags;

    zmw_get_wlan_dev(dev);
    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);

    pBssInfo = wd->sta.bssList.head;

    for(i=0; i<wd->sta.bssList.bssCount; i++)
    {
        if ( pBssInfo->capability[0] & ZM_BIT_4 )
        {
            apWepStatus = ZM_ENCRYPTION_WEP_ENABLED;
        }
        else
        {
            apWepStatus = ZM_ENCRYPTION_WEP_DISABLED;
        }

        if ( ((zfMemoryIsEqual(&(pBssInfo->ssid[2]), wd->sta.ssid,
                               wd->sta.ssidLen))&&
              (wd->sta.ssidLen == pBssInfo->ssid[1]))||
             ((wd->sta.ssidLen == 0)&&
               
              (wd->sta.wepStatus == apWepStatus )&&
              (pBssInfo->securityType != ZM_SECURITY_TYPE_WPA) ))
        {
            if ( wd->sta.ssidLen == 0 )
            {
                zm_debug_msg0("ANY BSS found");
            }

            if ( ((wd->sta.wepStatus == ZM_ENCRYPTION_WEP_DISABLED && apWepStatus == ZM_ENCRYPTION_WEP_ENABLED) ||
                 (wd->sta.wepStatus == ZM_ENCRYPTION_WEP_ENABLED &&
                 (apWepStatus == ZM_ENCRYPTION_WEP_DISABLED && wd->sta.dropUnencryptedPkts == 1))) &&
                 (wd->sta.authMode >= ZM_AUTH_MODE_OPEN && wd->sta.authMode <= ZM_AUTH_MODE_AUTO) )
            {
                zm_debug_msg0("Privacy policy is inconsistent");
                pBssInfo = pBssInfo->next;
                continue;
            }

            
            if ( !zfCheckAuthentication(dev, pBssInfo) )
            {
                pBssInfo = pBssInfo->next;
                continue;
            }

            
            if (wd->sta.bDesiredBssid == TRUE)
            {
                for (k=0; k<6; k++)
                {
                    if (wd->sta.desiredBssid[k] != pBssInfo->bssid[k])
                    {
                        zm_msg0_mm(ZM_LV_1, "desired bssid not matched 1");
                        break;
                    }
                }

                if (k != 6)
                {
                    zm_msg0_mm(ZM_LV_1, "desired bssid not matched 2");
                    pBssInfo = pBssInfo->next;
                    continue;
                }
            }

            
            if (pBssInfo->frequency > 3000) {
                if (   (pBssInfo->EnableHT == 1)
                    || (pBssInfo->apCap & ZM_All11N_AP) ) 
                {
                    channelFlags = CHANNEL_A_HT;
                    if (pBssInfo->enableHT40 == 1) {
                        channelFlags |= CHANNEL_HT40;
                    }
                } else {
                    channelFlags = CHANNEL_A;
                }
            } else {
                if (   (pBssInfo->EnableHT == 1)
                    || (pBssInfo->apCap & ZM_All11N_AP) ) 
                {
                    channelFlags = CHANNEL_G_HT;
                    if(pBssInfo->enableHT40 == 1) {
                        channelFlags |= CHANNEL_HT40;
                    }
                } else {
                    if (pBssInfo->extSupportedRates[1] == 0) {
                        channelFlags = CHANNEL_B;
                    } else {
                        channelFlags = CHANNEL_G;
                    }
                }
            }

            if (   ((channelFlags == CHANNEL_B) && (wd->connectMode & ZM_BIT_0))
                || ((channelFlags == CHANNEL_G) && (wd->connectMode & ZM_BIT_1))
                || ((channelFlags == CHANNEL_A) && (wd->connectMode & ZM_BIT_2))
                || ((channelFlags & CHANNEL_HT20) && (wd->connectMode & ZM_BIT_3)) )
            {
                pBssInfo = pBssInfo->next;
                continue;
            }

            
            if ((ret = zfStaIsApInBlockingList(dev, pBssInfo->bssid)) == TRUE)
            {
                zm_msg0_mm(ZM_LV_0, "Candidate AP in blocking List, skip if there's stilla choice!");
                pNowBssInfo = pBssInfo;
                pBssInfo = pBssInfo->next;
                continue;
            }

            if ( pBssInfo->capability[0] & ZM_BIT_0 )  
            {
                    pNowBssInfo = pBssInfo;
                    wd->sta.apWmeCapability = pBssInfo->wmeSupport;


                    goto done;
            }
        }

        pBssInfo = pBssInfo->next;
    }

done:
    if (pNowBssInfo != NULL)
    {
        zfwMemoryCopy((void*)candidateBss, (void*)pNowBssInfo, sizeof(struct zsBssInfo));
        pNowBssInfo = candidateBss;
    }

    zmw_leave_critical_section(dev);

    return pNowBssInfo;
}


void zfInfraConnectNetwork(zdev_t* dev)
{
    struct zsBssInfo* pBssInfo;
    struct zsBssInfo* pNowBssInfo=NULL;
    struct zsBssInfo candidateBss;
    
    
    u8_t ret=FALSE;
    u16_t k;
    u8_t density = ZM_MPDU_DENSITY_NONE;

    zmw_get_wlan_dev(dev);
    zmw_declare_for_critical_section();

    
    
    zmw_enter_critical_section(dev);
    wd->sta.bssNotFoundCount = 0;
    zmw_leave_critical_section(dev);

    
    zfUpdateDefaultQosParameter(dev, 0);

    zfStaRefreshBlockList(dev, 0);

    
    zfChangeAdapterState(dev, ZM_STA_STATE_CONNECTING);
    zfPowerSavingMgrWakeup(dev);

    wd->sta.wmeConnected = 0;
    wd->sta.psMgr.tempWakeUp = 0;
    wd->sta.qosInfo = 0;
    zfQueueFlush(dev, wd->sta.uapsdQ);

    wd->sta.connectState = ZM_STA_CONN_STATE_NONE;

    
    zfBssInfoReorderList(dev);

    pNowBssInfo = zfInfraFindAPToConnect(dev, &candidateBss);

	if (wd->sta.SWEncryptEnable != 0)
	{
	    if (wd->sta.bSafeMode == 0)
	    {
		    zfStaDisableSWEncryption(dev);
	    }
	}
    if ( pNowBssInfo != NULL )
    {
        

        pBssInfo = pNowBssInfo;
        wd->sta.ssidLen = pBssInfo->ssid[1];
        zfMemoryCopy(wd->sta.ssid, &(pBssInfo->ssid[2]), pBssInfo->ssid[1]);
        wd->frequency = pBssInfo->frequency;
        

        
        zfUpdateBssid(dev, pBssInfo->bssid);
        zfResetSupportRate(dev, ZM_DEFAULT_SUPPORT_RATE_ZERO);
        zfUpdateSupportRate(dev, pBssInfo->supportedRates);
        zfUpdateSupportRate(dev, pBssInfo->extSupportedRates);

        wd->beaconInterval = pBssInfo->beaconInterval[0] +
                             (((u16_t) pBssInfo->beaconInterval[1]) << 8);
        if (wd->beaconInterval == 0)
        {
            wd->beaconInterval = 100;
        }

        
        wd->sta.capability[0] |= ZM_BIT_0;
        
        wd->sta.capability[0] &= ~ZM_BIT_1;

        
        wd->sta.EnableHT = pBssInfo->EnableHT;
        wd->sta.SG40 = pBssInfo->SG40;
#ifdef ZM_ENABLE_CENC
        if ( pBssInfo->securityType == ZM_SECURITY_TYPE_CENC )
        {
            wd->sta.wmeEnabled = 0; 
            cencInit(dev);
            cencSetCENCMode(dev, NdisCENC_PSK);
            wd->sta.wpaState = ZM_STA_WPA_STATE_INIT;
            
            if ( pBssInfo->cencIe[1] != 0 )
            {
                
                
                zfwCencHandleBeaconProbrespon(dev, (u8_t *)&pBssInfo->cencIe,
                        (u8_t *)&pBssInfo->ssid, (u8_t *)&pBssInfo->macaddr);
                zfMemoryCopy(wd->sta.cencIe, pBssInfo->cencIe,
                        pBssInfo->cencIe[1]+2);
            }
            else
            {
                wd->sta.cencIe[1] = 0;
            }
        }
#endif 
        if ( pBssInfo->securityType == ZM_SECURITY_TYPE_WPA )
        {
            wd->sta.wpaState = ZM_STA_WPA_STATE_INIT;

            if ( wd->sta.wepStatus == ZM_ENCRYPTION_TKIP )
            {
                wd->sta.encryMode = ZM_TKIP;

                
                if (wd->sta.EnableHT == 1)
                {
                    zfStaEnableSWEncryption(dev, (ZM_SW_TKIP_ENCRY_EN|ZM_SW_TKIP_DECRY_EN));
                }

                
                
                
            }
            else if ( wd->sta.wepStatus == ZM_ENCRYPTION_AES )
            {
                wd->sta.encryMode = ZM_AES;

                
                if (wd->sta.EnableHT)
                {
                    
                    density = ZM_MPDU_DENSITY_8US;
                }
            }

            if ( pBssInfo->wpaIe[1] != 0 )
            {
                zfMemoryCopy(wd->sta.wpaIe, pBssInfo->wpaIe,
                             pBssInfo->wpaIe[1]+2);
            }
            else
            {
                wd->sta.wpaIe[1] = 0;
            }

            if ( pBssInfo->rsnIe[1] != 0 )
            {
                zfMemoryCopy(wd->sta.rsnIe, pBssInfo->rsnIe,
                             pBssInfo->rsnIe[1]+2);
            }
            else
            {
                wd->sta.rsnIe[1] = 0;
            }
        }



        
        wd->preambleTypeInUsed = wd->preambleType;
        if ( wd->preambleTypeInUsed == ZM_PREAMBLE_TYPE_AUTO )
        {
            if (pBssInfo->capability[0] & ZM_BIT_5)
            {
                wd->preambleTypeInUsed = ZM_PREAMBLE_TYPE_SHORT;
            }
            else
            {
                wd->preambleTypeInUsed = ZM_PREAMBLE_TYPE_LONG;
            }
        }

        if (wd->preambleTypeInUsed == ZM_PREAMBLE_TYPE_LONG)
        {
            wd->sta.capability[0] &= ~ZM_BIT_5;
        }
        else
        {
            wd->sta.capability[0] |= ZM_BIT_5;
        }

        
        if ((pBssInfo->enableHT40 == 1) &&
            ((pBssInfo->extChOffset == 1) || (pBssInfo->extChOffset == 3)))
        {
            wd->BandWidth40 = pBssInfo->enableHT40;
            wd->ExtOffset = pBssInfo->extChOffset;
        }
        else
        {
            wd->BandWidth40 = 0;
            wd->ExtOffset = 0;
        }

        

        
        if ( pBssInfo->athOwlAp & ZM_BIT_0 )
        {
            
            zfHpDisableHwRetry(dev);
            wd->sta.athOwlAp = 1;
            
            density = ZM_MPDU_DENSITY_8US;
        }
        else
        {
            
            zfHpEnableHwRetry(dev);
            wd->sta.athOwlAp = 0;
        }
        wd->reorder = 1;

        
        zfHpSetMPDUDensity(dev, density);

        
        if ( pBssInfo->capability[1] & ZM_BIT_2 )
        {
            wd->sta.capability[1] |= ZM_BIT_2;
        }

        if ( pBssInfo->erp & ZM_BIT_1 )
        {
            
            wd->sta.bProtectionMode = TRUE;
            zfHpSetSlotTime(dev, 0);
        }
        else
        {
            
            wd->sta.bProtectionMode = FALSE;
            zfHpSetSlotTime(dev, 1);
        }

        if (pBssInfo->marvelAp == 1)
        {
            wd->sta.enableDrvBA = 0;
            
            zfHpSetSlotTimeRegister(dev, 0);
        }
        else
        {
            wd->sta.enableDrvBA = 1;

            
            zfHpSetSlotTimeRegister(dev, 1);
        }

        
        wd->sta.beaconFrameBodySize = pBssInfo->frameBodysize + 12;
        if (wd->sta.beaconFrameBodySize > ZM_CACHED_FRAMEBODY_SIZE)
        {
            wd->sta.beaconFrameBodySize = ZM_CACHED_FRAMEBODY_SIZE;
        }
        for (k=0; k<8; k++)
        {
            wd->sta.beaconFrameBody[k] = pBssInfo->timeStamp[k];
        }
        wd->sta.beaconFrameBody[8] = pBssInfo->beaconInterval[0];
        wd->sta.beaconFrameBody[9] = pBssInfo->beaconInterval[1];
        wd->sta.beaconFrameBody[10] = pBssInfo->capability[0];
        wd->sta.beaconFrameBody[11] = pBssInfo->capability[1];
        for (k=0; k<(wd->sta.beaconFrameBodySize - 12); k++)
        {
            wd->sta.beaconFrameBody[k+12] = pBssInfo->frameBody[k];
        }

        if ( ( pBssInfo->capability[0] & ZM_BIT_4 )&&
             (( wd->sta.authMode == ZM_AUTH_MODE_OPEN )||
              ( wd->sta.authMode == ZM_AUTH_MODE_SHARED_KEY)||
              (wd->sta.authMode == ZM_AUTH_MODE_AUTO)) )
        {   

            if ( wd->sta.wepStatus == ZM_ENCRYPTION_WEP_DISABLED )
            {
                zm_debug_msg0("Adapter is no WEP, try to connect to WEP AP");
                ret = FALSE;
            }

            
            if ( wd->sta.wepStatus == ZM_ENCRYPTION_WEP_ENABLED )
            {
                
                if (wd->sta.EnableHT == 1)
                {
                    zfStaEnableSWEncryption(dev, (ZM_SW_WEP_ENCRY_EN|ZM_SW_WEP_DECRY_EN));
                }

                
                
                
            }

            wd->sta.capability[0] |= ZM_BIT_4;

            if ( wd->sta.authMode == ZM_AUTH_MODE_AUTO )
            { 
                if ( (wd->sta.connectTimeoutCount % 2) == 0 )
                    wd->sta.bIsSharedKey = 0;
                else
                    wd->sta.bIsSharedKey = 1;
            }
            else if ( wd->sta.authMode != ZM_AUTH_MODE_SHARED_KEY )
            {   
                
                wd->sta.bIsSharedKey = 0;
            }
            else if ( wd->sta.authMode != ZM_AUTH_MODE_OPEN )
            {   
                
                wd->sta.bIsSharedKey = 1;
            }
        }
        else
        {
            if ( (pBssInfo->securityType == ZM_SECURITY_TYPE_WPA)||
                 (pBssInfo->capability[0] & ZM_BIT_4) )
            {
                wd->sta.capability[0] |= ZM_BIT_4;
                
            }
            else
            {
                wd->sta.capability[0] &= (~ZM_BIT_4);
            }

            
            
            wd->sta.bIsSharedKey = 0;
        }

        
        
    }
    else
    {
        zm_debug_msg0("Desired SSID not found");
        goto zlConnectFailed;
    }


    zfCoreSetFrequencyV2(dev, wd->frequency, zfStaStartConnectCb);
    return;

zlConnectFailed:
    zfStaConnectFail(dev, ZM_STATUS_MEDIA_DISCONNECT_NOT_FOUND, wd->sta.bssid, 0);
    return;
}

u8_t zfCheckWPAAuth(zdev_t* dev, struct zsBssInfo* pBssInfo)
{
    u8_t   ret=TRUE;
    u8_t   pmkCount;
    u8_t   i;
    u16_t   encAlgoType = 0;

    zmw_get_wlan_dev(dev);

    if ( wd->sta.wepStatus == ZM_ENCRYPTION_TKIP )
    {
        encAlgoType = ZM_TKIP;
    }
    else if ( wd->sta.wepStatus == ZM_ENCRYPTION_AES )
    {
        encAlgoType = ZM_AES;
    }

    switch(wd->sta.authMode)
    {
        case ZM_AUTH_MODE_WPA:
        case ZM_AUTH_MODE_WPAPSK:
            if ( pBssInfo->wpaIe[1] == 0 )
            {
                ret = FALSE;
                break;
            }

            pmkCount = pBssInfo->wpaIe[12];
            for(i=0; i < pmkCount; i++)
            {
                if ( pBssInfo->wpaIe[17 + 4*i] == encAlgoType )
                {
                    ret = TRUE;
                    goto done;
                }
            }

            ret = FALSE;
            break;

        case ZM_AUTH_MODE_WPA2:
        case ZM_AUTH_MODE_WPA2PSK:
            if ( pBssInfo->rsnIe[1] == 0 )
            {
                ret = FALSE;
                break;
            }

            pmkCount = pBssInfo->rsnIe[8];
            for(i=0; i < pmkCount; i++)
            {
                if ( pBssInfo->rsnIe[13 + 4*i] == encAlgoType )
                {
                    ret = TRUE;
                    goto done;
                }
            }

            ret = FALSE;
            break;
    }

done:
    return ret;
}

u8_t zfCheckAuthentication(zdev_t* dev, struct zsBssInfo* pBssInfo)
{
    u8_t   ret=TRUE;
    u16_t  encAlgoType;
    u16_t UnicastCipherNum;

    zmw_get_wlan_dev(dev);

    
    if ( wd->sta.ssidLen == 0 )
    {
        return ret;
    }


	switch(wd->sta.authMode)
	
    {
        case ZM_AUTH_MODE_WPA_AUTO:
        case ZM_AUTH_MODE_WPAPSK_AUTO:
            encAlgoType = 0;
            if(pBssInfo->rsnIe[1] != 0)
            {
                UnicastCipherNum = (pBssInfo->rsnIe[8]) +
                                   (pBssInfo->rsnIe[9] << 8);

                
                if (UnicastCipherNum == 1)
                {
                    encAlgoType = pBssInfo->rsnIe[13];
                    
                }
                else
                {
                    u16_t ii;
                    u16_t desiredCipher = 0;
                    u16_t IEOffSet = 13;

                    
                    for (ii = 0; ii < UnicastCipherNum; ii++)
                    {
                        if (pBssInfo->rsnIe[IEOffSet+ii*4] > desiredCipher)
                        {
                            desiredCipher = pBssInfo->rsnIe[IEOffSet+ii*4];
                        }
                    }

                    encAlgoType = desiredCipher;
                }

                if ( encAlgoType == 0x02 )
                {
    			    wd->sta.wepStatus = ZM_ENCRYPTION_TKIP;

    			    if ( wd->sta.authMode == ZM_AUTH_MODE_WPA_AUTO )
                    {
                        wd->sta.currentAuthMode = ZM_AUTH_MODE_WPA2;
                    }
                    else 
                    {
                        wd->sta.currentAuthMode = ZM_AUTH_MODE_WPA2PSK;
                    }
                }
                else if ( encAlgoType == 0x04 )
                {
                    wd->sta.wepStatus = ZM_ENCRYPTION_AES;

                    if ( wd->sta.authMode == ZM_AUTH_MODE_WPA_AUTO )
                    {
                        wd->sta.currentAuthMode = ZM_AUTH_MODE_WPA2;
                    }
                    else 
                    {
                        wd->sta.currentAuthMode = ZM_AUTH_MODE_WPA2PSK;
                    }
                }
                else
                {
                    ret = FALSE;
                }
            }
            else if(pBssInfo->wpaIe[1] != 0)
            {
                UnicastCipherNum = (pBssInfo->wpaIe[12]) +
                                   (pBssInfo->wpaIe[13] << 8);

                
                if (UnicastCipherNum == 1)
                {
                    encAlgoType = pBssInfo->wpaIe[17];
                    
                }
                else
                {
                    u16_t ii;
                    u16_t desiredCipher = 0;
                    u16_t IEOffSet = 17;

                    
                    for (ii = 0; ii < UnicastCipherNum; ii++)
                    {
                        if (pBssInfo->wpaIe[IEOffSet+ii*4] > desiredCipher)
                        {
                            desiredCipher = pBssInfo->wpaIe[IEOffSet+ii*4];
                        }
                    }

                    encAlgoType = desiredCipher;
                }

                if ( encAlgoType == 0x02 )
                {
    			    wd->sta.wepStatus = ZM_ENCRYPTION_TKIP;

    			    if ( wd->sta.authMode == ZM_AUTH_MODE_WPA_AUTO )
                    {
                        wd->sta.currentAuthMode = ZM_AUTH_MODE_WPA;
                    }
                    else 
                    {
                        wd->sta.currentAuthMode = ZM_AUTH_MODE_WPAPSK;
                    }
                }
                else if ( encAlgoType == 0x04 )
                {
                    wd->sta.wepStatus = ZM_ENCRYPTION_AES;

                    if ( wd->sta.authMode == ZM_AUTH_MODE_WPA_AUTO )
                    {
                        wd->sta.currentAuthMode = ZM_AUTH_MODE_WPA;
                    }
                    else 
                    {
                        wd->sta.currentAuthMode = ZM_AUTH_MODE_WPAPSK;
                    }
                }
                else
                {
                    ret = FALSE;
                }


            }
            else
            {
                ret = FALSE;
            }

            break;

        case ZM_AUTH_MODE_WPA:
        case ZM_AUTH_MODE_WPAPSK:
        case ZM_AUTH_MODE_WPA_NONE:
        case ZM_AUTH_MODE_WPA2:
        case ZM_AUTH_MODE_WPA2PSK:
            {
                if ( pBssInfo->securityType != ZM_SECURITY_TYPE_WPA )
                {
                    ret = FALSE;
                }

                ret = zfCheckWPAAuth(dev, pBssInfo);
            }
            break;

        case ZM_AUTH_MODE_OPEN:
        case ZM_AUTH_MODE_SHARED_KEY:
        case ZM_AUTH_MODE_AUTO:
            {
                if ( pBssInfo->wscIe[1] )
                {
                    
                    break;
                }
                else if ( pBssInfo->securityType == ZM_SECURITY_TYPE_WPA )
                {
                    ret = FALSE;
                }
            }
            break;

        default:
            break;
    }

    return ret;
}

u8_t zfStaIsConnected(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    if ( wd->sta.adapterState == ZM_STA_STATE_CONNECTED )
    {
        return TRUE;
    }

    return FALSE;
}

u8_t zfStaIsConnecting(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    if ( wd->sta.adapterState == ZM_STA_STATE_CONNECTING )
    {
        return TRUE;
    }

    return FALSE;
}

u8_t zfStaIsDisconnect(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    if ( wd->sta.adapterState == ZM_STA_STATE_DISCONNECT )
    {
        return TRUE;
    }

    return FALSE;
}

u8_t zfChangeAdapterState(zdev_t* dev, u8_t newState)
{
    u8_t ret = TRUE;

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    
    
    
    

    switch(newState)
    {
    case ZM_STA_STATE_DISCONNECT:
        zfResetSupportRate(dev, ZM_DEFAULT_SUPPORT_RATE_DISCONNECT);

        #if 1
            zfScanMgrScanStop(dev, ZM_SCAN_MGR_SCAN_INTERNAL);
        #else
            if ( wd->sta.bChannelScan )
            {
                
                wd->sta.bChannelScan = FALSE;
                ret =  TRUE;
                break;
            }
        #endif

        break;
    case ZM_STA_STATE_CONNECTING:
        #if 1
            zfScanMgrScanStop(dev, ZM_SCAN_MGR_SCAN_INTERNAL);
        #else
            if ( wd->sta.bChannelScan )
            {
                
                wd->sta.bChannelScan = FALSE;
                ret =  TRUE;
                break;
            }
        #endif

        break;
    case ZM_STA_STATE_CONNECTED:
        break;
    default:
        break;
    }

    
    
        zmw_enter_critical_section(dev);
        wd->sta.adapterState = newState;
        zmw_leave_critical_section(dev);

        zm_debug_msg1("change adapter state = ", newState);
    

    return ret;
}


















u16_t zfStaAddIeSsid(zdev_t* dev, zbuf_t* buf, u16_t offset)
{
    u16_t i;

    zmw_get_wlan_dev(dev);

    
    zmw_tx_buf_writeb(dev, buf, offset++, ZM_WLAN_EID_SSID);

    
    zmw_tx_buf_writeb(dev, buf, offset++, wd->sta.ssidLen);

    
    for (i=0; i<wd->sta.ssidLen; i++)
    {
        zmw_tx_buf_writeb(dev, buf, offset++, wd->sta.ssid[i]);
    }

    return offset;
}


















u16_t zfStaAddIeWpaRsn(zdev_t* dev, zbuf_t* buf, u16_t offset, u8_t frameType)
{
    u32_t  i;
    u8_t    ssn[64]={
                        
                        0xdd,
                        
                        0x18,
                        
                        0x00, 0x50, 0xf2, 0x01,
                        
                        0x01, 0x00,
                        
                        0x00, 0x50, 0xf2, 0x02,
                        
                        0x01, 0x00,
                        
                        0x00, 0x50, 0xf2, 0x02,
                        
                        0x01, 0x00,
                        
                        0x00, 0x50, 0xf2, 0x02,
                        
                        0x00, 0x00
                    };

    u8_t    rsn[64]={
                        
                        0x30,
                        
                        0x14,
                        
                        0x01, 0x00,
                        
                        0x00, 0x0f, 0xac, 0x02,
                        
                        0x01, 0x00,
                        
                        0x00, 0x0f, 0xac, 0x02,
                        
                        0x01, 0x00,
                        
                        0x00, 0x0f, 0xac, 0x02,
                        
                        0x00, 0x00
                    };

    zmw_get_wlan_dev(dev);

    if ( wd->sta.currentAuthMode == ZM_AUTH_MODE_WPAPSK )
    {
        
        zfMemoryCopy(ssn+8, wd->sta.wpaIe+8, 4);

        if ( wd->sta.wepStatus == ZM_ENCRYPTION_AES )
        {
            
            zfMemoryCopy(ssn+14, zgWpaAesOui, 4);
        }

        zfCopyToIntTxBuffer(dev, buf, ssn, offset, ssn[1]+2);
        zfMemoryCopy(wd->sta.wpaIe, ssn, ssn[1]+2);
        offset += (ssn[1]+2);
    }
    else if ( wd->sta.currentAuthMode == ZM_AUTH_MODE_WPA )
    {
        
        zfMemoryCopy(ssn+8, wd->sta.wpaIe+8, 4);
        
        zfMemoryCopy(ssn+20, zgWpaRadiusOui, 4);

        if ( wd->sta.wepStatus == ZM_ENCRYPTION_AES )
        {
            
            zfMemoryCopy(ssn+14, zgWpaAesOui, 4);
        }

        zfCopyToIntTxBuffer(dev, buf, ssn, offset, ssn[1]+2);
        zfMemoryCopy(wd->sta.wpaIe, ssn, ssn[1]+2);
        offset += (ssn[1]+2);
    }
    else if ( wd->sta.currentAuthMode == ZM_AUTH_MODE_WPA2PSK )
    {
        
        zfMemoryCopy(rsn+4, wd->sta.rsnIe+4, 4);

        if ( wd->sta.wepStatus == ZM_ENCRYPTION_AES )
        {
            
            zfMemoryCopy(rsn+10, zgWpa2AesOui, 4);
        }

        if ( frameType == ZM_WLAN_FRAME_TYPE_REASOCREQ )
        {
            for(i=0; i<wd->sta.pmkidInfo.bssidCount; i++)
            {
                if ( zfMemoryIsEqual((u8_t*) wd->sta.pmkidInfo.bssidInfo[i].bssid,
                                     (u8_t*) wd->sta.bssid, 6) )
                {
                    
                    break;
                }

                if ( i < wd->sta.pmkidInfo.bssidCount )
                {
                    
                    rsn[22] = 0x01;
                    rsn[23] = 0x00;

                    
                    zfMemoryCopy(rsn+24,
                                 wd->sta.pmkidInfo.bssidInfo[i].pmkid, 16);
			                 rsn[1] += 18;
                }
            }
        }

        zfCopyToIntTxBuffer(dev, buf, rsn, offset, rsn[1]+2);
        zfMemoryCopy(wd->sta.rsnIe, rsn, rsn[1]+2);
        offset += (rsn[1]+2);
    }
    else if ( wd->sta.currentAuthMode == ZM_AUTH_MODE_WPA2 )
    {
        
        zfMemoryCopy(rsn+4, wd->sta.rsnIe+4, 4);
        
        zfMemoryCopy(rsn+16, zgWpa2RadiusOui, 4);

        if ( wd->sta.wepStatus == ZM_ENCRYPTION_AES )
        {
            
            zfMemoryCopy(rsn+10, zgWpa2AesOui, 4);
        }

        if (( frameType == ZM_WLAN_FRAME_TYPE_REASOCREQ || ( frameType == ZM_WLAN_FRAME_TYPE_ASOCREQ )))
        {

            if (wd->sta.pmkidInfo.bssidCount != 0) {
                
                rsn[22] = 1;
                rsn[23] = 0;
                
                for(i=0; i<wd->sta.pmkidInfo.bssidCount; i++)
                {
                    if ( zfMemoryIsEqual((u8_t*) wd->sta.pmkidInfo.bssidInfo[i].bssid, (u8_t*) wd->sta.bssid, 6) )
                    {
                        zfMemoryCopy(rsn+24, wd->sta.pmkidInfo.bssidInfo[i].pmkid, 16);
                        break;
                    }
                }
                rsn[1] += 18;
            }

        }

        zfCopyToIntTxBuffer(dev, buf, rsn, offset, rsn[1]+2);
        zfMemoryCopy(wd->sta.rsnIe, rsn, rsn[1]+2);
        offset += (rsn[1]+2);
    }

    return offset;
}


















u16_t zfStaAddIeIbss(zdev_t* dev, zbuf_t* buf, u16_t offset)
{
    zmw_get_wlan_dev(dev);

    
    zmw_tx_buf_writeb(dev, buf, offset++, ZM_WLAN_EID_IBSS);

    
    zmw_tx_buf_writeb(dev, buf, offset++, 2);

    
    zmw_tx_buf_writeh(dev, buf, offset, wd->sta.atimWindow);
    offset += 2;

    return offset;
}




















u16_t zfStaAddIeWmeInfo(zdev_t* dev, zbuf_t* buf, u16_t offset, u8_t qosInfo)
{
    
    zmw_tx_buf_writeb(dev, buf, offset++, ZM_WLAN_EID_WIFI_IE);

    
    zmw_tx_buf_writeb(dev, buf, offset++, 7);

    
    zmw_tx_buf_writeb(dev, buf, offset++, 0x00);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x50);
    zmw_tx_buf_writeb(dev, buf, offset++, 0xF2);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x02);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x00);
    zmw_tx_buf_writeb(dev, buf, offset++, 0x01);

    
    zmw_tx_buf_writeb(dev, buf, offset++, qosInfo);

    return offset;
}


















u16_t zfStaAddIePowerCap(zdev_t* dev, zbuf_t* buf, u16_t offset)
{
    u8_t MaxTxPower;
    u8_t MinTxPower;

    zmw_get_wlan_dev(dev);

    
    zmw_tx_buf_writeb(dev, buf, offset++, ZM_WLAN_EID_POWER_CAPABILITY);

    
    zmw_tx_buf_writeb(dev, buf, offset++, 2);

    MinTxPower = (u8_t)(zfHpGetMinTxPower(dev)/2);
    MaxTxPower = (u8_t)(zfHpGetMaxTxPower(dev)/2);

    
    zmw_tx_buf_writeh(dev, buf, offset++, MinTxPower);

    
    zmw_tx_buf_writeh(dev, buf, offset++, MaxTxPower);

    return offset;
}

















u16_t zfStaAddIeSupportCh(zdev_t* dev, zbuf_t* buf, u16_t offset)
{

    u8_t   i;
    u16_t  count_24G = 0;
    u16_t  count_5G = 0;
    u16_t  channelNum;
    u8_t   length;

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();
    zmw_enter_critical_section(dev);

    for (i = 0; i < wd->regulationTable.allowChannelCnt; i++)
    {
        if (wd->regulationTable.allowChannel[i].channel < 3000)
        { 
            count_24G++;
        }
        else
        { 
            count_5G++;
        }
    }

    length = (u8_t)(count_5G * 2 + 2); 

    
    zmw_tx_buf_writeb(dev, buf, offset++, ZM_WLAN_EID_SUPPORTED_CHANNELS );

    
    zmw_tx_buf_writeb(dev, buf, offset++, length);

    
    
    zmw_tx_buf_writeh(dev, buf, offset++, 1); 
    
    zmw_tx_buf_writeh(dev, buf, offset++, count_24G);

    for (i = 0; i < wd->regulationTable.allowChannelCnt ; i++)
    {
        if (wd->regulationTable.allowChannel[i].channel > 4000 && wd->regulationTable.allowChannel[i].channel < 5000)
        { 
            channelNum = (wd->regulationTable.allowChannel[i].channel-4000)/5;
            
            zmw_tx_buf_writeh(dev, buf, offset++, channelNum);
            
            zmw_tx_buf_writeh(dev, buf, offset++, 1);
        }
        else if (wd->regulationTable.allowChannel[i].channel >= 5000)
        { 
            channelNum = (wd->regulationTable.allowChannel[i].channel-5000)/5;
            
            zmw_tx_buf_writeh(dev, buf, offset++, channelNum);
            
            zmw_tx_buf_writeh(dev, buf, offset++, 1);
        }
    }
   zmw_leave_critical_section(dev);

    return offset;
}

void zfStaStartConnectCb(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    zfStaStartConnect(dev, wd->sta.bIsSharedKey);
}

void zfStaStartConnect(zdev_t* dev, u8_t bIsSharedKey)
{
    u32_t p1, p2;
    u8_t newConnState;

    zmw_get_wlan_dev(dev);
    zmw_declare_for_critical_section();

    
    if ( bIsSharedKey )
    {
        
        newConnState = ZM_STA_CONN_STATE_AUTH_SHARE_1;
        zm_debug_msg0("ZM_STA_CONN_STATE_AUTH_SHARE_1");
        p1 = ZM_AUTH_ALGO_SHARED_KEY;
    }
    else
    {
        
        newConnState = ZM_STA_CONN_STATE_AUTH_OPEN;
        zm_debug_msg0("ZM_STA_CONN_STATE_AUTH_OPEN");
        if( wd->sta.leapEnabled )
            p1 = ZM_AUTH_ALGO_LEAP;
        else
            p1 = ZM_AUTH_ALGO_OPEN_SYSTEM;
    }

    
    p2 = 0x0;

    zmw_enter_critical_section(dev);
    wd->sta.connectTimer = wd->tick;
    wd->sta.connectState = newConnState;
    zmw_leave_critical_section(dev);

    
    zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_AUTH, wd->sta.bssid, p1, p2, 0);

    return;
}

void zfSendNullData(zdev_t* dev, u8_t type)
{
    zbuf_t* buf;
    
    
    u16_t err;
    u16_t hlen;
    u16_t header[(34+8+1)/2];
    u16_t bcastAddr[3] = {0xffff,0xffff,0xffff};
    u16_t *dstAddr;

    zmw_get_wlan_dev(dev);

    if ((buf = zfwBufAllocate(dev, 1024)) == NULL)
    {
        zm_msg0_mm(ZM_LV_0, "Alloc mm buf Fail!");
        return;
    }

    zfwBufSetSize(dev, buf, 0);

    

    if ( wd->wlanMode == ZM_MODE_IBSS)
    {
        dstAddr = bcastAddr;
    }
    else
    {
        dstAddr = wd->sta.bssid;
    }

    if (wd->sta.wmeConnected != 0)
    {
        
        hlen = zfTxGenMmHeader(dev, ZM_WLAN_FRAME_TYPE_QOS_NULL, dstAddr, header, 0, buf, 0, 0);
    }
    else
    {
        hlen = zfTxGenMmHeader(dev, ZM_WLAN_FRAME_TYPE_NULL, dstAddr, header, 0, buf, 0, 0);
    }

    if (wd->wlanMode == ZM_MODE_INFRASTRUCTURE)
    {
        header[4] |= 0x0100; 
    }

    if ( type == 1 )
    {
        header[4] |= 0x1000;
    }

    
    
    
    
    
    

    
    wd->commTally.txUnicastFrm++;

    if ((err = zfHpSend(dev, header, hlen, NULL, 0, NULL, 0, buf, 0,
            ZM_INTERNAL_ALLOC_BUF, 0, 0xff)) != ZM_SUCCESS)
    {
        goto zlError;
    }


    return;

zlError:

    zfwBufFree(dev, buf, 0);
    return;

}

void zfSendPSPoll(zdev_t* dev)
{
    zbuf_t* buf;
    
    
    u16_t err;
    u16_t hlen;
    u16_t header[(8+24+1)/2];

    zmw_get_wlan_dev(dev);

    if ((buf = zfwBufAllocate(dev, 1024)) == NULL)
    {
        zm_msg0_mm(ZM_LV_0, "Alloc mm buf Fail!");
        return;
    }

    zfwBufSetSize(dev, buf, 0);

    

    zfTxGenMmHeader(dev, ZM_WLAN_FRAME_TYPE_PSPOLL, wd->sta.bssid, header, 0, buf, 0, 0);

    header[0] = 20;
    header[4] |= 0x1000;
    header[5] = wd->sta.aid | 0xc000; 
    hlen = 16 + 8;

    
    
    
    
    
    

    if ((err = zfHpSend(dev, header, hlen, NULL, 0, NULL, 0, buf, 0,
            ZM_INTERNAL_ALLOC_BUF, 0, 0xff)) != ZM_SUCCESS)
    {
        goto zlError;
    }

    return;

zlError:

    zfwBufFree(dev, buf, 0);
    return;

}

void zfSendBA(zdev_t* dev, u16_t start_seq, u8_t *bitmap)
{
    zbuf_t* buf;
    
    
    u16_t err;
    u16_t hlen;
    u16_t header[(8+24+1)/2];
    u16_t i, offset = 0;

    zmw_get_wlan_dev(dev);

    if ((buf = zfwBufAllocate(dev, 1024)) == NULL)
    {
        zm_msg0_mm(ZM_LV_0, "Alloc mm buf Fail!");
        return;
    }

    zfwBufSetSize(dev, buf, 12); 
                                 

    

    zfTxGenMmHeader(dev, ZM_WLAN_FRAME_TYPE_BA, wd->sta.bssid, header, 0, buf, 0, 0);

    header[0] = 32; 
    header[1] = 0x4;  

    
    header[2] = (u16_t)(zcRateToPhyCtrl[4] & 0xffff);
    header[3] = (u16_t)(zcRateToPhyCtrl[4]>>16) & 0xffff;

    hlen = 16 + 8;  
    offset = 0;
    zmw_tx_buf_writeh(dev, buf, offset, 0x05); 
    offset+=2;
    zmw_tx_buf_writeh(dev, buf, offset, start_seq);
    offset+=2;

    for (i=0; i<8; i++) {
        zmw_tx_buf_writeb(dev, buf, offset, bitmap[i]);
        offset++;
    }

    if ((err = zfHpSend(dev, header, hlen, NULL, 0, NULL, 0, buf, 0,
            ZM_INTERNAL_ALLOC_BUF, 0, 0xff)) != ZM_SUCCESS)
    {
        goto zlError;
    }

    return;

zlError:

    zfwBufFree(dev, buf, 0);
    return;

}

void zfStaGetTxRate(zdev_t* dev, u16_t* macAddr, u32_t* phyCtrl,
        u16_t* rcProbingFlag)
{
    u8_t   addr[6], i;
    u8_t   rate;
    zmw_get_wlan_dev(dev);
    zmw_declare_for_critical_section();

    ZM_MAC_WORD_TO_BYTE(macAddr, addr);
    *phyCtrl = 0;

    if ( wd->wlanMode == ZM_MODE_INFRASTRUCTURE )
    {
        zmw_enter_critical_section(dev);
        rate = (u8_t)zfRateCtrlGetTxRate(dev, &wd->sta.oppositeInfo[0].rcCell, rcProbingFlag);

        

        *phyCtrl = zcRateToPhyCtrl[rate];
        zmw_leave_critical_section(dev);
    }
    else
    {
        zmw_enter_critical_section(dev);
        for(i=0; i<wd->sta.oppositeCount; i++)
        {
            if ( addr[0] && 0x01 == 1 ) 
                                        
            {
                
                rate = wd->sta.oppositeInfo[i].rcCell.operationRateSet[0];
                *phyCtrl = zcRateToPhyCtrl[rate];
                break;
            }
            else if ( zfMemoryIsEqual(addr, wd->sta.oppositeInfo[i].macAddr, 6) )
            {
                rate = (u8_t)zfRateCtrlGetTxRate(dev, &wd->sta.oppositeInfo[i].rcCell, rcProbingFlag);
                *phyCtrl = zcRateToPhyCtrl[rate];
                break;
            }
        }
        zmw_leave_critical_section(dev);
    }

    return;
}

struct zsMicVar* zfStaGetRxMicKey(zdev_t* dev, zbuf_t* buf)
{
    u8_t keyIndex;
    u8_t da0;

    zmw_get_wlan_dev(dev);

    
    if ( ((wd->sta.encryMode != ZM_TKIP)&&(wd->sta.encryMode != ZM_AES))||
         (wd->sta.wpaState < ZM_STA_WPA_STATE_PK_OK) )
    {
        return NULL;
    }

    da0 = zmw_rx_buf_readb(dev, buf, ZM_WLAN_HEADER_A1_OFFSET);

    if ((zmw_rx_buf_readb(dev, buf, 0) & 0x80) == 0x80)
        keyIndex = zmw_rx_buf_readb(dev, buf, ZM_WLAN_HEADER_IV_OFFSET+5); 
    else
        keyIndex = zmw_rx_buf_readb(dev, buf, ZM_WLAN_HEADER_IV_OFFSET+3); 
    keyIndex = (keyIndex & 0xc0) >> 6;

    return (&wd->sta.rxMicKey[keyIndex]);
}

struct zsMicVar* zfStaGetTxMicKey(zdev_t* dev, zbuf_t* buf)
{
    zmw_get_wlan_dev(dev);

    
    
    
    if ( (wd->sta.encryMode != ZM_TKIP) || (wd->sta.wpaState < ZM_STA_WPA_STATE_PK_OK) )
    {
        return NULL;
    }

    return (&wd->sta.txMicKey);
}

u16_t zfStaRxValidateFrame(zdev_t* dev, zbuf_t* buf)
{
    u8_t   frameType, frameCtrl;
    u8_t   da0;
    
    u16_t  ret;
    u16_t  i;
    

    zmw_get_wlan_dev(dev);

    frameType = zmw_rx_buf_readb(dev, buf, 0);
    da0 = zmw_rx_buf_readb(dev, buf, ZM_WLAN_HEADER_A1_OFFSET);
    

    if ( (!zfStaIsConnected(dev))&&((frameType & 0xf) == ZM_WLAN_DATA_FRAME) )
    {
        return ZM_ERR_DATA_BEFORE_CONNECTED;
    }


    if ( (zfStaIsConnected(dev))&&((frameType & 0xf) == ZM_WLAN_DATA_FRAME) )
    {
        
        if ( wd->wlanMode == ZM_MODE_INFRASTRUCTURE )
        {
            
            u16_t mac[3];
            mac[0] = zmw_cpu_to_le16(wd->sta.bssid[0]);
            mac[1] = zmw_cpu_to_le16(wd->sta.bssid[1]);
            mac[2] = zmw_cpu_to_le16(wd->sta.bssid[2]);
            if ( !zfRxBufferEqualToStr(dev, buf, (u8_t *)mac,
                                       ZM_WLAN_HEADER_A2_OFFSET, 6) )
            {


#if 0
                
                if (( da0 & 0x01 ) == 0)
                {
                    for (i=0; i<3; i++)
                    {
                        sa[i] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A2_OFFSET+(i*2));
                    }
					
					if (( sa0 & 0x01 ) == 0)
	                  	zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_DEAUTH, sa, 7, 0, 0);
                }
#endif
                return ZM_ERR_DATA_BSSID_NOT_MATCHED;
            }
        }
        else if ( wd->wlanMode == ZM_MODE_IBSS )
        {
            
            u16_t mac[3];
            mac[0] = zmw_cpu_to_le16(wd->sta.bssid[0]);
            mac[1] = zmw_cpu_to_le16(wd->sta.bssid[1]);
            mac[2] = zmw_cpu_to_le16(wd->sta.bssid[2]);
            if ( !zfRxBufferEqualToStr(dev, buf, (u8_t *)mac,
                                       ZM_WLAN_HEADER_A3_OFFSET, 6) )
            {
                return ZM_ERR_DATA_BSSID_NOT_MATCHED;
            }
        }

        frameCtrl = zmw_rx_buf_readb(dev, buf, 1);

        
        if ( wd->sta.dropUnencryptedPkts &&
             (wd->sta.wepStatus != ZM_ENCRYPTION_WEP_DISABLED )&&
             ( !(frameCtrl & ZM_BIT_6) ) )
        {   

            #if 1
            ret = ZM_ERR_DATA_NOT_ENCRYPTED;
            if ( wd->sta.pStaRxSecurityCheckCb != NULL )
            {
                ret = wd->sta.pStaRxSecurityCheckCb(dev, buf);
            }
            else
            {
                ret = ZM_ERR_DATA_NOT_ENCRYPTED;
            }
            if (ret == ZM_ERR_DATA_NOT_ENCRYPTED)
            {
                wd->commTally.swRxDropUnencryptedCount++;
            }
            return ret;
            #else
            if ( (wd->sta.wepStatus != ZM_ENCRYPTION_TKIP)&&
                 (wd->sta.wepStatus != ZM_ENCRYPTION_AES) )
            {
                return ZM_ERR_DATA_NOT_ENCRYPTED;
            }
            #endif
        }
    }

    return ZM_SUCCESS;
}

void zfStaMicFailureHandling(zdev_t* dev, zbuf_t* buf)
{
    u8_t   da0;
    u8_t   micNotify = 1;

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    if ( wd->sta.wpaState <  ZM_STA_WPA_STATE_PK_OK )
    {
        return;
    }

    zmw_enter_critical_section(dev);

    wd->sta.cmMicFailureCount++;

    if ( wd->sta.cmMicFailureCount == 1 )
    {
        zm_debug_msg0("get the first MIC failure");
        

        
        
        zfTimerSchedule(dev, ZM_EVENT_CM_TIMER, ZM_TICK_CM_TIMEOUT - ZM_TICK_CM_TIMEOUT_OFFSET);
    }
    else if ( wd->sta.cmMicFailureCount == 2 )
    {
        zm_debug_msg0("get the second MIC failure");
        
        wd->sta.cmDisallowSsidLength = wd->sta.ssidLen;
        zfMemoryCopy(wd->sta.cmDisallowSsid, wd->sta.ssid, wd->sta.ssidLen);
        
        zfTimerCancel(dev, ZM_EVENT_CM_TIMER);
        

        
        
        zfTimerSchedule(dev, ZM_EVENT_CM_DISCONNECT, ZM_TICK_CM_DISCONNECT - ZM_TICK_CM_DISCONNECT_OFFSET);
    }
    else
    {
        micNotify = 0;
    }

    zmw_leave_critical_section(dev);

    if (micNotify == 1)
    {
        da0 = zmw_rx_buf_readb(dev, buf, ZM_WLAN_HEADER_A1_OFFSET);
        if ( da0 & 0x01 )
        {
            if (wd->zfcbMicFailureNotify != NULL)
            {
                wd->zfcbMicFailureNotify(dev, wd->sta.bssid, ZM_MIC_GROUP_ERROR);
            }
        }
        else
        {
            if (wd->zfcbMicFailureNotify != NULL)
            {
                wd->zfcbMicFailureNotify(dev, wd->sta.bssid, ZM_MIC_PAIRWISE_ERROR);
            }
        }
    }
}


u8_t zfStaBlockWlanScan(zdev_t* dev)
{
    u8_t   ret=FALSE;

    zmw_get_wlan_dev(dev);

    if ( wd->sta.bChannelScan )
    {
        return TRUE;
    }

    return ret;
}

void zfStaResetStatus(zdev_t* dev, u8_t bInit)
{
    u8_t   i;

    zmw_get_wlan_dev(dev);

    zfHpDisableBeacon(dev);

    wd->dtim = 1;
    wd->sta.capability[0] = 0x01;
    wd->sta.capability[1] = 0x00;
    
    if (wd->sta.DFSEnable || wd->sta.TPCEnable)
        wd->sta.capability[1] |= ZM_BIT_0;

    
    for(i=0; i<wd->sta.ibssPSDataCount; i++)
    {
        zfwBufFree(dev, wd->sta.ibssPSDataQueue[i], 0);
    }

    for(i=0; i<wd->sta.staPSDataCount; i++)
    {
        zfwBufFree(dev, wd->sta.staPSDataQueue[i], 0);
    }

    wd->sta.ibssPSDataCount = 0;
    wd->sta.staPSDataCount = 0;
    zfZeroMemory((u8_t*) &wd->sta.staPSList, sizeof(struct zsStaPSList));

    wd->sta.wmeConnected = 0;
    wd->sta.psMgr.tempWakeUp = 0;
    wd->sta.qosInfo = 0;
    zfQueueFlush(dev, wd->sta.uapsdQ);

    return;

}

void zfStaIbssMonitoring(zdev_t* dev, u8_t reset)
{
    u16_t i;
    u16_t oppositeCount;
    struct zsPartnerNotifyEvent event;

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    

    zmw_enter_critical_section(dev);

    if ( wd->sta.oppositeCount == 0 )
    {
        goto done;
    }

    if ( wd->sta.bChannelScan )
    {
        goto done;
    }

    oppositeCount = wd->sta.oppositeCount;

    for(i=0; i < ZM_MAX_OPPOSITE_COUNT; i++)
    {
        if ( oppositeCount == 0 )
        {
            break;
        }

        if ( reset )
        {
            wd->sta.oppositeInfo[i].valid = 0;
        }

        if ( wd->sta.oppositeInfo[i].valid == 0 )
        {
            continue;
        }

        oppositeCount--;

        if ( wd->sta.oppositeInfo[i].aliveCounter )
        {
            zm_debug_msg1("Setting alive to ", wd->sta.oppositeInfo[i].aliveCounter);

            zmw_leave_critical_section(dev);

            if ( wd->sta.oppositeInfo[i].aliveCounter != ZM_IBSS_PEER_ALIVE_COUNTER )
            {
                zfSendMmFrame(dev, ZM_WLAN_FRAME_TYPE_PROBEREQ,
                              (u16_t*)wd->sta.oppositeInfo[i].macAddr, 1, 0, 0);
            }

            zmw_enter_critical_section(dev);
            wd->sta.oppositeInfo[i].aliveCounter--;
        }
        else
        {
            zm_debug_msg0("zfStaIbssMonitoring remove the peer station");
            zfMemoryCopy(event.bssid, (u8_t *)(wd->sta.bssid), 6);
            zfMemoryCopy(event.peerMacAddr, wd->sta.oppositeInfo[i].macAddr, 6);

            wd->sta.oppositeInfo[i].valid = 0;
            wd->sta.oppositeCount--;
            if (wd->zfcbIbssPartnerNotify != NULL)
            {
                zmw_leave_critical_section(dev);
                wd->zfcbIbssPartnerNotify(dev, 0, &event);
                zmw_enter_critical_section(dev);
            }
        }
    }

done:
    if ( reset == 0 )
    {
        zfTimerSchedule(dev, ZM_EVENT_IBSS_MONITOR, ZM_TICK_IBSS_MONITOR);
    }

    zmw_leave_critical_section(dev);
}

void zfInitPartnerNotifyEvent(zdev_t* dev, zbuf_t* buf, struct zsPartnerNotifyEvent *event)
{
    u16_t  *peerMacAddr;

    zmw_get_wlan_dev(dev);

    peerMacAddr = (u16_t *)event->peerMacAddr;

    zfMemoryCopy(event->bssid, (u8_t *)(wd->sta.bssid), 6);
    peerMacAddr[0] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A2_OFFSET);
    peerMacAddr[1] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A2_OFFSET + 2);
    peerMacAddr[2] = zmw_rx_buf_readh(dev, buf, ZM_WLAN_HEADER_A2_OFFSET + 4);
}

void zfStaInitOppositeInfo(zdev_t* dev)
{
    int i;

    zmw_get_wlan_dev(dev);

    for(i=0; i<ZM_MAX_OPPOSITE_COUNT; i++)
    {
        wd->sta.oppositeInfo[i].valid = 0;
        wd->sta.oppositeInfo[i].aliveCounter = ZM_IBSS_PEER_ALIVE_COUNTER;
    }
}
#ifdef ZM_ENABLE_CENC
u16_t zfStaAddIeCenc(zdev_t* dev, zbuf_t* buf, u16_t offset)
{
    zmw_get_wlan_dev(dev);

    if (wd->sta.cencIe[1] != 0)
    {
        zfCopyToIntTxBuffer(dev, buf, wd->sta.cencIe, offset, wd->sta.cencIe[1]+2);
        offset += (wd->sta.cencIe[1]+2);
    }
    return offset;
}
#endif 
u16_t zfStaProcessAction(zdev_t* dev, zbuf_t* buf)
{
    u8_t category, actionDetails;
    zmw_get_wlan_dev(dev);

    category = zmw_rx_buf_readb(dev, buf, 24);
    actionDetails = zmw_rx_buf_readb(dev, buf, 25);
    switch (category)
    {
        case 0:		
	        switch(actionDetails)
	        {
	        	case 0:			
	        		break;
	        	case 1:			
	        		
	        		break;
	        	case 2:			
                    
                    
	        		break;
	        	case 3:			
                    
                    
	        		break;
	        	case 4:			
                    if (wd->sta.DFSEnable)
                        zfStaUpdateDot11HDFS(dev, buf);
	        		break;
	        	default:
	        		zm_debug_msg1("Action Frame contain not support action field ", actionDetails);
	        		break;
	        }
        	break;
        case ZM_WLAN_BLOCK_ACK_ACTION_FRAME:
            zfAggBlockAckActionFrame(dev, buf);
            break;
        case 17:	
        	break;
    }

    return 0;
}


void zfReWriteBeaconStartAddress(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);
    wd->tickIbssSendBeacon++;    
    zmw_leave_critical_section(dev);

    if ( wd->tickIbssSendBeacon == 40 )
    {

        zfHpEnableBeacon(dev, ZM_MODE_IBSS, wd->beaconInterval, wd->dtim, (u8_t)wd->sta.atimWindow);
        zmw_enter_critical_section(dev);
        wd->tickIbssSendBeacon = 0;
        zmw_leave_critical_section(dev);
    }
}

struct zsTkipSeed* zfStaGetRxSeed(zdev_t* dev, zbuf_t* buf)
{
    u8_t keyIndex;
    u8_t da0;

    zmw_get_wlan_dev(dev);

    
    if ( ((wd->sta.encryMode != ZM_TKIP)&&(wd->sta.encryMode != ZM_AES))||
         (wd->sta.wpaState < ZM_STA_WPA_STATE_PK_OK) )
    {
        return NULL;
    }

    da0 = zmw_rx_buf_readb(dev, buf, ZM_WLAN_HEADER_A1_OFFSET);

    if ((zmw_rx_buf_readb(dev, buf, 0) & 0x80) == 0x80)
        keyIndex = zmw_rx_buf_readb(dev, buf, ZM_WLAN_HEADER_IV_OFFSET+5); 
    else
        keyIndex = zmw_rx_buf_readb(dev, buf, ZM_WLAN_HEADER_IV_OFFSET+3); 
    keyIndex = (keyIndex & 0xc0) >> 6;

    return (&wd->sta.rxSeed[keyIndex]);
}

void zfStaEnableSWEncryption(zdev_t *dev, u8_t value)
{
    zmw_get_wlan_dev(dev);

    wd->sta.SWEncryptEnable = value;
    zfHpSWDecrypt(dev, 1);
    zfHpSWEncrypt(dev, 1);
}

void zfStaDisableSWEncryption(zdev_t *dev)
{
    zmw_get_wlan_dev(dev);

    wd->sta.SWEncryptEnable = 0;
    zfHpSWDecrypt(dev, 0);
    zfHpSWEncrypt(dev, 0);
}

u16_t zfComputeBssInfoWeightValue(zdev_t *dev, u8_t isBMode, u8_t isHT, u8_t isHT40, u8_t signalStrength)
{
	u8_t  weightOfB           = 0;
	u8_t  weightOfAGBelowThr  = 0;
	u8_t  weightOfAGUpThr     = 15;
	u8_t  weightOfN20BelowThr = 15;
	u8_t  weightOfN20UpThr    = 30;
	u8_t  weightOfN40BelowThr = 16;
	u8_t  weightOfN40UpThr    = 32;

    zmw_get_wlan_dev(dev);

    if( isBMode == 0 )
        return (signalStrength + weightOfB);    
    else
    {
        if( isHT == 0 && isHT40 == 0 )
        { 
            if( signalStrength < 18 ) 
				return signalStrength + weightOfAGBelowThr;
			else
				return (signalStrength + weightOfAGUpThr);
        }
        else if( isHT == 1 && isHT40 == 0 )
        { 
            if( signalStrength < 23 ) 
                return (signalStrength + weightOfN20BelowThr);
            else
                return (signalStrength + weightOfN20UpThr);
        }
        else 
        { 
            if( signalStrength < 16 ) 
                return (signalStrength + weightOfN40BelowThr);
            else
                return (signalStrength + weightOfN40UpThr);
        }
    }
}

u16_t zfStaAddIbssAdditionalIE(zdev_t* dev, zbuf_t* buf, u16_t offset)
{
	u16_t i;

    zmw_get_wlan_dev(dev);

    for (i=0; i<wd->sta.ibssAdditionalIESize; i++)
    {
        zmw_tx_buf_writeb(dev, buf, offset++, wd->sta.ibssAdditionalIE[i]);
    }

    return offset;
}
