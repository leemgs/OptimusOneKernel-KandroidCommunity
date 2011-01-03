

#ifndef _KEYS_KEYRING_TYPE_H
#define _KEYS_KEYRING_TYPE_H

#include <linux/key.h>
#include <linux/rcupdate.h>


struct keyring_list {
	struct rcu_head	rcu;		
	unsigned short	maxkeys;	
	unsigned short	nkeys;		
	unsigned short	delkey;		
	struct key	*keys[0];
};


#endif 
