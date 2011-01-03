#ifndef __ASM_GENERIC_SCATTERLIST_H
#define __ASM_GENERIC_SCATTERLIST_H

#include <linux/types.h>

struct scatterlist {
#ifdef CONFIG_DEBUG_SG
	unsigned long	sg_magic;
#endif
	unsigned long	page_link;
	unsigned int	offset;
	unsigned int	length;
	dma_addr_t	dma_address;
	unsigned int	dma_length;
};


#define sg_dma_address(sg)	((sg)->dma_address)
#ifndef sg_dma_len

#if __BITS_PER_LONG == 64
#define sg_dma_len(sg)		((sg)->dma_length)
#else
#define sg_dma_len(sg)		((sg)->length)
#endif 
#endif 

#ifndef ISA_DMA_THRESHOLD
#define ISA_DMA_THRESHOLD	(~0UL)
#endif

#define ARCH_HAS_SG_CHAIN

#endif 
