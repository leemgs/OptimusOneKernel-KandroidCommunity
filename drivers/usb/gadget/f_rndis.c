



#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/etherdevice.h>

#include <asm/atomic.h>

#include "u_ether.h"
#include "rndis.h"




struct rndis_ep_descs {
	struct usb_endpoint_descriptor	*in;
	struct usb_endpoint_descriptor	*out;
	struct usb_endpoint_descriptor	*notify;
};

struct f_rndis {
	struct gether			port;
	u8				ctrl_id, data_id;
	u8				ethaddr[ETH_ALEN];
	int				config;

	struct rndis_ep_descs		fs;
	struct rndis_ep_descs		hs;

	struct usb_ep			*notify;
	struct usb_endpoint_descriptor	*notify_desc;
	struct usb_request		*notify_req;
	atomic_t			notify_count;
};

static inline struct f_rndis *func_to_rndis(struct usb_function *f)
{
	return container_of(f, struct f_rndis, port.func);
}


static unsigned int bitrate(struct usb_gadget *g)
{
	if (gadget_is_dualspeed(g) && g->speed == USB_SPEED_HIGH)
		return 13 * 512 * 8 * 1000 * 8;
	else
		return 19 *  64 * 1 * 1000 * 8;
}





#define LOG2_STATUS_INTERVAL_MSEC	5	
#define STATUS_BYTECOUNT		8	




struct usb_interface_assoc_descriptor rndis_interface_assoc_desc = {
	.bLength           = USB_DT_INTERFACE_ASSOCIATION_SIZE,
	.bDescriptorType   = USB_DT_INTERFACE_ASSOCIATION,
	.bInterfaceCount   = 2,
#ifdef CONFIG_USB_ANDROID_RNDIS_WCEIS
	.bFunctionClass    = USB_CLASS_WIRELESS_CONTROLLER,
	.bFunctionSubClass = 1,
	.bFunctionProtocol = 3,
#else
	.bFunctionClass    = USB_CLASS_COMM,
	.bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
	.bFunctionProtocol = USB_CDC_ACM_PROTO_VENDOR,
#endif
};

static struct usb_interface_descriptor rndis_control_intf = {
	.bLength =		sizeof rndis_control_intf,
	.bDescriptorType =	USB_DT_INTERFACE,

	
	
	.bNumEndpoints =	1,
#ifdef CONFIG_USB_ANDROID_RNDIS_WCEIS
	.bInterfaceClass =	USB_CLASS_WIRELESS_CONTROLLER,
	.bInterfaceSubClass =   1,
	.bInterfaceProtocol =   3,
#else
	.bInterfaceClass =	USB_CLASS_COMM,
	.bInterfaceSubClass =   USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol =   USB_CDC_ACM_PROTO_VENDOR,
#endif
	
};

static struct usb_cdc_header_desc header_desc = {
	.bLength =		sizeof header_desc,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_HEADER_TYPE,

	.bcdCDC =		cpu_to_le16(0x0110),
};

static struct usb_cdc_call_mgmt_descriptor call_mgmt_descriptor = {
	.bLength =		sizeof call_mgmt_descriptor,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_CALL_MANAGEMENT_TYPE,

	.bmCapabilities =	0x00,
	.bDataInterface =	0x01,
};

static struct usb_cdc_acm_descriptor acm_descriptor = {
	.bLength =		sizeof acm_descriptor,
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_ACM_TYPE,

	.bmCapabilities =	0x00,
};

static struct usb_cdc_union_desc rndis_union_desc = {
	.bLength =		sizeof(rndis_union_desc),
	.bDescriptorType =	USB_DT_CS_INTERFACE,
	.bDescriptorSubType =	USB_CDC_UNION_TYPE,
	
	
};



static struct usb_interface_descriptor rndis_data_intf = {
	.bLength =		sizeof rndis_data_intf,
	.bDescriptorType =	USB_DT_INTERFACE,

	
	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_CDC_DATA,
	.bInterfaceSubClass =	0,
	.bInterfaceProtocol =	0,
	
};



