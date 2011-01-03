

#ifndef _DRIVERS_USB_FUNCTION_USB_FUNCTION_H_
#define _DRIVERS_USB_FUNCTION_USB_FUNCTION_H_

#include <linux/list.h>
#include <linux/usb/ch9.h>

#define EPT_BULK_IN   1
#define EPT_BULK_OUT  2
#define EPT_INT_IN  3

#define USB_CONFIG_ATT_SELFPOWER_POS	(6)	
#define USB_CONFIG_ATT_WAKEUP_POS	(5)	

struct usb_endpoint {
	struct usb_info *ui;
	struct msm_request *req; 
	struct msm_request *last;
	unsigned flags;

	
	unsigned char bit;
	unsigned char num;

	unsigned short max_pkt;

	unsigned ept_halted;

	
	
	struct ept_queue_head *head;
	struct usb_endpoint_descriptor *ep_descriptor;
	unsigned int alloced;
};

struct usb_request {
	void *buf;          
	unsigned length;    
	int status;         
	unsigned actual;    

	void (*complete)(struct usb_endpoint *ep, struct usb_request *req);
	void *context;

	void *device;

	struct list_head list;
};

struct usb_function {
	
	void (*bind)(void *);

	
	void (*unbind)(void *);

	
	void (*configure)(int configured, void *);
	void (*disconnect)(void *);

	
	int (*setup)(struct usb_ctrlrequest *req, void *buf,
			int len, void *);

	int (*set_interface)(int ifc_num, int alt_set, void *_ctxt);
	int (*get_interface)(int ifc_num, void *ctxt);
	
	const char *name;
	void *context;

	
	unsigned char ifc_class;
	unsigned char ifc_subclass;
	unsigned char ifc_protocol;

	
	const char *ifc_name;

	
	unsigned char ifc_ept_count;
	unsigned char ifc_ept_type[8];

	
	unsigned char   disabled;

	struct usb_descriptor_header **fs_descriptors;
	struct usb_descriptor_header **hs_descriptors;

	struct usb_request *ep0_out_req, *ep0_in_req;
	struct usb_endpoint *ep0_out, *ep0_in;
};

int usb_function_register(struct usb_function *driver);
int usb_function_unregister(struct usb_function *driver);

int usb_msm_get_speed(void);
void usb_configure_endpoint(struct usb_endpoint *ep,
			struct usb_endpoint_descriptor *ep_desc);
int usb_remote_wakeup(void);

struct usb_endpoint *usb_alloc_endpoint(unsigned direction);
int usb_free_endpoint(struct usb_endpoint *ept);

void usb_ept_enable(struct usb_endpoint *ept, int yes);
int usb_msm_get_next_ifc_number(struct usb_function *);
int usb_msm_get_next_strdesc_id(char *);
void usb_msm_enable_iad(void);

void usb_function_enable(const char *function, int enable);


struct usb_request *usb_ept_alloc_req(struct usb_endpoint *ept, unsigned bufsize);
void usb_ept_free_req(struct usb_endpoint *ept, struct usb_request *req);


int usb_ept_queue_xfer(struct usb_endpoint *ept, struct usb_request *req);
int usb_ept_flush(struct usb_endpoint *ept);
int usb_ept_get_max_packet(struct usb_endpoint *ept);
int usb_ept_cancel_xfer(struct usb_endpoint *ept, struct usb_request *_req);
void usb_ept_fifo_flush(struct usb_endpoint *ept);
int usb_ept_set_halt(struct usb_endpoint *ept);
int usb_ept_clear_halt(struct usb_endpoint *ept);
struct device *usb_get_device(void);
struct usb_endpoint *usb_ept_find(struct usb_endpoint **ept, int type);
struct usb_function *usb_ept_get_function(struct usb_endpoint *ept);
int usb_ept_is_stalled(struct usb_endpoint *ept);
void usb_request_set_buffer(struct usb_request *req, void *buf, dma_addr_t dma);
void usb_free_endpoint_all_req(struct usb_endpoint *ep);
void usb_remove_function_driver(struct usb_function *func);
int usb_remote_wakeup(void);
#endif
