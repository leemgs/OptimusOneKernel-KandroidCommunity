

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kmod.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/rcupdate.h>
#include <linux/uaccess.h>
#include <linux/net.h>
#include <linux/netdevice.h>
#include <linux/socket.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/skbuff.h>
#include <linux/can.h>
#include <linux/can/core.h>
#include <net/net_namespace.h>
#include <net/sock.h>

#include "af_can.h"

static __initdata const char banner[] = KERN_INFO
	"can: controller area network core (" CAN_VERSION_STRING ")\n";

MODULE_DESCRIPTION("Controller Area Network PF_CAN core");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Urs Thuermann <urs.thuermann@volkswagen.de>, "
	      "Oliver Hartkopp <oliver.hartkopp@volkswagen.de>");

MODULE_ALIAS_NETPROTO(PF_CAN);

static int stats_timer __read_mostly = 1;
module_param(stats_timer, int, S_IRUGO);
MODULE_PARM_DESC(stats_timer, "enable timer for statistics (default:on)");

HLIST_HEAD(can_rx_dev_list);
static struct dev_rcv_lists can_rx_alldev_list;
static DEFINE_SPINLOCK(can_rcvlists_lock);

static struct kmem_cache *rcv_cache __read_mostly;


static struct can_proto *proto_tab[CAN_NPROTO] __read_mostly;
static DEFINE_SPINLOCK(proto_tab_lock);

struct timer_list can_stattimer;   
struct s_stats    can_stats;       
struct s_pstats   can_pstats;      



static int can_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	struct sock *sk = sock->sk;

	switch (cmd) {

	case SIOCGSTAMP:
		return sock_get_timestamp(sk, (struct timeval __user *)arg);

	default:
		return -ENOIOCTLCMD;
	}
}

static void can_sock_destruct(struct sock *sk)
{
	skb_queue_purge(&sk->sk_receive_queue);
}

static int can_create(struct net *net, struct socket *sock, int protocol)
{
	struct sock *sk;
	struct can_proto *cp;
	int err = 0;

	sock->state = SS_UNCONNECTED;

	if (protocol < 0 || protocol >= CAN_NPROTO)
		return -EINVAL;

	if (net != &init_net)
		return -EAFNOSUPPORT;

#ifdef CONFIG_MODULES
	
	if (!proto_tab[protocol]) {
		err = request_module("can-proto-%d", protocol);

		
		if (err && printk_ratelimit())
			printk(KERN_ERR "can: request_module "
			       "(can-proto-%d) failed.\n", protocol);
	}
#endif

	spin_lock(&proto_tab_lock);
	cp = proto_tab[protocol];
	if (cp && !try_module_get(cp->prot->owner))
		cp = NULL;
	spin_unlock(&proto_tab_lock);

	

	if (!cp)
		return -EPROTONOSUPPORT;

	if (cp->type != sock->type) {
		err = -EPROTONOSUPPORT;
		goto errout;
	}

	if (cp->capability >= 0 && !capable(cp->capability)) {
		err = -EPERM;
		goto errout;
	}

	sock->ops = cp->ops;

	sk = sk_alloc(net, PF_CAN, GFP_KERNEL, cp->prot);
	if (!sk) {
		err = -ENOMEM;
		goto errout;
	}

	sock_init_data(sock, sk);
	sk->sk_destruct = can_sock_destruct;

	if (sk->sk_prot->init)
		err = sk->sk_prot->init(sk);

	if (err) {
		
		sock_orphan(sk);
		sock_put(sk);
	}

 errout:
	module_put(cp->prot->owner);
	return err;
}




int can_send(struct sk_buff *skb, int loop)
{
	struct sk_buff *newskb = NULL;
	struct can_frame *cf = (struct can_frame *)skb->data;
	int err;

	if (skb->len != sizeof(struct can_frame) || cf->can_dlc > 8) {
		kfree_skb(skb);
		return -EINVAL;
	}

	if (skb->dev->type != ARPHRD_CAN) {
		kfree_skb(skb);
		return -EPERM;
	}

	if (!(skb->dev->flags & IFF_UP)) {
		kfree_skb(skb);
		return -ENETDOWN;
	}

	skb->protocol = htons(ETH_P_CAN);
	skb_reset_network_header(skb);
	skb_reset_transport_header(skb);

	if (loop) {
		

		
		skb->pkt_type = PACKET_LOOPBACK;

		

		if (!(skb->dev->flags & IFF_ECHO)) {
			
			newskb = skb_clone(skb, GFP_ATOMIC);
			if (!newskb) {
				kfree_skb(skb);
				return -ENOMEM;
			}

			newskb->sk = skb->sk;
			newskb->ip_summed = CHECKSUM_UNNECESSARY;
			newskb->pkt_type = PACKET_BROADCAST;
		}
	} else {
		
		skb->pkt_type = PACKET_HOST;
	}

	
	err = dev_queue_xmit(skb);
	if (err > 0)
		err = net_xmit_errno(err);

	if (err) {
		kfree_skb(newskb);
		return err;
	}

	if (newskb)
		netif_rx_ni(newskb);

	
	can_stats.tx_frames++;
	can_stats.tx_frames_delta++;

	return 0;
}
EXPORT_SYMBOL(can_send);



