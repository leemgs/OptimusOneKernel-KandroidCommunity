

#include <linux/stddef.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/interrupt.h>
#include <linux/pagemap.h>
#include <linux/jiffies.h>
#include <linux/bootmem.h>
#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/kmemcheck.h>
#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/pagevec.h>
#include <linux/blkdev.h>
#include <linux/slab.h>
#include <linux/oom.h>
#include <linux/notifier.h>
#include <linux/topology.h>
#include <linux/sysctl.h>
#include <linux/cpu.h>
#include <linux/cpuset.h>
#include <linux/memory_hotplug.h>
#include <linux/nodemask.h>
#include <linux/vmalloc.h>
#include <linux/mempolicy.h>
#include <linux/stop_machine.h>
#include <linux/sort.h>
#include <linux/pfn.h>
#include <linux/backing-dev.h>
#include <linux/fault-inject.h>
#include <linux/page-isolation.h>
#include <linux/page_cgroup.h>
#include <linux/debugobjects.h>
#include <linux/kmemleak.h>
#include <trace/events/kmem.h>

#include <asm/tlbflush.h>
#include <asm/div64.h>
#include "internal.h"


nodemask_t node_states[NR_NODE_STATES] __read_mostly = {
	[N_POSSIBLE] = NODE_MASK_ALL,
	[N_ONLINE] = { { [0] = 1UL } },
#ifndef CONFIG_NUMA
	[N_NORMAL_MEMORY] = { { [0] = 1UL } },
#ifdef CONFIG_HIGHMEM
	[N_HIGH_MEMORY] = { { [0] = 1UL } },
#endif
	[N_CPU] = { { [0] = 1UL } },
#endif	
};
EXPORT_SYMBOL(node_states);

unsigned long totalram_pages __read_mostly;
unsigned long totalreserve_pages __read_mostly;
int percpu_pagelist_fraction;
gfp_t gfp_allowed_mask __read_mostly = GFP_BOOT_MASK;

#ifdef CONFIG_HUGETLB_PAGE_SIZE_VARIABLE
int pageblock_order __read_mostly;
#endif

static void __free_pages_ok(struct page *page, unsigned int order);


int sysctl_lowmem_reserve_ratio[MAX_NR_ZONES-1] = {
#ifdef CONFIG_ZONE_DMA
	 256,
#endif
#ifdef CONFIG_ZONE_DMA32
	 256,
#endif
#ifdef CONFIG_HIGHMEM
	 32,
#endif
	 32,
};

EXPORT_SYMBOL(totalram_pages);

static char * const zone_names[MAX_NR_ZONES] = {
#ifdef CONFIG_ZONE_DMA
	 "DMA",
#endif
#ifdef CONFIG_ZONE_DMA32
	 "DMA32",
#endif
	 "Normal",
#ifdef CONFIG_HIGHMEM
	 "HighMem",
#endif
	 "Movable",
};

int min_free_kbytes = 1024;
int min_free_order_shift = 1;

static unsigned long __meminitdata nr_kernel_pages;
static unsigned long __meminitdata nr_all_pages;
static unsigned long __meminitdata dma_reserve;

#ifdef CONFIG_ARCH_POPULATES_NODE_MAP
  
  #ifdef CONFIG_MAX_ACTIVE_REGIONS
    
    #define MAX_ACTIVE_REGIONS CONFIG_MAX_ACTIVE_REGIONS
  #else
    #if MAX_NUMNODES >= 32
      
      #define MAX_ACTIVE_REGIONS (MAX_NUMNODES*50)
    #else
      
      #define MAX_ACTIVE_REGIONS 256
    #endif
  #endif

  static struct node_active_region __meminitdata early_node_map[MAX_ACTIVE_REGIONS];
  static int __meminitdata nr_nodemap_entries;
  static unsigned long __meminitdata arch_zone_lowest_possible_pfn[MAX_NR_ZONES];
  static unsigned long __meminitdata arch_zone_highest_possible_pfn[MAX_NR_ZONES];
  static unsigned long __initdata required_kernelcore;
  static unsigned long __initdata required_movablecore;
  static unsigned long __meminitdata zone_movable_pfn[MAX_NUMNODES];

  
  int movable_zone;
  EXPORT_SYMBOL(movable_zone);
#endif 

#if MAX_NUMNODES > 1
int nr_node_ids __read_mostly = MAX_NUMNODES;
int nr_online_nodes __read_mostly = 1;
EXPORT_SYMBOL(nr_node_ids);
EXPORT_SYMBOL(nr_online_nodes);
#endif

int page_group_by_mobility_disabled __read_mostly;

static void set_pageblock_migratetype(struct page *page, int migratetype)
{

	if (unlikely(page_group_by_mobility_disabled))
		migratetype = MIGRATE_UNMOVABLE;

	set_pageblock_flags_group(page, (unsigned long)migratetype,
					PB_migrate, PB_migrate_end);
}

bool oom_killer_disabled __read_mostly;

#ifdef CONFIG_DEBUG_VM
static int page_outside_zone_boundaries(struct zone *zone, struct page *page)
{
	int ret = 0;
	unsigned seq;
	unsigned long pfn = page_to_pfn(page);

	do {
		seq = zone_span_seqbegin(zone);
		if (pfn >= zone->zone_start_pfn + zone->spanned_pages)
			ret = 1;
		else if (pfn < zone->zone_start_pfn)
			ret = 1;
	} while (zone_span_seqretry(zone, seq));

	return ret;
}

static int page_is_consistent(struct zone *zone, struct page *page)
{
	if (!pfn_valid_within(page_to_pfn(page)))
		return 0;
	if (zone != page_zone(page))
		return 0;

	return 1;
}

static int bad_range(struct zone *zone, struct page *page)
{
	if (page_outside_zone_boundaries(zone, page))
		return 1;
	if (!page_is_consistent(zone, page))
		return 1;

	return 0;
}
#else
static inline int bad_range(struct zone *zone, struct page *page)
{
	return 0;
}
#endif

static void bad_page(struct page *page)
{
	static unsigned long resume;
	static unsigned long nr_shown;
	static unsigned long nr_unshown;

	
	if (PageHWPoison(page)) {
		__ClearPageBuddy(page);
		return;
	}

	
	if (nr_shown == 60) {
		if (time_before(jiffies, resume)) {
			nr_unshown++;
			goto out;
		}
		if (nr_unshown) {
			printk(KERN_ALERT
			      "BUG: Bad page state: %lu messages suppressed\n",
				nr_unshown);
			nr_unshown = 0;
		}
		nr_shown = 0;
	}
	if (nr_shown++ == 0)
		resume = jiffies + 60 * HZ;

	printk(KERN_ALERT "BUG: Bad page state in process %s  pfn:%05lx\n",
		current->comm, page_to_pfn(page));
	printk(KERN_ALERT
		"page:%p flags:%p count:%d mapcount:%d mapping:%p index:%lx\n",
		page, (void *)page->flags, page_count(page),
		page_mapcount(page), page->mapping, page->index);

	dump_stack();
out:
	
	__ClearPageBuddy(page);
	add_taint(TAINT_BAD_PAGE);
}



static void free_compound_page(struct page *page)
{
	__free_pages_ok(page, compound_order(page));
}

void prep_compound_page(struct page *page, unsigned long order)
{
	int i;
	int nr_pages = 1 << order;

	set_compound_page_dtor(page, free_compound_page);
	set_compound_order(page, order);
	__SetPageHead(page);
	for (i = 1; i < nr_pages; i++) {
		struct page *p = page + i;

		__SetPageTail(p);
		p->first_page = page;
	}
}

static int destroy_compound_page(struct page *page, unsigned long order)
{
	int i;
	int nr_pages = 1 << order;
	int bad = 0;

	if (unlikely(compound_order(page) != order) ||
	    unlikely(!PageHead(page))) {
		bad_page(page);
		bad++;
	}

	__ClearPageHead(page);

	for (i = 1; i < nr_pages; i++) {
		struct page *p = page + i;

		if (unlikely(!PageTail(p) || (p->first_page != page))) {
			bad_page(page);
			bad++;
		}
		__ClearPageTail(p);
	}

	return bad;
}

static inline void prep_zero_page(struct page *page, int order, gfp_t gfp_flags)
{
	int i;

	
	VM_BUG_ON((gfp_flags & __GFP_HIGHMEM) && in_interrupt());
	for (i = 0; i < (1 << order); i++)
		clear_highpage(page + i);
}

static inline void set_page_order(struct page *page, int order)
{
	set_page_private(page, order);
	__SetPageBuddy(page);
}

static inline void rmv_page_order(struct page *page)
{
	__ClearPageBuddy(page);
	set_page_private(page, 0);
}


static inline struct page *
__page_find_buddy(struct page *page, unsigned long page_idx, unsigned int order)
{
	unsigned long buddy_idx = page_idx ^ (1 << order);

	return page + (buddy_idx - page_idx);
}

static inline unsigned long
__find_combined_index(unsigned long page_idx, unsigned int order)
{
	return (page_idx & ~(1 << order));
}


static inline int page_is_buddy(struct page *page, struct page *buddy,
								int order)
{
	if (!pfn_valid_within(page_to_pfn(buddy)))
		return 0;

	if (page_zone_id(page) != page_zone_id(buddy))
		return 0;

	if (PageBuddy(buddy) && page_order(buddy) == order) {
		VM_BUG_ON(page_count(buddy) != 0);
		return 1;
	}
	return 0;
}



static inline void __free_one_page(struct page *page,
		struct zone *zone, unsigned int order,
		int migratetype)
{
	unsigned long page_idx;

	if (unlikely(PageCompound(page)))
		if (unlikely(destroy_compound_page(page, order)))
			return;

	VM_BUG_ON(migratetype == -1);

	page_idx = page_to_pfn(page) & ((1 << MAX_ORDER) - 1);

	VM_BUG_ON(page_idx & ((1 << order) - 1));
	VM_BUG_ON(bad_range(zone, page));

	while (order < MAX_ORDER-1) {
		unsigned long combined_idx;
		struct page *buddy;

		buddy = __page_find_buddy(page, page_idx, order);
		if (!page_is_buddy(page, buddy, order))
			break;

		
		list_del(&buddy->lru);
		zone->free_area[order].nr_free--;
		rmv_page_order(buddy);
		combined_idx = __find_combined_index(page_idx, order);
		page = page + (combined_idx - page_idx);
		page_idx = combined_idx;
		order++;
	}
	set_page_order(page, order);
	list_add(&page->lru,
		&zone->free_area[order].free_list[migratetype]);
	zone->free_area[order].nr_free++;
}

#ifdef CONFIG_HAVE_MLOCKED_PAGE_BIT

static inline void free_page_mlock(struct page *page)
{
	__dec_zone_page_state(page, NR_MLOCK);
	__count_vm_event(UNEVICTABLE_MLOCKFREED);
}
#else
static void free_page_mlock(struct page *page) { }
#endif

static inline int free_pages_check(struct page *page)
{
	if (unlikely(page_mapcount(page) |
		(page->mapping != NULL)  |
		(atomic_read(&page->_count) != 0) |
		(page->flags & PAGE_FLAGS_CHECK_AT_FREE))) {
		bad_page(page);
		return 1;
	}
	if (page->flags & PAGE_FLAGS_CHECK_AT_PREP)
		page->flags &= ~PAGE_FLAGS_CHECK_AT_PREP;
	return 0;
}


static void free_pcppages_bulk(struct zone *zone, int count,
					struct per_cpu_pages *pcp)
{
	int migratetype = 0;
	int batch_free = 0;

	spin_lock(&zone->lock);
	zone_clear_flag(zone, ZONE_ALL_UNRECLAIMABLE);
	zone->pages_scanned = 0;

	__mod_zone_page_state(zone, NR_FREE_PAGES, count);
	while (count) {
		struct page *page;
		struct list_head *list;

		
		do {
			batch_free++;
			if (++migratetype == MIGRATE_PCPTYPES)
				migratetype = 0;
			list = &pcp->lists[migratetype];
		} while (list_empty(list));

		do {
			page = list_entry(list->prev, struct page, lru);
			
			list_del(&page->lru);
			
			__free_one_page(page, zone, 0, page_private(page));
			trace_mm_page_pcpu_drain(page, 0, page_private(page));
		} while (--count && --batch_free && !list_empty(list));
	}
	spin_unlock(&zone->lock);
}

static void free_one_page(struct zone *zone, struct page *page, int order,
				int migratetype)
{
	spin_lock(&zone->lock);
	zone_clear_flag(zone, ZONE_ALL_UNRECLAIMABLE);
	zone->pages_scanned = 0;

	__mod_zone_page_state(zone, NR_FREE_PAGES, 1 << order);
	__free_one_page(page, zone, order, migratetype);
	spin_unlock(&zone->lock);
}

static void __free_pages_ok(struct page *page, unsigned int order)
{
	unsigned long flags;
	int i;
	int bad = 0;
	int wasMlocked = __TestClearPageMlocked(page);

	kmemcheck_free_shadow(page, order);

	for (i = 0 ; i < (1 << order) ; ++i)
		bad += free_pages_check(page + i);
	if (bad)
		return;

	if (!PageHighMem(page)) {
		debug_check_no_locks_freed(page_address(page),PAGE_SIZE<<order);
		debug_check_no_obj_freed(page_address(page),
					   PAGE_SIZE << order);
	}
	arch_free_page(page, order);
	kernel_map_pages(page, 1 << order, 0);

	local_irq_save(flags);
	if (unlikely(wasMlocked))
		free_page_mlock(page);
	__count_vm_events(PGFREE, 1 << order);
	free_one_page(page_zone(page), page, order,
					get_pageblock_migratetype(page));
	local_irq_restore(flags);
}


void __meminit __free_pages_bootmem(struct page *page, unsigned int order)
{
	if (order == 0) {
		__ClearPageReserved(page);
		set_page_count(page, 0);
		set_page_refcounted(page);
		__free_page(page);
	} else {
		int loop;

		prefetchw(page);
		for (loop = 0; loop < BITS_PER_LONG; loop++) {
			struct page *p = &page[loop];

			if (loop + 1 < BITS_PER_LONG)
				prefetchw(p + 1);
			__ClearPageReserved(p);
			set_page_count(p, 0);
		}

		set_page_refcounted(page);
		__free_pages(page, order);
	}
}



static inline void expand(struct zone *zone, struct page *page,
	int low, int high, struct free_area *area,
	int migratetype)
{
	unsigned long size = 1 << high;

	while (high > low) {
		area--;
		high--;
		size >>= 1;
		VM_BUG_ON(bad_range(zone, &page[size]));
		list_add(&page[size].lru, &area->free_list[migratetype]);
		area->nr_free++;
		set_page_order(&page[size], high);
	}
}


static inline int check_new_page(struct page *page)
{
	if (unlikely(page_mapcount(page) |
		(page->mapping != NULL)  |
		(atomic_read(&page->_count) != 0)  |
		(page->flags & PAGE_FLAGS_CHECK_AT_PREP))) {
		bad_page(page);
		return 1;
	}
	return 0;
}

