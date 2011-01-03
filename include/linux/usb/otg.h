


#ifndef __LINUX_USB_OTG_H
#define __LINUX_USB_OTG_H


enum usb_otg_state {
	OTG_STATE_UNDEFINED = 0,

	
	OTG_STATE_B_IDLE,
	OTG_STATE_B_SRP_INIT,
	OTG_STATE_B_PERIPHERAL,

	
	OTG_STATE_B_WAIT_ACON,
	OTG_STATE_B_HOST,

	
	OTG_STATE_A_IDLE,
	OTG_STATE_A_WAIT_VRISE,
	OTG_STATE_A_WAIT_BCON,
	OTG_STATE_A_HOST,
	OTG_STATE_A_SUSPEND,
	OTG_STATE_A_PERIPHERAL,
	OTG_STATE_A_WAIT_VFALL,
	OTG_STATE_A_VBUS_ERR,
};


struct otg_transceiver {
	struct device		*dev;
	const char		*label;

	u8			default_a;
	enum usb_otg_state	state;

	struct usb_bus		*host;
	struct usb_gadget	*gadget;

	
	u16			port_status;
	u16			port_change;

	
	int	(*set_host)(struct otg_transceiver *otg,
				struct usb_bus *host);

	
	int	(*set_peripheral)(struct otg_transceiver *otg,
				struct usb_gadget *gadget);

	
	int	(*set_power)(struct otg_transceiver *otg,
				unsigned mA);

	
	int	(*set_suspend)(struct otg_transceiver *otg,
				int suspend);

	
	int	(*start_srp)(struct otg_transceiver *otg);

	
	int	(*start_hnp)(struct otg_transceiver *otg);

};



extern int otg_set_transceiver(struct otg_transceiver *);


extern void usb_nop_xceiv_register(void);
extern void usb_nop_xceiv_unregister(void);



extern struct otg_transceiver *otg_get_transceiver(void);
extern void otg_put_transceiver(struct otg_transceiver *);


static inline int
otg_start_hnp(struct otg_transceiver *otg)
{
	return otg->start_hnp(otg);
}



static inline int
otg_set_host(struct otg_transceiver *otg, struct usb_bus *host)
{
	return otg->set_host(otg, host);
}





static inline int
otg_set_peripheral(struct otg_transceiver *otg, struct usb_gadget *periph)
{
	return otg->set_peripheral(otg, periph);
}

static inline int
otg_set_power(struct otg_transceiver *otg, unsigned mA)
{
	return otg->set_power(otg, mA);
}


static inline int
otg_set_suspend(struct otg_transceiver *otg, int suspend)
{
	if (otg->set_suspend != NULL)
		return otg->set_suspend(otg, suspend);
	else
		return 0;
}

static inline int
otg_start_srp(struct otg_transceiver *otg)
{
	return otg->start_srp(otg);
}



extern int usb_bus_start_enum(struct usb_bus *bus, unsigned port_num);

#endif 
