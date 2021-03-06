
#include <linux/kernel.h>
#include <linux/if_arp.h>
#include <linux/netdevice.h>
#include <linux/workqueue.h>
#include "i2400m.h"


#define D_SUBMODULE rx
#include "debug-levels.h"

struct i2400m_report_hook_args {
	struct sk_buff *skb_rx;
	const struct i2400m_l3l4_hdr *l3l4_hdr;
	size_t size;
};



static
void i2400m_report_hook_work(struct work_struct *ws)
{
	struct i2400m_work *iw =
		container_of(ws, struct i2400m_work, ws);
	struct i2400m_report_hook_args *args = (void *) iw->pl;
	if (iw->i2400m->ready)
		i2400m_report_hook(iw->i2400m, args->l3l4_hdr, args->size);
	kfree_skb(args->skb_rx);
	i2400m_put(iw->i2400m);
	kfree(iw);
}



static
void i2400m_rx_ctl_ack(struct i2400m *i2400m,
		       const void *payload, size_t size)
{
	struct device *dev = i2400m_dev(i2400m);
	struct wimax_dev *wimax_dev = &i2400m->wimax_dev;
	unsigned long flags;
	struct sk_buff *ack_skb;

	
	spin_lock_irqsave(&i2400m->rx_lock, flags);
	if (i2400m->ack_skb != ERR_PTR(-EINPROGRESS)) {
		dev_err(dev, "Huh? reply to command with no waiters\n");
		goto error_no_waiter;
	}
	spin_unlock_irqrestore(&i2400m->rx_lock, flags);

	ack_skb = wimax_msg_alloc(wimax_dev, NULL, payload, size, GFP_KERNEL);

	
	spin_lock_irqsave(&i2400m->rx_lock, flags);
	if (i2400m->ack_skb != ERR_PTR(-EINPROGRESS)) {
		d_printf(1, dev, "Huh? waiter for command reply cancelled\n");
		goto error_waiter_cancelled;
	}
	if (ack_skb == NULL) {
		dev_err(dev, "CMD/GET/SET ack: cannot allocate SKB\n");
		i2400m->ack_skb = ERR_PTR(-ENOMEM);
	} else
		i2400m->ack_skb = ack_skb;
	spin_unlock_irqrestore(&i2400m->rx_lock, flags);
	complete(&i2400m->msg_completion);
	return;

error_waiter_cancelled:
	kfree_skb(ack_skb);
error_no_waiter:
	spin_unlock_irqrestore(&i2400m->rx_lock, flags);
	return;
}



static
void i2400m_rx_ctl(struct i2400m *i2400m, struct sk_buff *skb_rx,
		   const void *payload, size_t size)
{
	int result;
	struct device *dev = i2400m_dev(i2400m);
	const struct i2400m_l3l4_hdr *l3l4_hdr = payload;
	unsigned msg_type;

	result = i2400m_msg_size_check(i2400m, l3l4_hdr, size);
	if (result < 0) {
		dev_err(dev, "HW BUG? device sent a bad message: %d\n",
			result);
		goto error_check;
	}
	msg_type = le16_to_cpu(l3l4_hdr->type);
	d_printf(1, dev, "%s 0x%04x: %zu bytes\n",
		 msg_type & I2400M_MT_REPORT_MASK ? "REPORT" : "CMD/SET/GET",
		 msg_type, size);
	d_dump(2, dev, l3l4_hdr, size);
	if (msg_type & I2400M_MT_REPORT_MASK) {
		
		struct i2400m_report_hook_args args = {
			.skb_rx = skb_rx,
			.l3l4_hdr = l3l4_hdr,
			.size = size
		};
		if (unlikely(i2400m->ready == 0))	
			return;
		skb_get(skb_rx);
		i2400m_queue_work(i2400m, i2400m_report_hook_work,
				  GFP_KERNEL, &args, sizeof(args));
		if (unlikely(i2400m->trace_msg_from_user))
			wimax_msg(&i2400m->wimax_dev, "echo",
				  l3l4_hdr, size, GFP_KERNEL);
		result = wimax_msg(&i2400m->wimax_dev, NULL, l3l4_hdr, size,
				   GFP_KERNEL);
		if (result < 0)
			dev_err(dev, "error sending report to userspace: %d\n",
				result);
	} else		
		i2400m_rx_ctl_ack(i2400m, payload, size);
error_check:
	return;
}



