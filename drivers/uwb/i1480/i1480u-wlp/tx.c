

#include "i1480u-wlp.h"

enum {
	
	i1480u_MAX_PL_SIZE = i1480u_MAX_FRG_SIZE
		- sizeof(struct untd_hdr_rst),
};


static
void i1480u_tx_free(struct i1480u_tx *wtx)
{
	kfree(wtx->buf);
	if (wtx->skb)
		dev_kfree_skb_irq(wtx->skb);
	usb_free_urb(wtx->urb);
	kfree(wtx);
}

static
void i1480u_tx_destroy(struct i1480u *i1480u, struct i1480u_tx *wtx)
{
	unsigned long flags;
	spin_lock_irqsave(&i1480u->tx_list_lock, flags);	
	list_del(&wtx->list_node);
	i1480u_tx_free(wtx);
	spin_unlock_irqrestore(&i1480u->tx_list_lock, flags);
}

static
void i1480u_tx_unlink_urbs(struct i1480u *i1480u)
{
	unsigned long flags;
	struct i1480u_tx *wtx, *next;

	spin_lock_irqsave(&i1480u->tx_list_lock, flags);
	list_for_each_entry_safe(wtx, next, &i1480u->tx_list, list_node) {
		usb_unlink_urb(wtx->urb);
	}
	spin_unlock_irqrestore(&i1480u->tx_list_lock, flags);
}



static
void i1480u_tx_cb(struct urb *urb)
{
	struct i1480u_tx *wtx = urb->context;
	struct i1480u *i1480u = wtx->i1480u;
	struct net_device *net_dev = i1480u->net_dev;
	struct device *dev = &i1480u->usb_iface->dev;
	unsigned long flags;

	switch (urb->status) {
	case 0:
		spin_lock_irqsave(&i1480u->lock, flags);
		net_dev->stats.tx_packets++;
		net_dev->stats.tx_bytes += urb->actual_length;
		spin_unlock_irqrestore(&i1480u->lock, flags);
		break;
	case -ECONNRESET:	
	case -ENOENT:		
		dev_dbg(dev, "notif endp: reset/noent %d\n", urb->status);
		netif_stop_queue(net_dev);
		break;
	case -ESHUTDOWN:	
		dev_dbg(dev, "notif endp: down %d\n", urb->status);
		netif_stop_queue(net_dev);
		break;
	default:
		dev_err(dev, "TX: unknown URB status %d\n", urb->status);
		if (edc_inc(&i1480u->tx_errors, EDC_MAX_ERRORS,
					EDC_ERROR_TIMEFRAME)) {
			dev_err(dev, "TX: max acceptable errors exceeded."
					"Reset device.\n");
			netif_stop_queue(net_dev);
			i1480u_tx_unlink_urbs(i1480u);
			wlp_reset_all(&i1480u->wlp);
		}
		break;
	}
	i1480u_tx_destroy(i1480u, wtx);
	if (atomic_dec_return(&i1480u->tx_inflight.count)
	    <= i1480u->tx_inflight.threshold
	    && netif_queue_stopped(net_dev)
	    && i1480u->tx_inflight.threshold != 0) {
		netif_start_queue(net_dev);
		atomic_inc(&i1480u->tx_inflight.restart_count);
	}
	return;
}



