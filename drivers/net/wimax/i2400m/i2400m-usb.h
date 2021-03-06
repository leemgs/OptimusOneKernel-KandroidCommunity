

#ifndef __I2400M_USB_H__
#define __I2400M_USB_H__

#include "i2400m.h"
#include <linux/kthread.h>



enum {
	EDC_MAX_ERRORS = 10,
	EDC_ERROR_TIMEFRAME = HZ,
};


struct edc {
	unsigned long timestart;
	u16 errorcount;
};

static inline void edc_init(struct edc *edc)
{
	edc->timestart = jiffies;
}


static inline int edc_inc(struct edc *edc, u16 max_err, u16 timeframe)
{
	unsigned long now;

	now = jiffies;
	if (now - edc->timestart > timeframe) {
		edc->errorcount = 1;
		edc->timestart = now;
	} else if (++edc->errorcount > max_err) {
		edc->errorcount = 0;
		edc->timestart = now;
		return 1;
	}
	return 0;
}


enum {
	I2400MU_MAX_NOTIFICATION_LEN = 256,
	I2400MU_BLK_SIZE = 16,
	I2400MU_PL_SIZE_MAX = 0x3EFF,

	
	I2400MU_EP_BULK_OUT = 0,
	I2400MU_EP_NOTIFICATION,
	I2400MU_EP_RESET_COLD,
	I2400MU_EP_BULK_IN,
};



struct i2400mu {
	struct i2400m i2400m;		

	struct usb_device *usb_dev;
	struct usb_interface *usb_iface;
	struct edc urb_edc;		

	struct urb *notif_urb;
	struct task_struct *tx_kthread;
	wait_queue_head_t tx_wq;

	struct task_struct *rx_kthread;
	wait_queue_head_t rx_wq;
	atomic_t rx_pending_count;
	size_t rx_size, rx_size_acc, rx_size_cnt;
	atomic_t do_autopm;
	u8 rx_size_auto_shrink;

	struct dentry *debugfs_dentry;
};


static inline
void i2400mu_init(struct i2400mu *i2400mu)
{
	i2400m_init(&i2400mu->i2400m);
	edc_init(&i2400mu->urb_edc);
	init_waitqueue_head(&i2400mu->tx_wq);
	atomic_set(&i2400mu->rx_pending_count, 0);
	init_waitqueue_head(&i2400mu->rx_wq);
	i2400mu->rx_size = PAGE_SIZE - sizeof(struct skb_shared_info);
	atomic_set(&i2400mu->do_autopm, 1);
	i2400mu->rx_size_auto_shrink = 1;
}

extern int i2400mu_notification_setup(struct i2400mu *);
extern void i2400mu_notification_release(struct i2400mu *);

extern int i2400mu_rx_setup(struct i2400mu *);
extern void i2400mu_rx_release(struct i2400mu *);
extern void i2400mu_rx_kick(struct i2400mu *);

extern int i2400mu_tx_setup(struct i2400mu *);
extern void i2400mu_tx_release(struct i2400mu *);
extern void i2400mu_bus_tx_kick(struct i2400m *);

extern ssize_t i2400mu_bus_bm_cmd_send(struct i2400m *,
				       const struct i2400m_bootrom_header *,
				       size_t, int);
extern ssize_t i2400mu_bus_bm_wait_for_ack(struct i2400m *,
					   struct i2400m_bootrom_header *,
					   size_t);
#endif 
