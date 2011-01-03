

#include <linux/module.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/random.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/net.h>
#include <net/ip.h>
#include <net/inetpeer.h>




DEFINE_SPINLOCK(inet_peer_idlock);

static struct kmem_cache *peer_cachep __read_mostly;

#define node_height(x) x->avl_height
static struct inet_peer peer_fake_node = {
	.avl_left	= &peer_fake_node,
	.avl_right	= &peer_fake_node,
	.avl_height	= 0
};
#define peer_avl_empty (&peer_fake_node)
static struct inet_peer *peer_root = peer_avl_empty;
static DEFINE_RWLOCK(peer_pool_lock);
#define PEER_MAXDEPTH 40 

static int peer_total;

int inet_peer_threshold __read_mostly = 65536 + 128;	
int inet_peer_minttl __read_mostly = 120 * HZ;	
int inet_peer_maxttl __read_mostly = 10 * 60 * HZ;	
int inet_peer_gc_mintime __read_mostly = 10 * HZ;
int inet_peer_gc_maxtime __read_mostly = 120 * HZ;

static LIST_HEAD(unused_peers);
static DEFINE_SPINLOCK(inet_peer_unused_lock);

static void peer_check_expire(unsigned long dummy);
static DEFINE_TIMER(peer_periodic_timer, peer_check_expire, 0, 0);



void __init inet_initpeers(void)
{
	struct sysinfo si;

	
	si_meminfo(&si);
	
	if (si.totalram <= (32768*1024)/PAGE_SIZE)
		inet_peer_threshold >>= 1; 
	if (si.totalram <= (16384*1024)/PAGE_SIZE)
		inet_peer_threshold >>= 1; 
	if (si.totalram <= (8192*1024)/PAGE_SIZE)
		inet_peer_threshold >>= 2; 

	peer_cachep = kmem_cache_create("inet_peer_cache",
			sizeof(struct inet_peer),
			0, SLAB_HWCACHE_ALIGN|SLAB_PANIC,
			NULL);

	
	peer_periodic_timer.expires = jiffies
		+ net_random() % inet_peer_gc_maxtime
		+ inet_peer_gc_maxtime;
	add_timer(&peer_periodic_timer);
}


static void unlink_from_unused(struct inet_peer *p)
{
	spin_lock_bh(&inet_peer_unused_lock);
	list_del_init(&p->unused);
	spin_unlock_bh(&inet_peer_unused_lock);
}


#define lookup(_daddr, _stack) 					\
({								\
	struct inet_peer *u, **v;				\
	if (_stack != NULL) {					\
		stackptr = _stack;				\
		*stackptr++ = &peer_root;			\
	}							\
	for (u = peer_root; u != peer_avl_empty; ) {		\
		if (_daddr == u->v4daddr)			\
			break;					\
		if ((__force __u32)_daddr < (__force __u32)u->v4daddr)	\
			v = &u->avl_left;			\
		else						\
			v = &u->avl_right;			\
		if (_stack != NULL)				\
			*stackptr++ = v;			\
		u = *v;						\
	}							\
	u;							\
})


#define lookup_rightempty(start)				\
({								\
	struct inet_peer *u, **v;				\
	*stackptr++ = &start->avl_left;				\
	v = &start->avl_left;					\
	for (u = *v; u->avl_right != peer_avl_empty; ) {	\
		v = &u->avl_right;				\
		*stackptr++ = v;				\
		u = *v;						\
	}							\
	u;							\
})


static void peer_avl_rebalance(struct inet_peer **stack[],
		struct inet_peer ***stackend)
{
	struct inet_peer **nodep, *node, *l, *r;
	int lh, rh;

	while (stackend > stack) {
		nodep = *--stackend;
		node = *nodep;
		l = node->avl_left;
		r = node->avl_right;
		lh = node_height(l);
		rh = node_height(r);
		if (lh > rh + 1) { 
			struct inet_peer *ll, *lr, *lrl, *lrr;
			int lrh;
			ll = l->avl_left;
			lr = l->avl_right;
			lrh = node_height(lr);
			if (lrh <= node_height(ll)) {	
				node->avl_left = lr;	
				node->avl_right = r;	
				node->avl_height = lrh + 1; 
				l->avl_left = ll;	
				l->avl_right = node;	
				l->avl_height = node->avl_height + 1;
				*nodep = l;
			} else { 
				lrl = lr->avl_left;	
				lrr = lr->avl_right;	
				node->avl_left = lrr;	
				node->avl_right = r;	
				node->avl_height = rh + 1; 
				l->avl_left = ll;	
				l->avl_right = lrl;	
				l->avl_height = rh + 1;	
				lr->avl_left = l;	
				lr->avl_right = node;	
				lr->avl_height = rh + 2;
				*nodep = lr;
			}
		} else if (rh > lh + 1) { 
			struct inet_peer *rr, *rl, *rlr, *rll;
			int rlh;
			rr = r->avl_right;
			rl = r->avl_left;
			rlh = node_height(rl);
			if (rlh <= node_height(rr)) {	
				node->avl_right = rl;	
				node->avl_left = l;	
				node->avl_height = rlh + 1; 
				r->avl_right = rr;	
				r->avl_left = node;	
				r->avl_height = node->avl_height + 1;
				*nodep = r;
			} else { 
				rlr = rl->avl_right;	
				rll = rl->avl_left;	
				node->avl_right = rll;	
				node->avl_left = l;	
				node->avl_height = lh + 1; 
				r->avl_right = rr;	
				r->avl_left = rlr;	
				r->avl_height = lh + 1;	
				rl->avl_right = r;	
				rl->avl_left = node;	
				rl->avl_height = lh + 2;
				*nodep = rl;
			}
		} else {
			node->avl_height = (lh > rh ? lh : rh) + 1;
		}
	}
}