static struct dev_rcv_lists *find_dev_rcv_lists(struct net_device *dev)
{
	struct dev_rcv_lists *d = NULL;
	struct hlist_node *n;

	

	hlist_for_each_entry_rcu(d, n, &can_rx_dev_list, list) {
		if (d->dev == dev)
			break;
	}

	return n ? d : NULL;
}


static struct hlist_head *find_rcv_list(canid_t *can_id, canid_t *mask,
					struct dev_rcv_lists *d)
{
	canid_t inv = *can_id & CAN_INV_FILTER; 

	
	if (*mask & CAN_ERR_FLAG) {
		
		*mask &= CAN_ERR_MASK;
		return &d->rx[RX_ERR];
	}

	

#define CAN_EFF_RTR_FLAGS (CAN_EFF_FLAG | CAN_RTR_FLAG)

	
	if ((*mask & CAN_EFF_FLAG) && !(*can_id & CAN_EFF_FLAG))
		*mask &= (CAN_SFF_MASK | CAN_EFF_RTR_FLAGS);

	
	*can_id &= *mask;

	
	if (inv)
		return &d->rx[RX_INV];

	
	if (!(*mask))
		return &d->rx[RX_ALL];

	
	if (((*mask & CAN_EFF_RTR_FLAGS) == CAN_EFF_RTR_FLAGS)
	    && !(*can_id & CAN_RTR_FLAG)) {

		if (*can_id & CAN_EFF_FLAG) {
			if (*mask == (CAN_EFF_MASK | CAN_EFF_RTR_FLAGS)) {
				
				return &d->rx[RX_EFF];
			}
		} else {
			if (*mask == (CAN_SFF_MASK | CAN_EFF_RTR_FLAGS))
				return &d->rx_sff[*can_id];
		}
	}

	
	return &d->rx[RX_FIL];
}


int can_rx_register(struct net_device *dev, canid_t can_id, canid_t mask,
		    void (*func)(struct sk_buff *, void *), void *data,
		    char *ident)
{
	struct receiver *r;
	struct hlist_head *rl;
	struct dev_rcv_lists *d;
	int err = 0;

	

	r = kmem_cache_alloc(rcv_cache, GFP_KERNEL);
	if (!r)
		return -ENOMEM;

	spin_lock(&can_rcvlists_lock);

	d = find_dev_rcv_lists(dev);
	if (d) {
		rl = find_rcv_list(&can_id, &mask, d);

		r->can_id  = can_id;
		r->mask    = mask;
		r->matches = 0;
		r->func    = func;
		r->data    = data;
		r->ident   = ident;

		hlist_add_head_rcu(&r->list, rl);
		d->entries++;

		can_pstats.rcv_entries++;
		if (can_pstats.rcv_entries_max < can_pstats.rcv_entries)
			can_pstats.rcv_entries_max = can_pstats.rcv_entries;
	} else {
		kmem_cache_free(rcv_cache, r);
		err = -ENODEV;
	}

	spin_unlock(&can_rcvlists_lock);

	return err;
}
EXPORT_SYMBOL(can_rx_register);


static void can_rx_delete_device(struct rcu_head *rp)
{
	struct dev_rcv_lists *d = container_of(rp, struct dev_rcv_lists, rcu);

	kfree(d);
}


static void can_rx_delete_receiver(struct rcu_head *rp)
{
	struct receiver *r = container_of(rp, struct receiver, rcu);

	kmem_cache_free(rcv_cache, r);
}


