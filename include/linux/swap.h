#ifndef _LINUX_SWAP_H
#define _LINUX_SWAP_H

#include <linux/spinlock.h>
#include <linux/linkage.h>
#include <linux/mmzone.h>
#include <linux/list.h>
#include <linux/memcontrol.h>
#include <linux/sched.h>
#include <linux/node.h>

#include <asm/atomic.h>
#include <asm/page.h>

struct notifier_block;

struct bio;

#define SWAP_FLAG_PREFER	0x8000	
#define SWAP_FLAG_PRIO_MASK	0x7fff
#define SWAP_FLAG_PRIO_SHIFT	0

static inline int current_is_kswapd(void)
{
	return current->flags & PF_KSWAPD;
}


#define MAX_SWAPFILES_SHIFT	5




#ifdef CONFIG_MIGRATION
#define SWP_MIGRATION_NUM 2
#define SWP_MIGRATION_READ	(MAX_SWAPFILES + SWP_HWPOISON_NUM)
#define SWP_MIGRATION_WRITE	(MAX_SWAPFILES + SWP_HWPOISON_NUM + 1)
#else
#define SWP_MIGRATION_NUM 0
#endif


#ifdef CONFIG_MEMORY_FAILURE
#define SWP_HWPOISON_NUM 1
#define SWP_HWPOISON		MAX_SWAPFILES
#else
#define SWP_HWPOISON_NUM 0
#endif

#define MAX_SWAPFILES \
	((1 << MAX_SWAPFILES_SHIFT) - SWP_MIGRATION_NUM - SWP_HWPOISON_NUM)


union swap_header {
	struct {
		char reserved[PAGE_SIZE - 10];
		char magic[10];			
	} magic;
	struct {
		char		bootbits[1024];	
		__u32		version;
		__u32		last_page;
		__u32		nr_badpages;
		unsigned char	sws_uuid[16];
		unsigned char	sws_volume[16];
		__u32		padding[117];
		__u32		badpages[1];
	} info;
};

 
typedef struct {
	unsigned long val;
} swp_entry_t;


struct reclaim_state {
	unsigned long reclaimed_slab;
};

#ifdef __KERNEL__

struct address_space;
struct sysinfo;
struct writeback_control;
struct zone;


struct swap_extent {
	struct list_head list;
	pgoff_t start_page;
	pgoff_t nr_pages;
	sector_t start_block;
};


#define __swapoffset(x) ((unsigned long)&((union swap_header *)0)->x)
#define MAX_SWAP_BADPAGES \
	((__swapoffset(magic.magic) - __swapoffset(info.badpages)) / sizeof(int))

enum {
	SWP_USED	= (1 << 0),	
	SWP_WRITEOK	= (1 << 1),	
	SWP_DISCARDABLE = (1 << 2),	
	SWP_DISCARDING	= (1 << 3),	
	SWP_SOLIDSTATE	= (1 << 4),	
					
	SWP_SCANNING	= (1 << 8),	
};

#define SWAP_CLUSTER_MAX 32

#define SWAP_MAP_MAX	0x7ffe
#define SWAP_MAP_BAD	0x7fff
#define SWAP_HAS_CACHE  0x8000		
#define SWAP_COUNT_MASK (~SWAP_HAS_CACHE)

struct swap_info_struct {
	unsigned long flags;
	int prio;			
	int next;			
	struct file *swap_file;
	struct block_device *bdev;
	struct list_head extent_list;
	struct swap_extent *curr_swap_extent;
	unsigned short *swap_map;
	unsigned int lowest_bit;
	unsigned int highest_bit;
	unsigned int lowest_alloc;	
	unsigned int highest_alloc;	
	unsigned int cluster_next;
	unsigned int cluster_nr;
	unsigned int pages;
	unsigned int max;
	unsigned int inuse_pages;
	unsigned int old_block_size;
};

struct swap_list_t {
	int head;	
	int next;	
};


#define vm_swap_full() (nr_swap_pages*2 < total_swap_pages)


extern unsigned long totalram_pages;
extern unsigned long totalreserve_pages;
extern unsigned int nr_free_buffer_pages(void);
extern unsigned int nr_free_pagecache_pages(void);


