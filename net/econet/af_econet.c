

#include <linux/module.h>

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/in.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/route.h>
#include <linux/inet.h>
#include <linux/etherdevice.h>
#include <linux/if_arp.h>
#include <linux/wireless.h>
#include <linux/skbuff.h>
#include <linux/udp.h>
#include <net/sock.h>
#include <net/inet_common.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/if_ec.h>
#include <net/udp.h>
#include <net/ip.h>
#include <linux/spinlock.h>
#include <linux/rcupdate.h>
#include <linux/bitops.h>
#include <linux/mutex.h>

#include <asm/uaccess.h>
#include <asm/system.h>

static const struct proto_ops econet_ops;
static struct hlist_head econet_sklist;
static DEFINE_RWLOCK(econet_lock);
static DEFINE_MUTEX(econet_mutex);


static struct net_device *net2dev_map[256];

#define EC_PORT_IP	0xd2

#ifdef CONFIG_ECONET_AUNUDP
static DEFINE_SPINLOCK(aun_queue_lock);
static struct socket *udpsock;
#define AUN_PORT	0x8000


struct aunhdr
{
	unsigned char code;		
	unsigned char port;
	unsigned char cb;
	unsigned char pad;
	unsigned long handle;
};

static unsigned long aun_seq;


static struct sk_buff_head aun_queue;
static struct timer_list ab_cleanup_timer;

#endif		


struct ec_cb
{
	struct sockaddr_ec sec;
	unsigned long cookie;		
#ifdef CONFIG_ECONET_AUNUDP
	int done;
	unsigned long seq;		
	unsigned long timeout;		
	unsigned long start;		
#endif
#ifdef CONFIG_ECONET_NATIVE
	void (*sent)(struct sk_buff *, int result);
#endif
};

static void econet_remove_socket(struct hlist_head *list, struct sock *sk)
{
	write_lock_bh(&econet_lock);
	sk_del_node_init(sk);
	write_unlock_bh(&econet_lock);
}

static void econet_insert_socket(struct hlist_head *list, struct sock *sk)
{
	write_lock_bh(&econet_lock);
	sk_add_node(sk, list);
	write_unlock_bh(&econet_lock);
}



static int econet_recvmsg(struct kiocb *iocb, struct socket *sock,
			  struct msghdr *msg, size_t len, int flags)
{
	struct sock *sk = sock->sk;
	struct sk_buff *skb;
	size_t copied;
	int err;

	msg->msg_namelen = sizeof(struct sockaddr_ec);

	mutex_lock(&econet_mutex);

	

	skb=skb_recv_datagram(sk,flags,flags&MSG_DONTWAIT,&err);

	

	if(skb==NULL)
		goto out;

	

	copied = skb->len;
	if (copied > len)
	{
		copied=len;
		msg->msg_flags|=MSG_TRUNC;
	}

	
	err = memcpy_toiovec(msg->msg_iov, skb->data, copied);
	if (err)
		goto out_free;
	sk->sk_stamp = skb->tstamp;

	if (msg->msg_name)
		memcpy(msg->msg_name, skb->cb, msg->msg_namelen);

	
	err = copied;

out_free:
	skb_free_datagram(sk, skb);
out:
	mutex_unlock(&econet_mutex);
	return err;
}



static int econet_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len)
{
	struct sockaddr_ec *sec = (struct sockaddr_ec *)uaddr;
	struct sock *sk;
	struct econet_sock *eo;

	

	if (addr_len < sizeof(struct sockaddr_ec) ||
	    sec->sec_family != AF_ECONET)
		return -EINVAL;

	mutex_lock(&econet_mutex);

	sk = sock->sk;
	eo = ec_sk(sk);

	eo->cb	    = sec->cb;
	eo->port    = sec->port;
	eo->station = sec->addr.station;
	eo->net	    = sec->addr.net;

	mutex_unlock(&econet_mutex);

	return 0;
}

#if defined(CONFIG_ECONET_AUNUDP) || defined(CONFIG_ECONET_NATIVE)


