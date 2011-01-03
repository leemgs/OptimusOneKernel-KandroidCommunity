

#ifndef __MUSB_CORE_H__
#define __MUSB_CORE_H__

#include <linux/slab.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/timer.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb.h>
#include <linux/usb/otg.h>
#include <linux/usb/musb.h>

struct musb;
struct musb_hw_ep;
struct musb_ep;


#include "musb_debug.h"
#include "musb_dma.h"

#include "musb_io.h"
#include "musb_regs.h"

#include "musb_gadget.h"
#include "../core/hcd.h"
#include "musb_host.h"



#ifdef CONFIG_USB_MUSB_OTG

#define	is_peripheral_enabled(musb)	((musb)->board_mode != MUSB_HOST)
#define	is_host_enabled(musb)		((musb)->board_mode != MUSB_PERIPHERAL)
#define	is_otg_enabled(musb)		((musb)->board_mode == MUSB_OTG)


#define is_peripheral_active(m)		(!(m)->is_host)
#define is_host_active(m)		((m)->is_host)

#else
#define	is_peripheral_enabled(musb)	is_peripheral_capable()
#define	is_host_enabled(musb)		is_host_capable()
#define	is_otg_enabled(musb)		0

#define	is_peripheral_active(musb)	is_peripheral_capable()
#define	is_host_active(musb)		is_host_capable()
#endif

#if defined(CONFIG_USB_MUSB_OTG) || defined(CONFIG_USB_MUSB_PERIPHERAL)

#ifndef CONFIG_USB_GADGET_MUSB_HDRC
#error bogus Kconfig output ... select CONFIG_USB_GADGET_MUSB_HDRC
#endif
#endif	

#ifndef CONFIG_HAVE_CLK

#define clk_get(dev, id)	NULL
#define clk_put(clock)		do {} while (0)
#define clk_enable(clock)	do {} while (0)
#define clk_disable(clock)	do {} while (0)
#endif

#ifdef CONFIG_PROC_FS
#include <linux/fs.h>
#define MUSB_CONFIG_PROC_FS
#endif



#ifdef CONFIG_USB_GADGET_MUSB_HDRC

#define	is_peripheral_capable()	(1)

extern irqreturn_t musb_g_ep0_irq(struct musb *);
extern void musb_g_tx(struct musb *, u8);
extern void musb_g_rx(struct musb *, u8);
extern void musb_g_reset(struct musb *);
extern void musb_g_suspend(struct musb *);
extern void musb_g_resume(struct musb *);
extern void musb_g_wakeup(struct musb *);
extern void musb_g_disconnect(struct musb *);

#else

#define	is_peripheral_capable()	(0)

static inline irqreturn_t musb_g_ep0_irq(struct musb *m) { return IRQ_NONE; }
static inline void musb_g_reset(struct musb *m) {}
static inline void musb_g_suspend(struct musb *m) {}
static inline void musb_g_resume(struct musb *m) {}
static inline void musb_g_wakeup(struct musb *m) {}
static inline void musb_g_disconnect(struct musb *m) {}

#endif



#ifdef CONFIG_USB_MUSB_HDRC_HCD

#define	is_host_capable()	(1)

extern irqreturn_t musb_h_ep0_irq(struct musb *);
extern void musb_host_tx(struct musb *, u8);
extern void musb_host_rx(struct musb *, u8);

#else

#define	is_host_capable()	(0)

static inline irqreturn_t musb_h_ep0_irq(struct musb *m) { return IRQ_NONE; }
static inline void musb_host_tx(struct musb *m, u8 e) {}
static inline void musb_host_rx(struct musb *m, u8 e) {}

#endif




#ifndef MUSB_C_NUM_EPS
#define MUSB_C_NUM_EPS ((u8)16)
#endif

#ifndef MUSB_MAX_END0_PACKET
#define MUSB_MAX_END0_PACKET ((u16)MUSB_EP0_FIFOSIZE)
#endif


enum musb_h_ep0_state {
	MUSB_EP0_IDLE,
	MUSB_EP0_START,			
	MUSB_EP0_IN,			
	MUSB_EP0_OUT,			
	MUSB_EP0_STATUS,		
} __attribute__ ((packed));


enum musb_g_ep0_state {
	MUSB_EP0_STAGE_IDLE,		
	MUSB_EP0_STAGE_SETUP,		
	MUSB_EP0_STAGE_TX,		
	MUSB_EP0_STAGE_RX,		
	MUSB_EP0_STAGE_STATUSIN,	
	MUSB_EP0_STAGE_STATUSOUT,	
	MUSB_EP0_STAGE_ACKWAIT,		
} __attribute__ ((packed));


#define OTG_TIME_A_WAIT_VRISE	100		
#define OTG_TIME_A_WAIT_BCON	1100		
#define OTG_TIME_A_AIDL_BDIS	200		
#define OTG_TIME_B_ASE0_BRST	100		






#if defined(CONFIG_ARCH_DAVINCI) || defined(CONFIG_ARCH_OMAP2430) \
		|| defined(CONFIG_ARCH_OMAP3430) || defined(CONFIG_BLACKFIN)

