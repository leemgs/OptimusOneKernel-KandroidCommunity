

#include <linux/module.h>

#include <linux/socket.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/wait.h>

#include <asm/unaligned.h>

#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>
#include <net/bluetooth/l2cap.h>

#include "bnep.h"

#define BNEP_TX_QUEUE_LEN 20

static int bnep_net_open(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}

static int bnep_net_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

static void bnep_net_set_mc_list(struct net_device *dev)
{
#ifdef CONFIG_BT_BNEP_MC_FILTER
	struct bnep_session *s = netdev_priv(dev);
	struct sock *sk = s->sock->sk;
	struct bnep_set_filter_req *r;
	struct sk_buff *skb;
	int size;

	BT_DBG("%s mc_count %d", dev->name, dev->mc_count);

	size = sizeof(*r) + (BNEP_MAX_MULTICAST_FILTERS + 1) * ETH_ALEN * 2;
	skb  = alloc_skb(size, GFP_ATOMIC);
	if (!skb) {
		BT_ERR("%s Multicast list allocation failed", dev->name);
		return;
	}

	r = (void *) skb->data;
	__skb_put(skb, sizeof(*r));

	r->type = BNEP_CONTROL;
	r->ctrl = BNEP_FILTER_MULTI_ADDR_SET;

	if (dev->flags & (IFF_PROMISC | IFF_ALLMULTI)) {
		u8 start[ETH_ALEN] = { 0x01 };

		
		memcpy(__skb_put(skb, ETH_ALEN), start, ETH_ALEN);
		memcpy(__skb_put(skb, ETH_ALEN), dev->broadcast, ETH_ALEN);
		r->len = htons(ETH_ALEN * 2);
	} else {
		struct dev_mc_list *dmi = dev->mc_list;
		int i, len = skb->len;

		if (dev->flags & IFF_BROADCAST) {
			memcpy(__skb_put(skb, ETH_ALEN), dev->broadcast, ETH_ALEN);
			memcpy(__skb_put(skb, ETH_ALEN), dev->broadcast, ETH_ALEN);
		}

		

		for (i = 0; i < dev->mc_count && i < BNEP_MAX_MULTICAST_FILTERS; i++) {
			memcpy(__skb_put(skb, ETH_ALEN), dmi->dmi_addr, ETH_ALEN);
			memcpy(__skb_put(skb, ETH_ALEN), dmi->dmi_addr, ETH_ALEN);
			dmi = dmi->next;
		}
		r->len = htons(skb->len - len);
	}

	skb_queue_tail(&sk->sk_write_queue, skb);
	wake_up_interruptible(sk->sk_sleep);
#endif
}

static int bnep_net_set_mac_addr(struct net_device *dev, void *arg)
{
	BT_DBG("%s", dev->name);
	return 0;
}

static void bnep_net_timeout(struct net_device *dev)
{
	BT_DBG("net_timeout");
	netif_wake_queue(dev);
}

#ifdef CONFIG_BT_BNEP_MC_FILTER
static inline int bnep_net_mc_filter(struct sk_buff *skb, struct bnep_session *s)
{
	struct ethhdr *eh = (void *) skb->data;

	if ((eh->h_dest[0] & 1) && !test_bit(bnep_mc_hash(eh->h_dest), (ulong *) &s->mc_filter))
		return 1;
	return 0;
}
#endif

#ifdef CONFIG_BT_BNEP_PROTO_FILTER

static inline u16 bnep_net_eth_proto(struct sk_buff *skb)
{
	struct ethhdr *eh = (void *) skb->data;
	u16 proto = ntohs(eh->h_proto);

	if (proto >= 1536)
		return proto;

	if (get_unaligned((__be16 *) skb->data) == htons(0xFFFF))
		return ETH_P_802_3;

	return ETH_P_802_2;
}

static inline int bnep_net_proto_filter(struct sk_buff *skb, struct bnep_session *s)
{
	u16 proto = bnep_net_eth_proto(skb);
	struct bnep_proto_filter *f = s->proto_filter;
	int i;

	for (i = 0; i < BNEP_MAX_PROTO_FILTERS && f[i].end; i++) {
		if (proto >= f[i].start && proto <= f[i].end)
			return 0;
	}

	BT_DBG("BNEP: filtered skb %p, proto 0x%.4x", skb, proto);
	return 1;
}
#endif

static netdev_tx_t bnep_net_xmit(struct sk_buff *skb,
				 struct net_device *dev)
{
	struct bnep_session *s = netdev_priv(dev);
	struct sock *sk = s->sock->sk;

	BT_DBG("skb %p, dev %p", skb, dev);

#ifdef CONFIG_BT_BNEP_MC_FILTER
	if (bnep_net_mc_filter(skb, s)) {
		kfree_skb(skb);
		return NETDEV_TX_OK;
	}
#endif

#ifdef CONFIG_BT_BNEP_PROTO_FILTER
	if (bnep_net_proto_filter(skb, s)) {
		kfree_skb(skb);
		return NETDEV_TX_OK;
	}
#endif

	
	dev->trans_start = jiffies;
	skb_queue_tail(&sk->sk_write_queue, skb);
	wake_up_interruptible(sk->sk_sleep);

	if (skb_queue_len(&sk->sk_write_queue) >= BNEP_TX_QUEUE_LEN) {
		BT_DBG("tx queue is full");

		
		netif_stop_queue(dev);
	}

	return NETDEV_TX_OK;
}

static const struct net_device_ops bnep_netdev_ops = {
	.ndo_open            = bnep_net_open,
	.ndo_stop            = bnep_net_close,
	.ndo_start_xmit	     = bnep_net_xmit,
	.ndo_validate_addr   = eth_validate_addr,
	.ndo_set_multicast_list = bnep_net_set_mc_list,
	.ndo_set_mac_address = bnep_net_set_mac_addr,
	.ndo_tx_timeout      = bnep_net_timeout,
	.ndo_change_mtu	     = eth_change_mtu,

};

void bnep_net_setup(struct net_device *dev)
{

	memset(dev->broadcast, 0xff, ETH_ALEN);
	dev->addr_len = ETH_ALEN;

	ether_setup(dev);
	dev->netdev_ops = &bnep_netdev_ops;

	dev->watchdog_timeo  = HZ * 2;
}