static int prep_new_page(struct page *page, int order, gfp_t gfp_flags)
{
	int i;

	for (i = 0; i < (1 << order); i++) {
		struct page *p = page + i;
		if (unlikely(check_new_page(p)))
			return 1;
	}

	set_page_private(page, 0);
	set_page_refcounted(page);

	arch_alloc_page(page, order);
	kernel_map_pages(page, 1 << order, 1);

	if (gfp_flags & __GFP_ZERO)
		prep_zero_page(page, order, gfp_flags);

	if (order && (gfp_flags & __GFP_COMP))
		prep_compound_page(page, order);

	return 0;
}


static inline
struct page *__rmqueue_smallest(struct zone *zone, unsigned int order,
						int migratetype)
{
	unsigned int current_order;
	struct free_area * area;
	struct page *page;

	
	for (current_order = order; current_order < MAX_ORDER; ++current_order) {
		area = &(zone->free_area[current_order]);
		if (list_empty(&area->free_list[migratetype]))
			continue;

		page = list_entry(area->free_list[migratetype].next,
							struct page, lru);
		list_del(&page->lru);
		rmv_page_order(page);
		area->nr_free--;
		expand(zone, page, order, current_order, area, migratetype);
		return page;
	}

	return NULL;
}



static int fallbacks[MIGRATE_TYPES][MIGRATE_TYPES-1] = {
	[MIGRATE_UNMOVABLE]   = { MIGRATE_RECLAIMABLE, MIGRATE_MOVABLE,   MIGRATE_RESERVE },
	[MIGRATE_RECLAIMABLE] = { MIGRATE_UNMOVABLE,   MIGRATE_MOVABLE,   MIGRATE_RESERVE },
	[MIGRATE_MOVABLE]     = { MIGRATE_RECLAIMABLE, MIGRATE_UNMOVABLE, MIGRATE_RESERVE },
	[MIGRATE_RESERVE]     = { MIGRATE_RESERVE,     MIGRATE_RESERVE,   MIGRATE_RESERVE }, 
};


static int move_freepages(struct zone *zone,
			  struct page *start_page, struct page *end_page,
			  int migratetype)
{
	struct page *page;
	unsigned long order;
	int pages_moved = 0;

#ifndef CONFIG_HOLES_IN_ZONE
	
	BUG_ON(page_zone(start_page) != page_zone(end_page));
#endif

	for (page = start_page; page <= end_page;) {
		
		VM_BUG_ON(page_to_nid(page) != zone_to_nid(zone));

		if (!pfn_valid_within(page_to_pfn(page))) {
			page++;
			continue;
		}

		if (!PageBuddy(page)) {
			page++;
			continue;
		}

		order = page_order(page);
		list_del(&page->lru);
		list_add(&page->lru,
			&zone->free_area[order].free_list[migratetype]);
		page += 1 << order;
		pages_moved += 1 << order;
	}

	return pages_moved;
}

static int move_freepages_block(struct zone *zone, struct page *page,
				int migratetype)
{
	unsigned long start_pfn, end_pfn;
	struct page *start_page, *end_page;

	start_pfn = page_to_pfn(page);
	start_pfn = start_pfn & ~(pageblock_nr_pages-1);
	start_page = pfn_to_page(start_pfn);
	end_page = start_page + pageblock_nr_pages - 1;
	end_pfn = start_pfn + pageblock_nr_pages - 1;

	
	if (start_pfn < zone->zone_start_pfn)
		start_page = page;
	if (end_pfn >= zone->zone_start_pfn + zone->spanned_pages)
		return 0;

	return move_freepages(zone, start_page, end_page, migratetype);
}

static void change_pageblock_range(struct page *pageblock_page,
					int start_order, int migratetype)
{
	int nr_pageblocks = 1 << (start_order - pageblock_order);

	while (nr_pageblocks--) {
		set_pageblock_migratetype(pageblock_page, migratetype);
		pageblock_page += pageblock_nr_pages;
	}
}


static inline struct page *
__rmqueue_fallback(struct zone *zone, int order, int start_migratetype)
{
	struct free_area * area;
	int current_order;
	struct page *page;
	int migratetype, i;

	
	for (current_order = MAX_ORDER-1; current_order >= order;
						--current_order) {
		for (i = 0; i < MIGRATE_TYPES - 1; i++) {
			migratetype = fallbacks[start_migratetype][i];

			
			if (migratetype == MIGRATE_RESERVE)
				continue;

			area = &(zone->free_area[current_order]);
			if (list_empty(&area->free_list[migratetype]))
				continue;

			page = list_entry(area->free_list[migratetype].next,
					struct page, lru);
			area->nr_free--;

			
			if (unlikely(current_order >= (pageblock_order >> 1)) ||
					start_migratetype == MIGRATE_RECLAIMABLE ||
					page_group_by_mobility_disabled) {
				unsigned long pages;
				pages = move_freepages_block(zone, page,
								start_migratetype);

				
				if (pages >= (1 << (pageblock_order-1)) ||
						page_group_by_mobility_disabled)
					set_pageblock_migratetype(page,
								start_migratetype);

				migratetype = start_migratetype;
			}

			
			list_del(&page->lru);
			rmv_page_order(page);

			
			if (current_order >= pageblock_order)
				change_pageblock_range(page, current_order,
							start_migratetype);

			expand(zone, page, order, current_order, area, migratetype);

			trace_mm_page_alloc_extfrag(page, order, current_order,
				start_migratetype, migratetype);

			return page;
		}
	}

	return NULL;
}


static struct page *__rmqueue(struct zone *zone, unsigned int order,
						int migratetype)
{
	struct page *page;

retry_reserve:
	page = __rmqueue_smallest(zone, order, migratetype);

	if (unlikely(!page) && migratetype != MIGRATE_RESERVE) {
		page = __rmqueue_fallback(zone, order, migratetype);

		
		if (!page) {
			migratetype = MIGRATE_RESERVE;
			goto retry_reserve;
		}
	}

	trace_mm_page_alloc_zone_locked(page, order, migratetype);
	return page;
}


static int rmqueue_bulk(struct zone *zone, unsigned int order, 
			unsigned long count, struct list_head *list,
			int migratetype, int cold)
{
	int i;
	
	spin_lock(&zone->lock);
	for (i = 0; i < count; ++i) {
		struct page *page = __rmqueue(zone, order, migratetype);
		if (unlikely(page == NULL))
			break;

		
		if (likely(cold == 0))
			list_add(&page->lru, list);
		else
			list_add_tail(&page->lru, list);
		set_page_private(page, migratetype);
		list = &page->lru;
	}
	__mod_zone_page_state(zone, NR_FREE_PAGES, -(i << order));
	spin_unlock(&zone->lock);
	return i;
}

#ifdef CONFIG_NUMA

void drain_zone_pages(struct zone *zone, struct per_cpu_pages *pcp)
{
	unsigned long flags;
	int to_drain;

	local_irq_save(flags);
	if (pcp->count >= pcp->batch)
		to_drain = pcp->batch;
	else
		to_drain = pcp->count;
	free_pcppages_bulk(zone, to_drain, pcp);
	pcp->count -= to_drain;
	local_irq_restore(flags);
}
#endif


static void drain_pages(unsigned int cpu)
{
	unsigned long flags;
	struct zone *zone;

	for_each_populated_zone(zone) {
		struct per_cpu_pageset *pset;
		struct per_cpu_pages *pcp;

		pset = zone_pcp(zone, cpu);

		pcp = &pset->pcp;
		local_irq_save(flags);
		free_pcppages_bulk(zone, pcp->count, pcp);
		pcp->count = 0;
		local_irq_restore(flags);
	}
}


void drain_local_pages(void *arg)
{
	drain_pages(smp_processor_id());
}


void drain_all_pages(void)
{
	on_each_cpu(drain_local_pages, NULL, 1);
}

#ifdef CONFIG_HIBERNATION

void mark_free_pages(struct zone *zone)
{
	unsigned long pfn, max_zone_pfn;
	unsigned long flags;
	int order, t;
	struct list_head *curr;

	if (!zone->spanned_pages)
		return;

	spin_lock_irqsave(&zone->lock, flags);

	max_zone_pfn = zone->zone_start_pfn + zone->spanned_pages;
	for (pfn = zone->zone_start_pfn; pfn < max_zone_pfn; pfn++)
		if (pfn_valid(pfn)) {
			struct page *page = pfn_to_page(pfn);

			if (!swsusp_page_is_forbidden(page))
				swsusp_unset_page_free(page);
		}

	for_each_migratetype_order(order, t) {
		list_for_each(curr, &zone->free_area[order].free_list[t]) {
			unsigned long i;

			pfn = page_to_pfn(list_entry(curr, struct page, lru));
			for (i = 0; i < (1UL << order); i++)
				swsusp_set_page_free(pfn_to_page(pfn + i));
		}
	}
	spin_unlock_irqrestore(&zone->lock, flags);
}
#endif 


static void free_hot_cold_page(struct page *page, int cold)
{
	struct zone *zone = page_zone(page);
	struct per_cpu_pages *pcp;
	unsigned long flags;
	int migratetype;
	int wasMlocked = __TestClearPageMlocked(page);

	kmemcheck_free_shadow(page, 0);

	if (PageAnon(page))
		page->mapping = NULL;
	if (free_pages_check(page))
		return;

	if (!PageHighMem(page)) {
		debug_check_no_locks_freed(page_address(page), PAGE_SIZE);
		debug_check_no_obj_freed(page_address(page), PAGE_SIZE);
	}
	arch_free_page(page, 0);
	kernel_map_pages(page, 1, 0);

	pcp = &zone_pcp(zone, get_cpu())->pcp;
	migratetype = get_pageblock_migratetype(page);
	set_page_private(page, migratetype);
	local_irq_save(flags);
	if (unlikely(wasMlocked))
		free_page_mlock(page);
	__count_vm_event(PGFREE);

	
	if (migratetype >= MIGRATE_PCPTYPES) {
		if (unlikely(migratetype == MIGRATE_ISOLATE)) {
			free_one_page(zone, page, 0, migratetype);
			goto out;
		}
		migratetype = MIGRATE_MOVABLE;
	}

	if (cold)
		list_add_tail(&page->lru, &pcp->lists[migratetype]);
	else
		list_add(&page->lru, &pcp->lists[migratetype]);
	pcp->count++;
	if (pcp->count >= pcp->high) {
		free_pcppages_bulk(zone, pcp->batch, pcp);
		pcp->count -= pcp->batch;
	}

out:
	local_irq_restore(flags);
	put_cpu();
}

void free_hot_page(struct page *page)
{
	trace_mm_page_free_direct(page, 0);
	free_hot_cold_page(page, 0);
}
	

void split_page(struct page *page, unsigned int order)
{
	int i;

	VM_BUG_ON(PageCompound(page));
	VM_BUG_ON(!page_count(page));

#ifdef CONFIG_KMEMCHECK
	
	if (kmemcheck_page_is_tracked(page))
		split_page(virt_to_page(page[0].shadow), order);
#endif

	for (i = 1; i < (1 << order); i++)
		set_page_refcounted(page + i);
}


static inline
struct page *buffered_rmqueue(struct zone *preferred_zone,
			struct zone *zone, int order, gfp_t gfp_flags,
			int migratetype)
{
	unsigned long flags;
	struct page *page;
	int cold = !!(gfp_flags & __GFP_COLD);
	int cpu;

again:
	cpu  = get_cpu();
	if (likely(order == 0)) {
		struct per_cpu_pages *pcp;
		struct list_head *list;

		pcp = &zone_pcp(zone, cpu)->pcp;
		list = &pcp->lists[migratetype];
		local_irq_save(flags);
		if (list_empty(list)) {
			pcp->count += rmqueue_bulk(zone, 0,
					pcp->batch, list,
					migratetype, cold);
			if (unlikely(list_empty(list)))
				goto failed;
		}

		if (cold)
			page = list_entry(list->prev, struct page, lru);
		else
			page = list_entry(list->next, struct page, lru);

		list_del(&page->lru);
		pcp->count--;
	} else {
		if (unlikely(gfp_flags & __GFP_NOFAIL)) {
			
			WARN_ON_ONCE(order > 1);
		}
		spin_lock_irqsave(&zone->lock, flags);
		page = __rmqueue(zone, order, migratetype);
		spin_unlock(&zone->lock);
		if (!page)
			goto failed;
		__mod_zone_page_state(zone, NR_FREE_PAGES, -(1 << order));
	}

	__count_zone_vm_events(PGALLOC, zone, 1 << order);
	zone_statistics(preferred_zone, zone);
	local_irq_restore(flags);
	put_cpu();

	VM_BUG_ON(bad_range(zone, page));
	if (prep_new_page(page, order, gfp_flags))
		goto again;
	return page;

failed:
	local_irq_restore(flags);
	put_cpu();
	return NULL;
}


#define ALLOC_WMARK_MIN		WMARK_MIN
#define ALLOC_WMARK_LOW		WMARK_LOW
#define ALLOC_WMARK_HIGH	WMARK_HIGH
#define ALLOC_NO_WATERMARKS	0x04 


#define ALLOC_WMARK_MASK	(ALLOC_NO_WATERMARKS-1)

#define ALLOC_HARDER		0x10 
#define ALLOC_HIGH		0x20 
#define ALLOC_CPUSET		0x40 

#ifdef CONFIG_FAIL_PAGE_ALLOC

static struct fail_page_alloc_attr {
	struct fault_attr attr;

	u32 ignore_gfp_highmem;
	u32 ignore_gfp_wait;
	u32 min_order;

#ifdef CONFIG_FAULT_INJECTION_DEBUG_FS

	struct dentry *ignore_gfp_highmem_file;
	struct dentry *ignore_gfp_wait_file;
	struct dentry *min_order_file;

#endif 

} fail_page_alloc = {
	.attr = FAULT_ATTR_INITIALIZER,
	.ignore_gfp_wait = 1,
	.ignore_gfp_highmem = 1,
	.min_order = 1,
};

static int __init setup_fail_page_alloc(char *str)
{
	return setup_fault_attr(&fail_page_alloc.attr, str);
}
__setup("fail_page_alloc=", setup_fail_page_alloc);

static int should_fail_alloc_page(gfp_t gfp_mask, unsigned int order)
{
	if (order < fail_page_alloc.min_order)
		return 0;
	if (gfp_mask & __GFP_NOFAIL)
		return 0;
	if (fail_page_alloc.ignore_gfp_highmem && (gfp_mask & __GFP_HIGHMEM))
		return 0;
	if (fail_page_alloc.ignore_gfp_wait && (gfp_mask & __GFP_WAIT))
		return 0;

	return should_fail(&fail_page_alloc.attr, 1 << order);
}

#ifdef CONFIG_FAULT_INJECTION_DEBUG_FS

