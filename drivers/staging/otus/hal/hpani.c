
#include "../80211core/cprecomp.h"
#include "hpani.h"
#include "hpusb.h"


extern u16_t zfDelayWriteInternalReg(zdev_t* dev, u32_t addr, u32_t val);
extern u16_t zfFlushDelayWrite(zdev_t* dev);





#define ZM_HAL_NOISE_IMMUNE_MAX     4   
#define ZM_HAL_SPUR_IMMUNE_MAX      7   
#define ZM_HAL_FIRST_STEP_MAX       2   

#define ZM_HAL_ANI_OFDM_TRIG_HIGH       500
#define ZM_HAL_ANI_OFDM_TRIG_LOW        200
#define ZM_HAL_ANI_CCK_TRIG_HIGH        200
#define ZM_HAL_ANI_CCK_TRIG_LOW         100
#define ZM_HAL_ANI_NOISE_IMMUNE_LVL     4
#define ZM_HAL_ANI_USE_OFDM_WEAK_SIG    TRUE
#define ZM_HAL_ANI_CCK_WEAK_SIG_THR     FALSE
#define ZM_HAL_ANI_SPUR_IMMUNE_LVL      7
#define ZM_HAL_ANI_FIRSTEP_LVL          0
#define ZM_HAL_ANI_RSSI_THR_HIGH        40
#define ZM_HAL_ANI_RSSI_THR_LOW         7
#define ZM_HAL_ANI_PERIOD               100

#define ZM_HAL_EP_RND(x, mul) \
    ((((x)%(mul)) >= ((mul)/2)) ? ((x) + ((mul) - 1)) / (mul) : (x)/(mul))

s32_t BEACON_RSSI(zdev_t* dev)
{
    s32_t rssi;
    struct zsHpPriv *HpPriv;

    zmw_get_wlan_dev(dev);
    HpPriv = (struct zsHpPriv*)wd->hpPrivate;

    rssi = ZM_HAL_EP_RND(HpPriv->stats.ast_nodestats.ns_avgbrssi, ZM_HAL_RSSI_EP_MULTIPLIER);

    return rssi;
}



void zfHpAniAttach(zdev_t* dev)
{
#define N(a)     (sizeof(a) / sizeof(a[0]))
    u32_t i;
    struct zsHpPriv *HpPriv;

    const int totalSizeDesired[] = { -55, -55, -55, -55, -62 };
    const int coarseHigh[]       = { -14, -14, -14, -14, -12 };
    const int coarseLow[]        = { -64, -64, -64, -64, -70 };
    const int firpwr[]           = { -78, -78, -78, -78, -80 };

    zmw_get_wlan_dev(dev);
    HpPriv = (struct zsHpPriv*)wd->hpPrivate;

    for (i = 0; i < 5; i++)
    {
        HpPriv->totalSizeDesired[i] = totalSizeDesired[i];
        HpPriv->coarseHigh[i] = coarseHigh[i];
        HpPriv->coarseLow[i] = coarseLow[i];
        HpPriv->firpwr[i] = firpwr[i];
    }

    
    HpPriv->hasHwPhyCounters = 1;

    memset((char *)&HpPriv->ani, 0, sizeof(HpPriv->ani));
    for (i = 0; i < N(wd->regulationTable.allowChannel); i++)
    {
        
        HpPriv->ani[i].ofdmTrigHigh = ZM_HAL_ANI_OFDM_TRIG_HIGH;
        HpPriv->ani[i].ofdmTrigLow = ZM_HAL_ANI_OFDM_TRIG_LOW;
        HpPriv->ani[i].cckTrigHigh = ZM_HAL_ANI_CCK_TRIG_HIGH;
        HpPriv->ani[i].cckTrigLow = ZM_HAL_ANI_CCK_TRIG_LOW;
        HpPriv->ani[i].rssiThrHigh = ZM_HAL_ANI_RSSI_THR_HIGH;
        HpPriv->ani[i].rssiThrLow = ZM_HAL_ANI_RSSI_THR_LOW;
        HpPriv->ani[i].ofdmWeakSigDetectOff = !ZM_HAL_ANI_USE_OFDM_WEAK_SIG;
        HpPriv->ani[i].cckWeakSigThreshold = ZM_HAL_ANI_CCK_WEAK_SIG_THR;
        HpPriv->ani[i].spurImmunityLevel = ZM_HAL_ANI_SPUR_IMMUNE_LVL;
        HpPriv->ani[i].firstepLevel = ZM_HAL_ANI_FIRSTEP_LVL;
        if (HpPriv->hasHwPhyCounters)
        {
            HpPriv->ani[i].ofdmPhyErrBase = 0;
            HpPriv->ani[i].cckPhyErrBase = 0;
        }
    }
    if (HpPriv->hasHwPhyCounters)
    {
        
        
        
        
    }
    HpPriv->aniPeriod = ZM_HAL_ANI_PERIOD;
    
    HpPriv->procPhyErr |= ZM_HAL_PROCESS_ANI;

    HpPriv->stats.ast_nodestats.ns_avgbrssi = ZM_RSSI_DUMMY_MARKER;
    HpPriv->stats.ast_nodestats.ns_avgrssi = ZM_RSSI_DUMMY_MARKER;
    HpPriv->stats.ast_nodestats.ns_avgtxrssi = ZM_RSSI_DUMMY_MARKER;
#undef N
}


