









#include "oal_dt.h"
#include "usbdrv.h"

#include <linux/netlink.h>
#include <net/iw_handler.h>

extern void zfiRecv80211(zdev_t* dev, zbuf_t* buf, struct zsAdditionInfo* addInfo);
extern void zfCoreRecv(zdev_t* dev, zbuf_t* buf, struct zsAdditionInfo* addInfo);
extern void zfIdlChkRsp(zdev_t* dev, u32_t* rsp, u16_t rspLen);
extern void zfIdlRsp(zdev_t* dev, u32_t *rsp, u16_t rspLen);




extern struct zsVapStruct vap[ZM_VAP_PORT_NUMBER];

u32_t zfLnxUsbSubmitTxData(zdev_t* dev);
u32_t zfLnxUsbIn(zdev_t* dev, urb_t *urb, zbuf_t *buf);
u32_t zfLnxSubmitRegInUrb(zdev_t *dev);
u32_t zfLnxUsbSubmitBulkUrb(urb_t *urb, struct usb_device *usb, u16_t epnum, u16_t direction,
        void *transfer_buffer, int buffer_length, usb_complete_t complete, void *context);
u32_t zfLnxUsbSubmitIntUrb(urb_t *urb, struct usb_device *usb, u16_t epnum, u16_t direction,
        void *transfer_buffer, int buffer_length, usb_complete_t complete, void *context,
        u32_t interval);

u16_t zfLnxGetFreeTxUrb(zdev_t *dev)
{
    struct usbdrv_private *macp = dev->ml_priv;
    u16_t idx;
    unsigned long irqFlag;

    spin_lock_irqsave(&macp->cs_lock, irqFlag);

    

    
    if (macp->TxUrbCnt != 0)
    {
        idx = macp->TxUrbTail;
        macp->TxUrbTail = ((macp->TxUrbTail + 1) & (ZM_MAX_TX_URB_NUM - 1));
        macp->TxUrbCnt--;
    }
    else
    {
        
        idx = 0xffff;
    }

    spin_unlock_irqrestore(&macp->cs_lock, irqFlag);
    return idx;
}

void zfLnxPutTxUrb(zdev_t *dev)
{
    struct usbdrv_private *macp = dev->ml_priv;
    u16_t idx;
    unsigned long irqFlag;

    spin_lock_irqsave(&macp->cs_lock, irqFlag);

    idx = ((macp->TxUrbHead + 1) & (ZM_MAX_TX_URB_NUM - 1));

    
    if (macp->TxUrbCnt < ZM_MAX_TX_URB_NUM)
    {
        macp->TxUrbHead = idx;
        macp->TxUrbCnt++;
    }
    else
    {
        printk("UsbTxUrbQ inconsistent: TxUrbHead: %d, TxUrbTail: %d\n",
                macp->TxUrbHead, macp->TxUrbTail);
    }

    spin_unlock_irqrestore(&macp->cs_lock, irqFlag);
}

u16_t zfLnxCheckTxBufferCnt(zdev_t *dev)
{
    struct usbdrv_private *macp = dev->ml_priv;
    u16_t TxBufCnt;
    unsigned long irqFlag;

    spin_lock_irqsave(&macp->cs_lock, irqFlag);

    TxBufCnt = macp->TxBufCnt;

    spin_unlock_irqrestore(&macp->cs_lock, irqFlag);
    return TxBufCnt;
}

