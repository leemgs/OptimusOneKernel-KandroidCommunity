

#include "cprecomp.h"









































void zfLedCtrlType1(zdev_t* dev)
{
    u16_t i;
    u32_t ton, toff, tmp, period;
    zmw_get_wlan_dev(dev);

    for (i=0; i<ZM_MAX_LED_NUMBER; i++)
    {
        if (zfStaIsConnected(dev) != TRUE)
        {
            
            ton = ((wd->ledStruct.ledMode[i] & 0xf00) >> 8) * 5;
            toff = ((wd->ledStruct.ledMode[i] & 0xf000) >> 12) * 5;

            if ((ton + toff) != 0)
            {
                tmp = wd->ledStruct.counter / (ton+toff);
                tmp = wd->ledStruct.counter - (tmp * (ton+toff));
                if (tmp < ton)
                {
                    zfHpLedCtrl(dev, i, 1);
                }
                else
                {
                    zfHpLedCtrl(dev, i, 0);
                }
            }
        }
        else
        {
            if ((zfPowerSavingMgrIsSleeping(dev)) && ((wd->ledStruct.ledMode[i] & 0x8) == 0))
            {
                zfHpLedCtrl(dev, i, 0);
            }
            else
            {
                
                if ((wd->ledStruct.ledMode[i] & 0x40) == 0)
                {
                    if ((wd->ledStruct.counter & 1) == 0)
                    {
                        zfHpLedCtrl(dev, i, (wd->ledStruct.ledMode[i] & 0x10) >> 4);
                    }
                    else
                    {
                        if ((wd->ledStruct.txTraffic > 0) || (wd->ledStruct.rxTraffic > 0))
                        {
                            wd->ledStruct.txTraffic = wd->ledStruct.rxTraffic = 0;
                            if ((wd->ledStruct.ledMode[i] & 0x20) != 0)
                            {
                                zfHpLedCtrl(dev, i, ((wd->ledStruct.ledMode[i] & 0x10) >> 4)^1);
                            }
                        }
                    }
                }
                else
                {
                    period = 5 * (1 << ((wd->ledStruct.ledMode[i] & 0x30) >> 4));
                    tmp = wd->ledStruct.counter / (period*2);
                    tmp = wd->ledStruct.counter - (tmp * (period*2));
                    if (tmp < period)
                    {
                        if ((wd->ledStruct.counter & 1) == 0)
                        {
                            zfHpLedCtrl(dev, i, 0);
                        }
                        else
                        {
                            if ((wd->ledStruct.txTraffic > 0) || (wd->ledStruct.rxTraffic > 0))
                            {
                                wd->ledStruct.txTraffic = wd->ledStruct.rxTraffic = 0;
                                zfHpLedCtrl(dev, i, 1);
                            }
                        }
                    }
                    else
                    {
                        if ((wd->ledStruct.counter & 1) == 0)
                        {
                            zfHpLedCtrl(dev, i, 1);
                        }
                        else
                        {
                            if ((wd->ledStruct.txTraffic > 0) || (wd->ledStruct.rxTraffic > 0))
                            {
                                wd->ledStruct.txTraffic = wd->ledStruct.rxTraffic = 0;
                                zfHpLedCtrl(dev, i, 0);
                            }
                        }
                    }
                } 
            } 
        } 
    } 
}
































void zfLedCtrlType2_scan(zdev_t* dev);

void zfLedCtrlType2(zdev_t* dev)
{
    u32_t ton, toff, tmp, period;
    u16_t OperateLED;
    zmw_get_wlan_dev(dev);

    if (zfStaIsConnected(dev) != TRUE)
    {
        
        if(wd->ledStruct.counter % 4 != 0)
    	{
      	    
      	    
            
            
            
            
            
            return;
        }

        if (((wd->state == ZM_WLAN_STATE_DISABLED) && (wd->sta.bChannelScan))
            || ((wd->state != ZM_WLAN_STATE_DISABLED) && (wd->sta.bAutoReconnect)))
        {
            
            zfLedCtrlType2_scan(dev);
        }
        else
        {
            
            zfHpLedCtrl(dev, 0, 0);
            zfHpLedCtrl(dev, 1, 0);
        }
    }
    else
    {
        if( wd->sta.bChannelScan )
        {
            
            if(wd->ledStruct.counter % 4 != 0)
                return;
            zfLedCtrlType2_scan(dev);
            return;
        }

        if(wd->frequency < 3000)
        {
            OperateLED = 0;     
            zfHpLedCtrl(dev, 1, 0);
        }
        else
        {
            OperateLED = 1;     
            zfHpLedCtrl(dev, 0, 0);
        }

        if ((zfPowerSavingMgrIsSleeping(dev)) && ((wd->ledStruct.ledMode[OperateLED] & 0x8) == 0))
        {
            
            zfHpLedCtrl(dev, OperateLED, 0);
        }
        else
        {
            
            if ((wd->ledStruct.counter & 1) == 0)   
            {
                
                zfHpLedCtrl(dev, OperateLED, 1);
            }
            else       
            {
                if ((wd->ledStruct.txTraffic > 0) || (wd->ledStruct.rxTraffic > 0))
                {
                    
		            
		            
		            
		            
                    wd->ledStruct.txTraffic = wd->ledStruct.rxTraffic = 0;
                    zfHpLedCtrl(dev, OperateLED, 0);
                }
            }
        }
    }
}

