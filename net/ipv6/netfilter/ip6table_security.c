
#include <linux/module.h>
#include <linux/netfilter_ipv6/ip6_tables.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("James Morris <jmorris <at> redhat.com>");
MODULE_DESCRIPTION("ip6tables security table, for MAC rules");

#define SECURITY_VALID_HOOKS	(1 << NF_INET_LOCAL_IN) | \
				(1 << NF_INET_FORWARD) | \
				(1 << NF_INET_LOCAL_OUT)

static const struct
{
	struct ip6t_replace repl;
	struct ip6t_standard entries[3];
	struct ip6t_error term;
} initial_table __net_initdata = {
	.repl = {
		.name = "security",
		.valid_hooks = SECURITY_VALID_HOOKS,
		.num_entries = 4,
		.size = sizeof(struct ip6t_standard) * 3 + sizeof(struct ip6t_error),
		.hook_entry = {
			[NF_INET_LOCAL_IN] 	= 0,
			[NF_INET_FORWARD] 	= sizeof(struct ip6t_standard),
			[NF_INET_LOCAL_OUT] 	= sizeof(struct ip6t_standard) * 2,
		},
		.underflow = {
			[NF_INET_LOCAL_IN] 	= 0,
			[NF_INET_FORWARD] 	= sizeof(struct ip6t_standard),
			[NF_INET_LOCAL_OUT] 	= sizeof(struct ip6t_standard) * 2,
		},
	},
	.entries = {
		IP6T_STANDARD_INIT(NF_ACCEPT),	
		IP6T_STANDARD_INIT(NF_ACCEPT),	
		IP6T_STANDARD_INIT(NF_ACCEPT),	
	},
	.term = IP6T_ERROR_INIT,		
};

static const struct xt_table security_table = {
	.name		= "security",
	.valid_hooks	= SECURITY_VALID_HOOKS,
	.me		= THIS_MODULE,
	.af		= NFPROTO_IPV6,
};

static unsigned int
ip6t_local_in_hook(unsigned int hook,
		   struct sk_buff *skb,
		   const struct net_device *in,
		   const struct net_device *out,
		   int (*okfn)(struct sk_buff *))
{
	return ip6t_do_table(skb, hook, in, out,
			     dev_net(in)->ipv6.ip6table_security);
}

static unsigned int
ip6t_forward_hook(unsigned int hook,
		  struct sk_buff *skb,
		  const struct net_device *in,
		  const struct net_device *out,
		  int (*okfn)(struct sk_buff *))
{
	return ip6t_do_table(skb, hook, in, out,
			     dev_net(in)->ipv6.ip6table_security);
}

static unsigned int
ip6t_local_out_hook(unsigned int hook,
		    struct sk_buff *skb,
		    const struct net_device *in,
		    const struct net_device *out,
		    int (*okfn)(struct sk_buff *))
{
	
	return ip6t_do_table(skb, hook, in, out,
			     dev_net(out)->ipv6.ip6table_security);
}

static struct nf_hook_ops ip6t_ops[] __read_mostly = {
	{
		.hook		= ip6t_local_in_hook,
		.owner		= THIS_MODULE,
		.pf		= NFPROTO_IPV6,
		.hooknum	= NF_INET_LOCAL_IN,
		.priority	= NF_IP6_PRI_SECURITY,
	},
	{
		.hook		= ip6t_forward_hook,
		.owner		= THIS_MODULE,
		.pf		= NFPROTO_IPV6,
		.hooknum	= NF_INET_FORWARD,
		.priority	= NF_IP6_PRI_SECURITY,
	},
	{
		.hook		= ip6t_local_out_hook,
		.owner		= THIS_MODULE,
		.pf		= NFPROTO_IPV6,
		.hooknum	= NF_INET_LOCAL_OUT,
		.priority	= NF_IP6_PRI_SECURITY,
	},
};

static int __net_init ip6table_security_net_init(struct net *net)
{
	net->ipv6.ip6table_security =
		ip6t_register_table(net, &security_table, &initial_table.repl);

	if (IS_ERR(net->ipv6.ip6table_security))
		return PTR_ERR(net->ipv6.ip6table_security);

	return 0;
}

static void __net_exit ip6table_security_net_exit(struct net *net)
{
	ip6t_unregister_table(net->ipv6.ip6table_security);
}

static struct pernet_operations ip6table_security_net_ops = {
	.init = ip6table_security_net_init,
	.exit = ip6table_security_net_exit,
};

static int __init ip6table_security_init(void)
{
	int ret;

	ret = register_pernet_subsys(&ip6table_security_net_ops);
	if (ret < 0)
		return ret;

	ret = nf_register_hooks(ip6t_ops, ARRAY_SIZE(ip6t_ops));
	if (ret < 0)
		goto cleanup_table;

	return ret;

cleanup_table:
	unregister_pernet_subsys(&ip6table_security_net_ops);
	return ret;
}

static void __exit ip6table_security_fini(void)
{
	nf_unregister_hooks(ip6t_ops, ARRAY_SIZE(ip6t_ops));
	unregister_pernet_subsys(&ip6table_security_net_ops);
}

module_init(ip6table_security_init);
module_exit(ip6table_security_fini);
