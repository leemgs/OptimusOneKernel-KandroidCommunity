#ifndef _ASM_X86_PGTABLE_H
#define _ASM_X86_PGTABLE_H

#include <asm/page.h>
#include <asm/e820.h>

#include <asm/pgtable_types.h>


#define pgprot_noncached(prot)					\
	((boot_cpu_data.x86 > 3)				\
	 ? (__pgprot(pgprot_val(prot) | _PAGE_CACHE_UC_MINUS))	\
	 : (prot))

#ifndef __ASSEMBLY__


extern unsigned long empty_zero_page[PAGE_SIZE / sizeof(unsigned long)];
#define ZERO_PAGE(vaddr) (virt_to_page(empty_zero_page))

extern spinlock_t pgd_lock;
extern struct list_head pgd_list;

#ifdef CONFIG_PARAVIRT
#include <asm/paravirt.h>
#else  
#define set_pte(ptep, pte)		native_set_pte(ptep, pte)
#define set_pte_at(mm, addr, ptep, pte)	native_set_pte_at(mm, addr, ptep, pte)

#define set_pte_atomic(ptep, pte)					\
	native_set_pte_atomic(ptep, pte)

#define set_pmd(pmdp, pmd)		native_set_pmd(pmdp, pmd)

#ifndef __PAGETABLE_PUD_FOLDED
#define set_pgd(pgdp, pgd)		native_set_pgd(pgdp, pgd)
#define pgd_clear(pgd)			native_pgd_clear(pgd)
#endif

#ifndef set_pud
# define set_pud(pudp, pud)		native_set_pud(pudp, pud)
#endif

#ifndef __PAGETABLE_PMD_FOLDED
#define pud_clear(pud)			native_pud_clear(pud)
#endif

#define pte_clear(mm, addr, ptep)	native_pte_clear(mm, addr, ptep)
#define pmd_clear(pmd)			native_pmd_clear(pmd)

#define pte_update(mm, addr, ptep)              do { } while (0)
#define pte_update_defer(mm, addr, ptep)        do { } while (0)

#define pgd_val(x)	native_pgd_val(x)
#define __pgd(x)	native_make_pgd(x)

#ifndef __PAGETABLE_PUD_FOLDED
#define pud_val(x)	native_pud_val(x)
#define __pud(x)	native_make_pud(x)
#endif

#ifndef __PAGETABLE_PMD_FOLDED
#define pmd_val(x)	native_pmd_val(x)
#define __pmd(x)	native_make_pmd(x)
#endif

#define pte_val(x)	native_pte_val(x)
#define __pte(x)	native_make_pte(x)

#define arch_end_context_switch(prev)	do {} while(0)

#endif	


static inline int pte_dirty(pte_t pte)
{
	return pte_flags(pte) & _PAGE_DIRTY;
}

static inline int pte_young(pte_t pte)
{
	return pte_flags(pte) & _PAGE_ACCESSED;
}

static inline int pte_write(pte_t pte)
{
	return pte_flags(pte) & _PAGE_RW;
}

static inline int pte_file(pte_t pte)
{
	return pte_flags(pte) & _PAGE_FILE;
}

static inline int pte_huge(pte_t pte)
{
	return pte_flags(pte) & _PAGE_PSE;
}

static inline int pte_global(pte_t pte)
{
	return pte_flags(pte) & _PAGE_GLOBAL;
}

static inline int pte_exec(pte_t pte)
{
	return !(pte_flags(pte) & _PAGE_NX);
}

static inline int pte_special(pte_t pte)
{
	return pte_flags(pte) & _PAGE_SPECIAL;
}

static inline unsigned long pte_pfn(pte_t pte)
{
	return (pte_val(pte) & PTE_PFN_MASK) >> PAGE_SHIFT;
}

static inline unsigned long pmd_pfn(pmd_t pmd)
{
	return (pmd_val(pmd) & PTE_PFN_MASK) >> PAGE_SHIFT;
}

#define pte_page(pte)	pfn_to_page(pte_pfn(pte))