u8_t zfHpAniControl(zdev_t* dev, ZM_HAL_ANI_CMD cmd, int param)
{
#define N(a) (sizeof(a)/sizeof(a[0]))
    typedef s32_t TABLE[];
    struct zsHpPriv *HpPriv;
    struct zsAniState *aniState;

    zmw_get_wlan_dev(dev);
    HpPriv = (struct zsHpPriv*)wd->hpPrivate;
    aniState = HpPriv->curani;

    switch (cmd)
    {
    case ZM_HAL_ANI_NOISE_IMMUNITY_LEVEL:
    {
        u32_t level = param;

        if (level >= N(HpPriv->totalSizeDesired))
        {
          zm_debug_msg1("level out of range, desired level : ", level);
          zm_debug_msg1("max level : ", N(HpPriv->totalSizeDesired));
          return FALSE;
        }

        zfDelayWriteInternalReg(dev, AR_PHY_DESIRED_SZ,
                (HpPriv->regPHYDesiredSZ & ~AR_PHY_DESIRED_SZ_TOT_DES)
                | ((HpPriv->totalSizeDesired[level] << AR_PHY_DESIRED_SZ_TOT_DES_S)
                & AR_PHY_DESIRED_SZ_TOT_DES));
        zfDelayWriteInternalReg(dev, AR_PHY_AGC_CTL1,
                (HpPriv->regPHYAgcCtl1 & ~AR_PHY_AGC_CTL1_COARSE_LOW)
                | ((HpPriv->coarseLow[level] << AR_PHY_AGC_CTL1_COARSE_LOW_S)
                & AR_PHY_AGC_CTL1_COARSE_LOW));
        zfDelayWriteInternalReg(dev, AR_PHY_AGC_CTL1,
                (HpPriv->regPHYAgcCtl1 & ~AR_PHY_AGC_CTL1_COARSE_HIGH)
                | ((HpPriv->coarseHigh[level] << AR_PHY_AGC_CTL1_COARSE_HIGH_S)
                & AR_PHY_AGC_CTL1_COARSE_HIGH));
        zfDelayWriteInternalReg(dev, AR_PHY_FIND_SIG,
                (HpPriv->regPHYFindSig & ~AR_PHY_FIND_SIG_FIRPWR)
                | ((HpPriv->firpwr[level] << AR_PHY_FIND_SIG_FIRPWR_S)
                & AR_PHY_FIND_SIG_FIRPWR));
        zfFlushDelayWrite(dev);

        if (level > aniState->noiseImmunityLevel)
            HpPriv->stats.ast_ani_niup++;
        else if (level < aniState->noiseImmunityLevel)
            HpPriv->stats.ast_ani_nidown++;
        aniState->noiseImmunityLevel = (u8_t)level;
        break;
    }
    case ZM_HAL_ANI_OFDM_WEAK_SIGNAL_DETECTION:
    {
        const TABLE m1ThreshLow   = { 127,   50 };
        const TABLE m2ThreshLow   = { 127,   40 };
        const TABLE m1Thresh      = { 127, 0x4d };
        const TABLE m2Thresh      = { 127, 0x40 };
        const TABLE m2CountThr    = {  31,   16 };
        const TABLE m2CountThrLow = {  63,   48 };
        u32_t on = param ? 1 : 0;

        zfDelayWriteInternalReg(dev, AR_PHY_SFCORR_LOW,
                (HpPriv->regPHYSfcorrLow & ~AR_PHY_SFCORR_LOW_M1_THRESH_LOW)
                | ((m1ThreshLow[on] << AR_PHY_SFCORR_LOW_M1_THRESH_LOW_S)
                & AR_PHY_SFCORR_LOW_M1_THRESH_LOW));
        zfDelayWriteInternalReg(dev, AR_PHY_SFCORR_LOW,
                (HpPriv->regPHYSfcorrLow & ~AR_PHY_SFCORR_LOW_M2_THRESH_LOW)
                | ((m2ThreshLow[on] << AR_PHY_SFCORR_LOW_M2_THRESH_LOW_S)
                & AR_PHY_SFCORR_LOW_M2_THRESH_LOW));
        zfDelayWriteInternalReg(dev, AR_PHY_SFCORR,
                (HpPriv->regPHYSfcorr & ~AR_PHY_SFCORR_M1_THRESH)
                | ((m1Thresh[on] << AR_PHY_SFCORR_M1_THRESH_S)
                & AR_PHY_SFCORR_M1_THRESH));
        zfDelayWriteInternalReg(dev, AR_PHY_SFCORR,
                (HpPriv->regPHYSfcorr & ~AR_PHY_SFCORR_M2_THRESH)
                | ((m2Thresh[on] << AR_PHY_SFCORR_M2_THRESH_S)
                & AR_PHY_SFCORR_M2_THRESH));
        zfDelayWriteInternalReg(dev, AR_PHY_SFCORR,
                (HpPriv->regPHYSfcorr & ~AR_PHY_SFCORR_M2COUNT_THR)
                | ((m2CountThr[on] << AR_PHY_SFCORR_M2COUNT_THR_S)
                & AR_PHY_SFCORR_M2COUNT_THR));
        zfDelayWriteInternalReg(dev, AR_PHY_SFCORR_LOW,
                (HpPriv->regPHYSfcorrLow & ~AR_PHY_SFCORR_LOW_M2COUNT_THR_LOW)
                | ((m2CountThrLow[on] << AR_PHY_SFCORR_LOW_M2COUNT_THR_LOW_S)
                & AR_PHY_SFCORR_LOW_M2COUNT_THR_LOW));

        if (on)
        {
            zfDelayWriteInternalReg(dev, AR_PHY_SFCORR_LOW,
                    HpPriv->regPHYSfcorrLow | AR_PHY_SFCORR_LOW_USE_SELF_CORR_LOW);
        }
        else
        {
            zfDelayWriteInternalReg(dev, AR_PHY_SFCORR_LOW,
                    HpPriv->regPHYSfcorrLow & ~AR_PHY_SFCORR_LOW_USE_SELF_CORR_LOW);
        }
        zfFlushDelayWrite(dev);
        if (!on != aniState->ofdmWeakSigDetectOff)
        {
            if (on)
                HpPriv->stats.ast_ani_ofdmon++;
            else
                HpPriv->stats.ast_ani_ofdmoff++;
            aniState->ofdmWeakSigDetectOff = !on;
        }
        break;
    }
    case ZM_HAL_ANI_CCK_WEAK_SIGNAL_THR:
    {
        const TABLE weakSigThrCck = { 8, 6 };
        u32_t high = param ? 1 : 0;

        zfDelayWriteInternalReg(dev, AR_PHY_CCK_DETECT,
                (HpPriv->regPHYCckDetect & ~AR_PHY_CCK_DETECT_WEAK_SIG_THR_CCK)
                | ((weakSigThrCck[high] << AR_PHY_CCK_DETECT_WEAK_SIG_THR_CCK_S)
                & AR_PHY_CCK_DETECT_WEAK_SIG_THR_CCK));
        zfFlushDelayWrite(dev);
        if (high != aniState->cckWeakSigThreshold)
        {
            if (high)
                HpPriv->stats.ast_ani_cckhigh++;
            else
                HpPriv->stats.ast_ani_ccklow++;
            aniState->cckWeakSigThreshold = (u8_t)high;
        }
        break;
    }
    case ZM_HAL_ANI_FIRSTEP_LEVEL:
    {
        const TABLE firstep = { 0, 4, 8 };
        u32_t level = param;

        if (level >= N(firstep))
        {
            zm_debug_msg1("level out of range, desired level : ", level);
            zm_debug_msg1("max level : ", N(firstep));
            return FALSE;
        }
        zfDelayWriteInternalReg(dev, AR_PHY_FIND_SIG,
                (HpPriv->regPHYFindSig & ~AR_PHY_FIND_SIG_FIRSTEP)
                | ((firstep[level] << AR_PHY_FIND_SIG_FIRSTEP_S)
                & AR_PHY_FIND_SIG_FIRSTEP));
        zfFlushDelayWrite(dev);
        if (level > aniState->firstepLevel)
            HpPriv->stats.ast_ani_stepup++;
        else if (level < aniState->firstepLevel)
            HpPriv->stats.ast_ani_stepdown++;
        aniState->firstepLevel = (u8_t)level;
        break;
    }
    case ZM_HAL_ANI_SPUR_IMMUNITY_LEVEL:
    {
        const TABLE cycpwrThr1 = { 2, 4, 6, 8, 10, 12, 14, 16 };
        u32_t level = param;

        if (level >= N(cycpwrThr1))
        {
            zm_debug_msg1("level out of range, desired level : ", level);
            zm_debug_msg1("max level : ", N(cycpwrThr1));
            return FALSE;
        }
        zfDelayWriteInternalReg(dev, AR_PHY_TIMING5,
                (HpPriv->regPHYTiming5 & ~AR_PHY_TIMING5_CYCPWR_THR1)
                | ((cycpwrThr1[level] << AR_PHY_TIMING5_CYCPWR_THR1_S)
                & AR_PHY_TIMING5_CYCPWR_THR1));
        zfFlushDelayWrite(dev);
        if (level > aniState->spurImmunityLevel)
            HpPriv->stats.ast_ani_spurup++;
        else if (level < aniState->spurImmunityLevel)
            HpPriv->stats.ast_ani_spurdown++;
        aniState->spurImmunityLevel = (u8_t)level;
        break;
    }
    case ZM_HAL_ANI_PRESENT:
        break;
#ifdef AH_PRIVATE_DIAG
    case ZM_HAL_ANI_MODE:
        if (param == 0)
        {
            HpPriv->procPhyErr &= ~ZM_HAL_PROCESS_ANI;
            
            zfHpAniDetach(dev);
            
        }
        else
        {           
            HpPriv->procPhyErr |= ZM_HAL_PROCESS_ANI;
            if (HpPriv->hasHwPhyCounters)
            {
                
            }
            else
            {
                
            }
        }
        break;
    case ZM_HAL_ANI_PHYERR_RESET:
        HpPriv->stats.ast_ani_ofdmerrs = 0;
        HpPriv->stats.ast_ani_cckerrs = 0;
        break;
#endif 
    default:
        zm_debug_msg1("invalid cmd ", cmd);
        return FALSE;
    }
    return TRUE;
#undef  N
}

