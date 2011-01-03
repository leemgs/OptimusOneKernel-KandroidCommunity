


















#ifndef _CWM_H
#define _CWM_H

#define ATH_CWM_EXTCH_BUSY_THRESHOLD  30  

void zfCwmInit(zdev_t* dev);
void zfCoreCwmBusy(zdev_t* dev, u16_t busy);
u16_t zfCwmIsExtChanBusy(u32_t ctlBusy, u32_t extBusy);



#endif 
