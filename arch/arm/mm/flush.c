
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/pagemap.h>

#include <asm/cacheflush.h>
#include <asm/cachetype.h>
#include <asm/system.h>
#include <asm/tlbflush.h>

#include "mm.h"

#ifdef CONFIG_CPU_CACHE_VIPT

#define ALIAS_FLUSH_START	0xffff4000

static void flush_pfn_alias(unsigned long pfn, unsigned long vaddr)
{
	unsigned long to = ALIAS_FLUSH_START + (CACHE_COLOUR(vaddr) << PAGE_SHIFT);
	const int zero = 0;

	set_pte_ext(TOP_PTE(to), pfn_pte(pfn, PAGE_KERNEL), 0);
	flush_tlb_kernel_page(to);

	asm(	"mcrr	p15, 0, %1, %0, c14\n"
	"	mcr	p15, 0, %2, c7, c10, 4"
	    :
	    : "r" (to), "r" (to + PAGE_SIZE - L1_CACHE_BYTES), "r" (zero)
	    : "cc");
	__flush_icache_all();
}

void flush_cache_mm(struct mm_struct *mm)
{
	if (cache_is_vivt()) {
		if (cpumask_test_cpu(smp_processor_id(), mm_cpumask(mm)))
			__cpuc_flush_user_all();
		return;
	}

	if (cache_is_vipt_aliasing()) {
		asm(	"mcr	p15, 0, %0, c7, c14, 0\n"
		"	mcr	p15, 0, %0, c7, c10, 4"
		    :
		    : "r" (0)
		    : "cc");
		__flush_icache_all();
	}
}

void flush_cache_range(struct vm_area_struct *vma, unsigned long start, unsigned long end)
{
	if (cache_is_vivt()) {
		if (cpumask_test_cpu(smp_processor_id(), mm_cpumask(vma->vm_mm)))
			__cpuc_flush_user_range(start & PAGE_MASK, PAGE_ALIGN(end),
						vma->vm_flags);
		return;
	}

	if (cache_is_vipt_aliasing()) {
		asm(	"mcr	p15, 0, %0, c7, c14, 0\n"
		"	mcr	p15, 0, %0, c7, c10, 4"
		    :
		    : "r" (0)
		    : "cc");
		__flush_icache_all();
	}
}

void flush_cache_page(struct vm_area_struct *vma, unsigned long user_addr, unsigned long pfn)
{
	if (cache_is_vivt()) {
		if (cpumask_test_cpu(smp_processor_id(), mm_cpumask(vma->vm_mm))) {
			unsigned long addr = user_addr & PAGE_MASK;
			__cpuc_flush_user_range(addr, addr + PAGE_SIZE, vma->vm_flags);
		}
		return;
	}

	if (cache_is_vipt_aliasing())
		flush_pfn_alias(pfn, user_addr);
}

void flush_ptrace_access(struct vm_area_struct *vma, struct page *page,
			 unsigned long uaddr, void *kaddr,
			 unsigned long len, int write)
{
	if (cache_is_vivt()) {
		if (cpumask_test_cpu(smp_processor_id(), mm_cpumask(vma->vm_mm))) {
			unsigned long addr = (unsigned long)kaddr;
			__cpuc_coherent_kern_range(addr, addr + len);
		}
		return;
	}

	if (cache_is_vipt_aliasing()) {
		flush_pfn_alias(page_to_pfn(page), uaddr);
		return;
	}

	
	if (cpumask_test_cpu(smp_processor_id(), mm_cpumask(vma->vm_mm)) &&
	    vma->vm_flags & VM_EXEC) {
		unsigned long addr = (unsigned long)kaddr;
		
		__cpuc_coherent_kern_range(addr, addr + len);
	}
}
#else
#define flush_pfn_alias(pfn,vaddr)	do { } while (0)
#endif

void __flush_dcache_page(struct address_space *mapping, struct page *page)
{
	void *addr;

	
#ifdef CONFIG_HIGHMEM
	
	if (PageHighMem(page))
		addr = kmap_high_get(page);
	else
#endif
		addr = page_address(page);

	if (addr) {
		__cpuc_flush_dcache_page(addr);
#ifdef CONFIG_HIGHMEM
		if (PageHighMem(page))
			kunmap_high(page);
#endif
	}

	
	if (mapping && cache_is_vipt_aliasing())
		flush_pfn_alias(page_to_pfn(page),
				page->index << PAGE_CACHE_SHIFT);
}

static void __flush_dcache_aliases(struct address_space *mapping, struct page *page)
{
	struct mm_struct *mm = current->active_mm;
	struct vm_area_struct *mpnt;
	struct prio_tree_iter iter;
	pgoff_t pgoff;

	
	pgoff = page->index << (PAGE_CACHE_SHIFT - PAGE_SHIFT);

	flush_dcache_mmap_lock(mapping);
	vma_prio_tree_foreach(mpnt, &iter, &mapping->i_mmap, pgoff, pgoff) {
		unsigned long offset;

		
		if (mpnt->vm_mm != mm)
			continue;
		if (!(mpnt->vm_flags & VM_MAYSHARE))
			continue;
		offset = (pgoff - mpnt->vm_pgoff) << PAGE_SHIFT;
		flush_cache_page(mpnt, mpnt->vm_start + offset, page_to_pfn(page));
	}
	flush_dcache_mmap_unlock(mapping);
}


void flush_dcache_page(struct page *page)
{
	struct address_space *mapping = page_mapping(page);

#ifndef CONFIG_SMP
	if (!PageHighMem(page) && mapping && !mapping_mapped(mapping))
		set_bit(PG_dcache_dirty, &page->flags);
	else
#endif
	{
		__flush_dcache_page(mapping, page);
		if (mapping && cache_is_vivt())
			__flush_dcache_aliases(mapping, page);
		else if (mapping)
			__flush_icache_all();
	}
}
EXPORT_SYMBOL(flush_dcache_page);


void __flush_anon_page(struct vm_area_struct *vma, struct page *page, unsigned long vmaddr)
{
	unsigned long pfn;

	
	if (cache_is_vipt_nonaliasing())
		return;

	
	pfn = page_to_pfn(page);
	if (cache_is_vivt()) {
		flush_cache_page(vma, vmaddr, pfn);
	} else {
		
		flush_pfn_alias(pfn, vmaddr);
	}

	
	__cpuc_flush_dcache_page(page_address(page));
}