UsbTxQ_t *zfLnxGetUsbTxBuffer(zdev_t *dev)
{
    struct usbdrv_private *macp = dev->ml_priv;
    u16_t idx;
    UsbTxQ_t *TxQ;
    unsigned long irqFlag;

    spin_lock_irqsave(&macp->cs_lock, irqFlag);

    idx = ((macp->TxBufHead+1) & (ZM_MAX_TX_BUF_NUM - 1));

    
    if (macp->TxBufCnt > 0)
    {
        
        TxQ = (UsbTxQ_t *)&(macp->UsbTxBufQ[macp->TxBufHead]);
        macp->TxBufHead = ((macp->TxBufHead+1) & (ZM_MAX_TX_BUF_NUM - 1));
        macp->TxBufCnt--;
    }
    else
    {
        if (macp->TxBufHead != macp->TxBufTail)
        {
            printk(KERN_ERR "zfwGetUsbTxBuf UsbTxBufQ inconsistent: TxBufHead: %d, TxBufTail: %d\n",
                    macp->TxBufHead, macp->TxBufTail);
        }

        spin_unlock_irqrestore(&macp->cs_lock, irqFlag);
        return NULL;
    }

    spin_unlock_irqrestore(&macp->cs_lock, irqFlag);
    return TxQ;
}

u16_t zfLnxPutUsbTxBuffer(zdev_t *dev, u8_t *hdr, u16_t hdrlen,
        u8_t *snap, u16_t snapLen, u8_t *tail, u16_t tailLen,
        zbuf_t *buf, u16_t offset)
{
    struct usbdrv_private *macp = dev->ml_priv;
    u16_t idx;
    UsbTxQ_t *TxQ;
    unsigned long irqFlag;

    spin_lock_irqsave(&macp->cs_lock, irqFlag);

    idx = ((macp->TxBufTail+1) & (ZM_MAX_TX_BUF_NUM - 1));

    
    

    
    if (macp->TxBufCnt < ZM_MAX_TX_BUF_NUM)
    {
        
        TxQ = (UsbTxQ_t *)&(macp->UsbTxBufQ[macp->TxBufTail]);
        memcpy(TxQ->hdr, hdr, hdrlen);
        TxQ->hdrlen = hdrlen;
        memcpy(TxQ->snap, snap, snapLen);
        TxQ->snapLen = snapLen;
        memcpy(TxQ->tail, tail, tailLen);
        TxQ->tailLen = tailLen;
        TxQ->buf = buf;
        TxQ->offset = offset;

        macp->TxBufTail = ((macp->TxBufTail+1) & (ZM_MAX_TX_BUF_NUM - 1));
        macp->TxBufCnt++;
    }
    else
    {
        printk(KERN_ERR "zfLnxPutUsbTxBuffer UsbTxBufQ inconsistent: TxBufHead: %d, TxBufTail: %d, TxBufCnt: %d\n",
            macp->TxBufHead, macp->TxBufTail, macp->TxBufCnt);
        spin_unlock_irqrestore(&macp->cs_lock, irqFlag);
        return 0xffff;
    }

    spin_unlock_irqrestore(&macp->cs_lock, irqFlag);
    return 0;
}

zbuf_t *zfLnxGetUsbRxBuffer(zdev_t *dev)
{
    struct usbdrv_private *macp = dev->ml_priv;
    
    zbuf_t *buf;
    unsigned long irqFlag;

    spin_lock_irqsave(&macp->cs_lock, irqFlag);

    

    
    if (macp->RxBufCnt != 0)
    {
        buf = macp->UsbRxBufQ[macp->RxBufHead];
        macp->RxBufHead = ((macp->RxBufHead+1) & (ZM_MAX_RX_URB_NUM - 1));
        macp->RxBufCnt--;
    }
    else
    {
        printk("RxBufQ inconsistent: RxBufHead: %d, RxBufTail: %d\n",
                macp->RxBufHead, macp->RxBufTail);
        spin_unlock_irqrestore(&macp->cs_lock, irqFlag);
        return NULL;
    }

    spin_unlock_irqrestore(&macp->cs_lock, irqFlag);
    return buf;
}

