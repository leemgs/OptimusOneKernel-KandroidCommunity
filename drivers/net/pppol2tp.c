



#include <linux/module.h>
#include <linux/string.h>
#include <linux/list.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/jiffies.h>

#include <linux/netdevice.h>
#include <linux/net.h>
#include <linux/inetdevice.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/if_pppox.h>
#include <linux/if_pppol2tp.h>
#include <net/sock.h>
#include <linux/ppp_channel.h>
#include <linux/ppp_defs.h>
#include <linux/if_ppp.h>
#include <linux/file.h>
#include <linux/hash.h>
#include <linux/sort.h>
#include <linux/proc_fs.h>
#include <linux/nsproxy.h>
#include <net/net_namespace.h>
#include <net/netns/generic.h>
#include <net/dst.h>
#include <net/ip.h>
#include <net/udp.h>
#include <net/xfrm.h>

#include <asm/byteorder.h>
#include <asm/atomic.h>


#define PPPOL2TP_DRV_VERSION	"V1.0"


#define L2TP_HDRFLAG_T	   0x8000
#define L2TP_HDRFLAG_L	   0x4000
#define L2TP_HDRFLAG_S	   0x0800
#define L2TP_HDRFLAG_O	   0x0200
#define L2TP_HDRFLAG_P	   0x0100

#define L2TP_HDR_VER_MASK  0x000F
#define L2TP_HDR_VER	   0x0002


#define PPPOL2TP_HEADER_OVERHEAD	40


#define L2TP_TUNNEL_MAGIC	0x42114DDA
#define L2TP_SESSION_MAGIC	0x0C04EB7D

#define PPPOL2TP_HASH_BITS	4
#define PPPOL2TP_HASH_SIZE	(1 << PPPOL2TP_HASH_BITS)


#define PPPOL2TP_DEFAULT_DEBUG_FLAGS	0

