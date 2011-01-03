#ifndef __ASM_ARM_TYPES_H
#define __ASM_ARM_TYPES_H

#include <asm-generic/int-ll64.h>

#ifndef __ASSEMBLY__

typedef unsigned short umode_t;

#endif 


#ifdef __KERNEL__

#define BITS_PER_LONG 32

#ifndef __ASSEMBLY__



typedef u32 dma_addr_t;
typedef u32 dma64_addr_t;

#endif 

#endif 

#endif

