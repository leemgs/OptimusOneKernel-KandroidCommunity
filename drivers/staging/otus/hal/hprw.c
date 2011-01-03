
#include "../80211core/cprecomp.h"
#include "hpani.h"
#include "hpusb.h"
#include "hpreg.h"
#include "../80211core/ratectrl.h"

extern void zfIdlCmd(zdev_t* dev, u32_t* cmd, u16_t cmdLen);

extern void zfCoreCwmBusy(zdev_t* dev, u16_t busy);
u16_t zfDelayWriteInternalReg(zdev_t* dev, u32_t addr, u32_t val);
u16_t zfFlushDelayWrite(zdev_t* dev);



void zfInitCmdQueue(zdev_t* dev)
{
    struct zsHpPriv* hpPriv;

    zmw_get_wlan_dev(dev);
    hpPriv = (struct zsHpPriv*)(wd->hpPrivate);

    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);
#ifdef ZM_XP_USB_MULTCMD
    hpPriv->cmdTail = hpPriv->cmdHead = hpPriv->cmdSend = 0;
#else
    hpPriv->cmdTail = hpPriv->cmdHead = 0;
#endif
    hpPriv->cmdPending = 0;
    hpPriv->cmd.delayWcmdCount = 0;
    zmw_leave_critical_section(dev);
}

u16_t zfPutCmd(zdev_t* dev, u32_t* cmd, u16_t cmdLen, u16_t src, u8_t* buf)
{
    u16_t i;
    struct zsHpPriv* hpPriv;

    zmw_get_wlan_dev(dev);
    hpPriv=wd->hpPrivate;

    
    zm_assert(cmdLen <= ZM_MAX_CMD_SIZE);
    
    
    if (((hpPriv->cmdTail+1) & (ZM_CMD_QUEUE_SIZE-1)) == hpPriv->cmdHead ) {
        zm_debug_msg0("CMD queue full!!");
        return 0;
    }

    hpPriv->cmdQ[hpPriv->cmdTail].cmdLen = cmdLen;
    hpPriv->cmdQ[hpPriv->cmdTail].src = src;
    hpPriv->cmdQ[hpPriv->cmdTail].buf = buf;
    for (i=0; i<(cmdLen>>2); i++)
    {
        hpPriv->cmdQ[hpPriv->cmdTail].cmd[i] = cmd[i];
    }

    hpPriv->cmdTail = (hpPriv->cmdTail+1) & (ZM_CMD_QUEUE_SIZE-1);

    return 0;
}

u16_t zfGetCmd(zdev_t* dev, u32_t* cmd, u16_t* cmdLen, u16_t* src, u8_t** buf)
{
    u16_t i;
    struct zsHpPriv* hpPriv;

    zmw_get_wlan_dev(dev);
    hpPriv=wd->hpPrivate;

    if (hpPriv->cmdTail == hpPriv->cmdHead)
    {
        return 3;
    }

    *cmdLen = hpPriv->cmdQ[hpPriv->cmdHead].cmdLen;
    *src = hpPriv->cmdQ[hpPriv->cmdHead].src;
    *buf = hpPriv->cmdQ[hpPriv->cmdHead].buf;
    for (i=0; i<((*cmdLen)>>2); i++)
    {
        cmd[i] = hpPriv->cmdQ[hpPriv->cmdHead].cmd[i];
    }

    hpPriv->cmdHead = (hpPriv->cmdHead+1) & (ZM_CMD_QUEUE_SIZE-1);

    return 0;
}

#ifdef ZM_XP_USB_MULTCMD
void zfSendCmdEx(zdev_t* dev)
{
    u32_t ncmd[ZM_MAX_CMD_SIZE/4];
    u16_t ncmdLen = 0;
    u16_t cmdFlag = 0;
    u16_t i;
    struct zsHpPriv* hpPriv;

    zmw_get_wlan_dev(dev);
    hpPriv=wd->hpPrivate;

    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);

    if (hpPriv->cmdPending == 0)
    {
        if (hpPriv->cmdTail != hpPriv->cmdSend)
        {
            cmdFlag = 1;
            
            ncmdLen= hpPriv->cmdQ[hpPriv->cmdSend].cmdLen;
            for (i=0; i<(ncmdLen>>2); i++)
            {
                ncmd[i] = hpPriv->cmdQ[hpPriv->cmdSend].cmd[i];
            }
            hpPriv->cmdSend = (hpPriv->cmdSend+1) & (ZM_CMD_QUEUE_SIZE-1);

            hpPriv->cmdPending = 1;
        }
    }

    zmw_leave_critical_section(dev);

    if ((cmdFlag == 1))
    {
        zfIdlCmd(dev, ncmd, ncmdLen);
    }
}

