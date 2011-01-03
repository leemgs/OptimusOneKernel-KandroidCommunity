

#ifndef __F_MASS_STORAGE_H
#define __F_MASS_STORAGE_H


#ifdef CONFIG_USB_SUPPORT_LGE_ANDROID_AUTORUN
int mass_storage_function_add(struct usb_composite_dev *cdev,
	struct usb_configuration *c, int nluns, int autorun_enable);
#else
int mass_storage_function_add(struct usb_composite_dev *cdev,
	struct usb_configuration *c, int nluns);
#endif

#endif 
