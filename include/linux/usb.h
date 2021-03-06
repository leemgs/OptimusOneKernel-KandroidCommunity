#ifndef __LINUX_USB_H
#define __LINUX_USB_H

#include <linux/mod_devicetable.h>
#include <linux/usb/ch9.h>

#define USB_MAJOR			180
#define USB_DEVICE_MAJOR		189


#ifdef __KERNEL__

#include <linux/errno.h>        
#include <linux/delay.h>	
#include <linux/interrupt.h>	
#include <linux/list.h>		
#include <linux/kref.h>		
#include <linux/device.h>	
#include <linux/fs.h>		
#include <linux/completion.h>	
#include <linux/sched.h>	
#include <linux/mutex.h>	

struct usb_device;
struct usb_driver;
struct wusb_dev;





struct ep_device;



struct usb_host_ss_ep_comp {
	struct usb_ss_ep_comp_descriptor	desc;
	unsigned char				*extra;   
	int					extralen;
};


struct usb_host_endpoint {
	struct usb_endpoint_descriptor	desc;
	struct list_head		urb_list;
	void				*hcpriv;
	struct ep_device 		*ep_dev;	
	struct usb_host_ss_ep_comp	*ss_ep_comp;	

	unsigned char *extra;   
	int extralen;
	int enabled;
};


struct usb_host_interface {
	struct usb_interface_descriptor	desc;

	
	struct usb_host_endpoint *endpoint;

	char *string;		
	unsigned char *extra;   
	int extralen;
};

enum usb_interface_condition {
	USB_INTERFACE_UNBOUND = 0,
	USB_INTERFACE_BINDING,
	USB_INTERFACE_BOUND,
	USB_INTERFACE_UNBINDING,
};


struct usb_interface {
	
	struct usb_host_interface *altsetting;

	struct usb_host_interface *cur_altsetting;	
	unsigned num_altsetting;	

	
	struct usb_interface_assoc_descriptor *intf_assoc;

	int minor;			
	enum usb_interface_condition condition;		
	unsigned is_active:1;		
	unsigned sysfs_files_created:1;	
	unsigned ep_devs_created:1;	
	unsigned unregistering:1;	
	unsigned needs_remote_wakeup:1;	
	unsigned needs_altsetting0:1;	
	unsigned needs_binding:1;	
	unsigned reset_running:1;

	struct device dev;		
	struct device *usb_dev;
	atomic_t pm_usage_cnt;		
	struct work_struct reset_ws;	
};
#define	to_usb_interface(d) container_of(d, struct usb_interface, dev)
#define	interface_to_usbdev(intf) \
	container_of(intf->dev.parent, struct usb_device, dev)

static inline void *usb_get_intfdata(struct usb_interface *intf)
{
	return dev_get_drvdata(&intf->dev);
}

static inline void usb_set_intfdata(struct usb_interface *intf, void *data)
{
	dev_set_drvdata(&intf->dev, data);
}

struct usb_interface *usb_get_intf(struct usb_interface *intf);
void usb_put_intf(struct usb_interface *intf);


#define USB_MAXINTERFACES	32
#define USB_MAXIADS		USB_MAXINTERFACES/2


struct usb_interface_cache {
	unsigned num_altsetting;	
	struct kref ref;		

	
	struct usb_host_interface altsetting[0];
};
#define	ref_to_usb_interface_cache(r) \
		container_of(r, struct usb_interface_cache, ref)
#define	altsetting_to_usb_interface_cache(a) \
		container_of(a, struct usb_interface_cache, altsetting[0])


struct usb_host_config {
	struct usb_config_descriptor	desc;

	char *string;		

	
	struct usb_interface_assoc_descriptor *intf_assoc[USB_MAXIADS];

	
	struct usb_interface *interface[USB_MAXINTERFACES];

	
	struct usb_interface_cache *intf_cache[USB_MAXINTERFACES];

	unsigned char *extra;   
	int extralen;
};

int __usb_get_extra_descriptor(char *buffer, unsigned size,
	unsigned char type, void **ptr);