void can_rx_unregister(struct net_device *dev, canid_t can_id, canid_t mask,
		       void (*func)(struct sk_buff *, void *), void *data)
{
	struct receiver *r = NULL;
	struct hlist_head *rl;
	struct hlist_node *next;
	struct dev_rcv_lists *d;

	spin_lock(&can_rcvlists_lock);

	d = find_dev_rcv_lists(dev);
	if (!d) {
		printk(KERN_ERR "BUG: receive list not found for "
		       "dev %s, id %03X, mask %03X\n",
		       DNAME(dev), can_id, mask);
		goto out;
	}

	rl = find_rcv_list(&can_id, &mask, d);

	

	hlist_for_each_entry_rcu(r, next, rl, list) {
		if (r->can_id == can_id && r->mask == mask
		    && r->func == func && r->data == data)
			break;
	}

	

	if (!next) {
		printk(KERN_ERR "BUG: receive list entry not found for "
		       "dev %s, id %03X, mask %03X\n",
		       DNAME(dev), can_id, mask);
		r = NULL;
		d = NULL;
		goto out;
	}

	hlist_del_rcu(&r->list);
	d->entries--;

	if (can_pstats.rcv_entries > 0)
		can_pstats.rcv_entries--;

	
	if (d->remove_on_zero_entries && !d->entries)
		hlist_del_rcu(&d->list);
	else
		d = NULL;

 out:
	spin_unlock(&can_rcvlists_lock);

	
	if (r)
		call_rcu(&r->rcu, can_rx_delete_receiver);

	
	if (d)
		call_rcu(&d->rcu, can_rx_delete_device);
}
EXPORT_SYMBOL(can_rx_unregister);

static inline void deliver(struct sk_buff *skb, struct receiver *r)
{
	r->func(skb, r->data);
	r->matches++;
}

static int can_rcv_filter(struct dev_rcv_lists *d, struct sk_buff *skb)
{
	struct receiver *r;
	struct hlist_node *n;
	int matches = 0;
	struct can_frame *cf = (struct can_frame *)skb->data;
	canid_t can_id = cf->can_id;

	if (d->entries == 0)
		return 0;

	if (can_id & CAN_ERR_FLAG) {
		
		hlist_for_each_entry_rcu(r, n, &d->rx[RX_ERR], list) {
			if (can_id & r->mask) {
				deliver(skb, r);
				matches++;
			}
		}
		return matches;
	}

	
	hlist_for_each_entry_rcu(r, n, &d->rx[RX_ALL], list) {
		deliver(skb, r);
		matches++;
	}

	
	hlist_for_each_entry_rcu(r, n, &d->rx[RX_FIL], list) {
		if ((can_id & r->mask) == r->can_id) {
			deliver(skb, r);
			matches++;
		}
	}

	
	hlist_for_each_entry_rcu(r, n, &d->rx[RX_INV], list) {
		if ((can_id & r->mask) != r->can_id) {
			deliver(skb, r);
			matches++;
		}
	}

	
	if (can_id & CAN_RTR_FLAG)
		return matches;

	if (can_id & CAN_EFF_FLAG) {
		hlist_for_each_entry_rcu(r, n, &d->rx[RX_EFF], list) {
			if (r->can_id == can_id) {
				deliver(skb, r);
				matches++;
			}
		}
	} else {
		can_id &= CAN_SFF_MASK;
		hlist_for_each_entry_rcu(r, n, &d->rx_sff[can_id], list) {
			deliver(skb, r);
			matches++;
		}
	}

	return matches;
}

static int can_rcv(struct sk_buff *skb, struct net_device *dev,
		   struct packet_type *pt, struct net_device *orig_dev)
{
	struct dev_rcv_lists *d;
	struct can_frame *cf = (struct can_frame *)skb->data;
	int matches;

	if (!net_eq(dev_net(dev), &init_net))
		goto drop;

	if (WARN_ONCE(dev->type != ARPHRD_CAN ||
		      skb->len != sizeof(struct can_frame) ||
		      cf->can_dlc > 8,
		      "PF_CAN: dropped non conform skbuf: "
		      "dev type %d, len %d, can_dlc %d\n",
		      dev->type, skb->len, cf->can_dlc))
		goto drop;

	
	can_stats.rx_frames++;
	can_stats.rx_frames_delta++;

	rcu_read_lock();

	
	matches = can_rcv_filter(&can_rx_alldev_list, skb);

	
	d = find_dev_rcv_lists(dev);
	if (d)
		matches += can_rcv_filter(d, skb);

	rcu_read_unlock();

	
	consume_skb(skb);

	if (matches > 0) {
		can_stats.matches++;
		can_stats.matches_delta++;
	}

	return NET_RX_SUCCESS;

drop:
	kfree_skb(skb);
	return NET_RX_DROP;
}




