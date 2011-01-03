#include <linux/initrd.h>
#include <linux/ioport.h>
#include <linux/swap.h>

#include <asm/cacheflush.h>
#include <asm/e820.h>
#include <asm/init.h>
#include <asm/page.h>
#include <asm/page_types.h>
#include <asm/sections.h>
#include <asm/setup.h>
#include <asm/system.h>
#include <asm/tlbflush.h>
#include <asm/tlb.h>
#include <asm/proto.h>

DEFINE_PER_CPU(struct mmu_gather, mmu_gathers);

unsigned long __initdata e820_table_start;
unsigned long __meminitdata e820_table_end;
unsigned long __meminitdata e820_table_top;

int after_bootmem;

int direct_gbpages
#ifdef CONFIG_DIRECT_GBPAGES
				= 1
#endif
;

static void __init find_early_table_space(unsigned long end, int use_pse,
					  int use_gbpages)
{
	unsigned long puds, pmds, ptes, tables, start;

	puds = (end + PUD_SIZE - 1) >> PUD_SHIFT;
	tables = roundup(puds * sizeof(pud_t), PAGE_SIZE);

	if (use_gbpages) {
		unsigned long extra;

		extra = end - ((end>>PUD_SHIFT) << PUD_SHIFT);
		pmds = (extra + PMD_SIZE - 1) >> PMD_SHIFT;
	} else
		pmds = (end + PMD_SIZE - 1) >> PMD_SHIFT;

	tables += roundup(pmds * sizeof(pmd_t), PAGE_SIZE);

	if (use_pse) {
		unsigned long extra;

		extra = end - ((end>>PMD_SHIFT) << PMD_SHIFT);
#ifdef CONFIG_X86_32
		extra += PMD_SIZE;
#endif
		ptes = (extra + PAGE_SIZE - 1) >> PAGE_SHIFT;
	} else
		ptes = (end + PAGE_SIZE - 1) >> PAGE_SHIFT;

	tables += roundup(ptes * sizeof(pte_t), PAGE_SIZE);

#ifdef CONFIG_X86_32
	
	tables += roundup(__end_of_fixed_addresses * sizeof(pte_t), PAGE_SIZE);
#endif

	
#ifdef CONFIG_X86_32
	start = 0x7000;
#else
	start = 0x8000;
#endif
	e820_table_start = find_e820_area(start, max_pfn_mapped<<PAGE_SHIFT,
					tables, PAGE_SIZE);
	if (e820_table_start == -1UL)
		panic("Cannot find space for the kernel page tables");

	e820_table_start >>= PAGE_SHIFT;
	e820_table_end = e820_table_start;
	e820_table_top = e820_table_start + (tables >> PAGE_SHIFT);

	printk(KERN_DEBUG "kernel direct mapping tables up to %lx @ %lx-%lx\n",
		end, e820_table_start << PAGE_SHIFT, e820_table_top << PAGE_SHIFT);
}

struct map_range {
	unsigned long start;
	unsigned long end;
	unsigned page_size_mask;
};

#ifdef CONFIG_X86_32
#define NR_RANGE_MR 3
#else 
#define NR_RANGE_MR 5
#endif

static int __meminit save_mr(struct map_range *mr, int nr_range,
			     unsigned long start_pfn, unsigned long end_pfn,
			     unsigned long page_size_mask)
{
	if (start_pfn < end_pfn) {
		if (nr_range >= NR_RANGE_MR)
			panic("run out of range for init_memory_mapping\n");
		mr[nr_range].start = start_pfn<<PAGE_SHIFT;
		mr[nr_range].end   = end_pfn<<PAGE_SHIFT;
		mr[nr_range].page_size_mask = page_size_mask;
		nr_range++;
	}

	return nr_range;
}


