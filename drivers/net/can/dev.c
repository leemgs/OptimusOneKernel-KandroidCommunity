

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/can.h>
#include <linux/can/dev.h>
#include <linux/can/netlink.h>
#include <net/rtnetlink.h>

#define MOD_DESC "CAN device driver interface"

MODULE_DESCRIPTION(MOD_DESC);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Wolfgang Grandegger <wg@grandegger.com>");

#ifdef CONFIG_CAN_CALC_BITTIMING
#define CAN_CALC_MAX_ERROR 50 


static int can_update_spt(const struct can_bittiming_const *btc,
			  int sampl_pt, int tseg, int *tseg1, int *tseg2)
{
	*tseg2 = tseg + 1 - (sampl_pt * (tseg + 1)) / 1000;
	if (*tseg2 < btc->tseg2_min)
		*tseg2 = btc->tseg2_min;
	if (*tseg2 > btc->tseg2_max)
		*tseg2 = btc->tseg2_max;
	*tseg1 = tseg - *tseg2;
	if (*tseg1 > btc->tseg1_max) {
		*tseg1 = btc->tseg1_max;
		*tseg2 = tseg - *tseg1;
	}
	return 1000 * (tseg + 1 - *tseg2) / (tseg + 1);
}

static int can_calc_bittiming(struct net_device *dev, struct can_bittiming *bt)
{
	struct can_priv *priv = netdev_priv(dev);
	const struct can_bittiming_const *btc = priv->bittiming_const;
	long rate, best_rate = 0;
	long best_error = 1000000000, error = 0;
	int best_tseg = 0, best_brp = 0, brp = 0;
	int tsegall, tseg = 0, tseg1 = 0, tseg2 = 0;
	int spt_error = 1000, spt = 0, sampl_pt;
	u64 v64;

	if (!priv->bittiming_const)
		return -ENOTSUPP;

	
	if (bt->sample_point) {
		sampl_pt = bt->sample_point;
	} else {
		if (bt->bitrate > 800000)
			sampl_pt = 750;
		else if (bt->bitrate > 500000)
			sampl_pt = 800;
		else
			sampl_pt = 875;
	}

	
	for (tseg = (btc->tseg1_max + btc->tseg2_max) * 2 + 1;
	     tseg >= (btc->tseg1_min + btc->tseg2_min) * 2; tseg--) {
		tsegall = 1 + tseg / 2;
		
		brp = priv->clock.freq / (tsegall * bt->bitrate) + tseg % 2;
		
		brp = (brp / btc->brp_inc) * btc->brp_inc;
		if ((brp < btc->brp_min) || (brp > btc->brp_max))
			continue;
		rate = priv->clock.freq / (brp * tsegall);
		error = bt->bitrate - rate;
		
		if (error < 0)
			error = -error;
		if (error > best_error)
			continue;
		best_error = error;
		if (error == 0) {
			spt = can_update_spt(btc, sampl_pt, tseg / 2,
					     &tseg1, &tseg2);
			error = sampl_pt - spt;
			if (error < 0)
				error = -error;
			if (error > spt_error)
				continue;
			spt_error = error;
		}
		best_tseg = tseg / 2;
		best_brp = brp;
		best_rate = rate;
		if (error == 0)
			break;
	}

	if (best_error) {
		
		error = (best_error * 1000) / bt->bitrate;
		if (error > CAN_CALC_MAX_ERROR) {
			dev_err(dev->dev.parent,
				"bitrate error %ld.%ld%% too high\n",
				error / 10, error % 10);
			return -EDOM;
		} else {
			dev_warn(dev->dev.parent, "bitrate error %ld.%ld%%\n",
				 error / 10, error % 10);
		}
	}

	
	bt->sample_point = can_update_spt(btc, sampl_pt, best_tseg,
					  &tseg1, &tseg2);

	v64 = (u64)best_brp * 1000000000UL;
	do_div(v64, priv->clock.freq);
	bt->tq = (u32)v64;
	bt->prop_seg = tseg1 / 2;
	bt->phase_seg1 = tseg1 - bt->prop_seg;
	bt->phase_seg2 = tseg2;
	bt->sjw = 1;
	bt->brp = best_brp;
	
	bt->bitrate = priv->clock.freq / (bt->brp * (tseg1 + tseg2 + 1));

	return 0;
}
#else 
static int can_calc_bittiming(struct net_device *dev, struct can_bittiming *bt)
{
	dev_err(dev->dev.parent, "bit-timing calculation not available\n");
	return -EINVAL;
}
#endif 


