#ifndef _LINUX_SLUB_DEF_H
#define _LINUX_SLUB_DEF_H


#include <linux/types.h>
#include <linux/gfp.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/kmemtrace.h>
#include <linux/kmemleak.h>

enum stat_item {
	ALLOC_FASTPATH,		
	ALLOC_SLOWPATH,		
	FREE_FASTPATH,		
	FREE_SLOWPATH,		
	FREE_FROZEN,		
	FREE_ADD_PARTIAL,	
	FREE_REMOVE_PARTIAL,	
	ALLOC_FROM_PARTIAL,	
	ALLOC_SLAB,		
	ALLOC_REFILL,		
	FREE_SLAB,		
	CPUSLAB_FLUSH,		
	DEACTIVATE_FULL,	
	DEACTIVATE_EMPTY,	
	DEACTIVATE_TO_HEAD,	
	DEACTIVATE_TO_TAIL,	
	DEACTIVATE_REMOTE_FREES,
	ORDER_FALLBACK,		
	NR_SLUB_STAT_ITEMS };

struct kmem_cache_cpu {
	void **freelist;	
	struct page *page;	
	int node;		
	unsigned int offset;	
	unsigned int objsize;	
#ifdef CONFIG_SLUB_STATS
	unsigned stat[NR_SLUB_STAT_ITEMS];
#endif
};

struct kmem_cache_node {
	spinlock_t list_lock;	
	unsigned long nr_partial;
	struct list_head partial;
#ifdef CONFIG_SLUB_DEBUG
	atomic_long_t nr_slabs;
	atomic_long_t total_objects;
	struct list_head full;
#endif
};


struct kmem_cache_order_objects {
	unsigned long x;
};


struct kmem_cache {
	
	unsigned long flags;
	int size;		
	int objsize;		
	int offset;		
	struct kmem_cache_order_objects oo;

	
	struct kmem_cache_node local_node;

	
	struct kmem_cache_order_objects max;
	struct kmem_cache_order_objects min;
	gfp_t allocflags;	
	int refcount;		
	void (*ctor)(void *);
	int inuse;		
	int align;		
	unsigned long min_partial;
	const char *name;	
	struct list_head list;	
#ifdef CONFIG_SLUB_DEBUG
	struct kobject kobj;	
#endif

#ifdef CONFIG_NUMA
	
	int remote_node_defrag_ratio;
	struct kmem_cache_node *node[MAX_NUMNODES];
#endif
#ifdef CONFIG_SMP
	struct kmem_cache_cpu *cpu_slab[NR_CPUS];
#else
	struct kmem_cache_cpu cpu_slab;
#endif
};


#if defined(ARCH_KMALLOC_MINALIGN) && ARCH_KMALLOC_MINALIGN > 8
#define KMALLOC_MIN_SIZE ARCH_KMALLOC_MINALIGN
#else
#define KMALLOC_MIN_SIZE 8
#endif

#define KMALLOC_SHIFT_LOW ilog2(KMALLOC_MIN_SIZE)


#define SLUB_MAX_SIZE (2 * PAGE_SIZE)

#define SLUB_PAGE_SHIFT (PAGE_SHIFT + 2)


extern struct kmem_cache kmalloc_caches[SLUB_PAGE_SHIFT];


static __always_inline int kmalloc_index(size_t size)
{
	if (!size)
		return 0;

	if (size <= KMALLOC_MIN_SIZE)
		return KMALLOC_SHIFT_LOW;

	if (KMALLOC_MIN_SIZE <= 32 && size > 64 && size <= 96)
		return 1;
	if (KMALLOC_MIN_SIZE <= 64 && size > 128 && size <= 192)
		return 2;
	if (size <=          8) return 3;
	if (size <=         16) return 4;
	if (size <=         32) return 5;
	if (size <=         64) return 6;
	if (size <=        128) return 7;
	if (size <=        256) return 8;
	if (size <=        512) return 9;
	if (size <=       1024) return 10;
	if (size <=   2 * 1024) return 11;
	if (size <=   4 * 1024) return 12;

	if (size <=   8 * 1024) return 13;
	if (size <=  16 * 1024) return 14;
	if (size <=  32 * 1024) return 15;
	if (size <=  64 * 1024) return 16;
	if (size <= 128 * 1024) return 17;
	if (size <= 256 * 1024) return 18;
	if (size <= 512 * 1024) return 19;
	if (size <= 1024 * 1024) return 20;
	if (size <=  2 * 1024 * 1024) return 21;
	return -1;


}


static __always_inline struct kmem_cache *kmalloc_slab(size_t size)
{
	int index = kmalloc_index(size);

	if (index == 0)
		return NULL;

	return &kmalloc_caches[index];
}

#ifdef CONFIG_ZONE_DMA
#define SLUB_DMA __GFP_DMA
#else

#define SLUB_DMA (__force gfp_t)0
#endif

void *kmem_cache_alloc(struct kmem_cache *, gfp_t);
void *__kmalloc(size_t size, gfp_t flags);

#ifdef CONFIG_KMEMTRACE
extern void *kmem_cache_alloc_notrace(struct kmem_cache *s, gfp_t gfpflags);
#else
static __always_inline void *
kmem_cache_alloc_notrace(struct kmem_cache *s, gfp_t gfpflags)
{
	return kmem_cache_alloc(s, gfpflags);
}
#endif

static __always_inline void *kmalloc_large(size_t size, gfp_t flags)
{
	unsigned int order = get_order(size);
	void *ret = (void *) __get_free_pages(flags | __GFP_COMP, order);

	kmemleak_alloc(ret, size, 1, flags);
	trace_kmalloc(_THIS_IP_, ret, size, PAGE_SIZE << order, flags);

	return ret;
}

static __always_inline void *kmalloc(size_t size, gfp_t flags)
{
	void *ret;

	if (__builtin_constant_p(size)) {
		if (size > SLUB_MAX_SIZE)
			return kmalloc_large(size, flags);

		if (!(flags & SLUB_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

			if (!s)
				return ZERO_SIZE_PTR;

			ret = kmem_cache_alloc_notrace(s, flags);

			trace_kmalloc(_THIS_IP_, ret, size, s->size, flags);

			return ret;
		}
	}
	return __kmalloc(size, flags);
}

#ifdef CONFIG_NUMA
void *__kmalloc_node(size_t size, gfp_t flags, int node);
void *kmem_cache_alloc_node(struct kmem_cache *, gfp_t flags, int node);

#ifdef CONFIG_KMEMTRACE
extern void *kmem_cache_alloc_node_notrace(struct kmem_cache *s,
					   gfp_t gfpflags,
					   int node);
#else
static __always_inline void *
kmem_cache_alloc_node_notrace(struct kmem_cache *s,
			      gfp_t gfpflags,
			      int node)
{
	return kmem_cache_alloc_node(s, gfpflags, node);
}
#endif

static __always_inline void *kmalloc_node(size_t size, gfp_t flags, int node)
{
	void *ret;

	if (__builtin_constant_p(size) &&
		size <= SLUB_MAX_SIZE && !(flags & SLUB_DMA)) {
			struct kmem_cache *s = kmalloc_slab(size);

		if (!s)
			return ZERO_SIZE_PTR;

		ret = kmem_cache_alloc_node_notrace(s, flags, node);

		trace_kmalloc_node(_THIS_IP_, ret,
				   size, s->size, flags, node);

		return ret;
	}
	return __kmalloc_node(size, flags, node);
}
#endif

#endif 
