










#include "../80211core/cprecomp.h"
#include "hpani.h"
#include "hpusb.h"

extern void zfwUsbCmd(zdev_t* dev, u8_t endpt, u32_t* cmd, u16_t cmdLen);

extern void zfIdlRsp(zdev_t* dev, u32_t* rsp, u16_t rspLen);
extern u16_t zfDelayWriteInternalReg(zdev_t* dev, u32_t addr, u32_t val);
extern u16_t zfFlushDelayWrite(zdev_t* dev);


#define USB_ENDPOINT_TX_INDEX   1
#define USB_ENDPOINT_RX_INDEX   2
#define USB_ENDPOINT_INT_INDEX  3
#define USB_ENDPOINT_CMD_INDEX  4

void zfIdlCmd(zdev_t* dev, u32_t* cmd, u16_t cmdLen)
{
#if ZM_SW_LOOP_BACK != 1
    zfwUsbCmd(dev, USB_ENDPOINT_CMD_INDEX, cmd, cmdLen);
#endif

    return;
}




void zfAdjustCtrlSetting(zdev_t* dev, u16_t* header, zbuf_t* buf)
{
    
    

    u32_t oldMT;
	u32_t oldMCS;

    u32_t phyCtrl;
    u32_t oldPhyCtrl;

    u16_t tpc = 0;
    struct zsHpPriv* hpPriv;

    zmw_get_wlan_dev(dev);
    hpPriv=wd->hpPrivate;

   
    if (header == NULL)
    {
        oldPhyCtrl = zmw_buf_readh(dev, buf, 4) | ((u32_t)zmw_buf_readh(dev, buf, 6) << 16);
    }
    else
    {
        oldPhyCtrl = header[2] | ((u32_t)header[3] <<16);
    }

	phyCtrl = 0;


	
	oldMT = oldPhyCtrl&0x3;
	phyCtrl |= oldMT;
    if ( oldMT == 0x3 )   
		phyCtrl |= 0x1;


	
	phyCtrl |= (oldPhyCtrl&0x4);

	
	if ( oldPhyCtrl&0x800000 )    
	{
		#if 0
		if (oldMT == 0x3)             
            phyCtrl |= (0x3<<3);   
		else
			phyCtrl |= (0x2<<3);   
		#else
		if (oldMT == 0x2 && ((struct zsHpPriv*)wd->hpPrivate)->hwBw40)
		{
			phyCtrl |= (0x2<<3);   
		}
		#endif
	}
	else {
        oldPhyCtrl &= ~0x80000000;
    }

	
	oldMCS = (oldPhyCtrl&0x7f0000)>>16;  
	phyCtrl |= (oldMCS<<18);

	
    phyCtrl |= (oldPhyCtrl&0x80000000);

	
	
	if (hpPriv->halCapability & ZM_HP_CAP_11N_ONE_TX_STREAM)
	{
	    phyCtrl |= (0x1<<15);
	}
	else
	{
	    
	    
	    
	    
	    if ((oldMT == 2) || (oldMT == 3))
	    {
	        phyCtrl |= (0x5<<15);
	    }
	    else if (oldMT == 1)
	    {
	        if ((oldMCS == 0xb) || (oldMCS == 0xf) ||
	            (oldMCS == 0xa) || (oldMCS == 0xe) ||
	            (oldMCS == 0x9))                       
	        {
	            phyCtrl |= (0x5<<15);
	        }
	        else
	        {
	            phyCtrl |= (0x1<<15);
	        }
	    }
	    else 
	    {
	        phyCtrl |= (0x5<<15);
	    }
	}
	
	

	
	
	if (hpPriv->hwFrequency < 3000)
	{
        if (oldMT == 0)
        {
            
            tpc = (hpPriv->tPow2xCck[oldMCS]&0x3f);
        }
        else if (oldMT == 1)
        {
            
            if (oldMCS == 0xc)
            {
                tpc = (hpPriv->tPow2x2g[3]&0x3f);
            }
            else if (oldMCS == 0x8)
            {
                tpc = (hpPriv->tPow2x2g[2]&0x3f);
            }
            else if (oldMCS == 0xd)
            {
                tpc = (hpPriv->tPow2x2g[1]&0x3f);
            }
            else if (oldMCS == 0x9)
            {
                tpc = ((hpPriv->tPow2x2g[0]-hpPriv->tPow2x2g24HeavyClipOffset)&0x3f);
            }
            else
            {
                tpc = (hpPriv->tPow2x2g[0]&0x3f);
            }
        }
        else if (oldMT == 2)
        {
            if ( oldPhyCtrl&0x800000 )    
            {
                
                tpc = (hpPriv->tPow2x2gHt40[oldMCS&0x7]&0x3f);
            }
            else
            {
                
                tpc = (hpPriv->tPow2x2gHt20[oldMCS&0x7]&0x3f);
            }
        }
    }
    else  
    {
        if (oldMT == 1)
        {
            
            if (oldMCS == 0xc)
            {
                tpc = (hpPriv->tPow2x5g[3]&0x3f);
            }
            else if (oldMCS == 0x8)
            {
                tpc = (hpPriv->tPow2x5g[2]&0x3f);
            }
            else if (oldMCS == 0xd)
            {
                tpc = (hpPriv->tPow2x5g[1]&0x3f);
            }
            else
            {
                tpc = (hpPriv->tPow2x5g[0]&0x3f);
            }
        }
        else if (oldMT == 2)
        {
            if ( oldPhyCtrl&0x800000 )    
            {
                
                tpc = (hpPriv->tPow2x5gHt40[oldMCS&0x7]&0x3f);
            }
            else
            {
                
                tpc = (hpPriv->tPow2x5gHt20[oldMCS&0x7]&0x3f);
            }
        }
    }

    
	
	if ((oldMT==2) && (oldPhyCtrl&0x800000) )
	{
	    tpc += 2;
	}
	tpc &= 0x3f;

    
    if(wd->forceTxTPC)
    {
        tpc = (u16_t)(wd->forceTxTPC & 0x3f);
    }

    if (hpPriv->hwFrequency < 3000) {
        wd->maxTxPower2 &= 0x3f;
        tpc = (tpc > wd->maxTxPower2)? wd->maxTxPower2 : tpc;
    } else {
        wd->maxTxPower5 &= 0x3f;
        tpc = (tpc > wd->maxTxPower5)? wd->maxTxPower5 : tpc;
    }


#define ZM_MIN_TPC     5
#define ZM_TPC_OFFSET  5
#define ZM_SIGNAL_THRESHOLD  56
    if ((wd->sta.bScheduleScan == FALSE) && (wd->sta.bChannelScan == FALSE))
    {
        if (( wd->wlanMode == ZM_MODE_INFRASTRUCTURE )
                && (zfStaIsConnected(dev))
                && (wd->SignalStrength > ZM_SIGNAL_THRESHOLD))
        {
            if (tpc > ((ZM_MIN_TPC+ZM_TPC_OFFSET)*2))
            {
                tpc -= (ZM_TPC_OFFSET*2);
            }
            else if (tpc > (ZM_MIN_TPC*2))
            {
                tpc = (ZM_MIN_TPC*2);
            }
        }
    }
#undef ZM_MIN_TPC
#undef ZM_TPC_OFFSET
#undef ZM_SIGNAL_THRESHOLD

    #ifndef ZM_OTUS_LINUX_PHASE_2
    phyCtrl |= (tpc & 0x3f) << 9;
    #endif

    
    if ((phyCtrl&0x3) == 2)
	{
	    phyCtrl |= ((phyCtrl >> 12) & 0x1c0);
    }

	
    if (header == NULL)
    {
        zmw_buf_writeh(dev, buf, 4, (u16_t) (phyCtrl&0xffff));
        zmw_buf_writeh(dev, buf, 6, (u16_t) (phyCtrl>>16));
    }
    else
    {
        
        header[2] = (u16_t) (phyCtrl&0xffff);
        
        header[3] = (u16_t) (phyCtrl>>16);
    }

	zm_msg2_tx(ZM_LV_2, "old phy ctrl = ", oldPhyCtrl);
    zm_msg2_tx(ZM_LV_2, "new phy ctrl = ", phyCtrl);
	
    
}