void zfHpAniRestart(zdev_t* dev)
{
    struct zsAniState *aniState;
    struct zsHpPriv *HpPriv;

    zmw_get_wlan_dev(dev);
    HpPriv = (struct zsHpPriv*)wd->hpPrivate;
    aniState = HpPriv->curani;

    aniState->listenTime = 0;
    if (HpPriv->hasHwPhyCounters)
    {
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        aniState->ofdmPhyErrBase = 0;
        aniState->cckPhyErrBase = 0;
    }
    aniState->ofdmPhyErrCount = 0;
    aniState->cckPhyErrCount = 0;
}

void zfHpAniOfdmErrTrigger(zdev_t* dev)
{
    struct zsAniState *aniState;
    s32_t rssi;
    struct zsHpPriv *HpPriv;

    zmw_get_wlan_dev(dev);
    HpPriv = (struct zsHpPriv*)wd->hpPrivate;

    

    if ((HpPriv->procPhyErr & ZM_HAL_PROCESS_ANI) == 0)
        return;

    aniState = HpPriv->curani;
    
    if (aniState->noiseImmunityLevel < ZM_HAL_NOISE_IMMUNE_MAX)
    {
        zfHpAniControl(dev, ZM_HAL_ANI_NOISE_IMMUNITY_LEVEL, aniState->noiseImmunityLevel + 1);
        return;
    }
    
    if (aniState->spurImmunityLevel < ZM_HAL_SPUR_IMMUNE_MAX)
    {
        zfHpAniControl(dev, ZM_HAL_ANI_SPUR_IMMUNITY_LEVEL, aniState->spurImmunityLevel + 1);
        return;
    }
    rssi = BEACON_RSSI(dev);
    if (rssi > aniState->rssiThrHigh)
    {
        
        if (!aniState->ofdmWeakSigDetectOff)
        {
            zfHpAniControl(dev, ZM_HAL_ANI_OFDM_WEAK_SIGNAL_DETECTION, FALSE);
            zfHpAniControl(dev, ZM_HAL_ANI_SPUR_IMMUNITY_LEVEL, 0);
            return;
        }
        
        if (aniState->firstepLevel < ZM_HAL_FIRST_STEP_MAX)
        {
            zfHpAniControl(dev, ZM_HAL_ANI_FIRSTEP_LEVEL, aniState->firstepLevel + 1);
            return;
        }
    }
    else if (rssi > aniState->rssiThrLow)
    {
        
        if (aniState->ofdmWeakSigDetectOff)
            zfHpAniControl(dev, ZM_HAL_ANI_OFDM_WEAK_SIGNAL_DETECTION, TRUE);
        if (aniState->firstepLevel < ZM_HAL_FIRST_STEP_MAX)
            zfHpAniControl(dev, ZM_HAL_ANI_FIRSTEP_LEVEL, aniState->firstepLevel + 1);
        return;
    }
    else
    {
        
        if (wd->frequency < 3000)
        {
            if (!aniState->ofdmWeakSigDetectOff)
                zfHpAniControl(dev, ZM_HAL_ANI_OFDM_WEAK_SIGNAL_DETECTION, FALSE);
            if (aniState->firstepLevel > 0)
                zfHpAniControl(dev, ZM_HAL_ANI_FIRSTEP_LEVEL, 0);
            return;
        }
    }
}