unsigned long __init_refok init_memory_mapping(unsigned long start,
					       unsigned long end)
{
	unsigned long page_size_mask = 0;
	unsigned long start_pfn, end_pfn;
	unsigned long ret = 0;
	unsigned long pos;

	struct map_range mr[NR_RANGE_MR];
	int nr_range, i;
	int use_pse, use_gbpages;

	printk(KERN_INFO "init_memory_mapping: %016lx-%016lx\n", start, end);

#if defined(CONFIG_DEBUG_PAGEALLOC) || defined(CONFIG_KMEMCHECK)
	
	use_pse = use_gbpages = 0;
#else
	use_pse = cpu_has_pse;
	use_gbpages = direct_gbpages;
#endif

	set_nx();
	if (nx_enabled)
		printk(KERN_INFO "NX (Execute Disable) protection: active\n");

	
	if (cpu_has_pse)
		set_in_cr4(X86_CR4_PSE);

	
	if (cpu_has_pge) {
		set_in_cr4(X86_CR4_PGE);
		__supported_pte_mask |= _PAGE_GLOBAL;
	}

	if (use_gbpages)
		page_size_mask |= 1 << PG_LEVEL_1G;
	if (use_pse)
		page_size_mask |= 1 << PG_LEVEL_2M;

	memset(mr, 0, sizeof(mr));
	nr_range = 0;

	
	start_pfn = start >> PAGE_SHIFT;
	pos = start_pfn << PAGE_SHIFT;
#ifdef CONFIG_X86_32
	
	if (pos == 0)
		end_pfn = 1<<(PMD_SHIFT - PAGE_SHIFT);
	else
		end_pfn = ((pos + (PMD_SIZE - 1))>>PMD_SHIFT)
				 << (PMD_SHIFT - PAGE_SHIFT);
#else 
	end_pfn = ((pos + (PMD_SIZE - 1)) >> PMD_SHIFT)
			<< (PMD_SHIFT - PAGE_SHIFT);
#endif
	if (end_pfn > (end >> PAGE_SHIFT))
		end_pfn = end >> PAGE_SHIFT;
	if (start_pfn < end_pfn) {
		nr_range = save_mr(mr, nr_range, start_pfn, end_pfn, 0);
		pos = end_pfn << PAGE_SHIFT;
	}

	
	start_pfn = ((pos + (PMD_SIZE - 1))>>PMD_SHIFT)
			 << (PMD_SHIFT - PAGE_SHIFT);
#ifdef CONFIG_X86_32
	end_pfn = (end>>PMD_SHIFT) << (PMD_SHIFT - PAGE_SHIFT);
#else 
	end_pfn = ((pos + (PUD_SIZE - 1))>>PUD_SHIFT)
			 << (PUD_SHIFT - PAGE_SHIFT);
	if (end_pfn > ((end>>PMD_SHIFT)<<(PMD_SHIFT - PAGE_SHIFT)))
		end_pfn = ((end>>PMD_SHIFT)<<(PMD_SHIFT - PAGE_SHIFT));
#endif

	if (start_pfn < end_pfn) {
		nr_range = save_mr(mr, nr_range, start_pfn, end_pfn,
				page_size_mask & (1<<PG_LEVEL_2M));
		pos = end_pfn << PAGE_SHIFT;
	}

#ifdef CONFIG_X86_64
	
	start_pfn = ((pos + (PUD_SIZE - 1))>>PUD_SHIFT)
			 << (PUD_SHIFT - PAGE_SHIFT);
	end_pfn = (end >> PUD_SHIFT) << (PUD_SHIFT - PAGE_SHIFT);
	if (start_pfn < end_pfn) {
		nr_range = save_mr(mr, nr_range, start_pfn, end_pfn,
				page_size_mask &
				 ((1<<PG_LEVEL_2M)|(1<<PG_LEVEL_1G)));
		pos = end_pfn << PAGE_SHIFT;
	}

	
	start_pfn = ((pos + (PMD_SIZE - 1))>>PMD_SHIFT)
			 << (PMD_SHIFT - PAGE_SHIFT);
	end_pfn = (end >> PMD_SHIFT) << (PMD_SHIFT - PAGE_SHIFT);
	if (start_pfn < end_pfn) {
		nr_range = save_mr(mr, nr_range, start_pfn, end_pfn,
				page_size_mask & (1<<PG_LEVEL_2M));
		pos = end_pfn << PAGE_SHIFT;
	}
#endif

	
	start_pfn = pos>>PAGE_SHIFT;
	end_pfn = end>>PAGE_SHIFT;
	nr_range = save_mr(mr, nr_range, start_pfn, end_pfn, 0);

	
	for (i = 0; nr_range > 1 && i < nr_range - 1; i++) {
		unsigned long old_start;
		if (mr[i].end != mr[i+1].start ||
		    mr[i].page_size_mask != mr[i+1].page_size_mask)
			continue;
		
		old_start = mr[i].start;
		memmove(&mr[i], &mr[i+1],
			(nr_range - 1 - i) * sizeof(struct map_range));
		mr[i--].start = old_start;
		nr_range--;
	}

	for (i = 0; i < nr_range; i++)
		printk(KERN_DEBUG " %010lx - %010lx page %s\n",
				mr[i].start, mr[i].end,
			(mr[i].page_size_mask & (1<<PG_LEVEL_1G))?"1G":(
			 (mr[i].page_size_mask & (1<<PG_LEVEL_2M))?"2M":"4k"));

	
	if (!after_bootmem)
		find_early_table_space(end, use_pse, use_gbpages);

#ifdef CONFIG_X86_32
	for (i = 0; i < nr_range; i++)
		kernel_physical_mapping_init(mr[i].start, mr[i].end,
					     mr[i].page_size_mask);
	ret = end;
#else 
	for (i = 0; i < nr_range; i++)
		ret = kernel_physical_mapping_init(mr[i].start, mr[i].end,
						   mr[i].page_size_mask);
#endif

#ifdef CONFIG_X86_32
	early_ioremap_page_table_range_init();

	load_cr3(swapper_pg_dir);
#endif

#ifdef CONFIG_X86_64
	if (!after_bootmem && !start) {
		pud_t *pud;
		pmd_t *pmd;

		mmu_cr4_features = read_cr4();

		
		pud = pud_offset(pgd_offset_k(_brk_end), _brk_end);
		pmd = pmd_offset(pud, _brk_end - 1);
		while (++pmd <= pmd_offset(pud, (unsigned long)_end - 1))
			pmd_clear(pmd);
	}
#endif
	__flush_tlb_all();

	if (!after_bootmem && e820_table_end > e820_table_start)
		reserve_early(e820_table_start << PAGE_SHIFT,
				 e820_table_end << PAGE_SHIFT, "PGTABLE");

	if (!after_bootmem)
		early_memtest(start, end);

	return ret >> PAGE_SHIFT;
}