static
void i2400m_rx_trace(struct i2400m *i2400m,
		     const void *payload, size_t size)
{
	int result;
	struct device *dev = i2400m_dev(i2400m);
	struct wimax_dev *wimax_dev = &i2400m->wimax_dev;
	const struct i2400m_l3l4_hdr *l3l4_hdr = payload;
	unsigned msg_type;

	result = i2400m_msg_size_check(i2400m, l3l4_hdr, size);
	if (result < 0) {
		dev_err(dev, "HW BUG? device sent a bad trace message: %d\n",
			result);
		goto error_check;
	}
	msg_type = le16_to_cpu(l3l4_hdr->type);
	d_printf(1, dev, "Trace %s 0x%04x: %zu bytes\n",
		 msg_type & I2400M_MT_REPORT_MASK ? "REPORT" : "CMD/SET/GET",
		 msg_type, size);
	d_dump(2, dev, l3l4_hdr, size);
	if (unlikely(i2400m->ready == 0))	
		return;
	result = wimax_msg(wimax_dev, "trace", l3l4_hdr, size, GFP_KERNEL);
	if (result < 0)
		dev_err(dev, "error sending trace to userspace: %d\n",
			result);
error_check:
	return;
}



struct i2400m_roq_data {
	unsigned sn;		
	enum i2400m_cs cs;	
};



struct i2400m_roq
{
	unsigned ws;
	struct sk_buff_head queue;
	struct i2400m_roq_log *log;
};


static
void __i2400m_roq_init(struct i2400m_roq *roq)
{
	roq->ws = 0;
	skb_queue_head_init(&roq->queue);
}


static
unsigned __i2400m_roq_index(struct i2400m *i2400m, struct i2400m_roq *roq)
{
	return ((unsigned long) roq - (unsigned long) i2400m->rx_roq)
		/ sizeof(*roq);
}



static
unsigned __i2400m_roq_nsn(struct i2400m_roq *roq, unsigned sn)
{
	int r;
	r =  ((int) sn - (int) roq->ws) % 2048;
	if (r < 0)
		r += 2048;
	return r;
}



enum {
	I2400M_ROQ_LOG_LENGTH = 32,
};

struct i2400m_roq_log {
	struct i2400m_roq_log_entry {
		enum i2400m_ro_type type;
		unsigned ws, count, sn, nsn, new_ws;
	} entry[I2400M_ROQ_LOG_LENGTH];
	unsigned in, out;
};



static
void i2400m_roq_log_entry_print(struct i2400m *i2400m, unsigned index,
				unsigned e_index,
				struct i2400m_roq_log_entry *e)
{
	struct device *dev = i2400m_dev(i2400m);

	switch(e->type) {
	case I2400M_RO_TYPE_RESET:
		dev_err(dev, "q#%d reset           ws %u cnt %u sn %u/%u"
			" - new nws %u\n",
			index, e->ws, e->count, e->sn, e->nsn, e->new_ws);
		break;
	case I2400M_RO_TYPE_PACKET:
		dev_err(dev, "q#%d queue           ws %u cnt %u sn %u/%u\n",
			index, e->ws, e->count, e->sn, e->nsn);
		break;
	case I2400M_RO_TYPE_WS:
		dev_err(dev, "q#%d update_ws       ws %u cnt %u sn %u/%u"
			" - new nws %u\n",
			index, e->ws, e->count, e->sn, e->nsn, e->new_ws);
		break;
	case I2400M_RO_TYPE_PACKET_WS:
		dev_err(dev, "q#%d queue_update_ws ws %u cnt %u sn %u/%u"
			" - new nws %u\n",
			index, e->ws, e->count, e->sn, e->nsn, e->new_ws);
		break;
	default:
		dev_err(dev, "q#%d BUG? entry %u - unknown type %u\n",
			index, e_index, e->type);
		break;
	}
}


