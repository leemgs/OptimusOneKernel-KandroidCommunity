



static struct usb_device_id whitelist_table [] = {


{ USB_DEVICE_INFO(USB_CLASS_HUB, 0, 0), },
{ USB_DEVICE_INFO(USB_CLASS_HUB, 0, 1), },

#ifdef	CONFIG_USB_PRINTER		

{ USB_DEVICE_INFO(7, 1, 1) },
{ USB_DEVICE_INFO(7, 1, 2) },
{ USB_DEVICE_INFO(7, 1, 3) },
#endif

#ifdef	CONFIG_USB_NET_CDCETHER

{ USB_DEVICE(0x0525, 0xa4a1), },

{ USB_DEVICE(0x0525, 0xa4a2), },
#endif

#if	defined(CONFIG_USB_TEST) || defined(CONFIG_USB_TEST_MODULE)

{ USB_DEVICE(0x0525, 0xa4a0), },
#endif

{ }	
};

static int is_targeted(struct usb_device *dev)
{
	struct usb_device_id	*id = whitelist_table;

	
	if (!dev->bus->otg_port)
		return 1;

	
	if ((le16_to_cpu(dev->descriptor.idVendor) == 0x1a0a &&
	     le16_to_cpu(dev->descriptor.idProduct) == 0xbadd))
		return 0;

	
	for (id = whitelist_table; id->match_flags; id++) {
		if ((id->match_flags & USB_DEVICE_ID_MATCH_VENDOR) &&
		    id->idVendor != le16_to_cpu(dev->descriptor.idVendor))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_PRODUCT) &&
		    id->idProduct != le16_to_cpu(dev->descriptor.idProduct))
			continue;

		
		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_LO) &&
		    (id->bcdDevice_lo > le16_to_cpu(dev->descriptor.bcdDevice)))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_HI) &&
		    (id->bcdDevice_hi < le16_to_cpu(dev->descriptor.bcdDevice)))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_CLASS) &&
		    (id->bDeviceClass != dev->descriptor.bDeviceClass))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_SUBCLASS) &&
		    (id->bDeviceSubClass != dev->descriptor.bDeviceSubClass))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_PROTOCOL) &&
		    (id->bDeviceProtocol != dev->descriptor.bDeviceProtocol))
			continue;

		return 1;
	}

	


	
	dev_err(&dev->dev, "device v%04x p%04x is not supported\n",
		le16_to_cpu(dev->descriptor.idVendor),
		le16_to_cpu(dev->descriptor.idProduct));
#ifdef	CONFIG_USB_OTG_WHITELIST
	return 0;
#else
	return 1;
#endif
}