static int __init fail_page_alloc_debugfs(void)
{
	mode_t mode = S_IFREG | S_IRUSR | S_IWUSR;
	struct dentry *dir;
	int err;

	err = init_fault_attr_dentries(&fail_page_alloc.attr,
				       "fail_page_alloc");
	if (err)
		return err;
	dir = fail_page_alloc.attr.dentries.dir;

	fail_page_alloc.ignore_gfp_wait_file =
		debugfs_create_bool("ignore-gfp-wait", mode, dir,
				      &fail_page_alloc.ignore_gfp_wait);

	fail_page_alloc.ignore_gfp_highmem_file =
		debugfs_create_bool("ignore-gfp-highmem", mode, dir,
				      &fail_page_alloc.ignore_gfp_highmem);
	fail_page_alloc.min_order_file =
		debugfs_create_u32("min-order", mode, dir,
				   &fail_page_alloc.min_order);

	if (!fail_page_alloc.ignore_gfp_wait_file ||
            !fail_page_alloc.ignore_gfp_highmem_file ||
            !fail_page_alloc.min_order_file) {
		err = -ENOMEM;
		debugfs_remove(fail_page_alloc.ignore_gfp_wait_file);
		debugfs_remove(fail_page_alloc.ignore_gfp_highmem_file);
		debugfs_remove(fail_page_alloc.min_order_file);
		cleanup_fault_attr_dentries(&fail_page_alloc.attr);
	}

	return err;
}

late_initcall(fail_page_alloc_debugfs);

#endif 

#else 

static inline int should_fail_alloc_page(gfp_t gfp_mask, unsigned int order)
{
	return 0;
}

#endif 


int zone_watermark_ok(struct zone *z, int order, unsigned long mark,
		      int classzone_idx, int alloc_flags)
{
	
	long min = mark;
	long free_pages = zone_page_state(z, NR_FREE_PAGES) - (1 << order) + 1;
	int o;

	if (alloc_flags & ALLOC_HIGH)
		min -= min / 2;
	if (alloc_flags & ALLOC_HARDER)
		min -= min / 4;

	if (free_pages <= min + z->lowmem_reserve[classzone_idx])
		return 0;
	for (o = 0; o < order; o++) {
		
		free_pages -= z->free_area[o].nr_free << o;

		
		min >>= min_free_order_shift;

		if (free_pages <= min)
			return 0;
	}
	return 1;
}

#ifdef CONFIG_NUMA

static nodemask_t *zlc_setup(struct zonelist *zonelist, int alloc_flags)
{
	struct zonelist_cache *zlc;	
	nodemask_t *allowednodes;	

	zlc = zonelist->zlcache_ptr;
	if (!zlc)
		return NULL;

	if (time_after(jiffies, zlc->last_full_zap + HZ)) {
		bitmap_zero(zlc->fullzones, MAX_ZONES_PER_ZONELIST);
		zlc->last_full_zap = jiffies;
	}

	allowednodes = !in_interrupt() && (alloc_flags & ALLOC_CPUSET) ?
					&cpuset_current_mems_allowed :
					&node_states[N_HIGH_MEMORY];
	return allowednodes;
}


static int zlc_zone_worth_trying(struct zonelist *zonelist, struct zoneref *z,
						nodemask_t *allowednodes)
{
	struct zonelist_cache *zlc;	
	int i;				
	int n;				

	zlc = zonelist->zlcache_ptr;
	if (!zlc)
		return 1;

	i = z - zonelist->_zonerefs;
	n = zlc->z_to_n[i];

	
	return node_isset(n, *allowednodes) && !test_bit(i, zlc->fullzones);
}


static void zlc_mark_zone_full(struct zonelist *zonelist, struct zoneref *z)
{
	struct zonelist_cache *zlc;	
	int i;				

	zlc = zonelist->zlcache_ptr;
	if (!zlc)
		return;

	i = z - zonelist->_zonerefs;

	set_bit(i, zlc->fullzones);
}

#else	

static nodemask_t *zlc_setup(struct zonelist *zonelist, int alloc_flags)
{
	return NULL;
}

static int zlc_zone_worth_trying(struct zonelist *zonelist, struct zoneref *z,
				nodemask_t *allowednodes)
{
	return 1;
}

static void zlc_mark_zone_full(struct zonelist *zonelist, struct zoneref *z)
{
}
#endif	


static struct page *
get_page_from_freelist(gfp_t gfp_mask, nodemask_t *nodemask, unsigned int order,
		struct zonelist *zonelist, int high_zoneidx, int alloc_flags,
		struct zone *preferred_zone, int migratetype)
{
	struct zoneref *z;
	struct page *page = NULL;
	int classzone_idx;
	struct zone *zone;
	nodemask_t *allowednodes = NULL;
	int zlc_active = 0;		
	int did_zlc_setup = 0;		

	classzone_idx = zone_idx(preferred_zone);
zonelist_scan:
	
	for_each_zone_zonelist_nodemask(zone, z, zonelist,
						high_zoneidx, nodemask) {
		if (NUMA_BUILD && zlc_active &&
			!zlc_zone_worth_trying(zonelist, z, allowednodes))
				continue;
		if ((alloc_flags & ALLOC_CPUSET) &&
			!cpuset_zone_allowed_softwall(zone, gfp_mask))
				goto try_next_zone;

		BUILD_BUG_ON(ALLOC_NO_WATERMARKS < NR_WMARK);
		if (!(alloc_flags & ALLOC_NO_WATERMARKS)) {
			unsigned long mark;
			int ret;

			mark = zone->watermark[alloc_flags & ALLOC_WMARK_MASK];
			if (zone_watermark_ok(zone, order, mark,
				    classzone_idx, alloc_flags))
				goto try_this_zone;

			if (zone_reclaim_mode == 0)
				goto this_zone_full;

			ret = zone_reclaim(zone, gfp_mask, order);
			switch (ret) {
			case ZONE_RECLAIM_NOSCAN:
				
				goto try_next_zone;
			case ZONE_RECLAIM_FULL:
				
				goto this_zone_full;
			default:
				
				if (!zone_watermark_ok(zone, order, mark,
						classzone_idx, alloc_flags))
					goto this_zone_full;
			}
		}

try_this_zone:
		page = buffered_rmqueue(preferred_zone, zone, order,
						gfp_mask, migratetype);
		if (page)
			break;
this_zone_full:
		if (NUMA_BUILD)
			zlc_mark_zone_full(zonelist, z);
try_next_zone:
		if (NUMA_BUILD && !did_zlc_setup && nr_online_nodes > 1) {
			
			allowednodes = zlc_setup(zonelist, alloc_flags);
			zlc_active = 1;
			did_zlc_setup = 1;
		}
	}

	if (unlikely(NUMA_BUILD && page == NULL && zlc_active)) {
		
		zlc_active = 0;
		goto zonelist_scan;
	}
	return page;
}

static inline int
should_alloc_retry(gfp_t gfp_mask, unsigned int order,
				unsigned long pages_reclaimed)
{
	
	if (gfp_mask & __GFP_NORETRY)
		return 0;

	
	if (order <= PAGE_ALLOC_COSTLY_ORDER)
		return 1;

	
	if (gfp_mask & __GFP_REPEAT && pages_reclaimed < (1 << order))
		return 1;

	
	if (gfp_mask & __GFP_NOFAIL)
		return 1;

	return 0;
}

static inline struct page *
__alloc_pages_may_oom(gfp_t gfp_mask, unsigned int order,
	struct zonelist *zonelist, enum zone_type high_zoneidx,
	nodemask_t *nodemask, struct zone *preferred_zone,
	int migratetype)
{
	struct page *page;

	
	if (!try_set_zone_oom(zonelist, gfp_mask)) {
		schedule_timeout_uninterruptible(1);
		return NULL;
	}

	
	page = get_page_from_freelist(gfp_mask|__GFP_HARDWALL, nodemask,
		order, zonelist, high_zoneidx,
		ALLOC_WMARK_HIGH|ALLOC_CPUSET,
		preferred_zone, migratetype);
	if (page)
		goto out;

	
	if (order > PAGE_ALLOC_COSTLY_ORDER && !(gfp_mask & __GFP_NOFAIL))
		goto out;

	
	out_of_memory(zonelist, gfp_mask, order);

out:
	clear_zonelist_oom(zonelist, gfp_mask);
	return page;
}


static inline struct page *
__alloc_pages_direct_reclaim(gfp_t gfp_mask, unsigned int order,
	struct zonelist *zonelist, enum zone_type high_zoneidx,
	nodemask_t *nodemask, int alloc_flags, struct zone *preferred_zone,
	int migratetype, unsigned long *did_some_progress)
{
	struct page *page = NULL;
	struct reclaim_state reclaim_state;
	struct task_struct *p = current;

	cond_resched();

	
	cpuset_memory_pressure_bump();
	p->flags |= PF_MEMALLOC;
	lockdep_set_current_reclaim_state(gfp_mask);
	reclaim_state.reclaimed_slab = 0;
	p->reclaim_state = &reclaim_state;

	*did_some_progress = try_to_free_pages(zonelist, order, gfp_mask, nodemask);

	p->reclaim_state = NULL;
	lockdep_clear_current_reclaim_state();
	p->flags &= ~PF_MEMALLOC;

	cond_resched();

	if (order != 0)
		drain_all_pages();

	if (likely(*did_some_progress))
		page = get_page_from_freelist(gfp_mask, nodemask, order,
					zonelist, high_zoneidx,
					alloc_flags, preferred_zone,
					migratetype);
	return page;
}


static inline struct page *
__alloc_pages_high_priority(gfp_t gfp_mask, unsigned int order,
	struct zonelist *zonelist, enum zone_type high_zoneidx,
	nodemask_t *nodemask, struct zone *preferred_zone,
	int migratetype)
{
	struct page *page;

	do {
		page = get_page_from_freelist(gfp_mask, nodemask, order,
			zonelist, high_zoneidx, ALLOC_NO_WATERMARKS,
			preferred_zone, migratetype);

		if (!page && gfp_mask & __GFP_NOFAIL)
			congestion_wait(BLK_RW_ASYNC, HZ/50);
	} while (!page && (gfp_mask & __GFP_NOFAIL));

	return page;
}

static inline
void wake_all_kswapd(unsigned int order, struct zonelist *zonelist,
						enum zone_type high_zoneidx)
{
	struct zoneref *z;
	struct zone *zone;

	for_each_zone_zonelist(zone, z, zonelist, high_zoneidx)
		wakeup_kswapd(zone, order);
}

static inline int
gfp_to_alloc_flags(gfp_t gfp_mask)
{
	struct task_struct *p = current;
	int alloc_flags = ALLOC_WMARK_MIN | ALLOC_CPUSET;
	const gfp_t wait = gfp_mask & __GFP_WAIT;

	
	BUILD_BUG_ON(__GFP_HIGH != ALLOC_HIGH);

	
	alloc_flags |= (gfp_mask & __GFP_HIGH);

	if (!wait) {
		alloc_flags |= ALLOC_HARDER;
		
		alloc_flags &= ~ALLOC_CPUSET;
	} else if (unlikely(rt_task(p)) && !in_interrupt())
		alloc_flags |= ALLOC_HARDER;

	if (likely(!(gfp_mask & __GFP_NOMEMALLOC))) {
		if (!in_interrupt() &&
		    ((p->flags & PF_MEMALLOC) ||
		     unlikely(test_thread_flag(TIF_MEMDIE))))
			alloc_flags |= ALLOC_NO_WATERMARKS;
	}

	return alloc_flags;
}

static inline struct page *
__alloc_pages_slowpath(gfp_t gfp_mask, unsigned int order,
	struct zonelist *zonelist, enum zone_type high_zoneidx,
	nodemask_t *nodemask, struct zone *preferred_zone,
	int migratetype)
{
	const gfp_t wait = gfp_mask & __GFP_WAIT;
	struct page *page = NULL;
	int alloc_flags;
	unsigned long pages_reclaimed = 0;
	unsigned long did_some_progress;
	struct task_struct *p = current;

	
	if (order >= MAX_ORDER) {
		WARN_ON_ONCE(!(gfp_mask & __GFP_NOWARN));
		return NULL;
	}

	
	if (NUMA_BUILD && (gfp_mask & GFP_THISNODE) == GFP_THISNODE)
		goto nopage;

restart:
	wake_all_kswapd(order, zonelist, high_zoneidx);

	
	alloc_flags = gfp_to_alloc_flags(gfp_mask);

	
	page = get_page_from_freelist(gfp_mask, nodemask, order, zonelist,
			high_zoneidx, alloc_flags & ~ALLOC_NO_WATERMARKS,
			preferred_zone, migratetype);
	if (page)
		goto got_pg;

rebalance:
	
	if (alloc_flags & ALLOC_NO_WATERMARKS) {
		page = __alloc_pages_high_priority(gfp_mask, order,
				zonelist, high_zoneidx, nodemask,
				preferred_zone, migratetype);
		if (page)
			goto got_pg;
	}

	
	if (!wait)
		goto nopage;

	
	if (p->flags & PF_MEMALLOC)
		goto nopage;

	
	if (test_thread_flag(TIF_MEMDIE) && !(gfp_mask & __GFP_NOFAIL))
		goto nopage;

	
	page = __alloc_pages_direct_reclaim(gfp_mask, order,
					zonelist, high_zoneidx,
					nodemask,
					alloc_flags, preferred_zone,
					migratetype, &did_some_progress);
	if (page)
		goto got_pg;

	
	if (!did_some_progress) {
		if ((gfp_mask & __GFP_FS) && !(gfp_mask & __GFP_NORETRY)) {
			if (oom_killer_disabled)
				goto nopage;
			page = __alloc_pages_may_oom(gfp_mask, order,
					zonelist, high_zoneidx,
					nodemask, preferred_zone,
					migratetype);
			if (page)
				goto got_pg;

			
			if (order > PAGE_ALLOC_COSTLY_ORDER &&
						!(gfp_mask & __GFP_NOFAIL))
				goto nopage;

			goto restart;
		}
	}

	
	pages_reclaimed += did_some_progress;
	if (should_alloc_retry(gfp_mask, order, pages_reclaimed)) {
		
		congestion_wait(BLK_RW_ASYNC, HZ/50);
		goto rebalance;
	}

nopage:
	if (!(gfp_mask & __GFP_NOWARN) && printk_ratelimit()) {
		printk(KERN_WARNING "%s: page allocation failure."
			" order:%d, mode:0x%x\n",
			p->comm, order, gfp_mask);
		dump_stack();
		show_mem();
	}
	return page;
got_pg:
	if (kmemcheck_enabled)
		kmemcheck_pagealloc_alloc(page, order, gfp_mask);
	return page;

}


struct page *
__alloc_pages_nodemask(gfp_t gfp_mask, unsigned int order,
			struct zonelist *zonelist, nodemask_t *nodemask)
{
	enum zone_type high_zoneidx = gfp_zone(gfp_mask);
	struct zone *preferred_zone;
	struct page *page;
	int migratetype = allocflags_to_migratetype(gfp_mask);

	gfp_mask &= gfp_allowed_mask;

	lockdep_trace_alloc(gfp_mask);

	might_sleep_if(gfp_mask & __GFP_WAIT);

	if (should_fail_alloc_page(gfp_mask, order))
		return NULL;

	
	if (unlikely(!zonelist->_zonerefs->zone))
		return NULL;

	
	first_zones_zonelist(zonelist, high_zoneidx, nodemask, &preferred_zone);
	if (!preferred_zone)
		return NULL;

	
	page = get_page_from_freelist(gfp_mask|__GFP_HARDWALL, nodemask, order,
			zonelist, high_zoneidx, ALLOC_WMARK_LOW|ALLOC_CPUSET,
			preferred_zone, migratetype);
	if (unlikely(!page))
		page = __alloc_pages_slowpath(gfp_mask, order,
				zonelist, high_zoneidx, nodemask,
				preferred_zone, migratetype);