static
void i2400m_roq_log_add(struct i2400m *i2400m,
			struct i2400m_roq *roq, enum i2400m_ro_type type,
			unsigned ws, unsigned count, unsigned sn,
			unsigned nsn, unsigned new_ws)
{
	struct i2400m_roq_log_entry *e;
	unsigned cnt_idx;
	int index = __i2400m_roq_index(i2400m, roq);

	
	if (roq->log->in - roq->log->out == I2400M_ROQ_LOG_LENGTH)
		roq->log->out++;
	cnt_idx = roq->log->in++ % I2400M_ROQ_LOG_LENGTH;
	e = &roq->log->entry[cnt_idx];

	e->type = type;
	e->ws = ws;
	e->count = count;
	e->sn = sn;
	e->nsn = nsn;
	e->new_ws = new_ws;

	if (d_test(1))
		i2400m_roq_log_entry_print(i2400m, index, cnt_idx, e);
}



static
void i2400m_roq_log_dump(struct i2400m *i2400m, struct i2400m_roq *roq)
{
	unsigned cnt, cnt_idx;
	struct i2400m_roq_log_entry *e;
	int index = __i2400m_roq_index(i2400m, roq);

	BUG_ON(roq->log->out > roq->log->in);
	for (cnt = roq->log->out; cnt < roq->log->in; cnt++) {
		cnt_idx = cnt % I2400M_ROQ_LOG_LENGTH;
		e = &roq->log->entry[cnt_idx];
		i2400m_roq_log_entry_print(i2400m, index, cnt_idx, e);
		memset(e, 0, sizeof(*e));
	}
	roq->log->in = roq->log->out = 0;
}



static
void __i2400m_roq_queue(struct i2400m *i2400m, struct i2400m_roq *roq,
			struct sk_buff *skb, unsigned sn, unsigned nsn)
{
	struct device *dev = i2400m_dev(i2400m);
	struct sk_buff *skb_itr;
	struct i2400m_roq_data *roq_data_itr, *roq_data;
	unsigned nsn_itr;

	d_fnstart(4, dev, "(i2400m %p roq %p skb %p sn %u nsn %u)\n",
		  i2400m, roq, skb, sn, nsn);

	roq_data = (struct i2400m_roq_data *) &skb->cb;
	BUILD_BUG_ON(sizeof(*roq_data) > sizeof(skb->cb));
	roq_data->sn = sn;
	d_printf(3, dev, "ERX: roq %p [ws %u] nsn %d sn %u\n",
		 roq, roq->ws, nsn, roq_data->sn);

	
	if (skb_queue_empty(&roq->queue)) {
		d_printf(2, dev, "ERX: roq %p - first one\n", roq);
		__skb_queue_head(&roq->queue, skb);
		goto out;
	}
	
	skb_itr = skb_peek_tail(&roq->queue);
	roq_data_itr = (struct i2400m_roq_data *) &skb_itr->cb;
	nsn_itr = __i2400m_roq_nsn(roq, roq_data_itr->sn);
	
	if (nsn >= nsn_itr) {
		d_printf(2, dev, "ERX: roq %p - appended after %p (nsn %d sn %u)\n",
			 roq, skb_itr, nsn_itr, roq_data_itr->sn);
		__skb_queue_tail(&roq->queue, skb);
		goto out;
	}
	
	skb_queue_walk(&roq->queue, skb_itr) {
		roq_data_itr = (struct i2400m_roq_data *) &skb_itr->cb;
		nsn_itr = __i2400m_roq_nsn(roq, roq_data_itr->sn);
		
		if (nsn_itr > nsn) {
			d_printf(2, dev, "ERX: roq %p - queued before %p "
				 "(nsn %d sn %u)\n", roq, skb_itr, nsn_itr,
				 roq_data_itr->sn);
			__skb_queue_before(&roq->queue, skb_itr, skb);
			goto out;
		}
	}
	
