

#include "b43.h"
#include "pio.h"
#include "dma.h"
#include "main.h"
#include "xmit.h"

#include <linux/delay.h>
#include <linux/sched.h>


static u16 generate_cookie(struct b43_pio_txqueue *q,
			   struct b43_pio_txpacket *pack)
{
	u16 cookie;

	
	cookie = (((u16)q->index + 1) << 12);
	cookie |= pack->index;

	return cookie;
}

static
struct b43_pio_txqueue *parse_cookie(struct b43_wldev *dev,
				     u16 cookie,
				      struct b43_pio_txpacket **pack)
{
	struct b43_pio *pio = &dev->pio;
	struct b43_pio_txqueue *q = NULL;
	unsigned int pack_index;

	switch (cookie & 0xF000) {
	case 0x1000:
		q = pio->tx_queue_AC_BK;
		break;
	case 0x2000:
		q = pio->tx_queue_AC_BE;
		break;
	case 0x3000:
		q = pio->tx_queue_AC_VI;
		break;
	case 0x4000:
		q = pio->tx_queue_AC_VO;
		break;
	case 0x5000:
		q = pio->tx_queue_mcast;
		break;
	}
	if (B43_WARN_ON(!q))
		return NULL;
	pack_index = (cookie & 0x0FFF);
	if (B43_WARN_ON(pack_index >= ARRAY_SIZE(q->packets)))
		return NULL;
	*pack = &q->packets[pack_index];

	return q;
}

static u16 index_to_pioqueue_base(struct b43_wldev *dev,
				  unsigned int index)
{
	static const u16 bases[] = {
		B43_MMIO_PIO_BASE0,
		B43_MMIO_PIO_BASE1,
		B43_MMIO_PIO_BASE2,
		B43_MMIO_PIO_BASE3,
		B43_MMIO_PIO_BASE4,
		B43_MMIO_PIO_BASE5,
		B43_MMIO_PIO_BASE6,
		B43_MMIO_PIO_BASE7,
	};
	static const u16 bases_rev11[] = {
		B43_MMIO_PIO11_BASE0,
		B43_MMIO_PIO11_BASE1,
		B43_MMIO_PIO11_BASE2,
		B43_MMIO_PIO11_BASE3,
		B43_MMIO_PIO11_BASE4,
		B43_MMIO_PIO11_BASE5,
	};

	if (dev->dev->id.revision >= 11) {
		B43_WARN_ON(index >= ARRAY_SIZE(bases_rev11));
		return bases_rev11[index];
	}
	B43_WARN_ON(index >= ARRAY_SIZE(bases));
	return bases[index];
}

static u16 pio_txqueue_offset(struct b43_wldev *dev)
{
	if (dev->dev->id.revision >= 11)
		return 0x18;
	return 0;
}

static u16 pio_rxqueue_offset(struct b43_wldev *dev)
{
	if (dev->dev->id.revision >= 11)
		return 0x38;
	return 8;
}

static struct b43_pio_txqueue *b43_setup_pioqueue_tx(struct b43_wldev *dev,
						     unsigned int index)
{
	struct b43_pio_txqueue *q;
	struct b43_pio_txpacket *p;
	unsigned int i;

	q = kzalloc(sizeof(*q), GFP_KERNEL);
	if (!q)
		return NULL;
	q->dev = dev;
	q->rev = dev->dev->id.revision;
	q->mmio_base = index_to_pioqueue_base(dev, index) +
		       pio_txqueue_offset(dev);
	q->index = index;

	q->free_packet_slots = B43_PIO_MAX_NR_TXPACKETS;
	if (q->rev >= 8) {
		q->buffer_size = 1920; 
	} else {
		q->buffer_size = b43_piotx_read16(q, B43_PIO_TXQBUFSIZE);
		q->buffer_size -= 80;
	}

	INIT_LIST_HEAD(&q->packets_list);
	for (i = 0; i < ARRAY_SIZE(q->packets); i++) {
		p = &(q->packets[i]);
		INIT_LIST_HEAD(&p->list);
		p->index = i;
		p->queue = q;
		list_add(&p->list, &q->packets_list);
	}

	return q;
}

static struct b43_pio_rxqueue *b43_setup_pioqueue_rx(struct b43_wldev *dev,
						     unsigned int index)
{
	struct b43_pio_rxqueue *q;

	q = kzalloc(sizeof(*q), GFP_KERNEL);
	if (!q)
		return NULL;
	q->dev = dev;
	q->rev = dev->dev->id.revision;
	q->mmio_base = index_to_pioqueue_base(dev, index) +
		       pio_rxqueue_offset(dev);

	
	b43_dma_direct_fifo_rx(dev, index, 1);

	return q;
}