	trace_mm_page_alloc(page, order, gfp_mask, migratetype);
	return page;
}
EXPORT_SYMBOL(__alloc_pages_nodemask);


unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order)
{
	struct page *page;

	
	VM_BUG_ON((gfp_mask & __GFP_HIGHMEM) != 0);

	page = alloc_pages(gfp_mask, order);
	if (!page)
		return 0;
	return (unsigned long) page_address(page);
}
EXPORT_SYMBOL(__get_free_pages);

unsigned long get_zeroed_page(gfp_t gfp_mask)
{
	return __get_free_pages(gfp_mask | __GFP_ZERO, 0);
}
EXPORT_SYMBOL(get_zeroed_page);

void __pagevec_free(struct pagevec *pvec)
{
	int i = pagevec_count(pvec);

	while (--i >= 0) {
		trace_mm_pagevec_free(pvec->pages[i], pvec->cold);
		free_hot_cold_page(pvec->pages[i], pvec->cold);
	}
}

void __free_pages(struct page *page, unsigned int order)
{
	if (put_page_testzero(page)) {
		trace_mm_page_free_direct(page, order);
		if (order == 0)
			free_hot_page(page);
		else
			__free_pages_ok(page, order);
	}
}

EXPORT_SYMBOL(__free_pages);

void free_pages(unsigned long addr, unsigned int order)
{
	if (addr != 0) {
		VM_BUG_ON(!virt_addr_valid((void *)addr));
		__free_pages(virt_to_page((void *)addr), order);
	}
}

EXPORT_SYMBOL(free_pages);


void *alloc_pages_exact(size_t size, gfp_t gfp_mask)
{
	unsigned int order = get_order(size);
	unsigned long addr;

	addr = __get_free_pages(gfp_mask, order);
	if (addr) {
		unsigned long alloc_end = addr + (PAGE_SIZE << order);
		unsigned long used = addr + PAGE_ALIGN(size);

		split_page(virt_to_page((void *)addr), order);
		while (used < alloc_end) {
			free_page(used);
			used += PAGE_SIZE;
		}
	}

	return (void *)addr;
}
EXPORT_SYMBOL(alloc_pages_exact);


void free_pages_exact(void *virt, size_t size)
{
	unsigned long addr = (unsigned long)virt;
	unsigned long end = addr + PAGE_ALIGN(size);

	while (addr < end) {
		free_page(addr);
		addr += PAGE_SIZE;
	}
}
EXPORT_SYMBOL(free_pages_exact);

static unsigned int nr_free_zone_pages(int offset)
{
	struct zoneref *z;
	struct zone *zone;

	
	unsigned int sum = 0;

	struct zonelist *zonelist = node_zonelist(numa_node_id(), GFP_KERNEL);

	for_each_zone_zonelist(zone, z, zonelist, offset) {
		unsigned long size = zone->present_pages;
		unsigned long high = high_wmark_pages(zone);
		if (size > high)
			sum += size - high;
	}

	return sum;
}


unsigned int nr_free_buffer_pages(void)
{
	return nr_free_zone_pages(gfp_zone(GFP_USER));
}
EXPORT_SYMBOL_GPL(nr_free_buffer_pages);


unsigned int nr_free_pagecache_pages(void)
{
	return nr_free_zone_pages(gfp_zone(GFP_HIGHUSER_MOVABLE));
}

static inline void show_node(struct zone *zone)
{
	if (NUMA_BUILD)
		printk("Node %d ", zone_to_nid(zone));
}

void si_meminfo(struct sysinfo *val)
{
	val->totalram = totalram_pages;
	val->sharedram = 0;
	val->freeram = global_page_state(NR_FREE_PAGES);
	val->bufferram = nr_blockdev_pages();
	val->totalhigh = totalhigh_pages;
	val->freehigh = nr_free_highpages();
	val->mem_unit = PAGE_SIZE;
}

EXPORT_SYMBOL(si_meminfo);

#ifdef CONFIG_NUMA
void si_meminfo_node(struct sysinfo *val, int nid)
{
	pg_data_t *pgdat = NODE_DATA(nid);

	val->totalram = pgdat->node_present_pages;
	val->freeram = node_page_state(nid, NR_FREE_PAGES);
#ifdef CONFIG_HIGHMEM
	val->totalhigh = pgdat->node_zones[ZONE_HIGHMEM].present_pages;
	val->freehigh = zone_page_state(&pgdat->node_zones[ZONE_HIGHMEM],
			NR_FREE_PAGES);
#else
	val->totalhigh = 0;
	val->freehigh = 0;
#endif
	val->mem_unit = PAGE_SIZE;
}
#endif

#define K(x) ((x) << (PAGE_SHIFT-10))


void show_free_areas(void)
{
	int cpu;
	struct zone *zone;

	for_each_populated_zone(zone) {
		show_node(zone);
		printk("%s per-cpu:\n", zone->name);

		for_each_online_cpu(cpu) {
			struct per_cpu_pageset *pageset;

			pageset = zone_pcp(zone, cpu);

			printk("CPU %4d: hi:%5d, btch:%4d usd:%4d\n",
			       cpu, pageset->pcp.high,
			       pageset->pcp.batch, pageset->pcp.count);
		}
	}

	printk("active_anon:%lu inactive_anon:%lu isolated_anon:%lu\n"
		" active_file:%lu inactive_file:%lu isolated_file:%lu\n"
		" unevictable:%lu"
		" dirty:%lu writeback:%lu unstable:%lu\n"
		" free:%lu slab_reclaimable:%lu slab_unreclaimable:%lu\n"
		" mapped:%lu shmem:%lu pagetables:%lu bounce:%lu\n",
		global_page_state(NR_ACTIVE_ANON),
		global_page_state(NR_INACTIVE_ANON),
		global_page_state(NR_ISOLATED_ANON),
		global_page_state(NR_ACTIVE_FILE),
		global_page_state(NR_INACTIVE_FILE),
		global_page_state(NR_ISOLATED_FILE),
		global_page_state(NR_UNEVICTABLE),
		global_page_state(NR_FILE_DIRTY),
		global_page_state(NR_WRITEBACK),
		global_page_state(NR_UNSTABLE_NFS),
		global_page_state(NR_FREE_PAGES),
		global_page_state(NR_SLAB_RECLAIMABLE),
		global_page_state(NR_SLAB_UNRECLAIMABLE),
		global_page_state(NR_FILE_MAPPED),
		global_page_state(NR_SHMEM),
		global_page_state(NR_PAGETABLE),
		global_page_state(NR_BOUNCE));

	for_each_populated_zone(zone) {
		int i;

		show_node(zone);
		printk("%s"
			" free:%lukB"
			" min:%lukB"
			" low:%lukB"
			" high:%lukB"
			" active_anon:%lukB"
			" inactive_anon:%lukB"
			" active_file:%lukB"
			" inactive_file:%lukB"
			" unevictable:%lukB"
			" isolated(anon):%lukB"
			" isolated(file):%lukB"
			" present:%lukB"
			" mlocked:%lukB"
			" dirty:%lukB"
			" writeback:%lukB"
			" mapped:%lukB"
			" shmem:%lukB"
			" slab_reclaimable:%lukB"
			" slab_unreclaimable:%lukB"
			" kernel_stack:%lukB"
			" pagetables:%lukB"
			" unstable:%lukB"
			" bounce:%lukB"
			" writeback_tmp:%lukB"
			" pages_scanned:%lu"
			" all_unreclaimable? %s"
			"\n",
			zone->name,
			K(zone_page_state(zone, NR_FREE_PAGES)),
			K(min_wmark_pages(zone)),
			K(low_wmark_pages(zone)),
			K(high_wmark_pages(zone)),
			K(zone_page_state(zone, NR_ACTIVE_ANON)),
			K(zone_page_state(zone, NR_INACTIVE_ANON)),
			K(zone_page_state(zone, NR_ACTIVE_FILE)),
			K(zone_page_state(zone, NR_INACTIVE_FILE)),
			K(zone_page_state(zone, NR_UNEVICTABLE)),
			K(zone_page_state(zone, NR_ISOLATED_ANON)),
			K(zone_page_state(zone, NR_ISOLATED_FILE)),
			K(zone->present_pages),
			K(zone_page_state(zone, NR_MLOCK)),
			K(zone_page_state(zone, NR_FILE_DIRTY)),
			K(zone_page_state(zone, NR_WRITEBACK)),
			K(zone_page_state(zone, NR_FILE_MAPPED)),
			K(zone_page_state(zone, NR_SHMEM)),
			K(zone_page_state(zone, NR_SLAB_RECLAIMABLE)),
			K(zone_page_state(zone, NR_SLAB_UNRECLAIMABLE)),
			zone_page_state(zone, NR_KERNEL_STACK) *
				THREAD_SIZE / 1024,
			K(zone_page_state(zone, NR_PAGETABLE)),
			K(zone_page_state(zone, NR_UNSTABLE_NFS)),
			K(zone_page_state(zone, NR_BOUNCE)),
			K(zone_page_state(zone, NR_WRITEBACK_TEMP)),
			zone->pages_scanned,
			(zone_is_all_unreclaimable(zone) ? "yes" : "no")
			);
		printk("lowmem_reserve[]:");
		for (i = 0; i < MAX_NR_ZONES; i++)
			printk(" %lu", zone->lowmem_reserve[i]);
		printk("\n");
	}

	for_each_populated_zone(zone) {
 		unsigned long nr[MAX_ORDER], flags, order, total = 0;

		show_node(zone);
		printk("%s: ", zone->name);

		spin_lock_irqsave(&zone->lock, flags);
		for (order = 0; order < MAX_ORDER; order++) {
			nr[order] = zone->free_area[order].nr_free;
			total += nr[order] << order;
		}
		spin_unlock_irqrestore(&zone->lock, flags);
		for (order = 0; order < MAX_ORDER; order++)
			printk("%lu*%lukB ", nr[order], K(1UL) << order);
		printk("= %lukB\n", K(total));
	}

	printk("%ld total pagecache pages\n", global_page_state(NR_FILE_PAGES));

	show_swap_cache_info();
}

static void zoneref_set_zone(struct zone *zone, struct zoneref *zoneref)
{
	zoneref->zone = zone;
	zoneref->zone_idx = zone_idx(zone);
}


static int build_zonelists_node(pg_data_t *pgdat, struct zonelist *zonelist,
				int nr_zones, enum zone_type zone_type)
{
	struct zone *zone;

	BUG_ON(zone_type >= MAX_NR_ZONES);
	zone_type++;

	do {
		zone_type--;
		zone = pgdat->node_zones + zone_type;
		if (populated_zone(zone)) {
			zoneref_set_zone(zone,
				&zonelist->_zonerefs[nr_zones++]);
			check_highest_zone(zone_type);
		}

	} while (zone_type);
	return nr_zones;
}



#define ZONELIST_ORDER_DEFAULT  0
#define ZONELIST_ORDER_NODE     1
#define ZONELIST_ORDER_ZONE     2


static int current_zonelist_order = ZONELIST_ORDER_DEFAULT;
static char zonelist_order_name[3][8] = {"Default", "Node", "Zone"};


#ifdef CONFIG_NUMA

static int user_zonelist_order = ZONELIST_ORDER_DEFAULT;

#define NUMA_ZONELIST_ORDER_LEN	16
char numa_zonelist_order[16] = "default";



static int __parse_numa_zonelist_order(char *s)
{
	if (*s == 'd' || *s == 'D') {
		user_zonelist_order = ZONELIST_ORDER_DEFAULT;
	} else if (*s == 'n' || *s == 'N') {
		user_zonelist_order = ZONELIST_ORDER_NODE;
	} else if (*s == 'z' || *s == 'Z') {
		user_zonelist_order = ZONELIST_ORDER_ZONE;
	} else {
		printk(KERN_WARNING
			"Ignoring invalid numa_zonelist_order value:  "
			"%s\n", s);
		return -EINVAL;
	}
	return 0;
}

static __init int setup_numa_zonelist_order(char *s)
{
	if (s)
		return __parse_numa_zonelist_order(s);
	return 0;
}
early_param("numa_zonelist_order", setup_numa_zonelist_order);


int numa_zonelist_order_handler(ctl_table *table, int write,
		void __user *buffer, size_t *length,
		loff_t *ppos)
{
	char saved_string[NUMA_ZONELIST_ORDER_LEN];
	int ret;

	if (write)
		strncpy(saved_string, (char*)table->data,
			NUMA_ZONELIST_ORDER_LEN);
	ret = proc_dostring(table, write, buffer, length, ppos);
	if (ret)
		return ret;
	if (write) {
		int oldval = user_zonelist_order;
		if (__parse_numa_zonelist_order((char*)table->data)) {
			
			strncpy((char*)table->data, saved_string,
				NUMA_ZONELIST_ORDER_LEN);
			user_zonelist_order = oldval;
		} else if (oldval != user_zonelist_order)
			build_all_zonelists();
	}
	return 0;
}


#define MAX_NODE_LOAD (nr_online_nodes)
static int node_load[MAX_NUMNODES];


static int find_next_best_node(int node, nodemask_t *used_node_mask)
{
	int n, val;
	int min_val = INT_MAX;
	int best_node = -1;
	const struct cpumask *tmp = cpumask_of_node(0);

	
	if (!node_isset(node, *used_node_mask)) {
		node_set(node, *used_node_mask);
		return node;
	}

	for_each_node_state(n, N_HIGH_MEMORY) {

		
		if (node_isset(n, *used_node_mask))
			continue;

		
		val = node_distance(node, n);

		
		val += (n < node);

		
		tmp = cpumask_of_node(n);
		if (!cpumask_empty(tmp))
			val += PENALTY_FOR_NODE_WITH_CPUS;

		
		val *= (MAX_NODE_LOAD*MAX_NUMNODES);
		val += node_load[n];

		if (val < min_val) {
			min_val = val;
			best_node = n;
		}
	}

	if (best_node >= 0)
		node_set(best_node, *used_node_mask);

	return best_node;
}



static void build_zonelists_in_node_order(pg_data_t *pgdat, int node)
{
	int j;
	struct zonelist *zonelist;

	zonelist = &pgdat->node_zonelists[0];
	for (j = 0; zonelist->_zonerefs[j].zone != NULL; j++)
		;
	j = build_zonelists_node(NODE_DATA(node), zonelist, j,
							MAX_NR_ZONES - 1);
	zonelist->_zonerefs[j].zone = NULL;
	zonelist->_zonerefs[j].zone_idx = 0;
}


static void build_thisnode_zonelists(pg_data_t *pgdat)
{
	int j;
	struct zonelist *zonelist;

	zonelist = &pgdat->node_zonelists[1];
	j = build_zonelists_node(pgdat, zonelist, 0, MAX_NR_ZONES - 1);
	zonelist->_zonerefs[j].zone = NULL;
	zonelist->_zonerefs[j].zone_idx = 0;
}


static int node_order[MAX_NUMNODES];

