










#include "cprecomp.h"
#include "../hal/hpreg.h"

extern const u8_t zcUpToAc[8];

u16_t zcIndextoRateBG[16] = {1000, 2000, 5500, 11000, 0, 0, 0, 0, 48000,
                               24000, 12000, 6000, 54000, 36000, 18000, 9000};
u32_t zcIndextoRateN20L[16] = {6500, 13000, 19500, 26000, 39000, 52000, 58500,
                              65000, 13000, 26000, 39000, 52000, 78000, 104000,
                              117000, 130000};
u32_t zcIndextoRateN20S[16] = {7200, 14400, 21700, 28900, 43300, 57800, 65000,
                              72200, 14400, 28900, 43300, 57800, 86700, 115600,
                              130000, 144400};
u32_t zcIndextoRateN40L[16] = {13500, 27000, 40500, 54000, 81000, 108000, 121500,
                              135000, 27000, 54000, 81000, 108000, 162000, 216000,
                              243000, 270000};
u32_t zcIndextoRateN40S[16] = {15000, 30000, 45000, 60000, 90000, 120000, 135000,
                              150000, 30000, 60000, 90000, 120000, 180000, 240000,
                              270000, 300000};



















u16_t zfTxGenWlanHeader(zdev_t* dev, zbuf_t* buf, u16_t* header, u16_t seq,
                        u8_t flag, u16_t plusLen, u16_t minusLen, u16_t port,
                        u16_t* da, u16_t* sa, u8_t up, u16_t *micLen,
                        u16_t* snap, u16_t snapLen, struct aggControl *aggControl)
{

    u16_t len;
    u16_t macCtrl;
    u32_t phyCtrl;
    u16_t hlen = 16;
    u16_t icvLen = 0;
    u16_t wdsPortId;
    u16_t vap = 0;
    u16_t mcs = 0;
    u16_t mt = 0;
    u8_t  qosType;
    u8_t  b1, b2;
    u16_t wdsPort;
    u8_t  encExemptionActionType;
    u16_t rateProbingFlag = 0;
    u8_t  tkipFrameOffset = 0;

#ifdef ZM_ENABLE_IBSS_WPA2PSK
    u8_t    res, peerIdx;
    u8_t    userIdx=0;
    u16_t   *iv16;
    u32_t   *iv32;
#endif

    zmw_get_wlan_dev(dev);

   
    
    header[4] = 0x0008 | (flag<<8);
    
    header[5] = 0x0000;

    if (wd->wlanMode == ZM_MODE_INFRASTRUCTURE)
    {
        
        header[4] |= 0x0100;

        
        if ( zfPowerSavingMgrIsSleeping(dev) || wd->sta.psMgr.tempWakeUp == 1 )
        {
            header[4] |= 0x1000;
        }

        
        header[6] = wd->sta.bssid[0];
        header[7] = wd->sta.bssid[1];
        header[8] = wd->sta.bssid[2];
        
        header[12] = da[0];
        header[13] = da[1];
        header[14] = da[2];
    }
    else if (wd->wlanMode == ZM_MODE_PSEUDO)
    {
        
        header[6] = da[0];
        header[7] = da[1];
        header[8] = da[2];
        
        header[12] = 0;
        header[13] = 0;
        header[14] = 0;

        
        if (wd->enableWDS)
        {
            
            header[4] |= 0x0300;

            
            header[16] = 0;
            header[17] = 0;
            header[18] = 0;

            hlen = 19;
        }
    }
    else if (wd->wlanMode == ZM_MODE_IBSS)
    {
        
        header[6] = da[0];
        header[7] = da[1];
        header[8] = da[2];
        
        header[12] = wd->sta.bssid[0];
        header[13] = wd->sta.bssid[1];
        header[14] = wd->sta.bssid[2];

#ifdef ZM_ENABLE_IBSS_WPA2PSK
        zmw_enter_critical_section(dev);
        res = zfStaFindOppositeByMACAddr(dev, da, &peerIdx);
        if(res == 0) 
        {
            userIdx = peerIdx;
        }
        zmw_leave_critical_section(dev);
#endif
    }
    else if (wd->wlanMode == ZM_MODE_AP)
    {
        if (port < 0x20)
        
        {
            
            header[4] |= 0x0200;

            
            header[6] = da[0];
            header[7] = da[1];
            header[8] = da[2];
            
            header[12] = sa[0];
            header[13] = sa[1];
            header[14] = sa[2];

            if (port < ZM_MAX_AP_SUPPORT)
            {
                vap = port;
                header[14] += (vap<<8);
            }
        }
        else
        
        {
            
            header[4] |= 0x0300;

            wdsPortId = port - 0x20;

            
            header[6] = wd->ap.wds.macAddr[wdsPortId][0];
            header[7] = wd->ap.wds.macAddr[wdsPortId][1];
            header[8] = wd->ap.wds.macAddr[wdsPortId][2];
            
            header[12] = da[0];
            header[13] = da[1];
            header[14] = da[2];
            
            header[16] = sa[0];
            header[17] = sa[1];
            header[18] = sa[2];

            hlen = 19;
        }
    } 

    
    header[9] = wd->macAddr[0];
    header[10] = wd->macAddr[1];
#ifdef ZM_VAPMODE_MULTILE_SSID
    header[11] = wd->macAddr[2]; 
#else
    header[11] = wd->macAddr[2] + (vap<<8); 
#endif

    if ( (wd->wlanMode == ZM_MODE_IBSS) && (wd->XLinkMode) )
    {
        header[9]  = sa[0];
        header[10] = sa[1];
        header[11] = sa[2];
    }

    
    header[15] = seq;


    if (wd->wlanMode == ZM_MODE_AP)
    {
        zfApGetStaTxRateAndQosType(dev, da, &phyCtrl, &qosType, &rateProbingFlag);
        mt = (u16_t)(phyCtrl & 0x3);
        mcs = (u16_t)((phyCtrl >> 16) & 0x3f);
#if 1
        

        
        if (qosType == 1)
        {
            
            header[4] |= 0x0080;

            
            header[hlen] = up;
            hlen += 1;
        }
#endif
    }

#if 0
    
    if (header[6] == 0x8000)
    {
        
        header[4] |= 0x0080;

        
        header[hlen] = 0;
        hlen += 1;
    }
#endif

    if (wd->wlanMode == ZM_MODE_AP) {
        
    }
    else {
        
        zfStaGetTxRate(dev, da, &phyCtrl, &rateProbingFlag);
        mt = (u16_t)(phyCtrl & 0x3);
        mcs = (u16_t)((phyCtrl >> 16) & 0x3f);
    }

    if (wd->txMCS != 0xff)
    {
        
	    phyCtrl = ((u32_t)wd->txMCS<<16) + wd->txMT;
        mcs = wd->txMCS;
        mt = wd->txMT;
    }

    if (wd->enableAggregation)
    {
        
        if (wd->enableAggregation==2 && !(header[6]&0x1))
        {
            
            header[4] |= 0x0080;

            
            header[hlen] = 0;
            hlen += 1;
        }
        
        
    }

#ifdef ZM_ENABLE_AGGREGATION
    

    
    if (wd->wlanMode == ZM_MODE_AP) {
        if (aggControl && mt == 2) {
            if (wd->enableAggregation==0 && !(header[6]&0x1))
            {
                header[4] |= 0x0080;

                
                header[hlen] = 0;
                hlen += 1;
            }
        }
    }
#endif

    
    len = zfwBufGetSize(dev, buf);

    
    
    macCtrl = 0x208;

    
    if ((header[6] & 0x1) == 0x1)
    {
        
        macCtrl |= 0x4;
    }
    else
    {
        
    #if 0
        
        if (len >= wd->rtsThreshold)
        {
            
            macCtrl |= 1;
        }
    #endif
    }
    
    

    if (wd->wlanMode == ZM_MODE_AP)
    {
        u8_t encryType;
        u16_t iv16;
        u32_t iv32;

        
        if ((header[6] & 0x1) == 0x1)
        {
            
            if (wd->ap.encryMode[vap] == ZM_TKIP)
            {
                wd->ap.iv16[vap]++;

                if(wd->ap.iv16[vap] == 0)
                {
                    wd->ap.iv32[vap]++;
                }

                b1 = (u8_t) (wd->ap.iv16[vap] >> 8);
                b2 = (b1 | 0x20) & 0x7f;
                header[hlen] = ((u16_t)b2 << 8) + b1;
                b1 = (u8_t) wd->ap.iv16[vap];
                b2 = 0x20 | (wd->ap.bcKeyIndex[vap] << 6);
                header[hlen+1] = ((u16_t)b2 << 8) + b1;
                header[hlen+2] = (u16_t) wd->ap.iv32[vap];
                header[hlen+3] = (u16_t) (wd->ap.iv32[vap] >> 16);

                
                macCtrl |= 0x40;
                icvLen = 4;

                
                if ( (!(seq & 0xf))&&(!(flag & 0x4)) )
                {
                    macCtrl |= 0x100;
                    plusLen += 8;
                    *micLen = 8;
                }

                header[4] |= 0x4000;
                hlen += 4;
            }
            else if (wd->ap.encryMode[vap] == ZM_AES)
            {
                wd->ap.iv16[vap]++;

                if(wd->ap.iv16[vap] == 0)
                {
                    wd->ap.iv32[vap]++;
                }

                b1 = (u8_t) wd->ap.iv16[vap];
                b2 = (u8_t) (wd->ap.iv16[vap] >> 8);
                header[hlen] = ((u16_t)b2 << 8) + b1;
                header[hlen+1] = 0x2000 | (wd->ap.bcKeyIndex[vap] << 14);
                header[hlen+2] = (u16_t) (wd->ap.iv32[vap]);
                header[hlen+3] = (u16_t) (wd->ap.iv32[vap] >> 16);

                macCtrl |= 0xc0;
                icvLen = 8;  

                header[4] |= 0x4000;
                hlen += 4;
            }
            #ifdef ZM_ENABLE_CENC
            else if (wd->ap.encryMode[vap] == ZM_CENC)
            {
                

                wd->ap.txiv[vap][0]++;

                if (wd->ap.txiv[vap][0] == 0)
                {
                    wd->ap.txiv[vap][1]++;
                }

                if (wd->ap.txiv[vap][1] == 0)
                {
                    wd->ap.txiv[vap][2]++;
                }

                if (wd->ap.txiv[vap][2] == 0)
                {
                    wd->ap.txiv[vap][3]++;
                }

                if (wd->ap.txiv[vap][3] == 0)
                {
                    wd->ap.txiv[vap][0] = 0;
                    wd->ap.txiv[vap][1] = 0;
                    wd->ap.txiv[vap][2] = 0;
                }

                header[hlen] = (wd->ap.bcKeyIndex[vap] & 0x0001);    
                header[hlen+1] = (u16_t)wd->ap.txiv[vap][0];
                header[hlen+2] = (u16_t)(wd->ap.txiv[vap][0] >> 16);
                header[hlen+3] = (u16_t)wd->ap.txiv[vap][1];
                header[hlen+4] = (u16_t)(wd->ap.txiv[vap][1] >> 16);
                header[hlen+5] = (u16_t)wd->ap.txiv[vap][2];
                header[hlen+6] = (u16_t)(wd->ap.txiv[vap][2] >> 16);
                header[hlen+7] = (u16_t)wd->ap.txiv[vap][3];
                header[hlen+8] = (u16_t)(wd->ap.txiv[vap][3] >> 16);

                macCtrl |= 0x80;
                icvLen = 16;  

                header[4] |= 0x4000;
                hlen += 9;
            }
            #endif 
        }
        else
        {
            
            zfApGetStaEncryType(dev, da, &encryType);

            if (encryType == ZM_TKIP)
            {
                
                zfApGetStaWpaIv(dev, da, &iv16, &iv32);

                iv16++;
                if (iv16 == 0)
                {
                    iv32++;
                }

                b1 = (u8_t) (iv16 >> 8);
                b2 = (b1 | 0x20) & 0x7f;
                header[hlen] = ((u16_t)b2 << 8) + b1;
                b1 = (u8_t) iv16;
                b2 = 0x20;
                header[hlen+1] = ((u16_t)b2 << 8) + b1;
                header[hlen+2] = (u16_t) iv32;
                header[hlen+3] = (u16_t) (iv32 >> 16);

                
                macCtrl |= 0x40;
                icvLen = 4;

                
                if ( (!(seq & 0xf))&&(!(flag & 0x4)) )
                {
                    macCtrl |= 0x100;
                    plusLen += 8;
                    *micLen = 8;
                }

                header[4] |= 0x4000;
                hlen += 4;

                
                zfApSetStaWpaIv(dev, da, iv16, iv32);
            }
            else if (encryType == ZM_AES)
            {
                
                zfApGetStaWpaIv(dev, da, &iv16, &iv32);

                iv16++;
                if (iv16 == 0)
                {
                    iv32++;
                }

                b1 = (u8_t) iv16;
                b2 = (u8_t) (iv16 >> 8);
                header[hlen] = ((u16_t)b2 << 8) + b1;
                header[hlen+1] = 0x2000;
                header[hlen+2] = (u16_t) (iv32);
                header[hlen+3] = (u16_t) (iv32 >> 16);

                macCtrl |= 0xc0;
                icvLen = 8;  

                header[4] |= 0x4000;
                hlen += 4;

                
                zfApSetStaWpaIv(dev, da, iv16, iv32);
            }
            #ifdef ZM_ENABLE_CENC
            else if (encryType == ZM_CENC)
            {
                u32_t txiv[4];
                u8_t keyIdx;

                
                zfApGetStaCencIvAndKeyIdx(dev, da, txiv, &keyIdx);

                txiv[0] += 2;

                if (txiv[0] == 0 || txiv[0] == 1)
                {
                    txiv[1]++;
                }

                if (txiv[1] == 0)
                {
                    txiv[2]++;
                }

                if (txiv[2] == 0)
                {
                    txiv[3]++;
                }

                if (txiv[3] == 0)
                {
                    txiv[0] = 0;
                    txiv[1] = 0;
                    txiv[2] = 0;
                }

                header[hlen] = (keyIdx & 0x0001);    
                header[hlen+1] = (u16_t)txiv[0];
                header[hlen+2] = (u16_t)(txiv[0] >> 16);
                header[hlen+3] = (u16_t)txiv[1];
                header[hlen+4] = (u16_t)(txiv[1] >> 16);
                header[hlen+5] = (u16_t)txiv[2];
                header[hlen+6] = (u16_t)(txiv[2] >> 16);
                header[hlen+7] = (u16_t)txiv[3];
                header[hlen+8] = (u16_t)(txiv[3] >> 16);

                macCtrl |= 0x80;
                icvLen = 16;  

                header[4] |= 0x4000;
                hlen += 9;

                
                zfApSetStaCencIv(dev, da, txiv);
            }
            #endif 
        }

        
        if (wd->ap.protectionMode == 1)
        {
            
            macCtrl &= 0xFFFC;
            macCtrl |= 2;
        }

        
        if (port < 0x20)
        {
            
            
            if ((wd->ap.encryMode[vap] == ZM_WEP64) ||
                    (wd->ap.encryMode[vap] == ZM_WEP128) ||
                    (wd->ap.encryMode[vap] == ZM_WEP256))
            {
                header[4] |= 0x4000;
                header[hlen] = 0x0;   
                header[hlen+1] = wd->ap.bcKeyIndex[vap] << 14; 
                hlen += 2;
                icvLen = 4;
                macCtrl |= 0x40;
            }
        }
        else
        {
            

            
            phyCtrl = 0xc0001;   

            
            if ((wdsPort = (port - 0x20)) >= ZM_MAX_WDS_SUPPORT)
            {
                wdsPort = 0;
            }

            #if 1
            
            switch (wd->ap.wds.encryMode[wdsPort])
            {
            case ZM_WEP64:
            case ZM_WEP128:
            case ZM_WEP256:
                    header[4] |= 0x4000;
                    header[hlen] = 0x0;   
                    header[hlen+1] = wd->ap.bcKeyIndex[vap] << 14; 
                    hlen += 2;
                    icvLen = 4;
                    macCtrl |= 0x40;
                    break;

            case ZM_TKIP:
                    wd->sta.iv16++;

                    if ( wd->sta.iv16 == 0 )
                    {
                        wd->sta.iv32++;
                    }

                    b1 = (u8_t) (wd->sta.iv16 >> 8);
                    b2 = (b1 | 0x20) & 0x7f;
                    header[hlen] = ((u16_t)b2 << 8) + b1;
                    b1 = (u8_t) wd->sta.iv16;
                    b2 = 0x20;
                    header[hlen+1] = ((u16_t)b2 << 8) + b1;
                    header[hlen+2] = (u16_t) wd->sta.iv32;
                    header[hlen+3] = (u16_t) (wd->sta.iv32 >> 16);

                    
                    macCtrl |= 0x40;
                    icvLen = 4;

                    
                    if ( (!(seq & 0xf))&&(!(flag & 0x4)) )
                    {
                        macCtrl |= 0x100;
                        plusLen += 8;
                        *micLen = 8;
                    }

                    header[4] |= 0x4000;
                    hlen += 4;
                    break;

            case ZM_AES:
                    wd->sta.iv16++;
                    if ( wd->sta.iv16 == 0 )
                    {
                        wd->sta.iv32++;
                    }

                    b1 = (u8_t) wd->sta.iv16;
                    b2 = (u8_t) (wd->sta.iv16 >> 8);
                    header[hlen] = ((u16_t)b2 << 8) + b1;
                    header[hlen+1] = 0x2000;
                    header[hlen+2] = (u16_t) (wd->sta.iv32);
                    header[hlen+3] = (u16_t) (wd->sta.iv32 >> 16);

                    macCtrl |= 0xc0; 
                    icvLen = 8;  

                    header[4] |= 0x4000; 
                    hlen += 4; 
                    break;
            }
            #endif
        }
    }
    else   
    {
        encExemptionActionType = zfwGetPktEncExemptionActionType(dev, buf);

        if ( wd->wlanMode == ZM_MODE_INFRASTRUCTURE )
        {
            #if 1
            
            if (wd->sta.wmeConnected != 0)
            {
                
                header[4] |= 0x0080;

                
                header[hlen] = up;
                hlen += 1;
            }
            #endif

            if ( encExemptionActionType == ZM_ENCRYPTION_EXEMPT_NO_EXEMPTION )
            {
                if ( wd->sta.authMode < ZM_AUTH_MODE_WPA )
                {   
                    if ( wd->sta.wepStatus == ZM_ENCRYPTION_WEP_ENABLED )
                    {
                        if ( (wd->sta.encryMode == ZM_WEP64)||
                             (wd->sta.encryMode == ZM_WEP128)||
                             (wd->sta.encryMode == ZM_WEP256) )
                        {
                            header[4] |= 0x4000;
                            header[hlen] = 0x0;   
                            header[hlen+1] = 0x0; 
                            header[hlen+1] |= (((u16_t) wd->sta.keyId) << 14);
                            hlen += 2;
                            icvLen = 4;

                            
                            if ((wd->sta.SWEncryptEnable & ZM_SW_WEP_ENCRY_EN) != 0)
                            {
                                u8_t keyLen = 5;
                                u8_t iv[3];

                                iv[0] = 0x0;
                                iv[1] = 0x0;
                                iv[2] = 0x0;

                                if (wd->sta.SWEncryMode[wd->sta.keyId] == ZM_WEP64)
                                {
                                    keyLen = 5;
                                }
                                else if (wd->sta.SWEncryMode[wd->sta.keyId] == ZM_WEP128)
                                {
                                    keyLen = 13;
                                }
                                else if (wd->sta.SWEncryMode[wd->sta.keyId] == ZM_WEP256)
                                {
                                    keyLen = 29;
                                }

                                zfWEPEncrypt(dev, buf, (u8_t*) snap, snapLen, minusLen, keyLen,
                                        wd->sta.wepKey[wd->sta.keyId], iv);
                            }
                            else
                            {
                                macCtrl |= 0x40;
                            }
                        }
                    }
                }
                else
                {   
                    if ( wd->sta.wpaState >= ZM_STA_WPA_STATE_PK_OK )
                    {
                        wd->sta.iv16++;
                        if ( wd->sta.iv16 == 0 )
                        {
                            wd->sta.iv32++;
                        }

                        
                        if ( wd->sta.encryMode == ZM_TKIP )
                        {
                            b1 = (u8_t) (wd->sta.iv16 >> 8);
                            b2 = (b1 | 0x20) & 0x7f;
                            header[hlen] = ((u16_t)b2 << 8) + b1;
                            b1 = (u8_t) wd->sta.iv16;
                            b2 = 0x20;

                            
                            
                            header[hlen+1] = (((u16_t)b2 << 8) + b1);
                            header[hlen+2] = (u16_t) wd->sta.iv32;
                            header[hlen+3] = (u16_t) (wd->sta.iv32 >> 16);

                            
                            if ((wd->sta.SWEncryptEnable & ZM_SW_TKIP_ENCRY_EN) == 0)
                            {
                                
                                
                                macCtrl |= 0x40;
                                icvLen = 4;

                                
                                if ( (!(seq & 0xf))&&(!(flag & 0x4)) )
                                {
                                    macCtrl |= 0x100;
                                    plusLen += 8;
                                    *micLen = 8;
                                }
                            }
                            else
                            {
                                u8_t mic[8];
                                u16_t offset;
                                u32_t icv;
                                u8_t RC4Key[16];

                                
                                zmw_declare_for_critical_section();

                                zmw_enter_critical_section(dev);
                                
                                zfCalTxMic(dev, buf, (u8_t *)snap, snapLen, minusLen, da, sa, up, mic);

                                offset = zfwBufGetSize(dev, buf);

                                
                                zfCopyToIntTxBuffer(dev, buf, mic, offset, 8);
                                zfwBufSetSize(dev, buf, offset+8);
                                zmw_leave_critical_section(dev);

                                
                                zfTkipPhase1KeyMix(wd->sta.iv32, &wd->sta.txSeed);
                                zfTkipPhase2KeyMix(wd->sta.iv16, &wd->sta.txSeed);
                                zfTkipGetseeds(wd->sta.iv16, RC4Key, &wd->sta.txSeed);

                                
                                zfTKIPEncrypt(dev, buf, (u8_t *)snap, snapLen, minusLen, 16, RC4Key, &icv);

                                icvLen = 4;
                                len += 8;
                            }

                            header[4] |= 0x4000;
                            hlen += 4;
                        }
                        else if ( wd->sta.encryMode == ZM_AES )
                        {
                            b1 = (u8_t) wd->sta.iv16;
                            b2 = (u8_t) (wd->sta.iv16 >> 8);
                            header[hlen] = ((u16_t)b2 << 8) + b1;
                            
                            
                            header[hlen+1] = 0x2000;
                            header[hlen+2] = (u16_t) (wd->sta.iv32);
                            header[hlen+3] = (u16_t) (wd->sta.iv32 >> 16);

                            macCtrl |= 0xc0;
                            icvLen = 8;  

                            header[4] |= 0x4000;
                            hlen += 4;
                        }
                        #ifdef ZM_ENABLE_CENC
                        else if ( wd->sta.encryMode == ZM_CENC )
                        {
                            
                            wd->sta.txiv[0] += 2;

                            if (wd->sta.txiv[0] == 0 || wd->sta.txiv[0] == 1)
                            {
                                wd->sta.txiv[1]++;
                            }

                            if (wd->sta.txiv[1] == 0)
                            {
                                wd->sta.txiv[2]++;
                            }

                            if (wd->sta.txiv[2] == 0)
                            {
                                wd->sta.txiv[3]++;
                            }

                            if (wd->sta.txiv[3] == 0)
                            {
                                wd->sta.txiv[0] = 0;
                                wd->sta.txiv[1] = 0;
                                wd->sta.txiv[2] = 0;
                            }

                            header[hlen] = (wd->sta.cencKeyId & 0x0001);    
                            header[hlen+1] = (u16_t) wd->sta.txiv[0];
                            header[hlen+2] = (u16_t) (wd->sta.txiv[0] >> 16);
                            header[hlen+3] = (u16_t) wd->sta.txiv[1];
                            header[hlen+4] = (u16_t) (wd->sta.txiv[1] >> 16);
                            header[hlen+5] = (u16_t) wd->sta.txiv[2];
                            header[hlen+6] = (u16_t) (wd->sta.txiv[2] >> 16);
                            header[hlen+7] = (u16_t) wd->sta.txiv[3];
                            header[hlen+8] = (u16_t) (wd->sta.txiv[3] >> 16);

                            macCtrl |= 0x80;
                            icvLen = 16;  

                            header[4] |= 0x4000;
                            hlen += 9;
                        }
                        #endif 
                    }
                }
            } 
        } 

        if ( wd->wlanMode == ZM_MODE_IBSS )
        {
            if ( encExemptionActionType == ZM_ENCRYPTION_EXEMPT_NO_EXEMPTION )
            {
#ifdef ZM_ENABLE_IBSS_WPA2PSK
                if( wd->sta.oppositeInfo[userIdx].wpaState >= ZM_STA_WPA_STATE_PK_OK || wd->sta.wpaState >= ZM_STA_WPA_STATE_PK_OK)
                {
                    int isUnicast = 1 ;

                    if((da[0]& 0x1))
                    {
                        isUnicast = 0 ; 
                    }

                    if( wd->sta.ibssWpa2Psk == 1 )
                    { 
                        if ( isUnicast )
                        {
                            iv16 = &wd->sta.oppositeInfo[userIdx].iv16;
                            iv32 = &wd->sta.oppositeInfo[userIdx].iv32;
                        }
                        else
                        {
                            iv16 = &wd->sta.iv16;
                            iv32 = &wd->sta.iv32;
                        }
                    }
                    else
                    {
                        iv16 = &wd->sta.iv16;
                        iv32 = &wd->sta.iv32;
                    }

                    (*iv16)++;
                    if ( *iv16 == 0 )
                    {
                        *iv32++;
                    }

                    if ( wd->sta.oppositeInfo[userIdx].encryMode == ZM_AES || wd->sta.encryMode == ZM_AES)
                    {
                        
                        b1 = (u8_t) (*iv16);
                        b2 = (u8_t) ((*iv16) >> 8);
                        header[hlen] = ((u16_t)b2 << 8) + b1;

                        if ( isUnicast )
                        {
                            header[hlen+1] = 0x2000;
                        }
                        else
                        {
                            header[hlen+1] = 0x2000 | (((u16_t) wd->sta.keyId) << 14);
                        }

                        header[hlen+2] = (u16_t) (*iv32);
                        header[hlen+3] = (u16_t) ((*iv32) >> 16);
                        macCtrl |= 0xc0;
                        icvLen = 8;  
                    }

                    header[4] |= 0x4000;
                    hlen += 4;
                }
                else if ( wd->sta.wepStatus == ZM_ENCRYPTION_WEP_ENABLED)
                {
                    if ( (wd->sta.encryMode == ZM_WEP64)||
                        (wd->sta.encryMode == ZM_WEP128)||
                        (wd->sta.encryMode == ZM_WEP256) )
                    {
                        header[4] |= 0x4000;
                        header[hlen] = 0x0;   
                        header[hlen+1] = 0x0; 
                        header[hlen+1] |= (((u16_t) wd->sta.keyId) << 14);
                        hlen += 2;
                        icvLen = 4;
                        macCtrl |= 0x40;
                    }
                }
#else
                
                if( wd->sta.wpaState >= ZM_STA_WPA_STATE_PK_OK )
                {
                    int isUnicast = 1 ;

                    if((da[0]& 0x1))
                    {
                        isUnicast = 0 ; 
                    }

                    wd->sta.iv16++;
                    if ( wd->sta.iv16 == 0 )
                    {
                        wd->sta.iv32++;
                    }

                    if ( wd->sta.encryMode == ZM_AES )
                    {
                        
                        b1 = (u8_t) wd->sta.iv16;
                        b2 = (u8_t) (wd->sta.iv16 >> 8);
                        header[hlen] = ((u16_t)b2 << 8) + b1;

                        if ( isUnicast )
                        {
                            header[hlen+1] = 0x2000;
                        }
                        else
                        {
                            header[hlen+1] = 0x2000 | (((u16_t) wd->sta.keyId) << 14);
                        }

                            header[hlen+2] = (u16_t) (wd->sta.iv32);
                            header[hlen+3] = (u16_t) (wd->sta.iv32 >> 16);
                            macCtrl |= 0xc0;
                            icvLen = 8;  
                    }

                    header[4] |= 0x4000;
                    hlen += 4;
                }
                else if ( wd->sta.wepStatus == ZM_ENCRYPTION_WEP_ENABLED)
                {
                    if ( (wd->sta.encryMode == ZM_WEP64)||
                         (wd->sta.encryMode == ZM_WEP128)||
                         (wd->sta.encryMode == ZM_WEP256) )
                    {
                        header[4] |= 0x4000;
                        header[hlen] = 0x0;   
                        header[hlen+1] = 0x0; 
                        header[hlen+1] |= (((u16_t) wd->sta.keyId) << 14);
                        hlen += 2;
                        icvLen = 4;
                        macCtrl |= 0x40;
                    }
                }
#endif
            }   
        }   
        else if ( wd->wlanMode == ZM_MODE_PSEUDO )
        {
            switch (wd->sta.encryMode)
	        {
            case ZM_WEP64:
            case ZM_WEP128:
            case ZM_WEP256:
                header[4] |= 0x4000;
                header[hlen] = 0x0;   
                header[hlen+1] = 0x0; 
                hlen += 2;
                icvLen = 4;
                macCtrl |= 0x40;
                break;

            case ZM_TKIP:
            {
                wd->sta.iv16++;
                if ( wd->sta.iv16 == 0 )
                {
                    wd->sta.iv32++;
                }

                b1 = (u8_t) (wd->sta.iv16 >> 8);
                b2 = (b1 | 0x20) & 0x7f;
                header[hlen] = ((u16_t)b2 << 8) + b1;
                b1 = (u8_t) wd->sta.iv16;
                b2 = 0x20;
                header[hlen+1] = ((u16_t)b2 << 8) + b1;
                header[hlen+2] = (u16_t) wd->sta.iv32;
                header[hlen+3] = (u16_t) (wd->sta.iv32 >> 16);

                
                macCtrl |= 0x40;
                icvLen = 4;

                
                if ( (!(seq & 0xf))&&(!(flag & 0x4)) )
                {
                    macCtrl |= 0x100;
                    plusLen += 8;
                    *micLen = 8;
                }

                header[4] |= 0x4000;
                hlen += 4;
            }
                break;

            case ZM_AES:
            {
                wd->sta.iv16++;
                if ( wd->sta.iv16 == 0 )
                {
                    wd->sta.iv32++;
                }

                b1 = (u8_t) wd->sta.iv16;
                b2 = (u8_t) (wd->sta.iv16 >> 8);
                header[hlen] = ((u16_t)b2 << 8) + b1;
                header[hlen+1] = 0x2000;
                header[hlen+2] = (u16_t) (wd->sta.iv32);
                header[hlen+3] = (u16_t) (wd->sta.iv32 >> 16);
                macCtrl |= 0xc0;
                icvLen = 8;  
                header[4] |= 0x4000;
                hlen += 4;
            }
                    break;

              #ifdef ZM_ENABLE_CENC
              case ZM_CENC:
                    
                    wd->sta.txiv[0] += 2;

                    if (wd->sta.txiv[0] == 0 || wd->sta.txiv[0] == 1)
                    {
                        wd->sta.txiv[1]++;
                    }

                    if (wd->sta.txiv[1] == 0)
                    {
                        wd->sta.txiv[2]++;
                    }

                    if (wd->sta.txiv[2] == 0)
                    {
                        wd->sta.txiv[3]++;
                    }

                    if (wd->sta.txiv[3] == 0)
                    {
                        wd->sta.txiv[0] = 0;
                        wd->sta.txiv[1] = 0;
                        wd->sta.txiv[2] = 0;
                    }

                    header[hlen] = 0;
                    header[hlen+1] = (u16_t) wd->sta.txiv[0];
                    header[hlen+2] = (u16_t) (wd->sta.txiv[0] >> 16);
                    header[hlen+3] = (u16_t) wd->sta.txiv[1];
                    header[hlen+4] = (u16_t) (wd->sta.txiv[1] >> 16);
                    header[hlen+5] = (u16_t) wd->sta.txiv[2];
                    header[hlen+6] = (u16_t) (wd->sta.txiv[2] >> 16);
                    header[hlen+7] = (u16_t) wd->sta.txiv[3];
                    header[hlen+8] = (u16_t) (wd->sta.txiv[3] >> 16);

                    macCtrl |= 0x80;
                    icvLen = 16;  

                    header[4] |= 0x4000;
                    hlen += 9;
				break;
		    #endif 
			}
		}

        

        
        if (wd->enableProtectionMode)
        {
            if (wd->enableProtectionMode==2)
            {
                
                macCtrl &= 0xFFFC;
                macCtrl |= 2;
            }
            
            
        }
        else
        {

            
            if (wd->sta.bProtectionMode == TRUE)
            {
                
                macCtrl &= 0xFFFC;
                macCtrl |= 2;
            }
        }

    }

    if (wd->txMCS != 0xff)
    {
        
	    phyCtrl = ((u32_t)wd->txMCS<<16) + wd->txMT;
        mcs = wd->txMCS;
        mt = wd->txMT;
    }

    if (mt == 2)
    {
#if 0
        
	    if (wd->sta.preambleTypeHT == ZM_PREAMBLE_TYPE_GREEN_FIELD)
	    {
            phyCtrl |= 0x4;     
        }
#endif
        
        if (wd->sta.htCtrlBandwidth == ZM_BANDWIDTH_40MHZ)
        {
            phyCtrl |= (0x80<<16);  
        }
#if 0
        
        if (wd->sta.htCtrlSTBC<=0x3)
        {
            phyCtrl |= (wd->sta.htCtrlSTBC<<28);   
        }
#endif
        
        if(wd->sta.htCtrlSG)
        {
            phyCtrl |= (0x8000<<16);         
        }

        
        if ( ((mcs >=0x8) && (mcs<=0xf))  || (wd->sta.htCtrlSTBC) )
        {
       	    phyCtrl |= 0x1800;               
    	}
    }
    else if(mt == 1)
    {
        #if 0
        
        
        if (wd->sta.htCtrlBandwidth == ZM_BANDWIDTH_40MHZ)
        {
            phyCtrl |= (0x80<<16);  
            mt = 3;                 
            phyCtrl |= mt;
        }
        #endif
    }
    else if(mt == 0)
    {
	
        if (wd->preambleTypeInUsed == ZM_PREAMBLE_TYPE_SHORT)
        {
    	       
    	}
    }

    
    if (wd->sta.defaultTA)
    {
        phyCtrl |= 0x1000;
    }
    else
    {
        phyCtrl |= 0x0800;
    }

    
    if ((mt == 0) || (mt == 1)) 
    {
        if (mcs < 16)
        {
            wd->CurrentTxRateKbps = zcIndextoRateBG[mcs];
        }
    }
    else if (mt == 2)
    {
        if (mcs < 16)
        {
            if (wd->sta.htCtrlBandwidth == ZM_BANDWIDTH_40MHZ)
            {
                if((phyCtrl & 0x80000000) != 0)
                {
                    
                    wd->CurrentTxRateKbps = zcIndextoRateN40S[mcs];
                }
                else
                {
                    
                    wd->CurrentTxRateKbps = zcIndextoRateN40L[mcs];
                }
            }
            else
            {
                if((phyCtrl & 0x80000000) != 0)
                {
                    
                    wd->CurrentTxRateKbps = zcIndextoRateN20S[mcs];
                }
                else
                {
                    
                    wd->CurrentTxRateKbps = zcIndextoRateN20L[mcs];
                }
            }
        }
    }

    
    
    
    
    
    
    
    header[0] = ((hlen<<1)-8)+plusLen+(len-minusLen)+icvLen+4;  

    
    if ((header[6] & 0x1) != 0x1) 
    {
        if (header[0] >= wd->rtsThreshold)
        {
            
            macCtrl |= 1;
        }
    }

    if ( wd->sta.encryMode == ZM_TKIP )
        tkipFrameOffset = 8;

    if( wd->sta.EnableHT != 1 )
    { 
        if ( header[0] > ( wd->fragThreshold + tkipFrameOffset ) )
        {
            return 0; 
        }
    }

    
    
    
    

    
    if (rateProbingFlag != 0)
    {
        macCtrl |= 0x8000;
    }
    header[1] = macCtrl;
    
    header[2] = (u16_t) ((phyCtrl&0xffff) | 0x700 | (zcUpToAc[up&0x7]<<13));
    
    header[3] = (u16_t) ((phyCtrl>>16) | 0x700);

    if (wd->enableAggregation)
    {
        
        if (wd->enableAggregation==2 && !(header[6]&0x1))
        {
            if (((header[2] & 0x3) == 2))
            {
                
                header[1] |= 0x20;
            }
        }
        
        
    }

#ifdef ZM_ENABLE_AGGREGATION
    if (wd->addbaComplete) {
        #ifdef ZM_BYPASS_AGGR_SCHEDULING
        if (!(header[6]&0x1) && !rateProbingFlag && (wd->enableAggregation != 1))
        {
            if (((header[2] & 0x3) == 2))
            {
                
                
                if ((wd->sta.SWEncryptEnable & ZM_SW_TKIP_ENCRY_EN) == 0 &&
                    (wd->sta.SWEncryptEnable & ZM_SW_WEP_ENCRY_EN) == 0)
                {
                    
                    header[1] |= (0x20 | (zcUpToAc[up&0x7]<<10));

                    
                    {
                        
                        
                        if ( (wd->sta.athOwlAp == 1) || (wd->sta.RTSInAGGMode == TRUE) )
                        {
                            header[1] &= 0xfffc;
                            header[1] |= 0x1;
                        }

                        
                        
                        
                    }
                }
            }
        }
        #else
        
        if (aggControl && aggControl->aggEnabled) {
            if (wd->enableAggregation==0 && !(header[6]&0x1))
            {
                if (((header[2] & 0x3) == 2))
                {
                    
                    header[1] |= 0x20;
                    if (ZM_AGG_LAST_MPDU == aggControl->ampduIndication)
                        header[1] |= 0x4000;
                }
                else {
                    zm_debug_msg1("no aggr, header[2]&0x3 = ",header[2] & 0x3)
                    aggControl->aggEnabled = 0;
                }
            }
            else {
                zm_debug_msg1("no aggr, wd->enableAggregation = ", wd->enableAggregation);
                zm_debug_msg1("no aggr, !header[6]&0x1 = ",!(header[6]&0x1));
                aggControl->aggEnabled = 0;
            }
        }
        #endif

        #ifdef ZM_AGGR_BIT_ON
        if (!(header[6]&0x1) && !rateProbingFlag)
        {
            if (((header[2] & 0x3) == 2))
            {
                
                
                header[1] |= (0x20 | (zcUpToAc[up&0x7]<<10));

                
                {
                    
                    header[1] &= 0xfffc;
                    header[1] |= 0x1;
                }
            }
        }
        #endif
    }
#endif

    return (hlen<<1);
}