	dev_err(dev, "SW BUG? failed to insert packet\n");
	dev_err(dev, "ERX: roq %p [ws %u] skb %p nsn %d sn %u\n",
		roq, roq->ws, skb, nsn, roq_data->sn);
	skb_queue_walk(&roq->queue, skb_itr) {
		roq_data_itr = (struct i2400m_roq_data *) &skb_itr->cb;
		nsn_itr = __i2400m_roq_nsn(roq, roq_data_itr->sn);
		
		dev_err(dev, "ERX: roq %p skb_itr %p nsn %d sn %u\n",
			roq, skb_itr, nsn_itr, roq_data_itr->sn);
	}
	BUG();
out:
	d_fnend(4, dev, "(i2400m %p roq %p skb %p sn %u nsn %d) = void\n",
		i2400m, roq, skb, sn, nsn);
	return;
}



static
unsigned __i2400m_roq_update_ws(struct i2400m *i2400m, struct i2400m_roq *roq,
				unsigned sn)
{
	struct device *dev = i2400m_dev(i2400m);
	struct sk_buff *skb_itr, *tmp_itr;
	struct i2400m_roq_data *roq_data_itr;
	unsigned new_nws, nsn_itr;

	new_nws = __i2400m_roq_nsn(roq, sn);
	if (unlikely(new_nws >= 1024) && d_test(1)) {
		dev_err(dev, "SW BUG? __update_ws new_nws %u (sn %u ws %u)\n",
			new_nws, sn, roq->ws);
		WARN_ON(1);
		i2400m_roq_log_dump(i2400m, roq);
	}
	skb_queue_walk_safe(&roq->queue, skb_itr, tmp_itr) {
		roq_data_itr = (struct i2400m_roq_data *) &skb_itr->cb;
		nsn_itr = __i2400m_roq_nsn(roq, roq_data_itr->sn);
		
		if (nsn_itr < new_nws) {
			d_printf(2, dev, "ERX: roq %p - release skb %p "
				 "(nsn %u/%u new nws %u)\n",
				 roq, skb_itr, nsn_itr, roq_data_itr->sn,
				 new_nws);
			__skb_unlink(skb_itr, &roq->queue);
			i2400m_net_erx(i2400m, skb_itr, roq_data_itr->cs);
		}
		else
			break;	
	}
	roq->ws = sn;
	return new_nws;
}



static
void i2400m_roq_reset(struct i2400m *i2400m, struct i2400m_roq *roq)
{
	struct device *dev = i2400m_dev(i2400m);
	struct sk_buff *skb_itr, *tmp_itr;
	struct i2400m_roq_data *roq_data_itr;

	d_fnstart(2, dev, "(i2400m %p roq %p)\n", i2400m, roq);
	i2400m_roq_log_add(i2400m, roq, I2400M_RO_TYPE_RESET,
			     roq->ws, skb_queue_len(&roq->queue),
			     ~0, ~0, 0);
	skb_queue_walk_safe(&roq->queue, skb_itr, tmp_itr) {
		roq_data_itr = (struct i2400m_roq_data *) &skb_itr->cb;
		d_printf(2, dev, "ERX: roq %p - release skb %p (sn %u)\n",
			 roq, skb_itr, roq_data_itr->sn);
		__skb_unlink(skb_itr, &roq->queue);
		i2400m_net_erx(i2400m, skb_itr, roq_data_itr->cs);
	}
	roq->ws = 0;
	d_fnend(2, dev, "(i2400m %p roq %p) = void\n", i2400m, roq);
	return;
}



static
void i2400m_roq_queue(struct i2400m *i2400m, struct i2400m_roq *roq,
		      struct sk_buff * skb, unsigned lbn)
{
	struct device *dev = i2400m_dev(i2400m);
	unsigned nsn, len;

