

#ifndef _NF_CONNTRACK_EXPECT_H
#define _NF_CONNTRACK_EXPECT_H
#include <net/netfilter/nf_conntrack.h>

extern unsigned int nf_ct_expect_hsize;
extern unsigned int nf_ct_expect_max;

struct nf_conntrack_expect
{
	
	struct hlist_node lnode;

	
	struct hlist_node hnode;

	
	struct nf_conntrack_tuple tuple;
	struct nf_conntrack_tuple_mask mask;

	
	void (*expectfn)(struct nf_conn *new,
			 struct nf_conntrack_expect *this);

	
	struct nf_conntrack_helper *helper;

	
	struct nf_conn *master;

	
	struct timer_list timeout;

	
	atomic_t use;

	
	unsigned int flags;

	
	unsigned int class;

#ifdef CONFIG_NF_NAT_NEEDED
	__be32 saved_ip;
	
	union nf_conntrack_man_proto saved_proto;
	
	enum ip_conntrack_dir dir;
#endif

	struct rcu_head rcu;
};

static inline struct net *nf_ct_exp_net(struct nf_conntrack_expect *exp)
{
#ifdef CONFIG_NET_NS
	return exp->master->ct_net;	
#else
	return &init_net;
#endif
}

struct nf_conntrack_expect_policy
{
	unsigned int	max_expected;
	unsigned int	timeout;
};

#define NF_CT_EXPECT_CLASS_DEFAULT	0

#define NF_CT_EXPECT_PERMANENT	0x1
#define NF_CT_EXPECT_INACTIVE	0x2

int nf_conntrack_expect_init(struct net *net);
void nf_conntrack_expect_fini(struct net *net);

struct nf_conntrack_expect *
__nf_ct_expect_find(struct net *net, const struct nf_conntrack_tuple *tuple);

struct nf_conntrack_expect *
nf_ct_expect_find_get(struct net *net, const struct nf_conntrack_tuple *tuple);

struct nf_conntrack_expect *
nf_ct_find_expectation(struct net *net, const struct nf_conntrack_tuple *tuple);

void nf_ct_unlink_expect(struct nf_conntrack_expect *exp);
void nf_ct_remove_expectations(struct nf_conn *ct);
void nf_ct_unexpect_related(struct nf_conntrack_expect *exp);


struct nf_conntrack_expect *nf_ct_expect_alloc(struct nf_conn *me);
void nf_ct_expect_init(struct nf_conntrack_expect *, unsigned int, u_int8_t,
		       const union nf_inet_addr *,
		       const union nf_inet_addr *,
		       u_int8_t, const __be16 *, const __be16 *);
void nf_ct_expect_put(struct nf_conntrack_expect *exp);
int nf_ct_expect_related_report(struct nf_conntrack_expect *expect, 
				u32 pid, int report);
static inline int nf_ct_expect_related(struct nf_conntrack_expect *expect)
{
	return nf_ct_expect_related_report(expect, 0, 0);
}

#endif 

