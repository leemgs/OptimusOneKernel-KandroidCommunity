
#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H


#include <asm/page.h>
#include <asm/sizes.h>


#define DAVINCI_DDR_BASE	0x80000000
#define DA8XX_DDR_BASE		0xc0000000

#if defined(CONFIG_ARCH_DAVINCI_DA8XX) && defined(CONFIG_ARCH_DAVINCI_DMx)
#error Cannot enable DaVinci and DA8XX platforms concurrently
#elif defined(CONFIG_ARCH_DAVINCI_DA8XX)
#define PHYS_OFFSET DA8XX_DDR_BASE
#else
#define PHYS_OFFSET DAVINCI_DDR_BASE
#endif


#define CONSISTENT_DMA_SIZE (14<<20)

#ifndef __ASSEMBLY__

static inline void
__arch_adjust_zones(int node, unsigned long *size, unsigned long *holes)
{
	unsigned int sz = (128<<20) >> PAGE_SHIFT;

	if (node != 0)
		sz = 0;

	size[1] = size[0] - sz;
	size[0] = sz;
}

#define arch_adjust_zones(node, zone_size, holes) \
        if ((meminfo.bank[0].size >> 20) > 128) __arch_adjust_zones(node, zone_size, holes)

#define ISA_DMA_THRESHOLD	(PHYS_OFFSET + (128<<20) - 1)
#define MAX_DMA_ADDRESS		(PAGE_OFFSET + (128<<20))

#endif

#endif 
