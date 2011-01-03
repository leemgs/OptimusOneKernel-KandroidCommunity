











#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif

#include <linux/module.h>
#include <linux/usb.h>

#include "usbdrv.h"
#include "zdusb.h"

int zfLnxAllocAllUrbs(struct usbdrv_private *macp);
void zfLnxFreeAllUrbs(struct usbdrv_private *macp);
void zfLnxUnlinkAllUrbs(struct usbdrv_private *macp);

MODULE_AUTHOR("Atheros Communications");
MODULE_DESCRIPTION("Atheros 802.11n Wireless LAN adapter");
MODULE_LICENSE("Dual BSD/GPL");

static const char driver_name[] = "Otus";


static struct usb_device_id zd1221_ids [] = {
	{ USB_DEVICE(VENDOR_ATHR, PRODUCT_AR9170) },
        { USB_DEVICE(VENDOR_DLINK, PRODUCT_DWA160A) },
	{ USB_DEVICE(VENDOR_NETGEAR, PRODUCT_WNDA3100) },
	{ USB_DEVICE(VENDOR_NETGEAR, PRODUCT_WN111v2) },
	{ }					
};

MODULE_DEVICE_TABLE(usb, zd1221_ids);

extern u8_t zfLnxInitSetup(struct net_device *dev, struct usbdrv_private *macp);
extern int usbdrv_close(struct net_device *dev);
extern u8_t zfLnxClearStructs(struct net_device *dev);
extern int zfWdsClose(struct net_device *dev);
extern int zfUnregisterWdsDev(struct net_device* parentDev, u16_t wdsId);
extern int zfLnxVapClose(struct net_device *dev);
extern int zfLnxUnregisterVapDev(struct net_device* parentDev, u16_t vapId);


extern struct zsWdsStruct wds[ZM_WDS_PORT_NUMBER];


extern struct zsVapStruct vap[ZM_VAP_PORT_NUMBER];

static int zfLnxProbe(struct usb_interface *interface,
	const struct usb_device_id *id)
{
    struct usb_device *dev = interface_to_usbdev(interface);

    struct net_device *net = NULL;
    struct usbdrv_private *macp = NULL;
    int vendor_id, product_id;
    int result = 0;

    usb_get_dev(dev);

    vendor_id = dev->descriptor.idVendor;
    product_id = dev->descriptor.idProduct;

#ifdef HMAC_DEBUG
    printk(KERN_NOTICE "vendor_id = %04x\n", vendor_id);
    printk(KERN_NOTICE "product_id = %04x\n", product_id);

    if (dev->speed == USB_SPEED_HIGH)
        printk(KERN_NOTICE "USB 2.0 Host\n");
    else
        printk(KERN_NOTICE "USB 1.1 Host\n");
#endif

    if (!(macp = kmalloc(sizeof(struct usbdrv_private), GFP_KERNEL)))
    {
        printk(KERN_ERR "out of memory allocating device structure\n");
        result = -ENOMEM;
        goto fail;
    }

    
    memset(macp, 0, sizeof(struct usbdrv_private));

    net = alloc_etherdev(0);

    if (net == NULL)
    {
        printk(KERN_ERR "zfLnxProbe: Not able to alloc etherdev struct\n");
        result = -ENOMEM;
        goto fail1;
    }

    strcpy(net->name, "ath%d");

    net->ml_priv = macp;   
    macp->udev = dev;
    macp->device = net;

    
    
    macp->interface = interface;

    
    
    

    if (!zfLnxAllocAllUrbs(macp))
    {
        result = -ENOMEM;
        goto fail2;
    }

    if (!zfLnxInitSetup(net, macp))
    {
        result = -EIO;
        goto fail3;
    }
    else
    {
        usb_set_intfdata(interface, macp);
        SET_NETDEV_DEV(net, &interface->dev);

        if (register_netdev(net) != 0)
        {
            usb_set_intfdata(interface, NULL);
            goto fail3;
        }
    }

    netif_carrier_off(net);
    goto done;

fail3:
    zfLnxFreeAllUrbs(macp);
fail2:
    free_netdev(net);  
fail1:
    kfree(macp);

fail:
    usb_put_dev(dev);
    macp = NULL;

done:
    return result;
}

static void zfLnxDisconnect(struct usb_interface *interface)
{
    struct usbdrv_private *macp = (struct usbdrv_private *) usb_get_intfdata(interface);

    printk(KERN_DEBUG "zfLnxDisconnect\n");

    if (!macp)
    {
        printk(KERN_ERR "unregistering non-existant device\n");
        return;
    }

    if (macp->driver_isolated)
    {
        if (macp->device->flags & IFF_UP)
            usbdrv_close(macp->device);
    }

#if 0
    
    
    
    

    
    zfLnxVapClose(vap[0].dev);
    
    zfLnxUnregisterVapDev(macp->device, 0);
#endif

    zfLnxClearStructs(macp->device);

    unregister_netdev(macp->device);

    usb_put_dev(interface_to_usbdev(interface));

    
    

    
    free_netdev(macp->device);

    zfLnxFreeAllUrbs(macp);
    
    kfree(macp);
    macp = NULL;

    usb_set_intfdata(interface, NULL);
}

static struct usb_driver zd1221_driver = {
	.name         = driver_name,
	.probe        = zfLnxProbe,
	.disconnect   = zfLnxDisconnect,
	.id_table     = zd1221_ids,
};

int __init zfLnxIinit(void)
{
    printk(KERN_NOTICE "%s - version %s\n",  DRIVER_NAME, VERSIONID);
    return usb_register(&zd1221_driver);
}

void __exit zfLnxExit(void)
{
    usb_deregister(&zd1221_driver);
}

module_init(zfLnxIinit);
module_exit(zfLnxExit);