#define PRINTK(_mask, _type, _lvl, _fmt, args...)			\
	do {								\
		if ((_mask) & (_type))					\
			printk(_lvl "PPPOL2TP: " _fmt, ##args);		\
	} while(0)


#define PPPOL2TP_L2TP_HDR_SIZE_SEQ		10
#define PPPOL2TP_L2TP_HDR_SIZE_NOSEQ		6

struct pppol2tp_tunnel;


struct pppol2tp_session
{
	int			magic;		
	int			owner;		

	struct sock		*sock;		
	struct sock		*tunnel_sock;	

	struct pppol2tp_addr	tunnel_addr;	

	struct pppol2tp_tunnel	*tunnel;	

	char			name[20];	
	int			mtu;
	int			mru;
	int			flags;		
	unsigned		recv_seq:1;	
	unsigned		send_seq:1;	
	unsigned		lns_mode:1;	
	int			debug;		
	int			reorder_timeout; 
	u16			nr;		
	u16			ns;		
	struct sk_buff_head	reorder_q;	
	struct pppol2tp_ioc_stats stats;
	struct hlist_node	hlist;		
};


struct pppol2tp_tunnel
{
	int			magic;		
	rwlock_t		hlist_lock;	
	struct hlist_head	session_hlist[PPPOL2TP_HASH_SIZE];
						
	int			debug;		
	char			name[12];	
	struct pppol2tp_ioc_stats stats;

	void (*old_sk_destruct)(struct sock *);

	struct sock		*sock;		
	struct list_head	list;		
	struct net		*pppol2tp_net;	

	atomic_t		ref_count;
};


struct pppol2tp_skb_cb {
	u16			ns;
	u16			nr;
	u16			has_seq;
	u16			length;
	unsigned long		expires;
};

#define PPPOL2TP_SKB_CB(skb)	((struct pppol2tp_skb_cb *) &skb->cb[sizeof(struct inet_skb_parm)])

static int pppol2tp_xmit(struct ppp_channel *chan, struct sk_buff *skb);
static void pppol2tp_tunnel_free(struct pppol2tp_tunnel *tunnel);

static atomic_t pppol2tp_tunnel_count;
static atomic_t pppol2tp_session_count;
static struct ppp_channel_ops pppol2tp_chan_ops = { pppol2tp_xmit , NULL };
static const struct proto_ops pppol2tp_ops;


static int pppol2tp_net_id;
struct pppol2tp_net {
	struct list_head pppol2tp_tunnel_list;
	rwlock_t pppol2tp_tunnel_list_lock;
};

static inline struct pppol2tp_net *pppol2tp_pernet(struct net *net)
{
	BUG_ON(!net);

	return net_generic(net, pppol2tp_net_id);
}


static inline struct pppol2tp_session *pppol2tp_sock_to_session(struct sock *sk)
{
	struct pppol2tp_session *session;

	if (sk == NULL)
		return NULL;

	sock_hold(sk);
	session = (struct pppol2tp_session *)(sk->sk_user_data);
	if (session == NULL) {
		sock_put(sk);
		goto out;
	}

	BUG_ON(session->magic != L2TP_SESSION_MAGIC);
out:
	return session;
}

static inline struct pppol2tp_tunnel *pppol2tp_sock_to_tunnel(struct sock *sk)
{
	struct pppol2tp_tunnel *tunnel;

	if (sk == NULL)
		return NULL;

	sock_hold(sk);
	tunnel = (struct pppol2tp_tunnel *)(sk->sk_user_data);
	if (tunnel == NULL) {
		sock_put(sk);
		goto out;
	}

	BUG_ON(tunnel->magic != L2TP_TUNNEL_MAGIC);
out:
	return tunnel;
}


static inline void pppol2tp_tunnel_inc_refcount(struct pppol2tp_tunnel *tunnel)
{
	atomic_inc(&tunnel->ref_count);
}

static inline void pppol2tp_tunnel_dec_refcount(struct pppol2tp_tunnel *tunnel)
{
	if (atomic_dec_and_test(&tunnel->ref_count))
		pppol2tp_tunnel_free(tunnel);
}


static inline struct hlist_head *
pppol2tp_session_id_hash(struct pppol2tp_tunnel *tunnel, u16 session_id)
{
	unsigned long hash_val = (unsigned long) session_id;
	return &tunnel->session_hlist[hash_long(hash_val, PPPOL2TP_HASH_BITS)];
}


static struct pppol2tp_session *
pppol2tp_session_find(struct pppol2tp_tunnel *tunnel, u16 session_id)
{
	struct hlist_head *session_list =
		pppol2tp_session_id_hash(tunnel, session_id);
	struct pppol2tp_session *session;
	struct hlist_node *walk;

	read_lock_bh(&tunnel->hlist_lock);
	hlist_for_each_entry(session, walk, session_list, hlist) {
		if (session->tunnel_addr.s_session == session_id) {
			read_unlock_bh(&tunnel->hlist_lock);
			return session;
		}
	}
	read_unlock_bh(&tunnel->hlist_lock);

	return NULL;
}


static struct pppol2tp_tunnel *pppol2tp_tunnel_find(struct net *net, u16 tunnel_id)
{
	struct pppol2tp_tunnel *tunnel;
	struct pppol2tp_net *pn = pppol2tp_pernet(net);

	read_lock_bh(&pn->pppol2tp_tunnel_list_lock);
	list_for_each_entry(tunnel, &pn->pppol2tp_tunnel_list, list) {
		if (tunnel->stats.tunnel_id == tunnel_id) {
			read_unlock_bh(&pn->pppol2tp_tunnel_list_lock);
			return tunnel;
		}
	}
	read_unlock_bh(&pn->pppol2tp_tunnel_list_lock);

	return NULL;
}




static void pppol2tp_recv_queue_skb(struct pppol2tp_session *session, struct sk_buff *skb)
{
	struct sk_buff *skbp;
	struct sk_buff *tmp;
	u16 ns = PPPOL2TP_SKB_CB(skb)->ns;

	spin_lock_bh(&session->reorder_q.lock);
	skb_queue_walk_safe(&session->reorder_q, skbp, tmp) {
		if (PPPOL2TP_SKB_CB(skbp)->ns > ns) {
			__skb_queue_before(&session->reorder_q, skbp, skb);
			PRINTK(session->debug, PPPOL2TP_MSG_SEQ, KERN_DEBUG,
			       "%s: pkt %hu, inserted before %hu, reorder_q len=%d\n",
			       session->name, ns, PPPOL2TP_SKB_CB(skbp)->ns,
			       skb_queue_len(&session->reorder_q));
			session->stats.rx_oos_packets++;
			goto out;
		}
	}

	__skb_queue_tail(&session->reorder_q, skb);

out:
	spin_unlock_bh(&session->reorder_q.lock);
}


static void pppol2tp_recv_dequeue_skb(struct pppol2tp_session *session, struct sk_buff *skb)
{
	struct pppol2tp_tunnel *tunnel = session->tunnel;
	int length = PPPOL2TP_SKB_CB(skb)->length;
	struct sock *session_sock = NULL;

	
	skb_orphan(skb);

	tunnel->stats.rx_packets++;
	tunnel->stats.rx_bytes += length;
	session->stats.rx_packets++;
	session->stats.rx_bytes += length;

	if (PPPOL2TP_SKB_CB(skb)->has_seq) {
		
		session->nr++;
		PRINTK(session->debug, PPPOL2TP_MSG_SEQ, KERN_DEBUG,
		       "%s: updated nr to %hu\n", session->name, session->nr);
	}

	
	session_sock = session->sock;
	if (session_sock->sk_state & PPPOX_BOUND) {
		struct pppox_sock *po;
		PRINTK(session->debug, PPPOL2TP_MSG_DATA, KERN_DEBUG,
		       "%s: recv %d byte data frame, passing to ppp\n",
		       session->name, length);

		
		secpath_reset(skb);
		skb_dst_drop(skb);
		nf_reset(skb);

		po = pppox_sk(session_sock);
		ppp_input(&po->chan, skb);
	} else {
		PRINTK(session->debug, PPPOL2TP_MSG_DATA, KERN_INFO,
		       "%s: socket not bound\n", session->name);

		
		session->stats.rx_errors++;
		kfree_skb(skb);
	}

	sock_put(session->sock);
}


static void pppol2tp_recv_dequeue(struct pppol2tp_session *session)
{
	struct sk_buff *skb;
	struct sk_buff *tmp;

	
	spin_lock_bh(&session->reorder_q.lock);
	skb_queue_walk_safe(&session->reorder_q, skb, tmp) {
		if (time_after(jiffies, PPPOL2TP_SKB_CB(skb)->expires)) {
			session->stats.rx_seq_discards++;
			session->stats.rx_errors++;
			PRINTK(session->debug, PPPOL2TP_MSG_SEQ, KERN_DEBUG,
			       "%s: oos pkt %hu len %d discarded (too old), "
			       "waiting for %hu, reorder_q_len=%d\n",
			       session->name, PPPOL2TP_SKB_CB(skb)->ns,
			       PPPOL2TP_SKB_CB(skb)->length, session->nr,
			       skb_queue_len(&session->reorder_q));
			__skb_unlink(skb, &session->reorder_q);
			kfree_skb(skb);
			sock_put(session->sock);
			continue;
		}

		if (PPPOL2TP_SKB_CB(skb)->has_seq) {
			if (PPPOL2TP_SKB_CB(skb)->ns != session->nr) {
				PRINTK(session->debug, PPPOL2TP_MSG_SEQ, KERN_DEBUG,
				       "%s: holding oos pkt %hu len %d, "
				       "waiting for %hu, reorder_q_len=%d\n",
				       session->name, PPPOL2TP_SKB_CB(skb)->ns,
				       PPPOL2TP_SKB_CB(skb)->length, session->nr,
				       skb_queue_len(&session->reorder_q));
				goto out;
			}
		}
		__skb_unlink(skb, &session->reorder_q);

		
		spin_unlock_bh(&session->reorder_q.lock);
		pppol2tp_recv_dequeue_skb(session, skb);
		spin_lock_bh(&session->reorder_q.lock);
	}

out:
	spin_unlock_bh(&session->reorder_q.lock);
}

static inline int pppol2tp_verify_udp_checksum(struct sock *sk,
					       struct sk_buff *skb)
{
	struct udphdr *uh = udp_hdr(skb);
	u16 ulen = ntohs(uh->len);
	struct inet_sock *inet;
	__wsum psum;

	if (sk->sk_no_check || skb_csum_unnecessary(skb) || !uh->check)
		return 0;

	inet = inet_sk(sk);
	psum = csum_tcpudp_nofold(inet->saddr, inet->daddr, ulen,
				  IPPROTO_UDP, 0);

	if ((skb->ip_summed == CHECKSUM_COMPLETE) &&
	    !csum_fold(csum_add(psum, skb->csum)))
		return 0;

	skb->csum = psum;

	return __skb_checksum_complete(skb);
}


static int pppol2tp_recv_core(struct sock *sock, struct sk_buff *skb)
{
	struct pppol2tp_session *session = NULL;
	struct pppol2tp_tunnel *tunnel;
	unsigned char *ptr, *optr;
	u16 hdrflags;
	u16 tunnel_id, session_id;
	int length;
	int offset;

	tunnel = pppol2tp_sock_to_tunnel(sock);
	if (tunnel == NULL)
		goto no_tunnel;

	if (tunnel->sock && pppol2tp_verify_udp_checksum(tunnel->sock, skb))
		goto discard_bad_csum;

	
	__skb_pull(skb, sizeof(struct udphdr));

	
	if (!pskb_may_pull(skb, 12)) {
		PRINTK(tunnel->debug, PPPOL2TP_MSG_DATA, KERN_INFO,
		       "%s: recv short packet (len=%d)\n", tunnel->name, skb->len);
		goto error;
	}

	
	optr = ptr = skb->data;

	
	hdrflags = ntohs(*(__be16*)ptr);

	
	if (tunnel->debug & PPPOL2TP_MSG_DATA) {
		length = min(16u, skb->len);
		if (!pskb_may_pull(skb, length))
			goto error;

		printk(KERN_DEBUG "%s: recv: ", tunnel->name);

		offset = 0;
		do {
			printk(" %02X", ptr[offset]);
		} while (++offset < length);

		printk("\n");
	}

	
	length = skb->len;

	
	if (hdrflags & L2TP_HDRFLAG_T) {
		PRINTK(tunnel->debug, PPPOL2TP_MSG_DATA, KERN_DEBUG,
		       "%s: recv control packet, len=%d\n", tunnel->name, length);
		goto error;
	}

	
	ptr += 2;

	
	if (hdrflags & L2TP_HDRFLAG_L)
		ptr += 2;

	
	tunnel_id = ntohs(*(__be16 *) ptr);
	ptr += 2;
	session_id = ntohs(*(__be16 *) ptr);
	ptr += 2;

	
	session = pppol2tp_session_find(tunnel, session_id);
	if (!session) {
		
		PRINTK(tunnel->debug, PPPOL2TP_MSG_DATA, KERN_INFO,
		       "%s: no socket found (%hu/%hu). Passing up.\n",
		       tunnel->name, tunnel_id, session_id);
		goto error;
	}
	sock_hold(session->sock);

	

	
	if (hdrflags & L2TP_HDRFLAG_S) {
		u16 ns, nr;
		ns = ntohs(*(__be16 *) ptr);
		ptr += 2;
		nr = ntohs(*(__be16 *) ptr);
		ptr += 2;

		
		if ((!session->lns_mode) && (!session->send_seq)) {
			PRINTK(session->debug, PPPOL2TP_MSG_SEQ, KERN_INFO,
			       "%s: requested to enable seq numbers by LNS\n",
			       session->name);
			session->send_seq = -1;
		}

		
		PPPOL2TP_SKB_CB(skb)->ns = ns;
		PPPOL2TP_SKB_CB(skb)->nr = nr;
		PPPOL2TP_SKB_CB(skb)->has_seq = 1;

		PRINTK(session->debug, PPPOL2TP_MSG_SEQ, KERN_DEBUG,
		       "%s: recv data ns=%hu, nr=%hu, session nr=%hu\n",
		       session->name, ns, nr, session->nr);
	} else {
		
		if (session->recv_seq) {
			PRINTK(session->debug, PPPOL2TP_MSG_SEQ, KERN_WARNING,
			       "%s: recv data has no seq numbers when required. "
			       "Discarding\n", session->name);
			session->stats.rx_seq_discards++;
			goto discard;
		}

		
		if ((!session->lns_mode) && (session->send_seq)) {
			PRINTK(session->debug, PPPOL2TP_MSG_SEQ, KERN_INFO,
			       "%s: requested to disable seq numbers by LNS\n",
			       session->name);
			session->send_seq = 0;
		} else if (session->send_seq) {
			PRINTK(session->debug, PPPOL2TP_MSG_SEQ, KERN_WARNING,
			       "%s: recv data has no seq numbers when required. "
			       "Discarding\n", session->name);
			session->stats.rx_seq_discards++;
			goto discard;
		}

		
		PPPOL2TP_SKB_CB(skb)->has_seq = 0;
	}

	
	if (hdrflags & L2TP_HDRFLAG_O) {
		offset = ntohs(*(__be16 *)ptr);
		ptr += 2 + offset;
	}

	offset = ptr - optr;
	if (!pskb_may_pull(skb, offset))
		goto discard;

	__skb_pull(skb, offset);

	
	if (!pskb_may_pull(skb, 2))
		goto discard;

	if ((skb->data[0] == 0xff) && (skb->data[1] == 0x03))
		skb_pull(skb, 2);

	
	PPPOL2TP_SKB_CB(skb)->length = length;
	PPPOL2TP_SKB_CB(skb)->expires = jiffies +
		(session->reorder_timeout ? session->reorder_timeout : HZ);

	
	if (PPPOL2TP_SKB_CB(skb)->has_seq) {
		if (session->reorder_timeout != 0) {
			
			pppol2tp_recv_queue_skb(session, skb);
		} else {
			
			if (PPPOL2TP_SKB_CB(skb)->ns != session->nr) {
				session->stats.rx_seq_discards++;
				PRINTK(session->debug, PPPOL2TP_MSG_SEQ, KERN_DEBUG,
				       "%s: oos pkt %hu len %d discarded, "
				       "waiting for %hu, reorder_q_len=%d\n",
				       session->name, PPPOL2TP_SKB_CB(skb)->ns,
				       PPPOL2TP_SKB_CB(skb)->length, session->nr,
				       skb_queue_len(&session->reorder_q));
				goto discard;
			}
			skb_queue_tail(&session->reorder_q, skb);
		}
	} else {
		
		skb_queue_tail(&session->reorder_q, skb);
	}

	
	pppol2tp_recv_dequeue(session);

	return 0;

discard:
	session->stats.rx_errors++;
	kfree_skb(skb);
	sock_put(session->sock);
	sock_put(sock);

	return 0;

discard_bad_csum:
	LIMIT_NETDEBUG("%s: UDP: bad checksum\n", tunnel->name);
	UDP_INC_STATS_USER(&init_net, UDP_MIB_INERRORS, 0);
	tunnel->stats.rx_errors++;
	kfree_skb(skb);

	return 0;

error:
	
	__skb_push(skb, sizeof(struct udphdr));
	sock_put(sock);

no_tunnel:
	return 1;
}


static int pppol2tp_udp_encap_recv(struct sock *sk, struct sk_buff *skb)
{
	struct pppol2tp_tunnel *tunnel;

	tunnel = pppol2tp_sock_to_tunnel(sk);
	if (tunnel == NULL)
		goto pass_up;

	PRINTK(tunnel->debug, PPPOL2TP_MSG_DATA, KERN_DEBUG,
	       "%s: received %d bytes\n", tunnel->name, skb->len);

	if (pppol2tp_recv_core(sk, skb))
		goto pass_up_put;

	sock_put(sk);
	return 0;

pass_up_put:
	sock_put(sk);
pass_up:
	return 1;
}


static int pppol2tp_recvmsg(struct kiocb *iocb, struct socket *sock,
			    struct msghdr *msg, size_t len,
			    int flags)
{
	int err;
	struct sk_buff *skb;
	struct sock *sk = sock->sk;

	err = -EIO;
	if (sk->sk_state & PPPOX_BOUND)
		goto end;

	msg->msg_namelen = 0;

	err = 0;
	skb = skb_recv_datagram(sk, flags & ~MSG_DONTWAIT,
				flags & MSG_DONTWAIT, &err);
	if (!skb)
		goto end;

	if (len > skb->len)
		len = skb->len;
	else if (len < skb->len)
		msg->msg_flags |= MSG_TRUNC;

	err = skb_copy_datagram_iovec(skb, 0, msg->msg_iov, len);
	if (likely(err == 0))
		err = len;

	kfree_skb(skb);
end:
	return err;
}




static inline int pppol2tp_l2tp_header_len(struct pppol2tp_session *session)
{
	if (session->send_seq)
		return PPPOL2TP_L2TP_HDR_SIZE_SEQ;

	return PPPOL2TP_L2TP_HDR_SIZE_NOSEQ;
}


static void pppol2tp_build_l2tp_header(struct pppol2tp_session *session,
				       void *buf)
{
	__be16 *bufp = buf;
	u16 flags = L2TP_HDR_VER;

	if (session->send_seq)
		flags |= L2TP_HDRFLAG_S;

	
	*bufp++ = htons(flags);
	*bufp++ = htons(session->tunnel_addr.d_tunnel);
	*bufp++ = htons(session->tunnel_addr.d_session);
	if (session->send_seq) {
		*bufp++ = htons(session->ns);
		*bufp++ = 0;
		session->ns++;
		PRINTK(session->debug, PPPOL2TP_MSG_SEQ, KERN_DEBUG,
		       "%s: updated ns to %hu\n", session->name, session->ns);
	}
}


static int pppol2tp_sendmsg(struct kiocb *iocb, struct socket *sock, struct msghdr *m,
			    size_t total_len)
{
	static const unsigned char ppph[2] = { 0xff, 0x03 };
	struct sock *sk = sock->sk;
	struct inet_sock *inet;
	__wsum csum;
	struct sk_buff *skb;
	int error;
	int hdr_len;
	struct pppol2tp_session *session;
	struct pppol2tp_tunnel *tunnel;
	struct udphdr *uh;
	unsigned int len;
	struct sock *sk_tun;
	u16 udp_len;

	error = -ENOTCONN;
	if (sock_flag(sk, SOCK_DEAD) || !(sk->sk_state & PPPOX_CONNECTED))
		goto error;

	
	error = -EBADF;
	session = pppol2tp_sock_to_session(sk);
	if (session == NULL)
		goto error;

	sk_tun = session->tunnel_sock;
	tunnel = pppol2tp_sock_to_tunnel(sk_tun);
	if (tunnel == NULL)
		goto error_put_sess;

	
	hdr_len = pppol2tp_l2tp_header_len(session);

	
	error = -ENOMEM;
	skb = sock_wmalloc(sk, NET_SKB_PAD + sizeof(struct iphdr) +
			   sizeof(struct udphdr) + hdr_len +
			   sizeof(ppph) + total_len,
			   0, GFP_KERNEL);
	if (!skb)
		goto error_put_sess_tun;

	
	skb_reserve(skb, NET_SKB_PAD);
	skb_reset_network_header(skb);
	skb_reserve(skb, sizeof(struct iphdr));
	skb_reset_transport_header(skb);

	
	inet = inet_sk(sk_tun);
	udp_len = hdr_len + sizeof(ppph) + total_len;
	uh = (struct udphdr *) skb->data;
	uh->source = inet->sport;
	uh->dest = inet->dport;
	uh->len = htons(udp_len);
	uh->check = 0;
	skb_put(skb, sizeof(struct udphdr));

	
	pppol2tp_build_l2tp_header(session, skb->data);
	skb_put(skb, hdr_len);

	
	skb->data[0] = ppph[0];
	skb->data[1] = ppph[1];
	skb_put(skb, 2);

	
	error = memcpy_fromiovec(skb->data, m->msg_iov, total_len);
	if (error < 0) {
		kfree_skb(skb);
		goto error_put_sess_tun;
	}
	skb_put(skb, total_len);

	
	if (sk_tun->sk_no_check == UDP_CSUM_NOXMIT)
		skb->ip_summed = CHECKSUM_NONE;
	else if (!(skb_dst(skb)->dev->features & NETIF_F_V4_CSUM)) {
		skb->ip_summed = CHECKSUM_COMPLETE;
		csum = skb_checksum(skb, 0, udp_len, 0);
		uh->check = csum_tcpudp_magic(inet->saddr, inet->daddr,
					      udp_len, IPPROTO_UDP, csum);
		if (uh->check == 0)
			uh->check = CSUM_MANGLED_0;
	} else {
		skb->ip_summed = CHECKSUM_PARTIAL;
		skb->csum_start = skb_transport_header(skb) - skb->head;
		skb->csum_offset = offsetof(struct udphdr, check);
		uh->check = ~csum_tcpudp_magic(inet->saddr, inet->daddr,
					       udp_len, IPPROTO_UDP, 0);
	}

	
	if (session->send_seq)
		PRINTK(session->debug, PPPOL2TP_MSG_DATA, KERN_DEBUG,
		       "%s: send %Zd bytes, ns=%hu\n", session->name,
		       total_len, session->ns - 1);
	else
		PRINTK(session->debug, PPPOL2TP_MSG_DATA, KERN_DEBUG,
		       "%s: send %Zd bytes\n", session->name, total_len);

	if (session->debug & PPPOL2TP_MSG_DATA) {
		int i;
		unsigned char *datap = skb->data;

		printk(KERN_DEBUG "%s: xmit:", session->name);
		for (i = 0; i < total_len; i++) {
			printk(" %02X", *datap++);
			if (i == 15) {
				printk(" ...");
				break;
			}
		}
		printk("\n");
	}

	
	len = skb->len;
	error = ip_queue_xmit(skb, 1);

	
	if (error >= 0) {
		tunnel->stats.tx_packets++;
		tunnel->stats.tx_bytes += len;
		session->stats.tx_packets++;
		session->stats.tx_bytes += len;
	} else {
		tunnel->stats.tx_errors++;
		session->stats.tx_errors++;
	}

	return error;

error_put_sess_tun:
	sock_put(session->tunnel_sock);
error_put_sess:
	sock_put(sk);
error:
	return error;
}


static void pppol2tp_sock_wfree(struct sk_buff *skb)
{
	sock_put(skb->sk);
}


static inline void pppol2tp_skb_set_owner_w(struct sk_buff *skb, struct sock *sk)
{
	sock_hold(sk);
	skb->sk = sk;
	skb->destructor = pppol2tp_sock_wfree;
}


static int pppol2tp_xmit(struct ppp_channel *chan, struct sk_buff *skb)
{
	static const u8 ppph[2] = { 0xff, 0x03 };
	struct sock *sk = (struct sock *) chan->private;
	struct sock *sk_tun;
	int hdr_len;
	u16 udp_len;
	struct pppol2tp_session *session;
	struct pppol2tp_tunnel *tunnel;
	int rc;
	int headroom;
	int data_len = skb->len;
	struct inet_sock *inet;
	__wsum csum;
	struct udphdr *uh;
	unsigned int len;
	int old_headroom;
	int new_headroom;

	if (sock_flag(sk, SOCK_DEAD) || !(sk->sk_state & PPPOX_CONNECTED))
		goto abort;

	
	session = pppol2tp_sock_to_session(sk);
	if (session == NULL)
		goto abort;

	sk_tun = session->tunnel_sock;
	if (sk_tun == NULL)
		goto abort_put_sess;
	tunnel = pppol2tp_sock_to_tunnel(sk_tun);
	if (tunnel == NULL)
		goto abort_put_sess;

	
	hdr_len = pppol2tp_l2tp_header_len(session);

	
	headroom = NET_SKB_PAD + sizeof(struct iphdr) +
		sizeof(struct udphdr) + hdr_len + sizeof(ppph);
	old_headroom = skb_headroom(skb);
	if (skb_cow_head(skb, headroom))
		goto abort_put_sess_tun;

	new_headroom = skb_headroom(skb);
	skb_orphan(skb);
	skb->truesize += new_headroom - old_headroom;

	
	__skb_push(skb, sizeof(ppph));
	skb->data[0] = ppph[0];
	skb->data[1] = ppph[1];

	
	pppol2tp_build_l2tp_header(session, __skb_push(skb, hdr_len));

	udp_len = sizeof(struct udphdr) + hdr_len + sizeof(ppph) + data_len;

	
	inet = inet_sk(sk_tun);
	__skb_push(skb, sizeof(*uh));
	skb_reset_transport_header(skb);
	uh = udp_hdr(skb);
	uh->source = inet->sport;
	uh->dest = inet->dport;
	uh->len = htons(udp_len);
	uh->check = 0;

	
	if (session->send_seq)
		PRINTK(session->debug, PPPOL2TP_MSG_DATA, KERN_DEBUG,
		       "%s: send %d bytes, ns=%hu\n", session->name,
		       data_len, session->ns - 1);
	else
		PRINTK(session->debug, PPPOL2TP_MSG_DATA, KERN_DEBUG,
		       "%s: send %d bytes\n", session->name, data_len);

	if (session->debug & PPPOL2TP_MSG_DATA) {
		int i;
		unsigned char *datap = skb->data;

		printk(KERN_DEBUG "%s: xmit:", session->name);
		for (i = 0; i < data_len; i++) {
			printk(" %02X", *datap++);
			if (i == 31) {
				printk(" ...");
				break;
			}
		}
		printk("\n");
	}

	memset(&(IPCB(skb)->opt), 0, sizeof(IPCB(skb)->opt));
	IPCB(skb)->flags &= ~(IPSKB_XFRM_TUNNEL_SIZE | IPSKB_XFRM_TRANSFORMED |
			      IPSKB_REROUTED);
	nf_reset(skb);

	
	skb_dst_drop(skb);
	skb_dst_set(skb, dst_clone(__sk_dst_get(sk_tun)));
	pppol2tp_skb_set_owner_w(skb, sk_tun);

	
	if (sk_tun->sk_no_check == UDP_CSUM_NOXMIT)
		skb->ip_summed = CHECKSUM_NONE;
	else if (!(skb_dst(skb)->dev->features & NETIF_F_V4_CSUM)) {
		skb->ip_summed = CHECKSUM_COMPLETE;
		csum = skb_checksum(skb, 0, udp_len, 0);
		uh->check = csum_tcpudp_magic(inet->saddr, inet->daddr,
					      udp_len, IPPROTO_UDP, csum);
		if (uh->check == 0)
			uh->check = CSUM_MANGLED_0;
	} else {
		skb->ip_summed = CHECKSUM_PARTIAL;
		skb->csum_start = skb_transport_header(skb) - skb->head;
		skb->csum_offset = offsetof(struct udphdr, check);
		uh->check = ~csum_tcpudp_magic(inet->saddr, inet->daddr,
					       udp_len, IPPROTO_UDP, 0);
	}

	
	len = skb->len;
	rc = ip_queue_xmit(skb, 1);

	
	if (rc >= 0) {
		tunnel->stats.tx_packets++;
		tunnel->stats.tx_bytes += len;
		session->stats.tx_packets++;
		session->stats.tx_bytes += len;
	} else {
		tunnel->stats.tx_errors++;
		session->stats.tx_errors++;
	}

	sock_put(sk_tun);
	sock_put(sk);
	return 1;

abort_put_sess_tun:
	sock_put(sk_tun);
abort_put_sess:
	sock_put(sk);
abort:
	
	kfree_skb(skb);
	return 1;
}




static void pppol2tp_tunnel_closeall(struct pppol2tp_tunnel *tunnel)
{
	int hash;
	struct hlist_node *walk;
	struct hlist_node *tmp;
	struct pppol2tp_session *session;
	struct sock *sk;

	BUG_ON(tunnel == NULL);

	PRINTK(tunnel->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
	       "%s: closing all sessions...\n", tunnel->name);

	write_lock_bh(&tunnel->hlist_lock);
	for (hash = 0; hash < PPPOL2TP_HASH_SIZE; hash++) {
again:
		hlist_for_each_safe(walk, tmp, &tunnel->session_hlist[hash]) {
			struct sk_buff *skb;

			session = hlist_entry(walk, struct pppol2tp_session, hlist);

			sk = session->sock;

			PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
			       "%s: closing session\n", session->name);

			hlist_del_init(&session->hlist);

			
			sock_hold(sk);
			write_unlock_bh(&tunnel->hlist_lock);
			lock_sock(sk);

			if (sk->sk_state & (PPPOX_CONNECTED | PPPOX_BOUND)) {
				pppox_unbind_sock(sk);
				sk->sk_state = PPPOX_DEAD;
				sk->sk_state_change(sk);
			}

			
			skb_queue_purge(&sk->sk_receive_queue);
			skb_queue_purge(&sk->sk_write_queue);
			while ((skb = skb_dequeue(&session->reorder_q))) {
				kfree_skb(skb);
				sock_put(sk);
			}

			release_sock(sk);
			sock_put(sk);

			
			write_lock_bh(&tunnel->hlist_lock);
			goto again;
		}
	}
	write_unlock_bh(&tunnel->hlist_lock);
}


