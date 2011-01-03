










#include "oal_dt.h"
#include "usbdrv.h"

#include <linux/netlink.h>
#include <net/iw_handler.h>




void* zfwMemAllocate(zdev_t* dev, u32_t size)
{
    void* mem = NULL;
    mem = kmalloc(size, GFP_ATOMIC);
    return mem;
}



void zfwMemFree(zdev_t* dev, void* mem, u32_t size)
{
    kfree(mem);
    return;
}

void zfwMemoryCopy(u8_t* dst, u8_t* src, u16_t length)
{
    

    memcpy(dst, src, length);
    
    
    
    
    return;
}

void zfwZeroMemory(u8_t* va, u16_t length)
{
    
    memset(va, 0, length);
    
    
    
    
    return;
}

void zfwMemoryMove(u8_t* dst, u8_t* src, u16_t length)
{
    memcpy(dst, src, length);
    return;
}

u8_t zfwMemoryIsEqual(u8_t* m1, u8_t* m2, u16_t length)
{
    
    int ret;

    ret = memcmp(m1, m2, length);

    return ((ret==0)?TRUE:FALSE);
    
    
    
    
    
    
    

    
}