void zfiSendCmdComp(zdev_t* dev)
{
    struct zsHpPriv* hpPriv;

    zmw_get_wlan_dev(dev);
    hpPriv=wd->hpPrivate;

    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);
    hpPriv->cmdPending = 0;
    zmw_leave_critical_section(dev);

    zfSendCmdEx(dev);
}
#endif

u16_t zfIssueCmd(zdev_t* dev, u32_t* cmd, u16_t cmdLen, u16_t src, u8_t* buf)
{
    u16_t cmdFlag = 0;
    u16_t ret;
    struct zsHpPriv* hpPriv;

    zmw_get_wlan_dev(dev);
    hpPriv=wd->hpPrivate;

    zmw_declare_for_critical_section();

    zm_msg2_mm(ZM_LV_1, "cmdLen=", cmdLen);

    zmw_enter_critical_section(dev);

#ifdef ZM_XP_USB_MULTCMD
    ret = zfPutCmd(dev, cmd, cmdLen, src, buf);
    zmw_leave_critical_section(dev);

    if (ret != 0)
    {
        return 1;
    }

    zfSendCmdEx(dev);
#else
    if (hpPriv->cmdPending == 0)
    {
        hpPriv->cmdPending = 1;
        cmdFlag = 1;
    }
    ret = zfPutCmd(dev, cmd, cmdLen, src, buf);

    zmw_leave_critical_section(dev);

    if (ret != 0)
    {
        return 1;
    }

    if (cmdFlag == 1)
    {
        zfIdlCmd(dev, cmd, cmdLen);
    }
#endif
    return 0;
}

