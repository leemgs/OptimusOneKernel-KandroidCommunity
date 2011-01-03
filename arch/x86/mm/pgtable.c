#include <linux/mm.h>
#include <asm/pgalloc.h>
#include <asm/pgtable.h>
#include <asm/tlb.h>
#include <asm/fixmap.h>

#define PGALLOC_GFP GFP_KERNEL | __GFP_NOTRACK | __GFP_REPEAT | __GFP_ZERO

pte_t *pte_alloc_one_kernel(struct mm_struct *mm, unsigned long address)
{
	return (pte_t *)__get_free_page(PGALLOC_GFP);
}

pgtable_t pte_alloc_one(struct mm_struct *mm, unsigned long address)
{
	struct page *pte;

#ifdef CONFIG_HIGHPTE
	pte = alloc_pages(PGALLOC_GFP | __GFP_HIGHMEM, 0);
#else
	pte = alloc_pages(PGALLOC_GFP, 0);
#endif
	if (pte)
		pgtable_page_ctor(pte);
	return pte;
}

void ___pte_free_tlb(struct mmu_gather *tlb, struct page *pte)
{
	pgtable_page_dtor(pte);
	paravirt_release_pte(page_to_pfn(pte));
	tlb_remove_page(tlb, pte);
}

#if PAGETABLE_LEVELS > 2
void ___pmd_free_tlb(struct mmu_gather *tlb, pmd_t *pmd)
{
	paravirt_release_pmd(__pa(pmd) >> PAGE_SHIFT);
	tlb_remove_page(tlb, virt_to_page(pmd));
}

#if PAGETABLE_LEVELS > 3
void ___pud_free_tlb(struct mmu_gather *tlb, pud_t *pud)
{
	paravirt_release_pud(__pa(pud) >> PAGE_SHIFT);
	tlb_remove_page(tlb, virt_to_page(pud));
}
#endif	
#endif	

static inline void pgd_list_add(pgd_t *pgd)
{
	struct page *page = virt_to_page(pgd);

	list_add(&page->lru, &pgd_list);
}

static inline void pgd_list_del(pgd_t *pgd)
{
	struct page *page = virt_to_page(pgd);

	list_del(&page->lru);
}

#define UNSHARED_PTRS_PER_PGD				\
	(SHARED_KERNEL_PMD ? KERNEL_PGD_BOUNDARY : PTRS_PER_PGD)

static void pgd_ctor(pgd_t *pgd)
{
	
	if (PAGETABLE_LEVELS == 2 ||
	    (PAGETABLE_LEVELS == 3 && SHARED_KERNEL_PMD) ||
	    PAGETABLE_LEVELS == 4) {
		clone_pgd_range(pgd + KERNEL_PGD_BOUNDARY,
				swapper_pg_dir + KERNEL_PGD_BOUNDARY,
				KERNEL_PGD_PTRS);
		paravirt_alloc_pmd_clone(__pa(pgd) >> PAGE_SHIFT,
					 __pa(swapper_pg_dir) >> PAGE_SHIFT,
					 KERNEL_PGD_BOUNDARY,
					 KERNEL_PGD_PTRS);
	}

	
	if (!SHARED_KERNEL_PMD)
		pgd_list_add(pgd);
}

static void pgd_dtor(pgd_t *pgd)
{
	unsigned long flags; 

	if (SHARED_KERNEL_PMD)
		return;

	spin_lock_irqsave(&pgd_lock, flags);
	pgd_list_del(pgd);
	spin_unlock_irqrestore(&pgd_lock, flags);
}



#ifdef CONFIG_X86_PAE

#define PREALLOCATED_PMDS	UNSHARED_PTRS_PER_PGD

void pud_populate(struct mm_struct *mm, pud_t *pudp, pmd_t *pmd)
{
	paravirt_alloc_pmd(mm, __pa(pmd) >> PAGE_SHIFT);

	
	set_pud(pudp, __pud(__pa(pmd) | _PAGE_PRESENT));

	
	if (mm == current->active_mm)
		write_cr3(read_cr3());
}
#else  


#define PREALLOCATED_PMDS	0

#endif	

static void free_pmds(pmd_t *pmds[])
{
	int i;

	for(i = 0; i < PREALLOCATED_PMDS; i++)
		if (pmds[i])
			free_page((unsigned long)pmds[i]);
}

static int preallocate_pmds(pmd_t *pmds[])
{
	int i;
	bool failed = false;

	for(i = 0; i < PREALLOCATED_PMDS; i++) {
		pmd_t *pmd = (pmd_t *)__get_free_page(PGALLOC_GFP);
		if (pmd == NULL)
			failed = true;
		pmds[i] = pmd;
	}

	if (failed) {
		free_pmds(pmds);
		return -ENOMEM;
	}

	return 0;
}