static void build_zonelists_in_zone_order(pg_data_t *pgdat, int nr_nodes)
{
	int pos, j, node;
	int zone_type;		
	struct zone *z;
	struct zonelist *zonelist;

	zonelist = &pgdat->node_zonelists[0];
	pos = 0;
	for (zone_type = MAX_NR_ZONES - 1; zone_type >= 0; zone_type--) {
		for (j = 0; j < nr_nodes; j++) {
			node = node_order[j];
			z = &NODE_DATA(node)->node_zones[zone_type];
			if (populated_zone(z)) {
				zoneref_set_zone(z,
					&zonelist->_zonerefs[pos++]);
				check_highest_zone(zone_type);
			}
		}
	}
	zonelist->_zonerefs[pos].zone = NULL;
	zonelist->_zonerefs[pos].zone_idx = 0;
}

static int default_zonelist_order(void)
{
	int nid, zone_type;
	unsigned long low_kmem_size,total_size;
	struct zone *z;
	int average_size;
	
	
	low_kmem_size = 0;
	total_size = 0;
	for_each_online_node(nid) {
		for (zone_type = 0; zone_type < MAX_NR_ZONES; zone_type++) {
			z = &NODE_DATA(nid)->node_zones[zone_type];
			if (populated_zone(z)) {
				if (zone_type < ZONE_NORMAL)
					low_kmem_size += z->present_pages;
				total_size += z->present_pages;
			}
		}
	}
	if (!low_kmem_size ||  
	    low_kmem_size > total_size/2) 
		return ZONELIST_ORDER_NODE;
	
	average_size = total_size /
				(nodes_weight(node_states[N_HIGH_MEMORY]) + 1);
	for_each_online_node(nid) {
		low_kmem_size = 0;
		total_size = 0;
		for (zone_type = 0; zone_type < MAX_NR_ZONES; zone_type++) {
			z = &NODE_DATA(nid)->node_zones[zone_type];
			if (populated_zone(z)) {
				if (zone_type < ZONE_NORMAL)
					low_kmem_size += z->present_pages;
				total_size += z->present_pages;
			}
		}
		if (low_kmem_size &&
		    total_size > average_size && 
		    low_kmem_size > total_size * 70/100)
			return ZONELIST_ORDER_NODE;
	}
	return ZONELIST_ORDER_ZONE;
}

static void set_zonelist_order(void)
{
	if (user_zonelist_order == ZONELIST_ORDER_DEFAULT)
		current_zonelist_order = default_zonelist_order();
	else
		current_zonelist_order = user_zonelist_order;
}

static void build_zonelists(pg_data_t *pgdat)
{
	int j, node, load;
	enum zone_type i;
	nodemask_t used_mask;
	int local_node, prev_node;
	struct zonelist *zonelist;
	int order = current_zonelist_order;

	
	for (i = 0; i < MAX_ZONELISTS; i++) {
		zonelist = pgdat->node_zonelists + i;
		zonelist->_zonerefs[0].zone = NULL;
		zonelist->_zonerefs[0].zone_idx = 0;
	}

	
	local_node = pgdat->node_id;
	load = nr_online_nodes;
	prev_node = local_node;
	nodes_clear(used_mask);

	memset(node_order, 0, sizeof(node_order));
	j = 0;

	while ((node = find_next_best_node(local_node, &used_mask)) >= 0) {
		int distance = node_distance(local_node, node);

		
		if (distance > RECLAIM_DISTANCE)
			zone_reclaim_mode = 1;

		
		if (distance != node_distance(local_node, prev_node))
			node_load[node] = load;

		prev_node = node;
		load--;
		if (order == ZONELIST_ORDER_NODE)
			build_zonelists_in_node_order(pgdat, node);
		else
			node_order[j++] = node;	
	}

	if (order == ZONELIST_ORDER_ZONE) {
		
		build_zonelists_in_zone_order(pgdat, j);
	}

	build_thisnode_zonelists(pgdat);
}


static void build_zonelist_cache(pg_data_t *pgdat)
{
	struct zonelist *zonelist;
	struct zonelist_cache *zlc;
	struct zoneref *z;

	zonelist = &pgdat->node_zonelists[0];
	zonelist->zlcache_ptr = zlc = &zonelist->zlcache;
	bitmap_zero(zlc->fullzones, MAX_ZONES_PER_ZONELIST);
	for (z = zonelist->_zonerefs; z->zone; z++)
		zlc->z_to_n[z - zonelist->_zonerefs] = zonelist_node_idx(z);
}


#else	

static void set_zonelist_order(void)
{
	current_zonelist_order = ZONELIST_ORDER_ZONE;
}

static void build_zonelists(pg_data_t *pgdat)
{
	int node, local_node;
	enum zone_type j;
	struct zonelist *zonelist;

	local_node = pgdat->node_id;

	zonelist = &pgdat->node_zonelists[0];
	j = build_zonelists_node(pgdat, zonelist, 0, MAX_NR_ZONES - 1);

	
	for (node = local_node + 1; node < MAX_NUMNODES; node++) {
		if (!node_online(node))
			continue;
		j = build_zonelists_node(NODE_DATA(node), zonelist, j,
							MAX_NR_ZONES - 1);
	}
	for (node = 0; node < local_node; node++) {
		if (!node_online(node))
			continue;
		j = build_zonelists_node(NODE_DATA(node), zonelist, j,
							MAX_NR_ZONES - 1);
	}

	zonelist->_zonerefs[j].zone = NULL;
	zonelist->_zonerefs[j].zone_idx = 0;
}


static void build_zonelist_cache(pg_data_t *pgdat)
{
	pgdat->node_zonelists[0].zlcache_ptr = NULL;
}

#endif	


static int __build_all_zonelists(void *dummy)
{
	int nid;

#ifdef CONFIG_NUMA
	memset(node_load, 0, sizeof(node_load));
#endif
	for_each_online_node(nid) {
		pg_data_t *pgdat = NODE_DATA(nid);

		build_zonelists(pgdat);
		build_zonelist_cache(pgdat);
	}
	return 0;
}

void build_all_zonelists(void)
{
	set_zonelist_order();

	if (system_state == SYSTEM_BOOTING) {
		__build_all_zonelists(NULL);
		mminit_verify_zonelist();
		cpuset_init_current_mems_allowed();
	} else {
		
		stop_machine(__build_all_zonelists, NULL, NULL);
		
	}
	vm_total_pages = nr_free_pagecache_pages();
	
	if (vm_total_pages < (pageblock_nr_pages * MIGRATE_TYPES))
		page_group_by_mobility_disabled = 1;
	else
		page_group_by_mobility_disabled = 0;

	printk("Built %i zonelists in %s order, mobility grouping %s.  "
		"Total pages: %ld\n",
			nr_online_nodes,
			zonelist_order_name[current_zonelist_order],
			page_group_by_mobility_disabled ? "off" : "on",
			vm_total_pages);
#ifdef CONFIG_NUMA
	printk("Policy zone: %s\n", zone_names[policy_zone]);
#endif
}


#define PAGES_PER_WAITQUEUE	256

#ifndef CONFIG_MEMORY_HOTPLUG
static inline unsigned long wait_table_hash_nr_entries(unsigned long pages)
{
	unsigned long size = 1;

	pages /= PAGES_PER_WAITQUEUE;

	while (size < pages)
		size <<= 1;

	
	size = min(size, 4096UL);

	return max(size, 4UL);
}
#else

static inline unsigned long wait_table_hash_nr_entries(unsigned long pages)
{
	return 4096UL;
}
#endif


static inline unsigned long wait_table_bits(unsigned long size)
{
	return ffz(~size);
}

#define LONG_ALIGN(x) (((x)+(sizeof(long))-1)&~((sizeof(long))-1))


static int pageblock_is_reserved(unsigned long start_pfn)
{
	unsigned long end_pfn = start_pfn + pageblock_nr_pages;
	unsigned long pfn;

	for (pfn = start_pfn; pfn < end_pfn; pfn++)
		if (PageReserved(pfn_to_page(pfn)))
			return 1;
	return 0;
}


static void setup_zone_migrate_reserve(struct zone *zone)
{
	unsigned long start_pfn, pfn, end_pfn;
	struct page *page;
	unsigned long block_migratetype;
	int reserve;

	
	start_pfn = zone->zone_start_pfn;
	end_pfn = start_pfn + zone->spanned_pages;
	reserve = roundup(min_wmark_pages(zone), pageblock_nr_pages) >>
							pageblock_order;

	
	reserve = min(2, reserve);

	for (pfn = start_pfn; pfn < end_pfn; pfn += pageblock_nr_pages) {
		if (!pfn_valid(pfn))
			continue;
		page = pfn_to_page(pfn);

		
		if (page_to_nid(page) != zone_to_nid(zone))
			continue;

		
		if (pageblock_is_reserved(pfn))
			continue;

		block_migratetype = get_pageblock_migratetype(page);

		
		if (reserve > 0 && block_migratetype == MIGRATE_RESERVE) {
			reserve--;
			continue;
		}

		
		if (reserve > 0 && block_migratetype == MIGRATE_MOVABLE) {
			set_pageblock_migratetype(page, MIGRATE_RESERVE);
			move_freepages_block(zone, page, MIGRATE_RESERVE);
			reserve--;
			continue;
		}

		
		if (block_migratetype == MIGRATE_RESERVE) {
			set_pageblock_migratetype(page, MIGRATE_MOVABLE);
			move_freepages_block(zone, page, MIGRATE_MOVABLE);
		}
	}
}


void __meminit memmap_init_zone(unsigned long size, int nid, unsigned long zone,
		unsigned long start_pfn, enum memmap_context context)
{
	struct page *page;
	unsigned long end_pfn = start_pfn + size;
	unsigned long pfn;
	struct zone *z;

	if (highest_memmap_pfn < end_pfn - 1)
		highest_memmap_pfn = end_pfn - 1;

	z = &NODE_DATA(nid)->node_zones[zone];
	for (pfn = start_pfn; pfn < end_pfn; pfn++) {
		
		if (context == MEMMAP_EARLY) {
			if (!early_pfn_valid(pfn))
				continue;
			if (!early_pfn_in_nid(pfn, nid))
				continue;
		}
		page = pfn_to_page(pfn);
		set_page_links(page, zone, nid, pfn);
		mminit_verify_page_links(page, zone, nid, pfn);
		init_page_count(page);
		reset_page_mapcount(page);
		SetPageReserved(page);
		
		if ((z->zone_start_pfn <= pfn)
		    && (pfn < z->zone_start_pfn + z->spanned_pages)
		    && !(pfn & (pageblock_nr_pages - 1)))
			set_pageblock_migratetype(page, MIGRATE_MOVABLE);

		INIT_LIST_HEAD(&page->lru);
#ifdef WANT_PAGE_VIRTUAL
		
		if (!is_highmem_idx(zone))
			set_page_address(page, __va(pfn << PAGE_SHIFT));
#endif
	}
}

static void __meminit zone_init_free_lists(struct zone *zone)
{
	int order, t;
	for_each_migratetype_order(order, t) {
		INIT_LIST_HEAD(&zone->free_area[order].free_list[t]);
		zone->free_area[order].nr_free = 0;
	}
}

#ifndef __HAVE_ARCH_MEMMAP_INIT
#define memmap_init(size, nid, zone, start_pfn) \
	memmap_init_zone((size), (nid), (zone), (start_pfn), MEMMAP_EARLY)
#endif

static int zone_batchsize(struct zone *zone)
{
#ifdef CONFIG_MMU
	int batch;

	
	batch = zone->present_pages / 1024;
	if (batch * PAGE_SIZE > 512 * 1024)
		batch = (512 * 1024) / PAGE_SIZE;
	batch /= 4;		
	if (batch < 1)
		batch = 1;

	
	batch = rounddown_pow_of_two(batch + batch/2) - 1;

	return batch;

#else
	
	return 0;
#endif
}

static void setup_pageset(struct per_cpu_pageset *p, unsigned long batch)
{
	struct per_cpu_pages *pcp;
	int migratetype;

	memset(p, 0, sizeof(*p));

	pcp = &p->pcp;
	pcp->count = 0;
	pcp->high = 6 * batch;
	pcp->batch = max(1UL, 1 * batch);
	for (migratetype = 0; migratetype < MIGRATE_PCPTYPES; migratetype++)
		INIT_LIST_HEAD(&pcp->lists[migratetype]);
}



static void setup_pagelist_highmark(struct per_cpu_pageset *p,
				unsigned long high)
{
	struct per_cpu_pages *pcp;

	pcp = &p->pcp;
	pcp->high = high;
	pcp->batch = max(1UL, high/4);
	if ((high/4) > (PAGE_SHIFT * 8))
		pcp->batch = PAGE_SHIFT * 8;
}


#ifdef CONFIG_NUMA

static struct per_cpu_pageset boot_pageset[NR_CPUS];


static int __cpuinit process_zones(int cpu)
{
	struct zone *zone, *dzone;
	int node = cpu_to_node(cpu);

	node_set_state(node, N_CPU);	

	for_each_populated_zone(zone) {
		zone_pcp(zone, cpu) = kmalloc_node(sizeof(struct per_cpu_pageset),
					 GFP_KERNEL, node);
		if (!zone_pcp(zone, cpu))
			goto bad;

		setup_pageset(zone_pcp(zone, cpu), zone_batchsize(zone));

		if (percpu_pagelist_fraction)
			setup_pagelist_highmark(zone_pcp(zone, cpu),
			 	(zone->present_pages / percpu_pagelist_fraction));
	}

	return 0;
bad:
	for_each_zone(dzone) {
		if (!populated_zone(dzone))
			continue;
		if (dzone == zone)
			break;
		kfree(zone_pcp(dzone, cpu));
		zone_pcp(dzone, cpu) = &boot_pageset[cpu];
	}
	return -ENOMEM;
}

static inline void free_zone_pagesets(int cpu)
{
	struct zone *zone;

	for_each_zone(zone) {
		struct per_cpu_pageset *pset = zone_pcp(zone, cpu);

		
		if (pset != &boot_pageset[cpu])
			kfree(pset);
		zone_pcp(zone, cpu) = &boot_pageset[cpu];
	}
}

static int __cpuinit pageset_cpuup_callback(struct notifier_block *nfb,
		unsigned long action,
		void *hcpu)
{
	int cpu = (long)hcpu;
	int ret = NOTIFY_OK;

	switch (action) {
	case CPU_UP_PREPARE:
	case CPU_UP_PREPARE_FROZEN:
		if (process_zones(cpu))
			ret = NOTIFY_BAD;
		break;
	case CPU_UP_CANCELED:
	case CPU_UP_CANCELED_FROZEN:
	case CPU_DEAD:
	case CPU_DEAD_FROZEN:
		free_zone_pagesets(cpu);
		break;
	default:
		break;
	}
	return ret;
}

static struct notifier_block __cpuinitdata pageset_notifier =
	{ &pageset_cpuup_callback, NULL, 0 };

void __init setup_per_cpu_pageset(void)
{
	int err;

	
	err = process_zones(smp_processor_id());
	BUG_ON(err);
	register_cpu_notifier(&pageset_notifier);
}

#endif