void zfIdlRsp(zdev_t* dev, u32_t* rsp, u16_t rspLen)
{
    u32_t cmd[ZM_MAX_CMD_SIZE/4];
    u16_t cmdLen;
    u16_t src;
    u8_t* buf;
    u32_t ncmd[ZM_MAX_CMD_SIZE/4];
    u16_t ncmdLen = 0;
    u16_t ret;
    u16_t cmdFlag = 0;
    u16_t i;
    s32_t nf;
    s32_t noisefloor[4];
    struct zsHpPriv* hpPriv;

    zmw_get_wlan_dev(dev);
    hpPriv=wd->hpPrivate;


    zmw_declare_for_critical_section();

    zmw_enter_critical_section(dev);

    ret = zfGetCmd(dev, cmd, &cmdLen, &src, &buf);
    #if 0
    zm_assert(ret == 0);
    #else
    if (ret != 0)
    {
        zm_debug_msg0("Error IdlRsp because none cmd!!\n");
        #ifndef ZM_XP_USB_MULTCMD
        zmw_leave_critical_section(dev);
        return;
        #endif
    }
    #endif
#ifdef ZM_XP_USB_MULTCMD
    zmw_leave_critical_section(dev);
#else
    if (hpPriv->cmdTail != hpPriv->cmdHead)
    {
        cmdFlag = 1;
        
        ncmdLen= hpPriv->cmdQ[hpPriv->cmdHead].cmdLen;
        for (i=0; i<(ncmdLen>>2); i++)
        {
            ncmd[i] = hpPriv->cmdQ[hpPriv->cmdHead].cmd[i];
        }
    }
    else
    {
        hpPriv->cmdPending = 0;
    }

    zmw_leave_critical_section(dev);

    if (cmdFlag == 1)
    {
        zfIdlCmd(dev, ncmd, ncmdLen);
    }
#endif
    if (src == ZM_OID_READ)
    {
        ZM_PERFORMANCE_REG(dev, 0x11772c, rsp[1]);
        zfwDbgReadRegDone(dev, cmd[1], rsp[1]);
    }
    else if (src == ZM_OID_FLASH_CHKSUM)
    {
        zfwDbgGetFlashChkSumDone(dev, rsp+1);
    }
    else if (src == ZM_OID_FLASH_READ)
    {
        u32_t  datalen;
        u16_t i;

        datalen = (rsp[0] & 255);

        zfwDbgReadFlashDone(dev, cmd[1], rsp+1, datalen);
    }
    else if (src == ZM_OID_FLASH_PROGRAM)
    {
        
    }
    else if (src == ZM_OID_WRITE)
    {
        zfwDbgWriteRegDone(dev, cmd[1], cmd[2]);
    }
    else if (src == ZM_OID_TALLY)
    {
		zfCollectHWTally(dev, rsp, 0);
    }
    else if (src == ZM_OID_TALLY_APD)
    {
		zfCollectHWTally(dev, rsp, 1);
        zfwDbgReadTallyDone(dev);
#ifdef ZM_ENABLE_BA_RATECTRL
        zfRateCtrlAggrSta(dev);
#endif
    }
    else if (src == ZM_OID_DKTX_STATUS)
    {
        zm_debug_msg0("src = zm_OID_DKTX_STATUS");
        zfwDbgQueryHwTxBusyDone(dev, rsp[1]);
    }
    else if (src == ZM_CMD_SET_FREQUENCY)
    {


#if 0
    zm_debug_msg1("Retry Set Frequency = ", rsp[1]);

    #if 1
    
    nf = ((rsp[2]>>19) & 0x1ff);
    if ((nf & 0x100) != 0x0)
    {
        noisefloor[0] = 0 - ((nf ^ 0x1ff) + 1);
    }
    else
    {
        noisefloor[0] = nf;
    }

    zm_debug_msg1("Noise Floor[1] = ", noisefloor[0]);

    nf = ((rsp[3]>>19) & 0x1ff);
    if ((nf & 0x100) != 0x0)
    {
        noisefloor[1] = 0 - ((nf ^ 0x1ff) + 1);
    }
    else
    {
        noisefloor[1] = nf;
    }

    zm_debug_msg1("Noise Floor[2] = ", noisefloor[1]);
    zm_debug_msg1("Is Site Survey = ", hpPriv->isSiteSurvey);
    #endif

        if ( (rsp[1] && hpPriv->freqRetryCounter == 0) ||
             (((noisefloor[0]>-60)||(noisefloor[1]>-60)) && hpPriv->freqRetryCounter==0) ||
             ((abs(noisefloor[0]-noisefloor[1])>=9) && hpPriv->freqRetryCounter==0) )
        {
            zm_debug_msg0("Retry to issue the frequency change command");

            if ( hpPriv->recordFreqRetryCounter == 1 )
            {
                zm_debug_msg0("Cold Reset");

                zfHpSetFrequencyEx(dev, hpPriv->latestFrequency,
                                        hpPriv->latestBw40,
                                        hpPriv->latestExtOffset,
                                        2);

                if ( hpPriv->isSiteSurvey != 2 )
                {
                    hpPriv->freqRetryCounter++;
                }
                hpPriv->recordFreqRetryCounter = 0;
            }
            else
            {
                zfHpSetFrequencyEx(dev, hpPriv->latestFrequency,
                                        hpPriv->latestBw40,
                                        hpPriv->latestExtOffset,
                                        0);
            }
            hpPriv->recordFreqRetryCounter++;
        }
        else
#endif




        if ( (rsp[1] & 0x1) || (rsp[1] & 0x4) )
        {
            zm_debug_msg1("Set Frequency fail : ret = ", rsp[1]);

            
            
            
            if ( hpPriv->isSiteSurvey == 2 )
            {
                if ( hpPriv->recordFreqRetryCounter < 2 )
                {
                    
                    zfHpSetFrequencyEx(dev, hpPriv->latestFrequency,
                                            hpPriv->latestBw40,
                                            hpPriv->latestExtOffset,
                                            2);
                    hpPriv->recordFreqRetryCounter++;
                    zm_debug_msg1("Retry to issue the frequency change command(cold reset) counter = ", hpPriv->recordFreqRetryCounter);
                }
                else
                {
                    
                    zm_debug_msg0("\n\n\n\n  Fail twice cold reset \n\n\n\n");
                    hpPriv->coldResetNeedFreq = 0;
                    hpPriv->recordFreqRetryCounter = 0;
                    zfCoreSetFrequencyComplete(dev);
                }
            }
            else
            {
                
                hpPriv->coldResetNeedFreq = 1;
                hpPriv->recordFreqRetryCounter = 0;
                zfCoreSetFrequencyComplete(dev);
            }
        }
        else if (rsp[1] & 0x2)
        {
            zm_debug_msg1("Set Frequency fail 2 : ret = ", rsp[1]);

            
            
            if ( hpPriv->isSiteSurvey == 2 )
            {
                if ( hpPriv->recordFreqRetryCounter < 1 )
                {
                    
                    zfHpSetFrequencyEx(dev, hpPriv->latestFrequency,
                                            hpPriv->latestBw40,
                                            hpPriv->latestExtOffset,
                                            2);
                    hpPriv->recordFreqRetryCounter++;
                    zm_debug_msg1("2 Retry to issue the frequency change command(cold reset) counter = ", hpPriv->recordFreqRetryCounter);
                }
                else
                {
                    
                    zm_debug_msg0("\n\n\n\n  2 Fail twice cold reset \n\n\n\n");
                    hpPriv->coldResetNeedFreq = 0;
                    hpPriv->recordFreqRetryCounter = 0;
                    zfCoreSetFrequencyComplete(dev);
                }
            }
            else
            {
                
                hpPriv->coldResetNeedFreq = 0;
                hpPriv->recordFreqRetryCounter = 0;
                zfCoreSetFrequencyComplete(dev);
            }
        }
        
        
        
        
        
        
        
        else
        {
            
            zm_debug_msg2(" return complete, ret = ", rsp[1]);

            
            if (hpPriv->enableBBHeavyClip && hpPriv->hwBBHeavyClip &&
                hpPriv->doBBHeavyClip)
            {
                u32_t setValue = 0x200;

                setValue |= hpPriv->setValueHeavyClip;

                

                zfDelayWriteInternalReg(dev, 0x99e0+0x1bc000, setValue);
                zfFlushDelayWrite(dev);
            }

            hpPriv->coldResetNeedFreq = 0;
            hpPriv->recordFreqRetryCounter = 0;
    	    zfCoreSetFrequencyComplete(dev);
    	}

        #if 1
        
        nf = ((rsp[2]>>19) & 0x1ff);
        if ((nf & 0x100) != 0x0)
        {
            noisefloor[0] = 0 - ((nf ^ 0x1ff) + 1);
        }
        else
        {
            noisefloor[0] = nf;
        }

        

        nf = ((rsp[3]>>19) & 0x1ff);
        if ((nf & 0x100) != 0x0)
        {
            noisefloor[1] = 0 - ((nf ^ 0x1ff) + 1);
        }
        else
        {
            noisefloor[1] = nf;
        }

        

        nf = ((rsp[5]>>23) & 0x1ff);
        if ((nf & 0x100) != 0x0)
        {
            noisefloor[2] = 0 - ((nf ^ 0x1ff) + 1);
        }
        else
        {
            noisefloor[2] = nf;
        }

        

        nf = ((rsp[6]>>23) & 0x1ff);
        if ((nf & 0x100) != 0x0)
        {
            noisefloor[3] = 0 - ((nf ^ 0x1ff) + 1);
        }
        else
        {
            noisefloor[3] = nf;
        }

        

        
        #endif
    }
    else if (src == ZM_CMD_SET_KEY)
    {
        zfCoreSetKeyComplete(dev);
    }
    else if (src == ZM_CWM_READ)
    {
        zm_msg2_mm(ZM_LV_0, "CWM rsp[1]=", rsp[1]);
        zm_msg2_mm(ZM_LV_0, "CWM rsp[2]=", rsp[2]);
        zfCoreCwmBusy(dev, zfCwmIsExtChanBusy(rsp[1], rsp[2]));
    }
    else if (src == ZM_MAC_READ)
    {
        
        
        
        
        
        

        u8_t addr[6], CCS, WWR;
        u16_t CountryDomainCode;

        
        
        
        #if 0
        if (hpPriv->hwBBHeavyClip)
        {
            zm_msg0_mm(ZM_LV_0, "enable BB Heavy Clip");
        }
        else
        {
            zm_msg0_mm(ZM_LV_0, "Not enable BB Heavy Clip");
        }
        #endif
        zm_msg2_mm(ZM_LV_0, "MAC rsp[1]=", rsp[1]);
        zm_msg2_mm(ZM_LV_0, "MAC rsp[2]=", rsp[2]);

        addr[0] = (u8_t)(rsp[1] & 0xff);
        addr[1] = (u8_t)((rsp[1]>>8) & 0xff);
        addr[2] = (u8_t)((rsp[1]>>16) & 0xff);
        addr[3] = (u8_t)((rsp[1]>>24) & 0xff);
        addr[4] = (u8_t)(rsp[2] & 0xff);
        addr[5] = (u8_t)((rsp[2]>>8) & 0xff);


        zfDelayWriteInternalReg(dev, ZM_MAC_REG_MAC_ADDR_L,
                ((((u32_t)addr[3])<<24) | (((u32_t)addr[2])<<16) | (((u32_t)addr[1])<<8) | addr[0]));
        zfDelayWriteInternalReg(dev, ZM_MAC_REG_MAC_ADDR_H,
                ((((u32_t)addr[5])<<8) | addr[4]));
        zfFlushDelayWrite(dev);

        wd->ledStruct.ledMode[0] = (u16_t)(rsp[5]&0xffff);
        wd->ledStruct.ledMode[1] = (u16_t)(rsp[5]>>16);
        zm_msg2_mm(ZM_LV_0, "ledMode[0]=", wd->ledStruct.ledMode[0]);
        zm_msg2_mm(ZM_LV_0, "ledMode[1]=", wd->ledStruct.ledMode[1]);

        
        zm_msg2_mm(ZM_LV_0, "RegDomain rsp=", rsp[3]);
        zm_msg2_mm(ZM_LV_0, "OpFlags+EepMisc=", rsp[4]);
        hpPriv->OpFlags = (u8_t)((rsp[4]>>16) & 0xff);
        if ((rsp[2] >> 24) == 0x1) 
        {
            zm_msg0_mm(ZM_LV_0, "OTUS 1x2");
            hpPriv->halCapability |= ZM_HP_CAP_11N_ONE_TX_STREAM;
        }
        else
        {
            zm_msg0_mm(ZM_LV_0, "OTUS 2x2");
        }
        if (hpPriv->OpFlags & 0x1)
        {
            hpPriv->halCapability |= ZM_HP_CAP_5G;
        }
        if (hpPriv->OpFlags & 0x2)
        {
            hpPriv->halCapability |= ZM_HP_CAP_2G;
        }


        CCS = (u8_t)((rsp[3] & 0x8000) >> 15);
        WWR = (u8_t)((rsp[3] & 0x4000) >> 14);
        CountryDomainCode = (u16_t)(rsp[3] & 0x3FFF);

        if (rsp[3] != 0xffffffff)
        {
            if (CCS)
            {
                
                zfHpGetRegulationTablefromCountry(dev, CountryDomainCode);
            }
            else
            {
                
                zfHpGetRegulationTablefromRegionCode(dev, CountryDomainCode);
            }
            if (WWR)
            {
                
                
                
            }
        }
        else
        {
            zfHpGetRegulationTablefromRegionCode(dev, NO_ENUMRD);
        }

        zfCoreMacAddressNotify(dev, addr);

    }
    else if (src == ZM_EEPROM_READ)
    {
#if 0
        u8_t addr[6], CCS, WWR;
        u16_t CountryDomainCode;
#endif
        for (i=0; i<ZM_HAL_MAX_EEPROM_PRQ; i++)
        {
            if (hpPriv->eepromImageIndex < 1024)
            {
                hpPriv->eepromImage[hpPriv->eepromImageIndex++] = rsp[i+1];
            }
        }

        if (hpPriv->eepromImageIndex == (ZM_HAL_MAX_EEPROM_REQ*ZM_HAL_MAX_EEPROM_PRQ))
        {
            #if 0
            for (i=0; i<1024; i++)
            {
                zm_msg2_mm(ZM_LV_0, "index=", i);
                zm_msg2_mm(ZM_LV_0, "eepromImage=", hpPriv->eepromImage[i]);
            }
            #endif
            zm_msg2_mm(ZM_LV_0, "MAC [1]=", hpPriv->eepromImage[0x20c/4]);
            zm_msg2_mm(ZM_LV_0, "MAC [2]=", hpPriv->eepromImage[0x210/4]);
#if 0
            addr[0] = (u8_t)(hpPriv->eepromImage[0x20c/4] & 0xff);
            addr[1] = (u8_t)((hpPriv->eepromImage[0x20c/4]>>8) & 0xff);
            addr[2] = (u8_t)((hpPriv->eepromImage[0x20c/4]>>16) & 0xff);
            addr[3] = (u8_t)((hpPriv->eepromImage[0x20c/4]>>24) & 0xff);
            addr[4] = (u8_t)(hpPriv->eepromImage[0x210/4] & 0xff);
            addr[5] = (u8_t)((hpPriv->eepromImage[0x210/4]>>8) & 0xff);

            zfCoreMacAddressNotify(dev, addr);

            zfDelayWriteInternalReg(dev, ZM_MAC_REG_MAC_ADDR_L,
                    ((((u32_t)addr[3])<<24) | (((u32_t)addr[2])<<16) | (((u32_t)addr[1])<<8) | addr[0]));
            zfDelayWriteInternalReg(dev, ZM_MAC_REG_MAC_ADDR_H,
                    ((((u32_t)addr[5])<<8) | addr[4]));
            zfFlushDelayWrite(dev);

            
            zm_msg2_mm(ZM_LV_0, "RegDomain =", hpPriv->eepromImage[0x208/4]);
            CCS = (u8_t)((hpPriv->eepromImage[0x208/4] & 0x8000) >> 15);
            WWR = (u8_t)((hpPriv->eepromImage[0x208/4] & 0x4000) >> 14);
            
            
            CountryDomainCode = 8;
            if (CCS)
            {
                
                zfHpGetRegulationTablefromCountry(dev, CountryDomainCode);
            }
            else
            {
                
                zfHpGetRegulationTablefromRegionCode(dev, CountryDomainCode);
            }
            if (WWR)
            {
                
                
                
            }
#endif
            zfCoreHalInitComplete(dev);
        }
        else
        {
            hpPriv->eepromImageRdReq++;
            zfHpLoadEEPROMFromFW(dev);
        }
    }
    else if (src == ZM_EEPROM_WRITE)
    {
        zfwDbgWriteEepromDone(dev, cmd[1], cmd[2]);
    }
    else if (src == ZM_ANI_READ)
    {
        u32_t cycleTime, ctlClear;

        zm_msg2_mm(ZM_LV_0, "ANI rsp[1]=", rsp[1]);
        zm_msg2_mm(ZM_LV_0, "ANI rsp[2]=", rsp[2]);
        zm_msg2_mm(ZM_LV_0, "ANI rsp[3]=", rsp[3]);
        zm_msg2_mm(ZM_LV_0, "ANI rsp[4]=", rsp[4]);

        hpPriv->ctlBusy += rsp[1];
        hpPriv->extBusy += rsp[2];

        cycleTime = 100000; 

        if (cycleTime > rsp[1])
        {
            ctlClear = (cycleTime - rsp[1]) / 100;
        }
        else
        {
            ctlClear = 0;
        }
        if (wd->aniEnable)
            zfHpAniArPoll(dev, ctlClear, rsp[3], rsp[4]);
    }
    else if (src == ZM_CMD_ECHO)
    {
        if ( ((struct zsHpPriv*)wd->hpPrivate)->halReInit )
        {
            zfCoreHalInitComplete(dev);
            ((struct zsHpPriv*)wd->hpPrivate)->halReInit = 0;
        }
        else
        {
            zfHpLoadEEPROMFromFW(dev);
        }
    }
    else if (src == ZM_OID_FW_DL_INIT)
    {
        zfwDbgDownloadFwInitDone(dev);
    }
    return;
}




