	d_fnstart(2, dev, "(i2400m %p roq %p skb %p lbn %u) = void\n",
		  i2400m, roq, skb, lbn);
	len = skb_queue_len(&roq->queue);
	nsn = __i2400m_roq_nsn(roq, lbn);
	if (unlikely(nsn >= 1024)) {
		dev_err(dev, "SW BUG? queue nsn %d (lbn %u ws %u)\n",
			nsn, lbn, roq->ws);
		i2400m_roq_log_dump(i2400m, roq);
		i2400m->bus_reset(i2400m, I2400M_RT_WARM);
	} else {
		__i2400m_roq_queue(i2400m, roq, skb, lbn, nsn);
		i2400m_roq_log_add(i2400m, roq, I2400M_RO_TYPE_PACKET,
				     roq->ws, len, lbn, nsn, ~0);
	}
	d_fnend(2, dev, "(i2400m %p roq %p skb %p lbn %u) = void\n",
		i2400m, roq, skb, lbn);
	return;
}



static
void i2400m_roq_update_ws(struct i2400m *i2400m, struct i2400m_roq *roq,
			  unsigned sn)
{
	struct device *dev = i2400m_dev(i2400m);
	unsigned old_ws, nsn, len;

	d_fnstart(2, dev, "(i2400m %p roq %p sn %u)\n", i2400m, roq, sn);
	old_ws = roq->ws;
	len = skb_queue_len(&roq->queue);
	nsn = __i2400m_roq_update_ws(i2400m, roq, sn);
	i2400m_roq_log_add(i2400m, roq, I2400M_RO_TYPE_WS,
			     old_ws, len, sn, nsn, roq->ws);
	d_fnstart(2, dev, "(i2400m %p roq %p sn %u) = void\n", i2400m, roq, sn);
	return;
}



static
void i2400m_roq_queue_update_ws(struct i2400m *i2400m, struct i2400m_roq *roq,
				struct sk_buff * skb, unsigned sn)
{
	struct device *dev = i2400m_dev(i2400m);
	unsigned nsn, old_ws, len;

	d_fnstart(2, dev, "(i2400m %p roq %p skb %p sn %u)\n",
		  i2400m, roq, skb, sn);
	len = skb_queue_len(&roq->queue);
	nsn = __i2400m_roq_nsn(roq, sn);
	old_ws = roq->ws;
	if (unlikely(nsn >= 1024)) {
		dev_err(dev, "SW BUG? queue_update_ws nsn %u (sn %u ws %u)\n",
			nsn, sn, roq->ws);
		i2400m_roq_log_dump(i2400m, roq);
		i2400m->bus_reset(i2400m, I2400M_RT_WARM);
	} else {
		
		if (len == 0) {
			struct i2400m_roq_data *roq_data;
			roq_data = (struct i2400m_roq_data *) &skb->cb;
			i2400m_net_erx(i2400m, skb, roq_data->cs);
		}
		else
			__i2400m_roq_queue(i2400m, roq, skb, sn, nsn);
		__i2400m_roq_update_ws(i2400m, roq, sn + 1);
		i2400m_roq_log_add(i2400m, roq, I2400M_RO_TYPE_PACKET_WS,
				   old_ws, len, sn, nsn, roq->ws);
	}
	d_fnend(2, dev, "(i2400m %p roq %p skb %p sn %u) = void\n",
		i2400m, roq, skb, sn);
	return;
}