#define nr_free_pages() global_page_state(NR_FREE_PAGES)



extern void __lru_cache_add(struct page *, enum lru_list lru);
extern void lru_cache_add_lru(struct page *, enum lru_list lru);
extern void activate_page(struct page *);
extern void mark_page_accessed(struct page *);
extern void lru_add_drain(void);
extern int lru_add_drain_all(void);
extern void rotate_reclaimable_page(struct page *page);
extern void swap_setup(void);

extern void add_page_to_unevictable_list(struct page *page);


static inline void lru_cache_add_anon(struct page *page)
{
	__lru_cache_add(page, LRU_INACTIVE_ANON);
}

static inline void lru_cache_add_active_anon(struct page *page)
{
	__lru_cache_add(page, LRU_ACTIVE_ANON);
}

static inline void lru_cache_add_file(struct page *page)
{
	__lru_cache_add(page, LRU_INACTIVE_FILE);
}

static inline void lru_cache_add_active_file(struct page *page)
{
	__lru_cache_add(page, LRU_ACTIVE_FILE);
}


extern unsigned long try_to_free_pages(struct zonelist *zonelist, int order,
					gfp_t gfp_mask, nodemask_t *mask);
extern unsigned long try_to_free_mem_cgroup_pages(struct mem_cgroup *mem,
						  gfp_t gfp_mask, bool noswap,
						  unsigned int swappiness);
extern unsigned long mem_cgroup_shrink_node_zone(struct mem_cgroup *mem,
						gfp_t gfp_mask, bool noswap,
						unsigned int swappiness,
						struct zone *zone,
						int nid);
extern int __isolate_lru_page(struct page *page, int mode, int file);
extern unsigned long shrink_all_memory(unsigned long nr_pages);
extern int vm_swappiness;
extern int remove_mapping(struct address_space *mapping, struct page *page);
extern long vm_total_pages;

#ifdef CONFIG_NUMA
extern int zone_reclaim_mode;
extern int sysctl_min_unmapped_ratio;
extern int sysctl_min_slab_ratio;
extern int zone_reclaim(struct zone *, gfp_t, unsigned int);
#else
#define zone_reclaim_mode 0
static inline int zone_reclaim(struct zone *z, gfp_t mask, unsigned int order)
{
	return 0;
}
#endif

extern int page_evictable(struct page *page, struct vm_area_struct *vma);
extern void scan_mapping_unevictable_pages(struct address_space *);

extern unsigned long scan_unevictable_pages;
extern int scan_unevictable_handler(struct ctl_table *, int,
					void __user *, size_t *, loff_t *);
extern int scan_unevictable_register_node(struct node *node);
extern void scan_unevictable_unregister_node(struct node *node);

extern int kswapd_run(int nid);

#ifdef CONFIG_MMU

extern int shmem_unuse(swp_entry_t entry, struct page *page);
#endif 

extern void swap_unplug_io_fn(struct backing_dev_info *, struct page *);

#ifdef CONFIG_SWAP

extern int swap_readpage(struct page *);
extern int swap_writepage(struct page *page, struct writeback_control *wbc);
extern void end_swap_bio_read(struct bio *bio, int err);


extern struct address_space swapper_space;
#define total_swapcache_pages  swapper_space.nrpages
extern void show_swap_cache_info(void);
extern int add_to_swap(struct page *);
extern int add_to_swap_cache(struct page *, swp_entry_t, gfp_t);
extern void __delete_from_swap_cache(struct page *);
extern void delete_from_swap_cache(struct page *);
extern void free_page_and_swap_cache(struct page *);
extern void free_pages_and_swap_cache(struct page **, int);
extern struct page *lookup_swap_cache(swp_entry_t);
extern struct page *read_swap_cache_async(swp_entry_t, gfp_t,
			struct vm_area_struct *vma, unsigned long addr);
extern struct page *swapin_readahead(swp_entry_t, gfp_t,
			struct vm_area_struct *vma, unsigned long addr);


