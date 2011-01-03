

#ifndef __U_SERIAL_H
#define __U_SERIAL_H

#include <linux/usb/composite.h>
#include <linux/usb/cdc.h>


struct gserial {
	struct usb_function		func;

	
	struct gs_port			*ioport;

	struct usb_ep			*in;
	struct usb_ep			*out;
	struct usb_endpoint_descriptor	*in_desc;
	struct usb_endpoint_descriptor	*out_desc;

	
	struct usb_cdc_line_coding port_line_coding;	
	u16				serial_state;

	
	unsigned int (*get_dtr)(struct gserial *p);
	unsigned int (*get_rts)(struct gserial *p);

	
	void (*connect)(struct gserial *p);
	void (*disconnect)(struct gserial *p);
	int (*send_break)(struct gserial *p, int duration);
	unsigned int (*send_carrier_detect)(struct gserial *p, unsigned int);
	unsigned int (*send_ring_indicator)(struct gserial *p, unsigned int);
};


struct usb_request *gs_alloc_req(struct usb_ep *ep, unsigned len, gfp_t flags);
void gs_free_req(struct usb_ep *, struct usb_request *req);


int gserial_setup(struct usb_gadget *g, unsigned n_ports);
void gserial_cleanup(void);


int gserial_connect(struct gserial *, u8 port_num);
void gserial_disconnect(struct gserial *);


int acm_bind_config(struct usb_configuration *c, u8 port_num);
int gser_bind_config(struct usb_configuration *c, u8 port_num);
int obex_bind_config(struct usb_configuration *c, u8 port_num);

#endif 
