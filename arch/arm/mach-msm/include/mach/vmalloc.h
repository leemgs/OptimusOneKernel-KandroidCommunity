

#ifndef __ASM_ARCH_MSM_VMALLOC_H
#define __ASM_ARCH_MSM_VMALLOC_H

#ifdef CONFIG_VMSPLIT_2G
#define VMALLOC_END	  (PAGE_OFFSET + 0x60000000)
#else
#if defined (CONFIG_LGE_4G_DDR)


#define VMALLOC_END	  (PAGE_OFFSET + 0x30000000)
#else	
#define VMALLOC_END	  (PAGE_OFFSET + 0x20000000)
#endif
#endif

#endif