#define	MUSB_FLAT_REG
#endif


#if	defined(CONFIG_USB_TUSB6010)
#define musb_ep_select(_mbase, _epnum) \
	musb_writeb((_mbase), MUSB_INDEX, (_epnum))
#define	MUSB_EP_OFFSET			MUSB_TUSB_OFFSET


#elif	defined(MUSB_FLAT_REG)
#define musb_ep_select(_mbase, _epnum)	(((void)(_mbase)), ((void)(_epnum)))
#define	MUSB_EP_OFFSET			MUSB_FLAT_OFFSET


#else
#define musb_ep_select(_mbase, _epnum) \
	musb_writeb((_mbase), MUSB_INDEX, (_epnum))
#define	MUSB_EP_OFFSET			MUSB_INDEXED_OFFSET
#endif



#define MUSB_HST_MODE(_musb)\
	{ (_musb)->is_host = true; }
#define MUSB_DEV_MODE(_musb) \
	{ (_musb)->is_host = false; }

#define test_devctl_hst_mode(_x) \
	(musb_readb((_x)->mregs, MUSB_DEVCTL)&MUSB_DEVCTL_HM)

#define MUSB_MODE(musb) ((musb)->is_host ? "Host" : "Peripheral")




struct musb_hw_ep {
	struct musb		*musb;
	void __iomem		*fifo;
	void __iomem		*regs;

#ifdef CONFIG_USB_TUSB6010
	void __iomem		*conf;
#endif

	
	u8			epnum;

	
	bool			is_shared_fifo;
	bool			tx_double_buffered;
	bool			rx_double_buffered;
	u16			max_packet_sz_tx;
	u16			max_packet_sz_rx;

	struct dma_channel	*tx_channel;
	struct dma_channel	*rx_channel;

#ifdef CONFIG_USB_TUSB6010
	
	dma_addr_t		fifo_async;
	dma_addr_t		fifo_sync;
	void __iomem		*fifo_sync_va;
#endif

#ifdef CONFIG_USB_MUSB_HDRC_HCD
	void __iomem		*target_regs;

	
	struct musb_qh		*in_qh;
	struct musb_qh		*out_qh;

	u8			rx_reinit;
	u8			tx_reinit;
#endif

#ifdef CONFIG_USB_GADGET_MUSB_HDRC
	
	struct musb_ep		ep_in;			
	struct musb_ep		ep_out;			
#endif
};

static inline struct usb_request *next_in_request(struct musb_hw_ep *hw_ep)
{
#ifdef CONFIG_USB_GADGET_MUSB_HDRC
	return next_request(&hw_ep->ep_in);
#else
	return NULL;
#endif
}

static inline struct usb_request *next_out_request(struct musb_hw_ep *hw_ep)
{
#ifdef CONFIG_USB_GADGET_MUSB_HDRC
	return next_request(&hw_ep->ep_out);
#else
	return NULL;
#endif
}


struct musb {
	
	spinlock_t		lock;
	struct clk		*clock;
	irqreturn_t		(*isr)(int, void *);
	struct work_struct	irq_work;


#define MUSB_PORT_STAT_RESUME	(1 << 31)

	u32			port1_status;

#ifdef CONFIG_USB_MUSB_HDRC_HCD
	unsigned long		rh_timer;

	enum musb_h_ep0_state	ep0_stage;

	
	struct musb_hw_ep	*bulk_ep;

	struct list_head	control;	
	struct list_head	in_bulk;	
	struct list_head	out_bulk;	

	struct timer_list	otg_timer;
#endif

	
	void			(*board_set_vbus)(struct musb *, int is_on);

	struct dma_controller	*dma_controller;

	struct device		*controller;
	void __iomem		*ctrl_base;
	void __iomem		*mregs;

#ifdef CONFIG_USB_TUSB6010
	dma_addr_t		async;
	dma_addr_t		sync;
	void __iomem		*sync_va;
#endif

	
	u8			int_usb;
	u16			int_rx;
	u16			int_tx;

	struct otg_transceiver	*xceiv;

	int nIrq;
	unsigned		irq_wake:1;

	struct musb_hw_ep	 endpoints[MUSB_C_NUM_EPS];
#define control_ep		endpoints

#define VBUSERR_RETRY_COUNT	3
	u16			vbuserr_retry;
	u16 epmask;
	u8 nr_endpoints;

	u8 board_mode;		
	int			(*board_set_power)(int state);

	int			(*set_clock)(struct clk *clk, int is_active);

	u8			min_power;	

	bool			is_host;

	int			a_wait_bcon;	
	unsigned long		idle_timeout;	

	
	unsigned		is_active:1;

	unsigned is_multipoint:1;
	unsigned ignore_disconnect:1;	

