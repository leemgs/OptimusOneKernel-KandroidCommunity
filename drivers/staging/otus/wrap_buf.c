











#include "oal_dt.h"
#include "usbdrv.h"


#include <linux/netlink.h>

#include <net/iw_handler.h>



zbuf_t* zfwBufAllocate(zdev_t* dev, u16_t len)
{
    zbuf_t* buf;

    
    buf = dev_alloc_skb(len);

    return buf;
}



void zfwBufFree(zdev_t* dev, zbuf_t* buf, u16_t status)
{
    dev_kfree_skb_any(buf);
}


u16_t zfwBufRemoveHead(zdev_t* dev, zbuf_t* buf, u16_t size)
{
    

    buf->data += size;
    buf->len -= size;
    return 0;
}








u16_t zfwBufChain(zdev_t* dev, zbuf_t** head, zbuf_t* tail)
{

    *head = tail;
    return 0;
}



u16_t zfwBufCopy(zdev_t* dev, zbuf_t* dst, zbuf_t* src)
{
    memcpy(dst->data, src->data, src->len);
    dst->tail = dst->data;
    skb_put(dst, src->len);
    return 0;
}



u16_t zfwBufSetSize(zdev_t* dev, zbuf_t* buf, u16_t size)
{
#ifdef NET_SKBUFF_DATA_USES_OFFSET
    buf->tail = 0;
    buf->len = 0;
#else
    buf->tail = buf->data;
    buf->len = 0;
#endif

    skb_put(buf, size);
    return 0;
}

u16_t zfwBufGetSize(zdev_t* dev, zbuf_t* buf)
{
    return buf->len;
}

void zfwCopyBufContext(zdev_t* dev, zbuf_t* source, zbuf_t* dst)
{
}