u32_t zfWriteRegInternalReg(zdev_t* dev, u32_t addr, u32_t val)
{
    u32_t cmd[3];
    u16_t ret;

    cmd[0] = 0x00000108;
    cmd[1] = addr;
    cmd[2] = val;

    ret = zfIssueCmd(dev, cmd, 12, ZM_OID_INTERNAL_WRITE, NULL);
    return ret;
}





















u16_t zfDelayWriteInternalReg(zdev_t* dev, u32_t addr, u32_t val)
{
    u32_t cmd[(ZM_MAX_CMD_SIZE/4)];
    u16_t i;
    u16_t ret;
    struct zsHpPriv* hpPriv;

    zmw_get_wlan_dev(dev);
    hpPriv=wd->hpPrivate;

    zmw_declare_for_critical_section();

    
    zmw_enter_critical_section(dev);

    
    hpPriv->cmd.delayWcmdAddr[hpPriv->cmd.delayWcmdCount] = addr;
    hpPriv->cmd.delayWcmdVal[hpPriv->cmd.delayWcmdCount++] = val;

    
    if ((hpPriv->cmd.delayWcmdCount) >= ((ZM_MAX_CMD_SIZE - 4) >> 3))
    {
        cmd[0] = 0x00000100 + (hpPriv->cmd.delayWcmdCount<<3);

        
        for (i=0; i<hpPriv->cmd.delayWcmdCount; i++)
        {
            cmd[1+(i<<1)] = hpPriv->cmd.delayWcmdAddr[i];
            cmd[2+(i<<1)] = hpPriv->cmd.delayWcmdVal[i];
        }
        
        hpPriv->cmd.delayWcmdCount = 0;

        
        zmw_leave_critical_section(dev);

        
        ret = zfIssueCmd(dev, cmd, 4+(i<<3), ZM_OID_INTERNAL_WRITE, NULL);

        return 1;
    }
    else
    {
        
        zmw_leave_critical_section(dev);

        return 0;
    }
}


