#define usb_get_extra_descriptor(ifpoint, type, ptr) \
				__usb_get_extra_descriptor((ifpoint)->extra, \
				(ifpoint)->extralen, \
				type, (void **)ptr)




struct usb_devmap {
	unsigned long devicemap[128 / (8*sizeof(unsigned long))];
};


struct usb_bus {
	struct device *controller;	
	int busnum;			
	const char *bus_name;		
	u8 uses_dma;			
	u8 otg_port;			
	unsigned is_b_host:1;		
	unsigned b_hnp_enable:1;	

	int devnum_next;		

	struct usb_devmap devmap;	
	struct usb_device *root_hub;	
	struct list_head bus_list;	

	int bandwidth_allocated;	
	int bandwidth_int_reqs;		
	int bandwidth_isoc_reqs;	

#ifdef CONFIG_USB_DEVICEFS
	struct dentry *usbfs_dentry;	
#endif

#if defined(CONFIG_USB_MON) || defined(CONFIG_USB_MON_MODULE)
	struct mon_bus *mon_bus;	
	int monitored;			
#endif
};




#define USB_MAXCHILDREN		(31)

struct usb_tt;


struct usb_device {
	int		devnum;
	char		devpath [16];
	u32		route;
	enum usb_device_state	state;
	enum usb_device_speed	speed;

	struct usb_tt	*tt;
	int		ttport;

	unsigned int toggle[2];

	struct usb_device *parent;
	struct usb_bus *bus;
	struct usb_host_endpoint ep0;

	struct device dev;

	struct usb_device_descriptor descriptor;
	struct usb_host_config *config;

	struct usb_host_config *actconfig;
	struct usb_host_endpoint *ep_in[16];
	struct usb_host_endpoint *ep_out[16];

	char **rawdescriptors;

	unsigned short bus_mA;
	u8 portnum;
	u8 level;

	unsigned can_submit:1;
	unsigned discon_suspended:1;
	unsigned persist_enabled:1;
	unsigned have_langid:1;
	unsigned authorized:1;
 	unsigned authenticated:1;
	unsigned wusb:1;
	int string_langid;

	
	char *product;
	char *manufacturer;
	char *serial;

	struct list_head filelist;
#ifdef CONFIG_USB_DEVICE_CLASS
	struct device *usb_classdev;
#endif
#ifdef CONFIG_USB_DEVICEFS
	struct dentry *usbfs_dentry;
#endif

	int maxchild;
	struct usb_device *children[USB_MAXCHILDREN];

	int pm_usage_cnt;
	u32 quirks;
	atomic_t urbnum;

	unsigned long active_duration;

#ifdef CONFIG_PM
	struct delayed_work autosuspend;
	struct work_struct autoresume;
	struct mutex pm_mutex;

	unsigned long last_busy;
	int autosuspend_delay;
	unsigned long connect_time;

	unsigned auto_pm:1;
	unsigned do_remote_wakeup:1;
	unsigned reset_resume:1;
	unsigned autosuspend_disabled:1;
	unsigned autoresume_disabled:1;
	unsigned skip_sys_resume:1;
#endif
	struct wusb_dev *wusb_dev;
	int slot_id;
};
#define	to_usb_device(d) container_of(d, struct usb_device, dev)

extern struct usb_device *usb_get_dev(struct usb_device *dev);
extern void usb_put_dev(struct usb_device *dev);


#define usb_lock_device(udev)		down(&(udev)->dev.sem)
#define usb_unlock_device(udev)		up(&(udev)->dev.sem)
#define usb_trylock_device(udev)	down_trylock(&(udev)->dev.sem)
extern int usb_lock_device_for_reset(struct usb_device *udev,
				     const struct usb_interface *iface);


extern int usb_reset_device(struct usb_device *dev);
extern void usb_queue_reset_device(struct usb_interface *dev);

extern struct usb_device *usb_find_device(u16 vendor_id, u16 product_id);


