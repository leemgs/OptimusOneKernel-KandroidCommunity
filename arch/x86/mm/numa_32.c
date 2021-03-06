

#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/mmzone.h>
#include <linux/highmem.h>
#include <linux/initrd.h>
#include <linux/nodemask.h>
#include <linux/module.h>
#include <linux/kexec.h>
#include <linux/pfn.h>
#include <linux/swap.h>
#include <linux/acpi.h>

#include <asm/e820.h>
#include <asm/setup.h>
#include <asm/mmzone.h>
#include <asm/bios_ebda.h>
#include <asm/proto.h>

struct pglist_data *node_data[MAX_NUMNODES] __read_mostly;
EXPORT_SYMBOL(node_data);


unsigned long node_start_pfn[MAX_NUMNODES] __read_mostly;
unsigned long node_end_pfn[MAX_NUMNODES] __read_mostly;


#ifdef CONFIG_DISCONTIGMEM

s8 physnode_map[MAX_ELEMENTS] __read_mostly = { [0 ... (MAX_ELEMENTS - 1)] = -1};
EXPORT_SYMBOL(physnode_map);

void memory_present(int nid, unsigned long start, unsigned long end)
{
	unsigned long pfn;

	printk(KERN_INFO "Node: %d, start_pfn: %lx, end_pfn: %lx\n",
			nid, start, end);
	printk(KERN_DEBUG "  Setting physnode_map array to node %d for pfns:\n", nid);
	printk(KERN_DEBUG "  ");
	for (pfn = start; pfn < end; pfn += PAGES_PER_ELEMENT) {
		physnode_map[pfn / PAGES_PER_ELEMENT] = nid;
		printk(KERN_CONT "%lx ", pfn);
	}
	printk(KERN_CONT "\n");
}

unsigned long node_memmap_size_bytes(int nid, unsigned long start_pfn,
					      unsigned long end_pfn)
{
	unsigned long nr_pages = end_pfn - start_pfn;

	if (!nr_pages)
		return 0;

	return (nr_pages + 1) * sizeof(struct page);
}
#endif

extern unsigned long find_max_low_pfn(void);
extern unsigned long highend_pfn, highstart_pfn;

#define LARGE_PAGE_BYTES (PTRS_PER_PTE * PAGE_SIZE)

unsigned long node_remap_size[MAX_NUMNODES];
static void *node_remap_start_vaddr[MAX_NUMNODES];
void set_pmd_pfn(unsigned long vaddr, unsigned long pfn, pgprot_t flags);

static unsigned long kva_start_pfn;
static unsigned long kva_pages;

int __init get_memcfg_numa_flat(void)
{
	printk(KERN_DEBUG "NUMA - single node, flat memory mode\n");

	node_start_pfn[0] = 0;
	node_end_pfn[0] = max_pfn;
	e820_register_active_regions(0, 0, max_pfn);
	memory_present(0, 0, max_pfn);
	node_remap_size[0] = node_memmap_size_bytes(0, 0, max_pfn);

        
	nodes_clear(node_online_map);
	node_set_online(0);
	return 1;
}


static void __init propagate_e820_map_node(int nid)
{
	if (node_end_pfn[nid] > max_pfn)
		node_end_pfn[nid] = max_pfn;
	
	if (node_start_pfn[nid] > max_pfn)
		node_start_pfn[nid] = max_pfn;
	BUG_ON(node_start_pfn[nid] > node_end_pfn[nid]);
}


static void __init allocate_pgdat(int nid)
{
	char buf[16];

	if (node_has_online_mem(nid) && node_remap_start_vaddr[nid])
		NODE_DATA(nid) = (pg_data_t *)node_remap_start_vaddr[nid];
	else {
		unsigned long pgdat_phys;
		pgdat_phys = find_e820_area(min_low_pfn<<PAGE_SHIFT,
				 max_pfn_mapped<<PAGE_SHIFT,
				 sizeof(pg_data_t),
				 PAGE_SIZE);
		NODE_DATA(nid) = (pg_data_t *)(pfn_to_kaddr(pgdat_phys>>PAGE_SHIFT));
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "NODE_DATA %d",  nid);
		reserve_early(pgdat_phys, pgdat_phys + sizeof(pg_data_t), buf);
	}
	printk(KERN_DEBUG "allocate_pgdat: node %d NODE_DATA %08lx\n",
		nid, (unsigned long)NODE_DATA(nid));
}


