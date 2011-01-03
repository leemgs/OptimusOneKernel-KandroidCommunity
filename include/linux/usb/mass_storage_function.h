

#ifndef __LINUX_USB_MASS_STORAGE_FUNCTION_H__
#define __LINUX_USB_MASS_STORAGE_FUNCTION_H__


struct usb_mass_storage_platform_data {
	
	int         nluns;
	
	
	u32			buf_size;
	
	
	const char  *vendor;
	const char  *product;
	int			release;
};

#endif 
