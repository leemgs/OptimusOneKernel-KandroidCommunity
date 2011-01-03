
#ifndef	__LINUX_USB_ANDROID_H
#define	__LINUX_USB_ANDROID_H
#define ANDROID_ADB		0x0001
#define ANDROID_MSC		0x0002
#define ANDROID_ACM_MODEM	0x0003
#define ANDROID_DIAG		0x0004
#define ANDROID_ACM_NMEA	0x0005
#define ANDROID_GENERIC_MODEM	0x0006
#define ANDROID_GENERIC_NMEA	0x0007
#define ANDROID_CDC_ECM		0x0008
#define ANDROID_RMNET		0x0009
#define ANDROID_RNDIS		0x000A
#ifdef CONFIG_USB_GADGET_LG_MTP_DRIVER
#define ANDROID_MTP   0x000B
#endif
#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_AUTORUN
#define ANDROID_AUTORUN		0x000C
#endif

struct android_usb_platform_data {
	
	__u16 vendor_id;

	__u16 version;
	
	struct usb_composition *compositions;
	int num_compositions;

	char *product_name;
	char *manufacturer_name;

#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_GADGET
	char *serial_number;	
	__u16 init_product_id;
#endif


	
	int nluns;
	int self_powered;
};

struct usb_composition {
	__u16   product_id;
	unsigned long functions;
	__u16   adb_product_id;
	unsigned long adb_functions;
};


struct usb_mass_storage_platform_data {
	
	char *vendor;
	char *product;
	int release;

	
	int nluns;
};
#endif	
