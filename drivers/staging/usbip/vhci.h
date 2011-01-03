

#include <linux/platform_device.h>
#include "../../usb/core/hcd.h"


struct vhci_device {
	struct usb_device *udev;

	
	__u32 devid;

	
	enum usb_device_speed speed;

	
	__u32 rhport;

	struct usbip_device ud;


	
	spinlock_t priv_lock;

	
	struct list_head priv_tx;
	struct list_head priv_rx;

	
	struct list_head unlink_tx;
	struct list_head unlink_rx;

	
	wait_queue_head_t waitq_tx;
};



struct vhci_priv {
	unsigned long seqnum;
	struct list_head list;

	struct vhci_device *vdev;
	struct urb *urb;
};


struct vhci_unlink {
	
	unsigned long seqnum;

	struct list_head list;

	
	unsigned long unlink_seqnum;
};


#define VHCI_NPORTS 8


struct vhci_hcd {
	spinlock_t	lock;

	u32	port_status[VHCI_NPORTS];

	unsigned	resuming:1;
	unsigned long	re_timeout;

	atomic_t seqnum;

	
	struct vhci_device vdev[VHCI_NPORTS];

	
	int pending_port;
};


extern struct vhci_hcd *the_controller;
extern struct attribute_group dev_attr_group;






void rh_port_connect(int rhport, enum usb_device_speed speed);
void rh_port_disconnect(int rhport);
void vhci_rx_loop(struct usbip_task *ut);
void vhci_tx_loop(struct usbip_task *ut);

#define hardware		(&the_controller->pdev.dev)

static inline struct vhci_device *port_to_vdev(__u32 port)
{
	return &the_controller->vdev[port];
}

static inline struct vhci_hcd *hcd_to_vhci(struct usb_hcd *hcd)
{
	return (struct vhci_hcd *) (hcd->hcd_priv);
}

static inline struct usb_hcd *vhci_to_hcd(struct vhci_hcd *vhci)
{
	return container_of((void *) vhci, struct usb_hcd, hcd_priv);
}

static inline struct device *vhci_dev(struct vhci_hcd *vhci)
{
	return vhci_to_hcd(vhci)->self.controller;
}
