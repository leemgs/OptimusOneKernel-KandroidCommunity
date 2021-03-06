



#include <linux/module.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/workqueue.h>

#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/skbuff.h>
#include <linux/bitops.h>
#include <linux/ethtool.h>
#include <asm/uaccess.h>
#include <asm/delay.h>
#include <asm/unaligned.h>
#include <net/arp.h>

#include "config_roms.h"
#include "csr1212.h"
#include "eth1394.h"
#include "highlevel.h"
#include "ieee1394.h"
#include "ieee1394_core.h"
#include "ieee1394_hotplug.h"
#include "ieee1394_transactions.h"
#include "ieee1394_types.h"
#include "iso.h"
#include "nodemgr.h"

#define ETH1394_PRINT_G(level, fmt, args...) \
	printk(level "%s: " fmt, driver_name, ## args)

#define ETH1394_PRINT(level, dev_name, fmt, args...) \
	printk(level "%s: %s: " fmt, driver_name, dev_name, ## args)

struct fragment_info {
	struct list_head list;
	int offset;
	int len;
};

struct partial_datagram {
	struct list_head list;
	u16 dgl;
	u16 dg_size;
	__be16 ether_type;
	struct sk_buff *skb;
	char *pbuf;
	struct list_head frag_info;
};

struct pdg_list {
	struct list_head list;	
	unsigned int sz;	
	spinlock_t lock;	
};

struct eth1394_host_info {
	struct hpsb_host *host;
	struct net_device *dev;
};

struct eth1394_node_ref {
	struct unit_directory *ud;
	struct list_head list;
};

struct eth1394_node_info {
	u16 maxpayload;		
	u8 sspd;		
	u64 fifo;		
	struct pdg_list pdg;	
	int dgl;		
};

static const char driver_name[] = "eth1394";

static struct kmem_cache *packet_task_cache;

static struct hpsb_highlevel eth1394_highlevel;


static const int hdr_type_len[] = {
	sizeof(struct eth1394_uf_hdr),
	sizeof(struct eth1394_ff_hdr),
	sizeof(struct eth1394_sf_hdr),
	sizeof(struct eth1394_sf_hdr)
};

static const u16 eth1394_speedto_maxpayload[] = {

	512, 1024, 2048, 4096,  4096,  4096
};

MODULE_AUTHOR("Ben Collins (bcollins@debian.org)");
MODULE_DESCRIPTION("IEEE 1394 IPv4 Driver (IPv4-over-1394 as per RFC 2734)");
MODULE_LICENSE("GPL");


static int max_partial_datagrams = 25;
module_param(max_partial_datagrams, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(max_partial_datagrams,
		 "Maximum number of partially received fragmented datagrams "
		 "(default = 25).");


static int ether1394_header(struct sk_buff *skb, struct net_device *dev,
			    unsigned short type, const void *daddr,
			    const void *saddr, unsigned len);
static int ether1394_rebuild_header(struct sk_buff *skb);
static int ether1394_header_parse(const struct sk_buff *skb,
				  unsigned char *haddr);
static int ether1394_header_cache(const struct neighbour *neigh,
				  struct hh_cache *hh);
static void ether1394_header_cache_update(struct hh_cache *hh,
					  const struct net_device *dev,
					  const unsigned char *haddr);
static netdev_tx_t ether1394_tx(struct sk_buff *skb,
				struct net_device *dev);
static void ether1394_iso(struct hpsb_iso *iso);

static const struct ethtool_ops ethtool_ops;

static int ether1394_write(struct hpsb_host *host, int srcid, int destid,
			   quadlet_t *data, u64 addr, size_t len, u16 flags);
static void ether1394_add_host(struct hpsb_host *host);
static void ether1394_remove_host(struct hpsb_host *host);
static void ether1394_host_reset(struct hpsb_host *host);


static const struct hpsb_address_ops addr_ops = {
	.write =	ether1394_write,
};


static struct hpsb_highlevel eth1394_highlevel = {
	.name =		driver_name,
	.add_host =	ether1394_add_host,
	.remove_host =	ether1394_remove_host,
	.host_reset =	ether1394_host_reset,
};

static int ether1394_recv_init(struct eth1394_priv *priv)
{
	unsigned int iso_buf_size;

	
	iso_buf_size = min((unsigned int)PAGE_SIZE,
			   2 * (1U << (priv->host->csr.max_rec + 1)));

	priv->iso = hpsb_iso_recv_init(priv->host,
				       ETHER1394_GASP_BUFFERS * iso_buf_size,
				       ETHER1394_GASP_BUFFERS,
				       priv->broadcast_channel,
				       HPSB_ISO_DMA_PACKET_PER_BUFFER,
				       1, ether1394_iso);
	if (priv->iso == NULL) {
		ETH1394_PRINT_G(KERN_ERR, "Failed to allocate IR context\n");
		priv->bc_state = ETHER1394_BC_ERROR;
		return -EAGAIN;
	}

	if (hpsb_iso_recv_start(priv->iso, -1, (1 << 3), -1) < 0)
		priv->bc_state = ETHER1394_BC_STOPPED;
	else
		priv->bc_state = ETHER1394_BC_RUNNING;
	return 0;
}


static int ether1394_open(struct net_device *dev)
{
	struct eth1394_priv *priv = netdev_priv(dev);
	int ret;

	if (priv->bc_state == ETHER1394_BC_ERROR) {
		ret = ether1394_recv_init(priv);
		if (ret)
			return ret;
	}
	netif_start_queue(dev);
	return 0;
}


static int ether1394_stop(struct net_device *dev)
{
	
	flush_scheduled_work();

	netif_stop_queue(dev);
	return 0;
}


static void ether1394_tx_timeout(struct net_device *dev)
{
	struct hpsb_host *host =
			((struct eth1394_priv *)netdev_priv(dev))->host;

	ETH1394_PRINT(KERN_ERR, dev->name, "Timeout, resetting host\n");
	ether1394_host_reset(host);
}

static inline int ether1394_max_mtu(struct hpsb_host* host)
{
	return (1 << (host->csr.max_rec + 1))
			- sizeof(union eth1394_hdr) - ETHER1394_GASP_OVERHEAD;
}

static int ether1394_change_mtu(struct net_device *dev, int new_mtu)
{
	int max_mtu;

	if (new_mtu < 68)
		return -EINVAL;

	max_mtu = ether1394_max_mtu(
			((struct eth1394_priv *)netdev_priv(dev))->host);
	if (new_mtu > max_mtu) {
		ETH1394_PRINT(KERN_INFO, dev->name,
			      "Local node constrains MTU to %d\n", max_mtu);
		return -ERANGE;
	}

	dev->mtu = new_mtu;
	return 0;
}

static void purge_partial_datagram(struct list_head *old)
{
	struct partial_datagram *pd;
	struct list_head *lh, *n;
	struct fragment_info *fi;

	pd = list_entry(old, struct partial_datagram, list);

	list_for_each_safe(lh, n, &pd->frag_info) {
		fi = list_entry(lh, struct fragment_info, list);
		list_del(lh);
		kfree(fi);
	}
	list_del(old);
	kfree_skb(pd->skb);
	kfree(pd);
}



static struct eth1394_node_ref *eth1394_find_node(struct list_head *inl,
						  struct unit_directory *ud)
{
	struct eth1394_node_ref *node;

	list_for_each_entry(node, inl, list)
		if (node->ud == ud)
			return node;

	return NULL;
}

static struct eth1394_node_ref *eth1394_find_node_guid(struct list_head *inl,
						       u64 guid)
{
	struct eth1394_node_ref *node;

	list_for_each_entry(node, inl, list)
		if (node->ud->ne->guid == guid)
			return node;

	return NULL;
}

static struct eth1394_node_ref *eth1394_find_node_nodeid(struct list_head *inl,
							 nodeid_t nodeid)
{
	struct eth1394_node_ref *node;

	list_for_each_entry(node, inl, list)
		if (node->ud->ne->nodeid == nodeid)
			return node;

	return NULL;
}

static int eth1394_new_node(struct eth1394_host_info *hi,
			    struct unit_directory *ud)
{
	struct eth1394_priv *priv;
	struct eth1394_node_ref *new_node;
	struct eth1394_node_info *node_info;

	new_node = kmalloc(sizeof(*new_node), GFP_KERNEL);
	if (!new_node)
		return -ENOMEM;

	node_info = kmalloc(sizeof(*node_info), GFP_KERNEL);
	if (!node_info) {
		kfree(new_node);
		return -ENOMEM;
	}

	spin_lock_init(&node_info->pdg.lock);
	INIT_LIST_HEAD(&node_info->pdg.list);
	node_info->pdg.sz = 0;
	node_info->fifo = CSR1212_INVALID_ADDR_SPACE;

	dev_set_drvdata(&ud->device, node_info);
	new_node->ud = ud;

	priv = netdev_priv(hi->dev);
	list_add_tail(&new_node->list, &priv->ip_node_list);
	return 0;
}

static int eth1394_probe(struct device *dev)
{
	struct unit_directory *ud;
	struct eth1394_host_info *hi;

	ud = container_of(dev, struct unit_directory, device);
	hi = hpsb_get_hostinfo(&eth1394_highlevel, ud->ne->host);
	if (!hi)
		return -ENOENT;

	return eth1394_new_node(hi, ud);
}

static int eth1394_remove(struct device *dev)
{
	struct unit_directory *ud;
	struct eth1394_host_info *hi;
	struct eth1394_priv *priv;
	struct eth1394_node_ref *old_node;
	struct eth1394_node_info *node_info;
	struct list_head *lh, *n;
	unsigned long flags;

	ud = container_of(dev, struct unit_directory, device);
	hi = hpsb_get_hostinfo(&eth1394_highlevel, ud->ne->host);
	if (!hi)
		return -ENOENT;

	priv = netdev_priv(hi->dev);

	old_node = eth1394_find_node(&priv->ip_node_list, ud);
	if (!old_node)
		return 0;

	list_del(&old_node->list);
	kfree(old_node);

	node_info = dev_get_drvdata(&ud->device);

	spin_lock_irqsave(&node_info->pdg.lock, flags);
	
	list_for_each_safe(lh, n, &node_info->pdg.list)
		purge_partial_datagram(lh);
	spin_unlock_irqrestore(&node_info->pdg.lock, flags);

	kfree(node_info);
	dev_set_drvdata(&ud->device, NULL);
	return 0;
}

static int eth1394_update(struct unit_directory *ud)
{
	struct eth1394_host_info *hi;
	struct eth1394_priv *priv;
	struct eth1394_node_ref *node;

	hi = hpsb_get_hostinfo(&eth1394_highlevel, ud->ne->host);
	if (!hi)
		return -ENOENT;

	priv = netdev_priv(hi->dev);
	node = eth1394_find_node(&priv->ip_node_list, ud);
	if (node)
		return 0;

	return eth1394_new_node(hi, ud);
}

static const struct ieee1394_device_id eth1394_id_table[] = {
	{
		.match_flags = (IEEE1394_MATCH_SPECIFIER_ID |
				IEEE1394_MATCH_VERSION),
		.specifier_id =	ETHER1394_GASP_SPECIFIER_ID,
		.version = ETHER1394_GASP_VERSION,
	},
	{}
};

MODULE_DEVICE_TABLE(ieee1394, eth1394_id_table);

static struct hpsb_protocol_driver eth1394_proto_driver = {
	.name		= driver_name,
	.id_table	= eth1394_id_table,
	.update		= eth1394_update,
	.driver		= {
		.probe		= eth1394_probe,
		.remove		= eth1394_remove,
	},
};

static void ether1394_reset_priv(struct net_device *dev, int set_mtu)
{
	unsigned long flags;
	int i;
	struct eth1394_priv *priv = netdev_priv(dev);
	struct hpsb_host *host = priv->host;
	u64 guid = get_unaligned((u64 *)&(host->csr.rom->bus_info_data[3]));
	int max_speed = IEEE1394_SPEED_MAX;

	spin_lock_irqsave(&priv->lock, flags);

	memset(priv->ud_list, 0, sizeof(priv->ud_list));
	priv->bc_maxpayload = 512;

	
	
	for (i = 0; i < host->node_count; i++) {
		
		if (host->speed[i] == SELFID_SPEED_UNKNOWN) {
			max_speed = IEEE1394_SPEED_100;
			break;
		}
		if (max_speed > host->speed[i])
			max_speed = host->speed[i];
	}
	priv->bc_sspd = max_speed;

	if (set_mtu) {
		
		dev->mtu = min(1500, ether1394_max_mtu(host));

		
		memcpy(dev->dev_addr, &guid, sizeof(u64));
		memset(dev->broadcast, 0xff, sizeof(u64));
	}

	spin_unlock_irqrestore(&priv->lock, flags);
}

static const struct header_ops ether1394_header_ops = {
	.create		= ether1394_header,
	.rebuild	= ether1394_rebuild_header,
	.cache  	= ether1394_header_cache,
	.cache_update	= ether1394_header_cache_update,
	.parse		= ether1394_header_parse,
};

static const struct net_device_ops ether1394_netdev_ops = {
	.ndo_open	= ether1394_open,
	.ndo_stop	= ether1394_stop,
	.ndo_start_xmit	= ether1394_tx,
	.ndo_tx_timeout	= ether1394_tx_timeout,
	.ndo_change_mtu	= ether1394_change_mtu,
};

static void ether1394_init_dev(struct net_device *dev)
{

	dev->header_ops		= &ether1394_header_ops;
	dev->netdev_ops		= &ether1394_netdev_ops;

	SET_ETHTOOL_OPS(dev, &ethtool_ops);

	dev->watchdog_timeo	= ETHER1394_TIMEOUT;
	dev->flags		= IFF_BROADCAST | IFF_MULTICAST;
	dev->features		= NETIF_F_HIGHDMA;
	dev->addr_len		= ETH1394_ALEN;
	dev->hard_header_len 	= ETH1394_HLEN;
	dev->type		= ARPHRD_IEEE1394;

	
	dev->tx_queue_len	= 1000;
}


static void ether1394_wake_queue(struct work_struct *work)
{
	struct eth1394_priv *priv;
	struct hpsb_packet *packet;

	priv = container_of(work, struct eth1394_priv, wake);
	packet = hpsb_alloc_packet(0);

	
	if (!packet)
		goto out;

	packet->host = priv->host;
	packet->node_id = priv->wake_node;
	
	if (hpsb_get_tlabel(packet) == 0)
		hpsb_free_tlabel(packet);

	hpsb_free_packet(packet);
out:
	netif_wake_queue(priv->wake_dev);
}


static void ether1394_add_host(struct hpsb_host *host)
{
	struct eth1394_host_info *hi = NULL;
	struct net_device *dev = NULL;
	struct eth1394_priv *priv;
	u64 fifo_addr;

	if (hpsb_config_rom_ip1394_add(host) != 0) {
		ETH1394_PRINT_G(KERN_ERR, "Can't add IP-over-1394 ROM entry\n");
		return;
	}

	fifo_addr = hpsb_allocate_and_register_addrspace(
			&eth1394_highlevel, host, &addr_ops,
			ETHER1394_REGION_ADDR_LEN, ETHER1394_REGION_ADDR_LEN,
			CSR1212_INVALID_ADDR_SPACE, CSR1212_INVALID_ADDR_SPACE);
	if (fifo_addr == CSR1212_INVALID_ADDR_SPACE) {
		ETH1394_PRINT_G(KERN_ERR, "Cannot register CSR space\n");
		hpsb_config_rom_ip1394_remove(host);
		return;
	}

	dev = alloc_netdev(sizeof(*priv), "eth%d", ether1394_init_dev);
	if (dev == NULL) {
		ETH1394_PRINT_G(KERN_ERR, "Out of memory\n");
		goto out;
	}

	SET_NETDEV_DEV(dev, &host->device);

	priv = netdev_priv(dev);
	INIT_LIST_HEAD(&priv->ip_node_list);
	spin_lock_init(&priv->lock);
	priv->host = host;
	priv->local_fifo = fifo_addr;
	INIT_WORK(&priv->wake, ether1394_wake_queue);
	priv->wake_dev = dev;

	hi = hpsb_create_hostinfo(&eth1394_highlevel, host, sizeof(*hi));
	if (hi == NULL) {
		ETH1394_PRINT_G(KERN_ERR, "Out of memory\n");
		goto out;
	}

	ether1394_reset_priv(dev, 1);

	if (register_netdev(dev)) {
		ETH1394_PRINT_G(KERN_ERR, "Cannot register the driver\n");
		goto out;
	}

	ETH1394_PRINT(KERN_INFO, dev->name, "IPv4 over IEEE 1394 (fw-host%d)\n",
		      host->id);

	hi->host = host;
	hi->dev = dev;

	
	priv->broadcast_channel = host->csr.broadcast_channel & 0x3f;

	ether1394_recv_init(priv);
	return;
out:
	if (dev)
		free_netdev(dev);
	if (hi)
		hpsb_destroy_hostinfo(&eth1394_highlevel, host);
	hpsb_unregister_addrspace(&eth1394_highlevel, host, fifo_addr);
	hpsb_config_rom_ip1394_remove(host);
}


static void ether1394_remove_host(struct hpsb_host *host)
{
	struct eth1394_host_info *hi;
	struct eth1394_priv *priv;

	hi = hpsb_get_hostinfo(&eth1394_highlevel, host);
	if (!hi)
		return;
	priv = netdev_priv(hi->dev);
	hpsb_unregister_addrspace(&eth1394_highlevel, host, priv->local_fifo);
	hpsb_config_rom_ip1394_remove(host);
	if (priv->iso)
		hpsb_iso_shutdown(priv->iso);
	unregister_netdev(hi->dev);
	free_netdev(hi->dev);
}


static void ether1394_host_reset(struct hpsb_host *host)
{
	struct eth1394_host_info *hi;
	struct eth1394_priv *priv;
	struct net_device *dev;
	struct list_head *lh, *n;
	struct eth1394_node_ref *node;
	struct eth1394_node_info *node_info;
	unsigned long flags;

	hi = hpsb_get_hostinfo(&eth1394_highlevel, host);

	
	if (!hi)
		return;

	dev = hi->dev;
	priv = netdev_priv(dev);

	
	netif_stop_queue(dev);
	ether1394_reset_priv(dev, 0);

	list_for_each_entry(node, &priv->ip_node_list, list) {
		node_info = dev_get_drvdata(&node->ud->device);

		spin_lock_irqsave(&node_info->pdg.lock, flags);

		list_for_each_safe(lh, n, &node_info->pdg.list)
			purge_partial_datagram(lh);

		INIT_LIST_HEAD(&(node_info->pdg.list));
		node_info->pdg.sz = 0;

		spin_unlock_irqrestore(&node_info->pdg.lock, flags);
	}

	netif_wake_queue(dev);
}





static int ether1394_header(struct sk_buff *skb, struct net_device *dev,
			    unsigned short type, const void *daddr,
			    const void *saddr, unsigned len)
{
	struct eth1394hdr *eth =
			(struct eth1394hdr *)skb_push(skb, ETH1394_HLEN);

	eth->h_proto = htons(type);

	if (dev->flags & (IFF_LOOPBACK | IFF_NOARP)) {
		memset(eth->h_dest, 0, dev->addr_len);
		return dev->hard_header_len;
	}

	if (daddr) {
		memcpy(eth->h_dest, daddr, dev->addr_len);
		return dev->hard_header_len;
	}

	return -dev->hard_header_len;
}


static int ether1394_rebuild_header(struct sk_buff *skb)
{
	struct eth1394hdr *eth = (struct eth1394hdr *)skb->data;

	if (eth->h_proto == htons(ETH_P_IP))
		return arp_find((unsigned char *)&eth->h_dest, skb);

	ETH1394_PRINT(KERN_DEBUG, skb->dev->name,
		      "unable to resolve type %04x addresses\n",
		      ntohs(eth->h_proto));
	return 0;
}

static int ether1394_header_parse(const struct sk_buff *skb,
				  unsigned char *haddr)
{
	memcpy(haddr, skb->dev->dev_addr, ETH1394_ALEN);
	return ETH1394_ALEN;
}

static int ether1394_header_cache(const struct neighbour *neigh,
				  struct hh_cache *hh)
{
	__be16 type = hh->hh_type;
	struct net_device *dev = neigh->dev;
	struct eth1394hdr *eth =
		(struct eth1394hdr *)((u8 *)hh->hh_data + 16 - ETH1394_HLEN);

	if (type == htons(ETH_P_802_3))
		return -1;

	eth->h_proto = type;
	memcpy(eth->h_dest, neigh->ha, dev->addr_len);

	hh->hh_len = ETH1394_HLEN;
	return 0;
}


static void ether1394_header_cache_update(struct hh_cache *hh,
					  const struct net_device *dev,
					  const unsigned char * haddr)
{
	memcpy((u8 *)hh->hh_data + 16 - ETH1394_HLEN, haddr, dev->addr_len);
}




static __be16 ether1394_type_trans(struct sk_buff *skb, struct net_device *dev)
{
	struct eth1394hdr *eth;
	unsigned char *rawp;

	skb_reset_mac_header(skb);
	skb_pull(skb, ETH1394_HLEN);
	eth = eth1394_hdr(skb);

	if (*eth->h_dest & 1) {
		if (memcmp(eth->h_dest, dev->broadcast, dev->addr_len) == 0)
			skb->pkt_type = PACKET_BROADCAST;
#if 0
		else
			skb->pkt_type = PACKET_MULTICAST;
#endif
	} else {
		if (memcmp(eth->h_dest, dev->dev_addr, dev->addr_len))
			skb->pkt_type = PACKET_OTHERHOST;
	}

	if (ntohs(eth->h_proto) >= 1536)
		return eth->h_proto;

	rawp = skb->data;

	if (*(unsigned short *)rawp == 0xFFFF)
		return htons(ETH_P_802_3);

	return htons(ETH_P_802_2);
}


static __be16 ether1394_parse_encap(struct sk_buff *skb, struct net_device *dev,
				 nodeid_t srcid, nodeid_t destid,
				 __be16 ether_type)
{
	struct eth1394_priv *priv = netdev_priv(dev);
	__be64 dest_hw;
	__be16 ret = 0;

	
	if (destid == (LOCAL_BUS | ALL_NODES))
		dest_hw = ~cpu_to_be64(0);  
	else
		dest_hw = cpu_to_be64((u64)priv->host->csr.guid_hi << 32 |
				      priv->host->csr.guid_lo);

	
	if (ether_type == htons(ETH_P_ARP)) {
		struct eth1394_arp *arp1394 = (struct eth1394_arp *)skb->data;
		struct arphdr *arp = (struct arphdr *)skb->data;
		unsigned char *arp_ptr = (unsigned char *)(arp + 1);
		u64 fifo_addr = (u64)ntohs(arp1394->fifo_hi) << 32 |
					   ntohl(arp1394->fifo_lo);
		u8 max_rec = min(priv->host->csr.max_rec,
				 (u8)(arp1394->max_rec));
		int sspd = arp1394->sspd;
		u16 maxpayload;
		struct eth1394_node_ref *node;
		struct eth1394_node_info *node_info;
		__be64 guid;

		
		if (sspd > 5 || sspd < 0)
			sspd = 0;

		maxpayload = min(eth1394_speedto_maxpayload[sspd],
				 (u16)(1 << (max_rec + 1)));

		guid = get_unaligned(&arp1394->s_uniq_id);
		node = eth1394_find_node_guid(&priv->ip_node_list,
					      be64_to_cpu(guid));
		if (!node)
			return cpu_to_be16(0);

		node_info = dev_get_drvdata(&node->ud->device);

		
		node_info->maxpayload =	maxpayload;
		node_info->sspd =	sspd;
		node_info->fifo =	fifo_addr;

		

		arp->ar_hln = 8;
		arp_ptr += arp->ar_hln;		
		*(u32 *)arp_ptr = arp1394->sip;	
		arp_ptr += arp->ar_pln;		

		if (arp->ar_op == htons(ARPOP_REQUEST))
			memset(arp_ptr, 0, sizeof(u64));
		else
			memcpy(arp_ptr, dev->dev_addr, sizeof(u64));
	}

	
	if (dev_hard_header(skb, dev, ntohs(ether_type), &dest_hw, NULL,
			    skb->len) >= 0)
		ret = ether1394_type_trans(skb, dev);

	return ret;
}

static int fragment_overlap(struct list_head *frag_list, int offset, int len)
{
	struct fragment_info *fi;
	int end = offset + len;

	list_for_each_entry(fi, frag_list, list)
		if (offset < fi->offset + fi->len && end > fi->offset)
			return 1;

	return 0;
}

static struct list_head *find_partial_datagram(struct list_head *pdgl, int dgl)
{
	struct partial_datagram *pd;

	list_for_each_entry(pd, pdgl, list)
		if (pd->dgl == dgl)
			return &pd->list;

	return NULL;
}


static int new_fragment(struct list_head *frag_info, int offset, int len)
{
	struct list_head *lh;
	struct fragment_info *fi, *fi2, *new;

	list_for_each(lh, frag_info) {
		fi = list_entry(lh, struct fragment_info, list);
		if (fi->offset + fi->len == offset) {
			
			fi->len += len;
			
			fi2 = list_entry(lh->next, struct fragment_info, list);
			if (fi->offset + fi->len == fi2->offset) {
				
				fi->len += fi2->len;
				list_del(lh->next);
				kfree(fi2);
			}
			return 0;
		} else if (offset + len == fi->offset) {
			
			fi->offset = offset;
			fi->len += len;
			
			fi2 = list_entry(lh->prev, struct fragment_info, list);
			if (fi2->offset + fi2->len == fi->offset) {
				
				fi2->len += fi->len;
				list_del(lh);
				kfree(fi);
			}
			return 0;
		} else if (offset > fi->offset + fi->len) {
			break;
		} else if (offset + len < fi->offset) {
			lh = lh->prev;
			break;
		}
	}

	new = kmalloc(sizeof(*new), GFP_ATOMIC);
	if (!new)
		return -ENOMEM;

	new->offset = offset;
	new->len = len;

	list_add(&new->list, lh);
	return 0;
}

static int new_partial_datagram(struct net_device *dev, struct list_head *pdgl,
				int dgl, int dg_size, char *frag_buf,
				int frag_off, int frag_len)
{
	struct partial_datagram *new;

	new = kmalloc(sizeof(*new), GFP_ATOMIC);
	if (!new)
		return -ENOMEM;

	INIT_LIST_HEAD(&new->frag_info);

	if (new_fragment(&new->frag_info, frag_off, frag_len) < 0) {
		kfree(new);
		return -ENOMEM;
	}

	new->dgl = dgl;
	new->dg_size = dg_size;

	new->skb = dev_alloc_skb(dg_size + dev->hard_header_len + 15);
	if (!new->skb) {
		struct fragment_info *fi = list_entry(new->frag_info.next,
						      struct fragment_info,
						      list);
		kfree(fi);
		kfree(new);
		return -ENOMEM;
	}

	skb_reserve(new->skb, (dev->hard_header_len + 15) & ~15);
	new->pbuf = skb_put(new->skb, dg_size);
	memcpy(new->pbuf + frag_off, frag_buf, frag_len);

	list_add(&new->list, pdgl);
	return 0;
}

static int update_partial_datagram(struct list_head *pdgl, struct list_head *lh,
				   char *frag_buf, int frag_off, int frag_len)
{
	struct partial_datagram *pd =
			list_entry(lh, struct partial_datagram, list);

	if (new_fragment(&pd->frag_info, frag_off, frag_len) < 0)
		return -ENOMEM;

	memcpy(pd->pbuf + frag_off, frag_buf, frag_len);

	
	list_move(lh, pdgl);
	return 0;
}

static int is_datagram_complete(struct list_head *lh, int dg_size)
{
	struct partial_datagram *pd;
	struct fragment_info *fi;

	pd = list_entry(lh, struct partial_datagram, list);
	fi = list_entry(pd->frag_info.next, struct fragment_info, list);

	return (fi->len == dg_size);
}


static int ether1394_data_handler(struct net_device *dev, int srcid, int destid,
				  char *buf, int len)
{
	struct sk_buff *skb;
	unsigned long flags;
	struct eth1394_priv *priv = netdev_priv(dev);
	union eth1394_hdr *hdr = (union eth1394_hdr *)buf;
	__be16 ether_type = cpu_to_be16(0);  
	int hdr_len;
	struct unit_directory *ud = priv->ud_list[NODEID_TO_NODE(srcid)];
	struct eth1394_node_info *node_info;

	if (!ud) {
		struct eth1394_node_ref *node;
		node = eth1394_find_node_nodeid(&priv->ip_node_list, srcid);
		if (unlikely(!node)) {
			HPSB_PRINT(KERN_ERR, "ether1394 rx: sender nodeid "
				   "lookup failure: " NODE_BUS_FMT,
				   NODE_BUS_ARGS(priv->host, srcid));
			dev->stats.rx_dropped++;
			return -1;
		}
		ud = node->ud;

		priv->ud_list[NODEID_TO_NODE(srcid)] = ud;
	}

	node_info = dev_get_drvdata(&ud->device);

	
	hdr->words.word1 = ntohs(hdr->words.word1);

	hdr_len = hdr_type_len[hdr->common.lf];

	if (hdr->common.lf == ETH1394_HDR_LF_UF) {
		

		skb = dev_alloc_skb(len + dev->hard_header_len + 15);
		if (unlikely(!skb)) {
			ETH1394_PRINT_G(KERN_ERR, "Out of memory\n");
			dev->stats.rx_dropped++;
			return -1;
		}
		skb_reserve(skb, (dev->hard_header_len + 15) & ~15);
		memcpy(skb_put(skb, len - hdr_len), buf + hdr_len,
		       len - hdr_len);
		ether_type = hdr->uf.ether_type;
	} else {
		

		struct list_head *pdgl, *lh;
		struct partial_datagram *pd;
		int fg_off;
		int fg_len = len - hdr_len;
		int dg_size;
		int dgl;
		int retval;
		struct pdg_list *pdg = &(node_info->pdg);

		hdr->words.word3 = ntohs(hdr->words.word3);
		

		if (hdr->common.lf == ETH1394_HDR_LF_FF) {
			ether_type = hdr->ff.ether_type;
			dgl = hdr->ff.dgl;
			dg_size = hdr->ff.dg_size + 1;
			fg_off = 0;
		} else {
			hdr->words.word2 = ntohs(hdr->words.word2);
			dgl = hdr->sf.dgl;
			dg_size = hdr->sf.dg_size + 1;
			fg_off = hdr->sf.fg_off;
		}
		spin_lock_irqsave(&pdg->lock, flags);

		pdgl = &(pdg->list);
		lh = find_partial_datagram(pdgl, dgl);

		if (lh == NULL) {
			while (pdg->sz >= max_partial_datagrams) {
				
				purge_partial_datagram(pdgl->prev);
				pdg->sz--;
			}

			retval = new_partial_datagram(dev, pdgl, dgl, dg_size,
						      buf + hdr_len, fg_off,
						      fg_len);
			if (retval < 0) {
				spin_unlock_irqrestore(&pdg->lock, flags);
				goto bad_proto;
			}
			pdg->sz++;
			lh = find_partial_datagram(pdgl, dgl);
		} else {
			pd = list_entry(lh, struct partial_datagram, list);

			if (fragment_overlap(&pd->frag_info, fg_off, fg_len)) {
				
				purge_partial_datagram(lh);
				retval = new_partial_datagram(dev, pdgl, dgl,
							      dg_size,
							      buf + hdr_len,
							      fg_off, fg_len);
				if (retval < 0) {
					pdg->sz--;
					spin_unlock_irqrestore(&pdg->lock, flags);
					goto bad_proto;
				}
			} else {
				retval = update_partial_datagram(pdgl, lh,
								 buf + hdr_len,
								 fg_off, fg_len);
				if (retval < 0) {
					
					purge_partial_datagram(lh);
					pdg->sz--;
					spin_unlock_irqrestore(&pdg->lock, flags);
					goto bad_proto;
				}
			} 
		} 

		pd = list_entry(lh, struct partial_datagram, list);

		if (hdr->common.lf == ETH1394_HDR_LF_FF)
			pd->ether_type = ether_type;

		if (is_datagram_complete(lh, dg_size)) {
			ether_type = pd->ether_type;
			pdg->sz--;
			skb = skb_get(pd->skb);
			purge_partial_datagram(lh);
			spin_unlock_irqrestore(&pdg->lock, flags);
		} else {
			
			spin_unlock_irqrestore(&pdg->lock, flags);
			return 0;
		}
	} 

	
	skb->dev = dev;
	skb->ip_summed = CHECKSUM_UNNECESSARY;	

	
	skb->protocol = ether1394_parse_encap(skb, dev, srcid, destid,
					      ether_type);

	spin_lock_irqsave(&priv->lock, flags);

	if (!skb->protocol) {
		dev->stats.rx_errors++;
		dev->stats.rx_dropped++;
		dev_kfree_skb_any(skb);
	} else if (netif_rx(skb) == NET_RX_DROP) {
		dev->stats.rx_errors++;
		dev->stats.rx_dropped++;
	} else {
		dev->stats.rx_packets++;
		dev->stats.rx_bytes += skb->len;
	}

	spin_unlock_irqrestore(&priv->lock, flags);

bad_proto:
	if (netif_queue_stopped(dev))
		netif_wake_queue(dev);

	return 0;
}

static int ether1394_write(struct hpsb_host *host, int srcid, int destid,
			   quadlet_t *data, u64 addr, size_t len, u16 flags)
{
	struct eth1394_host_info *hi;

	hi = hpsb_get_hostinfo(&eth1394_highlevel, host);
	if (unlikely(!hi)) {
		ETH1394_PRINT_G(KERN_ERR, "No net device at fw-host%d\n",
				host->id);
		return RCODE_ADDRESS_ERROR;
	}

	if (ether1394_data_handler(hi->dev, srcid, destid, (char*)data, len))
		return RCODE_ADDRESS_ERROR;
	else
		return RCODE_COMPLETE;
}

static void ether1394_iso(struct hpsb_iso *iso)
{
	__be32 *data;
	char *buf;
	struct eth1394_host_info *hi;
	struct net_device *dev;
	struct eth1394_priv *priv;
	unsigned int len;
	u32 specifier_id;
	u16 source_id;
	int i;
	int nready;

	hi = hpsb_get_hostinfo(&eth1394_highlevel, iso->host);
	if (unlikely(!hi)) {
		ETH1394_PRINT_G(KERN_ERR, "No net device at fw-host%d\n",
				iso->host->id);
		return;
	}

	dev = hi->dev;

	nready = hpsb_iso_n_ready(iso);
	for (i = 0; i < nready; i++) {
		struct hpsb_iso_packet_info *info =
			&iso->infos[(iso->first_packet + i) % iso->buf_packets];
		data = (__be32 *)(iso->data_buf.kvirt + info->offset);

		
		buf = (char *)data + 8;
		len = info->len - 8;

		specifier_id = (be32_to_cpu(data[0]) & 0xffff) << 8 |
			       (be32_to_cpu(data[1]) & 0xff000000) >> 24;
		source_id = be32_to_cpu(data[0]) >> 16;

		priv = netdev_priv(dev);

		if (info->channel != (iso->host->csr.broadcast_channel & 0x3f)
		    || specifier_id != ETHER1394_GASP_SPECIFIER_ID) {
			
			continue;
		}
		ether1394_data_handler(dev, source_id, LOCAL_BUS | ALL_NODES,
				       buf, len);
	}

	hpsb_iso_recv_release_packets(iso, i);

}




static void ether1394_arp_to_1394arp(struct sk_buff *skb,
				     struct net_device *dev)
{
	struct eth1394_priv *priv = netdev_priv(dev);
	struct arphdr *arp = (struct arphdr *)skb->data;
	unsigned char *arp_ptr = (unsigned char *)(arp + 1);
	struct eth1394_arp *arp1394 = (struct eth1394_arp *)skb->data;

	arp1394->hw_addr_len	= 16;
	arp1394->sip		= *(u32*)(arp_ptr + ETH1394_ALEN);
	arp1394->max_rec	= priv->host->csr.max_rec;
	arp1394->sspd		= priv->host->csr.lnk_spd;
	arp1394->fifo_hi	= htons(priv->local_fifo >> 32);
	arp1394->fifo_lo	= htonl(priv->local_fifo & ~0x0);
}


static unsigned int ether1394_encapsulate_prep(unsigned int max_payload,
					       __be16 proto,
					       union eth1394_hdr *hdr,
					       u16 dg_size, u16 dgl)
{
	unsigned int adj_max_payload =
				max_payload - hdr_type_len[ETH1394_HDR_LF_UF];

	
	if (dg_size <= adj_max_payload) {
		hdr->uf.lf = ETH1394_HDR_LF_UF;
		hdr->uf.ether_type = proto;
	} else {
		hdr->ff.lf = ETH1394_HDR_LF_FF;
		hdr->ff.ether_type = proto;
		hdr->ff.dg_size = dg_size - 1;
		hdr->ff.dgl = dgl;
		adj_max_payload = max_payload - hdr_type_len[ETH1394_HDR_LF_FF];
	}
	return DIV_ROUND_UP(dg_size, adj_max_payload);
}

static unsigned int ether1394_encapsulate(struct sk_buff *skb,
					  unsigned int max_payload,
					  union eth1394_hdr *hdr)
{
	union eth1394_hdr *bufhdr;
	int ftype = hdr->common.lf;
	int hdrsz = hdr_type_len[ftype];
	unsigned int adj_max_payload = max_payload - hdrsz;

	switch (ftype) {
	case ETH1394_HDR_LF_UF:
		bufhdr = (union eth1394_hdr *)skb_push(skb, hdrsz);
		bufhdr->words.word1 = htons(hdr->words.word1);
		bufhdr->words.word2 = hdr->words.word2;
		break;

	case ETH1394_HDR_LF_FF:
		bufhdr = (union eth1394_hdr *)skb_push(skb, hdrsz);
		bufhdr->words.word1 = htons(hdr->words.word1);
		bufhdr->words.word2 = hdr->words.word2;
		bufhdr->words.word3 = htons(hdr->words.word3);
		bufhdr->words.word4 = 0;

		
		hdr->common.lf = ETH1394_HDR_LF_IF;
		hdr->sf.fg_off = 0;
		break;

	default:
		hdr->sf.fg_off += adj_max_payload;
		bufhdr = (union eth1394_hdr *)skb_pull(skb, adj_max_payload);
		if (max_payload >= skb->len)
			hdr->common.lf = ETH1394_HDR_LF_LF;
		bufhdr->words.word1 = htons(hdr->words.word1);
		bufhdr->words.word2 = htons(hdr->words.word2);
		bufhdr->words.word3 = htons(hdr->words.word3);
		bufhdr->words.word4 = 0;
	}
	return min(max_payload, skb->len);
}

static struct hpsb_packet *ether1394_alloc_common_packet(struct hpsb_host *host)
{
	struct hpsb_packet *p;

	p = hpsb_alloc_packet(0);
	if (p) {
		p->host = host;
		p->generation = get_hpsb_generation(host);
		p->type = hpsb_async;
	}
	return p;
}

static int ether1394_prep_write_packet(struct hpsb_packet *p,
				       struct hpsb_host *host, nodeid_t node,
				       u64 addr, void *data, int tx_len)
{
	p->node_id = node;

	if (hpsb_get_tlabel(p))
		return -EAGAIN;

	p->tcode = TCODE_WRITEB;
	p->header_size = 16;
	p->expect_response = 1;
	p->header[0] =
		p->node_id << 16 | p->tlabel << 10 | 1 << 8 | TCODE_WRITEB << 4;
	p->header[1] = host->node_id << 16 | addr >> 32;
	p->header[2] = addr & 0xffffffff;
	p->header[3] = tx_len << 16;
	p->data_size = (tx_len + 3) & ~3;
	p->data = data;

	return 0;
}

static void ether1394_prep_gasp_packet(struct hpsb_packet *p,
				       struct eth1394_priv *priv,
				       struct sk_buff *skb, int length)
{
	p->header_size = 4;
	p->tcode = TCODE_STREAM_DATA;

	p->header[0] = length << 16 | 3 << 14 | priv->broadcast_channel << 8 |
		       TCODE_STREAM_DATA << 4;
	p->data_size = length;
	p->data = (quadlet_t *)skb->data - 2;
	p->data[0] = cpu_to_be32(priv->host->node_id << 16 |
				 ETHER1394_GASP_SPECIFIER_ID_HI);
	p->data[1] = cpu_to_be32(ETHER1394_GASP_SPECIFIER_ID_LO << 24 |
				 ETHER1394_GASP_VERSION);

	p->speed_code = priv->bc_sspd;

	
	p->node_id = LOCAL_BUS | ALL_NODES;
}

static void ether1394_free_packet(struct hpsb_packet *packet)
{
	if (packet->tcode != TCODE_STREAM_DATA)
		hpsb_free_tlabel(packet);
	hpsb_free_packet(packet);
}

static void ether1394_complete_cb(void *__ptask);

static int ether1394_send_packet(struct packet_task *ptask, unsigned int tx_len)
{
	struct eth1394_priv *priv = ptask->priv;
	struct hpsb_packet *packet = NULL;

	packet = ether1394_alloc_common_packet(priv->host);
	if (!packet)
		return -ENOMEM;

	if (ptask->tx_type == ETH1394_GASP) {
		int length = tx_len + 2 * sizeof(quadlet_t);

		ether1394_prep_gasp_packet(packet, priv, ptask->skb, length);
	} else if (ether1394_prep_write_packet(packet, priv->host,
					       ptask->dest_node,
					       ptask->addr, ptask->skb->data,
					       tx_len)) {
		hpsb_free_packet(packet);
		return -EAGAIN;
	}

	ptask->packet = packet;
	hpsb_set_packet_complete_task(ptask->packet, ether1394_complete_cb,
				      ptask);

	if (hpsb_send_packet(packet) < 0) {
		ether1394_free_packet(packet);
		return -EIO;
	}

	return 0;
}


static void ether1394_dg_complete(struct packet_task *ptask, int fail)
{
	struct sk_buff *skb = ptask->skb;
	struct net_device *dev = skb->dev;
	struct eth1394_priv *priv = netdev_priv(dev);
	unsigned long flags;

	
	spin_lock_irqsave(&priv->lock, flags);
	if (fail) {
		dev->stats.tx_dropped++;
		dev->stats.tx_errors++;
	} else {
		dev->stats.tx_bytes += skb->len;
		dev->stats.tx_packets++;
	}
	spin_unlock_irqrestore(&priv->lock, flags);

	dev_kfree_skb_any(skb);
	kmem_cache_free(packet_task_cache, ptask);
}


static void ether1394_complete_cb(void *__ptask)
{
	struct packet_task *ptask = (struct packet_task *)__ptask;
	struct hpsb_packet *packet = ptask->packet;
	int fail = 0;

	if (packet->tcode != TCODE_STREAM_DATA)
		fail = hpsb_packet_success(packet);

	ether1394_free_packet(packet);

	ptask->outstanding_pkts--;
	if (ptask->outstanding_pkts > 0 && !fail) {
		int tx_len, err;

		
		tx_len = ether1394_encapsulate(ptask->skb, ptask->max_payload,
					       &ptask->hdr);
		err = ether1394_send_packet(ptask, tx_len);
		if (err) {
			if (err == -EAGAIN)
				ETH1394_PRINT_G(KERN_ERR, "Out of tlabels\n");

			ether1394_dg_complete(ptask, 1);
		}
	} else {
		ether1394_dg_complete(ptask, fail);
	}
}


static netdev_tx_t ether1394_tx(struct sk_buff *skb,
				struct net_device *dev)
{
	struct eth1394hdr hdr_buf;
	struct eth1394_priv *priv = netdev_priv(dev);
	__be16 proto;
	unsigned long flags;
	nodeid_t dest_node;
	eth1394_tx_type tx_type;
	unsigned int tx_len;
	unsigned int max_payload;
	u16 dg_size;
	u16 dgl;
	struct packet_task *ptask;
	struct eth1394_node_ref *node;
	struct eth1394_node_info *node_info = NULL;

	ptask = kmem_cache_alloc(packet_task_cache, GFP_ATOMIC);
	if (ptask == NULL)
		goto fail;

	
#if 0
	if ((priv->host->csr.broadcast_channel & 0xc0000000) != 0xc0000000)
		goto fail;
#endif

	skb = skb_share_check(skb, GFP_ATOMIC);
	if (!skb)
		goto fail;

	
	memcpy(&hdr_buf, skb->data, sizeof(hdr_buf));
	skb_pull(skb, ETH1394_HLEN);

	proto = hdr_buf.h_proto;
	dg_size = skb->len;

	
	if (memcmp(hdr_buf.h_dest, dev->broadcast, ETH1394_ALEN) == 0 ||
	    proto == htons(ETH_P_ARP) ||
	    (proto == htons(ETH_P_IP) &&
	     IN_MULTICAST(ntohl(ip_hdr(skb)->daddr)))) {
		tx_type = ETH1394_GASP;
		dest_node = LOCAL_BUS | ALL_NODES;
		max_payload = priv->bc_maxpayload - ETHER1394_GASP_OVERHEAD;
		BUG_ON(max_payload < 512 - ETHER1394_GASP_OVERHEAD);
		dgl = priv->bc_dgl;
		if (max_payload < dg_size + hdr_type_len[ETH1394_HDR_LF_UF])
			priv->bc_dgl++;
	} else {
		__be64 guid = get_unaligned((__be64 *)hdr_buf.h_dest);

		node = eth1394_find_node_guid(&priv->ip_node_list,
					      be64_to_cpu(guid));
		if (!node)
			goto fail;

		node_info = dev_get_drvdata(&node->ud->device);
		if (node_info->fifo == CSR1212_INVALID_ADDR_SPACE)
			goto fail;

		dest_node = node->ud->ne->nodeid;
		max_payload = node_info->maxpayload;
		BUG_ON(max_payload < 512 - ETHER1394_GASP_OVERHEAD);

		dgl = node_info->dgl;
		if (max_payload < dg_size + hdr_type_len[ETH1394_HDR_LF_UF])
			node_info->dgl++;
		tx_type = ETH1394_WRREQ;
	}

	
	if (proto == htons(ETH_P_ARP))
		ether1394_arp_to_1394arp(skb, dev);

	ptask->hdr.words.word1 = 0;
	ptask->hdr.words.word2 = 0;
	ptask->hdr.words.word3 = 0;
	ptask->hdr.words.word4 = 0;
	ptask->skb = skb;
	ptask->priv = priv;
	ptask->tx_type = tx_type;

	if (tx_type != ETH1394_GASP) {
		u64 addr;

		spin_lock_irqsave(&priv->lock, flags);
		addr = node_info->fifo;
		spin_unlock_irqrestore(&priv->lock, flags);

		ptask->addr = addr;
		ptask->dest_node = dest_node;
	}

	ptask->tx_type = tx_type;
	ptask->max_payload = max_payload;
	ptask->outstanding_pkts = ether1394_encapsulate_prep(max_payload,
					proto, &ptask->hdr, dg_size, dgl);

	
	tx_len = ether1394_encapsulate(skb, max_payload, &ptask->hdr);
	dev->trans_start = jiffies;
	if (ether1394_send_packet(ptask, tx_len)) {
		if (dest_node == (LOCAL_BUS | ALL_NODES))
			goto fail;

		
		skb_pull(skb, 4);
		ether1394_header(skb, dev, ntohs(hdr_buf.h_proto),
				 hdr_buf.h_dest, NULL, 0);

		
		netif_stop_queue(dev);
		priv->wake_node = dest_node;
		schedule_work(&priv->wake);
		kmem_cache_free(packet_task_cache, ptask);
		return NETDEV_TX_BUSY;
	}

	return NETDEV_TX_OK;
fail:
	if (ptask)
		kmem_cache_free(packet_task_cache, ptask);

	if (skb != NULL)
		dev_kfree_skb(skb);

	spin_lock_irqsave(&priv->lock, flags);
	dev->stats.tx_dropped++;
	dev->stats.tx_errors++;
	spin_unlock_irqrestore(&priv->lock, flags);

	return NETDEV_TX_OK;
}

static void ether1394_get_drvinfo(struct net_device *dev,
				  struct ethtool_drvinfo *info)
{
	strcpy(info->driver, driver_name);
	strcpy(info->bus_info, "ieee1394"); 
}

static const struct ethtool_ops ethtool_ops = {
	.get_drvinfo = ether1394_get_drvinfo
};

static int __init ether1394_init_module(void)
{
	int err;

	packet_task_cache = kmem_cache_create("packet_task",
					      sizeof(struct packet_task),
					      0, 0, NULL);
	if (!packet_task_cache)
		return -ENOMEM;

	hpsb_register_highlevel(&eth1394_highlevel);
	err = hpsb_register_protocol(&eth1394_proto_driver);
	if (err) {
		hpsb_unregister_highlevel(&eth1394_highlevel);
		kmem_cache_destroy(packet_task_cache);
	}
	return err;
}

static void __exit ether1394_exit_module(void)
{
	hpsb_unregister_protocol(&eth1394_proto_driver);
	hpsb_unregister_highlevel(&eth1394_highlevel);
	kmem_cache_destroy(packet_task_cache);
}

module_init(ether1394_init_module);
module_exit(ether1394_exit_module);