u16_t zfTxGenMmHeader(zdev_t* dev, u8_t frameType, u16_t* dst,
        u16_t* header, u16_t len, zbuf_t* buf, u16_t vap, u8_t encrypt)
{
    
    u8_t  hlen = 32;        

    zmw_get_wlan_dev(dev);

    zmw_declare_for_critical_section();

    
    
    header[0] = 24+len+4;   
    if ((dst[0] & 0x1) != 0) 
    {
        header[1] = 0xc;            
    }
    else
    {
        header[1] = 0x8;            
    }
    
    if (wd->wlanMode == ZM_MODE_AP)
    {
        if (wd->frequency < 3000)
        {
            
            header[2] = 0x0f00;          
            header[3] = 0x0000;          
        }
        else
        {
            
            header[2] = 0x0f01;          
            header[3] = 0x000B;          
        }
    }
    else
    {
        if (wd->sta.currentFrequency < 3000)
        {
            
            header[2] = 0x0f00;          
            header[3] = 0x0001;          
        }
        else
        {
            
            header[2] = 0x0f01;          
            header[3] = 0x000B;          
        }
    }
    
    
    header[4+0] = frameType;
    
    header[4+1] = 0;

    if (wd->wlanMode == ZM_MODE_INFRASTRUCTURE)
    {
        if ( frameType == ZM_WLAN_FRAME_TYPE_PROBEREQ )
        {
            header[4+8] = 0xFFFF;
            header[4+9] = 0xFFFF;
            header[4+10] = 0xFFFF;
        }
        else if ( frameType == ZM_WLAN_FRAME_TYPE_BA ) {
            
        }
        else
        {
            header[4+8] = wd->sta.bssid[0];
            header[4+9] = wd->sta.bssid[1];
            header[4+10] = wd->sta.bssid[2];
        }
    }
    else if (wd->wlanMode == ZM_MODE_PSEUDO)
    {
        
        header[4+8] = 0;
        header[4+9] = 0;
        header[4+10] = 0;
    }
    else if (wd->wlanMode == ZM_MODE_IBSS)
    {
        header[4+8] = wd->sta.bssid[0];
        header[4+9] = wd->sta.bssid[1];
        header[4+10] = wd->sta.bssid[2];

        if ( frameType == ZM_WLAN_FRAME_TYPE_ATIM )
        {
            
            
            header[2] |= ZM_BIT_15;
        }
    }
    else if (wd->wlanMode == ZM_MODE_AP)
    {
        
        header[4+8] = wd->macAddr[0];
        header[4+9] = wd->macAddr[1];
#ifdef ZM_VAPMODE_MULTILE_SSID
        header[4+10] = wd->macAddr[2]; 
#else
        header[4+10] = wd->macAddr[2] + (vap<<8); 
#endif
        
        
        
        
        if (frameType == ZM_WLAN_FRAME_TYPE_PROBEREQ)
        {
            header[4+8] = 0xFFFF;
            header[4+9] = 0xFFFF;
            header[4+10] = 0xFFFF;
        }
    }

    
    header[4+2] = dst[0];
    header[4+3] = dst[1];
    header[4+4] = dst[2];

    
    header[4+5] = wd->macAddr[0];
    header[4+6] = wd->macAddr[1];
    if (wd->wlanMode == ZM_MODE_AP)
    {
#ifdef ZM_VAPMODE_MULTILE_SSID
        header[4+7] = wd->macAddr[2]; 
#else
        header[4+7] = wd->macAddr[2] + (vap<<8); 
#endif
    }
    else
    {
        header[4+7] = wd->macAddr[2];
    }

    
    zmw_enter_critical_section(dev);
    header[4+11] = ((wd->mmseq++)<<4);
    zmw_leave_critical_section(dev);

    if( frameType == ZM_WLAN_FRAME_TYPE_QOS_NULL )
    {
        
        header[4+12] = 0x0;
        hlen+=2;
        header[0]+=2;
    }

    if ( encrypt )
    {
        if ( wd->sta.wepStatus == ZM_ENCRYPTION_WEP_ENABLED )
        {
            if ( (wd->sta.encryMode == ZM_WEP64)||
                 (wd->sta.encryMode == ZM_WEP128)||
                 (wd->sta.encryMode == ZM_WEP256) )
            {
                header[4] |= 0x4000;
                header[16] = 0x0;   
                header[17] = 0x0; 
                header[17] |= (((u16_t) wd->sta.keyId) << 14);
                hlen += 4;

                header[0] += 8;         
                header[1] |= 0x40;      
             }
        }
    }

    
    if ( frameType != ZM_WLAN_FRAME_TYPE_PSPOLL )
    {
        header[1] |= 0x200;
    }

    return hlen;
}