static
int i1480u_tx_create_n(struct i1480u_tx *wtx, struct sk_buff *skb,
		       gfp_t gfp_mask)
{
	int result;
	void *pl;
	size_t pl_size;

	void *pl_itr, *buf_itr;
	size_t pl_size_left, frgs, pl_size_1st, frg_pl_size = 0;
	struct untd_hdr_1st *untd_hdr_1st;
	struct wlp_tx_hdr *wlp_tx_hdr;
	struct untd_hdr_rst *untd_hdr_rst;

	wtx->skb = NULL;
	pl = skb->data;
	pl_itr = pl;
	pl_size = skb->len;
	pl_size_left = pl_size;	
	
	pl_size_1st = i1480u_MAX_FRG_SIZE
		- sizeof(struct untd_hdr_1st) - sizeof(struct wlp_tx_hdr);
	BUG_ON(pl_size_1st > pl_size);
	pl_size_left -= pl_size_1st;
	
	frgs = (pl_size_left + i1480u_MAX_PL_SIZE - 1) / i1480u_MAX_PL_SIZE;
	
	result = -ENOMEM;
	wtx->buf_size = sizeof(*untd_hdr_1st)
		+ sizeof(*wlp_tx_hdr)
		+ frgs * sizeof(*untd_hdr_rst)
		+ pl_size;
	wtx->buf = kmalloc(wtx->buf_size, gfp_mask);
	if (wtx->buf == NULL)
		goto error_buf_alloc;

	buf_itr = wtx->buf;		
	
	untd_hdr_1st = buf_itr;
	buf_itr += sizeof(*untd_hdr_1st);
	untd_hdr_set_type(&untd_hdr_1st->hdr, i1480u_PKT_FRAG_1ST);
	untd_hdr_set_rx_tx(&untd_hdr_1st->hdr, 0);
	untd_hdr_1st->hdr.len = cpu_to_le16(pl_size + sizeof(*wlp_tx_hdr));
	untd_hdr_1st->fragment_len =
		cpu_to_le16(pl_size_1st + sizeof(*wlp_tx_hdr));
	memset(untd_hdr_1st->padding, 0, sizeof(untd_hdr_1st->padding));
	
	wlp_tx_hdr = wtx->wlp_tx_hdr = buf_itr;
	buf_itr += sizeof(*wlp_tx_hdr);
	
	memcpy(buf_itr, pl_itr, pl_size_1st);
	pl_itr += pl_size_1st;
	buf_itr += pl_size_1st;

	
	result = -EINVAL;
	while (pl_size_left > 0) {
		if (buf_itr + sizeof(*untd_hdr_rst) - wtx->buf
		    > wtx->buf_size) {
			printk(KERN_ERR "BUG: no space for header\n");
			goto error_bug;
		}
		untd_hdr_rst = buf_itr;
		buf_itr += sizeof(*untd_hdr_rst);
		if (pl_size_left > i1480u_MAX_PL_SIZE) {
			frg_pl_size = i1480u_MAX_PL_SIZE;
			untd_hdr_set_type(&untd_hdr_rst->hdr, i1480u_PKT_FRAG_NXT);
		} else {
			frg_pl_size = pl_size_left;
			untd_hdr_set_type(&untd_hdr_rst->hdr, i1480u_PKT_FRAG_LST);
		}
		untd_hdr_set_rx_tx(&untd_hdr_rst->hdr, 0);
		untd_hdr_rst->hdr.len = cpu_to_le16(frg_pl_size);
		untd_hdr_rst->padding = 0;
		if (buf_itr + frg_pl_size - wtx->buf
		    > wtx->buf_size) {
			printk(KERN_ERR "BUG: no space for payload\n");
			goto error_bug;
		}
		memcpy(buf_itr, pl_itr, frg_pl_size);
		buf_itr += frg_pl_size;
		pl_itr += frg_pl_size;
		pl_size_left -= frg_pl_size;
	}
	dev_kfree_skb_irq(skb);
	return 0;

error_bug:
	printk(KERN_ERR
	       "BUG: skb %u bytes\n"
	       "BUG: frg_pl_size %zd i1480u_MAX_FRG_SIZE %u\n"
	       "BUG: buf_itr %zu buf_size %zu pl_size_left %zu\n",
	       skb->len,
	       frg_pl_size, i1480u_MAX_FRG_SIZE,
	       buf_itr - wtx->buf, wtx->buf_size, pl_size_left);

	kfree(wtx->buf);
error_buf_alloc:
	return result;
}