static void b43_pio_cancel_tx_packets(struct b43_pio_txqueue *q)
{
	struct b43_pio_txpacket *pack;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(q->packets); i++) {
		pack = &(q->packets[i]);
		if (pack->skb) {
			dev_kfree_skb_any(pack->skb);
			pack->skb = NULL;
		}
	}
}

static void b43_destroy_pioqueue_tx(struct b43_pio_txqueue *q,
				    const char *name)
{
	if (!q)
		return;
	b43_pio_cancel_tx_packets(q);
	kfree(q);
}

static void b43_destroy_pioqueue_rx(struct b43_pio_rxqueue *q,
				    const char *name)
{
	if (!q)
		return;
	kfree(q);
}

#define destroy_queue_tx(pio, queue) do {				\
	b43_destroy_pioqueue_tx((pio)->queue, __stringify(queue));	\
	(pio)->queue = NULL;						\
  } while (0)

#define destroy_queue_rx(pio, queue) do {				\
	b43_destroy_pioqueue_rx((pio)->queue, __stringify(queue));	\
	(pio)->queue = NULL;						\
  } while (0)

void b43_pio_free(struct b43_wldev *dev)
{
	struct b43_pio *pio;

	if (!b43_using_pio_transfers(dev))
		return;
	pio = &dev->pio;

	destroy_queue_rx(pio, rx_queue);
	destroy_queue_tx(pio, tx_queue_mcast);
	destroy_queue_tx(pio, tx_queue_AC_VO);
	destroy_queue_tx(pio, tx_queue_AC_VI);
	destroy_queue_tx(pio, tx_queue_AC_BE);
	destroy_queue_tx(pio, tx_queue_AC_BK);
}

int b43_pio_init(struct b43_wldev *dev)
{
	struct b43_pio *pio = &dev->pio;
	int err = -ENOMEM;

	b43_write32(dev, B43_MMIO_MACCTL, b43_read32(dev, B43_MMIO_MACCTL)
		    & ~B43_MACCTL_BE);
	b43_shm_write16(dev, B43_SHM_SHARED, B43_SHM_SH_RXPADOFF, 0);

	pio->tx_queue_AC_BK = b43_setup_pioqueue_tx(dev, 0);
	if (!pio->tx_queue_AC_BK)
		goto out;

	pio->tx_queue_AC_BE = b43_setup_pioqueue_tx(dev, 1);
	if (!pio->tx_queue_AC_BE)
		goto err_destroy_bk;

	pio->tx_queue_AC_VI = b43_setup_pioqueue_tx(dev, 2);
	if (!pio->tx_queue_AC_VI)
		goto err_destroy_be;

	pio->tx_queue_AC_VO = b43_setup_pioqueue_tx(dev, 3);
	if (!pio->tx_queue_AC_VO)
		goto err_destroy_vi;

	pio->tx_queue_mcast = b43_setup_pioqueue_tx(dev, 4);
	if (!pio->tx_queue_mcast)
		goto err_destroy_vo;

	pio->rx_queue = b43_setup_pioqueue_rx(dev, 0);
	if (!pio->rx_queue)
		goto err_destroy_mcast;

	b43dbg(dev->wl, "PIO initialized\n");
	err = 0;
out:
	return err;

err_destroy_mcast:
	destroy_queue_tx(pio, tx_queue_mcast);
err_destroy_vo:
	destroy_queue_tx(pio, tx_queue_AC_VO);
err_destroy_vi:
	destroy_queue_tx(pio, tx_queue_AC_VI);
err_destroy_be:
	destroy_queue_tx(pio, tx_queue_AC_BE);
err_destroy_bk:
	destroy_queue_tx(pio, tx_queue_AC_BK);
	return err;
}


static struct b43_pio_txqueue *select_queue_by_priority(struct b43_wldev *dev,
							u8 queue_prio)
{
	struct b43_pio_txqueue *q;

	if (dev->qos_enabled) {
		
		switch (queue_prio) {
		default:
			B43_WARN_ON(1);
			
		case 0:
			q = dev->pio.tx_queue_AC_VO;
			break;
		case 1:
			q = dev->pio.tx_queue_AC_VI;
			break;
		case 2:
			q = dev->pio.tx_queue_AC_BE;
			break;
		case 3:
			q = dev->pio.tx_queue_AC_BK;
			break;
		}
	} else
		q = dev->pio.tx_queue_AC_BE;

	return q;
}