static unsigned long node_remap_start_pfn[MAX_NUMNODES];
static void *node_remap_end_vaddr[MAX_NUMNODES];
static void *node_remap_alloc_vaddr[MAX_NUMNODES];
static unsigned long node_remap_offset[MAX_NUMNODES];

void *alloc_remap(int nid, unsigned long size)
{
	void *allocation = node_remap_alloc_vaddr[nid];

	size = ALIGN(size, L1_CACHE_BYTES);

	if (!allocation || (allocation + size) >= node_remap_end_vaddr[nid])
		return NULL;

	node_remap_alloc_vaddr[nid] += size;
	memset(allocation, 0, size);

	return allocation;
}

static void __init remap_numa_kva(void)
{
	void *vaddr;
	unsigned long pfn;
	int node;

	for_each_online_node(node) {
		printk(KERN_DEBUG "remap_numa_kva: node %d\n", node);
		for (pfn=0; pfn < node_remap_size[node]; pfn += PTRS_PER_PTE) {
			vaddr = node_remap_start_vaddr[node]+(pfn<<PAGE_SHIFT);
			printk(KERN_DEBUG "remap_numa_kva: %08lx to pfn %08lx\n",
				(unsigned long)vaddr,
				node_remap_start_pfn[node] + pfn);
			set_pmd_pfn((ulong) vaddr, 
				node_remap_start_pfn[node] + pfn, 
				PAGE_KERNEL_LARGE);
		}
	}
}

#ifdef CONFIG_HIBERNATION

void resume_map_numa_kva(pgd_t *pgd_base)
{
	int node;

	for_each_online_node(node) {
		unsigned long start_va, start_pfn, size, pfn;

		start_va = (unsigned long)node_remap_start_vaddr[node];
		start_pfn = node_remap_start_pfn[node];
		size = node_remap_size[node];

		printk(KERN_DEBUG "%s: node %d\n", __func__, node);

		for (pfn = 0; pfn < size; pfn += PTRS_PER_PTE) {
			unsigned long vaddr = start_va + (pfn << PAGE_SHIFT);
			pgd_t *pgd = pgd_base + pgd_index(vaddr);
			pud_t *pud = pud_offset(pgd, vaddr);
			pmd_t *pmd = pmd_offset(pud, vaddr);

			set_pmd(pmd, pfn_pmd(start_pfn + pfn,
						PAGE_KERNEL_LARGE_EXEC));

			printk(KERN_DEBUG "%s: %08lx -> pfn %08lx\n",
				__func__, vaddr, start_pfn + pfn);
		}
	}
}
#endif

static __init unsigned long calculate_numa_remap_pages(void)
{
	int nid;
	unsigned long size, reserve_pages = 0;

	for_each_online_node(nid) {
		u64 node_kva_target;
		u64 node_kva_final;

		
		printk(KERN_DEBUG "node %d pfn: [%lx - %lx]\n",
			nid, node_start_pfn[nid], node_end_pfn[nid]);
		if (node_start_pfn[nid] > max_pfn)
			continue;
		if (!node_end_pfn[nid])
			continue;
		if (node_end_pfn[nid] > max_pfn)
			node_end_pfn[nid] = max_pfn;

		
		size = node_remap_size[nid] + sizeof(pg_data_t);

		
		size = (size + LARGE_PAGE_BYTES - 1) / LARGE_PAGE_BYTES;
		
		size = size * PTRS_PER_PTE;

		node_kva_target = round_down(node_end_pfn[nid] - size,
						 PTRS_PER_PTE);
		node_kva_target <<= PAGE_SHIFT;
		do {
			node_kva_final = find_e820_area(node_kva_target,
					((u64)node_end_pfn[nid])<<PAGE_SHIFT,
						((u64)size)<<PAGE_SHIFT,
						LARGE_PAGE_BYTES);
			node_kva_target -= LARGE_PAGE_BYTES;
		} while (node_kva_final == -1ULL &&
			 (node_kva_target>>PAGE_SHIFT) > (node_start_pfn[nid]));

		if (node_kva_final == -1ULL)
			panic("Can not get kva ram\n");

		node_remap_size[nid] = size;
		node_remap_offset[nid] = reserve_pages;
		reserve_pages += size;
		printk(KERN_DEBUG "Reserving %ld pages of KVA for lmem_map of"
				  " node %d at %llx\n",
				size, nid, node_kva_final>>PAGE_SHIFT);

		
		reserve_early(node_kva_final,
			      node_kva_final+(((u64)size)<<PAGE_SHIFT),
			      "KVA RAM");

		node_remap_start_pfn[nid] = node_kva_final>>PAGE_SHIFT;
		remove_active_range(nid, node_remap_start_pfn[nid],
					 node_remap_start_pfn[nid] + size);
	}
	printk(KERN_INFO "Reserving total of %lx pages for numa KVA remap\n",
			reserve_pages);
	return reserve_pages;
}