static inline int pmd_large(pmd_t pte)
{
	return (pmd_flags(pte) & (_PAGE_PSE | _PAGE_PRESENT)) ==
		(_PAGE_PSE | _PAGE_PRESENT);
}

static inline pte_t pte_set_flags(pte_t pte, pteval_t set)
{
	pteval_t v = native_pte_val(pte);

	return native_make_pte(v | set);
}

static inline pte_t pte_clear_flags(pte_t pte, pteval_t clear)
{
	pteval_t v = native_pte_val(pte);

	return native_make_pte(v & ~clear);
}

static inline pte_t pte_mkclean(pte_t pte)
{
	return pte_clear_flags(pte, _PAGE_DIRTY);
}

static inline pte_t pte_mkold(pte_t pte)
{
	return pte_clear_flags(pte, _PAGE_ACCESSED);
}

static inline pte_t pte_wrprotect(pte_t pte)
{
	return pte_clear_flags(pte, _PAGE_RW);
}

static inline pte_t pte_mkexec(pte_t pte)
{
	return pte_clear_flags(pte, _PAGE_NX);
}

static inline pte_t pte_mkdirty(pte_t pte)
{
	return pte_set_flags(pte, _PAGE_DIRTY);
}

static inline pte_t pte_mkyoung(pte_t pte)
{
	return pte_set_flags(pte, _PAGE_ACCESSED);
}

static inline pte_t pte_mkwrite(pte_t pte)
{
	return pte_set_flags(pte, _PAGE_RW);
}

static inline pte_t pte_mkhuge(pte_t pte)
{
	return pte_set_flags(pte, _PAGE_PSE);
}

static inline pte_t pte_clrhuge(pte_t pte)
{
	return pte_clear_flags(pte, _PAGE_PSE);
}

static inline pte_t pte_mkglobal(pte_t pte)
{
	return pte_set_flags(pte, _PAGE_GLOBAL);
}

static inline pte_t pte_clrglobal(pte_t pte)
{
	return pte_clear_flags(pte, _PAGE_GLOBAL);
}

static inline pte_t pte_mkspecial(pte_t pte)
{
	return pte_set_flags(pte, _PAGE_SPECIAL);
}


static inline pgprotval_t massage_pgprot(pgprot_t pgprot)
{
	pgprotval_t protval = pgprot_val(pgprot);

	if (protval & _PAGE_PRESENT)
		protval &= __supported_pte_mask;

	return protval;
}

static inline pte_t pfn_pte(unsigned long page_nr, pgprot_t pgprot)
{
	return __pte(((phys_addr_t)page_nr << PAGE_SHIFT) |
		     massage_pgprot(pgprot));
}

static inline pmd_t pfn_pmd(unsigned long page_nr, pgprot_t pgprot)
{
	return __pmd(((phys_addr_t)page_nr << PAGE_SHIFT) |
		     massage_pgprot(pgprot));
}

static inline pte_t pte_modify(pte_t pte, pgprot_t newprot)
{
	pteval_t val = pte_val(pte);

	
	val &= _PAGE_CHG_MASK;
	val |= massage_pgprot(newprot) & ~_PAGE_CHG_MASK;

	return __pte(val);
}


#define pgprot_modify pgprot_modify
static inline pgprot_t pgprot_modify(pgprot_t oldprot, pgprot_t newprot)
{
	pgprotval_t preservebits = pgprot_val(oldprot) & _PAGE_CHG_MASK;
	pgprotval_t addbits = pgprot_val(newprot);
	return __pgprot(preservebits | addbits);
}

#define pte_pgprot(x) __pgprot(pte_flags(x) & PTE_FLAGS_MASK)

#define canon_pgprot(p) __pgprot(massage_pgprot(p))

static inline int is_new_memtype_allowed(u64 paddr, unsigned long size,
					 unsigned long flags,
					 unsigned long new_flags)
{
	
	if (is_ISA_range(paddr, paddr + size - 1))
		return 1;

	
	if ((flags == _PAGE_CACHE_UC_MINUS &&
	     new_flags == _PAGE_CACHE_WB) ||
	    (flags == _PAGE_CACHE_WC &&
	     new_flags == _PAGE_CACHE_WB)) {
		return 0;
	}

	return 1;
}