static void pppol2tp_tunnel_free(struct pppol2tp_tunnel *tunnel)
{
	struct pppol2tp_net *pn = pppol2tp_pernet(tunnel->pppol2tp_net);

	
	write_lock_bh(&pn->pppol2tp_tunnel_list_lock);
	list_del_init(&tunnel->list);
	write_unlock_bh(&pn->pppol2tp_tunnel_list_lock);

	atomic_dec(&pppol2tp_tunnel_count);
	kfree(tunnel);
}


static void pppol2tp_tunnel_destruct(struct sock *sk)
{
	struct pppol2tp_tunnel *tunnel;

	tunnel = sk->sk_user_data;
	if (tunnel == NULL)
		goto end;

	PRINTK(tunnel->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
	       "%s: closing...\n", tunnel->name);

	
	pppol2tp_tunnel_closeall(tunnel);

	
	(udp_sk(sk))->encap_type = 0;
	(udp_sk(sk))->encap_rcv = NULL;

	
	tunnel->sock = NULL;
	sk->sk_destruct = tunnel->old_sk_destruct;
	sk->sk_user_data = NULL;

	
	if (sk->sk_destruct != NULL)
		(*sk->sk_destruct)(sk);

	pppol2tp_tunnel_dec_refcount(tunnel);

end:
	return;
}