#define EXTRA_INFO_LEN      24    
u16_t zfHpSend(zdev_t* dev, u16_t* header, u16_t headerLen,
                u16_t* snap, u16_t snapLen,
                u16_t* tail, u16_t tailLen, zbuf_t* buf, u16_t offset,
                u16_t bufType, u8_t ac, u8_t keyIdx)
{
#if ZM_SW_LOOP_BACK == 1
    zbuf_t *rxbuf;
    u8_t *puRxBuf;
    u8_t *pHdr;
	   u8_t *psnap;
	   u16_t plcplen = 12;
    u16_t i;
   	u16_t swlpOffset;
#endif 
    struct zsHpPriv* hpPriv;

    zmw_get_wlan_dev(dev);
    hpPriv=wd->hpPrivate;

    zm_msg1_tx(ZM_LV_1, "zfHpSend(), len = ", 12 + headerLen-8 + snapLen + zfwBufGetSize(dev, buf) + 4 + 8);

	
    zfAdjustCtrlSetting(dev, header, buf);

#if ZM_SW_LOOP_BACK != 1
    hpPriv->usbSendBytes += zfwBufGetSize(dev, buf);
    hpPriv->usbAcSendBytes[ac&0x3] += zfwBufGetSize(dev, buf);

    
    zfwUsbSend(dev, USB_ENDPOINT_TX_INDEX, (u8_t *)header, headerLen,
                  (u8_t *)snap, snapLen, (u8_t *)tail, tailLen, buf, offset);
#endif

#if ZM_SW_LOOP_BACK == 1

    rxbuf = zfwBufAllocate(dev, plcplen + headerLen-8 + snapLen + (zfwBufGetSize(dev, buf)-offset) + 4 + EXTRA_INFO_LEN);
    pHdr = (u8_t *) header+8;
   	psnap = (u8_t *) snap;

    zmw_enter_critical_section(dev);
    
    
   	swlpOffset = plcplen;

    for(i = 0; i < headerLen-8; i++)
    {
        zmw_rx_buf_writeb(dev, rxbuf, swlpOffset+i, pHdr[i]);
    }

   	swlpOffset += headerLen-8;

    
    for(i = 0; i < snapLen; i++)
    {
		      zmw_rx_buf_writeb(dev, rxbuf, swlpOffset+i, psnap[i]);
    }

	   swlpOffset += snapLen;

    
    for(i = 0; i < (zfwBufGetSize(dev, buf)-offset); i++)
    {
        u8_t value = zmw_rx_buf_readb(dev, buf, i+offset);
        zmw_rx_buf_writeb(dev, rxbuf, swlpOffset+i, value);
    }

	   
	   
   	zfwSetBufSetSize(dev, rxbuf, swlpOffset + (zfwBufGetSize(dev, buf)-offset) + 4 + EXTRA_INFO_LEN );

    zmw_leave_critical_section(dev);

    zfwBufFree(dev, buf, 0);

	   
	   

    

#endif 

    return ZM_SUCCESS;
}


