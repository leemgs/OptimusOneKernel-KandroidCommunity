

#ifndef _MUSB_HOST_H
#define _MUSB_HOST_H

static inline struct usb_hcd *musb_to_hcd(struct musb *musb)
{
	return container_of((void *) musb, struct usb_hcd, hcd_priv);
}

static inline struct musb *hcd_to_musb(struct usb_hcd *hcd)
{
	return (struct musb *) (hcd->hcd_priv);
}


struct musb_qh {
	struct usb_host_endpoint *hep;		
	struct usb_device	*dev;
	struct musb_hw_ep	*hw_ep;		

	struct list_head	ring;		
		
	u8			mux;		

	unsigned		offset;		
	unsigned		segsize;	

	u8			type_reg;	
	u8			intv_reg;	
	u8			addr_reg;	
	u8			h_addr_reg;	
	u8			h_port_reg;	

	u8			is_ready;	
	u8			type;		
	u8			epnum;
	u8			hb_mult;	
	u16			maxpacket;
	u16			frame;		
	unsigned		iso_idx;	
};


static inline struct musb_qh *first_qh(struct list_head *q)
{
	if (list_empty(q))
		return NULL;
	return list_entry(q->next, struct musb_qh, ring);
}


extern void musb_root_disconnect(struct musb *musb);

struct usb_hcd;

extern int musb_hub_status_data(struct usb_hcd *hcd, char *buf);
extern int musb_hub_control(struct usb_hcd *hcd,
			u16 typeReq, u16 wValue, u16 wIndex,
			char *buf, u16 wLength);

extern const struct hc_driver musb_hc_driver;

static inline struct urb *next_urb(struct musb_qh *qh)
{
#ifdef CONFIG_USB_MUSB_HDRC_HCD
	struct list_head	*queue;

	if (!qh)
		return NULL;
	queue = &qh->hep->urb_list;
	if (list_empty(queue))
		return NULL;
	return list_entry(queue->next, struct urb, urb_list);
#else
	return NULL;
#endif
}

#endif				