static u16 tx_write_2byte_queue(struct b43_pio_txqueue *q,
				u16 ctl,
				const void *_data,
				unsigned int data_len)
{
	struct b43_wldev *dev = q->dev;
	struct b43_wl *wl = dev->wl;
	const u8 *data = _data;

	ctl |= B43_PIO_TXCTL_WRITELO | B43_PIO_TXCTL_WRITEHI;
	b43_piotx_write16(q, B43_PIO_TXCTL, ctl);

	ssb_block_write(dev->dev, data, (data_len & ~1),
			q->mmio_base + B43_PIO_TXDATA,
			sizeof(u16));
	if (data_len & 1) {
		
		ctl &= ~B43_PIO_TXCTL_WRITEHI;
		b43_piotx_write16(q, B43_PIO_TXCTL, ctl);
		wl->tx_tail[0] = data[data_len - 1];
		wl->tx_tail[1] = 0;
		ssb_block_write(dev->dev, wl->tx_tail, 2,
				q->mmio_base + B43_PIO_TXDATA,
				sizeof(u16));
	}

	return ctl;
}

static void pio_tx_frame_2byte_queue(struct b43_pio_txpacket *pack,
				     const u8 *hdr, unsigned int hdrlen)
{
	struct b43_pio_txqueue *q = pack->queue;
	const char *frame = pack->skb->data;
	unsigned int frame_len = pack->skb->len;
	u16 ctl;

	ctl = b43_piotx_read16(q, B43_PIO_TXCTL);
	ctl |= B43_PIO_TXCTL_FREADY;
	ctl &= ~B43_PIO_TXCTL_EOF;

	
	ctl = tx_write_2byte_queue(q, ctl, hdr, hdrlen);
	
	ctl = tx_write_2byte_queue(q, ctl, frame, frame_len);

	ctl |= B43_PIO_TXCTL_EOF;
	b43_piotx_write16(q, B43_PIO_TXCTL, ctl);
}

static u32 tx_write_4byte_queue(struct b43_pio_txqueue *q,
				u32 ctl,
				const void *_data,
				unsigned int data_len)
{
	struct b43_wldev *dev = q->dev;
	struct b43_wl *wl = dev->wl;
	const u8 *data = _data;

	ctl |= B43_PIO8_TXCTL_0_7 | B43_PIO8_TXCTL_8_15 |
	       B43_PIO8_TXCTL_16_23 | B43_PIO8_TXCTL_24_31;
	b43_piotx_write32(q, B43_PIO8_TXCTL, ctl);

	ssb_block_write(dev->dev, data, (data_len & ~3),
			q->mmio_base + B43_PIO8_TXDATA,
			sizeof(u32));
	if (data_len & 3) {
		wl->tx_tail[3] = 0;
		
		ctl &= ~(B43_PIO8_TXCTL_8_15 | B43_PIO8_TXCTL_16_23 |
			 B43_PIO8_TXCTL_24_31);
		switch (data_len & 3) {
		case 3:
			ctl |= B43_PIO8_TXCTL_16_23 | B43_PIO8_TXCTL_8_15;
			wl->tx_tail[0] = data[data_len - 3];
			wl->tx_tail[1] = data[data_len - 2];
			wl->tx_tail[2] = data[data_len - 1];
			break;
		case 2:
			ctl |= B43_PIO8_TXCTL_8_15;
			wl->tx_tail[0] = data[data_len - 2];
			wl->tx_tail[1] = data[data_len - 1];
			wl->tx_tail[2] = 0;
			break;
		case 1:
			wl->tx_tail[0] = data[data_len - 1];
			wl->tx_tail[1] = 0;
			wl->tx_tail[2] = 0;
			break;
		}
		b43_piotx_write32(q, B43_PIO8_TXCTL, ctl);
		ssb_block_write(dev->dev, wl->tx_tail, 4,
				q->mmio_base + B43_PIO8_TXDATA,
				sizeof(u32));
	}

	return ctl;
}