static void tx_result(struct sock *sk, unsigned long cookie, int result)
{
	struct sk_buff *skb = alloc_skb(0, GFP_ATOMIC);
	struct ec_cb *eb;
	struct sockaddr_ec *sec;

	if (skb == NULL)
	{
		printk(KERN_DEBUG "ec: memory squeeze, transmit result dropped.\n");
		return;
	}

	eb = (struct ec_cb *)&skb->cb;
	sec = (struct sockaddr_ec *)&eb->sec;
	memset(sec, 0, sizeof(struct sockaddr_ec));
	sec->cookie = cookie;
	sec->type = ECTYPE_TRANSMIT_STATUS | result;
	sec->sec_family = AF_ECONET;

	if (sock_queue_rcv_skb(sk, skb) < 0)
		kfree_skb(skb);
}
#endif

#ifdef CONFIG_ECONET_NATIVE


static void ec_tx_done(struct sk_buff *skb, int result)
{
	struct ec_cb *eb = (struct ec_cb *)&skb->cb;
	tx_result(skb->sk, eb->cookie, result);
}
#endif



static int econet_sendmsg(struct kiocb *iocb, struct socket *sock,
			  struct msghdr *msg, size_t len)
{
	struct sock *sk = sock->sk;
	struct sockaddr_ec *saddr=(struct sockaddr_ec *)msg->msg_name;
	struct net_device *dev;
	struct ec_addr addr;
	int err;
	unsigned char port, cb;
#if defined(CONFIG_ECONET_AUNUDP) || defined(CONFIG_ECONET_NATIVE)
	struct sk_buff *skb;
	struct ec_cb *eb;
#endif
#ifdef CONFIG_ECONET_AUNUDP
	struct msghdr udpmsg;
	struct iovec iov[msg->msg_iovlen+1];
	struct aunhdr ah;
	struct sockaddr_in udpdest;
	__kernel_size_t size;
	int i;
	mm_segment_t oldfs;
#endif

	

	if (msg->msg_flags & ~(MSG_DONTWAIT|MSG_CMSG_COMPAT))
		return -EINVAL;

	

	mutex_lock(&econet_mutex);

	if (saddr == NULL) {
		struct econet_sock *eo = ec_sk(sk);

		addr.station = eo->station;
		addr.net     = eo->net;
		port	     = eo->port;
		cb	     = eo->cb;
	} else {
		if (msg->msg_namelen < sizeof(struct sockaddr_ec)) {
			mutex_unlock(&econet_mutex);
			return -EINVAL;
		}
		addr.station = saddr->addr.station;
		addr.net = saddr->addr.net;
		port = saddr->port;
		cb = saddr->cb;
	}

	
	dev = net2dev_map[addr.net];

	
	if (dev == NULL) {
		dev = net2dev_map[0];
		
		if (dev == NULL) {
			mutex_unlock(&econet_mutex);
			return -ENETDOWN;
		}
	}

	if (len + 15 > dev->mtu) {
		mutex_unlock(&econet_mutex);
		return -EMSGSIZE;
	}

	if (dev->type == ARPHRD_ECONET) {
		
#ifdef CONFIG_ECONET_NATIVE
		unsigned short proto = 0;
		int res;

		dev_hold(dev);

		skb = sock_alloc_send_skb(sk, len+LL_ALLOCATED_SPACE(dev),
					  msg->msg_flags & MSG_DONTWAIT, &err);
		if (skb==NULL)
			goto out_unlock;

		skb_reserve(skb, LL_RESERVED_SPACE(dev));
		skb_reset_network_header(skb);

		eb = (struct ec_cb *)&skb->cb;

		
		eb->cookie = saddr->cookie;
		eb->sec = *saddr;
		eb->sent = ec_tx_done;

		err = -EINVAL;
		res = dev_hard_header(skb, dev, ntohs(proto), &addr, NULL, len);
		if (res < 0)
			goto out_free;
		if (res > 0) {
			struct ec_framehdr *fh;
			
			fh = (struct ec_framehdr *)(skb->data);
			fh->cb = cb;
			fh->port = port;
			if (sock->type != SOCK_DGRAM) {
				skb_reset_tail_pointer(skb);
				skb->len = 0;
			}
		}

		
		err = memcpy_fromiovec(skb_put(skb,len), msg->msg_iov, len);
		skb->protocol = proto;
		skb->dev = dev;
		skb->priority = sk->sk_priority;
		if (err)
			goto out_free;

		err = -ENETDOWN;
		if (!(dev->flags & IFF_UP))
			goto out_free;

		

		dev_queue_xmit(skb);
		dev_put(dev);
		mutex_unlock(&econet_mutex);
		return(len);

	out_free:
		kfree_skb(skb);
	out_unlock:
		if (dev)
			dev_put(dev);
#else
		err = -EPROTOTYPE;
#endif
		mutex_unlock(&econet_mutex);

		return err;
	}

#ifdef CONFIG_ECONET_AUNUDP
	