#ifdef CONFIG_USB_SUSPEND
extern int usb_autopm_set_interface(struct usb_interface *intf);
extern int usb_autopm_get_interface(struct usb_interface *intf);
extern void usb_autopm_put_interface(struct usb_interface *intf);
extern int usb_autopm_get_interface_async(struct usb_interface *intf);
extern void usb_autopm_put_interface_async(struct usb_interface *intf);

static inline void usb_autopm_enable(struct usb_interface *intf)
{
	atomic_set(&intf->pm_usage_cnt, 0);
	usb_autopm_set_interface(intf);
}

static inline void usb_autopm_disable(struct usb_interface *intf)
{
	atomic_set(&intf->pm_usage_cnt, 1);
	usb_autopm_set_interface(intf);
}

static inline void usb_mark_last_busy(struct usb_device *udev)
{
	udev->last_busy = jiffies;
}

#else

static inline int usb_autopm_set_interface(struct usb_interface *intf)
{ return 0; }

static inline int usb_autopm_get_interface(struct usb_interface *intf)
{ return 0; }

static inline int usb_autopm_get_interface_async(struct usb_interface *intf)
{ return 0; }

static inline void usb_autopm_put_interface(struct usb_interface *intf)
{ }
static inline void usb_autopm_put_interface_async(struct usb_interface *intf)
{ }
static inline void usb_autopm_enable(struct usb_interface *intf)
{ }
static inline void usb_autopm_disable(struct usb_interface *intf)
{ }
static inline void usb_mark_last_busy(struct usb_device *udev)
{ }
#endif




extern int usb_get_current_frame_number(struct usb_device *usb_dev);


extern int usb_driver_claim_interface(struct usb_driver *driver,
			struct usb_interface *iface, void *priv);


static inline int usb_interface_claimed(struct usb_interface *iface)
{
	return (iface->dev.driver != NULL);
}

extern void usb_driver_release_interface(struct usb_driver *driver,
			struct usb_interface *iface);
const struct usb_device_id *usb_match_id(struct usb_interface *interface,
					 const struct usb_device_id *id);
extern int usb_match_one_id(struct usb_interface *interface,
			    const struct usb_device_id *id);

extern struct usb_interface *usb_find_interface(struct usb_driver *drv,
		int minor);
extern struct usb_interface *usb_ifnum_to_if(const struct usb_device *dev,
		unsigned ifnum);
extern struct usb_host_interface *usb_altnum_to_altsetting(
		const struct usb_interface *intf, unsigned int altnum);



static inline int usb_make_path(struct usb_device *dev, char *buf, size_t size)
{
	int actual;
	actual = snprintf(buf, size, "usb-%s-%s", dev->bus->bus_name,
			  dev->devpath);
	return (actual >= (int)size) ? -1 : actual;
}



#define USB_DEVICE_ID_MATCH_DEVICE \
		(USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT)
#define USB_DEVICE_ID_MATCH_DEV_RANGE \
		(USB_DEVICE_ID_MATCH_DEV_LO | USB_DEVICE_ID_MATCH_DEV_HI)
#define USB_DEVICE_ID_MATCH_DEVICE_AND_VERSION \
		(USB_DEVICE_ID_MATCH_DEVICE | USB_DEVICE_ID_MATCH_DEV_RANGE)
#define USB_DEVICE_ID_MATCH_DEV_INFO \
		(USB_DEVICE_ID_MATCH_DEV_CLASS | \
		USB_DEVICE_ID_MATCH_DEV_SUBCLASS | \
		USB_DEVICE_ID_MATCH_DEV_PROTOCOL)
#define USB_DEVICE_ID_MATCH_INT_INFO \
		(USB_DEVICE_ID_MATCH_INT_CLASS | \
		USB_DEVICE_ID_MATCH_INT_SUBCLASS | \
		USB_DEVICE_ID_MATCH_INT_PROTOCOL)


#define USB_DEVICE(vend,prod) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE, \
	.idVendor = (vend), \
	.idProduct = (prod)

#define USB_DEVICE_VER(vend, prod, lo, hi) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE_AND_VERSION, \
	.idVendor = (vend), \
	.idProduct = (prod), \
	.bcdDevice_lo = (lo), \
	.bcdDevice_hi = (hi)


