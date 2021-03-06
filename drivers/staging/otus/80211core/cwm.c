











#include "cprecomp.h"



void zfCwmInit(zdev_t* dev) {
    
    zmw_get_wlan_dev(dev);

    switch (wd->wlanMode) {
    case ZM_MODE_AP:
        wd->cwm.cw_mode = CWM_MODE2040;
        wd->cwm.cw_width = CWM_WIDTH40;
        wd->cwm.cw_enable = 1;
        break;
    case ZM_MODE_INFRASTRUCTURE:
    case ZM_MODE_PSEUDO:
    case ZM_MODE_IBSS:
    default:
        wd->cwm.cw_mode = CWM_MODE2040;
        wd->cwm.cw_width = CWM_WIDTH20;
        wd->cwm.cw_enable = 1;
        break;
    }
}


void zfCoreCwmBusy(zdev_t* dev, u16_t busy)
{

    zmw_get_wlan_dev(dev);

    zm_msg1_mm(ZM_LV_0, "CwmBusy=", busy);

    if(wd->cwm.cw_mode == CWM_MODE20) {
        wd->cwm.cw_width = CWM_WIDTH20;
        return;
    }

    if(wd->cwm.cw_mode == CWM_MODE40) {
        wd->cwm.cw_width = CWM_WIDTH40;
        return;
    }

    if (busy) {
        wd->cwm.cw_width = CWM_WIDTH20;
        return;
    }


    if((wd->wlanMode == ZM_MODE_INFRASTRUCTURE || wd->wlanMode == ZM_MODE_PSEUDO ||
        wd->wlanMode == ZM_MODE_IBSS)) {
        if ((wd->sta.ie.HtCap.HtCapInfo & HTCAP_SupChannelWidthSet) &&
            (wd->sta.ie.HtInfo.ChannelInfo & ExtHtCap_RecomTxWidthSet) &&
            (wd->sta.ie.HtInfo.ChannelInfo & ExtHtCap_ExtChannelOffsetAbove)) {

            wd->cwm.cw_width = CWM_WIDTH40;
        }
        else {
            wd->cwm.cw_width = CWM_WIDTH20;
        }

        return;
    }

    if(wd->wlanMode == ZM_MODE_AP) {
        wd->cwm.cw_width = CWM_WIDTH40;
    }

}




u16_t zfCwmIsExtChanBusy(u32_t ctlBusy, u32_t extBusy)
{
    u32_t busy; 
    u32_t cycleTime, ctlClear;

    cycleTime = 1280000; 

    if (cycleTime > ctlBusy) {
        ctlClear = cycleTime - ctlBusy;
    }
    else
    {
        ctlClear = 0;
    }

    
    if (ctlClear) {
        busy = (extBusy * 100) / ctlClear;
    } else {
        busy = 0;
    }
    if (busy > ATH_CWM_EXTCH_BUSY_THRESHOLD) {
        return TRUE;
    }

    return FALSE;
}
