



#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/etherdevice.h>

#include "u_ether.h"



struct ecm_ep_descs {
	struct usb_endpoint_descriptor	*in;
	struct usb_endpoint_descriptor	*out;
	struct usb_endpoint_descriptor	*notify;
};

enum ecm_notify_state {
	ECM_NOTIFY_NONE,		
	ECM_NOTIFY_CONNECT,		
	ECM_NOTIFY_SPEED,		
};

struct f_ecm {
	struct gether			port;
	u8				ctrl_id, data_id;

	char				ethaddr[14];

	struct ecm_ep_descs		fs;
	struct ecm_ep_descs		hs;

	struct usb_ep			*notify;
	struct usb_endpoint_descriptor	*notify_desc;
	struct usb_request		*notify_req;
	u8				notify_state;
	bool				is_open;

	
};

static inline struct f_ecm *func_to_ecm(struct usb_function *f)
{
	return container_of(f, struct f_ecm, port.func);
}


static inline unsigned ecm_bitrate(struct usb_gadget *g)
{
	if (gadget_is_dualspeed(g) && g->speed == USB_SPEED_HIGH)
		return 13 * 512 * 8 * 1000 * 8;
	else
		return 19 *  64 * 1 * 1000 * 8;
}





#define LOG2_STATUS_INTERVAL_MSEC	5	


#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_ECM_FIX
#define ECM_STATUS_BYTECOUNT		64	
#else
#define ECM_STATUS_BYTECOUNT		16	
#endif






#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_ECM_FIX
#define USB_DT_IAD_SIZE     8
struct usb_interface_assoc_descriptor ecm_IAD = {
	.bLength           = USB_DT_IAD_SIZE,
	.bDescriptorType   = USB_DT_INTERFACE_ASSOCIATION,
	.bInterfaceCount   = 2,
	.bFunctionClass    = USB_CLASS_COMM,
	.bFunctionSubClass = USB_CDC_SUBCLASS_ETHERNET,
	.bFunctionProtocol = USB_CDC_PROTO_NONE,
	.iFunction         = 0,
};
#endif


static struct usb_interface_descriptor ecm_control_intf = {
	.bLength =		sizeof ecm_control_intf,
	.bDescriptorType =	USB_DT_INTERFACE,

	
	
	.bNumEndpoints =	1,
	.bInterfaceClass =	USB_CLASS_COMM,
	.bInterfaceSubClass =	USB_CDC_SUBCLASS_ETHERNET,
	.bInterfaceProtocol =	USB_CDC_PROTO_NONE,
	
};

static struct usb_cdc_header_desc ecm_header_desc = {
	.bLength =		sizeof ecm_header_desc,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_HEADER_TYPE,

	.bcdCDC =		cpu_to_le16(0x0110),
};

static struct usb_cdc_union_desc ecm_union_desc = {
	.bLength =		sizeof(ecm_union_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_UNION_TYPE,
	
	
};

static struct usb_cdc_ether_desc ecm_desc = {
	.bLength =		sizeof ecm_desc,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_ETHERNET_TYPE,

	
	
	.bmEthernetStatistics =	cpu_to_le32(0), 
	.wMaxSegmentSize =	cpu_to_le16(ETH_FRAME_LEN),
	.wNumberMCFilters =	cpu_to_le16(0),
	.bNumberPowerFilters =	0,
};



static struct usb_interface_descriptor ecm_data_nop_intf = {
	.bLength =		sizeof ecm_data_nop_intf,
	.bDescriptorType =	USB_DT_INTERFACE,

	.bInterfaceNumber =	1,
	.bAlternateSetting =	0,
	.bNumEndpoints =	0,
	.bInterfaceClass =	USB_CLASS_CDC_DATA,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	
};



static struct usb_interface_descriptor ecm_data_intf = {
	.bLength =		sizeof ecm_data_intf,
	.bDescriptorType =	USB_DT_INTERFACE,

	.bInterfaceNumber =	1,
	.bAlternateSetting =	1,
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_CDC_DATA,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	
};



static struct usb_endpoint_descriptor fs_ecm_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(ECM_STATUS_BYTECOUNT),

#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_ECM_FIX
	.bInterval =		4,
#else
	.bInterval =		1 << LOG2_STATUS_INTERVAL_MSEC,
#endif


};

static struct usb_endpoint_descriptor fs_ecm_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,

#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_ECM_FIX
	.wMaxPacketSize =	cpu_to_le16(64),
#endif

};

static struct usb_endpoint_descriptor fs_ecm_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,