static
void i2400m_rx_edata(struct i2400m *i2400m, struct sk_buff *skb_rx,
		     unsigned single_last, const void *payload, size_t size)
{
	struct device *dev = i2400m_dev(i2400m);
	const struct i2400m_pl_edata_hdr *hdr = payload;
	struct net_device *net_dev = i2400m->wimax_dev.net_dev;
	struct sk_buff *skb;
	enum i2400m_cs cs;
	u32 reorder;
	unsigned ro_needed, ro_type, ro_cin, ro_sn;
	struct i2400m_roq *roq;
	struct i2400m_roq_data *roq_data;

	BUILD_BUG_ON(ETH_HLEN > sizeof(*hdr));

	d_fnstart(2, dev, "(i2400m %p skb_rx %p single %u payload %p "
		  "size %zu)\n", i2400m, skb_rx, single_last, payload, size);
	if (size < sizeof(*hdr)) {
		dev_err(dev, "ERX: HW BUG? message with short header (%zu "
			"vs %zu bytes expected)\n", size, sizeof(*hdr));
		goto error;
	}

	if (single_last) {
		skb = skb_get(skb_rx);
		d_printf(3, dev, "ERX: skb %p reusing\n", skb);
	} else {
		skb = skb_clone(skb_rx, GFP_KERNEL);
		if (skb == NULL) {
			dev_err(dev, "ERX: no memory to clone skb\n");
			net_dev->stats.rx_dropped++;
			goto error_skb_clone;
		}
		d_printf(3, dev, "ERX: skb %p cloned from %p\n", skb, skb_rx);
	}
	
	skb_pull(skb, payload + sizeof(*hdr) - (void *) skb->data);
	skb_trim(skb, (void *) skb_end_pointer(skb) - payload - sizeof(*hdr));

	reorder = le32_to_cpu(hdr->reorder);
	ro_needed = reorder & I2400M_RO_NEEDED;
	cs = hdr->cs;
	if (ro_needed) {
		ro_type = (reorder >> I2400M_RO_TYPE_SHIFT) & I2400M_RO_TYPE;
		ro_cin = (reorder >> I2400M_RO_CIN_SHIFT) & I2400M_RO_CIN;
		ro_sn = (reorder >> I2400M_RO_SN_SHIFT) & I2400M_RO_SN;

		roq = &i2400m->rx_roq[ro_cin];
		roq_data = (struct i2400m_roq_data *) &skb->cb;
		roq_data->sn = ro_sn;
		roq_data->cs = cs;
		d_printf(2, dev, "ERX: reorder needed: "
			 "type %u cin %u [ws %u] sn %u/%u len %zuB\n",
			 ro_type, ro_cin, roq->ws, ro_sn,
			 __i2400m_roq_nsn(roq, ro_sn), size);
		d_dump(2, dev, payload, size);
		switch(ro_type) {
		case I2400M_RO_TYPE_RESET:
			i2400m_roq_reset(i2400m, roq);
			kfree_skb(skb);	
			break;
		case I2400M_RO_TYPE_PACKET:
			i2400m_roq_queue(i2400m, roq, skb, ro_sn);
			break;
		case I2400M_RO_TYPE_WS:
			i2400m_roq_update_ws(i2400m, roq, ro_sn);
			kfree_skb(skb);	
			break;
		case I2400M_RO_TYPE_PACKET_WS:
			i2400m_roq_queue_update_ws(i2400m, roq, skb, ro_sn);
			break;
		default:
			dev_err(dev, "HW BUG? unknown reorder type %u\n", ro_type);
		}
	}
	else
		i2400m_net_erx(i2400m, skb, cs);
error_skb_clone:
error:
	d_fnend(2, dev, "(i2400m %p skb_rx %p single %u payload %p "
		"size %zu) = void\n", i2400m, skb_rx, single_last, payload, size);
	return;
}



static
void i2400m_rx_payload(struct i2400m *i2400m, struct sk_buff *skb_rx,
		       unsigned single_last, const struct i2400m_pld *pld,
		       const void *payload)
{
	struct device *dev = i2400m_dev(i2400m);
	size_t pl_size = i2400m_pld_size(pld);
	enum i2400m_pt pl_type = i2400m_pld_type(pld);

	d_printf(7, dev, "RX: received payload type %u, %zu bytes\n",
		 pl_type, pl_size);
	d_dump(8, dev, payload, pl_size);

	switch (pl_type) {
	case I2400M_PT_DATA:
		d_printf(3, dev, "RX: data payload %zu bytes\n", pl_size);
		i2400m_net_rx(i2400m, skb_rx, single_last, payload, pl_size);
		break;
	case I2400M_PT_CTRL:
		i2400m_rx_ctl(i2400m, skb_rx, payload, pl_size);
		break;
	case I2400M_PT_TRACE:
		i2400m_rx_trace(i2400m, payload, pl_size);
		break;
	case I2400M_PT_EDATA:
		d_printf(3, dev, "ERX: data payload %zu bytes\n", pl_size);
		i2400m_rx_edata(i2400m, skb_rx, single_last, payload, pl_size);
		break;
	default:	
		if (printk_ratelimit())
			dev_err(dev, "RX: HW BUG? unexpected payload type %u\n",
				pl_type);
	}
}



