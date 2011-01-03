

#ifndef __U_LGEUSB_H__
#define __U_LGEUSB_H__


int lge_get_usb_serial_number(char *serial_number);
int lge_detect_factory_cable(void);




#ifdef CONFIG_USB_SUPPORT_LGE_GADGET_CDMA

#define LG_FACTORY_CABLE_TYPE 3
#define LG_FACTORY_CABLE_130K_TYPE 10
#define LT_ADB_CABLE 0xff

#endif 


#ifdef CONFIG_USB_SUPPORT_LGE_GADGET_GSM

#define MAX_IMEI_LEN 19
#define LGE_PIF_CABLE 2

#endif 

#endif 
