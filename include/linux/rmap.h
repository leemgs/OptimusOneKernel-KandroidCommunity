#ifndef _LINUX_RMAP_H
#define _LINUX_RMAP_H


#include <linux/list.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/spinlock.h>
#include <linux/memcontrol.h>


struct anon_vma {
	spinlock_t lock;	
	
	struct list_head head;	
};

#ifdef CONFIG_MMU

static inline void anon_vma_lock(struct vm_area_struct *vma)
{
	struct anon_vma *anon_vma = vma->anon_vma;
	if (anon_vma)
		spin_lock(&anon_vma->lock);
}

static inline void anon_vma_unlock(struct vm_area_struct *vma)
{
	struct anon_vma *anon_vma = vma->anon_vma;
	if (anon_vma)
		spin_unlock(&anon_vma->lock);
}


void anon_vma_init(void);	
int  anon_vma_prepare(struct vm_area_struct *);
void __anon_vma_merge(struct vm_area_struct *, struct vm_area_struct *);
void anon_vma_unlink(struct vm_area_struct *);
void anon_vma_link(struct vm_area_struct *);
void __anon_vma_link(struct vm_area_struct *);


void page_add_anon_rmap(struct page *, struct vm_area_struct *, unsigned long);
void page_add_new_anon_rmap(struct page *, struct vm_area_struct *, unsigned long);
void page_add_file_rmap(struct page *);
void page_remove_rmap(struct page *);

static inline void page_dup_rmap(struct page *page)
{
	atomic_inc(&page->_mapcount);
}


int page_referenced(struct page *, int is_locked,
			struct mem_cgroup *cnt, unsigned long *vm_flags);
enum ttu_flags {
	TTU_UNMAP = 0,			
	TTU_MIGRATION = 1,		
	TTU_MUNLOCK = 2,		
	TTU_ACTION_MASK = 0xff,

	TTU_IGNORE_MLOCK = (1 << 8),	
	TTU_IGNORE_ACCESS = (1 << 9),	
	TTU_IGNORE_HWPOISON = (1 << 10),
};
#define TTU_ACTION(x) ((x) & TTU_ACTION_MASK)

int try_to_unmap(struct page *, enum ttu_flags flags);


pte_t *page_check_address(struct page *, struct mm_struct *,
				unsigned long, spinlock_t **, int);


unsigned long page_address_in_vma(struct page *, struct vm_area_struct *);


int page_mkclean(struct page *);


int try_to_munlock(struct page *);


struct anon_vma *page_lock_anon_vma(struct page *page);
void page_unlock_anon_vma(struct anon_vma *anon_vma);
int page_mapped_in_vma(struct page *page, struct vm_area_struct *vma);

#else	

#define anon_vma_init()		do {} while (0)
#define anon_vma_prepare(vma)	(0)
#define anon_vma_link(vma)	do {} while (0)

static inline int page_referenced(struct page *page, int is_locked,
				  struct mem_cgroup *cnt,
				  unsigned long *vm_flags)
{
	*vm_flags = 0;
	return TestClearPageReferenced(page);
}

#define try_to_unmap(page, refs) SWAP_FAIL

static inline int page_mkclean(struct page *page)
{
	return 0;
}


#endif	


#define SWAP_SUCCESS	0
#define SWAP_AGAIN	1
#define SWAP_FAIL	2
#define SWAP_MLOCK	3

#endif	