void zfHpQueryMonHalRxInfo(zdev_t* dev, u8_t *monHalRxInfo)
{
    zmw_get_wlan_dev(dev);
    zfMemoryCopy(monHalRxInfo,
                (u8_t*)&(((struct zsHpPriv*)wd->hpPrivate)->halRxInfo),
                sizeof(struct zsHalRxInfo));
}


u8_t zfIsDataFrame(zdev_t* dev, zbuf_t* buf)
{
    u8_t frameType;
    u8_t mpduInd;

    mpduInd = zmw_rx_buf_readb(dev, buf, zfwBufGetSize(dev, buf)-1);

    
    if ((mpduInd & 0x30) == 0x00 || (mpduInd & 0x30) == 0x20)
    {
        frameType = zmw_rx_buf_readb(dev, buf, 12);
    }
    else
    {
        frameType = zmw_rx_buf_readb(dev, buf, 0);
    }

    if((frameType & 0xf) == ZM_WLAN_DATA_FRAME)
        return 1;
    else
        return 0;
}

u32_t zfcConvertRateOFDM(zdev_t* dev, zbuf_t* buf)
{
    
    u32_t MCS = 0;

    switch(zmw_rx_buf_readb(dev, buf, 0)& 0xf)
    {
        case 0xb:
            MCS = 0x4;
            break;
        case 0xf:
            MCS = 0x5;
            break;
        case 0xa:
            MCS = 0x6;
            break;
        case 0xe:
            MCS = 0x7;
            break;
        case 0x9:
            MCS = 0x8;
            break;
        case 0xd:
            MCS = 0x9;
            break;
        case 0x8:
            MCS = 0xa;
            break;
        case 0xc:
            MCS = 0xb;
            break;
    }
    return MCS;
}

u16_t zfHpGetPayloadLen(zdev_t* dev,
                        zbuf_t* buf,
                        u16_t len,
                        u16_t plcpHdrLen,
                        u32_t *rxMT,
                        u32_t *rxMCS,
                        u32_t *rxBW,
                        u32_t *rxSG
                        )
{
    u8_t modulation,mpduInd;
    u16_t low, high, msb;
    s16_t payloadLen = 0;

    zmw_get_wlan_dev(dev);

    mpduInd = zmw_rx_buf_readb(dev, buf, len-1);
    modulation = zmw_rx_buf_readb(dev, buf, (len-1)) & 0x3;
    *rxMT = modulation;

    
    switch (modulation) {
    case 0: 
        low = zmw_rx_buf_readb(dev, buf, 2);
        high = zmw_rx_buf_readb(dev, buf, 3);
        payloadLen = (low | high << 8) - 4;
        if (wd->enableHALDbgInfo)
        {
            *rxMCS = zmw_rx_buf_readb(dev, buf, 0);
            *rxBW  = 0;
            *rxSG  = 0;
        }
        break;
    case 1: 
        low = zmw_rx_buf_readb(dev, buf, 0) >> 5;
        high = zmw_rx_buf_readb(dev, buf, 1);
        msb = zmw_rx_buf_readb(dev, buf, 2) & 0x1;
        payloadLen = (low | (high << 3) | (msb << 11)) - 4;
        if (wd->enableHALDbgInfo)
        {
            *rxMCS = zfcConvertRateOFDM(dev, buf);
            *rxBW  = 0;
            *rxSG  = 0;
        }
        break;
    case 2: 
        
        if ((mpduInd & 0x30) == 0x00 || (mpduInd & 0x30) == 0x10)    
            payloadLen = len - 24 - 4 - plcpHdrLen;  
        else {
            payloadLen = len - 4 - 4 - plcpHdrLen;  
            
        }
        if (wd->enableHALDbgInfo)
        {
            *rxMCS = zmw_rx_buf_readb(dev, buf, 3) & 0x7f;
            *rxBW  = (zmw_rx_buf_readb(dev, buf, 3) >> 7) & 0x1;
            *rxSG  = (zmw_rx_buf_readb(dev, buf, 6) >> 7) & 0x1;
        }
        break;
    default:
        break;

    }
    
    if (payloadLen < 0) payloadLen = 0;
    return payloadLen;
}
















#define ZM_INT_USE_EP2                1
#define ZM_INT_USE_EP2_HEADER_SIZE   12

#if ZM_INT_USE_EP2 == 1
void zfiUsbRegIn(zdev_t* dev, u32_t* rsp, u16_t rspLen);
#endif