static void pio_tx_frame_4byte_queue(struct b43_pio_txpacket *pack,
				     const u8 *hdr, unsigned int hdrlen)
{
	struct b43_pio_txqueue *q = pack->queue;
	const char *frame = pack->skb->data;
	unsigned int frame_len = pack->skb->len;
	u32 ctl;

	ctl = b43_piotx_read32(q, B43_PIO8_TXCTL);
	ctl |= B43_PIO8_TXCTL_FREADY;
	ctl &= ~B43_PIO8_TXCTL_EOF;

	
	ctl = tx_write_4byte_queue(q, ctl, hdr, hdrlen);
	
	ctl = tx_write_4byte_queue(q, ctl, frame, frame_len);

	ctl |= B43_PIO8_TXCTL_EOF;
	b43_piotx_write32(q, B43_PIO_TXCTL, ctl);
}

static int pio_tx_frame(struct b43_pio_txqueue *q,
			struct sk_buff *skb)
{
	struct b43_wldev *dev = q->dev;
	struct b43_wl *wl = dev->wl;
	struct b43_pio_txpacket *pack;
	u16 cookie;
	int err;
	unsigned int hdrlen;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);

	B43_WARN_ON(list_empty(&q->packets_list));
	pack = list_entry(q->packets_list.next,
			  struct b43_pio_txpacket, list);

	cookie = generate_cookie(q, pack);
	hdrlen = b43_txhdr_size(dev);
	err = b43_generate_txhdr(dev, (u8 *)&wl->txhdr, skb,
				 info, cookie);
	if (err)
		return err;

	if (info->flags & IEEE80211_TX_CTL_SEND_AFTER_DTIM) {
		
		b43_shm_write16(dev, B43_SHM_SHARED,
				B43_SHM_SH_MCASTCOOKIE, cookie);
	}

	pack->skb = skb;
	if (q->rev >= 8)
		pio_tx_frame_4byte_queue(pack, (const u8 *)&wl->txhdr, hdrlen);
	else
		pio_tx_frame_2byte_queue(pack, (const u8 *)&wl->txhdr, hdrlen);

	
	list_del(&pack->list);

	
	q->buffer_used += roundup(skb->len + hdrlen, 4);
	q->free_packet_slots -= 1;

	return 0;
}

int b43_pio_tx(struct b43_wldev *dev, struct sk_buff *skb)
{
	struct b43_pio_txqueue *q;
	struct ieee80211_hdr *hdr;
	unsigned int hdrlen, total_len;
	int err = 0;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);

	hdr = (struct ieee80211_hdr *)skb->data;

	if (info->flags & IEEE80211_TX_CTL_SEND_AFTER_DTIM) {
		
		q = dev->pio.tx_queue_mcast;
		
		hdr->frame_control |= cpu_to_le16(IEEE80211_FCTL_MOREDATA);
	} else {
		
		q = select_queue_by_priority(dev, skb_get_queue_mapping(skb));
	}

	hdrlen = b43_txhdr_size(dev);
	total_len = roundup(skb->len + hdrlen, 4);

	if (unlikely(total_len > q->buffer_size)) {
		err = -ENOBUFS;
		b43dbg(dev->wl, "PIO: TX packet longer than queue.\n");
		goto out;
	}
	if (unlikely(q->free_packet_slots == 0)) {
		err = -ENOBUFS;
		b43warn(dev->wl, "PIO: TX packet overflow.\n");
		goto out;
	}
	B43_WARN_ON(q->buffer_used > q->buffer_size);

	if (total_len > (q->buffer_size - q->buffer_used)) {
		
		err = -EBUSY;
		ieee80211_stop_queue(dev->wl->hw, skb_get_queue_mapping(skb));
		q->stopped = 1;
		goto out;
	}

	
	q->queue_prio = skb_get_queue_mapping(skb);

	err = pio_tx_frame(q, skb);
	if (unlikely(err == -ENOKEY)) {
		
		dev_kfree_skb_any(skb);
		err = 0;
		goto out;
	}
	if (unlikely(err)) {
		b43err(dev->wl, "PIO transmission failure\n");
		goto out;
	}
	q->nr_tx_packets++;

	B43_WARN_ON(q->buffer_used > q->buffer_size);
	if (((q->buffer_size - q->buffer_used) < roundup(2 + 2 + 6, 4)) ||
	    (q->free_packet_slots == 0)) {
		
		ieee80211_stop_queue(dev->wl->hw, skb_get_queue_mapping(skb));
		q->stopped = 1;
	}

out:
	return err;
}