u16_t zfFlushDelayWrite(zdev_t* dev)
{
    u32_t cmd[(ZM_MAX_CMD_SIZE/4)];
    u16_t i;
    u16_t ret;
    struct zsHpPriv* hpPriv;

    zmw_get_wlan_dev(dev);
    hpPriv=wd->hpPrivate;

    zmw_declare_for_critical_section();

    
    zmw_enter_critical_section(dev);

    
    if (hpPriv->cmd.delayWcmdCount > 0)
    {
        cmd[0] = 0x00000100 + (hpPriv->cmd.delayWcmdCount<<3);

        
        for (i=0; i<hpPriv->cmd.delayWcmdCount; i++)
        {
            cmd[1+(i<<1)] = hpPriv->cmd.delayWcmdAddr[i];
            cmd[2+(i<<1)] = hpPriv->cmd.delayWcmdVal[i];
        }
        
        hpPriv->cmd.delayWcmdCount = 0;

        
        zmw_leave_critical_section(dev);

        
        ret = zfIssueCmd(dev, cmd, 4+(i<<3), ZM_OID_INTERNAL_WRITE, NULL);

        return 1;
    }
    else
    {
        
        zmw_leave_critical_section(dev);

        return 0;
    }
}


u32_t zfiDbgDelayWriteReg(zdev_t* dev, u32_t addr, u32_t val)
{
	zfDelayWriteInternalReg(dev, addr, val);
	return 0;
}