static void pppol2tp_session_destruct(struct sock *sk)
{
	struct pppol2tp_session *session = NULL;

	if (sk->sk_user_data != NULL) {
		struct pppol2tp_tunnel *tunnel;

		session = sk->sk_user_data;
		if (session == NULL)
			goto out;

		BUG_ON(session->magic != L2TP_SESSION_MAGIC);

		
		tunnel = session->tunnel;
		if (tunnel != NULL) {
			BUG_ON(tunnel->magic != L2TP_TUNNEL_MAGIC);

			
			if (session->tunnel_addr.s_session != 0) {
				
				write_lock_bh(&tunnel->hlist_lock);
				hlist_del_init(&session->hlist);
				write_unlock_bh(&tunnel->hlist_lock);

				atomic_dec(&pppol2tp_session_count);
			}

			
			session->tunnel = NULL;
			session->tunnel_sock = NULL;
			pppol2tp_tunnel_dec_refcount(tunnel);
		}
	}

	kfree(session);
out:
	return;
}


static int pppol2tp_release(struct socket *sock)
{
	struct sock *sk = sock->sk;
	struct pppol2tp_session *session;
	int error;

	if (!sk)
		return 0;

	error = -EBADF;
	lock_sock(sk);
	if (sock_flag(sk, SOCK_DEAD) != 0)
		goto error;

	pppox_unbind_sock(sk);

	
	sk->sk_state = PPPOX_DEAD;
	sock_orphan(sk);
	sock->sk = NULL;

	session = pppol2tp_sock_to_session(sk);

	
	skb_queue_purge(&sk->sk_receive_queue);
	skb_queue_purge(&sk->sk_write_queue);
	if (session != NULL) {
		struct sk_buff *skb;
		while ((skb = skb_dequeue(&session->reorder_q))) {
			kfree_skb(skb);
			sock_put(sk);
		}
		sock_put(sk);
	}

	release_sock(sk);

	
	sock_put(sk);

	return 0;

error:
	release_sock(sk);
	return error;
}