static struct usb_endpoint_descriptor fs_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(STATUS_BYTECOUNT),
	.bInterval =		1 << LOG2_STATUS_INTERVAL_MSEC,
};

static struct usb_endpoint_descriptor fs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor fs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *eth_fs_function[] = {
	
	(struct usb_descriptor_header *) &rndis_interface_assoc_desc,
	(struct usb_descriptor_header *) &rndis_control_intf,
	(struct usb_descriptor_header *) &header_desc,
	(struct usb_descriptor_header *) &call_mgmt_descriptor,
	(struct usb_descriptor_header *) &acm_descriptor,
	(struct usb_descriptor_header *) &rndis_union_desc,
	(struct usb_descriptor_header *) &fs_notify_desc,
	
	(struct usb_descriptor_header *) &rndis_data_intf,
	(struct usb_descriptor_header *) &fs_in_desc,
	(struct usb_descriptor_header *) &fs_out_desc,
	NULL,
};



static struct usb_endpoint_descriptor hs_notify_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize =	cpu_to_le16(STATUS_BYTECOUNT),
	.bInterval =		LOG2_STATUS_INTERVAL_MSEC + 4,
};
static struct usb_endpoint_descriptor hs_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_endpoint_descriptor hs_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_descriptor_header *eth_hs_function[] = {
	
	(struct usb_descriptor_header *) &rndis_interface_assoc_desc,
	(struct usb_descriptor_header *) &rndis_control_intf,
	(struct usb_descriptor_header *) &header_desc,
	(struct usb_descriptor_header *) &call_mgmt_descriptor,
	(struct usb_descriptor_header *) &acm_descriptor,
	(struct usb_descriptor_header *) &rndis_union_desc,
	(struct usb_descriptor_header *) &hs_notify_desc,
	
	(struct usb_descriptor_header *) &rndis_data_intf,
	(struct usb_descriptor_header *) &hs_in_desc,
	(struct usb_descriptor_header *) &hs_out_desc,
	NULL,
};



static struct usb_string rndis_string_defs[] = {
	[0].s = "RNDIS Communications Control",
	[1].s = "RNDIS Ethernet Data",
	{  } 
};

static struct usb_gadget_strings rndis_string_table = {
	.language =		0x0409,	
	.strings =		rndis_string_defs,
};

static struct usb_gadget_strings *rndis_strings[] = {
	&rndis_string_table,
	NULL,
};



static struct sk_buff *rndis_add_header(struct gether *port,
					struct sk_buff *skb)
{
	struct sk_buff *skb2;

	skb2 = skb_realloc_headroom(skb, sizeof(struct rndis_packet_msg_type));
	if (skb2)
		rndis_add_hdr(skb2);

	dev_kfree_skb_any(skb);
	return skb2;
}

static void rndis_response_available(void *_rndis)
{
	struct f_rndis			*rndis = _rndis;
	struct usb_request		*req = rndis->notify_req;
	struct usb_composite_dev	*cdev = rndis->port.func.config->cdev;
	__le32				*data = req->buf;
	int				status;

	if (atomic_inc_return(&rndis->notify_count) != 1)
		return;

	
	data[0] = cpu_to_le32(1);
	data[1] = cpu_to_le32(0);

	status = usb_ep_queue(rndis->notify, req, GFP_ATOMIC);
	if (status) {
		atomic_dec(&rndis->notify_count);
		DBG(cdev, "notify/0 --> %d\n", status);
	}
}

static void rndis_response_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_rndis			*rndis = req->context;
	struct usb_composite_dev	*cdev = rndis->port.func.config->cdev;
	int				status = req->status;

	
	switch (status) {
	case -ECONNRESET:
	case -ESHUTDOWN:
		
		atomic_set(&rndis->notify_count, 0);
		break;
	default:
		DBG(cdev, "RNDIS %s response error %d, %d/%d\n",
			ep->name, status,
			req->actual, req->length);
		
	case 0:
		if (ep != rndis->notify)
			break;

		
		if (atomic_dec_and_test(&rndis->notify_count))
			break;
		status = usb_ep_queue(rndis->notify, req, GFP_ATOMIC);
		if (status) {
			atomic_dec(&rndis->notify_count);
			DBG(cdev, "notify/1 --> %d\n", status);
		}
		break;
	}
}

