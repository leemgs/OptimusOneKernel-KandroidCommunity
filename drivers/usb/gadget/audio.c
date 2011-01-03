



#include <linux/kernel.h>
#include <linux/utsname.h>

#include "u_audio.h"

#define DRIVER_DESC		"Linux USB Audio Gadget"
#define DRIVER_VERSION		"Dec 18, 2008"




#include "composite.c"
#include "usbstring.c"
#include "config.c"
#include "epautoconf.c"

#include "u_audio.c"
#include "f_audio.c"






#define AUDIO_VENDOR_NUM		0x1d6b	
#define AUDIO_PRODUCT_NUM		0x0101	



static struct usb_device_descriptor device_desc = {
	.bLength =		sizeof device_desc,
	.bDescriptorType =	USB_DT_DEVICE,

	.bcdUSB =		__constant_cpu_to_le16(0x200),

	.bDeviceClass =		USB_CLASS_PER_INTERFACE,
	.bDeviceSubClass =	0,
	.bDeviceProtocol =	0,
	

	
	.idVendor =		__constant_cpu_to_le16(AUDIO_VENDOR_NUM),
	.idProduct =		__constant_cpu_to_le16(AUDIO_PRODUCT_NUM),
	
	
	
	
	.bNumConfigurations =	1,
};

static struct usb_otg_descriptor otg_descriptor = {
	.bLength =		sizeof otg_descriptor,
	.bDescriptorType =	USB_DT_OTG,

	
	.bmAttributes =		USB_OTG_SRP | USB_OTG_HNP,
};

static const struct usb_descriptor_header *otg_desc[] = {
	(struct usb_descriptor_header *) &otg_descriptor,
	NULL,
};





static int audio_set_endpoint_req(struct usb_configuration *c,
		const struct usb_ctrlrequest *ctrl)
{
	struct usb_composite_dev *cdev = c->cdev;
	int			value = -EOPNOTSUPP;
	u16			ep = le16_to_cpu(ctrl->wIndex);
	u16			len = le16_to_cpu(ctrl->wLength);
	u16			w_value = le16_to_cpu(ctrl->wValue);

	DBG(cdev, "bRequest 0x%x, w_value 0x%04x, len %d, endpoint %d\n",
			ctrl->bRequest, w_value, len, ep);

	switch (ctrl->bRequest) {
	case UAC_SET_CUR:
		value = 0;
		break;

	case UAC_SET_MIN:
		break;

	case UAC_SET_MAX:
		break;

	case UAC_SET_RES:
		break;

	case UAC_SET_MEM:
		break;

	default:
		break;
	}

	return value;
}

static int audio_get_endpoint_req(struct usb_configuration *c,
		const struct usb_ctrlrequest *ctrl)
{
	struct usb_composite_dev *cdev = c->cdev;
	int value = -EOPNOTSUPP;
	u8 ep = ((le16_to_cpu(ctrl->wIndex) >> 8) & 0xFF);
	u16 len = le16_to_cpu(ctrl->wLength);
	u16 w_value = le16_to_cpu(ctrl->wValue);

	DBG(cdev, "bRequest 0x%x, w_value 0x%04x, len %d, endpoint %d\n",
			ctrl->bRequest, w_value, len, ep);

	switch (ctrl->bRequest) {
	case UAC_GET_CUR:
	case UAC_GET_MIN:
	case UAC_GET_MAX:
	case UAC_GET_RES:
		value = 3;
		break;
	case UAC_GET_MEM:
		break;
	default:
		break;
	}

	return value;
}

static int
audio_setup(struct usb_configuration *c, const struct usb_ctrlrequest *ctrl)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct usb_request *req = cdev->req;
	int value = -EOPNOTSUPP;
	u16 w_index = le16_to_cpu(ctrl->wIndex);
	u16 w_value = le16_to_cpu(ctrl->wValue);
	u16 w_length = le16_to_cpu(ctrl->wLength);

	
	switch (ctrl->bRequestType) {
	case USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_ENDPOINT:
		value = audio_set_endpoint_req(c, ctrl);
		break;

	case USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_ENDPOINT:
		value = audio_get_endpoint_req(c, ctrl);
		break;

	default:
		ERROR(cdev, "Invalid control req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
	}

	
	if (value >= 0) {
		DBG(cdev, "Audio req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
		req->zero = 0;
		req->length = value;
		value = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
		if (value < 0)
			ERROR(cdev, "Audio response on err %d\n", value);
	}

	
	return value;
}



static int __init audio_do_config(struct usb_configuration *c)
{
	

	if (gadget_is_otg(c->cdev->gadget)) {
		c->descriptors = otg_desc;
		c->bmAttributes |= USB_CONFIG_ATT_WAKEUP;
	}

	audio_bind_config(c);

	return 0;
}

static struct usb_configuration audio_config_driver = {
	.label			= DRIVER_DESC,
	.bind			= audio_do_config,
	.setup			= audio_setup,
	.bConfigurationValue	= 1,
	
	.bmAttributes		= USB_CONFIG_ATT_SELFPOWER,
};



static int __init audio_bind(struct usb_composite_dev *cdev)
{
	int			gcnum;
	int			status;

	gcnum = usb_gadget_controller_number(cdev->gadget);
	if (gcnum >= 0)
		device_desc.bcdDevice = cpu_to_le16(0x0300 | gcnum);
	else {
		ERROR(cdev, "controller '%s' not recognized; trying %s\n",
			cdev->gadget->name,
			audio_config_driver.label);
		device_desc.bcdDevice =
			__constant_cpu_to_le16(0x0300 | 0x0099);
	}

	
	snprintf(manufacturer, sizeof manufacturer, "%s %s with %s",
		init_utsname()->sysname, init_utsname()->release,
		cdev->gadget->name);
	status = usb_string_id(cdev);
	if (status < 0)
		goto fail;
	strings_dev[STRING_MANUFACTURER_IDX].id = status;
	device_desc.iManufacturer = status;

	status = usb_string_id(cdev);
	if (status < 0)
		goto fail;
	strings_dev[STRING_PRODUCT_IDX].id = status;
	device_desc.iProduct = status;

	status = usb_add_config(cdev, &audio_config_driver);
	if (status < 0)
		goto fail;

	INFO(cdev, "%s, version: %s\n", DRIVER_DESC, DRIVER_VERSION);
	return 0;

fail:
	return status;
}

static int __exit audio_unbind(struct usb_composite_dev *cdev)
{
	return 0;
}

static struct usb_composite_driver audio_driver = {
	.name		= "g_audio",
	.dev		= &device_desc,
	.strings	= audio_strings,
	.bind		= audio_bind,
	.unbind		= __exit_p(audio_unbind),
};

static int __init init(void)
{
	return usb_composite_register(&audio_driver);
}
module_init(init);

static void __exit cleanup(void)
{
	usb_composite_unregister(&audio_driver);
}
module_exit(cleanup);

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR("Bryan Wu <cooloney@kernel.org>");
MODULE_LICENSE("GPL");