	if (udpsock == NULL) {
		mutex_unlock(&econet_mutex);
		return -ENETDOWN;		
	}

	

	memset(&udpdest, 0, sizeof(udpdest));
	udpdest.sin_family = AF_INET;
	udpdest.sin_port = htons(AUN_PORT);

	
	{
		struct in_device *idev;
		unsigned long network = 0;

		rcu_read_lock();
		idev = __in_dev_get_rcu(dev);
		if (idev) {
			if (idev->ifa_list)
				network = ntohl(idev->ifa_list->ifa_address) &
					0xffffff00;		
		}
		rcu_read_unlock();
		udpdest.sin_addr.s_addr = htonl(network | addr.station);
	}

	ah.port = port;
	ah.cb = cb & 0x7f;
	ah.code = 2;		
	ah.pad = 0;

	
	size = sizeof(struct aunhdr);
	
	iov[0].iov_base = (void *)&ah;
	iov[0].iov_len = size;
	for (i = 0; i < msg->msg_iovlen; i++) {
		void __user *base = msg->msg_iov[i].iov_base;
		size_t len = msg->msg_iov[i].iov_len;
		
		if (!access_ok(VERIFY_READ, base, len)) {
			mutex_unlock(&econet_mutex);
			return -EFAULT;
		}
		iov[i+1].iov_base = base;
		iov[i+1].iov_len = len;
		size += len;
	}

	
	if ((skb = sock_alloc_send_skb(sk, 0,
				       msg->msg_flags & MSG_DONTWAIT,
				       &err)) == NULL) {
		mutex_unlock(&econet_mutex);
		return err;
	}

	eb = (struct ec_cb *)&skb->cb;

	eb->cookie = saddr->cookie;
	eb->timeout = (5*HZ);
	eb->start = jiffies;
	ah.handle = aun_seq;
	eb->seq = (aun_seq++);
	eb->sec = *saddr;

	skb_queue_tail(&aun_queue, skb);

	udpmsg.msg_name = (void *)&udpdest;
	udpmsg.msg_namelen = sizeof(udpdest);
	udpmsg.msg_iov = &iov[0];
	udpmsg.msg_iovlen = msg->msg_iovlen + 1;
	udpmsg.msg_control = NULL;
	udpmsg.msg_controllen = 0;
	udpmsg.msg_flags=0;

	oldfs = get_fs(); set_fs(KERNEL_DS);	
	err = sock_sendmsg(udpsock, &udpmsg, size);
	set_fs(oldfs);
#else
	err = -EPROTOTYPE;
#endif
	mutex_unlock(&econet_mutex);

	return err;
}



static int econet_getname(struct socket *sock, struct sockaddr *uaddr,
			  int *uaddr_len, int peer)
{
	struct sock *sk;
	struct econet_sock *eo;
	struct sockaddr_ec *sec = (struct sockaddr_ec *)uaddr;

	if (peer)
		return -EOPNOTSUPP;

	memset(sec, 0, sizeof(*sec));
	mutex_lock(&econet_mutex);

	sk = sock->sk;
	eo = ec_sk(sk);

	sec->sec_family	  = AF_ECONET;
	sec->port	  = eo->port;
	sec->addr.station = eo->station;
	sec->addr.net	  = eo->net;

	mutex_unlock(&econet_mutex);

	*uaddr_len = sizeof(*sec);
	return 0;
}