static void rndis_command_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct f_rndis			*rndis = req->context;
	struct usb_composite_dev	*cdev = rndis->port.func.config->cdev;
	int				status;

	

	status = rndis_msg_parser(rndis->config, (u8 *) req->buf);
	if (status < 0)
		ERROR(cdev, "RNDIS command error %d, %d/%d\n",
			status, req->actual, req->length);

}

static int
rndis_setup(struct usb_function *f, const struct usb_ctrlrequest *ctrl)
{
	struct f_rndis		*rndis = func_to_rndis(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	struct usb_request	*req = cdev->req;
	int			value = -EOPNOTSUPP;
	u16			w_index = le16_to_cpu(ctrl->wIndex);
	u16			w_value = le16_to_cpu(ctrl->wValue);
	u16			w_length = le16_to_cpu(ctrl->wLength);

	
	switch ((ctrl->bRequestType << 8) | ctrl->bRequest) {

	
	case ((USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_CDC_SEND_ENCAPSULATED_COMMAND:
		if (w_length > req->length || w_value
				|| w_index != rndis->ctrl_id)
			goto invalid;
		
		value = w_length;
		req->complete = rndis_command_complete;
		req->context = rndis;
		
		break;

	case ((USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE) << 8)
			| USB_CDC_GET_ENCAPSULATED_RESPONSE:
		if (w_value || w_index != rndis->ctrl_id)
			goto invalid;
		else {
			u8 *buf;
			u32 n;

			
			buf = rndis_get_next_response(rndis->config, &n);
			if (buf) {
				memcpy(req->buf, buf, n);
				req->complete = rndis_response_complete;
				rndis_free_response(rndis->config, buf);
				value = n;
			}
			
		}
		break;

	default:
invalid:
		VDBG(cdev, "invalid control req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
	}

	
	if (value >= 0) {
		DBG(cdev, "rndis req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
		req->zero = (value < w_length);
		req->length = value;
		value = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);
		if (value < 0)
			ERROR(cdev, "rndis response on err %d\n", value);
	}

	
	return value;
}


static int rndis_set_alt(struct usb_function *f, unsigned intf, unsigned alt)
{
	struct f_rndis		*rndis = func_to_rndis(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	

	if (intf == rndis->ctrl_id) {
		if (rndis->notify->driver_data) {
			VDBG(cdev, "reset rndis control %d\n", intf);
			usb_ep_disable(rndis->notify);
		} else {
			VDBG(cdev, "init rndis ctrl %d\n", intf);
			rndis->notify_desc = ep_choose(cdev->gadget,
					rndis->hs.notify,
					rndis->fs.notify);
		}
		usb_ep_enable(rndis->notify, rndis->notify_desc);
		rndis->notify->driver_data = rndis;

	} else if (intf == rndis->data_id) {
		struct net_device	*net;

		if (rndis->port.in_ep->driver_data) {
			DBG(cdev, "reset rndis\n");
			gether_disconnect(&rndis->port);
		}

		if (!rndis->port.in) {
			DBG(cdev, "init rndis\n");
			rndis->port.in = ep_choose(cdev->gadget,
					rndis->hs.in, rndis->fs.in);
			rndis->port.out = ep_choose(cdev->gadget,
					rndis->hs.out, rndis->fs.out);
		}

		
		rndis->port.is_zlp_ok = false;

		
		rndis->port.cdc_filter = 0;

		DBG(cdev, "RNDIS RX/TX early activation ... \n");
		net = gether_connect(&rndis->port);
		if (IS_ERR(net))
			return PTR_ERR(net);

		rndis_set_param_dev(rndis->config, net,
				&rndis->port.cdc_filter);
	} else
		goto fail;

	return 0;
fail:
	return -EINVAL;
}

static void rndis_disable(struct usb_function *f)
{
	struct f_rndis		*rndis = func_to_rndis(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	if (!rndis->notify->driver_data)
		return;

	DBG(cdev, "rndis deactivated\n");

	rndis_uninit(rndis->config);
	gether_disconnect(&rndis->port);

	usb_ep_disable(rndis->notify);
	rndis->notify->driver_data = NULL;
}





static void rndis_open(struct gether *geth)
{
	struct f_rndis		*rndis = func_to_rndis(&geth->func);
	struct usb_composite_dev *cdev = geth->func.config->cdev;

	DBG(cdev, "%s\n", __func__);

	rndis_set_param_medium(rndis->config, NDIS_MEDIUM_802_3,
				bitrate(cdev->gadget) / 100);
	rndis_signal_connect(rndis->config);
}

static void rndis_close(struct gether *geth)
{
	struct f_rndis		*rndis = func_to_rndis(&geth->func);

	DBG(geth->func.config->cdev, "%s\n", __func__);

	rndis_set_param_medium(rndis->config, NDIS_MEDIUM_802_3, 0);
	rndis_signal_disconnect(rndis->config);
}





static int
rndis_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct f_rndis		*rndis = func_to_rndis(f);
	int			status;
	struct usb_ep		*ep;

	int vendorID= 0x1004;
    char *manufacturer="LG Electronics Inc.";
	

	
	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	rndis->ctrl_id = status;

	rndis_control_intf.bInterfaceNumber = status;
	rndis_union_desc.bMasterInterface0 = status;

	status = usb_interface_id(c, f);
	if (status < 0)
		goto fail;
	rndis->data_id = status;

	rndis_data_intf.bInterfaceNumber = status;
	rndis_union_desc.bSlaveInterface0 = status;

	status = -ENODEV;

	
	ep = usb_ep_autoconfig(cdev->gadget, &fs_in_desc);
	if (!ep)
		goto fail;
	rndis->port.in_ep = ep;
	ep->driver_data = cdev;	

	ep = usb_ep_autoconfig(cdev->gadget, &fs_out_desc);
	if (!ep)
		goto fail;
	rndis->port.out_ep = ep;
	ep->driver_data = cdev;	

	
	ep = usb_ep_autoconfig(cdev->gadget, &fs_notify_desc);
	if (!ep)
		goto fail;
	rndis->notify = ep;
	ep->driver_data = cdev;	

	status = -ENOMEM;

	
	rndis->notify_req = usb_ep_alloc_request(ep, GFP_KERNEL);
	if (!rndis->notify_req)
		goto fail;
	rndis->notify_req->buf = kmalloc(STATUS_BYTECOUNT, GFP_KERNEL);
	if (!rndis->notify_req->buf)
		goto fail;
	rndis->notify_req->length = STATUS_BYTECOUNT;
	rndis->notify_req->context = rndis;
	rndis->notify_req->complete = rndis_response_complete;

	
	f->descriptors = usb_copy_descriptors(eth_fs_function);
	if (!f->descriptors)
		goto fail;

	rndis->fs.in = usb_find_endpoint(eth_fs_function,
			f->descriptors, &fs_in_desc);
	rndis->fs.out = usb_find_endpoint(eth_fs_function,
			f->descriptors, &fs_out_desc);
	rndis->fs.notify = usb_find_endpoint(eth_fs_function,
			f->descriptors, &fs_notify_desc);

	
	if (gadget_is_dualspeed(c->cdev->gadget)) {
		hs_in_desc.bEndpointAddress =
				fs_in_desc.bEndpointAddress;
		hs_out_desc.bEndpointAddress =
				fs_out_desc.bEndpointAddress;
		hs_notify_desc.bEndpointAddress =
				fs_notify_desc.bEndpointAddress;

		
		f->hs_descriptors = usb_copy_descriptors(eth_hs_function);

		if (!f->hs_descriptors)
			goto fail;

		rndis->hs.in = usb_find_endpoint(eth_hs_function,
				f->hs_descriptors, &hs_in_desc);
		rndis->hs.out = usb_find_endpoint(eth_hs_function,
				f->hs_descriptors, &hs_out_desc);
		rndis->hs.notify = usb_find_endpoint(eth_hs_function,
				f->hs_descriptors, &hs_notify_desc);
	}

	rndis->port.open = rndis_open;
	rndis->port.close = rndis_close;

	status = rndis_register(rndis_response_available, rndis);
	if (status < 0)
		goto fail;
	rndis->config = status;

	rndis_set_param_medium(rndis->config, NDIS_MEDIUM_802_3, 0);
	rndis_set_host_mac(rndis->config, rndis->ethaddr);


#if 1 

	if (rndis_set_param_vendor(rndis->config, vendorID,
				manufacturer))
		goto fail;
#endif
	

	

	DBG(cdev, "RNDIS: %s speed IN/%s OUT/%s NOTIFY/%s\n",
			gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full",
			rndis->port.in_ep->name, rndis->port.out_ep->name,
			rndis->notify->name);
	return 0;

fail:
	if (gadget_is_dualspeed(c->cdev->gadget) && f->hs_descriptors)
		usb_free_descriptors(f->hs_descriptors);
	if (f->descriptors)
		usb_free_descriptors(f->descriptors);

	if (rndis->notify_req) {
		kfree(rndis->notify_req->buf);
		usb_ep_free_request(rndis->notify, rndis->notify_req);
	}

	
	if (rndis->notify)
		rndis->notify->driver_data = NULL;
	if (rndis->port.out)
		rndis->port.out_ep->driver_data = NULL;
	if (rndis->port.in)
		rndis->port.in_ep->driver_data = NULL;

	ERROR(cdev, "%s: can't bind, err %d\n", f->name, status);

	return status;
}

static void
rndis_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct f_rndis		*rndis = func_to_rndis(f);

	rndis_deregister(rndis->config);
	rndis_exit();

	if (gadget_is_dualspeed(c->cdev->gadget))
		usb_free_descriptors(f->hs_descriptors);
	usb_free_descriptors(f->descriptors);

	kfree(rndis->notify_req->buf);
	usb_ep_free_request(rndis->notify, rndis->notify_req);

	kfree(rndis);
}


static inline bool can_support_rndis(struct usb_configuration *c)
{
	
	if (gadget_is_sa1100(c->cdev->gadget))
		return false;

	
	return true;
}


int rndis_bind_config(struct usb_configuration *c, u8 ethaddr[ETH_ALEN])
{
	struct f_rndis	*rndis;
	int		status;

	if (!can_support_rndis(c) || !ethaddr)
		return -EINVAL;

	
	if (rndis_string_defs[0].id == 0) {

		
		status = rndis_init();
		if (status < 0)
			return status;

		
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		rndis_string_defs[0].id = status;
		rndis_control_intf.iInterface = status;

		
		status = usb_string_id(c->cdev);
		if (status < 0)
			return status;
		rndis_string_defs[1].id = status;
		rndis_data_intf.iInterface = status;
	}

	
	status = -ENOMEM;
	rndis = kzalloc(sizeof *rndis, GFP_KERNEL);
	if (!rndis)
		goto fail;

	memcpy(rndis->ethaddr, ethaddr, ETH_ALEN);

	
	rndis->port.cdc_filter = 0;

	
	rndis->port.header_len = sizeof(struct rndis_packet_msg_type);
	rndis->port.wrap = rndis_add_header;
	rndis->port.unwrap = rndis_rm_hdr;

	rndis->port.func.name = "rndis";
	rndis->port.func.strings = rndis_strings;
	
	rndis->port.func.bind = rndis_bind;
	rndis->port.func.unbind = rndis_unbind;
	rndis->port.func.set_alt = rndis_set_alt;
	rndis->port.func.setup = rndis_setup;
	rndis->port.func.disable = rndis_disable;

	status = usb_add_function(c, &rndis->port.func);
	if (status) {
		kfree(rndis);
fail:
		rndis_exit();
	}
	return status;
}