u32_t zfiDbgFlushDelayWrite(zdev_t* dev)
{
	zfFlushDelayWrite(dev);
	return 0;
}



















u32_t zfiDbgWriteReg(zdev_t* dev, u32_t addr, u32_t val)
{
    u32_t cmd[3];
    u16_t ret;

    cmd[0] = 0x00000108;
    cmd[1] = addr;
    cmd[2] = val;

    ret = zfIssueCmd(dev, cmd, 12, ZM_OID_WRITE, 0);
    return ret;
}


















u32_t zfiDbgWriteFlash(zdev_t* dev, u32_t addr, u32_t val)
{
    u32_t cmd[3];
    u16_t ret;

    
	
    cmd[0] = 8 | (ZM_CMD_WFLASH << 8);
    cmd[1] = addr;
    cmd[2] = val;

    ret = zfIssueCmd(dev, cmd, 12, ZM_OID_WRITE, 0);
    return ret;
}



















u32_t zfiDbgWriteEeprom(zdev_t* dev, u32_t addr, u32_t val)
{
    u32_t cmd[3];
    u16_t ret;

    
	
    cmd[0] = 8 | (ZM_CMD_WREEPROM << 8);
    cmd[1] = addr;
    cmd[2] = val;

    ret = zfIssueCmd(dev, cmd, 12, ZM_EEPROM_WRITE, 0);
    return ret;
}
























