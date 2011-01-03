

#include <asm/iomap.h>
#include <asm/pat.h>
#include <linux/module.h>
#include <linux/highmem.h>

static int is_io_mapping_possible(resource_size_t base, unsigned long size)
{
#if !defined(CONFIG_X86_PAE) && defined(CONFIG_PHYS_ADDR_T_64BIT)
	
	if (base + size > 0x100000000ULL)
		return 0;
#endif
	return 1;
}

int iomap_create_wc(resource_size_t base, unsigned long size, pgprot_t *prot)
{
	unsigned long flag = _PAGE_CACHE_WC;
	int ret;

	if (!is_io_mapping_possible(base, size))
		return -EINVAL;

	ret = io_reserve_memtype(base, base + size, &flag);
	if (ret)
		return ret;

	*prot = __pgprot(__PAGE_KERNEL | flag);
	return 0;
}
EXPORT_SYMBOL_GPL(iomap_create_wc);

void
iomap_free(resource_size_t base, unsigned long size)
{
	io_free_memtype(base, base + size);
}
EXPORT_SYMBOL_GPL(iomap_free);

void *kmap_atomic_prot_pfn(unsigned long pfn, enum km_type type, pgprot_t prot)
{
	enum fixed_addresses idx;
	unsigned long vaddr;

	pagefault_disable();

	debug_kmap_atomic(type);
	idx = type + KM_TYPE_NR * smp_processor_id();
	vaddr = __fix_to_virt(FIX_KMAP_BEGIN + idx);
	set_pte(kmap_pte - idx, pfn_pte(pfn, prot));
	arch_flush_lazy_mmu_mode();

	return (void *)vaddr;
}


void *
iomap_atomic_prot_pfn(unsigned long pfn, enum km_type type, pgprot_t prot)
{
	
	if (!pat_enabled && pgprot_val(prot) == pgprot_val(PAGE_KERNEL_WC))
		prot = PAGE_KERNEL_UC_MINUS;

	return kmap_atomic_prot_pfn(pfn, type, prot);
}
EXPORT_SYMBOL_GPL(iomap_atomic_prot_pfn);

void
iounmap_atomic(void *kvaddr, enum km_type type)
{
	unsigned long vaddr = (unsigned long) kvaddr & PAGE_MASK;
	enum fixed_addresses idx = type + KM_TYPE_NR*smp_processor_id();

	
	if (vaddr == __fix_to_virt(FIX_KMAP_BEGIN+idx))
		kpte_clear_flush(kmap_pte-idx, vaddr);

	pagefault_enable();
}
EXPORT_SYMBOL_GPL(iounmap_atomic);
