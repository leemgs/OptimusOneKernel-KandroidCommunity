

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>
#include <linux/if_arp.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <net/arp.h>

#include <net/irda/irda.h>
#include <net/irda/irmod.h>
#include <net/irda/irlan_common.h>
#include <net/irda/irlan_client.h>
#include <net/irda/irlan_event.h>
#include <net/irda/irlan_eth.h>

static int  irlan_eth_open(struct net_device *dev);
static int  irlan_eth_close(struct net_device *dev);
static netdev_tx_t  irlan_eth_xmit(struct sk_buff *skb,
					 struct net_device *dev);
static void irlan_eth_set_multicast_list( struct net_device *dev);
static struct net_device_stats *irlan_eth_get_stats(struct net_device *dev);

static const struct net_device_ops irlan_eth_netdev_ops = {
	.ndo_open               = irlan_eth_open,
	.ndo_stop               = irlan_eth_close,
	.ndo_start_xmit    	= irlan_eth_xmit,
	.ndo_get_stats	        = irlan_eth_get_stats,
	.ndo_set_multicast_list = irlan_eth_set_multicast_list,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_validate_addr	= eth_validate_addr,
};


static void irlan_eth_setup(struct net_device *dev)
{
	ether_setup(dev);

	dev->netdev_ops		= &irlan_eth_netdev_ops;
	dev->destructor		= free_netdev;


	
	
	dev->tx_queue_len = 4;
}


struct net_device *alloc_irlandev(const char *name)
{
	return alloc_netdev(sizeof(struct irlan_cb), name,
			    irlan_eth_setup);
}


static int irlan_eth_open(struct net_device *dev)
{
	struct irlan_cb *self = netdev_priv(dev);

	IRDA_DEBUG(2, "%s()\n", __func__ );

	
	netif_stop_queue(dev); 

	
	self->disconnect_reason = 0;
	irlan_client_wakeup(self, self->saddr, self->daddr);

	
	return wait_event_interruptible(self->open_wait,
					!self->tsap_data->connected);
}


static int irlan_eth_close(struct net_device *dev)
{
	struct irlan_cb *self = netdev_priv(dev);

	IRDA_DEBUG(2, "%s()\n", __func__ );

	
	netif_stop_queue(dev);

	irlan_close_data_channel(self);
	irlan_close_tsaps(self);

	irlan_do_client_event(self, IRLAN_LMP_DISCONNECT, NULL);
	irlan_do_provider_event(self, IRLAN_LMP_DISCONNECT, NULL);

	
	skb_queue_purge(&self->client.txq);

	self->client.tx_busy = 0;

	return 0;
}


static netdev_tx_t irlan_eth_xmit(struct sk_buff *skb,
					struct net_device *dev)
{
	struct irlan_cb *self = netdev_priv(dev);
	int ret;

	
	if ((skb_headroom(skb) < self->max_header_size) || (skb_shared(skb))) {
		struct sk_buff *new_skb =
			skb_realloc_headroom(skb, self->max_header_size);

		
		dev_kfree_skb(skb);

		
		if (new_skb == NULL)
			return NETDEV_TX_OK;

		
		skb = new_skb;
	}

	dev->trans_start = jiffies;

	
	if (self->use_udata)
		ret = irttp_udata_request(self->tsap_data, skb);
	else
		ret = irttp_data_request(self->tsap_data, skb);

	if (ret < 0) {
		
		
		self->stats.tx_dropped++;
	} else {
		self->stats.tx_packets++;
		self->stats.tx_bytes += skb->len;
	}

	return NETDEV_TX_OK;
}


int irlan_eth_receive(void *instance, void *sap, struct sk_buff *skb)
{
	struct irlan_cb *self = instance;

	if (skb == NULL) {
		++self->stats.rx_dropped;
		return 0;
	}
	if (skb->len < ETH_HLEN) {
		IRDA_DEBUG(0, "%s() : IrLAN frame too short (%d)\n",
			   __func__, skb->len);
		++self->stats.rx_dropped;
		dev_kfree_skb(skb);
		return 0;
	}

	
	skb->protocol = eth_type_trans(skb, self->dev); 

	self->stats.rx_packets++;
	self->stats.rx_bytes += skb->len;

	netif_rx(skb);   

	return 0;
}


void irlan_eth_flow_indication(void *instance, void *sap, LOCAL_FLOW flow)
{
	struct irlan_cb *self;
	struct net_device *dev;

	self = (struct irlan_cb *) instance;

	IRDA_ASSERT(self != NULL, return;);
	IRDA_ASSERT(self->magic == IRLAN_MAGIC, return;);

	dev = self->dev;

	IRDA_ASSERT(dev != NULL, return;);

	IRDA_DEBUG(0, "%s() : flow %s ; running %d\n", __func__,
		   flow == FLOW_STOP ? "FLOW_STOP" : "FLOW_START",
		   netif_running(dev));

	switch (flow) {
	case FLOW_STOP:
		
		netif_stop_queue(dev);
		break;
	case FLOW_START:
	default:
		
		
		netif_wake_queue(dev);
		break;
	}
}


#define HW_MAX_ADDRS 4 
static void irlan_eth_set_multicast_list(struct net_device *dev)
{
	struct irlan_cb *self = netdev_priv(dev);

	IRDA_DEBUG(2, "%s()\n", __func__ );

	
	if (self->client.state != IRLAN_DATA) {
		IRDA_DEBUG(1, "%s(), delaying!\n", __func__ );
		return;
	}

	if (dev->flags & IFF_PROMISC) {
		
		IRDA_WARNING("Promiscuous mode not implemented by IrLAN!\n");
	}
	else if ((dev->flags & IFF_ALLMULTI) || dev->mc_count > HW_MAX_ADDRS) {
		
		IRDA_DEBUG(4, "%s(), Setting multicast filter\n", __func__ );
		

		irlan_set_multicast_filter(self, TRUE);
	}
	else if (dev->mc_count) {
		IRDA_DEBUG(4, "%s(), Setting multicast filter\n", __func__ );
		
		

		irlan_set_multicast_filter(self, TRUE);
	}
	else {
		IRDA_DEBUG(4, "%s(), Clearing multicast filter\n", __func__ );
		irlan_set_multicast_filter(self, FALSE);
	}

	if (dev->flags & IFF_BROADCAST)
		irlan_set_broadcast_filter(self, TRUE);
	else
		irlan_set_broadcast_filter(self, FALSE);
}


static struct net_device_stats *irlan_eth_get_stats(struct net_device *dev)
{
	struct irlan_cb *self = netdev_priv(dev);

	return &self->stats;
}