#define USB_DEVICE_INTERFACE_PROTOCOL(vend, prod, pr) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE | \
		       USB_DEVICE_ID_MATCH_INT_PROTOCOL, \
	.idVendor = (vend), \
	.idProduct = (prod), \
	.bInterfaceProtocol = (pr)


#define USB_DEVICE_INFO(cl, sc, pr) \
	.match_flags = USB_DEVICE_ID_MATCH_DEV_INFO, \
	.bDeviceClass = (cl), \
	.bDeviceSubClass = (sc), \
	.bDeviceProtocol = (pr)


#define USB_INTERFACE_INFO(cl, sc, pr) \
	.match_flags = USB_DEVICE_ID_MATCH_INT_INFO, \
	.bInterfaceClass = (cl), \
	.bInterfaceSubClass = (sc), \
	.bInterfaceProtocol = (pr)


#define USB_DEVICE_AND_INTERFACE_INFO(vend, prod, cl, sc, pr) \
	.match_flags = USB_DEVICE_ID_MATCH_INT_INFO \
		| USB_DEVICE_ID_MATCH_DEVICE, \
	.idVendor = (vend), \
	.idProduct = (prod), \
	.bInterfaceClass = (cl), \
	.bInterfaceSubClass = (sc), \
	.bInterfaceProtocol = (pr)




struct usb_dynids {
	spinlock_t lock;
	struct list_head list;
};

struct usb_dynid {
	struct list_head node;
	struct usb_device_id id;
};

extern ssize_t usb_store_new_id(struct usb_dynids *dynids,
				struct device_driver *driver,
				const char *buf, size_t count);


struct usbdrv_wrap {
	struct device_driver driver;
	int for_devices;
};


struct usb_driver {
	const char *name;

	int (*probe) (struct usb_interface *intf,
		      const struct usb_device_id *id);

	void (*disconnect) (struct usb_interface *intf);

	int (*ioctl) (struct usb_interface *intf, unsigned int code,
			void *buf);

	int (*suspend) (struct usb_interface *intf, pm_message_t message);
	int (*resume) (struct usb_interface *intf);
	int (*reset_resume)(struct usb_interface *intf);

	int (*pre_reset)(struct usb_interface *intf);
	int (*post_reset)(struct usb_interface *intf);

	const struct usb_device_id *id_table;

	struct usb_dynids dynids;
	struct usbdrv_wrap drvwrap;
	unsigned int no_dynamic_id:1;
	unsigned int supports_autosuspend:1;
	unsigned int soft_unbind:1;
};
#define	to_usb_driver(d) container_of(d, struct usb_driver, drvwrap.driver)


struct usb_device_driver {
	const char *name;

	int (*probe) (struct usb_device *udev);
	void (*disconnect) (struct usb_device *udev);

	int (*suspend) (struct usb_device *udev, pm_message_t message);
	int (*resume) (struct usb_device *udev, pm_message_t message);
	struct usbdrv_wrap drvwrap;
	unsigned int supports_autosuspend:1;
};
#define	to_usb_device_driver(d) container_of(d, struct usb_device_driver, \
		drvwrap.driver)

extern struct bus_type usb_bus_type;


struct usb_class_driver {
	char *name;
	char *(*devnode)(struct device *dev, mode_t *mode);
	const struct file_operations *fops;
	int minor_base;
};


extern int usb_register_driver(struct usb_driver *, struct module *,
			       const char *);
static inline int usb_register(struct usb_driver *driver)
{
	return usb_register_driver(driver, THIS_MODULE, KBUILD_MODNAME);
}
extern void usb_deregister(struct usb_driver *);

extern int usb_register_device_driver(struct usb_device_driver *,
			struct module *);
extern void usb_deregister_device_driver(struct usb_device_driver *);

extern int usb_register_dev(struct usb_interface *intf,
			    struct usb_class_driver *class_driver);
extern void usb_deregister_dev(struct usb_interface *intf,
			       struct usb_class_driver *class_driver);

extern int usb_disabled(void);