#ifdef ZM_OTUS_RX_STREAM_MODE
void zfiUsbRecvPerPkt(zdev_t *dev, zbuf_t *buf)
#else
void zfiUsbRecv(zdev_t *dev, zbuf_t *buf)
#endif
{


#if ZM_FW_LOOP_BACK != 1
    u8_t mpduInd;
    u16_t plcpHdrLen;
    u16_t crcPlusRxStatusLen;
    u16_t len, payloadLen=0;
    u16_t i; 
    struct zsAdditionInfo addInfo;
    u32_t               rxMT;
    u32_t               rxMCS;
    u32_t               rxBW;
    u32_t               rxSG;
    struct zsHpPriv* hpPriv;

    zmw_get_wlan_dev(dev);
    hpPriv=wd->hpPrivate;

    

#if ZM_INT_USE_EP2 == 1

    for (i=0; i<(ZM_INT_USE_EP2_HEADER_SIZE>>1); i++)
    {
        if (zmw_rx_buf_readh(dev, buf, i*2) != 0xffff)
        	break;
    }

    if (i==(ZM_INT_USE_EP2_HEADER_SIZE>>1))
    {
        u32_t               rsp[ZM_USB_MAX_EPINT_BUFFER/4];
        u16_t               rspLen;
        u32_t               rspi;
        u8_t*               pdst = (u8_t*)rsp;

        
        rspLen = (u16_t) zfwBufGetSize(dev, buf)-ZM_INT_USE_EP2_HEADER_SIZE;

        if (rspLen > 60)
        {
            zm_debug_msg1("Get error len by EP2 = \n", rspLen);
            
	          zfwBufFree(dev, buf, 0);
	          return;
        }

        for (rspi=0; rspi<rspLen; rspi++)
        {
        	*pdst = zmw_rx_buf_readb(dev, buf, rspi+ZM_INT_USE_EP2_HEADER_SIZE);
        	pdst++;
        }

        
        
        zfiUsbRegIn(dev, rsp, rspLen);

	      
	      zfwBufFree(dev, buf, 0);
	      return;
    }
#endif 

    ZM_PERFORMANCE_RX_MPDU(dev, buf);

    if (wd->swSniffer)
    {
        
        if (wd->zfcbRecv80211 != NULL)
        {
            wd->zfcbRecv80211(dev, buf, NULL);
        }
    }

    
    len = zfwBufGetSize(dev, buf);
    mpduInd = zmw_rx_buf_readb(dev, buf, len-1);

    
    if((mpduInd & 0x30) == 0x20)
    {
        u16_t duration;
        if (zmw_rx_buf_readb(dev, buf, 36) == 0) 
        {
            duration = zmw_rx_buf_readh(dev, buf, 14);
            if (duration > hpPriv->aggMaxDurationBE)
            {
                hpPriv->aggMaxDurationBE = duration;
            }
            else
            {
                if (hpPriv->aggMaxDurationBE > 10)
                {
                    hpPriv->aggMaxDurationBE--;
                }
            }
            
        }
    }

#if 1
    
    if(((mpduInd & 0x30) == 0x00) || ((mpduInd & 0x30) == 0x20))
    
    {
        plcpHdrLen = 12;        
    }
    else
    {
        if (zmw_rx_buf_readh(dev, buf, 4) == wd->macAddr[0] &&
            zmw_rx_buf_readh(dev, buf, 6) == wd->macAddr[1] &&
            zmw_rx_buf_readh(dev, buf, 8) == wd->macAddr[2]) {
            plcpHdrLen = 0;
        }
        else if (zmw_rx_buf_readh(dev, buf, 16) == wd->macAddr[0] &&
                 zmw_rx_buf_readh(dev, buf, 18) == wd->macAddr[1] &&
                 zmw_rx_buf_readh(dev, buf, 20) == wd->macAddr[2]){
            plcpHdrLen = 12;
        }
        else {
            plcpHdrLen = 0;
        }
    }

    
    if ((mpduInd & 0x30) == 0x00 || (mpduInd & 0x30) == 0x10)
    {
        crcPlusRxStatusLen = EXTRA_INFO_LEN + 4;     
    }
    else
    {
        crcPlusRxStatusLen = 4 + 4;     
    }
#else
    plcpHdrLen = 12;
    crcPlusRxStatusLen = EXTRA_INFO_LEN + 4;     
#endif

    if (len < (plcpHdrLen+10+crcPlusRxStatusLen))
    {
        zm_msg1_rx(ZM_LV_0, "Invalid Rx length=", len);
        

        zfwBufFree(dev, buf, 0);
        return;
    }

    
    

    

    payloadLen = zfHpGetPayloadLen(dev, buf, len, plcpHdrLen, &rxMT, &rxMCS, &rxBW, &rxSG);

    
    
    if(((mpduInd & 0x30) == 0x00) || ((mpduInd & 0x30) == 0x20))
    {
        if (wd->enableHALDbgInfo && zfIsDataFrame(dev, buf))
        {
            ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxDataMT   = rxMT;
            ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxDataMCS  = rxMCS;
            ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxDataBW   = rxBW;
            ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxDataSG   = rxSG;
        }
    }

    if ((plcpHdrLen + payloadLen) > len) {
        zm_msg1_rx(ZM_LV_0, "Invalid payload length=", payloadLen);
        zfwBufFree(dev, buf, 0);
        return;
    }

    

#if 0
    for (i = 0; i < crcPlusRxStatusLen-4; i++)
    {
       addInfo.Tail.Byte[i] =
               zmw_rx_buf_readb(dev, buf, len - crcPlusRxStatusLen + 4 + i);
    }
#else


    
    
    if ((mpduInd & 0x30) == 0x00 || (mpduInd & 0x30) == 0x10)
    {
#define ZM_RX_RSSI_COMPENSATION     27
        u8_t zm_rx_rssi_compensation = ZM_RX_RSSI_COMPENSATION;

    	
        addInfo.Tail.Data.SignalStrength1 = zmw_rx_buf_readb(dev, buf,
                (len-1) - 17) + ((hpPriv->rxStrongRSSI == 1)?zm_rx_rssi_compensation:0);
#undef ZM_RX_RSSI_COMPENSATION

      

      
      
      if (wd->enableHALDbgInfo && zfIsDataFrame(dev, buf))
      {
            u8_t trssi;
            for (i=0; i<7; i++)
            {
                trssi = zmw_rx_buf_readb(dev, buf, (len-1) - 23 + i);
	            if (trssi&0x80)
	            {
                    trssi = ((~((u8_t)trssi) & 0x7f) + 1) & 0x7f;
                }
                ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRSSI[i] = trssi;

            }
          if (rxMT==2)
          {
            
            
            	  for (i=0; i<12; i++)
                    ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxEVM[i] =
                                       zmw_rx_buf_readb(dev, buf, (len-1) - 16 + i);
            
            
            
            
            
            
            
          }

          #if 0
          
            zm_dbg(("MT(%d) MCS(%d) BW(%d) SG(%d) RSSI:%d,%d,%d,%d,%d,%d,%d EVM:(%d,%d,%d,%d,%d,%d)(%d,%d,%d,%d,%d,%d)\n",
                       rxMT,
                       rxMCS,
                       rxBW,
                       rxSG,
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRSSI[0],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRSSI[1],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRSSI[2],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRSSI[3],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRSSI[4],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRSSI[5],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRSSI[6],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxEVM[0],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxEVM[1],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxEVM[2],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxEVM[3],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxEVM[4],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxEVM[5],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxEVM[6],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxEVM[7],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxEVM[8],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxEVM[9],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxEVM[10],
                       ((struct zsHpPriv*)wd->hpPrivate)->halRxInfo.currentRxEVM[11]
                       ));
          #endif
      } 

    }
    else
    {
        
        addInfo.Tail.Data.SignalStrength1 = 0;
    }

    addInfo.Tail.Data.SignalStrength2 = 0;
    addInfo.Tail.Data.SignalStrength3 = 0;
    addInfo.Tail.Data.SignalQuality   = 0;

    addInfo.Tail.Data.SAIndex           = zmw_rx_buf_readb(dev, buf, len - 4);
    addInfo.Tail.Data.DAIndex           = zmw_rx_buf_readb(dev, buf, len - 3);
    addInfo.Tail.Data.ErrorIndication   = zmw_rx_buf_readb(dev, buf, len - 2);
    addInfo.Tail.Data.RxMacStatus       = zmw_rx_buf_readb(dev, buf, len - 1);

#endif
    
    zfwBufSetSize(dev, buf, (len-crcPlusRxStatusLen));
    

    
    if (plcpHdrLen != 0)
    {
        for (i = 0; i < plcpHdrLen; i++)
        {
            addInfo.PlcpHeader[i] = zmw_rx_buf_readb(dev, buf, i);
        }
    }
    else
    {
        addInfo.PlcpHeader[0] = 0;
    }
    
    zfwBufRemoveHead(dev, buf, plcpHdrLen);

    
    zfCoreRecv(dev, buf, &addInfo);

#else
    
    
    zbuf_t *new_buf;
    u8_t    ctrl_offset = 8;
    u8_t    PLCP_Len = 12;
    u8_t    data;
    u8_t    i;


    
    

    
    

    
    new_buf = zfwBufAllocate(dev, zfwBufGetSize(dev, buf)-8+12+4+EXTRA_INFO_LEN);

    for (i=0; i<zfwBufGetSize(dev, buf)-ctrl_offset; i++)
    {
        data = zmw_rx_buf_readb(dev, buf, ctrl_offset+i);
        zmw_rx_buf_writeb(dev, new_buf, PLCP_Len+i, data);
    }

    zfwBufSetSize(dev, new_buf, zfwBufGetSize(dev, buf)-8+12+4+EXTRA_INFO_LEN);

    zfwBufFree(dev, buf, 0);

    
    

#endif

}

