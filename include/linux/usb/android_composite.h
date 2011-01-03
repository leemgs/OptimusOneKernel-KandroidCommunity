
#ifndef	__LINUX_USB_ANDROID_H
#define	__LINUX_USB_ANDROID_H

#include <linux/usb/composite.h>
#include <linux/if_ether.h>

struct android_usb_function {
	struct list_head	list;
	char			*name;
	int 			(*bind_config)(struct usb_configuration *c);
};

struct android_usb_product {
	
	__u16 product_id;

	
	int num_functions;
	char **functions;
};

struct android_usb_platform_data {
	
	__u16 vendor_id;

	
	__u16 product_id;

	__u16 version;

	char *product_name;
	char *manufacturer_name;
	char *serial_number;

	
	int num_products;
	struct android_usb_product *products;

	
	int num_functions;
	char **functions;
};


struct usb_mass_storage_platform_data {
	
	char *vendor;
	char *product;
	int release;

	
	int nluns;
};


struct usb_ether_platform_data {
	u8	ethaddr[ETH_ALEN];
	u32	vendorID;
	const char *vendorDescr;
};

extern void android_usb_set_connected(int on);

extern void android_register_function(struct android_usb_function *f);

extern void android_enable_function(struct usb_function *f, int enable);


#endif	
