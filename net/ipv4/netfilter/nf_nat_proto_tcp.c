

#include <linux/types.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/tcp.h>

#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink_conntrack.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_rule.h>
#include <net/netfilter/nf_nat_protocol.h>
#include <net/netfilter/nf_nat_core.h>

static u_int16_t tcp_port_rover;

static bool
tcp_unique_tuple(struct nf_conntrack_tuple *tuple,
		 const struct nf_nat_range *range,
		 enum nf_nat_manip_type maniptype,
		 const struct nf_conn *ct)
{
	return nf_nat_proto_unique_tuple(tuple, range, maniptype, ct,
					 &tcp_port_rover);
}

static bool
tcp_manip_pkt(struct sk_buff *skb,
	      unsigned int iphdroff,
	      const struct nf_conntrack_tuple *tuple,
	      enum nf_nat_manip_type maniptype)
{
	const struct iphdr *iph = (struct iphdr *)(skb->data + iphdroff);
	struct tcphdr *hdr;
	unsigned int hdroff = iphdroff + iph->ihl*4;
	__be32 oldip, newip;
	__be16 *portptr, newport, oldport;
	int hdrsize = 8; 

	
	if (skb->len >= hdroff + sizeof(struct tcphdr))
		hdrsize = sizeof(struct tcphdr);

	if (!skb_make_writable(skb, hdroff + hdrsize))
		return false;

	iph = (struct iphdr *)(skb->data + iphdroff);
	hdr = (struct tcphdr *)(skb->data + hdroff);

	if (maniptype == IP_NAT_MANIP_SRC) {
		
		oldip = iph->saddr;
		newip = tuple->src.u3.ip;
		newport = tuple->src.u.tcp.port;
		portptr = &hdr->source;
	} else {
		
		oldip = iph->daddr;
		newip = tuple->dst.u3.ip;
		newport = tuple->dst.u.tcp.port;
		portptr = &hdr->dest;
	}

	oldport = *portptr;
	*portptr = newport;

	if (hdrsize < sizeof(*hdr))
		return true;

	inet_proto_csum_replace4(&hdr->check, skb, oldip, newip, 1);
	inet_proto_csum_replace2(&hdr->check, skb, oldport, newport, 0);
	return true;
}

const struct nf_nat_protocol nf_nat_protocol_tcp = {
	.protonum		= IPPROTO_TCP,
	.me			= THIS_MODULE,
	.manip_pkt		= tcp_manip_pkt,
	.in_range		= nf_nat_proto_in_range,
	.unique_tuple		= tcp_unique_tuple,
#if defined(CONFIG_NF_CT_NETLINK) || defined(CONFIG_NF_CT_NETLINK_MODULE)
	.range_to_nlattr	= nf_nat_proto_range_to_nlattr,
	.nlattr_to_range	= nf_nat_proto_nlattr_to_range,
#endif
};
