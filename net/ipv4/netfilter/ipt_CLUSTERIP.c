
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/jhash.h>
#include <linux/bitops.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/if_arp.h>
#include <linux/seq_file.h>
#include <linux/netfilter_arp.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_CLUSTERIP.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/net_namespace.h>
#include <net/checksum.h>

#define CLUSTERIP_VERSION "0.8"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harald Welte <laforge@netfilter.org>");
MODULE_DESCRIPTION("Xtables: CLUSTERIP target");

struct clusterip_config {
	struct list_head list;			
	atomic_t refcount;			
	atomic_t entries;			

	__be32 clusterip;			
	u_int8_t clustermac[ETH_ALEN];		
	struct net_device *dev;			
	u_int16_t num_total_nodes;		
	unsigned long local_nodes;		

#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *pde;		
#endif
	enum clusterip_hashmode hash_mode;	
	u_int32_t hash_initval;			
};

static LIST_HEAD(clusterip_configs);


static DEFINE_RWLOCK(clusterip_lock);

#ifdef CONFIG_PROC_FS
static const struct file_operations clusterip_proc_fops;
static struct proc_dir_entry *clusterip_procdir;
#endif

static inline void
clusterip_config_get(struct clusterip_config *c)
{
	atomic_inc(&c->refcount);
}

static inline void
clusterip_config_put(struct clusterip_config *c)
{
	if (atomic_dec_and_test(&c->refcount))
		kfree(c);
}


static inline void
clusterip_config_entry_put(struct clusterip_config *c)
{
	write_lock_bh(&clusterip_lock);
	if (atomic_dec_and_test(&c->entries)) {
		list_del(&c->list);
		write_unlock_bh(&clusterip_lock);

		dev_mc_delete(c->dev, c->clustermac, ETH_ALEN, 0);
		dev_put(c->dev);

		
#ifdef CONFIG_PROC_FS
		remove_proc_entry(c->pde->name, c->pde->parent);
#endif
		return;
	}
	write_unlock_bh(&clusterip_lock);
}

static struct clusterip_config *
__clusterip_config_find(__be32 clusterip)
{
	struct clusterip_config *c;

	list_for_each_entry(c, &clusterip_configs, list) {
		if (c->clusterip == clusterip)
			return c;
	}

	return NULL;
}

static inline struct clusterip_config *
clusterip_config_find_get(__be32 clusterip, int entry)
{
	struct clusterip_config *c;

	read_lock_bh(&clusterip_lock);
	c = __clusterip_config_find(clusterip);
	if (!c) {
		read_unlock_bh(&clusterip_lock);
		return NULL;
	}
	atomic_inc(&c->refcount);
	if (entry)
		atomic_inc(&c->entries);
	read_unlock_bh(&clusterip_lock);

	return c;
}

static void
clusterip_config_init_nodelist(struct clusterip_config *c,
			       const struct ipt_clusterip_tgt_info *i)
{
	int n;

	for (n = 0; n < i->num_local_nodes; n++)
		set_bit(i->local_nodes[n] - 1, &c->local_nodes);
}

static struct clusterip_config *
clusterip_config_init(const struct ipt_clusterip_tgt_info *i, __be32 ip,
			struct net_device *dev)
{
	struct clusterip_config *c;

	c = kzalloc(sizeof(*c), GFP_ATOMIC);
	if (!c)
		return NULL;

	c->dev = dev;
	c->clusterip = ip;
	memcpy(&c->clustermac, &i->clustermac, ETH_ALEN);
	c->num_total_nodes = i->num_total_nodes;
	clusterip_config_init_nodelist(c, i);
	c->hash_mode = i->hash_mode;
	c->hash_initval = i->hash_initval;
	atomic_set(&c->refcount, 1);
	atomic_set(&c->entries, 1);

#ifdef CONFIG_PROC_FS
	{
		char buffer[16];

		
		sprintf(buffer, "%pI4", &ip);
		c->pde = proc_create_data(buffer, S_IWUSR|S_IRUSR,
					  clusterip_procdir,
					  &clusterip_proc_fops, c);
		if (!c->pde) {
			kfree(c);
			return NULL;
		}
	}
#endif

