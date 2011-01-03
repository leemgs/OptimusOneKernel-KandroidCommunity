

#include <linux/usb.h>
#include <linux/usb/quirks.h>
#include "usb.h"


static const struct usb_device_id usb_quirk_list[] = {
	
	{ USB_DEVICE(0x0204, 0x6025), .driver_info = USB_QUIRK_RESET_RESUME },

	
	{ USB_DEVICE(0x03f0, 0x0701), .driver_info =
			USB_QUIRK_STRING_FETCH_255 },

	
	{ USB_DEVICE(0x041e, 0x3020), .driver_info = USB_QUIRK_RESET_RESUME },

	
	{ USB_DEVICE(0x0471, 0x0155), .driver_info = USB_QUIRK_RESET_RESUME },

	
	{ USB_DEVICE(0x0582, 0x0007), .driver_info = USB_QUIRK_RESET_RESUME },

	
	{ USB_DEVICE(0x0582, 0x0027), .driver_info = USB_QUIRK_RESET_RESUME },

	
	{ USB_DEVICE(0x05ac, 0x021a), .driver_info = USB_QUIRK_RESET_RESUME },

	
	{ USB_DEVICE(0x0638, 0x0a13), .driver_info =
	  USB_QUIRK_STRING_FETCH_255 },

	
	{ USB_DEVICE(0x06a3, 0x0006), .driver_info =
			USB_QUIRK_CONFIG_INTF_STRINGS },

	
	{ USB_DEVICE(0x08ec, 0x1000), .driver_info = USB_QUIRK_RESET_RESUME },

	
	{ USB_DEVICE(0x0971, 0x2000), .driver_info = USB_QUIRK_NO_SET_INTF },

	
	{ USB_DEVICE(0x10d6, 0x2200), .driver_info =
			USB_QUIRK_STRING_FETCH_255 },

	
	{ USB_DEVICE(0x1516, 0x8628), .driver_info = USB_QUIRK_RESET_RESUME },

	
	{ USB_DEVICE(0x8086, 0xf1a5), .driver_info = USB_QUIRK_RESET_RESUME },

	{ }  
};

static const struct usb_device_id *find_id(struct usb_device *udev)
{
	const struct usb_device_id *id = usb_quirk_list;

	for (; id->idVendor || id->bDeviceClass || id->bInterfaceClass ||
			id->driver_info; id++) {
		if (usb_match_device(udev, id))
			return id;
	}
	return NULL;
}


void usb_detect_quirks(struct usb_device *udev)
{
	const struct usb_device_id *id = usb_quirk_list;

	id = find_id(udev);
	if (id)
		udev->quirks = (u32)(id->driver_info);
	if (udev->quirks)
		dev_dbg(&udev->dev, "USB quirks for this device: %x\n",
				udev->quirks);

	
#ifdef	CONFIG_USB_SUSPEND
	if (udev->descriptor.bDeviceClass != USB_CLASS_HUB)
		udev->autosuspend_disabled = 1;
#endif

	
#if 0		
	
	if (udev->descriptor.bDeviceClass == USB_CLASS_HUB)
		udev->persist_enabled = 1;

#else
	
	udev->persist_enabled = 1;
#endif	
}