#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_ECM_FIX
	.wMaxPacketSize =	cpu_to_le16(64),
#endif

};



#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_ECM_FIX
static struct usb_descriptor_header *ecm_fs_function[] = {
	(struct usb_descriptor_header *) &ecm_IAD,
	
	(struct usb_descriptor_header *) &ecm_control_intf,
	(struct usb_descriptor_header *) &ecm_header_desc,
	(struct usb_descriptor_header *) &ecm_union_desc,
	(struct usb_descriptor_header *) &ecm_desc,
	
	(struct usb_descriptor_header *) &fs_ecm_notify_desc,
	
	(struct usb_descriptor_header *) &ecm_data_nop_intf,
	(struct usb_descriptor_header *) &ecm_data_intf,
	(struct usb_descriptor_header *) &fs_ecm_out_desc,
	(struct usb_descriptor_header *) &fs_ecm_in_desc,
	NULL,
};
#else
static struct usb_descriptor_header *ecm_fs_function[] = {
	
	(struct usb_descriptor_header *) &ecm_control_intf,
	(struct usb_descriptor_header *) &ecm_header_desc,
	(struct usb_descriptor_header *) &ecm_union_desc,
	(struct usb_descriptor_header *) &ecm_desc,
	
	(struct usb_descriptor_header *) &fs_ecm_notify_desc,
	
	(struct usb_descriptor_header *) &ecm_data_nop_intf,
	(struct usb_descriptor_header *) &ecm_data_intf,
	(struct usb_descriptor_header *) &fs_ecm_in_desc,
	(struct usb_descriptor_header *) &fs_ecm_out_desc,
	NULL,
};
#endif




static struct usb_endpoint_descriptor hs_ecm_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(ECM_STATUS_BYTECOUNT),

#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_ECM_FIX
	.bInterval =		4,
#else
	.bInterval =		LOG2_STATUS_INTERVAL_MSEC + 4,
#endif

};
static struct usb_endpoint_descriptor hs_ecm_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_endpoint_descriptor hs_ecm_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};


#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_ECM_FIX
static struct usb_descriptor_header *ecm_hs_function[] = {
	(struct usb_descriptor_header *) &ecm_IAD,
	
	(struct usb_descriptor_header *) &ecm_control_intf,
	(struct usb_descriptor_header *) &ecm_header_desc,
	(struct usb_descriptor_header *) &ecm_union_desc,
	(struct usb_descriptor_header *) &ecm_desc,
	
	(struct usb_descriptor_header *) &hs_ecm_notify_desc,
	
	(struct usb_descriptor_header *) &ecm_data_nop_intf,
	(struct usb_descriptor_header *) &ecm_data_intf,
	(struct usb_descriptor_header *) &hs_ecm_out_desc,
	(struct usb_descriptor_header *) &hs_ecm_in_desc,
	NULL,
};
#else
static struct usb_descriptor_header *ecm_hs_function[] = {
	
	(struct usb_descriptor_header *) &ecm_control_intf,
	(struct usb_descriptor_header *) &ecm_header_desc,
	(struct usb_descriptor_header *) &ecm_union_desc,
	(struct usb_descriptor_header *) &ecm_desc,
	
	(struct usb_descriptor_header *) &hs_ecm_notify_desc,
	
	(struct usb_descriptor_header *) &ecm_data_nop_intf,
	(struct usb_descriptor_header *) &ecm_data_intf,
	(struct usb_descriptor_header *) &hs_ecm_in_desc,
	(struct usb_descriptor_header *) &hs_ecm_out_desc,
	NULL,
};
#endif




static struct usb_string ecm_string_defs[] = {
	[0].s = "CDC Ethernet Control Model (ECM)",
	[1].s = NULL ,
	[2].s = "CDC Ethernet Data",
	{  } 
};

static struct usb_gadget_strings ecm_string_table = {
	.language =		0x0409,	
	.strings =		ecm_string_defs,
};

static struct usb_gadget_strings *ecm_strings[] = {
	&ecm_string_table,
	NULL,
};