extern long nr_swap_pages;
extern long total_swap_pages;
extern void si_swapinfo(struct sysinfo *);
extern swp_entry_t get_swap_page(void);
extern swp_entry_t get_swap_page_of_type(int);
extern void swap_duplicate(swp_entry_t);
extern int swapcache_prepare(swp_entry_t);
extern int valid_swaphandles(swp_entry_t, unsigned long *);
extern void swap_free(swp_entry_t);
extern void swapcache_free(swp_entry_t, struct page *page);
extern int free_swap_and_cache(swp_entry_t);
extern int swap_type_of(dev_t, sector_t, struct block_device **);
extern unsigned int count_swap_pages(int, int);
extern sector_t map_swap_page(struct swap_info_struct *, pgoff_t);
extern sector_t swapdev_block(int, pgoff_t);
extern struct swap_info_struct *get_swap_info_struct(unsigned);
extern int reuse_swap_page(struct page *);
extern int try_to_free_swap(struct page *);
struct backing_dev_info;


extern struct mm_struct *swap_token_mm;
extern void grab_swap_token(struct mm_struct *);
extern void __put_swap_token(struct mm_struct *);

static inline int has_swap_token(struct mm_struct *mm)
{
	return (mm == swap_token_mm);
}

static inline void put_swap_token(struct mm_struct *mm)
{
	if (has_swap_token(mm))
		__put_swap_token(mm);
}

static inline void disable_swap_token(void)
{
	put_swap_token(swap_token_mm);
}

#ifdef CONFIG_CGROUP_MEM_RES_CTLR
extern void
mem_cgroup_uncharge_swapcache(struct page *page, swp_entry_t ent, bool swapout);
#else
static inline void
mem_cgroup_uncharge_swapcache(struct page *page, swp_entry_t ent, bool swapout)
{
}
#endif
#ifdef CONFIG_CGROUP_MEM_RES_CTLR_SWAP
extern void mem_cgroup_uncharge_swap(swp_entry_t ent);
#else
static inline void mem_cgroup_uncharge_swap(swp_entry_t ent)
{
}
#endif

#else 

#define nr_swap_pages				0L
#define total_swap_pages			0L
#define total_swapcache_pages			0UL

#define si_swapinfo(val) \
	do { (val)->freeswap = (val)->totalswap = 0; } while (0)

#define free_page_and_swap_cache(page) \
	page_cache_release(page)
#define free_pages_and_swap_cache(pages, nr) \
	release_pages((pages), (nr), 0);

static inline void show_swap_cache_info(void)
{
}

#define free_swap_and_cache(swp)	is_migration_entry(swp)
#define swapcache_prepare(swp)		is_migration_entry(swp)

static inline void swap_duplicate(swp_entry_t swp)
{
}

static inline void swap_free(swp_entry_t swp)
{
}

static inline void swapcache_free(swp_entry_t swp, struct page *page)
{
}

static inline struct page *swapin_readahead(swp_entry_t swp, gfp_t gfp_mask,
			struct vm_area_struct *vma, unsigned long addr)
{
	return NULL;
}

static inline int swap_writepage(struct page *p, struct writeback_control *wbc)
{
	return 0;
}

static inline struct page *lookup_swap_cache(swp_entry_t swp)
{
	return NULL;
}

static inline int add_to_swap(struct page *page)
{
	return 0;
}

static inline int add_to_swap_cache(struct page *page, swp_entry_t entry,
							gfp_t gfp_mask)
{
	return -1;
}

static inline void __delete_from_swap_cache(struct page *page)
{
}

static inline void delete_from_swap_cache(struct page *page)
{
}

#define reuse_swap_page(page)	(page_mapcount(page) == 1)

static inline int try_to_free_swap(struct page *page)
{
	return 0;
}

static inline swp_entry_t get_swap_page(void)
{
	swp_entry_t entry;
	entry.val = 0;
	return entry;
}


static inline void put_swap_token(struct mm_struct *mm)
{
}

static inline void grab_swap_token(struct mm_struct *mm)
{
}

static inline int has_swap_token(struct mm_struct *mm)
{
	return 0;
}

static inline void disable_swap_token(void)
{
}

static inline void
mem_cgroup_uncharge_swapcache(struct page *page, swp_entry_t ent)
{
}

#endif 
#endif 
#endif 