static int can_fixup_bittiming(struct net_device *dev, struct can_bittiming *bt)
{
	struct can_priv *priv = netdev_priv(dev);
	const struct can_bittiming_const *btc = priv->bittiming_const;
	int tseg1, alltseg;
	u64 brp64;

	if (!priv->bittiming_const)
		return -ENOTSUPP;

	tseg1 = bt->prop_seg + bt->phase_seg1;
	if (!bt->sjw)
		bt->sjw = 1;
	if (bt->sjw > btc->sjw_max ||
	    tseg1 < btc->tseg1_min || tseg1 > btc->tseg1_max ||
	    bt->phase_seg2 < btc->tseg2_min || bt->phase_seg2 > btc->tseg2_max)
		return -ERANGE;

	brp64 = (u64)priv->clock.freq * (u64)bt->tq;
	if (btc->brp_inc > 1)
		do_div(brp64, btc->brp_inc);
	brp64 += 500000000UL - 1;
	do_div(brp64, 1000000000UL); 
	if (btc->brp_inc > 1)
		brp64 *= btc->brp_inc;
	bt->brp = (u32)brp64;

	if (bt->brp < btc->brp_min || bt->brp > btc->brp_max)
		return -EINVAL;

	alltseg = bt->prop_seg + bt->phase_seg1 + bt->phase_seg2 + 1;
	bt->bitrate = priv->clock.freq / (bt->brp * alltseg);
	bt->sample_point = ((tseg1 + 1) * 1000) / alltseg;

	return 0;
}

int can_get_bittiming(struct net_device *dev, struct can_bittiming *bt)
{
	struct can_priv *priv = netdev_priv(dev);
	int err;

	
	if (priv->bittiming_const) {

		
		if (!bt->tq)
			
			err = can_calc_bittiming(dev, bt);
		else
			
			err = can_fixup_bittiming(dev, bt);
		if (err)
			return err;
	}

	return 0;
}


static void can_flush_echo_skb(struct net_device *dev)
{
	struct can_priv *priv = netdev_priv(dev);
	struct net_device_stats *stats = &dev->stats;
	int i;

	for (i = 0; i < CAN_ECHO_SKB_MAX; i++) {
		if (priv->echo_skb[i]) {
			kfree_skb(priv->echo_skb[i]);
			priv->echo_skb[i] = NULL;
			stats->tx_dropped++;
			stats->tx_aborted_errors++;
		}
	}
}


void can_put_echo_skb(struct sk_buff *skb, struct net_device *dev, int idx)
{
	struct can_priv *priv = netdev_priv(dev);

	
	if (!(dev->flags & IFF_ECHO) || skb->pkt_type != PACKET_LOOPBACK) {
		kfree_skb(skb);
		return;
	}

	if (!priv->echo_skb[idx]) {
		struct sock *srcsk = skb->sk;

		if (atomic_read(&skb->users) != 1) {
			struct sk_buff *old_skb = skb;

			skb = skb_clone(old_skb, GFP_ATOMIC);
			kfree_skb(old_skb);
			if (!skb)
				return;
		} else
			skb_orphan(skb);

		skb->sk = srcsk;

		
		skb->protocol = htons(ETH_P_CAN);
		skb->pkt_type = PACKET_BROADCAST;
		skb->ip_summed = CHECKSUM_UNNECESSARY;
		skb->dev = dev;

		
		priv->echo_skb[idx] = skb;
	} else {
		
		dev_err(dev->dev.parent, "%s: BUG! echo_skb is occupied!\n",
			__func__);
		kfree_skb(skb);
	}
}
EXPORT_SYMBOL_GPL(can_put_echo_skb);


void can_get_echo_skb(struct net_device *dev, int idx)
{
	struct can_priv *priv = netdev_priv(dev);

	if (priv->echo_skb[idx]) {
		netif_rx(priv->echo_skb[idx]);
		priv->echo_skb[idx] = NULL;
	}
}
EXPORT_SYMBOL_GPL(can_get_echo_skb);


void can_free_echo_skb(struct net_device *dev, int idx)
{
	struct can_priv *priv = netdev_priv(dev);

	if (priv->echo_skb[idx]) {
		kfree_skb(priv->echo_skb[idx]);
		priv->echo_skb[idx] = NULL;
	}
}
EXPORT_SYMBOL_GPL(can_free_echo_skb);