	write_lock_bh(&clusterip_lock);
	list_add(&c->list, &clusterip_configs);
	write_unlock_bh(&clusterip_lock);

	return c;
}

#ifdef CONFIG_PROC_FS
static int
clusterip_add_node(struct clusterip_config *c, u_int16_t nodenum)
{

	if (nodenum == 0 ||
	    nodenum > c->num_total_nodes)
		return 1;

	
	if (test_and_set_bit(nodenum - 1, &c->local_nodes))
		return 1;

	return 0;
}

static bool
clusterip_del_node(struct clusterip_config *c, u_int16_t nodenum)
{
	if (nodenum == 0 ||
	    nodenum > c->num_total_nodes)
		return true;

	if (test_and_clear_bit(nodenum - 1, &c->local_nodes))
		return false;

	return true;
}
#endif

static inline u_int32_t
clusterip_hashfn(const struct sk_buff *skb,
		 const struct clusterip_config *config)
{
	const struct iphdr *iph = ip_hdr(skb);
	unsigned long hashval;
	u_int16_t sport, dport;
	const u_int16_t *ports;

	switch (iph->protocol) {
	case IPPROTO_TCP:
	case IPPROTO_UDP:
	case IPPROTO_UDPLITE:
	case IPPROTO_SCTP:
	case IPPROTO_DCCP:
	case IPPROTO_ICMP:
		ports = (const void *)iph+iph->ihl*4;
		sport = ports[0];
		dport = ports[1];
		break;
	default:
		if (net_ratelimit())
			printk(KERN_NOTICE "CLUSTERIP: unknown protocol `%u'\n",
				iph->protocol);
		sport = dport = 0;
	}

	switch (config->hash_mode) {
	case CLUSTERIP_HASHMODE_SIP:
		hashval = jhash_1word(ntohl(iph->saddr),
				      config->hash_initval);
		break;
	case CLUSTERIP_HASHMODE_SIP_SPT:
		hashval = jhash_2words(ntohl(iph->saddr), sport,
				       config->hash_initval);
		break;
	case CLUSTERIP_HASHMODE_SIP_SPT_DPT:
		hashval = jhash_3words(ntohl(iph->saddr), sport, dport,
				       config->hash_initval);
		break;
	default:
		
		hashval = 0;
		
		printk("CLUSTERIP: unknown mode `%u'\n", config->hash_mode);
		BUG();
		break;
	}

	
	return (((u64)hashval * config->num_total_nodes) >> 32) + 1;
}

static inline int
clusterip_responsible(const struct clusterip_config *config, u_int32_t hash)
{
	return test_bit(hash - 1, &config->local_nodes);
}



