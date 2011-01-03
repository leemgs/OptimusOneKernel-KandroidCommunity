

#ifndef	__LINUX_USB_COMPOSITE_H
#define	__LINUX_USB_COMPOSITE_H



#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>


struct usb_configuration;


struct usb_function {
	const char			*name;
	struct usb_gadget_strings	**strings;
	struct usb_descriptor_header	**descriptors;
	struct usb_descriptor_header	**hs_descriptors;

	struct usb_configuration	*config;
	int				hidden;

	

	
	int			(*bind)(struct usb_configuration *,
					struct usb_function *);
	void			(*unbind)(struct usb_configuration *,
					struct usb_function *);

	
	int			(*set_alt)(struct usb_function *,
					unsigned interface, unsigned alt);
	int			(*get_alt)(struct usb_function *,
					unsigned interface);
	void			(*disable)(struct usb_function *);
	int			(*setup)(struct usb_function *,
					const struct usb_ctrlrequest *);
	void			(*suspend)(struct usb_function *);
	void			(*resume)(struct usb_function *);

	
	
	struct list_head		list;
	struct device			*dev;
};

int usb_add_function(struct usb_configuration *, struct usb_function *);

int usb_function_deactivate(struct usb_function *);
int usb_function_activate(struct usb_function *);

int usb_interface_id(struct usb_configuration *, struct usb_function *);


static inline struct usb_endpoint_descriptor *
ep_choose(struct usb_gadget *g, struct usb_endpoint_descriptor *hs,
		struct usb_endpoint_descriptor *fs)
{
	if (gadget_is_dualspeed(g) && g->speed == USB_SPEED_HIGH)
		return hs;
	return fs;
}

#define	MAX_CONFIG_INTERFACES		16	


struct usb_configuration {
	const char			*label;
	struct usb_gadget_strings	**strings;
	const struct usb_descriptor_header **descriptors;

	

	
	int			(*bind)(struct usb_configuration *);
	void			(*unbind)(struct usb_configuration *);
	int			(*setup)(struct usb_configuration *,
					const struct usb_ctrlrequest *);

	
	u8			bConfigurationValue;
	u8			iConfiguration;
	u8			bmAttributes;
	u8			bMaxPower;

	struct usb_composite_dev	*cdev;

	
	
	struct list_head	list;
	struct list_head	functions;
	u8			next_interface_id;
	unsigned		highspeed:1;
	unsigned		fullspeed:1;
	struct usb_function	*interface[MAX_CONFIG_INTERFACES];
};

int usb_add_config(struct usb_composite_dev *,
		struct usb_configuration *);


struct usb_composite_driver {
	const char				*name;
	const struct usb_device_descriptor	*dev;
	struct usb_gadget_strings		**strings;

	struct class		*class;
	atomic_t		function_count;

	

	int			(*bind)(struct usb_composite_dev *);
	int			(*unbind)(struct usb_composite_dev *);

	
	void			(*suspend)(struct usb_composite_dev *);
	void			(*resume)(struct usb_composite_dev *);

	void			(*enable_function)(struct usb_function *f, int enable);
};

extern int usb_composite_register(struct usb_composite_driver *);
extern void usb_composite_unregister(struct usb_composite_driver *);



struct usb_composite_dev {
	struct usb_gadget		*gadget;
	struct usb_request		*req;
	unsigned			bufsiz;

	struct usb_configuration	*config;

	
	
	struct usb_device_descriptor	desc;
	struct list_head		configs;
	struct usb_composite_driver	*driver;
	u8				next_string_id;

	
	unsigned			deactivations;

	
	spinlock_t			lock;
};

extern int usb_string_id(struct usb_composite_dev *c);


#define DBG(d, fmt, args...) \
	dev_dbg(&(d)->gadget->dev , fmt , ## args)
#define VDBG(d, fmt, args...) \
	dev_vdbg(&(d)->gadget->dev , fmt , ## args)
#define ERROR(d, fmt, args...) \
	dev_err(&(d)->gadget->dev , fmt , ## args)
#define WARNING(d, fmt, args...) \
	dev_warn(&(d)->gadget->dev , fmt , ## args)
#define INFO(d, fmt, args...) \
	dev_info(&(d)->gadget->dev , fmt , ## args)

#endif	
