

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/netdevice.h>
#include <linux/init.h>
#include <linux/ipsec.h>

#include <linux/ipv6.h>
#include <linux/icmpv6.h>
#include <linux/random.h>
#include <linux/seq_file.h>

#include <net/protocol.h>
#include <net/ndisc.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/transp_v6.h>
#include <net/addrconf.h>
#include <net/ip6_route.h>
#include <net/inet_common.h>
#include <net/inet_ecn.h>
#include <net/sctp/sctp.h>

#include <asm/uaccess.h>


static int sctp_inet6addr_event(struct notifier_block *this, unsigned long ev,
				void *ptr)
{
	struct inet6_ifaddr *ifa = (struct inet6_ifaddr *)ptr;
	struct sctp_sockaddr_entry *addr = NULL;
	struct sctp_sockaddr_entry *temp;
	int found = 0;

	switch (ev) {
	case NETDEV_UP:
		addr = kmalloc(sizeof(struct sctp_sockaddr_entry), GFP_ATOMIC);
		if (addr) {
			addr->a.v6.sin6_family = AF_INET6;
			addr->a.v6.sin6_port = 0;
			ipv6_addr_copy(&addr->a.v6.sin6_addr, &ifa->addr);
			addr->a.v6.sin6_scope_id = ifa->idev->dev->ifindex;
			addr->valid = 1;
			spin_lock_bh(&sctp_local_addr_lock);
			list_add_tail_rcu(&addr->list, &sctp_local_addr_list);
			spin_unlock_bh(&sctp_local_addr_lock);
		}
		break;
	case NETDEV_DOWN:
		spin_lock_bh(&sctp_local_addr_lock);
		list_for_each_entry_safe(addr, temp,
					&sctp_local_addr_list, list) {
			if (addr->a.sa.sa_family == AF_INET6 &&
					ipv6_addr_equal(&addr->a.v6.sin6_addr,
						&ifa->addr)) {
				found = 1;
				addr->valid = 0;
				list_del_rcu(&addr->list);
				break;
			}
		}
		spin_unlock_bh(&sctp_local_addr_lock);
		if (found)
			call_rcu(&addr->rcu, sctp_local_addr_free);
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block sctp_inet6addr_notifier = {
	.notifier_call = sctp_inet6addr_event,
};


SCTP_STATIC void sctp_v6_err(struct sk_buff *skb, struct inet6_skb_parm *opt,
			     u8 type, u8 code, int offset, __be32 info)
{
	struct inet6_dev *idev;
	struct sock *sk;
	struct sctp_association *asoc;
	struct sctp_transport *transport;
	struct ipv6_pinfo *np;
	sk_buff_data_t saveip, savesctp;
	int err;

	idev = in6_dev_get(skb->dev);

	
	saveip	 = skb->network_header;
	savesctp = skb->transport_header;
	skb_reset_network_header(skb);
	skb_set_transport_header(skb, offset);
	sk = sctp_err_lookup(AF_INET6, skb, sctp_hdr(skb), &asoc, &transport);
	
	skb->network_header   = saveip;
	skb->transport_header = savesctp;
	if (!sk) {
		ICMP6_INC_STATS_BH(dev_net(skb->dev), idev, ICMP6_MIB_INERRORS);
		goto out;
	}

	

	switch (type) {
	case ICMPV6_PKT_TOOBIG:
		sctp_icmp_frag_needed(sk, asoc, transport, ntohl(info));
		goto out_unlock;
	case ICMPV6_PARAMPROB:
		if (ICMPV6_UNK_NEXTHDR == code) {
			sctp_icmp_proto_unreachable(sk, asoc, transport);
			goto out_unlock;
		}
		break;
	default:
		break;
	}

	np = inet6_sk(sk);
	icmpv6_err_convert(type, code, &err);
	if (!sock_owned_by_user(sk) && np->recverr) {
		sk->sk_err = err;
		sk->sk_error_report(sk);
	} else {  
		sk->sk_err_soft = err;
	}

out_unlock:
	sctp_err_finish(sk, asoc);
out:
	if (likely(idev != NULL))
		in6_dev_put(idev);
}


static int sctp_v6_xmit(struct sk_buff *skb, struct sctp_transport *transport)
{
	struct sock *sk = skb->sk;
	struct ipv6_pinfo *np = inet6_sk(sk);
	struct flowi fl;

	memset(&fl, 0, sizeof(fl));

	fl.proto = sk->sk_protocol;

	
	ipv6_addr_copy(&fl.fl6_dst, &transport->ipaddr.v6.sin6_addr);
	ipv6_addr_copy(&fl.fl6_src, &transport->saddr.v6.sin6_addr);

	fl.fl6_flowlabel = np->flow_label;
	IP6_ECN_flow_xmit(sk, fl.fl6_flowlabel);
	if (ipv6_addr_type(&fl.fl6_src) & IPV6_ADDR_LINKLOCAL)
		fl.oif = transport->saddr.v6.sin6_scope_id;
	else
		fl.oif = sk->sk_bound_dev_if;

	if (np->opt && np->opt->srcrt) {
		struct rt0_hdr *rt0 = (struct rt0_hdr *) np->opt->srcrt;
		ipv6_addr_copy(&fl.fl6_dst, rt0->addr);
	}

	SCTP_DEBUG_PRINTK("%s: skb:%p, len:%d, src:%pI6 dst:%pI6\n",
			  __func__, skb, skb->len,
			  &fl.fl6_src, &fl.fl6_dst);

	SCTP_INC_STATS(SCTP_MIB_OUTSCTPPACKS);

	if (!(transport->param_flags & SPP_PMTUD_ENABLE))
		skb->local_df = 1;

	return ip6_xmit(sk, skb, &fl, np->opt, 0);
}


static struct dst_entry *sctp_v6_get_dst(struct sctp_association *asoc,
					 union sctp_addr *daddr,
					 union sctp_addr *saddr)
{
	struct dst_entry *dst;
	struct flowi fl;

	memset(&fl, 0, sizeof(fl));
	ipv6_addr_copy(&fl.fl6_dst, &daddr->v6.sin6_addr);
	if (ipv6_addr_type(&daddr->v6.sin6_addr) & IPV6_ADDR_LINKLOCAL)
		fl.oif = daddr->v6.sin6_scope_id;


	SCTP_DEBUG_PRINTK("%s: DST=%pI6 ", __func__, &fl.fl6_dst);

	if (saddr) {
		ipv6_addr_copy(&fl.fl6_src, &saddr->v6.sin6_addr);
		SCTP_DEBUG_PRINTK("SRC=%pI6 - ", &fl.fl6_src);
	}

	dst = ip6_route_output(&init_net, NULL, &fl);
	if (!dst->error) {
		struct rt6_info *rt;
		rt = (struct rt6_info *)dst;
		SCTP_DEBUG_PRINTK("rt6_dst:%pI6 rt6_src:%pI6\n",
			&rt->rt6i_dst.addr, &rt->rt6i_src.addr);
		return dst;
	}
	SCTP_DEBUG_PRINTK("NO ROUTE\n");
	dst_release(dst);
	return NULL;
}


static inline int sctp_v6_addr_match_len(union sctp_addr *s1,
					 union sctp_addr *s2)
{
	struct in6_addr *a1 = &s1->v6.sin6_addr;
	struct in6_addr *a2 = &s2->v6.sin6_addr;
	int i, j;

	for (i = 0; i < 4 ; i++) {
		__be32 a1xora2;

		a1xora2 = a1->s6_addr32[i] ^ a2->s6_addr32[i];

		if ((j = fls(ntohl(a1xora2))))
			return (i * 32 + 32 - j);
	}

	return (i*32);
}


static void sctp_v6_get_saddr(struct sctp_sock *sk,
			      struct sctp_association *asoc,
			      struct dst_entry *dst,
			      union sctp_addr *daddr,
			      union sctp_addr *saddr)
{
	struct sctp_bind_addr *bp;
	struct sctp_sockaddr_entry *laddr;
	sctp_scope_t scope;
	union sctp_addr *baddr = NULL;
	__u8 matchlen = 0;
	__u8 bmatchlen;

	SCTP_DEBUG_PRINTK("%s: asoc:%p dst:%p daddr:%pI6 ",
			  __func__, asoc, dst, &daddr->v6.sin6_addr);

	if (!asoc) {
		ipv6_dev_get_saddr(sock_net(sctp_opt2sk(sk)),
				   dst ? ip6_dst_idev(dst)->dev : NULL,
				   &daddr->v6.sin6_addr,
				   inet6_sk(&sk->inet.sk)->srcprefs,
				   &saddr->v6.sin6_addr);
		SCTP_DEBUG_PRINTK("saddr from ipv6_get_saddr: %pI6\n",
				  &saddr->v6.sin6_addr);
		return;
	}

	scope = sctp_scope(daddr);

	bp = &asoc->base.bind_addr;

	
	rcu_read_lock();
	list_for_each_entry_rcu(laddr, &bp->address_list, list) {
		if (!laddr->valid)
			continue;
		if ((laddr->state == SCTP_ADDR_SRC) &&
		    (laddr->a.sa.sa_family == AF_INET6) &&
		    (scope <= sctp_scope(&laddr->a))) {
			bmatchlen = sctp_v6_addr_match_len(daddr, &laddr->a);
			if (!baddr || (matchlen < bmatchlen)) {
				baddr = &laddr->a;
				matchlen = bmatchlen;
			}
		}
	}

	if (baddr) {
		memcpy(saddr, baddr, sizeof(union sctp_addr));
		SCTP_DEBUG_PRINTK("saddr: %pI6\n", &saddr->v6.sin6_addr);
	} else {
		printk(KERN_ERR "%s: asoc:%p Could not find a valid source "
		       "address for the dest:%pI6\n",
		       __func__, asoc, &daddr->v6.sin6_addr);
	}

	rcu_read_unlock();
}


static void sctp_v6_copy_addrlist(struct list_head *addrlist,
				  struct net_device *dev)
{
	struct inet6_dev *in6_dev;
	struct inet6_ifaddr *ifp;
	struct sctp_sockaddr_entry *addr;

	rcu_read_lock();
	if ((in6_dev = __in6_dev_get(dev)) == NULL) {
		rcu_read_unlock();
		return;
	}

	read_lock_bh(&in6_dev->lock);
	for (ifp = in6_dev->addr_list; ifp; ifp = ifp->if_next) {
		
		addr = t_new(struct sctp_sockaddr_entry, GFP_ATOMIC);
		if (addr) {
			addr->a.v6.sin6_family = AF_INET6;
			addr->a.v6.sin6_port = 0;
			addr->a.v6.sin6_addr = ifp->addr;
			addr->a.v6.sin6_scope_id = dev->ifindex;
			addr->valid = 1;
			INIT_LIST_HEAD(&addr->list);
			INIT_RCU_HEAD(&addr->rcu);
			list_add_tail(&addr->list, addrlist);
		}
	}

	read_unlock_bh(&in6_dev->lock);
	rcu_read_unlock();
}


static void sctp_v6_from_skb(union sctp_addr *addr,struct sk_buff *skb,
			     int is_saddr)
{
	void *from;
	__be16 *port;
	struct sctphdr *sh;

	port = &addr->v6.sin6_port;
	addr->v6.sin6_family = AF_INET6;
	addr->v6.sin6_flowinfo = 0; 
	addr->v6.sin6_scope_id = ((struct inet6_skb_parm *)skb->cb)->iif;

	sh = sctp_hdr(skb);
	if (is_saddr) {
		*port  = sh->source;
		from = &ipv6_hdr(skb)->saddr;
	} else {
		*port = sh->dest;
		from = &ipv6_hdr(skb)->daddr;
	}
	ipv6_addr_copy(&addr->v6.sin6_addr, from);
}


static void sctp_v6_from_sk(union sctp_addr *addr, struct sock *sk)
{
	addr->v6.sin6_family = AF_INET6;
	addr->v6.sin6_port = 0;
	addr->v6.sin6_addr = inet6_sk(sk)->rcv_saddr;
}


static void sctp_v6_to_sk_saddr(union sctp_addr *addr, struct sock *sk)
{
	if (addr->sa.sa_family == AF_INET && sctp_sk(sk)->v4mapped) {
		inet6_sk(sk)->rcv_saddr.s6_addr32[0] = 0;
		inet6_sk(sk)->rcv_saddr.s6_addr32[1] = 0;
		inet6_sk(sk)->rcv_saddr.s6_addr32[2] = htonl(0x0000ffff);
		inet6_sk(sk)->rcv_saddr.s6_addr32[3] =
			addr->v4.sin_addr.s_addr;
	} else {
		inet6_sk(sk)->rcv_saddr = addr->v6.sin6_addr;
	}
}


static void sctp_v6_to_sk_daddr(union sctp_addr *addr, struct sock *sk)
{
	if (addr->sa.sa_family == AF_INET && sctp_sk(sk)->v4mapped) {
		inet6_sk(sk)->daddr.s6_addr32[0] = 0;
		inet6_sk(sk)->daddr.s6_addr32[1] = 0;
		inet6_sk(sk)->daddr.s6_addr32[2] = htonl(0x0000ffff);
		inet6_sk(sk)->daddr.s6_addr32[3] = addr->v4.sin_addr.s_addr;
	} else {
		inet6_sk(sk)->daddr = addr->v6.sin6_addr;
	}
}


static void sctp_v6_from_addr_param(union sctp_addr *addr,
				    union sctp_addr_param *param,
				    __be16 port, int iif)
{
	addr->v6.sin6_family = AF_INET6;
	addr->v6.sin6_port = port;
	addr->v6.sin6_flowinfo = 0; 
	ipv6_addr_copy(&addr->v6.sin6_addr, &param->v6.addr);
	addr->v6.sin6_scope_id = iif;
}


static int sctp_v6_to_addr_param(const union sctp_addr *addr,
				 union sctp_addr_param *param)
{
	int length = sizeof(sctp_ipv6addr_param_t);

	param->v6.param_hdr.type = SCTP_PARAM_IPV6_ADDRESS;
	param->v6.param_hdr.length = htons(length);
	ipv6_addr_copy(&param->v6.addr, &addr->v6.sin6_addr);

	return length;
}


static void sctp_v6_dst_saddr(union sctp_addr *addr, struct dst_entry *dst,
			      __be16 port)
{
	struct rt6_info *rt = (struct rt6_info *)dst;
	addr->sa.sa_family = AF_INET6;
	addr->v6.sin6_port = port;
	ipv6_addr_copy(&addr->v6.sin6_addr, &rt->rt6i_src.addr);
}


static int sctp_v6_cmp_addr(const union sctp_addr *addr1,
			    const union sctp_addr *addr2)
{
	if (addr1->sa.sa_family != addr2->sa.sa_family) {
		if (addr1->sa.sa_family == AF_INET &&
		    addr2->sa.sa_family == AF_INET6 &&
		    ipv6_addr_v4mapped(&addr2->v6.sin6_addr)) {
			if (addr2->v6.sin6_port == addr1->v4.sin_port &&
			    addr2->v6.sin6_addr.s6_addr32[3] ==
			    addr1->v4.sin_addr.s_addr)
				return 1;
		}
		if (addr2->sa.sa_family == AF_INET &&
		    addr1->sa.sa_family == AF_INET6 &&
		    ipv6_addr_v4mapped(&addr1->v6.sin6_addr)) {
			if (addr1->v6.sin6_port == addr2->v4.sin_port &&
			    addr1->v6.sin6_addr.s6_addr32[3] ==
			    addr2->v4.sin_addr.s_addr)
				return 1;
		}
		return 0;
	}
	if (!ipv6_addr_equal(&addr1->v6.sin6_addr, &addr2->v6.sin6_addr))
		return 0;
	
	if (ipv6_addr_type(&addr1->v6.sin6_addr) & IPV6_ADDR_LINKLOCAL) {
		if (addr1->v6.sin6_scope_id && addr2->v6.sin6_scope_id &&
		    (addr1->v6.sin6_scope_id != addr2->v6.sin6_scope_id)) {
			return 0;
		}
	}

	return 1;
}


static void sctp_v6_inaddr_any(union sctp_addr *addr, __be16 port)
{
	memset(addr, 0x00, sizeof(union sctp_addr));
	addr->v6.sin6_family = AF_INET6;
	addr->v6.sin6_port = port;
}


static int sctp_v6_is_any(const union sctp_addr *addr)
{
	return ipv6_addr_any(&addr->v6.sin6_addr);
}


static int sctp_v6_available(union sctp_addr *addr, struct sctp_sock *sp)
{
	int type;
	struct in6_addr *in6 = (struct in6_addr *)&addr->v6.sin6_addr;

	type = ipv6_addr_type(in6);
	if (IPV6_ADDR_ANY == type)
		return 1;
	if (type == IPV6_ADDR_MAPPED) {
		if (sp && !sp->v4mapped)
			return 0;
		if (sp && ipv6_only_sock(sctp_opt2sk(sp)))
			return 0;
		sctp_v6_map_v4(addr);
		return sctp_get_af_specific(AF_INET)->available(addr, sp);
	}
	if (!(type & IPV6_ADDR_UNICAST))
		return 0;

	return ipv6_chk_addr(&init_net, in6, NULL, 0);
}


static int sctp_v6_addr_valid(union sctp_addr *addr,
			      struct sctp_sock *sp,
			      const struct sk_buff *skb)
{
	int ret = ipv6_addr_type(&addr->v6.sin6_addr);

	
	if (ret == IPV6_ADDR_MAPPED) {
		
		if (!sp || !sp->v4mapped)
			return 0;
		if (sp && ipv6_only_sock(sctp_opt2sk(sp)))
			return 0;
		sctp_v6_map_v4(addr);
		return sctp_get_af_specific(AF_INET)->addr_valid(addr, sp, skb);
	}

	
	if (!(ret & IPV6_ADDR_UNICAST))
		return 0;

	return 1;
}


static sctp_scope_t sctp_v6_scope(union sctp_addr *addr)
{
	int v6scope;
	sctp_scope_t retval;

	

	v6scope = ipv6_addr_scope(&addr->v6.sin6_addr);
	switch (v6scope) {
	case IFA_HOST:
		retval = SCTP_SCOPE_LOOPBACK;
		break;
	case IFA_LINK:
		retval = SCTP_SCOPE_LINK;
		break;
	case IFA_SITE:
		retval = SCTP_SCOPE_PRIVATE;
		break;
	default:
		retval = SCTP_SCOPE_GLOBAL;
		break;
	}

	return retval;
}


static struct sock *sctp_v6_create_accept_sk(struct sock *sk,
					     struct sctp_association *asoc)
{
	struct sock *newsk;
	struct ipv6_pinfo *newnp, *np = inet6_sk(sk);
	struct sctp6_sock *newsctp6sk;

	newsk = sk_alloc(sock_net(sk), PF_INET6, GFP_KERNEL, sk->sk_prot);
	if (!newsk)
		goto out;

	sock_init_data(NULL, newsk);

	sctp_copy_sock(newsk, sk, asoc);
	sock_reset_flag(sk, SOCK_ZAPPED);

	newsctp6sk = (struct sctp6_sock *)newsk;
	inet_sk(newsk)->pinet6 = &newsctp6sk->inet6;

	sctp_sk(newsk)->v4mapped = sctp_sk(sk)->v4mapped;

	newnp = inet6_sk(newsk);

	memcpy(newnp, np, sizeof(struct ipv6_pinfo));

	
	sctp_v6_to_sk_daddr(&asoc->peer.primary_addr, newsk);

	sk_refcnt_debug_inc(newsk);

	if (newsk->sk_prot->init(newsk)) {
		sk_common_release(newsk);
		newsk = NULL;
	}

out:
	return newsk;
}


static void sctp_v6_addr_v4map(struct sctp_sock *sp, union sctp_addr *addr)
{
	if (sp->v4mapped && AF_INET == addr->sa.sa_family)
		sctp_v4_map_v6(addr);
}


static int sctp_v6_skb_iif(const struct sk_buff *skb)
{
	struct inet6_skb_parm *opt = (struct inet6_skb_parm *) skb->cb;
	return opt->iif;
}


static int sctp_v6_is_ce(const struct sk_buff *skb)
{
	return *((__u32 *)(ipv6_hdr(skb))) & htonl(1 << 20);
}


static void sctp_v6_seq_dump_addr(struct seq_file *seq, union sctp_addr *addr)
{
	seq_printf(seq, "%pI6 ", &addr->v6.sin6_addr);
}

static void sctp_v6_ecn_capable(struct sock *sk)
{
	inet6_sk(sk)->tclass |= INET_ECN_ECT_0;
}


static void sctp_inet6_msgname(char *msgname, int *addr_len)
{
	struct sockaddr_in6 *sin6;

	sin6 = (struct sockaddr_in6 *)msgname;
	sin6->sin6_family = AF_INET6;
	sin6->sin6_flowinfo = 0;
	sin6->sin6_scope_id = 0; 
	*addr_len = sizeof(struct sockaddr_in6);
}


static void sctp_inet6_event_msgname(struct sctp_ulpevent *event,
				     char *msgname, int *addrlen)
{
	struct sockaddr_in6 *sin6, *sin6from;

	if (msgname) {
		union sctp_addr *addr;
		struct sctp_association *asoc;

		asoc = event->asoc;
		sctp_inet6_msgname(msgname, addrlen);
		sin6 = (struct sockaddr_in6 *)msgname;
		sin6->sin6_port = htons(asoc->peer.port);
		addr = &asoc->peer.primary_addr;

		

		
		if (sctp_sk(asoc->base.sk)->v4mapped &&
		    AF_INET == addr->sa.sa_family) {
			sctp_v4_map_v6((union sctp_addr *)sin6);
			sin6->sin6_addr.s6_addr32[3] =
				addr->v4.sin_addr.s_addr;
			return;
		}

		sin6from = &asoc->peer.primary_addr.v6;
		ipv6_addr_copy(&sin6->sin6_addr, &sin6from->sin6_addr);
		if (ipv6_addr_type(&sin6->sin6_addr) & IPV6_ADDR_LINKLOCAL)
			sin6->sin6_scope_id = sin6from->sin6_scope_id;
	}
}


static void sctp_inet6_skb_msgname(struct sk_buff *skb, char *msgname,
				   int *addr_len)
{
	struct sctphdr *sh;
	struct sockaddr_in6 *sin6;

	if (msgname) {
		sctp_inet6_msgname(msgname, addr_len);
		sin6 = (struct sockaddr_in6 *)msgname;
		sh = sctp_hdr(skb);
		sin6->sin6_port = sh->source;

		
		if (sctp_sk(skb->sk)->v4mapped &&
		    ip_hdr(skb)->version == 4) {
			sctp_v4_map_v6((union sctp_addr *)sin6);
			sin6->sin6_addr.s6_addr32[3] = ip_hdr(skb)->saddr;
			return;
		}

		
		ipv6_addr_copy(&sin6->sin6_addr, &ipv6_hdr(skb)->saddr);
		if (ipv6_addr_type(&sin6->sin6_addr) & IPV6_ADDR_LINKLOCAL) {
			struct sctp_ulpevent *ev = sctp_skb2event(skb);
			sin6->sin6_scope_id = ev->iif;
		}
	}
}


static int sctp_inet6_af_supported(sa_family_t family, struct sctp_sock *sp)
{
	switch (family) {
	case AF_INET6:
		return 1;
	
	case AF_INET:
		if (!__ipv6_only_sock(sctp_opt2sk(sp)))
			return 1;
	default:
		return 0;
	}
}


static int sctp_inet6_cmp_addr(const union sctp_addr *addr1,
			       const union sctp_addr *addr2,
			       struct sctp_sock *opt)
{
	struct sctp_af *af1, *af2;
	struct sock *sk = sctp_opt2sk(opt);

	af1 = sctp_get_af_specific(addr1->sa.sa_family);
	af2 = sctp_get_af_specific(addr2->sa.sa_family);

	if (!af1 || !af2)
		return 0;

	
	if (__ipv6_only_sock(sk) && af1 != af2)
		return 0;

	
	if (sctp_is_any(sk, addr1) || sctp_is_any(sk, addr2))
		return 1;

	if (addr1->sa.sa_family != addr2->sa.sa_family)
		return 0;

	return af1->cmp_addr(addr1, addr2);
}


static int sctp_inet6_bind_verify(struct sctp_sock *opt, union sctp_addr *addr)
{
	struct sctp_af *af;

	
	if (addr->sa.sa_family != AF_INET6)
		af = sctp_get_af_specific(addr->sa.sa_family);
	else {
		int type = ipv6_addr_type(&addr->v6.sin6_addr);
		struct net_device *dev;

		if (type & IPV6_ADDR_LINKLOCAL) {
			if (!addr->v6.sin6_scope_id)
				return 0;
			dev = dev_get_by_index(&init_net, addr->v6.sin6_scope_id);
			if (!dev)
				return 0;
			if (!ipv6_chk_addr(&init_net, &addr->v6.sin6_addr,
					   dev, 0)) {
				dev_put(dev);
				return 0;
			}
			dev_put(dev);
		} else if (type == IPV6_ADDR_MAPPED) {
			if (!opt->v4mapped)
				return 0;
		}

		af = opt->pf->af;
	}
	return af->available(addr, opt);
}


static int sctp_inet6_send_verify(struct sctp_sock *opt, union sctp_addr *addr)
{
	struct sctp_af *af = NULL;

	
	if (addr->sa.sa_family != AF_INET6)
		af = sctp_get_af_specific(addr->sa.sa_family);
	else {
		int type = ipv6_addr_type(&addr->v6.sin6_addr);
		struct net_device *dev;

		if (type & IPV6_ADDR_LINKLOCAL) {
			if (!addr->v6.sin6_scope_id)
				return 0;
			dev = dev_get_by_index(&init_net, addr->v6.sin6_scope_id);
			if (!dev)
				return 0;
			dev_put(dev);
		}
		af = opt->pf->af;
	}

	return af != NULL;
}


static int sctp_inet6_supported_addrs(const struct sctp_sock *opt,
				      __be16 *types)
{
	types[0] = SCTP_PARAM_IPV6_ADDRESS;
	if (!opt || !ipv6_only_sock(sctp_opt2sk(opt))) {
		types[1] = SCTP_PARAM_IPV4_ADDRESS;
		return 2;
	}
	return 1;
}

static const struct proto_ops inet6_seqpacket_ops = {
	.family		   = PF_INET6,
	.owner		   = THIS_MODULE,
	.release	   = inet6_release,
	.bind		   = inet6_bind,
	.connect	   = inet_dgram_connect,
	.socketpair	   = sock_no_socketpair,
	.accept		   = inet_accept,
	.getname	   = inet6_getname,
	.poll		   = sctp_poll,
	.ioctl		   = inet6_ioctl,
	.listen		   = sctp_inet_listen,
	.shutdown	   = inet_shutdown,
	.setsockopt	   = sock_common_setsockopt,
	.getsockopt	   = sock_common_getsockopt,
	.sendmsg	   = inet_sendmsg,
	.recvmsg	   = sock_common_recvmsg,
	.mmap		   = sock_no_mmap,
#ifdef CONFIG_COMPAT
	.compat_setsockopt = compat_sock_common_setsockopt,
	.compat_getsockopt = compat_sock_common_getsockopt,
#endif
};

static struct inet_protosw sctpv6_seqpacket_protosw = {
	.type          = SOCK_SEQPACKET,
	.protocol      = IPPROTO_SCTP,
	.prot 	       = &sctpv6_prot,
	.ops           = &inet6_seqpacket_ops,
	.capability    = -1,
	.no_check      = 0,
	.flags         = SCTP_PROTOSW_FLAG
};
static struct inet_protosw sctpv6_stream_protosw = {
	.type          = SOCK_STREAM,
	.protocol      = IPPROTO_SCTP,
	.prot 	       = &sctpv6_prot,
	.ops           = &inet6_seqpacket_ops,
	.capability    = -1,
	.no_check      = 0,
	.flags         = SCTP_PROTOSW_FLAG,
};

static int sctp6_rcv(struct sk_buff *skb)
{
	return sctp_rcv(skb) ? -1 : 0;
}

static const struct inet6_protocol sctpv6_protocol = {
	.handler      = sctp6_rcv,
	.err_handler  = sctp_v6_err,
	.flags        = INET6_PROTO_NOPOLICY | INET6_PROTO_FINAL,
};

static struct sctp_af sctp_af_inet6 = {
	.sa_family	   = AF_INET6,
	.sctp_xmit	   = sctp_v6_xmit,
	.setsockopt	   = ipv6_setsockopt,
	.getsockopt	   = ipv6_getsockopt,
	.get_dst	   = sctp_v6_get_dst,
	.get_saddr	   = sctp_v6_get_saddr,
	.copy_addrlist	   = sctp_v6_copy_addrlist,
	.from_skb	   = sctp_v6_from_skb,
	.from_sk	   = sctp_v6_from_sk,
	.to_sk_saddr	   = sctp_v6_to_sk_saddr,
	.to_sk_daddr	   = sctp_v6_to_sk_daddr,
	.from_addr_param   = sctp_v6_from_addr_param,
	.to_addr_param	   = sctp_v6_to_addr_param,
	.dst_saddr	   = sctp_v6_dst_saddr,
	.cmp_addr	   = sctp_v6_cmp_addr,
	.scope		   = sctp_v6_scope,
	.addr_valid	   = sctp_v6_addr_valid,
	.inaddr_any	   = sctp_v6_inaddr_any,
	.is_any		   = sctp_v6_is_any,
	.available	   = sctp_v6_available,
	.skb_iif	   = sctp_v6_skb_iif,
	.is_ce		   = sctp_v6_is_ce,
	.seq_dump_addr	   = sctp_v6_seq_dump_addr,
	.ecn_capable	   = sctp_v6_ecn_capable,
	.net_header_len	   = sizeof(struct ipv6hdr),
	.sockaddr_len	   = sizeof(struct sockaddr_in6),
#ifdef CONFIG_COMPAT
	.compat_setsockopt = compat_ipv6_setsockopt,
	.compat_getsockopt = compat_ipv6_getsockopt,
#endif
};

static struct sctp_pf sctp_pf_inet6 = {
	.event_msgname = sctp_inet6_event_msgname,
	.skb_msgname   = sctp_inet6_skb_msgname,
	.af_supported  = sctp_inet6_af_supported,
	.cmp_addr      = sctp_inet6_cmp_addr,
	.bind_verify   = sctp_inet6_bind_verify,
	.send_verify   = sctp_inet6_send_verify,
	.supported_addrs = sctp_inet6_supported_addrs,
	.create_accept_sk = sctp_v6_create_accept_sk,
	.addr_v4map    = sctp_v6_addr_v4map,
	.af            = &sctp_af_inet6,
};


void sctp_v6_pf_init(void)
{
	
	sctp_register_pf(&sctp_pf_inet6, PF_INET6);

	
	sctp_register_af(&sctp_af_inet6);
}

void sctp_v6_pf_exit(void)
{
	list_del(&sctp_af_inet6.list);
}


int sctp_v6_protosw_init(void)
{
	int rc;

	rc = proto_register(&sctpv6_prot, 1);
	if (rc)
		return rc;

	
	inet6_register_protosw(&sctpv6_seqpacket_protosw);
	inet6_register_protosw(&sctpv6_stream_protosw);

	return 0;
}

void sctp_v6_protosw_exit(void)
{
	inet6_unregister_protosw(&sctpv6_seqpacket_protosw);
	inet6_unregister_protosw(&sctpv6_stream_protosw);
	proto_unregister(&sctpv6_prot);
}



int sctp_v6_add_protocol(void)
{
	
	register_inet6addr_notifier(&sctp_inet6addr_notifier);

	if (inet6_add_protocol(&sctpv6_protocol, IPPROTO_SCTP) < 0)
		return -EAGAIN;

	return 0;
}


void sctp_v6_del_protocol(void)
{
	inet6_del_protocol(&sctpv6_protocol, IPPROTO_SCTP);
	unregister_inet6addr_notifier(&sctp_inet6addr_notifier);
}