static struct sock *pppol2tp_prepare_tunnel_socket(struct net *net,
					int fd, u16 tunnel_id, int *error)
{
	int err;
	struct socket *sock = NULL;
	struct sock *sk;
	struct pppol2tp_tunnel *tunnel;
	struct pppol2tp_net *pn;
	struct sock *ret = NULL;

	
	err = -EBADF;
	sock = sockfd_lookup(fd, &err);
	if (!sock) {
		PRINTK(-1, PPPOL2TP_MSG_CONTROL, KERN_ERR,
		       "tunl %hu: sockfd_lookup(fd=%d) returned %d\n",
		       tunnel_id, fd, err);
		goto err;
	}

	sk = sock->sk;

	
	err = -EPROTONOSUPPORT;
	if (sk->sk_protocol != IPPROTO_UDP) {
		PRINTK(-1, PPPOL2TP_MSG_CONTROL, KERN_ERR,
		       "tunl %hu: fd %d wrong protocol, got %d, expected %d\n",
		       tunnel_id, fd, sk->sk_protocol, IPPROTO_UDP);
		goto err;
	}
	err = -EAFNOSUPPORT;
	if (sock->ops->family != AF_INET) {
		PRINTK(-1, PPPOL2TP_MSG_CONTROL, KERN_ERR,
		       "tunl %hu: fd %d wrong family, got %d, expected %d\n",
		       tunnel_id, fd, sock->ops->family, AF_INET);
		goto err;
	}

	err = -ENOTCONN;

	
	tunnel = (struct pppol2tp_tunnel *)sk->sk_user_data;
	if (tunnel != NULL) {
		
		err = -EBUSY;
		BUG_ON(tunnel->magic != L2TP_TUNNEL_MAGIC);

		
		ret = tunnel->sock;
		goto out;
	}

	
	sk->sk_user_data = tunnel = kzalloc(sizeof(struct pppol2tp_tunnel), GFP_KERNEL);
	if (sk->sk_user_data == NULL) {
		err = -ENOMEM;
		goto err;
	}

	tunnel->magic = L2TP_TUNNEL_MAGIC;
	sprintf(&tunnel->name[0], "tunl %hu", tunnel_id);

	tunnel->stats.tunnel_id = tunnel_id;
	tunnel->debug = PPPOL2TP_DEFAULT_DEBUG_FLAGS;

	
	tunnel->old_sk_destruct = sk->sk_destruct;
	sk->sk_destruct = &pppol2tp_tunnel_destruct;

	tunnel->sock = sk;
	sk->sk_allocation = GFP_ATOMIC;

	
	rwlock_init(&tunnel->hlist_lock);

	
	tunnel->pppol2tp_net = net;
	pn = pppol2tp_pernet(net);

	
	INIT_LIST_HEAD(&tunnel->list);
	write_lock_bh(&pn->pppol2tp_tunnel_list_lock);
	list_add(&tunnel->list, &pn->pppol2tp_tunnel_list);
	write_unlock_bh(&pn->pppol2tp_tunnel_list_lock);
	atomic_inc(&pppol2tp_tunnel_count);

	
	pppol2tp_tunnel_inc_refcount(tunnel);

	
	(udp_sk(sk))->encap_type = UDP_ENCAP_L2TPINUDP;
	(udp_sk(sk))->encap_rcv = pppol2tp_udp_encap_recv;

	ret = tunnel->sock;

	*error = 0;
out:
	if (sock)
		sockfd_put(sock);

	return ret;

err:
	*error = err;
	goto out;
}

static struct proto pppol2tp_sk_proto = {
	.name	  = "PPPOL2TP",
	.owner	  = THIS_MODULE,
	.obj_size = sizeof(struct pppox_sock),
};


static int pppol2tp_create(struct net *net, struct socket *sock)
{
	int error = -ENOMEM;
	struct sock *sk;

	sk = sk_alloc(net, PF_PPPOX, GFP_KERNEL, &pppol2tp_sk_proto);
	if (!sk)
		goto out;

	sock_init_data(sock, sk);

	sock->state  = SS_UNCONNECTED;
	sock->ops    = &pppol2tp_ops;

	sk->sk_backlog_rcv = pppol2tp_recv_core;
	sk->sk_protocol	   = PX_PROTO_OL2TP;
	sk->sk_family	   = PF_PPPOX;
	sk->sk_state	   = PPPOX_NONE;
	sk->sk_type	   = SOCK_STREAM;
	sk->sk_destruct	   = pppol2tp_session_destruct;

	error = 0;

out:
	return error;
}


static int pppol2tp_connect(struct socket *sock, struct sockaddr *uservaddr,
			    int sockaddr_len, int flags)
{
	struct sock *sk = sock->sk;
	struct sockaddr_pppol2tp *sp = (struct sockaddr_pppol2tp *) uservaddr;
	struct pppox_sock *po = pppox_sk(sk);
	struct sock *tunnel_sock = NULL;
	struct pppol2tp_session *session = NULL;
	struct pppol2tp_tunnel *tunnel;
	struct dst_entry *dst;
	int error = 0;

	lock_sock(sk);

	error = -EINVAL;
	if (sp->sa_protocol != PX_PROTO_OL2TP)
		goto end;

	
	error = -EBUSY;
	if (sk->sk_state & PPPOX_CONNECTED)
		goto end;

	
	error = -EALREADY;
	if (sk->sk_user_data)
		goto end; 

	
	error = -EINVAL;
	if (sp->pppol2tp.s_tunnel == 0)
		goto end;

	
	if ((sp->pppol2tp.s_session == 0) && (sp->pppol2tp.d_session == 0)) {
		tunnel_sock = pppol2tp_prepare_tunnel_socket(sock_net(sk),
							     sp->pppol2tp.fd,
							     sp->pppol2tp.s_tunnel,
							     &error);
		if (tunnel_sock == NULL)
			goto end;

		tunnel = tunnel_sock->sk_user_data;
	} else {
		tunnel = pppol2tp_tunnel_find(sock_net(sk), sp->pppol2tp.s_tunnel);

		
		error = -ENOENT;
		if (tunnel == NULL)
			goto end;

		tunnel_sock = tunnel->sock;
	}

	
	error = -EEXIST;
	session = pppol2tp_session_find(tunnel, sp->pppol2tp.s_session);
	if (session != NULL)
		goto end;

	
	session = kzalloc(sizeof(struct pppol2tp_session), GFP_KERNEL);
	if (session == NULL) {
		error = -ENOMEM;
		goto end;
	}

	skb_queue_head_init(&session->reorder_q);

	session->magic	     = L2TP_SESSION_MAGIC;
	session->owner	     = current->pid;
	session->sock	     = sk;
	session->tunnel	     = tunnel;
	session->tunnel_sock = tunnel_sock;
	session->tunnel_addr = sp->pppol2tp;
	sprintf(&session->name[0], "sess %hu/%hu",
		session->tunnel_addr.s_tunnel,
		session->tunnel_addr.s_session);

	session->stats.tunnel_id  = session->tunnel_addr.s_tunnel;
	session->stats.session_id = session->tunnel_addr.s_session;

	INIT_HLIST_NODE(&session->hlist);

	
	session->debug = tunnel->debug;

	
	session->mtu = session->mru = 1500 - PPPOL2TP_HEADER_OVERHEAD;

	
	dst = sk_dst_get(sk);
	if (dst != NULL) {
		u32 pmtu = dst_mtu(__sk_dst_get(sk));
		if (pmtu != 0)
			session->mtu = session->mru = pmtu -
				PPPOL2TP_HEADER_OVERHEAD;
		dst_release(dst);
	}

	
	if ((session->tunnel_addr.s_session == 0) &&
	    (session->tunnel_addr.d_session == 0)) {
		error = 0;
		sk->sk_user_data = session;
		goto out_no_ppp;
	}

	
	tunnel = pppol2tp_sock_to_tunnel(tunnel_sock);
	if (tunnel == NULL) {
		error = -EBADF;
		goto end;
	}

	
	po->chan.hdrlen = PPPOL2TP_L2TP_HDR_SIZE_NOSEQ;

	po->chan.private = sk;
	po->chan.ops	 = &pppol2tp_chan_ops;
	po->chan.mtu	 = session->mtu;

	error = ppp_register_net_channel(sock_net(sk), &po->chan);
	if (error)
		goto end_put_tun;

	
	sk->sk_user_data = session;

	
	write_lock_bh(&tunnel->hlist_lock);
	hlist_add_head(&session->hlist,
		       pppol2tp_session_id_hash(tunnel,
						session->tunnel_addr.s_session));
	write_unlock_bh(&tunnel->hlist_lock);

	atomic_inc(&pppol2tp_session_count);

out_no_ppp:
	pppol2tp_tunnel_inc_refcount(tunnel);
	sk->sk_state = PPPOX_CONNECTED;
	PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
	       "%s: created\n", session->name);