void zfInitMacApMode(zdev_t* dev)
{
    u16_t i;

    zmw_get_wlan_dev(dev);

    zfHpEnableBeacon(dev, ZM_MODE_AP, (wd->beaconInterval/wd->ap.vapNumber), 1, 0);

    
    zfHpSetApStaMode(dev, ZM_HAL_80211_MODE_AP);

    
    
    if (wd->ap.vapNumber >= 2)
    {
        for (i=1; i<ZM_MAX_AP_SUPPORT; i++)
        {
            if (((wd->ap.apBitmap >> i) & 0x1) != 0)
            {
                u16_t mac[3];
                mac[0] = wd->macAddr[0];
                mac[1] = wd->macAddr[1];
#ifdef ZM_VAPMODE_MULTILE_SSID
                mac[2] = wd->macAddr[2]; 
#else
                mac[2] = wd->macAddr[2] + (i<<8); 
#endif
                zfHpSetMacAddress(dev, mac, i);

            }
        }
    }

    
    zfHpSetBasicRateSet(dev, wd->bRateBasic, wd->gRateBasic);

    
    zfUpdateDefaultQosParameter(dev, 1);

    return;
}

u16_t zfChGetNextChannel(zdev_t* dev, u16_t frequency, u8_t* pbPassive)
{
    u8_t   i;
    u8_t   bPassive;

    zmw_get_wlan_dev(dev);

    
    if ( pbPassive == NULL )
    {
        pbPassive = &bPassive;
    }

    for( i=0; i<wd->regulationTable.allowChannelCnt; i++ )
    {
        if ( wd->regulationTable.allowChannel[i].channel == frequency )
        {
            if ( i == (wd->regulationTable.allowChannelCnt-1) )
            {
                i = 0;
            }
            else
            {
                i++;
            }

            if ( wd->regulationTable.allowChannel[i].channelFlags
                    & ZM_REG_FLAG_CHANNEL_PASSIVE )
            {
                *pbPassive = TRUE;
            }
            else
            {
                *pbPassive = FALSE;
            }

            return wd->regulationTable.allowChannel[i].channel;
        }
    }

    return 0xffff;
}

