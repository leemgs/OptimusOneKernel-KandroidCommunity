

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <net/ip.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netfilter Core Team <coreteam@netfilter.org>");
MODULE_DESCRIPTION("iptables filter table");

#define FILTER_VALID_HOOKS ((1 << NF_INET_LOCAL_IN) | \
			    (1 << NF_INET_FORWARD) | \
			    (1 << NF_INET_LOCAL_OUT))

static struct
{
	struct ipt_replace repl;
	struct ipt_standard entries[3];
	struct ipt_error term;
} initial_table __net_initdata = {
	.repl = {
		.name = "filter",
		.valid_hooks = FILTER_VALID_HOOKS,
		.num_entries = 4,
		.size = sizeof(struct ipt_standard) * 3 + sizeof(struct ipt_error),
		.hook_entry = {
			[NF_INET_LOCAL_IN] = 0,
			[NF_INET_FORWARD] = sizeof(struct ipt_standard),
			[NF_INET_LOCAL_OUT] = sizeof(struct ipt_standard) * 2,
		},
		.underflow = {
			[NF_INET_LOCAL_IN] = 0,
			[NF_INET_FORWARD] = sizeof(struct ipt_standard),
			[NF_INET_LOCAL_OUT] = sizeof(struct ipt_standard) * 2,
		},
	},
	.entries = {
		IPT_STANDARD_INIT(NF_ACCEPT),	
		IPT_STANDARD_INIT(NF_ACCEPT),	
		IPT_STANDARD_INIT(NF_ACCEPT),	
	},
	.term = IPT_ERROR_INIT,			
};

static const struct xt_table packet_filter = {
	.name		= "filter",
	.valid_hooks	= FILTER_VALID_HOOKS,
	.me		= THIS_MODULE,
	.af		= NFPROTO_IPV4,
};


static unsigned int
ipt_local_in_hook(unsigned int hook,
		  struct sk_buff *skb,
		  const struct net_device *in,
		  const struct net_device *out,
		  int (*okfn)(struct sk_buff *))
{
	return ipt_do_table(skb, hook, in, out,
			    dev_net(in)->ipv4.iptable_filter);
}

static unsigned int
ipt_hook(unsigned int hook,
	 struct sk_buff *skb,
	 const struct net_device *in,
	 const struct net_device *out,
	 int (*okfn)(struct sk_buff *))
{
	return ipt_do_table(skb, hook, in, out,
			    dev_net(in)->ipv4.iptable_filter);
}

static unsigned int
ipt_local_out_hook(unsigned int hook,
		   struct sk_buff *skb,
		   const struct net_device *in,
		   const struct net_device *out,
		   int (*okfn)(struct sk_buff *))
{
	
	if (skb->len < sizeof(struct iphdr) ||
	    ip_hdrlen(skb) < sizeof(struct iphdr))
		return NF_ACCEPT;
	return ipt_do_table(skb, hook, in, out,
			    dev_net(out)->ipv4.iptable_filter);
}

static struct nf_hook_ops ipt_ops[] __read_mostly = {
	{
		.hook		= ipt_local_in_hook,
		.owner		= THIS_MODULE,
		.pf		= NFPROTO_IPV4,
		.hooknum	= NF_INET_LOCAL_IN,
		.priority	= NF_IP_PRI_FILTER,
	},
	{
		.hook		= ipt_hook,
		.owner		= THIS_MODULE,
		.pf		= NFPROTO_IPV4,
		.hooknum	= NF_INET_FORWARD,
		.priority	= NF_IP_PRI_FILTER,
	},
	{
		.hook		= ipt_local_out_hook,
		.owner		= THIS_MODULE,
		.pf		= NFPROTO_IPV4,
		.hooknum	= NF_INET_LOCAL_OUT,
		.priority	= NF_IP_PRI_FILTER,
	},
};


static int forward = NF_ACCEPT;
module_param(forward, bool, 0000);

static int __net_init iptable_filter_net_init(struct net *net)
{
	
	net->ipv4.iptable_filter =
		ipt_register_table(net, &packet_filter, &initial_table.repl);
	if (IS_ERR(net->ipv4.iptable_filter))
		return PTR_ERR(net->ipv4.iptable_filter);
	return 0;
}

static void __net_exit iptable_filter_net_exit(struct net *net)
{
	ipt_unregister_table(net->ipv4.iptable_filter);
}

static struct pernet_operations iptable_filter_net_ops = {
	.init = iptable_filter_net_init,
	.exit = iptable_filter_net_exit,
};

static int __init iptable_filter_init(void)
{
	int ret;

	if (forward < 0 || forward > NF_MAX_VERDICT) {
		printk("iptables forward must be 0 or 1\n");
		return -EINVAL;
	}

	
	initial_table.entries[1].target.verdict = -forward - 1;

	ret = register_pernet_subsys(&iptable_filter_net_ops);
	if (ret < 0)
		return ret;

	
	ret = nf_register_hooks(ipt_ops, ARRAY_SIZE(ipt_ops));
	if (ret < 0)
		goto cleanup_table;

	return ret;

 cleanup_table:
	unregister_pernet_subsys(&iptable_filter_net_ops);
	return ret;
}

static void __exit iptable_filter_fini(void)
{
	nf_unregister_hooks(ipt_ops, ARRAY_SIZE(ipt_ops));
	unregister_pernet_subsys(&iptable_filter_net_ops);
}

module_init(iptable_filter_init);
module_exit(iptable_filter_fini);