u32_t zfLnxPutUsbRxBuffer(zdev_t *dev, zbuf_t *buf)
{
    struct usbdrv_private *macp = dev->ml_priv;
    u16_t idx;
    unsigned long irqFlag;

    spin_lock_irqsave(&macp->cs_lock, irqFlag);

    idx = ((macp->RxBufTail+1) & (ZM_MAX_RX_URB_NUM - 1));

    
    if (macp->RxBufCnt != ZM_MAX_RX_URB_NUM)
    {
        macp->UsbRxBufQ[macp->RxBufTail] = buf;
        macp->RxBufTail = idx;
        macp->RxBufCnt++;
    }
    else
    {
        printk("RxBufQ inconsistent: RxBufHead: %d, RxBufTail: %d\n",
                macp->RxBufHead, macp->RxBufTail);
        spin_unlock_irqrestore(&macp->cs_lock, irqFlag);
        return 0xffff;
    }

    spin_unlock_irqrestore(&macp->cs_lock, irqFlag);
    return 0;
}

void zfLnxUsbDataOut_callback(urb_t *urb)
{
    zdev_t* dev = urb->context;
    

    
    zfLnxPutTxUrb(dev);

    
    
    if (zfLnxCheckTxBufferCnt(dev) != 0)
    {
        

        
        
        
        
        
        
        
            zfLnxUsbSubmitTxData(dev);
        
    }
}

