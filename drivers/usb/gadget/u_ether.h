

#ifndef __U_ETHER_H
#define __U_ETHER_H

#include <linux/err.h>
#include <linux/if_ether.h>
#include <linux/usb/composite.h>
#include <linux/usb/cdc.h>

#include "gadget_chips.h"



struct gether {
	struct usb_function		func;

	
	struct eth_dev			*ioport;

	
	struct usb_ep			*in_ep;
	struct usb_ep			*out_ep;

	
	struct usb_endpoint_descriptor	*in;
	struct usb_endpoint_descriptor	*out;

	bool				is_zlp_ok;

	u16				cdc_filter;

	
	u32				header_len;
	struct sk_buff			*(*wrap)(struct gether *port,
						struct sk_buff *skb);
	int				(*unwrap)(struct gether *port,
						struct sk_buff *skb,
						struct sk_buff_head *list);

	
	void				(*open)(struct gether *);
	void				(*close)(struct gether *);
};

#define	DEFAULT_FILTER	(USB_CDC_PACKET_TYPE_BROADCAST \
			|USB_CDC_PACKET_TYPE_ALL_MULTICAST \
			|USB_CDC_PACKET_TYPE_PROMISCUOUS \
			|USB_CDC_PACKET_TYPE_DIRECTED)



int gether_setup(struct usb_gadget *g, u8 ethaddr[ETH_ALEN]);
void gether_cleanup(void);


struct net_device *gether_connect(struct gether *);
void gether_disconnect(struct gether *);


static inline bool can_support_ecm(struct usb_gadget *gadget)
{
	if (!gadget_supports_altsettings(gadget))
		return false;

	
	if (gadget_is_sa1100(gadget))
		return false;

	
	return true;
}


int geth_bind_config(struct usb_configuration *c, u8 ethaddr[ETH_ALEN]);
int ecm_bind_config(struct usb_configuration *c, u8 ethaddr[ETH_ALEN]);
int eem_bind_config(struct usb_configuration *c);

#if defined(CONFIG_USB_ETH_RNDIS) || defined(CONFIG_USB_ANDROID_RNDIS)

int rndis_bind_config(struct usb_configuration *c, u8 ethaddr[ETH_ALEN]);

#else

static inline int
rndis_bind_config(struct usb_configuration *c, u8 ethaddr[ETH_ALEN])
{
	return 0;
}

#endif

#endif 
