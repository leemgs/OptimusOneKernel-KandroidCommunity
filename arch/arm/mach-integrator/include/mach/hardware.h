
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <asm/sizes.h>
#include <mach/platform.h>


#define IO_BASE			0xF0000000                 
#define IO_SIZE			0x0B000000                 
#define IO_START		INTEGRATOR_HDR_BASE        

#define PCIO_BASE		PCI_IO_VADDR
#define PCIMEM_BASE		PCI_MEMORY_VADDR

#ifdef CONFIG_MMU

#define IO_ADDRESS(x) (((x) >> 4) + IO_BASE) 
#else
#define IO_ADDRESS(x) (x)
#endif

#define pcibios_assign_all_busses()	1

#define PCIBIOS_MIN_IO		0x6000
#define PCIBIOS_MIN_MEM 	0x00100000

#endif

