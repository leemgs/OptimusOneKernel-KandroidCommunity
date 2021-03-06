

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/udp.h>

#include <net/netfilter/nf_nat_helper.h>
#include <net/netfilter/nf_nat_rule.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <linux/netfilter/nf_conntrack_amanda.h>

MODULE_AUTHOR("Brian J. Murrell <netfilter@interlinx.bc.ca>");
MODULE_DESCRIPTION("Amanda NAT helper");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ip_nat_amanda");

static unsigned int help(struct sk_buff *skb,
			 enum ip_conntrack_info ctinfo,
			 unsigned int matchoff,
			 unsigned int matchlen,
			 struct nf_conntrack_expect *exp)
{
	char buffer[sizeof("65535")];
	u_int16_t port;
	unsigned int ret;

	
	exp->saved_proto.tcp.port = exp->tuple.dst.u.tcp.port;
	exp->dir = IP_CT_DIR_ORIGINAL;

	
	exp->expectfn = nf_nat_follow_master;

	
	for (port = ntohs(exp->saved_proto.tcp.port); port != 0; port++) {
		exp->tuple.dst.u.tcp.port = htons(port);
		if (nf_ct_expect_related(exp) == 0)
			break;
	}

	if (port == 0)
		return NF_DROP;

	sprintf(buffer, "%u", port);
	ret = nf_nat_mangle_udp_packet(skb, exp->master, ctinfo,
				       matchoff, matchlen,
				       buffer, strlen(buffer));
	if (ret != NF_ACCEPT)
		nf_ct_unexpect_related(exp);
	return ret;
}

static void __exit nf_nat_amanda_fini(void)
{
	rcu_assign_pointer(nf_nat_amanda_hook, NULL);
	synchronize_rcu();
}

static int __init nf_nat_amanda_init(void)
{
	BUG_ON(nf_nat_amanda_hook != NULL);
	rcu_assign_pointer(nf_nat_amanda_hook, help);
	return 0;
}

module_init(nf_nat_amanda_init);
module_exit(nf_nat_amanda_fini);