static noinline __init_refok
int zone_wait_table_init(struct zone *zone, unsigned long zone_size_pages)
{
	int i;
	struct pglist_data *pgdat = zone->zone_pgdat;
	size_t alloc_size;

	
	zone->wait_table_hash_nr_entries =
		 wait_table_hash_nr_entries(zone_size_pages);
	zone->wait_table_bits =
		wait_table_bits(zone->wait_table_hash_nr_entries);
	alloc_size = zone->wait_table_hash_nr_entries
					* sizeof(wait_queue_head_t);

	if (!slab_is_available()) {
		zone->wait_table = (wait_queue_head_t *)
			alloc_bootmem_node(pgdat, alloc_size);
	} else {
		
		zone->wait_table = vmalloc(alloc_size);
	}
	if (!zone->wait_table)
		return -ENOMEM;

	for(i = 0; i < zone->wait_table_hash_nr_entries; ++i)
		init_waitqueue_head(zone->wait_table + i);

	return 0;
}

static int __zone_pcp_update(void *data)
{
	struct zone *zone = data;
	int cpu;
	unsigned long batch = zone_batchsize(zone), flags;

	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		struct per_cpu_pageset *pset;
		struct per_cpu_pages *pcp;

		pset = zone_pcp(zone, cpu);
		pcp = &pset->pcp;

		local_irq_save(flags);
		free_pcppages_bulk(zone, pcp->count, pcp);
		setup_pageset(pset, batch);
		local_irq_restore(flags);
	}
	return 0;
}

void zone_pcp_update(struct zone *zone)
{
	stop_machine(__zone_pcp_update, zone, NULL);
}

static __meminit void zone_pcp_init(struct zone *zone)
{
	int cpu;
	unsigned long batch = zone_batchsize(zone);

	for (cpu = 0; cpu < NR_CPUS; cpu++) {
#ifdef CONFIG_NUMA
		
		zone_pcp(zone, cpu) = &boot_pageset[cpu];
		setup_pageset(&boot_pageset[cpu],0);
#else
		setup_pageset(zone_pcp(zone,cpu), batch);
#endif
	}
	if (zone->present_pages)
		printk(KERN_DEBUG "  %s zone: %lu pages, LIFO batch:%lu\n",
			zone->name, zone->present_pages, batch);
}

__meminit int init_currently_empty_zone(struct zone *zone,
					unsigned long zone_start_pfn,
					unsigned long size,
					enum memmap_context context)
{
	struct pglist_data *pgdat = zone->zone_pgdat;
	int ret;
	ret = zone_wait_table_init(zone, size);
	if (ret)
		return ret;
	pgdat->nr_zones = zone_idx(zone) + 1;

	zone->zone_start_pfn = zone_start_pfn;

	mminit_dprintk(MMINIT_TRACE, "memmap_init",
			"Initialising map node %d zone %lu pfns %lu -> %lu\n",
			pgdat->node_id,
			(unsigned long)zone_idx(zone),
			zone_start_pfn, (zone_start_pfn + size));

	zone_init_free_lists(zone);

	return 0;
}

#ifdef CONFIG_ARCH_POPULATES_NODE_MAP

static int __meminit first_active_region_index_in_nid(int nid)
{
	int i;

	for (i = 0; i < nr_nodemap_entries; i++)
		if (nid == MAX_NUMNODES || early_node_map[i].nid == nid)
			return i;

	return -1;
}


static int __meminit next_active_region_index_in_nid(int index, int nid)
{
	for (index = index + 1; index < nr_nodemap_entries; index++)
		if (nid == MAX_NUMNODES || early_node_map[index].nid == nid)
			return index;

	return -1;
}

#ifndef CONFIG_HAVE_ARCH_EARLY_PFN_TO_NID

int __meminit __early_pfn_to_nid(unsigned long pfn)
{
	int i;

	for (i = 0; i < nr_nodemap_entries; i++) {
		unsigned long start_pfn = early_node_map[i].start_pfn;
		unsigned long end_pfn = early_node_map[i].end_pfn;

		if (start_pfn <= pfn && pfn < end_pfn)
			return early_node_map[i].nid;
	}
	
	return -1;
}
#endif 

int __meminit early_pfn_to_nid(unsigned long pfn)
{
	int nid;

	nid = __early_pfn_to_nid(pfn);
	if (nid >= 0)
		return nid;
	
	return 0;
}

#ifdef CONFIG_NODES_SPAN_OTHER_NODES
bool __meminit early_pfn_in_nid(unsigned long pfn, int node)
{
	int nid;

	nid = __early_pfn_to_nid(pfn);
	if (nid >= 0 && nid != node)
		return false;
	return true;
}
#endif


#define for_each_active_range_index_in_nid(i, nid) \
	for (i = first_active_region_index_in_nid(nid); i != -1; \
				i = next_active_region_index_in_nid(i, nid))


void __init free_bootmem_with_active_regions(int nid,
						unsigned long max_low_pfn)
{
	int i;

	for_each_active_range_index_in_nid(i, nid) {
		unsigned long size_pages = 0;
		unsigned long end_pfn = early_node_map[i].end_pfn;

		if (early_node_map[i].start_pfn >= max_low_pfn)
			continue;

		if (end_pfn > max_low_pfn)
			end_pfn = max_low_pfn;

		size_pages = end_pfn - early_node_map[i].start_pfn;
		free_bootmem_node(NODE_DATA(early_node_map[i].nid),
				PFN_PHYS(early_node_map[i].start_pfn),
				size_pages << PAGE_SHIFT);
	}
}

void __init work_with_active_regions(int nid, work_fn_t work_fn, void *data)
{
	int i;
	int ret;

	for_each_active_range_index_in_nid(i, nid) {
		ret = work_fn(early_node_map[i].start_pfn,
			      early_node_map[i].end_pfn, data);
		if (ret)
			break;
	}
}

void __init sparse_memory_present_with_active_regions(int nid)
{
	int i;

	for_each_active_range_index_in_nid(i, nid)
		memory_present(early_node_map[i].nid,
				early_node_map[i].start_pfn,
				early_node_map[i].end_pfn);
}


void __meminit get_pfn_range_for_nid(unsigned int nid,
			unsigned long *start_pfn, unsigned long *end_pfn)
{
	int i;
	*start_pfn = -1UL;
	*end_pfn = 0;

	for_each_active_range_index_in_nid(i, nid) {
		*start_pfn = min(*start_pfn, early_node_map[i].start_pfn);
		*end_pfn = max(*end_pfn, early_node_map[i].end_pfn);
	}

	if (*start_pfn == -1UL)
		*start_pfn = 0;
}


static void __init find_usable_zone_for_movable(void)
{
	int zone_index;
	for (zone_index = MAX_NR_ZONES - 1; zone_index >= 0; zone_index--) {
		if (zone_index == ZONE_MOVABLE)
			continue;

		if (arch_zone_highest_possible_pfn[zone_index] >
				arch_zone_lowest_possible_pfn[zone_index])
			break;
	}

	VM_BUG_ON(zone_index == -1);
	movable_zone = zone_index;
}


static void __meminit adjust_zone_range_for_zone_movable(int nid,
					unsigned long zone_type,
					unsigned long node_start_pfn,
					unsigned long node_end_pfn,
					unsigned long *zone_start_pfn,
					unsigned long *zone_end_pfn)
{
	
	if (zone_movable_pfn[nid]) {
		
		if (zone_type == ZONE_MOVABLE) {
			*zone_start_pfn = zone_movable_pfn[nid];
			*zone_end_pfn = min(node_end_pfn,
				arch_zone_highest_possible_pfn[movable_zone]);

		
		} else if (*zone_start_pfn < zone_movable_pfn[nid] &&
				*zone_end_pfn > zone_movable_pfn[nid]) {
			*zone_end_pfn = zone_movable_pfn[nid];

		
		} else if (*zone_start_pfn >= zone_movable_pfn[nid])
			*zone_start_pfn = *zone_end_pfn;
	}
}


static unsigned long __meminit zone_spanned_pages_in_node(int nid,
					unsigned long zone_type,
					unsigned long *ignored)
{
	unsigned long node_start_pfn, node_end_pfn;
	unsigned long zone_start_pfn, zone_end_pfn;

	
	get_pfn_range_for_nid(nid, &node_start_pfn, &node_end_pfn);
	zone_start_pfn = arch_zone_lowest_possible_pfn[zone_type];
	zone_end_pfn = arch_zone_highest_possible_pfn[zone_type];
	adjust_zone_range_for_zone_movable(nid, zone_type,
				node_start_pfn, node_end_pfn,
				&zone_start_pfn, &zone_end_pfn);

	
	if (zone_end_pfn < node_start_pfn || zone_start_pfn > node_end_pfn)
		return 0;

	
	zone_end_pfn = min(zone_end_pfn, node_end_pfn);
	zone_start_pfn = max(zone_start_pfn, node_start_pfn);

	
	return zone_end_pfn - zone_start_pfn;
}


static unsigned long __meminit __absent_pages_in_range(int nid,
				unsigned long range_start_pfn,
				unsigned long range_end_pfn)
{
	int i = 0;
	unsigned long prev_end_pfn = 0, hole_pages = 0;
	unsigned long start_pfn;

	
	i = first_active_region_index_in_nid(nid);
	if (i == -1)
		return 0;

	prev_end_pfn = min(early_node_map[i].start_pfn, range_end_pfn);

	
	if (early_node_map[i].start_pfn > range_start_pfn)
		hole_pages = prev_end_pfn - range_start_pfn;

	
	for (; i != -1; i = next_active_region_index_in_nid(i, nid)) {

		
		if (prev_end_pfn >= range_end_pfn)
			break;

		
		start_pfn = min(early_node_map[i].start_pfn, range_end_pfn);
		prev_end_pfn = max(prev_end_pfn, range_start_pfn);

		
		if (start_pfn > range_start_pfn) {
			BUG_ON(prev_end_pfn > start_pfn);
			hole_pages += start_pfn - prev_end_pfn;
		}
		prev_end_pfn = early_node_map[i].end_pfn;
	}

	
	if (range_end_pfn > prev_end_pfn)
		hole_pages += range_end_pfn -
				max(range_start_pfn, prev_end_pfn);

	return hole_pages;
}


unsigned long __init absent_pages_in_range(unsigned long start_pfn,
							unsigned long end_pfn)
{
	return __absent_pages_in_range(MAX_NUMNODES, start_pfn, end_pfn);
}


static unsigned long __meminit zone_absent_pages_in_node(int nid,
					unsigned long zone_type,
					unsigned long *ignored)
{
	unsigned long node_start_pfn, node_end_pfn;
	unsigned long zone_start_pfn, zone_end_pfn;

	get_pfn_range_for_nid(nid, &node_start_pfn, &node_end_pfn);
	zone_start_pfn = max(arch_zone_lowest_possible_pfn[zone_type],
							node_start_pfn);
	zone_end_pfn = min(arch_zone_highest_possible_pfn[zone_type],
							node_end_pfn);

	adjust_zone_range_for_zone_movable(nid, zone_type,
			node_start_pfn, node_end_pfn,
			&zone_start_pfn, &zone_end_pfn);
	return __absent_pages_in_range(nid, zone_start_pfn, zone_end_pfn);
}

#else
static inline unsigned long __meminit zone_spanned_pages_in_node(int nid,
					unsigned long zone_type,
					unsigned long *zones_size)
{
	return zones_size[zone_type];
}

static inline unsigned long __meminit zone_absent_pages_in_node(int nid,
						unsigned long zone_type,
						unsigned long *zholes_size)
{
	if (!zholes_size)
		return 0;

	return zholes_size[zone_type];
}

#endif

static void __meminit calculate_node_totalpages(struct pglist_data *pgdat,
		unsigned long *zones_size, unsigned long *zholes_size)
{
	unsigned long realtotalpages, totalpages = 0;
	enum zone_type i;

	for (i = 0; i < MAX_NR_ZONES; i++)
		totalpages += zone_spanned_pages_in_node(pgdat->node_id, i,
								zones_size);
	pgdat->node_spanned_pages = totalpages;

	realtotalpages = totalpages;
	for (i = 0; i < MAX_NR_ZONES; i++)
		realtotalpages -=
			zone_absent_pages_in_node(pgdat->node_id, i,
								zholes_size);
	pgdat->node_present_pages = realtotalpages;
	printk(KERN_DEBUG "On node %d totalpages: %lu\n", pgdat->node_id,
							realtotalpages);
}

#ifndef CONFIG_SPARSEMEM

static unsigned long __init usemap_size(unsigned long zonesize)
{
	unsigned long usemapsize;

	usemapsize = roundup(zonesize, pageblock_nr_pages);
	usemapsize = usemapsize >> pageblock_order;
	usemapsize *= NR_PAGEBLOCK_BITS;
	usemapsize = roundup(usemapsize, 8 * sizeof(unsigned long));

	return usemapsize / 8;
}

static void __init setup_usemap(struct pglist_data *pgdat,
				struct zone *zone, unsigned long zonesize)
{
	unsigned long usemapsize = usemap_size(zonesize);
	zone->pageblock_flags = NULL;
	if (usemapsize)
		zone->pageblock_flags = alloc_bootmem_node(pgdat, usemapsize);
}
#else
static void inline setup_usemap(struct pglist_data *pgdat,
				struct zone *zone, unsigned long zonesize) {}
#endif 

#ifdef CONFIG_HUGETLB_PAGE_SIZE_VARIABLE


static inline int pageblock_default_order(void)
{
	if (HPAGE_SHIFT > PAGE_SHIFT)
		return HUGETLB_PAGE_ORDER;

	return MAX_ORDER-1;
}


static inline void __init set_pageblock_order(unsigned int order)
{
	
	if (pageblock_order)
		return;

	
	pageblock_order = order;
}
#else 


static inline int pageblock_default_order(unsigned int order)
{
	return MAX_ORDER-1;
}
#define set_pageblock_order(x)	do {} while (0)

#endif 


static void __paginginit free_area_init_core(struct pglist_data *pgdat,
		unsigned long *zones_size, unsigned long *zholes_size)
{
	enum zone_type j;
	int nid = pgdat->node_id;
	unsigned long zone_start_pfn = pgdat->node_start_pfn;
	int ret;

	pgdat_resize_init(pgdat);
	pgdat->nr_zones = 0;
	init_waitqueue_head(&pgdat->kswapd_wait);
	pgdat->kswapd_max_order = 0;
	pgdat_page_cgroup_init(pgdat);
	
	for (j = 0; j < MAX_NR_ZONES; j++) {
		struct zone *zone = pgdat->node_zones + j;
		unsigned long size, realsize, memmap_pages;
		enum lru_list l;

		size = zone_spanned_pages_in_node(nid, j, zones_size);
		realsize = size - zone_absent_pages_in_node(nid, j,
								zholes_size);

		
		memmap_pages =
			PAGE_ALIGN(size * sizeof(struct page)) >> PAGE_SHIFT;
		if (realsize >= memmap_pages) {
			realsize -= memmap_pages;
			if (memmap_pages)
				printk(KERN_DEBUG
				       "  %s zone: %lu pages used for memmap\n",
				       zone_names[j], memmap_pages);
		} else
			printk(KERN_WARNING
				"  %s zone: %lu pages exceeds realsize %lu\n",
				zone_names[j], memmap_pages, realsize);

		
		if (j == 0 && realsize > dma_reserve) {
			realsize -= dma_reserve;
			printk(KERN_DEBUG "  %s zone: %lu pages reserved\n",
					zone_names[0], dma_reserve);
		}

		if (!is_highmem_idx(j))
			nr_kernel_pages += realsize;
		nr_all_pages += realsize;

		zone->spanned_pages = size;
		zone->present_pages = realsize;
#ifdef CONFIG_NUMA
		zone->node = nid;
		zone->min_unmapped_pages = (realsize*sysctl_min_unmapped_ratio)
						/ 100;
		zone->min_slab_pages = (realsize * sysctl_min_slab_ratio) / 100;
#endif
		zone->name = zone_names[j];
		spin_lock_init(&zone->lock);
		spin_lock_init(&zone->lru_lock);
		zone_seqlock_init(zone);
		zone->zone_pgdat = pgdat;

		zone->prev_priority = DEF_PRIORITY;

		zone_pcp_init(zone);
		for_each_lru(l) {
			INIT_LIST_HEAD(&zone->lru[l].list);
			zone->reclaim_stat.nr_saved_scan[l] = 0;
		}
		zone->reclaim_stat.recent_rotated[0] = 0;
		zone->reclaim_stat.recent_rotated[1] = 0;
		zone->reclaim_stat.recent_scanned[0] = 0;
		zone->reclaim_stat.recent_scanned[1] = 0;
		zap_zone_vm_stats(zone);
		zone->flags = 0;
		if (!size)
			continue;

		set_pageblock_order(pageblock_default_order());
		setup_usemap(pgdat, zone, size);
		ret = init_currently_empty_zone(zone, zone_start_pfn,
						size, MEMMAP_EARLY);
		BUG_ON(ret);
		memmap_init(size, nid, j, zone_start_pfn);
		zone_start_pfn += size;
	}
}