#ifdef ZM_OTUS_RX_STREAM_MODE
void zfiUsbRecv(zdev_t *dev, zbuf_t *buf)
{
    u16_t index = 0;
    u16_t chkIdx;
    u32_t status = 0;
    u16_t ii;
    zbuf_t *newBuf;
    zbuf_t *rxBufPool[8];
    u16_t rxBufPoolIndex = 0;
    struct zsHpPriv *halPriv;
    u8_t *srcBufPtr;
    u32_t bufferLength;
    u16_t usbRxRemainLen;
    u16_t usbRxPktLen;

    zmw_get_wlan_dev(dev);

    halPriv = (struct zsHpPriv*)wd->hpPrivate;
    srcBufPtr = zmw_buf_get_buffer(dev, buf);

    bufferLength = zfwBufGetSize(dev, buf);

    
    if (!bufferLength)
    {
        zfwBufFree(dev, buf, 0);
        return;
    }

    usbRxRemainLen = halPriv->usbRxRemainLen;
    usbRxPktLen = halPriv->usbRxTransferLen;

    
    if (usbRxRemainLen != 0 )
    {
        zbuf_t *remainBufPtr = halPriv->remainBuf;
        u8_t* BufPtr = NULL;

        if ( remainBufPtr != NULL )
        {
            BufPtr = zmw_buf_get_buffer(dev, remainBufPtr);
        }

        index = usbRxRemainLen;
        usbRxRemainLen -= halPriv->usbRxPadLen;

        
        if ( BufPtr != NULL )
        {
            zfwMemoryCopy(&(BufPtr[usbRxPktLen]), srcBufPtr, usbRxRemainLen);
        }

        usbRxPktLen += usbRxRemainLen;
        halPriv->usbRxRemainLen = 0;

        if ( remainBufPtr != NULL )
        {
            zfwBufSetSize(dev, remainBufPtr, usbRxPktLen);
            rxBufPool[rxBufPoolIndex++] = remainBufPtr;
        }
        halPriv->remainBuf = NULL;
    }

    

    bufferLength = zfwBufGetSize(dev, buf);

    while(index < bufferLength)
    {
        u16_t pktLen;
        u16_t pktTag;
        
        u8_t *ptr = srcBufPtr;

        
        pktLen = ptr[index] + (ptr[index+1] << 8);
        pktTag = ptr[index+2] + (ptr[index+3] << 8);

        if (pktTag == ZM_USB_STREAM_MODE_TAG)
        {
            u16_t padLen;

            zm_assert(pktLen < ZM_WLAN_MAX_RX_SIZE);

            
            #if 0
            
            for (ii = index; ii < pkt_len+4;)
            {
                DbgPrint("0x%02x ",
                        (zmw_rx_buf_readb(adapter, pUsbRxTransfer->buf, ii) & 0xff));

                if ((++ii % 16) == 0)
                    DbgPrint("\n");
            }

            DbgPrint("\n");
            #endif

            
            padLen = ZM_USB_STREAM_MODE_TAG_LEN - (pktLen & 0x3);

            if(padLen == ZM_USB_STREAM_MODE_TAG_LEN)
                padLen = 0;

            chkIdx = index;
            index = index + ZM_USB_STREAM_MODE_TAG_LEN + pktLen + padLen;

            if (chkIdx > ZM_MAX_USB_IN_TRANSFER_SIZE)
            {
                zm_debug_msg1("chkIdx is too large, chkIdx: %d\n", chkIdx);
                zm_assert(0);
                status = 1;
                break;
            }

            if (index > ZM_MAX_USB_IN_TRANSFER_SIZE)
            {
                
                
                u8_t *BufPtr;
                u8_t *UsbBufPtr;

                halPriv->usbRxRemainLen = index - ZM_MAX_USB_IN_TRANSFER_SIZE; 
                halPriv->usbRxTransferLen = ZM_MAX_USB_IN_TRANSFER_SIZE -
                        chkIdx - ZM_USB_STREAM_MODE_TAG_LEN;
                halPriv->usbRxPadLen = padLen;
                

                if (halPriv->usbRxTransferLen > ZM_WLAN_MAX_RX_SIZE)
                {
                    zm_debug_msg1("check_len is too large, chk_len: %d\n",
                            halPriv->usbRxTransferLen);
                    status = 1;
                    break;
                }

                
                newBuf = zfwBufAllocate(dev, ZM_WLAN_MAX_RX_SIZE);

                if ( newBuf != NULL )
                {
                    BufPtr = zmw_buf_get_buffer(dev, newBuf);
                    UsbBufPtr = srcBufPtr;

                    
                    zfwMemoryCopy(BufPtr, &(UsbBufPtr[chkIdx+ZM_USB_STREAM_MODE_TAG_LEN]), halPriv->usbRxTransferLen);

                    
                    halPriv->remainBuf = newBuf;
                }
            }
            else
            {
                u8_t* BufPtr;
                u8_t* UsbBufPtr;

                
                newBuf = zfwBufAllocate(dev, ZM_WLAN_MAX_RX_SIZE);
                if ( newBuf != NULL )
                {
                    BufPtr = zmw_buf_get_buffer(dev, newBuf);
                    UsbBufPtr = srcBufPtr;

                    
                    zfwMemoryCopy(BufPtr, &(UsbBufPtr[chkIdx+ZM_USB_STREAM_MODE_TAG_LEN]), pktLen);

                    zfwBufSetSize(dev, newBuf, pktLen);
                    rxBufPool[rxBufPoolIndex++] = newBuf;
                }
            }
        }
        else
        {
                u16_t i;

                DbgPrint("Can't find tag, pkt_len: 0x%04x, tag: 0x%04x\n",
                        pktLen, pktTag);

                #if 0
                for(i = 0; i < 32; i++)
                {
                    DbgPrint("%02x ", buf->data[index-16+i]);

                    if ((i & 0xf) == 0xf)
                        DbgPrint("\n");
                }
                #endif

                break;
        }
    }

    
    
    zfwBufFree(dev, buf, 0);

    for(ii = 0; ii < rxBufPoolIndex; ii++)
    {
        zfiUsbRecvPerPkt(dev, rxBufPool[ii]);
    }
}
#endif
