void zfHpAniCckErrTrigger(zdev_t* dev)
{
    struct zsAniState *aniState;
    s32_t rssi;
    struct zsHpPriv *HpPriv;

    zmw_get_wlan_dev(dev);
    HpPriv = (struct zsHpPriv*)wd->hpPrivate;

    

    if ((HpPriv->procPhyErr & ZM_HAL_PROCESS_ANI) == 0)
        return;

    
    aniState = HpPriv->curani;
    if (aniState->noiseImmunityLevel < ZM_HAL_NOISE_IMMUNE_MAX)
    {
        zfHpAniControl(dev, ZM_HAL_ANI_NOISE_IMMUNITY_LEVEL,
                 aniState->noiseImmunityLevel + 1);
        return;
    }
    rssi = BEACON_RSSI(dev);
    if (rssi >  aniState->rssiThrLow)
    {
        
        if (aniState->firstepLevel < ZM_HAL_FIRST_STEP_MAX)
            zfHpAniControl(dev, ZM_HAL_ANI_FIRSTEP_LEVEL, aniState->firstepLevel + 1);
    }
    else
    {
        
        if (wd->frequency < 3000)
        {
            if (aniState->firstepLevel > 0)
                zfHpAniControl(dev, ZM_HAL_ANI_FIRSTEP_LEVEL, 0);
        }
    }
}