	unsigned		hb_iso_rx:1;	
	unsigned		hb_iso_tx:1;	

#ifdef C_MP_TX
	unsigned bulk_split:1;
#define	can_bulk_split(musb,type) \
		(((type) == USB_ENDPOINT_XFER_BULK) && (musb)->bulk_split)
#else
#define	can_bulk_split(musb, type)	0
#endif

#ifdef C_MP_RX
	unsigned bulk_combine:1;
#define	can_bulk_combine(musb,type) \
		(((type) == USB_ENDPOINT_XFER_BULK) && (musb)->bulk_combine)
#else
#define	can_bulk_combine(musb, type)	0
#endif

#ifdef CONFIG_USB_GADGET_MUSB_HDRC
	
	unsigned		is_suspended:1;

	
	unsigned		may_wakeup:1;

	
	unsigned		is_self_powered:1;
	unsigned		is_bus_powered:1;

	unsigned		set_address:1;
	unsigned		test_mode:1;
	unsigned		softconnect:1;

	u8			address;
	u8			test_mode_nr;
	u16			ackpend;		
	enum musb_g_ep0_state	ep0_state;
	struct usb_gadget	g;			
	struct usb_gadget_driver *gadget_driver;	
#endif

	struct musb_hdrc_config	*config;

#ifdef MUSB_CONFIG_PROC_FS
	struct proc_dir_entry *proc_entry;
#endif
};

static inline void musb_set_vbus(struct musb *musb, int is_on)
{
	musb->board_set_vbus(musb, is_on);
}

#ifdef CONFIG_USB_GADGET_MUSB_HDRC
static inline struct musb *gadget_to_musb(struct usb_gadget *g)
{
	return container_of(g, struct musb, g);
}
#endif

#ifdef CONFIG_BLACKFIN
static inline int musb_read_fifosize(struct musb *musb,
		struct musb_hw_ep *hw_ep, u8 epnum)
{
	musb->nr_endpoints++;
	musb->epmask |= (1 << epnum);

	if (epnum < 5) {
		hw_ep->max_packet_sz_tx = 128;
		hw_ep->max_packet_sz_rx = 128;
	} else {
		hw_ep->max_packet_sz_tx = 1024;
		hw_ep->max_packet_sz_rx = 1024;
	}
	hw_ep->is_shared_fifo = false;

	return 0;
}

static inline void musb_configure_ep0(struct musb *musb)
{
	musb->endpoints[0].max_packet_sz_tx = MUSB_EP0_FIFOSIZE;
	musb->endpoints[0].max_packet_sz_rx = MUSB_EP0_FIFOSIZE;
	musb->endpoints[0].is_shared_fifo = true;
}

#else

static inline int musb_read_fifosize(struct musb *musb,
		struct musb_hw_ep *hw_ep, u8 epnum)
{
	void *mbase = musb->mregs;
	u8 reg = 0;

	
	reg = musb_readb(mbase, MUSB_EP_OFFSET(epnum, MUSB_FIFOSIZE));
	
	if (!reg)
		return -ENODEV;

	musb->nr_endpoints++;
	musb->epmask |= (1 << epnum);

	hw_ep->max_packet_sz_tx = 1 << (reg & 0x0f);

	
	if ((reg & 0xf0) == 0xf0) {
		hw_ep->max_packet_sz_rx = hw_ep->max_packet_sz_tx;
		hw_ep->is_shared_fifo = true;
		return 0;
	} else {
		hw_ep->max_packet_sz_rx = 1 << ((reg & 0xf0) >> 4);
		hw_ep->is_shared_fifo = false;
	}

	return 0;
}

static inline void musb_configure_ep0(struct musb *musb)
{
	musb->endpoints[0].max_packet_sz_tx = MUSB_EP0_FIFOSIZE;
	musb->endpoints[0].max_packet_sz_rx = MUSB_EP0_FIFOSIZE;
	musb->endpoints[0].is_shared_fifo = true;
}
#endif 




extern const char musb_driver_name[];

extern void musb_start(struct musb *musb);
extern void musb_stop(struct musb *musb);

extern void musb_write_fifo(struct musb_hw_ep *ep, u16 len, const u8 *src);
extern void musb_read_fifo(struct musb_hw_ep *ep, u16 len, u8 *dst);

extern void musb_load_testpacket(struct musb *);

extern irqreturn_t musb_interrupt(struct musb *);

extern void musb_platform_enable(struct musb *musb);
extern void musb_platform_disable(struct musb *musb);

extern void musb_hnp_stop(struct musb *musb);

extern int musb_platform_set_mode(struct musb *musb, u8 musb_mode);

#if defined(CONFIG_USB_TUSB6010) || defined(CONFIG_BLACKFIN) || \
	defined(CONFIG_ARCH_OMAP2430) || defined(CONFIG_ARCH_OMAP34XX)
extern void musb_platform_try_idle(struct musb *musb, unsigned long timeout);
#else
#define musb_platform_try_idle(x, y)		do {} while (0)
#endif

#if defined(CONFIG_USB_TUSB6010) || defined(CONFIG_BLACKFIN)
extern int musb_platform_get_vbus_status(struct musb *musb);
#else
#define musb_platform_get_vbus_status(x)	0
#endif

extern int __init musb_platform_init(struct musb *musb);
extern int musb_platform_exit(struct musb *musb);

#endif	