end_put_tun:
	sock_put(tunnel_sock);
end:
	release_sock(sk);

	if (error != 0) {
		if (session)
			PRINTK(session->debug,
				PPPOL2TP_MSG_CONTROL, KERN_WARNING,
				"%s: connect failed: %d\n",
				session->name, error);
		else
			PRINTK(-1, PPPOL2TP_MSG_CONTROL, KERN_WARNING,
				"connect failed: %d\n", error);
	}

	return error;
}


static int pppol2tp_getname(struct socket *sock, struct sockaddr *uaddr,
			    int *usockaddr_len, int peer)
{
	int len = sizeof(struct sockaddr_pppol2tp);
	struct sockaddr_pppol2tp sp;
	int error = 0;
	struct pppol2tp_session *session;

	error = -ENOTCONN;
	if (sock->sk->sk_state != PPPOX_CONNECTED)
		goto end;

	session = pppol2tp_sock_to_session(sock->sk);
	if (session == NULL) {
		error = -EBADF;
		goto end;
	}

	sp.sa_family	= AF_PPPOX;
	sp.sa_protocol	= PX_PROTO_OL2TP;
	memcpy(&sp.pppol2tp, &session->tunnel_addr,
	       sizeof(struct pppol2tp_addr));

	memcpy(uaddr, &sp, len);

	*usockaddr_len = len;

	error = 0;
	sock_put(sock->sk);

end:
	return error;
}




static int pppol2tp_session_ioctl(struct pppol2tp_session *session,
				  unsigned int cmd, unsigned long arg)
{
	struct ifreq ifr;
	int err = 0;
	struct sock *sk = session->sock;
	int val = (int) arg;

	PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_DEBUG,
	       "%s: pppol2tp_session_ioctl(cmd=%#x, arg=%#lx)\n",
	       session->name, cmd, arg);

	sock_hold(sk);

	switch (cmd) {
	case SIOCGIFMTU:
		err = -ENXIO;
		if (!(sk->sk_state & PPPOX_CONNECTED))
			break;

		err = -EFAULT;
		if (copy_from_user(&ifr, (void __user *) arg, sizeof(struct ifreq)))
			break;
		ifr.ifr_mtu = session->mtu;
		if (copy_to_user((void __user *) arg, &ifr, sizeof(struct ifreq)))
			break;

		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: get mtu=%d\n", session->name, session->mtu);
		err = 0;
		break;

	case SIOCSIFMTU:
		err = -ENXIO;
		if (!(sk->sk_state & PPPOX_CONNECTED))
			break;

		err = -EFAULT;
		if (copy_from_user(&ifr, (void __user *) arg, sizeof(struct ifreq)))
			break;

		session->mtu = ifr.ifr_mtu;

		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: set mtu=%d\n", session->name, session->mtu);
		err = 0;
		break;

	case PPPIOCGMRU:
		err = -ENXIO;
		if (!(sk->sk_state & PPPOX_CONNECTED))
			break;

		err = -EFAULT;
		if (put_user(session->mru, (int __user *) arg))
			break;

		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: get mru=%d\n", session->name, session->mru);
		err = 0;
		break;

	case PPPIOCSMRU:
		err = -ENXIO;
		if (!(sk->sk_state & PPPOX_CONNECTED))
			break;

		err = -EFAULT;
		if (get_user(val,(int __user *) arg))
			break;

		session->mru = val;
		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: set mru=%d\n", session->name, session->mru);
		err = 0;
		break;

	case PPPIOCGFLAGS:
		err = -EFAULT;
		if (put_user(session->flags, (int __user *) arg))
			break;

		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: get flags=%d\n", session->name, session->flags);
		err = 0;
		break;

	case PPPIOCSFLAGS:
		err = -EFAULT;
		if (get_user(val, (int __user *) arg))
			break;
		session->flags = val;
		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: set flags=%d\n", session->name, session->flags);
		err = 0;
		break;

	case PPPIOCGL2TPSTATS:
		err = -ENXIO;
		if (!(sk->sk_state & PPPOX_CONNECTED))
			break;

		if (copy_to_user((void __user *) arg, &session->stats,
				 sizeof(session->stats)))
			break;
		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: get L2TP stats\n", session->name);
		err = 0;
		break;

	default:
		err = -ENOSYS;
		break;
	}

	sock_put(sk);

	return err;
}


static int pppol2tp_tunnel_ioctl(struct pppol2tp_tunnel *tunnel,
				 unsigned int cmd, unsigned long arg)
{
	int err = 0;
	struct sock *sk = tunnel->sock;
	struct pppol2tp_ioc_stats stats_req;

	PRINTK(tunnel->debug, PPPOL2TP_MSG_CONTROL, KERN_DEBUG,
	       "%s: pppol2tp_tunnel_ioctl(cmd=%#x, arg=%#lx)\n", tunnel->name,
	       cmd, arg);

	sock_hold(sk);

	switch (cmd) {
	case PPPIOCGL2TPSTATS:
		err = -ENXIO;
		if (!(sk->sk_state & PPPOX_CONNECTED))
			break;

		if (copy_from_user(&stats_req, (void __user *) arg,
				   sizeof(stats_req))) {
			err = -EFAULT;
			break;
		}
		if (stats_req.session_id != 0) {
			
			struct pppol2tp_session *session =
				pppol2tp_session_find(tunnel, stats_req.session_id);
			if (session != NULL)
				err = pppol2tp_session_ioctl(session, cmd, arg);
			else
				err = -EBADR;
			break;
		}
#ifdef CONFIG_XFRM
		tunnel->stats.using_ipsec = (sk->sk_policy[0] || sk->sk_policy[1]) ? 1 : 0;
#endif
		if (copy_to_user((void __user *) arg, &tunnel->stats,
				 sizeof(tunnel->stats))) {
			err = -EFAULT;
			break;
		}
		PRINTK(tunnel->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: get L2TP stats\n", tunnel->name);
		err = 0;
		break;

	default:
		err = -ENOSYS;
		break;
	}

	sock_put(sk);

	return err;
}


static int pppol2tp_ioctl(struct socket *sock, unsigned int cmd,
			  unsigned long arg)
{
	struct sock *sk = sock->sk;
	struct pppol2tp_session *session;
	struct pppol2tp_tunnel *tunnel;
	int err;

	if (!sk)
		return 0;

	err = -EBADF;
	if (sock_flag(sk, SOCK_DEAD) != 0)
		goto end;

	err = -ENOTCONN;
	if ((sk->sk_user_data == NULL) ||
	    (!(sk->sk_state & (PPPOX_CONNECTED | PPPOX_BOUND))))
		goto end;

	
	err = -EBADF;
	session = pppol2tp_sock_to_session(sk);
	if (session == NULL)
		goto end;

	
	if ((session->tunnel_addr.s_session == 0) &&
	    (session->tunnel_addr.d_session == 0)) {
		err = -EBADF;
		tunnel = pppol2tp_sock_to_tunnel(session->tunnel_sock);
		if (tunnel == NULL)
			goto end_put_sess;

		err = pppol2tp_tunnel_ioctl(tunnel, cmd, arg);
		sock_put(session->tunnel_sock);
		goto end_put_sess;
	}

	err = pppol2tp_session_ioctl(session, cmd, arg);

end_put_sess:
	sock_put(sk);
end:
	return err;
}




static int pppol2tp_tunnel_setsockopt(struct sock *sk,
				      struct pppol2tp_tunnel *tunnel,
				      int optname, int val)
{
	int err = 0;

	switch (optname) {
	case PPPOL2TP_SO_DEBUG:
		tunnel->debug = val;
		PRINTK(tunnel->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: set debug=%x\n", tunnel->name, tunnel->debug);
		break;

	default:
		err = -ENOPROTOOPT;
		break;
	}

	return err;
}


static int pppol2tp_session_setsockopt(struct sock *sk,
				       struct pppol2tp_session *session,
				       int optname, int val)
{
	int err = 0;

	switch (optname) {
	case PPPOL2TP_SO_RECVSEQ:
		if ((val != 0) && (val != 1)) {
			err = -EINVAL;
			break;
		}
		session->recv_seq = val ? -1 : 0;
		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: set recv_seq=%d\n", session->name,
		       session->recv_seq);
		break;

	case PPPOL2TP_SO_SENDSEQ:
		if ((val != 0) && (val != 1)) {
			err = -EINVAL;
			break;
		}
		session->send_seq = val ? -1 : 0;
		{
			struct sock *ssk      = session->sock;
			struct pppox_sock *po = pppox_sk(ssk);
			po->chan.hdrlen = val ? PPPOL2TP_L2TP_HDR_SIZE_SEQ :
				PPPOL2TP_L2TP_HDR_SIZE_NOSEQ;
		}
		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: set send_seq=%d\n", session->name, session->send_seq);
		break;

	case PPPOL2TP_SO_LNSMODE:
		if ((val != 0) && (val != 1)) {
			err = -EINVAL;
			break;
		}
		session->lns_mode = val ? -1 : 0;
		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: set lns_mode=%d\n", session->name,
		       session->lns_mode);
		break;

	case PPPOL2TP_SO_DEBUG:
		session->debug = val;
		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: set debug=%x\n", session->name, session->debug);
		break;

	case PPPOL2TP_SO_REORDERTO:
		session->reorder_timeout = msecs_to_jiffies(val);
		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: set reorder_timeout=%d\n", session->name,
		       session->reorder_timeout);
		break;

	default:
		err = -ENOPROTOOPT;
		break;
	}

	return err;
}