static
int i1480u_tx_create_1(struct i1480u_tx *wtx, struct sk_buff *skb,
		       gfp_t gfp_mask)
{
	struct untd_hdr_cmp *untd_hdr_cmp;
	struct wlp_tx_hdr *wlp_tx_hdr;

	wtx->buf = NULL;
	wtx->skb = skb;
	BUG_ON(skb_headroom(skb) < sizeof(*wlp_tx_hdr));
	wlp_tx_hdr = (void *) __skb_push(skb, sizeof(*wlp_tx_hdr));
	wtx->wlp_tx_hdr = wlp_tx_hdr;
	BUG_ON(skb_headroom(skb) < sizeof(*untd_hdr_cmp));
	untd_hdr_cmp = (void *) __skb_push(skb, sizeof(*untd_hdr_cmp));

	untd_hdr_set_type(&untd_hdr_cmp->hdr, i1480u_PKT_FRAG_CMP);
	untd_hdr_set_rx_tx(&untd_hdr_cmp->hdr, 0);
	untd_hdr_cmp->hdr.len = cpu_to_le16(skb->len - sizeof(*untd_hdr_cmp));
	untd_hdr_cmp->padding = 0;
	return 0;
}



static
struct i1480u_tx *i1480u_tx_create(struct i1480u *i1480u,
				   struct sk_buff *skb, gfp_t gfp_mask)
{
	int result;
	struct usb_endpoint_descriptor *epd;
	int usb_pipe;
	unsigned long flags;

	struct i1480u_tx *wtx;
	const size_t pl_max_size =
		i1480u_MAX_FRG_SIZE - sizeof(struct untd_hdr_cmp)
		- sizeof(struct wlp_tx_hdr);

	wtx = kmalloc(sizeof(*wtx), gfp_mask);
	if (wtx == NULL)
		goto error_wtx_alloc;
	wtx->urb = usb_alloc_urb(0, gfp_mask);
	if (wtx->urb == NULL)
		goto error_urb_alloc;
	epd = &i1480u->usb_iface->cur_altsetting->endpoint[2].desc;
	usb_pipe = usb_sndbulkpipe(i1480u->usb_dev, epd->bEndpointAddress);
	
	if (skb->len > pl_max_size) {
		result = i1480u_tx_create_n(wtx, skb, gfp_mask);
		if (result < 0)
			goto error_create;
		usb_fill_bulk_urb(wtx->urb, i1480u->usb_dev, usb_pipe,
				  wtx->buf, wtx->buf_size, i1480u_tx_cb, wtx);
	} else {
		result = i1480u_tx_create_1(wtx, skb, gfp_mask);
		if (result < 0)
			goto error_create;
		usb_fill_bulk_urb(wtx->urb, i1480u->usb_dev, usb_pipe,
				  skb->data, skb->len, i1480u_tx_cb, wtx);
	}
	spin_lock_irqsave(&i1480u->tx_list_lock, flags);
	list_add(&wtx->list_node, &i1480u->tx_list);
	spin_unlock_irqrestore(&i1480u->tx_list_lock, flags);
	return wtx;

error_create:
	kfree(wtx->urb);
error_urb_alloc:
	kfree(wtx);
error_wtx_alloc:
	return NULL;
}


int i1480u_xmit_frame(struct wlp *wlp, struct sk_buff *skb,
		      struct uwb_dev_addr *dst)
{
	int result = -ENXIO;
	struct i1480u *i1480u = container_of(wlp, struct i1480u, wlp);
	struct device *dev = &i1480u->usb_iface->dev;
	struct net_device *net_dev = i1480u->net_dev;
	struct i1480u_tx *wtx;
	struct wlp_tx_hdr *wlp_tx_hdr;
	static unsigned char dev_bcast[2] = { 0xff, 0xff };

