











#include "oal_dt.h"
#include "usbdrv.h"

#include <linux/netlink.h>
#include <net/iw_handler.h>

#ifdef ZM_ENABLE_CENC
extern int zfLnxCencSendMsg(struct sock *netlink_sk, u_int8_t *msg, int len);

u16_t zfLnxCencAsocNotify(zdev_t* dev, u16_t* macAddr, u8_t* body, u16_t bodySize, u16_t port)
{
    struct usbdrv_private *macp = (struct usbdrv_private *)dev->priv;
    struct zydas_cenc_sta_info cenc_info;
    
    u8_t ie_len;
    int ii;

    
    

    if (macp->netlink_sk == NULL)
    {
        printk(KERN_ERR "NETLINK Socket is NULL\n");
        return -1;
    }

    memset(&cenc_info, 0, sizeof(cenc_info));

    
    zfiWlanQueryGSN(dev, cenc_info.gsn, port);
    cenc_info.datalen += ZM_CENC_IV_LEN;
    ie_len = body[1] + 2;
    memcpy(cenc_info.wie, body, ie_len);
    cenc_info.datalen += ie_len;

    memcpy(cenc_info.sta_mac, macAddr, 6);
    cenc_info.msg_type = ZM_CENC_WAI_REQUEST;
    cenc_info.datalen += 6 + 2;

    printk(KERN_ERR "===== zfwCencSendMsg, bodySize: %d =====\n", bodySize);

    for(ii = 0; ii < bodySize; ii++)
    {
        printk(KERN_ERR "%02x ", body[ii]);

        if ((ii & 0xf) == 0xf)
        {
            printk(KERN_ERR "\n");
        }
    }

    zfLnxCencSendMsg(macp->netlink_sk, (u8_t *)&cenc_info, cenc_info.datalen+4);

    
    

    return 0;
}
#endif 

u8_t zfwCencHandleBeaconProbrespon(zdev_t* dev, u8_t *pWIEc,
        u8_t *pPeerSSIDc, u8_t *pPeerAddrc)
{
    return 0;
}

u8_t zfwGetPktEncExemptionActionType(zdev_t* dev, zbuf_t* buf)
{
    return ZM_ENCRYPTION_EXEMPT_NO_EXEMPTION;
}

void copyToIntTxBuffer(zdev_t* dev, zbuf_t* buf, u8_t* src,
                         u16_t offset, u16_t length)
{
    u16_t i;

    for(i=0; i<length;i++)
    {
        
        *(u8_t*)((u8_t*)buf->data+offset+i) = src[i];
    }
}

u16_t zfwStaAddIeWpaRsn(zdev_t* dev, zbuf_t* buf, u16_t offset, u8_t frameType)
{
    struct usbdrv_private *macp = dev->ml_priv;
    
    if (macp->supIe[1] != 0)
    {
        copyToIntTxBuffer(dev, buf, macp->supIe, offset, macp->supIe[1]+2);
        
        offset += (macp->supIe[1]+2);
    }

    return offset;
}