void zfLnxUsbDataIn_callback(urb_t *urb)
{
    zdev_t* dev = urb->context;
    struct usbdrv_private *macp = dev->ml_priv;
    zbuf_t *buf;
    zbuf_t *new_buf;
    int status;

#if ZM_USB_STREAM_MODE == 1
    static int remain_len = 0, check_pad = 0, check_len = 0;
    int index = 0;
    int chk_idx;
    u16_t pkt_len;
    u16_t pkt_tag;
    u16_t ii;
    zbuf_t *rxBufPool[8];
    u16_t rxBufPoolIndex = 0;
#endif

    
    if (urb->status != 0){
        printk("zfLnxUsbDataIn_callback() : status=0x%x\n", urb->status);
        if ((urb->status != -ENOENT) && (urb->status != -ECONNRESET)
            && (urb->status != -ESHUTDOWN))
        {
                if (urb->status == -EPIPE){
                    
                    status = -1;
                }

                if (urb->status == -EPROTO){
                    
                    status = -1;
                }
        }

        

        
        buf = zfLnxGetUsbRxBuffer(dev);
        dev_kfree_skb_any(buf);
        #if 0
        
        zfLnxPutUsbRxBuffer(dev, buf);

        
        zfLnxUsbIn(dev, urb, buf);
        #endif
        return;
    }

    if (urb->actual_length == 0)
    {
        printk(KERN_ERR "Get an URB whose length is zero");
        status = -1;
    }

    
    buf = zfLnxGetUsbRxBuffer(dev);

    
#ifdef NET_SKBUFF_DATA_USES_OFFSET
    buf->tail = 0;
    buf->len = 0;
#else
    buf->tail = buf->data;
    buf->len = 0;
#endif

    BUG_ON((buf->tail + urb->actual_length) > buf->end);

    skb_put(buf, urb->actual_length);

#if ZM_USB_STREAM_MODE == 1
    if (remain_len != 0)
    {
        zbuf_t *remain_buf = macp->reamin_buf;

        index = remain_len;
        remain_len -= check_pad;

        
        memcpy(&(remain_buf->data[check_len]), buf->data, remain_len);
        check_len += remain_len;
        remain_len = 0;

        rxBufPool[rxBufPoolIndex++] = remain_buf;
    }

    while(index < urb->actual_length)
    {
        pkt_len = buf->data[index] + (buf->data[index+1] << 8);
        pkt_tag = buf->data[index+2] + (buf->data[index+3] << 8);

        if (pkt_tag == 0x4e00)
        {
            int pad_len;

            
            #if 0
            
            for (ii = index; ii < pkt_len+4;)
            {
                printk("%02x ", (buf->data[ii] & 0xff));

                if ((++ii % 16) == 0)
                    printk("\n");
            }

            printk("\n");
            #endif

            pad_len = 4 - (pkt_len & 0x3);

            if(pad_len == 4)
                pad_len = 0;

            chk_idx = index;
            index = index + 4 + pkt_len + pad_len;

            if (index > ZM_MAX_RX_BUFFER_SIZE)
            {
                remain_len = index - ZM_MAX_RX_BUFFER_SIZE; 
                check_len = ZM_MAX_RX_BUFFER_SIZE - chk_idx - 4;
                check_pad = pad_len;

                
                
                new_buf = dev_alloc_skb(ZM_MAX_RX_BUFFER_SIZE);

                
            #ifdef NET_SKBUFF_DATA_USES_OFFSET
                new_buf->tail = 0;
                new_buf->len = 0;
            #else
                new_buf->tail = new_buf->data;
                new_buf->len = 0;
            #endif

                skb_put(new_buf, pkt_len);

                
                memcpy(new_buf->data, &(buf->data[chk_idx+4]), check_len);

                
                macp->reamin_buf = new_buf;
            }
            else
            {
        #ifdef ZM_DONT_COPY_RX_BUFFER
                if (rxBufPoolIndex == 0)
                {
                    new_buf = skb_clone(buf, GFP_ATOMIC);

                    new_buf->data = &(buf->data[chk_idx+4]);
                    new_buf->len = pkt_len;
                }
                else
                {
        #endif
                
                new_buf = dev_alloc_skb(ZM_MAX_RX_BUFFER_SIZE);

                
            #ifdef NET_SKBUFF_DATA_USES_OFFSET
                new_buf->tail = 0;
                new_buf->len = 0;
            #else
                new_buf->tail = new_buf->data;
                new_buf->len = 0;
            #endif

                skb_put(new_buf, pkt_len);

                
                memcpy(new_buf->data, &(buf->data[chk_idx+4]), pkt_len);

        #ifdef ZM_DONT_COPY_RX_BUFFER
                }
        #endif
                rxBufPool[rxBufPoolIndex++] = new_buf;
            }
        }
        else
        {
            printk(KERN_ERR "Can't find tag, pkt_len: 0x%04x, tag: 0x%04x\n", pkt_len, pkt_tag);

            
            dev_kfree_skb_any(buf);

            
            new_buf = dev_alloc_skb(ZM_MAX_RX_BUFFER_SIZE);

            
            zfLnxPutUsbRxBuffer(dev, new_buf);

            
            zfLnxUsbIn(dev, urb, new_buf);

            return;
        }
    }

    
    dev_kfree_skb_any(buf);
#endif

    
    new_buf = dev_alloc_skb(ZM_MAX_RX_BUFFER_SIZE);

    
    zfLnxPutUsbRxBuffer(dev, new_buf);

    
    zfLnxUsbIn(dev, urb, new_buf);

#if ZM_USB_STREAM_MODE == 1
    for(ii = 0; ii < rxBufPoolIndex; ii++)
    {
        macp->usbCbFunctions.zfcbUsbRecv(dev, rxBufPool[ii]);
    }
#else
    
    macp->usbCbFunctions.zfcbUsbRecv(dev, buf);
#endif
}

void zfLnxUsbRegOut_callback(urb_t *urb)
{
    

    
}

void zfLnxUsbRegIn_callback(urb_t *urb)
{
    zdev_t* dev = urb->context;
    u32_t rsp[64/4];
    int status;
    struct usbdrv_private *macp = dev->ml_priv;

    
    if (urb->status != 0){
        printk("zfLnxUsbRegIn_callback() : status=0x%x\n", urb->status);
        if ((urb->status != -ENOENT) && (urb->status != -ECONNRESET)
            && (urb->status != -ESHUTDOWN))
        {
                if (urb->status == -EPIPE){
                    
                    status = -1;
                }

                if (urb->status == -EPROTO){
                    
                    status = -1;
                }
        }

        
        return;
    }

    if (urb->actual_length == 0)
    {
        printk(KERN_ERR "Get an URB whose length is zero");
        status = -1;
    }

    
    memcpy(rsp, macp->regUsbReadBuf, urb->actual_length);

    
    
    
    macp->usbCbFunctions.zfcbUsbRegIn(dev, rsp, (u16_t)urb->actual_length);

    
    zfLnxSubmitRegInUrb(dev);
}

