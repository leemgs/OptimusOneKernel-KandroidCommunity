

#ifndef _NET_INETPEER_H
#define _NET_INETPEER_H

#include <linux/types.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/spinlock.h>
#include <asm/atomic.h>

struct inet_peer
{
	
	struct inet_peer	*avl_left, *avl_right;
	__be32			v4daddr;	
	__u16			avl_height;
	__u16			ip_id_count;	
	struct list_head	unused;
	__u32			dtime;		
	atomic_t		refcnt;
	atomic_t		rid;		
	__u32			tcp_ts;
	unsigned long		tcp_ts_stamp;
};

void			inet_initpeers(void) __init;


struct inet_peer	*inet_getpeer(__be32 daddr, int create);


extern void inet_putpeer(struct inet_peer *p);

extern spinlock_t inet_peer_idlock;

static inline __u16	inet_getid(struct inet_peer *p, int more)
{
	__u16 id;

	spin_lock_bh(&inet_peer_idlock);
	id = p->ip_id_count;
	p->ip_id_count += 1 + more;
	spin_unlock_bh(&inet_peer_idlock);
	return id;
}

#endif 