static unsigned int
clusterip_tg(struct sk_buff *skb, const struct xt_target_param *par)
{
	const struct ipt_clusterip_tgt_info *cipinfo = par->targinfo;
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	u_int32_t hash;

	

	ct = nf_ct_get(skb, &ctinfo);
	if (ct == NULL) {
		printk(KERN_ERR "CLUSTERIP: no conntrack!\n");
			
		return NF_DROP;
	}

	
	if (ip_hdr(skb)->protocol == IPPROTO_ICMP
	    && (ctinfo == IP_CT_RELATED
		|| ctinfo == IP_CT_RELATED+IP_CT_IS_REPLY))
		return XT_CONTINUE;

	

	hash = clusterip_hashfn(skb, cipinfo->config);

	switch (ctinfo) {
		case IP_CT_NEW:
			ct->mark = hash;
			break;
		case IP_CT_RELATED:
		case IP_CT_RELATED+IP_CT_IS_REPLY:
			
		case IP_CT_ESTABLISHED:
		case IP_CT_ESTABLISHED+IP_CT_IS_REPLY:
			break;
		default:
			break;
	}

#ifdef DEBUG
	nf_ct_dump_tuple_ip(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
#endif
	pr_debug("hash=%u ct_hash=%u ", hash, ct->mark);
	if (!clusterip_responsible(cipinfo->config, hash)) {
		pr_debug("not responsible\n");
		return NF_DROP;
	}
	pr_debug("responsible\n");

	
	skb->pkt_type = PACKET_HOST;

	return XT_CONTINUE;
}

static bool clusterip_tg_check(const struct xt_tgchk_param *par)
{
	struct ipt_clusterip_tgt_info *cipinfo = par->targinfo;
	const struct ipt_entry *e = par->entryinfo;

	struct clusterip_config *config;

	if (cipinfo->hash_mode != CLUSTERIP_HASHMODE_SIP &&
	    cipinfo->hash_mode != CLUSTERIP_HASHMODE_SIP_SPT &&
	    cipinfo->hash_mode != CLUSTERIP_HASHMODE_SIP_SPT_DPT) {
		printk(KERN_WARNING "CLUSTERIP: unknown mode `%u'\n",
			cipinfo->hash_mode);
		return false;

	}
	if (e->ip.dmsk.s_addr != htonl(0xffffffff)
	    || e->ip.dst.s_addr == 0) {
		printk(KERN_ERR "CLUSTERIP: Please specify destination IP\n");
		return false;
	}

	

	config = clusterip_config_find_get(e->ip.dst.s_addr, 1);
	if (!config) {
		if (!(cipinfo->flags & CLUSTERIP_FLAG_NEW)) {
			printk(KERN_WARNING "CLUSTERIP: no config found for %pI4, need 'new'\n", &e->ip.dst.s_addr);
			return false;
		} else {
			struct net_device *dev;

			if (e->ip.iniface[0] == '\0') {
				printk(KERN_WARNING "CLUSTERIP: Please specify an interface name\n");
				return false;
			}

			dev = dev_get_by_name(&init_net, e->ip.iniface);
			if (!dev) {
				printk(KERN_WARNING "CLUSTERIP: no such interface %s\n", e->ip.iniface);
				return false;
			}

			config = clusterip_config_init(cipinfo,
							e->ip.dst.s_addr, dev);
			if (!config) {
				printk(KERN_WARNING "CLUSTERIP: cannot allocate config\n");
				dev_put(dev);
				return false;
			}
			dev_mc_add(config->dev,config->clustermac, ETH_ALEN, 0);
		}
	}
	cipinfo->config = config;

	if (nf_ct_l3proto_try_module_get(par->target->family) < 0) {
		printk(KERN_WARNING "can't load conntrack support for "
				    "proto=%u\n", par->target->family);
		return false;
	}

	return true;
}


static void clusterip_tg_destroy(const struct xt_tgdtor_param *par)
{
	const struct ipt_clusterip_tgt_info *cipinfo = par->targinfo;

	
	clusterip_config_entry_put(cipinfo->config);

	clusterip_config_put(cipinfo->config);

	nf_ct_l3proto_module_put(par->target->family);
}

#ifdef CONFIG_COMPAT
struct compat_ipt_clusterip_tgt_info
{
	u_int32_t	flags;
	u_int8_t	clustermac[6];
	u_int16_t	num_total_nodes;
	u_int16_t	num_local_nodes;
	u_int16_t	local_nodes[CLUSTERIP_MAX_NODES];
	u_int32_t	hash_mode;
	u_int32_t	hash_initval;
	compat_uptr_t	config;
};
#endif 

static struct xt_target clusterip_tg_reg __read_mostly = {
	.name		= "CLUSTERIP",
	.family		= NFPROTO_IPV4,
	.target		= clusterip_tg,
	.checkentry	= clusterip_tg_check,
	.destroy	= clusterip_tg_destroy,
	.targetsize	= sizeof(struct ipt_clusterip_tgt_info),
#ifdef CONFIG_COMPAT
	.compatsize	= sizeof(struct compat_ipt_clusterip_tgt_info),
#endif 
	.me		= THIS_MODULE
};





struct arp_payload {
	u_int8_t src_hw[ETH_ALEN];
	__be32 src_ip;
	u_int8_t dst_hw[ETH_ALEN];
	__be32 dst_ip;
} __attribute__ ((packed));

#ifdef DEBUG
static void arp_print(struct arp_payload *payload)
{
#define HBUFFERLEN 30
	char hbuffer[HBUFFERLEN];
	int j,k;

	for (k=0, j=0; k < HBUFFERLEN-3 && j < ETH_ALEN; j++) {
		hbuffer[k++] = hex_asc_hi(payload->src_hw[j]);
		hbuffer[k++] = hex_asc_lo(payload->src_hw[j]);
		hbuffer[k++]=':';
	}
	hbuffer[--k]='\0';

	printk("src %pI4@%s, dst %pI4\n",
		&payload->src_ip, hbuffer, &payload->dst_ip);
}
#endif

static unsigned int
arp_mangle(unsigned int hook,
	   struct sk_buff *skb,
	   const struct net_device *in,
	   const struct net_device *out,
	   int (*okfn)(struct sk_buff *))
{
	struct arphdr *arp = arp_hdr(skb);
	struct arp_payload *payload;
	struct clusterip_config *c;

	
	if (arp->ar_hrd != htons(ARPHRD_ETHER)
	    || arp->ar_pro != htons(ETH_P_IP)
	    || arp->ar_pln != 4 || arp->ar_hln != ETH_ALEN)
		return NF_ACCEPT;

	
	if (arp->ar_op != htons(ARPOP_REPLY)
	    && arp->ar_op != htons(ARPOP_REQUEST))
		return NF_ACCEPT;

	payload = (void *)(arp+1);

	
	c = clusterip_config_find_get(payload->src_ip, 0);
	if (!c)
		return NF_ACCEPT;

	
	if (c->dev != out) {
		pr_debug("CLUSTERIP: not mangling arp reply on different "
			 "interface: cip'%s'-skb'%s'\n",
			 c->dev->name, out->name);
		clusterip_config_put(c);
		return NF_ACCEPT;
	}

	
	memcpy(payload->src_hw, c->clustermac, arp->ar_hln);

#ifdef DEBUG
	pr_debug(KERN_DEBUG "CLUSTERIP mangled arp reply: ");
	arp_print(payload);
#endif

	clusterip_config_put(c);

	return NF_ACCEPT;
}

static struct nf_hook_ops cip_arp_ops __read_mostly = {
	.hook = arp_mangle,
	.pf = NFPROTO_ARP,
	.hooknum = NF_ARP_OUT,
	.priority = -1
};



#ifdef CONFIG_PROC_FS

struct clusterip_seq_position {
	unsigned int pos;	
	unsigned int weight;	
	unsigned int bit;	
	unsigned long val;	
};

static void *clusterip_seq_start(struct seq_file *s, loff_t *pos)
{
	const struct proc_dir_entry *pde = s->private;
	struct clusterip_config *c = pde->data;
	unsigned int weight;
	u_int32_t local_nodes;
	struct clusterip_seq_position *idx;

	
	local_nodes = c->local_nodes;
	weight = hweight32(local_nodes);
	if (*pos >= weight)
		return NULL;

	idx = kmalloc(sizeof(struct clusterip_seq_position), GFP_KERNEL);
	if (!idx)
		return ERR_PTR(-ENOMEM);

	idx->pos = *pos;
	idx->weight = weight;
	idx->bit = ffs(local_nodes);
	idx->val = local_nodes;
	clear_bit(idx->bit - 1, &idx->val);

	return idx;
}

static void *clusterip_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	struct clusterip_seq_position *idx = v;

	*pos = ++idx->pos;
	if (*pos >= idx->weight) {
		kfree(v);
		return NULL;
	}
	idx->bit = ffs(idx->val);
	clear_bit(idx->bit - 1, &idx->val);
	return idx;
}

