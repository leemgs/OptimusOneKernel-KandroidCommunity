#ifndef _ASM_GENERIC_TYPES_H
#define _ASM_GENERIC_TYPES_H

#include <asm-generic/int-ll64.h>

#ifndef __ASSEMBLY__

typedef unsigned short umode_t;

#endif 


#ifdef __KERNEL__
#ifndef __ASSEMBLY__

#ifndef dma_addr_t
#ifdef CONFIG_PHYS_ADDR_T_64BIT
typedef u64 dma_addr_t;
#else
typedef u32 dma_addr_t;
#endif 
#endif 

#endif 

#endif 

#endif 