void b43_pio_handle_txstatus(struct b43_wldev *dev,
			     const struct b43_txstatus *status)
{
	struct b43_pio_txqueue *q;
	struct b43_pio_txpacket *pack = NULL;
	unsigned int total_len;
	struct ieee80211_tx_info *info;

	q = parse_cookie(dev, status->cookie, &pack);
	if (unlikely(!q))
		return;
	B43_WARN_ON(!pack);

	info = IEEE80211_SKB_CB(pack->skb);

	b43_fill_txstatus_report(dev, info, status);

	total_len = pack->skb->len + b43_txhdr_size(dev);
	total_len = roundup(total_len, 4);
	q->buffer_used -= total_len;
	q->free_packet_slots += 1;

	ieee80211_tx_status(dev->wl->hw, pack->skb);
	pack->skb = NULL;
	list_add(&pack->list, &q->packets_list);

	if (q->stopped) {
		ieee80211_wake_queue(dev->wl->hw, q->queue_prio);
		q->stopped = 0;
	}
}

void b43_pio_get_tx_stats(struct b43_wldev *dev,
			  struct ieee80211_tx_queue_stats *stats)
{
	const int nr_queues = dev->wl->hw->queues;
	struct b43_pio_txqueue *q;
	int i;

	for (i = 0; i < nr_queues; i++) {
		q = select_queue_by_priority(dev, i);

		stats[i].len = B43_PIO_MAX_NR_TXPACKETS - q->free_packet_slots;
		stats[i].limit = B43_PIO_MAX_NR_TXPACKETS;
		stats[i].count = q->nr_tx_packets;
	}
}


static bool pio_rx_frame(struct b43_pio_rxqueue *q)
{
	struct b43_wldev *dev = q->dev;
	struct b43_wl *wl = dev->wl;
	u16 len;
	u32 macstat;
	unsigned int i, padding;
	struct sk_buff *skb;
	const char *err_msg = NULL;

	memset(&wl->rxhdr, 0, sizeof(wl->rxhdr));

	
	if (q->rev >= 8) {
		u32 ctl;

		ctl = b43_piorx_read32(q, B43_PIO8_RXCTL);
		if (!(ctl & B43_PIO8_RXCTL_FRAMERDY))
			return 0;
		b43_piorx_write32(q, B43_PIO8_RXCTL,
				  B43_PIO8_RXCTL_FRAMERDY);
		for (i = 0; i < 10; i++) {
			ctl = b43_piorx_read32(q, B43_PIO8_RXCTL);
			if (ctl & B43_PIO8_RXCTL_DATARDY)
				goto data_ready;
			udelay(10);
		}
	} else {
		u16 ctl;

		ctl = b43_piorx_read16(q, B43_PIO_RXCTL);
		if (!(ctl & B43_PIO_RXCTL_FRAMERDY))
			return 0;
		b43_piorx_write16(q, B43_PIO_RXCTL,
				  B43_PIO_RXCTL_FRAMERDY);
		for (i = 0; i < 10; i++) {
			ctl = b43_piorx_read16(q, B43_PIO_RXCTL);
			if (ctl & B43_PIO_RXCTL_DATARDY)
				goto data_ready;
			udelay(10);
		}
	}
	b43dbg(q->dev->wl, "PIO RX timed out\n");
	return 1;
data_ready:

	
	if (q->rev >= 8) {
		ssb_block_read(dev->dev, &wl->rxhdr, sizeof(wl->rxhdr),
			       q->mmio_base + B43_PIO8_RXDATA,
			       sizeof(u32));
	} else {
		ssb_block_read(dev->dev, &wl->rxhdr, sizeof(wl->rxhdr),
			       q->mmio_base + B43_PIO_RXDATA,
			       sizeof(u16));
	}
	
	len = le16_to_cpu(wl->rxhdr.frame_len);
	if (unlikely(len > 0x700)) {
		err_msg = "len > 0x700";
		goto rx_error;
	}
	if (unlikely(len == 0)) {
		err_msg = "len == 0";
		goto rx_error;
	}

	macstat = le32_to_cpu(wl->rxhdr.mac_status);
	if (macstat & B43_RX_MAC_FCSERR) {
		if (!(q->dev->wl->filter_flags & FIF_FCSFAIL)) {
			
			err_msg = "Frame FCS error";
			goto rx_error;
		}
	}

	
	padding = (macstat & B43_RX_MAC_PADDING) ? 2 : 0;
	skb = dev_alloc_skb(len + padding + 2);
	if (unlikely(!skb)) {
		err_msg = "Out of memory";
		goto rx_error;
	}
	skb_reserve(skb, 2);
	skb_put(skb, len + padding);
	if (q->rev >= 8) {
		ssb_block_read(dev->dev, skb->data + padding, (len & ~3),
			       q->mmio_base + B43_PIO8_RXDATA,
			       sizeof(u32));
		if (len & 3) {
			
			ssb_block_read(dev->dev, wl->rx_tail, 4,
				       q->mmio_base + B43_PIO8_RXDATA,
				       sizeof(u32));
			switch (len & 3) {
			case 3:
				skb->data[len + padding - 3] = wl->rx_tail[0];
				skb->data[len + padding - 2] = wl->rx_tail[1];
				skb->data[len + padding - 1] = wl->rx_tail[2];
				break;
			case 2:
				skb->data[len + padding - 2] = wl->rx_tail[0];
				skb->data[len + padding - 1] = wl->rx_tail[1];
				break;
			case 1:
				skb->data[len + padding - 1] = wl->rx_tail[0];
				break;
			}
		}
	} else {
		ssb_block_read(dev->dev, skb->data + padding, (len & ~1),
			       q->mmio_base + B43_PIO_RXDATA,
			       sizeof(u16));
		if (len & 1) {
			
			ssb_block_read(dev->dev, wl->rx_tail, 2,
				       q->mmio_base + B43_PIO_RXDATA,
				       sizeof(u16));
			skb->data[len + padding - 1] = wl->rx_tail[0];
		}
	}

	b43_rx(q->dev, skb, &wl->rxhdr);

	return 1;

rx_error:
	if (err_msg)
		b43dbg(q->dev->wl, "PIO RX error: %s\n", err_msg);
	b43_piorx_write16(q, B43_PIO_RXCTL, B43_PIO_RXCTL_DATARDY);
	return 1;
}