static void __init_refok alloc_node_mem_map(struct pglist_data *pgdat)
{
	
	if (!pgdat->node_spanned_pages)
		return;

#ifdef CONFIG_FLAT_NODE_MEM_MAP
	
	if (!pgdat->node_mem_map) {
		unsigned long size, start, end;
		struct page *map;

		
		start = pgdat->node_start_pfn & ~(MAX_ORDER_NR_PAGES - 1);
		end = pgdat->node_start_pfn + pgdat->node_spanned_pages;
		end = ALIGN(end, MAX_ORDER_NR_PAGES);
		size =  (end - start) * sizeof(struct page);
		map = alloc_remap(pgdat->node_id, size);
		if (!map)
			map = alloc_bootmem_node(pgdat, size);
		pgdat->node_mem_map = map + (pgdat->node_start_pfn - start);
	}
#ifndef CONFIG_NEED_MULTIPLE_NODES
	
	if (pgdat == NODE_DATA(0)) {
		mem_map = NODE_DATA(0)->node_mem_map;
#ifdef CONFIG_ARCH_POPULATES_NODE_MAP
		if (page_to_pfn(mem_map) != pgdat->node_start_pfn)
			mem_map -= (pgdat->node_start_pfn - ARCH_PFN_OFFSET);
#endif 
	}
#endif
#endif 
}

void __paginginit free_area_init_node(int nid, unsigned long *zones_size,
		unsigned long node_start_pfn, unsigned long *zholes_size)
{
	pg_data_t *pgdat = NODE_DATA(nid);

	pgdat->node_id = nid;
	pgdat->node_start_pfn = node_start_pfn;
	calculate_node_totalpages(pgdat, zones_size, zholes_size);

	alloc_node_mem_map(pgdat);
#ifdef CONFIG_FLAT_NODE_MEM_MAP
	printk(KERN_DEBUG "free_area_init_node: node %d, pgdat %08lx, node_mem_map %08lx\n",
		nid, (unsigned long)pgdat,
		(unsigned long)pgdat->node_mem_map);
#endif

	free_area_init_core(pgdat, zones_size, zholes_size);
}

#ifdef CONFIG_ARCH_POPULATES_NODE_MAP

#if MAX_NUMNODES > 1

static void __init setup_nr_node_ids(void)
{
	unsigned int node;
	unsigned int highest = 0;

	for_each_node_mask(node, node_possible_map)
		highest = node;
	nr_node_ids = highest + 1;
}
#else
static inline void setup_nr_node_ids(void)
{
}
#endif


void __init add_active_range(unsigned int nid, unsigned long start_pfn,
						unsigned long end_pfn)
{
	int i;

	mminit_dprintk(MMINIT_TRACE, "memory_register",
			"Entering add_active_range(%d, %#lx, %#lx) "
			"%d entries of %d used\n",
			nid, start_pfn, end_pfn,
			nr_nodemap_entries, MAX_ACTIVE_REGIONS);

	mminit_validate_memmodel_limits(&start_pfn, &end_pfn);

	
	for (i = 0; i < nr_nodemap_entries; i++) {
		if (early_node_map[i].nid != nid)
			continue;

		
		if (start_pfn >= early_node_map[i].start_pfn &&
				end_pfn <= early_node_map[i].end_pfn)
			return;

		
		if (start_pfn <= early_node_map[i].end_pfn &&
				end_pfn > early_node_map[i].end_pfn) {
			early_node_map[i].end_pfn = end_pfn;
			return;
		}

		
		if (start_pfn < early_node_map[i].end_pfn &&
				end_pfn >= early_node_map[i].start_pfn) {
			early_node_map[i].start_pfn = start_pfn;
			return;
		}
	}

	
	if (i >= MAX_ACTIVE_REGIONS) {
		printk(KERN_CRIT "More than %d memory regions, truncating\n",
							MAX_ACTIVE_REGIONS);
		return;
	}

	early_node_map[i].nid = nid;
	early_node_map[i].start_pfn = start_pfn;
	early_node_map[i].end_pfn = end_pfn;
	nr_nodemap_entries = i + 1;
}


void __init remove_active_range(unsigned int nid, unsigned long start_pfn,
				unsigned long end_pfn)
{
	int i, j;
	int removed = 0;

	printk(KERN_DEBUG "remove_active_range (%d, %lu, %lu)\n",
			  nid, start_pfn, end_pfn);

	
	for_each_active_range_index_in_nid(i, nid) {
		if (early_node_map[i].start_pfn >= start_pfn &&
		    early_node_map[i].end_pfn <= end_pfn) {
			
			early_node_map[i].start_pfn = 0;
			early_node_map[i].end_pfn = 0;
			removed = 1;
			continue;
		}
		if (early_node_map[i].start_pfn < start_pfn &&
		    early_node_map[i].end_pfn > start_pfn) {
			unsigned long temp_end_pfn = early_node_map[i].end_pfn;
			early_node_map[i].end_pfn = start_pfn;
			if (temp_end_pfn > end_pfn)
				add_active_range(nid, end_pfn, temp_end_pfn);
			continue;
		}
		if (early_node_map[i].start_pfn >= start_pfn &&
		    early_node_map[i].end_pfn > end_pfn &&
		    early_node_map[i].start_pfn < end_pfn) {
			early_node_map[i].start_pfn = end_pfn;
			continue;
		}
	}

	if (!removed)
		return;

	
	for (i = nr_nodemap_entries - 1; i > 0; i--) {
		if (early_node_map[i].nid != nid)
			continue;
		if (early_node_map[i].end_pfn)
			continue;
		
		for (j = i; j < nr_nodemap_entries - 1; j++)
			memcpy(&early_node_map[j], &early_node_map[j+1],
				sizeof(early_node_map[j]));
		j = nr_nodemap_entries - 1;
		memset(&early_node_map[j], 0, sizeof(early_node_map[j]));
		nr_nodemap_entries--;
	}
}


void __init remove_all_active_ranges(void)
{
	memset(early_node_map, 0, sizeof(early_node_map));
	nr_nodemap_entries = 0;
}


static int __init cmp_node_active_region(const void *a, const void *b)
{
	struct node_active_region *arange = (struct node_active_region *)a;
	struct node_active_region *brange = (struct node_active_region *)b;

	
	if (arange->start_pfn > brange->start_pfn)
		return 1;
	if (arange->start_pfn < brange->start_pfn)
		return -1;

	return 0;
}


static void __init sort_node_map(void)
{
	sort(early_node_map, (size_t)nr_nodemap_entries,
			sizeof(struct node_active_region),
			cmp_node_active_region, NULL);
}


static unsigned long __init find_min_pfn_for_node(int nid)
{
	int i;
	unsigned long min_pfn = ULONG_MAX;

	
	for_each_active_range_index_in_nid(i, nid)
		min_pfn = min(min_pfn, early_node_map[i].start_pfn);

	if (min_pfn == ULONG_MAX) {
		printk(KERN_WARNING
			"Could not find start_pfn for node %d\n", nid);
		return 0;
	}

	return min_pfn;
}


unsigned long __init find_min_pfn_with_active_regions(void)
{
	return find_min_pfn_for_node(MAX_NUMNODES);
}


static unsigned long __init early_calculate_totalpages(void)
{
	int i;
	unsigned long totalpages = 0;

	for (i = 0; i < nr_nodemap_entries; i++) {
		unsigned long pages = early_node_map[i].end_pfn -
						early_node_map[i].start_pfn;
		totalpages += pages;
		if (pages)
			node_set_state(early_node_map[i].nid, N_HIGH_MEMORY);
	}
  	return totalpages;
}


static void __init find_zone_movable_pfns_for_nodes(unsigned long *movable_pfn)
{
	int i, nid;
	unsigned long usable_startpfn;
	unsigned long kernelcore_node, kernelcore_remaining;
	
	nodemask_t saved_node_state = node_states[N_HIGH_MEMORY];
	unsigned long totalpages = early_calculate_totalpages();
	int usable_nodes = nodes_weight(node_states[N_HIGH_MEMORY]);

	
	if (required_movablecore) {
		unsigned long corepages;

		
		required_movablecore =
			roundup(required_movablecore, MAX_ORDER_NR_PAGES);
		corepages = totalpages - required_movablecore;

		required_kernelcore = max(required_kernelcore, corepages);
	}

	
	if (!required_kernelcore)
		goto out;

	
	find_usable_zone_for_movable();
	usable_startpfn = arch_zone_lowest_possible_pfn[movable_zone];

restart:
	
	kernelcore_node = required_kernelcore / usable_nodes;
	for_each_node_state(nid, N_HIGH_MEMORY) {
		
		if (required_kernelcore < kernelcore_node)
			kernelcore_node = required_kernelcore / usable_nodes;

		
		kernelcore_remaining = kernelcore_node;

		
		for_each_active_range_index_in_nid(i, nid) {
			unsigned long start_pfn, end_pfn;
			unsigned long size_pages;

			start_pfn = max(early_node_map[i].start_pfn,
						zone_movable_pfn[nid]);
			end_pfn = early_node_map[i].end_pfn;
			if (start_pfn >= end_pfn)
				continue;

			
			if (start_pfn < usable_startpfn) {
				unsigned long kernel_pages;
				kernel_pages = min(end_pfn, usable_startpfn)
								- start_pfn;

				kernelcore_remaining -= min(kernel_pages,
							kernelcore_remaining);
				required_kernelcore -= min(kernel_pages,
							required_kernelcore);

				
				if (end_pfn <= usable_startpfn) {

					
					zone_movable_pfn[nid] = end_pfn;
					continue;
				}
				start_pfn = usable_startpfn;
			}

			
			size_pages = end_pfn - start_pfn;
			if (size_pages > kernelcore_remaining)
				size_pages = kernelcore_remaining;
			zone_movable_pfn[nid] = start_pfn + size_pages;

			
			required_kernelcore -= min(required_kernelcore,
								size_pages);
			kernelcore_remaining -= size_pages;
			if (!kernelcore_remaining)
				break;
		}
	}

	
	usable_nodes--;
	if (usable_nodes && required_kernelcore > usable_nodes)
		goto restart;

	
	for (nid = 0; nid < MAX_NUMNODES; nid++)
		zone_movable_pfn[nid] =
			roundup(zone_movable_pfn[nid], MAX_ORDER_NR_PAGES);

out:
	
	node_states[N_HIGH_MEMORY] = saved_node_state;
}


static void check_for_regular_memory(pg_data_t *pgdat)
{
#ifdef CONFIG_HIGHMEM
	enum zone_type zone_type;

	for (zone_type = 0; zone_type <= ZONE_NORMAL; zone_type++) {
		struct zone *zone = &pgdat->node_zones[zone_type];
		if (zone->present_pages)
			node_set_state(zone_to_nid(zone), N_NORMAL_MEMORY);
	}
#endif
}


void __init free_area_init_nodes(unsigned long *max_zone_pfn)
{
	unsigned long nid;
	int i;

	
	sort_node_map();

	
	memset(arch_zone_lowest_possible_pfn, 0,
				sizeof(arch_zone_lowest_possible_pfn));
	memset(arch_zone_highest_possible_pfn, 0,
				sizeof(arch_zone_highest_possible_pfn));
	arch_zone_lowest_possible_pfn[0] = find_min_pfn_with_active_regions();
	arch_zone_highest_possible_pfn[0] = max_zone_pfn[0];
	for (i = 1; i < MAX_NR_ZONES; i++) {
		if (i == ZONE_MOVABLE)
			continue;
		arch_zone_lowest_possible_pfn[i] =
			arch_zone_highest_possible_pfn[i-1];
		arch_zone_highest_possible_pfn[i] =
			max(max_zone_pfn[i], arch_zone_lowest_possible_pfn[i]);
	}
	arch_zone_lowest_possible_pfn[ZONE_MOVABLE] = 0;
	arch_zone_highest_possible_pfn[ZONE_MOVABLE] = 0;

	
	memset(zone_movable_pfn, 0, sizeof(zone_movable_pfn));
	find_zone_movable_pfns_for_nodes(zone_movable_pfn);

	
	printk("Zone PFN ranges:\n");
	for (i = 0; i < MAX_NR_ZONES; i++) {
		if (i == ZONE_MOVABLE)
			continue;
		printk("  %-8s %0#10lx -> %0#10lx\n",
				zone_names[i],
				arch_zone_lowest_possible_pfn[i],
				arch_zone_highest_possible_pfn[i]);
	}

	
	printk("Movable zone start PFN for each node\n");
	for (i = 0; i < MAX_NUMNODES; i++) {
		if (zone_movable_pfn[i])
			printk("  Node %d: %lu\n", i, zone_movable_pfn[i]);
	}

	
	printk("early_node_map[%d] active PFN ranges\n", nr_nodemap_entries);
	for (i = 0; i < nr_nodemap_entries; i++)
		printk("  %3d: %0#10lx -> %0#10lx\n", early_node_map[i].nid,
						early_node_map[i].start_pfn,
						early_node_map[i].end_pfn);

	
	mminit_verify_pageflags_layout();
	setup_nr_node_ids();
	for_each_online_node(nid) {
		pg_data_t *pgdat = NODE_DATA(nid);
		free_area_init_node(nid, NULL,
				find_min_pfn_for_node(nid), NULL);

		
		if (pgdat->node_present_pages)
			node_set_state(nid, N_HIGH_MEMORY);
		check_for_regular_memory(pgdat);
	}
}

static int __init cmdline_parse_core(char *p, unsigned long *core)
{
	unsigned long long coremem;
	if (!p)
		return -EINVAL;

	coremem = memparse(p, &p);
	*core = coremem >> PAGE_SHIFT;

	
	WARN_ON((coremem >> PAGE_SHIFT) > ULONG_MAX);

	return 0;
}