	BUG_ON(i1480u->wlp.rc == NULL);
	if ((net_dev->flags & IFF_UP) == 0)
		goto out;
	result = -EBUSY;
	if (atomic_read(&i1480u->tx_inflight.count) >= i1480u->tx_inflight.max) {
		netif_stop_queue(net_dev);
		goto error_max_inflight;
	}
	result = -ENOMEM;
	wtx = i1480u_tx_create(i1480u, skb, GFP_ATOMIC);
	if (unlikely(wtx == NULL)) {
		if (printk_ratelimit())
			dev_err(dev, "TX: no memory for WLP TX URB,"
				"dropping packet (in flight %d)\n",
				atomic_read(&i1480u->tx_inflight.count));
		netif_stop_queue(net_dev);
		goto error_wtx_alloc;
	}
	wtx->i1480u = i1480u;
	
	wlp_tx_hdr = wtx->wlp_tx_hdr;
	*wlp_tx_hdr = i1480u->options.def_tx_hdr;
	wlp_tx_hdr->dstaddr = *dst;
	if (!memcmp(&wlp_tx_hdr->dstaddr, dev_bcast, sizeof(dev_bcast))
	    && (wlp_tx_hdr_delivery_id_type(wlp_tx_hdr) & WLP_DRP)) {
		
		wlp_tx_hdr_set_delivery_id_type(wlp_tx_hdr, i1480u->options.pca_base_priority);
	}

	result = usb_submit_urb(wtx->urb, GFP_ATOMIC);		
	if (result < 0) {
		dev_err(dev, "TX: cannot submit URB: %d\n", result);
		
		wtx->skb = NULL;
		goto error_tx_urb_submit;
	}
	atomic_inc(&i1480u->tx_inflight.count);
	net_dev->trans_start = jiffies;
	return result;

error_tx_urb_submit:
	i1480u_tx_destroy(i1480u, wtx);
error_wtx_alloc:
error_max_inflight:
out:
	return result;
}



netdev_tx_t i1480u_hard_start_xmit(struct sk_buff *skb,
					 struct net_device *net_dev)
{
	int result;
	struct i1480u *i1480u = netdev_priv(net_dev);
	struct device *dev = &i1480u->usb_iface->dev;
	struct uwb_dev_addr dst;

	if ((net_dev->flags & IFF_UP) == 0)
		goto error;
	result = wlp_prepare_tx_frame(dev, &i1480u->wlp, skb, &dst);
	if (result < 0) {
		dev_err(dev, "WLP verification of TX frame failed (%d). "
			"Dropping packet.\n", result);
		goto error;
	} else if (result == 1) {
		
		goto out;
	}
	result = i1480u_xmit_frame(&i1480u->wlp, skb, &dst);
	if (result < 0) {
		dev_err(dev, "Frame TX failed (%d).\n", result);
		goto error;
	}
	return NETDEV_TX_OK;
error:
	dev_kfree_skb_any(skb);
	net_dev->stats.tx_dropped++;
out:
	return NETDEV_TX_OK;
}



void i1480u_tx_timeout(struct net_device *net_dev)
{
	struct i1480u *i1480u = netdev_priv(net_dev);

	wlp_reset_all(&i1480u->wlp);
}


void i1480u_tx_release(struct i1480u *i1480u)
{
	unsigned long flags;
	struct i1480u_tx *wtx, *next;
	int count = 0, empty;

	spin_lock_irqsave(&i1480u->tx_list_lock, flags);
	list_for_each_entry_safe(wtx, next, &i1480u->tx_list, list_node) {
		count++;
		usb_unlink_urb(wtx->urb);
	}
	spin_unlock_irqrestore(&i1480u->tx_list_lock, flags);
	count = count*10; 
	
	while (1) {
		spin_lock_irqsave(&i1480u->tx_list_lock, flags);
		empty = list_empty(&i1480u->tx_list);
		spin_unlock_irqrestore(&i1480u->tx_list_lock, flags);
		if (empty)
			break;
		count--;
		BUG_ON(count == 0);
		msleep(20);
	}
}