u32_t zfLnxSubmitRegInUrb(zdev_t *dev)
{
    u32_t ret;
    struct usbdrv_private *macp = dev->ml_priv;

    
    
    
    
    
    
    

    ret = zfLnxUsbSubmitIntUrb(macp->RegInUrb, macp->udev,
            USB_REG_IN_PIPE, USB_DIR_IN, macp->regUsbReadBuf,
            ZM_USB_REG_MAX_BUF_SIZE, zfLnxUsbRegIn_callback, dev, 1);

    return ret;
}

u32_t zfLnxUsbSubmitTxData(zdev_t* dev)
{
    u32_t i;
    u32_t ret;
    u16_t freeTxUrb;
    u8_t *puTxBuf = NULL;
    UsbTxQ_t *TxData;
    int len = 0;
    struct usbdrv_private *macp = dev->ml_priv;
#if ZM_USB_TX_STREAM_MODE == 1
    u8_t               ii;
    u16_t              offset = 0;
    u16_t              usbTxAggCnt;
    u16_t              *pUsbTxHdr;
    UsbTxQ_t           *TxQPool[ZM_MAX_TX_AGGREGATE_NUM];
#endif

    
    freeTxUrb = zfLnxGetFreeTxUrb(dev);

    
    if (freeTxUrb == 0xffff)
    {
        
        
        return 0xffff;
    }

#if ZM_USB_TX_STREAM_MODE == 1
    usbTxAggCnt = zfLnxCheckTxBufferCnt(dev);

    if (usbTxAggCnt >= ZM_MAX_TX_AGGREGATE_NUM)
    {
       usbTxAggCnt = ZM_MAX_TX_AGGREGATE_NUM;
    }
    else
    {
       usbTxAggCnt = 1;
    }

    
#endif

#if ZM_USB_TX_STREAM_MODE == 1
    for(ii = 0; ii < usbTxAggCnt; ii++)
    {
#endif
    
    TxData = zfLnxGetUsbTxBuffer(dev);
    if (TxData == NULL)
    {
        
        zfLnxPutTxUrb(dev);
        return 0xffff;
    }

    
    puTxBuf = macp->txUsbBuf[freeTxUrb];

#if ZM_USB_TX_STREAM_MODE == 1
    puTxBuf += offset;
    pUsbTxHdr = (u16_t *)puTxBuf;

    
    *pUsbTxHdr++ = TxData->hdrlen + TxData->snapLen +
             (TxData->buf->len - TxData->offset) +  TxData->tailLen;

    *pUsbTxHdr++ = 0x697e;

    puTxBuf += 4;
#endif 

    
    for(i = 0; i < TxData->hdrlen; i++)
    {
        *puTxBuf++ = TxData->hdr[i];
    }

    
    for(i = 0; i < TxData->snapLen; i++)
    {
        *puTxBuf++ = TxData->snap[i];
    }

    
    for(i = 0; i < TxData->buf->len - TxData->offset; i++)
    {
    	
    	*puTxBuf++ = *(u8_t*)((u8_t*)TxData->buf->data+i+TxData->offset);
    }

    
    for(i = 0; i < TxData->tailLen; i++)
    {
        *puTxBuf++ = TxData->tail[i];
    }

    len = TxData->hdrlen+TxData->snapLen+TxData->buf->len+TxData->tailLen-TxData->offset;

    #if 0
    if (TxData->hdrlen != 0)
    {
        puTxBuf = macp->txUsbBuf[freeTxUrb];
        for (i = 0; i < len; i++)
        {
            printk("%02x ", puTxBuf[i]);
            if (i % 16 == 15)
                printk("\n");
        }
        printk("\n");
    }
    #endif
    #if 0
    
    if(TxData->hdr[9] & 0x40)
    {
        int i;
        u16_t ctrlLen = TxData->hdr[0] + (TxData->hdr[1] << 8);

        if (ctrlLen != len + 4)
        {
        
        for(i = 0; i < 8; i++)
        {
            printk(KERN_ERR "0x%02x ", TxData->hdr[i]);
        }
        printk(KERN_ERR "\n");

        printk(KERN_ERR "ctrLen: %d, hdrLen: %d, snapLen: %d\n", ctrlLen, TxData->hdrlen, TxData->snapLen);
        printk(KERN_ERR "bufLen: %d, tailLen: %d, len: %d\n", TxData->buf->len, TxData->tailLen, len);
        }
    }
    #endif

#if ZM_USB_TX_STREAM_MODE == 1
    
    len += 4;

    

    if (ii < (ZM_MAX_TX_AGGREGATE_NUM-1))
    {
        
        offset += (((len-1) / 4) + 1) * 4;
    }

    if (ii == (ZM_MAX_TX_AGGREGATE_NUM-1))
    {
        len += offset;
    }

    TxQPool[ii] = TxData;

    

    
    
    }
#endif
    
    
    ret = zfLnxUsbSubmitBulkUrb(macp->WlanTxDataUrb[freeTxUrb], macp->udev,
            USB_WLAN_TX_PIPE, USB_DIR_OUT, macp->txUsbBuf[freeTxUrb],
            len, zfLnxUsbDataOut_callback, dev);
    
    
    

    
    
#if ZM_USB_TX_STREAM_MODE == 1
    for(ii = 0; ii < usbTxAggCnt; ii++)
        macp->usbCbFunctions.zfcbUsbOutComplete(dev, TxQPool[ii]->buf, 1, TxQPool[ii]->hdr);
#else
    macp->usbCbFunctions.zfcbUsbOutComplete(dev, TxData->buf, 1, TxData->hdr);
#endif

    return ret;
}



