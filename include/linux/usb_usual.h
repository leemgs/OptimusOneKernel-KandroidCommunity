

#ifndef __LINUX_USB_USUAL_H
#define __LINUX_USB_USUAL_H








#define US_DO_ALL_FLAGS						\
	US_FLAG(SINGLE_LUN,	0x00000001)			\
				\
	US_FLAG(NEED_OVERRIDE,	0x00000002)			\
				\
	US_FLAG(SCM_MULT_TARG,	0x00000004)			\
					\
	US_FLAG(FIX_INQUIRY,	0x00000008)			\
				\
	US_FLAG(FIX_CAPACITY,	0x00000010)			\
				\
	US_FLAG(IGNORE_RESIDUE,	0x00000020)			\
					\
	US_FLAG(BULK32,		0x00000040)			\
					\
	US_FLAG(NOT_LOCKABLE,	0x00000080)			\
				\
	US_FLAG(GO_SLOW,	0x00000100)			\
				\
	US_FLAG(NO_WP_DETECT,	0x00000200)			\
				\
	US_FLAG(MAX_SECTORS_64,	0x00000400)			\
					\
	US_FLAG(IGNORE_DEVICE,	0x00000800)			\
					\
	US_FLAG(CAPACITY_HEURISTICS,	0x00001000)		\
				\
	US_FLAG(MAX_SECTORS_MIN,0x00002000)			\
				\
	US_FLAG(BULK_IGNORE_TAG,0x00004000)			\
		    \
	US_FLAG(SANE_SENSE,     0x00008000)			\
					\
	US_FLAG(CAPACITY_OK,	0x00010000)			\
				\
	US_FLAG(BAD_SENSE,	0x00020000)			\
		

#define US_FLAG(name, value)	US_FL_##name = value ,
enum { US_DO_ALL_FLAGS };
#undef US_FLAG


#define USB_US_TYPE_NONE   0
#define USB_US_TYPE_STOR   1		
#define USB_US_TYPE_UB     2		

#define USB_US_TYPE(flags) 		(((flags) >> 24) & 0xFF)
#define USB_US_ORIG_FLAGS(flags)	((flags) & 0x00FFFFFF)





#define US_SC_RBC	0x01		
#define US_SC_8020	0x02		
#define US_SC_QIC	0x03		
#define US_SC_UFI	0x04		
#define US_SC_8070	0x05		
#define US_SC_SCSI	0x06		
#define US_SC_LOCKABLE	0x07		

#define US_SC_ISD200    0xf0		
#define US_SC_CYP_ATACB 0xf1		
#define US_SC_DEVICE	0xff		



#define US_PR_CBI	0x00		
#define US_PR_CB	0x01		
#define US_PR_BULK	0x50		

#define US_PR_USBAT	0x80		
#define US_PR_EUSB_SDDR09	0x81	
#define US_PR_SDDR55	0x82		
#define US_PR_DPCM_USB  0xf0		
#define US_PR_FREECOM   0xf1		
#define US_PR_DATAFAB   0xf2		
#define US_PR_JUMPSHOT  0xf3		
#define US_PR_ALAUDA    0xf4		
#define US_PR_KARMA     0xf5		

#define US_PR_DEVICE	0xff		



#ifdef CONFIG_USB_LIBUSUAL


extern int usb_usual_ignore_device(struct usb_interface *intf);
extern struct usb_device_id usb_storage_usb_ids[];

extern void usb_usual_set_present(int type);
extern void usb_usual_clear_present(int type);
extern int usb_usual_check_type(const struct usb_device_id *, int type);
#else

#define usb_usual_set_present(t)	do { } while(0)
#define usb_usual_clear_present(t)	do { } while(0)
#define usb_usual_check_type(id, t)	(0)
#endif 

#endif 