void zfUsbInit(zdev_t* dev)
{
    
    zfwUsbEnableRxEpt(dev, USB_ENDPOINT_RX_INDEX);
    zfwUsbEnableIntEpt(dev, USB_ENDPOINT_INT_INDEX);

    return;
}

















void zfUsbFree(zdev_t* dev)
{
    struct zsHpPriv *halPriv;

    zmw_get_wlan_dev(dev);

    halPriv = (struct zsHpPriv*)wd->hpPrivate;

#ifdef ZM_OTUS_RX_STREAM_MODE
    if ( halPriv->remainBuf != NULL )
    {
        zfwBufFree(dev, halPriv->remainBuf, 0);
    }
#endif

    return;
}

void zfHpSendBeacon(zdev_t* dev, zbuf_t* buf, u16_t len)
{
    u32_t hw, lw;
    u16_t i;
    zmw_get_wlan_dev(dev);

    
    for (i = 0; i<len; i+=4)
    {
        lw = zmw_tx_buf_readh(dev, buf, i);
        hw = zmw_tx_buf_readh(dev, buf, i+2);

        zfDelayWriteInternalReg(dev, ZM_BEACON_BUFFER_ADDRESS+i, (hw<<16)+lw);
    }

    
    if (((struct zsHpPriv*)wd->hpPrivate)->hwFrequency < 3000)
    {
    zfDelayWriteInternalReg(dev, ZM_MAC_REG_BCN_PLCP, ((len+4)<<(3+16))+0x0400);
    }
    else
    {
        zfDelayWriteInternalReg(dev, ZM_MAC_REG_BCN_PLCP, ((len+4)<<(16))+0x001b);
    }

    
    zfDelayWriteInternalReg(dev, ZM_MAC_REG_BCN_LENGTH, len+4);

    
    zfDelayWriteInternalReg(dev, ZM_MAC_REG_BCN_CTRL, 1);
    zfFlushDelayWrite(dev);

    
    zfwBufFree(dev, buf, 0);

    return;
}


