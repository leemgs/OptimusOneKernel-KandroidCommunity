

#ifndef __i1480u_wlp_h__
#define __i1480u_wlp_h__

#include <linux/usb.h>
#include <linux/netdevice.h>
#include <linux/uwb.h>		
#include <linux/wlp.h>
#include "../i1480-wlp.h"

#undef i1480u_FLOW_CONTROL	


enum {
	i1480u_TX_INFLIGHT_MAX = 1000,
	i1480u_TX_INFLIGHT_THRESHOLD = 100,
};


enum {
	
	i1480u_MAX_RX_PKT_SIZE = 4114,
	i1480u_MAX_FRG_SIZE = 512,
	i1480u_RX_BUFS = 9,
};



enum i1480u_pkt_type {
	i1480u_PKT_FRAG_1ST = 0x1,
	i1480u_PKT_FRAG_NXT = 0x0,
	i1480u_PKT_FRAG_LST = 0x2,
	i1480u_PKT_FRAG_CMP = 0x3
};
enum {
	i1480u_PKT_NONE = 0x4,
};


struct untd_hdr {
	u8     type;
	__le16 len;
} __attribute__((packed));

static inline enum i1480u_pkt_type untd_hdr_type(const struct untd_hdr *hdr)
{
	return hdr->type & 0x03;
}

static inline int untd_hdr_rx_tx(const struct untd_hdr *hdr)
{
	return (hdr->type >> 2) & 0x01;
}

static inline void untd_hdr_set_type(struct untd_hdr *hdr, enum i1480u_pkt_type type)
{
	hdr->type = (hdr->type & ~0x03) | type;
}

static inline void untd_hdr_set_rx_tx(struct untd_hdr *hdr, int rx_tx)
{
	hdr->type = (hdr->type & ~0x04) | (rx_tx << 2);
}



struct untd_hdr_cmp {
	struct untd_hdr	hdr;
	u8		padding;
} __attribute__((packed));



struct untd_hdr_1st {
	struct untd_hdr	hdr;
	__le16		fragment_len;
	u8		padding[3];
} __attribute__((packed));



struct untd_hdr_rst {
	struct untd_hdr	hdr;
	u8		padding;
} __attribute__((packed));



struct i1480u_tx {
	struct list_head list_node;
	struct i1480u *i1480u;
	struct urb *urb;

	struct sk_buff *skb;
	struct wlp_tx_hdr *wlp_tx_hdr;

	void *buf;	
	size_t buf_size;
};


struct i1480u_tx_inflight {
	atomic_t count;
	unsigned long max;
	unsigned long threshold;
	unsigned long restart_ts;
	atomic_t restart_count;
};


struct i1480u {
	struct usb_device *usb_dev;
	struct usb_interface *usb_iface;
	struct net_device *net_dev;

	spinlock_t lock;

	
	struct sk_buff *rx_skb;
	struct uwb_dev_addr rx_srcaddr;
	size_t rx_untd_pkt_size;
	struct i1480u_rx_buf {
		struct i1480u *i1480u;	
		struct urb *urb;
		struct sk_buff *data;	
	} rx_buf[i1480u_RX_BUFS];	

	spinlock_t tx_list_lock;	
	struct list_head tx_list;
	u8 tx_stream;

	struct stats lqe_stats, rssi_stats;	

	
	struct wlp_options options;
	struct uwb_notifs_handler uwb_notifs_handler;
	struct edc tx_errors;
	struct edc rx_errors;
	struct wlp wlp;
#ifdef i1480u_FLOW_CONTROL
	struct urb *notif_urb;
	struct edc notif_edc;		
	u8 notif_buffer[1];
#endif
	struct i1480u_tx_inflight tx_inflight;
};


extern void i1480u_rx_cb(struct urb *urb);
extern int i1480u_rx_setup(struct i1480u *);
extern void i1480u_rx_release(struct i1480u *);
extern void i1480u_tx_release(struct i1480u *);
extern int i1480u_xmit_frame(struct wlp *, struct sk_buff *,
			     struct uwb_dev_addr *);
extern void i1480u_stop_queue(struct wlp *);
extern void i1480u_start_queue(struct wlp *);
extern int i1480u_sysfs_setup(struct i1480u *);
extern void i1480u_sysfs_release(struct i1480u *);


extern int i1480u_open(struct net_device *);
extern int i1480u_stop(struct net_device *);
extern netdev_tx_t i1480u_hard_start_xmit(struct sk_buff *,
						struct net_device *);
extern void i1480u_tx_timeout(struct net_device *);
extern int i1480u_set_config(struct net_device *, struct ifmap *);
extern int i1480u_change_mtu(struct net_device *, int);
extern void i1480u_uwb_notifs_cb(void *, struct uwb_dev *, enum uwb_notifs);


extern void  i1480u_bw_alloc_cb(struct uwb_rsv *);


extern struct attribute_group i1480u_wlp_attr_group;

#endif 