void can_restart(unsigned long data)
{
	struct net_device *dev = (struct net_device *)data;
	struct can_priv *priv = netdev_priv(dev);
	struct net_device_stats *stats = &dev->stats;
	struct sk_buff *skb;
	struct can_frame *cf;
	int err;

	BUG_ON(netif_carrier_ok(dev));

	
	can_flush_echo_skb(dev);

	
	skb = dev_alloc_skb(sizeof(struct can_frame));
	if (skb == NULL) {
		err = -ENOMEM;
		goto restart;
	}
	skb->dev = dev;
	skb->protocol = htons(ETH_P_CAN);
	cf = (struct can_frame *)skb_put(skb, sizeof(struct can_frame));
	memset(cf, 0, sizeof(struct can_frame));
	cf->can_id = CAN_ERR_FLAG | CAN_ERR_RESTARTED;
	cf->can_dlc = CAN_ERR_DLC;

	netif_rx(skb);

	stats->rx_packets++;
	stats->rx_bytes += cf->can_dlc;

restart:
	dev_dbg(dev->dev.parent, "restarted\n");
	priv->can_stats.restarts++;

	
	err = priv->do_set_mode(dev, CAN_MODE_START);

	netif_carrier_on(dev);
	if (err)
		dev_err(dev->dev.parent, "Error %d during restart", err);
}

int can_restart_now(struct net_device *dev)
{
	struct can_priv *priv = netdev_priv(dev);

	
	if (priv->restart_ms)
		return -EINVAL;
	if (priv->state != CAN_STATE_BUS_OFF)
		return -EBUSY;

	
	mod_timer(&priv->restart_timer, jiffies);

	return 0;
}


void can_bus_off(struct net_device *dev)
{
	struct can_priv *priv = netdev_priv(dev);

	dev_dbg(dev->dev.parent, "bus-off\n");

	netif_carrier_off(dev);
	priv->can_stats.bus_off++;

	if (priv->restart_ms)
		mod_timer(&priv->restart_timer,
			  jiffies + (priv->restart_ms * HZ) / 1000);
}
EXPORT_SYMBOL_GPL(can_bus_off);

static void can_setup(struct net_device *dev)
{
	dev->type = ARPHRD_CAN;
	dev->mtu = sizeof(struct can_frame);
	dev->hard_header_len = 0;
	dev->addr_len = 0;
	dev->tx_queue_len = 10;

	
	dev->flags = IFF_NOARP;
	dev->features = NETIF_F_NO_CSUM;
}


struct net_device *alloc_candev(int sizeof_priv)
{
	struct net_device *dev;
	struct can_priv *priv;

	dev = alloc_netdev(sizeof_priv, "can%d", can_setup);
	if (!dev)
		return NULL;

	priv = netdev_priv(dev);

	priv->state = CAN_STATE_STOPPED;

	init_timer(&priv->restart_timer);

	return dev;
}
EXPORT_SYMBOL_GPL(alloc_candev);


void free_candev(struct net_device *dev)
{
	free_netdev(dev);
}
EXPORT_SYMBOL_GPL(free_candev);


int open_candev(struct net_device *dev)
{
	struct can_priv *priv = netdev_priv(dev);

	if (!priv->bittiming.tq && !priv->bittiming.bitrate) {
		dev_err(dev->dev.parent, "bit-timing not yet defined\n");
		return -EINVAL;
	}

	
	if (!netif_carrier_ok(dev))
		netif_carrier_on(dev);

	setup_timer(&priv->restart_timer, can_restart, (unsigned long)dev);

	return 0;
}
EXPORT_SYMBOL_GPL(open_candev);


void close_candev(struct net_device *dev)
{
	struct can_priv *priv = netdev_priv(dev);

	if (del_timer_sync(&priv->restart_timer))
		dev_put(dev);
	can_flush_echo_skb(dev);
}
EXPORT_SYMBOL_GPL(close_candev);


static const struct nla_policy can_policy[IFLA_CAN_MAX + 1] = {
	[IFLA_CAN_STATE]	= { .type = NLA_U32 },
	[IFLA_CAN_CTRLMODE]	= { .len = sizeof(struct can_ctrlmode) },
	[IFLA_CAN_RESTART_MS]	= { .type = NLA_U32 },
	[IFLA_CAN_RESTART]	= { .type = NLA_U32 },
	[IFLA_CAN_BITTIMING]	= { .len = sizeof(struct can_bittiming) },
	[IFLA_CAN_BITTIMING_CONST]
				= { .len = sizeof(struct can_bittiming_const) },
	[IFLA_CAN_CLOCK]	= { .len = sizeof(struct can_clock) },
};

static int can_changelink(struct net_device *dev,
			  struct nlattr *tb[], struct nlattr *data[])
{
	struct can_priv *priv = netdev_priv(dev);
	int err;

	
	ASSERT_RTNL();

	if (data[IFLA_CAN_CTRLMODE]) {
		struct can_ctrlmode *cm;

		
		if (dev->flags & IFF_UP)
			return -EBUSY;
		cm = nla_data(data[IFLA_CAN_CTRLMODE]);
		priv->ctrlmode &= ~cm->mask;
		priv->ctrlmode |= cm->flags;
	}