static void clusterip_seq_stop(struct seq_file *s, void *v)
{
	kfree(v);
}

static int clusterip_seq_show(struct seq_file *s, void *v)
{
	struct clusterip_seq_position *idx = v;

	if (idx->pos != 0)
		seq_putc(s, ',');

	seq_printf(s, "%u", idx->bit);

	if (idx->pos == idx->weight - 1)
		seq_putc(s, '\n');

	return 0;
}

static const struct seq_operations clusterip_seq_ops = {
	.start	= clusterip_seq_start,
	.next	= clusterip_seq_next,
	.stop	= clusterip_seq_stop,
	.show	= clusterip_seq_show,
};

static int clusterip_proc_open(struct inode *inode, struct file *file)
{
	int ret = seq_open(file, &clusterip_seq_ops);

	if (!ret) {
		struct seq_file *sf = file->private_data;
		struct proc_dir_entry *pde = PDE(inode);
		struct clusterip_config *c = pde->data;

		sf->private = pde;

		clusterip_config_get(c);
	}

	return ret;
}

static int clusterip_proc_release(struct inode *inode, struct file *file)
{
	struct proc_dir_entry *pde = PDE(inode);
	struct clusterip_config *c = pde->data;
	int ret;

	ret = seq_release(inode, file);

	if (!ret)
		clusterip_config_put(c);

	return ret;
}