u16_t zfChGetFirstChannel(zdev_t* dev, u8_t* pbPassive)
{
    u8_t   bPassive;

    zmw_get_wlan_dev(dev);

    
    if ( pbPassive == NULL )
    {
        pbPassive = &bPassive;
    }

   if ( wd->regulationTable.allowChannel[0].channelFlags & ZM_REG_FLAG_CHANNEL_PASSIVE )
    {
        *pbPassive = TRUE;
    }
    else
    {
        *pbPassive = FALSE;
    }

    return wd->regulationTable.allowChannel[0].channel;
}

u16_t zfChGetFirst2GhzChannel(zdev_t* dev)
{
    u8_t    i;

    zmw_get_wlan_dev(dev);

    for( i=0; i<wd->regulationTable.allowChannelCnt; i++ )
    {
        if ( wd->regulationTable.allowChannel[i].channel < 3000 )
        {
            
            return wd->regulationTable.allowChannel[i].channel;
        }
    }

    
    return 0;
}

u16_t zfChGetFirst5GhzChannel(zdev_t* dev)
{
    u8_t    i;

    zmw_get_wlan_dev(dev);

    for( i=0; i<wd->regulationTable.allowChannelCnt; i++ )
    {
        if ( wd->regulationTable.allowChannel[i].channel > 3000 )
        {
            
            return wd->regulationTable.allowChannel[i].channel;
        }
    }

    
    return 0;
}

