




#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/workqueue.h>
#include <linux/mii.h>
#include <linux/usb.h>
#include <linux/usb/cdc.h>
#include <linux/usb/usbnet.h>


#if defined(CONFIG_USB_NET_RNDIS_HOST) || defined(CONFIG_USB_NET_RNDIS_HOST_MODULE)

static int is_rndis(struct usb_interface_descriptor *desc)
{
	return desc->bInterfaceClass == USB_CLASS_COMM
		&& desc->bInterfaceSubClass == 2
		&& desc->bInterfaceProtocol == 0xff;
}

static int is_activesync(struct usb_interface_descriptor *desc)
{
	return desc->bInterfaceClass == USB_CLASS_MISC
		&& desc->bInterfaceSubClass == 1
		&& desc->bInterfaceProtocol == 1;
}

static int is_wireless_rndis(struct usb_interface_descriptor *desc)
{
	return desc->bInterfaceClass == USB_CLASS_WIRELESS_CONTROLLER
		&& desc->bInterfaceSubClass == 1
		&& desc->bInterfaceProtocol == 3;
}

#else

#define is_rndis(desc)		0
#define is_activesync(desc)	0
#define is_wireless_rndis(desc)	0

#endif


int usbnet_generic_cdc_bind(struct usbnet *dev, struct usb_interface *intf)
{
	u8				*buf = intf->cur_altsetting->extra;
	int				len = intf->cur_altsetting->extralen;
	struct usb_interface_descriptor	*d;
	struct cdc_state		*info = (void *) &dev->data;
	int				status;
	int				rndis;
	struct usb_driver		*driver = driver_of(intf);

	if (sizeof dev->data < sizeof *info)
		return -EDOM;

	
	if (len == 0 && dev->udev->actconfig->extralen) {
		
		buf = dev->udev->actconfig->extra;
		len = dev->udev->actconfig->extralen;
		if (len)
			dev_dbg(&intf->dev,
				"CDC descriptors on config\n");
	}

	
	if (len == 0) {
		struct usb_host_endpoint	*hep;

		hep = intf->cur_altsetting->endpoint;
		if (hep) {
			buf = hep->extra;
			len = hep->extralen;
		}
		if (len)
			dev_dbg(&intf->dev,
				"CDC descriptors on endpoint\n");
	}

	
	rndis = is_rndis(&intf->cur_altsetting->desc)
		|| is_activesync(&intf->cur_altsetting->desc)
		|| is_wireless_rndis(&intf->cur_altsetting->desc);

	memset(info, 0, sizeof *info);
	info->control = intf;
	while (len > 3) {
		if (buf [1] != USB_DT_CS_INTERFACE)
			goto next_desc;

		
		switch (buf [2]) {
		case USB_CDC_HEADER_TYPE:
			if (info->header) {
				dev_dbg(&intf->dev, "extra CDC header\n");
				goto bad_desc;
			}
			info->header = (void *) buf;
			if (info->header->bLength != sizeof *info->header) {
				dev_dbg(&intf->dev, "CDC header len %u\n",
					info->header->bLength);
				goto bad_desc;
			}
			break;
		case USB_CDC_ACM_TYPE:
			
			if (rndis) {
				struct usb_cdc_acm_descriptor *acm;

				acm = (void *) buf;
				if (acm->bmCapabilities) {
					dev_dbg(&intf->dev,
						"ACM capabilities %02x, "
						"not really RNDIS?\n",
						acm->bmCapabilities);
					goto bad_desc;
				}
			}
			break;
		case USB_CDC_UNION_TYPE:
			if (info->u) {
				dev_dbg(&intf->dev, "extra CDC union\n");
				goto bad_desc;
			}
			info->u = (void *) buf;
			if (info->u->bLength != sizeof *info->u) {
				dev_dbg(&intf->dev, "CDC union len %u\n",
					info->u->bLength);
				goto bad_desc;
			}

			
			info->control = usb_ifnum_to_if(dev->udev,
						info->u->bMasterInterface0);
			info->data = usb_ifnum_to_if(dev->udev,
						info->u->bSlaveInterface0);
			if (!info->control || !info->data) {
				dev_dbg(&intf->dev,
					"master #%u/%p slave #%u/%p\n",
					info->u->bMasterInterface0,
					info->control,
					info->u->bSlaveInterface0,
					info->data);
				goto bad_desc;
			}
			if (info->control != intf) {
				dev_dbg(&intf->dev, "bogus CDC Union\n");
				
				if (info->data == intf) {
					info->data = info->control;
					info->control = intf;
				} else
					goto bad_desc;
			}

			
			d = &info->data->cur_altsetting->desc;
			if (d->bInterfaceClass != USB_CLASS_CDC_DATA) {
				dev_dbg(&intf->dev, "slave class %u\n",
					d->bInterfaceClass);
				goto bad_desc;
			}
			break;
		case USB_CDC_ETHERNET_TYPE:
			if (info->ether) {
				dev_dbg(&intf->dev, "extra CDC ether\n");
				goto bad_desc;
			}
			info->ether = (void *) buf;
			if (info->ether->bLength != sizeof *info->ether) {
				dev_dbg(&intf->dev, "CDC ether len %u\n",
					info->ether->bLength);
				goto bad_desc;
			}
			dev->hard_mtu = le16_to_cpu(
						info->ether->wMaxSegmentSize);
			
			break;
		}
next_desc:
		len -= buf [0];	
		buf += buf [0];
	}

	
	if (rndis && !info->u) {
		info->control = usb_ifnum_to_if(dev->udev, 0);
		info->data = usb_ifnum_to_if(dev->udev, 1);
		if (!info->control || !info->data) {
			dev_dbg(&intf->dev,
				"rndis: master #0/%p slave #1/%p\n",
				info->control,
				info->data);
			goto bad_desc;
		}

	} else if (!info->header || !info->u || (!rndis && !info->ether)) {
		dev_dbg(&intf->dev, "missing cdc %s%s%sdescriptor\n",
			info->header ? "" : "header ",
			info->u ? "" : "union ",
			info->ether ? "" : "ether ");
		goto bad_desc;
	}

	
	status = usb_driver_claim_interface(driver, info->data, dev);
	if (status < 0)
		return status;
	status = usbnet_get_endpoints(dev, info->data);
	if (status < 0) {
		
		usb_set_intfdata(info->data, NULL);
		usb_driver_release_interface(driver, info->data);
		return status;
	}

	
	dev->status = NULL;
	if (info->control->cur_altsetting->desc.bNumEndpoints == 1) {
		struct usb_endpoint_descriptor	*desc;

		dev->status = &info->control->cur_altsetting->endpoint [0];
		desc = &dev->status->desc;
		if (!usb_endpoint_is_int_in(desc)
				|| (le16_to_cpu(desc->wMaxPacketSize)
					< sizeof(struct usb_cdc_notification))
				|| !desc->bInterval) {
			dev_dbg(&intf->dev, "bad notification endpoint\n");
			dev->status = NULL;
		}
	}
	if (rndis && !dev->status) {
		dev_dbg(&intf->dev, "missing RNDIS status endpoint\n");
		usb_set_intfdata(info->data, NULL);
		usb_driver_release_interface(driver, info->data);
		return -ENODEV;
	}
	return 0;

bad_desc:
	dev_info(&dev->udev->dev, "bad CDC descriptors\n");
	return -ENODEV;
}
EXPORT_SYMBOL_GPL(usbnet_generic_cdc_bind);

