
 

 
 
#ifndef _ATMCLIP_H
#define _ATMCLIP_H

#include <linux/netdevice.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <linux/atmarp.h>
#include <linux/spinlock.h>
#include <net/neighbour.h>


#define CLIP_VCC(vcc) ((struct clip_vcc *) ((vcc)->user_back))
#define NEIGH2ENTRY(neigh) ((struct atmarp_entry *) (neigh)->primary_key)

struct sk_buff;

struct clip_vcc {
	struct atm_vcc	*vcc;		
	struct atmarp_entry *entry;	
	int		xoff;		
	unsigned char	encap;		
	unsigned long	last_use;	
	unsigned long	idle_timeout;	
	void (*old_push)(struct atm_vcc *vcc,struct sk_buff *skb);
					
	void (*old_pop)(struct atm_vcc *vcc,struct sk_buff *skb);
					
	struct clip_vcc	*next;		
};


struct atmarp_entry {
	__be32		ip;		
	struct clip_vcc	*vccs;		
	unsigned long	expires;	
	struct neighbour *neigh;	
};


#define PRIV(dev) ((struct clip_priv *) netdev_priv(dev))


struct clip_priv {
	int number;			
	spinlock_t xoff_lock;		
	struct net_device *next;	
};


#ifdef __KERNEL__
extern struct neigh_table *clip_tbl_hook;
#endif

#endif
