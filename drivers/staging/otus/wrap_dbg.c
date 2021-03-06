










#include "oal_dt.h"
#include "usbdrv.h"

#include <linux/netlink.h>
#include <net/iw_handler.h>

void zfwDumpBuf(zdev_t* dev, zbuf_t* buf)
{
    u16_t i;

    for (i=0; i<buf->len; i++)
    {
        printk("%02x ", *(((u8_t*)buf->data)+i));
        if ((i&0xf)==0xf)
        {
            printk("\n");
        }
    }
    printk("\n");
}


void zfwDbgReadRegDone(zdev_t* dev, u32_t addr, u32_t val)
{
    printk("Read addr:%x = %x\n", addr, val);
}

void zfwDbgWriteRegDone(zdev_t* dev, u32_t addr, u32_t val)
{
    printk("Write addr:%x = %x\n", addr, val);
}

void zfwDbgReadTallyDone(zdev_t* dev)
{
    
}

void zfwDbgWriteEepromDone(zdev_t* dev, u32_t addr, u32_t val)
{
}

void zfwDbgQueryHwTxBusyDone(zdev_t* dev, u32_t val)
{
}


void zfwDbgReadFlashDone(zdev_t* dev, u32_t addr, u32_t* rspdata, u32_t datalen)
{
    printk("Read Flash addr:%x length:%x\n", addr, datalen);
}

void zfwDbgProgrameFlashDone(zdev_t* dev)
{
    printk("Program Flash Done\n");
}

void zfwDbgProgrameFlashChkDone(zdev_t* dev)
{
    printk("Program Flash Done\n");
}

void zfwDbgGetFlashChkSumDone(zdev_t* dev, u32_t* rspdata)
{
    printk("Get Flash ChkSum Done\n");
}

void zfwDbgDownloadFwInitDone(zdev_t* dev)
{
    printk("Download FW Init Done\n");
}



