
#include <linux/if_arp.h>
#include <linux/netdevice.h>
#include "i2400m.h"


#define D_SUBMODULE netdev
#include "debug-levels.h"

enum {

	
	I2400M_MAX_MTU = 1400,
	I2400M_TX_TIMEOUT = HZ,
	I2400M_TX_QLEN = 5,
};


static
int i2400m_open(struct net_device *net_dev)
{
	int result;
	struct i2400m *i2400m = net_dev_to_i2400m(net_dev);
	struct device *dev = i2400m_dev(i2400m);

	d_fnstart(3, dev, "(net_dev %p [i2400m %p])\n", net_dev, i2400m);
	if (i2400m->ready == 0) {
		dev_err(dev, "Device is still initializing\n");
		result = -EBUSY;
	} else
		result = 0;
	d_fnend(3, dev, "(net_dev %p [i2400m %p]) = %d\n",
		net_dev, i2400m, result);
	return result;
}



static
int i2400m_stop(struct net_device *net_dev)
{
	struct i2400m *i2400m = net_dev_to_i2400m(net_dev);
	struct device *dev = i2400m_dev(i2400m);

	d_fnstart(3, dev, "(net_dev %p [i2400m %p])\n", net_dev, i2400m);
	
	if (cancel_work_sync(&i2400m->wake_tx_ws) == 0
	    && i2400m->wake_tx_skb != NULL) {
		unsigned long flags;
		struct sk_buff *wake_tx_skb;
		spin_lock_irqsave(&i2400m->tx_lock, flags);
		wake_tx_skb = i2400m->wake_tx_skb;	
		i2400m->wake_tx_skb = NULL;	
		spin_unlock_irqrestore(&i2400m->tx_lock, flags);
		i2400m_put(i2400m);
		kfree_skb(wake_tx_skb);
	}
	d_fnend(3, dev, "(net_dev %p [i2400m %p]) = 0\n", net_dev, i2400m);
	return 0;
}



void i2400m_wake_tx_work(struct work_struct *ws)
{
	int result;
	struct i2400m *i2400m = container_of(ws, struct i2400m, wake_tx_ws);
	struct device *dev = i2400m_dev(i2400m);
	struct sk_buff *skb = i2400m->wake_tx_skb;
	unsigned long flags;

	spin_lock_irqsave(&i2400m->tx_lock, flags);
	skb = i2400m->wake_tx_skb;
	i2400m->wake_tx_skb = NULL;
	spin_unlock_irqrestore(&i2400m->tx_lock, flags);

	d_fnstart(3, dev, "(ws %p i2400m %p skb %p)\n", ws, i2400m, skb);
	result = -EINVAL;
	if (skb == NULL) {
		dev_err(dev, "WAKE&TX: skb dissapeared!\n");
		goto out_put;
	}
	result = i2400m_cmd_exit_idle(i2400m);
	if (result == -EILSEQ)
		result = 0;
	if (result < 0) {
		dev_err(dev, "WAKE&TX: device didn't get out of idle: "
			"%d\n", result);
			goto error;
	}
	result = wait_event_timeout(i2400m->state_wq,
				    i2400m->state != I2400M_SS_IDLE, 5 * HZ);
	if (result == 0)
		result = -ETIMEDOUT;
	if (result < 0) {
		dev_err(dev, "WAKE&TX: error waiting for device to exit IDLE: "
			"%d\n", result);
		goto error;
	}
	msleep(20);	
	result = i2400m_tx(i2400m, skb->data, skb->len, I2400M_PT_DATA);
	netif_wake_queue(i2400m->wimax_dev.net_dev);
error:
	kfree_skb(skb);	
out_put:
	i2400m_put(i2400m);
	d_fnend(3, dev, "(ws %p i2400m %p skb %p) = void [%d]\n",
		ws, i2400m, skb, result);
}



static
void i2400m_tx_prep_header(struct sk_buff *skb)
{
	struct i2400m_pl_data_hdr *pl_hdr;
	skb_pull(skb, ETH_HLEN);
	pl_hdr = (struct i2400m_pl_data_hdr *) skb_push(skb, sizeof(*pl_hdr));
	pl_hdr->reserved = 0;
}



static
int i2400m_net_wake_tx(struct i2400m *i2400m, struct net_device *net_dev,
		       struct sk_buff *skb)
{
	int result;
	struct device *dev = i2400m_dev(i2400m);
	unsigned long flags;