pmd_t *populate_extra_pmd(unsigned long vaddr);
pte_t *populate_extra_pte(unsigned long vaddr);
#endif	

#ifdef CONFIG_X86_32
# include "pgtable_32.h"
#else
# include "pgtable_64.h"
#endif

#ifndef __ASSEMBLY__
#include <linux/mm_types.h>

static inline int pte_none(pte_t pte)
{
	return !pte.pte;
}

#define __HAVE_ARCH_PTE_SAME
static inline int pte_same(pte_t a, pte_t b)
{
	return a.pte == b.pte;
}

static inline int pte_present(pte_t a)
{
	return pte_flags(a) & (_PAGE_PRESENT | _PAGE_PROTNONE);
}

static inline int pte_hidden(pte_t pte)
{
	return pte_flags(pte) & _PAGE_HIDDEN;
}

static inline int pmd_present(pmd_t pmd)
{
	return pmd_flags(pmd) & _PAGE_PRESENT;
}

static inline int pmd_none(pmd_t pmd)
{
	
	return (unsigned long)native_pmd_val(pmd) == 0;
}

static inline unsigned long pmd_page_vaddr(pmd_t pmd)
{
	return (unsigned long)__va(pmd_val(pmd) & PTE_PFN_MASK);
}


#define pmd_page(pmd)	pfn_to_page(pmd_val(pmd) >> PAGE_SHIFT)


static inline unsigned long pmd_index(unsigned long address)
{
	return (address >> PMD_SHIFT) & (PTRS_PER_PMD - 1);
}


#define mk_pte(page, pgprot)   pfn_pte(page_to_pfn(page), (pgprot))


static inline unsigned long pte_index(unsigned long address)
{
	return (address >> PAGE_SHIFT) & (PTRS_PER_PTE - 1);
}

static inline pte_t *pte_offset_kernel(pmd_t *pmd, unsigned long address)
{
	return (pte_t *)pmd_page_vaddr(*pmd) + pte_index(address);
}

static inline int pmd_bad(pmd_t pmd)
{
	return (pmd_flags(pmd) & ~_PAGE_USER) != _KERNPG_TABLE;
}

static inline unsigned long pages_to_mb(unsigned long npg)
{
	return npg >> (20 - PAGE_SHIFT);
}

#define io_remap_pfn_range(vma, vaddr, pfn, size, prot)	\
	remap_pfn_range(vma, vaddr, pfn, size, prot)

#if PAGETABLE_LEVELS > 2
static inline int pud_none(pud_t pud)
{
	return native_pud_val(pud) == 0;
}

static inline int pud_present(pud_t pud)
{
	return pud_flags(pud) & _PAGE_PRESENT;
}

static inline unsigned long pud_page_vaddr(pud_t pud)
{
	return (unsigned long)__va((unsigned long)pud_val(pud) & PTE_PFN_MASK);
}


#define pud_page(pud)		pfn_to_page(pud_val(pud) >> PAGE_SHIFT)


static inline pmd_t *pmd_offset(pud_t *pud, unsigned long address)
{
	return (pmd_t *)pud_page_vaddr(*pud) + pmd_index(address);
}

static inline int pud_large(pud_t pud)
{
	return (pud_val(pud) & (_PAGE_PSE | _PAGE_PRESENT)) ==
		(_PAGE_PSE | _PAGE_PRESENT);
}

static inline int pud_bad(pud_t pud)
{
	return (pud_flags(pud) & ~(_KERNPG_TABLE | _PAGE_USER)) != 0;
}
#else
static inline int pud_large(pud_t pud)
{
	return 0;
}
#endif	

#if PAGETABLE_LEVELS > 3
static inline int pgd_present(pgd_t pgd)
{
	return pgd_flags(pgd) & _PAGE_PRESENT;
}

