
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/in.h>
#include <linux/init.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <net/sock.h>
#include <net/checksum.h>
#include <linux/if_ether.h>	
#include <linux/if_arp.h>	
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/percpu.h>
#include <net/net_namespace.h>

struct pcpu_lstats {
	unsigned long packets;
	unsigned long bytes;
	unsigned long drops;
};


static netdev_tx_t loopback_xmit(struct sk_buff *skb,
				 struct net_device *dev)
{
	struct pcpu_lstats *pcpu_lstats, *lb_stats;
	int len;

	skb_orphan(skb);

	skb->protocol = eth_type_trans(skb, dev);

	
	pcpu_lstats = dev->ml_priv;
	lb_stats = per_cpu_ptr(pcpu_lstats, smp_processor_id());

	len = skb->len;
	if (likely(netif_rx(skb) == NET_RX_SUCCESS)) {
		lb_stats->bytes += len;
		lb_stats->packets++;
	} else
		lb_stats->drops++;

	return NETDEV_TX_OK;
}

static struct net_device_stats *loopback_get_stats(struct net_device *dev)
{
	const struct pcpu_lstats *pcpu_lstats;
	struct net_device_stats *stats = &dev->stats;
	unsigned long bytes = 0;
	unsigned long packets = 0;
	unsigned long drops = 0;
	int i;

	pcpu_lstats = dev->ml_priv;
	for_each_possible_cpu(i) {
		const struct pcpu_lstats *lb_stats;

		lb_stats = per_cpu_ptr(pcpu_lstats, i);
		bytes   += lb_stats->bytes;
		packets += lb_stats->packets;
		drops   += lb_stats->drops;
	}
	stats->rx_packets = packets;
	stats->tx_packets = packets;
	stats->rx_dropped = drops;
	stats->rx_errors  = drops;
	stats->rx_bytes   = bytes;
	stats->tx_bytes   = bytes;
	return stats;
}

static u32 always_on(struct net_device *dev)
{
	return 1;
}

static const struct ethtool_ops loopback_ethtool_ops = {
	.get_link		= always_on,
	.set_tso		= ethtool_op_set_tso,
	.get_tx_csum		= always_on,
	.get_sg			= always_on,
	.get_rx_csum		= always_on,
};

static int loopback_dev_init(struct net_device *dev)
{
	struct pcpu_lstats *lstats;

	lstats = alloc_percpu(struct pcpu_lstats);
	if (!lstats)
		return -ENOMEM;

	dev->ml_priv = lstats;
	return 0;
}

static void loopback_dev_free(struct net_device *dev)
{
	struct pcpu_lstats *lstats = dev->ml_priv;

	free_percpu(lstats);
	free_netdev(dev);
}

static const struct net_device_ops loopback_ops = {
	.ndo_init      = loopback_dev_init,
	.ndo_start_xmit= loopback_xmit,
	.ndo_get_stats = loopback_get_stats,
};


static void loopback_setup(struct net_device *dev)
{
	dev->mtu		= (16 * 1024) + 20 + 20 + 12;
	dev->hard_header_len	= ETH_HLEN;	
	dev->addr_len		= ETH_ALEN;	
	dev->tx_queue_len	= 0;
	dev->type		= ARPHRD_LOOPBACK;	
	dev->flags		= IFF_LOOPBACK;
	dev->priv_flags	       &= ~IFF_XMIT_DST_RELEASE;
	dev->features 		= NETIF_F_SG | NETIF_F_FRAGLIST
		| NETIF_F_TSO
		| NETIF_F_NO_CSUM
		| NETIF_F_HIGHDMA
		| NETIF_F_LLTX
		| NETIF_F_NETNS_LOCAL;
	dev->ethtool_ops	= &loopback_ethtool_ops;
	dev->header_ops		= &eth_header_ops;
	dev->netdev_ops		= &loopback_ops;
	dev->destructor		= loopback_dev_free;
}


static __net_init int loopback_net_init(struct net *net)
{
	struct net_device *dev;
	int err;

	err = -ENOMEM;
	dev = alloc_netdev(0, "lo", loopback_setup);
	if (!dev)
		goto out;

	dev_net_set(dev, net);
	err = register_netdev(dev);
	if (err)
		goto out_free_netdev;

	net->loopback_dev = dev;
	return 0;


out_free_netdev:
	free_netdev(dev);
out:
	if (net == &init_net)
		panic("loopback: Failed to register netdevice: %d\n", err);
	return err;
}

static __net_exit void loopback_net_exit(struct net *net)
{
	struct net_device *dev = net->loopback_dev;

	unregister_netdev(dev);
}


struct pernet_operations __net_initdata loopback_net_ops = {
       .init = loopback_net_init,
       .exit = loopback_net_exit,
};