#define URB_SHORT_NOT_OK	0x0001	
#define URB_ISO_ASAP		0x0002	
#define URB_NO_TRANSFER_DMA_MAP	0x0004	
#define URB_NO_SETUP_DMA_MAP	0x0008	
#define URB_NO_FSBR		0x0020	
#define URB_ZERO_PACKET		0x0040	
#define URB_NO_INTERRUPT	0x0080	
#define URB_FREE_BUFFER		0x0100	

#define URB_DIR_IN		0x0200	
#define URB_DIR_OUT		0
#define URB_DIR_MASK		URB_DIR_IN

struct usb_iso_packet_descriptor {
	unsigned int offset;
	unsigned int length;		
	unsigned int actual_length;
	int status;
};

struct urb;

struct usb_anchor {
	struct list_head urb_list;
	wait_queue_head_t wait;
	spinlock_t lock;
	unsigned int poisoned:1;
};

static inline void init_usb_anchor(struct usb_anchor *anchor)
{
	INIT_LIST_HEAD(&anchor->urb_list);
	init_waitqueue_head(&anchor->wait);
	spin_lock_init(&anchor->lock);
}

typedef void (*usb_complete_t)(struct urb *);


struct urb {
	
	struct kref kref;		
	void *hcpriv;			
	atomic_t use_count;		
	atomic_t reject;		
	int unlinked;			

	
	struct list_head urb_list;	
	struct list_head anchor_list;	
	struct usb_anchor *anchor;
	struct usb_device *dev; 	
	struct usb_host_endpoint *ep;	
	unsigned int pipe;		
	int status;			
	unsigned int transfer_flags;	
	void *transfer_buffer;		
	dma_addr_t transfer_dma;	
	struct usb_sg_request *sg;	
	int num_sgs;			
	u32 transfer_buffer_length;	
	u32 actual_length;		
	unsigned char *setup_packet;	
	dma_addr_t setup_dma;		
	int start_frame;		
	int number_of_packets;		
	int interval;			
	int error_count;		
	void *context;			
	usb_complete_t complete;	
	struct usb_iso_packet_descriptor iso_frame_desc[0];
					
};




static inline void usb_fill_control_urb(struct urb *urb,
					struct usb_device *dev,
					unsigned int pipe,
					unsigned char *setup_packet,
					void *transfer_buffer,
					int buffer_length,
					usb_complete_t complete_fn,
					void *context)
{
	urb->dev = dev;
	urb->pipe = pipe;
	urb->setup_packet = setup_packet;
	urb->transfer_buffer = transfer_buffer;
	urb->transfer_buffer_length = buffer_length;
	urb->complete = complete_fn;
	urb->context = context;
}


static inline void usb_fill_bulk_urb(struct urb *urb,
				     struct usb_device *dev,
				     unsigned int pipe,
				     void *transfer_buffer,
				     int buffer_length,
				     usb_complete_t complete_fn,
				     void *context)
{
	urb->dev = dev;
	urb->pipe = pipe;
	urb->transfer_buffer = transfer_buffer;
	urb->transfer_buffer_length = buffer_length;
	urb->complete = complete_fn;
	urb->context = context;
}


static inline void usb_fill_int_urb(struct urb *urb,
				    struct usb_device *dev,
				    unsigned int pipe,
				    void *transfer_buffer,
				    int buffer_length,
				    usb_complete_t complete_fn,
				    void *context,
				    int interval)
{
	urb->dev = dev;
	urb->pipe = pipe;
	urb->transfer_buffer = transfer_buffer;
	urb->transfer_buffer_length = buffer_length;
	urb->complete = complete_fn;
	urb->context = context;
	if (dev->speed == USB_SPEED_HIGH)
		urb->interval = 1 << (interval - 1);
	else
		urb->interval = interval;
	urb->start_frame = -1;
}