static inline unsigned long pgd_page_vaddr(pgd_t pgd)
{
	return (unsigned long)__va((unsigned long)pgd_val(pgd) & PTE_PFN_MASK);
}


#define pgd_page(pgd)		pfn_to_page(pgd_val(pgd) >> PAGE_SHIFT)


static inline unsigned long pud_index(unsigned long address)
{
	return (address >> PUD_SHIFT) & (PTRS_PER_PUD - 1);
}

static inline pud_t *pud_offset(pgd_t *pgd, unsigned long address)
{
	return (pud_t *)pgd_page_vaddr(*pgd) + pud_index(address);
}

static inline int pgd_bad(pgd_t pgd)
{
	return (pgd_flags(pgd) & ~_PAGE_USER) != _KERNPG_TABLE;
}

static inline int pgd_none(pgd_t pgd)
{
	return !native_pgd_val(pgd);
}
#endif	

#endif	


#define pgd_index(address) (((address) >> PGDIR_SHIFT) & (PTRS_PER_PGD - 1))


#define pgd_offset(mm, address) ((mm)->pgd + pgd_index((address)))

#define pgd_offset_k(address) pgd_offset(&init_mm, (address))


#define KERNEL_PGD_BOUNDARY	pgd_index(PAGE_OFFSET)
#define KERNEL_PGD_PTRS		(PTRS_PER_PGD - KERNEL_PGD_BOUNDARY)

#ifndef __ASSEMBLY__

extern int direct_gbpages;


static inline pte_t native_local_ptep_get_and_clear(pte_t *ptep)
{
	pte_t res = *ptep;

	
	native_pte_clear(NULL, 0, ptep);
	return res;
}

static inline void native_set_pte_at(struct mm_struct *mm, unsigned long addr,
				     pte_t *ptep , pte_t pte)
{
	native_set_pte(ptep, pte);
}

#ifndef CONFIG_PARAVIRT

#define pte_update(mm, addr, ptep)		do { } while (0)
#define pte_update_defer(mm, addr, ptep)	do { } while (0)
#endif


struct vm_area_struct;

#define  __HAVE_ARCH_PTEP_SET_ACCESS_FLAGS
extern int ptep_set_access_flags(struct vm_area_struct *vma,
				 unsigned long address, pte_t *ptep,
				 pte_t entry, int dirty);

#define __HAVE_ARCH_PTEP_TEST_AND_CLEAR_YOUNG
extern int ptep_test_and_clear_young(struct vm_area_struct *vma,
				     unsigned long addr, pte_t *ptep);

#define __HAVE_ARCH_PTEP_CLEAR_YOUNG_FLUSH
extern int ptep_clear_flush_young(struct vm_area_struct *vma,
				  unsigned long address, pte_t *ptep);

#define __HAVE_ARCH_PTEP_GET_AND_CLEAR
static inline pte_t ptep_get_and_clear(struct mm_struct *mm, unsigned long addr,
				       pte_t *ptep)
{
	pte_t pte = native_ptep_get_and_clear(ptep);
	pte_update(mm, addr, ptep);
	return pte;
}

#define __HAVE_ARCH_PTEP_GET_AND_CLEAR_FULL
static inline pte_t ptep_get_and_clear_full(struct mm_struct *mm,
					    unsigned long addr, pte_t *ptep,
					    int full)
{
	pte_t pte;
	if (full) {
		
		pte = native_local_ptep_get_and_clear(ptep);
	} else {
		pte = ptep_get_and_clear(mm, addr, ptep);
	}
	return pte;
}

#define __HAVE_ARCH_PTEP_SET_WRPROTECT
static inline void ptep_set_wrprotect(struct mm_struct *mm,
				      unsigned long addr, pte_t *ptep)
{
	clear_bit(_PAGE_BIT_RW, (unsigned long *)&ptep->pte);
	pte_update(mm, addr, ptep);
}


static inline void clone_pgd_range(pgd_t *dst, pgd_t *src, int count)
{
       memcpy(dst, src, count * sizeof(pgd_t));
}


#include <asm-generic/pgtable.h>
#endif	

#endif 