void b43_pio_rx(struct b43_pio_rxqueue *q)
{
	unsigned int count = 0;
	bool stop;

	while (1) {
		stop = (pio_rx_frame(q) == 0);
		if (stop)
			break;
		cond_resched();
		if (WARN_ON_ONCE(++count > 10000))
			break;
	}
}

static void b43_pio_tx_suspend_queue(struct b43_pio_txqueue *q)
{
	if (q->rev >= 8) {
		b43_piotx_write32(q, B43_PIO8_TXCTL,
				  b43_piotx_read32(q, B43_PIO8_TXCTL)
				  | B43_PIO8_TXCTL_SUSPREQ);
	} else {
		b43_piotx_write16(q, B43_PIO_TXCTL,
				  b43_piotx_read16(q, B43_PIO_TXCTL)
				  | B43_PIO_TXCTL_SUSPREQ);
	}
}

static void b43_pio_tx_resume_queue(struct b43_pio_txqueue *q)
{
	if (q->rev >= 8) {
		b43_piotx_write32(q, B43_PIO8_TXCTL,
				  b43_piotx_read32(q, B43_PIO8_TXCTL)
				  & ~B43_PIO8_TXCTL_SUSPREQ);
	} else {
		b43_piotx_write16(q, B43_PIO_TXCTL,
				  b43_piotx_read16(q, B43_PIO_TXCTL)
				  & ~B43_PIO_TXCTL_SUSPREQ);
	}
}

void b43_pio_tx_suspend(struct b43_wldev *dev)
{
	b43_power_saving_ctl_bits(dev, B43_PS_AWAKE);
	b43_pio_tx_suspend_queue(dev->pio.tx_queue_AC_BK);
	b43_pio_tx_suspend_queue(dev->pio.tx_queue_AC_BE);
	b43_pio_tx_suspend_queue(dev->pio.tx_queue_AC_VI);
	b43_pio_tx_suspend_queue(dev->pio.tx_queue_AC_VO);
	b43_pio_tx_suspend_queue(dev->pio.tx_queue_mcast);
}

void b43_pio_tx_resume(struct b43_wldev *dev)
{
	b43_pio_tx_resume_queue(dev->pio.tx_queue_mcast);
	b43_pio_tx_resume_queue(dev->pio.tx_queue_AC_VO);
	b43_pio_tx_resume_queue(dev->pio.tx_queue_AC_VI);
	b43_pio_tx_resume_queue(dev->pio.tx_queue_AC_BE);
	b43_pio_tx_resume_queue(dev->pio.tx_queue_AC_BK);
	b43_power_saving_ctl_bits(dev, 0);
}