static void econet_destroy_timer(unsigned long data)
{
	struct sock *sk=(struct sock *)data;

	if (!sk_has_allocations(sk)) {
		sk_free(sk);
		return;
	}

	sk->sk_timer.expires = jiffies + 10 * HZ;
	add_timer(&sk->sk_timer);
	printk(KERN_DEBUG "econet socket destroy delayed\n");
}



static int econet_release(struct socket *sock)
{
	struct sock *sk;

	mutex_lock(&econet_mutex);

	sk = sock->sk;
	if (!sk)
		goto out_unlock;

	econet_remove_socket(&econet_sklist, sk);

	

	sk->sk_state_change(sk);	

	sock_orphan(sk);

	

	skb_queue_purge(&sk->sk_receive_queue);

	if (sk_has_allocations(sk)) {
		sk->sk_timer.data     = (unsigned long)sk;
		sk->sk_timer.expires  = jiffies + HZ;
		sk->sk_timer.function = econet_destroy_timer;
		add_timer(&sk->sk_timer);

		goto out_unlock;
	}

	sk_free(sk);

out_unlock:
	mutex_unlock(&econet_mutex);
	return 0;
}

static struct proto econet_proto = {
	.name	  = "ECONET",
	.owner	  = THIS_MODULE,
	.obj_size = sizeof(struct econet_sock),
};



static int econet_create(struct net *net, struct socket *sock, int protocol)
{
	struct sock *sk;
	struct econet_sock *eo;
	int err;

	if (net != &init_net)
		return -EAFNOSUPPORT;

	
	if (sock->type != SOCK_DGRAM)
		return -ESOCKTNOSUPPORT;

	sock->state = SS_UNCONNECTED;

	err = -ENOBUFS;
	sk = sk_alloc(net, PF_ECONET, GFP_KERNEL, &econet_proto);
	if (sk == NULL)
		goto out;

	sk->sk_reuse = 1;
	sock->ops = &econet_ops;
	sock_init_data(sock, sk);

	eo = ec_sk(sk);
	sock_reset_flag(sk, SOCK_ZAPPED);
	sk->sk_family = PF_ECONET;
	eo->num = protocol;

	econet_insert_socket(&econet_sklist, sk);
	return(0);
out:
	return err;
}



static int ec_dev_ioctl(struct socket *sock, unsigned int cmd, void __user *arg)
{
	struct ifreq ifr;
	struct ec_device *edev;
	struct net_device *dev;
	struct sockaddr_ec *sec;
	int err;

	

	if (copy_from_user(&ifr, arg, sizeof(struct ifreq)))
		return -EFAULT;

	if ((dev = dev_get_by_name(&init_net, ifr.ifr_name)) == NULL)
		return -ENODEV;

	sec = (struct sockaddr_ec *)&ifr.ifr_addr;

	mutex_lock(&econet_mutex);

	err = 0;
	switch (cmd) {
	case SIOCSIFADDR:
		edev = dev->ec_ptr;
		if (edev == NULL) {
			
			edev = kzalloc(sizeof(struct ec_device), GFP_KERNEL);
			if (edev == NULL) {
				err = -ENOMEM;
				break;
			}
			dev->ec_ptr = edev;
		} else
			net2dev_map[edev->net] = NULL;
		edev->station = sec->addr.station;
		edev->net = sec->addr.net;
		net2dev_map[sec->addr.net] = dev;
		if (!net2dev_map[0])
			net2dev_map[0] = dev;
		break;

	case SIOCGIFADDR:
		edev = dev->ec_ptr;
		if (edev == NULL) {
			err = -ENODEV;
			break;
		}
		memset(sec, 0, sizeof(struct sockaddr_ec));
		sec->addr.station = edev->station;
		sec->addr.net = edev->net;
		sec->sec_family = AF_ECONET;
		dev_put(dev);
		if (copy_to_user(arg, &ifr, sizeof(struct ifreq)))
			err = -EFAULT;
		break;

	default:
		err = -EINVAL;
		break;
	}

	mutex_unlock(&econet_mutex);

	dev_put(dev);

	return err;
}