u32_t zfiDbgBlockWriteEeprom(zdev_t* dev, u32_t addr, u32_t* buf)
{
    u32_t cmd[9];  
    u16_t ret,i;

    
	  

    
    cmd[0] = 32 | (ZM_CMD_WREEPROM << 8);    

    for (i=0; i<4; i++)   
    {
        cmd[(2*i)+1] = addr+(4*i);
        cmd[(2*i)+2] = *(buf+i);
    }

    ret = zfIssueCmd(dev, cmd, 36, ZM_EEPROM_WRITE, 0);    

    
    

    return ret;
}




u32_t zfiDbgBlockWriteEeprom_v2(zdev_t* dev, u32_t addr, u32_t* buf, u32_t wrlen)
{
    u32_t cmd[16];
    u16_t ret,i;

	  
	  
    cmd[0] = (wrlen+4) | (ZM_CMD_MEM_WREEPROM << 8);
    cmd[1] = addr;

    for (i=0; i<(wrlen/4); i++)   
    {
        cmd[2+i] = *(buf+i);
    }
    
    ret = zfIssueCmd(dev, cmd, (u16_t)(wrlen+8), ZM_EEPROM_WRITE, 0);

    return ret;
}















void zfDbgOpenEeprom(zdev_t* dev)
{
    
    zfDelayWriteInternalReg(dev, 0x1D1400, 0x12345678);
    zfDelayWriteInternalReg(dev, 0x1D1404, 0x55aa00ff);
    zfDelayWriteInternalReg(dev, 0x1D1408, 0x13579ace);
    zfDelayWriteInternalReg(dev, 0x1D1414, 0x0);
    zfFlushDelayWrite(dev);
}















void zfDbgCloseEeprom(zdev_t* dev)
{
    
    zfDelayWriteInternalReg(dev, 0x1D1400, 0x87654321);
    
    
    
    zfFlushDelayWrite(dev);
}
#if 0




















u32_t zfiSeriallyWriteEeprom(zdev_t* dev, u32_t addr, u32_t* buf, u32_t buflen)
{
    u32_t count;
    u16_t i,ret,blocksize;
    u8_t  temp[2];

    
    count = buflen/4;

    
    zfDbgOpenEeprom(dev);

    
    for (i=0; i<count; i++)
    {
        if (zfwWriteEeprom(dev, (addr+(4*i)), *(buf+i), 0) != 0)
        {
            
            zm_debug_msg0("zfwWriteEeprom failed \n");
            zfDbgCloseEeprom(dev);
            return 1;
        }
    }

    
    zfDbgCloseEeprom(dev);
    return 0;
}
#endif
#if 0




















u32_t zfiSeriallyBlockWriteEeprom(zdev_t* dev, u32_t addr, u32_t* buf, u32_t buflen)
{
    u32_t count;
    u16_t i,ret,blocksize;
    u8_t  temp[2];

    
    count = buflen/4;

    
    zfDbgOpenEeprom(dev);

    
    
    
    for (i=0; i<(count/4); i++)   
    {
        
        
        if (zfwBlockWriteEeprom(dev, (addr+(16*i)), buf+(4*i), 0) != 0)
        {
            zm_debug_msg0("zfiDbgBlockWriteEeprom failed \n");
            
            zfDbgCloseEeprom(dev);
            return 1;
        }
    }

    
    zfDbgCloseEeprom(dev);
    return 0;
}
#endif
#if 0



















u32_t zfiDbgDumpEeprom(zdev_t* dev, u32_t addr, u32_t datalen, u32_t* buf)
{
    u32_t count;
    u16_t i,ret;

    count = datalen/4;

    
    if(datalen > 0x2000)
    {
        return 1;
    }

    for(i=0; i<count; i++)
    {
        buf[i] = zfwReadEeprom(dev, addr+(4*i));
    }

    return 0;
}
#endif

