void usbnet_cdc_unbind(struct usbnet *dev, struct usb_interface *intf)
{
	struct cdc_state		*info = (void *) &dev->data;
	struct usb_driver		*driver = driver_of(intf);

	
	if (intf == info->control && info->data) {
		
		usb_set_intfdata(info->data, NULL);
		usb_driver_release_interface(driver, info->data);
		info->data = NULL;
	}

	
	else if (intf == info->data && info->control) {
		
		usb_set_intfdata(info->control, NULL);
		usb_driver_release_interface(driver, info->control);
		info->control = NULL;
	}
}
EXPORT_SYMBOL_GPL(usbnet_cdc_unbind);



static void dumpspeed(struct usbnet *dev, __le32 *speeds)
{
	if (netif_msg_timer(dev))
		devinfo(dev, "link speeds: %u kbps up, %u kbps down",
			__le32_to_cpu(speeds[0]) / 1000,
		__le32_to_cpu(speeds[1]) / 1000);
}

static void cdc_status(struct usbnet *dev, struct urb *urb)
{
	struct usb_cdc_notification	*event;

	if (urb->actual_length < sizeof *event)
		return;

	
	if (test_and_clear_bit(EVENT_STS_SPLIT, &dev->flags)) {
		dumpspeed(dev, (__le32 *) urb->transfer_buffer);
		return;
	}

	event = urb->transfer_buffer;
	switch (event->bNotificationType) {
	case USB_CDC_NOTIFY_NETWORK_CONNECTION:
		if (netif_msg_timer(dev))
			devdbg(dev, "CDC: carrier %s",
					event->wValue ? "on" : "off");
		if (event->wValue)
			netif_carrier_on(dev->net);
		else
			netif_carrier_off(dev->net);
		break;
	case USB_CDC_NOTIFY_SPEED_CHANGE:	
		if (netif_msg_timer(dev))
			devdbg(dev, "CDC: speed change (len %d)",
					urb->actual_length);
		if (urb->actual_length != (sizeof *event + 8))
			set_bit(EVENT_STS_SPLIT, &dev->flags);
		else
			dumpspeed(dev, (__le32 *) &event[1]);
		break;
	
	default:
		deverr(dev, "CDC: unexpected notification %02x!",
				 event->bNotificationType);
		break;
	}
}