int devmem_is_allowed(unsigned long pagenr)
{
	if (pagenr <= 256)
		return 1;
	if (iomem_is_exclusive(pagenr << PAGE_SHIFT))
		return 0;
	if (!page_is_ram(pagenr))
		return 1;
	return 0;
}

void free_init_pages(char *what, unsigned long begin, unsigned long end)
{
	unsigned long addr = begin;

	if (addr >= end)
		return;

	
#ifdef CONFIG_DEBUG_PAGEALLOC
	printk(KERN_INFO "debug: unmapping init memory %08lx..%08lx\n",
		begin, PAGE_ALIGN(end));
	set_memory_np(begin, (end - begin) >> PAGE_SHIFT);
#else
	
	set_memory_rw(begin, (end - begin) >> PAGE_SHIFT);

	printk(KERN_INFO "Freeing %s: %luk freed\n", what, (end - begin) >> 10);

	for (; addr < end; addr += PAGE_SIZE) {
		ClearPageReserved(virt_to_page(addr));
		init_page_count(virt_to_page(addr));
		memset((void *)(addr & ~(PAGE_SIZE-1)),
			POISON_FREE_INITMEM, PAGE_SIZE);
		free_page(addr);
		totalram_pages++;
	}
#endif
}

void free_initmem(void)
{
	free_init_pages("unused kernel memory",
			(unsigned long)(&__init_begin),
			(unsigned long)(&__init_end));
}

#ifdef CONFIG_BLK_DEV_INITRD
void free_initrd_mem(unsigned long start, unsigned long end)
{
	free_init_pages("initrd memory", start, end);
}
#endif
