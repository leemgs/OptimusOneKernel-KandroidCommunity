

#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

#include <mach/hardware.h>


#define PHYS_OFFSET		KS8695_SDRAM_PA

#ifndef __ASSEMBLY__

#ifdef CONFIG_PCI


#define __virt_to_bus(x)	((x) - PAGE_OFFSET + KS8695_PCIMEM_PA)
#define __bus_to_virt(x)	((x) - KS8695_PCIMEM_PA + PAGE_OFFSET)


extern struct bus_type platform_bus_type;
#define is_lbus_device(dev)		(dev && dev->bus == &platform_bus_type)
#define __arch_dma_to_virt(dev, x)	({ (void *) (is_lbus_device(dev) ? \
					__phys_to_virt(x) : __bus_to_virt(x)); })
#define __arch_virt_to_dma(dev, x)	({ is_lbus_device(dev) ? \
					(dma_addr_t)__virt_to_phys(x) : (dma_addr_t)__virt_to_bus(x); })
#define __arch_page_to_dma(dev, x)	\
	({ dma_addr_t __dma = page_to_phys(page); \
	   if (!is_lbus_device(dev)) \
		__dma = __dma - PHYS_OFFSET + KS8695_PCIMEM_PA; \
	   __dma; })

#define __arch_dma_to_page(dev, x)	\
	({ dma_addr_t __dma = x;				\
	   if (!is_lbus_device(dev))				\
		__dma += PHYS_OFFSET - KS8695_PCIMEM_PA;	\
	   phys_to_page(__dma);					\
	})

#endif

#endif

#endif
