
#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H


#define PHYS_OFFSET UL(CONFIG_PHYS_OFFSET)

#define MAX_PHYSMEM_BITS 32
#define SECTION_SIZE_BITS 25

#define HAS_ARCH_IO_REMAP_PFN_RANGE

#ifndef __ASSEMBLY__
void *alloc_bootmem_aligned(unsigned long size, unsigned long alignment);
void clean_and_invalidate_caches(unsigned long, unsigned long, unsigned long);
void clean_caches(unsigned long, unsigned long, unsigned long);
void invalidate_caches(unsigned long, unsigned long, unsigned long);
int platform_physical_remove_pages(unsigned long, unsigned long);
int platform_physical_add_pages(unsigned long, unsigned long);
int platform_physical_low_power_pages(unsigned long, unsigned long);

#ifdef CONFIG_ARCH_MSM_ARM11
void write_to_strongly_ordered_memory(void);
void map_zero_page_strongly_ordered(void);


#include <asm/mach-types.h>


#if defined(CONFIG_MACH_LGE)
#define arch_barrier_extra() do \
	{  \
		write_to_strongly_ordered_memory(); \
	} while (0)
#else	
#define arch_barrier_extra() do \
	{ if (machine_is_msm7x27_surf() || machine_is_msm7x27_ffa())  \
		write_to_strongly_ordered_memory(); \
	} while (0)
#endif	
	
#endif

#ifdef CONFIG_CACHE_L2X0
extern void l2x0_cache_sync(void);
#define finish_arch_switch(prev)     do { l2x0_cache_sync(); } while (0)
#endif

#endif

#ifdef CONFIG_ARCH_MSM_SCORPION
#define arch_has_speculative_dfetch()	1
#endif

#endif


#define MEMORY_DEEP_POWERDOWN	0
#define MEMORY_SELF_REFRESH	1
#define MEMORY_ACTIVE		2

#define NPA_MEMORY_NODE_NAME	"/mem/ebi1/cs1"