	if (data[IFLA_CAN_BITTIMING]) {
		struct can_bittiming bt;

		
		if (dev->flags & IFF_UP)
			return -EBUSY;
		memcpy(&bt, nla_data(data[IFLA_CAN_BITTIMING]), sizeof(bt));
		if ((!bt.bitrate && !bt.tq) || (bt.bitrate && bt.tq))
			return -EINVAL;
		err = can_get_bittiming(dev, &bt);
		if (err)
			return err;
		memcpy(&priv->bittiming, &bt, sizeof(bt));

		if (priv->do_set_bittiming) {
			
			err = priv->do_set_bittiming(dev);
			if (err)
				return err;
		}
	}

	if (data[IFLA_CAN_RESTART_MS]) {
		
		if (dev->flags & IFF_UP)
			return -EBUSY;
		priv->restart_ms = nla_get_u32(data[IFLA_CAN_RESTART_MS]);
	}

	if (data[IFLA_CAN_RESTART]) {
		
		if (!(dev->flags & IFF_UP))
			return -EINVAL;
		err = can_restart_now(dev);
		if (err)
			return err;
	}

	return 0;
}

static size_t can_get_size(const struct net_device *dev)
{
	struct can_priv *priv = netdev_priv(dev);
	size_t size;

	size = nla_total_size(sizeof(u32));   
	size += sizeof(struct can_ctrlmode);  
	size += nla_total_size(sizeof(u32));  
	size += sizeof(struct can_bittiming); 
	size += sizeof(struct can_clock);     
	if (priv->bittiming_const)	      
		size += sizeof(struct can_bittiming_const);

	return size;
}

static int can_fill_info(struct sk_buff *skb, const struct net_device *dev)
{
	struct can_priv *priv = netdev_priv(dev);
	struct can_ctrlmode cm = {.flags = priv->ctrlmode};
	enum can_state state = priv->state;

	if (priv->do_get_state)
		priv->do_get_state(dev, &state);
	NLA_PUT_U32(skb, IFLA_CAN_STATE, state);
	NLA_PUT(skb, IFLA_CAN_CTRLMODE, sizeof(cm), &cm);
	NLA_PUT_U32(skb, IFLA_CAN_RESTART_MS, priv->restart_ms);
	NLA_PUT(skb, IFLA_CAN_BITTIMING,
		sizeof(priv->bittiming), &priv->bittiming);
	NLA_PUT(skb, IFLA_CAN_CLOCK, sizeof(cm), &priv->clock);
	if (priv->bittiming_const)
		NLA_PUT(skb, IFLA_CAN_BITTIMING_CONST,
			sizeof(*priv->bittiming_const), priv->bittiming_const);

	return 0;

nla_put_failure:
	return -EMSGSIZE;
}

static size_t can_get_xstats_size(const struct net_device *dev)
{
	return sizeof(struct can_device_stats);
}

static int can_fill_xstats(struct sk_buff *skb, const struct net_device *dev)
{
	struct can_priv *priv = netdev_priv(dev);

	NLA_PUT(skb, IFLA_INFO_XSTATS,
		sizeof(priv->can_stats), &priv->can_stats);

	return 0;

nla_put_failure:
	return -EMSGSIZE;
}

static int can_newlink(struct net_device *dev,
		       struct nlattr *tb[], struct nlattr *data[])
{
	return -EOPNOTSUPP;
}

static struct rtnl_link_ops can_link_ops __read_mostly = {
	.kind		= "can",
	.maxtype	= IFLA_CAN_MAX,
	.policy		= can_policy,
	.setup		= can_setup,
	.newlink	= can_newlink,
	.changelink	= can_changelink,
	.get_size	= can_get_size,
	.fill_info	= can_fill_info,
	.get_xstats_size = can_get_xstats_size,
	.fill_xstats	= can_fill_xstats,
};


int register_candev(struct net_device *dev)
{
	dev->rtnl_link_ops = &can_link_ops;
	return register_netdev(dev);
}
EXPORT_SYMBOL_GPL(register_candev);


void unregister_candev(struct net_device *dev)
{
	unregister_netdev(dev);
}
EXPORT_SYMBOL_GPL(unregister_candev);

static __init int can_dev_init(void)
{
	int err;

	err = rtnl_link_register(&can_link_ops);
	if (!err)
		printk(KERN_INFO MOD_DESC "\n");

	return err;
}
module_init(can_dev_init);

static __exit void can_dev_exit(void)
{
	rtnl_link_unregister(&can_link_ops);
}
module_exit(can_dev_exit);

MODULE_ALIAS_RTNL_LINK("can");