static void ecm_do_notify(struct f_ecm *ecm)
{
	struct usb_request		*req = ecm->notify_req;
	struct usb_cdc_notification	*event;
	struct usb_composite_dev	*cdev = ecm->port.func.config->cdev;
	__le32				*data;
	int				status;

	
	if (!req)
		return;

	event = req->buf;
	switch (ecm->notify_state) {
	case ECM_NOTIFY_NONE:
		return;

	case ECM_NOTIFY_CONNECT:
		event->bNotificationType = USB_CDC_NOTIFY_NETWORK_CONNECTION;
		if (ecm->is_open)
			event->wValue = cpu_to_le16(1);
		else
			event->wValue = cpu_to_le16(0);
		event->wLength = 0;
		req->length = sizeof *event;

		DBG(cdev, "notify connect %s\n",
				ecm->is_open ? "true" : "false");
		ecm->notify_state = ECM_NOTIFY_SPEED;
		break;

	case ECM_NOTIFY_SPEED:
		event->bNotificationType = USB_CDC_NOTIFY_SPEED_CHANGE;
		event->wValue = cpu_to_le16(0);
		event->wLength = cpu_to_le16(8);
		req->length = ECM_STATUS_BYTECOUNT;

		
		data = req->buf + sizeof *event;

#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_ECM_FIX
		data[0] = cpu_to_le32(ecm_bitrate(cdev->gadget));
#else
		data[0] = cpu_to_le32(ecm_bitrate(cdev->gadget));
#endif


		data[1] = data[0];

		DBG(cdev, "notify speed %d\n", ecm_bitrate(cdev->gadget));
		pr_info("%s: notify speed %d\n", __func__, ecm_bitrate(cdev->gadget));
		ecm->notify_state = ECM_NOTIFY_NONE;
		break;
	}
	event->bmRequestType = 0xA1;
	event->wIndex = cpu_to_le16(ecm->ctrl_id);

	ecm->notify_req = NULL;
	status = usb_ep_queue(ecm->notify, req, GFP_ATOMIC);
	if (status < 0) {
		ecm->notify_req = req;
		DBG(cdev, "notify --> %d\n", status);
	}
}

static void ecm_notify(struct f_ecm *ecm)
{
	
	ecm->notify_state = ECM_NOTIFY_CONNECT;
	ecm_do_notify(ecm);
}

static void ecm_notify_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_ecm			*ecm = req->context;
	struct usb_composite_dev	*cdev = ecm->port.func.config->cdev;
	struct usb_cdc_notification	*event = req->buf;

	switch (req->status) {
	case 0:
		
		break;
	case -ECONNRESET:
	case -ESHUTDOWN:
		ecm->notify_state = ECM_NOTIFY_NONE;
		break;
	default:
		DBG(cdev, "event %02x --> %d\n",
			event->bNotificationType, req->status);
		break;
	}
	ecm->notify_req = req;
	ecm_do_notify(ecm);
}