	d_fnstart(3, dev, "(skb %p net_dev %p)\n", skb, net_dev);
	if (net_ratelimit()) {
		d_printf(3, dev, "WAKE&NETTX: "
			 "skb %p sending %d bytes to radio\n",
			 skb, skb->len);
		d_dump(4, dev, skb->data, skb->len);
	}
	
	result = 0;
	spin_lock_irqsave(&i2400m->tx_lock, flags);
	if (!work_pending(&i2400m->wake_tx_ws)) {
		netif_stop_queue(net_dev);
		i2400m_get(i2400m);
		i2400m->wake_tx_skb = skb_get(skb);	
		i2400m_tx_prep_header(skb);
		result = schedule_work(&i2400m->wake_tx_ws);
		WARN_ON(result == 0);
	}
	spin_unlock_irqrestore(&i2400m->tx_lock, flags);
	if (result == 0) {
		
		if (net_ratelimit())
			d_printf(1, dev, "NETTX: device exiting idle, "
				 "dropping skb %p, queue running %d\n",
				 skb, netif_queue_stopped(net_dev));
		result = -EBUSY;
	}
	d_fnend(3, dev, "(skb %p net_dev %p) = %d\n", skb, net_dev, result);
	return result;
}



static
int i2400m_net_tx(struct i2400m *i2400m, struct net_device *net_dev,
		  struct sk_buff *skb)
{
	int result;
	struct device *dev = i2400m_dev(i2400m);

	d_fnstart(3, dev, "(i2400m %p net_dev %p skb %p)\n",
		  i2400m, net_dev, skb);
	
	net_dev->trans_start = jiffies;
	i2400m_tx_prep_header(skb);
	d_printf(3, dev, "NETTX: skb %p sending %d bytes to radio\n",
		 skb, skb->len);
	d_dump(4, dev, skb->data, skb->len);
	result = i2400m_tx(i2400m, skb->data, skb->len, I2400M_PT_DATA);
	d_fnend(3, dev, "(i2400m %p net_dev %p skb %p) = %d\n",
		i2400m, net_dev, skb, result);
	return result;
}



static
netdev_tx_t i2400m_hard_start_xmit(struct sk_buff *skb,
					 struct net_device *net_dev)
{
	struct i2400m *i2400m = net_dev_to_i2400m(net_dev);
	struct device *dev = i2400m_dev(i2400m);
	int result;

	d_fnstart(3, dev, "(skb %p net_dev %p)\n", skb, net_dev);
	if (i2400m->state == I2400M_SS_IDLE)
		result = i2400m_net_wake_tx(i2400m, net_dev, skb);
	else
		result = i2400m_net_tx(i2400m, net_dev, skb);
	if (result <  0)
		net_dev->stats.tx_dropped++;
	else {
		net_dev->stats.tx_packets++;
		net_dev->stats.tx_bytes += skb->len;
	}
	kfree_skb(skb);

	d_fnend(3, dev, "(skb %p net_dev %p)\n", skb, net_dev);
	return NETDEV_TX_OK;
}


static
int i2400m_change_mtu(struct net_device *net_dev, int new_mtu)
{
	int result;
	struct i2400m *i2400m = net_dev_to_i2400m(net_dev);
	struct device *dev = i2400m_dev(i2400m);

	if (new_mtu >= I2400M_MAX_MTU) {
		dev_err(dev, "Cannot change MTU to %d (max is %d)\n",
			new_mtu, I2400M_MAX_MTU);
		result = -EINVAL;
	} else {
		net_dev->mtu = new_mtu;
		result = 0;
	}
	return result;
}


static
void i2400m_tx_timeout(struct net_device *net_dev)
{
	
	net_dev->stats.tx_errors++;
	return;
}



static
void i2400m_rx_fake_eth_header(struct net_device *net_dev,
			       void *_eth_hdr, __be16 protocol)
{
	struct i2400m *i2400m = net_dev_to_i2400m(net_dev);
	struct ethhdr *eth_hdr = _eth_hdr;

	memcpy(eth_hdr->h_dest, net_dev->dev_addr, sizeof(eth_hdr->h_dest));
	memcpy(eth_hdr->h_source, i2400m->src_mac_addr,
	       sizeof(eth_hdr->h_source));
	eth_hdr->h_proto = protocol;
}