int can_proto_register(struct can_proto *cp)
{
	int proto = cp->protocol;
	int err = 0;

	if (proto < 0 || proto >= CAN_NPROTO) {
		printk(KERN_ERR "can: protocol number %d out of range\n",
		       proto);
		return -EINVAL;
	}

	err = proto_register(cp->prot, 0);
	if (err < 0)
		return err;

	spin_lock(&proto_tab_lock);
	if (proto_tab[proto]) {
		printk(KERN_ERR "can: protocol %d already registered\n",
		       proto);
		err = -EBUSY;
	} else {
		proto_tab[proto] = cp;

		
		if (!cp->ops->ioctl)
			cp->ops->ioctl = can_ioctl;
	}
	spin_unlock(&proto_tab_lock);

	if (err < 0)
		proto_unregister(cp->prot);

	return err;
}
EXPORT_SYMBOL(can_proto_register);


void can_proto_unregister(struct can_proto *cp)
{
	int proto = cp->protocol;

	spin_lock(&proto_tab_lock);
	if (!proto_tab[proto]) {
		printk(KERN_ERR "BUG: can: protocol %d is not registered\n",
		       proto);
	}
	proto_tab[proto] = NULL;
	spin_unlock(&proto_tab_lock);

	proto_unregister(cp->prot);
}
EXPORT_SYMBOL(can_proto_unregister);


static int can_notifier(struct notifier_block *nb, unsigned long msg,
			void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct dev_rcv_lists *d;

	if (!net_eq(dev_net(dev), &init_net))
		return NOTIFY_DONE;

	if (dev->type != ARPHRD_CAN)
		return NOTIFY_DONE;

	switch (msg) {

	case NETDEV_REGISTER:

		

		d = kzalloc(sizeof(*d), GFP_KERNEL);
		if (!d) {
			printk(KERN_ERR
			       "can: allocation of receive list failed\n");
			return NOTIFY_DONE;
		}
		d->dev = dev;

		spin_lock(&can_rcvlists_lock);
		hlist_add_head_rcu(&d->list, &can_rx_dev_list);
		spin_unlock(&can_rcvlists_lock);

		break;

	case NETDEV_UNREGISTER:
		spin_lock(&can_rcvlists_lock);

		d = find_dev_rcv_lists(dev);
		if (d) {
			if (d->entries) {
				d->remove_on_zero_entries = 1;
				d = NULL;
			} else
				hlist_del_rcu(&d->list);
		} else
			printk(KERN_ERR "can: notifier: receive list not "
			       "found for dev %s\n", dev->name);

		spin_unlock(&can_rcvlists_lock);

		if (d)
			call_rcu(&d->rcu, can_rx_delete_device);

		break;
	}

	return NOTIFY_DONE;
}



static struct packet_type can_packet __read_mostly = {
	.type = cpu_to_be16(ETH_P_CAN),
	.dev  = NULL,
	.func = can_rcv,
};

static struct net_proto_family can_family_ops __read_mostly = {
	.family = PF_CAN,
	.create = can_create,
	.owner  = THIS_MODULE,
};


static struct notifier_block can_netdev_notifier __read_mostly = {
	.notifier_call = can_notifier,
};

static __init int can_init(void)
{
	printk(banner);

	rcv_cache = kmem_cache_create("can_receiver", sizeof(struct receiver),
				      0, 0, NULL);
	if (!rcv_cache)
		return -ENOMEM;

	

	spin_lock(&can_rcvlists_lock);
	hlist_add_head_rcu(&can_rx_alldev_list.list, &can_rx_dev_list);
	spin_unlock(&can_rcvlists_lock);

	if (stats_timer) {
		
		setup_timer(&can_stattimer, can_stat_update, 0);
		mod_timer(&can_stattimer, round_jiffies(jiffies + HZ));
	} else
		can_stattimer.function = NULL;

	can_init_proc();

	
	sock_register(&can_family_ops);
	register_netdevice_notifier(&can_netdev_notifier);
	dev_add_pack(&can_packet);

	return 0;
}

static __exit void can_exit(void)
{
	struct dev_rcv_lists *d;
	struct hlist_node *n, *next;

	if (stats_timer)
		del_timer(&can_stattimer);

	can_remove_proc();

	
	dev_remove_pack(&can_packet);
	unregister_netdevice_notifier(&can_netdev_notifier);
	sock_unregister(PF_CAN);

	
	spin_lock(&can_rcvlists_lock);
	hlist_del(&can_rx_alldev_list.list);
	hlist_for_each_entry_safe(d, n, next, &can_rx_dev_list, list) {
		hlist_del(&d->list);
		kfree(d);
	}
	spin_unlock(&can_rcvlists_lock);

	rcu_barrier(); 

	kmem_cache_destroy(rcv_cache);
}

module_init(can_init);
module_exit(can_exit);