static ssize_t clusterip_proc_write(struct file *file, const char __user *input,
				size_t size, loff_t *ofs)
{
#define PROC_WRITELEN	10
	char buffer[PROC_WRITELEN+1];
	const struct proc_dir_entry *pde = PDE(file->f_path.dentry->d_inode);
	struct clusterip_config *c = pde->data;
	unsigned long nodenum;

	if (copy_from_user(buffer, input, PROC_WRITELEN))
		return -EFAULT;

	if (*buffer == '+') {
		nodenum = simple_strtoul(buffer+1, NULL, 10);
		if (clusterip_add_node(c, nodenum))
			return -ENOMEM;
	} else if (*buffer == '-') {
		nodenum = simple_strtoul(buffer+1, NULL,10);
		if (clusterip_del_node(c, nodenum))
			return -ENOENT;
	} else
		return -EIO;

	return size;
}

static const struct file_operations clusterip_proc_fops = {
	.owner	 = THIS_MODULE,
	.open	 = clusterip_proc_open,
	.read	 = seq_read,
	.write	 = clusterip_proc_write,
	.llseek	 = seq_lseek,
	.release = clusterip_proc_release,
};

#endif 

static int __init clusterip_tg_init(void)
{
	int ret;

	ret = xt_register_target(&clusterip_tg_reg);
	if (ret < 0)
		return ret;

	ret = nf_register_hook(&cip_arp_ops);
	if (ret < 0)
		goto cleanup_target;

#ifdef CONFIG_PROC_FS
	clusterip_procdir = proc_mkdir("ipt_CLUSTERIP", init_net.proc_net);
	if (!clusterip_procdir) {
		printk(KERN_ERR "CLUSTERIP: Unable to proc dir entry\n");
		ret = -ENOMEM;
		goto cleanup_hook;
	}
#endif 

	printk(KERN_NOTICE "ClusterIP Version %s loaded successfully\n",
		CLUSTERIP_VERSION);
	return 0;

#ifdef CONFIG_PROC_FS
cleanup_hook:
	nf_unregister_hook(&cip_arp_ops);
#endif 
cleanup_target:
	xt_unregister_target(&clusterip_tg_reg);
	return ret;
}

static void __exit clusterip_tg_exit(void)
{
	printk(KERN_NOTICE "ClusterIP Version %s unloading\n",
		CLUSTERIP_VERSION);
#ifdef CONFIG_PROC_FS
	remove_proc_entry(clusterip_procdir->name, clusterip_procdir->parent);
#endif
	nf_unregister_hook(&cip_arp_ops);
	xt_unregister_target(&clusterip_tg_reg);
}

module_init(clusterip_tg_init);
module_exit(clusterip_tg_exit);