extern void usb_init_urb(struct urb *urb);
extern struct urb *usb_alloc_urb(int iso_packets, gfp_t mem_flags);
extern void usb_free_urb(struct urb *urb);
#define usb_put_urb usb_free_urb
extern struct urb *usb_get_urb(struct urb *urb);
extern int usb_submit_urb(struct urb *urb, gfp_t mem_flags);
extern int usb_unlink_urb(struct urb *urb);
extern void usb_kill_urb(struct urb *urb);
extern void usb_poison_urb(struct urb *urb);
extern void usb_unpoison_urb(struct urb *urb);
extern void usb_kill_anchored_urbs(struct usb_anchor *anchor);
extern void usb_poison_anchored_urbs(struct usb_anchor *anchor);
extern void usb_unpoison_anchored_urbs(struct usb_anchor *anchor);
extern void usb_unlink_anchored_urbs(struct usb_anchor *anchor);
extern void usb_anchor_urb(struct urb *urb, struct usb_anchor *anchor);
extern void usb_unanchor_urb(struct urb *urb);
extern int usb_wait_anchor_empty_timeout(struct usb_anchor *anchor,
					 unsigned int timeout);
extern struct urb *usb_get_from_anchor(struct usb_anchor *anchor);
extern void usb_scuttle_anchored_urbs(struct usb_anchor *anchor);
extern int usb_anchor_empty(struct usb_anchor *anchor);


static inline int usb_urb_dir_in(struct urb *urb)
{
	return (urb->transfer_flags & URB_DIR_MASK) == URB_DIR_IN;
}


static inline int usb_urb_dir_out(struct urb *urb)
{
	return (urb->transfer_flags & URB_DIR_MASK) == URB_DIR_OUT;
}

void *usb_buffer_alloc(struct usb_device *dev, size_t size,
	gfp_t mem_flags, dma_addr_t *dma);
void usb_buffer_free(struct usb_device *dev, size_t size,
	void *addr, dma_addr_t dma);

#if 0
struct urb *usb_buffer_map(struct urb *urb);
void usb_buffer_dmasync(struct urb *urb);
void usb_buffer_unmap(struct urb *urb);
#endif

struct scatterlist;
int usb_buffer_map_sg(const struct usb_device *dev, int is_in,
		      struct scatterlist *sg, int nents);
#if 0
void usb_buffer_dmasync_sg(const struct usb_device *dev, int is_in,
			   struct scatterlist *sg, int n_hw_ents);
#endif
void usb_buffer_unmap_sg(const struct usb_device *dev, int is_in,
			 struct scatterlist *sg, int n_hw_ents);



extern int usb_control_msg(struct usb_device *dev, unsigned int pipe,
	__u8 request, __u8 requesttype, __u16 value, __u16 index,
	void *data, __u16 size, int timeout);
extern int usb_interrupt_msg(struct usb_device *usb_dev, unsigned int pipe,
	void *data, int len, int *actual_length, int timeout);
extern int usb_bulk_msg(struct usb_device *usb_dev, unsigned int pipe,
	void *data, int len, int *actual_length,
	int timeout);


extern int usb_get_descriptor(struct usb_device *dev, unsigned char desctype,
	unsigned char descindex, void *buf, int size);
extern int usb_get_status(struct usb_device *dev,
	int type, int target, void *data);
extern int usb_string(struct usb_device *dev, int index,
	char *buf, size_t size);


extern int usb_clear_halt(struct usb_device *dev, int pipe);
extern int usb_reset_configuration(struct usb_device *dev);
extern int usb_set_interface(struct usb_device *dev, int ifnum, int alternate);
extern void usb_reset_endpoint(struct usb_device *dev, unsigned int epaddr);


extern int usb_driver_set_configuration(struct usb_device *udev, int config);


#define USB_CTRL_GET_TIMEOUT	5000
#define USB_CTRL_SET_TIMEOUT	5000



struct usb_sg_request {
	int			status;
	size_t			bytes;

	
	spinlock_t		lock;

	struct usb_device	*dev;
	int			pipe;
	struct scatterlist	*sg;
	int			nents;

	int			entries;
	struct urb		**urbs;

	int			count;
	struct completion	complete;
};

