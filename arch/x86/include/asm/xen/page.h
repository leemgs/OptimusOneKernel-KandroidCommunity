#ifndef _ASM_X86_XEN_PAGE_H
#define _ASM_X86_XEN_PAGE_H

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/pfn.h>

#include <asm/uaccess.h>
#include <asm/page.h>
#include <asm/pgtable.h>

#include <xen/interface/xen.h>
#include <xen/features.h>


typedef struct xmaddr {
	phys_addr_t maddr;
} xmaddr_t;


typedef struct xpaddr {
	phys_addr_t paddr;
} xpaddr_t;

#define XMADDR(x)	((xmaddr_t) { .maddr = (x) })
#define XPADDR(x)	((xpaddr_t) { .paddr = (x) })


#define INVALID_P2M_ENTRY	(~0UL)
#define FOREIGN_FRAME_BIT	(1UL<<31)
#define FOREIGN_FRAME(m)	((m) | FOREIGN_FRAME_BIT)


#define MAX_DOMAIN_PAGES						\
    ((unsigned long)((u64)CONFIG_XEN_MAX_DOMAIN_MEMORY * 1024 * 1024 * 1024 / PAGE_SIZE))


extern unsigned long get_phys_to_machine(unsigned long pfn);
extern void set_phys_to_machine(unsigned long pfn, unsigned long mfn);

static inline unsigned long pfn_to_mfn(unsigned long pfn)
{
	if (xen_feature(XENFEAT_auto_translated_physmap))
		return pfn;

	return get_phys_to_machine(pfn) & ~FOREIGN_FRAME_BIT;
}

static inline int phys_to_machine_mapping_valid(unsigned long pfn)
{
	if (xen_feature(XENFEAT_auto_translated_physmap))
		return 1;

	return get_phys_to_machine(pfn) != INVALID_P2M_ENTRY;
}

static inline unsigned long mfn_to_pfn(unsigned long mfn)
{
	unsigned long pfn;

	if (xen_feature(XENFEAT_auto_translated_physmap))
		return mfn;

#if 0
	if (unlikely((mfn >> machine_to_phys_order) != 0))
		return max_mapnr;
#endif

	pfn = 0;
	
	__get_user(pfn, &machine_to_phys_mapping[mfn]);

	return pfn;
}

static inline xmaddr_t phys_to_machine(xpaddr_t phys)
{
	unsigned offset = phys.paddr & ~PAGE_MASK;
	return XMADDR(PFN_PHYS(pfn_to_mfn(PFN_DOWN(phys.paddr))) | offset);
}

static inline xpaddr_t machine_to_phys(xmaddr_t machine)
{
	unsigned offset = machine.maddr & ~PAGE_MASK;
	return XPADDR(PFN_PHYS(mfn_to_pfn(PFN_DOWN(machine.maddr))) | offset);
}


static inline unsigned long mfn_to_local_pfn(unsigned long mfn)
{
	extern unsigned long max_mapnr;
	unsigned long pfn = mfn_to_pfn(mfn);
	if ((pfn < max_mapnr)
	    && !xen_feature(XENFEAT_auto_translated_physmap)
	    && (get_phys_to_machine(pfn) != mfn))
		return max_mapnr; 
	
	return pfn;
}


#define virt_to_machine(v)	(phys_to_machine(XPADDR(__pa(v))))
#define virt_to_pfn(v)          (PFN_DOWN(__pa(v)))
#define virt_to_mfn(v)		(pfn_to_mfn(virt_to_pfn(v)))
#define mfn_to_virt(m)		(__va(mfn_to_pfn(m) << PAGE_SHIFT))

static inline unsigned long pte_mfn(pte_t pte)
{
	return (pte.pte & PTE_PFN_MASK) >> PAGE_SHIFT;
}

static inline pte_t mfn_pte(unsigned long page_nr, pgprot_t pgprot)
{
	pte_t pte;

	pte.pte = ((phys_addr_t)page_nr << PAGE_SHIFT) |
			massage_pgprot(pgprot);

	return pte;
}

static inline pteval_t pte_val_ma(pte_t pte)
{
	return pte.pte;
}

static inline pte_t __pte_ma(pteval_t x)
{
	return (pte_t) { .pte = x };
}

#define pmd_val_ma(v) ((v).pmd)
#ifdef __PAGETABLE_PUD_FOLDED
#define pud_val_ma(v) ((v).pgd.pgd)
#else
#define pud_val_ma(v) ((v).pud)
#endif
#define __pmd_ma(x)	((pmd_t) { (x) } )

#define pgd_val_ma(x)	((x).pgd)


xmaddr_t arbitrary_virt_to_machine(void *address);
unsigned long arbitrary_virt_to_mfn(void *vaddr);
void make_lowmem_page_readonly(void *vaddr);
void make_lowmem_page_readwrite(void *vaddr);

#endif 