static int pppol2tp_setsockopt(struct socket *sock, int level, int optname,
			       char __user *optval, unsigned int optlen)
{
	struct sock *sk = sock->sk;
	struct pppol2tp_session *session = sk->sk_user_data;
	struct pppol2tp_tunnel *tunnel;
	int val;
	int err;

	if (level != SOL_PPPOL2TP)
		return udp_prot.setsockopt(sk, level, optname, optval, optlen);

	if (optlen < sizeof(int))
		return -EINVAL;

	if (get_user(val, (int __user *)optval))
		return -EFAULT;

	err = -ENOTCONN;
	if (sk->sk_user_data == NULL)
		goto end;

	
	err = -EBADF;
	session = pppol2tp_sock_to_session(sk);
	if (session == NULL)
		goto end;

	
	if ((session->tunnel_addr.s_session == 0) &&
	    (session->tunnel_addr.d_session == 0)) {
		err = -EBADF;
		tunnel = pppol2tp_sock_to_tunnel(session->tunnel_sock);
		if (tunnel == NULL)
			goto end_put_sess;

		err = pppol2tp_tunnel_setsockopt(sk, tunnel, optname, val);
		sock_put(session->tunnel_sock);
	} else
		err = pppol2tp_session_setsockopt(sk, session, optname, val);

	err = 0;

end_put_sess:
	sock_put(sk);
end:
	return err;
}


static int pppol2tp_tunnel_getsockopt(struct sock *sk,
				      struct pppol2tp_tunnel *tunnel,
				      int optname, int *val)
{
	int err = 0;

	switch (optname) {
	case PPPOL2TP_SO_DEBUG:
		*val = tunnel->debug;
		PRINTK(tunnel->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: get debug=%x\n", tunnel->name, tunnel->debug);
		break;

	default:
		err = -ENOPROTOOPT;
		break;
	}

	return err;
}


static int pppol2tp_session_getsockopt(struct sock *sk,
				       struct pppol2tp_session *session,
				       int optname, int *val)
{
	int err = 0;

	switch (optname) {
	case PPPOL2TP_SO_RECVSEQ:
		*val = session->recv_seq;
		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: get recv_seq=%d\n", session->name, *val);
		break;

	case PPPOL2TP_SO_SENDSEQ:
		*val = session->send_seq;
		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: get send_seq=%d\n", session->name, *val);
		break;

	case PPPOL2TP_SO_LNSMODE:
		*val = session->lns_mode;
		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: get lns_mode=%d\n", session->name, *val);
		break;

	case PPPOL2TP_SO_DEBUG:
		*val = session->debug;
		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: get debug=%d\n", session->name, *val);
		break;

	case PPPOL2TP_SO_REORDERTO:
		*val = (int) jiffies_to_msecs(session->reorder_timeout);
		PRINTK(session->debug, PPPOL2TP_MSG_CONTROL, KERN_INFO,
		       "%s: get reorder_timeout=%d\n", session->name, *val);
		break;

	default:
		err = -ENOPROTOOPT;
	}

	return err;
}


static int pppol2tp_getsockopt(struct socket *sock, int level,
			       int optname, char __user *optval, int __user *optlen)
{
	struct sock *sk = sock->sk;
	struct pppol2tp_session *session = sk->sk_user_data;
	struct pppol2tp_tunnel *tunnel;
	int val, len;
	int err;

	if (level != SOL_PPPOL2TP)
		return udp_prot.getsockopt(sk, level, optname, optval, optlen);

	if (get_user(len, (int __user *) optlen))
		return -EFAULT;

	len = min_t(unsigned int, len, sizeof(int));

	if (len < 0)
		return -EINVAL;

	err = -ENOTCONN;
	if (sk->sk_user_data == NULL)
		goto end;

	
	err = -EBADF;
	session = pppol2tp_sock_to_session(sk);
	if (session == NULL)
		goto end;

	
	if ((session->tunnel_addr.s_session == 0) &&
	    (session->tunnel_addr.d_session == 0)) {
		err = -EBADF;
		tunnel = pppol2tp_sock_to_tunnel(session->tunnel_sock);
		if (tunnel == NULL)
			goto end_put_sess;

		err = pppol2tp_tunnel_getsockopt(sk, tunnel, optname, &val);
		sock_put(session->tunnel_sock);
	} else
		err = pppol2tp_session_getsockopt(sk, session, optname, &val);

	err = -EFAULT;
	if (put_user(len, (int __user *) optlen))
		goto end_put_sess;

	if (copy_to_user((void __user *) optval, &val, len))
		goto end_put_sess;

	err = 0;

end_put_sess:
	sock_put(sk);
end:
	return err;
}



#ifdef CONFIG_PROC_FS

#include <linux/seq_file.h>

struct pppol2tp_seq_data {
	struct seq_net_private p;
	struct pppol2tp_tunnel *tunnel;		
	struct pppol2tp_session *session;	
};

static struct pppol2tp_session *next_session(struct pppol2tp_tunnel *tunnel, struct pppol2tp_session *curr)
{
	struct pppol2tp_session *session = NULL;
	struct hlist_node *walk;
	int found = 0;
	int next = 0;
	int i;

	read_lock_bh(&tunnel->hlist_lock);
	for (i = 0; i < PPPOL2TP_HASH_SIZE; i++) {
		hlist_for_each_entry(session, walk, &tunnel->session_hlist[i], hlist) {
			if (curr == NULL) {
				found = 1;
				goto out;
			}
			if (session == curr) {
				next = 1;
				continue;
			}
			if (next) {
				found = 1;
				goto out;
			}
		}
	}
out:
	read_unlock_bh(&tunnel->hlist_lock);
	if (!found)
		session = NULL;

	return session;
}

static struct pppol2tp_tunnel *next_tunnel(struct pppol2tp_net *pn,
					   struct pppol2tp_tunnel *curr)
{
	struct pppol2tp_tunnel *tunnel = NULL;

	read_lock_bh(&pn->pppol2tp_tunnel_list_lock);
	if (list_is_last(&curr->list, &pn->pppol2tp_tunnel_list)) {
		goto out;
	}
	tunnel = list_entry(curr->list.next, struct pppol2tp_tunnel, list);
out:
	read_unlock_bh(&pn->pppol2tp_tunnel_list_lock);

	return tunnel;
}

static void *pppol2tp_seq_start(struct seq_file *m, loff_t *offs)
{
	struct pppol2tp_seq_data *pd = SEQ_START_TOKEN;
	struct pppol2tp_net *pn;
	loff_t pos = *offs;

	if (!pos)
		goto out;

	BUG_ON(m->private == NULL);
	pd = m->private;
	pn = pppol2tp_pernet(seq_file_net(m));

	if (pd->tunnel == NULL) {
		if (!list_empty(&pn->pppol2tp_tunnel_list))
			pd->tunnel = list_entry(pn->pppol2tp_tunnel_list.next, struct pppol2tp_tunnel, list);
	} else {
		pd->session = next_session(pd->tunnel, pd->session);
		if (pd->session == NULL) {
			pd->tunnel = next_tunnel(pn, pd->tunnel);
		}
	}

	
	if ((pd->tunnel == NULL) && (pd->session == NULL))
		pd = NULL;

out:
	return pd;
}

static void *pppol2tp_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
	(*pos)++;
	return NULL;
}