static void init_remap_allocator(int nid)
{
	node_remap_start_vaddr[nid] = pfn_to_kaddr(
			kva_start_pfn + node_remap_offset[nid]);
	node_remap_end_vaddr[nid] = node_remap_start_vaddr[nid] +
		(node_remap_size[nid] * PAGE_SIZE);
	node_remap_alloc_vaddr[nid] = node_remap_start_vaddr[nid] +
		ALIGN(sizeof(pg_data_t), PAGE_SIZE);

	printk(KERN_DEBUG "node %d will remap to vaddr %08lx - %08lx\n", nid,
		(ulong) node_remap_start_vaddr[nid],
		(ulong) node_remap_end_vaddr[nid]);
}

void __init initmem_init(unsigned long start_pfn,
				  unsigned long end_pfn)
{
	int nid;
	long kva_target_pfn;

	

	get_memcfg_numa();

	kva_pages = roundup(calculate_numa_remap_pages(), PTRS_PER_PTE);

	kva_target_pfn = round_down(max_low_pfn - kva_pages, PTRS_PER_PTE);
	do {
		kva_start_pfn = find_e820_area(kva_target_pfn<<PAGE_SHIFT,
					max_low_pfn<<PAGE_SHIFT,
					kva_pages<<PAGE_SHIFT,
					PTRS_PER_PTE<<PAGE_SHIFT) >> PAGE_SHIFT;
		kva_target_pfn -= PTRS_PER_PTE;
	} while (kva_start_pfn == -1UL && kva_target_pfn > min_low_pfn);

	if (kva_start_pfn == -1UL)
		panic("Can not get kva space\n");

	printk(KERN_INFO "kva_start_pfn ~ %lx max_low_pfn ~ %lx\n",
		kva_start_pfn, max_low_pfn);
	printk(KERN_INFO "max_pfn = %lx\n", max_pfn);

	
	reserve_early(kva_start_pfn<<PAGE_SHIFT,
		      (kva_start_pfn + kva_pages)<<PAGE_SHIFT,
		     "KVA PG");
#ifdef CONFIG_HIGHMEM
	highstart_pfn = highend_pfn = max_pfn;
	if (max_pfn > max_low_pfn)
		highstart_pfn = max_low_pfn;
	printk(KERN_NOTICE "%ldMB HIGHMEM available.\n",
	       pages_to_mb(highend_pfn - highstart_pfn));
	num_physpages = highend_pfn;
	high_memory = (void *) __va(highstart_pfn * PAGE_SIZE - 1) + 1;
#else
	num_physpages = max_low_pfn;
	high_memory = (void *) __va(max_low_pfn * PAGE_SIZE - 1) + 1;
#endif
	printk(KERN_NOTICE "%ldMB LOWMEM available.\n",
			pages_to_mb(max_low_pfn));
	printk(KERN_DEBUG "max_low_pfn = %lx, highstart_pfn = %lx\n",
			max_low_pfn, highstart_pfn);

	printk(KERN_DEBUG "Low memory ends at vaddr %08lx\n",
			(ulong) pfn_to_kaddr(max_low_pfn));
	for_each_online_node(nid) {
		init_remap_allocator(nid);

		allocate_pgdat(nid);
	}
	remap_numa_kva();

	printk(KERN_DEBUG "High memory starts at vaddr %08lx\n",
			(ulong) pfn_to_kaddr(highstart_pfn));
	for_each_online_node(nid)
		propagate_e820_map_node(nid);

	for_each_online_node(nid) {
		memset(NODE_DATA(nid), 0, sizeof(struct pglist_data));
		NODE_DATA(nid)->bdata = &bootmem_node_data[nid];
	}

	setup_bootmem_allocator();
}

#ifdef CONFIG_MEMORY_HOTPLUG
static int paddr_to_nid(u64 addr)
{
	int nid;
	unsigned long pfn = PFN_DOWN(addr);

	for_each_node(nid)
		if (node_start_pfn[nid] <= pfn &&
		    pfn < node_end_pfn[nid])
			return nid;

	return -1;
}


int memory_add_physaddr_to_nid(u64 addr)
{
	int nid = paddr_to_nid(addr);
	return (nid >= 0) ? nid : 0;
}

EXPORT_SYMBOL_GPL(memory_add_physaddr_to_nid);
#endif