void zfHpAniLowerImmunity(zdev_t* dev)
{
    struct zsAniState *aniState;
    s32_t rssi;
    struct zsHpPriv *HpPriv;

    zmw_get_wlan_dev(dev);
    HpPriv = (struct zsHpPriv*)wd->hpPrivate;
    aniState = HpPriv->curani;

    rssi = BEACON_RSSI(dev);
    if (rssi > aniState->rssiThrHigh)
    {
        
    }
    else if (rssi > aniState->rssiThrLow)
    {
        
        if (aniState->ofdmWeakSigDetectOff)
        {
            zfHpAniControl(dev, ZM_HAL_ANI_OFDM_WEAK_SIGNAL_DETECTION, TRUE);
            return;
        }
        if (aniState->firstepLevel > 0)
        {
            zfHpAniControl(dev, ZM_HAL_ANI_FIRSTEP_LEVEL, aniState->firstepLevel - 1);
            return;
        }
    }
    else
    {
        
        if (aniState->firstepLevel > 0)
        {
            zfHpAniControl(dev, ZM_HAL_ANI_FIRSTEP_LEVEL, aniState->firstepLevel - 1);
            return;
        }
    }
    
    if (aniState->spurImmunityLevel > 0)
    {
        zfHpAniControl(dev, ZM_HAL_ANI_SPUR_IMMUNITY_LEVEL, aniState->spurImmunityLevel - 1);
        return;
    }
    
    if (aniState->noiseImmunityLevel > 0)
    {
        zfHpAniControl(dev, ZM_HAL_ANI_NOISE_IMMUNITY_LEVEL, aniState->noiseImmunityLevel - 1);
        return;
    }
}

