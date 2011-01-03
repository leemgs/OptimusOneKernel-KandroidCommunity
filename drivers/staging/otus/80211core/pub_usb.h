

#ifndef _PUB_USB_H
#define _PUB_USB_H

#include "../oal_dt.h"

#define ZM_HAL_80211_MODE_AP              0
#define ZM_HAL_80211_MODE_STA             1
#define ZM_HAL_80211_MODE_IBSS_GENERAL    2
#define ZM_HAL_80211_MODE_IBSS_WPA2PSK    3




















struct zfCbUsbFuncTbl {
    void (*zfcbUsbRecv)(zdev_t *dev, zbuf_t *buf);
    void (*zfcbUsbRegIn)(zdev_t* dev, u32_t* rsp, u16_t rspLen);
    void (*zfcbUsbOutComplete)(zdev_t* dev, zbuf_t *buf, u8_t status, u8_t *hdr);
    void (*zfcbUsbRegOutComplete)(zdev_t* dev);
};




















extern u32_t zfwUsbGetMaxTxQSize(zdev_t* dev);


extern u32_t zfwUsbGetFreeTxQSize(zdev_t* dev);


extern void zfwUsbRegisterCallBack(zdev_t* dev, struct zfCbUsbFuncTbl *zfUsbFunc);


extern u32_t zfwUsbEnableIntEpt(zdev_t *dev, u8_t endpt);


extern int zfwUsbEnableRxEpt(zdev_t* dev, u8_t endpt);


extern u32_t zfwUsbSubmitControl(zdev_t* dev, u8_t req, u16_t value,
        u16_t index, void *data, u32_t size);
extern u32_t zfwUsbSubmitControlIo(zdev_t* dev, u8_t req, u8_t reqtype,
        u16_t value, u16_t index, void *data, u32_t size);


extern void zfwUsbCmd(zdev_t* dev, u8_t endpt, u32_t* cmd, u16_t cmdLen);


extern u32_t zfwUsbSend(zdev_t* dev, u8_t endpt, u8_t *hdr, u16_t hdrlen, u8_t *snap, u16_t snapLen,
                u8_t *tail, u16_t tailLen, zbuf_t *buf, u16_t offset);


extern u32_t zfwUsbSetConfiguration(zdev_t *dev, u16_t value);

#endif