void zfLedCtrlType2_scan(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    
    
    
    
    
    

    switch(wd->ledStruct.counter % 16)
    {
        case 0:   
            if(wd->supportMode & ZM_WIRELESS_MODE_24)
            {
                zfHpLedCtrl(dev, 0, 1);
                zfHpLedCtrl(dev, 1, 0);
            }
            else
            {
                zfHpLedCtrl(dev, 1, 1);
                zfHpLedCtrl(dev, 0, 0);
            }
            break;

        case 8:   
            if(wd->supportMode & ZM_WIRELESS_MODE_5)
            {
                zfHpLedCtrl(dev, 1, 1);
                zfHpLedCtrl(dev, 0, 0);
            }
            else
            {
                zfHpLedCtrl(dev, 0, 1);
                zfHpLedCtrl(dev, 1, 0);
            }
            break;

        default:  
            zfHpLedCtrl(dev, 0, 0);
            zfHpLedCtrl(dev, 1, 0);
            break;
    }
}

































void zfLedCtrlType3_scan(zdev_t* dev, u16_t isConnect);

void zfLedCtrlType3(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    if (zfStaIsConnected(dev) != TRUE)
    {
        
        if(wd->ledStruct.counter % 2 != 0)
    	{
      	    
      	    
            
            
            
            
            return;
        }

        if (((wd->state == ZM_WLAN_STATE_DISABLED) && (wd->sta.bChannelScan))
            || ((wd->state != ZM_WLAN_STATE_DISABLED) && (wd->sta.bAutoReconnect)))
        {
            
            zfLedCtrlType3_scan(dev, 0);
        }
        else
        {
            
            zfHpLedCtrl(dev, 0, 0);
            zfHpLedCtrl(dev, 1, 0);
        }
    }
    else
    {
        if( wd->sta.bChannelScan )
        {
            
            if(wd->ledStruct.counter % 2 != 0)
                return;
            zfLedCtrlType3_scan(dev, 1);
            return;
        }

        if ((zfPowerSavingMgrIsSleeping(dev)) && ((wd->ledStruct.ledMode[0] & 0x8) == 0))
        {
            
            zfHpLedCtrl(dev, 0, 0);
            zfHpLedCtrl(dev, 1, 0);
        }
        else
        {
            
            if ((wd->ledStruct.counter & 1) == 0)   
            {
                
                zfHpLedCtrl(dev, 0, 1);
                zfHpLedCtrl(dev, 1, 1);
            }
            else       
            {
                if ((wd->ledStruct.txTraffic > 0) || (wd->ledStruct.rxTraffic > 0))
                {
                    
		            
		            
		            
		            
                    wd->ledStruct.txTraffic = wd->ledStruct.rxTraffic = 0;
                    zfHpLedCtrl(dev, 0, 0);
                    zfHpLedCtrl(dev, 1, 0);
                }
            }
        }
    }
}

void zfLedCtrlType3_scan(zdev_t* dev, u16_t isConnect)
{
    u32_t ton, toff, tmp;
    zmw_get_wlan_dev(dev);

    
    
    
    
    
    
    
    
    
    
    

    
    if(!isConnect)
        ton = 2, toff = 6;
    else
        ton = 6, toff = 2;

    if ((ton + toff) != 0)
    {
        tmp = wd->ledStruct.counter % (ton+toff);
       if (tmp < ton)
        {
            zfHpLedCtrl(dev, 0, 1);
            zfHpLedCtrl(dev, 1, 1);
        }
        else
        {
            zfHpLedCtrl(dev, 0, 0);
            zfHpLedCtrl(dev, 1, 0);
        }
    }
}




















void zfLedCtrl_BlinkWhenScan_Alpha(zdev_t* dev)
{
    static u32_t counter = 0;
    zmw_get_wlan_dev(dev);

    if(counter > 34)        
    {
        wd->ledStruct.LEDCtrlFlag &= ~(u8_t)ZM_LED_CTRL_FLAG_ALPHA;
        counter = 0;
    }

    if( (counter % 3) < 2)
        zfHpLedCtrl(dev, 0, 1);
    else
        zfHpLedCtrl(dev, 0, 0);

    counter++;
}

















void zfLed100msCtrl(zdev_t* dev)
{
    zmw_get_wlan_dev(dev);

    wd->ledStruct.counter++;

    if(wd->ledStruct.LEDCtrlFlag)
    {
        switch(wd->ledStruct.LEDCtrlFlag) {
        case ZM_LED_CTRL_FLAG_ALPHA:
            zfLedCtrl_BlinkWhenScan_Alpha(dev);
        break;
        }
    }
    else
    {
        switch(wd->ledStruct.LEDCtrlType) {
        case 1:			
            zfLedCtrlType1(dev);
        break;

        case 2:			
            zfLedCtrlType2(dev);
        break;

        case 3:			
            zfLedCtrlType3(dev);
        break;

        default:
            zfLedCtrlType1(dev);
        break;
        }
    }
}

