



#ifndef __ASM_ARCH_MXC_MEMORY_H__
#define __ASM_ARCH_MXC_MEMORY_H__

#if defined CONFIG_ARCH_MX1
#define PHYS_OFFSET		UL(0x08000000)
#elif defined CONFIG_ARCH_MX2
#ifdef CONFIG_MACH_MX21
#define PHYS_OFFSET		UL(0xC0000000)
#endif
#ifdef CONFIG_MACH_MX27
#define PHYS_OFFSET		UL(0xA0000000)
#endif
#elif defined CONFIG_ARCH_MX3
#define PHYS_OFFSET		UL(0x80000000)
#elif defined CONFIG_ARCH_MX25
#define PHYS_OFFSET		UL(0x80000000)
#elif defined CONFIG_ARCH_MXC91231
#define PHYS_OFFSET		UL(0x90000000)
#endif

#if defined(CONFIG_MX1_VIDEO)

#define CONSISTENT_DMA_SIZE SZ_4M
#endif 

#if defined(CONFIG_MX3_VIDEO)

#define CONSISTENT_DMA_SIZE SZ_8M
#endif 

#endif 
