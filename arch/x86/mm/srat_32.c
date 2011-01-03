
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/mmzone.h>
#include <linux/acpi.h>
#include <linux/nodemask.h>
#include <asm/srat.h>
#include <asm/topology.h>
#include <asm/smp.h>
#include <asm/e820.h>


#define NODE_ARRAY_INDEX(x)	((x) / 8)	
#define NODE_ARRAY_OFFSET(x)	((x) % 8)	
#define BMAP_SET(bmap, bit)	((bmap)[NODE_ARRAY_INDEX(bit)] |= 1 << NODE_ARRAY_OFFSET(bit))
#define BMAP_TEST(bmap, bit)	((bmap)[NODE_ARRAY_INDEX(bit)] & (1 << NODE_ARRAY_OFFSET(bit)))

#define PXM_BITMAP_LEN (MAX_PXM_DOMAINS / 8) 
static u8 __initdata pxm_bitmap[PXM_BITMAP_LEN];	

#define MAX_CHUNKS_PER_NODE	3
#define MAXCHUNKS		(MAX_CHUNKS_PER_NODE * MAX_NUMNODES)
struct node_memory_chunk_s {
	unsigned long	start_pfn;
	unsigned long	end_pfn;
	u8	pxm;		
	u8	nid;		
	u8	bank;		
};
static struct node_memory_chunk_s __initdata node_memory_chunk[MAXCHUNKS];

static int __initdata num_memory_chunks; 
static u8 __initdata apicid_to_pxm[MAX_APICID];

int numa_off __initdata;
int acpi_numa __initdata;

static __init void bad_srat(void)
{
        printk(KERN_ERR "SRAT: SRAT not used.\n");
        acpi_numa = -1;
	num_memory_chunks = 0;
}

static __init inline int srat_disabled(void)
{
	return numa_off || acpi_numa < 0;
}


void __init
acpi_numa_processor_affinity_init(struct acpi_srat_cpu_affinity *cpu_affinity)
{
	if (srat_disabled())
		return;
	if (cpu_affinity->header.length !=
	     sizeof(struct acpi_srat_cpu_affinity)) {
		bad_srat();
		return;
	}

	if ((cpu_affinity->flags & ACPI_SRAT_CPU_ENABLED) == 0)
		return;		

	
	BMAP_SET(pxm_bitmap, cpu_affinity->proximity_domain_lo);

	apicid_to_pxm[cpu_affinity->apic_id] = cpu_affinity->proximity_domain_lo;

	printk(KERN_DEBUG "CPU %02x in proximity domain %02x\n",
		cpu_affinity->apic_id, cpu_affinity->proximity_domain_lo);
}


void __init
acpi_numa_memory_affinity_init(struct acpi_srat_mem_affinity *memory_affinity)
{
	unsigned long long paddr, size;
	unsigned long start_pfn, end_pfn;
	u8 pxm;
	struct node_memory_chunk_s *p, *q, *pend;

	if (srat_disabled())
		return;
	if (memory_affinity->header.length !=
	     sizeof(struct acpi_srat_mem_affinity)) {
		bad_srat();
		return;
	}

	if ((memory_affinity->flags & ACPI_SRAT_MEM_ENABLED) == 0)
		return;		

	pxm = memory_affinity->proximity_domain & 0xff;

	
	BMAP_SET(pxm_bitmap, pxm);

	
	paddr = memory_affinity->base_address;
	size = memory_affinity->length;

	start_pfn = paddr >> PAGE_SHIFT;
	end_pfn = (paddr + size) >> PAGE_SHIFT;


	if (num_memory_chunks >= MAXCHUNKS) {
		printk(KERN_WARNING "Too many mem chunks in SRAT."
			" Ignoring %lld MBytes at %llx\n",
			size/(1024*1024), paddr);
		return;
	}

	
	pend = &node_memory_chunk[num_memory_chunks];
	for (p = &node_memory_chunk[0]; p < pend; p++) {
		if (start_pfn < p->start_pfn)
			break;
	}
	if (p < pend) {
		for (q = pend; q >= p; q--)
			*(q + 1) = *q;
	}
	p->start_pfn = start_pfn;
	p->end_pfn = end_pfn;
	p->pxm = pxm;

	num_memory_chunks++;

	printk(KERN_DEBUG "Memory range %08lx to %08lx"
			  " in proximity domain %02x %s\n",
		start_pfn, end_pfn,
		pxm,
		((memory_affinity->flags & ACPI_SRAT_MEM_HOT_PLUGGABLE) ?
		 "enabled and removable" : "enabled" ) );
}


