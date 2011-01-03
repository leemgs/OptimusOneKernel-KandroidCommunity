

#ifndef _NF_CONNTRACK_HELPER_H
#define _NF_CONNTRACK_HELPER_H
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_extend.h>

struct module;

#define NF_CT_HELPER_NAME_LEN	16

struct nf_conntrack_helper
{
	struct hlist_node hnode;	

	const char *name;		
	struct module *me;		
	const struct nf_conntrack_expect_policy *expect_policy;

	
	struct nf_conntrack_tuple tuple;

	
	int (*help)(struct sk_buff *skb,
		    unsigned int protoff,
		    struct nf_conn *ct,
		    enum ip_conntrack_info conntrackinfo);

	void (*destroy)(struct nf_conn *ct);

	int (*to_nlattr)(struct sk_buff *skb, const struct nf_conn *ct);
	unsigned int expect_class_max;
};

extern struct nf_conntrack_helper *
__nf_conntrack_helper_find_byname(const char *name);

extern int nf_conntrack_helper_register(struct nf_conntrack_helper *);
extern void nf_conntrack_helper_unregister(struct nf_conntrack_helper *);

extern struct nf_conn_help *nf_ct_helper_ext_add(struct nf_conn *ct, gfp_t gfp);

extern int __nf_ct_try_assign_helper(struct nf_conn *ct, gfp_t flags);

extern void nf_ct_helper_destroy(struct nf_conn *ct);

static inline struct nf_conn_help *nfct_help(const struct nf_conn *ct)
{
	return nf_ct_ext_find(ct, NF_CT_EXT_HELPER);
}

extern int nf_conntrack_helper_init(void);
extern void nf_conntrack_helper_fini(void);

#endif 