#define ZM_STATUS_TX_COMP       0x00
#define ZM_STATUS_RETRY_COMP    0x01
#define ZM_STATUS_TX_FAILED     0x02
void zfiUsbRegIn(zdev_t* dev, u32_t* rsp, u16_t rspLen)
{
    
    u8_t type;
    u8_t *u8rsp;
    u16_t status;
    u32_t bitmap;
    zmw_get_wlan_dev(dev);

    zm_msg0_mm(ZM_LV_3, "zfiUsbRegIn()");

    u8rsp = (u8_t *)rsp;

    
    type = *(u8rsp+1);
    u8rsp = u8rsp+4;


    
    if ((type & 0xC0) == 0xC0)
    {
        if (type == 0xC0)
        {
            zfCoreEvent(dev, 0, u8rsp);

        }
        else if (type == 0xC1)
        {
#if 0
            {
                u16_t i;
                DbgPrint("rspLen=%d\n", rspLen);
                for (i=0; i<(rspLen/4); i++)
                {
                    DbgPrint("rsp[%d]=0x%lx\n", i, rsp[i]);
                }
            }
#endif
            status = (u16_t)(rsp[3] >> 16);

            
            rsp[8] = rsp[8] >> 2 | (rsp[9] & 0x1) << 6;
            switch (status)
            {
            case ZM_STATUS_RETRY_COMP :
                zfCoreEvent(dev, 1, u8rsp);
                break;
            case ZM_STATUS_TX_FAILED :
                zfCoreEvent(dev, 2, u8rsp);
                break;
            case ZM_STATUS_TX_COMP :
                zfCoreEvent(dev, 3, u8rsp);
                break;
            }
        }
        else if (type == 0xC2)
        {
            zfBeaconCfgInterrupt(dev, u8rsp);
        }
        else if (type == 0xC3)
        {
            zfEndOfAtimWindowInterrupt(dev);
        }
        else if (type == 0xC4)
        {
#if 0
            {
                u16_t i;
                DbgPrint("0xC2:rspLen=%d\n", rspLen);
                for (i=0; i<(rspLen/4); i++)
                {
                    DbgPrint("0xC2:rsp[%d]=0x%lx\n", i, rsp[i]);
                }
            }
#endif
            bitmap = (rsp[1] >> 16) + ((rsp[2] & 0xFFFF) << 16 );
            
        }
        else if (type == 0xC5)
        {
            u16_t i;
#if 0

            for (i=0; i<(rspLen/4); i++) {
                DbgPrint("0xC5:rsp[%d]=0x%lx\n", i, rsp[i]);
            }
#endif
            for (i=1; i<(rspLen/4); i++) {
                u8rsp = (u8_t *)(rsp+i);
                
                zfCoreEvent(dev, 4, u8rsp);
            }
        }
        else if (type == 0xC6)
        {
            zm_debug_msg0("\n\n WatchDog interrupt!!! : 0xC6 \n\n");
            if (wd->zfcbHwWatchDogNotify != NULL)
            {
                wd->zfcbHwWatchDogNotify(dev);
            }
        }
        else if (type == 0xC8)
        {
            

            
            zfwDbgProgrameFlashChkDone(dev);
        }
        else if (type == 0xC9)
        {
            struct zsHpPriv* hpPriv=wd->hpPrivate;

            zm_debug_msg0("##### Tx retransmission 5 times event #####");

            
            hpPriv->retransmissionEvent = 1;
        }
    }
    else
    {
        zfIdlRsp(dev, rsp, rspLen);
    }
}