#define CLOCK_RATE 44000    



s32_t zfHpAniGetListenTime(zdev_t* dev)
{
    struct zsAniState *aniState;
    u32_t txFrameCount, rxFrameCount, cycleCount;
    s32_t listenTime;
    struct zsHpPriv *HpPriv;

    zmw_get_wlan_dev(dev);
    HpPriv = (struct zsHpPriv*)wd->hpPrivate;

    txFrameCount = 0;
    rxFrameCount = 0;
    cycleCount = 0;

    aniState = HpPriv->curani;
    if (aniState->cycleCount == 0 || aniState->cycleCount > cycleCount)
    {
        
        listenTime = 0;
        HpPriv->stats.ast_ani_lzero++;
    }
    else
    {
        s32_t ccdelta = cycleCount - aniState->cycleCount;
        s32_t rfdelta = rxFrameCount - aniState->rxFrameCount;
        s32_t tfdelta = txFrameCount - aniState->txFrameCount;
        listenTime = (ccdelta - rfdelta - tfdelta) / CLOCK_RATE;
    }
    aniState->cycleCount = cycleCount;
    aniState->txFrameCount = txFrameCount;
    aniState->rxFrameCount = rxFrameCount;
    return listenTime;
}


void zfHpAniArPoll(zdev_t* dev, u32_t listenTime, u32_t phyCnt1, u32_t phyCnt2)
{
    struct zsAniState *aniState;
    
    struct zsHpPriv *HpPriv;

    zmw_get_wlan_dev(dev);
    HpPriv = (struct zsHpPriv*)wd->hpPrivate;

    

    aniState = HpPriv->curani;
    

    
    
    
    
    
    
    
    
    
    aniState->listenTime += listenTime;

    if (HpPriv->hasHwPhyCounters)
    {
        
        u32_t ofdmPhyErrCnt, cckPhyErrCnt;

        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        ofdmPhyErrCnt = phyCnt1;
        HpPriv->stats.ast_ani_ofdmerrs += ofdmPhyErrCnt;
        aniState->ofdmPhyErrCount += ofdmPhyErrCnt;

        
        
        
        cckPhyErrCnt = phyCnt2;
        HpPriv->stats.ast_ani_cckerrs += cckPhyErrCnt;
        aniState->cckPhyErrCount += cckPhyErrCnt;
    }
    
    if ((HpPriv->procPhyErr & ZM_HAL_PROCESS_ANI) == 0)
        return;
    if (aniState->listenTime > 5 * HpPriv->aniPeriod)
    {
        
        if (aniState->ofdmPhyErrCount <= aniState->listenTime *
             aniState->ofdmTrigLow/1000 &&
            aniState->cckPhyErrCount <= aniState->listenTime *
             aniState->cckTrigLow/1000)
            zfHpAniLowerImmunity(dev);
        zfHpAniRestart(dev);
    }
    else if (aniState->listenTime > HpPriv->aniPeriod)
    {
        
        if (aniState->ofdmPhyErrCount > aniState->listenTime *
            aniState->ofdmTrigHigh / 1000)
        {
            zfHpAniOfdmErrTrigger(dev);
            zfHpAniRestart(dev);
        }
        else if (aniState->cckPhyErrCount > aniState->listenTime *
               aniState->cckTrigHigh / 1000)
        {
            zfHpAniCckErrTrigger(dev);
            zfHpAniRestart(dev);
        }
    }
}
