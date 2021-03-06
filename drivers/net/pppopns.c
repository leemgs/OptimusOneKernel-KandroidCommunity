



#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/skbuff.h>
#include <linux/file.h>
#include <linux/netdevice.h>
#include <linux/net.h>
#include <linux/ppp_defs.h>
#include <linux/if.h>
#include <linux/if_ppp.h>
#include <linux/if_pppox.h>
#include <linux/ppp_channel.h>
#include <asm/uaccess.h>

#define GRE_HEADER_SIZE		8

#define PPTP_GRE_BITS		htons(0x2001)
#define PPTP_GRE_BITS_MASK	htons(0xEF7F)
#define PPTP_GRE_SEQ_BIT	htons(0x1000)
#define PPTP_GRE_ACK_BIT	htons(0x0080)
#define PPTP_GRE_TYPE		htons(0x880B)

#define PPP_ADDR	0xFF
#define PPP_CTRL	0x03

struct header {
	__u16	bits;
	__u16	type;
	__u16	length;
	__u16	call;
	__u32	sequence;
} __attribute__((packed));

static int pppopns_recv_core(struct sock *sk_raw, struct sk_buff *skb)
{
	struct sock *sk = (struct sock *)sk_raw->sk_user_data;
	struct pppopns_opt *opt = &pppox_sk(sk)->proto.pns;
	struct header *hdr;

	
	skb_pull(skb, skb_transport_header(skb) - skb->data);

	
	if (skb->len < GRE_HEADER_SIZE)
		goto drop;

	
	hdr = (struct header *)skb->data;
	if (hdr->type != PPTP_GRE_TYPE || hdr->call != opt->local ||
			(hdr->bits & PPTP_GRE_BITS_MASK) != PPTP_GRE_BITS)
		goto drop;

	
	if (!skb_pull(skb, GRE_HEADER_SIZE +
			(hdr->bits & PPTP_GRE_SEQ_BIT ? 4 : 0) +
			(hdr->bits & PPTP_GRE_ACK_BIT ? 4 : 0)))
		goto drop;

	
	if (skb->len != ntohs(hdr->length))
		goto drop;

	
	if (skb->len >= 2 && skb->data[0] == PPP_ADDR &&
			skb->data[1] == PPP_CTRL)
		skb_pull(skb, 2);

	
	if (skb->len >= 1 && skb->data[0] & 1)
		skb_push(skb, 1)[0] = 0;

	
	skb_orphan(skb);
	ppp_input(&pppox_sk(sk)->chan, skb);
	return NET_RX_SUCCESS;
drop:
	kfree_skb(skb);
	return NET_RX_DROP;
}

static void pppopns_recv(struct sock *sk_raw, int length)
{
	struct sk_buff *skb;
	while ((skb = skb_dequeue(&sk_raw->sk_receive_queue))) {
		sock_hold(sk_raw);
		sk_receive_skb(sk_raw, skb, 0);
	}
}

static struct sk_buff_head delivery_queue;

static void pppopns_xmit_core(struct work_struct *delivery_work)
{
	mm_segment_t old_fs = get_fs();
	struct sk_buff *skb;

	set_fs(KERNEL_DS);
	while ((skb = skb_dequeue(&delivery_queue))) {
		struct sock *sk_raw = skb->sk;
		struct kvec iov = {.iov_base = skb->data, .iov_len = skb->len};
		struct msghdr msg = {
			.msg_iov = (struct iovec *)&iov,
			.msg_iovlen = 1,
			.msg_flags = MSG_NOSIGNAL | MSG_DONTWAIT,
		};
		sk_raw->sk_prot->sendmsg(NULL, sk_raw, &msg, skb->len);
		kfree_skb(skb);
	}
	set_fs(old_fs);
}

static DECLARE_WORK(delivery_work, pppopns_xmit_core);

static int pppopns_xmit(struct ppp_channel *chan, struct sk_buff *skb)
{
	struct sock *sk_raw = (struct sock *)chan->private;
	struct pppopns_opt *opt = &pppox_sk(sk_raw->sk_user_data)->proto.pns;
	struct header *hdr;
	__u16 length;

	
	skb_push(skb, 2);
	skb->data[0] = PPP_ADDR;
	skb->data[1] = PPP_CTRL;
	length = skb->len;

	
	hdr = (struct header *)skb_push(skb, 12);
	hdr->bits = PPTP_GRE_BITS | PPTP_GRE_SEQ_BIT;
	hdr->type = PPTP_GRE_TYPE;
	hdr->length = htons(length);
	hdr->call = opt->remote;
	hdr->sequence = htonl(opt->sequence);
	opt->sequence++;

	
	skb_set_owner_w(skb, sk_raw);
	skb_queue_tail(&delivery_queue, skb);
	schedule_work(&delivery_work);
	return 1;
}



static struct ppp_channel_ops pppopns_channel_ops = {
	.start_xmit = pppopns_xmit,
};