static int econet_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	struct sock *sk = sock->sk;
	void __user *argp = (void __user *)arg;

	switch(cmd) {
		case SIOCGSTAMP:
			return sock_get_timestamp(sk, argp);

		case SIOCGSTAMPNS:
			return sock_get_timestampns(sk, argp);

		case SIOCSIFADDR:
		case SIOCGIFADDR:
			return ec_dev_ioctl(sock, cmd, argp);
			break;

		default:
			return -ENOIOCTLCMD;
	}
	
	return 0;
}

static struct net_proto_family econet_family_ops = {
	.family =	PF_ECONET,
	.create =	econet_create,
	.owner	=	THIS_MODULE,
};

static const struct proto_ops econet_ops = {
	.family =	PF_ECONET,
	.owner =	THIS_MODULE,
	.release =	econet_release,
	.bind =		econet_bind,
	.connect =	sock_no_connect,
	.socketpair =	sock_no_socketpair,
	.accept =	sock_no_accept,
	.getname =	econet_getname,
	.poll =		datagram_poll,
	.ioctl =	econet_ioctl,
	.listen =	sock_no_listen,
	.shutdown =	sock_no_shutdown,
	.setsockopt =	sock_no_setsockopt,
	.getsockopt =	sock_no_getsockopt,
	.sendmsg =	econet_sendmsg,
	.recvmsg =	econet_recvmsg,
	.mmap =		sock_no_mmap,
	.sendpage =	sock_no_sendpage,
};

#if defined(CONFIG_ECONET_AUNUDP) || defined(CONFIG_ECONET_NATIVE)


static struct sock *ec_listening_socket(unsigned char port, unsigned char
				 station, unsigned char net)
{
	struct sock *sk;
	struct hlist_node *node;

	sk_for_each(sk, node, &econet_sklist) {
		struct econet_sock *opt = ec_sk(sk);
		if ((opt->port == port || opt->port == 0) &&
		    (opt->station == station || opt->station == 0) &&
		    (opt->net == net || opt->net == 0))
			goto found;
	}
	sk = NULL;
found:
	return sk;
}



static int ec_queue_packet(struct sock *sk, struct sk_buff *skb,
			   unsigned char stn, unsigned char net,
			   unsigned char cb, unsigned char port)
{
	struct ec_cb *eb = (struct ec_cb *)&skb->cb;
	struct sockaddr_ec *sec = (struct sockaddr_ec *)&eb->sec;

	memset(sec, 0, sizeof(struct sockaddr_ec));
	sec->sec_family = AF_ECONET;
	sec->type = ECTYPE_PACKET_RECEIVED;
	sec->port = port;
	sec->cb = cb;
	sec->addr.net = net;
	sec->addr.station = stn;

	return sock_queue_rcv_skb(sk, skb);
}
#endif

#ifdef CONFIG_ECONET_AUNUDP


static void aun_send_response(__u32 addr, unsigned long seq, int code, int cb)
{
	struct sockaddr_in sin = {
		.sin_family = AF_INET,
		.sin_port = htons(AUN_PORT),
		.sin_addr = {.s_addr = addr}
	};
	struct aunhdr ah = {.code = code, .cb = cb, .handle = seq};
	struct kvec iov = {.iov_base = (void *)&ah, .iov_len = sizeof(ah)};
	struct msghdr udpmsg;

	udpmsg.msg_name = (void *)&sin;
	udpmsg.msg_namelen = sizeof(sin);
	udpmsg.msg_control = NULL;
	udpmsg.msg_controllen = 0;
	udpmsg.msg_flags=0;

	kernel_sendmsg(udpsock, &udpmsg, &iov, 1, sizeof(ah));
}




