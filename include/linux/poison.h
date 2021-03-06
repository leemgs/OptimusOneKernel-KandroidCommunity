#ifndef _LINUX_POISON_H
#define _LINUX_POISON_H



#define LIST_POISON1  ((void *) 0x00100100)
#define LIST_POISON2  ((void *) 0x00200200)



#define TIMER_ENTRY_STATIC	((void *) 0x74737461)


#define PAGE_POISON 0xaa



#define	RED_INACTIVE	0x09F911029D74E35BULL	
#define	RED_ACTIVE	0xD84156C5635688C0ULL	

#define SLUB_RED_INACTIVE	0xbb
#define SLUB_RED_ACTIVE		0xcc


#define	POISON_INUSE	0x5a	
#define POISON_FREE	0x6b	
#define	POISON_END	0xa5	


#define POISON_FREE_INITMEM	0xcc





#define JBD_POISON_FREE		0x5b
#define JBD2_POISON_FREE	0x5c


#define	POOL_POISON_FREED	0xa7	
#define	POOL_POISON_ALLOCATED	0xa9	


#define ATM_POISON_FREE		0x12
#define ATM_POISON		0xdeadbeef


#define NEIGHBOR_DEAD		0xdeadbeef
#define NETFILTER_LINK_POISON	0xdead57ac


#define MUTEX_DEBUG_INIT	0x11
#define MUTEX_DEBUG_FREE	0x22


#define FLEX_ARRAY_FREE	0x6c	


#define KEY_DESTROY		0xbd


#define OSS_POISON_FREE		0xAB

#endif