static int pppopns_connect(struct socket *sock, struct sockaddr *useraddr,
	int addrlen, int flags)
{
	struct sock *sk = sock->sk;
	struct pppox_sock *po = pppox_sk(sk);
	struct sockaddr_pppopns *addr = (struct sockaddr_pppopns *)useraddr;
	struct sockaddr_storage ss;
	struct socket *sock_tcp = NULL;
	struct socket *sock_raw = NULL;
	struct sock *sk_tcp;
	struct sock *sk_raw;
	int error;

	if (addrlen != sizeof(struct sockaddr_pppopns))
		return -EINVAL;

	lock_sock(sk);
	error = -EALREADY;
	if (sk->sk_state != PPPOX_NONE)
		goto out;

	sock_tcp = sockfd_lookup(addr->tcp_socket, &error);
	if (!sock_tcp)
		goto out;
	sk_tcp = sock_tcp->sk;
	error = -EPROTONOSUPPORT;
	if (sk_tcp->sk_protocol != IPPROTO_TCP)
		goto out;
	addrlen = sizeof(struct sockaddr_storage);
	error = kernel_getpeername(sock_tcp, (struct sockaddr *)&ss, &addrlen);
	if (error)
		goto out;
	if (!sk_tcp->sk_bound_dev_if) {
		struct dst_entry *dst = sk_dst_get(sk_tcp);
		error = -ENODEV;
		if (!dst)
			goto out;
		sk_tcp->sk_bound_dev_if = dst->dev->ifindex;
		dst_release(dst);
	}

	error = sock_create(ss.ss_family, SOCK_RAW, IPPROTO_GRE, &sock_raw);
	if (error)
		goto out;
	sk_raw = sock_raw->sk;
	sk_raw->sk_bound_dev_if = sk_tcp->sk_bound_dev_if;
	error = kernel_connect(sock_raw, (struct sockaddr *)&ss, addrlen, 0);
	if (error)
		goto out;

	po->chan.hdrlen = 14;
	po->chan.private = sk_raw;
	po->chan.ops = &pppopns_channel_ops;
	po->chan.mtu = PPP_MTU - 80;
	po->proto.pns.local = addr->local;
	po->proto.pns.remote = addr->remote;
	po->proto.pns.data_ready = sk_raw->sk_data_ready;
	po->proto.pns.backlog_rcv = sk_raw->sk_backlog_rcv;

	error = ppp_register_channel(&po->chan);
	if (error)
		goto out;

	sk->sk_state = PPPOX_CONNECTED;
	lock_sock(sk_raw);
	sk_raw->sk_data_ready = pppopns_recv;
	sk_raw->sk_backlog_rcv = pppopns_recv_core;
	sk_raw->sk_user_data = sk;
	release_sock(sk_raw);
out:
	if (sock_tcp)
		sockfd_put(sock_tcp);
	if (error && sock_raw)
		sock_release(sock_raw);
	release_sock(sk);
	return error;
}

static int pppopns_release(struct socket *sock)
{
	struct sock *sk = sock->sk;

	if (!sk)
		return 0;

	lock_sock(sk);
	if (sock_flag(sk, SOCK_DEAD)) {
		release_sock(sk);
		return -EBADF;
	}

	if (sk->sk_state != PPPOX_NONE) {
		struct sock *sk_raw = (struct sock *)pppox_sk(sk)->chan.private;
		lock_sock(sk_raw);
		pppox_unbind_sock(sk);
		sk_raw->sk_data_ready = pppox_sk(sk)->proto.pns.data_ready;
		sk_raw->sk_backlog_rcv = pppox_sk(sk)->proto.pns.backlog_rcv;
		sk_raw->sk_user_data = NULL;
		release_sock(sk_raw);
		sock_release(sk_raw->sk_socket);
	}

	sock_orphan(sk);
	sock->sk = NULL;
	release_sock(sk);
	sock_put(sk);
	return 0;
}



static struct proto pppopns_proto = {
	.name = "PPPOPNS",
	.owner = THIS_MODULE,
	.obj_size = sizeof(struct pppox_sock),
};

static struct proto_ops pppopns_proto_ops = {
	.family = PF_PPPOX,
	.owner = THIS_MODULE,
	.release = pppopns_release,
	.bind = sock_no_bind,
	.connect = pppopns_connect,
	.socketpair = sock_no_socketpair,
	.accept = sock_no_accept,
	.getname = sock_no_getname,
	.poll = sock_no_poll,
	.ioctl = pppox_ioctl,
	.listen = sock_no_listen,
	.shutdown = sock_no_shutdown,
	.setsockopt = sock_no_setsockopt,
	.getsockopt = sock_no_getsockopt,
	.sendmsg = sock_no_sendmsg,
	.recvmsg = sock_no_recvmsg,
	.mmap = sock_no_mmap,
};

static int pppopns_create(struct net *net, struct socket *sock)
{
	struct sock *sk;

	sk = sk_alloc(net, PF_PPPOX, GFP_KERNEL, &pppopns_proto);
	if (!sk)
		return -ENOMEM;

	sock_init_data(sock, sk);
	sock->state = SS_UNCONNECTED;
	sock->ops = &pppopns_proto_ops;
	sk->sk_protocol = PX_PROTO_OPNS;
	sk->sk_state = PPPOX_NONE;
	return 0;
}



static struct pppox_proto pppopns_pppox_proto = {
	.create = pppopns_create,
	.owner = THIS_MODULE,
};

static int __init pppopns_init(void)
{
	int error;

	error = proto_register(&pppopns_proto, 0);
	if (error)
		return error;

	error = register_pppox_proto(PX_PROTO_OPNS, &pppopns_pppox_proto);
	if (error)
		proto_unregister(&pppopns_proto);
	else
		skb_queue_head_init(&delivery_queue);
	return error;
}

static void __exit pppopns_exit(void)
{
	unregister_pppox_proto(PX_PROTO_OPNS);
	proto_unregister(&pppopns_proto);
}

module_init(pppopns_init);
module_exit(pppopns_exit);

MODULE_DESCRIPTION("PPP on PPTP Network Server (PPPoPNS)");
MODULE_AUTHOR("Chia-chi Yeh <chiachi@android.com>");
MODULE_LICENSE("GPL");