static void aun_incoming(struct sk_buff *skb, struct aunhdr *ah, size_t len)
{
	struct iphdr *ip = ip_hdr(skb);
	unsigned char stn = ntohl(ip->saddr) & 0xff;
	struct sock *sk;
	struct sk_buff *newskb;
	struct ec_device *edev = skb->dev->ec_ptr;

	if (! edev)
		goto bad;

	if ((sk = ec_listening_socket(ah->port, stn, edev->net)) == NULL)
		goto bad;		

	newskb = alloc_skb((len - sizeof(struct aunhdr) + 15) & ~15,
			   GFP_ATOMIC);
	if (newskb == NULL)
	{
		printk(KERN_DEBUG "AUN: memory squeeze, dropping packet.\n");
		
		goto bad;
	}

	memcpy(skb_put(newskb, len - sizeof(struct aunhdr)), (void *)(ah+1),
	       len - sizeof(struct aunhdr));

	if (ec_queue_packet(sk, newskb, stn, edev->net, ah->cb, ah->port))
	{
		
		kfree_skb(newskb);
		goto bad;
	}

	aun_send_response(ip->saddr, ah->handle, 3, 0);
	return;

bad:
	aun_send_response(ip->saddr, ah->handle, 4, 0);
}



static void aun_tx_ack(unsigned long seq, int result)
{
	struct sk_buff *skb;
	unsigned long flags;
	struct ec_cb *eb;

	spin_lock_irqsave(&aun_queue_lock, flags);
	skb_queue_walk(&aun_queue, skb) {
		eb = (struct ec_cb *)&skb->cb;
		if (eb->seq == seq)
			goto foundit;
	}
	spin_unlock_irqrestore(&aun_queue_lock, flags);
	printk(KERN_DEBUG "AUN: unknown sequence %ld\n", seq);
	return;

foundit:
	tx_result(skb->sk, eb->cookie, result);
	skb_unlink(skb, &aun_queue);
	spin_unlock_irqrestore(&aun_queue_lock, flags);
	kfree_skb(skb);
}



static void aun_data_available(struct sock *sk, int slen)
{
	int err;
	struct sk_buff *skb;
	unsigned char *data;
	struct aunhdr *ah;
	struct iphdr *ip;
	size_t len;

	while ((skb = skb_recv_datagram(sk, 0, 1, &err)) == NULL) {
		if (err == -EAGAIN) {
			printk(KERN_ERR "AUN: no data available?!");
			return;
		}
		printk(KERN_DEBUG "AUN: recvfrom() error %d\n", -err);
	}

	data = skb_transport_header(skb) + sizeof(struct udphdr);
	ah = (struct aunhdr *)data;
	len = skb->len - sizeof(struct udphdr);
	ip = ip_hdr(skb);

	switch (ah->code)
	{
	case 2:
		aun_incoming(skb, ah, len);
		break;
	case 3:
		aun_tx_ack(ah->handle, ECTYPE_TRANSMIT_OK);
		break;
	case 4:
		aun_tx_ack(ah->handle, ECTYPE_TRANSMIT_NOT_LISTENING);
		break;
#if 0
		
	case 5:
		aun_send_response(ip->saddr, ah->handle, 6, ah->cb);
		break;
#endif
	default:
		printk(KERN_DEBUG "unknown AUN packet (type %d)\n", data[0]);
	}

	skb_free_datagram(sk, skb);
}



static void ab_cleanup(unsigned long h)
{
	struct sk_buff *skb, *n;
	unsigned long flags;

	spin_lock_irqsave(&aun_queue_lock, flags);
	skb_queue_walk_safe(&aun_queue, skb, n) {
		struct ec_cb *eb = (struct ec_cb *)&skb->cb;
		if ((jiffies - eb->start) > eb->timeout) {
			tx_result(skb->sk, eb->cookie,
				  ECTYPE_TRANSMIT_NOT_PRESENT);
			skb_unlink(skb, &aun_queue);
			kfree_skb(skb);
		}
	}
	spin_unlock_irqrestore(&aun_queue_lock, flags);

	mod_timer(&ab_cleanup_timer, jiffies + (HZ*2));
}