static void pgd_mop_up_pmds(struct mm_struct *mm, pgd_t *pgdp)
{
	int i;

	for(i = 0; i < PREALLOCATED_PMDS; i++) {
		pgd_t pgd = pgdp[i];

		if (pgd_val(pgd) != 0) {
			pmd_t *pmd = (pmd_t *)pgd_page_vaddr(pgd);

			pgdp[i] = native_make_pgd(0);

			paravirt_release_pmd(pgd_val(pgd) >> PAGE_SHIFT);
			pmd_free(mm, pmd);
		}
	}
}

static void pgd_prepopulate_pmd(struct mm_struct *mm, pgd_t *pgd, pmd_t *pmds[])
{
	pud_t *pud;
	unsigned long addr;
	int i;

	if (PREALLOCATED_PMDS == 0) 
		return;

	pud = pud_offset(pgd, 0);

 	for (addr = i = 0; i < PREALLOCATED_PMDS;
	     i++, pud++, addr += PUD_SIZE) {
		pmd_t *pmd = pmds[i];

		if (i >= KERNEL_PGD_BOUNDARY)
			memcpy(pmd, (pmd_t *)pgd_page_vaddr(swapper_pg_dir[i]),
			       sizeof(pmd_t) * PTRS_PER_PMD);

		pud_populate(mm, pud, pmd);
	}
}

pgd_t *pgd_alloc(struct mm_struct *mm)
{
	pgd_t *pgd;
	pmd_t *pmds[PREALLOCATED_PMDS];
	unsigned long flags;

	pgd = (pgd_t *)__get_free_page(PGALLOC_GFP);

	if (pgd == NULL)
		goto out;

	mm->pgd = pgd;

	if (preallocate_pmds(pmds) != 0)
		goto out_free_pgd;

	if (paravirt_pgd_alloc(mm) != 0)
		goto out_free_pmds;

	
	spin_lock_irqsave(&pgd_lock, flags);

	pgd_ctor(pgd);
	pgd_prepopulate_pmd(mm, pgd, pmds);

	spin_unlock_irqrestore(&pgd_lock, flags);

	return pgd;

out_free_pmds:
	free_pmds(pmds);
out_free_pgd:
	free_page((unsigned long)pgd);
out:
	return NULL;
}

void pgd_free(struct mm_struct *mm, pgd_t *pgd)
{
	pgd_mop_up_pmds(mm, pgd);
	pgd_dtor(pgd);
	paravirt_pgd_free(mm, pgd);
	free_page((unsigned long)pgd);
}

int ptep_set_access_flags(struct vm_area_struct *vma,
			  unsigned long address, pte_t *ptep,
			  pte_t entry, int dirty)
{
	int changed = !pte_same(*ptep, entry);

	if (changed && dirty) {
		*ptep = entry;
		pte_update_defer(vma->vm_mm, address, ptep);
		flush_tlb_page(vma, address);
	}

	return changed;
}

int ptep_test_and_clear_young(struct vm_area_struct *vma,
			      unsigned long addr, pte_t *ptep)
{
	int ret = 0;

	if (pte_young(*ptep))
		ret = test_and_clear_bit(_PAGE_BIT_ACCESSED,
					 (unsigned long *) &ptep->pte);

	if (ret)
		pte_update(vma->vm_mm, addr, ptep);

	return ret;
}

int ptep_clear_flush_young(struct vm_area_struct *vma,
			   unsigned long address, pte_t *ptep)
{
	int young;

	young = ptep_test_and_clear_young(vma, address, ptep);
	if (young)
		flush_tlb_page(vma, address);

	return young;
}


void __init reserve_top_address(unsigned long reserve)
{
#ifdef CONFIG_X86_32
	BUG_ON(fixmaps_set > 0);
	printk(KERN_INFO "Reserving virtual address space above 0x%08x\n",
	       (int)-reserve);
	__FIXADDR_TOP = -reserve - PAGE_SIZE;
#endif
}

int fixmaps_set;

void __native_set_fixmap(enum fixed_addresses idx, pte_t pte)
{
	unsigned long address = __fix_to_virt(idx);

	if (idx >= __end_of_fixed_addresses) {
		BUG();
		return;
	}
	set_pte_vaddr(address, pte);
	fixmaps_set++;
}

void native_set_fixmap(enum fixed_addresses idx, phys_addr_t phys,
		       pgprot_t flags)
{
	__native_set_fixmap(idx, pfn_pte(phys >> PAGE_SHIFT, flags));
}