#define ZM_PROGRAM_RAM_ADDR     0x200000 
#define FIRMWARE_DOWNLOAD       0x30
#define FIRMWARE_DOWNLOAD_COMP  0x31
#define FIRMWARE_CONFIRM        0x32

u16_t zfFirmwareDownload(zdev_t* dev, u32_t* fw, u32_t len, u32_t offset)
{
    u16_t ret = ZM_SUCCESS;
    u32_t uCodeOfst = offset;
    u8_t *image, *ptr;
    u32_t result;

    image = (u8_t*) fw;
    ptr = image;

    while (len > 0)
    {
        u32_t translen = (len > 4096) ? 4096 : len;

        result = zfwUsbSubmitControl(dev, FIRMWARE_DOWNLOAD,
                                     (u16_t) (uCodeOfst >> 8),
                                     0, image, translen);

        if (result != ZM_SUCCESS)
        {
            zm_msg0_init(ZM_LV_0, "FIRMWARE_DOWNLOAD failed");
            ret = 1;
            goto exit;
        }

        len -= translen;
        image += translen;
        uCodeOfst += translen; 

        result = 0;
    }

    
    if (ret == 0)
    {
        result = zfwUsbSubmitControl(dev, FIRMWARE_DOWNLOAD_COMP,
                                     0, 0, NULL, 0);

        if (result != ZM_SUCCESS)
        {
            zm_msg0_init(ZM_LV_0, "FIRMWARE_DOWNLOAD_COMP failed");
            ret = 1;
            goto exit;
        }
    }

#if 0
    
    
    result = zfwUsbSubmitControl(dev, FIRMWARE_CONFIRM, USB_DIR_IN | 0x40,
                     0, 0, &ret_value, sizeof(ret_value), HZ);

    if (result != 0)
    {
        zm_msg0_init(ZM_LV_0, "Can't receive firmware ready: ", result);
        ret = 1;
    }
#endif

exit:

    return ret;

}

u16_t zfFirmwareDownloadNotJump(zdev_t* dev, u32_t* fw, u32_t len, u32_t offset)
{
    u16_t ret = ZM_SUCCESS;
    u32_t uCodeOfst = offset;
    u8_t *image, *ptr;
    u32_t result;

    image = (u8_t*) fw;
    ptr = image;

    while (len > 0)
    {
        u32_t translen = (len > 4096) ? 4096 : len;

        result = zfwUsbSubmitControl(dev, FIRMWARE_DOWNLOAD,
                                     (u16_t) (uCodeOfst >> 8),
                                     0, image, translen);

        if (result != ZM_SUCCESS)
        {
            zm_msg0_init(ZM_LV_0, "FIRMWARE_DOWNLOAD failed");
            ret = 1;
            goto exit;
        }

        len -= translen;
        image += translen;
        uCodeOfst += translen; 

        result = 0;
    }

exit:

    return ret;

}
















u32_t zfHpGetFreeTxdCount(zdev_t* dev)
{
    return zfwUsbGetFreeTxQSize(dev);
}

u32_t zfHpGetMaxTxdCount(zdev_t* dev)
{
    
    return zfwUsbGetMaxTxQSize(dev);
}

void zfiUsbRegOutComplete(zdev_t* dev)
{
    return;
}

extern void zfPushVtxq(zdev_t* dev);

void zfiUsbOutComplete(zdev_t* dev, zbuf_t *buf, u8_t status, u8_t *hdr) {
#ifndef ZM_ENABLE_AGGREGATION
    if (buf) {
        zfwBufFree(dev, buf, 0);
    }
#else
    #ifdef ZM_BYPASS_AGGR_SCHEDULING
    
    if (buf)
    {
        zfwBufFree(dev, buf, 0);
    }
    zfPushVtxq(dev);
    #else
    zmw_get_wlan_dev(dev);

    #ifdef ZM_ENABLE_FW_BA_RETRANSMISSION
    
    if (buf)
    {
        zfwBufFree(dev, buf, 0);
    }
    #else
    u8_t agg;
    u16_t frameType;

    if(!hdr && buf) {
        zfwBufFree(dev, buf, 0);
        
        return;
    }

    if(hdr && buf) {
        frameType = hdr[8] & 0xf;
        agg = (u8_t)(hdr[2] >> 5 ) & 0x1;
        

        if (!status) {
            if (agg) {
                
                
            }
            else {
                zfwBufFree(dev, buf, 0);
            }
        }
        else {
            if (agg) {
                
                
            }
            else {
                zfwBufFree(dev, buf, 0);
            }
        }
    }
    #endif

    if (wd->state != ZM_WLAN_STATE_ENABLED) {
        return;
    }

    if( (wd->wlanMode == ZM_MODE_AP) ||
        (wd->wlanMode == ZM_MODE_INFRASTRUCTURE && wd->sta.EnableHT) ||
        (wd->wlanMode == ZM_MODE_PSEUDO) ) {
        zfAggTxScheduler(dev, 0);
    }
    #endif
#endif

    return;

}

