

#ifndef _CI13XXX_h_
#define _CI13XXX_h_


#define ENDPT_MAX          (16)
#define CTRL_PAYLOAD_MAX   (64)
#define RX        (0)  
#define TX        (1)  



struct ci13xxx_td {
	
	u32 next;
#define TD_TERMINATE          BIT(0)
	
	u32 token;
#define TD_STATUS             (0x00FFUL <<  0)
#define TD_STATUS_TR_ERR      BIT(3)
#define TD_STATUS_DT_ERR      BIT(5)
#define TD_STATUS_HALTED      BIT(6)
#define TD_STATUS_ACTIVE      BIT(7)
#define TD_MULTO              (0x0003UL << 10)
#define TD_IOC                BIT(15)
#define TD_TOTAL_BYTES        (0x7FFFUL << 16)
	
	u32 page[5];
#define TD_CURR_OFFSET        (0x0FFFUL <<  0)
#define TD_FRAME_NUM          (0x07FFUL <<  0)
#define TD_RESERVED_MASK      (0x0FFFUL <<  0)
} __attribute__ ((packed));


struct ci13xxx_qh {
	
	u32 cap;
#define QH_IOS                BIT(15)
#define QH_MAX_PKT            (0x07FFUL << 16)
#define QH_ZLT                BIT(29)
#define QH_MULT               (0x0003UL << 30)
	
	u32 curr;
	
	struct ci13xxx_td        td;
	
	u32 RESERVED;
	struct usb_ctrlrequest   setup;
} __attribute__ ((packed));


struct ci13xxx_req {
	struct usb_request   req;
	unsigned             map;
	struct list_head     queue;
	struct ci13xxx_td   *ptr;
	dma_addr_t           dma;
};


struct ci13xxx_ep {
	struct usb_ep                          ep;
	const struct usb_endpoint_descriptor  *desc;
	u8                                     dir;
	u8                                     num;
	u8                                     type;
	char                                   name[16];
	struct {
		struct list_head   queue;
		struct ci13xxx_qh *ptr;
		dma_addr_t         dma;
	}                                      qh[2];
	struct usb_request                    *status;
	int                                    wedge;

	
	spinlock_t                            *lock;
	struct device                         *device;
	struct dma_pool                       *td_pool;
};


struct ci13xxx {
	spinlock_t		  *lock;      

	struct dma_pool           *qh_pool;   
	struct dma_pool           *td_pool;   

	struct usb_gadget          gadget;     
	struct ci13xxx_ep          ci13xxx_ep[ENDPT_MAX]; 

	struct usb_gadget_driver  *driver;     
};



#define REG_BITS   (32)


#define HCCPARAMS_LEN         BIT(17)


#define DCCPARAMS_DEN         (0x1F << 0)
#define DCCPARAMS_DC          BIT(7)


#define TESTMODE_FORCE        BIT(0)


#define USBCMD_RS             BIT(0)
#define USBCMD_RST            BIT(1)
#define USBCMD_SUTW           BIT(13)


#define USBi_UI               BIT(0)
#define USBi_UEI              BIT(1)
#define USBi_PCI              BIT(2)
#define USBi_URI              BIT(6)
#define USBi_SLI              BIT(8)


#define DEVICEADDR_USBADRA    BIT(24)
#define DEVICEADDR_USBADR     (0x7FUL << 25)


#define PORTSC_SUSP           BIT(7)
#define PORTSC_HSP            BIT(9)
#define PORTSC_PTC            (0x0FUL << 16)


#define DEVLC_PSPD            (0x03UL << 25)
#define    DEVLC_PSPD_HS      (0x02UL << 25)


#define USBMODE_CM            (0x03UL <<  0)
#define    USBMODE_CM_IDLE    (0x00UL <<  0)
#define    USBMODE_CM_DEVICE  (0x02UL <<  0)
#define    USBMODE_CM_HOST    (0x03UL <<  0)
#define USBMODE_SLOM          BIT(3)


#define ENDPTCTRL_RXS         BIT(0)
#define ENDPTCTRL_RXT         (0x03UL <<  2)
#define ENDPTCTRL_RXR         BIT(6)         
#define ENDPTCTRL_RXE         BIT(7)
#define ENDPTCTRL_TXS         BIT(16)
#define ENDPTCTRL_TXT         (0x03UL << 18)
#define ENDPTCTRL_TXR         BIT(22)        
#define ENDPTCTRL_TXE         BIT(23)


#define ci13xxx_printk(level, format, args...) \
do { \
	if (_udc == NULL) \
		printk(level "[%s] " format "\n", __func__, ## args); \
	else \
		dev_printk(level, _udc->gadget.dev.parent, \
			   "[%s] " format "\n", __func__, ## args); \
} while (0)

#define err(format, args...)    ci13xxx_printk(KERN_ERR, format, ## args)
#define warn(format, args...)   ci13xxx_printk(KERN_WARNING, format, ## args)
#define info(format, args...)   ci13xxx_printk(KERN_INFO, format, ## args)

#ifdef TRACE
#define trace(format, args...)      ci13xxx_printk(KERN_DEBUG, format, ## args)
#define dbg_trace(format, args...)  dev_dbg(dev, format, ##args)
#else
#define trace(format, args...)      do {} while (0)
#define dbg_trace(format, args...)  do {} while (0)
#endif

#endif	