static int ecm_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
	struct f_ecm		*ecm = func_to_ecm(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_request	*req = cdev->req;
	int			value = -EOPNOTSUPP;
	u16			w_index = le16_to_cpu(ctrl->wIndex);
	u16			w_value = le16_to_cpu(ctrl->wValue);
	u16			w_length = le16_to_cpu(ctrl->wLength);

	
	switch ((ctrl->bRequestType << 8) | ctrl->bRequest) {
	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_CDC_SET_ETHERNET_PACKET_FILTER:
		
		if (w_length != 0 || w_index != ecm->ctrl_id)
			goto invalid;
		DBG(cdev, "packet filter %02x\n", w_value);
		
		ecm->port.cdc_filter = w_value;
		value = 0;
		break;

	

	default:
invalid:
		DBG(cdev, "invalid control req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
	}

	
	if (value >= 0) {
		DBG(cdev, "ecm req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
		req->zero = 0;
		req->length = value;
		value = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
		if (value < 0)
			ERROR(cdev, "ecm req %02x.%02x response err %d\n",
					ctrl->bRequestType, ctrl->bRequest,
					value);
	}

	
	return value;
}


static int ecm_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct f_ecm		*ecm = func_to_ecm(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	
	if (intf == ecm->ctrl_id) {
		if (alt != 0)
			goto fail;

		if (ecm->notify->driver_data) {
			VDBG(cdev, "reset ecm control %d\n", intf);
			usb_ep_disable(ecm->notify);
		} else {
			VDBG(cdev, "init ecm ctrl %d\n", intf);
			ecm->notify_desc = ep_choose(cdev->gadget,
					ecm->hs.notify,
					ecm->fs.notify);
		}
		usb_ep_enable(ecm->notify, ecm->notify_desc);
		ecm->notify->driver_data = ecm;

	
	} else if (intf == ecm->data_id) {
		if (alt > 1)
			goto fail;

		if (ecm->port.in_ep->driver_data) {
			DBG(cdev, "reset ecm\n");
			gether_disconnect(&ecm->port);
		}

		if (!ecm->port.in) {
			DBG(cdev, "init ecm\n");
			ecm->port.in = ep_choose(cdev->gadget,
					ecm->hs.in, ecm->fs.in);
			ecm->port.out = ep_choose(cdev->gadget,
					ecm->hs.out, ecm->fs.out);
		}

		
		if (alt == 1) {
			struct net_device	*net;

			
			ecm->port.is_zlp_ok = !(
				   gadget_is_sa1100(cdev->gadget)
				|| gadget_is_musbhdrc(cdev->gadget)
				);
			ecm->port.cdc_filter = DEFAULT_FILTER;
			DBG(cdev, "activate ecm\n");
			net = gether_connect(&ecm->port);
			if (IS_ERR(net))
				return PTR_ERR(net);
		}

		
		ecm_notify(ecm);
	} else
		goto fail;

	return 0;
fail:
	return -EINVAL;
}


static int ecm_get_alt(struct usb_function *f, unsigned intf)
{
	struct f_ecm		*ecm = func_to_ecm(f);

	if (intf == ecm->ctrl_id)
		return 0;
	return ecm->port.in_ep->driver_data ? 1 : 0;
}

static void ecm_disable(struct usb_function *f)
{
	struct f_ecm		*ecm = func_to_ecm(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	DBG(cdev, "ecm deactivated\n");

	if (ecm->port.in_ep->driver_data)
		gether_disconnect(&ecm->port);

	if (ecm->notify->driver_data) {
		usb_ep_disable(ecm->notify);
		usb_ep_fifo_flush(ecm->notify);
		ecm->notify->driver_data = NULL;
		ecm->notify_desc = NULL;
	}
}





static void ecm_open(struct gether *geth)
{
	struct f_ecm		*ecm = func_to_ecm(&geth->func);

	DBG(ecm->port.func.config->cdev, "%s\n", __func__);

	ecm->is_open = true;
	ecm_notify(ecm);
}

static void ecm_close(struct gether *geth)
{
	struct f_ecm		*ecm = func_to_ecm(&geth->func);

	DBG(ecm->port.func.config->cdev, "%s\n", __func__);

	ecm->is_open = false;
	ecm_notify(ecm);
}





static int ecm_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct f_ecm		*ecm = func_to_ecm(f);
	int			status;
	struct usb_ep		*ep;

	
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	ecm->ctrl_id = status;

	ecm_control_intf.bInterfaceNumber = status;

#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_ECM_FIX
	ecm_IAD.bFirstInterface = status;
#endif

	ecm_union_desc.bMasterInterface0 = status;

	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	ecm->data_id = status;

	ecm_data_nop_intf.bInterfaceNumber = status;
	ecm_data_intf.bInterfaceNumber = status;
	ecm_union_desc.bSlaveInterface0 = status;

	status = -ENODEV;

	
	ep = usb_ep_autoconfig(cdev->gadget, &fs_ecm_in_desc);
	if (!ep)
		goto fail;
	ecm->port.in_ep = ep;
	ep->driver_data = cdev;	

	ep = usb_ep_autoconfig(cdev->gadget, &fs_ecm_out_desc);
	if (!ep)
		goto fail;
	ecm->port.out_ep = ep;
	ep->driver_data = cdev;	

	
	ep = usb_ep_autoconfig(cdev->gadget, &fs_ecm_notify_desc);
	if (!ep)
		goto fail;
	ecm->notify = ep;
	ep->driver_data = cdev;	

	status = -ENOMEM;

	
	ecm->notify_req = usb_ep_alloc_request(ep, GFP_KERNEL);
	if (!ecm->notify_req)
		goto fail;
	ecm->notify_req->buf = kmalloc(ECM_STATUS_BYTECOUNT, GFP_KERNEL);
	if (!ecm->notify_req->buf)
		goto fail;
	ecm->notify_req->context = ecm;
	ecm->notify_req->complete = ecm_notify_complete;

	
	f->descriptors = usb_copy_descriptors(ecm_fs_function);
	if (!f->descriptors)
		goto fail;

	ecm->fs.in = usb_find_endpoint(ecm_fs_function,
			f->descriptors, &fs_ecm_in_desc);
	ecm->fs.out = usb_find_endpoint(ecm_fs_function,
			f->descriptors, &fs_ecm_out_desc);
	ecm->fs.notify = usb_find_endpoint(ecm_fs_function,
			f->descriptors, &fs_ecm_notify_desc);

	
	if (gadget_is_dualspeed(c->cdev->gadget)) {
		hs_ecm_in_desc.bEndpointAddress =
				fs_ecm_in_desc.bEndpointAddress;
		hs_ecm_out_desc.bEndpointAddress =
				fs_ecm_out_desc.bEndpointAddress;
		hs_ecm_notify_desc.bEndpointAddress =
				fs_ecm_notify_desc.bEndpointAddress;

		
		f->hs_descriptors = usb_copy_descriptors(ecm_hs_function);
		if (!f->hs_descriptors)
			goto fail;

		ecm->hs.in = usb_find_endpoint(ecm_hs_function,
				f->hs_descriptors, &hs_ecm_in_desc);
		ecm->hs.out = usb_find_endpoint(ecm_hs_function,
				f->hs_descriptors, &hs_ecm_out_desc);
		ecm->hs.notify = usb_find_endpoint(ecm_hs_function,
				f->hs_descriptors, &hs_ecm_notify_desc);
	}

	

	ecm->port.open = ecm_open;
	ecm->port.close = ecm_close;

	DBG(cdev, "CDC Ethernet: %s speed IN/%s OUT/%s NOTIFY/%s\n",
			gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full",
			ecm->port.in_ep->name, ecm->port.out_ep->name,
			ecm->notify->name);
	return 0;

fail:
	if (f->descriptors)
		usb_free_descriptors(f->descriptors);

	if (ecm->notify_req) {
		kfree(ecm->notify_req->buf);
		usb_ep_free_request(ecm->notify, ecm->notify_req);
	}

	
	if (ecm->notify)
		ecm->notify->driver_data = NULL;
	if (ecm->port.out)
		ecm->port.out_ep->driver_data = NULL;
	if (ecm->port.in)
		ecm->port.in_ep->driver_data = NULL;

	ERROR(cdev, "%s: can't bind, err %d\n", f->name, status);

	return status;
}

static void
ecm_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_ecm		*ecm = func_to_ecm(f);

	DBG(c->cdev, "ecm unbind\n");

	if (gadget_is_dualspeed(c->cdev->gadget))
		usb_free_descriptors(f->hs_descriptors);
	usb_free_descriptors(f->descriptors);

	kfree(ecm->notify_req->buf);
	usb_ep_free_request(ecm->notify, ecm->notify_req);

	ecm_string_defs[1].s = NULL;
	kfree(ecm);
}


int ecm_bind_config(struct usb_configuration *c, u8 ethaddr[ETH_ALEN])
{
	struct f_ecm	*ecm;
	int		status;

	if (!can_support_ecm(c->cdev->gadget) || !ethaddr)
		return -EINVAL;

	
	if (ecm_string_defs[0].id == 0) {

		
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		ecm_string_defs[0].id = status;
		ecm_control_intf.iInterface = status;


#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_ECM_FIX
		
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		ecm_string_defs[1].id = status;
		pr_info("%s: iMACAddress = %d\n", __func__, status);
		ecm_desc.iMACAddress = status;
#endif


		
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		ecm_string_defs[2].id = status;
		ecm_data_intf.iInterface = status;



#ifndef CONFIG_USB_SUPPORT_LGE_ANDROID_ECM_FIX
		
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		ecm_string_defs[1].id = status;
		pr_info("%s: iMACAddress = %d\n", __func__, status);
		ecm_desc.iMACAddress = status;
#endif

	}

	
	ecm = kzalloc(sizeof *ecm, GFP_KERNEL);
	if (!ecm)
		return -ENOMEM;

	
	snprintf(ecm->ethaddr, sizeof ecm->ethaddr,
		"%02X%02X%02X%02X%02X%02X",
		ethaddr[0], ethaddr[1], ethaddr[2],
		ethaddr[3], ethaddr[4], ethaddr[5]);
	ecm_string_defs[1].s = ecm->ethaddr;
	pr_info("%s: ecm_string_defs[1].s = %s\n", __func__, ecm_string_defs[1].s);

	ecm->port.cdc_filter = DEFAULT_FILTER;

	ecm->port.func.name = "cdc_ethernet";
	ecm->port.func.strings = ecm_strings;
	
	ecm->port.func.bind = ecm_bind;
	ecm->port.func.unbind = ecm_unbind;
	ecm->port.func.set_alt = ecm_set_alt;
	ecm->port.func.get_alt = ecm_get_alt;
	ecm->port.func.setup = ecm_setup;
	ecm->port.func.disable = ecm_disable;

	status = usb_add_function(c, &ecm->port.func);
	if (status) {
		ecm_string_defs[1].s = NULL;
		kfree(ecm);
	}
	return status;
}