u16_t zfChGetLastChannel(zdev_t* dev, u8_t* pbPassive)
{
    u8_t   bPassive;
    u8_t   ChannelIndex;

    zmw_get_wlan_dev(dev);

    ChannelIndex = wd->regulationTable.allowChannelCnt-1;

    
    if ( pbPassive == NULL )
    {
        pbPassive = &bPassive;
    }

    if ( wd->regulationTable.allowChannel[ChannelIndex].channelFlags
            & ZM_REG_FLAG_CHANNEL_PASSIVE )
    {
        *pbPassive = TRUE;
    }
    else
    {
        *pbPassive = FALSE;
    }

    return wd->regulationTable.allowChannel[ChannelIndex].channel;
}

u16_t zfChGetLast5GhzChannel(zdev_t* dev)
{
    u8_t    i;
    u16_t   last5Ghzfrequency;

    zmw_get_wlan_dev(dev);

    last5Ghzfrequency = 0;
    for( i=0; i<wd->regulationTable.allowChannelCnt; i++ )
    {
        if ( wd->regulationTable.allowChannel[i].channel > 3000 )
        {
            last5Ghzfrequency = wd->regulationTable.allowChannel[i].channel;
        }
    }

    return last5Ghzfrequency;
}




u16_t zfChNumToFreq(zdev_t* dev, u8_t ch, u8_t freqBand)
{
    u16_t freq = 0xffff;

    if ( freqBand == 0 )
    {
        if (ch > 14)
        {   
            freqBand = 2;
        }
        else
        {
            freqBand = 1;
        }
    }

    if ( freqBand == 2 )
    {   
        if ( (ch >= 184)&&(ch <= 196) )
        {
            freq = 4000 + ch*5;
        }
        else
        {
            freq = 5000 + ch*5;
        }
    }
    else
    {   
        if ( ch == 14 )
        {
            freq = ZM_CH_G_14;
        }
        else
        {
            freq = ZM_CH_G_1 + (ch-1)*5;
        }
    }

    return freq;
}

u8_t zfChFreqToNum(u16_t freq, u8_t* pbIs5GBand)
{
    u8_t   ch;
    u8_t   Is5GBand;

    
    if ( pbIs5GBand == NULL )
    {
        pbIs5GBand = &Is5GBand;
    }

    *pbIs5GBand = FALSE;

    if ( freq == ZM_CH_G_14 )
    {
        ch = 14;
    }
    else if ( freq < 4000 )
    {
        ch = (freq - ZM_CH_G_1) / 5 + 1;
    }
    else if ( freq < 5000 )
    {
        ch = (freq - 4000) / 5;
        *pbIs5GBand = TRUE;
    }
    else
    {
        ch = (freq - 5000) / 5;
        *pbIs5GBand = TRUE;
    }

    return ch;
}