static void pppol2tp_seq_stop(struct seq_file *p, void *v)
{
	
}

static void pppol2tp_seq_tunnel_show(struct seq_file *m, void *v)
{
	struct pppol2tp_tunnel *tunnel = v;

	seq_printf(m, "\nTUNNEL '%s', %c %d\n",
		   tunnel->name,
		   (tunnel == tunnel->sock->sk_user_data) ? 'Y':'N',
		   atomic_read(&tunnel->ref_count) - 1);
	seq_printf(m, " %08x %llu/%llu/%llu %llu/%llu/%llu\n",
		   tunnel->debug,
		   (unsigned long long)tunnel->stats.tx_packets,
		   (unsigned long long)tunnel->stats.tx_bytes,
		   (unsigned long long)tunnel->stats.tx_errors,
		   (unsigned long long)tunnel->stats.rx_packets,
		   (unsigned long long)tunnel->stats.rx_bytes,
		   (unsigned long long)tunnel->stats.rx_errors);
}

static void pppol2tp_seq_session_show(struct seq_file *m, void *v)
{
	struct pppol2tp_session *session = v;

	seq_printf(m, "  SESSION '%s' %08X/%d %04X/%04X -> "
		   "%04X/%04X %d %c\n",
		   session->name,
		   ntohl(session->tunnel_addr.addr.sin_addr.s_addr),
		   ntohs(session->tunnel_addr.addr.sin_port),
		   session->tunnel_addr.s_tunnel,
		   session->tunnel_addr.s_session,
		   session->tunnel_addr.d_tunnel,
		   session->tunnel_addr.d_session,
		   session->sock->sk_state,
		   (session == session->sock->sk_user_data) ?
		   'Y' : 'N');
	seq_printf(m, "   %d/%d/%c/%c/%s %08x %u\n",
		   session->mtu, session->mru,
		   session->recv_seq ? 'R' : '-',
		   session->send_seq ? 'S' : '-',
		   session->lns_mode ? "LNS" : "LAC",
		   session->debug,
		   jiffies_to_msecs(session->reorder_timeout));
	seq_printf(m, "   %hu/%hu %llu/%llu/%llu %llu/%llu/%llu\n",
		   session->nr, session->ns,
		   (unsigned long long)session->stats.tx_packets,
		   (unsigned long long)session->stats.tx_bytes,
		   (unsigned long long)session->stats.tx_errors,
		   (unsigned long long)session->stats.rx_packets,
		   (unsigned long long)session->stats.rx_bytes,
		   (unsigned long long)session->stats.rx_errors);
}

static int pppol2tp_seq_show(struct seq_file *m, void *v)
{
	struct pppol2tp_seq_data *pd = v;

	
	if (v == SEQ_START_TOKEN) {
		seq_puts(m, "PPPoL2TP driver info, " PPPOL2TP_DRV_VERSION "\n");
		seq_puts(m, "TUNNEL name, user-data-ok session-count\n");
		seq_puts(m, " debug tx-pkts/bytes/errs rx-pkts/bytes/errs\n");
		seq_puts(m, "  SESSION name, addr/port src-tid/sid "
			 "dest-tid/sid state user-data-ok\n");
		seq_puts(m, "   mtu/mru/rcvseq/sendseq/lns debug reorderto\n");
		seq_puts(m, "   nr/ns tx-pkts/bytes/errs rx-pkts/bytes/errs\n");
		goto out;
	}

	
	if (pd->session == NULL)
		pppol2tp_seq_tunnel_show(m, pd->tunnel);
	else
		pppol2tp_seq_session_show(m, pd->session);

out:
	return 0;
}

static const struct seq_operations pppol2tp_seq_ops = {
	.start		= pppol2tp_seq_start,
	.next		= pppol2tp_seq_next,
	.stop		= pppol2tp_seq_stop,
	.show		= pppol2tp_seq_show,
};


static int pppol2tp_proc_open(struct inode *inode, struct file *file)
{
	return seq_open_net(inode, file, &pppol2tp_seq_ops,
			    sizeof(struct pppol2tp_seq_data));
}

static const struct file_operations pppol2tp_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= pppol2tp_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release_net,
};

#endif 



static const struct proto_ops pppol2tp_ops = {
	.family		= AF_PPPOX,
	.owner		= THIS_MODULE,
	.release	= pppol2tp_release,
	.bind		= sock_no_bind,
	.connect	= pppol2tp_connect,
	.socketpair	= sock_no_socketpair,
	.accept		= sock_no_accept,
	.getname	= pppol2tp_getname,
	.poll		= datagram_poll,
	.listen		= sock_no_listen,
	.shutdown	= sock_no_shutdown,
	.setsockopt	= pppol2tp_setsockopt,
	.getsockopt	= pppol2tp_getsockopt,
	.sendmsg	= pppol2tp_sendmsg,
	.recvmsg	= pppol2tp_recvmsg,
	.mmap		= sock_no_mmap,
	.ioctl		= pppox_ioctl,
};

static struct pppox_proto pppol2tp_proto = {
	.create		= pppol2tp_create,
	.ioctl		= pppol2tp_ioctl
};

static __net_init int pppol2tp_init_net(struct net *net)
{
	struct pppol2tp_net *pn;
	struct proc_dir_entry *pde;
	int err;

	pn = kzalloc(sizeof(*pn), GFP_KERNEL);
	if (!pn)
		return -ENOMEM;

	INIT_LIST_HEAD(&pn->pppol2tp_tunnel_list);
	rwlock_init(&pn->pppol2tp_tunnel_list_lock);

	err = net_assign_generic(net, pppol2tp_net_id, pn);
	if (err)
		goto out;

	pde = proc_net_fops_create(net, "pppol2tp", S_IRUGO, &pppol2tp_proc_fops);
#ifdef CONFIG_PROC_FS
	if (!pde) {
		err = -ENOMEM;
		goto out;
	}
#endif

	return 0;

out:
	kfree(pn);
	return err;
}

static __net_exit void pppol2tp_exit_net(struct net *net)
{
	struct pppoe_net *pn;

	proc_net_remove(net, "pppol2tp");
	pn = net_generic(net, pppol2tp_net_id);
	
	net_assign_generic(net, pppol2tp_net_id, NULL);
	kfree(pn);
}

static struct pernet_operations pppol2tp_net_ops = {
	.init = pppol2tp_init_net,
	.exit = pppol2tp_exit_net,
};

static int __init pppol2tp_init(void)
{
	int err;

	err = proto_register(&pppol2tp_sk_proto, 0);
	if (err)
		goto out;
	err = register_pppox_proto(PX_PROTO_OL2TP, &pppol2tp_proto);
	if (err)
		goto out_unregister_pppol2tp_proto;

	err = register_pernet_gen_device(&pppol2tp_net_id, &pppol2tp_net_ops);
	if (err)
		goto out_unregister_pppox_proto;

	printk(KERN_INFO "PPPoL2TP kernel driver, %s\n",
	       PPPOL2TP_DRV_VERSION);

out:
	return err;
out_unregister_pppox_proto:
	unregister_pppox_proto(PX_PROTO_OL2TP);
out_unregister_pppol2tp_proto:
	proto_unregister(&pppol2tp_sk_proto);
	goto out;
}

static void __exit pppol2tp_exit(void)
{
	unregister_pppox_proto(PX_PROTO_OL2TP);
	unregister_pernet_gen_device(pppol2tp_net_id, &pppol2tp_net_ops);
	proto_unregister(&pppol2tp_sk_proto);
}

module_init(pppol2tp_init);
module_exit(pppol2tp_exit);

MODULE_AUTHOR("Martijn van Oosterhout <kleptog@svana.org>, "
	      "James Chapman <jchapman@katalix.com>");
MODULE_DESCRIPTION("PPP over L2TP over UDP");
MODULE_LICENSE("GPL");
MODULE_VERSION(PPPOL2TP_DRV_VERSION);