u32_t zfLnxUsbIn(zdev_t* dev, urb_t *urb, zbuf_t *buf)
{
    u32_t ret;
    struct usbdrv_private *macp = dev->ml_priv;

    
    ret = zfLnxUsbSubmitBulkUrb(urb, macp->udev, USB_WLAN_RX_PIPE,
            USB_DIR_IN, buf->data, ZM_MAX_RX_BUFFER_SIZE,
            zfLnxUsbDataIn_callback, dev);
    
    
    

    return ret;
}

u32_t zfLnxUsbWriteReg(zdev_t* dev, u32_t* cmd, u16_t cmdLen)
{
    struct usbdrv_private *macp = dev->ml_priv;
    u32_t ret;

#ifdef ZM_CONFIG_BIG_ENDIAN
    int ii = 0;

    for(ii=0; ii<(cmdLen>>2); ii++)
	cmd[ii] = cpu_to_le32(cmd[ii]);
#endif

    memcpy(macp->regUsbWriteBuf, cmd, cmdLen);

    
    
    ret = zfLnxUsbSubmitIntUrb(macp->RegOutUrb, macp->udev,
            USB_REG_OUT_PIPE, USB_DIR_OUT, macp->regUsbWriteBuf,
            cmdLen, zfLnxUsbRegOut_callback, dev, 1);

    return ret;
}


u32_t zfLnxUsbOut(zdev_t* dev, u8_t *hdr, u16_t hdrlen, u8_t *snap, u16_t snapLen,
        u8_t *tail, u16_t tailLen, zbuf_t *buf, u16_t offset)
{
    u32_t ret;
    struct usbdrv_private *macp = dev->ml_priv;

    
    

    
    if (zfLnxPutUsbTxBuffer(dev, hdr, hdrlen, snap, snapLen, tail, tailLen, buf, offset) == 0xffff)
    {
        
        
        
        macp->usbCbFunctions.zfcbUsbOutComplete(dev, buf, 0, hdr);
        return 0xffff;
    }

    
    
    ret = zfLnxUsbSubmitTxData(dev);
    return ret;
}