int usb_sg_init(
	struct usb_sg_request	*io,
	struct usb_device	*dev,
	unsigned		pipe,
	unsigned		period,
	struct scatterlist	*sg,
	int			nents,
	size_t			length,
	gfp_t			mem_flags
);
void usb_sg_cancel(struct usb_sg_request *io);
void usb_sg_wait(struct usb_sg_request *io);








#define PIPE_ISOCHRONOUS		0
#define PIPE_INTERRUPT			1
#define PIPE_CONTROL			2
#define PIPE_BULK			3

#define usb_pipein(pipe)	((pipe) & USB_DIR_IN)
#define usb_pipeout(pipe)	(!usb_pipein(pipe))

#define usb_pipedevice(pipe)	(((pipe) >> 8) & 0x7f)
#define usb_pipeendpoint(pipe)	(((pipe) >> 15) & 0xf)

#define usb_pipetype(pipe)	(((pipe) >> 30) & 3)
#define usb_pipeisoc(pipe)	(usb_pipetype((pipe)) == PIPE_ISOCHRONOUS)
#define usb_pipeint(pipe)	(usb_pipetype((pipe)) == PIPE_INTERRUPT)
#define usb_pipecontrol(pipe)	(usb_pipetype((pipe)) == PIPE_CONTROL)
#define usb_pipebulk(pipe)	(usb_pipetype((pipe)) == PIPE_BULK)

static inline unsigned int __create_pipe(struct usb_device *dev,
		unsigned int endpoint)
{
	return (dev->devnum << 8) | (endpoint << 15);
}


#define usb_sndctrlpipe(dev,endpoint)	\
	((PIPE_CONTROL << 30) | __create_pipe(dev, endpoint))
#define usb_rcvctrlpipe(dev,endpoint)	\
	((PIPE_CONTROL << 30) | __create_pipe(dev, endpoint) | USB_DIR_IN)
#define usb_sndisocpipe(dev,endpoint)	\
	((PIPE_ISOCHRONOUS << 30) | __create_pipe(dev, endpoint))
#define usb_rcvisocpipe(dev,endpoint)	\
	((PIPE_ISOCHRONOUS << 30) | __create_pipe(dev, endpoint) | USB_DIR_IN)
#define usb_sndbulkpipe(dev,endpoint)	\
	((PIPE_BULK << 30) | __create_pipe(dev, endpoint))
#define usb_rcvbulkpipe(dev,endpoint)	\
	((PIPE_BULK << 30) | __create_pipe(dev, endpoint) | USB_DIR_IN)
#define usb_sndintpipe(dev,endpoint)	\
	((PIPE_INTERRUPT << 30) | __create_pipe(dev, endpoint))
#define usb_rcvintpipe(dev,endpoint)	\
	((PIPE_INTERRUPT << 30) | __create_pipe(dev, endpoint) | USB_DIR_IN)



static inline __u16
usb_maxpacket(struct usb_device *udev, int pipe, int is_out)
{
	struct usb_host_endpoint	*ep;
	unsigned			epnum = usb_pipeendpoint(pipe);

	if (is_out) {
		WARN_ON(usb_pipein(pipe));
		ep = udev->ep_out[epnum];
	} else {
		WARN_ON(usb_pipeout(pipe));
		ep = udev->ep_in[epnum];
	}
	if (!ep)
		return 0;

	
	return le16_to_cpu(ep->desc.wMaxPacketSize);
}




#define USB_DEVICE_ADD		0x0001
#define USB_DEVICE_REMOVE	0x0002
#define USB_BUS_ADD		0x0003
#define USB_BUS_REMOVE		0x0004
extern void usb_register_notify(struct notifier_block *nb);
extern void usb_unregister_notify(struct notifier_block *nb);

#ifdef DEBUG
#define dbg(format, arg...) printk(KERN_DEBUG "%s: " format "\n" , \
	__FILE__ , ## arg)
#else
#define dbg(format, arg...) do {} while (0)
#endif

#define err(format, arg...) printk(KERN_ERR KBUILD_MODNAME ": " \
	format "\n" , ## arg)


extern struct dentry *usb_debug_root;

#endif  

#endif