#define link_to_pool(n)						\
do {								\
	n->avl_height = 1;					\
	n->avl_left = peer_avl_empty;				\
	n->avl_right = peer_avl_empty;				\
	**--stackptr = n;					\
	peer_avl_rebalance(stack, stackptr);			\
} while(0)


static void unlink_from_pool(struct inet_peer *p)
{
	int do_free;

	do_free = 0;

	write_lock_bh(&peer_pool_lock);
	
	if (atomic_read(&p->refcnt) == 1) {
		struct inet_peer **stack[PEER_MAXDEPTH];
		struct inet_peer ***stackptr, ***delp;
		if (lookup(p->v4daddr, stack) != p)
			BUG();
		delp = stackptr - 1; 
		if (p->avl_left == peer_avl_empty) {
			*delp[0] = p->avl_right;
			--stackptr;
		} else {
			
			struct inet_peer *t;
			t = lookup_rightempty(p);
			BUG_ON(*stackptr[-1] != t);
			**--stackptr = t->avl_left;
			
			*delp[0] = t;
			t->avl_left = p->avl_left;
			t->avl_right = p->avl_right;
			t->avl_height = p->avl_height;
			BUG_ON(delp[1] != &p->avl_left);
			delp[1] = &t->avl_left; 
		}
		peer_avl_rebalance(stack, stackptr);
		peer_total--;
		do_free = 1;
	}
	write_unlock_bh(&peer_pool_lock);

	if (do_free)
		kmem_cache_free(peer_cachep, p);
	else
		
		inet_putpeer(p);
}


static int cleanup_once(unsigned long ttl)
{
	struct inet_peer *p = NULL;

	
	spin_lock_bh(&inet_peer_unused_lock);
	if (!list_empty(&unused_peers)) {
		__u32 delta;

		p = list_first_entry(&unused_peers, struct inet_peer, unused);
		delta = (__u32)jiffies - p->dtime;

		if (delta < ttl) {
			
			spin_unlock_bh(&inet_peer_unused_lock);
			return -1;
		}

		list_del_init(&p->unused);

		
		atomic_inc(&p->refcnt);
	}
	spin_unlock_bh(&inet_peer_unused_lock);

	if (p == NULL)
		
		return -1;

	unlink_from_pool(p);
	return 0;
}


struct inet_peer *inet_getpeer(__be32 daddr, int create)
{
	struct inet_peer *p, *n;
	struct inet_peer **stack[PEER_MAXDEPTH], ***stackptr;

	
	read_lock_bh(&peer_pool_lock);
	p = lookup(daddr, NULL);
	if (p != peer_avl_empty)
		atomic_inc(&p->refcnt);
	read_unlock_bh(&peer_pool_lock);

	if (p != peer_avl_empty) {
		
		
		unlink_from_unused(p);
		return p;
	}

	if (!create)
		return NULL;

	
	n = kmem_cache_alloc(peer_cachep, GFP_ATOMIC);
	if (n == NULL)
		return NULL;
	n->v4daddr = daddr;
	atomic_set(&n->refcnt, 1);
	atomic_set(&n->rid, 0);
	n->ip_id_count = secure_ip_id(daddr);
	n->tcp_ts_stamp = 0;

	write_lock_bh(&peer_pool_lock);
	
	p = lookup(daddr, stack);
	if (p != peer_avl_empty)
		goto out_free;

	
	link_to_pool(n);
	INIT_LIST_HEAD(&n->unused);
	peer_total++;
	write_unlock_bh(&peer_pool_lock);

	if (peer_total >= inet_peer_threshold)
		
		cleanup_once(0);

	return n;

out_free:
	
	atomic_inc(&p->refcnt);
	write_unlock_bh(&peer_pool_lock);
	
	unlink_from_unused(p);
	
	kmem_cache_free(peer_cachep, n);
	return p;
}


static void peer_check_expire(unsigned long dummy)
{
	unsigned long now = jiffies;
	int ttl;

	if (peer_total >= inet_peer_threshold)
		ttl = inet_peer_minttl;
	else
		ttl = inet_peer_maxttl
				- (inet_peer_maxttl - inet_peer_minttl) / HZ *
					peer_total / inet_peer_threshold * HZ;
	while (!cleanup_once(ttl)) {
		if (jiffies != now)
			break;
	}

	
	if (peer_total >= inet_peer_threshold)
		peer_periodic_timer.expires = jiffies + inet_peer_gc_mintime;
	else
		peer_periodic_timer.expires = jiffies
			+ inet_peer_gc_maxtime
			- (inet_peer_gc_maxtime - inet_peer_gc_mintime) / HZ *
				peer_total / inet_peer_threshold * HZ;
	add_timer(&peer_periodic_timer);
}

void inet_putpeer(struct inet_peer *p)
{
	spin_lock_bh(&inet_peer_unused_lock);
	if (atomic_dec_and_test(&p->refcnt)) {
		list_add_tail(&p->unused, &unused_peers);
		p->dtime = (__u32)jiffies;
	}
	spin_unlock_bh(&inet_peer_unused_lock);
}
