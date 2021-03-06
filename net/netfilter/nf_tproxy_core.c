

#include <linux/module.h>

#include <linux/net.h>
#include <linux/if.h>
#include <linux/netdevice.h>
#include <net/udp.h>
#include <net/netfilter/nf_tproxy_core.h>

struct sock *
nf_tproxy_get_sock_v4(struct net *net, const u8 protocol,
		      const __be32 saddr, const __be32 daddr,
		      const __be16 sport, const __be16 dport,
		      const struct net_device *in, bool listening_only)
{
	struct sock *sk;

	
	switch (protocol) {
	case IPPROTO_TCP:
		if (listening_only)
			sk = __inet_lookup_listener(net, &tcp_hashinfo,
						    daddr, ntohs(dport),
						    in->ifindex);
		else
			sk = __inet_lookup(net, &tcp_hashinfo,
					   saddr, sport, daddr, dport,
					   in->ifindex);
		break;
	case IPPROTO_UDP:
		sk = udp4_lib_lookup(net, saddr, sport, daddr, dport,
				     in->ifindex);
		break;
	default:
		WARN_ON(1);
		sk = NULL;
	}

	pr_debug("tproxy socket lookup: proto %u %08x:%u -> %08x:%u, listener only: %d, sock %p\n",
		 protocol, ntohl(saddr), ntohs(sport), ntohl(daddr), ntohs(dport), listening_only, sk);

	return sk;
}
EXPORT_SYMBOL_GPL(nf_tproxy_get_sock_v4);

static void
nf_tproxy_destructor(struct sk_buff *skb)
{
	struct sock *sk = skb->sk;

	skb->sk = NULL;
	skb->destructor = NULL;

	if (sk)
		nf_tproxy_put_sock(sk);
}


int
nf_tproxy_assign_sock(struct sk_buff *skb, struct sock *sk)
{
	if (inet_sk(sk)->transparent) {
		skb_orphan(skb);
		skb->sk = sk;
		skb->destructor = nf_tproxy_destructor;
		return 1;
	} else
		nf_tproxy_put_sock(sk);

	return 0;
}
EXPORT_SYMBOL_GPL(nf_tproxy_assign_sock);

static int __init nf_tproxy_init(void)
{
	pr_info("NF_TPROXY: Transparent proxy support initialized, version 4.1.0\n");
	pr_info("NF_TPROXY: Copyright (c) 2006-2007 BalaBit IT Ltd.\n");
	return 0;
}

module_init(nf_tproxy_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Krisztian Kovacs");
MODULE_DESCRIPTION("Transparent proxy support core routines");