static int cdc_bind(struct usbnet *dev, struct usb_interface *intf)
{
	int				status;
	struct cdc_state		*info = (void *) &dev->data;

	status = usbnet_generic_cdc_bind(dev, intf);
	if (status < 0)
		return status;

	status = usbnet_get_ethernet_addr(dev, info->ether->iMACAddress);
	if (status < 0) {
		usb_set_intfdata(info->data, NULL);
		usb_driver_release_interface(driver_of(intf), info->data);
		return status;
	}

	
	return 0;
}

static const struct driver_info	cdc_info = {
	.description =	"CDC Ethernet Device",
	.flags =	FLAG_ETHER,
	
	.bind =		cdc_bind,
	.unbind =	usbnet_cdc_unbind,
	.status =	cdc_status,
};




static const struct usb_device_id	products [] = {


#define	ZAURUS_MASTER_INTERFACE \
	.bInterfaceClass	= USB_CLASS_COMM, \
	.bInterfaceSubClass	= USB_CDC_SUBCLASS_ETHERNET, \
	.bInterfaceProtocol	= USB_CDC_PROTO_NONE


{
	.match_flags	=   USB_DEVICE_ID_MATCH_INT_INFO
			  | USB_DEVICE_ID_MATCH_DEVICE,
	.idVendor		= 0x04DD,
	.idProduct		= 0x8004,
	ZAURUS_MASTER_INTERFACE,
	.driver_info		= 0,
},


{
	.match_flags	=   USB_DEVICE_ID_MATCH_INT_INFO
			  | USB_DEVICE_ID_MATCH_DEVICE,
	.idVendor		= 0x04DD,
	.idProduct		= 0x8005,	
	ZAURUS_MASTER_INTERFACE,
	.driver_info		= 0,
}, {
	.match_flags	=   USB_DEVICE_ID_MATCH_INT_INFO
			  | USB_DEVICE_ID_MATCH_DEVICE,
	.idVendor		= 0x04DD,
	.idProduct		= 0x8006,	
	ZAURUS_MASTER_INTERFACE,
	.driver_info		= 0,
}, {
	.match_flags    =   USB_DEVICE_ID_MATCH_INT_INFO
	          | USB_DEVICE_ID_MATCH_DEVICE,
	.idVendor		= 0x04DD,
	.idProduct		= 0x8007,	
	ZAURUS_MASTER_INTERFACE,
	.driver_info		= 0,
}, {
	.match_flags    =   USB_DEVICE_ID_MATCH_INT_INFO
		 | USB_DEVICE_ID_MATCH_DEVICE,
	.idVendor               = 0x04DD,
	.idProduct              = 0x9031,	
	ZAURUS_MASTER_INTERFACE,
	.driver_info		= 0,
}, {
	.match_flags    =   USB_DEVICE_ID_MATCH_INT_INFO
		 | USB_DEVICE_ID_MATCH_DEVICE,
	.idVendor               = 0x04DD,
	.idProduct              = 0x9032,	
	ZAURUS_MASTER_INTERFACE,
	.driver_info		= 0,
}, {
	.match_flags    =   USB_DEVICE_ID_MATCH_INT_INFO
		 | USB_DEVICE_ID_MATCH_DEVICE,
	.idVendor               = 0x04DD,
	
	.idProduct              = 0x9050,	
	ZAURUS_MASTER_INTERFACE,
	.driver_info		= 0,
},


{
	.match_flags    =   USB_DEVICE_ID_MATCH_INT_INFO
		 | USB_DEVICE_ID_MATCH_DEVICE,
	.idVendor               = 0x07B4,
	.idProduct              = 0x0F02,	
	ZAURUS_MASTER_INTERFACE,
	.driver_info		= 0,
},


{
	USB_INTERFACE_INFO(USB_CLASS_COMM, USB_CDC_SUBCLASS_ETHERNET,
			USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long) &cdc_info,
}, {
	
	USB_DEVICE_AND_INTERFACE_INFO(0x0bdb, 0x1900, USB_CLASS_COMM,
			USB_CDC_SUBCLASS_MDLM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long) &cdc_info,
}, {
	
	USB_DEVICE_AND_INTERFACE_INFO(0x0bdb, 0x1902, USB_CLASS_COMM,
			USB_CDC_SUBCLASS_MDLM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long) &cdc_info,
}, {
	
	USB_DEVICE_AND_INTERFACE_INFO(0x0bdb, 0x1904, USB_CLASS_COMM,
			USB_CDC_SUBCLASS_MDLM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long) &cdc_info,
}, {
	
	USB_DEVICE_AND_INTERFACE_INFO(0x0bdb, 0x1905, USB_CLASS_COMM,
			USB_CDC_SUBCLASS_MDLM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long) &cdc_info,
}, {
	
	USB_DEVICE_AND_INTERFACE_INFO(0x0bdb, 0x1906, USB_CLASS_COMM,
			USB_CDC_SUBCLASS_MDLM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long) &cdc_info,
}, {
	
	USB_DEVICE_AND_INTERFACE_INFO(0x0bdb, 0x190a, USB_CLASS_COMM,
			USB_CDC_SUBCLASS_MDLM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long) &cdc_info,
}, {
	
	USB_DEVICE_AND_INTERFACE_INFO(0x0bdb, 0x1909, USB_CLASS_COMM,
			USB_CDC_SUBCLASS_MDLM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long) &cdc_info,
}, {
	
	USB_DEVICE_AND_INTERFACE_INFO(0x0bdb, 0x1049, USB_CLASS_COMM,
			USB_CDC_SUBCLASS_MDLM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long) &cdc_info,
}, {
	
	USB_DEVICE_AND_INTERFACE_INFO(0x0930, 0x130b, USB_CLASS_COMM,
			USB_CDC_SUBCLASS_MDLM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long) &cdc_info,
}, {
	
	USB_DEVICE_AND_INTERFACE_INFO(0x0930, 0x130c, USB_CLASS_COMM,
			USB_CDC_SUBCLASS_MDLM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long) &cdc_info,
}, {
	
	USB_DEVICE_AND_INTERFACE_INFO(0x0930, 0x1311, USB_CLASS_COMM,
			USB_CDC_SUBCLASS_MDLM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long) &cdc_info,
}, {
	
	USB_DEVICE_AND_INTERFACE_INFO(0x413c, 0x8147, USB_CLASS_COMM,
			USB_CDC_SUBCLASS_MDLM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long) &cdc_info,
}, {
	
	USB_DEVICE_AND_INTERFACE_INFO(0x413c, 0x8183, USB_CLASS_COMM,
			USB_CDC_SUBCLASS_MDLM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long) &cdc_info,
}, {
	
	USB_DEVICE_AND_INTERFACE_INFO(0x413c, 0x8184, USB_CLASS_COMM,
			USB_CDC_SUBCLASS_MDLM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long) &cdc_info,
},
	{ },		
};
MODULE_DEVICE_TABLE(usb, products);

static struct usb_driver cdc_driver = {
	.name =		"cdc_ether",
	.id_table =	products,
	.probe =	usbnet_probe,
	.disconnect =	usbnet_disconnect,
	.suspend =	usbnet_suspend,
	.resume =	usbnet_resume,
};


static int __init cdc_init(void)
{
	BUILD_BUG_ON((sizeof(((struct usbnet *)0)->data)
			< sizeof(struct cdc_state)));

 	return usb_register(&cdc_driver);
}
module_init(cdc_init);

static void __exit cdc_exit(void)
{
 	usb_deregister(&cdc_driver);
}
module_exit(cdc_exit);

MODULE_AUTHOR("David Brownell");
MODULE_DESCRIPTION("USB CDC Ethernet devices");
MODULE_LICENSE("GPL");