void i2400m_net_rx(struct i2400m *i2400m, struct sk_buff *skb_rx,
		   unsigned i, const void *buf, int buf_len)
{
	struct net_device *net_dev = i2400m->wimax_dev.net_dev;
	struct device *dev = i2400m_dev(i2400m);
	struct sk_buff *skb;

	d_fnstart(2, dev, "(i2400m %p buf %p buf_len %d)\n",
		  i2400m, buf, buf_len);
	if (i) {
		skb = skb_get(skb_rx);
		d_printf(2, dev, "RX: reusing first payload skb %p\n", skb);
		skb_pull(skb, buf - (void *) skb->data);
		skb_trim(skb, (void *) skb_end_pointer(skb) - buf);
	} else {
		
		skb = __netdev_alloc_skb(net_dev, buf_len, GFP_KERNEL);
		if (skb == NULL) {
			dev_err(dev, "NETRX: no memory to realloc skb\n");
			net_dev->stats.rx_dropped++;
			goto error_skb_realloc;
		}
		memcpy(skb_put(skb, buf_len), buf, buf_len);
	}
	i2400m_rx_fake_eth_header(i2400m->wimax_dev.net_dev,
				  skb->data - ETH_HLEN,
				  cpu_to_be16(ETH_P_IP));
	skb_set_mac_header(skb, -ETH_HLEN);
	skb->dev = i2400m->wimax_dev.net_dev;
	skb->protocol = htons(ETH_P_IP);
	net_dev->stats.rx_packets++;
	net_dev->stats.rx_bytes += buf_len;
	d_printf(3, dev, "NETRX: receiving %d bytes to network stack\n",
		buf_len);
	d_dump(4, dev, buf, buf_len);
	netif_rx_ni(skb);	
error_skb_realloc:
	d_fnend(2, dev, "(i2400m %p buf %p buf_len %d) = void\n",
		i2400m, buf, buf_len);
}



void i2400m_net_erx(struct i2400m *i2400m, struct sk_buff *skb,
		    enum i2400m_cs cs)
{
	struct net_device *net_dev = i2400m->wimax_dev.net_dev;
	struct device *dev = i2400m_dev(i2400m);
	int protocol;

	d_fnstart(2, dev, "(i2400m %p skb %p [%u] cs %d)\n",
		  i2400m, skb, skb->len, cs);
	switch(cs) {
	case I2400M_CS_IPV4_0:
	case I2400M_CS_IPV4:
		protocol = ETH_P_IP;
		i2400m_rx_fake_eth_header(i2400m->wimax_dev.net_dev,
					  skb->data - ETH_HLEN,
					  cpu_to_be16(ETH_P_IP));
		skb_set_mac_header(skb, -ETH_HLEN);
		skb->dev = i2400m->wimax_dev.net_dev;
		skb->protocol = htons(ETH_P_IP);
		net_dev->stats.rx_packets++;
		net_dev->stats.rx_bytes += skb->len;
		break;
	default:
		dev_err(dev, "ERX: BUG? CS type %u unsupported\n", cs);
		goto error;

	}
	d_printf(3, dev, "ERX: receiving %d bytes to the network stack\n",
		 skb->len);
	d_dump(4, dev, skb->data, skb->len);
	netif_rx_ni(skb);	
error:
	d_fnend(2, dev, "(i2400m %p skb %p [%u] cs %d) = void\n",
		i2400m, skb, skb->len, cs);
}

static const struct net_device_ops i2400m_netdev_ops = {
	.ndo_open = i2400m_open,
	.ndo_stop = i2400m_stop,
	.ndo_start_xmit = i2400m_hard_start_xmit,
	.ndo_tx_timeout = i2400m_tx_timeout,
	.ndo_change_mtu = i2400m_change_mtu,
};



void i2400m_netdev_setup(struct net_device *net_dev)
{
	d_fnstart(3, NULL, "(net_dev %p)\n", net_dev);
	ether_setup(net_dev);
	net_dev->mtu = I2400M_MAX_MTU;
	net_dev->tx_queue_len = I2400M_TX_QLEN;
	net_dev->features =
		  NETIF_F_VLAN_CHALLENGED
		| NETIF_F_HIGHDMA;
	net_dev->flags =
		IFF_NOARP		
		& (~IFF_BROADCAST	
		   & ~IFF_MULTICAST);
	net_dev->watchdog_timeo = I2400M_TX_TIMEOUT;
	net_dev->netdev_ops = &i2400m_netdev_ops;
	d_fnend(3, NULL, "(net_dev %p) = void\n", net_dev);
}
EXPORT_SYMBOL_GPL(i2400m_netdev_setup);