void __init acpi_numa_slit_init(struct acpi_table_slit *slit)
{
}

void acpi_numa_arch_fixup(void)
{
}

static __init int node_read_chunk(int nid, struct node_memory_chunk_s *memory_chunk)
{
	
	if (memory_chunk->start_pfn >= max_pfn) {
		printk(KERN_INFO "Ignoring SRAT pfns: %08lx - %08lx\n",
			memory_chunk->start_pfn, memory_chunk->end_pfn);
		return -1;
	}
	if (memory_chunk->nid != nid)
		return -1;

	if (!node_has_online_mem(nid))
		node_start_pfn[nid] = memory_chunk->start_pfn;

	if (node_start_pfn[nid] > memory_chunk->start_pfn)
		node_start_pfn[nid] = memory_chunk->start_pfn;

	if (node_end_pfn[nid] < memory_chunk->end_pfn)
		node_end_pfn[nid] = memory_chunk->end_pfn;

	return 0;
}

int __init get_memcfg_from_srat(void)
{
	int i, j, nid;


	if (srat_disabled())
		goto out_fail;

	if (num_memory_chunks == 0) {
		printk(KERN_DEBUG
			 "could not find any ACPI SRAT memory areas.\n");
		goto out_fail;
	}

	
	
	nodes_clear(node_online_map);
	for (i = 0; i < MAX_PXM_DOMAINS; i++) {
		if (BMAP_TEST(pxm_bitmap, i)) {
			int nid = acpi_map_pxm_to_node(i);
			node_set_online(nid);
		}
	}
	BUG_ON(num_online_nodes() == 0);

	
	for (i = 0; i < num_memory_chunks; i++)
		node_memory_chunk[i].nid = pxm_to_node(node_memory_chunk[i].pxm);

	printk(KERN_DEBUG "pxm bitmap: ");
	for (i = 0; i < sizeof(pxm_bitmap); i++) {
		printk(KERN_CONT "%02x ", pxm_bitmap[i]);
	}
	printk(KERN_CONT "\n");
	printk(KERN_DEBUG "Number of logical nodes in system = %d\n",
			 num_online_nodes());
	printk(KERN_DEBUG "Number of memory chunks in system = %d\n",
			 num_memory_chunks);

	for (i = 0; i < MAX_APICID; i++)
		apicid_2_node[i] = pxm_to_node(apicid_to_pxm[i]);

	for (j = 0; j < num_memory_chunks; j++){
		struct node_memory_chunk_s * chunk = &node_memory_chunk[j];
		printk(KERN_DEBUG
			"chunk %d nid %d start_pfn %08lx end_pfn %08lx\n",
		       j, chunk->nid, chunk->start_pfn, chunk->end_pfn);
		if (node_read_chunk(chunk->nid, chunk))
			continue;

		e820_register_active_regions(chunk->nid, chunk->start_pfn,
					     min(chunk->end_pfn, max_pfn));
	}

	for_each_online_node(nid) {
		unsigned long start = node_start_pfn[nid];
		unsigned long end = min(node_end_pfn[nid], max_pfn);

		memory_present(nid, start, end);
		node_remap_size[nid] = node_memmap_size_bytes(nid, start, end);
	}
	return 1;
out_fail:
	printk(KERN_DEBUG "failed to get NUMA memory information from SRAT"
			" table\n");
	return 0;
}