static int __init cmdline_parse_kernelcore(char *p)
{
	return cmdline_parse_core(p, &required_kernelcore);
}


static int __init cmdline_parse_movablecore(char *p)
{
	return cmdline_parse_core(p, &required_movablecore);
}

early_param("kernelcore", cmdline_parse_kernelcore);
early_param("movablecore", cmdline_parse_movablecore);

#endif 


void __init set_dma_reserve(unsigned long new_dma_reserve)
{
	dma_reserve = new_dma_reserve;
}

#ifndef CONFIG_NEED_MULTIPLE_NODES
struct pglist_data __refdata contig_page_data = { .bdata = &bootmem_node_data[0] };
EXPORT_SYMBOL(contig_page_data);
#endif

void __init free_area_init(unsigned long *zones_size)
{
	free_area_init_node(0, zones_size,
			__pa(PAGE_OFFSET) >> PAGE_SHIFT, NULL);
}

static int page_alloc_cpu_notify(struct notifier_block *self,
				 unsigned long action, void *hcpu)
{
	int cpu = (unsigned long)hcpu;

	if (action == CPU_DEAD || action == CPU_DEAD_FROZEN) {
		drain_pages(cpu);

		
		vm_events_fold_cpu(cpu);

		
		refresh_cpu_vm_stats(cpu);
	}
	return NOTIFY_OK;
}

void __init page_alloc_init(void)
{
	hotcpu_notifier(page_alloc_cpu_notify, 0);
}


static void calculate_totalreserve_pages(void)
{
	struct pglist_data *pgdat;
	unsigned long reserve_pages = 0;
	enum zone_type i, j;

	for_each_online_pgdat(pgdat) {
		for (i = 0; i < MAX_NR_ZONES; i++) {
			struct zone *zone = pgdat->node_zones + i;
			unsigned long max = 0;

			
			for (j = i; j < MAX_NR_ZONES; j++) {
				if (zone->lowmem_reserve[j] > max)
					max = zone->lowmem_reserve[j];
			}

			
			max += high_wmark_pages(zone);

			if (max > zone->present_pages)
				max = zone->present_pages;
			reserve_pages += max;
		}
	}
	totalreserve_pages = reserve_pages;
}


static void setup_per_zone_lowmem_reserve(void)
{
	struct pglist_data *pgdat;
	enum zone_type j, idx;

	for_each_online_pgdat(pgdat) {
		for (j = 0; j < MAX_NR_ZONES; j++) {
			struct zone *zone = pgdat->node_zones + j;
			unsigned long present_pages = zone->present_pages;

			zone->lowmem_reserve[j] = 0;

			idx = j;
			while (idx) {
				struct zone *lower_zone;

				idx--;

				if (sysctl_lowmem_reserve_ratio[idx] < 1)
					sysctl_lowmem_reserve_ratio[idx] = 1;

				lower_zone = pgdat->node_zones + idx;
				lower_zone->lowmem_reserve[j] = present_pages /
					sysctl_lowmem_reserve_ratio[idx];
				present_pages += lower_zone->present_pages;
			}
		}
	}

	
	calculate_totalreserve_pages();
}


void setup_per_zone_wmarks(void)
{
	unsigned long pages_min = min_free_kbytes >> (PAGE_SHIFT - 10);
	unsigned long lowmem_pages = 0;
	struct zone *zone;
	unsigned long flags;

	
	for_each_zone(zone) {
		if (!is_highmem(zone))
			lowmem_pages += zone->present_pages;
	}

	for_each_zone(zone) {
		u64 tmp;

		spin_lock_irqsave(&zone->lock, flags);
		tmp = (u64)pages_min * zone->present_pages;
		do_div(tmp, lowmem_pages);
		if (is_highmem(zone)) {
			
			int min_pages;

			min_pages = zone->present_pages / 1024;
			if (min_pages < SWAP_CLUSTER_MAX)
				min_pages = SWAP_CLUSTER_MAX;
			if (min_pages > 128)
				min_pages = 128;
			zone->watermark[WMARK_MIN] = min_pages;
		} else {
			
			zone->watermark[WMARK_MIN] = tmp;
		}

		zone->watermark[WMARK_LOW]  = min_wmark_pages(zone) + (tmp >> 2);
		zone->watermark[WMARK_HIGH] = min_wmark_pages(zone) + (tmp >> 1);
		setup_zone_migrate_reserve(zone);
		spin_unlock_irqrestore(&zone->lock, flags);
	}

	
	calculate_totalreserve_pages();
}


void calculate_zone_inactive_ratio(struct zone *zone)
{
	unsigned int gb, ratio;

	
	gb = zone->present_pages >> (30 - PAGE_SHIFT);
	if (gb)
		ratio = int_sqrt(10 * gb);
	else
		ratio = 1;

	zone->inactive_ratio = ratio;
}

static void __init setup_per_zone_inactive_ratio(void)
{
	struct zone *zone;

	for_each_zone(zone)
		calculate_zone_inactive_ratio(zone);
}


static int __init init_per_zone_wmark_min(void)
{
	unsigned long lowmem_kbytes;

	lowmem_kbytes = nr_free_buffer_pages() * (PAGE_SIZE >> 10);

	min_free_kbytes = int_sqrt(lowmem_kbytes * 16);
	if (min_free_kbytes < 128)
		min_free_kbytes = 128;
	if (min_free_kbytes > 65536)
		min_free_kbytes = 65536;
	setup_per_zone_wmarks();
	setup_per_zone_lowmem_reserve();
	setup_per_zone_inactive_ratio();
	return 0;
}
module_init(init_per_zone_wmark_min)


int min_free_kbytes_sysctl_handler(ctl_table *table, int write, 
	void __user *buffer, size_t *length, loff_t *ppos)
{
	proc_dointvec(table, write, buffer, length, ppos);
	if (write)
		setup_per_zone_wmarks();
	return 0;
}

#ifdef CONFIG_NUMA
int sysctl_min_unmapped_ratio_sysctl_handler(ctl_table *table, int write,
	void __user *buffer, size_t *length, loff_t *ppos)
{
	struct zone *zone;
	int rc;

	rc = proc_dointvec_minmax(table, write, buffer, length, ppos);
	if (rc)
		return rc;

	for_each_zone(zone)
		zone->min_unmapped_pages = (zone->present_pages *
				sysctl_min_unmapped_ratio) / 100;
	return 0;
}

int sysctl_min_slab_ratio_sysctl_handler(ctl_table *table, int write,
	void __user *buffer, size_t *length, loff_t *ppos)
{
	struct zone *zone;
	int rc;

	rc = proc_dointvec_minmax(table, write, buffer, length, ppos);
	if (rc)
		return rc;

	for_each_zone(zone)
		zone->min_slab_pages = (zone->present_pages *
				sysctl_min_slab_ratio) / 100;
	return 0;
}
#endif


int lowmem_reserve_ratio_sysctl_handler(ctl_table *table, int write,
	void __user *buffer, size_t *length, loff_t *ppos)
{
	proc_dointvec_minmax(table, write, buffer, length, ppos);
	setup_per_zone_lowmem_reserve();
	return 0;
}



int percpu_pagelist_fraction_sysctl_handler(ctl_table *table, int write,
	void __user *buffer, size_t *length, loff_t *ppos)
{
	struct zone *zone;
	unsigned int cpu;
	int ret;

	ret = proc_dointvec_minmax(table, write, buffer, length, ppos);
	if (!write || (ret == -EINVAL))
		return ret;
	for_each_populated_zone(zone) {
		for_each_online_cpu(cpu) {
			unsigned long  high;
			high = zone->present_pages / percpu_pagelist_fraction;
			setup_pagelist_highmark(zone_pcp(zone, cpu), high);
		}
	}
	return 0;
}

int hashdist = HASHDIST_DEFAULT;

#ifdef CONFIG_NUMA
static int __init set_hashdist(char *str)
{
	if (!str)
		return 0;
	hashdist = simple_strtoul(str, &str, 0);
	return 1;
}
__setup("hashdist=", set_hashdist);
#endif


void *__init alloc_large_system_hash(const char *tablename,
				     unsigned long bucketsize,
				     unsigned long numentries,
				     int scale,
				     int flags,
				     unsigned int *_hash_shift,
				     unsigned int *_hash_mask,
				     unsigned long limit)
{
	unsigned long long max = limit;
	unsigned long log2qty, size;
	void *table = NULL;

	
	if (!numentries) {
		
		numentries = nr_kernel_pages;
		numentries += (1UL << (20 - PAGE_SHIFT)) - 1;
		numentries >>= 20 - PAGE_SHIFT;
		numentries <<= 20 - PAGE_SHIFT;

		
		if (scale > PAGE_SHIFT)
			numentries >>= (scale - PAGE_SHIFT);
		else
			numentries <<= (PAGE_SHIFT - scale);

		
		if (unlikely(flags & HASH_SMALL)) {
			
			WARN_ON(!(flags & HASH_EARLY));
			if (!(numentries >> *_hash_shift)) {
				numentries = 1UL << *_hash_shift;
				BUG_ON(!numentries);
			}
		} else if (unlikely((numentries * bucketsize) < PAGE_SIZE))
			numentries = PAGE_SIZE / bucketsize;
	}
	numentries = roundup_pow_of_two(numentries);

	
	if (max == 0) {
		max = ((unsigned long long)nr_all_pages << PAGE_SHIFT) >> 4;
		do_div(max, bucketsize);
	}

	if (numentries > max)
		numentries = max;

	log2qty = ilog2(numentries);

	do {
		size = bucketsize << log2qty;
		if (flags & HASH_EARLY)
			table = alloc_bootmem_nopanic(size);
		else if (hashdist)
			table = __vmalloc(size, GFP_ATOMIC, PAGE_KERNEL);
		else {
			
			if (get_order(size) < MAX_ORDER) {
				table = alloc_pages_exact(size, GFP_ATOMIC);
				kmemleak_alloc(table, size, 1, GFP_ATOMIC);
			}
		}
	} while (!table && size > PAGE_SIZE && --log2qty);

	if (!table)
		panic("Failed to allocate %s hash table\n", tablename);

	printk(KERN_INFO "%s hash table entries: %d (order: %d, %lu bytes)\n",
	       tablename,
	       (1U << log2qty),
	       ilog2(size) - PAGE_SHIFT,
	       size);

	if (_hash_shift)
		*_hash_shift = log2qty;
	if (_hash_mask)
		*_hash_mask = (1 << log2qty) - 1;

	return table;
}


static inline unsigned long *get_pageblock_bitmap(struct zone *zone,
							unsigned long pfn)
{
#ifdef CONFIG_SPARSEMEM
	return __pfn_to_section(pfn)->pageblock_flags;
#else
	return zone->pageblock_flags;
#endif 
}

static inline int pfn_to_bitidx(struct zone *zone, unsigned long pfn)
{
#ifdef CONFIG_SPARSEMEM
	pfn &= (PAGES_PER_SECTION-1);
	return (pfn >> pageblock_order) * NR_PAGEBLOCK_BITS;
#else
	pfn = pfn - zone->zone_start_pfn;
	return (pfn >> pageblock_order) * NR_PAGEBLOCK_BITS;
#endif 
}


unsigned long get_pageblock_flags_group(struct page *page,
					int start_bitidx, int end_bitidx)
{
	struct zone *zone;
	unsigned long *bitmap;
	unsigned long pfn, bitidx;
	unsigned long flags = 0;
	unsigned long value = 1;

	zone = page_zone(page);
	pfn = page_to_pfn(page);
	bitmap = get_pageblock_bitmap(zone, pfn);
	bitidx = pfn_to_bitidx(zone, pfn);

	for (; start_bitidx <= end_bitidx; start_bitidx++, value <<= 1)
		if (test_bit(bitidx + start_bitidx, bitmap))
			flags |= value;

	return flags;
}


void set_pageblock_flags_group(struct page *page, unsigned long flags,
					int start_bitidx, int end_bitidx)
{
	struct zone *zone;
	unsigned long *bitmap;
	unsigned long pfn, bitidx;
	unsigned long value = 1;

	zone = page_zone(page);
	pfn = page_to_pfn(page);
	bitmap = get_pageblock_bitmap(zone, pfn);
	bitidx = pfn_to_bitidx(zone, pfn);
	VM_BUG_ON(pfn < zone->zone_start_pfn);
	VM_BUG_ON(pfn >= zone->zone_start_pfn + zone->spanned_pages);

	for (; start_bitidx <= end_bitidx; start_bitidx++, value <<= 1)
		if (flags & value)
			__set_bit(bitidx + start_bitidx, bitmap);
		else
			__clear_bit(bitidx + start_bitidx, bitmap);
}



int set_migratetype_isolate(struct page *page)
{
	struct zone *zone;
	unsigned long flags;
	int ret = -EBUSY;
	int zone_idx;

	zone = page_zone(page);
	zone_idx = zone_idx(zone);
	spin_lock_irqsave(&zone->lock, flags);
	
	if (get_pageblock_migratetype(page) != MIGRATE_MOVABLE &&
	    zone_idx != ZONE_MOVABLE)
		goto out;
	set_pageblock_migratetype(page, MIGRATE_ISOLATE);
	move_freepages_block(zone, page, MIGRATE_ISOLATE);
	ret = 0;
out:
	spin_unlock_irqrestore(&zone->lock, flags);
	if (!ret)
		drain_all_pages();
	return ret;
}

void unset_migratetype_isolate(struct page *page)
{
	struct zone *zone;
	unsigned long flags;
	zone = page_zone(page);
	spin_lock_irqsave(&zone->lock, flags);
	if (get_pageblock_migratetype(page) != MIGRATE_ISOLATE)
		goto out;
	set_pageblock_migratetype(page, MIGRATE_MOVABLE);
	move_freepages_block(zone, page, MIGRATE_MOVABLE);
out:
	spin_unlock_irqrestore(&zone->lock, flags);
}

#ifdef CONFIG_MEMORY_HOTREMOVE

void
__offline_isolated_pages(unsigned long start_pfn, unsigned long end_pfn)
{
	struct page *page;
	struct zone *zone;
	int order, i;
	unsigned long pfn;
	unsigned long flags;
	
	for (pfn = start_pfn; pfn < end_pfn; pfn++)
		if (pfn_valid(pfn))
			break;
	if (pfn == end_pfn)
		return;
	zone = page_zone(pfn_to_page(pfn));
	spin_lock_irqsave(&zone->lock, flags);
	pfn = start_pfn;
	while (pfn < end_pfn) {
		if (!pfn_valid(pfn)) {
			pfn++;
			continue;
		}
		page = pfn_to_page(pfn);
		BUG_ON(page_count(page));
		BUG_ON(!PageBuddy(page));
		order = page_order(page);
#ifdef CONFIG_DEBUG_VM
		printk(KERN_INFO "remove from free list %lx %d %lx\n",
		       pfn, 1 << order, end_pfn);
#endif
		list_del(&page->lru);
		rmv_page_order(page);
		zone->free_area[order].nr_free--;
		__mod_zone_page_state(zone, NR_FREE_PAGES,
				      - (1UL << order));
		for (i = 0; i < (1 << order); i++)
			SetPageReserved((page+i));
		pfn += (1 << order);
	}
	spin_unlock_irqrestore(&zone->lock, flags);
}
#endif