static
int i2400m_rx_msg_hdr_check(struct i2400m *i2400m,
			    const struct i2400m_msg_hdr *msg_hdr,
			    size_t buf_size)
{
	int result = -EIO;
	struct device *dev = i2400m_dev(i2400m);
	if (buf_size < sizeof(*msg_hdr)) {
		dev_err(dev, "RX: HW BUG? message with short header (%zu "
			"vs %zu bytes expected)\n", buf_size, sizeof(*msg_hdr));
		goto error;
	}
	if (msg_hdr->barker != cpu_to_le32(I2400M_D2H_MSG_BARKER)) {
		dev_err(dev, "RX: HW BUG? message received with unknown "
			"barker 0x%08x (buf_size %zu bytes)\n",
			le32_to_cpu(msg_hdr->barker), buf_size);
		goto error;
	}
	if (msg_hdr->num_pls == 0) {
		dev_err(dev, "RX: HW BUG? zero payload packets in message\n");
		goto error;
	}
	if (le16_to_cpu(msg_hdr->num_pls) > I2400M_MAX_PLS_IN_MSG) {
		dev_err(dev, "RX: HW BUG? message contains more payload "
			"than maximum; ignoring.\n");
		goto error;
	}
	result = 0;
error:
	return result;
}



static
int i2400m_rx_pl_descr_check(struct i2400m *i2400m,
			      const struct i2400m_pld *pld,
			      size_t pl_itr, size_t buf_size)
{
	int result = -EIO;
	struct device *dev = i2400m_dev(i2400m);
	size_t pl_size = i2400m_pld_size(pld);
	enum i2400m_pt pl_type = i2400m_pld_type(pld);

	if (pl_size > i2400m->bus_pl_size_max) {
		dev_err(dev, "RX: HW BUG? payload @%zu: size %zu is "
			"bigger than maximum %zu; ignoring message\n",
			pl_itr, pl_size, i2400m->bus_pl_size_max);
		goto error;
	}
	if (pl_itr + pl_size > buf_size) {	
		dev_err(dev, "RX: HW BUG? payload @%zu: size %zu "
			"goes beyond the received buffer "
			"size (%zu bytes); ignoring message\n",
			pl_itr, pl_size, buf_size);
		goto error;
	}
	if (pl_type >= I2400M_PT_ILLEGAL) {
		dev_err(dev, "RX: HW BUG? illegal payload type %u; "
			"ignoring message\n", pl_type);
		goto error;
	}
	result = 0;
error:
	return result;
}