u32_t zfiDbgReadReg(zdev_t* dev, u32_t addr)
{
    u32_t cmd[2];
    u16_t ret;

    cmd[0] = 0x00000004;
    cmd[1] = addr;

    ret = zfIssueCmd(dev, cmd, 8, ZM_OID_READ, 0);
    return ret;
}


















u32_t zfiDbgReadTally(zdev_t* dev)
{
    u32_t cmd[1];
    u16_t ret;
	zmw_get_wlan_dev(dev);

	if ( ((struct zsHpPriv*)wd->hpPrivate)->halReInit )
	{
	    return 1;
	}

	
    cmd[0] = 0 | (ZM_CMD_TALLY << 8);
    ret = zfIssueCmd(dev, cmd, 4, ZM_OID_TALLY, 0);

	
    cmd[0] = 0 | (ZM_CMD_TALLY_APD << 8);
    ret = zfIssueCmd(dev, cmd, 4, ZM_OID_TALLY_APD, 0);

    return ret;
}


u32_t zfiDbgSetIFSynthesizer(zdev_t* dev, u32_t value)
{
    u32_t cmd[2];
    u16_t ret;

	
    cmd[0] = 0x4 | (ZM_OID_SYNTH << 8);
    cmd[1] = value;

    ret = zfIssueCmd(dev, cmd, 8, ZM_OID_SYNTH, 0);
    return ret;
}

u32_t zfiDbgQueryHwTxBusy(zdev_t* dev)
{
    u32_t cmd[1];
    u16_t ret;

	
	cmd[0] = 0 | (ZM_CMD_DKTX_STATUS << 8);

    ret = zfIssueCmd(dev, cmd, 4, ZM_OID_DKTX_STATUS, 0);
    return ret;
}


#if 0
u16_t zfHpBlockEraseFlash(zdev_t *dev, u32_t addr)
{
    u32_t cmd[(ZM_MAX_CMD_SIZE/4)];
    u16_t ret;

    cmd[0] = 0x00000004 | (ZM_CMD_FLASH_ERASE << 8);
    cmd[1] = addr;

    ret = zfIssueCmd(dev, cmd, 8, ZM_OID_INTERNAL_WRITE, NULL);
    return ret;
}
#endif

#if 0
u16_t zfiDbgProgramFlash(zdev_t *dev, u32_t offset, u32_t len, u32_t *data)
{
    u32_t cmd[(ZM_MAX_CMD_SIZE/4)];
    u16_t ret;
    u16_t i;


    cmd[0] = (ZM_CMD_FLASH_PROG << 8) | ((len+8) & 0xff);
    cmd[1] = offset;
    cmd[2] = len;

    for (i = 0; i < (len >> 2); i++)
    {
         cmd[3+i] = data[i];
    }

    ret = zfIssueCmd(dev, cmd, 12, ZM_OID_FLASH_PROGRAM, NULL);

    return ret;
}
#endif

















u16_t zfiDbgChipEraseFlash(zdev_t *dev)
{
    u32_t cmd[(ZM_MAX_CMD_SIZE/4)];
    u16_t ret;

    cmd[0] = 0x00000000 | (ZM_CMD_FLASH_ERASE << 8);

    ret = zfIssueCmd(dev, cmd, 4, ZM_OID_INTERNAL_WRITE, NULL);
    return ret;
}


















u32_t zfiDbgGetFlashCheckSum(zdev_t *dev, u32_t addr, u32_t len)
{
    u32_t cmd[(ZM_MAX_CMD_SIZE/4)];
    u32_t ret;

    cmd[0] = 0x00000008 | (ZM_CMD_FLASH_CHKSUM << 8);
    cmd[1] = addr;
    cmd[2] = len;

    ret = zfIssueCmd(dev, cmd, 12, ZM_OID_FLASH_CHKSUM, NULL);

    return ret;
}



















u32_t zfiDbgReadFlash(zdev_t *dev, u32_t addr, u32_t len)
{
    u32_t cmd[(ZM_MAX_CMD_SIZE/4)];
    u32_t ret;

    cmd[0] = len | (ZM_CMD_FLASH_READ << 8);
    cmd[1] = addr;

    ret = zfIssueCmd(dev, cmd, 8, ZM_OID_FLASH_READ, NULL);
    return ret;
}



















u32_t zfiDownloadFwSet(zdev_t *dev)
{


    u32_t cmd[(ZM_MAX_CMD_SIZE/4)];
    u32_t ret;

    cmd[0] = 0x00000008 | (ZM_CMD_FW_DL_INIT << 8);

    ret = zfIssueCmd(dev, cmd, 12, ZM_OID_FW_DL_INIT, NULL);

    return ret;
}