void zfLnxInitUsbTxQ(zdev_t* dev)
{
    struct usbdrv_private *macp = dev->ml_priv;

    printk(KERN_ERR "zfwInitUsbTxQ\n");

    
    memset(macp->UsbTxBufQ, 0, sizeof(UsbTxQ_t) * ZM_MAX_TX_URB_NUM);

    macp->TxBufHead = 0;
    macp->TxBufTail = 0;
    macp->TxUrbHead = 0;
    macp->TxUrbTail = 0;
    macp->TxUrbCnt = ZM_MAX_TX_URB_NUM;
}

void zfLnxInitUsbRxQ(zdev_t* dev)
{
    u16_t i;
    zbuf_t *buf;
    struct usbdrv_private *macp = dev->ml_priv;

    
    memset(macp->UsbRxBufQ, 0, sizeof(zbuf_t *) * ZM_MAX_RX_URB_NUM);

    macp->RxBufHead = 0;

    for (i = 0; i < ZM_MAX_RX_URB_NUM; i++)
    {
        
        buf = dev_alloc_skb(ZM_MAX_RX_BUFFER_SIZE);
        macp->UsbRxBufQ[i] = buf;
    }

    
    macp->RxBufTail = 0;

    
    for (i = 0; i < ZM_MAX_RX_URB_NUM; i++)
    {
        zfLnxPutUsbRxBuffer(dev, macp->UsbRxBufQ[i]);
        zfLnxUsbIn(dev, macp->WlanRxDataUrb[i], macp->UsbRxBufQ[i]);
    }
}



u32_t zfLnxUsbSubmitBulkUrb(urb_t *urb, struct usb_device *usb, u16_t epnum, u16_t direction,
        void *transfer_buffer, int buffer_length, usb_complete_t complete, void *context)
{
    u32_t ret;

    if(direction == USB_DIR_OUT)
    {
        usb_fill_bulk_urb(urb, usb, usb_sndbulkpipe(usb, epnum),
                transfer_buffer, buffer_length, complete, context);

        urb->transfer_flags |= URB_ZERO_PACKET;
    }
    else
    {
        usb_fill_bulk_urb(urb, usb, usb_rcvbulkpipe(usb, epnum),
                transfer_buffer, buffer_length, complete, context);
    }

    if (epnum == 4)
    {
        if (urb->hcpriv)
        {
            
            
        }
    }

    ret = usb_submit_urb(urb, GFP_ATOMIC);
    if ((epnum == 4) & (ret != 0))
    {
        
    }
    return ret;
}

u32_t zfLnxUsbSubmitIntUrb(urb_t *urb, struct usb_device *usb, u16_t epnum, u16_t direction,
        void *transfer_buffer, int buffer_length, usb_complete_t complete, void *context,
        u32_t interval)
{
    u32_t ret;

    if(direction == USB_DIR_OUT)
    {
        usb_fill_int_urb(urb, usb, usb_sndbulkpipe(usb, epnum),
                transfer_buffer, buffer_length, complete, context, interval);
    }
    else
    {
        usb_fill_int_urb(urb, usb, usb_rcvbulkpipe(usb, epnum),
                transfer_buffer, buffer_length, complete, context, interval);
    }

    ret = usb_submit_urb(urb, GFP_ATOMIC);

    return ret;
}