int i2400m_rx(struct i2400m *i2400m, struct sk_buff *skb)
{
	int i, result;
	struct device *dev = i2400m_dev(i2400m);
	const struct i2400m_msg_hdr *msg_hdr;
	size_t pl_itr, pl_size, skb_len;
	unsigned long flags;
	unsigned num_pls, single_last;

	skb_len = skb->len;
	d_fnstart(4, dev, "(i2400m %p skb %p [size %zu])\n",
		  i2400m, skb, skb_len);
	result = -EIO;
	msg_hdr = (void *) skb->data;
	result = i2400m_rx_msg_hdr_check(i2400m, msg_hdr, skb->len);
	if (result < 0)
		goto error_msg_hdr_check;
	result = -EIO;
	num_pls = le16_to_cpu(msg_hdr->num_pls);
	pl_itr = sizeof(*msg_hdr) +	
		num_pls * sizeof(msg_hdr->pld[0]);
	pl_itr = ALIGN(pl_itr, I2400M_PL_ALIGN);
	if (pl_itr > skb->len) {	
		dev_err(dev, "RX: HW BUG? message too short (%u bytes) for "
			"%u payload descriptors (%zu each, total %zu)\n",
			skb->len, num_pls, sizeof(msg_hdr->pld[0]), pl_itr);
		goto error_pl_descr_short;
	}
	
	for (i = 0; i < num_pls; i++) {
		
		pl_size = i2400m_pld_size(&msg_hdr->pld[i]);
		result = i2400m_rx_pl_descr_check(i2400m, &msg_hdr->pld[i],
						  pl_itr, skb->len);
		if (result < 0)
			goto error_pl_descr_check;
		single_last = num_pls == 1 || i == num_pls - 1;
		i2400m_rx_payload(i2400m, skb, single_last, &msg_hdr->pld[i],
				  skb->data + pl_itr);
		pl_itr += ALIGN(pl_size, I2400M_PL_ALIGN);
		cond_resched();		
	}
	kfree_skb(skb);
	
	spin_lock_irqsave(&i2400m->rx_lock, flags);
	i2400m->rx_pl_num += i;
	if (i > i2400m->rx_pl_max)
		i2400m->rx_pl_max = i;
	if (i < i2400m->rx_pl_min)
		i2400m->rx_pl_min = i;
	i2400m->rx_num++;
	i2400m->rx_size_acc += skb->len;
	if (skb->len < i2400m->rx_size_min)
		i2400m->rx_size_min = skb->len;
	if (skb->len > i2400m->rx_size_max)
		i2400m->rx_size_max = skb->len;
	spin_unlock_irqrestore(&i2400m->rx_lock, flags);
error_pl_descr_check:
error_pl_descr_short:
error_msg_hdr_check:
	d_fnend(4, dev, "(i2400m %p skb %p [size %zu]) = %d\n",
		i2400m, skb, skb_len, result);
	return result;
}
EXPORT_SYMBOL_GPL(i2400m_rx);



int i2400m_rx_setup(struct i2400m *i2400m)
{
	int result = 0;
	struct device *dev = i2400m_dev(i2400m);

	i2400m->rx_reorder = i2400m_rx_reorder_disabled? 0 : 1;
	if (i2400m->rx_reorder) {
		unsigned itr;
		size_t size;
		struct i2400m_roq_log *rd;

		result = -ENOMEM;

		size = sizeof(i2400m->rx_roq[0]) * (I2400M_RO_CIN + 1);
		i2400m->rx_roq = kzalloc(size, GFP_KERNEL);
		if (i2400m->rx_roq == NULL) {
			dev_err(dev, "RX: cannot allocate %zu bytes for "
				"reorder queues\n", size);
			goto error_roq_alloc;
		}

		size = sizeof(*i2400m->rx_roq[0].log) * (I2400M_RO_CIN + 1);
		rd = kzalloc(size, GFP_KERNEL);
		if (rd == NULL) {
			dev_err(dev, "RX: cannot allocate %zu bytes for "
				"reorder queues log areas\n", size);
			result = -ENOMEM;
			goto error_roq_log_alloc;
		}

		for(itr = 0; itr < I2400M_RO_CIN + 1; itr++) {
			__i2400m_roq_init(&i2400m->rx_roq[itr]);
			i2400m->rx_roq[itr].log = &rd[itr];
		}
	}
	return 0;

error_roq_log_alloc:
	kfree(i2400m->rx_roq);
error_roq_alloc:
	return result;
}



void i2400m_rx_release(struct i2400m *i2400m)
{
	if (i2400m->rx_reorder) {
		unsigned itr;
		for(itr = 0; itr < I2400M_RO_CIN + 1; itr++)
			__skb_queue_purge(&i2400m->rx_roq[itr].queue);
		kfree(i2400m->rx_roq[0].log);
		kfree(i2400m->rx_roq);
	}
}