static int __init aun_udp_initialise(void)
{
	int error;
	struct sockaddr_in sin;

	skb_queue_head_init(&aun_queue);
	spin_lock_init(&aun_queue_lock);
	setup_timer(&ab_cleanup_timer, ab_cleanup, 0);
	ab_cleanup_timer.expires = jiffies + (HZ*2);
	add_timer(&ab_cleanup_timer);

	memset(&sin, 0, sizeof(sin));
	sin.sin_port = htons(AUN_PORT);

	
	if ((error = sock_create_kern(PF_INET, SOCK_DGRAM, 0, &udpsock)) < 0)
	{
		printk("AUN: socket error %d\n", -error);
		return error;
	}

	udpsock->sk->sk_reuse = 1;
	udpsock->sk->sk_allocation = GFP_ATOMIC; 

	error = udpsock->ops->bind(udpsock, (struct sockaddr *)&sin,
				sizeof(sin));
	if (error < 0)
	{
		printk("AUN: bind error %d\n", -error);
		goto release;
	}

	udpsock->sk->sk_data_ready = aun_data_available;

	return 0;

release:
	sock_release(udpsock);
	udpsock = NULL;
	return error;
}
#endif

#ifdef CONFIG_ECONET_NATIVE



static int econet_rcv(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev)
{
	struct ec_framehdr *hdr;
	struct sock *sk;
	struct ec_device *edev = dev->ec_ptr;

	if (!net_eq(dev_net(dev), &init_net))
		goto drop;

	if (skb->pkt_type == PACKET_OTHERHOST)
		goto drop;

	if (!edev)
		goto drop;

	if ((skb = skb_share_check(skb, GFP_ATOMIC)) == NULL)
		return NET_RX_DROP;

	if (!pskb_may_pull(skb, sizeof(struct ec_framehdr)))
		goto drop;

	hdr = (struct ec_framehdr *) skb->data;

	
	if (hdr->port == EC_PORT_IP) {
		skb->protocol = htons(ETH_P_IP);
		skb_pull(skb, sizeof(struct ec_framehdr));
		netif_rx(skb);
		return NET_RX_SUCCESS;
	}

	sk = ec_listening_socket(hdr->port, hdr->src_stn, hdr->src_net);
	if (!sk)
		goto drop;

	if (ec_queue_packet(sk, skb, edev->net, hdr->src_stn, hdr->cb,
			    hdr->port))
		goto drop;

	return NET_RX_SUCCESS;

drop:
	kfree_skb(skb);
	return NET_RX_DROP;
}

static struct packet_type econet_packet_type __read_mostly = {
	.type =		cpu_to_be16(ETH_P_ECONET),
	.func =		econet_rcv,
};

static void econet_hw_initialise(void)
{
	dev_add_pack(&econet_packet_type);
}

#endif

static int econet_notifier(struct notifier_block *this, unsigned long msg, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct ec_device *edev;

	if (!net_eq(dev_net(dev), &init_net))
		return NOTIFY_DONE;

	switch (msg) {
	case NETDEV_UNREGISTER:
		
		edev = dev->ec_ptr;
		if (edev)
		{
			if (net2dev_map[0] == dev)
				net2dev_map[0] = NULL;
			net2dev_map[edev->net] = NULL;
			kfree(edev);
			dev->ec_ptr = NULL;
		}
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block econet_netdev_notifier = {
	.notifier_call =econet_notifier,
};

static void __exit econet_proto_exit(void)
{
#ifdef CONFIG_ECONET_AUNUDP
	del_timer(&ab_cleanup_timer);
	if (udpsock)
		sock_release(udpsock);
#endif
	unregister_netdevice_notifier(&econet_netdev_notifier);
#ifdef CONFIG_ECONET_NATIVE
	dev_remove_pack(&econet_packet_type);
#endif
	sock_unregister(econet_family_ops.family);
	proto_unregister(&econet_proto);
}

static int __init econet_proto_init(void)
{
	int err = proto_register(&econet_proto, 0);

	if (err != 0)
		goto out;
	sock_register(&econet_family_ops);
#ifdef CONFIG_ECONET_AUNUDP
	spin_lock_init(&aun_queue_lock);
	aun_udp_initialise();
#endif
#ifdef CONFIG_ECONET_NATIVE
	econet_hw_initialise();
#endif
	register_netdevice_notifier(&econet_netdev_notifier);
out:
	return err;
}

module_init(econet_proto_init);
module_exit(econet_proto_exit);

MODULE_LICENSE("GPL");
MODULE_ALIAS_NETPROTO(PF_ECONET);