#ifdef ZM_ENABLE_CENC
int zfLnxCencSendMsg(struct sock *netlink_sk, u_int8_t *msg, int len)
{
#define COMMTYPE_GROUP   8
#define WAI_K_MSG        0x11

	int ret = -1;
	int size;
	unsigned char *old_tail;
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	char *pos = NULL;

	size = NLMSG_SPACE(len);
	skb = alloc_skb(size, GFP_ATOMIC);

	if(skb == NULL)
	{
		printk("dev_alloc_skb failure \n");
		goto out;
	}
	old_tail = skb->tail;

	
	nlh = NLMSG_PUT(skb, 0, 0, WAI_K_MSG, size-sizeof(*nlh));
	pos = NLMSG_DATA(nlh);
	memset(pos, 0, len);

	
	memcpy(pos, msg,  len);
	
	nlh->nlmsg_len = skb->tail - old_tail;
	NETLINK_CB(skb).dst_group = COMMTYPE_GROUP;
	netlink_broadcast(netlink_sk, skb, 0, COMMTYPE_GROUP, GFP_ATOMIC);
	ret = 0;
out:
	return ret;
nlmsg_failure: 
	kfree_skb(skb);
	goto out;

#undef COMMTYPE_GROUP
#undef WAI_K_MSG
}
#endif 


u16_t zfLnxGetVapId(zdev_t* dev)
{
    u16_t i;

    for (i=0; i<ZM_VAP_PORT_NUMBER; i++)
    {
        if (vap[i].dev == dev)
        {
            return i;
        }
    }
    return 0xffff;
}

u32_t zfwReadReg(zdev_t* dev, u32_t offset)
{
    return 0;
}

#ifndef INIT_WORK
#define work_struct tq_struct

#define schedule_work(a)  schedule_task(a)

#define flush_scheduled_work  flush_scheduled_tasks
#define INIT_WORK(_wq, _routine, _data)  INIT_TQUEUE(_wq, _routine, _data)
#define PREPARE_WORK(_wq, _routine, _data)  PREPARE_TQUEUE(_wq, _routine, _data)
#endif

#define KEVENT_WATCHDOG        0x00000001

u32_t smp_kevent_Lock = 0;

void kevent(struct work_struct *work)
{
    struct usbdrv_private *macp =
               container_of(work, struct usbdrv_private, kevent);
    zdev_t *dev = macp->device;

    if (test_and_set_bit(0, (void *)&smp_kevent_Lock))
    {
        
        return;
    }

    down(&macp->ioctl_sem);

    if (test_and_clear_bit(KEVENT_WATCHDOG, &macp->kevent_flags))
    {
    extern u16_t zfHpStartRecv(zdev_t *dev);
        
        printk(("\n ************ Hw watchDog occur!! ************** \n"));
        zfiWlanSuspend(dev);
        zfiWlanResume(dev,0);
        zfHpStartRecv(dev);
    }

    clear_bit(0, (void *)&smp_kevent_Lock);
    up(&macp->ioctl_sem);
}
















u8_t zfLnxCreateThread(zdev_t *dev)
{
    struct usbdrv_private *macp = dev->ml_priv;

    
    INIT_WORK(&macp->kevent, kevent);
    init_MUTEX(&macp->ioctl_sem);

    return 0;
}

















void zfLnxSignalThread(zdev_t *dev, int flag)
{
    struct usbdrv_private *macp = dev->ml_priv;

    if (macp == NULL)
    {
        printk("macp is NULL\n");
        return;
    }

    if (0 && macp->kevent_ready != 1)
    {
        printk("Kevent not ready\n");
        return;
    }

    set_bit(flag, &macp->kevent_flags);

    if (!schedule_work(&macp->kevent))
    {
        
        
    }
}



void zfLnxWatchDogNotify(zdev_t* dev)
{
    zfLnxSignalThread(dev, KEVENT_WATCHDOG);
}


void zfwGetActiveScanDur(zdev_t* dev, u8_t* Dur)
{
    *Dur = 30; 
}

void zfwGetShowZeroLengthSSID(zdev_t* dev, u8_t* Dur)
{
    *Dur = 0;
}

